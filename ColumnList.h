#pragma once

#include <map>
class CColumnHeader;

typedef CWinTraitsOR< LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS | LVS_AUTOARRANGE | LVS_EDITLABELS /*| LVS_OWNERDATA*/, 0> CColumnTraits;


// Sort by StrCmpLogicalW
class CColumnList :
	public CWindowImpl<CColumnList, CListViewCtrl, CColumnTraits>
{
public:
	DECLARE_WND_SUPERCLASS(NULL, WC_LISTVIEW)

	BEGIN_MSG_MAP_EX(CColumnList)/*
		MSG_WM_NCCREATE(OnNcCreate)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_NCDESTROY(OnNcDestroy)*/
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()
/*
	BOOL OnNcCreate(LPCREATESTRUCT lpCreateStruct);
	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnNcDestroy();

	void Init(LPCITEMIDLIST new_pidl, CColumnHeader &header_to_init);
	LPITEMIDLIST GetSelectedPIDL(int which_item);*/
};
