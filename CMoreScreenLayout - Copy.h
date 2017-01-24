#include "stdafx.h"
#include "CScreenLayout.h"

extern class CFileBuffer;

// screen layout
enum Section { HEADER, FILEPOS, HEXDISPLAY, CHARDISPLAY, UNKNOWN };
const int SCROLL_RANGE = 1000;


class CMoreScreenLayout : public CScreenLayout {
private:
	int m_iCharsPerLine;
	int m_iLinesPerScreen;
	RECT m_rcHeader, m_rcFilePos, m_rcHexDisplay, m_rcCharDisplay;
	static  TCHAR *m_sz32bitMask;
	static  TCHAR *m_sz64bitMask;
	static int CHARS_32_BIT_ADDRESS;
	static int CHARS_64_BIT_ADDRESS;

	int m_iFilePosChars;
	TCHAR *m_szFilePosMask;
	int m_iFilePosWidth;
	POINT m_ptMaxSize, m_ptMinSize;
	int m_iPixelsPerChar;

	void Initialzie(void) { m_ptMinSize.x = 1; m_ptMinSize.y = 1; m_ptMaxSize.x = 1024; m_ptMaxSize.y = 1024; }
public:
	inline void put_LinesPerScreen(int iLinesPerScreen) { m_iLinesPerScreen = max(min(m_iLinesPerScreen, m_ptMaxSize.y), m_ptMinSize.y); }
	inline void put_CharsPerLine(int iCharsPerLine) 
	{ 
			m_iCharsPerLine = max(min(iCharsPerLine, m_ptMaxSize.x), m_ptMinSize.x); 
			Resize(NULL); 
	}

	inline int get_CharsPerLine(void) const { return m_iCharsPerLine; }
	inline int get_LinesPerScreen(void) const { return m_iLinesPerScreen; }

	CMoreScreenLayout() : CScreenLayout(), m_iCharsPerLine(64), m_iLinesPerScreen(64), m_iFilePosChars(2 * 8 + 2), m_szFilePosMask(m_sz64bitMask), m_iFilePosWidth(0)  { Initialzie(); }
	virtual void Resize(HWND hWnd);
	TCHAR *get_FilePosMask(void) { return m_szFilePosMask; }

	inline RECT *get_FilePanel(void) { return &m_rcFilePos; }
	inline RECT *get_HeaderPanel(void) { return &m_rcHeader; }
	inline RECT *get_HexPanel(void) { return &m_rcHexDisplay; }
	inline RECT *get_CharPanel(void) { return &m_rcCharDisplay; }

	void put_PixelsPerChar(int iPixelsPerChar) { m_iPixelsPerChar = iPixelsPerChar; }
	inline int get_PixelsPerChar(void) const { return m_iPixelsPerChar; };

	inline int get_ByteDisplayLength(void) const { return 3; }

	void put_FileBuffer(CFileBuffer *fb, HWND hWnd);
};