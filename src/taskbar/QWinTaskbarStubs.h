#ifndef QWINTASKBARSTUBS_H
#define QWINTASKBARSTUBS_H

// Qt6 compatibility stubs for removed QWinTaskbar classes
// These classes were removed from Qt6 and have not been replaced yet
// This provides minimal stub implementations to allow compilation

#include <QObject>
#include <QQuickWindow>
#include <QIcon>

class QWinTaskbarProgress : public QObject
{
    Q_OBJECT
public:
    explicit QWinTaskbarProgress(QObject *parent = nullptr) : QObject(parent) {}
    void setVisible(bool) {}
    void setValue(int) {}
    void setMinimum(int) {}
    void setMaximum(int) {}
    void setPaused(bool) {}
};

class QWinTaskbarButton : public QObject
{
    Q_OBJECT
public:
    explicit QWinTaskbarButton(QObject *parent = nullptr) : QObject(parent), m_progress(new QWinTaskbarProgress(this)) {}
    void setWindow(QWindow*) {}
    QWinTaskbarProgress* progress() { return m_progress; }
private:
    QWinTaskbarProgress* m_progress;
};

class QWinThumbnailToolButton : public QObject
{
    Q_OBJECT
public:
    explicit QWinThumbnailToolButton(QObject *parent = nullptr) : QObject(parent) {}
    void setToolTip(const QString&) {}
    void setIcon(const QIcon&) {}
    void setEnabled(bool) {}
    void setVisible(bool) {}
Q_SIGNALS:
    void clicked();
};

class QWinThumbnailToolBar : public QObject
{
    Q_OBJECT
public:
    explicit QWinThumbnailToolBar(QObject *parent = nullptr) : QObject(parent) {}
    void setWindow(QWindow*) {}
    void addButton(QWinThumbnailToolButton* button) { m_buttons.append(button); }
    QList<QWinThumbnailToolButton*> buttons() const { return m_buttons; }
private:
    QList<QWinThumbnailToolButton*> m_buttons;
};

#endif // QWINTASKBARSTUBS_H
