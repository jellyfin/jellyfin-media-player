#include "CefWebChannelTransport.h"

#include <QJsonDocument>
#include <QString>

CefWebChannelTransport::CefWebChannelTransport(CefRefPtr<CefBrowser> browser, QObject *parent)
    : QWebChannelAbstractTransport(parent), m_browser(browser) {}

void CefWebChannelTransport::sendMessage(const QJsonObject &message) {
    if (!m_browser) {
        qWarning() << "CefWebChannelTransport::sendMessage - no browser";
        return;
    }

    auto frame = m_browser->GetMainFrame();
    if (!frame) {
        qWarning() << "CefWebChannelTransport::sendMessage - no frame";
        return;
    }

    QByteArray jsonBytes = QJsonDocument(message).toJson(QJsonDocument::Compact);
    QString json = QString::fromUtf8(jsonBytes);

    // Escape for JS template literal: backticks, backslashes, and ${
    json.replace(QLatin1String("\\"), QLatin1String("\\\\"));
    json.replace(QLatin1String("`"), QLatin1String("\\`"));
    json.replace(QLatin1String("${"), QLatin1String("\\${"));

    QString js = QString("if(window.qt&&window.qt.webChannelTransport&&window.qt.webChannelTransport.onmessage){window.qt.webChannelTransport.onmessage({data:JSON.parse(`%1`)})}").arg(json);

    frame->ExecuteJavaScript(js.toStdString(), frame->GetURL(), 0);
}

void CefWebChannelTransport::onMessageFromJs(const QString &json) {
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &error);

    if (error.error != QJsonParseError::NoError) {
        qWarning() << "CefWebChannelTransport: JSON parse error:" << error.errorString();
        return;
    }

    if (!doc.isObject()) {
        qWarning() << "CefWebChannelTransport: expected JSON object";
        return;
    }

    emit messageReceived(doc.object(), this);
}
