/**************************************************************
	Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
	File name:		TxFutures.h
	Author:			�Ժ꿡
	Date:			2008-07-29
	Version:		1.0
	Description:	
					�����ڻ���ҵ��
					ĳЩ����������ֱ��ʹ��TxBusiness�ṩ�ķ���
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
	//�ڻ�ÿ����������=��������
	DataFileHisTradeData<blk_TxExFile_FileHead,DAY_HQ_ITEM_FUTURES>* m_pOneDayTradeData;
	DataFileNormal<blk_TxExFile_FileHead,SecurityCookiesDate>* m_pFuturesTradeDate;
	
	//Ŀ¼
	int iFuturesExFileId;
	//�����������ļ�id
	int iFuturesTradeDateIndexFileId;
	//�Ƿ���Ҫreset
	bool m_bNeedReset;
public:
	virtual ~TxFutures(void);
	//�����
	virtual bool BlockAnalysisAddRow(Table_Display& baTable,SecurityQuotation* pSecurity,int idate);
	//����������ݽ��������
	virtual bool GetBlockAnalysisCol(Table_Display& baTable,std::vector<int>& arrSamples,int& iSortCol);
	//��������������ݽ������
	virtual bool SetBlockAnalysisBasicCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow,int idate,int iSamplesCount);
	//��������������ݽ������[������]
	virtual bool SetBlockAnalysisHslCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow);
	//���������չ���ݽ������
	virtual bool SetBlockAnalysisCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow);
private:
	//���ݽ���ʵ��idȷ�����������ļ���id
	bool GetFuturesExFileId(SecurityQuotation* psq);
};
	}
}