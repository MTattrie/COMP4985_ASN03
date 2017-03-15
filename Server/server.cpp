#include "server.h"
#include "connection.h"

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <thread>
#include <QDebug>
#include <string>


void CALLBACK WorkerRoutine(DWORD Error, DWORD BytesTransferred,
        LPWSAOVERLAPPED Overlapped, DWORD InFlags);


Connection conn;
SOCKET AcceptSocket;


Server::Server(QObject *parent) : QObject(parent)
{

}


void Server::startTCP(){

    SOCKET ListenSocket;
    WSAEVENT AcceptEvent;

    if(!conn.WSAStartup())
        return;
    if(!conn.WSASocket_TCP(ListenSocket))
        return;
    if(!conn.bind(ListenSocket))
        return;
    if(!conn.listen(ListenSocket))
        return;
    if(!conn.WSACreateEvent(AcceptEvent))
        return;

    std::thread(&Server::workerThread, this, AcceptEvent).detach();

    while(TRUE)
    {
        AcceptSocket = accept(ListenSocket, NULL, NULL);

        if(!conn.WSASetEvent(AcceptEvent))
            return;
    }
}


void Server::workerThread(WSAEVENT event)
{
    LPSOCKET_INFORMATION SocketInfo;
    WSAEVENT EventArray[1];

    EventArray[0] = event;
    while(TRUE)
    {
        if(!conn.WSAWaitForMultipleEvents(EventArray))
            break;
        if(!conn.createSocketInfo(SocketInfo, AcceptSocket))
            break;
        if(!conn.WSARecv(SocketInfo, WorkerRoutine))
            break;

        qDebug() << "Socket " << AcceptSocket << " connected" << endl;
    }
    exit(1);
}


void CALLBACK WorkerRoutine(DWORD Error, DWORD BytesTransferred,
        LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
    (void)InFlags;//this is not used, but cannot be removed without breaking the callback.

    // Reference the WSAOVERLAPPED structure as a SOCKET_INFORMATION structure
    LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION) Overlapped;

    if (Error != 0)
    {
        qDebug() << "I/O operation failed with error " << Error << endl;
        closesocket(SI->Socket);
        GlobalFree(SI);
        return;
    }
    if (BytesTransferred == 0)
    {
        qDebug() << "Closing socket " << SI->Socket << endl;
        closesocket(SI->Socket);
        GlobalFree(SI);
        return;
    }

    // Check to see if the BytesRECV field equals zero. If this is so, then
    // this means a WSARecv call just completed so update the BytesRECV field
    // with the BytesTransferred value from the completed WSARecv() call.
    if (SI->BytesRECV == 0)
    {
        SI->BytesRECV = BytesTransferred;
        SI->BytesSEND = 0;
    }
    else
    {
        SI->BytesSEND += BytesTransferred;
    }

    if (SI->BytesRECV > SI->BytesSEND)
    {
        // Post another WSASend() request.
        // Since WSASend() is not gauranteed to send all of the bytes requested,
        // continue posting WSASend() calls until all received bytes are sent.
        ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
        SI->DataBuf.buf = SI->Buffer + SI->BytesSEND;
        SI->DataBuf.len = SI->BytesRECV - SI->BytesSEND;
        conn.WSASend(SI, WorkerRoutine);
    }
    else
    {
        SI->BytesRECV = 0;
        ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
        SI->DataBuf.len = DATA_BUFSIZE;
        SI->DataBuf.buf = SI->Buffer;
        conn.WSARecv(SI, WorkerRoutine);
    }
}
