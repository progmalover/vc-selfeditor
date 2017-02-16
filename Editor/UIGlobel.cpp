#include "stdafx.h"
#include "UIGlobel.h"

CUIGlobalData _uiGlobalData;

CUIGlobalData::CUIGlobalData() :m_pFont(nullptr)
{

	if (nullptr == m_pFont)
	{
		m_pFont = CFont::FromHandle((HFONT)::GetStockObject(ANSI_FIXED_FONT));
	}
}


CUIGlobalData::~CUIGlobalData()
{
}

CFont * CUIGlobalData::GetGFont()
{
	return m_pFont;
}

void CUIGlobalData::SetGFont(CFont *pFont)
{
	
	m_pFont = pFont;
}