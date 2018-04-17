#pragma once
#include "VarCalc/VarCalcParam.h"
#include "VarCalc/VarCalcGlobal.h"

#include "../Business.h"


class BUSINESS_EXT CVarCalcConfig
{
public:
	CVarCalcConfig();
	CVarCalcConfig(CString& strConfigPath);
	virtual ~CVarCalcConfig();

	enum emVarCalcMethod
	{
		eVarNormalMethod = 0,
		eVarHisSimuMethod,
		eVarMonteCarloMethod,
		eVarCalcMethodCount,
	};

	Tx::VAR::CVarCalcParam LoadParam();
	BOOL SaveParam(const Tx::VAR::CVarCalcParam& paramVar);

	int LoadCalcMethod();
	BOOL SaveCalcMethod(int iVarCalcMethod);

	BOOL LoadHoldingBeginDate(bool &bCurDate, int &iBeginDate);
	BOOL SaveHoldingBeginDate(bool bCurDate = true, int iBeginDate = -1);
protected:
	CString GetParamFilePath();
protected:
	CString m_strConfigPath;

};
