/**************************************************************
	Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
	File name:		TxIndex.h
	Author:			�Ժ꿡
	Date:			2007-10-29
	Version:		1.0
	Description:	
					ָ��ҵ������

***************************************************************/
#ifndef __TXINDEX_H__
#define __TXINDEX_H__

#include "TxBusiness.h"
#include "..\..\Data\HotBlockInfo.h"

#pragma once
namespace Tx
{
	namespace Business
	{
class BUSINESS_EXT TxIndex :
	public TxBusiness
{
public:
	TxIndex(void);
	virtual ~TxIndex(void);

public:
	//�����շ���[�߼�]
	bool BlockRiskIndicatorAdv(
		int iMenuID,						//����ID
		Tx::Core::Table_Indicator& resTable,//������ݱ�
		std::set<int>& lSecurityId,			//����ʵ��ID
		int iEndDate,						//��ֹ����
		long lRefoSecurityId,				//��׼����ʵ��ID
		int iStartDate=0,					//��ʼ����
		int iTradeDaysCount=20,				//��������
		int iDuration=0,					//��������0=�գ�1=�ܣ�2=�£�3=����4=��
		bool bLogRatioFlag=false,			//������ʵķ�ʽ,true=ָ������,false=�򵥱���
		bool bClosePrice=true,				//ʹ�����̼�
		int  iStdType=0,					//��׼������1=��ƫ��������ƫ
		int  iOffset=0,						//ƫ��
		bool bAhead=true,					//��Ȩ��־,true=ǰ��Ȩ,false=��Ȩ
		bool bUserRate=true,				//�û��Զ���������,����ȡһ�궨�ڻ�׼����
		double dUserRate=0.034,				//�û��Զ���������,����ȡһ�궨�ڻ�׼����
		bool bDayRate=true					//������
		);
	//ָ�������仯
	bool SampleChange(
		Tx::Core::Table_Display& resTable,//������ݱ�
		long lSecurityId	//����ʵ��ID
		);
	bool IndexSample(Tx::Core::Table_Display& resTable,
		long lSecurityId,
		int iSortCol = 5,
		bool bAscend = false
		);
	//ָ������ֵ
	enum TypeMv
	{
		typeFree,//������ͨ��ֵ
		typeTradable,//����ͨ��ֵ
		typeTotal//����ֵ
	};
	double GetIndexMV(long lSecurityId,TypeMv type=typeTradable);
	//����ETF�������������
	bool EtfSample( Tx::Core::Table_Display* pResTable,
		long lSecurityId
		);
	//ȡ�������ʮ���ز�,���������etfȡ����������
	bool GetWeightSample( int nSecurityId, std::vector< int >& nVec );
	//���=�������
	////baTable�Ѿ����������[����ʵ��]
	bool GetBlockAnalysis(Table_Display& baTable,std::vector<int>& arrSamples,int& iSortCol,int iMarketId=0,bool bNeedProgressWnd=false,int nCollID=-1);
	//bool GetBlockAnalysis(Table_Display& baTable,std::vector<int>& arrSamples,bool bNeedProgressWnd=false);

	virtual bool GetBlockAnalysisCol(Table_Display& baTable,std::vector<int>& arrSamples,int& iSortCol);

	//��������������ݽ������[������]
	bool SetBlockAnalysisHslCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow);
	//��������������ݽ������
	bool SetBlockAnalysisCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow);
	//bool SetBlockAnalysisCol(Table_Display& baTable,SecurityQuotation* psq,int& j,int ii);
	//�׶�����
	bool GetPeriodHq( Table_Display& resTable, std::set< int>& arrSample,int nStart, int nEnd );
	void ReadExIndexFile(int iSecId,Tx::Core::Table_Display& resTable);
	//����ʼ��ֹ���ڵı䶯�ļ�=====added by zhangxs 20100607
	void ReadExIndexFile(int iSecId,Tx::Core::Table_Display& resTable,int nStart,int nEnd);
	std::unordered_map<int,Tx::Data::TxShareDataEx> m_mapTxShareDataEx;
};
	}
}
#endif