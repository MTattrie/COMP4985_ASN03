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
        qDebug() << "WSAStartup()" << Ret << endl;
        return false;
    }
    return true;
}


bool Connection::WSASocketTCP(SOCKET &sd){
    if ((sd = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, 0)) == INVALID_SOCKET)
    {
        qDebug() << "WSASocket() failed with error " << WSAGetLastError() << endl;
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
        qDebug() << "Unknown server address" << endl;
        return false;
    }
    memcpy((char *)&server.sin_addr, hp->h_addr, hp->h_length);

    if (::connect (sd, (struct sockaddr *)&server, sizeof(server)) == -1) {
        qDebug() << "Can't connect to server " << GetLastError() << endl;
        return false;
    }
    return true;
}



bool Connection::WSASend(SOCKET &sd, char buffer[]){
    DWORD SendBytes;
    if (::WSASend(sd, (LPWSABUF)&buffer, 1, &SendBytes, 0, NULL, NULL) == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            qDebug() << "WSASend() failed with error" << WSAGetLastError() << endl;
            return false;
        }
    }
    return true;
}

bool Connection::WSARecv(SOCKET &sd, char buffer[]){
    DWORD RecvBytes = DATA_BUFSIZE;
    if (::WSARecv(sd, (LPWSABUF)&buffer, 1, &RecvBytes, 0, NULL, NULL) == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            qDebug() << "WSARecv() failed with error" << WSAGetLastError() << endl;
            return false;
        }
    }
    return true;
}



