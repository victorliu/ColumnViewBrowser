#include "StdAfx.h"
#include "ColumnPane.h"
#include "RowContainer.h"
#include "CPidlMgr.h"
#include <shlwapi.h>
#include "MainFrm.h"

LRESULT CColumnPane::FillLV(){
	// Populate list
	LPSHELLFOLDER pShellFolder;
	if(S_OK != spParentFolder->BindToObject(m_pidl, NULL, IID_IShellFolder, (void**)&pShellFolder)){
		return FALSE;
	}

	CComPtr<IEnumIDList> spEnumIDList;
	HRESULT hr = pShellFolder->EnumObjects(m_list.GetParent(), SHCONTF_FOLDERS | SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN, &spEnumIDList);
	if (FAILED(hr))
		return FALSE;

	LPITEMIDLIST lpi;
	ULONG ulFetched = 0;
	UINT uFlags = 0;
	LVITEM lvi = { 0 };
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	int iCtr = 0;

	while (spEnumIDList->Next(1, &lpi, &ulFetched) == S_OK)
	{
		// Get some memory for the ITEMDATA structure.
		LPLVITEMDATA lplvid = new LVITEMDATA;
		if (lplvid == NULL) 
			return FALSE;

		// Since you are interested in the display attributes as well as other attributes, 
		// you need to set ulAttrs to SFGAO_DISPLAYATTRMASK before calling GetAttributesOf()
		ULONG ulAttrs = SFGAO_DISPLAYATTRMASK;
		hr = pShellFolder->GetAttributesOf(1, (const struct _ITEMIDLIST **)&lpi, &ulAttrs);
		if(FAILED(hr)) 
			return FALSE;

		lplvid->ulAttribs = ulAttrs;

		lvi.iItem = iCtr;
		lvi.iSubItem = 0;
		lvi.pszText = LPSTR_TEXTCALLBACK;
		lvi.cchTextMax = MAX_PATH;
		uFlags = SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON;
		lvi.iImage = I_IMAGECALLBACK;

		lplvid->spParentFolder.p = pShellFolder; pShellFolder->AddRef();

		// Now make a copy of the ITEMIDLIST
		lplvid->lpi= CPidlMgr::Copy(lpi);

		lvi.lParam = (LPARAM)lplvid;

		// Add the item to the list view control
		int n = m_list.InsertItem(&lvi);
		m_list.AddItem(n, 1, LPSTR_TEXTCALLBACK, I_IMAGECALLBACK);
		m_list.AddItem(n, 2, LPSTR_TEXTCALLBACK, I_IMAGECALLBACK);
		//m_list.AddItem(n, 3, LPSTR_TEXTCALLBACK, I_IMAGECALLBACK);
		//m_list.AddItem(n, 4, LPSTR_TEXTCALLBACK, I_IMAGECALLBACK);

		iCtr++;
		CPidlMgr::Delete(lpi); // free PIDL the shell gave you
	}

	//SortData sd(m_nSort, m_bReverseSort);
	SortData sd(0, false);
	m_list.SortItems(CColumnPane::ListViewCompareProc, (LPARAM)&sd);
	return TRUE;
}
void CColumnPane::InitLV(){
	// Get Desktop folder
	LPITEMIDLIST spidl;
	HRESULT hRet = ::SHGetSpecialFolderLocation(m_hWnd, CSIDL_DESKTOP, &spidl);
	hRet;	// avoid level 4 warning
	ATLASSERT(SUCCEEDED(hRet));

	// Get system image lists
	SHFILEINFO sfi = { 0 };
	HIMAGELIST hImageList = (HIMAGELIST)::SHGetFileInfo((LPCWSTR)spidl, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_ICON);
	ATLASSERT(hImageList != NULL);

	memset(&sfi, 0, sizeof(SHFILEINFO));
	HIMAGELIST hImageListSmall = (HIMAGELIST)::SHGetFileInfo((LPCWSTR)spidl, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
	ATLASSERT(hImageListSmall != NULL);

	// Create list view columns
	m_list.InsertColumn(0, _T("Name"), LVCFMT_LEFT, 100, 0);
	m_list.InsertColumn(1, _T("Size"), LVCFMT_RIGHT, 50, 1);
	m_list.InsertColumn(2, _T("Modified"), LVCFMT_LEFT, 50, 3);
	//m_list.InsertColumn(2, _T("Type"), LVCFMT_LEFT, 20, 2);
	//m_list.InsertColumn(3, _T("Modified"), LVCFMT_LEFT, 30, 3);
	//m_list.InsertColumn(4, _T("Attributes"), LVCFMT_RIGHT, 20, 4);

	// Set list view image lists
	m_list.SetImageList(hImageList, LVSIL_NORMAL);
	m_list.SetImageList(hImageListSmall, LVSIL_SMALL);
}



CColumnPane::CColumnPane():g_IContext2(NULL),g_IContext3(NULL){}
CColumnPane::~CColumnPane(){
}

void CColumnPane::OnNcDestroy(){
	if(NULL != m_pidl){
		CPidlMgr::Delete(m_pidl);
	}
}
void CColumnPane::OnDestroy(){
	m_list.DestroyWindow();
	m_list.m_hWnd = NULL;
	m_header.DestroyWindow();
	m_header.m_hWnd = NULL;

	pShellFolder->Release();
	pShellDetails->Release();
}
int CColumnPane::OnCreate(LPCREATESTRUCT lpCreateStruct){
	LPCreateParams pcp = (LPCreateParams)lpCreateStruct->lpCreateParams;
	if(NULL != pcp){
		spParentFolder = pcp->parent_folder;
		m_pidl = CPidlMgr::Copy(pcp->rel_pidl);
	}else{
		spParentFolder = NULL;
		m_pidl = NULL;
	}

	m_nSizeBoxWidth = GetSystemMetrics(SM_CXVSCROLL);
	m_nSizeBoxHeight = GetSystemMetrics(SM_CYHSCROLL);

	CColumnHeader::CreateParams cp;
	cp.nMinHeight = m_nSizeBoxHeight;
	m_header.Create(m_hWnd, rcDefault, NULL, 0, 0, CColumnPane::m_nHeaderID, &cp);

	DWORD dwStyle = WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE
	              | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS
				  | LVS_AUTOARRANGE | LVS_EDITLABELS | WS_EX_CLIENTEDGE;
	DWORD dwExStyle = 0;
	m_list.Create(m_hWnd, rcDefault, NULL, dwStyle, dwExStyle, (HMENU)m_nListID, NULL);
	m_list.SetExtendedListViewStyle(LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_UNDERLINEHOT | LVS_EX_LABELTIP | LVS_EX_DOUBLEBUFFER);

	InitLV();
	FillLV();

	// Fill in the header text
	{
		STRRET strDispName;
		spParentFolder->GetDisplayNameOf(m_pidl, SHGDN_NORMAL, &strDispName);
		LPTSTR szDisplayName;
		HRESULT hr = StrRetToStr(&strDispName, m_pidl, &szDisplayName);
		m_header.SetWindowText(szDisplayName);
		CoTaskMemFree(szDisplayName);
	}

	return 0;
}

void CColumnPane::OnExitSizeMove(){
	UpdateLayout();
	SendMessage(GetParent(), GetResizeMessage(), (WPARAM)m_hWnd, (LPARAM)NULL);
}
void CColumnPane::OnSize(UINT nType, CSize size){
	if(nType != SIZE_MINIMIZED){
		UpdateLayout();
		SendMessage(GetParent(), GetResizeMessage(), (WPARAM)m_hWnd, (LPARAM)NULL);
	}
	SetMsgHandled(FALSE);
}
void CColumnPane::UpdateLayout(BOOL UpdateBars){
	RECT rect = { 0 };
	GetClientRect(&rect);

	// position bars and offset their dimensions
	if(UpdateBars){
		UpdateBarsPosition(rect, UpdateBars);
		// rect is now only non-header area
	}else{
		rect.bottom -= m_header.GetDesiredHeight();
		// now rect is now only non-header area
	}
	RECT rcHeader; // does not change
	m_header.GetClientRect(&rcHeader);

	// resize client window
	if(m_list != NULL){
		m_list.MoveWindow(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);

		// Move rect to the area not covered by header and list
		rect.top = rect.bottom;
		rect.bottom += rcHeader.bottom;
		rect.left = rcHeader.right;
		InvalidateRect(&rect, TRUE);
	}
}
void CColumnPane::UpdateBarsPosition(RECT& rect, BOOL UpdateBars){
	RECT rcPane;
	GetClientRect(&rcPane);

	if(m_header != NULL){
		if(UpdateBars)
			::SendMessage(m_header, WM_SIZE, 0, 0);
		
		int header_height = m_header.GetDesiredHeight();

		m_header.MoveWindow(0, rcPane.bottom-rcPane.top-header_height, rcPane.right-rcPane.left-m_nSizeBoxWidth, header_height);

		rect.bottom -= header_height;
	}
}

UINT CColumnPane::OnNcHitTest(CPoint point){
	CRect rc;
	GetWindowRect(rc);
	rc.left = rc.right - m_nSizeBoxWidth;
	rc.top = rc.bottom - m_nSizeBoxHeight;
	if(rc.PtInRect(point)){
		return HTRIGHT;
	}else{
		SetMsgHandled(false);
		return HTCLIENT;
	}
}

void CColumnPane::DestroyPIDL(){
	if(NULL != m_pidl){
		ILFree(m_pidl);
	}
}

void CColumnPane::SetDirectory(LPITEMIDLIST pidl){
	DestroyPIDL();
	m_pidl = pidl;
}

int CALLBACK CColumnPane::ListViewCompareProc(LPARAM lparam1, LPARAM lparam2, LPARAM lParamSort){
	ATLASSERT(lParamSort != NULL);

	LPLVITEMDATA lplvid1 = (LPLVITEMDATA)lparam1;
	LPLVITEMDATA lplvid2 = (LPLVITEMDATA)lparam2;
	SortData* pSD = (SortData*)lParamSort;

	HRESULT hr = 0;
	if(pSD->bReverseSort)
		hr = lplvid1->spParentFolder->CompareIDs(0, lplvid2->lpi, lplvid1->lpi);
	else
		hr = lplvid1->spParentFolder->CompareIDs(0, lplvid1->lpi, lplvid2->lpi);

	return (int)(short)HRESULT_CODE(hr);
}

UINT CColumnPane::GetResizeMessage(){
	static UINT uResizeMessage = 0;
	if(uResizeMessage == 0){
		CStaticDataInitCriticalSectionLock lock;
		if(FAILED(lock.Lock())){
			ATLTRACE2(atlTraceUI, 0, _T("ERROR : Unable to lock critical section in CColumnPane::GetResizeMessage.\n"));
			ATLASSERT(FALSE);
			return 0;
		}

		if(uResizeMessage == 0){
			uResizeMessage = ::RegisterWindowMessage(_T("CVBrowser_ColumnPane_InternalGetResizeMsg"));
		}

		lock.Unlock();
	}
	ATLASSERT(uResizeMessage != 0);
	return uResizeMessage;
}

BOOL CColumnPane::OnEraseBkgnd(CDCHandle dc){
    RECT rcPane;
	GetClientRect(&rcPane);
	RECT rcHeader;
	m_header.GetClientRect(&rcHeader);
	rcPane.left = rcHeader.left;
	rcPane.top = rcHeader.top;
	dc.FillRect(&rcPane, COLOR_3DFACE);
	return TRUE;
}
void CColumnPane::OnPaint(CDCHandle dcNonExistantWhyDoTheyGiveItToYou){
	CPaintDC dc(*this);
	HBRUSH  hbrSave, hbr3DFACE;
	RECT rcPane;

	GetClientRect(&rcPane);
	rcPane.left = rcPane.right - m_nSizeBoxWidth;
	rcPane.top = rcPane.bottom - m_nSizeBoxHeight;

	hbr3DFACE = GetSysColorBrush(COLOR_3DFACE);
	hbrSave = dc.SelectBrush(hbr3DFACE);
    
	dc.DrawFrameControl(&rcPane, DFC_SCROLL, DFCS_SCROLLSIZEGRIP);
	dc.SelectBrush(hbrSave);
}
LRESULT CColumnPane::OnColumnItemChanged(LPNMHDR lParam){
	LPNMLISTVIEW plv = (LPNMLISTVIEW)lParam; // only works for comctl32.dll >= v4.7, IE4 >= 4.0
	return 0;
}

LRESULT CColumnPane::OnRightClick(LPNMHDR pnmh){
	LPNMITEMACTIVATE pia = (LPNMITEMACTIVATE)pnmh;
	return 0;
}

LRESULT CColumnPane::OnDblClick(LPNMHDR pnmh){
	LPNMITEMACTIVATE pia = (LPNMITEMACTIVATE)pnmh;
	return 0;
}

LRESULT CColumnPane::OnNMRClick(int, LPNMHDR pnmh, BOOL&)
{
	POINT pt = { 0, 0 };
	::GetCursorPos(&pt);
	POINT ptClient = pt;
	if(pnmh->hwndFrom != NULL)
		::ScreenToClient(pnmh->hwndFrom, &ptClient);
	
	if(pnmh->hwndFrom == m_list.m_hWnd){
		LVHITTESTINFO lvhti = { 0 };
		lvhti.pt = ptClient;
		m_list.HitTest(&lvhti);
		if ((lvhti.flags & LVHT_ONITEMLABEL) != 0){
			LVITEM lvi = { 0 };
			lvi.mask = LVIF_PARAM;
			lvi.iItem = lvhti.iItem;
			if (m_list.GetItem(&lvi) != FALSE){
				LPLVITEMDATA lptvid = (LPLVITEMDATA)lvi.lParam;
				if (lptvid != NULL){
					// Parent of this is RowContainer, whose parent is the splitter, whose parent is the main form.
					HWND hwnd = ::GetParent(::GetParent(::GetParent(*this)));
					TCHAR buf[MAX_PATH];
					::GetWindowText(hwnd, buf, MAX_PATH);
					OnContextMenu(hwnd, spParentFolder, m_pidl, pt);
				}
			}
		}
	}

	return 0L;
}
/*
BOOL CColumnPane::DoContextMenu(HWND hWnd, LPSHELLFOLDER lpsfParent, LPITEMIDLIST lpi, POINT point){
	CComPtr<IContextMenu> spContextMenu;
	HRESULT hr = lpsfParent->GetUIObjectOf(hWnd, 1, (const struct _ITEMIDLIST**)&lpi, IID_IContextMenu, 0, (LPVOID*)&spContextMenu);
	if(FAILED(hr))
		return FALSE;

	HMENU hMenu = ::CreatePopupMenu();
	if(hMenu == NULL)
		return FALSE;

	// Get the context menu for the item.
	hr = spContextMenu->QueryContextMenu(hMenu, 0, 1, 0x7FFF, CMF_EXPLORE);
	if(FAILED(hr))
		return FALSE;

	int idCmd = ::TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, point.x, point.y, 0, hWnd, NULL);

	if (idCmd != 0){
		USES_CONVERSION;

		// Execute the command that was selected.
		CMINVOKECOMMANDINFO cmi = { 0 };
		cmi.cbSize = sizeof(CMINVOKECOMMANDINFO);
		cmi.fMask = 0;
		cmi.hwnd = hWnd;
		cmi.lpVerb = T2CA(MAKEINTRESOURCE(idCmd - 1));
		cmi.lpParameters = NULL;
		cmi.lpDirectory = NULL;
		cmi.nShow = SW_SHOWNORMAL;
		cmi.dwHotKey = 0;
		cmi.hIcon = NULL;
		hr = spContextMenu->InvokeCommand(&cmi);
	}

	::DestroyMenu(hMenu);

	return TRUE;
}*/

LRESULT CColumnPane::OnLVItemClick(int, LPNMHDR pnmh, BOOL&){
	if(pnmh->hwndFrom != m_list.m_hWnd)
		return 0L;
	POINT pt;
	::GetCursorPos((LPPOINT)&pt);
	m_list.ScreenToClient(&pt);

	LVHITTESTINFO lvhti;
	lvhti.pt = pt;
	m_list.HitTest(&lvhti);
	LVITEM lvi;

	if (lvhti.flags & LVHT_ONITEM){
		m_list.ClientToScreen(&pt);
		lvi.mask = LVIF_PARAM;
		lvi.iItem = lvhti.iItem;
		lvi.iSubItem = 0;

		if (!m_list.GetItem(&lvi))
			return 0;

		LPLVITEMDATA lplvid = (LPLVITEMDATA)lvi.lParam;

		if(lplvid == NULL){
			return 0L;
		}
		if(!(lplvid->ulAttribs & SFGAO_FOLDER)){ // clicked on non-folder
		}else{ // clicked on folder
			NMLISTVIEWSELECTPIDL nmhdr;
			nmhdr.parent_folder = lplvid->spParentFolder;
			nmhdr.selected_pidl = lplvid->lpi;
			nmhdr.hdr.code = m_nListViewSelectPIDLNotification;
			nmhdr.hdr.idFrom = GetDlgCtrlID();
			nmhdr.hdr.hwndFrom = m_hWnd;
			GetParent().SendMessage(WM_NOTIFY, (WPARAM)nmhdr.hdr.idFrom, (LPARAM)&nmhdr);
		}
	}

	return 0L;
}

LRESULT CColumnPane::OnListNotify(LPNMLISTVIEW pnmlv){
	switch(pnmlv->hdr.code){
	case LVN_GETDISPINFO:
		{
			//
		}
	}
	return 0;
}


LRESULT CColumnPane::OnLVGetDispInfo(int, LPNMHDR pnmh, BOOL&){
	NMLVDISPINFO* plvdi = (NMLVDISPINFO*)pnmh;
	if(plvdi == NULL)
		return 0L;
	LPLVITEMDATA lplvid = (LPLVITEMDATA)plvdi->item.lParam;
	
	LPITEMIDLIST pidlTemp = CPidlMgr::Copy(lplvid->lpi);

	{
		SHFILEINFO sfi = { 0 };
		DWORD_PTR dwRet = ::SHGetFileInfo((LPCTSTR)pidlTemp, 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
		plvdi->item.iImage = (dwRet != 0) ? sfi.iIcon : -1;
	}

	static const int subitem_map[5] = {
		0,  // col 0 of LV is name
		1,  // col 1 of LV is size
		3,  // col 2 of LV is date modified (which is index 3 of GetDetailsOf)
		-1,
		-1
	};

	if (plvdi->item.iSubItem == 0 && (plvdi->item.mask & LVIF_TEXT) ){   // File Name
		STRRET str = { STRRET_CSTR };
		if (lplvid->spParentFolder->GetDisplayNameOf(lplvid->lpi, SHGDN_NORMAL, &str) == NOERROR){
			USES_CONVERSION;

			switch (str.uType){
			case STRRET_WSTR:
				lstrcpy(plvdi->item.pszText, W2CT(str.pOleStr));
				::CoTaskMemFree(str.pOleStr);
				break;
			case STRRET_OFFSET:
				lstrcpy(plvdi->item.pszText, (LPTSTR)lplvid->lpi + str.uOffset);
				break;
			case STRRET_CSTR:
				lstrcpy(plvdi->item.pszText, A2CT(str.cStr));
				break;
			default:
				break;
			}
		}
	}else{
		CComPtr<IShellFolder2> spFolder2;
		HRESULT hr = spParentFolder->QueryInterface(IID_IShellFolder2, (void**)&spFolder2);
		if(FAILED(hr)){ return hr; }

		SHELLDETAILS sd = { 0 };
		sd.fmt = LVCFMT_CENTER;
		sd.cxChar = 15;
		
		// The subitem number determines which column we are interested in
		hr = spFolder2->GetDetailsOf(lplvid->lpi, subitem_map[plvdi->item.iSubItem], &sd);
		if(FAILED(hr)){ return hr; }

		if(sd.str.uType == STRRET_WSTR){
			StrRetToBuf(&sd.str, m_pidl, m_szListViewBuffer, MAX_PATH);
			plvdi->item.pszText = m_szListViewBuffer;
		}else if(sd.str.uType == STRRET_OFFSET){
			LPCITEMIDLIST parent_pidl;
			hr = SHBindToParent(m_pidl, IID_IShellFolder, (void**)&spParentFolder, &parent_pidl);
			plvdi->item.pszText = (LPTSTR)parent_pidl + sd.str.uOffset;
		}else if(sd.str.uType == STRRET_CSTR){
			USES_CONVERSION;
			plvdi->item.pszText = A2T(sd.str.cStr);
		}
	}
	
	plvdi->item.mask |= LVIF_DI_SETITEM;

	CPidlMgr::Delete(pidlTemp);
	return 0L;
}

LRESULT CColumnPane::OnLVDeleteItem(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
	LPNMLISTVIEW pnmlv = (LPNMLISTVIEW)pnmh;

	LVITEM lvi;
	lvi.mask = LVIF_PARAM;
	lvi.iItem = pnmlv->iItem;
	lvi.iSubItem = 0;

	if (!m_list.GetItem(&lvi))
		return 0;

	LPLVITEMDATA lplvid = (LPLVITEMDATA)lvi.lParam;
	delete lplvid;

	return 0;
}
