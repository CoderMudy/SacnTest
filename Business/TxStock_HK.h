/**************************************************************
	Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
	File name:		TxStock_HK.h
	Author:			�Ժ꿡
	Date:			2008-05-12
	Version:		1.0
	Description:	����۹ɹ�Ʊ��ҵ��
					ĳЩ����������ֱ��ʹ��TxStock�ṩ�ķ���
***************************************************************/
#ifndef __TXSTOCK_HK_H__
#define __TXSTOCK_HK_H__
#include "TxStock.h"
#pragma once
namespace Tx
{
	namespace Business
	{
		//�۹ɹ�Ʊҵ����
		class BUSINESS_EXT TxStockHK :
			public TxStock
		{
			DECLARE_DYNCREATE(TxStockHK)
		public:
			TxStockHK(void);
			virtual ~TxStockHK(void);

		public:
				//2008-04-25
				//�������-������
				virtual bool GetBlockAnalysisCol(Table_Display& baTable,std::vector<int>& arrSamples,int& iSortCol);
				virtual bool SetBlockAnalysisBasicCol(Table_Display& baTable,SecurityQuotation* psq,int& j,int ii,int idate,int iSamplesCount);
				//����������ݽ������
				virtual bool SetBlockAnalysisCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow);
				//��������������ݽ������[������]
				virtual bool SetBlockAnalysisHslCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow);
		};
	}
}
#endif