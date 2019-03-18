// Olm Simulator.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Olm Simulator.h"

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
INT_PTR CALLBACK    hostServer(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK    joinServer(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK    changeColor(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

//	Custom GUI stuff
HWND				main_hwnd;
WNDPROC				old_control;

//	Custom logic stuff
Orientation			orientation = NorthUp;
OlmLocation			olmLocation = West;

COLORREF			allPlayerColors[MAX_CONS] = {RGB(255, 0, 0)};
BYTE				allPlayerPositions[MAX_CONS] = {203};

//	Custom shithole
unsigned char		unpaintId = -1;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
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
	wcex.lpszMenuName	= MAKEINTRESOURCE(MAIN_MENU);
    wcex.lpszClassName  = szWindowClass;

    return RegisterClassExW(&wcex);
}

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
			if (!initOlmPosControls(controlArea))
			{
				DestroyWindow(hWnd);
				break;
			}

			SetWindowSubclass(controlArea, (SUBCLASSPROC) &ControlHandler, 1, 0);

			return DefWindowProc(hWnd, message, wParam, lParam);
		}
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);

			if ((wmId & GRID_BUTTON) == GRID_BUTTON)
			{
				unpaintId = allPlayerPositions[getThisClientID()];
				allPlayerPositions[getThisClientID()] = wmId & (~GRID_BUTTON);						////////////////////////////////////////////////////	BUTTON CLICKS HERE
				if (!redrawTile(allPlayerPositions[getThisClientID()], unpaintId))
				{
					DestroyWindow(hWnd);
					break;
				}
			}
			if ((wmId & OLMPART_BUTTON) == OLMPART_BUTTON)
			{
				OlmPart clickedPart = (OlmPart) (wmId & (~OLMPART_BUTTON));
			}

            // Parse the menu selections:
            switch (wmId)
            {
				case MENU_HOST:
					DialogBox(hInst, MAKEINTRESOURCE(DIALOG_HOST), hWnd, &hostServer);
					break;
				case MENU_JOIN:
					DialogBox(hInst, MAKEINTRESOURCE(DIALOG_JOIN), hWnd, &joinServer);
					break;
				case MENU_CHANGE_COLOR:
					DialogBox(hInst, MAKEINTRESOURCE(DIALOG_COLOR), hWnd, &changeColor);
					break;
				default:
					return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
	case WM_DRAWITEM:
	{
		if (wParam == (GRID_BUTTON | allPlayerPositions[getThisClientID()]))
		{
			LPDRAWITEMSTRUCT lpDIS = (LPDRAWITEMSTRUCT) lParam;
			SetDCBrushColor(lpDIS->hDC, allPlayerColors[getThisClientID()]);
			SelectObject(lpDIS->hDC, GetStockObject(DC_BRUSH));
			RoundRect(lpDIS->hDC, lpDIS->rcItem.left, lpDIS->rcItem.top, lpDIS->rcItem.right, lpDIS->rcItem.bottom, 0, 0);
		}/*
		else if (wParam == (GRID_BUTTON | unpaintId))
		{

		}*/
		else
		{
			LPDRAWITEMSTRUCT lpDIS = (LPDRAWITEMSTRUCT) lParam;
			SelectObject(lpDIS->hDC, GetStockObject(DC_BRUSH));
			RoundRect(lpDIS->hDC, lpDIS->rcItem.left, lpDIS->rcItem.top, lpDIS->rcItem.right, lpDIS->rcItem.bottom, 0, 0);
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
					orientation = (Orientation) ((wpl & (~CONTROL_BUTTON)) & (~CAMERA_CONTROL));
					if (!rotateGrid(orientation))
					{
						DestroyWindow(main_hwnd);
						break;
					}
					if (!rotateAllParts(olmLocation, orientation))
					{
						DestroyWindow(main_hwnd);
						break;
					}
					RECT wrecked = {0, 0, 525, 525};
					RedrawWindow(main_hwnd, &wrecked, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
				}
				else if ((wpl & OLMPOS_CONTROL) == OLMPOS_CONTROL)
				{
					olmLocation = (OlmLocation) ((wpl & (~CONTROL_BUTTON)) & (~OLMPOS_CONTROL));
					if (!rotateAllParts(olmLocation, orientation))
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
}

BOOL changedByCode = FALSE;
INT_PTR CALLBACK hostServer(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
		case WM_INITDIALOG:
		{
			changedByCode = TRUE;
			HWND editPort = GetDlgItem(hDlg, EDIT_PORT);
			SetWindowText(editPort, L"27015");
			return (INT_PTR) TRUE;
		}

		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case EDIT_PORT:
				{
					switch (HIWORD(wParam))
					{
						case EN_UPDATE:
						{
							if (changedByCode)
							{
								changedByCode = FALSE;
								break;
							}
							WCHAR buff[8];
							HWND editPort = GetDlgItem(hDlg, EDIT_PORT);
							GetWindowText(editPort, buff, 8);
							int portint = _tstoi(buff);
							if (portint >> 16 && !changedByCode)
							{
								changedByCode = TRUE;
								SetWindowText(editPort, L"65535");
							}
							break;
						}
						default:
							break;
					}
					break;
				}
				case IDOK:
				{
					WCHAR buffer[128], PORTstr[8], PASSstr[40];
					HWND editPort = GetDlgItem(hDlg, EDIT_PORT), editPass = GetDlgItem(hDlg, EDIT_PASSWORD);
					GetWindowText(editPort, PORTstr, 8);
					GetWindowText(editPass, PASSstr, 40);
					swprintf_s(buffer, L"HOST:%d - %s\n", _tstoi(PORTstr), PASSstr);
					OutputDebugString(buffer);
					EndDialog(hDlg, LOWORD(wParam));
					return (INT_PTR) TRUE;
				}
				case IDCANCEL:
				{
					EndDialog(hDlg, LOWORD(wParam));
					return (INT_PTR) TRUE;
				}
				default:
					break;
			}
			break;
		}
		default:
			break;
	}
	return (INT_PTR) FALSE;
}

INT_PTR CALLBACK joinServer(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
		case WM_INITDIALOG:
		{
			changedByCode = TRUE;
			HWND editIP = GetDlgItem(hDlg, EDIT_IP), editPort = GetDlgItem(hDlg, EDIT_PORT);
			SetWindowText(editIP, L"127.0.0.1");
			SetWindowText(editPort, L"27015");
			return (INT_PTR) TRUE;
		}

		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case EDIT_PORT:
				{
					switch (HIWORD(wParam))
					{
						case EN_UPDATE:
						{
							if (changedByCode)
							{
								changedByCode = FALSE;
								break;
							}
							WCHAR buff[8];
							HWND editPort = GetDlgItem(hDlg, EDIT_PORT);
							GetWindowText(editPort, buff, 8);
							int portint = _tstoi(buff);
							if (portint >> 16 && !changedByCode)
							{
								changedByCode = TRUE;
								SetWindowText(editPort, L"65535");
							}
							break;
						}
						default:
							break;
					}
					break;
				}
				case IDOK:
				{
					WCHAR buffer[128], IPstr[16], PORTstr[8], PASSstr[40];
					HWND editIP = GetDlgItem(hDlg, EDIT_IP), editPort = GetDlgItem(hDlg, EDIT_PORT), editPass = GetDlgItem(hDlg, EDIT_PASSWORD);
					GetWindowText(editIP, IPstr, 16);
					GetWindowText(editPort, PORTstr, 8);
					GetWindowText(editPass, PASSstr, 40);
					swprintf_s(buffer, L"JOIN: %s:%d - %s\n", IPstr, _tstoi(PORTstr), PASSstr);
					OutputDebugString(buffer);
					EndDialog(hDlg, LOWORD(wParam));
					return (INT_PTR) TRUE;
				}
				case IDCANCEL:
				{
					EndDialog(hDlg, LOWORD(wParam));
					return (INT_PTR) TRUE;
				}
				default:
					break;
			}
			break;
		}
		default:
			break;
	}
	return (INT_PTR) FALSE;
}

INT_PTR CALLBACK changeColor(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
		case WM_INITDIALOG:
		{
			changedByCode = TRUE;
			HWND red = GetDlgItem(hDlg, COLOR_RED), green = GetDlgItem(hDlg, COLOR_GREEN), blue = GetDlgItem(hDlg, COLOR_BLUE);
			WCHAR REDstr[4], GREENstr[4], BLUEstr[4];
			COLORREF current = allPlayerColors[getThisClientID()];
			_itow_s((BYTE) (current >> 0), REDstr, 10);
			_itow_s((BYTE) (current >> 8), GREENstr, 10);
			_itow_s((BYTE) (current >> 16), BLUEstr, 10);
			SetWindowText(red, REDstr);
			SetWindowText(green, GREENstr);
			SetWindowText(blue, BLUEstr);
			return (INT_PTR) TRUE;
		}

		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case COLOR_RED:
				case COLOR_GREEN:
				case COLOR_BLUE:
				{
					switch (HIWORD(wParam))
					{
						case EN_UPDATE:
						{
							if (changedByCode)
							{
								changedByCode = FALSE;
								break;
							}
							HWND red = GetDlgItem(hDlg, COLOR_RED), green = GetDlgItem(hDlg, COLOR_GREEN), blue = GetDlgItem(hDlg, COLOR_BLUE);
							WCHAR REDstr[16], GREENstr[16], BLUEstr[16];
							GetWindowText(red, REDstr, 4);
							GetWindowText(green, GREENstr, 4);
							GetWindowText(blue, BLUEstr, 4);
							int r = _tstoi(REDstr), g = _tstoi(GREENstr), b = _tstoi(BLUEstr);
							if (!changedByCode)
							{
								if (r >> 8)
								{
									changedByCode = TRUE;
									SetWindowText(red, L"255");
								}
								if (g >> 8)
								{
									changedByCode = TRUE;
									SetWindowText(green, L"255");
								}
								if (b >> 8)
								{
									changedByCode = TRUE;
									SetWindowText(blue, L"255");
								}
							}
							break;
						}
						default:
							break;
					}
					break;
				}
				case IDOK:
				{
					HWND red = GetDlgItem(hDlg, COLOR_RED), green = GetDlgItem(hDlg, COLOR_GREEN), blue = GetDlgItem(hDlg, COLOR_BLUE);
					WCHAR REDstr[16], GREENstr[16], BLUEstr[16];
					GetWindowText(red, REDstr, 4);
					GetWindowText(green, GREENstr, 4);
					GetWindowText(blue, BLUEstr, 4);
					short r = _tstoi(REDstr), g = _tstoi(GREENstr), b = _tstoi(BLUEstr);
					allPlayerColors[getThisClientID()] = RGB(r, g, b);
					EndDialog(hDlg, LOWORD(wParam));
					return (INT_PTR) TRUE;
				}
				case IDCANCEL:
				{
					EndDialog(hDlg, LOWORD(wParam));
					return (INT_PTR) TRUE;
				}
				default:
					break;
			}
			break;
		}
		default:
			break;
	}
	return (INT_PTR) FALSE;
}
