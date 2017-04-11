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

    void start(QString hostname, QString port);

    void startTCP();
    void connectTCP();
    void runTCP();

    void startUDP();
    void connectUDP();
    void runUDP();
    void reqeustCommand(int command, QString data = "");
    void storeServerDetails(QString hostname, QString port);

    bool sendFile(QString filename);
    bool loadFile(QQueue<QByteArray>& packets, const QString filename);

    QString filenames;
    QByteArray downloads;
    bool isDonwloading;
    bool isUploading;
signals:
    void receivedHeader(char *data, qint64 len);
    void receivedChunkData(char *data, qint64 len);
    void receivedAvailSongs(char *);
    void receivedPlaylist(char *);
    void receivedProgressData(char *);
    void receivedAddPlaylist(QString);
    void receivedCommand(int, char*, int);

public slots:
    void requestSong(QString song);
    void sendSong(QString song);

private:
    std::string serverHostName;
    int portNumber;



};

#endif // CLIENT_H
