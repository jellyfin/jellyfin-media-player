//
// Created by Tobias Hieta on 12/08/15.
//

#ifndef KONVERGO_SETTINGS_VALUE_H
#define KONVERGO_SETTINGS_VALUE_H

#include <QVariant>
#include <utils/Utils.h>

class SettingsValue : public QObject
{
  Q_OBJECT

public:
  SettingsValue(QObject* parent = 0)
    : QObject(parent)
    , m_platform(PLATFORM_UNKNOWN)
    , m_hidden(true)
  {}

  SettingsValue(const QString& _key, QVariant _defaultValue=QVariant(), quint8 platforms = PLATFORM_ANY, QObject* parent = 0)
    : QObject(parent)
    , m_key(_key)
    , m_value(QVariant())
    , m_defaultValue(_defaultValue)
    , m_platform(platforms)
    , m_hidden(true)
  {}

  const QString& key() const { return m_key; }

  const QVariant& value() const
  {
    if (!m_value.isValid())
      return m_defaultValue;
    return m_value;
  }

  void setValue(const QVariant& value)
  {
    m_value = value;
  }

  const QVariant& defaultValue() const
  {
    return m_defaultValue;
  }

  void setDefaultValue(const QVariant& defaultValue)
  {
    m_defaultValue = defaultValue;
  }

  const QVariantList& possibleValues() const
  {
    return m_possibleValues;
  }

  void setPossibleValues(const QVariantList& possibleValues)
  {
    m_possibleValues = possibleValues;
  }

  const quint8 platform() const
  {
    return m_platform;
  }

  void setPlatform(quint8 platform)
  {
    m_platform = platform;
  }

  void setHidden(bool hidden)
  {
    m_hidden = hidden;
  }

  bool isHidden() const
  {
    bool correctPlatform = ((m_platform & Utils::CurrentPlatform()) == Utils::CurrentPlatform());
    return (m_hidden || !correctPlatform);
  }

  void setInputType(const QString& inputType)
  {
    m_inputType = inputType;
  }

  void addPossibleValue(const QString& key, const QVariant& value)
  {
    QVariantMap entry;
    entry["value"] = value;
    entry["title"] = key;
    m_possibleValues << entry;
  }

  // goes back to use the default
  void reset()
  {
    m_value.clear();
  }

  QVariantMap descriptions() const
  {
    QVariantMap ret;
    ret.insert("key", m_key);

    if (!m_possibleValues.isEmpty())
      ret.insert("options", m_possibleValues);

    if (m_inputType.size())
      ret.insert("input_type", m_inputType);

    return ret;
  }

  void setIndexOrder(int order)
  {
    m_indexOrder = order;
  }

  int indexOrder() const { return m_indexOrder; }

private:
  QString m_key;
  QVariant m_value;
  QVariant m_defaultValue;
  QVariantList m_possibleValues;
  quint8 m_platform;
  bool m_hidden;
  QString m_inputType;

  int m_indexOrder;
};

#endif //KONVERGO_SETTINGS_VALUE_H

