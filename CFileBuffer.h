#include "stdafx.h"

class CFileBuffer {
	HANDLE m_fh;
	LARGE_INTEGER m_liCurrentFilePos;
	_int64 m_iFileSize, m_iTopOfPagePos;
	char *m_sBuffer;
	int m_iNumberLines, m_iLineSize, m_iBufferSize, m_iAllocatedBufferSize;
	_int64 m_iLinesPerFile;
	TCHAR m_szFileName[_MAX_PATH + 1];
	DWORD m_dwLastError;

	bool ReadLine(int iLine, _int64 iFilePos = -1);
	bool ReadPage(_int64 iFilePos = -1);
	void Initialize(void);
	void Free(void);
	bool AllocateBuffer(int iBufferSize);
	bool FreeBuffer(void);
	inline int PositionToOffset(int iRow, int iCol) const { return m_iLineSize * iRow + iCol; }
	inline char *PositionToAddress(int iRow, int iCol) const { return m_sBuffer + PositionToOffset(iRow, iCol); }
	inline _int64 PositionToFileLocation(int iRow, int iCol) const { return m_iTopOfPagePos + PositionToOffset(iRow, iCol); }
	bool MoveToPosition(_int64 iOffsetFromCurrent);
	bool Read(char *sBuf, DWORD iBytes, DWORD *iBytesRead);
public:
	CFileBuffer(void);
	~CFileBuffer(void);

	bool Open(TCHAR *szFileName);
	bool Close(void);

	bool put_PageSize(int iNumberLines, int iLineSize);
	inline int get_PageLines(void) const { return m_iNumberLines;  }
	inline int get_PageColumns(void) const { return m_iLineSize;  }

	bool get_Line(int iLine, _int64 *iPosition, unsigned char *szBuf, int *iLengthReturned) const;

	inline bool Is64BitFile(void) const { return (m_iFileSize & 0xFFFFFFFF00000000) != 0; }
	inline bool HasOpenFile(void) const { return m_fh != INVALID_HANDLE_VALUE; }

	inline _int64 get_TopOfPagePos(void) const { return m_iTopOfPagePos;  }
	inline _int64 get_FileSize(void) const { return m_iFileSize; }
	inline DWORD get_LastError(void) const { return m_dwLastError; }

	inline HANDLE get_fh(void) const { return m_fh;  }
	bool Seek(_int64 iNewFilePos);
	TCHAR *get_FileName(TCHAR *szFileName, size_t bufsize) const 
	{ 
		wcscpy_s(szFileName, bufsize, m_szFileName); 
		return szFileName; 
	}

	bool Refresh(void);
	bool PageUp(void);
	bool PageDown(void);
	bool LineUp(void);
	bool LineDown(void);
	bool MoveLeft(void);
	bool MoveRight(void);
	bool MoveTop(void);
};