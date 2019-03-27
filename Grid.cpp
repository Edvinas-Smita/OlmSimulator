#include "stdafx.h"
#include "Grid.h"

#define GRID_TILE_COUNT 11*19

HWND grid[GRID_TILE_COUNT];	//0 === Top Left; GRID_TILE_COUNT - 1 === Bottom Right
							//this is NOT transformed - north is up
							//[y * 11 + x] - flattened horizontal first

HWND createTile(HWND m_hwnd, int id)
{
	WCHAR buffer[8];
	swprintf_s(buffer, L"%d\n", id);

	int x = (id % 11 + 5) * 25, y = (id / 11 + 1) * 25;
	return CreateWindow(
		L"BUTTON",
		buffer,
		WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_OWNERDRAW,  // Styles 
		x,			// x position in window
		y,			// y position in window
		25,			// Button width
		25,			// Button height
		m_hwnd,		// Parent window
		(HMENU) (GRID_BUTTON | id),
		(HINSTANCE) GetWindowLong(m_hwnd, GWLP_HINSTANCE),
		NULL
	);
}

BOOL initGrid(HWND m_hwnd)
{
	for (int i = 0; i < GRID_TILE_COUNT; i++)
	{
		if ((grid[i] = createTile(m_hwnd, i)) == nullptr)
		{
			return FALSE;
		}
	}
	if (!DestroyWindow(grid[208]) || !DestroyWindow(grid[207]) || !DestroyWindow(grid[206]) || !DestroyWindow(grid[200]) || !DestroyWindow(grid[199]) || !DestroyWindow(grid[198]))
	{
		return FALSE;
	}
	grid[208] = grid[207] = grid[206] = grid[200] = grid[199] = grid[198] = nullptr;
	return TRUE;
}

BOOL rotateGrid(Orientation o)
{
	unsigned char id, xx, yy;
	switch (o)
	{
		case NorthUp:
			for (int y = 0; y < 19; y++)
			{
				for (int x = 0; x < 11; x++)
				{
					xx = x + 5;
					yy = y + 1;
					id = y * 11 + x;
					if (grid[id] != nullptr)
					{
						if (!MoveWindow(grid[id], xx * 25, yy * 25, 25, 25, 0))
						{
							return FALSE;
						}
					}
				}
			}
			break;
		case NorthRight:
			for (int y = 0; y < 19; y++)
			{
				for (int x = 0; x < 11; x++)
				{
					xx = 19 - y;
					yy = x + 5;
					id = y * 11 + x;
					if (grid[id] != nullptr)
					{
						if (!MoveWindow(grid[id], xx * 25, yy * 25, 25, 25, 0))
						{
							return FALSE;
						}
					}
				}
			}
			break;
		case NorthDown:
			for (int y = 0; y < 19; y++)
			{
				for (int x = 0; x < 11; x++)
				{
					xx = 15 - x;
					yy = 19 - y;
					id = y * 11 + x;
					if (grid[id] != nullptr)
					{
						if (!MoveWindow(grid[id], xx * 25, yy * 25, 25, 25, 0))
						{
							return FALSE;
						}
					}
				}
			}
			break;
		case NorthLeft:
			for (int y = 0; y < 19; y++)
			{
				for (int x = 0; x < 11; x++)
				{
					xx = y + 1;
					yy = 15 - x;
					id = y * 11 + x;
					if (grid[id] != nullptr)
					{
						if (!MoveWindow(grid[id], xx * 25, yy * 25, 25, 25, 0))
						{
							return FALSE;
						}
					}
				}
			}
			break;
		default:
			break;
	}
	return TRUE;
}

BOOL redrawTile(BYTE index)
{
	if (index >= GRID_TILE_COUNT)
	{
		return FALSE;
	}
	return RedrawWindow(grid[index], nullptr, NULL, RDW_INVALIDATE | RDW_ERASE);
}
BOOL redrawTwoTiles(BYTE newPos, BYTE oldPos)
{
	if (newPos >= GRID_TILE_COUNT || oldPos >= GRID_TILE_COUNT)
	{
		return FALSE;
	}
	return redrawTile(newPos) && redrawTile(oldPos);
}
BOOL redrawAllTiles()
{
	for (BYTE i = 0; i < GRID_TILE_COUNT; i++)
	{
		if (grid[i] != nullptr)
		{
			if (!redrawTile(i))
			{
				return FALSE;
			}
		}
	}
	return TRUE;
}
