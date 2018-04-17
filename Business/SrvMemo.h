/*************************************************
  Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
  File name:	SrvMemo.h
  Author:       wangwei
  Version:		1.0
  Date:			2007-9-4
  
  Description:  显示具体的备忘录信息并进行业务处理: 添加     删除    修改	
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
	afx_msg void OnBnClickedAddmemo();               //添加备忘
public:
	afx_msg void OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult);           //单击LISTCTRL控件——显示备忘正文
public:
	afx_msg void OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult);          //双击LISTCTRL控件——修改备忘
public:
	afx_msg void OnBnClickedDelmemo();              //删除备忘
public:
	afx_msg void OnBnClickedChange();               //修改备忘
public:
	CListCtrl m_ListCtrl;
	CEdit m_content;
public:
	virtual BOOL OnInitDialog();
	struct MemoItem           //备忘录结构
	{
		bool b_Sample ;       //是否指定证券
		int Index;            //文件索引
		int Date;             //日期
		int Code;             //代码
		char Name[50];        //简称
		char Type[100];       //类型
		char Title[100];      //标题
	};	
	MemoItem m_pItem;
	bool WriteMemo(CString strPath);     //写备忘录文件
	bool ReadMemo(CString strPath);      //读备忘录文件
	bool ReadContent(CString strPath);   //读备忘正文
	long m_index;                        //备忘录文件总索引
	int pos;                             //在LISTCTRL中行的索引
	CString content;                     //备忘正文
	void LoadMemo();                     //加载备忘录信息
	CStringArray arrType;                //备忘类型数组
	void AddType(CString str);           //添加备忘类型
	CString strFileName;                 //备忘录根路径
};
