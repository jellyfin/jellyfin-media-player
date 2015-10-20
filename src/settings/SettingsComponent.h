#ifndef SETTINGSCOMPONENT_H
#define SETTINGSCOMPONENT_H

#include <QObject>
#include "utils/Utils.h"
#include "ComponentManager.h"
#include "SettingsValue.h"

#define SETTINGS_SECTION_AUDIO "audio"
#define SETTINGS_SECTION_VIDEO "video"
#define SETTINGS_SECTION_MAIN "main"
#define SETTINGS_SECTION_STATE "state"
#define SETTINGS_SECTION_PATH "path"
#define SETTINGS_SECTION_WEBCLIENT "webclient"
#define SETTINGS_SECTION_SUBTITLES "subtitles"
#define SETTINGS_SECTION_OVERRIDES "overrides"
#define SETTINGS_SECTION_CEC "cec"
#define SETTINGS_SECTION_OPENELEC "openelec"

#define AUDIO_DEVICE_TYPE_BASIC "basic"
#define AUDIO_DEVICE_TYPE_SPDIF "spdif"
#define AUDIO_DEVICE_TYPE_HDMI "hdmi"


class SettingsSection;

///////////////////////////////////////////////////////////////////////////////////////////////////
class SettingsComponent : public ComponentBase
{
  Q_OBJECT
  DEFINE_SINGLETON(SettingsComponent);

public:
  bool componentInitialize();

  const char* componentName() { return "settings"; }
  bool componentExport() { return true; }

  SettingsSection* getSection(const QString& sectionID)
  {
    if (m_sections.contains(sectionID))
      return m_sections.value(sectionID);
    else
      return nullptr;
  }

  // JS interface
  Q_INVOKABLE void setValue(const QString& sectionID, const QString& key, const QVariant& value);
  Q_INVOKABLE void setValues(const QVariantMap& options);
  Q_INVOKABLE QVariant value(const QString& sectionID, const QString& key);
  Q_INVOKABLE QVariant allValues(const QString& section = "");
  Q_INVOKABLE void removeValue(const QString& sectionOrKey);
  Q_INVOKABLE void resetToDefault();
  Q_INVOKABLE QVariantList settingDescriptions();

  void updatePossibleValues(const QString& sectionID, const QString& key, const QVariantList& possibleValues);

  void saveSettings();
  void saveStorage();
  void load();

  Q_SIGNAL void groupUpdate(const QString& section, const QVariant& description);

  void setUserRoleList(const QStringList& userRoles);

private:
  explicit SettingsComponent(QObject *parent = 0);
  bool loadDescription();
  void parseSection(const QJsonObject& sectionObject);
  int platformMaskFromObject(const QJsonObject& object);
  Platform platformFromString(const QString& platformString);
  void saveSection(SettingsSection* section);

  QMap<QString, SettingsSection*> m_sections;

  int m_settingsVersion;
  int m_sectionIndex;

  void loadConf(const QString& path, bool storage);
};

#endif // SETTINGSCOMPONENT_H
