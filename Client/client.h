#ifndef CLIENT_H
#define CLIENT_H

#include "connection.h"
#include <QObject>
#include <stdio.h>
#include <winsock2.h>


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
    void reqeustCommand(int command);
signals:
    void receivedHeader(char *data, qint64 len);
    void receivedChunkData(char *data, qint64 len);
    void receivedAvailSongs(char *);
    void receivedPlaylist(char *);
    void receivedProgressData(char *);

public slots:
    void requestSong(QString song);

private:


};

#endif // CLIENT_H
