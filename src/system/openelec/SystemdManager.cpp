#include <QProcess>
#include <QDebug>

#include "SystemdManager.h"

#define CACHE_DIR "/storage/.cache/services/"
#define TEMPLATE_DIR "/usr/share/services/"

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SystemdManager::isEnabled(SystemdService &Service)
{
    return QFile::exists(CACHE_DIR + Service.ConfigFile + ".conf");
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SystemdManager::enable(SystemdService &Service, bool enable)
{
    QFile onFile(CACHE_DIR + Service.ConfigFile + ".conf");
    QFile offFile(CACHE_DIR + Service.ConfigFile + ".disabled");
    QFile templateFile(TEMPLATE_DIR + Service.ConfigFile + ".conf");

    qInfo() << "Setting service" << Service.Name << "to enable state :" << enable;

    if (!onFile.exists() && !offFile.exists())
    {
        // we take the template file and copy it over
        if(!templateFile.copy(enable ? onFile.fileName() : offFile.fileName()))
        {
            qCritical() << "Failed to copy template file for service " << Service.Name;
            return false;
        }
    }

    bool rename = false;
    if (onFile.exists() && !enable)
    {
       rename = onFile.rename(offFile.fileName());
    }

    if (offFile.exists() && enable)
    {
       rename = offFile.rename(onFile.fileName());
    }

    if (rename)
    {
        QProcess p;
        p.start("systemctl", QStringList() << "restart" << Service.Name);

        if (p.waitForFinished(1000))
        {
            return true;
        }
        else
        {
            qCritical() << "Failed to restart service " << Service.Name;
            return false;
        }
    }

    return false;
}

