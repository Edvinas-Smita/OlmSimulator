#include "stdafx.h"
#include "Olm Simulator.h"

HWND ControlNorthUp = nullptr, ControlNorthRight = nullptr, ControlNorthDown = nullptr, ControlNorthLeft = nullptr;

HWND ControlOlmWest = nullptr, ControlOlmEast = nullptr;

HWND initControlArea(HWND hwnd)
{
	return CreateWindow(L"STATIC", NULL, WS_VISIBLE | WS_CHILD | SS_BLACKFRAME, 525, 0, 300, 525, hwnd, NULL, (HINSTANCE) GetWindowLong(hwnd, GWLP_HINSTANCE), NULL);
}

BOOL initCameraControls(HWND hwnd)
{
	ControlNorthUp = CreateWindow(
		L"BUTTON",
		L"NORTH",
		WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
		100,		// x position 
		0,			// y position 
		100,		// Button width
		100,		// Button height
		hwnd,		// Parent window
		(HMENU) (CONTROL_BUTTON | CAMERA_CONTROL | Orientation::NorthUp),
		(HINSTANCE) GetWindowLong(hwnd, GWLP_HINSTANCE),
		NULL
	);

	ControlNorthRight = CreateWindow(
		L"BUTTON",
		L"NORTH",
		WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
		200,		// x position 
		50,		// y position 
		100,		// Button width
		100,		// Button height
		hwnd,		// Parent window
		(HMENU) (CONTROL_BUTTON | CAMERA_CONTROL | Orientation::NorthRight),
		(HINSTANCE) GetWindowLong(hwnd, GWLP_HINSTANCE),
		NULL
	);

	ControlNorthDown = CreateWindow(
		L"BUTTON",
		L"NORTH",
		WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
		100,		// x position 
		100,		// y position 
		100,		// Button width
		100,		// Button height
		hwnd,		// Parent window
		(HMENU) (CONTROL_BUTTON | CAMERA_CONTROL | Orientation::NorthDown),
		(HINSTANCE) GetWindowLong(hwnd, GWLP_HINSTANCE),
		NULL
	);

	ControlNorthLeft = CreateWindow(
		L"BUTTON",
		L"NORTH",
		WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
		0,			// x position 
		50,		// y position 
		100,		// Button width
		100,		// Button height
		hwnd,		// Parent window
		(HMENU) (CONTROL_BUTTON | CAMERA_CONTROL | Orientation::NorthLeft),
		(HINSTANCE) GetWindowLong(hwnd, GWLP_HINSTANCE),
		NULL
	);

	if (ControlNorthUp == nullptr || ControlNorthRight == nullptr || ControlNorthDown == nullptr || ControlNorthLeft == nullptr)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL initOlmPosControls(HWND hwnd)
{
	ControlOlmWest = CreateWindow(
		L"BUTTON",
		L"<--- Olm West",
		WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
		0,			// x position 
		200,		// y position 
		150,		// Button width
		50,			// Button height
		hwnd,		// Parent window
		(HMENU) (CONTROL_BUTTON | OLMPOS_CONTROL | OlmLocation::West),
		(HINSTANCE) GetWindowLong(hwnd, GWLP_HINSTANCE),
		NULL
	);

	ControlOlmEast = CreateWindow(
		L"BUTTON",
		L"Olm East --->",
		WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
		150,		// x position 
		200,		// y position 
		150,		// Button width
		50,			// Button height
		hwnd,		// Parent window
		(HMENU) (CONTROL_BUTTON | OLMPOS_CONTROL | OlmLocation::East),
		(HINSTANCE) GetWindowLong(hwnd, GWLP_HINSTANCE),
		NULL
	);

	if (ControlOlmWest == nullptr || ControlOlmEast == nullptr)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL initOlmPhaseControls(HWND hwnd)
{

	return TRUE;
}

BOOL initWeaponControls(HWND hwnd)
{

	return TRUE;
}
