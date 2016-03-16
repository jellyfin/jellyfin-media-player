/** private imeplementation.
 * https://github.com/azadkuh/qhttp
 *
 * @author amir zamani
 * @version 2.0.0
 * @date 2014-07-11
  */

#ifndef QHTTPSERVER_CONNECTION_PRIVATE_HPP
#define QHTTPSERVER_CONNECTION_PRIVATE_HPP
///////////////////////////////////////////////////////////////////////////////

#include "qhttpserverconnection.hpp"

#include "qhttpserverrequest.hpp"
#include "qhttpserverresponse.hpp"

#include "private/qhttpserverrequest_private.hpp"
#include "private/qhttpserverresponse_private.hpp"

#include <QBasicTimer>
#include <QFile>
///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace server {
///////////////////////////////////////////////////////////////////////////////
class QHttpConnectionPrivate  : public HttpParser<QHttpConnectionPrivate>
{
protected:
    Q_DECLARE_PUBLIC(QHttpConnection)
    QHttpConnection* const q_ptr;

public:
    QByteArray             itempUrl;

    // Since there can only be one request/response pair per connection at any time even with pipelining.
    QHttpRequest*          ilastRequest  = nullptr;
    QHttpResponse*         ilastResponse = nullptr;


    TServerHandler         ihandler = nullptr;

public:
    explicit     QHttpConnectionPrivate(QHttpConnection* q) : HttpParser(HTTP_REQUEST), q_ptr(q) {

        QObject::connect(q_func(), &QHttpConnection::disconnected, [this](){
            // if socket drops and http_parser can not call messageComplete, dispatch the ilastRequest
            onDispatchRequest();
            isocket.release();

            if ( ilastRequest )
                ilastRequest->deleteLater();
            if ( ilastResponse )
                ilastResponse->deleteLater();

            q_func()->deleteLater();
        });

        QHTTP_LINE_DEEPLOG
    }

    virtual     ~QHttpConnectionPrivate() {
        QHTTP_LINE_DEEPLOG
    }

    void         createSocket(qintptr sokDesc, TBackend bend) {
        isocket.ibackendType = bend;

        if        ( bend == ETcpSocket ) {
            QTcpSocket* sok    = new QTcpSocket( q_func() );
            isocket.itcpSocket = sok;
            sok->setSocketDescriptor(sokDesc);

            QObject::connect(sok,       &QTcpSocket::readyRead, [this](){
                onReadyRead();
            });
            QObject::connect(sok,       &QTcpSocket::bytesWritten, [this](){
                if ( isocket.itcpSocket->bytesToWrite() == 0  &&  ilastResponse )
                    emit ilastResponse->allBytesWritten();
            });
            QObject::connect(sok,       &QTcpSocket::disconnected,
                             q_func(),  &QHttpConnection::disconnected,
                             Qt::QueuedConnection);

        } else if ( bend == ELocalSocket ) {
            QLocalSocket* sok    = new QLocalSocket( q_func() );
            isocket.ilocalSocket = sok;
            sok->setSocketDescriptor(sokDesc);

            QObject::connect(sok,       &QLocalSocket::readyRead, [this](){
                onReadyRead();
            });
            QObject::connect(sok,       &QLocalSocket::bytesWritten, [this](){
                if ( isocket.ilocalSocket->bytesToWrite() == 0  &&  ilastResponse )
                    emit ilastResponse->allBytesWritten();
            });
            QObject::connect(sok,       &QLocalSocket::disconnected,
                             q_func(),  &QHttpConnection::disconnected,
                             Qt::QueuedConnection);
        }

    }

public:
    void         onReadyRead() {
        while ( isocket.bytesAvailable() > 0 ) {
            char buffer[4097] = {0};
            size_t readLength = (size_t) isocket.readRaw(buffer, 4096);

            parse(buffer, readLength);
        }

        onDispatchRequest();
    }

    void         onDispatchRequest() {
        // if ilastRequest has been sent previously, just return
        if ( !ilastRequest || ilastRequest->d_func()->ireadState == QHttpRequestPrivate::ESent )
            return;

        ilastRequest->d_func()->ireadState = QHttpRequestPrivate::ESent;
        emit ilastRequest->end();
    }

public:
    int          messageBegin(http_parser* parser);
    int          url(http_parser* parser, const char* at, size_t length);
    int          status(http_parser*, const char*, size_t) {
        return 0;   // not used in parsing incoming request.
    }
    int          headerField(http_parser* parser, const char* at, size_t length);
    int          headerValue(http_parser* parser, const char* at, size_t length);
    int          headersComplete(http_parser* parser);
    int          body(http_parser* parser, const char* at, size_t length);
    int          messageComplete(http_parser* parser);

#ifdef USE_CUSTOM_URL_CREATOR
public:
    static QUrl  createUrl(const char *urlData, const http_parser_url &urlInfo);
#endif // USE_CUSTOM_URL_CREATOR

};

///////////////////////////////////////////////////////////////////////////////
} // namespace server
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif // QHTTPSERVER_CONNECTION_PRIVATE_HPP
