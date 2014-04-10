
#include "resource.h"



VOID DoWM_CREATE(HWND hwnd)
{
    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(NULL, MAKEINTRESOURCE(IDI_ICON1)));

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

    if (hEdit == NULL)
        MessageBox(hwnd, "Could not create edit box.", "Error", MB_OK | MB_ICONERROR);

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

    int halfOfStatusAreaHeight = (STATUSAREAHEIGHT - 2 * 10) / 2;

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

    if (cmdlinearg_path_set) {

        //remove quotes from the file path
        if (cmdlinearg_path[0] == '"') {
            int len = strlen(cmdlinearg_path);
            memcpy(cmdlinearg_path, cmdlinearg_path + 1, len - 2);
            cmdlinearg_path[len - 2] = 0;
        }

        LoadTextFile(hwnd, hEdit, (LPSTR)cmdlinearg_path);
    }
}

VOID DoWM_SIZE(HWND hwnd)
{
    RECT rcClient;
    GetClientRect(hwnd, &rcClient);

    int border = 10;
    int buttonwidth = 150;
    int buttonheight = 150;


    int iStatusHeight = 0;
    int statusAreaHeight = 0;
    int origDataWindowHeight = 0;


    // set siStatusHeight
    if (statusbar) {
        RECT rcStatus;
        SendMessage(hStatus, WM_SIZE, 0, 0); //why?
        GetWindowRect(hStatus, &rcStatus);
        iStatusHeight = rcStatus.bottom - rcStatus.top;
    }
    
    // resize buttons, and set statusAreaHeight
    if (!statusarea) {
        MoveWindow(hOpenFolderButton, 0, 0, 0, 0, TRUE);
        MoveWindow(hRevertButton, 0, 0, 0, 0, TRUE);
        MoveWindow(hOpenButton, 0, 0, 0, 0, TRUE);
    }
    else  {

        statusAreaHeight = STATUSAREAHEIGHT;

        if (fileopen) {
            MoveWindow(hOpenFolderButton, rcClient.right - buttonwidth, 0, buttonwidth, STATUSAREAHEIGHT / 2, TRUE);
            MoveWindow(hRevertButton, rcClient.right - buttonwidth, buttonheight, buttonwidth, STATUSAREAHEIGHT / 2, TRUE);
            MoveWindow(hOpenButton, 0, 0, 0, 0, TRUE);
        }
        else {
            MoveWindow(hOpenFolderButton, 0, 0, 0, 0, TRUE);
            MoveWindow(hRevertButton, 0, 0, 0, 0, TRUE);
            MoveWindow(hOpenButton, rcClient.right - buttonwidth, 0, buttonwidth, STATUSAREAHEIGHT, TRUE);
        }
    }
    
    if (OrigDataWindow) {

        int height = (rcClient.bottom - 3 * border - statusAreaHeight - iStatusHeight) / 2;

        int top = border + (statusarea ? STATUSAREAHEIGHT + border : 0); //top
        int left = border;
        int right = rcClient.right - border;
        int bottom = rcClient.bottom - 2 * border - height;
        
        
        MoveWindow(hOriginalDataWindow, top, left, right, bottom, TRUE);

        MoveWindow(hEdit, top + border + height,
            border,
            rcClient.right - border,
            rcClient.bottom - iStatusHeight,
            TRUE);


    }
    else {
        // Set size of edit window
        SetWindowPos(hEdit,
            NULL,
            border,
            border + statusAreaHeight,
            rcClient.right - 2 * border,
            rcClient.bottom - iStatusHeight - statusAreaHeight - 2 * border,
            SWP_NOZORDER);
    }

  
}

//VOID DrawStatusArea(HWND hwnd, HDC hdc)
//{
//    //draw a blue boarder around a white status area
//    RECT rcClient;
//    GetClientRect(hwnd, &rcClient);
//    RECT rcStatusArea = { 0, 0, rcClient.right, STATUSAREAHEIGHT };
//    FillRect(hdc, &rcStatusArea, CreateSolidBrush(RGB(0, 0, 255)));
//    int statareaborder = 10;
//    RECT rcInnerStatusArea = { statareaborder,
//                               statareaborder,
//                               rcClient.right -  statareaborder - 150,
//                               STATUSAREAHEIGHT - statareaborder };
//    FillRect(hdc, &rcInnerStatusArea, CreateSolidBrush(RGB(255, 255, 255)));
//
//
//    //RECT rc = { 30, 30, 300, 300 };
//    if (fileopen) {
//        DrawText(hdc, g_filepath, strlen(g_filepath), &rcInnerStatusArea, DT_LEFT | DT_EDITCONTROL);
//        rcInnerStatusArea.top += 20;
//        DrawText(hdc, g_filename, strlen(g_filename), &rcInnerStatusArea, DT_LEFT | DT_EDITCONTROL);
//
//
//    }
//    else {
//        DrawText(hdc, "No File Opened", 14, &rcInnerStatusArea, DT_LEFT | DT_EDITCONTROL);
//
//    }
//
//}

VOID Do_WM_PAINT(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    if (!fileopen)
    {
        RECT rcClient;
        GetClientRect(hwnd, &rcClient);
        FillRect(hdc, &rcClient, CreateSolidBrush(RGB(255, 0, 0)));
    }


    if (statusarea)
    {
        RECT rcClient;
        GetClientRect(hwnd, &rcClient);
        RECT rcStatusArea = { 0, 0, rcClient.right, STATUSAREAHEIGHT };
        FillRect(hdc, &rcStatusArea, CreateSolidBrush(RGB(0, 0, 255)));
        int statareaborder = 10;
        RECT rcInnerStatusArea = { statareaborder,
            statareaborder,
            rcClient.right - statareaborder - 150,
            STATUSAREAHEIGHT - statareaborder };
        FillRect(hdc, &rcInnerStatusArea, CreateSolidBrush(RGB(255, 255, 255)));


        //RECT rc = { 30, 30, 300, 300 };
        if (fileopen) {
            DrawText(hdc, g_filepath, strlen(g_filepath), &rcInnerStatusArea, DT_LEFT | DT_EDITCONTROL);
            rcInnerStatusArea.top += 20;
            DrawText(hdc, g_filename, strlen(g_filename), &rcInnerStatusArea, DT_LEFT | DT_EDITCONTROL);


        }
        else {
            DrawText(hdc, "No File Opened", 14, &rcInnerStatusArea, DT_LEFT | DT_EDITCONTROL);

        }
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
        TimerTick(hwnd);
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

