#include "stdafx.h"
#include "CurrencyFundPriceSeries.h"
#include "AdapterTools/VARDateTimeAdapter.h"

#include "../TxBusiness.h"

CCurrencyFundPriceSeries::CCurrencyFundPriceSeries()
{
	m_lLatestDate = 19800101;
	m_iSampleCount = 100;
}
CCurrencyFundPriceSeries::CCurrencyFundPriceSeries(long lLatestDate, int iSampleCount) : CPriceSeriesGenerator(lLatestDate, iSampleCount)
{
}
CCurrencyFundPriceSeries::~CCurrencyFundPriceSeries()
{
}

double CCurrencyFundPriceSeries::GetLatestPrice(long lEntityID)
{
	double dLatestPrice = 1.0;

	return dLatestPrice;
}

int	CCurrencyFundPriceSeries::GetSeries(long lEntityID, std::vector<double> &arrSeries)
{
	arrSeries.clear();
	
	Tx::Business::TxBusiness business;
	Tx::Data::SecurityQuotation* pSecurity = business.GetSecurityNow(lEntityID);
	if(pSecurity==NULL)
	{
		return -eLOGICAL_ERROR;
	}

	
	int iLastIndex = -1;
	int iTryCount = 20;
	CVARDateTimeAdapter dt(this->m_lLatestDate, 0);

	while (iTryCount > 0)
	{
		iLastIndex = pSecurity->GetDataIntValue(dt_FundGain, dt.GetDate(),drt_Index);
		if (iLastIndex > 0)
		{
			break;
		}
		dt.LastDay();
		iTryCount--;
	}

	if (iLastIndex < 0)
	{
		return -eDATA_ERROR;
	}
	int iStartIndex = iLastIndex + 1 - this->m_iSampleCount;
	if (iStartIndex < 0)
	{
		return -eDATA_UNENOUGH;
	}

	double dProfitPerShare = 0.0;
	std::vector<double> arrPLSeries;
	for (int i = iStartIndex; i <= iLastIndex; i++)
	{
		FundGainData* pData = pSecurity->GetFundGainDataByIndex(i);
		if (NULL == pData)
		{
			return -eDATA_ERROR;
		}
		dProfitPerShare = pData->gain_ten_thousand_share / 10000;
		arrPLSeries.push_back(dProfitPerShare);
	}

	int iDataCount = arrPLSeries.size();
	if ( iDataCount< 1)
	{
		return -eDATA_ERROR;
	}
	double dCurValue = 1;
	double dCurPL = 0.0;

	for (int i = iDataCount - 1; i >= 0; i--)
	{
		dCurPL = arrPLSeries[i];
		if (dCurPL < -0.000001)
		{
			return eDATA_ERROR;
		}
		arrPLSeries[i] = dCurValue;
		dCurValue = dCurValue/(1 + dCurPL);
	}
	arrSeries.assign(arrPLSeries.begin(), arrPLSeries.end());

	return eSUCESS;
}

CString CCurrencyFundPriceSeries::ErrorCodeToString(int iError)
{
	return CPriceSeriesGenerator::ErrorCodeToString(iError);
}
