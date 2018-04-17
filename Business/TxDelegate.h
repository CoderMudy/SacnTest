/*************************************************
  Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
  File name:	TxDelegate.h
  Author:       �Ժ꿡
  Version:		1.0
  Date:			2008-03-18
  
  Description:
				ί�й�����

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
	TCHAR name[D_NAME];	//ί������
	TCHAR exe[D_EXE];	//ί��ִ�г���
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
	//ί�������ļ�
	CString m_sDelegateFile;
	//Ĭ��
	int		m_iDefaultIndex;

	//ί������
	CArray<TxDelegateData*> m_arrDelegate;
public:
	//����ί��
	bool Load(CString sFile=_T(""));
	//����ί������
	bool Save(void);



	//ִ��ί��
	bool Launch(int index=INT_MIN);

	//ɾ��ָ��ί��
	bool Remove(int index);
	//�޸�ָ��ί��
	bool Modify(int index,CString& sName,CString sExe);
	//����ί��
	bool Add(CString& sName,CString sExe);
	//ȡ������
	CString GetName(int index,bool bIsName=true);
	//ȡ��ί������
	int GetCount(void)
	{
		return (int)m_arrDelegate.GetSize();
	}
	//ȡ��Ĭ��ί��id
	int GetDefault(void)
	{
		return m_iDefaultIndex;
	}

	//����Ĭ��ί��
	void SetDefault(int index);
};
	}
}