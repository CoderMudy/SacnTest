/**************************************************************
	Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
	File name:		TxBusiness.cpp
	Author:			赵宏俊
	Date:			2007-07-03
	Version:		1.0
	Description:	业务类的基类
					集中处理各类交易实体具有共同属性的业务
					为各类交易实体提供基本支持
***************************************************************/
#include "StdAfx.h"
#include "TxBusiness.h"
#include "..\..\Core\Driver\TxFileUpdateEngine.h"
#include "..\..\core\driver\ClientFileEngine\public\LogRecorder.h"

#include "..\..\Core\Control\Manage.h"
#include "..\..\Data\SecurityExtentInfo.h"


#include <unordered_map>
#include <vector>
#include <algorithm>
//#include "..\..\Core\Core\Commonality.h"
#include "TxBond.h"

#include "../../Core/Driver/ClientFileEngine/base/HttpDnAndUp.h"
#include "../../Core/Driver/ClientFileEngine/base/zip/ZipWrapper.h"
#include "TxIndex.h"

#define TX_INT_YI 100000000 


namespace Tx
{
	namespace Business
	{

IMPLEMENT_DYNCREATE(TxBusiness, CObject)
TxBusiness::TxBusiness(void)
{
	m_pSecurity = NULL;
	m_pIndicatorData = NULL;
	m_pFunctionDataManager = NULL;
	m_pLogicalBusiness = NULL;
	m_pMixTradeDay = NULL;
	m_pShIndex = NULL;
	m_pSzIndex = NULL;

	m_pFunctionDataManager = new FunctionDataManager;
	m_pLogicalBusiness = new LogicalBusiness;
	m_iSecurityTypeId = 0;
	//20070711的汇率
	m_dDollar2RMB_ratio = 7.59;

	//取得上证指数的指针
	GetSecurityNow(4000208);
	m_pShIndex = m_pSecurity;

	//深市成份指数的指针
	GetSecurityNow(4000001);
	m_pSzIndex = m_pSecurity;
	// 将当前最新行情日期，改为上证指数的行情日期
//	Tx::Core::Manage::GetInstance()->m_pSystemInfo->SetCurHqDate (m_pShIndex->GetTradeDateLatest());
}
//2008-01-07
//取得两市场交易日期序列
bool TxBusiness::GetHuShenTradeDate(void)
{
	if(m_pShIndex==NULL)
		return false;
	if(m_pSzIndex==NULL)
		return false;

	if(m_pMixTradeDay!=NULL)
		return true;
	m_pMixTradeDay = new MixTradeDay;

	//构建两市场交易日期序列
	std::set<int> setMarketId;
	setMarketId.insert((int)m_pShIndex->GetId());
	setMarketId.insert((int)m_pSzIndex->GetId());

	return m_pMixTradeDay->bLoadTradeDate(setMarketId);
}

TxBusiness::~TxBusiness(void)
{
	Clear();
}
//2008-04-14
//取得指数样本涨跌数量
bool TxBusiness::GetIndexSamplesStatus(int iIndexSecurityId,int& iUp,int& iDown,int& iEqual)
{
	SecurityQuotation* p = GetSecurityNow(iIndexSecurityId);
	return GetIndexSamplesStatus(p,iUp,iDown,iEqual);
}
bool TxBusiness::GetIndexSamplesStatus(SecurityQuotation* p,int& iUp,int& iDown,int& iEqual)
{
	iUp = 0;
	iDown = 0;
	iEqual = 0;

	if(p==NULL)
		return false;
	if(p->IsIndex()==false)
		return false;
	int iSamplesCount = p->GetIndexConstituentDataCount();
	if(iSamplesCount<=0)
		return false;
	for(int i=0;i<iSamplesCount;i++)
	{
		IndexConstituentData* pData = p->GetIndexConstituentDataByIndex(i);
		if(pData==NULL)
			continue;
		SecurityQuotation* ps = GetSecurityNow(pData->iSecurityId);
		if(ps==NULL)
			continue;
		switch(ps->GetRaiseFlag())
		{
		case 1:
			iUp++;
			break;
		case 0:
			iEqual++;
			break;
		case -1:
			iDown++;
			break;
		}
	}
	return true;
}
bool TxBusiness::GetIndexSamplesStatus(SecurityQuotation* p,int& iUp,int& iDown,int& iEqual,int& iTop,int& iBottom,int& iHalt,bool bGetHalt)
{
DWORD ts=GetTickCount();
	iTop = 0;
	iBottom = 0;
	if(bGetHalt==true)
		iHalt = 0;

	iUp = 0;
	iDown = 0;
	iEqual = 0;

	if(p==NULL)
		return false;
	if(p->IsIndex()==false)
		return false;
	int iSamplesCount = p->GetIndexConstituentDataCount();
	if(iSamplesCount<=0)
		return false;
	for(int i=0;i<iSamplesCount;i++)
	{
		IndexConstituentData* pData = p->GetIndexConstituentDataByIndex(i);
		if(pData==NULL)
			continue;
		SecurityQuotation* ps = GetSecurityNow(pData->iSecurityId);
		if(ps==NULL)
			continue;
		//2008-12-04
		if(bGetHalt==true)
		{
			//if(	ps->IsHaltLong()==true
			//	//||ps->IsHalt()==true
			//  )
			//	iHalt++;
			if(ps->IsStop()==false && ps->IsIssued()==false)
			{
				if(!(ps->GetVolume()>0) || !(ps->GetAmount()>0))
				{
					if(p->GetId()==4000208)
					{
						//TRACE(_T("\n[%s-%s]-[%s-%s-%d]"),
						//	p->GetName(),
						//	p->GetCode(true),
						//	ps->GetName(),
						//	ps->GetCode(true),
						//	ps->GetId()
						//	);
					}
					iHalt++;
					continue;
				}
			}
			else
				continue;
		}
		switch(ps->GetRaiseFlag())
		{
		case 1:
			iUp++;
			break;
		case 0:
			iEqual++;
			break;
		case -1:
			iDown++;
			break;
		}
		switch(ps->HighStatus())
		{
		case 1:
			iTop++;
			break;
		case 0:
			break;
		case -1:
			iBottom++;
			break;
		}
		//MSG msg;
		//while(::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) == TRUE)
		//	::SendMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
	}

TRACE("\nTxBusiness::GetIndexSamplesStatus()=%d",GetTickCount()-ts);

	return true;
}

bool TxBusiness::GetSecurityHisTrade(int iSecurityId)
{
	//
	GetSecurityNow(iSecurityId);
	return GetSecurityHisTrade(m_pSecurity);
}
bool TxBusiness::GetSecurityHisTrade(Security* p)
{
	if(p==NULL || p->LoadHisTrade()==false)
		return false;
	return true;
}
bool TxBusiness::GetSecurityTradeDate(int iSecurityId)
{
	//
	GetSecurityNow(iSecurityId);
	return GetSecurityTradeDate(m_pSecurity);
}
bool TxBusiness::GetSecurityTradeDate(Security* p)
{
	//如果从交易实体的行情数据中拆分交易日期
	//必须在此之前调用GetSecurityHisTrade
	if(p==NULL || p->LoadTradeDate()==false)
		return false;
	return true;
}

CString TxBusiness::DownloadXmlFile(int iSecurityId,int iFunctionId)
{
	//return (CTxFileUpdateEngine::GetInstance(CTxFileUpdateEngine::modeClient))->GetXmlFile(iSecurityId,iFunctionId)==FALSE?false:true;
	//2007-07-10 章提出需要返回全路径文件名；
	return (CTxFileUpdateEngine::GetInstance(CTxFileUpdateEngine::modeClient))->GetXmlFile(iSecurityId,iFunctionId);
}

//设置样本
bool TxBusiness::SetBlockSamples(Table_Display& baTable,std::vector<int>& arrSamples,int iCol,int iInitNo)
{
	if(baTable.GetColCount()<=0)
		return false;

	int count = (int)arrSamples.size();
	if(count<=0)
		return false;
#ifdef _DEBUG
	GlobalWatch::_GetInstance()->WatchHere(_T("zhangw||before del"));
#endif
	//2008-04-14
	baTable.DeleteRow(0,baTable.GetRowCount());
	baTable.AddRow(count);
#ifdef _DEBUG
	GlobalWatch::_GetInstance()->WatchHere(_T("zhangw||after del"));
#endif

	int i = 0;
	for (std::vector<int>::iterator iter = arrSamples.begin(); iter != arrSamples.end(); iter++, i++)
	{
		baTable.SetCell(iCol,i,*iter);
		baTable.SetCell(iInitNo,i,i+1);
	}
#ifdef _DEBUG
	GlobalWatch::_GetInstance()->WatchHere(_T("zhangw||after set sample"));
#endif
	return true;
}
//排序分析数据结果标题列
//默认按照股票处理
bool TxBusiness::GetBlockAnalysisCol(Table_Display& baTable,std::vector<int>& arrSamples,int& iSortCol)
{
	iSortCol = 2;

	//如果UI没有输入列信息，自行添加
	if(baTable.GetColCount()<=0)
	{
		//加入样本for test
		//交易实体ID
		baTable.AddCol(Tx::Core::dtype_int4);
		//交易实体名称
		baTable.AddCol(Tx::Core::dtype_val_string);
		//交易实体外码
		baTable.AddCol(Tx::Core::dtype_val_string);
		//2008-02-19
		//样本原始顺序
		baTable.AddCol(Tx::Core::dtype_int4);

		//2007-08-23
		//增加 现量=最新成交量
		//加入列for test
		//前收、现价、涨跌、涨幅、换手率、现量、成交量、成交金额、流通股本、总股本、流通市值、总股市值
		//007：排序分析中，顺利为：涨幅、现价、钱收、涨跌 2007-12-12
		baTable.AddCol(Tx::Core::dtype_double);
		baTable.AddCol(Tx::Core::dtype_float);
		baTable.AddCol(Tx::Core::dtype_float);
		baTable.AddCol(Tx::Core::dtype_double);
		baTable.AddCol(Tx::Core::dtype_double);
		baTable.AddCol(Tx::Core::dtype_double);
		baTable.AddCol(Tx::Core::dtype_double);
		baTable.AddCol(Tx::Core::dtype_double);

		baTable.AddCol(Tx::Core::dtype_double);
		baTable.AddCol(Tx::Core::dtype_double);
		baTable.AddCol(Tx::Core::dtype_double);
		baTable.AddCol(Tx::Core::dtype_double);

		int nCol=1;
		baTable.SetTitle(nCol, _T("名称"));
		++nCol;
		baTable.SetTitle(nCol, _T("代码"));
		++nCol;
		baTable.SetTitle(nCol, _T("自选号"));
		++nCol;
		baTable.SetTitle(nCol, _T("涨幅"));
		baTable.SetKeyCol(nCol);
		++nCol;
		baTable.SetTitle(nCol, _T("现价"));
		++nCol;
		baTable.SetTitle(nCol, _T("前收"));
		++nCol;
		baTable.SetTitle(nCol, _T("涨跌"));
		++nCol;
		baTable.SetTitle(nCol, _T("换手率"));
		++nCol;
		baTable.SetTitle(nCol, _T("现量(手)"));
		baTable.SetPrecise(nCol, 0);
		//baTable.SetOutputItemRatio(nCol, 100);
		baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
		++nCol;
		baTable.SetTitle(nCol, _T("成交量(手)"));
		baTable.SetPrecise(nCol, 0);
		//baTable.SetOutputItemRatio(nCol, 100);
		baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
		++nCol;
		baTable.SetTitle(nCol, _T("成交金额(万元)"));
		baTable.SetPrecise(nCol, 0);//cyh80 2009-08-19 改为保留0位小数(2)
		baTable.SetOutputItemRatio(nCol, 10000);
		baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
		++nCol;

		//股票流通股本
		//SetDisplayTableColInfo(&baTable,nCol,30000050);
		baTable.SetTitle(nCol, _T("流通股本(亿)"));
		//小数点
		baTable.SetPrecise(nCol, 2);
		//倍数
		baTable.SetOutputItemRatio(nCol,100000000);
		baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
		++nCol;
		//股票总股本
		//SetDisplayTableColInfo(&baTable,nCol,30000049);
		baTable.SetTitle(nCol, _T("总股本(亿)"));
		//小数点
		baTable.SetPrecise(nCol, 2);
		//倍数
		baTable.SetOutputItemRatio(nCol,100000000);
		baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
		++nCol;
		
		//SetDisplayTableColInfo(&baTable,nCol,30000050,false);
		baTable.SetTitle(nCol, _T("流通市值(亿)"));
		//小数点
		baTable.SetPrecise(nCol, 2);
		//倍数
		baTable.SetOutputItemRatio(nCol,100000000);
		baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
		++nCol;

		//SetDisplayTableColInfo(&baTable,nCol,30000050,false);
		baTable.SetTitle(nCol, _T("总股市值(亿)"));
		//小数点
		baTable.SetPrecise(nCol, 2);
		//倍数
		baTable.SetOutputItemRatio(nCol,100000000);
		baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
		++nCol;
	}
	return true;
}
//排序分析基本数据结果设置
//默认按照股票处理
bool TxBusiness::SetBlockAnalysisCol(Table_Display& baTable,SecurityQuotation* psq,int& j,int ii)
{
	if(psq==NULL)
		return false;
	//初始化时缓存的最新股票信息
	double tradeableShare = psq->GetTradableShare();
	double share = psq->GetTotalShare();
	double dValue = Con_doubleInvalid;
	//int idate = psq->GetCurDataDate();
	float closePrice = psq->GetClosePrice(true);

	//流通股本
		baTable.SetCell(j,ii,tradeableShare);
		j++;
	//总股本
		baTable.SetCell(j,ii,share);
		j++;
	//流通市值
		if(tradeableShare>0 && closePrice>0)
			dValue = closePrice*tradeableShare;
		else
			//流通股本不可能小于0
			dValue = Tx::Core::Con_doubleInvalid;
		baTable.SetCell(j,ii,dValue);
		j++;
	//总市值
		if(share>0 && closePrice>0)
			dValue = closePrice*share;
		else
			//流通股本不可能小于0
			dValue = Tx::Core::Con_doubleInvalid;
		baTable.SetCell(j,ii,dValue);
		j++;
	return true;
}
//排序分析基本数据结果设置
bool TxBusiness::SetBlockAnalysisBasicCol(Table_Display& baTable,SecurityQuotation* psq,int& j,int ii,int idate,int iSamplesCount)
{
	float prePrice	= Con_floatInvalid;
	float closePrice	= Con_floatInvalid;
	float openPrice	= Con_floatInvalid;
	float lowPrice	= Con_floatInvalid;
	float highPrice	= Con_floatInvalid;
	double amount		= Con_doubleInvalid;
	double volume		= Con_doubleInvalid;
	double dRaiseValue = 0;
	double dRaise = 0;
	double dHsl = Con_doubleInvalid;
#ifdef _DEBUG
	//int iTestId = psq->GetId();
	//bool bn = psq->IsNormal();
	//bool bs = psq->IsStop();
	//bool bh = psq->IsHalt();
	//bool bhl = psq->IsHaltLong();

#endif
	//int idate = psq->GetCurDataDate();
	//0.406;0.515
	//0.437;0.422
	//0.062;0.047
	//if(psq->IsValid()==true)
	if(!psq->IsFund_Estimate())
	{
		prePrice	= psq->GetPreClosePrice();
		closePrice	= psq->GetClosePrice(true);
		openPrice	= psq->GetOpenPrice();
		//lowPrice	= psq->GetLowPrice();
		//highPrice	= psq->GetHighPrice();
		amount		= psq->GetAmount();
		volume		= psq->GetVolume(true);

		//if(TX_DATA_TRADE_STATUS_BEFORE0 == DataStatus::GetInstance()->GetStatusInt(Tx::Data::tag_DATA_TRADE_TIME_STATUS,psq->GetInnerBourseId()))
		//{
		//	TradeQuotation* pTradeQuotation = psq->GetFirstTradeQuotationPointer();
		//	if(pTradeQuotation!=NULL)
		//	{
		//		//get preclose
		//		//get buy price 1
		//		TradeQuotationData* pTradeQuotationData = pTradeQuotation->GetBuyData(1);
		//		if(pTradeQuotationData!=NULL)
		//		{
		//			if(prePrice>0 && pTradeQuotationData->fPrice>0)
		//			{
		//				dRaiseValue = pTradeQuotationData->fPrice-prePrice;
		//				dRaise = (pTradeQuotationData->fPrice-prePrice)/prePrice*100;;
		//			}
		//		}
		//	}
		//}
		//else
		{
			dRaise = psq->GetRaise();
			dRaiseValue = psq->GetRaiseValue();
		}
		//dHsl = psq->GetTradeRate();
	}
	//0.170
	//0.140
	//0.094;0.078
	//现量
	double dVolumeNow = psq->GetVolumeLatest(true);
	if(!(dVolumeNow>0))
		dVolumeNow = Con_doubleInvalid;
		/*
	int iDetailCount = 0;
	iDetailCount = psq->GetTrdaeDetailDataCount();
	if(iDetailCount<=0)
		dVolumeNow = Con_doubleInvalid;
	else
	{
		if(iDetailCount==1)
			dVolumeNow = volume;
		else
			dVolumeNow = volume - psq->GetTrdaeDetailData(iDetailCount-2)->dVolume;
	}
	*/
	CString sName;
	CString sExtCode;
	sName = Con_strInvalid;
	sExtCode = Con_strInvalid;
	sName = psq->GetName();
	sExtCode = psq->GetCode();

	//设置名称
		baTable.SetCell(j,ii,sName);
		j++;
		//0.597;0.620
		//return true;
	//设置外码
		baTable.SetCell(j,ii,sExtCode);
		j++;
		//2008-02-19
		//原始顺序
		j++;
	//涨幅
		baTable.SetCell(j,ii,dRaise);
		j++;
	//现价
		baTable.SetCell(j,ii,closePrice);
		j++;
	//前收
		baTable.SetCell(j,ii,prePrice);
		j++;
	//涨跌
		baTable.SetCell(j,ii,dRaiseValue);
		j++;
	//换手率
		//baTable.SetCell(j,ii,dHsl);
		SetBlockAnalysisHslCol(baTable,psq,j,ii);
		j++;
	//现量
		baTable.SetCell(j,ii,dVolumeNow);
		j++;
	//成交量
		double dVol = volume;//100;
		baTable.SetCell(j,ii,dVol);  // volume
		j++;
	//成交金额
		baTable.SetCell(j,ii,amount);
		j++;
	return true;
}
//排序分析基本数据结果设置[换手率]
bool TxBusiness::SetBlockAnalysisHslCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow)
{
	double dHsl = Con_doubleInvalid;
	dHsl = pSecurity->GetTradeRate();
	//换手率
	baTable.SetCell(nCol,nRow,dHsl);
	return true;
}
//添加行
bool TxBusiness::BlockAnalysisAddRow(Table_Display& baTable,SecurityQuotation* pSecurity,int idate)
{
	baTable.AddRow();
	return true;
}

//板块=排序分析
//baTable已经填充了样本[交易实体]
bool TxBusiness::GetBlockAnalysis(Table_Display& baTable,std::vector<int>& arrSamples,int& iSortCol,int iMarketId,bool bNeedProgressWnd,int nCollID)
{	
	baTable.Clear();

	//step1 处理结果数据的标题列	
	GetBlockAnalysisCol(baTable,arrSamples,iSortCol);

	//======================================================
	//step2 处理样本

	int iInitNo = 3;
	if(baTable.GetColCount()<=0)
	{
		CString sLog;
		sLog.Format(_T("排序分析数据列错误[0]")
			);
		Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
		return false;
	}

	int count = (int)arrSamples.size();
	if(count<=0)
	{
		CString sLog;
		sLog.Format(_T("排序分析样本错误[0]")
			);
		Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
		return false;
	}

//======================================================
	//2008-02-18
	//控制是否需要进度条
	//step3.1	
	Tx::Core::ProgressWnd prw;
	//step3.2
	CString sProgressPrompt;
	UINT progId = 0;
	//step3.3
	if(bNeedProgressWnd==true)
	{
		//2008-06-19
		//支持取消
		prw.EnableCancelButton(true);		
		sProgressPrompt.Format(_T("排序分析..."));
		progId = prw.AddItem(1,sProgressPrompt, 0.0);
		prw.Show(15);
	}

	//step4 循环样本
	int row= baTable.GetRowCount();
	int col = baTable.GetColCount();
	int idate = 0;

	//取行情日期
	if(iMarketId==0)
		idate = m_pFunctionDataManager->GetCurDataDateTime_ShangHai().GetDate().GetInt();
	else
		idate = m_pFunctionDataManager->GetCurDataDateTime(iMarketId).GetDate().GetInt();

	//2009-01-08
	if(Tx::Core::UserInfo::GetInstance()->GetNetType()==1)
	{
		if(m_pLogicalBusiness!=NULL)
			m_pLogicalBusiness->RequestSecuritysTradeData(arrSamples,50);
	}

	int ii = 0;
	int i =0;	
	bool bIsSpecialBLock = false;
	
	if(50010143 == nCollID || 50010145 == nCollID || 50010538 == nCollID || 50010031 == nCollID||50010120 ==nCollID
		||50014286 ==nCollID		// 2013-01-21 for mantis 14369
		)
		bIsSpecialBLock = true;

	//债券YTM
	GetBondYTM(arrSamples);

	for (std::vector<int>::iterator iter = arrSamples.begin(); iter != arrSamples.end(); iter++)	
	{
		//0.063;0.015
		if(bNeedProgressWnd==true)
		{
			prw.SetPercent(progId, (double)i/(double)count);
			if(prw.IsCanceled()==true)
				break;
		}

		int iSecurityId = 0;
		iSecurityId = *iter;

		//step5.2 取得样本的Security指针
		GetSecurityNow(iSecurityId);
		SecurityQuotation* psq = (SecurityQuotation*)m_pSecurity;
		if(psq!=NULL)
		{
			if(!bIsSpecialBLock && (m_pSecurity->IsHaltLong() || m_pSecurity->IsIssued() || m_pSecurity->IsStop() || m_pSecurity->IsFailureOfIssue()))
				continue; 

			int iCol = baTable.GetRowCount();
			if(BlockAnalysisAddRow(baTable,psq,idate)==true)
			{				
				baTable.SetCell(0,iCol,iSecurityId);
				baTable.SetCell(iInitNo,iCol,iCol+1);

				//step5.3 处理基本指标数据
				int j = 1;
				if(SetBlockAnalysisBasicCol(baTable,psq,j,iCol,idate,count)==true)
				{
					//step5.4 处理特殊指标数据
					SetBlockAnalysisCol(baTable,psq,j,iCol);
				}
			}
		}
		else
		{
			TRACE(_T("\nid none [%d]"),iSecurityId);
		}
		i++;
	}

	if(bNeedProgressWnd==true)
	{
		//step6 循环完成
		prw.SetPercent(progId, 1.0);
		sProgressPrompt+=_T(",完成!");
		prw.SetText(progId, sProgressPrompt);
	}

	return true;
}

double TxBusiness::GetLR(SecurityQuotation* psq,int calc_type,int* invalid_lr)
{
	double dLR = 0;
	int iF_FISCAL_YEAR_QUARTER=0;

	//上市公司基础指标[最新信息][合并] 功能ID 30007 文件30007404.dat
	InstitutionNewInfo* pInstitutionNewInfo = psq->GetInstitutionNewInfo();
	if(pInstitutionNewInfo==NULL)
		return 0;

	//byte iF_FISCAL_YEAR_QUARTER=0;
	/////////////////////////////////////////////////////////////////////////////////
	//最新利润
	//0=静态;1=滚动;2=简单;3=同比
	switch(calc_type)
	{
	case 0://静态
		{
			//5.byte bF_FISCAL_YEAR_QUARTER=0;			//	85930	30300200	最新报告期	tinyint
			//if(psq->GetIndicatorValueAbsolute(30300200,iF_FISCAL_YEAR_QUARTER)==false)
			//	iF_FISCAL_YEAR_QUARTER = 0;
			iF_FISCAL_YEAR_QUARTER = pInstitutionNewInfo->fiscal_year_quarter;
			if(iF_FISCAL_YEAR_QUARTER==Con_intInvalid)
				iF_FISCAL_YEAR_QUARTER = 0;

			if(iF_FISCAL_YEAR_QUARTER==0 || 
				TypeMapManage::GetInstance()->GetDatByID(TYPE_FISCAL_YEAR_QUARTER,iF_FISCAL_YEAR_QUARTER)==_T("年报"))
			{
				//5.30300203	当期归属于母公司利润	decimal=jt:最新报告期==年报
				//if(psq->GetIndicatorValueAbsolute(30300203,dLR)==false)
				//{
				//	dLR = 0;
				//	*invalid_lr++;
				//}
				dLR = pInstitutionNewInfo->profit_parent_cur;
			}
			else
			{
				//5.30300205	上年年报归属于母公司利润	decimal=jt:最新报告期!=年报
				//if(psq->GetIndicatorValueAbsolute(30300205,dLR)==false)
				//{
				//	dLR = 0;
				//	*invalid_lr++;
				//}
				dLR = pInstitutionNewInfo->profit_parent_last_annual;
			}
		}
		break;
	case 1://滚动
		//5.30300202	最新滚动利润	decimal
		//if(psq->GetIndicatorValueAbsolute(30300202,dLR)==false)
		//{
		//	dLR = 0;
		//	*invalid_lr++;
		//}
		dLR = pInstitutionNewInfo->ltm_profit_parent;
		break;
	case 2://简单
		//30300210	简单年化归属于母公司利润	decimal
		//if(psq->GetIndicatorValueAbsolute(30300210,dLR)==false)
		//{
		//	dLR = 0;
		//	*invalid_lr++;
		//}
		dLR = pInstitutionNewInfo->profit_parent_annual_basic;
		break;
	case 3://同比
		//30300211	同比年化归属母公司利润	decimal
		//if(psq->GetIndicatorValueAbsolute(30300211,dLR)==false)
		//{
		//	dLR = 0;
		//	*invalid_lr++;
		//}
		//2008-12-16
		if(pInstitutionNewInfo->profit_parent_last<0 || pInstitutionNewInfo->profit_parent_last_annual<0)
			dLR = pInstitutionNewInfo->profit_parent_annual_basic;
		else
			dLR = pInstitutionNewInfo->profit_parent_annual_yoy;
		break;
	default:
		dLR = 0;
		//invalid_lr++;
		break;
	}
	if(fabs(dLR-Con_doubleInvalid)<0.000001)
	{
		dLR = 0;
		*invalid_lr++;
	}
	return dLR;
}
//2007-12-18
bool TxBusiness::GetBlockPricedFuture(
	std::set<int>& iSecurityId,	//板块样本集
	Tx::Core::Table& resTable,	//定价特征计算结果
	long lBourseId,				//交易所ID,0表示所有样本
	int iCalcType,				//计算方法 0=静态;1=滚动;2=简单;3=同比
	bool bNeedPsw
	)
{
	resTable.Clear();
	//0=加入样本名称=大类
	resTable.AddCol(Tx::Core::dtype_val_string);
	//1=加入样本名称=小类
	resTable.AddCol(Tx::Core::dtype_val_string);
	//2=指标数据[沪市，深市，合计]
	resTable.AddCol(Tx::Core::dtype_val_string);
	//3=查看明细[在界面上查看明细]
	resTable.AddCol(Tx::Core::dtype_val_string);

	//4=输入样本数=tot_count
	resTable.AddCol(Tx::Core::dtype_int4);
	//5=本市场样本数=countByBourse
	resTable.AddCol(Tx::Core::dtype_int4);
	//6=无效样本数量=invalid_count
	resTable.AddCol(Tx::Core::dtype_int4);
	//7=当日交易状态不正常数量=invalid_tradestatus_count
	resTable.AddCol(Tx::Core::dtype_int4);
	//8=PEPB数据无效数量=invalid_pepb_count
	resTable.AddCol(Tx::Core::dtype_int4);
	//9=实际参与运算的样本数=sampleCount
	resTable.AddCol(Tx::Core::dtype_int4);
	//10=对应股本无效数据的样本数=invalid_share
	resTable.AddCol(Tx::Core::dtype_int4);
	//11=公司总股本无效数据的样本数=invalid_insititutionshare
	resTable.AddCol(Tx::Core::dtype_int4);
	//12=亏损家数=iLR
	resTable.AddCol(Tx::Core::dtype_int4);
	//13=负净资产家数=iZC
	resTable.AddCol(Tx::Core::dtype_int4);
	//14=流通股本无效数据样本数=invalid_tradeableshare
	resTable.AddCol(Tx::Core::dtype_int4);
	//15无效利润数据的样本数=invalid_lr
	resTable.AddCol(Tx::Core::dtype_int4);
	//16无效股东权益数据的样本数=invalid_equity;
	resTable.AddCol(Tx::Core::dtype_int4);

	//17股东权益[不含负净资产]
	//dShareHolder_Equity_Parent2
	resTable.AddCol(Tx::Core::dtype_double);

	//不含亏损+不含负净资产
	//18 dProfitCur1 += dLR;
	resTable.AddCol(Tx::Core::dtype_double);
	//19 dShareHolder_Equity_Parent1
	resTable.AddCol(Tx::Core::dtype_double);

	//20 含亏损的利润
	//dProfitCur
	resTable.AddCol(Tx::Core::dtype_double);

	//21 股东权益[含负净资产]
	//dShareHolder_Equity_Parent
	resTable.AddCol(Tx::Core::dtype_double);
	//22不含亏损利润
	resTable.AddCol(Tx::Core::dtype_double);


	//根据板块样计算定价特征需要的基础数据
	//step1数据准备
	//取得各样本的总股本、境内股本、流通股本、国有股本；
	//取得各样本的的最新价格；
	
	//板块样本数量
	int tot_count = iSecurityId.size();
	//无效样本数量
	int invalid_count = 0;
	//当日交易状态不正常数量
	int invalid_tradestatus_count = 0;
	//PEPB数据无效数量
	int invalid_pepb_count = 0;
	//实际参与运算的样本数
	int sampleCount = 0;

	//板块为空，不予处理

	if(tot_count<=0)
		return false;

	//指定交易所的样本数量，需要统计
	int countByBourse = 0;
	/*
	//亏损
	//亏损类总数，非亏损，亏损，无效数据
	int count_ks = 0,count_ks1 = 0,count_ks2=0,count_ks3 = 0;
	//净资产
	//净资产类总数，非负净资产，负净资产，无效数据
	int count_jzc = 0,count_jzc1 = 0,count_jzc2=0,count_jzc3 = 0;
	*/

	double dTotShareOk=0,dTotShareCancel=0;

	CString sMarketName = _T("");
	if(lBourseId==0)
		sMarketName = _T("全市场");
	else
	{
		//2007-11-02
		//索引方式修改
		//交易所索引
		sMarketName = TypeMapManage::GetInstance()->GetDatByID(TYPE_EXCHANGE,(int)lBourseId);
	}

	//step1
	// Tx::Core::ProgressWnd* pProgressWnd = Tx::Core::ProgressWnd::GetInstance();
	Tx::Core::ProgressWnd prw;
	//step2
	CString sProgressPrompt;
	sProgressPrompt.Format(_T("定价特征基础数据[%s]..."),sMarketName);
	UINT progId;
	if(bNeedPsw)
	{
		progId = prw.AddItem(1,sProgressPrompt, 0.0);
		//step3
		prw.Show(15);
	}

	//交易实体基础指标
	int iF_TRANS_OBJ_ID=0;			//	85920	4			交易实体id		int
	int iF_TRADE_DATE=0;			//	85921	30000094	交易日期		int
	double dF_CLOSE=0;				//	85922	30000095	交易价格		decimal
	double dF_SHARE=0;				//	85923	30000096	对应股本		decimal
	double dF_TOTAL_SHARE=0;		//	85924	30000097	总股本			decimal
	double dF_SHARE_PERCENT=0;		//	85925	30000098	对应股本占比	decimal
	double dF_INSTITUTION_ID=0;		//	85926	30000099	对应上市公司id	int
	double dF_CURRENCY_EXCHANGE=0;	//	85927	30000100	汇率			decimal
	double dF_TRADEABLE_SHARE=0;	//	85923	30000101	对应流通股本	decimal

	//上市公司基础指标
	int iF_INSTITUTION_ID=0;				//	4	4				公司id	int
	byte bF_CONSOLIDATED_NONCONSOLIDATED=0;	//	85928	5			是否合并	tinyint
	int iF_FISCAL_YEAR=0;					//	85929	30300199	最新财年	int
	byte bF_FISCAL_YEAR_QUARTER=0;			//	85930	30300200	最新报告期	tinyint
	double dF_SHAREHOLDER_EQUITY_PARENT=0;	//	85931	30300201	最新归属于母公司股东权益	decimal
	//double dF_LTM_PROFIT_PARENT=0;		//	85932	30300202	最新滚动利润	decimal
	double dF_PROFIT_PARENT_CUR=0;			//	85933	30300203	当期归属于母公司利润	decimal=jt:最新报告期==年报
	double dF_PROFIT_PARENT_LAST=0;			//	85934	30300204	上期归属于母公司利润	decimal
	double dF_PROFIT_PARENT_LAST_ANNUAL=0;	//	85935	30300205	上年年报归属于母公司利润	decimal=jt:最新报告期!=年报
	//double dF_TOTAL_SHARE;				//	85936	30300206	总股本	numeric
	double dF_BASIC_EPS_ANNUAL=0;			//	85937	30300207	上年年报基本EPS	decimal
	double dF_DYNAMIC_EPS1=0;				//	85938	30300208	分析师预测当年EPS	decimal
	double dF_DYNAMIC_EPS2=0;				//	85939	30300209	分析师预测下一年EPS	decimal
	double dF_PROFIT_PARENT_ANNUAL_BASIC=0;	//	85940	30300210	简单年化归属于母公司利润	decimal
	double dF_PROFIT_PARENT_ANNUAL_YOY=0;	//	85941	30300211	同比年化归属母公司利润	decimal
	double dF_PROFIT_PARENT_ANNUAL_LATEST=0;//	85942	30300212	最近年报eps	decimal

	//总市值
	double dTotValue = 0;
	//总股本[对应][含负净资产]
	double dTotShare = 0;
	//总股本[对应][不含负净资产]
	double dTotShare1 = 0;
	//总股本[对应][不含亏损]
	double dTotShare2 = 0;
	//总股本[公司含亏损]
	double dTotShareInstitution = 0;
	//总股本[公司不含亏损]
	double dTotShareInstitution1 = 0;
	//对应股本无效数据=股本和市值使用同一个统计结果
	int invalid_share = 0;
	//公司总股本无效数据=股本和市值使用同一个统计结果
	int invalid_insititutionshare = 0;

	//亏损家数
	int iLR = 0;
	//负净资产家数
	int iZC = 0;


	//流通市值
	double dTradeableValue = 0;
	//流通股本
	double dTotTradeableShare = 0;
	//国有股本
	double dTotNationShare = 0;
	//流通股本无效数据=股本和市值使用同一个统计结果
	int invalid_tradeableshare = 0;

	//市盈率PE总股本加权[含亏损]=dTotValue/dPE_denominator
	double dPE = 0;
	double dPE_denominator = 0;

	//市盈率PE总股本加权[不含亏损]=dTotValue1/dPE_denominator1
	double dPE1 = 0;
	//PE
	double dTotValue1 = 0;
	double dPE_denominator1 = 0;

	//市盈率PE流通股本加权[含亏损]=dTradeableValue/dPE_denominatorT
	double dPET = 0;
	double dPE_denominatorT = 0;

	//市盈率PE流通股本加权[不含亏损]=dTradeableValue1/dPE_denominator1T1
	double dPET1 = 0;
	double dTradeableValue1 = 0;
	double dPE_denominatorT1 = 0;

	//市净率PB总股本加权[含亏损]=dTotValue/dPB_denominator
	double dPB = 0;
	double dPB_denominator = 0;
	//市净率PB总股本加权[不含亏损]=dTotValuePB1/dPB_denominator1
	double dPB1 = 0;
	double dTotValuePB1 = 0;
	double dPB_denominator1 = 0;

	//市净率PB流通股本加权[含亏损]=dTradeableValue/dPB_denominator
	double dPBT = 0;
	double dPB_denominatorT = 0;
	//市净率PB流通股本加权[不含亏损]=dTradeableValuePBT1/dPB_denominatorT1
	double dPBT1 = 0;
	double dTradeableValuePBT1 = 0;
	double dPB_denominatorT1 = 0;

	//净资产收益率[含负净资产]=dProfitCur/dShareHolder_Equity_Parent
	double dProfit = 0;
	double dProfitCur = 0;
	//22不含亏损利润
	double dProfitCur2 = 0;

	//总净资产
	double dShareHolder_Equity_Parent=0;
	//净资产收益率[不含负净资产]=dProfitCur1/dShareHolder_Equity_Parent1
	double dProfit1 = 0;
	double dProfitCur1 = 0;
	//去亏损+去负资产
	double dShareHolder_Equity_Parent1=0;
	//去负资产
	double dShareHolder_Equity_Parent2=0;
	//无效股东权益数据
	int invalid_equity = 0;

	//算术平均价格 = dSumPrice/sampleCount
	double dAvgPrice = 0;
	double dSumPrice = 0;
	//算术平均价格[总股本加权] = dTotValue/dTotShare
	double dAvgPriceTot = 0;
	//算术平均价格[流通股本加权] = dTradeableValue/dTotTradeableShare
	double dAvgPriceTradeable = 0;

	//加权平均每股收益[含亏损]=dProfitCur/dTotShareInstitution
	double dEPS = 0;
	//加权平均每股收益[不含亏损]=dProfitCur1/dTotShareInstitution1
	double dEPS1 = 0;

	//利润根据算法不同分别取值
	double dLR = 0;
	//无效利润数据统计
	int invalid_lr = 0;


	//2009-01-12
	//徐：国有股净资产算法与个股同，累加而已
	double dShareHolder_Equity_Parent_nation_stat = 0;
	//dShareHolder_Equity_Parent_nation = dShareHolder_Equity_Parent*dTotNationShare/dTotShare;

	int iDjtz_count = 0;
	//Tx::Core::Table_Indicator resTable;
	//循环统计
	for (std::set<int>::const_iterator iter = iSecurityId.begin(); iter != iSecurityId.end(); ++iter)
	{
		//step4
		if(bNeedPsw)
			prw.SetPercent(progId, (double)iDjtz_count/(double)iSecurityId.size());
		iDjtz_count++;
		//取得样本的交易实体ID
		int iSecurity = *iter;
		//取得样本的Security指针
		GetSecurityNow(iSecurity);
		SecurityQuotation* psq = (SecurityQuotation*)m_pSecurity;

		if(psq==NULL)
		{
			invalid_count++;
			continue;
		}
		//2008-06-10
		//如果是指数，则跳过
		if(psq->IsIndex()==true)
		{
			invalid_count++;
			continue;
		}

		//2007-09-17
		//不处理开放式基金
		//if(psq->InFundOpenBlock()==true)
		//	continue;

		//统计指定交易所的样本
		if(lBourseId>0 && psq->GetBourseId()!=lBourseId)
			continue;

		if(psq->IsNormal()==false)
		{
			invalid_tradestatus_count++;
			continue;
		}

		countByBourse++;

		//最新PEPB
		//PePb*	pSecurityPePb = psq->GetPePbDataLatest();
		//if(pSecurityPePb==NULL)
		//{
		//	invalid_pepb_count++;
		//	continue;
		//}
		sampleCount++;

		//step1 数据准备
		//取得各样本的总股本、境内股本、流通股本、国有股本；
		//取得各样本的的最新价格；
		double dValue=Con_doubleInvalid;

		double dNationShare = 0;
		dValue = psq->GetNationShare();
		dValue = dValue < 0 || dValue == Con_doubleInvalid?0.0:dValue;
		dNationShare = dValue;
		if(dValue>0)
			dTotNationShare += dValue;

		//实时行情数据
		//初始化时缓存的最新股票信息

		//1.最新价格
		float closePrice = Con_floatInvalid;
		closePrice = psq->GetClosePrice(true);

		//2008-07-09
		if(closePrice<0) closePrice = 0;

		//2.30000096	对应股本
		//if(psq->GetIndicatorValueAbsolute(30000096,dF_SHARE)==false)
		//{
		//	dF_SHARE = 0;
		//	invalid_share++;
		//}
		dF_SHARE = psq->GetTotalShare();
		if(dF_SHARE<0)
		//if(fabs(dF_SHARE-Con_doubleInvalid)<0.000001)
		{
			dF_SHARE = 0;
			invalid_share++;
		}
		//3.30000101	对应流通股本
		//if(psq->GetIndicatorValueAbsolute(30000101,dF_TRADEABLE_SHARE)==false)
		//{
		//	dF_TRADEABLE_SHARE = 0;
		//	invalid_tradeableshare++;
		//}
		dF_TRADEABLE_SHARE = psq->GetTradableShare();
		if(dF_TRADEABLE_SHARE<0)
		//if(fabs(dF_TRADEABLE_SHARE-Con_doubleInvalid)<0.000001)
		{
			dF_TRADEABLE_SHARE = 0;
			invalid_tradeableshare++;
		}
		//4.30000097	总股本
		//if(psq->GetIndicatorValueAbsolute(30000097,dF_TOTAL_SHARE)==false)
		//{
		//	dF_TOTAL_SHARE = 0;
		//	invalid_insititutionshare++;
		//}
		dF_TOTAL_SHARE = psq->GetTotalInstitutionShare();
		if(dF_TOTAL_SHARE<0)
		//if(fabs(dF_TOTAL_SHARE-Con_doubleInvalid)<0.000001)
		{
			dF_TOTAL_SHARE = 0;
			invalid_insititutionshare++;
		}

		//5.byte bF_FISCAL_YEAR_QUARTER=0;			//	85930	30300200	最新报告期	tinyint
		//if(psq->GetIndicatorValueAbsolute(30300200,bF_FISCAL_YEAR_QUARTER)==false)
		//	bF_FISCAL_YEAR_QUARTER = 0;

		//6.30300201	最新归属于母公司股东权益	decimal
		//if(psq->GetIndicatorValueAbsolute(30300201,dF_SHAREHOLDER_EQUITY_PARENT)==false)
		//{
		//	dF_SHAREHOLDER_EQUITY_PARENT = 0;
		//	invalid_equity++;
		//}

		dF_SHAREHOLDER_EQUITY_PARENT = 0;
		InstitutionNewInfo* pInstitutionNewInfo = psq->GetInstitutionNewInfo();
		if(psq->IsStock()==true)
		{
		if(pInstitutionNewInfo!=NULL)
		{
			dF_SHAREHOLDER_EQUITY_PARENT = pInstitutionNewInfo->shareholder_equity_parent;
		}
		}
		if(fabs(dF_SHAREHOLDER_EQUITY_PARENT-Con_doubleInvalid)<0.000001)
		{
			dF_SHAREHOLDER_EQUITY_PARENT = 0;
			invalid_equity++;
		}

		//7.30000100	汇率
		//if(psq->GetIndicatorValueAbsolute(30000100,dF_CURRENCY_EXCHANGE)==false)
		//	dF_CURRENCY_EXCHANGE = 1;
		dF_CURRENCY_EXCHANGE = 1;
		if(psq->IsStock()==true)
		{
			StockNewInfo* pStockNewInfo = psq->GetStockNewInfo();
			if(pStockNewInfo!=NULL)
			dF_CURRENCY_EXCHANGE = pStockNewInfo->currency_exchange;
			if(fabs(dF_CURRENCY_EXCHANGE-Con_doubleInvalid)<0.000001)
				dF_CURRENCY_EXCHANGE = 1;
		}

		//step2 求和统计
		//2008-12-11
		closePrice *= (float)dF_CURRENCY_EXCHANGE;
/////////////////////////////////////////////////////////////////////////////////
//总市值=实时现价*对应总股本
dTotValue +=(closePrice*dF_SHARE);

/////////////////////////////////////////////////////////////////////////////////
//对应总股本
dTotShare += dF_SHARE;

/////////////////////////////////////////////////////////////////////////////////
//流通市值=实时现价*对应流通股本
dTradeableValue +=(closePrice*dF_TRADEABLE_SHARE);
//对应流通股本
dTotTradeableShare += dF_TRADEABLE_SHARE;


/////////////////////////////////////////////////////////////////////////////////
//最新利润
//0=静态;1=滚动;2=简单;3=同比
dLR = 0;
if(psq->IsStock()==true)
dLR = GetLR(psq,iCalcType,&invalid_lr);

/////////////////////////////////////////////////////////////////////////////////
//含亏损总股本[公司]
dTotShareInstitution += dF_TOTAL_SHARE;

/////////////////////////////////////////////////////////////////////////////////
if(dLR<0)
	//亏损家数
	iLR++;
else
{
	//不含亏损总股本[公司]
	dTotShareInstitution1 += dF_TOTAL_SHARE;

	//不含亏损对应总股本
	dTotShare2 += dF_SHARE;

	//不含亏损总市值
	dTotValue1 +=(closePrice*dF_SHARE);

	//不含亏损流通市值
	dTradeableValue1 +=(closePrice*dF_TRADEABLE_SHARE);

	if(dF_TOTAL_SHARE>0)
	{
		//22不含亏损利润
		dProfitCur2 += dLR*dF_SHARE/dF_TOTAL_SHARE;

		//PE总股本加权[不含亏损][中间计算参数]
		//PE总股本加权[不含亏损]=利润*对应股本/公司总股本
		dPE_denominator1 += dLR*dF_SHARE/dF_TOTAL_SHARE;

		//PE流通股本加权[不含亏损][中间计算参数]
		//PE流通股本加权[不含亏损]=利润*对应流通股本/公司总股本
		dPE_denominatorT1 += dLR*dF_TRADEABLE_SHARE/dF_TOTAL_SHARE;
	}
}

/////////////////////////////////////////////////////////////////////////////////
if(dF_TOTAL_SHARE>0)
{
	//2008-12-16
	if(iCalcType!=3 || (iCalcType==3 && dLR>0))
	{
		//PE总股本加权[含亏损]
		dPE_denominator += dLR*dF_SHARE/dF_TOTAL_SHARE;

		//PE流通股本加权[含亏损]
		dPE_denominatorT += dLR*dF_TRADEABLE_SHARE/dF_TOTAL_SHARE;
	}

	//PB总股本加权[含亏损]
	dPB_denominator += dF_SHAREHOLDER_EQUITY_PARENT*dF_SHARE/dF_TOTAL_SHARE;

	//PB流通股本加权[含亏损]
	dPB_denominatorT += dF_SHAREHOLDER_EQUITY_PARENT*dF_TRADEABLE_SHARE/dF_TOTAL_SHARE;

	//股东权益[含负净资产]
	dShareHolder_Equity_Parent += dF_SHAREHOLDER_EQUITY_PARENT*dF_SHARE/dF_TOTAL_SHARE;
}

//2009-01-12
//国有股净资产
if(dNationShare>0)
	dShareHolder_Equity_Parent_nation_stat += dF_SHAREHOLDER_EQUITY_PARENT*dNationShare/dF_TOTAL_SHARE;

/////////////////////////////////////////////////////////////////////////////////
if(dF_SHAREHOLDER_EQUITY_PARENT<0)
	//负净资产家数
	iZC++;
else
{
	//PB总市值[不含负净资产]
	dTotValuePB1 +=(closePrice*dF_SHARE);

	//PB流通市值[不含负净资产]
	dTradeableValuePBT1 +=(closePrice*dF_TRADEABLE_SHARE);

	//对应总股本[不含负净资产]
	dTotShare1+=dF_SHARE;

	if(dF_TOTAL_SHARE>0)
	{
		//股东权益[不含负净资产]
		dShareHolder_Equity_Parent2 += dF_SHAREHOLDER_EQUITY_PARENT*dF_SHARE/dF_TOTAL_SHARE;

		//PB总股本加权[不含负净资产][中间计算参数]
		dPB_denominator1 += dF_SHAREHOLDER_EQUITY_PARENT*dF_SHARE/dF_TOTAL_SHARE;

		//PB流通股本加权[不含亏损][中间计算参数]
		dPB_denominatorT1 += dF_SHAREHOLDER_EQUITY_PARENT*dF_TRADEABLE_SHARE/dF_TOTAL_SHARE;
	}
}

		//7.30300203	当期归属于母公司利润	decimal
		//if(psq->GetIndicatorValueAbsolute(30300203,dF_PROFIT_PARENT_CUR)==false)
		//	dF_PROFIT_PARENT_CUR = 0;

/////////////////////////////////////////////////////////////////////////////////
//净资产收益率[含负净资产]
if(dF_TOTAL_SHARE>0)
	dProfitCur += dLR*dF_SHARE/dF_TOTAL_SHARE;

/////////////////////////////////////////////////////////////////////////////////
//item11=净资产收益率[不含负净资产]
if(!(dLR<0) && dF_TOTAL_SHARE>0)
{
	dProfitCur1 += dLR*dF_SHARE/dF_TOTAL_SHARE;
}
if(dF_TOTAL_SHARE>0 && !(dF_SHAREHOLDER_EQUITY_PARENT<0))
{
	dShareHolder_Equity_Parent1 += dF_SHAREHOLDER_EQUITY_PARENT*dF_SHARE/dF_TOTAL_SHARE;
}

/////////////////////////////////////////////////////////////////////////////////
//考虑B股
//计算平均价格的中间参数
//dSumPrice += (closePrice*dF_CURRENCY_EXCHANGE);
dSumPrice += closePrice;

		//9.30300208	分析师预测当年EPS	decimal
		//if(psq->GetIndicatorValueAbsolute(30300208,dF_DYNAMIC_EPS1)==false)
		//	dF_DYNAMIC_EPS1 = 1;

		//10.30300209	分析师预测下一年EPS	decimal
		//if(psq->GetIndicatorValueAbsolute(30300209,dF_DYNAMIC_EPS2)==false)
		//	dF_DYNAMIC_EPS2 = 1;


		//if((dReturned-Tx::Core::Con_doubleInvalid)<0.000001)
		//	dReturned = 0;
	}

	//step3 保存数据
	//0=加入样本名称=大类
	//1=加入样本名称=小类
	//2=指标数据[沪市，深市，合计]
	//3=查看明细[在界面上查看明细]
	//4=输入样本数=tot_count
	//5=本市场样本数=countByBourse
	//6=无效样本数量=invalid_count
	//7=当日交易状态不正常数量=invalid_tradestatus_count
	//8=PEPB数据无效数量=invalid_pepb_count
	//9=实际参与运算的样本数=sampleCount
	//10=对应股本无效数据的样本数=invalid_share
	//11=公司总股本无效数据的样本数=invalid_insititutionshare
	//12=亏损家数=iLR
	//13=负净资产家数=iZC
	//14=流通股本无效数据样本数=invalid_tradeableshare
	//15无效利润数据的样本数=invalid_lr
	//16无效股东权益数据的样本数=invalid_equity;

	//17股东权益[不含负净资产]
	//dShareHolder_Equity_Parent2

	//不含亏损+不含负净资产
	//18 dProfitCur1 += dLR;
	//19 dShareHolder_Equity_Parent1

	//20 含亏损的利润
	//dProfitCur

	//21 股东权益[含负净资产]
	//dShareHolder_Equity_Parent

	//22不含亏损利润
	//double dProfitCur2

	CString sDetail = _T("查看明细");
	int iValueScale = 2;
	//item1=总市值=dTotValue
	SetBlockPricedFutureCell(resTable,_T("市值"),_T("境内总值"),dTotValue/TX_INT_YI,iValueScale,fabs(dTotValue-0)<0.000001?_T(""):sDetail,
		tot_count,countByBourse,invalid_count,
		invalid_tradestatus_count,invalid_pepb_count,sampleCount,
		invalid_share,invalid_insititutionshare,iLR,iZC,
		invalid_tradeableshare,invalid_lr,invalid_equity,
		dShareHolder_Equity_Parent2,
		dProfitCur1,
		dShareHolder_Equity_Parent1,
		dProfitCur,
		dShareHolder_Equity_Parent,
		dProfitCur2
		);
	//item2=流通市值=dTradeableValue
	SetBlockPricedFutureCell(resTable,_T("市值"),_T("流通总值"),dTradeableValue/TX_INT_YI,iValueScale,fabs(dTradeableValue-0)<0.000001?_T(""):sDetail,
		tot_count,countByBourse,invalid_count,
		invalid_tradestatus_count,invalid_pepb_count,sampleCount,
		invalid_share,invalid_insititutionshare,iLR,iZC,
		invalid_tradeableshare,invalid_lr,invalid_equity,
		dShareHolder_Equity_Parent2,
		dProfitCur1,
		dShareHolder_Equity_Parent1,
		dProfitCur,
		dShareHolder_Equity_Parent,
		dProfitCur2
		);

	//市盈率PE总股本加权=dTotValue/dPE_denominator
	if(fabs(dPE_denominator-0)>0.000001)
		dPE = dTotValue/dPE_denominator;
	SetBlockPricedFutureCell(resTable,_T("市盈率(倍)"),_T("总股本加权(含亏损)"),dPE,iValueScale,fabs(dPE-0)<0.000001?_T(""):sDetail,
		tot_count,countByBourse,invalid_count,
		invalid_tradestatus_count,invalid_pepb_count,sampleCount,
		invalid_share,invalid_insititutionshare,iLR,iZC,
		invalid_tradeableshare,invalid_lr,invalid_equity,
		dShareHolder_Equity_Parent2,
		dProfitCur1,
		dShareHolder_Equity_Parent1,
		dProfitCur,
		dShareHolder_Equity_Parent,
		dProfitCur2
		);

	if(fabs(dPE_denominator1-0)>0.000001)
		dPE1 = dTotValue1/dPE_denominator1;
	SetBlockPricedFutureCell(resTable,_T("市盈率(倍)"),_T("总股本加权(不含亏损)"),dPE1,iValueScale,fabs(dPE1-0)<0.000001?_T(""):sDetail,
		tot_count,countByBourse,invalid_count,
		invalid_tradestatus_count,invalid_pepb_count,sampleCount,
		invalid_share,invalid_insititutionshare,iLR,iZC,
		invalid_tradeableshare,invalid_lr,invalid_equity,
		dShareHolder_Equity_Parent2,
		dProfitCur1,
		dShareHolder_Equity_Parent1,
		dProfitCur,
		dShareHolder_Equity_Parent,
		dProfitCur2
		);

	if(fabs(dPE_denominatorT-0)>0.000001)
		dPET = dTradeableValue/dPE_denominatorT;
	SetBlockPricedFutureCell(resTable,_T("市盈率(倍)"),_T("流通股本加权(含亏损)"),dPET,iValueScale,fabs(dPET-0)<0.000001?_T(""):sDetail,
		tot_count,countByBourse,invalid_count,
		invalid_tradestatus_count,invalid_pepb_count,sampleCount,
		invalid_share,invalid_insititutionshare,iLR,iZC,
		invalid_tradeableshare,invalid_lr,invalid_equity,
		dShareHolder_Equity_Parent2,
		dProfitCur1,
		dShareHolder_Equity_Parent1,
		dProfitCur,
		dShareHolder_Equity_Parent,
		dProfitCur2
		);
	if(fabs(dPE_denominatorT1-0)>0.000001)
		dPET1 = dTradeableValue1/dPE_denominatorT1;
	SetBlockPricedFutureCell(resTable,_T("市盈率(倍)"),_T("流通股本加权(不含亏损)"),dPET1,iValueScale,fabs(dPET1-0)<0.000001?_T(""):sDetail,
		tot_count,countByBourse,invalid_count,
		invalid_tradestatus_count,invalid_pepb_count,sampleCount,
		invalid_share,invalid_insititutionshare,iLR,iZC,
		invalid_tradeableshare,invalid_lr,invalid_equity,
		dShareHolder_Equity_Parent2,
		dProfitCur1,
		dShareHolder_Equity_Parent1,
		dProfitCur,
		dShareHolder_Equity_Parent,
		dProfitCur2
		);

	//PB
	if(fabs(dPB_denominator-0)>0.000001)
		dPB = dTotValue/dPB_denominator;
	SetBlockPricedFutureCell(resTable,_T("市净率(倍)"),_T("总股本加权(含负净资产)"),dPB,iValueScale,fabs(dPB-0)<0.000001?_T(""):sDetail,
		tot_count,countByBourse,invalid_count,
		invalid_tradestatus_count,invalid_pepb_count,sampleCount,
		invalid_share,invalid_insititutionshare,iLR,iZC,
		invalid_tradeableshare,invalid_lr,invalid_equity,
		dShareHolder_Equity_Parent2,
		dProfitCur1,
		dShareHolder_Equity_Parent1,
		dProfitCur,
		dShareHolder_Equity_Parent,
		dProfitCur2
		);
	if(fabs(dPB_denominator1-0)>0.000001)
		dPB1 = dTotValuePB1/dPB_denominator1;
	SetBlockPricedFutureCell(resTable,_T("市净率(倍)"),_T("总股本加权(不含负净资产)"),dPB1,iValueScale,fabs(dPB1-0)<0.000001?_T(""):sDetail,
		tot_count,countByBourse,invalid_count,
		invalid_tradestatus_count,invalid_pepb_count,sampleCount,
		invalid_share,invalid_insititutionshare,iLR,iZC,
		invalid_tradeableshare,invalid_lr,invalid_equity,
		dShareHolder_Equity_Parent2,
		dProfitCur1,
		dShareHolder_Equity_Parent1,
		dProfitCur,
		dShareHolder_Equity_Parent,
		dProfitCur2
		);

	if(fabs(dPB_denominatorT-0)>0.000001)
		dPBT = dTradeableValue/dPB_denominatorT;
	SetBlockPricedFutureCell(resTable,_T("市净率(倍)"),_T("流通股本加权(含负净资产)"),dPBT,iValueScale,fabs(dPBT-0)<0.000001?_T(""):sDetail,
		tot_count,countByBourse,invalid_count,
		invalid_tradestatus_count,invalid_pepb_count,sampleCount,
		invalid_share,invalid_insititutionshare,iLR,iZC,
		invalid_tradeableshare,invalid_lr,invalid_equity,
		dShareHolder_Equity_Parent2,
		dProfitCur1,
		dShareHolder_Equity_Parent1,
		dProfitCur,
		dShareHolder_Equity_Parent,
		dProfitCur2
		);
	if(fabs(dPB_denominatorT1-0)>0.000001)
		dPBT1 = dTradeableValuePBT1/dPB_denominatorT1;
	SetBlockPricedFutureCell(resTable,_T("市净率(倍)"),_T("流通股本加权(不含负净资产)"),dPBT1,iValueScale,fabs(dPBT1-0)<0.000001?_T(""):sDetail,
		tot_count,countByBourse,invalid_count,
		invalid_tradestatus_count,invalid_pepb_count,sampleCount,
		invalid_share,invalid_insititutionshare,iLR,iZC,
		invalid_tradeableshare,invalid_lr,invalid_equity,
		dShareHolder_Equity_Parent2,
		dProfitCur1,
		dShareHolder_Equity_Parent1,
		dProfitCur,
		dShareHolder_Equity_Parent,
		dProfitCur2
		);

	//净资产收益率
	if(fabs(dShareHolder_Equity_Parent-0)>0.000001)
		dProfit = dProfitCur/dShareHolder_Equity_Parent*100;
	SetBlockPricedFutureCell(resTable,_T("净资产收益率"),_T("(含负净资产)"),dProfit,iValueScale,fabs(dProfit-0)<0.000001?_T(""):sDetail,
		tot_count,countByBourse,invalid_count,
		invalid_tradestatus_count,invalid_pepb_count,sampleCount,
		invalid_share,invalid_insititutionshare,iLR,iZC,
		invalid_tradeableshare,invalid_lr,invalid_equity,
		dShareHolder_Equity_Parent2,
		dProfitCur1,
		dShareHolder_Equity_Parent1,
		dProfitCur,
		dShareHolder_Equity_Parent,
		dProfitCur2
		);
	if(fabs(dShareHolder_Equity_Parent1-0)>0.000001)
		dProfit1 = dProfitCur1/dShareHolder_Equity_Parent1*100;
	SetBlockPricedFutureCell(resTable,_T("净资产收益率"),_T("(不含负净资产)"),dProfit1,iValueScale,fabs(dProfit1-0)<0.000001?_T(""):sDetail,
		tot_count,countByBourse,invalid_count,
		invalid_tradestatus_count,invalid_pepb_count,sampleCount,
		invalid_share,invalid_insititutionshare,iLR,iZC,
		invalid_tradeableshare,invalid_lr,invalid_equity,
		dShareHolder_Equity_Parent2,
		dProfitCur1,
		dShareHolder_Equity_Parent1,
		dProfitCur,
		dShareHolder_Equity_Parent,
		dProfitCur2
		);

	iValueScale = 3;
	//算术平均价格 = dSumPrice/sampleCount
	if(sampleCount>0)
		dAvgPrice = dSumPrice/sampleCount;
	SetBlockPricedFutureCell(resTable,_T("平均价格(元)"),_T("算术平均价格"),dAvgPrice,iValueScale,fabs(dAvgPrice-0)<0.000001?_T(""):sDetail,
		tot_count,countByBourse,invalid_count,
		invalid_tradestatus_count,invalid_pepb_count,sampleCount,
		invalid_share,invalid_insititutionshare,iLR,iZC,
		invalid_tradeableshare,invalid_lr,invalid_equity,
		dShareHolder_Equity_Parent2,
		dProfitCur1,
		dShareHolder_Equity_Parent1,
		dProfitCur,
		dShareHolder_Equity_Parent,
		dProfitCur2
		);

	//算术平均价格[总股本加权] = dTotValue/dTotShare
	if(dTotShare>0)
		dAvgPriceTot = dTotValue/dTotShare;
	SetBlockPricedFutureCell(resTable,_T("平均价格(元)"),_T("加权平均价格(总股本)"),dAvgPriceTot,iValueScale,fabs(dAvgPriceTot-0)<0.000001?_T(""):sDetail,
		tot_count,countByBourse,invalid_count,
		invalid_tradestatus_count,invalid_pepb_count,sampleCount,
		invalid_share,invalid_insititutionshare,iLR,iZC,
		invalid_tradeableshare,invalid_lr,invalid_equity,
		dShareHolder_Equity_Parent2,
		dProfitCur1,
		dShareHolder_Equity_Parent1,
		dProfitCur,
		dShareHolder_Equity_Parent,
		dProfitCur2
		);

	//算术平均价格[流通股本加权] = dTradeableValue/dTotTradeableShare
	if(dTotTradeableShare>0)
		dAvgPriceTradeable = dTradeableValue/dTotTradeableShare;
	SetBlockPricedFutureCell(resTable,_T("平均价格(元)"),_T("加权平均价格(流通股本)"),dAvgPriceTradeable,iValueScale,fabs(dAvgPriceTradeable-0)<0.000001?_T(""):sDetail,
		tot_count,countByBourse,invalid_count,
		invalid_tradestatus_count,invalid_pepb_count,sampleCount,
		invalid_share,invalid_insititutionshare,iLR,iZC,
		invalid_tradeableshare,invalid_lr,invalid_equity,
		dShareHolder_Equity_Parent2,
		dProfitCur1,
		dShareHolder_Equity_Parent1,
		dProfitCur,
		dShareHolder_Equity_Parent,
		dProfitCur2
		);

	//加权平均每股收益[含亏损]=dProfitCur/dTotShareInstitution
	if(dTotShareInstitution>0)
		//dEPS = dProfitCur/dTotShareInstitution;
		dEPS = dProfitCur/dTotShare;
	SetBlockPricedFutureCell(resTable,_T("每股收益(元)"),_T("加权平均每股收益(含亏损)"),dEPS,iValueScale,fabs(dEPS-0)<0.000001?_T(""):sDetail,
		tot_count,countByBourse,invalid_count,
		invalid_tradestatus_count,invalid_pepb_count,sampleCount,
		invalid_share,invalid_insititutionshare,iLR,iZC,
		invalid_tradeableshare,invalid_lr,invalid_equity,
		dShareHolder_Equity_Parent2,
		dProfitCur1,
		dShareHolder_Equity_Parent1,
		dProfitCur,
		dShareHolder_Equity_Parent,
		dProfitCur2
		);

	//加权平均每股收益[不含亏损]=dProfitCur1/dTotShareInstitution1
	if(dTotShareInstitution1>0)
		//dEPS1 = dProfitCur1/dTotShareInstitution1;
		dEPS1 = dProfitCur1/dTotShare1;
	SetBlockPricedFutureCell(resTable,_T("每股收益(元)"),_T("加权平均每股收益(不含亏损)"),dEPS1,iValueScale,fabs(dEPS1-0)<0.000001?_T(""):sDetail,
		tot_count,countByBourse,invalid_count,
		invalid_tradestatus_count,invalid_pepb_count,sampleCount,
		invalid_share,invalid_insititutionshare,iLR,iZC,
		invalid_tradeableshare,invalid_lr,invalid_equity,
		dShareHolder_Equity_Parent2,
		dProfitCur1,
		dShareHolder_Equity_Parent1,
		dProfitCur,
		dShareHolder_Equity_Parent,
		dProfitCur2
		);

	iValueScale = 2;
	//总净资产
	//dShareHolder_Equity_Parent
	SetBlockPricedFutureCell(resTable,_T("净资产(亿元)"),_T("总净资产"),dShareHolder_Equity_Parent/TX_INT_YI,iValueScale,fabs(dShareHolder_Equity_Parent-0)<0.000001?_T(""):sDetail,
		tot_count,countByBourse,invalid_count,
		invalid_tradestatus_count,invalid_pepb_count,sampleCount,
		invalid_share,invalid_insititutionshare,iLR,iZC,
		invalid_tradeableshare,invalid_lr,invalid_equity,
		dShareHolder_Equity_Parent2,
		dProfitCur1,
		dShareHolder_Equity_Parent1,
		dProfitCur,
		dShareHolder_Equity_Parent,
		dProfitCur2
		);

	//国有股净资产
	double dShareHolder_Equity_Parent_nation = 0;
	//if(dTotShare>0)
	//	dShareHolder_Equity_Parent_nation = dShareHolder_Equity_Parent*dTotNationShare/dTotShare;

	//2009-01-12
	dShareHolder_Equity_Parent_nation = dShareHolder_Equity_Parent_nation_stat;

	//=dShareHolder_Equity_Parent*dTotNationShare/dTotShare；
	SetBlockPricedFutureCell(resTable,_T("净资产(亿元)"),_T("国有股净资产"),dShareHolder_Equity_Parent_nation/TX_INT_YI,iValueScale,fabs(dShareHolder_Equity_Parent_nation-0)<0.000001?_T(""):sDetail,
		tot_count,countByBourse,invalid_count,
		invalid_tradestatus_count,invalid_pepb_count,sampleCount,
		invalid_share,invalid_insititutionshare,iLR,iZC,
		invalid_tradeableshare,invalid_lr,invalid_equity,
		dShareHolder_Equity_Parent2,
		dProfitCur1,
		dShareHolder_Equity_Parent1,
		dProfitCur,
		dShareHolder_Equity_Parent,
		dProfitCur2
		);

	//每股净资产=dShareHolder_Equity_Parent2/dTotShare1
	double dShareHolder_Equity_Parent_ps = 0;
	if(dTotShare1>0)
		dShareHolder_Equity_Parent_ps = dShareHolder_Equity_Parent2/dTotShare1;
	SetBlockPricedFutureCell(resTable,_T("净资产(亿元)"),_T("每股净资产(元)"),dShareHolder_Equity_Parent_ps,iValueScale,fabs(dShareHolder_Equity_Parent_ps-0)<0.000001?_T(""):sDetail,
		tot_count,countByBourse,invalid_count,
		invalid_tradestatus_count,invalid_pepb_count,sampleCount,
		invalid_share,invalid_insititutionshare,iLR,iZC,
		invalid_tradeableshare,invalid_lr,invalid_equity,
		dShareHolder_Equity_Parent2,
		dProfitCur1,
		dShareHolder_Equity_Parent1,
		dProfitCur,
		dShareHolder_Equity_Parent,
		dProfitCur2
		);

	//总股本=dTotShare
	SetBlockPricedFutureCell(resTable,_T("股本(亿股)"),_T("总股本"),dTotShare/TX_INT_YI,iValueScale,fabs(dTotShare-0)<0.000001?_T(""):sDetail,
		tot_count,countByBourse,invalid_count,
		invalid_tradestatus_count,invalid_pepb_count,sampleCount,
		invalid_share,invalid_insititutionshare,iLR,iZC,
		invalid_tradeableshare,invalid_lr,invalid_equity,
		dShareHolder_Equity_Parent2,
		dProfitCur1,
		dShareHolder_Equity_Parent1,
		dProfitCur,
		dShareHolder_Equity_Parent,
		dProfitCur2
		);

	//国有股=dTotNationShare
	SetBlockPricedFutureCell(resTable,_T("股本(亿股)"),_T("其中: 国有股本"),dTotNationShare/TX_INT_YI,iValueScale,fabs(dTotNationShare-0)<0.000001?_T(""):sDetail,
		tot_count,countByBourse,invalid_count,
		invalid_tradestatus_count,invalid_pepb_count,sampleCount,
		invalid_share,invalid_insititutionshare,iLR,iZC,
		invalid_tradeableshare,invalid_lr,invalid_equity,
		dShareHolder_Equity_Parent2,
		dProfitCur1,
		dShareHolder_Equity_Parent1,
		dProfitCur,
		dShareHolder_Equity_Parent,
		dProfitCur2
		);

	//流通股本=dTotTradeableShare;
	SetBlockPricedFutureCell(resTable,_T("股本(亿股)"),_T("其中: 流通股本"),dTotTradeableShare/TX_INT_YI,iValueScale,fabs(dTotTradeableShare-0)<0.000001?_T(""):sDetail,
		tot_count,countByBourse,invalid_count,
		invalid_tradestatus_count,invalid_pepb_count,sampleCount,
		invalid_share,invalid_insititutionshare,iLR,iZC,
		invalid_tradeableshare,invalid_lr,invalid_equity,
		dShareHolder_Equity_Parent2,
		dProfitCur1,
		dShareHolder_Equity_Parent1,
		dProfitCur,
		dShareHolder_Equity_Parent,
		dProfitCur2
		);

	//sProgressPrompt+=_T(",完成!");
	//prw.SetText(progId, sProgressPrompt);
	if(bNeedPsw)
	{
		sProgressPrompt.Format(_T("定价特征[%s]..."),sMarketName);
		prw.SetPercent(progId, 1.0);
		prw.SetText(progId, sProgressPrompt);
		prw.EnableCancelButton(false);
	}

	if(countByBourse<=0)
		return false;

	return true;
}
//设置定价特征单元个数据
bool TxBusiness::SetBlockPricedFutureCell(
		Tx::Core::Table& resTable,	//定价特征计算结果
		CString sName1,	//0=大类
		CString sName2,	//1=小类
		double dValue,	//2=指标数据[沪市，深市，合计]
		int iValueScale,//2.1指标数据的小数位数
		CString sDetail,//3=查看明细[在界面上查看明细]
		int tot_count,	//4=输入样本数=tot_count
		int countByBourse,	//5=本市场样本数=countByBourse
		int invalid_count,	//6=无效样本数量=invalid_count
		int invalid_tradestatus_count,//7=当日交易状态不正常数量=invalid_tradestatus_count
		int invalid_pepb_count,	//8=PEPB数据无效数量=invalid_pepb_count
		int sampleCount,		//9=实际参与运算的样本数=sampleCount
		int invalid_share,		//10=对应股本无效数据的样本数=invalid_share
		int invalid_insititutionshare,//11=公司总股本无效数据的样本数=invalid_insititutionshare
		int iLR,//12=亏损家数=iLR
		int iZC,//13=负净资产家数=iZC
		int invalid_tradeableshare,//14=流通股本无效数据样本数=invalid_tradeableshare
		int invalid_lr,		//15无效利润数据的样本数=invalid_lr
		int invalid_equity,	//16无效股东权益数据的样本数=invalid_equity;
		double dShareHolder_Equity_Parent2,//17股东权益[不含负净资产]
		double dProfitCur1,	//18不含亏损+不含负净资产=利润
		double dShareHolder_Equity_Parent1,//19不含亏损+不含负净资产=净资产
		double dProfitCur,	//20 含亏损的利润
		double dShareHolder_Equity_Parent,//21 股东权益[含负净资产]
		double dProfitCur2	//22不含亏损利润
	)
{
	resTable.AddRow();
	int count = (int)resTable.GetRowCount();
	count--;
	int nCol = 0;
	resTable.SetCell(nCol++,count,sName1);		//0=大类
	resTable.SetCell(nCol++,count,sName2);		//1=小类

	if(iValueScale<0)
		iValueScale = 0;
	if(iValueScale>6)
		iValueScale = 6;

	//CString sValue;
	//CString sFmt;
	//sFmt.Format(_T("%c.%df"),'%',iValueScale);
	//sValue.Format(sFmt,dValue);

	resTable.SetCell(nCol++,count,
		DisplayColInfo::GetFinanceString(dValue, iValueScale));		//2=指标数据[沪市，深市，合计]

	resTable.SetCell(nCol++,count,sDetail);		//3=查看明细[在界面上查看明细]
	resTable.SetCell(nCol++,count,tot_count);	//4=输入样本数=tot_count
	resTable.SetCell(nCol++,count,countByBourse);	//5=本市场样本数=countByBourse
	resTable.SetCell(nCol++,count,invalid_count);	//6=无效样本数量=invalid_count
	resTable.SetCell(nCol++,count,invalid_tradestatus_count);//7=当日交易状态不正常数量=invalid_tradestatus_count
	resTable.SetCell(nCol++,count,invalid_pepb_count);	//8=PEPB数据无效数量=invalid_pepb_count
	resTable.SetCell(nCol++,count,sampleCount);		//9=实际参与运算的样本数=sampleCount
	resTable.SetCell(nCol++,count,invalid_share);	//10=对应股本无效数据的样本数=invalid_share
	resTable.SetCell(nCol++,count,invalid_insititutionshare);//11=公司总股本无效数据的样本数=invalid_insititutionshare
	resTable.SetCell(nCol++,count,iLR);//12=亏损家数=iLR
	resTable.SetCell(nCol++,count,iZC);//13=负净资产家数=iZC
	resTable.SetCell(nCol++,count,invalid_tradeableshare);//14=流通股本无效数据样本数=invalid_tradeableshare
	resTable.SetCell(nCol++,count,invalid_lr);//15无效利润数据的样本数=invalid_lr
	resTable.SetCell(nCol++,count,invalid_equity);//16无效股东权益数据的样本数=invalid_equity;
	resTable.SetCell(nCol++,count,dShareHolder_Equity_Parent2);//17股东权益[不含负净资产]
	resTable.SetCell(nCol++,count,dProfitCur1);	//18不含亏损+不含负净资产=利润
	resTable.SetCell(nCol++,count,dShareHolder_Equity_Parent1);//19不含亏损+不含负净资产=净资产
	resTable.SetCell(nCol++,count,dProfitCur);	//20 含亏损的利润
	resTable.SetCell(nCol++,count,dShareHolder_Equity_Parent);//21 股东权益[含负净资产]
	resTable.SetCell(nCol++,count,dProfitCur2);//22不含亏损利润
	
	return true;
}
bool TxBusiness::GetBlockPricedFuture(
		std::set<int>& iSecurityId,	//板块样本集
		Tx::Core::Table& resTable,
		int icalc_type,
		bool bNeedPsw
		)
{
	if(resTable.GetColCount()<=0)
	{
		resTable.Clear();
		//0=加入样本名称=大类
		resTable.AddCol(Tx::Core::dtype_val_string);
		//1=加入样本名称=小类
		resTable.AddCol(Tx::Core::dtype_val_string);
		//2=指标数据[沪市]
		resTable.AddCol(Tx::Core::dtype_val_string);
		//3=指标数据[深市]
		resTable.AddCol(Tx::Core::dtype_val_string);
		//4=指标数据[合计]
		resTable.AddCol(Tx::Core::dtype_val_string);
		//5=查看明细[在界面上查看明细]
		resTable.AddCol(Tx::Core::dtype_val_string);
		//6=备注
		resTable.AddCol(Tx::Core::dtype_val_string);

		//目前23项指标=23条记录
		resTable.AddRow(23);
		CString sInitStr;
		sInitStr = _T("0.00");
		for(int i=0;i<23;i++)
		{
			resTable.SetCell(2,i,sInitStr);
			resTable.SetCell(3,i,sInitStr);
			resTable.SetCell(4,i,sInitStr);
		}
	}

	/*
	//市值
	double totValue;			//总市值
	double tradeableValue;		//流通市值

	//市盈率
	double totPE1;				//含亏损，总股本加权市盈率
	double tradeablePE2;		//含亏损，流通股本加权市盈率
	double totPE3;				//不含亏损，总股本加权市盈率
	double tradeablePE4;		//不含亏损，流通股本加权市盈率

	//市净率
	double totPB1;				//含亏损，总股本加权市净率
	double tradeablePB2;		//含亏损，流通股本加权市净率
	double totPB3;				//不含亏损，总股本加权市净率
	double tradeablePB4;		//不含亏损，流通股本加权市净率

	//净资产收益率
	double totNetValueRate1;	//净资产收益率[含负净资产]
	double totNetValueRate2;	//净资产收益率[不含负净资产]

	//平均价格
	double avgPeice1;			//算术平均价
	double avgPeiceTot;		//总股本加权平均价
	double avgPeiceTadeable;	//流通股本加权平均价

	//每股收益
	double interests1;			//加权平均每股收益[含亏损]
	double interests2;			//加权平均每股收益[不含亏损]

	//净资产
	double totNetValue;		//总净资产
	double nationNetValue;		//国有股净资产
	double NetValue;			//每股净资产

	//股本
	double totShare;			//总股本
	double nationShare;		//国有股本
	double tadeableShare;		//流通股本
	*/

	long lBourseId;			//交易所ID,0表示所有样本

	for(int i=0;i<3;i++)
	{
		switch(i)
		{
		case 0:
			//沪市
			lBourseId = m_pFunctionDataManager->GetBourseId_ShangHai();
			break;
		case 1:
			//深市
			lBourseId = m_pFunctionDataManager->GetBourseId_ShenZhen();
			break;
		case 2:
			//合计
			lBourseId = 0;
			break;
		}
	Tx::Core::Table resTable1;	//定价特征计算结果
	if(GetBlockPricedFuture(
		iSecurityId,	//板块样本集
		resTable1,	//定价特征计算结果
		lBourseId,				//交易所ID,0表示所有样本
		icalc_type,				//计算方法 0=静态;1=滚动;2=简单;3=同比
		bNeedPsw
		)==false)
		continue;
	int nColSrc,nColDest;
	if(i==0)
	{
		nColSrc = 0;
		nColDest = 0;
		resTable.CopyColumnData(resTable1,nColSrc,nColDest);
		nColSrc = 1;
		nColDest = 1;
		resTable.CopyColumnData(resTable1,nColSrc,nColDest);
	}
	nColSrc = 2;
	nColDest = i+2;
	resTable.CopyColumnData(resTable1,nColSrc,nColDest);

	//合计时，增加备注和查看明细
	if(i==2)
	{
		//查看明细
		nColSrc = 3;
		nColDest = i+2+1;
		resTable.CopyColumnData(resTable1,nColSrc,nColDest);

		/*
		resTable.SetCell(nCol++,count,iLR);//12=亏损家数=iLR
		resTable.SetCell(nCol++,count,iZC);//13=负净资产家数=iZC
		double dShareHolder_Equity_Parent2,//17股东权益[不含负净资产]
		double dProfitCur1,	//18不含亏损+不含负净资产=利润
		double dShareHolder_Equity_Parent1,//19不含亏损+不含负净资产=净资产
		double dProfitCur,	//20 含亏损的利润
		double dShareHolder_Equity_Parent//21 股东权益[含负净资产]
		double dProfitCur2,	//22不含亏损的利润
		*/

		//备注
		//临时使用，作为行索引
		//含亏损或者含负净资产的项目，增加备注
		//亏损的描述
		//负净资产的描述
		nColDest = i+2+2;
		CString sRemark;
		nColSrc = 2;//总股本加权[含亏损]市盈率
		//取得亏损家数
		int iLR = 0;
		resTable1.GetCell(12,nColSrc,iLR);

		if(iLR>0)
		{
			double dAllLR = 0;
			resTable1.GetCell(20,nColSrc,dAllLR);
			double dAllJLR = 0;
			resTable1.GetCell(22,nColSrc,dAllJLR);

			sRemark.Format(_T(" %d 家亏损, 累计亏损 %.2f 亿元"),iLR,(dAllJLR-dAllLR)/TX_INT_YI);

			//累计亏损=含亏损的总利润-不含亏损的总利润
			resTable.SetCell(nColDest,nColSrc,sRemark);
		}

		nColSrc = 6;//总股本加权[含负净资产]市净率
		//取得含负净资产家数
		int iZC = 0;
		resTable1.GetCell(13,nColSrc,iZC);
		if(iZC>0)
		{
			double dAllZC = 0;
			resTable1.GetCell(21,nColSrc,dAllZC);
			double dAllJZC = 0;
			resTable1.GetCell(17,nColSrc,dAllJZC);

			sRemark.Format(_T(" %d 家含负净资产, 累计负净资产 %.2f 亿元"),iZC,(dAllJZC-dAllZC)/TX_INT_YI);

			//累计负净资产=含负净资产的总资产-不含负净资产的总资产
			resTable.SetCell(nColDest,nColSrc,sRemark);
		}
	}

	}

	return true;
}
bool TxBusiness::GetBlockSamplesPricedFutureDetail(
		int iItem,							//指定输出数据项目
		std::set<int>& iSecurityId,			//板块样本集
		Tx::Core::Table_Indicator& resTable,//结果数据
		int calc_type //0,1,2,3
	)
{
	//样本个数
	int iCount = iSecurityId.size();

	if(iCount<=0)
		return false;

	//step1
//	Tx::Core::ProgressWnd* pProgressWnd = Tx::Core::ProgressWnd::GetInstance();
	Tx::Core::ProgressWnd prw;
	//step2
	CString sProgressPrompt;
	sProgressPrompt.Format(_T("定价特征明细..."));
	UINT progId = prw.AddItem(1,sProgressPrompt, 0.0);
	//step3
	prw.Show(15);

	resTable.Clear();

	//加入样本ID
	resTable.AddCol(Tx::Core::dtype_int4);
	//加入样本名称
	resTable.AddCol(Tx::Core::dtype_val_string);
	//加入样本外码
	resTable.AddCol(Tx::Core::dtype_val_string);

	//加入数据列
	double dValue = 0;
	resTable.AddCol(Tx::Core::dtype_double);

	//统计板块总股本和总流通股本
	double totShare=0;
	double tradeableShare=0;
	int iRow=0;
	Trade* pTrade = NULL;
	for (std::set<int>::const_iterator iter = iSecurityId.begin(); iter != iSecurityId.end(); ++iter)
	{
		dValue = Con_doubleInvalid;
		//准备数据
		//step4
		prw.SetPercent(progId, (double)iRow/(double)iSecurityId.size());

		//取得样本的交易实体ID
		int iSecurity = *iter;

		//取得样本的Security指针
		GetSecurityNow(iSecurity);
		SecurityQuotation* psq = (SecurityQuotation*)m_pSecurity;
		if(psq==NULL)
			continue;

		if(psq->IsNormal()==false)
			continue;

		//2008-06-10
		//如果是指数，则跳过
		if(psq->IsIndex()==true)
			continue;

		//最新PEPB
		PePb*	pSecurityPePb = psq->GetPePbDataLatest();
		//if(pSecurityPePb==NULL)
		//	continue;

		//取得样本的名称
		CString sSecurityName;
		sSecurityName = psq->GetName();

		//取得样本的外码
		CString sExtCode;
		sExtCode = psq->GetCode();

		//样本最新价格
		float closePrice = 0;
		closePrice = psq->GetClosePrice(true);
		if(closePrice<0) closePrice = 0;

		//样本总股本
		double share = 0;

		//样本流通股本
		double tradeableshare = 0;

		//2.30000096	对应股本
		//if(psq->GetIndicatorValueAbsolute(30000096,share)==false)
		//	share = 0;
		share = psq->GetTotalShare();
		if(share<0)
			share = 0;

		//3.30000101	对应流通股本
		//if(psq->GetIndicatorValueAbsolute(30000101,tradeableshare)==false)
		//	tradeableshare = 0;
		tradeableshare = psq->GetTradableShare();
		if(tradeableshare<0)
			tradeableshare = 0;

		//国有净资产
		double nationalShare = psq->GetNationShare();
		if(nationalShare<0)
			nationalShare = 0;

		//样本股东权益
		double dInterests = 0;
		//6.30300201	最新归属于母公司股东权益	decimal
		//if(psq->GetIndicatorValueAbsolute(30300201,dInterests)==false)
		//	dInterests = 0;
		dInterests = 0;
		InstitutionNewInfo* pInstitutionNewInfo = psq->GetInstitutionNewInfo();
		if(pInstitutionNewInfo!=NULL)
		{
			dInterests = pInstitutionNewInfo->shareholder_equity_parent;
			if(fabs(dInterests-Con_doubleInvalid)<0.000001)
				dInterests = 0;
		}
		//7.30000100	汇率
		//样本市值计算比率
		double dRatio = 1.0;
		//if(psq->GetIndicatorValueAbsolute(30000100,dRatio)==false)
		//	dRatio = 1;
		StockNewInfo* pStockNewInfo = psq->GetStockNewInfo();
		if(pStockNewInfo!=NULL)
		dRatio = pStockNewInfo->currency_exchange;
		if(fabs(dRatio-Con_doubleInvalid)<0.000001)
			dRatio = 1;

		closePrice *= (float)dRatio;

		double totValue = Con_doubleInvalid;
		if(closePrice>0 && share>0)
			totValue = closePrice*share;
		
		double tradeableshareValue = Con_doubleInvalid;
		if(closePrice>0 && tradeableshare>0)
			tradeableshareValue = closePrice*tradeableshare;

		// 2011-03-28 增加参与计算数据是 Con_doubleInvalid，则结果均为 Con_doubleInvalid
		//每股净利润
		double dEPS = 0;
		if(pSecurityPePb!=NULL)
		{
			switch(calc_type)
			{
			case 0://静态
				dEPS = pSecurityPePb->static_eps;
				break;
			case 1://滚动
				dEPS = pSecurityPePb->ltm_eps;
				break;
			case 2://简单
				dEPS = pSecurityPePb->basic_eps;
				break;
			case 3://同比
				dEPS = pSecurityPePb->yoy_eps;
				break;
			default:
				dEPS = 0;
				//invalid_lr++;
				break;
			}
		}

		//是否忽略的标志
		bool bIgnore = false;
		double dTemp=Con_doubleInvalid;
		//样本明细数据
		switch(iItem)
		{
		case 0:
			//市值
			dValue = totValue/TX_INT_YI;
			if(dValue<0)
				dValue = Con_doubleInvalid;
			break;
		case 1:
			//流通市值
			dValue = tradeableshareValue/TX_INT_YI;
			if(dValue<0)
				dValue = Con_doubleInvalid;
			break;
		case 2:
			//含亏损，总股本加权市盈率
		case 4:
			//含亏损，流通股本加权市盈率
			dValue=Con_doubleInvalid;
			if(closePrice>0 && IsZero(dEPS)==false && dEPS != Con_doubleInvalid)
				dValue = closePrice/dEPS;
			break;
		case 3:
			//不含亏损，总股本加权市盈率
		case 5:
			//不含亏损，流通股本加权市盈率
			if(dEPS>0)
			{
				dValue=Con_doubleInvalid;
				if(closePrice>0 && IsZero(dEPS)==false)
					dValue = closePrice/dEPS;
			}
			else
				bIgnore=true;
			break;
		case 6:
			//含亏损，总股本加权市净率
		case 8:
			//含亏损，流通股本加权市净率
			dValue=Con_doubleInvalid;
			if(pSecurityPePb!=NULL)
			{
				if(closePrice>0 && IsZero(pSecurityPePb->net_asset_per)==false && pSecurityPePb->net_asset_per != Con_doubleInvalid)
					dValue = closePrice/pSecurityPePb->net_asset_per;
			}
			break;
		case 7:
			//不含亏损，总股本加权市净率
		case 9:
			//不含亏损，流通股本加权市净率
			if(pSecurityPePb!=NULL)
			{
				if(pSecurityPePb->net_asset_per>0)
				{
					dValue=Con_doubleInvalid;
					if(closePrice>0 && IsZero(pSecurityPePb->net_asset_per)==false)
						dValue = closePrice/pSecurityPePb->net_asset_per;
					break;
				}
			}
			bIgnore=true;
			break;
		case 10:
			//净资产收益率[含负净资产]
			dValue=Con_doubleInvalid;
			if(dEPS != Con_doubleInvalid && pSecurityPePb!=NULL)		// 2011-03-18 for mantis 6085
			{
				if(IsZero(pSecurityPePb->net_asset_per)==false)
					dValue = dEPS/pSecurityPePb->net_asset_per*100;
			}
			break;
		case 11:
			//净资产收益率[不含负净资产]
			if(pSecurityPePb!=NULL)
			{
				if(!(dEPS<0 || pSecurityPePb->net_asset_per<0))
				{
					dValue=Con_doubleInvalid;
					if(IsZero(pSecurityPePb->net_asset_per)==false && pSecurityPePb->net_asset_per != Con_doubleInvalid)
						dValue = dEPS/pSecurityPePb->net_asset_per*100;
					break;
				}
			}
			bIgnore=true;
			break;
		case 12:
			//样本算术平均价
		case 13:
			//样本总股本加权平均价
		case 14:
			//样本流通股本加权平均价
			dValue=Con_doubleInvalid;
			if(closePrice>0)
				dValue = closePrice;
			break;
		case 15:
			//加权平均每股收益[含亏损]
			dValue = dEPS;
			break;
		case 16:
			//加权平均每股收益[不含亏损]
			if(dEPS>0)
			{
				dValue = dEPS;
			}
			else
				bIgnore=true;
			break;
		case 17:
			//净资产
			if(pSecurityPePb!=NULL && share>0 && pSecurityPePb->net_asset_per != Con_doubleInvalid && share !=Con_doubleInvalid)
				dValue = pSecurityPePb->net_asset_per*share/TX_INT_YI;
			else 
				dValue = Con_doubleInvalid;
			break;
		case 18:
			//国有净资产
			if(pSecurityPePb!=NULL && nationalShare>0 && pSecurityPePb->net_asset_per != Con_doubleInvalid && nationalShare != Con_doubleInvalid)
				dValue = pSecurityPePb->net_asset_per*nationalShare/TX_INT_YI;
			else
				dValue = Con_doubleInvalid; 
			break;
		case 19:
			//每股净资产
			if(pSecurityPePb!=NULL)
				dValue = pSecurityPePb->net_asset_per;
			else
				dValue = Con_doubleInvalid; 
			break;
		case 20:
			//总股本
			dValue = share/TX_INT_YI;
			if(dValue<0)
				dValue = Con_doubleInvalid;
			else
				dValue = Con_doubleInvalid; 
			break;
		case 21:
			//国有股本
			if (nationalShare != Con_doubleInvalid)
			{
				dValue = nationalShare/TX_INT_YI;
				if(dValue<0)
					dValue = Con_doubleInvalid;
			}
			else
				dValue = Con_doubleInvalid;
			break;
		case 22:
			//流通股本
			if (tradeableshare != Con_doubleInvalid)
			{
				dValue = tradeableshare/TX_INT_YI;
				if(dValue<0)
					dValue = Con_doubleInvalid;
			}
			else
				dValue = Con_doubleInvalid;
			break;
		case 23:
			//样本最新价格
			{
				float m_closePrice = 0;
				m_closePrice = psq->GetClosePrice(true);
				if(m_closePrice<0) m_closePrice = 0;
				dValue = m_closePrice;
			}
			break;
		default:
			return false;
			break;
		}

		//忽略=不处理当前样本，当前样本的明细数据不会进入展示界面
		if(bIgnore==true)
			continue;

		//根据当前交实体添加记录
		resTable.AddRow();
		int iCol = 0;
		//样本ID
		resTable.SetCell(iCol++,iRow,iSecurity);
		//样本Name
		resTable.SetCell(iCol++,iRow,sSecurityName);
		//样本ExtCode
		resTable.SetCell(iCol++,iRow,sExtCode);
		//样本明细数据
		resTable.SetCell(iCol++,iRow,dValue);

		iRow++;

	}
	resTable.DeleteCol(0);
	prw.SetPercent(progId, 1.0);
	sProgressPrompt+=_T(",完成!");
	prw.SetText(progId, sProgressPrompt);
	return true;
}

//取得单个指定交易实体的除权数据
bool TxBusiness::GetSecurityExdividend(
			int iSecurityId,					//交易实体ID
			Tx::Core::Table_Indicator& resTable	//取得iSecurityId的除权数据
			)
{
	/*
		//取得样本的Security指针
		GetSecurityNow(iSecurityId);
		SecurityQuotation* psq = (SecurityQuotation*)m_pSecurity;
		if(psq==NULL)
			return false;

		//return GetSecurity1Exdividend((int)psq->GetSecurity1Id(),resTable);
		return psq->GetExdividend(resTable);
		*/
		return false;
}
		//取得单个指定券的除权数据
bool TxBusiness::GetSecurity1Exdividend(
			int iSecurity1Id,					//券ID
			Tx::Core::Table_Indicator& resTable	//取得iInstitutionId的除权数据
			)
{
	/*
	int nColCount = m_AllExdividendDataTable.GetColCount();
	if(nColCount<=0)
		return false;
	UINT* nColArray = new UINT[nColCount];
	for(int i=0;i<nColCount;i++)
		nColArray[i]=i;

	
	std::set<int> iSecurity;
	iSecurity.insert(iSecurity1Id);
	resTable.CopyColumnInfoFrom(m_AllExdividendDataTable);
	m_AllExdividendDataTable.EqualsAt(resTable,nColArray,nColCount,0,iSecurity);
	delete nColArray; 
	return true;
	*/
	return false;
}
//2007-08-23
//板块热点分析
bool TxBusiness::BlockHotFocus(
	Tx::Core::Table& resTable,	//结果数据表
	std::vector<int> iBloackSamples//板块样本ID
	)
{
	//TRACE(_T("\r\n************************************************************\r\n"));
	int iSamplesCount = (int)iBloackSamples.size();

	resTable.Clear();

	//Tx::Core::HourglassWnd hourglass;
	//hourglass.Show(_T("热点分析..."));

	//0=板块样本的交易实体ID
	resTable.AddCol(Tx::Core::dtype_int4);
	//1=样本当前价格
	resTable.AddCol(Tx::Core::dtype_double);
	//2=样本涨跌幅
	resTable.AddCol(Tx::Core::dtype_double);
	//3=样本5分钟涨跌幅
	resTable.AddCol(Tx::Core::dtype_double);
	//4=样本振幅
	resTable.AddCol(Tx::Core::dtype_double);
	//5=样本换手率
	resTable.AddCol(Tx::Core::dtype_double);
	//6=样本委比
	resTable.AddCol(Tx::Core::dtype_double);
	//7=样本量比
	resTable.AddCol(Tx::Core::dtype_double);
	//8=样本成交金额
	resTable.AddCol(Tx::Core::dtype_double);
	//9=样本名称
	resTable.AddCol(Tx::Core::dtype_val_string);
	//10=样本涨跌标志
	resTable.AddCol(Tx::Core::dtype_int4);

	if(iSamplesCount<=0)
		return true;

	//2008-11-17
	//请求一次委比数据
#ifndef _DEBUG
	m_pShIndex->RequestDelegateTradeRate();
#else
	DWORD dwdtrS = GetTickCount();
	m_pShIndex->RequestDelegateTradeRate();
	TRACE("\nRequestDelegateTradeRate = %d",GetTickCount()-dwdtrS);
#endif

	int r = 0;
	Trade* pTrade = NULL;
	for (std::vector<int>::iterator iter = iBloackSamples.begin(); iter != iBloackSamples.end(); iter++)
	{
		int nCol=0;
		GetSecurityNow(*iter);
		//检查交易实体的有效性
		if(m_pSecurity==NULL)
			continue;

		//2007-09-17
		//开放式基金不予处理
		//if(m_pSecurity->InFundOpenBlock()==true)
		//	continue;

		SecurityQuotation* pSq = (SecurityQuotation*)m_pSecurity;

		//当日没有交易的不参与
		//2008-12-04展示所有有效样本
		//if(pSq->IsValid()==false)
		//	continue;


		double dClosePrice = Con_doubleInvalid;
		double dAmount = 0;
		float fValue = 0;
		//2008-12-16
		//2009-02-18测试最新5分钟涨幅，不再需要请求分时
		//pSq->GetTradeDetail();

		fValue = pSq->GetClosePrice(true);
		if(fValue>0)
			dClosePrice = fValue;
		dAmount = pSq->GetAmount();

		resTable.AddRow();

		//0=板块样本的交易实体ID
		resTable.SetCell(nCol, r, *iter);
		nCol++;
		//1=样本当前价格
		resTable.SetCell(nCol, r, dClosePrice );
		nCol++;
		//2=样本涨跌幅
		resTable.SetCell(nCol, r, pSq->GetRaise());
		nCol++;
		//3=样本5分钟涨跌幅
		resTable.SetCell(nCol, r, pSq->GetPreTradeRaise(5));
		nCol++;
		//4=样本振幅
		resTable.SetCell(nCol, r, pSq->GetAmplitude());
		nCol++;
		//5=样本换手率
		resTable.SetCell(nCol, r, pSq->GetTradeRate());
		nCol++;
		//6=样本委比
		if(pSq->IsIndex()==false)
			resTable.SetCell(nCol, r, pSq->GetDelegateTradeRateNew());
		else
			resTable.SetCell(nCol, r, Con_doubleInvalid);
		nCol++;
		//7=样本量比
		resTable.SetCell(nCol, r, pSq->GetVolumeRate());
		//resTable.SetCell(nCol, r, 0.0);
		nCol++;
		//8=样本成交金额
		resTable.SetCell(nCol, r, dAmount );
		nCol++;
		//9=样本名称
		resTable.SetCell(nCol, r, pSq->GetName(true));
		nCol++;
		//10=样本涨跌标志
		resTable.SetCell(nCol, r, pSq->GetRaiseFlag());
		nCol++;
		r++;
	}
//	hourglass.Hide();

	return true;
}
//板块风险分析
bool TxBusiness::BlockRiskIndicator(
	int				iFunID,				//功能ID
	std::set<int>& iSecurityId,			//交易实体ID
	int date,							//数据日期
	Tx::Core::Table_Indicator& resTable,//结果数据表
	bool bPrice	//现价
	)
{
	int ii = iSecurityId.size();
	//if(ii<=0)
	//	return false;

	//step1
//	Tx::Core::ProgressWnd* pProgressWnd = Tx::Core::ProgressWnd::GetInstance();
	Tx::Core::ProgressWnd prw;
	//step2
	CString sProgressPrompt;
	sProgressPrompt.Format(_T("风险分析..."));
	UINT progId = prw.AddItem(1,sProgressPrompt, 0.0);
	//step3
	prw.Show(15);
	//step4
	prw.SetPercent(progId, 0.1);

	//默认的返回值状态
	bool result = false;
	//清空数据
	resTable.Clear();
	int iCol = 0;

	//2007-08-17
	//交易实体名称
	resTable.AddParameterColumn(Tx::Core::dtype_val_string);
	//交易实体外码
	resTable.AddParameterColumn(Tx::Core::dtype_val_string);

	//准备样本集=第一参数列:F_Security_ID,int型
	resTable.AddParameterColumn(Tx::Core::dtype_int4);
	//根据样本数量添加记录数
	if(ii>0)
	resTable.AddRow(ii);
	//添加券ID
	int j=0;
	for(std::set<int>::iterator iter = iSecurityId.begin();iter!=iSecurityId.end();iter++,j++)
		resTable.SetCell(2,j,*iter);

	//2007-11-19
	//修正：该类数据与日期无关
	//2007-09-18
	//取得上证指数的最近交易日期
	//if(m_pShIndex!=NULL)
	//	date = m_pShIndex->GetTradeDateOffset(date,0);
	//数据日期参数=第二参数列;F_DATE, int型
	//iCol++;
	//resTable.AddParameterColumn(Tx::Core::dtype_int4,true);
	//resTable.SetCell(3,0,date);


	prw.SetPercent(progId, 0.3);

	long iIndicator = 30300036;	//指标=标准差

	if(iFunID==10228)
		//股票板块分析
		iIndicator = 30300036;	//指标=标准差
	else if(iFunID==10501)
		//基金板块分析
		iIndicator = 30300036;	//指标=标准差

	UINT varCfg[1];			//参数配置
	int varCount=1;			//参数个数

	varCfg[0]=2;
	//varCfg[1]=3;

	for (int i = 0; i < 6; i++)
	{
		if(i==0)
		{
			GetIndicatorDataNow(30300194);
			if (m_pIndicatorData==NULL)
			{
				prw.SetPercent(progId, 1.0);
				return false;
			}
			result = m_pLogicalBusiness->SetIndicatorIntoTable(
														m_pIndicatorData,	//指标
														varCfg,				//参数配置
														varCount,			//参数个数
														resTable	//计算需要的参数传输载体以及计算后结果的载体
						);
			if(result==false)
				break;
		}
	    GetIndicatorDataNow(iIndicator+i);
		if (m_pIndicatorData==NULL)
		{
			prw.SetPercent(progId, 1.0);
			return false;
		}
		result = m_pLogicalBusiness->SetIndicatorIntoTable(
													m_pIndicatorData,	//指标
													varCfg,				//参数配置
													varCount,			//参数个数
													resTable	//计算需要的参数传输载体以及计算后结果的载体
					);
		if(result==false)
			break;
	}
	prw.SetPercent(progId, 0.5);

	//根据之前3个步骤的设置进行数据读取，结果数据存放在table中
	if(ii>0)
	result = m_pLogicalBusiness->GetData(resTable);
#ifdef _DEBUG
	CString strTable;
	strTable = resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	if(result==false)
	{
		prw.SetPercent(progId, 1.0);
		return false;
	}
	prw.SetPercent(progId, 0.8);

	//删除内码列
	//resTable.DeleteCol(2);

	//2008-07-11
	//陈红梅：在截止日期后增加现价数据
	if(bPrice==true)
		resTable.InsertCol(4,Tx::Core::dtype_float);

	j=0;
	for(std::set<int>::iterator iter = iSecurityId.begin();iter!=iSecurityId.end();iter++,j++)
	{
		GetSecurityNow(*iter);
		if(m_pSecurity!=NULL)
		{
			//填充名称
			resTable.SetCell(0,j,m_pSecurity->GetName());
			//填充外码
			resTable.SetCell(1,j,m_pSecurity->GetCode());
			resTable.SetCell(2,j,(int)m_pSecurity->GetId());
			if(bPrice==true)
				resTable.SetCell(4,j,m_pSecurity->GetClosePrice(true));
		}
	}
	prw.SetPercent(progId, 1.0);
	sProgressPrompt+=_T(",完成!");
	prw.SetText(progId, sProgressPrompt);

	return true;
}
//行情数据同步
//以基准交易实体为参照,按照多剔少补的原则,将样本交易实体同基准交易实体进行同步
bool TxBusiness::SynTradeData(
	Security* pBase,
	Security* pSample,
	HisTradeData* pSampleSecurityTradeData,
	int baseDate,
	int iDays,
	bool bIsToday,					//截止日期是当日，且当日是交易日
	bool bSameDuration,				//截止日期是当日，且当日是交易日,与行情文件最后数据日期同周期
	int iDuration,
	bool bFirstDayInDuration
	)
{
	int iBaseDateIndex = pBase->GetTradeDateIndex(baseDate,0,iDuration,bFirstDayInDuration);
	int i = iBaseDateIndex-iDays+1;

	if(i<0)
		i=0;

	if(bIsToday==true)
	{
		//如果行情文件最后一条记录的日期和当前盘中日期不在同一个周期，则取当前盘中作为最后一个样本数据
		//总的样本数不变，则历史样本必须减少一个，最后一个样本采用盘中实时数据
		if(bSameDuration==false)
			i++;
	}

	////step1
	//Tx::Core::ProgressWnd* p = Tx::Core::ProgressWnd::GetInstance();
	////step2
	//CString sProgressPrompt;
	//sProgressPrompt.Format(_T("行情数据同步: %s->%s"),
	//	pSample->GetName(),
	//	pBase->GetName()
	//	);
	//UINT progId = p->AddItem(1,sProgressPrompt, 0.0);
	////step3
	//int itot=iBaseDateIndex-i;

	//p->Show(itot);
	//以基准交易实体为参照,逐日处理样本行情
	int iLastestDate = pSample->GetTradeDateLatest();
	int iListDate = pSample->GetIPOListedDate();
	int iSampleIndex = 0;
	for(;i<= iBaseDateIndex;i++)
	{
		////step4
		//p->SetPercent(progId, (double)i/(double)itot);
		//基准交易实体当前所引的交易日期
		int date = pBase->GetTradeDateByIndex(i,iDuration,bFirstDayInDuration);
		//根据基准交易日期取得样本行情所引
		int sampleDate = pSample->GetTradeDateOffset(date,0,iDuration,bFirstDayInDuration);
		if(sampleDate<0)
		{
			//当前停牌
			//2008-11-17
			//中国南车20081114没有行情数据
			//取最后交易日行情
			if(date>iLastestDate)
				sampleDate = iLastestDate;
			else if(date<iListDate)
				sampleDate = iListDate;
			//break;
		}
			//return false;
		//取得样本行情数据
		int sampleIndex = pSample->GetTradeDateIndex(sampleDate,0,iDuration,bFirstDayInDuration);
		HisTradeData* pData = NULL;//pSample->GetTradeDataByIndex(sampleIndex,iDuration,bFirstDayInDuration);
		if(iBaseDateIndex == i)
			pData = pSample->GetTradeDataByNatureDate(baseDate);
		else
			pData = pSample->GetTradeDataByIndex(sampleIndex,iDuration,bFirstDayInDuration);
		//HisTradeData* pData = pSample->GetTradeDataByIndex(i,iDuration,bFirstDayInDuration);
		//将数据复制到缓冲区
		if(pData!=NULL)
		{
			int iDataIndex = iSampleIndex;//iDays - (iBaseDateIndex - i + 1);
			memcpy(&pSampleSecurityTradeData[iDataIndex],pData,sizeof(HisTradeData));

			//如果日期不一致,需要修改日期和成交量和成交金额
			if(sampleDate!=date)
			{
				pSampleSecurityTradeData[iDataIndex].Amount=0;
				pSampleSecurityTradeData[iDataIndex].Date=date;
				pSampleSecurityTradeData[iDataIndex].Volume=0;
			}
		}
		iSampleIndex++;
	}
	if(bIsToday==true)
	{
		if(bSameDuration==true)
		{
			//如果行情文件最后一条记录的日期和当前盘中日期在同一个周期，则取当前盘中作为最后一个样本数据
			//行情文件最后一条记录不作为样本数据
			iSampleIndex--;
		}
		//bSameDuration==false
		//如果行情文件最后一条记录的日期和当前盘中日期不在同一个周期，则取当前盘中作为最后一个样本数据
		//行情文件最后一条记录作为倒数第二个样本数据

		//取得盘中行情
		HisTradeData todayData;
		SecurityQuotation* pQS = (SecurityQuotation*)pSample;
		todayData.Amount = pQS->GetAmount();
		if(todayData.Amount<0) todayData.Amount = 0;

		todayData.Close = pQS->GetClosePrice(true);
		todayData.Date = pBase->GetServerCurDate();

		todayData.High = pQS->GetHighPrice();
		if(todayData.High<0) todayData.High = 0;

		todayData.Low = pQS->GetLowPrice();
		if(todayData.Low<0) todayData.Low = 0;

		todayData.Open = pQS->GetOpenPrice();
		if(todayData.Open<0) todayData.Open = 0;

		todayData.Preclose = pQS->GetPreClosePrice();
		if(todayData.Preclose<0) todayData.Preclose = 0;

		todayData.Volume = pQS->GetVolume();
		if(todayData.Volume<0) todayData.Volume = 0;

		memcpy(&pSampleSecurityTradeData[iSampleIndex],&todayData,sizeof(HisTradeData));
	}
	////step5
	//p->SetPercent(progId, 1.0);
	//sProgressPrompt += _T(",完成!");
	//p->SetText(progId,sProgressPrompt);

	return true;
}
//2009-05-05
//创建板块风险分析[高级]数据表
bool TxBusiness::RiskIndicatorAdv(
	Tx::Core::Table_Indicator& resTable//结果数据表
	)
{
	resTable.Clear();
	UINT nIndex = 3;
	//// 增加三个ID参数列
	resTable.AddCol(Tx::Core::dtype_int4);
	resTable.AddCol(Tx::Core::dtype_int4);
	resTable.AddCol(Tx::Core::dtype_int4);

	//3
	int nCol = 3;
	resTable.AddCol(Tx::Core::dtype_decimal);
	resTable.AddCol(Tx::Core::dtype_decimal);
	resTable.AddCol(Tx::Core::dtype_decimal);
	resTable.AddCol(Tx::Core::dtype_decimal);
	resTable.AddCol(Tx::Core::dtype_decimal);
	resTable.AddCol(Tx::Core::dtype_decimal);

	return true;
}

//板块风险分析
bool TxBusiness::RiskIndicatorAdv(
	Tx::Core::Table_Indicator& resTable,//结果数据表
	std::set<int>& iSecurityId,			//交易实体ID
	int iEndDate,						//截止日期
	long lRefoSecurityId,				//基准交易实体ID
	bool bFirstDayInDuration,			//默认周期期初
	int iStartDate,						//起始日期
	int iTradeDaysCount,				//交易天数
	int iDuration,						//交易周期0=日；1=周；2=月；3=季；4=年
	bool bLogRatioFlag,					//计算比率的方式,true=指数比率,false=简单比率
	bool bClosePrice,					//使用收盘价
	int  iStdType,						//标准差类型1=有偏，否则无偏
	int  iOffset,						//偏移
	bool bAhead,						//复权标志,true=前复权,false=后复权
	//bool bUserRate,						//用户自定义年利率,否则取一年定期基准利率
	int bUserRate,						//0=忽略无风险收益率；1取一年定期基准利率计算无风险收益率；2用户自定义年利率计算无风险收益率
	double dUserRate,					//用户自定义年利率
	bool bDayRate,						//日利率
	bool bPrw 	,						//进度条显示与否
	bool bMean
								 )
{
#ifdef _DEBUG
	CString strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//检查样本个数
	int ii = iSecurityId.size();
	if(ii<=0)
		return false;
	//检查结果数据的列数
	if(resTable.GetColCount()<=0)
		return false;

	/*if(iTradeDaysCount>0)
		iStartDate = 0;*/

	//2007-08-28
	//默认周期期初
	//bFirstDayInDuration = false;

	//默认的返回值状态
	bool result = false;
	//清空数据
	//resTable.Clear();
	int iCol = 0;

	//增加数据列
	//交易实体名称
	resTable.InsertCol(0,Tx::Core::dtype_val_string);
	//交易实体外码
	resTable.InsertCol(0,Tx::Core::dtype_val_string);
	
	CString sLog;
	int i=0;
	//step1
	//step1.1
	//读取基准交易实体的历史行情数据
	GetSecurityNow(lRefoSecurityId);
	Security* pBaseSecurity = m_pSecurity;
	if(pBaseSecurity==NULL)
		return false;
	//取个股行情数据
	if(pBaseSecurity->LoadHisTrade()==false) return false;
	//取个股行情交易日期数据
	if(pBaseSecurity->LoadTradeDate()==false) return false;

	//取得服务器当前日期
	int iToday = pBaseSecurity->GetServerCurDate();
	bool bIsToday = false;

	if(pBaseSecurity->IsTodayTradeDate()==true && iToday==iEndDate)
	{
		//截止日期是今天,且今天是交易日
		bIsToday = true;
	}

	//get end date by base security
	//以基准交易实体为准取得截止日期
	int baseDate = pBaseSecurity->GetTradeDateOffset(iEndDate,0,iDuration,bFirstDayInDuration);
	if(baseDate<0)
		//如果是未来日期，取得最新交易日期
		baseDate = pBaseSecurity->GetTradeDateLatest();
	if(baseDate>iEndDate)
		baseDate = iEndDate;
	if(baseDate>iToday)
		baseDate = iToday;
	//以基准交易实体为准,取得截止日期的周期索引
	int baseIndex = pBaseSecurity->GetTradeDateIndex(baseDate,0,iDuration,bFirstDayInDuration);

	//2008-11-18
	//判断当日与行情数据文件中最后一条记录的日期是否在同一个周期
	bool bSameDuration = false;
	if(bIsToday==true)
	{
		if(baseDate == iEndDate)
			bSameDuration = true;
		else
		{
			int iBaseKey = SecurityTradeDate::CalcDateKey(baseDate,iDuration);
			int iEndKey = SecurityTradeDate::CalcDateKey(iEndDate,iDuration);
			if(iBaseKey == iEndKey)
				bSameDuration = true;
		}
	}

	//get start date by base security
	//基准交易实体的实际起始日期
	int startBaseDate = 0;
	//基准交易实体的实际起始索引
	int startBaseIndex = 0;
	//如果起始日期>0
	if(iStartDate>0)
	{
		if(iStartDate>iEndDate)
		{
			sLog.Format(_T("风险分析: 起始日期[%d]不能大于终止日期[%d]！"),iStartDate,iEndDate);
			Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
			return false;
		}
		if(iStartDate<iEndDate)
		{
			//根据
			//取得起始日期
			startBaseDate = pBaseSecurity->GetTradeDateOffset(iStartDate,0,iDuration,bFirstDayInDuration);
			if(startBaseDate<0)
			{
				//取第一个交易日期
				startBaseDate = pBaseSecurity->GetTradeDateByIndex(0,iDuration,bFirstDayInDuration);
				//iStartDate = 0;
				startBaseIndex = 0;
				//return false;
			}
			else
				//取得起始日期的索引
				startBaseIndex = pBaseSecurity->GetTradeDateIndex(startBaseDate,0,iDuration,bFirstDayInDuration);
			//计算实际的交易日数
			iTradeDaysCount = baseIndex - startBaseIndex;

			if(bIsToday==true)//包含当日
				iTradeDaysCount++;
		}
		else
		{
			//起止同日算一天
			iTradeDaysCount++;
		}

		if(iTradeDaysCount==0)
			return false;
	}

	//2007-12-06
	//必须多取一天行情数据
	iTradeDaysCount++;

	//取得基准总样本数
	int iBaseSecurityTradeDateCount = pBaseSecurity->GetTradeDateCount(iDuration);
	if(bIsToday == true && bSameDuration == false)
		iBaseSecurityTradeDateCount++;

	//基准样本数修正
	if(iTradeDaysCount>iBaseSecurityTradeDateCount)
		iTradeDaysCount = iBaseSecurityTradeDateCount;
	//避免分配无效内存
	if(iTradeDaysCount < 1)
		return false;
	//基准交易实体的历史行情数据
	HisTradeData* BaseSecurityTradeData = new HisTradeData[iTradeDaysCount];
	memset(BaseSecurityTradeData,0,iTradeDaysCount*sizeof(HisTradeData));
	//
	double* BaseBuf = new double[iTradeDaysCount-1];
	memset(BaseBuf,0,(iTradeDaysCount-1)*sizeof(double));

	//说明
	//如果基准交易实体的交易天数大于iTradeDaysCount，startBaseIndex=0；否则startBaseIndex>0

	//取得基准交易实体的数据
	for(i=0;i<iTradeDaysCount;i++)
	{
		//2007-08-28
		//倒序取得数据
		//首先根据baseIndex-i计算出实际的数据索引,然后才能取得数据;这个计算动作在GetTradeDataByIndex内部完成
		HisTradeData* tempHTD = NULL;
		if(i==0)
			tempHTD = pBaseSecurity->GetTradeDataByNatureDate(baseDate);
		else
			tempHTD = pBaseSecurity->GetTradeDataByIndex(baseIndex-i,iDuration,bFirstDayInDuration);
		if(tempHTD!=NULL)
		{
			if(iStartDate<=0)
			{
				if(i<iTradeDaysCount-1)
				{
					startBaseDate = tempHTD->Date;
					startBaseIndex = 1;
				}
			}
			if(bIsToday==false)
				//截止日期不是当日，且当日不是交易日
				memcpy(&BaseSecurityTradeData[iTradeDaysCount-i-1],tempHTD,sizeof(HisTradeData));
			else
			{
				if(i<iTradeDaysCount-1)
				{
					if(bSameDuration == false)
						//|||0
						memcpy(&BaseSecurityTradeData[iTradeDaysCount-i-2],tempHTD,sizeof(HisTradeData));
					else
						//||||
						memcpy(&BaseSecurityTradeData[iTradeDaysCount-i-1],tempHTD,sizeof(HisTradeData));
				}
				else
				{
					//区间第一条数据
					if(bSameDuration == true)
						memcpy(&BaseSecurityTradeData[iTradeDaysCount-i-1],tempHTD,sizeof(HisTradeData));

					//最后一条数据

					//当日是交易日，且截止日是当日
					//当日盘中数据为最后一条数据
					//取得当日盘中数据
					HisTradeData todayData;
					SecurityQuotation* pQS = (SecurityQuotation*)pBaseSecurity;
					todayData.Amount = pQS->GetAmount();
					if(todayData.Amount<0) todayData.Amount = 0;

					todayData.Close = pQS->GetClosePrice(true);
					todayData.Date = iToday;

					todayData.High = pQS->GetHighPrice();
					if(todayData.High<0) todayData.High = 0;

					todayData.Low = pQS->GetLowPrice();
					if(todayData.Low<0) todayData.Low = 0;

					todayData.Open = pQS->GetOpenPrice();
					if(todayData.Open<0) todayData.Open = 0;

					todayData.Preclose = pQS->GetPreClosePrice();
					if(todayData.Preclose<0) todayData.Preclose = 0;

					todayData.Volume = pQS->GetVolume();
					if(todayData.Volume<0) todayData.Volume = 0;

					memcpy(&BaseSecurityTradeData[iTradeDaysCount-1],&todayData,sizeof(HisTradeData));
				}
			}
		}
	}

	//step1.2
	//计算平均指数收益序列
		//行情数据和[基准交易实体]上证指数同步
			//方法1：先补后剔除
			//方法2：先剔除后补
			//个人意见，方法1合理

		//上市交易之前的记录时，收益率为零
		i=0;
		for( i=1;i<iTradeDaysCount;i++)
		{
			double dFqScale = pBaseSecurity->GetExdividendScale(
				BaseSecurityTradeData[i-1].Date,	//起始日期
				BaseSecurityTradeData[i].Date,		//终止日期
				bAhead			//前复权
				);
			BaseBuf[i-1]=CalcRatio(BaseSecurityTradeData[i-1].Close*dFqScale,BaseSecurityTradeData[i].Close,bLogRatioFlag);
#ifdef _DEBUG
			GlobalWatch::_GetInstance()->WatchHere(_T("zhaohongjun|| base buf [%d] = %.6f, Date = %d"),i-1,BaseBuf[i-1],BaseSecurityTradeData[i].Date );
#endif
		}

	//step1.3
	//计算无风险收益序列
		//bool bDayRate=true;
		double* BaseNoRiskBuf = new double[iTradeDaysCount-1];
		memset(BaseNoRiskBuf,0,(iTradeDaysCount-1)*sizeof(double));

		//2008-01-04
		//计算指定日期所在年的总天数 days
		int iDaysOfYear = 0;

		//double dUserRate = 0.034;

		//for(i=startBaseIndex;i<iTradeDaysCount;i++)
		if(bUserRate!=0)
		{
			for(i=0;i<(iTradeDaysCount-1);i++)
			{
				//假设数据为[date,rate]
				//
				if(bDayRate==true)//每日利率
				{
					//计算指定日期所在年的总天数 days
					//ZkkLib::DateTime curDateTime = ZkkLib::DateTime(BaseSecurityTradeData[i].Date);
					ZkkLib::DateTime curDateTime = ZkkLib::DateTime(BaseSecurityTradeData[i+1].Date);
					
					if(bUserRate==2)//用户自定义方式
					{
						//用户设定年利率 dUserRate
						//计算指定日期所在年的总天数 days
						//日利率=in/days/100
						//iDaysOfYear = curDateTime.GetDayOfYear();
						if(ZkkLib::DateTime::IsLeapYear(curDateTime.GetYear())==TRUE)
							iDaysOfYear = 366;
						else
							iDaysOfYear = 365;
						if(iDaysOfYear>0)
							BaseNoRiskBuf[i]=dUserRate/iDaysOfYear/100;
					}
					else if(bUserRate==1)//基准利率信息表:1年定期利率
					{
						//取得某日对应的实际利率[日利率]
						//定期存款方式
						//没有数据，暂时使用常量

						//取得指定日期的索引
						//int iInIndex = BasicInterest::GetInstance()->GetIndexByDate(BaseSecurityTradeData[i].Date,8,1);
						int iInIndex = BasicInterest::GetInstance()->GetIndexByDate(BaseSecurityTradeData[i+1].Date,8,1);
						if(iInIndex<0)
							BaseNoRiskBuf[i]=0;
						else
						{
							//取得某日对应的年利率 in
							BasicInterestData*	pB = BasicInterest::GetInstance()->GetDataByIndex(iInIndex);
							if(pB!=NULL)
							{
								//取得指定索引的利率
								//计算指定日期所在年的总天数 days
								//日利率=in/days/100
								//iDaysOfYear = curDateTime.GetDayOfYear();
								if(ZkkLib::DateTime::IsLeapYear(curDateTime.GetYear())==TRUE)
									iDaysOfYear = 366;
								else
									iDaysOfYear = 365;
								if(iDaysOfYear>0)
									BaseNoRiskBuf[i]=pB->dInterest/iDaysOfYear/100;
							}
							else
								BaseNoRiskBuf[i]=0;
						}
					}
				}
				else
				{
				//区间利率
					//区间起始日期
					int idStartDate=0;
					//取指定日期的前一个交易日期
					//idStartDate = pBaseSecurity->GetTradeDateOffset(BaseSecurityTradeData[i].Date,-1,iDuration,bFirstDayInDuration);
					//idStartDate = pBaseSecurity->GetTradeDateOffset(BaseSecurityTradeData[i+1].Date,-1,iDuration,bFirstDayInDuration);
					int oldStartDate = BaseSecurityTradeData[i].Date;
					idStartDate = SecurityTradeDate::CalcEndDate(oldStartDate,iDuration);

					//if(idStartDate<=0)
						//取第一个交易日期
					//	idStartDate = pBaseSecurity->GetTradeDateByIndex(0,iDuration,bFirstDayInDuration);

					//区间终止日期
					//int idEndDate=BaseSecurityTradeData[i].Date;
					int oldEndDate=BaseSecurityTradeData[i+1].Date;
					int idEndDate=0;
					idEndDate = SecurityTradeDate::CalcEndDate(oldEndDate,iDuration);
					if(idEndDate>iEndDate)
						idEndDate = oldEndDate;


					ZkkLib::DateTime ddDateTime1 = ZkkLib::DateTime(idStartDate);
					ZkkLib::DateTime ddDateTime2 = ZkkLib::DateTime(idEndDate);

					//取得某时间段的累积利率[计尾不计头]=用户自定义日利率之和
					while(idStartDate > 1900 && idEndDate > 1900 && ddDateTime1<=ddDateTime2)
					{
						if(bUserRate==2)//用户自定义方式
						{
							//iDaysOfYear = ddDateTime1.GetDayOfYear();
							if(ZkkLib::DateTime::IsLeapYear(ddDateTime1.GetYear())==TRUE)
								iDaysOfYear = 366;
							else
								iDaysOfYear = 365;
							if(iDaysOfYear>0)
								BaseNoRiskBuf[i]+=dUserRate/iDaysOfYear/100;
						}
						else if(bUserRate==1)//基准利率信息表:1年定期利率
						{
							//取得某时间段的累积利率[计尾不计头]=日利率之和
							if(ddDateTime1>=ddDateTime2)
								break;

							//定期存款方式
							//没有数据，暂时使用常量
							int iInIndex = BasicInterest::GetInstance()->GetIndexByDate(ddDateTime1.GetDate().GetInt(),8,1);
							//取得某日对应的年利率 in
							BasicInterestData*	pB = BasicInterest::GetInstance()->GetDataByIndex(iInIndex);
							if(pB!=NULL)
							{
								//取得指定索引的利率
								//计算指定日期所在年的总天数 days
								//日利率=in/days/100
								//iDaysOfYear = ddDateTime1.GetDayOfYear();
								if(ZkkLib::DateTime::IsLeapYear(ddDateTime1.GetYear())==TRUE)
									iDaysOfYear = 366;
								else
									iDaysOfYear = 365;
								if(iDaysOfYear>0)
									BaseNoRiskBuf[i]+=pB->dInterest/iDaysOfYear/100;
							}
						}
						ddDateTime1+=ZkkLib::TimeSpan(1,0,0,0);
					}
				}
	#ifdef _DEBUG
				GlobalWatch::_GetInstance()->WatchHere(_T("zhaohongjun|| BaseNoRiskBuf [%d] = %.6f, Date = %d"),i,BaseNoRiskBuf[i],BaseSecurityTradeData[i+1].Date);
	#endif
			}
		}

	//step2
	//计算样本的收益率数据序列
	HisTradeData* SampleSecurityTradeData = new HisTradeData[iTradeDaysCount];
	memset(SampleSecurityTradeData,0,iTradeDaysCount*sizeof(HisTradeData));
	double* SampleBuf = new double[iTradeDaysCount-1];
	memset(SampleBuf,0,(iTradeDaysCount-1)*sizeof(double));

	Tx::Core::CStat_Math* pMath = new Tx::Core::CStat_Math;

	pMath->m_offset = 	iOffset;
	pMath->m_std_type = iStdType;

	int j=0;
	//step1
//	Tx::Core::ProgressWnd* p = Tx::Core::ProgressWnd::GetInstance();
	Tx::Core::ProgressWnd prw;
	prw.EnableCancelButton(true);
	//step2
	CString sProgressPrompt;
	sProgressPrompt =  _T("风险分析计算");
	UINT progId; 
	if(!bPrw)
	{	progId = prw.AddItem(1,sProgressPrompt, 0.0);	
		//step3
		prw.Show(15);
	}
	

	bool bRet = true;
	for(std::set<int>::iterator iter = iSecurityId.begin();iter!=iSecurityId.end();iter++)
	{
		//step4
		if(!bPrw)
		{
			if(prw.IsCanceled())
			{
				bRet = false;
				break;
			}
			prw.SetPercent(progId, (double)j/(double)ii);
		}
		//resTable.AddRow();

		//step2.1
		//读取样本历史行情数据
		GetSecurityNow(*iter);
		Security* pSecurity = m_pSecurity;
		if(pSecurity==NULL)
			continue;
		if(pSecurity->IsIssued()==true)
			continue;
		if(pSecurity->IsIndex())
			continue;
		/*if(pSecurity->IsShenzhen()==false && pSecurity->IsShanghai()==false)
		continue;*/

		//取个股行情数据
		pSecurity->LoadHisTrade();
		//取个股行情交易日期数据
		pSecurity->LoadTradeDate();
		//取得个股总样本数
		int iSecurityTradeDateCount = pSecurity->GetTradeDateCount(iDuration);
		////取得截止日期的索引
		//int sampleDate = pSecurity->GetTradeDateOffset(iEndDate,0);
		//int sampleIndex = pBaseSecurity->GetTradeDateIndex(sampleDate);

		////取得样本数据
		//for(i=0;i<iTradeDaysCount;i++)
		//{
		//	HisTradeData* tempHTD = pSecurity->GetTradeDataByIndex(sampleIndex-i);
		//	if(tempHTD!=NULL)
		//	memcpy(&SampleSecurityTradeData[iTradeDaysCount-i-1],tempHTD,sizeof(HisTradeData));
		//}

		//step2.2
		//行情数据同步
		//行情数据和[基准交易实体]上证指数同步
		//方法1：先补后剔除
		//方法2：先剔除后补
		//个人意见，方法1合理
		if(SynTradeData(
			pBaseSecurity,
			m_pSecurity,
			SampleSecurityTradeData,
			baseDate,
			iTradeDaysCount,
			bIsToday,
			bSameDuration,
			iDuration,
			bFirstDayInDuration
			)==false)
			continue;

		//step2.3
		//计算收益率序列
		//2007-09-20
		//对于没有行情数据的券种只能采用收盘价，比如部分开放式基金,或者按照净值计算
		//这里的处理：如果成交量为0，则采用收盘价
		//计算复权因子
		double dFqScale = 1.0;
		//dFqScale = pSecurity->GetExdividendScale(
		//	startBaseDate,	//起始日期
		//	baseDate,		//终止日期
		//	bAhead			//前复权
		//	);
		if(bClosePrice==true)
		{
			//收盘价
			i=0;
			//CalcRatio(Preclose,Close);
			for( i=1;i<iTradeDaysCount;i++)
			{
				dFqScale = pSecurity->GetExdividendScale(
					SampleSecurityTradeData[i-1].Date,	//起始日期
					SampleSecurityTradeData[i].Date,		//终止日期
					bAhead			//前复权
					);
				SampleBuf[i-1]=CalcRatio(SampleSecurityTradeData[i-1].Close*dFqScale,SampleSecurityTradeData[i].Close,bLogRatioFlag);
#ifdef _DEBUG
				GlobalWatch::_GetInstance()->WatchHere(_T("zhaohongjun|| SampleBuf[true] [%d] = %.6f date = %d"),i-1,SampleBuf[i-1],SampleSecurityTradeData[i].Date);
#endif
			}
		}
		else
		{
			//采用均价

			//CalcRatio(x,y);
			for( i=0;i<(iTradeDaysCount-1);i++)
			{
				float x=0,y=0;
				if(SampleSecurityTradeData[i].Volume>0)
				{
					//正常交易
					//int itDateIndex = pSecurity->GetTradeDateIndex(SampleSecurityTradeData[i].Date,0,iDuration,bFirstDayInDuration);
					int itDateIndex = pSecurity->GetTradeDateIndex(SampleSecurityTradeData[i+1].Date,0,iDuration,bFirstDayInDuration);
					HisTradeData* pData = pSecurity->GetTradeDataByIndex(itDateIndex-1,iDuration,bFirstDayInDuration);

					//x = 第 n-1 天的均价的复权价
					if(pData!=NULL && pData->Volume>0 )
						x = (float)(pData->Amount/pData->Volume*dFqScale);

					//y = 第 n 天的均价的复权价
					//y = (float)(SampleSecurityTradeData[i].Amount/SampleSecurityTradeData[i].Volume*dFqScale);
					if(SampleSecurityTradeData[i].Volume>0)
						y = (float)(SampleSecurityTradeData[i].Amount/SampleSecurityTradeData[i].Volume*dFqScale);
				}
				else
					//停牌
					//如果停牌使用x=close
					//如果停牌使用y=close
					//x = y = SampleSecurityTradeData[i].Close;
					x = y = SampleSecurityTradeData[i+1].Close;

				SampleBuf[i]=CalcRatio(x,y,bLogRatioFlag);
#ifdef _DEBUG
				GlobalWatch::_GetInstance()->WatchHere(_T("zhaohongjun|| SampleBuf[false] [%d] = %.6f,date=%d"),i,SampleBuf[i],SampleSecurityTradeData[i+1].Date);
#endif
			}
		}

		//step3
		//计算指标
		//int iListDate = pSecurity->GetIPOListedDate();
		int iListDate = pSecurity->GetTradeDateByIndex(0,iDuration,bFirstDayInDuration);
		int iSampleListIndex = pBaseSecurity->GetTradeDateIndex(iListDate,0,iDuration,bFirstDayInDuration);
		int iSampleCount = baseIndex - iSampleListIndex;
		if(bIsToday == true && bSameDuration == false)
			iSampleCount++;

		//iSecurityTradeDateCount
		int iStartBufPos = 0;
		int iDataCount = iTradeDaysCount-1;
		if(iSampleCount<iDataCount)
		{
			int iDif = iDataCount-iSampleCount;
			iDataCount = iSampleCount;
			iStartBufPos += iDif;
		}

		double *pSampleBuf = SampleBuf+iStartBufPos;
		double *pBaseNoRiskBuf = BaseNoRiskBuf+iStartBufPos;
		double *pBaseBuf = BaseBuf+iStartBufPos;
		/*
		//计算样本标准差 = CStat_Math::Std(iTradeDaysCount,样本的收益率数据序列);
		double std = pMath->Std(iDataCount,SampleBuf);
		//计算样本Beta系数 = CStat_Math::Beta(iTradeDaysCount,样本的收益率数据序列,无风险收益序列,平均指数收益序列);
		double beta = pMath->Beta(iDataCount,SampleBuf,BaseNoRiskBuf,BaseBuf);
		//计算样本Alpha系数	= CStat_Math::Alpha(iTradeDaysCount,Beta系数,样本的收益率数据序列,无风险收益序列,平均指数收益序列);
		double alpha = pMath->Alpha(iDataCount,beta,SampleBuf,BaseNoRiskBuf,BaseBuf);
		//如果Beta系数=0
		//	 Beta系数= CStat_Math::Beta(iTradeDaysCount,样本的收益率数据序列,无风险收益序列,平均指数收益序列);
		//计算样本Sharpe系数 = CStat_Math::Sharpe(m_item_count,标准差,样本的收益率数据序列,无风险收益序列);
		double sharp = pMath->Sharpe(iDataCount,std,SampleBuf,BaseNoRiskBuf);
		//计算样本Treynor系数 = CStat_Math::Treynor(m_item_count,Beta系数,样本的收益率数据序列,无风险收益序列);
		double treynor = pMath->Treynor(iDataCount,beta,SampleBuf,BaseNoRiskBuf);
		//计算样本跟踪误差  = CStat_Math::TrackError(m_item_count,样本的收益率数据序列,平均指数收益序列);
		double trackerror = pMath->TrackError(iDataCount,SampleBuf,BaseBuf);
		*/
		//计算样本标准差 = CStat_Math::Std(iTradeDaysCount,样本的收益率数据序列);
		double std = pMath->Std(iDataCount,pSampleBuf);
		//计算样本Beta系数 = CStat_Math::Beta(iTradeDaysCount,样本的收益率数据序列,无风险收益序列,平均指数收益序列);
		double beta = pMath->Beta(iDataCount,pSampleBuf,pBaseNoRiskBuf,pBaseBuf);
		//计算样本Alpha系数	= CStat_Math::Alpha(iTradeDaysCount,Beta系数,样本的收益率数据序列,无风险收益序列,平均指数收益序列);
		double alpha = pMath->Alpha(iDataCount,beta,pSampleBuf,pBaseNoRiskBuf,pBaseBuf);
		//如果Beta系数=0
		//	 Beta系数= CStat_Math::Beta(iTradeDaysCount,样本的收益率数据序列,无风险收益序列,平均指数收益序列);
		//计算样本Sharpe系数 = CStat_Math::Sharpe(m_item_count,标准差,样本的收益率数据序列,无风险收益序列);
		double sharp = pMath->Sharpe(iDataCount,std,pSampleBuf,pBaseNoRiskBuf);
		//计算样本Treynor系数 = CStat_Math::Treynor(m_item_count,Beta系数,样本的收益率数据序列,无风险收益序列);
		double treynor = pMath->Treynor(iDataCount,beta,pSampleBuf,pBaseNoRiskBuf);
		//计算样本跟踪误差  = CStat_Math::TrackError(m_item_count,样本的收益率数据序列,平均指数收益序列);
		double trackerror = pMath->TrackError(iDataCount,pSampleBuf,pBaseBuf);
		//标准差,alpha,beta,sharp,treynor,trackerror
		
		//2007-08-22
		//有效数据添加到表
		resTable.AddRow();

		//2007-08-17
		int nCol=0;
		//0 name
		resTable.SetCell(nCol,j,pSecurity->GetName());
		nCol++;
		//1 ExtCode
		resTable.SetCell(nCol,j,pSecurity->GetCode());
		nCol++;
		//2 securityid
		resTable.SetCell(nCol,j,(int)pSecurity->GetId());
		nCol++;
		//4 std
		resTable.SetCell(nCol,j,std);
		nCol++;
		resTable.SetCell(nCol,j,alpha);
		nCol++;
		resTable.SetCell(nCol,j,beta);
		nCol++;
		resTable.SetCell(nCol,j,sharp);
		nCol++;
		resTable.SetCell(nCol,j,treynor);
		nCol++;
		resTable.SetCell(nCol,j,trackerror);
		nCol++;
		//2007-08-22
		//只有满足条件并且已经计算出结果的交易实体才展示
		//2013-07-09
		//Excel引擎函数需要的收益均值数据，其他地方暂未用到
		if (bMean)
		{
			resTable.AddCol(Tx::Core::dtype_decimal);
			double dMean = pMath->Avr(iDataCount,pSampleBuf);
			resTable.SetCell(nCol,j,dMean);
			nCol++;
		}
		j++;
	}

	//step5
	if(!bPrw)
	{
		prw.SetPercent(progId, 1.0);
		sProgressPrompt += _T(",完成!");
		prw.SetText(progId,sProgressPrompt);
	}
	

	delete pMath;
	delete BaseSecurityTradeData;
	delete BaseBuf;
	delete BaseNoRiskBuf;

	delete SampleSecurityTradeData;
	delete SampleBuf;
	return bRet;
}
//2007-10-08
//板块阶段行情[涨幅]
//[2007-07-13]测试通过
bool TxBusiness::BlockCycleRate(
	std::set<int>& iSecurityId,		//交易实体ID
	int date,							//数据日期
	Tx::Core::Table_Indicator& resTable,//结果数据表
	bool bPrice
	)
{
	int ii = iSecurityId.size();

	//step1
//	Tx::Core::ProgressWnd* pProgressWnd = Tx::Core::ProgressWnd::GetInstance();
	Tx::Core::ProgressWnd prw;
	//step2
	CString sProgressPrompt;
	sProgressPrompt.Format(_T("阶段表现..."));
	UINT progId = prw.AddItem(1,sProgressPrompt, 0.0);
	//step3
	prw.Show(15);
	//step4
	prw.SetPercent(progId, 0.1);

	//默认的返回值状态
	bool result = false;
	//清空数据
	resTable.Clear();
	int iCol = 0;

	//2007-08-17
	//样本名称
	resTable.AddParameterColumn(Tx::Core::dtype_val_string);
	//样本外码
	resTable.AddParameterColumn(Tx::Core::dtype_val_string);

	//准备样本集=第一参数列:F_Security_ID,int型
	resTable.AddParameterColumn(Tx::Core::dtype_int4);
	//根据样本数量添加记录数
	resTable.AddRow(ii);
	//添加券ID
	int j=0;
	for(std::set<int>::iterator iter = iSecurityId.begin();iter!=iSecurityId.end();iter++,j++)
		//resTable.SetCell(0,j,iSecurityId[j]);
		resTable.SetCell(2,j,*iter);

	//2007-11-19
	//修正：该类数据与日期无关
	//2007-09-18
	//取得上证指数的最近交易日期
	//if(m_pShIndex!=NULL)
	//	date = m_pShIndex->GetTradeDateOffset(date,0);
	//数据日期参数=第二参数列;F_DATE, int型
	//iCol++;
	//resTable.AddParameterColumn(Tx::Core::dtype_int4,true);
	//resTable.SetCell(3,0,date);

	prw.SetPercent(progId, 0.3);

	long iIndicator = 30300042;	//指标=本周以来涨幅
	UINT varCfg[1];			//参数配置
	int varCount=1;			//参数个数
	for (int i = 0; i < 14; i++)
	{
		if(i==0)
		{
			GetIndicatorDataNow(30300195);
			if (m_pIndicatorData==NULL)
			{
				prw.SetPercent(progId, 1.0);
				return false;
			}
			varCfg[0]=2;
			//varCfg[1]=3;
			result = m_pLogicalBusiness->SetIndicatorIntoTable(
														m_pIndicatorData,	//指标
														varCfg,				//参数配置
														varCount,			//参数个数
														resTable
														//resTable	//计算需要的参数传输载体以及计算后结果的载体
						);
			if(result==false)
				break;
		}
	    GetIndicatorDataNow(iIndicator+i);
		if (m_pIndicatorData==NULL)
		{
			prw.SetPercent(progId, 1.0);
		    return false;
		}
		varCfg[0]=2;
		//varCfg[1]=3;
		result = m_pLogicalBusiness->SetIndicatorIntoTable(
													m_pIndicatorData,	//指标
													varCfg,				//参数配置
													varCount,			//参数个数
													resTable
													//resTable	//计算需要的参数传输载体以及计算后结果的载体
					);
		if(result==false)
			break;
	}
	prw.SetPercent(progId, 0.5);
	if(ii<=0)
	{
		prw.SetPercent(progId, 1.0);
		return false;
	}

	//根据之前3个步骤的设置进行数据读取，结果数据存放在table中
	result = m_pLogicalBusiness->GetData(resTable);
	if(result==false)
	{
		prw.SetPercent(progId, 1.0);
		return false;
	}

	//resTable.DeleteCol(2);

	//2007-11-28 cenxw add
	//resTable.AddCol(Tx::Core::dtype_int4);
	if(bPrice==true)
		resTable.InsertCol(4,Tx::Core::dtype_float);

	j=0;
	for(std::set<int>::iterator iter = iSecurityId.begin();iter!=iSecurityId.end();iter++,j++)
	{
		GetSecurityNow(*iter);
		if(m_pSecurity!=NULL)
		{
			//填充名称
			resTable.SetCell(0,j,m_pSecurity->GetName());
			//填充外码
			resTable.SetCell(1,j,m_pSecurity->GetCode());
			//resTable.SetCell(resTable.GetColCount()-1,j,(int)m_pSecurity->GetId());
			if(bPrice==true)
				resTable.SetCell(4,j,m_pSecurity->GetClosePrice(true));
		}
		else
		{
#ifdef _DEBUG
			GlobalWatch::_GetInstance()->WatchHere(_T("bond security id = %d non-exist"),*iter);
#endif
		}
	}
	prw.SetPercent(progId, 1.0);
	sProgressPrompt+=_T(",完成!");
	prw.SetText(progId, sProgressPrompt);

	return true;
}
bool TxBusiness::BlockCycleRateAdv(
	std::vector<int>& iSecurityId,			//交易实体ID
	int startDate,						//起始日期
	int endDate,						//终止日期
	bool bCutFirstDateRaise,			//剔除首日涨幅
	int	iFQLX,							//复权类型 0-不复权;1=后复权
	Tx::Core::Table_Indicator& resTable,//结果数据表
	int iPrecise,						//小数点位数
	int iFlag,						//计算类型0-默认；1-债券[bCutFirstDateRaise=true表示全价模式,bCutFirstDateRaise=false表示净价模式]
	int iMarketId,					//交易所ID
	bool bOnlyRaise,				//只计算涨幅
	bool bUseNewlyData
	)
{
	std::set<int> iId;
	for(std::vector<int>::iterator iter=iSecurityId.begin();iter!=iSecurityId.end();iter++)
		iId.insert(*iter);
	return BlockCycleRateAdv(
		iId,				//交易实体ID
		startDate,			//起始日期
		endDate,			//终止日期
		bCutFirstDateRaise,	//剔除首日涨幅
		iFQLX,				//复权类型 0-不复权;1=后复权
		resTable,			//结果数据表
		iPrecise,
		iFlag,				//计算类型0-默认；1-债券[bCutFirstDateRaise=true表示全价模式,bCutFirstDateRaise=false表示净价模式]
		iMarketId,			//交易所ID
		bOnlyRaise,
		bUseNewlyData
		);
}

//板块阶段行情[涨幅][高级]
bool TxBusiness::BlockCycleRateAdv(
	std::set<int>& iSecurityId,			//交易实体ID
	int startDate,						//起始日期
	int endDate,						//终止日期
	bool bCutFirstDateRaise,			//剔除新股首日涨幅
	int	iFQLX,							//复权类型 0:不复权;1:前复权;2:后复权;3:全复权+后复权;4:全复权+前复权
	Tx::Core::Table_Indicator& resTable,//结果数据表
	int iPrecise,						//小数点位数
	int iFlag,							//计算类型0-默认；1-债券[bCutFirstDateRaise=true表示全价模式,bCutFirstDateRaise=false表示净价模式]
	int iMarketId,						//交易所ID
	bool bOnlyRaise,					//只计算涨幅
	bool bUseNewlyData
				)
{
#ifdef _DEBUG
	//CString strIn;
	//strIn = _T("\n计\n\n长\n\n\n");
	//std::vector<CString> arrOut;
	//Tx::Core::Commonality::String().Split(strIn, arrOut, _T("\n"));
//endDate=20080801;
#endif
	
	CString msg;
	msg.Format(_T("IndexWnd6::BlockCycleRateAdv--复权类型:%d;计算类型:%d;交易所ID:%d"),iFQLX,iFlag,iMarketId);
	CLogRecorder::GetInstance()->WriteToLog(msg);
	//清空表
	resTable.Clear();

	//创建结果数据列
	//0 security id
	resTable.AddCol(Tx::Core::dtype_int4);
	//1 name
	resTable.AddCol(Tx::Core::dtype_val_string);
	//2 extcode外码
	resTable.AddCol(Tx::Core::dtype_val_string);
	//3 阶段涨跌幅（％）：= (收盘价 - 前收价) / 前收价 * 100
	resTable.AddCol(Tx::Core::dtype_double);
	//4涨跌(元)
	resTable.AddCol(Tx::Core::dtype_double);
	//5 阶段前收价：=preclose
	resTable.AddCol(Tx::Core::dtype_float);
	//6 阶段开盘价：=Open
	resTable.AddCol(Tx::Core::dtype_float);
	//7 阶段收盘价：=Close
	resTable.AddCol(Tx::Core::dtype_float);
	//8 阶段成交量（万股）：= SumVolume2- SumVolume1
	resTable.AddCol(Tx::Core::dtype_double);
	//9 阶段成交额（亿元）：= SumAmount2- SumAmount1
	resTable.AddCol(Tx::Core::dtype_double);

	//10换手率(%)
	resTable.AddCol(Tx::Core::dtype_double);

	//11交易日
	resTable.AddCol(Tx::Core::dtype_int4);
	//12日换手
	resTable.AddCol(Tx::Core::dtype_double);

	//13 阶段均价：= 成交金额 / 成交量
	resTable.AddCol(Tx::Core::dtype_double);
	//14 日均成交量（万股）：=阶段成交量/交易日天数
	resTable.AddCol(Tx::Core::dtype_double);
	//15 日均成交额（亿元）：=阶段成交额/交易日天数
	resTable.AddCol(Tx::Core::dtype_double);

	double dCustomIndexAvgPrice = 0;

	CString sLog;
	//2008-04-30
	//上午添加
	//下午徐勉要求取消
	//if(AfxMessageBox(_T("阶段行情统计会占用较长时间，是否继续？"),MB_OKCANCEL)!=IDOK)
	//	return false;
	int ii= iSecurityId.size();
	if(ii<=0)
	{
		sLog = _T("阶段行情: 样本不能为空，请选择样本！");
		AfxMessageBox(sLog);
		Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
		return false;
	}
	//bug:12314      2012-08-08
	CTime tTemp = CTime::GetCurrentTime();
	int nTime = tTemp.GetYear()*10000 + tTemp.GetMonth()*100 + tTemp.GetDay();
	if(startDate > nTime || endDate > nTime)
	{
		sLog.Format(_T("阶段行情: 起始日期或终止日期不能选取未来日期，请重新输入!"));
		AfxMessageBox(sLog);
		return false;
	}

	if(endDate<startDate) 
	{
		sLog.Format(_T("阶段行情: 终止日期[%d]应该大于等于起始日期[%d]，请重新输入！"),endDate,startDate);
		AfxMessageBox(sLog);
		Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
		return false;
	}

	//构建两市场交易日期序列
	if(GetHuShenTradeDate()==false)
	{
		sLog.Format(_T("阶段行情: 大盘交易日序列数据需要更新，建议重新启动程序！"));
		AfxMessageBox(sLog);
		Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
		return false;
	}

	//4000208=证指数
	//取得上证指数的交易实体指针
	//Security* pBaseSecurity;
	//pBaseSecurity = m_pShIndex;
	if(iMarketId == 0)
		iMarketId = m_pFunctionDataManager->GetBourseId_ShangHai();

	//服务器当前日期
	int iToday = m_pFunctionDataManager->GetServerCurDateTime(iMarketId).GetDate().GetInt();
	//数据日期
	int iDataToday = m_pFunctionDataManager->GetCurDataDateTime(iMarketId).GetDate().GetInt();

	bool bIsToday = false;
	//仅当日，交易日
	bool bOnlyToday = false;
	//当日是否交易日
	if(m_pFunctionDataManager->IsTodayTradeDate(iMarketId)==true && iToday==endDate && iDataToday==iToday)
	{
		bIsToday = true;
		if(endDate==startDate)
			bOnlyToday = true;
	}

	//取得两市场的起始日期和最新日期
	//大盘交易天数
	int iMixAllCount = m_pMixTradeDay->GetTradeDayCount();
	//大盘起始日期
	int iBaseStartDate = m_pMixTradeDay->GetDateByIndex(0);
	//大盘终止日期=最新交易日期
	int iBaseEndDate = m_pMixTradeDay->GetLatestDate();

	//取得指定日期的索引值
	int iStartIndex = m_pMixTradeDay->GetIndexByDate(startDate);
	if(iStartIndex<0)
		iStartIndex = m_pMixTradeDay->SearchNearestIndex(startDate);
	if(iStartIndex<0)
	{
		sLog.Format(_T("阶段行情: 起始日期[%d]不在大盘交易日序列中，大盘交易日序列数据需要更新，建议重新启动程序！"),startDate);
		AfxMessageBox(sLog);
		Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
		return false;
	}
	int iEndIndex = m_pMixTradeDay->GetIndexByDate(endDate);
	if(iEndIndex<0)
	{
		iEndIndex = m_pMixTradeDay->SearchNearestIndex(endDate);
	}
	if(iEndIndex<0)
	{
		sLog.Format(_T("阶段行情: 终止日期[%d]不在大盘交易日序列中，大盘交易日序列数据需要更新，建议重新启动程序！"),endDate);
		AfxMessageBox(sLog);
		Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
		return false;
	}

	/*
	//取得起始日期=交易日期
	int iStartDate = pBaseSecurity->GetTradeDateOffset(startDate,0);
	if(iStartDate<0)
		return false;
	//取得终止日期=交易日期
	int iEndDate = pBaseSecurity->GetTradeDateOffset(endDate,0);
	if(iEndDate<0)
		iEndDate = pBaseSecurity->GetTradeDateLatest();
	if(iEndDate<0)
		return false;
	*/
	int iEndDate = endDate;
	iEndDate = m_pMixTradeDay->GetDateByIndex(iEndIndex);

	//2008-05-15
	//是否使用今日横向数据
	//bool bUseNewlyData = false;
	//确保取得昨天的横向数据
	if(bUseNewlyData==false)
	{
		if(iToday==iEndDate)
		{
			iEndIndex--;
			iEndDate = m_pMixTradeDay->GetDateByIndex(iEndIndex);
		}
	}
	//取得指定日期[可能是自然日期]对应的实际交易日期
	int iStartDate = m_pMixTradeDay->GetDateByIndex(iStartIndex);
	if(iStartDate<startDate)
	{
		iStartIndex++;
		if(iStartIndex>=iMixAllCount)
			iStartIndex = iMixAllCount-1;
		if(iStartIndex>iEndIndex)
			iStartIndex = iEndIndex;
		iStartDate = m_pMixTradeDay->GetDateByIndex(iStartIndex);
	}

	//2009-03-24
	if(iStartDate>iEndDate)
	{
		iStartDate = iEndDate;
		iStartIndex = iEndIndex;
	}

	//计算交易日期天数
	//int iDays = 0;
		//int iStartIndex = pBaseSecurity->GetTradeDateIndex(iStartDate);
		//int iEndIndex = pBaseSecurity->GetTradeDateIndex(iEndDate);
	//iDays = iEndIndex-iStartIndex+1;

	//取得起始日期的数据
	//DataFileHisTradeData<DAY_HQ_HEADER,DAY_HQ_ITEM>* pOneDayTradeData = new DataFileHisTradeData<DAY_HQ_HEADER,DAY_HQ_ITEM>;
	DataFileHisTradeData<blk_TxExFile_FileHead,DAY_HQ_ITEM>* pOneDayTradeData = new DataFileHisTradeData<blk_TxExFile_FileHead,DAY_HQ_ITEM>;
	if(pOneDayTradeData==NULL)
	{
		sLog.Format(_T("阶段行情: 取得起始日期[%d,数据日期=%d]的数据时，内存分配失败！"),startDate,iStartDate);
		AfxMessageBox(sLog);
		Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
		return false;
	}
	pOneDayTradeData->SetDownloadMode(1);
	if(pOneDayTradeData->Load(
		Tx::Data::DataStatus::GetInstance()->GetExFileIdFromIni(
			Tx::Core::SystemPath::GetInstance()->GetSystemDataPath(),
			_T("day_trade_data_hu_shen")),
		iStartDate,
		true)==false
		)
	{
		delete pOneDayTradeData;
		sLog.Format(_T("阶段行情: 加载起始日期[%d,数据日期=%d]的数据失败[网速慢，文件更新失败]！"),startDate,iStartDate);
		AfxMessageBox(sLog);
		Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
		return false;
	}
	if(pOneDayTradeData->GetDataCount()<=0)
	{
		delete pOneDayTradeData;
		sLog.Format(_T("阶段行情: 起始日期[%d,数据日期=%d]的数据记录数无效！"),startDate,iStartDate);
		Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
		return false;
	}

	//取得终止日期的数据
	//DataFileHisTradeData<DAY_HQ_HEADER,DAY_HQ_ITEM>* pOneDayTradeData1 = new DataFileHisTradeData<DAY_HQ_HEADER,DAY_HQ_ITEM>;
	DataFileHisTradeData<blk_TxExFile_FileHead,DAY_HQ_ITEM>* pOneDayTradeData1 = new DataFileHisTradeData<blk_TxExFile_FileHead,DAY_HQ_ITEM>;
	if(pOneDayTradeData1==NULL)
	{
		delete pOneDayTradeData;
		sLog.Format(_T("阶段行情: 取得终止日期[%d,数据日期=%d]的数据时，内存分配失败！"),endDate,iEndDate);
		AfxMessageBox(sLog);
		Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
		return false;
	}
	pOneDayTradeData1->SetDownloadMode(1);
	if(pOneDayTradeData1->Load(
		Tx::Data::DataStatus::GetInstance()->GetExFileIdFromIni(
			Tx::Core::SystemPath::GetInstance()->GetSystemDataPath(),
			_T("day_trade_data_hu_shen")),
			iEndDate,true)==false || pOneDayTradeData1->GetDataCount()<=0)
	{
		delete pOneDayTradeData;
		delete pOneDayTradeData1;
		sLog.Format(_T("阶段行情: 加载终止日期[%d,数据日期=%d]的数据失败[网速慢，文件更新失败]！"),endDate,iEndDate);
		AfxMessageBox(sLog);
		Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
		return false;
	}
	if(pOneDayTradeData1->GetDataCount()<=0)
	{
		delete pOneDayTradeData;
		delete pOneDayTradeData1;
		sLog.Format(_T("阶段行情: 终止日期[%d,数据日期=%d]的数据记录数无效！"),endDate,iEndDate);
		Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
		return false;
	}

	//ZkkLib::Date ds,de;
	//ds.SetInt(startDate);
	//de.SetInt(endDate);

	//ZkkLib::DateTime ds1,de1;
	//ds1.SetDate(ds.GetYear(),ds.GetMonth(),ds.GetDay());
	//de1.SetDate(de.GetYear(),de.GetMonth(),de.GetDay());
	//int iMaxDays = (int)((de1-ds1).GetTotalDays());
	COleDateTime ds(startDate/10000,(startDate%10000)/100,(startDate%10000)%100,0,0,0);
	COleDateTime de(endDate/10000,(endDate%10000)/100,(endDate%10000)%100,0,0,0);
	int iMaxDays = (int)(de-ds).GetTotalDays();
	iMaxDays++;
	//sLog.Format(_T("阶段行情: 起始日期[%d],终止日期[%d]自然天数[%d]"),
	//	startDate,
	//	endDate,
	//	iMaxDays
	//	);
	//Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);


	//新股首日数据
	DAY_HQ_ITEM* pFirstDateData = new DAY_HQ_ITEM;
	DAY_HQ_ITEM* pLastestDateData = new DAY_HQ_ITEM;

	//2008-12-15
	//保存非交易时间区间之前的最近交易日的数据
	DAY_HQ_ITEM sDAY_HQ_ITEM;
	DAY_HQ_ITEM eDAY_HQ_ITEM;
	DAY_HQ_ITEM cDAY_HQ_ITEM;

	//初始化进度条
	int i=0;
	//step1
//	Tx::Core::ProgressWnd* p = Tx::Core::ProgressWnd::GetInstance();
	Tx::Core::ProgressWnd prw;
	//step2
	CString sProgressPrompt;
	sProgressPrompt =  _T("阶段行情统计");
	UINT progId = prw.AddItem(1,sProgressPrompt, 0.0);
	//step3
	if(bOnlyRaise==false)
		prw.Show(15);

	bool bTempOnlyToday = bOnlyToday;
	//CLogRecorder::GetInstance()->WriteToLog(_T("IndexWnd6::BlockCycleRateAdv--Calc--Start"));
	//循环处理样本
	for(std::set<int>::iterator iter=iSecurityId.begin();iter!=iSecurityId.end();iter++)
	{
		//msg.Format(_T("IndexWnd6::BlockCycleRateAdv-Calc--Start--SecurityID:%d"),*iter);
		//CLogRecorder::GetInstance()->WriteToLog(msg);
		bOnlyToday = bTempOnlyToday;
		//step4 进度条位置
		if(bOnlyRaise==false)
			prw.SetPercent(progId, (double)i/(double)ii);

		//取得当前样本的指针
		SecurityQuotation* pSecurity = GetSecurityNow(*iter);
		if(pSecurity!=NULL)
		{
//#ifndef _DEBUG
//			if(pSecurity->IsIndexCustomByCode()==true)
//				continue;
//#endif
			//if(pSecurity->IsNormal()==false)
			//	continue;
			memset(&sDAY_HQ_ITEM,0,sizeof(DAY_HQ_ITEM));
			memset(&eDAY_HQ_ITEM,0,sizeof(DAY_HQ_ITEM));
			memset(&cDAY_HQ_ITEM,0,sizeof(DAY_HQ_ITEM));
			dCustomIndexAvgPrice = 0;

			//是否采用纵向行情数据
			bool bIsLineData = false;
			if(
				pSecurity->IsIndexCustomByCode()==true
				|| pSecurity->IsIndex_Global()==true
				|| pSecurity->IsIndex_BDI()==true
				)
				bIsLineData = true;

			int iListedDate = pSecurity->GetIPOListedDate();			// 上市日期
			int iTradeDateLast = pSecurity->GetTradeDateLatest();		// 取得最新交易日期

			// zway 在下面代码增加注释
			if (bIsToday==true					// 服务器当前日期 iToday == endDate && iToday == iDataToday(取得最新行情数据时间)
				&& bUseNewlyData==true			// 参数
				&& iStartDate>iTradeDateLast	// iStartDate(调整后的实际交易日期) > iTradeDateLast(最新交易日期) zway ??? 这个条件不可能 ???
				&& iTradeDateLast>0
				)
				bOnlyToday = true;

			if(bIsLineData == true)				// IsIndexCustomByCode() || IsIndex_Global() || IsIndex_BDI()
			{
				pSecurity->LoadHisTrade();
				pSecurity->LoadTradeDate();
				//取得日期的索引
				int iCiCount = pSecurity->GetTradeDataCount();
				if(iCiCount>0)
				{
					//pSecurity->GetTradeDataByIndex(0);
					HisTradeData* pHisTradeData = NULL;
					pHisTradeData = pSecurity->GetTradeDataByIndex(0);
					if(pHisTradeData!=NULL)
						iListedDate = pHisTradeData->Date;		// 之前 iListedDate = pSecurity->GetIPOListedDate();
					pHisTradeData = pSecurity->GetTradeDataByIndex(iCiCount-1);
					if(pHisTradeData!=NULL)
						iTradeDateLast = pHisTradeData->Date;	// 之前 iTradeDateLast = pSecurity->GetTradeDateLatest();
				}
			}
#ifdef _DEBUG
			if(ii<=3)
			{
				int iMaxCountOfTheSecurity = pSecurity->GetTradeDataCount();
				if(iMaxCountOfTheSecurity>0)
				{
					int iLastTradeDate = pSecurity->GetTradeDateByIndex(iMaxCountOfTheSecurity-1);
					if(iTradeDateLast != iLastTradeDate && iLastTradeDate!=iToday)
					{
						sLog.Format(_T("最新交易日期无效[%s,%s],today=%d,tradedatelast from basicinfo=%d,from kline=%d"),
							pSecurity->GetName(),
							pSecurity->GetCode(true),
							iToday,
							iTradeDateLast,
							iLastTradeDate
							);
						Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
					}
				}
			}
#endif

			//CLogRecorder::GetInstance()->WriteToLog(_T("IndexWnd6::BlockCycleRateAdv--Calc--1-加载历史数据(OK)"));
			//起始日期是否停牌
			bool bFirstDayHalt = false;
			bool bIsFirstDay = false;

			//取得起始日期的行情数据
			DAY_HQ_ITEM* pStart = NULL;
			pStart = pOneDayTradeData->GetDataByObj(*iter,false);

			pFirstDateData->Code=*iter;

			pFirstDateData->Preclose = Con_floatInvalid;
			pFirstDateData->Open = Con_floatInvalid;
			pFirstDateData->High = Con_floatInvalid;
			pFirstDateData->Low = Con_floatInvalid;
			pFirstDateData->Close = Con_floatInvalid;
			pFirstDateData->Price = Con_floatInvalid;

			pFirstDateData->lSumTradeDays=0;
			pFirstDateData->Volume=Con_doubleInvalid;
			pFirstDateData->Amount=Con_doubleInvalid;
			pFirstDateData->SumVolume=Con_doubleInvalid;
			pFirstDateData->SumAmount=Con_doubleInvalid;
			pFirstDateData->TotalShare = Con_doubleInvalid;
			pFirstDateData->TradeableShare = Con_doubleInvalid;
			pLastestDateData->TurnOverSum = Con_doubleInvalid;
			pLastestDateData->TurnOver = Con_doubleInvalid;

			if(pStart!=NULL)
			{
				memcpy(&sDAY_HQ_ITEM,pStart,sizeof(DAY_HQ_ITEM));
				// zway ???// bIsFirstDay = iListedDate > iStartDate ? true : false;
			}
			else
			{
				// pOneDayTradeData 为起始日期(iStartDate)的数据，如果没有该样本，说明其未上市 ???
			}

			if(bOnlyToday == true)
			{
				//取得盘中数据
				if(pSecurity->HaveDetailData()==true)
				{
					pFirstDateData->Amount = pSecurity->GetAmount();
					pFirstDateData->Close = pSecurity->GetClosePrice(true);
					pFirstDateData->High = pSecurity->GetHighPrice();
					pFirstDateData->Low = pSecurity->GetLowPrice();
					pFirstDateData->Open = pSecurity->GetOpenPrice();
					pFirstDateData->Preclose = pSecurity->GetPreClosePrice();
					//后复权
					pFirstDateData->Price = (float)(pFirstDateData->Close*pSecurity->GetExdividendScale(iListedDate,endDate,false));
					pFirstDateData->Volume = pSecurity->GetVolume();

					pFirstDateData->SumAmount = pFirstDateData->Amount;
					pFirstDateData->SumVolume = pFirstDateData->Volume;
					pFirstDateData->lSumTradeDays = 1;
					if(bOnlyRaise==false)
					{
					pFirstDateData->TotalShare = pSecurity->GetTotalShare();
					pFirstDateData->TradeableShare = pSecurity->GetTradableShare();
					}

					if(pFirstDateData->TradeableShare>0)
						pFirstDateData->TurnOver = pFirstDateData->Volume/pFirstDateData->TradeableShare*100;
					pFirstDateData->TurnOverSum = pFirstDateData->TurnOver;

				}
				pStart = NULL;
				//天相指数有时没有行情,所以必须判断收盘价
				if(pFirstDateData->Close>0)
					pStart = pFirstDateData;
				//CLogRecorder::GetInstance()->WriteToLog(_T("IndexWnd6::BlockCycleRateAdv--end of: if (bOnlyToday)(OK)"));
			}	// end of: if (bOnlyToday)
			else
			{
				if(pStart==NULL)
				{
					// zway 未上市新股会有此情况???
					if(bIsLineData == false)
					{
						//bIsFirstDay = true;
						//如果没有找到样本数据
						//取得新股首日数据
						//bFirstDate = true;
						//fix 20091113 wangyc 创业板等天相指数没有首日，取第一天数据 暂时先这样修改
						if (pSecurity->IsIndex_TX())
						{
							//2009-02-06
							//自定义指数
							bIsFirstDay = true;

							//pStart = pOneDayTradeData->GetDataByObj(*iter,false);
							//取得自定义指数的行情数据
							pSecurity->LoadHisTrade();
							pSecurity->LoadTradeDate();
							//取得日期的索引
							int iCiIndex = pSecurity->GetTradeDateIndex(startDate/*iStartDate*/);
							//pSecurity->GetTradeDataByIndex(0);
							HisTradeData* pHisTradeData = pSecurity->GetTradeDataByNatureDate(startDate/*iStartDate*/,tag_DI_None);
							if(pHisTradeData==NULL)
								pHisTradeData = pSecurity->GetTradeDataByIndex(0);
							if(pHisTradeData!=NULL)
							{
								if(pHisTradeData->Date<startDate/*iStartDate*/)
								{
									//可能落在停牌区间
									iCiIndex++;
									pHisTradeData = pSecurity->GetTradeDataByIndex(iCiIndex);
								}
								if(pHisTradeData!=NULL)
								{
									pFirstDateData->Preclose = pHisTradeData->Preclose;
									pFirstDateData->Open = pHisTradeData->Open;
									pFirstDateData->High = pHisTradeData->High;
									pFirstDateData->Low = pHisTradeData->Low;
									pFirstDateData->Close = pHisTradeData->Close;
									pFirstDateData->Price = pHisTradeData->Close;//指数不复权

									pFirstDateData->lSumTradeDays=iCiIndex+1;
									pFirstDateData->Volume=pHisTradeData->Volume;
									pFirstDateData->Amount=pHisTradeData->Amount;
									pFirstDateData->SumVolume=pHisTradeData->Volume;
									pFirstDateData->SumAmount=pHisTradeData->Amount;

									pStart = pFirstDateData;
									memcpy(&sDAY_HQ_ITEM,pStart,sizeof(DAY_HQ_ITEM));
								}
							}
							//CLogRecorder::GetInstance()->WriteToLog(_T("IndexWnd6::BlockCycleRateAdv--end of: if (IsIndex_TX())(OK)"));
						}	// end of: if (IsIndex_TX())
						//fix 20091113 wangyc 创业板等天相指数没有首日，取第一天数据
						else
						{
							if(iListedDate<=0) continue;
							if(pSecurity->HaveDetailData()==true && iListedDate>0)
							{
								//取得上市日期的数据
								//DataFileHisTradeData<DAY_HQ_HEADER,DAY_HQ_ITEM>* pOneDayTradeDataListed = new DataFileHisTradeData<DAY_HQ_HEADER,DAY_HQ_ITEM>;
								DataFileHisTradeData<blk_TxExFile_FileHead,DAY_HQ_ITEM>* pOneDayTradeDataListed = new DataFileHisTradeData<blk_TxExFile_FileHead,DAY_HQ_ITEM>;
								if(pOneDayTradeDataListed!=NULL)
								{
									pOneDayTradeDataListed->SetDownloadMode(1);
									if(pOneDayTradeDataListed->Load(
										Tx::Data::DataStatus::GetInstance()->GetExFileIdFromIni(
										Tx::Core::SystemPath::GetInstance()->GetSystemDataPath(),
										_T("day_trade_data_hu_shen")),
										iListedDate,true)==true)
									{

										DAY_HQ_ITEM* pListed = pOneDayTradeDataListed->GetDataByObj(*iter,false);

										if(pListed!=NULL)
										{
											memcpy(pFirstDateData,pListed,sizeof(DAY_HQ_ITEM));
											bIsFirstDay = true;	// 2011-04-13 zhangw
										}
									}
									delete pOneDayTradeDataListed;
									pOneDayTradeDataListed = NULL;
								}
							}
							pStart = pFirstDateData;
							memcpy(&sDAY_HQ_ITEM,pStart,sizeof(DAY_HQ_ITEM));
							//CLogRecorder::GetInstance()->WriteToLog(_T("IndexWnd6::BlockCycleRateAdv--end of: else (not => IsIndex_TX())(OK)"));
						}	// end of: else (not => IsIndex_TX())
					}	// end of: if (not => IsIndexCustomByCode() || IsIndex_Global() || IsIndex_BDI())
					else
					{
						// 当 IsIndexCustomByCode() || IsIndex_Global() || IsIndex_BDI() 时：
						//2009-02-06
						//自定义指数
						bIsFirstDay = true;

						//pStart = pOneDayTradeData->GetDataByObj(*iter,false);
						//取得自定义指数的行情数据
						pSecurity->LoadHisTrade();
						pSecurity->LoadTradeDate();
						//取得日期的索引
						int iCiIndex = pSecurity->GetTradeDateIndex(startDate/*iStartDate*/);
						//pSecurity->GetTradeDataByIndex(0);
						HisTradeData* pHisTradeData = pSecurity->GetTradeDataByNatureDate(startDate/*iStartDate*/,tag_DI_None);
						if(pHisTradeData==NULL)
							pHisTradeData = pSecurity->GetTradeDataByIndex(0);
						if(pHisTradeData!=NULL)
						{
							if(pHisTradeData->Date<startDate/*iStartDate*/)
							{
								//可能落在停牌区间
								iCiIndex++;
								pHisTradeData = pSecurity->GetTradeDataByIndex(iCiIndex);
							}
							if(pHisTradeData!=NULL)
							{
								pFirstDateData->Preclose = pHisTradeData->Preclose;
								pFirstDateData->Open = pHisTradeData->Open;
								pFirstDateData->High = pHisTradeData->High;
								pFirstDateData->Low = pHisTradeData->Low;
								pFirstDateData->Close = pHisTradeData->Close;
								pFirstDateData->Price = pHisTradeData->Close;//指数不复权

								pFirstDateData->lSumTradeDays=iCiIndex+1;
								pFirstDateData->Volume=pHisTradeData->Volume;
								pFirstDateData->Amount=pHisTradeData->Amount;
								pFirstDateData->SumVolume=pHisTradeData->Volume;
								pFirstDateData->SumAmount=pHisTradeData->Amount;

								pStart = pFirstDateData;
								memcpy(&sDAY_HQ_ITEM,pStart,sizeof(DAY_HQ_ITEM));
							}
						}
					}
					//CLogRecorder::GetInstance()->WriteToLog(_T("IndexWnd6::BlockCycleRateAdv--end of: if (pStart == NULL)"));
				}	// end of: if (pStart == NULL)
				else
				{
					//2008-04-16
					//如果当前交易实体指定日期没有交易，则取下一个交易日
					//此时需要通过历史行情数据来判断下一个交易日，所以会慢
					if(pSecurity->IsKLine()==true && pSecurity->HaveVolume()==true && !(pStart->Volume>0))
					{
						bFirstDayHalt = true;
						int iStartIndexV0 = pSecurity->GetTradeDateIndex(iStartDate);
						int iStartDateV0 = pSecurity->GetTradeDateByIndex(iStartIndexV0+1);
						pStart = NULL;
						DataFileHisTradeData<blk_TxExFile_FileHead,DAY_HQ_ITEM>* pODT = new DataFileHisTradeData<blk_TxExFile_FileHead,DAY_HQ_ITEM>;
						if(pODT!=NULL)
						{
							pODT->SetDownloadMode(1);
							bool bStartV0 = pODT->Load(
								Tx::Data::DataStatus::GetInstance()->GetExFileIdFromIni(
									Tx::Core::SystemPath::GetInstance()->GetSystemDataPath(),
									_T("day_trade_data_hu_shen")),
								iStartDateV0,true);
							if(bStartV0==true)
							{
								DAY_HQ_ITEM* pListed = pODT->GetDataByObj(*iter,false);
								if(pListed!=NULL)
								{
									memcpy(pFirstDateData,pListed,sizeof(DAY_HQ_ITEM));
									bIsFirstDay = iStartDateV0 == iListedDate ? true : false; // 2011-04-13 zhangw
								}
							}
							delete pODT;
						}
						pStart = pFirstDateData;
					}
					else
					{
						bIsFirstDay = iStartDate == iListedDate ? true : false; // 2011-04-13 zhangw
					}
				}
				//CLogRecorder::GetInstance()->WriteToLog(_T("IndexWnd6::BlockCycleRateAdv--end of: else (bOnlyToday == false)"));
			}	// end of: else (bOnlyToday == false)

			//查找当前样本终止日期的数据
			DAY_HQ_ITEM* pEnd = NULL;
			pEnd = pOneDayTradeData1->GetDataByObj(*iter,false);

			if(pEnd!=NULL)
				memcpy(&eDAY_HQ_ITEM,pEnd,sizeof(DAY_HQ_ITEM));

			if(bOnlyToday == true)
			{
				//取盘中数据
				pLastestDateData->Code = *iter;
				pLastestDateData->Amount=Con_doubleInvalid;
				pLastestDateData->Close = Con_floatInvalid;
				pLastestDateData->High = Con_floatInvalid;
				pLastestDateData->Low = Con_floatInvalid;

				pLastestDateData->lSumTradeDays=0;

				pLastestDateData->Open = Con_floatInvalid;
				pLastestDateData->Preclose = Con_floatInvalid;
				pLastestDateData->Price = Con_floatInvalid;

				pLastestDateData->Volume=Con_doubleInvalid;
				pLastestDateData->SumVolume=Con_doubleInvalid;
				pLastestDateData->SumAmount=Con_doubleInvalid;

				pLastestDateData->TurnOverSum = Con_doubleInvalid;
				pLastestDateData->TurnOver = Con_doubleInvalid;

				pLastestDateData->TotalShare = Con_doubleInvalid;
				pLastestDateData->TradeableShare = Con_doubleInvalid;
				//取得盘中数据
				//if(pSecurity->HaveDetailData()==true && iListedDate>0)
				if(pSecurity->HaveDetailData()==true)
				{
					pLastestDateData->Amount = pSecurity->GetAmount();
					pLastestDateData->Close = pSecurity->GetClosePrice(true);
					pLastestDateData->High = pSecurity->GetHighPrice();
					pLastestDateData->Low = pSecurity->GetLowPrice();
					pLastestDateData->Open = pSecurity->GetOpenPrice();
					pLastestDateData->Preclose = pSecurity->GetPreClosePrice();
					if(bIsLineData == false)
					{
						//后复权
						pLastestDateData->Price = (float)(pLastestDateData->Close*pSecurity->GetExdividendScale(iListedDate,endDate,false));
					}
					else
						pLastestDateData->Price = pSecurity->GetClosePrice(true);
					pLastestDateData->Volume = pSecurity->GetVolume();

					pLastestDateData->SumAmount = pLastestDateData->Amount;
					pLastestDateData->SumVolume = pLastestDateData->Volume;
					pLastestDateData->lSumTradeDays = 1;
					if(bOnlyRaise==false)
					{
						pLastestDateData->TotalShare = pSecurity->GetTotalShare();
						pLastestDateData->TradeableShare = pSecurity->GetTradableShare();
					}

					if(pLastestDateData->TradeableShare>0)
						pLastestDateData->TurnOver = pLastestDateData->Volume/pLastestDateData->TradeableShare*100;
					pLastestDateData->TurnOverSum = pLastestDateData->TurnOver;

				}
				//天相指数有时没有行情,所以必须判断收盘价
				if(pLastestDateData->Close>0)
					pEnd = pLastestDateData;
				//CLogRecorder::GetInstance()->WriteToLog(_T("IndexWnd6::BlockCycleRateAdv--end of2: (bOnlyToday == true)"));
			} // end of2: bOnlyToday == true
			else
			{
				//2008-05-15
				//今日大盘交易，但是取得横向数据的终止日期小于今日，则需要取得今日数据，以便累计
				//否则直接计算,比如，当日kline已经下载到本地，已经取得当日横向数据，则不需要当日数据进行累计
				if(bIsToday==true && iEndDate<iToday)
				{
					pLastestDateData->Code = *iter;
					pLastestDateData->Amount=Con_doubleInvalid;
					pLastestDateData->Close = Con_floatInvalid;
					pLastestDateData->High = Con_floatInvalid;
					pLastestDateData->Low = Con_floatInvalid;

					pLastestDateData->lSumTradeDays=0;

					pLastestDateData->Open = Con_floatInvalid;
					pLastestDateData->Preclose = Con_floatInvalid;
					pLastestDateData->Price = Con_floatInvalid;

					pLastestDateData->Volume=Con_doubleInvalid;
					pLastestDateData->SumVolume=Con_doubleInvalid;
					pLastestDateData->SumAmount=Con_doubleInvalid;

					pLastestDateData->TurnOverSum = Con_doubleInvalid;
					pLastestDateData->TurnOver = Con_doubleInvalid;

					pLastestDateData->TotalShare = Con_doubleInvalid;
					pLastestDateData->TradeableShare = Con_doubleInvalid;
					//取得盘中数据
					//if(pSecurity->HaveDetailData()==true && iListedDate>0)
					if(pSecurity->HaveDetailData()==true)
					{
						pLastestDateData->Amount = pSecurity->GetAmount();
						pLastestDateData->Close = pSecurity->GetClosePrice(true);
						pLastestDateData->High = pSecurity->GetHighPrice();
						pLastestDateData->Low = pSecurity->GetLowPrice();
						pLastestDateData->Open = pSecurity->GetOpenPrice();
						pLastestDateData->Preclose = pSecurity->GetPreClosePrice();
						//后复权
						if(bIsLineData == false)
						{
							pLastestDateData->Price = (float)(pLastestDateData->Close*pSecurity->GetExdividendScale(iListedDate,endDate,false));
						}
						else
						{
							pLastestDateData->Price = pSecurity->GetClosePrice(true);
						}
						pLastestDateData->Volume = pSecurity->GetVolume();

						pLastestDateData->SumAmount = pLastestDateData->Amount;
						pLastestDateData->SumVolume = pLastestDateData->Volume;
						pLastestDateData->lSumTradeDays = 0;
						if(bOnlyRaise==false)
						{
							pLastestDateData->TotalShare = pSecurity->GetTotalShare();
							pLastestDateData->TradeableShare = pSecurity->GetTradableShare();
						}

						if(bIsLineData == false)
						{
							if(pLastestDateData->TradeableShare>0)
								pLastestDateData->TurnOver = pLastestDateData->Volume/pLastestDateData->TradeableShare*100;
							pLastestDateData->TurnOverSum = pLastestDateData->TurnOver;
							if(pEnd!=NULL)
							{
								pLastestDateData->lSumTradeDays += pEnd->lSumTradeDays;
								if(pLastestDateData->TurnOverSum<0)
									pLastestDateData->TurnOverSum = 0;
								pLastestDateData->TurnOverSum += pEnd->TurnOverSum;

								if(pLastestDateData->SumAmount<0)
									pLastestDateData->SumAmount = 0;
								pLastestDateData->SumAmount += pEnd->SumAmount;

								if(pLastestDateData->SumVolume<0)
									pLastestDateData->SumVolume = 0;
								pLastestDateData->SumVolume += pEnd->SumVolume;
							}
						}
						else
						{
							pSecurity->LoadHisTrade();
							pSecurity->LoadTradeDate();
							//取得日期的索引
							int iCiIndex = pSecurity->GetTradeDateIndex(endDate/*iEndDate*/);
							HisTradeData* pHisTradeData = pSecurity->GetTradeDataByNatureDate(endDate/*iEndDate*/,tag_DI_None);
							if(pHisTradeData==NULL)
							{
								iCiIndex = pSecurity->GetTradeDataCount();
								iCiIndex--;
								pHisTradeData = pSecurity->GetTradeDataByIndex(iCiIndex);
							}
							if(pHisTradeData!=NULL)
							{
								pLastestDateData->Preclose = pHisTradeData->Preclose;
								pLastestDateData->Open = pHisTradeData->Open;
								pLastestDateData->High = pHisTradeData->High;
								pLastestDateData->Low = pHisTradeData->Low;
								pLastestDateData->Close = pHisTradeData->Close;
								pLastestDateData->Price = pHisTradeData->Close;//指数不复权

								pLastestDateData->lSumTradeDays=iCiIndex+1;
								pLastestDateData->Volume=pHisTradeData->Volume;
								pLastestDateData->Amount=pHisTradeData->Amount;
								int cis,cie;
								cis = pStart->lSumTradeDays-1;
								cie = iCiIndex;
								//2009-02-09
								//计算累计成交量和金额
								pLastestDateData->SumVolume=0;
								pLastestDateData->SumAmount=0;
								for(int cii=cis;cii<=cie;cii++)
								{
									pHisTradeData = pSecurity->GetTradeDataByIndex(cii);
									if(pHisTradeData!=NULL)
									{
										if(pHisTradeData->Close>0)
										dCustomIndexAvgPrice += pHisTradeData->Close;
										if(pHisTradeData->Volume>0)
										pLastestDateData->SumVolume+=pHisTradeData->Volume;
										if(pHisTradeData->Amount>0)
										pLastestDateData->SumAmount+=pHisTradeData->Amount;
									}
								}

								if(pLastestDateData->Close>0)
									dCustomIndexAvgPrice += pLastestDateData->Close;
								if(pSecurity->GetVolume()>0)
									pLastestDateData->SumVolume+=pSecurity->GetVolume();
								if(pSecurity->GetAmount()>0)
									pLastestDateData->SumAmount+=pSecurity->GetAmount();

								pEnd = pLastestDateData;
								memcpy(&eDAY_HQ_ITEM,pEnd,sizeof(DAY_HQ_ITEM));
							}
						}

						if(pStart!=NULL && pLastestDateData->lSumTradeDays < pStart->lSumTradeDays)
						{
							sLog.Format(_T("阶段行情: 无效阶段行情[起始日交易天数=%d,终止日交易天数=%d][%s-%s-%d-%d]！"),
								pStart->lSumTradeDays,
								pLastestDateData->lSumTradeDays,
								pSecurity->GetName(),
								pSecurity->GetCode(true),
								startDate,
								iEndDate);
							Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
							pLastestDateData->lSumTradeDays = pStart->lSumTradeDays;
						}
					}
					//2008-09-25
					//天相指数有时没有行情,所以必须判断收盘价
					if(pEnd!=NULL && pLastestDateData->Close>0)
						pEnd = pLastestDateData;
				}
				else
				{
					//历史数据
					if(bIsLineData == true)
					{
						pLastestDateData->Code = *iter;
						pLastestDateData->Amount=Con_doubleInvalid;
						pLastestDateData->Close = Con_floatInvalid;
						pLastestDateData->High = Con_floatInvalid;
						pLastestDateData->Low = Con_floatInvalid;
						pLastestDateData->lSumTradeDays=0;
						pLastestDateData->Open = Con_floatInvalid;
						pLastestDateData->Preclose = Con_floatInvalid;
						pLastestDateData->Price = Con_floatInvalid;

						pLastestDateData->Volume=Con_doubleInvalid;
						pLastestDateData->SumVolume=Con_doubleInvalid;
						pLastestDateData->SumAmount=Con_doubleInvalid;
						pLastestDateData->TurnOverSum = Con_doubleInvalid;
						pLastestDateData->TurnOver = Con_doubleInvalid;
						pLastestDateData->TotalShare = Con_doubleInvalid;
						pLastestDateData->TradeableShare = Con_doubleInvalid;

						pSecurity->LoadHisTrade();
						pSecurity->LoadTradeDate();
						//取得终止日期的索引
						int iCiIndex = pSecurity->GetTradeDateIndex(endDate/*iEndDate*/);
						HisTradeData* pHisTradeData = pSecurity->GetTradeDataByNatureDate(endDate/*iEndDate*/,tag_DI_None);
						if(pHisTradeData==NULL)
						{
							iCiIndex = pSecurity->GetTradeDataCount();
							iCiIndex--;
							pHisTradeData = pSecurity->GetTradeDataByIndex(iCiIndex);
						}
						if(pHisTradeData!=NULL)
						{
							//终止日期行情
							pLastestDateData->Preclose = pHisTradeData->Preclose;
							pLastestDateData->Open = pHisTradeData->Open;
							pLastestDateData->High = pHisTradeData->High;
							pLastestDateData->Low = pHisTradeData->Low;
							pLastestDateData->Close = pHisTradeData->Close;
							pLastestDateData->Price = pHisTradeData->Close;//指数不复权

							pLastestDateData->lSumTradeDays=iCiIndex+1;
							pLastestDateData->Volume=pHisTradeData->Volume;
							pLastestDateData->Amount=pHisTradeData->Amount;

							int cis,cie;
							cis = pStart->lSumTradeDays-1;
							cie = iCiIndex;
							//2009-02-09
							//计算累计成交量和金额
							pLastestDateData->SumVolume=0;
							pLastestDateData->SumAmount=0;
							for(int cii=cis;cii<=cie;cii++)
							{
								pHisTradeData = pSecurity->GetTradeDataByIndex(cii);
								if(pHisTradeData!=NULL)
								{
									if(pHisTradeData->Close>0)
									dCustomIndexAvgPrice += pHisTradeData->Close;
									if(pHisTradeData->Volume>0)
									pLastestDateData->SumVolume+=pHisTradeData->Volume;
									if(pHisTradeData->Amount>0)
									pLastestDateData->SumAmount+=pHisTradeData->Amount;
								}
							}

							pEnd = pLastestDateData;
							memcpy(&eDAY_HQ_ITEM,pEnd,sizeof(DAY_HQ_ITEM));
						}
						if(pStart!=NULL && pLastestDateData->lSumTradeDays < pStart->lSumTradeDays)
						{
							sLog.Format(_T("阶段行情: 无效阶段行情[起始日交易天数=%d,终止日交易天数=%d][%s-%s-%d-%d]！"),
								pStart->lSumTradeDays,
								pLastestDateData->lSumTradeDays,
								pSecurity->GetName(),
								pSecurity->GetCode(true),
								startDate,
								iEndDate);
							Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
							pLastestDateData->lSumTradeDays = pStart->lSumTradeDays;
						}
					}
				}
				//CLogRecorder::GetInstance()->WriteToLog(_T("IndexWnd6::BlockCycleRateAdv--end of2: else(bOnlyToday == false)"));
			} //end of2: bOnlyToday == false

			//2008-04-23尚未上市的日期区间
			//startDate
			//iStartDate

			if(bOnlyToday == false)
			{
				if(
					startDate<iListedDate && 
					iEndDate<iListedDate
					)
				{
					pEnd = NULL;
					pStart = NULL;
					sLog.Format(_T("阶段行情: 尚未上市[%d]的日期区间[%s-%s-%d-%d]！"),
						iListedDate,
						pSecurity->GetName(),
						pSecurity->GetCode(true),
						startDate,
						iEndDate);
					Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
					//2008-12-15
					//剔除样本
					continue;
				}
				//2008-05-05当前正在停牌或者摘牌，日期区间在停牌区间
				if(startDate>iTradeDateLast && iEndDate>iTradeDateLast
					)
				{
					memcpy(&cDAY_HQ_ITEM,&eDAY_HQ_ITEM,sizeof(DAY_HQ_ITEM));
					pEnd = NULL;
					pStart = NULL;
					sLog.Format(_T("阶段行情: 当前正在停牌或者已摘牌[最新日期=%d]的日期区间[%s-%s-%d-%d]！"),
						iTradeDateLast,
						pSecurity->GetName(),
						pSecurity->GetCode(true),
						startDate,
						iEndDate);
					Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
				}

				if(
					((iStartDate==iEndDate) && (startDate > iStartDate || endDate<iEndDate))||
					iStartDate>iEndDate || //历史非交易日,在交易日序列内
					(startDate>iTradeDateLast && endDate<iToday)|| //历史非交易日,在交易日序列外
					(!(pEnd!=NULL && pStart!=NULL && (pEnd->SumVolume-pStart->SumVolume+pStart->Volume)>0) && pSecurity->IsIndex()==false)//阶段成交量=0
					)
				{
					if(startDate>iTradeDateLast && endDate<iToday)
						memcpy(&cDAY_HQ_ITEM,&eDAY_HQ_ITEM,sizeof(DAY_HQ_ITEM));
					else
					//if((iStartDate==iEndDate) && (startDate > iStartDate || endDate<iEndDate))
					//if(iStartDate>iEndDate)
						memcpy(&cDAY_HQ_ITEM,&sDAY_HQ_ITEM,sizeof(DAY_HQ_ITEM));

					pEnd = NULL;
					pStart = NULL;
					sLog.Format(_T("阶段行情: 历史非交易日[最新日期=%d]的日期区间[%s-%s-%d-%d]！"),
						iTradeDateLast,
						pSecurity->GetName(),
						pSecurity->GetCode(true),
						startDate,
						iEndDate);
					Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
				}
				//CLogRecorder::GetInstance()->WriteToLog(_T("IndexWnd6::BlockCycleRateAdv--end of3: (bOnlyToday == false)"));
			} // end of3: bOnlyToday == false
			else
			{
				//样本当日停牌
				//实时数据
				if(pEnd!=NULL)
				{
					if(pEnd->Amount<0 || pEnd->Volume<0)
					{
						memcpy(&cDAY_HQ_ITEM,&eDAY_HQ_ITEM,sizeof(DAY_HQ_ITEM));
						pEnd = NULL;
						pStart = NULL;
					}
				}
				//CLogRecorder::GetInstance()->WriteToLog(_T("IndexWnd6::BlockCycleRateAdv--end of3: else(bOnlyToday == true)"));
			} // end of3: bOnlyToday == true)

			resTable.AddRow();
			int iCol=0;
			//0 security id
			resTable.SetCell(iCol,i,(int)pSecurity->GetId());
			iCol++;
			//resTable.SetCell(iCol,i,*iter);
			//1 name
			resTable.SetCell(iCol,i,pSecurity->GetName());
			iCol++;

			CString sExtCode;

			sExtCode = pSecurity->GetCode();
			//计算
			//2 code
			resTable.SetCell(iCol,i,sExtCode);
			iCol++;

			double dExdividendScale = 1;
			//2008-04-23
			//if(pEnd==NULL)
			//{
			//	i++;
			//	continue;
			//}

			//复权计算
			//pSecurity
			//2007-09-10
			//考虑复权类型
			//复权类型 0-不复权;1=后复权;2-前复权;3-全复权+后复权
			//考虑是否剔除首日涨幅

			//前复权以enddate为基准计算startDate的价格
			//后复权以startdate为基准计算enddate的价格


			float prePrice = Con_floatInvalid;
			float preClose = Con_floatInvalid;
			float nowClose = Con_floatInvalid;
			float openPrice = Con_floatInvalid;
			double raise = 0;

			double raiseValue = 0;
			//取得开盘价
			if(pStart!=NULL && pStart->Open>0)
				openPrice = pStart->Open;

			//取得前收价
			if(pStart!=NULL && pStart->Preclose>0)
				prePrice = pStart->Preclose;
			preClose = prePrice;

			//取得收盘价
			if(pEnd!=NULL && pEnd->Close>0)
				nowClose = pEnd->Close;

			//2009-05-11 指数不考虑复权计算
			if(bIsLineData == false && pSecurity->IsIndex()!=true)
			{
				//剔除新股首日涨幅  wanglm
				if(bCutFirstDateRaise==true && bIsFirstDay==true)
				{
					if(pStart!=NULL && pStart->Close>0)
					{
						//视同剔除首日行情
						preClose = pStart->Close;
						prePrice = preClose;
					}
				}
				//CLogRecorder::GetInstance()->WriteToLog(_T("IndexWnd6::BlockCycleRateAdv-- 复权操作-Start"));
				switch(iFQLX)
				{
				case 0://不复权
					break;
				case 2://后复权
				//case 3://全复权+后复权
					{
						double dScale = pSecurity->GetExdividendScale(startDate,endDate,false);
						dExdividendScale = dScale;
						//2008-05-28
						//检查无效数据
						if(nowClose>0)
							nowClose = (float)(nowClose*dScale);

						//如果获取数据日期=除权日且指定日期范围内没有其他除权日，该除权日前收价做后复权处理     2012-5-3   bug:11864
                        //取得当前交易实体的除权数据[除权日期序列]
						int count = pSecurity->GetExdividendDataCount();
						int icount = 0;

						for(int i=0;i<count;i++)
						{
							ExdividendData*	pExdividendData = pSecurity->GetExdividendDataByIndex(i);
							if(pExdividendData==NULL)
								continue;

							// 当终止日期>=最新交易日；最新交易日==除权日；实际取得数据的终止日期 < 除权日(最新交易日) ，这时数据不做复权处理  
							// bug:12192   2012-6-10 
							if (startDate <= pExdividendData->iDate && endDate > pExdividendData->iDate && iEndDate < pExdividendData->iDate)
							{
								if (nowClose>0)
									nowClose = (float)(nowClose/dScale);
							}

							if(pExdividendData->iDate>=startDate && pExdividendData->iDate<=endDate)        
							{
								if(pExdividendData->iDate == iStartDate)
								{
									if (preClose > 0)
									{
										preClose = (float)(preClose*dScale);
										prePrice = preClose;
									}
                                    //如果获取数据日期=除权日且指定日期范围内没有其他除权日，该除权日开盘价做后复权处理  2012-5-31
									if (openPrice > 0)
										openPrice = (float)(openPrice*dScale);
								}
								//else
								//{
								//	//剔除新股首日涨幅  wanglm
								//	if(bCutFirstDateRaise==true && bIsFirstDay==true)
								//	{
								//		if(pStart!=NULL && pStart->Close>0)
								//		{
								//			//视同剔除首日行情
								//			preClose = pStart->Close;
								//			prePrice = preClose;
								//		}
								//	}
								//	else
								//	{
								//		//取得前收价
								//		if(pStart!=NULL && pStart->Preclose>0)
								//			prePrice = pStart->Preclose;
								//		preClose = prePrice;
								//	}
								//}
							}
						}
						//2008-06-27
						//计算前收价
						if(bFirstDayHalt == true)
						{
							//if(preClose>0)
							//	preClose = (float)(preClose*dScale);
							//if(openPrice>0)
							//	openPrice = (float)(openPrice*dScale);
							//if(prePrice>0)
							//	prePrice = (float)(prePrice*dScale);
						}
					}
					break;
				case 1://前复权
					{
						//取得收盘价
						//if(pEnd!=NULL && pEnd->Close>0)
						//	nowClose = pEnd->Preclose;
						double dScale = pSecurity->GetExdividendScale(iStartDate,endDate,true);
						dExdividendScale = dScale;
						if(preClose>0)
							preClose = (float)(preClose*dScale);
						if(openPrice>0)
							openPrice = (float)(openPrice*dScale);
						if(prePrice>0)
							prePrice = (float)(prePrice*dScale);
					}
					break;
				case 3://全复权+后复权
					{
						if(pEnd!=NULL && pEnd->Price>0)
						{
							nowClose = pEnd->Price;
						}
						if(pStart!=NULL && pStart->Price>0 && pStart->Close>0)
						{
							dExdividendScale = pStart->Price/pStart->Close;
							if(prePrice>0)
								prePrice = (float)(prePrice*pStart->Price/pStart->Close);
							if(preClose>0)
								preClose = (float)(preClose*pStart->Price/pStart->Close);
							if(openPrice>0)
								openPrice = (float)(openPrice*pStart->Price/pStart->Close);

							if(pStart!=NULL && pStart->Price>0)
							{
								if(bCutFirstDateRaise==true && bIsFirstDay==true)
								{
									preClose = pStart->Price;
									prePrice = preClose;
								}
							}
						}
					}
					break;
				case 4://全复权+前复权
					{
						//样本最新交易日期date_latest
						//终止日期date_end
						//起始日期date_start
						//计算终止日期的前复权价格close
						//计算起始日期的前复权价格preclose
						//计算起始日期的前复权价格openclose
						//取得收盘价
						//fix bug 9101,刘鹏,2011-11-3
						double dStartScale = pSecurity->GetExdividendScale(iStartDate,iTradeDateLast,true);
						if(preClose>0)
							preClose = (float)(preClose*dStartScale);
						if(openPrice>0)
							openPrice = (float)(openPrice*dStartScale);
						if(prePrice>0)
							prePrice = (float)(prePrice*dStartScale);

						double dEndScale = pSecurity->GetExdividendScale(iEndDate,iTradeDateLast,true);
						if(nowClose>0)
							nowClose = (float)(nowClose*dEndScale);
						//给均价用的，取个近似吧。
						dExdividendScale = (dStartScale + dEndScale)/2;
					}
					break;
				default:
					break;
				}
				//CLogRecorder::GetInstance()->WriteToLog(_T("IndexWnd6::BlockCycleRateAdv-- 复权操作-End"));
				//int iFlag
				//计算类型0-默认；1-债券[bCutFirstDateRaise=true表示全价模式,bCutFirstDateRaise=false表示净价模式]
				if(iFlag==1)
				{
					//2008-12-09
					//国债考虑全价净价
					if(pSecurity->IsBond_National()==true)
					{
						//计算应计利息
						double dInterestStart = GetBondInterest(pSecurity,iStartDate);
						double dInterestEnd = GetBondInterest(pSecurity,iEndDate);
						if(bCutFirstDateRaise==true)
						{
							if(pSecurity->IsBondNetPrice()==true)
							{
								//计算全价
								if(preClose>0)
									preClose +=(float)dInterestStart;
								if(openPrice>0)
									openPrice +=(float)dInterestStart;
								if(nowClose>0)
									nowClose +=(float)dInterestEnd;
								if(prePrice>0)
									prePrice +=(float)dInterestEnd;
							}
						}
						else
						{
							if(pSecurity->IsBondNetPrice()==false)
							{
								//计算净价
								if(preClose>0)
									preClose -=(float)dInterestStart;
								if(openPrice>0)
									openPrice -=(float)dInterestStart;
								if(nowClose>0)
									nowClose -=(float)dInterestEnd;
								if(prePrice>0)
									prePrice -=(float)dInterestEnd;
							}
						}
					}
				}
			}
			//计算涨跌幅
			if(nowClose>0 && preClose>0)
			{
				//计算涨跌
				raiseValue = nowClose-preClose;
				raise = raiseValue/preClose*100;

				////iPrecise
				//int iUpRaise = 1;
				//int ifRaise = 0;
				//ifRaise = GetDataByScale((float)raise,iPrecise,iUpRaise);
				//raise = (float)(ifRaise/(float)iUpRaise);
				//int iPreciseEnd = iPrecise+2;
				//double dadd = 5;
				//for(int a=0;a<iPreciseEnd;a++)
				//{
				//	dadd /= 10;
				//}
				//raise += dadd;
			}

			//3 阶段涨跌幅（％）：= (收盘价 - 前收价) / 前收价 * 100
			resTable.SetCell(iCol,i,raise);    // 当选择剔除涨幅时的计算方式 2010-12-10 wanglm
			iCol++;
			//4涨跌
			resTable.SetCell(iCol,i,raiseValue);
			iCol++;

			
			if(bOnlyRaise==false)
			{
				////5 preclose
				//if(prePrice<0)
				//	prePrice = cDAY_HQ_ITEM.Preclose;
				//resTable.SetCell(iCol,i,prePrice);
				//iCol++;

				////6 open
				//if(openPrice<0)
				//	openPrice=cDAY_HQ_ITEM.Open;
				//resTable.SetCell(iCol,i,openPrice);
				//iCol++;

				////7 close
				//if(nowClose<0)
				//	nowClose=cDAY_HQ_ITEM.Close;
				//resTable.SetCell(iCol,i,nowClose);
				//iCol++;

				// 当最新交易日(iTradeDateLast)<起始日期(startDate)<=终止日期(endDate) 时 前收、开盘、收盘无值，显示"-"
				// bug:12192   2012-6-10
				if (bIsToday == true && startDate > iToday/*iTradeDateLast*/ && startDate <=endDate)
				{
					//5 preclose
					prePrice = Con_floatInvalid;
					resTable.SetCell(iCol,i,prePrice);
					iCol++;

					//6 open
					openPrice = Con_floatInvalid;
					resTable.SetCell(iCol,i,openPrice);
					iCol++;

					//7 close
					nowClose = Con_floatInvalid;
					resTable.SetCell(iCol,i,nowClose);
					iCol++;
				}
				else if (bIsToday == false && startDate > iTradeDateLast && startDate <= endDate)
				{
					//5 preclose
					prePrice = Con_floatInvalid;
					resTable.SetCell(iCol,i,prePrice);
					iCol++;

					//6 open
					openPrice = Con_floatInvalid;
					resTable.SetCell(iCol,i,openPrice);
					iCol++;

					//7 close
					nowClose = Con_floatInvalid;
					resTable.SetCell(iCol,i,nowClose);
					iCol++;
				}
				else
				{
					//5 preclose
					if(prePrice<0)
						prePrice = cDAY_HQ_ITEM.Preclose;
					resTable.SetCell(iCol,i,prePrice);
					iCol++;

					//6 open
					if(openPrice<0)
						openPrice=cDAY_HQ_ITEM.Open;
					resTable.SetCell(iCol,i,openPrice);
					iCol++;

					//7 close
					if(nowClose<0)
						nowClose=cDAY_HQ_ITEM.Close;
					resTable.SetCell(iCol,i,nowClose);
					iCol++;
				}

				//8 阶段成交量（万股）
				double v = Con_doubleInvalid;
				if(bOnlyToday == false)
				{
					if(pStart!=NULL && pEnd!=NULL && pEnd->SumVolume>0 && pStart->SumVolume>0)
					{
						v = pEnd->SumVolume-pStart->SumVolume;

						//if(bCutFirstDateRaise==false || bIsFirstDay==false)
						{
							if(pStart->Volume>0)
								v += pStart->Volume;
						}

						if(!(v>0))
							v = Con_doubleInvalid;
					}
				}
				else
				{
					if(pEnd!=NULL)
						v = pEnd->Volume;
				}
				resTable.SetCell(iCol,i,v);
				iCol++;

				//9 阶段成交额（亿元）
				double a = Con_doubleInvalid;
				if(bOnlyToday == false)
				{
					if(pStart!=NULL && pEnd!=NULL && pEnd->SumAmount>0 && pStart->SumAmount>0)
					{
						a = pEnd->SumAmount-pStart->SumAmount;
						//if(bCutFirstDateRaise==false || bIsFirstDay==false)
						{
							if(pStart->Amount>0)
								a+=pStart->Amount;
						}
						if(!(a>0))
							a = Con_doubleInvalid;
					}
				}
				else
				{
					if(pEnd!=NULL)
					a = pEnd->Amount;
				}
				resTable.SetCell(iCol,i,a);
				iCol++;

				double tValue = Con_doubleInvalid;

				//10换手率=成交量/流通股本*100
				if(bIsLineData == false)
				{
					//if(pEnd!=NULL && pEnd->TradeableShare>0 && v>0)
					//	tValue = v/pEnd->TradeableShare*100;
					if(bOnlyToday == false)
					{
						if(pEnd!=NULL && pEnd->TurnOverSum>0 && pStart!=NULL && pStart->TurnOverSum>0)
						{
							//tValue = pEnd->TurnOverSum-pStart->TurnOverSum+pStart->TurnOver;
							tValue = pEnd->TurnOverSum-pStart->TurnOverSum;//+pStart->TurnOver;
							//if(bCutFirstDateRaise==false || bIsFirstDay==false)
							{
								if(pStart->TurnOver>0)
									tValue+=pStart->TurnOver;
							}
							//tValue = v/pEnd->TradeableShare*100;
						}
					}
					else
					{
						if(pEnd!=NULL)
						tValue = pEnd->TurnOver;
					}
				}
				resTable.SetCell(iCol,i,tValue);
				iCol++;

				//iDays=交易天数
				//11交易天数
				int lDays = 0;
				
				if(bOnlyToday == false)
				{
					if(pStart!=NULL && pEnd!=NULL && pStart->lSumTradeDays>0 && pEnd->lSumTradeDays>0)
					{
						lDays = pEnd->lSumTradeDays-pStart->lSumTradeDays+1;
						if(bIsToday==true)
						{
							//2008-05-05
							//如果最新交易日期不是今日,则有交易的交易实体天数要加1
							//2008-05-15
							if(bUseNewlyData==false)
							{
								//使用上个交易日的横向数据
								//if(pLastestDateData!=NULL && (pLastestDateData->Volume>0 || pSecurity->InXfIndexBlock()==true) && iStartDate<iEndDate && iEndDate<endDate)
								//if(pLastestDateData!=NULL && (pLastestDateData->Volume>0 || (pSecurity->IsNormal()==true && pSecurity->IsStop()==false)) && iStartDate<iEndDate && iEndDate<endDate)
								//if(pLastestDateData!=NULL && pLastestDateData->Close>0 && pSecurity->AllowTradeToday()==true && iStartDate<=iEndDate && iEndDate<endDate)
								if(pLastestDateData!=NULL && pLastestDateData->Close>0 && pSecurity->AllowTradeToday()==true && iEndDate<endDate)
									
								{
									lDays += 1;
								}
							}
							else
							{
								//允许使用今日的横向数据
								//if(iTradeDateLast<iToday && pLastestDateData!=NULL && (pLastestDateData->Volume>0 || pSecurity->InXfIndexBlock()==true) && iStartDate<iEndDate && iEndDate<endDate)
								//if(iTradeDateLast<iToday && pLastestDateData!=NULL && pLastestDateData->Close>0 && pSecurity->AllowTradeToday()==true && iStartDate<iEndDate && iEndDate<endDate)
								if(iTradeDateLast<iToday && pLastestDateData!=NULL && pLastestDateData->Close>0 && pSecurity->AllowTradeToday()==true && iEndDate<endDate)
								{
									lDays += 1;
								}
							}
						}
						if(bIsLineData == false)
						{
							if(lDays>iMaxDays)
							{
								sLog.Format(_T("阶段行情数据错误: 交易天数[%d]大于自然天数[%d][%s-%s-%d-%d]！"),
									lDays,iMaxDays,
									pSecurity->GetName(),
									pSecurity->GetCode(true),
									startDate,
									iEndDate);
								Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
								lDays = 0;
							}
						}
					}
				}
				else
				{
					//if(pEnd!=NULL && pSecurity->IsNormal()==true)
					//fix bug 9131,刘鹏,2011-11-1
					if(pSecurity->IsNormal()==true && pSecurity->IsHaltLong()==false && pSecurity->IsValid())
						lDays = 1;
					else
					{
						sLog.Format(_T("阶段行情: 没有盘中数据[%s-%s-%d-%d]！"),
							pSecurity->GetName(),
							pSecurity->GetCode(true),
							startDate,
							iEndDate);
						Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
					}
				}
				if(lDays<0)
				{
					sLog.Format(_T("阶段行情: 交易天数[%d]无效[%s-%s-%d-%d]！"),
						lDays,
						pSecurity->GetName(),
						pSecurity->GetCode(true),
						startDate,
						iEndDate
						);
					Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
					lDays = 0;
				}
				
				resTable.SetCell(iCol,i,lDays);
				iCol++;

				//12日换手率
				if(bIsLineData == false)
				{
					if(tValue>0 && lDays>0)
						tValue /= lDays;
					else
						tValue = Con_doubleInvalid;
				}
				else
					tValue = Con_doubleInvalid;
				resTable.SetCell(iCol,i,tValue);
				iCol++;

				tValue = Con_doubleInvalid;

				//13
				//均价
				if(pSecurity->IsIndex() == true)
				{
					if(pEnd!=NULL)
					{
						if(
							bIsLineData==true
							&& lDays>0 && dCustomIndexAvgPrice>0 && bOnlyToday==false
							
						  )
							tValue = dCustomIndexAvgPrice / lDays;
						else
							tValue = pEnd->Close;
					}
					resTable.SetCell(iCol,i,tValue);
				}
				else
				{
					if(v>0 && a>0)
					{
						//2009-05-25 均价复权
						resTable.SetCell(iCol,i,a/v*dExdividendScale);
					}
					else
						resTable.SetCell(iCol,i,tValue);
				}
				iCol++;

				//14 日均成交量
				/*
				if(lDays>0 && pSecurity->IsIndexCustomByCode()==false)
				{
					if(v>0)
					resTable.SetCell(iCol,i,v/lDays);
					else
					resTable.SetCell(iCol,i,tValue);
					iCol++;
					//15 日均成交额
					if(a>0)
					resTable.SetCell(iCol,i,a/lDays);
					else
					resTable.SetCell(iCol,i,tValue);
					iCol++;
				}
				else
				{
					resTable.SetCell(iCol,i,tValue);
					iCol++;
					//15 日均成交额
					resTable.SetCell(iCol,i,tValue);
					iCol++;
				}
				*/
				tValue = Con_doubleInvalid;
				if(lDays>0)
				{
					if(v>0)
					resTable.SetCell(iCol,i,v/lDays);
					else
					resTable.SetCell(iCol,i,tValue);
					iCol++;
					//15 日均成交额
					if(a>0)
					resTable.SetCell(iCol,i,a/lDays);
					else
					resTable.SetCell(iCol,i,tValue);
					iCol++;
				}
			}
			i++;
		}	// end of: if (security != NULL)

		msg.Format(_T("IndexWnd6::BlockCycleRateAdv-Calc--End--SecurityID:%d"),*iter);
		CLogRecorder::GetInstance()->WriteToLog(msg);
	}	// end of: each security 

	//CLogRecorder::GetInstance()->WriteToLog(_T("IndexWnd6::BlockCycleRateAdv--Calc--step4-End"));
	//step5
	if(bOnlyRaise==false)
	{
		prw.SetPercent(progId, 1.0);
		sProgressPrompt += _T(",完成!");
		prw.SetText(progId,sProgressPrompt);
	}

	//释放每日行情数据内存
	delete pOneDayTradeData;
	delete pOneDayTradeData1;

	//释放内存
	delete pFirstDateData;
	delete pLastestDateData;

	//delete pStartDateData;
	//delete pEndDateData;
	double dValue;
	if(bOnlyRaise==false)
	{
		for(UINT i=0;i<resTable.GetRowCount();i++)
		{
			int iCol = 8;
			resTable.GetCell(iCol,i,dValue);
			if(dValue>-10000000)
			{
				resTable.SetCell(iCol,i,dValue/10000);
				/*
				if(iFlag==0)
					resTable.SetCell(iCol,i,dValue/100);
				else if(iFlag==1)
					resTable.SetCell(iCol,i,dValue/10);
				*/
			}
			iCol++;
			resTable.GetCell(iCol,i,dValue);
			if(dValue>-10000000)
				resTable.SetCell(iCol,i,dValue/10000);
			iCol = 14;
			resTable.GetCell(iCol,i,dValue);
			if(dValue>-10000000)
			{
				/*
				if(iFlag==0)
					resTable.SetCell(iCol,i,dValue/100);
				else if(iFlag==1)
					resTable.SetCell(iCol,i,dValue/10);
				*/
				resTable.SetCell(iCol,i,dValue/10000);
			}
			iCol++;
			resTable.GetCell(iCol,i,dValue);
			if(dValue>-10000000)
				resTable.SetCell(iCol,i,dValue/10000);
		}
	}

	//CLogRecorder::GetInstance()->WriteToLog(_T("IndexWnd6::BlockCycleRateAdv--Calc--End"));
	return true;
}

//阶段涨幅[近一周、近一月、近一季、近一年]
bool TxBusiness::BlockCycleRateHome(std::vector<int>& iSecurityId, int date, Tx::Core::Table_Display& table,bool bHaveIndex)
{
	int ii = iSecurityId.size();

	if (ii <= 0)
		return false;

	table.Clear();
	table.AddCol(Tx::Core::dtype_int4, false);	    // 0 - Security指针
	table.AddCol(Tx::Core::dtype_double, false);	// 1 - 最近一周涨幅
	table.SetPrecise(1, 2);
	table.AddCol(Tx::Core::dtype_double, false);	// 1 - 最近一月涨幅
	table.SetPrecise(2, 2);
	table.AddCol(Tx::Core::dtype_double, false);	// 1 - 最近一季涨幅
	table.SetPrecise(3, 2);
	table.AddCol(Tx::Core::dtype_double, false);	// 1 - 最近一年涨幅
	table.SetPrecise(4, 2);

	table.AddRow(ii);

	Tx::Core::Table_Indicator resTable;
	//step1
	//默认的返回值状态
	bool result = false;
	//清空数据
	resTable.Clear();
	int iCol = 0;
	//准备样本集=第一参数列:F_Security_ID,int型
	resTable.AddParameterColumn(Tx::Core::dtype_int4);
	//根据样本数量添加记录数
	resTable.AddRow(ii);
	//添加券ID
	for (int r=0; r<(int)iSecurityId.size(); r++)
	{
		resTable.SetCell(0,r,iSecurityId[r]);
	}

    long iIndicator = 30300044;   //指标 = 过去一周涨幅
	UINT varCfg[1];			//参数配置
	int varCount=1;			//参数个数
	for (int i = 0; i < 4; i++)
	{
		if(i==3)
		    GetIndicatorDataNow(iIndicator+i+1);
		else
			GetIndicatorDataNow(iIndicator+i);
		if (m_pIndicatorData==NULL)
		{
			result = false;
			break;
		}
		varCfg[0]=0;
		result = m_pLogicalBusiness->SetIndicatorIntoTable(
			m_pIndicatorData,	//指标
			varCfg,				//参数配置
			varCount,			//参数个数
			resTable
			);
		if(result==false)
			break;
	}

	//根据之前3个步骤的设置进行数据读取，结果数据存放在table中
	result = m_pLogicalBusiness->GetData(resTable);

	if(result==false)
	{
		for (int r=0;r<(int)iSecurityId.size();r++)
		{
			Security* pSecurity = GetSecurityNow(iSecurityId[r]);
			table.SetCell(0,r,(int)pSecurity);
			table.SetCell(1,r,0.0);
			table.SetCell(2,r,0.0);
			table.SetCell(3,r,0.0);
			table.SetCell(4,r,0.0);
		}

		if (bHaveIndex)
		{
			table.AddRow();
			Security* pSecurity = GetSecurityNow(4998600);
			table.SetCell(0,table.GetRowCount()-1,(int)pSecurity);
			table.SetCell(1,table.GetRowCount()-1,0.0);
			table.SetCell(2,table.GetRowCount()-1,0.0);
			table.SetCell(3,table.GetRowCount()-1,0.0);
			table.SetCell(4,table.GetRowCount()-1,0.0);
		}

		return false;
	}

#ifdef _DEBUG
	CString strTable1=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable1);
#endif

	for (int r=0; r<resTable.GetRowCount();r++)
	{
		int nTmp = 0;
		resTable.GetCell(0,r,nTmp); //id
		//0 - Security
		Security* pSecurity = GetSecurityNow(nTmp);
	    nTmp = (int)pSecurity;
		table.SetCell(0,r,nTmp);
		//1 - 近一周涨幅
		double dValue = 0.00;
		resTable.GetCell(1,r,dValue);
		table.SetCell(1,r,dValue);
		//2 - 近一月涨幅
		resTable.GetCell(2,r,dValue);
		table.SetCell(2,r,dValue);
		//3 - 近一季涨幅
		resTable.GetCell(3,r,dValue);
		table.SetCell(3,r,dValue);
		//4 - 近一年涨幅
		resTable.GetCell(4,r,dValue);
		table.SetCell(4,r,dValue);
	}

	resTable.Clear();

	if (bHaveIndex)
	{
		table.AddRow();
		Security* pSecurity = GetSecurityNow(4998600);
		table.SetCell(0,table.GetRowCount()-1,(int)pSecurity);
		table.SetCell(1,table.GetRowCount()-1,0.0);
		table.SetCell(2,table.GetRowCount()-1,0.0);
		table.SetCell(3,table.GetRowCount()-1,0.0);
		table.SetCell(4,table.GetRowCount()-1,0.0);
	}


#ifdef _DEBUG
	strTable1=table.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable1);
#endif

	return true;
}
//
double TxBusiness::GetBondInterest(SecurityQuotation* pSecurity,int iDate)
{
	return 0;
}

void TxBusiness::TableOutput(Tx::Core::Table_Indicator &resTable)
{
	CString strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
	TRACE(strTable);
}
//设置数据表的列信息
void TxBusiness::SetDisplayTableColInfo(Table_Display* pTable,int iCol,int iIndicatorId,bool bSetTitle)
{
	if(pTable==NULL)
		return ;
	GetIndicatorDataNow(iIndicatorId);
	if(m_pIndicatorData==NULL)
		return;
	//标题
	if(bSetTitle==true)
	{
		CString sTitle;
		CString sUnit(m_pIndicatorData->sOutputUnit);
		CString sDisName(m_pIndicatorData->sDisPlayName);

		if(sUnit.GetLength()>0)
			sTitle.Format(_T("%s(%s)"),sDisName,sUnit);
		else
			sTitle = sDisName;
		pTable->SetTitle(iCol,sTitle);
	}
	//小数点
	pTable->SetPrecise(iCol, m_pIndicatorData->byOutputItemDec);
	//倍数
	pTable->SetOutputItemRatio(iCol,m_pIndicatorData->fOutputItemRatio);
}
//处理实型数据小数位数
int TxBusiness::GetDataByScale(float fValue,int iScale,int& iRatio)
{
	iScale++;
	int iV = 1;
	for(int i=0;i<iScale;i++)
		iV *=10;
	iRatio = iV;
	//return ((int)(fValue*iV)+5.05)/10*10;
	return ((int)(fValue*iV)+5)/10*10;
}
//取得预测pe的年度
int TxBusiness::GetPeYear(void)
{
	int iCurYear = m_pFunctionDataManager->GetServerCurDateTime_ShangHai().GetYear();
	int iCurMonth = m_pFunctionDataManager->GetServerCurDateTime_ShangHai().GetMonth();
	//int iCurDay = m_pFunctionDataManager->GetServerCurDateTime_ShangHai().GetDay();

	if(iCurMonth<4)//4月1日之前取上年,否则取当年
		iCurYear--;
	iCurYear%=100;
	return iCurYear;
}
int TxBusiness::GetPeYear(std::vector<int> arrSamples)
{
	int iCurYear = m_pFunctionDataManager->GetServerCurDateTime_ShangHai().GetYear();
	std::vector<int>::iterator iter = arrSamples.begin();
	int iCurYearTmp = 0;
	for(;iter != arrSamples.end();iter++)
	{
		SecurityQuotation* psq = GetSecurityNow(*iter);
		if(psq!=NULL)
		{
			SecurityEPS_PE* pSecurityEPS_PE = psq->GetSecurityEPS_PE();
			if(pSecurityEPS_PE != NULL)
			{
				if(iCurYearTmp < pSecurityEPS_PE->f_year)
					iCurYearTmp = pSecurityEPS_PE->f_year;
			}
		}
	}	
	if(iCurYearTmp < 1)
	{
		int iCurMonth = m_pFunctionDataManager->GetServerCurDateTime_ShangHai().GetMonth();
		if(iCurMonth<4)//4月1日之前取上年,否则取当年
			iCurYear--;		
	}
	else
	{
		iCurYear = iCurYearTmp+1;
	}
	iCurYear %= 100;
	return iCurYear;
}
//2008-03-26
//输出周期行情
//void TxBusiness::SecurityExportDuration(void)
//{
//	SecurityTradeDuration* pSecurityTradeDuration = new SecurityTradeDuration;
//	if(pSecurityTradeDuration==NULL)
//		return;
//	//
//	delete pSecurityTradeDuration;
//
//}

bool TxBusiness::SetTableCol(Table_Display& baTable)
{
	//如果UI没有输入列信息，自行添加
	if(baTable.GetColCount()<=0)
	{
		int nCol=0;
		//加入样本for test
		//交易实体ID
		//baTable.AddCol(Tx::Core::dtype_int4);
		//nCol++;
		////交易实体名称
		//baTable.AddCol(Tx::Core::dtype_val_string);
		//nCol++;
		////交易实体外码
		//baTable.AddCol(Tx::Core::dtype_val_string);
		//nCol++;

		//交易日期
		baTable.AddCol(Tx::Core::dtype_int4);
		baTable.SetFormatStyle(nCol,fs_date);
		nCol++;
		//涨幅
		baTable.AddCol(Tx::Core::dtype_double);
		nCol++;
		//涨跌
		baTable.AddCol(Tx::Core::dtype_double);
		nCol++;
		//前收
		baTable.AddCol(Tx::Core::dtype_float);
		nCol++;
		//开盘
		baTable.AddCol(Tx::Core::dtype_float);
		nCol++;
		//最高
		baTable.AddCol(Tx::Core::dtype_float);
		nCol++;
		//最低
		baTable.AddCol(Tx::Core::dtype_float);
		nCol++;
		//收盘
		baTable.AddCol(Tx::Core::dtype_float);
		nCol++;
		//成交量
		baTable.AddCol(Tx::Core::dtype_double);
		nCol++;
		//成交金额
		baTable.AddCol(Tx::Core::dtype_double);
		nCol++;

		nCol=0;
		//baTable.SetTitle(nCol, _T("名称"));
		//++nCol;
		baTable.SetTitle(nCol, _T("日期"));
		++nCol;
		baTable.SetTitle(nCol, _T("涨幅"));
		baTable.SetPrecise(nCol, 4);
		++nCol;
		baTable.SetTitle(nCol, _T("涨跌"));
		baTable.SetPrecise(nCol, 4);
		++nCol;
		baTable.SetTitle(nCol, _T("前收"));
		baTable.SetPrecise(nCol, 4);
		++nCol;
		baTable.SetTitle(nCol, _T("开盘"));
		baTable.SetPrecise(nCol, 4);
		++nCol;
		baTable.SetTitle(nCol, _T("最高"));
		baTable.SetPrecise(nCol, 4);
		++nCol;
		baTable.SetTitle(nCol, _T("最低"));
		baTable.SetPrecise(nCol, 4);
		++nCol;
		baTable.SetTitle(nCol, _T("收盘"));
		baTable.SetPrecise(nCol, 4);
		++nCol;
		baTable.SetTitle(nCol, _T("成交量(股)"));
		baTable.SetPrecise(nCol, 0);
		//baTable.SetOutputItemRatio(nCol, 100);
		baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
		++nCol;
		baTable.SetTitle(nCol, _T("成交金额(万元)"));
		baTable.SetPrecise(nCol, 4);//2
		baTable.SetOutputItemRatio(nCol, 10000);
		baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
		++nCol;
	}
	return true;
}
bool TxBusiness::SetTableData(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow,int idate,int iFlag,bool bFirst,int iWeight)
{
	////交易实体id
	//baTable.SetCell(nCol,nRow,(int)pSecurity->GetId());
	//nCol++;

	////名称
	//baTable.SetCell(nCol,nRow,pSecurity->GetName());
	//nCol++;

	HisTradeData* pHisTradeData = pSecurity->GetTradeDataByIndex(nRow,iFlag,bFirst);
	if(pHisTradeData==NULL)
		return false;
	double dValue = Con_doubleInvalid;
	double dValueRaise = Con_doubleInvalid;
	float fOpen,fHigh,fLow,fClose,fPreClose;
	double dVolume,dAmount;

	int iDataDate = pHisTradeData->Date;

	fOpen = pHisTradeData->Open;
	fClose = pHisTradeData->Close;
	fHigh = pHisTradeData->High;
	fLow = pHisTradeData->Low;
	fPreClose = pHisTradeData->Preclose;
	dVolume = pHisTradeData->Volume;
	dAmount = pHisTradeData->Amount;

	//iWeight == -1 表示不复权
	//0-前复权
	//1-后复权
	//2-全复权前复权
	//3-全复权后复权
	if(iWeight!=-1)
	{
		int iFirstDate = 0;
		HisTradeData* pFirstHisTradeData = pSecurity->GetTradeDataByIndex(0,iFlag,bFirst);
		if(pFirstHisTradeData!=NULL)
			iFirstDate = pFirstHisTradeData->Date;

		if(iWeight == 0)
		{
			double dScale = pSecurity->GetExdividendScale(iFirstDate,iDataDate,true);
			if(fClose>0)
				fClose = (float)(fClose*dScale);
			if(fOpen>0)
				fOpen = (float)(fOpen*dScale);
			if(fHigh>0)
				fHigh = (float)(fHigh*dScale);
			if(fLow>0)
				fLow = (float)(fLow*dScale);
			if(fPreClose>0)
				fPreClose = (float)(fPreClose*dScale);
		}
		else if(iWeight == 1)
		{
			double dScale = pSecurity->GetExdividendScale(iFirstDate,iDataDate,false);
			if(fClose>0)
				fClose = (float)(fClose*dScale);
			if(fOpen>0)
				fOpen = (float)(fOpen*dScale);
			if(fHigh>0)
				fHigh = (float)(fHigh*dScale);
			if(fLow>0)
				fLow = (float)(fLow*dScale);
			if(fPreClose>0)
				fPreClose = (float)(fPreClose*dScale);
		}
		else if(iWeight == 2)
		{
			double dScale = pSecurity->GetExdividendScale(iDataDate,21001231,true);
			if(fClose>0)
				fClose = (float)(fClose*dScale);
			if(fOpen>0)
				fOpen = (float)(fOpen*dScale);
			if(fHigh>0)
				fHigh = (float)(fHigh*dScale);
			if(fLow>0)
				fLow = (float)(fLow*dScale);
			if(fPreClose>0)
				fPreClose = (float)(fPreClose*dScale);
		}
		else if(iWeight == 3)
		{
			double dScale = pSecurity->GetExdividendScale(19791201,iDataDate,false);
			if(fClose>0)
				fClose = (float)(fClose*dScale);
			if(fOpen>0)
				fOpen = (float)(fOpen*dScale);
			if(fHigh>0)
				fHigh = (float)(fHigh*dScale);
			if(fLow>0)
				fLow = (float)(fLow*dScale);
			if(fPreClose>0)
				fPreClose = (float)(fPreClose*dScale);
		}
	}
	if(fClose<0 || fPreClose<0)
		dValue = Con_doubleInvalid;
	else
		dValue = fClose - fPreClose;

	//日期
	baTable.SetCell(nCol,nRow,iDataDate);
	nCol++;

	//涨幅
	dValueRaise = dValue;
	if( fabs(dValueRaise-Con_doubleInvalid)>0.000001 && fPreClose>0)
	{
		dValueRaise /= fPreClose;
		dValueRaise *= 100;
	}
	baTable.SetCell(nCol,nRow,dValueRaise);
	nCol++;

	//涨跌
	baTable.SetCell(nCol,nRow,dValue);
	nCol++;

	//前收
	baTable.SetCell(nCol,nRow,fPreClose);
	nCol++;

	//开盘
	baTable.SetCell(nCol,nRow,fOpen);
	nCol++;

	//最高
	baTable.SetCell(nCol,nRow,fHigh);
	nCol++;

	//最低
	baTable.SetCell(nCol,nRow,fLow);
	nCol++;

	//收盘
	baTable.SetCell(nCol,nRow,fClose);
	nCol++;

	//成交量
	baTable.SetCell(nCol,nRow,dVolume);
	nCol++;
	//成交金额
	baTable.SetCell(nCol,nRow,dAmount);
	nCol++;

	return true;
}
//added by zhangxs 股票交易数据Table
bool TxBusiness::SetStockTableCol(Table_Display& baTable,SecurityQuotation* pSecurity)
{
	//如果UI没有输入列信息，自行添加
	if(baTable.GetColCount()<=0)
	{
		int nCol=0;

		//交易日期
		baTable.AddCol(Tx::Core::dtype_int4);
		baTable.SetFormatStyle(nCol,fs_date);
		nCol++;
		//涨幅
		baTable.AddCol(Tx::Core::dtype_double);
		nCol++;
		//涨跌
		baTable.AddCol(Tx::Core::dtype_double);
		nCol++;
		//前收
		baTable.AddCol(Tx::Core::dtype_float);
		nCol++;
		//开盘
		baTable.AddCol(Tx::Core::dtype_float);
		nCol++;
		//最高
		baTable.AddCol(Tx::Core::dtype_float);
		nCol++;
		//最低
		baTable.AddCol(Tx::Core::dtype_float);
		nCol++;
		//收盘
		baTable.AddCol(Tx::Core::dtype_float);
		nCol++;
		//均价
		baTable.AddCol(Tx::Core::dtype_float);
		nCol++;
		////涨幅
		//baTable.AddCol(Tx::Core::dtype_double);
		//nCol++;
		////涨跌
		//baTable.AddCol(Tx::Core::dtype_double);
		//nCol++;
		//成交量
		baTable.AddCol(Tx::Core::dtype_double);
		nCol++;
		//成交金额
		baTable.AddCol(Tx::Core::dtype_double);
		nCol++;
		//换手率
		baTable.AddCol(Tx::Core::dtype_double);
		nCol++;
		//流通市值
		baTable.AddCol(Tx::Core::dtype_double);
		nCol++;
		////境内总值
		//baTable.AddCol(Tx::Core::dtype_double);
		//nCol++;
		//总市值
		baTable.AddCol(Tx::Core::dtype_double);
		nCol++;
		//封闭式基金/etf/lof和沪市b股保留3位
		if(pSecurity->IsFund_Close()||pSecurity->IsFund_ETF()||pSecurity->IsFund_LOF() 
			|| (pSecurity->IsStockB()&&pSecurity->IsShanghai())
			|| pSecurity->IsFund_QDII()
			|| pSecurity->IsFund_Intermix() || pSecurity->IsFund_Bond()//混合型和债券型也保留3位   2012-09-25
			)
		{
			//封闭式基金交易数据保留3位小数
			nCol=0;
			//baTable.SetTitle(nCol, _T("名称"));
			//++nCol;
			baTable.SetTitle(nCol, _T("日期"));
			++nCol;
			baTable.SetTitle(nCol, _T("涨幅(%)"));
			baTable.SetPrecise(nCol, 3);
			++nCol;
			baTable.SetTitle(nCol, _T("涨跌"));
			baTable.SetPrecise(nCol, 3);
			++nCol;		
			baTable.SetTitle(nCol, _T("前收"));
			baTable.SetPrecise(nCol, 3);
			++nCol;
			baTable.SetTitle(nCol, _T("开盘"));
			baTable.SetPrecise(nCol, 3);
			++nCol;
			baTable.SetTitle(nCol, _T("最高"));
			baTable.SetPrecise(nCol, 3);
			++nCol;
			baTable.SetTitle(nCol, _T("最低"));
			baTable.SetPrecise(nCol, 3);
			++nCol;
			baTable.SetTitle(nCol, _T("收盘"));
			baTable.SetPrecise(nCol, 3);
			++nCol;
			baTable.SetTitle(nCol, _T("均价"));
			baTable.SetPrecise(nCol, 3);
			++nCol;
			/*baTable.SetTitle(nCol, _T("涨幅(%)"));
			++nCol;
			baTable.SetTitle(nCol, _T("涨跌(元)"));
			++nCol;*/

			baTable.SetTitle(nCol, _T("成交量(份)"));//("成交量(股)")); BUG:13052    2012-09-26
			baTable.SetPrecise(nCol, 0);
			//baTable.SetOutputItemRatio(nCol, 100);
			baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
			++nCol;
			baTable.SetTitle(nCol, _T("成交金额(万)"));
			baTable.SetPrecise(nCol, 2); //2
			baTable.SetOutputItemRatio(nCol, 10000);
			baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
			++nCol;
			baTable.SetTitle(nCol, _T("换手率(%)"));
			++nCol;
			baTable.SetTitle(nCol, _T("流通市值(亿)"));
			baTable.SetPrecise(nCol, 2); //2
			baTable.SetOutputItemRatio(nCol, 1e+8);
			baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
			++nCol;
			/*baTable.SetTitle(nCol, _T("境内总值(亿)"));
			baTable.SetPrecise(nCol, 2);
			baTable.SetOutputItemRatio(nCol, 1e+8);
			baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
			++nCol;*/
			baTable.SetTitle(nCol, _T("总市值(亿)"));
			baTable.SetPrecise(nCol, 2); //2
			baTable.SetOutputItemRatio(nCol, 1e+8);
			baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
			++nCol;
		}
		else
		{   // by 20101026 wanglm 修改保留小数点后4位
			nCol=0;
			//baTable.SetTitle(nCol, _T("名称"));
			//++nCol;
			baTable.SetTitle(nCol, _T("日期"));
			++nCol;
			baTable.SetTitle(nCol, _T("涨幅(%)"));
			baTable.SetPrecise(nCol, 2); //2
			++nCol;
			baTable.SetTitle(nCol, _T("涨跌"));
			baTable.SetPrecise(nCol, 2); //2
			++nCol;		
			baTable.SetTitle(nCol, _T("前收"));
			baTable.SetPrecise(nCol, 2); //2
			++nCol;
			baTable.SetTitle(nCol, _T("开盘"));
			baTable.SetPrecise(nCol, 2); //2
			++nCol;
			baTable.SetTitle(nCol, _T("最高"));
			baTable.SetPrecise(nCol, 2); //2
			++nCol;
			baTable.SetTitle(nCol, _T("最低"));
			baTable.SetPrecise(nCol, 2); //2
			++nCol;
			baTable.SetTitle(nCol, _T("收盘"));
			baTable.SetPrecise(nCol, 2); //2
			++nCol;
			baTable.SetTitle(nCol, _T("均价"));
			baTable.SetPrecise(nCol, 2); //2
			++nCol;
			/*baTable.SetTitle(nCol, _T("涨幅(%)"));
			++nCol;
			baTable.SetTitle(nCol, _T("涨跌(元)"));
			++nCol;*/

			baTable.SetTitle(nCol, _T("成交量(股)"));
			baTable.SetPrecise(nCol, 0); // 0
			//baTable.SetOutputItemRatio(nCol, 100);
			baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
			++nCol;
			baTable.SetTitle(nCol, _T("成交金额(万)"));
			baTable.SetPrecise(nCol, 2); // 2
			baTable.SetOutputItemRatio(nCol, 10000);
			baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
			++nCol;
			baTable.SetTitle(nCol, _T("换手率(%)"));
			baTable.SetPrecise(nCol, 2); //2
			++nCol;
			baTable.SetTitle(nCol, _T("流通市值(亿)"));
			baTable.SetPrecise(nCol, 2); // 2
			baTable.SetOutputItemRatio(nCol, 1e+8);
			baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
			++nCol;
			/*baTable.SetTitle(nCol, _T("境内总值(亿)"));
			baTable.SetPrecise(nCol, 2);
			baTable.SetOutputItemRatio(nCol, 1e+8);
			baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
			++nCol;*/
			baTable.SetTitle(nCol, _T("总市值(亿)"));
			baTable.SetPrecise(nCol, 2); // 2
			baTable.SetOutputItemRatio(nCol, 1e+8);
			baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
			++nCol;
		}
		
	}
	return true;
}

bool TxBusiness::GetFundNavData(Table_Display& baTable,
								   int iSecurityId,
								   int iStartdate,
								   int iEndDate,
								   int iFlag,
								   bool bFirst,
								   int iWeight,
								   bool bNeedProgressWnd)
{
	baTable.Clear();
	//step1 处理结果数据的标题列
	

	SecurityQuotation* psq = GetSecurityNow(iSecurityId);
	if(psq==NULL)
		return false;
	SetStockTableCol(baTable,psq);
	int iCol = 0;
	if(baTable.GetColCount()<=0)
		return false;

	//step2
	//控制是否需要进度条
	Tx::Core::ProgressWnd prw;
	CString sProgressPrompt;
	UINT progId = 0;
	if(bNeedProgressWnd==true)
	{
		//支持取消
		prw.EnableCancelButton(true);
		sProgressPrompt.Format(_T("交易数据..."));
		progId = prw.AddItem(1,sProgressPrompt, 0.0);
		prw.Show(15);
	}

	//step3 取得指定交易所的当前日期
	//int idate = 0;
	int i =0;

	int row= baTable.GetRowCount();
	int col = baTable.GetColCount();

	bool bNeedCheckToday = false;
	bool bNeedAppendToday = false;
	CalStatusAboutToday(psq,iEndDate,bNeedCheckToday,bNeedAppendToday);

	//step4 循环日期
	int count = psq->GetFundNetValueDataCount();
	//
	//int count = psq->GetTradeDateCount(iFlag);
	if(iWeight!=-1)
	{
		//只有沪深股票，基金和债券要处理复权
		if(
			psq->IsIndex() == true ||
			psq->IsWarrant() == true ||
			(psq->IsStock() == true && psq->IsHK_Market()==true) ||
			psq->IsFutures() == true
			)
			iWeight = -1;
	}
	std::vector<HisTradeData> m_vecHisTrade;
	float	preclose = -1; 
	for (i=0;i<count;i++)
	{
		if(bNeedProgressWnd==true)
		{
			prw.SetPercent(progId, (double)i/(double)count);
			if(prw.IsCanceled()==true)
				break;
		}

		//step 5.1
		//第一列为样本ID
		//取得样本的ID

		//step5.2 取得样本的Security指针
		//baTable.AddRow();
		//step5.3 处理基本指标数据
		int j = 0;
		//SetStockTableData(baTable,psq,j,i,iStartdate,iEndDate,iFlag,bFirst,iWeight);
		FundNetValueData * pNetData = psq->GetFundNetValueDataByIndex(i); 
		HisTradeData hisTradeData;
		memset(&hisTradeData,0,sizeof(HisTradeData));
		hisTradeData.Date = pNetData->iDate;
		hisTradeData.Preclose = preclose>0 ? preclose : pNetData->fNetvalue;
		hisTradeData.Open = pNetData->fNetvalue;
		hisTradeData.High = pNetData->fNetvalue;
		hisTradeData.Low = pNetData->fNetvalue;
		hisTradeData.Close = pNetData->fNetvalue;
		m_vecHisTrade.push_back(hisTradeData);
		preclose = pNetData->fNetvalue;

		if(bNeedCheckToday && iEndDate == hisTradeData.Date)
			bNeedAppendToday = false;
	}
	//刘鹏，2012-04-06，加入实时行情的数据
	PushBackToday(bNeedAppendToday,psq,m_vecHisTrade);

	GetCQDate(m_vecHisTrade,psq,iStartdate,iEndDate,iWeight);
	std::vector<HisTradeData> m_vecHisTradeTerm;
	// 2011-02-25 换手率计算修改
	if (iFlag != 0)
	{
		HisTradeData& tmp = m_vecHisTrade[0];
		memcpy(tmp.Res, (BYTE*)(&psq), sizeof(&psq));
	}
	TransitionData(m_vecHisTrade,m_vecHisTradeTerm,psq,iStartdate,iEndDate,iFlag);
	SetStockTableData(baTable,psq,m_vecHisTradeTerm,iStartdate,iEndDate,iFlag,bFirst,iWeight);

	//step6 循环完成
	if(bNeedProgressWnd==true)
	{
		prw.SetPercent(progId, 1.0);
		sProgressPrompt+=_T(",完成!");
		prw.SetText(progId, sProgressPrompt);
	}
	return true;
}

//股票和基金交易数据 20090326
bool TxBusiness::GetStockTradeData(Table_Display& baTable,
								   int iSecurityId,
								   int iStartdate,
								   int iEndDate,
								   int iFlag,
								   bool bFirst,
								   int iWeight,
								   bool bNeedProgressWnd)
{
	baTable.Clear();
	//step1 处理结果数据的标题列
	

	SecurityQuotation* psq = GetSecurityNow(iSecurityId);
	if(psq==NULL)
		return false;

	CString strCode = psq->GetCode(true);
	if (strCode.Find(_T(".OF")) > 0)
	{
		int idMaster = Tx::Data::FundNV2MasterMap::GetInstance()->GetMasterId(iSecurityId);
		if (idMaster > 0)
		{
			psq = (SecurityQuotation*)GetSecurity(idMaster);
		}
	}

	SetStockTableCol(baTable,psq);
	int iCol = 0;
	if(baTable.GetColCount()<=0)
		return false;

	//step2
	//控制是否需要进度条
	Tx::Core::ProgressWnd prw;
	CString sProgressPrompt;
	UINT progId = 0;
	if(bNeedProgressWnd==true)
	{
		//支持取消
		prw.EnableCancelButton(true);
		sProgressPrompt.Format(_T("交易数据..."));
		progId = prw.AddItem(1,sProgressPrompt, 0.0);
		prw.Show(15);
	}

	//step3 取得指定交易所的当前日期
	//int idate = 0;
	int i =0;

	int row= baTable.GetRowCount();
	int col = baTable.GetColCount();

	bool bNeedCheckToday = false;
	bool bNeedAppendToday = false;
	CalStatusAboutToday(psq,iEndDate,bNeedCheckToday,bNeedAppendToday);

	//step4 循环日期
	int count = psq->GetTradeDataCount();
	//
	//int count = psq->GetTradeDateCount(iFlag);
	if(iWeight!=-1)
	{
		//只有沪深股票，基金和债券要处理复权
		if(
			psq->IsIndex() == true ||
			psq->IsWarrant() == true ||
			(psq->IsStock() == true && psq->IsHK_Market()==true) ||
			psq->IsFutures() == true
			)
			iWeight = -1;
	}
	std::vector<HisTradeData> m_vecHisTrade;
	for (i=0;i<count;i++)
	{
		if(bNeedProgressWnd==true)
		{
			prw.SetPercent(progId, (double)i/(double)count);
			if(prw.IsCanceled()==true)
				break;
		}

		//step 5.1
		//第一列为样本ID
		//取得样本的ID

		//step5.2 取得样本的Security指针
		//baTable.AddRow();
		//step5.3 处理基本指标数据
		int j = 0;
		//SetStockTableData(baTable,psq,j,i,iStartdate,iEndDate,iFlag,bFirst,iWeight);
		HisTradeData* pHisTradeData = psq->GetTradeDataByIndex(i);
		if(pHisTradeData==NULL)
			continue;
		m_vecHisTrade.push_back(*pHisTradeData);

		if(bNeedCheckToday && iEndDate == pHisTradeData->Date)
			bNeedAppendToday = false;
	}
	//刘鹏，2012-04-06，加入实时行情的数据
	PushBackToday(bNeedAppendToday,psq,m_vecHisTrade);

	GetCQDate(m_vecHisTrade,psq,iStartdate,iEndDate,iWeight);
	std::vector<HisTradeData> m_vecHisTradeTerm;
	// 2011-02-25 换手率计算修改
	if (iFlag != 0)
	{
		HisTradeData& tmp = m_vecHisTrade[0];
		memcpy(tmp.Res, (BYTE*)(&psq), sizeof(&psq));
	}
	TransitionData(m_vecHisTrade,m_vecHisTradeTerm,psq,iStartdate,iEndDate,iFlag);
	SetStockTableData(baTable,psq,m_vecHisTradeTerm,iStartdate,iEndDate,iFlag,bFirst,iWeight);

	//step6 循环完成
	if(bNeedProgressWnd==true)
	{
		prw.SetPercent(progId, 1.0);
		sProgressPrompt+=_T(",完成!");
		prw.SetText(progId, sProgressPrompt);
	}
	return true;
}

//2008-07-29
//交易数据
bool TxBusiness::GetTradeData(Table_Display& baTable,int iSecurityId,int iFlag,bool bFirst,int iWeight,bool bNeedProgressWnd)
{
#ifdef _DEBUG
	GlobalWatch::_GetInstance()->WatchHere(_T("zhaohj|| 板块=交易数据-start"));
#endif

	baTable.Clear();
	//step1 处理结果数据的标题列
	SetTableCol(baTable);

	SecurityQuotation* psq = GetSecurityNow(iSecurityId);
	if(psq==NULL)
		return false;

	int iCol = 0;
	if(baTable.GetColCount()<=0)
		return false;

	//step2
	//控制是否需要进度条
	Tx::Core::ProgressWnd prw;
	CString sProgressPrompt;
	UINT progId = 0;
	if(bNeedProgressWnd==true)
	{
		//支持取消
		prw.EnableCancelButton(true);
		sProgressPrompt.Format(_T("交易数据..."));
		progId = prw.AddItem(1,sProgressPrompt, 0.0);
		prw.Show(15);
	}

	//step3 取得指定交易所的当前日期
	int idate = 0;
	int i =0;

	int row= baTable.GetRowCount();
	int col = baTable.GetColCount();
	//step4 循环日期
	//int count = psq->GetTradeDataCount();
	//
	int count = psq->GetTradeDateCount(iFlag);
	if(iWeight!=-1)
	{
		//只有沪深股票，基金和债券要处理复权
		if(
			psq->IsIndex() == true ||
			psq->IsWarrant() == true ||
			(psq->IsStock() == true && psq->IsHK_Market()==true) ||
			psq->IsFutures() == true
		  )
			iWeight = -1;
	}
	for (i=0;i<count;i++)
	{
		if(bNeedProgressWnd==true)
		{
			prw.SetPercent(progId, (double)i/(double)count);
			if(prw.IsCanceled()==true)
				break;
		}

		//step 5.1
		//第一列为样本ID
		//取得样本的ID

		//step5.2 取得样本的Security指针
		baTable.AddRow();
		//step5.3 处理基本指标数据
		int j = 0;
		SetTableData(baTable,psq,j,i,idate,iFlag,bFirst,iWeight);
	}

	//step6 循环完成
	if(bNeedProgressWnd==true)
	{
		prw.SetPercent(progId, 1.0);
		sProgressPrompt+=_T(",完成!");
		prw.SetText(progId, sProgressPrompt);
	}
#ifdef _DEBUG
	GlobalWatch::_GetInstance()->WatchHere(_T("zhaohj|| 板块=交易数据-end"));
#endif

	return true;
}
void BlockAmountFlowData(Table_Display& baTable,int iInsCol,int iDelCol,int iDelCodeCol,int iSamplesCount,int iBlockId,CString sBlockName, double dRaise)
{
	if(baTable.GetRowCount() <= 0)
	{
		baTable.AddRow();
	}
	else
	{
		//删除样本，保留汇总结果
		baTable.DeleteRow(0,baTable.GetRowCount()-1);
		baTable.Arrange();
	}
	//自选股最后一行为汇总
	int iNoRow = 0;//usbaTable.GetRowCount();
	//删除涨幅
	//baTable.DeleteCol(iDelCol);
	//删除代码
	baTable.DeleteCol(iDelCodeCol);
	//插入样本数量列
	baTable.InsertCol(iInsCol,dtype_int4);
	//写入样本数量
	baTable.SetCell(iInsCol,iNoRow,iSamplesCount);
	//写入名称
	baTable.SetCell(1,iNoRow,sBlockName);
	//板块id=自己定义也可以
	baTable.SetCell(0,iNoRow,iBlockId);
	//修正 流入/流出
	//取得流入，流出
	double dIn,dOut,dV=Con_doubleInvalid;
	baTable.GetCell(6,iNoRow,dIn);
	baTable.GetCell(7,iNoRow,dOut);
	if(!(dIn<0) && dOut>0)
	{
		dV = dIn/dOut;
	}
	//2008-12-08
	baTable.SetCell(4,iNoRow,dV);

	//2009-8-25
	baTable.SetCell(3,iNoRow, dRaise);
}
bool TxBusiness::GetBlockAmountFlow(
									std::vector<int>& vecSecurityId,
									int istart_date,
									int iend_date,
									Table_Display& baTable,
									bool bAddSamplesOnly,
									bool bStat,
									bool bDurRaise,
									int iMarketid,
									bool bFocusSamples)
									
{
	//step 1: 添加表头
	baTable.Clear();
	int col = 0;
	int row = 0;
	//板块id
	baTable.AddCol(dtype_int4);
	baTable.SetTitle(col++, _T("交易实体id"));

	//名称
	baTable.AddCol(dtype_val_string);
	baTable.SetTitle(col++, _T("名称"));

	//样本数
	baTable.AddCol(dtype_int4);
	baTable.SetTitle(col++, _T("样本数"));

	//涨幅
	baTable.AddCol(dtype_double);
	baTable.SetTitle(col++, _T("板块涨幅(%)"));

	//流入/流出
	baTable.AddCol(dtype_double);
	baTable.SetPrecise(col, 3);// cyh, 2009-08-18
	baTable.SetTitle(col++, _T("流入/流出"));

	//净流入
	baTable.AddCol(dtype_double);
	baTable.SetOutputItemRatio(col, 10000);
	baTable.SetFormatStyle(col, Tx::Core::fs_finance);
	baTable.SetTitle(col++, _T("净流入(万元)"));

	//流入
	baTable.AddCol(dtype_double);
	baTable.SetOutputItemRatio(col, 10000);
	baTable.SetFormatStyle(col, Tx::Core::fs_finance);
	baTable.SetTitle(col++, _T("流入(万元)"));

	//流出
	baTable.AddCol(dtype_double);
	baTable.SetOutputItemRatio(col, 10000);
	baTable.SetFormatStyle(col, Tx::Core::fs_finance);
	baTable.SetTitle(col++, _T("流出(万元)"));

	//step 2: param check
	if(vecSecurityId.size() <= 0)
	{
		if(bFocusSamples == true)
		{
			AfxMessageBox(_T("资金流向：样本不能为空!"));
		}
		return false;
	}

	if( istart_date>iend_date && 0 >= istart_date)
	{
		AfxMessageBox(_T("起始日期应该小于等于终止日期!"));
		return false;
	}
	//取得当日日期
	//int curDate = m_pFunctionDataManager->GetServerCurDateTime(iMarketid).GetDate().GetInt();
	//int curDataDate = m_pFunctionDataManager->GetCurDataDateTime(iMarketid).GetDate().GetInt();


	//step 3：阶段涨幅计算
	//2008-09-18
	//先计算阶段涨幅[col=3]
	Tx::Core::Table_Indicator durRaiseTable;
	if(bDurRaise == true)
	{
		if(BlockCycleRateAdv(
								vecSecurityId,	//交易实体ID
								istart_date,		//起始日期
								iend_date,		//终止日期
								true,			//剔除首日涨幅
			//0,				//复权类型 0:不复权;1:前复权;2:后复权;3:全复权+后复权;4:全复权+前复权
								2,//2009-05-11 徐勉：资金流向需要复权，后复权
								durRaiseTable,	//结果数据表
								2,
								0,				//计算类型0-默认；1-债券[bCutFirstDateRaise=true表示全价模式,bCutFirstDateRaise=false表示净价模式]
								iMarketid,		//交易所ID
								true,			//true只计算涨幅,false计算所有
								false
							)==false)
		{
			return false;
		}
		durRaiseTable.MakeReference(0);
	}
#ifdef _DEBUG
	CString strTable=durRaiseTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif

	//step 4:取得板块资金流向数据并追加板块涨幅列

	int iInsCol = 2;
	int iDelCol = 4;
	int iDelCodeCol = 2;

	int iBlockIndexId = 0;
	Table_Display tempTable;
	std::vector<int> vecSamples;
	CString strBlockName = _T("-");
	std::vector<int>::iterator iterSecurityId = vecSecurityId.begin();
	for (; iterSecurityId != vecSecurityId.end(); iterSecurityId++)
	{
		vecSamples.clear();
		tempTable.Clear();

		SecurityQuotation* pS = GetSecurityNow(*iterSecurityId);
		if(pS==NULL)
		{
			continue;
		}

		iBlockIndexId = *iterSecurityId;
		switch (iBlockIndexId)
		{
		case 4000209://沪A
			{
				GetLeafItems(_T("ha_stock"),vecSamples);
			}
			break;
		case 4000008://深A
			{
				GetLeafItems(_T("sa_stock"),vecSamples);
			}
			break;
		default:
			{

				int iIndexSamplesCount = pS->GetIndexConstituentDataCount();
				for (int i = 0; i < iIndexSamplesCount; i++)
				{
					Tx::Data::IndexConstituentData *pIndexConstituentData = pS->GetIndexConstituentDataByIndex(i);
					if (NULL != pIndexConstituentData)
					{
						vecSamples.push_back(pIndexConstituentData->iSecurityId);
					}
				}
			}
			break;
		}

		GetAmountFlow(vecSamples,istart_date,iend_date,tempTable,bAddSamplesOnly,true,false,iMarketid);

		strBlockName = pS->GetName();
		int iSamplesCount = vecSamples.size();

		double dRaise = 0;
		if(bDurRaise==true)
		{
			if(durRaiseTable.GetRowCount()>0)
			{
				UINT nCol = 3;
				UINT nRow = durRaiseTable.FindValueIndex(*iterSecurityId);
				if(Con_uintInvalid!=nRow)
				{
					durRaiseTable.GetCell(nCol,nRow,dRaise);
				}
			}
		}		
		BlockAmountFlowData(tempTable,iInsCol,iDelCol,iDelCodeCol,iSamplesCount, *iterSecurityId,strBlockName, dRaise);
		
		//加入汇总结果
		baTable.AppendTableByRow(tempTable);
	}

	return true;
}
bool TxBusiness::GetAmountFlow(
	int start_date,
	int end_date,
	Table_Display& baTable,
	bool bAddSamplesOnly,
	bool bStat,
	int iMarketid
	)
{
	Tx::Core::ProgressWnd prw;
	//step2
	CString sProgressPrompt;
	sProgressPrompt =  _T("基本项目资金流向...");
	UINT progId = prw.AddItem(1,sProgressPrompt, 0.0);
	//step3
	prw.Show(15);

	prw.SetPercent(progId, 0.1);

	std::vector<int> vecSecurityId;

	//自选股
	vecSecurityId.clear();
	Tx::Core::UserStock::GetInstance()->GetCustomStock(vecSecurityId,6);
	bool bCustomRes = GetAmountFlow(vecSecurityId,start_date,end_date,baTable,bAddSamplesOnly,true,false,iMarketid,false);
	CString sTitle = _T("样本数");;
	int iSamplesCount = vecSecurityId.size();
	int iInsCol = 2;
	int iDelCol = 4;
	int iDelCodeCol = 2;
	//自选股最后一行为汇总
	BlockAmountFlowData(baTable,iInsCol,iDelCol,iDelCodeCol,iSamplesCount,0,_T("默认自选股"), Con_doubleInvalid);
	baTable.SetTitle(iInsCol,sTitle);

	//2008-12-17 
	if(start_date>end_date)
	{
		return false;
	}
	//2009-01-23 自选股有样本，但是处理失败
	if(iSamplesCount>0 && bCustomRes==false)
	{
		return false;
	}

	prw.SetPercent(progId, 0.3);

	//全A
	//vecSecurityId.clear();
	//GetLeafItems(_T("a_stock"),vecSecurityId);
	//Table_Display asbaTable;
	//if(GetAmountFlow(vecSecurityId,start_date,end_date,asbaTable,bAddSamplesOnly,true,true,iMarketid)==false)
	//{
	//	baTable.DeleteRow(0,baTable.GetRowCount());
	//	return false;
	//}
	//iSamplesCount = vecSecurityId.size();
	//BlockAmountFlowData(asbaTable,iInsCol,iDelCol,iDelCodeCol,iSamplesCount,1,_T("全部A股"),Con_doubleInvalid);
	//baTable.AppendTableByRow(asbaTable);	//加入汇总结果

	//prw.SetPercent(progId, 0.4);

	
	Table_Display tempTable;
	vecSecurityId.clear();
	vecSecurityId.push_back(4000363);//全部A股，取得是天相总股
	if(!GetBlockAmountFlow(vecSecurityId,start_date,end_date,tempTable,bAddSamplesOnly,false,true,iMarketid))
	{
		baTable.DeleteRow(0,baTable.GetRowCount());
		return false;
	}
	CString m_str = _T("全部A股");
	tempTable.SetCell(1,0,m_str);
	baTable.AppendTableByRow(tempTable);
	prw.SetPercent(progId, 0.4);
#ifdef _DEBUG
	CString strTable=tempTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	vecSecurityId.clear();	
	vecSecurityId.push_back(4000209);//沪A
	vecSecurityId.push_back(4000008);//深A
	vecSecurityId.push_back(4000006);//中小企业板
	vecSecurityId.push_back(4001079);//天相创业板
	vecSecurityId.push_back(4000223);//沪深300
	if(!GetBlockAmountFlow(vecSecurityId,start_date,end_date,tempTable,bAddSamplesOnly,false,true,iMarketid))
	{
		baTable.DeleteRow(0,baTable.GetRowCount());
		return false;
	}

	baTable.AppendTableByRow(tempTable);

	prw.SetPercent(progId, 0.5);

	vecSecurityId.clear();
	tempTable.Clear();
	GetLeafItems(_T("tx_industry_index"),vecSecurityId);
	if(!GetBlockAmountFlow(vecSecurityId,start_date,end_date,tempTable,bAddSamplesOnly,false,true,iMarketid))
	{
		baTable.DeleteRow(0,baTable.GetRowCount());
		return false;
	}

	baTable.AppendTableByRow(tempTable);

	prw.SetPercent(progId, 1.0);
	sProgressPrompt += _T(",完成!");
	prw.SetText(progId,sProgressPrompt);

	return true;
}
//2008-10-29
//取得指数实时资金流入流出
bool TxBusiness::GetAmountFlow(int iIndexId,double& dAmountIn,double& dAmountOut)
{
	SecurityQuotation* pSecurityQuotation = GetSecurityNow(iIndexId);
	if(pSecurityQuotation==NULL)
		return false;
	if(pSecurityQuotation->IsIndex()==false || pSecurityQuotation->IsHK_Market()==true)
		return false;

	bool bRes = false;
	if(DataStatus::GetInstance()->IsBefore(pSecurityQuotation->GetInnerBourseId())==false)
	{
		//2009-02-23
		//集合竞价时不再赋值
		std::vector<int> vecSecurityId;
		//指数样本个数
		int iCount =
		pSecurityQuotation->GetIndexConstituentDataCount();
		for(int i=0;i<iCount;i++)
		{
			IndexConstituentData* pIndexConstituentData = pSecurityQuotation->GetIndexConstituentDataByIndex(i);
			if(pIndexConstituentData==NULL) continue;
			vecSecurityId.push_back(pIndexConstituentData->iSecurityId);
		}
		//取得指数样本
		bRes = GetAmountFlow(vecSecurityId,dAmountIn,dAmountOut);
		pSecurityQuotation->SetAmountIO(dAmountIn,true);
		pSecurityQuotation->SetAmountIO(dAmountOut,false);
	}
	//else
	//{
	//	CString sLog;
	//	sLog.Format(_T("[%d-%d][%d]特殊时间段，不处理资金流向数据[%s-%s-%d][data:%d-%d]"),
	//		pSecurityQuotation->GetServerCurDateTime().GetDate().GetInt(),
	//		pSecurityQuotation->GetServerCurDateTime().GetTime().GetInt(),
	//		pSecurityQuotation->GetInnerBourseId(),
	//		pSecurityQuotation->GetName(),
	//		pSecurityQuotation->GetCode(true),
	//		pSecurityQuotation->GetId(),
	//		pSecurityQuotation->GetCurDataDate(),
	//		pSecurityQuotation->GetCurDataTime()
	//		);
	//	Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
	//}
	/*CString str = _T("");
	str.Format(_T("指数id：%d 流入:%.02f 流出 :%.02f\r\n"),iIndexId,dAmountIn,dAmountOut);
	TRACE(str);*/
	return bRes;
}

//实时资金流入流出求和
bool TxBusiness::GetAmountFlow(std::vector<int>& vecSecurityId,double& dAmountIn,double& dAmountOut)
{
	dAmountIn = 0;
	dAmountOut = 0;
	int iSecurityCount = (int)vecSecurityId.size();
	//循环样本
	for(int i=0;i<iSecurityCount;i++)
	{
		double dValuein = Con_doubleInvalid;
		double dValueout = Con_doubleInvalid;
		//样本id
		int iSecurityId = vecSecurityId[i];
		//样本对象
		SecurityQuotation* pSecurityQuotation = GetSecurityNow(iSecurityId);
		if(pSecurityQuotation==NULL)
		{
#ifdef _DEBUG
			GlobalWatch::_GetInstance()->WatchHere(_T("zhaohj|| TxBusiness::GetAmountFlow invalid id = %d"),iSecurityId);
#endif
			continue;
		}
		if(pSecurityQuotation->IsHK_Market()==true)
			continue;

		//实时数据
		dValuein = pSecurityQuotation->GetBuyAmount();
		dValueout = pSecurityQuotation->GetSaleAmount();
		/*CString str = _T("");
		str.Format(_T("样本id：%d 流入:%.02f 流出 :%.02f\r\n"),iSecurityId,dValuein,dValueout);
		TRACE(str);*/
		if(!(dValuein<0))
			dAmountIn+=dValuein;
		if(!(dValueout<0))
			dAmountOut+=dValueout;
	}
	if(!(dAmountIn>0))
		dAmountIn = Con_doubleInvalid;
	if(!(dAmountOut>0))
		dAmountOut = Con_doubleInvalid;
	return true;
}


//2008-09-05
//历史资金流入流出
bool TxBusiness::GetAmountFlow(
	std::vector<int>& vecSecurityId,
	int istart_date,
	int iend_date,
	Table_Display& baTable,
	bool bAddSamplesOnly,
	bool bStat,
	bool bDurRaise,
	int iMarketid,
	bool bFocusSamples)
{
	baTable.Clear();
	int col = 0;
	int row = 0;
	//交易实体id
	baTable.AddCol(dtype_int4);
	baTable.SetTitle(col++, _T("交易实体id"));

	//名称
	baTable.AddCol(dtype_val_string);
	baTable.SetTitle(col++, _T("名称"));

	//代码
	baTable.AddCol(dtype_val_string);
	baTable.SetTitle(col++, _T("代码"));

	//涨幅
	baTable.AddCol(dtype_double);
	baTable.SetTitle(col++, _T("个股涨幅(%)"));

	//流入/流出
	baTable.AddCol(dtype_double);
	baTable.SetPrecise(col, 3);// cyh, 2009-08-18
	baTable.SetTitle(col++, _T("流入/流出"));

	//净流入
	baTable.AddCol(dtype_double);
	baTable.SetOutputItemRatio(col, 10000);
	baTable.SetFormatStyle(col, Tx::Core::fs_finance);
	baTable.SetTitle(col++, _T("净流入(万元)"));

	//流入
	baTable.AddCol(dtype_double);
	baTable.SetOutputItemRatio(col, 10000);
	baTable.SetFormatStyle(col, Tx::Core::fs_finance);
	baTable.SetTitle(col++, _T("流入(万元)"));

	//流出
	baTable.AddCol(dtype_double);
	baTable.SetOutputItemRatio(col, 10000);
	baTable.SetFormatStyle(col, Tx::Core::fs_finance);
	baTable.SetTitle(col++, _T("流出(万元)"));

	int start_date,end_date;

	//2008-11-11
	if(vecSecurityId.size()<=0)
	{
		if(bFocusSamples == true)
			AfxMessageBox(_T("资金流向：样本不能为空!"));
		return false;
	}
	if(bAddSamplesOnly == true)
	{
		int iSecurityCount = (int)vecSecurityId.size();
		//循环样本
		for(int i=0;i<iSecurityCount;i++)
		{
			//样本id
			int iSecurityId = vecSecurityId[i];
			//样本对象
			SecurityQuotation* pSecurityQuotation = GetSecurityNow(iSecurityId);
			if(pSecurityQuotation==NULL)
				continue;
			if(pSecurityQuotation->IsHK_Market()==true)
				continue;

			row = (int)baTable.GetRowCount();
			col = 0;
			//添加一行记录
			baTable.AddRow();
			//securityid
			baTable.SetCell(col++,row,iSecurityId);
			//name
			baTable.SetCell(col++,row,pSecurityQuotation->GetName());
			//code
			baTable.SetCell(col++,row,pSecurityQuotation->GetCode());
		}
		return true;
	}
	if(istart_date>iend_date)
	{
		AfxMessageBox(_T("起始日期应该小于等于终止日期!"));
		return false;
	}
	//默认为上交所
	if(iMarketid<=0)
	{
		iMarketid = m_pFunctionDataManager->GetBourseId_ShangHai();
	}

	//取得当日日期
	int curDate = m_pFunctionDataManager->GetServerCurDateTime(iMarketid).GetDate().GetInt();
	int curDataDate = m_pFunctionDataManager->GetCurDataDateTime(iMarketid).GetDate().GetInt();
	//取得当日是否交易的标志
	bool bTodayTradeDay = m_pFunctionDataManager->IsTodayTradeDate(iMarketid);

	bool bAddToday = false;
	//处理默认日期
	if(istart_date<=0)
	{
		istart_date = curDate;
		return false;
	}
	if(iend_date<=0)
	{
		iend_date = curDate;
		return false;
	}

	start_date = istart_date;
	end_date = iend_date;

	//是否取得实时数据的标志
	bool bReal = false;
	//起始和终止日期相同且为当日，则取内存中数据,如果当日非交易日，则取上个交易日数据
	if(end_date==curDate)
	{
		//一天
		if(start_date==end_date)
		{
			//if(bTodayTradeDay==true)
			{
				//只处理当日实时数据
				bReal = true;
			}
		}
		if(bTodayTradeDay==true && curDataDate==curDate)
			//终止日是交易日期，所以要考虑增加终止日的数据
			bAddToday = true;
	}
	//2008-09-18
	//先计算阶段涨幅[col=3]
	Tx::Core::Table_Indicator durRaiseTable;
	if(bDurRaise == true && bReal == false)
	{
		if(BlockCycleRateAdv(
			vecSecurityId,	//交易实体ID
			istart_date,		//起始日期
			iend_date,		//终止日期
			true,			//剔除首日涨幅
			//0,				//复权类型 0:不复权;1:前复权;2:后复权;3:全复权+后复权;4:全复权+前复权
			2,//2009-05-11 徐勉：资金流向需要复权，后复权
			durRaiseTable,	//结果数据表
			2,
			0,				//计算类型0-默认；1-债券[bCutFirstDateRaise=true表示全价模式,bCutFirstDateRaise=false表示净价模式]
			iMarketid,		//交易所ID
			true,			//true只计算涨幅,false计算所有
			false
			)==false)
			return false;
		durRaiseTable.MakeReference(0);
	}

	DataFileNormal<blk_TxExFile_FileHead,AmountFlow>* pDataFile1=NULL;
	DataFileNormal<blk_TxExFile_FileHead,AmountFlow>* pDataFile2=NULL;
	//是否使用今日横向数据
	bool bUseNewlyData = false;

	//没有交易数据
	bool bNonTradeData = false;

	int iExFileId=
	Tx::Data::DataStatus::GetInstance()->GetExFileIdFromIni(
		Tx::Core::SystemPath::GetInstance()->GetSystemDataPath(),
		_T("amount_flow_for_date"));

	if(bReal == false)
	{
		//历史数据
		//准备大盘指数
		if(GetHuShenTradeDate()==false)
			return false;
		if(m_pMixTradeDay==NULL)
			return false;
		//时间以大盘为基准
		//样本的资金流入流出数据
		//
		int start_index = m_pMixTradeDay->SearchNearestIndex(start_date);
		int end_index = m_pMixTradeDay->SearchNearestIndex(end_date);

		start_date = m_pMixTradeDay->GetDateByIndex(start_index);
		end_date = m_pMixTradeDay->GetDateByIndex(end_index);

		int imax_index = m_pMixTradeDay->GetTradeDayCount()-1;
		int ilast_date = m_pMixTradeDay->GetDateByIndex(imax_index);
		int ifirst_date = m_pMixTradeDay->GetDateByIndex(0);

		
		if(
			start_index>end_index ||//历史日期，非交易日时间区间,
			(start_index==end_index && start_date<istart_date)||
			iend_date<ifirst_date ||//在大盘上市之前
			(istart_date>ilast_date && iend_date<curDate)	//在大盘最新交易日期之后和当日之间为非交易日区间
		  )
			bNonTradeData = true;
		else if(
			istart_date>ilast_date && iend_date==curDate//在大盘最新交易日期之后和当日之间[含]
			)
		{
			if(bTodayTradeDay==true)//终止日期为当日，当日为交易日
				bReal = true;
			else
				bNonTradeData = true;//终止日期为当日，当日为非交易日
		}
		else
		{
			//确保取得昨天的横向数据
			if(bUseNewlyData==false)
			{
				if(curDate==end_date)
				{
					end_index--;
					end_date = m_pMixTradeDay->GetDateByIndex(end_index);
				}
			}

			if(start_date==end_date)
				start_index = end_index;

			//取得指定阶段内的第一天[交易日]
			//start_date = m_pMixTradeDay->GetDateByIndex(start_index);
			if(start_date<istart_date)
			{
				start_index++;
				if(start_index>imax_index) start_index = imax_index;
				start_date = m_pMixTradeDay->GetDateByIndex(start_index);
			}

			end_date = m_pMixTradeDay->GetDateByIndex(end_index);

			if(end_date>=start_date)
			{
				//起始日期的数据
				pDataFile1 = new DataFileNormal<blk_TxExFile_FileHead,AmountFlow>;
				if(pDataFile1==NULL)
					return false;
				if(pDataFile1->Load(
					start_date,
					iExFileId,
					true
					)==false)
				{
					delete pDataFile1;
					AfxMessageBox(_T("加载起始日期的数据失败!"));
					return false;
				}
				if(pDataFile1->GetDataCount()<=0)
				{
					delete pDataFile1;
					AfxMessageBox(_T("加载起始日期的数据失败[记录数为0]!"));
					return false;
				}
				//终止日期的数据
				if(start_date==end_date)
					//日期相同则不需要加载
					pDataFile2 = pDataFile1;
				else
				{
					pDataFile2 = new DataFileNormal<blk_TxExFile_FileHead,AmountFlow>;
					if(pDataFile2==NULL)
					{
						delete pDataFile1;
						return false;
					}
					if(pDataFile2->Load(
						end_date,
						iExFileId,
						true
						)==false)
					{
						delete pDataFile1;
						delete pDataFile2;
						AfxMessageBox(_T("加载终止日期的数据失败!"));
						return false;
					}

					if(pDataFile2->GetDataCount()<=0)
					{
						delete pDataFile1;
						delete pDataFile2;
						AfxMessageBox(_T("加载终止日期的数据失败[记录数为0]!"));
						return false;
					}
				}
			}
			else
			{
				bNonTradeData = true;
			}
		}
	}
	Tx::Core::ProgressWnd prw;
	//step2
	CString sProgressPrompt;
	sProgressPrompt =  _T("资金流向...");
	UINT progId = prw.AddItem(1,sProgressPrompt, 0.0);
	//step3
	prw.Show(15);

	double dValuein = Con_doubleInvalid;
	double dValueout = Con_doubleInvalid;
	double dValue = Con_doubleInvalid;
	double dRatio = Con_doubleInvalid;
	int iSecurityCount = (int)vecSecurityId.size();
	//循环样本
	for(int i=0;i<iSecurityCount;i++)
	{
		prw.SetPercent(progId, (double)i/(double)iSecurityCount);

		dValuein = Con_doubleInvalid;
		dValueout = Con_doubleInvalid;
		dValue = Con_doubleInvalid;
		dRatio = Con_doubleInvalid;
		//样本id
		int iSecurityId = vecSecurityId[i];
		//样本对象
		SecurityQuotation* pSecurityQuotation = GetSecurityNow(iSecurityId);
		if(pSecurityQuotation==NULL)
		{
#ifdef _DEBUG
			GlobalWatch::_GetInstance()->WatchHere(_T("zhaohj|| TxBusiness::GetAmountFlow invalid id = %d"),iSecurityId);
#endif
			continue;
		}
		if(pSecurityQuotation->IsHK_Market()==true)
			continue;

		row = (int)baTable.GetRowCount();
		col = 0;
		//添加一行记录
		baTable.AddRow();

		//securityid
		baTable.SetCell(col++,row,iSecurityId);
		//name
		baTable.SetCell(col++,row,pSecurityQuotation->GetName());
		//code
		baTable.SetCell(col++,row,pSecurityQuotation->GetCode());

		if(bReal == false)
		{
			 if(bNonTradeData==false)
			 {
				//历史数据
				int iListDate = pSecurityQuotation->GetIPOListedDate();
				//起始日期的数据
				AmountFlow* pAmountFlow = NULL;
				AmountFlow pAmountFlow1,pAmountFlow2;

				memset(&pAmountFlow1,'0',sizeof(AmountFlow));
				memset(&pAmountFlow2,'0',sizeof(AmountFlow));
				pAmountFlow = pDataFile1->GetDataByObj(iSecurityId,false);
				//if(pAmountFlow==NULL || !(pAmountFlow->f_inflow>0 || pAmountFlow->f_outFlow>0))
				if(pAmountFlow==NULL)
				{
					if(iListDate<=0)
						goto DEAL_INVALID_SECURITY;
					DataFileNormal<blk_TxExFile_FileHead,AmountFlow>* pDataFileList = new DataFileNormal<blk_TxExFile_FileHead,AmountFlow>;
					if(pDataFileList==NULL)
						goto DEAL_INVALID_SECURITY;
					if(pDataFileList->Load(
						iListDate,
						iExFileId,
						true
						)==false)
					{
						delete pDataFileList;
						pDataFileList = NULL;
						goto DEAL_INVALID_SECURITY;
					}
					pAmountFlow = pDataFileList->GetDataByObj(iSecurityId,false);
					if(pAmountFlow!=NULL)
						memcpy(&pAmountFlow1,pAmountFlow,sizeof(AmountFlow));
					delete pDataFileList;
					pDataFileList = NULL;
				}
				else
				{
					memcpy(&pAmountFlow1,pAmountFlow,sizeof(AmountFlow));
				}
				//else
				//{
				//	memcpy(&pAmountFlow1,pAmountFlow,sizeof(AmountFlow));
				//}
				if(pAmountFlow1.f_Suminflow>0 || pAmountFlow1.f_SumoutFlow>0)
				{
					//终止日期的数据
					pAmountFlow = pDataFile2->GetDataByObj(iSecurityId,false);
					if(pAmountFlow!=NULL)
					{
						memcpy(&pAmountFlow2,pAmountFlow,sizeof(AmountFlow));

						//inflow
						if(!(pAmountFlow2.f_Suminflow<0 || pAmountFlow1.f_Suminflow<0 || pAmountFlow1.f_inflow<0))
						{
							dValuein = pAmountFlow2.f_Suminflow-pAmountFlow1.f_Suminflow+pAmountFlow1.f_inflow;
							if(bAddToday==true && bUseNewlyData==false)
								dValuein += pSecurityQuotation->GetBuyAmount();
						}
						if(!(pAmountFlow2.f_SumoutFlow<0 || pAmountFlow1.f_SumoutFlow<0 || pAmountFlow1.f_outFlow<0))
						{
							dValueout = pAmountFlow2.f_SumoutFlow-pAmountFlow1.f_SumoutFlow+pAmountFlow1.f_outFlow;
							if(bAddToday==true && bUseNewlyData==false)
								dValueout += pSecurityQuotation->GetSaleAmount();
						}
					}
				}
			 }
			 /*
			CString sLog;
			sLog.Format(_T("[%s-%s-%d]历史[可能包含实时]资金流向数据[%d-%d,in=%.4f,out=%.4f]"),
				pSecurityQuotation->GetName(),
				pSecurityQuotation->GetCode(true),
				pSecurityQuotation->GetId(),
				istart_date,iend_date,
				dValuein,
				dValueout
				);
			Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
			*/
		}
		else
		{
			//实时数据
			dValuein = pSecurityQuotation->GetBuyAmount();
			dValueout = pSecurityQuotation->GetSaleAmount();

			/*
			CString sLog;
			sLog.Format(_T("[%s-%s-%d]实时资金流向数据[%d-%d,in=%.4f,out=%.4f]"),
				pSecurityQuotation->GetName(),
				pSecurityQuotation->GetCode(true),
				pSecurityQuotation->GetId(),
				istart_date,iend_date,
				dValuein,
				dValueout
				);
			Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
			*/
		}
		if(!(dValuein<0 || dValueout<0))
		{
			dValue = dValuein-dValueout;
			if(dValuein>0 && dValueout>0)
				dRatio = dValuein/dValueout;
		}

DEAL_INVALID_SECURITY:
		//raise
		double dRaise = 0;
		if(bReal==true)
			//当日实时
			dRaise = pSecurityQuotation->GetRaise();
		else
		{
			if(bDurRaise==true)
			{
				if(durRaiseTable.GetRowCount()>0)
				{
					UINT nCol = 3;
					UINT nRow = durRaiseTable.FindValueIndex(iSecurityId);
					if(Con_uintInvalid!=nRow)
						durRaiseTable.GetCell(nCol,nRow,dRaise);
				}
			}
		}
		baTable.SetCell(col++,row,dRaise);

		//ratio
		baTable.SetCell(col++,row,dRatio);
		//netflow
		if(fabs(dValue)<0.000001)
			dValue = Con_doubleInvalid;
		baTable.SetCell(col++,row,dValue);
		//inflow
		if(!(dValuein>0))
			dValuein = Con_doubleInvalid;
		baTable.SetCell(col++,row,dValuein);
		//outflow
		if(!(dValueout>0))
			dValueout = Con_doubleInvalid;
		baTable.SetCell(col++,row,dValueout);
	}

	if(bReal == false)
	{
		if(pDataFile1!=NULL)
			delete pDataFile1;
		if(start_date!=end_date)
		{
			if(pDataFile2!=NULL)
				delete pDataFile2;
		}
	}

	//2008-09-16
	//sum
	//
	if(bStat==true && baTable.GetRowCount()>0)
	{
		dValuein = 0;
		dValueout = 0;
		dValue = 0;
		dRatio = 0;
		col = 0;
		double curValue = 0;
		row = (int)baTable.GetRowCount();
		//添加一行记录
		baTable.AddRow();

		//securityid
		baTable.SetCell(col++,row,Con_intInvalid);
		//name
		CString sStat(_T("汇总"));
		baTable.SetCell(col++,row,sStat);
		//code
		baTable.SetCell(col++,row,Con_strInvalid);
		//raise
		//涨幅不需要统计
		baTable.SetCell(col++,row,Con_doubleInvalid);

		//统计
		int stat_col = col;
		for(int i=0;i<(int)baTable.GetRowCount();i++)
		{
			stat_col = col;
			//ratio
			baTable.GetCell(stat_col,i,curValue);
			if(curValue>0)
				dRatio+=curValue;
			stat_col++;
			//netflow
			baTable.GetCell(stat_col,i,curValue);
			if(fabs(curValue-Con_doubleInvalid)>0.000001)
				dValue+=curValue;
			stat_col++;
			//inflow
			baTable.GetCell(stat_col,i,curValue);
			if(curValue>0)
				dValuein+=curValue;
			stat_col++;
			//outflow
			baTable.GetCell(stat_col,i,curValue);
			if(curValue>0)
				dValueout+=curValue;
			stat_col++;
		}

		//ratio
		if(dValuein>0 && dValueout>0)
			baTable.SetCell(col++,row,dValuein/dValueout);
		else
			baTable.SetCell(col++,row,Con_doubleInvalid);

		if(!(dValuein>0)) dValuein = Con_doubleInvalid;
		if(!(dValueout>0)) dValueout = Con_doubleInvalid;
		if(dValueout<0 || dValuein<0) dValue = Con_doubleInvalid;

		curValue = 0;
		//netflow
		baTable.SetCell(col++,row,dValue);
		//inflow
		baTable.SetCell(col++,row,dValuein);
		//outflow
		baTable.SetCell(col++,row,dValueout);
	}
	prw.SetPercent(progId, 1.0);
	sProgressPrompt += _T(",完成!");
	prw.SetText(progId,sProgressPrompt);

	return true;
}
//2008-09-25
//取得指定机构id所属的样本
//机构,券,交易实体映射关系专用接口
//如果bIsSecurityId=true表示样本是交易实体,bIsSecurityId=false表示样本是券
bool	TxBusiness::GetItemsByInstitution(
	int iInstitutionId,			//机构id
	std::vector<int>& items,	//样本
	bool bIsSecurityId	//是否交易实体样本
	)
{
	//机构id
DWORD iNow = GetTickCount();
	//建立券，机构和交易实体的映射关系
	m_pLogicalBusiness->DoSecurityInsituttion();
TRACE(_T("\nDoSecurityInsituttion=%d\n"),GetTickCount()-iNow);

	//取得机构集合
	Collection* pIns = m_pFunctionDataManager->GetCollection(iInstitutionId);
	if(pIns==NULL)
		return false;

	//取得券样本
	std::vector<int> vecSecurity1Id;
	pIns->GetCollectionItems(vecSecurity1Id);
	if(bIsSecurityId==false)
	{
		items.assign(vecSecurity1Id.begin(),vecSecurity1Id.end());
		return true;
	}
	std::set<int> itemsS;
	//pIns->GetItems(vecSecurity1Id);
	//循环券样本，取得所有交易实体id
	for(int i=0;i<(int)vecSecurity1Id.size();i++)
	{
		Collection* pSec1 = m_pFunctionDataManager->GetCollection(vecSecurity1Id[i]);
		if(pSec1!=NULL)
		{
			std::set<int> vecSecurityIds;
			pSec1->GetCollectionItems(vecSecurityIds);
			itemsS.insert(vecSecurityIds.begin(),vecSecurityIds.end());
		}
	}
	items.insert(items.end(),itemsS.begin(),itemsS.end());

	return true;
}
	//取得指定券id所属的交易实体样本
	//券,交易实体映射关系专用接口
bool	TxBusiness::GetItemsBySecurity(
		int iSecurity1Id,			//券id
		std::vector<int>& items	//样本
		)
{
	//机构id
DWORD iNow = GetTickCount();
	//建立券，机构和交易实体的映射关系
	m_pLogicalBusiness->DoSecurityInsituttion();
TRACE(_T("\nDoSecurityInsituttion=%d\n"),GetTickCount()-iNow);

	//取得机构集合
	Collection* pC = m_pFunctionDataManager->GetCollection(iSecurity1Id);
	if(pC==NULL)
		return false;

	//取得券样本
	pC->GetCollectionItems(items);
	return true;
}
//2008-10-17
//代销机构
bool TxBusiness::GetSecurityBySaleAgency(int iSaleAgencyId,std::vector<int>& items)
{
	return m_pFunctionDataManager->GetSecurityBySaleAgency(iSaleAgencyId,items);
}


// added by zhoup 2008.12.01
void TxBusiness::InistitutionFilter(std::vector<int>& vecSecurity)
{
	if (vecSecurity.empty())
		return;
	std::unordered_map<int,int> mapInstToTransObj;
	for (std::vector<int>::iterator iter = vecSecurity.begin();iter != vecSecurity.end();iter++)
	{
		int iSecurityId= *iter;
		SecurityQuotation* ps = (SecurityQuotation*)GetSecurity(iSecurityId);
		if (ps == NULL || ps->GetInstitutionId() < 0)
		{
			// 从vector中剔除
			ASSERT(FALSE);
			iter = vecSecurity.erase(iter);
			continue;
		}
		std::pair<std::unordered_map<int,int>::iterator,bool> instPair = mapInstToTransObj.insert(std::make_pair(ps->GetInstitutionId(),iSecurityId));
		if (instPair.second == false)
		{
			// 已经存在,first是已经存在的位置
			// 若已经存在的是B股,从vecSecurity中剔除已存在的B股
			SecurityQuotation* p = (SecurityQuotation*)GetSecurity(instPair.first->second);
			// 理论上之前插入过,p应该肯定不为空
			if (p != NULL)
			{
				if (p->IsStockB())
				{
					// 删除,both vector&map,插入A股到map,好像也可以不插,如果样本保证没有重复的话
					std::vector<int>::iterator iterfind = std::find(vecSecurity.begin(),vecSecurity.end(),instPair.first->second);
					if (iterfind != vecSecurity.end())
						iter = vecSecurity.erase(iterfind);
					else
						iter++;
					mapInstToTransObj.erase(instPair.first);
					mapInstToTransObj.insert(std::make_pair(ps->GetInstitutionId(),iSecurityId));
				}
				else if (p->IsStockA())
				{
					// 若已经存在的是A股,从vecSecurity中剔除当前
					// 删除 from vector
					iter = vecSecurity.erase(iter);
				}
				else
				{
					// 删除,both vector&map
					iter = vecSecurity.erase(iter);
					mapInstToTransObj.erase(instPair.first);
				}
			}
			else
			{
				ASSERT(FALSE);
				// 删除,both vector&map
				iter = vecSecurity.erase(iter);
				mapInstToTransObj.erase(instPair.first);
			}
		}
		else
			iter++;
	}
}

// added by zhoup 2008.12.01
void TxBusiness::InistitutionFilter(std::set<int>& setSecurity)
{
	if (setSecurity.empty())
		return;
	std::unordered_map<int,int> mapInstToTransObj;
	for (std::set<int>::iterator iter = setSecurity.begin();iter != setSecurity.end();)
	{
		int iSecurityId = *iter;
		SecurityQuotation* ps = (SecurityQuotation*)GetSecurity(iSecurityId);
		if (ps == NULL || ps->GetInstitutionId() < 0)
		{
			// 从vector中剔除
			ASSERT(FALSE);
			iter = setSecurity.erase(iter);
			continue;
		}
		std::pair<std::unordered_map<int,int>::iterator,bool> instPair = mapInstToTransObj.insert(std::make_pair(ps->GetInstitutionId(),iSecurityId));
		if (instPair.second == false)
		{
			// 已经存在,first是已经存在的位置
			// 若已经存在的是B股,从vecSecurity中剔除已存在的B股
			SecurityQuotation* p = (SecurityQuotation*)GetSecurity(instPair.first->second);
			// 理论上之前插入过,p应该肯定不为空
			if (p != NULL)
			{
				if (p->IsStockB())
				{
					// 删除,both vector&map,插入A股到map,好像也可以不插,如果样本保证没有重复的话
					std::set<int>::iterator iterfind = std::find(setSecurity.begin(),setSecurity.end(),instPair.first->second);
					if (iterfind != setSecurity.end())
						iter = setSecurity.erase(iterfind);
					else
						iter++;
					mapInstToTransObj.erase(instPair.first);
					mapInstToTransObj.insert(std::make_pair(ps->GetInstitutionId(),iSecurityId));
				}
				else if (p->IsStockA())
				{
					// 若已经存在的是A股,从vecSecurity中剔除当前
					// 删除 from vector
					iter = setSecurity.erase(iter);
				}
				else
				{
					// 删除,both vector&map
					iter = setSecurity.erase(iter);
					mapInstToTransObj.erase(instPair.first);
				}
			}
			else
			{
				ASSERT(FALSE);
				// 删除,both vector&map
				iter = setSecurity.erase(iter);
				mapInstToTransObj.erase(instPair.first);
			}
		}
		else
			iter++;
	}
}
//2009-03-11
//过滤样本
//ST、摘牌、长期停牌、已发行未上市
bool TxBusiness::SelectItems(
	std::vector<int>& src,
	std::vector<int>& dst,
	bool bSt,
	bool bStop,
	bool bHalt,
	bool bIssued
	)
{
	int count = src.size();
	for(int i=0;i<count;i++)
	{
		int id = src[i];
		SecurityQuotation* p = GetSecurityNow(id);
		if(SelectItems(p,bSt,bStop,bHalt,bIssued)==true)
			dst.push_back(id);
	}
	return true;
}
bool TxBusiness::SelectItems(
	SecurityQuotation* p,
	bool bSt,		//ST
	bool bStop,		//摘牌
	bool bHalt,		//长期停牌
	bool bIssued	//已发行未上市
	)
{
	if(p!=NULL)
	{
		if(bStop==false)
		{
			if(p->IsStop()==true)
				return false;
		}
		if(bHalt==false)
		{
			if(p->IsHaltLong()==true)
				return false;
		}
		if(bIssued==false)
		{
			if(p->IsIssued()==true)
				return false;
		}
		if(bSt==false)
		{
			if(p->IsST()==true)
				return false;
		}
		return true;
	}
	return false;
}

bool TxBusiness::SelectItems(
	std::set<int>& src,
	std::set<int>& dst,
	bool bSt,		//ST
	bool bStop,		//摘牌
	bool bHalt,		//长期停牌
	bool bIssued	//已发行未上市
	)
{
	for(std::set<int>::iterator iter = src.begin();iter!=src.end();iter++)
	{
		int id = *iter;
		SecurityQuotation* p = GetSecurityNow(id);
		if(SelectItems(p,bSt,bStop,bHalt,bIssued)==true)
			dst.insert(id);
	}
	return true;
}
bool TxBusiness::SelectItems(
	std::vector<int>& src,
	std::set<int>& dst,
	bool bSt,		//ST
	bool bStop,		//摘牌
	bool bHalt,		//长期停牌
	bool bIssued	//已发行未上市
	)
{
	for(std::vector<int>::iterator iter = src.begin();iter!=src.end();iter++)
	{
		int id = *iter;
		SecurityQuotation* p = GetSecurityNow(id);
		if(SelectItems(p,bSt,bStop,bHalt,bIssued)==true)
			dst.insert(id);
	}
	return true;
}
bool TxBusiness::SelectItems(
	std::set<int>& src,
	std::vector<int>& dst,
	bool bSt,		//ST
	bool bStop,		//摘牌
	bool bHalt,		//长期停牌
	bool bIssued	//已发行未上市
	)
{
	for(std::set<int>::iterator iter = src.begin();iter!=src.end();iter++)
	{
		int id = *iter;
		SecurityQuotation* p = GetSecurityNow(id);
		if(SelectItems(p,bSt,bStop,bHalt,bIssued)==true)
			dst.push_back(id);
	}
	return true;
}
//added by zhangxs 指数交易数据Table
bool TxBusiness::SetIndexTableCol(Table_Display& baTable)
{
	//如果UI没有输入列信息，自行添加
	if(baTable.GetColCount()<=0)
	{
		int nCol=0;

		//交易日期
		baTable.AddCol(Tx::Core::dtype_int4);
		baTable.SetFormatStyle(nCol,fs_date);
		nCol++;
		//涨幅
		baTable.AddCol(Tx::Core::dtype_double);
		nCol++;
		//涨跌
		baTable.AddCol(Tx::Core::dtype_double);
		nCol++;
		//前收
		baTable.AddCol(Tx::Core::dtype_float);
		nCol++;
		//开盘
		baTable.AddCol(Tx::Core::dtype_float);
		nCol++;
		//最高
		baTable.AddCol(Tx::Core::dtype_float);
		nCol++;
		//最低
		baTable.AddCol(Tx::Core::dtype_float);
		nCol++;
		//收盘
		baTable.AddCol(Tx::Core::dtype_float);
		nCol++;
		//成交量
		baTable.AddCol(Tx::Core::dtype_double);
		nCol++;
		//成交金额
		baTable.AddCol(Tx::Core::dtype_double);
		nCol++;

		nCol=0;
		//baTable.SetTitle(nCol, _T("名称"));
		//++nCol;
		baTable.SetTitle(nCol, _T("日期"));
		++nCol;
		baTable.SetTitle(nCol, _T("涨幅(%)"));
		baTable.SetPrecise(nCol, 2); //2
		++nCol;
		baTable.SetTitle(nCol, _T("涨跌"));
		baTable.SetPrecise(nCol, 2); //2
		++nCol;		
		baTable.SetTitle(nCol, _T("前收"));
		baTable.SetPrecise(nCol, 2); //2
		++nCol;
		baTable.SetTitle(nCol, _T("开盘"));
		baTable.SetPrecise(nCol, 2); //2
		++nCol;
		baTable.SetTitle(nCol, _T("最高"));
		baTable.SetPrecise(nCol, 2); //2
		++nCol;
		baTable.SetTitle(nCol, _T("最低"));
		baTable.SetPrecise(nCol, 2); //2
		++nCol;
		baTable.SetTitle(nCol, _T("收盘"));
		baTable.SetPrecise(nCol, 2);//4); //2   //收盘价改为2位  bug:11640  2012-09-25
		++nCol;
		baTable.SetTitle(nCol, _T("成交量(股)"));
		baTable.SetPrecise(nCol, 0);
		//baTable.SetOutputItemRatio(nCol, 100);
		baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
		++nCol;
		baTable.SetTitle(nCol, _T("成交金额(万)"));
		baTable.SetPrecise(nCol, 2); //2
		baTable.SetOutputItemRatio(nCol, 10000);
		baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
		++nCol;
	}
	return true;
}
//adsded by zhangxs 指数交易数据
bool TxBusiness::SetIndexTableData(Table_Display& baTable,
					   SecurityQuotation* pSecurity,
					   std::vector<HisTradeData> m_vecHisTrade,
					   int iStartdate,
					   int iEndDate,
					   int iFlag,
					   bool bFirst,
					   int iWeight)
{
	int m_iRowcount = (int)m_vecHisTrade.size();
	if(m_iRowcount < 1)
		return false;
	for(int nRow = 0;nRow<m_iRowcount;nRow++)
	{
		baTable.AddRow();
		int nCol = 0;
		HisTradeData pHisTradeData = m_vecHisTrade[nRow];
		double dValue = Con_doubleInvalid;
		double dValueRaise = Con_doubleInvalid;
		float fOpen,fHigh,fLow,fClose,fPreClose;
		double dVolume,dAmount;
		float fAvgPrice = Con_floatInvalid;

		int iDataDate = pHisTradeData.Date;

		fOpen = pHisTradeData.Open;
		fClose = pHisTradeData.Close;
		fHigh = pHisTradeData.High;
		fLow = pHisTradeData.Low;
		fPreClose = pHisTradeData.Preclose;
		dVolume = pHisTradeData.Volume;
		dAmount = pHisTradeData.Amount;
		if(dVolume > 0.01)
			fAvgPrice = (float)(dAmount/dVolume);

		//iWeight == -1 表示不复权
		//0-前复权
		//1-后复权
		//2-全复权前复权
		//3-全复权后复权
		if(iWeight!=-1)
		{
			int iFirstDate = 0;
			HisTradeData* pFirstHisTradeData = pSecurity->GetTradeDataByIndex(0,iFlag,bFirst);
			if(pFirstHisTradeData!=NULL)
				iFirstDate = pFirstHisTradeData->Date;

			if(iWeight == 0)
			{
				double dScale = pSecurity->GetExdividendScale(iDataDate,iEndDate,true);
				if(fClose>0)
					fClose = (float)(fClose*dScale);
				if(fOpen>0)
					fOpen = (float)(fOpen*dScale);
				if(fHigh>0)
					fHigh = (float)(fHigh*dScale);
				if(fLow>0)
					fLow = (float)(fLow*dScale);
				if(fPreClose>0)
					fPreClose = (float)(fPreClose*dScale);
				if(fAvgPrice >0)
					fAvgPrice = (float)(fAvgPrice *dScale);
			}
			else if(iWeight == 1)
			{
				double dScale = pSecurity->GetExdividendScale(iStartdate,iDataDate,false);
				if(fClose>0)
					fClose = (float)(fClose*dScale);
				if(fOpen>0)
					fOpen = (float)(fOpen*dScale);
				if(fHigh>0)
					fHigh = (float)(fHigh*dScale);
				if(fLow>0)
					fLow = (float)(fLow*dScale);
				if(fPreClose>0)
					fPreClose = (float)(fPreClose*dScale);
				if(fAvgPrice >0)
					fAvgPrice = (float)(fAvgPrice *dScale);
			}
			else if(iWeight == 2)
			{
				double dScale = pSecurity->GetExdividendScale(iDataDate,21001231,true);
				if(fClose>0)
					fClose = (float)(fClose*dScale);
				if(fOpen>0)
					fOpen = (float)(fOpen*dScale);
				if(fHigh>0)
					fHigh = (float)(fHigh*dScale);
				if(fLow>0)
					fLow = (float)(fLow*dScale);
				if(fPreClose>0)
					fPreClose = (float)(fPreClose*dScale);
				if(fAvgPrice >0)
					fAvgPrice = (float)(fAvgPrice *dScale);
			}
			else if(iWeight == 3)
			{
				double dScale = pSecurity->GetExdividendScale(19791201,iDataDate,false);
				if(fClose>0)
					fClose = (float)(fClose*dScale);
				if(fOpen>0)
					fOpen = (float)(fOpen*dScale);
				if(fHigh>0)
					fHigh = (float)(fHigh*dScale);
				if(fLow>0)
					fLow = (float)(fLow*dScale);
				if(fPreClose>0)
					fPreClose = (float)(fPreClose*dScale);
				if(fAvgPrice >0)
					fAvgPrice = (float)(fAvgPrice *dScale);
			}
		}
		if(fClose<0 || fPreClose<0)
			dValue = Con_doubleInvalid;
		else
			dValue = fClose - fPreClose;
		int m_iTabCount = baTable.GetRowCount();
		//日期
		baTable.SetCell(nCol,nRow,iDataDate);
		nCol++;

		//涨幅
		dValueRaise = dValue;
		if( fabs(dValueRaise-Con_doubleInvalid)>0.000001 && fPreClose>0)
		{
			dValueRaise /= fPreClose;
			dValueRaise *= 100;
		}
		baTable.SetCell(nCol,nRow,dValueRaise);
		nCol++;

		//涨跌
		baTable.SetCell(nCol,nRow,dValue);
		nCol++;

		//前收
		baTable.SetCell(nCol,nRow,fPreClose);
		nCol++;

		//开盘
		baTable.SetCell(nCol,nRow,fOpen);
		nCol++;

		//最高
		baTable.SetCell(nCol,nRow,fHigh);
		nCol++;

		//最低
		baTable.SetCell(nCol,nRow,fLow);
		nCol++;

		//收盘
		baTable.SetCell(nCol,nRow,fClose);
		nCol++;


		//成交量
		if(dVolume < 0.001)
			dVolume = Con_doubleInvalid;
		baTable.SetCell(nCol,nRow,dVolume);
		nCol++;
		//成交金额
		if(dAmount < 0.001)
			dAmount = Con_doubleInvalid;
		baTable.SetCell(nCol,nRow,dAmount);
		nCol++;
	}
	return true;
}

//指数交易数据 20090326
bool TxBusiness::GetIndexTradeData(Table_Display& baTable,
								   int iSecurityId,
								   int iStartdate,
								   int iEndDate,
								   int iFlag,
								   bool bFirst,
								   int iWeight,
								   bool bNeedProgressWnd)
{
	baTable.Clear();
	//step1 处理结果数据的标题列
	SetIndexTableCol(baTable);

	SecurityQuotation* psq = GetSecurityNow(iSecurityId);
	if(psq==NULL)
		return false;

	int iCol = 0;
	if(baTable.GetColCount()<=0)
		return false;

	//step2
	//控制是否需要进度条
	Tx::Core::ProgressWnd prw;
	CString sProgressPrompt;
	UINT progId = 0;
	if(bNeedProgressWnd==true)
	{
		//支持取消
		prw.EnableCancelButton(true);
		sProgressPrompt.Format(_T("交易数据..."));
		progId = prw.AddItem(1,sProgressPrompt, 0.0);
		prw.Show(15);
	}

	//step3 取得指定交易所的当前日期
	//int idate = 0;
	int i =0;

	int row= baTable.GetRowCount();
	int col = baTable.GetColCount();

	bool bNeedCheckToday = false;
	bool bNeedAppendToday = false;
	CalStatusAboutToday(psq,iEndDate,bNeedCheckToday,bNeedAppendToday);

	//step4 循环日期
	int count = psq->GetTradeDataCount();
	//
	//int count = psq->GetTradeDateCount(iFlag);
	if(iWeight!=-1)
	{
		//只有沪深股票，基金和债券要处理复权
		if(
			psq->IsIndex() == true ||
			psq->IsWarrant() == true ||
			(psq->IsStock() == true && psq->IsHK_Market()==true) ||
			psq->IsFutures() == true
			)
			iWeight = -1;
	}
	std::vector<HisTradeData> m_vecHisTrade;
	for (i=0;i<count;i++)
	{
		if(bNeedProgressWnd==true)
		{
			prw.SetPercent(progId, (double)i/(double)count);
			if(prw.IsCanceled()==true)
				break;
		}

		//step 5.1
		//第一列为样本ID
		//取得样本的ID

		//step5.2 取得样本的Security指针
		//baTable.AddRow();
		//step5.3 处理基本指标数据
		int j = 0;
		//SetStockTableData(baTable,psq,j,i,iStartdate,iEndDate,iFlag,bFirst,iWeight);
		HisTradeData* pHisTradeData = psq->GetTradeDataByIndex(i);
		if(pHisTradeData==NULL)
			continue;
		m_vecHisTrade.push_back(*pHisTradeData);

		if(bNeedCheckToday && iEndDate == pHisTradeData->Date)
			bNeedAppendToday = false;
	}
	//刘鹏，2012-04-06，加入实时行情的数据.需求：10415
	PushBackToday(bNeedAppendToday,psq,m_vecHisTrade);

	GetCQDate(m_vecHisTrade,psq,iStartdate,iEndDate,iWeight);
	std::vector<HisTradeData> m_vecHisTradeTerm;
	TransitionData(m_vecHisTrade,m_vecHisTradeTerm,psq,iStartdate,iEndDate,iFlag);
	SetIndexTableData(baTable,psq,m_vecHisTradeTerm,iStartdate,iEndDate,iFlag,bFirst,iWeight);

	//step6 循环完成
	if(bNeedProgressWnd==true)
	{
		prw.SetPercent(progId, 1.0);
		sProgressPrompt+=_T(",完成!");
		prw.SetText(progId, sProgressPrompt);
	}
	return true;
}
//权证及港股交易数据
bool TxBusiness::SetNormalTableCol(Table_Display& baTable,SecurityQuotation* pSecurity)
{
	if(pSecurity == NULL)
		return false;
	//如果UI没有输入列信息，自行添加
	if(baTable.GetColCount()<=0)
	{
		int nCol=0;

		//交易日期
		baTable.AddCol(Tx::Core::dtype_int4);
		baTable.SetFormatStyle(nCol,fs_date);
		nCol++;
		//涨幅
		baTable.AddCol(Tx::Core::dtype_double);
		nCol++;
		//涨跌
		baTable.AddCol(Tx::Core::dtype_double);
		nCol++;
		//前收
		baTable.AddCol(Tx::Core::dtype_float);
		nCol++;
		//开盘
		baTable.AddCol(Tx::Core::dtype_float);
		nCol++;
		//最高
		baTable.AddCol(Tx::Core::dtype_float);
		nCol++;
		//最低
		baTable.AddCol(Tx::Core::dtype_float);
		nCol++;
		//收盘
		baTable.AddCol(Tx::Core::dtype_float);
		nCol++;
		//均价
		baTable.AddCol(Tx::Core::dtype_float);
		nCol++;
		//成交量
		baTable.AddCol(Tx::Core::dtype_double);
		nCol++;
		//成交金额
		baTable.AddCol(Tx::Core::dtype_double);
		nCol++;
		if(pSecurity->IsWarrant())
		{
			nCol=0;
			//baTable.SetTitle(nCol, _T("名称"));
			//++nCol;
			baTable.SetTitle(nCol, _T("日期"));
			++nCol;
			baTable.SetTitle(nCol, _T("涨幅(%)"));
			baTable.SetPrecise(nCol, 3); //2
			++nCol;
			baTable.SetTitle(nCol, _T("涨跌"));
			baTable.SetPrecise(nCol, 3); //2
			++nCol;		
			baTable.SetTitle(nCol, _T("前收"));
			baTable.SetPrecise(nCol, 3); //2
			++nCol;
			baTable.SetTitle(nCol, _T("开盘"));
			baTable.SetPrecise(nCol, 3); //2
			++nCol;
			baTable.SetTitle(nCol, _T("最高"));
			baTable.SetPrecise(nCol, 3); //2
			++nCol;
			baTable.SetTitle(nCol, _T("最低"));
			baTable.SetPrecise(nCol, 3); //2
			++nCol;
			baTable.SetTitle(nCol, _T("收盘"));
			baTable.SetPrecise(nCol, 3); //2
			++nCol;
			baTable.SetTitle(nCol, _T("均价"));
			baTable.SetPrecise(nCol, 3); //2
			++nCol;
			baTable.SetTitle(nCol, _T("成交量(股)"));
			baTable.SetPrecise(nCol, 0);
			//baTable.SetOutputItemRatio(nCol, 100);
			baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
			++nCol;
			baTable.SetTitle(nCol, _T("成交金额(万)"));
			baTable.SetPrecise(nCol, 3); //2
			baTable.SetOutputItemRatio(nCol, 10000);
			baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
			++nCol;
		}
		else
		{
			nCol=0;
			//baTable.SetTitle(nCol, _T("名称"));
			//++nCol;
			baTable.SetTitle(nCol, _T("日期"));
			++nCol;
			baTable.SetTitle(nCol, _T("涨幅(%)"));
			baTable.SetPrecise(nCol, 2); //2
			++nCol;
			baTable.SetTitle(nCol, _T("涨跌"));
			baTable.SetPrecise(nCol, 2); //2
			++nCol;		
			baTable.SetTitle(nCol, _T("前收"));
			baTable.SetPrecise(nCol, 2); //2
			++nCol;
			baTable.SetTitle(nCol, _T("开盘"));
			baTable.SetPrecise(nCol, 2); //2
			++nCol;
			baTable.SetTitle(nCol, _T("最高"));
			baTable.SetPrecise(nCol, 2); //2
			++nCol;
			baTable.SetTitle(nCol, _T("最低"));
			baTable.SetPrecise(nCol, 2); //2
			++nCol;
			baTable.SetTitle(nCol, _T("收盘"));
			baTable.SetPrecise(nCol, 2); //2
			++nCol;
			baTable.SetTitle(nCol, _T("均价"));
			baTable.SetPrecise(nCol, 2); //2
			++nCol;
			baTable.SetTitle(nCol, _T("成交量(股)"));
			baTable.SetPrecise(nCol, 0);
			//baTable.SetOutputItemRatio(nCol, 100);
			baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
			++nCol;
			baTable.SetTitle(nCol, _T("成交金额(万)"));
			baTable.SetPrecise(nCol, 2); //2
			baTable.SetOutputItemRatio(nCol, 10000);
			baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
			++nCol;
		}
		
	}
	return true;
}
//adsded by zhangxs //权证及港股交易数据
bool TxBusiness::SetNormalTableData(Table_Display& baTable,
						SecurityQuotation* pSecurity,
						std::vector<HisTradeData> m_vecHisTrade,
						int iStartdate,
						int iEndDate,
						int iFlag,
						bool bFirst,
						int iWeight)
{
	int m_iRowcount = (int)m_vecHisTrade.size();
	if(m_iRowcount < 1)
		return false;
	for(int nRow = 0;nRow<m_iRowcount;nRow++)
	{
		if((int)baTable.GetRowCount() >= m_iRowcount)
			continue;
		baTable.AddRow();
		int nCol = 0;
		HisTradeData pHisTradeData = m_vecHisTrade[nRow];
		double dValue = Con_doubleInvalid;
		double dValueRaise = Con_doubleInvalid;
		float fOpen,fHigh,fLow,fClose,fPreClose;
		double dVolume,dAmount;
		float fAvgPrice =  Con_floatInvalid;
		int iDataDate = pHisTradeData.Date;
		fOpen = pHisTradeData.Open;
		fClose = pHisTradeData.Close;
		fHigh = pHisTradeData.High;
		fLow = pHisTradeData.Low;
		fPreClose = pHisTradeData.Preclose;
		dVolume = pHisTradeData.Volume;
		dAmount = pHisTradeData.Amount;
		if(dVolume > 0.01)
			fAvgPrice = (float)(dAmount/dVolume);

		//iWeight == -1 表示不复权
		//0-前复权
		//1-后复权
		//2-全复权前复权
		//3-全复权后复权
		if(iWeight!=-1)
		{
			int iFirstDate = 0;
			HisTradeData* pFirstHisTradeData = pSecurity->GetTradeDataByIndex(0,iFlag,bFirst);
			if(pFirstHisTradeData!=NULL)
				iFirstDate = pFirstHisTradeData->Date;

			if(iWeight == 0)
			{
				double dScale = pSecurity->GetExdividendScale(iDataDate,iEndDate,true);
				if(fClose>0)
					fClose = (float)(fClose*dScale);
				if(fOpen>0)
					fOpen = (float)(fOpen*dScale);
				if(fHigh>0)
					fHigh = (float)(fHigh*dScale);
				if(fLow>0)
					fLow = (float)(fLow*dScale);
				if(fPreClose>0)
					fPreClose = (float)(fPreClose*dScale);
				if(fAvgPrice >0)
					fAvgPrice = (float)(fAvgPrice *dScale);
			}
			else if(iWeight == 1)
			{
				double dScale = pSecurity->GetExdividendScale(iStartdate,iDataDate,false);
				if(fClose>0)
					fClose = (float)(fClose*dScale);
				if(fOpen>0)
					fOpen = (float)(fOpen*dScale);
				if(fHigh>0)
					fHigh = (float)(fHigh*dScale);
				if(fLow>0)
					fLow = (float)(fLow*dScale);
				if(fPreClose>0)
					fPreClose = (float)(fPreClose*dScale);
				if(fAvgPrice >0)
					fAvgPrice = (float)(fAvgPrice *dScale);
			}
			else if(iWeight == 2)
			{
				double dScale = pSecurity->GetExdividendScale(iDataDate,21001231,true);
				if(fClose>0)
					fClose = (float)(fClose*dScale);
				if(fOpen>0)
					fOpen = (float)(fOpen*dScale);
				if(fHigh>0)
					fHigh = (float)(fHigh*dScale);
				if(fLow>0)
					fLow = (float)(fLow*dScale);
				if(fPreClose>0)
					fPreClose = (float)(fPreClose*dScale);
				if(fAvgPrice >0)
					fAvgPrice = (float)(fAvgPrice *dScale);
			}
			else if(iWeight == 3)
			{
				double dScale = pSecurity->GetExdividendScale(19791201,iDataDate,false);
				if(fClose>0)
					fClose = (float)(fClose*dScale);
				if(fOpen>0)
					fOpen = (float)(fOpen*dScale);
				if(fHigh>0)
					fHigh = (float)(fHigh*dScale);
				if(fLow>0)
					fLow = (float)(fLow*dScale);
				if(fPreClose>0)
					fPreClose = (float)(fPreClose*dScale);
				if(fAvgPrice >0)
					fAvgPrice = (float)(fAvgPrice *dScale);
			}
		}
		if(fClose<0 || fPreClose<0)
			dValue = Con_doubleInvalid;
		else
			dValue = fClose - fPreClose;
		int m_iTabCount = baTable.GetRowCount();
		//日期
		baTable.SetCell(nCol,nRow,iDataDate);
		nCol++;

		//涨幅
		dValueRaise = dValue;
		if( fabs(dValueRaise-Con_doubleInvalid)>0.000001 && fPreClose>0)
		{
			dValueRaise /= fPreClose;
			dValueRaise *= 100;
		}
		baTable.SetCell(nCol,nRow,dValueRaise);
		nCol++;

		//涨跌
		baTable.SetCell(nCol,nRow,dValue);
		nCol++;

		//前收
		baTable.SetCell(nCol,nRow,fPreClose);
		nCol++;

		//开盘
		baTable.SetCell(nCol,nRow,fOpen);
		nCol++;

		//最高
		baTable.SetCell(nCol,nRow,fHigh);
		nCol++;

		//最低
		baTable.SetCell(nCol,nRow,fLow);
		nCol++;

		//收盘
		baTable.SetCell(nCol,nRow,fClose);
		nCol++;


		//均价
		if( fAvgPrice < 0.000001 )
		{
			fAvgPrice = Con_floatInvalid;
		}
		baTable.SetCell(nCol,nRow,fAvgPrice);
		nCol++;

		//成交量
		if(dVolume < 0.001)
			dVolume = Con_doubleInvalid;
		baTable.SetCell(nCol,nRow,dVolume);
		nCol++;
		//成交金额
		if(dAmount < 0.001)
			dAmount = Con_doubleInvalid;
		baTable.SetCell(nCol,nRow,dAmount);
		nCol++;
	}
	return true;
}

////权证及港股交易数据 20090326
bool TxBusiness::GetNormalTradeData(Table_Display& baTable,
									int iSecurityId,
									int iStartdate,
									int iEndDate,
									int iFlag,
									bool bFirst,
									int iWeight,
									bool bNeedProgressWnd)
{
	baTable.Clear();	

	SecurityQuotation* psq = GetSecurityNow(iSecurityId);
	if(psq==NULL)
		return false;

	//step1 处理结果数据的标题列
	SetNormalTableCol(baTable,psq);

	int iCol = 0;
	if(baTable.GetColCount()<=0)
		return false;

	//step2
	//控制是否需要进度条
	Tx::Core::ProgressWnd prw;
	CString sProgressPrompt;
	UINT progId = 0;
	if(bNeedProgressWnd==true)
	{
		//支持取消
		prw.EnableCancelButton(true);
		sProgressPrompt.Format(_T("交易数据..."));
		progId = prw.AddItem(1,sProgressPrompt, 0.0);
		prw.Show(15);
	}

	//step3 取得指定交易所的当前日期
	//int idate = 0;
	int i =0;

	int row= baTable.GetRowCount();
	int col = baTable.GetColCount();

	bool bNeedCheckToday = false;
	bool bNeedAppendToday = false;
	CalStatusAboutToday(psq,iEndDate,bNeedCheckToday,bNeedAppendToday);

	//step4 循环日期
	int count = psq->GetTradeDataCount();
	//
	//int count = psq->GetTradeDateCount(iFlag);
	if(iWeight!=-1)
	{
		//只有沪深股票，基金和债券要处理复权
		if(
			psq->IsIndex() == true ||
			psq->IsWarrant() == true ||
			(psq->IsStock() == true && psq->IsHK_Market()==true) ||
			psq->IsFutures() == true
			)
			iWeight = -1;
	}
	std::vector<HisTradeData> m_vecHisTrade;
	for (i=0;i<count;i++)
	{
		if(bNeedProgressWnd==true)
		{
			prw.SetPercent(progId, (double)i/(double)count);
			if(prw.IsCanceled()==true)
				break;
		}

		//step 5.1
		//第一列为样本ID
		//取得样本的ID

		//step5.2 取得样本的Security指针
		//baTable.AddRow();
		//step5.3 处理基本指标数据
		int j = 0;
		//SetStockTableData(baTable,psq,j,i,iStartdate,iEndDate,iFlag,bFirst,iWeight);
		HisTradeData* pHisTradeData = psq->GetTradeDataByIndex(i);
		if(pHisTradeData==NULL)
			continue;
		m_vecHisTrade.push_back(*pHisTradeData);

		if(bNeedCheckToday && iEndDate == pHisTradeData->Date)
			bNeedAppendToday = false;
	}
	//刘鹏，2012-04-06，加入实时行情的数据
	PushBackToday(bNeedAppendToday,psq,m_vecHisTrade);

	GetCQDate(m_vecHisTrade,psq,iStartdate,iEndDate,iWeight);
	std::vector<HisTradeData> m_vecHisTradeTerm;
	TransitionData(m_vecHisTrade,m_vecHisTradeTerm,psq,iStartdate,iEndDate,iFlag);
	SetNormalTableData(baTable,psq,m_vecHisTradeTerm,iStartdate,iEndDate,iFlag,bFirst,iWeight);

	//step6 循环完成
	if(bNeedProgressWnd==true)
	{
		prw.SetPercent(progId, 1.0);
		sProgressPrompt+=_T(",完成!");
		prw.SetText(progId, sProgressPrompt);
	}
	return true;
}

//转换不同周期数据
void TxBusiness::TransitionData(std::vector<HisTradeData>& m_vecHisTradeRsc,
								std::vector<HisTradeData> &m_vecHisTradeTem,
								SecurityQuotation* pSecurity,
								int iStartDate,
								int iEndDate,
								long lType)
{
	// 2012-07-09  bug:12354    如果第一条数据为无效数据，去掉第一条
	std::vector<HisTradeData> vecTemp;
	if ((int)m_vecHisTradeRsc.size()>0 && m_vecHisTradeRsc[0].Date < 19800101)
	{
		vecTemp.assign(m_vecHisTradeRsc.begin()+1,m_vecHisTradeRsc.end());
		m_vecHisTradeRsc.clear();
		m_vecHisTradeRsc.assign(vecTemp.begin(),vecTemp.end());
		vecTemp.clear();
	}

	m_vecHisTradeTem.clear();
	long lConut	= m_vecHisTradeRsc.size();
	if(lConut<1)
		return;
	if(pSecurity == NULL)
		return;
	HisTradeData TempK;
	long	MaxDate;
	if(lType <1)
	{
		m_vecHisTradeTem.clear();
		m_vecHisTradeTem.assign(m_vecHisTradeRsc.begin(),m_vecHisTradeRsc.end());
		return;
	}	

	// 2011-02-25 换手率计算，用 HisTreadData 结构的 byte Res[8] 字段传递 Security 指针
	//Tx::Data::SecurityQuotation*& pSecurity = (Tx::Data::SecurityQuotation*&)(m_vecHisTradeRsc.front().Res);
	for(int i=0; i<lConut; i++)
	{
    	long lDate = m_vecHisTradeRsc[i].Date;

		// 2011-02-25 换手率累加 ---- 计算日换手率
		double dVolume = m_vecHisTradeRsc[i].Volume;
		//换手率
		double dHSl = Con_doubleInvalid;
		if(pSecurity->IsFund())
		{
			if (pSecurity->IsFund_Close())
			{
				TxFundShareData* pFundShare = pSecurity->GetTxFundShareDataByDate(lDate);
				if(pFundShare!= NULL)
				{
					double dltgb = pFundShare->TradeableShare;
					if (dltgb > 0.1) dHSl = dVolume/dltgb*100;
				}
			}
		}
		else
		{
			TxShareData* pShare = pSecurity->GetTxShareDataByDate(lDate);
			if(pShare!= NULL)
			{
				double dltgb  = pShare->TradeableShare;
				if (dltgb > 0.1) dHSl = dVolume/dltgb*100;
			}
		}		
		//else ASSERT(0);
		// end of 2011-02-25

		if(i==0)
		{
			TempK = m_vecHisTradeRsc[i];	
			CTime tm(lDate/10000,lDate/100%100,lDate%100,0,0,0);
			long datetime = (long)(tm.GetTime());
			MaxDate = GetMaxDate(datetime,lType);
			// 利用 TempK.Res 传回换手率
			memcpy(TempK.Res, (BYTE*)(&dHSl), sizeof(double));
			// end of 2011-02-25
			if(lConut==1)
			{
				m_vecHisTradeTem.push_back(TempK);
				break;
			}
			continue;
		}
		if(lDate <= MaxDate)
		{
			TempK.Date = lDate;
			TempK.Close = m_vecHisTradeRsc[i].Close;
			TempK.Amount += m_vecHisTradeRsc[i].Amount;
			TempK.Volume += m_vecHisTradeRsc[i].Volume;
			if(m_vecHisTradeRsc[i].High>TempK.High /*&& (m_vecHisTradeRsc[i].Date > iStartDate&&m_vecHisTradeRsc[i].Date < iEndDate)*/)
				TempK.High = m_vecHisTradeRsc[i].High ;
			if(TempK.Low > m_vecHisTradeRsc[i].Low /*&&(m_vecHisTradeRsc[i].Date > iStartDate&&m_vecHisTradeRsc[i].Date < iEndDate)*/)
				TempK.Low = m_vecHisTradeRsc[i].Low;

			// 2011-02-25 换手率累加
			// 利用 TempK.Res 传回换手率
			double oldHSl = *((double*)TempK.Res);
			if (oldHSl != Con_doubleInvalid)
			{
				if (dHSl != Con_doubleInvalid)
				{
					dHSl += oldHSl;
				}
				memcpy(TempK.Res, (BYTE*)(&dHSl), sizeof(double));
			}
			//if (dHSl != Con_doubleInvalid)
			//{
			//	double oldHSl = *((double*)TempK.Res);
			//	if (oldHSl != Con_doubleInvalid)
			//	{
			//		dHSl += oldHSl;
			//	}
			//	memcpy(TempK.Res, (BYTE*)(&dHSl), sizeof(double));
			//}
			// end of 2011-02-25
		}
		else
		{
			m_vecHisTradeTem.push_back(TempK);
			TempK = m_vecHisTradeRsc[i];
			// 利用 TempK.Res 传回换手率
			memcpy(TempK.Res, (BYTE*)(&dHSl), sizeof(double));
			// end of 2011-02-25
			/*TempK.High = -1;
			TempK.Low = 100000;*/
			CTime tm(lDate/10000,lDate/100%100,lDate%100,0,0,0);
			long datetime = (long)(tm.GetTime());
			MaxDate = GetMaxDate(datetime,lType);
		}
		if(i ==(lConut-1))
		{
			m_vecHisTradeTem.push_back(TempK);
			break;
		}
	}
}
//计算周期时间
long TxBusiness::GetMaxDate(long lTime,long lType)
{
	long reDate = 0;
	CTime	tm(lTime);
	switch(lType)
	{
	case 0://TRADE_DAY:
		{
			CTimeSpan tmsp(1,0,0,0);
			tm += tmsp;
			reDate = tm.GetYear()*10000+ tm.GetMonth()*100+tm.GetDay();
		}
		break;
	case 1://TRADE_WEEK:
		{
			int week = tm.GetDayOfWeek();
			int cha = 8 - week;
			CTimeSpan tmsp(cha,0,0,0);
			tm += tmsp;
			reDate = tm.GetYear()*10000+ tm.GetMonth()*100+tm.GetDay();
		}
		break;
	case 2://TRADE_MONTH:
		{
			int year = tm.GetYear();
			int mon	 = tm.GetMonth();
			mon += 1;
			if(mon>12)
			{
				year +=1;
				mon = 1;
			}
			int day  = 0;

			reDate = year*10000+ mon*100+day;
		}
		break;
	case 4://TRADE_YEAR:
		{
			int year = tm.GetYear();
			int mon	 = 1;
			int day  = 1;
			year +=1;
			reDate = year*10000+ mon*100+day;
		}
		break;
	default:
		break;
	}
	return reDate;
}
//adsded by zhangxs 股票交易数据
bool TxBusiness::SetStockTableData(Table_Display& baTable,
								   SecurityQuotation* pSecurity,
								   std::vector<HisTradeData> m_vecHisTrade,
								   int iStartdate,
								   int iEndDate,
								   int iFlag,
								   bool bFirst,
								   int iWeight)
{
	int m_iRowcount = (int)m_vecHisTrade.size();
	if(m_iRowcount < 1)
		return false;
	for(int nRow = 0;nRow<m_iRowcount;nRow++)
	{
		if((int)baTable.GetRowCount() >= m_iRowcount)
			continue;
		baTable.AddRow();
		int nCol = 0;
		HisTradeData pHisTradeData = m_vecHisTrade[nRow];

		double dValue = Con_doubleInvalid;
		double dValueRaise = Con_doubleInvalid;
		float fOpen,fHigh,fLow,fClose,fPreClose;
		double dVolume,dAmount;
		float fAvgPrice = Con_floatInvalid;

		int iDataDate = pHisTradeData.Date;

		fOpen = pHisTradeData.Open;
		fClose = pHisTradeData.Close;
		float m_fClose = fClose;
		fHigh = pHisTradeData.High;
		fLow = pHisTradeData.Low;
		fPreClose = pHisTradeData.Preclose;
		dVolume = pHisTradeData.Volume;
		dAmount = pHisTradeData.Amount;
		if(dVolume > 0.01)
			fAvgPrice = (float)(dAmount/dVolume);

		// 2011-02-25 换手率
		double dHSl = Con_doubleInvalid;
		if (iFlag != 0)
		{
			// 2011-02-25 如果不是“按日”统计，则换手率已经在 TransitionData 中计算过了，取回换手率
			dHSl = *((double*)pHisTradeData.Res);
		}

		//iWeight == -1 表示不复权
		//0-前复权
		//1-后复权
		//2-全复权前复权
		//3-全复权后复权
		double dScale = 1.0;
		if(iWeight!=-1)
		{
			int iFirstDate = 0;
			HisTradeData* pFirstHisTradeData = pSecurity->GetTradeDataByIndex(0,iFlag,bFirst);
			if(pFirstHisTradeData!=NULL)
				iFirstDate = pFirstHisTradeData->Date;

			if(iWeight == 0)
			{
				dScale = pSecurity->GetExdividendScale(iDataDate,iEndDate,true);
				/*if(iDataDate >= iStartdate && iDataDate <= iEndDate)
				{
					int m_iDataDate = iDataDate;
					if(nRow > 0)
						m_iDataDate = m_vecHisTrade[nRow-1].Date;
					dScale = pSecurity->GetExdividendScale(m_iDataDate,iEndDate,true);
					fOpen *=(float)dScale;
					fPreClose *= (float)dScale;
				}			*/	
			}
			else if(iWeight == 1)
				dScale = pSecurity->GetExdividendScale(iStartdate,iDataDate,false);
			else if(iWeight == 2)
				dScale = pSecurity->GetExdividendScale(iDataDate,21001231,true);
			else if(iWeight == 3)
				dScale = pSecurity->GetExdividendScale(19791201,iDataDate,false);
		}
		if(fAvgPrice >0)
			fAvgPrice = (float)(fAvgPrice *dScale);
		if(fClose<0 || fPreClose<0)
			dValue = Con_doubleInvalid;
		else
			dValue = fClose - fPreClose;
		int m_iTabCount = baTable.GetRowCount();
		//日期
		baTable.SetCell(nCol,nRow,iDataDate);
		nCol++;

		//涨幅
		dValueRaise = dValue;
		if( fabs(dValueRaise-Con_doubleInvalid)>0.000001 && fPreClose>0)
		{
			dValueRaise /= fPreClose;
			dValueRaise *= 100;
		}
		baTable.SetCell(nCol,nRow,dValueRaise);
		nCol++;

		//涨跌
		baTable.SetCell(nCol,nRow,dValue);
		nCol++;

		//前收
		baTable.SetCell(nCol,nRow,fPreClose);
		nCol++;

		//开盘
		baTable.SetCell(nCol,nRow,fOpen);
		nCol++;

		//最高
		baTable.SetCell(nCol,nRow,fHigh);
		nCol++;

		//最低
		baTable.SetCell(nCol,nRow,fLow);
		nCol++;

		//收盘
		baTable.SetCell(nCol,nRow,fClose);
		nCol++;

		//均价
		if( fAvgPrice < 0.000001 )
		{
			fAvgPrice = Con_floatInvalid;
		}
		baTable.SetCell(nCol,nRow,fAvgPrice);
		nCol++;

		//成交量
		if(dVolume < 0.001)
			dVolume = Con_doubleInvalid;
		baTable.SetCell(nCol,nRow,dVolume);
		nCol++;
		//成交金额
		if(dAmount < 0.001)
			dAmount = Con_doubleInvalid;
		baTable.SetCell(nCol,nRow,dAmount);
		nCol++;

		//换手率
		double dltgb = Con_doubleInvalid;
		double dLtsz = Con_doubleInvalid;
		double dTotalValue = Con_doubleInvalid;
		double dDomainValue = Con_doubleInvalid;
		//double dHSl = Con_doubleInvalid;	// 2011-02-25 
		if(pSecurity->IsFund())
		{
			
			TxFundShareData* pFundShare = pSecurity->GetTxFundShareDataByDate(iDataDate);
			if(pFundShare!= NULL)
			{
				dltgb  = pFundShare->TradeableShare;
				dTotalValue = pFundShare->TotalShare;
			}
		}
		else
		{
			TxShareData* pShare = pSecurity->GetTxShareDataByDate(iDataDate);
			if(pShare!= NULL)
			{
				dltgb  = pShare->TradeableShare;
				dTotalValue = pShare->TotalShare;
			}
		}		
		// 2011-02-25 如果不是“按日”统计，则换手率已经在 TransitionData 中计算过了，
		if (iFlag == 0)
		{
			if(pSecurity->IsFund()) 
			{
				if (pSecurity->IsFund_Close()) 
				{
					//dltgb = pSecurity->GetTradableShare();
					if(dltgb > 0.1 && dltgb != Con_doubleInvalid)
						dHSl = dVolume/dltgb*100;
					if(dHSl < 0.001)
						dHSl = Con_doubleInvalid;
				}
			}
			else
			{
				if(dltgb > 0.1 && dltgb != Con_doubleInvalid)
					dHSl = dVolume/dltgb*100;
				if(dHSl < 0.001)
					dHSl = Con_doubleInvalid;
			}
		}
		// end of 2011-02-25
		baTable.SetCell(nCol,nRow,dHSl);
		nCol++;

		//流通市值	
		dLtsz = (double)m_fClose * dltgb;
		if(dLtsz <0.1)
			dLtsz = Con_doubleInvalid;
		baTable.SetCell(nCol,nRow,dLtsz);
		nCol++;
		//境内总值	
		dTotalValue = (double)m_fClose * dTotalValue;//总市值
		if(dTotalValue < 0.1)
			dTotalValue = Con_doubleInvalid;	
		dDomainValue = (double)m_fClose * (pSecurity->GetNationShare());
		//境内流通总值为0，则用总市值来补充
		/*if(dDomainValue < 0.01)
		dDomainValue = dTotalValue;
		baTable.SetCell(nCol,nRow,dDomainValue);
		nCol++;*/
		//总市值
		/*double dTotalValue = Con_doubleInvalid;
		dTotalValue = pSecurity->GetPreClosePrice(iDataDate) * (pSecurity->GetTotalShare());*/
		baTable.SetCell(nCol,nRow,dTotalValue);
		nCol++;
	}
	return true;
}
void TxBusiness::GetCQDate(std::vector<HisTradeData>& m_vecHisTradeRsc,
			   SecurityQuotation* pSecurity,
			   int m_iStartDate,
			   int m_iEndDate,
			   int iWeight)
{
	if((int)m_vecHisTradeRsc.size() < 1)
		return;
	if(pSecurity == NULL)
		return;
	if(iWeight == -1)
		return;
	std::vector<ExdividendData> m_vecCqData;
	int nCount = pSecurity->GetExdividendDataCount();
	for(int i=0; i<nCount; i++)
	{
		ExdividendData*	pCqInfo = pSecurity->GetExdividendDataByIndex(i);
		if(pCqInfo)
		{
			ExdividendData m_tempExdiv;
			m_tempExdiv.iDate = pCqInfo->iDate;
			m_tempExdiv.ExdividendPrice = pCqInfo->ExdividendPrice;
			m_tempExdiv.PreClosePrice = pCqInfo->PreClosePrice;
			m_tempExdiv.ExdividendType = pCqInfo->ExdividendType;
			m_vecCqData.push_back(m_tempExdiv);
		}
	}
	int bAhead;
	//iWeight == -1 表示不复权
	//0-前复权
	//1-后复权
	//2-全复权前复权
	//3-全复权后复权
	if(iWeight == 0)
		bAhead = 0;
	else if(iWeight == 1)
		bAhead = 1;
	else if(iWeight == 2)
	{
		bAhead = 0;
		m_iStartDate = 19000101;
		m_iEndDate = 25000101;
	}
	else if(iWeight == 3)
	{
		bAhead = 1;
		m_iStartDate = 19000101;
		m_iEndDate = 25000101;
	}
	else
		bAhead = -1;
	int CQdate = m_iStartDate;
	int Predate = m_iStartDate;
	double CQcale = 1;
	if(!bAhead)				//前复权
	{	
		CQdate = 0;
		for(int i=0; i<(int)m_vecHisTradeRsc.size(); i++)
		{
			int curDate = m_vecHisTradeRsc[i].Date;
			if(curDate>m_iEndDate)
				continue;
			if(curDate>=CQdate)
			{
				CQcale = GetCQcaleFromDate(m_vecCqData,m_iStartDate,m_iEndDate,Predate,&CQdate,bAhead);
				Predate = CQdate;
			}
			m_vecHisTradeRsc[i].Preclose *= (float)CQcale;
			m_vecHisTradeRsc[i].Close *= (float)CQcale;
			m_vecHisTradeRsc[i].High *= (float)CQcale;
			m_vecHisTradeRsc[i].Low *= (float)CQcale;
			m_vecHisTradeRsc[i].Open *= (float)CQcale;
		}
	}
	else	//后复权
	{
		for(int i=0; i<(int)m_vecHisTradeRsc.size(); i++)
		{
			int curDate = m_vecHisTradeRsc[i].Date;
			if(curDate<m_iStartDate)
				continue;
			if(curDate>=CQdate)
			{
				CQcale = GetCQcaleFromDate(m_vecCqData,m_iStartDate,m_iEndDate,Predate,&CQdate,bAhead);
				Predate = CQdate;
			}
			m_vecHisTradeRsc[i].Preclose *= (float)CQcale;
			m_vecHisTradeRsc[i].Close *= (float)CQcale;
			m_vecHisTradeRsc[i].High *= (float)CQcale;
			m_vecHisTradeRsc[i].Low *= (float)CQcale;
			m_vecHisTradeRsc[i].Open *= (float)CQcale;
		}
	}
}
double TxBusiness::GetCQcaleFromDate(std::vector<ExdividendData> m_vecCqData,
									 int StraDate,
									 int EndDate,
									 int PreDate,
									 int* CQDate,
									 int m_bAspect)
{
	double CQcale = 1;
	*CQDate = 25000101;
	int i=0;
	if(m_bAspect==-1||m_vecCqData.size()<1)		//取消复权
	{
		*CQDate = 25000101;
		return CQcale;
	}
	else if(m_bAspect==0)		//向前复权
	{
		int lCQDate = 0;
		for(i=m_vecCqData.size()-1; i>=0; i--)
		{
			if(m_vecCqData[i].iDate>EndDate)
				continue;
			else if(m_vecCqData[i].iDate<StraDate)
			{
				if(lCQDate>0)
					*CQDate = lCQDate;
				else
					*CQDate = 25000101;
				break;
			}
			else
			{
				if(m_vecCqData[i].iDate>PreDate)
				{
					if(m_vecCqData[i].ExdividendPrice>0.001)
						CQcale *= m_vecCqData[i].ExdividendPrice/m_vecCqData[i].PreClosePrice;
					else
						continue;

					*CQDate = m_vecCqData[i].iDate;
					lCQDate = m_vecCqData[i].iDate;
					if(i == 0)
						break;
				}
			}
		}
	}
	else						//向后复权
	{
		for(i=0 ; i<(int)m_vecCqData.size(); i++)
		{
			if(m_vecCqData[i].iDate<StraDate)
				continue;
			else if(m_vecCqData[i].iDate>EndDate)
			{
				*CQDate = 25000101;
				break;
			}
			else
			{
				if(m_vecCqData[i].iDate<=PreDate)
				{
					if(m_vecCqData[i].ExdividendPrice>0.001)
						CQcale *= m_vecCqData[i].PreClosePrice/m_vecCqData[i].ExdividendPrice;
					else
						continue;
					if(i == m_vecCqData.size()-1)
					{
						*CQDate = 25000101;
						break;
					}
				}
				else 
				{
					*CQDate = m_vecCqData[i].iDate;
					break;
				}
			}
		}
	}

	return CQcale;
}

//债券交易数据
bool TxBusiness::SetBondTableCol(Table_Display& baTable, int nPrecise)	// 2011-03-03 修改深市债券价格小数点位数
{
	//如果UI没有输入列信息，自行添加
	if(baTable.GetColCount()<=0)
	{
		int nCol=0;

		//交易日期
		baTable.AddCol(Tx::Core::dtype_int4);
		baTable.SetFormatStyle(nCol,fs_date);
		nCol++;
		//涨幅
		baTable.AddCol(Tx::Core::dtype_double);
		nCol++;
		//涨跌
		baTable.AddCol(Tx::Core::dtype_double);
		nCol++;
		//前收
		baTable.AddCol(Tx::Core::dtype_float);
		nCol++;
		//开盘
		baTable.AddCol(Tx::Core::dtype_float);
		nCol++;
		//最高
		baTable.AddCol(Tx::Core::dtype_float);
		nCol++;
		//最低
		baTable.AddCol(Tx::Core::dtype_float);
		nCol++;
		//收盘
		baTable.AddCol(Tx::Core::dtype_float);
		nCol++;
		//均价
		baTable.AddCol(Tx::Core::dtype_float);
		nCol++;
		//成交量
		baTable.AddCol(Tx::Core::dtype_double);
		nCol++;
		//成交金额
		baTable.AddCol(Tx::Core::dtype_double);
		nCol++;
		// 应计利息	
		baTable.AddCol(Tx::Core::dtype_double);		
		nCol++;
		// YTM
		baTable.AddCol(Tx::Core::dtype_double);	
		nCol++;
		// 修正久期
		baTable.AddCol(Tx::Core::dtype_double);		
		nCol++;
		//凸性
		baTable.AddCol(Tx::Core::dtype_double);
		nCol++;
		// 剩余期限
		baTable.AddCol(Tx::Core::dtype_double);
		nCol++;

		nCol=0;
		//baTable.SetTitle(nCol, _T("名称"));
		//++nCol;
		baTable.SetTitle(nCol, _T("日期"));
		++nCol;
		baTable.SetTitle(nCol, _T("涨幅(%)"));
        baTable.SetPrecise(nCol, 2); // 2
		++nCol;
		baTable.SetTitle(nCol, _T("涨跌"));
		baTable.SetPrecise(nCol, nPrecise); // 2
		++nCol;		
		baTable.SetTitle(nCol, _T("前收"));
		baTable.SetPrecise(nCol, nPrecise); // 2
		++nCol;
		baTable.SetTitle(nCol, _T("开盘"));
		baTable.SetPrecise(nCol, nPrecise); // 2
		++nCol;
		baTable.SetTitle(nCol, _T("最高"));
		baTable.SetPrecise(nCol, nPrecise); // 2
		++nCol;
		baTable.SetTitle(nCol, _T("最低"));
		baTable.SetPrecise(nCol, nPrecise); // 2
		++nCol;
		baTable.SetTitle(nCol, _T("收盘"));
		baTable.SetPrecise(nCol, nPrecise); // 2
		++nCol;
		baTable.SetTitle(nCol, _T("均价"));
		baTable.SetPrecise(nCol, nPrecise); // 2
		++nCol;
		baTable.SetTitle(nCol, _T("成交量(张)"));//("成交量(股)"));  BUG:13052   2012-09-26
		baTable.SetPrecise(nCol, 0);
		//baTable.SetOutputItemRatio(nCol, 100);
		baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
		++nCol;
		baTable.SetTitle(nCol, _T("成交金额(万)"));
		baTable.SetPrecise(nCol, 2);
		baTable.SetOutputItemRatio(nCol, 10000);
		baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
		++nCol;
		baTable.SetTitle(nCol, _T("应计利息"));
		baTable.SetPrecise(nCol, 4);
		++nCol;
		baTable.SetTitle(nCol, _T("YTM"));
		baTable.SetPrecise(nCol, 3);
		++nCol;
		baTable.SetTitle(nCol, _T("修正久期"));
		baTable.SetPrecise(nCol, 2);
		++nCol;
		baTable.SetTitle(nCol, _T("凸性"));
		baTable.SetPrecise(nCol, 2);
		++nCol;
		baTable.SetTitle(nCol, _T("剩余期限"));
		baTable.SetPrecise(nCol, 3);
		//++nCol;
	}
	return true;
}
//adsded by zhangxs 债券交易数据
bool TxBusiness::SetBondTableData(Table_Display& baTable,
									SecurityQuotation* pSecurity,
									std::vector<HisTradeData> m_vecHisTrade,
									int iStartdate,
									int iEndDate,
									int iFlag,
									bool bFirst,
									int iWeight,
									bool bIsFullPrice)
{
	int m_iRowcount = (int)m_vecHisTrade.size();
	if(m_iRowcount < 1)
		return false;
	CTime time = CTime::GetCurrentTime();
	int iTime = time.GetYear()*10000 + time.GetMonth()*100 + time.GetDay();
	Tx::Data::RepurchaseAvgPrice	repoData;
	for(int nRow = 0;nRow<m_iRowcount;nRow++)
	{
		if((int)baTable.GetRowCount() >= m_iRowcount)
			continue;
		baTable.AddRow();
		int nCol = 0;
		HisTradeData pHisTradeData = m_vecHisTrade[nRow];
		double dValue = Con_doubleInvalid;
		double dValueRaise = Con_doubleInvalid;
		float fOpen,fHigh,fLow,fClose,fPreClose;
		double dVolume,dAmount;
		float fAvgPrice =  Con_floatInvalid;
		int iDataDate = pHisTradeData.Date;
		fOpen = pHisTradeData.Open;
		fClose = pHisTradeData.Close;
		fHigh = pHisTradeData.High;
		fLow = pHisTradeData.Low;
		fPreClose = pHisTradeData.Preclose;
		dVolume = pHisTradeData.Volume;
		dAmount = pHisTradeData.Amount;
		if(dVolume > 0.01)
		{
			if(pSecurity->IsRepurchase())
			{
				pSecurity->GetDataByObj( Tx::Data::dt_RepurchaseAvgPrice, iDataDate, repoData );
				fAvgPrice = (float)repoData.dAvgPrice;
			}
		}
		else
			fAvgPrice = fPreClose;
		fAvgPrice = fAvgPrice > 0.0001?fAvgPrice:Con_floatInvalid;
		//企业债使用全价交易
		//其他债使用净价交易,2003年之前使用全价交易
		//if(p->IsBond_Company()==false)
		Tx::Business::TxBond bond;	
		double dInterest;
		//dInterest = bond.GetInterest(pSecurity,iDataDate);
		//2012-7-16  应计利息(新)
		dInterest = bond.GetInterest_New(pSecurity,iDataDate,true);
		if(bIsFullPrice && pSecurity->IsBondNetPrice()==true && dInterest >0)
		{
			fOpen += (float)dInterest;
			fClose += (float)dInterest;
			fHigh += (float)dInterest;
			fLow += (float)dInterest;
			fPreClose += (float)dInterest;
			fAvgPrice += (float)dInterest;
		}

		//iWeight == -1 表示不复权
		//0-前复权
		//1-后复权
		//2-全复权前复权
		//3-全复权后复权
		if(iWeight!=-1)
		{
			int iFirstDate = 0;
			HisTradeData* pFirstHisTradeData = pSecurity->GetTradeDataByIndex(0,iFlag,bFirst);
			if(pFirstHisTradeData!=NULL)
				iFirstDate = pFirstHisTradeData->Date;

			if(iWeight == 0)
			{
				double dScale = pSecurity->GetExdividendScale(iDataDate,iEndDate,true);
				if(fClose>0)
					fClose = (float)(fClose*dScale);
				if(fOpen>0)
					fOpen = (float)(fOpen*dScale);
				if(fHigh>0)
					fHigh = (float)(fHigh*dScale);
				if(fLow>0)
					fLow = (float)(fLow*dScale);
				if(fPreClose>0)
					fPreClose = (float)(fPreClose*dScale);
				if(fAvgPrice >0)
					fAvgPrice = (float)(fAvgPrice *dScale);
			}
			else if(iWeight == 1)
			{
				double dScale = pSecurity->GetExdividendScale(iStartdate,iDataDate,false);
				if(fClose>0)
					fClose = (float)(fClose*dScale);
				if(fOpen>0)
					fOpen = (float)(fOpen*dScale);
				if(fHigh>0)
					fHigh = (float)(fHigh*dScale);
				if(fLow>0)
					fLow = (float)(fLow*dScale);
				if(fPreClose>0)
					fPreClose = (float)(fPreClose*dScale);
				if(fAvgPrice >0)
					fAvgPrice = (float)(fAvgPrice *dScale);
			}
			else if(iWeight == 2)
			{
				double dScale = pSecurity->GetExdividendScale(iDataDate,21001231,true);
				if(fClose>0)
					fClose = (float)(fClose*dScale);
				if(fOpen>0)
					fOpen = (float)(fOpen*dScale);
				if(fHigh>0)
					fHigh = (float)(fHigh*dScale);
				if(fLow>0)
					fLow = (float)(fLow*dScale);
				if(fPreClose>0)
					fPreClose = (float)(fPreClose*dScale);
				if(fAvgPrice >0)
					fAvgPrice = (float)(fAvgPrice *dScale);
			}
			else if(iWeight == 3)
			{
				double dScale = pSecurity->GetExdividendScale(19791201,iDataDate,false);
				if(fClose>0)
					fClose = (float)(fClose*dScale);
				if(fOpen>0)
					fOpen = (float)(fOpen*dScale);
				if(fHigh>0)
					fHigh = (float)(fHigh*dScale);
				if(fLow>0)
					fLow = (float)(fLow*dScale);
				if(fPreClose>0)
					fPreClose = (float)(fPreClose*dScale);
				if(fAvgPrice >0)
					fAvgPrice = (float)(fAvgPrice *dScale);
			}
		}
		if(fClose<0 || fPreClose<0)
			dValue = Con_doubleInvalid;
		else
			dValue = fClose - fPreClose;
		int m_iTabCount = baTable.GetRowCount();
		//日期
		baTable.SetCell(nCol,nRow,iDataDate);
		nCol++;

		//涨幅
		dValueRaise = dValue;
		if( fabs(dValueRaise-Con_doubleInvalid)>0.000001 && fPreClose>0)
		{
			dValueRaise /= fPreClose;
			dValueRaise *= 100;
		}
		baTable.SetCell(nCol,nRow,dValueRaise);
		nCol++;

		//涨跌
		baTable.SetCell(nCol,nRow,dValue);
		nCol++;

		//前收
		baTable.SetCell(nCol,nRow,fPreClose);
		nCol++;

		//开盘
		baTable.SetCell(nCol,nRow,fOpen);
		nCol++;

		//最高
		baTable.SetCell(nCol,nRow,fHigh);
		nCol++;

		//最低
		baTable.SetCell(nCol,nRow,fLow);
		nCol++;

		//收盘
		baTable.SetCell(nCol,nRow,fClose);
		nCol++;


		//均价
		if( fAvgPrice < 0.000001 )
		{
			fAvgPrice = Con_floatInvalid;
		}
		baTable.SetCell(nCol,nRow,fAvgPrice);
		nCol++;

		//成交量
		if(dVolume < 0.001)
			dVolume = Con_doubleInvalid;
		baTable.SetCell(nCol,nRow,dVolume);
		nCol++;
		//成交金额
		if(dAmount < 0.001)
			dAmount = Con_doubleInvalid;
		baTable.SetCell(nCol,nRow,dAmount);
		nCol++;

		//应计利息	
		if(pSecurity->IsRepurchase())
			dInterest = Con_doubleInvalid;
		if(dInterest < 0)
			dInterest = Con_doubleInvalid;
		baTable.SetCell(nCol,nRow,dInterest);
		nCol++;
		double dYtm = Con_doubleInvalid;
		double dDuration = Con_doubleInvalid;
		double dMduration = Con_doubleInvalid;
		double dConvexity = Con_doubleInvalid;
		//bond.CalcYTM(dYtm,dDuration,dMduration,dConvexity,pSecurity->GetId(),iDataDate);

		if (iTime == iDataDate)
			bond.GetYTM(pSecurity,dYtm,dDuration,dMduration,dConvexity);
		else
		    bond.GetYTM(pSecurity,iDataDate,dYtm,dDuration,dMduration,dConvexity);
		//YTM
		if(!(fabs(dYtm-Con_doubleInvalid)<0.000001))
			dYtm *= 100;
		baTable.SetCell(nCol,nRow,dYtm);
		nCol++;
		//修正久期
		baTable.SetCell(nCol,nRow,dMduration);
		nCol++;
		//凸性
		baTable.SetCell(nCol,nRow,dConvexity);
		nCol++;
		//剩余期限
		double m_iday = 0;
		m_iday = bond.GetRemnants(pSecurity,iDataDate);
		m_iday = m_iday < 0? Con_doubleInvalid:m_iday;
		baTable.SetCell(nCol,nRow,m_iday);
		nCol++;
	}
	return true;
}

//债券交易数据 20090326
bool TxBusiness::GetBondTradeData(Table_Display& baTable,
									int iSecurityId,
									int iStartdate,
									int iEndDate,
									int iFlag,
									bool bFirst,
									int iWeight,
									bool bNeedProgressWnd,
									bool bIsFullPrice)
{
	baTable.Clear();
	// 2011-03-03 修改深市债券价格小数点位数
	//step1 处理结果数据的标题列
	SecurityQuotation* psq = GetSecurityNow(iSecurityId);
	int nPrecise = (psq && psq->IsShenzhen()) ? 3 : 2;
	SetBondTableCol(baTable, nPrecise);

	// end of: 2011-03-03 修改深市债券价格小数点位数
	if(psq==NULL)
		return false;

	int iCol = 0;
	if(baTable.GetColCount()<=0)
		return false;

	//step2
	//控制是否需要进度条
	Tx::Core::ProgressWnd prw;
	CString sProgressPrompt;
	UINT progId = 0;
	if(bNeedProgressWnd==true)
	{
		//支持取消
		prw.EnableCancelButton(true);
		sProgressPrompt.Format(_T("交易数据..."));
		progId = prw.AddItem(1,sProgressPrompt, 0.0);
		prw.Show(15);
	}

	//step3 取得指定交易所的当前日期
	//int idate = 0;
	int i =0;

	int row= baTable.GetRowCount();
	int col = baTable.GetColCount();

	bool bNeedCheckToday = false;
	bool bNeedAppendToday = false;
	CalStatusAboutToday(psq,iEndDate,bNeedCheckToday,bNeedAppendToday);

	//step4 循环日期
	int count = psq->GetTradeDataCount();
	if(count < 1)
	{
		return false;
	}
	//
	//int count = psq->GetTradeDateCount(iFlag);
	if(iWeight!=-1)
	{
		//只有沪深股票，基金和债券要处理复权
		if(
			psq->IsIndex() == true ||
			psq->IsWarrant() == true ||
			(psq->IsStock() == true && psq->IsHK_Market()==true) ||
			psq->IsFutures() == true
			)
			iWeight = -1;
	}
	std::vector<HisTradeData> m_vecHisTrade;
	for (i=0;i<count;i++)
	{
		if(bNeedProgressWnd==true)
		{
			prw.SetPercent(progId, (double)i/(double)count);
			if(prw.IsCanceled()==true)
				break;
		}

		//step 5.1
		//第一列为样本ID
		//取得样本的ID

		//step5.2 取得样本的Security指针
		//baTable.AddRow();
		//step5.3 处理基本指标数据
		int j = 0;
		//SetStockTableData(baTable,psq,j,i,iStartdate,iEndDate,iFlag,bFirst,iWeight);
		HisTradeData* pHisTradeData = psq->GetTradeDataByIndex(i);
		if(pHisTradeData==NULL)
			continue;
		m_vecHisTrade.push_back(*pHisTradeData);

		if(bNeedCheckToday && iEndDate == pHisTradeData->Date)
			bNeedAppendToday = false;
	}
	//刘鹏，2012-04-06，加入实时行情的数据
	PushBackToday(bNeedAppendToday,psq,m_vecHisTrade);

	GetCQDate(m_vecHisTrade,psq,iStartdate,iEndDate,iWeight);
	std::vector<HisTradeData> m_vecHisTradeTerm;
	TransitionData(m_vecHisTrade,m_vecHisTradeTerm,psq,iStartdate,iEndDate,iFlag);
	SetBondTableData(baTable,psq,m_vecHisTradeTerm,iStartdate,iEndDate,iFlag,bFirst,iWeight,bIsFullPrice);

	//step6 循环完成
	if(bNeedProgressWnd==true)
	{
		prw.SetPercent(progId, 1.0);
		sProgressPrompt+=_T(",完成!");
		prw.SetText(progId, sProgressPrompt);
	}
	return true;
}
//2009-06-25
double TxBusiness::GetBigBillValue(SecurityQuotation* p,int date)
{
	if(p==NULL || date<=0)
		return 0;
	//取得指定日期的股本数据
	TxShareData* pTxShareData = p->Security::GetTxShareDataByDate(date);
	if(pTxShareData==NULL) return 0;
/*
个股流通盘（股）	大单下限（股）
不大于1亿	20000
1亿到5亿	40000
5亿到10亿	80000
10亿到100亿	100000
超过100亿	120000
*/
	double dTradeableShare = pTxShareData->TradeableShare/100000000;
	if(dTradeableShare>100)
		return 120000;
	else if(dTradeableShare>10)
		return 100000;
	else if(dTradeableShare>5)
		return 80000;
	else if(dTradeableShare>1)
		return 40000;
	else 
		return 20000;
}

//2009-06-25
void TxBusiness::StatDetailData(std::vector<int>& vSample,int iStart,int iEnd,CString sBlockName)
{
	int date = 20090606;
	double dBigBillValue = 0;
	//COleDateTime dt;
	int iRow = 1;

	Tx::Core::CExcel excel;
	//2008-06-26
	if(excel.IsInited()==false)
		return;
	CString sContent;
	sContent = _T("名称\t日期\t大单买量\t大单卖量\t大单流入\t大单流出\t小单买量\t小单卖量\t小单流入\t小单流出");
	excel.PasteTo(sContent,iRow++,1,false);

	//int iStart = m_pStartDate->GetTime(dt);
	//iStart = atol(dt.Format(_T("%Y%m%d")));

	//int iEnd = m_pEndDate->GetTime(dt);
	//iEnd = atol(dt.Format(_T("%Y%m%d")));

	CString sPmp;
	if(iEnd<iStart)
	{
		sPmp.Format(_T("起始日期[%d]应该小于等于终止日期[%d],请重新输入!"),iStart,iEnd);
		AfxMessageBox(sPmp);
		return;
	}

	int count = vSample.size();//m_pSecurityList->GetCount();
	if(count<=0)
	{
		sPmp.Format(_T("请选择输出样本!"));
		AfxMessageBox(sPmp);
		return;
	}
	int startIndex,endIndex;
	//Tx::Business::TxBusiness tb;
	//startIndex = tb.m_pShIndex->GetTradeDateIndex(iStart);
	//endIndex = tb.m_pShIndex->GetTradeDateIndex(iEnd);
	startIndex = m_pShIndex->GetTradeDateIndex(iStart);
	if(startIndex<0) startIndex = 0;
	endIndex = m_pShIndex->GetTradeDateIndex(iEnd);
	if(endIndex<0)
		endIndex = m_pShIndex->GetTradeDateCount()-1;

	Tx::Core::ProgressWnd prw;
	//step2
	CString sProgressPrompt;
	sProgressPrompt =  _T("大单输出...");
	UINT progId = prw.AddItem(1,sProgressPrompt, 0.0);
	//step3
	prw.Show(15);

	//循环处理每个交易日
	for(int si=startIndex;si<=endIndex;si++)
	{
		double dbVolume0=0,dbAmount0=0;
		double dbVolume1=0,dbAmount1=0;
		double dbVolume2=0,dbAmount2=0;
		double dbVolume3=0,dbAmount3=0;
		date = m_pShIndex->GetTradeDateByIndex(si);
		if(date<iStart) continue;

		//循环处理样本
		for(int i=0;i<count;i++)
		{
			//step4 进度条位置
			prw.SetPercent(progId, (double)i/(double)count);

			//取得样本id
			int id = vSample[i];//(int)m_pSecurityList->GetItemData(i);
			SecurityQuotation *p = (SecurityQuotation*)GetSecurity(id);
			if(p==NULL)
				continue;
			{
			CString sTitle;
			sTitle.Format(_T("大单输出...[%s-%s] %d"),p->GetName(),p->GetCode(),date);
			prw.SetText(progId,sTitle);
			}

			//取得交易日期
			//date = p->GetTradeDateByIndex(si);
			//取得交易日期数据
			HisTradeLatestDataOfaDayStruct* pHisTradeLatestDataOfaDayStruct = p->GetHisTradeLatestDataOfaDay( date );
			if(pHisTradeLatestDataOfaDayStruct==NULL)
				continue;
			if(pHisTradeLatestDataOfaDayStruct->Date != date)
				continue;

			TradeDetail* pHisTradeDetail = NULL;
			//取得历史成交明细
			pHisTradeDetail = p->GetHisTradeDetailData(date);

			if(pHisTradeDetail!=NULL)
			{
				//取得大单阀值
				dBigBillValue = GetBigBillValue(p,date);
				int htdCount = pHisTradeDetail->GetTrdaeDetailDataCount();
				//循环处理大单
				for(int ihtd=0;ihtd<htdCount;ihtd++)
				{
					//确定基准价格
					float fPreClose=0,fOpenPrice=0;
					if(ihtd==0)
					{
						//第一笔前收价为基准价
						fPreClose = pHisTradeLatestDataOfaDayStruct->Preclose;
						fOpenPrice = pHisTradeLatestDataOfaDayStruct->Open;
					}
					else
					{
						//后续笔，取前一笔价格为基准价
						TradeDetailData* pTradeDetailData = pHisTradeDetail->GetTrdaeDetailData(ihtd-1);
						if(pTradeDetailData!=NULL)
							fPreClose = pTradeDetailData->fPrice;
					}

					//确定资金流向
					int iRes = pHisTradeDetail->CheckFloatDirection(
						ihtd,
						fPreClose,
						fOpenPrice
						);

					//确定大单
					pHisTradeDetail->AddBillIndex(ihtd,iRes,dBigBillValue,true);
				}

				//统计个股各单类数据
				double dVolume0=0,dAmount0=0;
				double dVolume1=0,dAmount1=0;
				double dVolume2=0,dAmount2=0;
				double dVolume3=0,dAmount3=0;
				pHisTradeDetail->StatFloatData(0,dVolume0,dAmount0);
				pHisTradeDetail->StatFloatData(1,dVolume1,dAmount1);
				pHisTradeDetail->StatFloatData(2,dVolume2,dAmount2);
				pHisTradeDetail->StatFloatData(3,dVolume3,dAmount3);
				//sContent = _T("名称\t日期\t大单买量\t大单卖量\t大单流入\t大单流出\t小单买量\t小单卖量\t小单流入\t小单流出");
				//sContent.Format(_T("%s\t%d\t%.4f\t%.4f\t%.4f\t%.4f\t%.4f\t%.4f\t%.4f\t%.4f"),
				//	p->GetName(),
				//	date,
				//	dVolume0,
				//	dVolume1,
				//	dAmount0,
				//	dAmount1,
				//	dVolume2,
				//	dVolume3,
				//	dAmount2,
				//	dAmount3
				//	);
				//excel.PasteTo(sContent,iRow++,1,false);

			dbVolume0 +=dVolume0;
			dbAmount0 +=dAmount0;
			dbVolume1 +=dVolume1;
			dbAmount1 +=dAmount1;
			dbVolume2 +=dVolume2;
			dbAmount2 +=dAmount2;
			dbVolume3 +=dVolume3;
			dbAmount3 +=dAmount3;

			}
		}
		sContent.Format(_T("%s\t%d\t%.4f\t%.4f\t%.4f\t%.4f\t%.4f\t%.4f\t%.4f\t%.4f"),
			sBlockName,
			date,
			dbVolume0,
			dbVolume1,
			dbAmount0,
			dbAmount1,
			dbVolume2,
			dbVolume3,
			dbAmount2,
			dbAmount3
			);
		excel.PasteTo(sContent,iRow++,1,false);
	}
	prw.SetPercent(progId, 1.0);
	sProgressPrompt += _T(",完成!");
	prw.SetText(progId,sProgressPrompt);
	excel.Show();
}
void TxBusiness::StatDetailData(int iCollectionId,int iStart,int iEnd)
{
	std::vector<int> vSample;
	CString sBlockName;
	Collection* pCol = m_pFunctionDataManager->GetCollection(iCollectionId);
	if(pCol==NULL)
		return;
	sBlockName = pCol->GetName();
	pCol->GetCollectionItems(vSample);
	return StatDetailData(vSample,iStart,iEnd,sBlockName);
}
///////////////
//2009-06-29
//周期资金流入流出
bool TxBusiness::GetAmountFlow(
	std::vector<int>& vecSecurityId,	//样本
	int istart_date,					//起始日期
	int iend_date,						//终止日期
	Table_Display& baTable,				//结果
	int iFlag,							//周期类型
	int iMarketid						//交易所
	)
{
	baTable.Clear();
	int col = 0;
	int row = 0;

	//交易实体id
	baTable.AddCol(dtype_int4);
	baTable.SetTitle(col++, _T("交易实体id"));

	//名称
	baTable.AddCol(dtype_val_string);
	baTable.SetTitle(col++, _T("名称"));

	//代码
	baTable.AddCol(dtype_val_string);
	baTable.SetTitle(col++, _T("代码"));

	int start_date,end_date;

	//2008-11-11
	if(vecSecurityId.size()<=0)
	{
		//if(bFocusSamples == true)
			AfxMessageBox(_T("资金流向：样本不能为空!"));
		return false;
	}
	if(istart_date>iend_date)
	{
		AfxMessageBox(_T("起始日期应该小于等于终止日期!"));
		return false;
	}
	//默认为上交所
	if(iMarketid<=0)
	{
		iMarketid = m_pFunctionDataManager->GetBourseId_ShangHai();
	}

	//取得当日日期
	int curDate = m_pFunctionDataManager->GetServerCurDateTime(iMarketid).GetDate().GetInt();
	int curDataDate = m_pFunctionDataManager->GetCurDataDateTime(iMarketid).GetDate().GetInt();
	//取得当日是否交易的标志
	bool bTodayTradeDay = m_pFunctionDataManager->IsTodayTradeDate(iMarketid);

	bool bAddToday = false;
	//处理默认日期
	if(istart_date<=0)
	{
		istart_date = curDate;
		return false;
	}
	if(iend_date<=0)
	{
		iend_date = curDate;
		return false;
	}

	start_date = istart_date;
	end_date = iend_date;

	//DataFileNormal<blk_TxExFile_FileHead,AmountFlow>* pDataFile1=NULL;
	//DataFileNormal<blk_TxExFile_FileHead,AmountFlow>* pDataFile2=NULL;
	bool bFirstOfDuration = false;
	//历史数据
		//时间以大盘为基准
		//样本的资金流入流出数据
		//

	int start_index = m_pShIndex->GetTradeDateIndex(start_date,0,iFlag,bFirstOfDuration);
	if(start_index<0) start_index = 0;
	int end_index = m_pShIndex->GetTradeDateIndex(end_date,0,iFlag,bFirstOfDuration);
	bool bEnd = false;
	if(end_index<0)
	{
		bEnd = true;
		end_index = m_pShIndex->GetTradeDataCount()-1;
	}

	Tx::Core::ProgressWnd prw;
	//step2
	CString sProgressPrompt;
	sProgressPrompt =  _T("资金流向...");
	UINT progId = prw.AddItem(1,sProgressPrompt, 0.0);
	//step3
	prw.Show(15);

	//title
	for(int index = start_index;index<=end_index;index++)
	{
		int date = m_pShIndex->GetTradeDateByIndex(index,iFlag,bFirstOfDuration);
		if(date<start_date) continue;

		CString sDate;
		sDate.Format(_T("%d"),date);
		//流入/流出
		baTable.AddCol(dtype_double);
		baTable.SetTitle(col++, sDate+_T("流入/流出"));

		//净流入
		baTable.AddCol(dtype_double);
		baTable.SetOutputItemRatio(col, 10000);
		baTable.SetFormatStyle(col, Tx::Core::fs_finance);
		baTable.SetTitle(col++, _T("净流入(万元)"));

		//流入
		baTable.AddCol(dtype_double);
		baTable.SetOutputItemRatio(col, 10000);
		baTable.SetFormatStyle(col, Tx::Core::fs_finance);
		baTable.SetTitle(col++, _T("流入(万元)"));

		//流出
		baTable.AddCol(dtype_double);
		baTable.SetOutputItemRatio(col, 10000);
		baTable.SetFormatStyle(col, Tx::Core::fs_finance);
		baTable.SetTitle(col++, _T("流出(万元)"));
	}
	if(bEnd==true)
	{
		CString sDate;
		sDate.Format(_T("%d"),end_date);
		//流入/流出
		baTable.AddCol(dtype_double);
		baTable.SetTitle(col++, sDate+_T("流入/流出"));

		//净流入
		baTable.AddCol(dtype_double);
		baTable.SetOutputItemRatio(col, 10000);
		baTable.SetFormatStyle(col, Tx::Core::fs_finance);
		baTable.SetTitle(col++, _T("净流入(万元)"));

		//流入
		baTable.AddCol(dtype_double);
		baTable.SetOutputItemRatio(col, 10000);
		baTable.SetFormatStyle(col, Tx::Core::fs_finance);
		baTable.SetTitle(col++, _T("流入(万元)"));

		//流出
		baTable.AddCol(dtype_double);
		baTable.SetOutputItemRatio(col, 10000);
		baTable.SetFormatStyle(col, Tx::Core::fs_finance);
		baTable.SetTitle(col++, _T("流出(万元)"));
	}


	double dValuein = Con_doubleInvalid;
	double dValueout = Con_doubleInvalid;
	double dValue = Con_doubleInvalid;
	int iSecurityCount = (int)vecSecurityId.size();
	//循环样本
	for(int i=0;i<iSecurityCount;i++)
	{
		dValuein = Con_doubleInvalid;
		dValueout = Con_doubleInvalid;
		dValue = Con_doubleInvalid;
		//样本id
		int iSecurityId = vecSecurityId[i];
		//样本对象
		SecurityQuotation* pSecurityQuotation = GetSecurityNow(iSecurityId);
		if(pSecurityQuotation==NULL)
		{
#ifdef _DEBUG
			GlobalWatch::_GetInstance()->WatchHere(_T("zhaohj|| TxBusiness::GetAmountFlow invalid id = %d"),iSecurityId);
#endif
			continue;
		}
		//港股不处理
		if(pSecurityQuotation->IsHK_Market()==true)
			continue;

		row = (int)baTable.GetRowCount();
		col = 0;
		//添加一行记录
		baTable.AddRow();

		//securityid
		baTable.SetCell(col++,row,iSecurityId);
		//name
		baTable.SetCell(col++,row,pSecurityQuotation->GetName());
		//code
		baTable.SetCell(col++,row,pSecurityQuotation->GetCode());

		DataFileNormal<blk_TxExFile_FileHead,AmountFlow>* pDataFile = (DataFileNormal<blk_TxExFile_FileHead,AmountFlow>*)pSecurityQuotation->GetDataPointer(Tx::Data::dt_AmountFlow);
		if(pDataFile==NULL) continue;

		//循环输出个周期数据
		AmountFlow* pAmountFlowLast = NULL;
		AmountFlow* pAmountFlow;
		int date = 0;
		//周期累计数据
		double dValueInSum = 0;
		double dValueOutSum = 0;
		for(int index = start_index;index<=end_index;index++)
		{
			//进度条
			prw.SetPercent(progId, (double)(index-start_index)/(double)(end_index-start_index+1));

			//取得日期
			date = m_pShIndex->GetTradeDateByIndex(index,iFlag,bFirstOfDuration);
			//检查日期是否符合阶段要求
			if(date<start_date) continue;

			//根据变化情况修改进度条提示信息
			CString sTitle;
			sTitle.Format(_T("资金流向...%s,%s,%d"),
				pSecurityQuotation->GetName(),
				pSecurityQuotation->GetCode(),
				date
				);
			prw.SetText(progId,sTitle);

			int index1 = pDataFile->GetIndex(date);
			pAmountFlow = pDataFile->GetDataByObj(date);
			if(pAmountFlow==NULL)
				continue;

			if(pAmountFlowLast==NULL)
			{
				//取得上一个
				pAmountFlowLast = pDataFile->GetDataByObj(start_date);
				if(pAmountFlowLast==NULL)
					continue;
			}
			dValueInSum = pAmountFlow->f_Suminflow-pAmountFlowLast->f_Suminflow+pAmountFlowLast->f_inflow;
			dValueOutSum = pAmountFlow->f_SumoutFlow-pAmountFlowLast->f_SumoutFlow+pAmountFlowLast->f_outFlow;
			/*
			if(index1>0)
			{
				if(pAmountFlowLast==NULL)
				{
					//取得上一个
					if(index>0)
					{
						int date1 = m_pShIndex->GetTradeDateByIndex(index-1,iFlag,bFirstOfDuration);
						pAmountFlowLast = pDataFile->GetDataByObj(date1);
						//pAmountFlowLast = pDataFile->GetDataByObj(start_date);

						//
						if(pAmountFlowLast==NULL)
							continue;
						dValueInSum = pAmountFlow->f_Suminflow-pAmountFlowLast->f_Suminflow+pAmountFlowLast->f_inflow;
						dValueOutSum = pAmountFlow->f_SumoutFlow-pAmountFlowLast->f_SumoutFlow+pAmountFlowLast->f_outFlow;
					}
					else
					{
						//可能性极小
						//如果是第一条，直接取值即可
						dValueInSum = pAmountFlow->f_Suminflow;
						dValueOutSum = pAmountFlow->f_SumoutFlow;
					}
				}
				else
				{
					dValueInSum = pAmountFlow->f_Suminflow-pAmountFlowLast->f_Suminflow+pAmountFlowLast->f_inflow;
					dValueOutSum = pAmountFlow->f_SumoutFlow-pAmountFlowLast->f_SumoutFlow+pAmountFlowLast->f_outFlow;
				}
			}
			else
			{
				//如果是第一条，直接取值即可
				dValueInSum = pAmountFlow->f_Suminflow;
				dValueOutSum = pAmountFlow->f_SumoutFlow;
			}
			*/
			if(dValueOutSum>0 || dValueOutSum<0)
				baTable.SetCell(col++,row,dValueInSum/dValueOutSum);
			else
				baTable.SetCell(col++,row,Con_doubleInvalid);

			baTable.SetCell(col++,row,dValueInSum-dValueOutSum);

			baTable.SetCell(col++,row,dValueInSum);
			baTable.SetCell(col++,row,dValueOutSum);

			pAmountFlowLast = pAmountFlow;
		}
		if(date>0 && date<end_date)
		{
			pDataFile->SetReturnWhenInvalidIndex(tag_DI_End);
			pAmountFlow = pDataFile->GetDataByObj(end_date);
			pDataFile->SetReturnWhenInvalidIndex(tag_DI_Begin);

			dValueInSum = pAmountFlow->f_Suminflow-pAmountFlowLast->f_Suminflow+pAmountFlowLast->f_inflow;
			dValueOutSum = pAmountFlow->f_SumoutFlow-pAmountFlowLast->f_SumoutFlow+pAmountFlowLast->f_outFlow;
			if(dValueOutSum>0 || dValueOutSum<0)
				baTable.SetCell(col++,row,dValueInSum/dValueOutSum);
			else
				baTable.SetCell(col++,row,Con_doubleInvalid);

			baTable.SetCell(col++,row,dValueInSum-dValueOutSum);

			baTable.SetCell(col++,row,dValueInSum);
			baTable.SetCell(col++,row,dValueOutSum);
		}
	}

	prw.SetPercent(progId, 1.0);
	sProgressPrompt += _T(",完成!");
	prw.SetText(progId,sProgressPrompt);

	return true;
}
//2009-07-01
//周期资金流入流出
bool	TxBusiness::GetAmountFlow(
		   Table_Display& resTable,			//结果数据表
		   std::vector<int>	&iSecurityId,	//输入样本集
		   int		iStartDate,				//起始日期
		   int		iEndDate,				//终止日期
		   int		iTimeCycle,				//时间周期。日周月季年
		   std::vector<CString> &vDates,	//日期标题
		   std::vector<CString> &vColName	//数据列标题
		   )
{
	//样本为空
	if (iSecurityId.empty())
		return false;

	if(iEndDate<iStartDate)
	{
		AfxMessageBox(_T("起始日期应该小于终止日期!"));
		return false;
	}

	Tx::Core::ProgressWnd prw;
	//step2
	CString sProgressPrompt;
	sProgressPrompt =  _T("周期资金流向...");
	UINT progId = prw.AddItem(1,sProgressPrompt, 0.0);
	//step3
	prw.Show(15);

	int iMarketid = m_pFunctionDataManager->GetBourseId_ShangHai();

	//取得当日日期
	ZkkLib::DateTime dtCur = m_pFunctionDataManager->GetServerCurDateTime(iMarketid);
	int curDate = dtCur.GetDate().GetInt();
	if(curDate == iEndDate)
	{
		ZkkLib::TimeSpan ts(-1, 0, 0,0);
		int iLastDay = (dtCur + ts).GetDate().GetInt();
		iEndDate = iLastDay;
		if (curDate == iStartDate)
		{
			iStartDate = iLastDay;
		}
	}

	//取得周期日期序列
	std::vector<int> iDates;
	m_pShIndex->LoadHisTrade();
	m_pShIndex->LoadTradeDate();
	m_pShIndex->GetDate(iStartDate,iEndDate,iTimeCycle,iDates,0);

	//建立结果数据列=样本id,名称和代码
	resTable.Clear();
	resTable.AddCol(Tx::Core::dtype_int4);			// 0列 不展示
	resTable.AddCol(Tx::Core::dtype_val_string);	// 名称
	resTable.AddCol(Tx::Core::dtype_val_string);	// 代码

	//输入的id，名称，代码
	int fixedCol = 3;
	//每天数据字段数
	int dataCol = 4;

	int iAddDateHead = 0;
	int iAddDateTail = 0;
	int iRealStartDate = iStartDate;
	if((int)iDates.size() < 1)
		return false;
	//检查周期第一天
	if(m_pShIndex->IsTradeDate(iRealStartDate)==true)
	{
		if(iDates[0]>iRealStartDate) iAddDateHead++;
	}
	else
	{
		int istartindex = m_pShIndex->GetTradeDateIndex(iRealStartDate);
		iRealStartDate = m_pShIndex->GetTradeDateByIndex(istartindex+1);
		if(iDates[0]>iRealStartDate) iAddDateHead++;
	}

	//检查周期最后一天
	//if(iDates[iDates.size()-1]<iEndDate) iAddDateTail++;

	//进度条
	prw.SetPercent(progId, 0.1);

	//建立结果数据列=样本数据
	int colDates = dataCol * (iDates.size()+iAddDateTail);

	if(iTimeCycle==0)
		colDates = dataCol * (iDates.size()+iAddDateTail+iAddDateHead);

	UINT iColCount = resTable.GetColCount();
	for (UINT i=0;i<(UINT)colDates;i++)
	{
		resTable.AddCol(Tx::Core::dtype_double);
		resTable.SetFormatStyle(iColCount + i, Tx::Core::fs_finance);
	}

	//根据样本建立数据行
	int iRow = 0;
	for (UINT i=0;i<iSecurityId.size();i++)
	{
		SecurityQuotation* p = (SecurityQuotation*)GetSecurity(iSecurityId[i]);
		if (p == NULL)
			continue;
		resTable.AddRow();
		resTable.SetCell(0,iRow,iSecurityId[i]);
		resTable.SetCell(1,iRow,p->GetName());
		resTable.SetCell(2,iRow,p->GetCode());
		iRow++;
	}
	//进度条
	prw.SetPercent(progId, 0.2);

	//计算请求字符串长度
	int iSize = sizeof(int) * (  1/*1请求类型*/ + 1 /*券个数*/ + resTable.GetRowCount() + 1 /*日期个数*/ + iDates.size());
	iSize += sizeof(int) * iAddDateHead;
	iSize += sizeof(int) * iAddDateTail;

	//分配内存
	LPBYTE pBuffer = new BYTE [iSize];
	if (pBuffer == NULL)
	{
		return false;
		prw.SetPercent(progId, 1.0);
	}

	//初始化
	LPBYTE pWrite = pBuffer;
	memset(pBuffer,0,iSize);

	//写入请求类型
	int iType = 0;
 //enum FundFlowsType
 //   {
 //       All=0,//流入、流出、累计流入、累计流出
 //       Inflow,
 //       Outflow,
 //       SumInflow,
 //       SumOutflow
 //   };
	memcpy_s(pWrite,iSize,&iType,sizeof(int));
	pWrite += sizeof(int);

	//写入样本个数
	int nSecSize = (int)resTable.GetRowCount();
	memcpy_s(pWrite,iSize,&nSecSize,sizeof(int));
	pWrite += sizeof(int);

	//写入样本id
	for (int i=0;i<nSecSize;i++)
	{
		int iId;
		resTable.GetCell(0,i,iId);
		memcpy_s(pWrite,iSize,&iId,sizeof(int));
		pWrite += sizeof(int);
	}

	//写入日期个数
	nSecSize = (int)iDates.size();
	int nSecSize1 = nSecSize+iAddDateTail+iAddDateHead;
	memcpy_s(pWrite,iSize,&nSecSize1,sizeof(int));
	pWrite += sizeof(int);

	if(iAddDateHead>0)
	{
		memcpy_s(pWrite,iSize,&iRealStartDate,sizeof(int));
		pWrite += sizeof(int);
	}
	//写入日期
	for (int i=0;i<nSecSize;i++)
	{
		int dateNode = iDates[i];
		memcpy_s(pWrite,iSize,&dateNode,sizeof(int));
		pWrite += sizeof(int);
	}
	if(iAddDateTail>0)
	{
		memcpy_s(pWrite,iSize,&iEndDate,sizeof(int));
		pWrite += sizeof(int);
	}
	//进度条
	prw.SetPercent(progId, 0.3);

	//请求地址
	//LPCTSTR lpUrl = _T("http://192.168.5.1/FundFlows/Handler.ashx");
	LPCTSTR lpUrl = CORE_GETSYSINFO->GetServerAddr(_T("File"),_T("CycleAmountFloat"));

	//发出请求
	Tx::Drive::Http::CSyncUpload upload;
	int iStart = ::GetTickCount();
	if ( upload.Post(lpUrl, pBuffer, iSize) )
	{
		//进度条
		prw.SetPercent(progId, 0.6);
		//收到请求结果
		int iEnd = ::GetTickCount();
		TRACE(_T("\r\nCycleAmountFloat URL Cost Time %d(ms)\r\n"),iEnd-iStart);
		CONST Tx::Drive::Mem::MemSlice &data = upload.Rsp().Body();
		LPBYTE lpRes = data.DataPtr();
		UINT nRetSize = data.Size();

		//检查请求结果的有效性
		if (nRetSize <= 0)
		{
			prw.SetPercent(progId, 1.0);
			delete pBuffer;
			pBuffer = NULL;
			return false;
		}

		//取得结果数据的字节数长度
		UINT nPreSize = *(reinterpret_cast<UINT*>(lpRes));

		//分配内存准备保存结果数据
		LPBYTE lpData = new BYTE[nPreSize];
		if ( lpData == NULL )
		{
			prw.SetPercent(progId, 1.0);
			delete pBuffer;
			pBuffer = NULL;
			return false;
		}

		//解压缩结果数据
		if ( !Tx::Drive::IO::Zip::CZipWrapper::MemUnZip(lpData, 
			nPreSize, lpRes + sizeof(UINT), nRetSize - sizeof(UINT)) )
		{
			prw.SetPercent(progId, 1.0);
			delete [] lpData;
			delete [] pBuffer;
			pBuffer = NULL;
			lpData = NULL;
			return false;
		}

		//进度条
		prw.SetPercent(progId, 0.8);

		iStart = ::GetTickCount();
		LPBYTE pRecv = lpData;
		//计算数据数
		//数据源列数
		int nDataCols = iDates.size();
		nDataCols += iAddDateHead;
		nDataCols += iAddDateTail;

		//数据输出列数
		int nDataColsOut = iDates.size();
		nDataColsOut += iAddDateTail;

		//数据行数
		int nDataRows = resTable.GetRowCount();

		//保存上期数据
		float dCurInLast=0;
		//当日流出
		float dCurOutLast=0;
		//累计流入
		float dInTotLast=0;
		//累计流出
		float dOutTotLast=0;

		//进度条
		prw.SetPercent(progId, 0.0);

		//循环处理所有数据
		for(UINT j=0;j<resTable.GetRowCount();j++)
		{
			prw.SetPercent(progId, (double)j/(double)resTable.GetRowCount());

			for (int i=0;i < nDataCols; i++)
			{
				//当日流入
				float dCurIn=0;
				memcpy_s(&dCurIn,sizeof(float),pRecv,sizeof(float));
				pRecv += sizeof(float);
				if(dCurIn<0)
					dCurIn = 0;

				//当日流出
				float dCurOut=0;
				memcpy_s(&dCurOut,sizeof(float),pRecv,sizeof(float));
				pRecv += sizeof(float);
				if(dCurOut<0)
					dCurOut = 0;

				//累计流入
				float dInTot=0;
				memcpy_s(&dInTot,sizeof(float),pRecv,sizeof(float));
				pRecv += sizeof(float);
				if(dInTot<0)
					dInTot = 0;

				//累计流出
				float dOutTot=0;
				memcpy_s(&dOutTot,sizeof(float),pRecv,sizeof(float));
				pRecv += sizeof(float);
				if(dOutTot<0)
					dOutTot = 0;

				if(i==0 && iTimeCycle>0 && iAddDateHead>0)
				{
					//第一天为计算基准
					dCurInLast=dCurIn;
					dCurOutLast=dCurOut;
					dInTotLast=dInTot;
					dOutTotLast=dOutTot;
					if(nDataCols>1)//如果只有一天，则直接计算
						continue;
				}
				//周期流入
				double dCycleIn = 0;
				//周期流出
				double dCycleOut = 0;

				//2009-07-08
				//如果只有一天，则取当天；
				//如果是多个日期，第一天为辅助数据；其余均为周期数据
				//所以，第一周期的处理需要加上起始日期的当日资金流数据[阶段处理]
				//之后的每个周期不再需要如此处理；[周期处理]

				//计算输出列
				//int icol = (i>0?i-1:i)*dataCol+fixedCol;
				int icol = i*dataCol+fixedCol;
				if(iTimeCycle>0)
				{
					if(iAddDateHead==0)
						icol = i*dataCol+fixedCol;
					else
						icol = (i>0?i-1:i)*dataCol+fixedCol;
				}
				else
				{
					icol = i*dataCol+fixedCol;
				}

				if(iTimeCycle > 0)
				{
					if(i==0 && iAddDateHead==0)
					{
						dCycleIn = dCurIn;
						dCycleOut = dCurOut;
					}
					else
					{
				if(icol==fixedCol)
				{
					dCycleIn = dInTot-dInTotLast+dCurInLast;
					dCycleOut = dOutTot-dOutTotLast+dCurOutLast;
				}
				else
				{
					dCycleIn = dInTot-dInTotLast;
					dCycleOut = dOutTot-dOutTotLast;
				}
					}
				}
				else
				{
					dCycleIn = dCurIn;
					dCycleOut = dCurOut;
				}

				//周期净流入
				double dCycleInNet = dCycleIn-dCycleOut;

				//周期流入/周期流出
				double dCycleRate = Tx::Core::Con_doubleInvalid;
				if(dCycleOut>0)
					dCycleRate = dCycleIn/dCycleOut;
			
				//输出到Table
				resTable.SetCell(icol++,j,dCycleRate);
				resTable.SetCell(icol++,j,dCycleInNet);
				resTable.SetCell(icol++,j,dCycleIn);
				resTable.SetCell(icol++,j,dCycleOut);

	TRACE("\nin=%.4f,out=%.4f,net in=%.4f,rate=%.4f",
		  dCycleIn,
		  dCycleOut,
		  dCycleInNet,
		  dCycleRate
		  );
				if(iTimeCycle > 0)
				{
					dCurInLast=dCurIn;
					dCurOutLast=dCurOut;
					dInTotLast=dInTot;
					dOutTotLast=dOutTot;
				}
			}
		}
		delete [] lpData;
		lpData = NULL;
		iEnd = ::GetTickCount();
		TRACE(_T("\r\nCycleAmountFloat Parse Result Cost Time %d(ms)\r\n"),iEnd-iStart);
	}
	delete [] pBuffer;
	pBuffer = NULL;
	
	//进度条
	prw.SetPercent(progId, 0.8);
	
	//列名
	vColName.push_back(_T("名称"));
	vColName.push_back(_T("代码"));

	CString tmpStr;
	for(UINT j=0;j<iDates.size();j++)
	{
		tmpStr.Format(_T("流入/流出"));
		vColName.push_back(tmpStr);
		tmpStr.Format(_T("净流入"));
		vColName.push_back(tmpStr);
		tmpStr.Format(_T("流入"));
		vColName.push_back(tmpStr);
		tmpStr.Format(_T("流出"));
		vColName.push_back(tmpStr);

		tmpStr.Format(_T("%d-%d-%d"),iDates[j]/10000,iDates[j]/100%100,iDates[j]%100);
		vDates.push_back(tmpStr);
	}
	if(iAddDateTail>0)
	{
		//补充最后日期
		tmpStr.Format(_T("流入/流出"));
		vColName.push_back(tmpStr);
		tmpStr.Format(_T("净流入"));
		vColName.push_back(tmpStr);
		tmpStr.Format(_T("流入"));
		vColName.push_back(tmpStr);
		tmpStr.Format(_T("流出"));
		vColName.push_back(tmpStr);

		tmpStr.Format(_T("%d-%d-%d"),iEndDate/10000,iEndDate/100%100,iEndDate%100);
		vDates.push_back(tmpStr);
	}
	resTable.SetTitle(0,_T("内码"));
	resTable.SetTitle(1,_T("名称"));
	resTable.SetTitle(2,_T("代码"));


	//设置数据表的标题
	for(int i=0;i<(int)vDates.size();i++)
	{
		int ii = i*4;
		int iii = ii+2;
		//resTable.SetTitle(ii+3,vDates[i]+vColName[iii]);
		resTable.SetTitle(ii+3,vColName[iii]);
		resTable.SetTitle(ii+3+1,vColName[iii+1]);
		resTable.SetTitle(ii+3+2,vColName[iii+2]);
		resTable.SetTitle(ii+3+3,vColName[iii+3]);
	}

	prw.SetPercent(progId, 1.0);
	sProgressPrompt += _T(",完成!");
	prw.SetText(progId,sProgressPrompt);
	return true;
}

void TxBusiness::CalStatusAboutToday(SecurityQuotation * psq, int date, bool & bNeedCheckToday, bool & bNeedAppendToday)
{
	ZkkLib::DateTime tmdate = psq->GetDateTime();
	int curDate = tmdate.GetYear()*10000 + tmdate.GetMonth()*100 + tmdate.GetDay();
	if ( psq->IsTodayTradeDate() && date == curDate)
	{
		bNeedCheckToday = true;
		bNeedAppendToday = true;
	}
}
void TxBusiness::PushBackToday(bool bNeedAppendToday,SecurityQuotation * psq,std::vector<HisTradeData> & m_vecHisTrade)
{
	if(bNeedAppendToday)
	{
		if(psq->IsIndex_TX() && psq->GetAmount()<=0.0000001)//天相的指数，有成交金额才加入
			return;
		if(psq->IsFund_Estimate())
			return;
		HisTradeData today;
		ZkkLib::DateTime tmdate = psq->GetDateTime();
		int curDate = tmdate.GetYear()*10000 + tmdate.GetMonth()*100 + tmdate.GetDay();
		today.Date = curDate;
		today.Preclose = psq->GetPreClosePrice();
		today.Open = psq->GetOpenPrice();
		today.High = psq->GetHighPrice();
		today.Low = psq->GetLowPrice();
		today.Close = psq->GetClosePrice();
		today.Volume = psq->GetVolume();
		today.Amount = psq->GetAmount();
		m_vecHisTrade.push_back(today);
	}
}

CString TxBusiness::GetUserCompanyName(void)
{
	CString ret = _T("");
	CString strName = Tx::Core::UserInfo::GetInstance()->GetUserLoginName();
	CString sUrl = Tx::Core::SystemInfo::GetInstance()->GetServerAddr(_T("Info"),_T("UserInfo"));
	sUrl.Format(_T("%sUserCompany.aspx?userid=%s"), sUrl, strName);
	Tx::Drive::Http::CSyncDnload dn;
	if (dn.Open(sUrl, NULL))
	{
		CONST Tx::Drive::Mem::MemSlice &data = dn.Rsp().Body();
		LPBYTE lpRes = data.DataPtr();
		UINT nRetSize = data.Size();
		int len = nRetSize/2+1;
		if(len<=1)
			return ret;
		wchar_t * p = new wchar_t[len];
		memset(p,0,len*sizeof(wchar_t));
		memcpy(p,lpRes,nRetSize);
		ret = p;
		delete [] p;
	}

	return ret;
}

void TxBusiness::GetBondYTM(std::vector<int> nSecurityIds)
{
	return;
}
void TxBusiness::GetBondYTM(int nSecurityId,double &dYtm,double &dMdur,double &dCon)
{
	return;
}

}
}
