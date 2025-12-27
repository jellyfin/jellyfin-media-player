#include "ProfileManager.h"
#include "Paths.h"
#include "Names.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QUuid>
#include <QDebug>

static const QString PROFILES_DIR = "profiles";

QString Profile::dataDir(const QString& subpath) const
{
    QString path = Paths::globalDataDir(PROFILES_DIR + "/" + id);
    if (!subpath.isEmpty())
        path += "/" + subpath;
    return path;
}

QString Profile::cacheDir(const QString& subpath) const
{
    QString path = Paths::globalCacheDir(PROFILES_DIR + "/" + id);
    if (!subpath.isEmpty())
        path += "/" + subpath;
    return path;
}

QString Profile::logDir() const
{
#ifdef Q_OS_MAC
    // macOS: ~/Library/Logs/<AppName>/profiles/<id>/
    QString home = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    return home + "/Library/Logs/" + Names::DataName() + "/profiles/" + id;
#else
    // Other platforms: profiles/<id>/logs/
    return dataDir("logs");
#endif
}

QString Profile::name() const
{
    QFile meta(dataDir("profile.json"));
    if (meta.open(QIODevice::ReadOnly))
    {
        QJsonDocument doc = QJsonDocument::fromJson(meta.readAll());
        meta.close();
        return doc.object()["name"].toString();
    }
    return QString();
}

void Profile::ensureDirectories() const
{
    QDir().mkpath(dataDir());
    QDir().mkpath(logDir());
    QDir().mkpath(cacheDir());
}

ProfileManager::ProfileManager(QObject* parent)
    : QObject(parent)
{
}

void ProfileManager::setActiveProfile(const Profile& profile)
{
    if (m_activeProfile.id == profile.id)
        return;

    m_activeProfile = profile;
    m_activeProfile.ensureDirectories();

    qInfo() << "Activated profile:" << profile.name();
    emit activeProfileChanged();
}

Profile ProfileManager::profileById(const QString& id)
{
    Profile p;
    p.id = id;
    return p;
}

std::optional<Profile> ProfileManager::profileByName(const QString& name)
{
    QDir dir(profilesDir());
    if (!dir.exists())
        return std::nullopt;

    for (const QString& id : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
    {
        if (id.length() != 32 || !id.contains(QRegularExpression("^[0-9a-f]+$")))
            continue;

        Profile p = profileById(id);
        if (p.name() == name)
            return p;
    }
    return std::nullopt;
}

QList<Profile> ProfileManager::profiles()
{
    QList<Profile> result;
    QDir dir(profilesDir());
    if (!dir.exists())
        return result;

    for (const QString& id : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name))
    {
        if (id.length() != 32 || !id.contains(QRegularExpression("^[0-9a-f]+$")))
            continue;

        result.append(profileById(id));
    }
    return result;
}

Profile ProfileManager::createProfile(const QString& name)
{
    QString id = QUuid::createUuid().toString(QUuid::Id128);
    Profile profile = profileById(id);
    profile.ensureDirectories();

    // Write profile.json with name
    QFile meta(profile.dataDir("profile.json"));
    if (meta.open(QIODevice::WriteOnly))
    {
        QJsonObject obj;
        obj["name"] = name;
        meta.write(QJsonDocument(obj).toJson());
        meta.close();
    }

    return profile;
}

bool ProfileManager::deleteProfile(const Profile& profile)
{
    // Delete data, cache, and log directories
    QDir(profile.dataDir()).removeRecursively();
    QDir(profile.cacheDir()).removeRecursively();
    QDir(profile.logDir()).removeRecursively();

    // Update default if we deleted it
    auto current = defaultProfile();
    if (current && current->id == profile.id)
    {
        auto remaining = profiles();
        if (!remaining.isEmpty())
            setDefaultProfile(remaining.first());
    }

    return true;
}

std::optional<Profile> ProfileManager::defaultProfile()
{
    QString profilesFile = Paths::globalDataDir("profiles.json");
    QFile file(profilesFile);
    if (file.open(QIODevice::ReadOnly))
    {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();
        QString id = doc.object()["defaultProfile"].toString();
        if (!id.isEmpty())
        {
            Profile p = profileById(id);
            if (QDir(p.dataDir()).exists())
                return p;
        }
    }
    return std::nullopt;
}

void ProfileManager::setDefaultProfile(const Profile& profile)
{
    QString profilesFile = Paths::globalDataDir("profiles.json");
    QJsonObject root;
    root["defaultProfile"] = profile.id;
    QFile out(profilesFile);
    if (out.open(QIODevice::WriteOnly))
    {
        out.write(QJsonDocument(root).toJson());
        out.close();
    }
}

QString ProfileManager::profilesDir()
{
    return Paths::globalDataDir(PROFILES_DIR);
}
