#include "stdafx.h"

// sprintf formats for displaying file addresses of 32-bit and 64-bit files
TCHAR * const CMoreDrawingHelper::m_sz32bitMask = TEXT("%08X");
TCHAR * const CMoreDrawingHelper::m_sz64bitMask = TEXT("%08X:%08X");

// number of characters required to display the above formats
const int CMoreDrawingHelper::CHARS_32_BIT_ADDRESS = 9;
const int CMoreDrawingHelper::CHARS_64_BIT_ADDRESS = 18;
