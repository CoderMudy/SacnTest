#pragma once

#include "..\..\..\Core\Driver\Table_Display.h"
#include <vector>
using std::vector;

class AFX_EXT_CLASS CStatDataService
{
public:
	CStatDataService();
	virtual ~CStatDataService();
public:
	enum emUpdateType
	{
		eUPDATE_NULL = 0,
		eUPDATE_PRE_HALF,
		eUPDATE_POST_HALF,
		eUPDATE_ALL,
		eCOUNT,
	};

public:
	virtual BOOL GetShowData(vector<int> vSamples, long lStartDate, long lEndDate, Tx::Core::Table_Display& tabResult);
	BOOL GetData(long lStartDate, long lEndDate, Tx::Core::Table_Display& tabResult);
	void CacheClear();
protected:
	virtual BOOL GetDataFromCache(long lStartDate, long lEndDate, Tx::Core::Table_Display& tabResult);
	virtual BOOL UpdateDataCache(long lStartDate, long lEndDate);
private:
	int GetUpdateType(long lStartDate, long lEndDate);
protected:
	long m_lStartCacheDate;
	long m_lEndCacheDate;
	Tx::Core::Table_Display m_tabCache;
};


