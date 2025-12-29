#include "CefView.h"
#include "CefApp.h"

#include <cstring>
#include <QMouseEvent>
#include <QCursor>
#include <QSGSimpleTextureNode>
#include <QQuickWindow>
#include <unistd.h>  // for close()

#ifdef __linux__
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#endif
#include <QKeyEvent>
#include <QWheelEvent>
#include <QFocusEvent>
#include <QHoverEvent>
#include <QTouchEvent>
#include <QGuiApplication>
#include <QStyleHints>
#include <QQuickWindow>
#include <QMenu>
#include <QAction>
#include <QScreen>

#include "include/cef_app.h"
#include "include/cef_browser.h"

CefView::CefView(QQuickItem* parent)
    : QQuickItem(parent)
{
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptHoverEvents(true);
    setAcceptTouchEvents(true);
    setFocus(true);
    setFlag(ItemAcceptsInputMethod, true);
    setFlag(ItemHasContents, true);  // Required for updatePaintNode

    // Create handler with callbacks
    m_handler = new JellyfinCefHandler();

    m_handler->SetPaintCallback([this](const void* buffer, int w, int h) {
        // Validate dimensions
        if (!buffer || w <= 0 || h <= 0 || w > 16384 || h > 16384) {
            qWarning() << "CefView: invalid paint buffer" << w << "x" << h;
            return;
        }

        // Create image and copy buffer directly (single memcpy instead of two copies)
        QImage newImage(w, h, QImage::Format_ARGB32);
        memcpy(newImage.bits(), buffer, static_cast<size_t>(w) * h * 4);

        // Mark dirty and ensure vsync timer is running
        m_frameDirty.store(true, std::memory_order_relaxed);

        // Marshal to Qt main thread
        QMetaObject::invokeMethod(this, [this, img = std::move(newImage), w, h]() mutable {
            m_viewWidth = w;
            m_viewHeight = h;
            m_image = std::move(img);
            if (!m_vsyncTimer.isActive()) {
                m_vsyncTimer.start();
            }
        }, Qt::QueuedConnection);
    });

    m_handler->SetLoadStateCallback([this](bool isLoading, bool canGoBack, bool canGoForward) {
        QMetaObject::invokeMethod(this, [this, isLoading, canGoBack, canGoForward]() {
            onLoadStateChanged(isLoading, canGoBack, canGoForward);
        }, Qt::QueuedConnection);
    });

    m_handler->SetConsoleMessageCallback([this](int level, const std::string& message, int lineNumber, const std::string& sourceId) {
        QString msg = QString::fromStdString(message);
        QString src = QString::fromStdString(sourceId);
        QMetaObject::invokeMethod(this, [this, level, msg, lineNumber, src]() {
            emit javaScriptConsoleMessage(level, msg, lineNumber, src);
        }, Qt::QueuedConnection);
    });

    m_handler->SetWebChannelMessageCallback([this](const std::string& jsonStr) {
        QString json = QString::fromStdString(jsonStr);
        QMetaObject::invokeMethod(this, [this, json]() {
            if (m_webChannelTransport) {
                m_webChannelTransport->onMessageFromJs(json);
            } else {
                m_pendingWebChannelMessages.append(json);
            }
        }, Qt::QueuedConnection);
    });

    m_handler->SetPopupPaintCallback([this](const void* buffer, int w, int h, int x, int y, int popupW, int popupH) {
        if (!buffer || w <= 0 || h <= 0 || w > 16384 || h > 16384) {
            return;
        }

        QImage newImage(w, h, QImage::Format_ARGB32);
        memcpy(newImage.bits(), buffer, static_cast<size_t>(w) * h * 4);
        QRect rect(x, y, popupW, popupH);

        QMetaObject::invokeMethod(this, [this, img = std::move(newImage), rect]() mutable {
            m_popupImage = std::move(img);
            m_popupRect = rect;
            update();
        }, Qt::QueuedConnection);
    });

    m_handler->SetPopupShowCallback([this](bool show) {
        QMetaObject::invokeMethod(this, [this, show]() {
            m_popupVisible = show;
            if (!show) {
                m_popupImage = QImage();
                m_popupRect = QRect();
            }
            update();
        }, Qt::QueuedConnection);
    });

    m_handler->SetContextMenuCallback([this](int x, int y,
            const std::vector<JellyfinCefHandler::ContextMenuItem>& items,
            CefRefPtr<CefRunContextMenuCallback> callback) {
        QMetaObject::invokeMethod(this, [this, x, y, items, callback]() {
            showContextMenu(x, y, items, callback);
        }, Qt::QueuedConnection);
    });

    m_handler->SetFullscreenCallback([this](bool fullscreen) {
        QMetaObject::invokeMethod(this, [this, fullscreen]() {
            emit fullScreenRequested(fullscreen);
        }, Qt::QueuedConnection);
    });

    m_handler->SetCursorCallback([this](cef_cursor_type_t type) {
        QMetaObject::invokeMethod(this, [this, type]() {
            updateCursor(type);
        }, Qt::QueuedConnection);
    });

    m_handler->SetAcceleratedPaintCallback([this](const AcceleratedPaintInfo& info) {
        // Queue DMA-BUF for import on render thread (no update() call - vsync timer handles it)
        queueDmaBuf(info);
    });

    // Provide current size directly from window when CEF asks
    m_handler->SetViewRectCallback([this](int& w, int& h) {
        if (window()) {
            qreal dpr = window()->devicePixelRatio();
            w = static_cast<int>(window()->width() * dpr);
            h = static_cast<int>(window()->height() * dpr);
        }
    });

}

CefView::~CefView()
{

#ifdef __linux__
    // Clean up GL texture
    if (m_glTexture != 0) {
        glDeleteTextures(1, &m_glTexture);
        m_glTexture = 0;
    }
    m_eglImporter.cleanup();
#endif

    if (m_handler) {
        // Clear callbacks before closing to prevent use-after-free
        m_handler->SetPaintCallback(nullptr);
        m_handler->SetPopupPaintCallback(nullptr);
        m_handler->SetPopupShowCallback(nullptr);
        m_handler->SetViewRectCallback(nullptr);
        m_handler->SetLoadStateCallback(nullptr);
        m_handler->SetConsoleMessageCallback(nullptr);
        m_handler->SetWebChannelMessageCallback(nullptr);
        m_handler->SetContextMenuCallback(nullptr);
        m_handler->SetFullscreenCallback(nullptr);
        m_handler->SetCursorCallback(nullptr);
        m_handler->SetAcceleratedPaintCallback(nullptr);

        auto browser = m_handler->GetBrowser();
        if (browser) {
            browser->GetHost()->CloseBrowser(true);
        }
    }

    delete m_webChannelTransport;
}

void CefView::setWebChannel(QWebChannel* channel)
{
    if (m_webChannel == channel) {
        return;
    }

    m_webChannel = channel;

    // Create transport when we have both browser and web channel
    if (m_handler && m_handler->GetBrowser() && m_webChannel && !m_webChannelTransport) {
        m_webChannelTransport = new CefWebChannelTransport(m_handler->GetBrowser(), this);
        m_webChannel->connectTo(m_webChannelTransport);
        // Flush any queued messages
        for (const QString& msg : m_pendingWebChannelMessages) {
            m_webChannelTransport->onMessageFromJs(msg);
        }
        m_pendingWebChannelMessages.clear();
    }

    emit webChannelChanged();
}

void CefView::setUrl(const QUrl& url)
{
    if (m_url == url) {
        return;
    }

    m_url = url;

    // Only create browser after componentComplete when QML layout is done
    if (!m_browserCreated && !m_url.isEmpty() && m_componentComplete) {
        createBrowser();
    } else if (m_handler && m_handler->GetBrowser()) {
        m_handler->GetBrowser()->GetMainFrame()->LoadURL(m_url.toString().toStdString());
    }

    emit urlChanged();
}

void CefView::setZoomFactor(qreal factor)
{
    if (qFuzzyCompare(m_zoomFactor, factor)) {
        return;
    }

    m_zoomFactor = factor;

    if (m_handler && m_handler->GetBrowser()) {
        m_handler->GetBrowser()->GetHost()->SetZoomLevel(std::log(m_zoomFactor) / std::log(1.2));
    }

    emit zoomFactorChanged();
}

void CefView::setBackgroundColor(const QColor& color)
{
    if (m_backgroundColor == color) {
        return;
    }

    m_backgroundColor = color;
    emit backgroundColorChanged();
}

void CefView::setUserAgent(const QString& userAgent)
{
    if (m_userAgent == userAgent) {
        return;
    }

    m_userAgent = userAgent;
    emit userAgentChanged();
}

void CefView::setStoragePath(const QString& path)
{
    if (m_storagePath == path) {
        return;
    }

    m_storagePath = path;
    emit storagePathChanged();
}

void CefView::setSettingsJson(const QString& json)
{
    if (m_settingsJson == json) {
        return;
    }

    m_settingsJson = json;

    // Pass settings to CEF app for renderer subprocess
    JellyfinCefApp::SetSettingsJson(json.toStdString());

    emit settingsJsonChanged();
}

void CefView::setNativeshellScript(const QString& script)
{
    if (m_nativeshellScript == script) {
        return;
    }

    m_nativeshellScript = script;

    // Pass script to CEF app for renderer subprocess
    JellyfinCefApp::SetNativeShellScript(script.toStdString());

    emit nativeshellScriptChanged();
}

void CefView::reload()
{
    if (m_handler && m_handler->GetBrowser()) {
        m_handler->GetBrowser()->Reload();
    }
}

void CefView::triggerWebAction(int action)
{
    if (!m_handler || !m_handler->GetBrowser()) {
        return;
    }

    auto frame = m_handler->GetBrowser()->GetMainFrame();
    if (!frame) {
        return;
    }

    // Map WebEngine actions to CEF operations
    // These constants should match WebEngineView action values
    switch (action) {
        case 0: // Back
            if (m_handler->GetBrowser()->CanGoBack()) {
                m_handler->GetBrowser()->GoBack();
            }
            break;
        case 1: // Forward
            if (m_handler->GetBrowser()->CanGoForward()) {
                m_handler->GetBrowser()->GoForward();
            }
            break;
        case 2: // Stop
            m_handler->GetBrowser()->StopLoad();
            break;
        case 3: // Reload
            m_handler->GetBrowser()->Reload();
            break;
        case 16: // Copy
            frame->Copy();
            break;
        case 17: // Cut
            frame->Cut();
            break;
        case 18: // Paste
            frame->Paste();
            break;
        case 20: // Undo
            frame->Undo();
            break;
        case 21: // Redo
            frame->Redo();
            break;
        case 22: // SelectAll
            frame->SelectAll();
            break;
        default:
            break;
    }
}

void CefView::forceActiveFocus()
{
    setFocus(true);
    if (m_handler && m_handler->GetBrowser()) {
        m_handler->GetBrowser()->GetHost()->SetFocus(true);
    }
}

void CefView::injectScript(const QString& code)
{
    if (!m_handler || !m_handler->GetBrowser()) {
        return;
    }

    auto frame = m_handler->GetBrowser()->GetMainFrame();
    if (frame) {
        frame->ExecuteJavaScript(code.toStdString(), frame->GetURL(), 0);
    }
}

void CefView::componentComplete()
{
    QQuickItem::componentComplete();
    m_componentComplete = true;

    // Set up vsync timer - only runs when frames are being delivered
    int refreshRate = getScreenRefreshRate();
    int interval = refreshRate > 0 ? 1000 / refreshRate : 16;  // Default 60Hz
    m_vsyncTimer.setInterval(interval);
    connect(&m_vsyncTimer, &QTimer::timeout, this, [this]() {
        if (m_frameDirty.exchange(false)) {
            m_idleTicks = 0;
            update();
        } else {
            // No new frame - stop after a few idle ticks
            if (++m_idleTicks > 10) {
                m_vsyncTimer.stop();
            }
        }
    });
    // Timer starts when first frame arrives (see paint callbacks)

    // Try to create browser if we already have valid size
    if (!m_browserCreated && !m_url.isEmpty() && width() > 0 && height() > 0) {
        createBrowser();
    }
}

QSGNode* CefView::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* /*data*/)
{
    QSGSimpleTextureNode* node = static_cast<QSGSimpleTextureNode*>(oldNode);

    if (!node) {
        node = new QSGSimpleTextureNode();
        node->setFiltering(QSGTexture::Linear);
    }

    // Set the geometry
    node->setRect(boundingRect());

#ifdef __linux__
    // Try DMA-BUF path first
    if (m_dmaBufQueued) {
        // Initialize EGL importer on first use (must be called on render thread with GL context)
        if (!m_eglInitialized) {
            m_eglInitialized = true;
            if (!m_eglImporter.init()) {
                qWarning() << "CefView: EGL DMA-BUF import not supported, falling back to software";
            }
        }

        if (m_eglImporter.isSupported()) {
            AcceleratedPaintInfo info;
            {
                std::lock_guard<std::mutex> lock(m_dmaBufMutex);
                if (m_dmaBufQueued) {
                    info = std::move(m_pendingDmaBuf);
                    m_dmaBufQueued = false;
                }
            }

            if (!info.planes.empty() && info.planes[0].fd >= 0) {
                // Size changed - delete old texture and wait for GPU
                if (m_glTexture != 0 && (m_textureWidth != info.width || m_textureHeight != info.height)) {
                    glFinish();  // Wait for GPU before deleting
                    glDeleteTextures(1, &m_glTexture);
                    m_glTexture = 0;
                }

                // Create texture if needed
                if (m_glTexture == 0) {
                    glGenTextures(1, &m_glTexture);
                    glBindTexture(GL_TEXTURE_2D, m_glTexture);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    m_textureWidth = info.width;
                    m_textureHeight = info.height;
                }

                // Import DMA-BUF to texture
                if (m_eglImporter.importDmaBufToTexture(m_glTexture, info.planes[0].fd,
                        info.width, info.height, info.format, info.modifier,
                        info.planes[0].stride, info.planes[0].offset)) {

                    // Create QSGTexture from native GL texture
                    QSGTexture* texture = QNativeInterface::QSGOpenGLTexture::fromNative(
                        m_glTexture, window(), QSize(info.width, info.height),
                        QQuickWindow::TextureHasAlphaChannel);

                    if (texture) {
                        // Take ownership - node will delete the texture
                        node->setOwnsTexture(true);
                        node->setTexture(texture);
                        m_textureNeedsUpdate = false;
                    }
                }

                // Close fds after import
                for (auto& plane : info.planes) {
                    if (plane.fd >= 0) close(plane.fd);
                }
            }
        }
    }
#endif

    // Fallback to software path if no DMA-BUF texture
    if (!node->texture() && !m_image.isNull()) {
        QSGTexture* texture = window()->createTextureFromImage(m_image);
        if (texture) {
            node->setOwnsTexture(true);
            node->setTexture(texture);
        }
    }

    // Handle popup overlay (TODO: could be a separate child node)
    // For now, popup is composited in OnPaint callback

    return node;
}

void CefView::geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);

    // Create browser on first valid geometry if not yet created
    if (!m_browserCreated && m_componentComplete && !m_url.isEmpty() && window()) {
        createBrowser();
        return;
    }

    // On resize: clear stale DMA-BUFs and notify CEF
    if (newGeometry.size() != oldGeometry.size()) {
#ifdef __linux__
        // Clear any pending DMA-BUF (stale after resize)
        {
            std::lock_guard<std::mutex> lock(m_dmaBufMutex);
            if (m_dmaBufQueued && !m_pendingDmaBuf.planes.empty()) {
                for (auto& plane : m_pendingDmaBuf.planes) {
                    if (plane.fd >= 0) close(plane.fd);
                }
                m_pendingDmaBuf.planes.clear();
                m_dmaBufQueued = false;
            }
        }
#endif
        // Trigger immediate repaint to show scaled existing content
        update();

        // Notify CEF to render at new size
        if (m_handler && m_handler->GetBrowser()) {
            m_handler->GetBrowser()->GetHost()->WasResized();
        }
    }
}

void CefView::mousePressEvent(QMouseEvent* event)
{
    // Ensure Qt focus on click (important for window activation)
    if (!hasActiveFocus()) {
        forceActiveFocus();
    }

    if (!m_handler || !m_handler->GetBrowser()) {
        return;
    }

    // Set CEF browser focus
    m_handler->GetBrowser()->GetHost()->SetFocus(true);

    QPoint pos = scaleToView(event->position());

    // Track multi-click (double/triple click detection)
    // Match QtWebEngine behavior: reset if button changed, timeout, moved, or already at 3
    if (event->button() != m_lastClickButton
        || (event->timestamp() - m_lastClickTime
            > static_cast<ulong>(qGuiApp->styleHints()->mouseDoubleClickInterval()))
        || (event->position().toPoint() - m_lastClickPosition).manhattanLength()
            > qGuiApp->styleHints()->startDragDistance()
        || m_clickCount >= 3)
        m_clickCount = 0;

    m_lastClickTime = event->timestamp();
    m_lastClickPosition = event->position().toPoint();
    m_lastClickButton = event->button();
    ++m_clickCount;

    CefMouseEvent cef_event;
    cef_event.x = pos.x();
    cef_event.y = pos.y();
    cef_event.modifiers = getCefModifiers(event->modifiers(), event->buttons());

    m_handler->GetBrowser()->GetHost()->SendMouseClickEvent(
        cef_event, getCefMouseButton(event->button()), false, m_clickCount);
}

void CefView::mouseReleaseEvent(QMouseEvent* event)
{
    if (!m_handler || !m_handler->GetBrowser()) {
        return;
    }

    QPoint pos = scaleToView(event->position());
    CefMouseEvent cef_event;
    cef_event.x = pos.x();
    cef_event.y = pos.y();
    cef_event.modifiers = getCefModifiers(event->modifiers(), event->buttons());

    m_handler->GetBrowser()->GetHost()->SendMouseClickEvent(
        cef_event, getCefMouseButton(event->button()), true, m_clickCount);
}

void CefView::mouseDoubleClickEvent(QMouseEvent* /*event*/)
{
    // Qt already called mousePressEvent with the second click before this.
    // Our click count tracking in mousePressEvent handles double-click detection.
    // Don't forward again or we'll send 3 clicks instead of 2.
}

void CefView::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_handler || !m_handler->GetBrowser()) {
        return;
    }

    QPoint pos = scaleToView(event->position());
    CefMouseEvent cef_event;
    cef_event.x = pos.x();
    cef_event.y = pos.y();
    cef_event.modifiers = getCefModifiers(event->modifiers(), event->buttons());

    m_handler->GetBrowser()->GetHost()->SendMouseMoveEvent(cef_event, false);
}

void CefView::wheelEvent(QWheelEvent* event)
{
    if (!m_handler || !m_handler->GetBrowser()) {
        return;
    }

    QPoint pos = scaleToView(event->position());
    CefMouseEvent cef_event;
    cef_event.x = pos.x();
    cef_event.y = pos.y();
    cef_event.modifiers = getCefModifiers(event->modifiers(), Qt::NoButton);

    m_handler->GetBrowser()->GetHost()->SendMouseWheelEvent(
        cef_event, event->angleDelta().x(), event->angleDelta().y());
}

void CefView::keyPressEvent(QKeyEvent* event)
{
    if (!m_handler || !m_handler->GetBrowser()) {
        return;
    }

    CefKeyEvent cef_event;
    cef_event.modifiers = getCefModifiers(event->modifiers(), Qt::NoButton);

    cef_event.native_key_code = event->nativeScanCode();
    cef_event.windows_key_code = getWindowsKeyCode(event);

    // Always send KEYDOWN first (JS listens for keydown events)
    cef_event.type = KEYEVENT_KEYDOWN;
    m_handler->GetBrowser()->GetHost()->SendKeyEvent(cef_event);

    // Then send CHAR event for printable characters
    if (!event->text().isEmpty()) {
        cef_event.type = KEYEVENT_CHAR;
        cef_event.character = event->text().at(0).unicode();
        cef_event.unmodified_character = cef_event.character;
        m_handler->GetBrowser()->GetHost()->SendKeyEvent(cef_event);
    }
}

void CefView::keyReleaseEvent(QKeyEvent* event)
{
    // Ignore auto-repeating KeyRelease events per spec - autorepeat should only
    // generate keypress events, with a single keyup at the end
    if (event->isAutoRepeat())
        return;

    if (!m_handler || !m_handler->GetBrowser()) {
        return;
    }

    CefKeyEvent cef_event;
    cef_event.type = KEYEVENT_KEYUP;
    cef_event.modifiers = getCefModifiers(event->modifiers(), Qt::NoButton);
    cef_event.native_key_code = event->nativeScanCode();
    cef_event.windows_key_code = getWindowsKeyCode(event);

    m_handler->GetBrowser()->GetHost()->SendKeyEvent(cef_event);
}

void CefView::focusInEvent(QFocusEvent* event)
{
    QQuickItem::focusInEvent(event);
    if (m_handler && m_handler->GetBrowser()) {
        m_handler->GetBrowser()->GetHost()->SetFocus(true);
    }
}

void CefView::focusOutEvent(QFocusEvent* event)
{
    QQuickItem::focusOutEvent(event);
    if (m_handler && m_handler->GetBrowser()) {
        m_handler->GetBrowser()->GetHost()->SetFocus(false);
    }
}

void CefView::hoverEnterEvent(QHoverEvent* event)
{
    QQuickItem::hoverEnterEvent(event);
    // Restore cursor when mouse enters - JS idle detection will hide it again if needed
    qApp->restoreOverrideCursor();
}

void CefView::hoverLeaveEvent(QHoverEvent* event)
{
    QQuickItem::hoverLeaveEvent(event);
}

void CefView::hoverMoveEvent(QHoverEvent* event)
{
    QQuickItem::hoverMoveEvent(event);

    if (!m_handler || !m_handler->GetBrowser()) {
        return;
    }

    QPoint pos = scaleToView(event->position());
    CefMouseEvent cef_event;
    cef_event.x = pos.x();
    cef_event.y = pos.y();
    cef_event.modifiers = getCefModifiers(event->modifiers(), Qt::NoButton);

    m_handler->GetBrowser()->GetHost()->SendMouseMoveEvent(cef_event, false);
}

void CefView::touchEvent(QTouchEvent* event)
{
    if (!m_handler || !m_handler->GetBrowser()) {
        event->ignore();
        return;
    }

    event->accept();

    for (const QTouchEvent::TouchPoint& point : event->points()) {
        CefTouchEvent cef_event;
        cef_event.id = point.id();

        QPointF pos = point.position();
        QPoint scaledPos = scaleToView(pos);
        cef_event.x = static_cast<float>(scaledPos.x());
        cef_event.y = static_cast<float>(scaledPos.y());

        // Touch area
        QSizeF area = point.ellipseDiameters();
        cef_event.radius_x = static_cast<float>(area.width() / 2.0);
        cef_event.radius_y = static_cast<float>(area.height() / 2.0);
        cef_event.rotation_angle = static_cast<float>(point.rotation());
        cef_event.pressure = static_cast<float>(point.pressure());

        cef_event.modifiers = getCefModifiers(event->modifiers(), Qt::NoButton);
        cef_event.pointer_type = CEF_POINTER_TYPE_TOUCH;

        switch (point.state()) {
            case QEventPoint::Pressed:
                cef_event.type = CEF_TET_PRESSED;
                break;
            case QEventPoint::Updated:
                cef_event.type = CEF_TET_MOVED;
                break;
            case QEventPoint::Released:
                cef_event.type = CEF_TET_RELEASED;
                break;
            case QEventPoint::Stationary:
                continue;  // Skip stationary points
            default:
                cef_event.type = CEF_TET_CANCELLED;
                break;
        }

        m_handler->GetBrowser()->GetHost()->SendTouchEvent(cef_event);
    }
}

void CefView::itemChange(ItemChange change, const ItemChangeData& value)
{
    QQuickItem::itemChange(change, value);

    if (change == ItemSceneChange && value.window) {
        // Connect to window size changes
        connect(value.window, &QQuickWindow::widthChanged, this, [this]() {
            updateBrowserSize();
        });
        connect(value.window, &QQuickWindow::heightChanged, this, [this]() {
            updateBrowserSize();
        });

    }
}

void CefView::updateBrowserSize()
{
    if (!m_handler || !m_handler->GetBrowser() || !window()) {
        return;
    }

    qreal dpr = window()->devicePixelRatio();
    int w = static_cast<int>(window()->width() * dpr);
    int h = static_cast<int>(window()->height() * dpr);

    if (w > 0 && h > 0) {
        m_handler->SetViewSize(w, h);
    }
}

void CefView::createBrowser()
{
    if (m_browserCreated || m_url.isEmpty() || !window()) {
        return;
    }

    // Use window size directly
    qreal dpr = window()->devicePixelRatio();
    int w = static_cast<int>(window()->width() * dpr);
    int h = static_cast<int>(window()->height() * dpr);

    if (w > 0 && h > 0) {
        m_handler->SetViewSize(w, h);
    }

    CefWindowInfo window_info;
    window_info.SetAsWindowless(0);
#if defined(__linux__)
    // Enable GPU-accelerated shared texture mode for DMA-BUF on Linux
    window_info.shared_texture_enabled = true;
#endif

    CefBrowserSettings settings;
    settings.windowless_frame_rate = getScreenRefreshRate();

    // Apply background color
    if (m_backgroundColor.alpha() == 0) {
        settings.background_color = 0; // Transparent
    } else {
        settings.background_color = CefColorSetARGB(
            m_backgroundColor.alpha(),
            m_backgroundColor.red(),
            m_backgroundColor.green(),
            m_backgroundColor.blue()
        );
    }

    CefBrowserHost::CreateBrowser(window_info, m_handler,
                                   m_url.toString().toStdString(),
                                   settings, nullptr, nullptr);

    m_browserCreated = true;
}

void CefView::onLoadStateChanged(bool isLoading, bool canGoBack, bool canGoForward)
{
    m_isLoading = isLoading;

    if (m_handler && m_handler->GetBrowser()) {
        auto frame = m_handler->GetBrowser()->GetMainFrame();
        if (frame) {
            QUrl currentUrl = QUrl(QString::fromStdString(frame->GetURL().ToString()));

            if (isLoading) {
                emit loadingChanged(currentUrl, 0, 0, QString()); // LoadStartedStatus
            } else {
                emit loadingChanged(currentUrl, 1, 0, QString()); // LoadSucceededStatus

                // Create web channel transport after first successful load
                if (m_webChannel && !m_webChannelTransport) {
                    m_webChannelTransport = new CefWebChannelTransport(m_handler->GetBrowser(), this);
                    m_webChannel->connectTo(m_webChannelTransport);
                    // Flush any queued messages
                    for (const QString& msg : m_pendingWebChannelMessages) {
                        m_webChannelTransport->onMessageFromJs(msg);
                    }
                    m_pendingWebChannelMessages.clear();
                }
            }
        }
    }
}

uint32_t CefView::getCefModifiers(Qt::KeyboardModifiers mods, Qt::MouseButtons buttons)
{
    uint32_t modifiers = 0;
    if (mods & Qt::ShiftModifier)   modifiers |= EVENTFLAG_SHIFT_DOWN;
    if (mods & Qt::ControlModifier) modifiers |= EVENTFLAG_CONTROL_DOWN;
    if (mods & Qt::AltModifier)     modifiers |= EVENTFLAG_ALT_DOWN;
    if (mods & Qt::MetaModifier)    modifiers |= EVENTFLAG_COMMAND_DOWN;
    if (buttons & Qt::LeftButton)   modifiers |= EVENTFLAG_LEFT_MOUSE_BUTTON;
    if (buttons & Qt::MiddleButton) modifiers |= EVENTFLAG_MIDDLE_MOUSE_BUTTON;
    if (buttons & Qt::RightButton)  modifiers |= EVENTFLAG_RIGHT_MOUSE_BUTTON;
    return modifiers;
}

cef_mouse_button_type_t CefView::getCefMouseButton(Qt::MouseButton button)
{
    switch (button) {
        case Qt::LeftButton:   return MBT_LEFT;
        case Qt::RightButton:  return MBT_RIGHT;
        case Qt::MiddleButton: return MBT_MIDDLE;
        default:               return MBT_LEFT;
    }
}

QPoint CefView::scaleToView(const QPointF& widgetPos)
{
    qreal dpr = window() ? window()->devicePixelRatio() : 1.0;

    if (m_viewWidth <= 0 || m_viewHeight <= 0 || width() <= 0 || height() <= 0) {
        return QPoint(static_cast<int>(widgetPos.x() * dpr),
                      static_cast<int>(widgetPos.y() * dpr));
    }

    int x = static_cast<int>(widgetPos.x() * m_viewWidth / width());
    int y = static_cast<int>(widgetPos.y() * m_viewHeight / height());
    return QPoint(x, y);
}

void CefView::updateCursor(cef_cursor_type_t type)
{
    Qt::CursorShape shape;
    switch (type) {
        case CT_POINTER:       shape = Qt::ArrowCursor; break;
        case CT_CROSS:         shape = Qt::CrossCursor; break;
        case CT_HAND:          shape = Qt::PointingHandCursor; break;
        case CT_IBEAM:         shape = Qt::IBeamCursor; break;
        case CT_WAIT:          shape = Qt::WaitCursor; break;
        case CT_HELP:          shape = Qt::WhatsThisCursor; break;
        case CT_EASTRESIZE:    shape = Qt::SizeHorCursor; break;
        case CT_NORTHRESIZE:   shape = Qt::SizeVerCursor; break;
        case CT_NORTHEASTRESIZE: shape = Qt::SizeBDiagCursor; break;
        case CT_NORTHWESTRESIZE: shape = Qt::SizeFDiagCursor; break;
        case CT_SOUTHRESIZE:   shape = Qt::SizeVerCursor; break;
        case CT_SOUTHEASTRESIZE: shape = Qt::SizeFDiagCursor; break;
        case CT_SOUTHWESTRESIZE: shape = Qt::SizeBDiagCursor; break;
        case CT_WESTRESIZE:    shape = Qt::SizeHorCursor; break;
        case CT_NORTHSOUTHRESIZE: shape = Qt::SizeVerCursor; break;
        case CT_EASTWESTRESIZE: shape = Qt::SizeHorCursor; break;
        case CT_NORTHEASTSOUTHWESTRESIZE: shape = Qt::SizeBDiagCursor; break;
        case CT_NORTHWESTSOUTHEASTRESIZE: shape = Qt::SizeFDiagCursor; break;
        case CT_COLUMNRESIZE:  shape = Qt::SplitHCursor; break;
        case CT_ROWRESIZE:     shape = Qt::SplitVCursor; break;
        case CT_MOVE:          shape = Qt::SizeAllCursor; break;
        case CT_PROGRESS:      shape = Qt::BusyCursor; break;
        case CT_NODROP:        shape = Qt::ForbiddenCursor; break;
        case CT_NOTALLOWED:    shape = Qt::ForbiddenCursor; break;
        case CT_GRAB:          shape = Qt::OpenHandCursor; break;
        case CT_GRABBING:      shape = Qt::ClosedHandCursor; break;
        case CT_NONE:          shape = Qt::BlankCursor; break;
        default:               shape = Qt::ArrowCursor; break;
    }
    setCursor(QCursor(shape));
}

void CefView::queueDmaBuf(const AcceleratedPaintInfo& info)
{
    std::lock_guard<std::mutex> lock(m_dmaBufMutex);

    // Close previous queued fds if not yet imported
    if (m_dmaBufQueued && !m_pendingDmaBuf.planes.empty()) {
        for (auto& plane : m_pendingDmaBuf.planes) {
            if (plane.fd >= 0) close(plane.fd);
        }
    }

    m_pendingDmaBuf = info;
    m_dmaBufQueued = true;

    // Mark dirty and ensure vsync timer is running
    m_frameDirty.store(true, std::memory_order_relaxed);
    QMetaObject::invokeMethod(this, [this]() {
        if (!m_vsyncTimer.isActive()) {
            m_vsyncTimer.start();
        }
    }, Qt::QueuedConnection);
}

void CefView::showContextMenu(int x, int y,
                               const std::vector<JellyfinCefHandler::ContextMenuItem>& items,
                               CefRefPtr<CefRunContextMenuCallback> callback)
{
    if (!window()) {
        callback->Cancel();
        return;
    }

    // Convert CEF coordinates to screen coordinates
    qreal scaleX = m_viewWidth > 0 ? width() / m_viewWidth : 1.0;
    qreal scaleY = m_viewHeight > 0 ? height() / m_viewHeight : 1.0;
    QPointF localPos(x * scaleX, y * scaleY);
    QPointF scenePos = mapToScene(localPos);
    QPoint globalPos = window()->mapToGlobal(scenePos.toPoint());

    QMenu menu;
    QMap<QAction*, int> actionMap;

    for (const auto& item : items) {
        if (item.isSeparator) {
            menu.addSeparator();
        } else {
            QAction* action = menu.addAction(QString::fromStdString(item.label));
            action->setEnabled(item.enabled);
            actionMap[action] = item.commandId;
        }
    }

    QAction* selectedAction = menu.exec(globalPos);

    if (selectedAction && actionMap.contains(selectedAction)) {
        callback->Continue(actionMap[selectedAction], EVENTFLAG_NONE);
    } else {
        callback->Cancel();
    }
}

int CefView::getScreenRefreshRate() const
{
    if (window()) {
        if (QScreen* screen = window()->screen()) {
            qreal rate = screen->refreshRate();
            if (rate > 0) {
                return static_cast<int>(rate);
            }
        }
    }
    return 60;  // Default fallback
}

int CefView::getWindowsKeyCode(QKeyEvent* event)
{
    // Start with native virtual key code
    int code = event->nativeVirtualKey();

    // Map Qt keys to Windows virtual key codes
    switch (event->key()) {
        case Qt::Key_Backspace: return 0x08;
        case Qt::Key_Tab: return 0x09;
        case Qt::Key_Clear: return 0x0C;
        case Qt::Key_Return:
        case Qt::Key_Enter: return 0x0D;
        case Qt::Key_Shift: return 0x10;
        case Qt::Key_Control: return 0x11;
        case Qt::Key_Alt: return 0x12;
        case Qt::Key_Pause: return 0x13;
        case Qt::Key_CapsLock: return 0x14;
        case Qt::Key_Escape: return 0x1B;
        case Qt::Key_Space: return 0x20;
        case Qt::Key_PageUp: return 0x21;
        case Qt::Key_PageDown: return 0x22;
        case Qt::Key_End: return 0x23;
        case Qt::Key_Home: return 0x24;
        case Qt::Key_Left: return 0x25;
        case Qt::Key_Up: return 0x26;
        case Qt::Key_Right: return 0x27;
        case Qt::Key_Down: return 0x28;
        case Qt::Key_Select: return 0x29;
        case Qt::Key_Print: return 0x2A;
        case Qt::Key_Execute: return 0x2B;
        case Qt::Key_SysReq: return 0x2C;
        case Qt::Key_Insert: return 0x2D;
        case Qt::Key_Delete: return 0x2E;
        case Qt::Key_Help: return 0x2F;
        // 0-9 (same as ASCII)
        case Qt::Key_0: return 0x30;
        case Qt::Key_1: return 0x31;
        case Qt::Key_2: return 0x32;
        case Qt::Key_3: return 0x33;
        case Qt::Key_4: return 0x34;
        case Qt::Key_5: return 0x35;
        case Qt::Key_6: return 0x36;
        case Qt::Key_7: return 0x37;
        case Qt::Key_8: return 0x38;
        case Qt::Key_9: return 0x39;
        // A-Z (same as ASCII uppercase)
        case Qt::Key_A: return 0x41;
        case Qt::Key_B: return 0x42;
        case Qt::Key_C: return 0x43;
        case Qt::Key_D: return 0x44;
        case Qt::Key_E: return 0x45;
        case Qt::Key_F: return 0x46;
        case Qt::Key_G: return 0x47;
        case Qt::Key_H: return 0x48;
        case Qt::Key_I: return 0x49;
        case Qt::Key_J: return 0x4A;
        case Qt::Key_K: return 0x4B;
        case Qt::Key_L: return 0x4C;
        case Qt::Key_M: return 0x4D;
        case Qt::Key_N: return 0x4E;
        case Qt::Key_O: return 0x4F;
        case Qt::Key_P: return 0x50;
        case Qt::Key_Q: return 0x51;
        case Qt::Key_R: return 0x52;
        case Qt::Key_S: return 0x53;
        case Qt::Key_T: return 0x54;
        case Qt::Key_U: return 0x55;
        case Qt::Key_V: return 0x56;
        case Qt::Key_W: return 0x57;
        case Qt::Key_X: return 0x58;
        case Qt::Key_Y: return 0x59;
        case Qt::Key_Z: return 0x5A;
        case Qt::Key_Meta: return 0x5B;  // Left Windows key
        case Qt::Key_Menu: return 0x5D;
        // Numpad
        case Qt::Key_multiply: return 0x6A;
        case Qt::Key_Plus: return 0x6B;
        case Qt::Key_Minus: return 0x6D;
        case Qt::Key_Period: return 0x6E;
        case Qt::Key_Slash: return 0x6F;
        // Function keys F1-F12
        case Qt::Key_F1: return 0x70;
        case Qt::Key_F2: return 0x71;
        case Qt::Key_F3: return 0x72;
        case Qt::Key_F4: return 0x73;
        case Qt::Key_F5: return 0x74;
        case Qt::Key_F6: return 0x75;
        case Qt::Key_F7: return 0x76;
        case Qt::Key_F8: return 0x77;
        case Qt::Key_F9: return 0x78;
        case Qt::Key_F10: return 0x79;
        case Qt::Key_F11: return 0x7A;
        case Qt::Key_F12: return 0x7B;
        case Qt::Key_NumLock: return 0x90;
        case Qt::Key_ScrollLock: return 0x91;
        // OEM keys
        case Qt::Key_Semicolon: return 0xBA;
        case Qt::Key_Equal: return 0xBB;
        case Qt::Key_Comma: return 0xBC;
        case Qt::Key_Underscore: return 0xBD;
        case Qt::Key_BracketLeft: return 0xDB;
        case Qt::Key_Backslash: return 0xDC;
        case Qt::Key_BracketRight: return 0xDD;
        case Qt::Key_QuoteDbl:
        case Qt::Key_Apostrophe: return 0xDE;
        // Media keys
        case Qt::Key_VolumeMute: return 0xAD;
        case Qt::Key_VolumeDown: return 0xAE;
        case Qt::Key_VolumeUp: return 0xAF;
        case Qt::Key_MediaNext: return 0xB0;
        case Qt::Key_MediaPrevious: return 0xB1;
        case Qt::Key_MediaStop: return 0xB2;
        case Qt::Key_MediaPlay:
        case Qt::Key_MediaTogglePlayPause: return 0xB3;
        default: break;
    }

    return code;
}
