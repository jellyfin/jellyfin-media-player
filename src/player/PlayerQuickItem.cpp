#include "PlayerQuickItem.h"

#include <stdexcept>

#include <QOpenGLContext>

#include <QtGui/QOpenGLFramebufferObject>

#include <QtQuick/QQuickWindow>
#include <QOpenGLFunctions>

#include "QsLog.h"
#include "utils/Utils.h"

#ifdef Q_OS_WIN32

#include <windows.h>
#include <d3d9.h>

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
static void* __stdcall MPGetD3DInterface(const char* name)
{
  QLOG_INFO() << "Asking for " << qPrintable(QString::fromUtf8((name)));
  if (strcmp(name, "IDirect3DDevice9") == 0)
  {
    QLOG_INFO() << "Returning device " << (void *)d3ddevice;
    IDirect3DDevice9_AddRef(d3ddevice);
    return (void *)d3ddevice;
  }
  return NULL;
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
static void* get_proc_address(void* ctx, const char* name)
{
  Q_UNUSED(ctx);

  QOpenGLContext* glctx = QOpenGLContext::currentContext();
  if (!glctx)
    return NULL;

  void *res = (void *)glctx->getProcAddress(QByteArray(name));
#ifdef Q_OS_WIN32
  if (strcmp(name, "glMPGetD3DInterface") == 0)
  {
    return (void *)&MPGetD3DInterface;
  }
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

///////////////////////////////////////////////////////////////////////////////////////////////////
PlayerRenderer::PlayerRenderer(mpv::qt::Handle mpv, QQuickWindow* window)
: m_mpv(mpv), m_mpvGL(0), m_window(window), m_size()
{
  m_mpvGL = (mpv_opengl_cb_context *)mpv_get_sub_api(m_mpv, MPV_SUB_API_OPENGL_CB);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool PlayerRenderer::init()
{
  const char *extensions = "";
#ifdef Q_OS_WIN32
  // For the custom IDirect3DDevice9 hack above.
  extensions = "GL_MP_D3D_interfaces";
#endif
  return mpv_opengl_cb_init_gl(m_mpvGL, extensions, get_proc_address, NULL) >= 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
PlayerRenderer::~PlayerRenderer()
{
  // Keep in mind that the m_mpv handle must be held until this is done.
  if (m_mpvGL)
    mpv_opengl_cb_uninit_gl(m_mpvGL);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerRenderer::render()
{
  int fbo = m_window->renderTargetId();

  m_window->resetOpenGLState();

  // The negative height signals to mpv that the video should be flipped
  // (according to the flipped OpenGL coordinate system).
  mpv_opengl_cb_draw(m_mpvGL, fbo, m_size.width(), -m_size.height());

  m_window->resetOpenGLState();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerRenderer::swap()
{
  mpv_opengl_cb_report_flip(m_mpvGL, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
PlayerQuickItem::PlayerQuickItem(QQuickItem* parent)
: QQuickItem(parent), m_mpvGL(NULL), m_renderer(NULL)
{
  connect(this, &QQuickItem::windowChanged, this, &PlayerQuickItem::onWindowChanged, Qt::DirectConnection);
  connect(this, &PlayerQuickItem::onFatalError, this, &PlayerQuickItem::onHandleFatalError, Qt::QueuedConnection);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
PlayerQuickItem::~PlayerQuickItem()
{
  if (m_mpvGL)
    mpv_opengl_cb_set_update_callback(m_mpvGL, NULL, NULL);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerQuickItem::onWindowChanged(QQuickWindow* win)
{
  if (win)
  {
    connect(win, &QQuickWindow::beforeSynchronizing, this, &PlayerQuickItem::onSynchronize, Qt::DirectConnection);
    connect(win, &QQuickWindow::sceneGraphInvalidated, this, &PlayerQuickItem::onInvalidate, Qt::DirectConnection);
    connect(this, &PlayerQuickItem::onUpdate, win, &QQuickWindow::update, Qt::QueuedConnection);
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
      m_renderer = NULL;
      emit onFatalError(tr("Could not initialize OpenGL."));
      return;
    }
    connect(window(), &QQuickWindow::beforeRendering, m_renderer, &PlayerRenderer::render, Qt::DirectConnection);
    connect(window(), &QQuickWindow::frameSwapped, m_renderer, &PlayerRenderer::swap, Qt::DirectConnection);
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
    m_renderer->m_size = window()->size() * window()->devicePixelRatio();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerQuickItem::onInvalidate()
{
  if (m_renderer)
    delete m_renderer;
  m_renderer = NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerQuickItem::on_update(void *ctx)
{
  PlayerQuickItem *self = (PlayerQuickItem *)ctx;
  emit self->onUpdate();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerQuickItem::initMpv(PlayerComponent* player)
{
  m_mpv = player->getMpvHandle();

  m_mpvGL = (mpv_opengl_cb_context *)mpv_get_sub_api(m_mpv, MPV_SUB_API_OPENGL_CB);
  if (!m_mpvGL)
    throw FatalException(tr("OpenGL not enabled in libmpv."));

  mpv_opengl_cb_set_update_callback(m_mpvGL, on_update, (void *)this);

  connect(player, &PlayerComponent::windowVisible, this, &QQuickItem::setVisible);
  window()->update();
}
