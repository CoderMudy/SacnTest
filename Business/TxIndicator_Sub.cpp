#pragma once
#include "stdafx.h"
#include "TxIndicator.h"
#include <io.h>
#include "..\..\core\driver\ClientFileEngine\public\LogRecorder.h"
#include "../../Core/Driver/ClientFileEngine/base/zip/ZipWrapper.h"

namespace Tx
{
	namespace Business
	{
//------------------------------------------------------------------------
bool	TxIndicator::IsSampleOfIndex( int iSecurity, int iIndex )
{
	Tx::Business::TxBusiness index;
	index.GetSecurityNow( iIndex );
	if ( index.m_pSecurity == NULL )
		return false;
	if ( !index.m_pSecurity->IsIndexHaveSample())
		return false;
	int count= index.m_pSecurity->GetIndexConstituentDataCount();	
	for (int i=0;i<count;i++)											//获取样本到lSid
	{
		IndexConstituentData * isd = index.m_pSecurity->GetIndexConstituentDataByIndex(i);
		if ( isd->iSecurityId == iSecurity )
			return true;
	}
	return false;
}

int	TxIndicator::Get_FinancialDate(int iYear, int iQuarter )
{
	switch( iQuarter )
	{
	case 1:
		return iYear * 10000 + 1;
	case 2:
		return iYear * 10000 + 3;
	case 3:
		return iYear * 10000 + 5;
	case 4:
		return iYear * 10000 + 9;
	default:
		return 0;
	}
}
int	TxIndicator::Get_PreFinancailDate(int iYear, int iQuarter )
{
	switch( iQuarter )
	{
	case 1:
		return --iYear * 10000 + 9;
	case 2:
		return iYear * 10000 + 3;
	case 3:
		return iYear * 10000 + 5;
	case 4:
		return iYear * 10000 + 9;
	default:
		return 0;
	}
}
/*
PE = Price / EPS = Price * Share / 最新归属母公司净利润

*/
double	TxIndicator::Get_PE_Basic( int iSecurity, int iDate, bool bSecurity )
{
	// 1 检查数据有效性
	double dRet = Con_doubleInvalid;
	if ( iDate > COleDateTime::GetCurrentTime().GetYear()*10000 + COleDateTime::GetCurrentTime().GetMonth()*100 + COleDateTime::GetCurrentTime().GetDay())
		return dRet;
	int iYear = iDate / 10000;
	int iQuarter = 0;
	double dFactor = 1;
	if ( iDate % 10000 < 331 )
	{
		iYear--;
		iQuarter = 9;
	}	
	else
		if ( iDate % 10000 < 630 )
		{
			iQuarter = 1;
			dFactor = 4;
		}
		else
			if ( iDate % 10000 < 930 )
			{
				iQuarter = 3;
				dFactor = 2;
			}
			else
				if ( iDate %10000 < 1231)
				{
					iQuarter = 5;
					dFactor = double(4)/3;
				}
				else
				{
					iQuarter = 9;
				}
	Tx::Business::TxBusiness	business;
	business.GetSecurityNow( iSecurity );
	if ( business.m_pSecurity == NULL || !business.m_pSecurity->IsStock())
		return dRet;
	Tx::Data::Income*	pIncome = NULL;
	TxShareData*	pShare = business.m_pSecurity->GetTxShareDataByDate( iDate );
	if ( pShare == NULL )
		return dRet;
	double	dEaring = 0.0;
	double	dPrice = business.m_pSecurity->GetClosePrice( iDate, true);
	if ( dPrice == Con_floatInvalid || dPrice < 0.0 )
		return dRet;
	double	dShare = pShare->TotalShare; 
	
	if ( bSecurity )//按照交易实体走
	{	//先算利润
		int iIndex = iYear * 100000 + iQuarter * 10+3;
		business.m_pSecurity->GetDataByObj( Tx::Data::dt_Income,iIndex,&pIncome,false);
		if ( pIncome == NULL )
		{
			switch( iQuarter )
			{
			case 1:
				iQuarter = 9;
				dFactor = 1;
				iYear--;
				break;
			case 3:
				iQuarter = 1;
				dFactor = 4;
				break;
			case 5:
				iQuarter = 3;
				dFactor =2; 
				break;
			case 9:
				iQuarter = 5;
				dFactor = double(4)/3;
				break;
			}
			iIndex = iYear * 100000 + iQuarter*10 + 3;
			business.m_pSecurity->GetDataByObj( Tx::Data::dt_Income,iIndex,&pIncome,false);
			if ( pIncome == NULL )
				return dRet;
		}
		dEaring = dFactor * pIncome->dIncome[18];
		return dPrice * dShare / dEaring;
	}
	else			//按照财年财季走
	{
		int iIndex = iYear * 10000 + iQuarter;
		bool bLoaded = false;
		if(m_pIncomeDataFile!=NULL)
			bLoaded = m_pIncomeDataFile->Load(iIndex,30071,true);
		if ( !bLoaded )
		{
			switch( iQuarter )
			{
			case 1:
				iQuarter = 9;
				dFactor = 1;
				iYear--;
				break;
			case 3:
				iQuarter = 1;
				dFactor = 4;
				break;
			case 5:
				iQuarter = 3;
				dFactor = 2;
					break;
			case 9:
				iQuarter = 5;
				dFactor = double(4)/3;
				break;
			}
			iIndex = iYear * 10000 + iQuarter;
			bLoaded = m_pIncomeDataFile->Load(iIndex,30071,true);
			if ( bLoaded )
				pIncome = m_pIncomeDataFile->GetDataByObj( business.m_pSecurity->GetInstitutionId()*10 + 3,false );
		}
		else
		{
			pIncome = m_pIncomeDataFile->GetDataByObj( business.m_pSecurity->GetInstitutionId()*10 + 3,false );
			if ( pIncome == NULL )
			{
				switch( iQuarter )
				{
				case 1:
					iQuarter = 9;
					dFactor = 1;
					iYear--;
					break;
				case 3:
					iQuarter = 1;
					dFactor = 4;
					break;
				case 5:
					iQuarter = 3;
					dFactor = 2;
					break;
				case 9:
					iQuarter = 5;
					dFactor = double(4)/3;
					break;
				}
				iIndex = iYear * 10000 + iQuarter;
				bLoaded = m_pIncomeDataFile->Load(iIndex,30071,true);
				if ( bLoaded )
					pIncome = m_pIncomeDataFile->GetDataByObj( business.m_pSecurity->GetInstitutionId()*10 + 3,false );
			}
		}
		pIncome = m_pIncomeDataFile->GetDataByObj( business.m_pSecurity->GetInstitutionId()*10 + 3,false );
		if ( pIncome == NULL )
			return dRet;
		dEaring = dFactor * pIncome->dIncome[18];
		return dPrice * dShare / dEaring;
	}
}
/*
1月至3月份的日期 取利润是前一年的年报的利润
4月至6月的日期，取利润是 拿该年一季报简单模拟该年的利润：（该年一季报*4）
7月至9月的日期，取利润是 拿该年中报简单模拟该年的利润（该年中报*2）
10月至12月的日期，取利润是 拿该年三季报简单模拟该年的利润（该年三季报*4/3）
*/
double	TxIndicator::Get_Profit_PE_Basic( Tx::Business::TxBusiness *pBusiness, int& iDate, bool bSecurity)
{
	// 1 检查数据有效性
	double dRet = Con_doubleInvalid;
	if(pBusiness == NULL )
		return dRet;
	CTime tm = CTime::GetCurrentTime();
	int m_iCurDate = tm.GetYear()* 10000 + tm.GetMonth()*100 + tm.GetDay();
	if(iDate > m_iCurDate || iDate < 19600101)
		return dRet;
	int m_iYear = iDate /10000;
	int m_iMonth = iDate%10000/100;
	if(m_iMonth < 1 || m_iMonth > 12)
		return dRet;
	int iQuarter = 0;


	if ( pBusiness->m_pSecurity == NULL || !pBusiness->m_pSecurity->IsStock())
		return dRet;
	//Tx::Data::Income*	pIncome = NULL;
	//Tx::Data::Income*	pIncome1 = NULL;
	//Tx::Data::Income*	pIncome2 = NULL;
	double	dEaring = 0.0;

	if(m_iMonth < 4 && m_iMonth >0)//1月至3月份
	{
		//取利润是前一年的年报的利润
		iQuarter = 9;			
		dEaring = Get_Profit_From_Date(pBusiness,m_iYear-1,iQuarter,bSecurity);
	}
	else if(m_iMonth < 7 && m_iMonth >3)//4月至6月的日期
	{
		//拿该年一季报简单模拟该年的利润：（该年一季报*4）
		iQuarter = 1;		
		dEaring = Get_Profit_From_Date(pBusiness,m_iYear,iQuarter,bSecurity);
		if(dEaring != Con_doubleInvalid)
			return dEaring*4;
		//若为空，取去年年报		
		iQuarter = 9;			
		dEaring = Get_Profit_From_Date(pBusiness,m_iYear-1,iQuarter,bSecurity);		
	}
	else if(m_iMonth < 10 && m_iMonth >6)//7月至9月的日期
	{
		//拿该年中报简单模拟该年的利润（该年中报*2）
		iQuarter = 3;		
		dEaring = Get_Profit_From_Date(pBusiness,m_iYear,iQuarter,bSecurity);
		if(dEaring != Con_doubleInvalid)
			return dEaring*2;
		//若为空，取去年年报		
		iQuarter = 9;			
		dEaring = Get_Profit_From_Date(pBusiness,m_iYear-1,iQuarter,bSecurity);		
	}
	else//10月至12月的日期
	{
		//拿该年三季报简单模拟该年的利润（该年三季报*4/3）
		iQuarter = 5;
		dEaring = Get_Profit_From_Date(pBusiness,m_iYear,iQuarter,bSecurity);
		if(dEaring != Con_doubleInvalid)
			return dEaring*4/3;
		//若为空，取中报		
		iQuarter = 3;
		dEaring = Get_Profit_From_Date(pBusiness,m_iYear,iQuarter,bSecurity);
		if(dEaring != Con_doubleInvalid)
			return dEaring*2;;
		
		//若为再空，取去年年报		
		iQuarter = 9;			
		dEaring = Get_Profit_From_Date(pBusiness,m_iYear-1,iQuarter,bSecurity);		
	}	
	return dEaring;
}
double	TxIndicator::Get_PE_Basic_NEW( int iSecurity, int iDate, bool bSecurity /*= true*/ )
{
	// 1 检查数据有效性
	double dRet = Con_doubleInvalid;
	CTime tm = CTime::GetCurrentTime();
	int m_iCurDate = tm.GetYear()* 10000 + tm.GetMonth()*100 + tm.GetDay();
	if(iDate > m_iCurDate || iDate < 19600101)
		return dRet;
	int m_iYear = iDate /10000;
	int m_iMonth = iDate%10000/100;
	if(m_iMonth < 1 || m_iMonth > 12)
		return dRet;
	int iQuarter = 0;

	Tx::Business::TxBusiness	business;
	business.GetSecurityNow( iSecurity );
	if ( business.m_pSecurity == NULL || !business.m_pSecurity->IsStock())
		return dRet;
	TxShareData*	pShare = business.m_pSecurity->GetTxShareDataByDate( iDate );
	if ( pShare == NULL )
		return dRet;
	double	dEaring = Con_doubleInvalid;
	double	dPrice = business.m_pSecurity->GetClosePrice( iDate, true );
	if ( dPrice == Con_floatInvalid || dPrice <- 0.0 )
		return dRet;
	double	dShare = pShare->TotalShare; 
	dEaring = Get_Profit_PE_Basic(&business,iDate,bSecurity);
	if((dEaring < 0.001 && dEaring > -0.001)||dEaring == Con_doubleInvalid)
		return dRet;
	dRet = dPrice * dShare / dEaring;
	return dRet;
}
//静态PE    2012-3-19
double  TxIndicator::Get_PE_Static(int iSecurity, int iDate, bool bSecurity /* = true  */)
{
	// 1 检查数据是否有效
	double dRet = Con_doubleInvalid;
	CTime tm = CTime::GetCurrentTime();
	int m_iCurDate = tm.GetYear()*10000 + tm.GetMonth()*100 + tm.GetDay();
	if (iDate > m_iCurDate || iDate < 19600101)
         return dRet;
	int m_iYear = iDate / 10000;
	int m_iMonth = iDate%10000/100;
	if (m_iMonth < 1 || m_iMonth >12)
	     return dRet;
	Tx::Business::TxBusiness    business;
	business.GetSecurityNow(iSecurity);
    if (business.m_pSecurity == NULL || !business.m_pSecurity->IsStock())
		return dRet;
	TxShareData*    pShare = business.m_pSecurity->GetTxShareDataByDate(iDate);
	if (pShare == NULL)
	    return dRet;
	double dEaring = Con_doubleInvalid;
	double dPrice = business.m_pSecurity->GetClosePrice( iDate,true );
	if (dPrice == Con_doubleInvalid || dPrice < -0.0)
	    return dRet;
	double dShare = pShare->TotalShare;
	int iQuarter = 9;  //取年报
	dEaring = Get_Profit_From_Date(&business,m_iYear,iQuarter,bSecurity);
	if (dEaring == Con_doubleInvalid)
	   dEaring = Get_Profit_From_Date(&business,m_iYear-1,iQuarter,bSecurity);
	if ((dEaring < 0.001 && dEaring > -0.001) || dEaring == Con_doubleInvalid)
	    return dRet;
	dRet = dPrice * dShare / dEaring;
	return dRet;
}
double	TxIndicator::Get_Profit_PE_Lmt( Tx::Business::TxBusiness *pBusiness, int& iDate, bool bSecurity)
{
	// 1 检查数据有效性
	double dRet = Con_doubleInvalid;
	if(pBusiness == NULL )
		return dRet;
	CTime tm = CTime::GetCurrentTime();
	int m_iCurDate = tm.GetYear()* 10000 + tm.GetMonth()*100 + tm.GetDay();
	if(iDate > m_iCurDate || iDate < 19600101)
		return dRet;
	int m_iYear = iDate /10000;
	int m_iMonth = iDate%10000/100;
	if(m_iMonth < 1 || m_iMonth > 12)
		return dRet;
	int iQuarter = 0;


	if ( pBusiness->m_pSecurity == NULL || !pBusiness->m_pSecurity->IsStock())
		return dRet;
	//Tx::Data::Income*	pIncome = NULL;
	//Tx::Data::Income*	pIncome1 = NULL;
	//Tx::Data::Income*	pIncome2 = NULL;
	double	dEaring = 0.0;

	if(m_iMonth < 4 && m_iMonth >0)//1月至3月份
	{
		//取利润是前一年的年报的利润
		iQuarter = 9;			
		dEaring = Get_Profit_From_Date(pBusiness,m_iYear-1,iQuarter,bSecurity);
	}
	else if(m_iMonth < 7 && m_iMonth >3)//4月至6月的日期
	{
		//该年一季报/前一年一季报*前一年年报
		iQuarter = 9;
		dEaring = Get_Profit_From_Date(pBusiness,m_iYear-1,iQuarter,bSecurity);

		iQuarter = 1;
		double dCur = Con_doubleInvalid;//该年一季报
		double dLast = Con_doubleInvalid;//前一年一季报
		dCur = Get_Profit_From_Date(pBusiness,m_iYear,iQuarter,bSecurity);
		dLast = Get_Profit_From_Date(pBusiness,m_iYear-1,iQuarter,bSecurity);
		if(dCur == Con_doubleInvalid || dLast == Con_doubleInvalid)
			return dEaring;			
		dEaring = dCur-dLast+dEaring;
	}
	else if(m_iMonth < 10 && m_iMonth >6)//7月至9月的日期
	{
		//（该年中报/前一年中报*前一年年报）
		iQuarter = 9;
		dEaring = Get_Profit_From_Date(pBusiness,m_iYear-1,iQuarter,bSecurity);

		iQuarter = 3;
		double dCur = Con_doubleInvalid;//该年中报
		double dLast = Con_doubleInvalid;//前一年中报
		dCur = Get_Profit_From_Date(pBusiness,m_iYear,iQuarter,bSecurity);
		dLast = Get_Profit_From_Date(pBusiness,m_iYear-1,iQuarter,bSecurity);
		if(dCur == Con_doubleInvalid || dLast == Con_doubleInvalid)
			return dEaring;		
		dEaring = dCur-dLast+dEaring;
	}
	else//10月至12月的日期
	{
		//该年三季报/前一年三季报*前一年年报
		iQuarter = 9;
		dEaring = Get_Profit_From_Date(pBusiness,m_iYear-1,iQuarter,bSecurity);

		iQuarter = 5;
		double dCur = Con_doubleInvalid;//该年三季报
		double dLast = Con_doubleInvalid;//前一年三季报
		dCur = Get_Profit_From_Date(pBusiness,m_iYear,iQuarter,bSecurity);
		dLast = Get_Profit_From_Date(pBusiness,m_iYear-1,iQuarter,bSecurity);
		//若为空，取中报
		if(dCur == Con_doubleInvalid || dLast == Con_doubleInvalid)
		{
			iQuarter = 3;
			dCur = Get_Profit_From_Date(pBusiness,m_iYear,iQuarter,bSecurity);
			dLast = Get_Profit_From_Date(pBusiness,m_iYear-1,iQuarter,bSecurity);
		}
		//若为再空，取去年年报
		if(dCur == Con_doubleInvalid || dLast == Con_doubleInvalid)
			return dEaring;
		dEaring = dCur-dLast+dEaring;
	}	
	return dEaring;
}
double	TxIndicator::Get_PE_Ltm_NEW( int iSecurity, int iDate, bool bSecurity /*= true*/ )
{
	// 1 检查数据有效性
	double dRet = Con_doubleInvalid;
	CTime tm = CTime::GetCurrentTime();
	int m_iCurDate = tm.GetYear()* 10000 + tm.GetMonth()*100 + tm.GetDay();
	if(iDate > m_iCurDate || iDate < 19600101)
		return dRet;
	int m_iYear = iDate /10000;
	int m_iMonth = iDate%10000/100;
	if(m_iMonth < 1 || m_iMonth > 12)
		return dRet;
	int iQuarter = 0;

	Tx::Business::TxBusiness	business;
	business.GetSecurityNow( iSecurity );
	if ( business.m_pSecurity == NULL || !business.m_pSecurity->IsStock())
		return dRet;
	TxShareData*	pShare = business.m_pSecurity->GetTxShareDataByDate( iDate );
	if ( pShare == NULL )
		return dRet;
	double	dEaring = Con_doubleInvalid;
	double	dPrice = business.m_pSecurity->GetClosePrice( iDate, true );
	if ( dPrice == Con_floatInvalid || dPrice <- 0.0 )
		return dRet;
	double	dShare = pShare->TotalShare; 
	dEaring = Get_Profit_PE_Lmt(&business,iDate,bSecurity);
	if((dEaring < 0.001 && dEaring > -0.001)||dEaring == Con_doubleInvalid)
		return dRet;
	dRet = dPrice * dShare / dEaring;
	return dRet;
}
double	TxIndicator::Get_PE_Ltm( int iSecurity, int iDate,  bool bSecurity )//gundong 
{
	// 1 检查数据有效性
	double dRet = Con_doubleInvalid;
	if ( iDate > COleDateTime::GetCurrentTime().GetYear()*10000 + COleDateTime::GetCurrentTime().GetMonth()*100 + COleDateTime::GetCurrentTime().GetDay())
		return dRet;
	int iYear = iDate / 10000;
	int iQuarter = 0;
	double dFactor = 1;
	if ( iDate % 10000 < 331 )
	{
		iYear--;
		iQuarter = 9;
	}	
	else
		if ( iDate % 10000 < 630 )
		{
			iQuarter = 1;
			dFactor = 4;
		}
		else
			if ( iDate % 10000 < 930 )
			{
				iQuarter = 3;
				dFactor = 2;
			}
			else
				if ( iDate %10000 < 1231)
				{
					iQuarter = 5;
					dFactor = double(4)/3;
				}
				else
				{
					iQuarter = 9;
				}
	Tx::Business::TxBusiness	business;
	business.GetSecurityNow( iSecurity );
	if ( business.m_pSecurity == NULL || !business.m_pSecurity->IsStock())
		return dRet;
	Tx::Data::Income*	pIncome = NULL;
	Tx::Data::Income*	pIncome1 = NULL;
	Tx::Data::Income*	pIncome2 = NULL;
	TxShareData*	pShare = business.m_pSecurity->GetTxShareDataByDate( iDate );
	if ( pShare == NULL )
		return dRet;
	double	dEaring = 0.0;
	double	dPrice = business.m_pSecurity->GetClosePrice( iDate, true );
	if ( dPrice == Con_floatInvalid || dPrice <= 0.0 )
		return dRet;
	double	dShare = pShare->TotalShare; 

	if ( bSecurity )//按照交易实体走
	{	//先算利润
		int iIndex = iYear * 100000 + iQuarter * 10 + 3;
		business.m_pSecurity->GetDataByObj( Tx::Data::dt_Income,iIndex,&pIncome,false);
		if ( pIncome == NULL )
		{
			switch( iQuarter )
			{
			case 1:
				iQuarter = 9;
				dFactor = 1;
				iYear--;
				break;
			case 3:
				iQuarter = 1;
				dFactor = 4;
				break;
			case 5:
				iQuarter = 3;
				dFactor =2; 
				break;
			case 9:
				iQuarter = 5;
				dFactor = double(4)/3;
				break;
			}
			iIndex = iYear * 100000 + iQuarter*10 + 3;
			business.m_pSecurity->GetDataByObj( Tx::Data::dt_Income,iIndex,&pIncome,false);
			if ( pIncome == NULL )
				return dRet;
		}
		dEaring = pIncome->dIncome[18];
		switch(iQuarter)
		{
		case 1:
			{
				int iIndex1 = --iYear * 100000 + 90 + 3;
				int iIndex2 = iYear * 100000 + 10 + 3;
				business.m_pSecurity->GetDataByObj( Tx::Data::dt_Income,iIndex1,&pIncome1,false);
				business.m_pSecurity->GetDataByObj( Tx::Data::dt_Income,iIndex2,&pIncome2,false);
				if ( pIncome2 == NULL || pIncome1 == NULL )
					return dRet;
				dEaring += pIncome1->dIncome[18]-pIncome2->dIncome[18];
			}
			break;
		case 3:
			{
				int iIndex1 = --iYear * 100000 + 90 + 3;
				int iIndex2 = iYear * 100000 + 30 + 3;
				business.m_pSecurity->GetDataByObj( Tx::Data::dt_Income,iIndex1,&pIncome1,false);
				business.m_pSecurity->GetDataByObj( Tx::Data::dt_Income,iIndex2,&pIncome2,false);
				if ( pIncome2 == NULL || pIncome1 == NULL )
					return dRet;
				dEaring += pIncome1->dIncome[18]-pIncome2->dIncome[18];
			}
			break;
		case 5:
			{
				int iIndex1 = --iYear * 100000 + 90 + 3;
				int iIndex2 = iYear * 100000 + 50 + 3;
				business.m_pSecurity->GetDataByObj( Tx::Data::dt_Income,iIndex1,&pIncome1,false);
				business.m_pSecurity->GetDataByObj( Tx::Data::dt_Income,iIndex2,&pIncome2,false);
				if ( pIncome2 == NULL || pIncome1 == NULL )
					return dRet;
				dEaring += pIncome1->dIncome[18]-pIncome2->dIncome[18];
			}
			break;
		case 9:
			//{
			//	int iIndex1 = --iYear * 100000 + 90 + 3;
			//	int iIndex2 = iYear * 100000 + 10 + 3;
			//	business.m_pSecurity->GetDataByObj( Tx::Data::dt_Income,iIndex1,&pIncome1,false);
			//	business.m_pSecurity->GetDataByObj( Tx::Data::dt_Income,iIndex2,&pIncome2,false);
			//	if ( pIncome2 == NULL || pIncome1 == NULL )
			//		return dRet;
			//	dEaring += pIncome2->dIncome[18]-pIncome1->dIncome[18];
			//}
			break;
		}
		return dPrice * dShare / dEaring;
	}
	else			//按照财年财季走
	{
		int iIndex = iYear * 10000 + iQuarter;
		bool bLoaded = false;
		if(m_pIncomeDataFile!=NULL)
			bLoaded = m_pIncomeDataFile->Load(iIndex,30071,true);
		if ( !bLoaded)
		{
			switch( iQuarter )
			{
			case 1:
				iQuarter = 9;
				dFactor = 1;
				iYear--;
				break;
			case 3:
				iQuarter = 1;
				dFactor = 4;
				break;
			case 5:
				iQuarter = 3;
				dFactor = 2;
				break;
			case 9:
				iQuarter = 5;
				dFactor = double(4)/3;
				break;
			}
			iIndex = iYear * 10000 + iQuarter;
			bLoaded = m_pIncomeDataFile->Load(iIndex,30071,true);
			if ( bLoaded )
				pIncome = m_pIncomeDataFile->GetDataByObj( business.m_pSecurity->GetInstitutionId()*10 + 3,false );
		}
		else
		{
			pIncome = m_pIncomeDataFile->GetDataByObj( business.m_pSecurity->GetInstitutionId()*10 + 3,false );
			if ( pIncome == NULL )
			{
				switch( iQuarter )
				{
				case 1:
					iQuarter = 9;
					dFactor = 1;
					iYear--;
					break;
				case 3:
					iQuarter = 1;
					dFactor = 4;
					break;
				case 5:
					iQuarter = 3;
					dFactor = 2;
					break;
				case 9:
					iQuarter = 5;
					dFactor = double(4)/3;
					break;
				}
				iIndex = iYear * 10000 + iQuarter;
				bLoaded = m_pIncomeDataFile->Load(iIndex,30071,true);
				if ( bLoaded )
					pIncome = m_pIncomeDataFile->GetDataByObj( business.m_pSecurity->GetInstitutionId()*10 + 3,false );
			}
		}
		if ( pIncome == NULL)
			return dRet;
		dEaring = pIncome->dIncome[18];
		switch( iQuarter )
		{
		case 1:
			{
				int iIndex1 = --iYear * 10000 + 9;
				int iIndex2 = iYear * 10000 + 1;
				double dTmp1 = 0.0;
				double dTmp2 = 0.0;
				if( !m_pIncomeDataFile->Load(iIndex1,30071,true))
					return dRet;
				pIncome1 = m_pIncomeDataFile->GetDataByObj( business.m_pSecurity->GetInstitutionId()*10 + 3,false );
				if ( pIncome1 == NULL )
					return dRet;
				dTmp1 = pIncome1->dIncome[18];
				if ( !m_pIncomeDataFile->Load(iIndex2,30071,true))
					return dRet;
				pIncome2 = m_pIncomeDataFile->GetDataByObj( business.m_pSecurity->GetInstitutionId()*10 + 3,false );
				if ( pIncome2 == NULL )
					return dRet;
				dTmp2 = pIncome2->dIncome[18];
				dEaring += dTmp1-dTmp2;
			}
			break;
		case 3:
			{
				int iIndex1 = --iYear * 10000 + 9;
				int iIndex2 = iYear * 10000 + 3;
				double dTmp1 = 0.0;
				double dTmp2 = 0.0;
				if( !m_pIncomeDataFile->Load(iIndex1,30071,true))
					return dRet;
				pIncome1 = m_pIncomeDataFile->GetDataByObj( business.m_pSecurity->GetInstitutionId()*10 + 3,false );
				if ( pIncome1 == NULL )
					return dRet;
				dTmp1 = pIncome1->dIncome[18];
				if ( !m_pIncomeDataFile->Load(iIndex2,30071,true))
					return dRet;
				pIncome2 = m_pIncomeDataFile->GetDataByObj( business.m_pSecurity->GetInstitutionId()*10 + 3,false );
				if ( pIncome2 == NULL )
					return dRet;
				dTmp2 = pIncome2->dIncome[18];
				dEaring += dTmp1-dTmp2;
			}
			break;
		case 5:
			{
				int iIndex1 = --iYear * 10000 + 9;
				int iIndex2 = iYear * 10000 + 5;
				double dTmp1 = 0.0;
				double dTmp2 = 0.0;
				if( !m_pIncomeDataFile->Load(iIndex1,30071,true))
					return dRet;
				pIncome1 = m_pIncomeDataFile->GetDataByObj( business.m_pSecurity->GetInstitutionId()*10 + 3,false);
				if ( pIncome1 == NULL )
					return dRet;
				dTmp1 = pIncome1->dIncome[18];
				if ( !m_pIncomeDataFile->Load(iIndex2,30071,true))
					return dRet;
				pIncome2 = m_pIncomeDataFile->GetDataByObj( business.m_pSecurity->GetInstitutionId()*10 + 3,false );
				if ( pIncome2 == NULL )
					return dRet;
				dTmp2 = pIncome2->dIncome[18];
				dEaring += dTmp1-dTmp2;
			}
			break;
		case 9:
			{
				;
			}
			break;
		}
		return dPrice * dShare / dEaring;
	}
}
/*
同比市盈率----利润计算方法
1月至3月份的日期 取利润是前一年的年报的利润
4月至6月的日期，取利润是 拿该年一季报同比模拟该年的利润：（该年一季报/前一年一季报*前一年年报-）
7月至9月的日期，取利润是 拿该年中季报同比模拟该年的利润：（该年中报/前一年中报*前一年年报）
10月至12月的日期，取利润是 拿该年三季报同比模拟该年的利润：（该年三季报/前一年三季报*前一年年报）
(只根据指定的报告期计算，不往前推)

第二步，对利润进行处理
对利润进行折算：因为步骤一得出的是该公司的利润，即全部股本的利润，所以要根据计算者选择的是流通股本计算还是全部股本计算，把步骤一得到的利润进行折算
，得出相应的利润————这个利润标记为相应利润。

第三步，对价格进行处理
取相应价格，根据选择的日期，取出当天的价格（收盘价，或者均价），没有则市盈率不参与计算。
然后用价格乘以股本（根据计算者选择的是流通股本或者全部股本）得出价格————这个价格记为相应价格

第四步，计算
个股某日的市盈率为  相应价格/相应利润
合计的市盈率为 	相应价格和/相应利润和
*/
double TxIndicator::Get_Profit_From_Date( Tx::Business::TxBusiness *pBusiness, int iYear,int iQuarter, bool bSecurity /*= true*/ )
{
	double dRet = Con_doubleInvalid;
	Tx::Data::Income*	pIncome = NULL;
	if(bSecurity)
	{
		int iIndex = iYear * 100000 + iQuarter * 10 + 3;
		pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_Income,iIndex,&pIncome,false);
		if(pIncome != NULL)
			dRet = pIncome->dIncome[18];
	}
	else
	{		
		bool bLoaded = false;		
		//bug:8616   wangzf
		//int iIndex = iYear * 100000 + iQuarter;
		int iIndex = iYear * 10000 + iQuarter;
		if(m_pIncomeDataFile!=NULL)
			bLoaded = m_pIncomeDataFile->Load(iIndex,30071,true);
		if ( bLoaded )
			pIncome = m_pIncomeDataFile->GetDataByObj( pBusiness->m_pSecurity->GetInstitutionId()*10 + 3,false );
		if(pIncome == NULL)
			return dRet;
		dRet = pIncome->dIncome[18];
	}
	return dRet;
}
double	TxIndicator::Get_Profit_PE_Yoy( Tx::Business::TxBusiness *pBusiness, int& iDate, bool bSecurity)
{
	// 1 检查数据有效性
	double dRet = Con_doubleInvalid;
	if(pBusiness == NULL )
		return dRet;
	CTime tm = CTime::GetCurrentTime();
	int m_iCurDate = tm.GetYear()* 10000 + tm.GetMonth()*100 + tm.GetDay();
	if(iDate > m_iCurDate || iDate < 19600101)
		return dRet;
	int m_iYear = iDate /10000;
	int m_iMonth = iDate%10000/100;
	if(m_iMonth < 1 || m_iMonth > 12)
		return dRet;
	int iQuarter = 0;


	if ( pBusiness->m_pSecurity == NULL || !pBusiness->m_pSecurity->IsStock())
		return dRet;
	//Tx::Data::Income*	pIncome = NULL;
	//Tx::Data::Income*	pIncome1 = NULL;
	//Tx::Data::Income*	pIncome2 = NULL;
	double	dEaring = 0.0;

	if(m_iMonth < 4 && m_iMonth >0)//1月至3月份
	{
		//取利润是前一年的年报的利润
		iQuarter = 9;			
		dEaring = Get_Profit_From_Date(pBusiness,m_iYear-1,iQuarter,bSecurity);
	}
	else if(m_iMonth < 7 && m_iMonth >3)//4月至6月的日期
	{
		//该年一季报/前一年一季报*前一年年报
		iQuarter = 9;
		dEaring = Get_Profit_From_Date(pBusiness,m_iYear-1,iQuarter,bSecurity);

		iQuarter = 1;
		double dCur = Con_doubleInvalid;//该年一季报
		double dLast = Con_doubleInvalid;//前一年一季报
		dCur = Get_Profit_From_Date(pBusiness,m_iYear,iQuarter,bSecurity);
		dLast = Get_Profit_From_Date(pBusiness,m_iYear-1,iQuarter,bSecurity);
		if(dCur == Con_doubleInvalid || dLast == Con_doubleInvalid)
			return dEaring;			
		dEaring = dCur/dLast*dEaring;
	}
	else if(m_iMonth < 10 && m_iMonth >6)//7月至9月的日期
	{
		//（该年中报/前一年中报*前一年年报）
		iQuarter = 9;
		dEaring = Get_Profit_From_Date(pBusiness,m_iYear-1,iQuarter,bSecurity);

		iQuarter = 3;
		double dCur = Con_doubleInvalid;//该年中报
		double dLast = Con_doubleInvalid;//前一年中报
		dCur = Get_Profit_From_Date(pBusiness,m_iYear,iQuarter,bSecurity);
		dLast = Get_Profit_From_Date(pBusiness,m_iYear-1,iQuarter,bSecurity);
		if(dCur == Con_doubleInvalid || dLast == Con_doubleInvalid)
			return dEaring;		
		dEaring = dCur/dLast*dEaring;
	}
	else//10月至12月的日期
	{
		//该年三季报/前一年三季报*前一年年报
		iQuarter = 9;
		dEaring = Get_Profit_From_Date(pBusiness,m_iYear-1,iQuarter,bSecurity);

		iQuarter = 5;
		double dCur = Con_doubleInvalid;//该年三季报
		double dLast = Con_doubleInvalid;//前一年三季报
		dCur = Get_Profit_From_Date(pBusiness,m_iYear,iQuarter,bSecurity);
		dLast = Get_Profit_From_Date(pBusiness,m_iYear-1,iQuarter,bSecurity);
		//若为空，取中报
		if(dCur == Con_doubleInvalid || dLast == Con_doubleInvalid)
		{
			iQuarter = 3;
			dCur = Get_Profit_From_Date(pBusiness,m_iYear,iQuarter,bSecurity);
			dLast = Get_Profit_From_Date(pBusiness,m_iYear-1,iQuarter,bSecurity);
		}
		//若为再空，取去年年报
		if(dCur == Con_doubleInvalid || dLast == Con_doubleInvalid)
			return dEaring;
		dEaring = dCur/dLast*dEaring;
	}	
	return dEaring;
}
double	TxIndicator::Get_PE_Yoy_NEW(int iSecurity, int iDate , bool bSecurity)
{
	// 1 检查数据有效性
	double dRet = Con_doubleInvalid;
	CTime tm = CTime::GetCurrentTime();
	int m_iCurDate = tm.GetYear()* 10000 + tm.GetMonth()*100 + tm.GetDay();
	if(iDate > m_iCurDate || iDate < 19600101)
		return dRet;
	int m_iYear = iDate /10000;
	int m_iMonth = iDate%10000/100;
	if(m_iMonth < 1 || m_iMonth > 12)
		return dRet;
	int iQuarter = 0;

	Tx::Business::TxBusiness	business;
	business.GetSecurityNow( iSecurity );
	if ( business.m_pSecurity == NULL || !business.m_pSecurity->IsStock())
		return dRet;
	TxShareData*	pShare = business.m_pSecurity->GetTxShareDataByDate( iDate );
	if ( pShare == NULL )
		return dRet;
	double	dEaring = Con_doubleInvalid;
	double	dPrice = business.m_pSecurity->GetClosePrice( iDate, true );
	if ( dPrice == Con_floatInvalid || dPrice <- 0.0 )
		return dRet;
	double	dShare = pShare->TotalShare; 
	dEaring = Get_Profit_PE_Yoy(&business,iDate,bSecurity);
	if((dEaring < 0.001 && dEaring > -0.001)||dEaring == Con_doubleInvalid)
		return dRet;
	dRet = dPrice * dShare / dEaring;
	return dRet;
}
double	TxIndicator::Get_PE_Yoy( int iSecurity, int iDate , bool bSecurity)
{
	// 1 检查数据有效性
	double dRet = Con_doubleInvalid;
	if ( iDate > COleDateTime::GetCurrentTime().GetYear()*10000 + COleDateTime::GetCurrentTime().GetMonth()*100 + COleDateTime::GetCurrentTime().GetDay())
		return dRet;
	int iYear = iDate / 10000;
	int iQuarter = 0;
	double dFactor = 1;
	if ( iDate % 10000 < 331 )
	{
		iYear--;
		iQuarter = 9;
	}	
	else
		if ( iDate % 10000 < 630 )
		{
			iQuarter = 1;
		}
		else
			if ( iDate % 10000 < 930 )
			{
				iQuarter = 3;
			}
			else
				if ( iDate%10000 < 1231)				
				{
					iQuarter = 5;
				}
				else
				{
					iQuarter = 9;
				}
	Tx::Business::TxBusiness	business;
	business.GetSecurityNow( iSecurity );
	if ( business.m_pSecurity == NULL || !business.m_pSecurity->IsStock())
		return dRet;
	Tx::Data::Income*	pIncome = NULL;
	Tx::Data::Income*	pIncome1 = NULL;
	Tx::Data::Income*	pIncome2 = NULL;
	TxShareData*	pShare = business.m_pSecurity->GetTxShareDataByDate( iDate );
	if ( pShare == NULL )
		return dRet;
	double	dEaring = 0.0;
	double	dPrice = business.m_pSecurity->GetClosePrice( iDate, true );
	if ( dPrice == Con_floatInvalid || dPrice <- 0.0 )
		return dRet;
	double	dShare = pShare->TotalShare; 

	if ( bSecurity )//按照交易实体走
	{	//先算利润
		int iIndex = iYear * 100000 + iQuarter * 10 + 3;
		business.m_pSecurity->GetDataByObj( Tx::Data::dt_Income,iIndex,&pIncome,false);
		if ( pIncome == NULL )
		{
			switch( iQuarter )
			{
			case 1:
				iQuarter = 9;
				iYear--;
				break;
			case 3:
				iQuarter = 1;
				dFactor = 4;
				break;
			case 5:
				iQuarter = 3;
				dFactor =2; 
				break;
			case 9:
				iQuarter = 5;
				dFactor = double(4)/3;
				break;
			}
			iIndex = iYear * 100000 + iQuarter*10 + 3;
			business.m_pSecurity->GetDataByObj( Tx::Data::dt_Income,iIndex,&pIncome,false);
			if ( pIncome == NULL )
				return dRet;
		}
		dEaring = pIncome->dIncome[18];
		switch(iQuarter)
		{
		case 1:
			{
				int iIndex1 = --iYear * 100000 + 90 + 3;
				int iIndex2 = iYear * 100000 + 10 + 3;
				business.m_pSecurity->GetDataByObj( Tx::Data::dt_Income,iIndex1,&pIncome1,false);
				business.m_pSecurity->GetDataByObj( Tx::Data::dt_Income,iIndex2,&pIncome2,false);
				if ( pIncome2 == NULL || pIncome1 == NULL )
					return dRet;
				dEaring *= pIncome1->dIncome[18]/pIncome2->dIncome[18];
			}
			break;
		case 3:
			{
				int iIndex1 = --iYear * 100000 + 90 + 3;
				int iIndex2 = iYear * 100000 + 30 + 3;
				business.m_pSecurity->GetDataByObj( Tx::Data::dt_Income,iIndex1,&pIncome1,false);
				business.m_pSecurity->GetDataByObj( Tx::Data::dt_Income,iIndex2,&pIncome2,false);
				if ( pIncome2 == NULL || pIncome1 == NULL )
					return dRet;
				dEaring *= pIncome1->dIncome[18]/pIncome2->dIncome[18];
			}
			break;
		case 5:
			{
				int iIndex1 = --iYear * 100000 + 90 + 3;
				int iIndex2 = iYear * 100000 + 50 + 3;
				business.m_pSecurity->GetDataByObj( Tx::Data::dt_Income,iIndex1,&pIncome1,false);
				business.m_pSecurity->GetDataByObj( Tx::Data::dt_Income,iIndex2,&pIncome2,false);
				if ( pIncome2 == NULL || pIncome1 == NULL )
					return dRet;
				dEaring *= pIncome1->dIncome[18]/pIncome2->dIncome[18];
			}
			break;
		case 9:
			//{
			//	int iIndex1 = --iYear * 100000 + 90 + 3;
			//	int iIndex2 = iYear * 100000 + 10 + 3;
			//	business.m_pSecurity->GetDataByObj( Tx::Data::dt_Income,iIndex1,&pIncome1,false);
			//	business.m_pSecurity->GetDataByObj( Tx::Data::dt_Income,iIndex2,&pIncome2,false);
			//	if ( pIncome2 == NULL || pIncome1 == NULL )
			//		return dRet;
			//	dEaring += pIncome2->dIncome[18]-pIncome1->dIncome[18];
			//}
			break;
		}
		return dPrice * dShare / dEaring;
	}
	else			//按照财年财季走
	{
		int iIndex = iYear * 10000 + iQuarter ;
		bool bLoaded = false;
		if(m_pIncomeDataFile!=NULL)
			bLoaded = m_pIncomeDataFile->Load(iIndex,30071,true);
		if ( !bLoaded)
		{
			switch( iQuarter )
			{
			case 1:
				iQuarter = 9;
				iYear--;
				break;
			case 3:
				iQuarter = 1;
				break;
			case 5:
				iQuarter = 3;
				break;
			case 9:
				iQuarter = 5;
				break;
			}
			iIndex = iYear * 10000 + iQuarter;
			bLoaded = m_pIncomeDataFile->Load(iIndex,30071,true);
			if ( bLoaded )
				pIncome = m_pIncomeDataFile->GetDataByObj( business.m_pSecurity->GetInstitutionId()*10 + 3,false );
		}
		else
		{
			pIncome = m_pIncomeDataFile->GetDataByObj( business.m_pSecurity->GetInstitutionId()*10 + 3,false );
			if ( pIncome == NULL )
			{
				switch( iQuarter )
				{
				case 1:
					iQuarter = 9;
					iYear--;
					break;
				case 3:
					iQuarter = 1;
					break;
				case 5:
					iQuarter = 3;
					break;
				case 9:
					iQuarter = 5;
					break;
				}
				iIndex = iYear * 10000 + iQuarter;
				bLoaded = m_pIncomeDataFile->Load(iIndex,30071,true);
				if ( bLoaded )
					pIncome = m_pIncomeDataFile->GetDataByObj( business.m_pSecurity->GetInstitutionId()*10 + 3,false );
			}
		}
		if ( pIncome == NULL)
			return dRet;
		dEaring = pIncome->dIncome[18];
		switch( iQuarter )
		{
		case 1:
			{
				int iIndex1 = --iYear * 10000 + 9;
				int iIndex2 = iYear * 10000 + 1;
				double dTmp1 = 0.0;
				double dTmp2 = 0.0;
				if( !m_pIncomeDataFile->Load(iIndex1,30071,true))
					return dRet;
				pIncome1 = m_pIncomeDataFile->GetDataByObj( business.m_pSecurity->GetInstitutionId()*10 + 3);
				if ( pIncome1 == NULL)
					return dRet;
				dTmp1 = pIncome1->dIncome[18];
				if ( !m_pIncomeDataFile->Load(iIndex2,30071,true))
					return dRet;
				pIncome2 = m_pIncomeDataFile->GetDataByObj( business.m_pSecurity->GetInstitutionId()*10 + 3);
				if ( pIncome2 == NULL)
					return dRet;
				dTmp2 = pIncome2->dIncome[18];
				dEaring *= dTmp1/dTmp2;
			}
			break;
		case 3:
			{
				int iIndex1 = --iYear * 10000 + 9;
				int iIndex2 = iYear * 10000 + 3;
				double dTmp1 = 0.0;
				double dTmp2 = 0.0;
				if( !m_pIncomeDataFile->Load(iIndex1,30071,true))
					return dRet;
				pIncome1 = m_pIncomeDataFile->GetDataByObj( business.m_pSecurity->GetInstitutionId()*10 + 3);
				if ( pIncome1 == NULL)
					return dRet;
				dTmp1 = pIncome1->dIncome[18];
				if ( !m_pIncomeDataFile->Load(iIndex2,30071,true))
					return dRet;
				pIncome2 = m_pIncomeDataFile->GetDataByObj( business.m_pSecurity->GetInstitutionId()*10 + 3);
				if ( pIncome2 == NULL)
					return dRet;
				dTmp2 = pIncome2->dIncome[18];
				dEaring *= dTmp1/dTmp2;
			}
			break;
		case 5:
			{
				int iIndex1 = --iYear * 10000 + 9;
				int iIndex2 = iYear * 10000 + 5;
				double dTmp1 = 0.0;
				double dTmp2 = 0.0;
				if( !m_pIncomeDataFile->Load(iIndex1,30071,true))
					return dRet;
				pIncome1 = m_pIncomeDataFile->GetDataByObj( business.m_pSecurity->GetInstitutionId()*10 + 3);
				if ( pIncome1 == NULL)
					return dRet;
				dTmp1 = pIncome1->dIncome[18];
				if ( !m_pIncomeDataFile->Load(iIndex2,30071,true))
					return dRet;
				pIncome2 = m_pIncomeDataFile->GetDataByObj( business.m_pSecurity->GetInstitutionId()*10 + 3);
				if ( pIncome2 == NULL)
					return dRet;
				dTmp2 = pIncome2->dIncome[18];
				dEaring *= dTmp1/dTmp2;
			}
			break;
		case 9:
			{
				;
			}
			break;
		}
		return dPrice * dShare / dEaring;
	}
}
double	TxIndicator::Get_PB( int iSecurity, int iDate , bool bSecurity)
{
	// 1 检查数据有效性
	double dRet = Con_doubleInvalid;
	if ( iDate > COleDateTime::GetCurrentTime().GetYear()*10000 + COleDateTime::GetCurrentTime().GetMonth()*100 + COleDateTime::GetCurrentTime().GetDay())
		return dRet;
	int iYear = iDate / 10000;
	int iQuarter = 0;
	double dFactor = 1;
	if ( iDate % 10000 < 331 )
	{
		iYear--;
		iQuarter = 9;
	}	
	else
		if ( iDate % 10000 < 630 )
		{
			iQuarter = 1;
			dFactor = 4;
		}
		else
			if ( iDate % 10000 < 930 )
			{
				iQuarter = 3;
				dFactor = 2;
			}
			else
				if ( iDate %10000 < 1231)
				{
					iQuarter = 5;
					dFactor = double(4)/3;
				}
				else
				{
					iQuarter = 9;
				}
	Tx::Business::TxBusiness	business;
	business.GetSecurityNow( iSecurity );
	if ( business.m_pSecurity == NULL || !business.m_pSecurity->IsStock())
		return dRet;
	Tx::Data::Balance*	pBalance = NULL;
	TxShareData*	pShare = business.m_pSecurity->GetTxShareDataByDate( iDate );
	if ( pShare == NULL )
		return dRet;
	double	dEaring = 0.0;
	double	dPrice = business.m_pSecurity->GetClosePrice( iDate, true );
	if ( dPrice == Con_floatInvalid || dPrice <= 0.0 )
		return dRet;
	double	dShare = pShare->TotalShare; 

	if ( bSecurity )//按照交易实体走
	{	//先算利润
		int iIndex = iYear * 100000 + iQuarter * 10 + 3;
		business.m_pSecurity->GetDataByObj( Tx::Data::dt_Balance,iIndex,&pBalance,false);
		if ( pBalance == NULL )
		{
			switch( iQuarter )
			{
			case 1:
				iQuarter = 9;
				dFactor = 1;
				iYear--;
				break;
			case 3:
				iQuarter = 1;
				dFactor = 4;
				break;
			case 5:
				iQuarter = 3;
				dFactor =2; 
				break;
			case 9:
				iQuarter = 5;
				dFactor = double(4)/3;
				break;
			}
			iIndex = iYear * 100000 + iQuarter*10 + 3;
			business.m_pSecurity->GetDataByObj( Tx::Data::dt_Balance,iIndex,&pBalance,false);
			if ( pBalance == NULL )
				return dRet;
		}
		dFactor = 1;
		dEaring = dFactor * pBalance->dBalance[66];
		return dPrice * dShare / dEaring;
	}
	else			//按照财年财季走
	{
		int iIndex = iYear * 10000 + iQuarter;
		bool bLoaded = false;
		if(m_pBalanceDataFile!=NULL)
			bLoaded = m_pBalanceDataFile->Load(iIndex,30067,true);
		if ( !bLoaded)
		{
			switch( iQuarter )
			{
			case 1:
				iQuarter = 9;
				dFactor = 1;
				iYear--;
				break;
			case 3:
				iQuarter = 1;
				dFactor = 4;
				break;
			case 5:
				iQuarter = 3;
				dFactor = 2;
				break;
			case 9:
				iQuarter = 5;
				dFactor = double(4)/3;
				break;
			}
			iIndex = iYear * 10000 + iQuarter;
			bLoaded = m_pBalanceDataFile->Load(iIndex,30067,true);
			if ( bLoaded )
				pBalance = m_pBalanceDataFile->GetDataByObj( business.m_pSecurity->GetInstitutionId()*10 + 3,false);
		}
		else
		{
			pBalance = m_pBalanceDataFile->GetDataByObj( business.m_pSecurity->GetInstitutionId()*10 + 3);
			if ( pBalance == NULL )
			{
				switch( iQuarter )
				{
				case 1:
					iQuarter = 9;
					dFactor = 1;
					iYear--;
					break;
				case 3:
					iQuarter = 1;
					dFactor = 4;
					break;
				case 5:
					iQuarter = 3;
					dFactor = 2;
					break;
				case 9:
					iQuarter = 5;
					dFactor = double(4)/3;
					break;
				}
				iIndex = iYear * 10000 + iQuarter;
				bLoaded = m_pBalanceDataFile->Load(iIndex,30067,true);
				if ( bLoaded )
					pBalance = m_pBalanceDataFile->GetDataByObj( business.m_pSecurity->GetInstitutionId()*10 + 3,false);
			}
		}
		if ( pBalance == NULL )
			return dRet;
		dFactor = 1;
		dEaring = dFactor * pBalance->dBalance[66];
		return dPrice * dShare / dEaring;
	}
}
double	TxIndicator::Get_PE_Basic( std::set<int> iSet, int iDate, bool bsecurity )
{
	return 0.0;
}
double	TxIndicator::Get_PE_Ltm( std::set<int> iSet, int iDate, bool bsecurity )
{
	return 0.0;
}
double	TxIndicator::Get_PE_Yoy( std::set<int> iSet, int iDate, bool bsecurity )
{
	return 0.0;
}
double	TxIndicator::Get_PB( std::set<int> iSet, int iDate, bool bsecurity )
{
	return 0.0;
}
//-------------------------------------------------------------------------
DOUBLE TxIndicator::GetKLine(Tx::Business::TxBusiness *pBusiness, int nDate, int iType,int m_iSecId)
{
	int nTradeDateIdx, nTradeDate, nLatestTradeDate, nCurrentDate;
	DOUBLE dValue = 0;
	DAY_HQ_ITEM *pDayHQItem;
	//Tx::Business::TxBusiness business;
	BOOL bHisHQ = TRUE;		//是否需要下载历史K线行情文件
	CString sLog;
	//作nDate是否超越已停牌股票最后一个交易日的判断
	nLatestTradeDate = pBusiness->m_pSecurity->GetStopedLastDate();
	int nFirstDay = 0;//pBusiness->m_pSecurity->GetTradeDateByIndex(0);
	//if( (nLatestTradeDate > 0 && nDate > nLatestTradeDate) || nDate < nFirstDay )
	//	return 0;
	nCurrentDate = (CTime::GetCurrentTime()).GetYear() * 10000 + (CTime::GetCurrentTime()).GetMonth() * 100 + (CTime::GetCurrentTime()).GetDay();
	if(pBusiness->m_pSecurity->IsShanghai() || pBusiness->m_pSecurity->IsTianxiang())
	{	//取得上证指数交易日期序列
		//business.GetSecurityNow((LONG)iSHIndexID);
		//bHisHQ = FALSE;
		////这里将异步加载改为同步，否则取记录个数会取错20090104--wangzhy--
		//business.m_pSecurity->LoadHisTrade();
		//business.m_pSecurity->LoadTradeDate();
		//nLatestTradeDate = business.m_pSecurity->GetTradeDateLatest();
		//sLog.Format(_T("当前日期%d上证最新交易日期为%d.\r\n"),nCurrentDate,nLatestTradeDate);
		//Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
		bHisHQ = FALSE;
		nLatestTradeDate = pBusiness->m_pShIndex->GetTradeDateLatest();
	}else if(pBusiness->m_pSecurity->IsShenzhen())
	{	//取得深证成指交易日期序列
		/*business.GetSecurityNow((LONG)iSZIndexID);
		bHisHQ = FALSE;
		business.m_pSecurity->LoadHisTrade();
		business.m_pSecurity->LoadTradeDate();
		nLatestTradeDate = business.m_pSecurity->GetTradeDateLatest();
		sLog.Format(_T("当前日期%d深证最新交易日期为%d.\r\n"),nCurrentDate,nLatestTradeDate);
		Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);*/
		bHisHQ = FALSE;
		nLatestTradeDate = pBusiness->m_pSzIndex->GetTradeDateLatest();
	}else if ( pBusiness->m_pSecurity->IsIndexCustomByCode())
	{
		int nCount = pBusiness->m_pSecurity->GetTradeDataCount();
		if ( nCount > 0 )
		{
			bHisHQ = TRUE;
			HisTradeData*	pTrade = pBusiness->m_pSecurity->GetTradeDataByIndex( nCount-1 );
			if ( pTrade != NULL)
				nLatestTradeDate = pTrade->Date;
		}
	}else
	{	//港股
		nLatestTradeDate = pBusiness->m_pSecurity->GetTradeDateLatest();
	}
	//申万指数
	std::set<int>::iterator m_iter;
	m_iter = m_setSWIndex.find(m_iSecId);
	if(m_iter != m_setSWIndex.end())
		bHisHQ = TRUE;
	//nCurrentDate = (CTime::GetCurrentTime()).GetYear() * 10000 + (CTime::GetCurrentTime()).GetMonth() * 100 + (CTime::GetCurrentTime()).GetDay();
	if(nDate == nCurrentDate)	//区当前交易日
	{
		if(pBusiness->m_pSecurity->IsValid())
		{	//今日是交易日
			switch(iType)
			{
			case 0:		//开盘价
				dValue = pBusiness->m_pSecurity->GetOpenPrice();
				break;
			case 1:		//前收价
				dValue = pBusiness->m_pSecurity->GetPreClosePrice();
				break;
			case 2:		//最高价
				dValue = pBusiness->m_pSecurity->GetHighPrice();
				break;
			case 3:		//最低价
				dValue = pBusiness->m_pSecurity->GetLowPrice();
				break;
			case 4:		//收盘价
				dValue = pBusiness->m_pSecurity->GetClosePrice();
				break;
			case 5:		//成交金额
				dValue = pBusiness->m_pSecurity->GetAmount();
				break;
			case 6:		//成交量
				dValue = pBusiness->m_pSecurity->GetVolume();
				break;
			case 7:		//涨跌值
				dValue = pBusiness->m_pSecurity->GetClosePrice() - pBusiness->m_pSecurity->GetPreClosePrice();
				break;
			case 8:		//涨跌幅
				dValue = (pBusiness->m_pSecurity->GetClosePrice()/pBusiness->m_pSecurity->GetPreClosePrice()-1) * 100;
				break;
			case 9:
				{
					if(pBusiness->m_pSecurity->IsRepurchase())
					{
						Tx::Data::RepurchaseAvgPrice	repoData;
						pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_RepurchaseAvgPrice, nDate, repoData );
						dValue = repoData.dAvgPrice;
						if(dValue < 0.01)
							dValue = pBusiness->m_pSecurity->GetPreClosePrice();
					}
					else
					{
						if(pBusiness->m_pSecurity->GetVolume() > 0 && pBusiness->m_pSecurity->GetVolume() != Con_doubleInvalid)
							dValue = pBusiness->m_pSecurity->GetAmount()/(DOUBLE)pBusiness->m_pSecurity->GetVolume();
					}
					
				}
				
				break;
			default:	
				break;
			}
		}else
		{	//今日停牌
			if(pBusiness->m_pSecurity->IsStop())
			{	//摘牌
				; //return 0;
			}
			switch(iType)
			{
			case 0:		//开盘价
			case 1:		//前收价
			case 2:		//最高价
			case 3:		//最低价
			case 4:		//收盘价
			case 9:
				if(bHisHQ)
				{	//需要下载历史K线行情文件
					dValue = pBusiness->m_pSecurity->GetClosePrice();	//在停牌的日子里，默认回取到最后一个交易日的收盘价
					//dValue = pBusiness->m_pSecurity->GetClosePrice(nLatestTradeDate);
				}else
				{
					//这个地方经常nLatestTradeDate取得不正确，故此在周六的时候取到周四的数据
					pDayHQItem = pBusiness->m_pSecurity->GetOneDayTradeData(nLatestTradeDate);
					if(pDayHQItem == NULL)
					{
						dValue = 0;
					}else
					{
						dValue = (DOUBLE)pDayHQItem->Close;
					}
				}
				break;
			case 5:		//成交金额
			case 6:		//成交量
			case 7:
			case 8:
				dValue = 0;
				break;
			default:
				break;
			}
		}

		return dValue;
	}

	//Bug:13891  / wangzf / 2012-12-05
	//if(nLatestTradeDate <= 0)
	//{
	//	return 0;
	//}
	if(nDate > nLatestTradeDate)
	{	//最近一个交易日之后
		if(pBusiness->m_pSecurity->IsStop())
		{	//摘牌
			return 0;
		}
		//switch(iType)
		//{
		//case 0:		//开盘价
		//case 1:		//前收价
		//case 2:		//最高价
		//case 3:		//最低价
		//case 4:		//收盘价
		//case 9:
		//	if(bHisHQ)
		//	{	//需要下载历史K线行情文件
		//		dValue = pBusiness->m_pSecurity->GetClosePrice(nLatestTradeDate);
		//	}else
		//	{
		//		pDayHQItem = pBusiness->m_pSecurity->GetOneDayTradeData(nLatestTradeDate);
		//		if(pDayHQItem == NULL)
		//		{
		//			dValue = 0;
		//		}else
		//		{
		//			dValue = (DOUBLE)pDayHQItem->Close;
		//		}
		//	}
		//	break;
		//case 5:		//成交金额
		//case 6:		//成交量
		//case 7:
		//case 8:
		//	dValue = 0;
		//	break;
		//default:
		//	break;
		//}
	}//else
	{	//截止最近一个交易日
		if(bHisHQ)
		{	//需要下载历史K线行情文件
			nTradeDateIdx = pBusiness->m_pSecurity->GetTradeDateIndex(nDate);
			nTradeDate = pBusiness->m_pSecurity->GetTradeDateByIndex(nTradeDateIdx);
			if(nTradeDate < nDate)
			{	//nDate停牌
				switch(iType)
				{
				case 0:		//开盘价
				case 1:		//前收价
				case 2:		//最高价
				case 3:		//最低价
				case 4:		//收盘价
				case 9:
					//dValue = pBusiness->m_pSecurity->GetClosePrice(nTradeDate);
					dValue = pBusiness->m_pSecurity->GetClosePrice(nDate);
					break;
				case 5:		//成交金额
				case 6:		//成交量
				case 7:
				case 8:				
					dValue = 0;
					break;
				default:
					break;
				}
			}else
			{	//nDate交易日
				switch(iType)
				{
				case 0:		//开盘价
					dValue = pBusiness->m_pSecurity->GetOpenPrice(nDate);
					break;
				case 1:		//前收价
					dValue = pBusiness->m_pSecurity->GetPreClosePrice(nDate);
					break;
				case 2:		//最高价
					dValue = pBusiness->m_pSecurity->GetHighPrice(nDate);
					break;
				case 3:		//最低价
					dValue = pBusiness->m_pSecurity->GetLowPrice(nDate);
					break;
				case 4:		//收盘价
					dValue = pBusiness->m_pSecurity->GetClosePrice(nDate);
					break;
				case 5:		//成交金额
					dValue = pBusiness->m_pSecurity->GetAmount(nDate);
					break;
				case 6:		//成交量
					dValue = pBusiness->m_pSecurity->GetVolume(nDate);
					break;
				case 7:		//涨跌值
					dValue = pBusiness->m_pSecurity->GetClosePrice(nDate) - pBusiness->m_pSecurity->GetPreClosePrice(nDate);
					break;
				case 8:		//涨跌幅
					dValue = (pBusiness->m_pSecurity->GetClosePrice(nDate)/pBusiness->m_pSecurity->GetPreClosePrice(nDate)-1) * 100;
					break;
				case 9:
					{
						if(pBusiness->m_pSecurity->IsRepurchase())
						{
							Tx::Data::RepurchaseAvgPrice	repoData;
							pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_RepurchaseAvgPrice, nDate, repoData );
							dValue = repoData.dAvgPrice;
							if(dValue < 0.01)
								dValue = pBusiness->m_pSecurity->GetPreClosePrice();
						}
						else
						{
							if(pBusiness->m_pSecurity->GetVolume(nDate) > 0 && pBusiness->m_pSecurity->GetVolume(nDate) != Con_doubleInvalid)
								dValue = pBusiness->m_pSecurity->GetAmount(nDate)/(DOUBLE)pBusiness->m_pSecurity->GetVolume(nDate);
						}
					}					
					break;
				default:	
					break;
				}
			}
		}else
		{
			nTradeDateIdx = pBusiness->m_pSecurity->GetTradeDateIndex(nDate);
			if( nTradeDateIdx < 0 )
			{
				nTradeDate = nDate;
				switch(iType)
				{
				case 0:		//开盘价
				case 1:		//前收价
				case 2:		//最高价
				case 3:		//最低价
				case 4:		//收盘价
				case 9:
					pDayHQItem = pBusiness->m_pSecurity->GetOneDayTradeData(nTradeDate);
					if(pDayHQItem == NULL)
					{
						dValue = 0;
					}else
					{
						dValue = (DOUBLE)pDayHQItem->Close;
					}
					break;
				case 5:		//成交金额
				case 6:		//成交量
				case 7:
				case 8:
					dValue = 0;
					break;
				default:
					break;
				}
			}
			else
			{
				nTradeDate = pBusiness->m_pSecurity->GetTradeDateByIndex(nTradeDateIdx);
				pDayHQItem = pBusiness->m_pSecurity->GetOneDayTradeData(nTradeDate);
				if(pDayHQItem == NULL)
				{
					return 0;
				}
				if ( nTradeDate != nDate )
				{
					switch(iType)
					{
					case 0:		//开盘价
					case 1:		//前收价
					case 2:		//最高价
					case 3:		//最低价
					case 4:		//收盘价
					case 9:
						dValue = (DOUBLE)pDayHQItem->Close;
						break;
					case 5:		//成交金额
					case 6:		//成交量
					case 7:
					case 8:
						dValue = 0.0;
						break;
					default:	
						break;
					}
				}
				else
				{
					switch(iType)
					{
					case 0:		//开盘价
						dValue = (DOUBLE)pDayHQItem->Open;
						break;
					case 1:		//前收价
						dValue = (DOUBLE)pDayHQItem->Preclose;
						break;
					case 2:		//最高价
						dValue = (DOUBLE)pDayHQItem->High;
						break;
					case 3:		//最低价
						dValue = (DOUBLE)pDayHQItem->Low;
						break;
					case 4:		//收盘价
						dValue = (DOUBLE)pDayHQItem->Close;
						break;
					case 5:		//成交金额
						dValue = pDayHQItem->Amount;
						break;
					case 6:		//成交量
						dValue = pDayHQItem->Volume;
						break;
					case 7:
						dValue = (DOUBLE)pDayHQItem->Close - (DOUBLE)pDayHQItem->Preclose;
						break;
					case 8:
						dValue = ((DOUBLE)pDayHQItem->Close/(DOUBLE)pDayHQItem->Preclose-1)*100;
						break;
					case 9:
						{
							if(pBusiness->m_pSecurity->IsRepurchase())
							{
								Tx::Data::RepurchaseAvgPrice	repoData;
								pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_RepurchaseAvgPrice, nDate, repoData );
								dValue = repoData.dAvgPrice;
								if(dValue < 0.01)
									dValue = pBusiness->m_pSecurity->GetPreClosePrice();
							}
							else
							{
								if(pDayHQItem->Volume > 0 && pDayHQItem->Volume != Con_doubleInvalid)
									dValue = pDayHQItem->Amount/(DOUBLE)pDayHQItem->Volume;
							}
						}						
						break;
					default:	
						break;
					}
				}

			}
		}
	}
		//	nTradeDate = pBusiness->m_pSecurity->GetTradeDateByIndex(nTradeDateIdx);
		//	if(nTradeDate < nDate)
		//	{	//nDate交易所停牌
		//		switch(iType)
		//		{
		//		case 0:		//开盘价
		//		case 1:		//前收价
		//		case 2:		//最高价
		//		case 3:		//最低价
		//		case 4:		//收盘价
		//			pDayHQItem = pBusiness->m_pSecurity->GetOneDayTradeData(nTradeDate);
		//			if(pDayHQItem == NULL)
		//			{
		//				dValue = 0;
		//			}else
		//			{
		//				dValue = (DOUBLE)pDayHQItem->Close;
		//			}
		//			break;
		//		case 5:		//成交金额
		//		case 6:		//成交量
		//		case 7:
		//		case 8:
		//			dValue = 0;
		//			break;
		//		default:
		//			break;
		//		}
		//	}else
		//	{	//nDate交易所交易日
		//		pDayHQItem = pBusiness->m_pSecurity->GetOneDayTradeData(nTradeDate);
		//		if(pDayHQItem == NULL)
		//		{
		//			return 0;
		//		}
		//		switch(iType)
		//		{
		//		case 0:		//开盘价
		//			dValue = (DOUBLE)pDayHQItem->Open;
		//			break;
		//		case 1:		//前收价
		//			dValue = (DOUBLE)pDayHQItem->Preclose;
		//			break;
		//		case 2:		//最高价
		//			dValue = (DOUBLE)pDayHQItem->High;
		//			break;
		//		case 3:		//最低价
		//			dValue = (DOUBLE)pDayHQItem->Low;
		//			break;
		//		case 4:		//收盘价
		//			dValue = (DOUBLE)pDayHQItem->Close;
		//			break;
		//		case 5:		//成交金额
		//			dValue = pDayHQItem->Amount;
		//			break;
		//		case 6:		//成交量
		//			dValue = pDayHQItem->Volume;
		//			break;
		//		case 7:
		//			dValue = (DOUBLE)pDayHQItem->Close - (DOUBLE)pDayHQItem->Preclose;
		//			break;
		//		case 8:
		//			dValue = ((DOUBLE)pDayHQItem->Close/(DOUBLE)pDayHQItem->Preclose-1)*100;
		//			break;
		//		default:	
		//			break;
		//		}
		//	}
		//}
	return dValue;
}

DOUBLE TxIndicator::GetKLineEx(Tx::Business::TxBusiness *pBusiness, int nDate, int iType,int m_iSecId)
{
	int nTradeDateIdx, nTradeDate, nLatestTradeDate, nCurrentDate;

	DOUBLE dValue = 0;
	HisTradeData *pTradeData = NULL;
	//DAY_HQ_ITEM *pDayHQItem;
	//Tx::Business::TxBusiness business;
	BOOL bHisHQ = TRUE;		//是否需要下载历史K线行情文件
	CString sLog;
	//作nDate是否超越已停牌股票最后一个交易日的判断
	nLatestTradeDate = pBusiness->m_pSecurity->GetStopedLastDate();
	// mantis:14059  wangzf  2013-04-16
	int nFirstDay = 0;//pBusiness->m_pSecurity->GetTradeDateByIndex(0);
	if( (nLatestTradeDate > 0 && nDate > nLatestTradeDate) || nDate < nFirstDay )
		return 0;
	nCurrentDate = (CTime::GetCurrentTime()).GetYear() * 10000 + (CTime::GetCurrentTime()).GetMonth() * 100 + (CTime::GetCurrentTime()).GetDay();
	if(pBusiness->m_pSecurity->IsShanghai() || pBusiness->m_pSecurity->IsTianxiang())
	{	//取得上证指数交易日期序列
		/*business.GetSecurityNow((LONG)iSHIndexID);
		bHisHQ = FALSE;
		business.m_pSecurity->LoadHisTrade();
		business.m_pSecurity->LoadTradeDate();
		nLatestTradeDate = business.m_pSecurity->GetTradeDateLatest();
		sLog.Format(_T("当前日期%d上证最新交易日期为%d.\r\n"),nCurrentDate,nLatestTradeDate);
		Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);*/
		bHisHQ = FALSE;
		nLatestTradeDate = pBusiness->m_pShIndex->GetTradeDateLatest();
	}else if(pBusiness->m_pSecurity->IsShenzhen())
	{	//取得深证成指交易日期序列
		//business.GetSecurityNow((LONG)iSZIndexID);
		//bHisHQ = FALSE;
		//business.m_pSecurity->LoadHisTrade();
		//business.m_pSecurity->LoadTradeDate();
		//nLatestTradeDate = business.m_pSecurity->GetTradeDateLatest();
		//sLog.Format(_T("当前日期%d深证最新交易日期为%d.\r\n"),nCurrentDate,nLatestTradeDate);
		//Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
		bHisHQ = FALSE;
		nLatestTradeDate = pBusiness->m_pSzIndex->GetTradeDateLatest();
	}else if ( pBusiness->m_pSecurity->IsIndexCustomByCode())
	{
		int nCount = pBusiness->m_pSecurity->GetTradeDataCount();
		if ( nCount > 0 )
		{
			bHisHQ = TRUE;
			HisTradeData*	pTrade = pBusiness->m_pSecurity->GetTradeDataByIndex( nCount-1 );
			if ( pTrade != NULL)
				nLatestTradeDate = pTrade->Date;
		}
	}
	else
	{	//港股
		nLatestTradeDate = pBusiness->m_pSecurity->GetTradeDateLatest();
	}

	nCurrentDate = (CTime::GetCurrentTime()).GetYear() * 10000 + (CTime::GetCurrentTime()).GetMonth() * 100 + (CTime::GetCurrentTime()).GetDay();
	if(nDate == nCurrentDate)
	{
		if(pBusiness->m_pSecurity->IsValid())
		{	//今日是交易日
			switch(iType)
			{
			case 0:		//开盘价
				dValue = pBusiness->m_pSecurity->GetOpenPrice();
				break;
			case 1:		//前收价价
				dValue = pBusiness->m_pSecurity->GetPreClosePrice();
				break;
			case 2:		//最高价
				dValue = pBusiness->m_pSecurity->GetHighPrice();
				break;
			case 3:		//最低价
				dValue = pBusiness->m_pSecurity->GetLowPrice();
				break;
			case 4:		//收盘价
				dValue = pBusiness->m_pSecurity->GetClosePrice();
				break;
			case 5:		//成交金额
				dValue = pBusiness->m_pSecurity->GetAmount();
				break;
			case 6:		//成交量
				dValue = pBusiness->m_pSecurity->GetVolume();
				break;
			case 7:		//涨跌值
				dValue = pBusiness->m_pSecurity->GetClosePrice() - pBusiness->m_pSecurity->GetPreClosePrice();
				break;
			case 8:		//涨跌幅
				dValue = (pBusiness->m_pSecurity->GetClosePrice()/pBusiness->m_pSecurity->GetPreClosePrice() - 1) * 100;
				break;
			default:	
				break;
			}
		}
		else
		{	//今日停牌
			dValue = 0.0;
			if(pBusiness->m_pSecurity->IsStop())
			{	//摘牌
				return dValue;
			}
			if(bHisHQ)
			{
				//开盘价//前收价//最高价//最低价//收盘价
				if(iType == 0 || iType == 1 ||iType == 2 || iType == 3 || iType == 4)
					dValue = pBusiness->m_pSecurity->GetClosePrice(nLatestTradeDate);
				else if(iType == 5)//成交金额
					dValue = pBusiness->m_pSecurity->GetAmount(nLatestTradeDate);
				else if(iType == 6)//成交量
					dValue = pBusiness->m_pSecurity->GetVolume(nLatestTradeDate);
			}
			else
			{
				pTradeData = pBusiness->m_pSecurity->GetTradeDataByNatureDate( nLatestTradeDate,tag_DI_End);
				if(pTradeData != NULL)
				{
					//开盘价//前收价//最高价//最低价//收盘价
					if(iType == 0 || iType == 1 ||iType == 2 || iType == 3 || iType == 4)
						dValue = (DOUBLE)pTradeData->Close;
					else if(iType == 5)//成交金额
						dValue = (DOUBLE)pTradeData->Amount;
					else if(iType == 6)//成交量
						dValue = (DOUBLE)pTradeData->Volume;
				}
			}
		}

		return dValue;
	}
    //Bug:13891  / wangzf / 2012-12-05
	//if(nLatestTradeDate <= 0)
	//{
	//	return 0;
	//}
	if(nDate > nLatestTradeDate)
	{	//最近一个交易日之后
		dValue = 0.0;
		if(pBusiness->m_pSecurity->IsStop())
		{	//摘牌
			return dValue;
		}
	}	
	//	if(bHisHQ)
	//	{
	//		//开盘价//前收价//最高价//最低价//收盘价
	//		if(iType == 0 || iType == 1 ||iType == 2 || iType == 3 || iType == 4)
	//			dValue = pBusiness->m_pSecurity->GetClosePrice(nLatestTradeDate);
	//		else if(iType == 5)//成交金额
	//			dValue = pBusiness->m_pSecurity->GetAmount(nLatestTradeDate);
	//		else if(iType == 6)//成交量
	//			dValue = pBusiness->m_pSecurity->GetVolume(nLatestTradeDate);
	//	}
	//	else
	//	{
	//		pTradeData = pBusiness->m_pSecurity->GetTradeDataByNatureDate( nLatestTradeDate,tag_DI_End);
	//		if(pTradeData != NULL)
	//		{
	//			//开盘价//前收价//最高价//最低价//收盘价
	//			if(iType == 0 || iType == 1 ||iType == 2 || iType == 3 || iType == 4)
	//				dValue = (DOUBLE)pTradeData->Close;
	//			else if(iType == 5)//成交金额
	//				dValue = (DOUBLE)pTradeData->Amount;
	//			else if(iType == 6)//成交量
	//				dValue = (DOUBLE)pTradeData->Volume;
	//		}
	//	}
	//}else
	{	//截止最近一个交易日
		if(bHisHQ)
		{	//需要下载历史K线行情文件
			nTradeDateIdx = pBusiness->m_pSecurity->GetTradeDateIndex(nDate);
			nTradeDate = pBusiness->m_pSecurity->GetTradeDateByIndex(nTradeDateIdx);
			if(nTradeDate < nDate)
			{	//nDate停牌
				//开盘价//前收价//最高价//最低价//收盘价
				if(iType == 0 || iType == 1 ||iType == 2 || iType == 3 || iType == 4)
					dValue = pBusiness->m_pSecurity->GetClosePrice(nDate);
				else if(iType == 5)//成交金额
					dValue = pBusiness->m_pSecurity->GetAmount(nDate);
				else if(iType == 6)//成交量
					dValue = pBusiness->m_pSecurity->GetVolume(nDate);
			}else
			{	//nDate交易日
				switch(iType)
				{
				case 0:		//开盘价
					dValue = pBusiness->m_pSecurity->GetOpenPrice(nDate);
					break;
				case 1:		//前收价
					dValue = pBusiness->m_pSecurity->GetPreClosePrice(nDate);
					break;
				case 2:		//最高价
					dValue = pBusiness->m_pSecurity->GetHighPrice(nDate);
					break;
				case 3:		//最低价
					dValue = pBusiness->m_pSecurity->GetLowPrice(nDate);
					break;
				case 4:		//收盘价
					dValue = pBusiness->m_pSecurity->GetClosePrice(nDate);
					break;
				case 5:		//成交金额
					dValue = pBusiness->m_pSecurity->GetAmount(nDate);
					break;
				case 6:		//成交量
					dValue = pBusiness->m_pSecurity->GetVolume(nDate);
					break;
				case 7:		//涨跌值
					dValue = pBusiness->m_pSecurity->GetClosePrice(nDate) - pBusiness->m_pSecurity->GetPreClosePrice(nDate);
					break;
				case 8:		//涨跌幅
					dValue = (pBusiness->m_pSecurity->GetClosePrice(nDate)/pBusiness->m_pSecurity->GetPreClosePrice(nDate) - 1) * 100;
					break;
				default:	
					break;
				}
			}
		}else
		{
			nTradeDateIdx = pBusiness->m_pSecurity->GetTradeDateIndex(nDate);
			if( nTradeDateIdx < 0 )
			{
				nTradeDate = nDate;
				dValue = 0.0;
				pTradeData = pBusiness->m_pSecurity->GetTradeDataByNatureDate(nTradeDate,tag_DI_End);
				if(pTradeData != NULL)
				{
					//开盘价//前收价//最高价//最低价//收盘价
					if(iType == 0 || iType == 1 ||iType == 2 || iType == 3 || iType == 4)
						dValue = (DOUBLE)pTradeData->Close;
					else if(iType == 5)//成交金额
						dValue = (DOUBLE)pTradeData->Amount;
					else if(iType == 6)//成交量
						dValue = (DOUBLE)pTradeData->Volume;
				}
			}
			else
			{
				dValue = 0.0;
				nTradeDate = pBusiness->m_pSecurity->GetTradeDateByIndex(nTradeDateIdx);
				pTradeData = pBusiness->m_pSecurity->GetTradeDataByNatureDate(nTradeDate,tag_DI_End);
				if(pTradeData != NULL)
				{
					if ( nDate != nTradeDate )
					{
						/* 20100629 wanglm 成交量，成交金额，换手率当为非交易日时不去最近交易日的数据，而是返回“-” */
						//开盘价//前收价//最高价//最低价//收盘价
						if(iType == 0 || iType == 1 ||iType == 2 || iType == 3 || iType == 4)
							dValue = (DOUBLE)pTradeData->Close;
						else if(iType == 5)//成交金额
							//dValue = (DOUBLE)pTradeData->Amount;
							 dValue = 0; 
						else if(iType == 6)//成交量
							// dValue = (DOUBLE)pTradeData->Volume;
						    dValue = 0; 
						// ******************************** wanglm ******************************** //

					}
					else
					{
						switch(iType)
						{
						case 0:		//开盘价
							dValue = (DOUBLE)pTradeData->Open;
							break;
						case 1:		//前收价
							dValue = (DOUBLE)pTradeData->Preclose;
							break;
						case 2:		//最高价
							dValue = (DOUBLE)pTradeData->High;
							break;
						case 3:		//最低价
							dValue = (DOUBLE)pTradeData->Low;
							break;
						case 4:		//收盘价
							dValue = (DOUBLE)pTradeData->Close;
							break;
						case 5:		//成交金额
							dValue = pTradeData->Amount;
							break;
						case 6:		//成交量
							dValue = pTradeData->Volume;
							break;
						case 7:		//涨跌值
							dValue = (DOUBLE)pTradeData->Close - (DOUBLE)pTradeData->Preclose;
							break;
						case 8:		//涨跌幅
							dValue = ((DOUBLE)pTradeData->Close/(DOUBLE)pTradeData->Preclose-1) * 100;
							break;
						default:	
							break;
						}
					}
				}	
			}
		}
	}
/*
			nTradeDate = pBusiness->m_pSecurity->GetTradeDateByIndex(nTradeDateIdx);
			if(nTradeDate < nDate)
			{	//nDate交易所停牌
				switch(iType)
				{
				case 0:		//开盘价
				case 1:		//前收价
				case 2:		//最高价
				case 3:		//最低价
				case 4:		//收盘价
					pTradeData = pBusiness->m_pSecurity->GetTradeDataByNatureDate(nTradeDate,tag_DI_End);
					if(pTradeData == NULL)
					{
						dValue = 0;
					}else
					{
						dValue = (DOUBLE)pTradeData->Close;
					}
					break;
				case 5:		//成交金额
				case 6:		//成交量
				case 7:
				case 8:
					dValue = 0;
					break;
				default:
					break;
				}
			}else
			{	//nDate交易所交易日
				pTradeData = pBusiness->m_pSecurity->GetTradeDataByNatureDate(nTradeDate,tag_DI_End);
				if(pTradeData == NULL)
				{
					return 0;
				}
				switch(iType)
				{
				case 0:		//开盘价
					dValue = (DOUBLE)pTradeData->Open;
					break;
				case 1:		//前收价
					dValue = (DOUBLE)pTradeData->Preclose;
					break;
				case 2:		//最高价
					dValue = (DOUBLE)pTradeData->High;
					break;
				case 3:		//最低价
					dValue = (DOUBLE)pTradeData->Low;
					break;
				case 4:		//收盘价
					dValue = (DOUBLE)pTradeData->Close;
					break;
				case 5:		//成交金额
					dValue = pTradeData->Amount;
					break;
				case 6:		//成交量
					dValue = pTradeData->Volume;
					break;
				case 7:		//涨跌值
					dValue = (DOUBLE)pTradeData->Close - (DOUBLE)pTradeData->Preclose;
					break;
				case 8:		//涨跌幅
					dValue = ((DOUBLE)pTradeData->Close/(DOUBLE)pTradeData->Preclose-1) * 100;
					break;
				default:	
					break;
				}
			}
		}
		*/
	return dValue;
}

DOUBLE TxIndicator::GetKLineExNew(Tx::Business::TxBusiness *pBusiness, int nDate, int iType,int m_iSecId)
{
	int nTradeDateIdx, nTradeDate, nLatestTradeDate, nCurrentDate;

	DOUBLE dValue = 0;
	HisTradeData *pTradeData = NULL;
	Tx::Business::TxBusiness business;
	BOOL bHisHQ = TRUE;		//是否需要下载历史K线行情文件
	CString sLog;
	//作nDate是否超越已停牌股票最后一个交易日的判断
	nLatestTradeDate = pBusiness->m_pSecurity->GetStopedLastDate();
	int nFirstDay = pBusiness->m_pSecurity->GetTradeDateByIndex(0);
	if( (nLatestTradeDate > 0 && nDate > nLatestTradeDate) || nDate < nFirstDay )
		return 0;
	CTime tm = CTime::GetCurrentTime();
	nCurrentDate = tm.GetYear() * 10000 + tm.GetMonth() * 100 + tm.GetDay();
	//通过市场判断当日是否为交易日
	if(pBusiness->m_pSecurity->IsShanghai() || pBusiness->m_pSecurity->IsTianxiang())
	{	//取得上证指数交易日期序列
		bHisHQ = FALSE;
		nLatestTradeDate = pBusiness->m_pSecurity->GetTradeDateLatest();
		if(nCurrentDate != nDate )
		{
			pTradeData = pBusiness->m_pShIndex->GetTradeDataByNatureDate(nDate);
			if(pTradeData != NULL)
				nDate = pTradeData->Date;
		}
		
	}else if(pBusiness->m_pSecurity->IsShenzhen())
	{	//取得深证成指交易日期序列
		bHisHQ = FALSE;
		nLatestTradeDate = pBusiness->m_pSecurity->GetTradeDateLatest();
		if(nCurrentDate != nDate )
		{
			pTradeData = pBusiness->m_pSzIndex->GetTradeDataByNatureDate(nDate);
			if(pTradeData != NULL)
				nDate = pTradeData->Date;
		}
		
	}else if ( pBusiness->m_pSecurity->IsIndexCustomByCode())
	{
		int nCount = pBusiness->m_pSecurity->GetTradeDataCount();
		if ( nCount > 0 )
		{
			bHisHQ = TRUE;
			HisTradeData*	pTrade = pBusiness->m_pSecurity->GetTradeDataByIndex( nCount-1 );
			if ( pTrade != NULL)
				nLatestTradeDate = pTrade->Date;
		}
	}
	else
	{	//港股通过恒生指数判断4000868
		business.GetSecurityNow(4000868);
		nLatestTradeDate = pBusiness->m_pSecurity->GetTradeDateLatest();
		if(nCurrentDate != nDate )
		{
			pTradeData = business.m_pSecurity->GetTradeDataByNatureDate(nDate);		
			if(pTradeData != NULL)
				nDate = pTradeData->Date;
		}
		
	}

	if(nDate == nCurrentDate)
	{
		if(pBusiness->m_pSecurity->IsValid())
		{	//今日是交易日
			switch(iType)
			{
			case 0:		//开盘价
				dValue = pBusiness->m_pSecurity->GetOpenPrice();
				break;
			case 1:		//前收价
				dValue = pBusiness->m_pSecurity->GetPreClosePrice();
				break;
			case 2:		//最高价
				dValue = pBusiness->m_pSecurity->GetHighPrice();
				break;
			case 3:		//最低价
				dValue = pBusiness->m_pSecurity->GetLowPrice();
				break;
			case 4:		//收盘价
				dValue = pBusiness->m_pSecurity->GetClosePrice();
				break;
			case 5:		//成交金额
				dValue = pBusiness->m_pSecurity->GetAmount();
				break;
			case 6:		//成交量
				dValue = pBusiness->m_pSecurity->GetVolume();
				break;
			case 7:		//涨跌值
				dValue = pBusiness->m_pSecurity->GetClosePrice() - pBusiness->m_pSecurity->GetPreClosePrice();
				break;
			case 8:		//涨跌幅
				dValue = (pBusiness->m_pSecurity->GetClosePrice()/pBusiness->m_pSecurity->GetPreClosePrice() - 1) * 100;
				break;
			case 9:    //均价
				{
					if(pBusiness->m_pSecurity->IsRepurchase())
					{
						Tx::Data::RepurchaseAvgPrice	repoData;
						pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_RepurchaseAvgPrice, nDate, repoData );
						dValue = repoData.dAvgPrice;
						if(dValue < 0.01)
							dValue =pBusiness->m_pSecurity->GetPreClosePrice();
					}
					else
					{
						if(pBusiness->m_pSecurity->GetAmount() > 0.1 && pBusiness->m_pSecurity->GetVolume() > 0.1 )
							dValue = pBusiness->m_pSecurity->GetAmount() / (DOUBLE)pBusiness->m_pSecurity->GetVolume();
					}					
				}
				break;
			default:	
				break;
			}
		}else
		{	//今日停牌
			if(pBusiness->m_pSecurity->IsStop())
			{	//摘牌
				return 0;
			}
			pTradeData = pBusiness->m_pSecurity->GetTradeDataByNatureDate(nDate);
			if(pTradeData != NULL)
			{
				switch(iType)
				{
				case 0:		//开盘价
					dValue = (DOUBLE)pTradeData->Open;
					break;
				case 1:		//前收价
					dValue = (DOUBLE)pTradeData->Preclose;
					break;
				case 2:		//最高价
					dValue = (DOUBLE)pTradeData->High;
					break;
				case 3:		//最低价
					dValue = (DOUBLE)pTradeData->Low;
					break;
				case 4:		//收盘价
					dValue = (DOUBLE)pTradeData->Close;
					break;
				case 5:		//成交金额
					dValue = pTradeData->Amount;
					break;
				case 6:		//成交量
					dValue = pTradeData->Volume;
					break;
				case 7:		//涨跌值
					dValue = (DOUBLE)pTradeData->Close - (DOUBLE)pTradeData->Preclose;
					break;
				case 8:		//涨跌幅
					dValue = ((DOUBLE)pTradeData->Close/(DOUBLE)pTradeData->Preclose-1) * 100;
					break;
				case 9:
					{
						if(pBusiness->m_pSecurity->IsRepurchase())
						{
							Tx::Data::RepurchaseAvgPrice	repoData;
							pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_RepurchaseAvgPrice, pTradeData->Date, repoData );
							dValue = repoData.dAvgPrice;
							if(dValue < 0.01)
								dValue =pBusiness->m_pSecurity->GetPreClosePrice();
						}
						else
						{
							if(pTradeData->Amount > 0.1 && pTradeData->Volume > 0.1)
								dValue = pTradeData->Amount / (DOUBLE)pTradeData->Volume;
						}						
					}					
					break;
				default:
					break;
				}
			}			
		}
		return dValue;
	}

	if(nLatestTradeDate <= 0)
	{
		return 0;
	}
	//通过市场日期获得交易实体的日期
	pTradeData = NULL;
	pTradeData = pBusiness->m_pSecurity->GetTradeDataByNatureDate( nDate);
	if(pTradeData != NULL)
	{
		switch(iType)
		{
		case 0:		//开盘价
			dValue = (DOUBLE)pTradeData->Open;
			break;
		case 1:		//前收价
			dValue = (DOUBLE)pTradeData->Preclose;
			break;
		case 2:		//最高价
			dValue = (DOUBLE)pTradeData->High;
			break;
		case 3:		//最低价
			dValue = (DOUBLE)pTradeData->Low;
			break;
		case 4:		//收盘价
			dValue = (DOUBLE)pTradeData->Close;
			break;
		case 5:		//成交金额
			dValue = pTradeData->Amount;
			break;
		case 6:		//成交量
			dValue = pTradeData->Volume;
			break;
		case 7:		//涨跌值
			dValue = (DOUBLE)pTradeData->Close - (DOUBLE)pTradeData->Preclose;
			break;
		case 8:		//涨跌幅
			dValue = ((DOUBLE)pTradeData->Close/(DOUBLE)pTradeData->Preclose-1) * 100;
			break;
		case 9:
			{
				if(pBusiness->m_pSecurity->IsRepurchase())
				{
					Tx::Data::RepurchaseAvgPrice	repoData;
					pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_RepurchaseAvgPrice, pTradeData->Date, repoData );
					dValue = repoData.dAvgPrice;
					if(dValue < 0.01)
						dValue =pBusiness->m_pSecurity->GetPreClosePrice();
				}
				else
				{
					if(pTradeData->Amount > 0.1 && pTradeData->Volume > 0.1)
						dValue = pTradeData->Amount / (DOUBLE)pTradeData->Volume;
				}
			}					
			break;
		default:
			break;
		}
	}
	
	return dValue;
}

int TxIndicator::GetTradeDateByOffset(Tx::Business::TxBusiness *pBusiness, int nDate, int nOffsetDays)
{
	int nStartDateIdx, nEndDateIdx, nOffsetDateIdx, nTradeDate, nStartDate, nCurDate;
	CTime tmCurrent = CTime::GetCurrentTime();

	if(pBusiness->m_pSecurity->GetTradeDateCount() == 0)
	{
		return 0;
	}

	nStartDateIdx = pBusiness->m_pSecurity->GetTradeDateIndex(nDate);
	if(nStartDateIdx < 0)
	{	//基准日在最后一个历史交易日之后
		nCurDate = tmCurrent.GetYear() * 10000 + tmCurrent.GetMonth() * 100 + tmCurrent.GetDay();
		if(nDate == nCurDate)
		{	//基准日为当前日
			if(pBusiness->m_pSecurity->IsValid())
			{
				if(nOffsetDays < 0)
				{
					nStartDateIdx = pBusiness->m_pSecurity->GetTradeDateCount();
					nOffsetDateIdx = nStartDateIdx + nOffsetDays;
					if(nOffsetDateIdx < 0)
					{
						nOffsetDateIdx = 0;
					}
				}else
				{
					return nCurDate;
				}
			}else
			{
				//取最后一个历史交易日为基准日，Offset自然缩减一天
				nStartDateIdx = pBusiness->m_pSecurity->GetTradeDateCount() - 1;
				if(nOffsetDays < 0)
				{
					nOffsetDateIdx = nStartDateIdx + nOffsetDays + 1;
					if(nOffsetDateIdx < 0)
					{
						nOffsetDateIdx = 0;
					}
				}else
				{
					nOffsetDateIdx = nStartDateIdx;
				}
			}
		}else
		{
			//取最后一个历史交易日为基准日，Offset自然缩减一天
			nStartDateIdx = pBusiness->m_pSecurity->GetTradeDateCount() - 1;
			if(nOffsetDays < 0)
			{
				nOffsetDateIdx = nStartDateIdx + nOffsetDays + 1;
				if(nOffsetDateIdx < 0)
				{
					nOffsetDateIdx = 0;
				}
			}else
			{
				nOffsetDateIdx = nStartDateIdx;
			}
		}
	}else
	{
		nStartDate = pBusiness->m_pSecurity->GetTradeDateByIndex((LONG)nStartDateIdx);
		if(nOffsetDays <= 0)
		{	//向前偏移
			if(nStartDate < nDate)
			{
				nStartDateIdx ++;
			}
		}
		nOffsetDateIdx = nStartDateIdx + nOffsetDays;
		if(nOffsetDateIdx < 0)
		{
			nOffsetDateIdx = 0;
		}
	}

	nEndDateIdx = pBusiness->m_pSecurity->GetTradeDateCount() - 1;

	if(nOffsetDateIdx > nEndDateIdx)
	{	//nDate为当前交易日或者nDate + nOffsetDays > nTradeDateLatest
		if(pBusiness->m_pSecurity->IsValid())
		{
			nTradeDate = tmCurrent.GetYear() * 10000 + tmCurrent.GetMonth() * 100 + tmCurrent.GetDay();
		}else
		{
			if(nEndDateIdx < 0)
			{
				return 0;
			}else
			{
				nTradeDate = pBusiness->m_pSecurity->GetTradeDateByIndex((LONG)nEndDateIdx);
			}
		}
		return nTradeDate;
	}

	nTradeDate = pBusiness->m_pSecurity->GetTradeDateByIndex((LONG)nOffsetDateIdx);
	return nTradeDate;
}

int TxIndicator::GetTradeDays(Tx::Business::TxBusiness *pBusiness, int nStartDate, int nEndDate)
{
	int nStartDateIdx, nEndDateIdx, nStartTradeDate, nEndTradeDate, nCurDate, nTradeDays = 0;
	CTime tmCurrent = CTime::GetCurrentTime();
	DAY_HQ_ITEM *pDayHQItem;
	DAY_HQ_ITEM DayHQItem_Start, DayHQItem_End;
	Tx::Business::TxBusiness business;
	BOOL bHisHQ = TRUE;		//是否需要下载历史K线行情文件

	//作nEndDate是否超越已停牌股票最后一个交易日的判断
	nEndTradeDate = pBusiness->m_pSecurity->GetStopedLastDate();
	if(nEndTradeDate > 0 && nStartDate > nEndTradeDate)
		return 0;
	if( pBusiness->m_pSecurity->GetTradeDateLatest() < nStartDate )
		return 0;
	if(pBusiness->m_pSecurity->IsShanghai())
	{	//取得上证指数交易日期序列
		business.GetSecurityNow((LONG)iSHIndexID);
		bHisHQ = FALSE;
		nEndTradeDate = business.m_pSecurity->GetTradeDateLatest();
	}else if(pBusiness->m_pSecurity->IsShenzhen())
	{	//取得深证成指交易日期序列
		business.GetSecurityNow((LONG)iSZIndexID);
		bHisHQ = FALSE;
		nEndTradeDate = business.m_pSecurity->GetTradeDateLatest();
	}

	nCurDate = tmCurrent.GetYear() * 10000 + tmCurrent.GetMonth() * 100 + tmCurrent.GetDay();
	if(!bHisHQ)
	{
		if(nStartDate > nEndTradeDate)
		{
			if(nStartDate <= nCurDate && nEndDate >= nCurDate)
			{
				if(pBusiness->m_pSecurity->IsValid())
				{
					return 1;
				}else
				{
					return 0;
				}
			}else
			{
				return 0;
			}
		}
	}

	if(bHisHQ)
	{
		if(pBusiness->m_pSecurity->GetTradeDateCount() == 0)
		{
			return 0;
		}

		nStartDateIdx = pBusiness->m_pSecurity->GetTradeDateIndex(nStartDate);
		if(nStartDateIdx < 0)
		{
			if(pBusiness->m_pSecurity->IsStop())
			{	//摘牌
				return 0;
			}

			if(nStartDate <= nCurDate && nEndDate >= nCurDate)
			{
				if(pBusiness->m_pSecurity->IsValid())
				{
					return 1;
				}else
				{
					return 0;
				}
			}else
			{
				return 0;
			}
		}

		nStartTradeDate = pBusiness->m_pSecurity->GetTradeDateByIndex((LONG)nStartDateIdx);
		if(nStartTradeDate < nStartDate)
		{
			nStartDateIdx ++;
		}
		if(nStartDateIdx == pBusiness->m_pSecurity->GetTradeDateCount())
		{
			nStartDateIdx --;
		}

		nEndTradeDate = pBusiness->m_pSecurity->GetTradeDateLatest();
		if(nEndTradeDate >= nEndDate)
		{
			nEndDateIdx = pBusiness->m_pSecurity->GetTradeDateIndex(nEndDate);
			nTradeDays = nEndDateIdx - nStartDateIdx + 1;
		}else
		{
			nEndDateIdx = pBusiness->m_pSecurity->GetTradeDateCount() - 1;
			if(pBusiness->m_pSecurity->IsValid() && nEndDate >= nCurDate)
			{
				nTradeDays = nEndDateIdx - nStartDateIdx + 2;
			}else
			{
				nTradeDays = nEndDateIdx - nStartDateIdx + 1;
			}
		}
	}else
	{
		nTradeDays = 0;
		//起始日期
		nStartTradeDate = pBusiness->m_pSecurity->GetIPOListedDate();
		if(nStartDate > nStartTradeDate)
		{
			nStartDateIdx = business.m_pSecurity->GetTradeDateIndex(nStartDate);
			nStartTradeDate = business.m_pSecurity->GetTradeDateByIndex(nStartDateIdx);
			if(nStartTradeDate < nStartDate)
			{
				nStartDateIdx ++ ;
			}
			if(nStartDateIdx == business.m_pSecurity->GetTradeDateCount())
			{
				nStartDateIdx --;
			}
			nStartTradeDate = business.m_pSecurity->GetTradeDateByIndex(nStartDateIdx);
		}
		//终止日期
		if(pBusiness->m_pSecurity->IsStop())
		{
			nEndTradeDate = pBusiness->m_pSecurity->GetStopedLastDate();
		}

		{
			//刘鹏，2011-10-26.fix bug 9053
			if(nEndTradeDate > nEndDate)
			{
				nEndDateIdx = business.m_pSecurity->GetTradeDateIndex(nEndDate);
			}else
			{
				nEndDateIdx = business.m_pSecurity->GetTradeDateCount() - 1;
				//刘鹏，2011-10-26.fix bug 9053
				if(pBusiness->m_pSecurity->IsValid() && nEndTradeDate == nCurDate)
				{
					nTradeDays = 1;
				}
			}
			nEndTradeDate = business.m_pSecurity->GetTradeDateByIndex(nEndDateIdx);
			//刘鹏，2011-10-26.fix bug 9053
			if(nEndTradeDate == nCurDate)
			{
				nEndTradeDate = business.m_pSecurity->GetTradeDateOffset(nEndTradeDate,-1);
				if(pBusiness->m_pSecurity->IsValid())
					nTradeDays = 1;
				else
					nTradeDays = 0;
			}
		}
		pDayHQItem = pBusiness->m_pSecurity->GetOneDayTradeData(nStartTradeDate);
		if(pDayHQItem != NULL)
		{
			memcpy(&DayHQItem_Start, pDayHQItem, sizeof(DAY_HQ_ITEM));
		}else
		{
			return nTradeDays;
		}
		pDayHQItem = pBusiness->m_pSecurity->GetOneDayTradeData(nEndTradeDate);
		if(pDayHQItem != NULL)
		{
			memcpy(&DayHQItem_End, pDayHQItem, sizeof(DAY_HQ_ITEM));
			nTradeDays += DayHQItem_End.lSumTradeDays - DayHQItem_Start.lSumTradeDays + 1;
		}
	}

	return nTradeDays;
}

DOUBLE TxIndicator::GetSumVolume(Tx::Business::TxBusiness *pBusiness, int nStartDate, int nEndDate)
{
	int i, nStartDateIdx, nEndDateIdx, nStartTradeDate, nEndTradeDate, nDate, nCurDate;
	BOOL bCurDayInPeriod = FALSE;
	DOUBLE dValue = 0, dValue1 = 0;
	CTime tmCurrent = CTime::GetCurrentTime();
	DAY_HQ_ITEM *pDayHQItem;
	DAY_HQ_ITEM DayHQItem_Start, DayHQItem_End;
	Tx::Business::TxBusiness business;
	BOOL bHisHQ = TRUE;		//是否需要下载历史K线行情文件

	//作nEndDate是否超越已停牌股票最后一个交易日的判断
	nEndTradeDate = pBusiness->m_pSecurity->GetStopedLastDate();
	if(nEndTradeDate > 0 && nStartDate > nEndTradeDate)
	{
		return 0;
	}
	
	if( pBusiness->m_pSecurity->IsIndexCustomByCode())
	{
		bHisHQ = TRUE;
		int nCount = pBusiness->m_pSecurity->GetTradeDataCount();
		if ( nCount > 0 )
		{
			HisTradeData* pTrade = pBusiness->m_pSecurity->GetTradeDataByIndex( nCount -1 );
			if ( pTrade != NULL)
				nEndTradeDate = pTrade->Date;
		}
	}else if(pBusiness->m_pSecurity->IsShanghai())
	{	//取得上证指数交易日期序列
		business.GetSecurityNow((LONG)iSHIndexID);
		bHisHQ = FALSE;
		nEndTradeDate = business.m_pSecurity->GetTradeDateLatest();
	}else if(pBusiness->m_pSecurity->IsShenzhen())
	{	//取得深证成指交易日期序列
		business.GetSecurityNow((LONG)iSZIndexID);
		bHisHQ = FALSE;
		nEndTradeDate = business.m_pSecurity->GetTradeDateLatest();
	}

	nCurDate = tmCurrent.GetYear() * 10000 + tmCurrent.GetMonth() * 100 + tmCurrent.GetDay();
	if(!bHisHQ)
	{
		if(nStartDate > nEndTradeDate)
		{
			if(nStartDate <= nCurDate && nEndDate >= nCurDate)
			{
				if(pBusiness->m_pSecurity->IsValid())
				{
					dValue = pBusiness->m_pSecurity->GetVolume();
					return dValue;
				}else
				{
					return 0;
				}
			}else
			{
				return 0;
			}
		}
	}

	if(bHisHQ)
	{
		if(pBusiness->m_pSecurity->GetTradeDateCount() == 0)
		{
			return 0;
		}

		nStartDateIdx = pBusiness->m_pSecurity->GetTradeDateIndex(nStartDate);
		if(nStartDateIdx < 0)
		{
			if(pBusiness->m_pSecurity->IsStop())
			{	//摘牌
				return 0;
			}

			if(nStartDate <= nCurDate && nEndDate >= nCurDate)
			{
				if(pBusiness->m_pSecurity->IsValid())
				{
					dValue = pBusiness->m_pSecurity->GetVolume();
					return dValue;
				}else
				{
					return 0;
				}
			}else
			{
				return 0;
			}
		}

		nStartTradeDate = pBusiness->m_pSecurity->GetTradeDateByIndex((LONG)nStartDateIdx);
		if(nStartTradeDate < nStartDate)
		{
			nStartDateIdx ++;
		}
		if(nStartDateIdx == pBusiness->m_pSecurity->GetTradeDateCount())
		{
			nStartDateIdx --;
		}

		nEndTradeDate = pBusiness->m_pSecurity->GetTradeDateLatest();
		if(nEndTradeDate >= nEndDate)
		{
			nEndDateIdx = pBusiness->m_pSecurity->GetTradeDateIndex(nEndDate);
		}else
		{
			nEndDateIdx = pBusiness->m_pSecurity->GetTradeDateCount() - 1;
			if(pBusiness->m_pSecurity->IsValid() && nEndDate >= nCurDate)
			{
				bCurDayInPeriod = TRUE;
			}
		}

		for(i = nStartDateIdx; i <= nEndDateIdx; i++)
		{
			nDate = pBusiness->m_pSecurity->GetTradeDateByIndex((LONG)i);
			dValue1 = pBusiness->m_pSecurity->GetVolume(nDate);
			if(dValue1 > 0)
			{
				dValue += dValue1;
			}
		}

		if(bCurDayInPeriod)
		{
			dValue1 = pBusiness->m_pSecurity->GetVolume();
			if(dValue1 > 0)
			{
				dValue += dValue1;
			}
		}
	}else
	{
		dValue = 0;
		//起始日期
		nStartTradeDate = pBusiness->m_pSecurity->GetIPOListedDate();
		if(nStartDate > nStartTradeDate)
		{
			nStartDateIdx = business.m_pSecurity->GetTradeDateIndex(nStartDate);
			nStartTradeDate = business.m_pSecurity->GetTradeDateByIndex(nStartDateIdx);
			if(nStartTradeDate < nStartDate)
			{
				nStartDateIdx ++ ;
			}
			if(nStartDateIdx == business.m_pSecurity->GetTradeDateCount())
			{
				nStartDateIdx --;
			}
			nStartTradeDate = business.m_pSecurity->GetTradeDateByIndex(nStartDateIdx);
		}
		//终止日期
		if(pBusiness->m_pSecurity->IsStop())
		{
			nEndTradeDate = pBusiness->m_pSecurity->GetStopedLastDate();
		}else
		{
			if(nEndTradeDate >= nEndDate)
			{
				nEndDateIdx = business.m_pSecurity->GetTradeDateIndex(nEndDate);
			}else
			{
				nEndDateIdx = business.m_pSecurity->GetTradeDateCount() - 1;
				if(pBusiness->m_pSecurity->IsValid() && nEndDate >= nCurDate)
				{
					dValue = pBusiness->m_pSecurity->GetVolume();;
				}
			}
			nEndTradeDate = business.m_pSecurity->GetTradeDateByIndex(nEndDateIdx);
		}
		pDayHQItem = pBusiness->m_pSecurity->GetOneDayTradeData(nStartTradeDate);
		if(pDayHQItem != NULL)
		{
			memcpy(&DayHQItem_Start, pDayHQItem, sizeof(DAY_HQ_ITEM));
		}else
		{
			return dValue;
		}
		pDayHQItem = pBusiness->m_pSecurity->GetOneDayTradeData(nEndTradeDate);
		if(pDayHQItem != NULL)
		{
			memcpy(&DayHQItem_End, pDayHQItem, sizeof(DAY_HQ_ITEM));
			dValue += DayHQItem_End.SumVolume - DayHQItem_Start.SumVolume + DayHQItem_Start.Volume;
		}
	}

	return dValue;
}

DOUBLE TxIndicator::GetSumAmount(Tx::Business::TxBusiness *pBusiness, int nStartDate, int nEndDate)
{
	int i, nStartDateIdx, nEndDateIdx, nStartTradeDate, nEndTradeDate, nDate, nCurDate;
	BOOL bCurDayInPeriod = FALSE;
	DOUBLE dValue = 0, dValue1 = 0;
	CTime tmCurrent = CTime::GetCurrentTime();
	DAY_HQ_ITEM *pDayHQItem;
	DAY_HQ_ITEM DayHQItem_Start, DayHQItem_End;
	Tx::Business::TxBusiness business;
	BOOL bHisHQ = TRUE;		//是否需要下载历史K线行情文件

	//作nEndDate是否超越已停牌股票最后一个交易日的判断
	nEndTradeDate = pBusiness->m_pSecurity->GetStopedLastDate();
	if(nEndTradeDate > 0 && nStartDate > nEndTradeDate)
	{
		return 0;
	}

	if(pBusiness->m_pSecurity->IsShanghai())
	{	//取得上证指数交易日期序列
		business.GetSecurityNow((LONG)iSHIndexID);
		bHisHQ = FALSE;
		nEndTradeDate = business.m_pSecurity->GetTradeDateLatest();
	}else if(pBusiness->m_pSecurity->IsShenzhen())
	{	//取得深证成指交易日期序列
		business.GetSecurityNow((LONG)iSZIndexID);
		bHisHQ = FALSE;
		nEndTradeDate = business.m_pSecurity->GetTradeDateLatest();
	}else if( pBusiness->m_pSecurity->IsIndexCustomByCode())
	{
		bHisHQ = TRUE;
		int nCount = pBusiness->m_pSecurity->GetTradeDataCount();
		if ( nCount > 0 )
		{
			HisTradeData* pTrade = pBusiness->m_pSecurity->GetTradeDataByIndex( nCount -1 );
			if ( pTrade != NULL)
				nEndTradeDate = pTrade->Date;
		}
	}

	nCurDate = tmCurrent.GetYear() * 10000 + tmCurrent.GetMonth() * 100 + tmCurrent.GetDay();
	if(!bHisHQ)
	{
		if(nStartDate > nEndTradeDate)
		{
			if(nStartDate <= nCurDate && nEndDate >= nCurDate)
			{
				if(pBusiness->m_pSecurity->IsValid())
				{
					dValue = pBusiness->m_pSecurity->GetAmount();
					return dValue;
				}else
				{
					return 0;
				}
			}else
			{
				return 0;
			}
		}
	}

	if(bHisHQ)
	{
		if(pBusiness->m_pSecurity->GetTradeDateCount() == 0)
		{
			return 0;
		}

		nStartDateIdx = pBusiness->m_pSecurity->GetTradeDateIndex(nStartDate);
		if(nStartDateIdx < 0)
		{
			if(pBusiness->m_pSecurity->IsStop())
			{	//摘牌
				return 0;
			}

			if(nStartDate <= nCurDate && nEndDate >= nCurDate)
			{
				if(pBusiness->m_pSecurity->IsValid())
				{
					dValue = pBusiness->m_pSecurity->GetAmount();
					return dValue;
				}else
				{
					return 0;
				}
			}else
			{
				return 0;
			}
		}

		nStartTradeDate = pBusiness->m_pSecurity->GetTradeDateByIndex((LONG)nStartDateIdx);
		if(nStartTradeDate < nStartDate)
		{
			nStartDateIdx ++;
		}
		if(nStartDateIdx == pBusiness->m_pSecurity->GetTradeDateCount())
		{
			nStartDateIdx --;
		}

		nEndTradeDate = pBusiness->m_pSecurity->GetTradeDateLatest();
		if(nEndTradeDate >= nEndDate)
		{
			nEndDateIdx = pBusiness->m_pSecurity->GetTradeDateIndex(nEndDate);
		}else
		{
			nEndDateIdx = pBusiness->m_pSecurity->GetTradeDateCount() - 1;
			if(pBusiness->m_pSecurity->IsValid() && nEndDate >= nCurDate)
			{
				bCurDayInPeriod = TRUE;
			}
		}

		for(i = nStartDateIdx; i <= nEndDateIdx; i++)
		{
			nDate = pBusiness->m_pSecurity->GetTradeDateByIndex((LONG)i);
			dValue1 = pBusiness->m_pSecurity->GetAmount(nDate);
			if(dValue1 > 0)
			{
				dValue += dValue1;
			}
		}

		if(bCurDayInPeriod)
		{
			dValue1 = pBusiness->m_pSecurity->GetAmount();
			if(dValue1 > 0)
			{
				dValue += dValue1;
			}
		}
	}else
	{
		dValue = 0;
		//起始日期
		nStartTradeDate = pBusiness->m_pSecurity->GetIPOListedDate();
		if(nStartDate > nStartTradeDate)
		{
			nStartDateIdx = business.m_pSecurity->GetTradeDateIndex(nStartDate);
			nStartTradeDate = business.m_pSecurity->GetTradeDateByIndex(nStartDateIdx);
			if(nStartTradeDate < nStartDate)
			{
				nStartDateIdx ++ ;
			}
			if(nStartDateIdx == business.m_pSecurity->GetTradeDateCount())
			{
				nStartDateIdx --;
			}
			nStartTradeDate = business.m_pSecurity->GetTradeDateByIndex(nStartDateIdx);
		}
		//终止日期
		if(pBusiness->m_pSecurity->IsStop())
		{
			nEndTradeDate = pBusiness->m_pSecurity->GetStopedLastDate();
		}else
		{
			if(nEndTradeDate >= nEndDate)
			{
				nEndDateIdx = business.m_pSecurity->GetTradeDateIndex(nEndDate);
			}else
			{
				nEndDateIdx = business.m_pSecurity->GetTradeDateCount() - 1;
				if(pBusiness->m_pSecurity->IsValid() && nEndDate >= nCurDate)
				{
					dValue = pBusiness->m_pSecurity->GetAmount();
				}
			}
			nEndTradeDate = business.m_pSecurity->GetTradeDateByIndex(nEndDateIdx);
		}
		pDayHQItem = pBusiness->m_pSecurity->GetOneDayTradeData(nStartTradeDate);
		if(pDayHQItem != NULL)
		{
			memcpy(&DayHQItem_Start, pDayHQItem, sizeof(DAY_HQ_ITEM));
		}else
		{
			return dValue;
		}
		pDayHQItem = pBusiness->m_pSecurity->GetOneDayTradeData(nEndTradeDate);
		if(pDayHQItem != NULL)
		{
			memcpy(&DayHQItem_End, pDayHQItem, sizeof(DAY_HQ_ITEM));
			dValue += DayHQItem_End.SumAmount - DayHQItem_Start.SumAmount + DayHQItem_Start.Amount;
		}
	}

	return dValue;
}

DOUBLE TxIndicator::GetSumExchangeRatio(Tx::Business::TxBusiness *pBusiness, int nStartDate, int nEndDate)
{
	Tx::Data::TxShareData *pTxShareData;
	Tx::Data::TxFundShareData *pTxFundShareData;
	Tx::Data::BondNewInfo *pBondNewInfo;
	Tx::Data::BondNotChangeAmount *pCBondAmount;
	int i, j, nStartDateIdx, nEndDateIdx, nStartTradeDate, nEndTradeDate, nDate, nCurDate;
	BOOL bCurDayInPeriod = FALSE;
	DOUBLE dValue = 0, dValue1 = 0, dValue2 = 0;
	CTime tmCurrent = CTime::GetCurrentTime();

	if(pBusiness->m_pSecurity->GetTradeDateCount() == 0)
	{
		return 0;
	}

	nStartDateIdx = pBusiness->m_pSecurity->GetTradeDateIndex(nStartDate);
	if(nStartDateIdx < 0)
	{
		if(pBusiness->m_pSecurity->IsStop())
		{	//摘牌
			return 0;
		}

		nCurDate = tmCurrent.GetYear() * 10000 + tmCurrent.GetMonth() * 100 + tmCurrent.GetDay();	
		if(nStartDate <= nCurDate && nEndDate >= nCurDate)
		{
			if(pBusiness->m_pSecurity->IsValid())
			{
				dValue1 = pBusiness->m_pSecurity->GetVolume();
				dValue2 = 0;
				if(pBusiness->m_pSecurity->IsStock())
				{	//股票
					dValue2 = pBusiness->m_pSecurity->GetTradableShare();
				}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
				{	//基金
					dValue2 = pBusiness->m_pSecurity->GetTradableShare();
				}else if(pBusiness->m_pSecurity->IsBond_Change())
				{	//可转债
					pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
					j = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
					if(j > 0)
					{	//最近一次未转换债券金额
						pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(j - 1);
						dValue2 = pCBondAmount->not_change_bond_amount / 100;
					}else
					{	//可转债发行量
						dValue2 = pBondNewInfo->share / 100 ;
					}
				}
				if(dValue1 > 0 && dValue2 > 0)
				{
					return dValue1 / dValue2;
				}
			}else
			{
				return 0;
			}
		}else
		{
			return 0;
		}
	}

	nStartTradeDate = pBusiness->m_pSecurity->GetTradeDateByIndex((LONG)nStartDateIdx);
	if(nStartTradeDate < nStartDate)
	{
		nStartDateIdx ++;
	}
	if(nStartDateIdx == pBusiness->m_pSecurity->GetTradeDateCount())
	{
		nStartDateIdx --;
	}

	nEndTradeDate = pBusiness->m_pSecurity->GetTradeDateLatest();
	if ( nStartTradeDate > nEndDate )
	{
		return 0;
	}
	if(nEndTradeDate >= nEndDate)
	{
		nEndDateIdx = pBusiness->m_pSecurity->GetTradeDateIndex(nEndDate);
	}else
	{
		nEndDateIdx = pBusiness->m_pSecurity->GetTradeDateCount() - 1;
		if(pBusiness->m_pSecurity->IsValid())
		{
			bCurDayInPeriod = TRUE;
		}
	}

	for(i = nStartDateIdx; i <= nEndDateIdx; i++)
	{
		nDate = pBusiness->m_pSecurity->GetTradeDateByIndex((LONG)i);
		dValue1 = pBusiness->m_pSecurity->GetVolume(nDate);
		dValue2 = 0;
		if(pBusiness->m_pSecurity->IsStock())
		{	//股票
			pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(nDate);
			if(pTxShareData != NULL)
			{
				dValue2 = pTxShareData->TradeableShare;
			}
		}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
		{	//基金
			pTxFundShareData = pBusiness->m_pSecurity->GetTxFundShareDataByDate(nDate);
			if(pTxFundShareData != NULL)
			{
				dValue2 = pTxFundShareData->TradeableShare;
			}
		}else if(pBusiness->m_pSecurity->IsBond_Change())
		{	//可转债
			pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
			j = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
			if(j > 0)
			{
				pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex();
				if(nDate < pCBondAmount->end_date)
				{	//可转债发行量
					dValue2 = pBondNewInfo->share / 100;
				}else
				{	//未转换债券金额
					pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(j - 1);
					if(nDate >= pCBondAmount->end_date)
					{
						dValue2 = pCBondAmount->not_change_bond_amount / 100;
					}else
					{
						pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByDate(nDate);
						dValue2 = pCBondAmount->not_change_bond_amount / 100;
					}
				}
			}else
			{	//可转债发行量
				dValue2 = pBondNewInfo->share / 100 ;
			}
		}

		if(dValue1 > 0 && dValue2 > 0)
		{
			dValue += dValue1 / dValue2;
		}
	}

	if(bCurDayInPeriod)
	{
		dValue1 = pBusiness->m_pSecurity->GetVolume();
		dValue2 = 0;
		if(pBusiness->m_pSecurity->IsStock())
		{	//股票
			dValue2 = pBusiness->m_pSecurity->GetTradableShare();
		}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
		{	//基金
			dValue2 = pBusiness->m_pSecurity->GetTradableShare();
		}else if(pBusiness->m_pSecurity->IsBond_Change())
		{	//可转债
			pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
			j = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
			if(j > 0)
			{	//最近一次未转换债券金额
				pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(j - 1);
				dValue2 = pCBondAmount->not_change_bond_amount / 100;
			}else
			{	//可转债发行量
				dValue2 = pBondNewInfo->share / 100 ;
			}
		}
		if(dValue1 > 0 && dValue2 > 0)
		{
			dValue += dValue1 / dValue2;
		}
	}

	return dValue;
}

DOUBLE TxIndicator::GetMMFGrowth(Tx::Business::TxBusiness *pBusiness, int nStartDate, int nEndDate)
{
	Tx::Data::FundGainData *pFundGainData;
	int i, nStartDateIdx, nEndDateIdx;
	DOUBLE dValue = 0;

	nStartDateIdx = pBusiness->m_pSecurity->GetFundGainDataIndexByDate(nStartDate);
	nEndDateIdx = pBusiness->m_pSecurity->GetFundGainDataIndexByDate(nEndDate);

	if(nStartDateIdx >= 0 && nEndDateIdx >= 0)
	{
		if(nStartDateIdx > 0)
		{
			nStartDateIdx -- ;
		}
		if(nEndDateIdx > 0)
		{
			nEndDateIdx -- ;
		}
		dValue = 1;
		for(i = nStartDateIdx; i <= nEndDateIdx; i++)
		{
			pFundGainData = pBusiness->m_pSecurity->GetFundGainDataByIndex(i);
			dValue = dValue * (1 + pFundGainData->gain_ten_thousand_share / 10000 / 1);
		}
		dValue = (dValue - 1);
	}

	return dValue;
}

int TxIndicator::Get_Value_Occur_Date(Tx::Business::TxBusiness *pBusiness, DOUBLE dFindValue, int iValueType, int nStartDate, int nEndDate)
{
	int i, j, nStartDateIdx, nEndDateIdx, nStartTradeDate, nEndTradeDate, nDate, nCurDate, nOccurDate = 0;
	CString strValue;
	Tx::Data::TxShareData *pTxShareData;
	Tx::Data::TxFundShareData *pTxFundShareData;
	Tx::Data::BondNewInfo *pBondNewInfo;
	Tx::Data::BondNotChangeAmount *pCBondAmount;
	BOOL bCurDayInPeriod = FALSE;
	DOUBLE dValue = 0, dValue1 = 0, dValue2 = 0, dOccurValue = 0;
	CTime tmCurrent = CTime::GetCurrentTime();

	if(pBusiness->m_pSecurity->GetTradeDateCount() == 0)
	{
		return 0;
	}

	nCurDate = tmCurrent.GetYear() * 10000 + tmCurrent.GetMonth() * 100 + tmCurrent.GetDay();

	nStartDateIdx = pBusiness->m_pSecurity->GetTradeDateIndex(nStartDate);
	if(nStartDateIdx < 0)
	{
		if(pBusiness->m_pSecurity->IsStop())
		{	//摘牌
			return 0;
		}

		nCurDate = tmCurrent.GetYear() * 10000 + tmCurrent.GetMonth() * 100 + tmCurrent.GetDay();	
		if(nStartDate <= nCurDate && nEndDate >= nCurDate)
		{
			if(pBusiness->m_pSecurity->IsValid())
			{
				return nCurDate;
			}else
			{
				return 0;
			}
		}else
		{
			return 0;
		}
	}

	nStartTradeDate = pBusiness->m_pSecurity->GetTradeDateByIndex((LONG)nStartDateIdx);
	if(nStartTradeDate < nStartDate)
	{
		nStartDateIdx ++;
	}
	if(nStartDateIdx == pBusiness->m_pSecurity->GetTradeDateCount())
	{
		nStartDateIdx --;
	}

	nEndTradeDate = pBusiness->m_pSecurity->GetTradeDateLatest();
	if(nEndTradeDate >= nEndDate)
	{
		nEndDateIdx = pBusiness->m_pSecurity->GetTradeDateIndex(nEndDate);
	}else
	{
		nEndDateIdx = pBusiness->m_pSecurity->GetTradeDateCount() - 1;
		if(pBusiness->m_pSecurity->IsValid())
		{
			bCurDayInPeriod = TRUE;
		}
	}

	for(i = nStartDateIdx; i <= nEndDateIdx; i++)
	{
		dValue = 0;
		nDate = pBusiness->m_pSecurity->GetTradeDateByIndex((LONG)i);
		switch(iValueType)
		{
		case 0:		//成交量
			dValue = pBusiness->m_pSecurity->GetVolume(nDate);
			break;
		case 1:		//成交金额
			dValue = pBusiness->m_pSecurity->GetAmount(nDate);
			break;
		case 2:		//换手率
			dValue1 = pBusiness->m_pSecurity->GetVolume(nDate);
			dValue2 = 0;
			if(pBusiness->m_pSecurity->IsStock())
			{	//股票
				pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(nDate);
				if(pTxShareData != NULL)
				{
					dValue2 = pTxShareData->TradeableShare;
				}
			}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
			{	//基金
				pTxFundShareData = pBusiness->m_pSecurity->GetTxFundShareDataByDate(nDate);
				if(pTxFundShareData != NULL)
				{
					dValue2 = pTxFundShareData->TradeableShare;
				}
			}else if(pBusiness->m_pSecurity->IsBond_Change())
			{	//可转债
				pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
				j = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
				if(j > 0)
				{
					pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex();
					if(nDate < pCBondAmount->end_date)
					{	//可转债发行量
						dValue2 = pBondNewInfo->share / 100;
					}else
					{	//未转换债券金额
						pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(j - 1);
						if(nDate >= pCBondAmount->end_date)
						{
							dValue2 = pCBondAmount->not_change_bond_amount / 100;
						}else
						{
							pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByDate(nDate);
							dValue2 = pCBondAmount->not_change_bond_amount / 100;
						}
					}
				}else
				{	//可转债发行量
					dValue2 = pBondNewInfo->share / 100 ;
				}
			}
			if(dValue1 > 0 && dValue2 > 0)
			{
				dValue = dValue1 * 100 / dValue2;
			}
			break;
		case 3:	//流通市值
			dValue1 = pBusiness->m_pSecurity->GetClosePrice(nDate);
			dValue2 = 0;
			if(pBusiness->m_pSecurity->IsStock())
			{	//股票
				pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(nDate);
				if(pTxShareData != NULL)
				{
					dValue2 = pTxShareData->TradeableShare;
				}
			}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
			{	//基金
				pTxFundShareData = pBusiness->m_pSecurity->GetTxFundShareDataByDate(nDate);
				if(pTxFundShareData != NULL)
				{
					dValue2 = pTxFundShareData->TradeableShare;
				}
			}else if(pBusiness->m_pSecurity->IsBond_Change())
			{	//可转债
				pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
				j = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
				if(j > 0)
				{
					pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex();
					if(nDate < pCBondAmount->end_date)
					{	//可转债发行量
						dValue2 = pBondNewInfo->share / 100;
					}else
					{	//未转换债券金额
						pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(j - 1);
						if(nDate >= pCBondAmount->end_date)
						{
							dValue2 = pCBondAmount->not_change_bond_amount / 100;
						}else
						{
							pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByDate(nDate);
							dValue2 = pCBondAmount->not_change_bond_amount / 100;
						}
					}
				}else
				{	//可转债发行量
					dValue2 = pBondNewInfo->share / 100 ;
				}
			}
			if(dValue1 > 0 && dValue2 > 0)
			{
				if(pBusiness->m_pSecurity->IsStock())
				{
					strValue.Format(_T("%.2f"), dValue1);
				}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
				{
					strValue.Format(_T("%.3f"), dValue1);
				}else if(pBusiness->m_pSecurity->IsBond_Change())
				{
					strValue.Format(_T("%.2f"), dValue1);
				}
				dValue1 = atof(strValue);
				dValue2 = (ULONGLONG)dValue2;
				dValue = dValue1 * dValue2;
			}			
			break;
		case 4:	//境内总值
			dValue1 = pBusiness->m_pSecurity->GetClosePrice(nDate);
			dValue2 = 0;
			if(pBusiness->m_pSecurity->IsStock())
			{	//股票
				pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(nDate);
				if(pTxShareData != NULL)
				{
					dValue2 = pTxShareData->TheShare;
				}
			}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
			{	//基金
				pTxFundShareData = pBusiness->m_pSecurity->GetTxFundShareDataByDate(nDate);
				if(pTxFundShareData != NULL)
				{
					dValue2 = pTxFundShareData->TotalShare;
				}
			}else if(pBusiness->m_pSecurity->IsBond_Change())
			{	//可转债
				pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
				j = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
				if(j > 0)
				{
					pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex();
					if(nDate < pCBondAmount->end_date)
					{	//可转债发行量
						dValue2 = pBondNewInfo->share / 100;
					}else
					{	//未转换债券金额
						pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(j - 1);
						if(nDate >= pCBondAmount->end_date)
						{
							dValue2 = pCBondAmount->not_change_bond_amount / 100;
						}else
						{
							pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByDate(nDate);
							dValue2 = pCBondAmount->not_change_bond_amount / 100;
						}
					}
				}else
				{	//可转债发行量
					dValue2 = pBondNewInfo->share / 100 ;
				}
			}
			if(dValue1 > 0 && dValue2 > 0)
			{
				if(pBusiness->m_pSecurity->IsStock())
				{
					strValue.Format(_T("%.2f"), dValue1);
				}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
				{
					strValue.Format(_T("%.3f"), dValue1);
				}else if(pBusiness->m_pSecurity->IsBond_Change())
				{
					strValue.Format(_T("%.2f"), dValue1);
				}
				dValue1 = atof(strValue);
				dValue2 = (ULONGLONG)dValue2;
				dValue = dValue1 * dValue2;
			}
			break;
		case 5:	//总市值
			dValue1 = pBusiness->m_pSecurity->GetClosePrice(nDate);
			dValue2 = 0;
			if(pBusiness->m_pSecurity->IsStock())
			{	//股票
				pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(nDate);
				if(pTxShareData != NULL)
				{
					dValue2 = pTxShareData->TotalShare;
				}
			}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
			{	//基金
				pTxFundShareData = pBusiness->m_pSecurity->GetTxFundShareDataByDate(nDate);
				if(pTxFundShareData != NULL)
				{
					dValue2 = pTxFundShareData->TotalShare;
				}
			}else if(pBusiness->m_pSecurity->IsBond_Change())
			{	//可转债
				pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
				j = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
				if(j > 0)
				{
					pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex();
					if(nDate < pCBondAmount->end_date)
					{	//可转债发行量
						dValue2 = pBondNewInfo->share / 100;
					}else
					{	//未转换债券金额
						pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(j - 1);
						if(nDate >= pCBondAmount->end_date)
						{
							dValue2 = pCBondAmount->not_change_bond_amount / 100;
						}else
						{
							pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByDate(nDate);
							dValue2 = pCBondAmount->not_change_bond_amount / 100;
						}
					}
				}else
				{	//可转债发行量
					dValue2 = pBondNewInfo->share / 100 ;
				}
			}
			if(dValue1 > 0 && dValue2 > 0)
			{
				if(pBusiness->m_pSecurity->IsStock())
				{
					strValue.Format(_T("%.2f"), dValue1);
				}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
				{
					strValue.Format(_T("%.3f"), dValue1);
				}else if(pBusiness->m_pSecurity->IsBond_Change())
				{
					strValue.Format(_T("%.2f"), dValue1);
				}
				dValue1 = atof(strValue);
				dValue2 = (ULONGLONG)dValue2;
				dValue = dValue1 * dValue2;
			}
			break;
		case 6:	//外盘量
			break;
		case 7:	//内盘量
			break;
		default:
			break;
		}

		if(dValue <= dFindValue && dValue > dOccurValue)
		{
			dOccurValue = dValue;
			nOccurDate = nDate;
		}
	}

	if(bCurDayInPeriod)
	{
		dValue = 0;
		switch(iValueType)
		{
		case 0:		//成交量
			dValue = pBusiness->m_pSecurity->GetVolume();
			break;
		case 1:		//成交金额
			dValue = pBusiness->m_pSecurity->GetAmount();
			break;
		case 2:		//换手率
			dValue1 = pBusiness->m_pSecurity->GetVolume();
			dValue2 = 0;
			if(pBusiness->m_pSecurity->IsStock())
			{	//股票
				dValue2 = pBusiness->m_pSecurity->GetTradableShare();
			}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
			{	//基金
				dValue2 = pBusiness->m_pSecurity->GetTradableShare();
			}else if(pBusiness->m_pSecurity->IsBond_Change())
			{	//可转债
				pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
				j = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
				if(j > 0)
				{	//最近一次未转换债券金额
					pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(j - 1);
					dValue2 = pCBondAmount->not_change_bond_amount / 100;
				}else
				{	//可转债发行量
					dValue2 = pBondNewInfo->share / 100 ;
				}
			}
			if(dValue1 > 0 && dValue2 > 0)
			{
				dValue = dValue1 * 100 / dValue2;
			}
			break;
		case 3:	//流通市值
			dValue1 = pBusiness->m_pSecurity->GetClosePrice();
			dValue2 = 0;
			if(pBusiness->m_pSecurity->IsStock())
			{	//股票
				dValue2 = pBusiness->m_pSecurity->GetTradableShare();
			}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
			{	//基金
				dValue2 = pBusiness->m_pSecurity->GetTradableShare();
			}else if(pBusiness->m_pSecurity->IsBond_Change())
			{	//可转债
				pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
				j = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
				if(j > 0)
				{	//最近一次未转换债券金额
					pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(j - 1);
					dValue2 = pCBondAmount->not_change_bond_amount / 100;
				}else
				{	//可转债发行量
					dValue2 = pBondNewInfo->share / 100 ;
				}
			}
			if(dValue1 > 0 && dValue2 > 0)
			{
				if(pBusiness->m_pSecurity->IsStock())
				{
					strValue.Format(_T("%.2f"), dValue1);
				}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
				{
					strValue.Format(_T("%.3f"), dValue1);
				}else if(pBusiness->m_pSecurity->IsBond_Change())
				{
					strValue.Format(_T("%.2f"), dValue1);
				}
				dValue1 = atof(strValue);
				dValue2 = (ULONGLONG)dValue2;
				dValue = dValue1 * dValue2;
			}
			break;
		case 4:	//境内总值
			dValue1 = pBusiness->m_pSecurity->GetClosePrice();
			dValue2 = 0;
			if(pBusiness->m_pSecurity->IsStock())
			{	//股票
				dValue2 = pBusiness->m_pSecurity->GetTotalInstitutionShare();
			}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
			{	//基金
				dValue2 = pBusiness->m_pSecurity->GetTotalInstitutionShare();
			}else if(pBusiness->m_pSecurity->IsBond_Change())
			{	//可转债
				pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
				j = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
				if(j > 0)
				{	//最近一次未转换债券金额
					pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(j - 1);
					dValue2 = pCBondAmount->not_change_bond_amount / 100;
				}else
				{	//可转债发行量
					dValue2 = pBondNewInfo->share / 100 ;
				}
			}
			if(dValue1 > 0 && dValue2 > 0)
			{
				if(pBusiness->m_pSecurity->IsStock())
				{
					strValue.Format(_T("%.2f"), dValue1);
				}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
				{
					strValue.Format(_T("%.3f"), dValue1);
				}else if(pBusiness->m_pSecurity->IsBond_Change())
				{
					strValue.Format(_T("%.2f"), dValue1);
				}
				dValue1 = atof(strValue);
				dValue2 = (ULONGLONG)dValue2;
				dValue = dValue1 * dValue2;
			}
			break;
		case 5:	//总市值
			dValue1 = pBusiness->m_pSecurity->GetClosePrice();
			dValue2 = 0;
			if(pBusiness->m_pSecurity->IsStock())
			{	//股票
				dValue2 = pBusiness->m_pSecurity->GetTotalShare();
			}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
			{	//基金
				dValue2 = pBusiness->m_pSecurity->GetTotalShare();
			}else if(pBusiness->m_pSecurity->IsBond_Change())
			{	//可转债
				pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
				j = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
				if(j > 0)
				{	//最近一次未转换债券金额
					pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(j - 1);
					dValue2 = pCBondAmount->not_change_bond_amount / 100;
				}else
				{	//可转债发行量
					dValue2 = pBondNewInfo->share / 100 ;
				}
			}
			if(dValue1 > 0 && dValue2 > 0)
			{
				if(pBusiness->m_pSecurity->IsStock())
				{
					strValue.Format(_T("%.2f"), dValue1);
				}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
				{
					strValue.Format(_T("%.3f"), dValue1);
				}else if(pBusiness->m_pSecurity->IsBond_Change())
				{
					strValue.Format(_T("%.2f"), dValue1);
				}
				dValue1 = atof(strValue);
				dValue2 = (ULONGLONG)dValue2;
				dValue = dValue1 * dValue2;
			}
			break;
		case 6:	//外盘量
			break;
		case 7:	//内盘量
			break;
		default:
			break;
		}

		if(dValue <= dFindValue && dValue > dOccurValue)
		{
			dOccurValue = dValue;
			nOccurDate = nCurDate;
		}
	}

	return nOccurDate;
}

int TxIndicator::Get_Price_Occur_Date(Tx::Business::TxBusiness *pBusiness, DOUBLE dPrice, int iPriceType, int iIRType, int nStartDate, int nEndDate)
{
	int i, nStartDateIdx, nEndDateIdx, nStartTradeDate, nEndTradeDate, nDate, nCurDate, nOccurDate = 0;
	BOOL bCurDayInPeriod = FALSE;
	DOUBLE dValue = 0, dValue1 = 0, dValue2 = 0, dOccurValue = 0;
	CTime tmCurrent = CTime::GetCurrentTime();

	if(pBusiness->m_pSecurity->GetTradeDateCount() == 0)
	{
		return 0;
	}

	nCurDate = tmCurrent.GetYear() * 10000 + tmCurrent.GetMonth() * 100 + tmCurrent.GetDay();

	nStartDateIdx = pBusiness->m_pSecurity->GetTradeDateIndex(nStartDate);
	if(nStartDateIdx < 0)
	{
		if(pBusiness->m_pSecurity->IsStop())
		{	//摘牌
			return 0;
		}

		nCurDate = tmCurrent.GetYear() * 10000 + tmCurrent.GetMonth() * 100 + tmCurrent.GetDay();	
		if(nStartDate <= nCurDate && nEndDate >= nCurDate)
		{
			if(pBusiness->m_pSecurity->IsValid())
			{
				return nCurDate;
			}else
			{
				return 0;
			}
		}else
		{
			return 0;
		}
	}

	nStartTradeDate = pBusiness->m_pSecurity->GetTradeDateByIndex((LONG)nStartDateIdx);
	if(nStartTradeDate < nStartDate)
	{
		nStartDateIdx ++;
	}
	if(nStartDateIdx == pBusiness->m_pSecurity->GetTradeDateCount())
	{
		nStartDateIdx --;
	}
	nStartTradeDate = pBusiness->m_pSecurity->GetTradeDateByIndex((LONG)nStartDateIdx);

	nEndTradeDate = pBusiness->m_pSecurity->GetTradeDateLatest();
	if(nEndTradeDate >= nEndDate)
	{
		nEndDateIdx = pBusiness->m_pSecurity->GetTradeDateIndex(nEndDate);
	}else
	{
		nEndDateIdx = pBusiness->m_pSecurity->GetTradeDateCount() - 1;
		if(pBusiness->m_pSecurity->IsValid())
		{
			bCurDayInPeriod = TRUE;
		}
	}
	nEndTradeDate = pBusiness->m_pSecurity->GetTradeDateByIndex((LONG)nEndDateIdx);

	for(i = nStartDateIdx; i <= nEndDateIdx; i++)
	{
		dValue = 0;
		nDate = pBusiness->m_pSecurity->GetTradeDateByIndex(i);

		switch(iPriceType)
		{
		case 0:		//开盘价
			if(iIRType == 1)
			{	//后复权
				dValue = pBusiness->m_pSecurity->GetOpenPrice(nDate) * pBusiness->m_pSecurity->GetExdividendScale(nStartTradeDate, nDate, false);
			}else
			{	//前复权
				dValue = pBusiness->m_pSecurity->GetOpenPrice(nDate) * pBusiness->m_pSecurity->GetExdividendScale(nDate, nEndTradeDate, true);
			}
			break;
		case 1:		//前收价
			if(iIRType == 1)
			{	//后复权
				dValue = pBusiness->m_pSecurity->GetPreClosePrice(nDate) * pBusiness->m_pSecurity->GetExdividendScale(nStartTradeDate, nDate, false);
			}else
			{	//前复权
				dValue = pBusiness->m_pSecurity->GetPreClosePrice(nDate) * pBusiness->m_pSecurity->GetExdividendScale(nDate, nEndTradeDate, true);
			}
			break;
		case 2:		//最高价
			if(iIRType == 1)
			{	//后复权
				dValue = pBusiness->m_pSecurity->GetHighPrice(nDate) * pBusiness->m_pSecurity->GetExdividendScale(nStartTradeDate, nDate, false);
			}else
			{	//前复权
				dValue = pBusiness->m_pSecurity->GetHighPrice(nDate) * pBusiness->m_pSecurity->GetExdividendScale(nDate, nEndTradeDate, true);
			}
			break;
		case 3:		//最低价
			if(iIRType == 1)
			{	//后复权
				dValue = pBusiness->m_pSecurity->GetLowPrice(nDate) * pBusiness->m_pSecurity->GetExdividendScale(nStartTradeDate, nDate, false);
			}else
			{	//前复权
				dValue = pBusiness->m_pSecurity->GetLowPrice(nDate) * pBusiness->m_pSecurity->GetExdividendScale(nDate, nEndTradeDate, true);
			}
			break;
		case 4:		//收盘价
			if(iIRType == 1)
			{	//后复权
				dValue = pBusiness->m_pSecurity->GetClosePrice(nDate) * pBusiness->m_pSecurity->GetExdividendScale(nStartTradeDate, nDate, false);
			}else
			{	//前复权
				dValue = pBusiness->m_pSecurity->GetClosePrice(nDate) * pBusiness->m_pSecurity->GetExdividendScale(nDate, nEndTradeDate, true);
			}
			break;
		case 5:		//日均价
			dValue1 = pBusiness->m_pSecurity->GetAmount(nDate);
			dValue2 = pBusiness->m_pSecurity->GetVolume(nDate);
			if(dValue1 > 0 && dValue2 > 0)
			{
				if(iIRType == 1)
				{	//后复权
					dValue = dValue1 / dValue2 * pBusiness->m_pSecurity->GetExdividendScale(nStartTradeDate, nDate, false);
				}else
				{	//前复权
					dValue = dValue1 / dValue2 * pBusiness->m_pSecurity->GetExdividendScale(nDate, nEndTradeDate, true);
				}
			}
			break;
		default:
			break;
		}

		if(dValue <= dPrice && dValue > dOccurValue)
		{
			dOccurValue = dValue;
			nOccurDate = nDate;
		}
	}

	if(bCurDayInPeriod)
	{
		dValue = 0;
		switch(iPriceType)
		{
		case 0:		//开盘价
			if(iIRType == 1)
			{	//后复权
				dValue = pBusiness->m_pSecurity->GetOpenPrice() * pBusiness->m_pSecurity->GetExdividendScale(nStartTradeDate, nCurDate, false);
			}else
			{	//前复权
				dValue = pBusiness->m_pSecurity->GetOpenPrice();
			}
			break;
		case 1:		//前收价
			if(iIRType == 1)
			{	//后复权
				dValue = pBusiness->m_pSecurity->GetPreClosePrice() * pBusiness->m_pSecurity->GetExdividendScale(nStartTradeDate, nCurDate, false);
			}else
			{	//前复权
				dValue = pBusiness->m_pSecurity->GetPreClosePrice();
			}
			break;
		case 2:		//最高价
			if(iIRType == 1)
			{	//后复权
				dValue = pBusiness->m_pSecurity->GetHighPrice() * pBusiness->m_pSecurity->GetExdividendScale(nStartTradeDate, nCurDate, false);
			}else
			{	//前复权
				dValue = pBusiness->m_pSecurity->GetHighPrice();
			}
			break;
		case 3:		//最低价
			if(iIRType == 1)
			{	//后复权
				dValue = pBusiness->m_pSecurity->GetLowPrice() * pBusiness->m_pSecurity->GetExdividendScale(nStartTradeDate, nCurDate, false);
			}else
			{	//前复权
				dValue = pBusiness->m_pSecurity->GetLowPrice();
			}
			break;
		case 4:		//收盘价
			if(iIRType == 1)
			{	//后复权
				dValue = pBusiness->m_pSecurity->GetClosePrice() * pBusiness->m_pSecurity->GetExdividendScale(nStartTradeDate, nCurDate, false);
			}else
			{	//前复权
				dValue = pBusiness->m_pSecurity->GetClosePrice();
			}
			break;
		case 5:		//日均价
			dValue1 = pBusiness->m_pSecurity->GetAmount();
			dValue2 = pBusiness->m_pSecurity->GetVolume();
			if(dValue1 > 0 && dValue2 > 0)
			{
				if(iIRType == 1)
				{	//后复权
					dValue = dValue1 / dValue2 * pBusiness->m_pSecurity->GetExdividendScale(nStartTradeDate, nCurDate, false);
				}else
				{	//前复权
					dValue = dValue1 / dValue2;
				}
			}
			break;
		default:
			break;
		}

		if(dValue <= dPrice && dValue > dOccurValue)
		{
			dOccurValue = dValue;
			nOccurDate = nCurDate;
		}
	}

	return nOccurDate;
}

int TxIndicator::Get_Value_Extremum_Date(Tx::Business::TxBusiness *pBusiness, int iValueType, int nStartDate, int nEndDate, BOOL bMinimum)
{
	int i, j, nStartDateIdx, nEndDateIdx, nStartTradeDate, nEndTradeDate, nDate, nCurDate, nExtremumDate = 0;
	CString strValue;
	Tx::Data::TxShareData *pTxShareData;
	Tx::Data::TxFundShareData *pTxFundShareData;
	Tx::Data::BondNewInfo *pBondNewInfo;
	Tx::Data::BondNotChangeAmount *pCBondAmount;
	BOOL bCurDayInPeriod = FALSE;
	DOUBLE dValue = 0, dValue1 = 0, dValue2 = 0, dExtremumValue = 0;
	CTime tmCurrent = CTime::GetCurrentTime();

	if(pBusiness->m_pSecurity->GetTradeDateCount() == 0)
	{
		return 0;
	}

	nCurDate = tmCurrent.GetYear() * 10000 + tmCurrent.GetMonth() * 100 + tmCurrent.GetDay();

	nStartDateIdx = pBusiness->m_pSecurity->GetTradeDateIndex(nStartDate);
	if(nStartDateIdx < 0)
	{
		if(pBusiness->m_pSecurity->IsStop())
		{	//摘牌
			return 0;
		}

		nCurDate = tmCurrent.GetYear() * 10000 + tmCurrent.GetMonth() * 100 + tmCurrent.GetDay();	
		if(nStartDate <= nCurDate && nEndDate >= nCurDate)
		{
			if(pBusiness->m_pSecurity->IsValid())
			{
				return nCurDate;
			}else
			{
				return 0;
			}
		}else
		{
			return 0;
		}
	}

	nStartTradeDate = pBusiness->m_pSecurity->GetTradeDateByIndex((LONG)nStartDateIdx);
	if(nStartTradeDate < nStartDate)
	{
		nStartDateIdx ++;
	}
	if(nStartDateIdx == pBusiness->m_pSecurity->GetTradeDateCount())
	{
		nStartDateIdx --;
	}

	nEndTradeDate = pBusiness->m_pSecurity->GetTradeDateLatest();
	if(nEndTradeDate >= nEndDate)
	{
		nEndDateIdx = pBusiness->m_pSecurity->GetTradeDateIndex(nEndDate);
	}else
	{
		nEndDateIdx = pBusiness->m_pSecurity->GetTradeDateCount() - 1;
		if(pBusiness->m_pSecurity->IsValid())
		{
			bCurDayInPeriod = TRUE;
		}
	}

	for(i = nStartDateIdx; i <= nEndDateIdx; i++)
	{
		dValue = 0;
		nDate = pBusiness->m_pSecurity->GetTradeDateByIndex((LONG)i);
		switch(iValueType)
		{
		case 0:		//成交量
			dValue = pBusiness->m_pSecurity->GetVolume(nDate);
			break;
		case 1:		//成交金额
			dValue = pBusiness->m_pSecurity->GetAmount(nDate);
			break;
		case 2:		//换手率
			dValue1 = pBusiness->m_pSecurity->GetVolume(nDate);
			dValue2 = 0;
			if(pBusiness->m_pSecurity->IsStock())
			{	//股票
				pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(nDate);
				if(pTxShareData != NULL)
				{
					dValue2 = pTxShareData->TradeableShare;
				}
			}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
			{	//基金
				pTxFundShareData = pBusiness->m_pSecurity->GetTxFundShareDataByDate(nDate);
				if(pTxFundShareData != NULL)
				{
					dValue2 = pTxFundShareData->TradeableShare;
				}
			}else if(pBusiness->m_pSecurity->IsBond_Change())
			{	//可转债
				pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
				j = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
				if(j > 0)
				{
					pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex();
					if(nDate < pCBondAmount->end_date)
					{	//可转债发行量
						dValue2 = pBondNewInfo->share / 100;
					}else
					{	//未转换债券金额
						pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(j - 1);
						if(nDate >= pCBondAmount->end_date)
						{
							dValue2 = pCBondAmount->not_change_bond_amount / 100;
						}else
						{
							pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByDate(nDate);
							dValue2 = pCBondAmount->not_change_bond_amount / 100;
						}
					}
				}else
				{	//可转债发行量
					dValue2 = pBondNewInfo->share / 100 ;
				}
			}
			if(dValue1 > 0 && dValue2 > 0)
			{
				dValue = dValue1 * 100 / dValue2;
			}
			break;
		case 3:	//流通市值
			dValue1 = pBusiness->m_pSecurity->GetClosePrice(nDate);
			dValue2 = 0;
			if(pBusiness->m_pSecurity->IsStock())
			{	//股票
				pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(nDate);
				if(pTxShareData != NULL)
				{
					dValue2 = pTxShareData->TradeableShare;
				}
			}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
			{	//基金
				pTxFundShareData = pBusiness->m_pSecurity->GetTxFundShareDataByDate(nDate);
				if(pTxFundShareData != NULL)
				{
					dValue2 = pTxFundShareData->TradeableShare;
				}
			}else if(pBusiness->m_pSecurity->IsBond_Change())
			{	//可转债
				pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
				j = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
				if(j > 0)
				{
					pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex();
					if(nDate < pCBondAmount->end_date)
					{	//可转债发行量
						dValue2 = pBondNewInfo->share / 100;
					}else
					{	//未转换债券金额
						pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(j - 1);
						if(nDate >= pCBondAmount->end_date)
						{
							dValue2 = pCBondAmount->not_change_bond_amount / 100;
						}else
						{
							pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByDate(nDate);
							dValue2 = pCBondAmount->not_change_bond_amount / 100;
						}
					}
				}else
				{	//可转债发行量
					dValue2 = pBondNewInfo->share / 100 ;
				}
			}
			if(dValue1 > 0 && dValue2 > 0)
			{
				if(pBusiness->m_pSecurity->IsStock())
				{
					strValue.Format(_T("%.2f"), dValue1);
				}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
				{
					strValue.Format(_T("%.3f"), dValue1);
				}else if(pBusiness->m_pSecurity->IsBond_Change())
				{
					strValue.Format(_T("%.2f"), dValue1);
				}
				dValue1 = atof(strValue);
				dValue2 = (ULONGLONG)dValue2;
				dValue = dValue1 * dValue2;
			}		
			break;
		case 4:	//境内总值
			dValue1 = pBusiness->m_pSecurity->GetClosePrice(nDate);
			dValue2 = 0;
			if(pBusiness->m_pSecurity->IsStock())
			{	//股票
				pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(nDate);
				if(pTxShareData != NULL)
				{
					dValue2 = pTxShareData->TheShare;
				}
			}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
			{	//基金
				pTxFundShareData = pBusiness->m_pSecurity->GetTxFundShareDataByDate(nDate);
				if(pTxFundShareData != NULL)
				{
					dValue2 = pTxFundShareData->TradeableShare;
				}
			}else if(pBusiness->m_pSecurity->IsBond_Change())
			{	//可转债
				pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
				j = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
				if(j > 0)
				{
					pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex();
					if(nDate < pCBondAmount->end_date)
					{	//可转债发行量
						dValue2 = pBondNewInfo->share / 100;
					}else
					{	//未转换债券金额
						pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(j - 1);
						if(nDate >= pCBondAmount->end_date)
						{
							dValue2 = pCBondAmount->not_change_bond_amount / 100;
						}else
						{
							pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByDate(nDate);
							dValue2 = pCBondAmount->not_change_bond_amount / 100;
						}
					}
				}else
				{	//可转债发行量
					dValue2 = pBondNewInfo->share / 100 ;
				}
			}
			if(dValue1 > 0 && dValue2 > 0)
			{
				if(pBusiness->m_pSecurity->IsStock())
				{
					strValue.Format(_T("%.2f"), dValue1);
				}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
				{
					strValue.Format(_T("%.3f"), dValue1);
				}else if(pBusiness->m_pSecurity->IsBond_Change())
				{
					strValue.Format(_T("%.2f"), dValue1);
				}
				dValue1 = atof(strValue);
				dValue2 = (ULONGLONG)dValue2;
				dValue = dValue1 * dValue2;
			}
			break;
		case 5:	//总市值
			dValue1 = pBusiness->m_pSecurity->GetClosePrice(nDate);
			dValue2 = 0;
			if(pBusiness->m_pSecurity->IsStock())
			{	//股票
				pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(nDate);
				if(pTxShareData != NULL)
				{
					dValue2 = pTxShareData->TotalShare;
				}
			}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
			{	//基金
				pTxFundShareData = pBusiness->m_pSecurity->GetTxFundShareDataByDate(nDate);
				if(pTxFundShareData != NULL)
				{
					dValue2 = pTxFundShareData->TotalShare;
				}
			}else if(pBusiness->m_pSecurity->IsBond_Change())
			{	//可转债
				pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
				j = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
				if(j > 0)
				{
					pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex();
					if(nDate < pCBondAmount->end_date)
					{	//可转债发行量
						dValue2 = pBondNewInfo->share / 100;
					}else
					{	//未转换债券金额
						pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(j - 1);
						if(nDate >= pCBondAmount->end_date)
						{
							dValue2 = pCBondAmount->not_change_bond_amount / 100;
						}else
						{
							pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByDate(nDate);
							dValue2 = pCBondAmount->not_change_bond_amount / 100;
						}
					}
				}else
				{	//可转债发行量
					dValue2 = pBondNewInfo->share / 100 ;
				}
			}
			if(dValue1 > 0 && dValue2 > 0)
			{
				if(pBusiness->m_pSecurity->IsStock())
				{
					strValue.Format(_T("%.2f"), dValue1);
				}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
				{
					strValue.Format(_T("%.3f"), dValue1);
				}else if(pBusiness->m_pSecurity->IsBond_Change())
				{
					strValue.Format(_T("%.2f"), dValue1);
				}
				dValue1 = atof(strValue);
				dValue2 = (ULONGLONG)dValue2;
				dValue = dValue1 * dValue2;
			}
			break;
		case 6:	//外盘量
			break;
		case 7:	//内盘量
			break;
		default:
			break;
		}

		if(bMinimum)
		{
			if(dExtremumValue == 0 && dValue > 0)
			{
				dExtremumValue = dValue;
				nExtremumDate = nDate;
			}else if(dValue < dExtremumValue)
			{
				dExtremumValue = dValue;
				nExtremumDate = nDate;
			}
		}else
		{
			if(dValue > dExtremumValue)
			{
				dExtremumValue = dValue;
				nExtremumDate = nDate;
			}
		}
	}

	if(bCurDayInPeriod)
	{
		dValue = 0;
		switch(iValueType)
		{
		case 0:		//成交量
			dValue = pBusiness->m_pSecurity->GetVolume();
			break;
		case 1:		//成交金额
			dValue = pBusiness->m_pSecurity->GetAmount();
			break;
		case 2:		//换手率
			dValue1 = pBusiness->m_pSecurity->GetVolume();
			dValue2 = 0;
			if(pBusiness->m_pSecurity->IsStock())
			{	//股票
				dValue2 = pBusiness->m_pSecurity->GetTradableShare();
			}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
			{	//基金
				dValue2 = pBusiness->m_pSecurity->GetTradableShare();
			}else if(pBusiness->m_pSecurity->IsBond_Change())
			{	//可转债
				pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
				j = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
				if(j > 0)
				{	//最近一次未转换债券金额
					pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(j - 1);
					dValue2 = pCBondAmount->not_change_bond_amount / 100;
				}else
				{	//可转债发行量
					dValue2 = pBondNewInfo->share / 100 ;
				}
			}
			if(dValue1 > 0 && dValue2 > 0)
			{
				dValue = dValue1 * 100 / dValue2;
			}
			break;
		case 3:	//流通市值
			dValue1 = pBusiness->m_pSecurity->GetClosePrice();
			dValue2 = 0;
			if(pBusiness->m_pSecurity->IsStock())
			{	//股票
				dValue2 = pBusiness->m_pSecurity->GetTradableShare();
			}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
			{	//基金
				dValue2 = pBusiness->m_pSecurity->GetTradableShare();
			}else if(pBusiness->m_pSecurity->IsBond_Change())
			{	//可转债
				pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
				j = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
				if(j > 0)
				{	//最近一次未转换债券金额
					pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(j - 1);
					dValue2 = pCBondAmount->not_change_bond_amount / 100;
				}else
				{	//可转债发行量
					dValue2 = pBondNewInfo->share / 100 ;
				}
			}
			if(dValue1 > 0 && dValue2 > 0)
			{
				if(pBusiness->m_pSecurity->IsStock())
				{
					strValue.Format(_T("%.2f"), dValue1);
				}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
				{
					strValue.Format(_T("%.3f"), dValue1);
				}else if(pBusiness->m_pSecurity->IsBond_Change())
				{
					strValue.Format(_T("%.2f"), dValue1);
				}
				dValue1 = atof(strValue);
				dValue2 = (ULONGLONG)dValue2;
				dValue = dValue1 * dValue2;
			}
			break;
		case 4:	//境内总值
			dValue1 = pBusiness->m_pSecurity->GetClosePrice();
			dValue2 = 0;
			if(pBusiness->m_pSecurity->IsStock())
			{	//股票
				dValue2 = pBusiness->m_pSecurity->GetTotalInstitutionShare();
			}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
			{	//基金
				dValue2 = pBusiness->m_pSecurity->GetTotalInstitutionShare();
			}else if(pBusiness->m_pSecurity->IsBond_Change())
			{	//可转债
				pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
				j = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
				if(j > 0)
				{	//最近一次未转换债券金额
					pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(j - 1);
					dValue2 = pCBondAmount->not_change_bond_amount / 100;
				}else
				{	//可转债发行量
					dValue2 = pBondNewInfo->share / 100 ;
				}
			}
			if(dValue1 > 0 && dValue2 > 0)
			{
				if(pBusiness->m_pSecurity->IsStock())
				{
					strValue.Format(_T("%.2f"), dValue1);
				}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
				{
					strValue.Format(_T("%.3f"), dValue1);
				}else if(pBusiness->m_pSecurity->IsBond_Change())
				{
					strValue.Format(_T("%.2f"), dValue1);
				}
				dValue1 = atof(strValue);
				dValue2 = (ULONGLONG)dValue2;
				dValue = dValue1 * dValue2;
			}
			break;
		case 5:	//总市值
			dValue1 = pBusiness->m_pSecurity->GetClosePrice();
			dValue2 = 0;
			if(pBusiness->m_pSecurity->IsStock())
			{	//股票
				dValue2 = pBusiness->m_pSecurity->GetTotalShare();
			}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
			{	//基金
				dValue2 = pBusiness->m_pSecurity->GetTotalShare();
			}else if(pBusiness->m_pSecurity->IsBond_Change())
			{	//可转债
				pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
				j = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
				if(j > 0)
				{	//最近一次未转换债券金额
					pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(j - 1);
					dValue2 = pCBondAmount->not_change_bond_amount / 100;
				}else
				{	//可转债发行量
					dValue2 = pBondNewInfo->share / 100 ;
				}
			}
			if(dValue1 > 0 && dValue2 > 0)
			{
				if(pBusiness->m_pSecurity->IsStock())
				{
					strValue.Format(_T("%.2f"), dValue1);
				}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
				{
					strValue.Format(_T("%.3f"), dValue1);
				}else if(pBusiness->m_pSecurity->IsBond_Change())
				{
					strValue.Format(_T("%.2f"), dValue1);
				}
				dValue1 = atof(strValue);
				dValue2 = (ULONGLONG)dValue2;
				dValue = dValue1 * dValue2;
			}
			break;
		case 6:	//外盘量
			break;
		case 7:	//内盘量
			break;
		default:
			break;
		}

		if(bMinimum)
		{
			if(dExtremumValue == 0 && dValue > 0)
			{
				dExtremumValue = dValue;
				nExtremumDate = nCurDate;
			}else if(dValue < dExtremumValue)
			{
				dExtremumValue = dValue;
				nExtremumDate = nCurDate;
			}
		}else
		{
			if(dValue > dExtremumValue)
			{
				dExtremumValue = dValue;
				nExtremumDate = nCurDate;
			}
		}
	}

	return nExtremumDate;
}

int TxIndicator::Get_Price_Extremum_Date(Tx::Business::TxBusiness *pBusiness, int iPriceType, int nStartDate, int nEndDate, BOOL bMinimum)
{
	int i, nStartDateIdx, nEndDateIdx, nStartTradeDate, nEndTradeDate, nDate, nCurDate, nExtremumDate = 0;
	BOOL bCurDayInPeriod = FALSE;
	DOUBLE dValue = 0, dValue1 = 0, dValue2 = 0, dExtremumValue = 0;
	CTime tmCurrent = CTime::GetCurrentTime();

	if(pBusiness->m_pSecurity->GetTradeDateCount() == 0)
	{
		return 0;
	}

	nCurDate = tmCurrent.GetYear() * 10000 + tmCurrent.GetMonth() * 100 + tmCurrent.GetDay();

	nStartDateIdx = pBusiness->m_pSecurity->GetTradeDateIndex(nStartDate);
	if(nStartDateIdx < 0)
	{
		if(pBusiness->m_pSecurity->IsStop())
		{	//摘牌
			return 0;
		}

		nCurDate = tmCurrent.GetYear() * 10000 + tmCurrent.GetMonth() * 100 + tmCurrent.GetDay();	
		if(nStartDate <= nCurDate && nEndDate >= nCurDate)
		{
			if(pBusiness->m_pSecurity->IsValid())
			{
				return nCurDate;
			}else
			{
				return 0;
			}
		}else
		{
			return 0;
		}
	}

	nStartTradeDate = pBusiness->m_pSecurity->GetTradeDateByIndex((LONG)nStartDateIdx);
	if(nStartTradeDate < nStartDate)
	{
		nStartDateIdx ++;
	}
	if(nStartDateIdx == pBusiness->m_pSecurity->GetTradeDateCount())
	{
		nStartDateIdx --;
	}
	nStartTradeDate = pBusiness->m_pSecurity->GetTradeDateByIndex((LONG)nStartDateIdx);

	nEndTradeDate = pBusiness->m_pSecurity->GetTradeDateLatest();
	if(nEndTradeDate >= nEndDate)
	{
		nEndDateIdx = pBusiness->m_pSecurity->GetTradeDateIndex(nEndDate);
	}else
	{
		nEndDateIdx = pBusiness->m_pSecurity->GetTradeDateCount() - 1;
		if(pBusiness->m_pSecurity->IsValid())
		{
			bCurDayInPeriod = TRUE;
		}
	}
	nEndTradeDate = pBusiness->m_pSecurity->GetTradeDateByIndex((LONG)nEndDateIdx);

	for(i = nStartDateIdx; i <= nEndDateIdx; i++)
	{
		dValue = 0;
		nDate = pBusiness->m_pSecurity->GetTradeDateByIndex(i);

		switch(iPriceType)
		{
		case 0:		//开盘价
			if(i > nStartDateIdx)
			{	//后复权
				dValue = pBusiness->m_pSecurity->GetOpenPrice(nDate) * pBusiness->m_pSecurity->GetExdividendScale(nStartTradeDate, nDate, false);
			}else
			{
				dValue = pBusiness->m_pSecurity->GetOpenPrice(nDate);
			}
			break;
		case 1:		//前收价
			if(i > nStartDateIdx)
			{	//后复权
				dValue = pBusiness->m_pSecurity->GetPreClosePrice(nDate) * pBusiness->m_pSecurity->GetExdividendScale(nStartTradeDate, nDate, false);
			}else
			{
				dValue = pBusiness->m_pSecurity->GetPreClosePrice(nDate);
			}
			break;
		case 2:		//最高价
			if(i > nStartDateIdx)
			{	//后复权
				dValue = pBusiness->m_pSecurity->GetHighPrice(nDate) * pBusiness->m_pSecurity->GetExdividendScale(nStartTradeDate, nDate, false);
			}else
			{
				dValue = pBusiness->m_pSecurity->GetHighPrice(nDate);
			}
			break;
		case 3:		//最低价
			if(i > nStartDateIdx)
			{	//后复权
				dValue = pBusiness->m_pSecurity->GetLowPrice(nDate) * pBusiness->m_pSecurity->GetExdividendScale(nStartTradeDate, nDate, false);
			}else
			{
				dValue = pBusiness->m_pSecurity->GetLowPrice(nDate);
			}
			break;
		case 4:		//收盘价
			if(i > nStartDateIdx)
			{	//后复权
				dValue = pBusiness->m_pSecurity->GetClosePrice(nDate) * pBusiness->m_pSecurity->GetExdividendScale(nStartTradeDate, nDate, false);
			}else
			{
				dValue = pBusiness->m_pSecurity->GetClosePrice(nDate);
			}
			break;
		case 5:		//日均价
			dValue1 = pBusiness->m_pSecurity->GetAmount(nDate);
			dValue2 = pBusiness->m_pSecurity->GetVolume(nDate);
			if(dValue1 > 0 && dValue2 > 0)
			{
				if(i > nStartDateIdx)
				{	//后复权
					dValue = dValue1 / dValue2 * pBusiness->m_pSecurity->GetExdividendScale(nStartTradeDate, nDate, false);
				}else
				{
					dValue = dValue1 / dValue2;
				}
			}
			break;
		default:
			break;
		}

		if(bMinimum)
		{
			if(dExtremumValue == 0 && dValue > 0)
			{
				dExtremumValue = dValue;
				nExtremumDate = nDate;
			}else if(dValue < dExtremumValue)
			{
				dExtremumValue = dValue;
				nExtremumDate = nDate;
			}
		}else
		{
			if(dValue > dExtremumValue)
			{
				dExtremumValue = dValue;
				nExtremumDate = nDate;
			}
		}
	}

	if(bCurDayInPeriod)
	{
		dValue = 0;

		switch(iPriceType)
		{
		case 0:		//开盘价
			dValue = pBusiness->m_pSecurity->GetOpenPrice() * pBusiness->m_pSecurity->GetExdividendScale(nStartTradeDate, nCurDate, false);		//后复权
			break;
		case 1:		//前收价
			dValue = pBusiness->m_pSecurity->GetPreClosePrice() * pBusiness->m_pSecurity->GetExdividendScale(nStartTradeDate, nCurDate, false);	//后复权
			break;
		case 2:		//最高价
			dValue = pBusiness->m_pSecurity->GetHighPrice() * pBusiness->m_pSecurity->GetExdividendScale(nStartTradeDate, nCurDate, false);		//后复权
			break;
		case 3:		//最低价
			dValue = pBusiness->m_pSecurity->GetLowPrice() * pBusiness->m_pSecurity->GetExdividendScale(nStartTradeDate, nCurDate, false);			//后复权
			break;
		case 4:		//收盘价
			dValue = pBusiness->m_pSecurity->GetClosePrice() * pBusiness->m_pSecurity->GetExdividendScale(nStartTradeDate, nCurDate, false);		//后复权
			break;
		case 5:		//日均价
			dValue1 = pBusiness->m_pSecurity->GetAmount();
			dValue2 = pBusiness->m_pSecurity->GetVolume();
			if(dValue1 > 0 && dValue2 > 0)
			{
				dValue = dValue1 / dValue2 * pBusiness->m_pSecurity->GetExdividendScale(nStartTradeDate, nCurDate, false);
			}
			break;
		default:
			break;
		}

		if(bMinimum)
		{
			if(dExtremumValue == 0 && dValue > 0)
			{
				dExtremumValue = dValue;
				nExtremumDate = nCurDate;
			}else if(dValue < dExtremumValue)
			{
				dExtremumValue = dValue;
				nExtremumDate = nCurDate;
			}
		}else
		{
			if(dValue > dExtremumValue)
			{
				dExtremumValue = dValue;
				nExtremumDate = nCurDate;
			}
		}
	}

	return nExtremumDate;
}

CString	TxIndicator::Get_QS_Name( int nID )
{
	if ( nID <= 0 )
		return _T("");
	if ( m_tableQS.GetRowCount() == 0 )
	{
		UINT array_ParamCol[3] = {0, 1, 2};
		m_tableQS.AddParameterColumn(Tx::Core::dtype_int4);		//券商ID
		m_tableQS.AddIndicatorColumn(30001064, Tx::Core::dtype_val_string, array_ParamCol, 1);	//券商名称
		Tx::Business::TxBusiness business;
		business.m_pLogicalBusiness->GetData(m_tableQS, true);
	}
	if ( m_tableQS.GetRowCount() == 0 )
		return _T("");
	std::vector< UINT > vRow;
	m_tableQS.Find( 0, nID, vRow );
	if ( vRow.size() == 1 )
	{
		CString str;
		m_tableQS.GetCell( 1,*(vRow.begin()), str );
		return str;
	}
	else
		return _T("");

}

//获取行业分布(证监会(新))市值
//--30377/以财年、财季为文件名则
//<报告期,<交易实体ID+行业ID，行业市值>>
double  TxIndicator::GetFundCombineIndustryDistribute(int nSecurityId,int nReport,int nIndustryId)
{
	bool bMap = false;  
	double dResult = Con_doubleInvalid;

	//<报告期, 交易实体ID, 行业ID, 市值>::iterator
	map<int,map<int,map<int,double>>>::iterator iterMMMap;
    //<交易实体ID, 行业ID, 市值>::iterator
	map<int,map<int,double>>::iterator iterMMap;
	//<交易实体ID, 行业ID, 市值>
	map<int,map<int,double>> dataMMap;
	//<行业ID, 市值>::iterator
	map<int,double>::iterator iterMap;
	//<行业ID, 市值>
	map<int,double> dataMap;

    // 判断map中是否已经存在该报告期的数据
	if((int)mapFundIndustryDistribute.size() > 0)
	{
		iterMMMap = mapFundIndustryDistribute.find(nReport);
		if(iterMMMap == mapFundIndustryDistribute.end())
			bMap = false;
		else
			bMap = true;
	}

	// 如果bMap==false,判断是否下载相应的数据文件
	if (bMap == false)
	{
		DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundCombineIndustryDistribute>* pFile;
		pFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundCombineIndustryDistribute>;
		if(pFile == NULL)
			return dResult;

		CString strFileName ;
		strFileName.Format(_T("%s\\%s\\%d\\%d.dat"),Tx::Core::SystemPath::GetInstance()->GetDataPath(),_T("TxExFile"),30377,nReport);

		pFile->Load(nReport,30377,true);

		// 解析文件，并将数据填入map
		CFile hd;
		if (hd.Open(strFileName,CFile::modeRead | CFile::shareDenyNone) == FALSE)
		{
			delete pFile;
			pFile = NULL;
			return dResult;
		}
		int iSize = sizeof(Tx::Data::FundCombineIndustryDistribute);
		long len = (long)hd.GetLength();
		len -= sizeof(blk_TxExFile_FileHead);
		int iCount = len/iSize;
		//检查数据有效性
		if (iCount*iSize != len)
		{
			hd.Close();
			delete pFile;
			pFile = NULL;
			return dResult;
		}
		if (len <= 0)
		{
			hd.Close();
			delete pFile;
			pFile = NULL;
			return dResult;
		}

		Tx::Data::FundCombineIndustryDistribute* pData = new Tx::Data::FundCombineIndustryDistribute[iCount];
		if (pData == NULL)
		{
			hd.Close();
			delete pFile;
			pFile = NULL;
			return dResult;
		}
		//
		hd.Seek(sizeof(blk_TxExFile_FileHead),CFile::begin);
		//读取数据
		hd.Read(pData,iCount*iSize);
		hd.Close();
		delete pFile;
		pFile = NULL;

		//解析-将数据填入map
		//	int transId;/交易实体ID、int indsid;/行业ID、double amount;/市值
		// <transId+indsid,amount>
		Tx::Data::FundCombineIndustryDistribute data;
		Tx::Data::FundCombineIndustryDistribute* pTmp = pData;
		for (int i=0;i<iCount;i++)
		{
			// pTmp = pData + sizeof(Tx::Data::FundCombineIndustryDistribute)*i;
			memcpy_s(&data,iSize,pTmp,iSize);
			// 交易实体ID 、 行业ID 、市值
			int iTransID = data.transId;
			int iIndsId = data.indsid;
			double dvalue = data.amount;
			//dataMap.insert(std::pair<int,double>(iIndsId,dvalue));
			// 判断map中是否有该交易实体ID
			iterMMap = dataMMap.find(iTransID);
			if (iterMMap == dataMMap.end())
			{
				dataMap.clear();
				dataMap.insert(std::pair<int,double>(iIndsId,dvalue));
				dataMMap.insert(std::pair<int,map<int,double>>(iTransID,dataMap));
			}
			else
				iterMMap->second.insert(std::pair<int,double>(iIndsId,dvalue));
			pTmp++;
		}

		// 将数据填入map-map
		mapFundIndustryDistribute.insert(std::pair<int,map<int,map<int,double>>>(nReport,dataMMap));

		delete pData;
		pData = NULL;
	}

	//从一级map中获取数据
	iterMMMap = mapFundIndustryDistribute.find(nReport);
	if(iterMMMap != mapFundIndustryDistribute.end())
	{
		//从二级map中获取数据
		iterMMap = iterMMMap->second.find(nSecurityId);
		if (iterMMap != iterMMMap->second.end())
		{
			// 从三级map中查找市值
			iterMap = iterMMap->second.find(nIndustryId);
			if (iterMap != iterMMap->second.end())
				dResult = iterMap->second;
		}
	}
	return dResult;
}


//--------------获取基金单位净值-----------------
bool	TxIndicator::GetFundNvr( std::set< int > nSecurity,std::set< int > nDates, Tx::Core::Table* pResTable )
{
	if ( pResTable == NULL )
		pResTable = &m_dFundNvrTable;
	pResTable->Clear();
	m_FundNvrRowMap.clear();
	m_FundNvrColMap.clear();

	if ( 0 == nSecurity.size()|| 0 == nDates.size())
		return false;
	std::vector<int> iSecurity( nSecurity.begin(),nSecurity.end());
	std::vector<int> iDates( nDates.begin(),nDates.end());
	pResTable->AddCol( Tx::Core::dtype_int4 );		//ID
	int nCol = 1;
	for ( std::vector< int >::iterator iter = iDates.begin(); iter != iDates.end(); ++iter )
	{
		pResTable->AddCol( Tx::Core::dtype_double );
		nCol = pResTable->GetColCount()-1;
		m_FundNvrColMap.insert( std::pair<int,int>( *iter,nCol));

	}	
	int nRow = 0;
	for ( std::vector< int >::iterator iter = iSecurity.begin(); iter != iSecurity.end(); ++iter )
	{
		pResTable->AddRow();
		nRow = pResTable->GetRowCount()-1;
		pResTable->SetCell( 0,nRow,*iter );
		m_FundNvrRowMap.insert( std::pair<int,int>( *iter,nRow));
	}	
	{
		// 必然选择了单位净值
		int iSize = sizeof(int) * ( 1 /*请求类型*/ + 1 /*券个数*/ + pResTable->GetRowCount() + 1 /*日期个数*/ + iDates.size());
		LPBYTE pBuffer = new BYTE [iSize];
		if (pBuffer == NULL)
			return false;

		LPBYTE pWrite = pBuffer;
		memset(pBuffer,0,iSize);
		int iType = 1;
		memcpy_s(pWrite,iSize,&iType,sizeof(int));
		pWrite += sizeof(int);
		int nSecSize = (int)pResTable->GetRowCount();
		memcpy_s(pWrite,iSize,&nSecSize,sizeof(int));
		pWrite += sizeof(int);
		for (int i=0;i<nSecSize;i++)
		{
			int iId;
			pResTable->GetCell(0,i,iId);
			memcpy_s(pWrite,iSize,&iId,sizeof(int));
			pWrite += sizeof(int);
		}

		nSecSize = (int)iDates.size();
		memcpy_s(pWrite,iSize,&nSecSize,sizeof(int));
		pWrite += sizeof(int);

		for (int i=0;i<nSecSize;i++)
		{
			memcpy_s(pWrite,iSize,&iDates[i],sizeof(int));
			pWrite += sizeof(int);
		}
		LPCTSTR lpUrl = CORE_GETSYSINFO->GetServerAddr(_T("File"),_T("FundNavRaiseRate"));

		Tx::Drive::Http::CSyncUpload upload;
		int iStart = ::GetTickCount();
		if ( upload.Post(lpUrl, pBuffer, iSize) )
		{
			int iEnd = ::GetTickCount();
			TRACE(_T("\r\nURL Cost Time %d(ms)\r\n"),iEnd-iStart);
			CONST Tx::Drive::Mem::MemSlice &data = upload.Rsp().Body();
			LPBYTE lpRes = data.DataPtr();
			UINT nRetSize = data.Size();
			if (nRetSize <= 0)
			{
				delete pBuffer;
				pBuffer = NULL;
				return false;
			}
			UINT nPreSize = *(reinterpret_cast<UINT*>(lpRes));
			LPBYTE lpData = new BYTE[nPreSize];
			if ( lpData == NULL )
			{
				delete pBuffer;
				pBuffer = NULL;
				return false;
			}
			if ( !Tx::Drive::IO::Zip::CZipWrapper::MemUnZip(lpData, 
				nPreSize, lpRes + sizeof(UINT), nRetSize - sizeof(UINT)) )
			{
				delete []lpData;
				delete[] pBuffer;
				pBuffer = NULL;
				return false;
			}
			iStart = ::GetTickCount();
			LPBYTE pRecv = lpData;
			UINT nParseCount = pResTable->GetRowCount() * iDates.size();
			float fValue = 0.0;
			double dValue = 0.0;
			for (UINT i=0;i < nParseCount; i++)
			{
				memcpy_s(&fValue,sizeof(float),pRecv,sizeof(float));
				pRecv += sizeof(float);
				if (fabs(fValue - Tx::Core::Con_floatInvalid) < 0.00001)
					pResTable->SetCell(i%iDates.size() + 1,i/iDates.size(),Tx::Core::Con_doubleInvalid);
				else
				{
					dValue = (double)fValue;
					pResTable->SetCell(i%iDates.size() + 1,i/iDates.size(),dValue);
				}
			}
			delete []lpData;
			lpData = NULL;
			iEnd = ::GetTickCount();
			TRACE(_T("\r\nParse Result Cost Time %d(ms)\r\n"),iEnd-iStart);
		}
		else
		{
			AfxMessageBox(_T("基金净值网络请求返回失败!"));
		}
		delete[] pBuffer;
		pBuffer = NULL;
	}
	return true;		
}

double	TxIndicator::GetFundNvrVal( int iSecurity,int iDate )
{
	double dRet = Con_doubleInvalid;
	int nRow = -1;
	int nCol = -1;
	std::unordered_map<int,int>::iterator iterRow = m_FundNvrRowMap.find( iSecurity );	//行索引
	std::unordered_map<int,int>::iterator iterCol = m_FundNvrColMap.find( iDate );
	if ( iterRow != m_FundNvrRowMap.end() && iterCol != m_FundNvrColMap.end() )
	{
		nRow = iterRow->second;
		nCol = iterCol->second;
		m_dFundNvrTable.GetCell( nCol,nRow,dRet );
	}
	return dRet;
}
//--------------获取基金累计净值-----------------
bool	TxIndicator::GetFundAcuNvr( std::set< int > nSecurity,std::set< int > nDates, Tx::Core::Table* pResTable )
{
	if ( pResTable == NULL )
		pResTable = &m_dFundAcuNvrTable;
	pResTable->Clear();
	m_dFundAcuNvrColMap.clear();
	m_dFundAcuNvrRowMap.clear();
	if ( 0 == nSecurity.size()|| 0 == nDates.size() )
		return false;
	std::vector<int> iSecurity( nSecurity.begin(),nSecurity.end());
	std::vector<int> iDates( nDates.begin(),nDates.end());

	pResTable->AddCol( Tx::Core::dtype_int4 );		//ID
	int nCol = 1;
	for ( std::vector< int >::iterator iter = iDates.begin(); iter != iDates.end(); ++iter )
	{
		pResTable->AddCol( Tx::Core::dtype_double );
		m_dFundAcuNvrColMap.insert( std::pair<int,int>( *iter,nCol++));
	}	
	int nRow = 0;
	for ( std::vector< int >::iterator iter = iSecurity.begin(); iter != iSecurity.end(); ++iter )
	{
		pResTable->AddRow();
		pResTable->SetCell( 0,nRow,*iter );
		m_dFundAcuNvrRowMap.insert( std::pair<int,int>( *iter,nRow++));
	}	
	{
			// 必然选择了累计净值
			int iSize = sizeof(int) * ( 1 /*请求类型*/ + 1 /*券个数*/ + pResTable->GetRowCount() + 1 /*日期个数*/ + iDates.size());
			LPBYTE pBuffer = new BYTE [iSize];
			if (pBuffer == NULL)
				return false;
			LPBYTE pWrite = pBuffer;
			memset(pBuffer,0,iSize);
			int iType = 2;
			memcpy_s(pWrite,iSize,&iType,sizeof(int));
			pWrite += sizeof(int);
			int nSecSize = (int)pResTable->GetRowCount();
			memcpy_s(pWrite,iSize,&nSecSize,sizeof(int));
			pWrite += sizeof(int);
			for (int i=0;i<nSecSize;i++)
			{
				int iId;
				pResTable->GetCell(0,i,iId);
				memcpy_s(pWrite,iSize,&iId,sizeof(int));
				pWrite += sizeof(int);
			}

			nSecSize = (int)iDates.size();
			memcpy_s(pWrite,iSize,&nSecSize,sizeof(int));
			pWrite += sizeof(int);

			for (int i=0;i<nSecSize;i++)
			{
				memcpy_s(pWrite,iSize,&iDates[i],sizeof(int));
				pWrite += sizeof(int);
			}
			LPCTSTR lpUrl = CORE_GETSYSINFO->GetServerAddr(_T("File"),_T("FundNavRaiseRate"));

			Tx::Drive::Http::CSyncUpload upload;
			int iStart = ::GetTickCount();
			if ( upload.Post(lpUrl, pBuffer, iSize) )
			{
				int iEnd = ::GetTickCount();
				TRACE(_T("\r\nURL Cost Time %d(ms)\r\n"),iEnd-iStart);
				CONST Tx::Drive::Mem::MemSlice &data = upload.Rsp().Body();
				LPBYTE lpRes = data.DataPtr();
				UINT nRetSize = data.Size();
				if (nRetSize <= 0)
				{
					delete pBuffer;
					pBuffer = NULL;
					return false;
				}

				UINT nPreSize = *(reinterpret_cast<UINT*>(lpRes));
				LPBYTE lpData = new BYTE[nPreSize];
				if ( lpData == NULL )
				{
					delete pBuffer;
					pBuffer = NULL;
					return false;
				}
				if ( !Tx::Drive::IO::Zip::CZipWrapper::MemUnZip(lpData, 
					nPreSize, lpRes + sizeof(UINT), nRetSize - sizeof(UINT)) )
				{
					delete []lpData;
					delete[] pBuffer;
					pBuffer = NULL;
					return false;
				}

				iStart = ::GetTickCount();
				LPBYTE pRecv = lpData;
				UINT nParseCount = pResTable->GetRowCount() * iDates.size();
				float fValue = 0.0;
				double dValue = 0.0;
				for (UINT i=0;i < nParseCount; i++)
				{
					memcpy_s(&fValue,sizeof(float),pRecv,sizeof(float));
					pRecv += sizeof(float);
					if (fabs(fValue - Tx::Core::Con_floatInvalid) < 0.00001)
						pResTable->SetCell(i%iDates.size() + 1,i/iDates.size(),Tx::Core::Con_doubleInvalid);
					else
					{
						dValue = (double)fValue;
						pResTable->SetCell(i%iDates.size() + 1,i/iDates.size(),dValue);
					}
				}
				delete []lpData;
				lpData = NULL;
				iEnd = ::GetTickCount();
				TRACE(_T("\r\nParse Result Cost Time %d(ms)\r\n"),iEnd-iStart);
			}
			else
			{
				AfxMessageBox(_T("基金净值网络请求返回失败!"));
			}
			delete[] pBuffer;
			pBuffer = NULL;
	}
	return true;
}

double TxIndicator::GetFundAcuNvrVal( int iSecurity,int iDate )
{
	double dRet = Con_doubleInvalid;
	int nRow = -1;
	int nCol = -1;
	std::unordered_map<int,int>::iterator iterRow = m_dFundAcuNvrRowMap.find( iSecurity );	//行索引
	std::unordered_map<int,int>::iterator iterCol = m_dFundAcuNvrColMap.find( iDate );
	if ( iterRow != m_dFundAcuNvrRowMap.end() && iterCol != m_dFundAcuNvrColMap.end() )
	{
		nRow = iterRow->second;
		nCol = iterCol->second;
		m_dFundAcuNvrTable.GetCell( nCol,nRow,dRet );
	}
	return dRet;
}

//--------------获取货币基金万份收益-----------------
bool	TxIndicator::GetMmfFundNvr( std::set< int > nSecurity,std::set< int > nDates, Tx::Core::Table* pResTable )
{
	if ( pResTable == NULL )
		pResTable = &m_dMmfFundNvrTable;
	pResTable->Clear();
	m_MmfFundNvrRowMap.clear();
	m_MmfFundNvrColMap.clear();

	if ( 0 == nSecurity.size()|| 0 == nDates.size())
		return false;
	std::vector<int> iSecurity( nSecurity.begin(),nSecurity.end());
	std::vector<int> iDates( nDates.begin(),nDates.end());
	pResTable->AddCol( Tx::Core::dtype_int4 );		//ID
	int nCol = 1;
	for ( std::vector< int >::iterator iter = iDates.begin(); iter != iDates.end(); ++iter )
	{
		pResTable->AddCol( Tx::Core::dtype_double );
		m_MmfFundNvrColMap.insert( std::pair<int,int>( *iter,nCol++));

	}	
	int nRow = 0;
	for ( std::vector< int >::iterator iter = iSecurity.begin(); iter != iSecurity.end(); ++iter )
	{
		pResTable->AddRow();
		pResTable->SetCell( 0,nRow,*iter );
		m_MmfFundNvrRowMap.insert( std::pair<int,int>( *iter,nRow++));
	}	
	{
		// 必然选择了单位净值
		int iSize = sizeof(int) * ( 1 /*请求类型*/ + 1 /*券个数*/ + pResTable->GetRowCount() + 1 /*日期个数*/ + iDates.size());
		LPBYTE pBuffer = new BYTE [iSize];
		if (pBuffer == NULL)
			return false;

		LPBYTE pWrite = pBuffer;
		memset(pBuffer,0,iSize);
		int iType = 3;
		memcpy_s(pWrite,iSize,&iType,sizeof(int));
		pWrite += sizeof(int);
		int nSecSize = (int)pResTable->GetRowCount();
		memcpy_s(pWrite,iSize,&nSecSize,sizeof(int));
		pWrite += sizeof(int);
		for (int i=0;i<nSecSize;i++)
		{
			int iId;
			pResTable->GetCell(0,i,iId);
			memcpy_s(pWrite,iSize,&iId,sizeof(int));
			pWrite += sizeof(int);
		}

		nSecSize = (int)iDates.size();
		memcpy_s(pWrite,iSize,&nSecSize,sizeof(int));
		pWrite += sizeof(int);

		for (int i=0;i<nSecSize;i++)
		{
			memcpy_s(pWrite,iSize,&iDates[i],sizeof(int));
			pWrite += sizeof(int);
		}
		LPCTSTR lpUrl = CORE_GETSYSINFO->GetServerAddr(_T("File"),_T("FundNavRaiseRate"));

		Tx::Drive::Http::CSyncUpload upload;
		int iStart = ::GetTickCount();
		if ( upload.Post(lpUrl, pBuffer, iSize) )
		{
			int iEnd = ::GetTickCount();
			TRACE(_T("\r\nURL Cost Time %d(ms)\r\n"),iEnd-iStart);
			CONST Tx::Drive::Mem::MemSlice &data = upload.Rsp().Body();
			LPBYTE lpRes = data.DataPtr();
			UINT nRetSize = data.Size();
			if (nRetSize <= 0)
			{
				delete pBuffer;
				pBuffer = NULL;
				return false;
			}
			UINT nPreSize = *(reinterpret_cast<UINT*>(lpRes));
			LPBYTE lpData = new BYTE[nPreSize];
			if ( lpData == NULL )
			{
				delete pBuffer;
				pBuffer = NULL;
				return false;
			}
			if ( !Tx::Drive::IO::Zip::CZipWrapper::MemUnZip(lpData, 
				nPreSize, lpRes + sizeof(UINT), nRetSize - sizeof(UINT)) )
			{
				delete []lpData;
				delete[] pBuffer;
				pBuffer = NULL;
				return false;
			}
			iStart = ::GetTickCount();
			LPBYTE pRecv = lpData;
			UINT nParseCount = pResTable->GetRowCount() * iDates.size();
			float fValue = 0.0;
			double dValue = 0.0;
			for (UINT i=0;i < nParseCount; i++)
			{
				memcpy_s(&fValue,sizeof(float),pRecv,sizeof(float));
				pRecv += sizeof(float);
				if (fabs(fValue - Tx::Core::Con_floatInvalid) < 0.00001)
					pResTable->SetCell(i%iDates.size() + 1,i/iDates.size(),Tx::Core::Con_doubleInvalid);
				else
				{
					dValue = (double)fValue;
					pResTable->SetCell(i%iDates.size() + 1,i/iDates.size(),dValue);
				}
			}
			delete []lpData;
			lpData = NULL;
			iEnd = ::GetTickCount();
			TRACE(_T("\r\nParse Result Cost Time %d(ms)\r\n"),iEnd-iStart);
		}
		else
		{
			AfxMessageBox(_T("货币基金净值网络请求返回失败!"));
		}
		delete[] pBuffer;
		pBuffer = NULL;
	}
	return true;		
}

double	TxIndicator::GetMmfFundNvrVal( int iSecurity,int iDate )
{
	double dRet = Con_doubleInvalid;
	int nRow = -1;
	int nCol = -1;
	std::unordered_map<int,int>::iterator iterRow = m_MmfFundNvrRowMap.find( iSecurity );	//行索引
	std::unordered_map<int,int>::iterator iterCol = m_MmfFundNvrColMap.find( iDate );
	if ( iterRow != m_MmfFundNvrRowMap.end() && iterCol != m_MmfFundNvrColMap.end() )
	{
		nRow = iterRow->second;
		nCol = iterCol->second;
		m_dMmfFundNvrTable.GetCell( nCol,nRow,dRet );
	}
	return dRet;
}
//--------------获取货币基金基金七日年化-----------------
bool	TxIndicator::GetMmfFundAcuNvr( std::set< int > nSecurity,std::set< int > nDates, Tx::Core::Table* pResTable )
{
	if ( pResTable == NULL )
		pResTable = &m_dMmfFundAcuNvrTable;
	pResTable->Clear();
	m_dMmfFundAcuNvrColMap.clear();
	m_dMmfFundAcuNvrRowMap.clear();
	if ( 0 == nSecurity.size()|| 0 == nDates.size() )
		return false;
	std::vector<int> iSecurity( nSecurity.begin(),nSecurity.end());
	std::vector<int> iDates( nDates.begin(),nDates.end());

	pResTable->AddCol( Tx::Core::dtype_int4 );		//ID
	int nCol = 1;
	for ( std::vector< int >::iterator iter = iDates.begin(); iter != iDates.end(); ++iter )
	{
		pResTable->AddCol( Tx::Core::dtype_double );
		m_dMmfFundAcuNvrColMap.insert( std::pair<int,int>( *iter,nCol++));
	}	
	int nRow = 0;
	for ( std::vector< int >::iterator iter = iSecurity.begin(); iter != iSecurity.end(); ++iter )
	{
		pResTable->AddRow();
		pResTable->SetCell( 0,nRow,*iter );
		m_dMmfFundAcuNvrRowMap.insert( std::pair<int,int>( *iter,nRow++));
	}	
	{
		// 必然选择了累计净值
		int iSize = sizeof(int) * ( 1 /*请求类型*/ + 1 /*券个数*/ + pResTable->GetRowCount() + 1 /*日期个数*/ + iDates.size());
		LPBYTE pBuffer = new BYTE [iSize];
		if (pBuffer == NULL)
			return false;
		LPBYTE pWrite = pBuffer;
		memset(pBuffer,0,iSize);
		int iType = 4;
		memcpy_s(pWrite,iSize,&iType,sizeof(int));
		pWrite += sizeof(int);
		int nSecSize = (int)pResTable->GetRowCount();
		memcpy_s(pWrite,iSize,&nSecSize,sizeof(int));
		pWrite += sizeof(int);
		for (int i=0;i<nSecSize;i++)
		{
			int iId;
			pResTable->GetCell(0,i,iId);
			memcpy_s(pWrite,iSize,&iId,sizeof(int));
			pWrite += sizeof(int);
		}

		nSecSize = (int)iDates.size();
		memcpy_s(pWrite,iSize,&nSecSize,sizeof(int));
		pWrite += sizeof(int);

		for (int i=0;i<nSecSize;i++)
		{
			memcpy_s(pWrite,iSize,&iDates[i],sizeof(int));
			pWrite += sizeof(int);
		}
		LPCTSTR lpUrl = CORE_GETSYSINFO->GetServerAddr(_T("File"),_T("FundNavRaiseRate"));

		Tx::Drive::Http::CSyncUpload upload;
		int iStart = ::GetTickCount();
		if ( upload.Post(lpUrl, pBuffer, iSize) )
		{
			int iEnd = ::GetTickCount();
			TRACE(_T("\r\nURL Cost Time %d(ms)\r\n"),iEnd-iStart);
			CONST Tx::Drive::Mem::MemSlice &data = upload.Rsp().Body();
			LPBYTE lpRes = data.DataPtr();
			UINT nRetSize = data.Size();
			if (nRetSize <= 0)
			{
				delete pBuffer;
				pBuffer = NULL;
				return false;
			}

			UINT nPreSize = *(reinterpret_cast<UINT*>(lpRes));
			LPBYTE lpData = new BYTE[nPreSize];
			if ( lpData == NULL )
			{
				delete pBuffer;
				pBuffer = NULL;
				return false;
			}
			if ( !Tx::Drive::IO::Zip::CZipWrapper::MemUnZip(lpData, 
				nPreSize, lpRes + sizeof(UINT), nRetSize - sizeof(UINT)) )
			{
				delete []lpData;
				delete[] pBuffer;
				pBuffer = NULL;
				return false;
			}

			iStart = ::GetTickCount();
			LPBYTE pRecv = lpData;
			UINT nParseCount = pResTable->GetRowCount() * iDates.size();
			float fValue = 0.0;
			double dValue = 0.0;
			for (UINT i=0;i < nParseCount; i++)
			{
				memcpy_s(&fValue,sizeof(float),pRecv,sizeof(float));
				pRecv += sizeof(float);
				if (fabs(fValue - Tx::Core::Con_floatInvalid) < 0.00001)
					pResTable->SetCell(i%iDates.size() + 1,i/iDates.size(),Tx::Core::Con_doubleInvalid);
				else
				{
					dValue = (double)fValue;
					pResTable->SetCell(i%iDates.size() + 1,i/iDates.size(),dValue);
				}
			}
			delete []lpData;
			lpData = NULL;
			iEnd = ::GetTickCount();
			TRACE(_T("\r\nParse Result Cost Time %d(ms)\r\n"),iEnd-iStart);
		}
		else
		{
			AfxMessageBox(_T("货币基金净值网络请求返回失败!"));
		}
		delete[] pBuffer;
		pBuffer = NULL;
	}
	return true;
}

double TxIndicator::GetMmfFundAcuNvrVal( int iSecurity,int iDate )
{
	double dRet = Con_doubleInvalid;
	int nRow = -1;
	int nCol = -1;
	std::unordered_map<int,int>::iterator iterRow = m_dMmfFundAcuNvrRowMap.find( iSecurity );	//行索引
	std::unordered_map<int,int>::iterator iterCol = m_dMmfFundAcuNvrColMap.find( iDate );
	if ( iterRow != m_dMmfFundAcuNvrRowMap.end() && iterCol != m_dMmfFundAcuNvrColMap.end() )
	{
		nRow = iterRow->second;
		nCol = iterCol->second;
		m_dMmfFundAcuNvrTable.GetCell( nCol,nRow,dRet );
	}
	return dRet;
}
//非冗余模式
//--------------获取基金单位净值ext-----------------
bool	TxIndicator::GetFundNvrExt( std::set< int > nSecurity,std::set< int > nDates, Tx::Core::Table* pResTable)
{
	if ( pResTable == NULL )
		pResTable = &m_dFundNvrExtTable;
	pResTable->Clear();
	m_FundNvrRowExtMap.clear();
	m_FundNvrColExtMap.clear();

	if ( 0 == nSecurity.size()|| 0 == nDates.size())
		return false;
	std::vector<int> iSecurity( nSecurity.begin(),nSecurity.end());
	std::vector<int> iDates( nDates.begin(),nDates.end());
	pResTable->AddCol( Tx::Core::dtype_int4 );		//ID
	int nCol = 1;
	for ( std::vector< int >::iterator iter = iDates.begin(); iter != iDates.end(); ++iter )
	{
		pResTable->AddCol( Tx::Core::dtype_double );
		m_FundNvrColExtMap.insert( std::pair<int,int>( *iter,nCol++));

	}	
	int nRow = 0;
	for ( std::vector< int >::iterator iter = iSecurity.begin(); iter != iSecurity.end(); ++iter )
	{
		pResTable->AddRow();
		pResTable->SetCell( 0,nRow,*iter );
		m_FundNvrRowExtMap.insert( std::pair<int,int>( *iter,nRow++));
	}	
	{
		// 必然选择了单位净值
		int iSize = sizeof(int) * ( 1 /*请求类型*/ + 1 /*券个数*/ + pResTable->GetRowCount() + 1 /*日期个数*/ + iDates.size());
		LPBYTE pBuffer = new BYTE [iSize];
		if (pBuffer == NULL)
			return false;

		LPBYTE pWrite = pBuffer;
		memset(pBuffer,0,iSize);
		int iType = 1;
		memcpy_s(pWrite,iSize,&iType,sizeof(int));
		pWrite += sizeof(int);
		int nSecSize = (int)pResTable->GetRowCount();
		memcpy_s(pWrite,iSize,&nSecSize,sizeof(int));
		pWrite += sizeof(int);
		for (int i=0;i<nSecSize;i++)
		{
			int iId;
			pResTable->GetCell(0,i,iId);
			memcpy_s(pWrite,iSize,&iId,sizeof(int));
			pWrite += sizeof(int);
		}

		nSecSize = (int)iDates.size();
		memcpy_s(pWrite,iSize,&nSecSize,sizeof(int));
		pWrite += sizeof(int);

		for (int i=0;i<nSecSize;i++)
		{
			memcpy_s(pWrite,iSize,&iDates[i],sizeof(int));
			pWrite += sizeof(int);
		}
		LPCTSTR lpUrl = CORE_GETSYSINFO->GetServerAddr(_T("File"),_T("FundMMF"));

		Tx::Drive::Http::CSyncUpload upload;
		int iStart = ::GetTickCount();
		if ( upload.Post(lpUrl, pBuffer, iSize) )
		{
			int iEnd = ::GetTickCount();
			TRACE(_T("\r\nURL Cost Time %d(ms)\r\n"),iEnd-iStart);
			CONST Tx::Drive::Mem::MemSlice &data = upload.Rsp().Body();
			LPBYTE lpRes = data.DataPtr();
			UINT nRetSize = data.Size();
			if (nRetSize <= 0)
			{
				delete pBuffer;
				pBuffer = NULL;
				return false;
			}
			UINT nPreSize = *(reinterpret_cast<UINT*>(lpRes));
			LPBYTE lpData = new BYTE[nPreSize];
			if ( lpData == NULL )
			{
				delete pBuffer;
				pBuffer = NULL;
				return false;
			}
			if ( !Tx::Drive::IO::Zip::CZipWrapper::MemUnZip(lpData, 
				nPreSize, lpRes + sizeof(UINT), nRetSize - sizeof(UINT)) )
			{
				delete []lpData;
				delete[] pBuffer;
				pBuffer = NULL;
				return false;
			}
			iStart = ::GetTickCount();
			LPBYTE pRecv = lpData;
			UINT nParseCount = pResTable->GetRowCount() * iDates.size();
			float fValue = 0.0;
			double dValue = 0.0;
			for (UINT i=0;i < nParseCount; i++)
			{
				memcpy_s(&fValue,sizeof(float),pRecv,sizeof(float));
				pRecv += sizeof(float);
				if (fabs(fValue - Tx::Core::Con_floatInvalid) < 0.00001)
					pResTable->SetCell(i%iDates.size() + 1,i/iDates.size(),Tx::Core::Con_doubleInvalid);
				else
				{
					dValue = (double)fValue;
					pResTable->SetCell(i%iDates.size() + 1,i/iDates.size(),dValue);
				}
			}
			delete []lpData;
			lpData = NULL;
			iEnd = ::GetTickCount();
			TRACE(_T("\r\nParse Result Cost Time %d(ms)\r\n"),iEnd-iStart);
		}
		else
		{
			AfxMessageBox(_T("基金净值网络请求返回失败!"));
		}
		delete[] pBuffer;
		pBuffer = NULL;
	}
	return true;	
}
double	TxIndicator::GetFundNvrExtVal( int iSecurity,int iDate )
{
	double dRet = Con_doubleInvalid;
	int nRow = -1;
	int nCol = -1;
	std::unordered_map<int,int>::iterator iterRow = m_FundNvrRowExtMap.find( iSecurity );	//行索引
	std::unordered_map<int,int>::iterator iterCol = m_FundNvrColExtMap.find( iDate );
	if ( iterRow != m_FundNvrRowExtMap.end() && iterCol != m_FundNvrColExtMap.end() )
	{
		nRow = iterRow->second;
		nCol = iterCol->second;
		m_dFundNvrExtTable.GetCell( nCol,nRow,dRet );
	}
	return dRet;
}
//--------------获取基金累计净值ext-----------------
bool	TxIndicator::GetFundAcuNvrExt( std::set< int > nSecurity,std::set< int > nDates, Tx::Core::Table* pResTable )
{
	if ( pResTable == NULL )
		pResTable = &m_dFundAcuNvrExtTable;
	pResTable->Clear();
	m_dFundAcuNvrColExtMap.clear();
	m_dFundAcuNvrRowExtMap.clear();
	if ( 0 == nSecurity.size()|| 0 == nDates.size() )
		return false;
	std::vector<int> iSecurity( nSecurity.begin(),nSecurity.end());
	std::vector<int> iDates( nDates.begin(),nDates.end());

	pResTable->AddCol( Tx::Core::dtype_int4 );		//ID
	int nCol = 1;
	for ( std::vector< int >::iterator iter = iDates.begin(); iter != iDates.end(); ++iter )
	{
		pResTable->AddCol( Tx::Core::dtype_double );
		m_dFundAcuNvrColExtMap.insert( std::pair<int,int>( *iter,nCol++));
	}	
	int nRow = 0;
	for ( std::vector< int >::iterator iter = iSecurity.begin(); iter != iSecurity.end(); ++iter )
	{
		pResTable->AddRow();
		pResTable->SetCell( 0,nRow,*iter );
		m_dFundAcuNvrRowExtMap.insert( std::pair<int,int>( *iter,nRow++));
	}	
	{
		// 必然选择了累计净值
		int iSize = sizeof(int) * ( 1 /*请求类型*/ + 1 /*券个数*/ + pResTable->GetRowCount() + 1 /*日期个数*/ + iDates.size());
		LPBYTE pBuffer = new BYTE [iSize];
		if (pBuffer == NULL)
			return false;
		LPBYTE pWrite = pBuffer;
		memset(pBuffer,0,iSize);
		int iType = 2;
		memcpy_s(pWrite,iSize,&iType,sizeof(int));
		pWrite += sizeof(int);
		int nSecSize = (int)pResTable->GetRowCount();
		memcpy_s(pWrite,iSize,&nSecSize,sizeof(int));
		pWrite += sizeof(int);
		for (int i=0;i<nSecSize;i++)
		{
			int iId;
			pResTable->GetCell(0,i,iId);
			memcpy_s(pWrite,iSize,&iId,sizeof(int));
			pWrite += sizeof(int);
		}

		nSecSize = (int)iDates.size();
		memcpy_s(pWrite,iSize,&nSecSize,sizeof(int));
		pWrite += sizeof(int);

		for (int i=0;i<nSecSize;i++)
		{
			memcpy_s(pWrite,iSize,&iDates[i],sizeof(int));
			pWrite += sizeof(int);
		}
		LPCTSTR lpUrl = CORE_GETSYSINFO->GetServerAddr(_T("File"),_T("FundMMF"));

		Tx::Drive::Http::CSyncUpload upload;
		int iStart = ::GetTickCount();
		if ( upload.Post(lpUrl, pBuffer, iSize) )
		{
			int iEnd = ::GetTickCount();
			TRACE(_T("\r\nURL Cost Time %d(ms)\r\n"),iEnd-iStart);
			CONST Tx::Drive::Mem::MemSlice &data = upload.Rsp().Body();
			LPBYTE lpRes = data.DataPtr();
			UINT nRetSize = data.Size();
			if (nRetSize <= 0)
			{
				delete pBuffer;
				pBuffer = NULL;
				return false;
			}

			UINT nPreSize = *(reinterpret_cast<UINT*>(lpRes));
			LPBYTE lpData = new BYTE[nPreSize];
			if ( lpData == NULL )
			{
				delete pBuffer;
				pBuffer = NULL;
				return false;
			}
			if ( !Tx::Drive::IO::Zip::CZipWrapper::MemUnZip(lpData, 
				nPreSize, lpRes + sizeof(UINT), nRetSize - sizeof(UINT)) )
			{
				delete []lpData;
				delete[] pBuffer;
				pBuffer = NULL;
				return false;
			}

			iStart = ::GetTickCount();
			LPBYTE pRecv = lpData;
			UINT nParseCount = pResTable->GetRowCount() * iDates.size();
			float fValue = 0.0;
			double dValue = 0.0;
			for (UINT i=0;i < nParseCount; i++)
			{
				memcpy_s(&fValue,sizeof(float),pRecv,sizeof(float));
				pRecv += sizeof(float);
				if (fabs(fValue - Tx::Core::Con_floatInvalid) < 0.00001)
					pResTable->SetCell(i%iDates.size() + 1,i/iDates.size(),Tx::Core::Con_doubleInvalid);
				else
				{
					dValue = (double)fValue;
					pResTable->SetCell(i%iDates.size() + 1,i/iDates.size(),dValue);
				}
			}
			delete []lpData;
			lpData = NULL;
			iEnd = ::GetTickCount();
			TRACE(_T("\r\nParse Result Cost Time %d(ms)\r\n"),iEnd-iStart);
		}
		else
		{
			AfxMessageBox(_T("基金净值网络请求返回失败!"));
		}
		delete[] pBuffer;
		pBuffer = NULL;
	}
	return true;
}
double	TxIndicator::GetFundAcuNvrExtVal( int iSecurity,int iDate )
{
	double dRet = Con_doubleInvalid;
	int nRow = -1;
	int nCol = -1;
	std::unordered_map<int,int>::iterator iterRow = m_dFundAcuNvrRowExtMap.find( iSecurity );	//行索引
	std::unordered_map<int,int>::iterator iterCol = m_dFundAcuNvrColExtMap.find( iDate );
	if ( iterRow != m_dFundAcuNvrRowExtMap.end() && iterCol != m_dFundAcuNvrColExtMap.end() )
	{
		nRow = iterRow->second;
		nCol = iterCol->second;
		m_dFundAcuNvrExtTable.GetCell( nCol,nRow,dRet );
	}
	return dRet;
}
//--------------获取货币基金万份收益ext-------------
bool	TxIndicator::GetMmfFundNvrExt( std::set< int > nSecurity,std::set< int > nDates, Tx::Core::Table* pResTable)
{
	if ( pResTable == NULL )
		pResTable = &m_dMmfFundNvrExtTable;
	pResTable->Clear();
	m_MmfFundNvrRowExtMap.clear();
	m_MmfFundNvrColExtMap.clear();

	if ( 0 == nSecurity.size()|| 0 == nDates.size())
		return false;
	std::vector<int> iSecurity( nSecurity.begin(),nSecurity.end());
	std::vector<int> iDates( nDates.begin(),nDates.end());
	pResTable->AddCol( Tx::Core::dtype_int4 );		//ID
	int nCol = 1;
	for ( std::vector< int >::iterator iter = iDates.begin(); iter != iDates.end(); ++iter )
	{
		pResTable->AddCol( Tx::Core::dtype_double );
		m_MmfFundNvrColExtMap.insert( std::pair<int,int>( *iter,nCol++));

	}	
	int nRow = 0;
	for ( std::vector< int >::iterator iter = iSecurity.begin(); iter != iSecurity.end(); ++iter )
	{
		pResTable->AddRow();
		pResTable->SetCell( 0,nRow,*iter );
		m_MmfFundNvrRowExtMap.insert( std::pair<int,int>( *iter,nRow++));
	}	
	{
		// 必然选择了单位净值
		int iSize = sizeof(int) * ( 1 /*请求类型*/ + 1 /*券个数*/ + pResTable->GetRowCount() + 1 /*日期个数*/ + iDates.size());
		LPBYTE pBuffer = new BYTE [iSize];
		if (pBuffer == NULL)
			return false;

		LPBYTE pWrite = pBuffer;
		memset(pBuffer,0,iSize);
		int iType = 3;
		memcpy_s(pWrite,iSize,&iType,sizeof(int));
		pWrite += sizeof(int);
		int nSecSize = (int)pResTable->GetRowCount();
		memcpy_s(pWrite,iSize,&nSecSize,sizeof(int));
		pWrite += sizeof(int);
		for (int i=0;i<nSecSize;i++)
		{
			int iId;
			pResTable->GetCell(0,i,iId);
			memcpy_s(pWrite,iSize,&iId,sizeof(int));
			pWrite += sizeof(int);
		}

		nSecSize = (int)iDates.size();
		memcpy_s(pWrite,iSize,&nSecSize,sizeof(int));
		pWrite += sizeof(int);

		for (int i=0;i<nSecSize;i++)
		{
			memcpy_s(pWrite,iSize,&iDates[i],sizeof(int));
			pWrite += sizeof(int);
		}
		LPCTSTR lpUrl = CORE_GETSYSINFO->GetServerAddr(_T("File"),_T("FundMMF"));

		Tx::Drive::Http::CSyncUpload upload;
		int iStart = ::GetTickCount();
		if ( upload.Post(lpUrl, pBuffer, iSize) )
		{
			int iEnd = ::GetTickCount();
			TRACE(_T("\r\nURL Cost Time %d(ms)\r\n"),iEnd-iStart);
			CONST Tx::Drive::Mem::MemSlice &data = upload.Rsp().Body();
			LPBYTE lpRes = data.DataPtr();
			UINT nRetSize = data.Size();
			if (nRetSize <= 0)
			{
				delete pBuffer;
				pBuffer = NULL;
				return false;
			}
			UINT nPreSize = *(reinterpret_cast<UINT*>(lpRes));
			LPBYTE lpData = new BYTE[nPreSize];
			if ( lpData == NULL )
			{
				delete pBuffer;
				pBuffer = NULL;
				return false;
			}
			if ( !Tx::Drive::IO::Zip::CZipWrapper::MemUnZip(lpData, 
				nPreSize, lpRes + sizeof(UINT), nRetSize - sizeof(UINT)) )
			{
				delete []lpData;
				delete[] pBuffer;
				pBuffer = NULL;
				return false;
			}
			iStart = ::GetTickCount();
			LPBYTE pRecv = lpData;
			UINT nParseCount = pResTable->GetRowCount() * iDates.size();
			float fValue = 0.0;
			double dValue = 0.0;
			for (UINT i=0;i < nParseCount; i++)
			{
				memcpy_s(&fValue,sizeof(float),pRecv,sizeof(float));
				pRecv += sizeof(float);
				if (fabs(fValue - Tx::Core::Con_floatInvalid) < 0.00001)
					pResTable->SetCell(i%iDates.size() + 1,i/iDates.size(),Tx::Core::Con_doubleInvalid);
				else
				{
					dValue = (double)fValue;
					pResTable->SetCell(i%iDates.size() + 1,i/iDates.size(),dValue);
				}
			}
			delete []lpData;
			lpData = NULL;
			iEnd = ::GetTickCount();
			TRACE(_T("\r\nParse Result Cost Time %d(ms)\r\n"),iEnd-iStart);
		}
		else
		{
			AfxMessageBox(_T("货币基金净值网络请求返回失败!"));
		}
		delete[] pBuffer;
		pBuffer = NULL;
	}
	return true;	
}
double	TxIndicator::GetMmfFundNvrExtVal( int iSecurity,int iDate )
{
	double dRet = Con_doubleInvalid;
	int nRow = -1;
	int nCol = -1;
	std::unordered_map<int,int>::iterator iterRow = m_MmfFundNvrRowExtMap.find( iSecurity );	//行索引
	std::unordered_map<int,int>::iterator iterCol = m_MmfFundNvrColExtMap.find( iDate );
	if ( iterRow != m_MmfFundNvrRowExtMap.end() && iterCol != m_MmfFundNvrColExtMap.end() )
	{
		nRow = iterRow->second;
		nCol = iterCol->second;
		m_dMmfFundNvrExtTable.GetCell( nCol,nRow,dRet );
	}
	return dRet;
}
//--------------获取基金7日年化ext------------------
bool	TxIndicator::GetMmfFundAcuNvrExt( std::set< int > nSecurity,std::set< int > nDates, Tx::Core::Table* pResTable)
{
	if ( pResTable == NULL )
		pResTable = &m_dMmfFundAcuNvrExtTable;
	pResTable->Clear();
	m_dMmfFundAcuNvrColExtMap.clear();
	m_dMmfFundAcuNvrRowExtMap.clear();
	if ( 0 == nSecurity.size()|| 0 == nDates.size() )
		return false;
	std::vector<int> iSecurity( nSecurity.begin(),nSecurity.end());
	std::vector<int> iDates( nDates.begin(),nDates.end());

	pResTable->AddCol( Tx::Core::dtype_int4 );		//ID
	int nCol = 1;
	for ( std::vector< int >::iterator iter = iDates.begin(); iter != iDates.end(); ++iter )
	{
		pResTable->AddCol( Tx::Core::dtype_double );
		m_dMmfFundAcuNvrColExtMap.insert( std::pair<int,int>( *iter,nCol++));
	}	
	int nRow = 0;
	for ( std::vector< int >::iterator iter = iSecurity.begin(); iter != iSecurity.end(); ++iter )
	{
		pResTable->AddRow();
		pResTable->SetCell( 0,nRow,*iter );
		m_dMmfFundAcuNvrRowExtMap.insert( std::pair<int,int>( *iter,nRow++));
	}	
	{
		// 必然选择了累计净值
		int iSize = sizeof(int) * ( 1 /*请求类型*/ + 1 /*券个数*/ + pResTable->GetRowCount() + 1 /*日期个数*/ + iDates.size());
		LPBYTE pBuffer = new BYTE [iSize];
		if (pBuffer == NULL)
			return false;
		LPBYTE pWrite = pBuffer;
		memset(pBuffer,0,iSize);
		int iType = 4;
		memcpy_s(pWrite,iSize,&iType,sizeof(int));
		pWrite += sizeof(int);
		int nSecSize = (int)pResTable->GetRowCount();
		memcpy_s(pWrite,iSize,&nSecSize,sizeof(int));
		pWrite += sizeof(int);
		for (int i=0;i<nSecSize;i++)
		{
			int iId;
			pResTable->GetCell(0,i,iId);
			memcpy_s(pWrite,iSize,&iId,sizeof(int));
			pWrite += sizeof(int);
		}

		nSecSize = (int)iDates.size();
		memcpy_s(pWrite,iSize,&nSecSize,sizeof(int));
		pWrite += sizeof(int);

		for (int i=0;i<nSecSize;i++)
		{
			memcpy_s(pWrite,iSize,&iDates[i],sizeof(int));
			pWrite += sizeof(int);
		}
		LPCTSTR lpUrl = CORE_GETSYSINFO->GetServerAddr(_T("File"),_T("FundMMF"));

		Tx::Drive::Http::CSyncUpload upload;
		int iStart = ::GetTickCount();
		if ( upload.Post(lpUrl, pBuffer, iSize) )
		{
			int iEnd = ::GetTickCount();
			TRACE(_T("\r\nURL Cost Time %d(ms)\r\n"),iEnd-iStart);
			CONST Tx::Drive::Mem::MemSlice &data = upload.Rsp().Body();
			LPBYTE lpRes = data.DataPtr();
			UINT nRetSize = data.Size();
			if (nRetSize <= 0)
			{
				delete pBuffer;
				pBuffer = NULL;
				return false;
			}

			UINT nPreSize = *(reinterpret_cast<UINT*>(lpRes));
			LPBYTE lpData = new BYTE[nPreSize];
			if ( lpData == NULL )
			{
				delete pBuffer;
				pBuffer = NULL;
				return false;
			}
			if ( !Tx::Drive::IO::Zip::CZipWrapper::MemUnZip(lpData, 
				nPreSize, lpRes + sizeof(UINT), nRetSize - sizeof(UINT)) )
			{
				delete []lpData;
				delete[] pBuffer;
				pBuffer = NULL;
				return false;
			}

			iStart = ::GetTickCount();
			LPBYTE pRecv = lpData;
			UINT nParseCount = pResTable->GetRowCount() * iDates.size();
			float fValue = 0.0;
			double dValue = 0.0;
			for (UINT i=0;i < nParseCount; i++)
			{
				memcpy_s(&fValue,sizeof(float),pRecv,sizeof(float));
				pRecv += sizeof(float);
				if (fabs(fValue - Tx::Core::Con_floatInvalid) < 0.00001)
					pResTable->SetCell(i%iDates.size() + 1,i/iDates.size(),Tx::Core::Con_doubleInvalid);
				else
				{
					dValue = (double)fValue;
					pResTable->SetCell(i%iDates.size() + 1,i/iDates.size(),dValue);
				}
			}
			delete []lpData;
			lpData = NULL;
			iEnd = ::GetTickCount();
			TRACE(_T("\r\nParse Result Cost Time %d(ms)\r\n"),iEnd-iStart);
		}
		else
		{
			AfxMessageBox(_T("货币基金净值网络请求返回失败!"));
		}
		delete[] pBuffer;
		pBuffer = NULL;
	}
	return true;
}
double	TxIndicator::GetMmfFundAcuNvrExtVal( int iSecurity,int iDate )
{
	double dRet = Con_doubleInvalid;
	int nRow = -1;
	int nCol = -1;
	std::unordered_map<int,int>::iterator iterRow = m_dMmfFundAcuNvrRowExtMap.find( iSecurity );	//行索引
	std::unordered_map<int,int>::iterator iterCol = m_dMmfFundAcuNvrColExtMap.find( iDate );
	if ( iterRow != m_dMmfFundAcuNvrRowExtMap.end() && iterCol != m_dMmfFundAcuNvrColExtMap.end() )
	{
		nRow = iterRow->second;
		nCol = iterCol->second;
		m_dMmfFundAcuNvrExtTable.GetCell( nCol,nRow,dRet );
	}
	return dRet;
}
CString TxIndicator::Text2Date(CString m_str)
{
	int i = m_str.GetLength();
	if( i < 0)
		return NULL;
	CString m_strDate  = _T("");
	int m_iDate = atoi(m_str);
	if(i != 8 )
	{
		m_strDate = _T("");
		return NULL;
	}
	if(m_iDate < 19790101 || m_iDate >20501231)
	{
		m_strDate = _T("");
		return NULL;
	}
	CString m_strDay = _T("");
	CString m_strMonth = _T("");
	int m_iDay = 0;
	int m_iMonth = 0;
	m_iDay = m_iDate%100;
	m_iMonth = (m_iDate/100)%100;
	if(m_iDay < 10)
		m_strDay.Format("0%d",m_iDay);
	else
		m_strDay.Format("%d",m_iDay);
	if(m_iMonth < 10)
		m_strMonth.Format("0%d",m_iMonth);
	else
		m_strMonth.Format("%d",m_iMonth);
	m_strDate.Format("%d",m_iDate/10000);
	m_strDate = m_strDate + _T("-") + m_strMonth + _T("-") + m_strDay;
	return m_strDate;
}

CString TxIndicator::Date2Text(CString m_str)
{
	CString m_strDate =_T("");
	int i = m_str.GetLength();
	if( i < 1)
		return NULL;
	int m_iYear = 0;
	int m_iMonth = 0;
	int m_iDay = 0;
	int iFirst = 0;
	int iLast = m_str.Find(_T('-'), iFirst);
	m_iYear = _ttoi(m_str.Mid(iFirst,iLast - iFirst));
	iFirst = iLast + 1;
	iLast = m_str.Find(_T('-'), iFirst);
	m_iMonth = _ttoi(m_str.Mid(iFirst,iLast - iFirst));
	iFirst = iLast + 1;
	m_iDay = _ttoi(m_str.Mid(iFirst,i - iFirst));
	if(m_iYear < 1 || m_iMonth < 1 || m_iDay < 1 || m_iMonth >12 || m_iDay > 31)
	{
		m_strDate = _T("");
		return NULL;
	}
	int m_iDate = 0;
	m_iDate = m_iYear*10000 + m_iMonth*100 + m_iDay;
	m_strDate.Format("%d",m_iDate);
	return m_strDate;
}

CString TxIndicator::GetDateSeries( int nSecurity, int nBegin, int nEnd, int nCycle, int nStyle )
{
	CString strRet = _T("");
	std::vector<int>	vRet;
	vRet.clear();
	Tx::Business::TxBusiness business;
	business.GetSecurityNow( nSecurity );
	if ( business.m_pSecurity == NULL )
		return strRet;
	business.m_pSecurity->LoadHisTrade(true);
	business.m_pSecurity->LoadTradeDate();
	//根据日期方式确认
	switch( nCycle )
	{
	case 0://day
		business.m_pSecurity->GetDate( nBegin, nEnd, 0, vRet, 0 );
		break;
	case 1://week
		business.m_pSecurity->GetDate( nBegin, nEnd, 1, vRet, 0 );
		break;
	case 2://month
		business.m_pSecurity->GetDate( nBegin, nEnd, 2, vRet, 0 );
		break;
	case 3://quarter
		business.m_pSecurity->GetDate( nBegin, nEnd, 3, vRet, 0 );
		break;
	case 4:
		business.m_pSecurity->GetDate( nBegin, nEnd, 6, vRet, 0 );
		break;
	case 5:
		business.m_pSecurity->GetDate( nBegin, nEnd, 4, vRet, 0 );
		break;
	default:
		break;
	}
	CString str;
	switch( nStyle )
	{
	case 0:
		for ( std::vector<int>::iterator iter = vRet.begin(); iter != vRet.end(); ++iter)
		{
			str.Format(_T("%d\t"),*iter );
			strRet += str; 
		}
		break;
	case 1:
		for ( std::vector<int>::iterator iter = vRet.begin(); iter != vRet.end(); ++iter)
		{
			str.Format(_T("%d-%d-%d\t"),*iter/10000,*iter%10000/100,*iter%100 );
			strRet += str; 
		}
		break;
	case 2:
		for ( std::vector<int>::iterator iter = vRet.begin(); iter != vRet.end(); ++iter)
		{
			str.Format(_T("%d/%d/%d\t"),*iter/10000,*iter%10000/100,*iter%100 );
			strRet += str; 
		}
		break;
	default:
		break;
	}
	return strRet;
}

bool	TxIndicator::SetSynStyle( bool bSynFalg )
{
	m_bSynFlag = bSynFalg;
	return true;
}

bool	TxIndicator::GetSynStyle( void )
{
	return m_bSynFlag;
}

CString TxIndicator::GetLatestReport( CString strRequest )
{
	CString strResult = _T("");
	//解析请求串
	CString strSecurity = _T("");
	CString strTable = _T("");
	CString strDate = _T("");
	CString strItem = _T("");

	//校验有效
	std::vector< CString > arrOut;
	Tx::Core::Commonality::GetInstance()->StrFunc.Split(strRequest,arrOut, _T(";"),false);
	if ( arrOut.size() < 3 )
		return strResult;
	strSecurity = arrOut[0].Trim();
	strTable = arrOut[1].Trim();
	strItem = arrOut[2].Trim();
	if ( arrOut.size() == 3 )
		strDate = _T("");
	else
		strDate = arrOut[3].Trim();
	if ( strSecurity.GetLength() <= 0 || strTable.GetLength() <= 0|| strItem.GetLength() <= 0 )
		return Con_strInvalid;
	//拼接请求串
	OleInitialize(NULL);
	CXmlDocumentWrapper doc;
	doc.LoadXML(_T("<?xml version=\"1.0\" encoding=\"utf-8\"?><root></root>"));
	CXmlNodeWrapper root(doc.AsNode());	
	CXmlNodeWrapper nodeTable(root.InsertNode(0,_T("Data")));
	nodeTable.SetValue(_T("ID"),strSecurity );
	nodeTable.SetValue(_T("TableName"),strTable);
	nodeTable.SetValue(_T("Date"),strDate);
	CXmlNodeWrapper nodeItem(nodeTable.InsertNode(0,strItem));
	CString strReq = doc.GetXML();
	char tempch[200];
	CString sPath = Tx::Core::Manage::GetInstance()->m_pSystemPath->GetConfigExcelPath() + _T("\\common.ini");
	GetPrivateProfileString( _T("Report"),_T("Address"),_T("http://007webdata.chinaciia.com/CommentDataService/ServerHandler.ashx"),tempch,200,sPath );
	CString strAddress(tempch);
	//发送请求，接受返回
	CString strRes = DownLoad( strReq,strAddress );
	if ( strRes.GetLength() <= 0 )
	{
		OleUninitialize();
		return Con_strInvalid;
	}
	CXmlDocumentWrapper docRes;
	docRes.LoadXML(strRes);
	CXmlNodeWrapper resRoot(docRes.AsNode());
	ASSERT( resRoot.IsValid());
	CXmlNodeWrapper resTable(resRoot.GetNode( 0 ));
	ASSERT( resTable.IsValid());
	CXmlNodeWrapper resItem(resTable.GetNode( 0 ));
	strResult = resItem.GetText();
	OleUninitialize();
	if ( strResult.GetLength() > 0 )
		return strResult;
	else
		return Con_strInvalid;
}
CString	TxIndicator::DownLoad( CString strReq,CString strAddress )
{
	if ( strReq.GetLength() <= 0 )
		return _T("");
	//下载文件
	LPCWSTR lpSrc = NULL;
#ifdef _UNICODE
	lpSrc = strReq;
#else
	USES_CONVERSION;
	lpSrc = A2W(strReq);
#endif
	int iLen = (INT)wcslen(lpSrc) * sizeof(wchar_t);
	LPBYTE lpPost = (LPBYTE)lpSrc;
	UINT nPostSize = iLen;
	CString strTemp = strAddress;
	LPCTSTR lpUrl = (LPCTSTR)strTemp;
	Tx::Drive::Http::CSyncUpload upload;
	if ( !upload.Post(lpUrl, lpPost, nPostSize, NULL) )	
		return _T("");
	Tx::Drive::Mem::MemSlice dnData = upload.Rsp().Body();
	if ( dnData.IsEmpty() )
		return _T("");
	LPBYTE lpData = dnData.DataPtr();
	UINT preSize = dnData.Size();

	UINT nBufLen = preSize + sizeof(wchar_t);
	UINT nCount = nBufLen / sizeof(wchar_t);		
	CStringW strRes;
	LPWSTR lpBuf = (LPWSTR)strRes.GetBuffer(nBufLen);
	if ( lpBuf != NULL )
	{
		wcsncpy_s(lpBuf, nCount, (LPWSTR)lpData, nCount - 1);
		dnData.Dispose();
		lpBuf[nCount - 1] = 0;
		strRes.ReleaseBuffer(nBufLen);
		CString strTemp(lpBuf);
		return strTemp;
	}
	dnData.Dispose();
	return _T("");
}

int		TxIndicator::GetTradeDetailCount( int iSecurity,int iDate )
{
	Tx::Business::TxBusiness business;
	business.GetSecurityNow( iSecurity );
	if ( business.m_pSecurity == NULL )
		return -1;
	Tx::Data::TradeDetail* pDetail = business.m_pSecurity->GetTradeDetail( iDate );
	if ( pDetail == NULL )
		return -1;
	return pDetail->GetTrdaeDetailDataCount();
}

Tx::Data::TradeDetailData* TxIndicator::GetTradeDetailData( int iSecurity,int iDate,int iSeq )
{
	if ( GetTradeDetailCount( iSecurity,iDate ) < iSeq + 1 )
		return NULL;
	Tx::Business::TxBusiness business;
	business.GetSecurityNow( iSecurity );
	if(business.m_pSecurity==NULL)
		return NULL;
	Tx::Data::TradeDetail* pDetail = business.m_pSecurity->GetTradeDetail( iDate );
	Tx::Data::TradeDetailData* p = pDetail->GetTrdaeDetailData( iSeq );
	return p;
}

CString TxIndicator::Test( CString strReq )
{
	Tx::Data::TradeDetailData*  p = GetTradeDetailData( 1,20090326,5 );
	return _T("");
}

bool	TxIndicator::GetExcelConfig(void)
{
	CString sPath = Tx::Core::Manage::GetInstance()->m_pUserInfo->GetUserIni();
	CString strSection = _T("Excel");
	memset(&m_Cfg,0,sizeof(ExcelCfg));
	m_Cfg.invalidFormat = (int)GetPrivateProfileInt( strSection,_T("Invalid"),1,sPath);
	m_Cfg.dateFormat = (int)GetPrivateProfileInt( strSection,_T("Date"),0,sPath);
	m_Cfg.calculateStyle = (int)GetPrivateProfileInt( strSection,_T("Calculate"),0,sPath);
	return true;
}
bool	TxIndicator::SavaExcelConfig(void)
{
	CString sPath = Tx::Core::Manage::GetInstance()->m_pUserInfo->GetUserIni();
	CString strSection = _T("Excel");
	CString strValue = _T("");
	strValue.Format(_T("%d"),m_Cfg.invalidFormat );
	WritePrivateProfileString( strSection,_T("Invalid"),strValue,sPath);
	strValue.Format(_T("%d"),m_Cfg.dateFormat );
	WritePrivateProfileString( strSection,_T("Date"),strValue,sPath);
	strValue.Format(_T("%d"),m_Cfg.calculateStyle );
	WritePrivateProfileString( strSection,_T("Calculate"),strValue,sPath);
	return true;
}
CString	TxIndicator::GetXmlData( CString strFineName,CString strCol,CString strKey ,CString strCol1)
{
	CString strRet = _T("");
	OleInitialize(NULL);
	CXmlDocumentWrapper doc;
	if ( doc.Load((LPCTSTR)strFineName))
	{
		CXmlNodeWrapper root(doc.AsNode());
		if ( root.IsValid())
		{
			CString strSearch;
			if ( strKey.GetLength() == 0 )
			{
				strSearch.Format(_T("//tr"));
				CXmlNodeWrapper node(root.FindNode((LPCTSTR)strSearch));
				if ( node.IsValid())
				{
					CXmlNodeWrapper retNode(node.FindNode((LPCTSTR)strCol));
					if ( retNode.IsValid())
					{
						strRet = retNode.GetText();
					}
				}
			}
			else
			{
				strSearch.Format(_T("//tr[%s='%s']"),strCol1,strKey);
				CXmlNodeWrapper node(root.FindNode((LPCTSTR)strSearch));
				if (node.IsValid())
				{
					CXmlNodeWrapper retNode(node.FindNode((LPCTSTR)strCol));
					if ( retNode.IsValid())
						strRet = retNode.GetText();
				}
			}
		}

	}
	OleUninitialize();
	return strRet;
}

void TxIndicator::GetBondYTM(void)
{
	m_AllYtmMap.clear();

	CString strSvrList;
	strSvrList.Format(_T("%sServerList%d.ini"),Tx::Core::SystemPath::GetInstance()->GetExePath(),Tx::Core::UserInfo::GetInstance()->GetNetId());//
	if (false == Tx::Core::Commonality::File().IsFileExist (strSvrList))
		return;

	char tempch[200];
	memset(tempch,0,200);
	GetPrivateProfileString(_T("File"), _T("BondYTM"), _T(""), tempch, 200, strSvrList);

	CString strUrl = _T("%sBond/BondReal/0");
	strUrl.Format(_T("%sBond/BondReal/0"),tempch);

	CString strHeader = _T("Content-Type: application/xml; charset=utf-8\n");
	strHeader += _T("Accept: text/html, application/xhtml+xml, */*\n");
	strHeader += _T("Accept-Encoding: gzip, deflate\n");


	Tx::Drive::Http::CSyncDnload upload;
	if (upload.Open(strUrl,strHeader))
	{
		CONST Tx::Drive::Mem::MemSlice &dnData = upload.Rsp().Body();
		if (!upload.Rsp().IsSuccess() || dnData.IsEmpty() || dnData.Size()<=1)
		{	
			return;
		}

		CString strPath = SystemPath::GetInstance()->GetTempPath() + _T("\\YTM.xml");

		if (0 != Tx::Drive::IO::Zip::CZipWrapper::UnGzipToFile(dnData.DataPtr(),dnData.Size(),strPath))
		{
			return;
		}

		OleInitialize(NULL);
		Tx::Data::CXmlDocumentWrapper xmlDoc;

		/*2 读取XML*/
		if (!xmlDoc.Load(strPath))
		{
			TRACE(_T("fail to load xml: %s.\r\n"), _T("excelytm"));
			OleUninitialize();
			return;
		}

		/*3得到根结点*/
		Tx::Data::CXmlNodeWrapper root(xmlDoc.AsNode());
		if (!root.IsValid())
		{
			OleUninitialize();
			return;
		}

		for (int i=0;i<root.NumNodes();i++)
		{
			Tx::Data::BondYTM  bondYtm;
			Tx::Data::CXmlNodeWrapper nodeItem = root.GetNode(i);
			if (nodeItem.IsValid())
			{
				Tx::Data::CXmlNodeWrapper node = nodeItem.FindNode(_T("transID"));
				if (node.IsValid())
					bondYtm.object = atoi(node.GetText());
				else
					continue;

				node = nodeItem.FindNode(_T("ytm"));
				if(node.IsValid())
					bondYtm.fYtm = atof(node.GetText());

				node = nodeItem.FindNode(_T("duration"));
				if(node.IsValid())
					bondYtm.fDur = atof(node.GetText());
				
				node = nodeItem.FindNode(_T("mdDuration"));
				if(node.IsValid())
					bondYtm.fADur = atof(node.GetText());

				node = nodeItem.FindNode(_T("convexity"));
				if(node.IsValid())
					bondYtm.fCon = atof(node.GetText());

				m_AllYtmMap.insert(std::pair<int,BondYTM>(bondYtm.object,bondYtm));
			}
		}

		OleUninitialize();
	}
}

void TxIndicator::GetBondYTM(int nSecurityId,double &dytm,double &dDur,double &dMDur,double &dCon,bool bCur)
{
	std::unordered_map<int,BondYTM>::iterator iter = m_AllYtmMap.find(nSecurityId);

	if (bCur && iter!=m_AllYtmMap.end())
	{
		dytm = iter->second.fYtm;
		dDur = iter->second.fDur;
		dMDur = iter->second.fADur;
		dCon = iter->second.fCon;
	}
	else
	{
		dytm = Con_doubleInvalid;
		dDur = Con_doubleInvalid;
		dMDur = Con_doubleInvalid;
		dCon = Con_doubleInvalid;
	}
}

//最新五挡买卖盘到期收益率
//std::unordered_map<int,double> m_mmpytm[10];
void TxIndicator::GetMmpBondYtm(int no,bool mmp) //no：盘号1,2,3,4,5；mmp:true-买盘，flase-买盘；
{
	int listNo = 0;
	if (mmp) //买盘
		m_bpytm[no-1].clear();
	else  //卖盘
		m_spytm[no-1].clear();

	CString strSvrList;
	strSvrList.Format(_T("%sServerList%d.ini"),Tx::Core::SystemPath::GetInstance()->GetExePath(),Tx::Core::UserInfo::GetInstance()->GetNetId());//
	if (false == Tx::Core::Commonality::File().IsFileExist (strSvrList))
		return;

	char tempch[200];
	memset(tempch,0,200);
	GetPrivateProfileString(_T("File"), _T("BondYTM"), _T(""), tempch, 200, strSvrList);

	CString strUrl;
	if (mmp)//买盘
		strUrl.Format(_T("%sBond/bondrealmmp/?market=0&mmpflag=1&no=%d"),tempch,no);
	else //卖盘
		strUrl.Format(_T("%sBond/bondrealmmp/?market=0&mmpflag=2&no=%d"),tempch,no);


	CString strHeader = _T("Content-Type: application/xml; charset=utf-8\n");
	strHeader += _T("Accept: text/html, application/xhtml+xml, */*\n");
	strHeader += _T("Accept-Encoding: gzip, deflate\n");


	Tx::Drive::Http::CSyncDnload upload;
	if (upload.Open(strUrl,strHeader))
	{
		CONST Tx::Drive::Mem::MemSlice &dnData = upload.Rsp().Body();
		if (!upload.Rsp().IsSuccess() || dnData.IsEmpty() || dnData.Size()<=1)
		{	
			return;
		}

		CString strPath = SystemPath::GetInstance()->GetTempPath() + _T("\\MmpYTM.xml");

		if (0 != Tx::Drive::IO::Zip::CZipWrapper::UnGzipToFile(dnData.DataPtr(),dnData.Size(),strPath))
		{
			return;
		}

		OleInitialize(NULL);
		Tx::Data::CXmlDocumentWrapper xmlDoc;

		/*2 读取XML*/
		if (!xmlDoc.Load(strPath))
		{
			TRACE(_T("fail to load xml: %s.\r\n"), _T("excelmmpytm"));
			OleUninitialize();
			return;
		}

		/*3得到根结点*/
		Tx::Data::CXmlNodeWrapper root(xmlDoc.AsNode());
		if (!root.IsValid())
		{
			OleUninitialize();
			return;
		}

		for (int i=0;i<root.NumNodes();i++)
		{
			int transId=-1;
			double dValue = Con_doubleInvalid;
			Tx::Data::CXmlNodeWrapper nodeItem = root.GetNode(i);
			if (nodeItem.IsValid())
			{
				Tx::Data::CXmlNodeWrapper node = nodeItem.FindNode(_T("transID"));
				if (node.IsValid())
					transId = atoi(node.GetText());

				if (mmp)
					node = nodeItem.FindNode(_T("YP"));
				else
					node = nodeItem.FindNode(_T("SP"));
				
				if(node.IsValid())
					dValue = atof(node.GetText());
				if (mmp)
					m_bpytm[no-1].insert(std::pair<int,double>(transId,dValue));
				else
					m_spytm[no-1].insert(std::pair<int,double>(transId,dValue));
			}
		}

		OleUninitialize();
	}
}

double TxIndicator::GetMmpBondYtm(int iSecurityId,int no,bool mmp )
{
	std::unordered_map<int,double>::iterator iter;
	if (mmp) //买盘
	{
		iter = m_bpytm[no-1].find(iSecurityId);
		if (iter != m_bpytm[no-1].end())
			return iter->second;
	}
	else   //卖盘
	{
		iter = m_spytm[no-1].find(iSecurityId);
		if (iter != m_spytm[no-1].end())
			return iter->second;
	}

	return -1;
}


	}
}