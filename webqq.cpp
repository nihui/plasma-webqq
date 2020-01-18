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

#include "webqq.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QGraphicsLinearLayout>
#include <QWebFrame>
#include <QWebSettings>
#include <QX11Info>
#include <KActivities/Controller>
#include <KComponentData>
#include <KConfigDialog>
#include <KGraphicsWebView>
#include <KLocale>
#include <KNotification>
#include <KWindowSystem>
#include <Plasma/Context>
#include <Plasma/Corona>
#include <netwm.h>

#include <QTimer>

WebQQ::WebQQ(QObject* parent, const QVariantList& args)
: Plasma::Containment(parent, args)
{
    setDrawWallpaper(false);
    setHasConfigurationInterface(true);

    m_layout = new QGraphicsLinearLayout;
    m_layout->setContentsMargins(0, 0, 0, 0);
    setLayout(m_layout);

    m_view = new KGraphicsWebView;
    m_layout->addItem(m_view);

    m_notify = 0;
    m_showDesktop = false;

    QWebSettings* s = QWebSettings::globalSettings();
    s->setAttribute(QWebSettings::PluginsEnabled, true);
    s->setAttribute(QWebSettings::JavascriptCanAccessClipboard, true);

    connect(KWindowSystem::self(), SIGNAL(currentDesktopChanged(int)),
            this, SLOT(slotCurrentDesktopChanged(int)));
    connect(KWindowSystem::self(), SIGNAL(windowChanged(WId,unsigned int)),
            this, SLOT(slotWindowChanged(WId,unsigned int)));

//     connect(m_view->page(), SIGNAL(frameCreated(QWebFrame*)),
//             this, SLOT(slotFrameCreated(QWebFrame*)));
}

WebQQ::~WebQQ()
{
    if (m_notify) m_notify->close();
}

void WebQQ::createConfigurationInterface(KConfigDialog* parent)
{
    QWidget* cw = new QWidget;
    m_configUi.setupUi(cw);

    m_configUi.m_autoLogin->setChecked(m_autoLogin);
    m_configUi.m_accountEdit->setText(m_account);
    m_configUi.m_passwordEdit->setText(m_password);
    m_configUi.m_loginQQ->setChecked(m_loginQQ);
    m_configUi.m_invisibleMode->setChecked(m_invisibleMode);

    m_configUi.accountLabel->setEnabled(m_autoLogin);
    m_configUi.passwordLabel->setEnabled(m_autoLogin);
    m_configUi.m_accountEdit->setEnabled(m_autoLogin);
    m_configUi.m_passwordEdit->setEnabled(m_autoLogin);
    m_configUi.m_loginQQ->setEnabled(m_autoLogin);
    m_configUi.m_invisibleMode->setEnabled(m_autoLogin);

    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    connect(m_configUi.m_autoLogin, SIGNAL(clicked(bool)), parent, SLOT(settingsModified()));
    connect(m_configUi.m_accountEdit, SIGNAL(textChanged(QString)), parent, SLOT(settingsModified()));
    connect(m_configUi.m_passwordEdit, SIGNAL(textChanged(QString)), parent, SLOT(settingsModified()));
    connect(m_configUi.m_loginQQ, SIGNAL(clicked(bool)), parent, SLOT(settingsModified()));
    connect(m_configUi.m_invisibleMode, SIGNAL(clicked(bool)), parent, SLOT(settingsModified()));

    parent->addPage(cw, i18n("WebQQ"), "plasmawebqq");
}

void WebQQ::init()
{
    Plasma::Containment::init();
    setContainmentType(Plasma::Containment::DesktopContainment);

    m_autoLogin = config().readEntry("autoLogin", false);
    m_account = config().readEntry("account", QString());
    m_password = config().readEntry("password", QString());
    m_loginQQ = config().readEntry("loginQQ", false);
    m_invisibleMode = config().readEntry("invisibleMode", false);

    Plasma::Corona* c = corona();
    connect(c, SIGNAL(availableScreenRegionChanged()), this, SLOT(updateScreenRegion()));

    connect(m_view, SIGNAL(titleChanged(QString)), this, SLOT(slotTitleChanged(QString)));
    m_view->setUrl(QUrl("http://webqq.qq.com"));

    if (m_autoLogin && !m_account.isEmpty() && !m_password.isEmpty())
        QTimer::singleShot(5000, this, SLOT(slotShowLoginWindow()));
}

void WebQQ::configChanged()
{
    m_autoLogin = config().readEntry("autoLogin", false);
    m_account = config().readEntry("account", QString());
    m_password = config().readEntry("password", QString());
    m_loginQQ = config().readEntry("loginQQ", false);
    m_invisibleMode = config().readEntry("invisibleMode", false);
}

void WebQQ::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::ScreenConstraint) {
        updateScreenRegion();
    }
}

void WebQQ::configAccepted()
{
    config().writeEntry("autoLogin", m_configUi.m_autoLogin->isChecked());
    config().writeEntry("account", m_configUi.m_accountEdit->text());
    config().writeEntry("password", m_configUi.m_passwordEdit->text());
    config().writeEntry("loginQQ", m_configUi.m_loginQQ->isChecked());
    config().writeEntry("invisibleMode", m_configUi.m_invisibleMode->isChecked());
    emit configNeedsSaving();
}

void WebQQ::updateScreenRegion()
{
    Plasma::Corona* c = corona();
    QRect screenRect = c->screenGeometry(screen());
    QRect availRect;
    int maxArea = 0;
    foreach (const QRect& rect, c->availableScreenRegion(screen()).rects()) {
        int area = rect.width() * rect.height();
        if (area > maxArea) {
            availRect = rect;
            maxArea = area;
        }
    }

    m_layout->setContentsMargins(availRect.x() - screenRect.x(),
                                 availRect.y() - screenRect.y(),
                                 screenRect.right() - availRect.right(),
                                 screenRect.bottom() - availRect.bottom());
}

void WebQQ::slotShowLoginWindow()
{
    // open login box
    QString open_loginbox_js("qqweb.layout.showLoginWindow(); null");
    m_view->page()->mainFrame()->evaluateJavaScript(open_loginbox_js);

    QTimer::singleShot(2000, this, SLOT(slotLogin()));
}

void WebQQ::slotLogin()
{
    // set qq account
    QString set_qqaccount_js("document.getElementById(\"ifram_login\").contentWindow.document.getElementById('u').value = '%1'; null");
    set_qqaccount_js = set_qqaccount_js.arg(m_account);
    m_view->page()->mainFrame()->evaluateJavaScript(set_qqaccount_js);

    // set password
    QString set_password_js("document.getElementById('ifram_login').contentWindow.document.getElementById('p').value = '%1'; null");
    set_password_js = set_password_js.arg(m_password);
    m_view->page()->mainFrame()->evaluateJavaScript(set_password_js);

    if (m_loginQQ) {
        // set login qq
        QString set_loginqq_js("document.getElementById('ifram_login').contentWindow.document.getElementById('login2qq_checkbox').click(); null");
        m_view->page()->mainFrame()->evaluateJavaScript(set_loginqq_js);
    }

    if (m_invisibleMode) {
        // set invisible mode
        QString set_invisible_js("document.getElementById('ifram_login').contentWindow.onStateItemClick(40); null");
        m_view->page()->mainFrame()->evaluateJavaScript(set_invisible_js);
    }

    // click login
    QString click_login_js("document.getElementById('ifram_login').contentWindow.document.getElementById('login_button').click(); null");
    m_view->page()->mainFrame()->evaluateJavaScript(click_login_js);
}

void WebQQ::slotTitleChanged(const QString& title)
{
    // ignore default titles
    if (title.isEmpty() || title.startsWith("Q+ Web"))
        return;

    if (m_showDesktop) {
        // close existing notification
        if (m_notify) m_notify->close();
        return;
    }

    if (m_notify) {
        m_notify->setText(title);
        m_notify->update();
    }
    else {
        m_notify = new KNotification("message", KNotification::Persistent);
        m_notify->setComponentData(KComponentData("plasma_webqq"));
        m_notify->setText(title);
        m_notify->setActions(QStringList() << i18n("Chat"));
        connect(m_notify, SIGNAL(activated(unsigned int)), this, SLOT(slotNotificationActivated()));
        connect(m_notify, SIGNAL(ignored()), this, SLOT(slotNotificationIgnored()));
        connect(m_notify, SIGNAL(closed()), this, SLOT(slotNotificationClosed()));
        m_notify->sendEvent();
    }
}

void WebQQ::slotNotificationActivated()
{
    if (m_notify) m_notify->close();

    if (m_showDesktop)
        return;

    // switch activity
    KActivities::Controller c;
    c.setCurrentActivity(context()->currentActivityId());

    // find all windows can be minimized on current desktop and activity, and then minimize them
    QList<WId> iconifiedList;
    const QList<WId> windows = KWindowSystem::self()->windows();
    foreach (WId w, windows) {
        const unsigned long properties[2] = { NET::XAWMState | NET::WMDesktop, NET::WM2Activities };
        NETWinInfo info(QX11Info::display(), w, QX11Info::appRootWindow(), properties, 2);
        QStringList activities = QString(info.activities()).split(',');
        if (info.mappingState() == NET::Visible
            && (info.desktop() == NETWinInfo::OnAllDesktops
                || info.desktop() == (int)KWindowSystem::self()->currentDesktop())
            && activities.contains(context()->currentActivityId())) {
            iconifiedList.append(w);
        }
    }
    foreach (WId w, iconifiedList) {
        KWindowSystem::minimizeWindow(w, false);
    }
    m_showDesktop = true;
}

void WebQQ::slotNotificationIgnored()
{
    if (m_notify) m_notify->close();
}

void WebQQ::slotNotificationClosed()
{
    m_notify = 0;
}

void WebQQ::slotCurrentDesktopChanged(int)
{
    m_showDesktop = false;
}

void WebQQ::slotWindowChanged(WId w, unsigned int props)
{
    if (!m_showDesktop)
        return;

    if (props & NET::XAWMState) {
        NETWinInfo inf(QX11Info::display(), w, QX11Info::appRootWindow(),
                       NET::XAWMState | NET::WMWindowType);
        NET::WindowType windowType = inf.windowType(NET::AllTypesMask);
        if ((windowType == NET::Normal || windowType == NET::Unknown)
            && inf.mappingState() == NET::Visible) {
            m_showDesktop = false;
        }
    }
}
// #include <KDebug>
// void WebQQ::slotFrameCreated(QWebFrame* frame)
// {
//     kWarning() << frame;
//     connect(frame, SIGNAL(loadFinished(bool)), this, SLOT(aaa(bool)));
// }
// 
// void WebQQ::aaa(bool ok)
// {
//     QWebFrame* frame = static_cast<QWebFrame*>(sender());
//     kWarning() << frame << frame->title() << frame->url() << frame->toHtml();
// }

K_EXPORT_PLASMA_APPLET(webqq, WebQQ)
