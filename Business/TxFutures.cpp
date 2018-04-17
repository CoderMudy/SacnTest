/**************************************************************
	Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
	File name:		TxFutures.cpp
	Author:			�Ժ꿡
	Date:			2008-07-29
	Version:		1.0
	Description:
					�����ڻ���ҵ��
					ĳЩ����������ֱ��ʹ��TxBusiness�ṩ�ķ���
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
//��������������ݽ������[������]
bool TxFutures::SetBlockAnalysisHslCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow)
{
	//double dHsl = Con_doubleInvalid;
	//dHsl = pSecurity->GetTradeRate();
	////������
	//baTable.SetCell(nCol,nRow,dHsl);
	return true;
}
//�����
bool TxFutures::BlockAnalysisAddRow(Table_Display& baTable,SecurityQuotation* psq,int idate)
{
	//idate = 20080729;
	if(GetFuturesExFileId(psq)==false)
		return false;

	//ȷ����������
	int iDataCount = m_pFuturesTradeDate->GetDataCount();
	if(iDataCount<=0)
		return false;
	int iDataIndex = m_pFuturesTradeDate->GetIndex(idate);
	if(iDataIndex<0)
		//ȡ���½���������
		iDataIndex = iDataCount-1;
	//ȡ���½�����
	idate = m_pFuturesTradeDate->GetObj(iDataIndex);
	if(idate<=0)
	{
		//����У��
		return false;
	}

	//ȡ����ֹ���ڵ�����
	if(m_pOneDayTradeData==NULL)
		m_pOneDayTradeData = new DataFileHisTradeData<blk_TxExFile_FileHead,DAY_HQ_ITEM_FUTURES>;
	if(m_pOneDayTradeData==NULL)
	{
		AfxMessageBox(_T("ȡ��ָ�����ڵ��ڻ�����ʱ���ڴ����ʧ�ܣ�"));
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
		GlobalWatch::_GetInstance()->WatchHere(_T("zhaohj|| bool TxBusiness::BlockCycleRateAdv ȡ��ָ�����ڵĺ�������ʧ��"));
#endif
		AfxMessageBox(_T("����ָ�����ڵ�����ʧ��[�ļ������ڻ������ݸ�ʽ����]��"));
		return false;
	}

	DAY_HQ_ITEM_FUTURES* pDAY_HQ_ITEM_FUTURES = m_pOneDayTradeData->GetDataByObj(psq->GetId(),false);
	if(pDAY_HQ_ITEM_FUTURES==NULL)
		return false;
	baTable.AddRow();
	return true;
}
//����������ݽ��������
//Ĭ�ϰ��չ�Ʊ����
bool TxFutures::GetBlockAnalysisCol(Table_Display& baTable,std::vector<int>& arrSamples,int& iSortCol)
{
	iSortCol = 3;
	//���UIû����������Ϣ���������
	if(baTable.GetColCount()<=0)
	{
		//��������for test
		//����ʵ��ID
		baTable.AddCol(Tx::Core::dtype_int4);
		//����ʵ������
		baTable.AddCol(Tx::Core::dtype_val_string);
		//����ʵ������
		baTable.AddCol(Tx::Core::dtype_val_string);
		//2008-02-19
		//����ԭʼ˳��
		baTable.AddCol(Tx::Core::dtype_int4);

	//float f_preclose;//     --float ǰ��
	//float f_open;//  --float ����
	//float f_high;//  --float ���
	//float f_low;//   --float ���
	//float f_close;// --float ���� 
	//double f_volume;// --double ������
	//double f_amount;// --double  ���׶�

		//����
		baTable.AddCol(Tx::Core::dtype_int4);
		//�Ƿ�
		baTable.AddCol(Tx::Core::dtype_double);
		//ǰ��
		baTable.AddCol(Tx::Core::dtype_float);
		//�ǵ�
		baTable.AddCol(Tx::Core::dtype_double);
		//����
		baTable.AddCol(Tx::Core::dtype_float);
		//���
		baTable.AddCol(Tx::Core::dtype_float);
		//���
		baTable.AddCol(Tx::Core::dtype_float);
		//����
		baTable.AddCol(Tx::Core::dtype_float);

		//�ɽ���
		baTable.AddCol(Tx::Core::dtype_double);
		//�ɽ����
		baTable.AddCol(Tx::Core::dtype_double);


		int nCol=1;
		baTable.SetTitle(nCol, _T("����"));
		++nCol;
		baTable.SetTitle(nCol, _T("����"));
		++nCol;
		baTable.SetTitle(nCol, _T("��ѡ��"));
		++nCol;
		baTable.SetTitle(nCol, _T("����"));
		baTable.SetFormatStyle(nCol,fs_date);
		++nCol;
		baTable.SetTitle(nCol, _T("�Ƿ�"));
		baTable.SetKeyCol(nCol);
		++nCol;
		baTable.SetTitle(nCol, _T("ǰ��"));
		++nCol;
		baTable.SetTitle(nCol, _T("�ǵ�"));
		++nCol;
		baTable.SetTitle(nCol, _T("����"));
		++nCol;
		baTable.SetTitle(nCol, _T("���"));
		++nCol;
		baTable.SetTitle(nCol, _T("���"));
		++nCol;
		baTable.SetTitle(nCol, _T("����"));
		++nCol;


		baTable.SetTitle(nCol, _T("�ɽ���(��)"));
		baTable.SetPrecise(nCol, 0);
		//baTable.SetOutputItemRatio(nCol, 100);
		baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
		++nCol;

		baTable.SetTitle(nCol, _T("�ɽ����(��Ԫ)"));
		baTable.SetPrecise(nCol, 0);//cyh80 2009-08-19 ��Ϊ����0λС��
		baTable.SetOutputItemRatio(nCol, 10000);
		baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
	}
	return true;
}
//��������������ݽ������
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

	//ȷ����������
	int iDataCount = m_pFuturesTradeDate->GetDataCount();
	if(iDataCount<=0)
		return false;
	int iDataIndex = m_pFuturesTradeDate->GetIndex(idate);
	if(iDataIndex<0)
		//ȡ���½���������
		iDataIndex = iDataCount-1;
	//ȡ���½�����
	idate = m_pFuturesTradeDate->GetObj(iDataIndex);
	if(idate<=0)
	{
		//����У��
		return false;
	}

	//ȡ����ֹ���ڵ�����
	if(m_pOneDayTradeData==NULL)
		m_pOneDayTradeData = new DataFileHisTradeData<blk_TxExFile_FileHead,DAY_HQ_ITEM_FUTURES>;
	if(m_pOneDayTradeData==NULL)
	{
		AfxMessageBox(_T("ȡ��ָ�����ڵ��ڻ�����ʱ���ڴ����ʧ�ܣ�"));
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
		GlobalWatch::_GetInstance()->WatchHere(_T("zhaohj|| bool TxBusiness::BlockCycleRateAdv ȡ��ָ�����ڵĺ�������ʧ��"));
#endif
		AfxMessageBox(_T("����ָ�����ڵ�����ʧ��[�ļ������ڻ������ݸ�ʽ����]��"));
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

	//��������
		baTable.SetCell(j,ii,sName);
		j++;
		//0.597;0.620
		//return true;
	//��������
		baTable.SetCell(j,ii,sExtCode);
		j++;
		//2008-02-19
		//ԭʼ˳��
		j++;
	//����
		//2008-12-22
		//�ڻ�ȡ���е���������
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
	//�Ƿ�
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
		//ǰ��
		baTable.SetCell(j,ii,pDAY_HQ_ITEM_FUTURES->Preclose);
		j++;
		//�ǵ�
		baTable.SetCell(j,ii,dRaiseValue);
		j++;
		//����
		baTable.SetCell(j,ii,pDAY_HQ_ITEM_FUTURES->Open);
		j++;
		//���
		baTable.SetCell(j,ii,pDAY_HQ_ITEM_FUTURES->High);
		j++;
		//���
		baTable.SetCell(j,ii,pDAY_HQ_ITEM_FUTURES->Low);
		j++;
		//����
		baTable.SetCell(j,ii,pDAY_HQ_ITEM_FUTURES->Close);
		j++;
		//�ɽ���
		baTable.SetCell(j,ii,pDAY_HQ_ITEM_FUTURES->Volume);
		j++;
		//�ɽ����
		baTable.SetCell(j,ii,pDAY_HQ_ITEM_FUTURES->Amount);
		j++;
	return true;
}
bool TxFutures::SetBlockAnalysisCol(Table_Display& baTable,SecurityQuotation* psq,int& j,int ii)
{
	//if(psq==NULL)
	//	return false;
	////��ʼ��ʱ��������¹�Ʊ��Ϣ
	//double tradeableShare = psq->GetTradableShare();
	//double share = psq->GetTotalShare();
	//double dValue = Con_doubleInvalid;
	////int idate = psq->GetCurDataDate();
	//float closePrice = psq->GetClosePrice(true);

	////��ͨ�ɱ�
	//	baTable.SetCell(j,ii,tradeableShare);
	//	j++;
	////�ܹɱ�
	//	baTable.SetCell(j,ii,share);
	//	j++;
	////��ͨ��ֵ
	//	if(tradeableShare>0 && closePrice>0)
	//		dValue = closePrice*tradeableShare;
	//	else
	//		//��ͨ�ɱ�������С��0
	//		dValue = Tx::Core::Con_doubleInvalid;
	//	baTable.SetCell(j,ii,dValue);
	//	j++;
	////����ֵ
	//	if(share>0 && closePrice>0)
	//		dValue = closePrice*share;
	//	else
	//		//��ͨ�ɱ�������С��0
	//		dValue = Tx::Core::Con_doubleInvalid;
	//	baTable.SetCell(j,ii,dValue);
	//	j++;
	return true;
}
//���ݽ���ʵ��idȷ�����������ļ���id
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

	//ȡ�ù����ڻ���չ�����ļ�id
	iFuturesExFileId = 
	Tx::Data::DataStatus::GetInstance()->GetExFileIdFromIni(
		Tx::Core::SystemPath::GetInstance()->GetSystemDataPath(),
		sFuturesExFileIdKey);

	iFuturesTradeDateIndexFileId = 
	Tx::Data::DataStatus::GetInstance()->GetExFileIdFromIni(
		Tx::Core::SystemPath::GetInstance()->GetSystemDataPath(),
		sFuturesTradeDateIndexFileIdKey);

	//�����ڻ��г�������������
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
		iFuturesTradeDateIndexFileId,//�ļ���=2020202.dat
		iFuturesExFileId,//�ļ�����Ŀ¼
		true);
	return true;
}
	}
}