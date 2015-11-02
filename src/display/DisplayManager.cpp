//
//  DisplayManager.cpp
//  konvergo
//
//  Created by Lionel CHAZALLON on 28/09/2014.
//
//

#include "QsLog.h"
#include "DisplayManager.h"
#include "math.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
DisplayManager::DisplayManager(QObject* parent) : QObject(parent) {}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool DisplayManager::initialize()
{
  QLOG_INFO() << QString("DisplayManager found %1 Display(s).").arg(displays.size());

  // list video modes
  foreach(int displayid, displays.keys())
  {
    DMDisplayPtr display = displays[displayid];
    QLOG_INFO() << QString("Available modes for Display #%1 (%2)").arg(displayid).arg(display->name);
    for (int modeid = 0; modeid < display->videoModes.size(); modeid++)
    {
      DMVideoModePtr mode = display->videoModes[modeid];
      QLOG_INFO() << QString("Mode %1: %2").arg(modeid, 2).arg(mode->getPrettyName());
    }
  }

  // Log current display mode
  int mainDisplay = getMainDisplay();
  if (mainDisplay >= 0)
  {
    int currentMode = getCurrentDisplayMode(mainDisplay);
    if (currentMode >= 0)
      QLOG_INFO() << QString("DisplayManager : Current Display Mode on Display #%1 is %2")
                     .arg(mainDisplay)
                     .arg(displays[mainDisplay]->videoModes[currentMode]->getPrettyName());
    else
      QLOG_ERROR() << "DisplayManager : unable to retrieve current video mode";
  }
  else
    QLOG_ERROR() << "DisplayManager : unable to retrieve main display";

  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
DMVideoModePtr DisplayManager::getCurrentVideoMode(int display)
{
  int currentMode = getCurrentDisplayMode(display);
  DMVideoModePtr currentVideoMode;

  if (currentMode >= 0)
    currentVideoMode = displays[display]->videoModes[currentMode];

  return currentVideoMode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool DisplayManager::isValidDisplay(int display) { return display >= 0 && display < displays.size(); }

///////////////////////////////////////////////////////////////////////////////////////////////////
bool DisplayManager::isValidDisplayMode(int display, int mode)
{
  if (isValidDisplay(display))
    if (mode >= 0 && mode < displays[display]->videoModes.size())
      return true;

  return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool DisplayManager::isRateMultipleOf(float refresh, float multiple, bool exact)
{
  if (((int)refresh == 0) || ((int)multiple == 0))
      return false;

  if (((int)multiple % (int)refresh) == 0)
    return true;

  int roundedRefresh = (int)round(refresh);
  int roundedMultiple = (int)round(multiple);
  if (((roundedMultiple % roundedRefresh) == 0) && (!exact))
    return true;

  return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int DisplayManager::findBestMatch(int display, DMMatchMediaInfo& matchInfo)
{

  // Grab current videomode information
  DMVideoModePtr currentVideoMode = getCurrentVideoMode(display);
  if (!currentVideoMode)
    return -1;

  // now we try to find the exact match in current resolution
  // then fill a list
  DMVideoModeWeightMap weights;

  DMVideoModeMap::const_iterator modeit = displays[display]->videoModes.constBegin();

  while (modeit != displays[display]->videoModes.constEnd())
  {
    DMVideoModePtr candidate = modeit.value();

    weights[candidate->id] = DMVideoModeWeightPtr(new DMVideoModeWeight);
    weights[candidate->id]->mode = candidate;
    weights[candidate->id]->weight = 0;

    // Weight Resolution match
    if ((candidate->width == currentVideoMode->width) &&
        (candidate->height == currentVideoMode->height) &&
        (candidate->bitsPerPixel == currentVideoMode->bitsPerPixel))
    {
      weights[candidate->id]->weight += MATCH_WEIGHT_RES;
    }

    // weight refresh rate
    // exact Match
    if (candidate->refreshRate == matchInfo.refreshRate)
      weights[candidate->id]->weight += MATCH_WEIGHT_REFRESH_RATE_EXACT;

    // exact multiple refresh rate
    if (isRateMultipleOf(matchInfo.refreshRate, candidate->refreshRate, true))
      weights[candidate->id]->weight += MATCH_WEIGHT_REFRESH_RATE_MULTIPLE;

    // close refresh match (less than 1 hz diff to match all 23.xxx modes to 24p)
    if (fabs(candidate->refreshRate - matchInfo.refreshRate) <= 1)
    {
      weights[candidate->id]->weight += MATCH_WEIGHT_REFRESH_RATE_CLOSE;
    }

    // approx multiple refresh rate
    if (isRateMultipleOf(matchInfo.refreshRate, candidate->refreshRate, false))
      weights[candidate->id]->weight += MATCH_WEIGHT_REFRESH_RATE_MULTIPLE_CLOSE;

    // weight interlacing
    if (candidate->interlaced == matchInfo.interlaced)
      weights[candidate->id]->weight += MATCH_WEIGHT_INTERLACE;

    if (candidate->id == currentVideoMode->id)
      weights[candidate->id]->weight += MATCH_WEIGHT_CURRENT;

    modeit++;
  }

  // now grab the mode with the highest weight
  DMVideoModeWeightPtr chosen;
  float maxWeight = 0;

  DMVideoModeWeightMap::const_iterator weightit = weights.constBegin();
  while (weightit != weights.constEnd())
  {
    QLOG_DEBUG() << "Mode " << weightit.value()->mode->id << "("
                 << weightit.value()->mode->getPrettyName() << ") has weight "
                 << weightit.value()->weight;
    if (weightit.value()->weight > maxWeight)
    {
      chosen = weightit.value();
      maxWeight = chosen->weight;
    }

    weightit++;
  }

  if ((chosen) && (chosen->weight > MATCH_WEIGHT_RES))
  {
    QLOG_INFO() << "DisplayManager RefreshMatch : found a suitable mode : "
                << chosen->mode->getPrettyName();
    return chosen->mode->id;
  }

  QLOG_INFO() << "DisplayManager RefreshMatch : found no suitable videomode";
  return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int DisplayManager::findBestMode(int display)
{
  int best_mode = -1;

  foreach (auto mode, displays[display]->videoModes)
  {
    if (best_mode < 0)
    {
      best_mode = mode->id;
    }
    else
    {
      DMVideoModePtr best = displays[display]->videoModes[best_mode];
      DMVideoModePtr candidate = mode;

      // Highest priority: prefer non-interlaced modes.
      if (!best->interlaced && candidate->interlaced)
        continue;

      if (best->bitsPerPixel > candidate->bitsPerPixel)
        continue;

      if (best->width > candidate->width)
        continue;

      if (best->height > candidate->height)
        continue;

      if (best->refreshRate > candidate->refreshRate)
        continue;

      best_mode = candidate->id;
    }
  }

  return best_mode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int DisplayManager::getDisplayFromPoint(const QPoint& pt)
{
  return getDisplayFromPoint(pt.x(), pt.y());
}
