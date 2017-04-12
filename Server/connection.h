#ifndef CONNECTION_H
#define CONNECTION_H

#include "global.h"

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <string>
#include <QByteArray>

typedef struct _SOCKET_INFORMATION {
   OVERLAPPED Overlapped;
   SOCKET Socket;
   struct sockaddr_in client_address;
   CHAR Buffer[BUFFERSIZE];
   WSABUF DataBuf;
   DWORD BytesSEND;
   DWORD BytesRECV;
   DWORD BytesToSend;
   QByteArray *upload;
   void *thisObj;
} SOCKET_INFORMATION, * LPSOCKET_INFORMATION;


class Connection
{


public:
    Connection() = default;

    void WSAError(std::string method, int error);

    bool WSAStartup();

    bool WSASocketTCP(SOCKET &s);
    bool WSASocketUDP(SOCKET &s);
    bool listen(SOCKET &s);
    bool bind(SOCKET &s, int port);
    bool setoptSO_REUSEADDR(SOCKET &s);
    bool setoptIP_MULTICAST_TTL(SOCKET &s);
    bool setoptIP_MULTICAST_LOOP(SOCKET &s);
    bool setoptIP_ADD_MEMBERSHIP(SOCKET &s);

    bool WSACreateEvent(WSAEVENT &event);
    bool WSAEventSelect(SOCKET &sd, WSAEVENT &event, long networkEvents);
    bool WSASetEvent(WSAEVENT &event);
    bool WSAWaitForMultipleEvents(WSAEVENT &event);


    bool WSASend(LPSOCKET_INFORMATION &SI,
            LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

    bool WSASendTo(LPSOCKET_INFORMATION &SI,
                   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);//udp

    bool WSARecv(LPSOCKET_INFORMATION &SI,
            LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
    bool WSARecvFrom(LPSOCKET_INFORMATION SI,
            LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);


    bool createSocketInfo(LPSOCKET_INFORMATION &SocketInfo, SOCKET s);
    bool checkError(LPSOCKET_INFORMATION &SI, DWORD error);
    bool checkFinished(LPSOCKET_INFORMATION &SI, DWORD BytesTransferred);

    int port;
};

#endif // CONNECTION_H
