#ifndef _varcalctestservice_h_1987192
#define _varcalctestservice_h_1987192

#include "../../Business.h"
#include "../VarDataProvider.h"

#include "../VarCalc/VarCalcParam.h"
#include "../VarCalc/VarCalcDataSource.h"

class BUSINESS_EXT CVarCalcTestService
{
public:
	CVarCalcTestService();
	CVarCalcTestService(CString& strConfigPath, long lHodingBeginDate);
	virtual ~CVarCalcTestService();
public:
	bool AddPotofolioItem(long lEntityId, double dHodingNumer);
	bool CalcVar(double &dVar, double &dCVar);

	bool TestAllParamCombos(bool withTestData = false);
	bool TestDefaultParam(bool withTestData = false);
protected:
	bool GetParamOneByOne(int iIndex, Tx::VAR::CVarCalcParam &paramVar);
	bool TestNewFunctions(CString& strText, Tx::VAR::CVarDataSource& ds, Tx::VAR::CVarCalcParam &paramVar, int iVarCalcMethod = -1);
	bool TestOldFunctions(CString& strText, Tx::VAR::CVarDataSource& ds, Tx::VAR::CVarCalcParam &paramVar, int iVarCalcMethod = -1);
protected:
	CString m_strConfigPath;
	CVarDataProvider *m_pVarDataProvider;
	long m_lHodingBeginDate;
};

#endif