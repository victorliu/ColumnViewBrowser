#pragma once

#include <list>

#include "ColumnPane.h"

typedef CWinTraitsOR< WS_HSCROLL , WS_EX_CLIENTEDGE | WS_EX_ACCEPTFILES > CRowContainerTraits;

class CRowContainer : public CWindowImpl<CRowContainer, CWindow, CRowContainerTraits>
{
protected:
	typedef std::list<CColumnPane> PaneList_t;
	PaneList_t m_panes;
	int m_nTotalPanesWidth;
	int m_nFirstPaneOffset;
	static const int m_nPaneDefaultWidth = 200;
	static const int m_nScrollPixels = 10;
public:

	enum{
		m_nPaneBaseID = 100 // must be last element!
	};
	typedef struct tagCreateParams{
		LPITEMIDLIST root_pidl;
	} CreateParams, *LPCreateParams;

	BEGIN_MSG_MAP_EX(CRowContainer)
		MSG_WM_USER_PANE_RESIZE(OnPaneResize)
		MSG_WM_HSCROLL(OnHScroll)
		//MSG_WM_MOUSEWHEEL(OnMouseWheel)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_SIZE(OnSize)
		MSG_WM_CREATE(OnCreate)
		NOTIFY_CODE_HANDLER(CColumnPane::m_nListViewSelectPIDLNotification, OnPaneItemSelected)
		NOTIFY_CODE_HANDLER(CColumnPane::m_nListViewDeselectPIDLNotification, OnPaneItemDeselected)
	END_MSG_MAP()

	//static UINT GetGetRowRectMsg();
	//LRESULT OnGetGetRowRect(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);


	void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar pScrollBar);
	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnDestroy();
	void OnSize(UINT nType, CSize size);
	LRESULT OnEraseBkgnd(CDCHandle dc);

	//BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	int OnPaneResize(HWND hPane);
	void GetRowRect(RECT *rc);
	void UpdateColumnLayout(RECT *rc, UINT startcol = 0); // rc is the available client area to use
	void RecomputeTotalPanesWidth();
	void UpdateScrollBar(BOOL bScrollToEnd = FALSE);
	void ScrollTo(LONG pos);
	void ScrollDelta(LONG dpos);

	// The Pane configuration can only change in the following ways:
	// 1) Append a pane to the list (from selecting a directory in the pane at the end of the list if none was previously selected)
	// 2) All panes after a given pane are removed and a new one appended
	// 3) Remove all panes and set a new first pane
	// We can say that 2 is an extension of 1 where we remove the panes first
	// We will therefore handle things thusly:
	// On item deselection, drop all panes after the pane in which the item was deselected
	// On item selection, make sure all panes after the pane in which the selection are removed, and add the new one
	void RemovePanesAfter(HWND hPane);
	void AppendPane(CComPtr<IShellFolder> parent, LPITEMIDLIST pidl); // pidl follows move semantics, should be a full path PIDL
	void NewRootPane(LPITEMIDLIST pidl); // pidl follows move semantics

	LRESULT OnPaneItemSelected(int id, LPNMHDR lParam, BOOL &bHandled);
	LRESULT OnPaneItemDeselected(int id, LPNMHDR lParam, BOOL &bHandled);
private:
	void RemoveAllPanes();
};
