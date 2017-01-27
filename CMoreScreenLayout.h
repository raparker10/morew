#include "stdafx.h"
#include "CScreenLayout.h"

//
// CMoreScreenLayout
// derived from CScreenLayout
//
// Tracks positions of the various screen "panels"
//

class CFileBuffer;

// the maximum vertical scrollbar position
const int SCROLL_RANGE = 1000;


class CMoreScreenLayout : public CScreenLayout {
private:
	// size of the screen in terms of characters
	int m_iCharsPerLine;
	int m_iLinesPerScreen;

	// location and size of the various screen sections
	RECT m_rcHeader, 
		m_rcFilePos, 
		m_rcHexDisplay, 
		m_rcCharDisplay;

	// Drawing constants.
	// RAP: most of these should come directly from the CMoreDrawingHelper (m_fsh)
	int m_iFilePosChars;
	POINT m_ptMaxSize, m_ptMinSize;
	int m_iPixelsPerChar;
	CMoreDrawingHelper *m_fsh;

	// initialize to arbitrary defaults
	void Initialzie(void) { 
		m_ptMinSize.x = 1; 
		m_ptMinSize.y = 1; 
		m_ptMaxSize.x = 1024; // arbitrary
		m_ptMaxSize.y = 1024; // arbitrary
	}

public:
	// constructor
	CMoreScreenLayout() : CScreenLayout(),
		m_iCharsPerLine(64),
		m_iLinesPerScreen(64)
	{
		Initialzie();
	}

	// screen and line size functions
	inline void put_LinesPerScreen(int iLinesPerScreen) { 
		m_iLinesPerScreen = max(min(m_iLinesPerScreen, m_ptMaxSize.y), m_ptMinSize.y); 
	}
	inline void put_CharsPerLine(int iCharsPerLine) 
	{ 
			m_iCharsPerLine = max(min(iCharsPerLine, m_ptMaxSize.x), m_ptMinSize.x); 
			Resize(NULL); 
	}
	inline int get_CharsPerLine(void) const { return m_iCharsPerLine; }
	inline int get_LinesPerScreen(void) const { return m_iLinesPerScreen; }
	void put_PixelsPerChar(int iPixelsPerChar) {
		m_iPixelsPerChar = iPixelsPerChar;
	}
	inline int get_PixelsPerChar(void) const { return m_iPixelsPerChar; };

	inline int get_ByteDisplayLength(void) const { return 3; }

	// drawing helper functions
	void put_DrawingHelper(CMoreDrawingHelper *fsh)
	{
		m_fsh = fsh;
	}
	CMoreDrawingHelper *get_DrawingHelper(void) { return m_fsh; }

	// message handler functions
	virtual void Resize(HWND hWnd);

	TCHAR *get_FilePosMask(void) { return m_fsh->get_FilePosMask(); }

	// panel position funcitons
	// RAP: use the 'const' functions?
	inline const RECT *get_FilePosPanel(void) const { return &m_rcFilePos; }
	inline const RECT *get_HeaderPanel(void) const { return &m_rcHeader; }
	inline const RECT *get_HexPanel(void) const { return &m_rcHexDisplay; }
	inline const RECT *get_CharPanel(void) const { return &m_rcCharDisplay; }

	inline RECT *get_FilePosPanel(void) { return &m_rcFilePos; }
	inline RECT *get_HeaderPanel(void) { return &m_rcHeader; }
	inline RECT *get_HexPanel(void) { return &m_rcHexDisplay; }
	inline RECT *get_CharPanel(void) { return &m_rcCharDisplay; }

	// file buffer functions RAP: is this still needed?
	void put_FileBuffer(CFileBuffer *fb, HWND hWnd);
};