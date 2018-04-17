/**************************************************************
	Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
	File name:		TxFutures.cpp
	Author:			赵宏俊
	Date:			2008-07-29
	Version:		1.0
	Description:
					处理期货类业务
					某些处理方法可以直接使用TxBusiness提供的方法
***************************************************************/
#include "StdAfx.h"
#include "TxFutures.h"
namespace Tx
{
	namespace Business
	{

TxFutures::TxFutures(void)
{
	m_pOneDayTradeData = NULL;
	m_pFuturesTradeDate = NULL;
	iFuturesExFileId = -1;
	iFuturesTradeDateIndexFileId = -1;
	m_bNeedReset = false;
}

TxFutures::~TxFutures(void)
{
	if(m_pOneDayTradeData!=NULL)
	{
		delete m_pOneDayTradeData;
		m_pOneDayTradeData = NULL;
	}
	if(m_pFuturesTradeDate!=NULL)
	{
		delete m_pFuturesTradeDate;
		m_pFuturesTradeDate = NULL;
	}
	
}
//排序分析基本数据结果设置[换手率]
bool TxFutures::SetBlockAnalysisHslCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow)
{
	//double dHsl = Con_doubleInvalid;
	//dHsl = pSecurity->GetTradeRate();
	////换手率
	//baTable.SetCell(nCol,nRow,dHsl);
	return true;
}
//添加行
bool TxFutures::BlockAnalysisAddRow(Table_Display& baTable,SecurityQuotation* psq,int idate)
{
	//idate = 20080729;
	if(GetFuturesExFileId(psq)==false)
		return false;

	//确定交易日期
	int iDataCount = m_pFuturesTradeDate->GetDataCount();
	if(iDataCount<=0)
		return false;
	int iDataIndex = m_pFuturesTradeDate->GetIndex(idate);
	if(iDataIndex<0)
		//取最新交易日索引
		iDataIndex = iDataCount-1;
	//取最新交易日
	idate = m_pFuturesTradeDate->GetObj(iDataIndex);
	if(idate<=0)
	{
		//日期校验
		return false;
	}

	//取得终止日期的数据
	if(m_pOneDayTradeData==NULL)
		m_pOneDayTradeData = new DataFileHisTradeData<blk_TxExFile_FileHead,DAY_HQ_ITEM_FUTURES>;
	if(m_pOneDayTradeData==NULL)
	{
		AfxMessageBox(_T("取得指定日期的期货数据时，内存分配失败！"));
		return false;
	}

	if(m_bNeedReset==true)
		m_pOneDayTradeData->Reset();

	m_pOneDayTradeData->SetDownloadMode(2);
	if(m_pOneDayTradeData->Load(
		iFuturesExFileId,
		idate,true)==false)
	{
#ifdef _DEBUG
		GlobalWatch::_GetInstance()->WatchHere(_T("zhaohj|| bool TxBusiness::BlockCycleRateAdv 取得指定日期的横向数据失败"));
#endif
		AfxMessageBox(_T("加载指定日期的数据失败[文件不存在或者数据格式错误]！"));
		return false;
	}

	DAY_HQ_ITEM_FUTURES* pDAY_HQ_ITEM_FUTURES = m_pOneDayTradeData->GetDataByObj(psq->GetId(),false);
	if(pDAY_HQ_ITEM_FUTURES==NULL)
		return false;
	baTable.AddRow();
	return true;
}
//排序分析数据结果标题列
//默认按照股票处理
bool TxFutures::GetBlockAnalysisCol(Table_Display& baTable,std::vector<int>& arrSamples,int& iSortCol)
{
	iSortCol = 3;
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

	//float f_preclose;//     --float 前收
	//float f_open;//  --float 开盘
	//float f_high;//  --float 最高
	//float f_low;//   --float 最高
	//float f_close;// --float 收盘 
	//double f_volume;// --double 交易量
	//double f_amount;// --double  交易额

		//日期
		baTable.AddCol(Tx::Core::dtype_int4);
		//涨幅
		baTable.AddCol(Tx::Core::dtype_double);
		//前收
		baTable.AddCol(Tx::Core::dtype_float);
		//涨跌
		baTable.AddCol(Tx::Core::dtype_double);
		//开盘
		baTable.AddCol(Tx::Core::dtype_float);
		//最高
		baTable.AddCol(Tx::Core::dtype_float);
		//最低
		baTable.AddCol(Tx::Core::dtype_float);
		//收盘
		baTable.AddCol(Tx::Core::dtype_float);

		//成交量
		baTable.AddCol(Tx::Core::dtype_double);
		//成交金额
		baTable.AddCol(Tx::Core::dtype_double);


		int nCol=1;
		baTable.SetTitle(nCol, _T("名称"));
		++nCol;
		baTable.SetTitle(nCol, _T("代码"));
		++nCol;
		baTable.SetTitle(nCol, _T("自选号"));
		++nCol;
		baTable.SetTitle(nCol, _T("日期"));
		baTable.SetFormatStyle(nCol,fs_date);
		++nCol;
		baTable.SetTitle(nCol, _T("涨幅"));
		baTable.SetKeyCol(nCol);
		++nCol;
		baTable.SetTitle(nCol, _T("前收"));
		++nCol;
		baTable.SetTitle(nCol, _T("涨跌"));
		++nCol;
		baTable.SetTitle(nCol, _T("开盘"));
		++nCol;
		baTable.SetTitle(nCol, _T("最高"));
		++nCol;
		baTable.SetTitle(nCol, _T("最低"));
		++nCol;
		baTable.SetTitle(nCol, _T("收盘"));
		++nCol;


		baTable.SetTitle(nCol, _T("成交量(手)"));
		baTable.SetPrecise(nCol, 0);
		//baTable.SetOutputItemRatio(nCol, 100);
		baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
		++nCol;

		baTable.SetTitle(nCol, _T("成交金额(万元)"));
		baTable.SetPrecise(nCol, 0);//cyh80 2009-08-19 改为保留0位小数
		baTable.SetOutputItemRatio(nCol, 10000);
		baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
	}
	return true;
}
//排序分析基本数据结果设置
bool TxFutures::SetBlockAnalysisBasicCol(Table_Display& baTable,SecurityQuotation* psq,int& j,int ii,int idate,int iSamplesCount)
{
	float prePrice	= Con_floatInvalid;
	float closePrice	= Con_floatInvalid;
	float openPrice	= Con_floatInvalid;
	float lowPrice	= Con_floatInvalid;
	float highPrice	= Con_floatInvalid;
	double amount		= Con_doubleInvalid;
	double volume		= Con_doubleInvalid;
	double dRaiseValue = Con_doubleInvalid;
	double dRaise = Con_doubleInvalid;
	double dHsl = Con_doubleInvalid;

#ifdef _DEBUG
	//int iTestId = psq->GetId();
	//bool bn = psq->IsNormal();
	//bool bs = psq->IsStop();
	//bool bh = psq->IsHalt();
	//bool bhl = psq->IsHaltLong();
#endif
	/*
	//idate = 20080729;
	if(GetFuturesExFileId(psq)==false)
		return false;

	//确定交易日期
	int iDataCount = m_pFuturesTradeDate->GetDataCount();
	if(iDataCount<=0)
		return false;
	int iDataIndex = m_pFuturesTradeDate->GetIndex(idate);
	if(iDataIndex<0)
		//取最新交易日索引
		iDataIndex = iDataCount-1;
	//取最新交易日
	idate = m_pFuturesTradeDate->GetObj(iDataIndex);
	if(idate<=0)
	{
		//日期校验
		return false;
	}

	//取得终止日期的数据
	if(m_pOneDayTradeData==NULL)
		m_pOneDayTradeData = new DataFileHisTradeData<blk_TxExFile_FileHead,DAY_HQ_ITEM_FUTURES>;
	if(m_pOneDayTradeData==NULL)
	{
		AfxMessageBox(_T("取得指定日期的期货数据时，内存分配失败！"));
		return false;
	}

	if(m_bNeedReset==true)
		m_pOneDayTradeData->Reset();

	m_pOneDayTradeData->SetDownloadMode(2);
	if(m_pOneDayTradeData->Load(
		iFuturesExFileId,
		idate,true)==false)
	{
#ifdef _DEBUG
		GlobalWatch::_GetInstance()->WatchHere(_T("zhaohj|| bool TxBusiness::BlockCycleRateAdv 取得指定日期的横向数据失败"));
#endif
		AfxMessageBox(_T("加载指定日期的数据失败[文件不存在或者数据格式错误]！"));
		return false;
	}
*/
	CString sName;
	CString sExtCode;
	sName = Con_strInvalid;
	sExtCode = Con_strInvalid;
	sName = psq->GetName();
	sExtCode = psq->GetCode();

//		baTable.SetCell(j,ii,(int)psq->GetId());
//		j++;

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
	//日期
		//2008-12-22
		//期货取序列的最新日期
		idate = Con_intInvalid;
		if(m_pFuturesTradeDate!=NULL)
		{
			int iCount = m_pFuturesTradeDate->GetDataCount();
			if(iCount>0)
				idate = m_pFuturesTradeDate->GetObj(iCount-1);
		}
		baTable.SetCell(j,ii,idate);
		j++;

	DAY_HQ_ITEM_FUTURES* pDAY_HQ_ITEM_FUTURES = m_pOneDayTradeData->GetDataByObj(psq->GetId(),false);
	if(pDAY_HQ_ITEM_FUTURES==NULL)
		return false;
	//涨幅
		if(pDAY_HQ_ITEM_FUTURES->Close > 0 && pDAY_HQ_ITEM_FUTURES->Preclose > 0)
			dRaiseValue = pDAY_HQ_ITEM_FUTURES->Close - pDAY_HQ_ITEM_FUTURES->Preclose;
		else
			dRaiseValue = Con_doubleInvalid;
		dRaise = dRaiseValue;
		if( fabs(dRaise - Con_doubleInvalid) > 0.000001 && pDAY_HQ_ITEM_FUTURES->Preclose > 0)
		{
			dRaise /= pDAY_HQ_ITEM_FUTURES->Preclose;
			dRaise *= 100;
		}
		else
			dRaise = Con_doubleInvalid;
		baTable.SetCell(j,ii,dRaise);
		j++;
		//前收
		baTable.SetCell(j,ii,pDAY_HQ_ITEM_FUTURES->Preclose);
		j++;
		//涨跌
		baTable.SetCell(j,ii,dRaiseValue);
		j++;
		//开盘
		baTable.SetCell(j,ii,pDAY_HQ_ITEM_FUTURES->Open);
		j++;
		//最高
		baTable.SetCell(j,ii,pDAY_HQ_ITEM_FUTURES->High);
		j++;
		//最低
		baTable.SetCell(j,ii,pDAY_HQ_ITEM_FUTURES->Low);
		j++;
		//收盘
		baTable.SetCell(j,ii,pDAY_HQ_ITEM_FUTURES->Close);
		j++;
		//成交量
		baTable.SetCell(j,ii,pDAY_HQ_ITEM_FUTURES->Volume);
		j++;
		//成交金额
		baTable.SetCell(j,ii,pDAY_HQ_ITEM_FUTURES->Amount);
		j++;
	return true;
}
bool TxFutures::SetBlockAnalysisCol(Table_Display& baTable,SecurityQuotation* psq,int& j,int ii)
{
	//if(psq==NULL)
	//	return false;
	////初始化时缓存的最新股票信息
	//double tradeableShare = psq->GetTradableShare();
	//double share = psq->GetTotalShare();
	//double dValue = Con_doubleInvalid;
	////int idate = psq->GetCurDataDate();
	//float closePrice = psq->GetClosePrice(true);

	////流通股本
	//	baTable.SetCell(j,ii,tradeableShare);
	//	j++;
	////总股本
	//	baTable.SetCell(j,ii,share);
	//	j++;
	////流通市值
	//	if(tradeableShare>0 && closePrice>0)
	//		dValue = closePrice*tradeableShare;
	//	else
	//		//流通股本不可能小于0
	//		dValue = Tx::Core::Con_doubleInvalid;
	//	baTable.SetCell(j,ii,dValue);
	//	j++;
	////总市值
	//	if(share>0 && closePrice>0)
	//		dValue = closePrice*share;
	//	else
	//		//流通股本不可能小于0
	//		dValue = Tx::Core::Con_doubleInvalid;
	//	baTable.SetCell(j,ii,dValue);
	//	j++;
	return true;
}
//根据交易实体id确定加载数据文件的id
bool TxFutures::GetFuturesExFileId(SecurityQuotation* psq)
{
	if(psq->IsFutures()==false)
		return false;

	CString sFuturesExFileIdKey;
	CString sFuturesTradeDateIndexFileIdKey;
	if(psq->IsInternational_Market()==true)
	{
		sFuturesExFileIdKey = _T("day_trade_data_futures_inter");
		sFuturesTradeDateIndexFileIdKey = _T("day_trade_data_futures_inter_index");
	}
	else if(psq->IsCN_Market()==true)
	{
		sFuturesExFileIdKey = _T("day_trade_data_futures_inner");
		sFuturesTradeDateIndexFileIdKey = _T("day_trade_data_futures_inner_index");
	}
	else
		return false;
	int iFuturesExFileId_last = iFuturesExFileId;
	int iFuturesTradeDateIndexFileId_last = iFuturesTradeDateIndexFileId;

	//取得国际期货扩展数据文件id
	iFuturesExFileId = 
	Tx::Data::DataStatus::GetInstance()->GetExFileIdFromIni(
		Tx::Core::SystemPath::GetInstance()->GetSystemDataPath(),
		sFuturesExFileIdKey);

	iFuturesTradeDateIndexFileId = 
	Tx::Data::DataStatus::GetInstance()->GetExFileIdFromIni(
		Tx::Core::SystemPath::GetInstance()->GetSystemDataPath(),
		sFuturesTradeDateIndexFileIdKey);

	//加载期货市场交易日期序列
	if(m_pFuturesTradeDate==NULL)
		m_pFuturesTradeDate = new DataFileNormal<blk_TxExFile_FileHead,SecurityCookiesDate>;
	if(	iFuturesExFileId_last != iFuturesExFileId ||
		iFuturesTradeDateIndexFileId_last != iFuturesTradeDateIndexFileId
		)
	{
		m_bNeedReset = true;
		m_pFuturesTradeDate->Reset();
	}
	m_pFuturesTradeDate->Load(
		iFuturesTradeDateIndexFileId,//文件名=2020202.dat
		iFuturesExFileId,//文件所在目录
		true);
	return true;
}
	}
}