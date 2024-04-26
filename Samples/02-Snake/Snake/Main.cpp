// Snake.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Snake.h"
#include <Keyboard.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
TCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

static constexpr UINT
c_Rows = 16u,
c_Cols = 16u,
c_MaxSnakeLength = 255;

static Sample02 g_engine(c_Rows, c_Cols, c_MaxSnakeLength);
static DirectX::Keyboard g_keyboard;

static LONGLONG g_qpcFrequency;
static LONGLONG g_qpcLastCounter;

static constexpr double c_FrameTimeSeconds = .33;

struct SnakeGame 
{
    std::vector<snake_vector> snake;

    snake_vector apple;

    snake_vector facing_dir;

    void Tick(bool bGrow);

};
static SnakeGame g_game;


// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

extern int APIENTRY 
_tWinMain(_In_      HINSTANCE hInstance,
          _In_opt_  HINSTANCE hPrevInstance,
          _In_      LPTSTR    lpCmdLine,
          _In_      int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    if (!DirectX::XMVerifyCPUSupport())
    {
        __debugbreak();
        return -1;
    }
    if (!g_keyboard.IsConnected())
    {
        __debugbreak();
        return -1;
    }
    if (FAILED(CoInitialize(NULL)))
    {
        __debugbreak();
        return -1;
    }

    QueryPerformanceFrequency((PLARGE_INTEGER)&g_qpcFrequency);
    QueryPerformanceCounter((PLARGE_INTEGER)&g_qpcLastCounter);

    g_engine.OnInit();

    g_game.snake = { snake_vector(2, 7), snake_vector{1,7} };
    g_game.apple = snake_vector{ 10,10 };

    g_game.facing_dir = snake_vector{ 1, 0 };

    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_SNAKE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SNAKE));

    // Main message loop:
    MSG msg;
    while (true)
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        if (msg.message == WM_QUIT)
            goto EXIT;

        LONGLONG qpcCurrentTime;
        QueryPerformanceCounter((PLARGE_INTEGER)&qpcCurrentTime);

        double dt = double(qpcCurrentTime - g_qpcLastCounter) / g_qpcFrequency;

        g_engine.OnRender();

        if (dt < c_FrameTimeSeconds)
            continue;
        g_qpcLastCounter = qpcCurrentTime;

        auto state = g_keyboard.GetState();
        
        // Will only be able to move in one direction at a time.
        
        if (g_game.facing_dir.y == 0)
        {
            auto vertical_turn = INT(state.IsKeyDown(g_keyboard.Up)) - INT(state.IsKeyDown(g_keyboard.Down));
            if(vertical_turn != 0)
                g_game.facing_dir = snake_vector{ 0, vertical_turn};
        }
        else if (g_game.facing_dir.x == 0)
        {
            auto horizontal_turn = INT(state.IsKeyDown(g_keyboard.Right)) - INT(state.IsKeyDown(g_keyboard.Left));
            if (horizontal_turn != 0)
                g_game.facing_dir = snake_vector{ horizontal_turn, 0 };
        }

        // Handle Apple
        if (g_game.apple == g_game.snake[0])
        {
            snake_vector candidate;
            while (true)
            {
                candidate = snake_vector{ INT(rand() % c_Rows), INT(rand() % c_Cols) };
                for (auto& body : g_game.snake)
                {
                    if (body == candidate)
                        goto CONTINUE;

                }
                break;
            CONTINUE:
                continue;
            }

            g_game.apple = candidate;

            g_game.Tick(true);
        }
        else
            g_game.Tick(false);

        g_engine.UpdateEntityPositions(std::span(g_game.snake), g_game.apple);        
    }
EXIT:
    g_engine.OnDestroy();
    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex = {};

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SNAKE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
       // X * Y 
       CW_USEDEFAULT, 0, 
       // Width * Height
       CW_USEDEFAULT, 0, 
       nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
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
    g_keyboard.ProcessMessage(message, wParam, lParam);
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_CREATE:
    {
        RECT cr;
        GetClientRect(hWnd, &cr);
        g_engine.OnSurfaceLoaded(hWnd, cr.right, cr.bottom);
        break;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
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

void SnakeGame::Tick(bool bGrow)
{
    auto tail = snake[snake.size() - 1];

    // Move all of them a step forward.
    for (auto ix = snake.size()-1; ix > 0; ix--)
    {
        auto& curr = snake[ix];
        auto prev = snake[ix - 1];

        curr = prev;
    }
    snake[0] = snake_vector{ snake[0].x + facing_dir.x, snake[0].y + facing_dir.y };

    if (bGrow)
        snake.push_back(tail);
}
