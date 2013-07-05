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


/********* Function prototypes *********/

/* load configure*/
int ReadConfig(char* key,char* dest);
char * LineParser(char * key ,char * line);

/* load task data */
int load_task(char* file,char* task_list[]);
char* LineParser(char* line);
void split( char * arr[], char * str, const char * del);
int add_task(task * task_element,char* task_array[]);



/* loop */
int MainLoop();

/* receive dominostat data  */
int ServiceStart();
int InitService();

void ServiceMain(int argc, char** argv);
void  ControlHandler(DWORD request);


/* the thread for data process */
void __cdecl ssh_sw(LPVOID lpParam);
void __cdecl exec(LPVOID lpParam);

/** log function **/
void TextLog(char* priority,char* msg);

/* util */
void free_parray(char* pa[],int len);
void free_task(task* ts[],int len);

/* timer */
void CALLBACK my_timer_proc(HWND hWnd, UINT nMsg, UINT nIDEvent, DWORD dwTime);





