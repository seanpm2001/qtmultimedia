/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <qmediaserviceproviderplugin.h>
#include <qmediaservice.h>
#include "../mockservice.h"

class MockServicePlugin4 : public QMediaServiceProviderPlugin,
                            public QMediaServiceSupportedFormatsInterface,
                            public QMediaServiceFeaturesInterface
{
    Q_OBJECT
    Q_INTERFACES(QMediaServiceSupportedFormatsInterface)
    Q_INTERFACES(QMediaServiceFeaturesInterface)
    Q_PLUGIN_METADATA(IID "org.qt-project.qt.mediaserviceproviderfactory/5.0" FILE "mockserviceplugin4.json")
public:
    [[nodiscard]] QStringList keys() const
    {
        return QStringList() << QLatin1String(Q_MEDIASERVICE_MEDIAPLAYER);
    }

    QMediaService* create(QString const& key) override
    {
        if (keys().contains(key))
            return new MockMediaService("MockServicePlugin4");
        else
            return nullptr;
    }

    void release(QMediaService *service) override
    {
        delete service;
    }

    [[nodiscard]] QMultimedia::SupportEstimate hasSupport(const QString &mimeType, const QStringList& codecs) const override
    {
        if (codecs.contains(QLatin1String("jpeg2000")))
            return QMultimedia::NotSupported;

        if (supportedMimeTypes().contains(mimeType))
            return QMultimedia::ProbablySupported;

        return QMultimedia::MaybeSupported;
    }

    [[nodiscard]] QStringList supportedMimeTypes() const override
    {
        return QStringList() << "video/mp4" << "video/quicktime";
    }

    [[nodiscard]] QMediaServiceFeaturesInterface::Features supportedFeatures(const QByteArray &service) const override
    {
        QMediaServiceFeaturesInterface::Features result;
        if (service == QByteArray(Q_MEDIASERVICE_MEDIAPLAYER))
            result |= QMediaServiceFeaturesInterface::StreamPlayback;
        return result;
    }
};

#include "mockserviceplugin4.moc"

