#include "stdafx.h"
#include "VarCalcDataSource.h"

#include "VarCalcGlobal.h"
namespace Tx
{
	namespace VAR
	{
CVarDataSource::CVarDataSource()
{
	this->m_iSamplesCount = 0;
	this->m_iDateCount = 0;
}

CVarDataSource::~CVarDataSource()
{
	Clear();
}
bool CVarDataSource::AssertValid()
{
	if (0 == m_iSamplesCount || 0 == this->m_iDateCount)
	{
		return false;
	}
	if (this->m_matrixPrice.GetColCount() != m_iSamplesCount  || this->m_matrixPrice.GetRowCount() != this->m_iDateCount)
	{
		return false;
	}
	if (this->m_matrixStorage.GetRowCount() != this->m_iSamplesCount || 1 != this->m_matrixStorage.GetColCount())
	{
		return false;
	}
	if (this->m_matrixLatestValue.GetRowCount() != this->m_iSamplesCount || 1 != this->m_matrixLatestValue.GetColCount())
	{
		return false;
	}

	return true;
}
void CVarDataSource::Clear()
{
	this->m_matrixPrice.SetRowCol(0,0);
	this->m_matrixStorage.SetRowCol(0,0);
	this->m_matrixLatestValue.SetRowCol(0,0);

	this->m_iSamplesCount = 0;
	this->m_iDateCount = 0;
}
bool CVarDataSource::FillData(Tx::Core::CArrayMatrix& matrixPrice, Tx::Core::CArrayMatrix& matrixStorage)
{	
	if (matrixPrice.GetColCount() != matrixStorage.GetRowCount() ||1 != matrixStorage.GetColCount())
	{
		return false;
	}

	this->m_matrixPrice = matrixPrice;
	this->m_matrixStorage = matrixStorage;

	this->m_iDateCount = this->m_matrixPrice.GetRowCount();
	this->m_iSamplesCount = this->m_matrixPrice.GetColCount();

	if (!this->TransformCoeff())
	{
		return false;
	}

	if (!AssertValid())
	{
		Clear();
		return false;
	}

	return true;
}

bool CVarDataSource::TransformCoeff()
{
	int iRowCount = this->m_matrixPrice.GetRowCount(); 
	int nLastRow = iRowCount - 1;
	double dValue = 0.0;
	this->m_matrixLatestValue.SetRowCol(m_iSamplesCount, 1);
	for(int i = 0; i< m_iSamplesCount;i++)
	{
		dValue = this->m_matrixPrice.GetData(nLastRow, i);
		dValue *= this->m_matrixStorage.GetData(i, 0);
		this->m_matrixLatestValue.SetData(i, 0, dValue);
	}

	return true;
}

int CVarDataSource::GetSamplesCount()
{
	return this->m_iSamplesCount;
}
int CVarDataSource::GetSampleDays()
{
	return this->m_iDateCount;
}
void CVarDataSource::GetPricesMatrix(Tx::Core::CArrayMatrix& matrixPrice)
{
	matrixPrice = this->m_matrixPrice;
}

void CVarDataSource::GetStorageMatrix(Tx::Core::CArrayMatrix& matrixStorage)
{
	matrixStorage = this->m_matrixStorage;
}
void CVarDataSource::GetLatestValeMatrix(Tx::Core::CArrayMatrix& matrixLatestVale)
{
	matrixLatestVale = this->m_matrixLatestValue;
}
bool CVarDataSource::GetPLRateMatrix(int iType, int iSpan, Tx::Core::CArrayMatrix& matrixPLRate)
{
	matrixPLRate.SetRowCol(0,0);
	
	if (!this->AssertValid())
	{
		return false;
	}


	int iPLDateCount = this->m_iDateCount - iSpan;
	if (iPLDateCount < 1 || iPLDateCount >= this->m_iDateCount)
	{
		return false;
	}

	matrixPLRate.SetRowCol(iPLDateCount, m_iSamplesCount);

	double dTempValue = 0.0;

	switch(iType)
	{
	case eProfit: //0-收益
		{
			for(int i = 0; i < this->m_iSamplesCount; i++)
			{
				for(int j = 0; j < iPLDateCount; j++)
				{
					dTempValue = this->m_matrixPrice.GetData(j + iSpan, i) - this->m_matrixPrice.GetData(j, i);
					matrixPLRate.SetData(j, i, dTempValue);
				}
			}
		}
		break;
	case ePercentProfitRate://1-百分比收益率
		{
			double dTemp2 = 0.0;
			for(int i = 0; i < this->m_iSamplesCount; i++)
			{
				for(int j = 0; j < iPLDateCount; j++)
				{
					dTemp2 = this->m_matrixPrice.GetData(j, i);
					if (dTemp2 < 0.000001)
					{
						dTempValue = 0.0;
					}
					else
					{
						dTempValue = (this->m_matrixPrice.GetData(j + iSpan, i) - dTemp2)/dTemp2;
					}

					matrixPLRate.SetData(j, i, dTempValue);
				}
			}
		}
		break;
	case eLogProfitRate://2-对数收益率
		{
			double dTemp2 = 0.0;
			for(int i = 0; i < this->m_iSamplesCount; i++)
			{
				for(int j = 0; j < iPLDateCount; j++)
				{
					dTempValue = this->m_matrixPrice.GetData(j + iSpan, i);
					dTemp2 = this->m_matrixPrice.GetData(j, i);
					if (dTemp2 < 0.000001 || dTempValue < 0.000001)
					{
						dTempValue = 0.0;
					}
					else
					{
						dTempValue = log(dTempValue) - log(dTemp2);
					}

					matrixPLRate.SetData(j, i, dTempValue);
				}
			}
		}
		break;
	case ePrice: //3 价格
		{
			for(int i = 0; i < this->m_iSamplesCount; i++)
			{
				for(int j = 0; j < iPLDateCount; j++)
				{
					dTempValue = this->m_matrixPrice.GetData(j + iSpan, i);
					matrixPLRate.SetData(j, i, dTempValue);
				}
			}
		}
		break;
	}

	return true;
}
}
}