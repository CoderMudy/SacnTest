/*************************************************
  Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
  File name:	Memo.cpp
  Author:       wangwei
  Version:		1.0
  Date:			2007-9-4
  
  Description:  备忘录添加与修改对话框
			
*************************************************/

// Memo.cpp : implementation file
//

#include "stdafx.h"
#include "Memo.h"
#include "..\..\core\control\Manage.h"	// zway: 测试弹出键盘精灵
#include "..\..\core\control\KBWizard.h"

// Memo dialog

IMPLEMENT_DYNAMIC(Memo, CDialog)

Memo::Memo(SrvMemo::MemoItem* Item,bool Change,CString Content,CStringArray& arr,CWnd* pParent):CDialog(Memo::IDD, pParent)
{
	pItem = Item;                //MEMO结构
	b_Change = Change;	         //是否为修改
	strContent = Content;        //备忘正文
	m_arrType.Copy(arr);         //备忘类型
}

Memo::~Memo()
{
}

void Memo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RADIO1, m_button1);
	DDX_Control(pDX, IDC_RADIO2, m_button2);
	DDX_Control(pDX, IDC_DATETIMEPICKER1, m_Date);
	DDX_Control(pDX, IDC_TYPE, m_Combo);
	DDX_Control(pDX, IDC_TITLE, m_Title);
	DDX_Control(pDX, IDC_CONTENT, m_Msg);
}


BEGIN_MESSAGE_MAP(Memo, CDialog)
	ON_BN_CLICKED(IDC_RADIO1, &Memo::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &Memo::OnBnClickedRadio2)
	ON_BN_CLICKED(IDOK, &Memo::OnBnClickedOk)
	ON_EN_CHANGE(IDC_CODE, &Memo::OnEnChangeCode)
	ON_MESSAGE(WM_USER + 77, OnSecuritySelOk)
END_MESSAGE_MAP()


// Memo message handlers

BOOL Memo::OnInitDialog()
{
	CDialog::OnInitDialog();
	if(b_Change)
		SetWindowText("修改备忘");
	else
		SetWindowText("添加备忘");

	if(!m_arrType.IsEmpty())          //备忘类型不为空则将备忘类型填入控件
	{
		for(int i =0;i< m_arrType.GetSize();i++)
		{
			m_Combo.AddString(m_arrType.GetAt(i));
		}
	}

	m_ID = -1;               //初始化证券代码
	if(b_Change)             //为备忘修改
	{
		m_bSample = pItem->b_Sample;              //是否为指定
		CTime time(pItem->Date/10000,pItem->Date/100%100,pItem->Date%100,0,0,0);
		m_Date.SetTime(&time);                    //日期初始化
		CString str1,str2,str3;
		str1.Format("%s",pItem->Title);           //标题初始化
		m_Title.SetWindowText(str1);
		str2.Format("%s",pItem->Type);            //类型初始化
		m_Combo.SetWindowText(str2);
		str3.Format("%s",pItem->Name);            //简称初始化
		GetDlgItem(IDC_NAME)->SetWindowText(str3);
		m_Msg.SetWindowText(strContent);          //备忘正文初始化
		m_ID = pItem->Code;                       //得到证券代码
	}
	else
		m_bSample = true;

	//以下根据m_bSample标志对控件进行相应操作
	m_button1.SetCheck(m_bSample);
	m_button2.SetCheck(!m_bSample);
	GetDlgItem(IDC_NAME)->EnableWindow(m_bSample);
	GetDlgItem(IDC_CODE)->EnableWindow(m_bSample);

	bOpenKBW=true;             //打开键盘精灵
	return TRUE;               // 除非将焦点设置到控件，否则返回 TRUE
}
void Memo::OnBnClickedRadio1()             //指定样本按钮选中
{
	// TODO: Add your control notification handler code here
	m_bSample = true;
	GetDlgItem(IDC_NAME)->EnableWindow(true);
	GetDlgItem(IDC_CODE)->EnableWindow(true);
}

void Memo::OnBnClickedRadio2()            //无须指定按钮选中
{
	// TODO: Add your control notification handler code here
	m_bSample = false;
	GetDlgItem(IDC_NAME)->EnableWindow(false);
	GetDlgItem(IDC_CODE)->EnableWindow(false);
}

void Memo::OnBnClickedOk()               //确定按钮选中
{
	// TODO: Add your control notification handler code here
	if(m_button1.GetCheck() == 1 && m_ID < 0)        //选择了指定但未指定证券
	{
		AfxMessageBox("请指定证券");
		return;
	}
	if(m_button2.GetCheck() == 1)          //选择了无须指定
	{
		m_ID = -1;
		strName = (_T(""));
	}

	COleDateTime time;
	m_Date.GetTime(time);
	int m_time = time.GetYear()*10000+time.GetMonth()*100+time.GetDay();          //从控件得到日期

	CString strType;
	m_Combo.GetWindowText(strType);          //从控件得到类型

	CString strTitle;
	m_Title.GetWindowText(strTitle);         //从控件得到标题
	GetDlgItem(IDC_NAME)->GetWindowText(strName);         //在关联证券中添加证券简称
	
	m_Msg.GetWindowText(strContent);         //从控件得到备忘正文

	pItem->b_Sample = m_bSample;             //改变结构内容——指定标识
	pItem->Date = m_time;                    //日期
	pItem->Code = m_ID;                      //代码
	strcpy_s(pItem->Name,strName);             //简称
	strcpy_s(pItem->Type,strType);             //类型
	strcpy_s(pItem->Title,strTitle);           //标题
                  
	OnOK();
}

void Memo::OnEnChangeCode()                  //指定证券
{
	// TODO:  Add your control notification handler code here
	if (!bOpenKBW) return;
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_CODE);          //相关EDIT控件
	CString str;
	if (pEdit && pEdit->GetSafeHwnd())
	{
		CString str;
		pEdit->GetWindowText(str);           //取得控件字符串
		CRect rect;
		pEdit->GetWindowRect(rect);
		Tx::Core::KBWizardPopWnd::Show(this, rect, Tx::Core::Manage::GetInstance()->m_pStockKbw, str);   //显示键盘精灵
	}
}

LRESULT Memo::OnSecuritySelOk(WPARAM wp, LPARAM lp)
{
	if ( wp > 0)
	{
		// 这个 wp 是选择的 SecurityId
		CString strTemp =_T("");
		Security* pSecurity= (Security*)GetSecurity(wp);           //获得pSecurity
		m_ID = _ttoi(pSecurity->GetCode());            //得到对应代码
		strName = pSecurity->GetName();       //得到对应简称
		GetDlgItem(IDC_NAME)->SetWindowText(strName);         //在关联证券中添加证券简称
	}
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_CODE);
	bOpenKBW = false;
	if (pEdit && pEdit->GetSafeHwnd())
	{
		pEdit->SetWindowText(_T(""));
	}
	bOpenKBW = true;
	
	return 1;
}