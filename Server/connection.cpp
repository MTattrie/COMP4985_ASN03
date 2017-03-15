#include "connection.h"

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <QDebug>
#include <string>



Connection::Connection()
{

}


void Connection::WSAError(std::string method, int error){
    qDebug() << method.c_str() << " failed with error " << error << endl;
    WSACleanup();
    exit(1);
}


bool Connection::WSAStartup(){
    WSADATA wsa;
    DWORD Ret;
    if ((Ret = ::WSAStartup(0x0202, &wsa)) != 0)
    {
        WSAError("WSAStartup()", Ret);
        return false;
    }
    return true;
}


bool Connection::WSASocket_TCP(SOCKET &s){
    if ((s = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0,
            WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        WSAError("WSASocket()", WSAGetLastError());
        return false;
    }
    return true;
}


bool Connection::bind(SOCKET &s){
    SOCKADDR_IN InternetAddr;
    InternetAddr.sin_family = AF_INET;
    InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    InternetAddr.sin_port = htons(PORT);

    if (::bind(s, (PSOCKADDR) &InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
    {
       WSAError("bind()", WSAGetLastError());
       return false;
    }
    return true;
}


bool Connection::listen(SOCKET &s){
    int backlog = 5;
    if (::listen(s, backlog))
    {
        WSAError("listen()", WSAGetLastError());
        return false;
    }
    return true;
}


bool Connection::WSACreateEvent(WSAEVENT &event){
    if ((event = ::WSACreateEvent()) == WSA_INVALID_EVENT)
    {
        WSAError("WSACreateEvent()", WSAGetLastError());
        return false;
    }
    return true;
}


bool Connection::WSASetEvent(WSAEVENT &event){
    if (::WSASetEvent(event) == FALSE)
    {
        WSAError("WSASetEvent()", WSAGetLastError());
        return false;
    }
    return true;
}


bool Connection::WSAWaitForMultipleEvents(WSAEVENT EventArray[]){
    DWORD index;
    while(TRUE)
    {
        index = ::WSAWaitForMultipleEvents(1, EventArray, FALSE, WSA_INFINITE, TRUE);
        if (index == WSA_WAIT_FAILED)
        {
            WSAError("WSAWaitForMultipleEvents()", WSAGetLastError());
            return false;
        }
        if (index != WAIT_IO_COMPLETION)
        {
            break;
        }
    }
    WSAResetEvent(EventArray[index - WSA_WAIT_EVENT_0]);
    return true;
}


bool Connection::createSocketInfo(LPSOCKET_INFORMATION &SocketInfo, SOCKET &s){
    if ((SocketInfo = (LPSOCKET_INFORMATION) GlobalAlloc(GPTR, sizeof(SOCKET_INFORMATION))) == NULL)
    {
        WSAError("GlobalAlloc()", GetLastError());
        return false;
    }
    SocketInfo->Socket = s;
    ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));
    SocketInfo->BytesSEND = 0;
    SocketInfo->BytesRECV = 0;
    SocketInfo->DataBuf.len = DATA_BUFSIZE;
    SocketInfo->DataBuf.buf = SocketInfo->Buffer;
    return true;
}


bool Connection::WSASend(LPSOCKET_INFORMATION &SI,
        LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine){
    DWORD SendBytes;
    if (::WSASend(SI->Socket, &(SI->DataBuf), 1, &SendBytes, 0,
            &(SI->Overlapped), lpCompletionRoutine) == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            WSAError("WSASend()", WSAGetLastError());
            return false;
        }
    }
    return true;
}


bool Connection::WSARecv(LPSOCKET_INFORMATION &SI,
        LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine){
    DWORD Flags = 0;
    DWORD RecvBytes;
    if (::WSARecv(SI->Socket, &(SI->DataBuf), 1, &RecvBytes, &Flags,
            &(SI->Overlapped), lpCompletionRoutine) == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            WSAError("WSARecv()", WSAGetLastError());
            return false;
        }
    }
    return true;
}


/*
//FD_ACCEPT|FD_CLOSE
bool Connection::_WSAEventSelect(SOCKET &s, long lNetworkEvents){
    if (WSAEventSelect(s, EventArray[EventTotal - 1], lNetworkEvents) == SOCKET_ERROR)
    {

        return false;
    }
    return true;
}



bool Connection::_SocketTCP(SOCKET &s){
    if ((s = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
    {
        qDebug() << "socket() failed with error " << WSAGetLastError() << endl;
        return false;
    }
    return true;
}
bool Connection::_SocketUDP(SOCKET &s){
    if ((s = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
    {
        qDebug() << "socket() failed with error " << WSAGetLastError() << endl;
        return false;
    }
    return true;
}





bool Connection::_Connect(SOCKET &s, SOCKADDR_IN &clientService){
    // Connect to server.
    int iResult;
    iResult = WSAConnect( s, (SOCKADDR*) &clientService, sizeof(clientService), NULL, NULL, NULL, NULL);
    if (iResult == SOCKET_ERROR) {
        wprintf(L"connect failed with error: %d\n", WSAGetLastError() );
        return false;
    }
    return true;
}

bool Connection::_Send(SOCKET &s, char *sendbuf, int bytes){
    int iResult;
    iResult = send( s, sendbuf, bytes, 0 );
    if (iResult == SOCKET_ERROR) {
        qDebug() << "send failed with error: " << WSAGetLastError() << endl;
        return false;
    }
    return true;
}

bool Connection::_Shutdown(SOCKET &s){
    int iResult;
    iResult = shutdown(s, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        qDebug() << "shutdown failed with error: " << WSAGetLastError() << endl;
        return false;
    }
    return true;
}



bool Connection::_WSAWaitForMultipleEvents(DWORD &Event){
    // Wait for one of the sockets to receive I/O notification and
    if ((Event = WSAWaitForMultipleEvents(EventTotal, EventArray, FALSE,
       WSA_INFINITE, FALSE)) == WSA_WAIT_FAILED)
    {
       qDebug() << "WSAWaitForMultipleEvents failed with error " << WSAGetLastError() << endl;
       return false;
    }
    return true;
}

bool Connection::_WSAEnumNetworkEvents(DWORD &Event, WSANETWORKEVENTS &NetworkEvents){
    if (WSAEnumNetworkEvents(SocketArray[Event - WSA_WAIT_EVENT_0]->Socket, EventArray[Event -
       WSA_WAIT_EVENT_0], &NetworkEvents) == SOCKET_ERROR)
    {
       qDebug() << "WSAEnumNetworkEvents failed with error " << WSAGetLastError() << endl;
       return false;
    }
    return true;
}

bool Connection::_Accept(SOCKET &s, DWORD &Event){
    if ((s = accept(SocketArray[Event - WSA_WAIT_EVENT_0]->Socket, NULL, NULL)) == INVALID_SOCKET)
    {
       qDebug() << "accept() failed with error " << WSAGetLastError() << endl;
       return false;
    }
    return true;
}

bool Connection::_WSARecv(LPSOCKET_INFORMATION &SocketInfo, DWORD &Event){
    DWORD Flags = 0;
    DWORD RecvBytes;

    if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes,
        &Flags, NULL, NULL) == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSAEWOULDBLOCK)
        {
            qDebug() << "WSARecv() failed with error" << WSAGetLastError() << endl;
            FreeSocketInformation(Event - WSA_WAIT_EVENT_0);
            return false;
        }
    }
    else
    {
        SocketInfo->BytesRECV = RecvBytes;
        return true;
    }
    return false;
}


*/

