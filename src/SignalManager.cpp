#include "sys/socket.h"
#include "unistd.h"
#include "signal.h"

#include "QsLog.h"
#include "SignalManager.h"
#include "settings/SettingsComponent.h"

int SignalManager::sigtermFd[2];

///////////////////////////////////////////////////////////////////////////////////////////////////
SignalManager::SignalManager(QGuiApplication* app) : QObject(NULL), m_app(app)
{
  if (setupHandlers())
  {
    QLOG_ERROR() << "Failed to install SignalDaemon handlers.";
  }

  QLOG_DEBUG() << "Signal handlers installed successfully.";

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, SignalManager::sigtermFd))
  {
    QLOG_ERROR() << "Couldn't create TERM socketpair";
  }

  snTerm = new QSocketNotifier(SignalManager::sigtermFd[1], QSocketNotifier::Read, this);
  connect(snTerm, SIGNAL(activated(int)), this, SLOT(handleSignal()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int SignalManager::setupHandlers()
{
  struct sigaction term;

  term.sa_handler = SignalManager::signalHandler;
  sigemptyset(&term.sa_mask);
  term.sa_flags = SA_RESTART | SA_RESETHAND;

  if (sigaction(SIGHUP, &term, 0) < 0)
    return -1;

  if (sigaction(SIGTERM, &term, 0) < 0)
    return -2;

  term.sa_flags = SA_RESTART;
  if (sigaction(SIGUSR1, &term, 0) < 0)
    return -3;

  return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SignalManager::signalHandler(int signal_num)
{
  unsigned char a = signal_num < 255 ? signal_num : 0;
  write(sigtermFd[0], &a, sizeof(a));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SignalManager::handleSignal()
{
  snTerm->setEnabled(false);
  unsigned char signal_number = 0;
  read(sigtermFd[1], &signal_number, sizeof(signal_number));

  // do Qt stuff
  if (signal_number == SIGUSR1)
  {
    QLOG_DEBUG() << "Received SIGUSR1, reloading config file";
    SettingsComponent::Get().load();
  }
  else
  {
    QLOG_DEBUG() << "Received signal, closing application";
    closeApplication();
  }

  snTerm->setEnabled(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SignalManager::closeApplication()
{
  if (m_app)
    m_app->quit();
}
