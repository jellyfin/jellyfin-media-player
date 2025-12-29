#pragma once

#include <QQuickItem>
#include <QQuickWindow>
#include <QSGNode>
#include <QSGSimpleTextureNode>
#include <QUrl>
#include <QColor>
#include <QImage>
#include <QTimer>
#include <QWebChannel>
#include <atomic>
#include <mutex>

#include "include/cef_browser.h"
#include "include/cef_context_menu_handler.h"
#include "CefHandler.h"
#include "CefWebChannelTransport.h"

#ifdef __linux__
#include "EglDmaBufImporter.h"
#endif

class CefView : public QQuickItem {
    Q_OBJECT
    Q_PROPERTY(QWebChannel* webChannel READ webChannel WRITE setWebChannel NOTIFY webChannelChanged)
    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(qreal zoomFactor READ zoomFactor WRITE setZoomFactor NOTIFY zoomFactorChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(QString userAgent READ userAgent WRITE setUserAgent NOTIFY userAgentChanged)
    Q_PROPERTY(QString storagePath READ storagePath WRITE setStoragePath NOTIFY storagePathChanged)
    Q_PROPERTY(QString settingsJson READ settingsJson WRITE setSettingsJson NOTIFY settingsJsonChanged)
    Q_PROPERTY(QString nativeshellScript READ nativeshellScript WRITE setNativeshellScript NOTIFY nativeshellScriptChanged)

public:
    explicit CefView(QQuickItem* parent = nullptr);
    ~CefView() override;

    // Property getters
    QWebChannel* webChannel() const { return m_webChannel; }
    QUrl url() const { return m_url; }
    qreal zoomFactor() const { return m_zoomFactor; }
    QColor backgroundColor() const { return m_backgroundColor; }
    QString userAgent() const { return m_userAgent; }
    QString storagePath() const { return m_storagePath; }
    QString settingsJson() const { return m_settingsJson; }
    QString nativeshellScript() const { return m_nativeshellScript; }

    // Property setters
    void setWebChannel(QWebChannel* channel);
    void setUrl(const QUrl& url);
    void setZoomFactor(qreal factor);
    void setBackgroundColor(const QColor& color);
    void setUserAgent(const QString& userAgent);
    void setStoragePath(const QString& path);
    void setSettingsJson(const QString& json);
    void setNativeshellScript(const QString& script);

    // Q_INVOKABLE methods
    Q_INVOKABLE void reload();
    Q_INVOKABLE void triggerWebAction(int action);
    Q_INVOKABLE void forceActiveFocus();
    Q_INVOKABLE void injectScript(const QString& code);

signals:
    void loadingChanged(const QUrl& url, int status, int errorCode, const QString& errorString);
    void newWindowRequested(const QUrl& url, bool userInitiated);
    void fullScreenRequested(bool toggleOn);
    void javaScriptConsoleMessage(int level, const QString& message, int lineNumber, const QString& sourceId);
    void certificateError(const QUrl& url, const QString& description);
    void linkHovered(const QString& url);
    void urlChanged();
    void zoomFactorChanged();
    void webChannelChanged();
    void backgroundColorChanged();
    void userAgentChanged();
    void storagePathChanged();
    void settingsJsonChanged();
    void nativeshellScriptChanged();

protected:
    void componentComplete() override;
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* data) override;
    void geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void hoverEnterEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;
    void hoverMoveEvent(QHoverEvent* event) override;
    void touchEvent(QTouchEvent* event) override;
    void itemChange(ItemChange change, const ItemChangeData& value) override;

private:
    void createBrowser();
    void updateBrowserSize();
    void onLoadStateChanged(bool isLoading, bool canGoBack, bool canGoForward);
    void showContextMenu(int x, int y,
                         const std::vector<JellyfinCefHandler::ContextMenuItem>& items,
                         CefRefPtr<CefRunContextMenuCallback> callback);

    uint32_t getCefModifiers(Qt::KeyboardModifiers mods, Qt::MouseButtons buttons);
    cef_mouse_button_type_t getCefMouseButton(Qt::MouseButton button);
    int getWindowsKeyCode(QKeyEvent* event);
    QPoint scaleToView(const QPointF& widgetPos);
    int getScreenRefreshRate() const;
    void updateCursor(cef_cursor_type_t type);
    void queueDmaBuf(const AcceleratedPaintInfo& info);

    CefRefPtr<JellyfinCefHandler> m_handler;
    CefWebChannelTransport* m_webChannelTransport = nullptr;
    QWebChannel* m_webChannel = nullptr;
    QUrl m_url;
    qreal m_zoomFactor = 1.0;
    QColor m_backgroundColor = Qt::transparent;
    QString m_userAgent;
    QString m_storagePath;
    QString m_settingsJson;
    QString m_nativeshellScript;

    QImage m_image;
    int m_viewWidth = 0;
    int m_viewHeight = 0;
    bool m_browserCreated = false;
    bool m_isLoading = false;
    bool m_componentComplete = false;
    QStringList m_pendingWebChannelMessages;  // Queue until transport ready
    QTimer m_vsyncTimer;  // Drives rendering at display refresh rate
    std::atomic<bool> m_frameDirty{false};  // New frame available
    int m_idleTicks = 0;  // Count ticks with no new frame

    // Popup tracking
    QImage m_popupImage;
    QRect m_popupRect;
    bool m_popupVisible = false;

    // Multi-click tracking (matches QtWebEngine behavior)
    ulong m_lastClickTime = 0;
    QPoint m_lastClickPosition;
    Qt::MouseButton m_lastClickButton = Qt::NoButton;
    int m_clickCount = 0;

    // DMA-BUF accelerated paint
    AcceleratedPaintInfo m_pendingDmaBuf;
    bool m_dmaBufQueued = false;
    std::mutex m_dmaBufMutex;

#ifdef __linux__
    // EGL DMA-BUF importer
    EglDmaBufImporter m_eglImporter;
    bool m_eglInitialized = false;
    uint32_t m_glTexture = 0;
    int m_textureWidth = 0;
    int m_textureHeight = 0;
    bool m_textureNeedsUpdate = false;
#endif
};
