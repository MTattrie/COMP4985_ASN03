/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Client.cpp - handles client side function of the program such as establishing connection
--                              and sending commands
--
-- PROGRAM: Client (ComAudio - Final Project)
--
-- FUNCTIONS:
-- AudioPlayer();
-- bool openWavFile(const QString &fileName);
-- const QAudioFormat &fileFormat() const;
-- qint64 headerLength() const;
-- qint64 pos() const;
-- bool seek(qint64 pos);
-- bool pause();
-- bool start();
-- QAudioFormat &fastForward();
-- bool isPlaying();
-- bool isPaused();
-- bool isFastForwarding();
-- bool isFastForwarding(bool forward);
-- QByteArray readHeaderData();
-- QByteArray readChunkData(qint64 len, qint64 pos);
-- bool addChunkData(const char *data, qint64 len);
-- bool readHeader(const char *data, qint64 len);
-- void resetPlayer();
-- qint64 bytesAvailable() const;
-- void setProgressData(int current, int max = 0);
--
--
-- DATE: April 11 2017
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- NOTES:
-- Client cpp containing all functionality to connect to the server, and other clients
-- on client start it creates a thread to listen for other clients and the server return commands and server streaming
----------------------------------------------------------------------------------------------------------------------*/

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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: Client(QObject *parent)
--
-- DATE: April 10, 2017
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Jacob Frank, Terry Kang
--
-- INTERFACE: Client(QObject *parent)
--              *parent - parent object
--
-- RETURNS: is a constructor
--
-- NOTES:
-- Client constructor
----------------------------------------------------------------------------------------------------------------------*/
Client::Client(QObject *parent) : QObject(parent)
{
    isDonwloading = false;
    peerUDPRunning = false;
    isUploading = false;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: start
--
-- DATE: April 10, 2017
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER:
--
-- INTERFACE: start(QString hostname, QString port)
--
-- RETURNS: void.
--
-- NOTES:
--starts threads to store server details(available songs, and playlist
--starts tcp thread to listen for connection/commands
--starts udp thread to listen for song packets
----------------------------------------------------------------------------------------------------------------------*/
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
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER:
--
-- INTERFACE: storeServerDetails(QString hostname, QString port)
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: startTCP
--
-- DATE: April 10, 2017
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER:
--
-- INTERFACE: startTCP()
--
-- RETURNS: void.
--
-- NOTES:
--  starts tcp connection, checks WSA startup has happened
----------------------------------------------------------------------------------------------------------------------*/
void Client::startTCP(){
    if(!conn.WSAStartup())
        return;
    connectTCP();
    WSACleanup();
    closesocket (socket_tcp);
    qDebug() << "Client::startTCP() Socket " << socket_tcp << " closed";
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: startUDP
--
-- DATE: April 10, 2017
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: startUDP()
--
-- RETURNS: void.
--
-- NOTES:
--  starts udp connection, checks WSA startup has happened
----------------------------------------------------------------------------------------------------------------------*/
void Client::startUDP(){
    if(!conn.WSAStartup())
        return;
    connectUDP();
    WSACleanup();
    closesocket (socket_udp);
    qDebug() << "Client::startUDP() Socket " << socket_udp << " closed";
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: connectTCP
--
-- DATE: April 10, 2017
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Terry Kang
--
-- INTERFACE: connectTCP()
--
-- RETURNS: void.
--
-- NOTES:
--  connects TCP
----------------------------------------------------------------------------------------------------------------------*/
void Client::connectTCP(){

    if(!conn.WSASocketTCP(socket_tcp))
        return;
    if(!conn.setoptSO_REUSEADDR(socket_tcp))
        return;
    if(!conn.connect(socket_tcp, serverHostName, portNumber))
        return;
    runTCP();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: connectUDP
--
-- DATE: April 10, 2017
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER:
--
-- INTERFACE: connectUDP()
--
-- RETURNS: void.
--
-- NOTES:
--  connects UDP
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: runTCP
--
-- DATE: April 10, 2017
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER:
--
-- INTERFACE: runTCP()
--
-- RETURNS: void.
--
-- NOTES:
--  run TCP
----------------------------------------------------------------------------------------------------------------------*/
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


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: runUDP
--
-- DATE: April 10, 2017
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER:
--
-- INTERFACE: runUDP()
--
-- RETURNS: void.
--
-- NOTES:
--  run UDP connection, while there is information in the socket read from it.
----------------------------------------------------------------------------------------------------------------------*/
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


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: requestSong()
--
-- DATE: April 10, 2017
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER:
--
-- INTERFACE: requestSong(QString song)
--              QString song - the song filename to download
--
-- RETURNS: void.
--
-- NOTES:
--  requests a song to be downloaded by writing a download command to the socket
----------------------------------------------------------------------------------------------------------------------*/
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


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: reqeustCommand()
--
-- DATE: April 10, 2017
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER:
--
-- INTERFACE: reqeustCommand(int command, QString data)
--                          int command - the command to request (view global.h)
--                          QString data - the data to write to the socket (i.e what song to request)
--
-- RETURNS: void.
--
-- NOTES:
--  requests a song to be downloaded by writing a download command to the socket
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: sendSong
--
-- DATE: April 10, 2017
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER:
--
-- INTERFACE: sendSong(QString song)
--              QString song - the song to send
--
-- RETURNS: void.
--
-- NOTES:
--  wrapper to send song to server, prevents it from trying to send multiple times by having a boolean lock on it
----------------------------------------------------------------------------------------------------------------------*/
void Client::sendSong(QString song){
    if(isUploading)
        return;

    isUploading = true;
    sendFile(song);
    isUploading = false;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: sendFile
--
-- DATE: April 10, 2017
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER:
--
-- INTERFACE: sendFile(QString filename){
--              QString filename - the song filename to send
--
-- RETURNS: bool - success on successfully sending a file, false on overlapped error
--
-- NOTES:
--  sends files to the socket, loads the file into the QByte array then starts the overlapped routine
--  and sets the flags. While there are packets, copy the packet from the file into the buffer to send and
--  pops it from the Queue. Writes the buffer to the socket and continues overlapped io function
--
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: loadFile
--
-- DATE: April 10, 2017
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER:
--
-- INTERFACE: loadFile(QQueue<QByteArray>& packets, const QString filepathname){
--
-- RETURNS: bool. success on loading a file, failure on opening input/output device
--
-- NOTES:
--  opens file to send copying the song into a QByteArray. At the end of a song it appends a COMPLETE command to let the
--  other side its finished
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: startPeerUDP
--
-- DATE: April 10, 2017
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER:
--
-- INTERFACE: startPeerUDP(QString hostname, QString port)
--              QString hostname - the ip address of the client to connect to for peer to peer
--              QString port - the client port for peer to peer udp
--
-- RETURNS: bool. success on connection to other client
--
-- NOTES:
-- starts peer to peer udp connection getting the port and ip address from the gui
-- once the socket is set up, it starts a thread to send microphone data and receive microphone data
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: peerUDPSend
--
-- DATE: April 10, 2017
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER:
--
-- INTERFACE: peerUDPSend()
--
-- RETURNS: void
--
-- NOTES:
-- starts peer to peer udp connection getting the port and ip address from the gui
-- once the socket is set up, it starts a thread to send microphone data and receive microphone data
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: peerUDPRead
--
-- DATE: April 10, 2017
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER:
--
-- INTERFACE: peerUDPRead()
--
-- RETURNS: void
--
-- NOTES:
-- starts peer to peer udp connection getting the port and ip address from the gui
-- once the socket is set up, it starts a thread to send microphone data and receive microphone data
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: addMicStream
--
-- DATE: April 10, 2017
--
-- DESIGNER: Terry Kang, Deric Mccadden, Jacob Frank, Mark Tattrie
--
-- PROGRAMMER: Terry Kang, Mark Tattrie
--
-- INTERFACE: addMicStream(QByteArray &&data)
--              QByteArray &&data - data to be added to the micQueue to stream
--
-- RETURNS: void
--
-- NOTES:
-- mic stream to play from the microphone
----------------------------------------------------------------------------------------------------------------------*/
void Client::addMicStream(QByteArray &&data) {
    micQueue.push_back(data);
}
