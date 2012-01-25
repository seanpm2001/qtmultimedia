/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QCAMERAIMAGEPROCESSING_H
#define QCAMERAIMAGEPROCESSING_H

#include <QtCore/qstringlist.h>
#include <QtCore/qpair.h>
#include <QtCore/qsize.h>
#include <QtCore/qpoint.h>
#include <QtCore/qrect.h>

#include <qmediacontrol.h>
#include <qmediaobject.h>
#include <qmediaservice.h>
#include <qmediaenumdebug.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Multimedia)


class QCamera;

class QCameraImageProcessingPrivate;
class Q_MULTIMEDIA_EXPORT QCameraImageProcessing : public QObject
{
    Q_OBJECT
    Q_ENUMS(WhiteBalanceMode)
public:
    enum WhiteBalanceMode {
        WhiteBalanceAuto = 0,
        WhiteBalanceManual = 1,
        WhiteBalanceSunlight = 2,
        WhiteBalanceCloudy = 3,
        WhiteBalanceShade = 4,
        WhiteBalanceTungsten = 5,
        WhiteBalanceFluorescent = 6,
        WhiteBalanceFlash = 7,
        WhiteBalanceSunset = 8,
        WhiteBalanceVendor = 1000
    };

    bool isAvailable() const;

    WhiteBalanceMode whiteBalanceMode() const;
    void setWhiteBalanceMode(WhiteBalanceMode mode);
    bool isWhiteBalanceModeSupported(WhiteBalanceMode mode) const;

    qreal manualWhiteBalance() const;
    void setManualWhiteBalance(qreal colorTemperature);

    qreal contrast() const;
    void setContrast(qreal value);

    qreal saturation() const;
    void setSaturation(qreal value);

    qreal sharpeningLevel() const;
    void setSharpeningLevel(qreal value);

    qreal denoisingLevel() const;
    void setDenoisingLevel(qreal value);

private:
    friend class QCamera;
    QCameraImageProcessing(QCamera *camera);
    ~QCameraImageProcessing();

    Q_DISABLE_COPY(QCameraImageProcessing)
    Q_DECLARE_PRIVATE(QCameraImageProcessing)
    QCameraImageProcessingPrivate *d_ptr;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QCameraImageProcessing::WhiteBalanceMode)

Q_MEDIA_ENUM_DEBUG(QCameraImageProcessing, WhiteBalanceMode)

QT_END_HEADER

#endif  // QCAMERAIMAGEPROCESSING_H
