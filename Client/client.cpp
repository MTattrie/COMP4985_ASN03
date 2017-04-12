#include "client.h"
#include <stdio.h>
#include <winsock2.h>
#include <errno.h>
#include <thread>
#include <QDebug>
#include "connection.h"
#include "global.h"
#include <ws2tcpip.h>
#include <iostream>
#include <fstream>
#include <QFile>
#include <QQueue>
#include <QFileInfo>


Client::Client(QObject *parent) : QObject(parent)
{
    isDonwloading = false;
    peerUDPRunning = false;
    isUploading = false;
}

void Client::start(QString hostname, QString port){
    storeServerDetails(hostname, port);
    std::thread(&Client::startTCP, this).detach();
    std::thread(&Client::startUDP, this).detach();
}



/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: storeServerDetails
--
-- DATE: April 10, 2017
--
-- DESIGNER:
--
-- PROGRAMMER:
--
-- INTERFACE: void Client::storeServerDetails(QString hostname, QString port)
--                  QString hostname: The hostname to connect to
--                  QString port: The specified port number for communication
--
-- RETURNS: void.
--
-- NOTES:
--  Stores the user input hostname and port number into the client's local variables
--  so they can be used to connect to the server.
----------------------------------------------------------------------------------------------------------------------*/
void Client::storeServerDetails(QString hostname, QString port) {
    if (!port[0].isDigit())
        return;
    serverHostName = hostname.toLocal8Bit().constData();
    portNumber = port.toInt();
    conn.port = portNumber;
}


void Client::startTCP(){
    if(!conn.WSAStartup())
        return;
    connectTCP();
    WSACleanup();
    closesocket (socket_tcp);
    qDebug() << "Client::startTCP() Socket " << socket_tcp << " closed";
}


void Client::startUDP(){
    if(!conn.WSAStartup())
        return;
    connectUDP();
    WSACleanup();
    closesocket (socket_udp);
    qDebug() << "Client::startUDP() Socket " << socket_udp << " closed";
}

void Client::connectTCP(){

    if(!conn.WSASocketTCP(socket_tcp))
        return;
    if(!conn.setoptSO_REUSEADDR(socket_tcp))
        return;
    if(!conn.connect(socket_tcp, serverHostName, portNumber))
        return;
    runTCP();
}

void Client::connectUDP(){

    if(!conn.WSASocketUDP(socket_udp))
        return;
    if(!conn.setoptSO_REUSEADDR(socket_udp))
        return;
    if(!conn.bind(socket_udp, server, portNumber))
        return;
    if(!conn.setoptIP_ADD_MEMBERSHIP(socket_udp))
        return;
    runUDP();
}


void Client::runTCP(){
    char rbuf[BUFFERSIZE];
    WSAEVENT readEvent;

    if(!conn.WSACreateEvent(readEvent))
        return;
    if(!conn.WSAEventSelect(socket_tcp, readEvent, FD_READ))
        return;

    qDebug()<<"runTCP ";

    while(true) {
        if(!conn.WSAWaitForMultipleEvents(readEvent))
            return;
        WSAResetEvent(readEvent);

        if(!conn.recv(socket_tcp, rbuf))
            continue;

        int command = rbuf[0];

        qDebug()<<"command : " << command;


        char *data = new char[BUFFERSIZE-1];
        memcpy(data, &rbuf[1], BUFFERSIZE-1);
        emit receivedCommand(command, data, BUFFERSIZE-1);

    }
}



void Client::runUDP(){
    char rbuf[BUFFERSIZE];
    WSAEVENT readEvent;

    if(!conn.WSACreateEvent(readEvent))
        return;
    if(!conn.WSAEventSelect(socket_udp, readEvent, FD_READ))
        return;
    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("234.57.7.8");
    server.sin_port = htons(7000);

    while (true) {
        if(!conn.WSAWaitForMultipleEvents(readEvent))
            return;
        WSAResetEvent(readEvent);

        int n = conn.recvfrom(socket_udp, server, rbuf);
        if(n<=0)
            continue;
        int command = rbuf[0] - '0';

        char *data = new char[n-1];
        memcpy(data, &rbuf[1], n-1);
        emit receivedCommand(command, data, n-1);
    }
}


void Client::requestSong(QString song){
    if(isDonwloading)
        return;
    char buffer[BUFFERSIZE];

    filenames = song;
    downloads.clear();

    QString packet;
    packet.append(DOWNLOAD);
    packet.append(song);

    memset((char *)buffer, 0, BUFFERSIZE);
    memcpy(buffer, packet.toStdString().c_str(), packet.size());

    if(!conn.send(socket_tcp, buffer))
        return;
    isDonwloading=true;
}

void Client::reqeustCommand(int command, QString data){
    char buffer[BUFFERSIZE];
    qDebug()<<"reqeustCommand";
    QString packet(command);
    packet.append(data);
    memset((char *)buffer, 0, BUFFERSIZE);
    memcpy(buffer, packet.toStdString().c_str(), packet.size());

    if(!conn.send(socket_tcp, buffer))
        return;
}

void Client::sendSong(QString song){
    if(isUploading)
        return;

    isUploading = true;
    sendFile(song);
    isUploading = false;
}

bool Client::sendFile(QString filename){
    QQueue<QByteArray> packets;
    if(!loadFile(packets, filename))
        return false;
    OVERLAPPED Overlapped;
    CHAR Buffer[BUFFERSIZE];
    WSABUF DataBuf;
    DWORD BytesSEND;

    ZeroMemory(&Overlapped, sizeof(WSAOVERLAPPED));
    Overlapped.hEvent = WSACreateEvent();
    if(Overlapped.hEvent == NULL){
        return false;
    }
    DWORD Flags;
    while(!packets.empty()){
        QByteArray packet = packets.front();
        packets.pop_front();

        memset(Buffer, 0, BUFFERSIZE);
        memcpy(Buffer, packet.data(), packet.size());
        DataBuf.buf = Buffer;
        DataBuf.len = BUFFERSIZE;
        BytesSEND = 0;

        if (WSASend(socket_tcp, &DataBuf, 1, &BytesSEND, 0,
                &Overlapped, NULL) == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSA_IO_PENDING)
            {
                qDebug() << "Client::sendFile() failed with error" << WSAGetLastError();
                return false;
            }
            if(WSAWaitForMultipleEvents(1, &Overlapped.hEvent, FALSE, INFINITE, FALSE) == WAIT_TIMEOUT){
                qDebug() << "Client::sendFile() failed with timeout";
            }
            qDebug() << "Client::sendFile() WSA_IO_PENDING";
        }
        if(!WSAGetOverlappedResult(socket_tcp, &Overlapped, &BytesSEND, FALSE, &Flags)){
            qDebug()<<"sendFile failed with error: " << WSAGetLastError();
        }
    }

    qDebug() << "Client::sendFile() Completed transfer: " << filename;
    return true;
}

bool Client::loadFile(QQueue<QByteArray>& packets, const QString filepathname){
    const QString p(filepathname);
    QFile file(p);

    QFileInfo fileInfo(file.fileName());
    QString filename(fileInfo.fileName());


    if(!file.open(QIODevice::ReadOnly))
        return false;

    QByteArray data = file.readAll();

    qint64 pos = 0;
    qint64 len =  BUFFERSIZE - 1;

    while(pos < data.size()) {
        QByteArray packet;
        qint64 chunk = qMin((data.size() - pos), len);
        packet.append(UPLOAD);
        packet.append(data.mid(pos, chunk));
        packets.push_back(packet);
        pos += chunk;
    }
    QByteArray packet;
    packet.append(COMPLETE);
    packet.append(filename);
    packets.push_back(packet);
    file.close();
    return true;
}

bool Client::startPeerUDP(QString hostname, QString port){
    if (!port[0].isDigit())
        return false;

    int peerPort = port.toInt();
    memset((char *)&peer_addr, 0, sizeof(peer_addr));
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_addr.s_addr = inet_addr(hostname.toStdString().c_str());
    peer_addr.sin_port = htons(peerPort);

    sockaddr_in myaddr;
    memset((char *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = (0);
    myaddr.sin_port = htons(peerPort);
    if(!conn.WSAStartup())
        return false;
    if(!conn.WSASocketUDP(socket_peerUDP))
        return false;
    if(!conn.setoptSO_REUSEADDR(socket_peerUDP))
        return false;
    if(!conn.bind(socket_peerUDP, myaddr, peerPort))
        return false;

    peerUDPRunning = true;
    std::thread(&Client::peerUDPRead, this).detach();
    std::thread(&Client::peerUDPSend, this).detach();

    return true;
}

WSAEVENT readEvent;

void Client::peerUDPSend(){
    char *bp;

    while(peerUDPRunning){
        if(micQueue.isEmpty()){
            continue;
        }
        QByteArray data = micQueue.front();
        bp = data.data();

        conn.sendto(socket_peerUDP, peer_addr, bp, data.size());
        micQueue.pop_front();
    }
    WSACleanup();
    qDebug()<<"close socket_peerUDP";
    closesocket (socket_peerUDP);
}


void Client::peerUDPRead(){
    char rbuf[BUFFERSIZE];
    WSAEVENT readEvent;

    if(!conn.WSACreateEvent(readEvent))
        return;
    if(!conn.WSAEventSelect(socket_peerUDP, readEvent, FD_READ))
        return;

    while (peerUDPRunning) {

        if(!conn.WSAWaitForMultipleEvents(readEvent))
            break;

        WSAResetEvent(readEvent);

        int n = conn.recvfrom(socket_peerUDP, peer_addr, rbuf);

        if(n<=0)
            continue;

        char *data = new char[n];
        memcpy(data, rbuf, n);
        emit receivedPeerData(data, n);
    }
    WSACleanup();
    qDebug()<<"close socket_peerUDP";
    closesocket (socket_peerUDP);
}

void Client::addMicStream(QByteArray &&data) {
    micQueue.push_back(data);
}
