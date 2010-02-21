#include "StdAfx.h"
#include "ColumnList.h"
#include "ColumnHeader.h"
#include "CPidlMgr.h"
/*
BOOL CColumnList::OnNcCreate(LPCREATESTRUCT lpCreateStruct){
	m_nIconSize = GetSystemMetrics(SM_CXSMICON | SM_CYSMICON);
	icons.Create(m_nIconSize, m_nIconSize, ILC_COLOR32, 64, 64);

	SetMsgHandled(FALSE);
	return FALSE;
}
void CColumnList::OnNcDestroy(){
	pidl_map_t::iterator i;
	for(i = m_pidls.begin(); i != m_pidls.end(); ++i){
		CPidlMgr::Delete(i->second);
	}
	m_pidls.clear();

	icons.Destroy();
}
int CColumnList::OnCreate(LPCREATESTRUCT lpCreateStruct){
	SetMsgHandled(FALSE);
	return 0;
}

void CColumnList::Init(LPCITEMIDLIST new_pidl, CColumnHeader &header_to_init){
	SetImageList(icons, LVSIL_SMALL);
	SetExtendedListViewStyle(LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_UNDERLINEHOT | LVS_EX_LABELTIP | LVS_EX_DOUBLEBUFFER); // Place here, not in ColumnTraits, in order to remove flickering

	InsertColumn(0, _T("Filename"), LVCFMT_LEFT, 0, 0);
	InsertColumn(1, _T("Size"), LVCFMT_LEFT, 0, 0);
	InsertColumn(2, _T("Date Modified"), LVCFMT_LEFT, 0, 0);
	SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
	SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);
	SetColumnWidth(2, LVSCW_AUTOSIZE_USEHEADER);

	HRESULT hr;

	// Populate list
	LPSHELLFOLDER pIDesktop;
	if(S_OK != SHGetDesktopFolder(&pIDesktop)){
		return;
	}
	LPSHELLFOLDER pICurrentFolder;
	if(S_OK != pIDesktop->BindToObject(new_pidl, NULL, IID_IShellFolder, (void**)&pICurrentFolder)){
		pIDesktop->Release();
		return;
	}
	// Initialize the header's text
	LPSHELLFOLDER psfParent;
	LPCITEMIDLIST pidlCurrentRelToParent; // need not free due to SHBindToParent
	hr = SHBindToParent(new_pidl, IID_IShellFolder, (void**)&psfParent, &pidlCurrentRelToParent);
	STRRET strDispName;
	psfParent->GetDisplayNameOf(pidlCurrentRelToParent, SHGDN_NORMAL, &strDispName);
	LPTSTR szDisplayName;
	hr = StrRetToStr(&strDispName, pidlCurrentRelToParent, &szDisplayName);
	header_to_init.SetWindowText(szDisplayName);
	CoTaskMemFree(szDisplayName);
	psfParent->Release();

	pIDesktop->Release();
	// At this point, pICurrentFolder need freeing later on

	LPENUMIDLIST penumIDL = NULL;
	hr = pICurrentFolder->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &penumIDL);

	int pidl_id = 0;
	while(1){
		LPITEMIDLIST pidl;
		if(NO_ERROR == penumIDL->Next(1, &pidl, NULL)){
			LPITEMIDLIST abs_item_pidl = ILCombine(new_pidl, pidl);

			hr = pICurrentFolder->GetDisplayNameOf(pidl, SHGDN_INFOLDER, &strDispName);
			LPTSTR szDisplayName;
			hr = StrRetToStr(&strDispName, pidl, &szDisplayName);

			//pICurrentFolder->GetAttributesOf(

			SHFILEINFO fi;
			ZeroMemory(&fi, sizeof(fi));
			DWORD_PTR ret = SHGetFileInfo((LPCTSTR)abs_item_pidl, 0, &fi, sizeof(fi), SHGFI_PIDL | SHGFI_ICON | SHGFI_SMALLICON);
			int iconindex = icons.AddIcon(fi.hIcon);
			DestroyIcon(fi.hIcon);

			LVITEM item = { 0 };
			item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
			item.iItem = GetItemCount();
			item.iSubItem = 0;
			item.pszText = szDisplayName;
			item.state = 0;
			item.stateMask = 0;
			item.iImage = iconindex;
			item.lParam = abs_item_pidl;
			int itemindex = InsertItem(&item);

			CoTaskMemFree(szDisplayName);
			ILFree(pidl);

			++pidl_id;
		}else{
			break;
		}
	}

	penumIDL->Release();
	pICurrentFolder->Release();


	SetColumnWidth(0, LVSCW_AUTOSIZE);
	SetColumnWidth(1, LVSCW_AUTOSIZE);
	SetColumnWidth(2, LVSCW_AUTOSIZE);


}
LPITEMIDLIST CColumnList::GetSelectedPIDL(int which_item){
	if(which_item == -1){
		which_item = GetSelectionMark();
	}
	LVITEM item = {0};
	item.iItem = which_item;
	item.mask = LVIF_PARAM;
	GetItem(&item);
	// TODO: validation

	return m_pidls[(int)item.lParam];
}
*/