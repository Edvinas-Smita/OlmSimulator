// Olm Simulator.cpp : Defines the entry point for the application.
//
#pragma comment(lib, "comctl32.lib")

#include "stdafx.h"
#include "Olm Simulator.h"
#include <CommCtrl.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    ControlHandler(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);

HWND main_hwnd;
WNDPROC old_control;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_OLMSIMULATOR, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_OLMSIMULATOR));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
    wcex.lpszClassName  = szWindowClass;

    return RegisterClassExW(&wcex);
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

   HWND hWnd = CreateWindowW(
	   szWindowClass,
	   szTitle,
	   WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,
	   CW_USEDEFAULT,
	   CW_USEDEFAULT,
	   541 + 301,	//21 * 25 + 16 (borders)
	   564,	//21 * 25 + 8 (bottom border) + 31 (top border)
	   nullptr,
	   nullptr,
	   hInstance,
	   nullptr
   );

   if (!hWnd)
   {
      return FALSE;
   }

   main_hwnd = hWnd;

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
    switch (message)
    {
		case WM_CREATE:
		{
			if (!initParts(hWnd))
			{
				DestroyWindow(hWnd);
				break;
			}
			if (!initGrid(hWnd))
			{
				DestroyWindow(hWnd);
				break;
			}

			HWND controlArea = initControlArea(hWnd);
			if (controlArea == nullptr)
			{
				DestroyWindow(hWnd);
				break;
			}
			if (!initCameraControls(controlArea))
			{
				DestroyWindow(hWnd);
				break;
			}

			SetWindowSubclass(controlArea, (SUBCLASSPROC) &ControlHandler, 1, 0);
			/*LONG_PTR newP = (LONG_PTR) &ControlHandler;
			old_control = (WNDPROC) SetWindowLongPtr(controlArea, GCLP_WNDPROC, newP);
			SendMessage(controlArea, 0, 0, 0);

			{
				WCHAR buff[256];
				swprintf_s(buff, L"old: %p | new: %p | actual: %p\n", old_control, newP, GetWindowLongPtr(controlArea, GCLP_WNDPROC));
				OutputDebugString(buff);
			}*/

			return DefWindowProc(hWnd, message, wParam, lParam);
		}
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);

			if ((wmId & GRID_BUTTON) == GRID_BUTTON)
			{
				int buttonId = wmId & (~GRID_BUTTON);						////////////////////////////////////////////////////	BUTTON CLICKS HERE
				WCHAR buffer[32];
				swprintf_s(buffer, L"Button NR: %d\n", buttonId);
				OutputDebugString(buffer);
			}

            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                //DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK ControlHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	switch (msg)
	{
		case WM_COMMAND:
		{
			int wpl = LOWORD(wp);
			if ((wpl & CONTROL_BUTTON) == CONTROL_BUTTON)
			{
				if ((wpl & CAMERA_CONTROL) == CAMERA_CONTROL)
				{
					Orientation o = (Orientation) ((wpl & (~CONTROL_BUTTON)) & (~CAMERA_CONTROL));
					if (!rotateGrid(o))
					{
						DestroyWindow(main_hwnd);
						break;
					}
					if (!rotateParts(West, o))
					{
						DestroyWindow(main_hwnd);
						break;
					}
					RECT wrecked = {0, 0, 525, 525};
					RedrawWindow(main_hwnd, &wrecked, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
				}
			}
		}
		break;
		default:
			break;
	}
	return DefSubclassProc(hwnd, msg, wp, lp);
	//return CallWindowProc(old_control, hwnd, msg, wp, lp);
}

// Message handler for about box.
/*INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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
}*/
