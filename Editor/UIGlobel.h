#pragma once
class CUIGlobalData
{
public:
	CUIGlobalData();
	~CUIGlobalData();
	CFont * GetGFont();
	void    SetGFont(CFont *);
private:
	CFont  *m_pFont;
	 
};

extern CUIGlobalData _uiGlobalData;