//
//  DisplayManager.h
//  konvergo
//
//  Created by Lionel CHAZALLON on 28/09/2014.
//
//

#ifndef _DISPLAYMANAGER_H_
#define _DISPLAYMANAGER_H_

#include <QMap>
#include <QPoint>
#include <QString>
#include <QSharedPointer>

///////////////////////////////////////////////////////////////////////////////////////////////////
// Video Modes
class DMVideoMode
{
public:
  int id;
  int width;
  int height;
  int bitsPerPixel;
  float refreshRate;
  bool interlaced;

  int priv_id;

  inline QString getPrettyName()
  {
    QString name;

    name = QString("%1 x%2%3").arg(width, 5).arg(height, 5).arg((interlaced ? "i" : " "));
    name += QString("x %1bpp @%2Hz").arg(bitsPerPixel, 2).arg(refreshRate);
    return name;
  }
};

typedef QSharedPointer<DMVideoMode> DMVideoModePtr;
typedef QMap<int, DMVideoModePtr> DMVideoModeMap;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Displays
class DMDisplay
{
public:
  int id;
  QString name;

  int priv_id;

  DMVideoModeMap videoModes;
};

typedef QSharedPointer<DMDisplay> DMDisplayPtr;
typedef QMap<int, DMDisplayPtr> DMDisplayMap;

///////////////////////////////////////////////////////////////////////////////////////////////////
// MatchMediaInfo
class DMMatchMediaInfo
{
public:
  DMMatchMediaInfo() : refreshRate(0), interlaced(false) {};
  DMMatchMediaInfo(float refreshRate, bool interlaced)
    : refreshRate(refreshRate), interlaced(interlaced) {};

  float refreshRate;
  bool interlaced;
};

// Matching weights
#define MATCH_WEIGHT_RES 1000

#define MATCH_WEIGHT_REFRESH_RATE_EXACT               100
#define MATCH_WEIGHT_REFRESH_RATE_MULTIPLE            75
#define MATCH_WEIGHT_REFRESH_RATE_CLOSE               50
#define MATCH_WEIGHT_REFRESH_RATE_MULTIPLE_CLOSE      25

#define MATCH_WEIGHT_INTERLACE 10

#define MATCH_WEIGHT_CURRENT 5

///////////////////////////////////////////////////////////////////////////////////////////////////
// VideoMode Weight
class DMVideoModeWeight
{
public:
  float weight;
  DMVideoModePtr mode;
};

typedef QSharedPointer<DMVideoModeWeight> DMVideoModeWeightPtr;
typedef QMap<int, DMVideoModeWeightPtr> DMVideoModeWeightMap;

///////////////////////////////////////////////////////////////////////////////////////////////////
// DisplayManager
class DisplayManager : public QObject
{
  Q_OBJECT
public:
  DisplayManager(QObject* parent);
  virtual ~DisplayManager() {}

  DMDisplayMap displays;

  // functions that should be implemented on each platform
  virtual bool initialize();
  virtual bool setDisplayMode(int display, int mode) = 0;
  virtual int getCurrentDisplayMode(int display) = 0;
  virtual int getMainDisplay() = 0;
  virtual int getDisplayFromPoint(int x, int y) = 0;

  // extra functions that can be implemented
  virtual void resetRendering() {}

  // other classes functions
  int findBestMatch(int display, DMMatchMediaInfo& matchInfo);
  DMVideoModePtr getCurrentVideoMode(int display);

  int findBestMode(int display);
  
  bool isValidDisplay(int display);
  bool isValidDisplayMode(int display, int mode);
  int getDisplayFromPoint(const QPoint& pt);

private:
  bool isRateMultipleOf(float refresh, float multiple, bool exact = true);
};

typedef QSharedPointer<DisplayManager> DisplayManagerPtr;

#endif /* _DISPLAYMANAGER_H_ */
