/*
 *  This file is part of plasma-webqq, KDE Plasma Web QQ intergration
 *  Copyright (C) 2012 Ni Hui <shuizhuyuanluo@126.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License or (at your option) version 3 or any later version
 *  accepted by the membership of KDE e.V. (or its successor approved
 *  by the membership of KDE e.V.), which shall act as a proxy
 *  defined in Section 14 of version 3 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef WEBQQ_H
#define WEBQQ_H

#include <Plasma/Containment>
#include "ui_webqqconfig.h"

class QGraphicsLinearLayout;
class QWebFrame;
class KGraphicsWebView;
class KNotification;
class WebQQ : public Plasma::Containment
{
    Q_OBJECT
public:
    WebQQ(QObject* parent, const QVariantList& args);
    virtual ~WebQQ();
    virtual void createConfigurationInterface(KConfigDialog* parent);
public Q_SLOTS:
    virtual void init();
    virtual void configChanged();
protected:
    virtual void constraintsEvent(Plasma::Constraints constraints);
private Q_SLOTS:
    void configAccepted();
    void updateScreenRegion();
    void slotShowLoginWindow();
    void slotLogin();
    void slotTitleChanged(const QString& title);
    void slotNotificationActivated();
    void slotNotificationIgnored();
    void slotNotificationClosed();
    void slotCurrentDesktopChanged(int);
    void slotWindowChanged(WId w, unsigned int props);

//     void slotFrameCreated(QWebFrame* frame);
//     void aaa(bool);
private:
    QGraphicsLinearLayout* m_layout;
    KGraphicsWebView* m_view;
    KNotification* m_notify;
    bool m_showDesktop;
    Ui::WebQQConfig m_configUi;
    bool m_autoLogin;
    QString m_account;
    QString m_password;
    bool m_loginQQ;
    bool m_invisibleMode;
};

#endif // WEBQQ_H
