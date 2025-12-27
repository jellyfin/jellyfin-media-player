#ifndef WAYLANDMPVPLAYER_H
#define WAYLANDMPVPLAYER_H

#ifdef USE_WAYLAND_SUBSURFACE

#include <QObject>
#include <QPointer>
#include <QVariant>
#include <thread>
#include <atomic>
class QQuickWindow;
class WaylandVulkanContext;

struct wp_color_manager_v1;

struct wl_display;
struct wl_compositor;
struct wl_subcompositor;
struct wl_surface;
struct wl_subsurface;
struct wl_output;

struct mpv_handle;
struct mpv_render_context;

class WaylandMpvPlayer : public QObject
{
    Q_OBJECT

public:
    explicit WaylandMpvPlayer(QObject *parent = nullptr);
    ~WaylandMpvPlayer();

    bool attachToWindow(QQuickWindow *window);
    void detach();

    void loadFile(const QString &path);
    void command(const QVariant &args);
    void setProperty(const QString &name, const QVariant &value);
    QVariant getProperty(const QString &name) const;

    bool isPlaying() const;
    qint64 position() const;
    qint64 duration() const;

signals:
    void fileStarted();
    void playbackStateChanged(bool playing);
    void positionChanged(qint64 pos);
    void durationChanged(qint64 duration);
    void coreIdleChanged(bool idle);
    void bufferingChanged(int percent);
    void windowVisibleChanged(bool visible);
    void endOfFile();
    void error(const QString &message);

public:
    // Wayland registry callbacks (must be public for struct initialization)
    static void registryGlobal(void *data, struct wl_registry *registry,
                               uint32_t name, const char *interface, uint32_t version);
    static void registryGlobalRemove(void *data, struct wl_registry *registry, uint32_t name);

private slots:
    void onWindowWidthChanged(int width);
    void onWindowHeightChanged(int height);

public:
    // Display HDR characteristics (from Wayland color management protocol)
    // Public for callback access
    double m_displayMaxLuminance = 0;
    double m_displayMinLuminance = 0;
    double m_displayRefLuminance = 0;

private:
    bool initializeWayland();
    bool createSubsurface(QQuickWindow *window);
    bool initializeColorManagement();
    void updateSurfaceColorspace(const QString &gamma, const QString &primaries);
    bool initializeMpv();
    void renderLoop();
    void processEvents();

    wp_color_manager_v1 *m_colorManager = nullptr;
    struct wp_color_management_surface_v1 *m_colorSurface = nullptr;
    struct wp_image_description_v1 *m_hdrImageDesc = nullptr;
    QString m_currentGamma;
    QString m_currentPrimaries;
    QString m_signaledGamma;
    QString m_signaledPrimaries;

    QPointer<QQuickWindow> m_window;
    WaylandVulkanContext *m_vulkan = nullptr;

    // Wayland
    wl_display *m_wlDisplay = nullptr;
    wl_compositor *m_wlCompositor = nullptr;
    wl_subcompositor *m_wlSubcompositor = nullptr;
    wl_output *m_wlOutput = nullptr;
    wl_surface *m_mpvSurface = nullptr;
    wl_subsurface *m_mpvSubsurface = nullptr;

    // mpv
    mpv_handle *m_mpv = nullptr;
    mpv_render_context *m_renderCtx = nullptr;

    // Threading
    std::thread m_renderThread;
    std::atomic<bool> m_running{false};
    std::atomic<bool> m_needsResize{false};
    std::atomic<bool> m_needsUpdate{false};
    std::atomic<int> m_pendingWidth{0};
    std::atomic<int> m_pendingHeight{0};

    static void onMpvUpdate(void *ctx);
};

#endif // USE_WAYLAND_SUBSURFACE
#endif // WAYLANDMPVPLAYER_H
