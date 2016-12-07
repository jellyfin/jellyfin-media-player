#include <QtGlobal>
#include <QSurfaceFormat>
#include <QCoreApplication>
#include <QOpenGLContext>

#include <mpv/client.h>
#include <mpv/qthelper.hpp>

#include "QsLog.h"

#include "OpenGLDetect.h"

#if defined(Q_OS_MAC)

///////////////////////////////////////////////////////////////////////////////////////////////////
void detectOpenGLEarly()
{
  // Request OpenGL 4.1 if possible on OSX, otherwise it defaults to 2.0
  // This needs to be done before we create the QGuiApplication
  //
  QSurfaceFormat format = QSurfaceFormat::defaultFormat();
  format.setMajorVersion(3);
  format.setMinorVersion(2);
  format.setProfile(QSurfaceFormat::CoreProfile);
  QSurfaceFormat::setDefaultFormat(format);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void detectOpenGLLate()
{
}

#elif defined(Q_OS_LINUX)

///////////////////////////////////////////////////////////////////////////////////////////////////
// Attempt to reuse mpv's code for detecting whether we want GLX or EGL (which
// is tricky to do because of hardware decoding concerns). This is not pretty,
// but quite effective and without having to duplicate too much GLX/EGL code.
static QString probeHwdecInterop()
{
  auto mpv = mpv::qt::Handle::FromRawHandle(mpv_create());
  if (!mpv)
    return "";
  mpv::qt::set_property(mpv, "hwdec-preload", "auto");
  // Actually creating a window is required. There is currently no way to keep
  // this window hidden or invisible.
  mpv::qt::set_property(mpv, "force-window", true);
  // As a mitigation, put the window in the top/right corner, and make it as
  // small as possible by forcing 1x1 size and removing window borders.
  mpv::qt::set_property(mpv, "geometry", "1x1+0+0");
  mpv::qt::set_property(mpv, "border", false);
  if (mpv_initialize(mpv) < 0)
    return "";
  return mpv::qt::get_property(mpv, "hwdec-interop").toString();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void detectOpenGLEarly()
{
  // The putenv call must happen before Qt initializes its platform stuff.
  if (probeHwdecInterop() == "vaapi-egl")
    qputenv("QT_XCB_GL_INTEGRATION", "xcb_egl");
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void detectOpenGLLate()
{
}

#elif defined(Q_OS_WIN)


///////////////////////////////////////////////////////////////////////////////////////////////////
void detectOpenGLEarly()
{
}

/////////////////////////////////////////////////////////////////////////////////////////
void detectOpenGLLate()
{
  if (!QCoreApplication::testAttribute(Qt::AA_UseOpenGLES))
    return;

  // Workaround for broken QSGDefaultDistanceFieldGlyphCache::resizeTexture in ES 3 mode
  qputenv("QML_USE_GLYPHCACHE_WORKAROUND", "1");

  QList<int> versions = { 3, 2 };
  for (auto version : versions)
  {
    QLOG_INFO() << "Trying GLES version" << version;
    QSurfaceFormat fmt = QSurfaceFormat::defaultFormat();
    fmt.setMajorVersion(version);
#ifdef HAVE_OPTIMALORIENTATION
    fmt.setOption(QSurfaceFormat::UseOptimalOrientation);
#endif
    QOpenGLContext ctx;
    ctx.setFormat(fmt);
    if (ctx.create())
    {
      QLOG_INFO() << "Using GLES version" << version;
      QSurfaceFormat::setDefaultFormat(fmt);
      break;
    }
  }
}

#else

///////////////////////////////////////////////////////////////////////////////////////////////////
void detectOpenGLEarly()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void detectOpenGLLate()
{
}

#endif
