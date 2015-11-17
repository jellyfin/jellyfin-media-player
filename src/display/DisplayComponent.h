#ifndef DISPLAYCOMPONENT_H
#define DISPLAYCOMPONENT_H

#include "DisplayManager.h"
#include "ComponentManager.h"
#include <QScreen>
#include <QTimer>

class DisplayComponent : public ComponentBase
{
  Q_OBJECT
  DEFINE_SINGLETON(DisplayComponent);

public:
  ~DisplayComponent();

  virtual const char* componentName() { return "display"; }
  virtual bool componentExport() { return true; }
  virtual bool componentInitialize();
  virtual void componentPostInitialize();

  inline DisplayManager* getDisplayManager() { return m_displayManager; }
  int getApplicationDisplay(bool silent = false);

  void setApplicationWindow(QWindow* window) { m_applicationWindow = window; }

  // Switch to the best video mode for the given video framerate. Return true only if the actual
  // mode was switched. If a good match was found, but the current video mode didn't have to be
  // changed, return false. Return false on failure too.
  bool switchToBestVideoMode(float frameRate);

  // Switch to best overall video mode. This will also switch the resolution.
  bool switchToBestOverallVideoMode(int display);

  // The syntax is as follows: the command string consists of multiple arguments separated
  // by spaces. Each argument can be one of the following:
  //    <N>hz (e.g.: "24hz"): change the refresh rate
  //    <W>x<H> (e.g.: "1280x720"): change the resolution
  //    i: change to interlaced
  //    p: change to progressive ("not interlaced")
  // Example: "123x456 p 45hz"
  void switchCommand(QString command);

  double currentRefreshRate();

  QString debugInformation();

private:
  DisplayComponent(QObject *parent = 0);
  QString displayName(int display);
  QString modePretty(int display, int mode);

  DisplayManager  *m_displayManager;
  int m_lastVideoMode;
  int m_lastDisplay;
  QTimer m_initTimer;
  QWindow* m_applicationWindow;

public Q_SLOTS:
  void  monitorChange();
  bool  initializeDisplayManager();
  bool  restorePreviousVideoMode();

Q_SIGNALS:
  void refreshRateChanged();

};

#endif // DISPLAYCOMPONENT_H
