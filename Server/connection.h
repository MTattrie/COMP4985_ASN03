#ifndef CONNECTION_H
#define CONNECTION_H

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <string>


#define PORT 7000
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

    void WSAError(std::string method, int error);

    bool WSAStartup();
    bool WSASocketTCP(SOCKET &s);
    bool WSASocketUDP(SOCKET &s);
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
    bool checkError(LPSOCKET_INFORMATION &SI, DWORD error);
    bool checkFinished(LPSOCKET_INFORMATION &SI, DWORD BytesTransferred);

    bool setsockopt(int &socket, int level, int option, const char* value);


};

#endif // CONNECTION_H
