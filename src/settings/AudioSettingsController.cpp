//
// Created by Tobias Hieta on 21/08/15.
//

#include "AudioSettingsController.h"
#include "SettingsComponent.h"
#include "SettingsSection.h"
#include "PlayerComponent.h"

/////////////////////////////////////////////////////////////////////////////////////////
AudioSettingsController::AudioSettingsController(QObject* parent) : QObject(parent)
{
  SettingsSection* audioSection = SettingsComponent::Get().getSection(SETTINGS_SECTION_AUDIO);
  connect(audioSection, &SettingsSection::valuesUpdated, this, &AudioSettingsController::valuesUpdated);
}

/////////////////////////////////////////////////////////////////////////////////////////
void AudioSettingsController::setHiddenPassthrough(const QStringList& codecs, bool hidden)
{
  SettingsSection* audioSection = SettingsComponent::Get().getSection(SETTINGS_SECTION_AUDIO);
  foreach(const QString& codec, codecs)
    audioSection->setValueHidden("passthrough." + codec, hidden);
}

/////////////////////////////////////////////////////////////////////////////////////////
void AudioSettingsController::valuesUpdated(const QVariantMap& values)
{
  bool advanced = SettingsComponent::Get().value(SETTINGS_SECTION_AUDIO, "advanced").toBool();
  SettingsSection* audioSection = SettingsComponent::Get().getSection(SETTINGS_SECTION_AUDIO);

  if (values.contains("devicetype"))
  {
    QString type = values.value("devicetype").toString();
    if (type == AUDIO_DEVICE_TYPE_BASIC)
    {
      setHiddenPassthrough(PlayerComponent::AudioCodecsAll(), true);
    }
    else if (type == AUDIO_DEVICE_TYPE_HDMI)
    {
      setHiddenPassthrough(PlayerComponent::AudioCodecsAll(), !advanced);
    }
    else if (type == AUDIO_DEVICE_TYPE_SPDIF)
    {
      setHiddenPassthrough(PlayerComponent::AudioCodecsAll(), true);
      setHiddenPassthrough(PlayerComponent::AudioCodecsSPDIF(), false);
    }

    emit settingsUpdated(SETTINGS_SECTION_AUDIO, audioSection->descriptions());
  }

  if (values.contains("advanced"))
  {
    advanced = values.value("advanced").toBool();
    if (audioSection->value("devicetype") == AUDIO_DEVICE_TYPE_HDMI)
      setHiddenPassthrough(PlayerComponent::AudioCodecsAll(), !advanced);

    audioSection->setValueHidden("exclusive", !advanced);

    emit settingsUpdated(SETTINGS_SECTION_AUDIO, audioSection->descriptions());
  }
}
