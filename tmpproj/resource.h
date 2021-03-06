

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
const int STATUSAREAHEIGHT      = 100;

const int DEFWINDOWWIDITH= 500;
const int DEFWINDOWHEIGHT = 700;

extern char cmdlinearg_path[MAX_PATH];
extern BOOL cmdlinearg_path_set;

//Globals
extern HWND hStatus;
extern HWND hTool;
extern HWND hEdit;
extern HWND hOpenFolderButton;
extern HWND hOpenButton;
extern HWND hRevertButton;
extern HWND hOriginalDataWindow;



extern BOOL statusarea;
extern BOOL statusbar;
extern BOOL fileopen;
extern BOOL filedirty;
extern BOOL OrigDataWindow;

extern UINT dirtycount;
extern LPSTR backup;
extern DWORD backupsize;
extern char g_filepath[MAX_PATH];
extern char g_folderpath[MAX_PATH];
extern char g_filename[MAX_PATH];



//Located in HelperRountines.cpp:
VOID SetFont(HWND hwnd, int step);
BOOL SaveTextFile(HWND hEdit, LPCTSTR FilePath);
BOOL LoadTextFile(HWND hwnd, HWND hEdit, LPCTSTR pszFileName);
VOID Revert(HWND hwnd);
VOID TimerTick(HWND hwnd);
VOID MouseScroll(HWND hwnd, WPARAM wParam);
VOID TimerReset();
VOID PromptAndLoadFile(HWND hwnd); 
VOID EmergenecySave(HWND hwnd);
BOOL SaveOpenedFile(HWND hwnd);
VOID OpenFolder(HWND hwnd);
VOID FullRedraw(HWND hwnd);


//Located in Source.cpp:
VOID DoWM_SIZE(HWND hwnd);
VOID Do_WM_PAINT(HWND hwnd);



#define IDI_ICON1                       10002
#define IDR_MENU1                       101
#define ID_VIEW_HEADERAREA              40001
#define ID_VIEW_STATUSBAR               40002
#define ID_VIEW_ORIGINALFILEDATA        40003
#define ID_FILE_EXIT                    40004
#define M_SELECTALL                     30303

#define BTN_OPENFOLDER                  30301
#define BTN_REVERT                      30302
#define BTN_OPENFILE                    30302




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


