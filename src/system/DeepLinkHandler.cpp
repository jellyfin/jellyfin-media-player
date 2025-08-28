#include "DeepLinkHandler.h"
#include <QUrl>
#include <QUrlQuery>
#include <QDebug>
#include <QRegularExpression>
#include "utils/Log.h"
#include "Globals.h"
#include "ui/KonvergoWindow.h"

const QStringList DeepLinkHandler::SUPPORTED_SCHEMES = { "jellyfin", "jmp" };

///////////////////////////////////////////////////////////////////////////////////////////////////
bool DeepLinkHandler::componentInitialize()
{
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool DeepLinkHandler::processDeepLinkUrl(const QString& url)
{
  DeepLinkHandler& instance = DeepLinkHandler::Get();
  return instance.processDeepLink(url);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool DeepLinkHandler::processDeepLink(const QString& url)
{
  qInfo() << "Processing deeplink URL:" << url;
  
  QVariantMap params = parseDeepLink(url);
  if (params.isEmpty())
  {
    qWarning() << "Failed to parse deeplink URL:" << url;
    return false;
  }

  QString action = params.value("action").toString();
  if (action == "play")
  {
    handlePlayAction(params);
  }
  else if (action == "connect")
  {
    handleConnectAction(params);
  }
  else if (action == "navigate")
  {
    handleNavigateAction(params);
  }
  else if (action == "sso")
  {
    handleSsoAction(params);
  }
  else
  {
    qWarning() << "Unknown deeplink action:" << action;
    return false;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QVariantMap DeepLinkHandler::parseDeepLink(const QString& url)
{
  QVariantMap result;
  
  if (!isValidDeepLink(url))
    return result;

  QUrl parsedUrl(url);
  if (!validateUrl(parsedUrl))
    return result;

  QString host = parsedUrl.host();
  if (host.isEmpty())
  {
    // Handle URLs like jellyfin:play?... (without //)
    QString path = parsedUrl.path();
    if (!path.isEmpty() && path.at(0) == ':')
      path = path.mid(1); // Remove leading ':'
    host = path;
  }

  result["action"] = host;
  result["scheme"] = parsedUrl.scheme();

  QUrlQuery query(parsedUrl);
  QVariantMap parameters;
  
  // Extract query parameters
  for (const auto& item : query.queryItems())
  {
    parameters[item.first] = item.second;
  }

  if (!sanitizeParameters(parameters))
  {
    qWarning() << "Failed to sanitize deeplink parameters";
    return QVariantMap();
  }

  result["parameters"] = parameters;
  return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool DeepLinkHandler::isValidDeepLink(const QString& url) const
{
  QUrl parsedUrl(url);
  QString scheme = parsedUrl.scheme().toLower();
  return SUPPORTED_SCHEMES.contains(scheme);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool DeepLinkHandler::validateUrl(const QUrl& url) const
{
  if (!url.isValid())
    return false;

  QString scheme = url.scheme().toLower();
  if (!SUPPORTED_SCHEMES.contains(scheme))
    return false;

  // Additional validation for suspicious patterns
  QString urlString = url.toString();
  
  // Prevent javascript: or data: schemes in parameters
  if (urlString.contains("javascript:", Qt::CaseInsensitive) ||
      urlString.contains("data:", Qt::CaseInsensitive))
  {
    qWarning() << "Potentially dangerous URL detected:" << urlString;
    return false;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool DeepLinkHandler::sanitizeParameters(QVariantMap& parameters) const
{
  // Remove potentially dangerous parameters
  QStringList dangerousKeys;
  for (auto it = parameters.begin(); it != parameters.end(); ++it)
  {
    QString key = it.key();
    QString value = it.value().toString();
    
    // Check for suspicious content
    if (value.contains("javascript:", Qt::CaseInsensitive) ||
        value.contains("data:", Qt::CaseInsensitive) ||
        value.contains("<script", Qt::CaseInsensitive) ||
        value.length() > 2048) // Prevent extremely long parameters
    {
      dangerousKeys.append(key);
    }
  }
  
  for (const QString& key : dangerousKeys)
  {
    qWarning() << "Removing dangerous parameter:" << key;
    parameters.remove(key);
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DeepLinkHandler::handlePlayAction(const QVariantMap& params)
{
  QVariantMap parameters = params.value("parameters").toMap();
  QString server = parameters.value("server").toString();
  QString itemId = parameters.value("item").toString();

  if (server.isEmpty() || itemId.isEmpty())
  {
    qWarning() << "Play action requires server and item parameters";
    return;
  }

  qInfo() << "Handling play action - Server:" << server << "Item:" << itemId;
  emit playRequested(server, itemId, parameters);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DeepLinkHandler::handleConnectAction(const QVariantMap& params)
{
  QVariantMap parameters = params.value("parameters").toMap();
  QString server = parameters.value("server").toString();
  QString token = parameters.value("token").toString();

  if (server.isEmpty())
  {
    qWarning() << "Connect action requires server parameter";
    return;
  }

  qInfo() << "Handling connect action - Server:" << server;
  emit connectRequested(server, token);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DeepLinkHandler::handleNavigateAction(const QVariantMap& params)
{
  QVariantMap parameters = params.value("parameters").toMap();
  QString path = parameters.value("path").toString();

  if (path.isEmpty())
  {
    qWarning() << "Navigate action requires path parameter";
    return;
  }

  qInfo() << "Handling navigate action - Path:" << path;
  emit navigateRequested(path);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DeepLinkHandler::handleSsoAction(const QVariantMap& params)
{
  QVariantMap parameters = params.value("parameters").toMap();
  QString server = parameters.value("server").toString();
  QString returnUrl = parameters.value("return_url").toString();

  if (server.isEmpty())
  {
    qWarning() << "SSO action requires server parameter";
    return;
  }

  qInfo() << "Handling SSO action - Server:" << server << "Return URL:" << returnUrl;
  emit ssoRequested(server, returnUrl);
}