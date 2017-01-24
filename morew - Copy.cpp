// morew.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "CFileBuffer.h"
#include "morew.h"
#include "CMoreScreenLayout.h"
#include "CCanvas.h"
#include <Commdlg.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
TCHAR szFileName[_MAX_PATH + 1];				// the file beign displayed

// formatting information
const int MAX_CHARS_PER_LINE = 1024;
const int PADDING_LENGTH = 5;
const int HEADER_LINES = 3;
CMoreScreenLayout ScreenLayout;

// tracks the current file viewing location
class CCursor {
public:
	Section cur_Section;
	int cur_line, cur_column;
	CCursor(void) : cur_Section(Section::UNKNOWN), cur_line(0), cur_column(0) {}
};
CCursor Cursor;
bool move_relative(__int64 iPages, __int64 iLines, __int64 iChars);
bool move_absolute(__int64 iPage, __int64 iLine, __int64 iChar);
bool move_percent(int percent10);

// caches the drawing information
class CDrawInfo {
public:
	TEXTMETRIC tm;
	int iBorderLeft, iBorderTop, iBorderRight, iBorderBottom;
	int iPixelsPerLine, iPixelsPerChar, iPixelsPerBorderedChar;
	CDrawInfo(void) : iBorderLeft(1), iBorderTop(1), iBorderRight(0), iBorderBottom(0), iPixelsPerLine(0), iPixelsPerChar(0), iPixelsPerBorderedChar(0) {}
};
CDrawInfo DrawInfo;

// scrollbar positioning
void update_scrollbar(HWND hWnd);

// file drawing information
void draw_line(HDC hdc, int x, int y, LONG lPosHigh, DWORD dwPosLow, unsigned char *szReadLineBuffer, int iBytesRead, int iCharactersPerLine);

// double-buffering object
CCanvas Canvas;
TCHAR *format_header(int iHeaderLine, TCHAR * szDisplayLineBuffer, int iDisplayLineBufferCount, int iCharactersPerLine);

// mouse movement and hilighting
bool get_position(int x, int y, Section *section, int *line, int *column);
void handle_mousemove(HWND hWnd, WPARAM wParam, WORD x, WORD y);
void update_screen_regions(HWND hWnd);
void change_display_width(HWND hWnd, int iCharsDelta);
bool display_line(HWND hWnd, HDC hdc, HANDLE fh, int iCharactersPerLine, int iLine, bool bFramed);

// file handling
CFileBuffer FileBuffer;
bool open_file(TCHAR *szFName, HWND hWnd);
bool close_current_file(void);
bool display_screen_db(HWND hWnd, HDC hdc, int iSingleLineNumber = INT_MAX);

// screen size information
void handle_resize(HWND hWnd, HDC hdc);

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MOREW, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	*szFileName = 0;

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MOREW));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW; // | WS_VSCROLL;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MOREW));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_MOREW);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_VSCROLL,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
// prompts the user for a file selection
//
// 
bool handle_file_selection(TCHAR *szFileName, int iFileNameSize)
{
	OPENFILENAME ofn;
	TCHAR szFName[_MAX_PATH + 1];

	*szFileName = 0;

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFName;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFName);
	ofn.lpstrFilter = TEXT("All\0*.*\0AVI\0*.avi\0Matroska\0*.MKV\0MP4\0*.mp4\0Text\0*.TXT\0Video\0*.avi;*.mkv;*.mp4\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (FALSE == GetOpenFileName(&ofn))
		return false;

	if (lstrlen(ofn.lpstrFile) == 0)
		return false;

	_tcscpy_s(szFileName, iFileNameSize, ofn.lpstrFile);

	return true;
}

// move up one line
void handle_move_up(HWND hWnd)
{
	FileBuffer.LineUp();
	display_screen_db(hWnd, GetDC(hWnd));
}

// move down one line
void handle_move_down(HWND hWnd)
{
	FileBuffer.LineDown();
	display_screen_db(hWnd, GetDC(hWnd));
}

// move up one page
void handle_page_up(HWND hWnd)
{
	FileBuffer.PageUp();
	InvalidateRect(hWnd, NULL, TRUE);
	update_scrollbar(hWnd);
}

// move down one page
void handle_page_down(HWND hWnd)
{
	FileBuffer.PageDown();
	InvalidateRect(hWnd, NULL, TRUE);
	update_scrollbar(hWnd);
}

// move to the top
void handle_move_top(HWND hWnd)
{
	FileBuffer.MoveTop();
	InvalidateRect(hWnd, NULL, TRUE);
	update_scrollbar(hWnd);
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	SCROLLINFO si;

	switch (message)
	{
	case WM_CREATE:
		handle_resize(hWnd, GetDC(hWnd));
		break;

	case WM_ERASEBKGND:
		return 1;

	case WM_MOUSEMOVE:
		handle_mousemove(hWnd, wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_FILE_OPEN:
			*szFileName = 0;
			handle_file_selection(szFileName, _countof(szFileName));
			open_file(szFileName, hWnd);
			InvalidateRect(hWnd, NULL, TRUE);
			break;

		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			close_current_file();
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_NEXT:
			handle_page_down(hWnd);
			return 0;

		case VK_PRIOR:
			handle_page_up(hWnd);
			return 0;

		case VK_HOME:
			move_absolute(0, 0, 0);
			InvalidateRect(hWnd, NULL, TRUE);
			return 0;
		}
		break;

	case WM_CHAR:
		switch (wParam){
		case 'u':
		case 'U':
			handle_move_up(hWnd);
			break;

		case 'l':
		case 'L':
			move_relative(0, 0, -1);
			break;

		case 'r':
		case 'R':
			move_relative(0, 0, 1);
			break;

		case 'b':
		case 'B':
			move_relative(-1, 0, 0);
			break;

		case '1':
			handle_move_down(hWnd);
			break;

		case 't':
		case 'T':
			move_absolute(0, 0, 0);
			break;

		case 'w':
		case 'W':
			change_display_width(hWnd, 1);
			break;

		case 'n':
		case 'N':
			change_display_width(hWnd, -1);
			break;

		case ' ':
			break;
		}

		InvalidateRect(hWnd, NULL, TRUE);
		update_scrollbar(hWnd);
		break;

	case WM_VSCROLL:
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		GetScrollInfo(hWnd, SB_VERT, &si);

		switch (LOWORD(wParam)){
/*		case SB_TOP:
			si.nPos = si.nMin;
			move_absolute(0, 0, 0);
			break;

		case SB_BOTTOM:
			si.nPos = si.nMax;
			bMoveAbsolute = true;
			liFilePos.QuadPart = iFileSize - (iLinesPerScreen * iCharsPerLine);
			liFilePos.QuadPart = (liFilePos.QuadPart < 0) ? 0 : liFilePos.QuadPart;
			break;
*/
		case SB_LINEUP:
			handle_move_up(hWnd);
			return 0;

		case SB_LINEDOWN:
			handle_move_down(hWnd);
			return 0;

		case SB_PAGEUP:
			handle_page_up(hWnd);
			return 0;

		case SB_PAGEDOWN:
			handle_page_down(hWnd);
			return 0;

		case SB_THUMBTRACK:
			move_percent(si.nTrackPos);
			break;

		case SB_ENDSCROLL:
			return DefWindowProc(hWnd, message, wParam, lParam);

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);

		}
		InvalidateRect(hWnd, NULL, TRUE);
		update_scrollbar(hWnd);
		return 0;

	case WM_SIZE:
		handle_resize(hWnd, GetDC(hWnd));
		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		if (FileBuffer.HasOpenFile())
		{
			if ((ps.rcPaint.bottom - ps.rcPaint.top) > DrawInfo.tm.tmHeight)
				display_screen_db(hWnd, hdc, INT_MAX);
		}
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

// increase or decrease the number of columns displayed per line
void change_display_width(HWND hWnd, int iCharsDelta)
{
	FileBuffer.put_PageSize(FileBuffer.get_PageLines(), FileBuffer.get_PageColumns() + iCharsDelta);
	ScreenLayout.put_CharsPerLine(FileBuffer.get_PageColumns());
	FileBuffer.Refresh();
	update_screen_regions(hWnd);
	InvalidateRect(hWnd, NULL, TRUE);
}

// handle resizing the screen
void handle_resize(HWND hWnd, HDC hdc)
{

	update_screen_regions(hWnd);

	HGDIOBJ hOldFont = SelectObject(hdc, GetStockObject(ANSI_FIXED_FONT));
	GetTextMetrics(hdc, &DrawInfo.tm);
	ScreenLayout.put_TextMetrics(&DrawInfo.tm);
	
	int iBorderLeft = 1, iBorderTop = 1, iBorderRight = 0, iBorderBottom = 0;
	DrawInfo.iPixelsPerLine = DrawInfo.tm.tmHeight + iBorderTop + iBorderBottom;
	DrawInfo.iPixelsPerChar = DrawInfo.tm.tmAveCharWidth;
	DrawInfo.iPixelsPerBorderedChar = DrawInfo.tm.tmAveCharWidth + iBorderRight + iBorderLeft;
	
	ScreenLayout.put_PixelsPerChar(DrawInfo.iPixelsPerChar);
//	iPixelsPerChar = tm.tmMaxCharWidth + iBorderRight + iBorderLeft;

	SelectObject(hdc, hOldFont);

	ScreenLayout.Resize(hWnd);
	FileBuffer.put_PageSize(ScreenLayout.get_LinesPerScreen(), ScreenLayout.get_CharsPerLine());

	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE;
	si.nMin = 0;
	si.nMax = SCROLL_RANGE;
	SetScrollInfo(hWnd, SB_VERT, &si, TRUE);

	// release existing double buffering objects
	if (Canvas.bMemCreated)
	{
		SelectObject(Canvas.hdcMem, Canvas.hbmMemOld);
		DeleteObject(Canvas.hbmMem);
		DeleteDC(Canvas.hdcMem);
	}
	// create double buffer for the new window size
	GetClientRect(hWnd, &Canvas.rcMem);
	Canvas.hdcMem = CreateCompatibleDC(hdc);
	Canvas.hbmMem = CreateCompatibleBitmap(hdc, Canvas.rcMem.right - Canvas.rcMem.left, Canvas.rcMem.bottom - Canvas.rcMem.top);
	Canvas.hbmMemOld = (HBITMAP)SelectObject(Canvas.hdcMem, Canvas.hbmMem);
}

// draw one line to a drawing contect (in this program, it will be the HDC of the double buffer)
void draw_line(HDC hdc, int x, int y, LONG lPosHigh, DWORD dwPosLow, unsigned char * szReadLineBuffer, int iBytesRead, int iCharactersPerLine)
{
	const unsigned char * s = szReadLineBuffer;
	int iChars = 0;
	int i;
	int iCurX = x;
	int iCurCharX = 0;
	TCHAR szLineBuf[MAX_CHARS_PER_LINE + 1];
	static COLORREF clrZero = RGB(192, 192, 192);
	static COLORREF clrAlphaNum = RGB(0, 0, 0);
	static COLORREF clrPunct = RGB(0, 0, 128);
	static COLORREF clrCRLF = RGB(128, 0, 0);
	static COLORREF clrDefault = RGB(0, 128, 0);

	// only display the line if there are characters to display
	if (iBytesRead > 0)
	{
		if (FileBuffer.Is64BitFile())
		{
			_stprintf_s(szLineBuf, _countof(szLineBuf), ScreenLayout.get_FilePosMask(), lPosHigh, dwPosLow);
		}
		else
		{
			_stprintf_s(szLineBuf, _countof(szLineBuf), ScreenLayout.get_FilePosMask(), dwPosLow);
		}
		TextOut(hdc, ScreenLayout.get_FilePanel()->left + DrawInfo.iBorderLeft, y + DrawInfo.iBorderTop, szLineBuf, lstrlen(szLineBuf));
		iCurX += lstrlen(szLineBuf) * DrawInfo.iPixelsPerChar;
	}

	// display characters in HEX
	iCurX = ScreenLayout.get_HexPanel()->left + DrawInfo.iBorderLeft;
	iCurCharX = ScreenLayout.get_CharPanel()->left + DrawInfo.iBorderLeft;

	// display the characters
	for (i = 0; i < iBytesRead; ++i, ++iChars)
	{
		_stprintf_s(szLineBuf, _countof(szLineBuf), TEXT("%02X"), (unsigned char)*s);
		LPCOLORREF pc = &clrDefault;

		// determine which color to use based on the type of character being displayed
		if (*s == 0)
		{
			pc = &clrZero;
		}
		else if (isalnum(*s))
		{
			pc = &clrAlphaNum;
		}
		else if (ispunct(*s))
		{
			pc = &clrPunct;
		}
		else if (*s == 0x0d || *s == 0x0A)
		{
			pc = &clrCRLF;
		}

		COLORREF clrOld = SetTextColor(hdc, *pc);
		TextOut(hdc, iCurX, y + DrawInfo.iBorderTop, szLineBuf, lstrlen(szLineBuf));

		// display the character
		TCHAR c = _istprint(*s) ? *s : TEXT('.');
		TextOut(hdc, iCurCharX, y + DrawInfo.iBorderTop, &c, 1);

		// restore the default color
		SetTextColor(hdc, clrOld);
		
		// advance the position
		iCurX += (3 * DrawInfo.iPixelsPerChar);
		iCurCharX += DrawInfo.iPixelsPerChar;
		s += 1;
	}

	// add the separator
	MoveToEx(hdc, ScreenLayout.get_HexPanel()->right - DrawInfo.iPixelsPerChar / 2, y, NULL);
	LineTo(hdc, ScreenLayout.get_HexPanel()->right - DrawInfo.iPixelsPerChar / 2, y + DrawInfo.tm.tmHeight + DrawInfo.iBorderTop + DrawInfo.iBorderBottom);
}

// store the current screen size
void update_screen_regions(HWND hWnd)
{
	RECT rcClient;

	GetClientRect(hWnd, &rcClient);
}

// Display a line of characters to the double buffer
// Returns false if there is an error reading, or if at the end of the file
bool display_line(HWND hWnd, HDC hdc, HANDLE fh, int iCharactersPerLine, int iLine, bool bFramed)
{
	unsigned char szReadLineBuffer[MAX_CHARS_PER_LINE + 1];

	// make sure there is a file to display
	if (FileBuffer.HasOpenFile() == false)
		return false;

	//only display relevant lines
	if (iLine < 0)
		return false;

	HGDIOBJ hOldFont = SelectObject(hdc, GetStockObject(OEM_FIXED_FONT));

	// get the characters to display
	_int64 iPosition;
	int iLengthReturned;
	FileBuffer.get_Line(iLine, &iPosition, szReadLineBuffer, &iLengthReturned);

	// draw the line to the DC (usually the double buff
	draw_line(hdc, 0, DrawInfo.tm.tmHeight * (iLine + HEADER_LINES), iPosition >> 32, iPosition & 0x0FFFFFFFF, szReadLineBuffer, iLengthReturned, iCharactersPerLine);

	// frame the line, if requested (e.g. if it is the currently-selected line
	RECT rc;
	rc.left = ScreenLayout.get_HexPanel()->left;
	rc.right = ScreenLayout.get_CharPanel()->right;
	rc.top = iLine * DrawInfo.tm.tmHeight + ScreenLayout.get_HexPanel()->top;
	rc.bottom = rc.top + DrawInfo.tm.tmHeight;
	if (bFramed)
	{
		FrameRect(hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
	}
	else
	{
		FrameRect(hdc, &rc, (HBRUSH)(COLOR_WINDOW + 1));
	}

	// restore the previous font
	SelectObject(hdc, hOldFont);
	return true;
}

// draw the header
bool display_header(HDC hdc, int iCharactersPerLine)
{
	TCHAR szBuf[1024];
	SIZE s;

	// make sure there is an open file
	if (FileBuffer.HasOpenFile() == false)
		return false;

	// use the system fixed ont
	HGDIOBJ hOldFont = SelectObject(hdc, GetStockObject(OEM_FIXED_FONT));

	// display the file position, line number, and horizontal offset
	TCHAR szFileName[_MAX_PATH + 1];
	_stprintf_s(szBuf, _countof(szBuf), TEXT("%s Position: %I64u"), FileBuffer.get_FileName(szFileName), FileBuffer.get_TopOfPagePos());
	TextOut(hdc, ScreenLayout.get_HeaderPanel()->left + DrawInfo.iBorderLeft, ScreenLayout.get_HeaderPanel()->top + DrawInfo.iBorderTop, szBuf, lstrlen(szBuf));

	// display tick marks for the characters
	for (int i = 5; i < ScreenLayout.get_CharsPerLine(); i += 5)
	{
		int x;

		_stprintf_s(szBuf, _countof(szBuf), TEXT("%d"), i);
		GetTextExtentPoint(hdc, szBuf, lstrlen(szBuf), &s);

		x = ScreenLayout.get_HexPanel()->left + DrawInfo.iBorderLeft + DrawInfo.iPixelsPerChar * i * ScreenLayout.get_ByteDisplayLength() - DrawInfo.iPixelsPerChar / 2;
		MoveToEx(hdc, x, ScreenLayout.get_HeaderPanel()->bottom - DrawInfo.tm.tmHeight / 2, NULL);
		LineTo(hdc, x, ScreenLayout.get_HeaderPanel()->bottom);
		TextOut(hdc, x - s.cx, ScreenLayout.get_HeaderPanel()->bottom - DrawInfo.tm.tmHeight - DrawInfo.tm.tmHeight / 2, szBuf, lstrlen(szBuf));

		MoveToEx(hdc, ScreenLayout.get_CharPanel()->left + DrawInfo.iBorderLeft + DrawInfo.iPixelsPerChar * i, ScreenLayout.get_HeaderPanel()->bottom - DrawInfo.tm.tmHeight / 2, NULL);
		LineTo(hdc, ScreenLayout.get_CharPanel()->left + DrawInfo.iBorderLeft + DrawInfo.iPixelsPerChar * i, ScreenLayout.get_HeaderPanel()->bottom);
		TextOut(hdc, ScreenLayout.get_CharPanel()->left + DrawInfo.iBorderLeft + DrawInfo.iPixelsPerChar* i - s.cx, ScreenLayout.get_HeaderPanel()->bottom - DrawInfo.tm.tmHeight - DrawInfo.tm.tmHeight / 2, szBuf, lstrlen(szBuf));
	}

	// restore the old font  RAP: check this
	SelectObject(Canvas.hdcMem, hOldFont);
	return true;
}
// displays the screen using double buffering
bool display_screen_db(HWND hWnd, HDC hdc, int iSingleLineNumber)
{
	bool retval = false;
	LARGE_INTEGER liOldFilePos;
	unsigned char szReadLineBuffer[MAX_CHARS_PER_LINE + 1];

	// get the locations to draw various pieces of information, based on the current screen size
	update_screen_regions(hWnd);

	// erase the double-buffer background
	FillRect(Canvas.hdcMem, &Canvas.rcMem, GetSysColorBrush(COLOR_WINDOW)); // erase the background

	// use the system fixed font for drawing characters
	HGDIOBJ hOldFont = SelectObject(Canvas.hdcMem, GetStockObject(OEM_FIXED_FONT));

	// save the current file position
	liOldFilePos.QuadPart = FileBuffer.get_TopOfPagePos();

	// display the header
	display_header(Canvas.hdcMem, FileBuffer.get_PageColumns());

	// display the lines of the file
	for (int i = 0; i < FileBuffer.get_PageLines(); ++i)
	{
		DWORD iBytesRead = 0;
		int iBytesReturned;
		_int64 iPosition;

		if (FileBuffer.get_Line(i, &iPosition, szReadLineBuffer, &iBytesReturned))
		{
			if (iBytesReturned > 0)
			{
				draw_line(Canvas.hdcMem, 0, DrawInfo.tm.tmHeight * (i + HEADER_LINES), iPosition >> 32, iPosition & 0x0FFFFFFFF, szReadLineBuffer, iBytesReturned, FileBuffer.get_PageColumns());
			}
		}
	}
	SelectObject(Canvas.hdcMem, hOldFont);

	// frame the various screen panels
	HBRUSH hbr = CreateSolidBrush(RGB(255, 0, 0));
	HBRUSH hbrOld = (HBRUSH)SelectObject(Canvas.hdcMem, (HGDIOBJ)hbr);
	FrameRect(Canvas.hdcMem, ScreenLayout.get_HeaderPanel(), hbr);
	FrameRect(Canvas.hdcMem, ScreenLayout.get_FilePanel(), hbr);
	FrameRect(Canvas.hdcMem, ScreenLayout.get_HexPanel(), hbr);
	FrameRect(Canvas.hdcMem, ScreenLayout.get_CharPanel(), hbr);
	DeleteObject((HGDIOBJ)SelectObject(Canvas.hdcMem, (HGDIOBJ)hbrOld));

	// blt the bitmap into from the memory DC to the the client window DC
	BitBlt(hdc, Canvas.rcMem.left, Canvas.rcMem.top, Canvas.rcMem.right - Canvas.rcMem.left, Canvas.rcMem.bottom - Canvas.rcMem.top, Canvas.hdcMem, 0, 0, SRCCOPY);

	retval = true;
	return retval;
}

// close the current file
bool close_current_file(void)
{
	return FileBuffer.Close();
}

// open a new file for display
bool open_file(TCHAR *szFName, HWND hWnd)
{
	// open the file
	if (FileBuffer.Open(szFName))
	{
		// load the initial page of data from the file
		if (FileBuffer.Refresh())
		{
			// inform the Screen Layout object of the FileBuffer
			ScreenLayout.put_FileBuffer(&FileBuffer, hWnd);
			return true;
		}
	}
	// return false if any of the commands fail
	return false;
}

// update the scrollbar to indicate the location of the cursor in the file
void update_scrollbar(HWND hWnd)
{
	SCROLLINFO si;

	si.cbSize = sizeof(si);
	si.fMask = SIF_POS;
	__int64 sbpos;

	if (FileBuffer.get_FileSize() == 0)
	{
		sbpos = 0;
	}
	else
	{
		sbpos = (FileBuffer.get_TopOfPagePos() * SCROLL_RANGE) / FileBuffer.get_FileSize();
	}
	si.nPos = (int)sbpos;
	SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
}

// move the current file position to a specific location in the file
bool move_position(LARGE_INTEGER liPosition)
{
	if (FileBuffer.Seek(liPosition.QuadPart))
	{
		return FileBuffer.Refresh();
	}
	return false;
}

// move the a specific page, line, and characte
bool move_absolute(__int64 iPage, __int64 iLine, __int64 iChar)
{
	LARGE_INTEGER liNewFilePos;

	liNewFilePos.QuadPart = iPage * ScreenLayout.get_LinesPerScreen() * ScreenLayout.get_CharsPerLine() + iLine * ScreenLayout.get_CharsPerLine() + iChar;
	return move_position(liNewFilePos);
}

// move to a position relative to the current location
bool move_relative(__int64 iPages, __int64 iLines, __int64 iChars)
{

	LARGE_INTEGER liNewPos;

	// adjustst the current position by the specified number of pages, lines, and characters
	liNewPos.QuadPart = FileBuffer.get_TopOfPagePos() + (iPages * ScreenLayout.get_LinesPerScreen() * ScreenLayout.get_CharsPerLine()) + (iLines * ScreenLayout.get_CharsPerLine()) + iChars;
	return move_position(liNewPos);
}

// move to a percent of the file size
bool move_percent(int percent10)
{
	LARGE_INTEGER iNewPos;

	// do math while preventing over or underflow
	// assume if the file is over a MB in size that it can be divided by 1000 without adverse impact
	if (FileBuffer.get_FileSize() > 1024 * 1024)
	{
		iNewPos.QuadPart = FileBuffer.get_FileSize() / 1000 * percent10;
	}
	else
	{
		iNewPos.QuadPart = FileBuffer.get_FileSize() * percent10 / 1000;
	}
	return move_position(iNewPos);
}

// turn the mouse client pixel location into a section, line, and column
bool get_position(int x, int y, Section *section, int *line, int *column)
{
	POINT p;
	p.x = x;
	p.y = y;

	*section = Section::UNKNOWN;

	// determine the section
	if (PtInRect(ScreenLayout.get_HeaderPanel(), p))
	{
		*section = Section::HEADER;
		*line = 0;
		*column = 0;
	}
	else if (PtInRect(ScreenLayout.get_FilePanel(), p))
	{
		*section = Section::FILEPOS;
		*line = (y - ScreenLayout.get_FilePanel()->top) / DrawInfo.tm.tmHeight;
		*column = 0;
	}
	else if (PtInRect(ScreenLayout.get_HexPanel(), p))
	{
		*section = Section::HEXDISPLAY;
		*line = (y - ScreenLayout.get_HexPanel()->top) / DrawInfo.tm.tmHeight;
		*column = (x - ScreenLayout.get_HexPanel()->left) / (ScreenLayout.get_ByteDisplayLength() * DrawInfo.iPixelsPerChar);
	}
	else if (PtInRect(ScreenLayout.get_CharPanel(), p))
	{
		*section = Section::CHARDISPLAY;
		*line = (y - ScreenLayout.get_CharPanel()->top) / DrawInfo.tm.tmHeight;
		*column = (x - ScreenLayout.get_CharPanel()->left) / (1 * DrawInfo.iPixelsPerChar);
	}
	return *section != Section::UNKNOWN;
}

// respond to the movement of the mouse
void handle_mousemove(HWND hWnd, WPARAM wParam, WORD x, WORD y)
{

	HDC hdc = GetDC(hWnd);
	int iOldLine = Cursor.cur_line;

	// get the mouse VERTICAL position
	get_position(x, y, &Cursor.cur_Section, &Cursor.cur_line, &Cursor.cur_column);

	// unhilight the previous line
	if (iOldLine != Cursor.cur_line)
	{
		display_line(hWnd, hdc, FileBuffer.get_fh(), ScreenLayout.get_CharsPerLine(), iOldLine, false);
		display_line(hWnd, hdc, FileBuffer.get_fh(), ScreenLayout.get_CharsPerLine(), Cursor.cur_line, true);
	}

	display_header(hdc, ScreenLayout.get_CharsPerLine());
}
