#include "StdAfx.h"
#include "MixTradeDay.h"

namespace Tx
{
	namespace Business
	{
		MixTradeDay::MixTradeDay(void)
		{
		}

		MixTradeDay::~MixTradeDay(void)
		{
		}

		bool	MixTradeDay::bLoadTradeDate( std::set<int> nSet, bool bFlag )
		{
			std::set<int> nResult;
			std::vector<int> nVec;

			if(bFlag == true)
			{

				for ( std::set< int >::iterator iter = nSet.begin(); iter != nSet.end(); ++iter )
				{
					Tx::Data::Security *p = (Tx::Data::Security*)GetSecurity( *iter );
					p->LoadHisTrade();
					p->LoadTradeDate();
					int nCount = p->GetTradeDataCount();
					for ( int i = 0; i < nCount; i++ )
					{
						int nDate = p->GetTradeDateByIndex( i );
						nResult.insert( nDate );
					}
				}
			}
			else
			{
				;
			}

			for ( std::set< int >::iterator iter = nResult.begin(); iter != nResult.end(); ++iter )
			{
				nVec.push_back( *iter );
			}

			int nCount = nVec.size();
			for ( int i = 0; i < nCount; i++ )
			{
				mIndexToDate[i]=nVec[i];
				mDateToIndex[nVec[i]]=i;
			}
			return true;
		}

		int MixTradeDay::GetDateByIndex(int nIndex)
		{
			auto it = mIndexToDate.find(nIndex);
			return it == mIndexToDate.end() ? -1 : it->second;
		}

		int MixTradeDay::GetIndexByDate(int nDate)
		{
			auto it = mDateToIndex.find(nDate);
			return it == mDateToIndex.end() ? -1 : it->second;
		}

		int MixTradeDay::GetLatestDate()
		{
			return GetDateByIndex( GetTradeDayCount() - 1 );
		}

		int MixTradeDay::GetTradeDayCount()
		{
			int nCount = mDateToIndex.size();
			return nCount;
		}

		//查找最近日期
		int MixTradeDay::SearchNearestIndex(int date)
		{
			int start_index=0;
			int end_index = GetTradeDayCount()-1;
			if(end_index<0)
				return -1;
			if(end_index==0)
				return 0;

			if(date>=GetLatestDate())
				return end_index;

			int index = 0;

			//二分法查找交易日
			while(true)
			{
				int middle_index = (start_index+end_index)/2;
				int middle_date = GetDateByIndex(middle_index);
				if(middle_date<0)
					break;
				if(date<middle_date)
					end_index = middle_index;
				else
					start_index = middle_index;

				if((end_index-start_index)<=1)
					break;
			}

			index = start_index;
			return index;
		}
	}
}