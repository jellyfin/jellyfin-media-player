//
// DeepLinkHandler.h - Handle custom URL scheme deeplinks for OIDC authentication
//

#ifndef JELLYFIN_DEEPLINKHANDLER_H
#define JELLYFIN_DEEPLINKHANDLER_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QVariantMap>
#include <QTimer>
#include <QSet>

class DeepLinkHandler : public QObject
{
  Q_OBJECT

public:
  struct AuthCallbackResult
  {
    enum Type {
      Success,
      Error,
      InvalidUrl,
      InvalidState,
      ExpiredState
    };
    
    Type type;
    QString code;
    QString state;
    QString error;
    QString errorDescription;
  };

  explicit DeepLinkHandler(QObject* parent = nullptr);

  // Parse and validate a deeplink URL
  AuthCallbackResult handleDeepLink(const QString& url);
  
  // Register a pending auth state with timeout
  void registerPendingState(const QString& state, int timeoutMinutes = 5);
  
  // Clear expired states
  void clearExpiredStates();

Q_SIGNALS:
  void authCallbackReceived(const AuthCallbackResult& result);

private:
  bool isValidAuthCallbackUrl(const QUrl& url);
  AuthCallbackResult parseAuthCallback(const QUrl& url);
  
  // Map of state -> expiration timestamp
  QMap<QString, qint64> m_pendingStates;
  QTimer* m_cleanupTimer;
};

#endif // JELLYFIN_DEEPLINKHANDLER_H