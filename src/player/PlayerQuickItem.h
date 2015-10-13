#ifndef PLAYERQUICKITEM_H
#define PLAYERQUICKITEM_H

#include <Qt>
#include <QtQuick/QQuickItem>

#include <mpv/client.h>
#include <mpv/opengl_cb.h>
#include <mpv/qthelper.hpp>

#include "PlayerComponent.h"

class PlayerRenderer : public QObject
{
  Q_OBJECT
  friend class PlayerQuickItem;

  PlayerRenderer(mpv::qt::Handle mpv, QQuickWindow* window);
  bool init();
  virtual ~PlayerRenderer();
  void render();
  void swap();

  mpv::qt::Handle m_mpv;
  mpv_opengl_cb_context* m_mpvGL;
  QQuickWindow* m_window;
  QSize m_size;
};

class PlayerQuickItem : public QQuickItem
{
    Q_OBJECT
    friend class PlayerRenderer;

public:
    PlayerQuickItem(QQuickItem* parent = 0);
    virtual ~PlayerQuickItem();
    void initMpv(PlayerComponent* player);

signals:
    void onUpdate();
    void onFatalError(QString message);

private slots:
    void onWindowChanged(QQuickWindow* win);
    void onSynchronize();
    void onInvalidate();
    void onHandleFatalError(QString message);

private:
    static void on_update(void *ctx);
    mpv::qt::Handle m_mpv;
    mpv_opengl_cb_context* m_mpvGL;
    PlayerRenderer* m_renderer;
};

#endif
