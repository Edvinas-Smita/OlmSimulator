#pragma once

#include <windows.h>
#include "Grid.h"

enum OlmLocation{West, East};
enum OlmPart{Mage = 0, Head = 125, Melee = 250};

HWND createPart(HWND m_hwnd, OlmPart part);

BOOL initParts(HWND m_hwnd);

BOOL rotatePart(OlmLocation ol, Orientation ort, OlmPart part);

BOOL rotateAllParts(OlmLocation ol, Orientation ort)