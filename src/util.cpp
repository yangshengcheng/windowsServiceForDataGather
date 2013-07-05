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
#include <windows.h>
#include <stdio.h>
#include "global.h"
#include "util.h"

/* free a pointer array */
void free_parray(char* pa[],int len)
{
	for(int i = 0;i < len;i++)
	{
		if(pa[i] != NULL)
		{
			free(pa[i]);
			pa[i] = (char*)NULL;
		}
	}
	return;
}

/* free a task struct array */
void free_task(task* ts[],int len)
{
	return;
}
