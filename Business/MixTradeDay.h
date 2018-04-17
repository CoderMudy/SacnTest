#ifndef __MIXTRADEDAY_H__
#define __MIXTRADEDAY_H__

#pragma once

#include "Business.h"
#include <unordered_map>
#include "..\..\Data\SecurityAPI.h"
#include "..\..\Data\SecurityBase.h"

namespace Tx
{
	namespace Business
	{
		class BUSINESS_EXT MixTradeDay
		{
			public:
				MixTradeDay(void);
				~MixTradeDay(void);

			private:
				std::unordered_map<int,int> mIndexToDate;
				std::unordered_map<int,int> mDateToIndex;

			public:
				bool bLoadTradeDate(std::set<int>nSet, bool bFlag = true );
				int GetDateByIndex( int nIndex );
				int GetIndexByDate( int nDate );
				int GetLatestDate();
				int GetTradeDayCount();				
				int SearchNearestIndex(int date);
		};
	}
}

#endif