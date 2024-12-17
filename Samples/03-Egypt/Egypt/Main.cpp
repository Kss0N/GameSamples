// HelloTriangle.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Egypt.h"

static Egypt g_engine;

#define MAX_LOADSTRING 64

// Global Variables:

static TCHAR 
    g_zTitle[MAX_LOADSTRING],                  // The title bar text
    g_zWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
static ATOM                RegisterMainClass(HINSTANCE hInstance);
static LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
static INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


_Use_decl_annotations_
extern int APIENTRY 
_tWinMain(HINSTANCE hInstance,
          HINSTANCE hPrevInstance,
          LPTSTR    lpCmdLine,
          int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    g_engine.OnInit();

    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, g_zTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_MAIN_WND, g_zWindowClass, MAX_LOADSTRING);
    
    const ATOM aCls = RegisterMainClass(hInstance);
    if (!aCls)
        return FALSE;

    // Perform application initialization:

    HWND hWnd = CreateWindowEx(0L, g_zWindowClass, g_zTitle, WS_OVERLAPPEDWINDOW,
        // X * Y
        CW_USEDEFAULT, 0,
        // Width * Height
        CW_USEDEFAULT, 0,
        NULL, NULL, hInstance, NULL);

    if (!hWnd)
        return FALSE;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_HELLOTRIANGLE));

    MSG msg = {};

    // Main message loop:
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            continue; // keep going until message loop is empty.
        }

        g_engine.OnRender();
    }

    g_engine.OnDestroy();
    
    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM RegisterMainClass(HINSTANCE hInstance)
{
    const WNDCLASSEX wcex = {
        .cbSize = sizeof(wcex),
        .style          = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc    = WndProc,
        .cbClsExtra     = 0,
        .cbWndExtra     = 0,
        .hInstance      = hInstance,
        .hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP)),
        .hCursor        = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground  = (HBRUSH)(COLOR_WINDOWFRAME),
        .lpszClassName  = g_zWindowClass,
        .hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL)),
    };
    return RegisterClassEx(&wcex);
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox((HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_CREATE:
    {
        RECT cr;
        GetClientRect(hWnd, &cr);
        WORD width = (WORD)(cr.right - cr.left),
            height = (WORD)(cr.bottom - cr.top);
        g_engine.OnSurfaceLoaded(hWnd, width, height);
        break;
    }
    default:
        break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}