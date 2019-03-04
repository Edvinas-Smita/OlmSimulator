#pragma once

#include <windows.h>

#define CONTROL_BUTTON 0xB800
#define CAMERA_CONTROL 0x20
#define OLMPOS_CONTROL 0x40
#define OLMPHASE_CONTROL 0x80
#define WEAPON_CONTROL 0xA0

HWND initControlArea(HWND hwnd);
BOOL initCameraControls(HWND hwnd);
BOOL initOlmPosControls(HWND hwnd);
BOOL initOlmPhaseControls(HWND hwnd);
BOOL initWeaponControls(HWND hwnd);