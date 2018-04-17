/*************************************************
  Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
  File name:	TxDelegate.h
  Author:       赵宏俊
  Version:		1.0
  Date:			2008-03-18
  
  Description:
				委托管理类

*************************************************/
#pragma once
#include "Business.h"

#define D_NAME 33
#define D_EXE 256
namespace Tx
{
	namespace Business
	{

struct TxDelegateData{
	TCHAR name[D_NAME];	//委托名称
	TCHAR exe[D_EXE];	//委托执行程序
};
class BUSINESS_EXT TxDelegate
{
private:
	TxDelegate(void);
	void Clear(void);
public:
	~TxDelegate(void);
	static TxDelegate* GetInstance(void)
	{
		static TxDelegate _instance;
		return &_instance;
	}
private:
	//委托数据文件
	CString m_sDelegateFile;
	//默认
	int		m_iDefaultIndex;

	//委托数据
	CArray<TxDelegateData*> m_arrDelegate;
public:
	//加载委托
	bool Load(CString sFile=_T(""));
	//保存委托数据
	bool Save(void);



	//执行委托
	bool Launch(int index=INT_MIN);

	//删除指定委托
	bool Remove(int index);
	//修改指定委托
	bool Modify(int index,CString& sName,CString sExe);
	//增加委托
	bool Add(CString& sName,CString sExe);
	//取得名称
	CString GetName(int index,bool bIsName=true);
	//取得委托数量
	int GetCount(void)
	{
		return (int)m_arrDelegate.GetSize();
	}
	//取得默认委托id
	int GetDefault(void)
	{
		return m_iDefaultIndex;
	}

	//设置默认委托
	void SetDefault(int index);
};
	}
}