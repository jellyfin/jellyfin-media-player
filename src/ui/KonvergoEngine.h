#ifndef KONVERGOENGINE_H
#define KONVERGOENGINE_H

#include <QQmlApplicationEngine>

static QQmlApplicationEngine* g_qmlEngine = nullptr;

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
