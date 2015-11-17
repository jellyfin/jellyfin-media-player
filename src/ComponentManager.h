#ifndef __COMPONENT_MANAGER_H__
#define __COMPONENT_MANAGER_H__

#include <QObject>
#include <QMap>
#include <QQmlContext>
#include <QQmlPropertyMap>
#include <QWebChannel>

#include "utils/Utils.h"
#include "server/HTTPServer.h"

class ComponentBase : public QObject
{
public:
  ComponentBase(QObject* parent = 0) : QObject(parent) { }
  
  virtual bool componentInitialize() = 0;
  virtual const char* componentName() = 0;
  virtual bool componentExport() = 0;

  // executed after ALL components are initialized
  virtual void componentPostInitialize() { }
};

class ComponentManager : public QObject
{
  Q_OBJECT
  DEFINE_SINGLETON(ComponentManager);

public:
  void initialize();
  inline QQmlPropertyMap &getQmlPropertyMap() { return m_qmlProperyMap; }
  void setWebChannel(QWebChannel* webChannel);

private:
  ComponentManager();
  void registerComponent(ComponentBase* comp);

  HttpServer* m_server;

  QMap<QString, ComponentBase*> m_components;
  QQmlPropertyMap m_qmlProperyMap;
};

#endif
