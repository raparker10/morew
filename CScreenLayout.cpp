#include "stdafx.h"

 void CScreenLayout::Resize(HWND hWnd)
 {
	 GetClientRect(hWnd, &m_rcClient);
 }


