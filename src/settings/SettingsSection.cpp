#include "SettingsSection.h"
#include "QsLog.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
SettingsSection::SettingsSection(const QString& sectionID, quint8 platforms, int _orderIndex, QObject* parent)
  : QObject(parent), m_sectionID(sectionID), m_orderIndex(_orderIndex), m_platform(platforms), m_hidden(false), m_storage(false)
{
  m_values.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////
void SettingsSection::registerSetting(SettingsValue* value)
{
  if (m_values.contains(value->key()))
  {
    QLOG_WARN() << QString("Trying to register %1.%2 multiple times").arg(m_sectionID).arg(value->key());
    return;
  }

  value->setParent(this);
  m_values[value->key()] = value;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SettingsSection::setValues(const QVariant& values)
{
  QVariantMap map = values.toMap();
  QVariantMap updatedValues;

  // values not included in the map are reset to default
  foreach (const QString& key, m_values.keys())
  {
    if (!map.contains(key))
      map[key] = m_values[key]->defaultValue();
  }

  foreach (const QString& key, map.keys())
  {
    if (key.isEmpty())
      continue;

    bool haveKey = m_values.contains(key);
    if (haveKey && m_values[key]->value() == map[key])
      continue;

    if (haveKey)
    {
      m_values[key]->setValue(map[key]);
    }
    else
    {
      m_values[key] = new SettingsValue(key, QVariant(), PLATFORM_ANY, this);
      m_values[key]->setValue(map[key]);
    }

    updatedValues.insert(key, map[key]);
  }

  if (updatedValues.size() > 0)
    emit valuesUpdated(updatedValues);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QVariant SettingsSection::value(const QString& key)
{
  if (m_values.contains(key))
    return m_values[key]->value();

  QLOG_WARN() << "Looking for value:" << key << "in section:" << m_sectionID << "but it can't be found";
  return QVariant();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SettingsSection::updatePossibleValues(const QString &key, const QVariantList &possibleValues)
{
  if (m_values.contains(key))
    m_values[key]->setPossibleValues(possibleValues);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SettingsSection::setValue(const QString& key, const QVariant& value)
{
  if (key == "index")
    return false;

  QVariantMap values;
  // populate with default values (setValues() resets missing values)
  foreach (const QString& entry, m_values.keys())
    values[entry] = m_values[entry]->value();

  values[key] = value;

  setValues(values);
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SettingsSection::removeValue(const QString &key)
{
  if (m_values.contains(key))
  {
    m_values[key]->deleteLater();
    m_values.remove(key);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SettingsSection::removeValues()
{
  m_values.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
const QVariantMap SettingsSection::allValues() const
{
  QVariantMap values;

  foreach (SettingsValue* val, m_values.values())
    values[val->key()] = val->value();

  return values;
}

/////////////////////////////////////////////////////////////////////////////////////////
struct ValueSortOrder
{
  inline bool operator()(SettingsValue* a, SettingsValue* b)
  {
    return a->indexOrder() < b->indexOrder();
  }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
const QVariantMap SettingsSection::descriptions() const
{
  QVariantMap map;

  map.insert("key", m_sectionID);

  QList<SettingsValue*> list = m_values.values();
  std::sort(list.begin(), list.end(), ValueSortOrder());

  QVariantList settings;
  foreach(SettingsValue* value, list)
  {
    if (!value->isHidden())
      settings.push_back(value->descriptions());
  }
  map.insert("settings", settings);
  return map;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SettingsSection::resetToDefault()
{
  foreach (const QString& key, m_values.keys())
    m_values[key]->reset();
}

/////////////////////////////////////////////////////////////////////////////////////////
bool SettingsSection::isHidden() const
{
  bool correctPlatform = ((m_platform & Utils::CurrentPlatform()) == Utils::CurrentPlatform());
  return (m_hidden || !correctPlatform);
}