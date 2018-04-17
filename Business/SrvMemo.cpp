/*************************************************
  Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
  File name:	SrvMemo.cpp
  Author:       wangwei
  Version:		1.0
  Date:			2007-9-4
  
  Description:  显示具体的备忘录信息并进行业务处理: 添加     删除    修改	
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
	strFileName = Tx::Core::Manage::GetInstance()->m_pUserInfo->GetMemoPath();    //得到写入文件的根路径
	if(file.Open(strFileName + "\\index.dat",CFile::modeRead))
	{
		file.Read(&m_index,sizeof(int));        //从文件得到备忘总索引
		file.Close();
	}
	else
		m_index = 0;                           //如果未得到则为0
}

SrvMemo::~SrvMemo()                            //析够时将新的备忘总索引写入文件
{
	CFile file; 
	file.Open(strFileName + "\\index.dat",CFile::modeCreate|CFile::modeWrite);
	file.Write(&m_index,sizeof(int));
	file.Close();
}

BOOL SrvMemo::OnInitDialog()                  //初始化函数
{
	CDialog::OnInitDialog();
	SetWindowText(_T("备忘录"));
	GetDlgItem(IDC_DELMEMO)->EnableWindow(false);           //删除备忘无效
	GetDlgItem(IDC_CHANGE)->EnableWindow(false);            //修改备忘无效
	m_ListCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT);      //LISTCTRL可选中整行
	m_ListCtrl.InsertColumn(0,"序号",LVCFMT_LEFT,50);       //第一列为序号
	m_ListCtrl.InsertColumn(1,"日期",LVCFMT_LEFT,100);      //第二列为日期
	m_ListCtrl.InsertColumn(2,"代码",LVCFMT_LEFT,80);       //第三列为代码
	m_ListCtrl.InsertColumn(3,"简称",LVCFMT_LEFT,100);      //第四列为简称
	m_ListCtrl.InsertColumn(4,"类型",LVCFMT_LEFT,150);      //第五列为类型
	m_ListCtrl.InsertColumn(5,"标题",LVCFMT_LEFT,200);      //第六列为标题
	LoadMemo();                                             //加载备忘录信息
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

void SrvMemo::OnBnClickedAddmemo()                       //添加备忘
{
	// TODO: Add your control notification handler code here
	MemoItem item;
	memset(&item, NULL, sizeof(MemoItem));               //清空结构
	Memo *dlg = new Memo(&item,false,NULL,arrType);      //初始化MEMO——非修改时
	if(dlg->DoModal() == IDOK)
	{
		CString strNo;
		strNo.Format("%d",pos+1);                        //从现有行的下一行添加
		int row = m_ListCtrl.InsertItem(pos,strNo);      
		memset(&m_pItem,0,sizeof(MemoItem));            //清空m_pItem
		memcpy(&m_pItem,dlg->pItem,sizeof(MemoItem));   //获得备忘录信息
		m_pItem.Index = m_index;                        //添加文件索引值

		//数据的显示
		CString strDate;
		strDate.Format("%d-%d-%d",m_pItem.Date/10000,m_pItem.Date/100%100,m_pItem.Date%100);
		m_ListCtrl.SetItemText(row,1,strDate);         //日期显示
		CString strCode;
		if(m_pItem.Code>0)
			strCode.Format("%d",m_pItem.Code);
		else
			strCode = "-";	
		m_ListCtrl.SetItemText(row,2,strCode);        //代码显示，非指定时为“-”
		m_ListCtrl.SetItemText(row,3,m_pItem.Name);   //简称显示
		m_ListCtrl.SetItemText(row,4,m_pItem.Type);   //类型显示

		CString strType;
		strType.Format("%s",m_pItem.Type);
		AddType(strType);                             //添加备忘类型以便下次选择

		m_ListCtrl.SetItemText(row,5,m_pItem.Title);  //标题显示
		m_ListCtrl.SetItemData(row,m_pItem.Index);    //为对应的行添加索引数据信息
		CString str;
		str.Format("%d.dat",m_index);
		WriteMemo(strFileName+"\\"+str);            //写入文件

		//获得备忘录内容并写入文件
		m_content.SetWindowText(dlg->strContent);
		CStdioFile file; 
		CString strContent = strFileName + _T("\\content");    //得到写入备忘正文的文件夹路径
		try
		{
			DWORD   dwAttr = GetFileAttributes(strContent);   
			if(dwAttr == 0xFFFFFFFF)                          //文件夹不存在   
				CreateDirectory(strContent,NULL);          //创建文件夹

			file.Open(strContent+"\\"+str,CFile::modeWrite|CFile::modeCreate);   //打开文件
			file.WriteString(dlg->strContent+"\r\n");         //写文件
			file.Close();                                     //关闭文件
		}
		catch(CFileException * e)                             //错误报告
		{ 
			e->ReportError(); 
		}
		m_index++;                                            //总索引++
		pos++;                                                //当前行索引++
	}
	delete dlg;                                                 
	GetDlgItem(IDC_DELMEMO)->EnableWindow(false);
	GetDlgItem(IDC_CHANGE)->EnableWindow(false);
}

bool SrvMemo::WriteMemo(CString strPath)                        //写备忘信息  strPath——文件路径
{
	CFile file;
	if(!file.Open(strPath,CFile::modeCreate|CFile::modeWrite))   //打开文件
	{
		AfxMessageBox("打开文件失败");
		return false;
	}
	file.Flush();                                                //清空
	file.SeekToBegin();                                          //从文件头开始
	file.Write(&m_pItem,sizeof(MemoItem));                       //按结构写文件
	file.Close();
	return true;
}

bool SrvMemo::ReadMemo(CString strPath)                         //读取备忘录信息   strPath——文件路径
{
	CFile file;
	if(!file.Open(strPath,CFile::modeRead))
	{
		return false;
	}
	long len = (long)file.GetLength();                          //文件长度
	int alreadyRead = 0;                                        //已经读取的长度
	while(alreadyRead < len)
	{
		alreadyRead+=file.Read(&m_pItem,sizeof(MemoItem));	    //按结构读取文件
	}
	file.Close();
	return true;
}

void SrvMemo::OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult)   //单击LISTCTRL控件——显示备忘正文
{
	// TODO: Add your control notification handler code here
	int index = -1;
	POSITION pos = m_ListCtrl.GetFirstSelectedItemPosition();
	if (pos != NULL) 
		index=m_ListCtrl.GetNextSelectedItem(pos);              //获得选中的行的索引——index
	if(index >=0)                                               //索引有效
	{
		GetDlgItem(IDC_DELMEMO)->EnableWindow(true);            //删除有效
		GetDlgItem(IDC_CHANGE)->EnableWindow(true);             //修改有效
		int a=(int)m_ListCtrl.GetItemData(index);               //求得对应的文件索引也就是文件路径
		CString str;
		CString strContent = strFileName + _T("\\content");    //得到写入备忘正文的文件夹路径
		str.Format(strContent+"\\%d.dat",a);
		if(ReadContent(str))                                    //读取备忘正文
			m_content.SetWindowText(content);
	}
	else
	{
		GetDlgItem(IDC_DELMEMO)->EnableWindow(false);
		GetDlgItem(IDC_CHANGE)->EnableWindow(false);
		m_content.SetWindowText(NULL);
	}
}

bool SrvMemo::ReadContent(CString strPath)                    //读取备忘录正文
{
	CStdioFile file; 
/////////////////////////////////
//注意
	CString str;
	content.Empty();                                          //清空正文字符串
	if(file.Open(strPath,CFile::modeRead))
	{
		while(file.ReadString(str))                           //一次读取一行，需要循环读取
			content+=(str+"\r\n");    
		file.Close(); 
		return true;
	}
	else 
		return false;
/////////////////////////////////
}

void SrvMemo::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)        //双击LISTCTRL控件——修改备忘
{    
	// TODO: Add your control notification handler code here
	OnBnClickedChange();
}

void SrvMemo::OnBnClickedDelmemo()                   //删除备忘
{
	// TODO: Add your control notification handler code here
	int index = -1;
	POSITION pos = m_ListCtrl.GetFirstSelectedItemPosition();
	if (pos != NULL) 
		index=m_ListCtrl.GetNextSelectedItem(pos);             //获得选中的行的索引——index
	if(index>=0)
	{
		if (AfxMessageBox(_T("确定要删除 ?"), MB_YESNO) != IDYES)
			return;
		int a=(int)m_ListCtrl.GetItemData(index);              //求得对应的文件索引也就是文件路径
		m_ListCtrl.DeleteAllItems();                           //清空LISTCTRL
		m_content.SetWindowText(NULL);
		CString strIndex;
		strIndex.Format("%d",a);
		CString str;
		str.Format(strFileName+"\\content\\%d.dat",a);              
		DeleteFile(strFileName+"\\"+strIndex+".dat");                    //删除备忘录信息文件
		DeleteFile(str);                                       //删除备忘录正文文件
		GetDlgItem(IDC_DELMEMO)->EnableWindow(false);        
		GetDlgItem(IDC_CHANGE)->EnableWindow(false);
		LoadMemo();                                            //进行备忘录从新加载
	}
}

void SrvMemo::OnBnClickedChange()                              //修改备忘录
{
	// TODO: Add your control notification handler code here
	int index = -1;
	POSITION pos = m_ListCtrl.GetFirstSelectedItemPosition();
	if (pos != NULL) 
		index=m_ListCtrl.GetNextSelectedItem(pos);            //获得选中的行的索引——index
	if(index>=0)
	{
		int a=(int)m_ListCtrl.GetItemData(index);             //求得对应的文件索引也就是文件路径
		CString str;
		str.Format(strFileName+"\\%d.dat",a);

		if(ReadMemo(str))                                     //备忘文件读取成功
		{
			str.Format(strFileName+"\\content\\%d.dat",a);
			if(ReadContent(str))                              //正文文件读取成功
			{
				MemoItem item;
				memset(&item,0,sizeof(MemoItem));
				memcpy(&item, &m_pItem, sizeof(MemoItem));    
				Memo *dlg = new Memo(&item,true,content,arrType);    //将修改的信息发给MEMO对话框
				INT ret = (INT)dlg->DoModal();
				if(ret == IDOK)
				{
					m_pItem = *dlg->pItem;                          //确定后，获得修改后信息
					CString strDate;
					strDate.Format("%d-%d-%d",m_pItem.Date/10000,m_pItem.Date/100%100,m_pItem.Date%100);
					m_ListCtrl.SetItemText(index,1,strDate);        //重新显示日期
					CString strCode;
					if(m_pItem.Code>0)
						strCode.Format("%d",m_pItem.Code);
					else
						strCode = "-";	                           
					m_ListCtrl.SetItemText(index,2,strCode);       //显示代码
					m_ListCtrl.SetItemText(index,3,m_pItem.Name);  //显示简称
					m_ListCtrl.SetItemText(index,4,m_pItem.Type);  //显示类型

					CString strType;
					strType.Format("%s",m_pItem.Type);
					AddType(strType);                              //将类型添加到备忘类型数组

					m_ListCtrl.SetItemText(index,5,m_pItem.Title); //显示标题
					m_ListCtrl.SetItemData(index,m_pItem.Index);   //为对应的行添加索引数据信息 
					CString strIndex;
					strIndex.Format("%d",a);
					WriteMemo(strFileName+"\\"+strIndex+".dat");             //写备忘文件
					
					m_content.SetWindowText(dlg->strContent);      //显示备忘正文
					CStdioFile file;                               //写正文信息
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

void SrvMemo::LoadMemo()             //加载备忘录信息
{
	pos = 0;                        //行初始为0
	arrType.RemoveAll();            //清空类型字符串数组
	for(int i = 0 ; i<m_index;i++)  //根据总索引进行加载
	{
		CString str;
		str.Format(strFileName+"\\%d.dat",i);
		if(ReadMemo(str))           //如果文件存在（未被删除）
		{
			CString strNo;
			strNo.Format("%d",pos+1);
			int row = m_ListCtrl.InsertItem(pos,strNo);        //添加行
			CString strDate;
			strDate.Format("%d-%d-%d",m_pItem.Date/10000,m_pItem.Date/100%100,m_pItem.Date%100);
			m_ListCtrl.SetItemText(row,1,strDate);             //显示日期
			CString strCode;               
			if(m_pItem.Code>0)
				strCode.Format("%d",m_pItem.Code);
			else
				strCode = "-";	
			m_ListCtrl.SetItemText(row,2,strCode);            //显示代码
			m_ListCtrl.SetItemText(row,3,m_pItem.Name);       //显示简称
			m_ListCtrl.SetItemText(row,4,m_pItem.Type);       //显示类型
			CString strType;
			strType.Format("%s",m_pItem.Type);
			AddType(strType);                                 //将类型进行添加

			m_ListCtrl.SetItemText(row,5,m_pItem.Title);      //显示标题
			m_ListCtrl.SetItemData(row,m_pItem.Index);        //为对应的行添加索引数据信息 
			pos++;                                            //行索引加一
		}
	}
}

void SrvMemo::AddType(CString str)          //添加已填的类型
{
	if(!str.IsEmpty())                      //待添加类型不为空
	{
		for(int i =0;i < arrType.GetSize();i++)        //查看待添加的是否已在备忘类型数组中，如果已在则不添加，否则添加      
		{
			if(str == arrType.GetAt(i))      
				return;
		}
		arrType.Add(str);
	}
}