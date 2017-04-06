#include "client.h"

#include <stdio.h>
#include <winsock2.h>
#include <errno.h>
#include <thread>
#include <QDebug>
#include "connection.h"
#include "packet.h"


Client::Client(QObject *parent) : QObject(parent)
{

}


Connection conn;
SOCKET socket_tcp;
SOCKET socket_udp;



void Client::startThreads(){
    std::thread(&Client::startTCP, this).detach();
    std::thread(&Client::startUDP, this).detach();
}

void Client::startTCP(){
    if(!conn.WSAStartup())
        return;
    runTCP();
    WSACleanup();
    closesocket (socket_tcp);
    qDebug() << "Client::startTCP() Socket " << socket_tcp << " closed";
}

void Client::startUDP(){
    if(!conn.WSAStartup())
        return;
    runUDP();
    WSACleanup();
    closesocket (socket_udp);
    qDebug() << "Client::startUDP() Socket " << socket_tcp << " closed";
}


void Client::runTCP(){
    WSAEVENT readEvent;
    char rbuf[DATA_BUFSIZE];
    string host = "localhost";
    int port =	7000;

    if(!conn.WSASocketTCP(socket_tcp))
        return;
    if(!conn.setsockopt(socket_tcp, SOL_SOCKET, SO_REUSEADDR))
        return;
    if(!conn.connect(socket_tcp, host, port))
        return;
    if(!conn.WSACreateEvent(readEvent))
        return;
    if(!conn.WSAEventSelect(socket_tcp, readEvent, FD_READ))
        return;

    while(true) {
        if(!conn.WSAWaitForMultipleEvents(readEvent))
            return;
        WSAResetEvent(readEvent);

        if(!conn.recv(socket_tcp, rbuf))
            continue;

        //Handle received songs / lists here.
    }
}



void Client::runUDP(){
    int port =	7000;
    char rbuf[DATA_BUFSIZE];

    if(!conn.WSASocketUDP(socket_udp))
        return;
    if(!conn.setsockopt(socket_udp, SOL_SOCKET, SO_REUSEADDR))
        return;

    if(!conn.bind(socket_udp, port))
        return;

    while (true) {
        if(!conn.recv(socket_tcp, rbuf))
            continue;
        qDebug() << "UDP stream: " << rbuf;
    }
}


void Client::requestSong(QString song){
    char sbuf[DATA_BUFSIZE];

    QString header = "HEADER REQUEST SONG: ";
    header.append(song);
    header.append(" HEADER");

//    CommandPacket packet;
//    packet.command = Command::SEND;
//    packet.song = song.toStdString();

    memset((char *)sbuf, 0, DATA_BUFSIZE);
    memcpy(sbuf, header.toStdString().c_str(), DATA_BUFSIZE);

    if(!conn.send(socket_tcp, sbuf))
        return;
}












