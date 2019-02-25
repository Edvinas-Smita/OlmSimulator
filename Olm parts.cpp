#include "stdafx.h"
#include "Olm parts.h"

#include <iostream>

HWND head = nullptr;
HWND mage = nullptr;
HWND melee = nullptr;

HWND createPart(HWND m_hwnd, OlmLocation ol, Orientation ort, OlmPart part, LPCWSTR text)
{
	int windowWidth = 525, windowHeigth = 525;
	int offsetFromEdge = (ol == West ? 100 : 50) + part;
	int x, y;
	switch (ol)
	{
		case East:
			switch (ort)
			{
				case NorthUp:
					x = windowWidth - 125;
					y = offsetFromEdge;
					break;
				case NorthRight:
					x = windowWidth - offsetFromEdge - 125;
					y = windowHeigth - 125;
					break;
				case NorthDown:
					x = 0;
					y = windowHeigth - offsetFromEdge - 125;
					break;
				case NorthLeft:
					x = offsetFromEdge;
					y = 0;
					break;
				default:
					return nullptr;
			};
			break;
		case West:
			switch (ort)
			{
				case NorthUp:
					x = 0;
					y = windowHeigth - offsetFromEdge - 125;
					break;
				case NorthRight:
					x = offsetFromEdge;
					y = 0;
					break;
				case NorthDown:
					x = windowWidth - 125;
					y = offsetFromEdge;
					break;
				case NorthLeft:
					x = windowWidth - offsetFromEdge - 125;
					y = windowHeigth - 125;
					break;
				default:
					return nullptr;
			};
			break;
		default:
			return nullptr;
	}

	std::cout << "X: " << x << "; Y: " << y << "\n";
	return CreateWindow(
		L"BUTTON",
		text,
		WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
		x,			// x position 
		y,			// y position 
		125,		// Button width
		125,		// Button height
		m_hwnd,		// Parent window
		NULL,		// No menu.
		(HINSTANCE) GetWindowLong(m_hwnd, GWL_HINSTANCE),
		NULL
	);
}

BOOL initParts(HWND m_hwnd, OlmLocation ol, Orientation ort)
{
	mage = createPart(m_hwnd, ol, ort, Mage, L"MAGE");
	head = createPart(m_hwnd, ol, ort, Head, L"HEAD");
	melee = createPart(m_hwnd, ol, ort, Melee, L"MELEE");

	if (head == nullptr || mage == nullptr || melee == nullptr)
	{
		return false;
	}
	return true;
}