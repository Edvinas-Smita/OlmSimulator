#include "stdafx.h"
#include "Grid.h"

#define GRID_TILE_COUNT 11*19

HWND grid[11*19];	//0 === Top Left; GRID_TILE_COUNT - 1 === Bottom Right
					//this is NOT transformed - north is up
					//[y * 11 + x] - flattened horizontal first

HWND createTile(HWND m_hwnd, char x, char y, int xy)
{
	WCHAR buffer[32];
	swprintf_s(buffer, L"%d\n", xy);
	return CreateWindowW(
		L"BUTTON",
		buffer,
		WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
		x * 25,		// x position in window
		y * 25,		// y position in window
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
			for (int y = 0; y < 18; y++)	//main room
			{
				for (int x = 0; x < 11; x++)
				{
					HWND toSave = createTile(m_hwnd, x + 5, y + 1, y * 11 + x);
					if (toSave == nullptr)
					{
						return FALSE;
					}
					grid[y * 11 + x] = toSave;
				}
			}

			for (int i = 0; i < 5; i++)	//door tiles
			{
				HWND toSave = createTile(m_hwnd, i + 8, 19, i);
				if (toSave == nullptr)
				{
					return FALSE;
				}
				grid[18 * 11 + i + 3] = toSave;
			}
			//for (int i = 0; i < 11 * 19; i++)
			//{
			//	int xx = (i % 11) * 25, yy = (i / 11) * 25;
			//	if (grid[i] != nullptr)
			//		MoveWindow(grid[i], xx, yy, 25, 25, 0);
			//}
			return TRUE;
		case Orientation::NorthRight:
			for (int y = 0; y < 18; y++)	//main room
			{
				for (int x = 0; x < 11; x++)
				{
					HWND toSave = createTile(m_hwnd, 19 - y, x + 5, y * 11 + x);
					if (toSave == nullptr)
					{
						return FALSE;
					}
					grid[y * 11 + x] = toSave;
				}
			}

			for (int i = 0; i < 5; i++)	//door tiles
			{
				HWND toSave = createTile(m_hwnd, 1, i + 8, i);
				if (toSave == nullptr)
				{
					return FALSE;
				}
				grid[18 * 11 + i + 3] = toSave;
			}
			return TRUE;
		case Orientation::NorthDown:
			for (int y = 0; y < 18; y++)	//main room
			{
				for (int x = 0; x < 11; x++)
				{
					HWND toSave = createTile(m_hwnd, 15 - x, 19 - y, y * 11 + x);
					if (toSave == nullptr)
					{
						return FALSE;
					}
					grid[y * 11 + x] = toSave;
				}
			}

			for (int i = 0; i < 5; i++)	//door tiles
			{
				HWND toSave = createTile(m_hwnd, 12 - i, 1, i);
				if (toSave == nullptr)
				{
					return FALSE;
				}
				grid[18 * 11 + i + 3] = toSave;
			}
			return TRUE;
		case Orientation::NorthLeft:
			for (int y = 0; y < 18; y++)	//main room
			{
				for (int x = 0; x < 11; x++)
				{
					HWND toSave = createTile(m_hwnd, y + 1, 15 - x, y * 11 + x);
					if (toSave == nullptr)
					{
						return FALSE;
					}
					grid[y * 11 + x] = toSave;
				}
			}

			for (int i = 0; i < 5; i++)	//door tiles
			{
				HWND toSave = createTile(m_hwnd, 19, 12 - i, i);
				if (toSave == nullptr)
				{
					return FALSE;
				}
				grid[18 * 11 + i + 3] = toSave;
			}
			return TRUE;
		default:
			return FALSE;
	}
}