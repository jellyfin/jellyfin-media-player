#ifndef KONVERGOWINDOW_H
#define KONVERGOWINDOW_H

#include <QQuickWindow>
#include <QEvent>
#include <settings/SettingsComponent.h>


// This controls how big the web view will zoom using semantic zoom
// over a specific number of pixels and we run out of space for on screen
// tiles in chromium. This only happens on OSX since on other platforms
// we can use the GPU to transfer tiles directly but we set the limit on all platforms
// to keep it consistent.
//
// See more discussion in: https://github.com/plexinc/plex-media-player/issues/10
// The number of pixels here are REAL pixels, the code in webview.qml will compensate
// for a higher DevicePixelRatio
//
#define WEBUI_MAX_HEIGHT 1440.0
#define WEBUI_SIZE QSize(1280, 720)
#define WINDOWW_MIN_SIZE QSize(426, 240)

///////////////////////////////////////////////////////////////////////////////////////////////////
class KonvergoWindow : public QQuickWindow
{
  Q_OBJECT

  Q_PROPERTY(bool fullScreen READ isFullScreen WRITE setFullScreen NOTIFY fullScreenSwitched)
  Q_PROPERTY(bool showDebugLayer MEMBER m_debugLayer NOTIFY debugLayerChanged)
  Q_PROPERTY(QString debugInfo MEMBER m_debugInfo NOTIFY debugInfoChanged)
  Q_PROPERTY(QString videoInfo MEMBER m_videoInfo NOTIFY debugInfoChanged)
  Q_PROPERTY(qreal webScale READ webScale NOTIFY webScaleChanged)
  Q_PROPERTY(qreal webHeightMax READ webHeightMax CONSTANT)
  Q_PROPERTY(qreal windowScale READ windowScale NOTIFY windowScaleChanged)
  Q_PROPERTY(QSize windowMinSize READ windowMinSize CONSTANT)
  Q_PROPERTY(bool alwaysOnTop READ isAlwaysOnTop WRITE setAlwaysOnTop)
  Q_PROPERTY(bool webDesktopMode MEMBER m_webDesktopMode NOTIFY webDesktopModeChanged)
  Q_PROPERTY(QString webUrl READ webUrl NOTIFY webUrlChanged)
  Q_PROPERTY(qreal tvUIWidth READ tvUIWidth NOTIFY tvUIWidthChanged)
  Q_PROPERTY(qreal tvUIHeight READ tvUIHeight NOTIFY tvUIHeightChanged)

public:
  static void RegisterClass();

  explicit KonvergoWindow(QWindow* parent = nullptr);
  ~KonvergoWindow() override;

  bool isFullScreen()
  {
    return ((flags() & Qt::FramelessWindowHint) || (visibility() == QWindow::FullScreen));
  }

  Q_INVOKABLE void setFullScreen(bool enable);

  bool isAlwaysOnTop()
  {
    Qt::WindowFlags forceOnTopFlags = Qt::WindowStaysOnTopHint;
#ifdef Q_OS_LINUX
    forceOnTopFlags = forceOnTopFlags | Qt::X11BypassWindowManagerHint;
#endif
    return (flags() & forceOnTopFlags);
  }

  void setAlwaysOnTop(bool enable);

  Q_SLOT void otherAppFocus()
  {
    setWindowState((Qt::WindowState)((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive));
    raise();
  }

  Q_SLOT void toggleDebug();

  Q_SLOT void toggleFullscreen(bool noSwitchMode = false);

  Q_SLOT void toggleFullscreenNoSwitch() { toggleFullscreen(true); }

  Q_SLOT void toggleAlwaysOnTop()
  {
    setAlwaysOnTop(!isAlwaysOnTop());
  }

  Q_SLOT void reloadWeb()
  {
    emit reloadWebClient();
  }

  Q_INVOKABLE Q_SLOT void minimizeWindow()
  {
    if (!isFullScreen())
      setVisibility(QWindow::Minimized);
  }

  qreal windowScale() { return m_lastWindowScale; }
  qreal webScale() { return m_lastWebScale; }
  qreal webHeightMax() { return WEBUI_MAX_HEIGHT; }
  QSize windowMinSize() { return WINDOWW_MIN_SIZE; }
  QString webUrl();

  qreal tvUIWidth()
  {
    return m_tvUIw;
  }

  qreal tvUIHeight()
  {
    return m_tvUIh;
  }

Q_SIGNALS:
  void fullScreenSwitched();
  void webScaleChanged();
  void windowScaleChanged();
  void enableVideoWindowSignal();
  void debugLayerChanged();
  void debugInfoChanged();
  void reloadWebClient();
  void webDesktopModeChanged();
  void webUrlChanged();
  void tvUIWidthChanged();
  void tvUIHeightChanged();

protected:
  void focusOutEvent(QFocusEvent* ev) override;
  void resizeEvent(QResizeEvent* event) override;

private slots:
  void closingWindow();
  void enableVideoWindow();
  void onVisibilityChanged(QWindow::Visibility visibility);
  void updateMainSectionSettings(const QVariantMap& values);
  void updateWindowState(bool saveGeo = true);
  void updateDebugInfo();
  void playerWindowVisible(bool visible);
  void showUpdateDialog();
  void onScreenAdded(QScreen *screen);
  void onScreenRemoved(QScreen *screen);
  void updateCurrentScreen();

private:
  void updateSizeDependendProperties(const QSize& size);
  void saveGeometry();
  QRect loadGeometry();
  QRect loadGeometryRect();
  bool fitsInScreens(const QRect& rc);
  QScreen* loadLastScreen();
  void updateScreens();
  void updateForcedScreen();
  QScreen* findCurrentScreen();

  bool m_debugLayer;
  qreal m_lastWindowScale, m_lastWebScale, m_tvUIw, m_tvUIh;
  QTimer* m_infoTimer;
  QString m_debugInfo, m_systemDebugInfo, m_videoInfo;
  int m_ignoreFullscreenSettingsChange;
  bool m_webDesktopMode;
  bool m_showedUpdateDialog;

  unsigned long m_osxPresentationOptions;
  QString m_currentScreenName;

  void setWebMode(bool newDesktopMode, bool fullscreen);

  static qreal CalculateScale(const QSize& size);
  static qreal CalculateWebScale(const QSize& size, qreal devicePixelRatio);
};

#endif // KONVERGOWINDOW_H
