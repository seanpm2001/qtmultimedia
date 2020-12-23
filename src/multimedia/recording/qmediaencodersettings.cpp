/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "qmediaencodersettings.h"

QT_BEGIN_NAMESPACE

static void qRegisterEncoderSettingsMetaTypes()
{
    qRegisterMetaType<QAudioEncoderSettings>();
    qRegisterMetaType<QVideoEncoderSettings>();
    qRegisterMetaType<QImageEncoderSettings>();
}

Q_CONSTRUCTOR_FUNCTION(qRegisterEncoderSettingsMetaTypes)


class QAudioEncoderSettingsPrivate  : public QSharedData
{
public:
    bool isNull = true;
    QMultimedia::EncodingMode encodingMode = QMultimedia::ConstantQualityEncoding;
    QString codec;
    int bitrate = -1;
    int sampleRate = -1;
    int channels = -1;
    QMultimedia::EncodingQuality quality = QMultimedia::NormalQuality;
    QVariantMap encodingOptions;
};

/*!
    \class QAudioEncoderSettings

    \brief The QAudioEncoderSettings class provides a set of audio encoder settings.

    \inmodule QtMultimedia
    \ingroup multimedia
    \ingroup multimedia_recording

    A audio encoder settings object is used to specify the audio encoder
    settings used by QMediaRecorder.  Audio encoder settings are selected by
    constructing a QAudioEncoderSettings object, setting the desired properties
    and then passing it to a QMediaRecorder instance using the
    QMediaRecorder::setEncodingSettings() function.

    \snippet multimedia-snippets/media.cpp Audio encoder settings

    \sa QMediaRecorder, QAudioEncoderSettingsControl
*/

/*!
    Construct a null audio encoder settings object.
*/
QAudioEncoderSettings::QAudioEncoderSettings()
    :d(new QAudioEncoderSettingsPrivate)
{
}

/*!
    Constructs a copy of the audio encoder settings object \a other.
*/

QAudioEncoderSettings::QAudioEncoderSettings(const QAudioEncoderSettings& other)
    :d(other.d)
{
}

/*!
    Destroys an audio encoder settings object.
*/

QAudioEncoderSettings::~QAudioEncoderSettings()
{
}

/*!
    Assigns the value of \a other to an audio encoder settings object.
*/

QAudioEncoderSettings& QAudioEncoderSettings::operator=(const QAudioEncoderSettings &other)
{
    d = other.d;
    return *this;
}

/*!
    Determines if \a other is of equal value to an audio encoder settings
    object.

    Returns true if the settings objects are of equal value, and false if they
    are not of equal value.
*/

bool QAudioEncoderSettings::operator==(const QAudioEncoderSettings &other) const
{
    return (d == other.d) ||
           (d->isNull == other.d->isNull &&
            d->encodingMode == other.d->encodingMode &&
            d->bitrate == other.d->bitrate &&
            d->sampleRate == other.d->sampleRate &&
            d->channels == other.d->channels &&
            d->quality == other.d->quality &&
            d->codec == other.d->codec &&
            d->encodingOptions == other.d->encodingOptions);
}

/*!
    Determines if \a other is of equal value to an audio encoder settings
    object.

    Returns true if the settings objects are not of equal value, and true if
    they are of equal value.
*/

bool QAudioEncoderSettings::operator!=(const QAudioEncoderSettings &other) const
{
    return !(*this == other);
}

/*!
    Identifies if an audio settings object is initialized.

    Returns true if the settings object is null, and false if it is not.
*/

bool QAudioEncoderSettings::isNull() const
{
    return d->isNull;
}

/*!
    Returns the audio encoding mode.

    \sa QMultimedia::EncodingMode
*/
QMultimedia::EncodingMode QAudioEncoderSettings::encodingMode() const
{
    return d->encodingMode;
}

/*!
    Sets the audio encoding \a mode setting.

    If QMultimedia::ConstantQualityEncoding is set, the quality
    encoding parameter is used and bit rate is ignored,
    otherwise the bitrate is used.

    The audio codec, channels count and sample rate settings are used in all
    the encoding modes.

    \sa encodingMode(), QMultimedia::EncodingMode
*/
void QAudioEncoderSettings::setEncodingMode(QMultimedia::EncodingMode mode)
{
    d->encodingMode = mode;
}

/*!
    Returns the audio codec.
*/
QString QAudioEncoderSettings::codec() const
{
    return d->codec;
}

/*!
    Sets the audio \a codec.
*/
void QAudioEncoderSettings::setCodec(const QString& codec)
{
    d->isNull = false;
    d->codec = codec;
}

/*!
    Returns the bit rate of the compressed audio stream in bits per second.
*/
int QAudioEncoderSettings::bitRate() const
{
    return d->bitrate;
}

/*!
    Returns the number of audio channels.
*/
int QAudioEncoderSettings::channelCount() const
{
    return d->channels;
}

/*!
    Sets the number of audio \a channels.

    A value of -1 indicates the encoder should make an optimal choice based on
    what is available from the audio source and the limitations of the codec.
*/
void QAudioEncoderSettings::setChannelCount(int channels)
{
    d->isNull = false;
    d->channels = channels;
}

/*!
    Sets the audio bit \a rate in bits per second.
*/
void QAudioEncoderSettings::setBitRate(int rate)
{
    d->isNull = false;
    d->bitrate = rate;
}

/*!
    Returns the audio sample rate in Hz.
*/
int QAudioEncoderSettings::sampleRate() const
{
    return d->sampleRate;
}

/*!
    Sets the audio sample \a rate in Hz.

    A value of -1 indicates the encoder should make an optimal choice based on what is avaialbe
    from the audio source and the limitations of the codec.
  */
void QAudioEncoderSettings::setSampleRate(int rate)
{
    d->isNull = false;
    d->sampleRate = rate;
}

/*!
    Returns the audio encoding quality.
*/

QMultimedia::EncodingQuality QAudioEncoderSettings::quality() const
{
    return d->quality;
}

/*!
    Set the audio encoding \a quality.

    Setting the audio quality parameter allows backend to choose the balanced
    set of encoding parameters to achieve the desired quality level.

    The \a quality settings parameter is only used in the
    \l {QMultimedia::ConstantQualityEncoding}{constant quality} \l{encodingMode()}{encoding mode}.
*/
void QAudioEncoderSettings::setQuality(QMultimedia::EncodingQuality quality)
{
    d->isNull = false;
    d->quality = quality;
}

/*!
    Returns the value of encoding \a option.

    \sa setEncodingOption(), encodingOptions()
*/
QVariant QAudioEncoderSettings::encodingOption(const QString &option) const
{
    return d->encodingOptions.value(option);
}

/*!
    Returns the all the encoding options as QVariantMap.

    \sa encodingOption(), setEncodingOptions()
*/
QVariantMap QAudioEncoderSettings::encodingOptions() const
{
    return d->encodingOptions;
}

/*!
    Set the encoding \a option to \a value.

    The supported set and meaning of encoding options are
    system and selected codec specific.

    \sa encodingOption(), setEncodingOptions()
*/
void QAudioEncoderSettings::setEncodingOption(const QString &option, const QVariant &value)
{
    d->isNull = false;
    if (value.isNull())
        d->encodingOptions.remove(option);
    else
        d->encodingOptions.insert(option, value);
}

/*!
    Replace all the encoding options with \a options.

    The supported set and meaning of encoding options are
    system and selected codec specific.

    \sa encodingOption(), setEncodingOption()
*/
void QAudioEncoderSettings::setEncodingOptions(const QVariantMap &options)
{
    d->isNull = false;
    d->encodingOptions = options;
}

class QVideoEncoderSettingsPrivate  : public QSharedData
{
public:
    bool isNull = true;
    QMultimedia::EncodingMode encodingMode = QMultimedia::ConstantQualityEncoding;
    QString codec;
    int bitrate = -1;
    QSize resolution;
    qreal frameRate = 0;
    QMultimedia::EncodingQuality quality = QMultimedia::NormalQuality;
    QVariantMap encodingOptions;
};

/*!
    \class QVideoEncoderSettings

    \brief The QVideoEncoderSettings class provides a set of video encoder settings.

    \inmodule QtMultimedia
    \ingroup multimedia
    \ingroup multimedia_recording

    A video encoder settings object is used to specify the video encoder
    settings used by QMediaRecorder.  Video encoder settings are selected by
    constructing a QVideoEncoderSettings object, setting the desired properties
    and then passing it to a QMediaRecorder instance using the
    QMediaRecorder::setEncodingSettings() function.

    \snippet multimedia-snippets/media.cpp Video encoder settings

    \sa QMediaRecorder, QVideoEncoderSettingsControl
*/

/*!
    Constructs a null video encoder settings object.
*/

QVideoEncoderSettings::QVideoEncoderSettings()
    :d(new QVideoEncoderSettingsPrivate)
{
}

/*!
    Constructs a copy of the video encoder settings object \a other.
*/

QVideoEncoderSettings::QVideoEncoderSettings(const QVideoEncoderSettings& other)
    :d(other.d)
{
}

/*!
    Destroys a video encoder settings object.
*/

QVideoEncoderSettings::~QVideoEncoderSettings()
{
}

/*!
    Assigns the value of \a other to a video encoder settings object.
*/
QVideoEncoderSettings &QVideoEncoderSettings::operator=(const QVideoEncoderSettings &other)
{
    d = other.d;
    return *this;
}

/*!
    Determines if \a other is of equal value to a video encoder settings object.

    Returns true if the settings objects are of equal value, and false if they
    are not of equal value.
*/
bool QVideoEncoderSettings::operator==(const QVideoEncoderSettings &other) const
{
    return (d == other.d) ||
           (d->isNull == other.d->isNull &&
            d->encodingMode == other.d->encodingMode &&
            d->bitrate == other.d->bitrate &&
            d->quality == other.d->quality &&
            d->codec == other.d->codec &&
            d->resolution == other.d->resolution &&
            qFuzzyCompare(d->frameRate, other.d->frameRate) &&
            d->encodingOptions == other.d->encodingOptions);
}

/*!
    Determines if \a other is of equal value to a video encoder settings object.

    Returns true if the settings objects are not of equal value, and false if
    they are of equal value.
*/
bool QVideoEncoderSettings::operator!=(const QVideoEncoderSettings &other) const
{
    return !(*this == other);
}

/*!
    Identifies if a video encoder settings object is uninitalized.

    Returns true if the settings are null, and false if they are not.
*/
bool QVideoEncoderSettings::isNull() const
{
    return d->isNull;
}

/*!
    Returns the video encoding mode.

    \sa QMultimedia::EncodingMode
*/
QMultimedia::EncodingMode QVideoEncoderSettings::encodingMode() const
{
    return d->encodingMode;
}

/*!
    Sets the video encoding \a mode.

    If QMultimedia::ConstantQualityEncoding is set,
    the quality encoding parameter is used and bit rate is ignored,
    otherwise the bitrate is used.

    The rest of encoding settings are respected regardless of encoding mode.

    \sa QMultimedia::EncodingMode
*/
void QVideoEncoderSettings::setEncodingMode(QMultimedia::EncodingMode mode)
{
    d->isNull = false;
    d->encodingMode = mode;
}

/*!
    Returns the video codec.
*/

QString QVideoEncoderSettings::codec() const
{
    return d->codec;
}

/*!
    Sets the video \a codec.
*/
void QVideoEncoderSettings::setCodec(const QString& codec)
{
    d->isNull = false;
    d->codec = codec;
}

/*!
    Returns bit rate of the encoded video stream in bits per second.
*/
int QVideoEncoderSettings::bitRate() const
{
    return d->bitrate;
}

/*!
    Sets the bit rate of the encoded video stream to \a value.
*/

void QVideoEncoderSettings::setBitRate(int value)
{
    d->isNull = false;
    d->bitrate = value;
}

/*!
    Returns the video frame rate.
*/
qreal QVideoEncoderSettings::frameRate() const
{
    return d->frameRate;
}

/*!
    \fn QVideoEncoderSettings::setFrameRate(qreal rate)

    Sets the video frame \a rate.

    A value of 0 indicates the encoder should make an optimal choice based on what is available
    from the video source and the limitations of the codec.
*/

void QVideoEncoderSettings::setFrameRate(qreal rate)
{
    d->isNull = false;
    d->frameRate = rate;
}

/*!
    Returns the resolution of the encoded video.
*/

QSize QVideoEncoderSettings::resolution() const
{
    return d->resolution;
}

/*!
    Sets the \a resolution of the encoded video.

    An empty QSize indicates the encoder should make an optimal choice based on
    what is available from the video source and the limitations of the codec.
*/

void QVideoEncoderSettings::setResolution(const QSize &resolution)
{
    d->isNull = false;
    d->resolution = resolution;
}

/*!
    Sets the \a width and \a height of the resolution of the encoded video.

    \overload
*/

void QVideoEncoderSettings::setResolution(int width, int height)
{
    d->isNull = false;
    d->resolution = QSize(width, height);
}

/*!
    Returns the video encoding quality.
*/

QMultimedia::EncodingQuality QVideoEncoderSettings::quality() const
{
    return d->quality;
}

/*!
    Sets the video encoding \a quality.

    Setting the video quality parameter allows backend to choose the balanced
    set of encoding parameters to achieve the desired quality level.

    The \a quality settings parameter is only used in the
    \l {QMultimedia::ConstantQualityEncoding}{constant quality} \l{encodingMode()}{encoding mode}.
    The \a quality settings parameter is only used in the \l
    {QMultimedia::ConstantQualityEncoding}{constant quality}
    \l{encodingMode()}{encoding mode}.
*/

void QVideoEncoderSettings::setQuality(QMultimedia::EncodingQuality quality)
{
    d->isNull = false;
    d->quality = quality;
}

/*!
    Returns the value of encoding \a option.

    \sa setEncodingOption(), encodingOptions()
*/
QVariant QVideoEncoderSettings::encodingOption(const QString &option) const
{
    return d->encodingOptions.value(option);
}

/*!
    Returns the all the encoding options as QVariantMap.

    \sa encodingOption(), setEncodingOptions()
*/
QVariantMap QVideoEncoderSettings::encodingOptions() const
{
    return d->encodingOptions;
}

/*!
    Set the encoding \a option \a value.

    The supported set and meaning of encoding options are
    system and selected codec specific.

    \sa encodingOption(), setEncodingOptions()
*/
void QVideoEncoderSettings::setEncodingOption(const QString &option, const QVariant &value)
{
    d->isNull = false;
    if (value.isNull())
        d->encodingOptions.remove(option);
    else
        d->encodingOptions.insert(option, value);
}

/*!
    Replace all the encoding options with \a options.

    The supported set and meaning of encoding options are
    system and selected codec specific.

    \sa encodingOption(), setEncodingOption()
*/
void QVideoEncoderSettings::setEncodingOptions(const QVariantMap &options)
{
    d->isNull = false;
    d->encodingOptions = options;
}


class QImageEncoderSettingsPrivate  : public QSharedData
{
public:
    bool isNull = true;
    QString codec;
    QSize resolution;
    QMultimedia::EncodingQuality quality = QMultimedia::NormalQuality;
    QVariantMap encodingOptions;
};

/*!
    \class QImageEncoderSettings


    \brief The QImageEncoderSettings class provides a set of image encoder
    settings.

    \inmodule QtMultimedia
    \ingroup multimedia
    \ingroup multimedia_camera

    A image encoder settings object is used to specify the image encoder
    settings used by QCameraImageCapture.  Image encoder settings are selected
    by constructing a QImageEncoderSettings object, setting the desired
    properties and then passing it to a QCameraImageCapture instance using the
    QCameraImageCapture::setImageSettings() function.

    \snippet multimedia-snippets/media.cpp Image encoder settings

    \sa QImageEncoderControl
*/

/*!
    Constructs a null image encoder settings object.
*/

QImageEncoderSettings::QImageEncoderSettings()
    :d(new QImageEncoderSettingsPrivate)
{
}

/*!
    Constructs a copy of the image encoder settings object \a other.
*/

QImageEncoderSettings::QImageEncoderSettings(const QImageEncoderSettings& other)
    :d(other.d)
{
}

/*!
    Destroys a image encoder settings object.
*/

QImageEncoderSettings::~QImageEncoderSettings()
{
}

/*!
    Assigns the value of \a other to a image encoder settings object.
*/
QImageEncoderSettings &QImageEncoderSettings::operator=(const QImageEncoderSettings &other)
{
    d = other.d;
    return *this;
}

/*!
    Determines if \a other is of equal value to a image encoder settings
    object.

    Returns true if the settings objects are of equal value, and false if they
    are not of equal value.
*/
bool QImageEncoderSettings::operator==(const QImageEncoderSettings &other) const
{
    return (d == other.d) ||
           (d->isNull == other.d->isNull &&
            d->quality == other.d->quality &&
            d->codec == other.d->codec &&
            d->resolution == other.d->resolution &&
            d->encodingOptions == other.d->encodingOptions);

}

/*!
    Determines if \a other is of equal value to a image encoder settings
    object.

    Returns true if the settings objects are not of equal value, and false if
    they are of equal value.
*/
bool QImageEncoderSettings::operator!=(const QImageEncoderSettings &other) const
{
    return !(*this == other);
}

/*!
    Identifies if a image encoder settings object is uninitalized.

    Returns true if the settings are null, and false if they are not.
*/
bool QImageEncoderSettings::isNull() const
{
    return d->isNull;
}

/*!
    Returns the image codec.
*/

QString QImageEncoderSettings::codec() const
{
    return d->codec;
}

/*!
    Sets the image \a codec.
*/
void QImageEncoderSettings::setCodec(const QString& codec)
{
    d->isNull = false;
    d->codec = codec;
}

/*!
    Returns the resolution of the encoded image.
*/

QSize QImageEncoderSettings::resolution() const
{
    return d->resolution;
}

/*!
    Sets the \a resolution of the encoded image.

    An empty QSize indicates the encoder should make an optimal choice based on
    what is available from the image source and the limitations of the codec.
*/

void QImageEncoderSettings::setResolution(const QSize &resolution)
{
    d->isNull = false;
    d->resolution = resolution;
}

/*!
    Sets the \a width and \a height of the resolution of the encoded image.

    \overload
*/

void QImageEncoderSettings::setResolution(int width, int height)
{
    d->isNull = false;
    d->resolution = QSize(width, height);
}

/*!
    Returns the image encoding quality.
*/

QMultimedia::EncodingQuality QImageEncoderSettings::quality() const
{
    return d->quality;
}

/*!
    Sets the image encoding \a quality.
*/

void QImageEncoderSettings::setQuality(QMultimedia::EncodingQuality quality)
{
    d->isNull = false;
    d->quality = quality;
}

/*!
    Returns the value of encoding \a option.

    \sa setEncodingOption(), encodingOptions()
*/
QVariant QImageEncoderSettings::encodingOption(const QString &option) const
{
    return d->encodingOptions.value(option);
}

/*!
    Returns the all the encoding options as QVariantMap.

    \sa encodingOption(), setEncodingOptions()
*/
QVariantMap QImageEncoderSettings::encodingOptions() const
{
    return d->encodingOptions;
}

/*!
    Set the encoding \a option \a value.

    The supported set and meaning of encoding options are
    system and selected codec specific.

    \sa encodingOption(), setEncodingOptions()
*/
void QImageEncoderSettings::setEncodingOption(const QString &option, const QVariant &value)
{
    d->isNull = false;
    if (value.isNull())
        d->encodingOptions.remove(option);
    else
        d->encodingOptions.insert(option, value);
}

/*!
    Replace all the encoding options with \a options.

    The supported set and meaning of encoding options are
    system and selected codec specific.

    \sa encodingOption(), setEncodingOption()
*/
void QImageEncoderSettings::setEncodingOptions(const QVariantMap &options)
{
    d->isNull = false;
    d->encodingOptions = options;
}


QT_END_NAMESPACE

