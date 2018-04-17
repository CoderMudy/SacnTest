/**************************************************************
Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
File name:		TxBusiness.h
Author:			�Ժ꿡
Date:			2007-07-03
Version:		1.0
Description:	ҵ����Ļ���
���д�����ཻ��ʵ����й�ͬ���Ե�ҵ��
Ϊ���ཻ��ʵ���ṩ����֧��
***************************************************************/
#ifndef __TXBUSINESS_H__
#define __TXBUSINESS_H__

#pragma once

#include <math.h>
#include "Business.h"
#include "..\..\Data\SecurityAPI.h"
#include "..\..\Data\Indicator.h"
#include "..\..\Data\SecurityBase.h"
#include "..\..\Data\SecurityQuotation.h"
#include "..\..\Data\FunctionDataManager.h"
#include "..\..\Data\LogicalBusiness.h"
#include "..\..\Data\Collection.h"
#include "..\..\core\driver\Table_Display.h"
#include "..\..\core\driver\Table_Indicator.h"
#include "..\..\core\core\DataType.h"
#include "..\..\Data\BasicInterrest.h"
#include "..\..\core\core\progresswnd.h"
#include "..\..\core\core\HourglassWnd.h"
#include "..\..\core\TxMath\Stat_Math.h"
#include "..\..\Data\TypeMapManage.h"
#include "..\..\core\core\Commonality.h"
#include "MixTradeDay.h"

using namespace Tx::Core;
using namespace Tx::Data;

namespace Tx
{
	namespace Business
	{
		//ҵ��������
		//�����ݲ�ȡ������
		//�����Ҫ����,��ȡ�õ����ݸ���ָ��Ҫ����м���
		//���������ύ�����ܴ���

		//RUNTIME_CLASS( TxBusiness ) == this->GetRuntimeClass() 
		//IsKindOf(RUNTIME_CLASS( TxBusiness ))
		class BUSINESS_EXT TxBusiness : public CObject  
		{
			DECLARE_DYNCREATE(TxBusiness)
			//������
		public:
			TxBusiness(void);
			virtual ~TxBusiness(void);

			//�ڴ���
		public:
			void	Clear(void)
			{
				if(m_pFunctionDataManager != NULL)
					delete m_pFunctionDataManager;
				m_pFunctionDataManager = NULL;

				if(m_pLogicalBusiness != NULL)
					delete m_pLogicalBusiness;
				m_pLogicalBusiness = NULL;

				if(m_pMixTradeDay!=NULL)
					delete m_pMixTradeDay;
				m_pMixTradeDay = NULL;
			}

			//ҵ����
		public:

			//ȡ���û�����Ӧ�Ĺ�˾���ơ���ʱ�Ķ�����������ȥ���������
			static CString GetUserCompanyName(void);
			//2009-06-25
			double GetBigBillValue(SecurityQuotation* p,int date);
			void StatDetailData(std::vector<int>& vSample,int iStart,int iEnd,CString sBlockName=_T("���"));
			void StatDetailData(int iCollectionId,int iStart,int iEnd);

			//2009-07-01
			//�����ʽ���������
			bool GetAmountFlow(
				Table_Display& resTable,			//������ݱ�
				std::vector<int>	&iSecurityId,	//����������
				int		iStartDate,				//��ʼ����
				int		iEndDate,				//��ֹ����
				int		iTimeCycle,				//ʱ�����ڡ������¼���
				std::vector<CString> &vDates,	//���ڱ���
				std::vector<CString> &vColName	//�����б���
				);

			//2009-06-29
			//�����ʽ���������
			bool GetAmountFlow(
				std::vector<int>& vecSecurityId,	//����
				int istart_date,					//��ʼ����
				int iend_date,						//��ֹ����
				Table_Display& baTable,				//���
				int iFlag,							//��������
				int iMarketid						//������
				);

			bool GetBlockAmountFlow(
				std::vector<int>& vecSecurityId,
				int istart_date,
				int iend_date,
				Table_Display& baTable,
				bool bAddSamplesOnly = false,
				bool bStat = true,
				bool bDurRaise = true,
				int iMarketid = 0,
				bool bFocusSamples = true);

			//2008-10-29
			//ʵʱ�ʽ������������
			//ȡ��ָ��ʵʱ�ʽ���������
			bool GetAmountFlow(int iIndexId,double& dAmountIn,double& dAmountOut);
			//ȡ��ָ��������ʵʱ�ʽ��������������
			bool GetAmountFlow(std::vector<int>& vecSecurityId,double& dAmountIn,double& dAmountOut);
			//2008-09-05
			//��ʷ�ʽ���������
			bool GetAmountFlow(
				std::vector<int>& vecSecurityId,
				int start_date,
				int end_date,
				Table_Display& baTable,
				bool bAddSamplesOnly=false,
				bool bStat=true,
				bool bDurRaise=true,
				int iMarketid=0,
				bool bFocusSamples=true
				);
			bool GetAmountFlow(
				int start_date,
				int end_date,
				Table_Display& baTable,
				bool bAddSamplesOnly=false,
				bool bStat=true,
				int iMarketid=0
				);

			//2008-04-14
			//ȡ��ָ�������ǵ�����
			bool GetIndexSamplesStatus(int iIndexSecurityId,int& iUp,int& iDown,int& iEqual);
			bool GetIndexSamplesStatus(SecurityQuotation* p,int& iUp,int& iDown,int& iEqual);
			bool GetIndexSamplesStatus(SecurityQuotation* p,int& iUp,int& iDown,int& iEqual,int& iTop,int& iBottom,int& iHalt,bool bGetHalt=false);

			//��ʷ����

			//ȡ��ָ������ʵ�����ʷ��������
			bool GetSecurityHisTrade(int iSecurityId);
			bool GetSecurityHisTrade(Security* p);

			//ȡ��ָ������ʵ��Ľ�����������
			bool GetSecurityTradeDate(int iSecurityId);
			bool GetSecurityTradeDate(Security* p);

			//�ļ�������
			//����ָ���Ľ���ʵ������͹��ܴ���,����XML�ļ�������
			CString DownloadXmlFile(int iSecurityId,int iFunctionId);

			//ָ��
			//2007-10-24
			//��������������Ա��Ҫ���������ʹ��ϰ�ߣ����޸�
			IndicatorData* GetIndicatorDataNow(long iIndicatorId)
			{
				m_pIndicatorData = (IndicatorData*)GetIndicatorData(iIndicatorId);
				return m_pIndicatorData;
			}

			//����ʵ��
			//2007-10-24
			//��������������Ա��Ҫ���������ʹ��ϰ�ߣ����޸�
			SecurityQuotation* GetSecurityNow(long iSecurityId)
			{
				m_pSecurity = (SecurityQuotation*)GetSecurity(iSecurityId);
				return m_pSecurity;
			}

			SecurityQuotation * GetStockAByInstitutionId(long lInstitutionId) const
			{				
				return  (SecurityQuotation *)::TXGetStockAByInstitutionId(lInstitutionId);
			}

			//2008-03-19
			//ɾ��ָ������ʵ��
			bool RemoveSecurityNow(int iSecurityId)
			{
				return RemoveSecurity(iSecurityId);
			}

			//����ʵ��ȯ���ж�
			//�������������жϽ���ʵ���ȯ������
			//�ݹ�ģʽ����ȫ֧�ּ�����
			bool	IsValidSecurity(void)
			{
				if(m_pSecurity==NULL || m_pFunctionDataManager==NULL)
					return false;
				std::set<int> items;
				if(m_pFunctionDataManager->GetItems(m_iSecurityTypeId,items)==false)
					return false;
				if(items.find(m_pSecurity->GetId())!=items.end())
					return true;
				items.clear();
				return false;
			}
			//���ģʽ��֧�ּ��������Ѿ�����ȫ���еļ��Ͻڵ�
			bool	IsValidSecurity1(void)
			{
				if(m_pSecurity==NULL || m_pFunctionDataManager==NULL)
					return false;
				std::set<int> items;
				if(m_pFunctionDataManager->GetItemsFromSecurity(m_iSecurityTypeId,items)==false)
					return false;
				if(items.find(m_pSecurity->GetId())!=items.end())
					return true;
				items.clear();
				return false;
			}


			//ȡ�ü�������
			//ר�����֤�����ҵ��������ҵ
			bool	GetCollectionItems(int iCollectionId,std::set<int>& items)
			{
				return m_pFunctionDataManager->GetItemsFromSecurity(iCollectionId,items);
			}
			bool	GetCollectionItems(CString sCollectionName,std::set<int>& items)
			{
				return m_pFunctionDataManager->GetItemsFromSecurity(sCollectionName,items);
			}
			//���������ļ���Ҳ�ֽڵ��Keyȡ������
			bool	GetLeafItems(CString sLeafName,std::vector<int>& items)
			{
				return m_pFunctionDataManager->GetLeafItems(items,sLeafName);
			}
			//2009-03-11
			//��������
			bool SelectItems(
				SecurityQuotation* p,
				bool bSt,		//ST
				bool bStop,		//ժ��
				bool bHalt,		//����ͣ��
				bool bIssued	//�ѷ���δ����
				);
			bool SelectItems(
				std::vector<int>& src,
				std::vector<int>& dst,
				bool bSt,		//ST
				bool bStop,		//ժ��
				bool bHalt,		//����ͣ��
				bool bIssued	//�ѷ���δ����
				);
			bool SelectItems(
				std::set<int>& src,
				std::set<int>& dst,
				bool bSt,		//ST
				bool bStop,		//ժ��
				bool bHalt,		//����ͣ��
				bool bIssued	//�ѷ���δ����
				);
			bool SelectItems(
				std::vector<int>& src,
				std::set<int>& dst,
				bool bSt,		//ST
				bool bStop,		//ժ��
				bool bHalt,		//����ͣ��
				bool bIssued	//�ѷ���δ����
				);
			bool SelectItems(
				std::set<int>& src,
				std::vector<int>& dst,
				bool bSt,		//ST
				bool bStop,		//ժ��
				bool bHalt,		//����ͣ��
				bool bIssued	//�ѷ���δ����
				);


			//2008-09-25
			//ȡ��ָ������id����������
			//��Ի���,ȯ,����ʵ������ӳ���ϵר�ýӿ�
			//���bIsSecurityId=true��ʾ�����ǽ���ʵ��,bIsSecurityId=false��ʾ������ȯ
			bool	GetItemsByInstitution(
				int iInstitutionId,			//����id
				std::vector<int>& items,	//����
				bool bIsSecurityId=true		//�Ƿ���ʵ������
				);
			//ȡ��ָ��ȯid�����Ľ���ʵ������
			//���ȯ,����ʵ��ӳ���ϵר�ýӿڣ�����ȯ������
			bool	GetItemsBySecurity(
				int iSecurity1Id,			//ȯid
				std::vector<int>& items	//����
				);
			//2008-10-17
			//��������
			bool GetSecurityBySaleAgency(int iSaleAgencyId,std::vector<int>& items);

			//ͳ��׼��=�������
			bool AddSampleBeforeStat(
				int&	iCol,				//������ֵ
				int		iStartDate,			//ͳ��ʱ���������ʼ����
				int		iEndDate,			//ͳ��ʱ���������ֹ����
				std::set<int>& iSecurity	//������
				);
			//ͳ��׼��=���ָ��
			bool AddIndicatorBeforeStat(
				int		iIndicator,			//ͳ��ָ��
				int		iStartDateIndex,	//ͳ��ʱ���������ʼ���ڵĲ���������ֵ
				int		iEndDateIndex		//ͳ��ʱ���������ֹ���ڵĲ���������ֵ
				);

			//���ڰ��ҵ���ܾ��й��ԣ�������ҵ����Ļ�����ʵ��
			/*
			2008-01-09
			GetBlockAnalysis	Ϊ������Ŀ�ܽӿڣ�ֻ����������������ݴ�������
			GetBlockAnalysisCol ������������Ľ�����ݵı�������Ϣ�Ľ���,������ʵ����������Ϣ�������ﴦ��
			SetBlockAnalysisBasicCol ������ֽ���ʵ�干��ָ������ݴ���,���ֽ���ʵ��Ļ���ָ�걣�ֲ���
			SetBlockAnalysisCol	������ֽ���ʵ������ָ������ݴ���,�������ﴦ��

			//���ӿ��Ѿ�����Ĭ�����ݵĴ���
			*/
			//���=�������
			//baTable�Ѿ����������[����ʵ��]
			//virtual bool GetBlockAnalysis(Table_Display& baTable,std::set<int>& arrSamples,bool bNeedProgressWnd=false);
			virtual bool GetBlockAnalysis(Table_Display& baTable,std::vector<int>& arrSamples,int& iSortCol,int iMarketId=0,bool bNeedProgressWnd=false,int nCollID=-1);
			//�����
			virtual bool BlockAnalysisAddRow(Table_Display& baTable,SecurityQuotation* pSecurity,int idate);
			//����������ݽ��������
			virtual bool GetBlockAnalysisCol(Table_Display& baTable,std::vector<int>& arrSamples,int& iSortCol);
			//��������������ݽ������
			virtual bool SetBlockAnalysisBasicCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow,int idate,int iSamplesCount);
			//��������������ݽ������[������]
			virtual bool SetBlockAnalysisHslCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow);
			//����������ݽ������
			virtual bool SetBlockAnalysisCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow);

			//��������
			//bool SetBlockSamples(Table_Display& baTable,std::set<int>& arrSamples,int iCol);
			bool SetBlockSamples(Table_Display& baTable,std::vector<int>& arrSamples,int iCol,int iInitNo);

			//2008-07-29
			//��������
			virtual bool GetTradeData(Table_Display& baTable,int iSecurityId,int iFlag=0,bool bFirst=false,int iWeight=-1,bool bNeedProgressWnd=false);
			//�������ݱ������Ϣ
			virtual bool SetTableCol(Table_Display& baTable);
			//�������ݽ��
			virtual bool SetTableData(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow,int idate,int iFlag=0,bool bFirst=false,int iWeight=-1);
			//���ù�Ʊ���ݱ������Ϣ
			bool SetStockTableCol(Table_Display& baTable,SecurityQuotation* pSecurity);
			//ȡ�ù�Ʊ��������
			bool GetStockTradeData(Table_Display& baTable,
				int iSecurityId,
				int iStartdate,
				int iEndDate,
				int iFlag=0,
				bool bFirst=false,
				int iWeight=-1,
				bool bNeedProgressWnd=false);

			//ȡ�û���ֵ����
			bool GetFundNavData(Table_Display& baTable,
				int iSecurityId,
				int iStartdate,
				int iEndDate,
				int iFlag=0,
				bool bFirst=false,
				int iWeight=-1,
				bool bNeedProgressWnd=false);

			//����ָ�����ݱ������Ϣ
			bool SetIndexTableCol(Table_Display& baTable);
			//ȡ��ָ����������
			bool GetIndexTradeData(Table_Display& baTable,
				int iSecurityId,
				int iStartdate,
				int iEndDate,
				int iFlag=0,
				bool bFirst=false,
				int iWeight=-1,
				bool bNeedProgressWnd=false);
			bool SetNormalTableCol(Table_Display& baTable,SecurityQuotation* pSecurity);
			//ȡ����������
			bool GetNormalTradeData(Table_Display& baTable,
				int iSecurityId,
				int iStartdate,
				int iEndDate,
				int iFlag=0,
				bool bFirst=false,
				int iWeight=-1,
				bool bNeedProgressWnd=false);
			bool SetBondTableCol(Table_Display& baTable, int nPrecise = 2);	// 2011-03-03 �޸�����ծȯ�۸�С����λ��
			//ȡ��ծȯ��������
			bool GetBondTradeData(Table_Display& baTable,
				int iSecurityId,
				int iStartdate,
				int iEndDate,
				int iFlag=0,
				bool bFirst=false,
				int iWeight=-1,
				bool bNeedProgressWnd=false,
				bool bIsFullPrice = false);
			//ծȯ��������
			bool SetBondTableData(Table_Display& baTable,
				SecurityQuotation* pSecurity,
				std::vector<HisTradeData> m_vecHisTrade,
				int iStartdate,
				int iEndDate,
				int iFlag,
				bool bFirst,
				int iWeight,
				bool bIsFullPrice = false);
			long GetMaxDate(long lTime,long lType);
			//����������ת����������
			void TransitionData(std::vector<HisTradeData>& m_vecHisTradeRsc,
				std::vector<HisTradeData> &m_vecHisTradeTem,
				SecurityQuotation* pSecurity,
				int iStartDate,
				int iEndDate,
				long lType);
			void GetCQDate(std::vector<HisTradeData>& m_vecHisTradeRsc,
				SecurityQuotation* pSecurity,
				int m_iStartDate,
				int m_iEndDate,
				int iWeight = -1);
			double GetCQcaleFromDate(std::vector<ExdividendData> m_vecCqData,
				int StraDate,
				int EndDate,
				int PreDate,
				int* CQDate,
				int m_bAspect);
			//��Ʊ��������
			bool SetStockTableData(Table_Display& baTable,
				SecurityQuotation* pSecurity,
				std::vector<HisTradeData> m_vecHisTrade,
				int iStartdate,
				int iEndDate,
				int iFlag,
				bool bFirst,
				int iWeight);
			//ָ����������
			bool SetIndexTableData(Table_Display& baTable,
				SecurityQuotation* pSecurity,
				std::vector<HisTradeData> m_vecHisTrade,
				int iStartdate,
				int iEndDate,
				int iFlag,
				bool bFirst,
				int iWeight);
			//��������
			bool SetNormalTableData(Table_Display& baTable,
				SecurityQuotation* pSecurity,
				std::vector<HisTradeData> m_vecHisTrade,
				int iStartdate,
				int iEndDate,
				int iFlag,
				bool bFirst,
				int iWeight);
			//���=��������
			//���ݰ���������㶨����������
			//ÿ�μ���һ��ָ���Ľ��������lBourseId=��������������������ֽ�����
			bool GetBlockPricedFuture(
				std::set<int>& iSecurityId,	//���������
				Tx::Core::Table& baTable,	//������
				int icalc_type,				//���㷽��
				//int iSecurityType			//���������ȯ������0=��Ʊ��1=ծȯ��2=ָ����3=Ȩ֤
				bool bNeedPsw=true
				);

			//�����������������ϸ����
			/*
			iItemʹ��˵��
			0:��ֵ
			1:��ͨ��ֵ 
			2:�������ܹɱ���Ȩ��ӯ��
			3:���������ܹɱ���Ȩ��ӯ��
			4:��������ͨ�ɱ���Ȩ��ӯ��
			5:����������ͨ�ɱ���Ȩ��ӯ��

			6:�������ܹɱ���Ȩ�о���
			7:���������ܹɱ���Ȩ�о���
			8:��������ͨ�ɱ���Ȩ�о���
			9:����������ͨ�ɱ���Ȩ�о���

			10:���ʲ�������[�������ʲ�]
			11:���ʲ�������[���������ʲ�]

			12:��������ƽ����
			13:�����ܹɱ���Ȩƽ����
			14:������ͨ�ɱ���Ȩƽ����

			15:��Ȩƽ��ÿ������[������]
			16:��Ȩƽ��ÿ������[��������]

			17:���ʲ�
			18:���о��ʲ�
			19:ÿ�ɾ��ʲ�

			20:�ܹɱ�
			21:���йɱ�
			22:��ͨ�ɱ�
			*/
			bool GetBlockPricedFuture(
				std::set<int>& iSecurityId,	//���������
				Tx::Core::Table& resTable,	//��������������
				long lBourseId,				//������ID,0��ʾ��������
				int iCalcType,				//���㷽�� 0=��̬;1=����;2=��;3=ͬ��
				//int iSecurityType			//���������ȯ������0=��Ʊ��1=ծȯ��2=ָ����3=Ȩ֤
				bool bNeedPsw=true
				);
			//���ö���������Ԫ������
			bool TxBusiness::SetBlockPricedFutureCell(
				Tx::Core::Table& resTable,	//��������������
				CString sName1,	//0=����
				CString sName2,	//1=С��
				double dValue,	//2=ָ������[���У����У��ϼ�]
				int iValueScale,//2.1ָ�����ݵ�С��λ��
				CString sDetail,//3=�鿴��ϸ[�ڽ����ϲ鿴��ϸ]
				int tot_count,	//4=����������=tot_count
				int countByBourse,	//5=���г�������=countByBourse
				int invalid_count,	//6=��Ч��������=invalid_count
				int invalid_tradestatus_count,//7=���ս���״̬����������=invalid_tradestatus_count
				int invalid_pepb_count,	//8=PEPB������Ч����=invalid_pepb_count
				int sampleCount,		//9=ʵ�ʲ��������������=sampleCount
				int invalid_share,		//10=��Ӧ�ɱ���Ч���ݵ�������=invalid_share
				int invalid_insititutionshare,//11=��˾�ܹɱ���Ч���ݵ�������=invalid_insititutionshare
				int iLR,//12=�������=iLR
				int iZC,//13=�����ʲ�����=iZC
				int invalid_tradeableshare,//14=��ͨ�ɱ���Ч����������=invalid_tradeableshare
				int invalid_lr,//15��Ч�������ݵ�������=invalid_lr
				int invalid_equity,	//16��Ч�ɶ�Ȩ�����ݵ�������=invalid_equity;
				double dShareHolder_Equity_Parent2,//17�ɶ�Ȩ��[���������ʲ�]
				double dProfitCur1,	//18��������+���������ʲ�=����
				double dShareHolder_Equity_Parent1,//19��������+���������ʲ�=���ʲ�
				double dProfitCur,	//20 �����������
				double dShareHolder_Equity_Parent,//21 �ɶ�Ȩ��[�������ʲ�]
				double dProfitCur2	//22������������
				);
			//ȡ��ָ�����㷽��������
			double GetLR(SecurityQuotation* psq,int calc_type,int* invalid_lr=NULL);

			bool GetBlockSamplesPricedFutureDetail(
				int iItem,							//ָ�����������Ŀ
				std::set<int>& iSecurityId,			//���������
				Tx::Core::Table_Indicator& resTable,//�������
				int calc_type //0,1,2,3
				);

			//ȡ�õ���ָ������ʵ��ĳ�Ȩ����
			bool GetSecurityExdividend(
				int iSecurityId,					//����ʵ��ID
				Tx::Core::Table_Indicator& resTable	//ȡ��iSecurityId�ĳ�Ȩ����
				);
			//ȡ�õ���ָ��ȯ�ĳ�Ȩ����
			bool GetSecurity1Exdividend(
				int iSecurity1Id,					//ȯID
				Tx::Core::Table_Indicator& resTable	//ȡ��iInstitutionId�ĳ�Ȩ����
				);
			//2007-08-23
			//����ȵ����
			bool BlockHotFocus(
				Tx::Core::Table& resTable,	//������ݱ�
				std::vector<int> iBloackSamples//�������ID
				);
			//�����շ���
			bool BlockRiskIndicator(
				int				iFunID,				//����ID
				std::set<int>& iSecurityId,			//����ʵ��ID
				int date,							//��������
				Tx::Core::Table_Indicator& resTable,//������ݱ�
				bool bPrice=false	//�ּ۱�־
				);
			//2009-05-05
			//���������շ���[�߼�]���ݱ�
			bool RiskIndicatorAdv(
				Tx::Core::Table_Indicator& resTable//������ݱ�
				);
			//2007-09-19
			//�����շ���[�߼�]
			bool RiskIndicatorAdv(
				Tx::Core::Table_Indicator& resTable,//������ݱ�
				std::set<int>& lSecurityId,			//����ʵ��ID
				int iEndDate,						//��ֹ����
				long lRefoSecurityId,				//��׼����ʵ��ID
				bool bFirstDayInDuration = false,	//Ĭ�������ڳ�
				int iStartDate=0,					//��ʼ����
				int iTradeDaysCount=20,				//��������
				int iDuration=0,					//��������0=�գ�1=�ܣ�2=�£�3=����4=��
				bool bLogRatioFlag=false,			//������ʵķ�ʽ,true=ָ������,false=�򵥱���
				bool bClosePrice=true,				//ʹ�����̼�
				int  iStdType=0,					//��׼������1=��ƫ��������ƫ
				int  iOffset=0,						//ƫ��
				bool bAhead=true,					//��Ȩ��־,true=ǰ��Ȩ,false=��Ȩ
				//bool bUserRate=true,				//�û��Զ���������,����ȡһ�궨�ڻ�׼����
				int bUserRate=0,					//0=�����޷��������ʣ�1ȡһ�궨�ڻ�׼���ʼ����޷��������ʣ�2�û��Զ��������ʼ����޷���������
				double dUserRate=0.034,				//�û��Զ���������,����ȡһ�궨�ڻ�׼����
				bool bDayRate=true,					//������
				bool bPrw = false,					//��������ʾ���
				bool bMean = false                  //�Ƿ��ڽ�����м��������ֵ����
				);

			//2007-10-08
			//���׶�����[�Ƿ�]
			bool BlockCycleRate(
				std::set<int>& iSecurityId,			//����ʵ��ID
				int date,							//��������
				Tx::Core::Table_Indicator& resTable,//������ݱ�
				bool bPrice=false//�ּ۱�־
				);
			//���׶�����[�Ƿ�][�߼�]
			bool BlockCycleRateAdv(
				std::set<int>& iSecurityId,			//����ʵ��ID
				int startDate,						//��ʼ����
				int endDate,						//��ֹ����
				bool bCutFirstDateRaise,			//�޳������Ƿ�
				int	iFQLX,							//��Ȩ���� 0-����Ȩ;1=��Ȩ
				Tx::Core::Table_Indicator& resTable,//������ݱ�
				int iPrecise=2,						//С����λ��
				int iFlag=0,						//��������0-Ĭ�ϣ�1-ծȯ[bCutFirstDateRaise=true��ʾȫ��ģʽ,bCutFirstDateRaise=false��ʾ����ģʽ]
				int iMarketId=0,					//������ID
				bool bOnlyRaise=false,				//ֻ�����Ƿ�
				bool bUseCurData=false				//�Ƿ�ʹ�õ�������
				);
			bool BlockCycleRateAdv(
				std::vector<int>& iSecurityId,			//����ʵ��ID
				int startDate,						//��ʼ����
				int endDate,						//��ֹ����
				bool bCutFirstDateRaise,			//�޳������Ƿ�
				int	iFQLX,							//��Ȩ���� 0-����Ȩ;1=��Ȩ
				Tx::Core::Table_Indicator& resTable,//������ݱ�
				int iPrecise=2,						//С����λ��
				int iFlag=0,						//��������0-Ĭ�ϣ�1-ծȯ[bCutFirstDateRaise=true��ʾȫ��ģʽ,bCutFirstDateRaise=false��ʾ����ģʽ]
				int iMarketId=0,					//������ID
				bool bOnlyRaise=false,				//ֻ�����Ƿ�
				bool bUseCurData=false				//�Ƿ�ʹ�õ�������
				);
			//�ۺ����׶��Ƿ�
			bool BlockCycleRateHome(
				std::vector<int>& iSecurityId,			//����ʵ��ID
				int date,							//��������
				Tx::Core::Table_Display& table, //������ݱ�
				bool bHaveIndex = false  //�Ƿ������ѡ��ָ��
				);
		protected:
			virtual double GetBondInterest(SecurityQuotation* pSecurity,int iDate);

		public:
			//2008-01-07
			//ȡ�����г�������������
			bool GetHuShenTradeDate(void);

			//2007-09-18��TxStock��ֲ����
			//��������ͬ��
			//�Ի�׼����ʵ��Ϊ����,���ն����ٲ���ԭ��,����������ʵ��ͬ��׼����ʵ�����ͬ��
			bool SynTradeData(
				Security* pBase,				//��׼����ʵ��
				Security* pSample,				//��������ʵ��
				HisTradeData* pSampleSecurityTradeData,//ͬ�����������������
				int baseDate,					//��ֹ����
				int iDays,						//׼�������������������
				bool bIsToday,					//��ֹ�����ǵ��գ��ҵ����ǽ�����
				bool bSameDuration,				//��ֹ�����ǵ��գ��ҵ����ǽ�����,�������ļ������������ͬ����
				int iDuration=0,				//��������0=�գ�1=�ܣ�2=�£�3=����4=��
				bool bFirstDayInDuration=true	//true=�����ڳ���false=������ĩ
				);

			//�������
			double CalcRatio(double x,double y,bool bLogRatioFlag)
			{
				if(x == y || x == 0) return 0;
				if(bLogRatioFlag == true)//��Ȼ����
				{
					if(y==0) return 0;
					return log(y) - log(x);
				}
				else//�򵥱���
					return (y - x)/x;
			}
			//add by guanyd 20071127
			void TableOutput(Tx::Core::Table_Indicator &resTable);
		public:
			//�������ݱ������Ϣ
			void SetDisplayTableColInfo(Table_Display* pTable,int iCol,int iIndicatorId,bool bSetTitle=true);

		public:
			//����ʵ������С��λ��
			int GetDataByScale(float fValue,int iScale,int& iRatio);
			//int GetDataByScale(int iScale);
			//2008-04-28
			//ȡ��Ԥ��pe�����
			int GetPeYear(void);
			//ȡ��Ԥ��pe�����
			int GetPeYear(std::vector<int> arrSamples);
		public:
			//����ʹ��
			//ʹ��֮ǰ���� GetIndicatorDataNow
			//ȡ��ָ������
			IndicatorData*			m_pIndicatorData;

			//��ʱ���ݽ����
			Tx::Core::Table_Indicator m_txTable;
			//����ʹ��
			//ʹ��֮ǰ���� GetSecurityNow
			//ȡ�ý���ʵ����ز���
			SecurityQuotation*		m_pSecurity;

			//��ָ֤����ָ��4000228
			SecurityQuotation*		m_pShIndex;
			//���гɷ�ָ��4000001
			SecurityQuotation*		m_pSzIndex;
			//2008-01-08
			//�������н�����������
			MixTradeDay*	m_pMixTradeDay;

		protected:
			//ȯ�ַ�������ID
			int						m_iSecurityTypeId;

			//��Ԫ������ҵĻ���
			double					m_dDollar2RMB_ratio;

			//���г�Ȩ����
			//�������һ��
			Tx::Core::Table_Indicator m_AllExdividendDataTable;
			//ȡ��pe���������²���
			int						m_iYearPe;

			//˽�г�Ա��
		public:
			//ȡ�ü���[���]����ز���
			FunctionDataManager*	m_pFunctionDataManager;
			//ȡ��ָ�����ز���
			LogicalBusiness*		m_pLogicalBusiness;

			// added by zhoup 2008.12.01
			// ʵ�ֶ�A��B�ɵ��ظ�����
			// ����һ���������˽ӿڣ���ͬʱ����A��B�ɵ�ʱ�򣬽�B�ɹ��˵���ֻ��A����B�ɵ�ʱ�򣬲�����
			// ����Ĳ�����Ϊ���Ľ��
			void InistitutionFilter(std::vector<int>& vecSecurity);

			// added by zhoup 2008.12.01
			// ʵ�ֶ�A��B�ɵ��ظ�����
			// ����һ���������˽ӿڣ���ͬʱ����A��B�ɵ�ʱ�򣬽�B�ɹ��˵���ֻ��A����B�ɵ�ʱ�򣬲�����
			// ����Ĳ�����Ϊ���Ľ��
			void InistitutionFilter(std::set<int>& setSecurity);
		private:
			void CalStatusAboutToday(SecurityQuotation * psq, int date, bool & bNeedCheckToday, bool & bNeedAppendToday);
			void PushBackToday(bool bNeedAppendToday,SecurityQuotation * psq,std::vector<HisTradeData> & m_vecHisTrade);
		public:
			//��ȡYTM
			virtual void GetBondYTM(std::vector<int> nSecurityIds);
			virtual void GetBondYTM(int nSecurityId,double &dYtm,double &dMdur,double &dCon);
		};
	}
}

#endif