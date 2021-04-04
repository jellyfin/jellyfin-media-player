#include "SettingsComponent.h"
#include "SettingsSection.h"
#include "Paths.h"
#include "utils/Utils.h"
#include "QsLog.h"
#include "AudioSettingsController.h"
#include "Names.h"

#include <QSaveFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QList>
#include <QSettings>
#include "input/InputComponent.h"
#include "system/SystemComponent.h"
#include "Version.h"

#define OLDEST_PREVIOUS_VERSION_KEY "oldestPreviousVersion"

///////////////////////////////////////////////////////////////////////////////////////////////////
SettingsComponent::SettingsComponent(QObject *parent) : ComponentBase(parent), m_settingsVersion(-1)
{
}

/////////////////////////////////////////////////////////////////////////////////////////
void SettingsComponent::componentPostInitialize()
{
  InputComponent::Get().registerHostCommand("cycle_setting", this, "cycleSettingCommand");
  InputComponent::Get().registerHostCommand("set_setting", this, "setSettingCommand");
}

/////////////////////////////////////////////////////////////////////////////////////////
void SettingsComponent::cycleSettingCommand(const QString& args)
{
  QString settingName = args;
  QStringList sub = settingName.split(".");
  if (sub.size() != 2)
  {
    QLOG_ERROR() << "Setting must be in the form section.name but got:" << settingName;
    return;
  }
  QString sectionID = sub[0];
  QString valueName = sub[1];
  SettingsSection* section = getSection(sectionID);
  if (!section)
  {
    QLOG_ERROR() << "Section" << sectionID << "is unknown";
    return;
  }
  QVariantList values = section->possibleValues(valueName);
  // If no possible values are defined, check the type of the default value.
  // In the case it's a boolean simply negate the current value to cycle through.
  // Otherwise log an error message, that it's not possible to cycle through the value.
  if (values.size() == 0)
  {
    if (section->defaultValue(valueName).type() == QVariant::Bool)
    {
      QVariant currentValue = section->value(valueName);
      auto nextValue = currentValue.toBool() ? false : true;
      setValue(sectionID, valueName, nextValue);
      QLOG_DEBUG() << "Setting" << settingName << "to " << (nextValue ? "Enabled" : "Disabled");
      emit SystemComponent::Get().settingsMessage(valueName, nextValue ? "Enabled" : "Disabled");
      return;
    }
    else
    {
      QLOG_ERROR() << "Setting" << settingName << "is unknown or is not cycleable.";
      return;
    }
  }
  QVariant currentValue = section->value(valueName);
  int nextValueIndex = 0;
  for (int n = 0; n < values.size(); n++)
  {
    if (currentValue == values[n].toMap()["value"])
    {
      nextValueIndex = n + 1;
      break;
    }
  }
  if (nextValueIndex >= values.size())
    nextValueIndex = 0;
  auto nextSetting = values[nextValueIndex].toMap();
  auto nextValue = nextSetting["value"];
  QLOG_DEBUG() << "Setting" << settingName << "to" << nextValue;
  setValue(sectionID, valueName, nextValue);
  emit SystemComponent::Get().settingsMessage(valueName, nextSetting["title"].toString());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SettingsComponent::setSettingCommand(const QString& args)
{
  int spaceIndex = args.indexOf(" ");
  if (spaceIndex < 0)
  {
    QLOG_ERROR() << "No value provided to settings set command.";
    return;
  }
  QString settingName = args.mid(0, spaceIndex);
  QString settingValue = args.mid(spaceIndex + 1);
  int subIndex = settingName.indexOf(".");
  if (subIndex < 0 || subIndex == args.size() - 1)
  {
    QLOG_ERROR() << "Setting must be in the form section.name but got:" << settingName;
    return;
  }
  QString sectionID = settingName.mid(0, subIndex);
  QString valueName = settingName.mid(subIndex + 1);
  SettingsSection* section = getSection(sectionID);
  if (!section)
  {
    QLOG_ERROR() << "Section" << sectionID << "is unknown";
    return;
  }
  QString jsonString = "{\"value\": " + settingValue + "}";
  QJsonParseError err;
  QVariant value = QJsonDocument::fromJson(jsonString.toUtf8(), &err).object()["value"].toVariant();
  printf("val: '%s'\n", settingValue.toUtf8().data());
  if (!value.isValid())
  {
    QLOG_ERROR() << "Invalid settings value:" << settingValue << "(if it's a string, make sure to quote it)";
    return;
  }
  QLOG_DEBUG() << "Setting" << settingName << "to" << value;
  setValue(sectionID, valueName, value);
  emit SystemComponent::Get().settingsMessage(valueName, value.toString());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SettingsComponent::updatePossibleValues(const QString &sectionID, const QString &key, const QVariantList &possibleValues)
{
  SettingsSection* section = getSection(sectionID);
  if (!section)
  {
    QLOG_ERROR() << "Section" << sectionID << "is unknown";
    return;
  }
  section->updatePossibleValues(key, possibleValues);
  QLOG_DEBUG() << "Updated possible values for:" << key << "to" << possibleValues;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QVariant SettingsComponent::allValues(const QString& section)
{
  if (section.isEmpty())
  {
    QVariantMap all;
    for(const QString& sname : m_sections.keys())
      all[sname] = m_sections[sname]->allValues();
    return all;
  }
  else if (m_sections.contains(section))
  {
    return m_sections[section]->allValues();
  }
  else
  {
    // look for the value in the webclient section
    return m_sections[SETTINGS_SECTION_WEBCLIENT]->value(section);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
static void writeFile(const QString& filename, const QByteArray& data)
{
  QSaveFile file(filename);
  file.open(QIODevice::WriteOnly | QIODevice::Text);
  file.write(data);
  if (!file.commit())
  {
    QLOG_ERROR() << "Could not write" << filename;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
static QJsonObject loadJson(const QString& filename)
{
  // Checking existence before opening is technically a race condition, but
  // it looks like Qt doesn't let us distinguish errors on opening.
  if (!QFile(filename).exists())
    return QJsonObject();

  QJsonParseError err;
  QJsonDocument json = Utils::OpenJsonDocument(filename, &err);
  if (json.isNull())
  {
    QLOG_ERROR() << "Could not open" << filename << "due to" << err.errorString();
  }
  return json.object();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
static void writeJson(const QString& filename, const QJsonObject& data, bool pretty = true)
{
  QJsonDocument json(data);
  writeFile(filename, json.toJson(pretty ? QJsonDocument::Indented : QJsonDocument::Compact));
}

/////////////////////////////////////////////////////////////////////////////////////////
QVariant SettingsComponent::readPreinitValue(const QString& sectionID, const QString& key)
{
  QJsonObject json = loadJson(Paths::dataDir("jellyfinmediaplayer.conf"));
  return json["sections"].toObject()[sectionID].toObject()[key].toVariant();
}

/////////////////////////////////////////////////////////////////////////////////////////
void SettingsComponent::load()
{
  loadConf(Paths::dataDir("jellyfinmediaplayer.conf"), false);
  loadConf(Paths::dataDir("storage.json"), true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SettingsComponent::loadConf(const QString& path, bool storage)
{
  bool migrateTvMode = false;
  QJsonObject json = loadJson(path);

  int version = json["version"].toInt(0);

  // Migrate to settings version 4
  if (version == 3 && m_settingsVersion == 4)
  {
    // We only need to make sure that old users have the
    // webmode set to "tv" so that we don't flip their
    // interface unnecessarly
    migrateTvMode = true;
  }
  else if (version != m_settingsVersion)
  {
    QString backup = path + ".broken";
    QFile::remove(backup);
    QFile::rename(path, backup);
    if (version == 0)
      QLOG_ERROR() << "Could not read config file.";
    else
      QLOG_ERROR() << "Config version is" << version << "but" << m_settingsVersion << "expected. Moving old config to" << backup;
    // Overwrite/create it with the defaults.
    if (storage)
      saveStorage();
    else
      saveSettings();
    return;
  }

  QJsonObject jsonSections = json["sections"].toObject();

  for(const QString& section : jsonSections.keys())
  {
    QJsonObject jsonSection = jsonSections[section].toObject();

    SettingsSection* sec = getSection(section);
    if (!sec && storage)
    {
      sec = new SettingsSection(section, PLATFORM_ANY, -1, this);
      sec->setHidden(true);
      sec->setStorage(true);
      m_sections.insert(section, sec);
    }
    else if (!sec)
    {
      QLOG_ERROR() << "Trying to load section:" << section << "from config file, but we don't want that.";
      continue;
    }

    for(const QString& setting : jsonSection.keys())
      sec->setValue(setting, jsonSection.value(setting).toVariant());
  }

  if (migrateTvMode)
    getSection(SETTINGS_SECTION_MAIN)->setValue("webMode", "tv");
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SettingsComponent::saveSettings()
{
  if (m_oldestPreviousVersion.isEmpty())
  {
    QLOG_ERROR() << "Not writing settings: uninitialized.\n";
    return;
  }

  QVariantMap sections;

  for(SettingsSection* section : m_sections.values())
  {
    if (!section->isStorage())
      sections.insert(section->sectionName(), section->allValues());
  }

  QJsonObject json;
  json.insert("sections", QJsonValue::fromVariant(sections));
  json.insert("version", m_settingsVersion);
  writeJson(Paths::dataDir("jellyfinmediaplayer.conf"), json);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SettingsComponent::saveStorage()
{
  QVariantMap storage;

  for(SettingsSection* section : m_sections.values())
  {
    if (section->isStorage())
      storage.insert(section->sectionName(), section->allValues());
  }

  QJsonObject storagejson;
  storagejson.insert("sections", QJsonValue::fromVariant(storage));
  storagejson.insert("version", m_settingsVersion);
  writeJson(Paths::dataDir("storage.json"), storagejson, false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SettingsComponent::saveSection(SettingsSection* section)
{
  if (section && section->isStorage())
    saveStorage();
  else
    saveSettings();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QVariant SettingsComponent::value(const QString& sectionID, const QString &key)
{
  SettingsSection* section = getSection(sectionID);
  if (!section)
  {
    QLOG_ERROR() << "Section" << sectionID << "is unknown";
    return QVariant();
  }
  return section->value(key);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SettingsComponent::setValue(const QString& sectionID, const QString &key, const QVariant &value)
{
  SettingsSection* section = getSection(sectionID);
  if (!section)
  {
    QLOG_ERROR() << "Section" << sectionID << "is unknown";
    return;
  }
  section->setValue(key, value);
  saveSection(section);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SettingsComponent::setValues(const QVariantMap& options)
{
  Q_ASSERT(options.contains("key"));
  Q_ASSERT(options.contains("value"));

  QString key = options["key"].toString();
  QVariant values = options["value"];

  if (values.type() == QVariant::Map || values.isNull())
  {
    SettingsSection* section = getSection(key);
    if (!section)
    {
      // let's create this section since it's most likely created by the webclient
      section = new SettingsSection(key, PLATFORM_ANY, -1, this);
      section->setHidden(true);
      section->setStorage(true);
      m_sections.insert(key, section);
    }

    if (values.isNull())
      section->resetValues();
    else
      section->setValues(values);

    saveSection(section);
  }
  else if (values.type() == QVariant::String)
  {
    setValue(SETTINGS_SECTION_WEBCLIENT, key, values);
  }
  else
  {
    QLOG_WARN() << "the values sent was not a map, string or empty value. it will be ignored";

    // return so we don't call save()
    return;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SettingsComponent::removeValue(const QString &sectionOrKey)
{
  SettingsSection* section = getSection(sectionOrKey);

  if (section)
  {
    // we want to remove a full section
    // dont remove the section, but remove all keys
    section->resetValues();
    saveSection(section);
  }
  else
  {
    // we want to remove a root key from webclient
    // which is stored in webclient section
    section = m_sections[SETTINGS_SECTION_WEBCLIENT];
    section->resetValue(sectionOrKey);
    saveSection(section);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SettingsComponent::resetToDefault(const QString &sectionID)
{
  SettingsSection* section = getSection(sectionID);

  if (section)
  {
    section->resetValues();
    saveSection(section);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SettingsComponent::resetToDefaultAll()
{
  for(SettingsSection *section : m_sections)
  {
    section->resetValues();
    saveSection(section);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
struct SectionOrderIndex
{
  inline bool operator ()(SettingsSection* a, SettingsSection* b)
  {
    return a->orderIndex() < b->orderIndex();
  }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
QVariantList SettingsComponent::settingDescriptions()
{
  QJsonArray desc;

  QList<SettingsSection*> sectionList = m_sections.values();
  std::sort(sectionList.begin(), sectionList.end(), SectionOrderIndex());

  for(SettingsSection* section : sectionList)
  {
    if (!section->isHidden())
      desc.push_back(QJsonValue::fromVariant(section->descriptions()));
  }

  return desc.toVariantList();
}

/////////////////////////////////////////////////////////////////////////////////////////
bool SettingsComponent::loadDescription()
{
  QJsonParseError err;
  auto doc = Utils::OpenJsonDocument(":/settings/settings_description.json", &err);
  if (doc.isNull())
  {
    QLOG_ERROR() << "Failed to read settings description:" << err.errorString();
    throw FatalException("Failed to read settings description!");
  }

  if (!doc.isArray())
  {
    QLOG_ERROR() << "The object needs to be an array";
    return false;
  }

  m_sectionIndex = 0;

  for(const QJsonValue& val : doc.array())
  {
    if (!val.isObject())
    {
      QLOG_ERROR() << "Hoped to find sections in the root array, but they where not JSON objects";
      return false;
    }

    QJsonObject section = val.toObject();
    if (!section.contains("section"))
    {
      QLOG_ERROR() << "A section needs to contain the section keyword.";
      return false;
    }

    if (section["section"] == "__meta__")
      m_settingsVersion = section.value("version").toInt();
    else
      parseSection(section);
  }

  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
void SettingsComponent::parseSection(const QJsonObject& sectionObject)
{
  QString sectionName = sectionObject.value("section").toString();
  if (!sectionObject.contains("values") || !sectionObject.value("values").isArray())
  {
    QLOG_ERROR() << "section object:" << sectionName << "did not contain a values array";
    return;
  }

  int platformMask = platformMaskFromObject(sectionObject);

  auto  section = new SettingsSection(sectionName, (quint8)platformMask, m_sectionIndex ++, this);
  section->setHidden(sectionObject.value("hidden").toBool(false));
  section->setStorage(sectionObject.value("storage").toBool(false));

  auto values = sectionObject.value("values").toArray();
  int order = 0;
  for(auto val : values)
  {
    if (!val.isObject())
      continue;

    QJsonObject valobj = val.toObject();
    if (!valobj.contains("value") || !valobj.contains("default") || valobj.value("value").isNull())
      continue;

    QJsonValue defaults = valobj.value("default");
    QVariant defaultval = defaults.toVariant();
    if (defaults.isArray())
    {
      defaultval = QVariant();
      // Whichever default matches the current platform first is used.
      for(const auto& v : defaults.toArray())
      {
        auto vobj = v.toObject();
        int defPlatformMask = platformMaskFromObject(vobj);
        if ((defPlatformMask & Utils::CurrentPlatform()) == Utils::CurrentPlatform())
        {
          defaultval = vobj.value("value").toVariant();
          break;
        }
      }
    }

    int vPlatformMask = platformMaskFromObject(valobj);
    SettingsValue* setting = new SettingsValue(valobj.value("value").toString(), defaultval, (quint8)vPlatformMask, this);
    setting->setHasDescription(true);
    setting->setHidden(valobj.value("hidden").toBool(false));
    setting->setIndexOrder(order ++);

    if (valobj.contains("input_type"))
      setting->setInputType(valobj.value("input_type").toString());

    if (valobj.contains("possible_values") && valobj.value("possible_values").isArray())
    {
      auto list = valobj.value("possible_values").toArray();
      for(const auto& v : list)
      {
        int platform = PLATFORM_ANY;

        auto vl = v.toArray();
        if (vl.size() < 2)
          continue;

        if (vl.size() == 3 && vl.at(2).isObject())
        {
          // platform options
          QJsonObject options = vl.at(2).toObject();
          platform = platformMaskFromObject(options);
        }

        if ((platform & Utils::CurrentPlatform()) == Utils::CurrentPlatform())
          setting->addPossibleValue(vl.at(1).toString(), vl.at(0).toVariant());
      }
    }

    section->registerSetting(setting);
  }

  m_sections.insert(sectionName, section);
}

/////////////////////////////////////////////////////////////////////////////////////////
int SettingsComponent::platformMaskFromObject(const QJsonObject& object)
{
  int platformMask = PLATFORM_ANY;

  // only include the listed platforms
  if (object.contains("platforms"))
  {
    // start by resetting it to no platforms
    platformMask = PLATFORM_UNKNOWN;

    QJsonValue platforms = object.value("platforms");

    // platforms can be both array or a single string
    if (platforms.isArray())
    {
      for(const QJsonValue& pl : platforms.toArray())
      {
        if (!pl.isString())
          continue;

        platformMask |= platformFromString(pl.toString());
      }
    }
    else
    {
      platformMask = platformFromString(platforms.toString());
    }
  }
  else if (object.contains("platforms_excluded"))
  {
    QJsonValue val = object.value("platforms_excluded");
    if (val.isArray())
    {
      for(const QJsonValue& pl : val.toArray())
      {
        if (!pl.isString())
          continue;

        platformMask &= ~platformFromString(pl.toString());
      }
    }
    else
    {
      platformMask &= ~platformFromString(val.toString());
    }
  }

  return platformMask;
}

/////////////////////////////////////////////////////////////////////////////////////////
Platform SettingsComponent::platformFromString(const QString& platformString)
{
  if (platformString == "osx")
    return PLATFORM_OSX;
  else if (platformString == "windows")
    return PLATFORM_WINDOWS;
  else if (platformString == "linux")
    return PLATFORM_LINUX;
  else if (platformString == "oe")
    return PLATFORM_OE;
  else if (platformString == "oe_rpi")
    return PLATFORM_OE_RPI;
  else if (platformString == "oe_x86")
    return PLATFORM_OE_X86;
  else if (platformString == "any")
    return PLATFORM_ANY;

  return PLATFORM_UNKNOWN;
}

/////////////////////////////////////////////////////////////////////////////////////////
bool SettingsComponent::componentInitialize()
{
  if (!loadDescription())
    return false;

  // Must be called before we possibly write the config file.
  setupVersion();

  load();

  // add our AudioSettingsController that will inspect audio settings and react.
  // then run the signal the first time to make sure that we set the proper visibility
  // on the items from the start.
  //
  auto  ctrl = new AudioSettingsController(this);
  QVariantMap val;
  val.insert("devicetype", value(SETTINGS_SECTION_AUDIO, "devicetype"));
  ctrl->valuesUpdated(val);
  connect(ctrl, &AudioSettingsController::settingsUpdated, this, &SettingsComponent::groupUpdate);

  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
void SettingsComponent::setupVersion()
{
  QSettings settings;
  m_oldestPreviousVersion = settings.value(OLDEST_PREVIOUS_VERSION_KEY).toString();
  if (m_oldestPreviousVersion.isEmpty())
  {
    // Version key was not present. It could still be a pre-1.1 PMP install,
    // so here we try to find out whether this is the very first install, or
    // if an older one exists.
    QFile configFile(Paths::dataDir("jellyfinmediaplayer.conf"));
    if (configFile.exists())
      m_oldestPreviousVersion = "legacy";
    else
      m_oldestPreviousVersion = Version::GetVersionString();
    settings.setValue(OLDEST_PREVIOUS_VERSION_KEY, m_oldestPreviousVersion);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
void SettingsComponent::setUserRoleList(const QStringList& userRoles)
{
  QVariantList values;

  // Channel names and values are aligned with values expected by plex.tv.
  // See: https://github.com/plexinc/plex-media-player-private/issues/642

  // Public is always available as the default value.
  QVariantMap publicChannel;
  publicChannel.insert("value", 0);
  publicChannel.insert("title", "Public");
  values << publicChannel;

  updatePossibleValues(SETTINGS_SECTION_MAIN, "updateChannel", values);
}

/////////////////////////////////////////////////////////////////////////////////////////
bool SettingsComponent::resetAndSaveOldConfiguration()
{
  QFile settingsFile(Paths::dataDir("jellyfinmediaplayer.conf"));
  return settingsFile.rename(Paths::dataDir("jellyfinmediaplayer.conf.old"));
}

/////////////////////////////////////////////////////////////////////////////////////////
QString SettingsComponent::getWebClientUrl(bool desktop)
{
  QString url;

#ifdef KONVERGO_OPENELEC
  desktop = false;
#endif

  if (desktop)
    url = SettingsComponent::Get().value(SETTINGS_SECTION_PATH, "startupurl_desktop").toString();
  else
    url = SettingsComponent::Get().value(SETTINGS_SECTION_PATH, "startupurl_tv").toString();

  // Transition to the new value so that old users are not screwed.
  if (url == "qrc:/konvergo/index.html")
  {
    SettingsComponent::Get().setValue(SETTINGS_SECTION_PATH, "startupurl", "bundled");
    url = "bundled";
  }

  if (url == "bundled")
  {
    auto path = Paths::webClientPath(desktop ? "desktop" : "tv");
    if (path.startsWith("/"))
      url = "file://" + path;
    url = "file:///" + path;
  }

  QLOG_DEBUG() << "Using web-client URL: " << url;

  return url;
}

/////////////////////////////////////////////////////////////////////////////////////////
void SettingsComponent::setCommandLineValues(const QStringList& values)
{
  QLOG_DEBUG() << values;

  QString mode = ""; // unset, different from "auto"

  for (const QString& value : values)
  {
    if (value == "fullscreen")
      setValue(SETTINGS_SECTION_MAIN, "fullscreen", true);
    else if (value == "windowed")
      setValue(SETTINGS_SECTION_MAIN, "fullscreen", false);
    else if (value == "desktop")
      mode = "desktop";
    else if (value == "tv")
      mode = "tv";
    else if (value == "auto-layout")
      mode = "auto";
  }

  setValue(SETTINGS_SECTION_MAIN, "layout", "auto");
  setValue(SETTINGS_SECTION_MAIN, "webMode", "desktop");
}

