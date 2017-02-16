// LMEditWnd.cpp : implementation file
//

#include "stdafx.h"
#include "Editor.h"
#include "LMEditWnd.h"
#include "UIGlobel.h"
#define ID_COPY  2222 //copy string
#define ID_PARST 3333 // parst string
#define ID_UNDO  334
#define ID_REDO  335
// CLMEditWnd

#define MARGIN_PAD 2
class wndRegister
{
public:
	wndRegister()
	{
		strClass = AfxRegisterWndClass(
			CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS,
			  ::LoadCursor(NULL, IDC_ARROW),
			  (HBRUSH) ::GetStockObject(WHITE_BRUSH),
			  ::LoadIcon(NULL, IDI_APPLICATION));
	}
	~wndRegister()
	{
			 UnregisterClass(strClass,AfxGetInstanceHandle());
	}
	static CString strClass;
};

CString wndRegister::strClass;
 
IMPLEMENT_DYNAMIC(CLMEditWnd, CWnd)

CLMEditWnd::CLMEditWnd()
{
  static	wndRegister reg;
}

CLMEditWnd::~CLMEditWnd()
{
}


BEGIN_MESSAGE_MAP(CLMEditWnd, CWnd)
	ON_WM_LBUTTONDOWN()
	ON_WM_CONTEXTMENU()
//	ON_WM_CHAR()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
	ON_WM_GETDLGCODE()
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_LBUTTONUP()
    ON_WM_CHAR()
    ON_COMMAND(ID_COPY,OnCopy)
    ON_COMMAND(ID_PARST,OnParst)
	ON_COMMAND(ID_UNDO,OnUndo)
    ON_COMMAND(ID_REDO,OnRedo)
	
    ON_UPDATE_COMMAND_UI(ID_COPY,OnUpdateCopy)
    ON_UPDATE_COMMAND_UI(ID_PARST,OnUpdateParst)
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()



// CLMEditWnd message handlers

BOOL CLMEditWnd::CreateEdit(LPCTSTR lpStrName,CRect rect,CWnd *pOwner ,DWORD dwID)
{
	if(!pOwner || !::IsWindow(pOwner->GetSafeHwnd()))
		return FALSE;
	return CWnd::Create(wndRegister::strClass,
		lpStrName,WS_CHILD|WS_TABSTOP|WS_CLIPSIBLINGS ,rect,pOwner,dwID);
}


BOOL CLMEditWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Add your specialized code here and/or call the base class
	cs.dwExStyle |= WS_EX_CLIENTEDGE ;
	return CWnd::PreCreateWindow(cs);
}

void CLMEditWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	// TODO: Add your message handler code here
    CMenu contextMenu;
    contextMenu.CreatePopupMenu();
    contextMenu.AppendMenu(MF_STRING|MF_POPUP,ID_COPY,_T("Copy"));
    contextMenu.AppendMenu(MF_STRING|MF_POPUP,ID_PARST,_T("Parste"));
    contextMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON,point.x,point.y,this);
}


void CLMEditWnd::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	this->m_txtEditor.selectAll();
	this->SetCaretPos(this->GetCaretPos());
	this->Invalidate();
	CWnd::OnLButtonDblClk(nFlags, point);
}


BOOL CLMEditWnd::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	

	if(pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST)
	{

		DWORD dwRet = ::TranslateAccelerator(GetSafeHwnd(),m_cmdAccel,pMsg);
		TRACE("TranslateAccelerator dwRet :%d ,err %d\n",dwRet,GetLastError());
	}
	return CWnd::PreTranslateMessage(pMsg);
}


void CLMEditWnd::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus(pOldWnd);
	if(!m_bcaret)//recreate caret
	{
		m_bcaret = true;
		CDC *pDC = GetDC();
		pDC->SaveDC();
		pDC->SelectObject(_uiGlobalData .GetGFont());
		int ch = pDC->GetTextExtent(_T("H"),1).cy;
		::CreateCaret(GetSafeHwnd(),NULL,1,ch);
		pDC->RestoreDC(-1);
	}

	ShowCaret();

	CPoint pt = GetCaretPos();
	if (pt.x == 0)pt.x += MARGIN_PAD;
	this->SetCaretPos(pt);
	
	this->Invalidate();
	// TODO: Add your message handler code here
}


void CLMEditWnd::OnKillFocus(CWnd* pNewWnd)
{
	CWnd::OnKillFocus(pNewWnd);
	m_bcaret = false;
    this->Invalidate();
    this->HideCaret();
	// TODO: Add your message handler code here
}

void CLMEditWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	 
	this->SetFocus();
	// TODO: Add your message handler code here and/or call default
	
	int i = m_txtEditor.mapPointToIndex(point);
	POINT pt = m_txtEditor.getIndexPos(i); // index to client pos.
	while(pt.x < 0)
	{
		m_txtEditor.scrollLeft();
		pt = m_txtEditor.getIndexPos(i);
	}

	CRect rc;
	GetClientRect(&rc);
	while (pt.x > rc.right - MARGIN_PAD)
	{
		m_txtEditor.scrollRight();
		pt = m_txtEditor.getIndexPos(i);
	}
	m_txtEditor.setCaretIndex(i);

	if (pt.x <= 0)
		pt.x = MARGIN_PAD;

	this->SetCaretPos(CPoint(pt.x,0));
	m_bDrag = false;
	m_txtEditor.resetDrag();
	::SetCapture(this->GetSafeHwnd());
	Invalidate();
	
	CWnd::OnLButtonDown(nFlags, point);
}

void CLMEditWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	
	
	::ReleaseCapture();
	CWnd::OnLButtonUp(nFlags, point);
}

void CLMEditWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	//
	if((nFlags & MK_LBUTTON) == MK_LBUTTON && m_txtEditor.GetTxtLen()>0)
	{
		if(false == m_bDrag)
		{
			m_txtEditor.setDragPosIndex(m_txtEditor.getCaretIndex());//set drag index in string buffer.
			m_bDrag = true;
		}
		int i = m_txtEditor.mapPointToIndex(point);

		TRACE1("move index:%d\n",i);
		m_txtEditor.setCaretIndex(i);//set current caret pos.
		
		SetCaretPos(GetCaretPos());
		Invalidate();
	}
	
	CWnd::OnMouseMove(nFlags, point);
}

void CLMEditWnd::InputChar(TCHAR nChar)
{
    m_txtEditor.enterChar(nChar);
    while(!IsVisibleIndex(m_txtEditor.getCaretIndex()))
        m_txtEditor.scrollRight();
    SetCaretPos(GetCaretPos());
}

void  CLMEditWnd::AdjustCaret()
{

}

void CLMEditWnd::OnBackSpace()
{
    if(!m_txtEditor.delPreChar())
        return;
    CPoint pt = m_txtEditor.getIndexPos(m_txtEditor.getCaretIndex());
    this->SetCaretPos(pt);
    Invalidate();
    return;
}

#define LEFT  0
#define RIGHT 1
void CLMEditWnd::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{// TODO: Add your message handler code here and/or call default
	if(GetKeyState(VK_LBUTTON)&0x80 )
		return;
	if(CWnd::GetFocus()!= this)
		return;  
    if(GetAsyncKeyState(VK_CONTROL)!= 0)
        return;
    // TODO: Add your message handler code here and/or call default
   // if(::isalpha(nChar) || ::isalnum(nChar) /*||AllowIn(nChar)*/)
    if(nChar == 8)//back space
    {
        return OnBackSpace();
    }

    InputChar(nChar);
    Invalidate();
    
    //CWnd::OnChar(nChar, nRepCnt, nFlags);
}

void CLMEditWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch(nChar)
	{
		case VK_LEFT:
			{
				m_txtEditor.MoveCaret(LEFT);

				if (m_txtEditor.canScrollLeft())
					m_txtEditor.scrollLeft();

				SetCaretPos(GetCaretPos());
				Invalidate();
			}
			return;

		case VK_RIGHT:
			m_txtEditor.MoveCaret(RIGHT);

			//if (m_txtEditor.canScrollRight())
			if (!IsVisibleIndex(m_txtEditor.getCaretIndex()))
				m_txtEditor.scrollRight();

			SetCaretPos(GetCaretPos());
			Invalidate();
			return;
        
        case VK_DELETE:
            {
                m_txtEditor.delSelection();
                Invalidate();
            }
            return;
        case VK_UP:
        case VK_DOWN:
            return;
	};
}

CPoint CLMEditWnd::GetCaretPos()
{
	CRect cliRc;
	CPoint pt(0,0);
	pt = m_txtEditor.getIndexPos(m_txtEditor.getCaretIndex()); // index to client pos.
	GetClientRect(cliRc);
	if(pt.x < 0) pt.x = MARGIN_PAD;
	if(pt.x > cliRc.right)
		pt.x = cliRc.right - 5;
	return pt;
}

UINT CLMEditWnd::OnGetDlgCode()
{
	// TODO: Add your message handler code here and/or call default
	return CWnd::OnGetDlgCode()|DLGC_WANTARROWS|DLGC_WANTCHARS;
}


int CLMEditWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CFont *cf = CFont::FromHandle((HFONT)::GetStockObject(ANSI_FIXED_FONT));
	/*
	LOGFONT lgf;
	cf->GetLogFont(&lgf);
	_tcscpy(lgf.lfFaceName , _T("新宋体"));

	CFont *pnft = new CFont(); 
	pnft->CreatePointFontIndirect(&lgf);
	*/
	this->SetFont(cf ,1);
   

	CDC *pDC = GetDC();

	int ch = pDC->GetTextExtent(_T("h"),1).cy;
	::CreateCaret(GetSafeHwnd(),NULL,1,ch);
	ReleaseDC(pDC);
	// TODO:  Add your specialized creation code here
	m_bcaret = true;
	m_txtEditor.setLmWnd(this);
	ACCEL accels[4] =
	{
		{FCONTROL|FVIRTKEY,'Z',ID_UNDO},
		{FCONTROL|FVIRTKEY,'R',ID_REDO},
		{FCONTROL|FVIRTKEY,'C',ID_COPY},
		{FCONTROL|FVIRTKEY,'V',ID_PARST}
	};
	m_cmdAccel = ::CreateAcceleratorTable(accels,sizeof(accels)/sizeof(ACCEL));
	return 0;
}

bool  CLMEditWnd::IsVisibleIndex(int i)
{
	CPoint pt(0,0);
	pt = m_txtEditor.getIndexPos(i);
	if(pt.x <= 0) //Ramain 2 pixels width margin on left.
		return false;

	CRect cliRect;
	GetClientRect(cliRect);
	if(pt.x > (cliRect.right -3)) //Remain 3 pixels width margin on right.
		return false;
	return true;
}

void CLMEditWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	
	if(m_txtEditor.GetTxtLen() <= 0)
		return;
	
	const TCHAR *lpStr = m_txtEditor.GetVisibleString();
	int ise = m_txtEditor.GetVisiblilityEnabledLen();
	int icn = _tcslen(lpStr);
	ise = (ise <= _tcslen(lpStr)) ? ise : icn;
	CRect rc(0,0,0,0);

	dc.SaveDC();
	dc.SelectObject(_uiGlobalData.GetGFont());

	dc.DrawText((LPTSTR)lpStr, ise, &rc, DT_CALCRECT | DT_SINGLELINE | DT_LEFT | DT_NOCLIP);
	dc.DrawText(lpStr, ise, &rc, DT_LEFT | DT_NOCLIP);

	if (CWnd::GetFocus() == this)
	{
		if (m_txtEditor.hasSelection())
		{
			int ss, se;
			const TCHAR *pbuf = m_txtEditor.getSelection(ss, se);
			TRACE2("ss:%d,se:%d",ss,se);
			
			LM_TXTSTRUCTION txt = m_txtEditor.getText();
			/*if ((se - ss) > m_txtEditor.GetVisiblilityEnabledLen() && txt.lm_vStart <= ss)
			{
				pbuf += ss - txt.lm_vStart;
			}*/
			
			dc.SetBkColor(::GetSysColor(COLOR_HOTLIGHT));
			dc.SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));

			int ibound = se - ss;
			int offss = ss;
			if (m_txtEditor.getDragDirector() == 0) //drag to right
			{
				int ive = ise + txt.lm_vStart;
				
				if (m_txtEditor.isSelectAllMode()) //already select all
				{
					offss = txt.lm_vStart;
					pbuf += offss;
				}else
				if (se > ive)
				{
					offss -= (se - ive) ;
					if (offss <= 0)
					{
						pbuf -= offss;
						offss = 0;
					}

				}
			}
			ibound = min(ibound, ise);
			CPoint offPos = m_txtEditor.getIndexPos(offss);
			dc.DrawText(pbuf, ibound, &rc, DT_CALCRECT);
			rc.OffsetRect(offPos.x, 0);
			dc.DrawText(pbuf, ibound, &rc, DT_NOCLIP | DT_LEFT);
			 
		}
	}

	dc.RestoreDC(-1);
}

void CLMEditWnd::OnUpdateCopy(CCmdUI *pIt)
{
    if(m_txtEditor.hasSelection())
        pIt->Enable(TRUE);
    else
        pIt->Enable(FALSE);
}

void CLMEditWnd::OnUpdateParst(CCmdUI *pIt)
{
        pIt->Enable(TRUE);
}

void CLMEditWnd::OnCopy()
{
    //int s = 30;
    if(!m_txtEditor.hasSelection())
        return;
  
    if (!OpenClipboard ())
    {
        TRACE0("Can't open clipboard\n");
        return ;
    }

    if (!::EmptyClipboard ())
    {
        TRACE0("Can't empty clipboard\n");
        ::CloseClipboard ();
        return ;
    }

    int ss,se;
    const TCHAR *pTxtData = m_txtEditor.getSelection(ss,se,true);
    int nlen = se - ss;
    HGLOBAL  hglbCopy = GlobalAlloc(GMEM_SHARE,(nlen + 1)*sizeof(TCHAR)); 
    if (hglbCopy == NULL) 
    { 
        CloseClipboard(); 
        return ; 
    } 

    char *pBuffer = (char *)GlobalLock(hglbCopy); 

#ifdef _UNICODE
	std::string str = CW2A(pTxtData).m_psz;
	int leng = str.length();
    memcpy(pBuffer,str.c_str(),str.length());
#else
	memcpy(pBuffer, pTxtData,nlen );
#endif

    pBuffer[leng] =  0;    // null character 
    GlobalUnlock(hglbCopy); 

    // Place the handle on the clipboard. 

    SetClipboardData(CF_TEXT, hglbCopy); 
    CloseClipboard(); 

}

void CLMEditWnd::OnParst()
{
    if (!IsClipboardFormatAvailable(CF_TEXT)) 
        return; 
    if (!OpenClipboard()) 
        return; 

    HGLOBAL  hglb = GetClipboardData(CF_TEXT); 
    if (hglb != NULL) 
    { 
        char *lptstr = (char *)GlobalLock(hglb); 
#ifdef  _UNICODE
		CString wStr = CA2W(lptstr).m_psz;
		for (int i = 0; i < wStr.GetLength(); i++){
				InputChar(wStr.GetAt(i));
		}

#else
		if (lptstr != NULL)
		{
			for (int i = 0; i < strlen(lptstr); i++)
				InputChar(lptstr[i]);

		}

#endif
    
		GlobalUnlock(hglb);
    } 
    CloseClipboard(); 
    Invalidate();
}

void CLMEditWnd:: OnUndo()
{
	m_txtEditor.m_CmdEngin.unexec();
}

void CLMEditWnd:: OnRedo()
{
	m_txtEditor.m_CmdEngin.reexec();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//backsapce pressed
bool CLMEditWnd::lm_txtEditor::delPreChar(bool bTrace)
{
    if(delSelection(bTrace))
        return true;
    if(m_txt.lm_txtBuffer.GetLength() <= 0)
        return false;
    if(m_txt.lm_caretPoint <= 0)
    {
        m_txt.lm_caretPoint = 0;
        return false;
    }

	if(bTrace)
	m_CmdEngin.BeginDelChar(m_txt.lm_caretPoint-1,1);
    if(m_txt.lm_vStart > 0)
        m_txt.lm_vStart--;
    m_txt.lm_caretPoint--;
	
    m_txt.lm_txtBuffer.Delete(m_txt.lm_caretPoint,1);
	if(bTrace)
    m_CmdEngin.EndDelChar();
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//lm_txtEditor
//delete last selection range.
 
bool CLMEditWnd::lm_txtEditor::delSelection(bool bTrace)
{
	if(-1 == m_txt.lm_dragOver) //no selection
		return false;

	
	if(m_txt.lm_dragOver != m_txt.lm_caretPoint)
	{
		
		int delStart = min(m_txt.lm_dragOver,m_txt.lm_caretPoint);
		int delEnd = max(m_txt.lm_dragOver,m_txt.lm_caretPoint);
        int delNum = delEnd - delStart;
		
		//caret index:1->char num
		setCaretIndex(delEnd);//according caret index to restore and redel,so set to right place.
		while(--delNum >= 0)
		{
			if(bTrace)
			m_CmdEngin.BeginDelChar(delEnd-1,1);

			m_txt.lm_txtBuffer.Delete(delEnd-1,1);
			m_txt.lm_dragOver = -1;
			if(m_txt.lm_vStart > 0 && delNum-1 >0)
			{
				m_txt.lm_vStart--;
			}
			delEnd--;
			setCaretIndex(delEnd);
			if(bTrace)
			m_CmdEngin.EndDelChar();
		}
	}
	
	return true;
}

int CLMEditWnd::lm_txtEditor::mapPointToIndex(POINT pos)
{
	//buffer is empty,or all scroll by.
	if(m_txt.lm_vStart >= m_txt.lm_txtBuffer.GetLength())
		return 0;
	
	const TCHAR * txt_buf = m_txt.lm_txtBuffer.GetBuffer() + m_txt.lm_vStart;
	int bufLen = _tcslen(txt_buf);

	CDC dc;
	dc.Attach(::GetDC(m_CmdEngin.getOwnerWnd()->GetSafeHwnd()));
	dc.SaveDC();
	dc.SelectObject(_uiGlobalData.GetGFont());
	//position is at end of text.

	int index = 0;
	if (pos.x >= dc.GetTextExtent(txt_buf, bufLen).cx)
	{
		//dc.RestoreDC(-1);
		index =  m_txt.lm_txtBuffer.GetLength();
		goto _Exit;
	}
    int chW = dc.GetTextExtent(_T("h"),1).cx / 2;

	//find compatible index.
	for(int i = 1;i < bufLen;i++)
	{
		int x = dc.GetTextExtent(txt_buf,i).cx;
		if (abs(x - pos.x) <= chW)
		{
			//dc.RestoreDC(-1);
			index =  i + m_txt.lm_vStart;
			goto _Exit;
		}
	}

	_Exit:
		dc.RestoreDC(-1);
	return index;
}

//移动光标,必要时带动字串移动
bool CLMEditWnd::lm_txtEditor::MoveCaret(int tag)
{
	if( 1 == tag) //move left
	{
		m_txt.lm_caretPoint++;
		if( m_txt.lm_caretPoint > m_txt.lm_txtBuffer.GetLength())
			m_txt.lm_caretPoint = m_txt.lm_txtBuffer.GetLength();
	}else //move right
	{
		m_txt.lm_caretPoint--;
		if(m_txt.lm_caretPoint <= 0)
			m_txt.lm_caretPoint = 0;
	}
	m_txt.lm_dragOver = -1;
	return true;
}

const TCHAR * CLMEditWnd::lm_txtEditor::GetVisibleString()
{
	const TCHAR *ch_buff = m_txt.lm_txtBuffer.GetBuffer() + m_txt.lm_vStart;
	return ch_buff; 
}

const int CLMEditWnd::lm_txtEditor::GetVisiblilityEnabledLen()
{
	CLMEditWnd *plmew = m_CmdEngin.getOwnerWnd();
	if (NULL == plmew)
		return 0;

	CRect  cwRect;
	int    count = 0;
	plmew->GetClientRect(&cwRect);

	CDC *pDC = plmew->GetDC();
	pDC->SaveDC();

	TEXTMETRIC tm;
	pDC->GetTextMetrics(&tm);
	//int cw = pDC->GetTextExtent(_T("H"),1).cx;
	//int cw2 = pDC->GetTextExtent(_T("HH"), 2).cx;
	cwRect.DeflateRect(2, 0);
	count = cwRect.Width() / (tm.tmAveCharWidth + 1) ;

	TRACE2( "clirect width:%d, enable visibility:%d \n",cwRect.Width(),count);

	pDC->RestoreDC(-1);
	
	return count;
}
//
//get position according to client window .
//
POINT CLMEditWnd::lm_txtEditor::getIndexPos(int i)
{
	POINT pt = {0,0};

	int vStart = ((m_txt.lm_vStart < 0) ? 0 : m_txt.lm_vStart);
	int iRange = i - vStart;

	//ASSERT(iRange >= 0 );
	if(iRange <= 0)
	{
		//pt.x = -1;
		return pt;
	}

	 
	CDC dc;
	dc.Attach(::GetDC(m_CmdEngin.getOwnerWnd()->GetSafeHwnd()));
	dc.SaveDC();
	dc.SelectObject(_uiGlobalData.GetGFont());
	
	const TCHAR *ch_buff = m_txt.lm_txtBuffer.GetBuffer() + vStart;
	if(i >= m_txt.lm_txtBuffer.GetLength())
	{
		pt.x = dc.GetTextExtent(ch_buff,_tcslen(ch_buff)).cx;
		//return pt;
		goto _Exit;
	}

	pt.x = dc.GetTextExtent(ch_buff,iRange).cx;
	if(pt.x <= 0)
		pt.x = MARGIN_PAD;

	_Exit:
	dc.RestoreDC(-1);
	return pt; 
}

//set caret index
int CLMEditWnd::lm_txtEditor::setCaretIndex(int i)
{
	 int n = m_txt.lm_caretPoint;
	 m_txt.lm_caretPoint = i;
	 //m_txt.lm_dragOver = -1;//free selection
	 return n;
}
//get  caret index of textbuffer. 
int CLMEditWnd::lm_txtEditor::getCaretIndex()
{
	return m_txt.lm_caretPoint;
}

void  CLMEditWnd::lm_txtEditor::setDragPosIndex(int index)//set drag end index
{
	m_txt.lm_dragOver = index;
}

bool  CLMEditWnd::lm_txtEditor::canScrollLeft()
{

	return ((m_txt.lm_vStart >= 1) && ((getCaretIndex() - m_txt.lm_vStart) <= 0));
}

void  CLMEditWnd::lm_txtEditor::scrollLeft()  //called move left one char.
{
	m_txt.lm_vStart--;
}

bool  CLMEditWnd::lm_txtEditor::canScrollRight()
{
	int iCaret = getCaretIndex();
	return ((m_txt.lm_vStart + iCaret) < m_txt.lm_txtBuffer.GetLength()) ;
}

void  CLMEditWnd::lm_txtEditor::scrollRight() //called move right one char.
{
	m_txt.lm_vStart++;
}

bool  CLMEditWnd::lm_txtEditor::enterChar(TCHAR ch,bool bTrace)//put char to caret location 
{
	this->delSelection(bTrace);
	if(bTrace)
	m_CmdEngin.BeginEnterChar(ch);

	int ins = m_txt.lm_caretPoint;

	//ins += m_txt.lm_vStart;

	if(m_txt.lm_txtBuffer.GetLength() -1 <= m_txt.lm_caretPoint)
		m_txt.lm_txtBuffer += ch;
	else
		m_txt.lm_txtBuffer.Insert(ins, ch);
	
	m_txt.lm_caretPoint++;
    m_txt.lm_dragOver = -1;
	if(bTrace)
	m_CmdEngin.EndEnterChar();
	return true;
}

bool  CLMEditWnd::lm_txtEditor::isSelectAllMode()
{
	return (this->m_txt.lm_dragOver == 0 && this->m_txt.lm_caretPoint == this->m_txt.lm_txtBuffer.GetLength());
}

bool  CLMEditWnd::lm_txtEditor::hasSelection()//some chars in drag option range
{
	return (m_txt.lm_dragOver != -1);
}

CString  CLMEditWnd::lm_txtEditor::getText(int index,int num)
{
	CString strText;
	if(index < 0 || index >= m_txt.lm_txtBuffer.GetLength())
	{
		TRACE("Get Index %d error!\n",index);
		return strText;	
	}

	if(index + num >= m_txt.lm_txtBuffer.GetLength())
	{
		TRACE("get text is byoned!");
		num = m_txt.lm_txtBuffer.GetLength() - index;
		//return strText;
	}
	strText.SetString(m_txt.lm_txtBuffer.GetBuffer() + index,num);
	
	return strText;
}

int  CLMEditWnd::lm_txtEditor::getDragDirector()
{
	return (m_txt.lm_caretPoint > m_txt.lm_dragOver) ? 0 : 1;
}

const TCHAR *  CLMEditWnd::lm_txtEditor::getSelection(int &iStart,int &iEnd ,bool bsub)//get selection range
{
	iStart = 0;
	iEnd = 0;

	if(m_txt.lm_dragOver == -1)
		return false;
	iStart = min(m_txt.lm_caretPoint,m_txt.lm_dragOver);
	iEnd = max(m_txt.lm_caretPoint,m_txt.lm_dragOver);

	if (bsub)
	{
		static CString substr;

		substr.Empty();

		for (int i = iStart; i < iEnd; i++)
		{
			substr += m_txt.lm_txtBuffer.GetAt(i);
		}

		return substr;
	}
	else
		return (m_txt.lm_txtBuffer.GetBuffer() + iStart);
}

bool  CLMEditWnd::lm_txtEditor::selectAll() //select all char
{
//his->hasSelection();
	m_txt.lm_dragOver = 0;
	m_txt.lm_caretPoint = m_txt.lm_txtBuffer.GetLength();
	m_txt.lm_vStart = max(m_txt.lm_caretPoint - this->GetVisiblilityEnabledLen(), 0);
	return true;
}

bool  CLMEditWnd::lm_txtEditor::unselectAll()//unselect all char
{
	m_txt.lm_dragOver = -1;
	return true;
}

//
//only implement "insert" and "del" command object here. 
//three months ago ,i implement cmd system for our "magicview controller" project ,about eight thousant
//codes,so very hate it.
//
bool  CLMEditWnd::lm_txtCmdEngin::enter_cmd::reexec()
{
	CLMEditWnd *pWnd = m_pEngin->getOwnerWnd();
	lm_txtEditor &Editor = *m_pEngin->getEditor();
	Editor.setCaretIndex(m_startCaret);
	for(int i = 0;i < m_strChars.GetLength();i++)
		Editor.enterChar(m_strChars.GetAt(i),false);
	//restore context
	Editor.m_txt.lm_vStart = m_vStart2;
	Editor.setCaretIndex(m_endCaret);
	pWnd->SetCaretPos(pWnd->GetCaretPos());
	pWnd->Invalidate();
	return true;
}

bool CLMEditWnd::lm_txtCmdEngin::enter_cmd::unexec()
{
	CLMEditWnd *pWnd = m_pEngin->getOwnerWnd();
	lm_txtEditor &Editor = *m_pEngin->getEditor();
	Editor.setCaretIndex(m_endCaret);
	for(int i = 0;i < m_strChars.GetLength();i++)
		Editor.delPreChar(false);
	//restore context
	Editor.m_txt.lm_vStart = m_vStart1;
	Editor.setCaretIndex(m_startCaret);
    pWnd->SetCaretPos(pWnd->GetCaretPos());
	pWnd->Invalidate();
	return true;
}

bool CLMEditWnd::lm_txtCmdEngin::del_cmd::reexec()
{
	CLMEditWnd *pWnd = m_pEngin->getOwnerWnd();
	lm_txtEditor &Editor = *m_pEngin->getEditor();
	Editor.setCaretIndex(max(m_startCaret,m_endCaret));
	for(int i = 0;i < m_strChars.GetLength();i++)
		Editor.delPreChar(false);
	//restore context
	Editor.m_txt.lm_vStart = m_vStart2;
	Editor.setCaretIndex(m_endCaret);
	Editor.resetDrag();
	pWnd->SetCaretPos(pWnd->GetCaretPos());
	pWnd->Invalidate();
	return true;
}

bool CLMEditWnd::lm_txtCmdEngin::del_cmd::unexec()
{
	CLMEditWnd *pWnd = m_pEngin->getOwnerWnd();
	lm_txtEditor &Editor = *m_pEngin->getEditor();
	Editor.setCaretIndex(m_endCaret);
	int i = m_strChars.GetLength();
	while(--i >= 0)
		Editor.enterChar(m_strChars.GetAt(i),false);
	//restore context
	Editor.m_txt.lm_vStart = m_vStart1;
	Editor.setCaretIndex(m_startCaret);
	pWnd->SetCaretPos(pWnd->GetCaretPos());
	pWnd->Invalidate();
	return true;
}

//
//lm_txtCmdEngin
//

CLMEditWnd::lm_txtCmdEngin::~lm_txtCmdEngin()
{
	while(m_unStack.size())
	{
		cmd *p = m_unStack.top();
		m_unStack.pop();
		delete p;
	}

	while(m_reStack.size())
	{
		cmd *p = m_reStack.top();
		m_reStack.pop();
		delete p;
	}
}

bool CLMEditWnd::lm_txtCmdEngin::reexec()
{
	if(m_reStack.size())
	{
		lm_txtCmdEngin::cmd *p = m_reStack.top();
		p->reexec();
		m_reStack.pop();
		m_unStack.push(p);
	}
	return true;
}
bool CLMEditWnd::lm_txtCmdEngin::unexec()
{
	if(m_unStack.size())
	{
		lm_txtCmdEngin::cmd *p = m_unStack.top();
		p->unexec();
		m_unStack.pop();
		m_reStack.push(p);
	}
	return true;
}

bool CLMEditWnd::lm_txtCmdEngin::canreexec()
{
	return m_reStack.size() > 0;
}

bool CLMEditWnd::lm_txtCmdEngin::canunexec()
{
	return m_unStack.size() > 0;
}

void CLMEditWnd::lm_txtCmdEngin::ClearStack(cmd_stack *stack)
{
	while(stack->size())
	{
		cmd *pcmd = stack->top();
		switch(pcmd->getTag())
		{
		case CMD_CHAR:
			delete (enter_cmd *)pcmd;
			break;
		case CMD_DEL:
			delete (del_cmd *)pcmd;
			break;
		}
		stack->pop();
	}
	
}
//record and merger cmd information in un_stack.
void CLMEditWnd::lm_txtCmdEngin::BeginEnterChar(TCHAR nChar)
{
	//first to clear reStack,before new 'cmd'.
	ClearStack(&m_reStack);
	//find last cmd,if it's "char cmd" ,try to merger information.
	//or create new cmd object.
	cmd_stack &unStack = getUnStack();
	bool bAppend = false;

	lm_txtEditor &Editor = m_pOwner->m_txtEditor;
	if(unStack.size())
	{
		cmd *pcmd =unStack.top();
		if(pcmd->getTag() == CMD_CHAR)//try to merger 'cmd'
		{
			enter_cmd *pcharcmd = 
							(enter_cmd *)pcmd;
			//need last caret index location.
			if(pcharcmd->m_endCaret == Editor.getCaretIndex())//can append it.
			{
				bAppend = true;
				pcharcmd->m_strChars += nChar;
			}
		}
	}

	if(!bAppend)
	{
		enter_cmd *pcharcmd = new enter_cmd(this);
		pcharcmd->m_strChars = nChar;
		pcharcmd->m_startCaret = Editor.getCaretIndex();
		
		pcharcmd->m_vStart1 = Editor.m_txt.lm_vStart;
		unStack.push(pcharcmd);
	}
}

void CLMEditWnd::lm_txtCmdEngin::EndEnterChar()
{
	//record end status ,for redo option.
	cmd_stack &unStack = getUnStack();

	lm_txtEditor &Editor = m_pOwner->m_txtEditor;
	if(unStack.size())
	{
		enter_cmd *pcharcmd = 
			(enter_cmd *)unStack.top();
		pcharcmd->m_endCaret = Editor.getCaretIndex();
		pcharcmd->m_vStart2 = Editor.m_txt.lm_vStart;
	}
}

//record and merger cmd information in re_stack.
void CLMEditWnd::lm_txtCmdEngin::BeginDelChar(int index,int num)
{
	//find last cmd,if it's "del cmd" ,try to merger information.
	//or create new cmd object.
	//first to clear reStack,before new 'cmd'.
	ClearStack(&m_reStack);
	//find last cmd,if it's "char cmd" ,try to merger information.
	//or create new cmd object.
	cmd_stack &unStack = getUnStack();
	bool bAppend = false;

	lm_txtEditor &Editor = m_pOwner->m_txtEditor;
	if(unStack.size())
	{
		cmd *pcmd =unStack.top();
		if(pcmd->getTag() == CMD_DEL)//try to merger 'cmd'
		{
			del_cmd *pdelcmd = 
							(del_cmd *)pcmd;
			//need last caret index location.
			if(pdelcmd->m_endCaret == Editor.getCaretIndex())//can append it.
			{
				bAppend = true;
				CString ch = Editor.getText(index,num);;
				pdelcmd->m_strChars += Editor.getText(index,num);
				 
			}
		}
	}

	if(!bAppend)
	{
		del_cmd *pdelcmd = new del_cmd(this);
		pdelcmd->m_strChars = Editor.getText(index,num);
		pdelcmd->m_delLocal = index;
		pdelcmd->m_startCaret = Editor.getCaretIndex();
		pdelcmd->m_vStart1 = Editor.m_txt.lm_vStart;
		unStack.push(pdelcmd);
	}
}

void CLMEditWnd::lm_txtCmdEngin::EndDelChar()
{
	//record end status ,for redo option.
	lm_txtCmdEngin::cmd_stack &unStack = getUnStack();
	del_cmd *pdelcmd = (del_cmd*)unStack.top();
	lm_txtEditor &Editor = m_pOwner->m_txtEditor;
	pdelcmd->m_endCaret = Editor.getCaretIndex();
	pdelcmd->m_vStart2 = Editor.m_txt.lm_vStart;
}
