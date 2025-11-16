#include <QDebug>
#include "SettingsSection.h"

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
    qWarning() << QString("Trying to register %1.%2 multiple times").arg(m_sectionID).arg(value->key());
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

  // values not included in the map are "removed"
  for(const QString& key : m_values.keys())
  {
    if (!map.contains(key))
      resetValueNoNotify(key, updatedValues);
  }

  for(const QString& key : map.keys())
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
  {
    emit valuesUpdated(updatedValues);
    emit SettingsComponent::Get().sectionValueUpdate(m_sectionID, updatedValues);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QVariant SettingsSection::defaultValue(const QString& key)
{
  if (m_values.contains(key))
    return m_values[key]->defaultValue();

  qWarning() << "Looking for defaultValue:" << key << "in section:" << m_sectionID << "but it can't be found";
  return QVariant();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QVariant SettingsSection::value(const QString& key)
{
  if (m_values.contains(key))
    return m_values[key]->value();

  qWarning() << "Looking for value:" << key << "in section:" << m_sectionID << "but it can't be found";
  return QVariant();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SettingsSection::updatePossibleValues(const QString &key, const QVariantList &possibleValues)
{
  if (m_values.contains(key))
    m_values[key]->setPossibleValues(possibleValues);
  emit SettingsComponent::Get().groupUpdate(m_sectionID, descriptions());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QVariantList SettingsSection::possibleValues(const QString& key)
{
  if (m_values.contains(key))
    return m_values[key]->possibleValues();
  return QVariantList();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SettingsSection::setValue(const QString& key, const QVariant& value)
{
  if (key == "index")
    return false;

  QVariantMap values;
  // populate with default values (setValues() resets missing values)
  for(const QString& entry : m_values.keys())
    values[entry] = m_values[entry]->value();

  values[key] = value;

  setValues(values);
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SettingsSection::setValueNoSave(const QString& key, const QVariant& value)
{
  if (key == "index" || !m_values.contains(key))
    return;

  if (m_values[key]->value() == value)
    return;

  m_values[key]->setValue(value);

  QVariantMap updatedValues;
  updatedValues[key] = value;

  emit valuesUpdated(updatedValues);
  emit SettingsComponent::Get().sectionValueUpdate(m_sectionID, updatedValues);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SettingsSection::resetValueNoNotify(const QString& key, QVariantMap& updatedValues)
{
  if (!m_values.contains(key))
    return;

  SettingsValue* val = m_values[key];

  if (val->hasDescription())
  {
    if (val->value() == val->defaultValue())
      return;
    val->setValue(val->defaultValue());
    updatedValues[key] = val->value();
  }
  else
  {
    m_values[key]->deleteLater();
    m_values.remove(key);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SettingsSection::resetValue(const QString &key)
{
  QVariantMap updatedValues;

  resetValueNoNotify(key, updatedValues);

  if (updatedValues.size() > 0)
    emit valuesUpdated(updatedValues);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SettingsSection::resetValues()
{
  QVariantMap updatedValues;

  for (auto key : m_values.keys())
    resetValueNoNotify(key, updatedValues);

  if (updatedValues.size() > 0)
    emit valuesUpdated(updatedValues);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
const QVariantMap SettingsSection::allValues() const
{
  QVariantMap values;

  for(SettingsValue* val : m_values.values())
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
  for(SettingsValue* value : list)
  {
    if (!value->isHidden())
      settings.push_back(value->descriptions());
  }
  map.insert("settings", settings);
  return map;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
const QVariantMap SettingsSection::sectionOrder() const
{
  QVariantMap map;

  map.insert("key", m_sectionID);
  map.insert("order", m_orderIndex);

  return map;
}

/////////////////////////////////////////////////////////////////////////////////////////
bool SettingsSection::isHidden() const
{
  bool correctPlatform = ((m_platform & Utils::CurrentPlatform()) == Utils::CurrentPlatform());
  return (m_hidden || !correctPlatform);
}
