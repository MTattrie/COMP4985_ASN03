#include "connection.h"

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <QDebug>
#include <string>
#include <ws2tcpip.h>

using std::string;


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


/* ================================================================================== *
 *                                   SOCKETS
 * ================================================================================== */


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
    sockaddr_in InternetAddr;
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


bool Connection::setoptSO_REUSEADDR(SOCKET &s){
    int opt = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt)) < 0)
    {
       qDebug() << "WSASocket() failed with error " << WSAGetLastError();
        return false;
    }
    return true;
}


bool Connection::setoptIP_MULTICAST_TTL(SOCKET &s){
    u_long time = 1;
    if(setsockopt(s, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&time, sizeof(time)) == SOCKET_ERROR)
    {
        qDebug() << "setsockopt() failed with error on time to live" << WSAGetLastError();
        return false;
    }
    return true;
}


bool Connection::setoptIP_MULTICAST_LOOP(SOCKET &s){
    BOOL LoopBackFlag = false;
    if(::setsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&LoopBackFlag, sizeof(LoopBackFlag)) == SOCKET_ERROR)
    {
        qDebug() << "Setsocketopt() failed with error on loop back" << WSAGetLastError();
        return false;
    }
    return true;
}


bool Connection::setoptIP_ADD_MEMBERSHIP(SOCKET &s){
    struct ip_mreq   MulticastAddress;
    MulticastAddress.imr_multiaddr.s_addr = inet_addr(MULTICASTSERVER);
    MulticastAddress.imr_interface.s_addr = INADDR_ANY;
    if(::setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&MulticastAddress, sizeof(MulticastAddress)) == SOCKET_ERROR)
    {
        qDebug() << "setsockopt() failed with error on multicast address " <<  WSAGetLastError();
        return false;
    }
    return true;
}


bool Connection::createSocketInfo(LPSOCKET_INFORMATION &SocketInfo, SOCKET s){
    if ((SocketInfo = (LPSOCKET_INFORMATION) GlobalAlloc(GPTR, sizeof(SOCKET_INFORMATION))) == NULL)
    {
        qDebug() << "Connection::createSocketInfo(): GlobalAlloc() failed with error " << GetLastError();
        return false;
    }
    SocketInfo->Socket = s;
    ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));
    SocketInfo->BytesSEND = 0;
    SocketInfo->BytesRECV = 0;
    SocketInfo->BytesToSend = 0;
    SocketInfo->DataBuf.len = BUFFERSIZE;
    SocketInfo->DataBuf.buf = SocketInfo->Buffer;

    memset(SocketInfo->Buffer, 0, sizeof(SocketInfo->Buffer));

    qDebug() << "Connection::createSocketInfo() Socket " << s << " connected";
    return true;
}


/* ================================================================================== *
 *                                   EVENTS
 * ================================================================================== */


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


bool Connection::WSASend(LPSOCKET_INFORMATION &SI,
        LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine){
    DWORD SendBytes = 0;
    if (::WSASend(SI->Socket, &(SI->DataBuf), 1, &SendBytes, 0,
            &(SI->Overlapped), lpCompletionRoutine) == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            qDebug() << "Connection::WSASend() failed with error" << WSAGetLastError();
            return false;
        }
        qDebug() << "Connection::WSASend() WSA_IO_PENDING";
    }
    return true;
}


bool Connection::WSASendTo(LPSOCKET_INFORMATION &SI,
                           LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine){
    DWORD SendBytes = 0;

    sockaddr_in InternetAddr;
    InternetAddr.sin_family = AF_INET;
    InternetAddr.sin_addr.s_addr = inet_addr(MULTICASTSERVER);
    InternetAddr.sin_port = htons(port);

    int	client_len = sizeof(InternetAddr);

    if (::WSASendTo(SI->Socket, &(SI->DataBuf), 1, &SendBytes, 0,
            (struct sockaddr *)&(InternetAddr), client_len,
            &(SI->Overlapped), lpCompletionRoutine) == SOCKET_ERROR)
    {
        int error = WSAGetLastError();
        if (error != WSA_IO_PENDING)
        {
            qDebug() << "Connection::WSASendTo() failed with error" << error;
            return false;
        }
        qDebug() << "Connection::WSASendTo() WSA_IO_PENDING";
    }
   // qDebug() << "Connection::WSASendTo() SendBytes : " << SendBytes;

    return true;
}


bool Connection::WSARecv(LPSOCKET_INFORMATION &SI,
        LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine){
    DWORD Flags = 0;
    DWORD RecvBytes = 0;
    if (::WSARecv(SI->Socket, &(SI->DataBuf), 1, &RecvBytes, &Flags,
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
    qDebug() << "Connection::WSARecv() RecvBytes : " << RecvBytes;

    return true;
}



bool Connection::WSARecvFrom(LPSOCKET_INFORMATION SI,
        LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine){
    DWORD Flags = 0;
    DWORD RecvBytes = 0;
    int client_len = sizeof(SI->client_address);
    if (::WSARecvFrom(SI->Socket, &(SI->DataBuf), 1, &RecvBytes, &Flags,
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
        qDebug() << "Connection::checkError() I/O operation failed for socket: " << SI->Socket;
//        closesocket(SI->socket_tcp);
//        GlobalFree(SI);
        return false;
    }
    return true;
}


bool Connection::checkFinished(LPSOCKET_INFORMATION &SI, DWORD BytesTransferred){
    if (BytesTransferred == 0)
    {
        qDebug() << "Connection::checkFinished() Closing socketTCP " << SI->Socket;
//        closesocket(SI->socket_tcp);
//        GlobalFree(SI);
        return false;
    }
    return true;
}







