class CApplication {
	HWND m_hWnd;
public:
	CApplication();
	~CApplication();
	
	static LRESULT CALLBACK CApplication::MessageProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	virtual bool handle_paint(HWND hWnd, HDC hdc, LPPAINTSTRUCT ps) { return false; }
	virtual bool handle_create(HWND hWnd, LPCREATESTRUCT *lpcs) { return false; }
	virtual bool handle_command(HWND hWnd, int wmID, int wmEvent) { return false; }
	virtual bool handle_keydown(HWND hWnd, int iVKey) { return false; }
	virtual bool handle_char(HWND hWnd, int iChar) { return false; }
	virtual bool handle_vscroll(HWND hWnd, int iScrollRequest, int iScrollPosition) { return false; }
	virtual bool handle_size(HWND hWnd, HDC hdc, int iWidth, int iHeight) { return false; }
	virtual bool handle_mousemove(HWND hWnd, WPARAM wParam, WORD x, WORD y) { return false; }
};