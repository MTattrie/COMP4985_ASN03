#include "server.h"
#include "connection.h"

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <thread>
#include <QDebug>
#include <string>

using namespace std;

Server::Server(QObject *parent) : QObject(parent)
{

}


void Server::startTCP(){
    Connection c;

    SOCKET Listen;
    SOCKET Accept;
    SOCKADDR_IN InternetAddr;
    DWORD Event;
    WSANETWORKEVENTS NetworkEvents;
    WSADATA wsaData;

    c._WSAStartup(wsaData);
    c._SocketTCP(Listen);
    c.CreateSocketInformation(Listen);
    c._WSAEventSelect(Listen, FD_ACCEPT|FD_CLOSE);

    InternetAddr.sin_family = AF_INET;
    InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    InternetAddr.sin_port = htons(PORT_TCP);

    c._Bind(Listen, InternetAddr);
    c._Listen(Listen);

    while(TRUE)
    {
        c._WSAWaitForMultipleEvents(Event);
        c._WSAEnumNetworkEvents(Event, NetworkEvents);

        if(!c.ReceiveNewConnection(Accept, Event, NetworkEvents)){
            break;
        }

        string buffer;
        c.ReadData(buffer, NetworkEvents, Event);

        QString log = QString::fromStdString(buffer);
        emit update_log(log);

        if(!c.CloseSocket(Event, NetworkEvents)){
            break;
        }
    }
}

void Server::startUDP(){
    Connection c;

    SOCKET Listen;
    SOCKADDR_IN InternetAddr;
    DWORD Event;
    WSANETWORKEVENTS NetworkEvents;
    WSADATA wsaData;

    c._WSAStartup(wsaData);
    c._SocketUDP(Listen);

    InternetAddr.sin_family = AF_INET;
    InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    InternetAddr.sin_port = htons(PORT_UDP);

    c._Bind(Listen, InternetAddr);

    while(TRUE)
    {

        char buf[1024];
        int nbytes;
        if ((nbytes = recv(Listen, buf, 1024, 0)) < 0)
        {
            perror ("recvfrom error");
        }

        string buffer(buf);

        QString log = QString::fromStdString(buffer);
        emit update_log2(log);

        if(!c.CloseSocket(Event, NetworkEvents)){
            break;
        }
    }
}

