#include "stdafx.h"
#include "Olm Parts.h"

HWND head = nullptr;
HWND mage = nullptr;
HWND melee = nullptr;

HWND createPart(HWND m_hwnd, OlmPart part)
{
	LPCWSTR text;
	switch (part)
	{
		case Mage:
			text = L"MAGE";
			break;
		case Head:
			text = L"HEAD";
			break;
		case Melee:
			text = L"MELEE";
			break;
		default:
			return nullptr;
	}

	return CreateWindow(
		L"BUTTON",
		text,
		WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,  // Styles 
		0,			// x position
		300 - part,	// y position 
		125,		// Button width
		125,		// Button height
		m_hwnd,		// Parent window
		(HMENU) (OLMPART_BUTTON | part),
		(HINSTANCE) GetWindowLong(m_hwnd, GWLP_HINSTANCE),
		NULL
	);
}

BOOL initParts(HWND m_hwnd)
{
	mage = createPart(m_hwnd, Mage);
	head = createPart(m_hwnd, Head);
	melee = createPart(m_hwnd, Melee);

	if (head == nullptr || mage == nullptr || melee == nullptr)
	{
		return false;
	}
	return true;
}

BOOL rotatePart(OlmLocation ol, Orientation ort, OlmPart part)
{
	int windowWidth = 525, windowHeigth = 525;
	int offsetFromEdge = (ol == West ? 100 : 50) + part;
	int x, y;

	char c = ort + 2 * (ol == West);
	switch (c % 4)
	{
		case 0:
			x = windowWidth - 125;
			y = offsetFromEdge;
			break;
		case 1:
			x = windowWidth - offsetFromEdge - 125;
			y = windowHeigth - 125;
			break;
		case 2:
			x = 0;
			y = windowHeigth - offsetFromEdge - 125;
			break;
		case 3:
			x = offsetFromEdge;
			y = 0;
			break;
		default:
			return FALSE;
	};
	/*switch (ol)
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
					return FALSE;
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
					return FALSE;
			};
			break;
		default:
			return nullptr;
			return FALSE;
	}*/

	switch (part)
	{
		case Mage:
			MoveWindow(mage, x, y, 125, 125, FALSE);
			break;
		case Head:
			MoveWindow(head, x, y, 125, 125, FALSE);
			break;
		case Melee:
			MoveWindow(melee, x, y, 125, 125, FALSE);
			break;
		default:
			return FALSE;
	}

	return TRUE;
}

BOOL rotateAllParts(OlmLocation ol, Orientation ort)
{
	if (!rotatePart(ol, ort, Mage))
	{
		return FALSE;
	}
	if (!rotatePart(ol, ort, Head))
	{
		return FALSE;
	}
	if (!rotatePart(ol, ort, Melee))
	{
		return FALSE;
	}
	return TRUE;
}
