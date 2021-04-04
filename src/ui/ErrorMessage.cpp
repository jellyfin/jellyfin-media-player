//
// Created by Tobias Hieta on 18/03/16.
//

#include <QCoreApplication>
#include <QPushButton>
#include <QDesktopServices>

#include "system/SystemComponent.h"
#include "settings/SettingsComponent.h"
#include "ErrorMessage.h"

/////////////////////////////////////////////////////////////////////////////////////////
ErrorMessage::ErrorMessage(const QString& errorMessage, bool allowResetConfig)
  : QMessageBox(nullptr)
{
  setIcon(Critical);
  setText("Jellyfin Media Player encountered a fatal error and must exit");
  setDetailedText(errorMessage);
  setInformativeText("Press help below to be redirected to our friendly support forums." \
                     "Press reset to reset configuration to the default and try again." \
                     "Press abort to exit.");
  setWindowTitle("PMP Fatal Error!");

  auto exitButton = addButton(Abort);
  auto helpButton = addButton(Help);

  QPushButton* resetButton = nullptr;
  if (allowResetConfig)
    resetButton = addButton(Reset);

  connect(this, &QMessageBox::buttonClicked, [=](QAbstractButton* button)
  {
    if (button == exitButton)
    {
      qApp->quit();
    }
    else if (button == resetButton)
    {
      SettingsComponent::resetAndSaveOldConfiguration();
      SystemComponent::restart();
    }
    else if (button == helpButton)
    {
      QDesktopServices::openUrl(QUrl("https://jellyfin.org"));
    }
  });
}


