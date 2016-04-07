#ifndef SIGNALMANAGER_H
#define SIGNALMANAGER_H

#include <QObject>
#include <QSocketNotifier>
#include <QGuiApplication>

class SignalManager : public QObject
{
  Q_OBJECT

public:
  explicit SignalManager(QGuiApplication* app);
  ~SignalManager() override {}

  // Unix signal handlers.
  static void signalHandler(int signal_num);

  int setupHandlers();
  void closeApplication();

public slots:
  // Qt signal handlers.
  void handleSignal();

private:
  static int g_sigtermFd[2];

  QSocketNotifier* m_snTerm;
  QGuiApplication* m_app;
};

#endif // SIGNALMANAGER_H
