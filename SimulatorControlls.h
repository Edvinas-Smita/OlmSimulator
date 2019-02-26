#pragma once

#include <windows.h>

#include "Olm parts.h"
#include "Grid.h"

HWND initControllArea(HWND hwnd);
BOOL initCameraControlls(HWND hwnd, INT_PTR CALLBACK NU, INT_PTR CALLBACK NR, INT_PTR CALLBACK ND, INT_PTR CALLBACK NL);
BOOL initOlmPosControlls(HWND hwnd);
BOOL initOlmPhaseControlls(HWND hwnd);
BOOL initWeaponControlls(HWND hwnd);