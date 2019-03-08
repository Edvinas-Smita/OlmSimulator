#pragma once

#include <windows.h>

#define GRID_BUTTON 0xB700

enum Orientation
{
	NorthUp, NorthRight, NorthDown, NorthLeft
};

BOOL initGrid(HWND m_hwnd);
BOOL rotateGrid(Orientation o);

BOOL redrawTile(unsigned char newPos, unsigned char oldPos);
