#pragma once

#include "include/cef_browser.h"
#include <QWebChannelAbstractTransport>
#include <QJsonObject>

class CefWebChannelTransport : public QWebChannelAbstractTransport {
    Q_OBJECT
public:
    explicit CefWebChannelTransport(CefRefPtr<CefBrowser> browser, QObject *parent = nullptr);

    // C++ → JS: inject message into page
    void sendMessage(const QJsonObject &message) override;

    // Called by CEF message router when JS sends a message
    void onMessageFromJs(const QString &json);

private:
    CefRefPtr<CefBrowser> m_browser;
};
