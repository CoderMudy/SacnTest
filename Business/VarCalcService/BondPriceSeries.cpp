#include "stdafx.h"
#include "BondPriceSeries.h"

#include "../TxBusiness.h"

#include "AdapterTools/VARDateTimeAdapter.h"

CBondPriceSeries::CBondPriceSeries()
{
	m_lLatestDate = 19800101;
	m_iSampleCount = 100;
}
CBondPriceSeries::CBondPriceSeries(long lLatestDate, int iSampleCount) : CPriceSeriesGenerator(lLatestDate, iSampleCount)
{
}
CBondPriceSeries::~CBondPriceSeries()
{
}

double CBondPriceSeries::GetLatestPrice(long lEntityID)
{
	return this->GetBondTotalPrice(lEntityID, this->m_lLatestDate);
}

int	CBondPriceSeries::GetSeries(long lEntityID, std::vector<double> &arrSeries)
{
	arrSeries.clear();

	Tx::Business::TxBusiness business;
	Tx::Data::SecurityQuotation* pSecurity = business.GetSecurityNow(lEntityID);
	if(pSecurity==NULL)
	{
		return -eLOGICAL_ERROR;
	}

	if (!pSecurity->LoadHisTrade())
	{
		return -eLOGICAL_ERROR;
	}

	if (!pSecurity->LoadTradeDate())
	{
		return -eLOGICAL_ERROR;
	}

	int iLastIndex = -1;
	int iTryCount = 20;
	CVARDateTimeAdapter dt(this->m_lLatestDate, 0);

	while (iTryCount > 0)
	{
		iLastIndex = pSecurity->GetTradeDateIndex(dt.GetDate(), 0, 0 );
		if (iLastIndex > 0)
		{
			break;
		}
		dt.LastDay();
		iTryCount--;
	}

	int iStartIndex = iLastIndex + 1 - this->m_iSampleCount;
	if (iStartIndex < 0)
	{
		return -eDATA_UNENOUGH;
	}

	Tx::Data::HisTradeData *pData = NULL;
	for (int i = iStartIndex; i <= iLastIndex; i++)
	{
		int iDate = pSecurity->GetTradeDateByIndex(i);
		double dPrice = this->GetBondTotalPrice(lEntityID, (long)iDate);
		if (dPrice < 0.00001)
		{
			return -eDATA_ERROR;
		}
		arrSeries.push_back(dPrice);
	}
	return eSUCESS;
}

CString CBondPriceSeries::ErrorCodeToString(int iError)
{
	CString strError = _T("");

	return strError;
}

double CBondPriceSeries::GetBondTotalPrice(long lEntityID, long lDate)
{
	double dLatestPrice = 100.00;

	Tx::Business::TxBusiness business;
	Tx::Data::SecurityQuotation*	pSecurity = business.GetSecurityNow(lEntityID);
	if(pSecurity==NULL)
	{
		return dLatestPrice;
	}

	dLatestPrice = (double)pSecurity->GetClosePrice(lDate,true);

	if (dLatestPrice < 0.0001)
	{
		dLatestPrice = 100.0;
	}
	if (pSecurity->IsBondNetPrice((int)lDate))
	{
		double dInterestPerShare = CalcBondInterestPerShare(lEntityID, lDate);
		dLatestPrice += dInterestPerShare;
	}
	return dLatestPrice;
}

double CBondPriceSeries::CalcBondInterestPerShare(long lEntityID, long lDate)//包括lDate的利息
{
	double dInterest = 0.0;
	double dDailyInterestRate = 0.0;

	VAR_BondInfo infoBond;
	this->GetBondInfo(lEntityID, infoBond);
	if (5 == infoBond.m_iInterestType)//贴现债券
	{
		CVARDateTimeAdapter dt(infoBond.m_lEndDate);
		long lDateDiff = dt.GetDayDiff(infoBond.m_lBeginDate);
		if (lDateDiff > 0)
		{
			dDailyInterestRate =  (infoBond.m_dParValue - infoBond.m_dIssuePrice)/lDateDiff;
		}
	}
	else //除贴现债券外，都从现金流取数据
	{
		dDailyInterestRate =  GetBondCashFlowYearRate(lEntityID, lDate)/(365*100);//取回数据是百分比
	}

	long lNearestPayDate = GetBondNearestInterestPayDate(lEntityID,lDate);//获得最近的起息日
	if (lNearestPayDate >= infoBond.m_lBeginDate && lNearestPayDate > 19700101)//开始日也算付息日
	{
		CVARDateTimeAdapter dt(lDate,0);
		dt.NextDay();//lDate也算利息

		long lDateDiff = dt.GetDayDiff(lNearestPayDate);
		if (lDateDiff > 0)
		{
			dInterest = dDailyInterestRate * infoBond.m_dParValue * lDateDiff;
		}
	}
	return dInterest;
}
bool CBondPriceSeries::GetBondInfo(long lSecurities, VAR_BondInfo &infoBond)
{
	Tx::Business::TxBusiness business;
	Tx::Data::SecurityQuotation* pSecurity = business.GetSecurityNow(lSecurities);
	if(pSecurity==NULL)
	{
		return false;
	}
	Tx::Data::BondNewInfo *pBondNewInfo = pSecurity->GetBondNewInfo();
	if(NULL == pBondNewInfo)
	{
		return false;
	}

	infoBond.m_lSecuritiesId = (long)pBondNewInfo->trans_object_id;
	infoBond.m_dParRate = pBondNewInfo->par_rate;
	infoBond.m_dHoldYear= pBondNewInfo->hold_year;
	infoBond.m_iInterestType = pBondNewInfo->interest_type;
	infoBond.m_lBeginDate= pBondNewInfo->begin_date;
	infoBond.m_lEndDate= pBondNewInfo->end_date;
	infoBond.m_iPayInterestFrequence= pBondNewInfo->pay_interest_frequence;
	infoBond.m_dParValue= pBondNewInfo->par_val;
	infoBond.m_iBondType= pBondNewInfo->bond_type;
	infoBond.m_dIssuePrice= pBondNewInfo->issue_price;
	return true;
}
double CBondPriceSeries::GetBondCashFlowYearRate(long lSecurities, long lDate)
{
	double dFlowRate = 0.0;

	Tx::Business::TxBusiness business;
	Tx::Data::SecurityQuotation* pSecurity = business.GetSecurityNow(lSecurities);
	if(pSecurity==NULL)
	{
		return dFlowRate;
	}
	Tx::Data::BondCashFlowData *pBondCashFlow = pSecurity->GetBondCashFlowDataByDate(lDate);
	if(NULL == pBondCashFlow)
	{
		return dFlowRate;
	}
	dFlowRate = pBondCashFlow->F_CASH;
	if (dFlowRate < 0.00000001)
	{
		dFlowRate = 0.0;
	}
	return dFlowRate;
}
long CBondPriceSeries::GetBondNearestInterestPayDate(long lSecurities, long lDate)
{
	VAR_BondInfo infoBond;
	GetBondInfo(lSecurities, infoBond);
	if (infoBond.m_iPayInterestFrequence > 0)
	{
		CVARDateTimeAdapter dt(infoBond.m_lBeginDate,0);
		while(dt.GetDate() < infoBond.m_lEndDate)
		{
			dt.AddMonth(infoBond.m_iPayInterestFrequence);
			if (dt.GetDate() >= lDate)
			{
				break;
			}
		}

		if (dt.GetDate() >= lDate)
		{
			dt.AddMonth(-infoBond.m_iPayInterestFrequence);
			return dt.GetDate();
		}

	}
	return infoBond.m_lBeginDate;
}
