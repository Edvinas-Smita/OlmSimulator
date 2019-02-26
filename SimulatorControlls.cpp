#include "stdafx.h"
#include "Olm parts.h"

HWND ControllNorthUp, ControllNorthRight, ControllNorthDown, ControllNorthLeft;

HWND initControllArea(HWND hwnd)
{
	return CreateWindow(L"STATIC", NULL, WS_VISIBLE | WS_CHILD | SS_BLACKFRAME, 525, 0, 300, 525, hwnd, NULL, (HINSTANCE) GetWindowLong(hwnd, GWLP_HINSTANCE), NULL);	//divider
}

BOOL initCameraControlls(HWND hwnd, INT_PTR CALLBACK NU, INT_PTR CALLBACK NR, INT_PTR CALLBACK ND, INT_PTR CALLBACK NL)
{
	WNDCLASS wcNU = {0}, wcNR = {0}, wcND = {0}, wcNL = {0};
	UNREFERENCED_PARAMETER(NU);
	UNREFERENCED_PARAMETER(NR);
	UNREFERENCED_PARAMETER(ND);
	UNREFERENCED_PARAMETER(NL);
	ControllNorthUp = CreateWindow(
		L"BUTTON",
		L"NORTH",
		WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
		100,		// x position 
		0,			// y position 
		100,		// Button width
		100,		// Button height
		hwnd,		// Parent window
		NULL,		// No menu.
		(HINSTANCE) GetWindowLong(hwnd, GWLP_HINSTANCE),
		NULL
	);

	ControllNorthRight = CreateWindow(
		L"BUTTON",
		L"NORTH",
		WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
		200,		// x position 
		100,		// y position 
		100,		// Button width
		100,		// Button height
		hwnd,		// Parent window
		NULL,		// No menu.
		(HINSTANCE) GetWindowLong(hwnd, GWLP_HINSTANCE),
		NULL
	);

	ControllNorthDown = CreateWindow(
		L"BUTTON",
		L"NORTH",
		WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
		100,		// x position 
		200,		// y position 
		100,		// Button width
		100,		// Button height
		hwnd,		// Parent window
		NULL,		// No menu.
		(HINSTANCE) GetWindowLong(hwnd, GWLP_HINSTANCE),
		NULL
	);

	ControllNorthLeft = CreateWindow(
		L"BUTTON",
		L"NORTH",
		WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
		0,			// x position 
		100,		// y position 
		100,		// Button width
		100,		// Button height
		hwnd,		// Parent window
		NULL,		// No menu.
		(HINSTANCE) GetWindowLong(hwnd, GWLP_HINSTANCE),
		NULL
	);

	if (ControllNorthUp == nullptr || ControllNorthRight == nullptr || ControllNorthDown == nullptr || ControllNorthLeft == nullptr)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL initOlmPosControlls(HWND hwnd)
{

	return TRUE;
}

BOOL initOlmPhaseControlls(HWND hwnd)
{

	return TRUE;
}

BOOL initWeaponControlls(HWND hwnd)
{

	return TRUE;
}
