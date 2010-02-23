#pragma once

#include "ColumnList.h"
#include "ColumnHeader.h"

// int OnPaneResize(HWND hPane)
#define MSG_WM_USER_PANE_RESIZE(func) \
	if (uMsg == CColumnPane::GetResizeMessage()) \
	{ \
		SetMsgHandled(TRUE); \
		lResult = (LRESULT)func((HWND)wParam); \
		if(IsMsgHandled()) \
			return TRUE; \
	}


typedef CWinTraitsOR< WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0 > CColumnPaneTraits;

class CColumnPane : public CWindowImpl<CColumnPane, CWindow, CColumnPaneTraits>
{
	struct SortData
	{
		SortData(int nSortNum, bool bReverse) : nSort(nSortNum), bReverseSort(bReverse)
		{ }

		int nSort;
		bool bReverseSort;
	};
protected:
	CListViewCtrl m_list;
	CColumnHeader m_header;

	// This is owned by instances of this class
	CComPtr<IShellFolder> spParentFolder;
	LPITEMIDLIST m_pidl; // fully qualified!

	int m_nSizeBoxWidth, m_nSizeBoxHeight;

	IContextMenu2 *g_IContext2;
	IContextMenu3 *g_IContext3;

	IShellFolder *pShellFolder;
	IShellFolder2 *pShellFolder2;
    IShellDetails *pShellDetails;

	IDropTarget *pIDropTarget;     // these two are for drag and drop
	IDataObject *pIDataObjectDrop; //

	TCHAR m_szListViewBuffer[MAX_PATH]; // Buffer for OnLVGetDispInfo

	
	typedef struct _LVItemData
	{
		_LVItemData() : ulAttribs(0)
		{ }
		
		CComPtr<IShellFolder> spParentFolder;
		LPITEMIDLIST lpi;
		ULONG ulAttribs;

	} LVITEMDATA, *LPLVITEMDATA;
	HWND GetMainWindowHWND();
public:
	CColumnPane();
	~CColumnPane();
	DECLARE_WND_CLASS_EX(_T("ColumnPaneContainer"), 0, -1)

	enum{
		m_nListViewSelectPIDLNotification = 1,
		m_nListViewDeselectPIDLNotification,
		m_nListViewFocusNotification,
		m_nHeaderID = 100,
		m_nListID
	};

	typedef struct tagNMLISTVIEWSELECTPIDL{
		NMHDR hdr;
		CComPtr<IShellFolder> parent_folder;
		LPITEMIDLIST selected_pidl; // follows move semantics
	} NMLISTVIEWSELECTPIDL, *LPNMLISTVIEWSELECTPIDL;

	typedef struct tagCreateParams{
		CComPtr<IShellFolder> parent_folder; // this pointer will be released when a ColumnPane gets destroyed
		LPITEMIDLIST rel_pidl; // this will be copied out of here, so whoever fills this in can safely keep that copy
		bool is_folder;
	} CreateParams, *LPCreateParams;

	static int CALLBACK ListViewCompareProc(LPARAM lparam1, LPARAM lparam2, LPARAM lparamSort);

	BEGIN_MSG_MAP_EX(CColumnPane)
		MSG_WM_SIZE(OnSize)
		MSG_WM_EXITSIZEMOVE(OnExitSizeMove)
		//MSG_WM_SIZING(OnSizing)
		//MSG_WM_WINDOWPOSCHANGED(OnWindowPosChanged)
		//MSG_WM_WINDOWPOSCHANGING(OnWindowPosChanging)

		MSG_WM_CREATE(OnCreate)
		MSG_WM_NCHITTEST(OnNcHitTest)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_NCDESTROY(OnNcDestroy)
		MSG_WM_PAINT(OnPaint)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)

		// We use callbacks to get text and icons for the listview
		NOTIFY_CODE_HANDLER(LVN_GETDISPINFO, OnLVGetDispInfo)
		// Mouse capture:
		NOTIFY_CODE_HANDLER(NM_RCLICK, OnNMRClick)
		NOTIFY_CODE_HANDLER(NM_CLICK, OnLVItemClick)
		NOTIFY_CODE_HANDLER(NM_DBLCLK, OnLVItemClick)
		// We store a heap allocated object in each LV item's lParam, and
		// we need to go and delete them.
		NOTIFY_CODE_HANDLER(LVN_DELETEITEM, OnLVDeleteItem)
		NOTIFY_CODE_HANDLER(LVN_ENDLABELEDIT, OnLVEndLabelEdit)
		
		NOTIFY_CODE_HANDLER_EX(LVN_ITEMCHANGED, OnColumnItemChanged)
		NOTIFY_HANDLER_EX(m_nListID, NM_RCLICK, OnRightClick)
		NOTIFY_HANDLER_EX(m_nListID, NM_DBLCLK, OnDblClick)
		NOTIFY_HANDLER_EX(m_nListID, NM_SETFOCUS, OnSetFocus)
		NOTIFY_HANDLER_EX(m_nListID, NM_KILLFOCUS, OnKillFocus)
		//MSG_WM_GETMINMAXINFO(OnGetMinMaxInfo)
		/*
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		
		MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
		MESSAGE_HANDLER(WM_COMMAND, OnCommand)
		*/
		FORWARD_NOTIFICATIONS()
	END_MSG_MAP()

	// Retrieve the message for telling parent RowContainer about a pane resize event
	// wParam is the HWND of the resized pane
	// lParam is unused
	// Return value ignored (return 0)
	static UINT GetResizeMessage();

	void InitLV();
	LRESULT FillLV();

	LRESULT OnNotify(int idCtrl, LPNMHDR pnmh);
	LRESULT OnListNotify(LPNMLISTVIEW pnmlv);

	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnSize(UINT nType, CSize size);
	void OnExitSizeMove();
	//void OnSizing(UINT fwSide, LPRECT pRect);
	//void OnWindowPosChanged(LPWINDOWPOS lpWndPos);
	//void OnWindowPosChanging(LPWINDOWPOS lpWndPos);

	LRESULT OnSetFocus(LPNMHDR pnmh);
	LRESULT OnKillFocus(LPNMHDR pnmh);
	void OnPaint(CDCHandle dc);
	void OnDestroy();
	void OnNcDestroy();
	BOOL OnEraseBkgnd(CDCHandle dc);
	UINT OnNcHitTest(CPoint point);
	void UpdateLayout(BOOL UpdateBars = TRUE);
	void UpdateBarsPosition(RECT& rect, BOOL UpdateBars = TRUE);

	LRESULT OnLVGetDispInfo(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	LRESULT OnLVItemClick(int , LPNMHDR pnmh, BOOL& );
	LRESULT OnLVDeleteItem(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	LRESULT OnLVEndLabelEdit(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	
	LRESULT OnColumnItemChanged(LPNMHDR lParam);
	LRESULT OnNMRClick(int , LPNMHDR pnmh, BOOL& );
	LRESULT OnRightClick(LPNMHDR pnmh);
	LRESULT OnDblClick(LPNMHDR pnmh);

	BOOL DoContextMenu(HWND hWnd, LPSHELLFOLDER lpsfParent, LPITEMIDLIST lpi, POINT point);
	

	void SetDirectory(LPITEMIDLIST pidl);
	void DestroyPIDL();


	// API for allowed operations:
	void RenameSelectedItem(); // doesn't do the renaming, just simulates a second click on the item
	void CreateNewFile();
	void CreateNewDirectory();
	void DeleteSelectedItems();
	void CutSelectedItemsToClipBoard();
	void CopySelectedItemsToClipBoard();
	void PasteFromClipboard();
	void EditSelectedItems();
	void ViewSelectedItems();
	void SelectAll();
	void UnselectAll();
	void GetFullPath(LPTSTR lpPath, DWORD dwPathLen);
};
