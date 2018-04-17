/**************************************************************
	Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
	File name:		ExeDelegate.h
	Author:			赵宏俊
	Date:			2008-03-26
	Version:		1.0
	Description:	委托窗口
***************************************************************/
#pragma once

#include "..\..\ui\framebase\DialogWithoutResource.h"
#include "..\..\ui\framebase\draglistbox.h"
#include "..\..\ui\framebase\editex.h"
#include "..\..\ui\framebase\BehaviorDrag.h"
#include "TxDelegate.h"
namespace Tx
{
	namespace Business
	{

class TxDelegateList : public MultiColListBoxBase
{
	DECLARE_DYNAMIC(TxDelegateList)

public:
	TxDelegateList();
	virtual ~TxDelegateList();

public:
	virtual void PostRefresh();
	virtual void Refresh();
	int GetHeadHeight(void) { return m_nHeaderHeight; }

protected:
	DECLARE_MESSAGE_MAP()
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

protected:
	virtual bool _ResetColWidth(int nWidth);
	virtual void _DrawCell(CDC* pdc, const CRect& rcCell, int nRow, int nCol, bool bSelected, bool bCurrent);
	virtual void _DrawHeaderCell(CDC* pdc, const CRect& rcCell, int nCol);

protected:
	int m_iDelegateColCount;
};


class BUSINESS_EXT ExeDelegate : public DialogWithoutResource
{
	DECLARE_DYNAMIC(ExeDelegate)

public:
	// 调用接口
	static void ExeDelegateWnd(bool bExeNow = true,CWnd* parent = NULL);
	static CString GetDefaultName(void)
	{
		if(TxDelegate::GetInstance()->GetCount()<=0)
			return _T("");
		return TxDelegate::GetInstance()->GetName(TxDelegate::GetInstance()->GetDefault());
	}
public:
	ExeDelegate();
	virtual ~ExeDelegate();

public:
	/*
	// 我的菜单列表
	class UserMenuList : public OwnerDrawListBox//DragListBox
	{
	public:
		UserMenuList()
		{
		}
		virtual ~UserMenuList()
		{
		}
	public:
		virtual bool MoveItem(int nFrom, int nTo);
	protected:
		// 确认是否在可拖动范围内
		virtual bool _PtInDrag(CPoint point, int nIndex) const;
	};
	*/

	// 提示窗口
	class PromptWnd : public CWnd
	{
	public:
		PromptWnd() : m_nMode(0), m_strPrompt(_T(""))
		{
		}
		PromptWnd(int mode, const CString& prompt) : m_nMode(mode), m_strPrompt(prompt)
		{
		}
		virtual ~PromptWnd()
		{
		}
	public:
		void SetPromptText(const CString& text);
	protected:
		virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

		void DrawPrompt(CDC* pdc, const CRect& rcClient);

		CString m_strPrompt;
		int m_nMode;
	};

protected:
	//委托列表
	//UserMenuList* m_pUserMenu;
	//OwnerDrawListBox* m_pUserMenu;
	//TxDelegateList* m_pUserMenu;
	CListCtrl* m_pUserMenu;

	//CListCtrl m_ListShow;
	CImageList m_hImageList;
	//CString m_sDelegatePath;
	//CString m_sDelegateDesc;
	BOOL m_bSmallIcon;
	bool m_bPressESC;

	//直接输入或者通过浏览按钮得到的可执行文件全路径名称
	//直接回车视同添加
	//再添加的时候再弹出小窗口允许用户修改委托名称

	//功能按钮
	CButton* m_pBtnAdd;
	CButton* m_pBtnDel;
	CButton* m_pBtnMod;
	CButton* m_pBtnRun;

	//设为默认
	CButton* m_pBtnSet;
	//退出
	CButton* m_pBtnExt;

	PromptWnd* m_pMyMenu;
	PromptWnd* m_pPrompt;
protected:
	DECLARE_MESSAGE_MAP()
	
public:
	BOOL PreTranslateMessage(MSG* pMsg);

protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	// 在对话框创建后调用(可在这里 Create 子控件)
	virtual void _CreateControls();
	//virtual void _DrawHeader(CDC* pdc, const CRect& rcHeader);

	void ResetUserMenuList();

	//2008-07-23
private:
	void ShowDelegate(void);
	int GetFileIconIndex( CString strFileName , BOOL bSmallIcon );
	int GetCurSel(void);
	void InitImageList(void);
	int AddDelegate(int i,CString ItemText,CString file);

	void OnStdicon();//设置大图标显示方式
	void OnSmlicon();// file://设置小图标显示方式
	void OnList();// file://设置列表显示方式
	void SetListStyle(LONG lStyleNew);
	void OnReport();// file://详细资料显示方式

};

class InputDelegate : public DialogWithoutResource
{
	DECLARE_DYNAMIC(InputDelegate)

public:
	// 调用接口
public:
	InputDelegate();
	virtual ~InputDelegate();

protected:
	
	//直接输入或者通过浏览按钮得到的可执行文件全路径名称
	//直接回车视同添加
	//再添加的时候再弹出小窗口允许用户修改委托名称
	EditEx* m_pEditFind;
	CStatic*	m_pEditStatic;

	EditEx* m_pName;
	CStatic*	m_pNameStatic;

	//浏览
	CButton* m_pBtnBrw;
	//设为默认
	CButton* m_pBtnCan;
	//退出
	CButton* m_pBtnExt;

protected:
	DECLARE_MESSAGE_MAP()
	
public:
	BOOL PreTranslateMessage(MSG* pMsg);

protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	// 在对话框创建后调用(可在这里 Create 子控件)
	virtual void _CreateControls();
public:
	CString m_sExe;
	CString m_sName;
	bool	m_bOk;
};

}
}