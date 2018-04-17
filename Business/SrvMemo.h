/*************************************************
  Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
  File name:	SrvMemo.h
  Author:       wangwei
  Version:		1.0
  Date:			2007-9-4
  
  Description:  ��ʾ����ı���¼��Ϣ������ҵ����: ���     ɾ��    �޸�	
*************************************************/
#pragma once
#include "Business.h"
// SrvMemo dialog
#include "Resource.h"
class BUSINESS_EXT SrvMemo : public CDialog
{
	DECLARE_DYNAMIC(SrvMemo)

public:
	SrvMemo(CWnd* pParent = NULL);   // standard constructor
	virtual ~SrvMemo();

// Dialog Data
	enum { IDD = IDD_SRVMEMO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedAddmemo();               //��ӱ���
public:
	afx_msg void OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult);           //����LISTCTRL�ؼ�������ʾ��������
public:
	afx_msg void OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult);          //˫��LISTCTRL�ؼ������޸ı���
public:
	afx_msg void OnBnClickedDelmemo();              //ɾ������
public:
	afx_msg void OnBnClickedChange();               //�޸ı���
public:
	CListCtrl m_ListCtrl;
	CEdit m_content;
public:
	virtual BOOL OnInitDialog();
	struct MemoItem           //����¼�ṹ
	{
		bool b_Sample ;       //�Ƿ�ָ��֤ȯ
		int Index;            //�ļ�����
		int Date;             //����
		int Code;             //����
		char Name[50];        //���
		char Type[100];       //����
		char Title[100];      //����
	};	
	MemoItem m_pItem;
	bool WriteMemo(CString strPath);     //д����¼�ļ�
	bool ReadMemo(CString strPath);      //������¼�ļ�
	bool ReadContent(CString strPath);   //����������
	long m_index;                        //����¼�ļ�������
	int pos;                             //��LISTCTRL���е�����
	CString content;                     //��������
	void LoadMemo();                     //���ر���¼��Ϣ
	CStringArray arrType;                //������������
	void AddType(CString str);           //��ӱ�������
	CString strFileName;                 //����¼��·��
};
