#include "MprisPlayerAdaptor.h"
#include "MprisComponent.h"

MprisPlayerAdaptor::MprisPlayerAdaptor(MprisComponent* parent)
  : QDBusAbstractAdaptor(parent)
  , m_component(parent)
{
}

QString MprisPlayerAdaptor::playbackStatus() const
{
  return m_component->playbackStatus();
}

QString MprisPlayerAdaptor::loopStatus() const
{
  return m_component->loopStatus();
}

void MprisPlayerAdaptor::setLoopStatus(const QString& value)
{
  m_component->setLoopStatus(value);
}

double MprisPlayerAdaptor::rate() const
{
  return m_component->rate();
}

void MprisPlayerAdaptor::setRate(double value)
{
  m_component->setRate(value);
}

bool MprisPlayerAdaptor::shuffle() const
{
  return m_component->shuffle();
}

void MprisPlayerAdaptor::setShuffle(bool value)
{
  m_component->setShuffle(value);
}

QVariantMap MprisPlayerAdaptor::metadata() const
{
  return m_component->metadata();
}

double MprisPlayerAdaptor::volume() const
{
  return m_component->volume();
}

void MprisPlayerAdaptor::setVolume(double value)
{
  m_component->setVolume(value);
}

qint64 MprisPlayerAdaptor::position() const
{
  return m_component->position();
}

double MprisPlayerAdaptor::minimumRate() const
{
  return m_component->minimumRate();
}

double MprisPlayerAdaptor::maximumRate() const
{
  return m_component->maximumRate();
}

bool MprisPlayerAdaptor::canGoNext() const
{
  return m_component->canGoNext();
}

bool MprisPlayerAdaptor::canGoPrevious() const
{
  return m_component->canGoPrevious();
}

bool MprisPlayerAdaptor::canPlay() const
{
  return m_component->canPlay();
}

bool MprisPlayerAdaptor::canPause() const
{
  return m_component->canPause();
}

bool MprisPlayerAdaptor::canSeek() const
{
  return m_component->canSeek();
}

bool MprisPlayerAdaptor::canControl() const
{
  return m_component->canControl();
}

void MprisPlayerAdaptor::Next()
{
  m_component->Next();
}

void MprisPlayerAdaptor::Previous()
{
  m_component->Previous();
}

void MprisPlayerAdaptor::Pause()
{
  m_component->Pause();
}

void MprisPlayerAdaptor::PlayPause()
{
  m_component->PlayPause();
}

void MprisPlayerAdaptor::Stop()
{
  m_component->Stop();
}

void MprisPlayerAdaptor::Play()
{
  m_component->Play();
}

void MprisPlayerAdaptor::Seek(qint64 offset)
{
  m_component->Seek(offset);
}

void MprisPlayerAdaptor::SetPosition(const QDBusObjectPath& trackId, qint64 position)
{
  m_component->SetPosition(trackId, position);
}

void MprisPlayerAdaptor::OpenUri(const QString& uri)
{
  m_component->OpenUri(uri);
}