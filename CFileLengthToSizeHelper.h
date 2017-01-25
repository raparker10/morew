#include "stdafx.h"

class CFileLengthToSizeHelper {
	TEXTMETRIC m_tm;

	static TCHAR * const m_sz32bitMask;		// the sprintf formatting string for 32-bit files
	static TCHAR * const m_sz64bitMask;		// the sprintf formatting string for 64-bit files
	static const int CHARS_32_BIT_ADDRESS;	// the number of characters required to display a 32-bit address
	static const int CHARS_64_BIT_ADDRESS;	// the number of characters required to display a 64-bit address

	TCHAR *m_szFilePosMask;			// the address format string needed based on the file size
	int m_iFilePosWidthChars;		// the address with in characters

	_int64 m_iFileSizeBytes;		// size of the file being viewed
	int m_iClientWindowSize_X;		// client window size X
	int m_iClientWindowSize_Y;		// client window size Y
	int m_iFilePosWidthPixels;		// width in pixels of the address panel

public:
	// constructor
	CFileLengthToSizeHelper() :
		m_iClientWindowSize_X(0),
		m_iClientWindowSize_Y(0),
		m_iFileSizeBytes(0),
		m_iFilePosWidthPixels(0),
		// assume a 32-bit file size
		m_szFilePosMask(m_sz32bitMask),
		m_iFilePosWidthChars(CHARS_32_BIT_ADDRESS)
	{
	}

	// get the number of characters that can fit into the window at the current resolution
	int get_DefaultWidthChars(void) const
	{
		int chars = (3 * (m_iClientWindowSize_X - m_iFilePosWidthPixels) / 4) / (3 * m_tm.tmAveCharWidth);
		return chars;

	}

	// get the current file position display format
	TCHAR *get_FilePosMask(void) { return m_szFilePosMask; }

	// get the file address width in characers
	int get_FilePosWidthChars(void) const { return m_iFilePosWidthChars; }

	// get the file address width in pixels
	int get_FilePosWidthPixels(void) { return m_iFilePosWidthPixels; }

	// save the Windows textmetrics of the selected font
	void put_TextMetrics(TEXTMETRIC *tm) { 
		memcpy(&m_tm, tm, sizeof(m_tm)); 
		m_iFilePosWidthPixels = m_iFilePosWidthChars * m_tm.tmAveCharWidth;
	}

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

	// save the size of the client window in pixels
	void put_ClientWindowSizePixels(int iWidth, int iHeight) {
		m_iClientWindowSize_X = iWidth;
		m_iClientWindowSize_Y = iHeight;
	}
};