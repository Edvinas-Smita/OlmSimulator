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

//	Custom GUI stuff
HWND				main_hwnd, controlArea;
COLORREF			customColors[16];
Orientation			orientation = NorthUp;

//	Custom logic stuff
ClientData clientData;
SharedData sharedData;

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

			controlArea = initControlArea(hWnd);
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
			if (!initStatFields(controlArea, MAX_CONS))
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
				BYTE unpaintId = sharedData.playerPositions[clientData.thisClientID], clickedTile = wmId & 0xff;
				if (isPlayerConnected())
				{
					sendToServer(CLICK_TILE, clickedTile);
					break;
				}
			}
			if ((wmId & OLMPART_BUTTON) == OLMPART_BUTTON)
			{
				OlmPart clickedPart = (OlmPart) (wmId & 0xff);
				if (isPlayerConnected())
				{
					sendToServer(CLICK_PART, clickedPart);
				}
			}

			// Parse the menu selections:
			switch (wmId)
			{
				case MENU_HOST:
				{
					if (isServerRunning())
					{
						dwprintf(L"Server is already running!\n");
						break;
					}
					DialogBox(hInst, MAKEINTRESOURCE(DIALOG_HOST), hWnd, &hostServer);
					break;
				}
				case MENU_JOIN:
				{
					if (isPlayerConnected())
					{
						dwprintf(L"Client is already running!\n");
						break;
					}
					DialogBox(hInst, MAKEINTRESOURCE(DIALOG_JOIN), hWnd, &joinServer);
					break;
				}
				case MENU_STOP_SERVER:
				{
					if (isServerRunning())
					{
						stopServer();
					}
					break;
				}
				case MENU_STOP_CLIENT:
				{
					if (isServerRunning())
					{
						stopServer();
					}
					else if (isPlayerConnected())
					{
						stopClient();
						sharedData = {};

						redrawAllTiles();
						for (BYTE i = 0; i < MAX_CONS; i++)
						{
							updatePlayerStats(i, 0, 0);
						}
					}
					break;
				}
				case MENU_CHANGE_COLOR:
				{
					CHOOSECOLOR cc = {0};
					cc.lStructSize = sizeof(CHOOSECOLOR);
					cc.hwndOwner = hWnd;
					cc.lpCustColors = customColors;
					cc.Flags = CC_RGBINIT | CC_FULLOPEN;
					cc.rgbResult = sharedData.playerColors[clientData.thisClientID];
					ChooseColor(&cc);
					sendToServer(CHANGE_COLOR, cc.rgbResult);
					break;
				}
				default:
				{
					return DefWindowProc(hWnd, message, wParam, lParam);
				}
			}
			break;
		}
		case WM_MULTIPLAYER:
		{
			Command command = (Command) wParam;

			switch (command)
			{
				case PLAYER_JOIN:
				{
					ValueAndSenderID vsid = *(ValueAndSenderID*) lParam;
					free((ValueAndSenderID*) lParam);
					sharedData.playerCount++;
					BYTE player = vsid.sender;
					COLORREF clr;
					switch (player % 7)
					{
						case 0:
						{
							clr = RGB(255, 0, 0);
							break;
						}
						case 1:
						{
							clr = RGB(0, 255, 0);
							break;
						}
						case 2:
						{
							clr = RGB(0, 0, 255);
							break;
						}
						case 3:
						{
							clr = RGB(255, 255, 0);
							break;
						}
						case 4:
						{
							clr = RGB(255, 0, 255);
							break;
						}
						case 5:
						{
							clr = RGB(0, 255, 255);
							break;
						}
						case 6:
						{
							clr = RGB(255, 128, 0);
							break;
						}
					}
					sharedData.playerColors[player] = clr;
					sharedData.playerPositions[player] = 203;
					redrawTile(203);
					break;
				}
				case DATA_CATCHUP:
				{
					clientData = ((CatchupData*) lParam)->clientData;
					sharedData = ((CatchupData*) lParam)->sharedData;
					redrawAllTiles();
					for (BYTE i = 0; i < sharedData.playerCount; i++)
					{
						updatePlayerStats(i, sharedData.statsGrid[i], sharedData.statsParts[i]);
					}
					break;
				}
				case PLAYER_QUIT:
				{
					ValueAndSenderID vsid = *(ValueAndSenderID*) lParam;
					free((ValueAndSenderID*) lParam);
					sharedData.playerCount--;
					if (vsid.sender < clientData.thisClientID)
					{
						clientData.thisClientID--;
					}
					BYTE player = vsid.sender;
					redrawTile(sharedData.playerPositions[player]);
					COLORREF fillerClr = 0;
					BYTE fillerByte = 0;
					int fillerInt = 0;
					deleteArrayElementAndCollapse(sharedData.playerColors, player, sizeof(COLORREF), MAX_CONS, &fillerClr);
					deleteArrayElementAndCollapse(sharedData.playerPositions, player, sizeof(BYTE), MAX_CONS, &fillerByte);
					deleteArrayElementAndCollapse(sharedData.statsGrid, player, sizeof(int), MAX_CONS, &fillerInt);
					deleteArrayElementAndCollapse(sharedData.statsParts, player, sizeof(int), MAX_CONS, &fillerInt);
					for (BYTE i = player; i < sharedData.playerCount + 1; i++)
					{
						updatePlayerStats(i, sharedData.statsGrid[i], sharedData.statsParts[i]);
					}
					break;
				}
				case SERVER_CLOSED:
				{
					clientData = {};
					sharedData = {};
					for (BYTE i = 0; i < MAX_CONS; i++)
					{
						updatePlayerStats(i, 0, 0);
					}
					redrawAllTiles();
					stopClient(FALSE);
					break;
				}
				case CHANGE_COLOR:
				{
					ValueAndSenderID vsid = *(ValueAndSenderID*) lParam;
					free((ValueAndSenderID*) lParam);
					sharedData.playerColors[vsid.sender] = vsid.value;
					redrawTile(sharedData.playerPositions[vsid.sender]);
					break;
				}
				case CLICK_TILE:
				{
					ValueAndSenderID vsid = *(ValueAndSenderID*) lParam;
					free((ValueAndSenderID*) lParam);
					BYTE player = vsid.sender, tile = vsid.value;
					BYTE previousTile = sharedData.playerPositions[player];
					sharedData.playerPositions[player] = tile;

					redrawTwoTiles(tile, previousTile);
					
					sharedData.statsGrid[player]++;
					updatePlayerStats(player, sharedData.statsGrid[player], sharedData.statsParts[player]);
					break;
				}
				case CLICK_PART:
				{
					ValueAndSenderID vsid = *(ValueAndSenderID*) lParam;
					free((ValueAndSenderID*) lParam);
					BYTE player = vsid.sender, part = vsid.value;
					sharedData.statsParts[player]++;
					updatePlayerStats(player, sharedData.statsGrid[player], sharedData.statsParts[player]);
					break;
				}
				case CHANGE_WEAPON:
				{
					break;
				}
				case CHANGE_OLM_LOC:
				{
					ValueAndSenderID vsid = *(ValueAndSenderID*) lParam;
					free((ValueAndSenderID*) lParam);
					if (!rotateAllParts((OlmLocation) vsid.value, orientation))
					{
						DestroyWindow(main_hwnd);
						break;
					}
					RECT wrecked = {0, 0, 525, 525};
					RedrawWindow(main_hwnd, &wrecked, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
					break;
				}
				case TOGGLE_RUN:
				{
					break;
				}
				default:
				{
					break;
				}
			}
		}
		case WM_DRAWITEM:
		{
			int wmId = LOWORD(wParam);
			if ((wmId & GRID_BUTTON) == GRID_BUTTON)
			{
				BYTE playersOnThisTile[MAX_CONS];	//boolean array
				BYTE tile = wmId & 0xff, playerCountOnThisTile = 0, thisPlayerOnThisTile = FALSE;
				for (BYTE i = 0; i < sharedData.playerCount; i++)
				{
					if (tile == sharedData.playerPositions[i])
					{
						playersOnThisTile[playerCountOnThisTile++] = i;
						if (i == clientData.thisClientID)
						{
							thisPlayerOnThisTile = TRUE;
						}
					}
				}
				LPDRAWITEMSTRUCT lpDIS = (LPDRAWITEMSTRUCT) lParam;
				if (thisPlayerOnThisTile)
				{
					SetDCBrushColor(lpDIS->hDC, sharedData.playerColors[clientData.thisClientID]);
					SelectObject(lpDIS->hDC, GetStockObject(DC_BRUSH));
					RoundRect(lpDIS->hDC, lpDIS->rcItem.left, lpDIS->rcItem.top, lpDIS->rcItem.right, lpDIS->rcItem.bottom, 0, 0);
					if (playerCountOnThisTile > 1)
					{
						SetDCBrushColor(lpDIS->hDC, ~sharedData.playerColors[clientData.thisClientID]);
						WCHAR buffer[4];
						swprintf_s(buffer, L"%d", playerCountOnThisTile - 1);
						DrawTextW(lpDIS->hDC, buffer, -1, &(lpDIS->rcItem), DT_CENTER);
					}
				}
				else if (playerCountOnThisTile)
				{
					SetDCBrushColor(lpDIS->hDC, sharedData.playerColors[playersOnThisTile[0]]);
					SelectObject(lpDIS->hDC, GetStockObject(DC_BRUSH));
					RoundRect(lpDIS->hDC, lpDIS->rcItem.left, lpDIS->rcItem.top, lpDIS->rcItem.right, lpDIS->rcItem.bottom, 0, 0);
					if (playerCountOnThisTile > 1)
					{
						SetDCBrushColor(lpDIS->hDC, ~sharedData.playerColors[playersOnThisTile[0]]);
						WCHAR buffer[4];
						swprintf_s(buffer, L"%d", playerCountOnThisTile - 1);
						DrawTextW(lpDIS->hDC, buffer, -1, &(lpDIS->rcItem), DT_CENTER);
					}
				}
				else
				{
					SelectObject(lpDIS->hDC, GetStockObject(DC_BRUSH));
					RoundRect(lpDIS->hDC, lpDIS->rcItem.left, lpDIS->rcItem.top, lpDIS->rcItem.right, lpDIS->rcItem.bottom, 0, 0);
				}
			}
			break;
		}
		case WM_PAINT:	//not mine
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code that uses hdc here...
			EndPaint(hWnd, &ps);
			break;
		}
		case WM_DESTROY:
		{
			if (isServerRunning())
			{
				stopServer();
			}
			else if (isPlayerConnected())
			{
				stopClient();
			}
			PostQuitMessage(0);
			break;
		}
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
					if (!rotateAllParts(sharedData.olmLocation, orientation))
					{
						DestroyWindow(main_hwnd);
						break;
					}
					RECT wrecked = {0, 0, 525, 525};
					RedrawWindow(main_hwnd, &wrecked, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
				}
				else if ((wpl & OLMPOS_CONTROL) == OLMPOS_CONTROL)
				{
					if (isPlayerConnected())
					{
						sharedData.olmLocation = (OlmLocation) ((wpl & (~CONTROL_BUTTON)) & (~OLMPOS_CONTROL));
						sendToServer(CHANGE_OLM_LOC, sharedData.olmLocation);
					}
					else
					{
						sharedData.olmLocation = (OlmLocation) ((wpl & (~CONTROL_BUTTON)) & (~OLMPOS_CONTROL));
						if (!rotateAllParts(sharedData.olmLocation, orientation))
						{
							DestroyWindow(main_hwnd);
							break;
						}
						RECT wrecked = {0, 0, 525, 525};
						RedrawWindow(main_hwnd, &wrecked, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
						break;
					}
				}
			}
			break;
		}
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
					WCHAR IPstr[16], PORTstr[8], PASSstr[40];
					HWND editIP = GetDlgItem(hDlg, EDIT_IP), editPort = GetDlgItem(hDlg, EDIT_PORT), editPass = GetDlgItem(hDlg, EDIT_PASSWORD);
					GetWindowText(editIP, IPstr, 16);
					GetWindowText(editPort, PORTstr, 8);
					GetWindowText(editPass, PASSstr, 40);
					//dwprintf(L"HOST %s:%d - %s\n", IPstr, _tstoi(PORTstr), PASSstr);
					WSA();
					int err = startServer(_tstoi(PORTstr), PASSstr, &sharedData);
					if (err)
					{
						dwprintf(L"_____START SERVER FAILED______ ERR: %d\n", err);
					}
					err = startClient((HWND) GetWindowLongPtr(hDlg, GWLP_HWNDPARENT), IPstr, _tstoi(PORTstr), PASSstr);
					if (err)
					{
						dwprintf(L"_____START CLIENT FAILED______ ERR: %d\n", err);
					}
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
					WCHAR IPstr[16], PORTstr[8], PASSstr[40];
					HWND editIP = GetDlgItem(hDlg, EDIT_IP), editPort = GetDlgItem(hDlg, EDIT_PORT), editPass = GetDlgItem(hDlg, EDIT_PASSWORD);
					GetWindowText(editIP, IPstr, 16);
					GetWindowText(editPort, PORTstr, 8);
					GetWindowText(editPass, PASSstr, 40);
					//dwprintf(L"JOIN: %s:%d - %s\n", IPstr, _tstoi(PORTstr), PASSstr);
					WSA();
					int err = startClient((HWND) GetWindowLongPtr(hDlg, GWLP_HWNDPARENT), IPstr, _tstoi(PORTstr), PASSstr);
					if (err)
					{
						dwprintf(L"_____START CLIENT FAILED______ ERR: %d\n", err);
					}
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
