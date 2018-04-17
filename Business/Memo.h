/*************************************************
  Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
  File name:	Memo.h
  Author:       wangwei
  Version:		1.0
  Date:			2007-9-4
  
  Description:  ����¼������޸ĶԻ���
			
*************************************************/

#pragma once


// Memo dialog
#include "SrvMemo.h"

class Memo : public CDialog
{
	DECLARE_DYNAMIC(Memo)

public:
	Memo(SrvMemo::MemoItem* Item,bool Change,CString Content,
		CStringArray& arr,CWnd* pParent = NULL);        //���캯������SRVMEMO�����ʼ������
	virtual ~Memo();

// Dialog Data
	enum { IDD = IDD_MEMODLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();          //��ʼ������

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedRadio1();     //ָ��������ť
public:
	afx_msg void OnBnClickedRadio2();     //����ָ����ť
public:
	afx_msg void OnBnClickedOk();         //ȷ����ť
public:
	CButton m_button1;         
	CButton m_button2;
public:
	bool m_bSample;        //�Ƿ�ָ��֤ȯ
	bool bOpenKBW;         //ȷ���Ƿ���Դ򿪼��̾���
	CString strContent;      //��������
	SrvMemo::MemoItem* pItem;       //MEMO�ṹ
	bool b_Change;       //�Ƿ�Ϊ�����޸�
	CStringArray m_arrType;           //�洢�������͵�����
	afx_msg LRESULT OnSecuritySelOk(WPARAM wp, LPARAM lp);      //���ü��̾������
	int m_ID;            //֤ȯ����
	CString strName;     //֤ȯ���
public:
	CDateTimeCtrl m_Date;         //���ڿؼ�
public:
	CComboBox m_Combo;            //���Ϳؼ�   
public:
	CEdit m_Title;                //����ؼ�
public:
	CEdit m_Msg;                  //�������Ŀؼ�
public:
	afx_msg void OnEnChangeCode();          //ָ��֤ȯ
};
