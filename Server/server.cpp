#include "server.h"
#include "connection.h"
#include "packet.h"

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <thread>
#include <QDebug>
#include <string>
#include <fstream>
#include <queue>

#include <ws2tcpip.h>

using std::string;




Connection conn;
SOCKET socket_tcp_accept;
SOCKET socket_tcp_listen;
SOCKET socket_udp;
int port = 7000;

std::vector<LPSOCKET_INFORMATION> client_addresses;

std::queue<string> packet_queue;


Server::Server(QObject *parent) : QObject(parent)
{
}


void Server::start(){
    std::thread(&Server::startTCP, this).detach();
    std::thread(&Server::startUDP, this).detach();
}

bool Server::setPort(QString _port) {
    bool convertOK;
    port = _port.toInt(&convertOK);

    if(!convertOK)
        return false;
    return true;
}




void Server::startTCP() {
    if(!conn.WSAStartup())
        return;
    connectTCP();
    closesocket(socket_tcp_accept);
    closesocket(socket_tcp_listen);
    WSACleanup();
}

void Server::connectTCP(){
    if(!conn.WSASocketTCP(socket_tcp_listen))
        return;
    if(!conn.setoptSO_REUSEADDR(socket_tcp_listen))
        return;
    if(!conn.bind(socket_tcp_listen, port))
        return;
    if(!conn.listen(socket_tcp_listen))
        return;
    runTCP();
}


void Server::runTCP() {
    WSAEVENT acceptEvent;
    if(!conn.WSACreateEvent(acceptEvent))
        return;
    std::thread(&Server::acceptThread, this, acceptEvent).detach();

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
    if(!conn.WSAEventSelect(SI->Socket, readEvent, FD_READ))
        return;

    client_addresses.push_back(SI);

    SI->thisObj = this;
    this->receivedCommand(1);
    while(true){
        if(!conn.WSAWaitForMultipleEvents(readEvent))
            break;

        ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
        SI->BytesSEND = 0;
        SI->BytesRECV = 0;
        SI->DataBuf.len = BUFFERSIZE;
        SI->DataBuf.buf = SI->Buffer;
        memset(SI->Buffer, 0, sizeof(SI->Buffer));

        //qDebug() << endl << "Server::readThread() New Read Event:";
        if(!conn.WSARecv(SI, WorkerRoutine_RecvCommand))
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
    SI->DataBuf.len = BUFFERSIZE - SI->BytesRECV;

    if(BytesTransferred == 0 && SI->BytesRECV == 0){
        return;
    }else if(BytesTransferred == 0 || SI->BytesRECV >= BUFFERSIZE){

        int command = SI->Buffer[0];

        //parse command
        SI->DataBuf.buf = SI->Buffer;
        SI->DataBuf.len = BUFFERSIZE;

        qDebug() << "Server::WorkerRoutine_RecvCommand receved command :  " <<command;

        switch(command){
        case 0: // upload song
            return;
        case 1: //download song
            //conn.WSASend(SI, WorkerRoutine_SendList);
            return;
        case 2: //add list
            return;
        case 3: //play or pause
        case 4: // fastforward
        case 5: // rewind
        case 6: // skip track
            emit ((Server *)(SI->thisObj))->receivedCommand(command);
            return;
        case 7: //stream
            return;
        }
    }
    else if(SI->BytesRECV < BUFFERSIZE){
        qDebug()<<"BUFFERSIZE : " << BUFFERSIZE;
        conn.WSARecv(SI, WorkerRoutine_RecvCommand);
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
    SI->DataBuf.len = BUFFERSIZE - SI->BytesSEND;
    conn.WSASend(SI, WorkerRoutine_SendList);

}

bool Server::multicast(char *message, const int len){
    for(auto& SI: client_addresses){
        qDebug() << "UDPMulticaset() Send to address: " << inet_ntoa(SI->client_address.sin_addr);
        qDebug() << message;
        SI->DataBuf.buf = message;
        SI->DataBuf.len = len;

        //if(!conn.WSASendTo(SI->socket_udp, SI->DataBuf.buf)){
            //close
        //}
    }
    return true;
}



void Server::startUDP() {
    if(!conn.WSAStartup())
        return;
    connectUDP();
    closesocket(socket_udp);
    WSACleanup();
}

void Server::connectUDP(){
    if(!conn.WSASocketUDP(socket_udp))
        return;
    if(!conn.setoptSO_REUSEADDR(socket_udp))
        return;
    if(!conn.bind(socket_udp, port))
        return;
    if(!conn.setoptIP_MULTICAST_TTL(socket_udp))
        return;
    if(!conn.setoptIP_MULTICAST_LOOP(socket_udp))
        return;
    if(!conn.setoptIP_ADD_MEMBERSHIP(socket_udp))
        return;
    runUDP();
}

void Server::runUDP(){

    LPSOCKET_INFORMATION SI;
    if(!conn.createSocketInfo(SI, socket_udp))
        return;

    int count = 0;

    while(true){
        if(streamQueue.isEmpty()){
            count = 0;
            continue;
        }

        qDebug()<<++count;
        QByteArray data = streamQueue.front();
        SI->DataBuf.buf = data.data();
        SI->DataBuf.len = data.size();
        SI->BytesToSend = data.size();
        SI->BytesSEND = 0;
        qDebug()<<"BytesToSend : " << SI->BytesToSend;

        if(!conn.WSASendTo(SI, WorkerRoutine_UDPSend)) {
            continue;
        }

        qDebug() << "UDPMulticaset() finished sending to group.";
        streamQueue.pop_front();
    }
    /*
    while(true){
        //Sleep(2000);


        if(packet_queue.empty()){
            count = 0;
            continue;
        }

        qDebug()<<++count;
        std::string data = packet_queue.front();
        SI->DataBuf.buf = const_cast<char*>(data.c_str());
        SI->BytesToSend = data.size();

        if(!conn.WSASendTo(SI, WorkerRoutine_UDPSend)) {
            continue;
        }

        packet_queue.pop();
        Sleep(10);

        qDebug() << "UDPMulticaset() finished sending to group.";

    }*/
    /*
    sockaddr_in InternetAddr;
    InternetAddr.sin_family = AF_INET;
    InternetAddr.sin_addr.s_addr = inet_addr("234.57.7.8");
    InternetAddr.sin_port = htons(7000);
    while(true){
        //Sleep(2000);


        if(packet_queue.empty()){
            count = 0;
            continue;
        }

        qDebug()<<++count;

        DWORD bytes_to_send = BUFFERSIZE;

        sendto(socket_udp, packet_queue.front().c_str(), bytes_to_send, 0, (struct sockaddr *)&InternetAddr, sizeof(InternetAddr));

        packet_queue.pop();
        Sleep(1);

        qDebug() << "UDPMulticaset() finished sending to group.";

    }*/
}

void CALLBACK Server::WorkerRoutine_UDPSend(DWORD Error, DWORD BytesTransferred,
   LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
    LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION) Overlapped;

    if(!conn.checkError(SI, Error))
        return;

   SI->BytesSEND += BytesTransferred;

   if (SI->BytesSEND < SI->BytesToSend)
   {
       conn.WSASendTo(SI, WorkerRoutine_UDPSend);
   }

}

void Server::addStreamData(QByteArray data){
    qDebug() << "addStreamData";

    streamQueue.push_back(data);
}
