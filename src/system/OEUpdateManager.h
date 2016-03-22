#ifndef OEUPDATEMANAGER_H
#define OEUPDATEMANAGER_H

#include "UpdateManager.h"

class OEUpdateManager : public UpdateManager
{
public:
  OEUpdateManager(QObject *parent = nullptr) {};
  ~OEUpdateManager() {};

  virtual QString HaveUpdate();
  virtual bool applyUpdate(const QString &version);
  virtual void doUpdate(const QString& version);

private:
  bool isMiniUpdateArchive(QString archivePath);
};

#endif // OEUPDATEMANAGER_H
