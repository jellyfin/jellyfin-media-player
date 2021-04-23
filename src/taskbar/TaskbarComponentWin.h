#ifndef TASKBARCOMPONENTWIN_H
#define TASKBARCOMPONENTWIN_H

#include <QWinTaskbarButton>
#include <QWinTaskbarProgress>
#include <QWinThumbnailToolBar>
#include <QWinThumbnailToolButton>


#include "TaskbarComponent.h"

class TaskbarComponentWin : public TaskbarComponent
{
public:
  TaskbarComponentWin(): TaskbarComponent(nullptr) {}
  virtual void setWindow(QQuickWindow* window) override;

private:
  void onPauseClicked();
  void onPrevClicked();
  void onNextClicked();
  void setProgress(quint64 value);
  void setControlsVisible(bool value);
  void setPaused(bool value);
  void playing();
  void stopped();
  void paused();

  QWinTaskbarButton* m_button;
  QWinThumbnailToolBar* m_toolbar;
  QWinThumbnailToolButton* m_pause;
  QWinThumbnailToolButton* m_prev;
  QWinThumbnailToolButton* m_next;
};

#endif // TASKBARCOMPONENTWIN_H
