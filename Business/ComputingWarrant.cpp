/**********************************************************
  Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
  File name:	ComputingWarrant.cpp
  Author:		��־��
  Version:		1.0
  Date:			2007-11-13
  Description:	Ȩ֤���㹤�ߵ��㷨ʵ��

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
			double	dStd = 0.0;			//��󷵻صı�׼��
			int		nStartDate = (int(nEndDate/10000)-1)*10000 + nEndDate%10000;	//��ʼ����,������Ҫ��������ж�
			if ( nStartDate %10000 == 229 )
				nStartDate--;
			//ȡ�ô˽׶��ڵ���������
			std::vector< HisTradeData >		resVec;
			bGetHQByCycle( nID, nStartDate, nEndDate, nCycle, resVec );

			double dAveReturn = 0.0;
			double	dReturn = 0.0;
			double	dTemp = 0.0;
			int nCount = resVec.size();
			//��ʱΪ��ͳһ��ϵͳ���޸�
			//��������˵�һ�����ݣ����Դ�1��ʼ
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
		//��ȡ��Ʊһ���ڲ�����
		double	ComputingWarrant::GetStockStd(int nID, int nEndDate, int nCycle)
		{
			double	dStd = 0.0;			//��󷵻صı�׼��
			int		nStartDate = (int(nEndDate/10000)-1)*10000 + nEndDate%10000;	//��ʼ����,������Ҫ��������ж�
			std::vector< int >	iDates;
			//step 1:  prepare for calculating
			TxBusiness*	pBusiness = new TxBusiness();
			pBusiness->GetSecurityNow( nID );
			pBusiness->m_pSecurity->LoadHisTrade();
			pBusiness->m_pSecurity->LoadTradeDate();
			bGetTradeDaySequence(	nID,		//����ʵ��ID
									nEndDate,	//��ֹ����
									nStartDate,	//��ʼ����
									nCycle,		//����
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
		//��ȡ��Ʊһ���ڲ�����
		double	ComputingWarrant::GetStockStd(int nID, int nEndDate, int nCycle)
		{
			double	dStd = 0.0;			//��󷵻صı�׼��
			int		nStartDate = (int(nEndDate/10000)-1)*10000 + nEndDate%10000;	//��ʼ����
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
		////��ȡ��Ʊһ���ڲ�����
		//double	ComputingWarrant::GetStockStd(int nID, int nEndDate, int nCycle)
		//{
		//	double	dStd = 0.0;			//��󷵻صı�׼��
		//	int		nStartDate = (int(nEndDate/10000)-1)*10000 + nEndDate%10000;	//��ʼ����
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

		//ȡ��Ȩ֤�����ۼ۸�
		double	ComputingWarrant::GetTheoryPrice(	bool	bFlag,		//�Ϲ������Ϲ�true�Ϲ�
													int		nStyle,		//Ȩ֤����0ŷʽ,1��ʽ,2��Ľ��
													double	dNoInterest,//�޷�������
													double	dSClose,	//��ǰ���֤ȯ�۸�
													double	dExPrice,	//��Ȩ�۸�
													double	dExRatio,	//��Ȩ����
													double	dDelta,		//��Ĺ�Ʊ�����������
													double	dLasted,	//ʣ������
													double	dSigma		//��Ʊ���������������ʱ�׼��
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
			dTheory = dExRatio * WarrantMath.Calc_Option_Price(  nType,	//ʹ�õ���Ȩ����
													  dSClose,	//��ǰ��Ʊ�۸�
													  dExPrice,	//��Ȩִ�м۸�
													  dNoInterest,//�޷�������
													  dDelta,	//��Ĺ�Ʊ�����������
													  dLasted,	//ʣ������(��λ����)
													  dSigma	//��Ʊ���������������ʱ�׼��
													 );
			return dTheory;
		}

		//ȡ��Ȩ֤��ʣ������
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
		//�Ϲ�Ȩ֤�����=����Ȩ��+�Ϲ�Ȩ֤�۸�/��Ȩ����-���ɼۣ�/���ɼ� 
		//�Ϲ�Ȩ֤�����=�����ɼ�+�Ϲ�Ȩ֤�۸�/��Ȩ����-��Ȩ�ۣ�/���ɼ�
		//Ͷ������Ҫ�仯���ٰٷֱȣ��ſ���ʵ��ƽ��
		//ȡ��Ȩ֤�������
		double	ComputingWarrant::GetPremiumRate(double dWPrice, double dSPrice, double dExPrice, bool bFlag, double	dRatio )
		{
			double dPre= 0.0;
			if( bFlag )//�Ϲ�
			{
				dPre = (dExPrice + dWPrice / dRatio)/dSPrice - 1; 
			}
			else		//�Ϲ�
			{
				dPre = 1- (dExPrice - dWPrice/dRatio )/ dSPrice;
			}

			return dPre;
		}
		
		//ȡ��Ȩ֤�ܸ˱���
		double	ComputingWarrant::GetGearRate(	double	dSClose,	//����ʲ������̼�
												double	dWClose,	//Ȩ֤�����̼�
												double	dExRatio	//��Ȩ����
												)
		{
			double	dGear = 0.0;
			dGear = dSClose/( dWClose / dExRatio );
			return dGear;
		}
		
		//ȡ��Ȩ֤�ĵ�ͣ��
		double	ComputingWarrant::GetDropLimit(	double	dWPrice,	//Ȩ֤ǰһ�����̼�
												//double	dSOpen,		//���֤ȯ���տ��̼۸�
												double	dSPreClose,	//���֤ȯǰһ�����̼�
												double	dRatio		//��Ȩ����
												)
		{
			double	dRaiseLimit = 0.0;
			dRaiseLimit = dWPrice - 1.25 * ( 1.1 * dSPreClose - dSPreClose ) * dRatio;
			return  max( 0, dRaiseLimit );

		}
		
		//ȡ��Ȩ֤����ͣ��
		double	ComputingWarrant::GetRaiseLimit(	double	dWPrice,	//Ȩ֤ǰһ�����̼�
													//double	dSOpen,		//���֤ȯ���տ��̼۸�
													double	dSPreClose,	//���֤ȯǰһ�����̼�
													double	dRatio		//��Ȩ����
													)
		{
			double	dRaiseLimit = 0.0;
			dRaiseLimit = dWPrice + 1.25 * ( 1.1 * dSPreClose - dSPreClose ) * dRatio;
			return  dRaiseLimit;
		}
		
		//ȡ������������
		double	ComputingWarrant::GetSigma(	bool	bCall_Put,		//�Ϲ��Ϲ�	true�Ϲ�
											double	dStockPrice,//���֤ȯ�۸�
											double	dExPrice,	//��Ȩ�۸�
											double	dNoRiskInterest,//�޷�������
											double	dDelta,		
											double	dLastedYear,//ʣ�����ޣ��꣩
											double	dMarketValue//��Ȩ�г���ֵ
											)
		{
			//-------------ȡ����Щ����---------------------------
			double	dSigma = 0.0;
			////����Ȩ֤ID�õ���������
			//bool	bCall_Put = false;	
			////����ID���ڵõ���Ʊ�۸�
			//double	dStockPrice = 1;
			////��Ȩִ�м۸�
			//double	dExPrice = 1.0;
			////�޷���������
			//double	dNoRiskInterest = 0.02;
			////��Ĺ�Ʊ�����������
			//double	dDelta = 0.0;
			////ʣ�����ޣ��꣩
			//double	dLastedYear = 1.0;
			////��Ȩ�г���ֵ
			//double	dMarketValue = 1.0;
			//***************************************************************
			CWarrantMath	WarrantMath;
			WarrantMath.implied_volatility(bCall_Put,dStockPrice,dExPrice,dNoRiskInterest,dDelta,dLastedYear,dMarketValue,&dSigma );
			
			if( dSigma == 0.0 )
				return	-DBL_MAX;
			else
				return dSigma;
		}
		
		//�����ۺ�ָ�꣬������ŵ�resTable��
		bool	ComputingWarrant::bCalWarrant(Tx::Core::Table &resTable, std::set<int> ID, int nStartDate, int nEndDate, bool bInterest, double dInterest, bool bStd, double dStd, int nCycle )
		{
			//һЩԤ����
			resTable.Clear();
			resTable.AddCol( Tx::Core::dtype_val_string );		//����
			resTable.AddCol( Tx::Core::dtype_int4 );			//����
			resTable.AddCol( Tx::Core::dtype_double );			//���ۼ۸�
			resTable.AddCol( Tx::Core::dtype_double );			//ʵ�ʼ۸�
			resTable.AddCol( Tx::Core::dtype_double );			//ʣ������
			resTable.AddCol( Tx::Core::dtype_double );			//����������
			resTable.AddCol( Tx::Core::dtype_double );			//������
			resTable.AddCol( Tx::Core::dtype_double );			//�����
			resTable.AddCol( Tx::Core::dtype_double );			//�ܸ˱���
			resTable.AddCol( Tx::Core::dtype_double );			//��ͣ��
			resTable.AddCol( Tx::Core::dtype_double );			//��ͣ��
			TxBusiness*		pBusiness = new TxBusiness;			//ծȯ
			TxBusiness*		pStock = new TxBusiness;			//��Ĺ�Ʊ
			
			//������ӽ�����			
			Tx::Core::ProgressWnd prw;
			CString sProgress = _T("Ȩ֤�����С���");
			UINT progId = prw.AddItem(1,sProgress, 0.0);
			prw.Show( 500 );


			//��Ҫ��������ѭ��
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
				//һЩǰ�ڴ���		
				//ȡ��Ȩ֤��ʽ
				bool bFlag = pBusiness->m_pSecurity->IsWarrant_Buy();
				//��Ȩ����
				double		dExRatio = pTxWarrant->GetPowerRatio( *iter );
				//ȡ������
				CString sName = pBusiness->m_pSecurity->GetName();
				//��Ȩ�۸�
				double		dExPrice = pTxWarrant->GetPowerPrice( *iter );
				//ŷʽ0,��ʽ1,��Ľ��2
				int			nStyle = 0;
				if( pTxWarrant->IsAmericanStyle( *iter) )
					nStyle = 1;
				if( pTxWarrant->IsBermudaStyle( *iter ) )
					nStyle = 2;
				//����Ϊһ���ڶ�������
				double		dNoInterest = 0.018;
				if( bInterest )
					dNoInterest = dInterest/100;

				//��������ѭ��
				//������������������飬��ȡ��һ��������������0������ӿ��Ѿ�֧��
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
						//һЩ����
						resTable.AddRow();
						int nCount = resTable.GetRowCount()-1;	

						//���ָ�������õ�wal?
						double dDelta = 0.0;

						//��Ȩ�г���ֵwal?
						double dMarketValue = 0.0;

						//ȡ������,��1��
						int		nDate = nEndDate;

						//��Ʊ���������������ʱ�׼��wal?
						double dSigma1 = GetStockStd(	nSecurityId, nDate, nCycle );

						//�����������ͬʱȡ�ñ��֤ȯ�����̼۸�
						double	dStockPrice = pStock->m_pSecurity->GetClosePrice(true);
						resTable.SetCell( 1, nCount, nDate );
						//����
						resTable.SetCell( 0, nCount, sName );

						//ȡ��Ȩ֤ʵ�ʼ۸�
						double		dClose = pBusiness->m_pSecurity->GetClosePrice( true );
						resTable.SetCell( 3, nCount, dClose );
						//ȡ��ʣ������
						double		dPeriod = GetLastedPeriod( *iter, nDate );
						dPeriod *= 1000;
						dPeriod = 0.001 * (int)dPeriod;
						resTable.SetCell( 4, nCount, dPeriod );
						//ȡ�����ۼ۸�
						double		dTheory = GetTheoryPrice(	bFlag,		//�Ϲ������Ϲ�true�Ϲ�
							nStyle,		//Ȩ֤����0ŷʽ,1��ʽ,2��Ľ��
							dNoInterest,//�޷�������
							dStockPrice,//��ǰ���֤ȯ�۸�
							dExPrice,	//��Ȩ�۸�
							dExRatio,	//��Ȩ����
							dDelta,		//��Ĺ�Ʊ�����������
							dPeriod,	//ʣ������
							dSigma1		//��Ʊ���������������ʱ�׼��
							);
						resTable.SetCell( 2, nCount, dTheory );

						//ȡ������������
						TRACE(_T("%d\n"),nDate );
						double		dSigma = GetSigma(	!bFlag,		//�Ϲ��Ϲ�	true�Ϲ�
							dStockPrice,//���֤ȯ�۸�
							dExPrice,	//��Ȩ�۸�
							dNoInterest,//�޷�������
							dDelta,		
							dPeriod,	//ʣ�����ޣ��꣩
							dClose /dExRatio	//��Ȩ�г���ֵ
							);
						resTable.SetCell( 5, nCount, dSigma );

						//ȡ�ò�����
						double temp;
						if( bStd )
							temp = dStd;
						else
							temp = GetStockStd( nSecurityId, nDate, nCycle);
						resTable.SetCell( 6, nCount, temp );

						//ȡ�������
						double		dPreminum = GetPremiumRate( dClose, dStockPrice, dExPrice, bFlag, dExRatio );
						resTable.SetCell( 7, nCount, dPreminum );

						//ȡ�øܸ˱���
						double		dGear = GetGearRate( dStockPrice,	//����ʲ������̼�
							dClose,		//Ȩ֤�����̼�
							dExRatio		//��Ȩ����
							);
						resTable.SetCell( 8, nCount, dGear );

						//ȡ����ͣ��
						//ȡ��Ȩ֤ǰһ�����̼�
						//double		dWPrice = pBusiness->m_pSecurity->GetTradeDataByIndex( i -1 )->Close;
						double		dWPrice = pBusiness->m_pSecurity->GetTradeDataByNatureDate( nDate )->Preclose;
						//���֤ȯ���տ��̼�
						//double		dSOpen = pStock->m_pSecurity->GetTradeDataByIndex( i )->Open;
						//���֤ȯǰһ�����̼۸�
						//double		dSPreClose = (pStock->m_pSecurity->GetTradeDataByIndex( i-1 ))->Close;	
						double		dSPreClose = (pStock->m_pSecurity->GetTradeDataByNatureDate( nDate ))->Preclose;	
						double		dRaise = GetRaiseLimit(	dWPrice,	//Ȩ֤ǰһ�����̼�
							//dSOpen,		//���֤ȯ���տ��̼۸�
							dSPreClose,	//���֤ȯǰһ�����̼�
							dExRatio		//��Ȩ����
							);
						resTable.SetCell( 9, nCount, dRaise );

						//ȡ�õ�ͣ��
						double		dDrop = GetDropLimit(	dWPrice,	//Ȩ֤ǰһ�����̼�
							//dSOpen,		//���֤ȯ���տ��̼۸�
							dSPreClose,	//���֤ȯǰһ�����̼�
							dExRatio		//��Ȩ����
							);
						resTable.SetCell( 10, nCount, dDrop );
					}
				}
*/
				

				for( int i = nIndexEnd ; i >= nIndexBegin; i-- )
				{
					//һЩ����
					resTable.AddRow();
					int nCount = resTable.GetRowCount()-1;	
				
					//���ָ�������õ�wal?
					double dDelta = 0.0;
					
					//��Ȩ�г���ֵwal?
					double dMarketValue = 0.0;
				
					//ȡ������,��1��
					int		nDate = pBusiness->m_pSecurity->GetTradeDateByIndex( i );

					//���������Ȩ�۸�,�������ɳ�Ȩ
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

					//��Ʊ���������������ʱ�׼��wal?
					//ȡ�ò�����
					double dSigma1;
					if( bStd )
						dSigma1 = dStd;
					else
						dSigma1 = GetStockStd( nSecurityId, nDate, nCycle);
					resTable.SetCell( 6, nCount, dSigma1 );
					
					//�����������ͬʱȡ�ñ��֤ȯ�����̼۸�
					double	dStockPrice = pStock->m_pSecurity->GetTradeDataByNatureDate( nDate )->Close;
					resTable.SetCell( 1, nCount, nDate );
					//����
					resTable.SetCell( 0, nCount, sName );

					//ȡ��Ȩ֤ʵ�ʼ۸�
					double		dClose = pBusiness->m_pSecurity->GetTradeDataByNatureDate( nDate )->Close;
					resTable.SetCell( 3, nCount, dClose );
					//ȡ��ʣ������
					double		dPeriod = GetLastedPeriod( *iter, nDate );
					dPeriod *= 1000;
					dPeriod = 0.001 * (int)dPeriod;
					resTable.SetCell( 4, nCount, dPeriod );
					//ȡ�����ۼ۸�
					double		dTheory = GetTheoryPrice(	bFlag,		//�Ϲ������Ϲ�true�Ϲ�
															nStyle,		//Ȩ֤����0ŷʽ,1��ʽ,2��Ľ��
															dNoInterest,//�޷�������
															dStockPrice,//��ǰ���֤ȯ�۸�
															dAdjustExPrice,	//��Ȩ�۸�
															dAdjustExRatio,	//��Ȩ����
															dDelta,		//��Ĺ�Ʊ�����������
															dPeriod,	//ʣ������
															dSigma1		//��Ʊ���������������ʱ�׼��
															);
					resTable.SetCell( 2, nCount, dTheory );
					TRACE(_T("���ۼ۸�:%f-�޷�������:%f-֤ȯ�۸�:%f-��Ȩ�۸�:%f-��Ȩ����:%f-ʣ������:%f-sigma%f"),dTheory,dNoInterest,dStockPrice,dAdjustExPrice,dAdjustExRatio,dPeriod,dSigma1);
					//ȡ������������
					TRACE(_T("%d\n"),nDate );
					double		dSigma = GetSigma(	!bFlag,		//�Ϲ��Ϲ�	true�Ϲ�
													dStockPrice,//���֤ȯ�۸�
													dAdjustExPrice,	//��Ȩ�۸�
													dNoInterest,//�޷�������
													dDelta,		
													dPeriod,	//ʣ�����ޣ��꣩
													dClose /dAdjustExRatio	//��Ȩ�г���ֵ
											);
					resTable.SetCell( 5, nCount, dSigma );

					//ȡ�������
					double		dPreminum = GetPremiumRate( dClose, dStockPrice, dAdjustExPrice, bFlag, dAdjustExRatio );
					resTable.SetCell( 7, nCount, dPreminum );

					//ȡ�øܸ˱���
					double		dGear = GetGearRate( dStockPrice,	//����ʲ������̼�
													 dClose,		//Ȩ֤�����̼�
													 dAdjustExRatio		//��Ȩ����
													);
					resTable.SetCell( 8, nCount, dGear );

					//ȡ����ͣ��
					//ȡ��Ȩ֤ǰһ�����̼�
					//double		dWPrice = pBusiness->m_pSecurity->GetTradeDataByIndex( i -1 )->Close;
					double		dWPrice = pBusiness->m_pSecurity->GetTradeDataByNatureDate( nDate )->Preclose;
					//���֤ȯ���տ��̼�
					//double		dSOpen = pStock->m_pSecurity->GetTradeDataByIndex( i )->Open;
					//���֤ȯǰһ�����̼۸�
					//double		dSPreClose = (pStock->m_pSecurity->GetTradeDataByIndex( i-1 ))->Close;	
					double		dSPreClose = (pStock->m_pSecurity->GetTradeDataByNatureDate( nDate ))->Preclose;	
					double		dRaise = GetRaiseLimit(	dWPrice,	//Ȩ֤ǰһ�����̼�
														//dSOpen,		//���֤ȯ���տ��̼۸�
														dSPreClose,	//���֤ȯǰһ�����̼�
														dAdjustExRatio		//��Ȩ����
														);
					resTable.SetCell( 9, nCount, dRaise );

					//ȡ�õ�ͣ��
					double		dDrop = GetDropLimit(	dWPrice,	//Ȩ֤ǰһ�����̼�
														//dSOpen,		//���֤ȯ���տ��̼۸�
														dSPreClose,	//���֤ȯǰһ�����̼�
														dAdjustExRatio		//��Ȩ����
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

		bool		ComputingWarrant::bGetTradeDaySequence(	int iSecurityId,	//����ʵ��ID
															int iEnd,	//��ֹ����
															int iStart,	//��ʼ����
															int iCycle,	//����
															std::vector< int > &iDates		//�����
															)
		{
			iDates.clear();
			//��������Ҫͬ�������У�����ȡ��ָ֤������ʱ���ǲ�����ͬ��
			Tx::Business::TxBusiness	shBusiness;
			shBusiness.m_pShIndex->LoadTradeDate();
			shBusiness.m_pShIndex->LoadHisTrade();

			int iIndexBegin = shBusiness.m_pSecurity->GetTradeDateIndex( iStart );
			int iIndexEnd = shBusiness.m_pSecurity->GetTradeDateIndex( iEnd );
			shBusiness.m_pShIndex->GetDate( iIndexBegin,iIndexBegin ,iCycle , iDates, 0 );

			//��ȡ��Ӧ����ʵ��
			//Tx::Business::TxBusiness	Business;
			//Business.GetSecurityNow( iSecurityId );
			//Business.m_pSecurity->LoadHisTrade();
			//Business.m_pSecurity->LoadTradeDate();
			//ȡ�ö�Ӧ����ʵ���ָ�����ڶ�Ӧ���Լ�������
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
		
		bool		ComputingWarrant::bGetHQByCycle(	int nSecurityId,	//����ʵ��Id
														int nStartDate,		//��ʼ����
														int nEndDate,		//��ֹ����
														int	nCycle,			//����
														std::vector< HisTradeData >	&resVec	//�������
														)
		{
			//��ս����
			resVec.clear();
			TxBusiness	Business;
			Business.GetSecurityNow( nSecurityId );
			Business.m_pSecurity->LoadHisTrade();
			Business.m_pSecurity->LoadTradeDate();

			//-----------ȡ�ÿ�ʼ���ֹ�����������е�����-----------------
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
			//---------------ѭ��ȡ�ð������ڵ���������----------------
			if ( nCycle == 0 )
			{
				//����ѭ��
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
				//�����ڷ��յ����������ϵͳ���ڲ���
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
					//������������
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
		
		int			ComputingWarrant::GetNextCycleDay(	int nDate,			//��ʼ����
														int nCycle			//����	
														)
		{
			int  date;
			//int  month;
			switch( nCycle)
			{	//����-----------------------------------------------
			case 1: 
				{	
					COleDateTime time;
					time.SetDateTime( nDate/10000, nDate/100%100, nDate%100, 0, 0, 0);
					//time += COleDateTimeSpan( 7 - time.GetDayOfWeek(), 0, 0, 0);
					time += COleDateTimeSpan( 7, 0, 0, 0);
					date = time.GetYear()*10000 + time.GetMonth()*100 + time.GetDay();
				}
				break;
				//����-----------------------------------------------
			case 2:
				//date = nDate/100*100 + 31;
				{
					COleDateTime time;
					//���ҵ����¸���
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

				//����-----------------------------------------------
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