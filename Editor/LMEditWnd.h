#pragma once

//
//zxy designed for lm(little mi) 
//2012.10.24 20:43
// CLMEditWnd
//

//
//These codes,only for lm to verify my abillity ,and i dont allow it is used in any project. 
//
#include <string>
#include <stack>

class CLMEditWnd : public CWnd
{
	DECLARE_DYNAMIC(CLMEditWnd)
	//
	//implement "insert" and "del" command object here. 
	//three months ago ,i implement 'cmd' system for "magicview controller" ,about eight thousant
	//codes,so very hate 'cmd'.
	//
	#define CMD_CHAR 1
	#define CMD_DEL 2
	class lm_txtEditor;
	class lm_txtCmdEngin 
	{
	public:
		class cmd
		{
		public:
			virtual ~cmd(){};
			virtual bool reexec() = 0;
			virtual bool unexec() = 0;
			int getTag(){return m_nTag;};
	
		protected:
			lm_txtCmdEngin *m_pEngin;
			int m_nTag;
		};

		class enter_cmd :public cmd
		{
		public: 
			enter_cmd(lm_txtCmdEngin *p){
				m_pEngin = p;
				m_nTag = CMD_CHAR;
			    m_vStart1 = -1;
			    m_vStart2 = -1;
				m_startCaret=-1;
			    m_endCaret=-1;
			};
		   ~enter_cmd(){};
		
		public:
			bool  reexec();
			bool  unexec();
		public:
			int m_startCaret;
			int m_endCaret;
			int m_vStart1;
			int m_vStart2;
			CString m_strChars;
		};

		class del_cmd :public cmd
		{
		public:
			del_cmd(lm_txtCmdEngin *p){
				m_pEngin = p;
				m_nTag = CMD_DEL;
				m_vStart1 = -1;
			    m_vStart2 = -1;
				m_startCaret=-1;
			    m_endCaret=-1;
			};
		   ~del_cmd(){};
		public:
			bool  reexec();
			bool  unexec();
		public:
			int m_startCaret;
			int m_endCaret;
			int m_delLocal;
			int m_vStart1;
			int m_vStart2;
			CString m_strChars;
		};

		friend class entered_cmd;
		friend class del_cmd;
		typedef std::stack<cmd *>  cmd_stack;

	public:
		lm_txtCmdEngin(){};
		~lm_txtCmdEngin();
		void bind(CLMEditWnd *pOwner = NULL)
		{
			assert(pOwner != NULL);
			m_pOwner = pOwner;
		}
		cmd_stack &getUnStack(){return m_unStack;};
	    cmd_stack &getReStack(){return m_reStack;};
		CLMEditWnd *getOwnerWnd(){return m_pOwner;};
		lm_txtEditor *getEditor(){return &m_pOwner->m_txtEditor;};
		void ClearStack(cmd_stack *pstack);
		//command engin helper functions.
		void BeginEnterChar(char nchar);
		void EndEnterChar();

		void BeginDelChar(int index,int num);
		void EndDelChar();
	public:
		bool reexec();
		bool unexec();
		bool canreexec();
		bool canunexec();
	private:
		cmd_stack  m_unStack; //undo cmd stack
		cmd_stack  m_reStack; //redo cmd stack
		CLMEditWnd *m_pOwner;
	};

	friend class lm_txtCmdEngin::enter_cmd;
	friend class lm_txtCmdEngin::del_cmd;

	///////////////////////////////////////////////////////////////
	typedef struct lm_txtStruction
	{
	 int			lm_vStart; //visible start index of char in string buffer.
	 int			lm_caretPoint;  //caret point index for enter char
	 int			lm_dragOver;   // drag over point index.
	 CString        lm_txtBuffer;   //hold chars.
	 lm_txtStruction()
	 {
		lm_vStart = 0;
		lm_caretPoint = 0;
		lm_dragOver = -1;//-1,is a new drag option.min is zero,and max is text buffer length.
	 };
	}LM_TXTSTRUCTION;

	class lm_txtEditor
	{
		friend class CLMEditWnd;
	public:
		lm_txtEditor(){};
		~lm_txtEditor(){};
	protected:
		void resetDrag(){m_txt.lm_dragOver = -1;};
		int  setCaretIndex(int i);
		int  getCaretIndex();
		void setDragPosIndex(int index); //set drag over index
		void scrollLeft();  //called move left one char.
		void scrollRight(); //called move right one char.
		bool enterChar(char ch,bool bTrace = true);//put char to caret location 

		bool hasSelection();//some chars in drag option range
		const char *getSelection(int &iStart,int &iEnd);//get selection range
        bool delPreChar(bool bTrace = true);
		bool delSelection(bool bTrace = true);
		bool selectAll(); //select all char
		bool unselectAll();//unselect all char
		CString getText(int index,int num);
		bool MoveCaret(int tag);
		int  GetTxtLen(){return m_txt.lm_txtBuffer.GetLength();};
		const char *GetVisibleString();
		//map index to local position.
		int mapPointToIndex(POINT pos);
	    POINT getIndexPos(int i); // index to client pos.

		//suport command engin
		void setLmWnd(CLMEditWnd *pLmEdit){m_CmdEngin.bind(pLmEdit);};
	protected:
		LM_TXTSTRUCTION  m_txt;
		lm_txtCmdEngin   m_CmdEngin;
		
		 
	};
 
	 
protected:
	lm_txtEditor   m_txtEditor;
	bool		   m_bcaret;
	HACCEL		   m_cmdAccel;  
	bool			m_bDrag;
public:
	CLMEditWnd();
	virtual ~CLMEditWnd();
public:
    CString GetText(){return m_txtEditor.m_txt.lm_txtBuffer;};
protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	
	CPoint GetCaretPos();
	void  AdjustCaret();
	bool  IsVisibleIndex(int i);
    void  InputChar(char nChar);
    void  OnBackSpace();
public:
    BOOL CreateEdit(LPCTSTR lpStrName,CRect rect,CWnd *pOwner=NULL,DWORD dwID=NULL);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg UINT OnGetDlgCode();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnCopy();
    afx_msg void OnParst();
	afx_msg void OnUndo();
	afx_msg void OnRedo();
    afx_msg void OnUpdateCopy(CCmdUI *pIt);
    afx_msg void OnUpdateParst(CCmdUI *pIt);
};


