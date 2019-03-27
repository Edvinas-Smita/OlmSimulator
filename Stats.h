#pragma once

#include <Windows.h>

#define STAT_FIELD_GRID 0x9D00
#define STAT_FIELD_PART 0xAD00

BOOL initOneField(HWND m_hwnd, BYTE index);
BOOL initStatFields(HWND m_hwnd, BYTE playerCount);

void updatePlayerStats(BYTE index, int gridClicks, int partClicks);
