#include "stdafx.h"

// formatting information
const int MAX_CHARS_PER_LINE = 1024;
const int PADDING_LENGTH = 5;
const int HEADER_LINES = 3;

// about dialog box information
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


// Constructor.
CFileViewerApp::CFileViewerApp()
{
	// Initialize the ScreenLayout
	ScreenLayout.put_DrawingHelper(&DrawHelper);
}

// Destructor
CFileViewerApp::~CFileViewerApp()
{

}

// repaint the screen from the double-buffer
bool CFileViewerApp::handle_paint(HWND hWnd, HDC hdc, LPPAINTSTRUCT ps)
{
	if (FileBuffer.HasOpenFile())
	{
		if ((ps->rcPaint.bottom - ps->rcPaint.top) > DrawHelper.get_FixedTextMetric()->tmHeight)
			display_screen_db(hWnd, hdc);
	}
	return true;
}

// handle the WM_CREATE message by initializing environment information
bool CFileViewerApp::handle_create(HWND hWnd, LPCREATESTRUCT *lpcs) 
{ 
	TEXTMETRIC tm;
	HDC hdc;
	HGDIOBJ hOldFont;

	// get the metrics of the fixed and variable fonts used for drawing
	hdc = GetDC(hWnd);
	hOldFont = SelectObject(hdc, GetStockObject(ANSI_FIXED_FONT));
	GetTextMetrics(hdc, &tm);
	DrawHelper.put_FixedTextMetric(&tm);
	ScreenLayout.put_TextMetrics(&tm);

	// get variable font information
	SelectObject(hdc, GetStockObject(ANSI_VAR_FONT));
	GetTextMetrics(hdc, &tm);
	DrawHelper.put_VariableTextMetric(&tm);

	SelectObject(hdc, hOldFont);
	return 0;
}

// handle the WM_COMMAND message 
bool CFileViewerApp::handle_command(HWND hWnd, int wmID, int wmEvent) 
{ 
	TCHAR szFileName[_MAX_PATH + 1];
	// Parse the menu selections:
	switch (wmID)
	{

	// open a new file
	case IDM_FILE_OPEN:
		szFileName[0] = 0;
		handle_file_selection(szFileName, _countof(szFileName));
		open_file(szFileName, hWnd);
		InvalidateRect(hWnd, NULL, TRUE);
		break;

	// display more characters per line
	case IDM_VIEW_WIDER:
		change_display_width(hWnd, 1);
		update_scrollbar(hWnd);
		break;

	// dieplay fewer characters per line
	case IDM_VIEW_NARROWER:
		change_display_width(hWnd, -1);
		update_scrollbar(hWnd);
		break;

	// go to the top of the file
	case IDM_VIEW_GO_TOP:
		move_absolute(0, 0, 0);
		update_scrollbar(hWnd);
		break;

	// display one line farther down the file
	case IDM_VIEW_LINE_DOWN:
		handle_move_down(hWnd);
		update_scrollbar(hWnd);
		break;

	// display starting from one line earlier in the file
	case IDM_VIEW_LINE_UP:
		handle_move_up(hWnd);
		update_scrollbar(hWnd);
		break;

	// RAP: display one page offset from current location
	case IDM_VIEW_PAGE_DOWN:
	case IDM_VIEW_PAGE_UP:
		break;

	// shift the display one character to the left
	case IDM_VIEW_SHIFT_LEFT:
		move_relative(0, 0, -1);
		update_scrollbar(hWnd);
		break;

	// shift the display one character to the right
	case IDM_VIEW_SHIFT_RIGHT:
		move_relative(0, 0, 1);
		update_scrollbar(hWnd);
		break;

	// display the "About" dialog box
	case IDM_ABOUT:
		DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
		break;

	// exit the application
	case IDM_EXIT:
		close_current_file();
		DestroyWindow(hWnd);
		break;

	default:
		// indicate that the command was not handled
		return false;
	}

	// indicate that the command was handled
	return true;

}
bool CFileViewerApp::handle_keydown(HWND hWnd, int iVKey) 
{ 
	return false; 
}
bool CFileViewerApp::handle_char(HWND hWnd, int iChar) 
{ 
	switch (iChar)
	{
	case 'u':
	case 'U':
		PostMessage(hWnd, WM_COMMAND, IDM_VIEW_LINE_UP, 0);
		break;

	case 'l':
	case 'L':
		PostMessage(hWnd, WM_COMMAND, IDM_VIEW_SHIFT_LEFT, 0);
		break;

	case 'r':
	case 'R':
		PostMessage(hWnd, WM_COMMAND, IDM_VIEW_SHIFT_RIGHT, 0);
		break;

	case 'b':
	case 'B':
		move_relative(-1, 0, 0);
		break;

	case '1':
		PostMessage(hWnd, WM_COMMAND, IDM_VIEW_LINE_DOWN, 0);
		break;

	case 't':
	case 'T':
		PostMessage(hWnd, WM_COMMAND, IDM_VIEW_GO_TOP, 0);
		break;

	case 'w':
	case 'W':
		PostMessage(hWnd, WM_COMMAND, IDM_VIEW_WIDER, 0);
		break;

	case 'n':
	case 'N':
		PostMessage(hWnd, WM_COMMAND, IDM_VIEW_NARROWER, 0);
		break;

	case ' ':
		break;

	default:
		return false;
	}

	InvalidateRect(hWnd, NULL, TRUE);
	update_scrollbar(hWnd);
	return true;
}
bool CFileViewerApp::handle_vscroll(HWND hWnd, int iScrollRequest, int iScrollPosition) 
{ 
	SCROLLINFO si;


	// get the vertical scroll bar information
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	GetScrollInfo(hWnd, SB_VERT, &si);

	// handle the scrollbar motion
	switch (iScrollRequest)
	{
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
		update_scrollbar(hWnd);
		return 0;

	case SB_LINEDOWN:
		handle_move_down(hWnd);
		update_scrollbar(hWnd);
		return 0;

	case SB_PAGEUP:
		handle_page_up(hWnd);
		update_scrollbar(hWnd);
		return 0;

	case SB_PAGEDOWN:
		handle_page_down(hWnd);
		update_scrollbar(hWnd);
		return 0;

	case SB_THUMBTRACK:
		move_percent(si.nTrackPos);
		update_scrollbar(hWnd);
		break;

	default:
		return false;

	}
	InvalidateRect(hWnd, NULL, TRUE);
	update_scrollbar(hWnd);
	return true;
}
bool CFileViewerApp::handle_size(HWND hWnd, HDC hdc, int iWidth, int iHeight) 
{ 
	// save the window cliend size
	DrawHelper.handle_Size(0, iWidth, iHeight);

	// determine the default number of characters to display
	int iDefaultWidth = DrawHelper.get_DefaultWidthChars();

	int iBorderLeft = 1, iBorderTop = 1, iBorderRight = 0, iBorderBottom = 0;
	DrawInfo.iPixelsPerLine = DrawHelper.get_FixedTextMetric()->tmHeight + iBorderTop + iBorderBottom;
	DrawInfo.iPixelsPerChar = DrawHelper.get_FixedTextMetric()->tmAveCharWidth;
	DrawInfo.iPixelsPerBorderedChar = DrawHelper.get_FixedTextMetric()->tmAveCharWidth + iBorderRight + iBorderLeft;

	ScreenLayout.put_PixelsPerChar(DrawInfo.iPixelsPerChar);


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
	return true;
}
bool CFileViewerApp::handle_mousemove(HWND hWnd, WPARAM wParam, WORD x, WORD y) 
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
	return true;
}

void CFileViewerApp::set_display_width(HWND hWnd, int iDisplayWidthChars)
{
	FileBuffer.put_PageSize(FileBuffer.get_PageLines(), iDisplayWidthChars);
	ScreenLayout.put_CharsPerLine(FileBuffer.get_PageColumns());
	FileBuffer.Refresh();
	InvalidateRect(hWnd, NULL, TRUE);
}
// increase or decrease the number of columns displayed per line
void CFileViewerApp::change_display_width(HWND hWnd, int iCharsDelta)
{
	set_display_width(hWnd, FileBuffer.get_PageColumns() + iCharsDelta);
}


// close the current file
bool CFileViewerApp::close_current_file(void)
{
	return FileBuffer.Close();
}


// draw one line to a drawing contect (in this program, it will be the HDC of the double buffer)
void CFileViewerApp::draw_line(HDC hdc, int x, int y, LONG lPosHigh, DWORD dwPosLow, unsigned char * szReadLineBuffer, int iBytesRead, int iCharactersPerLine)
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
		TextOut(hdc, ScreenLayout.get_FilePosPanel()->left + DrawInfo.iBorderLeft, y + DrawInfo.iBorderTop, szLineBuf, lstrlen(szLineBuf));
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
	LineTo(hdc, ScreenLayout.get_HexPanel()->right - DrawInfo.iPixelsPerChar / 2, y + DrawHelper.get_FixedTextMetric()->tmHeight + DrawInfo.iBorderTop + DrawInfo.iBorderBottom);
}

// Display a line of characters to the double buffer
// Returns false if there is an error reading, or if at the end of the file
bool CFileViewerApp::display_line(HWND hWnd, HDC hdc, HANDLE fh, int iCharactersPerLine, int iLine, bool bFramed)
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
	draw_line(hdc, 0, DrawHelper.get_FixedTextMetric()->tmHeight * (iLine + HEADER_LINES), iPosition >> 32, iPosition & 0x0FFFFFFFF, szReadLineBuffer, iLengthReturned, iCharactersPerLine);

	// frame the line, if requested (e.g. if it is the currently-selected line
	RECT rc;
	rc.left = ScreenLayout.get_HexPanel()->left;
	rc.right = ScreenLayout.get_CharPanel()->right;
	rc.top = iLine * DrawHelper.get_FixedTextMetric()->tmHeight + ScreenLayout.get_HexPanel()->top;
	rc.bottom = rc.top + DrawHelper.get_FixedTextMetric()->tmHeight;
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
bool CFileViewerApp::display_header(HDC hdc, int iCharactersPerLine)
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
	_stprintf_s(szBuf, _countof(szBuf), TEXT("%s Position: %I64u"), FileBuffer.get_FileName(szFileName, _countof(szFileName)), FileBuffer.get_TopOfPagePos());
	TextOut(hdc, ScreenLayout.get_HeaderPanel()->left + DrawInfo.iBorderLeft, ScreenLayout.get_HeaderPanel()->top + DrawInfo.iBorderTop, szBuf, lstrlen(szBuf));

	// display tick marks for the characters
	for (int i = 5; i < ScreenLayout.get_CharsPerLine(); i += 5)
	{
		int x;

		_stprintf_s(szBuf, _countof(szBuf), TEXT("%d"), i);
		GetTextExtentPoint(hdc, szBuf, lstrlen(szBuf), &s);

		x = ScreenLayout.get_HexPanel()->left + DrawInfo.iBorderLeft + DrawInfo.iPixelsPerChar * i * ScreenLayout.get_ByteDisplayLength() - DrawInfo.iPixelsPerChar / 2;
		MoveToEx(hdc, x, ScreenLayout.get_HeaderPanel()->bottom - DrawHelper.get_FixedTextMetric()->tmHeight / 2, NULL);
		LineTo(hdc, x, ScreenLayout.get_HeaderPanel()->bottom);
		TextOut(hdc, x - s.cx, ScreenLayout.get_HeaderPanel()->bottom - DrawHelper.get_FixedTextMetric()->tmHeight - DrawHelper.get_FixedTextMetric()->tmHeight / 2, szBuf, lstrlen(szBuf));

		MoveToEx(hdc, ScreenLayout.get_CharPanel()->left + DrawInfo.iBorderLeft + DrawInfo.iPixelsPerChar * i, ScreenLayout.get_HeaderPanel()->bottom - DrawHelper.get_FixedTextMetric()->tmHeight / 2, NULL);
		LineTo(hdc, ScreenLayout.get_CharPanel()->left + DrawInfo.iBorderLeft + DrawInfo.iPixelsPerChar * i, ScreenLayout.get_HeaderPanel()->bottom);
		TextOut(hdc, ScreenLayout.get_CharPanel()->left + DrawInfo.iBorderLeft + DrawInfo.iPixelsPerChar* i - s.cx, ScreenLayout.get_HeaderPanel()->bottom - DrawHelper.get_FixedTextMetric()->tmHeight - DrawHelper.get_FixedTextMetric()->tmHeight / 2, szBuf, lstrlen(szBuf));
	}

	// restore the old font  RAP: check this
	SelectObject(Canvas.hdcMem, hOldFont);
	return true;
}
// displays the screen using double buffering
bool CFileViewerApp::display_screen_db(HWND hWnd, HDC hdc)
{
	bool retval = false;
	LARGE_INTEGER liOldFilePos;
	unsigned char szReadLineBuffer[MAX_CHARS_PER_LINE + 1];

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
				draw_line(Canvas.hdcMem, 0, DrawHelper.get_FixedTextMetric()->tmHeight * (i + HEADER_LINES), iPosition >> 32, iPosition & 0x0FFFFFFFF, szReadLineBuffer, iBytesReturned, FileBuffer.get_PageColumns());
			}
		}
	}
	SelectObject(Canvas.hdcMem, hOldFont);

#ifdef _DEBUG
	// frame the various screen panels
	HBRUSH hbr = CreateSolidBrush(RGB(255, 0, 0));
	HBRUSH hbrOld = (HBRUSH)SelectObject(Canvas.hdcMem, (HGDIOBJ)hbr);
	FrameRect(Canvas.hdcMem, ScreenLayout.get_HeaderPanel(), hbr);
	FrameRect(Canvas.hdcMem, ScreenLayout.get_FilePosPanel(), hbr);
	FrameRect(Canvas.hdcMem, ScreenLayout.get_HexPanel(), hbr);
	FrameRect(Canvas.hdcMem, ScreenLayout.get_CharPanel(), hbr);
	DeleteObject((HGDIOBJ)SelectObject(Canvas.hdcMem, (HGDIOBJ)hbrOld));
#endif
	// blt the bitmap into from the memory DC to the the client window DC
	BitBlt(hdc, Canvas.rcMem.left, Canvas.rcMem.top, Canvas.rcMem.right - Canvas.rcMem.left, Canvas.rcMem.bottom - Canvas.rcMem.top, Canvas.hdcMem, 0, 0, SRCCOPY);

	retval = true;
	return retval;
}







// open a new file for display
bool CFileViewerApp::open_file(TCHAR *szFName, HWND hWnd)
{
	// open the file
	if (false == FileBuffer.Open(szFName))
	{
		TCHAR szBuf[64];
		swprintf_s(szBuf, _countof(szBuf), TEXT("Error %d opening file."), FileBuffer.get_LastError());
		MessageBox(hWnd, szBuf, TEXT("Error Opening File"), MB_ICONERROR | MB_OK);
		return false;
	}

	// get the file size
	DrawHelper.put_FileSizeBytes(FileBuffer.get_FileSize());
	set_display_width(hWnd, DrawHelper.get_DefaultWidthChars());


	// load the initial page of data from the file
	if (FileBuffer.Refresh())
	{
		// inform the Screen Layout object of the FileBuffer
		ScreenLayout.put_FileBuffer(&FileBuffer, hWnd);
		return true;
	}

	// return false if any of the commands fail
	return false;
}

// update the scrollbar to indicate the location of the cursor in the file
void CFileViewerApp::update_scrollbar(HWND hWnd)
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
bool CFileViewerApp::move_position(LARGE_INTEGER liPosition)
{
	if (FileBuffer.Seek(liPosition.QuadPart))
	{
		return FileBuffer.Refresh();
	}
	return false;
}

// move the a specific page, line, and characte
bool CFileViewerApp::move_absolute(__int64 iPage, __int64 iLine, __int64 iChar)
{
	LARGE_INTEGER liNewFilePos;

	liNewFilePos.QuadPart = iPage * ScreenLayout.get_LinesPerScreen() * ScreenLayout.get_CharsPerLine() + iLine * ScreenLayout.get_CharsPerLine() + iChar;
	return move_position(liNewFilePos);
}

// move to a position relative to the current location
bool CFileViewerApp::move_relative(__int64 iPages, __int64 iLines, __int64 iChars)
{

	LARGE_INTEGER liNewPos;

	// adjustst the current position by the specified number of pages, lines, and characters
	liNewPos.QuadPart = FileBuffer.get_TopOfPagePos() + (iPages * ScreenLayout.get_LinesPerScreen() * ScreenLayout.get_CharsPerLine()) + (iLines * ScreenLayout.get_CharsPerLine()) + iChars;
	return move_position(liNewPos);
}

// move to a percent of the file size
bool CFileViewerApp::move_percent(int percent10)
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
bool CFileViewerApp::get_position(int x, int y, Section *section, int *line, int *column)
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
	else if (PtInRect(ScreenLayout.get_FilePosPanel(), p))
	{
		*section = Section::FILEPOS;
		*line = (y - ScreenLayout.get_FilePosPanel()->top) / DrawHelper.get_FixedTextMetric()->tmHeight;
		*column = 0;
	}
	else if (PtInRect(ScreenLayout.get_HexPanel(), p))
	{
		*section = Section::HEXDISPLAY;
		*line = (y - ScreenLayout.get_HexPanel()->top) / DrawHelper.get_FixedTextMetric()->tmHeight;
		*column = (x - ScreenLayout.get_HexPanel()->left) / (ScreenLayout.get_ByteDisplayLength() * DrawInfo.iPixelsPerChar);
	}
	else if (PtInRect(ScreenLayout.get_CharPanel(), p))
	{
		*section = Section::CHARDISPLAY;
		*line = (y - ScreenLayout.get_CharPanel()->top) / DrawHelper.get_FixedTextMetric()->tmHeight;
		*column = (x - ScreenLayout.get_CharPanel()->left) / (1 * DrawInfo.iPixelsPerChar);
	}
	return *section != Section::UNKNOWN;
}

//
// prompts the user for a file selection
//
// 
bool CFileViewerApp::handle_file_selection(TCHAR *szFileName, int iFileNameSize)
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
void CFileViewerApp::handle_move_up(HWND hWnd)
{
	FileBuffer.LineUp();
	display_screen_db(hWnd, GetDC(hWnd));
}

// move down one line
void CFileViewerApp::handle_move_down(HWND hWnd)
{
	FileBuffer.LineDown();
	display_screen_db(hWnd, GetDC(hWnd));
}

// move up one page
void CFileViewerApp::handle_page_up(HWND hWnd)
{
	FileBuffer.PageUp();
	InvalidateRect(hWnd, NULL, TRUE);
	update_scrollbar(hWnd);
}

// move down one page
void CFileViewerApp::handle_page_down(HWND hWnd)
{
	FileBuffer.PageDown();
	InvalidateRect(hWnd, NULL, TRUE);
	update_scrollbar(hWnd);
}

// move to the top
void CFileViewerApp::handle_move_top(HWND hWnd)
{
	FileBuffer.MoveTop();
	InvalidateRect(hWnd, NULL, TRUE);
	update_scrollbar(hWnd);
}

