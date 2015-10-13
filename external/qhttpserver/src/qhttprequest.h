/*
 * Copyright 2011-2014 Nikhil Marathe <nsm.nikhil@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef Q_HTTP_REQUEST
#define Q_HTTP_REQUEST

#include "qhttpserverapi.h"
#include "qhttpserverfwd.h"

#include <QObject>
#include <QMetaEnum>
#include <QMetaType>
#include <QUrl>

/// The QHttpRequest class represents the header and body data sent by the client.
/** The requests header data is available immediately. Body data is streamed as
    it comes in via the data() signal. As a consequence the application's request
    callback should ensure that it connects to the data() signal before control
    returns back to the event loop. Otherwise there is a risk of some data never
    being received by the application.

    The class is <b>read-only</b>. */
class QHTTPSERVER_API QHttpRequest : public QObject
{
    Q_OBJECT

    Q_PROPERTY(HeaderHash headers READ headers)
    Q_PROPERTY(QString remoteAddress READ remoteAddress)
    Q_PROPERTY(quint16 remotePort READ remotePort)
    Q_PROPERTY(QString method READ method)
    Q_PROPERTY(QUrl url READ url)
    Q_PROPERTY(QString path READ path)
    Q_PROPERTY(QString httpVersion READ httpVersion)

    Q_ENUMS(HttpMethod)

    /// @cond nodoc
    friend class QHttpConnection;
    /// @endcond

public:
    virtual ~QHttpRequest();

    /// Request method enumeration.
    /** @note Taken from http_parser.h -- make sure to keep synced */
    enum HttpMethod {
        HTTP_DELETE = 0,
        HTTP_GET,
        HTTP_HEAD,
        HTTP_POST,
        HTTP_PUT,
        // pathological
        HTTP_CONNECT,
        HTTP_OPTIONS,
        HTTP_TRACE,
        // webdav
        HTTP_COPY,
        HTTP_LOCK,
        HTTP_MKCOL,
        HTTP_MOVE,
        HTTP_PROPFIND,
        HTTP_PROPPATCH,
        HTTP_SEARCH,
        HTTP_UNLOCK,
        // subversion
        HTTP_REPORT,
        HTTP_MKACTIVITY,
        HTTP_CHECKOUT,
        HTTP_MERGE,
        // upnp
        HTTP_MSEARCH,
        HTTP_NOTIFY,
        HTTP_SUBSCRIBE,
        HTTP_UNSUBSCRIBE,
        // RFC-5789
        HTTP_PATCH,
        HTTP_PURGE
    };

    /// The method used for the request.
    HttpMethod method() const;

    /// Returns the method string for the request.
    /** @note This will plainly transform the enum into a string HTTP_GET -> "HTTP_GET". */
    const QString methodString() const;

    /// The complete URL for the request.
    /** This includes the path and query string.
        @sa path() */
    const QUrl &url() const;

    /// The path portion of the query URL.
    /** @sa url() */
    const QString path() const;

    /// The HTTP version of the request.
    /** @return A string in the form of "x.x" */
    const QString &httpVersion() const;

    /// Return all the headers sent by the client.
    /** This returns a reference. If you want to store headers
        somewhere else, where the request may be deleted,
        make sure you store them as a copy.
        @note All header names are <b>lowercase</b>
        so that Content-Length becomes content-length etc. */
    const HeaderHash &headers() const;

    /// Get the value of a header.
    /** Headers are stored as lowercase so the input @c field will be lowercased.
        @param field Name of the header field
        @return Value of the header or empty string if not found. */
    QString header(const QString &field);

    /// IP Address of the client in dotted decimal format.
    const QString &remoteAddress() const;

    /// Outbound connection port for the client.
    quint16 remotePort() const;

    /// Request body data, empty for non POST/PUT requests.
    /** @sa storeBody() */
    const QByteArray &body() const
    {
        return m_body;
    }

    /// If this request was successfully received.
    /** Set before end() has been emitted, stating whether
        the message was properly received. This is false
        until the receiving the full request has completed. */
    bool successful() const
    {
        return m_success;
    }

    /// Utility function to make this request store all body data internally.
    /** If you call this when the request is received via QHttpServer::newRequest()
        the request will take care of storing the body data for you.
        Once the end() signal is emitted you can access the body data with
        the body() function.

        If you wish to handle incoming data yourself don't call this function
        and see the data() signal.
        @sa data() body() */
    void storeBody();

Q_SIGNALS:
    /// Emitted when new body data has been received.
    /** @note This may be emitted zero or more times
        depending on the request type.
        @param data Received data. */
    void data(const QByteArray &data);

    /// Emitted when the request has been fully received.
    /** @note The no more data() signals will be emitted after this. */
    void end();

private Q_SLOTS:
    void appendBody(const QByteArray &body);

private:
    QHttpRequest(QHttpConnection *connection, QObject *parent = 0);

    static QString MethodToString(HttpMethod method);

    void setMethod(HttpMethod method) { m_method = method; }
    void setVersion(const QString &version) { m_version = version; }
    void setUrl(const QUrl &url) { m_url = url; }
    void setHeaders(const HeaderHash headers) { m_headers = headers; }
    void setSuccessful(bool success) { m_success = success; }

    QHttpConnection *m_connection;
    HeaderHash m_headers;
    HttpMethod m_method;
    QUrl m_url;
    QString m_version;
    QString m_remoteAddress;
    quint16 m_remotePort;
    QByteArray m_body;
    bool m_success;
};

#endif
