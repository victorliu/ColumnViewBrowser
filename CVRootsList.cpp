#include "StdAfx.h"
#include "CVRootsList.h"
#include "CPidlMgr.h"

// Assumes COM already initialized (CoInitialize() already run)
void CCVRootsList::Init(){
	int iconsize = GetSystemMetrics(SM_CXSMICON | SM_CYSMICON);
	icons.Create(iconsize, iconsize, ILC_COLOR32, 10, 10);
	SetExtendedListViewStyle ( LVS_EX_FULLROWSELECT );
	SetImageList(icons, LVSIL_SMALL);
	EnableGroupView(TRUE);

	InsertColumn(0, _T("Locations"), LVCFMT_LEFT, 0, 0);
	SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
	LVGROUP group;
	group.cbSize = sizeof(LVGROUP);
	group.mask = LVGF_HEADER | LVGF_STATE | LVGF_ALIGN | LVGF_GROUPID;
	group.pszHeader = _T("Drives");
	group.cchHeader = 0;
	group.iGroupId = 0;
	group.stateMask = LVGS_NORMAL;
	group.state = LVGS_NORMAL;
	group.uAlign = LVGA_HEADER_LEFT;
	InsertGroup(0, &group);
	group.pszHeader = _T("Favorites");
	InsertGroup(1, &group);

	// Find the drives in the system
	LPSHELLFOLDER pIDesktop;
	if(S_OK != SHGetDesktopFolder(&pIDesktop)){
		return;
	}
	LPITEMIDLIST drives_pidl;
	if(S_OK != SHGetFolderLocation(NULL, CSIDL_DRIVES, NULL, 0, &drives_pidl)){
		pIDesktop->Release();
		return;
	}
	LPSHELLFOLDER pIDrives;
	if(S_OK != pIDesktop->BindToObject(drives_pidl, NULL, IID_IShellFolder, (void**)&pIDrives)){
		pIDesktop->Release();
		ILFree(drives_pidl);
		return;
	}
	pIDesktop->Release();

	HRESULT hr;
	LPENUMIDLIST penumIDL = NULL;
	hr = pIDrives->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &penumIDL);

	STRRET strDispName;
	while(1){
		LPITEMIDLIST pidl;
		if(NO_ERROR == penumIDL->Next(1, &pidl, NULL)){
			LPITEMIDLIST abs_pidl = CPidlMgr::Concatenate(drives_pidl, pidl); // DO NOT FREE THIS!
			m_pidls[m_nNextMapKey] = abs_pidl;

			hr = pIDrives->GetDisplayNameOf(pidl, SHGDN_INFOLDER, &strDispName);
			LPTSTR szDisplayName;
			hr = StrRetToStr(&strDispName, pidl, &szDisplayName);

			SHFILEINFO fi;
			ZeroMemory(&fi, sizeof(fi));
			DWORD_PTR ret = SHGetFileInfo((LPCTSTR)abs_pidl, 0, &fi, sizeof(fi), SHGFI_PIDL | SHGFI_ICON | SHGFI_SMALLICON);
			int iconindex = icons.AddIcon(fi.hIcon);
			DestroyIcon(fi.hIcon);

			LVITEM item = { 0 };
			item.mask = LVIF_GROUPID | LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
			item.iItem = GetItemCount();
			item.iSubItem = 0;
			item.pszText = szDisplayName;
			item.state = 0;
			item.stateMask = 0;
			item.iImage = iconindex;
			item.lParam = m_nNextMapKey;
			item.iGroupId = 0;
			int itemindex = InsertItem(&item);

			m_nNextMapKey++;

			CoTaskMemFree(szDisplayName);
			ILFree(pidl);
		}else{
			break;
		}
	}

	ILFree(drives_pidl);
	penumIDL->Release();
	pIDrives->Release();

	// Find out out of username/Links exists

	// Make a decent initial selection choice
	int n = GetItemCount();
	int best_type_so_far = DRIVE_UNKNOWN;
	int best_item_so_far = -1;
	for(int i = 0; i < n; ++i){
		LVITEM item = {0};
		item.iItem = i;
		item.mask = LVIF_PARAM;
		if(!GetItem(&item)){ continue; }
		TCHAR szPath[MAX_PATH];
		if(!SHGetPathFromIDList(m_pidls[(int)item.lParam], szPath)){ continue; }
		int type = GetDriveType(szPath);
		if(DRIVE_FIXED == type){
			best_type_so_far = DRIVE_FIXED;
			best_item_so_far = i;
			break;
		}else if(DRIVE_FIXED != best_type_so_far && DRIVE_UNKNOWN != type){
			best_type_so_far = type;
			best_item_so_far = i;
		}
	}
	SetItemState(best_item_so_far, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
	NMLISTVIEW nmlv = {0};
	nmlv.hdr.hwndFrom = *this;
	nmlv.hdr.idFrom = GetDlgCtrlID();
	nmlv.hdr.code = LVN_ITEMCHANGED;
	nmlv.iItem = best_item_so_far;
	nmlv.uNewState = LVIS_SELECTED;
	PostMessage(WM_NOTIFY, nmlv.hdr.idFrom, (LPARAM)&nmlv) ;
}

void CCVRootsList::OnDestroy(){
	// free all the pidls
	pidl_map_t::iterator i;
	for(i = m_pidls.begin(); i != m_pidls.end(); ++i){
		CPidlMgr::Delete(i->second);
	}
}
LPITEMIDLIST CCVRootsList::GetSelectedPIDL(int which_item){
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