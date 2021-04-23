#include <QApplication>
#include <QStyle>


#include "TaskbarComponentWin.h"
#include "PlayerComponent.h"
#include "input/InputComponent.h"

/////////////////////////////////////////////////////////////////////////////////////////
void TaskbarComponentWin::setWindow(QQuickWindow* window)
{
  TaskbarComponent::setWindow(window);

  m_button = new QWinTaskbarButton(m_window);
  m_button->setWindow(m_window);

  m_toolbar = new QWinThumbnailToolBar(m_window);
  m_toolbar->setWindow(m_window);

  m_prev = new QWinThumbnailToolButton(m_toolbar);
  connect(m_prev, &QWinThumbnailToolButton::clicked, this, &TaskbarComponentWin::onPrevClicked);

  m_pause = new QWinThumbnailToolButton(m_toolbar);
  connect(m_pause, &QWinThumbnailToolButton::clicked, this, &TaskbarComponentWin::onPauseClicked);

  m_next = new QWinThumbnailToolButton(m_toolbar);
  connect(m_next, &QWinThumbnailToolButton::clicked, this, &TaskbarComponentWin::onNextClicked);

  m_prev->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaSkipBackward));
  m_next->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaSkipForward));

  m_toolbar->addButton(m_prev);
  m_toolbar->addButton(m_pause);
  m_toolbar->addButton(m_next);

  connect(&PlayerComponent::Get(), &PlayerComponent::positionUpdate, this, &TaskbarComponentWin::setProgress);
  connect(&PlayerComponent::Get(), &PlayerComponent::playing, this, &TaskbarComponentWin::playing);
  connect(&PlayerComponent::Get(), &PlayerComponent::paused, this, &TaskbarComponentWin::paused);
  connect(&PlayerComponent::Get(), &PlayerComponent::stopped, this, &TaskbarComponentWin::stopped);

  setControlsVisible(false);
  setPaused(false);
}

/////////////////////////////////////////////////////////////////////////////////////////
void TaskbarComponentWin::onPauseClicked()
{
  InputComponent::Get().sendAction("play_pause");
}

/////////////////////////////////////////////////////////////////////////////////////////
void TaskbarComponentWin::onNextClicked()
{
  InputComponent::Get().sendAction("next");
}

/////////////////////////////////////////////////////////////////////////////////////////
void TaskbarComponentWin::onPrevClicked()
{
  InputComponent::Get().sendAction("previous");
}

/////////////////////////////////////////////////////////////////////////////////////////
void TaskbarComponentWin::playing()
{
  setControlsVisible(true);
  setPaused(false);
}

/////////////////////////////////////////////////////////////////////////////////////////
void TaskbarComponentWin::paused()
{
  setPaused(true);
}

/////////////////////////////////////////////////////////////////////////////////////////
void TaskbarComponentWin::stopped()
{
  setControlsVisible(false);
}

/////////////////////////////////////////////////////////////////////////////////////////
void TaskbarComponentWin::setControlsVisible(bool value)
{
  m_button->progress()->setVisible(value);

  for (auto& button : m_toolbar->buttons())
  {
    button->setVisible(value);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
void TaskbarComponentWin::setProgress(quint64 value)
{
  qint64 duration = PlayerComponent::Get().getDuration();
  int progress = 0;
  if (duration != 0) {
    progress = (int) (value / duration / 10);
  }
  m_button->progress()->setValue(progress);
}

/////////////////////////////////////////////////////////////////////////////////////////
void TaskbarComponentWin::setPaused(bool value)
{
  if (value)
  {
    // m_pause->setToolTip("Resume");
    m_pause->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
  }
  else
  {
    // m_pause->setToolTip("Pause");
    m_pause->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPause));
  }

  m_button->progress()->setPaused(value);
}
