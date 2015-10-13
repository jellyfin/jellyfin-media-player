#include "greeting.h"

#include <QCoreApplication>
#include <QRegExp>
#include <QStringList>

#include <qhttpserver.h>
#include <qhttprequest.h>
#include <qhttpresponse.h>

/// Greeting

Greeting::Greeting()
{
    QHttpServer *server = new QHttpServer(this);
    connect(server, SIGNAL(newRequest(QHttpRequest*, QHttpResponse*)),
            this, SLOT(handleRequest(QHttpRequest*, QHttpResponse*)));
            
    server->listen(QHostAddress::Any, 8080);
}

void Greeting::handleRequest(QHttpRequest *req, QHttpResponse *resp)
{
    QRegExp exp("^/user/([a-z]+)$");
    if( exp.indexIn(req->path()) != -1 )
    {
        resp->setHeader("Content-Type", "text/html");
        resp->writeHead(200);
        
        QString name = exp.capturedTexts()[1];
        QString body = tr("<html><head><title>Greeting App</title></head><body><h1>Hello %1!</h1></body></html>");
        resp->end(body.arg(name).toUtf8());
    }
    else
    {
        resp->writeHead(403);
        resp->end(QByteArray("You aren't allowed here!"));
    }
}

/// main

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    Greeting greeting;
    app.exec();
}
