//
// DeepLinkHandler.cpp - Handle custom URL scheme deeplinks for OIDC authentication
//

#include "DeepLinkHandler.h"
#include <QDebug>
#include <QUrlQuery>
#include <QDateTime>

///////////////////////////////////////////////////////////////////////////////////////////
DeepLinkHandler::DeepLinkHandler(QObject* parent) : QObject(parent)
{
  // Setup cleanup timer to remove expired states every minute
  m_cleanupTimer = new QTimer(this);
  connect(m_cleanupTimer, &QTimer::timeout, this, &DeepLinkHandler::clearExpiredStates);
  m_cleanupTimer->start(60000); // 1 minute
}

///////////////////////////////////////////////////////////////////////////////////////////
DeepLinkHandler::AuthCallbackResult DeepLinkHandler::handleDeepLink(const QString& url)
{
  qDebug() << "DeepLinkHandler: Processing URL:" << url;
  
  QUrl parsedUrl(url);
  if (!parsedUrl.isValid())
  {
    qWarning() << "DeepLinkHandler: Invalid URL format:" << url;
    return { AuthCallbackResult::InvalidUrl, "", "", "", "" };
  }

  if (!isValidAuthCallbackUrl(parsedUrl))
  {
    qWarning() << "DeepLinkHandler: URL not recognized as auth callback:" << url;
    return { AuthCallbackResult::InvalidUrl, "", "", "", "" };
  }

  return parseAuthCallback(parsedUrl);
}

///////////////////////////////////////////////////////////////////////////////////////////
void DeepLinkHandler::registerPendingState(const QString& state, int timeoutMinutes)
{
  qint64 expirationTime = QDateTime::currentMSecsSinceEpoch() + (timeoutMinutes * 60 * 1000);
  m_pendingStates[state] = expirationTime;
  qDebug() << "DeepLinkHandler: Registered pending state:" << state << "expires in" << timeoutMinutes << "minutes";
}

///////////////////////////////////////////////////////////////////////////////////////////
void DeepLinkHandler::clearExpiredStates()
{
  qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
  auto it = m_pendingStates.begin();
  
  while (it != m_pendingStates.end())
  {
    if (it.value() < currentTime)
    {
      qDebug() << "DeepLinkHandler: Removing expired state:" << it.key();
      it = m_pendingStates.erase(it);
    }
    else
    {
      ++it;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////
bool DeepLinkHandler::isValidAuthCallbackUrl(const QUrl& url)
{
  // Check scheme
  if (url.scheme() != "jellyfinmp")
  {
    return false;
  }

  QString host = url.host();
  QString path = url.path();

  // Accept these patterns:
  // 1. jellyfinmp://auth/callback (host=auth, path=/callback)
  // 2. jellyfinmp:///callback (host=empty, path=/callback) 
  // 3. jellyfinmp:/callback (host=empty, path=/callback)
  
  if (host == "auth" && path == "/callback")
  {
    return true; // Pattern 1
  }
  
  if (host.isEmpty() && path == "/callback")
  {
    return true; // Pattern 2 & 3
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////////////////
DeepLinkHandler::AuthCallbackResult DeepLinkHandler::parseAuthCallback(const QUrl& url)
{
  QUrlQuery query(url);
  
  QString state = query.queryItemValue("state");
  QString code = query.queryItemValue("code");
  QString error = query.queryItemValue("error");
  QString errorDescription = query.queryItemValue("error_description");

  // Validate state parameter is present
  if (state.isEmpty())
  {
    qWarning() << "DeepLinkHandler: Missing state parameter";
    return { AuthCallbackResult::InvalidUrl, "", "", "", "" };
  }

  // Check if state is registered and not expired
  if (!m_pendingStates.contains(state))
  {
    qWarning() << "DeepLinkHandler: Unknown or expired state:" << state;
    return { AuthCallbackResult::InvalidState, "", state, "", "" };
  }

  qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
  if (m_pendingStates[state] < currentTime)
  {
    qWarning() << "DeepLinkHandler: Expired state:" << state;
    m_pendingStates.remove(state);
    return { AuthCallbackResult::ExpiredState, "", state, "", "" };
  }

  // Remove the used state
  m_pendingStates.remove(state);

  // Check for error condition
  if (!error.isEmpty())
  {
    qDebug() << "DeepLinkHandler: Auth error received:" << error;
    return { AuthCallbackResult::Error, "", state, error, errorDescription };
  }

  // Check for success condition (code present)
  if (!code.isEmpty())
  {
    qDebug() << "DeepLinkHandler: Auth success received with code";
    return { AuthCallbackResult::Success, code, state, "", "" };
  }

  qWarning() << "DeepLinkHandler: Neither code nor error parameter found";
  return { AuthCallbackResult::InvalidUrl, "", state, "", "" };
}