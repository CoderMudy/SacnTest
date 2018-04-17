#include "StdAfx.h"
#include "StatDataService.h"

CStatDataService::CStatDataService()
{
	this->m_lStartCacheDate = this->m_lEndCacheDate = 0;
}

CStatDataService::~CStatDataService()
{
	this->CacheClear();
}
void CStatDataService::CacheClear()
{
	this->m_lStartCacheDate = this->m_lEndCacheDate = 0;
	this->m_tabCache.Clear();
}
BOOL CStatDataService::GetShowData(vector<int> vSamples, long lStartDate, long lEndDate, Tx::Core::Table_Display& tabResult)
{
	return TRUE;
}
BOOL CStatDataService::GetData(long lStartDate, long lEndDate, Tx::Core::Table_Display& tabResult)
{
	if (lStartDate < 19800101 || lEndDate < 19800101)
	{
		return FALSE;
	}
	int iType = this->GetUpdateType(lStartDate, lEndDate);
	if (this->eUPDATE_NULL != iType)
	{
		long lUpdateStartDate = lStartDate;
		long lUpdateEndDate = lEndDate;
		if (eUPDATE_PRE_HALF == iType)
		{
			lUpdateStartDate = lStartDate;
			lUpdateEndDate = this->m_lStartCacheDate;
		}
		else if (this->eUPDATE_POST_HALF == iType)
		{
			lUpdateStartDate = this->m_lEndCacheDate;
			lUpdateEndDate = lEndDate;
		}
		else
		{
			this->m_tabCache.Clear();
		}

		if (!UpdateDataCache(lUpdateStartDate, lUpdateEndDate))
		{
			return FALSE;
		}		
	}

	return GetDataFromCache(lStartDate, lEndDate, tabResult);
}

int CStatDataService::GetUpdateType(long lStartDate, long lEndDate)
{
	if (0 == this->m_lStartCacheDate && 0== this->m_lEndCacheDate)
	{
		return this->eUPDATE_ALL;
	}
	if (lStartDate >= this->m_lStartCacheDate && lEndDate <= this->m_lEndCacheDate)
	{
		return this->eUPDATE_NULL;
	}
	else if (lStartDate < this->m_lStartCacheDate && lEndDate <= this->m_lEndCacheDate)
	{
		return this->eUPDATE_PRE_HALF;
	}
	else if (lStartDate >= this->m_lStartCacheDate && lEndDate > this->m_lEndCacheDate)
	{
		return this->eUPDATE_POST_HALF;
	}
	else
	{
		return this->eUPDATE_ALL;
	}
}

BOOL CStatDataService::UpdateDataCache(long lStartDate, long lEndDate)
{
	if (lStartDate < 19800101 || lEndDate < 19800101)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CStatDataService::GetDataFromCache(long lStartDate, long lEndDate, Tx::Core::Table_Display& tabResult)
{
	tabResult.Clear();
	if (lStartDate < this->m_lStartCacheDate || lEndDate > this->m_lEndCacheDate)
	{
		return FALSE;
	}

	return TRUE;
}