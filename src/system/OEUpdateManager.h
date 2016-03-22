#ifndef OEUPDATEMANAGER_H
#define OEUPDATEMANAGER_H

#include "UpdateManager.h"

class OEUpdateManager : public UpdateManager
{
public:
  explicit OEUpdateManager(QObject *parent = nullptr) {};
  ~OEUpdateManager() override {};

  QString HaveUpdate() override;
  bool applyUpdate(const QString &version) override;
  void doUpdate(const QString& version) override;

private:
  bool isMiniUpdateArchive(QString archivePath);
};

#endif // OEUPDATEMANAGER_H
