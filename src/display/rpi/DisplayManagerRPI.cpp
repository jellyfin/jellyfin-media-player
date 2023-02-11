#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <private/qguiapplication_p.h>
#include <qpa/qplatformintegration.h>

#include "QsLog.h"
#include "DisplayManagerRPI.h"
#include "display/DisplayComponent.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
void DisplayManagerRPI::tv_callback(void *callback_data, uint32_t reason, uint32_t param1, uint32_t param2)
{
  DisplayManagerRPI* obj = (DisplayManagerRPI *)callback_data;

  emit obj->onTvChange(reason);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
DisplayManagerRPI::DisplayManagerRPI(QObject* parent) : DisplayManager(parent)
{
  connect(this, &DisplayManagerRPI::onTvChange,
          this, &DisplayManagerRPI::handleTvChange,
          Qt::QueuedConnection);

  vc_tv_register_callback(&tv_callback, (void *)this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
DisplayManagerRPI::~DisplayManagerRPI()
{
  vc_tv_unregister_callback_full(&tv_callback, (void *)this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DisplayManagerRPI::handleTvChange(uint32_t reason)
{
  QLOG_INFO() << "tv_service notification:" << reason;

  if (reason & (VC_HDMI_DVI | VC_HDMI_HDMI))
  {
    // Looks like a mode change - Qt's dispmanx state is probably trashed, recreate it.
    resetRendering();
  }
  else if (reason & VC_HDMI_ATTACHED)
  {
    // Plugged in, but is in standby mode. May happen when reconnecting a monitor via HDMI.
    QLOG_INFO() << "Powering on screen.";
    initialize();
    DisplayComponent::Get().switchToBestOverallVideoMode(0);
  }
  else if (reason & VC_HDMI_UNPLUGGED)
  {
    QLOG_INFO() << "Screen was unplugged.";
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
static void get_modes(std::vector<TV_SUPPORTED_MODE_NEW_T>& modes, HDMI_RES_GROUP_T group)
{
  const int max_modes = 256;

  size_t count = modes.size();
  modes.resize(count + max_modes);

  HDMI_RES_GROUP_T preferred_group;
  uint32_t preferred_mode;
  int got = vc_tv_hdmi_get_supported_modes_new(group, &modes[count], max_modes, &preferred_group, &preferred_mode);
  modes.resize(count + got);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool DisplayManagerRPI::initialize()
{
  bcm_host_init();

  m_displays.clear();

  // create the main display
  DMDisplayPtr display = DMDisplayPtr(new DMDisplay);
  display->m_id = 0;
  display->m_name = "Display";
  m_displays[display->m_id] = display;

  // fills mode array with both CEA and DMT
  m_modes.resize(0);
  get_modes(m_modes, HDMI_RES_GROUP_CEA); // TV
  get_modes(m_modes, HDMI_RES_GROUP_DMT); // PC

  for (size_t n = 0; n < m_modes.size(); n++)
  {
    TV_SUPPORTED_MODE_NEW_T* tvmode = &m_modes[n];
    DMVideoModePtr mode = DMVideoModePtr(new DMVideoMode);
    mode->m_id = n * 2 + 0;
    display->m_videoModes[mode->m_id] = mode;

    mode->m_height = tvmode->height;
    mode->m_width = tvmode->width;
    mode->m_refreshRate = tvmode->frame_rate;
    mode->m_interlaced = (tvmode->scan_mode == 1);
    mode->m_bitsPerPixel = 32;

    mode = DMVideoModePtr(new DMVideoMode(*mode));
    mode->m_id = n * 2 + 1;
    display->m_videoModes[mode->m_id] = mode;
    mode->m_refreshRate /= 1.001;
  }

  if (m_modes.size() == 0)
    return false;
  else
    return DisplayManager::initialize();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool DisplayManagerRPI::setDisplayMode(int display, int mode)
{
  if (!isValidDisplayMode(display, mode))
    return false;

  HDMI_PROPERTY_PARAM_T property;
  property.property = HDMI_PROPERTY_PIXEL_CLOCK_TYPE;
  property.param1 = (mode % 2) ? HDMI_PIXEL_CLOCK_TYPE_NTSC : HDMI_PIXEL_CLOCK_TYPE_PAL;
  property.param2 = 0;
  vc_tv_hdmi_set_property(&property);

  TV_SUPPORTED_MODE_NEW_T* tvmode = &m_modes[mode / 2];
  bool ret = vc_tv_hdmi_power_on_explicit_new(HDMI_MODE_HDMI, (HDMI_RES_GROUP_T)tvmode->group, tvmode->code) == 0;
  if (!ret)
  {
    QLOG_ERROR() << "Failed to switch display mode" << ret;
  }

  return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int DisplayManagerRPI::getCurrentDisplayMode(int display)
{
  if (!isValidDisplay(display))
    return -1;

  TV_GET_STATE_RESP_T tvstate;
  if (vc_tv_get_state(&tvstate))
    return -1;

  for (int mode = 0; mode < m_displays[display]->m_videoModes.size(); mode++)
  {
    TV_SUPPORTED_MODE_NEW_T* tvmode = &m_modes[mode];
    if (tvmode->width == tvstate.width &&
        tvmode->height == tvstate.height &&
        tvmode->frame_rate == tvstate.frame_rate &&
        tvmode->scan_mode == tvstate.scan_mode)
    {
      bool ntsc = false;
      HDMI_PROPERTY_PARAM_T property;
      property.property = HDMI_PROPERTY_PIXEL_CLOCK_TYPE;
      property.param1 = 0;
      property.param2 = 0;
      if (!vc_tv_hdmi_get_property(&property))
        ntsc = property.param1 == HDMI_PIXEL_CLOCK_TYPE_NTSC;
      return mode * 2 + (ntsc ? 1 : 0);
    }
  }

  return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DisplayManagerRPI::resetRendering()
{
  QGuiApplication *guiApp = (QGuiApplication*)QGuiApplication::instance();
  QQuickWindow *window = (QQuickWindow*)guiApp->focusWindow();
  if (window)
  {
    QLOG_INFO() << "Recreating Qt UI renderer";

    // destroy the window to reset  OpenGL context
    window->setPersistentGraphics(false);
    window->setPersistentSceneGraph(false);
    window->destroy();

    // Grab the Platform integration private object and recreate it
    // this allows to clean / recreate the dispmanx objects
    QGuiApplicationPrivate *privateApp = (QGuiApplicationPrivate *)QGuiApplicationPrivate::get(guiApp);
    QPlatformIntegration *integration = privateApp->platformIntegration();

    if (integration)
    {
      integration->destroy();
      QThread::msleep(500);
      integration->initialize();
    }
    else
    {
      QLOG_ERROR() << "Failed to retrieve platform integration";
    }

    // now recreate the window OpenGL context
    window->setScreen(QGuiApplication::primaryScreen());
    window->create();
  }
}
