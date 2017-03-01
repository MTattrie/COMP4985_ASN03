#include "connection.h"

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <QDebug>
#include <string>

using std::string;


Connection::Connection()
{

}


bool Connection::_WSAStartup(WSADATA &wsa){
    DWORD Ret;
    if ((Ret = WSAStartup(0x0202, &wsa)) != 0)
    {
        qDebug() << "WSAStartup() failed with error " << Ret << endl;
        return false;
    }
    return true;
}
//FD_ACCEPT|FD_CLOSE
bool Connection::_WSAEventSelect(SOCKET &s, long lNetworkEvents){
    if (WSAEventSelect(s, EventArray[EventTotal - 1], lNetworkEvents) == SOCKET_ERROR)
    {
        qDebug() << "WSAEventSelect() failed with error " << WSAGetLastError() << endl;
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

bool Connection::_Bind(SOCKET &s, SOCKADDR_IN &InternetAddr){
    if (bind(s, (PSOCKADDR) &InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
    {
        qDebug() << "bind() failed with error " << WSAGetLastError() << endl;
        return false;
    }
    return true;
}

bool Connection::_Listen(SOCKET &s){
    int backlog = 5;
    if (listen(s, backlog))
    {
        qDebug() << "listen() failed with error " << WSAGetLastError() << endl;
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




bool Connection::ReadData(string &buffer, WSANETWORKEVENTS &NetworkEvents, DWORD &Event){

    if (NetworkEvents.lNetworkEvents & FD_READ ||
        NetworkEvents.lNetworkEvents & FD_WRITE)
    {
        if (NetworkEvents.lNetworkEvents & FD_READ &&
            NetworkEvents.iErrorCode[FD_READ_BIT] != 0)
        {
            qDebug() << "FD_READ failed with error" << NetworkEvents.iErrorCode[FD_READ_BIT] << endl;
            return false;
        }

        if (NetworkEvents.lNetworkEvents & FD_WRITE &&
            NetworkEvents.iErrorCode[FD_WRITE_BIT] != 0)
        {
            qDebug() << "FD_WRITE failed with error" << NetworkEvents.iErrorCode[FD_WRITE_BIT] << endl;
            return false;
        }

        LPSOCKET_INFORMATION SocketInfo = SocketArray[Event - WSA_WAIT_EVENT_0];

        // Read data only if the receive buffer is empty.
        if (SocketInfo->BytesRECV == 0)
        {
           SocketInfo->DataBuf.buf = SocketInfo->Buffer;
           SocketInfo->DataBuf.len = DATA_BUFSIZE;

           _WSARecv(SocketInfo, Event);

        }
        // Write buffer data if it is available.
        if(SocketInfo->DataBuf.buf == NULL){
            return false;
        }
        buffer = string(SocketInfo->DataBuf.buf);
        qDebug() << "Server: RecvPacket:" << SocketInfo->DataBuf.buf << endl;

        SocketInfo->BytesRECV = 0;
    }
    return true;
}


bool Connection::ReceiveNewConnection(SOCKET &s, DWORD &Event, WSANETWORKEVENTS &NetworkEvents){
    if (NetworkEvents.lNetworkEvents & FD_ACCEPT)
    {
       if (NetworkEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
       {
           qDebug() << "FD_ACCEPT failed with error " << NetworkEvents.iErrorCode[FD_ACCEPT_BIT] << endl;
           return false;
       }
       if(!_Accept(s, Event)){
           return false;
       }
       if (EventTotal > WSA_MAXIMUM_WAIT_EVENTS)
       {
           qDebug() << "Too many connections - closing socket." << endl;
           closesocket(s);
           return false;
       }
       CreateSocketInformation(s);

       if(!_WSAEventSelect(s, FD_READ|FD_WRITE|FD_CLOSE)){
           qDebug() << "Too many connections - closing socket." << endl;
           return false;
       }
       qDebug() << "Socket " << s << " connected" << endl;
    }
    return true;
}

bool Connection::CloseSocket(DWORD &Event, WSANETWORKEVENTS &NetworkEvents){
    if (NetworkEvents.lNetworkEvents & FD_CLOSE)
    {
        if (NetworkEvents.iErrorCode[FD_CLOSE_BIT] != 0)
        {
            qDebug() << "FD_CLOSE failed with error " << NetworkEvents.iErrorCode[FD_CLOSE_BIT] << endl;
            return false;
        }
        qDebug() << "Closing socket information " << SocketArray[Event - WSA_WAIT_EVENT_0]->Socket << endl;
        FreeSocketInformation(Event - WSA_WAIT_EVENT_0);
    }
    return true;
}



bool Connection::CreateSocketInformation(SOCKET s)
{
    LPSOCKET_INFORMATION SI;

    if ((EventArray[EventTotal] = WSACreateEvent()) == WSA_INVALID_EVENT)
    {
        qDebug() << "WSACreateEvent() failed with error " << WSAGetLastError() << endl;
        return false;
    }

    if ((SI = (LPSOCKET_INFORMATION) GlobalAlloc(GPTR,
      sizeof(SOCKET_INFORMATION))) == NULL)
    {
        qDebug() << "GlobalAlloc() failed with error " << GetLastError() << endl;
        return false;
    }
    // Prepare SocketInfo structure for use.
    SI->Socket = s;
    SI->BytesSEND = 0;
    SI->BytesRECV = 0;
    SocketArray[EventTotal] = SI;
    EventTotal++;

    return true;
}


void Connection::FreeSocketInformation(DWORD Event)
{
    LPSOCKET_INFORMATION SI = SocketArray[Event];
    DWORD i;
    closesocket(SI->Socket);
    GlobalFree(SI);
    WSACloseEvent(EventArray[Event]);
    // Squash the socket and event arrays
    for (i = Event; i < EventTotal; i++)
    {
        EventArray[i] = EventArray[i + 1];
        SocketArray[i] = SocketArray[i + 1];
    }
    EventTotal--;
}
