/**************************************************************
	Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
	File name:		TxStock.cpp
	Author:			�Ժ꿡
	Date:			2007-07-09
	Version:		1.0
	Description:	�����Ʊ��ҵ��
					ĳЩ����������ֱ��ʹ��TxBusiness�ṩ�ķ���
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
			//40000001(��Ʊ)
			m_iSecurityTypeId = 40000001;
			//2007-10-29
			//��ʱֹͣ�����͵��ж�
			m_iSecurityTypeId = 0;
		}

		TxStockHK::~TxStockHK(void)
		{
		}

//Ĭ�ϰ��չ�Ʊ����
bool TxStockHK::GetBlockAnalysisCol(Table_Display& baTable,std::vector<int>& arrSamples,int& iSortCol)
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

		//2007-08-23
		//���� ����=���³ɽ���
		//������for test
		//ǰ�ա��ּۡ��ǵ����Ƿ��������ʡ��������ɽ������ɽ�����ͨ�ɱ����ܹɱ�����ͨ��ֵ���ܹ���ֵ
		//007����������У�˳��Ϊ���Ƿ����ּۡ�Ǯ�ա��ǵ� 2007-12-12
		baTable.AddCol(Tx::Core::dtype_double);
		baTable.AddCol(Tx::Core::dtype_float);
		baTable.AddCol(Tx::Core::dtype_float);

		baTable.AddCol(Tx::Core::dtype_double);
		baTable.AddCol(Tx::Core::dtype_double);
		baTable.AddCol(Tx::Core::dtype_double);
		baTable.AddCol(Tx::Core::dtype_double);

		int nCol=1;
		baTable.SetTitle(nCol, _T("����"));
		++nCol;
		baTable.SetTitle(nCol, _T("����"));
		++nCol;
		baTable.SetTitle(nCol, _T("��ѡ��"));
		++nCol;
		baTable.SetTitle(nCol, _T("�Ƿ�"));
		baTable.SetKeyCol(nCol);
		++nCol;
		baTable.SetTitle(nCol, _T("�ּ�"));
		baTable.SetPrecise(nCol, 3);
		++nCol;
		baTable.SetTitle(nCol, _T("ǰ��"));
		baTable.SetPrecise(nCol, 3);
		++nCol;
		baTable.SetTitle(nCol, _T("�ǵ�"));
		++nCol;
		baTable.SetTitle(nCol, _T("����(��)"));
		baTable.SetPrecise(nCol, 0);
		//baTable.SetOutputItemRatio(nCol, 100);
		baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
		++nCol;
		baTable.SetTitle(nCol, _T("�ɽ���(��)"));
		baTable.SetPrecise(nCol, 0);
		//baTable.SetOutputItemRatio(nCol, 100);
		baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
		++nCol;
		baTable.SetTitle(nCol, _T("�ɽ����(��Ԫ)"));
		baTable.SetPrecise(nCol, 0);
		baTable.SetOutputItemRatio(nCol, 10000);
		baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
		++nCol;
	}
	return true;
}
//��������������ݽ������
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
	//����
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
	//�Ƿ�
		baTable.SetCell(j,ii,dRaise);
		j++;
	//�ּ�
		baTable.SetCell(j,ii,closePrice);
		j++;
	//ǰ��
		baTable.SetCell(j,ii,prePrice);
		j++;
	//�ǵ�
		baTable.SetCell(j,ii,dRaiseValue);
		j++;
	//����
		baTable.SetCell(j,ii,dVolumeNow);
		j++;
	//�ɽ���
		baTable.SetCell(j,ii,volume);
		j++;
	//�ɽ����
		baTable.SetCell(j,ii,amount);
		j++;
	//������
		//baTable.SetCell(j,ii,dHsl);
		SetBlockAnalysisHslCol(baTable,psq,j,ii);
		j++;

	return true;
}
bool TxStockHK::SetBlockAnalysisCol(Table_Display& baTable,SecurityQuotation* psq,int& j,int ii)
{
	return true;
}
//��������������ݽ������[������]
bool TxStockHK::SetBlockAnalysisHslCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow)
{
	return true;
}

	}//end of business
}//end of tx


