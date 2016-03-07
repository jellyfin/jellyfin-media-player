//
// Created by Tobias Hieta on 07/03/16.
//

#ifndef PLEXMEDIAPLAYER_EVENTFILTER_H
#define PLEXMEDIAPLAYER_EVENTFILTER_H

#include <QObject>
#include <QEvent>

class EventFilter : public QObject
{
  Q_OBJECT
public:
  EventFilter(QObject* parent = 0) : QObject(parent) {}

protected:
  bool eventFilter(QObject* watched, QEvent* event);
};

#endif //PLEXMEDIAPLAYER_EVENTFILTER_H
