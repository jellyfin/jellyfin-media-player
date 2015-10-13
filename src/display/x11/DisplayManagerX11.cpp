#include "DisplayManagerX11.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
bool DisplayManagerX11::initialize()
{
  displays.clear();

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
      display->id = displays.size();
      display->name = out->name;
      display->priv_id = o;
      displays[display->id] = display;

      for (int om = 0; om < out->nmode; om++) {
        RRMode xm = out->modes[om];
        for (int n = 0; n < resources->nmode; n++) {
          XRRModeInfo m = resources->modes[n];
          if (m.id != xm)
            continue;

          DMVideoModePtr mode = DMVideoModePtr(new DMVideoMode());
          mode->priv_id = n;
          mode->id = display->videoModes.size();
          display->videoModes[mode->id] = mode;

          mode->interlaced = m.modeFlags & RR_Interlace;

          double vTotal = m.vTotal;
          if (m.modeFlags & RR_DoubleScan)
            vTotal *= 2;
          if (m.modeFlags & RR_Interlace)
            vTotal /= 2;
          mode->refreshRate = m.dotClock / (m.hTotal * vTotal);

          mode->width = m.width;
          mode->height = m.height;
          mode->bitsPerPixel = 0; // we can't know; depth is not managed by xrandr
        }
      }
    }
  next:
    if (crtc)
      XRRFreeCrtcInfo(crtc);
    if (out)
      XRRFreeOutputInfo(out);
  }

  if (displays.empty())
    return false;
  else
    return DisplayManager::initialize();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool DisplayManagerX11::setDisplayMode(int display, int mode)
{
  if (!isValidDisplayMode(display, mode) || !resources)
    return false;

  DMDisplayPtr displayptr = displays[display];
  DMVideoModePtr videomode = displayptr->videoModes[mode];

  RROutput output = resources->outputs[displayptr->priv_id];
  RRMode xrmode = resources->modes[videomode->priv_id].id;

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

  DMDisplayPtr displayptr = displays[display];
  RROutput output = resources->outputs[displayptr->priv_id];

  int videomode_id = -1;
  XRRCrtcInfo *crtc = NULL;
  XRROutputInfo *out = XRRGetOutputInfo(xdisplay, resources, output);
  if (!out || !out->crtc)
    goto done;
  crtc = XRRGetCrtcInfo(xdisplay, resources, out->crtc);
  if (!crtc)
    goto done;

  foreach (DMVideoModePtr mode, displayptr->videoModes)
  {
    XRRModeInfo m = resources->modes[mode->priv_id];
    if (crtc->mode == m.id)
    {
      videomode_id = mode->id;
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
      for (int displayid = 0; displayid < displays.size(); displayid++)
      {
        if (displays[displayid]->priv_id == o)
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
  for (int displayid = 0; displayid < displays.size(); displayid++)
  {
    RROutput output = resources->outputs[displays[displayid]->priv_id];
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
