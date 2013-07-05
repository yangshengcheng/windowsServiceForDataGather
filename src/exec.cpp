/****************************************************************************

    PROGRAM:   shantou_cloud_storage

    FILE:      exec.cpp

    PURPOSE:   gather performence and configure data from storages and switchs at schedule time 

    DESCRIPTION:
    					1) install a windows service name "stDataGather"
							2) gather performence and configure data from storages and switchs at schedule time,and transfer data to data manager server

    CREATE:   20130425
    AUTHOR:   yangshengcheng@gzcss.net
    PLATFORM: windows7/windows2003

****************************************************************************/
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <sys/stat.h>
//PathStripPath
#include "Shlwapi.h"

#ifndef MAX_ELEMENT
	#define MAX_ELEMENT 20
#endif

#ifndef MAX_BUFF_SIZE
	#define MAX_BUFF_SIZE 1024*10
#endif

#include "global.h"
#include "sock.h"
#include "log.h"
#include "exec.h"


void __cdecl exec(LPVOID lpParam)
{
	
	task * ts_temp= (task*)lpParam;
	char temp[MAX_BUFF_SIZE];
	sprintf(temp,"schedule->%d,timeout->%d,timer->%d,task_type->%s,cmd->%s,ipaddress->%s,path->%s\n",ts_temp->schedule,ts_temp->timeout,ts_temp->timer,ts_temp->type,ts_temp->cmd,ts_temp->ipaddress,ts_temp->path);
	TextLog("debug",temp);
	
	char* filename = (char*)malloc(sizeof(char) * MAX_BUFF_SIZE);
	memset(filename,0,sizeof(char) * MAX_BUFF_SIZE);
	strcpy(filename,ts_temp->path);
	
	//clean the data file
	FILE *datafile;
	datafile = fopen(ts_temp->path,"w");
	if(datafile == NULL)
	{
		char temp[MAX_BUFF_SIZE];
		sprintf(temp,"open %s fail",ts_temp->path);
		TextLog("error",temp);
		return;
	}
	fclose(datafile);
	
	
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	memset(&si,0,sizeof(si));
	si.cb = sizeof(si);
	memset(&pi,0,sizeof(pi));
	
	// Start the child process. 
	if( !CreateProcess( NULL,   // No module name (use command line)
       ts_temp->cmd,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
	) 
	{
		TextLog("error","execute dir fail");
		return ;
	}
	else
	{
		char temp[512];
		sprintf(temp,"processID:%d ,dwProcessId:%d\n",pi.hProcess,pi.dwProcessId);
		TextLog("debug",temp);
	}

	if(WaitForSingleObject( pi.hProcess, 1000 * 10 ) == WAIT_TIMEOUT)
	{
		TerminateProcess(pi.hProcess,1);
		TextLog("error","timeout kill");
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );
		return ;
	}

	// Close process and thread handles. 
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );
	
	/* upload data file */
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

	/** transfer data file to receive server **/
	
	//send file name
	char* ret = "\n";
	PathStripPath(filename); 
	sendlen = WSockSend(filename);
	if(sendlen == -1)
	{
		TextLog("warning","send filename fail");
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
	
	
	return;
}