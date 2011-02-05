// MainFrm.cpp : implmentation of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "aboutdlg.h"
#include "MainFrm.h"
#include "CPidlMgr.h"

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
		m_cmbAddress.Create(m_hWndToolBar, rcDefault, NULL, WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_TABSTOP|CBS_DROPDOWN | CBS_AUTOHSCROLL, 0, m_nAddressBarID);
		m_cmbAddress.SetFont(m_fontAddress);
	}
	AddSimpleReBarBandCtrl(m_hWndToolBar, m_cmbAddress, 0, NULL, TRUE, 0, TRUE);
	m_cmbAddress.SetCueBannerText(_T("Address"));

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

	// Set up the status bar
	{
		m_status.SubclassWindow(m_hWndStatusBar);
		int arrPanes[] = {
			ID_DEFAULT_PANE,
			IDR_STATUS_PANE_ATTRIBUTES,
			IDR_STATUS_PANE_NUM_ITEMS,
			IDR_STATUS_PANE_DISK_FREE
		};
		int nPanes = sizeof(arrPanes) / sizeof(int);
		m_status.SetPanes(arrPanes, nPanes, false);
		int arrWidths[] = { 0, 250, 100, 100 };
		SetPaneWidths(arrWidths, sizeof(arrWidths) / sizeof(int));

		//Load the text
		CString str;
		m_status.SetPaneText(ID_DEFAULT_PANE, _T("Ready")); //, SBT_NOBORDERS);
	}

	return 0;
}

void CMainFrame::SetPaneWidths(int* arrWidths, int nPanes){
    // find the size of the borders

    int arrBorders[3];
    m_status.GetBorders(arrBorders);

    // calculate right edge of default pane (0)
    arrWidths[0] += arrBorders[2];
    for (int i = 1; i < nPanes; i++)
        arrWidths[0] += arrWidths[i];

    // calculate right edge of remaining panes (1 thru nPanes-1)
    for (int j = 1; j < nPanes; j++)
        arrWidths[j] += arrBorders[2] + arrWidths[j - 1];

    // set the pane widths
    m_status.SetParts(m_status.m_nPanes, arrWidths); 
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
	DWORD_PTR dwItemData = combo.GetItemData(nSel);
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

LRESULT CMainFrame::OnFileNewFile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/){
	// TODO: add code to initialize document
	return 0;
}
LRESULT CMainFrame::OnFileRename(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/){
	CColumnPane* active_pane = m_rowMain.GetActivePane();
	if(NULL != active_pane){
		active_pane->RenameSelectedItem();
	}
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

IContextMenu2 *g_pcm2; // these shouldn't be global, they are per-window instance variables
IContextMenu3 *g_pcm3;
IContextMenu *g_pcm;
#define SCRATCH_QCM_FIRST 1
#define SCRATCH_QCM_LAST  0x7FFF
LRESULT CMainFrame::OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled){
	if(wParam){ return 0; } // not menu related
	if(g_pcm2){
		g_pcm2->HandleMenuMsg(uMsg, wParam, lParam);
	}
	return 0;
}
LRESULT CMainFrame::OnInitMenuPopup(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled){
	if(g_pcm2){
		g_pcm2->HandleMenuMsg(uMsg, wParam, lParam);
	}
	return TRUE;
}
LRESULT CMainFrame::OnMenuSelect(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled){
	UINT uItem;
	TCHAR szBuf[MAX_PATH];
	// if this is a shell item, get it's descriptive text
	uItem = (UINT) LOWORD(wParam);   
	if(g_pcm && 0 == (MF_POPUP & HIWORD(wParam)) && uItem >= SCRATCH_QCM_FIRST && uItem <=  SCRATCH_QCM_LAST){
		g_pcm->GetCommandString(uItem-SCRATCH_QCM_FIRST, GCS_HELPTEXTW, NULL, (LPSTR)szBuf, sizeof(szBuf)/sizeof(szBuf[0]) );

		// set the status bar text
		m_status.SetPaneText(ID_DEFAULT_PANE, szBuf);
	}

	return 0;
}

// pt is in screen coords
void OnContextMenu(HWND hwnd, LPSHELLFOLDER lpsfParent, LPITEMIDLIST lpi, POINT pt){
	IContextMenu *pcm;
	if (SUCCEEDED(lpsfParent->GetUIObjectOf(hwnd, 1, (const struct _ITEMIDLIST**)&lpi, IID_IContextMenu, 0, (LPVOID*)&pcm))) {
		HMENU hmenu = CreatePopupMenu();
		if (hmenu) {
			HRESULT qcmResult;
			if(GetKeyState(VK_SHIFT) < 0){
				qcmResult = pcm->QueryContextMenu(hmenu, 0, SCRATCH_QCM_FIRST, SCRATCH_QCM_LAST, CMF_CANRENAME | CMF_EXTENDEDVERBS );
			}else{
				qcmResult = pcm->QueryContextMenu(hmenu, 0, SCRATCH_QCM_FIRST, SCRATCH_QCM_LAST, CMF_CANRENAME | CMF_NORMAL);
			}
			if (SUCCEEDED(qcmResult )) {
				if(SUCCEEDED(pcm->QueryInterface(IID_IContextMenu2, (void**)&g_pcm2))){
					if(SUCCEEDED(pcm->QueryInterface(IID_IContextMenu3, (void**)&g_pcm3))){
						// well, good!
					}else{
						g_pcm3 = NULL;
					}
				}else{
					g_pcm2 = NULL;
				}
				pcm->QueryInterface(IID_IContextMenu3, (void**)&g_pcm3);
				g_pcm = pcm;
				int iCmd = TrackPopupMenuEx(hmenu, TPM_RETURNCMD, pt.x, pt.y, hwnd, NULL);
				g_pcm = NULL;
				if (g_pcm2) {
					g_pcm2->Release();
					g_pcm2 = NULL;
				}
				if (g_pcm3) {
					g_pcm3->Release();
					g_pcm3 = NULL;
				}

				if (iCmd > 0) {
					CHAR szVerb[10];
					if(SUCCEEDED(pcm->GetCommandString(iCmd, GCS_VERBA, NULL, szVerb, sizeof(szVerb)/sizeof(szVerb[0])))
						&& 0 == ::StrCmpA("rename", szVerb))
					{

					}else{
						CMINVOKECOMMANDINFOEX info = { 0 };
						info.cbSize = sizeof(info);
						info.fMask = CMIC_MASK_UNICODE | CMIC_MASK_PTINVOKE;
						if (GetKeyState(VK_CONTROL) < 0) {
							info.fMask |= CMIC_MASK_CONTROL_DOWN;
						}
						if (GetKeyState(VK_SHIFT) < 0) {
							info.fMask |= CMIC_MASK_SHIFT_DOWN;
						}
						info.hwnd = hwnd;
						info.lpVerb  = MAKEINTRESOURCEA(iCmd - SCRATCH_QCM_FIRST);
						info.lpVerbW = MAKEINTRESOURCEW(iCmd - SCRATCH_QCM_FIRST);
						info.nShow = SW_SHOWNORMAL;
						info.ptInvoke = pt;
						pcm->InvokeCommand((LPCMINVOKECOMMANDINFO)&info);
					}
				}
				//char buf[MAX_PATH];
				//pcm->GetCommandString(iCmd - SCRATCH_QCM_FIRST, GCS_VERBA, NULL, buf, MAX_PATH);
			}
			DestroyMenu(hmenu);
		}
		pcm->Release();
	}
}

LRESULT CMainFrame::OnAddressUpdate(int /*id*/, LPNMHDR lParam, BOOL &bHandled){
	CColumnPane::LPNMLISTVIEWSELECTPIDL plvsp = (CColumnPane::LPNMLISTVIEWSELECTPIDL)lParam;
	TCHAR buffer[MAX_PATH];
	BOOL address_obtained = FALSE;

	do{
		CComPtr<IShellFolder> pShellFolder;
		if(S_OK != plvsp->parent_folder->BindToObject(plvsp->selected_pidl, NULL, IID_IShellFolder, (void**)&pShellFolder)){
			break;
		}

		CComPtr<IPersistFolder2> spPersistFolder2;
		if(S_OK != pShellFolder->QueryInterface(IID_IPersistFolder2, (void**)&spPersistFolder2)){
			break;
		}

		LPITEMIDLIST lpi;
		spPersistFolder2->GetCurFolder(&lpi);

		SHGetPathFromIDList(lpi, buffer);
		m_cmbAddress.DeleteItem(0);
		m_cmbAddress.AddItem(buffer, 0, 0, 0, 0);
		m_cmbAddress.SetCurSel(0);
		CPidlMgr::Delete(lpi);
	}while(0);

	if(!address_obtained){
	}
	bHandled = TRUE;
	return 0;
}

LRESULT CMainFrame::OnAddressChanged(int /*id*/, LPNMHDR lParam, BOOL &bHandled){
	// Browse to address
	NMCBEENDEDIT *pnmcbeendedit = (NMCBEENDEDIT*)lParam;
	if(pnmcbeendedit->fChanged && CBENF_RETURN == pnmcbeendedit->iWhy){
		m_rowMain.BrowseToPath(pnmcbeendedit->szText);
	}
	return 0;
}
