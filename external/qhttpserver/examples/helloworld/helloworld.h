#include "qhttpserverfwd.h"

#include <QObject>

/// HelloWorld

class HelloWorld : public QObject
{
    Q_OBJECT

public:
    HelloWorld();

private slots:
    void handleRequest(QHttpRequest *req, QHttpResponse *resp);
};
