/**************************************************************
	Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
	File name:		TxBond.cpp
	Author:			赵宏俊
	Date:			2007-10-29
	Version:		1.0
	Description:	
					债券业务功能类

***************************************************************/
#include "StdAfx.h"
#include "TxBond.h"
#include "MyIndicator.h"
#include "..\..\Data\XmlNodeWrapper.h"
#include "../../Core/Driver/ClientFileEngine/base/zip/ZipWrapper.h"

namespace Tx
{
	namespace Business
	{

TxBond::TxBond(void)
{
	//构造
	m_bCalcYTMoneDay = false;
	m_iYtmDate = -1;
}

TxBond::~TxBond(void)
{
	//析构
}
//付息频率 30001125 int
int	TxBond::GetPayInterstFrequency(int iSecurityId)
{
	GetSecurityNow(iSecurityId);
	return GetPayInterstFrequency(m_pSecurity);
}
int	TxBond::GetPayInterstFrequency(SecurityQuotation* pSecurity)
{
	if(pSecurity==NULL)
		return Con_intInvalid;
	//return pSecurity->GetIndicateIntValue(30001125);
	BondNewInfo* pBondNewInfo = pSecurity->GetBondNewInfo();
	if(pBondNewInfo==NULL)
		return Con_intInvalid;
	return pBondNewInfo->pay_interest_frequence;
}
CString TxBond::GetPayInterstFrequencyString(int iSecurityId)
{
	SecurityQuotation* pSecurity = GetSecurityNow(iSecurityId);
	return GetPayInterstFrequencyString(pSecurity);
}
CString TxBond::GetPayInterstFrequencyString(SecurityQuotation* pSecurity)
{
	if(pSecurity->IsBond()==false)
		return Con_strInvalid;
	BondNewInfo* pBondNewInfo = pSecurity->GetBondNewInfo();
	if(pBondNewInfo==NULL)
		return Con_strInvalid;
	int m = GetPayInterstFrequency(pSecurity);
	if(m==Con_intInvalid)
		return Con_strInvalid;
	CString sStr;
	sStr = Con_strInvalid;
	switch(m)
	{
	case 3:
		sStr = _T("每季度");
		break;
	case 6:
		sStr = _T("每半年");
		break;
	case 12:
		{
			if(pBondNewInfo->interest_type == 5)
				sStr = _T("到期一次还本付息");
			else
				sStr = _T("每年");
		}
		break;
	default:
		sStr.Format(_T("每%d个月"),m);
		break;
	}
	return sStr;
}

//计算天数计算方法 30001126 int
//1:30/360,2:ACT/365,3:ACT/ACT,4:ACT/360,5:NL/365,6:30/365
int TxBond::GetFigureMethod(int iSecurityId)
{
	GetSecurityNow(iSecurityId);
	return GetFigureMethod(m_pSecurity);
}
int TxBond::GetFigureMethod(SecurityQuotation* pSecurity)
{
	if(pSecurity==NULL)
		return Con_intInvalid;
	return pSecurity->GetIndicateIntValue(30001126);
}

//到期日期 30001128 int
int TxBond::GetEndDate(int iSecurityId)
{
	GetSecurityNow(iSecurityId);
	return GetEndDate(m_pSecurity);
}
int TxBond::GetEndDate(SecurityQuotation* pSecurity)
{
	if(pSecurity!=NULL)
	{
		BondNewInfo* pBondNewInfo = pSecurity->GetBondNewInfo();
		if(pBondNewInfo!=NULL)
		{
			return pBondNewInfo->end_date;
		}
	}
	return Con_intInvalid;
	//if(pSecurity==NULL)
	//	return Con_intInvalid;
	//return pSecurity->GetIndicateIntValue(30001128);
}
//发行量	30001204
double	TxBond::GetShare(int iSecurityId)
{
	GetSecurityNow(iSecurityId);
	return GetShare(m_pSecurity);
}
double	TxBond::GetShare(SecurityQuotation* pSecurity)
{
	if(pSecurity!=NULL)
	{
		//return pSecurity->GetIndicateDoubleValue(30001204);
		BondNewInfo* pBondNewInfo = pSecurity->GetBondNewInfo();
		if(pBondNewInfo!=NULL)
		{
			if(pBondNewInfo->share>0)
				return pBondNewInfo->share;
		}
	}
	return Con_doubleInvalid;
}
//票面利率	30001205
double TxBond::GetParRate(int iSecurityId)
{
	GetSecurityNow(iSecurityId);
	return GetParRate(m_pSecurity);
}
double TxBond::GetParRate(SecurityQuotation* pSecurity)
{
	if(pSecurity!=NULL)
	{
		BondNewInfo* pBondNewInfo = pSecurity->GetBondNewInfo();
		if(pBondNewInfo!=NULL)
		{
			return pBondNewInfo->par_rate;
		}
	}
	return Con_doubleInvalid;
	//return pSecurity->GetIndicateDoubleValue(30001205);
}
//30001125		付息频率
//发行年限	30001206
double	TxBond::GetHoldYear(int iSecurityId)
{
	GetSecurityNow(iSecurityId);
	return GetHoldYear(m_pSecurity);
}
double	TxBond::GetHoldYear(SecurityQuotation* pSecurity)
{
	if(pSecurity!=NULL)
	{
		BondNewInfo* pBondNewInfo = pSecurity->GetBondNewInfo();
		if(pBondNewInfo!=NULL)
		{
			return pBondNewInfo->hold_year;
		}
	}
	return Con_doubleInvalid;

	//if(pSecurity==NULL)
	//	return Con_intInvalid;
	//return pSecurity->GetIndicateDoubleValue(30001206);
}
//起息日期	30001208
int	TxBond::GetBeginDate(int iSecurityId)
{
	GetSecurityNow(iSecurityId);
	return GetBeginDate(m_pSecurity);
}
int	TxBond::GetBeginDate(SecurityQuotation* pSecurity)
{
	if(pSecurity!=NULL)
	{
		BondNewInfo* pBondNewInfo = pSecurity->GetBondNewInfo();
		if(pBondNewInfo!=NULL)
		{
			return pBondNewInfo->begin_date;
		}
	}
	return Con_intInvalid;
	//if(pSecurity==NULL)
	//	return Con_intInvalid;
	//return pSecurity->GetIndicateIntValue(30001208);
}
//30001207		付息方式
int	TxBond::GetInterestType(int iSecurityId)
{
	GetSecurityNow(iSecurityId);
	return GetInterestType(m_pSecurity);
}
int	TxBond::GetInterestType(SecurityQuotation* pSecurity)
{
	//if(pSecurity==NULL)
	//	return Con_intInvalid;
	//return pSecurity->GetIndicateIntValue(30001207);
	if(pSecurity!=NULL)
	{
		BondNewInfo* pBondNewInfo = pSecurity->GetBondNewInfo();
		if(pBondNewInfo!=NULL)
		{
			return pBondNewInfo->interest_type;
		}
	}
	return Con_intInvalid;
	
}
//债券类型
CString TxBond::GetBondType(int iSecurityId)
{
	GetSecurityNow(iSecurityId);
	return GetBondType(m_pSecurity);
}
CString TxBond::GetBondType(SecurityQuotation* pSecurity)
{
	if(pSecurity!=NULL)
	{
		BondNewInfo* pBondNewInfo = pSecurity->GetBondNewInfo();
		if(pBondNewInfo!=NULL)
		{
			return Tx::Data::TypeMapManage::GetInstance()->GetDatByID(TYPE_BOND_TYPE,pBondNewInfo->bond_type);
		}
	}
	return Con_strInvalid;
}
//债券形式
CString TxBond::GetBondForm(int iSecurityId)
{
	GetSecurityNow(iSecurityId);
	return GetBondForm(m_pSecurity);
}
CString TxBond::GetBondForm(SecurityQuotation* pSecurity)
{
	if(pSecurity!=NULL)
	{
		BondNewInfo* pBondNewInfo = pSecurity->GetBondNewInfo();
		if(pBondNewInfo!=NULL)
		{
			return Tx::Data::TypeMapManage::GetInstance()->GetDatByID(TYPE_BOND_FORM,pBondNewInfo->bond_form);
		}
	}
	return Con_strInvalid;
}
//记息方式1:单利,2:复利,3:浮动利率,4:累进利率,5:贴现
CString TxBond::GetBondInterestType(int iSecurityId)
{
	GetSecurityNow(iSecurityId);
	return GetBondInterestType(m_pSecurity);
}
CString TxBond::GetBondInterestType(SecurityQuotation* pSecurity)
{
	if(pSecurity!=NULL)
	{
		BondNewInfo* pBondNewInfo = pSecurity->GetBondNewInfo();
		if(pBondNewInfo!=NULL)
		{
			//return Tx::Data::TypeMapManage::GetInstance()->GetDatByID(TYPE_BOND_IPO_TYPE,pBondNewInfo->interest_type);
			switch(pBondNewInfo->interest_type)
			{
			case 1:
				return _T("单利");
				break;
			case 2:
				return _T("复利");
				break;
			case 3:
				return _T("浮动利率");
				break;
			case 4:
				return _T("累进利率");
				break;
			case 5:
				return _T("贴现");
				break;
			}
		}
	}
	return Con_strInvalid;
}

//30001123		记息方式
int TxBond::GetMarkManner(int iSecurityId)
{
	GetSecurityNow(iSecurityId);
	return GetMarkManner(m_pSecurity);
}
int	TxBond::GetMarkManner(SecurityQuotation* pSecurity)
{
	if(pSecurity==NULL)
		return Con_intInvalid;
	return pSecurity->GetIndicateIntValue(30001123);
}
//剩余期限
int	TxBond::GetRemnantsDays(int iSecurityId,int iDate)
{
	GetSecurityNow(iSecurityId);
	return GetRemnantsDays(m_pSecurity,iDate);
}
int	TxBond::GetRemnantsDays(SecurityQuotation* pSecurity,int iDate)
{
	//取得到期日期
	int iEndDate = GetEndDate(pSecurity);
	if(iEndDate==Con_intInvalid)
		return Con_intInvalid;
	if(iDate>iEndDate)
		return Con_intInvalid;
	ZkkLib::DateTime s(iDate);
	ZkkLib::DateTime e(iEndDate);
	double dtotdays = (e-s).GetTotalDays();
	return (int)dtotdays;
}
double	TxBond::GetRemnants(int iSecurityId,int iDate)
{
	int days = GetRemnantsDays(iSecurityId,iDate);
	return (double)days/(double)365;
}
double	TxBond::GetRemnants(SecurityQuotation* pSecurity,int iDate)
{
	return GetRemnants(pSecurity->GetId(),iDate);
}
//计算指定日期基本剩余期限[年+天数]
bool	TxBond::GetRemnantsBasic(int iStart,int iEnd,int& iYear,int& iDays,int& iTotDays)
{
	if(iStart>iEnd || iEnd<0)
		return false;
	ZkkLib::DateTime s(iStart);
	ZkkLib::DateTime e(iEnd);
	int sYear = s.GetYear();
	int eYear = e.GetYear();

	int iS = s.GetDate().GetInt();
	int iE = e.GetDate().GetInt();

	//int iYear = eYear - sYear+1;
	iYear = 0;
	while((iS+10000)<=iE)
	{
		iYear++;
		iS+=10000;
	}

	//20070124 闰年保护
	if(ZkkLib::DateTime::IsLeapYear(iS/10000)!=TRUE && iS%10000 == 229)
		iS -= 1;
	ZkkLib::DateTime s1(iS);
	ZkkLib::DateTime e1(iE);

	iTotDays = 365;
	if(ZkkLib::DateTime::IsLeapYear(iS/10000)==TRUE)
		iTotDays = 366;

	//int iDays = (int)(e1-s1).GetTotalDays();
	iDays = (int)(e1-s1).GetTotalDays();
	return true;
}

//计算单位为年的剩余期限
double	TxBond::GetRemnantsYear(int iStart,int iEnd,bool bNewBond)
{
	/*
	int iYear = 0;
	int iDays = 0;
	int iTotDays = 0;
	if(GetRemnantsBasic(iStart,iEnd,iYear,iDays,iTotDays)==false)
		return Con_doubleInvalid;

	if(iTotDays==0)
		return Con_doubleInvalid;

	//[闰年2月29日不计]
	//------------------------------------------------------------
	int start_year;
	int end_year;

	if(iStart%10000 <= 228)
		start_year = iStart/10000;
	else
	{
		if(iStart%10000 == 229)
				iDays--;
		start_year = iStart/10000 + 1;
	}

	if(iEnd%10000 <= 228)
		end_year = iEnd/10000 - 1;
	else
		end_year = iEnd/10000;

	//------------------------------------------------------------
	for(int year=start_year; year<=end_year; year++)
	{
		if(ZkkLib::DateTime::IsLeapYear(year) == TRUE)
				iDays--;
	}

	//

	if(bNewBond==false)
	{
		iDays++;
		//return ((double)iYear + (double)iDays / (double)iTotDays);
		return ((double)iYear + (double)iDays / 365);
	}
	else
	{
		iDays++;
		return ((double)iYear + (double)iDays / 365);
	}
	*/
	int iDays = 0;
	iDays = Get_Span_Days_For_Bond(iStart,iEnd);
	return (double)iDays / 365;
}
//计算单位为年的剩余期限
int	TxBond::GetRemnants2Days(int iStart,int iEnd)
{
	int iYear = 0;
	int iDays = 0;
	int iTotDays = 0;
	if(GetRemnantsBasic(iStart,iEnd,iYear,iDays,iTotDays)==false)
		return Con_intInvalid;

	if(iTotDays==0)
		return Con_intInvalid;

	return (iYear*365+iTotDays);
}

CString TxBond::GetRemnantsString(int iStart,int iEnd)
{
	if(iEnd<=0)
		return _T("");
	if(iStart>=iEnd)
		return _T("[已到期]");
	int iYear = 0;
	int iDays = 0;
	int iTotDays = 0;
	if(GetRemnantsBasic(iStart,iEnd,iYear,iDays,iTotDays)==false)
		return Con_strInvalid;
	CString strDate;
	if(iYear>0)
		strDate.Format(_T("%d年%d天"),iYear,iDays);
	else
		strDate.Format(_T("%d天"),iDays);
	return strDate;
}
CString TxBond::GetRemnants(int days)
{
	if(days<=0)
		return _T("-");

	int nYears = days/365;
	int ndays = days%365;
	ndays -= m_iNdays;
	CString strDate;
	if(nYears>0)
		strDate.Format(_T("%d年%d天"),nYears,ndays);
	else
		strDate.Format(_T("%d天"),ndays);
	return strDate;
}

//****************************************************************
//	取得两日期间的天数
//****************************************************************
/*FUN_EXT_API(LONG) On_TXR_Get_Span_Days(LONG start_date, LONG end_date)
{
	if(start_date > end_date)
		return 0;

	else if(start_date == end_date)
		return 1;

	//------------------------------------------------------------
	m_Ole_time_start.SetDateTime(start_date/10000, start_date/100%100, start_date%100, 0, 0, 0);
	m_Ole_time_end.SetDateTime(end_date/10000, end_date/100%100, end_date%100, 0, 0, 0);
	m_Ole_time_span = m_Ole_time_end - m_Ole_time_start;
	return m_Ole_time_span.GetDays() + 1;
}
*/
//YTM_365
//	取得两日期间的天数 [闰年2月29日不计]
int TxBond::Get_Span_Days_For_Bond(int start_date, int end_date)
{
	int span_days;

	//------------------------------------------------------------
	if(start_date > end_date)
		return 0;

	else if(start_date == end_date)
		return 1;

	else if(start_date%10000 == end_date%10000+1)
		return (end_date/10000 - start_date/10000)*365;

	else
	{
		ZkkLib::DateTime s(start_date);
		ZkkLib::DateTime e(end_date);
		//span_days = (int)((e-s).GetTotalDays())+1;
		span_days = (int)(e-s).GetTotalDays();
		//将当日加上
		span_days++;
	}

	//------------------------------------------------------------
	if(span_days < 2)
		return span_days;

	//------------------------------------------------------------
	int start_year;
	int end_year;

	if(start_date%10000 <= 228)
		start_year = start_date/10000;
	else
		{	if(start_date%10000 == 229)
				span_days--;
			start_year = start_date/10000 + 1;
		}

	if(end_date%10000 <= 228)
		end_year = end_date/10000 - 1;
	else
		end_year = end_date/10000;

	//------------------------------------------------------------
	for(int year=start_year; year<=end_year; year++)
	{
		if(ZkkLib::DateTime::IsLeapYear(year)==TRUE)
				span_days--;
	}

	//------------------------------------------------------------
	return span_days;
}


//应计利息
double	TxBond::GetInterest(int iSecurityId,int iDate)
{
	GetSecurityNow(iSecurityId);
	return GetInterest(m_pSecurity,iDate);
}
double	TxBond::GetInterest(SecurityQuotation* pSecurity,int iDate)
{
	if(pSecurity==NULL)
		return 0;
	//30001123 记息方式=5,贴现债
	//int iInterestType = pSecurity->GetIndicateIntValue(30001123);
	//if(iInterestType<0 || iInterestType==5)
	//	return 0;
	BondNewInfo* pBondNewInfo = pSecurity->GetBondNewInfo();
	if(pBondNewInfo==NULL || pBondNewInfo->interest_type==5 )
		return 0;

	//取得指定日期的现金流
	BondCashFlowData*	pBondCashFlowData = pSecurity->GetBondCashFlowDataByDate(iDate);
	if(pBondCashFlowData==NULL)
		return 0;
	//取得已经支付利息的天数
	//int spanDays = GetRemainsDays(pBondCashFlowData->F_LIPD,iDate);
	//取得本期/应计利息天数
	//int allSpanDays = GetRemainsDays(pBondCashFlowData->F_LIPD,pBondCashFlowData->F_NIPD);

	int spanDays = Get_Span_Days_For_Bond(pBondCashFlowData->F_LIPD,iDate);
	int allSpanDays = Get_Span_Days_For_Bond(pBondCashFlowData->F_LIPD,pBondCashFlowData->F_NIPD);

	//2008-11-25 因为使用了计息截止日期
	//allSpanDays--;

	//allSpanDays = GetRemnants2Days(pBondCashFlowData->F_LIPD,pBondCashFlowData->F_NIPD);
	//
	if(allSpanDays > 0)
		return pBondCashFlowData->F_RAW_CASH * spanDays / allSpanDays;
	else
		return 0;
}

//应计利息_新   2012-7-3  bFlag-- true=="日间"，false=="日终"
double TxBond::GetInterest_New(int iSecurityId,int iDate,bool bFlag)
{
	GetSecurityNow(iSecurityId);
	return GetInterest_New(m_pSecurity,iDate,bFlag);
}
double TxBond::GetInterest_New(SecurityQuotation* pSecurity,int iDate,bool bFlag)
{
	if(pSecurity == NULL) 
		return Con_doubleInvalid;
	BondPmllAndYjlx* pBondPmllAndYjlx = pSecurity->GetBondPmllAndYjlxByDate(iDate);
	//应计利息只能取当天数据，如果今天没有数据，则返回 Con_doubleInvalid
	if (pBondPmllAndYjlx == NULL || iDate > pBondPmllAndYjlx->iDate || iDate < pBondPmllAndYjlx->iDate)
		return Con_doubleInvalid;
	double interest;
	if (bFlag)
		interest = pBondPmllAndYjlx->interest;
	else
		interest = pBondPmllAndYjlx->interest_DayEnd;
	
	return interest;
}
//取得指定日期的股本数据
BondCashFlowData*	TxBond::GetBondCashFlowDataByDate(DataFileNormal<blk_TxExFile_FileHead,BondCashFlowData>* pCashFlow,int date)
{
	if(pCashFlow==NULL)
		return NULL;
	int count = pCashFlow->GetDataCount();
	if(count<=0)
		return NULL;

	BondCashFlowData* pData = pCashFlow->GetDataByIndex();
	if(pData==NULL)
		return NULL;
	int sdate = pData->F_LIPD;
	pData = pCashFlow->GetDataByIndex(count-1);
	if(pData==NULL)
		return NULL;
	int edate = pData->F_NIPD;

	int index = -1;
	
	//从头开始查找
	for(int i=0;i<count;i++)
	{
		BondCashFlowData* pitem = pCashFlow->GetDataByIndex(i);
		if(pitem==NULL)
			continue;
		if(	(pitem->F_YXQSR <= date && (pitem->F_YXZZR >= date || pitem->F_YXZZR == 0)) &&
			(pitem->F_LIPD > 0 && pitem->F_LIPD <= date && (pitem->F_NIPD >= date || pitem->F_NIPD == 0) &&
			pitem->F_TYPE != 0
			)
			)
		{
			index = i;
			break;
		}
	}
	if(index<0)
		return NULL;
	
	pData = pCashFlow->GetDataByIndex(index);

	return pData;
}

double	TxBond::GetInterest(BondNewInfo* pBondNewInfo,DataFileNormal<blk_TxExFile_FileHead,BondCashFlowData>* pCashFlow,int iDate)
{
	if(pCashFlow==NULL)
		return 0;
	//30001123 记息方式=5,贴现债
	//int iInterestType = pSecurity->GetIndicateIntValue(30001123);
	//if(iInterestType<0 || iInterestType==5)
	//	return 0;
	if(pBondNewInfo==NULL || pBondNewInfo->interest_type==5 )
		return 0;

	//取得指定日期的现金流
	BondCashFlowData*	pBondCashFlowData = GetBondCashFlowDataByDate(pCashFlow,iDate);
	if(pBondCashFlowData==NULL)
		return 0;
	//取得已经支付利息的天数
	//int spanDays = GetRemainsDays(pBondCashFlowData->F_LIPD,iDate);
	//取得本期/应计利息天数
	//int allSpanDays = GetRemainsDays(pBondCashFlowData->F_LIPD,pBondCashFlowData->F_NIPD);


	int spanDays = Get_Span_Days_For_Bond(pBondCashFlowData->F_LIPD,iDate);
	int allSpanDays = Get_Span_Days_For_Bond(pBondCashFlowData->F_LIPD,pBondCashFlowData->F_NIPD);

	//注意：在老系统里F_LIPD和F_NIPD的月日不同年不同；在新系统里F_LIPD和F_NIPD的月日相同同年不同；
	//新债测算使用老系统算法
	//allSpanDays--;

	//到期一次付息还本
	if(pBondNewInfo->pay_interest_frequence == 0)
		allSpanDays--;

	//allSpanDays = GetRemnants2Days(pBondCashFlowData->F_LIPD,pBondCashFlowData->F_NIPD);
	//
	if(allSpanDays > 0)
		return pBondCashFlowData->F_RAW_CASH * spanDays / allSpanDays;
	else
		return 0;
}
double TxBond::GetBondInterest(SecurityQuotation* pSecurity,int iDate)
{
	return GetInterest(pSecurity,iDate);
	//2012-7-16
	//return GetInterest_New(pSecurity,iDate);
}

//计息天数
int	TxBond::GetInterestDays(int iSecurityId,int iDate)
{
	GetSecurityNow(iSecurityId);
	return GetInterestDays(m_pSecurity,iDate);
}
int	TxBond::GetInterestDays(SecurityQuotation* pSecurity,int iDate)
{
	//取得指定日期的现金流
	BondCashFlowData*	pBondCashFlowData = pSecurity->GetBondCashFlowDataByDate(iDate);
	if(pBondCashFlowData==NULL)
		return 0;
	//取得已经支付利息的天数
	int iSdate = pBondCashFlowData->F_LIPD;
	//if(pSecurity->GetIndicateIntValue(30001206)==1)
	//{
	//	iSdate = pSecurity->GetIndicateIntValue(30001127);
	//}

	//return GetRemainsDays(iSdate,iDate);
	return Get_Span_Days_For_Bond(iSdate,iDate);
}
//用于新债模拟测算
int	TxBond::GetInterestDays(DataFileNormal<blk_TxExFile_FileHead,BondCashFlowData>* pCashFlow,int iDate)
{
	//取得指定日期的现金流
	BondCashFlowData*	pBondCashFlowData = GetBondCashFlowDataByDate(pCashFlow,iDate);
	if(pBondCashFlowData==NULL)
		return 0;
	//取得已经支付利息的天数
	int iSdate = pBondCashFlowData->F_LIPD;
	//if(pSecurity->GetIndicateIntValue(30001206)==1)
	//{
	//	iSdate = pSecurity->GetIndicateIntValue(30001127);
	//}

	//return GetRemainsDays(iSdate,iDate);
	return Get_Span_Days_For_Bond(iSdate,iDate);
}

//起息日期=第一笔现金流的起息日期
int	TxBond::GetFirstBeginDate(int iSecurityId)
{
	GetSecurityNow(iSecurityId);
	return GetFirstBeginDate(m_pSecurity);
}
int	TxBond::GetFirstBeginDate(SecurityQuotation* pSecurity)
{
	//取得指定日期的现金流
	//BondCashFlowData*	pBondCashFlowData = pSecurity->GetBondCashFlowDataByIndex();
	//if(pBondCashFlowData==NULL)
	//	return Con_intInvalid;
	//return pBondCashFlowData->F_LIPD;
/*
	//30001127
	if(pSecurity==NULL)
		return Con_intInvalid;
	if(pSecurity->IsBond()==false)
		return Con_intInvalid;
	return pSecurity->GetIndicateIntValue(30001127);
*/
	if(pSecurity!=NULL)
	{
		BondNewInfo* pBondNewInfo = pSecurity->GetBondNewInfo();
		if(pBondNewInfo!=NULL)
		{
			return pBondNewInfo->begin_date;
		}
	}
	return Con_intInvalid;
}

//计算两日期间的天数
//从指定日期到付息日期之间的天数
float TxBond::CalcPower(int iStart,int iEnd,int iLip,float fFxpl,int iFigureMethod)
{
	float fPower = 0;
	int dayOfMonth = 0;
	int dayOfYear = 0;

	m_dSpanDays = 0;
	m_iTotalDaysOfYear = 0;

	if(iStart<=0 || iEnd<=0 || iStart>iEnd)
	{
		m_dSpanDays = Con_doubleInvalid;
		return Con_floatInvalid;
	}

	ZkkLib::DateTime startDate(iStart);
	ZkkLib::DateTime endDate(iEnd);
	ZkkLib::DateTime lipDate(iLip);

	int isDay = startDate.GetDay();
	int ieDay = endDate.GetDay();
	//1:30/360,2:ACT/365,3:ACT/ACT,4:ACT/360,5:NL/365,6:30/365
	switch(iFigureMethod)
	{
	case 1:
		//30/360
		//这一惯例被用于市政债券和企业债券。
		//每一月被视为有30天，因此从2005年2月1日到 2005年4月1日共有60天。
		//每一年被视为有360天。
		//这一惯例较为常用的原因是因为其计算简便：支付利息较为规律以及利息金额具有可预测性。
		if(ieDay==31)
		{
			if(isDay>=30)
				ieDay--;
			else
				ieDay++;
		}
		if(isDay==31)
			isDay--;
		m_dSpanDays = (endDate.GetYear()-startDate.GetYear())*360+(endDate.GetMonth()-startDate.GetMonth())*30+(ieDay-isDay);
		m_iTotalDaysOfYear = 360;
		//fPower = (float)((endDate.GetYear()-startDate.GetYear())*360+(endDate.GetMonth()-startDate.GetMonth())*30+(ieDay-isDay))/(float)360;

		break;
	case 2:
		//ACT/365
		//这一惯例应用于大多数国家政府债券的发行，包括美国，英国，加拿大，澳大利亚和新西兰。
		//每个月份被视为标准月，且假定每一年有365天，而不考虑闰年的情况。
		//例如，从2005年2月1日到 2005年4月1日，就认为有59天。
		//应用这一惯例的计算结果将与ACT/360方法计算的结果稍有不同。
		m_dSpanDays = (endDate-startDate).GetTotalDays();
		m_iTotalDaysOfYear = 365;

		//fPower = (float)((endDate-startDate).GetTotalDays())/(float)365;
		break;
	case 3:
		//ACT/ACT
		m_dSpanDays = (endDate-startDate).GetTotalDays();
		m_iTotalDaysOfYear = (int)((endDate-lipDate).GetTotalDays());
		//fPower = (float)((endDate-startDate).GetTotalDays())/(float)((endDate-lipDate).GetTotalDays())/fFxpl;
		break;
	case 4:
		//ACT/360 = 实际天数/360天
		//这一惯例应用于货币市场进行短期融资，币种包括美元和欧元，这一方法并应用于欧洲中央银行体系的货币政策操作。
		//每一个月被视为标准月，且假定每一年有360天。
		m_dSpanDays = (endDate-startDate).GetTotalDays();
		m_iTotalDaysOfYear = 360;
		//fPower = (float)((endDate-startDate).GetTotalDays())/(float)360;
		break;
	case 5://NL/365
		{
			//计算应计利息时是准确的
			//计算剩余期限时与老系统不一致
			m_iNdays = 0;
			double totdays = (endDate-startDate).GetTotalDays();
			for(int ii=startDate.GetYear();ii<=endDate.GetYear();ii++)
			{
				if(ii==startDate.GetYear())
				{
					if(ZkkLib::DateTime::IsLeapYear(ii)==TRUE)
					{
						if(startDate.GetMonth()<3)
						{
							totdays-=1;
							m_iNdays++;
						}
					}
				}
				else if(ZkkLib::DateTime::IsLeapYear(ii)==TRUE)
				{
					totdays-=1;
					m_iNdays++;
				}
			}
			m_dSpanDays = totdays;
			if(m_dSpanDays==365)
				m_dSpanDays--;
			m_iTotalDaysOfYear = 365;
			//fPower = (float)totdays/(float)365;
			
			/*
			m_iTotalDaysOfYear = 365;

			int sYear = startDate.GetYear();
			int eYear = endDate.GetYear();

			int iS = startDate.GetDate().GetInt();
			int iE = endDate.GetDate().GetInt();

			int iYear = eYear - sYear+1;
			iYear = 0;
			while((iS+10000)<=iE)
			{
				iYear++;
				iS+=10000;
			}

			//20070124 闰年保护
			if(ZkkLib::DateTime::IsLeapYear(iS/10000)!=TRUE && iS%10000 == 229)
				iS -= 1;
			ZkkLib::DateTime s1(iS);
			ZkkLib::DateTime e1(iE);

			int iDays = (int)(e1-s1).GetTotalDays();
			m_dSpanDays = iDays + m_iTotalDaysOfYear*iYear;
			*/
		}

		break;
	case 6:
		//30/365
		if(ieDay==31)
		{
			if(isDay>=30)
				ieDay--;
			else
				ieDay++;
		}
		if(isDay==31)
			isDay--;
		m_dSpanDays = (endDate.GetYear()-startDate.GetYear())*365+(endDate.GetMonth()-startDate.GetMonth())*30+(ieDay-isDay);
		m_iTotalDaysOfYear = 365;
		//fPower = (float)((endDate.GetYear()-startDate.GetYear())*365+(endDate.GetMonth()-startDate.GetMonth())*30+(ieDay-isDay))/(float)365;
		break;
	default:
		return 0;
	}
	m_dSpanDays++;
	fPower = (float)m_dSpanDays/(float)m_iTotalDaysOfYear;
	if(iFigureMethod==3)
	{
		fPower /= fFxpl;
	}

	return fPower;
}
//取得债券剩余期限[天数] NL/365
int TxBond::GetRemainsDays(int iStart,int iEnd)
{
	CalcPower(iStart,iEnd,0,0,5);
	return m_dSpanDays<0?Con_intInvalid:(int)m_dSpanDays;
}
bool TxBond::Calc1(int iSecurityId,int iDate)
{
	if(iDate==m_iYtmDate)
		return true;

	//取得指定债券
	GetSecurityNow(iSecurityId);
	if(m_pSecurity==NULL)
		return false;
	if(m_pSecurity->IsBond()==false)
		return false;

	//取得到期日起
	int iEndDate = GetEndDate(m_pSecurity);
	if(iEndDate==Con_intInvalid)
		return false;

	//有效性判断
	if( iDate > iEndDate )
	   return false;

	//取得指定债券的现金流数据
	//取得指定债券的现金流数据的数量
	int iCashFlowCount = m_pSecurity->GetBondCashFlowDataCount();

	//类型
	int iFigureMethod = 0;//GetFigureMethod(m_pSecurity);
	iFigureMethod = 5;

	int i = 0;
	{
		int iLastnipd = 0;
		//现金流折现修正----------------------------------------------
		for(i=0; i<iCashFlowCount; i++)
		{
			BondCashFlowData* pitem = m_pSecurity->GetBondCashFlowDataByIndex(i);

			//[除息日，付息日]之间，本期现金流忽略 +20050803
			if(pitem->F_DIVD > 0 && pitem->F_DIVD <= iDate
				&& pitem->F_NIPD >= iDate)
			//	continue;

			//if(pitem->F_NIPD < iDate)
			//	continue;
			//pitem->F_POWER = GetRemnantsYear(iDate,pitem->F_NIPD);
			{
				if(pitem->F_NIPD>0)
					iLastnipd = pitem->F_NIPD;
				continue;
			}

			int nipd = pitem->F_NIPD;
			if(nipd<=0)
			{
				if(pitem->F_TYPE==0)//本金
					//nipd = pitem->F_YXZZR;
					nipd = iLastnipd;
			}
			iLastnipd = nipd;
			if(nipd < iDate)
			{
				continue;
			}

			pitem->F_POWER = GetRemnantsYear(iDate,nipd);

		}
		m_pSecurity->SetBondCashFlowModified(true);
	}
	m_iYtmDate = iDate;
	return true;
}
//	计算到期收益率
bool TxBond::Calc(int iSecurityId,int iDate,float fPrice,int iBondCount)
{
	//CString sss = m_pFunctionDataManager->GetCollectionTreeName(50010183);
	//初始化
	m_dYTM = Con_doubleInvalid;
	m_dMDUR = Con_doubleInvalid;
	m_dDURATION = Con_doubleInvalid;
	m_dCONVEXITY = Con_doubleInvalid;

	//有效性判断
	if(iDate < 19000101   ||
	   fPrice < 0)
	   return false;
	////fPrice+=0.0005f;
	//int iUpRaise = 1;
	//int ifRaise = 0;
	//ifRaise = GetDataByScale(fPrice,2,iUpRaise);
	//fPrice = (float)(ifRaise/(float)iUpRaise);

//	GetSecurityNow(1);
//	m_pSecurity->DoHisTrade();
//
//	int count = m_pSecurity->m_pHisTradeData->GetDataCount();
//	m_pSecurity->m_pHisTradeData->SetBuildMap(true);
//	//
//#ifdef _DEBUG
//	TRACE("\nhis trade data start[%d]",GetTickCount());
//#endif
//	m_pSecurity->m_pHisTradeData->BuildMap();
//#ifdef _DEBUG
//	TRACE("\nhis trade data after[%d]",GetTickCount());
//#endif
//	int z = 0;
//	int idate = 0;
//	for(int i=0;i<1000;i++)
//	{
//		for(z=0;z<count;z++)
//		{
//			HisTradeData *p11 = m_pSecurity->m_pHisTradeData->GetDataByIndex(z);
//			idate = p11->GetMapObj();
//			m_pSecurity->m_pHisTradeData->GetDataByObj(idate);
//		}
//	}
//#ifdef _DEBUG
//	TRACE("\nhis trade data end[%d][z=%d][date=%d]",GetTickCount(),z,idate);
//#endif
//
	//取得指定债券
	GetSecurityNow(iSecurityId);
	if(m_pSecurity==NULL)
		return false;
	if(m_pSecurity->IsBond()==false)
		return false;

	//取得到期日起
	int iEndDate = GetEndDate(m_pSecurity);
	if(iEndDate==Con_intInvalid)
		return false;

	//有效性判断
	if( iDate > iEndDate )
	   return false;
	BondNewInfo* pBondNewInfo = m_pSecurity->GetBondNewInfo();
	if(pBondNewInfo==NULL)
		return false;

	//确定付息频率[每年几次]
	double fxpl;
	if(iBondCount == 1)
	{
		// m_pCash_flow->F_FXPL;
		//应该以指标方式提供
		//fxpl = (float)12/(float)GetPayInterstFrequency(m_pSecurity);
		fxpl = (double)pBondNewInfo->pay_interest_frequence;
		if(fxpl<0)
		{
			fxpl = 1;
		}
		else
		{
			fxpl = 12/fxpl;
		}
	}
	else
		fxpl = 1;

	//2008-08-19
	//double dInterest = GetInterest(m_pSecurity,iDate);
	//2012-7-16  应计利息(新)
	double dInterest = GetInterest_New(m_pSecurity,iDate,true);
	//公司债使用净价
	//if(m_pSecurity->IsBond_Corporation()==true && dInterest>0)
	//	fPrice -= (float)dInterest;

	//取得指定债券的现金流数据
	//取得指定债券的现金流数据的数量
	int iCashFlowCount = m_pSecurity->GetBondCashFlowDataCount();

	//类型
	int iFigureMethod = 0;//GetFigureMethod(m_pSecurity);
	iFigureMethod = 5;

	int i = 0;
	if(m_bCalcYTMoneDay== false || m_pSecurity->GetBondCashFlowModified()==false)
	{
		int iLastnipd = 0;
		//现金流折现修正----------------------------------------------
		for(i=0; i<iCashFlowCount; i++)
		{
			BondCashFlowData* pitem = m_pSecurity->GetBondCashFlowDataByIndex(i);

			//[除息日，付息日]之间，本期现金流忽略 +20050803
			if(pitem->F_DIVD > 0 && pitem->F_DIVD <= iDate
				&& pitem->F_NIPD >= iDate)
			//	continue;

			//if(pitem->F_NIPD < iDate)
			//	continue;

			//pitem->F_POWER = GetRemnantsYear(iDate,pitem->F_NIPD);

			{
				if(pitem->F_NIPD>0)
					iLastnipd = pitem->F_NIPD;
				continue;
			}

			int nipd = pitem->F_NIPD;
			if(nipd<=0)
			{
				//刘鹏,2012-7-24,fix bug 8800,11554
				//if(pitem->F_TYPE==0)//本金
					//nipd = pitem->F_YXZZR;
					nipd = iLastnipd;
			}
			iLastnipd = nipd;
			if(nipd < iDate)
			{
				continue;
			}

			pitem->F_POWER = GetRemnantsYear(iDate,nipd);
		}
		m_pSecurity->SetBondCashFlowModified(true);
	}
	//m_iYtmDate = iDate;

	//计算到期收益率 Yield To Maturity----------------------------
	INT retry_num = 0;
	DOUBLE temp;
	DOUBLE low_yield = -1.0;
	DOUBLE high_yield = 1.0;
	DOUBLE yield = 0;//(high_yield + low_yield)/2;
	DOUBLE error_range;
	DOUBLE error_value = 1;

	//------------------------------------------------------------
	if(fPrice < 200)
		error_range = 0.000001;
	else
		error_range = 0.0001;

	//------------------------------------------------------------
	while(error_value > error_range)
	{
		temp = 0;

			yield = (high_yield + low_yield)/2;//20050831

			//----------------------------------------------------
			for(i=0; i<iCashFlowCount; i++)
			{
				BondCashFlowData* pitem = m_pSecurity->GetBondCashFlowDataByIndex(i);
					//[除息日，付息日]之间，本期现金流忽略 +20050803
					if((pitem->F_DIVD > iDate||pitem->F_DIVD == 0) && 
						pitem->F_YXQSR<=iDate && 
						(pitem->F_YXZZR>=iDate ||pitem->F_YXZZR==0))
						temp += pitem->F_CASH *
						        pitem->F_VOLUME /
								pow((double)(1+yield/fxpl), (double)(pitem->F_POWER*fxpl));
				}

			//----------------------------------------------------
			if(temp<0 || retry_num>2000)
			{
				yield = 0;
					break;
			}
			else if(temp > fPrice)
				low_yield = yield;
			else
				high_yield = yield;
			//yield = (high_yield + low_yield)/2;

			//----------------------------------------------------
			error_value = fabs(temp - fPrice);
			retry_num++;
	}

	if(yield<=-0.9999)//2005-10-17
	{
		m_dYTM = Con_doubleInvalid;
		m_dDURATION = Con_doubleInvalid;
		m_dCONVEXITY = Con_doubleInvalid;
		return false;
	}

	m_dYTM = yield;

	//计算债券的持久期,凸性
	temp = 0;
	double temp1=0;
	for(i=0; i<iCashFlowCount; i++)
	{
		BondCashFlowData* pitem = m_pSecurity->GetBondCashFlowDataByIndex(i);

		//[除息日，付息日]之间，本期现金流忽略 +20050803
		if((pitem->F_DIVD > iDate||pitem->F_DIVD == 0) && 
			pitem->F_YXQSR<=iDate && 
			(pitem->F_YXZZR>=iDate ||pitem->F_YXZZR==0))
		{
			//计算债券的持久期
			temp += pitem->F_CASH *  
			pitem->F_VOLUME *
				    pitem->F_POWER /
				    pow((double)(1+yield/fxpl), (double)(pitem->F_POWER*fxpl));

			//计算债券的凸性
			temp1 += pitem->F_CASH * 
					pitem->F_VOLUME *
				    pitem->F_POWER *
					(pitem->F_POWER+1) / 
					pow((double)(1+yield/fxpl), (double)(pitem->F_POWER*fxpl+2));
		}
	}

	if(fabs(temp) > 0)
	{
		m_dDURATION = temp / fPrice;
		m_dMDUR = m_dDURATION/(1+m_dYTM/fxpl);
	}

	if(fabs(temp1) > 0)
		m_dCONVEXITY = temp1 / fPrice;

	return true;
}

bool TxBond::CalcYTM(double& dYtm,double& dDuration,double& dMduration,double& dConvexity,int iSecurityId,int iDate,float fPrice,int iBondCount)
{
	dYtm = Con_doubleInvalid;
	dDuration = Con_doubleInvalid;
	dMduration = Con_doubleInvalid;
	dConvexity = Con_doubleInvalid;

	//有效性判断
	//if(iDate < 19000101   ||
	//   fPrice < 0)
	//   return false;

	//取得指定债券
	SecurityQuotation* pSecurity = GetSecurityNow(iSecurityId);
	if(pSecurity==NULL)
		return false;
	if(pSecurity->IsBond()==false)
		return false;

	//double dInterest = GetInterest(pSecurity,pSecurity->GetCurDataDate());
	//double dInterest = GetInterest(pSecurity,iDate);
	//2012-7-16  应计利息(新)
	double dInterest = GetInterest_New(pSecurity,iDate,true);
	
	if(fPrice<0)
	{
		//需要取得价格
		//并且计算全价
		fPrice = pSecurity->GetClosePrice(iDate,true);
		//if(fPrice<0)
		//	fPrice = pSecurity->GetPreClosePrice(iDate);

		//企业债使用全价交易
		//其他债使用净价交易,2003年之前使用全价交易
		//if(pSecurity->IsBond_Company()==false && dInterest>0)
		if(pSecurity->IsBondNetPrice(iDate)==true && dInterest>0)
			fPrice += (float)dInterest;
	}
	//else
	//{
	//	//公司债使用净价
	//	if(pSecurity->IsBond_Corporation()==true && dInterest>0)
	//		fPrice -= (float)dInterest;
	//}
	//fPrice+=0.0005f;
	//int iUpRaise = 1;
	//int ifRaise = 0;
	//ifRaise = GetDataByScale(fPrice,2,iUpRaise);
	//fPrice = (float)(ifRaise/(float)iUpRaise);

	//取得到期日起
	int iEndDate = GetEndDate(pSecurity);
	if(iEndDate==Con_intInvalid)
		return false;

	//有效性判断
	if( iDate > iEndDate )
	   return false;
	BondNewInfo* pBondNewInfo = pSecurity->GetBondNewInfo();
	if(pBondNewInfo==NULL)
		return false;

	//确定付息频率[每年几次]
	double fxpl;
	if(iBondCount == 1)
	{
		//应该以指标方式提供
		fxpl = (double)pBondNewInfo->pay_interest_frequence;
		if(fxpl>0)
		{
			fxpl = 12/fxpl;
		}
		else
		{
			fxpl = 1;
		}
	}
	else
		fxpl = 1;
	//取得指定债券的现金流数据
	//取得指定债券的现金流数据的数量
	int iCashFlowCount = pSecurity->GetBondCashFlowDataCount();

	//类型
	int iFigureMethod = 0;//GetFigureMethod(m_pSecurity);
	iFigureMethod = 5;

	int i = 0;
	if(m_bCalcYTMoneDay== false || pSecurity->GetBondCashFlowModified()==false)
	{
		int iLastnipd = 0;
		//现金流折现修正----------------------------------------------
		for(i=0; i<iCashFlowCount; i++)
		{
			BondCashFlowData* pitem = pSecurity->GetBondCashFlowDataByIndex(i);
			if(pitem==NULL)
				continue;
			//[除息日，付息日]之间，本期现金流忽略 +20050803
			if(pitem->F_DIVD > 0 && pitem->F_DIVD <= iDate
				&& pitem->F_NIPD >= iDate)
			{
				if(pitem->F_NIPD>0)
					iLastnipd = pitem->F_NIPD;
				continue;
			}

			int nipd = pitem->F_NIPD;
			if(nipd<=0)
			{
				if(pitem->F_TYPE==0)//本金
				{
					//nipd = pitem->F_YXZZR;
					nipd = iLastnipd;
					pitem->F_NIPD = nipd;
				}
			}
			iLastnipd = nipd;
			if(nipd < iDate)
			{
				continue;
			}

			pitem->F_POWER = GetRemnantsYear(iDate,nipd);
#ifdef _DEBUG
			GlobalWatch::_GetInstance()->WatchHere(_T("zhaohongjun|| 现金流折现i=%d,cash=%.4f,power=%.4f,raw_cash=%.4f,fxpl=%.4f,lipd=%d,nipd=%d,divd=%d,volume=%.4f,yxqsr=%d,yxzzr=%d,type=%d,year=%d,bbdate=%d"),
				i,
					pitem->F_CASH,			//01 实付现金:8
					pitem->F_POWER,			//02 累计幂[支付日-交易日]:8
					pitem->F_RAW_CASH,		//03 每期应付现金:8
					pitem->F_FXPL,			//04 付息频率[每年几次]:8
					pitem->F_LIPD,			//05 起息日期==为了模版类的使用方便:4
					pitem->F_NIPD,			//06 付息日期:4
					pitem->F_DIVD,			//07 除息日期:4
					pitem->F_VOLUME,		//08 持股数量:8
					pitem->F_YXQSR,			//09 有效起始日:4
					pitem->F_YXZZR,			//09 有效终止日:4
					pitem->F_TYPE,			//00 0=本金 2008-01-08:1
					pitem->F_YEAR,			//,F_YEAR --付息年度int:4
					pitem->F_BOND_BOOK_DATE	//,F_BONG_BOOK_DATE --债权登记日intpublic:4
				);
#endif
		}
		pSecurity->SetBondCashFlowModified(true);
	}

	//计算到期收益率 Yield To Maturity----------------------------
	int retry_num = 0;
	double temp;
	double low_yield = -1.0;
	double high_yield = 1.0;
	double yield = 0;//(high_yield + low_yield)/2;
	double error_range;
	double error_value = 1;

	//2008-11-13
	int iMaxTryNumber = 2000;

	if(fPrice < 200)
		error_range = 0.000001;
	else
		error_range = 0.0001;

	while(error_value > error_range)
	{
		temp = 0;

		yield = (high_yield + low_yield)/2;//20050831

		for(i=0; i<iCashFlowCount; i++)
		{
			BondCashFlowData* pitem = pSecurity->GetBondCashFlowDataByIndex(i);
			if(pitem==NULL)
				continue;
			//[除息日，付息日]之间，本期现金流忽略 +20050803
			if((pitem->F_DIVD > iDate||pitem->F_DIVD == 0) && 
				pitem->F_YXQSR<=iDate && 
				(pitem->F_YXZZR>=iDate ||pitem->F_YXZZR==0))
			{
				temp += pitem->F_CASH *
					pitem->F_VOLUME /
					pow((double)(1+yield/fxpl), (double)(pitem->F_POWER*fxpl));
#ifdef _DEBUG
				GlobalWatch::_GetInstance()->WatchHere(_T("zhaohongjun|| 现金流temp i=%d,cash=%.4f,power=%.4f,raw_cash=%.4f,fxpl=%.4f,lipd=%d,nipd=%d,divd=%d,volume=%.4f,yxqsr=%d,yxzzr=%d,type=%d,year=%d,bbdate=%d"),
					i,
					pitem->F_CASH,			//01 实付现金:8
					pitem->F_POWER,			//02 累计幂[支付日-交易日]:8
					pitem->F_RAW_CASH,		//03 每期应付现金:8
					pitem->F_FXPL,			//04 付息频率[每年几次]:8
					pitem->F_LIPD,			//05 起息日期==为了模版类的使用方便:4
					pitem->F_NIPD,			//06 付息日期:4
					pitem->F_DIVD,			//07 除息日期:4
					pitem->F_VOLUME,		//08 持股数量:8
					pitem->F_YXQSR,			//09 有效起始日:4
					pitem->F_YXZZR,			//09 有效终止日:4
					pitem->F_TYPE,			//00 0=本金 2008-01-08:1
					pitem->F_YEAR,			//,F_YEAR --付息年度int:4
					pitem->F_BOND_BOOK_DATE	//,F_BONG_BOOK_DATE --债权登记日intpublic:4
					);
#endif
			}
		}

		if(temp<0 || retry_num>iMaxTryNumber)
		{
			yield = 0;
			break;
		}
		else if(temp > fPrice)
			low_yield = yield;
		else
			high_yield = yield;
#ifdef _DEBUG
		GlobalWatch::_GetInstance()->WatchHere(_T("zhaohongjun|| [%s-%s-%d]temp=%.6f,price=%.6f,yield=%.6f"),
			pSecurity->GetName(),
			pSecurity->GetCode(true),
			pSecurity->GetId(),
			temp,
			fPrice,
			yield
			);
#endif

		error_value = fabs(temp - fPrice);
		retry_num++;
	}

	if(yield<=-0.9999)//2005-10-17
	{
		return false;
	}

	dYtm = yield;

	//计算债券的持久期,凸性
	temp = 0;
	double temp1=0;
	for(i=0; i<iCashFlowCount; i++)
	{
		BondCashFlowData* pitem = pSecurity->GetBondCashFlowDataByIndex(i);
			if(pitem==NULL)
				continue;

		//[除息日，付息日]之间，本期现金流忽略 +20050803
		if((pitem->F_DIVD > iDate||pitem->F_DIVD == 0) && 
			pitem->F_YXQSR<=iDate && 
			(pitem->F_YXZZR>=iDate ||pitem->F_YXZZR==0))
		{
			//计算债券的持久期
			temp += pitem->F_CASH *  
			pitem->F_VOLUME *
				    pitem->F_POWER /
				    pow((double)(1+yield/fxpl), (double)(pitem->F_POWER*fxpl));

			//计算债券的凸性
			temp1 += pitem->F_CASH * 
					pitem->F_VOLUME *
				    pitem->F_POWER *
					(pitem->F_POWER+1) / 
					pow((double)(1+yield/fxpl), (double)(pitem->F_POWER*fxpl+2));
		}
	}

	if(fabs(temp) > 0)
	{
		dDuration = temp / fPrice;
		dMduration = dDuration/(1+dYtm/fxpl);
	}

	if(fabs(temp1) > 0)
		dConvexity = temp1 / fPrice;
	return true;
}
bool TxBond::CalcNewBondYTM(
		double& dYtm,
		double& dDuration,
		double& dMduration,
		double& dConvexity,
		BondNewInfo* pBondNewInfo,
		DataFileNormal<blk_TxExFile_FileHead,BondCashFlowData>* pCashFlow,
		int iDate,
		float fPrice,
		int iBondCount//iBondCount>1表示组合测算
		)
{
	dYtm = Con_doubleInvalid;
	dDuration = Con_doubleInvalid;
	dMduration = Con_doubleInvalid;
	dConvexity = Con_doubleInvalid;

	if(iBondCount==1)
	{
		if(pBondNewInfo==NULL)
			return false;
	}
	if(pCashFlow==NULL)
		return false;

	if(!(fPrice>0))
		return false;

	//取得到期日起
	int iEndDate = Con_intInvalid;
	if(iBondCount==1)
		iEndDate = pBondNewInfo->end_date;
	else
		iEndDate = 20301231;
//#define MIN_DATE				19010101//最小日期	
//#define MAX_DATE    			20301231//最大日期

	if(iEndDate==Con_intInvalid)
		return false;

	//有效性判断
	if( iDate > iEndDate )
	   return false;
	//确定付息频率[每年几次]
	double fxpl;
	if(iBondCount == 1)
	{
		//应该以指标方式提供
		fxpl = (double)pBondNewInfo->pay_interest_frequence;
		if(fxpl>0)
		{
			fxpl = 12/fxpl;
		}
		else
		{
			fxpl = 1;
		}
	}
	else
		fxpl = 1;
	//取得指定债券的现金流数据
	//取得指定债券的现金流数据的数量
	int iCashFlowCount = pCashFlow->GetDataCount();

	//类型
	int iFigureMethod = 0;//GetFigureMethod(m_pSecurity);
	iFigureMethod = 5;

	int i = 0;
	int iLastnipd = 0;
	//现金流折现修正----------------------------------------------
	for(i=0; i<iCashFlowCount; i++)
	{
		BondCashFlowData* pitem = pCashFlow->GetDataByIndex(i);
			if(pitem==NULL)
				continue;
		//[除息日，付息日]之间，本期现金流忽略 +20050803
		if(pitem->F_DIVD > 0 && pitem->F_DIVD <= iDate
			&& pitem->F_NIPD >= iDate)
		//	continue;

		//if(pitem->F_NIPD < iDate)
		//	continue;

		//pitem->F_POWER = GetRemnantsYear(iDate,pitem->F_NIPD,true);
			{
				if(iBondCount==1)
				{
					if(pitem->F_NIPD>0)
						iLastnipd = pitem->F_NIPD;
				}
				continue;
			}

			int nipd = pitem->F_NIPD;
			if(iBondCount==1)
			{
				if(nipd<=0)
				{
					if(pitem->F_TYPE==0)//本金
						nipd = iLastnipd;
				}
				iLastnipd = nipd;
			}
			if(nipd < iDate)
			{
				continue;
			}

			if(nipd==iEndDate)
			{
				if(pBondNewInfo->pay_interest_frequence<=0)
				{
					//主要处理贴现债的剩余期限
					COleDateTime eDt(nipd/10000,(nipd%10000)/100,(nipd%10000)%100,0,0,0);
					eDt -= COleDateTimeSpan(1,0,0,0);
					nipd = atoi(eDt.Format(_T("%Y%m%d")));
				}
			}
			pitem->F_POWER = GetRemnantsYear(iDate,nipd,true);
TRACE(_T("\nDIVD=%d,LIPD=%d,NIPD=%d[%d],cash=%.6f,volume=%.6f,power=%.6f"),
					pitem->F_DIVD,
					pitem->F_LIPD,
					pitem->F_NIPD,nipd,
					//pitem->F_YXQSR,
					//pitem->F_YXZZR,
					pitem->F_CASH,
					pitem->F_VOLUME,
					pitem->F_POWER
	  );
	}

	//计算到期收益率 Yield To Maturity----------------------------
	int retry_num = 0;
	double temp;
	double low_yield = -1.0;
	double high_yield = 1.0;
	double yield = 0;//(high_yield + low_yield)/2;
	double error_range;
	double error_value = 1;

	if(fPrice < 200)
		error_range = 0.000001;
	else
		error_range = 0.0001;

	while(error_value > error_range)
	{
		temp = 0;

			yield = (high_yield + low_yield)/2;//20050831

			for(i=0; i<iCashFlowCount; i++)
			{
				BondCashFlowData* pitem = pCashFlow->GetDataByIndex(i);
			if(pitem==NULL)
				continue;

					//2008-05-28
					//参照老系统处理
					if(pitem->F_DIVD > 0 && pitem->F_DIVD <= iDate
						&& pitem->F_NIPD >= iDate)
							continue;
					if(pitem->F_NIPD >= iDate)
						temp += pitem->F_CASH *
						        pitem->F_VOLUME /
								pow((double)(1+yield/fxpl), (double)(pitem->F_POWER*fxpl));

/*
					//[除息日，付息日]之间，本期现金流忽略 +20050803
					if((pitem->F_DIVD > iDate||pitem->F_DIVD == 0) && 
						pitem->F_YXQSR<=iDate && 
						(pitem->F_YXZZR>=iDate ||pitem->F_YXZZR==0))
						temp += pitem->F_CASH *
						        pitem->F_VOLUME /
								pow((double)(1+yield/fxpl), (double)(pitem->F_POWER*fxpl));*/
				}

			if(temp<0 || retry_num>2000)
			{
				yield = 0;
					break;
			}
			else if(temp > fPrice)
				low_yield = yield;
			else
				high_yield = yield;

			error_value = fabs(temp - fPrice);
			retry_num++;
	}

	if(yield<=-0.9999)//2005-10-17
	{
		return false;
	}

	dYtm = yield;

	//计算债券的持久期,凸性
	temp = 0;
	double temp1=0;
	for(i=0; i<iCashFlowCount; i++)
	{
		BondCashFlowData* pitem = pCashFlow->GetDataByIndex(i);
			if(pitem==NULL)
				continue;

		////[除息日，付息日]之间，本期现金流忽略 +20050803
		//if((pitem->F_DIVD > iDate||pitem->F_DIVD == 0) && 
		//	pitem->F_YXQSR<=iDate && 
		//	(pitem->F_YXZZR>=iDate ||pitem->F_YXZZR==0))

		//2008-05-28
		//参照老系统处理
		if(pitem->F_DIVD > 0 && pitem->F_DIVD <= iDate
			&& pitem->F_NIPD >= iDate)
				continue;
		if(pitem->F_NIPD >= iDate)
		{
			//计算债券的持久期
			temp += pitem->F_CASH *  
			pitem->F_VOLUME *
				    pitem->F_POWER /
				    pow((double)(1+yield/fxpl), (double)(pitem->F_POWER*fxpl));

			//计算债券的凸性
			temp1 += pitem->F_CASH * 
					pitem->F_VOLUME *
				    pitem->F_POWER *
					(pitem->F_POWER+1) / 
					pow((double)(1+yield/fxpl), (double)(pitem->F_POWER*fxpl+2));
		}
	}

	if(fabs(temp) > 0)
	{
		dDuration = temp / fPrice;
		dMduration = dDuration/(1+dYtm/fxpl);
	}

	if(fabs(temp1) > 0)
		dConvexity = temp1 / fPrice;
	return true;
}

double	TxBond::Get_YTM(int iSecurityId,int iDate,double dPrice,int iBondCount)
{
	if(Calc1(iSecurityId,iDate)==false)
		return Con_doubleInvalid;

	GetSecurityNow(iSecurityId);
	BondNewInfo* pBondNewInfo = m_pSecurity->GetBondNewInfo();
	if(pBondNewInfo==NULL)
		return false;
	//确定付息频率[每年几次]
	double fxpl;
	if(iBondCount == 1)
	{
		// m_pCash_flow->F_FXPL;
		//应该以指标方式提供
		//fxpl = (float)12/(float)GetPayInterstFrequency(m_pSecurity);
		fxpl = (double)pBondNewInfo->pay_interest_frequence;
		if(fxpl<0)
		{
			fxpl = 0;
		}
		else
		{
			fxpl = 12/fxpl;
		}
	}
	else
		fxpl = 1;

	float fPrice = 0;
	if(dPrice<0)
	{
		fPrice = m_pSecurity->GetClosePrice(iDate);
		if(!(fPrice>0))
			fPrice = m_pSecurity->GetPreClosePrice(iDate);
	}
	else
		fPrice = (float)dPrice;

	//企业债使用全价交易
	//其他债使用净价交易,2003年之前使用全价交易
	if(m_pSecurity->IsBond_Company()==false)
	{
		//double dInterest = GetInterest(m_pSecurity,iDate);
		//2012-7-16  应计利息(新)
		double dInterest = GetInterest_New(m_pSecurity,iDate,true);
		if(fPrice>0 && dInterest>0)
		fPrice += (float)dInterest;
	}

	//取得指定债券的现金流数据
	//取得指定债券的现金流数据的数量
	int iCashFlowCount = m_pSecurity->GetBondCashFlowDataCount();

	//计算到期收益率 Yield To Maturity----------------------------
	INT retry_num = 0;
	DOUBLE temp;
	DOUBLE low_yield = -1.0;
	DOUBLE high_yield = 1.0;
	DOUBLE yield = 0;//(high_yield + low_yield)/2;
	DOUBLE error_range;
	DOUBLE error_value = 1;

	//------------------------------------------------------------
	if(fPrice < 200)
		error_range = 0.000001;
	else
		error_range = 0.0001;

	int i = 0;
	//------------------------------------------------------------
	while(error_value > error_range)
	{
		temp = 0;

			yield = (high_yield + low_yield)/2;//20050831

			//----------------------------------------------------
			for(i=0; i<iCashFlowCount; i++)
			{
				BondCashFlowData* pitem = m_pSecurity->GetBondCashFlowDataByIndex(i);
			if(pitem==NULL)
				continue;
					//[除息日，付息日]之间，本期现金流忽略 +20050803
					if((pitem->F_DIVD > iDate||pitem->F_DIVD == 0) && 
						pitem->F_YXQSR<=iDate && 
						(pitem->F_YXZZR>=iDate ||pitem->F_YXZZR==0))
						temp += pitem->F_CASH *
						        pitem->F_VOLUME /
								pow((double)(1+yield/fxpl), (double)(pitem->F_POWER*fxpl));
				}

			//----------------------------------------------------
			if(temp<0 || retry_num>2000)
			{
				yield = 0;
					break;
			}
			else if(temp > fPrice)
				low_yield = yield;
			else
				high_yield = yield;
			//yield = (high_yield + low_yield)/2;

			//----------------------------------------------------
			error_value = fabs(temp - fPrice);
			retry_num++;
	}

	if(yield<=-0.9999)//2005-10-17
	{
		m_dYTM = Con_doubleInvalid;
		//m_dDURATION = Con_doubleInvalid;
		//m_dCONVEXITY = Con_doubleInvalid;
		return Con_doubleInvalid;
	}

	m_dYTM = yield;
	return m_dYTM;
};//		{ return m_dYTM; }
bool	TxBond::Get_MDURATION_CONVEXITY(int iSecurityId,int iDate,double& dDuration,double& dMduration,double& dConvexity,int iBondCount)
{
	m_dMDUR = Con_doubleInvalid;
	m_dDURATION = Con_doubleInvalid;
	m_dCONVEXITY = Con_doubleInvalid;

	if(Calc1(iSecurityId,iDate)==false)
		return false;

	GetSecurityNow(iSecurityId);
	BondNewInfo* pBondNewInfo = m_pSecurity->GetBondNewInfo();
	if(pBondNewInfo==NULL)
		return false;
	//确定付息频率[每年几次]
	double fxpl;
	if(iBondCount == 1)
	{
		// m_pCash_flow->F_FXPL;
		//应该以指标方式提供
		//fxpl = (float)12/(float)GetPayInterstFrequency(m_pSecurity);
		fxpl = (double)pBondNewInfo->pay_interest_frequence;
		if(fxpl<0)
		{
			fxpl = 0;
		}
		else
		{
			fxpl = 12/fxpl;
		}
	}
	else
		fxpl = 1;

	float fPrice = m_pSecurity->GetClosePrice(iDate);
	if(!(fPrice>0))
		fPrice = m_pSecurity->GetPreClosePrice(iDate);

	//企业债使用全价交易
	//其他债使用净价交易,2003年之前使用全价交易
	if(m_pSecurity->IsBond_Company()==false)
	{
		//double dInterest = GetInterest(m_pSecurity,iDate);
		//2012-7-16  应计利息(新)
		double dInterest = GetInterest_New(m_pSecurity,iDate,true);
		if(fPrice>0 && dInterest>0)
		fPrice += (float)dInterest;
	}
	//取得指定债券的现金流数据
	//取得指定债券的现金流数据的数量
	int iCashFlowCount = m_pSecurity->GetBondCashFlowDataCount();
	//计算债券的持久期,凸性
	double temp = 0;
	double temp1=0;
	double yield = 0;
	int i = 0;
	for(i=0; i<iCashFlowCount; i++)
	{
		BondCashFlowData* pitem = m_pSecurity->GetBondCashFlowDataByIndex(i);
			if(pitem==NULL)
				continue;

		//[除息日，付息日]之间，本期现金流忽略 +20050803
		if((pitem->F_DIVD > iDate||pitem->F_DIVD == 0) && 
			pitem->F_YXQSR<=iDate && 
			(pitem->F_YXZZR>=iDate ||pitem->F_YXZZR==0))
		{
			//计算债券的持久期
			temp += pitem->F_CASH *  
			pitem->F_VOLUME *
				    pitem->F_POWER /
				    pow((double)(1+yield/fxpl), (double)(pitem->F_POWER*fxpl));

			//计算债券的凸性
			temp1 += pitem->F_CASH * 
					pitem->F_VOLUME *
				    pitem->F_POWER *
					(pitem->F_POWER+1) / 
					pow((double)(1+yield/fxpl), (double)(pitem->F_POWER*fxpl+2));
		}
	}

	if(fabs(temp) > 0)
	{
		m_dDURATION = temp / fPrice;
		m_dMDUR = m_dDURATION/(1+m_dYTM/fxpl);
	}

	if(fabs(temp1) > 0)
		m_dCONVEXITY = temp1 / fPrice;

	dDuration = m_dDURATION;
	dMduration = m_dMDUR;
	dConvexity = m_dCONVEXITY;
	return true;
};//	{ return m_dDURATION; }
float TxBond::Get_PriceByYTM(int iSecurityId,float fYtm,int iDate,int iBondCount)
{
	if(CalcPriceByYtm(iSecurityId,fYtm,iDate,iBondCount)==false)
		return Con_floatInvalid;
	return m_fPRICE; 
}

float	TxBond::Get_PriceByYTM(BondNewInfo* pBondNewInfo,DataFileNormal<blk_TxExFile_FileHead,BondCashFlowData>* pCashFlow,float fYtm,int iDate,int iBondCount)
{
	if(CalcNewBondPriceByYtm(pBondNewInfo,pCashFlow,fYtm,iDate,iBondCount)==false)
		return Con_floatInvalid;
	return m_fPRICE; 
}

bool TxBond::CalcPriceByYtm(int iSecurityId,float fYtm,int iDate,int iBondCount)
{
	//初始化
	m_dYTM = Con_doubleInvalid;
	m_dMDUR = Con_doubleInvalid;
	m_dDURATION = Con_doubleInvalid;
	m_dCONVEXITY = Con_doubleInvalid;

	//有效性判断
	if(iDate < 19000101 )
	   return false;

	//取得指定债券
	GetSecurityNow(iSecurityId);
	if(m_pSecurity==NULL)
		return false;
	if(m_pSecurity->IsBond()==false)
		return false;
	int iBondCashFlowCount = m_pSecurity->GetBondCashFlowDataCount();
	BondNewInfo* pBondNewInfo = m_pSecurity->GetBondNewInfo();

	//取得到期日起
	int iEndDate = GetEndDate(m_pSecurity);
	if(iEndDate==Con_intInvalid)
		return false;

	if(iBondCount < 1 || iDate > iEndDate)
	   return false;

	//确定付息频率[每年几次]
	double fxpl;
	if(iBondCount == 1)
	{
		// m_pCash_flow->F_FXPL;
		//应该以指标方式提供
		if(pBondNewInfo==NULL)
			fxpl = 0;
		else
		{
			//fxpl = (float)12/(float)GetPayInterstFrequency(m_pSecurity);
			fxpl = pBondNewInfo->pay_interest_frequence;
			if(fxpl>0)
			{
				fxpl = 12/fxpl;
			}
			else
			{
				fxpl = 0;
			}
		}
	}
	else
		fxpl = 1;

	//取得指定债券的现金流数据
	//取得指定债券的现金流数据的数量
	int iCashFlowCount = m_pSecurity->GetBondCashFlowDataCount();

	//类型
	int iFigureMethod = GetFigureMethod(m_pSecurity);
	iFigureMethod = 5;

	int i = 0;
	int iLastnipd = 0;
	//现金流折现修正----------------------------------------------
	for(i=0; i<iCashFlowCount; i++)
	{
		BondCashFlowData* pitem = m_pSecurity->GetBondCashFlowDataByIndex(i);
			if(pitem==NULL)
				continue;

		//[除息日，付息日]之间，本期现金流忽略 +20050803
		if(pitem->F_DIVD > 0 && pitem->F_DIVD <= iDate
			&& pitem->F_NIPD >= iDate)
		//	continue;

		//if(pitem->F_NIPD < iDate)
		//	continue;

		//pitem->F_POWER = GetRemnantsYear(iDate,pitem->F_NIPD);
			{
				if(pitem->F_NIPD>0)
					iLastnipd = pitem->F_NIPD;
				continue;
			}

			int nipd = pitem->F_NIPD;
			if(nipd<=0)
			{
				if(pitem->F_TYPE==0)//本金
					//nipd = pitem->F_YXZZR;
					nipd = iLastnipd;
			}
			iLastnipd = nipd;
			if(nipd < iDate)
			{
				continue;
			}

			pitem->F_POWER = GetRemnantsYear(iDate,nipd);

	}

	if(fYtm<=-1)//2005-10-17
	{
		m_dYTM = Con_doubleInvalid;
		m_dDURATION = Con_doubleInvalid;
		m_dCONVEXITY = Con_doubleInvalid;
		return false;
	}

	//计算折现价格------------------------------------------------
	double temp = 0;
	for(i=0; i<iCashFlowCount; i++)
	{
		BondCashFlowData* pitem = m_pSecurity->GetBondCashFlowDataByIndex(i);
			if(pitem==NULL)
				continue;
			//[除息日，付息日]之间，本期现金流忽略 +20050803
/*			if(pitem->F_DIVD > 0 && pitem->F_DIVD <= pytm->F_DATE
				&& pitem->F_NIPD >= pytm->F_DATE)
				continue;

			//if(pitem->F_NIPD >= pytm->F_DATE)//2005-10-14
					if((pitem->F_NIPD >= pytm->F_DATE||pitem->F_NIPD==0) && 
						pitem->F_YXQSR<=pytm->F_DATE && 
						(pitem->F_YXZZR>=pytm->F_DATE ||pitem->F_YXZZR==0))*/

			if((pitem->F_DIVD > iDate || pitem->F_DIVD == 0) && 
				pitem->F_YXQSR <= iDate && 
				(pitem->F_YXZZR >= iDate || pitem->F_YXZZR==0))
				temp += pitem->F_CASH /
				        pow((double)(1+fYtm/fxpl), (double)(pitem->F_POWER*fxpl));
	}

	m_fPRICE = Con_floatInvalid;
	if(fabs(temp) > 0.000001)
		m_fPRICE = (float)temp;
/*
	//计算债券的持久期--------------------------------------------
	temp = 0;
	for(i=0; i<iCashFlowCount; i++)
	{
		BondCashFlowData* pitem = m_pSecurity->GetBondCashFlowDataByIndex(i);
			//[除息日，付息日]之间，本期现金流忽略 +20050803
			if((pitem->F_DIVD > iDate|| pitem->F_DIVD == 0) && 
				pitem->F_YXQSR <= iDate && 
				(pitem->F_YXZZR >= iDate || pitem->F_YXZZR == 0))
				temp += pitem->F_CASH * 
					    pitem->F_POWER /
					    pow((1+fYtm/fxpl), pitem->F_POWER*fxpl);
	}

	if(fabs(temp) > 0.000001)
	{
		m_dDURATION = temp / m_fPRICE;
		m_dMDUR = m_dDURATION/(1+fYtm/fxpl);
	}

	//计算债券的凸性----------------------------------------------
	temp = 0;
	for(i=0; i<iCashFlowCount; i++)
	{
		BondCashFlowData* pitem = m_pSecurity->GetBondCashFlowDataByIndex(i);
			//[除息日，付息日]之间，本期现金流忽略 +20050803
			if((pitem->F_DIVD > iDate || pitem->F_DIVD == 0) && 
				pitem->F_YXQSR <= iDate && 
				(pitem->F_YXZZR >= iDate || pitem->F_YXZZR ==0 ))
				temp += pitem->F_CASH * 
					    pitem->F_POWER *
						(pitem->F_POWER+1) / 
						pow((1+fYtm/fxpl), pitem->F_POWER*fxpl+2);
		}

	if(fabs(temp) > 0.000001)
		m_dCONVEXITY = temp / m_fPRICE;
	*/
	return true;
}
bool TxBond::CalcNewBondPriceByYtm(
		BondNewInfo* pBondNewInfo,//in,模拟债券的基本信息
		DataFileNormal<blk_TxExFile_FileHead,BondCashFlowData>* pCashFlow,//in,模拟债券的现金流
		float fYtm,
		int iDate,
		int iBondCount
		)
{
	//初始化
	m_dYTM = Con_doubleInvalid;
	m_dMDUR = Con_doubleInvalid;
	m_dDURATION = Con_doubleInvalid;
	m_dCONVEXITY = Con_doubleInvalid;

	//有效性判断
	if(iDate < 19000101 )
	   return false;

	//取得指定债券
	if(pBondNewInfo==NULL)
		return false;
	if(pCashFlow==NULL)
		return false;
	int iBondCashFlowCount = pCashFlow->GetDataCount();
	//取得到期日起
	int iEndDate = pBondNewInfo->end_date;
	if(iEndDate==Con_intInvalid)
		return false;

	if(iBondCount < 1 || iDate > iEndDate)
	   return false;

	//确定付息频率[每年几次]
	double fxpl;
	if(iBondCount == 1)
	{
		// m_pCash_flow->F_FXPL;
		//应该以指标方式提供
		if(pBondNewInfo==NULL)
			fxpl = 0;
		else
		{
			//fxpl = (float)12/(float)GetPayInterstFrequency(m_pSecurity);
			fxpl = pBondNewInfo->pay_interest_frequence;
			if(fxpl>0)
			{
				fxpl = 12/fxpl;
			}
			else
			{
				fxpl = 1;
			}
		}
	}
	else
		fxpl = 1;

	//取得指定债券的现金流数据
	//取得指定债券的现金流数据的数量
	int iCashFlowCount = pCashFlow->GetDataCount();

	//类型
	int iFigureMethod =0;// GetFigureMethod(m_pSecurity);
	iFigureMethod = 5;

	int i = 0;
	int iLastnipd = 0;
	//现金流折现修正----------------------------------------------
	for(i=0; i<iCashFlowCount; i++)
	{
		BondCashFlowData* pitem = pCashFlow->GetDataByIndex(i);
			if(pitem==NULL)
				continue;

		//[除息日，付息日]之间，本期现金流忽略 +20050803
		if(pitem->F_DIVD > 0 && pitem->F_DIVD <= iDate
			&& pitem->F_NIPD >= iDate)
		//	continue;

		//if(pitem->F_NIPD < iDate)
		//	continue;

		//pitem->F_POWER = GetRemnantsYear(iDate,pitem->F_NIPD,true);
			{
				if(pitem->F_NIPD>0)
					iLastnipd = pitem->F_NIPD;
				continue;
			}

			int nipd = pitem->F_NIPD;
			if(nipd<=0)
			{
				if(pitem->F_TYPE==0)//本金
					//nipd = pitem->F_YXZZR;
					nipd = iLastnipd;
			}
			iLastnipd = nipd;
			if(nipd < iDate)
			{
				continue;
			}

			pitem->F_POWER = GetRemnantsYear(iDate,nipd,true);

	}

	if(fYtm<=-1)//2005-10-17
	{
		m_dYTM = Con_doubleInvalid;
		m_dDURATION = Con_doubleInvalid;
		m_dCONVEXITY = Con_doubleInvalid;
		return false;
	}

	//计算折现价格------------------------------------------------
	double temp = 0;
	for(i=0; i<iCashFlowCount; i++)
	{
		BondCashFlowData* pitem = pCashFlow->GetDataByIndex(i);
			if(pitem==NULL)
				continue;
			////[除息日，付息日]之间，本期现金流忽略 +20050803
			//if((pitem->F_DIVD > iDate || pitem->F_DIVD == 0) && 
			//	pitem->F_YXQSR <= iDate && 
			//	(pitem->F_YXZZR >= iDate || pitem->F_YXZZR==0))

			//2008-05-28
			//参照老系统处理=仅针对新债测算
			if(pitem->F_DIVD > 0 && pitem->F_DIVD <= iDate
				&& pitem->F_NIPD >= iDate)
					continue;
			if(pitem->F_NIPD >= iDate)
				temp += pitem->F_CASH /
				        pow((double)(1+fYtm/fxpl), (double)(pitem->F_POWER*fxpl));
	}

	m_fPRICE = Con_floatInvalid;
	if(fabs(temp) > 0.000001)
		m_fPRICE = (float)temp;
/*
	//计算债券的持久期--------------------------------------------
	temp = 0;
	for(i=0; i<iCashFlowCount; i++)
	{
		BondCashFlowData* pitem = m_pSecurity->GetBondCashFlowDataByIndex(i);
			//[除息日，付息日]之间，本期现金流忽略 +20050803
			if((pitem->F_DIVD > iDate|| pitem->F_DIVD == 0) && 
				pitem->F_YXQSR <= iDate && 
				(pitem->F_YXZZR >= iDate || pitem->F_YXZZR == 0))
				temp += pitem->F_CASH * 
					    pitem->F_POWER /
					    pow((1+fYtm/fxpl), pitem->F_POWER*fxpl);
	}

	if(fabs(temp) > 0.000001)
	{
		m_dDURATION = temp / m_fPRICE;
		m_dMDUR = m_dDURATION/(1+fYtm/fxpl);
	}

	//计算债券的凸性----------------------------------------------
	temp = 0;
	for(i=0; i<iCashFlowCount; i++)
	{
		BondCashFlowData* pitem = m_pSecurity->GetBondCashFlowDataByIndex(i);
			//[除息日，付息日]之间，本期现金流忽略 +20050803
			if((pitem->F_DIVD > iDate || pitem->F_DIVD == 0) && 
				pitem->F_YXQSR <= iDate && 
				(pitem->F_YXZZR >= iDate || pitem->F_YXZZR ==0 ))
				temp += pitem->F_CASH * 
					    pitem->F_POWER *
						(pitem->F_POWER+1) / 
						pow((1+fYtm/fxpl), pitem->F_POWER*fxpl+2);
		}

	if(fabs(temp) > 0.000001)
		m_dCONVEXITY = temp / m_fPRICE;
	*/
	return true;
}
//取得转债底价
double TxBond::GetFloor(int iSecurityId)
{
	SecurityQuotation* pSecurity = GetSecurityNow(iSecurityId);
	if(pSecurity==NULL)
		return Con_doubleInvalid;
	BondChangePF* pBondChangePF = NULL;
	if(pSecurity->GetGlobalData(gdt_BondChangePF,&pBondChangePF)==false)
		return Con_doubleInvalid;
	return pBondChangePF->dFloor;
}

//计算转债底价
double TxBond::CalcFloor(int iSecurityId,int iDate,bool bIsNetPrice,double left_years)
{
	//2008-03-24
	//return GetFloor(iSecurityId);

	SecurityQuotation* pSecurity = GetSecurityNow(iSecurityId);
	if(pSecurity==NULL)
		return 0;

	CArray<BondCompnySamples> sampleList;
	if(GetCompnyBondByLeftYear(sampleList,left_years,iDate)==false)
		return 0;

	int count = (int)sampleList.GetCount();

	double* pbuf = new double[count*2];
	if(pbuf==NULL)
		return 0;

	//填充数据
	int ibuf = 0;
	for(int i=0;i<count;i++)
	{
		pbuf[ibuf++]=sampleList[i].left_year;
		pbuf[ibuf++]=sampleList[i].ytm;
	}

	double result = 0;

	//count /= 2;
	Tx::Core::YTM_curve *pytm = new Tx::Core::YTM_curve;
	double a1,a2,param[6];
	if(pytm!=NULL)
	{
		if(pytm->create_YTM_curve(pbuf,count,7,&a1,&a2,param,NULL) == 0)
		{
			double year = GetRemnantsYear(iDate,GetEndDate(pSecurity));
			if(pytm->get_YTM_frm_curve(param,a1,a2,year,&result) == 0)
			{
				m_fPRICE = 0;
				CalcPriceByYtm(iSecurityId,(float)result,iDate,1);
				result = m_fPRICE;
			}
		}
		delete pytm;
	}
	delete pbuf;

	//如果需要全价，则加上应计利息
	if(bIsNetPrice==false)
	{
		//2008-01-18
		//double dInterest = GetInterest(pSecurity,iDate);
		//2012-7-16  应计利息(新)
		double dInterest = GetInterest_New(pSecurity,iDate,true);
		if(dInterest>0)
			result += dInterest;
	}

	return result;
}
//****************************************************************
// 取得某日剩余期限在n年以内的沪深交易所企业债品种代码和剩余期限
//1、拟合YTM曲线的企业债代码和剩余期限
//2、剩余年限
//3、日期
//****************************************************************
bool TxBond::GetCompnyBondByLeftYear(CArray<BondCompnySamples>& sampleList, double left_years, int iDate)
{
//	if(left_years <= 0)
//		return false;

	Tx::Core::Table_Indicator* pTable = m_pLogicalBusiness->GetSecurityTable(3);
	if(pTable==NULL)
		return false;

	sampleList.RemoveAll();

	int iRowCount = (int)pTable->GetRowCount();
	for(int i=0;i<iRowCount;i++)
	{
		BondCompnySamples bcs;

		//取得样本
		int iBondId = 0;
		pTable->GetCell(0,i,iBondId);
		SecurityQuotation* pSecurity = (SecurityQuotation*)GetSecurity(iBondId);
		if(pSecurity==NULL)
			continue;

		//是否企业债
		if(pSecurity->IsBond_Company()==false)
			continue;
		BondNewInfo* pBondNewInfo = pSecurity->GetBondNewInfo();
		if(pBondNewInfo==NULL)
			continue;

		if(iDate < MIN_JYS_QBOND_LIST_DATE || iDate > m_pFunctionDataManager->GetServerCurDateTime(pSecurity->GetInnerBourseId()).GetDate().GetInt())
			iDate = m_pFunctionDataManager->GetServerCurDateTime(pSecurity->GetInnerBourseId()).GetDate().GetInt();

		//30001211		上市日期
		int iListeddate = 0;//pSecurity->GetIndicateIntValue(30001211);
		iListeddate = pBondNewInfo->ipo_date;
		if(iListeddate<=0 || iListeddate>iDate)
			continue;

		//30001123 记息方式=3,浮
		//int iInterestType = pSecurity->GetIndicateIntValue(30001123);
		int iInterestType = pBondNewInfo->interest_type;
		//是否浮息债
		if(iInterestType<0 || iInterestType==3)
			continue;

		//付息频率
		int iFxpl = pBondNewInfo->pay_interest_frequence;// pSecurity->GetIndicateIntValue(30001125);

		//只保留每年付息和到期一次
		if(iFxpl<12)
			continue;

		//到期日期
		int iEndDate = 0;//pSecurity->GetIndicateIntValue(30001128);
		iEndDate = pBondNewInfo->end_date;
		//剩余期限
		double year = GetRemnantsYear(iDate,iEndDate);
		if(year>1)
		{
			//取得企业债价格
			//float fPrice = pSecurity->GetClosePrice(iDate);
			//Calc(pSecurity->GetId(),iDate,fPrice);
			bcs.left_year = year;
			bcs.ytm = Get_YTM(pSecurity->GetId(),iDate);
			sampleList.Add(bcs);
		}
	}
	return true;
}
/*
//****************************************************************
// 取可转债某日对应的转股价格-20050318
// date = 0 时返回初始转股价格
//****************************************************************
FUN_EXT_API(double) On_TXB_Get_ZBond_Convert_Price(LPCTSTR pcode, long date)
{
	double fRet=NULL;
	CString sql_str;
	CTKzzZgjg *ptable = new CTKzzZgjg;
	if(ptable == NULL)
		return fRet;

	ptable->On_Get_Table_Name();

	if(date == 0)//初始转股价格
		sql_str.Format("SELECT TOP 1 * FROM %s WHERE F_ID = \'%s\' ORDER BY F_SXRQ", \
			ptable->m_Table_name, pcode);
	else
		sql_str.Format("SELECT TOP 1 * FROM %s WHERE F_ID = \'%s\' AND F_SXRQ <= \'%d\' \
			ORDER BY F_SXRQ DESC",ptable->m_Table_name, pcode, date);

	if(ptable->On_Get_Data(sql_str) == TRUE)
	{
		if(ptable->m_Record_count > 0)
		{
			T_KZZ_ZGJG* prec = (T_KZZ_ZGJG*)*(ptable->m_ppRecord);
			fRet = prec->F_ZGJG;
		}
	}
	delete ptable;

	if(fRet < FLOAT_NULL)
		return NULL;
	else
		return fRet;
}
*/
//计算转债平价
double TxBond::CalcParity(int iSecurityId,int iDate)
{
	SecurityQuotation* pSecurity = GetSecurityNow(iSecurityId);
	if(pSecurity==NULL)
		return 0;
	//转股价格
	double convertprice = 0;
	/*
	int iIndicatorId = 30301170;
	Tx::Core::Table_Indicator resTable;
	if(pSecurity->GetIndicatorValueAbsoluteTable(iIndicatorId,resTable)==false)
		return 0;

	if(resTable.GetColCount()<3) return 0;

	//按照生效日期排序-降序
	resTable.Sort(1,false);
	int count = (int)resTable.GetRowCount();
	for(int i=0;i<count;i++)
	{
		int date = 0;
		resTable.GetCell(1,i,date);
		if(date==Con_intInvalid)
			continue;
		if(iDate==0 && i==0)
			iDate = date;
		if(date<=iDate)
		{
			resTable.GetCell(2,i,convertprice);
			break;
		}
	}
	if(!(convertprice>0))
		return 0;
	*/
/*
//可转债转股价格
//取得指定索引的转股价格数据
BondChangePrice*	Security::GetBondChangePriceByIndex(int index = 0);
	{
		BondChangePrice* pData = NULL;
		if(GetDataByIndex(dt_BondChangePrice,index,&pData)==false)
			return NULL;
		return pData;
	}
//取得指定日期的转股价格数据
BondChangePrice*	Security::GetBondChangePriceByDate(int date = 0);
	{
		BondChangePrice* pData = NULL;
		if(GetDataByObj(dt_BondChangePrice,date,&pData)==false)
			return NULL;
		return pData;
	}
//取得转股价格数据的记录数
int	Security::GetBondChangePriceCount(void);
	{
		return GetDataCount(dt_BondChangePrice);
	}

*/
	BondChangePrice* pData = NULL;
	if(pSecurity->GetDataByObj(Tx::Data::dt_BondChangePrice,iDate,&pData)==false || pData==NULL)
		return 0;

	convertprice = pData->fPrice;
	if(!(convertprice>0))
		return 0;

	//iIndicatorId = 30001140;
	//iSecurityId = pSecurity->GetIndicateIntValue(iIndicatorId);

	/*
	iSecurityId = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_FROM_SECURITYID,iSecurityId);

	SecurityQuotation* pSecurityObj = GetSecurityNow(iSecurityId);
	if(pSecurityObj==NULL)
		return 0;
		*/
	float closeprice;// = pSecurityObj->GetClosePrice(iDate);
	closeprice = (float)GetBondStockPrice(iSecurityId,iDate);
	if(!(closeprice>0))
		return 0;
	return 100/convertprice*closeprice;
}
//计算可转债-转换溢价率
double TxBond::CalcPremium(int iSecurityId,int iDate)
{
	double cp = CalcParity(iSecurityId,iDate);
	SecurityQuotation* pSecurity = GetSecurityNow(iSecurityId);
	if(pSecurity==NULL)
		return Con_doubleInvalid;
	float closeprice = pSecurity->GetClosePrice(iDate);
	return CalcPremium((double)closeprice,cp);

	//if(closeprice<0)
	//	return Con_doubleInvalid;
	//if(fabs(cp-0)<0.000001)
	//	return Con_doubleInvalid;
	//return (closeprice/cp-1);
}
//溢价率
double TxBond::CalcPremium(double dClosePrice,double dPrice)
{
	if(dClosePrice<0)
		return Con_doubleInvalid;
	if(fabs(dPrice-0)<0.000001)
		return Con_doubleInvalid;
	return (dClosePrice/dPrice-1);
}
//========================================================================//by zhangxs 20080807
//取得正股价格    
double TxBond::GetBondStockPrice(int iSecurityId,int iDate)
{
	/*
	int iIndicatorId = 30001140;
	SecurityQuotation* pSecurity = GetSecurityNow(iSecurityId);
	if(pSecurity==NULL)
		return Con_doubleInvalid;


	iSecurityId = pSecurity->GetIndicateIntValue(iIndicatorId);
	SecurityQuotation* pSecurityObj = GetSecurityNow(iSecurityId);
	if(pSecurityObj==NULL)
		return Con_doubleInvalid;
	return pSecurityObj->GetClosePrice();
*/
	iSecurityId = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_FROM_SECURITYID,iSecurityId);

	SecurityQuotation* pSecurityObj = GetSecurityNow(iSecurityId);
	if(pSecurityObj==NULL)
		return 0;
	if(iDate<=0)
		return pSecurityObj->GetClosePrice();
	else
		return  pSecurityObj->GetClosePrice(iDate);

}
//取得正股涨幅
double TxBond::GetBondStockRaiseRatio(int iSecurityId,int iDate)
{
	/*
	int iIndicatorId = 30001140;
	double m_dRaise = 0.0;
	CString m_strRaise;
	SecurityQuotation* pSecurity = GetSecurityNow(iSecurityId);
	if(pSecurity==NULL)	
		return Con_doubleInvalid;
	iSecurityId = pSecurity->GetIndicateIntValue(iIndicatorId);
	SecurityQuotation* pSecurityObj = GetSecurityNow(iSecurityId);
	if(pSecurityObj==NULL)
		return Con_doubleInvalid;
	return pSecurityObj ->GetRaise();
*/
	iSecurityId = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_FROM_SECURITYID,iSecurityId);
	SecurityQuotation* pSecurityObj = GetSecurityNow(iSecurityId);
	if(pSecurityObj==NULL)
		return Con_doubleInvalid;
	if(iDate<=0)
		return pSecurityObj ->GetRaise();
	else
		return  pSecurityObj ->GetRaise(iDate);
}
//=========================================================================
//排序分析数据结果标题列
bool TxBond::GetBlockAnalysisCol(Table_Display& baTable,std::vector<int>& arrSamples,int& iSortCol)
{
	if(TxBusiness::GetBlockAnalysisCol(baTable,arrSamples,iSortCol)==false)
		return false;

	int nCol = baTable.GetColCount();
	int sCol = 12;
	if(nCol>sCol)
	baTable.DeleteCol(sCol,nCol-sCol);

	nCol = 9;
	//baTable.SetTitle(nCol, _T("现量(手)"));
	baTable.SetPrecise(nCol, 0);
	//baTable.SetOutputItemRatio(nCol, 10);
	//baTable.SetFormatStyle(nCol, Tx::Business::fs_finance);
	++nCol;
	//baTable.SetTitle(nCol, _T("成交量(手)"));
	baTable.SetPrecise(nCol, 0);
	//baTable.SetOutputItemRatio(nCol, 10);
	//baTable.SetFormatStyle(nCol, Tx::Business::fs_finance);
	++nCol;
	//baTable.SetTitle(nCol, _T("成交金额(万元)"));
	baTable.SetPrecise(nCol, 2);
	baTable.SetOutputItemRatio(nCol, 10000);
	//baTable.SetFormatStyle(nCol, Tx::Business::fs_finance);
	++nCol;

	if(m_iBondType == 2)
	{
		return true;
	}

	//11 发行量
	baTable.AddCol(Tx::Core::dtype_double);


	//12 票面利率
	baTable.AddCol(Tx::Core::dtype_double);
	//13 付息频率
	//baTable.AddCol(Tx::Core::dtype_double);
	baTable.AddCol(Tx::Core::dtype_val_string);
	//14 发行年限
	baTable.AddCol(Tx::Core::dtype_double);

	//15 起息日期
	baTable.AddCol(Tx::Core::dtype_int4);
	//16 到期日期
	baTable.AddCol(Tx::Core::dtype_int4);
	//17 剩余期限
	baTable.AddCol(Tx::Core::dtype_val_string);
	//18 计息天数
	baTable.AddCol(Tx::Core::dtype_int4);

	//19 应计利息
	baTable.AddCol(Tx::Core::dtype_double);
	//20 本期起息日
	baTable.AddCol(Tx::Core::dtype_int4);
	//21 本期付息日
	baTable.AddCol(Tx::Core::dtype_int4);
	//22 YTM
	baTable.AddCol(Tx::Core::dtype_double);
	//23 修正久期
	baTable.AddCol(Tx::Core::dtype_double);
	//24 凸性
	baTable.AddCol(Tx::Core::dtype_double);

	//nCol = 11;
	//SetDisplayTableColInfo(&baTable,nCol,30000050,false);
	baTable.SetTitle(nCol, _T("发行规模(亿元)"));
	//小数点
	baTable.SetPrecise(nCol, 2);
	//倍数
	baTable.SetOutputItemRatio(nCol,100000000);
	baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
	++nCol;

	baTable.SetTitle(nCol, _T("票面利率"));
	++nCol;
	baTable.SetTitle(nCol, _T("付息频率"));
	++nCol;
	baTable.SetTitle(nCol, _T("发行年限(年)"));
	baTable.SetPrecise(nCol, 3);
	++nCol;
	baTable.SetTitle(nCol, _T("起息日期"));
	baTable.SetFormatStyle(nCol, Tx::Core::fs_date);
	++nCol;
	baTable.SetTitle(nCol, _T("到期日期"));
	baTable.SetFormatStyle(nCol, Tx::Core::fs_date);
	++nCol;
	baTable.SetTitle(nCol, _T("剩余期限"));
	++nCol;
	baTable.SetTitle(nCol, _T("计息天数"));
	++nCol;
	baTable.SetTitle(nCol, _T("应计利息"));
	baTable.SetPrecise(nCol, 4);
	++nCol;
	baTable.SetTitle(nCol, _T("本期起息日"));
	baTable.SetFormatStyle(nCol, Tx::Core::fs_date);
	++nCol;
	//2008-11-21
	baTable.SetTitle(nCol, _T("本期付息日"));
	baTable.SetFormatStyle(nCol, Tx::Core::fs_date);
	++nCol;
	baTable.SetTitle(nCol, _T("YTM"));
	baTable.SetPrecise(nCol, 3);
	++nCol;
	baTable.SetTitle(nCol, _T("修正久期"));
	++nCol;
	baTable.SetTitle(nCol, _T("凸性"));

	//2009-02-23
	if(m_iBondType == 1)
	{
		//可转债
		//25平价溢价率
		baTable.AddCol(Tx::Core::dtype_double);
		++nCol;
		baTable.SetTitle(nCol, _T("平价溢价率"));
		//26底价溢价率
		baTable.AddCol(Tx::Core::dtype_double);
		++nCol;
		baTable.SetTitle(nCol, _T("底价溢价率"));
		++nCol;

		//正股名称
		baTable.AddCol(Tx::Core::dtype_val_string);
		baTable.SetTitle(nCol, _T("正股名称"));
		++nCol;
		//正股代码
		baTable.AddCol(Tx::Core::dtype_val_string);
		baTable.SetTitle(nCol, _T("正股代码"));
		++nCol;
		//正股价格
		baTable.AddCol(Tx::Core::dtype_double);
		baTable.SetTitle(nCol, _T("正股价格"));
		//小数点
		baTable.SetPrecise(nCol, 2);
		++nCol;

	}
	return true;
}
//排序分析数据结果设置
bool TxBond::SetBlockAnalysisCol(Table_Display& baTable,SecurityQuotation* psq,int& j,int ii)
{
	if(psq==NULL)
		return false;

	if(m_iBondType == 2)
		return true;

	int idate = psq->GetCurDataDate();

	//11 发行量
		baTable.SetCell(j,ii,GetShare(psq));
		j++;
	//12 票面利率
		baTable.SetCell(j,ii,GetParRate(psq));
		j++;
	//13 付息频率
		CString sFrequency;
		sFrequency = GetPayInterstFrequencyString(psq);
		baTable.SetCell(j,ii,sFrequency);
		j++;
	//14 发行年限
		baTable.SetCell(j,ii,GetHoldYear(psq));
		j++;
	//15 起息日期
		baTable.SetCell(j,ii,GetFirstBeginDate(psq));
		j++;
	//16 到期日期
		int iEndDate = GetEndDate(psq);
		baTable.SetCell(j,ii,iEndDate);
		j++;
	//17 剩余期限
		bool bStop = false;
		int iStopedDate = psq->GetStopedLastDate();
		if((iStopedDate>0 && idate>iStopedDate) || (iEndDate>0 && idate>=iEndDate))
		{
			sFrequency = _T("[已到期]");
			bStop = true;
		}
		else
			sFrequency = GetRemnantsString(idate,iEndDate);
		baTable.SetCell(j,ii,sFrequency);
		j++;

	//float closePrice = psq->GetClosePrice(idate,true);
	//float closePrice = psq->GetClosePrice(true);
	//float prePrice = psq->GetPreClosePrice();
	/*
	//11 发行量
		baTable.SetCell(j,ii,GetShare(psq));
		j++;
	//12 票面利率
		baTable.SetCell(j,ii,GetParRate(psq));
		j++;
	//13 付息频率
		CString sFrequency;
		sFrequency = GetPayInterstFrequencyString(psq);
		baTable.SetCell(j,ii,sFrequency);
		j++;
	//14 发行年限
		baTable.SetCell(j,ii,GetHoldYear(psq));
		j++;
	//15 起息日期
		baTable.SetCell(j,ii,GetFirstBeginDate(psq));
		j++;
	//16 到期日期
		int iEndDate = GetEndDate(psq);
		baTable.SetCell(j,ii,iEndDate);
		j++;
	//17 剩余期限
		baTable.SetCell(j,ii,GetRemnantsString(idate,iEndDate));
		j++;
		*/
	if(psq->IsStop()==true || bStop==true)
	{
	//18 计息天数、
		j++;
	//19 应计利息
		j++;
	//20 本期起息日
		j++;
		//
		j++;
	//21 YTM
		j++;
	//22 修正久期
		j++;
	//23 凸性
		j++;
		//2009-02-23
		if(m_iBondType == 1)
		{
			j++;
			j++;
			j++;
		}
	}
	else
	{
	//18 计息天数、
		baTable.SetCell(j,ii,GetInterestDays(psq,idate));
		j++;
	//19 应计利息
		//double dInterest = GetInterest(psq,idate);
		//2012-7-16  应计利息(新)
		double dInterest = GetInterest_New(psq,idate,true);
		baTable.SetCell(j,ii,dInterest);
		j++;
	//20 本期起息日
		int iBeginDate = Con_intInvalid;
		BondCashFlowData* pBondCashFlowData = psq->GetBondCashFlowDataByDate(idate);
		if(pBondCashFlowData!=NULL)
			iBeginDate = pBondCashFlowData->F_LIPD;
		baTable.SetCell(j,ii,iBeginDate);
		j++;
		//
		int nipid = Con_intInvalid;
		if(pBondCashFlowData!=NULL)
			nipid = pBondCashFlowData->F_NIPD;
		if(nipid<=0)
			nipid=iEndDate;
		//if(nipid>0)
		//{
		//	Tx::Core::TxDateTime nipdDate(nipid);
		//	nipdDate -= ZkkLib::TimeSpan(1,0,0,0);
		//	nipid = nipdDate.GetDate().GetInt();
		//}
		//else
		//	nipid = Con_intInvalid;
		baTable.SetCell(j,ii,nipid);
		j++;

		//2008-02-01
		double dYtm;
		//double dDuration;
		double dMduration;
		double dConvexity;
		//CalcYTM(dYtm,dDuration,dMduration,dConvexity,psq->GetId(),idate);
		GetBondYTM(psq->GetId(),dYtm,dMduration,dConvexity);

	//21 YTM
		if(!(fabs(dYtm-Con_doubleInvalid)<0.000001))
			dYtm *= 100;
		baTable.SetCell(j,ii,dYtm);
		j++;
	//22 修正久期
		baTable.SetCell(j,ii,dMduration);
		j++;
	//23 凸性
		baTable.SetCell(j,ii,dConvexity);
		j++;
		//2009-02-23
		if(m_iBondType == 1)
		{
			//24
			int iSid = psq->GetId();
			float fClosePrice;
			baTable.GetCell(5,ii,fClosePrice);
			double dParity = CalcParity(iSid,idate);
			double dParityR;
			dParityR = CalcPremium(fClosePrice,dParity);

			if(fabs(dParityR-Con_doubleInvalid)>0.000001)
				dParityR*=100;
			baTable.SetCell(j,ii,dParityR);
			j++;

			//25
			double dFloor = GetFloor(iSid);
			double dFloorR = CalcPremium(fClosePrice,dFloor);
			if(fabs(dFloorR-Con_doubleInvalid)>0.000001)
				dFloorR*=100;
			baTable.SetCell(j,ii,dFloorR);
			j++;

			int iObjSecurityId = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_FROM_SECURITYID,psq->GetId());
			SecurityQuotation* pSecurityObj = GetSecurityNow(iObjSecurityId);

			if(pSecurityObj!=NULL)
				baTable.SetCell(j,ii,pSecurityObj->GetName());
			j++;
			if(pSecurityObj!=NULL)
				baTable.SetCell(j,ii,pSecurityObj->GetCode());
			j++;
			baTable.SetCell(j,ii,GetBondStockPrice(psq->GetId()));
			j++;
		}
	}
	return true;
}
//排序分析基本数据结果设置[换手率]
bool TxBond::SetBlockAnalysisHslCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow)
{
	if(pSecurity!=NULL)
	{
		if(pSecurity->IsStop()==true)
			return true;
	}
	double dHsl = Con_doubleInvalid;
	//dHsl = pSecurity->GetTradeRate();
	//换手率
	baTable.SetCell(nCol,nRow,dHsl);
	return true;
}
//板块=排序分析
//baTable已经填充了样本[交易实体]
bool TxBond::GetBlockAnalysis(Table_Display& baTable,std::vector<int>& arrSamples,int& iSortCol,int iMarketId,bool bNeedProgressWnd,int nCollID)
{
	//2009-02-23

//50010130:可转债
//
//50010125：全部回购
//50010126：沪市回购
//50010127：深市回购
//
//
//最新可转债
//50012529	可转债
//50012551	可转债
//50012555	分离式可转债
//50012559	可转债
//50012563	分离式可转债
//50012753	分离式可转债
//50012774	可转债
//50012775	分离式可转债

	if(nCollID == 50010130
	   || nCollID == 50012529	// 可转债
	   || nCollID == 50012551	// 可转债
	   || nCollID == 50012555	// 分离式可转债
	   || nCollID == 50012753	// 分离式可转债
	   || nCollID == 50012774	// 可转债
	   || nCollID == 50012775	// 分离式可转债
		)
		SetBondType(1);
	//2009-05-14
	else if(
		nCollID == 50010125 ||
		nCollID == 50010126 ||
		nCollID == 50010127
		)
		SetBondType(2);
	else
		SetBondType(0);

	TxBusiness::GetBlockAnalysis(baTable,arrSamples,iSortCol,iMarketId,bNeedProgressWnd,nCollID);
	//换手率
	baTable.DeleteCol(8);
	return true;
}

//2008-01-28
//交易提示
bool TxBond::GetTradeInfo(std::vector<int> iSecurityId,Table_Display& baTable,int iStartDate,int iEndDate,bool bAllDate,int flag)
{
	int icount = (int)iSecurityId.size();
	if(icount<=0)
		return false;

	baTable.Clear();
	//0 交易实体ID
	baTable.AddCol(Tx::Core::dtype_int4);
	//1 名称
	baTable.AddCol(Tx::Core::dtype_val_string);
	//2 代码
	baTable.AddCol(Tx::Core::dtype_val_string);
	//2.0类型
	baTable.AddCol(Tx::Core::dtype_val_string);
	//2.1 分配方案
	baTable.AddCol(Tx::Core::dtype_val_string);
	//3 实付现金=付息金额
	baTable.AddCol(Tx::Core::dtype_double);
	//4 付息年度
	baTable.AddCol(Tx::Core::dtype_int4);
	//5 登记日期
	baTable.AddCol(Tx::Core::dtype_int4);
	//6 计息日期
	baTable.AddCol(Tx::Core::dtype_int4);
	//7 付息日期
	baTable.AddCol(Tx::Core::dtype_int4);
	//8 除息日期
	baTable.AddCol(Tx::Core::dtype_int4);
	//9 备注
	baTable.AddCol(Tx::Core::dtype_val_string);

	int nCol = 7;
	baTable.SetFormatStyle(nCol++, Tx::Core::fs_date);
	baTable.SetFormatStyle(nCol++, Tx::Core::fs_date);
	baTable.SetFormatStyle(nCol++, Tx::Core::fs_date);
	baTable.SetFormatStyle(nCol++, Tx::Core::fs_date);

	//初始化进度条
	//step1
//	Tx::Core::ProgressWnd* pProgressWnd = Tx::Core::ProgressWnd::GetInstance();
	Tx::Core::ProgressWnd prw;
	//step2
	CString sProgressPrompt;
	sProgressPrompt =  _T("交易提示");
	UINT progId = prw.AddItem(1,sProgressPrompt, 0.0);
	//step3
	prw.Show(15);
	prw.EnableCancelButton(true);

	CString str;
	CString sFx;
	sFx = _T("付息");
	int count = 0;
	for(int ii=0;ii<icount;ii++)
	{
		//step4 进度条位置
		prw.SetPercent(progId, (double)ii/(double)icount);
		if(prw.IsCanceled()==true)
		{
			prw.SetPercent(progId,1.0);
			prw.EnableCancelButton(false);
			baTable.DeleteRow(0,baTable.GetRowCount());
			return false;
		}
		SecurityQuotation* pSecurity = (SecurityQuotation*)GetSecurityNow(iSecurityId[ii]);
		if(pSecurity==NULL)
			continue;

		//最新信息
		BondNewInfo* pBondNewInfo = pSecurity->GetBondNewInfo();
		if(pBondNewInfo==NULL)
		{
			CString sLog;
			sLog.Format(_T("[%d-%d][%d]债券交易提示,该债券[%s-%s-%d]最新基本信息无效"),
				pSecurity->GetServerCurDate(),
				pSecurity->GetServerCurDateTime().GetTime().GetInt(),
				pSecurity->GetInnerBourseId(),
				pSecurity->GetName(),
				pSecurity->GetCode(true),
				pSecurity->GetId()
				);
			Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);

			continue;
		}
		int date = 0;
		//还本
		date = GetEndDate(pSecurity);
		if(bAllDate==true || date>=iStartDate && date<=iEndDate)
		{
			//还本
			count = (int)baTable.GetRowCount();
			baTable.AddRow();
			nCol = 0;
			//0
			baTable.SetCell(nCol,count,iSecurityId[ii]);
			nCol++;
			//1
			baTable.SetCell(nCol,count,pSecurity->GetName());
			nCol++;
			//2
			baTable.SetCell(nCol,count,pSecurity->GetCode());
			nCol++;
			str=_T("还本");
			//2.0
			baTable.SetCell(nCol,count,str);
			nCol++;
			//2.1
			str.Format(_T("%.4f"),pBondNewInfo->par_val);
			baTable.SetCell(nCol,count,str);
			nCol++;
			//3 实付现金
			// 付息金额
			baTable.SetCell(nCol,count,Con_doubleInvalid);
			nCol++;
			//4 付息年度
			baTable.SetCell(nCol,count,Con_intInvalid);
			nCol++;
			//5 登记日期
			baTable.SetCell(nCol,count,Con_intInvalid);
			nCol++;
			//6 计息日期
			baTable.SetCell(nCol,count,Con_intInvalid);
			nCol++;
			//7 付息日期
			baTable.SetCell(nCol,count,Con_intInvalid);
			nCol++;
			//8 除息日期
			baTable.SetCell(nCol,count,Con_intInvalid);
			nCol++;
			//9 备注
			str.Format(_T("到期日∶ %s 面值∶ %.2f元"),
				DisplayColInfo::GetDateString(date),
				pBondNewInfo->par_val
				);
			baTable.SetCell(nCol,count,str);
			nCol++;
		}

		switch(flag)
		{
		case 0:
		//flag==0 起息日期
			date = pBondNewInfo->begin_date;
			break;
		case 1:
		//flag==1 付息日期
			date = pBondNewInfo->ipo_issue_date;
			break;
		case 2:
		//flag==2 除息日期
			date = pBondNewInfo->ipo_issue_date;
			break;
		case 3:
		//flag==3 登记日期
			date = pBondNewInfo->ipo_issue_date;
			break;
		default:
			break;
		}

		if(bAllDate==true || date>=iStartDate && date<=iEndDate)
		{
			//发行
			count = (int)baTable.GetRowCount();
			baTable.AddRow();
			nCol = 0;
			//0
			baTable.SetCell(nCol,count,iSecurityId[ii]);
			nCol++;
			//1
			baTable.SetCell(nCol,count,pSecurity->GetName());
			nCol++;
			//2
			baTable.SetCell(nCol,count,pSecurity->GetCode());
			nCol++;
			str=_T("发行");
			//2.0
			baTable.SetCell(nCol,count,str);
			nCol++;
			//2.1
			str=_T("-");
			baTable.SetCell(nCol,count,str);
			nCol++;
			//3 实付现金
			// 付息金额
			baTable.SetCell(nCol,count,Con_doubleInvalid);
			nCol++;
			//4 付息年度
			baTable.SetCell(nCol,count,Con_intInvalid);
			nCol++;
			//5 登记日期
			baTable.SetCell(nCol,count,Con_intInvalid);
			nCol++;
			//6 计息日期
			baTable.SetCell(nCol,count,Con_intInvalid);
			nCol++;
			//7 付息日期
			baTable.SetCell(nCol,count,Con_intInvalid);
			nCol++;
			//8 除息日期
			baTable.SetCell(nCol,count,Con_intInvalid);
			nCol++;
			//9 备注
			str.Format(_T("发行日∶ %s 发行量∶ %.2f亿元 面值∶ %.2f元"),
				DisplayColInfo::GetDateString(pBondNewInfo->ipo_issue_date),
				pBondNewInfo->share/100000000,
				pBondNewInfo->par_val
				);
			baTable.SetCell(nCol,count,str);
			nCol++;
		}

		switch(flag)
		{
		case 0:
		//flag==0 起息日期
			date = pBondNewInfo->begin_date;
			break;
		case 1:
		//flag==1 付息日期
			date = pBondNewInfo->ipo_date;
			break;
		case 2:
		//flag==2 除息日期
			date = pBondNewInfo->ipo_date;
			break;
		case 3:
		//flag==3 登记日期
			date = pBondNewInfo->ipo_date;
			break;
		default:
			break;
		}
		if(bAllDate==true || pBondNewInfo->ipo_issue_date>=iStartDate && pBondNewInfo->ipo_issue_date<=iEndDate)
		{
			//上市
			count = (int)baTable.GetRowCount();
			baTable.AddRow();
			nCol = 0;
			//0
			baTable.SetCell(nCol,count,iSecurityId[ii]);
			nCol++;
			//1
			baTable.SetCell(nCol,count,pSecurity->GetName());
			nCol++;
			//2
			baTable.SetCell(nCol,count,pSecurity->GetCode());
			nCol++;
			str=_T("上市");
			//2.0
			baTable.SetCell(nCol,count,str);
			nCol++;
			//2.1
			str=_T("-");
			baTable.SetCell(nCol,count,str);
			nCol++;
			//3 实付现金
			// 付息金额
			baTable.SetCell(nCol,count,Con_doubleInvalid);
			nCol++;
			//4 付息年度
			baTable.SetCell(nCol,count,Con_intInvalid);
			nCol++;
			//5 登记日期
			baTable.SetCell(nCol,count,Con_intInvalid);
			nCol++;
			//6 计息日期
			baTable.SetCell(nCol,count,Con_intInvalid);
			nCol++;
			//7 付息日期
			baTable.SetCell(nCol,count,Con_intInvalid);
			nCol++;
			//8 除息日期
			baTable.SetCell(nCol,count,Con_intInvalid);
			nCol++;
			//9 备注
			str.Format(_T("上市日∶ %s 发行日∶ %s 发行量∶ %.2f亿元 面值∶ %.2f元"),
				DisplayColInfo::GetDateString(pBondNewInfo->ipo_date),
				DisplayColInfo::GetDateString(pBondNewInfo->ipo_issue_date),
				pBondNewInfo->share/100000000,
				pBondNewInfo->par_val
				);
			baTable.SetCell(nCol,count,str);
			nCol++;
		}

		//从现金流数据中取得付息数据
		int iBCFD_count = pSecurity->GetBondCashFlowDataCount();
		for(int i=0;i<iBCFD_count;i++)
		{
			if(prw.IsCanceled()==true)
			{
				prw.SetPercent(progId,1.0);
				prw.EnableCancelButton(false);
				baTable.DeleteRow(0,baTable.GetRowCount());
				return false;
			}
			BondCashFlowData*	pBondCashFlowData = pSecurity->GetBondCashFlowDataByIndex(i);
			if(pBondCashFlowData==NULL)
				continue;
			//本金
			if(pBondCashFlowData->F_TYPE == 0)
				continue;
			date = 0;
			switch(flag)
			{
			case 0:
			//flag==0 起息日期
				date = pBondCashFlowData->F_LIPD;
				break;
			case 1:
			//flag==1 付息日期
				date = pBondCashFlowData->F_NIPD;
				break;
			case 2:
			//flag==2 除息日期
				date = pBondCashFlowData->F_DIVD;
				break;
			case 3:
			//flag==3 登记日期
				date = pBondCashFlowData->F_BOND_BOOK_DATE;
				break;
			default:
				break;
			}
			if(date==0)
				continue;

			if(bAllDate==true || date>=iStartDate && date<=iEndDate)
			{
				count = (int)baTable.GetRowCount();
				baTable.AddRow();
				nCol = 0;
				//0
				baTable.SetCell(nCol,count,iSecurityId[ii]);
				nCol++;
				//1
				baTable.SetCell(nCol,count,pSecurity->GetName());
				nCol++;
				//2
				baTable.SetCell(nCol,count,pSecurity->GetCode());
				nCol++;
				double dVolume = Con_doubleInvalid;
				if(pBondNewInfo!=NULL && pBondNewInfo->share && pBondNewInfo->par_val>0 && pBondCashFlowData->F_CASH>0)
				{
					dVolume = pBondCashFlowData->F_CASH*pBondNewInfo->share/pBondNewInfo->par_val/10000;
				}
				//2.0
				baTable.SetCell(nCol,count,sFx);
				nCol++;
				//2.1
				str=_T("-");
				if(pBondNewInfo!=NULL && pBondNewInfo->par_val>0 && pBondCashFlowData->F_CASH>0)
				{
					//str.Format(_T("%.0f ∶ %.4f"),pBondNewInfo->par_val,pBondCashFlowData->F_CASH);
					str.Format(_T("%.4f"),pBondCashFlowData->F_CASH);
				}
				baTable.SetCell(nCol,count,str);
				nCol++;
				//3 实付现金
				// 付息金额
				baTable.SetCell(nCol,count,dVolume);
				nCol++;
				//4 付息年度
				baTable.SetCell(nCol,count,pBondCashFlowData->F_YEAR);
				nCol++;
				//5 登记日期
				baTable.SetCell(nCol,count,pBondCashFlowData->F_BOND_BOOK_DATE);
				nCol++;
				//6 计息日期
				baTable.SetCell(nCol,count,pBondCashFlowData->F_LIPD);
				nCol++;
				//7 付息日期
				baTable.SetCell(nCol,count,pBondCashFlowData->F_NIPD);
				nCol++;
				//8 除息日期
				baTable.SetCell(nCol,count,pBondCashFlowData->F_DIVD);
				nCol++;
				//9 备注
				str.Format(_T("付息日∶ %s 付息年度∶ %d 付息∶ %.4f元"),
					DisplayColInfo::GetDateString(pBondCashFlowData->F_NIPD),
					pBondCashFlowData->F_YEAR,
					pBondCashFlowData->F_CASH
					);
				baTable.SetCell(nCol,count,str);
				nCol++;
			}
		}
	}
	//step5
	prw.SetPercent(progId, 1.0);
	sProgressPrompt += _T(",完成!");
	prw.SetText(progId,sProgressPrompt);
	prw.EnableCancelButton(false);
	return true;
}
//2008-05-15
//取得债券付息方式int=记息方式1:单利,2:复利,3:浮动利率,4:累进利率,5:贴现
CString TxBond::GetInterestTypeName(int iSecurityId)
{
	SecurityQuotation* pSecurity = GetSecurityNow(iSecurityId);
	return GetInterestTypeName(pSecurity);
}
CString TxBond::GetInterestTypeName(SecurityQuotation* pSecurity)
{
	if(pSecurity->IsBond()==false)
		return Con_strInvalid;
	BondNewInfo* pBondNewInfo = pSecurity->GetBondNewInfo();
	if(pBondNewInfo==NULL)
		return Con_strInvalid;
	CString sType;
	switch(pBondNewInfo->interest_type)
	{
	case 1:
		sType = _T("单利");
		break;
	case 2:
		sType = _T("复利");
		break;
	case 3:
		sType = _T("浮动利率");
		break;
	case 4:
		sType = _T("累进利率");
		break;
	case 5:
		sType = _T("贴现");
		break;
	case 6:
		sType = _T("附息债");
		break;
	default:
		sType = Con_strInvalid;
		break;
	}
	return sType;
}

//****************************************************************
//	取得下一期的付息日期和付息现金流
//****************************************************************
int TxBond::GetCashFlow(BondNewInfo* pBondNewInfo,int date_lipd/*起息日*/,double dInterest, float* pcash_flow)
{
	*pcash_flow = 0;

	//------------------------------------------------------------
	int date_nipd = Con_intInvalid;
	if(pBondNewInfo == NULL)
		return date_nipd;
	else if(date_lipd > pBondNewInfo->end_date)
	{
		date_nipd = pBondNewInfo->end_date;
		return date_nipd;
	}

	//债券利率----------------------------------------------------
	double interset_rate;
	switch(pBondNewInfo->interest_type)//	--付息方式int=记息方式1:单利,2:复利,3:浮动利率,4:累进利率,5:贴现
	{
	case 3://浮息券
		{
			interset_rate = pBondNewInfo->par_rate;
			//int iIndex = Tx::Data::BasicInterest::GetInstance()->GetIndexByDate(date_lipd,8,1);
			//Tx::Data::BasicInterestData* pBIData = Tx::Data::BasicInterest::GetInstance()->GetInByIndex(iIndex);
			//if(pBIData!=NULL)
				//interset_rate += pBIData->dInterest;
			if (dInterest > 0 )
			{
				interset_rate += dInterest;
			}
		}
				break;

			case 1://附息券
			case 2://附息券
			case 4://附息券
			case 6://附息券
				interset_rate = pBondNewInfo->par_rate;
				break;

			case 5://贴现券
				interset_rate = 0;
				break;

			default://DD
				return Con_intInvalid;
		}

	//------------------------------------------------------------
	if(pBondNewInfo->pay_interest_frequence > 0)
	{
		date_nipd = OffsetMonth(date_lipd,pBondNewInfo->pay_interest_frequence);

		//取得付息日期[推前一天]------------------------------
			//date_nipd = On_TXR_Get_Date_Offset(date_lipd, NULL, pBondNewInfo->pay_interest_frequence, -1);

			//取得付息现金流--------------------------------------
			*pcash_flow = (float)(pBondNewInfo->par_val *
		                          interset_rate * 
						          pBondNewInfo->pay_interest_frequence/12/100);
	}
	else
	{
		//一次性还本付息--------------------------------------
			//date_nipd = On_TXR_Get_Date_Offset(pBondNewInfo->end_date, 0, 0, -1);
		if(pBondNewInfo->end_date>0)
		{
			ZkkLib::DateTime dt(pBondNewInfo->end_date);
			dt -= ZkkLib::TimeSpan(-1,0,0,0);
			date_nipd = dt.GetDate().GetInt();
		}
			
			//取得付息现金流--------------------------------------
			*pcash_flow = (float)(pBondNewInfo->par_val *
		                          interset_rate *
								  pBondNewInfo->hold_year/100);
	}

	//------------------------------------------------------------
	return date_nipd;
}
int TxBond::OffsetMonth(int date,int iMonth)
{
		//ZkkLib::DateTime dt;
		ZkkLib::Date dt;
		dt.SetInt(date);
		//ZkkLib::DateTime dt(date);
		int iaddYear = iMonth/12;
		int iaddMonth = iMonth%12;

		int year = dt.GetYear();
		year += iaddYear;
		int month = dt.GetMonth();
		month += iaddMonth;

		int day = dt.GetDay();

		if(month==2)
		{
			if(ZkkLib::Date::IsLeapYear(year)==FALSE)
			{
				if(day>28)
					day = 28;
			}
			else
			{
				if(day>29)
					day = 29;
			}
		}
		dt.SetDate(year,month,day);
		//return dt.GetDate().GetInt();
		return dt.GetInt();
}
bool TxBond::CreateCashFlow(DataFileNormal<blk_TxExFile_FileHead,BondCashFlowData>* pCashFlow,BondNewInfo* pBondNewInfo,DataFileNormal<blk_TxExFile_FileHead,NEW_BOND_INTEREST>* pNewBondInterest,int start_date,bool bIsNew)
{
	if(pCashFlow==NULL)
		return false;

	pCashFlow->Clear();
	//------------------------------------------------------------
	if(pBondNewInfo == NULL)
		return false;
	if(start_date < 0)
		start_date = m_pFunctionDataManager->GetServerCurDateTime_ShangHai().GetDate().GetInt();

	int  date_lipd = pBondNewInfo->begin_date;//起息日期
	int  date_nipd;//付息日期
	float cash;
	BondCashFlowData	new_item;


	//取得付息频率------------------------------------------------
	float fxpl = pBondNewInfo->pay_interest_frequence>0 ? 12/pBondNewInfo->pay_interest_frequence:1.0f;

	ZkkLib::DateTime edt(pBondNewInfo->end_date);
	edt -= ZkkLib::TimeSpan(1,0,0,0);
	int iyxzzr = edt.GetDate().GetInt();
	//确定预期现金流 [起始日->到期日]-----------------------------
	while(date_lipd < pBondNewInfo->end_date)
	{
		//2008-05-28
		//从基准利率计算应计利息
		double dInterest = 0;
		if(pNewBondInterest!=NULL)
		{
			NEW_BOND_INTEREST* pNEW_BOND_INTEREST = pNewBondInterest->GetDataByObj(date_lipd,true);
			if(pNEW_BOND_INTEREST!=NULL)
				dInterest = pNEW_BOND_INTEREST->dInterest/100;
		}

		date_nipd = GetCashFlow(pBondNewInfo,date_lipd,dInterest, &cash);

		if(bIsNew==true || pBondNewInfo->pay_interest_frequence<=0 || pBondNewInfo->interest_type==5)
			cash*=100;

		if(date_nipd < start_date)
		{
			date_lipd =	date_nipd;
			continue;
		}

		ZkkLib::DateTime dtn(date_nipd);
		dtn -= ZkkLib::TimeSpan(1,0,0,0);
		int inipd = dtn.GetDate().GetInt();

		//加入一条--------------------------------------------
		new_item.F_RAW_CASH = cash;
		new_item.F_CASH = new_item.F_RAW_CASH;
		new_item.F_LIPD = date_lipd;
		new_item.F_NIPD = inipd;//date_nipd;
		new_item.F_FXPL = fxpl;
		new_item.F_VOLUME = 1;
		new_item.F_DIVD = 0; //默认 +20050803
		//new_item.F_POWER = 0;
		new_item.F_YXQSR = pBondNewInfo->begin_date;
		//if(pBondNewInfo->end_date<=0)
			new_item.F_YXZZR = 0;
			new_item.F_TYPE = 1;
		//else
		//	new_item.F_YXZZR = iyxzzr;
		pCashFlow->AddBuf(&new_item);

		ZkkLib::DateTime dt(date_nipd);
		dt += ZkkLib::TimeSpan(1,0,0,0);
		date_lipd = dt.GetDate().GetInt();

		//是否已到最后一期: [还本] ---------------------------
		if(date_lipd >= pBondNewInfo->end_date)
		{
			new_item.F_RAW_CASH = (FLOAT)pBondNewInfo->par_val;
			new_item.F_CASH = new_item.F_RAW_CASH;
			new_item.F_LIPD = inipd;//date_nipd;
			new_item.F_NIPD = inipd;//date_nipd;
			new_item.F_FXPL = fxpl;
			new_item.F_VOLUME = 1;
			new_item.F_DIVD = 0; //默认 +20050803
			//new_item.F_POWER = 0;
			new_item.F_YXQSR = pBondNewInfo->begin_date;
			//if(pBondNewInfo->end_date<=0)
				new_item.F_YXZZR = 0;
			new_item.F_TYPE = 0;
			//else
			//	new_item.F_YXZZR = iyxzzr;
			pCashFlow->AddBuf(&new_item);
		}
		date_lipd =	date_nipd;
	}
	return true;
}
//added by zhangxs  提供给派息统计用
bool TxBond::GetTradeInfoDivideBonus(std::vector<int> iSecurityId,
									 Table_Display& baTable,
									 int iStartDate,
									 int iEndDate,
									 bool bAllDate,
									 int flag)
{
	int icount = (int)iSecurityId.size();
	if(icount<=0)
		return false;

	baTable.Clear();
	//0 交易实体ID
	baTable.AddCol(Tx::Core::dtype_int4);
	//1 名称
	baTable.AddCol(Tx::Core::dtype_val_string);
	//2 代码
	baTable.AddCol(Tx::Core::dtype_val_string);
	//2.1 分配方案
	baTable.AddCol(Tx::Core::dtype_val_string);
	//3 实付现金=付息金额
	baTable.AddCol(Tx::Core::dtype_double);
	//4 付息年度
	baTable.AddCol(Tx::Core::dtype_int4);
	//5 登记日期
	baTable.AddCol(Tx::Core::dtype_int4);
	//6 计息日期
	baTable.AddCol(Tx::Core::dtype_int4);
	//7 付息日期
	baTable.AddCol(Tx::Core::dtype_int4);
	//8 除息日期
	baTable.AddCol(Tx::Core::dtype_int4);

	int nCol = 6;
	baTable.SetFormatStyle(nCol++, Tx::Core::fs_date);
	baTable.SetFormatStyle(nCol++, Tx::Core::fs_date);
	baTable.SetFormatStyle(nCol++, Tx::Core::fs_date);
	baTable.SetFormatStyle(nCol++, Tx::Core::fs_date);

	//初始化进度条
	//step1
	//	Tx::Core::ProgressWnd* pProgressWnd = Tx::Core::ProgressWnd::GetInstance();
	Tx::Core::ProgressWnd prw;
	//step2
	CString sProgressPrompt;
	sProgressPrompt =  _T("交易提示");
	UINT progId = prw.AddItem(1,sProgressPrompt, 0.0);
	//step3
	prw.Show(15);
	prw.EnableCancelButton(true);

	CString str;
	CString sFx;
	sFx = _T("付息");
	int count = 0;
	for(int ii=0;ii<icount;ii++)
	{
		//step4 进度条位置
		prw.SetPercent(progId, (double)ii/(double)icount);
		if(prw.IsCanceled()==true)
		{
			prw.SetPercent(progId,1.0);
			prw.EnableCancelButton(false);
			baTable.DeleteRow(0,baTable.GetRowCount());
			return false;
		}
		SecurityQuotation* pSecurity = (SecurityQuotation*)GetSecurityNow(iSecurityId[ii]);
		if(pSecurity==NULL)
			continue;

		//最新信息
		BondNewInfo* pBondNewInfo = pSecurity->GetBondNewInfo();
		if(pBondNewInfo==NULL)
		{
			CString sLog;
			sLog.Format(_T("[%d-%d][%d]债券交易提示,该债券[%s-%s-%d]最新基本信息无效"),
				pSecurity->GetServerCurDate(),
				pSecurity->GetServerCurDateTime().GetTime().GetInt(),
				pSecurity->GetInnerBourseId(),
				pSecurity->GetName(),
				pSecurity->GetCode(true),
				pSecurity->GetId()
				);
			Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);

			continue;
		}
		int date = 0;
		//switch(flag)
		//{
		//case 0:
		//	//flag==0 起息日期
		//	date = pBondNewInfo->begin_date;
		//	break;
		//case 1:
		//	//flag==1 付息日期
		//	date = pBondNewInfo->ipo_issue_date;
		//	break;
		//case 2:
		//	//flag==2 除息日期
		//	date = pBondNewInfo->ipo_issue_date;
		//	break;
		//case 3:
		//	//flag==3 登记日期
		//	date = pBondNewInfo->ipo_issue_date;
		//	break;
		//default:
		//	break;
		//}
		//if(bAllDate==true || date>=iStartDate && date<iEndDate)
		//{
		//	//发行
		//	count = (int)baTable.GetRowCount();
		//	baTable.AddRow();
		//	nCol = 0;
		//	//0
		//	baTable.SetCell(nCol,count,iSecurityId[ii]);
		//	nCol++;
		//	//1
		//	baTable.SetCell(nCol,count,pSecurity->GetName());
		//	nCol++;
		//	//2
		//	baTable.SetCell(nCol,count,pSecurity->GetCode());
		//	nCol++;
		//	//2.1
		//	str=_T("-");
		//	baTable.SetCell(nCol,count,str);
		//	nCol++;
		//	//3 实付现金
		//	// 付息金额
		//	baTable.SetCell(nCol,count,Con_doubleInvalid);
		//	nCol++;
		//	//4 付息年度
		//	baTable.SetCell(nCol,count,Con_intInvalid);
		//	nCol++;
		//	//5 登记日期
		//	baTable.SetCell(nCol,count,Con_intInvalid);
		//	nCol++;
		//	//6 计息日期
		//	baTable.SetCell(nCol,count,Con_intInvalid);
		//	nCol++;
		//	//7 付息日期
		//	baTable.SetCell(nCol,count,Con_intInvalid);
		//	nCol++;
		//	//8 除息日期
		//	baTable.SetCell(nCol,count,Con_intInvalid);
		//	nCol++;
		//}

		//switch(flag)
		//{
		//case 0:
		//	//flag==0 起息日期
		//	date = pBondNewInfo->begin_date;
		//	break;
		//case 1:
		//	//flag==1 付息日期
		//	date = pBondNewInfo->ipo_date;
		//	break;
		//case 2:
		//	//flag==2 除息日期
		//	date = pBondNewInfo->ipo_date;
		//	break;
		//case 3:
		//	//flag==3 登记日期
		//	date = pBondNewInfo->ipo_date;
		//	break;
		//default:
		//	break;
		//}
		//if(bAllDate==true || pBondNewInfo->ipo_issue_date>=iStartDate && pBondNewInfo->ipo_issue_date<iEndDate)
		//{
		//	//上市
		//	count = (int)baTable.GetRowCount();
		//	baTable.AddRow();
		//	nCol = 0;
		//	//0
		//	baTable.SetCell(nCol,count,iSecurityId[ii]);
		//	nCol++;
		//	//1
		//	baTable.SetCell(nCol,count,pSecurity->GetName());
		//	nCol++;
		//	//2
		//	baTable.SetCell(nCol,count,pSecurity->GetCode());
		//	nCol++;
		//	//2.1
		//	str=_T("-");
		//	baTable.SetCell(nCol,count,str);
		//	nCol++;
		//	//3 实付现金
		//	// 付息金额
		//	baTable.SetCell(nCol,count,Con_doubleInvalid);
		//	nCol++;
		//	//4 付息年度
		//	baTable.SetCell(nCol,count,Con_intInvalid);
		//	nCol++;
		//	//5 登记日期
		//	baTable.SetCell(nCol,count,Con_intInvalid);
		//	nCol++;
		//	//6 计息日期
		//	baTable.SetCell(nCol,count,Con_intInvalid);
		//	nCol++;
		//	//7 付息日期
		//	baTable.SetCell(nCol,count,Con_intInvalid);
		//	nCol++;
		//	//8 除息日期
		//	baTable.SetCell(nCol,count,Con_intInvalid);
		//	nCol++;
		//}

		//从现金流数据中取得付息数据
		int iBCFD_count = pSecurity->GetBondCashFlowDataCount();
		for(int i=0;i<iBCFD_count;i++)
		{
			if(prw.IsCanceled()==true)
			{
				prw.SetPercent(progId,1.0);
				prw.EnableCancelButton(false);
				baTable.DeleteRow(0,baTable.GetRowCount());
				return false;
			}
			BondCashFlowData*	pBondCashFlowData = pSecurity->GetBondCashFlowDataByIndex(i);
			if(pBondCashFlowData==NULL)
				continue;
			//本金
			if(pBondCashFlowData->F_TYPE == 0)
				continue;
			date = 0;
			switch(flag)
			{
			case 0:
				//flag==0 起息日期
				date = pBondCashFlowData->F_LIPD;
				break;
			case 1:
				//flag==1 付息日期
				date = pBondCashFlowData->F_NIPD;
				break;
			case 2:
				//flag==2 除息日期
				date = pBondCashFlowData->F_DIVD;
				break;
			case 3:
				//flag==3 登记日期
				date = pBondCashFlowData->F_BOND_BOOK_DATE;
				break;
			default:
				break;
			}
			if(date==0)
				continue;

			if(bAllDate==true || date>=iStartDate && date<iEndDate)
			{
				count = (int)baTable.GetRowCount();
				baTable.AddRow();
				nCol = 0;
				//0
				baTable.SetCell(nCol,count,iSecurityId[ii]);
				nCol++;
				//1
				baTable.SetCell(nCol,count,pSecurity->GetName());
				nCol++;
				//2
				baTable.SetCell(nCol,count,pSecurity->GetCode());
				nCol++;
				double dVolume = Con_doubleInvalid;
				if(pBondNewInfo!=NULL && pBondNewInfo->share && pBondNewInfo->par_val>0 && pBondCashFlowData->F_CASH>0)
				{
					dVolume = pBondCashFlowData->F_CASH*pBondNewInfo->share/pBondNewInfo->par_val/10000;
				}
				//2.1
				str=_T("-");
				if(pBondNewInfo!=NULL && pBondNewInfo->par_val>0 && pBondCashFlowData->F_CASH>0)
				{
					//str.Format(_T("%.0f : %.4f"),pBondNewInfo->par_val,pBondCashFlowData->F_CASH);
					str.Format(_T("%.0f ∶ %.4f"),pBondNewInfo->par_val,pBondCashFlowData->F_CASH);
				}
				baTable.SetCell(nCol,count,str);
				nCol++;
				//3 实付现金
				// 付息金额
				baTable.SetCell(nCol,count,dVolume);
				nCol++;
				//4 付息年度
				baTable.SetCell(nCol,count,pBondCashFlowData->F_YEAR);
				nCol++;
				//5 登记日期
				baTable.SetCell(nCol,count,pBondCashFlowData->F_BOND_BOOK_DATE);
				nCol++;
				//6 计息日期
				baTable.SetCell(nCol,count,pBondCashFlowData->F_LIPD);
				nCol++;
				//7 付息日期
				baTable.SetCell(nCol,count,pBondCashFlowData->F_NIPD);
				nCol++;
				//8 除息日期
				baTable.SetCell(nCol,count,pBondCashFlowData->F_DIVD);
				nCol++;
			}
		}
	}
	//step5
	prw.SetPercent(progId, 1.0);
	sProgressPrompt += _T(",完成!");
	prw.SetText(progId,sProgressPrompt);
	prw.EnableCancelButton(false);
	return true;
}

//[新]债券YTM、久期、修正久期、凸性[单券]    2013-09-02
void TxBond::GetYTM(SecurityQuotation* pSecurity,int iDate,double& dYtm,double& dDuration,double& dMduration,double& dConvexity)  
{
	dYtm        = Con_doubleInvalid;
	dDuration   = Con_doubleInvalid;
	dMduration  = Con_doubleInvalid;
	dConvexity  = Con_doubleInvalid;
	if(pSecurity == NULL) 
		return;
	BondYTM* pData = pSecurity->GetBondYTMByDate(iDate);
	//YTM取当天数据，如果今天没有数据，则返回 Con_doubleInvalid
	if (pData == NULL || pData->object != iDate)
		return ;

	dYtm        = pData->fYtm;
	dDuration   = pData->fDur;
	dMduration  = pData->fADur;
	dConvexity  = pData->fCon;	
}

//获取实时YTM[单券]
void TxBond::GetYTM(SecurityQuotation* pSecurity,double& dYtm,double& dDuration,double& dMduration,double& dConvexity)
{
	dYtm = Con_doubleInvalid;
	dDuration = Con_doubleInvalid;
	dMduration = Con_doubleInvalid;
	dConvexity = Con_doubleInvalid;

	CString strSvrList;
	strSvrList.Format(_T("%sServerList%d.ini"),Tx::Core::SystemPath::GetInstance()->GetExePath(),Tx::Core::UserInfo::GetInstance()->GetNetId());//
	if (false == Tx::Core::Commonality::File().IsFileExist (strSvrList))
		return;

	char tempch[200];
	memset(tempch,0,200);
	GetPrivateProfileString(_T("File"), _T("BondYTM"), _T(""), tempch, 200, strSvrList);

	CString strUrl;
	int iMarket = 1; //1-sh,2-sz
	if(pSecurity==NULL) return;
	if(pSecurity->IsShanghai())
		iMarket = 1;
	else if(pSecurity->IsShenzhen())
		iMarket = 2;
	else
		return;
	CString strCode = pSecurity->GetCode();
	strUrl.Format(_T("%sBond/BondReal/%d/%s"),tempch,iMarket,strCode);

	Tx::Drive::Http::CSyncDnload gload;
	if (gload.Open(strUrl,NULL,5000))
	{
		CONST Tx::Drive::Mem::MemSlice &dnData = gload.Rsp().Body();
		if (!gload.Rsp().IsSuccess() || dnData.IsEmpty() || dnData.Size()<=1)
		{
			return;
		}

		int nSize = dnData.Size();

		CString strXML = (LPCTSTR)dnData.DataPtr();
		strXML = strXML.Left(nSize);

		OleInitialize(NULL);
		Tx::Data::CXmlDocumentWrapper xmlDoc;
		/*2 读取XML*/
		if (!xmlDoc.LoadXML(strXML))
		{

			TRACE(_T("fail to load xml: %s.\r\n"), strXML);
			OleUninitialize();
			return;
		}

		/*3得到根结点*/
		Tx::Data::CXmlNodeWrapper root(xmlDoc.AsNode());
		if (_T("bondcalcresult") != root.Name().MakeLower())
		{
			OleUninitialize();
			return;
		}

		//获取节点数据
		//YTM
		CXmlNodeWrapper nodeItem = root.FindNode(_T("ytm"));
		if (nodeItem.IsValid())
		{
			dYtm = atof(nodeItem.GetText());
		}
		nodeItem = root.FindNode(_T("duration"));
		if (nodeItem.IsValid())
		{
			dDuration = atof(nodeItem.GetText());
		}
		//修正久期
		nodeItem = root.FindNode(_T("mdDuration"));
		if (nodeItem.IsValid())
		{
			dMduration = atof(nodeItem.GetText());
		}
		//凸性
		nodeItem = root.FindNode(_T("convexity"));
		if (nodeItem.IsValid())
		{
			dConvexity = atof(nodeItem.GetText());
		}

		OleUninitialize();
	}

}

//获取YTM等 [单券单日]
bool TxBond::GetYTM(SecurityQuotation* pSecurity,int iDate,float fPrice, int nPriceType, double& dYtm,double& dDuration,double& dMduration,double& dConvexity)
{
	dYtm = Con_doubleInvalid;
	dDuration = Con_doubleInvalid;
	dMduration = Con_doubleInvalid;
	dConvexity = Con_doubleInvalid;

	CString strSvrList;
	strSvrList.Format(_T("%sServerList%d.ini"),Tx::Core::SystemPath::GetInstance()->GetExePath(),Tx::Core::UserInfo::GetInstance()->GetNetId());//
	if (false == Tx::Core::Commonality::File().IsFileExist (strSvrList))
		return false;

	char tempch[200];
	memset(tempch,0,200);
    GetPrivateProfileString(_T("File"), _T("BondYTM"), _T(""), tempch, 200, strSvrList);

	CString strUrl;
	strUrl.Format(_T("%sBond/BondCalc/Basic"),tempch);

	int iMarket = 1; //1-sh,2-sz
	if(pSecurity==NULL) 
		return false;
	if(pSecurity->IsShanghai())
		iMarket = 1;
	else if(pSecurity->IsShenzhen())
		iMarket = 2;
	else
		return false;
	CString strCode = pSecurity->GetCode();

	CString strHeader = _T("Content-Type: application/xml; charset=utf-8");
	CString strTemp;
	CString strReq = _T("<ArrayOfBondIndicator xmlns=\"www.txsec.com/mb810\">");
	strReq += _T("<BondIndicator>");

	strReq += _T("<code>");
	strReq += strCode;
	strReq += _T("</code>");

	strReq += _T("<market>");
	strTemp.Format(_T("%d"),iMarket);
	strReq += strTemp;
	strReq += _T("</market>");

	strReq += _T("<date>");
	strTemp.Format(_T("%d"),iDate);
	strReq += strTemp;
	strReq += _T("</date>");

	strReq += _T("<priceFlag>");
	strTemp.Format(_T("%d"),nPriceType);
	strReq += strTemp;
	strReq += _T("</priceFlag>");

	strReq += _T("<price>");
	strTemp.Format(_T("%f"),fPrice);
	strReq += strTemp;
	strReq += _T("</price>");

	strReq += _T("</BondIndicator>");
	strReq += _T("</ArrayOfBondIndicator>");

	//Post请求
	int iSize = strReq.GetLength();
	LPBYTE pBuffer = new BYTE [iSize];
	if (pBuffer == NULL) 
		return false;
	memcpy_s(pBuffer,iSize,strReq.GetBuffer(),iSize);
	strReq.ReleaseBuffer(iSize);

	Tx::Drive::Http::CSyncUpload gload;
	if (gload.Post(strUrl,pBuffer,iSize,strHeader))
	{
		delete pBuffer;
		pBuffer = NULL;
		CONST Tx::Drive::Mem::MemSlice &dnData = gload.Rsp().Body();
		if (!gload.Rsp().IsSuccess() || dnData.IsEmpty() || dnData.Size()<=1)
		{
			return false;
		}

		int nSize = dnData.Size();

		CString strXML = (LPCTSTR)dnData.DataPtr();
		strXML = strXML.Left(nSize);

		OleInitialize(NULL);
		Tx::Data::CXmlDocumentWrapper xmlDoc;
		/*2 读取XML*/
		if (!xmlDoc.LoadXML(strXML))
		{

			TRACE(_T("fail to load xml: %s.\r\n"), strXML);
			OleUninitialize();
			return false;
		}

		/*3得到根结点*/
		Tx::Data::CXmlNodeWrapper root(xmlDoc.AsNode());

		//获取节点数据
		//YTM
		CXmlNodeWrapper node = root.FindNode(_T("BondCalcResult"));
		if(!node.IsValid())
		{
			OleUninitialize();
			return false;
		}

		CXmlNodeWrapper nodeItem = node.FindNode(_T("ytm"));
		if (nodeItem.IsValid())
		{
			dYtm = atof(nodeItem.GetText());
		}
		nodeItem = node.FindNode(_T("duration"));
		if (nodeItem.IsValid())
		{
			dDuration = atof(nodeItem.GetText());
		}
		//修正久期
		nodeItem = node.FindNode(_T("mdDuration"));
		if (nodeItem.IsValid())
		{
			dMduration = atof(nodeItem.GetText());
		}
		//凸性
		nodeItem = node.FindNode(_T("convexity"));
		if (nodeItem.IsValid())
		{
			dConvexity = atof(nodeItem.GetText());
		}

		OleUninitialize();
		return true;
	}

	if (pBuffer)
	{
		delete pBuffer;
		pBuffer = NULL;
	}
	return false;
}
void TxBond::GetBondYTM(std::vector<int> nSecurityIds)
{
	if(nSecurityIds.size() <= 0) 
		return;

	m_BondYTMMap.clear();

	Tx::Business::TxBusiness business;
	CString strSvrList;
	strSvrList.Format(_T("%sServerList%d.ini"),Tx::Core::SystemPath::GetInstance()->GetExePath(),Tx::Core::UserInfo::GetInstance()->GetNetId());//
	if (false == Tx::Core::Commonality::File().IsFileExist (strSvrList))
		return;

	char tempch[200];
	memset(tempch,0,200);
	GetPrivateProfileString(_T("File"), _T("BondYTM"), _T(""), tempch, 200, strSvrList);

	CString strUrl = _T("%sBond/BondReal/0");
	strUrl.Format(_T("%sBond/BondReal/0"),tempch);

	CString strHeader = _T("Content-Type: application/xml; charset=utf-8\n");
	strHeader += _T("Accept: text/html, application/xhtml+xml, */*\n");
	strHeader += _T("Accept-Encoding: gzip, deflate\n");

	CString strReq = _T("<ArrayOfstring xmlns=\"http://schemas.microsoft.com/2003/10/Serialization/Arrays\">\n");
	for(int i=0;i<(int)nSecurityIds.size();i++)
	{
		business.GetSecurityNow(nSecurityIds[i]);
		if (business.m_pSecurity != NULL && business.m_pSecurity->IsBond())
		{
			strReq += _T("<string>");
			strReq += business.m_pSecurity->GetCode(true);
			strReq += _T("</string>\n");
		}
	}
	strReq += _T("</ArrayOfstring>");

	//Post请求
	int iSize = strReq.GetLength();
	LPBYTE pBuffer = new BYTE [iSize];
	if (pBuffer == NULL) 
		return;
	memcpy_s(pBuffer,iSize,strReq.GetBuffer(),iSize);
	strReq.ReleaseBuffer(iSize);

	Tx::Drive::Http::CSyncUpload upload;
	if (upload.Post(strUrl,pBuffer,iSize,strHeader,3000))
	{
		delete pBuffer;
		pBuffer = NULL;
		CONST Tx::Drive::Mem::MemSlice &dnData = upload.Rsp().Body();
		if (!upload.Rsp().IsSuccess() || dnData.IsEmpty() || dnData.Size()<=1)
		{	
			return;
		}

		CString strPath = SystemPath::GetInstance()->GetTempPath() + _T("\\YTM.xml");

		if (0 != Tx::Drive::IO::Zip::CZipWrapper::UnGzipToFile(dnData.DataPtr(),dnData.Size(),strPath))
		{
			return;
		}

		OleInitialize(NULL);
		Tx::Data::CXmlDocumentWrapper xmlDoc;

		/*2 读取XML*/
		if (!xmlDoc.Load(strPath))
		{
			TRACE(_T("fail to load xml: %s.\r\n"), _T("excelytm"));
			OleUninitialize();
			return;
		}

		/*3得到根结点*/
		Tx::Data::CXmlNodeWrapper root(xmlDoc.AsNode());
		if (!root.IsValid())
		{
			OleUninitialize();
			return;
		}

		for (int i=0;i<root.NumNodes();i++)
		{
			Tx::Data::BondYTM  bondYtm;
			Tx::Data::CXmlNodeWrapper nodeItem = root.GetNode(i);
			if (nodeItem.IsValid())
			{
				Tx::Data::CXmlNodeWrapper node = nodeItem.FindNode(_T("transID"));
				if (node.IsValid())
					bondYtm.object = atoi(node.GetText());
				else
					continue;

				node = nodeItem.FindNode(_T("ytm"));
				if(node.IsValid())
					bondYtm.fYtm = atof(node.GetText());

				node = nodeItem.FindNode(_T("duration"));
				if(node.IsValid())
					bondYtm.fDur = atof(node.GetText());

				node = nodeItem.FindNode(_T("mdDuration"));
				if(node.IsValid())
					bondYtm.fADur = atof(node.GetText());

				node = nodeItem.FindNode(_T("convexity"));
				if(node.IsValid())
					bondYtm.fCon = atof(node.GetText());

				m_BondYTMMap.insert(std::pair<int,BondYTM>(bondYtm.object,bondYtm));

			}
		}

		OleUninitialize();
	}
	if (pBuffer)
	{
		delete pBuffer;
		pBuffer = NULL;
	}
}
void TxBond::GetBondYTM(int nSecurityId,double &dYtm,double &dMdur,double &dCon)
{
	std::unordered_map<int,BondYTM>::iterator iter = m_BondYTMMap.find(nSecurityId);

	if ( iter!=m_BondYTMMap.end())
	{
		dYtm = iter->second.fYtm;
		dMdur = iter->second.fADur;
		dCon = iter->second.fCon;
	}
	else
	{
		dYtm = Con_doubleInvalid;
		dMdur = Con_doubleInvalid;
		dCon = Con_doubleInvalid;
	}
}

float TxBond::GetPriceByYtmNew(SecurityQuotation* pSecurity,int iDate,double dYtm)
{
	float fPrice = Con_floatInvalid;
	if (dYtm < 0.00000001)
		return fPrice;
	CString strSvrList;
	strSvrList.Format(_T("%sServerList%d.ini"),Tx::Core::SystemPath::GetInstance()->GetExePath(),Tx::Core::UserInfo::GetInstance()->GetNetId());//
	if (false == Tx::Core::Commonality::File().IsFileExist (strSvrList))
		return fPrice;

	char tempch[200];
	memset(tempch,0,200);
	GetPrivateProfileString(_T("File"), _T("BondYTM"), _T(""), tempch, 200, strSvrList);

	CString strUrl;
	strUrl.Format(_T("%sBond/BondCalc/YtmCalcPrice"),tempch);

	int iMarket = 1; //1-sh,2-sz
	if(pSecurity==NULL) 
		return fPrice;
	if(pSecurity->IsShanghai())
		iMarket = 1;
	else if(pSecurity->IsShenzhen())
		iMarket = 2;
	else
		return fPrice;
	CString strCode = pSecurity->GetCode();

	CString strHeader = _T("Content-Type: application/xml; charset=utf-8");
	CString strTemp;
	CString strReq = _T("<ArrayOfBondIndicatorYTM xmlns=\"www.txsec.com/mb810\">");
	strReq += _T("<BondIndicatorYTM>");
	strReq += _T("<code>");
	strReq += strCode;
	strReq += _T("</code>");
	strReq += _T("<market>");
	strTemp.Format(_T("%d"),iMarket);
	strReq += strTemp;
	strReq += _T("</market>");
	strReq += _T("<date>");
	strTemp.Format(_T("%d"),iDate);
	strReq += strTemp;
	strReq += _T("</date>");
	strReq += _T("<ytm>");
	strTemp.Format(_T("%f"),dYtm);
	strReq += strTemp;
	strReq += _T("</ytm>");
	strReq += _T("</BondIndicatorYTM>");
	strReq += _T("</ArrayOfBondIndicatorYTM>");

	//Post请求
	int iSize = strReq.GetLength();
	LPBYTE pBuffer = new BYTE [iSize];
	if (pBuffer == NULL) 
		return fPrice;
	memcpy_s(pBuffer,iSize,strReq.GetBuffer(),iSize);
	strReq.ReleaseBuffer(iSize);

	Tx::Drive::Http::CSyncUpload gload;
	if (gload.Post(strUrl,pBuffer,iSize,strHeader))
	{
		delete pBuffer;
		pBuffer = NULL;
		CONST Tx::Drive::Mem::MemSlice &dnData = gload.Rsp().Body();
		if (!gload.Rsp().IsSuccess() || dnData.IsEmpty() || dnData.Size()<=1)
		{
			return fPrice;
		}

		int nSize = dnData.Size();

		CString strXML = (LPCTSTR)dnData.DataPtr();
		strXML = strXML.Left(nSize);

		OleInitialize(NULL);
		Tx::Data::CXmlDocumentWrapper xmlDoc;
		/*2 读取XML*/
		if (!xmlDoc.LoadXML(strXML))
		{

			TRACE(_T("fail to load xml: %s.\r\n"), strXML);
			OleUninitialize();
			return fPrice;
		}

		/*3得到根结点*/
		Tx::Data::CXmlNodeWrapper root(xmlDoc.AsNode());

		//获取节点数据
		//YTM
		CXmlNodeWrapper node = root.FindNode(_T("BondCalcResult"));
		if(!node.IsValid())
		{
			OleUninitialize();
			return fPrice;
		}

		CXmlNodeWrapper nodeItem = node.FindNode(_T("price"));
		if (nodeItem.IsValid())
			fPrice = atof(nodeItem.GetText());

		OleUninitialize();
		return fPrice;
	}

	if (pBuffer)
	{
		delete pBuffer;
		pBuffer = NULL;
	}
	return fPrice;
}


	}
}