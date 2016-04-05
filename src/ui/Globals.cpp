//
// Created by Tobias Hieta on 05/04/16.
//

#include "Globals.h"
#include "KonvergoWindow.h"

#include <QQmlContext>

static QQmlApplicationEngine* g_qmlEngine = nullptr;

#define QT_FORCE_ASSERTS 1

/////////////////////////////////////////////////////////////////////////////////////////
QQmlApplicationEngine* Globals::Engine()
{
  if (!g_qmlEngine)
    g_qmlEngine = new QQmlApplicationEngine();

  return g_qmlEngine;
}

/////////////////////////////////////////////////////////////////////////////////////////
QVariant Globals::ContextProperty(const QString& property)
{
  Q_ASSERT_X(g_qmlEngine, "Globals", "QmlEngine not inited yet");
  return g_qmlEngine->rootContext()->contextProperty(property);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Globals::SetContextProperty(const QString& property, QObject* object)
{
  Q_ASSERT_X(g_qmlEngine, "Globals", "QmlEngine not inited yet");
  g_qmlEngine->rootContext()->setContextProperty(property, object);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Globals::SetContextProperty(const QString& property, const QVariant& value)
{
  Q_ASSERT_X(g_qmlEngine, "Globals", "QmlEngine not inited yet");
  g_qmlEngine->rootContext()->setContextProperty(property, value);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Globals::EngineDestroy()
{
  delete g_qmlEngine;
  g_qmlEngine = nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
KonvergoWindow* Globals::MainWindow()
{
  Q_ASSERT_X(g_qmlEngine, "Globals", "QmlEngine not inited yet");

  auto rootObject = g_qmlEngine->rootObjects().first();
  Q_ASSERT_X(g_qmlEngine, "Globals", "No root objects in QmlEngine");

  auto window = qobject_cast<KonvergoWindow*>(rootObject);
  Q_ASSERT_X(g_qmlEngine, "Globals", "RootObject in QmlEngine is not a KonvergoWindow");

  return window;
}
