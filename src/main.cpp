#include <locale.h>

#include <QGuiApplication>
#include <QApplication>
#include <QFileInfo>
#include <QIcon>
#include <QtQml>
#include <QtWebEngine/qtwebengineglobal.h>
#include <QErrorMessage>
#include <QCommandLineOption>

#include "shared/Names.h"
#include "system/SystemComponent.h"
#include "system/UpdateManager.h"
#include "QsLog.h"
#include "Paths.h"
#include "player/CodecsComponent.h"
#include "player/PlayerComponent.h"
#include "player/OpenGLDetect.h"
#include "breakpad/CrashDumps.h"
#include "Version.h"
#include "settings/SettingsComponent.h"
#include "settings/SettingsSection.h"
#include "ui/KonvergoWindow.h"
#include "Globals.h"
#include "ui/ErrorMessage.h"
#include "UniqueApplication.h"
#include "utils/HelperLauncher.h"
#include "utils/Log.h"

#ifdef Q_OS_MAC
#include "PFMoveApplication.h"
#endif

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
#include "SignalManager.h"
#endif

/////////////////////////////////////////////////////////////////////////////////////////
static void preinitQt(const QString& scale)
{
  QCoreApplication::setApplicationName(Names::MainName());
  QCoreApplication::setApplicationVersion(Version::GetVersionString());
  QCoreApplication::setOrganizationDomain("plex.tv");

  if (scale.isEmpty() || scale == "auto")
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  else
    qputenv("QT_SCALE_FACTOR", scale.toUtf8());

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
  size_t newSize = (argc + args.length() + 1) * sizeof(char*);
  char** newArgv = (char**)calloc(1, newSize);
  memcpy(newArgv, argv, (size_t)(argc * sizeof(char*)));

  int pos = argc;
  for(const QString& str : args)
    newArgv[pos++] = qstrdup(str.toUtf8().data());

  return newArgv;
}

/////////////////////////////////////////////////////////////////////////////////////////
void ShowLicenseInfo()
{
  QFile licenses(":/misc/licenses.txt");
  licenses.open(QIODevice::ReadOnly | QIODevice::Text);
  QByteArray contents = licenses.readAll();
  printf("%.*s\n", contents.size(), contents.data());
}

/////////////////////////////////////////////////////////////////////////////////////////
QStringList g_qtFlags = {"--enable-viewport", "--disable-gpu", "--disable-web-security"};

/////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
  try
  {
    QCommandLineParser parser;
    parser.setApplicationDescription("Plex Media Player");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOptions({{{"l", "licenses"},         "Show license information"},
                       {{"a", "from-auto-update"}, "When invoked from auto-update"},
                       {"desktop",                 "Start in desktop mode"},
                       {"tv",                      "Start in TV mode"},
                       {"windowed",                "Start in windowed mode"},
                       {"fullscreen",              "Start in fullscreen"},
                       {"terminal",                "Log to terminal"}});

    auto scaleOption = QCommandLineOption("scale-factor", "Set to a integer or default auto which controls" \
                                                          "the scale (DPI) of the desktop interface.");
    scaleOption.setDefaultValue("auto");
    parser.addOption(scaleOption);

    char **newArgv = appendCommandLineArguments(argc, argv, g_qtFlags);
    argc += g_qtFlags.size();

    // Suppress SSL related warnings on OSX
    // See https://bugreports.qt.io/browse/QTBUG-43173 for more info
    //
#ifdef Q_OS_MAC
    qputenv("QT_LOGGING_RULES", "qt.network.ssl.warning=false");
#endif

    // Qt calls setlocale(LC_ALL, "") in a bunch of places, which breaks
    // float/string processing in mpv and ffmpeg.
#ifdef Q_OS_UNIX
    qputenv("LC_ALL", "C");
    qputenv("LC_NUMERIC", "C");
#endif

    QStringList arguments;
    for (int i = 1; i < argc; i++)
    {
      auto a = QString::fromLatin1(argv[i]);
      if (!g_qtFlags.contains(a))
        arguments << a;
    }

    // Now parse the command line.
    parser.process(arguments);

    if (parser.isSet("licenses"))
    {
      ShowLicenseInfo();
      return EXIT_SUCCESS;
    }

    detectOpenGLEarly();

    QString scale = parser.value("scale-factor");
    preinitQt(scale);

    QApplication app(argc, newArgv);
    app.setWindowIcon(QIcon(":/images/icon.png"));

#if defined(Q_OS_MAC) && defined(NDEBUG)
    PFMoveToApplicationsFolderIfNecessary();
#endif

    // init breakpad.
    setupCrashDumper();

    UniqueApplication* uniqueApp = new UniqueApplication();
    if (!uniqueApp->ensureUnique())
      return EXIT_SUCCESS;

#ifdef Q_OS_UNIX
    // install signals handlers for proper app closing.
    SignalManager signalManager(&app);
    Q_UNUSED(signalManager);
#endif

    Log::Init();
    if (parser.isSet("terminal"))
      Log::EnableTerminalOutput();

    // Quit app and apply update if we find one.
    if (UpdateManager::CheckForUpdates())
    {
      app.quit();
      return 0;
    }

    detectOpenGLLate();

#ifdef Q_OS_WIN32
    initD3DDevice();
#endif

    Codecs::preinitCodecs();

    // Initialize all the components. This needs to be done
    // early since most everything else relies on it
    //
    ComponentManager::Get().initialize();

    SettingsComponent::Get().setCommandLineValues(parser.optionNames());

    // enable remote inspection if we have the correct setting for it.
    if (SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "remoteInspector").toBool())
      qputenv("QTWEBENGINE_REMOTE_DEBUGGING", "0.0.0.0:9992");

    QtWebEngine::initialize();

    // start our helper
#if ENABLE_HELPER
    HelperLauncher::Get().connectToHelper();
#endif
    // load QtWebChannel so that we can register our components with it.
    QQmlApplicationEngine *engine = Globals::Engine();

    KonvergoWindow::RegisterClass();
    Globals::SetContextProperty("components", &ComponentManager::Get().getQmlPropertyMap());

    // the only way to detect if QML parsing fails is to hook to this signal and then see
    // if we get a valid object passed to it. Any error messages will be reported on stderr
    // but since no normal user should ever see this it should be fine
    //
    QObject::connect(engine, &QQmlApplicationEngine::objectCreated, [=](QObject* object, const QUrl& url)
    {
      Q_UNUSED(url);

      if (object == nullptr)
        throw FatalException(QObject::tr("Failed to parse application engine script."));

      KonvergoWindow* window = Globals::MainWindow();

      QObject* webChannel = qvariant_cast<QObject*>(window->property("webChannel"));
      Q_ASSERT(webChannel);
      ComponentManager::Get().setWebChannel(qobject_cast<QWebChannel*>(webChannel));

      QObject::connect(uniqueApp, &UniqueApplication::otherApplicationStarted, window, &KonvergoWindow::otherAppFocus);
    });
    engine->load(QUrl(QStringLiteral("qrc:/ui/webview.qml")));

    Log::UpdateLogLevel();

    // run our application
    int ret = app.exec();

    delete uniqueApp;
    Globals::EngineDestroy();

    Log::Uninit();
    return ret;
  }
  catch (FatalException& e)
  {
    QLOG_FATAL() << "Unhandled FatalException:" << qPrintable(e.message());
    QApplication errApp(argc, argv);

    auto  msg = new ErrorMessage(e.message(), true);
    msg->show();

    errApp.exec();

    Log::Uninit();
    return 1;
  }
}
