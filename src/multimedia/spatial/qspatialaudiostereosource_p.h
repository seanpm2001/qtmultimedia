/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Spatial Audio module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL-NOGPL2$
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
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QSPATIALAUDIOSTEREOSOURCE_P_H
#define QSPATIALAUDIOSTEREOSOURCE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qspatialaudiostereosource.h>
#include <qurl.h>
#include <qvector3d.h>
#include <qquaternion.h>
#include <qaudiobuffer.h>
#include <qaudiodevice.h>
#include <qmutex.h>

QT_BEGIN_NAMESPACE

class QSpatialAudioStereoSource;
class QAudioDecoder;
class QSpatialAudioEnginePrivate;

class QSpatialAudioStereoSourcePrivate
{
public:
    static QSpatialAudioStereoSourcePrivate *get(QSpatialAudioStereoSource *sound) { return sound ? sound->d : nullptr; }
    QUrl url;
    float volume = 1.;
    std::unique_ptr<QAudioDecoder> decoder;
    QSpatialAudioEngine *engine = nullptr;

    QMutex mutex;
    int currentBuffer = 0;
    int bufPos = 0;
    QList<QAudioBuffer> buffers;
    int sourceId = -1; // kInvalidSourceId

    void load(QSpatialAudioStereoSource *q);
    void getBuffer(QAudioBuffer::F32S *buf, int bufSize);
};

QT_END_NAMESPACE

#endif