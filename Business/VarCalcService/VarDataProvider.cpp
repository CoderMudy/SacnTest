#include "stdafx.h"
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include<fstream>

#include "VarDataProvider.h"
#include "AdapterTools/VARTypeChange.h"

#include "StockPriceSeries.h"
#include "OPFundPriceSeries.h"
#include "CurrencyFundPriceSeries.h"
#include "BondPriceSeries.h"
#include "FileLoadPriceSeries.h"

#include "../TxBusiness.h"

CVarDataProvider::CVarDataProvider()
{
	this->m_iDate = 0;// 默认去当天
	this->m_iSamplesDays = 250;
}

CVarDataProvider::~CVarDataProvider()
{
	this->Clear();
}
void CVarDataProvider::SetParams(int iDate, int iSamplesDays)
{
	this->m_iDate = iDate;
	this->m_iSamplesDays = iSamplesDays;
}

bool CVarDataProvider::AddInvestItem(long lEntityID, double dHodingNumber)
{
	if (lEntityID < 1 || dHodingNumber < 0.0001)
	{
		return false;
	}
	this->m_arrPortfolio.push_back(std::make_pair(lEntityID, dHodingNumber));
	return true;
}

void CVarDataProvider::Clear()
{
	m_arrPortfolio.clear();
}

bool CVarDataProvider::AssetSetting()
{
	if (this->m_iDate < 19800101 || this->m_iSamplesDays < 1)
	{
		return false;
	}
	if (this->m_arrPortfolio.size() < 1)
	{
		return false;
	}
	return true;
}
bool CVarDataProvider::AssertPricesSeries(long lEntityID)
{
	std::vector<double> arrPrice;
	double dLatestPrice = 0.0;


	if (!this->GetPriceArrayAndLatestPrice(lEntityID, arrPrice, dLatestPrice))
	{
		return false;
	}
	if (dLatestPrice < -0.0001)//价格可以为0
	{
		return false;
	}
	if (arrPrice.size() != this->m_iSamplesDays)
	{
		return false;
	}
	return true;
}
bool CVarDataProvider::UpdateDSFromFile(CString& strPath, Tx::VAR::CVarDataSource* pDs)
{
	if (NULL == pDs)
	{
		return false;
	}
	if (!this->AssetSetting())
	{
		return false;
	}

	Tx::Core::CArrayMatrix matrixPLRate;
	Tx::Core::CArrayMatrix matrixStorage;

	int iSamplesCount = (int)this->m_arrPortfolio.size();
	matrixPLRate.SetRowCol(this->m_iSamplesDays, iSamplesCount);
	matrixStorage.SetRowCol(iSamplesCount, 1);

	std::vector<double> arrPLRate;
	double dHolding = 0.0;
	double dLatestPrice = 0.0;

	std::vector<std::pair<long, double> >::iterator iter = this->m_arrPortfolio.begin();
	for (int iSampleIdx = 0; iter != this->m_arrPortfolio.end(); iter++, iSampleIdx++)
	{
		dHolding = iter->second;
		if (dHolding < 0.0001)
		{
			return false;
		}

		if (!this->GetPriceArrayFromFile(strPath, iter->first, arrPLRate))
		{
			return false;
		}
		if (dLatestPrice < -0.0001)//价格可以为0
		{
			return false;
		}
		if (arrPLRate.size() != this->m_iSamplesDays)
		{
			return false;
		}

		for (int iDays = 0; iDays < this->m_iSamplesDays; iDays++)
		{
			matrixPLRate.SetData(iDays, iSampleIdx, arrPLRate.at(iDays));
		}
		matrixStorage.SetData(iSampleIdx, 0, dHolding);
	}

	if (!pDs->FillData(matrixPLRate, matrixStorage))
	{
		return false;
	}
	return true;
}
bool CVarDataProvider::UpdateDS(Tx::VAR::CVarDataSource* pDs)
{
	if (NULL == pDs)
	{
		return false;
	}
	if (!this->AssetSetting())
	{
		return false;
	}

	Tx::Core::CArrayMatrix matrixPrice;
	Tx::Core::CArrayMatrix matrixStorage;

	int iSamplesCount = (int)this->m_arrPortfolio.size();
	matrixPrice.SetRowCol(this->m_iSamplesDays, iSamplesCount);
	matrixStorage.SetRowCol(iSamplesCount, 1);

	std::vector<double> arrPrice;
	double dHolding = 0.0;
	double dLatestPrice = 0.0;

	std::vector<std::pair<long, double> >::iterator iter = this->m_arrPortfolio.begin();
	for (int iSampleIdx = 0; iter != this->m_arrPortfolio.end(); iter++, iSampleIdx++)
	{
		dHolding = iter->second;
		if (dHolding < 0.0001)
		{
			return false;
		}

		if (!this->GetPriceArrayAndLatestPrice(iter->first, arrPrice, dLatestPrice))
		{
			return false;
		}
		if (dLatestPrice < -0.0001)//价格可以为0
		{
			return false;
		}
		if (arrPrice.size() != this->m_iSamplesDays)
		{
			return false;
		}

		for (int iDays = 0; iDays < this->m_iSamplesDays; iDays++)
		{
			matrixPrice.SetData(iDays, iSampleIdx, arrPrice.at(iDays));
		}
		matrixStorage.SetData(iSampleIdx, 0, dHolding);
	}

	if (!pDs->FillData(matrixPrice, matrixStorage))
	{
		return false;
	}
	return true;
}

bool CVarDataProvider::GetPriceArrayAndLatestPrice(long lEntityID, vector<double>& arrPrice, double &dLatestPrice)
{
	/*1 参数检查*/	
	arrPrice.clear();
	dLatestPrice = 0.0;
	if (lEntityID < 1)
	{
		return false;
	}

	Tx::Business::TxBusiness business;
	Security*	pSecurity = business.GetSecurityNow(lEntityID);
	if(pSecurity==NULL)
	{
		return false;
	}	

	/*2 券种确认*/
	if (!pSecurity->IsNormal())//正常交易
	{
		return false;
	}	
	if (!pSecurity->IsCN_Market())
	{
		return false;
	}
	if ((!pSecurity->IsBond()) && (!pSecurity->IsFund()) && (!pSecurity->IsStockA() && (!pSecurity->IsWarrant())))//目前暂不支持b股
	{
		return false;
	}
	/*3取从开始持有期之前m_iSamplesDays个交易日的数据， 如果数据量不足告知*/
	CPriceSeriesGenerator *pSeries = this->CreatePLRateSeriesObj(lEntityID);
	if (NULL == pSeries)
	{
		return false;
	}

	dLatestPrice = pSeries->GetLatestPrice(lEntityID);
	int iError = pSeries->GetSeries(lEntityID, arrPrice);
	if (CPriceSeriesGenerator::eSUCESS != iError)
	{
		CString strError = pSeries->ErrorCodeToString(iError);
		//ReportError
		delete pSeries;
		pSeries = NULL;
		return false;
	}
	delete pSeries;
	pSeries = NULL;

	return true;
}

bool CVarDataProvider::GetPriceArrayFromFile(CString& strPath, long lEntityID, vector<double>& arrPrice)
{
	CFileLoadPriceSeries *pSeries = new CFileLoadPriceSeries(this->m_iDate, this->m_iSamplesDays);
	if (NULL == pSeries)
	{
		return false;
	}
	pSeries->SetDataDirPath(strPath);

	int iError = pSeries->GetSeries(lEntityID, arrPrice);
	if (CPriceSeriesGenerator::eSUCESS != iError)
	{
		CString strError = pSeries->ErrorCodeToString(iError);
		//ReportError
		delete pSeries;
		pSeries = NULL;
		return false;
	}
	delete pSeries;
	pSeries = NULL;

	return true;
	
}
CPriceSeriesGenerator* CVarDataProvider::CreatePLRateSeriesObj(long lEntityID)
{
	CPriceSeriesGenerator *pSeries = NULL;

	Tx::Business::TxBusiness business;
	Security*	pSecurity = business.GetSecurityNow(lEntityID);
	if(pSecurity==NULL)
	{
		return pSeries;
	}

	int iSampleCount = this->m_iSamplesDays; 

	if (pSecurity->IsFund_Open())
	{
		if (pSecurity->IsFund_Currency())
		{
			pSeries = new CCurrencyFundPriceSeries(this->m_iDate, iSampleCount);
		}
		else
		{
			pSeries = new COPFundPriceSeries(this->m_iDate, iSampleCount);//普通取前复权净值
		}
	}
	else if (pSecurity->IsBond())
	{
		pSeries = new CBondPriceSeries(this->m_iDate, iSampleCount);
	}
	else
	{
		pSeries = new CStockPriceSeries(this->m_iDate, iSampleCount);//取前复权价格
	}

	return pSeries;
}
