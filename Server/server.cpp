#include "server.h"
#include "connection.h"

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <thread>
#include <QDebug>
#include <string>

#include <ws2tcpip.h>


Connection conn;
SOCKET socket_tcp_accept;
SOCKET socket_tcp_listen;
SOCKET socket_udp;
int port = 7000;


Server::Server(QObject *parent) : QObject(parent)
{

}

void Server::startTCP() {
    if(!conn.WSAStartup())
        return;
    connect();
    closesocket(socket_tcp_accept);
    closesocket(socket_tcp_listen);
    closesocket(socket_udp);
    WSACleanup();
}


void Server::connect(){
    if(!conn.WSASocketTCP(socket_tcp_listen))
        return;
    if(!conn.bind(socket_tcp_listen, port))
        return;
    if(!conn.listen(socket_tcp_listen))
        return;

    if(!conn.WSASocketUDP(socket_udp))
        return;
    if(!conn.bind(socket_udp, port))
        return;

    runTCP();
}


void Server::runTCP() {
    WSAEVENT AcceptEvent;
    if(!conn.WSACreateEvent(AcceptEvent))
        return;
//    if(!conn.WSAEventSelect(sd_tcp, readEvent, FD_READ))
//        return;

    std::thread(&Server::workerThreadTCP, this, AcceptEvent).detach();

    while(TRUE) {
        socket_tcp_accept = accept(socket_tcp_listen, NULL, NULL);
        if(!conn.WSASetEvent(AcceptEvent))
            return;
    }
}


void Server::workerThreadTCP(WSAEVENT AcceptEvent) {
    LPSOCKET_INFORMATION SocketInfo;

    while(true) {

        if(!conn.WSAWaitForMultipleEvents(AcceptEvent))
            break;
        if(!conn.createSocketInfo(SocketInfo, socket_tcp_accept))
            break;
        qDebug() << endl << "Server::workerThreadTCP New Transfer:";
        if(!conn.WSARecvFrom(SocketInfo, WorkerRoutine_RecvCommand))
            break;
        qDebug() << "Server::workerThreadTCP() After recv:";
    }
    qDebug() << "workerThreadTCP() Closing WorkerThread:";
}


void CALLBACK Server::WorkerRoutine_RecvCommand(DWORD Error, DWORD BytesTransferred,
        LPWSAOVERLAPPED Overlapped, DWORD InFlags) {
    (void)InFlags;//this is not used, but cannot be removed without breaking the callback.
    LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION) Overlapped;

    qDebug() << "Server::WorkerRoutine_RecvCommand receved " << BytesTransferred << " bytes. "
             << "From Client Address " << inet_ntoa(SI->client_address.sin_addr)
             << " DataBuf Contents: " << SI->DataBuf.buf;

    if(!conn.checkError(SI, Error))
        return;

    ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));

    if(SI->BytesRECV < DATA_BUFSIZE){
        SI->BytesRECV += BytesTransferred;
        SI->DataBuf.buf = SI->Buffer + SI->BytesRECV;
        SI->DataBuf.len = DATA_BUFSIZE - SI->BytesRECV;
        qDebug() << "inside RECV---------";
        conn.WSARecv(SI, WorkerRoutine_RecvCommand);
    }

    if(SI->BytesRECV >= DATA_BUFSIZE){
        //String packet(SI->Buffer);
        int command = 1;
        //parse command
        SI->DataBuf.buf = SI->Buffer;
        SI->DataBuf.len = DATA_BUFSIZE;

        switch(command){
        case 1://send list using tcp
            conn.WSASend(SI, WorkerRoutine_SendList);
            return;
        case 3:
            //recv song
            return;
        }
    }
}


void CALLBACK Server::WorkerRoutine_SendList(DWORD Error, DWORD BytesTransferred,
        LPWSAOVERLAPPED Overlapped, DWORD InFlags) {
    (void)InFlags;//this is not used, but cannot be removed without breaking the callback.
    LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION) Overlapped;

    qDebug() << "Server::WorkerRoutine_SendList() Sent " << BytesTransferred << " bytes. "
             << "Client Address " << inet_ntoa(SI->client_address.sin_addr)
             << " DataBuf Contents: " << SI->DataBuf.buf;

    if(!conn.checkError(SI, Error))
        return;
    if(!conn.checkFinished(SI, BytesTransferred)){
        qDebug() << "End Transfer:" << endl;
        return;
    }

    ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
    SI->BytesSEND += BytesTransferred;
    SI->DataBuf.buf = SI->Buffer + SI->BytesSEND;
    SI->DataBuf.len = DATA_BUFSIZE - SI->BytesSEND;
    conn.WSASend(SI, WorkerRoutine_SendList);

}






//#define BUFSIZE     1024
//#define MAXADDRSTR  16

//#define TIMECAST_TTL    2
//#define TIMECAST_INTRVL 30

//void Server::UDPMulticaset(){
//    char achMCAddr[MAXADDRSTR] = "234.5.6.7";
//    u_short nPort              = 8910;
//    u_long  lTTL               = TIMECAST_TTL;
//    u_short nInterval          = TIMECAST_INTRVL;
//    SYSTEMTIME stSysTime;

//    int nRet;
//    BOOL  fFlag;
//    SOCKADDR_IN stDstAddr;
//    struct ip_mreq stMreq;        /* Multicast interface structure */
//    SOCKET hSocket;


//    /* Join the multicast group
//    *
//    * NOTE: According to RFC 1112, a sender does not need to join the
//    *  group, however Microsoft requires a socket to join a group in
//    *  order to use setsockopt() IP_MULTICAST_TTL (or fails with error
//    *  WSAEINVAL).
//    */
//    stMreq.imr_multiaddr.s_addr = inet_addr(achMCAddr);
//    stMreq.imr_interface.s_addr = INADDR_ANY;


//    nRet = setsockopt(socket_udp,
//     IPPROTO_IP,
//     IP_ADD_MEMBERSHIP,
//     (char *)&stMreq,
//     sizeof(stMreq));
//    if (nRet == SOCKET_ERROR) {
//    printf (
//      "setsockopt() IP_ADD_MEMBERSHIP address %s failed, Err: %d\n",
//      achMCAddr, WSAGetLastError());
//    }

//    /* Set IP TTL to traverse up to multiple routers */
//    nRet = setsockopt(socket_udp,
//     IPPROTO_IP,
//     IP_MULTICAST_TTL,
//     (char *)&lTTL,
//     sizeof(lTTL));
//    if (nRet == SOCKET_ERROR) {
//    printf ("setsockopt() IP_MULTICAST_TTL failed, Err: %d\n",
//      WSAGetLastError());
//    }

//    /* Disable loopback */
//    fFlag = FALSE;
//    nRet = setsockopt(socket_udp,
//     IPPROTO_IP,
//     IP_MULTICAST_LOOP,
//     (char *)&fFlag,
//     sizeof(fFlag));
//    if (nRet == SOCKET_ERROR) {
//    printf ("setsockopt() IP_MULTICAST_LOOP failed, Err: %d\n",
//      WSAGetLastError());
//    }

//    /* Assign our destination address */
//    stDstAddr.sin_family =      AF_INET;
//    stDstAddr.sin_addr.s_addr = inet_addr(achMCAddr);
//    stDstAddr.sin_port =        htons(nPort);
//    for (;;) {
//        /* Get System (UTC) Time */
//        GetSystemTime (&stSysTime);

//        /* Send the time to our multicast group! */
//        nRet = sendto(socket_udp,
//          (char *)&stSysTime,
//          sizeof(stSysTime),
//          0,
//          (struct sockaddr*)&stDstAddr,
//          sizeof(stDstAddr));
//        if (nRet < 0) {
//          printf ("sendto() failed, Error: %d\n", WSAGetLastError());
//          exit(1);
//        } else {
//            printf("Sent UTC Time %02d:%02d:%02d:%03d ",
//                stSysTime.wHour,
//                stSysTime.wMinute,
//                stSysTime.wSecond,
//                stSysTime.wMilliseconds);
//            printf("Date: %02d-%02d-%02d to: %s:%d\n",
//                stSysTime.wMonth,
//                stSysTime.wDay,
//                stSysTime.wYear,
//                inet_ntoa(stDstAddr.sin_addr),
//                ntohs(stDstAddr.sin_port));
//        }

//        Sleep(nInterval*1000);

//    }

//    /* Close the socket */
//    closesocket(hSocket);

//}

