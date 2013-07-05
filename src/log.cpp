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
#include <stdio.h>
#include <time.h>

#include "global.h"
#include "log.h"


/* micro define */
//#define LOGFILE "d:\\codeDemo\\c\\shantou_cloud_storage.log"



//log
void TextLog(char* priority,char* msg)
{
	/* file handle for log file */
	FILE* file;
	
	/* timestamp */
	time_t rawtime;
	struct tm * timeinfo;

	time (&rawtime);
	timeinfo = localtime ( &rawtime );


	file = fopen(logFilePath,"a+");
	if(file != NULL)
	{
		fprintf(file, "%4d%02d%02d%02d%02d%02d    ",1900+timeinfo->tm_year, 1+timeinfo->tm_mon,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
		fprintf(file, "%s    %s\n", priority,msg);
		fflush(file);
	  fclose(file);
	}
	return ;
}