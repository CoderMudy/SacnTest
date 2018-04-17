#ifndef _varcalcservice_h_1987192
#define _varcalcservice_h_1987192

#include "../Business.h"
#include "VarDataProvider.h"

class BUSINESS_EXT CVarCalcService
{
public:
	CVarCalcService();
	CVarCalcService(CString& strConfigPath, long lHodingBeginDate);
	virtual ~CVarCalcService();
public:
	bool Init();
	bool AddPotofolioItem(long lEntityId, double dHodingNumer);
	bool CalcVar(double &dVar, double &dCVar);

protected:
	CString m_strConfigPath;
	CVarDataProvider *m_pVarDataProvider;
	long m_lHodingBeginDate;
};

#endif