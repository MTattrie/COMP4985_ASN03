#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>

#include <stdio.h>
#include <winsock2.h>

#include "connection.h"

class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = 0);

    void start();

    void startTCP();
    void connectTCP();
    void runTCP();

    void startUDP();
    void connectUDP();
    void runUDP();


signals:

public slots:
    void requestSong(QString song);

private:


};

#endif // CLIENT_H
