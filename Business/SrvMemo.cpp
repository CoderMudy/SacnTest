/*************************************************
  Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
  File name:	SrvMemo.cpp
  Author:       wangwei
  Version:		1.0
  Date:			2007-9-4
  
  Description:  ��ʾ����ı���¼��Ϣ������ҵ����: ���     ɾ��    �޸�	
*************************************************/

// SrvMemo.cpp : implementation file
//

#include "stdafx.h"
#include "SrvMemo.h"
#include "Memo.h"
#include "..\..\Core\Control\Manage.h"

// SrvMemo dialog

IMPLEMENT_DYNAMIC(SrvMemo, CDialog)

SrvMemo::SrvMemo(CWnd* pParent /*=NULL*/)
	: CDialog(SrvMemo::IDD, pParent)
{
	CFile file; 
	strFileName = Tx::Core::Manage::GetInstance()->m_pUserInfo->GetMemoPath();    //�õ�д���ļ��ĸ�·��
	if(file.Open(strFileName + "\\index.dat",CFile::modeRead))
	{
		file.Read(&m_index,sizeof(int));        //���ļ��õ�����������
		file.Close();
	}
	else
		m_index = 0;                           //���δ�õ���Ϊ0
}

SrvMemo::~SrvMemo()                            //����ʱ���µı���������д���ļ�
{
	CFile file; 
	file.Open(strFileName + "\\index.dat",CFile::modeCreate|CFile::modeWrite);
	file.Write(&m_index,sizeof(int));
	file.Close();
}

BOOL SrvMemo::OnInitDialog()                  //��ʼ������
{
	CDialog::OnInitDialog();
	SetWindowText(_T("����¼"));
	GetDlgItem(IDC_DELMEMO)->EnableWindow(false);           //ɾ��������Ч
	GetDlgItem(IDC_CHANGE)->EnableWindow(false);            //�޸ı�����Ч
	m_ListCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT);      //LISTCTRL��ѡ������
	m_ListCtrl.InsertColumn(0,"���",LVCFMT_LEFT,50);       //��һ��Ϊ���
	m_ListCtrl.InsertColumn(1,"����",LVCFMT_LEFT,100);      //�ڶ���Ϊ����
	m_ListCtrl.InsertColumn(2,"����",LVCFMT_LEFT,80);       //������Ϊ����
	m_ListCtrl.InsertColumn(3,"���",LVCFMT_LEFT,100);      //������Ϊ���
	m_ListCtrl.InsertColumn(4,"����",LVCFMT_LEFT,150);      //������Ϊ����
	m_ListCtrl.InsertColumn(5,"����",LVCFMT_LEFT,200);      //������Ϊ����
	LoadMemo();                                             //���ر���¼��Ϣ
	return TRUE;
}
void SrvMemo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ListCtrl);
	DDX_Control(pDX, IDC_EDIT1, m_content);
}


BEGIN_MESSAGE_MAP(SrvMemo, CDialog)
	ON_BN_CLICKED(IDC_ADDMEMO, &SrvMemo::OnBnClickedAddmemo)
	ON_NOTIFY(NM_CLICK, IDC_LIST1, &SrvMemo::OnNMClickList1)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &SrvMemo::OnNMDblclkList1)
	ON_BN_CLICKED(IDC_DELMEMO, &SrvMemo::OnBnClickedDelmemo)
	ON_BN_CLICKED(IDC_CHANGE, &SrvMemo::OnBnClickedChange)
END_MESSAGE_MAP()


// SrvMemo message handlers

void SrvMemo::OnBnClickedAddmemo()                       //��ӱ���
{
	// TODO: Add your control notification handler code here
	MemoItem item;
	memset(&item, NULL, sizeof(MemoItem));               //��սṹ
	Memo *dlg = new Memo(&item,false,NULL,arrType);      //��ʼ��MEMO�������޸�ʱ
	if(dlg->DoModal() == IDOK)
	{
		CString strNo;
		strNo.Format("%d",pos+1);                        //�������е���һ�����
		int row = m_ListCtrl.InsertItem(pos,strNo);      
		memset(&m_pItem,0,sizeof(MemoItem));            //���m_pItem
		memcpy(&m_pItem,dlg->pItem,sizeof(MemoItem));   //��ñ���¼��Ϣ
		m_pItem.Index = m_index;                        //����ļ�����ֵ

		//���ݵ���ʾ
		CString strDate;
		strDate.Format("%d-%d-%d",m_pItem.Date/10000,m_pItem.Date/100%100,m_pItem.Date%100);
		m_ListCtrl.SetItemText(row,1,strDate);         //������ʾ
		CString strCode;
		if(m_pItem.Code>0)
			strCode.Format("%d",m_pItem.Code);
		else
			strCode = "-";	
		m_ListCtrl.SetItemText(row,2,strCode);        //������ʾ����ָ��ʱΪ��-��
		m_ListCtrl.SetItemText(row,3,m_pItem.Name);   //�����ʾ
		m_ListCtrl.SetItemText(row,4,m_pItem.Type);   //������ʾ

		CString strType;
		strType.Format("%s",m_pItem.Type);
		AddType(strType);                             //��ӱ��������Ա��´�ѡ��

		m_ListCtrl.SetItemText(row,5,m_pItem.Title);  //������ʾ
		m_ListCtrl.SetItemData(row,m_pItem.Index);    //Ϊ��Ӧ�����������������Ϣ
		CString str;
		str.Format("%d.dat",m_index);
		WriteMemo(strFileName+"\\"+str);            //д���ļ�

		//��ñ���¼���ݲ�д���ļ�
		m_content.SetWindowText(dlg->strContent);
		CStdioFile file; 
		CString strContent = strFileName + _T("\\content");    //�õ�д�뱸�����ĵ��ļ���·��
		try
		{
			DWORD   dwAttr = GetFileAttributes(strContent);   
			if(dwAttr == 0xFFFFFFFF)                          //�ļ��в�����   
				CreateDirectory(strContent,NULL);          //�����ļ���

			file.Open(strContent+"\\"+str,CFile::modeWrite|CFile::modeCreate);   //���ļ�
			file.WriteString(dlg->strContent+"\r\n");         //д�ļ�
			file.Close();                                     //�ر��ļ�
		}
		catch(CFileException * e)                             //���󱨸�
		{ 
			e->ReportError(); 
		}
		m_index++;                                            //������++
		pos++;                                                //��ǰ������++
	}
	delete dlg;                                                 
	GetDlgItem(IDC_DELMEMO)->EnableWindow(false);
	GetDlgItem(IDC_CHANGE)->EnableWindow(false);
}

bool SrvMemo::WriteMemo(CString strPath)                        //д������Ϣ  strPath�����ļ�·��
{
	CFile file;
	if(!file.Open(strPath,CFile::modeCreate|CFile::modeWrite))   //���ļ�
	{
		AfxMessageBox("���ļ�ʧ��");
		return false;
	}
	file.Flush();                                                //���
	file.SeekToBegin();                                          //���ļ�ͷ��ʼ
	file.Write(&m_pItem,sizeof(MemoItem));                       //���ṹд�ļ�
	file.Close();
	return true;
}

bool SrvMemo::ReadMemo(CString strPath)                         //��ȡ����¼��Ϣ   strPath�����ļ�·��
{
	CFile file;
	if(!file.Open(strPath,CFile::modeRead))
	{
		return false;
	}
	long len = (long)file.GetLength();                          //�ļ�����
	int alreadyRead = 0;                                        //�Ѿ���ȡ�ĳ���
	while(alreadyRead < len)
	{
		alreadyRead+=file.Read(&m_pItem,sizeof(MemoItem));	    //���ṹ��ȡ�ļ�
	}
	file.Close();
	return true;
}

void SrvMemo::OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult)   //����LISTCTRL�ؼ�������ʾ��������
{
	// TODO: Add your control notification handler code here
	int index = -1;
	POSITION pos = m_ListCtrl.GetFirstSelectedItemPosition();
	if (pos != NULL) 
		index=m_ListCtrl.GetNextSelectedItem(pos);              //���ѡ�е��е���������index
	if(index >=0)                                               //������Ч
	{
		GetDlgItem(IDC_DELMEMO)->EnableWindow(true);            //ɾ����Ч
		GetDlgItem(IDC_CHANGE)->EnableWindow(true);             //�޸���Ч
		int a=(int)m_ListCtrl.GetItemData(index);               //��ö�Ӧ���ļ�����Ҳ�����ļ�·��
		CString str;
		CString strContent = strFileName + _T("\\content");    //�õ�д�뱸�����ĵ��ļ���·��
		str.Format(strContent+"\\%d.dat",a);
		if(ReadContent(str))                                    //��ȡ��������
			m_content.SetWindowText(content);
	}
	else
	{
		GetDlgItem(IDC_DELMEMO)->EnableWindow(false);
		GetDlgItem(IDC_CHANGE)->EnableWindow(false);
		m_content.SetWindowText(NULL);
	}
}

bool SrvMemo::ReadContent(CString strPath)                    //��ȡ����¼����
{
	CStdioFile file; 
/////////////////////////////////
//ע��
	CString str;
	content.Empty();                                          //��������ַ���
	if(file.Open(strPath,CFile::modeRead))
	{
		while(file.ReadString(str))                           //һ�ζ�ȡһ�У���Ҫѭ����ȡ
			content+=(str+"\r\n");    
		file.Close(); 
		return true;
	}
	else 
		return false;
/////////////////////////////////
}

void SrvMemo::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)        //˫��LISTCTRL�ؼ������޸ı���
{    
	// TODO: Add your control notification handler code here
	OnBnClickedChange();
}

void SrvMemo::OnBnClickedDelmemo()                   //ɾ������
{
	// TODO: Add your control notification handler code here
	int index = -1;
	POSITION pos = m_ListCtrl.GetFirstSelectedItemPosition();
	if (pos != NULL) 
		index=m_ListCtrl.GetNextSelectedItem(pos);             //���ѡ�е��е���������index
	if(index>=0)
	{
		if (AfxMessageBox(_T("ȷ��Ҫɾ�� ?"), MB_YESNO) != IDYES)
			return;
		int a=(int)m_ListCtrl.GetItemData(index);              //��ö�Ӧ���ļ�����Ҳ�����ļ�·��
		m_ListCtrl.DeleteAllItems();                           //���LISTCTRL
		m_content.SetWindowText(NULL);
		CString strIndex;
		strIndex.Format("%d",a);
		CString str;
		str.Format(strFileName+"\\content\\%d.dat",a);              
		DeleteFile(strFileName+"\\"+strIndex+".dat");                    //ɾ������¼��Ϣ�ļ�
		DeleteFile(str);                                       //ɾ������¼�����ļ�
		GetDlgItem(IDC_DELMEMO)->EnableWindow(false);        
		GetDlgItem(IDC_CHANGE)->EnableWindow(false);
		LoadMemo();                                            //���б���¼���¼���
	}
}

void SrvMemo::OnBnClickedChange()                              //�޸ı���¼
{
	// TODO: Add your control notification handler code here
	int index = -1;
	POSITION pos = m_ListCtrl.GetFirstSelectedItemPosition();
	if (pos != NULL) 
		index=m_ListCtrl.GetNextSelectedItem(pos);            //���ѡ�е��е���������index
	if(index>=0)
	{
		int a=(int)m_ListCtrl.GetItemData(index);             //��ö�Ӧ���ļ�����Ҳ�����ļ�·��
		CString str;
		str.Format(strFileName+"\\%d.dat",a);

		if(ReadMemo(str))                                     //�����ļ���ȡ�ɹ�
		{
			str.Format(strFileName+"\\content\\%d.dat",a);
			if(ReadContent(str))                              //�����ļ���ȡ�ɹ�
			{
				MemoItem item;
				memset(&item,0,sizeof(MemoItem));
				memcpy(&item, &m_pItem, sizeof(MemoItem));    
				Memo *dlg = new Memo(&item,true,content,arrType);    //���޸ĵ���Ϣ����MEMO�Ի���
				INT ret = (INT)dlg->DoModal();
				if(ret == IDOK)
				{
					m_pItem = *dlg->pItem;                          //ȷ���󣬻���޸ĺ���Ϣ
					CString strDate;
					strDate.Format("%d-%d-%d",m_pItem.Date/10000,m_pItem.Date/100%100,m_pItem.Date%100);
					m_ListCtrl.SetItemText(index,1,strDate);        //������ʾ����
					CString strCode;
					if(m_pItem.Code>0)
						strCode.Format("%d",m_pItem.Code);
					else
						strCode = "-";	                           
					m_ListCtrl.SetItemText(index,2,strCode);       //��ʾ����
					m_ListCtrl.SetItemText(index,3,m_pItem.Name);  //��ʾ���
					m_ListCtrl.SetItemText(index,4,m_pItem.Type);  //��ʾ����

					CString strType;
					strType.Format("%s",m_pItem.Type);
					AddType(strType);                              //��������ӵ�������������

					m_ListCtrl.SetItemText(index,5,m_pItem.Title); //��ʾ����
					m_ListCtrl.SetItemData(index,m_pItem.Index);   //Ϊ��Ӧ�����������������Ϣ 
					CString strIndex;
					strIndex.Format("%d",a);
					WriteMemo(strFileName+"\\"+strIndex+".dat");             //д�����ļ�
					
					m_content.SetWindowText(dlg->strContent);      //��ʾ��������
					CStdioFile file;                               //д������Ϣ
					try
					{
						file.Open(strFileName+"\\content\\"+strIndex+".dat",CFile::modeWrite|CFile::modeCreate); 
						file.WriteString(dlg->strContent); 
						file.Close(); 
					}
					catch(CFileException * e) 
					{ 
						e->ReportError(); 
					}

				}
				delete dlg;
			}
		}
		
	}
}

void SrvMemo::LoadMemo()             //���ر���¼��Ϣ
{
	pos = 0;                        //�г�ʼΪ0
	arrType.RemoveAll();            //��������ַ�������
	for(int i = 0 ; i<m_index;i++)  //�������������м���
	{
		CString str;
		str.Format(strFileName+"\\%d.dat",i);
		if(ReadMemo(str))           //����ļ����ڣ�δ��ɾ����
		{
			CString strNo;
			strNo.Format("%d",pos+1);
			int row = m_ListCtrl.InsertItem(pos,strNo);        //�����
			CString strDate;
			strDate.Format("%d-%d-%d",m_pItem.Date/10000,m_pItem.Date/100%100,m_pItem.Date%100);
			m_ListCtrl.SetItemText(row,1,strDate);             //��ʾ����
			CString strCode;               
			if(m_pItem.Code>0)
				strCode.Format("%d",m_pItem.Code);
			else
				strCode = "-";	
			m_ListCtrl.SetItemText(row,2,strCode);            //��ʾ����
			m_ListCtrl.SetItemText(row,3,m_pItem.Name);       //��ʾ���
			m_ListCtrl.SetItemText(row,4,m_pItem.Type);       //��ʾ����
			CString strType;
			strType.Format("%s",m_pItem.Type);
			AddType(strType);                                 //�����ͽ������

			m_ListCtrl.SetItemText(row,5,m_pItem.Title);      //��ʾ����
			m_ListCtrl.SetItemData(row,m_pItem.Index);        //Ϊ��Ӧ�����������������Ϣ 
			pos++;                                            //��������һ
		}
	}
}

void SrvMemo::AddType(CString str)          //������������
{
	if(!str.IsEmpty())                      //��������Ͳ�Ϊ��
	{
		for(int i =0;i < arrType.GetSize();i++)        //�鿴����ӵ��Ƿ����ڱ������������У������������ӣ��������      
		{
			if(str == arrType.GetAt(i))      
				return;
		}
		arrType.Add(str);
	}
}