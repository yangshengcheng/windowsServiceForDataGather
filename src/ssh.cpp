/****************************************************************************

    PROGRAM:   shantou_cloud_storage

    FILE:      ssh.cpp

    PURPOSE:   gather performence and configure data from storages and switchs at schedule time 

    DESCRIPTION:
    					1) install a windows service name "stDataGather"
							2) gather performence and configure data from storages and switchs at schedule time,and transfer data to data manager server

    CREATE:   20130425
    AUTHOR:   yangshengcheng@gzcss.net
    PLATFORM: windows7/windows2003

****************************************************************************/

#include "libssh2_config.h"
#include <libssh2.h>
#include <windows.h>

#ifdef HAVE_WINSOCK2_H
# include <winsock2.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#ifdef HAVE_SYS_SELECT_H
# include <sys/select.h>
#endif
# ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif

#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>

//PathStripPath
#include "Shlwapi.h"

#include "global.h"
#include "sock.h"
#include "ssh.h"
#include "log.h"



static int waitsocket(int socket_fd, LIBSSH2_SESSION *session)
{
    struct timeval timeout;
    int rc;
    fd_set fd;
    fd_set *writefd = NULL;
    fd_set *readfd = NULL;
    int dir;

    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    FD_ZERO(&fd);

    FD_SET(socket_fd, &fd);

    /* now make sure we wait in the correct direction */
    dir = libssh2_session_block_directions(session);

    if(dir & LIBSSH2_SESSION_BLOCK_INBOUND)
        readfd = &fd;

    if(dir & LIBSSH2_SESSION_BLOCK_OUTBOUND)
        writefd = &fd;

    rc = select(socket_fd + 1, readfd, writefd, NULL, &timeout);

    return rc;
}

void __cdecl ssh_sw(LPVOID lpParam)
{
	
	task * ts_temp= (task*)lpParam;
	char temp[MAX_BUFF_SIZE];
	memset(temp,0,sizeof(temp));
	sprintf(temp,"ipaddress->%s",ts_temp->ipaddress);
	//sprintf(temp,"ipaddress->%s,passwd->%s,port->%d,schedule->%d,timeout->%d,timer->%d,task_type->%s,cmd->%s,path->%s\n",ts_temp->ipaddress,ts_temp->user,ts_temp->passwd,ts_temp->port,ts_temp->schedule,ts_temp->timeout,ts_temp->timer,ts_temp->type,ts_temp->cmd,ts_temp->path);
	TextLog("debug",temp);
	
	const char *hostname = ts_temp->ipaddress;
	const char *commandline = ts_temp->cmd;
	const char *username    = ts_temp->user;
	const char *password    = ts_temp->passwd;
	int port = ts_temp->port;
	
	char* filename = (char*)malloc(sizeof(char) * MAX_BUFF_SIZE);
	memset(filename,0,sizeof(char) * MAX_BUFF_SIZE);
	strcpy(filename,ts_temp->path);
			
	unsigned long hostaddr;
	int sock;
	struct sockaddr_in sin;
	const char *fingerprint;
	LIBSSH2_SESSION *session;
	LIBSSH2_CHANNEL *channel;
	int rc;
	int exitcode;
	char *exitsignal=(char *)"none";
	int bytecount = 0;
	int max_byte = 1024 * 1024;
	size_t len;
	LIBSSH2_KNOWNHOSTS *nh;
	int type;
	
	#ifdef WIN32
		WSADATA wsadata;
		WSAStartup(MAKEWORD(2,0), &wsadata);
	#endif
	
	rc = libssh2_init (0);
	if (rc != 0) 
	{
		
		//fprintf (stderr, "libssh2 initialization failed (%d)\n", rc);
		TextLog("error","libssh2 initialization failed");
		return ;
	}
	
	hostaddr = inet_addr(hostname);
	
	/* Ultra basic "connect to port 22 on localhost"
	* Your code is responsible for creating the socket establishing the
	* connection
	*/
	sock = socket(AF_INET, SOCK_STREAM, 0);
	
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = hostaddr;
	if (connect(sock, (struct sockaddr*)(&sin),sizeof(struct sockaddr_in)) != 0) 
	{
		//fprintf(stderr, "failed to connect!\n");
		TextLog("error","failed to connect!");
		return ;
	}
	
	/* Create a session instance */
	session = libssh2_session_init();
	if (!session)
	{
		return ;
	}
	/* tell libssh2 we want it all done non-blocking */
	libssh2_session_set_blocking(session, 1);
	
	/* ... start it up. This will trade welcome banners, exchange keys,
	* and setup crypto, compression, and MAC layers
	*/
	while ((rc = libssh2_session_handshake(session, sock)) == LIBSSH2_ERROR_EAGAIN);
	if (rc) 
	{
		//fprintf(stderr, "Failure establishing SSH session: %d\n", rc);
		TextLog("error","Failure establishing SSH session");
		return ;
	}
	
	nh = libssh2_knownhost_init(session);
	if(!nh) 
	{
		/* eeek, do cleanup here */
		return ;
	}
	
	/* read all hosts from here */
	libssh2_knownhost_readfile(nh, "known_hosts",LIBSSH2_KNOWNHOST_FILE_OPENSSH);
	
	/* store all known hosts to here */
	libssh2_knownhost_writefile(nh, "dumpfile",LIBSSH2_KNOWNHOST_FILE_OPENSSH);
	
	fingerprint = libssh2_session_hostkey(session, &len, &type);
	if(fingerprint)
	{
		struct libssh2_knownhost *host;
		#if LIBSSH2_VERSION_NUM >= 0x010206
			/* introduced in 1.2.6 */
			int check = libssh2_knownhost_checkp(nh, hostname, 22,
	                                             fingerprint, len,
	                                             LIBSSH2_KNOWNHOST_TYPE_PLAIN|
	                                             LIBSSH2_KNOWNHOST_KEYENC_RAW,
	                                             &host);
		#else
		/* 1.2.5 or older */
			int check = libssh2_knownhost_check(nh, hostname,
	                                            fingerprint, len,
	                                            LIBSSH2_KNOWNHOST_TYPE_PLAIN|
	                                            LIBSSH2_KNOWNHOST_KEYENC_RAW,
	                                            &host);
	#endif
			//fprintf(stderr, "Host check: %d, key: %s\n", check,(check <= LIBSSH2_KNOWNHOST_CHECK_MISMATCH)?host->key:"<none>");
			
	
		/*****
	         * At this point, we could verify that 'check' tells us the key is
	         * fine or bail out.
	         *****/
	}
	else 
	{
		/* eeek, do cleanup here */
		return ;
	}
	
	libssh2_knownhost_free(nh);
	
	if ( strlen(password) != 0 ) 
	{
		/* We could authenticate via password */
		while ((rc = libssh2_userauth_password(session, username, password)) == LIBSSH2_ERROR_EAGAIN);
		if (rc) 
		{
			//fprintf(stderr, "Authentication by password failed.\n");
			TextLog("error","Authentication by password failed.");
			goto shutdown;
		}
	}
	else 
	{
		//do not support public key ,yangshengcheng@gzcss.net
		/* Or by public key */
		/*
		while ((rc = libssh2_userauth_publickey_fromfile(session, username,
	                                                         "/home/user/"
	                                                         ".ssh/id_rsa.pub",
	                                                         "/home/user/"
	                                                         ".ssh/id_rsa",
	                                                         password)) == LIBSSH2_ERROR_EAGAIN);
		if (rc) 
		{
			fprintf(stderr, "\tAuthentication by public key failed\n");
			goto shutdown;
		}
		*/
		TextLog("error","we are not support Authentication by public key");
		goto shutdown;
	}
	
	#if 0
	    libssh2_trace(session, ~0 );
	#endif
	
	/* Exec non-blocking on the remove host */
	while( (channel = libssh2_channel_open_session(session)) == NULL && libssh2_session_last_error(session,NULL,NULL,0) == LIBSSH2_ERROR_EAGAIN )
	{
		waitsocket(sock, session);
	}
	if( channel == NULL )
	{
		//fprintf(stderr,"Error\n");
		TextLog("error","channel NULL error ");
		return ;
	}

	while( (rc = libssh2_channel_exec(channel, commandline)) == LIBSSH2_ERROR_EAGAIN )
	{
		waitsocket(sock, session);
	}
	if( rc != 0 )
	{
		//fprintf(stderr,"Error\n");
		TextLog("error","exec command fail ");
		return ;
	}
	
	/* open data file for data buff */
	FILE *file = NULL;
	file = fopen(ts_temp->path,"w");
	if(file == NULL)
	{
		char temp[MAX_BUFF_SIZE];
		memset(temp,0,sizeof(temp));
		sprintf(temp,"open file %s fail",ts_temp->path);
		TextLog("error",temp);	
		return ;
	}
	
	for( ;; )
	{
		/* loop until we block */
		int rc;
		do
		{
			char buffer[0x8000];
			rc = libssh2_channel_read( channel, buffer, sizeof(buffer) );
	
			if( rc > 0 )
			{
				int i;
				bytecount += rc;
				/* max size 1 M*/
				if(bytecount > max_byte)
				{
					break;
				}
				//TextLog("debug",buffer);
				//fprintf(stderr, "We read:\n");

					for( i=0; i < rc; ++i )
					{
							fputc( buffer[i], file);
					}

				//fprintf(stderr, "\n");
			}
			else 
			{
				if( rc != LIBSSH2_ERROR_EAGAIN )
				{
					/* no need to output this for the EAGAIN case */
					char temp[1024];
					sprintf(temp,"libssh2_channel_read returned %d\n", rc);
					TextLog("warning",temp);
					
					//fprintf(stderr, "libssh2_channel_read returned %d\n", rc);
				}
			}
		}while( rc > 0 );
	
	
		/* this is due to blocking that would occur otherwise so we loop on this condition */
		if( rc == LIBSSH2_ERROR_EAGAIN )
		{
			waitsocket(sock, session);
		}
		else
		{
			break;
		}
	}
	
	/* close the file */
	fclose(file);
	
	exitcode = 127;
	while( (rc = libssh2_channel_close(channel)) == LIBSSH2_ERROR_EAGAIN )
	{
		waitsocket(sock, session);
	}
	if( rc == 0 )
	{
		exitcode = libssh2_channel_get_exit_status( channel );
		libssh2_channel_get_exit_signal(channel, &exitsignal,NULL, NULL, NULL, NULL, NULL);
	}
	
	if (exitsignal)
	{
		char temp[256];
		sprintf(temp,"Got signal:%s",exitsignal);
		TextLog("debug",temp);
		//fprintf(stderr, "\nGot signal: %s\n", exitsignal);
	}
	else 
	{
		char temp[256];
		sprintf(temp,"exit code:%d",exitcode);
		TextLog("debug",temp);
		//fprintf(stderr, "\nEXIT: %d bytecount: %d\n", exitcode, bytecount);
	}
	libssh2_channel_free(channel);
	channel = NULL;
	
	shutdown:
	
	    libssh2_session_disconnect(session,"Normal Shutdown");
	    libssh2_session_free(session);
	
	#ifdef WIN32
		closesocket(sock);
	#else
		close(sock);
	#endif
	
	//fprintf(stderr, "all done\n");
	TextLog("debug","ssh all done");
	
	libssh2_exit();
	
	
	/* now upload the file */
	struct stat ss;
	unsigned long filesize;

	FILE* StatusFile;
	char line[MAX_BUFF_SIZE + 1];

	unsigned long count = 0;
	int sendlen;

	/* get file size */
	if(stat(ts_temp->path,&ss) < 0)
	{
		TextLog("error","get data file size fail");
		return;
	}
	//filesize = getFileSize(path);
	filesize = (unsigned long)ss.st_size;

	if(filesize == 0)
	{

		TextLog("debug","data file is empty");
		return ;
	}

	if(WSockStart() == -1)
	{
		TextLog("error","WSockStart fail");
		return;
	}

	if(WSockOpen(server_hostname,server_port) == -1)
	{
		TextLog("error","WSockOpen fail");
		return;
	}

	/** transfer status file to receive server **/

	//send file name
	char* ret = "\n";
	PathStripPath(filename); 
	sendlen = WSockSend(filename);
	if(sendlen == -1)
	{
		TextLog("warning","send filename fail");
			// close socket
		WSockClose();

	// socket stop
		WSockStop();
		return;
	}
	
	WSockSend(ret);
	
	StatusFile = fopen(ts_temp->path,"r");
	if(StatusFile == NULL)
	{
		TextLog("warning","open datafile fail");
		return;
	}

	while(fgets(line,MAX_BUFF_SIZE,StatusFile) != NULL)
	{
		/* send data */
		sendlen = WSockSend(line);
		if(sendlen == -1)
		{
			TextLog("warning","send status data fail");
			break;
		}
	}
	
	fclose(StatusFile);

	// close socket
	WSockClose();

	// socket stop
	WSockStop();
	
	return ;
}