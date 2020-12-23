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

class MockServicePlugin5 : public QMediaServiceProviderPlugin,
                           public QMediaServiceSupportedDevicesInterface
{
    Q_OBJECT
    Q_INTERFACES(QMediaServiceSupportedDevicesInterface)
    Q_PLUGIN_METADATA(IID "org.qt-project.qt.mediaserviceproviderfactory/5.0" FILE "mockserviceplugin5.json")
public:
    [[nodiscard]] QStringList keys() const
    {
        return QStringList() << QLatin1String(Q_MEDIASERVICE_CAMERA);
    }

    QMediaService* create(QString const& key) override
    {
        if (keys().contains(key))
            return new MockMediaService("MockServicePlugin5");
        else
            return nullptr;
    }

    void release(QMediaService *service) override
    {
        delete service;
    }

    [[nodiscard]] QByteArray defaultDevice(const QByteArray &service) const override
    {
        if (service == Q_MEDIASERVICE_CAMERA)
            return "backcamera";

        return QByteArray();
    }

    [[nodiscard]] QList<QByteArray> devices(const QByteArray &service) const override
    {
        QList<QByteArray> res;
        if (service == Q_MEDIASERVICE_CAMERA)
            res << "backcamera" << "somecamera";

        return res;
    }

    QString deviceDescription(const QByteArray &service, const QByteArray &device) override
    {
        if (devices(service).contains(device))
            return QString(device)+" description";
        else
            return QString();
    }
};

#include "mockserviceplugin5.moc"

