// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "playbackengine/qffmpegvideorenderer_p.h"
#include "qffmpegvideobuffer_p.h"
#include "qvideosink.h"

QT_BEGIN_NAMESPACE

namespace QFFmpeg::PlaybackEngineInternal {

VideoRenderer::VideoRenderer(const TimeController &tc, QVideoSink *sink)
    : Renderer(tc), m_sink(sink)
{
}

VideoRenderer::RenderingResult VideoRenderer::renderInternal(Frame frame)
{
    if (!frame.isValid())
        return {};

    if (!m_sink)
        return {};

    //        qCDebug(qLcVideoRenderer) << "RHI:" << accel.isNull() << accel.rhi() << sink->rhi();

    // in practice this only happens with mediacodec
    if (frame.codec()->hwAccel() && !frame.avFrame()->hw_frames_ctx) {
        HWAccel *hwaccel = frame.codec()->hwAccel();
        AVFrame *avframe = frame.avFrame();
        if (!hwaccel->hwFramesContext())
            hwaccel->createFramesContext(AVPixelFormat(avframe->format),
                                         { avframe->width, avframe->height });

        if (hwaccel->hwFramesContext())
            avframe->hw_frames_ctx = av_buffer_ref(hwaccel->hwFramesContextAsBuffer());
    }

    auto buffer = std::make_unique<QFFmpegVideoBuffer>(frame.takeAVFrame());
    QVideoFrameFormat format(buffer->size(), buffer->pixelFormat());
    format.setColorSpace(buffer->colorSpace());
    format.setColorTransfer(buffer->colorTransfer());
    format.setColorRange(buffer->colorRange());
    format.setMaxLuminance(buffer->maxNits());
    QVideoFrame videoFrame(buffer.release(), format);
    videoFrame.setStartTime(frame.pts());
    videoFrame.setEndTime(frame.end());
    m_sink->setVideoFrame(videoFrame);

    return {};
}

}

QT_END_NAMESPACE