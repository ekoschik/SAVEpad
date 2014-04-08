#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include "resource.h"
#include <stdio.h>
#include <string.h>
#define IDC_MAIN_EDIT   101
#define IDC_MAIN_STATUS	103


const char g_szClassName[] = "myWindowClass";
const int ID_TIMER = 1;

const char CleanMsg[] = "clean";
const char DirtyMsg[] = "dirty";

LPSTR backup;
DWORD backupsize;

char cmdlinearg_path[MAX_PATH];
BOOL cmdlinearg_path_set;

HWND hStatus;
HWND hTool;
HWND hEdit;

char filename[MAX_PATH];
BOOL fileopen;
BOOL filedirty;

UINT dirtycount;
const UINT dirtythreshold = 15;

void DoFileSave(HWND hwnd, BOOL forcesave);
void DoFileOpen(HWND hwnd, LPCTSTR szFileName);

const int deffontsize = 20;
const int max_fontsize = 100;
const int min_fontsize = 10;
int fontsize;
VOID SetFont(HWND hwnd, int step)
{
    //hacktastic.  fix me
    if (step == 545) step = -1;

    //change font size
    int prevfontsize = fontsize;
    fontsize = min(max(fontsize + step, min_fontsize), max_fontsize);
    
    if (prevfontsize != fontsize) 
    {
        //broadcast change
        char buf[100];
        sprintf_s(buf, "Setting font size to %d", fontsize);
        SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)buf);

        //send WM_SETFONT
        HFONT hf = CreateFont(fontsize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Times New Roman");
        SendMessage(hEdit, WM_SETFONT, (WPARAM)hf, MAKELPARAM(TRUE, 0));
    }
}

VOID Revert(HWND hwnd)
{
    SetWindowText(hEdit, backup);
    DoFileSave(hwnd, TRUE);
}

VOID SaveFileName(HWND hwnd, LPCTSTR szFileName)
{
    //save the file name to our global string
    memcpy(filename, szFileName, strlen(szFileName));
    fileopen = TRUE;

    //update the window title to inlude opend file name
    int size = strlen(szFileName) + 10;
    char* wintitle = (LPSTR)GlobalAlloc(GPTR, size);
    sprintf(wintitle, "SAVEpad: %s", filename);
    SetWindowText(hwnd, wintitle);
    GlobalFree(wintitle);

}

BOOL LoadTextFile(HWND hEdit, LPCTSTR pszFileName)
{
    HANDLE hFile;
    BOOL bSuccess = FALSE;

    hFile = CreateFile(pszFileName, 
                       GENERIC_READ, 
                       FILE_SHARE_READ, 
                       NULL,
                       OPEN_EXISTING, 
                       0, 
                       NULL);

    if (hFile != INVALID_HANDLE_VALUE)
    {
        backupsize = GetFileSize(hFile, NULL);
        if (backupsize != 0xFFFFFFFF)
        {
            
            backup = (LPSTR)GlobalAlloc(GPTR, backupsize + 1);
            if (backup != NULL)
            {
                DWORD dwRead;

                if (ReadFile(hFile, backup, backupsize, &dwRead, NULL))
                {
                    // Add null terminator
                    backup[backupsize] = 0;

                    if (SetWindowText(hEdit, backup)) {
                        // It worked!
                        bSuccess = TRUE; 
                    }
                }
                
            }
        }
        CloseHandle(hFile);
    }
    return bSuccess;
}

void DoFileOpen(HWND hwnd, LPSTR filepath)
{
    if (fileopen) DoFileSave(hwnd, TRUE);

    if (filepath == NULL)
    {
        OPENFILENAME ofn;
        char szFileName[MAX_PATH] = "";

        ZeroMemory(&ofn, sizeof(ofn));

        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = hwnd;
        ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
        ofn.lpstrFile = szFileName;
        ofn.nMaxFile = MAX_PATH;

        ofn.Flags = OFN_EXPLORER |
            OFN_CREATEPROMPT |
            OFN_HIDEREADONLY;

        ofn.lpstrDefExt = "txt";

        if (GetOpenFileName(&ofn))
        {
            filepath = szFileName;
        }

    }
    

    if (filepath != NULL) {
        if (!LoadTextFile(hEdit, filepath)) {
            SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)"Created New File.");
            SetWindowText(hEdit, "");
        }
        else{
            char buf[2 * MAX_PATH] = { 0 };
            sprintf_s(buf, "File Opened: %s", filepath);
            SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)buf);
        }
        SaveFileName(hwnd, filepath);
    }

}

BOOL SaveTextFile(HWND hEdit, LPCTSTR pszFileName)
{
    HANDLE hFile;
    BOOL bSuccess = FALSE;

    hFile = CreateFile(pszFileName, 
                       GENERIC_WRITE, 
                       0, 
                       NULL,
                       CREATE_ALWAYS, 
                       FILE_ATTRIBUTE_NORMAL, 
                       NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        DWORD dwTextLength;

        dwTextLength = GetWindowTextLength(hEdit);
        
        LPSTR pszText;
        DWORD dwBufferSize = dwTextLength + 1;

        pszText = (LPSTR)GlobalAlloc(GPTR, dwBufferSize);
        if (pszText != NULL)
        {
            GetWindowText(hEdit, pszText, dwBufferSize);
            
            DWORD dwWritten;

            WriteFile(hFile, pszText, dwTextLength, &dwWritten, NULL);
            bSuccess = TRUE;
            
            GlobalFree(pszText);
        }
        
        CloseHandle(hFile);
    }
    return bSuccess;
}

void DoFileSave(HWND hwnd, BOOL forcesave)
{
    if (fileopen && !filedirty)
        return;


    BOOL success = FALSE;
    if (fileopen) 
    {
        //We have a file opened, assuming that we are saving the same file
        if (SaveTextFile(hEdit, filename))
            success = TRUE;
    }
    else if (forcesave){

        //Prompt user to pick a save location

        OPENFILENAME ofn;
        char szFileName[MAX_PATH] = "";

        ZeroMemory(&ofn, sizeof(ofn));

        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = hwnd;
        ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
        ofn.lpstrFile = szFileName;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrDefExt = "txt";
        ofn.Flags = OFN_EXPLORER | 
                    OFN_PATHMUSTEXIST | 
                    OFN_HIDEREADONLY |
                    OFN_OVERWRITEPROMPT;

        if (GetSaveFileName(&ofn) && SaveTextFile(hEdit, szFileName)) {
            success = TRUE;
            SaveFileName(hwnd, szFileName);
        }

    }

    if (success) {
        SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)CleanMsg);
        filedirty = FALSE;
        dirtycount = 0;
    }
    else if (forcesave || fileopen)
        MessageBox(hwnd, "File not saved!", "Error", MB_OK | MB_ICONERROR);
}

LRESULT CALLBACK WndProc(HWND hwnd,
                         UINT msg,
                         WPARAM wParam,
                         LPARAM lParam)
{
    switch (msg)
    {
    
    case WM_CREATE:
    {
        SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(NULL, MAKEINTRESOURCE(IDI_ICON1)));
        
        backup = NULL;
        backupsize = 0;

        //Create Edit Control
         hEdit = CreateWindowEx(
                    WS_EX_CLIENTEDGE, 
                    "EDIT", "", 
                    WS_CHILD |
                        WS_VISIBLE | 
                        WS_VSCROLL | 
                        WS_HSCROLL | 
                        ES_MULTILINE | 
                        ES_AUTOVSCROLL | 
                        ES_AUTOHSCROLL, 
                    0, 0, 100, 100, 
                    hwnd, 
                    (HMENU)IDC_MAIN_EDIT, 
                    GetModuleHandle(NULL),
                    NULL);

        if(hEdit == NULL)
            MessageBox(hwnd, "Could not create edit box.", "Error", MB_OK | MB_ICONERROR);
    
        SetFont(hwnd, deffontsize);
        
        // Create Status bar
        hStatus = CreateWindowEx(
                        0, 
                        STATUSCLASSNAME, 
                        NULL,
                        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 
                        0, 0, 0, 0,
                        hwnd, 
                        (HMENU)IDC_MAIN_STATUS, 
                        GetModuleHandle(NULL), 
                        NULL);

        SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)"I'm a status bar!");
        
        if (cmdlinearg_path_set) {

            //when opening a txt file using savepad, windows adds quotes
            if (cmdlinearg_path[0] == '"') {
                int len = strlen(cmdlinearg_path);
                memcpy(cmdlinearg_path, cmdlinearg_path + 1, len - 2);
                cmdlinearg_path[len - 2] = 0;
            }

            DoFileOpen(hwnd, (LPSTR)cmdlinearg_path);
        }
        else
        {
            DoFileOpen(hwnd, (LPSTR)0);
        }

        break; 
    }

    case WM_SIZE:
    {
        //Broadcast New Window Size
        RECT rcWindow;
        GetWindowRect(hwnd, &rcWindow);
        char windowSizeBroadcast[200];
        sprintf(windowSizeBroadcast, "New Window Rect: [ %d, %d, %d, %d ]",
            rcWindow.top, rcWindow.left, rcWindow.right, rcWindow.bottom);
        SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)windowSizeBroadcast);

        // Size status bar and get height
        RECT rcStatus;
        int iStatusHeight;
        SendMessage(hStatus, WM_SIZE, 0, 0);
        GetWindowRect(hStatus, &rcStatus);
        iStatusHeight = rcStatus.bottom - rcStatus.top;

        // Calculate remaining height and size edit
        int iEditHeight;
        RECT rcClient;
        GetClientRect(hwnd, &rcClient);
        iEditHeight = rcClient.bottom - iStatusHeight;
        SetWindowPos(hEdit, NULL, 0, 0, rcClient.right, iEditHeight, SWP_NOZORDER);

        break;
    }

    case WM_COMMAND:

        switch (HIWORD(wParam)) {
        case EN_CHANGE:
            if (fileopen) {
                SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)DirtyMsg);
                filedirty = TRUE;
                
                dirtycount = 0; //reset the dirty clock
            }
            else {
                SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)"File not opened! (save or open new file).");
            }
            break;

        case M_SELECTALL:
        {
            UINT lSel = (LONG)SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0L);
            SendMessage(hEdit, EM_SETSEL, 0, lSel);
            SendMessage(hEdit, EM_SCROLLCARET, 0, 0);
            break;
        }
        }

        switch (LOWORD(wParam))
        {
        case ID_FILE_OPEN:
            DoFileOpen(hwnd, (LPSTR)0);
            break;
        case ID_FILE_EXIT:
            PostMessage(hwnd, WM_CLOSE, 0, 0);
            break;
        case ID_FILE_EXITWITHOUTSAVING:
            Revert(hwnd);
            PostMessage(hwnd, WM_CLOSE, 0, 0);
            break;
        case ID_FILE_REVERT:
            Revert(hwnd);
            break;

        case M_SELECTALL:
        {
            UINT lSel = (LONG)SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0L);
            SendMessage(hEdit, EM_SETSEL, 0, lSel);
            SendMessage(hEdit, EM_SCROLLCARET, 0, 0);
            break;
        }
        
        }
        break;

    case WM_MOUSEWHEEL:
        if (LOWORD(wParam) == MK_CONTROL) {
            SetFont(hwnd, HIWORD(wParam) / WHEEL_DELTA);
        }
        else
            SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)"ProTip: hold control while you scroll ;)");
        break;

    case WM_TIMER:
        if (++dirtycount >= dirtythreshold)
        {
            DoFileSave(hwnd, FALSE);
            dirtycount = 0;
        }
        break;
    
    case WM_CLOSE:
        if (GetWindowTextLength(hEdit) > 0)
            DoFileSave(hwnd, TRUE);
        GlobalFree(backup);
        DestroyWindow(hwnd);
        break;
    
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}


int WINAPI WinMain(HINSTANCE hInstance, 
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, 
                   int nCmdShow)
{
    //Read command line, and interpret anything as a file path to open
    ZeroMemory(cmdlinearg_path, MAX_PATH);
    cmdlinearg_path_set = FALSE;
    if (strcmp(lpCmdLine, ""))
    {
        sprintf_s(cmdlinearg_path, lpCmdLine);
        cmdlinearg_path_set = TRUE;
    }

    WNDCLASSEX wc = { 0 };
    MSG Msg;
    HWND hwnd;

    InitCommonControls();
    fileopen = FALSE;
    filedirty = FALSE;
    dirtycount = 0;

    wc.cbSize = sizeof(WNDCLASSEX); 
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
    wc.lpszClassName = g_szClassName;
    wc.hIconSm = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_ICON1));
    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    
    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,        //extended window style
        g_szClassName,           //ties the new window to the window class we just created
        "SAVEpad",               //Window Title
        WS_OVERLAPPEDWINDOW,     //window style
        CW_USEDEFAULT,           //start x
        CW_USEDEFAULT,           //start y
        800,                     //start width 
        700,                     //start height
        NULL,                    //Parent Window Handle
        NULL,                    //Menu Handle
        hInstance,               //This Application's Instance Handle
        NULL);                   //Window Creation Data

    if (hwnd == NULL) {
        MessageBox(NULL, "Window Creation Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    
    if (SetTimer(hwnd, ID_TIMER, 80, NULL) == 0)
        MessageBox(hwnd, "Could not SetTimer()!", "Error", MB_OK | MB_ICONEXCLAMATION);

    while (GetMessage(&Msg, NULL, 0, 0) > 0) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return Msg.wParam;
}

