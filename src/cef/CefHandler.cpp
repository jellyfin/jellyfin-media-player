#include "CefHandler.h"
#include "include/cef_app.h"
#include <iostream>
#include <unistd.h>  // for dup()

void JellyfinCefHandler::SetPaintCallback(PaintCallback callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    paint_callback_ = std::move(callback);
}

void JellyfinCefHandler::SetPopupPaintCallback(PopupPaintCallback callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    popup_paint_callback_ = std::move(callback);
}

void JellyfinCefHandler::SetPopupShowCallback(PopupShowCallback callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    popup_show_callback_ = std::move(callback);
}

void JellyfinCefHandler::SetViewRectCallback(ViewRectCallback callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    view_rect_callback_ = std::move(callback);
}

void JellyfinCefHandler::SetLoadStateCallback(LoadStateCallback callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    load_state_callback_ = std::move(callback);
}

void JellyfinCefHandler::SetConsoleMessageCallback(ConsoleMessageCallback callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    console_message_callback_ = std::move(callback);
}

void JellyfinCefHandler::SetWebChannelMessageCallback(WebChannelMessageCallback callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    webchannel_message_callback_ = std::move(callback);
}

void JellyfinCefHandler::SetContextMenuCallback(ContextMenuCallback callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    context_menu_callback_ = std::move(callback);
}

void JellyfinCefHandler::SetFullscreenCallback(FullscreenCallback callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    fullscreen_callback_ = std::move(callback);
}

void JellyfinCefHandler::OnFullscreenModeChange(CefRefPtr<CefBrowser> /*browser*/, bool fullscreen) {
    FullscreenCallback callback;
    {
        std::lock_guard<std::mutex> lock(callback_mutex_);
        callback = fullscreen_callback_;
    }
    if (callback) {
        callback(fullscreen);
    }
}

void JellyfinCefHandler::SetCursorCallback(CursorCallback callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    cursor_callback_ = std::move(callback);
}

bool JellyfinCefHandler::OnCursorChange(CefRefPtr<CefBrowser> /*browser*/,
                                         CefCursorHandle /*cursor*/,
                                         cef_cursor_type_t type,
                                         const CefCursorInfo& /*custom_cursor_info*/) {
    CursorCallback callback;
    {
        std::lock_guard<std::mutex> lock(callback_mutex_);
        callback = cursor_callback_;
    }
    if (callback) {
        callback(type);
        return true;
    }
    return false;
}

void JellyfinCefHandler::SetAcceleratedPaintCallback(AcceleratedPaintCallback callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    accelerated_paint_callback_ = std::move(callback);
}

void JellyfinCefHandler::OnAcceleratedPaint(CefRefPtr<CefBrowser> /*browser*/,
                                             PaintElementType type,
                                             const RectList& /*dirtyRects*/,
                                             const CefAcceleratedPaintInfo& info) {
    // Only handle main view for now (not popups)
    if (type != PET_VIEW) {
        return;
    }

    AcceleratedPaintCallback callback;
    int width, height;
    {
        std::lock_guard<std::mutex> lock(callback_mutex_);
        callback = accelerated_paint_callback_;
        width = view_width_;
        height = view_height_;
    }

    if (callback && info.plane_count > 0) {
        AcceleratedPaintInfo paintInfo;
        paintInfo.width = width;
        paintInfo.height = height;
        paintInfo.modifier = info.modifier;
        paintInfo.format = info.format;

        for (int i = 0; i < info.plane_count; i++) {
            DmaBufPlane plane;
            plane.fd = dup(info.planes[i].fd);  // Duplicate before CEF releases
            plane.stride = info.planes[i].stride;
            plane.offset = info.planes[i].offset;
            plane.size = info.planes[i].size;
            paintInfo.planes.push_back(plane);
        }

        callback(paintInfo);
    }
}

void JellyfinCefHandler::GetViewRect(CefRefPtr<CefBrowser> /*browser*/, CefRect& rect) {
    int width, height;
    ViewRectCallback callback;

    {
        std::lock_guard<std::mutex> lock(callback_mutex_);
        width = view_width_;
        height = view_height_;
        callback = view_rect_callback_;
    }

    if (callback) {
        callback(width, height);
    }

    rect.Set(0, 0, width, height);
}

void JellyfinCefHandler::OnPopupShow(CefRefPtr<CefBrowser> /*browser*/, bool show) {
    PopupShowCallback callback;
    {
        std::lock_guard<std::mutex> lock(callback_mutex_);
        popup_visible_ = show;
        callback = popup_show_callback_;
        if (!show) {
            popup_rect_.Set(0, 0, 0, 0);
        }
    }
    if (callback) {
        callback(show);
    }
}

void JellyfinCefHandler::OnPopupSize(CefRefPtr<CefBrowser> /*browser*/, const CefRect& rect) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    popup_rect_ = rect;
}

void JellyfinCefHandler::OnPaint(CefRefPtr<CefBrowser> /*browser*/,
                                  PaintElementType type,
                                  const RectList& /*dirtyRects*/,
                                  const void* buffer,
                                  int width,
                                  int height) {
    if (type == PET_POPUP) {
        PopupPaintCallback callback;
        CefRect rect;
        {
            std::lock_guard<std::mutex> lock(callback_mutex_);
            callback = popup_paint_callback_;
            rect = popup_rect_;
        }
        if (callback) {
            callback(buffer, width, height, rect.x, rect.y, rect.width, rect.height);
        }
    } else {
        PaintCallback callback;
        {
            std::lock_guard<std::mutex> lock(callback_mutex_);
            callback = paint_callback_;
        }
        if (callback) {
            callback(buffer, width, height);
        }
    }
}

void JellyfinCefHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    browser_ = browser;
}

void JellyfinCefHandler::OnBeforeClose(CefRefPtr<CefBrowser> /*browser*/) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    browser_ = nullptr;
}

void JellyfinCefHandler::OnLoadStart(CefRefPtr<CefBrowser> /*browser*/,
                                      CefRefPtr<CefFrame> frame,
                                      TransitionType /*transition_type*/) {
    if (!frame->IsMain()) {
        return;
    }

    LoadStateCallback callback;
    {
        std::lock_guard<std::mutex> lock(callback_mutex_);
        callback = load_state_callback_;
    }
    if (callback) {
        callback(true, false, false);
    }
}

void JellyfinCefHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    int /*httpStatusCode*/) {
    if (!frame->IsMain()) {
        return;
    }

    // Set focus after page load for autofocus elements
    browser->GetHost()->SetFocus(true);

    bool canGoBack = browser->CanGoBack();
    bool canGoForward = browser->CanGoForward();

    LoadStateCallback callback;
    {
        std::lock_guard<std::mutex> lock(callback_mutex_);
        callback = load_state_callback_;
    }
    if (callback) {
        callback(false, canGoBack, canGoForward);
    }
}

void JellyfinCefHandler::OnLoadError(CefRefPtr<CefBrowser> /*browser*/,
                                      CefRefPtr<CefFrame> /*frame*/,
                                      ErrorCode errorCode,
                                      const CefString& errorText,
                                      const CefString& failedUrl) {
    // Don't display errors for downloads
    if (errorCode == ERR_ABORTED) {
        return;
    }

    std::cerr << "Load error: " << failedUrl.ToString()
              << " (error " << errorCode << ": "
              << errorText.ToString() << ")" << std::endl;
}

bool JellyfinCefHandler::OnConsoleMessage(CefRefPtr<CefBrowser> /*browser*/,
                                           cef_log_severity_t level,
                                           const CefString& message,
                                           const CefString& source,
                                           int line) {
    ConsoleMessageCallback callback;
    {
        std::lock_guard<std::mutex> lock(callback_mutex_);
        callback = console_message_callback_;
    }
    if (callback) {
        callback(level, message.ToString(), line, source.ToString());
        return true;
    }
    return false;
}

void JellyfinCefHandler::OnTitleChange(CefRefPtr<CefBrowser> /*browser*/,
                                        const CefString& /*title*/) {
    // Can be used to update window title if needed
}

bool JellyfinCefHandler::OnCertificateError(CefRefPtr<CefBrowser> /*browser*/,
                                             ErrorCode /*cert_error*/,
                                             const CefString& /*request_url*/,
                                             CefRefPtr<CefSSLInfo> /*ssl_info*/,
                                             CefRefPtr<CefCallback> /*callback*/) {
    // By default, reject certificate errors
    // In the future, could add callback to allow user to override
    return false;
}

void JellyfinCefHandler::SetViewSize(int width, int height) {
    CefRefPtr<CefBrowser> browser;
    {
        std::lock_guard<std::mutex> lock(callback_mutex_);
        if (width > 0 && height > 0) {
            view_width_ = width;
            view_height_ = height;
        }
        browser = browser_;
    }

    if (browser) {
        browser->GetHost()->WasResized();
    }
}

bool JellyfinCefHandler::OnProcessMessageReceived(
    CefRefPtr<CefBrowser> /*browser*/,
    CefRefPtr<CefFrame> /*frame*/,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message) {

    if (source_process != PID_RENDERER) {
        return false;
    }

    const std::string& name = message->GetName().ToString();
    if (name == "webChannelMessage") {
        CefRefPtr<CefListValue> args = message->GetArgumentList();
        if (args->GetSize() >= 1 && args->GetType(0) == VTYPE_STRING) {
            std::string jsonStr = args->GetString(0).ToString();
            WebChannelMessageCallback callback;
            {
                std::lock_guard<std::mutex> lock(callback_mutex_);
                callback = webchannel_message_callback_;
            }
            if (callback) {
                callback(jsonStr);
            }
        }
        return true;
    }

    return false;
}

bool JellyfinCefHandler::RunContextMenu(
    CefRefPtr<CefBrowser> /*browser*/,
    CefRefPtr<CefFrame> /*frame*/,
    CefRefPtr<CefContextMenuParams> params,
    CefRefPtr<CefMenuModel> model,
    CefRefPtr<CefRunContextMenuCallback> callback) {

    ContextMenuCallback menuCallback;
    {
        std::lock_guard<std::mutex> lock(callback_mutex_);
        menuCallback = context_menu_callback_;
    }

    if (!menuCallback) {
        return false;  // Use default handling
    }

    // Build menu items list
    std::vector<ContextMenuItem> items;
    size_t count = model->GetCount();
    for (size_t i = 0; i < count; ++i) {
        ContextMenuItem item;
        item.commandId = model->GetCommandIdAt(i);
        item.label = model->GetLabelAt(i).ToString();
        item.enabled = model->IsEnabledAt(i);
        item.isSeparator = (model->GetTypeAt(i) == MENUITEMTYPE_SEPARATOR);
        items.push_back(item);
    }

    int x = params->GetXCoord();
    int y = params->GetYCoord();

    menuCallback(x, y, items, callback);
    return true;  // We handle it
}
