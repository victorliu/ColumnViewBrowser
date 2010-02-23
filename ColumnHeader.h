#pragma once


// The display text is kept in the window text (use SetWindowText)
class CColumnHeader : public CWindowImpl<CColumnHeader>
{
protected:
	int m_nDesiredHeight;
	static const int m_nPadding = 1;
	BOOL m_bActive;
public:
	DECLARE_WND_CLASS_EX(_T("ColumnPaneHeader"), 0, -1)

	enum{
		m_NotifyCodeResize = 1
	};
	typedef struct{
		int nMinHeight;
	} CreateParams, *LPCREATEPARAMS;

	BEGIN_MSG_MAP_EX(CColumnHeader)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_PAINT(OnPaint)
		MSG_WM_SETTEXT(OnSetText)
		/*
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
		MESSAGE_HANDLER(WM_COMMAND, OnCommand)
		*/
		FORWARD_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnCreate(LPCREATESTRUCT cs);
	void OnPaint(CDCHandle dc);
	void DrawHeader(CDCHandle dc);
	int OnSetText(LPCTSTR lpstrText);

	int GetDesiredHeight() const{ return m_nDesiredHeight; }

	void SetActiveState(BOOL bActive);
};
