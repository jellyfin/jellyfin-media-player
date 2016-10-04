//
// Created by Tobias Hieta on 28/08/15.
//

#include "HelperLauncher.h"
#include "QsLog.h"
#include "settings/SettingsComponent.h"
#include "settings/SettingsSection.h"
#include "utils/Utils.h"
#include "Names.h"

#include <QTimer>

/////////////////////////////////////////////////////////////////////////////////////////
HelperLauncher::HelperLauncher(QObject* parent) : QObject(parent)
{
  start();
}

/////////////////////////////////////////////////////////////////////////////////////////
void HelperLauncher::start()
{
  m_jsonClient = new LocalJsonClient("pmpHelper");
  connect(m_jsonClient, &QLocalSocket::connected, this, &HelperLauncher::didConnect);
  connect(m_jsonClient, static_cast<void (QLocalSocket::*)(QLocalSocket::LocalSocketError)>(&QLocalSocket::error), this, &HelperLauncher::socketError);
  connect(m_jsonClient, &LocalJsonClient::messageReceived, this, &HelperLauncher::gotMessage);
  connect(m_jsonClient, &QLocalSocket::disconnected, this, &HelperLauncher::socketDisconnect);

#ifdef Q_OS_MAC
  m_launchd = new HelperLaunchd(this);
#endif

  m_helperProcess = new QProcess(this);

  connect(SettingsComponent::Get().getSection(SETTINGS_SECTION_WEBCLIENT), &SettingsSection::valuesUpdated, [=](const QVariantMap& values)
  {
    if (values.contains("clientID"))
      updateClientId();
  });
}

/////////////////////////////////////////////////////////////////////////////////////////
bool HelperLauncher::connectToHelper()
{
  if (m_jsonClient->state() == QLocalSocket::ConnectedState ||
      m_jsonClient->state() == QLocalSocket::ConnectingState)
    return true;

  QLOG_DEBUG() << "Connecting to helper";
  m_jsonClient->connectToServer();

  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
bool HelperLauncher::killHelper()
{
  QVariantMap msg;
  msg.insert("command", "quit");
  m_jsonClient->sendMessage(msg);

  if (m_jsonClient->waitForBytesWritten(1000))
  {
    QLOG_DEBUG() << "Waiting for helper to die";
    if (!m_jsonClient->waitForDisconnected(3000))
    {
      QLOG_ERROR() << "Helper refused to disconnect. Didn't it get the PM?";
      return false;
    }

    QLOG_DEBUG() << "Helper gone";
    return true;
  }
  return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
void HelperLauncher::gotMessage(const QVariantMap& message)
{
  if (message.value("version").toString() != Version::GetVersionString())
  {
    QLOG_WARN() << "Running helper does not match our current version. Killing it and starting a new one.";
    killHelper();
  }
  else
  {
    QLOG_DEBUG() << "Helper is running version:" << message.value("version").toString();
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
void HelperLauncher::socketError(QLocalSocket::LocalSocketError error)
{
  QLOG_DEBUG() << "Failed to connect to helper:" << m_jsonClient->errorString();

  if (error == QLocalSocket::ConnectionRefusedError ||
      error == QLocalSocket::ServerNotFoundError)
    launch();
}

/////////////////////////////////////////////////////////////////////////////////////////
void HelperLauncher::socketDisconnect()
{
  QLOG_DEBUG() << "Disconnected from helper, trying to relaunch";
  connectToHelper();
}

/////////////////////////////////////////////////////////////////////////////////////////
void HelperLauncher::updateClientId()
{
  // update clientId if we have it
  if (!SettingsComponent::Get().value(SETTINGS_SECTION_WEBCLIENT, "clientID").toString().isEmpty())
  {
    QVariantMap msg;
    msg.insert("command", "info");

    QVariantMap arg;
    arg.insert("clientId", SettingsComponent::Get().value(SETTINGS_SECTION_WEBCLIENT, "clientID").toString());

    QString userId = Utils::CurrentUserId();
    if (!userId.isEmpty())
      arg.insert("userId", userId);

    msg.insert("argument", arg);

    m_jsonClient->sendMessage(msg);
  }
};

/////////////////////////////////////////////////////////////////////////////////////////
void HelperLauncher::didConnect()
{
  QLOG_DEBUG() << "Connected to helper";
  updateClientId();
}

/////////////////////////////////////////////////////////////////////////////////////////
void HelperLauncher::launch()
{
  QLOG_DEBUG() << "Launching helper:" << HelperPath();

#ifdef Q_OS_MAC
  m_launchd->start();
#else
  if (!m_helperProcess->startDetached(HelperPath(), QStringList()))
  {
    QLOG_ERROR() << "Failed to open helper: " + m_helperProcess->errorString();
    throw FatalException("Failed to launch helper: " + m_helperProcess->errorString());
  }
#endif

  QTimer::singleShot(1000, this, &HelperLauncher::connectToHelper);
}

/////////////////////////////////////////////////////////////////////////////////////////
QString HelperLauncher::HelperPath()
{
  QString programName = Names::HelperName();
#ifdef Q_OS_WIN
  programName += ".exe";
#endif

  QString helperPath = SettingsComponent::Get().value(SETTINGS_SECTION_PATH, "helperprogram").toString();

  // fallback to the resource dir
  if (helperPath.isEmpty() || !QFile().exists(helperPath))
    helperPath = Paths::resourceDir(programName);

  if (!QFile().exists(helperPath))
    throw FatalException("Can't find helper program");

  return helperPath;
}

/////////////////////////////////////////////////////////////////////////////////////////
void HelperLauncher::stop()
{
  // this method needs to disconnect all signals from the helper as well, so it doesn't start up again.
  m_jsonClient->disconnect();
  killHelper();
}
