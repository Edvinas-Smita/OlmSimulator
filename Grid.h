#pragma once

#include <windows.h>

#define GRID_BUTTON 0xB700

enum Orientation
{
	NorthUp, NorthRight, NorthDown, NorthLeft
};

BOOL initGrid(HWND m_hwnd);
BOOL rotateGrid(Orientation o);

BOOL redrawTile(BYTE index);
BOOL redrawTwoTiles(BYTE newPos, BYTE oldPos);
BOOL redrawAllTiles();
