#include "DisplayManagerX11.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
bool DisplayManagerX11::initialize()
{
  m_displays.clear();

  if (!xdisplay)
    xdisplay = XOpenDisplay(NULL);
  if (!xdisplay)
    return false;

  int event_base, error_base;
  if (!XRRQueryExtension(xdisplay, &event_base, &error_base))
    return false;

  if (resources)
    XRRFreeScreenResources(resources);

  resources = XRRGetScreenResources(xdisplay, RootWindow(xdisplay, DefaultScreen(xdisplay)));
  if (!resources)
    return false;

  for (int o = 0; o < resources->noutput; o++) {
    RROutput output = resources->outputs[o];
    XRRCrtcInfo *crtc = NULL;
    XRROutputInfo *out = XRRGetOutputInfo(xdisplay, resources, output);
    if (!out || !out->crtc)
      goto next;
    crtc = XRRGetCrtcInfo(xdisplay, resources, out->crtc);
    if (!crtc)
      goto next;

    {
      DMDisplayPtr display = DMDisplayPtr(new DMDisplay());
      display->m_id = m_displays.size();
      display->m_name = out->name;
      display->m_privId = o;
      m_displays[display->m_id] = display;

      for (int om = 0; om < out->nmode; om++) {
        RRMode xm = out->modes[om];
        for (int n = 0; n < resources->nmode; n++) {
          XRRModeInfo m = resources->modes[n];
          if (m.id != xm)
            continue;

          DMVideoModePtr mode = DMVideoModePtr(new DMVideoMode());
          mode->m_privId = n;
          mode->m_id = display->m_videoModes.size();
          display->m_videoModes[mode->m_id] = mode;

          mode->m_interlaced = m.modeFlags & RR_Interlace;

          double vTotal = m.vTotal;
          if (m.modeFlags & RR_DoubleScan)
            vTotal *= 2;
          if (m.modeFlags & RR_Interlace)
            vTotal /= 2;
          mode->m_refreshRate = m.dotClock / (m.hTotal * vTotal);

          mode->m_width = m.width;
          mode->m_height = m.height;
          mode->m_bitsPerPixel = 0; // we can't know; depth is not managed by xrandr
        }
      }
    }
  next:
    if (crtc)
      XRRFreeCrtcInfo(crtc);
    if (out)
      XRRFreeOutputInfo(out);
  }

  if (m_displays.empty())
    return false;
  else
    return DisplayManager::initialize();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool DisplayManagerX11::setDisplayMode(int display, int mode)
{
  if (!isValidDisplayMode(display, mode) || !resources)
    return false;

  DMDisplayPtr displayptr = m_displays[display];
  DMVideoModePtr videomode = displayptr->m_videoModes[mode];

  RROutput output = resources->outputs[displayptr->m_privId];
  RRMode xrmode = resources->modes[videomode->m_privId].id;

  bool success = false;
  XRRCrtcInfo *crtc = NULL;
  XRROutputInfo *out = XRRGetOutputInfo(xdisplay, resources, output);
  if (!out || !out->crtc)
    goto done;
  crtc = XRRGetCrtcInfo(xdisplay, resources, out->crtc);
  if (!crtc)
    goto done;

  // Keep all information, except the mode.
  success = XRRSetCrtcConfig(xdisplay, resources, out->crtc, crtc->timestamp,
                             crtc->x, crtc->y, xrmode, crtc->rotation,
                             crtc->outputs, crtc->noutput);

  // The return value isn't always accurate, apparently.
  success = true;

done:
  if (crtc)
    XRRFreeCrtcInfo(crtc);
  if (out)
    XRRFreeOutputInfo(out);

  return success;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int DisplayManagerX11::getCurrentDisplayMode(int display)
{
  if (!isValidDisplay(display) || !resources)
    return -1;

  DMDisplayPtr displayptr = m_displays[display];
  RROutput output = resources->outputs[displayptr->m_privId];

  int videomode_id = -1;
  XRRCrtcInfo *crtc = NULL;
  XRROutputInfo *out = XRRGetOutputInfo(xdisplay, resources, output);
  if (!out || !out->crtc)
    goto done;
  crtc = XRRGetCrtcInfo(xdisplay, resources, out->crtc);
  if (!crtc)
    goto done;

  foreach (DMVideoModePtr mode, displayptr->m_videoModes)
  {
    XRRModeInfo m = resources->modes[mode->m_privId];
    if (crtc->mode == m.id)
    {
      videomode_id = mode->m_id;
      break;
    }
  }

done:
  if (crtc)
    XRRFreeCrtcInfo(crtc);
  if (out)
    XRRFreeOutputInfo(out);

  return videomode_id;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
int DisplayManagerX11::getMainDisplay()
{
  // This is probably not what DisplayManager means.
  RROutput main = XRRGetOutputPrimary(xdisplay, RootWindow(xdisplay, DefaultScreen(xdisplay)));
  if (!main || !resources)
    return -1;

  for (int o = 0; o < resources->noutput; o++)
  {
    if (main == resources->outputs[o])
    {
      for (int displayid = 0; displayid < m_displays.size(); displayid++)
      {
        if (m_displays[displayid]->m_privId == o)
          return displayid;
      }
      break;
    }
  }
  return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int DisplayManagerX11::getDisplayFromPoint(int x, int y)
{
  for (int displayid = 0; displayid < m_displays.size(); displayid++)
  {
    RROutput output = resources->outputs[m_displays[displayid]->m_privId];
    XRRCrtcInfo *crtc = NULL;
    XRROutputInfo *out = XRRGetOutputInfo(xdisplay, resources, output);
    bool matches = false;
    if (!out || !out->crtc)
      goto done;
    crtc = XRRGetCrtcInfo(xdisplay, resources, out->crtc);
    if (!crtc)
      goto done;

    matches = x >= crtc->x && y >= crtc->y &&
              x < crtc->x + crtc->width &&
              y < crtc->y + crtc->height;

  done:
    if (crtc)
      XRRFreeCrtcInfo(crtc);
    if (out)
      XRRFreeOutputInfo(out);

    if (matches)
      return displayid;
  }

  return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
DisplayManagerX11::~DisplayManagerX11()
{
  if (resources)
    XRRFreeScreenResources(resources);
  if (xdisplay)
    XCloseDisplay(xdisplay);
}
