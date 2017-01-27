#include "stdafx.h"

// notify the object that the screen was resized
 void CScreenLayout::Resize(HWND hWnd)
 {
	 GetClientRect(hWnd, &m_rcClient);
 }


