/********************************************************************
	FileName:	FundDeriveData.h
	Created:	2007/09/25
	Project:	Business
	Author:		xum
	Version:	v1.0

	Purpose:	use to calculate the derived data of fund 
	
	History:	

*********************************************************************
	Copyright (C) 2007 - All Rights Reserved
*********************************************************************/

#pragma once

#include "TxBusiness.h"

namespace Tx
{
	namespace Business
	{

class BUSINESS_EXT FundDeriveData : 
			public TxBusiness
{
public:
	FundDeriveData(void);
	~FundDeriveData(void);

public:
	//计算一组基金，每个间隔时间的净值增长率的加权值
	bool CalcWeightedFundNvr (std::vector<double> & dWeighted, Tx::Core::Table& resultTable, std::vector<int> & iSecurityId, std::set< std::pair<int,int> >& date);

	//计算一组基金，某区间内，每个间隔时间的单位净值增长率
	bool CalcFundNvr (Tx::Core::Table& resultTable, std::vector<int> & iSecurityId, std::set< std::pair<int,int> >& date);

	//计算一只基金一段时间区间的净值增长率
	double CalcFundNvr_OneFund (int iSecurityId, int iBeginDate, int iEndDate, BOOL bFlag_Sryl = FALSE);

	//计算一只基金某阶段区间内，每个间隔时间的净值增长率
	bool CalcFundNvr_OneFund (std::vector<double>& dNvr, int iSecurityId, std::set<int> & dates, int iBeginDate);

	//计算多只基金在某个时间区间内，净值增长率
	bool CalcFundNvr_Funds (std::vector<double>& dNvr, std::vector<int> & iSecurityId, int iEndDate, int iBeginDate);

	//计算一组封闭式基金，某个时间序列的折溢价率
	bool CalcFundPremium (Tx::Core::Table& resultTable, std::vector<int> & iSecurityId, std::vector<int>& date);

	//计算某一组证券，某个时间序列的增长率序列
	bool CalCloseFundNav(Tx::Core::Table& resultTable, std::vector<int> & iSecurityId, std::set< std::pair<int,int> >& date,bool bNav = true,bool bSim = true);

	//计算股票某段时间内的增长序列
	bool CalStockRate( Tx::Core::Table& resultTable, std::vector<int> & iSecurityId, std::set< std::pair<int,int> >& date,bool bSim = true ); 
	//计算某一组证券，某个时间序列的增长率序列，将碎文件转换为了后台计算
	bool CalFundNav_Ext(Tx::Core::Table& resultTable, std::vector<int> & iSecurityId, std::set< std::pair<int,int> >& date,bool bNav = true,bool bSim = true);

	//计算某一组基金，某个日期的单位净值、累计净值、收盘价、折溢价率，某个时间区间内，业绩基准收益率、平均折溢价率
	bool CalFundData(Tx::Core::Table& resultTable, std::vector<int>& iSecurityId, std::vector<std::pair<int,int>>& date,int iEndDate,bool bAvg = true);


};

	}
}
