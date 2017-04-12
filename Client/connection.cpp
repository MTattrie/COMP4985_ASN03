/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: connection.cpp
--
-- PROGRAM: inotd
--
-- FUNCTIONS:
--    bool Connection::WSAStartup()
--    bool Connection::WSASocketTCP(SOCKET &s)
--    bool Connection::WSASocketUDP(SOCKET &s)
--    bool Connection::bind(SOCKET &s, int port)
--    bool Connection::connect(SOCKET &s, string host, int port)
--    bool Connection::setoptSO_REUSEADDR(SOCKET &s)
--    bool Connection::setoptIP_ADD_MEMBERSHIP(SOCKET &s)
--    bool Connection::WSACreateEvent(WSAEVENT &event)
--    bool Connection::WSAEventSelect(SOCKET &sd, WSAEVENT &event, long networkEvents)
--    bool Connection::WSASetEvent(WSAEVENT &event)
--    bool Connection::WSAWaitForMultipleEvents(WSAEVENT &event)
--    bool Connection::send(SOCKET &s, char buffer[])
--    int Connection::sendto(SOCKET &s, sockaddr_in &server, char buffer[], int len)
--    bool Connection::recv(SOCKET &s, char buffer[])
--    int Connection::recvfrom(SOCKET &s, sockaddr_in &server, char buffer[])
--
--
-- DATE: April 11, 2017
--
-- DESIGNER: Mark Tattrie, Deric Mccadden, Terry Kang, Jacob Frank
--
-- PROGRAMMER:
--      Deric Mccadden, Terry Kang.
--
-- NOTES:
-- This class contains connection functions.
----------------------------------------------------------------------------------------------------------------------*/
#include "connection.h"

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <QDebug>
#include <string>
#include <ws2tcpip.h>

using std::string;

Connection::Connection()
{

}



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
        qDebug() << "Connection::WSAStartup() WSAStartup() failed with error " << Ret;
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
    if ((s = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, 0)) == INVALID_SOCKET)
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
bool Connection::bind(SOCKET &s, sockaddr_in &server, int port){
    memset((char *)&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    if (::bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
    {
       qDebug() << "Connection::bind() failed with error " << WSAGetLastError();
       return false;
    }
    qDebug() << "Connection::bind() Bind socket " << s << " to port " << port;
    return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: connect
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: bool Connection::connect(SOCKET &s, string host, int port)
--
-- RETURNS: bool.
--
-- NOTES:
--  connects a scoket to a host and port.
----------------------------------------------------------------------------------------------------------------------*/
bool Connection::connect(SOCKET &s, string host, int port){
    struct hostent	*hp;
    struct sockaddr_in server;

    memset((char *)&server, 0, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    if ((hp = gethostbyname(host.c_str())) == NULL) {
        qDebug() << "Connection::connect() Unknown server address";
        return false;
    }
    memcpy((char *)&server.sin_addr, hp->h_addr, hp->h_length);

    if (::connect (s, (struct sockaddr *)&server, sizeof(server)) == -1) {
        qDebug() << "Connection::connect() Can't connect to server " << GetLastError();
        return false;
    }
    qDebug() << "Connection::connect() New socket:" << s << " host:" << host.c_str() << " port:" << port;
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

/* ================================================================================== *
 *                                   SEND/RECV
 * ================================================================================== */


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: send
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: bool Connection::send(SOCKET &s, char buffer[])
--
-- RETURNS: bool.
--
-- NOTES:
--  Wrapper for send
----------------------------------------------------------------------------------------------------------------------*/
bool Connection::send(SOCKET &s, char buffer[]){
    DWORD bytes_to_send = BUFFERSIZE;
    char *bp = buffer;
    int n;
    while ((n = ::send(s, bp, bytes_to_send, 0)) < BUFFERSIZE)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            qDebug() << "Connection::WSASend() failed with error" << WSAGetLastError();
            return false;
        }
        bp += n;
        bytes_to_send -= n;
        if (n == 0)
            break;
    }
    qDebug() << "Client::send() buffer contents: " << buffer;
    qDebug() << "Client::send() buffer BUFFERSIZE: " << BUFFERSIZE;

    return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: sendto
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden, Terry Kang
--
-- INTERFACE: bool Connection::sendto(SOCKET &s, char buffer[])
--
-- RETURNS: bool.
--
-- NOTES:
--  Wrapper for sendto
----------------------------------------------------------------------------------------------------------------------*/
int Connection::sendto(SOCKET &s, sockaddr_in &server, char buffer[], int len){
    DWORD bytes_to_send = len;
    char *bp = buffer;
    int n;
    n = ::sendto(s, bp, bytes_to_send, 0, (struct sockaddr *)&server, sizeof(server));
    return n;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: recv
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: bool Connection::recv(SOCKET &s, char buffer[])
--
-- RETURNS: bool.
--
-- NOTES:
--  Wrapper for recv
----------------------------------------------------------------------------------------------------------------------*/
bool Connection::recv(SOCKET &s, char buffer[]){
    DWORD bytes_to_read = BUFFERSIZE;
    char * bp = buffer;
    int n;
    while (n = ::recv(s, bp, bytes_to_read, 0))
    {
        int error = WSAGetLastError();
        if (error != WSA_IO_PENDING && error != 0)
        {
            if(error == 10035)
                continue;
            qDebug() << "Connection::WSARecv() failed with error" << error;
            return false;
        }
        bp += n;
        bytes_to_read -= n;
        if (n == 0)
            break;
        if(bytes_to_read == 0)
            break;
    }
    qDebug() << "Client::recv() buffer contents: " << buffer;
    return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: recvfrom
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden, Terry Kang
--
-- INTERFACE: int Connection::recvfrom(SOCKET &s, sockaddr_in &server, char buffer[])
--
-- RETURNS: int.
--
-- NOTES:
--  Wrapper for recvfrom
----------------------------------------------------------------------------------------------------------------------*/
int Connection::recvfrom(SOCKET &s, sockaddr_in &server, char buffer[]){
    DWORD bytes_to_read = BUFFERSIZE;
    char * bp = buffer;
    int n;
    int serverlen = sizeof(server);
    n = ::recvfrom(s, bp, bytes_to_read, 0, (struct sockaddr *)&server, &serverlen);

    return n;
}


/* ================================================================================== *
 *                                   EVENTS
 * ================================================================================== */

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
bool Connection::WSAEventSelect(SOCKET &s, WSAEVENT &event, long networkEvents){
    if (::WSAEventSelect(s, event, networkEvents) == SOCKET_ERROR)
    {
        qDebug() << "Connection::WSAEventSelect() failed with error " << WSAGetLastError();
        return false;
    }
    return true;
}

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
    if ((::WSAWaitForMultipleEvents(1, &event, FALSE, WSA_INFINITE, FALSE)) == WSA_WAIT_FAILED)
    {
        qDebug() << "Connection::WSAWaitForMultipleEvents() failed with error " << WSAGetLastError();
        return false;
    }
    return true;
}
