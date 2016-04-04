#ifndef INPUTMAPPING_H
#define INPUTMAPPING_H

#include <QMap>
#include <QObject>
#include <QFileSystemWatcher>
#include <QRegExp>
#include <QVariantMap>
#include <QMutex>
#include <utils/CachedRegexMatcher.h>

class InputMapping : public QObject
{
  Q_OBJECT

public:
  explicit InputMapping(QObject *parent = nullptr);
  bool loadMappings();
  QVariantList mapToAction(const QString& source, const QString& keycode);

private Q_SLOTS:
  void dirChange();

signals:
  void mappingChanged();

private:
  bool loadMappingFile(const QString &path, QPair<QString, QVariantMap> &mappingPair);
  bool loadMappingDirectory(const QString& path, bool copy);

  QFileSystemWatcher* m_watcher;

  QHash<QString, CachedRegexMatcher*> m_inputMatcher;
  CachedRegexMatcher m_sourceMatcher;
};

#endif // INPUTMAPPING_H
