#include "StdAfx.h"
#include "ColumnPane.h"
#include "RowContainer.h"
#include "CPidlMgr.h"
#include <shlwapi.h>

LRESULT CColumnPane::FillLV(){
	// Populate list
	LPSHELLFOLDER pIDesktop;
	if(S_OK != SHGetDesktopFolder(&pIDesktop)){
		return FALSE;
	}
	LPSHELLFOLDER pShellFolder;
	if(S_OK != pIDesktop->BindToObject(m_pidl, NULL, IID_IShellFolder, (void**)&pShellFolder)){
		pIDesktop->Release();
		return FALSE;
	}

	CComPtr<IEnumIDList> spEnumIDList;
	HRESULT hr = pShellFolder->EnumObjects(m_list.GetParent(), SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &spEnumIDList);
	if (FAILED(hr))
		return FALSE;

	CShellItemIDList lpifqThisItem;
	CShellItemIDList lpi;
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

		lpifqThisItem = m_ShellMgr.ConcatPidls(m_pidl, lpi);

		lvi.iItem = iCtr;
		lvi.iSubItem = 0;
		lvi.pszText = LPSTR_TEXTCALLBACK;
		lvi.cchTextMax = MAX_PATH;
		uFlags = SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON;
		lvi.iImage = I_IMAGECALLBACK;

		lplvid->spParentFolder.p = pShellFolder;
		pShellFolder->AddRef();

		// Now make a copy of the ITEMIDLIST
		lplvid->lpi= m_ShellMgr.CopyITEMID(lpi);

		lvi.lParam = (LPARAM)lplvid;

		// Add the item to the list view control
		int n = m_list.InsertItem(&lvi);
		m_list.AddItem(n, 1, LPSTR_TEXTCALLBACK, I_IMAGECALLBACK);
		m_list.AddItem(n, 2, LPSTR_TEXTCALLBACK, I_IMAGECALLBACK);
		m_list.AddItem(n, 3, LPSTR_TEXTCALLBACK, I_IMAGECALLBACK);
		m_list.AddItem(n, 4, LPSTR_TEXTCALLBACK, I_IMAGECALLBACK);

		iCtr++;
		lpifqThisItem = NULL;
		lpi = NULL;   // free PIDL the shell gave you
	}

	//SortData sd(m_nSort, m_bReverseSort);
	SortData sd(0, false);
	m_list.SortItems(CColumnPane::ListViewCompareProc, (LPARAM)&sd);
	return TRUE;
}
void CColumnPane::InitLV(){
	// Get Desktop folder
	CShellItemIDList spidl;
	HRESULT hRet = ::SHGetSpecialFolderLocation(m_hWnd, CSIDL_DESKTOP, &spidl);
	hRet;	// avoid level 4 warning
	ATLASSERT(SUCCEEDED(hRet));

	// Get system image lists
	SHFILEINFO sfi = { 0 };
	HIMAGELIST hImageList = (HIMAGELIST)::SHGetFileInfo(spidl, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_ICON);
	ATLASSERT(hImageList != NULL);

	memset(&sfi, 0, sizeof(SHFILEINFO));
	HIMAGELIST hImageListSmall = (HIMAGELIST)::SHGetFileInfo(spidl, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
	ATLASSERT(hImageListSmall != NULL);

	// Create list view columns
	m_list.InsertColumn(0, _T("Name"), LVCFMT_LEFT, 100, 0);
	m_list.InsertColumn(1, _T("Size"), LVCFMT_RIGHT, 30, 1);
	m_list.InsertColumn(2, _T("Type"), LVCFMT_LEFT, 20, 2);
	m_list.InsertColumn(3, _T("Modified"), LVCFMT_LEFT, 30, 3);
	m_list.InsertColumn(4, _T("Attributes"), LVCFMT_RIGHT, 20, 4);

	// Set list view image lists
	m_list.SetImageList(hImageList, LVSIL_NORMAL);
	m_list.SetImageList(hImageListSmall, LVSIL_SMALL);
}


int CALLBACK CColumnPane::ListViewCompareProc(LPARAM lparam1, LPARAM lparam2, LPARAM lParamSort)
{
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
		m_pidl = pcp->new_pidl;
	}else{
		m_pidl = NULL;
	}


	m_nSizeBoxWidth = GetSystemMetrics(SM_CXVSCROLL);
	m_nSizeBoxHeight = GetSystemMetrics(SM_CYHSCROLL);

	CColumnHeader::CreateParams cp;
	cp.nMinHeight = m_nSizeBoxHeight;
	m_header.Create(m_hWnd, rcDefault, NULL, 0, 0, CColumnPane::m_nHeaderID, &cp);

	DWORD dwStyle = WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE
	              | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS
				  | LVS_AUTOARRANGE | LVS_EDITLABELS | WS_EX_CLIENTEDGE; // | LVS_OWNERDATA
	DWORD dwExStyle = 0;
	m_list.Create(m_hWnd, rcDefault, NULL, dwStyle, dwExStyle, (HMENU)m_nListID, NULL);
	m_list.SetExtendedListViewStyle(LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_UNDERLINEHOT | LVS_EX_LABELTIP | LVS_EX_DOUBLEBUFFER);

	InitLV();
	FillLV();

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
	
	CShellItemIDList pidlTemp = m_ShellMgr.ConcatPidls(m_pidl, lplvid->lpi);
	plvdi->item.iImage = m_ShellMgr.GetIconIndex(pidlTemp, SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
	if (plvdi->item.iSubItem == 0 && (plvdi->item.mask & LVIF_TEXT) )   // File Name
	{
		m_ShellMgr.GetName(lplvid->spParentFolder, lplvid->lpi, SHGDN_NORMAL, plvdi->item.pszText);
	}	
	else
	{
		/*
		CComPtr<IShellFolder2> spFolder2;
		HRESULT hr = lptvid->spParentFolder->QueryInterface(IID_IShellFolder2, (void**)&spFolder2);
		if(FAILED(hr))
			return hr;

		SHELLDETAILS sd = { 0 };
		sd.fmt = LVCFMT_CENTER;
		sd.cxChar = 15;
		
		hr = spFolder2->GetDetailsOf(lplvid->lpi, plvdi->item.iSubItem, &sd);
		if(FAILED(hr))
			return hr;

		if(sd.str.uType == STRRET_WSTR)
		{
			StrRetToBuf(&sd.str, lplvid->lpi.m_pidl, m_szListViewBuffer, MAX_PATH);
			plvdi->item.pszText=m_szListViewBuffer;
		}
		else if(sd.str.uType == STRRET_OFFSET)
		{
			plvdi->item.pszText = (LPTSTR)lptvid->lpi + sd.str.uOffset;
		}
		else if(sd.str.uType == STRRET_CSTR)
		{
			USES_CONVERSION;
			plvdi->item.pszText = A2T(sd.str.cStr);
		}*/
	}
	
	plvdi->item.mask |= LVIF_DI_SETITEM;

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
