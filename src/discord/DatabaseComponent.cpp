
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

#include "DatabaseComponent.h"

DatabaseComponent::DatabaseComponent(QObject* parent) : ComponentBase(parent)
{
  qDebug() << "[DatabaseComponent] Init";
}

bool DatabaseComponent::componentInitialize()
{
  qDebug() << "[DatabaseComponent] Initializing database";
  QString dbPath = getDatabasePath();
  bool dbExists = QFile::exists(dbPath);

  QSqlDatabase m_db = QSqlDatabase::addDatabase("QSQLITE"); // Driver for SQLite
  m_db.setDatabaseName(dbPath);
  
  if (!m_db.open())
  {
    qCritical() << "[DatabaseComponent] Failed to open database:" << m_db.lastError().text();
    return false;
  }

  if (!dbExists)
  {
    qDebug() << "[DatabaseComponent] Database created for the first time at:" << dbPath;
    
    QSqlQuery query;
    QString createTable = QString(
      "CREATE TABLE IF NOT EXISTS %1 ("
      " hash TEXT PRIMARY KEY,"
      " value TEXT NOT NULL"
      ");"
    ).arg(DATABASE_NAME);

    if (!query.exec(createTable))
    {
      qCritical() << "[DatabaseComponent] Failed to create table:" << query.lastError().text();
      return false;
    }
  }
  else
  {
    qDebug() << "[DatabaseComponent] Database already exists at:" << dbPath;
  }

  return true;
}

bool DatabaseComponent::storeUrl(const QString& hash, const QString& url)
{
  QSqlQuery query;
  QString sql = QString(
        "INSERT INTO %1(hash, value)"
        "VALUES (:hash, :url)"
        "ON CONFLICT(hash) DO UPDATE SET value = excluded.value;"
    ).arg(DATABASE_NAME);
  
    query.prepare(sql);
  query.bindValue(":hash", hash);
  query.bindValue(":url", url);

  if (!query.exec())
  {
    qCritical() << "Failed to store URL:" << query.lastError().text();
    return false;
  }

  qDebug() << "Stored URL for hash" << hash << ":" << url;
  return true;
}

QString DatabaseComponent::getUrlForHash(const QString& hash)
{
  QSqlQuery query;
  QString sql = QString("SELECT value FROM %1 WHERE hash = :hash").arg(DATABASE_NAME);
  query.prepare(sql);
  query.bindValue(":hash", hash);

  if (!query.exec())
  {
    qCritical() << "URL lookup failed:" << query.lastError().text();
    return {};
  }

  if (query.next())
  {
    QString result = query.value(0).toString();
    qDebug() << "Found URL for hash" << hash << ":" << result;
    return result; // URL
  }

  return {}; // Not found
}

QString DatabaseComponent::getDatabasePath()
{
  QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir().mkpath(path); // Ensure the directory exists
  return path + "/" + DATABASE_NAME + ".sqlite";
}

const char* DatabaseComponent::componentName() { return "DatabaseComponent"; }
bool DatabaseComponent::componentExport() { return true; }