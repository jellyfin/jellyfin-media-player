#include "PlayerQuickItem.h"

#include <stdexcept>

#include <QCoreApplication>
#include <QOpenGLContext>
#include <QRunnable>

#include <QtGui/QOpenGLFramebufferObject>

#include <QtQuick/QQuickWindow>
#include <QOpenGLFunctions>

#include "QsLog.h"
#include "utils/Utils.h"

#ifdef USE_X11EXTRAS
#include <QX11Info>
#endif

#if defined(Q_OS_WIN32)

#include <windows.h>
#include <d3d9.h>
#include <dwmapi.h>
#include <avrt.h>

typedef IDirect3D9* WINAPI pDirect3DCreate9(UINT);

static IDirect3DDevice9* d3ddevice;

// This must be run before the konvergo main window switches to FS mode.
void initD3DDevice(void)
{
  // Boilerplate for creating a "blank" D3D device.
  // Most of this is copied from FFmpeg (LGPL).
  pDirect3DCreate9 *createD3D = NULL;
  HRESULT hr;
  D3DPRESENT_PARAMETERS d3dpp = {};
  D3DDISPLAYMODE        d3ddm;
  UINT adapter = D3DADAPTER_DEFAULT;

  if (QCoreApplication::testAttribute(Qt::AA_UseOpenGLES))
    return;

  HMODULE d3dlib = LoadLibraryW(L"d3d9.dll");
  if (!d3dlib) {
      QLOG_ERROR() << "Failed to load D3D9 library";
      return;
  }

  createD3D = (pDirect3DCreate9 *)GetProcAddress(d3dlib, "Direct3DCreate9");
  if (!createD3D) {
      QLOG_ERROR() << "Failed to locate Direct3DCreate9";
      return;
  }

  IDirect3D9 *d3d9 = createD3D(D3D_SDK_VERSION);
  if (!d3d9) {
      QLOG_ERROR() << "Failed to create IDirect3D object";
      return;
  }

  IDirect3D9_GetAdapterDisplayMode(d3d9, adapter, &d3ddm);
  d3dpp.Windowed         = TRUE;
  d3dpp.BackBufferWidth  = 640;
  d3dpp.BackBufferHeight = 480;
  d3dpp.BackBufferCount  = 0;
  d3dpp.BackBufferFormat = d3ddm.Format;
  d3dpp.SwapEffect       = D3DSWAPEFFECT_DISCARD;
  d3dpp.Flags            = D3DPRESENTFLAG_VIDEO;

  hr = IDirect3D9_CreateDevice(d3d9, adapter, D3DDEVTYPE_HAL, GetShellWindow(),
                                D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE,
                                &d3dpp, &d3ddevice);
  if (FAILED(hr)) {
    QLOG_ERROR() << "Failed to create Direct3D device";
    return;
  }

  QLOG_INFO() << "Successfully created a Direct3D device";
};

// Special libmpv-specific pseudo extension for better behavior with OpenGL
// fullscreen modes. This is needed with some drivers which do not allow the
// libmpv DXVA code to create a new D3D device.
static void* __stdcall MPGetNativeDisplay(const char* name)
{
  QLOG_INFO() << "Asking for " << qPrintable(QString::fromUtf8((name)));
  if (strcmp(name, "IDirect3DDevice9") == 0)
  {
    QLOG_INFO() << "Returning device " << (void *)d3ddevice;
    if (d3ddevice)
      IDirect3DDevice9_AddRef(d3ddevice);
    return (void *)d3ddevice;
  }
  return NULL;
}
// defined(Q_OS_WIN32)
#elif defined(USE_X11EXTRAS)
// Linux
static void* MPGetNativeDisplay(const char* name)
{
  if (strcmp(name, "x11") == 0)
    return QX11Info::display();
  return nullptr;
}
#else
// Unsupported or not needed. Also, not using Windows-specific calling convention.
static void* MPGetNativeDisplay(const char* name)
{
  return nullptr;
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
static void* get_proc_address(void* ctx, const char* name)
{
  Q_UNUSED(ctx);

  QOpenGLContext* glctx = QOpenGLContext::currentContext();
  if (!glctx)
    return nullptr;

  void *res = (void *)glctx->getProcAddress(QByteArray(name));
  if (strcmp(name, "glMPGetNativeDisplay") == 0)
  {
    return (void *)&MPGetNativeDisplay;
  }
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
  m_mpvGL = (mpv_opengl_cb_context *)mpv_get_sub_api(m_mpv, MPV_SUB_API_OPENGL_CB);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool PlayerRenderer::init()
{
#ifdef Q_OS_WIN32
  // Request Multimedia Class Schedule Service.
  DwmEnableMMCSS(TRUE);
#endif

  mpv_opengl_cb_set_update_callback(m_mpvGL, on_update, (void *)this);

  // Signals presence of MPGetNativeDisplay().
  const char *extensions = "GL_MP_MPGetNativeDisplay";
  return mpv_opengl_cb_init_gl(m_mpvGL, extensions, get_proc_address, nullptr) >= 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
PlayerRenderer::~PlayerRenderer()
{
  // Keep in mind that the m_mpv handle must be held until this is done.
  if (m_mpvGL)
    mpv_opengl_cb_uninit_gl(m_mpvGL);
  delete m_fbo;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerRenderer::render()
{
  QOpenGLContext *context = QOpenGLContext::currentContext();

  GLint fbo = 0;
  context->functions()->glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo);
  bool flip = true;
#if HAVE_OPTIMALORIENTATION
  flip = !(context->format().orientationFlags() & QSurfaceFormat::MirrorVertically);
#endif
  bool screenFlip = flip;
  QSize fboSize = m_size;
  QOpenGLFramebufferObject *blitFbo = 0;

  m_window->resetOpenGLState();

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

  // The negative height signals to mpv that the video should be flipped
  // (according to the flipped OpenGL coordinate system).
  mpv_opengl_cb_draw(m_mpvGL, fbo, fboSize.width(), (flip ? -1 : 1) * fboSize.height());

  m_window->resetOpenGLState();

  if (blitFbo)
  {
    QRect dstRect = m_videoRectangle;
    if (screenFlip)
      dstRect = QRect(dstRect.x(), m_size.height() - dstRect.y(), dstRect.width(), dstRect.top() - dstRect.bottom());

    QOpenGLFramebufferObject::blitFramebuffer(0, dstRect, blitFbo, QRect(QPoint(0, 0), blitFbo->size()));
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerRenderer::swap()
{
  mpv_opengl_cb_report_flip(m_mpvGL, 0);
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
    mpv_opengl_cb_set_update_callback(m_mpvGL, nullptr, nullptr);
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
    connect(window(), &QQuickWindow::beforeRendering, m_renderer, &PlayerRenderer::render, Qt::DirectConnection);
    connect(window(), &QQuickWindow::frameSwapped, m_renderer, &PlayerRenderer::swap, Qt::DirectConnection);
    connect(&PlayerComponent::Get(), &PlayerComponent::videoPlaybackActive, m_renderer, &PlayerRenderer::onVideoPlaybackActive, Qt::QueuedConnection);
    connect(&PlayerComponent::Get(), &PlayerComponent::onVideoRecangleChanged, window(), &QQuickWindow::update, Qt::QueuedConnection);
    window()->setPersistentOpenGLContext(true);
    window()->setPersistentSceneGraph(true);
    window()->setClearBeforeRendering(false);
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

  m_mpvGL = (mpv_opengl_cb_context *)mpv_get_sub_api(m_mpv, MPV_SUB_API_OPENGL_CB);
  if (!m_mpvGL)
    throw FatalException(tr("OpenGL not enabled in libmpv."));

  connect(player, &PlayerComponent::windowVisible, this, &QQuickItem::setVisible);
  window()->update();
}
