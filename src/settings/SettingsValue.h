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
  explicit SettingsValue(QObject* parent = nullptr)
    : QObject(parent)
    , m_platform(PLATFORM_UNKNOWN)
    , m_hidden(true)
    , m_indexOrder(0)
    , m_hasDescription(false)
  {}

  explicit SettingsValue(const QString& _key, QVariant _defaultValue=QVariant(), quint8 platforms = PLATFORM_ANY, QObject* parent = nullptr)
    : QObject(parent)
    , m_key(_key)
    , m_value(QVariant())
    , m_defaultValue(_defaultValue)
    , m_platform(platforms)
    , m_hidden(true)
    , m_indexOrder(0)
    , m_hasDescription(false)
  {}

  const QString& key() const { return m_key; }

  const QString& displayName() const { return m_displayName; }

  void setDisplayName(const QString& displayName)
  {
    m_displayName = displayName;
  }

  const QString& help() const { return m_help; }

  void setHelp(const QString& help)
  {
    m_help = help;
  }

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

  quint8 platform() const
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

  QVariantMap descriptions() const
  {
    QVariantMap ret;
    ret.insert("key", m_key);
    ret.insert("displayName", m_displayName);
    ret.insert("help", m_help);

    if (!m_possibleValues.isEmpty())
      ret.insert("options", m_possibleValues);

    if (m_inputType.size())
      ret.insert("inputType", m_inputType);

    return ret;
  }

  void setIndexOrder(int order)
  {
    m_indexOrder = order;
  }

  int indexOrder() const { return m_indexOrder; }

  void setHasDescription(bool hasDescription)
  {
    m_hasDescription = hasDescription;
  }

  bool hasDescription() { return m_hasDescription; }

private:
  QString m_key;
  QString m_displayName;
  QString m_help;
  QVariant m_value;
  QVariant m_defaultValue;
  QVariantList m_possibleValues;
  quint8 m_platform;
  bool m_hidden;
  QString m_inputType;

  int m_indexOrder;
  bool m_hasDescription;
};

#endif //KONVERGO_SETTINGS_VALUE_H

