//
// Created by Tobias Hieta on 01/09/15.
//

#ifndef KONVERGO_PATHS_H
#define KONVERGO_PATHS_H

#include <QString>
#include <QUrl>
#include <QVariant>

namespace Paths
{
  void setConfigDir(const QString& path);
  void setCacheDir(const QString& path);
  bool isPortableMode();
  bool detectAndEnablePortableMode();
  QString resourceDir(const QString& file = QString());
  QString socketName(const QString& serverName);
  QString soundsPath(const QString& sound);
  QString webClientPath(const QString& mode = "tv");
  QString webExtensionPath(const QString& mode = "extension");

  // Global paths (always ~/.local/share/jellyfin-desktop or equivalent)
  QString globalDataDir(const QString& file = QString());
  QString globalCacheDir(const QString& file = QString());

  // Profile-aware paths (profile-specific when active, fallback to global)
  void setActiveProfileId(const QString& id);
  QString activeProfileId();
  QString dataDir(const QString& file = QString());
  QString cacheDir(const QString& file = QString());
  QString logDir(const QString& file = QString());
};

#endif //KONVERGO_PATHS_H
