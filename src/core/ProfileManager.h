#ifndef PROFILEMANAGER_H
#define PROFILEMANAGER_H

#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include <QDateTime>
#include "ComponentManager.h"
#include "utils/Utils.h"

struct Profile
{
    QString id;
    QString name;
    QString url;
    QDateTime created;

    QVariantMap toVariantMap() const;
    static Profile fromVariantMap(const QVariantMap& map);
};

class ProfileManager : public ComponentBase
{
    Q_OBJECT
    DEFINE_SINGLETON(ProfileManager);

    Q_PROPERTY(QString activeProfileId READ activeProfileId NOTIFY activeProfileChanged)
    Q_PROPERTY(QString profileDataDir READ profileDataDir NOTIFY activeProfileChanged)
    Q_PROPERTY(QString profileCacheDir READ profileCacheDir NOTIFY activeProfileChanged)
    Q_PROPERTY(bool hasActiveProfile READ hasActiveProfile NOTIFY activeProfileChanged)

public:
    bool componentExport() override { return true; }
    const char* componentName() override { return "profiles"; }
    bool componentInitialize() override;

    // Profile management
    Q_INVOKABLE QString createProfile(const QString& name, const QString& url);
    Q_INVOKABLE bool deleteProfile(const QString& id);
    Q_INVOKABLE bool renameProfile(const QString& id, const QString& newName);
    Q_INVOKABLE QVariantList listProfiles();
    Q_INVOKABLE QVariantMap getProfile(const QString& id);

    // Active profile
    Q_INVOKABLE bool setActiveProfile(const QString& id);
    QString activeProfileId() const { return m_activeProfileId; }
    bool hasActiveProfile() const { return !m_activeProfileId.isEmpty(); }

    // Path accessors for QML
    QString profileDataDir() const;
    QString profileCacheDir() const;

Q_SIGNALS:
    void activeProfileChanged();
    void profilesChanged();

private:
    explicit ProfileManager(QObject* parent = nullptr);

    void loadProfiles();
    void saveProfiles();
    void ensureActiveProfile();
    void scanProfilesDirectory();
    QString generateProfileId();
    void ensureProfileDirectories(const QString& id);

    QMap<QString, Profile> m_profiles;
    QString m_activeProfileId;
};

#endif // PROFILEMANAGER_H
