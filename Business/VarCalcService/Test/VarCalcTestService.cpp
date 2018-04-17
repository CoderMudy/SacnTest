#include "stdafx.h"
#include "VarCalcTestService.h"

#include "../VarCalcConfig.h"
#include "../VarCalc/VarCalcMethod.h"
#include "../VarCalc/VarCalcDataSource.h"
#include "../VarCalc/VarCalcGlobal.h"
#include "functions.h"

#include "TestLog.h"

CVarCalcTestService::CVarCalcTestService()
{
}
CVarCalcTestService::CVarCalcTestService(CString& strConfigPath, long lHodingBeginDate)
{
	this->m_strConfigPath = strConfigPath;
	this->m_lHodingBeginDate = lHodingBeginDate;
	m_pVarDataProvider = new CVarDataProvider;
}

CVarCalcTestService::~CVarCalcTestService()
{
	if (NULL != m_pVarDataProvider)
	{
		delete m_pVarDataProvider;
	}
}
bool CVarCalcTestService::AddPotofolioItem(long lEntityId, double dHodingNumer)
{
	return m_pVarDataProvider->AddInvestItem(lEntityId, dHodingNumer);
}
bool CVarCalcTestService::CalcVar(double &dVar, double &dCVar)
{
	Tx::VAR::CVarCalcParam paramVar;

	CVarCalcConfig configVar;
	paramVar = configVar.LoadParam();
	int iVarCalcMethod = configVar.LoadCalcMethod();

	if (!paramVar.IsValid() || iVarCalcMethod < 0 || iVarCalcMethod >= CVarCalcConfig::eVarCalcMethodCount)
	{
		return false;
	}
	if (NULL == this->m_pVarDataProvider)
	{
		return false;
	}

	this->m_pVarDataProvider->SetParams(this->m_lHodingBeginDate, paramVar.GetSamplesCount());
	if (!this->m_pVarDataProvider->AssetSetting())
	{
		return false;
	}

	Tx::VAR::CVarDataSource ds;
	if (!this->m_pVarDataProvider->UpdateDS(&ds))
	{
		return false;
	}

	if (!ds.AssertValid())
	{
		return false;
	}	

	std::vector<int> arrCalcMethod;
	switch (iVarCalcMethod)
	{
	case CVarCalcConfig::eVarNormalMethod:
		{
			Tx::VAR::CVarCalcMethod *pMethod = new Tx::VAR::CVarNormalMethod(&paramVar, &ds);
			if (!pMethod->Calculate(dVar, dCVar))
			{
				delete pMethod; 
				return false;
			}
			if (fabs(dVar) < 0.000001 && fabs(dCVar) < 0.000001)
			{
				delete pMethod; 
				return false;
			}
			delete pMethod;
		}
		break;
	case CVarCalcConfig::eVarHisSimuMethod:
		{
			Tx::VAR::CVarCalcMethod *pMethod = new Tx::VAR::CHistorSimulationMethod(&paramVar, &ds);
			if (!pMethod->Calculate(dVar, dCVar))
			{
				delete pMethod; 
				return false;
			}
			if (fabs(dVar) < 0.000001 && fabs(dCVar) < 0.000001)
			{
				delete pMethod; 
				return false;
			}
			delete pMethod;
		}
		break;
	case CVarCalcConfig::eVarMonteCarloMethod:
		{
			Tx::VAR::CVarCalcMethod *pMethod = new Tx::VAR::CMonteCarloMethod(&paramVar, &ds);
			if (!pMethod->Calculate(dVar, dCVar))
			{
				delete pMethod; 
				return false;
			}
			if (fabs(dVar) < 0.000001 && fabs(dCVar) < 0.000001)
			{
				delete pMethod; 
				return false;
			}
			delete pMethod;
		}
		break;
	}
	return true;
}

bool CVarCalcTestService::TestAllParamCombos(bool withTestData)
{
	if (NULL == this->m_pVarDataProvider)
	{
		return false;
	}

		int iDate = 20100523;
	int iSamplesDays = 250;


	if (withTestData)
	{
		this->m_pVarDataProvider->Clear();
		this->m_pVarDataProvider->SetParams(iDate, iSamplesDays);
		this->m_pVarDataProvider->AddInvestItem(1, 2000);
		this->m_pVarDataProvider->AddInvestItem(2, 2000);
		this->m_pVarDataProvider->AddInvestItem(74, 2000);
	}
	else
	{
		this->m_pVarDataProvider->SetParams(iDate, iSamplesDays);

	}

	if ( !this->m_pVarDataProvider->AssetSetting())
	{
		return false;
	}

	Tx::VAR::CVarDataSource ds;
	if (withTestData)
	{
		CString strPath;
		strPath.Format(_T("c://Var/data/"));
		if (!this->m_pVarDataProvider->UpdateDSFromFile(strPath, &ds))
		{
			return false;
		}
	}
	else 
	{
		if (!this->m_pVarDataProvider->UpdateDS(&ds))
		{
			return false;
		}
	}

	if (!ds.AssertValid())
	{
		return false;
	}	

	CString strPath;
	strPath.Format(_T("C:/Var/result1/result.txt"));//, idx
	CTestLog log(strPath);
	log.ClearLog();
	
	Tx::VAR::CVarCalcParam paramVar;
	for (int idx = 0; idx < 4000; idx++)
	{
		if (!this->GetParamOneByOne(idx, paramVar))
		{
			break;
		}

		CString strText;
		CString strTemp;

		CString strPLType[] = {_T("收益"), _T("百分比收益率"), _T("对数收益率"), _T("Price")};
		strTemp.Format(_T("测试参数\r\n收益率类型:%s, Alpha:%f, Holding:%d, Lmd:%f, Simulation:%d\r\n"),
			strPLType[paramVar.GetPLType()], paramVar.GetAlpha(), paramVar.GetHoldingDays(), paramVar.GetLmd(), paramVar.GetSimulationDays());
		strText += strTemp;
		strTemp.Format(_T("=====================================================\r\n"));
		strText += strTemp;

		long lStart = (long)(::GetTickCount());

		if (this->TestNewFunctions(strTemp, ds, paramVar))
		{
			long lEnd = (long)(::GetTickCount());
			strText += strTemp;

			strTemp.Format(_T("所花时间:%dms\r\n"), (lEnd -lStart));


			strText += strTemp;
			strTemp.Format(_T("=====================================================\r\n"));
			strText += strTemp;
		}
		lStart = (long)(::GetTickCount());
		if (this->TestOldFunctions(strTemp, ds, paramVar))
		{
			long lEnd = (long)(::GetTickCount());
			strText += strTemp;
			strTemp.Format(_T("所花时间:%dms\r\n"), (lEnd -lStart));
			strText += strTemp;

			strTemp.Format(_T("=====================================================\r\n"));
			strText += strTemp;
		}

		//AfxMessageBox(strText);
		log.WiriteToLogFile(strText);
	}

	return true;
}
bool CVarCalcTestService::TestDefaultParam(bool withTestData)
{
	if (NULL == this->m_pVarDataProvider)
	{
		return false;
	}
	int iDate = 20100523;
	int iSamplesDays = 250;


	if (withTestData)
	{
		this->m_pVarDataProvider->Clear();
		this->m_pVarDataProvider->SetParams(iDate, iSamplesDays);
		this->m_pVarDataProvider->AddInvestItem(1, 2000);
		this->m_pVarDataProvider->AddInvestItem(2, 2000);
		this->m_pVarDataProvider->AddInvestItem(74, 2000);
	}
	else
	{
		this->m_pVarDataProvider->SetParams(iDate, iSamplesDays);

	}

	if ( !this->m_pVarDataProvider->AssetSetting())
	{
		return false;
	}

	Tx::VAR::CVarDataSource ds;
	if (withTestData)
	{
		CString strPath;
		strPath.Format(_T("c:/Var/data/"));
		if (!this->m_pVarDataProvider->UpdateDSFromFile(strPath, &ds))
		{
			return false;
		}
	}
	else 
	{
		if (!this->m_pVarDataProvider->UpdateDS(&ds))
		{
			return false;
		}
	}

	if (!ds.AssertValid())
	{
		return false;
	}	

	CString strPath;
	if (withTestData)
	{
		strPath.Format(_T("C://Var/result/result_defparam.txt"));
	}
	else
	{
		strPath.Format(_T("C://Var/result1/result_defparam.txt"));
	}
	CTestLog log(strPath);
	log.ClearLog();

	Tx::VAR::CVarCalcParam paramVar;

	CString strText;
	CString strTemp;

	CString strPLType[] = {_T("收益"), _T("百分比收益率"), _T("对数收益率"), _T("Price")};
	strTemp.Format(_T("测试参数\r\n收益率类型:%s, Alpha:%f, Holding:%d, Lmd:%f, Simulation:%d\r\n"),
		strPLType[paramVar.GetPLType()], paramVar.GetAlpha(), paramVar.GetHoldingDays(), paramVar.GetLmd(), paramVar.GetSimulationDays());
	strText += strTemp;
	strTemp.Format(_T("=====================================================\r\n"));
	strText += strTemp;

	long lStart = (long)(::GetTickCount());
	if (this->TestOldFunctions(strTemp, ds, paramVar))
	{
		long lEnd = (long)(::GetTickCount());
		strText += strTemp;
		strTemp.Format(_T("所花时间:%dms\r\n"), (lEnd -lStart));
		strText += strTemp;

		strTemp.Format(_T("=====================================================\r\n"));
		strText += strTemp;
	}

	lStart = (long)(::GetTickCount());
	if (this->TestNewFunctions(strTemp, ds, paramVar))
	{
		long lEnd = (long)(::GetTickCount());
		strText += strTemp;

		strTemp.Format(_T("所花时间:%dms\r\n"), (lEnd -lStart));


		strText += strTemp;
		strTemp.Format(_T("=====================================================\r\n"));
		strText += strTemp;
	}

	AfxMessageBox(strText);
	log.WiriteToLogFile(strText);

	return true;
}

bool CVarCalcTestService::GetParamOneByOne(int iIndex, Tx::VAR::CVarCalcParam &paramVar)
{
	int iTotalTestTimes = 1;

	int iPLType[2] = {Tx::VAR::ePercentProfitRate, Tx::VAR::eLogProfitRate};
	iTotalTestTimes *= 2;

	double dAlpha[3] = {0.10, 0.05, 0.01};
	iTotalTestTimes *= 3;

	int iHodingDays[6] = {1, 2, 3, 4, 5, 10};
	iTotalTestTimes *= 6;

	double dLmd[13] = {0.01, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.83, 0.9, 0.99, 1};//lmd > 0.00000001 && lmd < 1.00000001
	iTotalTestTimes *= 13;

	int iSimulationTimes[7] = {100, 200, 500, 1000, 1500, 2000, 5000};
	iTotalTestTimes *= 7;


	if (iIndex < 0 || iIndex >= iTotalTestTimes)
	{
		return false;
	}

	int iIdxBegin = 0;

	int idxCount[5] = {2, 3, 6, 13, 7};
	int paramIndex[5];
	for (int i = 0; i < 5; i++)
	{
		int iCountPerSeg = iTotalTestTimes / idxCount[i];
		paramIndex[i] = (iIndex - iIdxBegin) /iCountPerSeg;
		iIdxBegin +=  paramIndex[i] * iCountPerSeg;
		iTotalTestTimes /= idxCount[i];
	}

	Tx::VAR::CVarCalcParam paramVar1( iPLType[paramIndex[0]], iHodingDays[paramIndex[2]],dAlpha[paramIndex[1]],  dLmd[paramIndex[3]], iSimulationTimes[paramIndex[4]], 250);
	paramVar = paramVar1;
	return true;
}

bool CVarCalcTestService::TestNewFunctions(CString& strText, Tx::VAR::CVarDataSource& ds, Tx::VAR::CVarCalcParam &paramVar, int iVarCalcMethod)
{
	if (!paramVar.IsValid() || !ds.AssertValid())
	{
		return false;
	}

	strText = _T("计算结果:\r\n");
	CString strTemp = _T("");

	std::vector<int> arrCalcMethod;
	switch (iVarCalcMethod)
	{
	case -1:
		{
			arrCalcMethod.push_back(0);
			arrCalcMethod.push_back(1);
			arrCalcMethod.push_back(2);
		}
		break;
	case 0:
		{
			arrCalcMethod.push_back(0);
		}
		break;
	case 1:
		{
			arrCalcMethod.push_back(1);
		}
		break;
	case 2:
		{
			arrCalcMethod.push_back(2);
		}
		break;
	default:break;
	}

	for (std::vector<int>::iterator iter  = arrCalcMethod.begin(); iter != arrCalcMethod.end(); iter++)
	{
		switch (*iter)
		{
		case 0:
			{
				Tx::VAR::CVarCalcMethod *pMethod = new Tx::VAR::CVarNormalMethod(&paramVar, &ds);
				double dVar1 = 0.0;
				double dCVar1 = 0.0;
				if (!pMethod->Calculate(dVar1, dCVar1))
				{
					return false;
				}
				if (fabs(dVar1) < 0.000001 && fabs(dCVar1) < 0.000001)
				{
					return false;
				}
				delete pMethod;

				strTemp.Format(_T("正态分布法\tVar: %f\tCVar: %f\r\n"), dVar1, dCVar1);
				strText += strTemp;
			}
			break;
		case 1:
			{
				Tx::VAR::CVarCalcMethod *pMethod2 = new Tx::VAR::CHistorSimulationMethod(&paramVar, &ds);
				double dVar2 = 0.0;
				double dCVar2 = 0.0;
				if (!pMethod2->Calculate(dVar2, dCVar2))
				{
					return false;
				}
				if (fabs(dVar2) < 0.000001 && fabs(dCVar2) < 0.000001)
				{
					return false;
				}
				delete pMethod2;

				strTemp.Format(_T("历史模拟法\tVar: %f\tCVar: %f\r\n"), dVar2, dCVar2);
				strText += strTemp;

			}
			break;
		case 2:
			{
				Tx::VAR::CVarCalcMethod *pMethod3 = new Tx::VAR::CMonteCarloMethod(&paramVar, &ds);
				double dVar3 = 0.0;
				double dCVar3 = 0.0;
				if (!pMethod3->Calculate(dVar3, dCVar3))
				{
					return false;
				}
				if (fabs(dVar3) < 0.000001 && fabs(dCVar3) < 0.000001)
				{
					return false;
				}
				delete pMethod3;

				strTemp.Format(_T("蒙特卡罗法\tVar: %f\tCVar: %f\r\n"), dVar3, dCVar3);
				strText += strTemp;
			}
			break;
		}

	}

	return true;
}

bool CVarCalcTestService::TestOldFunctions(CString& strText, Tx::VAR::CVarDataSource& ds, Tx::VAR::CVarCalcParam &paramVar, int iVarCalcMethod)
{
	if (!paramVar.IsValid() || !ds.AssertValid())
	{
		return false;
	}

	int n = ds.GetSamplesCount();
	int m = ds.GetSampleDays();

	Tx::Core::CArrayMatrix matrixPrices;
	ds.GetPricesMatrix(matrixPrices);
	if (m != matrixPrices.GetRowCount() || n != matrixPrices.GetColCount())
	{
		return false;
	}

	Tx::Core::CArrayMatrix matrixStorage;
	ds.GetStorageMatrix(matrixStorage);
	if (n != matrixStorage.GetRowCount())
	{
		return false;
	}

	double *sample = new double[m * n];
	double *coeff = new double[n]; 

	double dVar = 0.0;
	double dCVar = 0.0;

	strText = _T("计算结果2:\r\n");
	CString strTemp = _T("");
	CString strDesCrip[] = {_T("正态分布法"), _T("历史模拟法"), _T("蒙特卡罗法")};

	std::vector<int> arrCalcMethod;
	switch (iVarCalcMethod)
	{
	case -1:
		{
			arrCalcMethod.push_back(0);
			arrCalcMethod.push_back(1);
			arrCalcMethod.push_back(2);
		}
		break;
	case 0:
		{
			arrCalcMethod.push_back(0);

		}
		break;
	case 1:
		{
			arrCalcMethod.push_back(1);
		}
		break;
	case 2:
		{
			arrCalcMethod.push_back(2);

		}
		break;
	default:break;
	}
	for (std::vector<int>::iterator iter  = arrCalcMethod.begin(); iter != arrCalcMethod.end(); iter++)
	{
		for (int j = 0; j < n; j++)
		{
			for (int k = 0; k < m; k++)
			{
				sample[n * k + j] = matrixPrices.GetData(k, j);
			}
			coeff[j] = matrixStorage.GetData(j, 0);
		}
		caculate_var
			(sample, n, m, 
			paramVar.GetSimulationDays(),
			paramVar.GetLmd(),
			coeff,
			paramVar.GetHoldingDays(),
			paramVar.GetAlpha(),
			&dVar,
			&dCVar,
			*iter,
			paramVar.GetPLType()
			);

		strTemp.Format(_T("%s\tVar: %f\tCVar: %f\r\n"), strDesCrip[*iter], dVar, dCVar);
		strText += strTemp;
	}

	delete[] sample;
	delete[] coeff;

	return true;
}

/*
bool CVarCalcTest::UIParamTest()
{
	CVarCalcConfigDlg dlgConfig;
	if (IDOK == dlgConfig.DoModal())
	{
		CVarCalcParam paramVar;
		CVarCalcConfig configVar;
		paramVar = configVar.LoadParam();
		int iVarCalcMethod = configVar.LoadCalcMethod();

		int iDate = 20100505;
		int iSamplesDays = 1000;
		CVarDataProvider providerVarDS(iDate, iSamplesDays);
		providerVarDS.AddInvestItem(1, 2000);
		providerVarDS.AddInvestItem(2, 2000);
		providerVarDS.AddInvestItem(74, 2000);
		CVarDataSource ds;
		if (!providerVarDS.UpdateDS(&ds))
		{
			return false;
		}

		if (!ds.AssertValid())
		{
			return false;
		}	

		CString strPath;
		strPath.Format(_T("C:/Var/result/result.txt"));
		CTestLog log(strPath);
		log.ClearLog();

		CString strText;
		CString strTemp;

		CString strPLType[] = {_T("收益"), _T("百分比收益率"), _T("对数收益率"), _T("Price")};
		strTemp.Format(_T("测试参数\r\n收益率类型:%s, Alpha:%f, Holding:%d, Lmd:%f, Simulation:%d\r\n"),
			strPLType[paramVar.GetPLType()], paramVar.GetAlpha(), paramVar.GetHoldingDays(), paramVar.GetLmd(), paramVar.GetSimulationDays());
		strText += strTemp;
		strTemp.Format(_T("=====================================================\r\n"));
		strText += strTemp;

		long lStart = (long)(::GetTickCount());
		if (this->TestOldFunctions(strTemp, ds, paramVar, iVarCalcMethod))
		{
			long lEnd = (long)(::GetTickCount());
			strText += strTemp;
			strTemp.Format(_T("所花时间:%dms\r\n"), (lEnd -lStart));
			strText += strTemp;

			strTemp.Format(_T("=====================================================\r\n"));
			strText += strTemp;
		}

		lStart = (long)(::GetTickCount());
		if (this->TestNewFunctions(strTemp, ds, paramVar, iVarCalcMethod))
		{
			long lEnd = (long)(::GetTickCount());
			strText += strTemp;

			strTemp.Format(_T("所花时间:%dms\r\n"), (lEnd -lStart));


			strText += strTemp;
			strTemp.Format(_T("=====================================================\r\n"));
			strText += strTemp;
		}

		AfxMessageBox(strText);
		log.WiriteToLogFile(strText);

		return true;
	}
	return false;
}
*/

