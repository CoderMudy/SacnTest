#include "stdafx.h"
#include "VarCalcConfig.h"
#include "AdapterTools/VARTypeChange.h"

CVarCalcConfig::CVarCalcConfig()
{
	m_strConfigPath = _T("C:/Var/param/VarParam.ini");
}
CVarCalcConfig::CVarCalcConfig(CString& strConfigPath)
{
	m_strConfigPath = strConfigPath;
}
CVarCalcConfig::~CVarCalcConfig()
{
}

Tx::VAR::CVarCalcParam CVarCalcConfig::LoadParam()
{
	Tx::VAR::CVarCalcParam param;

	CString strFilePath = this->GetParamFilePath();
	if (strFilePath.IsEmpty())
	{
		return param;
	}

	CVARTypeChange *pType = CVARTypeChange::GetInstance();

	char tempch[256];
	DWORD dw = 0;

	memset(tempch,0,256);
	dw = GetPrivateProfileString(_T("VarParamConfig"), _T("PLType"), _T(""), tempch, 256, strFilePath);
	if (0 == dw)
	{
		return param;
	}
	int iPLType = pType->AM_StrToInt(tempch);

	memset(tempch,0,256);
	dw = GetPrivateProfileString(_T("VarParamConfig"), _T("Alpha"), _T(""), tempch, 256, strFilePath);
	if (0 == dw)
	{
		return param;
	}
	double dAlpha = pType->AM_StrToDouble(tempch);

	memset(tempch,0,256);
	dw = GetPrivateProfileString(_T("VarParamConfig"), _T("HoldingDays"), _T(""), tempch, 256, strFilePath);
	if (0 == dw)
	{
		return param;
	}
	int iHoldingDays = pType->AM_StrToInt(tempch);

	memset(tempch,0,256);
	dw = GetPrivateProfileString(_T("VarParamConfig"), _T("Lmd"), _T(""), tempch, 256, strFilePath);
	if (0 == dw)
	{
		return param;
	}
	double dLmd = pType->AM_StrToDouble(tempch);
	
	memset(tempch,0,256);
	dw = GetPrivateProfileString(_T("VarParamConfig"), _T("SimulationTimes"), _T(""), tempch, 256, strFilePath);
	if (0 == dw)
	{
		return param;
	}
	int iSimulationTimes = pType->AM_StrToInt(tempch);

	memset(tempch,0,256);
	dw = GetPrivateProfileString(_T("VarParamConfig"), _T("SamplesCount"), _T(""), tempch, 256, strFilePath);
	if (0 == dw)
	{
		return param;
	}
	int iSamplesCount = pType->AM_StrToInt(tempch);

	Tx::VAR::CVarCalcParam param1(iPLType, iHoldingDays, dAlpha,  dLmd, iSimulationTimes, iSamplesCount);
	return param1;
}

BOOL CVarCalcConfig::SaveParam(const Tx::VAR::CVarCalcParam& paramVar)
{
	CString strFilePath = this->GetParamFilePath();
	if (strFilePath.IsEmpty())
	{
		return FALSE;
	}
	
	CString strTemp;

	strTemp.Format(_T("%d"), paramVar.GetPLType());
	WritePrivateProfileString(_T("VarParamConfig"), _T("PLType"), strTemp, strFilePath);

	strTemp.Format(_T("%.2f"), paramVar.GetAlpha());
	WritePrivateProfileString(_T("VarParamConfig"), _T("Alpha"), strTemp, strFilePath);

	strTemp.Format(_T("%d"), paramVar.GetHoldingDays());
	WritePrivateProfileString(_T("VarParamConfig"), _T("HoldingDays"), strTemp, strFilePath);

	strTemp.Format(_T("%.4f"), paramVar.GetLmd());
	WritePrivateProfileString(_T("VarParamConfig"), _T("Lmd"), strTemp, strFilePath);

	strTemp.Format(_T("%d"), paramVar.GetSimulationDays());
	WritePrivateProfileString(_T("VarParamConfig"), _T("SimulationTimes"), strTemp, strFilePath);

	strTemp.Format(_T("%d"), paramVar.GetSamplesCount());
	WritePrivateProfileString(_T("VarParamConfig"), _T("SamplesCount"), strTemp, strFilePath);
	
	return TRUE;
}

int CVarCalcConfig::LoadCalcMethod()
{
	int iType = eVarMonteCarloMethod;

	CString strFilePath = this->GetParamFilePath();
	if (strFilePath.IsEmpty())
	{
		return iType;
	}
	CVARTypeChange *pType = CVARTypeChange::GetInstance();

	char tempch[256];
	DWORD dw = 0;

	memset(tempch,0,256);
	dw = GetPrivateProfileString(_T("VarCalcMethod"), _T("VarCalcMethod"), _T(""), tempch, 256, strFilePath);
	if (0 == dw)
	{
		return iType;
	}
	iType = pType->AM_StrToInt(tempch);

	return iType;

}

BOOL CVarCalcConfig::SaveCalcMethod(int iVarCalcMethod)
{
	CString strFilePath = this->GetParamFilePath();
	if (strFilePath.IsEmpty())
	{
		return FALSE;
	}

	if (iVarCalcMethod < 0 || iVarCalcMethod >= eVarCalcMethodCount)
	{
		return FALSE;
	}

	CString strTemp;
	strTemp.Format(_T("%d"), iVarCalcMethod);
	WritePrivateProfileString(_T("VarCalcMethod"), _T("VarCalcMethod"), strTemp, strFilePath);
	return TRUE;
}

BOOL CVarCalcConfig::LoadHoldingBeginDate(bool &bCurDate, int &iBeginDate)
{
	bCurDate = true;
	iBeginDate = -1;

	CString strFilePath = this->GetParamFilePath();
	if (strFilePath.IsEmpty())
	{
		return FALSE;
	}
	CVARTypeChange *pType = CVARTypeChange::GetInstance();

	char tempch[256];
	DWORD dw = 0;

	memset(tempch,0,256);
	dw = GetPrivateProfileString(_T("VarHoldingBeginDate"), _T("DateType"), _T(""), tempch, 256, strFilePath);
	if (0 == dw)
	{
		return FALSE;
	}
	int iCurDate = pType->AM_StrToInt(tempch);
	bCurDate = (iCurDate > 0);

	memset(tempch,0,256);
	dw = GetPrivateProfileString(_T("VarHoldingBeginDate"), _T("BeginDate"), _T(""), tempch, 256, strFilePath);
	if (0 == dw)
	{
		return FALSE;
	}
	iBeginDate = pType->AM_StrToInt(tempch);

	return TRUE;
}

BOOL CVarCalcConfig::SaveHoldingBeginDate(bool bCurDate, int iBeginDate)
{
	CString strFilePath = this->GetParamFilePath();
	if (strFilePath.IsEmpty())
	{
		return FALSE;
	}

	if (!bCurDate && iBeginDate < 19900101)
	{
		return FALSE;
	}

	int iCurDate = 1;
	if (!bCurDate)
	{
		iCurDate = 0;
	}

	CString strTemp;
	strTemp.Format(_T("%d"), iCurDate);
	WritePrivateProfileString(_T("VarHoldingBeginDate"), _T("DateType"), strTemp, strFilePath);

	strTemp.Format(_T("%d"), iBeginDate);
	WritePrivateProfileString(_T("VarHoldingBeginDate"), _T("BeginDate"), strTemp, strFilePath);

	return TRUE;
}

CString CVarCalcConfig::GetParamFilePath()
{
	return m_strConfigPath;
}

