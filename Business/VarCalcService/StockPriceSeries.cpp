#include "stdafx.h"
#include "StockPriceSeries.h"

#include "AdapterTools/VARDateTimeAdapter.h"

#include "../TxBusiness.h"

CStockPriceSeries::CStockPriceSeries()
{
	m_lLatestDate = 19800101;
	m_iSampleCount = 100;
}

CStockPriceSeries::CStockPriceSeries(long lLatestDate, int iSampleCount) : CPriceSeriesGenerator(lLatestDate, iSampleCount)
{
}

CStockPriceSeries::~CStockPriceSeries()
{
}

double CStockPriceSeries::GetLatestPrice(long lEntityID)
{
	double dLatestPrice = 0.0;
	
	Tx::Business::TxBusiness business;
	Tx::Data::SecurityQuotation*	pSecurity = business.GetSecurityNow(lEntityID);
	if(pSecurity==NULL)
	{
		return dLatestPrice;
	}
	dLatestPrice = (double)pSecurity->GetClosePrice(this->m_lLatestDate,true);//指定日期[历史]收盘价格，如果指定日期没有交易，择取前收价
	if (dLatestPrice < 0.00001)
	{
		dLatestPrice = 0.00;
	}

	return dLatestPrice;
}

int	CStockPriceSeries::GetSeries1(long lEntityID, std::vector<double> &arrSeries)
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

	bool bAhead = true;
	int iStartDate = pSecurity->GetTradeDateByIndex(iStartIndex, 0, 0 );
	Tx::Data::HisTradeData *pData = NULL;
	for (int i = iStartIndex; i <= iLastIndex; i++)
	{
		int iDate = pSecurity->GetTradeDateByIndex(i);

		double dSccale = pSecurity->GetExdividendScale(iDate, (int)this->m_lLatestDate, bAhead);
		double dPrice = pSecurity->GetClosePrice(iDate);
		if (dPrice < 0.00001 || dSccale < 0.001)
		{
			return -eDATA_ERROR;
		}
		dPrice *= dSccale;

		arrSeries.push_back(dPrice);
	}
	return eSUCESS;
}

int	 CStockPriceSeries::GetSeries(long lEntityID, std::vector<double> &arrSeries)
{
	arrSeries.clear();

	int iMaxInvalidPriceCount = (int)(this->m_iSampleCount * 1.0);//暂时写死，以后可扩展成可设置的，最大无效价格数量，也可选择this->m_iSampleCount * 0.2;

	//1 下载数据
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

	//2 获取上证指数交易日序列
	std::vector<int> arrShIndexTradeDates;
	if (!this->GetShIndexTradeDateSeries(arrShIndexTradeDates))
	{
		return -eDATA_ERROR;
	}

	int iInvalidPriceCount = 0;

	double dPrePrice = 0.0;
	double dPrice = 0.0;
	double dVolume = 0.0;

	//获取按上证指数交易日序列同步过的价格序列
	int iDate = 0;
	int iIndex = -1;
	std::vector<int>::iterator iterDate = arrShIndexTradeDates.begin();
	for (; iterDate != arrShIndexTradeDates.end(); iterDate++)
	{
		iDate  = *iterDate;
		if (!pSecurity->IsTradeDate(iDate))//不是交易日不取数据
		{
			dPrice = 0.0;
			dVolume = 0.0;
		}
		else
		{
			dPrice = pSecurity->GetClosePrice(iDate);
			dVolume = pSecurity->GetVolume(iDate);
		}
		if (dPrice < 0.00001 )
		{
			if (dVolume > 0.00001) 
			{
				return -eDATA_ERROR;
			}
			else
			{
				iInvalidPriceCount++;//未成功获取而进行修正的数据个数

				if(dPrePrice < 0.000001)
				{
					dPrice = 0.0;
				}
				else
				{
					dPrice = dPrePrice;
				}
			}
		}
		else 
		{
			if (iInvalidPriceCount > 0 && dPrePrice < 0.00001)//如果之前从未取得或有效数据， 都设为第一次取得有效数据的
			{
				arrSeries.assign(arrSeries.size(), dPrice);
			}
			dPrePrice = dPrice;
		}
		
		arrSeries.push_back(dPrice);
	}
	/* 无效数据超过一定数据就不再计算*/
	if (iInvalidPriceCount > iMaxInvalidPriceCount)
	{
		arrSeries.clear();
		return -eDATA_ERROR;
	}
	//3 目前默认将价格进行前复权处理， 以防止价格的突变
	bool PriceCorrection = true;//是否用前复权价格进行修正
	if (PriceCorrection)
	{	
		if (arrShIndexTradeDates.size() != arrSeries.size())
		{
			return -eDATA_ERROR;
		}

		bool bAhead = true;

		double dSccale = 1.0;
		iterDate = arrShIndexTradeDates.begin();
		int i = 0;
		for (; iterDate != arrShIndexTradeDates.end(); iterDate++, i++)
		{

			iDate  = *iterDate;
			dSccale = pSecurity->GetExdividendScale(iDate, (int)this->m_lLatestDate, bAhead);
			if (dSccale < 0.001)
			{
				return -eDATA_ERROR;
			}
			arrSeries.at(i) *= dSccale;
		}
	}
	return eSUCESS;
}

CString CStockPriceSeries::ErrorCodeToString(int iError)
{
	return CPriceSeriesGenerator::ErrorCodeToString(iError);
}