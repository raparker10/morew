#include <Windows.h>

class CFileLengthToSizeHelper {
	TEXTMETRIC m_tm;

public:
	CFileLengthToSizeHelper();
	int put_TextMetrics(TEXTMETRIC *tm) { memcpy(&m_tm, tm, sizeof(m_tm)); }
	int get_WidthChars(int iClientWidth);
};