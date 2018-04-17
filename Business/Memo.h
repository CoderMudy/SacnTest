/*************************************************
  Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
  File name:	Memo.h
  Author:       wangwei
  Version:		1.0
  Date:			2007-9-4
  
  Description:  备忘录添加与修改对话框
			
*************************************************/

#pragma once


// Memo dialog
#include "SrvMemo.h"

class Memo : public CDialog
{
	DECLARE_DYNAMIC(Memo)

public:
	Memo(SrvMemo::MemoItem* Item,bool Change,CString Content,
		CStringArray& arr,CWnd* pParent = NULL);        //构造函数，由SRVMEMO传入初始化参数
	virtual ~Memo();

// Dialog Data
	enum { IDD = IDD_MEMODLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();          //初始化函数

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedRadio1();     //指定样本按钮
public:
	afx_msg void OnBnClickedRadio2();     //无须指定按钮
public:
	afx_msg void OnBnClickedOk();         //确定按钮
public:
	CButton m_button1;         
	CButton m_button2;
public:
	bool m_bSample;        //是否指定证券
	bool bOpenKBW;         //确定是否可以打开键盘精灵
	CString strContent;      //备忘正文
	SrvMemo::MemoItem* pItem;       //MEMO结构
	bool b_Change;       //是否为备忘修改
	CStringArray m_arrType;           //存储备忘类型的数组
	afx_msg LRESULT OnSecuritySelOk(WPARAM wp, LPARAM lp);      //利用键盘精灵添加
	int m_ID;            //证券代码
	CString strName;     //证券简称
public:
	CDateTimeCtrl m_Date;         //日期控件
public:
	CComboBox m_Combo;            //类型控件   
public:
	CEdit m_Title;                //标题控件
public:
	CEdit m_Msg;                  //备忘正文控件
public:
	afx_msg void OnEnChangeCode();          //指定证券
};
