/**************************************************************
	Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
	File name:		TxWarrant.h
	Author:			赵宏俊
	Date:			2007-10-29
	Version:		1.0
	Description:	
					权证业务功能类

***************************************************************/

#ifndef __TXWARRANT_H__
#define __TXWARRANT_H__

#include "..\..\Core\TxMath\CWarrantMath.h"
#include "TxBusiness.h"

#pragma once
namespace Tx
{
	namespace Business
	{

class BUSINESS_EXT TxWarrant :
	public TxBusiness
{
public:
	TxWarrant(void);
	virtual ~TxWarrant(void);

private:
	//涨幅限制比例125%
	double m_dWarRatio;
private:
	//行权方式 30301139 tinyint short
	short GetPowerStyle(SecurityQuotation* pSecurity);
public:
	//美式权证
	bool IsAmericanStyle(int iSecurityId);
	bool IsAmericanStyle(SecurityQuotation* pSecurity);
	//欧式权证
	bool IsEuropeanStyle(int iSecurityId);
	bool IsEuropeanStyle(SecurityQuotation* pSecurity);
	//百慕大
	bool IsBermudaStyle(int iSecurityId);
	bool IsBermudaStyle(SecurityQuotation* pSecurity);

	//标的证券代码 30301129 int
	//标的证券代码 30301257 int
	int GetObjectSecurityId(int iSecurityId);
	int GetObjectSecurityId(SecurityQuotation* pSecurity);

	//行权比例 30301140 decimal double
	double GetPowerRatio(int iSecurityId);
	double GetPowerRatio(SecurityQuotation* pSecurity);

	//行权价格 30301141  decimal double
	double GetPowerPrice(int iSecurityId);
	double GetPowerPrice(SecurityQuotation* pSecurity);

	//行权截止日期[权证的到期日] 30301143 int
	int GetPowerEndDate(int iSecurityId);
	int GetPowerEndDate(SecurityQuotation* pSecurity);

	//剩余期限
	int GetRemainsDays(int iSecurityId,int iDate);
	int GetRemainsDays(SecurityQuotation* pSecurity,int iDate);

	//取得涨停价格
	float	GetUpper(int iSecurityId);
	float	GetUpper(SecurityQuotation* pSecurity);
	//取得跌停价格
	float	GetLower(int iSecurityId);
	float	GetLower(SecurityQuotation* pSecurity);
	//取得溢价率
	double		GetPremiumRate( double dWPrice,		//权证价格
								double dSPrice,		//标的股票价格
								double dExPrice,	//行权价
								bool	bFlag,		//认购为true,反之为false
								double	dRatio		//行权比例
								);
	//取得溢价率
	double		GetPremiumRate(int iSecurityId);
	double		GetPremiumRate(SecurityQuotation* pSecurity);
	double		GetPremiumRate(SecurityQuotation* pSecurity,int date);
	//取得杠杆比率
	double		GetGearRate(	double	dSClose,	//相关资产的收盘价
								double	dWClose,	//权证的收盘价
								double	dExRatio	//行权比例
								);
	double		GetGearRate(int iSecurityId);
	double		GetGearRate(SecurityQuotation* pSecurity);
	double		GetGearRate(SecurityQuotation* pSecurity,int date);
	//取得隐含波动率
	double		GetSigma(	bool	bCall_Put,		//认购认沽	true认购
							double	dStockPrice,	//标的证券价格
							double	dExPrice,		//行权价格
							double	dNoRiskInterest,//无风险利率
							double	dDelta,		
							double	dLastedYear,	//剩余期限（年）
							double	dMarketValue	//期权市场价值
							);
	double		GetSigma(int iSecurityId);
	double		GetSigma(SecurityQuotation* pSecurity);
	double		GetSigma(SecurityQuotation* pSecurity,int date);

	//板块=排序分析
	//排序分析数据结果标题列
	bool GetBlockAnalysisCol(Table_Display& baTable,std::vector<int>& arrSamples,int& iSortCol);
	//排序分析数据结果设置
	bool SetBlockAnalysisCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow);
};
	}
}
#endif