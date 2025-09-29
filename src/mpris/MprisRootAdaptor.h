#ifndef MPRISROOTADAPTOR_H
#define MPRISROOTADAPTOR_H

#include <QDBusAbstractAdaptor>
#include <QStringList>

class MprisComponent;

class MprisRootAdaptor : public QDBusAbstractAdaptor
{
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2")

  // Properties
  Q_PROPERTY(bool CanQuit READ canQuit)
  Q_PROPERTY(bool CanRaise READ canRaise)
  Q_PROPERTY(bool CanSetFullscreen READ canSetFullscreen)
  Q_PROPERTY(bool Fullscreen READ fullscreen WRITE setFullscreen)
  Q_PROPERTY(bool HasTrackList READ hasTrackList)
  Q_PROPERTY(QString Identity READ identity)
  Q_PROPERTY(QString DesktopEntry READ desktopEntry)
  Q_PROPERTY(QStringList SupportedUriSchemes READ supportedUriSchemes)
  Q_PROPERTY(QStringList SupportedMimeTypes READ supportedMimeTypes)

public:
  explicit MprisRootAdaptor(MprisComponent* parent);
  ~MprisRootAdaptor() override = default;

  // Properties
  bool canQuit() const;
  bool canRaise() const;
  bool canSetFullscreen() const;
  bool fullscreen() const;
  void setFullscreen(bool value);
  bool hasTrackList() const;
  QString identity() const;
  QString desktopEntry() const;
  QStringList supportedUriSchemes() const;
  QStringList supportedMimeTypes() const;

public Q_SLOTS:
  // Methods
  void Raise();
  void Quit();

private:
  MprisComponent* m_component;
};

#endif // MPRISROOTADAPTOR_H