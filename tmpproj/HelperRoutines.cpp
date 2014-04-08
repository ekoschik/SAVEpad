
#include "resource.h"


LPSTR backup;
DWORD backupsize;
char foldername[MAX_PATH]; 
BOOL statusarea;


char cmdlinearg_path[MAX_PATH];
BOOL cmdlinearg_path_set;

HWND hStatus;
HWND hTool;
HWND hEdit;

char filename[MAX_PATH];
BOOL fileopen;
BOOL filedirty;

UINT dirtycount;

VOID TimerTick(HWND hwnd)
{
    if (++dirtycount >= DIRTYTHRESHOLD)
    {
        DoFileSave(hwnd, FALSE);
        dirtycount = 0;
    }
}
VOID TimerReset()
{
    dirtycount = 0;
}

VOID MouseScroll(HWND hwnd, WPARAM wParam)
{
    if (LOWORD(wParam) == MK_CONTROL) {
        SetFont(hwnd, HIWORD(wParam) / WHEEL_DELTA);
    }
    else
        SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)"ProTip: hold control while you scroll ;)");
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


    //create folder name
    //BEFORE "C:\Users\evkoschi\Desktop\tmp.txt"
    //AFTR   "C:\\Users\\evkoschi\\Desktop"
    //ex: double backslashed, and take of everything after and including last slash

    //to open folder:
    //    ShellExecute(hwnd, "open", NULL, NULL, **folder name**, SW_SHOWNORMAL);


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

    DoWM_SIZE(hwnd);
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