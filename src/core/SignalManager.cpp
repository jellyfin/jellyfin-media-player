#include <QDebug>

#include "sys/socket.h"
#include "unistd.h"
#include "signal.h"

#include "SignalManager.h"
#include "settings/SettingsComponent.h"

int SignalManager::g_sigtermFd[2];

///////////////////////////////////////////////////////////////////////////////////////////////////
SignalManager::SignalManager(QGuiApplication* app) : QObject(nullptr), m_app(app)
{
  if (setupHandlers())
  {
    qCritical() << "Failed to install SignalDaemon handlers.";
  }

  qDebug() << "Signal handlers installed successfully.";

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, SignalManager::g_sigtermFd))
  {
    qCritical() << "Couldn't create TERM socketpair";
  }

  m_snTerm = new QSocketNotifier(SignalManager::g_sigtermFd[1], QSocketNotifier::Read, this);
  connect(m_snTerm, SIGNAL(activated(int)), this, SLOT(handleSignal()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int SignalManager::setupHandlers()
{
  struct sigaction term;

  term.sa_handler = SignalManager::signalHandler;
  sigemptyset(&term.sa_mask);
  term.sa_flags = SA_RESTART | SA_RESETHAND;

  if (sigaction(SIGHUP, &term, nullptr) < 0)
    return -1;

  if (sigaction(SIGTERM, &term, nullptr) < 0)
    return -2;

  term.sa_flags = SA_RESTART;
  if (sigaction(SIGUSR1, &term, nullptr) < 0)
    return -3;

  return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SignalManager::signalHandler(int signal_num)
{
  unsigned char a = signal_num < 255 ? signal_num : 0;
  write(g_sigtermFd[0], &a, sizeof(a));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SignalManager::handleSignal()
{
  m_snTerm->setEnabled(false);
  unsigned char signalNumber = 0;
  read(g_sigtermFd[1], &signalNumber, sizeof(signalNumber));

  // do Qt stuff
  if (signalNumber == SIGUSR1)
  {
    qDebug() << "Received SIGUSR1, reloading config file";
    SettingsComponent::Get().load();
  }
  else
  {
    qDebug() << "Received signal, closing application";
    closeApplication();
  }

  m_snTerm->setEnabled(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SignalManager::closeApplication()
{
  if (m_app)
    m_app->quit();
}
