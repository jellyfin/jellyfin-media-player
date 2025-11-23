// Compatibility header for Qt < 6.5 that lacks QWaylandApplication
#ifndef QT_WAYLAND_COMPAT_H
#define QT_WAYLAND_COMPAT_H

#include <QtCore/qglobal.h>

#if QT_VERSION < QT_VERSION_CHECK(6, 5, 0)

#include <QtCore/qnativeinterface.h>
#include <QtGui/qguiapplication.h>

struct wl_display;

namespace QNativeInterface {

struct Q_GUI_EXPORT QWaylandApplication
{
    QT_DECLARE_NATIVE_INTERFACE(QWaylandApplication, 1, QGuiApplication)
    virtual wl_display *display() const = 0;
};

}

#endif // QT_VERSION < 6.5.0

#endif // QT_WAYLAND_COMPAT_H
