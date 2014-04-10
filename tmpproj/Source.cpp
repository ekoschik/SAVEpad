
#include "resource.h"



VOID DoWM_CREATE(HWND hwnd)
{
    //Icons ;)
    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(NULL, MAKEINTRESOURCE(IDI_ICON1)));
    SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(NULL, MAKEINTRESOURCE(IDI_ICON1)));


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

    if (hStatus == NULL)
        MessageBox(hwnd, "Could not create edit box.", "Error", MB_OK | MB_ICONERROR);
    else
        SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)"I'm a status bar!");

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
        0, 0, 0, 0, 
        hwnd,
        (HMENU)IDC_MAIN_EDIT,
        GetModuleHandle(NULL),
        NULL);

    if (hEdit == NULL)
        MessageBox(hwnd, "Could not create edit box.", "Error", MB_OK | MB_ICONERROR);

    //create backup window
    hOriginalDataWindow = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        "EDIT", "",
        WS_CHILD |
        WS_VISIBLE |
        WS_VSCROLL |
        WS_HSCROLL |
        ES_MULTILINE |
        ES_AUTOVSCROLL |
        ES_AUTOHSCROLL |
        ES_READONLY,
        0, 0, 0, 0,
        hwnd,
        (HMENU)IDC_MAIN_EDIT,
        GetModuleHandle(NULL),
        NULL);

    if (hOriginalDataWindow == NULL)
        MessageBox(hwnd, "Could not create edit box.", "Error", MB_OK | MB_ICONERROR);

    SetFont(hwnd, 0);

    //create 'open folder' button
    hOpenFolderButton = CreateWindowEx(
        NULL,
        "Button",
        "Open Folder",
        WS_BORDER | WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0, 0, 0, 0,
        hwnd, 
        (HMENU)BTN_OPENFOLDER,
        GetModuleHandle(NULL),
        NULL);

    //create 'revert' button
    hRevertButton = CreateWindowEx(
        NULL,
        "Button",
        "Revert",
        WS_BORDER | WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0, 0, 0, 0,
        hwnd, 
        (HMENU)BTN_REVERT,
        GetModuleHandle(NULL),
        NULL);

    //create 'Open File' button
    hOpenButton = CreateWindowEx(
        NULL,
        "Button",
        "Open File",
        WS_BORDER | WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0, 0, 0, 0,
        hwnd,
        (HMENU)BTN_OPENFILE,
        GetModuleHandle(NULL),
        NULL);

    if (hOpenFolderButton == NULL || hRevertButton == NULL || hOpenButton == NULL)
        MessageBox(hwnd, "Could not create one of the buttons.", "Error", MB_OK | MB_ICONERROR);

    if (cmdlinearg_path_set) {

        //remove quotes from the file path
        if (cmdlinearg_path[0] == '"') {
            int len = strlen(cmdlinearg_path);
            memcpy(cmdlinearg_path, cmdlinearg_path + 1, len - 2);
            cmdlinearg_path[len - 2] = 0;
        }

        //use given path to load the initial file
        LoadTextFile(hwnd, hEdit, (LPSTR)cmdlinearg_path);
    }
}


VOID DoWM_SIZE(HWND hwnd)
{

    int buttonwidth = 150;//fixme
    int buttonheight = 150;
    int border = 10;

    RECT rcClient;
    if (!GetClientRect(hwnd, &rcClient)) { };

    RECT rcempty;
    if (!SetRect(&rcempty, 0, 0, 0, 0)) { };

    RECT remaining_space;
    if (!CopyRect(&remaining_space, &rcClient)) { }



    //get status bar rect, subtrace from remaining space
    RECT rcstatusbar;
    if (!CopyRect(&rcstatusbar, &rcempty)) {}
    if (statusbar) {
        SendMessage(hStatus, WM_SIZE, 0, 0);
        GetWindowRect(hStatus, &rcstatusbar);
        remaining_space.bottom -= rcstatusbar.bottom - rcstatusbar.top;
    }
    



    //status area, the open folder, revert, and open file buttons
    RECT rcstatusarea;
    if (!CopyRect(&rcstatusarea, &rcempty)) {}
    if (!statusarea) {
        MoveWindow(hOpenFolderButton, 0, 0, 0, 0, TRUE);
        MoveWindow(hRevertButton, 0, 0, 0, 0, TRUE);
        MoveWindow(hOpenButton, 0, 0, 0, 0, TRUE);
    }
    else {

        //subtract status area from remaining space
        if (!CopyRect(&rcstatusarea, &remaining_space)) {}
        rcstatusarea.bottom = rcstatusarea.top + STATUSAREAHEIGHT;
        remaining_space.top = rcstatusarea.bottom;

        if (fileopen) {

            //hide open file button
            MoveWindow(hOpenButton, 0, 0, 0, 0, TRUE);

            //show the open folder, and revert button
            RECT rcopenfolder;
            if (!SetRect(&rcopenfolder, rcstatusarea.right - buttonwidth, 0, buttonwidth, STATUSAREAHEIGHT / 2)) { }
            MoveWindow(hOpenFolderButton, rcstatusarea.right - buttonwidth, 0, buttonwidth, STATUSAREAHEIGHT / 2, TRUE);
            MoveWindow(hRevertButton, rcstatusarea.right - buttonwidth, 50, buttonwidth, STATUSAREAHEIGHT / 2, TRUE);
        }
        else {

            //hide open folder, and revert button
            MoveWindow(hOpenFolderButton, 0, 0, 0, 0, TRUE);
            MoveWindow(hRevertButton, 0, 0, 0, 0, TRUE);

            //show open file button
            MoveWindow(hOpenButton, rcstatusarea.right - buttonwidth, 0, buttonwidth, STATUSAREAHEIGHT, TRUE);
        }
    }

    //original data window is either hidden or the top half of remaining
    if (OrigDataWindow) {
        RECT rcODW;
        if (!CopyRect(&rcODW, &remaining_space)) { }
        rcODW.bottom -= (rcODW.bottom - rcODW.top) / 2;
        remaining_space.top = rcODW.bottom;
        MoveWindow(hOriginalDataWindow, rcODW.left, rcODW.top, rcODW.right - rcODW.left, rcODW.bottom - rcODW.top, TRUE);
    }
    else {
        MoveWindow(hOriginalDataWindow, 0, 0, 0, 0, TRUE);
    }

    //remaining_space is reduced by border if there is no file
    if (!fileopen) {
        remaining_space.bottom -= border;
        remaining_space.left += border;
        remaining_space.top += border;
        remaining_space.right -= border;
    }

    //edit window takes the remaining space!
    MoveWindow(hEdit, 
               remaining_space.left,
               remaining_space.top,
               remaining_space.right - remaining_space.left,
               remaining_space.bottom - remaining_space.top,
               TRUE);

}

VOID DrawStatusArea(HWND hwnd, HDC hdc, RECT rc)
{
    //blue background
    FillRect(hdc, &rc, CreateSolidBrush(RGB(0, 0, 255)));
    int border = 10;
    int buttonwidth = 150;

    //white main area, constricted by a border and the button width
    rc.bottom -= border;
    rc.left += border;
    rc.top += border;
    rc.right = rc.right - border - buttonwidth;
    FillRect(hdc, &rc, CreateSolidBrush(RGB(255, 255, 255)));

    if (fileopen) {

        //top left corner, filepath
        DrawText(hdc, g_filepath, strlen(g_filepath), &rc, DT_LEFT | DT_EDITCONTROL);

        //20 px down, filename
        rc.top += 20;
        DrawText(hdc, g_filename, strlen(g_filename), &rc, DT_LEFT | DT_EDITCONTROL);
    }
    else {

        //draw 'no file open'
        DrawText(hdc, "No File Opened", 14, &rc, DT_LEFT | DT_EDITCONTROL);
    }

}

VOID Do_WM_PAINT(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    RECT rcClient;
    if (!GetClientRect(hwnd, &rcClient)) { };
    
    RECT rcempty; 
    if (!SetRect(&rcempty, 0, 0, 0, 0)) { };
    

    //background is red for no file, green with a file
    FillRect(hdc, &rcClient, CreateSolidBrush((fileopen ? RGB(0, 255, 0) : RGB(255, 0, 0))));
    
    //status area
    RECT rcstatus;
    if (!CopyRect(&rcstatus, &rcempty)) { }
    if (statusarea) {
        if (!CopyRect(&rcstatus, &rcClient)) {}
        rcstatus.bottom = rcstatus.top + STATUSAREAHEIGHT;
        DrawStatusArea(hwnd, hdc, rcstatus);
    }
    

    EndPaint(hwnd, &ps);
}

VOID DoWM_COMMAND(HWND hwnd, WPARAM wParam, LPARAM lParam)
{

    switch (HIWORD(wParam))
    {
    case EN_CHANGE:
        if (fileopen) {
            SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)DirtyMsg);
            filedirty = TRUE;
            TimerReset();
        }
        else {
            SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)"File not opened! (save or open new file).");
        }
        break;
    case BN_CLICKED:
        if (fileopen) {
            if (LOWORD(wParam) == BTN_OPENFOLDER)
                OpenFolder(hwnd);
            if (LOWORD(wParam) == BTN_REVERT)
                Revert(hwnd);
        }
        else {
            if (LOWORD(wParam) == BTN_OPENFILE)
                PromptAndLoadFile(hwnd);
        }
        break;
    
    
    
    }


    switch (LOWORD(wParam))
    {
    case ID_FILE_EXIT:
        PostMessage(hwnd, WM_CLOSE, 0, 0);
        break;
    case ID_VIEW_HEADERAREA:
    {
        HMENU hMenu = GetMenu(hwnd);
        statusarea = !statusarea;
        CheckMenuItem(hMenu, ID_VIEW_HEADERAREA, statusarea ? MF_CHECKED : MF_UNCHECKED);
        FullRedraw(hwnd);
        break;
    }
    case ID_VIEW_STATUSBAR:
    {   
        HMENU hMenu = GetMenu(hwnd);
        statusbar = !statusbar;
        CheckMenuItem(hMenu, ID_VIEW_STATUSBAR, statusbar ? MF_CHECKED : MF_UNCHECKED);
        ShowWindow(hStatus, statusbar);
        FullRedraw(hwnd);
        break;
    }

    case ID_VIEW_ORIGINALFILEDATA: 
    {
        HMENU hMenu = GetMenu(hwnd);
        OrigDataWindow = !OrigDataWindow;
        CheckMenuItem(hMenu, ID_VIEW_ORIGINALFILEDATA, statusbar ? MF_CHECKED : MF_UNCHECKED);
        ShowWindow(hOriginalDataWindow, OrigDataWindow);
        FullRedraw(hwnd);

        break;
    }

    }
}

LRESULT CALLBACK WndProc(HWND hwnd,
                         UINT msg,
                         WPARAM wParam,
                         LPARAM lParam)
{
    switch (msg)
    {
    
    case WM_CREATE:
        DoWM_CREATE(hwnd);
        break;
    
    case WM_SIZE:
        DoWM_SIZE(hwnd);
        break;

    case WM_PAINT:
        Do_WM_PAINT(hwnd);
        break;
    
    case WM_COMMAND:
        DoWM_COMMAND(hwnd, wParam, lParam);
        break;

    case WM_MOUSEWHEEL:
        MouseScroll(hwnd, wParam);
        break;

    case WM_TIMER:
        if (fileopen) {
            if (++dirtycount >= DIRTYTHRESHOLD)
            {
                SaveOpenedFile(hwnd);
            }
        }
        break;
    
    case WM_CLOSE:
        EmergenecySave(hwnd);
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

HWND RegisterAndCreateMainWindow(HINSTANCE hInstance)
{
    WNDCLASSEX wc = { 0 };
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
        MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    HWND hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,        //extended window style
        g_szClassName,           //ties the new window to the window class we just created
        "SAVEpad",               //(Initial) Window Title
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
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    return hwnd;
}

int WINAPI WinMain(HINSTANCE hInstance, 
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, 
                   int nCmdShow)
{
    statusarea = TRUE;
    statusbar = TRUE;
    OrigDataWindow = TRUE;
    fileopen = FALSE;
    filedirty = FALSE;
    dirtycount = 0;
    backup = NULL;
    backupsize = 0;

    InitCommonControls();

    //Create Window
    HWND hwnd;
    if ((hwnd = RegisterAndCreateMainWindow(hInstance)) == NULL) {
        return 0;
    }
    
    //Read command line args
    ZeroMemory(cmdlinearg_path, MAX_PATH);
    cmdlinearg_path_set = FALSE;
    if (strcmp(lpCmdLine, ""))
    {
        sprintf_s(cmdlinearg_path, lpCmdLine);
        cmdlinearg_path_set = TRUE;
    }

    //Mark Window as Visible
    ShowWindow(hwnd, nCmdShow);

    //Sends initial WM_PAINT, possibly not needed?
    UpdateWindow(hwnd);
    
    //Register 'dirty' timer
    if (SetTimer(hwnd, ID_TIMER, TIMERGRANULARITY, NULL) == 0)
        MessageBox(hwnd, "Could not SetTimer()!", "Error", MB_OK | MB_ICONEXCLAMATION);

    //Ye Ol' Message Pump
    MSG m;
    while (GetMessage(&m, NULL, 0, 0) > 0) {
        TranslateMessage(&m);
        DispatchMessage(&m);
    }

    return m.wParam;
}

