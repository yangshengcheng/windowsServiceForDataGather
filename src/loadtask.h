/****************************************************************************

    PROGRAM:   shantou_cloud_storage

    FILE:      loadtask.h

    PURPOSE:   gather performence and configure data from storages and switchs at schedule time 

    DESCRIPTION:
    					1) install a windows service name "stDataGather"
							2) gather performence and configure data from storages and switchs at schedule time,and transfer data to data manager server

    CREATE:   20130425
    AUTHOR:   yangshengcheng@gzcss.net
    PLATFORM: windows7/windows2003

****************************************************************************/
#ifndef MAX_TASK
	#define MAX_TASK 5000
#endif


extern task *ts[MAX_TASK];
extern char *taskList[MAX_TASK];
int load_task(char* file_path,char* task_list[]);
int LineParser1(char * line);
void split( char * arr[], char * str, const char * del);
int add_task(task * task_element,char* task_array[]);

