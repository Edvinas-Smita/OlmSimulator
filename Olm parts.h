#pragma once

#include <windows.h>
#include "Grid.h"

enum OlmLocation{West, East};
enum OlmPart{Mage = 0, Head = 125, Melee = 250};

HWND createPart(HWND m_hwnd, OlmLocation ol, Orientation ort, OlmPart part, LPCWSTR text);

BOOL initParts(HWND m_hwnd, OlmLocation ol, Orientation ort);