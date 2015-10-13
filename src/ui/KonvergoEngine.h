#ifndef KONVERGOENGINE_H
#define KONVERGOENGINE_H

#include <QQmlApplicationEngine>

static QQmlApplicationEngine* g_qmlEngine = NULL;

class KonvergoEngine
{
public:
  static QQmlApplicationEngine* Get()
  {
    if (!g_qmlEngine)
      g_qmlEngine = new QQmlApplicationEngine();

    return g_qmlEngine;
  }
};

#endif // KONVERGOENGINE_H
