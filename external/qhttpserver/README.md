QHttpServer
===========

A Qt HTTP Server - because hard-core programmers write web-apps in C++ :)

It uses Joyent's [HTTP Parser](http://github.com/joyent/http-parser) and is asynchronous and does not require any inheritance.

QHttpServer is available under the MIT License.

**NOTE: QHttpServer is NOT fully HTTP compliant right now! DO NOT use it for
anything complex**

Installation
------------

Requires Qt 4 or Qt 5.

    qmake && make && su -c 'make install'

To link to your projects put this in your project's qmake project file

    LIBS += -lqhttpserver

By default, the installation prefix is /usr/local. To change that to /usr,
for example, run:

    qmake -r PREFIX=/usr

Usage
-----

Include the headers

    #include <qhttpserver.h>
    #include <qhttprequest.h>
    #include <qhttpresponse.h>

Create a server, and connect to the signal for new requests

    QHttpServer *server = new QHttpServer;
    connect(server, SIGNAL(newRequest(QHttpRequest*, QHttpResponse*)),
            handler, SLOT(handle(QHttpRequest*, QHttpResponse*)));

    // let's go
    server->listen(8080);

In the handler, you may dispatch on routes or do whatever other things
you want. See the API documentation for what information
is provided about the request via the QHttpRequest object.

To send data back to the browser and end the request:

    void Handler::handle(QHttpRequest *req, QHttpResponse *resp)
    {
    	resp->setHeader("Content-Length", 11);
    	resp->writeHead(200); // everything is OK
    	resp->write("Hello World");
    	resp->end();
    }

The server and request/response objects emit various signals
and have guarantees about memory management. See the API documentation for
these.

Contribute
----------

Feel free to file issues, branch and send pull requests. If you plan to work on a major feature (say WebSocket support), please run it by me first by filing an issue! qhttpserver has a narrow scope and API and I'd like to keep it that way, so a thousand line patch that implements the kitchen sink is unlikely to be accepted.

- Nikhil Marathe (maintainer)

Everybody who has ever contributed shows up in [Contributors](https://github.com/nikhilm/qhttpserver/graphs/contributors).
