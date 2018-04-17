/**************************************************************
	Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
	File name:		TxStock_HK.h
	Author:			赵宏俊
	Date:			2008-05-12
	Version:		1.0
	Description:	处理港股股票类业务
					某些处理方法可以直接使用TxStock提供的方法
***************************************************************/
#ifndef __TXSTOCK_HK_H__
#define __TXSTOCK_HK_H__
#include "TxStock.h"
#pragma once
namespace Tx
{
	namespace Business
	{
		//港股股票业务类
		class BUSINESS_EXT TxStockHK :
			public TxStock
		{
			DECLARE_DYNCREATE(TxStockHK)
		public:
			TxStockHK(void);
			virtual ~TxStockHK(void);

		public:
				//2008-04-25
				//排序分析-增加列
				virtual bool GetBlockAnalysisCol(Table_Display& baTable,std::vector<int>& arrSamples,int& iSortCol);
				virtual bool SetBlockAnalysisBasicCol(Table_Display& baTable,SecurityQuotation* psq,int& j,int ii,int idate,int iSamplesCount);
				//排序分析数据结果设置
				virtual bool SetBlockAnalysisCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow);
				//排序分析基本数据结果设置[换手率]
				virtual bool SetBlockAnalysisHslCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow);
		};
	}
}
#endif