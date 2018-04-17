/**************************************************************
	Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
	File name:		TxStock.cpp
	Author:			赵宏俊
	Date:			2007-07-09
	Version:		1.0
	Description:	处理股票类业务
					某些处理方法可以直接使用TxBusiness提供的方法
***************************************************************/
#include "StdAfx.h"
#include "TxStock_HK.h"
#include "..\..\core\core\SystemPath.h"

#include "..\..\core\core\Commonality.h"
#include "..\..\core\driver\TxFileUpdateEngine.h"
#include "MyIndicator.h"

#ifdef _DEBUG
#include "..\..\core\driver\structs_updatefile.h"
#endif

namespace Tx
{
	namespace Business
	{
		IMPLEMENT_DYNCREATE(TxStockHK,TxStock)
			TxStockHK::TxStockHK(void)
		{
			//40000001(股票)
			m_iSecurityTypeId = 40000001;
			//2007-10-29
			//暂时停止该类型的判断
			m_iSecurityTypeId = 0;
		}

		TxStockHK::~TxStockHK(void)
		{
		}

//默认按照股票处理
bool TxStockHK::GetBlockAnalysisCol(Table_Display& baTable,std::vector<int>& arrSamples,int& iSortCol)
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
		baTable.SetPrecise(nCol, 3);
		++nCol;
		baTable.SetTitle(nCol, _T("前收"));
		baTable.SetPrecise(nCol, 3);
		++nCol;
		baTable.SetTitle(nCol, _T("涨跌"));
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
		baTable.SetPrecise(nCol, 0);
		baTable.SetOutputItemRatio(nCol, 10000);
		baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
		++nCol;
	}
	return true;
}
//排序分析基本数据结果设置
bool TxStockHK::SetBlockAnalysisBasicCol(Table_Display& baTable,SecurityQuotation* psq,int& j,int ii,int idate,int iSamplesCount)
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
	//int idate = psq->GetCurDataDate();
	//0.406;0.515
	prePrice	= psq->GetPreClosePrice();
	//0.437;0.422
	//0.062;0.047
	//if(psq->IsValid()==true)
	{
		closePrice	= psq->GetClosePrice(true);
		openPrice	= psq->GetOpenPrice();
		//lowPrice	= psq->GetLowPrice();
		//highPrice	= psq->GetHighPrice();
		amount		= psq->GetAmount();
		volume		= psq->GetVolume(true);
		dRaiseValue = psq->GetRaiseValue();
		dRaise = psq->GetRaise();
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
	//现量
		baTable.SetCell(j,ii,dVolumeNow);
		j++;
	//成交量
		baTable.SetCell(j,ii,volume);
		j++;
	//成交金额
		baTable.SetCell(j,ii,amount);
		j++;
	//换手率
		//baTable.SetCell(j,ii,dHsl);
		SetBlockAnalysisHslCol(baTable,psq,j,ii);
		j++;

	return true;
}
bool TxStockHK::SetBlockAnalysisCol(Table_Display& baTable,SecurityQuotation* psq,int& j,int ii)
{
	return true;
}
//排序分析基本数据结果设置[换手率]
bool TxStockHK::SetBlockAnalysisHslCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow)
{
	return true;
}

	}//end of business
}//end of tx


