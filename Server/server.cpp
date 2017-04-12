/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: server.cpp
--
-- PROGRAM: inotd
--
-- FUNCTIONS:
--    void Server::Server(QObject *parent) : QObject(parent)
--    bool Server::setPort(QString _port)
--    void Server::start()
--    bool Server::setPort(QString _port)
--    void Server::startTCP()
--    void Server::connectTCP()
--    void Server::runTCP()
--    void Server::acceptThread(WSAEVENT acceptEvent)
--    void Server::readThread()
--    void CALLBACK Server::WorkerRoutine_RecvCommand(DWORD Error, DWORD BytesTransferred,
--      LPWSAOVERLAPPED Overlapped, DWORD InFlags)
--    void Server::saveFile(QByteArray data, QString filename)
--    void CALLBACK Server::WorkerRoutine_SendList(DWORD Error, DWORD BytesTransferred,
--      LPWSAOVERLAPPED Overlapped, DWORD InFlags)
--    void Server::startUDP()
--    void Server::connectUDP()
--    void Server::runUDP()
--    void CALLBACK Server::WorkerRoutine_UDPSend(DWORD Error, DWORD BytesTransferred,
--      LPWSAOVERLAPPED Overlapped, DWORD InFlags)
--    void CALLBACK Server::WorkerRoutine_TCPSend(DWORD Error, DWORD BytesTransferred,
--      LPWSAOVERLAPPED Overlapped, DWORD InFlags)
--    void Server::addStreamData(QByteArray data)
--    void Server::resetStreamData()
--    void Server::sendToClient(int client_num, int command, QByteArray data)
--    void Server::TCPBroadCast(int command, QByteArray data)
--    bool Server::sendFile(LPSOCKET_INFORMATION &SI, string filename)
--    bool Server::loadFile(QQueue<QByteArray>& packets, const string filename)
--    char *Server::getClientIP(int client_num)
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
-- This class contains the core Server logic.
----------------------------------------------------------------------------------------------------------------------*/
#include "server.h"
#include "connection.h"
#include "global.h"

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <thread>
#include <QDebug>
#include <string>
#include <fstream>
#include <queue>
#include <QFileDialog>

#include <ws2tcpip.h>

using std::string;

Connection conn;
SOCKET socket_tcp_accept;
SOCKET socket_tcp_listen;
struct sockaddr_in client_address;
SOCKET socket_udp;
std::vector<LPSOCKET_INFORMATION> client_addresses;
std::queue<string> packet_queue;


Server::Server(QObject *parent) : QObject(parent)
{
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: start
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: void Server::start()
--
-- RETURNS: void.
--
-- NOTES:
--  starts the server threads.
----------------------------------------------------------------------------------------------------------------------*/
void Server::start(){
    std::thread(&Server::startTCP, this).detach();
    std::thread(&Server::startUDP, this).detach();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: setPort
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden, Terry Kang
--
-- INTERFACE: bool Server::setPort(QString _port)
--
-- RETURNS: bool.
--
-- NOTES:
--  Sets the server port.
----------------------------------------------------------------------------------------------------------------------*/
bool Server::setPort(QString _port) {
    bool convertOK;
    port = _port.toInt(&convertOK);
    conn.port = port;

    if(!convertOK)
        return false;
    return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: startTCP
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: void Server::startTCP()
--
-- RETURNS: void.
--
-- NOTES:
--  Starts the tcp server.
----------------------------------------------------------------------------------------------------------------------*/
void Server::startTCP() {
    if(!conn.WSAStartup())
        return;
    connectTCP();
    closesocket(socket_tcp_accept);
    closesocket(socket_tcp_listen);
    WSACleanup();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: connectTCP
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: void Server::connectTCP()
--
-- RETURNS: void.
--
-- NOTES:
--  connects the tcp server.
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: runTCP
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: void Server::runTCP()
--
-- RETURNS: void.
--
-- NOTES:
--  runs the tcp server.
----------------------------------------------------------------------------------------------------------------------*/
void Server::runTCP() {
    WSAEVENT acceptEvent;
    memset(&client_address, 0, sizeof(client_address));
    int client_len = sizeof(client_address);
    if(!conn.WSACreateEvent(acceptEvent))
        return;
    std::thread(&Server::acceptThread, this, acceptEvent).detach();

    while(TRUE) {
        socket_tcp_accept = accept(socket_tcp_listen, (struct sockaddr *)&(client_address), &client_len);
        if(!conn.WSASetEvent(acceptEvent))
            return;
    }
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: acceptThread
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: void Server::acceptThread(WSAEVENT acceptEvent)
--
-- RETURNS: void.
--
-- NOTES:
--  acceps connections and makes read threads.
----------------------------------------------------------------------------------------------------------------------*/
void Server::acceptThread(WSAEVENT acceptEvent) {
    while(true) {
        if(!conn.WSAWaitForMultipleEvents(acceptEvent))
            break;
        std::thread(&Server::readThread, this).detach();
        qDebug() << "Server::acceptThread() New Connection:";
    }
    qDebug() << "acceptThread() Closing acceptThread:";
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: readThread
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden, Terry Kang
--
-- INTERFACE: void Server::readThread()
--
-- RETURNS: void.
--
-- NOTES:
--  handles read events. starts a thread for each read event.
----------------------------------------------------------------------------------------------------------------------*/
void Server::readThread(){
    LPSOCKET_INFORMATION SI;
    WSAEVENT readEvent;
    WSANETWORKEVENTS wsaEvents;
    memset(&wsaEvents, 0, sizeof(WSANETWORKEVENTS));

    int client_num;

    if(!conn.createSocketInfo(SI, socket_tcp_accept))
        return;
    if(!conn.WSACreateEvent(readEvent))
        return;
    if(!conn.WSAEventSelect(SI->Socket, readEvent, FD_READ | FD_CLOSE))
        return;

    SI->client_address = client_address;
    client_num = (int)client_addresses.size();
    client_addresses.push_back(SI);

    QByteArray upload;
    SI->upload = &upload;


    emit newClientConnected(client_num);

    SI->thisObj = this;
    while(true){
        if(!conn.WSAWaitForMultipleEvents(readEvent))
            break;

        if(WSAEnumNetworkEvents(SI->Socket,readEvent,&wsaEvents) == SOCKET_ERROR)
            break;

        if(wsaEvents.lNetworkEvents & FD_CLOSE){
            emit clientDisconnected(getClientIP(client_num));
            client_addresses.erase(
                        std::remove_if(
                            client_addresses.begin(),
                            client_addresses.end(),
                            [SI](auto s){ return s->Socket == SI->Socket;}
            ), client_addresses.end());
            qDebug()<<"Disconnect";
            break;
        }

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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WorkerRoutine_RecvCommand
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden,Terry Kang
--
-- INTERFACE: void CALLBACK Server::WorkerRoutine_RecvCommand(DWORD Error, DWORD BytesTransferred,
        LPWSAOVERLAPPED Overlapped, DWORD InFlags)
--
-- RETURNS: void.
--
-- NOTES:
--  Receive commands from the clients and call the appropriate functions per command.
----------------------------------------------------------------------------------------------------------------------*/
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
        case UPLOAD: { // upload song
            SI->upload->append(QByteArray(&SI->Buffer[1], BytesTransferred-1));
            return;
        }
        case COMPLETE: {// upload song
            ((Server *)(SI->thisObj))->saveFile(*(SI->upload), &(SI->Buffer[1]));
            SI->upload->clear();
            emit  ((Server *)(SI->thisObj))->updateAvaliableSongs();
            return;
        }
        case DOWNLOAD: { //download song
            string filename(&(SI->Buffer[1]));
            ((Server *)(SI->thisObj))->sendFile(SI, filename);
            return;
        }
        case ADDLIST: //add list
            emit ((Server *)(SI->thisObj))->receivedAddPlaylist(QString(&SI->Buffer[1]));
            return;
        case PLAYPAUSE: //play or pause
        case FASTFORWORD: // fastforward
        case REWIND: // rewind
        case SKIPTRACK: // skip track
            emit ((Server *)(SI->thisObj))->receivedCommand(command);
            return;
        case STREAM: //stream
            return;
        }
    }
    else if(SI->BytesRECV < BUFFERSIZE){
        qDebug()<<"BUFFERSIZE : " << BUFFERSIZE;
        conn.WSARecv(SI, WorkerRoutine_RecvCommand);
    }

}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: saveFile
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: void Server::saveFile(QByteArray data, QString filename)
--
-- RETURNS: void.
--
-- NOTES:
--  Save a received file to disk.
----------------------------------------------------------------------------------------------------------------------*/
void Server::saveFile(QByteArray data, QString filename){
    QString path("../assets/musics/");
    path.append(filename);
    qDebug() << "saved file: " << path;
    QFile file(path);
    file.open(QIODevice::WriteOnly);
    file.write(data);
    file.close();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: saveFile
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: void CALLBACK Server::WorkerRoutine_SendList(DWORD Error, DWORD BytesTransferred,
        LPWSAOVERLAPPED Overlapped, DWORD InFlags)
--
-- RETURNS: void.
--
-- NOTES:
--  Worker routine for sending a list.
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: startUDP
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: void Server::startUDP()
--
-- RETURNS: void.
--
-- NOTES:
--  Start a UDP server.
----------------------------------------------------------------------------------------------------------------------*/
void Server::startUDP() {
    if(!conn.WSAStartup())
        return;
    connectUDP();
    closesocket(socket_udp);
    WSACleanup();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: connectUDP
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: void Server::connectUDP()
--
-- RETURNS: void.
--
-- NOTES:
--  connect the UDP server.
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: runUDP
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: void Server::runUDP()
--
-- RETURNS: void.
--
-- NOTES:
--  runs the UDP server.
----------------------------------------------------------------------------------------------------------------------*/
void Server::runUDP(){

    LPSOCKET_INFORMATION SI;
    if(!conn.createSocketInfo(SI, socket_udp))
        return;

    while(true){
        if(streamQueue.isEmpty()){
            continue;
        }

        QByteArray data = streamQueue.front();
        SI->DataBuf.buf = data.data();
        SI->DataBuf.len = data.size();
        SI->BytesToSend = data.size();
        SI->BytesSEND = 0;
        //qDebug()<<"BytesToSend : " << SI->BytesToSend;

        if(!conn.WSASendTo(SI, WorkerRoutine_UDPSend)) {
            continue;
        }

        //qDebug() << "UDPMulticaset() finished sending to group.";
        streamQueue.pop_front();
    }
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WorkerRoutine_UDPSend
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: void CALLBACK Server::WorkerRoutine_UDPSend(DWORD Error, DWORD BytesTransferred,
        LPWSAOVERLAPPED Overlapped, DWORD InFlags)
--
-- RETURNS: void.
--
-- NOTES:
--  Worker routine for sending UDP.
----------------------------------------------------------------------------------------------------------------------*/
void CALLBACK Server::WorkerRoutine_UDPSend(DWORD Error, DWORD BytesTransferred,
   LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
    Q_UNUSED(InFlags);
    LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION) Overlapped;

    if(!conn.checkError(SI, Error))
        return;

   SI->BytesSEND += BytesTransferred;

   if (SI->BytesSEND < SI->BytesToSend)
   {
       conn.WSASendTo(SI, WorkerRoutine_UDPSend);
   }
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WorkerRoutine_TCPSend
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: void CALLBACK Server::WorkerRoutine_TCPSend(DWORD Error, DWORD BytesTransferred,
        LPWSAOVERLAPPED Overlapped, DWORD InFlags)
--
-- RETURNS: void.
--
-- NOTES:
--  Worker routine for sending TCP.
----------------------------------------------------------------------------------------------------------------------*/
void CALLBACK Server::WorkerRoutine_TCPSend(DWORD Error, DWORD BytesTransferred,
   LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
    Q_UNUSED(InFlags);
    LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION) Overlapped;

    if(!conn.checkError(SI, Error))
        return;

   SI->BytesSEND += BytesTransferred;
   SI->DataBuf.buf = SI->Buffer + SI->BytesSEND;
   SI->DataBuf.len = BUFFERSIZE - SI->BytesSEND;

   if (SI->BytesSEND < SI->BytesToSend)
   {
       conn.WSASend(SI, WorkerRoutine_TCPSend);
   }
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: addStreamData
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: void Server::addStreamData(QByteArray data)
--
-- RETURNS: void.
--
-- NOTES:
--  add stream data.
----------------------------------------------------------------------------------------------------------------------*/
void Server::addStreamData(QByteArray data){
    streamQueue.push_back(data);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: resetStreamData
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: void Server::resetStreamData(QByteArray data)
--
-- RETURNS: void.
--
-- NOTES:
--  reset stream data.
----------------------------------------------------------------------------------------------------------------------*/
void Server::resetStreamData(){
    streamQueue.clear();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: sendToClient
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: void Server::sendToClient(int client_num, int command, QByteArray data)
--
-- RETURNS: void.
--
-- NOTES:
--  send command to client.
----------------------------------------------------------------------------------------------------------------------*/
void Server::sendToClient(int client_num, int command, QByteArray data){
    LPSOCKET_INFORMATION SI = client_addresses.at(client_num);
    data.prepend(command);

    ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
    memset(SI->Buffer, 0, sizeof(SI->Buffer));
    memcpy(SI->Buffer, data.data(), data.size());
    SI->DataBuf.buf = SI->Buffer;
    SI->DataBuf.len = BUFFERSIZE;
    SI->BytesToSend = BUFFERSIZE;
    SI->BytesSEND = 0;

    conn.WSASend(SI, WorkerRoutine_TCPSend);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: TCPBroadCast
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: void Server::TCPBroadCast(int command, QByteArray data)
--
-- RETURNS: void.
--
-- NOTES:
--  TCP broadcast to all clients
----------------------------------------------------------------------------------------------------------------------*/
void Server::TCPBroadCast(int command, QByteArray data){
    data.prepend(command);

   for(LPSOCKET_INFORMATION SI : client_addresses){
       ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
       memset(SI->Buffer, 0, sizeof(SI->Buffer));
       memcpy(SI->Buffer, data.data(), data.size());
       SI->DataBuf.buf = SI->Buffer;
       SI->DataBuf.len = BUFFERSIZE;
       SI->BytesToSend = BUFFERSIZE;
       SI->BytesSEND = 0;
       conn.WSASend(SI, WorkerRoutine_UDPSend);
   }
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: sendFile
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: bool Server::sendFile(LPSOCKET_INFORMATION &SI, string filename)
--
-- RETURNS: void.
--
-- NOTES:
--  Send a file to a client.
----------------------------------------------------------------------------------------------------------------------*/
bool Server::sendFile(LPSOCKET_INFORMATION &SI, string filename){
    QQueue<QByteArray> packets;
    if(!loadFile(packets, filename))
        return false;

    ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
    SI->Overlapped.hEvent = WSACreateEvent();
    if(SI->Overlapped.hEvent == NULL){
        return false;
    }
    DWORD Flags;
    while(!packets.empty()){
        QByteArray packet = packets.front();
        packets.pop_front();

        memset(SI->Buffer, 0, BUFFERSIZE);
        memcpy(SI->Buffer, packet.data(), packet.size());
        SI->DataBuf.buf = SI->Buffer;
        SI->DataBuf.len = BUFFERSIZE;
        SI->BytesToSend = BUFFERSIZE;
        SI->BytesSEND = 0;

        if (WSASend(SI->Socket, &(SI->DataBuf), 1, &SI->BytesSEND, 0,
                &(SI->Overlapped), NULL) == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSA_IO_PENDING)
            {
                qDebug() << "Connection::WSASend() failed with error" << WSAGetLastError();
                return false;
            }
            if(WSAWaitForMultipleEvents(1, &SI->Overlapped.hEvent, FALSE, INFINITE, FALSE) == WAIT_TIMEOUT){
                qDebug() << "Connection::WSASend() failed with timeout";
            }
            qDebug() << "Connection::WSASend() WSA_IO_PENDING";
        }
        if(!WSAGetOverlappedResult(SI->Socket, &(SI->Overlapped), &SI->BytesSEND, FALSE, &Flags)){
            qDebug()<<"WSASend failed with error: " << WSAGetLastError();
        }
    }

    qDebug() << "Server::sendFile() Completed transfer: " << filename.c_str();
    return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: loadFile
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Deric Mccadden
--
-- INTERFACE: bool Server::loadFile(QQueue<QByteArray>& packets, const string filename)
--
-- RETURNS: bool.
--
-- NOTES:
--  load a file for sending.
----------------------------------------------------------------------------------------------------------------------*/
bool Server::loadFile(QQueue<QByteArray>& packets, const string filename){
    QString path("../assets/musics/");
    path.append(filename.c_str());
    const QString p(path);
    QFile file(p);
    if(!file.open(QIODevice::ReadOnly))
        return false;

    QByteArray data = file.readAll();

    qint64 pos = 0;
    qint64 len =  BUFFERSIZE - 1;

    while(pos < data.size()) {
        QByteArray packet;
        qint64 chunk = qMin((data.size() - pos), len);
        packet.append(DOWNLOAD);
        packet.append(data.mid(pos, chunk));
        packets.push_back(packet);
        pos += chunk;
    }
    QByteArray packet;
    packet.append(COMPLETE);
    packets.push_back(packet);
    file.close();
    return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: getClientIP
--
-- DATE: April 10, 2017
--
-- DESIGNER: Mark Tattrie, Jacob frank, Terry Kang, Deric Mccadden
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: char *Server::getClientIP(int client_num)
--
-- RETURNS: char.
--
-- NOTES:
--  get the client ip.
----------------------------------------------------------------------------------------------------------------------*/
char *Server::getClientIP(int client_num){
    char *ip = inet_ntoa(client_addresses.at(client_num)->client_address.sin_addr);
    return ip;
}


