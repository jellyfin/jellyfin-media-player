//
// AuthComponent.h - Example integration with DeepLink system
// This is a template/example for integrating deeplinks with an auth component
//

#ifndef JELLYFIN_AUTHCOMPONENT_H
#define JELLYFIN_AUTHCOMPONENT_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "shared/DeepLinkHandler.h"

class AuthComponent : public QObject
{
  Q_OBJECT

public:
  explicit AuthComponent(QObject* parent = nullptr);
  
  // Start OIDC flow
  void initiateOIDCAuth(const QString& authUrl, const QString& redirectUri);
  
  // Handle deeplink callback
  void handleDeepLinkCallback(const DeepLinkHandler::AuthCallbackResult& result);

Q_SIGNALS:
  void authenticationSucceeded(const QString& accessToken);
  void authenticationFailed(const QString& error);
  void authenticationStarted();

private Q_SLOTS:
  void onTokenExchangeFinished();

private:
  void exchangeCodeForTokens(const QString& code, const QString& state);
  void clearPendingAuth();

  DeepLinkHandler* m_deepLinkHandler;
  QNetworkAccessManager* m_networkManager;
  
  // Pending auth state
  QString m_pendingState;
  QString m_authUrl;
  QString m_redirectUri;
  QString m_codeVerifier; // For PKCE
};

#endif // JELLYFIN_AUTHCOMPONENT_H