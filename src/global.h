/****************************************************************************

    PROGRAM:   shantou_cloud_storage

    FILE:      global.h

    PURPOSE:   gather performence and configure data from storages and switchs at schedule time 

    DESCRIPTION:
    					1) install a windows service name "stDataGather"
							2) gather performence and configure data from storages and switchs at schedule time,and transfer data to data manager server

    CREATE:   20130425
    AUTHOR:   yangshengcheng@gzcss.net
    PLATFORM: windows7/windows2003

****************************************************************************/

/***** mcro define *****/

/* buff size define */
#ifndef MAX_BUFF_SIZE
	#define MAX_BUFF_SIZE 1024 * 10
#endif

#define MAX_TASK 5000
#define MAX_ELEMENT 20

/* timer */
#define SERVICE_TIMER_ID 0
#define INTERVAL 1000*60

/* task type */
#define SSHTYPE "ssh_task"
#define CMDTYPE "cmd_task"

 /* task list */
 struct task
{
	char *type;
	char *cmd;
	int schedule;
	int timer;
	int timeout;
	char *ipaddress;
	char *user;
	char *passwd;
	int port;
	char *path;
} ;

