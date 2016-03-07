#include <locale.h>

#include <QGuiApplication>
#include <QFileInfo>
#include <QIcon>
#include <QtQml>
#include <QtWebEngine/qtwebengineglobal.h>
#include <shared/Names.h>

#include "system/SystemComponent.h"
#include "system/UpdateManager.h"
#include "QsLog.h"
#include "Paths.h"
#include "player/PlayerComponent.h"
#include "breakpad/CrashDumps.h"
#include "Version.h"
#include "settings/SettingsComponent.h"
#include "settings/SettingsSection.h"
#include "ui/KonvergoWindow.h"
#include "ui/KonvergoEngine.h"
#include "UniqueApplication.h"
#include "utils/HelperLauncher.h"
#include "utils/Log.h"

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
#include "SignalManager.h"
#endif

/////////////////////////////////////////////////////////////////////////////////////////
static void preinitQt()
{
  QCoreApplication::setApplicationName(Names::MainName());
  QCoreApplication::setApplicationVersion(Version::GetVersionString());
  QCoreApplication::setOrganizationDomain("plex.tv");

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
  foreach(const QString& str, args)
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
int main(int argc, char *argv[])
{
  try
  {
    QCommandLineParser parser;
    parser.setApplicationDescription("Plex Media Player");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOptions({{{"l", "licenses"}, "Show license information"}});

    char **newArgv = appendCommandLineArguments(argc, argv, {"--enable-viewport", "--enable-viewport-meta"});
    argc += 2;

#ifdef KONVERGO_OPENELEC
    newArgv = appendCommandLineArguments(argc, newArgv, {"--disable-gpu"});
    argc ++;
#endif

    // Suppress SSL related warnings on OSX
    // See https://bugreports.qt.io/browse/QTBUG-43173 for more info
    //
#ifdef Q_OS_MAC
    qputenv("QT_LOGGING_RULES", "qt.network.ssl.warning=false");

    // Request OpenGL 4.1 if possible on OSX, otherwise it defaults to 2.0
    // This needs to be done before we create the QGuiApplication
    //
    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setMajorVersion(3);
    format.setMinorVersion(2);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);
#endif

    preinitQt();

    QGuiApplication app(argc, newArgv);
    app.setWindowIcon(QIcon(":/images/icon.png"));

    // Get the arguments from the app, this is the parsed version of newArgc and newArgv
    QStringList args = app.arguments();

    // Remove the viewport arguments so that the parser doesn't barf
    args.removeAll("--enable-viewport");
    args.removeAll("--enable-viewport-meta");

    // Now parse the command line.
    parser.process(args);

    if (parser.isSet("licenses"))
    {
      ShowLicenseInfo();
      return EXIT_SUCCESS;
    }

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

    // Quit app and apply update if we find one.
    if (UpdateManager::CheckForUpdates())
    {
      app.quit();
      return 0;
    }

#ifdef Q_OS_WIN32
    initD3DDevice();
#endif

#ifdef Q_OS_UNIX
    setlocale(LC_NUMERIC, "C");
#endif

    // Initialize all the components. This needs to be done
    // early since most everything else relies on it
    //
    ComponentManager::Get().initialize();

    // enable remote inspection if we have the correct setting for it.
    if (SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "remoteInspector").toBool())
      qputenv("QTWEBENGINE_REMOTE_DEBUGGING", "0.0.0.0:9992");

    QtWebEngine::initialize();

    // Qt and QWebEngineProfile set the locale, which breaks parsing and
    // formatting float numbers in a few countries.
#ifdef Q_OS_UNIX
    setlocale(LC_NUMERIC, "C");
#endif

    // start our helper
    HelperLauncher::Get().connectToHelper();

    // load QtWebChannel so that we can register our components with it.
    QQmlApplicationEngine *engine = KonvergoEngine::Get();
    KonvergoWindow::RegisterClass();
    engine->rootContext()->setContextProperty("components", &ComponentManager::Get().getQmlPropertyMap());

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
    engine->rootContext()->setContextProperty("webMaxHeight", 1440);

    // the only way to detect if QML parsing fails is to hook to this signal and then see
    // if we get a valid object passed to it. Any error messages will be reported on stderr
    // but since no normal user should ever see this it should be fine
    //
    QObject::connect(engine, &QQmlApplicationEngine::objectCreated, [=](QObject* object, const QUrl& url)
    {
      Q_UNUSED(url);

      if (object == 0)
        throw FatalException(QObject::tr("Failed to parse application engine script."));

      QObject* rootObject = engine->rootObjects().first();

      QObject* webChannel = qvariant_cast<QObject*>(rootObject->property("webChannel"));
      Q_ASSERT(webChannel);
      ComponentManager::Get().setWebChannel(qobject_cast<QWebChannel*>(webChannel));

      KonvergoWindow* window = qobject_cast<KonvergoWindow*>(rootObject);
      Q_ASSERT(window);
      QObject::connect(uniqueApp, &UniqueApplication::otherApplicationStarted, window, &KonvergoWindow::otherAppFocus);

    });
    engine->load(QUrl(QStringLiteral("qrc:/ui/webview.qml")));

    Log::UpdateLogLevel();

    // run our application
    int ret = app.exec();

    delete KonvergoEngine::Get();
    delete uniqueApp;

    return ret;
  }
  catch (FatalException& e)
  {

    QLOG_FATAL() << "Unhandled FatalException:" << qPrintable(e.message());

    QGuiApplication app(argc, argv);
    QString text = e.message() + "<br>" + QObject::tr("Please visit Plex support forums for support.");

    QQmlApplicationEngine* engine = new QQmlApplicationEngine(NULL);
    engine->rootContext()->setContextProperty("errorTitle", QObject::tr("A critical error occurred."));
    engine->rootContext()->setContextProperty("errorText", text);
    engine->load(QUrl(QStringLiteral("qrc:/ui/errormessage.qml")));

    app.exec();
    return 1;

  }
}
