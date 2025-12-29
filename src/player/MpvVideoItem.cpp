#include "MpvVideoItem.h"
#include "PlayerComponent.h"
#include <MpvController>
#include <QDebug>

MpvVideoItem::MpvVideoItem(QQuickItem *parent)
    : MpvAbstractItem(parent)
{
    qDebug() << "MpvVideoItem constructed";
    // Critical: Set vo=libmpv for Qt integration
    Q_EMIT setProperty("vo", "libmpv");

#ifdef Q_OS_WIN32
    // Force desktop OpenGL and disable advanced features for compatibility with older GPUs
    Q_EMIT setProperty("gpu-api", "opengl");
    Q_EMIT setProperty("opengl-es", "no");
#endif
}

MpvVideoItem::~MpvVideoItem()
{
    // Clear PlayerComponent's reference before mpv is destroyed
    if (m_player) {
        m_player->setMpvController(nullptr);
    }
}

void MpvVideoItem::setPlayerComponent(PlayerComponent* player)
{
    qDebug() << "MpvVideoItem::setPlayerComponent called, mpvController():" << mpvController();
    m_player = player;

    // When mpv is ready, give controller to PlayerComponent
    connect(this, &MpvAbstractItem::ready, this, [this]() {
        qDebug() << "MpvVideoItem ready() signal fired!";
        if (m_player && mpvController()) {
            qDebug() << "Setting mpv controller and initializing";
            m_player->setMpvController(mpvController());
            m_player->initializeMpv();
        } else {
            qWarning() << "ready() fired but m_player:" << m_player << "mpvController():" << mpvController();
        }
    });

    // Check if already ready
    if (mpvController()) {
        qDebug() << "MpvVideoItem already ready, initializing now";
        m_player->setMpvController(mpvController());
        m_player->initializeMpv();
    } else {
        qDebug() << "MpvVideoItem not ready yet, waiting for ready() signal";
    }
}
