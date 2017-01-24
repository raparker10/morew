#include "stdafx.h"

class CFileLengthToSizeHelper {
	TEXTMETRIC m_tm;

	static TCHAR * const m_sz32bitMask;
	static TCHAR * const m_sz64bitMask;
	static const int CHARS_32_BIT_ADDRESS;
	static const int CHARS_64_BIT_ADDRESS;

	TCHAR *m_szFilePosMask;
	int m_iFilePosWidthChars;

	_int64 m_iFileSizeBytes;
	int m_iClientWindowSize_X;
	int m_iClientWindowSize_Y;
	int m_iFilePosWidthPixels;

public:
	CFileLengthToSizeHelper()
	{
		m_iClientWindowSize_X = 0;
		m_iClientWindowSize_Y = 0;
		m_iFileSizeBytes = 0;
		m_iFilePosWidthPixels;
		// assume a 32-bit file size
		m_szFilePosMask = m_sz32bitMask;
		m_iFilePosWidthChars = CHARS_32_BIT_ADDRESS;
	}
	int get_DefaultWidthChars(void) const
	{
		return (3 * (m_iClientWindowSize_X - m_iFilePosWidthPixels) / 4) / m_tm.tmAveCharWidth;

	}
	TCHAR *get_FilePosMask(void) { return m_szFilePosMask; }
	int get_FilePosWidthChars(void) const { return m_iFilePosWidthChars; }


	void put_TextMetrics(TEXTMETRIC *tm) { 
		memcpy(&m_tm, tm, sizeof(m_tm)); 
		m_iFilePosWidthPixels = m_iFilePosWidthChars * m_tm.tmAveCharWidth;
	}
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
	void put_ClientWindowSize(int iWidth, int iHeight) {
		m_iClientWindowSize_X = iWidth;
		m_iClientWindowSize_Y = iHeight;
	}
	int get_FilePosWidthPixels(void) { return m_iFilePosWidthPixels;  }
};