#include "stdafx.h"

// seek to a specific location within the file
bool CFileBuffer::Seek(_int64 iNewFilePos)
{
	if (iNewFilePos < 0)
		return true;
	LARGE_INTEGER i, iActualPos;
	i.QuadPart = iNewFilePos;

	if (0 != SetFilePointerEx(m_fh, i, &iActualPos, FILE_BEGIN))
	{
		m_liCurrentFilePos.QuadPart = iActualPos.QuadPart;
		m_iTopOfPagePos = iActualPos.QuadPart;
		return true;
	}
	return false;
}

// read from a file
bool CFileBuffer::Read(char *sBuf, DWORD iBytes, DWORD *iBytesRead = NULL)
{
	*sBuf = 0;
	DWORD liBytesRead;
	if (iBytesRead != NULL)
	{
		*iBytesRead = 0;
	}
	if (0 == ReadFile(m_fh, sBuf, iBytes, &liBytesRead, NULL))
	{
		LARGE_INTEGER i;
		i.QuadPart = 0;
		SetFilePointerEx(m_fh, i, &m_liCurrentFilePos, FILE_CURRENT);
		return false;
	}
//	sBuf[liBytesRead] = 0; //RAP: why does uncommenting this and maximizing the screen cause a creash due to heap corruption?
	if (iBytesRead != NULL)
	{
		*iBytesRead = liBytesRead;
	}
	return true;

}
bool CFileBuffer::ReadLine(int iLine, _int64 iFilePos)
{
	if (Seek(iFilePos))
	{
		return Read(PositionToAddress(iLine, 0), m_iLineSize);
	}
	return false;
}
bool CFileBuffer::ReadPage(_int64 iFilePos)
{
	if (Seek(iFilePos))
	{
		return Read(m_sBuffer, m_iBufferSize);
	}
	return false;
}

bool CFileBuffer::AllocateBuffer(int iBufferSize)
{
	if (iBufferSize > m_iAllocatedBufferSize)
	{
		free(m_sBuffer);
		m_sBuffer = (char*)malloc(iBufferSize);
		if (m_sBuffer == NULL)
		{
			m_iBufferSize = m_iAllocatedBufferSize = 0;
		}
		else
		{
			m_iBufferSize = m_iAllocatedBufferSize = iBufferSize;
		}
	}
	else
	{
		m_iBufferSize = iBufferSize;
	}
	return m_sBuffer != NULL;
}

void CFileBuffer::Initialize(void)
{
	m_fh = INVALID_HANDLE_VALUE;
	m_liCurrentFilePos.QuadPart = 0;
	m_iFileSize = 0;
	m_sBuffer = NULL;
	m_dwLastError = 0;
	m_iTopOfPagePos = 0;
	m_iAllocatedBufferSize = 0;
	m_iBufferSize = 0;
	m_iLineSize = 0;
	m_iLinesPerFile = 0;
	m_iNumberLines = 0;
}
bool CFileBuffer::Close(void)
{
	if (m_fh != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_fh);
		m_fh = INVALID_HANDLE_VALUE;
	}
	m_liCurrentFilePos.QuadPart = 0;
	m_iTopOfPagePos = 0;
	m_iFileSize = 0;
	m_szFileName[0] = 0;
	return true;
}
void CFileBuffer::Free(void)
{
	Close();
}
CFileBuffer::CFileBuffer(void)
{
	Initialize();
}
CFileBuffer::~CFileBuffer(void)
{
	Free();
}

bool CFileBuffer::Open(TCHAR *szFileName)
{
	Close();

	m_fh = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_READ_ATTRIBUTES, NULL);
	if (m_fh == INVALID_HANDLE_VALUE)
	{
		this->m_dwLastError = GetLastError();
		return false;
	}

	LARGE_INTEGER liFileSize;
	GetFileSizeEx(m_fh, &liFileSize);

	m_iFileSize = liFileSize.QuadPart;
	if (m_iLineSize <= 0)
	{
		m_iLinesPerFile = 0;
	}
	else
	{
		m_iLinesPerFile = m_iFileSize / m_iLineSize;
	}
	m_liCurrentFilePos.QuadPart = 0;
	m_iTopOfPagePos = 0;

	wcscpy_s(m_szFileName, _countof(m_szFileName), szFileName);

	return true;
}
bool CFileBuffer::put_PageSize(int iNumberLines, int iLineSize)
{
	// there must be at least one line and one character
	if (iNumberLines < 1 || iLineSize < 1)
		return false;

	if (AllocateBuffer(iNumberLines * iLineSize))
	{
		m_iLineSize = iLineSize;
		m_iNumberLines = iNumberLines;
		if (HasOpenFile())
		{
			return Refresh();
		}
		return true;
	}
	return false;
}

// load one page of data from the top-of-page location
bool CFileBuffer::Refresh(void)
{
	if (Seek(m_iTopOfPagePos))
	{
		return Read(m_sBuffer, m_iBufferSize);
	}
	return false;
}
bool CFileBuffer::MoveToPosition(_int64 iOffsetFromCurrent)
{
	// update the top-of-page file position
	m_iTopOfPagePos += iOffsetFromCurrent;

	// if we've moved past the end of the file, then back it up
	if (m_iTopOfPagePos >= m_iFileSize)
		m_iTopOfPagePos = m_iFileSize - m_iBufferSize;

	// if we've moved past the beginning of the file, then set to the beginning
	if (m_iTopOfPagePos < 0)
		m_iTopOfPagePos = 0;

	return true;
}

bool CFileBuffer::PageUp(void)
{
	if (MoveToPosition(-m_iBufferSize))
	{
		if (Seek(m_iTopOfPagePos))
		{
			return ReadPage();
		}
	}
	return false;
}
bool CFileBuffer::PageDown(void)
{
	if (MoveToPosition(m_iBufferSize))
	{
		if (Seek(m_iTopOfPagePos))
		{
			return ReadPage();
		}
	}
	return false;
}
bool CFileBuffer::LineUp(void)
{
	// scroll the buffer down one line and just read the first line in
	_int64 iOldPos = m_iTopOfPagePos;

	if (MoveToPosition(-m_iLineSize))
	{
		// determine how far the position *actually* moved (it might be different that what was requested)
		if (Seek(m_iTopOfPagePos))
		{
			return ReadPage(); // for now just read the entire page
		}
	}
	return false;

}
bool CFileBuffer::LineDown(void)
{
	// scroll the buffer down one line and just read the first line in
	_int64 iOldPos = m_iTopOfPagePos;

	if (MoveToPosition(m_iLineSize))
	{
		// determine how far the position *actually* moved (it might be different that what was requested)
		if (Seek(m_iTopOfPagePos))
		{
			return ReadPage(); // for now just read the entire page
		}
	}
	return false;
}
bool CFileBuffer::MoveLeft(void)
{
	// scroll the buffer down one line and just read the first line in
	_int64 iOldPos = m_iTopOfPagePos;

	if (MoveToPosition(-1))
	{
		// determine how far the position *actually* moved (it might be different that what was requested)
		if (Seek(m_iTopOfPagePos))
		{
			return ReadPage(); // for now just read the entire page
		}
	}
	return false;
}
bool CFileBuffer::MoveRight(void)
{
	// scroll the buffer down one line and just read the first line in
	_int64 iOldPos = m_iTopOfPagePos;

	if (MoveToPosition(1))
	{
		// determine how far the position *actually* moved (it might be different that what was requested)
		if (Seek(m_iTopOfPagePos))
		{
			return ReadPage(); // for now just read the entire page
		}
	}
	return false;
}
bool CFileBuffer::MoveTop(void)
{
	if (MoveToPosition(0))
	{
		if (Seek(m_iTopOfPagePos))
		{
			return ReadPage();
		}
	}
	return false;
}

bool CFileBuffer::get_Line(int iLine, _int64 *iPosition, unsigned char *szBuf, int *iLengthReturned) const
{
	// always terminate the buffer first
	*szBuf = 0;
	*iLengthReturned = 0;
	*iPosition = 0;

	// make sure the line number is valid
	if (iLine < 0 || iLine >= this->m_iNumberLines)
		return false;

	// copy the line to the provided buffer
	memcpy(szBuf, PositionToAddress(iLine, 0), m_iLineSize); // get the line
	szBuf[m_iLineSize] = 0; // terminate the line

	// also return the file position of the first byte
	*iPosition = PositionToFileLocation(iLine, 0);

	// if all characters are before the end of the file, then return with success now
	if (m_iTopOfPagePos + PositionToOffset(iLine, m_iLineSize) < m_iFileSize)
	{
		*iLengthReturned = m_iLineSize;
		return true;
	}

	// if the requested line extends beyond the end of the file, then terminate the line before its end
	_int64 pos = max(0, m_iFileSize - m_iTopOfPagePos - PositionToOffset(iLine, 0));
	if (pos < m_iLineSize)
	{
		szBuf[pos] = 0;
		*iLengthReturned = (int)pos;
	}
	return true;
}
