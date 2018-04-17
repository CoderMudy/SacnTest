/*************************************************
  Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
  File name:	Memo.cpp
  Author:       wangwei
  Version:		1.0
  Date:			2007-9-4
  
  Description:  ����¼������޸ĶԻ���
			
*************************************************/

// Memo.cpp : implementation file
//

#include "stdafx.h"
#include "Memo.h"
#include "..\..\core\control\Manage.h"	// zway: ���Ե������̾���
#include "..\..\core\control\KBWizard.h"

// Memo dialog

IMPLEMENT_DYNAMIC(Memo, CDialog)

Memo::Memo(SrvMemo::MemoItem* Item,bool Change,CString Content,CStringArray& arr,CWnd* pParent):CDialog(Memo::IDD, pParent)
{
	pItem = Item;                //MEMO�ṹ
	b_Change = Change;	         //�Ƿ�Ϊ�޸�
	strContent = Content;        //��������
	m_arrType.Copy(arr);         //��������
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
		SetWindowText("�޸ı���");
	else
		SetWindowText("��ӱ���");

	if(!m_arrType.IsEmpty())          //�������Ͳ�Ϊ���򽫱�����������ؼ�
	{
		for(int i =0;i< m_arrType.GetSize();i++)
		{
			m_Combo.AddString(m_arrType.GetAt(i));
		}
	}

	m_ID = -1;               //��ʼ��֤ȯ����
	if(b_Change)             //Ϊ�����޸�
	{
		m_bSample = pItem->b_Sample;              //�Ƿ�Ϊָ��
		CTime time(pItem->Date/10000,pItem->Date/100%100,pItem->Date%100,0,0,0);
		m_Date.SetTime(&time);                    //���ڳ�ʼ��
		CString str1,str2,str3;
		str1.Format("%s",pItem->Title);           //�����ʼ��
		m_Title.SetWindowText(str1);
		str2.Format("%s",pItem->Type);            //���ͳ�ʼ��
		m_Combo.SetWindowText(str2);
		str3.Format("%s",pItem->Name);            //��Ƴ�ʼ��
		GetDlgItem(IDC_NAME)->SetWindowText(str3);
		m_Msg.SetWindowText(strContent);          //�������ĳ�ʼ��
		m_ID = pItem->Code;                       //�õ�֤ȯ����
	}
	else
		m_bSample = true;

	//���¸���m_bSample��־�Կؼ�������Ӧ����
	m_button1.SetCheck(m_bSample);
	m_button2.SetCheck(!m_bSample);
	GetDlgItem(IDC_NAME)->EnableWindow(m_bSample);
	GetDlgItem(IDC_CODE)->EnableWindow(m_bSample);

	bOpenKBW=true;             //�򿪼��̾���
	return TRUE;               // ���ǽ��������õ��ؼ������򷵻� TRUE
}
void Memo::OnBnClickedRadio1()             //ָ��������ťѡ��
{
	// TODO: Add your control notification handler code here
	m_bSample = true;
	GetDlgItem(IDC_NAME)->EnableWindow(true);
	GetDlgItem(IDC_CODE)->EnableWindow(true);
}

void Memo::OnBnClickedRadio2()            //����ָ����ťѡ��
{
	// TODO: Add your control notification handler code here
	m_bSample = false;
	GetDlgItem(IDC_NAME)->EnableWindow(false);
	GetDlgItem(IDC_CODE)->EnableWindow(false);
}

void Memo::OnBnClickedOk()               //ȷ����ťѡ��
{
	// TODO: Add your control notification handler code here
	if(m_button1.GetCheck() == 1 && m_ID < 0)        //ѡ����ָ����δָ��֤ȯ
	{
		AfxMessageBox("��ָ��֤ȯ");
		return;
	}
	if(m_button2.GetCheck() == 1)          //ѡ��������ָ��
	{
		m_ID = -1;
		strName = (_T(""));
	}

	COleDateTime time;
	m_Date.GetTime(time);
	int m_time = time.GetYear()*10000+time.GetMonth()*100+time.GetDay();          //�ӿؼ��õ�����

	CString strType;
	m_Combo.GetWindowText(strType);          //�ӿؼ��õ�����

	CString strTitle;
	m_Title.GetWindowText(strTitle);         //�ӿؼ��õ�����
	GetDlgItem(IDC_NAME)->GetWindowText(strName);         //�ڹ���֤ȯ�����֤ȯ���
	
	m_Msg.GetWindowText(strContent);         //�ӿؼ��õ���������

	pItem->b_Sample = m_bSample;             //�ı�ṹ���ݡ���ָ����ʶ
	pItem->Date = m_time;                    //����
	pItem->Code = m_ID;                      //����
	strcpy_s(pItem->Name,strName);             //���
	strcpy_s(pItem->Type,strType);             //����
	strcpy_s(pItem->Title,strTitle);           //����
                  
	OnOK();
}

void Memo::OnEnChangeCode()                  //ָ��֤ȯ
{
	// TODO:  Add your control notification handler code here
	if (!bOpenKBW) return;
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_CODE);          //���EDIT�ؼ�
	CString str;
	if (pEdit && pEdit->GetSafeHwnd())
	{
		CString str;
		pEdit->GetWindowText(str);           //ȡ�ÿؼ��ַ���
		CRect rect;
		pEdit->GetWindowRect(rect);
		Tx::Core::KBWizardPopWnd::Show(this, rect, Tx::Core::Manage::GetInstance()->m_pStockKbw, str);   //��ʾ���̾���
	}
}

LRESULT Memo::OnSecuritySelOk(WPARAM wp, LPARAM lp)
{
	if ( wp > 0)
	{
		// ��� wp ��ѡ��� SecurityId
		CString strTemp =_T("");
		Security* pSecurity= (Security*)GetSecurity(wp);           //���pSecurity
		m_ID = _ttoi(pSecurity->GetCode());            //�õ���Ӧ����
		strName = pSecurity->GetName();       //�õ���Ӧ���
		GetDlgItem(IDC_NAME)->SetWindowText(strName);         //�ڹ���֤ȯ�����֤ȯ���
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