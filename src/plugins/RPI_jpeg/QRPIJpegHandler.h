#ifndef QRPIJPEGHANDLER_H
#define QRPIJPEGHANDLER_H

#include <QtGui/qimageiohandler.h>
#include <QtCore/QSize>
#include <QtCore/QRect>
#include "brcmjpeg.h"

QT_BEGIN_NAMESPACE

///////////////////////////////////////////////////////////////////////////////////////////////////
// JPEG header struct
#define JFIF_DATA_SIZE 4096 // additionnal data size for header parsing

typedef unsigned char BYTE;

typedef struct _JFIFHeader
{
  BYTE SOI[2];          /* 00h  Start of Image Marker     */
  BYTE APP0[2];         /* 02h  Application Use Marker    */
  BYTE Length[2];       /* 04h  Length of APP0 Field      */
  BYTE Identifier[5];   /* 06h  "JFIF" (zero terminated) Id String */
  BYTE Version[2];      /* 07h  JFIF Format Revision      */
  BYTE Units;           /* 09h  Units used for Resolution */
  BYTE Xdensity[2];     /* 0Ah  Horizontal Resolution     */
  BYTE Ydensity[2];     /* 0Ch  Vertical Resolution       */
  BYTE XThumbnail;      /* 0Eh  Horizontal Pixel Count    */
  BYTE YThumbnail;      /* 0Fh  Vertical Pixel Count      */
  BYTE data[JFIF_DATA_SIZE];
} JFIFHEAD;

// Hardware decoder buffers
#define MAX_WIDTH   5000
#define MAX_HEIGHT  5000
#define MAX_ENCODED (15*1024*1024)
#define MAX_DECODED (MAX_WIDTH*MAX_HEIGHT*2)

static BYTE encodedInBuf[MAX_ENCODED];
static BYTE decodedBuf[MAX_DECODED];

///////////////////////////////////////////////////////////////////////////////////////////////////
// RPI JPEG decoding handling class
class QRPIJpegHandler : public QImageIOHandler
{
public:
    QRPIJpegHandler();
    ~QRPIJpegHandler();

    bool canRead() const Q_DECL_OVERRIDE;
    bool read(QImage *image) Q_DECL_OVERRIDE;
    bool write(const QImage &image) Q_DECL_OVERRIDE;

    QByteArray name() const Q_DECL_OVERRIDE;

    static bool canRead(QIODevice *device);

    QVariant option(ImageOption option) const Q_DECL_OVERRIDE;
    void setOption(ImageOption option, const QVariant &value) Q_DECL_OVERRIDE;
    bool supportsOption(ImageOption option) const Q_DECL_OVERRIDE;

private:
    bool readJpegSize(QIODevice *device, QSize &size) const;
    int m_quality;
    BRCMJPEG_REQUEST_T m_dec_request;
    BRCMJPEG_T *m_decoder;
};

QT_END_NAMESPACE

#endif // QRPIJPEGHANDLER_H
