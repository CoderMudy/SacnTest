#include "stdafx.h"
#include "OPFundPriceSeries.h"

#include "../TxBusiness.h"
#include "AdapterTools/VARDateTimeAdapter.h"


COPFundPriceSeries::COPFundPriceSeries()
{
	m_lLatestDate = 19800101;
	m_iSampleCount = 100;
}
COPFundPriceSeries::COPFundPriceSeries(long lLatestDate, int iSampleCount) : CPriceSeriesGenerator(lLatestDate, iSampleCount)
{
}
COPFundPriceSeries::~COPFundPriceSeries()
{
}

double COPFundPriceSeries::GetLatestPrice(long lEntityID)
{
	double dLatestPrice = 0.0;
	
	Tx::Business::TxBusiness business;
	Tx::Data::SecurityQuotation*	pSecurity = business.GetSecurityNow(lEntityID);
	if(pSecurity==NULL)
	{
		return dLatestPrice;
	}
	dLatestPrice = (double)pSecurity->GetFundNAV(this->m_lLatestDate);
	if (dLatestPrice < 0.00001)
	{
		dLatestPrice = 0.00;
	}

	return dLatestPrice;
}

int	COPFundPriceSeries::GetSeries1(long lEntityID, std::vector<double> &arrSeries)
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
		iLastIndex = pSecurity->GetDataIntValue(dt_FundNetValue, dt.GetDate(), drt_Index);
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

	double dAddupNetVale = 0.0;
	std::vector<double> arrAddupNetvalueSeries;
	for (int i = iStartIndex; i <= iLastIndex; i++)
	{
		FundNetValueData* pData = pSecurity->GetFundNetValueDataByIndex(i);
		if (NULL == pData)
		{
			return -eDATA_ERROR;
		}
		dAddupNetVale = pData->fNetvalueAcu;
		arrAddupNetvalueSeries.push_back(dAddupNetVale);
	}

	int iDataCount = arrAddupNetvalueSeries.size();
	if ( iDataCount< 1)
	{
		return -eDATA_ERROR;
	}

	double dCurValue = this->GetLatestPrice(lEntityID);
	if (dCurValue < 0.0001)
	{
		return -eDATA_ERROR;
	}

	double dPriceRatio = arrAddupNetvalueSeries[iDataCount - 1]/dCurValue;
	if (dPriceRatio < 0.0001)
	{
		return -eDATA_ERROR;
	}

	for (int i = iDataCount - 1; i >= 0; i--)
	{
		arrAddupNetvalueSeries[i] /=dPriceRatio;
	}
	arrSeries.assign(arrAddupNetvalueSeries.begin(), arrAddupNetvalueSeries.end());

	return eSUCESS;
}

int	COPFundPriceSeries::GetSeries(long lEntityID, std::vector<double> &arrSeries)
{
	arrSeries.clear();

	int iMaxInvalidPriceCount = (int)(this->m_iSampleCount * 1.0);//最大无效价格数量，也可选择固定值;

	Tx::Business::TxBusiness business;
	Tx::Data::SecurityQuotation* pSecurity = business.GetSecurityNow(lEntityID);
	if(pSecurity==NULL)
	{
		return -eLOGICAL_ERROR;
	}
	//1 获取上证指数交易日序列
	std::vector<int> arrShIndexTradeDates;
	if (!this->GetShIndexTradeDateSeries(arrShIndexTradeDates))
	{
		return -eDATA_ERROR;
	}

	int iInvalidPriceCount = 0;

	double dPrePrice = 0.0;
	double dPrice = 0.0;
	double dVolume = 0.0;

	//2获取按上证指数交易日序列同步过的累计净值序列
	double dAddupNetVale = 0.0;
	double dPreAddupNetVale = 0.0;
	std::vector<double> arrAddupNetvalueSeries;

	int iDate = 0;
	int iIdx = 0;
	std::vector<int>::iterator iterDate = arrShIndexTradeDates.begin();
	for (; iterDate != arrShIndexTradeDates.end(); iterDate++)
	{
		iDate  = *iterDate;
		FundNetValueData* pData = NULL;
		iIdx  = pSecurity->GetDataIntValue(dt_FundNetValue, iDate, drt_Index);
		if (iIdx >= 0)
		{
			pData = pSecurity->GetFundNetValueDataByIndex(iIdx);
		}
		if (NULL == pData)
		{
			dAddupNetVale = 0.0;
		}
		else
		{
			dAddupNetVale = pData->fNetvalueAcu;
		}

		if (dAddupNetVale < 0.00001 )
		{

			iInvalidPriceCount++;//未成功获取而进行修正的数据个数

			if(dPreAddupNetVale < 0.000001)
			{
				dAddupNetVale = 0.0;
			}
			else
			{
				dAddupNetVale = dPreAddupNetVale;
			}
		}
		else 
		{
			if (iInvalidPriceCount > 0 && dPreAddupNetVale < 0.00001)//如果之前从未取得或有效数据
			{
				arrAddupNetvalueSeries.assign(arrAddupNetvalueSeries.size(), dAddupNetVale);
			}
			dPreAddupNetVale = dAddupNetVale;
		}

		arrAddupNetvalueSeries.push_back(dAddupNetVale);
	}

	int iDataCount = arrAddupNetvalueSeries.size();
	if ( iDataCount< 1)
	{
		return -eDATA_ERROR;
	}

	/*3 无效数据超过一定数据就不再计算*/
	if (iInvalidPriceCount > iMaxInvalidPriceCount)
	{
		return eDATA_ERROR;
	}

	/*4 将所有累计净值数据按最新单位净值/最新累计净值折算*/
	double dCurValue = this->GetLatestPrice(lEntityID);
	if (dCurValue < 0.0001)
	{
		return -eDATA_ERROR;
	}

	double dPriceRatio = arrAddupNetvalueSeries[iDataCount - 1]/dCurValue;
	if (dPriceRatio < 0.0001)
	{
		return -eDATA_ERROR;
	}

	for (int i = iDataCount - 1; i >= 0; i--)
	{
		arrAddupNetvalueSeries[i] /=dPriceRatio;
	}
	arrSeries.assign(arrAddupNetvalueSeries.begin(), arrAddupNetvalueSeries.end());

	return eSUCESS;
}
CString COPFundPriceSeries::ErrorCodeToString(int iError)
{
	return CPriceSeriesGenerator::ErrorCodeToString(iError);
}

