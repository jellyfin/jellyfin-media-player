#include "qhttpserverfwd.h"

#include <QObject>

/// Greeting

class Greeting : public QObject
{
    Q_OBJECT

public:
    Greeting();

private slots:
    void handleRequest(QHttpRequest *req, QHttpResponse *resp);
};
