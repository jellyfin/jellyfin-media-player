#include <QDebug>
#include "QRPIJpegPlugin.h"
#include "QRPIJpegHandler.h"


QT_BEGIN_NAMESPACE

///////////////////////////////////////////////////////////////////////////////////////////////////
QImageIOPlugin::Capabilities QRPIJpegPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "jpeg" || format == "jpg")
        return Capabilities(CanRead);
    if (!format.isEmpty())
        return 0;
    if (!device->isOpen())
        return 0;

    Capabilities cap;
    if (device->isReadable() && QRPIJpegHandler::canRead(device))
        cap |= CanRead;
    if (device->isWritable())
        cap |= CanWrite;

    return cap;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QImageIOHandler *QRPIJpegPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new QRPIJpegHandler();
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

QT_END_NAMESPACE
