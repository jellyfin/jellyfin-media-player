#ifndef SETTINGSSECTION_H
#define SETTINGSSECTION_H

#include <QObject>
#include <QMap>
#include <QVariant>
#include "SettingsValue.h"
#include "SettingsComponent.h"
#include "QsLog.h"

class SettingsSection : public QObject
{
  Q_OBJECT
public:
  explicit SettingsSection(const QString& sectionID, quint8 platforms = PLATFORM_ANY,
                           int _orderIndex = -1, QObject* parent = nullptr);

  void updatePossibleValues(const QString& key, const QVariantList& possibleValues);
  QVariantList possibleValues(const QString& key);

  void setValues(const QVariant& values);
  bool setValue(const QString& key, const QVariant& value);
  void resetValue(const QString& key);
  void resetValues();
  void registerSetting(SettingsValue* value);
  bool isHidden() const;

  QVariant value(const QString& key);
  QVariant defaultValue(const QString& key);
  QString sectionName() const { return m_sectionID; }

  const QVariantMap allValues() const;
  const QVariantMap descriptions() const;

  bool isValueHidden(const QString& key) const { return m_values[key]->isHidden(); }
  int orderIndex() const { return m_orderIndex; }

  void setHidden(bool hidden=true)
  {
    m_hidden = hidden;
  }

  void setValueHidden(const QString& value, bool hidden)
  {
    if (m_values.contains(value))
      m_values.value(value)->setHidden(hidden);
  }

  void setStorage(bool storage) { m_storage = storage; }
  bool isStorage() const
  {
    return m_storage;
  }

  Q_SIGNAL void valuesUpdated(const QVariantMap& values);

protected:
  // if the value is _not_ removed, _and_ changes, it's added to updatedValues
  void resetValueNoNotify(const QString& key, QVariantMap& updatedValues);

  QHash<QString, SettingsValue*> m_values;
  QString m_sectionID;
  int m_orderIndex;
  quint8 m_platform;
  bool m_hidden;
  bool m_storage;
};

#endif // SETTINGSSECTION_H
