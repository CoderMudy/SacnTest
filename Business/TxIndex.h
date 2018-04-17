/**************************************************************
	Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
	File name:		TxIndex.h
	Author:			赵宏俊
	Date:			2007-10-29
	Version:		1.0
	Description:	
					指数业务功能类

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
	//板块风险分析[高级]
	bool BlockRiskIndicatorAdv(
		int iMenuID,						//功能ID
		Tx::Core::Table_Indicator& resTable,//结果数据表
		std::set<int>& lSecurityId,			//交易实体ID
		int iEndDate,						//截止日期
		long lRefoSecurityId,				//基准交易实体ID
		int iStartDate=0,					//起始日期
		int iTradeDaysCount=20,				//交易天数
		int iDuration=0,					//交易周期0=日；1=周；2=月；3=季；4=年
		bool bLogRatioFlag=false,			//计算比率的方式,true=指数比率,false=简单比率
		bool bClosePrice=true,				//使用收盘价
		int  iStdType=0,					//标准差类型1=有偏，否则无偏
		int  iOffset=0,						//偏移
		bool bAhead=true,					//复权标志,true=前复权,false=后复权
		bool bUserRate=true,				//用户自定义年利率,否则取一年定期基准利率
		double dUserRate=0.034,				//用户自定义年利率,否则取一年定期基准利率
		bool bDayRate=true					//日利率
		);
	//指数样本变化
	bool SampleChange(
		Tx::Core::Table_Display& resTable,//结果数据表
		long lSecurityId	//交易实体ID
		);
	bool IndexSample(Tx::Core::Table_Display& resTable,
		long lSecurityId,
		int iSortCol = 5,
		bool bAscend = false
		);
	//指数的市值
	enum TypeMv
	{
		typeFree,//自由流通市值
		typeTradable,//已流通市值
		typeTotal//总市值
	};
	double GetIndexMV(long lSecurityId,TypeMv type=typeTradable);
	//计算ETF基金的样本行情
	bool EtfSample( Tx::Core::Table_Display* pResTable,
		long lSecurityId
		);
	//取出基金的十大重仓,对于特殊的etf取出所有样本
	bool GetWeightSample( int nSecurityId, std::vector< int >& nVec );
	//板块=排序分析
	////baTable已经填充了样本[交易实体]
	bool GetBlockAnalysis(Table_Display& baTable,std::vector<int>& arrSamples,int& iSortCol,int iMarketId=0,bool bNeedProgressWnd=false,int nCollID=-1);
	//bool GetBlockAnalysis(Table_Display& baTable,std::vector<int>& arrSamples,bool bNeedProgressWnd=false);

	virtual bool GetBlockAnalysisCol(Table_Display& baTable,std::vector<int>& arrSamples,int& iSortCol);

	//排序分析基本数据结果设置[换手率]
	bool SetBlockAnalysisHslCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow);
	//排序分析基本数据结果设置
	bool SetBlockAnalysisCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow);
	//bool SetBlockAnalysisCol(Table_Display& baTable,SecurityQuotation* psq,int& j,int ii);
	//阶段行情
	bool GetPeriodHq( Table_Display& resTable, std::set< int>& arrSample,int nStart, int nEnd );
	void ReadExIndexFile(int iSecId,Tx::Core::Table_Display& resTable);
	//带起始截止日期的变动文件=====added by zhangxs 20100607
	void ReadExIndexFile(int iSecId,Tx::Core::Table_Display& resTable,int nStart,int nEnd);
	std::unordered_map<int,Tx::Data::TxShareDataEx> m_mapTxShareDataEx;
};
	}
}
#endif