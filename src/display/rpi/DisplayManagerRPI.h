#ifndef DISPLAYMANAGERRPI_H
#define DISPLAYMANAGERRPI_H

#include <vector>
#include <bcm_host.h>
#include <interface/vmcs_host/vc_tvservice.h>

#include "display/DisplayManager.h"

class DisplayManagerRPI : public DisplayManager
{
  Q_OBJECT
private:
  std::vector<TV_SUPPORTED_MODE_NEW_T> m_modes;

private Q_SLOTS:
  void handleTvChange(uint32_t reason);

  static void tv_callback(void *callback_data, uint32_t reason, uint32_t param1, uint32_t param2);

Q_SIGNALS:
  void onTvChange(uint32_t reason);

public:
  DisplayManagerRPI(QObject* parent);
  virtual ~DisplayManagerRPI();

  virtual bool initialize();
  virtual void resetRendering();
  virtual bool setDisplayMode(int display, int mode);
  virtual int getCurrentDisplayMode(int display);
  virtual int getMainDisplay() { return 0; }
  virtual int getDisplayFromPoint(int x, int y) { return 0; }
};

#endif
