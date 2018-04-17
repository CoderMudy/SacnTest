/**************************************************************
	Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
	File name:		ExeDelegate.h
	Author:			�Ժ꿡
	Date:			2008-03-26
	Version:		1.0
	Description:	ί�д���
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
	// ���ýӿ�
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
	// �ҵĲ˵��б�
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
		// ȷ���Ƿ��ڿ��϶���Χ��
		virtual bool _PtInDrag(CPoint point, int nIndex) const;
	};
	*/

	// ��ʾ����
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
	//ί���б�
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

	//ֱ���������ͨ�������ť�õ��Ŀ�ִ���ļ�ȫ·������
	//ֱ�ӻس���ͬ���
	//����ӵ�ʱ���ٵ���С���������û��޸�ί������

	//���ܰ�ť
	CButton* m_pBtnAdd;
	CButton* m_pBtnDel;
	CButton* m_pBtnMod;
	CButton* m_pBtnRun;

	//��ΪĬ��
	CButton* m_pBtnSet;
	//�˳�
	CButton* m_pBtnExt;

	PromptWnd* m_pMyMenu;
	PromptWnd* m_pPrompt;
protected:
	DECLARE_MESSAGE_MAP()
	
public:
	BOOL PreTranslateMessage(MSG* pMsg);

protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	// �ڶԻ��򴴽������(�������� Create �ӿؼ�)
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

	void OnStdicon();//���ô�ͼ����ʾ��ʽ
	void OnSmlicon();// file://����Сͼ����ʾ��ʽ
	void OnList();// file://�����б���ʾ��ʽ
	void SetListStyle(LONG lStyleNew);
	void OnReport();// file://��ϸ������ʾ��ʽ

};

class InputDelegate : public DialogWithoutResource
{
	DECLARE_DYNAMIC(InputDelegate)

public:
	// ���ýӿ�
public:
	InputDelegate();
	virtual ~InputDelegate();

protected:
	
	//ֱ���������ͨ�������ť�õ��Ŀ�ִ���ļ�ȫ·������
	//ֱ�ӻس���ͬ���
	//����ӵ�ʱ���ٵ���С���������û��޸�ί������
	EditEx* m_pEditFind;
	CStatic*	m_pEditStatic;

	EditEx* m_pName;
	CStatic*	m_pNameStatic;

	//���
	CButton* m_pBtnBrw;
	//��ΪĬ��
	CButton* m_pBtnCan;
	//�˳�
	CButton* m_pBtnExt;

protected:
	DECLARE_MESSAGE_MAP()
	
public:
	BOOL PreTranslateMessage(MSG* pMsg);

protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	// �ڶԻ��򴴽������(�������� Create �ӿؼ�)
	virtual void _CreateControls();
public:
	CString m_sExe;
	CString m_sName;
	bool	m_bOk;
};

}
}