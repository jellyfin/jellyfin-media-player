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

#include "qhttprequest.h"

#include "qhttpconnection.h"

QHttpRequest::QHttpRequest(QHttpConnection *connection, QObject *parent)
    : QObject(parent), m_connection(connection), m_url("http://localhost/"), m_success(false)
{
}

QHttpRequest::~QHttpRequest()
{
}

QString QHttpRequest::header(const QString &field)
{
    return m_headers.value(field.toLower(), "");
}

const HeaderHash &QHttpRequest::headers() const
{
    return m_headers;
}

const QString &QHttpRequest::httpVersion() const
{
    return m_version;
}

const QUrl &QHttpRequest::url() const
{
    return m_url;
}

const QString QHttpRequest::path() const
{
    return m_url.path();
}

const QString QHttpRequest::methodString() const
{
    return MethodToString(method());
}

QHttpRequest::HttpMethod QHttpRequest::method() const
{
    return m_method;
}

const QString &QHttpRequest::remoteAddress() const
{
    return m_remoteAddress;
}

quint16 QHttpRequest::remotePort() const
{
    return m_remotePort;
}

void QHttpRequest::storeBody()
{
    connect(this, SIGNAL(data(const QByteArray &)), this, SLOT(appendBody(const QByteArray &)),
            Qt::UniqueConnection);
}

QString QHttpRequest::MethodToString(HttpMethod method)
{
    int index = staticMetaObject.indexOfEnumerator("HttpMethod");
    return staticMetaObject.enumerator(index).valueToKey(method);
}

void QHttpRequest::appendBody(const QByteArray &body)
{
    m_body.append(body);
}
