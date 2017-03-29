#include "client.h"

#include <stdio.h>
#include <winsock2.h>
#include <errno.h>
#include <thread>
#include <QDebug>
#include "connection.h"


Client::Client(QObject *parent) : QObject(parent)
{

}


Connection conn;
SOCKET sd_tcp;



void Client::startTCP(){
    std::thread(&Client::connectTCP, this).detach();
}


void Client::connectTCP(){
    if(!conn.WSAStartup())
        return;
    runTCP();
    WSACleanup();
    closesocket (sd_tcp);
    qDebug() << "Client::connectTCP() Socket " << sd_tcp << " closed";
}


void Client::runTCP(){
    WSAEVENT readEvent;
    char rbuf[DATA_BUFSIZE];
    string host = "localhost";
    int port =	7000;

    if(!conn.WSASocketTCP(sd_tcp))
        return;
    if(!conn.connect(sd_tcp, host, port))
        return;
    if(!conn.WSACreateEvent(readEvent))
        return;
    if(!conn.WSAEventSelect(sd_tcp, readEvent, FD_READ))
        return;

    while(true) {
        if(!conn.WSAWaitForMultipleEvents(readEvent))
            return;
        WSAResetEvent(readEvent);

        if(!conn.recv(sd_tcp, rbuf))
            continue;

        //Handle received songs / lists here.
    }
}


void Client::requestSong(QString song){
    char sbuf[DATA_BUFSIZE];

    QString header = "HEADER REQUEST SONG: ";
    header.append(song);
    header.append(" HEADER");

    memset((char *)sbuf, 0, DATA_BUFSIZE);
    memcpy(sbuf, header.toStdString().c_str(), DATA_BUFSIZE);

    if(!conn.send(sd_tcp, sbuf))
        return;
}








