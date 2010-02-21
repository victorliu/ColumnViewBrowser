#pragma once

class CAddressBar : public CWindowImpl<CAddressBar, CComboBox>
{
public:
	DECLARE_WND_SUPERCLASS(NULL, WC_COMBOBOX)
protected:
	CComboBox m_comboAddress;

	BEGIN_MSG_MAP(CAddressBar)
		MSG_WM_CREATE(OnCreate)
		FORWARD_NOTIFICATIONS()
	END_MSG_MAP()

	int OnCreate(LPCREATESTRUCT lpCreateStruct);
};
