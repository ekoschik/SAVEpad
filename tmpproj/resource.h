

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdio.h>
#include <string.h>


//Constants
const char g_szClassName[]      = "myWindowClass";
const char CleanMsg[]           = "clean";
const char DirtyMsg[]           = "dirty";
const UINT TIMERGRANULARITY     = 15;
const UINT DIRTYTHRESHOLD       = 15;
const int ID_TIMER              = 1;
const int DEFFONTSIZE           = 20;
const int MAXFONTSIZE           = 100;
const int MINFONTSIZE           = 10;
const int NOFILE_BOARDER_SIZE   = 10;
const int STATUSAREAHEIGHT      = 70;

//Globals
extern HWND hStatus;
extern HWND hTool;
extern HWND hEdit;
extern BOOL statusarea;
extern BOOL fileopen;
extern BOOL filedirty;
extern UINT dirtycount;
extern LPSTR backup;
extern DWORD backupsize;
extern char filename[MAX_PATH];
extern char cmdlinearg_path[MAX_PATH];
extern BOOL cmdlinearg_path_set;


//Located in HelperRountines.cpp:
VOID SetFont(HWND hwnd, int step);
void DoFileSave(HWND hwnd, BOOL forcesave);
void DoFileOpen(HWND hwnd, LPSTR filepath);
VOID Revert(HWND hwnd);
VOID TimerTick(HWND hwnd);
VOID MouseScroll(HWND hwnd, WPARAM wParam);
VOID TimerReset();

//Located in Source.cpp:
VOID DoWM_SIZE(HWND hwnd);




#define IDI_ICON1                       10002
#define IDR_MENU1                       103
#define ID_FILE_OPEN                    40001
#define ID_FILE_SAVE                    40002
#define ID_FILE_EXIT                    40003
#define ID_FILE_REVERT                  40004
#define ID_FILE_EXITWITHOUTSAVING       40005

#define M_SELECTALL                     30303


#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_NEXT_RESOURCE_VALUE        107
#define _APS_NEXT_COMMAND_VALUE         40006
#define _APS_NEXT_CONTROL_VALUE         1001
#define _APS_NEXT_SYMED_VALUE           101
#endif
#endif

#define IDC_MAIN_EDIT   101
#define IDC_MAIN_STATUS	103


