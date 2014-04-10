
#include "resource.h"


LPSTR backup;
DWORD backupsize;
char foldername[MAX_PATH]; 
BOOL statusarea;
BOOL statusbar;
BOOL OrigDataWindow;

char cmdlinearg_path[MAX_PATH];
BOOL cmdlinearg_path_set;

HWND hStatus;
HWND hTool;
HWND hEdit;
HWND hOpenFolderButton;
HWND hRevertButton;
HWND hOpenButton;
HWND hOriginalDataWindow;

char g_filepath[MAX_PATH];
char g_folderpath[MAX_PATH];
char g_filename[MAX_PATH];

BOOL fileopen;
BOOL filedirty;

UINT dirtycount;

VOID TimerReset()
{
    dirtycount = 0;
}

VOID Revert(HWND hwnd)
{
    if (MessageBox(hwnd,
                   "Are you sure you want revert all of your changes?", 
                   "Message", 
                   MB_YESNO) == IDYES)
    {
        if (fileopen) {
            SetWindowText(hEdit, backup);
            SaveOpenedFile(hwnd);
        }
    }
}

VOID OpenFolder(HWND hwnd)
{
    ShellExecute(hwnd, "open", NULL, NULL, g_folderpath, SW_SHOWNORMAL);
}

VOID MouseScroll(HWND hwnd, WPARAM wParam)
{
    if (LOWORD(wParam) == MK_CONTROL) {
        SetFont(hwnd, HIWORD(wParam) / WHEEL_DELTA);
    }
    else
        SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)"ProTip: hold control while you scroll ;)");
}

VOID FullRedraw(HWND hwnd)
{
    //TODO this is unnecessary

    RECT rc;
    GetClientRect(hwnd, &rc);
    InvalidateRect(hwnd, &rc, TRUE);
    DoWM_SIZE(hwnd);
    Do_WM_PAINT(hwnd);
}

int fontsize;
VOID SetFont(HWND hwnd, int step)
{
    //fix me
    if (step == 545) step = -1;

    int prevfontsize = fontsize;

    if (step == 0) { //condition when setting the initial font
        prevfontsize = 0;
        fontsize = DEFFONTSIZE;
    }
    else
        fontsize = min(max(fontsize + step, MINFONTSIZE), MAXFONTSIZE);

    if (prevfontsize != fontsize)
    {
        //broadcast change
        char buf[100];
        sprintf_s(buf, "Setting font size to %d", fontsize);
        SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)buf);

        //send WM_SETFONT
        HFONT hf = CreateFont(fontsize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Times New Roman");
        SendMessage(hEdit, WM_SETFONT, (WPARAM)hf, MAKELPARAM(TRUE, 0));
        SendMessage(hwnd, WM_SETFONT, (WPARAM)hf, MAKELPARAM(TRUE, 0));

    }
}

VOID SaveFileName(HWND hwnd, LPCTSTR FilePath)
{
    //save the file name to our global string
    memcpy(g_filepath, FilePath, strlen(FilePath));

    //save folder path in 'shell executable' format
    ZeroMemory(g_folderpath, MAX_PATH);
    int srcptr = 0;
    int dstptr = 0;
    int filenameptr = strlen(g_filepath) - 1;
    while (g_filepath[filenameptr] != '\\') { filenameptr--; }
    while (srcptr < filenameptr) {
        if (g_filepath[srcptr] == '\\')
            g_folderpath[dstptr++] = '\\';
        g_folderpath[dstptr++] = g_filepath[srcptr++];
    }
    
    //copy just the file name
    memcpy(g_filename, g_filepath + filenameptr + 1, strlen(g_filepath + filenameptr + 1));

    

    //update the window title to inlude opend file name
    int size = strlen(FilePath) + 10;
    char* wintitle = (LPSTR)GlobalAlloc(GPTR, size);
    sprintf(wintitle, "SAVEpad: %s", g_filename);
    SetWindowText(hwnd, wintitle);
    GlobalFree(wintitle);


}

BOOL PromptForFilePath(HWND hwnd, LPSTR buf)
{
    OPENFILENAME ofn;

    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = buf;
    ofn.nMaxFile = MAX_PATH;

    ofn.Flags = OFN_EXPLORER |
        OFN_CREATEPROMPT |
        OFN_HIDEREADONLY;

    ofn.lpstrDefExt = "txt";

    if (!GetOpenFileName(&ofn)) {
        SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)"Error, prompt for file failed.");
        return FALSE;
    }
    return TRUE;

}


/*
 * Loading Files
*/

BOOL LoadTextFile(HWND hwnd, HWND hEdit, LPCTSTR FilePath)
{
    HANDLE hFile;
    BOOL bSuccess = FALSE;

    //First assume file exists and try to open it
    hFile = CreateFile(FilePath,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    //Failure here means we should assume file needs to be created
    if (hFile == INVALID_HANDLE_VALUE)
    {
        hFile = CreateFile(FilePath,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            CREATE_NEW, //new file
            0,
            NULL);

        if (hFile == INVALID_HANDLE_VALUE) {
            SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)"CreateFile Failed, is it a new file?");
            return FALSE;
        }

        //broadcast file created
        char buf[2 * MAX_PATH] = { 0 };
        sprintf_s(buf, "File Created: %s", FilePath);
        SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)buf);
    }
    else {

        //broadcast file opened
        char buf[2 * MAX_PATH] = { 0 };
        sprintf_s(buf, "File Opened: %s", FilePath);
        SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)buf);
    }

    //backupsize and backup are globals

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

                    if (!SetWindowText(hOriginalDataWindow, backup)) {
                        MessageBox(NULL, "Saving text to hOriginalDataWindow failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
                    }

                }
            }

        }
    }

    CloseHandle(hFile);
    
    SaveFileName(hwnd, FilePath);

    fileopen = TRUE;

    if (!bSuccess) {
        //broadcast failure
        char buf[2 * MAX_PATH] = { 0 };
        sprintf_s(buf, "LoadTextFile failed. passed [%s]", FilePath);
        SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)buf);
    }
    else {
        //boradcast success
        char msg[100] = "";
        sprintf_s(msg, "Loaded File: %s", FilePath);
        SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)msg);
    }

    
    FullRedraw(hwnd);

    return bSuccess;
}

VOID PromptAndLoadFile(HWND hwnd) 
{
    char filePath[MAX_PATH] = "";
    if (PromptForFilePath(hwnd, filePath)) {
        LoadTextFile(hwnd, hEdit, filePath);
    }
}








/*
* Saving Files
*/


BOOL SaveTextFile(HWND hEdit, LPCTSTR FilePath)
{
    HANDLE hFile;
    BOOL bSuccess = FALSE;

    hFile = CreateFile(FilePath,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        char buf[2 * MAX_PATH] = { 0 };
        sprintf_s(buf, "Error: SaveTextFile failed to open file [%s]", FilePath);
        SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)buf);
        return FALSE;
    }


    DWORD filesize = GetWindowTextLength(hEdit) + 1;
    LPSTR buf = (LPSTR)GlobalAlloc(GPTR, filesize);
    if (buf == NULL) {
        SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)"SaveTextFile failed, couldnt allocate memory.");
        return FALSE;
    }

    GetWindowText(hEdit, buf, filesize);
    DWORD tmp;
    if (!WriteFile(hFile, buf, filesize, &tmp, NULL)) {
        SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)"SaveTextFile, WriteFile failed.");
        bSuccess = FALSE;
    }

    GlobalFree(buf);

    CloseHandle(hFile);

    SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)CleanMsg);
    
    filedirty = FALSE;

    dirtycount = 0;

    return bSuccess;

}

BOOL SaveOpenedFile(HWND hwnd)
{
    if (!fileopen) return FALSE;
    return SaveTextFile(hEdit, g_filepath);
}

VOID EmergenecySave(HWND hwnd)
{
    if (SaveOpenedFile(hwnd))
        return;

    if (!fileopen && GetWindowTextLength(hEdit) > 0) {
        //TODO: should warn user that they are losing data
    }
}
