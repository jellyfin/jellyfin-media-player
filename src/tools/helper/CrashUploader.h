//
// Created by Tobias Hieta on 26/08/15.
//

#ifndef KONVERGO_CRASHUPLOADER_H
#define KONVERGO_CRASHUPLOADER_H

#include <QObject>
#include <QMutex>
#include <qhttpmultipart.h>
#include <qnetworkreply.h>
#include <qfilesystemwatcher.h>
#include <qtimer.h>
#include "Version.h"

class CrashUploader : public QObject
{
  Q_OBJECT
public:
  CrashUploader(QObject* parent = 0);
  bool startUploader();

private:
  Q_SLOT void uploadCrashDump(const QString& version, const QString& path);

  QString incomingCurrentVersion() { return m_incoming + "/" + Version::GetVersionString(); }
  void deleteOldCrashDumps();
  void addFormField(QHttpMultiPart* multipart, const QString& name, const QString& value);
  void uploadAndDeleteCrashDumps();
  void moveFileBackToIncoming(const QString& version, const QString& filename);
  void watchCrashDir(bool watch);

  QNetworkAccessManager* m_manager;
  QFileSystemWatcher* m_watcher;
  QHash<QString, QString> m_urlToFileMap;
  QString m_old, m_incoming, m_processing;
  QTimer* m_scanTimer;
  QMutex m_scanLock;
};

#endif //KONVERGO_CRASHUPLOADER_H
