#ifndef CONNECTION_H
#define CONNECTION_H

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <string>

using std::string;

#define PORT_TCP 5150
#define PORT_UDP 5151
#define DATA_BUFSIZE 1024

typedef struct _SOCKET_INFORMATION {
   CHAR Buffer[DATA_BUFSIZE];
   WSABUF DataBuf;
   SOCKET Socket;
   DWORD BytesSEND;
   DWORD BytesRECV;
} SOCKET_INFORMATION, * LPSOCKET_INFORMATION;


class Connection
{


public:
    DWORD                EventTotal = 0;
    WSAEVENT             EventArray[WSA_MAXIMUM_WAIT_EVENTS];
    LPSOCKET_INFORMATION SocketArray[WSA_MAXIMUM_WAIT_EVENTS];

    Connection();

    bool _WSAStartup(WSADATA &wsa);
    bool _WSAEventSelect(SOCKET &s, long lNetworkEvents);
    bool _SocketTCP(SOCKET &s);
    bool _SocketUDP(SOCKET &s);
    bool _Bind(SOCKET &s, SOCKADDR_IN &InternetAddr);
    bool _Connect(SOCKET &s, SOCKADDR_IN &clientService);
    bool _Listen(SOCKET &s);
    bool _WSAWaitForMultipleEvents(DWORD &Event);
    bool _WSAEnumNetworkEvents(DWORD &Event, WSANETWORKEVENTS &NetworkEvents);
    bool _Accept(SOCKET &s, DWORD &Event);
    bool _WSARecv(LPSOCKET_INFORMATION &SocketInfo, DWORD &Event);
    bool _Send(SOCKET &s, char *sendbuf, int bytes);
    bool _Shutdown(SOCKET &s);


    bool ReadData(string &buffer, WSANETWORKEVENTS &NetworkEvents, DWORD &Event);
    bool ReceiveNewConnection(SOCKET &s, DWORD &Event, WSANETWORKEVENTS &NetworkEvents);
    bool CloseSocket(DWORD &Event, WSANETWORKEVENTS &NetworkEvents);
    bool CreateSocketInformation(SOCKET s);
    void FreeSocketInformation(DWORD Event);



};

#endif // CONNECTION_H
