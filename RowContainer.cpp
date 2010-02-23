#include "StdAfx.h"
#include "RowContainer.h"
#include "CPidlMgr.h"
/*
BOOL CRowContainer::PreTranslateMessage(MSG* pMsg){
	if(WM_MOUSEWHEEL == pMsg->message){
		POINT p;
		p.x = GET_X_LPARAM(pMsg->lParam);
		p.y = GET_Y_LPARAM(pMsg->lParam);
		HWND hWndPt = WindowFromPoint(p);
		if(NULL != hWndPt){
			::PostMessage(hWndPt, WM_MOUSEWHEEL, pMsg->wParam, pMsg->lParam);
			return TRUE;
		}
	}
	return FALSE;
}*/

int CRowContainer::OnCreate(LPCREATESTRUCT lpCreateStruct){
	CRowContainer::LPCreateParams pcp = (CRowContainer::LPCreateParams)lpCreateStruct->lpCreateParams;

	m_nFirstPaneOffset = 0;
	RecomputeTotalPanesWidth();

	SetMsgHandled(FALSE);
	return 0;
}

void CRowContainer::OnDestroy(){
	RemoveAllPanes();
}
void CRowContainer::RemoveAllPanes(){
	PaneList_t::iterator pane_iter = m_panes.begin();
	while(pane_iter != m_panes.end()){
		PaneList_t::iterator current = pane_iter++;
		current->DestroyWindow();
		current->m_hWnd = NULL;
		m_panes.erase(current);
	}
//	m_panes.clear();
}

void CRowContainer::OnSize(UINT nType, CSize size){
	RECT rc;
	GetClientRect(&rc);
	UpdateColumnLayout(&rc, 0);
	UpdateScrollBar();
}

// TODO: optimize this to only erase visible part of bkgnd
LRESULT CRowContainer::OnEraseBkgnd(CDCHandle dc){
	RECT rc;
	GetClientRect(&rc);
	dc.FillRect(&rc, COLOR_3DFACE);
	return TRUE;
}

int CRowContainer::OnPaneResize(HWND hPane){
	RECT rc;
	GetRowRect(&rc);

	BOOL bFound = FALSE;
	int nWidthSoFar = 0;
	PaneList_t::iterator pane_iter;
	for(pane_iter = m_panes.begin(); pane_iter != m_panes.end(); ++pane_iter){
		RECT crc;
		pane_iter->GetClientRect(&crc);
		if(bFound || (hPane == pane_iter->m_hWnd)){
			pane_iter->MoveWindow(m_nFirstPaneOffset+nWidthSoFar, rc.top, crc.right-crc.left, rc.bottom-rc.top);
			bFound = TRUE;
		}
		nWidthSoFar += crc.right-crc.left;
	}
	m_nTotalPanesWidth = nWidthSoFar;

	int nOffsetOld = m_nFirstPaneOffset;

	int rcWidth = rc.right - rc.left;
	BOOL bFitsInContainer = (m_nTotalPanesWidth < rcWidth);
	BOOL bRightSidePinned = (m_nFirstPaneOffset + m_nTotalPanesWidth <= rcWidth);
	if(bFitsInContainer){ // if all panes fit in the RowContainer, just pin the root pane at the left edge
		if(m_nFirstPaneOffset != 0){
			m_nFirstPaneOffset = 0;
			UpdateColumnLayout(&rc, 0);
		}
	}else if(bRightSidePinned){
		m_nFirstPaneOffset = rcWidth - m_nTotalPanesWidth;
		UpdateColumnLayout(&rc, 0);
	}

	UpdateScrollBar();
	return 0;
}
void CRowContainer::GetRowRect(RECT *rc){
	GetClientRect(rc);
}

//LRESULT CRowContainer::OnGetGetRowRect(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/){
//	LPRECT prc = (LPRECT)wParam;
//	if(NULL != prc){
//		GetRowRect(prc);
//	}
//	return 0;
//}

void CRowContainer::UpdateColumnLayout(RECT *rc, UINT startcol){
	if(startcol >= m_panes.size()){ return; }
	// Get to the startcol
	int nWidthSoFar = 0;
	PaneList_t::iterator pane_iter = m_panes.begin();
	for(UINT col = 0; col < startcol; ++col){
		RECT crc;
		pane_iter->GetClientRect(&crc);
		nWidthSoFar += crc.right-crc.left;

		++pane_iter;
	}
	while(pane_iter != m_panes.end()){
		RECT crc;
		pane_iter->GetClientRect(&crc);

		pane_iter->MoveWindow(m_nFirstPaneOffset+nWidthSoFar, rc->top, crc.right-crc.left, rc->bottom-rc->top);
		nWidthSoFar += crc.right-crc.left;

		++pane_iter;
	}
	m_nTotalPanesWidth = nWidthSoFar;
}
void CRowContainer::RecomputeTotalPanesWidth(){
	m_nTotalPanesWidth = 0;
	PaneList_t::iterator pane_iter = m_panes.begin();
	while(pane_iter != m_panes.end()){
		RECT rc;
		pane_iter->GetClientRect(&rc);
		m_nTotalPanesWidth += rc.right-rc.left;
		++pane_iter;
	}
}

// Assumes m_nTotalPanesWidth and m_nFirstPaneOffset are correct
void CRowContainer::UpdateScrollBar(BOOL bScrollToEnd){
	CRect rc;
	GetClientRect(&rc);
	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.nMin = 0;
	si.nPage = rc.right;
	si.nMax = m_nTotalPanesWidth;
	if(bScrollToEnd){
		si.nPos = si.nMax;
	}else{
		si.nPos = -m_nFirstPaneOffset;
	}

	si.fMask = SIF_DISABLENOSCROLL | SIF_PAGE | SIF_POS | SIF_RANGE;
	SetScrollInfo(SB_HORZ, &si, TRUE);
}

void CRowContainer::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar pScrollBar){
	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE;
	GetScrollInfo(SB_HORZ, &si);
	int page = si.nPage;
	switch(nSBCode){
	case SB_LINELEFT:  ScrollDelta(-CRowContainer::m_nScrollPixels); break;
	case SB_LINERIGHT: ScrollDelta(+CRowContainer::m_nScrollPixels); break;
	case SB_PAGELEFT:  ScrollDelta(-page); break;
	case SB_PAGERIGHT: ScrollDelta(+page); break;
	case SB_THUMBPOSITION: ScrollTo(nPos); break;
	case SB_THUMBTRACK:    ScrollTo(nPos); break;
	case SB_LEFT:          ScrollTo(0); break;
	case SB_RIGHT:         ScrollTo(MAXLONG); break;
	}
}
void CRowContainer::ScrollTo(LONG pos){
	RECT rc;
	GetRowRect(&rc);

	pos = max(pos, 0);
	pos = min(pos, m_nTotalPanesWidth - rc.right);
	m_nFirstPaneOffset = -pos;
	
	UpdateColumnLayout(&rc, 0);

	SCROLLINFO si;
    si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_POS | SIF_TRACKPOS;
	si.nTrackPos = si.nPos = -m_nFirstPaneOffset;
    SetScrollInfo(SB_HORZ, &si, TRUE);
}
void CRowContainer::ScrollDelta(LONG dpos){
	ScrollTo(-m_nFirstPaneOffset + dpos);
}
void CRowContainer::RemovePanesAfter(HWND hPane){
	BOOL bFound = FALSE;
	PaneList_t::iterator pane_iter;
	for(pane_iter = m_panes.begin(); pane_iter != m_panes.end(); ++pane_iter){
		if((*pane_iter) == hPane){
			++pane_iter;
			break;
		}
	}
	while(pane_iter != m_panes.end()){
		PaneList_t::iterator current = pane_iter++;
		current->DestroyWindow();
		current->m_hWnd = NULL;
		m_panes.erase(current);
	}
}
void CRowContainer::AppendPane(CComPtr<IShellFolder> parent, LPITEMIDLIST pidl){
	RECT rc, crc;
	GetClientRect(&crc);
	CopyRect(&rc, &crc);

	// Must set the width of the rect passed to a CColumnPane::Create
	rc.left = m_nFirstPaneOffset + m_nTotalPanesWidth;
	rc.right = rc.left + CRowContainer::m_nPaneDefaultWidth;
	int dx = rc.right - crc.right; // after this, dx is amount protruding past right edge of container

	// Automatically set the position of the new pane visible (right adjusted if possible)
	m_nTotalPanesWidth += CRowContainer::m_nPaneDefaultWidth;
	int width = crc.right - crc.left;
	if(m_nTotalPanesWidth > width){
		m_nFirstPaneOffset = width - m_nTotalPanesWidth;
	}else{
		m_nTotalPanesWidth = 0;
	}

	m_panes.push_back(CColumnPane());
	CColumnPane::CreateParams cp;
	cp.parent_folder = parent;
	cp.rel_pidl = pidl;
	cp.is_folder = true;
	m_panes.back().Create(*this, &rc, NULL, 0, 0, (HMENU)(m_nPaneBaseID+m_panes.size()), &cp);
}
void CRowContainer::NewRootPane(LPITEMIDLIST pidl){
	RemoveAllPanes();
	{
		LPSHELLFOLDER pIDesktop;
		if(S_OK == SHGetDesktopFolder(&pIDesktop)){
			AppendPane(CComPtr<IShellFolder>(pIDesktop), pidl);
		}
	}
	UpdateScrollBar(TRUE);
}

LRESULT CRowContainer::OnPaneItemSelected(int id, LPNMHDR lParam, BOOL &bHandled){
	CColumnPane::LPNMLISTVIEWSELECTPIDL plvsp = (CColumnPane::LPNMLISTVIEWSELECTPIDL)lParam;
	HWND hFrom = plvsp->hdr.hwndFrom;
	RemovePanesAfter(hFrom);
	AppendPane(plvsp->parent_folder, plvsp->selected_pidl);

	UpdateScrollBar(TRUE);
	return 0;
}
LRESULT CRowContainer::OnPaneItemDeselected(int id, LPNMHDR lParam, BOOL &bHandled){
	CColumnPane::LPNMLISTVIEWSELECTPIDL plvsp = (CColumnPane::LPNMLISTVIEWSELECTPIDL)lParam;
	HWND hFrom = plvsp->hdr.hwndFrom;
	RemovePanesAfter(hFrom);
	
	UpdateScrollBar(TRUE);
	return 0;
}
LRESULT CRowContainer::OnPaneFocused(int id, LPNMHDR lParam, BOOL &bHandled){
	m_hwndActivePane = lParam->hwndFrom;
	return 0;
}
/*
BOOL CRowContainer::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt){
	UINT uScroll = 0;
	if(!SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &uScroll, 0)) {
		uScroll = 3;
	}else{
	}
	if(uScroll == WHEEL_PAGESCROLL){
		uScroll = CRowContainer::m_nPaneDefaultWidth; // not great, but works for now
	}
	if(0 == uScroll){
		return FALSE;
	}
	int dAmountToScroll = zDelta * (int)uScroll / (WHEEL_DELTA * CRowContainer::m_nScrollPixels);
	ScrollDelta(-dAmountToScroll);
	return 0;
}
*/

int CRowContainer::GetActivePaneIndex(){
	int j = 0;
	for(PaneList_t::const_iterator i = m_panes.begin(); i != m_panes.end(); ++i){
		if(HWND(*i) == m_hwndActivePane){
			return j;
		}
		++j;
	}
	return -1;
}
void CRowContainer::SetActivePaneIndex(int index){
	int j = 0;
	for(PaneList_t::iterator i = m_panes.begin(); i != m_panes.end(); ++i){
		if(j == index){
			i->SetFocus();
			break;
		}
	}
}
CColumnPane* CRowContainer::GetActivePane(){
	for(PaneList_t::iterator i = m_panes.begin(); i != m_panes.end(); ++i){
		if(HWND(*i) == m_hwndActivePane){
			return &(*i);
		}
	}
	return NULL;
}
CColumnPane* CRowContainer::GetPane(int index){
	int j = 0;
	for(PaneList_t::iterator i = m_panes.begin(); i != m_panes.end(); ++i){
		if(j == index){
			return &(*i);
		}
		++j;
	}
	return NULL;
}
