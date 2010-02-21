#pragma once
#include "atlwin.h"
#include <shlobj.h>
#include <map>

typedef CWinTraitsOR< LVS_SHOWSELALWAYS | LVS_NOCOLUMNHEADER | LVS_REPORT | LVS_SINGLESEL | LVS_NOSORTHEADER, WS_EX_CLIENTEDGE> CListTraits;

class CCVRootsList :
	public CWindowImpl<CCVRootsList, CListViewCtrl, CListTraits>
{
protected:
	typedef std::map<int, LPITEMIDLIST> pidl_map_t;
	pidl_map_t m_pidls;
	CImageList icons;
	BOOL static_favs; // on pre-Vista, favorites (places) are not stored by OS
	int m_nNextMapKey;
	int last_selected_key;
public:
/*
	enum{
		RLN_ITEM_ACTIVATED = 1
	};
	typedef struct tagNMCVRootsList{
		NMHDR hdr;
		LPITEMIDLIST pCurPIDL;
	} NMCVRootsList, *LPNMCVRootsList;
*/
	CCVRootsList():m_nNextMapKey(0),last_selected_key(-1){}

	DECLARE_WND_SUPERCLASS(NULL, WC_LISTVIEW)

	BEGIN_MSG_MAP(CCVRootsList)
		MSG_WM_DESTROY(OnDestroy)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	void Init();
	void OnDestroy();
	LPITEMIDLIST GetSelectedPIDL(int which = -1) /*const*/; // TODO: make const
};
