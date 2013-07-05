/****************************************************************************

    PROGRAM:   shantou_cloud_storage

    FILE:      loadtask.cpp

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
#include <string.h>

#include "global.h"
#include "loadtask.h"
#include "log.h"

#ifndef  MAX_TASK
	#define MAX_TASK 5000  
#endif

#ifndef MAX_BUFF_SIZE
	#define MAX_BUFF_SIZE  1024*10
#endif
#define GET_ARRAY_LEN(array,len) {len = (sizeof(array) / sizeof(array[0]));}

#ifndef MAX_ELEMENT
	#define MAX_ELEMENT 20
#endif

#ifndef SSHTYPE
	#define SSHTYPE "ssh_task"
#endif

#ifndef CMDTYPE
	#define CMDTYPE "cmd_task"
#endif
/*
argument: file_path-> the task descript file
return; if success return the number of line, -1 if fail
*/
int load_task(char* file_path,char** task_list)
{
	FILE* config;
	char LineBuff[MAX_BUFF_SIZE + 1];
	char* value = NULL;
	int len;
	

	int valid_rows = 0;
	
	config = fopen(file_path,"r");
	if(config != NULL)
	{
		
		while(fgets(LineBuff,MAX_BUFF_SIZE,config) != NULL)
		{
			if (valid_rows > MAX_TASK)
			{
				fclose(config);
				return MAX_TASK;
			}

			len = LineParser1(LineBuff);
			if( len > 0)
			{
				task_list[valid_rows] = (char*)malloc(len);
				memset(task_list[valid_rows],0,len);
				strncpy(task_list[valid_rows],LineBuff,len);
				*(task_list[valid_rows]+len) = '\0';
				valid_rows++;
			}
			else
			{
				continue;
			}
		}

		fclose(config);
	}
	else
	{
		return -1;
	}

	return valid_rows;
}

/*
desc: ignore blank and comment lines
return: line's character numbers

*/
int LineParser1(char * line)
{
	int len = 0;

	if(line == NULL)
	{
		return -1;
	}

	while(*line == ' ')
	{
		line++;
		if(*line == '#')
		{
			return -1;
		}

		if(*line == '\n')
		{
			return -1;
		}

		if(*line == '\r')
		{
			return -1;
		}
	}

	if(*line=='#')
	{
		return -1;
	}

	if(*line == '\n')
	{
		return -1;
	}

	if(*line == '\r')
	{
		return -1;
	}

	
	for(int i =0;*line++ != '\n'; i++  )
	{
		if(i >= MAX_BUFF_SIZE)
		{
			return MAX_BUFF_SIZE;
		}
		
		len++;
	}
	return len;
}


void split( char * arr[], char * str, const char * del)
{
    char * s = strtok(str, del); 
    while (s != NULL) 
	{
		*arr ++   = s;
		s = strtok(NULL, del);
    }
}

/*
desc:	add schedule task to task struct list
param: task point,char** 
return: 1 if ok,0 if invalid
*/

int add_task(task * task_element,char* task_array[])
{
	if(strncmp(task_array[0],CMDTYPE,strlen(CMDTYPE)) == 0 )
	{
		// memory malloc 
		task_element->type = (char*)malloc(sizeof(char) * strlen(task_array[0])+1 );
		task_element->cmd  = (char*)malloc(sizeof(char) *strlen(task_array[4])+1 );
		task_element->ipaddress = (char*)malloc(sizeof(char) * strlen(task_array[3]) + 1);
		task_element->user = (char*)malloc(sizeof(char) + 1);
		task_element->passwd = (char*)malloc(sizeof(char) + 1);
		task_element->path  = (char*)malloc(sizeof(char)* strlen(task_array[5])+ 1);
		
		memset(task_element->type,0,sizeof(char) * strlen(task_array[0])+1);
		memset(task_element->cmd,0,sizeof(char) * strlen(task_array[3])+1);
		memset(task_element->ipaddress,0,sizeof(char) * strlen(task_array[3]) + 1);
		memset(task_element->user,0,sizeof(char) +1);
		memset(task_element->passwd,0,sizeof(char) +1);
		memset(task_element->path,0,sizeof(char)* strlen(task_array[5])+ 1);
		
		*task_element->user = ' ';
		*task_element->passwd = ' ';

		strcpy(task_element->type,task_array[0]);
		task_element->schedule = atoi(task_array[1]);
		task_element->timeout = atoi(task_array[2]);
		strcpy(task_element->ipaddress,task_array[3]);
		strcpy(task_element->cmd, task_array[4]);
		strcpy(task_element->path, task_array[5]);
		task_element->timer = -1;
		return 1;
	}
	else if(strncmp(task_array[0],SSHTYPE,strlen(SSHTYPE)) == 0)
	{
		// memory malloc 
		task_element->type = (char*)malloc(sizeof(char) * strlen(task_array[0]) +1);
		task_element->ipaddress = (char*)malloc(sizeof(char) * strlen(task_array[3]) + 1);
		task_element->user = (char*)malloc(sizeof(char) * strlen(task_array[4]) + 1);
		task_element->passwd = (char*)malloc(sizeof(char) * strlen(task_array[5]) + 1);
		task_element->cmd  = (char*)malloc(sizeof(char) * strlen(task_array[7]) + 1);
		task_element->path  = (char*)malloc(sizeof(char) * strlen(task_array[8]) + 1);

		memset(task_element->type,0,sizeof(char) * strlen(task_array[0]) + 1);
		memset(task_element->ipaddress,0,sizeof(char) * strlen(task_array[3]) + 1);
		memset(task_element->user,0,sizeof(char) * strlen(task_array[4]) + 1);
		memset(task_element->passwd,0,sizeof(char) * strlen(task_array[5]) + 1);
		memset(task_element->cmd,0,sizeof(char) * strlen(task_array[7]) + 1);
		memset(task_element->path,0,sizeof(char) * strlen(task_array[8]) + 1);

		strcpy(task_element->type,task_array[0]);
		task_element->schedule = atoi(task_array[1]);
		task_element->timeout = atoi(task_array[2]);
		strcpy(task_element->ipaddress,task_array[3]);
		strcpy(task_element->user,task_array[4]);
		strcpy(task_element->passwd,task_array[5]);
		task_element->port = atoi(task_array[6]);
		strcpy(task_element->cmd, task_array[7]);
		strcpy(task_element->path, task_array[8]);
		task_element->timer = -1;
		return 1;
	}
	else
	{
		return 0;
	}

}

