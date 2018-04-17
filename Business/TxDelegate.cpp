/*************************************************
  Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
  File name:	TxDelegate.h
  Author:       赵宏俊
  Version:		1.0
  Date:			2008-03-18
  
  Description:
				委托管理类

*************************************************/
#include "StdAfx.h"
#include "TxDelegate.h"
#include "..\..\Core\Control\Manage.h"
namespace Tx
{
	namespace Business
	{

TxDelegate::TxDelegate(void)
{
	m_sDelegateFile = _T("");
	m_iDefaultIndex = 0;

	Load(Tx::Core::SystemPath::GetInstance()->GetSystemDataPath() + _T("\\TxDelegate.dat"));
}

TxDelegate::~TxDelegate(void)
{
	Save();
	Clear();
}
void TxDelegate::Clear(void)
{
	for(int i=0; i<m_arrDelegate.GetSize(); i++)
	{
		TxDelegateData* pData = m_arrDelegate[i];
		if(pData!=NULL)
		{
			delete pData;
			pData = NULL;
		}
	}
	m_arrDelegate.RemoveAll();
	m_iDefaultIndex = 0;
}

//加载委托
bool TxDelegate::Load(CString sFile)
{
	if(sFile.GetLength()>0)
		m_sDelegateFile = sFile;
	CFile hd;
	if(hd.Open(m_sDelegateFile,CFile::modeRead|CFile::shareDenyNone)==FALSE)
		return false;


	int iDefaultIndex = 0;
	int count = 0;
	hd.Read(&iDefaultIndex,sizeof(int));
	hd.Read(&count,sizeof(int));

	if(count>0)
	{
		Clear();
		m_iDefaultIndex = iDefaultIndex;
		for(int i=0; i<count; i++)
		{
			TxDelegateData* pData = new TxDelegateData;
			if(pData!=NULL)
			{
				hd.Read(pData,sizeof(TxDelegateData));
				m_arrDelegate.Add(pData);
			}
		}
	}
	hd.Close();
	return true;
}
bool TxDelegate::Save(void)
{
	int count = m_arrDelegate.GetSize();
	if(count<=0)
	{
		::DeleteFile(m_sDelegateFile);
		return false;
	}

	CFile hd;
	if(hd.Open(m_sDelegateFile,CFile::modeWrite|CFile::modeCreate)==FALSE)
		return false;

	hd.Write(&m_iDefaultIndex,sizeof(int));
	hd.Write(&count,sizeof(int));

	for(int i=0; i<count; i++)
	{
		TxDelegateData* pData = m_arrDelegate[i];
		if(pData!=NULL)
		{
			if(_tcslen(pData->exe)>0 && _tcslen(pData->name)>0)
				hd.Write(pData,sizeof(TxDelegateData));
		}
	}
	hd.Close();
	return true;
}


//执行委托
bool TxDelegate::Launch(int index)
{
	if(index>=0)
		m_iDefaultIndex = index;

	int count =(int)m_arrDelegate.GetSize();

	if(count<=0)
		return false;

	if(m_iDefaultIndex<0 || m_iDefaultIndex>=count)
	{
		m_iDefaultIndex = 0;
		return false;
	}

	TxDelegateData* pData = m_arrDelegate[m_iDefaultIndex];
	if(pData==NULL)
		return false;

	TRACE(_T("\n\nTxDelegate::Launch %s--%s"),pData->name,pData->exe);

	::ShellExecute(NULL,_T("open"), pData->exe, _T(""), _T(""), SW_SHOWNORMAL);
	return true;
}

//删除指定委托
bool TxDelegate::Remove(int index)
{
	int count =(int)m_arrDelegate.GetSize();
	if(count<=0 || index<0 || index>=count)
		return false;
	TxDelegateData* pData = m_arrDelegate[index];
	if(pData!=NULL)
	{

		memset(pData,0,sizeof(TxDelegateData));
		delete pData;
		pData = NULL;
	}
	m_arrDelegate.RemoveAt(index);
	return true;
}
//修改指定委托
bool TxDelegate::Modify(int index,CString& sName,CString sExe)
{
	if(sExe.GetLength()>=D_EXE)
		return false;
	int count =(int)m_arrDelegate.GetSize();
	if(count<=0 || index<0 || index>=count)
		return false;
	TxDelegateData* pData = m_arrDelegate[index];
	if(pData==NULL)
		return false;
	_tcscpy_s(pData->exe,sExe.GetLength(),sExe);
	sName = sName.Left(D_NAME-1);
	_tcscpy_s(pData->name,sName.GetLength(),sName);
	return true;
}
bool TxDelegate::Add(CString& sName,CString sExe)
{
	if(sExe.GetLength()>=D_EXE)
		return false;

	TxDelegateData* pData = new TxDelegateData;
	if(pData==NULL)
		return false;

	_tcscpy_s(pData->exe,sExe);
	sName = sName.Left(D_NAME-1);
	_tcscpy_s(pData->name,sName);
	m_arrDelegate.Add(pData);
	return true;
}
//取得名称
CString TxDelegate::GetName(int index,bool bIsName)
{
	int count =(int)m_arrDelegate.GetSize();
	if(count<=0 || index<0 || index>=count)
		return _T("");
	TxDelegateData* pData = m_arrDelegate[index];
	if(pData==NULL)
		return _T("");
	if(bIsName==true)
		return pData->name;
	else
		return pData->exe;
}

//设置默认委托
void TxDelegate::SetDefault(int index)
{
	int count =(int)m_arrDelegate.GetSize();
	if(count<=0 || index<0 || index>=count)
		return ;
	m_iDefaultIndex = index;
}
}
}