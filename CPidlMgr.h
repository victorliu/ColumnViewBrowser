#ifndef _CPIDLMGR_H_
#define _CPIDLMGR_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef	enum tagITEM_TYPE
{
	NWS_FOLDER=0x00000001,
	NWS_FILE=0x00000002,
}ITEM_TYPE;

typedef struct tagPIDLDATA
   {
   ITEM_TYPE type;
   TCHAR    szName[1];
   }PIDLDATA, FAR *LPPIDLDATA;

class CPidlMgr
{
public:

	//static LPITEMIDLIST Create(ITEM_TYPE iItemType,LPTSTR szName);
	static void Delete(LPITEMIDLIST pidl); // ILFree
	static LPITEMIDLIST GetNextItem(LPCITEMIDLIST pidl);
	static LPITEMIDLIST GetLastItem(LPCITEMIDLIST pidl);
	static UINT GetByteSize(LPCITEMIDLIST pidl);
	static bool IsSingle(LPCITEMIDLIST pidl);
	static LPITEMIDLIST Concatenate(LPCITEMIDLIST, LPCITEMIDLIST); // ILCombine
	static LPITEMIDLIST Copy(LPCITEMIDLIST pidlSrc); // ILCline

	// Retrieve the item type (see above)
	static ITEM_TYPE GetItemType(LPCITEMIDLIST pidl);
	static HRESULT GetFullName(LPCITEMIDLIST pidl,LPTSTR szFullName,DWORD *pdwLen); 
	static BOOL  HasSubFolder(LPCITEMIDLIST pidl); 
	static HRESULT GetItemAttributes(LPCITEMIDLIST pidl,USHORT iAttrNum,LPTSTR pszAttrOut);

private:
	static LPPIDLDATA GetDataPointer(LPCITEMIDLIST);
	static HRESULT GetName(LPCITEMIDLIST pidl,LPTSTR pszName);
};

#endif//_CNWSPIDLMGR_H_