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
  virtual void setControlsVisible(bool value) override;
  virtual void setProgress(int value) override;
  virtual void setPaused(bool value) override;

  virtual void setWindow(QQuickWindow* window) override;

private:
  void onPauseClicked();

  QWinTaskbarButton* m_button;
  QWinThumbnailToolBar* m_toolbar;
  QWinThumbnailToolButton* m_pause;
};

#endif // TASKBARCOMPONENTWIN_H
