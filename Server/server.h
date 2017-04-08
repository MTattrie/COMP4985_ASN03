#ifndef SERVER_H
#define SERVER_H

#include "connection.h"
#include <QObject>
#include <QQueue>

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = 0);

    bool setPort(QString _port);
    void startTCP();
    void startUDP();
    void runTCP();
    void connect();

    void addStreamData(QByteArray data);

    void acceptThread(WSAEVENT event);
    void readThread();

    void UDPMulticast();

    bool multicast(char *message, const int len);

    static void CALLBACK WorkerRoutine_RecvCommand(DWORD Error, DWORD BytesTransferred,
            LPWSAOVERLAPPED Overlapped, DWORD InFlags);

    static void CALLBACK WorkerRoutine_SendSong(DWORD Error, DWORD BytesTransferred,
            LPWSAOVERLAPPED Overlapped, DWORD InFlags);

    static void CALLBACK WorkerRoutine_SendList(DWORD Error, DWORD BytesTransferred,
            LPWSAOVERLAPPED Overlapped, DWORD InFlags);


signals:
    void update_log(QString packet);

public slots:

private:
    QQueue<QByteArray> streamQueue;


};

#endif // SERVER_H
