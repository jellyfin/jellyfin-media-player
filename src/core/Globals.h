#ifndef KONVERGOENGINE_H
#define KONVERGOENGINE_H

#include <QQmlApplicationEngine>

class KonvergoWindow;

/////////////////////////////////////////////////////////////////////////////////////////
namespace Globals
{
  QQmlApplicationEngine* Engine();
  QVariant ContextProperty(const QString& property);
  void SetContextProperty(const QString& property, QObject* object);
  void SetContextProperty(const QString& property, const QVariant& value);
  void EngineDestroy();
  KonvergoWindow* MainWindow();
};

#endif // KONVERGOENGINE_H
