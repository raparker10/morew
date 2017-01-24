#include "stdafx.h"
#include "CScreenLayout.h"

class CFileBuffer;

// screen layout
enum Section { HEADER, FILEPOS, HEXDISPLAY, CHARDISPLAY, UNKNOWN };
const int SCROLL_RANGE = 1000;


class CMoreScreenLayout : public CScreenLayout {
private:
	int m_iCharsPerLine;
	int m_iLinesPerScreen;
	RECT m_rcHeader, 
		m_rcFilePos, 
		m_rcHexDisplay, 
		m_rcCharDisplay;


	int m_iFilePosChars;
	POINT m_ptMaxSize, m_ptMinSize;
	int m_iPixelsPerChar;
	CFileLengthToSizeHelper *m_fsh;

	void Initialzie(void) { 
		m_ptMinSize.x = 1; 
		m_ptMinSize.y = 1; 
		m_ptMaxSize.x = 1024; 
		m_ptMaxSize.y = 1024; 
	}

public:
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
	int get_DefaultCharsPerLine(const int iClientWidth) const;

	CMoreScreenLayout(CFileLengthToSizeHelper *fsh) : CScreenLayout(), 
		m_iCharsPerLine(64), 
		m_iLinesPerScreen(64), 
		m_fsh(fsh)
	{ 
		Initialzie(); 
	}
	virtual void Resize(HWND hWnd);
	TCHAR *get_FilePosMask(void) { return m_fsh->get_FilePosMask(); }

	inline const RECT *get_FilePosPanel(void) const { return &m_rcFilePos; }
	inline const RECT *get_HeaderPanel(void) const { return &m_rcHeader; }
	inline const RECT *get_HexPanel(void) const { return &m_rcHexDisplay; }
	inline const RECT *get_CharPanel(void) const { return &m_rcCharDisplay; }

	inline RECT *get_FilePosPanel(void) { return &m_rcFilePos; }
	inline RECT *get_HeaderPanel(void) { return &m_rcHeader; }
	inline RECT *get_HexPanel(void) { return &m_rcHexDisplay; }
	inline RECT *get_CharPanel(void) { return &m_rcCharDisplay; }

	void put_PixelsPerChar(int iPixelsPerChar) { 
		m_iPixelsPerChar = iPixelsPerChar; 
	}
	inline int get_PixelsPerChar(void) const { return m_iPixelsPerChar; };

	inline int get_ByteDisplayLength(void) const { return 3; }

	void put_FileBuffer(CFileBuffer *fb, HWND hWnd);
};