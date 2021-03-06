#include "stdafx.h"
#include <math.h>
#include "VarCalcMethod.h"

namespace Tx
{
	namespace VAR
	{
CCommonMathMethod::CCommonMathMethod()
{
}

CCommonMathMethod::~CCommonMathMethod()
{
}

bool CCommonMathMethod::PercentileFromDencity(double dAlpha, Tx::Core::CArrayMatrix &matrixM2, double &dVar, double &dCVar)
{
	if (dAlpha < 0.0000001 || dAlpha > 1.0)
	{
		return false;
	}
	if (2 != matrixM2.GetColCount())
	{
		return false;
	}

#ifdef _DEBUG
	long lStart = long(::GetTickCount());
#endif
	int iSampleDays = matrixM2.GetRowCount();

	//权重归一化
	int i = 0; 
	double dSum = 0.0;
	for (i=0; i< iSampleDays; i++)
	{
		dSum += matrixM2.GetData(i, 1);
	}
	for (i=0; i< iSampleDays; i++)
	{
		matrixM2.SetData(i,1,  matrixM2.GetData(i, 1)/dSum);
	}

	//matrixM2排序，并同时移动权重
	double dTempI = 0.0;
	double dTempJ = 0.0;
	double t0 = 0.0;
	double t1 = 0.0;

	for(i = 0; i < iSampleDays - 1; i++)
	{
		for(int j = i+1; j < iSampleDays; j++)
		{
			double dTempI = matrixM2.GetData(i, 0);
			double dTempJ = matrixM2.GetData(j, 0);
			if(dTempI > dTempJ)
			{
				t0 = dTempI; 
				t1=	 matrixM2.GetData(i, 1);


				matrixM2.SetData(i, 0, dTempJ);
				matrixM2.SetData(i, 1, matrixM2.GetData(j, 1));

				matrixM2.SetData(j, 0, t0);
				matrixM2.SetData(j, 1, t1);
			}
		}
	}

	t0 = 0.0;
	for (i=0; i < iSampleDays; i++)
	{
		t0 += matrixM2.GetData(i, 1);
		if (t0 > dAlpha)
		{
			break;		
		}
	}
	dVar = matrixM2.GetData(i, 0);

	t0 = 0.0;
	t1 = 0.0;
	for(int j=0; j <= i; j++)
	{
		t1 += matrixM2.GetData(j, 0) * matrixM2.GetData(j, 1); 
		t0 += matrixM2.GetData(j,1);
	}
	dCVar = t1 / t0;

#ifdef _DEBUG
	long lEnd = long(::GetTickCount());
	long lDiff = lEnd - lStart;
#endif

	return true;
}

bool CCommonMathMethod::GenerateNormalRand(Tx::Core::CArrayMatrix& matrixNormalRand)
{
	int nRow = matrixNormalRand.GetRowCount();
	int nCol = matrixNormalRand.GetColCount();

	if (1 > nRow || 1 > nCol)
	{
		return false;
	}

	srand((unsigned)time(NULL));
	double dTemp = 0.0;
	for (int i = 0; i < nRow; i++)
	{
		for (int j = 0; j < nCol; j++)
		{
			dTemp = 0.0;
			for (int k = 0; k < 6; k++)
			{
				dTemp += (rand() - rand())/32768.0;
			}
			matrixNormalRand.SetData(i, j, dTemp);
		}
	}
	return true;
}
//=======================================================================================================
//=======================================================================================================
//=======================================================================================================

CVarCalcMethod::CVarCalcMethod(CVarCalcParam *pParam, CVarDataSource *pDataSrc)
{
	this->m_pVarDataSource = pDataSrc;
	this->m_pVarCalcParam = pParam;
}

CVarCalcMethod::~CVarCalcMethod()
{
}
bool CVarCalcMethod::Calculate(double& dVar, double& dCVar)
{
	dVar = 0.0;
	dCVar = 0.0;
	if (NULL == this->m_pVarCalcParam || NULL == this->m_pVarDataSource)
	{
		return false;
	}
	return true;
}

bool CVarCalcMethod::Covar(Tx::Core::CArrayMatrix& matrixCov, Tx::Core::CArrayMatrix& matrixAvg)
{
	/*1 参数检查*/
	if (!this->m_pVarCalcParam->IsValid())
	{
		return false;
	}
	/*2 获取收益率数据矩阵,并调整样本天数*/ 
	Tx::Core::CArrayMatrix matrixDsCopy;
	this->m_pVarDataSource->GetPLRateMatrix(this->m_pVarCalcParam->GetPLType(), 1, matrixDsCopy);
	int iSamplesCount = this->m_pVarDataSource->GetSamplesCount();
	int iSamplesDays = matrixDsCopy.GetRowCount();

	double dLmd = this->m_pVarCalcParam->GetLmd();
	
	/*3 Generate LMD series*/
	Tx::Core::CArrayMatrix matrixLMD;
	matrixLMD.SetRowCol(1, iSamplesDays);

	double dTemp = 1.0;
	double dSum = 0.0;

	int i = 0;
	for(i = iSamplesDays - 1; i >= 0; i--)// 0 < LMD <= 1, 日期越近比重越大 
	{
		matrixLMD.SetData(0, i, dTemp);
		dSum += dTemp; 
		dTemp *= dLmd; 
	}
	for(i=0; i<iSamplesDays; i++)
	{
		matrixLMD.SetData(0, i, matrixLMD.GetData(0, i)/dSum);
	}

	/*4 计算各样本均值*/
	double dAvg = 0.0;
	matrixAvg.SetRowCol(iSamplesCount, 1);
	for (i = 0; i < iSamplesCount; i++)
	{
		dAvg = 0.0;
		for (int j = 0; j < iSamplesDays; j++)
		{
			dAvg += matrixLMD.GetData(0, j) * matrixDsCopy.GetData(j, i);
		}
		matrixAvg.SetData(i, 0, dAvg);
	}

	/*5 计算协方差矩阵*/
	double dTempVal = 0.0;
	for (i = 0; i < iSamplesCount; i++)
	{
		for(int j = 0; j < iSamplesCount; j++)
		{
			dTempVal = 0.0;
			for (int k = 0; k < iSamplesDays; k++)
			{
				dTempVal += matrixLMD.GetData(0, k) * matrixDsCopy.GetData(k, i) * matrixDsCopy.GetData(k, j);
			}
			dTempVal -= matrixAvg.GetData(i, 0) * matrixAvg.GetData(j, 0);

			matrixCov.SetData(i,j, dTempVal);
		}
	}

	return true;
}

bool CVarCalcMethod::Covar2(Tx::Core::CArrayMatrix& matrixCov, Tx::Core::CArrayMatrix& matrixAvg)
{
	/*1 参数检查*/
	if (!this->m_pVarCalcParam->IsValid())
	{
		return false;
	}
	/*2 获取收益率数据矩阵,并调整样本天数*/ 
	Tx::Core::CArrayMatrix matrixDsCopy;
	this->m_pVarDataSource->GetPLRateMatrix(this->m_pVarCalcParam->GetPLType(), 1, matrixDsCopy);
	int iSamplesCount = this->m_pVarDataSource->GetSamplesCount();
	int iSamplesDays = matrixDsCopy.GetRowCount();

	double dLmd = this->m_pVarCalcParam->GetLmd();
	
	/*3 Generate LMD series*/
	Tx::Core::CArrayMatrix matrixLMD;
	matrixLMD.SetRowCol(1, iSamplesDays);

	double dTemp = 1.0;
	double dSum = 0.0;

	int i = 0;
	for(i = iSamplesDays - 1; i >= 0; i--)// 0 < LMD <= 1, 日期越近比重越大 
	{
		matrixLMD.SetData(0, i, dTemp);
		dSum += dTemp; 
		dTemp *= dLmd; 
	}
	for(i=0; i<iSamplesDays; i++)
	{
		matrixLMD.SetData(0, i, matrixLMD.GetData(0, i)/dSum);
	}

	/*4 计算各样本均值*/
	double dAvg = 0.0;
	matrixAvg.SetRowCol(iSamplesCount, 1);
	for (i = 0; i < iSamplesCount; i++)
	{
		dAvg = 0.0;
		for (int j = 0; j < iSamplesDays; j++)
		{
			dAvg += matrixDsCopy.GetData(j, i);//matrixLMD.GetData(0, j) * 
		}
		matrixAvg.SetData(i, 0, dAvg);
	}

	/*5 中心化*/
	for (i = 0; i < iSamplesCount; i++)
	{
		dAvg = matrixAvg.GetData(i, 0);
		for (int j = 0; j < iSamplesDays; j++)
		{
			matrixDsCopy.SetData(j,  i,  (matrixDsCopy.GetData(j,i) - dAvg));//matrixLMD.GetData(0, j) *
		}
	}

	/*6 计算协方差矩阵*/
	double dTempVal = 0.0;
	for (i = 0; i < iSamplesCount; i++)
	{
		for(int j = 0; j < iSamplesCount; j++)
		{
			dTempVal = 0.0;
			for (int k = 0; k < iSamplesDays; k++)
			{
				dTempVal += matrixLMD.GetData(0, k) * matrixDsCopy.GetData(k, i) * matrixDsCopy.GetData(k, j);
			}
			matrixCov.SetData(i,j, dTempVal);
		}
	}

	return true;
}

bool CVarCalcMethod::Covar3(Tx::Core::CArrayMatrix& matrixCov, Tx::Core::CArrayMatrix& matrixAvg)
{
	/*1 参数检查*/
	if (!this->m_pVarCalcParam->IsValid())
	{
		return false;
	}
	/*2 获取收益率数据矩阵,并调整样本天数*/ 
	Tx::Core::CArrayMatrix matrixDsCopy;
	this->m_pVarDataSource->GetPLRateMatrix(this->m_pVarCalcParam->GetPLType(), 1, matrixDsCopy);
	int iSamplesCount = this->m_pVarDataSource->GetSamplesCount();
	int iSamplesDays = matrixDsCopy.GetRowCount();

	double dLmd = this->m_pVarCalcParam->GetLmd();
	
	/*3 Generate LMD series*/
	Tx::Core::CArrayMatrix matrixLMD;
	matrixLMD.SetRowCol(1, iSamplesDays);

	double dTemp = 1.0;
	double dSum = 0.0;

	int i = 0;
	for(i = iSamplesDays - 1; i >= 0; i--)// 0 < LMD <= 1, 日期越近比重越大 
	{
		matrixLMD.SetData(0, i, dTemp);
		dSum += dTemp; 
		dTemp *= dLmd; 
	}
	for(i=0; i<iSamplesDays; i++)
	{
		matrixLMD.SetData(0, i, matrixLMD.GetData(0, i)/dSum);
	}

	/*4 Set weights for this profit rate series*/
	for (i = 0; i < iSamplesCount; i++)
	{
		for (int j = 0; j < iSamplesDays; j++)
		{
			matrixDsCopy.SetData(j, i, matrixLMD.GetData(0, j) * matrixDsCopy.GetData(j, i));
		}
	}

	/*5 Average calculation*/
	double dAvg = 0.0;
	Tx::Core::CArrayMatrix matrixAvg;
	matrixAvg.SetRowCol(iSamplesCount, 1);
	for (i = 0; i < iSamplesCount; i++)
	{
		dAvg = 0.0;
		for (int j = 0; j < iSamplesDays; j++)
		{
			dAvg += matrixDsCopy.GetData(j, i);
		}
		matrixAvg.SetData(i, 0, dAvg);
	}

	/*6 中心化*/
	for (i = 0; i < iSamplesCount; i++)
	{
		dAvg = matrixAvg.GetData(i, 0);
		for (int j = 0; j < iSamplesDays; j++)
		{
			matrixDsCopy.SetData(j,  i,  (matrixDsCopy.GetData(j,i) - dAvg));
		}
	}

	/*7 计算协方差矩阵*/
	double dTempVal = 0.0;
	for (i = 0; i < iSamplesCount; i++)
	{
		for(int j = 0; j < iSamplesCount; j++)
		{
			dTempVal = 0.0;
			for (int k = 0; k < iSamplesDays; k++)
			{
				dTempVal += matrixDsCopy.GetData(k, i) * matrixDsCopy.GetData(k, j);
			}
			matrixCov.SetData(i,j, dTempVal);
		}
	}

	return true;
}

//=======================================================================================================
//=======================================================================================================
//=======================================================================================================

CVarNormalMethod::CVarNormalMethod(CVarCalcParam *pParam, CVarDataSource *pDataSrc) : CVarCalcMethod(pParam, pDataSrc) 
{

}

CVarNormalMethod::~CVarNormalMethod()
{
}

bool CVarNormalMethod::Calculate(double& dVar, double& dCVar)
{
	dVar = 0.0;
	dCVar = 0.0;
	if (NULL == this->m_pVarCalcParam || NULL == this->m_pVarDataSource)
	{
		return false;
	}
	if (!m_pVarCalcParam->IsValid() || !this->m_pVarDataSource->AssertValid())
	{
		return false;
	}

	int nSamples = this->m_pVarDataSource->GetSamplesCount();
	int nSampleDays = this->m_pVarDataSource->GetSampleDays();
	if (nSamples <= 0 || nSampleDays <= 0) 
	{
		return false;
	}
	Tx::Core::CArrayMatrix matrixCov;
	matrixCov.SetRowCol(nSamples, nSamples);
	Tx::Core::CArrayMatrix matrixAvg;
	matrixAvg.SetRowCol(nSamples, 1);
	this->Covar(matrixCov, matrixAvg);

	double dZ_Alpha = this->Percentile(this->m_pVarCalcParam->GetAlpha());
	if (fabs(dZ_Alpha) < 0.000001)
	{
		return false;
	}

	Tx::Core::CArrayMatrix matrixValue;
	this->m_pVarDataSource->GetLatestValeMatrix(matrixValue);
	Tx::Core::CArrayMatrix matrixVariant;

	matrixVariant = matrixValue.T() * matrixCov * matrixValue;
	double dVariant =  this->m_pVarCalcParam->GetHoldingDays() * matrixVariant.GetData(0,0);

	//double dProfitPerDay = (matrixAvg.T() * matrixValue).GetData(0,0);
	dVar = dZ_Alpha * sqrt(dVariant);// +  dProfitPerDay* this->m_pVarCalcParam->GetHoldingDays();
	dCVar = -0.39894 * exp(-dZ_Alpha * dZ_Alpha * 0.5) * sqrt(dVariant) / this->m_pVarCalcParam->GetAlpha();
	//*average=sclprd(avg, coeff, n);

	return true;
}

//正态分布不同置信度对应的Z值
double CVarNormalMethod::Percentile(double dAlpha)
{
	switch ((int)(dAlpha*100))
	{
	case 10: 
		return -1.282;
	case 5:
		return -1.645;
	case 1:
		return -2.327;
	default: return 0;
	}
}

//=======================================================================================================
//=======================================================================================================
//=======================================================================================================
CHistorSimulationMethod::CHistorSimulationMethod(CVarCalcParam *pParam, CVarDataSource *pDataSrc) : CVarCalcMethod(pParam, pDataSrc) 
{
}
CHistorSimulationMethod::~CHistorSimulationMethod()
{
}
bool CHistorSimulationMethod::Calculate(double& dVar, double& dCVar)
{
	dVar = 0.0;
	dCVar = 0.0;
	if (NULL == this->m_pVarCalcParam || NULL == this->m_pVarDataSource)
	{
		return false;
	}

	if (!m_pVarCalcParam->IsValid() || !this->m_pVarDataSource->AssertValid())
	{
		return false;
	}
#ifdef _DEBUG
	long lStart = long(::GetTickCount());
#endif
	//收益率比较小，当差不多时可替代使用，推荐使用指数收益率

	/*2 获取收益率数据矩阵,并调整样本天数*/ 
	Tx::Core::CArrayMatrix matrixDsCopy;
	this->m_pVarDataSource->GetPLRateMatrix(this->m_pVarCalcParam->GetPLType(), this->m_pVarCalcParam->GetHoldingDays(), matrixDsCopy);

	int iSamplesCount = this->m_pVarDataSource->GetSamplesCount();
	int iSamplesDays = matrixDsCopy.GetRowCount();
	Tx::Core::CArrayMatrix matrixValue;
	this->m_pVarDataSource->GetLatestValeMatrix(matrixValue);

	int i = 0;
	double dTemp = 0.0;
	for (; i < iSamplesDays; i++)
	{
		for(int j = 0; j < iSamplesCount; j++)
		{
			dTemp = matrixValue.GetData(j, 0) * (exp(matrixDsCopy.GetData(i, j)) - 1);
			matrixDsCopy.SetData(i, j, dTemp);
		}
	}

	Tx::Core::CArrayMatrix matrixM2;
	matrixM2.SetRowCol(iSamplesDays, 2);

	for (i = 0; i < iSamplesDays; i++)
	{
		dTemp =0.0;
		for(int j = 0; j < iSamplesCount; j++)
		{
			dTemp += matrixDsCopy.GetData(i, j);
		}
		matrixM2.SetData(i, 0, dTemp);
	}

	/*3 Generate LMD series*/
	double dLmd = this->m_pVarCalcParam->GetLmd();
		
	dTemp = 1.0;
	for (i = iSamplesDays - 1; i >= 0; i--)// 0 < LMD <= 1, 日期越近比重越大 
	{
		double dValue = matrixM2.GetData(i, 0);
		matrixM2.SetData(i, 1, dTemp);
		dTemp *= dLmd; 
	}
#ifdef _DEBUG
	long lEnd = long(::GetTickCount());
	long lDiff = lEnd - lStart;
#endif
	//获得分为数对应的资产值
	return CCommonMathMethod::PercentileFromDencity(this->m_pVarCalcParam->GetAlpha(), matrixM2, dVar, dCVar);
}
//=======================================================================================================
//=======================================================================================================
//=======================================================================================================
CMonteCarloMethod::CMonteCarloMethod(CVarCalcParam *pParam, CVarDataSource *pDataSrc) : CVarCalcMethod(pParam, pDataSrc) 
{
}
CMonteCarloMethod::~CMonteCarloMethod()
{
}
bool CMonteCarloMethod::Calculate(double& dVar, double& dCVar)
{
	dVar = 0.0;
	dCVar = 0.0;
	if (NULL == this->m_pVarCalcParam || NULL == this->m_pVarDataSource)
	{
		return false;
	}
	if (!m_pVarCalcParam->IsValid() || !this->m_pVarDataSource->AssertValid())
	{
		return false;
	}

	int nSamples = this->m_pVarDataSource->GetSamplesCount();
	int nSampleDays = this->m_pVarDataSource->GetSampleDays();
	if (nSamples <= 0 || nSampleDays <= 0) 
	{
		return false;
	}

	//Step2：计算方差－协方差的预测值： ，其中 是第 种股票的收益率平均值。
	Tx::Core::CArrayMatrix matrixCov;
	matrixCov.SetRowCol(nSamples, nSamples);
	Tx::Core::CArrayMatrix matrixAvg;
	matrixAvg.SetRowCol(nSamples, 1);
	this->Covar(matrixCov, matrixAvg);
	
	//Step3：对方差－协方差矩阵∑，进行Cholesky分解，使得 ∑，∑是一个T*T的对角方阵，矩阵A为T*T的下三角阵，A中的元素 ，  j=i+1,i+2,…,T。
	this->CovMatrixCholesky(matrixCov);

	Tx::Core::CArrayMatrix matrixValue;
	this->m_pVarDataSource->GetLatestValeMatrix(matrixValue);

	int iM = this->m_pVarCalcParam->GetSimulationDays();
	Tx::Core::CArrayMatrix matrixM2;
	matrixM2.SetRowCol(iM, 2);

	//Step4：产生序列Y(K*iM)，Y中的元素为k个服从标准正态分布N(0,1)的随机数。//同时生成防止重复数据
	Tx::Core::CArrayMatrix matrixNormalRand;
	matrixNormalRand.SetRowCol(nSamples, iM);
	if (!CCommonMathMethod::GenerateNormalRand(matrixNormalRand))
	{
		return false;
	}

	//Step5：计算协方差为∑的k个随机数Z(K*1)：Z＝A*Y。
	Tx::Core::CArrayMatrix matrixResult;
	matrixResult.SetRowCol(nSamples, iM);
	matrixResult = matrixCov * matrixNormalRand;

	for (int i = 0; i < iM; i++)
	{
		int iT = this->m_pVarCalcParam->GetHoldingDays();
		double dTemp = 0.0;
		double dTempTotal = 0.0; 
		for(int j = 0; j< nSamples; j++)
		{
			dTemp = matrixResult.GetData(j,i) * sqrt((double)iT);
			dTemp += matrixAvg.GetData(j, 0) * iT;

			dTempTotal += dTemp * matrixValue.GetData(j, 0);
		}
		matrixM2.SetData(i, 0, dTempTotal);
		matrixM2.SetData(i, 1, 1);
	}

	/*strText = _T("M2#\r\n");
	for (int i = 0; i < iM; i++)
	{
		CString strTemp;
		strTemp.Format(_T("%f\t%f\t"), matrixM2.GetData(i, 0), matrixM2.GetData(i, 1));

		strText += strTemp;
	}
	CTestLog::WiriteToLogFile(strText);
	*/
	//step6: 获得分位数对应的资产值
	return CCommonMathMethod::PercentileFromDencity(this->m_pVarCalcParam->GetAlpha(), matrixM2, dVar, dCVar);
}

//协方差矩阵Cholesky分解
bool CMonteCarloMethod::CovMatrixCholesky(Tx::Core::CArrayMatrix& matrixCov)
{
	int iSamplesCount = this->m_pVarDataSource->GetSamplesCount();
	Tx::Core::CArrayMatrix matrixTemp;
	matrixTemp.SetRowCol(iSamplesCount, iSamplesCount);

	int i = 0;
	for (i = 0; i < iSamplesCount; i++)
	{
		for (int j = 0; j < iSamplesCount; j++)
		{
			matrixTemp.SetData(i, j, 0.0);
		}
	}
	double dTemp1 = 0.0;
	double dTemp2 = 0.0;
	double dTemp3 = 0.0;
	double dValue = 0.0;
	for (i = 0; i < iSamplesCount; i++)
	{
		for (int j = 0; j <= i; j++)
		{
			dTemp1 = 0.0;
			dTemp2 = 0.0;
			for (int k = 0; k < j; k++)
			{
				dValue = matrixTemp.GetData(j, k);
				dTemp1 += matrixTemp.GetData(i, k) * dValue;
				dTemp2 +=  dValue* dValue;
			}
			dValue = matrixCov.GetData(i, j) - dTemp1;
			dTemp3 = matrixCov.GetData(j, j) - dTemp2;
			if (dTemp3 < 0.000001)
			{
				dValue = 0.0;
			}
			else
			{
				dValue /= sqrt(dTemp3);
			}
			matrixTemp.SetData(i,j, dValue);
		}
	}

	matrixCov = matrixTemp;
	return true;
}
}
}


