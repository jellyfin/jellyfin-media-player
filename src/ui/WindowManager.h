#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <QObject>
#include <QQuickWindow>
#include <QRect>
#include <QScreen>
#include <QTimer>
#include <QWindow>
#include "core/ComponentManager.h"

#define WEBUI_MAX_HEIGHT 1440.0
#define WEBUI_SIZE QSize(1280, 720)
#define WINDOWW_MIN_SIZE QSize(213, 120)

class WindowManager : public ComponentBase
{
  Q_OBJECT

public:
  static WindowManager& Get();

  explicit WindowManager(QObject* parent = nullptr);
  ~WindowManager() override;

  const char* componentName() override { return "window"; }
  bool componentExport() override { return true; }
  bool componentInitialize() override { return true; }
  void componentPostInitialize() override;

  // Called from main.cpp after window created
  void initializeWindow(QQuickWindow* window);

  // Always on top
  Q_INVOKABLE void setAlwaysOnTop(bool enable);
  Q_INVOKABLE bool isAlwaysOnTop() const;
  Q_INVOKABLE void toggleAlwaysOnTop();

  // Platform detection
  Q_INVOKABLE bool isWayland() const;

  // Fullscreen
  Q_INVOKABLE void setFullScreen(bool enable);
  Q_INVOKABLE bool isFullScreen() const;

  // Window activation
  Q_INVOKABLE void raiseWindow();

public slots:
  void toggleFullscreen();

private slots:
  void onVisibilityChanged(QWindow::Visibility visibility);
  void onScreenAdded(QScreen* screen);
  void onScreenRemoved(QScreen* screen);
  void onScreenGeometryChanged(const QRect& geometry);
  void onScreenDpiChanged(qreal dpi);
  void updateMainSectionSettings(const QVariantMap& values);
  void updateWindowState(bool saveGeo = true);
  void saveGeometrySlot();

private:
  // Geometry
  void loadGeometry();
  void saveGeometry();
  QRect loadGeometryRect();
  bool fitsInScreens(const QRect& rc);

  // Screens
  void updateScreens();
  void updateCurrentScreen();
  QScreen* findCurrentScreen();
  QScreen* loadLastScreen();
  void updateForcedScreen();

  // Settings
  void connectSettings();
  void applySettings();

  QQuickWindow* m_window;
  QString m_currentScreenName;
  int m_ignoreFullscreenSettingsChange;
  QRect m_normalGeometry;
  bool m_maximized;
  bool m_fullscreen;
  QTimer* m_geometryChangeTimer;
  QRect m_pendingGeometry;
};

#endif // WINDOWMANAGER_H
