#include "MprisRootAdaptor.h"
#include "MprisComponent.h"

MprisRootAdaptor::MprisRootAdaptor(MprisComponent* parent)
  : QDBusAbstractAdaptor(parent)
  , m_component(parent)
{
}

bool MprisRootAdaptor::canQuit() const
{
  return m_component->canQuit();
}

bool MprisRootAdaptor::canRaise() const
{
  return m_component->canRaise();
}

bool MprisRootAdaptor::canSetFullscreen() const
{
  return m_component->canSetFullscreen();
}

bool MprisRootAdaptor::fullscreen() const
{
  return m_component->fullscreen();
}

void MprisRootAdaptor::setFullscreen(bool value)
{
  m_component->setFullscreen(value);
}

bool MprisRootAdaptor::hasTrackList() const
{
  return m_component->hasTrackList();
}

QString MprisRootAdaptor::identity() const
{
  return m_component->identity();
}

QString MprisRootAdaptor::desktopEntry() const
{
  return m_component->desktopEntry();
}

QStringList MprisRootAdaptor::supportedUriSchemes() const
{
  return m_component->supportedUriSchemes();
}

QStringList MprisRootAdaptor::supportedMimeTypes() const
{
  return m_component->supportedMimeTypes();
}

void MprisRootAdaptor::Raise()
{
  m_component->Raise();
}

void MprisRootAdaptor::Quit()
{
  m_component->Quit();
}