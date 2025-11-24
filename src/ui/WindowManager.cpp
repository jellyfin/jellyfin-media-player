#include "WindowManager.h"
#include "core/Globals.h"
#include "settings/SettingsComponent.h"
#include "settings/SettingsSection.h"
#include "player/PlayerComponent.h"
#include "display/DisplayComponent.h"
#include "taskbar/TaskbarComponent.h"
#include "input/InputComponent.h"

#include <QGuiApplication>
#include <QScreen>
#include <QDebug>

///////////////////////////////////////////////////////////////////////////////////////////////////
WindowManager& WindowManager::Get()
{
  static WindowManager instance;
  return instance;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
WindowManager::WindowManager(QObject* parent)
  : ComponentBase(parent),
    m_window(nullptr),
    m_ignoreFullscreenSettingsChange(0),
    m_maximized(false),
    m_fullscreen(false),
    m_geometryChangeTimer(nullptr)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
WindowManager::~WindowManager()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WindowManager::componentPostInitialize()
{
  // Window not available yet - will be initialized via initializeWindow()
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WindowManager::initializeWindow(QQuickWindow* window)
{
  m_window = window;

  if (!m_window)
  {
    qCritical() << "WindowManager: Failed to get main window";
    return;
  }

  // Initialize components that need window reference
  PlayerComponent::Get().setWindow(m_window);
  DisplayComponent::Get().setApplicationWindow(m_window);
  TaskbarComponent::Get().setWindow(m_window);

  // Register host command for fullscreen toggle
  InputComponent::Get().registerHostCommand("fullscreen", this, "toggleFullscreen");

  // Load and apply saved geometry
  loadGeometry();

  // Connect to settings
  connectSettings();

  // Apply initial settings
  applySettings();

  // Setup screen management
  updateScreens();

  // Setup timer for delayed geometry updates (avoid saving during maximize transitions)
  m_geometryChangeTimer = new QTimer(this);
  m_geometryChangeTimer->setSingleShot(true);
  m_geometryChangeTimer->setInterval(200); // 200ms delay
  connect(m_geometryChangeTimer, &QTimer::timeout, this, [this]() {
    // Only commit if still in windowed mode after delay
    if (m_window->visibility() == QWindow::Windowed && !m_maximized && !m_fullscreen)
      m_normalGeometry = m_pendingGeometry;
  });

  // Connect to window signals
  connect(m_window, &QQuickWindow::visibilityChanged,
          this, &WindowManager::onVisibilityChanged);

  // Track geometry changes with delay
  connect(m_window, &QQuickWindow::widthChanged, this, [this]() {
    if (m_window->visibility() == QWindow::Windowed && !m_maximized && !m_fullscreen)
    {
      m_pendingGeometry = m_window->geometry();
      m_geometryChangeTimer->start(); // Restart timer
    }
  });
  connect(m_window, &QQuickWindow::heightChanged, this, [this]() {
    if (m_window->visibility() == QWindow::Windowed && !m_maximized && !m_fullscreen)
    {
      m_pendingGeometry = m_window->geometry();
      m_geometryChangeTimer->start();
    }
  });
  connect(m_window, &QQuickWindow::xChanged, this, [this]() {
    if (m_window->visibility() == QWindow::Windowed && !m_maximized && !m_fullscreen)
    {
      m_pendingGeometry = m_window->geometry();
      m_geometryChangeTimer->start();
    }
  });
  connect(m_window, &QQuickWindow::yChanged, this, [this]() {
    if (m_window->visibility() == QWindow::Windowed && !m_maximized && !m_fullscreen)
    {
      m_pendingGeometry = m_window->geometry();
      m_geometryChangeTimer->start();
    }
  });

  // Connect to application shutdown
  connect(qApp, &QGuiApplication::aboutToQuit,
          this, &WindowManager::saveGeometrySlot);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WindowManager::setAlwaysOnTop(bool enable)
{
  if (!m_window)
    return;

  Qt::WindowFlags flags = m_window->flags();
  Qt::WindowFlags onTopFlags = Qt::WindowStaysOnTopHint;

#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
  // X11 needs bypass hint, Wayland doesn't support it
  if (QGuiApplication::platformName() == "xcb")
    onTopFlags |= Qt::X11BypassWindowManagerHint;
#endif

  if (enable)
    m_window->setFlags(flags | onTopFlags);
  else
    m_window->setFlags(flags & ~onTopFlags);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool WindowManager::isAlwaysOnTop() const
{
  if (!m_window)
    return false;

  Qt::WindowFlags checkFlags = Qt::WindowStaysOnTopHint;

#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
  if (QGuiApplication::platformName() == "xcb")
    checkFlags |= Qt::X11BypassWindowManagerHint;
#endif

  return (m_window->flags() & checkFlags) == checkFlags;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WindowManager::toggleAlwaysOnTop()
{
  setAlwaysOnTop(!isAlwaysOnTop());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool WindowManager::isWayland() const
{
  return QGuiApplication::platformName() == "wayland";
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WindowManager::setFullScreen(bool enable)
{
  if (!m_window)
    return;

  m_window->setVisibility(enable ? QWindow::FullScreen : QWindow::Windowed);

  if (enable)
    updateForcedScreen();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool WindowManager::isFullScreen() const
{
  if (!m_window)
    return false;

  return m_window->visibility() == QWindow::FullScreen;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WindowManager::toggleFullscreen()
{
  setFullScreen(!isFullScreen());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WindowManager::raiseWindow()
{
  if (!m_window)
    return;

  // Restore from minimized state if needed
  if (m_window->windowState() & Qt::WindowMinimized)
    m_window->setWindowState((Qt::WindowState)(m_window->windowState() & ~Qt::WindowMinimized));

  // Raise and request activation
  // Note: Wayland blocks requestActivate() for security (prevents focus stealing)
  // On Wayland, window will be raised but user must click to activate
  m_window->show();
  m_window->raise();
  m_window->requestActivate();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WindowManager::onVisibilityChanged(QWindow::Visibility visibility)
{
  bool isFS = (visibility == QWindow::FullScreen);
  bool wasMaximized = m_maximized;
  bool wasFullscreen = m_fullscreen;

  // Kiosk mode: force back to fullscreen if user tried to exit
  if (!isFS)
  {
    bool forceAlwaysFS = SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "forceAlwaysFS").toBool();
    if (forceAlwaysFS)
    {
      setFullScreen(true);
      return;
    }
  }

  // Wayland: manually restore geometry when un-maximizing
  if (wasMaximized && visibility == QWindow::Windowed)
  {
    qDebug() << "Un-maximizing, restoring to m_normalGeometry:" << m_normalGeometry;
    m_window->setGeometry(m_normalGeometry);
  }

  // Update maximized/fullscreen state AFTER geometry restore (but not when Hidden)
  if (visibility != QWindow::Hidden)
  {
    m_maximized = (visibility == QWindow::Maximized);
    m_fullscreen = isFS;
  }

  // Update setting in-memory only (no disk write) when fullscreen state changes
  if (m_fullscreen != wasFullscreen && (visibility == QWindow::FullScreen || visibility == QWindow::Windowed))
  {
    SettingsSection* section = SettingsComponent::Get().getSection(SETTINGS_SECTION_MAIN);
    if (section)
    {
      section->setValueNoSave("fullscreen", m_fullscreen);
    }

    emit fullScreenSwitched();
  }

  // Save geometry when exiting fullscreen
  if (!isFS)
    saveGeometry();

  // Update window state
  updateWindowState(false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WindowManager::onScreenAdded(QScreen* screen)
{
  if (!screen)
    return;

  qDebug() << "Screen added:" << screen->name();

  // Connect to new screen's signals
  connect(screen, &QScreen::geometryChanged,
          this, &WindowManager::onScreenGeometryChanged);
  connect(screen, &QScreen::logicalDotsPerInchChanged,
          this, &WindowManager::onScreenDpiChanged);

  updateScreens();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WindowManager::onScreenRemoved(QScreen* screen)
{
  qDebug() << "Screen removed:" << (screen ? screen->name() : "unknown");

  updateScreens();

  // Check if window was on removed screen
  if (m_window && screen && m_currentScreenName == screen->name())
  {
    qDebug() << "Window was on removed screen, moving to primary";
    QScreen* primary = QGuiApplication::primaryScreen();
    if (primary)
      m_window->setScreen(primary);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WindowManager::onScreenGeometryChanged(const QRect& geometry)
{
  Q_UNUSED(geometry);
  qDebug() << "Screen geometry changed";
  updateScreens();
  updateWindowState();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WindowManager::onScreenDpiChanged(qreal dpi)
{
  Q_UNUSED(dpi);
  qDebug() << "Screen DPI changed";
  updateScreens();
  updateWindowState();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WindowManager::updateMainSectionSettings(const QVariantMap& values)
{
  // Fullscreen
  if (values.contains("fullscreen"))
  {
    bool fs = values["fullscreen"].toBool();
    if (fs != isFullScreen())
      setFullScreen(fs);
  }

  // Always on top
  if (values.contains("alwaysOnTop"))
  {
    bool onTop = values["alwaysOnTop"].toBool();
    if (onTop != isAlwaysOnTop())
      setAlwaysOnTop(onTop);
  }

  // Web mode
  if (values.contains("webMode"))
  {
    QString mode = values["webMode"].toString();
    bool desktopMode = (mode == "desktop");
    m_window->setProperty("webDesktopMode", desktopMode);
  }

  // Forced screen
  if (values.contains("forceFSScreen"))
  {
    if (isFullScreen())
      updateForcedScreen();
  }

  // Always fullscreen
  if (values.contains("forceAlwaysFS"))
  {
    bool alwaysFS = values["forceAlwaysFS"].toBool();
    if (alwaysFS && !isFullScreen())
      setFullScreen(true);
  }

  // Startup URL
  if (values.contains("startupurl"))
  {
    QString url = values["startupurl"].toString();
    if (!url.isEmpty())
      m_window->setProperty("webUrl", url);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WindowManager::updateWindowState(bool saveGeo)
{
  updateCurrentScreen();

  if (saveGeo)
    saveGeometry();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WindowManager::saveGeometrySlot()
{
  saveGeometry();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WindowManager::loadGeometry()
{
  QRect rect = loadGeometryRect();

  // Validate geometry fits in available screens
  if (!fitsInScreens(rect))
  {
    qDebug() << "Saved geometry doesn't fit in available screens, using defaults";
    QScreen* primary = QGuiApplication::primaryScreen();
    if (primary)
    {
      QRect screenRect = primary->geometry();
      rect = QRect(
        screenRect.x() + (screenRect.width() - WEBUI_SIZE.width()) / 2,
        screenRect.y() + (screenRect.height() - WEBUI_SIZE.height()) / 2,
        WEBUI_SIZE.width(),
        WEBUI_SIZE.height()
      );
    }
    else
    {
      rect = QRect(0, 0, WEBUI_SIZE.width(), WEBUI_SIZE.height());
    }
  }

  // Apply minimum size
  if (rect.width() < WINDOWW_MIN_SIZE.width())
    rect.setWidth(WINDOWW_MIN_SIZE.width());
  if (rect.height() < WINDOWW_MIN_SIZE.height())
    rect.setHeight(WINDOWW_MIN_SIZE.height());

  // Set geometry
  m_window->setGeometry(rect);

  // Store as normal geometry
  m_normalGeometry = rect;

  // Restore maximized state
  m_maximized = SettingsComponent::Get().value(SETTINGS_SECTION_STATE, "maximized").toBool();
  if (m_maximized)
    m_window->setVisibility(QWindow::Maximized);

  // Restore to last screen
  QScreen* lastScreen = loadLastScreen();
  if (lastScreen)
    m_window->setScreen(lastScreen);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WindowManager::saveGeometry()
{
  if (!m_window)
    return;

  // m_normalGeometry is updated by geometry signals, just save it
  QString geoStr = QString("%1,%2,%3,%4")
    .arg(m_normalGeometry.x()).arg(m_normalGeometry.y())
    .arg(m_normalGeometry.width()).arg(m_normalGeometry.height());

  SettingsComponent::Get().setValue(SETTINGS_SECTION_STATE, "geometry", geoStr);
  SettingsComponent::Get().setValue(SETTINGS_SECTION_STATE, "maximized", m_maximized);
  SettingsComponent::Get().setValue(SETTINGS_SECTION_STATE, "lastusedscreen", m_currentScreenName);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QRect WindowManager::loadGeometryRect()
{
  QString geoStr = SettingsComponent::Get().value(SETTINGS_SECTION_STATE, "geometry").toString();

  if (geoStr.isEmpty())
    return QRect(0, 0, WEBUI_SIZE.width(), WEBUI_SIZE.height());

  QStringList parts = geoStr.split(',');
  if (parts.size() != 4)
    return QRect(0, 0, WEBUI_SIZE.width(), WEBUI_SIZE.height());

  return QRect(
    parts[0].toInt(),
    parts[1].toInt(),
    parts[2].toInt(),
    parts[3].toInt()
  );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool WindowManager::fitsInScreens(const QRect& rc)
{
  for (QScreen* screen : QGuiApplication::screens())
  {
    if (screen->geometry().intersects(rc))
      return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WindowManager::updateScreens()
{
  // Connect to all screen signals
  for (QScreen* screen : QGuiApplication::screens())
  {
    connect(screen, &QScreen::geometryChanged,
            this, &WindowManager::onScreenGeometryChanged, Qt::UniqueConnection);
    connect(screen, &QScreen::logicalDotsPerInchChanged,
            this, &WindowManager::onScreenDpiChanged, Qt::UniqueConnection);
  }

  // Connect to app screen add/remove
  connect(qApp, &QGuiApplication::screenAdded,
          this, &WindowManager::onScreenAdded, Qt::UniqueConnection);
  connect(qApp, &QGuiApplication::screenRemoved,
          this, &WindowManager::onScreenRemoved, Qt::UniqueConnection);

  updateCurrentScreen();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WindowManager::updateCurrentScreen()
{
  QScreen* screen = findCurrentScreen();
  if (screen)
  {
    QString newName = screen->name();
    if (newName != m_currentScreenName)
    {
      qDebug() << "Current screen changed:" << m_currentScreenName << "->" << newName;
      m_currentScreenName = newName;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QScreen* WindowManager::findCurrentScreen()
{
  if (!m_window)
    return QGuiApplication::primaryScreen();

  QRect windowRect = m_window->geometry();
  QScreen* bestScreen = nullptr;
  int maxIntersect = 0;

  for (QScreen* screen : QGuiApplication::screens())
  {
    QRect intersect = screen->geometry().intersected(windowRect);
    int area = intersect.width() * intersect.height();
    if (area > maxIntersect)
    {
      maxIntersect = area;
      bestScreen = screen;
    }
  }

  return bestScreen ? bestScreen : QGuiApplication::primaryScreen();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QScreen* WindowManager::loadLastScreen()
{
  QString screenName = SettingsComponent::Get().value(SETTINGS_SECTION_STATE, "lastusedscreen").toString();

  if (screenName.isEmpty())
    return QGuiApplication::primaryScreen();

  for (QScreen* screen : QGuiApplication::screens())
  {
    if (screen->name() == screenName)
    {
      qDebug() << "Restored to last screen:" << screenName;
      return screen;
    }
  }

  qDebug() << "Last screen not found:" << screenName << "using primary";
  return QGuiApplication::primaryScreen();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WindowManager::updateForcedScreen()
{
  QString forcedScreen = SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "forceFSScreen").toString();

  if (forcedScreen.isEmpty() || !isFullScreen())
    return;

  for (QScreen* screen : QGuiApplication::screens())
  {
    if (screen->name() == forcedScreen)
    {
      qDebug() << "Forcing fullscreen to screen:" << forcedScreen;
      m_window->setScreen(screen);
      m_window->setVisibility(QWindow::FullScreen);
      return;
    }
  }

  qDebug() << "Forced screen not found:" << forcedScreen;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WindowManager::connectSettings()
{
  connect(SettingsComponent::Get().getSection(SETTINGS_SECTION_MAIN),
          &SettingsSection::valuesUpdated,
          this, &WindowManager::updateMainSectionSettings);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WindowManager::applySettings()
{
  // Read and apply all settings
  QVariantMap values;
  values["fullscreen"] = SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "fullscreen");
  values["alwaysOnTop"] = SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "alwaysOnTop");
  values["webMode"] = SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "webMode");
  values["forceFSScreen"] = SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "forceFSScreen");
  values["forceAlwaysFS"] = SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "forceAlwaysFS");
  values["startupurl"] = SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "startupurl");

  updateMainSectionSettings(values);
}
