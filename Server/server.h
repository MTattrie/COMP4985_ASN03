#ifndef SERVER_H
#define SERVER_H

#include "connection.h"
#include <QObject>



struct Client
{
      SOCKET_INFORMATION SocketInfo;
      SOCKADDR_IN        Connection;
};



class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = 0);

    void start();

    void startTCP();
    void connectTCP();
    void runTCP();
    void acceptThread(WSAEVENT event);
    void readThread();


    void startUDP();
    void connectUDP();
    void runUDP();


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



};

#endif // SERVER_H
