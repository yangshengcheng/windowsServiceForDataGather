/****************************************************************************

    PROGRAM:   shantou_cloud_storage

    FILE:      getopt.cpp

    PURPOSE:   gather performence and configure data from storages and switchs at schedule time 

    DESCRIPTION:
    					1) install a windows service name "stDataGather"
							2) gather performence and configure data from storages and switchs at schedule time,and transfer data to data manager server

    CREATE:   20130425
    AUTHOR:   yangshengcheng@gzcss.net
    PLATFORM: windows7/windows2003

****************************************************************************/
#include <string.h>
#include <stdio.h>

#include "global.h"
#include "getopt.h"


/* micro define */
#define MAXLINESIZE 1024

/* sorry for hard code,by yangshengcheng@gzcss.net */
//#define CONFIG "C:\\Windows\\System32\\stDataGather.conf"

/* follow prevent hard code ,windows service process's default dir is system32 */
#define CONFIG "stDataGather.conf"


/************************************************************************

    FUNCTION:  ReadConfig

    PURPOSE:   read configure from config file, and return the assign special argument 

    INPUTS:
     path - configure file,include cacti server 's ip and port etc.  

    RETURNS:
         return 0 value if ok, return -1 if error  

*************************************************************************/
int ReadConfig(char* key,char* dest)
{
	FILE* config;
	unsigned int i;
	char LineBuff[MAXLINESIZE + 1];
	char* value = NULL;

	config = fopen(CONFIG,"r");
	if(config != NULL)
	{
		while(fgets(LineBuff,MAXLINESIZE,config) != NULL)
		{

			value = LineParser(key,LineBuff);
			if( value != NULL)
			{
				break;
			}
		}

		fclose(config);
	}
	else
	{
		return -1;
	}

	if(value == NULL)
	{
		return -1;
	}

	for(i=0;i < strlen(value) - 1; i++)
	{
		dest[i] = value[i];
	}
	
	/* add string end mark*/
	dest[strlen(value) - 1] = '\0';

	return 0;

}

/************************************************************************

    FUNCTION:  LineParser

    PURPOSE:  parser line  

    INPUTS:
     key - the key word to check,  LineBuff 

    RETURNS:
         return NOT NULL value if ok, return NULL if error  
*************************************************************************/
char * LineParser(char * key ,char * line)
{
	if(strstr(line,key)== NULL)
	{
		return NULL;
	}

	while(*line != '=')
	{
		line++;
	}

	line++;

	return line;

}