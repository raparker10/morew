#include "stdafx.h"

//
// CDrawingHelper
//
// Encapsulates functionality to facilitate drawing.
//


class CDrawingHelper {
protected:

	// screen size
	int m_iClientWidthPixels, m_iClientHeightPixels;

	// font information
	TEXTMETRIC m_tmFixed;
	TEXTMETRIC m_tmVariable;

	// window information
	HWND m_hWnd;

public:
	CDrawingHelper() :
		m_iClientWidthPixels(0),
		m_iClientHeightPixels(0),
		m_hWnd((HWND)INVALID_HANDLE_VALUE)
	{
		memset(&m_tmFixed, 0, sizeof(m_tmFixed));
		memset(&m_tmVariable, 0, sizeof(m_tmVariable));
	}
	inline void put_Window(HWND hWnd)
	{
		m_hWnd = hWnd;
	}
	inline HWND get_Window(void) const
	{
		return m_hWnd;
	}
	void handle_Size(int iSizeType, int iWidth, int iHeight)
	{
		m_iClientWidthPixels = iWidth;
		m_iClientHeightPixels = iHeight;
	}
	inline int get_WindowWidthPixels(void) const
	{
		return m_iClientWidthPixels;
	}
	inline int get_WindowHeightPixels(void) const
	{
		return m_iClientHeightPixels;
	}
	int get_FixedTextWidth(void) 
	{
		return 5; // return get_FixedTextMetric()->tmAveCharWidth;
	}
	TEXTMETRIC *get_FixedTextMetric(void)
	{
		return &m_tmFixed;
	}
	TEXTMETRIC *get_VariableTextMetric(void)
	{
		// return the fixed metri if the variable metric is not defined
		if (m_tmVariable.tmAveCharWidth == 0)
		{
			return get_FixedTextMetric();
		}
		return &m_tmVariable;
	}
	void put_FixedTextMetric(const TEXTMETRIC * const tmFixed)
	{
		memcpy(&m_tmFixed, tmFixed, sizeof(m_tmFixed));
	}
	void put_VariableTextMetric(const TEXTMETRIC * const tmVariable)
	{
		memcpy(&m_tmVariable, tmVariable, sizeof(m_tmVariable));
	}

};

// 
// CMoreDrawingHelper
//
// Drawing helper class specific to the "Morew" file viewing application
//
//

class CMoreDrawingHelper : public CDrawingHelper {
	TEXTMETRIC m_tm;

	static TCHAR * const m_sz32bitMask;		// the sprintf formatting string for 32-bit files
	static TCHAR * const m_sz64bitMask;		// the sprintf formatting string for 64-bit files
	static const int CHARS_32_BIT_ADDRESS;	// the number of characters required to display a 32-bit address
	static const int CHARS_64_BIT_ADDRESS;	// the number of characters required to display a 64-bit address

	TCHAR *m_szFilePosMask;			// the address format string needed based on the file size
	int m_iFilePosWidthChars;		// the address with in characters

	_int64 m_iFileSizeBytes;		// size of the file being viewed
	int m_iFilePosWidthPixels;		// width in pixels of the address panel

public:
	// constructor
	CMoreDrawingHelper() : CDrawingHelper(),
		m_iFileSizeBytes(0),
		m_iFilePosWidthPixels(0),
		// assume a 32-bit file size
		m_szFilePosMask(m_sz32bitMask),
		m_iFilePosWidthChars(CHARS_32_BIT_ADDRESS)
	{
	}

	// get the number of characters that can fit into the window at the current resolution
	int get_DefaultWidthChars(void)
	{
		int clientpix = get_WindowWidthPixels();
		int pospix = get_FilePosWidthPixels();
		int charpix = get_FixedTextMetric()->tmAveCharWidth;

		int chars = (3 * (clientpix - pospix) / 4)
			/ (3 * charpix);
		return chars;
		
	}

	// get the current file position display format
	TCHAR *get_FilePosMask(void) { return m_szFilePosMask; }

	// get the file address width in characers
	int get_FilePosWidthChars(void) const { return m_iFilePosWidthChars; }

	// get the file address width in pixels
	int get_FilePosWidthPixels(void) { return get_FilePosWidthChars() * get_FixedTextMetric()->tmAveCharWidth; }

	// save the size of the file being viewed
	void put_FileSizeBytes(_int64 iFileSizeBytes) { 
		m_iFileSizeBytes = iFileSizeBytes;  

		// assign drawing information based on file size
		if (m_iFileSizeBytes >> 32 > 0)
		{
			m_szFilePosMask = m_sz64bitMask;
			m_iFilePosWidthChars = CHARS_64_BIT_ADDRESS;
		}
		else
		{
			m_szFilePosMask = m_sz32bitMask;
			m_iFilePosWidthChars = CHARS_32_BIT_ADDRESS;
		}
		m_iFilePosWidthPixels = m_iFilePosWidthChars * m_tm.tmAveCharWidth;
	}
};