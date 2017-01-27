#include "stdafx.h"

enum Section { UNKNOWN, HEADER, FILEPOS, HEXDISPLAY, CHARDISPLAY };
class CCursor {
public:
	Section cur_Section;
	int cur_line, cur_column;
	CCursor(void) : cur_Section(Section::UNKNOWN), cur_line(0), cur_column(0) {}
};
class CDrawInfo {
public:
	//	TEXTMETRIC tm;
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

public:
	CFileViewerApp();
	~CFileViewerApp();

//	virtual bool handle_size(HWND hWnd);
	virtual bool handle_create(HWND hWnd, LPCREATESTRUCT* lpcs);
	virtual bool handle_command(HWND hWnd, int wmID, int wmEvent);
	virtual bool handle_keydown(HWND hWnd, int iVKey);
	virtual bool handle_char(HWND hWnd, int iChar);
	virtual bool handle_vscroll(HWND hWnd, int iScrollRequest, int iScrollPosition);
	virtual bool handle_size(HWND hWnd, HDC hdc, int iWidth, int iHeight);
	virtual bool handle_mousemove(HWND hWnd, WPARAM wParam, WORD x, WORD y);
	virtual bool handle_paint(HWND hWnd, HDC hdc, LPPAINTSTRUCT ps);


	void set_display_width(HWND hWnd, int iDisplayWidthChars);
	void change_display_width(HWND hWnd, int iCharsDelta);

	bool display_header(HDC hdc, int iCharactersPerLine);
	bool display_line(HWND hWnd, HDC hdc, HANDLE fh, int iCharactersPerLine, int iLine, bool bFramed);
	bool display_screen_db(HWND hWnd, HDC hdc);
	void draw_line(HDC hdc, int x, int y, LONG lPosHigh, DWORD dwPosLow, unsigned char * szReadLineBuffer, int iBytesRead, int iCharactersPerLine);


	bool close_current_file(void);
	bool open_file(TCHAR *szFName, HWND hWnd);

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


	static LRESULT CALLBACK MessageProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};