#pragma once

#include "StatDataService.h"

class AFX_EXT_CLASS CBigDealStatService : public CStatDataService
{
public:
	CBigDealStatService();
	CBigDealStatService(int iSecuritiesType, bool bAllSample);
	virtual ~CBigDealStatService();
public:
	enum emSecuritiesSetType
	{
		eSTOCK_AND_BOND = 0,
		eSTOCK,
		eBOND,
		eFUND,
		eSECURITIES_TYPE_COUNT,
	};
	virtual	BOOL GetShowData(vector<int> vSamples, long lStartDate, long lEndDate, Tx::Core::Table_Display& tabResult);

protected:
	virtual BOOL GetDataFromCache(long lStartDate, long lEndDate, Tx::Core::Table_Display& tabResult);
	virtual BOOL UpdateDataCache(long lStartDate, long lEndDate);
private:
	BOOL AppendUpdatedTabToCache(Tx::Core::Table_Display& tabResult);
protected:
	int m_iSecuritiesType;
	bool m_bAllSample;
};


