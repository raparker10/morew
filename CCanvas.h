#pragma once

// double buffering variables
class CCanvas {
public:
	bool bMemCreated = false;
	HDC hdcMem;
	HBITMAP hbmMem, hbmMemOld;
	RECT rcMem;
	CCanvas(void) : bMemCreated(false), hdcMem(NULL), hbmMem((HBITMAP)INVALID_HANDLE_VALUE), hbmMemOld((HBITMAP)INVALID_HANDLE_VALUE) {}
};
