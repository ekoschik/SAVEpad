
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

    if (cmdlinearg_path_set) {

        //remove quotes from the file path
        if (cmdlinearg_path[0] == '"') {
            int len = strlen(cmdlinearg_path);
            memcpy(cmdlinearg_path, cmdlinearg_path + 1, len - 2);
            cmdlinearg_path[len - 2] = 0;
        }

        DoFileOpen(hwnd, (LPSTR)cmdlinearg_path);
    }
}

VOID DoWM_SIZE(HWND hwnd)
{
    // Get height of status bar
    RECT rcStatus;
    SendMessage(hStatus, WM_SIZE, 0, 0); //why?
    GetWindowRect(hStatus, &rcStatus);
    int iStatusHeight = rcStatus.bottom - rcStatus.top;

    //Size of red boarder to show no file is open
    int boardersize = fileopen ? 0 : NOFILE_BOARDER_SIZE;

    int statusAreaHeight = statusarea && fileopen ? STATUSAREAHEIGHT : 0;

    // Set size of edit window
    RECT rcClient;
    GetClientRect(hwnd, &rcClient);
    int iEditHeight = rcClient.bottom - iStatusHeight;
    SetWindowPos(hEdit,
        NULL,
        boardersize,
        boardersize + statusAreaHeight,
        rcClient.right - 2 * boardersize,
        iEditHeight - 2 * boardersize,
        SWP_NOZORDER);
}

VOID Do_WM_PANT(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    if (!fileopen)
    {
        RECT rcClient;
        GetClientRect(hwnd, &rcClient);
        FillRect(hdc, &rcClient, CreateSolidBrush(RGB(255, 0, 0)));

    }

    else if (statusarea)
    {
        RECT rcClient;
        GetClientRect(hwnd, &rcClient);

        RECT rcStatusArea = { 0, 0, rcClient.right, STATUSAREAHEIGHT };
        //            FillRect(hdc, &rcClient, CreateSolidBrush(RGB(0, 0, 255)));


        RECT rc = { 30, 30, 150, 150 };
        DrawText(hdc, "This is some text.", 18, &rc, DT_LEFT | DT_EDITCONTROL);
    }

    EndPaint(hwnd, &ps);
}

VOID DoWM_COMMAND(HWND hwnd, WPARAM wParam, LPARAM lParam)
{

    //The Edit control's change messages are stored in the HIWORD of wParam
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

    }


    //Messages from the Menu in LOWORD of wParam
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
        Do_WM_PANT(hwnd);
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

