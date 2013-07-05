/****************************************************************************

    PROGRAM:   shantou_cloud_storage

    FILE:      sock.h

    PURPOSE:   gather performence and configure data from storages and switchs at schedule time 

    DESCRIPTION:
    					1) install a windows service name "stDataGather"
							2) gather performence and configure data from storages and switchs at schedule time,and transfer data to data manager server

    CREATE:   20130425
    AUTHOR:   yangshengcheng@gzcss.net
    PLATFORM: windows7/windows2003

****************************************************************************/

/** socket function and  global variables**/
int WSockStart(void);
void WSockStop(void);
int WSockOpen(char * loghost, int port);
int WSockListen();
void WSockClose(void);
int WSockSend(char * message);





