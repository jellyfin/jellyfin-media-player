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

///////////////////////////////////////////////////////////////////////////////////////////////////
SettingsComponent::SettingsComponent(QObject *parent) : ComponentBase(parent), m_settingsVersion(-1)
{
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
    foreach (const QString& sname, m_sections.keys())
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
static QByteArray loadFile(const QString& filename)
{
  QLOG_DEBUG() << "Opening:" << filename;

  QFile file(filename);
  // Checking existence before opening is technically a race condition, but
  // it looks like Qt doesn't let us distinguish errors on opening.
  if (!file.exists())
    return "";
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    QLOG_ERROR() << "Could not open" << filename;
    return "";
  }
  return file.readAll();
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
  QJsonDocument json = QJsonDocument::fromJson(loadFile(filename));
  return json.object();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
static void writeJson(const QString& filename, const QJsonObject& data, bool pretty = true)
{
  QJsonDocument json(data);
  writeFile(filename, json.toJson(pretty ? QJsonDocument::Indented : QJsonDocument::Compact));
}

/////////////////////////////////////////////////////////////////////////////////////////
void SettingsComponent::load()
{
  loadConf(Paths::dataDir("plexmediaplayer.conf"), false);
  loadConf(Paths::dataDir("storage.json"), true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SettingsComponent::loadConf(const QString& path, bool storage)
{
  QJsonObject json = loadJson(path);

  int version = json["version"].toInt(0);
  if (version != m_settingsVersion)
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

  foreach (const QString& section, jsonSections.keys())
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

    foreach (const QString& setting, jsonSection.keys())
      sec->setValue(setting, jsonSection.value(setting).toVariant());
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SettingsComponent::saveSettings()
{
  QVariantMap sections;

  foreach(SettingsSection* section, m_sections.values())
  {
    if (!section->isStorage())
      sections.insert(section->sectionName(), section->allValues());
  }

  QJsonObject json;
  json.insert("sections", QJsonValue::fromVariant(sections));
  json.insert("version", m_settingsVersion);
  writeJson(Paths::dataDir("plexmediaplayer.conf"), json);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SettingsComponent::saveStorage()
{
  QVariantMap storage;

  foreach(SettingsSection* section, m_sections.values())
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
      section->removeValues();
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
    section->removeValues();
    saveSection(section);
  }
  else
  {
    // we want to remove a root key from webclient
    // which is stored in webclient section
    section = m_sections[SETTINGS_SECTION_WEBCLIENT];
    section->removeValue(sectionOrKey);
    saveSection(section);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SettingsComponent::resetToDefault()
{
  foreach(SettingsSection *section, m_sections)
  {
   section->resetToDefault();
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
struct section_order_index
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
  std::sort(sectionList.begin(), sectionList.end(), section_order_index());

  foreach(SettingsSection* section, sectionList)
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

  foreach(const QJsonValue& val, doc.array())
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

  SettingsSection* section = new SettingsSection(sectionName, (quint8)platformMask, m_sectionIndex ++, this);
  section->setHidden(sectionObject.value("hidden").toBool(false));
  section->setStorage(sectionObject.value("storage").toBool(false));

  auto values = sectionObject.value("values").toArray();
  int order = 0;
  foreach(auto val, values)
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
      foreach(const auto& v, defaults.toArray())
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
    setting->setHidden(valobj.value("hidden").toBool(false));
    setting->setIndexOrder(order ++);

    if (valobj.contains("possible_values") && valobj.value("possible_values").isArray())
    {
      auto list = valobj.value("possible_values").toArray();
      foreach(const auto& v, list)
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
      foreach(const QJsonValue& pl, platforms.toArray())
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
      foreach(const QJsonValue& pl, val.toArray())
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

  load();

  // add our AudioSettingsController that will inspect audio settings and react.
  // then run the signal the first time to make sure that we set the proper visibility
  // on the items from the start.
  //
  AudioSettingsController* ctrl = new AudioSettingsController(this);
  QVariantMap val;
  val.insert("devicetype", value(SETTINGS_SECTION_AUDIO, "devicetype"));
  val.insert("advanced", value(SETTINGS_SECTION_AUDIO, "advanced"));
  ctrl->valuesUpdated(val);
  connect(ctrl, &AudioSettingsController::settingsUpdated, this, &SettingsComponent::groupUpdate);

  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
void SettingsComponent::setUserRoleList(const QStringList& userRoles)
{
  QVariantList values;

  // stable is always available
  QVariantMap stable;
  stable.insert("value", 0);
  stable.insert("title", "Stable");

  values << stable;

  foreach(const QString& role, userRoles)
  {
    QVariantMap channel;
    int value = 0;
    QString title;

    if (role == "ninja")
    {
      value = 4;
      title = "Ninja";
    }
    else if (role == "plexpass")
    {
      value = 8;
      title = "PlexPass";
    }
    else if (role == "employee")
    {
      value = 2;
      title = "Employee";
    }

    channel.insert("value", value);
    channel.insert("title", title);

    values << channel;
  }

  updatePossibleValues(SETTINGS_SECTION_MAIN, "updateChannel", values);

}