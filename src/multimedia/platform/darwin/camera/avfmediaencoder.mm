/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd and/or its subsidiary(-ies).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/


#include "avfmediaencoder_p.h"
#include "avfcamerarenderer_p.h"
#include "avfcamerasession_p.h"
#include "avfcamera_p.h"
#include "avfcameraservice_p.h"
#include "avfcameradebug_p.h"
#include "avfcamerautility_p.h"

#include "qaudiodevice.h"
#include "qmediadevices.h"
#include "private/qmediarecorder_p.h"
#include "private/qdarwinformatsinfo_p.h"
#include "private/qplatformaudiooutput_p.h"

#include <QtCore/qmath.h>
#include <QtCore/qdebug.h>
#include <QtCore/qmimetype.h>

QT_USE_NAMESPACE

namespace {

bool qt_is_writable_file_URL(NSURL *fileURL)
{
    Q_ASSERT(fileURL);

    if (![fileURL isFileURL])
        return false;

    if (NSString *path = [[fileURL path] stringByExpandingTildeInPath]) {
        return [[NSFileManager defaultManager]
                isWritableFileAtPath:[path stringByDeletingLastPathComponent]];
    }

    return false;
}

bool qt_file_exists(NSURL *fileURL)
{
    Q_ASSERT(fileURL);

    if (NSString *path = [[fileURL path] stringByExpandingTildeInPath])
        return [[NSFileManager defaultManager] fileExistsAtPath:path];

    return false;
}

}

AVFMediaEncoder::AVFMediaEncoder(QMediaRecorder *parent)
    : QObject(parent)
    , QPlatformMediaEncoder(parent)
    , m_state(QMediaRecorder::StoppedState)
    , m_duration(0)
    , m_audioSettings(nil)
    , m_videoSettings(nil)
    //, m_restoreFPS(-1, -1)
{
    m_writer.reset([[QT_MANGLE_NAMESPACE(AVFMediaAssetWriter) alloc] initWithDelegate:this]);
    if (!m_writer) {
        qDebugCamera() << Q_FUNC_INFO << "failed to create an asset writer";
        return;
    }
}

AVFMediaEncoder::~AVFMediaEncoder()
{
    [m_writer abort];

    if (m_audioSettings)
        [m_audioSettings release];
    if (m_videoSettings)
        [m_videoSettings release];
}

bool AVFMediaEncoder::isLocationWritable(const QUrl &location) const
{
    return location.scheme() == QLatin1String("file") || location.scheme().isEmpty();
}

QMediaRecorder::RecorderState AVFMediaEncoder::state() const
{
    return m_state;
}

qint64 AVFMediaEncoder::duration() const
{
    return m_duration;
}

void AVFMediaEncoder::updateDuration(qint64 duration)
{
    m_duration = duration;
    durationChanged(m_duration);
}

static NSDictionary *avfAudioSettings(const QMediaEncoderSettings &encoderSettings)
{
    NSMutableDictionary *settings = [NSMutableDictionary dictionary];

    int codecId = QDarwinFormatInfo::audioFormatForCodec(encoderSettings.mediaFormat().audioCodec());
    [settings setObject:[NSNumber numberWithInt:codecId] forKey:AVFormatIDKey];

    // Setting AVEncoderQualityKey is not allowed when format ID is alac or lpcm
    if (codecId != kAudioFormatAppleLossless && codecId != kAudioFormatLinearPCM
        && encoderSettings.encodingMode() == QMediaRecorder::ConstantQualityEncoding) {
        int quality;
        switch (encoderSettings.quality()) {
        case QMediaRecorder::VeryLowQuality:
            quality = AVAudioQualityMin;
            break;
        case QMediaRecorder::LowQuality:
            quality = AVAudioQualityLow;
            break;
        case QMediaRecorder::HighQuality:
            quality = AVAudioQualityHigh;
            break;
        case QMediaRecorder::VeryHighQuality:
            quality = AVAudioQualityMax;
            break;
        case QMediaRecorder::NormalQuality:
        default:
            quality = AVAudioQualityMedium;
            break;
        }
        [settings setObject:[NSNumber numberWithInt:quality] forKey:AVEncoderAudioQualityKey];
    } else {
        bool isBitRateSupported = false;
        int bitRate = encoderSettings.audioBitRate();
        if (bitRate > 0) {
            QList<AudioValueRange> bitRates = qt_supported_bit_rates_for_format(codecId);
            for (int i = 0; i < bitRates.count(); i++) {
                if (bitRate >= bitRates[i].mMinimum &&
                    bitRate <= bitRates[i].mMaximum) {
                    isBitRateSupported = true;
                    break;
                }
            }
            if (isBitRateSupported)
                [settings setObject:[NSNumber numberWithInt:encoderSettings.audioBitRate()]
                                              forKey:AVEncoderBitRateKey];
        }
    }

    int sampleRate = encoderSettings.audioSampleRate();
    bool isSampleRateSupported = false;
    if (sampleRate >= 8000 && sampleRate <= 192000) {
        QList<AudioValueRange> sampleRates = qt_supported_sample_rates_for_format(codecId);
        for (int i = 0; i < sampleRates.count(); i++) {
            if (sampleRate >= sampleRates[i].mMinimum && sampleRate <= sampleRates[i].mMaximum) {
                isSampleRateSupported = true;
                break;
            }
        }
    }

    int channelCount = encoderSettings.audioChannelCount();
    bool isChannelCountSupported = false;
    if (channelCount > 0) {
        std::optional<QList<UInt32>> channelCounts = qt_supported_channel_counts_for_format(codecId);
        // An std::nullopt result indicates that
        // any number of channels can be encoded.
        if (channelCounts == std::nullopt) {
            isChannelCountSupported = true;
        } else {
            for (int i = 0; i < channelCounts.value().count(); i++) {
                if ((UInt32)channelCount == channelCounts.value()[i]) {
                    isChannelCountSupported = true;
                    break;
                }
            }
        }
    }

#ifdef Q_OS_IOS
    // Some keys are mandatory only on iOS
    if (!isSampleRateSupported) {
        sampleRate = 44100;
        isSampleRateSupported = true;
    }
    if (!isChannelCountSupported) {
        channelCount = 2;
        isChannelCountSupported = true;
    }

    if (codecId == kAudioFormatAppleLossless)
        [settings setObject:[NSNumber numberWithInt:24] forKey:AVEncoderBitDepthHintKey];

    if (codecId == kAudioFormatLinearPCM) {
        [settings setObject:[NSNumber numberWithInt:16] forKey:AVLinearPCMBitDepthKey];
        [settings setObject:[NSNumber numberWithInt:NO] forKey:AVLinearPCMIsBigEndianKey];
        [settings setObject:[NSNumber numberWithInt:NO] forKey:AVLinearPCMIsFloatKey];
        [settings setObject:[NSNumber numberWithInt:NO] forKey:AVLinearPCMIsNonInterleaved];
    }
#endif
    if (isSampleRateSupported)
        [settings setObject:[NSNumber numberWithInt:sampleRate] forKey:AVSampleRateKey];
    if (isChannelCountSupported)
        [settings setObject:[NSNumber numberWithInt:channelCount] forKey:AVNumberOfChannelsKey];

    return settings;
}

NSDictionary *avfVideoSettings(QMediaEncoderSettings &encoderSettings, AVCaptureDevice *device, AVCaptureConnection *connection)
{
    if (!device)
        return nil;


    // ### re-add needFpsChange
//    AVFPSRange currentFps = qt_current_framerates(device, connection);

    NSMutableDictionary *videoSettings = [NSMutableDictionary dictionary];

    // -- Codec

    // AVVideoCodecKey is the only mandatory key
    auto codec = encoderSettings.mediaFormat().videoCodec();
    NSString *c = QDarwinFormatInfo::videoFormatForCodec(codec);
    [videoSettings setObject:c forKey:AVVideoCodecKey];
    [c release];

    // -- Resolution

    int w = encoderSettings.videoResolution().width();
    int h = encoderSettings.videoResolution().height();

    if (AVCaptureDeviceFormat *currentFormat = device.activeFormat) {
        CMFormatDescriptionRef formatDesc = currentFormat.formatDescription;
        CMVideoDimensions dim = CMVideoFormatDescriptionGetDimensions(formatDesc);
        FourCharCode formatCodec = CMVideoFormatDescriptionGetCodecType(formatDesc);

        // We have to change the device's activeFormat in 3 cases:
        // - the requested recording resolution is higher than the current device resolution
        // - the requested recording resolution has a different aspect ratio than the current device aspect ratio
        // - the requested frame rate is not available for the current device format
        AVCaptureDeviceFormat *newFormat = nil;
        if ((w <= 0 || h <= 0)
                && encoderSettings.videoFrameRate() > 0
                && !qt_format_supports_framerate(currentFormat, encoderSettings.videoFrameRate())) {

            newFormat = qt_find_best_framerate_match(device,
                                                     formatCodec,
                                                     encoderSettings.videoFrameRate());

        } else if (w > 0 && h > 0) {
            AVCaptureDeviceFormat *f = qt_find_best_resolution_match(device,
                                                                     encoderSettings.videoResolution(),
                                                                     formatCodec);

            if (f) {
                CMVideoDimensions d = CMVideoFormatDescriptionGetDimensions(f.formatDescription);
                qreal fAspectRatio = qreal(d.width) / d.height;

                if (w > dim.width || h > dim.height
                        || qAbs((qreal(dim.width) / dim.height) - fAspectRatio) > 0.01) {
                    newFormat = f;
                }
            }
        }

        if (qt_set_active_format(device, newFormat, false /*### !needFpsChange*/)) {
            formatDesc = newFormat.formatDescription;
            dim = CMVideoFormatDescriptionGetDimensions(formatDesc);
        }

        if (w > 0 && h > 0) {
            // Make sure the recording resolution has the same aspect ratio as the device's
            // current resolution
            qreal deviceAspectRatio = qreal(dim.width) / dim.height;
            qreal recAspectRatio = qreal(w) / h;
            if (qAbs(deviceAspectRatio - recAspectRatio) > 0.01) {
                if (recAspectRatio > deviceAspectRatio)
                    w = qRound(h * deviceAspectRatio);
                else
                    h = qRound(w / deviceAspectRatio);
            }

            // recording resolution can't be higher than the device's active resolution
            w = qMin(w, dim.width);
            h = qMin(h, dim.height);
        }
    }

    if (w > 0 && h > 0) {
        // Width and height must be divisible by 2
        w += w & 1;
        h += h & 1;

        [videoSettings setObject:[NSNumber numberWithInt:w] forKey:AVVideoWidthKey];
        [videoSettings setObject:[NSNumber numberWithInt:h] forKey:AVVideoHeightKey];
        encoderSettings.setVideoResolution(QSize(w, h));
    } else {
        encoderSettings.setVideoResolution(qt_device_format_resolution(device.activeFormat));
    }

    // -- FPS

    if (true /*needFpsChange*/) {
        const qreal fps = encoderSettings.videoFrameRate();
        qt_set_framerate_limits(device, connection, fps, fps);
    }
    encoderSettings.setVideoFrameRate(qt_current_framerates(device, connection).second);

    // -- Codec Settings

    NSMutableDictionary *codecProperties = [NSMutableDictionary dictionary];
    int bitrate = -1;
    float quality = -1.f;

    if (encoderSettings.encodingMode() == QMediaRecorder::ConstantQualityEncoding) {
        if (encoderSettings.quality() != QMediaRecorder::NormalQuality) {
            if (codec != QMediaFormat::VideoCodec::MotionJPEG) {
                qWarning("ConstantQualityEncoding is not supported for MotionJPEG");
            } else {
                switch (encoderSettings.quality()) {
                case QMediaRecorder::VeryLowQuality:
                    quality = 0.f;
                    break;
                case QMediaRecorder::LowQuality:
                    quality = 0.25f;
                    break;
                case QMediaRecorder::HighQuality:
                    quality = 0.75f;
                    break;
                case QMediaRecorder::VeryHighQuality:
                    quality = 1.f;
                    break;
                default:
                    quality = -1.f; // NormalQuality, let the system decide
                    break;
                }
            }
        }
    } else if (encoderSettings.encodingMode() == QMediaRecorder::AverageBitRateEncoding){
        if (codec != QMediaFormat::VideoCodec::H264 && codec != QMediaFormat::VideoCodec::H265)
            qWarning() << "AverageBitRateEncoding is not supported for codec" <<  QMediaFormat::videoCodecName(codec);
        else
            bitrate = encoderSettings.videoBitRate();
    } else {
        qWarning("Encoding mode is not supported");
    }

    if (bitrate != -1)
        [codecProperties setObject:[NSNumber numberWithInt:bitrate] forKey:AVVideoAverageBitRateKey];
    if (quality != -1.f)
        [codecProperties setObject:[NSNumber numberWithFloat:quality] forKey:AVVideoQualityKey];

    [videoSettings setObject:codecProperties forKey:AVVideoCompressionPropertiesKey];

    return videoSettings;
}

void AVFMediaEncoder::applySettings(QMediaEncoderSettings &settings)
{
    AVFCameraSession *session = m_service->session();

    // audio settings
    m_audioSettings = avfAudioSettings(settings);
    if (m_audioSettings)
        [m_audioSettings retain];

    // video settings
    AVCaptureDevice *device = session->videoCaptureDevice();
    if (!device)
        return;
    const AVFConfigurationLock lock(device); // prevents activeFormat from being overridden
    AVCaptureConnection *conn = [session->videoOutput()->videoDataOutput() connectionWithMediaType:AVMediaTypeVideo];
    m_videoSettings = avfVideoSettings(settings, device, conn);
    if (m_videoSettings)
        [m_videoSettings retain];
}

void AVFMediaEncoder::unapplySettings()
{
    if (m_audioSettings) {
        [m_audioSettings release];
        m_audioSettings = nil;
    }
    if (m_videoSettings) {
        [m_videoSettings release];
        m_videoSettings = nil;
    }
}

void AVFMediaEncoder::setMetaData(const QMediaMetaData &metaData)
{
    m_metaData = metaData;
}

QMediaMetaData AVFMediaEncoder::metaData() const
{
    return m_metaData;
}

void AVFMediaEncoder::setCaptureSession(QPlatformMediaCaptureSession *session)
{
    AVFCameraService *captureSession = static_cast<AVFCameraService *>(session);
    if (m_service == captureSession)
        return;

    if (m_service)
        stop();

    m_service = captureSession;
    if (!m_service)
        return;

    connect(m_service, &AVFCameraService::cameraChanged, this, &AVFMediaEncoder::onCameraChanged);
    onCameraChanged();
}

void AVFMediaEncoder::record(QMediaEncoderSettings &settings)
{
    if (!m_service || !m_service->session()) {
        qWarning() << Q_FUNC_INFO << "Encoder is not set to a capture session";
        return;
    }

    if (!m_writer) {
        qDebugCamera() << Q_FUNC_INFO << "Invalid recorder";
        return;
    }

    if (QMediaRecorder::RecordingState == m_state)
        return;

    m_service->session()->setActive(true);
    const bool audioOnly = settings.videoCodec() == QMediaFormat::VideoCodec::Unspecified;
    AVCaptureSession *session = m_service->session()->captureSession();
    float rotation = 0;

    if (!audioOnly) {
        AVFCamera *cameraControl = m_service->avfCameraControl();
        if (!cameraControl || !cameraControl->isActive()) {
            qDebugCamera() << Q_FUNC_INFO << "can not start record while camera is not active";
            Q_EMIT error(QMediaRecorder::ResourceError,
                         QMediaRecorderPrivate::msgFailedStartRecording());
            return;
        }

        // Make sure the video is recorded in device orientation.
        // The top of the video will match the side of the device which is on top
        // when recording starts (regardless of the UI orientation).
        // QCameraDevice cameraDevice = m_service->session()->activecameraDevice();
        // int screenOrientation = 360 - m_orientationHandler.currentOrientation();

            // ###
    //        if (cameraDevice.position() == QCameraDevice::FrontFace)
    //            rotation = (screenOrientation + cameraDevice.orientation()) % 360;
    //        else
    //            rotation = (screenOrientation + (360 - cameraDevice.orientation())) % 360;
    }

    const QString path(outputLocation().scheme() == QLatin1String("file") ?
                           outputLocation().path() : outputLocation().toString());
    const QUrl fileURL(QUrl::fromLocalFile(m_storageLocation.generateFileName(path,
                    audioOnly ? AVFStorageLocation::Audio : AVFStorageLocation::Video,
                    QLatin1String("clip_"),
                    settings.mimeType().preferredSuffix())));

    NSURL *nsFileURL = fileURL.toNSURL();
    if (!nsFileURL) {
        qWarning() << Q_FUNC_INFO << "invalid output URL:" << fileURL;
        Q_EMIT error(QMediaRecorder::ResourceError, tr("Invalid output file URL"));
        return;
    }
    if (!qt_is_writable_file_URL(nsFileURL)) {
        qWarning() << Q_FUNC_INFO << "invalid output URL:" << fileURL
                    << "(the location is not writable)";
        Q_EMIT error(QMediaRecorder::ResourceError, tr("Non-writeable file location"));
        return;
    }
    if (qt_file_exists(nsFileURL)) {
        // We test for/handle this error here since AWAssetWriter will raise an
        // Objective-C exception, which is not good at all.
        qWarning() << Q_FUNC_INFO << "invalid output URL:" << fileURL
                    << "(file already exists)";
        Q_EMIT error(QMediaRecorder::ResourceError, tr("File already exists"));
        return;
    }

    applySettings(settings);

    // We stop session now so that no more frames for renderer's queue
    // generated, will restart in assetWriterStarted.
    [session stopRunning];

    if ([m_writer setupWithFileURL:nsFileURL
                    cameraService:m_service
                    audioSettings:m_audioSettings
                    videoSettings:m_videoSettings
                    fileFormat:settings.fileFormat()
                    transform:CGAffineTransformMakeRotation(qDegreesToRadians(rotation))]) {

        m_state = QMediaRecorder::RecordingState;

        Q_EMIT actualLocationChanged(fileURL);
        Q_EMIT stateChanged(m_state);

        // Apple recommends to call startRunning and do all
        // setup on a special queue, and that's what we had
        // initially (dispatch_async to writerQueue). Unfortunately,
        // writer's queue is not the only queue/thread that can
        // access/modify the session, and as a result we have
        // all possible data/race-conditions with Obj-C exceptions
        // at best and something worse in general.
        // Now we try to only modify session on the same thread.
        [m_writer start];
    } else {
        [session startRunning];
        Q_EMIT error(QMediaRecorder::FormatError,
                     QMediaRecorderPrivate::msgFailedStartRecording());
    }
}

void AVFMediaEncoder::stop()
{
    if (m_state != QMediaRecorder::StoppedState) {
        // Do not check the camera status, we can stop if we started.
        stopWriter();
    }
}

void AVFMediaEncoder::assetWriterStarted()
{
}

void AVFMediaEncoder::assetWriterFinished()
{
    Q_ASSERT(m_service && m_service->session());
    AVFCameraSession *session = m_service->session();

    const QMediaRecorder::RecorderState lastState = m_state;

    unapplySettings();

    if (session->videoOutput()) {
        session->videoOutput()->resetCaptureDelegate();
    }
    [session->captureSession() startRunning];

    m_state = QMediaRecorder::StoppedState;
    if (m_state != lastState)
        Q_EMIT stateChanged(m_state);
}

void AVFMediaEncoder::onCameraChanged()
{
    if (m_service && m_service->avfCameraControl()) {
        AVFCamera *cameraControl = m_service->avfCameraControl();
        connect(cameraControl, SIGNAL(activeChanged(bool)),
                            SLOT(cameraActiveChanged(bool)));
    }
}

void AVFMediaEncoder::cameraActiveChanged(bool active)
{
    Q_ASSERT(m_service);
    AVFCamera *cameraControl = m_service->avfCameraControl();
    Q_ASSERT(cameraControl);

    if (!active) {
        return stopWriter();
    }
}

void AVFMediaEncoder::stopWriter()
{
    [m_writer stop];
}

#include "moc_avfmediaencoder_p.cpp"
