#ifndef CONNECTION_H
#define CONNECTION_H

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <string>

using std::string;

#define SERVER_TCP_PORT 7000
#define DATA_BUFSIZE 8192

#define MODE_SEND    0
#define MODE_RECEIVE 1
#define MODE_COMMAND 2

typedef struct _SOCKET_INFORMATION {
   OVERLAPPED Overlapped;
   SOCKET Socket;
   CHAR Buffer[DATA_BUFSIZE];
   WSABUF DataBuf;
   DWORD BytesSEND;
   DWORD BytesRECV;
   int mode;
} SOCKET_INFORMATION, * LPSOCKET_INFORMATION;


class Connection
{
public:
    Connection();

    bool WSAStartup();
    bool WSASocketTCP(SOCKET &s);
    bool connect(SOCKET &sd, string host, int port);

    bool WSASend(SOCKET &sd, char buffer[]);
    bool WSARecv(SOCKET &sd, char buffer[]);



};

#endif // CONNECTION_H
