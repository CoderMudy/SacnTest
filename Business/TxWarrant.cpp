/**************************************************************
	Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
	File name:		TxWarrant.cpp
	Author:			赵宏俊
	Date:			2007-10-29
	Version:		1.0
	Description:	
					权证业务功能类

***************************************************************/

#include "StdAfx.h"
#include "TxWarrant.h"
#include "MyIndicator.h"
#include "ComputingWarrant.h"

namespace Tx
{
	namespace Business
	{

TxWarrant::TxWarrant(void)
{
	//构造
	m_dWarRatio = 1.25;
}

TxWarrant::~TxWarrant(void)
{
	//析构
}
//行权方式 30301139 tinyint short
short TxWarrant::GetPowerStyle(SecurityQuotation* pSecurity)
{
	if(pSecurity==NULL)
		return -1;
	WarrantBasicInfo* pWarrantBasicInfo = pSecurity->GetWarrantBasicInfo();
	if(pWarrantBasicInfo==NULL)
		return -1;
	//byte bytePowerStyle = pSecurity->GetIndicateByteValue(30301139);
	//return (short)bytePowerStyle;//pSecurity->GetIndicateShortValue(30301139);
	return (short)pWarrantBasicInfo->power_style;
}
//美式权证
bool TxWarrant::IsAmericanStyle(int iSecurityId)
{
	GetSecurityNow(iSecurityId);
	return IsAmericanStyle(m_pSecurity);
}
bool TxWarrant::IsAmericanStyle(SecurityQuotation* pSecurity)
{
	if(GetPowerStyle(pSecurity)==0)
		return true;
	return false;
}
//欧式权证
bool TxWarrant::IsEuropeanStyle(int iSecurityId)
{
	GetSecurityNow(iSecurityId);
	return IsEuropeanStyle(m_pSecurity);
}

bool TxWarrant::IsEuropeanStyle(SecurityQuotation* pSecurity)
{
	if(GetPowerStyle(pSecurity)==1)
		return true;
	return false;
}
//百慕大
bool TxWarrant::IsBermudaStyle(int iSecurityId)
{
	GetSecurityNow(iSecurityId);
	return IsBermudaStyle(m_pSecurity);
}
bool TxWarrant::IsBermudaStyle(SecurityQuotation* pSecurity)
{
	if(GetPowerStyle(pSecurity)==2)
		return true;
	return false;
}
//标的证券代码 30301129 int
int TxWarrant::GetObjectSecurityId(int iSecurityId)
{
	GetSecurityNow(iSecurityId);
	return GetObjectSecurityId(m_pSecurity);
}
int TxWarrant::GetObjectSecurityId(SecurityQuotation* pSecurity)
{
	if(pSecurity==NULL)
		return Con_intInvalid;
	WarrantBasicInfo* pWarrantBasicInfo = pSecurity->GetWarrantBasicInfo();
	if(pWarrantBasicInfo==NULL)
		return Con_intInvalid;
	//return pSecurity->GetIndicateIntValue(30301257);
	return pWarrantBasicInfo->obj_id;
}

//行权比例 30301140 decimal double
double TxWarrant::GetPowerRatio(int iSecurityId)
{
	GetSecurityNow(iSecurityId);
	return GetPowerRatio(m_pSecurity);
}
double TxWarrant::GetPowerRatio(SecurityQuotation* pSecurity)
{
	if(pSecurity==NULL)
		return Con_doubleInvalid;
	WarrantBasicInfo* pWarrantBasicInfo = pSecurity->GetWarrantBasicInfo();
	if(pWarrantBasicInfo==NULL)
		return Con_doubleInvalid;
	//return pSecurity->GetIndicateDoubleValue(30301140);
	return pWarrantBasicInfo->power_ratio;
}

//行权价格 30301141  decimal double
double TxWarrant::GetPowerPrice(int iSecurityId)
{
	GetSecurityNow(iSecurityId);
	return GetPowerPrice(m_pSecurity);
}
double TxWarrant::GetPowerPrice(SecurityQuotation* pSecurity)
{
	if(pSecurity==NULL)
		return Con_doubleInvalid;
	WarrantBasicInfo* pWarrantBasicInfo = pSecurity->GetWarrantBasicInfo();
	if(pWarrantBasicInfo==NULL)
		return Con_doubleInvalid;
	//return pSecurity->GetIndicateDoubleValue(30301141);
	return pWarrantBasicInfo->power_price;
}
//存续截止日期 30301135
//行权截止日期[权证的到期日] 30301143 int
int TxWarrant::GetPowerEndDate(int iSecurityId)
{
	GetSecurityNow(iSecurityId);
	return GetPowerEndDate(m_pSecurity);
}
int TxWarrant::GetPowerEndDate(SecurityQuotation* pSecurity)
{
	if(pSecurity==NULL)
		return Con_intInvalid;
	WarrantBasicInfo* pWarrantBasicInfo = pSecurity->GetWarrantBasicInfo();
	if(pWarrantBasicInfo==NULL)
		return Con_intInvalid;
	//return pSecurity->GetIndicateIntValue(30301143);
	return pWarrantBasicInfo->power_end_date;
}
//取得涨停价格
float	TxWarrant::GetUpper(int iSecurityId)
{
	SecurityQuotation* pSecurity = GetSecurityNow(iSecurityId);
	if(pSecurity==NULL)
		return Con_floatInvalid;
	return GetUpper(pSecurity);
}
float	TxWarrant::GetUpper(SecurityQuotation* pSecurity)
{
	//取得标的证券
	int TartgetId = GetObjectSecurityId(pSecurity->GetId());
	//行权比例
	double fPowerratio = GetPowerRatio(pSecurity->GetId());
	if(!(fPowerratio>0))
		return Con_floatInvalid;
	
	SecurityQuotation* pObjSecurity = GetSecurityNow(TartgetId);
	if(pObjSecurity==NULL)
		return Con_floatInvalid;
	//权证前收价
	float fPreClose = pSecurity->GetPreClosePrice();
	if(fPreClose<0)
		return Con_floatInvalid;
	//标的前收价
	float fTarPreClose = pObjSecurity->GetPreClosePrice();
	if(fTarPreClose<0)
		return Con_floatInvalid;
	//标的涨停价
	//2008-03-03
	float ratio = 0.1f;
	if(pObjSecurity->InSTblock()==true)
		ratio = 0.05f;
	/*
	float fUpper = pObjSecurity->GetUpper(ratio);
	if(fUpper<0)
		return Con_floatInvalid;
	int iScale = 3;
	//深交所=3
	//上交所=2
	if(pSecurity->IsShanghai()==true)
		iScale = 2;
	int ifUpper = 0;
	int iSRup = 1;
	ifUpper = GetDataByScale(fUpper,iScale,iSRup);
	int ifPtclose= 0;
	int iSRptc = 1;
	ifPtclose = GetDataByScale(fTarPreClose,3,iSRptc);
	int ifPclose= 0;
	int iSRpc = 1;
	ifPclose = GetDataByScale(fPreClose,3,iSRpc);
	//权证涨停价
	float fRaise = (float)(ifPclose/(double)iSRpc+(ifUpper/(double)iSRup-ifPtclose/(double)iSRptc)*m_dWarRatio*fPowerratio);
	//float fRaise = (float)(fPreClose+(fUpper-fTarPreClose)*m_dWarRatio*fPowerratio);
	//fRaise = GetDataByScale(fRaise,3);
	fRaise+=0.00005f;
	*/
	//涨幅
	float fUpValue = fTarPreClose*ratio;
	int iScale = 3;
	//深交所=3位小数点
	//上交所=2位小数点
	if(pSecurity->IsShanghai()==true)
		iScale = 2;
	int iUp = 1;
	int ifUpValue = 0;
	ifUpValue = GetDataByScale(fUpValue,iScale,iUp);
	fUpValue = (float)(ifUpValue/(float)iUp);

	//权证涨停价
	float fRaise = (float)(fPreClose + fUpValue*m_dWarRatio*fPowerratio);
	fRaise+=0.00005f;
	int iUpRaise = 1;
	int ifRaise = 0;
	ifRaise = GetDataByScale(fRaise,3,iUpRaise);
	fRaise = (float)(ifRaise/(float)iUpRaise);

	return fRaise;
}
//取得跌停价格
float	TxWarrant::GetLower(int iSecurityId)
{
	SecurityQuotation* pSecurity = GetSecurityNow(iSecurityId);
	if(pSecurity==NULL)
		return Con_floatInvalid;
	return GetLower(pSecurity);
}
float	TxWarrant::GetLower(SecurityQuotation* pSecurity)
{
	//取得标的证券
	int TartgetId = GetObjectSecurityId(pSecurity->GetId());
	//行权比例
	double fPowerratio = GetPowerRatio(pSecurity->GetId());
	if(!(fPowerratio>0))
		return Con_floatInvalid;
	
	SecurityQuotation* pObjSecurity = GetSecurityNow(TartgetId);
	if(pObjSecurity==NULL)
		return Con_floatInvalid;
	//权证前收价
	float fPreClose = pSecurity->GetPreClosePrice();
	if(fPreClose<0)
		return Con_floatInvalid;
	//标的前收价
	float fTarPreClose = pObjSecurity->GetPreClosePrice();
	if(fTarPreClose<0)
		return Con_floatInvalid;

	//标的跌停价
	//2008-03-03
	float ratio = 0.1f;
	if(pObjSecurity->InSTblock()==true)
		ratio = 0.05f;
/*
	float fLower = pObjSecurity->GetLower(ratio);
	if(fLower<0)
		return Con_floatInvalid;
	int iScale = 3;
	//深交所=3位小数点
	//上交所=2位小数点
	if(pSecurity->IsShanghai()==true)
		iScale = 2;
	int ifUpper = 0;
	int iSRup = 1;
	ifUpper = GetDataByScale(fLower,iScale,iSRup);

	int ifPtclose= 0;
	int iSRptc = 1;
	ifPtclose = GetDataByScale(fTarPreClose,3,iSRptc);
	int ifPclose= 0;
	int iSRpc = 1;
	ifPclose = GetDataByScale(fPreClose,3,iSRpc);

	//权证涨停价
	float fRaise = (float)(ifPclose/(double)iSRpc-(ifPtclose/(double)iSRptc-ifUpper/(double)iSRup)*m_dWarRatio*fPowerratio);
	fRaise+=0.00005f;
*/
	//跌幅
	float fLowValue = fTarPreClose*ratio;
	int iScale = 3;
	//深交所=3位小数点
	//上交所=2位小数点
	if(pSecurity->IsShanghai()==true)
		iScale = 2;
	int iLow = 1;
	int ifLowValue = 0;
	ifLowValue = GetDataByScale(fLowValue,iScale,iLow);
	fLowValue = (float)(ifLowValue/(float)iLow);

	//权证跌停价
	float fRaise = (float)(fPreClose - fLowValue*m_dWarRatio*fPowerratio);
	fRaise+=0.00005f;
	int iLowRaise = 1;
	int ifRaise = 0;
	ifRaise = GetDataByScale(fRaise,3,iLowRaise);
	fRaise = (float)(ifRaise/(float)iLowRaise);
/*	double v = 0;
	v = GetDataByScale(fLower,iScale);

	fLower = v;//GetDataByScale(fLower,iScale);
	v = GetDataByScale(fTarPreClose,3);
	fTarPreClose =v;
	v = GetDataByScale(fPreClose,3);
	fPreClose = v;

	//权证涨停价
	float fRaise = (float)(fPreClose-(fTarPreClose-fLower)*m_dWarRatio*fPowerratio);
	fRaise = GetDataByScale(fRaise,3);
	*/
	if(fRaise<0)
		fRaise = 0;
	return fRaise;
}
//剩余期限
int TxWarrant::GetRemainsDays(int iSecurityId,int iDate)
{
	SecurityQuotation* pSecurity = GetSecurityNow(iSecurityId);
	if(pSecurity==NULL)
		return Con_intInvalid;
	return GetRemainsDays(pSecurity,iDate);
}
int TxWarrant::GetRemainsDays(SecurityQuotation* pSecurity,int iDate)
{
	int iEndDate = 0;//GetPowerEndDate(pSecurity);
	//存续截止日期 30301135
	//if(pSecurity->GetIndicatorValueAbsolute(30301135,iEndDate)==false)
	//	return Con_intInvalid;
	WarrantBasicInfo* pWarrantBasicInfo = pSecurity->GetWarrantBasicInfo();
	if(pWarrantBasicInfo==NULL)
		return Con_intInvalid;
	iEndDate = pWarrantBasicInfo->end_date;

	if(iEndDate==Con_intInvalid)
		return Con_intInvalid;
	ZkkLib::DateTime startDate(iDate);
	ZkkLib::DateTime endDate(iEndDate);
	return (int)(endDate-startDate).GetTotalDays();
}
//取得溢价率
//---------------------------------------------------------------
//认购权证溢价率=（行权价+认购权证价格/行权比例-正股价）/正股价 
//认沽权证溢价率=（正股价+认沽权证价格/行权比例-行权价）/正股价
//投资者需要变化多少百分比，才可以实现平本
//*******************************************************************
double	TxWarrant::GetPremiumRate(
		double dWPrice,		//权证价格
		double dSPrice,		//标的股票价格
		double dExPrice,	//行权价
		bool	bFlag,		//认购为true,反之为false
		double	dRatio		//行权比例
		)
{
	ComputingWarrant cw;
	return 100*cw.GetPremiumRate(
			dWPrice,		//权证价格
			dSPrice,		//标的股票价格
			dExPrice,	//行权价
			bFlag,		//认购为true,反之为false
			dRatio		//行权比例
		);

	//double dPre= 0.0;
	//if( bFlag )//认购
	//{
	//	dPre = (dExPrice + dWPrice / dRatio)/dSPrice - 1; 
	//	//认购权证溢价率=（行权价+认购权证价格/行权比例-正股价）/正股价 
	//	//dPre = (dExPrice + dWPrice/dRatio -  dSPrice)/dSPrice;
	//}
	//else		//认沽
	//{
	//	//认沽权证溢价率=（正股价+认沽权证价格/行权比例-行权价）/正股价
	//	//dPre = 1- (dExPrice - dWPrice/dRatio )/ dSPrice;
	//	dPre = 1 + (dWPrice/dRatio - dExPrice )/ dSPrice;
	//}

	//dPre *= 100;
	//return dPre;
}
//取得溢价率
double		TxWarrant::GetPremiumRate(int iSecurityId)
{
	SecurityQuotation* pSecurity = GetSecurityNow(iSecurityId);
	if(pSecurity==NULL)
		return Con_intInvalid;
	return GetPremiumRate(pSecurity);
}
double		TxWarrant::GetPremiumRate(SecurityQuotation* pSecurity)
{
	//行权价
	double dPowerPrice = GetPowerPrice(pSecurity);
	if(dPowerPrice<0)
		return Con_doubleInvalid;
	//现价
	float fPrice = pSecurity->GetClosePrice();
	if(fPrice<0)
		return Con_doubleInvalid;
	//认购
	bool bBuy = pSecurity->IsWarrant_Buy();
	//取得标的证券
	int TartgetId = GetObjectSecurityId(pSecurity->GetId());
	//行权比例
	double fPowerratio = GetPowerRatio(pSecurity->GetId());
	if(!(fPowerratio>0))
		return Con_doubleInvalid;
	
	SecurityQuotation* pObjSecurity = GetSecurityNow(TartgetId);
	if(pObjSecurity==NULL)
		return Con_doubleInvalid;
	float fObjPrice = pObjSecurity->GetClosePrice();
	if(fObjPrice<0)
		return Con_doubleInvalid;
	return GetPremiumRate(fPrice,fObjPrice,dPowerPrice,bBuy,fPowerratio);
}
double		TxWarrant::GetPremiumRate(SecurityQuotation* pSecurity,int date)
{
	//行权价
	double dPowerPrice = GetPowerPrice(pSecurity);
	if(dPowerPrice<0)
		return Con_doubleInvalid;
	//现价
	float fPrice = pSecurity->GetClosePrice(date,true);
	if(fPrice<0)
		return Con_doubleInvalid;
	//认购
	bool bBuy = pSecurity->IsWarrant_Buy();
	//取得标的证券
	int TartgetId = GetObjectSecurityId(pSecurity->GetId());
	//行权比例
	double fPowerratio = GetPowerRatio(pSecurity->GetId());
	if(!(fPowerratio>0))
		return Con_doubleInvalid;
	
	SecurityQuotation* pObjSecurity = GetSecurityNow(TartgetId);
	if(pObjSecurity==NULL)
		return Con_doubleInvalid;
	float fObjPrice = pObjSecurity->GetClosePrice(date,true);
	if(fObjPrice<0)
		return Con_doubleInvalid;
	return GetPremiumRate(fPrice,fObjPrice,dPowerPrice,bBuy,fPowerratio);
}
//杠杆比率
double	TxWarrant::GetGearRate(	double	dSClose,	//相关资产的收盘价
										double	dWClose,	//权证的收盘价
										double	dExRatio	//行权比例
										)
{
	ComputingWarrant cw;
	return cw.GetGearRate(dSClose,dWClose,dExRatio);
	//double	dGear = 0.0;
	//dGear = dSClose/( dWClose / dExRatio );
	//return dGear;
}
double		TxWarrant::GetGearRate(int iSecurityId)
{
	SecurityQuotation* pSecurity = GetSecurityNow(iSecurityId);
	if(pSecurity==NULL)
		return Con_intInvalid;
	return GetGearRate(pSecurity);
}
double		TxWarrant::GetGearRate(SecurityQuotation* pSecurity)
{
	//现价
	float fPrice = pSecurity->GetClosePrice();
	if(fPrice<0)
		return Con_doubleInvalid;
	//取得标的证券
	int TartgetId = GetObjectSecurityId(pSecurity->GetId());
	//行权比例
	double fPowerratio = GetPowerRatio(pSecurity->GetId());
	if(!(fPowerratio>0))
		return Con_doubleInvalid;
	
	SecurityQuotation* pObjSecurity = GetSecurityNow(TartgetId);
	if(pObjSecurity==NULL)
		return Con_doubleInvalid;
	float fObjPrice = pObjSecurity->GetClosePrice();
	if(fObjPrice<0)
		return Con_doubleInvalid;
	return GetGearRate(fObjPrice,fPrice,fPowerratio);
}
double		TxWarrant::GetGearRate(SecurityQuotation* pSecurity,int date)
{
	//现价
	float fPrice = pSecurity->GetClosePrice(date,true);
	if(fPrice<0)
		return Con_doubleInvalid;
	//取得标的证券
	int TartgetId = GetObjectSecurityId(pSecurity->GetId());
	//行权比例
	double fPowerratio = GetPowerRatio(pSecurity->GetId());
	if(!(fPowerratio>0))
		return Con_doubleInvalid;

	SecurityQuotation* pObjSecurity = GetSecurityNow(TartgetId);
	if(pObjSecurity==NULL)
		return Con_doubleInvalid;
	float fObjPrice = pObjSecurity->GetClosePrice(date,true);
	if(fObjPrice<0)
		return Con_doubleInvalid;
	return GetGearRate(fObjPrice,fPrice,fPowerratio);
}
//取得隐含波动率
double	TxWarrant::GetSigma(	bool	bCall_Put,		//认购认沽	true认购
									double	dStockPrice,//标的证券价格
									double	dExPrice,	//行权价格
									double	dNoRiskInterest,//无风险利率
									double	dDelta,		
									double	dLastedYear,//剩余期限（年）
									double	dMarketValue//期权市场价值
									)
{
	//-------------取得这些参数---------------------------
	double	dSigma = 0.0;
	////根据权证ID得到看跌看涨
	//bool	bCall_Put = false;	
	////根据ID日期得到股票价格
	//double	dStockPrice = 1;
	////期权执行价格
	//double	dExPrice = 1.0;
	////无风险收益率
	//double	dNoRiskInterest = 0.02;
	////标的股票年股利收益率
	//double	dDelta = 0.0;
	////剩余期限（年）
	//double	dLastedYear = 1.0;
	////期权市场价值
	//double	dMarketValue = 1.0;
	//***************************************************************
	CWarrantMath	WarrantMath;
	WarrantMath.implied_volatility(bCall_Put,dStockPrice,dExPrice,dNoRiskInterest,dDelta,dLastedYear,dMarketValue,&dSigma );
	
	return dSigma;
}
double		TxWarrant::GetSigma(int iSecurityId)
{
	SecurityQuotation* pSecurity = GetSecurityNow(iSecurityId);
	if(pSecurity==NULL)
		return Con_intInvalid;
	return GetSigma(pSecurity);
}
double		TxWarrant::GetSigma(SecurityQuotation* pSecurity)
{
	int iRenmainsgays = GetRemainsDays(pSecurity,ZkkLib::DateTime::getCurrentTime().GetDate().GetInt());
	double dLastYear = (double)iRenmainsgays/365;
	//行权价
	double dPowerPrice = GetPowerPrice(pSecurity);
	if(dPowerPrice<0)
		return Con_doubleInvalid;	
	
	//现价
	float fPrice = pSecurity->GetClosePrice();
	if(fPrice<0)
		return Con_doubleInvalid;
	//认购
	bool bBuy = pSecurity->IsWarrant_Buy();
	//取得标的证券
	int TartgetId = GetObjectSecurityId(pSecurity->GetId());
	//行权比例
	double fPowerratio = GetPowerRatio(pSecurity->GetId());
	if(!(fPowerratio>0))
		return Con_doubleInvalid;
	
	SecurityQuotation* pObjSecurity = GetSecurityNow(TartgetId);
	if(pObjSecurity==NULL)
		return Con_doubleInvalid;
	float fObjPrice = pObjSecurity->GetClosePrice();
	if(fObjPrice<0)
		return Con_doubleInvalid;
	double		dNoInterest = 0.018;
	
	double	dMarketValue=fPrice/fPowerratio;
	//标的股票年股利收益率
	double	dDelta = 0.0;
	return GetSigma(!bBuy,fObjPrice,dPowerPrice,dNoInterest,dDelta,dLastYear,dMarketValue);
}
double		TxWarrant::GetSigma(SecurityQuotation* pSecurity,int date)
{
	int iRenmainsgays = GetRemainsDays(pSecurity,date);
	double dLastYear = (double)iRenmainsgays/365;
	//行权价
	double dPowerPrice = GetPowerPrice(pSecurity);
	if(dPowerPrice<0)
		return Con_doubleInvalid;
	
	//现价
	float fPrice = pSecurity->GetClosePrice(date,true);
	if(fPrice<0)
		return Con_doubleInvalid;
	//认购
	bool bBuy = pSecurity->IsWarrant_Buy();
	//取得标的证券
	int TartgetId = GetObjectSecurityId(pSecurity->GetId());
	//行权比例
	double fPowerratio = GetPowerRatio(pSecurity->GetId());
	if(!(fPowerratio>0))
		return Con_doubleInvalid;
	
	SecurityQuotation* pObjSecurity = GetSecurityNow(TartgetId);
	if(pObjSecurity==NULL)
		return Con_doubleInvalid;
	float fObjPrice = pObjSecurity->GetClosePrice(date,true);
	if(fObjPrice<0)
		return Con_doubleInvalid;
	double		dNoInterest = 0.018;
	
	double	dMarketValue=fPrice/fPowerratio;
	//标的股票年股利收益率
	double	dDelta = 0.0;
	return GetSigma(!bBuy,fObjPrice,dPowerPrice,dNoInterest,dDelta,dLastYear,dMarketValue);
}
//排序分析数据结果标题列
bool TxWarrant::GetBlockAnalysisCol(Table_Display& baTable,std::vector<int>& arrSamples,int& iSortCol)
{
	if(TxBusiness::GetBlockAnalysisCol(baTable,arrSamples,iSortCol)==false)
		return false;

	baTable.SetPrecise(5, 3);
	baTable.SetPrecise(6, 3);
	baTable.SetPrecise(7, 3);

	int nCol = baTable.GetColCount();
	int sCol = 12;
	if(nCol>sCol)
		baTable.DeleteCol(sCol,nCol-sCol);

	//11 到期日期
	baTable.AddCol(Tx::Core::dtype_int4);
	//12 剩余期限
	baTable.AddCol(Tx::Core::dtype_val_string);
	//正股名称
	baTable.AddCol(Tx::Core::dtype_val_string);
	//正股代码
	baTable.AddCol(Tx::Core::dtype_val_string);
	//正股价格
	baTable.AddCol(Tx::Core::dtype_double);
	//13 波动
	baTable.AddCol(Tx::Core::dtype_double);
	//14 溢价
	baTable.AddCol(Tx::Core::dtype_double);
	//15 杠杆
	baTable.AddCol(Tx::Core::dtype_double);

	nCol = 12;
	baTable.SetTitle(nCol, _T("到期日期"));
	baTable.SetFormatStyle(nCol, Tx::Core::fs_date);
	++nCol;
	baTable.SetTitle(nCol, _T("剩余期限"));
	baTable.SetPrecise(nCol, 2);
	++nCol;

	//正股名称
	baTable.SetTitle(nCol, _T("正股名称"));
	++nCol;
	//正股代码
	baTable.SetTitle(nCol, _T("正股代码"));
	++nCol;
	//正股价格
	baTable.SetTitle(nCol, _T("正股价格"));
	++nCol;

	baTable.SetTitle(nCol, _T("波动"));
	baTable.SetPrecise(nCol, 3);
	++nCol;
	baTable.SetTitle(nCol, _T("溢价"));
	baTable.SetPrecise(nCol, 2);
	++nCol;
	baTable.SetTitle(nCol, _T("杠杆"));
	baTable.SetPrecise(nCol, 2);
#ifdef _DEBUG
	CString m_str=baTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(m_str);
#endif
	return true;
}
//排序分析数据结果设置
bool TxWarrant::SetBlockAnalysisCol(Table_Display& baTable,SecurityQuotation* psq,int& j,int ii)
{
	if(psq==NULL)
		return false;
	//11 到期日期
		int iEndDate=Con_intInvalid;
		//存续截止日期 30301135
		WarrantBasicInfo* pWarrantBasicInfo = psq->GetWarrantBasicInfo();
		if(pWarrantBasicInfo!=NULL)
		{
			iEndDate = pWarrantBasicInfo->end_date;
		}
		//if(psq->GetIndicatorValueAbsolute(30301135,iEndDate)==false)
		//	iEndDate=Con_intInvalid;
		baTable.SetCell(j,ii,iEndDate);
		j++;
	//12 剩余期限
		CString strDate;
		int iDate = psq->GetCurDataDateTime().GetDate().GetInt();
		int days = GetRemainsDays(psq,iDate);
		if((iEndDate>0 && iDate>iEndDate) || days<=0 )
			strDate = _T("已到期");
		else
			strDate.Format(_T("%d天"),days);
		baTable.SetCell(j,ii,strDate);
		j++;

		//
		int iObjId = GetObjectSecurityId(psq);
		SecurityQuotation* pSecurityObj = GetSecurityNow(iObjId);

		if(pSecurityObj!=NULL)
			baTable.SetCell(j,ii,pSecurityObj->GetName());
		j++;
		if(pSecurityObj!=NULL)
			baTable.SetCell(j,ii,pSecurityObj->GetCode());
		j++;
		double m_dPrice = (double)pSecurityObj->GetClosePrice(true);
		if(m_dPrice < 0.01)
			m_dPrice = Con_doubleInvalid;
		baTable.SetCell(j,ii,m_dPrice);
		j++;

	//13 波动
		baTable.SetCell(j,ii,GetSigma(psq));
		j++;
	//14 溢价
		baTable.SetCell(j,ii,GetPremiumRate(psq));
		j++;
	//15 杠杆
		baTable.SetCell(j,ii,GetGearRate(psq));
		j++;
#ifdef _DEBUG
		CString m_str=baTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(m_str);
#endif
	return true;
}

	}
}