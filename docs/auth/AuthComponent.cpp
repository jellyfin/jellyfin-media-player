//
// AuthComponent.cpp - Example integration with DeepLink system
// This is a template/example for integrating deeplinks with an auth component
//

#include "AuthComponent.h"
#include <QDesktopServices>
#include <QUrl>
#include <QUrlQuery>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QDebug>

///////////////////////////////////////////////////////////////////////////////////////////
AuthComponent::AuthComponent(QObject* parent) 
  : QObject(parent)
  , m_deepLinkHandler(new DeepLinkHandler(this))
  , m_networkManager(new QNetworkAccessManager(this))
{
}

///////////////////////////////////////////////////////////////////////////////////////////
void AuthComponent::initiateOIDCAuth(const QString& authUrl, const QString& redirectUri)
{
  // Generate PKCE parameters
  QByteArray codeVerifier = generateCodeVerifier();
  QByteArray codeChallenge = QCryptographicHash::hash(codeVerifier, QCryptographicHash::Sha256).toBase64(QByteArray::Base64UrlEncoding);
  
  // Generate state parameter for CSRF protection
  m_pendingState = generateRandomString(32);
  
  // Register state with deeplink handler
  m_deepLinkHandler->registerPendingState(m_pendingState, 10); // 10 minutes
  
  // Store auth parameters
  m_authUrl = authUrl;
  m_redirectUri = redirectUri;
  m_codeVerifier = codeVerifier;
  
  // Construct authorization URL
  QUrl url(authUrl);
  QUrlQuery query;
  query.addQueryItem("response_type", "code");
  query.addQueryItem("client_id", "jellyfin-media-player");
  query.addQueryItem("redirect_uri", redirectUri);
  query.addQueryItem("state", m_pendingState);
  query.addQueryItem("code_challenge", codeChallenge);
  query.addQueryItem("code_challenge_method", "S256");
  url.setQuery(query);
  
  qDebug() << "AuthComponent: Starting OIDC auth with URL:" << url.toString();
  
  // Open browser
  QDesktopServices::openUrl(url);
  
  emit authenticationStarted();
}

///////////////////////////////////////////////////////////////////////////////////////////
void AuthComponent::handleDeepLinkCallback(const DeepLinkHandler::AuthCallbackResult& result)
{
  qDebug() << "AuthComponent: Received deeplink callback, type:" << result.type;
  
  switch (result.type)
  {
    case DeepLinkHandler::AuthCallbackResult::Success:
      qDebug() << "AuthComponent: Auth success, exchanging code for tokens";
      exchangeCodeForTokens(result.code, result.state);
      break;
      
    case DeepLinkHandler::AuthCallbackResult::Error:
      qWarning() << "AuthComponent: Auth error:" << result.error << result.errorDescription;
      clearPendingAuth();
      emit authenticationFailed(result.error + ": " + result.errorDescription);
      break;
      
    case DeepLinkHandler::AuthCallbackResult::InvalidUrl:
      qWarning() << "AuthComponent: Invalid deeplink URL received";
      emit authenticationFailed("Invalid authentication callback URL");
      break;
      
    case DeepLinkHandler::AuthCallbackResult::InvalidState:
    case DeepLinkHandler::AuthCallbackResult::ExpiredState:
      qWarning() << "AuthComponent: Invalid or expired state in callback";
      emit authenticationFailed("Authentication session expired or invalid");
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////
void AuthComponent::exchangeCodeForTokens(const QString& code, const QString& state)
{
  // Verify state matches
  if (state != m_pendingState)
  {
    qWarning() << "AuthComponent: State mismatch in token exchange";
    emit authenticationFailed("Authentication state mismatch");
    return;
  }
  
  // Prepare token exchange request
  QNetworkRequest request;
  request.setUrl(QUrl(m_authUrl.replace("/authorize", "/token"))); // Simple URL transformation
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
  
  // Prepare POST data
  QUrlQuery postData;
  postData.addQueryItem("grant_type", "authorization_code");
  postData.addQueryItem("client_id", "jellyfin-media-player");
  postData.addQueryItem("code", code);
  postData.addQueryItem("redirect_uri", m_redirectUri);
  postData.addQueryItem("code_verifier", m_codeVerifier);
  
  QByteArray data = postData.toString().toUtf8();
  
  qDebug() << "AuthComponent: Exchanging authorization code for tokens";
  
  QNetworkReply* reply = m_networkManager->post(request, data);
  connect(reply, &QNetworkReply::finished, this, &AuthComponent::onTokenExchangeFinished);
}

///////////////////////////////////////////////////////////////////////////////////////////
void AuthComponent::onTokenExchangeFinished()
{
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply)
    return;
    
  reply->deleteLater();
  
  if (reply->error() != QNetworkReply::NoError)
  {
    qWarning() << "AuthComponent: Token exchange failed:" << reply->errorString();
    clearPendingAuth();
    emit authenticationFailed("Failed to exchange authorization code");
    return;
  }
  
  // Parse response
  QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
  QJsonObject obj = doc.object();
  
  if (obj.contains("access_token"))
  {
    QString accessToken = obj["access_token"].toString();
    qDebug() << "AuthComponent: Token exchange successful";
    clearPendingAuth();
    emit authenticationSucceeded(accessToken);
  }
  else
  {
    qWarning() << "AuthComponent: No access token in response";
    clearPendingAuth();
    emit authenticationFailed("Invalid token response");
  }
}

///////////////////////////////////////////////////////////////////////////////////////////
void AuthComponent::clearPendingAuth()
{
  m_pendingState.clear();
  m_authUrl.clear();
  m_redirectUri.clear();
  m_codeVerifier.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////
// Helper functions (these would typically be utility functions)
QByteArray generateCodeVerifier()
{
  const QString chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~";
  QByteArray result;
  result.reserve(128);
  
  for (int i = 0; i < 128; ++i)
  {
    result.append(chars.at(QRandomGenerator::global()->bounded(chars.length())));
  }
  
  return result;
}

QString generateRandomString(int length)
{
  const QString chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
  QString result;
  result.reserve(length);
  
  for (int i = 0; i < length; ++i)
  {
    result.append(chars.at(QRandomGenerator::global()->bounded(chars.length())));
  }
  
  return result;
}