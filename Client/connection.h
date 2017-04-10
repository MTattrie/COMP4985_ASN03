#ifndef CONNECTION_H
#define CONNECTION_H
#include "packet.h"

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
   DWORD BytesToSend;
   int mode;
} SOCKET_INFORMATION, * LPSOCKET_INFORMATION;


class Connection
{
public:
    Connection();

    bool WSAStartup();

    bool WSASocketTCP(SOCKET &s);
    bool WSASocketUDP(SOCKET &s);
    bool bind(SOCKET &s, sockaddr_in &server, int port);
    bool connect(SOCKET &s, string host, int port);
    bool setoptSO_REUSEADDR(SOCKET &s);
    bool setoptIP_ADD_MEMBERSHIP(SOCKET &s);

    bool WSAEventSelect(SOCKET &s, WSAEVENT &event, long networkEvents);
    bool WSACreateEvent(WSAEVENT &event);
    bool WSAWaitForMultipleEvents(WSAEVENT &event);

    bool send(SOCKET &sd, char buffer[]);
    int recv(SOCKET &sd, char buffer[]);

    bool sendto(SOCKET &s, sockaddr_in &server, char buffer[]);
    bool recvfrom(SOCKET &s, sockaddr_in &server, char buffer[]);

};

#endif // CONNECTION_H
