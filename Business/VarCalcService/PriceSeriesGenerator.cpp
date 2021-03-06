#include "stdafx.h"
#include "PriceSeriesGenerator.h"

#include "../TxBusiness.h"
#include "AdapterTools/VARDateTimeAdapter.h"

CPriceSeriesGenerator::CPriceSeriesGenerator()
{
	m_lLatestDate = 19800101;
	m_iSampleCount = 100;
}
CPriceSeriesGenerator::CPriceSeriesGenerator(long lLatestDate, int iSampleCount)
{
	Tx::Business::TxBusiness business;
	long lDate = (long)business.m_pFunctionDataManager->GetServerCurDateTime_ShangHai().GetDate().GetInt();
	if (lLatestDate >= lDate)
	{
		CVARDateTimeAdapter dt(lDate, 0);
		dt.LastDay();
		lLatestDate = dt.GetDate();
	}

	m_lLatestDate = lLatestDate;
	this->m_iSampleCount = iSampleCount;
}

CPriceSeriesGenerator::~CPriceSeriesGenerator()
{
	
}

double CPriceSeriesGenerator::GetLatestPrice(long lEntityID)
{
	double dLatestPrice = 0.0;

	return dLatestPrice;
}

int	CPriceSeriesGenerator::GetSeries(long lEntityID, std::vector<double> &arrSeries)
{
	arrSeries.clear();

	return eSUCESS;
}
bool CPriceSeriesGenerator::GetShIndexTradeDateSeries(std::vector<int>& arrTradeDates)
{
	arrTradeDates.clear();

	long lShIndexID = 4000208;
	Tx::Business::TxBusiness business;
	Tx::Data::SecurityQuotation* pSecurity = business.GetSecurityNow(4000208);
	if(pSecurity==NULL)
	{
		return false;
	}

	if (!pSecurity->LoadHisTrade())
	{
		return false;
	}

	if (!pSecurity->LoadTradeDate())
	{
		return false;
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
		return false;
	}

	for (int i = iStartIndex; i <= iLastIndex; i++)
	{
		int iDate = pSecurity->GetTradeDateByIndex(i);
		arrTradeDates.push_back(iDate);
	}
	return true;
}

CString CPriceSeriesGenerator::ErrorCodeToString(int iError)
{
	CString strError = _T("");

	return strError;
}