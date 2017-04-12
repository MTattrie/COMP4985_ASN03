#include "connection.h"

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <QDebug>
#include <string>
#include <ws2tcpip.h>

using std::string;


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WSAStartup
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: WSAStartup()
--
-- RETURNS: bool.
--
-- NOTES:
--  Wraper for WSAStartup
----------------------------------------------------------------------------------------------------------------------*/
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


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WSASocketTCP
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: WSASocketTCP(SOCKET &s)
--
-- RETURNS: bool.
--
-- NOTES:
--  Creates a TCP socket.
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WSASocketUDP
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: WSASocketUDP(SOCKET &s)
--
-- RETURNS: bool.
--
-- NOTES:
--  Creates a UDP socket.
----------------------------------------------------------------------------------------------------------------------*/
bool Connection::WSASocketUDP(SOCKET &s){
    if ((s = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, 0)) == INVALID_SOCKET)
    {
        qDebug() << "Connection::WSASocketUDP() failed with error " << WSAGetLastError();
        return false;
    }
    qDebug() << "Connection::WSASocketUDP() Setup UDP socket " << s;
    return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: bind
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: bind(SOCKET &s, int port){
--
-- RETURNS: bool.
--
-- NOTES:
--  Binds a socket to a specified port.
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: listen
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: listen(SOCKET &s)
--
-- RETURNS: bool.
--
-- NOTES:
--  Listen to a specfied socket..
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: setoptSO_REUSEADDR
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: setoptSO_REUSEADDR(SOCKET &s)
--
-- RETURNS: bool.
--
-- NOTES:
--  Sets SO_REUSEADR on a specified socket.
----------------------------------------------------------------------------------------------------------------------*/
bool Connection::setoptSO_REUSEADDR(SOCKET &s){
    int opt = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt)) < 0)
    {
       qDebug() << "WSASocket() failed with error " << WSAGetLastError();
        return false;
    }
    return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: setoptIP_MULTICAST_TTL
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: setoptIP_MULTICAST_TTL(SOCKET &s)
--
-- RETURNS: bool.
--
-- NOTES:
--  Sets IP_MULTICAST_TTL on a specified socket.
----------------------------------------------------------------------------------------------------------------------*/
bool Connection::setoptIP_MULTICAST_TTL(SOCKET &s){
    u_long time = 1;
    if(setsockopt(s, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&time, sizeof(time)) == SOCKET_ERROR)
    {
        qDebug() << "setsockopt() failed with error on time to live" << WSAGetLastError();
        return false;
    }
    return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: setoptIP_MULTICAST_LOOP
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: setoptIP_MULTICAST_LOOP(SOCKET &s)
--
-- RETURNS: bool.
--
-- NOTES:
--  Sets IP_MULTICAST_LOOP on a specified socket.
----------------------------------------------------------------------------------------------------------------------*/
bool Connection::setoptIP_MULTICAST_LOOP(SOCKET &s){
    BOOL LoopBackFlag = false;
    if(::setsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&LoopBackFlag, sizeof(LoopBackFlag)) == SOCKET_ERROR)
    {
        qDebug() << "Setsocketopt() failed with error on loop back" << WSAGetLastError();
        return false;
    }
    return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: setoptIP_ADD_MEMBERSHIP
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: setoptIP_ADD_MEMBERSHIP(SOCKET &s)
--
-- RETURNS: bool.
--
-- NOTES:
--  Sets IP_ADD_MEMBERSHIP on a specified socket.
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: createSocketInfo
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: bool Connection::createSocketInfo(LPSOCKET_INFORMATION &SocketInfo, SOCKET s)
--
-- RETURNS: bool.
--
-- NOTES:
--  Creates the socket information.
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WSACreateEvent
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: bool Connection::WSACreateEvent(WSAEVENT &event)
--
-- RETURNS: bool.
--
-- NOTES:
--  Wrapper for WSACreateEvent
----------------------------------------------------------------------------------------------------------------------*/
bool Connection::WSACreateEvent(WSAEVENT &event){
    if ((event = ::WSACreateEvent()) == WSA_INVALID_EVENT)
    {
        qDebug() << "Connection::WSACreateEvent() failed with error " << WSAGetLastError();
        return false;
    }
    return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WSAEventSelect
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: bool Connection::WSAEventSelect(SOCKET &sd, WSAEVENT &event, long networkEvents){
--
-- RETURNS: bool.
--
-- NOTES:
--  Wrapper for WSAEventSelect
----------------------------------------------------------------------------------------------------------------------*/
bool Connection::WSAEventSelect(SOCKET &sd, WSAEVENT &event, long networkEvents){
    if (::WSAEventSelect(sd, event, networkEvents) == SOCKET_ERROR)
    {
        qDebug() << "Connection::WSAEventSelect() failed with error " << WSAGetLastError();
        return false;
    }
    return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WSASetEvent
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: bool Connection::WSASetEvent(WSAEVENT &event)
--
-- RETURNS: bool.
--
-- NOTES:
--  Wrapper for WSASetEvent
----------------------------------------------------------------------------------------------------------------------*/
bool Connection::WSASetEvent(WSAEVENT &event){
    if (::WSASetEvent(event) == FALSE)
    {
        qDebug() << "Connection::WSASetEvent() failed with error" << WSAGetLastError();
        return false;
    }
    return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WSAWaitForMultipleEvents
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: bool Connection::WSAWaitForMultipleEvents(WSAEVENT &event)
--
-- RETURNS: bool.
--
-- NOTES:
--  Wrapper for WSAWaitForMultipleEvents
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WSASend
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: bool Connection::WSASend(LPSOCKET_INFORMATION &SI,
        LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
--
-- RETURNS: bool.
--
-- NOTES:
--  Wrapper for WSASend
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WSASendTo
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: bool Connection::WSASendTo(LPSOCKET_INFORMATION &SI,
        LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
--
-- RETURNS: bool.
--
-- NOTES:
--  Wrapper for WSASendTo
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WSARecv
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: bool Connection::WSARecv(LPSOCKET_INFORMATION &SI,
        LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
--
-- RETURNS: bool.
--
-- NOTES:
--  Wrapper for WSARecv
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WSARecvFrom
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: bool Connection::WSARecvFrom(LPSOCKET_INFORMATION &SI,
        LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
--
-- RETURNS: bool.
--
-- NOTES:
--  Wrapper for WSARecvFrom
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: checkError
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: bool Connection::checkError(LPSOCKET_INFORMATION &SI,
        LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
--
-- RETURNS: bool.
--
-- NOTES:
--  checks for an error.
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: checkFinished
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: bool Connection::checkFinished(LPSOCKET_INFORMATION &SI, DWORD BytesTransferred)
--
-- RETURNS: bool.
--
-- NOTES:
--  checks if the transfer is finished.
----------------------------------------------------------------------------------------------------------------------*/
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







