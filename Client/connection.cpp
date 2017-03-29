#include "connection.h"
#include <QDebug>
#include <string>

using std::string;

Connection::Connection()
{

}




bool Connection::WSAStartup(){
    WSADATA wsa;
    DWORD Ret;
    if ((Ret = ::WSAStartup(0x0202, &wsa)) != 0)
    {
        qDebug() << "Connection::WSAStartup() WSAStartup() failed with error " << Ret;
        return false;
    }
    return true;
}


bool Connection::WSASocketTCP(SOCKET &sd){
    if ((sd = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, 0)) == INVALID_SOCKET)
    {
        qDebug() << "Connection::WSASocket() failed with error " << WSAGetLastError();
        return false;
    }
    return true;
}




bool Connection::connect(SOCKET &sd, string host, int port){
    struct hostent	*hp;
    struct sockaddr_in server;

    memset((char *)&server, 0, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    if ((hp = gethostbyname(host.c_str())) == NULL) {
        qDebug() << "Connection::connect() Unknown server address";
        return false;
    }
    memcpy((char *)&server.sin_addr, hp->h_addr, hp->h_length);

    if (::connect (sd, (struct sockaddr *)&server, sizeof(server)) == -1) {
        qDebug() << "Connection::connect() Can't connect to server " << GetLastError();
        return false;
    }
    qDebug() << "Connection::connect() New socket:" << sd << " host:" << host.c_str() << " port:" << port;
    return true;
}



bool Connection::send(SOCKET &sd, char buffer[]){
    DWORD bytes_to_send = DATA_BUFSIZE;
    char *bp = buffer;
    int n;
    while ((n = ::send(sd, bp, bytes_to_send, 0)) < DATA_BUFSIZE)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            qDebug() << "Connection::WSASend() failed with error" << WSAGetLastError();
            return false;
        }
        bp += n;
        bytes_to_send -= n;
        if (n == 0)
            break;
    }
    qDebug() << "Client::send() buffer contents: " << buffer;
    return true;
}

bool Connection::recv(SOCKET &sd, char buffer[]){
    DWORD bytes_to_read = DATA_BUFSIZE;
    char * bp = buffer;
    int n;
    while ((n = ::recv(sd, bp, bytes_to_read, 0)) < DATA_BUFSIZE)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            qDebug() << "Connection::WSARecv() failed with error" << WSAGetLastError();
            return false;
        }
        bp += n;
        bytes_to_read -= n;
        if (n == 0)
            break;
    }
    qDebug() << "Client::recv() buffer contents: " << buffer;
    return true;
}


bool Connection::WSAEventSelect(SOCKET &sd, WSAEVENT &event, long networkEvents){
    if (::WSAEventSelect(sd, event, networkEvents) == SOCKET_ERROR)
    {
        qDebug() << "Connection::WSAEventSelect() failed with error " << WSAGetLastError();
        return false;
    }
    return true;
}


bool Connection::WSACreateEvent(WSAEVENT &event){
    if ((event = ::WSACreateEvent()) == WSA_INVALID_EVENT)
    {
        qDebug() << "Connection::WSACreateEvent() failed with error " << WSAGetLastError();
        return false;
    }
    return true;
}


bool Connection::WSAWaitForMultipleEvents(WSAEVENT &event){
    if ((::WSAWaitForMultipleEvents(1, &event, FALSE, WSA_INFINITE, FALSE)) == WSA_WAIT_FAILED)
    {
        qDebug() << "Connection::WSAWaitForMultipleEvents() failed with error " << WSAGetLastError();
        return false;
    }
    return true;
}

