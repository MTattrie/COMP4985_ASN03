#include "client.h"

#include <stdio.h>
#include <winsock2.h>
#include <errno.h>
//#include <string.h>
//#include <memory.h>

#define SERVER_TCP_PORT			7000	// Default port
#define BUFSIZE					8192		// Buffer length


Client::Client(QObject *parent) : QObject(parent)
{

}



void Client::start(){
    int n, ns, bytes_to_read;
    int port, err;
    SOCKET sd;
    struct hostent	*hp;
    struct sockaddr_in server;
    char  *host, *bp, rbuf[BUFSIZE], sbuf[BUFSIZE], **pptr;
    WSADATA WSAData;
    WORD wVersionRequested;

    host =	(char*)"localhost";
    port =	SERVER_TCP_PORT;

    wVersionRequested = MAKEWORD( 2, 2 );
    err = WSAStartup( wVersionRequested, &WSAData );
    if ( err != 0 ) //No usable DLL
    {
        printf ("DLL not found!\n");
        exit(1);
    }

    // Create the socket
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Cannot create socket");
        exit(1);
    }

    // Initialize and set up the address structure
    memset((char *)&server, 0, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    if ((hp = gethostbyname(host)) == NULL)
    {
        fprintf(stderr, "Unknown server address\n");
        exit(1);
    }

    // Copy the server address
    memcpy((char *)&server.sin_addr, hp->h_addr, hp->h_length);

    // Connecting to the server
    if (::connect (sd, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        fprintf(stderr, "Can't connect to server\n");
        perror("connect");
        exit(1);
    }
    printf("Connected:    Server Name: %s\n", hp->h_name);
    pptr = hp->h_addr_list;
    printf("\t\tIP Address: %s\n", inet_ntoa(server.sin_addr));
    printf("Transmiting:\n");

    memset((char *)sbuf, 0, sizeof(sbuf));

    //gets(sbuf); // get user's text
    memcpy(sbuf, "test message", sizeof(sbuf));

    // Transmit data through the socket
    ns = send (sd, sbuf, BUFSIZE, 0);
    printf("Receive:\n");
    bp = rbuf;
    bytes_to_read = BUFSIZE;

    // client makes repeated calls to recv until no more data is expected to arrive.
    while ((n = recv (sd, bp, bytes_to_read, 0)) < BUFSIZE)
    {
        bp += n;
        bytes_to_read -= n;
        if (n == 0)
            break;
    }
    printf ("%s\n", rbuf);
    closesocket (sd);
    WSACleanup();
    exit(0);
}

