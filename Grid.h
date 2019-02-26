#pragma once

#include <windows.h>

#define GRID_BUTTON 0xB700

enum Orientation
{
	NorthUp, NorthRight, NorthDown, NorthLeft
};

//HWND getTileAtCoords(int x, int y);

BOOL initGrid(HWND m_hwnd, Orientation o);