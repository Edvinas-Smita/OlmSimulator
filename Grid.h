#pragma once

#include <windows.h>

enum Orientation
{
	NorthUp, NorthRight, NorthDown, NorthLeft
};

//HWND getTileAtCoords(int x, int y);

BOOL initGrid(HWND m_hwnd, Orientation o);