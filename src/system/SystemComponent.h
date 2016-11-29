#ifndef __SYSTEM_COMPONENT_H__
#define __SYSTEM_COMPONENT_H__

#include "ComponentManager.h"
#include <QTimer>
#include "utils/Utils.h"
#include "Paths.h"
#include "Names.h"

// System modifiers
#define SYSTEM_MODIFIER_OPENELEC "OpenELEC"

class SystemComponent : public ComponentBase
{
  Q_OBJECT
  DEFINE_SINGLETON(SystemComponent);

  Q_PROPERTY(bool isMacos READ platformIsMac CONSTANT)
  Q_PROPERTY(bool isWindows READ platformIsWindows CONSTANT)
  Q_PROPERTY(bool isLinux READ platformIsLinux CONSTANT)

public:
  bool componentExport() override { return true; }
  const char* componentName() override { return "system"; }
  bool componentInitialize() override;
  void componentPostInitialize() override;

  Q_INVOKABLE QVariantMap systemInformation() const;
  Q_INVOKABLE void exit();
  Q_INVOKABLE static void restart();

  Q_INVOKABLE void info(QString text);

  Q_INVOKABLE void setCursorVisibility(bool visible);

  Q_INVOKABLE QString getUserAgent();

  Q_INVOKABLE QString debugInformation();

  Q_INVOKABLE QStringList networkAddresses() const;
  Q_INVOKABLE int networkPort() const;

  Q_INVOKABLE void userInformation(const QVariantMap& userModel);

  Q_INVOKABLE void openExternalUrl(const QString& url);

  Q_INVOKABLE void runUserScript(QString script);

  // called by the web-client when everything is properly inited
  Q_INVOKABLE void hello(const QString& version);

  Q_INVOKABLE QString getCapabilitiesString();
  Q_SIGNAL void capabilitiesChanged(const QString& capabilities);
  Q_SIGNAL void userInfoChanged();

  // possible os types type enum
  enum PlatformType
  {
    platformTypeUnknown,
    platformTypeOsx,
    platformTypeWindows,
    platformTypeLinux,
    platformTypeOpenELEC
  };

  // possible values for target types
  enum PlatformArch
  {
    platformArchUnknown,
    platformArchX86_32,
    platformArchX86_64,
    platformArchRpi2
  };

  inline PlatformType getPlatformType() { return m_platformType; }
  inline PlatformArch getPlatformArch() { return m_platformArch; }

  QString getPlatformTypeString() const;
  QString getPlatformArchString() const;

  inline bool isOpenELEC() const { return m_platformType == platformTypeOpenELEC; }
  bool isWebClientConnected() const { return !m_webClientVersion.isEmpty(); }

  inline QString authenticationToken() { return m_authenticationToken; }

  Q_INVOKABLE void crashApp();

signals:
  void hostMessage(const QString& message);
  void settingsMessage(const QString& setting, const QString& value);
  void scaleChanged(qreal scale);

private:
  explicit SystemComponent(QObject* parent = nullptr);
  static QMap<QString, QString> networkInterfaces();

  bool platformIsWindows() const { return m_platformType == platformTypeWindows; }
  bool platformIsMac() const { return m_platformType == platformTypeOsx; }
  bool platformIsLinux() const { return m_platformType == platformTypeLinux; }

  QTimer* m_mouseOutTimer;
  PlatformType m_platformType;
  PlatformArch m_platformArch;
  bool m_doLogMessages;
  QString m_authenticationToken;
  QString m_webClientVersion;

};

#endif
