//
// Created by Tobias Hieta on 18/09/15.
//

#ifndef PLEXTHEATER_HELPERLAUNCHD_H
#define PLEXTHEATER_HELPERLAUNCHD_H

#include <QObject>
#include <QProcess>

class HelperLaunchd : public QObject
{
  Q_OBJECT
public:
  explicit HelperLaunchd(QObject* parent = nullptr);

  void start();
  void stop();

private:
  bool checkHelperPath();
  bool writePlist();
  bool loadHelper();
  bool unloadHelper();
  QString launchPlistPath();

  QProcess* m_launchctl;
};

#endif //PLEXTHEATER_HELPERLAUNCHD_H
