#include <QVariant>
#include <QIODevice>
#include <QImage>
#include <QColor>
#include <QDebug>
#include <QElapsedTimer>
#include <QDateTime>

#include "QRPIJpegHandler.h"

QT_BEGIN_NAMESPACE

///////////////////////////////////////////////////////////////////////////////////////////////////
QRPIJpegHandler::QRPIJpegHandler() : m_quality(100), m_decoder(NULL)
{
  setFormat("jpeg");

  // initialize decoder parameters
  memset(&m_dec_request, 0, sizeof(m_dec_request));
  m_dec_request.input = encodedInBuf;
  m_dec_request.output = decodedBuf;
  m_dec_request.output_handle = 0;
  m_dec_request.output_alloc_size = MAX_DECODED;
  m_dec_request.pixel_format = PIXEL_FORMAT_RGBA;

  BRCMJPEG_STATUS_T status = brcmjpeg_create(BRCMJPEG_TYPE_DECODER, &m_decoder);
  if (status != BRCMJPEG_SUCCESS)
  {
    qWarning() << "QRPIJpegHandler : could not create decoder";
    brcmjpeg_release(m_decoder);
    m_decoder = NULL;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QRPIJpegHandler::~QRPIJpegHandler()
{
  if (m_decoder)
    brcmjpeg_release(m_decoder);

  m_decoder = NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool QRPIJpegHandler::canRead() const
{
  return canRead(device());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool QRPIJpegHandler::canRead(QIODevice* device)
{
  if (!device)
  {
    qWarning("QRPIJpegHandler::canRead() called with no device");
    return false;
  }

  unsigned char buffer[2];
  if (device->peek((char*)buffer, 2) != 2)
    return false;

  return uchar(buffer[0]) == 0xff && uchar(buffer[1]) == 0xd8;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool QRPIJpegHandler::read(QImage* image)
{
  if (device())
  {

    m_dec_request.buffer_width = 0;
    m_dec_request.buffer_height = 0;
    m_dec_request.input_size = device()->read((char*)encodedInBuf, sizeof(encodedInBuf));

    QElapsedTimer timer;
    timer.start();

    BRCMJPEG_STATUS_T status = brcmjpeg_process(m_decoder, &m_dec_request);

    if (status == BRCMJPEG_SUCCESS)
    {

      for (int i = 0; i < m_dec_request.height; i++)
      {
        memcpy(image->scanLine(i), decodedBuf + m_dec_request.buffer_width * i * 4,
               m_dec_request.buffer_width * 4);
      }

      //*image = decodedImage;
      qDebug() << QDateTime::currentDateTime().toMSecsSinceEpoch()
               << "QRPIJpegHandler : decoded a"
               << m_dec_request.width << "x" << m_dec_request.height
               << "image in" << timer.elapsed() << "ms";

      return true;
    }
    else
    {
      qWarning() << "QRPIJpegHandler : Decoding failed with status" << status;
      return false;
    }

  }
  else
  {
    qDebug() << "QRPIJpegHandler : read() was called with NULL device";
    return false;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool QRPIJpegHandler::write(const QImage& image)
{
  Q_UNUSED(image);
  qWarning() << "QRPIJpegHandler : call to unsupported write()";
  return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool QRPIJpegHandler::supportsOption(ImageOption option) const
{
  return option == Quality || option == Size || option == ImageFormat;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool QRPIJpegHandler::readJpegSize(QIODevice* device, QSize& size) const
{
  if (canRead(device))
  {
    JFIFHEAD header;
    int bytesRead = device->peek((char*)&header, sizeof(header));
    if (bytesRead >= (sizeof(header) - JFIF_DATA_SIZE))
    {
      BYTE* dataptr = header.data;

      while (dataptr < ((BYTE*)&header + bytesRead))
      {
        if (dataptr[0] != 0xFF)
        {
          qWarning() << "readJpegSize : got wrong marker " << dataptr[0];
          return false;
        }

        // we look for size block marker
        if (dataptr[1] == 0xC0 && dataptr[2] == 0x0)
        {
          size.setWidth(dataptr[8] + dataptr[7] * 256);
          size.setHeight(dataptr[6] + dataptr[5] * 256);
          return true;
        }

        dataptr += dataptr[3] + (dataptr[2] * 256) + 2;
      }
    }
    else
    {
       qWarning() << "readJpegSize : could not read " << sizeof(header) << "bytes, read " << bytesRead;
       return false;
    }
  }
  else
  {
    qWarning() << "readJpegSize : device " << device << "can't be read!" ;
    return false;
  }

  qWarning() << "readJpegSize : could not find the proper size marker";
  return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QVariant QRPIJpegHandler::option(ImageOption option) const
{
  switch (option)
  {
    case Size:
    {
      QSize size(0, 0);
      readJpegSize(device(), size);
      return QVariant(size);
      break;
    }

    case ImageFormat:
    {
      return QImage::Format_RGBA8888_Premultiplied;
    }

    default:
    {
      qWarning() << "QRPIJpegHandler : requesting unsupported option" << option;
      return QVariant();
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void QRPIJpegHandler::setOption(ImageOption option, const QVariant& value)
{
  switch (option)
  {
    case Quality:
      m_quality = value.toInt();
      break;

    default:
      qWarning() << "QRPIJpegHandler : setOption unsupported option" << option << "to" << value;
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QByteArray QRPIJpegHandler::name() const { return "RPIjpeg"; }

QT_END_NAMESPACE
