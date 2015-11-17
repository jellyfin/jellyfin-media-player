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

public:
  virtual bool componentExport() { return true; }
  virtual const char* componentName() { return "system"; }
  virtual bool componentInitialize() { return true; }
  virtual void componentPostInitialize();

  Q_INVOKABLE QVariantMap systemInformation() const;
  Q_INVOKABLE void exit();
  Q_INVOKABLE void restart();

  Q_INVOKABLE void info(QString text);

  Q_INVOKABLE void setCursorVisibility(bool visible);

  Q_INVOKABLE QString getUserAgent();

  Q_INVOKABLE QString debugInformation();

  Q_INVOKABLE QString logFilePath() { return Paths::logDir(Names::MainName() + ".log"); }

  Q_INVOKABLE QStringList networkAddresses() const;
  Q_INVOKABLE int networkPort() const;

  Q_INVOKABLE void userInformation(const QVariantMap& userModel);

  Q_INVOKABLE void openExternalUrl(const QString& url);

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

  inline bool isOpenELEC() { return m_platformType == platformTypeOpenELEC; }

  Q_INVOKABLE void crashApp();

private:
  SystemComponent(QObject* parent = 0);
  static QMap<QString, QString> networkInterfaces();

  QTimer* m_mouseOutTimer;
  PlatformType m_platformType;
  PlatformArch m_platformArch;
  QString m_overridePlatform;
  bool m_doLogMessages;

};

#endif
