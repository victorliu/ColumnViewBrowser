#include "StdAfx.h"
#include "AddressBar.h"

int AddressBar::OnCreate(LPCREATESTRUCT lpCreateStruct){
	LoadToolBar(IDR_ADDRESSBAR_TOOLBAR);
	m_comboAddress.Create(*this, rcDefault);
	SetButtonInfo(1, IDW_ADDRESS_COMBO, TBBS_SEPARATOR, 100);
}