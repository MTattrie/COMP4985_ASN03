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

std::vector<LPSOCKET_INFORMATION> client_addresses;


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
    WSAEVENT acceptEvent;
    if(!conn.WSACreateEvent(acceptEvent))
        return;

    std::thread(&Server::acceptThread, this, acceptEvent).detach();
    std::thread(&Server::UDPMulticast, this).detach();

    while(TRUE) {
        socket_tcp_accept = accept(socket_tcp_listen, NULL, NULL);
        if(!conn.WSASetEvent(acceptEvent))
            return;
    }
}


void Server::acceptThread(WSAEVENT acceptEvent) {

    while(true) {
        if(!conn.WSAWaitForMultipleEvents(acceptEvent))
            break;
        std::thread(&Server::readThread, this).detach();
        qDebug() << "Server::acceptThread() New Connection:";
    }
    qDebug() << "acceptThread() Closing acceptThread:";
}


void Server::readThread(){
    LPSOCKET_INFORMATION SI;
    WSAEVENT readEvent;

    if(!conn.createSocketInfo(SI, socket_tcp_accept))
        return;

    if(!conn.WSACreateEvent(readEvent))
        return;
    if(!conn.WSAEventSelect(SI->socket_tcp, readEvent, FD_READ))
        return;

    client_addresses.push_back(SI);


    while(true){
        if(!conn.WSAWaitForMultipleEvents(readEvent))
            break;

        ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
        SI->BytesSEND = 0;
        SI->BytesRECV = 0;
        SI->DataBuf.len = DATA_BUFSIZE;
        SI->DataBuf.buf = SI->Buffer;
        memset(SI->Buffer, 0, sizeof(SI->Buffer));



        qDebug() << endl << "Server::readThread() New Read Event:";
        if(!conn.WSARecvFrom(SI, WorkerRoutine_RecvCommand))
            break;
    }
    qDebug() << "readThread() Closing readThread:";
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
    SI->BytesRECV += BytesTransferred;
    SI->DataBuf.buf = SI->Buffer + SI->BytesRECV;
    SI->DataBuf.len = DATA_BUFSIZE - SI->BytesRECV;

    if(SI->BytesRECV < DATA_BUFSIZE){
        conn.WSARecv(SI, WorkerRoutine_RecvCommand);
    }
    if(SI->BytesRECV >= DATA_BUFSIZE){
        //parse paccket
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



void Server::UDPMulticast(){
    char buff[DATA_BUFSIZE] = "stuff stuff stuff";

    while(true){

        qDebug() << endl << "UDPMulticaset() Start sending to group.";
        for(auto& SI: client_addresses){
            qDebug() << "UDPMulticaset() Send to address: " << inet_ntoa(SI->client_address.sin_addr);

            SI->DataBuf.buf = buff;
            SI->DataBuf.len = DATA_BUFSIZE;

            if(!conn.WSASendTo(SI)){
                //close
            }
        }
        qDebug() << "UDPMulticaset() finished sending to group.";
        Sleep(2000);
    }
}








