#include "stdafx.h"
#include "Olm Simulator.h"

#define GRID_TILE_COUNT 11*19

HWND grid[11*19];	//0 === Top Left; GRID_TILE_COUNT - 1 === Bottom Right
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
		WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
		x,			// x position in window
		y,			// y position in window
		25,			// Button width
		25,			// Button height
		m_hwnd,		// Parent window
		(HMENU) (GRID_BUTTON | id),		// No menu.
		(HINSTANCE) GetWindowLong(m_hwnd, GWLP_HINSTANCE),
		NULL
	);
}

BOOL initGrid(HWND m_hwnd)
{
	for (int i = 0; i < 11 * 19; i++)
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

	//switch (o)
	//{
	//	case Orientation::NorthUp:
	//		for (int y = 0; y < 18; y++)	//main room
	//		{
	//			for (int x = 0; x < 11; x++)
	//			{
	//				HWND toSave = createTile(m_hwnd, x + 5, y + 1, y * 11 + x);
	//				if (toSave == nullptr)
	//				{
	//					return FALSE;
	//				}
	//				grid[y * 11 + x] = toSave;
	//			}
	//		}

	//		for (int i = 0; i < 5; i++)	//door tiles
	//		{
	//			HWND toSave = createTile(m_hwnd, i + 8, 19, i);
	//			if (toSave == nullptr)
	//			{
	//				return FALSE;
	//			}
	//			grid[18 * 11 + i + 3] = toSave;
	//		}
	//		return TRUE;
	//	case Orientation::NorthRight:
	//		for (int y = 0; y < 18; y++)	//main room
	//		{
	//			for (int x = 0; x < 11; x++)
	//			{
	//				HWND toSave = createTile(m_hwnd, 19 - y, x + 5, y * 11 + x);
	//				if (toSave == nullptr)
	//				{
	//					return FALSE;
	//				}
	//				grid[y * 11 + x] = toSave;
	//			}
	//		}

	//		for (int i = 0; i < 5; i++)	//door tiles
	//		{
	//			HWND toSave = createTile(m_hwnd, 1, i + 8, i);
	//			if (toSave == nullptr)
	//			{
	//				return FALSE;
	//			}
	//			grid[18 * 11 + i + 3] = toSave;
	//		}
	//		return TRUE;
	//	case Orientation::NorthDown:
	//		for (int y = 0; y < 18; y++)	//main room
	//		{
	//			for (int x = 0; x < 11; x++)
	//			{
	//				HWND toSave = createTile(m_hwnd, 15 - x, 19 - y, y * 11 + x);
	//				if (toSave == nullptr)
	//				{
	//					return FALSE;
	//				}
	//				grid[y * 11 + x] = toSave;
	//			}
	//		}

	//		for (int i = 0; i < 5; i++)	//door tiles
	//		{
	//			HWND toSave = createTile(m_hwnd, 12 - i, 1, i);
	//			if (toSave == nullptr)
	//			{
	//				return FALSE;
	//			}
	//			grid[18 * 11 + i + 3] = toSave;
	//		}
	//		return TRUE;
	//	case Orientation::NorthLeft:
	//		for (int y = 0; y < 18; y++)	//main room
	//		{
	//			for (int x = 0; x < 11; x++)
	//			{
	//				HWND toSave = createTile(m_hwnd, y + 1, 15 - x, y * 11 + x);
	//				if (toSave == nullptr)
	//				{
	//					return FALSE;
	//				}
	//				grid[y * 11 + x] = toSave;
	//			}
	//		}

	//		for (int i = 0; i < 5; i++)	//door tiles
	//		{
	//			HWND toSave = createTile(m_hwnd, 19, 12 - i, i);
	//			if (toSave == nullptr)
	//			{
	//				return FALSE;
	//			}
	//			grid[18 * 11 + i + 3] = toSave;
	//		}
	//		return TRUE;
	//	default:
	//		return FALSE;
	//}
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
