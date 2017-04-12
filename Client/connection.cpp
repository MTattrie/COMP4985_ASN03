#include "connection.h"

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <QDebug>
#include <string>
#include <ws2tcpip.h>

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



/* ================================================================================== *
 *                                   SOCKETS
 * ================================================================================== */


bool Connection::WSASocketTCP(SOCKET &s){
    if ((s = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, 0)) == INVALID_SOCKET)
    {
        qDebug() << "Connection::WSASocketTCP() failed with error " << WSAGetLastError();
        return false;
    }
    qDebug() << "Connection::WSASocketTCP() Setup TCP socket " << s;
    return true;
}


bool Connection::WSASocketUDP(SOCKET &s){
    if ((s = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, 0)) == INVALID_SOCKET)
    {
        qDebug() << "Connection::WSASocketUDP() failed with error " << WSAGetLastError();
        return false;
    }
    qDebug() << "Connection::WSASocketUDP() Setup UDP socket " << s;
    return true;
}


bool Connection::bind(SOCKET &s, sockaddr_in &server, int port){
    memset((char *)&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    if (::bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
    {
       qDebug() << "Connection::bind() failed with error " << WSAGetLastError();
       return false;
    }
    qDebug() << "Connection::bind() Bind socket " << s << " to port " << port;
    return true;
}


bool Connection::connect(SOCKET &s, string host, int port){
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

    if (::connect (s, (struct sockaddr *)&server, sizeof(server)) == -1) {
        qDebug() << "Connection::connect() Can't connect to server " << GetLastError();
        return false;
    }
    qDebug() << "Connection::connect() New socket:" << s << " host:" << host.c_str() << " port:" << port;
    return true;
}


bool Connection::setoptSO_REUSEADDR(SOCKET &s){
    int opt = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt)) < 0)
    {
       qDebug() << "WSASocket() failed with error " << WSAGetLastError();
        return false;
    }
    return true;
}


bool Connection::setoptIP_ADD_MEMBERSHIP(SOCKET &s){
    struct ip_mreq   MulticastAddress;
    MulticastAddress.imr_multiaddr.s_addr = inet_addr(MULTICASTSERVER);
    MulticastAddress.imr_interface.s_addr = INADDR_ANY;
    if(::setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&MulticastAddress, sizeof(MulticastAddress)) == SOCKET_ERROR)
    {
        qDebug() << "setsockopt() failed with error on multicast address " <<  WSAGetLastError();
        return false;
    }
    return true;
}

/* ================================================================================== *
 *                                   SEND/RECV
 * ================================================================================== */



bool Connection::send(SOCKET &s, char buffer[]){
    DWORD bytes_to_send = BUFFERSIZE;
    char *bp = buffer;
    int n;
    while ((n = ::send(s, bp, bytes_to_send, 0)) < BUFFERSIZE)
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
    qDebug() << "Client::send() buffer BUFFERSIZE: " << BUFFERSIZE;

    return true;
}


int Connection::sendto(SOCKET &s, sockaddr_in &server, char buffer[], int len){
    DWORD bytes_to_send = len;
    char *bp = buffer;
    int n;
    n = ::sendto(s, bp, bytes_to_send, 0, (struct sockaddr *)&server, sizeof(server));
    return n;
}

bool Connection::recv(SOCKET &s, char buffer[]){
    DWORD bytes_to_read = BUFFERSIZE;
    char * bp = buffer;
    int n;
    while (n = ::recv(s, bp, bytes_to_read, 0))
    {
        int error = WSAGetLastError();
        if (error != WSA_IO_PENDING && error != 0)
        {
            if(error == 10035)
                continue;
            qDebug() << "Connection::WSARecv() failed with error" << error;
            return false;
        }
        bp += n;
        bytes_to_read -= n;
        if (n == 0)
            break;
        if(bytes_to_read == 0)
            break;
    }
    qDebug() << "Client::recv() buffer contents: " << buffer;
    return true;
}

int Connection::recvfrom(SOCKET &s, sockaddr_in &server, char buffer[]){
    DWORD bytes_to_read = BUFFERSIZE;
    char * bp = buffer;
    int n;
    int serverlen = sizeof(server);
    n = ::recvfrom(s, bp, bytes_to_read, 0, (struct sockaddr *)&server, &serverlen);

    return n;
}


/* ================================================================================== *
 *                                   EVENTS
 * ================================================================================== */


bool Connection::WSAEventSelect(SOCKET &s, WSAEVENT &event, long networkEvents){
    if (::WSAEventSelect(s, event, networkEvents) == SOCKET_ERROR)
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
