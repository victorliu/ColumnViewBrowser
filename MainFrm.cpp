// MainFrm.cpp : implmentation of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "aboutdlg.h"
#include "MainFrm.h"

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	/*
	if(CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
		return TRUE;

	return FALSE;
	*/
	return CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg);
}

BOOL CMainFrame::OnIdle()
{
	UIUpdateToolBar();
	return FALSE;
}

LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// create command bar window
	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	// attach menu
	m_CmdBar.AttachMenu(GetMenu());
	// load command bar images
	m_CmdBar.LoadImages(IDR_MAINFRAME);
	//m_CmdBar.LoadImages(IDR_MENU_IMAGES);
	// remove old menu
	SetMenu(NULL);

	HWND hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);
	
	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(hWndCmdBar);
	AddSimpleReBarBand(hWndToolBar, NULL, TRUE);
	
	{
		int nWidth = 80;
		int nHeight = 16;
		// Get the size of the combobox font
		NONCLIENTMETRICS ncm;
		ncm.cbSize = sizeof(NONCLIENTMETRICS);
		::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0);
		// Create menu font
		m_fontAddress.CreateFontIndirect(&ncm.lfMenuFont);
		ATLASSERT(m_fontAddress != NULL);
		// We need a temporary DC
		CClientDC dc(m_hWnd);
		// Select in the menu font
		CFontHandle fontOld = dc.SelectFont(m_fontAddress);
		// Get the font size
		TEXTMETRIC tm;
		dc.GetTextMetrics(&tm);
		// Done with the font
		dc.SelectFont(fontOld);
		// Return the width and height
		CSize sizeFont(tm.tmAveCharWidth, tm.tmHeight + tm.tmExternalLeading);
		
		// Compute the width and height
		UINT cx = (nWidth + 8) * sizeFont.cx;
		UINT cy = nHeight * sizeFont.cy;
		m_cmbAddress.Create(m_hWndToolBar, rcDefault, NULL, WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_TABSTOP|CBS_DROPDOWN | CBS_AUTOHSCROLL);
		m_cmbAddress.SetFont(m_fontAddress);
	}
	AddSimpleReBarBandCtrl(m_hWndToolBar, m_cmbAddress, 0, NULL, TRUE, 0, TRUE);
	m_cmbAddress.AddString(_T("Address"));
	m_cmbAddress.SetCurSel(0);

	CreateSimpleStatusBar();
	UIAddToolBar(hWndToolBar);
	UIAddChildWindowContainer(m_cmbAddress);
	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	m_wndSplit.Create(m_hWnd, rcDefault);
	
	// Create left pane
	m_lstRoots.Create(m_wndSplit, rcDefault, NULL, 0, 0, m_nRootsListID);
	// Create right pane
	m_rowMain.Create(m_wndSplit, rcDefault);

	m_hWndClient = m_wndSplit;

	m_wndSplit.SetSplitterPanes(m_lstRoots, m_rowMain);
	UpdateLayout();
	m_wndSplit.SetSplitterPos(200);
	m_wndSplit.SetSplitterExtendedStyle(0);

	m_lstRoots.Init();

	return 0;
}

LRESULT CMainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	DeleteObject(m_fontAddress);

	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
//	pLoop->RemoveIdleHandler(this);

	bHandled = FALSE;
	return 1;
}

/*
BOOL CMainFrame::OnEraseBkgnd(CDCHandle dc){
	return TRUE;
}*/
LRESULT CMainFrame::OnSelChange(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
{
	// Get the newly selected item index
	CComboBox combo(hWndCtl);
	int nSel = combo.GetCurSel();
	// Get the item text
	WTL::CString strItemText;
	combo.GetLBText(nSel, strItemText);
	// Get the item data
	DWORD dwItemData = combo.GetItemData(nSel);
	// Call special function to handle the selection change
	//OnToolBarCombo(combo, wID, nSel, strItemText, dwItemData);
	// Set focus to the main window
	SetFocus();
	return TRUE;
}
LRESULT CMainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	PostMessage(WM_CLOSE);
	return 0;
}

LRESULT CMainFrame::OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: add code to initialize document

	return 0;
}

LRESULT CMainFrame::OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	static BOOL bVisible = TRUE;	// initially visible
	bVisible = !bVisible;
	CReBarCtrl rebar = m_hWndToolBar;
	int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);	// toolbar is 2nd added band
	rebar.ShowBand(nBandIndex, bVisible);
	UISetCheck(ID_VIEW_TOOLBAR, bVisible);

	nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 2);	// toolbar is 3rd added band
	rebar.ShowBand(nBandIndex, bVisible);
	UISetCheck(ID_VIEW_TOOLBAR, bVisible);

	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
	::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}
LRESULT CMainFrame::OnRootsItemChanged(int /*id*/, LPNMHDR lParam, BOOL &bHandled){
	bHandled = FALSE;
	LPNMLISTVIEW plv = (LPNMLISTVIEW)lParam; // only works for comctl32.dll >= v4.7, IE4 >= 4.0
	if((plv->uChanged & LVIF_STATE) && (plv->uNewState & LVIS_FOCUSED)){
		m_rowMain.NewRootPane(m_lstRoots.GetSelectedPIDL(plv->iItem));
	}
	return 0;
}