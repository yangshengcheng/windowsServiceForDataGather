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

/** log function **/
void TextLog(char* priority,char* msg);
extern char* logFilePath;
