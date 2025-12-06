#include "InputMapping.h"
#include <QDir>
#include <QDirIterator>
#include <QByteArray>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>

#include "Paths.h"
#include "utils/Utils.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
InputMapping::InputMapping(QObject *parent) : QObject(parent), m_sourceMatcher(false)
{
  m_watcher = new QFileSystemWatcher(this);
  connect(m_watcher, &QFileSystemWatcher::directoryChanged, this, &InputMapping::dirChange);
  connect(m_watcher, &QFileSystemWatcher::fileChanged, this, &InputMapping::dirChange);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void InputMapping::dirChange()
{
  qInfo() << "Change to user input path, reloading mappings.";
  loadMappings();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool InputMapping::loadMappings()
{
  m_inputMatcher.clear();
  m_sourceMatcher.clear();

  // don't watch the path while we potentially copy files to the directory
  if (m_watcher->directories().size() > 0)
    m_watcher->removePath(Paths::dataDir("inputmaps"));

  // first we load the bundled mappings
  loadMappingDirectory(":/inputmaps", true);

  // now we load the user ones, if there are any
  // they will now overload the built-in ones.
  //
  loadMappingDirectory(Paths::dataDir("inputmaps"), false);

  // we want to watch this dir for new files and changed files
  m_watcher->addPath(Paths::dataDir("inputmaps"));

  emit mappingChanged();
  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
QVariantList InputMapping::mapToAction(const QString& source, const QString& keycode)
{
  // if the source is direct we will just use the keycode as the action
  if (source == "direct")
    return { QVariant(keycode) };

  QVariantList strActions;

  // first we need to match the source
  for (auto src : m_sourceMatcher.match(source))
    strActions << m_inputMatcher.value(src.toString())->match(keycode);

  return strActions;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool InputMapping::loadMappingFile(const QString& path, QPair<QString, QVariantMap> &mappingPair)
{
  QJsonParseError err;
  auto doc = Utils::OpenJsonDocument(path, &err);
  if (doc.isNull())
  {
    qWarning() << "Failed to parse input mapping file:" << path << "," << err.errorString();
    return false;
  }

  if (doc.isObject())
  {
    auto obj = doc.object();
    if (!obj.contains("name"))
    {
      qWarning() << "Missing elements 'name' from mapping file:" << path;
      return false;
    }

    if (!obj.contains("idmatcher"))
    {
      // Missing idmatcher is valid for disabled mapping files
      return false;
    }

    if (!obj.contains("mapping"))
    {
      qWarning() << "Missing element 'mapping' from mapping file:" << path;
      return false;
    }

    mappingPair = qMakePair(obj["name"].toString(), obj.toVariantMap());
    return true;
  }

  qWarning() << "Wrong format for file:" << path;
  return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool InputMapping::loadMappingDirectory(const QString& path, bool copy)
{
  qInfo() << "Loading inputmaps from:" << path;
  QDirIterator it(path);
  while (it.hasNext())
  {
    QFileInfo finfo = QFileInfo(it.next());
    if (finfo.isFile() && finfo.isReadable() && finfo.fileName().endsWith(".json"))
    {
      // make a copy of the original file to the example directory
      if (copy)
      {
        QDir userdir(Paths::dataDir());
        userdir.mkpath("inputmaps/examples/");
        QString examplePath(userdir.filePath("inputmaps/examples/" + finfo.fileName()));

        // make sure we really overwrite the file. copy will not do this.
        if (QFile(examplePath).exists())
          QFile::remove(examplePath);

        QFile::copy(finfo.absoluteFilePath(), examplePath);
        QFile(examplePath).setPermissions(QFileDevice::ReadOwner | QFileDevice::ReadGroup | QFileDevice::WriteOwner |
                                            QFileDevice::WriteGroup | QFileDevice::ReadOther);
      }


      QPair<QString, QVariantMap> mapping;
      if (loadMappingFile(finfo.absoluteFilePath(), mapping))
      {
        // add the source regexp to the matcher
        if (m_sourceMatcher.addMatcher(mapping.second.value("idmatcher").toString(), mapping.first))
        {
          // get the input map and add it to a new CachedMatcher
          QVariantMap inputMap = mapping.second.value("mapping").toMap();
          auto inputMatcher = new CachedRegexMatcher(true, this);
          for(const QString& pattern : inputMap.keys())
            inputMatcher->addMatcher("^" + pattern + "$", inputMap.value(pattern));

          m_inputMatcher.insert(mapping.first, inputMatcher);
        }
      }
    }
  }

  return true;
}
