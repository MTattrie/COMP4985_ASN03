#ifndef SERVER_H
#define SERVER_H

#include "connection.h"
#include <QObject>

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = 0);

    void startTCP();
    void startUDP();

    void workerThread(WSAEVENT event);




signals:
    void update_log(QString packet);

public slots:

private:



};

#endif // SERVER_H
