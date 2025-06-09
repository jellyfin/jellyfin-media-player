#ifndef DATABASECOMPONENT_H
#define DATABASECOMPONENT_H

#include "ComponentManager.h"
#include <atomic>
#include <QObject>
#include <QtSql/QSqlDatabase>

class DatabaseComponent : public ComponentBase
{
    Q_OBJECT
    DEFINE_SINGLETON(DatabaseComponent);

public:
    explicit DatabaseComponent(QObject* parent = nullptr);
    ~DatabaseComponent() = default;

    virtual bool componentInitialize() override;
    virtual const char* componentName() override;
    virtual bool componentExport() override;
    bool storeUrl(const QString& hash, const QString& url);
    QString getUrlForHash(const QString& hash);

private:
    const QString DATABASE_NAME = "imgur_link_mapping";
    QString getDatabasePath();
};

#endif