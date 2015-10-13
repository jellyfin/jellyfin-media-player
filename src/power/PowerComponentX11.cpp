#include <stdlib.h>

#include "PowerComponentX11.h"
#include "QsLog.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
PowerComponentX11::PowerComponentX11() : PowerComponent(0)
{
  m_timer = new QTimer(this);
  connect(m_timer, &QTimer::timeout, this, &PowerComponentX11::onTimer);
  m_timer->setInterval(15 * 1000);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PowerComponentX11::onTimer()
{
  if (!m_process && !m_broken)
  {
    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::ForwardedChannels);
    connect(m_process, (void (QProcess::*)(int,QProcess::ExitStatus))&QProcess::finished,
            this, &PowerComponentX11::onProcessFinished);
    connect(m_process, (void (QProcess::*)(QProcess::ProcessError))&QProcess::error,
            this, &PowerComponentX11::onProcessError);
    m_process->start("xdg-screensaver", {"reset"});
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PowerComponentX11::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  if (m_process)
    m_process->deleteLater();
  m_process = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PowerComponentX11::onProcessError(QProcess::ProcessError error)
{
  QLOG_ERROR() << "Disabling screensaver is not working. Make sure xdg-screensaver is installed.";
  m_broken = true;
  onProcessFinished(-1, QProcess::CrashExit);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PowerComponentX11::doDisableScreensaver()
{
  m_timer->start();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PowerComponentX11::doEnableScreensaver()
{
  m_timer->stop();
}
