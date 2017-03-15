#ifndef CONNECTION_H
#define CONNECTION_H

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <string>


#define PORT 7000
#define DATA_BUFSIZE 8192

typedef struct _SOCKET_INFORMATION {
   OVERLAPPED Overlapped;
   SOCKET Socket;
   CHAR Buffer[DATA_BUFSIZE];
   WSABUF DataBuf;
   DWORD BytesSEND;
   DWORD BytesRECV;
} SOCKET_INFORMATION, * LPSOCKET_INFORMATION;


class Connection
{


public:
    Connection();

    void WSAError(std::string method, int error);

    bool WSAStartup();
    bool WSASocket_TCP(SOCKET &s);
    bool listen(SOCKET &s);
    bool bind(SOCKET &s);
    bool WSACreateEvent(WSAEVENT &event);
    bool WSASetEvent(WSAEVENT &event);

    bool WSAWaitForMultipleEvents(WSAEVENT EventArray[]);
    bool WSASend(LPSOCKET_INFORMATION &SI,
            LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
    bool WSARecv(LPSOCKET_INFORMATION &SI,
            LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

    bool createSocketInfo(LPSOCKET_INFORMATION &SocketInfo, SOCKET &s);


    /*
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

*/


};

#endif // CONNECTION_H
