#ifndef PROFILEMANAGER_H
#define PROFILEMANAGER_H

#include <QObject>
#include <QList>
#include <optional>
#include "utils/Utils.h"

class ProfileManager;

struct Profile
{
    // Paths
    QString dataDir(const QString& subpath = QString()) const;
    QString cacheDir(const QString& subpath = QString()) const;
    QString logDir() const;

    // Metadata
    QString name() const;

    // Directory management
    void ensureDirectories() const;

private:
    QString id;
    friend class ProfileManager;
};

class ProfileManager : public QObject
{
    Q_OBJECT
    DEFINE_SINGLETON(ProfileManager);

    Q_PROPERTY(bool hasActiveProfile READ hasActiveProfile NOTIFY activeProfileChanged)

public:
    // Active profile
    static Profile& activeProfile() { return Get().m_activeProfile; }
    void setActiveProfile(const Profile& profile);
    bool hasActiveProfile() const { return !m_activeProfile.id.isEmpty(); }

    // Profile lookup
    static std::optional<Profile> profileByName(const QString& name);
    static QList<Profile> profiles();

    // Profile management
    static Profile createProfile(const QString& name);
    static bool deleteProfile(const Profile& profile);

    // Default profile
    static std::optional<Profile> defaultProfile();
    static void setDefaultProfile(const Profile& profile);

Q_SIGNALS:
    void activeProfileChanged();

private:
    explicit ProfileManager(QObject* parent = nullptr);

    static Profile profileById(const QString& id);
    static QString profilesDir();

    Profile m_activeProfile;
};

#endif // PROFILEMANAGER_H
