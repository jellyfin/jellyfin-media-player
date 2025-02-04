#ifndef TASKBARCOMPONENTWIN_H
#define TASKBARCOMPONENTWIN_H

#include <wrl.h>
#include <Windows.Media.h>

#include "TaskbarComponent.h"

class TaskbarComponentWin : public TaskbarComponent
{
public:
  TaskbarComponentWin(): TaskbarComponent(nullptr) {}
  ~TaskbarComponentWin() override;
  virtual void setWindow(QQuickWindow* window) override;

private:
  void onPauseClicked();
  void onPrevClicked();
  void onNextClicked();
  void setProgress(quint64 value);
  void setControlsVisible(bool value);
  void setPaused(bool value);
  void initializeMediaTransport(HWND hwnd);
  void onMetaData(const QVariantMap &meta, QUrl baseUrl);
  void setAudioMeta(const QVariantMap &meta);
  void setVideoMeta(const QVariantMap &meta);
  void setThumbnail(const QVariantMap &meta, QUrl baseUrl);
  void playing();
  void stopped();
  void paused();

  HRESULT buttonPressed(ABI::Windows::Media::ISystemMediaTransportControls* sender,
    ABI::Windows::Media::ISystemMediaTransportControlsButtonPressedEventArgs* args);

  bool m_initialized;
  EventRegistrationToken m_buttonPressedToken;
  Microsoft::WRL::ComPtr<ABI::Windows::Media::ISystemMediaTransportControls> m_systemControls;
  Microsoft::WRL::ComPtr<ABI::Windows::Media::ISystemMediaTransportControlsDisplayUpdater> m_displayUpdater;
  Microsoft::WRL::ComPtr<ABI::Windows::Storage::Streams::IRandomAccessStreamReference> m_thumbnail;
};

#endif // TASKBARCOMPONENTWIN_H
