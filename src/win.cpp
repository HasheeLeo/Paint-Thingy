// Target Windows XP
#define WINVER 0x0501

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include "rsc/resource.h"

#define IDC_MAIN_TOOLBAR   4001
#define IDC_MAIN_PAINTDUMP 4002
#define IDC_MAIN_PAINTAREA 4003

#define IDC_PEN 5001
#define IDC_LINE 5002

// Simplifiers
int CheckButtonPushed(HWND paintArea)
{
    HWND paintDump = GetDlgItem(GetParent(paintArea), IDC_MAIN_PAINTDUMP);
    if(IsDlgButtonChecked(paintDump, IDC_PEN) == BST_CHECKED)
        return IDC_PEN;

    else if(IsDlgButtonChecked(paintDump, IDC_LINE) == BST_CHECKED)
        return IDC_LINE;

    return 0;
}

// Forward declarations
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK PaintPanelProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK PaintAreaProc(HWND, UINT, WPARAM, LPARAM);

int CALLBACK WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
    const char mainWndClassName[] = "Main Window";

    WNDCLASSEX mainWndClass;
    ZeroMemory(&mainWndClass, sizeof(mainWndClass));

    mainWndClass.cbSize = sizeof(WNDCLASSEX);
    mainWndClass.lpfnWndProc = MainWndProc;
    mainWndClass.hInstance = hInst;
    mainWndClass.hCursor = LoadCursor(0, IDC_ARROW);
    mainWndClass.hbrBackground = CreateSolidBrush(RGB(220,220,200));
    mainWndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MAINMENU);
    mainWndClass.lpszClassName = mainWndClassName;

    if(!RegisterClassEx(&mainWndClass)) {
        MessageBox(0, "Could not register the main window class.", "Error", MB_OK|MB_ICONERROR);
        return 0;
    }

    HWND hMainWnd = CreateWindowEx(0,
                                   mainWndClassName,
                                   "Paint thingy",
                                   WS_OVERLAPPEDWINDOW,
                                   CW_USEDEFAULT,
                                   CW_USEDEFAULT,
                                   CW_USEDEFAULT,
                                   CW_USEDEFAULT,
                                   0,
                                   0,
                                   hInst,
                                   0);

    if(!hMainWnd) {
        MessageBox(0, "Could not create the main window.", "Error", MB_OK|MB_ICONERROR);
        return 0;
    }

    ShowWindow(hMainWnd, nCmdShow);

    MSG mainWndMsg;
    while(GetMessage(&mainWndMsg, hMainWnd, 0, 0) > 0) {
        TranslateMessage(&mainWndMsg);
        DispatchMessage(&mainWndMsg);
    }

    return mainWndMsg.wParam;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_CREATE:
        {
            // Create toolbar
            HWND toolbar = CreateWindowEx(0, TOOLBARCLASSNAME, 0, WS_CHILD|WS_VISIBLE, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(IDC_MAIN_TOOLBAR), GetModuleHandle(0), 0);
            SendMessage(toolbar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);

            TBADDBITMAP tBitmaps;
            tBitmaps.hInst = HINST_COMMCTRL;
            tBitmaps.nID = IDB_STD_SMALL_COLOR;
            SendMessage(toolbar, TB_ADDBITMAP, 0, reinterpret_cast<LPARAM>(&tBitmaps));

            TBBUTTON tButtons[1];
            ZeroMemory(tButtons, sizeof(tButtons));

            tButtons[0].iBitmap = STD_FILENEW;
            tButtons[0].fsState = TBSTATE_ENABLED;
            tButtons[0].fsStyle = TBSTYLE_BUTTON;
            tButtons[0].idCommand = IDM_MAINMENU_NEW;

            SendMessage(toolbar, TB_ADDBUTTONS, sizeof(tButtons)/sizeof(tButtons[0]), reinterpret_cast<LPARAM>(&tButtons));

            // Create paint panel
            WNDCLASSEX paintPanelClass;
            ZeroMemory(&paintPanelClass, sizeof(paintPanelClass));

            const char paintPanelClassName[] = "Paint Panel";

            paintPanelClass.cbSize = sizeof(WNDCLASSEX);
            paintPanelClass.lpfnWndProc = PaintPanelProc;
            paintPanelClass.hInstance = GetModuleHandle(0);
            paintPanelClass.hCursor = LoadCursor(0, IDC_ARROW);
            paintPanelClass.hbrBackground = CreateSolidBrush(RGB(220,220,220));
            paintPanelClass.lpszClassName = paintPanelClassName;

            RegisterClassEx(&paintPanelClass);

            RECT toolbarRect;
            GetWindowRect(toolbar, &toolbarRect);

            CreateWindowEx(WS_EX_TOOLWINDOW, paintPanelClassName, 0, WS_BORDER|WS_CHILD|WS_VISIBLE, 1, toolbarRect.bottom-toolbarRect.top, 80, 300, hwnd, reinterpret_cast<HMENU>(IDC_MAIN_PAINTDUMP), GetModuleHandle(0), 0);

            // Set menu bitmaps
            HBITMAP bNew = static_cast<HBITMAP>(LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(IDB_NEW), IMAGE_BITMAP, 16, 16, LR_DEFAULTCOLOR|LR_SHARED));
            HBITMAP bExit = static_cast<HBITMAP>(LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(IDB_EXIT), IMAGE_BITMAP, 16, 16, LR_DEFAULTCOLOR|LR_SHARED));

            HMENU menu = GetSubMenu(GetMenu(hwnd), 0);
            SetMenuItemBitmaps(menu, IDM_MAINMENU_NEW, MF_BYCOMMAND, bNew, 0);
            SetMenuItemBitmaps(menu, IDM_MAINMENU_EXIT, MF_BYCOMMAND, bExit, 0);
        }

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDM_MAINMENU_NEW:
                {
                    const char paintAreaClassName[] = "Paint Area";
                    HINSTANCE hInst = GetModuleHandle(0);

                    WNDCLASSEX paintAreaClass;
                    ZeroMemory(&paintAreaClass, sizeof(paintAreaClass));

                    // If the paint area isn't already created
                    if(!GetClassInfoEx(hInst, paintAreaClassName, &paintAreaClass)) {
                        paintAreaClass.cbSize = sizeof(WNDCLASSEX);
                        paintAreaClass.lpfnWndProc = PaintAreaProc;
                        paintAreaClass.hInstance = hInst;
                        paintAreaClass.hCursor = LoadCursor(0, IDC_ARROW);
                        paintAreaClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW+1);
                        paintAreaClass.lpszClassName = paintAreaClassName;

                        RegisterClassEx(&paintAreaClass);

                        HWND paintPanel = GetDlgItem(hwnd, IDC_MAIN_PAINTDUMP);

                        RECT paintPanelRect;

                        // Convert coordinates from relative to desktop to relative to the main window.
                        GetWindowRect(paintPanel, &paintPanelRect);
                        MapWindowPoints(HWND_DESKTOP, hwnd, reinterpret_cast<LPPOINT>(&paintPanelRect), 2);

                        CreateWindowEx(0, paintAreaClassName, 0, WS_CHILD|WS_SIZEBOX|WS_VISIBLE, (paintPanelRect.right-paintPanelRect.left)+1, paintPanelRect.top, 400, paintPanelRect.bottom-paintPanelRect.top, hwnd, reinterpret_cast<HMENU>(IDC_MAIN_PAINTAREA), hInst, 0);

                        SetWindowText(hwnd, "Paint thingy - Untitled");
                    }
                    else {
                        InvalidateRect(GetDlgItem(hwnd, IDC_MAIN_PAINTAREA), 0, 1);
                        UpdateWindow(GetDlgItem(hwnd, IDC_MAIN_PAINTAREA));
                    }
                }
                break;

                case IDM_MAINMENU_EXIT:
                    SendMessage(hwnd, WM_CLOSE, 0, 0);
                break;
            }
        break;

        case WM_SIZE:
            SendMessage(GetDlgItem(hwnd, IDC_MAIN_TOOLBAR), TB_AUTOSIZE, 0, 0);
        break;

        case WM_CLOSE:
            DestroyWindow(hwnd);
        break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK PaintPanelProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_CREATE:
        {
            HINSTANCE hInst = GetModuleHandle(0);

            HWND bPen = CreateWindowEx(0, "BUTTON", 0, BS_AUTORADIOBUTTON|BS_PUSHLIKE|BS_ICON|WS_CHILD|WS_GROUP|WS_VISIBLE, 0, 0, 24, 24, hwnd, reinterpret_cast<HMENU>(IDC_PEN), hInst, 0);
            HWND bLine = CreateWindowEx(0, "BUTTON", 0, BS_AUTORADIOBUTTON|BS_PUSHLIKE|BS_ICON|WS_CHILD|WS_VISIBLE, 0, 24, 24, 24, hwnd, reinterpret_cast<HMENU>(IDC_LINE), hInst, 0);

            HICON iPen = static_cast<HICON>(LoadImage(hInst, MAKEINTRESOURCE(IDI_PEN), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR|LR_SHARED));
            HICON iLine = static_cast<HICON>(LoadImage(hInst, MAKEINTRESOURCE(IDI_LINE), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR|LR_SHARED));

            SendMessage(bPen, BM_SETIMAGE, IMAGE_ICON, reinterpret_cast<LPARAM>(iPen));
            SendMessage(bLine, BM_SETIMAGE, IMAGE_ICON, reinterpret_cast<LPARAM>(iLine));
        }
        break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK PaintAreaProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        static int startX = 0;
        static int startY = 0;

        static int lastStartX = 0;
        static int lastStartY = 0;
        static int lastEndX = 0;
        static int lastEndY = 0;

        case WM_LBUTTONDOWN:
            switch(CheckButtonPushed(hwnd))
            {
                case IDC_PEN:
                {
                    HDC dc = GetDC(hwnd);
                    SetPixel(dc, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), RGB(0,0,0));
                    ReleaseDC(hwnd, dc);

                    startX = GET_X_LPARAM(lParam);
                    startY = GET_Y_LPARAM(lParam);
                }
                break;

                case IDC_LINE:
                    startX = GET_X_LPARAM(lParam);
                    startY = GET_Y_LPARAM(lParam);
                break;
            }
        break;

        case WM_MOUSEMOVE:
        {
            if(!(wParam == MK_LBUTTON))
                break;

            switch(CheckButtonPushed(hwnd))
            {
                case IDC_PEN:
                {
                    HDC dc = GetDC(hwnd);
                    MoveToEx(dc, startX ? startX : GET_X_LPARAM(lParam), startY ? startY : GET_Y_LPARAM(lParam), 0);
                    LineTo(dc, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
                    startX = GET_X_LPARAM(lParam);
                    startY = GET_Y_LPARAM(lParam);
                    ReleaseDC(hwnd, dc);

                    TRACKMOUSEEVENT tme;
                    tme.cbSize = sizeof(TRACKMOUSEEVENT);
                    tme.dwFlags = TME_LEAVE;
                    tme.hwndTrack = hwnd;
                    tme.dwHoverTime = 0;
                    TrackMouseEvent(&tme);
                }
                break;

                case IDC_LINE:
                {
                    HDC dc = GetDC(hwnd);
                    HGDIOBJ oldPen;

                    if(lastStartX && lastEndX) {
                        oldPen = SelectObject(dc, GetStockObject(DC_PEN));
                        SetDCPenColor(dc, RGB(255,255,255));

                        MoveToEx(dc, lastStartX, lastStartY, 0);
                        LineTo(dc, lastEndX, lastEndY);
                    }

                    if(oldPen)
                        SelectObject(dc, oldPen);

                    MoveToEx(dc, startX ? startX : GET_X_LPARAM(lParam), startY ? startY : GET_Y_LPARAM(lParam), 0);
                    LineTo(dc, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
                    ReleaseDC(hwnd, dc);

                    lastStartX = startX;
                    lastStartY = startY;
                    lastEndX = GET_X_LPARAM(lParam);
                    lastEndY = GET_Y_LPARAM(lParam);
                }
            }
        }
        break;

        case WM_LBUTTONUP:
        {
            switch(CheckButtonPushed(hwnd))
            {
                case IDC_PEN:
                    startX = 0;
                    startY = 0;
                break;

                case IDC_LINE:
                {
                    lastStartX = 0;
                    lastStartY = 0;
                    lastEndX = 0;
                    lastEndY = 0;
                }
                break;
            }
        }
        break;

        case WM_MOUSELEAVE:
            startX = 0;
            startY = 0;
        break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}
