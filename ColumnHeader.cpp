#include "StdAfx.h"
#include "ColumnHeader.h"

LRESULT CColumnHeader::OnCreate(LPCREATESTRUCT cs){
	LPCREATEPARAMS cp = (LPCREATEPARAMS)cs->lpCreateParams;
	m_nDesiredHeight = cp->nMinHeight;
	
	CDCHandle hDC = GetDC();
	hDC.SelectFont(AtlGetDefaultGuiFont());
	TEXTMETRIC tm;
	GetTextMetrics(hDC, &tm);
	int fontheight = tm.tmHeight + tm.tmInternalLeading;
	int cyborder = GetSystemMetrics(SM_CYBORDER);
	if(0 == tm.tmInternalLeading){ fontheight += cyborder * 2; }

	int newheight = fontheight + 2 * cyborder ;

	newheight += 2*m_nPadding;

	if(newheight > m_nDesiredHeight){ m_nDesiredHeight = newheight; }
	ReleaseDC(hDC);

	return 0;
}
int CColumnHeader::OnSetText(LPCTSTR lpstrText){
	SetMsgHandled(FALSE);
	Invalidate();
	return TRUE;
}
void CColumnHeader::OnPaint(CDCHandle dc){
	if(dc != NULL){
		DrawHeader(dc);
	}else{
		CPaintDC dc(m_hWnd);
		DrawHeader(dc.m_hDC);
	}
}
void CColumnHeader::DrawHeader(CDCHandle dc){
	if(NULL == dc){ return; }

	RECT rc;
	GetClientRect(&rc);

	dc.DrawEdge(&rc, EDGE_ETCHED, BF_LEFT | BF_TOP | BF_ADJUST | BF_FLAT);
	dc.FillRect(&rc, COLOR_3DFACE);

	int len = 0;
	if(len = GetWindowTextLength()){
		LPTSTR buf = new TCHAR[len+1];
		GetWindowText(buf, len+1);
		

		// Get the right font
		NONCLIENTMETRICS ncm;
		ncm.cbSize = sizeof(NONCLIENTMETRICS);
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);
		const HFONT sysfont = CreateFontIndirect(&(ncm.lfStatusFont));
		HFONT prevfont = dc.SelectFont(sysfont);


		dc.SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
		dc.SetBkMode(TRANSPARENT);
		rc.left += m_nPadding;
		rc.right -= m_nPadding+rc.bottom;
		dc.DrawTextExW(buf, len, &rc, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
		
		// Clean up
		delete [] buf;
		dc.SelectFont(prevfont);
		DeleteObject(sysfont); // Not necessary (not harmful either)
	}
}

