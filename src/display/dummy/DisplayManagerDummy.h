#ifndef DISPLAYMANAGERDUMMY_H_
#define DISPLAYMANAGERDUMMY_H_

#include "display/DisplayManager.h"

class DisplayManagerDummy : public DisplayManager
{
  Q_OBJECT
private:

  void addMode(float rate);

  int m_currentMode;

public:
  explicit DisplayManagerDummy(QObject* parent) : DisplayManager(parent), m_currentMode(0) {};

  bool initialize() override;
  bool setDisplayMode(int display, int mode) override;
  int getCurrentDisplayMode(int display) override;
  int getMainDisplay() override;
  int getDisplayFromPoint(int x, int y) override;
};

#endif /* DISPLAYMANAGERX11_H_ */
