#include "stdafx.h"
#include "Stats.h"

HWND *gridHWNDS;
HWND *partHWNDS;

BOOL initOneField(HWND m_hwnd, BYTE index)
{
	WCHAR buffer[10];
	swprintf_s(buffer, 10, L"%d", index);
	HWND txt = CreateWindow(
		L"STATIC",
		buffer,
		WS_VISIBLE | WS_CHILD | WS_BORDER | SS_CENTER,  // Styles 
		0,			// x position
		300 + index * 20,	// y position 
		50,			// Button width
		20,			// Button height
		m_hwnd,		// Parent window
		(HMENU) (-1),
		(HINSTANCE) GetWindowLong(m_hwnd, GWLP_HINSTANCE),
		NULL
	);
	gridHWNDS[index] = CreateWindow(
		L"EDIT",
		L"0",
		WS_VISIBLE | WS_CHILD | WS_BORDER | WS_DISABLED | ES_CENTER,  // Styles 
		50,			// x position
		300 + index * 20,	// y position 
		125,		// Button width
		20,			// Button height
		m_hwnd,		// Parent window
		(HMENU) (STAT_FIELD_GRID | index),
		(HINSTANCE) GetWindowLong(m_hwnd, GWLP_HINSTANCE),
		NULL
	);
	partHWNDS[index] = CreateWindow(
		L"EDIT",
		L"0",
		WS_VISIBLE | WS_CHILD | WS_BORDER | WS_DISABLED | ES_CENTER,  // Styles 
		175,		// x position
		300 + index * 20,	// y position 
		125,		// Button width
		20,			// Button height
		m_hwnd,		// Parent window
		(HMENU) (STAT_FIELD_PART | index),
		(HINSTANCE) GetWindowLong(m_hwnd, GWLP_HINSTANCE),
		NULL
	);

	return txt != nullptr && gridHWNDS[index] != nullptr && partHWNDS[index] != nullptr;
}
BOOL initStatFields(HWND m_hwnd, BYTE playerCount)
{
	CreateWindow(
		L"STATIC",
		L"Player | Clicks on grid | Clicks on parts",
		WS_VISIBLE | WS_CHILD | WS_BORDER,  // Styles 
		0,			// x position
		275,	// y position 
		300,		// Button width
		25,			// Button height
		m_hwnd,		// Parent window
		(HMENU) (-1),
		(HINSTANCE) GetWindowLong(m_hwnd, GWLP_HINSTANCE),
		NULL
	);
	gridHWNDS = (HWND*) malloc(playerCount * sizeof(HWND));
	partHWNDS = (HWND*) malloc(playerCount * sizeof(HWND));
	for (BYTE i = 0; i < playerCount; i++)
	{
		if (!initOneField(m_hwnd, i))
		{
			return FALSE;
		}
	}
	return TRUE;
}

void updatePlayerStats(BYTE index, int gridClicks, int partClicks)
{
	WCHAR buffer[64];
	swprintf_s(buffer, 64, L"%d", gridClicks);
	SetWindowText(gridHWNDS[index], buffer);
	swprintf_s(buffer, 64, L"%d", partClicks);
	SetWindowText(partHWNDS[index], buffer);
}
