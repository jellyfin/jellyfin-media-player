#ifdef USE_WAYLAND_SUBSURFACE

#include "WaylandMpvPlayer.h"
#include "WaylandVulkanContext.h"

#include <QGuiApplication>
#include <QQuickWindow>
#include <qpa/qplatformnativeinterface.h>

#define VK_USE_PLATFORM_WAYLAND_KHR
#include <vulkan/vulkan.h>
#include <wayland-client.h>
#include <mpv/client.h>
#include <mpv/render_vk.h>

#include "wayland-protocols/color-management-v1-client.h"

#include "QtHelper.h"

#include <QDebug>

#include <clocale>
#include <chrono>
#include <string>

// Color management protocol listeners
struct ColorMgmtContext {
    WaylandMpvPlayer *player;
    wp_image_description_v1 *imageDesc;
    wp_image_description_info_v1 *imageInfo;
    bool ready;
    bool done;
};

static void image_desc_info_luminances(void *data, struct wp_image_description_info_v1 *,
                                        uint32_t min_lum, uint32_t max_lum, uint32_t reference_lum)
{
    auto *ctx = static_cast<ColorMgmtContext*>(data);
    ctx->player->m_displayMinLuminance = min_lum / 10000.0;
    ctx->player->m_displayMaxLuminance = max_lum;
    ctx->player->m_displayRefLuminance = reference_lum;
    qInfo() << "Display luminances: min=" << ctx->player->m_displayMinLuminance
            << "max=" << ctx->player->m_displayMaxLuminance
            << "ref=" << ctx->player->m_displayRefLuminance;
}

static void image_desc_info_done(void *data, struct wp_image_description_info_v1 *info)
{
    auto *ctx = static_cast<ColorMgmtContext*>(data);
    ctx->done = true;
    wp_image_description_info_v1_destroy(info);
}

// Stubs for other events we don't need
static void image_desc_info_icc_file(void *, struct wp_image_description_info_v1 *, int32_t, uint32_t) {}
static void image_desc_info_primaries(void *, struct wp_image_description_info_v1 *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t) {}
static void image_desc_info_primaries_named(void *, struct wp_image_description_info_v1 *, uint32_t) {}
static void image_desc_info_tf_power(void *, struct wp_image_description_info_v1 *, uint32_t) {}
static void image_desc_info_tf_named(void *, struct wp_image_description_info_v1 *, uint32_t tf)
{
    // 1=sRGB, 2=gamma22, 3=ST2084(PQ), 4=HLG, etc.
    qInfo() << "Transfer function:" << tf << (tf == 3 ? "(PQ/HDR)" : tf == 1 ? "(sRGB)" : "");
}
static void image_desc_info_target_primaries(void *, struct wp_image_description_info_v1 *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t) {}
static void image_desc_info_target_luminance(void *data, struct wp_image_description_info_v1 *, uint32_t min_lum, uint32_t max_lum)
{
    auto *ctx = static_cast<ColorMgmtContext*>(data);
    // target_luminance gives actual display HDR range
    ctx->player->m_displayMinLuminance = min_lum / 10000.0;
    ctx->player->m_displayMaxLuminance = max_lum;
    qInfo() << "Target luminance (display HDR): min=" << ctx->player->m_displayMinLuminance
            << "max=" << ctx->player->m_displayMaxLuminance;
}
static void image_desc_info_target_max_cll(void *, struct wp_image_description_info_v1 *, uint32_t) {}
static void image_desc_info_target_max_fall(void *, struct wp_image_description_info_v1 *, uint32_t) {}

static const struct wp_image_description_info_v1_listener s_imageDescInfoListener = {
    .done = image_desc_info_done,
    .icc_file = image_desc_info_icc_file,
    .primaries = image_desc_info_primaries,
    .primaries_named = image_desc_info_primaries_named,
    .tf_power = image_desc_info_tf_power,
    .tf_named = image_desc_info_tf_named,
    .luminances = image_desc_info_luminances,
    .target_primaries = image_desc_info_target_primaries,
    .target_luminance = image_desc_info_target_luminance,
    .target_max_cll = image_desc_info_target_max_cll,
    .target_max_fall = image_desc_info_target_max_fall,
};

static void image_desc_failed(void *, struct wp_image_description_v1 *, uint32_t, const char *msg)
{
    qWarning() << "Image description failed:" << msg;
}

static void image_desc_ready(void *data, struct wp_image_description_v1 *, uint32_t)
{
    auto *ctx = static_cast<ColorMgmtContext*>(data);
    ctx->ready = true;
}

static void image_desc_ready2(void *data, struct wp_image_description_v1 *, uint32_t, uint32_t)
{
    auto *ctx = static_cast<ColorMgmtContext*>(data);
    ctx->ready = true;
}

static const struct wp_image_description_v1_listener s_imageDescListener = {
    .failed = image_desc_failed,
    .ready = image_desc_ready,
    .ready2 = image_desc_ready2,
};

static const struct wl_registry_listener s_registryListener = {
    .global = WaylandMpvPlayer::registryGlobal,
    .global_remove = WaylandMpvPlayer::registryGlobalRemove,
};

WaylandMpvPlayer::WaylandMpvPlayer(QObject *parent)
    : QObject(parent)
    , m_vulkan(new WaylandVulkanContext())
{
    // Stop render thread BEFORE Qt destroys objects (like the example does)
    connect(qApp, &QGuiApplication::aboutToQuit, this, [this]() {
        m_running = false;
    });
}

WaylandMpvPlayer::~WaylandMpvPlayer()
{
    detach();
    // Only delete vulkan context if app is still running
    // During shutdown, Wayland display is already gone - let OS clean up
    bool appClosing = !QGuiApplication::instance() || QGuiApplication::closingDown();
    if (!appClosing) {
        delete m_vulkan;
    }
}

void WaylandMpvPlayer::registryGlobal(void *data, struct wl_registry *registry,
                                       uint32_t name, const char *interface, uint32_t version)
{
    auto *self = static_cast<WaylandMpvPlayer*>(data);
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        self->m_wlCompositor = static_cast<wl_compositor*>(
            wl_registry_bind(registry, name, &wl_compositor_interface, 4));
    } else if (strcmp(interface, wl_subcompositor_interface.name) == 0) {
        self->m_wlSubcompositor = static_cast<wl_subcompositor*>(
            wl_registry_bind(registry, name, &wl_subcompositor_interface, 1));
    } else if (strcmp(interface, wl_output_interface.name) == 0 && !self->m_wlOutput) {
        // Bind first output for HDR capability query
        self->m_wlOutput = static_cast<wl_output*>(
            wl_registry_bind(registry, name, &wl_output_interface, std::min(version, 4u)));
    } else if (strcmp(interface, wp_color_manager_v1_interface.name) == 0) {
        self->m_colorManager = static_cast<wp_color_manager_v1*>(
            wl_registry_bind(registry, name, &wp_color_manager_v1_interface, std::min(version, 2u)));
    }
}

void WaylandMpvPlayer::registryGlobalRemove(void *, struct wl_registry *, uint32_t)
{
}

bool WaylandMpvPlayer::initializeWayland()
{
    QPlatformNativeInterface *native = QGuiApplication::platformNativeInterface();
    m_wlDisplay = static_cast<wl_display*>(native->nativeResourceForIntegration("wl_display"));
    if (!m_wlDisplay) {
        qCritical() << "Failed to get Wayland display from Qt";
        return false;
    }

    struct wl_registry *registry = wl_display_get_registry(m_wlDisplay);
    wl_registry_add_listener(registry, &s_registryListener, this);
    wl_display_roundtrip(m_wlDisplay);

    if (!m_wlCompositor || !m_wlSubcompositor) {
        qCritical() << "Missing Wayland globals";
        return false;
    }

    return true;
}

bool WaylandMpvPlayer::createSubsurface(QQuickWindow *window)
{
    QPlatformNativeInterface *native = QGuiApplication::platformNativeInterface();
    wl_surface *qtSurface = static_cast<wl_surface*>(
        native->nativeResourceForWindow("surface", window));
    if (!qtSurface) {
        qCritical() << "Failed to get Qt's wl_surface";
        return false;
    }

    m_mpvSurface = wl_compositor_create_surface(m_wlCompositor);

    // Note: Color surface created lazily in updateSurfaceColorspace() after
    // Vulkan swapchain is set up, to avoid conflicts with Mesa's color management

    m_mpvSubsurface = wl_subcompositor_get_subsurface(m_wlSubcompositor, m_mpvSurface, qtSurface);

    wl_subsurface_set_position(m_mpvSubsurface, 0, 0);
    wl_subsurface_place_below(m_mpvSubsurface, qtSurface);
    wl_subsurface_set_desync(m_mpvSubsurface);

    // Create color management surface BEFORE Vulkan surface (like mpv does)
    // Must be done here, before vkCreateWaylandSurfaceKHR, or Mesa claims it
    if (m_colorManager) {
        m_colorSurface = wp_color_manager_v1_get_surface(m_colorManager, m_mpvSurface);
        if (m_colorSurface) {
            qInfo() << "Created color management surface";
        }
    }

    wl_surface_commit(m_mpvSurface);
    wl_display_roundtrip(m_wlDisplay);

    qInfo() << "Created mpv subsurface below Qt";
    return true;
}

bool WaylandMpvPlayer::initializeColorManagement()
{
    if (!m_colorManager) {
        qWarning() << "Color manager not available (compositor may not support wp-color-management-v1)";
        return false;
    }

    if (!m_wlOutput) {
        qWarning() << "No wl_output available for HDR capability query";
        return false;
    }

    // Query the OUTPUT's image description (actual display HDR capabilities)
    // not the surface feedback (which defaults to SDR)
    wp_color_management_output_v1 *colorOutput =
        wp_color_manager_v1_get_output(m_colorManager, m_wlOutput);
    if (!colorOutput) {
        qWarning() << "Failed to get color management output";
        return false;
    }

    // Get the output's image description
    wp_image_description_v1 *imageDesc =
        wp_color_management_output_v1_get_image_description(colorOutput);
    if (!imageDesc) {
        qWarning() << "Failed to get output image description";
        wp_color_management_output_v1_destroy(colorOutput);
        return false;
    }

    // Set up context for callbacks
    ColorMgmtContext ctx{};
    ctx.player = this;
    ctx.imageDesc = imageDesc;
    ctx.ready = false;
    ctx.done = false;

    // Listen for ready event
    wp_image_description_v1_add_listener(imageDesc, &s_imageDescListener, &ctx);
    wl_display_roundtrip(m_wlDisplay);

    if (!ctx.ready) {
        qWarning() << "Image description not ready";
        wp_image_description_v1_destroy(imageDesc);
        wp_color_management_output_v1_destroy(colorOutput);
        return false;
    }

    // Get the information from the image description
    wp_image_description_info_v1 *imageInfo =
        wp_image_description_v1_get_information(imageDesc);
    if (!imageInfo) {
        qWarning() << "Failed to get image description info";
        wp_image_description_v1_destroy(imageDesc);
        wp_color_management_output_v1_destroy(colorOutput);
        return false;
    }

    // Listen for luminance info
    wp_image_description_info_v1_add_listener(imageInfo, &s_imageDescInfoListener, &ctx);

    // Process events until done
    while (!ctx.done) {
        wl_display_roundtrip(m_wlDisplay);
    }

    wp_image_description_v1_destroy(imageDesc);
    wp_color_management_output_v1_destroy(colorOutput);

    qInfo() << "Color management initialized successfully";
    return true;
}

void WaylandMpvPlayer::updateSurfaceColorspace(const QString &gamma, const QString &primaries)
{
    if (gamma.isEmpty() || primaries.isEmpty())
        return;

    // Skip if already signaled this exact colorspace
    if (m_signaledGamma == gamma && m_signaledPrimaries == primaries)
        return;

    qInfo() << "Updating surface colorspace: gamma=" << gamma << "primaries=" << primaries;

    // Only signal for HDR content (PQ/HLG) - SDR uses compositor default
    // Compositor may not support sRGB/bt.1886 transfer functions
    if (gamma != "pq" && gamma != "hlg") {
        qInfo() << "SDR content, using compositor default";
        m_signaledGamma = gamma;
        m_signaledPrimaries = primaries;
        return;
    }

    // Map gamma/primaries to Wayland color management protocol values
    uint32_t waylandTf;
    uint32_t waylandPrimaries;

    if (gamma == "pq") {
        waylandTf = WP_COLOR_MANAGER_V1_TRANSFER_FUNCTION_ST2084_PQ;
    } else {
        waylandTf = WP_COLOR_MANAGER_V1_TRANSFER_FUNCTION_HLG;
    }

    if (primaries == "bt.2020") {
        waylandPrimaries = WP_COLOR_MANAGER_V1_PRIMARIES_BT2020;
    } else {
        waylandPrimaries = WP_COLOR_MANAGER_V1_PRIMARIES_SRGB;
    }

    // Update image description on EXISTING color surface (created in createSubsurface)
    // Never recreate swapchain or color surface - that causes protocol errors
    if (m_colorSurface && m_colorManager) {
        // Destroy old image description if any
        if (m_hdrImageDesc) {
            wp_image_description_v1_destroy(m_hdrImageDesc);
            m_hdrImageDesc = nullptr;
        }

        wp_image_description_creator_params_v1 *creator =
            wp_color_manager_v1_create_parametric_creator(m_colorManager);
        if (creator) {
            wp_image_description_creator_params_v1_set_primaries_named(creator, waylandPrimaries);
            wp_image_description_creator_params_v1_set_tf_named(creator, waylandTf);

            // For HDR content (PQ/HLG), set luminance metadata
            // This tells compositor about content's luminance range
            if (gamma == "pq" || gamma == "hlg") {
                // HDR10 typical values: min 0.0001 cd/m², max 1000+ cd/m², ref 203 cd/m²
                // min_lum is in 0.0001 cd/m² units, max/ref in cd/m²
                uint32_t minLum = 1;      // 0.0001 cd/m²
                uint32_t maxLum = 1000;   // 1000 cd/m² (typical HDR10)
                uint32_t refLum = 203;    // Reference white per ITU-R BT.2408
                wp_image_description_creator_params_v1_set_luminances(creator, minLum, maxLum, refLum);

                // Also set mastering display luminance if we have it
                // min in 0.0001 cd/m², max in cd/m²
                wp_image_description_creator_params_v1_set_mastering_luminance(creator, 1, 1000);

                qInfo() << "Set HDR luminances: min=0.0001 max=1000 ref=203";
            }

            m_hdrImageDesc = wp_image_description_creator_params_v1_create(creator);
            if (m_hdrImageDesc) {
                wp_color_management_surface_v1_set_image_description(
                    m_colorSurface, m_hdrImageDesc,
                    WP_COLOR_MANAGER_V1_RENDER_INTENT_PERCEPTUAL);
                wl_surface_commit(m_mpvSurface);
                wl_display_flush(m_wlDisplay);
                qInfo() << "Updated Wayland surface to tf=" << waylandTf
                        << "primaries=" << waylandPrimaries;
            }
        }
    }

    m_signaledGamma = gamma;
    m_signaledPrimaries = primaries;
}

bool WaylandMpvPlayer::initializeMpv()
{
    qInfo() << "=== initializeMpv() called ===";
    setlocale(LC_NUMERIC, "C");

    m_mpv = mpv_create();
    if (!m_mpv) {
        qCritical() << "Failed to create mpv";
        return false;
    }

    mpv_set_option_string(m_mpv, "vo", "libmpv");
    mpv_set_option_string(m_mpv, "msg-level", "all=trace");

    // Match MpvQt settings
    mpv_set_option_string(m_mpv, "force-window", "yes");  // Keep window open between files
    mpv_set_option_string(m_mpv, "osd-level", "0");
    mpv_set_option_string(m_mpv, "demuxer-mkv-probe-start-time", "no");
    mpv_set_option_string(m_mpv, "demuxer-lavf-probe-info", "yes");
    mpv_set_option_string(m_mpv, "audio-fallback-to-null", "yes");
    mpv_set_option_string(m_mpv, "ad-lavc-downmix", "no");
    mpv_set_option_string(m_mpv, "cache-seek-min", "5000");
    mpv_set_option_string(m_mpv, "ytdl", "no");

    // Scaling - match standalone mpv gpu-next defaults
    mpv_set_option_string(m_mpv, "scale", "lanczos");
    mpv_set_option_string(m_mpv, "dscale", "lanczos");
    mpv_set_option_string(m_mpv, "cscale", "lanczos");

    // HDR: Tell mpv to output in PQ/BT.2020 for HDR passthrough
    // Without this, mpv converts to SDR (BT.1886/BT.709) by default
    qInfo() << "HDR capable swapchain:" << m_vulkan->isHdr()
            << "displayMaxLuminance:" << m_displayMaxLuminance;
    if (m_vulkan->isHdr()) {
        mpv_set_option_string(m_mpv, "target-prim", "bt.2020");
        mpv_set_option_string(m_mpv, "target-trc", "pq");
        // Set peak to display max or reasonable default
        double peak = m_displayMaxLuminance > 0 ? m_displayMaxLuminance : 1000;
        mpv_set_option(m_mpv, "target-peak", MPV_FORMAT_DOUBLE, &peak);
        qInfo() << "Set HDR target: bt.2020/pq peak=" << peak;
    }

    if (mpv_initialize(m_mpv) < 0) {
        qCritical() << "Failed to initialize mpv";
        return false;
    }

    // Request log messages through event loop (same as MpvPlayer)
    mpv_request_log_messages(m_mpv, "terminal-default");

    // Observe properties for signals (do this once, not on every file load)
    mpv_observe_property(m_mpv, 0, "playback-time", MPV_FORMAT_DOUBLE);  // not time-pos
    mpv_observe_property(m_mpv, 0, "duration", MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_mpv, 0, "pause", MPV_FORMAT_FLAG);
    mpv_observe_property(m_mpv, 0, "core-idle", MPV_FORMAT_FLAG);
    mpv_observe_property(m_mpv, 0, "cache-buffering-state", MPV_FORMAT_INT64);
    mpv_observe_property(m_mpv, 0, "vo-configured", MPV_FORMAT_FLAG);
    // Observe source colorspace for dynamic color management
    mpv_observe_property(m_mpv, 0, "video-params/gamma", MPV_FORMAT_STRING);
    mpv_observe_property(m_mpv, 0, "video-params/primaries", MPV_FORMAT_STRING);

    // Create render context
    mpv_vulkan_init_params vkParams{};
    vkParams.instance = m_vulkan->instance();
    vkParams.physical_device = m_vulkan->physicalDevice();
    vkParams.device = m_vulkan->device();
    vkParams.graphics_queue = m_vulkan->queue();
    vkParams.graphics_queue_family = m_vulkan->queueFamily();
    vkParams.get_instance_proc_addr = vkGetInstanceProcAddr;
    vkParams.features = m_vulkan->features();
    vkParams.extensions = m_vulkan->deviceExtensions();
    vkParams.num_extensions = m_vulkan->deviceExtensionCount();

    // Don't pass WL_DISPLAY - we handle Wayland color management ourselves
    // (passing it causes mpv to create its own color surface, conflicting with ours)
    int advancedControl = 1;
    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_API_TYPE, const_cast<char*>(MPV_RENDER_API_TYPE_VULKAN)},
        {MPV_RENDER_PARAM_BACKEND, const_cast<char*>("gpu-next")},
        {MPV_RENDER_PARAM_VULKAN_INIT_PARAMS, &vkParams},
        {MPV_RENDER_PARAM_ADVANCED_CONTROL, &advancedControl},
        {MPV_RENDER_PARAM_INVALID, nullptr}
    };

    int result = mpv_render_context_create(&m_renderCtx, m_mpv, params);
    if (result < 0) {
        qCritical() << "Failed to create mpv render context:" << mpv_error_string(result);
        return false;
    }

    // Set up update callback - required for ADVANCED_CONTROL
    mpv_render_context_set_update_callback(m_renderCtx, onMpvUpdate, this);

    qInfo() << "mpv render context created";
    return true;
}

void WaylandMpvPlayer::onMpvUpdate(void *ctx)
{
    auto *self = static_cast<WaylandMpvPlayer*>(ctx);
    self->m_needsUpdate.store(true, std::memory_order_release);
}

bool WaylandMpvPlayer::attachToWindow(QQuickWindow *window)
{
    if (m_window)
        detach();

    m_window = window;

    if (!initializeWayland())
        return false;

    QGuiApplication::processEvents();

    if (!createSubsurface(window))
        return false;

    int width = window->width();
    int height = window->height();

    if (!m_vulkan->initialize(m_wlDisplay, m_mpvSurface))
        return false;

    if (!m_vulkan->createSwapchain(width, height))
        return false;

    // Try to get display HDR characteristics (optional, continues if unavailable)
    initializeColorManagement();

    // Color management surface is created lazily in updateSurfaceColorspace()
    // when we know the actual content colorspace

    if (!initializeMpv())
        return false;

    // Connect resize signals
    connect(window, &QWindow::widthChanged, this, &WaylandMpvPlayer::onWindowWidthChanged);
    connect(window, &QWindow::heightChanged, this, &WaylandMpvPlayer::onWindowHeightChanged);

    // Start render thread
    m_running = true;
    m_renderThread = std::thread(&WaylandMpvPlayer::renderLoop, this);

    return true;
}

void WaylandMpvPlayer::detach()
{
    m_running = false;
    if (m_renderThread.joinable())
        m_renderThread.join();

    // mpv cleanup happens inside renderLoop() after it exits

    // Only cleanup Vulkan/Wayland resources if the app is still running
    // During shutdown, Qt may have already destroyed the Wayland connection
    bool appClosing = !QGuiApplication::instance() || QGuiApplication::closingDown();

    // Clean up color management
    if (!appClosing) {
        if (m_hdrImageDesc) {
            wp_image_description_v1_destroy(m_hdrImageDesc);
            m_hdrImageDesc = nullptr;
        }
        if (m_colorSurface) {
            wp_color_management_surface_v1_destroy(m_colorSurface);
            m_colorSurface = nullptr;
        }
        if (m_colorManager) {
            wp_color_manager_v1_destroy(m_colorManager);
            m_colorManager = nullptr;
        }
    }

    if (!appClosing && m_vulkan) {
        m_vulkan->cleanup();
    }

    if (!appClosing) {
        if (m_mpvSubsurface) {
            wl_subsurface_destroy(m_mpvSubsurface);
            m_mpvSubsurface = nullptr;
        }

        if (m_mpvSurface) {
            wl_surface_destroy(m_mpvSurface);
            m_mpvSurface = nullptr;
        }
    }

    if (m_window) {
        disconnect(m_window, nullptr, this, nullptr);
        m_window = nullptr;
    }
}

void WaylandMpvPlayer::onWindowWidthChanged(int)
{
    if (m_window) {
        m_pendingWidth = m_window->width();
        m_pendingHeight = m_window->height();
        m_needsResize = true;
    }
}

void WaylandMpvPlayer::onWindowHeightChanged(int)
{
    if (m_window) {
        m_pendingWidth = m_window->width();
        m_pendingHeight = m_window->height();
        m_needsResize = true;
    }
}

void WaylandMpvPlayer::renderLoop()
{
    qInfo() << "Render loop started";

    // Create fence for acquire synchronization
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    VkFence acquireFence = VK_NULL_HANDLE;
    vkCreateFence(m_vulkan->device(), &fenceInfo, nullptr, &acquireFence);

    while (m_running) {
        // Handle resize
        if (m_needsResize) {
            int w = m_pendingWidth;
            int h = m_pendingHeight;
            if (w > 0 && h > 0) {
                vkDeviceWaitIdle(m_vulkan->device());
                m_vulkan->recreateSwapchain(w, h);
            }
            m_needsResize = false;
        }

        // Process mpv events
        processEvents();

        if (!m_running)
            break;

        // Wait for update callback to signal new frame available
        if (!m_needsUpdate.load(std::memory_order_acquire)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        m_needsUpdate.store(false, std::memory_order_release);

        // Check what mpv needs
        uint64_t flags = mpv_render_context_update(m_renderCtx);
        if (!(flags & MPV_RENDER_UPDATE_FRAME)) {
            continue;
        }

        // Acquire swapchain image with fence
        uint32_t imageIdx;
        vkResetFences(m_vulkan->device(), 1, &acquireFence);
        VkResult result = vkAcquireNextImageKHR(m_vulkan->device(), m_vulkan->swapchain(),
                                                 100000000, VK_NULL_HANDLE, acquireFence, &imageIdx);
        if (result == VK_TIMEOUT || result == VK_NOT_READY) {
            continue;
        }
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
            continue;
        }
        vkWaitForFences(m_vulkan->device(), 1, &acquireFence, VK_TRUE, UINT64_MAX);

        // Render with mpv - no sync, mpv uses pl_gpu_finish internally
        mpv_vulkan_fbo fbo{};
        fbo.image = m_vulkan->swapchainImages()[imageIdx];
        fbo.image_view = m_vulkan->swapchainViews()[imageIdx];
        fbo.width = m_vulkan->width();
        fbo.height = m_vulkan->height();
        fbo.format = m_vulkan->swapchainFormat();
        fbo.current_layout = VK_IMAGE_LAYOUT_UNDEFINED;
        fbo.target_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        int flipY = 0;
        mpv_render_param renderParams[] = {
            {MPV_RENDER_PARAM_VULKAN_FBO, &fbo},
            {MPV_RENDER_PARAM_FLIP_Y, &flipY},
            {MPV_RENDER_PARAM_INVALID, nullptr}
        };

        mpv_render_context_render(m_renderCtx, renderParams);

        // Present
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        VkSwapchainKHR swapchain = m_vulkan->swapchain();
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapchain;
        presentInfo.pImageIndices = &imageIdx;
        vkQueuePresentKHR(m_vulkan->queue(), &presentInfo);

        // Commit surface to compositor
        wl_surface_commit(m_mpvSurface);
        wl_display_flush(m_wlDisplay);
    }

    vkDeviceWaitIdle(m_vulkan->device());
    if (acquireFence)
        vkDestroyFence(m_vulkan->device(), acquireFence, nullptr);

    // Cleanup mpv inside render loop (like the example does)
    if (m_renderCtx) {
        mpv_render_context_free(m_renderCtx);
        m_renderCtx = nullptr;
    }
    if (m_mpv) {
        mpv_terminate_destroy(m_mpv);
        m_mpv = nullptr;
    }
}

void WaylandMpvPlayer::processEvents()
{
    while (m_mpv) {
        mpv_event *event = mpv_wait_event(m_mpv, 0);
        if (event->event_id == MPV_EVENT_NONE)
            break;

        // Log all events except property-change which is too spammy
        if (event->event_id != MPV_EVENT_PROPERTY_CHANGE) {
            qInfo() << "mpv event:" << mpv_event_name(event->event_id);
        }

        switch (event->event_id) {
        case MPV_EVENT_SHUTDOWN:
            m_running = false;
            break;
        case MPV_EVENT_START_FILE:
            QMetaObject::invokeMethod(this, [this]() {
                emit fileStarted();
            }, Qt::QueuedConnection);
            break;
        case MPV_EVENT_END_FILE:
            // Don't set m_running = false here - that stops the render loop
            // and prevents playing the next file
            QMetaObject::invokeMethod(this, [this]() {
                emit endOfFile();
            }, Qt::QueuedConnection);
            break;
        case MPV_EVENT_LOG_MESSAGE: {
            mpv_event_log_message *msg = static_cast<mpv_event_log_message*>(event->data);
            QString text = QString("[%1] %2").arg(msg->prefix, QString(msg->text).trimmed());
            if (strcmp(msg->level, "error") == 0 || strcmp(msg->level, "fatal") == 0)
                qCritical().noquote() << text;
            else if (strcmp(msg->level, "warn") == 0)
                qWarning().noquote() << text;
            else
                qInfo().noquote() << text;
            break;
        }
        case MPV_EVENT_PROPERTY_CHANGE: {
            mpv_event_property *prop = static_cast<mpv_event_property*>(event->data);
            if (strcmp(prop->name, "playback-time") == 0 && prop->format == MPV_FORMAT_DOUBLE) {
                qint64 pos = static_cast<qint64>(*static_cast<double*>(prop->data) * 1000);
                QMetaObject::invokeMethod(this, [this, pos]() {
                    emit positionChanged(pos);
                }, Qt::QueuedConnection);
            } else if (strcmp(prop->name, "duration") == 0 && prop->format == MPV_FORMAT_DOUBLE) {
                qint64 dur = static_cast<qint64>(*static_cast<double*>(prop->data) * 1000);
                QMetaObject::invokeMethod(this, [this, dur]() {
                    emit durationChanged(dur);
                }, Qt::QueuedConnection);
            } else if (strcmp(prop->name, "pause") == 0 && prop->format == MPV_FORMAT_FLAG) {
                bool paused = *static_cast<int*>(prop->data);
                QMetaObject::invokeMethod(this, [this, paused]() {
                    emit playbackStateChanged(!paused);
                }, Qt::QueuedConnection);
            } else if (strcmp(prop->name, "core-idle") == 0 && prop->format == MPV_FORMAT_FLAG) {
                bool idle = *static_cast<int*>(prop->data);
                QMetaObject::invokeMethod(this, [this, idle]() {
                    emit coreIdleChanged(idle);
                }, Qt::QueuedConnection);
            } else if (strcmp(prop->name, "cache-buffering-state") == 0 && prop->format == MPV_FORMAT_INT64) {
                int percent = static_cast<int>(*static_cast<int64_t*>(prop->data));
                QMetaObject::invokeMethod(this, [this, percent]() {
                    emit bufferingChanged(percent);
                }, Qt::QueuedConnection);
            } else if (strcmp(prop->name, "vo-configured") == 0 && prop->format == MPV_FORMAT_FLAG) {
                bool visible = *static_cast<int*>(prop->data);
                QMetaObject::invokeMethod(this, [this, visible]() {
                    emit windowVisibleChanged(visible);
                }, Qt::QueuedConnection);
            } else if (strcmp(prop->name, "video-params/gamma") == 0) {
                if (prop->format == MPV_FORMAT_STRING && prop->data) {
                    const char *gamma = *static_cast<const char**>(prop->data);
                    if (gamma && gamma[0]) {
                        QString newGamma = QString::fromUtf8(gamma);
                        qInfo() << "Content gamma:" << newGamma;
                        m_currentGamma = newGamma;  // Store even if primaries not ready
                        if (!m_currentPrimaries.isEmpty()) {
                            updateSurfaceColorspace(m_currentGamma, m_currentPrimaries);
                        }
                    }
                }
            } else if (strcmp(prop->name, "video-params/primaries") == 0) {
                if (prop->format == MPV_FORMAT_STRING && prop->data) {
                    const char *primaries = *static_cast<const char**>(prop->data);
                    if (primaries && primaries[0]) {
                        QString newPrimaries = QString::fromUtf8(primaries);
                        qInfo() << "Content primaries:" << newPrimaries;
                        m_currentPrimaries = newPrimaries;  // Store even if gamma not ready
                        if (!m_currentGamma.isEmpty()) {
                            updateSurfaceColorspace(m_currentGamma, m_currentPrimaries);
                        }
                    }
                }
            }
            break;
        }
        default:
            break;
        }
    }
}

void WaylandMpvPlayer::loadFile(const QString &path)
{
    if (!m_mpv) {
        qWarning() << "loadFile: m_mpv is null!";
        return;
    }
    qInfo() << "loadFile:" << path;
    const char *cmd[] = {"loadfile", path.toUtf8().constData(), nullptr};
    int err = mpv_command(m_mpv, cmd);
    if (err < 0) {
        qCritical() << "mpv_command loadfile failed:" << mpv_error_string(err);
    }
}

void WaylandMpvPlayer::command(const QVariant &args)
{
    if (!m_mpv)
        return;

    // Use mpv::qt::command which properly handles QVariantMap options
    mpv::qt::command(m_mpv, args);
}

void WaylandMpvPlayer::setProperty(const QString &name, const QVariant &value)
{
    if (!m_mpv)
        return;

    QByteArray nameUtf8 = name.toUtf8();

    if (value.typeId() == QMetaType::Bool) {
        int flag = value.toBool() ? 1 : 0;
        mpv_set_property(m_mpv, nameUtf8.constData(), MPV_FORMAT_FLAG, &flag);
    } else if (value.typeId() == QMetaType::Int || value.typeId() == QMetaType::LongLong) {
        int64_t val = value.toLongLong();
        mpv_set_property(m_mpv, nameUtf8.constData(), MPV_FORMAT_INT64, &val);
    } else if (value.typeId() == QMetaType::Double) {
        double val = value.toDouble();
        mpv_set_property(m_mpv, nameUtf8.constData(), MPV_FORMAT_DOUBLE, &val);
    } else {
        QByteArray valUtf8 = value.toString().toUtf8();
        mpv_set_property_string(m_mpv, nameUtf8.constData(), valUtf8.constData());
    }
}

QVariant WaylandMpvPlayer::getProperty(const QString &name) const
{
    if (!m_mpv)
        return QVariant();

    QByteArray nameUtf8 = name.toUtf8();
    char *result = mpv_get_property_string(m_mpv, nameUtf8.constData());
    if (result) {
        QString str = QString::fromUtf8(result);
        mpv_free(result);
        return str;
    }
    return QVariant();
}

bool WaylandMpvPlayer::isPlaying() const
{
    if (!m_mpv)
        return false;
    int paused = 0;
    mpv_get_property(m_mpv, "pause", MPV_FORMAT_FLAG, &paused);
    return !paused;
}

qint64 WaylandMpvPlayer::position() const
{
    if (!m_mpv)
        return 0;
    double pos = 0;
    mpv_get_property(m_mpv, "time-pos", MPV_FORMAT_DOUBLE, &pos);
    return static_cast<qint64>(pos * 1000);
}

qint64 WaylandMpvPlayer::duration() const
{
    if (!m_mpv)
        return 0;
    double dur = 0;
    mpv_get_property(m_mpv, "duration", MPV_FORMAT_DOUBLE, &dur);
    return static_cast<qint64>(dur * 1000);
}

#endif // USE_WAYLAND_SUBSURFACE
