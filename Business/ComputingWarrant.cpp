/**********************************************************
  Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
  File name:	ComputingWarrant.cpp
  Author:		王志勇
  Version:		1.0
  Date:			2007-11-13
  Description:	权证测算工具的算法实现

***********************************************************/
#include "stdafx.h"
#include "ComputingWarrant.h"
#include <math.h>


namespace Tx
{
	namespace Business
	{
		ComputingWarrant::ComputingWarrant()
		{
		
		}
		
		ComputingWarrant::~ComputingWarrant()
		{
		
		}
		double	ComputingWarrant::GetStockStd(int nID, int nEndDate, int nCycle)
		{
			double	dStd = 0.0;			//最后返回的标准差
			int		nStartDate = (int(nEndDate/10000)-1)*10000 + nEndDate%10000;	//开始日期,这里需要加闰年的判断
			if ( nStartDate %10000 == 229 )
				nStartDate--;
			//取得此阶段内的行情序列
			std::vector< HisTradeData >		resVec;
			bGetHQByCycle( nID, nStartDate, nEndDate, nCycle, resVec );

			double dAveReturn = 0.0;
			double	dReturn = 0.0;
			double	dTemp = 0.0;
			int nCount = resVec.size();
			//暂时为了统一老系统，修改
			//这里过滤了第一条数据，所以从1开始
			//for ( int i = 1; i < nCount; i++ )
			//{
			//	double dPreClose = resVec[i].Preclose;
			//	double dClose = resVec[i].Close;
			//	if( dPreClose > 0.0 && dClose > 0.0 )
			//	{
			//		dReturn = log (dClose) - log(dPreClose);
			//		dAveReturn += dReturn;
			//		dTemp += dReturn * dReturn;
			//	}
			//}
			//dAveReturn = dAveReturn * dAveReturn / nCount;
			//dStd =  sqrt( dTemp - dAveReturn ) / sqrt( double(nCount) );
			for ( int i = 1; i < nCount; i++ )
			{
				double dPreClose = resVec[i].Preclose;
				double dClose = resVec[i].Close;
				if( dPreClose > 0.0 && dClose > 0.0 )
				{
					dReturn = log (dClose) - log(dPreClose);
					dAveReturn += dReturn;
					dTemp += dReturn * dReturn;
				}
			}
			dAveReturn = dAveReturn * dAveReturn / nCount;
			dStd =  sqrt( dTemp - dAveReturn ) / sqrt( double(nCount-1) );
			//------------

			if ( nCycle == 0 )
				dStd *= sqrt( double(240) );
			if ( nCycle == 1 )
				dStd *= sqrt( double(52) );
			if ( nCycle ==2 )
				dStd *= sqrt( double(12) );
			return dStd;

		}
		
		
		/*
		//获取股票一年内波动率
		double	ComputingWarrant::GetStockStd(int nID, int nEndDate, int nCycle)
		{
			double	dStd = 0.0;			//最后返回的标准差
			int		nStartDate = (int(nEndDate/10000)-1)*10000 + nEndDate%10000;	//开始日期,这里需要加闰年的判断
			std::vector< int >	iDates;
			//step 1:  prepare for calculating
			TxBusiness*	pBusiness = new TxBusiness();
			pBusiness->GetSecurityNow( nID );
			pBusiness->m_pSecurity->LoadHisTrade();
			pBusiness->m_pSecurity->LoadTradeDate();
			bGetTradeDaySequence(	nID,		//交易实体ID
									nEndDate,	//截止日期
									nStartDate,	//开始日期
									nCycle,		//周期
									iDates	
									);
			//step 2:	calculate average return
			double dAveReturn = 0.0;
			double	dReturn = 0.0;
			double	dTemp = 0.0;	
			int		nExStantard = *(iDates.end()-1);
			//for( std::vector< int >::iterator iter = iDates.begin()+1; iter != iDates.end(); ++iter )
			//{
			//	double	dPreClose = pBusiness->m_pSecurity->GetAheadFQ( *(iter-1), nExStantard );
			//	double	dClose = pBusiness->m_pSecurity->GetAheadFQ( *iter, nExStantard );
			//	if( dPreClose > 0.0 && dClose > 0.0 )
			//	{
			//		dReturn = log (dClose) - log(dPreClose);
			//		dAveReturn += dReturn;
			//		dTemp += dReturn * dReturn;
			//	}
			//}
			//int nItemCount = iDates.size()-1;
			//dAveReturn = dAveReturn * dAveReturn / nItemCount;
			//dStd =  sqrt( dTemp - dAveReturn ) / sqrt( double(nItemCount) );
			for( std::vector< int >::iterator iter = iDates.begin()+1; iter != iDates.end(); ++iter )
			{
				//HisTradeData*	pTmp1 = pBusiness->m_pSecurity->GetTradeDataByNatureDate( *(iter-1) );
				//HisTradeData*	pTmp2 = pBusiness->m_pSecurity->GetTradeDataByNatureDate( *iter );
				double	dPreClose = pBusiness->m_pSecurity->GetAheadFQ( *(iter-1), nExStantard );
				double	dClose = pBusiness->m_pSecurity->GetAheadFQ( *iter, nExStantard );
				if( dPreClose == 0.0 )
					dReturn = 0.0;
				else
					dReturn = dClose / dPreClose -1;
				dAveReturn += dReturn;
				dTemp += dReturn * dReturn;
			}
			int nItemCount = iDates.size()-1;
			dAveReturn = dAveReturn * dAveReturn / nItemCount;
			dStd =  sqrt( dTemp - dAveReturn ) / sqrt( double(nItemCount) );
			if ( nCycle == 0 )
				dStd *= sqrt( double(240) );
			if ( nCycle == 1 )
				dStd *= sqrt( double(52) );
			if ( nCycle ==2 )
				dStd *= sqrt( double(12) );
			delete pBusiness;
			return dStd;	
		}
		*/
		/*	
		//获取股票一年内波动率
		double	ComputingWarrant::GetStockStd(int nID, int nEndDate, int nCycle)
		{
			double	dStd = 0.0;			//最后返回的标准差
			int		nStartDate = (int(nEndDate/10000)-1)*10000 + nEndDate%10000;	//开始日期
			TxBusiness*	pBusiness = new TxBusiness();
			pBusiness->GetSecurityNow( nID );
			pBusiness->m_pSecurity->LoadHisTrade();
			pBusiness->m_pSecurity->LoadTradeDate();
			double dAveReturn = 0.0;
			double	dReturn = 0.0;
			double	dTemp = 0.0;	
			int nItemCount = 0;
			int	nIndexEnd = 0;
			int nIndexBegin = 0;
			switch( nCycle )
			{
			case 0:
				// day
				nIndexEnd = pBusiness->m_pSecurity->GetTradeDateIndex( nEndDate );
				nIndexBegin = pBusiness->m_pSecurity->GetTradeDateIndex( nStartDate );
				nItemCount = nIndexEnd - nIndexBegin + 1;
				//step 2:	calculate average return
				//double dAveReturn = 0.0;
				//double	dReturn = 0.0;
				//double	dTemp = 0.0;	 
				for( int i = nIndexBegin;  i<= nIndexEnd; i++ )
				{
					HisTradeData*	temp;
					temp = pBusiness->m_pSecurity->GetTradeDataByIndex( i );
					if( temp->Close == 0.0 || temp->Preclose == 0 )
						dReturn = 0.0;
					else
						dReturn	= temp->Close / temp->Preclose - 1;
					dAveReturn += dReturn ;
					dTemp += dReturn*dReturn ;
				}
				dAveReturn = dAveReturn * dAveReturn / nItemCount;
				dStd =  sqrt( dTemp - dAveReturn ) / sqrt( double(nItemCount) );
				break;
			case 1:
				nIndexEnd = pBusiness->m_pSecurity->GetTradeDateIndex( nEndDate, 0, 2 );
				nIndexBegin = pBusiness->m_pSecurity->GetTradeDateIndex( nStartDate, 0, 2 );
				if ( nIndexBegin == 0 )
					nIndexBegin++;
				nItemCount = nIndexEnd - nIndexBegin +1;
				for( int i = nIndexBegin;  i<= nIndexEnd; i++ )
				{
					HisTradeData*	temp1;
					HisTradeData*	temp2;
					temp1 = pBusiness->m_pSecurity->GetTradeDataByIndex( i-1, 0, 2 );
					temp2 = pBusiness->m_pSecurity->GetTradeDataByIndex( i, 0, 2 );
					if( temp2->Close == 0.0 || temp1->Close == 0 )
						dReturn = 0.0;
					else
						dReturn	= temp2->Close / temp1->Close - 1;
					dAveReturn += dReturn ;
					dTemp += dReturn*dReturn ;
				}
				dAveReturn = dAveReturn * dAveReturn / nItemCount;
				dStd =  sqrt( dTemp - dAveReturn ) / sqrt( double(nItemCount) );
				break;
			case 2:
				nIndexEnd = pBusiness->m_pSecurity->GetTradeDateIndex( nEndDate, 0, 1 );
				nIndexBegin = pBusiness->m_pSecurity->GetTradeDateIndex( nStartDate, 0, 1 );
				if ( nIndexBegin == 0 )
					nIndexBegin++;
				nItemCount = nIndexEnd - nIndexBegin +1;
				for( int i = nIndexBegin;  i<= nIndexEnd; i++ )
				{
					HisTradeData*	temp1;
					HisTradeData*	temp2;
					temp1 = pBusiness->m_pSecurity->GetTradeDataByIndex( i-1, 0, 1 );
					temp2 = pBusiness->m_pSecurity->GetTradeDataByIndex( i, 0, 1 );
					if( temp2->Close == 0.0 || temp1->Close == 0 )
						dReturn = 0.0;
					else
						dReturn	= temp2->Close / temp1->Close - 1;
					dAveReturn += dReturn ;
					dTemp += dReturn*dReturn ;
				}
				dAveReturn = dAveReturn * dAveReturn / nItemCount;
				dStd =  sqrt( dTemp - dAveReturn ) / sqrt( double(nItemCount) );
				break;
			}
			//step 1:  prepare for calculating


			
			delete pBusiness;
			return dStd;	
		}
		*/
		////获取股票一年内波动率
		//double	ComputingWarrant::GetStockStd(int nID, int nEndDate, int nCycle)
		//{
		//	double	dStd = 0.0;			//最后返回的标准差
		//	int		nStartDate = (int(nEndDate/10000)-1)*10000 + nEndDate%10000;	//开始日期
		//	//step 1:  prepare for calculating
		//	TxBusiness*	pBusiness = new TxBusiness();
		//	pBusiness->GetSecurityNow( nID );
		//	pBusiness->m_pSecurity->LoadHisTrade();
		//	pBusiness->m_pSecurity->LoadTradeDate();
		//	int	nIndexEnd = pBusiness->m_pSecurity->GetTradeDateIndex( nEndDate );
		//	int	nIndexBegin = pBusiness->m_pSecurity->GetTradeDateIndex( nStartDate );
		//	int nItemCount = nIndexEnd - nIndexBegin + 1;
		//	//step 2:	calculate average return
		//	double dAveReturn = 0.0;
		//	double	dReturn = 0.0;
		//	double	dTemp = 0.0;	 
		//	for( int i = nIndexBegin;  i<= nIndexEnd; i++ )
		//	{
		//		HisTradeData*	temp;
		//		temp = pBusiness->m_pSecurity->GetTradeDataByIndex( i );
		//		if( temp->Close == 0.0 || temp->Preclose == 0 )
		//			dReturn = 0.0;
		//		else
		//			dReturn	= temp->Close / temp->Preclose - 1;
		//		dAveReturn += dReturn ;
		//		dTemp += dReturn*dReturn ;
		//	}
		//	dStd = sqrt( dAveReturn - dTemp );
		//	if( nCycle == 0 )		//day
		//		//dStd = dStd / sqrt( nItemCount );
		//	if( nCycle == 1 )		//week
		//	{
		//		int temp = pBusiness->m_pSecurity->GetTradeDateIndex( nStartDate, 0, 1 );
		//		temp = pBusiness->m_pSecurity->GetTradeDateIndex( nEndDate, 0, 1 ) - temp ;
		//		//dStd = dStd / sqrt( temp );
		//	}
		//	if( nCycle == 2 )		//month
		//	{
		//		int temp = pBusiness->m_pSecurity->GetTradeDateIndex( nStartDate, 0, 2 );
		//		temp = pBusiness->m_pSecurity->GetTradeDateIndex( nEndDate, 0, 2 ) - temp ;
		//		//dStd = dStd / sqrt( temp );
		//	}
		//	delete pBusiness;
		//	return dStd;	
		//}

		//取得权证的理论价格
		double	ComputingWarrant::GetTheoryPrice(	bool	bFlag,		//认购还是认沽true认购
													int		nStyle,		//权证类型0欧式,1美式,2百慕大
													double	dNoInterest,//无风险利率
													double	dSClose,	//当前标的证券价格
													double	dExPrice,	//行权价格
													double	dExRatio,	//行权比例
													double	dDelta,		//标的股票年股利收益率
													double	dLasted,	//剩余年限
													double	dSigma		//股票连续复利年收益率标准差
													)
		{
			double	dTheory = 0.0;
			int nType = 0;
			if( bFlag)
			{
				if( nStyle == 1 )
					nType = 2;
			}
			else
			{
				if( nStyle == 0 || nStyle == 2 )
					nType = 1;
				if( nStyle == 1 )
					nType = 3;
			}
			CWarrantMath	WarrantMath;
			dTheory = dExRatio * WarrantMath.Calc_Option_Price(  nType,	//使用的期权类型
													  dSClose,	//当前股票价格
													  dExPrice,	//期权执行价格
													  dNoInterest,//无风险利率
													  dDelta,	//标的股票年股利收益率
													  dLasted,	//剩余期限(单位：年)
													  dSigma	//股票连续复利年收益率标准差
													 );
			return dTheory;
		}

		//取得权证的剩余年限
		double	ComputingWarrant::GetLastedPeriod(int nID, int nDate)
		{
			TxWarrant*	pTxWarrant = new TxWarrant();
			double	dLastedPeriod = 0.0;
			pTxWarrant->GetSecurityNow( nID );
			int nEnd;
			pTxWarrant->m_pSecurity->GetIndicatorValueAbsolute( 30301135, nEnd );
			//int nEnd = pTxWarrant->GetPowerEndDate( nID );
			if( nEnd <= nDate )
				return 0.0;
			int nY = 0;
			while( nDate + 10000 <= nEnd)
			{
				nY++;
				nDate += 10000;
			}
			if ( !((nDate/10000%4==0&&nDate/10000%100!=0)||nDate/10000%400==0)&&(nDate)%10000==229 )
				nDate--;
			ZkkLib::DateTime	time1,time2;
			time1.SetDate( nDate/10000, nDate%10000/100, nDate - nDate / 100 *100 );
			time2.SetDate( nEnd/10000, nEnd%10000/100, nEnd - nEnd / 100 *100 );
			ZkkLib::TimeSpan	time;
			time = time2 - time1;
			delete	pTxWarrant;
			int nDays = 365;
			if ((nDate/10000%4==0&&nDate/10000%100!=0)||nDate/10000%400==0)
				nDays = 366;
			return nY+time.GetTotalDays()/nDays;
/*
			ZkkLib::DateTime	time1,time2;
			time1.SetDate( nDate/10000, nDate%10000/100, nDate - nDate / 100 *100 );
			time2.SetDate( nEnd/10000, nEnd%10000/100, nEnd - nEnd / 100 *100 );
			ZkkLib::TimeSpan	time;
			time = time2 - time1;
			delete	pTxWarrant;
			double dDay = time.GetTotalDays();
			return dDay / 365;
*/
		}
		//认购权证溢价率=（行权价+认购权证价格/行权比例-正股价）/正股价 
		//认沽权证溢价率=（正股价+认沽权证价格/行权比例-行权价）/正股价
		//投资者需要变化多少百分比，才可以实现平本
		//取得权证的溢价率
		double	ComputingWarrant::GetPremiumRate(double dWPrice, double dSPrice, double dExPrice, bool bFlag, double	dRatio )
		{
			double dPre= 0.0;
			if( bFlag )//认购
			{
				dPre = (dExPrice + dWPrice / dRatio)/dSPrice - 1; 
			}
			else		//认沽
			{
				dPre = 1- (dExPrice - dWPrice/dRatio )/ dSPrice;
			}

			return dPre;
		}
		
		//取得权证杠杆比率
		double	ComputingWarrant::GetGearRate(	double	dSClose,	//相关资产的收盘价
												double	dWClose,	//权证的收盘价
												double	dExRatio	//行权比例
												)
		{
			double	dGear = 0.0;
			dGear = dSClose/( dWClose / dExRatio );
			return dGear;
		}
		
		//取得权证的跌停价
		double	ComputingWarrant::GetDropLimit(	double	dWPrice,	//权证前一日收盘价
												//double	dSOpen,		//标的证券当日开盘价格
												double	dSPreClose,	//标的证券前一日收盘价
												double	dRatio		//行权比例
												)
		{
			double	dRaiseLimit = 0.0;
			dRaiseLimit = dWPrice - 1.25 * ( 1.1 * dSPreClose - dSPreClose ) * dRatio;
			return  max( 0, dRaiseLimit );

		}
		
		//取得权证的涨停价
		double	ComputingWarrant::GetRaiseLimit(	double	dWPrice,	//权证前一日收盘价
													//double	dSOpen,		//标的证券当日开盘价格
													double	dSPreClose,	//标的证券前一日收盘价
													double	dRatio		//行权比例
													)
		{
			double	dRaiseLimit = 0.0;
			dRaiseLimit = dWPrice + 1.25 * ( 1.1 * dSPreClose - dSPreClose ) * dRatio;
			return  dRaiseLimit;
		}
		
		//取得隐含波动率
		double	ComputingWarrant::GetSigma(	bool	bCall_Put,		//认购认沽	true认购
											double	dStockPrice,//标的证券价格
											double	dExPrice,	//行权价格
											double	dNoRiskInterest,//无风险利率
											double	dDelta,		
											double	dLastedYear,//剩余期限（年）
											double	dMarketValue//期权市场价值
											)
		{
			//-------------取得这些参数---------------------------
			double	dSigma = 0.0;
			////根据权证ID得到看跌看涨
			//bool	bCall_Put = false;	
			////根据ID日期得到股票价格
			//double	dStockPrice = 1;
			////期权执行价格
			//double	dExPrice = 1.0;
			////无风险收益率
			//double	dNoRiskInterest = 0.02;
			////标的股票年股利收益率
			//double	dDelta = 0.0;
			////剩余期限（年）
			//double	dLastedYear = 1.0;
			////期权市场价值
			//double	dMarketValue = 1.0;
			//***************************************************************
			CWarrantMath	WarrantMath;
			WarrantMath.implied_volatility(bCall_Put,dStockPrice,dExPrice,dNoRiskInterest,dDelta,dLastedYear,dMarketValue,&dSigma );
			
			if( dSigma == 0.0 )
				return	-DBL_MAX;
			else
				return dSigma;
		}
		
		//计算综合指标，将结果放到resTable中
		bool	ComputingWarrant::bCalWarrant(Tx::Core::Table &resTable, std::set<int> ID, int nStartDate, int nEndDate, bool bInterest, double dInterest, bool bStd, double dStd, int nCycle )
		{
			//一些预处理
			resTable.Clear();
			resTable.AddCol( Tx::Core::dtype_val_string );		//名称
			resTable.AddCol( Tx::Core::dtype_int4 );			//日期
			resTable.AddCol( Tx::Core::dtype_double );			//理论价格
			resTable.AddCol( Tx::Core::dtype_double );			//实际价格
			resTable.AddCol( Tx::Core::dtype_double );			//剩余期限
			resTable.AddCol( Tx::Core::dtype_double );			//隐含波动率
			resTable.AddCol( Tx::Core::dtype_double );			//波动率
			resTable.AddCol( Tx::Core::dtype_double );			//溢价率
			resTable.AddCol( Tx::Core::dtype_double );			//杠杆比率
			resTable.AddCol( Tx::Core::dtype_double );			//涨停价
			resTable.AddCol( Tx::Core::dtype_double );			//跌停价
			TxBusiness*		pBusiness = new TxBusiness;			//债券
			TxBusiness*		pStock = new TxBusiness;			//标的股票
			
			//这里添加进度条			
			Tx::Core::ProgressWnd prw;
			CString sProgress = _T("权证测算中……");
			UINT progId = prw.AddItem(1,sProgress, 0.0);
			prw.Show( 500 );


			//主要按照样本循环
			for( std::set< int >::iterator iter = ID.begin(); iter != ID.end(); ++iter )
			{
				pBusiness->GetSecurityNow( *iter );
				pBusiness->m_pSecurity->LoadHisTrade();
				pBusiness->m_pSecurity->LoadTradeDate();			
				TxWarrant* pTxWarrant = new TxWarrant();
				int nSecurityId = pTxWarrant->GetObjectSecurityId( *iter );	
				if ( nSecurityId <= 0 )
					continue;
				pStock->GetSecurityNow( nSecurityId );
				pStock->m_pSecurity->LoadHisTrade();
				pStock->m_pSecurity->LoadTradeDate();
				//一些前期处理		
				//取得权证方式
				bool bFlag = pBusiness->m_pSecurity->IsWarrant_Buy();
				//行权比例
				double		dExRatio = pTxWarrant->GetPowerRatio( *iter );
				//取得名称
				CString sName = pBusiness->m_pSecurity->GetName();
				//行权价格
				double		dExPrice = pTxWarrant->GetPowerPrice( *iter );
				//欧式0,美式1,百慕大2
				int			nStyle = 0;
				if( pTxWarrant->IsAmericanStyle( *iter) )
					nStyle = 1;
				if( pTxWarrant->IsBermudaStyle( *iter ) )
					nStyle = 2;
				//这里为一年期定期利率
				double		dNoInterest = 0.018;
				if( bInterest )
					dNoInterest = dInterest/100;

				//按照日期循环
				//如果日期早于最早行情，则取第一条行情日期索引0，这里接口已经支持
				int nIndexBegin = pBusiness->m_pSecurity->GetTradeDateIndex( nStartDate );
				int nIndexEnd = pBusiness->m_pSecurity->GetTradeDateIndex( nEndDate );
				if ( nIndexBegin == -1 && nIndexEnd == -1)
				{
					delete pTxWarrant;
					continue;
				}
				if ( nIndexEnd == -1 )
				{
					nIndexEnd = pBusiness->m_pSecurity->GetTradeDataCount()-1;
					if( pBusiness->m_pSecurity->GetTradeDateByIndex( nIndexEnd ) < nStartDate)
					{
						delete pTxWarrant;
						continue;
					}	
				}
				int nHeadDate = nStartDate;
				/*
				if( nEndDate == CTime::GetCurrentTime().GetYear() * 10000 + CTime::GetCurrentTime().GetMonth() * 100 + CTime::GetCurrentTime().GetDay())
				{
					if( pBusiness->m_pSecurity->GetTradeDateByIndex( nIndexEnd ) < nEndDate && pBusiness->m_pSecurity->IsTodayTradeDate())
					{
						//一些处理
						resTable.AddRow();
						int nCount = resTable.GetRowCount()-1;	

						//这个指标从哪里得到wal?
						double dDelta = 0.0;

						//期权市场价值wal?
						double dMarketValue = 0.0;

						//取得日期,第1列
						int		nDate = nEndDate;

						//股票连续复利年收益率标准差wal?
						double dSigma1 = GetStockStd(	nSecurityId, nDate, nCycle );

						//这里根据日期同时取得标的证券的收盘价格
						double	dStockPrice = pStock->m_pSecurity->GetClosePrice(true);
						resTable.SetCell( 1, nCount, nDate );
						//名称
						resTable.SetCell( 0, nCount, sName );

						//取得权证实际价格
						double		dClose = pBusiness->m_pSecurity->GetClosePrice( true );
						resTable.SetCell( 3, nCount, dClose );
						//取得剩余年限
						double		dPeriod = GetLastedPeriod( *iter, nDate );
						dPeriod *= 1000;
						dPeriod = 0.001 * (int)dPeriod;
						resTable.SetCell( 4, nCount, dPeriod );
						//取得理论价格
						double		dTheory = GetTheoryPrice(	bFlag,		//认购还是认沽true认购
							nStyle,		//权证类型0欧式,1美式,2百慕大
							dNoInterest,//无风险利率
							dStockPrice,//当前标的证券价格
							dExPrice,	//行权价格
							dExRatio,	//行权比例
							dDelta,		//标的股票年股利收益率
							dPeriod,	//剩余年限
							dSigma1		//股票连续复利年收益率标准差
							);
						resTable.SetCell( 2, nCount, dTheory );

						//取得隐含波动率
						TRACE(_T("%d\n"),nDate );
						double		dSigma = GetSigma(	!bFlag,		//认购认沽	true认购
							dStockPrice,//标的证券价格
							dExPrice,	//行权价格
							dNoInterest,//无风险利率
							dDelta,		
							dPeriod,	//剩余期限（年）
							dClose /dExRatio	//期权市场价值
							);
						resTable.SetCell( 5, nCount, dSigma );

						//取得波动率
						double temp;
						if( bStd )
							temp = dStd;
						else
							temp = GetStockStd( nSecurityId, nDate, nCycle);
						resTable.SetCell( 6, nCount, temp );

						//取得溢价率
						double		dPreminum = GetPremiumRate( dClose, dStockPrice, dExPrice, bFlag, dExRatio );
						resTable.SetCell( 7, nCount, dPreminum );

						//取得杠杆比率
						double		dGear = GetGearRate( dStockPrice,	//相关资产的收盘价
							dClose,		//权证的收盘价
							dExRatio		//行权比例
							);
						resTable.SetCell( 8, nCount, dGear );

						//取得涨停价
						//取得权证前一日收盘价
						//double		dWPrice = pBusiness->m_pSecurity->GetTradeDataByIndex( i -1 )->Close;
						double		dWPrice = pBusiness->m_pSecurity->GetTradeDataByNatureDate( nDate )->Preclose;
						//标的证券当日开盘价
						//double		dSOpen = pStock->m_pSecurity->GetTradeDataByIndex( i )->Open;
						//标的证券前一日收盘价格
						//double		dSPreClose = (pStock->m_pSecurity->GetTradeDataByIndex( i-1 ))->Close;	
						double		dSPreClose = (pStock->m_pSecurity->GetTradeDataByNatureDate( nDate ))->Preclose;	
						double		dRaise = GetRaiseLimit(	dWPrice,	//权证前一日收盘价
							//dSOpen,		//标的证券当日开盘价格
							dSPreClose,	//标的证券前一日收盘价
							dExRatio		//行权比例
							);
						resTable.SetCell( 9, nCount, dRaise );

						//取得跌停价
						double		dDrop = GetDropLimit(	dWPrice,	//权证前一日收盘价
							//dSOpen,		//标的证券当日开盘价格
							dSPreClose,	//标的证券前一日收盘价
							dExRatio		//行权比例
							);
						resTable.SetCell( 10, nCount, dDrop );
					}
				}
*/
				

				for( int i = nIndexEnd ; i >= nIndexBegin; i-- )
				{
					//一些处理
					resTable.AddRow();
					int nCount = resTable.GetRowCount()-1;	
				
					//这个指标从哪里得到wal?
					double dDelta = 0.0;
					
					//期权市场价值wal?
					double dMarketValue = 0.0;
				
					//取得日期,第1列
					int		nDate = pBusiness->m_pSecurity->GetTradeDateByIndex( i );

					//这里调整行权价格,根据正股除权
					double	dDivid = 1.0;
					double	dAdjustExPrice = dExPrice;
					double	dAdjustExRatio = dExRatio;
					int nNow = COleDateTime::GetCurrentTime().GetYear()*10000 + COleDateTime::GetCurrentTime().GetMonth()*100 + COleDateTime::GetCurrentTime().GetDay();
					dDivid = pStock->m_pSecurity->GetExdividendScale( nDate,nNow,true );
					if( dDivid > 0.0 )
					{
						dAdjustExPrice = dExPrice / dDivid;
						dAdjustExRatio = dExRatio * dDivid;
					}

					//股票连续复利年收益率标准差wal?
					//取得波动率
					double dSigma1;
					if( bStd )
						dSigma1 = dStd;
					else
						dSigma1 = GetStockStd( nSecurityId, nDate, nCycle);
					resTable.SetCell( 6, nCount, dSigma1 );
					
					//这里根据日期同时取得标的证券的收盘价格
					double	dStockPrice = pStock->m_pSecurity->GetTradeDataByNatureDate( nDate )->Close;
					resTable.SetCell( 1, nCount, nDate );
					//名称
					resTable.SetCell( 0, nCount, sName );

					//取得权证实际价格
					double		dClose = pBusiness->m_pSecurity->GetTradeDataByNatureDate( nDate )->Close;
					resTable.SetCell( 3, nCount, dClose );
					//取得剩余年限
					double		dPeriod = GetLastedPeriod( *iter, nDate );
					dPeriod *= 1000;
					dPeriod = 0.001 * (int)dPeriod;
					resTable.SetCell( 4, nCount, dPeriod );
					//取得理论价格
					double		dTheory = GetTheoryPrice(	bFlag,		//认购还是认沽true认购
															nStyle,		//权证类型0欧式,1美式,2百慕大
															dNoInterest,//无风险利率
															dStockPrice,//当前标的证券价格
															dAdjustExPrice,	//行权价格
															dAdjustExRatio,	//行权比例
															dDelta,		//标的股票年股利收益率
															dPeriod,	//剩余年限
															dSigma1		//股票连续复利年收益率标准差
															);
					resTable.SetCell( 2, nCount, dTheory );
					TRACE(_T("理论价格:%f-无风险利率:%f-证券价格:%f-行权价格:%f-行权比例:%f-剩余年限:%f-sigma%f"),dTheory,dNoInterest,dStockPrice,dAdjustExPrice,dAdjustExRatio,dPeriod,dSigma1);
					//取得隐含波动率
					TRACE(_T("%d\n"),nDate );
					double		dSigma = GetSigma(	!bFlag,		//认购认沽	true认购
													dStockPrice,//标的证券价格
													dAdjustExPrice,	//行权价格
													dNoInterest,//无风险利率
													dDelta,		
													dPeriod,	//剩余期限（年）
													dClose /dAdjustExRatio	//期权市场价值
											);
					resTable.SetCell( 5, nCount, dSigma );

					//取得溢价率
					double		dPreminum = GetPremiumRate( dClose, dStockPrice, dAdjustExPrice, bFlag, dAdjustExRatio );
					resTable.SetCell( 7, nCount, dPreminum );

					//取得杠杆比率
					double		dGear = GetGearRate( dStockPrice,	//相关资产的收盘价
													 dClose,		//权证的收盘价
													 dAdjustExRatio		//行权比例
													);
					resTable.SetCell( 8, nCount, dGear );

					//取得涨停价
					//取得权证前一日收盘价
					//double		dWPrice = pBusiness->m_pSecurity->GetTradeDataByIndex( i -1 )->Close;
					double		dWPrice = pBusiness->m_pSecurity->GetTradeDataByNatureDate( nDate )->Preclose;
					//标的证券当日开盘价
					//double		dSOpen = pStock->m_pSecurity->GetTradeDataByIndex( i )->Open;
					//标的证券前一日收盘价格
					//double		dSPreClose = (pStock->m_pSecurity->GetTradeDataByIndex( i-1 ))->Close;	
					double		dSPreClose = (pStock->m_pSecurity->GetTradeDataByNatureDate( nDate ))->Preclose;	
					double		dRaise = GetRaiseLimit(	dWPrice,	//权证前一日收盘价
														//dSOpen,		//标的证券当日开盘价格
														dSPreClose,	//标的证券前一日收盘价
														dAdjustExRatio		//行权比例
														);
					resTable.SetCell( 9, nCount, dRaise );

					//取得跌停价
					double		dDrop = GetDropLimit(	dWPrice,	//权证前一日收盘价
														//dSOpen,		//标的证券当日开盘价格
														dSPreClose,	//标的证券前一日收盘价
														dAdjustExRatio		//行权比例
														);
					resTable.SetCell( 10, nCount, dDrop );
					prw.SetPercent( progId, double( nIndexEnd - i ) / ( nIndexEnd - nIndexBegin ) );
				}
				

				delete	pTxWarrant;
			}
			prw.SetPercent( progId, 1.0 );
			delete	pBusiness;
			delete	pStock;			
			return true;
		
		}

		bool		ComputingWarrant::bGetTradeDaySequence(	int iSecurityId,	//交易实体ID
															int iEnd,	//截止日期
															int iStart,	//开始日期
															int iCycle,	//周期
															std::vector< int > &iDates		//结果集
															)
		{
			iDates.clear();
			//这里是需要同步的序列，我们取上证指数，暂时我们不考虑同步
			Tx::Business::TxBusiness	shBusiness;
			shBusiness.m_pShIndex->LoadTradeDate();
			shBusiness.m_pShIndex->LoadHisTrade();

			int iIndexBegin = shBusiness.m_pSecurity->GetTradeDateIndex( iStart );
			int iIndexEnd = shBusiness.m_pSecurity->GetTradeDateIndex( iEnd );
			shBusiness.m_pShIndex->GetDate( iIndexBegin,iIndexBegin ,iCycle , iDates, 0 );

			//获取对应交易实体
			//Tx::Business::TxBusiness	Business;
			//Business.GetSecurityNow( iSecurityId );
			//Business.m_pSecurity->LoadHisTrade();
			//Business.m_pSecurity->LoadTradeDate();
			//取得对应交易实体的指定日期对应的自己的索引
			//int iIndexBegin = Business.m_pSecurity->GetTradeDateIndex( iStart );
			//int iIndexEnd = Business.m_pSecurity->GetTradeDateIndex( iEnd );
			//if ( iIndexEnd <= iIndexBegin || iIndexBegin == -1 )
			//	return false;

			//switch( iCycle )
			//{
			//case 0:
			//	//day
			//	//for ( int i = iIndexBegin; i<= iIndexEnd; i++ )
			//	//{
			//	//	int iDate = Business.m_pSecurity->GetTradeDateByIndex( i );
			//	//	iDates.push_back( iDate );
			//	//	
			//	//}
			//	break;
			//case 1:
			//	//week
			//	shBusiness.m_pShIndex->GetDate( , , , iDates, 0 );
			//	//for ( int i = iIndexBegin; i<= iIndexEnd;  )
			//	//{
			//	//	int iDate = Business.m_pSecurity->GetTradeDateByIndex( i );
			//	//	iDates.push_back( iDate );
			//	//	i += 5;
			//	//}
			//	break;
			//case 2:
			//	//month
			//	//for ( int i = iIndexBegin; i<= iIndexEnd;  )
			//	//{
			//	//	int iDate = Business.m_pSecurity->GetTradeDateByIndex( i );
			//	//	iDates.push_back( iDate );
			//	//	i += 30;
			//	//}
			//	break;
			//}
			return true;
		}
		
		bool		ComputingWarrant::bGetHQByCycle(	int nSecurityId,	//交易实体Id
														int nStartDate,		//开始日期
														int nEndDate,		//截止日期
														int	nCycle,			//周期
														std::vector< HisTradeData >	&resVec	//结果集合
														)
		{
			//清空结果集
			resVec.clear();
			TxBusiness	Business;
			Business.GetSecurityNow( nSecurityId );
			Business.m_pSecurity->LoadHisTrade();
			Business.m_pSecurity->LoadTradeDate();

			//-----------取得开始与截止日期在行情中的索引-----------------
			int nHead = Business.m_pSecurity->GetTradeDateIndex( nStartDate );
			if ( nStartDate > Business.m_pSecurity->GetTradeDateByIndex( nHead) )
			{
				nHead--;
			}
			Tx::Data::HisTradeData* pTradeData = Business.m_pSecurity->GetTradeDataByNatureDate( nEndDate,tag_DI_End );
			if( pTradeData != NULL )
				nEndDate = pTradeData->Date;
			else
				nEndDate = Business.m_pSecurity->GetTradeDateLatest();
			int nTail = Business.m_pSecurity->GetTradeDateIndex( nEndDate );
			ASSERT( nTail >= 0 );
			if ( nHead == -1 || (nHead==nTail))
				return false;
			HisTradeData*	pHisTrade;
			HisTradeData	m_HisTrade;
			double			dExFactor = 1.0;
			int				nExStandard = Business.m_pSecurity->GetTradeDateByIndex( nTail );
			//*************************************************************
			//---------------循环取得按照周期的行情结果集----------------
			if ( nCycle == 0 )
			{
				//按日循环
				for ( int i = nHead; i <= nTail; i++)
				{
				
					pHisTrade = Business.m_pSecurity->GetTradeDataByIndex( i );
					if ( pHisTrade != NULL )
					{
						m_HisTrade.Date = pHisTrade->Date;
						dExFactor = Business.m_pSecurity->GetExdividendScale( m_HisTrade.Date, nExStandard, true );
						//m_HisTrade.Amount = pHisTrade->Amount;
						//m_HisTrade.Volume = pHisTrade->Volume;
						m_HisTrade.Close = pHisTrade->Close * (float)dExFactor;
						//m_HisTrade.High = pHisTrade->High * dExFactor;
						//m_HisTrade.Low = pHisTrade->Low * dExFactor;
						m_HisTrade.Open = pHisTrade->Open * (float)dExFactor;
						m_HisTrade.Preclose = pHisTrade->Preclose * (float)dExFactor;
						resVec.push_back( m_HisTrade );
					}
				}
			}
			else
			{
				std::vector<int>	nDates;
				Business.m_pSecurity->GetDate( nStartDate,nEndDate,nCycle,nDates,0 );
				double dPreClose = Business.m_pSecurity->GetPreClosePrice( *nDates.begin());
				for ( std::vector<int>::iterator iter = nDates.begin(); iter != nDates.end(); ++iter )
				{
					pHisTrade = Business.m_pSecurity->GetTradeDataByNatureDate( *iter );
					if ( NULL ==  pHisTrade )
						continue;
					memset( &m_HisTrade,0, sizeof(HisTradeData));
					m_HisTrade.Date = *iter;
					dExFactor = Business.m_pSecurity->GetExdividendScale( m_HisTrade.Date, nExStandard, true );
					m_HisTrade.Preclose = (float)dPreClose;
					double dClose = pHisTrade->Close * (float)dExFactor;
					m_HisTrade.Close = (float)dClose;
					dPreClose = dClose;
					resVec.push_back( m_HisTrade );
				}	
				//这里在非日的情况下与老系统存在差异
				/*
				int	nHd = Business.m_pSecurity->GetTradeDateByIndex( nHead );
				int nTl = GetNextCycleDay( nHd, 1 );
				pHisTrade = Business.m_pSecurity->GetTradeDataByIndex( nHead );
				if ( pHisTrade != NULL )
				{
					m_HisTrade.Amount = pHisTrade->Amount;
					m_HisTrade.Close = pHisTrade->Close;
					m_HisTrade.Date = pHisTrade->Date;
					m_HisTrade.High = pHisTrade->High;
					m_HisTrade.Low = pHisTrade->Low;
					m_HisTrade.Open = pHisTrade->Open;
					m_HisTrade.Preclose = pHisTrade->Preclose;
					m_HisTrade.Volume = pHisTrade->Volume;
					
				}
				for ( int i = nHead; i <= nTail; i++ )
				{
					int nToday = Business.m_pSecurity->GetTradeDateByIndex( i );
					dExFactor = Business.m_pSecurity->GetExdividendScale( nToday, nExStandard, true );	
					pHisTrade = Business.m_pSecurity->GetTradeDataByIndex( i );
					if ( nToday > nTl )
					{
						nHd = nToday;
						nTl = GetNextCycleDay( nHd, 1 );
						if ( m_HisTrade.Date != 0 )					
							resVec.push_back( m_HisTrade );
						else
							continue;
						m_HisTrade.Amount = 0.0;
						m_HisTrade.Date = 0;
						m_HisTrade.Volume = 0;
						if ( pHisTrade != NULL )
						{
							m_HisTrade.Open = pHisTrade->Open * dExFactor;
							// ??????????????????maybe bug?????????????????????
							m_HisTrade.Preclose = pHisTrade->Preclose * dExFactor;
						}
					}
					//更新行情数据
					if ( pHisTrade!=NULL)
					{
						m_HisTrade.Amount += pHisTrade->Amount;
						m_HisTrade.Close = pHisTrade->Close * dExFactor;
						m_HisTrade.Date = pHisTrade->Date;
						m_HisTrade.Volume += pHisTrade->Volume;
						double	dHigh = pHisTrade->High * dExFactor;
						m_HisTrade.High>dHigh?m_HisTrade.High:dHigh;
						double	dLow = pHisTrade->Low * dExFactor;
						m_HisTrade.Low<dLow?m_HisTrade.Low:dLow;	
					}
				}
				*/
			}
			//************************************************************
			return true;
		}
		
		int			ComputingWarrant::GetNextCycleDay(	int nDate,			//开始日期
														int nCycle			//周期	
														)
		{
			int  date;
			//int  month;
			switch( nCycle)
			{	//周线-----------------------------------------------
			case 1: 
				{	
					COleDateTime time;
					time.SetDateTime( nDate/10000, nDate/100%100, nDate%100, 0, 0, 0);
					//time += COleDateTimeSpan( 7 - time.GetDayOfWeek(), 0, 0, 0);
					time += COleDateTimeSpan( 7, 0, 0, 0);
					date = time.GetYear()*10000 + time.GetMonth()*100 + time.GetDay();
				}
				break;
				//月线-----------------------------------------------
			case 2:
				//date = nDate/100*100 + 31;
				{
					COleDateTime time;
					//先找到下下个月
					int nDay = nDate % 100;
					int nYear = nDate / 10000;
					int nMonth = nDate % 10000 / 100 + 2;
					if( nMonth > 12 )
					{
						nYear += 1;
						nMonth-= 12;
					}
					time.SetDateTime( nYear,nMonth,1,0,0,0);
					time -= COleDateTimeSpan( 1,0,0,0 );
					if ( time.GetDay() > nDay )
					{
						date = time.GetYear() * 10000 + time.GetMonth() * 100 + nDay;
					}
					else
					{
						date = time.GetYear() * 10000 + time.GetMonth() * 100 + time.GetDay();
					}
				}
				break;

				//年线-----------------------------------------------
			case 3:
				date = nDate/10000*10000 + 1231;
				break;
			default:
				date = nDate;
				break;
			}
			//------------------------------------------------------------
			return date;
		}
		
	
	}
}