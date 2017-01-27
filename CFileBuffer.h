#include "stdafx.h"

//
// CFileBuffer
//
// Manages the buffering of a file to match a specified screen size.
// This helps keep just enough of the file in memory to facilitate
// screen drawing.  In principal, this could also be provided with
// a memory-mapped file at the risk of portability issues
//

class CFileBuffer {
	HANDLE m_fh;
	LARGE_INTEGER m_liCurrentFilePos;
	_int64 m_iFileSize, m_iTopOfPagePos;
	char *m_sBuffer;
	int m_iNumberLines, m_iLineSize, m_iBufferSize, m_iAllocatedBufferSize;
	_int64 m_iLinesPerFile;
	TCHAR m_szFileName[_MAX_PATH + 1];
	DWORD m_dwLastError;

	// startup / shutdown functions
	void Initialize(void);
	void Free(void);

	// file reading functions
	bool ReadLine(int iLine, _int64 iFilePos = -1);
	bool ReadPage(_int64 iFilePos = -1);
	bool Read(char *sBuf, DWORD iBytes, DWORD *iBytesRead);

	// buffer functions
	bool AllocateBuffer(int iBufferSize);
	bool FreeBuffer(void);

	// file positioning functions
	inline int PositionToOffset(int iRow, int iCol) const { return m_iLineSize * iRow + iCol; }
	inline char *PositionToAddress(int iRow, int iCol) const { return m_sBuffer + PositionToOffset(iRow, iCol); }
	inline _int64 PositionToFileLocation(int iRow, int iCol) const { return m_iTopOfPagePos + PositionToOffset(iRow, iCol); }
	bool MoveToPosition(_int64 iOffsetFromCurrent);

public:
	// object startup and shutdown
	CFileBuffer(void);
	~CFileBuffer(void);

	// file open and close
	bool Open(TCHAR *szFileName);
	bool Close(void);

	// page size functions
	bool put_PageSize(int iNumberLines, int iLineSize);
	inline int get_PageLines(void) const { return m_iNumberLines;  }
	inline int get_PageColumns(void) const { return m_iLineSize;  }

	// data retrieval funcitons
	bool get_Line(int iLine, _int64 *iPosition, unsigned char *szBuf, int *iLengthReturned) const;

	// file information functions
	inline bool Is64BitFile(void) const { return (m_iFileSize & 0xFFFFFFFF00000000) != 0; }
	inline _int64 get_FileSize(void) const { return m_iFileSize; }
	inline _int64 get_TopOfPagePos(void) const { return m_iTopOfPagePos; }
	TCHAR *get_FileName(TCHAR *szFileName, size_t bufsize) const
	{
		wcscpy_s(szFileName, bufsize, m_szFileName);
		return szFileName;
	}

	// status functions
	inline bool HasOpenFile(void) const { return m_fh != INVALID_HANDLE_VALUE; }
	inline DWORD get_LastError(void) const { return m_dwLastError; }

	// file handle functions
	inline HANDLE get_fh(void) const { return m_fh;  }

	// file positioning functions
	bool Seek(_int64 iNewFilePos);
	bool Refresh(void);
	bool PageUp(void);
	bool PageDown(void);
	bool LineUp(void);
	bool LineDown(void);
	bool MoveLeft(void);
	bool MoveRight(void);
	bool MoveTop(void);
};