/****************************************************************************

    PROGRAM:   shantou_cloud_storage

    FILE:      log.cpp

    PURPOSE:   gather performence and configure data from storages and switchs at schedule time 

    DESCRIPTION:
    					1) install a windows service name "stDataGather"
							2) gather performence and configure data from storages and switchs at schedule time,and transfer data to data manager server

    CREATE:   20130425
    AUTHOR:   yangshengcheng@gzcss.net
    PLATFORM: windows7/windows2003

****************************************************************************/


/* os and c include file */
#include <windows.h>
#include <winsock.h>
#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <lm.h>

/* _beginthread */
#include <process.h>

/* _access */
#include <io.h>

/* my own head file */
#include "global.h"
#include "stat.h"


task *ts[MAX_TASK];

char* task_list[MAX_TASK];
int len = 0;
int task_len = 0;
char* task_path = NULL;

 
/***** global varibales *****/

/* Service dispatch table */
SERVICE_TABLE_ENTRY ServiceDispatchTable[] = {
	{"stDataGather", (LPSERVICE_MAIN_FUNCTION)ServiceMain },
	{ NULL, NULL }
};

/* Service handle */
 SERVICE_STATUS_HANDLE ServiceStatusHandle;
 SERVICE_STATUS ServiceStatus;


/* some enviroment configure ,initialization from configure file */
char* data_home = NULL;
char* log_dir = NULL;
int server_port=0;
char* server_hostname = NULL;
char* logFileName = "shantou_cloud_storage.log";
char* logFilePath = NULL;

int debug = 0;



int main()
{	
	/* start the service*/
	if(ServiceStart() == -1)
	{
		TextLog("warning","start service fail");
		return 1;
	}

	return 0;

}


/* base function */


//service 
int InitService() 
{ 
	/* malloc */
	data_home = (char*)malloc(sizeof(char)*1024);
	log_dir = (char*)malloc(sizeof(char)*1024);
	server_hostname = (char*)malloc(sizeof(char)*15);
	logFilePath = (char*)malloc(sizeof(char)*1024);
	task_path = (char*)malloc(sizeof(char) * 1024);
	
	/* clean */
	memset(data_home,0,sizeof(char)*1024);
	memset(log_dir,0,sizeof(char)*1024);
	memset(server_hostname,0,sizeof(char)*15);
	memset(logFilePath,0,sizeof(char)*1024);
	memset(task_path,0,sizeof(char)*1024);
	
	/* get configure variables from config file */
	char portStr[10];
	char debugStr[10];
	
	ReadConfig("data_home",data_home);
	ReadConfig("log_dir",log_dir);
	ReadConfig("hostname",server_hostname);
	ReadConfig("port",portStr);
	ReadConfig("debug",debugStr);
	ReadConfig("task_path",task_path);
	
	server_port = atoi(portStr);
	debug = atoi(debugStr);
	
	if(log_dir == NULL)
	{
		logFilePath = "shantou_cloud_storage.log";
	}
	else
	{
		logFilePath = strncat(log_dir,logFileName,strlen(logFileName));
		logFilePath[strlen(log_dir)+strlen(logFileName)-1] = '\0';
	}
	
	if(task_path == NULL)
	{
		task_path ="st_cloundstorage_task.txt";
	}
		
	if(1)
	{
		/* log out the configure variables */
		char temp[1024];
		memset(temp,0,sizeof(temp));
		sprintf(temp,"data_home:%s",data_home);
		TextLog("debug",temp);
		
		memset(temp,0,sizeof(temp));
		sprintf(temp,"log_dir:%s",log_dir);
		TextLog("debug",temp);
		
		memset(temp,0,sizeof(temp));
		sprintf(temp,"hostname:%s",server_hostname);
		TextLog("debug",temp);
		
		memset(temp,0,sizeof(temp));
		sprintf(temp,"port:%s",portStr);
		TextLog("debug",temp);	
		
		memset(temp,0,sizeof(temp));
		sprintf(temp,"port:%s",task_path);
		TextLog("debug",temp);	
	}		
	
	/* check configure variables */
	if(server_port == 0)
	{
		server_port = 9527;
	}
	if(server_hostname == NULL)
	{
		//hostname = "0.0.0.0";
		TextLog("error","appoint the data manager server ip");
		return -1;
	}
	
	/* check if data_home exists */
	if(_access(data_home,0) == -1)
	{
		TextLog("error","the data_home directory is not exists");
		return -1;
	}


	/* load task data */
	len = load_task(task_path,task_list);
	
	for(int i = 0;i < len;i++)
	{
		TextLog("debug",task_list[i]);
	}
	
	char* temp[MAX_ELEMENT]  ;
	for(int k=0;k < MAX_ELEMENT;k++)
	{
		temp[k] = (char*)malloc(sizeof(char)* MAX_BUFF_SIZE);
		memset(temp[k],0,sizeof(char)* MAX_BUFF_SIZE);
	}
	
	for(int i=0;i < len;i++ )
	{
		split(temp,task_list[i],"##");

		task *ts_temp =(struct task*)malloc(sizeof(struct task));
		if(add_task(ts_temp,temp) == 1)
		{
			ts[i] = ts_temp;
			task_len++;
		}
		//fprintf(stdout,"schedule->%d,timeout->%d,timer->%d,task_type->%s,cmd->%s\n",ts_temp->schedule,ts_temp->timeout,ts_temp->timer,ts_temp->type,ts_temp->cmd);
		//fprintf(stdout,"type->%s,timer->%d,cmd->%s\n",ts[i]->list[0],ts[i]->timer,ts[i]->list[3]);
	}
	
	for(int i = 0;i < task_len;i++)
	{
		char temp[MAX_BUFF_SIZE];
		memset(temp,0,sizeof(temp));
		sprintf(temp,"schedule->%d,timeout->%d,timer->%d,task_type->%s,cmd->%s,ipaddress->%s,user->%s,passwd->%s,path->%s,port->%d\n",ts[i]->schedule,ts[i]->timeout,ts[i]->timer,ts[i]->type,ts[i]->cmd,ts[i]->ipaddress,ts[i]->user,ts[i]->passwd,ts[i]->path,ts[i]->port);
		TextLog("debug",temp);
	}
	
	TextLog("debug","stDataGather service init success!");
	
	return 0;
}

/* Process service changes */
void ControlHandler(DWORD request) 
{  
   switch(request) 
   { 
      case SERVICE_CONTROL_STOP: 

		/* free the memory when exit service */
		if(data_home != NULL)
		{
			free(data_home);
		}
		
		if(log_dir != NULL)
		{
			free(log_dir);
		}
		
		if(server_hostname != NULL)
		{
			free(server_hostname);
		}
		
		free_parray(task_list,MAX_TASK);
		free_task(ts,task_len);
		
		if(logFilePath != NULL)
		{
			free(logFilePath);
		}

		OutputDebugString("Monitoring stopped.");
        TextLog("warning","stDataGather service stopped.");

        ServiceStatus.dwWin32ExitCode = 0; 
        ServiceStatus.dwCurrentState = SERVICE_STOPPED; 
        SetServiceStatus (ServiceStatusHandle, &ServiceStatus);
        return; 
 
      case SERVICE_CONTROL_SHUTDOWN: 
		
		/* free the memory when exit service */
		if(data_home != NULL)
		{
			free(data_home);
		}
		
		if(log_dir != NULL)
		{
			free(log_dir);
		}
		
		if(server_hostname != NULL)
		{
			free(server_hostname);
		}
		
		if(logFilePath != NULL)
		{
			free(logFilePath);
		}
		
		free_parray(task_list,MAX_TASK);
		free_task(ts,task_len);
		
		OutputDebugString("Monitoring stopped.");
        TextLog("warning","stDataGather service stopped.");

        ServiceStatus.dwWin32ExitCode = 0; 
        ServiceStatus.dwCurrentState = SERVICE_STOPPED; 
        SetServiceStatus (ServiceStatusHandle, &ServiceStatus);
        return; 
        
      default:
         break;
    } 
 
    // Report current status
    SetServiceStatus (ServiceStatusHandle, &ServiceStatus);
 
    return; 
}


/* Process main loop */
void  ServiceMain(int argc, char** argv) 
{ 
   int error; 
 
   ServiceStatus.dwServiceType = SERVICE_WIN32; 
   ServiceStatus.dwCurrentState =  SERVICE_START_PENDING; 
   ServiceStatus.dwControlsAccepted   =   SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
   ServiceStatus.dwWin32ExitCode = 0; 
   ServiceStatus.dwServiceSpecificExitCode = 0; 
   ServiceStatus.dwCheckPoint = 0; 
   ServiceStatus.dwWaitHint = 0; 
 
   ServiceStatusHandle = RegisterServiceCtrlHandler("stDataGather",(LPHANDLER_FUNCTION)ControlHandler); 
   if (ServiceStatusHandle == (SERVICE_STATUS_HANDLE)0) 
   { 
      // Registering Control Handler failed
      return; 
   }  

   // Initialize Service 
   error = InitService(); 
   if (error < 0) 
   {
      // Initialization failed
      ServiceStatus.dwCurrentState =  SERVICE_STOPPED; 
      ServiceStatus.dwWin32ExitCode = -1; 
      SetServiceStatus(ServiceStatusHandle, &ServiceStatus); 
      return; 
   } 
   // We report the running status to SCM. 
   ServiceStatus.dwCurrentState = SERVICE_RUNNING; 
   SetServiceStatus (ServiceStatusHandle, &ServiceStatus);
 
   /* keep accept new connect */
   while (ServiceStatus.dwCurrentState == SERVICE_RUNNING)
   {
	   MainLoop();
   }

   return; 
}

/* Start service dispatcher */
int ServiceStart()
{
	/* Start service dispatch */
	if (StartServiceCtrlDispatcher(ServiceDispatchTable) == FALSE) {
		TextLog("warning","Cannot start service dispatcher");
		return -1;
	}
	return 0;
}



// data manage,settimer use less cpu resource
int MainLoop()
{
//	MSG Msg;
//	UINT TimerId = SetTimer(NULL, SERVICE_TIMER_ID, INTERVAL, &my_timer_proc);
//	
//	while(1)
//	{
//		//timer Loop
//		if(GetMessage(&Msg, NULL, 0, 0))
//		{
//			if(Msg.message == WM_TIMER)
//			{
//				TranslateMessage(&Msg); 
//				DispatchMessage(&Msg); 
//			}
//		}
//	}
//	
//	KillTimer(NULL,SERVICE_TIMER_ID);

	while(1)
	{
		if(task_len <= 0)
		{
			TextLog("warning","0 item task");
			return -1;
		}
			
		for(int i =0;i < task_len;i++)
		{
			if(ts[i]->timer <= 0)
			{
				//reset
				ts[i]->timer = ts[i]->schedule;
				if(strncmp(ts[i]->type,SSHTYPE,strlen(SSHTYPE)) == 0)
				{
						/* execute task*/
						TextLog("debug","ssh task");
					_beginthread(ssh_sw, 0, (LPVOID*)ts[i]);
					
				}
				else if(strncmp(ts[i]->type,CMDTYPE,strlen(CMDTYPE)) == 0)
				{
					/* execute task*/
					TextLog("debug","cmd task");
					_beginthread(exec, 0, (LPVOID*)ts[i]);
				}
				else
				{
					TextLog("error","invalid task type!");
				}
				
			}
			else
			{
				ts[i]->timer -=  INTERVAL;
			}
		}		
		
		
		//sleep
		Sleep(INTERVAL);
	}
	
	return -1;
}


void CALLBACK my_timer_proc(HWND hWnd, UINT nMsg, UINT nIDEvent, DWORD dwTime) 
{
	char temp[1024];
	sprintf(temp,"nMsg:%d nIDEvent: %d Time: %d\n" ,nMsg,nIDEvent,dwTime);
	TextLog("debug",temp);
	
	if(task_len <= 0)
	{
		TextLog("warning","0 item task");
		return ;
	}
	
	
	for(int i =0;i < task_len;i++)
	{
		if(ts[i]->timer <= 0)
		{
			//reset
			ts[i]->timer = ts[i]->schedule;
			if(strncmp(ts[i]->type,SSHTYPE,strlen(SSHTYPE)) == 0)
			{
					/* execute task*/
				_beginthread(ssh_sw, 0, (LPVOID*)ts[i]);
				
			}
			else if(strncmp(ts[i]->type,CMDTYPE,strlen(CMDTYPE)) == 0)
			{
				/* execute task*/
				_beginthread(exec, 0, (LPVOID*)ts[i]);
			}
			else
			{
				TextLog("error","invalid task type!");
			}
			
		}
		else
		{
			ts[i]->timer -=  INTERVAL;
		}
	}		
}


