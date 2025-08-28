#ifndef DEEPLINKHANDLER_H
#define DEEPLINKHANDLER_H

#include <QObject>
#include <QUrl>
#include <QVariantMap>
#include "ComponentManager.h"

class DeepLinkHandler : public ComponentBase
{
  Q_OBJECT
  DEFINE_SINGLETON(DeepLinkHandler);

public:
  bool componentExport() override { return true; }
  const char* componentName() override { return "deeplink"; }
  bool componentInitialize() override;

  Q_INVOKABLE bool processDeepLink(const QString& url);
  Q_INVOKABLE QVariantMap parseDeepLink(const QString& url);
  Q_INVOKABLE bool isValidDeepLink(const QString& url) const;

  // Static method for processing URLs from command line
  static bool processDeepLinkUrl(const QString& url);

  // Supported URL schemes
  static const QStringList SUPPORTED_SCHEMES;

Q_SIGNALS:
  void playRequested(const QString& server, const QString& itemId, const QVariantMap& parameters);
  void connectRequested(const QString& server, const QString& token);
  void navigateRequested(const QString& path);
  void ssoRequested(const QString& server, const QString& returnUrl);

private:
  bool validateUrl(const QUrl& url) const;
  bool sanitizeParameters(QVariantMap& parameters) const;
  void handlePlayAction(const QVariantMap& params);
  void handleConnectAction(const QVariantMap& params);
  void handleNavigateAction(const QVariantMap& params);
  void handleSsoAction(const QVariantMap& params);
};

#endif // DEEPLINKHANDLER_H