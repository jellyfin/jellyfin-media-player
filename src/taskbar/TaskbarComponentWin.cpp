#include <QApplication>
#include <QStyle>


#include "TaskbarComponentWin.h"

/////////////////////////////////////////////////////////////////////////////////////////
void TaskbarComponentWin::setWindow(QQuickWindow* window)
{
  TaskbarComponent::setWindow(window);

  m_button = new QWinTaskbarButton(m_window);
  m_button->setWindow(m_window);

  m_toolbar = new QWinThumbnailToolBar(m_window);
  m_toolbar->setWindow(m_window);

  m_pause = new QWinThumbnailToolButton(m_toolbar);
  connect(m_pause, &QWinThumbnailToolButton::clicked, this, &TaskbarComponentWin::onPauseClicked);

  m_toolbar->addButton(m_pause);

  setControlsVisible(false);
  setPaused(false);
}

/////////////////////////////////////////////////////////////////////////////////////////
void TaskbarComponentWin::onPauseClicked()
{
  emit pauseClicked();
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
void TaskbarComponentWin::setProgress(int value)
{
  m_button->progress()->setValue(value);
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
