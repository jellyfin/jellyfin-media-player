#include "PlayerQuickItem.h"

#include <stdexcept>

#include <QCoreApplication>
#include <QGuiApplication>
#include <QOpenGLContext>
#include <QRunnable>
#include <QSGRendererInterface>
#include <QQuickOpenGLUtils>

#include <QOpenGLFramebufferObject>

#include <QtQuick/QQuickWindow>
#include <QOpenGLFunctions>

#include <mpv/render_gl.h>

#include "QsLog.h"
#include "utils/Utils.h"


#if defined(Q_OS_WIN32)
#include <windows.h>
#include <dwmapi.h>
#include <avrt.h>
#endif

#ifdef USE_X11EXTRAS
#include <QGuiApplication>
#include <qpa/qplatformnativeinterface.h>
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
static void* get_proc_address(void* ctx, const char* name)
{
  Q_UNUSED(ctx);

  QOpenGLContext* glctx = QOpenGLContext::currentContext();
  if (!glctx)
    return nullptr;

  void *res = (void *)glctx->getProcAddress(QByteArray(name));
#ifdef Q_OS_WIN32
  // wglGetProcAddress(), which is used by Qt, does not always resolve all
  // builtin functions with all drivers (only extensions). Qt compensates this
  // for a degree, but does this only for functions Qt happens to need. So
  // we need our own falback as well.
  if (!res)
  {
    HMODULE handle = (HMODULE)QOpenGLContext::openGLModuleHandle();
    if (handle)
      res = (void *)GetProcAddress(handle, name);
  }
#endif
  return res;
}

namespace {

/////////////////////////////////////////////////////////////////////////////////////////
class RequestRepaintJob : public QRunnable
{
public:
  explicit RequestRepaintJob(QQuickWindow *window) : m_window(window) { }

  void run() override
  {
    // QSGThreadedRenderLoop::update has a special code path that will render
    // without syncing the render and GUI threads unless asked elsewhere to support
    // QQuickAnimator animations. This is currently triggered by the fact that
    // QQuickWindow::update() is called from the render thread.
    // This allows continuing rendering video while the GUI thread is busy.
    //
    m_window->update();
  }

private:
  QQuickWindow *m_window;
};

}

///////////////////////////////////////////////////////////////////////////////////////////////////
PlayerRenderer::PlayerRenderer(mpv::qt::Handle mpv, QQuickWindow* window)
: m_mpv(mpv), m_mpvGL(nullptr), m_window(window), m_size(), m_hAvrtHandle(nullptr), m_videoRectangle(-1, -1, -1, -1), m_fbo(0)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool PlayerRenderer::init()
{
#ifdef Q_OS_WIN32
  // Request Multimedia Class Schedule Service.
  DwmEnableMMCSS(TRUE);
#endif

mpv_opengl_init_params opengl_params = {
#ifdef Q_OS_WIN32
      get_proc_address,
      NULL,
#else
      .get_proc_address = get_proc_address,
      .get_proc_address_ctx = NULL,
#endif
};

  const QString platformName = QGuiApplication::platformName();

  mpv_render_param params[] = {
    {MPV_RENDER_PARAM_API_TYPE, (void*)MPV_RENDER_API_TYPE_OPENGL},
    {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &opengl_params},
    {MPV_RENDER_PARAM_INVALID},
    {MPV_RENDER_PARAM_INVALID},
  };
#ifdef USE_X11EXTRAS
  if (platformName.contains("xcb")) {
    params[2].type = MPV_RENDER_PARAM_X11_DISPLAY;
    QNativeInterface::QX11Application *x11AppInfo = qApp->nativeInterface<QNativeInterface::QX11Application>();
    params[2].data = x11AppInfo->display();
  } else if (platformName.contains("wayland")) {
    QPlatformNativeInterface *native = QGuiApplication::platformNativeInterface();
    params[2].type = MPV_RENDER_PARAM_WL_DISPLAY;
    params[2].data = native->nativeResourceForWindow("display", NULL);
  }
#endif
  int err = mpv_render_context_create(&m_mpvGL, m_mpv, params);

  if (err >= 0) {
    mpv_render_context_set_update_callback(m_mpvGL, on_update, (void *)this);
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
PlayerRenderer::~PlayerRenderer()
{
  // Keep in mind that the m_mpv handle must be held until this is done.
  if (m_mpvGL)
    mpv_render_context_free(m_mpvGL);
  m_mpvGL = nullptr;
  delete m_fbo;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerRenderer::render()
{
  QOpenGLContext *context = (QOpenGLContext*) (m_window->rendererInterface()->getResource(m_window, QSGRendererInterface::OpenGLContextResource));
  if (!context)
    return;

  GLint fbo = 0;
  context->functions()->glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo);
  bool flip = true;
#if HAVE_OPTIMALORIENTATION
  flip = !(context->format().orientationFlags() & QSurfaceFormat::MirrorVertically);
#endif
  bool screenFlip = flip;
  QSize fboSize = m_size;
  QOpenGLFramebufferObject *blitFbo = 0;
  QQuickOpenGLUtils::resetOpenGLState();

  m_window->beginExternalCommands();

  // this may help: https://doc.qt.io/qt-6/quick-changes-qt6.html#changes-to-qquick-apis

  // this is a canary. if the window retains textures, this is broken
  // remove this when the window is fixed
  context->functions()->glClearColor(0, 0, 0, 0);
  context->functions()->glClear(GL_COLOR_BUFFER_BIT);

  QRect fullWindow(0, 0, m_size.width(), m_size.height());
  if (m_videoRectangle.width() > 0 && m_videoRectangle.height() > 0 && m_videoRectangle != fullWindow && QOpenGLFramebufferObject::hasOpenGLFramebufferBlit() && QOpenGLFramebufferObject::hasOpenGLFramebufferObjects())
  {
    if (!m_fbo || !m_fbo->isValid() || m_fbo->size() != m_videoRectangle.size())
    {
      delete m_fbo;
      m_fbo = new QOpenGLFramebufferObject(m_videoRectangle.size());
    }
    if (m_fbo && m_fbo->isValid())
    {
      blitFbo = m_fbo;
      fboSize = m_fbo->size();
      fbo = m_fbo->handle();
      flip = false;

      // Need to clear the background manually, since nothing else knows it has to be done.
      context->functions()->glClearColor(0, 0, 0, 0);
      context->functions()->glClear(GL_COLOR_BUFFER_BIT);
    }
  }

  mpv_opengl_fbo mpv_fbo = {
#ifdef Q_OS_WIN32
    fbo,
    fboSize.width(),
    fboSize.height(),
#else
    .fbo = fbo,
    .w = fboSize.width(),
    .h = fboSize.height(),
#endif
  };
  int mpv_flip = flip ? -1 : 0;
  mpv_render_param params[] = {
    {MPV_RENDER_PARAM_OPENGL_FBO, &mpv_fbo},
    {MPV_RENDER_PARAM_FLIP_Y, &mpv_flip},
    {MPV_RENDER_PARAM_INVALID}
  };
  mpv_render_context_render(m_mpvGL, params);
  QQuickOpenGLUtils::resetOpenGLState();

  if (blitFbo)
  {
    QRect dstRect = m_videoRectangle;
    if (screenFlip)
      dstRect = QRect(dstRect.x(), m_size.height() - dstRect.y(), dstRect.width(), dstRect.top() - dstRect.bottom());

    QOpenGLFramebufferObject::blitFramebuffer(0, dstRect, blitFbo, QRect(QPoint(0, 0), blitFbo->size()));
  }

  m_window->endExternalCommands();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerRenderer::swap()
{
  if (m_mpvGL)
    mpv_render_context_report_swap(m_mpvGL);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerRenderer::onVideoPlaybackActive(bool active)
{
#ifdef Q_OS_WIN32
  if (active && !m_hAvrtHandle)
  {
    DWORD handle = 0;
    m_hAvrtHandle = AvSetMmThreadCharacteristicsW(L"Low Latency", &handle);
  }
  else if (!active && m_hAvrtHandle)
  {
    AvRevertMmThreadCharacteristics(m_hAvrtHandle);
    m_hAvrtHandle = 0;
  }
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerRenderer::on_update(void *ctx)
{
  PlayerRenderer *self = (PlayerRenderer *)ctx;
  // QQuickWindow::scheduleRenderJob is expected to be called from the GUI thread but
  // is thread-safe when using the QSGThreadedRenderLoop. We can detect a non-threaded render
  // loop by checking if QQuickWindow::beforeSynchronizing was called from the GUI thread
  // (which affects the QObject::thread() of the PlayerRenderer).
  //
  if (self->thread() == self->m_window->thread())
    QMetaObject::invokeMethod(self->m_window, "update", Qt::QueuedConnection);
  else
    self->m_window->scheduleRenderJob(new RequestRepaintJob(self->m_window), QQuickWindow::NoStage);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
PlayerQuickItem::PlayerQuickItem(QQuickItem* parent)
: QQuickItem(parent), m_mpvGL(nullptr), m_renderer(nullptr)
{
  connect(this, &QQuickItem::windowChanged, this, &PlayerQuickItem::onWindowChanged, Qt::DirectConnection);
  connect(this, &PlayerQuickItem::onFatalError, this, &PlayerQuickItem::onHandleFatalError, Qt::QueuedConnection);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
PlayerQuickItem::~PlayerQuickItem()
{
  if (m_mpvGL)
    mpv_render_context_set_update_callback(m_mpvGL, nullptr, nullptr);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerQuickItem::onWindowChanged(QQuickWindow* win)
{
  if (win)
  {
    connect(win, &QQuickWindow::beforeSynchronizing, this, &PlayerQuickItem::onSynchronize, Qt::DirectConnection);
    connect(win, &QQuickWindow::sceneGraphInvalidated, this, &PlayerQuickItem::onInvalidate, Qt::DirectConnection);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerQuickItem::onHandleFatalError(QString message)
{
  throw FatalException(message);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerQuickItem::onSynchronize()
{
  if (!m_renderer && m_mpv)
  {
    m_renderer = new PlayerRenderer(m_mpv, window());
    if (!m_renderer->init())
    {
      delete m_renderer;
      m_renderer = nullptr;
      emit onFatalError(tr("Could not initialize OpenGL."));
      return;
    }
    connect(window(), &QQuickWindow::beforeRenderPassRecording, m_renderer, &PlayerRenderer::render, Qt::DirectConnection);
    connect(window(), &QQuickWindow::frameSwapped, m_renderer, &PlayerRenderer::swap, Qt::DirectConnection);
    connect(&PlayerComponent::Get(), &PlayerComponent::videoPlaybackActive, m_renderer, &PlayerRenderer::onVideoPlaybackActive, Qt::QueuedConnection);
    connect(&PlayerComponent::Get(), &PlayerComponent::onVideoRecangleChanged, window(), &QQuickWindow::update, Qt::QueuedConnection);
    window()->setPersistentGraphics(true);
    window()->setPersistentSceneGraph(true);
    //window()->setClearBeforeRendering(false);
    m_debugInfo = "";
    QOpenGLContext* glctx = QOpenGLContext::currentContext();
    if (glctx && glctx->isValid())
    {
      m_debugInfo += "\nOpenGL:\n";
      int syms[4] = {GL_VENDOR, GL_RENDERER, GL_VERSION, GL_SHADING_LANGUAGE_VERSION};
      for (auto sym : syms)
      {
        auto s = (char *)glctx->functions()->glGetString(sym);
        if (s)
          m_debugInfo += QString("  ") + QString::fromUtf8(s) + "\n";
      }
      m_debugInfo += "\n";
    }
  }
  if (m_renderer)
  {
    m_renderer->m_size = window()->size() * window()->devicePixelRatio();
    m_renderer->m_videoRectangle = PlayerComponent::Get().videoRectangle();
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerQuickItem::onInvalidate()
{
  if (m_renderer)
    delete m_renderer;
  m_renderer = nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerQuickItem::initMpv(PlayerComponent* player)
{
  m_mpv = player->getMpvHandle();

  connect(player, &PlayerComponent::windowVisible, this, &QQuickItem::setVisible);
  window()->update();
}
