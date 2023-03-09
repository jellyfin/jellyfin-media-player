#include "settings/SettingsComponent.h"
#include "settings/SettingsSection.h"
#include "OESystemComponent.h"
#include "SystemdManager.h"
#include <QDebug>
#include <unistd.h>
#include <QFile>

QMap<QString, SystemdService> services = {
     { "samba" , SystemdService("smbd", "samba") },
     { "lirc" ,  SystemdService("lircd", "lircd") },
     { "ssh" ,  SystemdService("sshd", "sshd") }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
OESystemComponent::OESystemComponent(QObject *parent) : ComponentBase(parent)
{

}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool OESystemComponent::componentInitialize()
{

  connect(SettingsComponent::Get().getSection(SETTINGS_SECTION_SYSTEM), &SettingsSection::valuesUpdated,
            this, &OESystemComponent::updateSectionSettings);

  setHostName(SettingsComponent::Get().value(SETTINGS_SECTION_SYSTEM, "systemname").toString());

  foreach(QString service, services.keys())
  {
    bool Enabled = SystemdManager::isEnabled(services[service]);
    qCritical() << "Service " << services[service].Name << " enabled :" << Enabled;
    SettingsComponent::Get().setValue(SETTINGS_SECTION_SYSTEM,
                                      services[service].Name + "_enabled",
                                      Enabled);
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OESystemComponent::updateSectionSettings(const QVariantMap& values)
{
    foreach(QString service, services.keys())
    {
        QString keyName = services[service].Name + "_enabled";
        if (values.contains(keyName))
        {
            SystemdManager::enable(services[service], values[keyName].toBool());
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool OESystemComponent::setHostName(QString name)
{
  // first we change the hostname name for this session
  char* hostname = name.toUtf8().data();
  sethostname(hostname, strlen(hostname));

  // then edit the hostname file so that its persistent
  QFile file("/storage/.cache/hostname");
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
  {
    qCritical() << "setHostName : Failed to open" << file.fileName();
    return false;
  }

  QTextStream writer(&file);
  writer << name;
  file.close();

  return true;
}
