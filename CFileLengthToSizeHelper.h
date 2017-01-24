#include "stdafx.h"

class CFileLengthToSizeHelper {
	TEXTMETRIC m_tm;

	static TCHAR * const m_sz32bitMask;
	static TCHAR * const m_sz64bitMask;
	static const int CHARS_32_BIT_ADDRESS;
	static const int CHARS_64_BIT_ADDRESS;

	TCHAR *m_szFilePosMask;
	int m_iFilePosChars;

	_int64 m_iFileLength;
	int m_iClientWindowSize_X;
	int m_iClientWindowSize_Y;
	int m_iFilePosWidth;

public:
	CFileLengthToSizeHelper()
	{
		m_iClientWindowSize_X = 0;
		m_iClientWindowSize_Y = 0;
		m_iFileLength = 0;
		m_iFilePosWidth;
		// assume a 32-bit file size
		m_szFilePosMask = m_sz32bitMask;
		m_iFilePosChars = CHARS_32_BIT_ADDRESS;
	}
	int get_DefaultWidthChars(int iClientWidth, int iFilePosWidth)
	{
		return 32; //  return (3 * (iClientWidth - m_iFilePosWidth) / 4) / m_iPixelsPerChar;

	}
	TCHAR *get_FilePosMask(void) { return m_szFilePosMask; }
	int get_FilePosChars(void) const { return m_iFilePosChars; }


	void put_TextMetrics(TEXTMETRIC *tm) { 
		memcpy(&m_tm, tm, sizeof(m_tm)); 
		m_iFilePosWidth = m_iFilePosChars * m_tm.tmAveCharWidth;
	}
	void put_FileLength(_int64 iFileLength) { 
		m_iFileLength = iFileLength;  

		// assign drawing information based on file size
		if (m_iFileLength >> 32 > 0)
		{
			m_szFilePosMask = m_sz64bitMask;
			m_iFilePosChars = CHARS_64_BIT_ADDRESS;
		}
		else
		{
			m_szFilePosMask = m_sz32bitMask;
			m_iFilePosChars = CHARS_32_BIT_ADDRESS;
		}
		m_iFilePosWidth = m_iFilePosChars * m_tm.tmAveCharWidth;
	}
	void put_ClientWindowSize(int iWidth, int iHeight) {
		m_iClientWindowSize_X = iWidth;
		m_iClientWindowSize_Y = iHeight;
	}
	int get_FilePosWidth(void) { return m_iFilePosWidth;  }
};