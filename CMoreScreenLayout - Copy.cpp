#include "stdafx.h"
#include "CFileBuffer.h"
#include "CMoreScreenLayout.h"

TCHAR *CMoreScreenLayout::m_sz32bitMask = TEXT("%08X");
TCHAR *CMoreScreenLayout::m_sz64bitMask = TEXT("%08X:%08X");
int CMoreScreenLayout::CHARS_32_BIT_ADDRESS = 9;
int CMoreScreenLayout::CHARS_64_BIT_ADDRESS = 18;

// adjust the size and location of screen display panels based on a new client window size
void CMoreScreenLayout::Resize(HWND hWnd)
{
	static const int HEADER_LINES = 3;
	static const int BYTE_DISPLAY_LENGTH = 3;

	if (hWnd != NULL)
	{
		CScreenLayout::Resize(hWnd);
	}
	int h = get_ClientHeight();
	int th = get_TextMetrics()->tmHeight;

	put_LinesPerScreen(h / th - HEADER_LINES);

	// setup the header area
	CopyRect(get_HeaderPanel(), get_ClientRect());
	m_rcHeader.bottom = HEADER_LINES * get_TextMetrics()->tmHeight;

	// setup the file position area
	m_rcFilePos.top = m_rcHeader.bottom + 1;
	m_rcFilePos.bottom = get_ClientRect()->bottom;
	m_rcFilePos.left = get_ClientRect()->left;
	m_rcFilePos.right = get_ClientRect()->left + m_iFilePosChars * get_PixelsPerChar();

	// setup the hex display area
	m_rcHexDisplay.top = m_rcFilePos.top;
	m_rcHexDisplay.bottom = m_rcFilePos.bottom;
	m_rcHexDisplay.left = m_rcFilePos.right + 1;
	m_rcHexDisplay.right = m_rcHexDisplay.left + (get_ByteDisplayLength() * get_CharsPerLine() * get_PixelsPerChar());

	// setup the character display area
	m_rcCharDisplay.top = m_rcHexDisplay.top;
	m_rcCharDisplay.bottom = m_rcHexDisplay.bottom;
	m_rcCharDisplay.left = m_rcHexDisplay.right + 1;
	m_rcCharDisplay.right = get_ClientRect()->right;

}

// inform the layout object about the file to be displayed
void CMoreScreenLayout::put_FileBuffer(CFileBuffer *fb, HWND hWnd)
{
	if (fb->Is64BitFile())
	{
		m_szFilePosMask = m_sz64bitMask;
		m_iFilePosChars = CHARS_64_BIT_ADDRESS;
	}
	else
	{
		m_szFilePosMask = m_sz32bitMask;
		m_iFilePosChars = CHARS_32_BIT_ADDRESS;
	}
	m_iFilePosWidth = m_iPixelsPerChar * m_iFilePosChars;
	Resize(hWnd);
}
