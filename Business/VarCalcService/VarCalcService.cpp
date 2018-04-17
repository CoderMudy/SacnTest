#include "stdafx.h"
#include "VarCalcService.h"

#include "VarCalcConfig.h"
#include "VarCalc/VarCalcMethod.h"
#include "VarCalc/VarCalcDataSource.h"
#include "VarCalc/VarCalcParam.h"
#include "AdapterTools/VARDateTimeAdapter.h"

CVarCalcService::CVarCalcService()
{
}
CVarCalcService::CVarCalcService(CString& strConfigPath, long lHodingBeginDate)
{
	this->m_strConfigPath = strConfigPath;
	this->m_lHodingBeginDate = lHodingBeginDate;
	m_pVarDataProvider = new CVarDataProvider;
}

CVarCalcService::~CVarCalcService()
{
	if (NULL != m_pVarDataProvider)
	{
		delete m_pVarDataProvider;
	}
}

bool CVarCalcService::Init()
{
	Tx::VAR::CVarCalcParam paramVar;

	CVarCalcConfig configVar(this->m_strConfigPath);
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

	CVARDateTimeAdapter dt(this->m_lHodingBeginDate, 0);
	dt.LastDay();//从开始持仓日向前一日取数据

	this->m_pVarDataProvider->SetParams(dt.GetDate(), paramVar.GetSamplesCount());

	return true;
}
bool CVarCalcService::AddPotofolioItem(long lEntityId, double dHodingNumer)
{
	if (!m_pVarDataProvider->AssertPricesSeries(lEntityId))
	{
		return false;
	}
	return m_pVarDataProvider->AddInvestItem(lEntityId, dHodingNumer);
}

bool CVarCalcService::CalcVar(double &dVar, double &dCVar)
{
	Tx::VAR::CVarCalcParam paramVar;

	CVarCalcConfig configVar(this->m_strConfigPath);
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
