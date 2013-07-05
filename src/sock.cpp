/****************************************************************************

    PROGRAM:   shantou_cloud_storage

    FILE:      sock.cpp

    PURPOSE:   gather performence and configure data from storages and switchs at schedule time 

    DESCRIPTION:
    					1) install a windows service name "stDataGather"
							2) gather performence and configure data from storages and switchs at schedule time,and transfer data to data manager server

    CREATE:   20130425
    AUTHOR:   yangshengcheng@gzcss.net
    PLATFORM: windows7/windows2003

****************************************************************************/
#include <windows.h>
#include <winsock.h>
#include <stdio.h>

#include "global.h"
#include "sock.h"
#include "log.h"

/* micro define */
 /* socket */
#define in_addr_t	unsigned long

/* Indicate if WSAStartup was called */
WSADATA ws_data;
BOOL WSockStarted = false;
/* Connection socket */
SOCKET WSockSocket = INVALID_SOCKET;
/* Where to send syslog information */
struct sockaddr_in WSockAddress;
 
//socket
/* Start Winsock access */
int WSockStart()
{
	/* Check to see if started */
	if (WSockStarted == FALSE) {

		/* See if version 2.0 is available */
		if (WSAStartup(MAKEWORD(2, 0), &ws_data)) {
			//cuteLog(LOG_ERROR, "Cannot initialize WinSock interface");
			return -1;
		}

		/* Set indicator */
		WSockStarted = TRUE;
	}

	/* Success */
	return 0;
}

/* Stop Winsock access */
void WSockStop()
{
	/* Check to see if started */
	if (WSockStarted) {

		/* Clean up winsock interface */
		WSACleanup();

		/* Reset indicator */
		WSockStarted = FALSE;
	}
}


/* Open connection to remote server */
int WSockOpen(char * loghost,int port)
{
	in_addr_t ip;
	struct hostent * host;

	
	/* Attempt to convert IP number */
	ip = inet_addr(loghost);
	if (ip == (in_addr_t)(-1))
	{

		/* Attempt to convert host name */
		host = gethostbyname(loghost);
		if (host == NULL) 
		{
			TextLog("warning","Invalid log host");
			return -1;
		}

		/* Set ip */
		ip = *(in_addr_t *)host->h_addr;
	}

	/* Initialize remote address structure */
	memset(&WSockAddress, 0, sizeof(WSockAddress));
	WSockAddress.sin_family = AF_INET;
	WSockAddress.sin_port = htons(port);
	WSockAddress.sin_addr.s_addr = ip;

	/* Create socket */
	WSockSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (WSockSocket == INVALID_SOCKET) 
	{
		TextLog("warning","Cannot create a datagram socket");
		return -1;
	}

	/* connect to tcp server*/
	if(connect(WSockSocket,(SOCKADDR*)&WSockAddress,sizeof(WSockAddress)) == SOCKET_ERROR)
	{
		TextLog("warning","connect tcp server fail");
		return -1;
	}

	/* Success */
	return 0;
}

/** listen to client **/
int WSockListen()
{
	if(bind(WSockSocket,(LPSOCKADDR)&WSockAddress,sizeof(sockaddr_in)) < 0)
	{
		TextLog("error","bind socket fail\n");
		return -1;
	}

	if(listen(WSockSocket,20) < 0)
	{
		TextLog("error","listen fail\n");
		return -1;
	}

	return 0;

}

/* Send data to receive */
int WSockSend(char * message)
{
	int len;
	int sendlen;

	/* Get message length */
	len = (int) strlen(message);

	/* Send to remote server */
	sendlen = send(WSockSocket, message, len, 0);
	if (sendlen == SOCKET_ERROR) 
	{
		TextLog("error","Cannot send message through socket");
		return -1;
	}

	/* Success */
	return sendlen;
}


/* Close connection */
void WSockClose()
{
	/* Close if open */
	if (WSockSocket != INVALID_SOCKET) {
		closesocket(WSockSocket);
		WSockSocket = INVALID_SOCKET;
	}
}