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

// read a line from a specific location in the current file
bool CFileBuffer::ReadLine(int iLine, _int64 iFilePos)
{
	if (Seek(iFilePos))
	{
		return Read(PositionToAddress(iLine, 0), m_iLineSize);
	}
	return false;
}

// read a page from a specific location in the file
bool CFileBuffer::ReadPage(_int64 iFilePos)
{
	if (Seek(iFilePos))
	{
		return Read(m_sBuffer, m_iBufferSize);
	}
	return false;
}

// allocate a buffer to hold a page worth of data
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

// initialize the object's variables
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
	m_szFileName[0] = 0;
}

// close the file
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

// free any used resources
void CFileBuffer::Free(void)
{
	Close();
}

// Constructor
CFileBuffer::CFileBuffer(void)
{
	Initialize();
}

// destructor
CFileBuffer::~CFileBuffer(void)
{
	Free();
}

// open a new file
bool CFileBuffer::Open(TCHAR *szFileName)
{
	Close();

	m_fh = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_READ_ATTRIBUTES, NULL);
	if (m_fh == INVALID_HANDLE_VALUE)
	{
		// if the file open fails, save the error code
		this->m_dwLastError = GetLastError();

		// return failure
		return false;
	}

	// determine the file length
	LARGE_INTEGER liFileSize;
	GetFileSizeEx(m_fh, &liFileSize);
	m_iFileSize = liFileSize.QuadPart;

	// determine the number of lines in the file
	if (m_iLineSize <= 0)
	{
		m_iLinesPerFile = 0;
	}
	else
	{
		m_iLinesPerFile = m_iFileSize / m_iLineSize;
	}

	// reset the current position to the top of the file
	m_liCurrentFilePos.QuadPart = 0;
	m_iTopOfPagePos = 0;

	// save the file name
	wcscpy_s(m_szFileName, _countof(m_szFileName), szFileName);

	// return success
	return true;
}

// set the number of rows and columns in the current display
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
			// refresh the buffer with the new file size
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

// move the file pointer to a new location via an offset
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

// read one page before the current file location
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

// read one page down from the current location
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

// read one line up from the current location
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

// read one line down from the current location
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

// read one chararacter before the current location
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

// read one character after the current location
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

// move to the top of the file
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

// read one line from the file from a specified location
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
