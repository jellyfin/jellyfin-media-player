#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include <QObject>

class UpdateManager : public QObject
{
  Q_OBJECT
public:
  static bool CheckForUpdates();

  explicit UpdateManager(QObject *parent = nullptr) {};
  ~UpdateManager() override {};

  static UpdateManager* Get();

  virtual QString HaveUpdate();
  virtual bool applyUpdate(const QString &version);
  virtual void doUpdate(const QString& version);

  static QString GetPath(const QString &file, const QString& version, bool package);
};

#endif // UPDATEMANAGER_H
