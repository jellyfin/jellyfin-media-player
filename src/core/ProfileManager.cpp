#include "ProfileManager.h"
#include "Paths.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUuid>
#include <QDebug>
#include <QRegularExpression>

static const QString PROFILES_FILE = "profiles.json";
static const QString PROFILES_DIR = "profiles";

QVariantMap Profile::toVariantMap() const
{
    QVariantMap map;
    map["id"] = id;
    map["name"] = name;
    map["url"] = url;
    map["created"] = created.toString(Qt::ISODate);
    return map;
}

Profile Profile::fromVariantMap(const QVariantMap& map)
{
    Profile p;
    p.id = map["id"].toString();
    p.name = map["name"].toString();
    p.url = map["url"].toString();
    p.created = QDateTime::fromString(map["created"].toString(), Qt::ISODate);
    return p;
}

ProfileManager::ProfileManager(QObject* parent)
    : ComponentBase(parent)
{
}

bool ProfileManager::componentInitialize()
{
    loadProfiles();
    ensureActiveProfile();
    return true;
}

void ProfileManager::ensureActiveProfile()
{
    // CLI --profile flag takes precedence (already set in Paths before ComponentManager)
    QString cliProfileId = Paths::activeProfileId();
    if (!cliProfileId.isEmpty())
    {
        m_activeProfileId = cliProfileId;
        // Ensure profile is in our list
        if (!m_profiles.contains(cliProfileId))
        {
            scanProfilesDirectory();
        }
        return;
    }

    // Already have valid active profile from profiles.json
    if (!m_activeProfileId.isEmpty() && m_profiles.contains(m_activeProfileId))
    {
        Paths::setActiveProfileId(m_activeProfileId);
        return;
    }

    // Try to find first existing profile from profiles directory
    if (m_profiles.isEmpty())
    {
        scanProfilesDirectory();
    }

    // Use first available profile
    if (!m_profiles.isEmpty())
    {
        QString firstId = m_profiles.keys().first();
        qInfo() << "Activating first available profile:" << firstId;
        m_activeProfileId = firstId;
        Paths::setActiveProfileId(firstId);
        saveProfiles();
        return;
    }

    // No profiles exist - create one
    qInfo() << "No profiles found, creating new profile";
    QString id = generateProfileId();

    Profile p;
    p.id = id;
    p.name = "Default";
    p.url = "";
    p.created = QDateTime::currentDateTime();

    ensureProfileDirectories(id);
    m_profiles[id] = p;
    m_activeProfileId = id;
    Paths::setActiveProfileId(id);
    saveProfiles();
}

void ProfileManager::scanProfilesDirectory()
{
    QString profilesPath = Paths::globalDataDir(PROFILES_DIR);
    QDir profilesDir(profilesPath);

    if (!profilesDir.exists())
        return;

    for (const QString& dirName : profilesDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name))
    {
        // Check if it looks like a UUID
        if (dirName.length() == 32 && dirName.contains(QRegularExpression("^[0-9a-f]+$")))
        {
            Profile p;
            p.id = dirName;
            p.name = "Recovered Profile";
            p.url = "";
            p.created = QDateTime::currentDateTime();
            m_profiles[dirName] = p;
            qInfo() << "Found orphan profile directory:" << dirName;
        }
    }
}

void ProfileManager::loadProfiles()
{
    QString filePath = Paths::globalDataDir(PROFILES_FILE);
    QFile file(filePath);

    if (!file.exists())
    {
        qInfo() << "No profiles.json found, starting fresh";
        return;
    }

    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Failed to open profiles.json for reading";
        return;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    file.close();

    if (error.error != QJsonParseError::NoError)
    {
        qWarning() << "Failed to parse profiles.json:" << error.errorString();
        return;
    }

    QJsonObject root = doc.object();

    // Load default profile ID
    m_activeProfileId = root["defaultProfile"].toString();

    // Load profiles array
    QJsonArray profilesArray = root["profiles"].toArray();
    for (const QJsonValue& val : profilesArray)
    {
        QJsonObject obj = val.toObject();
        Profile p;
        p.id = obj["id"].toString();
        p.name = obj["name"].toString();
        p.url = obj["url"].toString();
        p.created = QDateTime::fromString(obj["created"].toString(), Qt::ISODate);

        if (!p.id.isEmpty())
        {
            m_profiles[p.id] = p;
        }
    }

    // Validate active profile exists
    if (!m_activeProfileId.isEmpty() && !m_profiles.contains(m_activeProfileId))
    {
        qWarning() << "Active profile" << m_activeProfileId << "not found in profiles list";
        m_activeProfileId.clear();
    }

    qInfo() << "Loaded" << m_profiles.size() << "profiles from profiles.json";
}

void ProfileManager::saveProfiles()
{
    QJsonObject root;
    root["defaultProfile"] = m_activeProfileId;

    QJsonDocument doc(root);
    QByteArray data = doc.toJson(QJsonDocument::Indented);

    QString filePath = Paths::globalDataDir(PROFILES_FILE);
    if (!Utils::safelyWriteFile(filePath, data))
    {
        qCritical() << "Failed to save profiles.json";
    }
}

QString ProfileManager::generateProfileId()
{
    return QUuid::createUuid().toString(QUuid::Id128);
}

void ProfileManager::ensureProfileDirectories(const QString& id)
{
    // Data directory (use global since profile may not be active yet)
    QString dataPath = Paths::globalDataDir(PROFILES_DIR + "/" + id);
    QDir dataDir(dataPath);
    if (!dataDir.exists())
    {
        dataDir.mkpath(dataPath);
    }

    // Logs subdirectory
    QString logsPath = dataPath + "/logs";
    QDir logsDir(logsPath);
    if (!logsDir.exists())
    {
        logsDir.mkpath(logsPath);
    }

    // Cache directory (use global since profile may not be active yet)
    QString cachePath = Paths::globalCacheDir(PROFILES_DIR + "/" + id);
    QDir cacheDir(cachePath);
    if (!cacheDir.exists())
    {
        cacheDir.mkpath(cachePath);
    }
}

QString ProfileManager::createProfile(const QString& name, const QString& url)
{
    QString id = generateProfileId();

    Profile p;
    p.id = id;
    p.name = name;
    p.url = url;
    p.created = QDateTime::currentDateTime();

    ensureProfileDirectories(id);

    m_profiles[id] = p;
    saveProfiles();

    emit profilesChanged();

    qInfo() << "Created profile" << id << ":" << name << "for" << url;
    return id;
}

bool ProfileManager::deleteProfile(const QString& id)
{
    if (!m_profiles.contains(id))
    {
        qWarning() << "Cannot delete profile" << id << "- not found";
        return false;
    }

    // Clear active if deleting current
    if (m_activeProfileId == id)
    {
        m_activeProfileId.clear();
        emit activeProfileChanged();
    }

    m_profiles.remove(id);
    saveProfiles();

    // Optionally delete profile directories (leave for now, user can clean manually)
    // This avoids accidental data loss

    emit profilesChanged();

    qInfo() << "Deleted profile" << id;
    return true;
}

bool ProfileManager::renameProfile(const QString& id, const QString& newName)
{
    if (!m_profiles.contains(id))
    {
        qWarning() << "Cannot rename profile" << id << "- not found";
        return false;
    }

    m_profiles[id].name = newName;
    saveProfiles();

    emit profilesChanged();

    qInfo() << "Renamed profile" << id << "to" << newName;
    return true;
}

QVariantList ProfileManager::listProfiles()
{
    QVariantList list;
    for (const Profile& p : m_profiles)
    {
        list.append(p.toVariantMap());
    }
    return list;
}

QVariantMap ProfileManager::getProfile(const QString& id)
{
    if (m_profiles.contains(id))
    {
        return m_profiles[id].toVariantMap();
    }
    return QVariantMap();
}

bool ProfileManager::setActiveProfile(const QString& id)
{
    if (id.isEmpty())
    {
        m_activeProfileId.clear();
        Paths::setActiveProfileId(QString());
        saveProfiles();
        emit activeProfileChanged();
        return true;
    }

    if (!m_profiles.contains(id))
    {
        qWarning() << "Cannot activate profile" << id << "- not found";
        return false;
    }

    m_activeProfileId = id;
    Paths::setActiveProfileId(id);
    ensureProfileDirectories(id);
    saveProfiles();

    emit activeProfileChanged();

    qInfo() << "Activated profile" << id << ":" << m_profiles[id].name;
    return true;
}

QString ProfileManager::profileDataDir() const
{
    if (m_activeProfileId.isEmpty())
        return QString();

    return Paths::dataDir();
}

QString ProfileManager::profileCacheDir() const
{
    if (m_activeProfileId.isEmpty())
        return QString();

    return Paths::cacheDir();
}
