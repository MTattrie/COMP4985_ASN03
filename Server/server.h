#ifndef SERVER_H
#define SERVER_H

#include "connection.h"
#include <QObject>
#include <QQueue>




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
    void addStreamData(QByteArray data);
    void resetStreamData();
    void acceptThread(WSAEVENT event);
    void readThread();

    bool setPort(QString _port);
    void startUDP();
    void connectUDP();
    void runUDP();

    char *getClientIP(int client_num);



    bool multicast(char *message, const int len);

    static void CALLBACK WorkerRoutine_RecvCommand(DWORD Error, DWORD BytesTransferred,
            LPWSAOVERLAPPED Overlapped, DWORD InFlags);

    static void CALLBACK WorkerRoutine_SendSong(DWORD Error, DWORD BytesTransferred,
            LPWSAOVERLAPPED Overlapped, DWORD InFlags);

    static void CALLBACK WorkerRoutine_SendList(DWORD Error, DWORD BytesTransferred,
            LPWSAOVERLAPPED Overlapped, DWORD InFlags);

    static void CALLBACK WorkerRoutine_UDPSend(DWORD Error, DWORD BytesTransferred,
            LPWSAOVERLAPPED Overlapped, DWORD InFlags);
    static void CALLBACK WorkerRoutine_TCPSend(DWORD Error, DWORD BytesTransferred,
            LPWSAOVERLAPPED Overlapped, DWORD InFlags);
    void sendToClient(int client_num, int command, QByteArray data);

    void TCPBroadCast(int command, QByteArray data = "");

signals:
    void update_log(QString packet);
    void receivedCommand(int command);
    void newClientConnected(int client_num);
    void clientDisconnected(QString);

    void receivedAddPlaylist(QString);

public slots:

private:
    QQueue<QByteArray> streamQueue;
    int port;


};

#endif // SERVER_H
