#include "stdafx.h"
#include "Grid.h"

#define GRID_TILE_COUNT 11*19

HWND *grid = new HWND[GRID_TILE_COUNT];	//0 === Top Left; GRID_TILE_COUNT - 1 === Bottom Right	//this is NOT transformed - north is up

HWND createTile(HWND m_hwnd, char x, char y)
{
	return CreateWindow(
		L"BUTTON",
		L"",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
		x*25,		// x position
		y*25,		// y position 
		25,			// Button width
		25,			// Button height
		m_hwnd,		// Parent window
		NULL,		// No menu.
		(HINSTANCE) GetWindowLong(m_hwnd, GWL_HINSTANCE),
		NULL
	);
}

BOOL initGrid(HWND m_hwnd, Orientation o)
{
	switch (o)
	{
		case Orientation::NorthUp:
			for (int x = 5; x < 16; x++)	//main room
			{
				for (int y = 1; y < 19; y++)
				{
					HWND toSave = createTile(m_hwnd, x, y);
					if (toSave == nullptr)
					{
						return FALSE;
					}
					grid[x + y * 21] = toSave;
				}
			}

			for (int i = 8; i < 13; i++)	//door tiles
			{
				HWND toSave = createTile(m_hwnd, i, 19);
				if (toSave == nullptr)
				{
					return FALSE;
				}
				grid[19 * 21 + i] = toSave;
			}
			return TRUE;
		case Orientation::NorthRight:
			for (int x = 2; x < 20; x++)	//main room
			{
				for (int y = 5; y < 16; y++)
				{
					HWND toSave = createTile(m_hwnd, x, y);
					if (toSave == nullptr)
					{
						return FALSE;
					}
					grid[x + y * 21] = toSave;
				}
			}

			for (int i = 8; i < 13; i++)	//door tiles
			{
				HWND toSave = createTile(m_hwnd, 1, i);
				if (toSave == nullptr)
				{
					return FALSE;
				}
				grid[i * 21 + 1] = toSave;
			}
			return TRUE;
		case Orientation::NorthDown:
			for (int x = 5; x < 16; x++)	//main room
			{
				for (int y = 2; y < 20; y++)
				{
					HWND toSave = createTile(m_hwnd, x, y);
					if (toSave == nullptr)
					{
						return FALSE;
					}
					grid[x + y * 21] = toSave;
				}
			}

			for (int i = 8; i < 13; i++)	//door tiles
			{
				HWND toSave = createTile(m_hwnd, i, 1);
				if (toSave == nullptr)
				{
					return FALSE;
				}
				grid[i] = toSave;
			}
			return TRUE;
		case Orientation::NorthLeft:
			for (int x = 1; x < 19; x++)	//main room
			{
				for (int y = 5; y < 16; y++)
				{
					HWND toSave = createTile(m_hwnd, x, y);
					if (toSave == nullptr)
					{
						return FALSE;
					}
					grid[x + y * 21] = toSave;
				}
			}

			for (int i = 8; i < 13; i++)	//door tiles
			{
				HWND toSave = createTile(m_hwnd, 19, i);
				if (toSave == nullptr)
				{
					return FALSE;
				}
				grid[i * 21 + 18] = toSave;
			}
			return TRUE;
		default:
			return FALSE;
	}
}