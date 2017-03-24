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
SOCKET AcceptSocket;


Server::Server(QObject *parent) : QObject(parent)
{

}

void Server::startTCP() {
    runTCP();
    WSACleanup();
}

void Server::runTCP() {
    SOCKET ListenSocket;
    WSAEVENT AcceptEvent;

    if(!conn.WSAStartup())
        return;
    if(!conn.WSASocketTCP(ListenSocket))
        return;
    if(!conn.bind(ListenSocket))
        return;
    if(!conn.listen(ListenSocket))
        return;
    if(!conn.WSACreateEvent(AcceptEvent))
        return;

    std::thread(&Server::workerThreadTCP, this, AcceptEvent).detach();

    while(TRUE) {
        AcceptSocket = accept(ListenSocket, NULL, NULL);
        if(!conn.WSASetEvent(AcceptEvent))
            return;
    }
}


void Server::workerThreadTCP(WSAEVENT event) {
    LPSOCKET_INFORMATION SocketInfo;
    WSAEVENT EventArray[1];
    EventArray[0] = event;

    while(true) {
        if(!conn.WSAWaitForMultipleEvents(EventArray))
            break;
        if(!conn.createSocketInfo(SocketInfo, AcceptSocket))
            break;
        if(!conn.WSARecv(SocketInfo, WorkerRoutineTCP))
            break;

        qDebug() << "Socket " << AcceptSocket << " connected" << endl;
    }
}


void CALLBACK Server::WorkerRoutineTCP(DWORD Error, DWORD BytesTransferred,
        LPWSAOVERLAPPED Overlapped, DWORD InFlags) {
    (void)InFlags;//this is not used, but cannot be removed without breaking the callback.
    LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION) Overlapped;

    if(!conn.checkError(SI, Error))
        return;
    if(!conn.checkFinished(SI, BytesTransferred))
        return;

    // check if its a new transfer
    if (SI->BytesRECV == 0) {
        SI->BytesRECV = BytesTransferred;
        SI->BytesSEND = 0;
    }
    else {
        SI->BytesSEND += BytesTransferred;
    }

    if (SI->BytesRECV > SI->BytesSEND) {
        // continue posting WSASend() calls until all received bytes are sent.
        ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
        SI->DataBuf.buf = SI->Buffer + SI->BytesSEND;
        SI->DataBuf.len = SI->BytesRECV - SI->BytesSEND;
        conn.WSASend(SI, WorkerRoutineTCP);
    }
    else {
        SI->BytesRECV = 0;
        ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
        SI->DataBuf.len = DATA_BUFSIZE;
        SI->DataBuf.buf = SI->Buffer;
        conn.WSARecv(SI, WorkerRoutineTCP);
        qDebug() << "buffer:" << SI->Buffer << endl;
    }
}

//void Server::sendSong(){

//        SI->BytesRECV += BytesTransferred;
//        if(SI->BytesRECV >= HEADER_SIZE){
//            int mode = decodeHeader();
//            switch(mode){
//            case MODE_SEND:
//                //send a song
//                break;
//            case MODE_RECEIVE:
//                //receive a new song
//                break;
//            case MODE_COMMAND:
//                //song command
//                break;
//            }
//        }

//        ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
//        SI->DataBuf.len = DATA_BUFSIZE;
//        SI->DataBuf.buf = SI->Buffer + SI->BytesRECV;
//        conn.WSARecv(SI, WorkerRoutineTCP);



//    if (SI->BytesRECV > SI->BytesSEND) {
//            ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
//            SI->DataBuf.buf = SI->Buffer + SI->BytesSEND;
//            SI->DataBuf.len = SI->BytesRECV - SI->BytesSEND;
//            conn.WSASend(SI, WorkerRoutineTCP);
//        }
//}


/*
void Server::startUDP(){

    char achMCAddr[MAXADDRSTR] = TIMECAST_ADDR;
    u_short nPort              = TIMECAST_PORT;
    u_long  lTTL               = TIMECAST_TTL;
    u_short nInterval          = TIMECAST_INTRVL;
    SYSTEMTIME stSysTime;

    int nRet;
    SOCKADDR_IN stDstAddr;
    struct ip_mreq stMreq;        // Multicast interface structure
    SOCKET hSocket;

    if(!conn.WSAStartup())
        return;
    if(!conn.WSASocketUDP(hSocket))
        return;
    if(!conn.bind(hSock))
        return;

    // Join the multicast group
    stMreq.imr_multiaddr.s_addr = inet_addr(achMCAddr);
    stMreq.imr_interface.s_addr = INADDR_ANY;

    conn.setsockopt(hSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, stMreq);
    conn.setsockopt(hSocket, IPPROTO_IP, IP_MULTICAST_TTL, lTTL);
    conn.setsockopt(hSocket, IPPROTO_IP, IP_MULTICAST_LOOP, FALSE);

    // Assign our destination address
    stDstAddr.sin_family =      AF_INET;
    stDstAddr.sin_addr.s_addr = inet_addr(achMCAddr);
    stDstAddr.sin_port =        htons(nPort);

    for (;;) {
      /// Get System (UTC) Time
      GetSystemTime (&stSysTime);

      // Send the time to our multicast group!
      nRet = sendto(hSocket,
        (char *)&stSysTime,
        sizeof(stSysTime),
        0,
        (struct sockaddr*)&stDstAddr,
        sizeof(stDstAddr));
      if (nRet < 0) {
        printf ("sendto() failed, Error: %d\n", WSAGetLastError());
        exit(1);
      } else {
          printf("Sent UTC Time %02d:%02d:%02d:%03d ",
              stSysTime.wHour,
              stSysTime.wMinute,
              stSysTime.wSecond,
              stSysTime.wMilliseconds);
          printf("Date: %02d-%02d-%02d to: %s:%d\n",
              stSysTime.wMonth,
              stSysTime.wDay,
              stSysTime.wYear,
              inet_ntoa(stDstAddr.sin_addr),
              ntohs(stDstAddr.sin_port));
      }

      Sleep(nInterval*1000);
    }
    closesocket(hSocket);
    WSACleanup();
}
*/
