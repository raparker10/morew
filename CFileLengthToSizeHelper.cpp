#include "stdafx.h"

TCHAR * const CFileLengthToSizeHelper::m_sz32bitMask = TEXT("%08X");
TCHAR * const CFileLengthToSizeHelper::m_sz64bitMask = TEXT("%08X:%08X");
const int CFileLengthToSizeHelper::CHARS_32_BIT_ADDRESS = 9;
const int CFileLengthToSizeHelper::CHARS_64_BIT_ADDRESS = 18;
