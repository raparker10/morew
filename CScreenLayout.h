#include "stdafx.h"

// RAP this is a test of git change management - again

class CScreenLayout {
	RECT m_rcClient;
	TEXTMETRIC m_tm;

protected:

public:
	CScreenLayout(void) {}
	inline void put_TextMetrics(TEXTMETRIC *tm) { memcpy(&m_tm, tm, sizeof(tm)); }
	virtual void Resize(HWND hWnd);

	inline RECT *get_ClientRect(void) { return &m_rcClient;  }
	inline TEXTMETRIC *get_TextMetrics(void) { return &m_tm; }
	inline int get_ClientHeight(void) const { return m_rcClient.bottom - m_rcClient.top;  }
};


