#include "connection.h"

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <QDebug>
#include <string>


bool Connection::WSAStartup(){
    WSADATA wsa;
    DWORD Ret;
    if ((Ret = ::WSAStartup(0x0202, &wsa)) != 0)
    {
        qDebug() << "Connection::WSAStartup() failed with error " << Ret;
        return false;
    }
    return true;
}


bool Connection::WSASocketTCP(SOCKET &s){
    if ((s = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0,
            WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
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


bool Connection::bind(SOCKET &s, int port){
    SOCKADDR_IN InternetAddr;
    InternetAddr.sin_family = AF_INET;
    InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    InternetAddr.sin_port = htons(port);

    if (::bind(s, (PSOCKADDR) &InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
    {
       qDebug() << "Connection::bind() failed with error " << WSAGetLastError();
       return false;
    }
    qDebug() << "Connection::bind() Bind socket " << s << " to port " << port;
    return true;
}


bool Connection::listen(SOCKET &s){
    int backlog = 5;
    if (::listen(s, backlog))
    {
        qDebug() << "Connection::listen() failed with error " << WSAGetLastError();
        return false;
    }
    qDebug() << "Connection::listen() listen to socket " << s << " with backlog of " << backlog;
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

bool Connection::WSAEventSelect(SOCKET &sd, WSAEVENT &event, long networkEvents){
    if (::WSAEventSelect(sd, event, networkEvents) == SOCKET_ERROR)
    {
        qDebug() << "Connection::WSAEventSelect() failed with error " << WSAGetLastError();
        return false;
    }
    return true;
}


bool Connection::WSASetEvent(WSAEVENT &event){
    if (::WSASetEvent(event) == FALSE)
    {
        qDebug() << "Connection::WSASetEvent() failed with error" << WSAGetLastError();
        return false;
    }
    return true;
}


bool Connection::WSAWaitForMultipleEvents(WSAEVENT &event){
    DWORD index;
    while(TRUE)
    {
        index = ::WSAWaitForMultipleEvents(1, &event, FALSE, WSA_INFINITE, TRUE);
        if (index == WSA_WAIT_FAILED)
        {
            qDebug() << "Connection::WSAWaitForMultipleEvents() failed with error" << WSAGetLastError();
            return false;
        }
        if (index != WAIT_IO_COMPLETION)
        {
            break;
        }
    }
    WSAResetEvent(event);
    return true;
}


bool Connection::createSocketInfo(LPSOCKET_INFORMATION &SocketInfo, SOCKET s){
    if ((SocketInfo = (LPSOCKET_INFORMATION) GlobalAlloc(GPTR, sizeof(SOCKET_INFORMATION))) == NULL)
    {
        qDebug() << "Connection::createSocketInfo(): GlobalAlloc() failed with error " << GetLastError();
        return false;
    }
    SocketInfo->socket_tcp = s;
    ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));
    SocketInfo->BytesSEND = 0;
    SocketInfo->BytesRECV = 0;
    SocketInfo->DataBuf.len = DATA_BUFSIZE;
    SocketInfo->DataBuf.buf = SocketInfo->Buffer;

    memset(SocketInfo->Buffer, 0, sizeof(SocketInfo->Buffer));

    qDebug() << "Connection::createSocketInfo() Socket " << s << " connected";
    return true;
}


bool Connection::WSASend(LPSOCKET_INFORMATION &SI,
        LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine){
    DWORD SendBytes = 0;
    if (::WSASend(SI->socket_tcp, &(SI->DataBuf), 1, &SendBytes, 0,
            &(SI->Overlapped), lpCompletionRoutine) == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            qDebug() << "Connection::WSASend() failed with error" << WSAGetLastError();
            return false;
        }
        qDebug() << "Connection::WSASend() WSA_IO_PENDING";
    }
    qDebug() << "Connection::WSASend() Sent " << SendBytes << " bytes.";
    return true;
}


bool Connection::WSASendTo(LPSOCKET_INFORMATION &SI){
    DWORD SendBytes = 0;
    int	client_len = sizeof(SI->client_address);
    if (::WSASendTo(SI->socket_tcp, &(SI->DataBuf), 1, &SendBytes, 0,
            (struct sockaddr *)&(SI->client_address), client_len,
            0, NULL) == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            qDebug() << "Connection::WSASendTo() failed with error" << WSAGetLastError();
            return false;
        }
        qDebug() << "Connection::WSASendTo() WSA_IO_PENDING";
    }
    return true;
}


bool Connection::WSARecv(LPSOCKET_INFORMATION &SI,
        LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine){
    DWORD Flags = 0;
    DWORD RecvBytes = 0;
    if (::WSARecv(SI->socket_tcp, &(SI->DataBuf), 1, &RecvBytes, &Flags,
            &(SI->Overlapped), lpCompletionRoutine) == SOCKET_ERROR)
    {
        int error = WSAGetLastError();
        if (error != WSA_IO_PENDING)
        {
            qDebug() << "Connection::WSARecv() failed with error" << error;
            if (error == 10054)
            {
                qDebug() << "Connection::WSARecv() Connection terminated by Client.";
            }
            return false;
        }
        qDebug() << "Connection::WSARecv() WSA_IO_PENDING";
    }
    return true;
}



bool Connection::WSARecvFrom(LPSOCKET_INFORMATION SI,
        LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine){
    DWORD Flags = 0;
    DWORD RecvBytes = 0;
    int client_len = sizeof(SI->client_address);
    if (::WSARecvFrom(SI->socket_tcp, &(SI->DataBuf), 1, &RecvBytes, &Flags,
            (struct sockaddr *)&(SI->client_address), (LPINT)client_len,
            &(SI->Overlapped), lpCompletionRoutine) == SOCKET_ERROR)
    {
        int error = WSAGetLastError();
        if (error != WSA_IO_PENDING)
        {
            qDebug() << "Connection::WSARecvFrom() failed with error" << error;
            if (error == 10054)
            {
                qDebug() << "Connection::WSARecvFrom() Connection terminated by Client.";
            }
            return false;
        }
        qDebug() << "Connection::WSARecvFrom() WSA_IO_PENDING";
    }
    qDebug() << "Connection::WSARecvFrom() Received " << RecvBytes << "bytes.";
    return true;
}



bool Connection::checkError(LPSOCKET_INFORMATION &SI, DWORD error){
    if (error != 0)
    {
        qDebug() << "Connection::checkError() I/O operation failed for socket: " << SI->socket_tcp;
//        closesocket(SI->socket_tcp);
//        GlobalFree(SI);
        return false;
    }
    return true;
}


bool Connection::checkFinished(LPSOCKET_INFORMATION &SI, DWORD BytesTransferred){
    if (BytesTransferred == 0)
    {
        qDebug() << "Connection::checkFinished() Closing socketTCP " << SI->socket_tcp;
//        closesocket(SI->socket_tcp);
//        GlobalFree(SI);
        return false;
    }
    return true;
}


bool Connection::setsockopt(SOCKET &s, int level, int option, const char* value){
    int nRet = ::setsockopt(s, level, option, (char *)&value, sizeof(value));
    if (nRet == SOCKET_ERROR) {
        qDebug() << "Connection::setsockopt() Failed with error " << WSAGetLastError();
        return false;
    }
    return true;
}



