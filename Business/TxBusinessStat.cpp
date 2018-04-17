/**************************************************************
	Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
	File name:		TxBusinessStat.cpp
	Author:			�Ժ꿡
	Date:			2007-07-03
	Version:		1.0
	Description:	
					���д������ͳ��ҵ��
***************************************************************/
#include "StdAfx.h"
#include "TxBusiness.h"
#include "..\..\Core\Driver\TxFileUpdateEngine.h"

namespace Tx
{
	namespace Business
	{

//ͳ��׼��=�������
bool TxBusiness::AddSampleBeforeStat(
	int&	iCol,
	int		iStartDate,
	int		iEndDate,
	std::set<int>& iSecurity
	)
{
	//Ĭ�ϵķ���ֵ״̬
	bool result = false;

	//step1-SetSecurityIntoTable
	//���û�ѡ��Ľ���ʵ����뼯����Ϊ�������õ�table
	//�ֲ�����1.1-���ý���ʵ��col=0
	result = m_pLogicalBusiness->SetSecurityIntoTable(
		iCol,		//����������ֵ
		iSecurity,	//����ʵ�弯��
		m_txTable	//������Ҫ�Ĳ������������Լ��������������
		);
	if(result==false)
		return false;
	//ͬʱ��ӻ�����ȯ��id
	//1.2;col=1,col=2
	result = m_pLogicalBusiness->SetSecurityIntoTable(iCol,m_txTable);
	if(result==false)
		return false;

	//step2-SetVariantIntoTable
	//���õ�������,����������
	int count = 1;	//��������
	//��ʼ����col=3
	result = m_pLogicalBusiness->SetVariantIntoTable(
		iCol,									//����������ֵ
		&(Tx::Core::VariantData(iStartDate)),	//VariantData����
		count,									//��������
		m_txTable	//������Ҫ�Ĳ������������Լ��������������
		);
	if(result==false)
		return false;
	////��ֹ����col=4
	//result = m_pLogicalBusiness->SetVariantIntoTable(
	//	iCol,								//����������ֵ
	//	&(Tx::Core::VariantData(iEndDate)),	//VariantData����
	//	count,								//��������
	//	m_txTable	//������Ҫ�Ĳ������������Լ��������������
	//	);
	//if(result==false)
	//	return false;
	return true;
}

//ͳ��׼��=���ָ��
bool TxBusiness::AddIndicatorBeforeStat(
	int		iIndicator,
	int		iStartDateIndex,
	int		iEndDateIndex
	)
{
	//step3-SetIndicatorIntoTable
	//�����Ѿ����õĲ����У�����ָ�����������ָ����Ϣ�����÷�����������
	//������ָ��
	//int iIndicator = 30300014;	//ָ��=��ͨ��
	UINT varCfg[3];			//��������
	int varCount=2;			//��������

	GetIndicatorDataNow(iIndicator);
	if(m_pIndicatorData==NULL)
		return false;

	//����id
	//varCfg[0]=m_pIndicatorData->Belong2Entity;
	varCfg[0]=0;
	//
	varCfg[1]=iStartDateIndex;
	//varCfg[2]=iEndDateIndex;
	bool result = m_pLogicalBusiness->SetIndicatorIntoTable(
		m_pIndicatorData,	//ָ��
		varCfg,				//��������
		varCount,			//��������
		m_txTable	//������Ҫ�Ĳ������������Լ��������������
		);
	if(result==false)
		return false;
	return true;
}
	}
}