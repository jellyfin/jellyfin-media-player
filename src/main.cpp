#include <QGuiApplication>
#include <QApplication>
#include <QFileInfo>
#include <QIcon>
#include <QtQml>
#include <Qt>
#include <QtWebEngineQuick>
#include <qtwebenginecoreglobal.h>
#include <QtWebEngineCore/QWebEngineProfile>
#include <QErrorMessage>
#include <QtWebEngineCore/QWebEngineScript>
#include <QCommandLineOption>
#include <QDebug>
#include <QSettings>

#include "shared/Names.h"
#include "system/SystemComponent.h"
#include <QDebug>
#include "Paths.h"
#include "player/CodecsComponent.h"
#include "player/PlayerComponent.h"
#include "player/OpenGLDetect.h"
#include "display/DisplayComponent.h"
#include "Version.h"
#include "settings/SettingsComponent.h"
#include "settings/SettingsSection.h"
#include "ui/EventFilter.h"
#include "ui/WindowManager.h"
#include "Globals.h"
#include "ui/ErrorMessage.h"
#include "UniqueApplication.h"
#include "utils/Log.h"

#ifdef Q_OS_MAC
#include "PFMoveApplication.h"
#endif

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
#include "SignalManager.h"
#endif

/////////////////////////////////////////////////////////////////////////////////////////
static void preinitQt()
{
  QCoreApplication::setApplicationName(Names::MainName());
  QCoreApplication::setApplicationVersion(Version::GetVersionString());
  QCoreApplication::setOrganizationDomain("jellyfin.org");

#ifdef Q_OS_WIN32
  QVariant useOpengl = SettingsComponent::readPreinitValue(SETTINGS_SECTION_MAIN, "useOpenGL");

  // Warning: this must be the same as the default value as declared in
  // the settings_description.json file, or confusion will result.
  if (useOpengl.type() != QMetaType::Bool)
    useOpengl = false;

  if (useOpengl.toBool())
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
  else
    QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////
char** appendCommandLineArguments(int argc, char **argv, const QStringList& args)
{
  size_t newSize = static_cast<size_t>(argc + args.length() + 1) * sizeof(char*);
  char** newArgv = static_cast<char**>(calloc(1, newSize));
  memcpy(newArgv, argv, static_cast<size_t>(argc) * sizeof(char*));

  int pos = argc;
  for(const QString& str : args)
    newArgv[pos++] = qstrdup(str.toUtf8().data());

  return newArgv;
}

/////////////////////////////////////////////////////////////////////////////////////////
void ShowLicenseInfo()
{
  QFile licenses(":/misc/licenses.txt");
  if (!licenses.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    fprintf(stderr, "Error: Could not open license file\n");
    return;
  }
  QByteArray contents = licenses.readAll();
  printf("%.*s\n", static_cast<int>(contents.size()), contents.data());
}

/////////////////////////////////////////////////////////////////////////////////////////
QStringList g_qtFlags = {
  "--enable-gpu-rasterization",
  "--disable-features=MediaSessionService"
};

/////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
  try
  {
    QCommandLineParser parser;
    parser.setApplicationDescription("Jellyfin Desktop");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOptions({{{"l", "licenses"},         "Show license information"},
                       {"desktop",                 "Start in desktop mode"},
                       {"tv",                      "Start in TV mode"},
                       {"windowed",                "Start in windowed mode"},
                       {"fullscreen",              "Start in fullscreen"},
                       {"disable-gpu",             "Disable QtWebEngine gpu accel"},
                       {"force-external-webclient","Use webclient provided by server"}});

    auto scaleOption = QCommandLineOption("scale-factor", "Set to a integer or default auto which controls" \
                                                          "the scale (DPI) of the desktop interface.");
    scaleOption.setValueName("scale");
    scaleOption.setDefaultValue("auto");

    auto platformOption = QCommandLineOption("platform", "Equivalant to QT_QPA_PLATFORM.");
    platformOption.setValueName("platform");
    platformOption.setDefaultValue("default");

    auto devOption = QCommandLineOption("remote-debugging-port", "Port number for devtools.");
    devOption.setValueName("port");

    auto configDirOption = QCommandLineOption("config-dir", "Override config directory path.");
    configDirOption.setValueName("path");

    auto logLevelOption = QCommandLineOption("log-level", "Log level: debug, info, warn, error, fatal (default: error)");
    logLevelOption.setValueName("level");

    parser.addOption(scaleOption);
    parser.addOption(devOption);
    parser.addOption(platformOption);
    parser.addOption(configDirOption);
    parser.addOption(logLevelOption);

    char **newArgv = appendCommandLineArguments(argc, argv, g_qtFlags);
    int newArgc = argc + g_qtFlags.size();

    preinitQt();
    detectOpenGLEarly();
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    QStringList arguments;
    for (int i = 0; i < argc; i++)
      arguments << QString::fromLatin1(argv[i]);

    {
      // This is kinda dumb. But in order for the QCommandLineParser
      // to work properly we need to init if before we call process
      // but we don't want to do that for the main application since
      // we need to set the scale factor before we do that. So it becomes
      // a small chicken-or-egg problem, which we "solve" by making
      // this temporary console app.
      //
      QCoreApplication core(newArgc, newArgv);

      // Now parse the command line.
      parser.process(arguments);
    }

    if (parser.isSet("licenses"))
    {
      ShowLicenseInfo();
      return EXIT_SUCCESS;
    }

    QString logLevel = parser.value("log-level");
    if (parser.isSet("log-level") && (logLevel.isEmpty() || Log::ParseLogLevel(logLevel) == -1))
    {
      fprintf(stderr, "Error: invalid log level '%s'. Valid levels: debug, info, warn, error, fatal\n", qPrintable(logLevel));
      return EXIT_FAILURE;
    }

    if (parser.isSet("log-level"))
      Log::SetLogLevel(logLevel);

    Log::Init();

    auto scale = parser.value("scale-factor");
    if (scale.isEmpty() || scale == "auto")
    {
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
      QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    }
    else if (scale != "none")
    {
      qputenv("QT_SCALE_FACTOR", scale.toUtf8());
    }

    auto platform = parser.value("platform");
    if (!(platform.isEmpty() || platform == "default"))
    {
      qputenv("QT_QPA_PLATFORM", platform.toUtf8());
    }
#if defined(Q_OS_UNIX) && !defined(Q_OS_DARWIN) && !defined(Q_OS_ANDROID) && QT_VERSION < QT_VERSION_CHECK(6, 5, 0)
    // Force xcb on Qt < 6.5 due to mpvqt wayland requiring >=6.5
    else if (platform.isEmpty() || platform == "default")
    {
      qputenv("QT_QPA_PLATFORM", "xcb");
    }
#endif

    auto configDir = parser.value("config-dir");
    QString webEngineDataDir;
    if (!configDir.isEmpty())
    {
      QFileInfo fi(configDir);
      QString absPath = fi.absoluteFilePath();
      QDir parentDir = fi.dir();

      if (!parentDir.exists())
      {
        qFatal("Config directory parent does not exist: %s", qPrintable(parentDir.absolutePath()));
      }

      Paths::setConfigDir(absPath);
      QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, absPath);
      webEngineDataDir = absPath + "/QtWebEngine";
    }
    else
    {
      // Use Paths::dataDir() equivalent inline to avoid double nesting
      QDir d(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation));
      d.mkpath(d.absolutePath() + "/" + Names::DataName());
      d.cd(Names::DataName());
      webEngineDataDir = d.absolutePath() + "/QtWebEngine";
    }

    QStringList chromiumFlags;
#ifdef Q_OS_LINUX
    // Disable QtWebEngine's automatic MPRIS registration - we handle it ourselves
    chromiumFlags << "--disable-features=MediaSessionService,HardwareMediaKeyHandling";
#endif
    // Disable pinch-to-zoom if browser zoom is not allowed
    QVariant allowZoom = SettingsComponent::readPreinitValue(SETTINGS_SECTION_MAIN, "allowBrowserZoom");
    if (allowZoom.isValid() && !allowZoom.toBool())
      chromiumFlags << "--disable-pinch";

    if (!chromiumFlags.isEmpty())
      qputenv("QTWEBENGINE_CHROMIUM_FLAGS", chromiumFlags.join(" ").toUtf8());

    if (parser.isSet("remote-debugging-port"))
      qputenv("QTWEBENGINE_REMOTE_DEBUGGING", parser.value("remote-debugging-port").toUtf8());

    QtWebEngineQuick::initialize();
    QApplication app(newArgc, newArgv);

#if defined(Q_OS_WIN) 
    // Setting window icon on OSX will break user ability to change it
    app.setWindowIcon(QIcon(":/images/icon.png"));
#endif

#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
  	// Set window icon on Linux using system icon theme
  	app.setWindowIcon(QIcon::fromTheme("org.jellyfin.JellyfinDesktop", QIcon(":/images/icon.png")));
    // Set app id for Wayland compositor window icon
    app.setDesktopFileName("org.jellyfin.JellyfinDesktop");
#endif

#if defined(Q_OS_MAC) && defined(NDEBUG)
    PFMoveToApplicationsFolderIfNecessary();
#endif

    UniqueApplication* uniqueApp = new UniqueApplication();
    if (!uniqueApp->ensureUnique())
    {
      Log::Cleanup();
      return EXIT_SUCCESS;
    }

    Log::RotateLog();

    qInfo() << "Config directory:" << qPrintable(Paths::dataDir());

#ifdef Q_OS_UNIX
    // install signals handlers for proper app closing.
    SignalManager signalManager(&app);
    Q_UNUSED(signalManager);
#endif

    detectOpenGLLate();

    Codecs::preinitCodecs();

    // Initialize all the components. This needs to be done
    // early since most everything else relies on it
    //
    ComponentManager::Get().initialize();

    Log::ApplyConfigLogLevel();

    SettingsComponent::Get().setCommandLineValues(parser.optionNames());

    // Configure QtWebEngine paths
    QWebEngineProfile* defaultProfile = QWebEngineProfile::defaultProfile();
    defaultProfile->setCachePath(webEngineDataDir);
    defaultProfile->setPersistentStoragePath(webEngineDataDir);

    // load QtWebChannel so that we can register our components with it.
    QQmlApplicationEngine *engine = Globals::Engine();

    Globals::SetContextProperty("components", &ComponentManager::Get().getQmlPropertyMap());

    // the only way to detect if QML parsing fails is to hook to this signal and then see
    // if we get a valid object passed to it. Any error messages will be reported on stderr
    // but since no normal user should ever see this it should be fine
    //
    QObject::connect(engine, &QQmlApplicationEngine::objectCreated, [&](QObject* object, const QUrl& url)
    {
      Q_UNUSED(url);

      if (object == nullptr)
        throw FatalException(QObject::tr("Failed to parse application engine script."));

      QQuickWindow* window = Globals::MainWindow();

      // Set window flags for proper popup handling (e.g., WebEngineView dropdowns)
      window->setFlags(window->flags() | Qt::WindowFullscreenButtonHint);

      // Install event filter for proper event handling
      window->installEventFilter(new EventFilter(window));

      // Install application event filter to catch popup window creation early
      class PopupFixer : public QObject {
        QQuickWindow* m_mainWindow;
      public:
        PopupFixer(QQuickWindow* mainWin) : m_mainWindow(mainWin) {}
        bool eventFilter(QObject* obj, QEvent* event) override {
          auto* win = qobject_cast<QWindow*>(obj);
          if (!win || win == m_mainWindow) {
            return QObject::eventFilter(obj, event);
          }

          // Fix WebEngineView popup flags to accept focus
          if (event->type() == QEvent::Show) {
            Qt::WindowFlags flags = win->flags();

            // Only fix WebEngineView dropdowns (Tool + FramelessWindowHint + WindowStaysOnTopHint)
            // Don't touch other windows (e.g., MPV-related)
            bool isWebEnginePopup = (flags & Qt::Tool) &&
                                     (flags & Qt::FramelessWindowHint) &&
                                     (flags & Qt::WindowStaysOnTopHint);

            if (!isWebEnginePopup) {
              return QObject::eventFilter(obj, event);
            }

            if (win->transientParent() == nullptr) {
              win->setTransientParent(m_mainWindow);
            }

            if (win->modality() != Qt::NonModal) {
              win->setModality(Qt::NonModal);
            }

            // WebEngineView creates popups with Qt::Tool | WindowDoesNotAcceptFocus
            // which prevents interaction. Change to Qt::Popup to accept focus.
            flags &= ~Qt::Tool;
            flags |= Qt::Popup;
            flags &= ~Qt::WindowDoesNotAcceptFocus;
            win->setFlags(flags);
          }

          return QObject::eventFilter(obj, event);
        }
      };
      app.installEventFilter(new PopupFixer(window));

      QObject* webChannel = qvariant_cast<QObject*>(window->property("webChannel"));
      Q_ASSERT(webChannel);
      ComponentManager::Get().setWebChannel(qobject_cast<QWebChannel*>(webChannel));

      // Initialize WindowManager with window reference
      WindowManager::Get().initializeWindow(window);

      // Handle other app focus by raising window
      QObject::connect(uniqueApp, &UniqueApplication::otherApplicationStarted, []() {
        WindowManager::Get().raiseWindow();
      });
    });
    engine->load(QUrl(QStringLiteral("qrc:/ui/webview.qml")));

    // run our application
    int ret = app.exec();

    delete uniqueApp;
    Globals::EngineDestroy();

    Codecs::Uninit();
    return ret;
  }
  catch (FatalException& e)
  {
    qFatal("Unhandled FatalException: %s", qPrintable(e.message()));
    QApplication errApp(argc, argv);

    auto  msg = new ErrorMessage(e.message(), true);
    msg->show();

    errApp.exec();

    Codecs::Uninit();
    return 1;
  }
}
