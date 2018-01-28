#ifndef SYSTEMDMANAGER_H
#define SYSTEMDMANAGER_H
#include <QObject>

class SystemdService
{
  public:
    SystemdService() {};
    SystemdService(QString name, QString configfile) : Name(name), ConfigFile(configfile) {};
    QString Name;
    QString ConfigFile;

};

class SystemdManager : public QObject
{
  Q_OBJECT
public:
    explicit SystemdManager(QObject *parent = nullptr) {};
    ~SystemdManager() override {};

    static bool isEnabled(SystemdService &Service);
    static bool enable(SystemdService &Service, bool enable);

};
#endif // SYSTEMDMANAGER_H
