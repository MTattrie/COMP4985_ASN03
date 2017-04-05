#include "connection.h"

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <QDebug>
#include <string>



Connection::Connection()
{

}


bool Connection::WSAStartup(){
    WSADATA wsa;
    DWORD Ret;
    if ((Ret = ::WSAStartup(0x0202, &wsa)) != 0)
    {
        qDebug() << "WSAStartup() failed with error " << Ret << endl;
        return false;
    }
    return true;
}


bool Connection::WSASocketTCP(SOCKET &s){
    if ((s = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0,
            WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        qDebug() << "WSASocket() failed with error " << WSAGetLastError() << endl;
        return false;
    }
    return true;
}

bool Connection::WSASocketUDP(SOCKET &s){
    if ((s = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, 0)) == INVALID_SOCKET)
    {
        qDebug() << "WSASocket() failed with error " << WSAGetLastError() << endl;
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
       qDebug() << "bind() failed with error " << WSAGetLastError() << endl;
       return false;
    }
    return true;
}


bool Connection::listen(SOCKET &s){
    int backlog = 5;
    if (::listen(s, backlog))
    {
        qDebug() << "listen() failed with error " << WSAGetLastError() << endl;
        return false;
    }
    return true;
}


bool Connection::WSACreateEvent(WSAEVENT &event){
    if ((event = ::WSACreateEvent()) == WSA_INVALID_EVENT)
    {
        qDebug() << "WSACreateEvent() failed with error " << WSAGetLastError() << endl;
        return false;
    }
    return true;
}


bool Connection::WSASetEvent(WSAEVENT &event){
    if (::WSASetEvent(event) == FALSE)
    {
        qDebug() << "WSASetEvent() failed with error" << WSAGetLastError() << endl;
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
            qDebug() << "WSAWaitForMultipleEvents() failed with error" << WSAGetLastError() << endl;
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
        qDebug() << "GlobalAlloc() failed with error " << GetLastError() << endl;
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
            qDebug() << "WSASend() failed with error" << WSAGetLastError() << endl;
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
            qDebug() << "WSARecv() failed with error" << WSAGetLastError() << endl;
            return false;
        }
    }
    return true;
}



bool Connection::checkError(LPSOCKET_INFORMATION &SI, DWORD error){
    if (error != 0)
    {
        qDebug() << "I/O operation failed for socket: " << SI->Socket << endl;
        closesocket(SI->Socket);
        GlobalFree(SI);
        return false;
    }
    return true;
}


bool Connection::checkFinished(LPSOCKET_INFORMATION &SI, DWORD BytesTransferred){
    if (BytesTransferred == 0)
    {
        qDebug() << "Closing socket " << SI->Socket << endl;
        closesocket(SI->Socket);
        GlobalFree(SI);
        return false;
    }
    return true;
}


bool Connection::setsockopt(int &socket, int level, int option, const char* value){
    int nRet = ::setsockopt(socket, level, option, (char *)&value, sizeof(value));
    if (nRet == SOCKET_ERROR) {
        qDebug() << "setsockopt() failed with error " << WSAGetLastError() << endl;
        return false;
    }
    return true;
}
