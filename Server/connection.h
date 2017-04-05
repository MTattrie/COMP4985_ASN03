#ifndef CONNECTION_H
#define CONNECTION_H

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <string>

#define DATA_BUFSIZE 8192

#define MODE_SEND    0
#define MODE_RECEIVE 1
#define MODE_COMMAND 2

typedef struct _SOCKET_INFORMATION {
   OVERLAPPED Overlapped;
   SOCKET socket_tcp;
   SOCKET socket_udp;
   struct sockaddr_in client_address;
   CHAR Buffer[DATA_BUFSIZE];
   WSABUF DataBuf;
   DWORD BytesSEND;
   DWORD BytesRECV;
} SOCKET_INFORMATION, * LPSOCKET_INFORMATION;


class Connection
{


public:
    Connection() = default;

    void WSAError(std::string method, int error);

    bool WSAStartup();
    bool WSASocketTCP(SOCKET &s);
    bool WSASocketUDP(SOCKET &s);

    bool setsockopt(SOCKET &socket, int level, int option, const char* value);
    bool listen(SOCKET &s);
    bool bind(SOCKET &s, int port);

    bool WSACreateEvent(WSAEVENT &event);
    bool WSAEventSelect(SOCKET &sd, WSAEVENT &event, long networkEvents);
    bool WSASetEvent(WSAEVENT &event);
    bool WSAWaitForMultipleEvents(WSAEVENT &event);

    bool WSASend(LPSOCKET_INFORMATION &SI,
            LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

    bool WSASendTo(LPSOCKET_INFORMATION &SI);//udp

    bool WSARecv(LPSOCKET_INFORMATION &SI,
            LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
    bool WSARecvFrom(LPSOCKET_INFORMATION SI,
            LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

    bool createSocketInfo(LPSOCKET_INFORMATION &SocketInfo, SOCKET s);
    bool checkError(LPSOCKET_INFORMATION &SI, DWORD error);
    bool checkFinished(LPSOCKET_INFORMATION &SI, DWORD BytesTransferred);

    int join_group(int &sd, unsigned long grpaddr);
    int leave_group(int &sd, unsigned long grpaddr);




};

#endif // CONNECTION_H
