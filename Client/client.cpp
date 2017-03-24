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
    runTCP();
    WSACleanup();
    closesocket (sd_tcp);
}


void Client::runTCP(){

    string host = "localhost";
    int port =	SERVER_TCP_PORT;

    if(!conn.WSAStartup())
        return;
    if(!conn.WSASocketTCP(sd_tcp))
        return;
    if(!conn.connect(sd_tcp, host, port))
        return;

    char sbuf[DATA_BUFSIZE];

    QString header = "HEADER REQUEST SONG: ";
    header.append("song");
    header.append(" HEADER");

    memset((char *)sbuf, 0, sizeof(sbuf));
    memcpy(sbuf, header.toStdString().c_str(), sizeof(sbuf));

    if(!conn.WSASend(sd_tcp, sbuf))
        return;

//    char rbuf[DATA_BUFSIZE];
//    while(true) {
//        if(!conn.WSARecv(sd_tcp, rbuf))
//            return;

//        qDebug() << "Socket " << sd_tcp << " connected" << endl;
//    }
}


void Client::requestSong(QString song){
    char sbuf[DATA_BUFSIZE];

    QString header = "HEADER REQUEST SONG: ";
    header.append(song);
    header.append(" HEADER");

    memset((char *)sbuf, 0, DATA_BUFSIZE);
    memcpy(sbuf, header.toStdString().c_str(), DATA_BUFSIZE);

    if(!conn.WSASend(sd_tcp, sbuf))
        return;
}





