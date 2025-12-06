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
  if (auto* audioSection = SettingsComponent::Get().getSection(SETTINGS_SECTION_AUDIO))
    connect(audioSection, &SettingsSection::valuesUpdated, this, &AudioSettingsController::valuesUpdated);
}

/////////////////////////////////////////////////////////////////////////////////////////
void AudioSettingsController::setHiddenPassthrough(const QStringList& codecs, bool hidden)
{
  SettingsSection* audioSection = SettingsComponent::Get().getSection(SETTINGS_SECTION_AUDIO);
  for(const QString& codec : codecs)
    audioSection->setValueHidden("passthrough." + codec, hidden);
}

/////////////////////////////////////////////////////////////////////////////////////////
void AudioSettingsController::valuesUpdated(const QVariantMap& values)
{
  (void)values;
  SettingsSection* audioSection = SettingsComponent::Get().getSection(SETTINGS_SECTION_AUDIO);
  auto prevDescriptions = audioSection->descriptions();

  QString type = SettingsComponent::Get().value(SETTINGS_SECTION_AUDIO, "devicetype").toString();

  audioSection->setValueHidden("channels", false);

  if (type == AUDIO_DEVICE_TYPE_BASIC)
  {
    setHiddenPassthrough(PlayerComponent::AudioCodecsAll(), true);
  }
  else if (type == AUDIO_DEVICE_TYPE_HDMI)
  {
    setHiddenPassthrough(PlayerComponent::AudioCodecsAll(), false);
  }
  else if (type == AUDIO_DEVICE_TYPE_SPDIF)
  {
    setHiddenPassthrough(PlayerComponent::AudioCodecsAll(), true);
    setHiddenPassthrough(PlayerComponent::AudioCodecsSPDIF(), false);
    audioSection->setValueHidden("channels", true);
  }

  auto newDescriptions = audioSection->descriptions();
  if (prevDescriptions != newDescriptions)
    emit settingsUpdated(SETTINGS_SECTION_AUDIO, newDescriptions);
}
