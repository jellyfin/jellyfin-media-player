#pragma once

#include "include/cef_client.h"
#include "include/cef_context_menu_handler.h"
#include "include/cef_display_handler.h"
#include "include/cef_life_span_handler.h"
#include "include/cef_load_handler.h"
#include "include/cef_render_handler.h"
#include "include/cef_request_handler.h"
#include <functional>
#include <mutex>
#include <vector>

// DMA-BUF plane info for accelerated paint
struct DmaBufPlane {
    int fd;
    uint32_t stride;
    uint64_t offset;
    uint64_t size;
};

struct AcceleratedPaintInfo {
    int width;
    int height;
    uint64_t modifier;
    uint32_t format;  // DRM format
    std::vector<DmaBufPlane> planes;
};

class JellyfinCefHandler : public CefClient,
                           public CefContextMenuHandler,
                           public CefRenderHandler,
                           public CefLifeSpanHandler,
                           public CefLoadHandler,
                           public CefDisplayHandler,
                           public CefRequestHandler {
public:
    using PaintCallback = std::function<void(const void*, int, int)>;
    using PopupPaintCallback = std::function<void(const void*, int, int, int, int, int, int)>; // buffer, w, h, x, y, popup_w, popup_h
    using PopupShowCallback = std::function<void(bool)>;
    using ViewRectCallback = std::function<void(int&, int&)>;
    using LoadStateCallback = std::function<void(bool, bool, bool)>;
    using ConsoleMessageCallback = std::function<void(int level, const std::string&, int, const std::string&)>;
    using WebChannelMessageCallback = std::function<void(const std::string&)>;
    using FullscreenCallback = std::function<void(bool)>;
    using CursorCallback = std::function<void(cef_cursor_type_t)>;
    using AcceleratedPaintCallback = std::function<void(const AcceleratedPaintInfo&)>;

    // Context menu item info
    struct ContextMenuItem {
        int commandId;
        std::string label;
        bool enabled;
        bool isSeparator;
    };
    using ContextMenuCallback = std::function<void(int x, int y, const std::vector<ContextMenuItem>&, CefRefPtr<CefRunContextMenuCallback>)>;

    JellyfinCefHandler() = default;
    JellyfinCefHandler(const JellyfinCefHandler&) = delete;
    JellyfinCefHandler& operator=(const JellyfinCefHandler&) = delete;

    // Setters for callbacks
    void SetPaintCallback(PaintCallback callback);
    void SetPopupPaintCallback(PopupPaintCallback callback);
    void SetPopupShowCallback(PopupShowCallback callback);
    void SetViewRectCallback(ViewRectCallback callback);
    void SetLoadStateCallback(LoadStateCallback callback);
    void SetConsoleMessageCallback(ConsoleMessageCallback callback);
    void SetWebChannelMessageCallback(WebChannelMessageCallback callback);
    void SetContextMenuCallback(ContextMenuCallback callback);
    void SetFullscreenCallback(FullscreenCallback callback);
    void SetCursorCallback(CursorCallback callback);
    void SetAcceleratedPaintCallback(AcceleratedPaintCallback callback);

    // CefClient methods
    CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() override { return this; }
    CefRefPtr<CefRenderHandler> GetRenderHandler() override { return this; }
    CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }
    CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }
    CefRefPtr<CefDisplayHandler> GetDisplayHandler() override { return this; }
    CefRefPtr<CefRequestHandler> GetRequestHandler() override { return this; }
    bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefFrame> frame,
                                  CefProcessId source_process,
                                  CefRefPtr<CefProcessMessage> message) override;

    // CefRenderHandler methods
    void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;
    void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) override;
    void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) override;
    void OnPaint(CefRefPtr<CefBrowser> browser,
                 PaintElementType type,
                 const RectList& dirtyRects,
                 const void* buffer,
                 int width,
                 int height) override;
    void OnAcceleratedPaint(CefRefPtr<CefBrowser> browser,
                            PaintElementType type,
                            const RectList& dirtyRects,
                            const CefAcceleratedPaintInfo& info) override;

    // CefLifeSpanHandler methods
    void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
    void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

    // CefLoadHandler methods
    void OnLoadStart(CefRefPtr<CefBrowser> browser,
                     CefRefPtr<CefFrame> frame,
                     TransitionType transition_type) override;
    void OnLoadEnd(CefRefPtr<CefBrowser> browser,
                   CefRefPtr<CefFrame> frame,
                   int httpStatusCode) override;
    void OnLoadError(CefRefPtr<CefBrowser> browser,
                     CefRefPtr<CefFrame> frame,
                     ErrorCode errorCode,
                     const CefString& errorText,
                     const CefString& failedUrl) override;

    // CefDisplayHandler methods
    bool OnConsoleMessage(CefRefPtr<CefBrowser> browser,
                          cef_log_severity_t level,
                          const CefString& message,
                          const CefString& source,
                          int line) override;
    void OnTitleChange(CefRefPtr<CefBrowser> browser,
                       const CefString& title) override;
    void OnFullscreenModeChange(CefRefPtr<CefBrowser> browser,
                                bool fullscreen) override;
    bool OnCursorChange(CefRefPtr<CefBrowser> browser,
                        CefCursorHandle cursor,
                        cef_cursor_type_t type,
                        const CefCursorInfo& custom_cursor_info) override;

    // CefRequestHandler methods
    bool OnCertificateError(CefRefPtr<CefBrowser> browser,
                            ErrorCode cert_error,
                            const CefString& request_url,
                            CefRefPtr<CefSSLInfo> ssl_info,
                            CefRefPtr<CefCallback> callback) override;

    // CefContextMenuHandler methods
    bool RunContextMenu(CefRefPtr<CefBrowser> browser,
                        CefRefPtr<CefFrame> frame,
                        CefRefPtr<CefContextMenuParams> params,
                        CefRefPtr<CefMenuModel> model,
                        CefRefPtr<CefRunContextMenuCallback> callback) override;

    // Browser access
    CefRefPtr<CefBrowser> GetBrowser() const {
        std::lock_guard<std::mutex> lock(callback_mutex_);
        return browser_;
    }

    // View size management
    void SetViewSize(int width, int height);

private:
    CefRefPtr<CefBrowser> browser_;
    PaintCallback paint_callback_;
    PopupPaintCallback popup_paint_callback_;
    PopupShowCallback popup_show_callback_;
    ViewRectCallback view_rect_callback_;
    LoadStateCallback load_state_callback_;
    ConsoleMessageCallback console_message_callback_;
    WebChannelMessageCallback webchannel_message_callback_;
    ContextMenuCallback context_menu_callback_;
    FullscreenCallback fullscreen_callback_;
    CursorCallback cursor_callback_;
    AcceleratedPaintCallback accelerated_paint_callback_;

    int view_width_ = 1024;
    int view_height_ = 768;

    // Popup tracking
    CefRect popup_rect_;
    bool popup_visible_ = false;

    mutable std::mutex callback_mutex_;

    IMPLEMENT_REFCOUNTING(JellyfinCefHandler);
};
