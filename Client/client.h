#ifndef CLIENT_H
#define CLIENT_H

#include "connection.h"
#include <QObject>
#include <stdio.h>
#include <winsock2.h>
#include <QQueue>


class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = 0);

    void start(QString hostname, QString port);

    void startTCP();
    void connectTCP();
    void runTCP();

    void startUDP();
    void connectUDP();
    void runUDP();
    void reqeustCommand(int command, QString data = "");
    void storeServerDetails(QString hostname, QString port);

    bool startPeerUDP(QString, QString);
    void peerUDPRead();
    void peerUDPSend();
    void addMicStream(QByteArray &&data);

    bool sendFile(QString filename);
    bool loadFile(QQueue<QByteArray>& packets, const QString filename);

    QString filenames;
    QByteArray downloads;
    bool isDonwloading;
    bool peerUDPRunning;
    bool isUploading;
signals:
    void receivedHeader(char *data, qint64 len);
    void receivedChunkData(char *data, qint64 len);
    void receivedAvailSongs(char *);
    void receivedPlaylist(char *);
    void receivedProgressData(char *);
    void receivedAddPlaylist(QString);
    void receivedCommand(int, char*, int);
    void receivedPeerData(char*, int);

public slots:
    void requestSong(QString song);
    void sendSong(QString song);

private:
    std::string serverHostName;
    int portNumber;
    QQueue<QByteArray> micQueue;

    Connection conn;
    SOCKET socket_tcp;
    SOCKET socket_udp;
    SOCKET socket_peerUDP;
    sockaddr_in server;
    sockaddr_in peer_addr;

};

#endif // CLIENT_H
