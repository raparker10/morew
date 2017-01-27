#include "stdafx.h"

//
// CFileViewerApp
//
// Application class for the "morew" Large File Viewer application
//


// Screen sections
enum Section { 
	UNKNOWN,		// screen section is unititialized or unknown
	HEADER,			// heasder section where summary information is displayed
	FILEPOS,		// the file position section on the left side of the screen
	HEXDISPLAY,		// the main hex display section in the middle of the screen
	CHARDISPLAY,	// character display section on the right side of the screen
};

//
// CCursor
//
// Tracks the current cursor location in terms of line, column, and screen section
//
class CCursor {
public:
	Section cur_Section;
	int cur_line, cur_column;
	CCursor(void) : cur_Section(Section::UNKNOWN), cur_line(0), cur_column(0) {}
};

//
// CDrawInfo
//
// Information used for drawing in this application.
// RAP: this functionality should be moved to the CDrawingHelper class
//

class CDrawInfo {
public:
	// how much space should be left around characters and lines on the screen
	int iBorderLeft, iBorderTop, iBorderRight, iBorderBottom;
	int iPixelsPerLine, iPixelsPerChar, iPixelsPerBorderedChar;
	CDrawInfo(void) :
		iBorderLeft(1),
		iBorderTop(1),
		iBorderRight(0),
		iBorderBottom(0),
		iPixelsPerLine(0),
		iPixelsPerChar(0),
		iPixelsPerBorderedChar(0) {}
};

class CFileViewerApp : public CApplication {
	CCursor Cursor;
	CDrawInfo DrawInfo;
	CMoreDrawingHelper DrawHelper;

	CMoreScreenLayout ScreenLayout;

	// double-buffering object
	CCanvas Canvas;

	// file handling
	CFileBuffer FileBuffer;

	// message loop
	static LRESULT CALLBACK MessageProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:
	// application startup and shutdown
	CFileViewerApp();
	~CFileViewerApp();

	// message handlers
	virtual bool handle_create(HWND hWnd, LPCREATESTRUCT* lpcs);
	virtual bool handle_command(HWND hWnd, int wmID, int wmEvent);
	virtual bool handle_keydown(HWND hWnd, int iVKey);
	virtual bool handle_char(HWND hWnd, int iChar);
	virtual bool handle_vscroll(HWND hWnd, int iScrollRequest, int iScrollPosition);
	virtual bool handle_size(HWND hWnd, HDC hdc, int iWidth, int iHeight);
	virtual bool handle_mousemove(HWND hWnd, WPARAM wParam, WORD x, WORD y);
	virtual bool handle_paint(HWND hWnd, HDC hdc, LPPAINTSTRUCT ps);

	// functions for managing the number of characters displayed per line
	void set_display_width(HWND hWnd, int iDisplayWidthChars);
	void change_display_width(HWND hWnd, int iCharsDelta);

	// functions for displaying lines of text
	bool display_header(HDC hdc, int iCharactersPerLine);
	bool display_line(HWND hWnd, HDC hdc, HANDLE fh, int iCharactersPerLine, int iLine, bool bFramed);
	bool display_screen_db(HWND hWnd, HDC hdc);
	void draw_line(HDC hdc, int x, int y, LONG lPosHigh, DWORD dwPosLow, unsigned char * szReadLineBuffer, int iBytesRead, int iCharactersPerLine);

	// file management functions
	bool close_current_file(void);
	bool open_file(TCHAR *szFName, HWND hWnd);

	// file movement functions
	void update_scrollbar(HWND hWnd);
	bool get_position(int x, int y, Section *section, int *line, int *column);
	bool move_percent(int percent10);
	bool move_relative(__int64 iPages, __int64 iLines, __int64 iChars);
	bool move_absolute(__int64 iPage, __int64 iLine, __int64 iChar);
	bool move_position(LARGE_INTEGER liPosition);

	bool handle_file_selection(TCHAR *szFileName, int iFileNameSize);
	void handle_move_up(HWND hWnd);
	void handle_move_down(HWND hWnd);
	void handle_page_up(HWND hWnd);
	void handle_page_down(HWND hWnd);
	void handle_move_top(HWND hWnd);
};