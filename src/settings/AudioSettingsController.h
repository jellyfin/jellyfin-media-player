//
// Created by Tobias Hieta on 21/08/15.
//

#ifndef KONVERGO_AUDIOSETTINGSCONTROLLER_H
#define KONVERGO_AUDIOSETTINGSCONTROLLER_H

#include <QObject>

class AudioSettingsController : public QObject
{
  Q_OBJECT
public:
  AudioSettingsController(QObject* parent = 0);
  Q_SLOT void valuesUpdated(const QVariantMap& values);
  Q_SIGNAL void settingsUpdated(const QString& section, const QVariant& description);

private:
  void setHiddenPassthrough(const QStringList& codecs, bool hidden);
};

#endif //KONVERGO_AUDIOSETTINGSCONTROLLER_H
