/**************************************************************
	Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
	File name:		TxFutures.h
	Author:			赵宏俊
	Date:			2008-07-29
	Version:		1.0
	Description:	
					处理期货类业务
					某些处理方法可以直接使用TxBusiness提供的方法
***************************************************************/
#pragma once

#include "TxBusiness.h"
namespace Tx
{
	namespace Business
	{

class BUSINESS_EXT TxFutures :
	public TxBusiness
{
public:
	TxFutures(void);
private:
	//2008-07-31
	//期货每日行情数据=横向数据
	DataFileHisTradeData<blk_TxExFile_FileHead,DAY_HQ_ITEM_FUTURES>* m_pOneDayTradeData;
	DataFileNormal<blk_TxExFile_FileHead,SecurityCookiesDate>* m_pFuturesTradeDate;
	
	//目录
	int iFuturesExFileId;
	//交易日索引文件id
	int iFuturesTradeDateIndexFileId;
	//是否需要reset
	bool m_bNeedReset;
public:
	virtual ~TxFutures(void);
	//添加行
	virtual bool BlockAnalysisAddRow(Table_Display& baTable,SecurityQuotation* pSecurity,int idate);
	//排序分析数据结果标题列
	virtual bool GetBlockAnalysisCol(Table_Display& baTable,std::vector<int>& arrSamples,int& iSortCol);
	//排序分析基本数据结果设置
	virtual bool SetBlockAnalysisBasicCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow,int idate,int iSamplesCount);
	//排序分析基本数据结果设置[换手率]
	virtual bool SetBlockAnalysisHslCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow);
	//排序分析扩展数据结果设置
	virtual bool SetBlockAnalysisCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow);
private:
	//根据交易实体id确定加载数据文件的id
	bool GetFuturesExFileId(SecurityQuotation* psq);
};
	}
}