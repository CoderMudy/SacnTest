/**************************************************************
Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
File name:		Fund.h
Author:			�Ժ꿡
Date:			2007-07-09
Version:		1.0
Description:	
����ҵ������

2007-09-17
���ڻ���͹�Ʊ�İ�����������������
ֻҪ��������ʽ����Ϳ����ˣ�
���Ի�������������͹�Ʊ��������������TxBusiness::GetBlockAnalysis

��鶨������������ʽ�����⣬�㷨���Ʊ��ͬʹ��TxBusiness::GetBlockPricedFuture

�ȵ�����ͷ�ʱͬ���Ѿ��ڵײ��ṩ���ݽӿڣ���UI����ͼ�κ���ȡ����ʱֱ�ӵ���

�����շ�������ָ��ID��ͬ��������TxFund�ﴦ�����ķ��շ���

***************************************************************/
#if !defined(AFX_TXFUND_H__5BA14E0A_27BC_4FD4_B281_1AEFC5CFB1D8__INCLUDED_)
#define AFX_TXFUND_H__5BA14E0A_27BC_4FD4_B281_1AEFC5CFB1D8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MyIndicator.h"
#include "..\..\core\core\Commonality.h"
#include "..\..\UI\FrameBase\GridAdapterTable.h"//2008-08-07
#include "..\..\Data\TypeMapManage.h"
#include "..\..\Data\DataFileStruct.h"
#include "..\..\Data\SecurityBase.h"
#include "..\..\Data\DataFileStruct.h"
#include "../../Core/Driver/ClientFileEngine/base/HttpDnAndUp.h"

namespace Tx
{
	class fund_db_context;

	//
	// fund_total_share_db_set
	//
	class fund_total_share_db_set
	{
		friend class fund_db_context;

	public:
		explicit fund_total_share_db_set(int date);
		~fund_total_share_db_set();

		int get_count() const;

		// ��ȡ�����ܷݶ�
		double get_value(int id) const;	

	protected:
		bool _Download();
		bool _Read_response(unsigned char * _Buffer,unsigned int _NumberOfBytes);

	private:
		int _M_date;
		std::unordered_map<int,double> _M_set;
	};

	//
	// fund_db_context
	//
	class fund_db_context
	{
	public:
		fund_db_context();
		~fund_db_context();

		std::shared_ptr<fund_total_share_db_set> get_total_share_db_set(int date,bool update);

	private:
		// key Ϊ����
		std::unordered_map<int,std::shared_ptr<fund_total_share_db_set>> _M_totalShares;
	};

	namespace Business
	{
		//guanyd
		//������
		//���������������
		struct	FundStyle
		{
			//���������	
			//0x0001��Ʊ��	0x0002�����	0x0004ծȯ��	0x0008������	0x0010������
			WORD	wBasicFundStyle;

			//��Ʊ����ϸ
			//0x0001�ɳ���	0x0002ƽ����	0x0004��ֵ��	0x0008ָ����	0x0010��ҵ��	0x0020������	0x0040������
			WORD	wShareDetail;
			//�������ϸ
			//0x0001��������	0x0002������	0x0004������
			WORD	wMixDetail;
			//ծȯ����ϸ
			//0x0001��ծ��	0x0002ƫծ��
			WORD	wBondDetail;
			//��������ϸ
			WORD	wCurrencyDetail;
			//��������ϸ
			WORD	wPrincipalDetail;

			void clear()
			{
				wBasicFundStyle=31;
				wShareDetail=127;
				wMixDetail=7;
				wBondDetail=3;
				wCurrencyDetail=1;
				wPrincipalDetail=1;
			}
		};//end of struct FundStyle
		//����ҵ������
		class BUSINESS_EXT TxFund : 
			public TxBusiness
		{
			DECLARE_DYNCREATE(TxFund)
		public:
			TxFund();
			virtual ~TxFund();

		private:
			static fund_db_context _M_dbContext;

		public:
			//2008-09-12
			//������Ȼ����ȡ�û��𱨸�������
			int GetFundReportDate(int date,bool bQuarter,int iMarketId=0);
			int GetFundReportDateMD(int date,bool bHis,bool bQuarter);

			//2008-09-11
			//�ز�ծȯ=��ϸ=ǰ5��Ϊ�ز�
			bool GetMyBonds(std::vector<int> iSecurityId,Table_Display& baTable,int iYear,int iQuarte,int no=5);

			//2008-08-27
			//�������
			double	GetOverflowValueRate(
				int     iSecurityId,
				int		iDate
				);

			//����������ݽ��������
			bool GetBlockAnalysisCol(Table_Display& baTable,std::vector<int>& arrSamples,int& iSortCol);
			//����������ݽ������
			bool SetBlockAnalysisCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow);

			//2007-09-17
			//zhaohj
			//�����շ���[�߼�]
			bool BlockRiskIndicatorAdvFund(
				int iMenuID,						//����ID
				Tx::Core::Table_Indicator& resTable,//������ݱ�
				std::set<int>& lSecurityId,			//����ʵ��ID
				int iEndDate,						//��ֹ����
				long lRefoSecurityId,				//��׼����ʵ��ID
				int iStartDate=0,					//��ʼ����
				int iTradeDaysCount=20,				//��������
				int iDuration=0,					//��������0=�գ�1=�ܣ�2=�£�3=����4=��
				bool bLogRatioFlag=false,			//������ʵķ�ʽ,true=ָ������,false=�򵥱���
				bool bClosePrice=true,				//ʹ�����̼�
				int  iStdType=0,					//��׼������1=��ƫ��������ƫ
				int  iOffset=0,						//ƫ��
				bool bAhead=true,					//��Ȩ��־,true=ǰ��Ȩ,false=��Ȩ
				bool bUserRate=true,				//�û��Զ���������,����ȡһ�궨�ڻ�׼����
				double dUserRate=0.034,				//�û��Զ���������,����ȡһ�궨�ڻ�׼����
				bool bDayRate=true,					//������
				bool bPrw = false,					//�Ƿ���ӽ�����
				bool bMean = false                  //�Ƿ���������ֵ
				);
			//2007-10-15
			//
			/*bool BlockCycleRate(
			std::set<int>& iSecurityId,			//����ʵ��ID
			int date,							//��������
			Tx::Core::Table_Indicator& resTable	//������ݱ�
			);
			*/
			//2007-10-15
			//���һ�����������
			bool StatCurrencyFundSurplusTerm(
				//		int iMenuID,						//����ID
				Tx::Core::Table_Indicator& resTable,//������ݱ�
				std::vector<int>& iSecurityId,		//����ʵ��ID
				std::vector<int>  iDate				//������
				);
			//�زֹɼ��ж�ͳ��
			bool StatFundZCGjzd(
				int iMenuID,						//����ID
				Tx::Core::Table_Indicator& resTable,//������ݱ�
				std::vector<int>& iSecurityId,		//����ʵ��ID
				std::vector<int> vDate				//������
				);
			//�زֹɼ��ȱȽ�
			bool StatFundZCGjdbj(
				int iMenuID,						//����ID
				Tx::Core::Table_Indicator& resTable,//������ݱ�
				std::vector<int>& iSecurityId,		//����ʵ��ID
				std::vector<int> vDate				//������
				);

			//modified by zhangxs
			bool TxFund::StatFundStockHolding(
				Tx::Core::Table_Indicator& sourceTable,	//Ϊ˫���󱣴����ݡ�
				Tx::Core::Table_Indicator& resTableStock,	//��Ʊ������ݱ�
				Tx::Core::Table_Indicator& resTableFund,	//���������ݱ�
				std::vector<int>& iFundSecurityId,		//������ʵ��ID
				std::vector<int>& iStockSecurityId,		//��Ʊ����ʵ��ID
				std::vector<int> vDate,				//������
				bool bIsZCG,							//����ԴΪ�زֹɣ�����Ϊ�ֹ���ϸ
				//std::set<int> & TradeId,			//������ȡ��sourceTable������ݵ�ȫ���Ľ���ʵ��ID
				std::vector<CString> &vColName,
				std::vector<CString> &vColHeaderName,
				int& m_iFirstSampleId
				);
			//�������������ϸ
			//modified by zhangxs
			bool TxFund::StatFundStockHolding(
				Tx::Core::Table_Indicator& sourceTable,	//Ϊ˫���󱣴����ݡ�
				Tx::Core::Table_Indicator& resTableStock,	//��Ʊ������ݱ�
				Tx::Core::Table_Indicator& resTableFund,	//���������ݱ�
				std::vector<int>& iFundSecurityId,		//������ʵ��ID
				std::vector<int>& iStockSecurityId,		//��Ʊ����ʵ��ID
				std::vector<int> vDate,				//������
				bool bIsZCG,							//����ԴΪ�زֹɣ�����Ϊ�ֹ���ϸ
				//std::set<int> & TradeId,			//������ȡ��sourceTable������ݵ�ȫ���Ľ���ʵ��ID
				std::vector<CString> &vColName,
				std::vector<CString> &vColHeaderName
				);
			//��Ʊ����״��ͳ��--����ԴΪ��ϸ
			bool GetHeldStockHeldDetail(
				Tx::Core::Table_Indicator &resTable,
				std::vector<int> TradeId,
				std::vector<int> ViFundSecurityId,
				std::vector<int> iDate
				);
			//added by zhangxs
			//��Ʊ����״��ͳ��--����ԴΪ�زֹ�

			bool TxFund::GetHeldStockFromTopTen(
				Tx::Core::Table_Indicator &resTable,
				std::vector<int> TradeId,
				std::vector<int> ViFundSecurityId,
				std::vector<int>   iDate);
			//ר��Ϊ��Ʊ���������������������

			//added by zhangxs 20081120
			//��Ʊ����״��ͳ��--����ԴΪ��ϸ
			bool GetHeldStockHeldDetail(
				Tx::Core::Table_Indicator &resTable,
				int	iSecurityId,
				std::vector<int> ViFundSecurityId,
				std::vector<int> iDate
				);
			//added by zhangxs
			//��Ʊ����״��ͳ��--����ԴΪ�زֹ�

			bool TxFund::GetHeldStockFromTopTen(
				Tx::Core::Table_Indicator &resTable,
				int	iSecurityId,
				std::vector<int> ViFundSecurityId,
				std::vector<int>   iDate);

			//����ԴΪ�زֹ�
			bool StockHoldingTopTen(
				Tx::Core::Table_Indicator &resTable,
				std::vector<int>	iSecurityId,
				std::vector<int>	iDate
				);

			//����ԴΪ�ֹ���ϸ
			bool StockHoldingList(
				Tx::Core::Table_Indicator &resTable,
				std::vector<int>	iSecurityId,
				std::vector<int>	iDate
				);	
			//ĳ��Ʊ��������е���ϸ
			bool StatFundStockDetail(
				Tx::Core::Table_Indicator& resTable,	//������ݱ�
				Tx::Core::Table_Indicator& resTableMx,	//������ݱ���ϸ
				int iSecurityStockId	//��Ʊ����ʵ��ID
				);
			//�����깺��ط��ʱ�
			bool StatFundFeeRate(
				Tx::Core::Table_Indicator& resTable,	//������ݱ�
				std::vector<int>& iFundSecurityId,		//������ʵ��ID
				int iStartDate,							//��ʼ����
				int iEndDate,							//��ֹ����
				int FeeType,							//����ģʽ
				bool IsAppoint = true					//�Ƿ�����������
				);
		private:
			//2007-10-16
			//bFullData = false =>��ȷͳ��=ͳ�ƽ���ļ�¼����������������һ��[һ��һ]
			//bFullData = true =>ͳ��=ͳ�ƽ���ļ�¼��������������һ��[һ�Զ�]
			bool StatCommon(
				int iMenuID,							//����ID
				Tx::Core::Table_Indicator& resTable,	//������ݱ�
				std::vector<int>& iSecurityId,		//����ʵ��ID
				std::vector<Tx::Core::VariantData> arrParam,	//����
				bool bFullData=false					//ͳ������
				);



		public:
			//guanyd 
			//ר��ͳ��
			//2007-09-17

			//�׶�����
			//��������
			bool	StatCycleRate(
				Tx::Core::Table_Indicator	&resTable,
				std::set<int>	&iSecurityId,
				int		iStartDate,
				int		iEndDate,
				bool bCutFirstDateRaise,			//�޳������Ƿ�
				int	iFQLX							//��Ȩ����
				);

			//�г����
			//����ݶ�
			bool	StatFundShare(
				Tx::Core::Table_Indicator	&resTable,
				std::vector<int>	&iObjectId,
				int		iStartDate,
				int		iEndDate,
				bool	bAllDate,
				UINT	uReason,
				//		UINT	uStyle,
				UINT	uType
				);

			//����ֺ�
			bool	StatFundDividend(
				Tx::Core::Table_Indicator	&resTable,
				std::vector<int>	&iSecurityId,
				int		iStartDate,
				int		iEndDate,
				bool	bGetAllDate
				);

			//������
			//wIssueType�Ƿ������ͣ�	0x0001�׷���0x0002��ļ
			bool	StatFundIssue(
				Tx::Core::Table_Indicator	&resTable,
				std::vector<int>	&iSecurityId,
				int		iStartDate,
				int		iEndDate,
				bool	bGetAllDate,
				WORD	wIssueType,
				bool    FXRQ = true
				);
			//���ʽ����
			bool	StatFundIssueClose(
				Tx::Core::Table_Indicator	&resTable,
				std::vector<int>	&iSecurityId,
				int		iStartDate,
				int		iEndDate,
				bool	bGetAllDate,
				WORD	wIssueType,	
				bool    FXRQ 
				);
			//����ʽ����
			bool	StatFundIssueOpen(
				Tx::Core::Table_Indicator	&resTable,
				std::vector<int>	&iSecurityId,
				int		iStartDate,
				int		iEndDate,
				bool	bGetAllDate,
				bool    FXRQ 
				);
			//����ʽ����==�¿�
			bool	StatFundIssueOpen_New(
				Tx::Core::Table_Indicator	&resTable,
				std::vector<int>	&iSecurityId,
				int		iStartDate,
				int		iEndDate,
				bool	bGetAllDate,
				bool    FXRQ 
				);
			//����˾�ſ�
			bool	StatFundCompanySituation(
				Tx::Core::Table_Indicator	&resTable,
				std::vector<int>	&iSecurityId,
				int		iStartDate,
				int		iEndDate,
				bool	bGetAllDate	
				);

			//����˾�ɶ�
			bool	StatFundHolder(
				Tx::Core::Table_Indicator	&resTable,
				std::vector<int>	&iSecurityId,
				int		iStartDate,
				int		iEndDate,
				bool	bGetAllDate		
				);

			//������
			bool	StatFundManager(
				Tx::Core::Table_Indicator	&resTable,
				std::vector<int>	&iSecurityId,
				int		iStartDate,
				int		iEndDate,
				bool	bGetAllDate
				);
			bool StatFundHoldingStructure(
				Tx::Core::Table_Indicator &resTable,
				std::vector<int>	iSecurityId,
				std::vector<int>	iDate,
				int		iType
				);
			//ҵ������
			//����ֵ���
			bool	StatFundNetValueOutput(
				Tx::Core::Table_Indicator	&resTable,
				std::vector<int>	&iSecurityId,
				int		iStartDate,
				int		iEndDate,
				int		iTimeCycle,		//ʱ�����ڡ������¼���
				int		iStatIndicator,	//����λ��ֵ�������ۼƾ�ֵ
				std::vector<CString> &vDates,
				std::vector<CString> &vColName
				);
			//����ֵ -- ��ȯ������--���ڵ㣬��������
			bool	StatFundNetValueOutput(Tx::Core::Table_Indicator &resTable,std::vector<int>	&iSecurityId,std::vector<int> iDates);

			bool	GetMmfFundNvr( std::vector< int > iSecurity,std::vector< int > iDates, Tx::Core::Table_Indicator* pResTable );
			//--------------��ȡ���һ�����������껯-----------------
			bool	GetMmfFundAcuNvr( std::vector< int > iSecurity,std::vector< int > iDates, Tx::Core::Table_Indicator* pResTable );

			std::unordered_map< int,int >		m_MmfFundNvrColMap;	//������

			//���һ�������
			bool	StatFundCurrencyIncome(
				Tx::Core::Table_Indicator	&resTable,
				std::vector<int>	&iSecurityId,
				int		iStartDate,
				int		iEndDate,
				int		iTimeCycle,			//ʱ������:�����¼�������
				int		iStatIndicator,		//ͳ��ָ��:��ݻ���λ���桢������껯������
				bool    IsOut,				//��ֱ����������ۼƼ��㡣
				std::vector<CString> &vDates,
				std::vector<CString> &vColName
				);
			//add by lijw 2008-05-16
			//��ר��Ϊ�˼�����һ�����������ۼƼ��㡣
			bool AddUpCalculator(
				Tx::Core::Table_Indicator &resTable,   //��Ž���ı�
				Tx::Core::Table_Indicator &m_txTable,  //����Ҫ��������ݵ�Դ��
				std::vector<int>	&iSecurityId,	  //�����Ľ���ʵ��ID
				int		iStartDate,					  //��ʼ����
				int		iEndDate,					 //��ֹ����
				int		iStatIndicator,		        //ͳ��ָ��:��ݻ���λ���桢������껯������
				int		iTimeCycle,			        //ʱ������:�����¼�������
				bool	IsOut,				//��ֱ����������ۼƼ���
				std::vector<CString> &vDates,
				std::vector<CString> &vColName
				);
			void CalculatorEndDate(int iTimeCycle,int& dateE,int & dateB);
			//�������
			bool	StatOverflowValueRate(
				Tx::Core::Table_Indicator	&resTable,
				std::vector<int>	&iSecurityId,
				int		iStartDate,
				int		iEndDate,
				int		iTimeCycle,		//ͳ������
				int		iCustomCycle,	//�Զ�������
				std::vector<CString> &vColName
				);
			//��ֵ������
			bool	StatFundNetValueRiseRate(
				Tx::Core::Table_Indicator	&resTable,
				std::vector<int>	&iSecurityId,
				int		iStartDate,
				int		iEndDate,
				int		iTimeCycle,		//ͳ������
				int		iCustomCycle,	//�Զ�������
				std::vector<CString> &vColName
				);
			//��ֵ����
			bool	StatFundNetValueAnalyse(
				Tx::Core::Table_Indicator	&resTable,
				std::vector<int>	&iSecurityId,
				int		iEndDate,
				std::vector<CString> ExplainVec,
				std::vector<int> StartVec,
				std::vector<int> EndVec,
				std::unordered_map<int,double>	mapMarketSetting,
				bool	bCalculateValueRate,
				int		iType,
				std::vector<CString> &vColName,
				std::vector<CString> &vColHeaderName
				);
			//Ͷ����Ϸ���
			//�ֲֽṹͳ��
			bool StatHoldingStatistics(
				Tx::Core::Table_Indicator &resTable,
				std::vector<int>	iSecurityId,
				std::vector<int>	iDate,
				int		iType
				);
			bool StatFundHoldingStatistics(
				Tx::Core::Table_Indicator &resTable,
				std::vector<int>	iSecurityId,
				std::vector<int>	iDate,
				int		iType
				)	;
			//�ʲ����ͳ�� t_asset_allocation
			bool StatAssetAllocation(
				Tx::Core::Table_Indicator &resTable,
				std::vector<int>	iSecurityId,
				std::vector<int>	iDate,
				int		iType
				);

			//��ҵ�ֲ�ͳ��
			//		T_STOCK_HOLDING_INDUSTRY_DISTRIBUTION_ACTIVE	����Ͷ�ʲ�����ҵ�ֲ���
			//		T_STOCK_HOLDING_INDUSTRY_DISTRIBUTION_INACTIVE	ָ����Ͷ�ʲ�����ҵ�ֲ���
			//		T_STOCK_HOLDING_INDUSTRY_DISTRIBUTION	�ϲ�Ͷ�ʲ�����ҵ�ֲ���
			bool StatIndustryDistribution(
				Tx::Core::Table_Indicator &resTable,
				std::vector<int>	iSecurityId,
				std::vector<int>	iDate,
				int		iType
				);
			//ծȯ��Ͻṹͳ��T_BOND_HOLDING_BOND_TYPE_DISTRIBUTION
			bool StatBondDistribution(
				Tx::Core::Table_Indicator &resTable,
				std::vector<int>	iSecurityId,
				std::vector<int>	iDate,
				int		iType
				);
			//������ҵ�ֲ�ͳ��
			bool StatIndustryDistributionTx(
				Tx::Core::Table_Indicator &resTable,
				std::vector<int>	iSecurityId,
				std::vector<int>    iStartData,
				std::vector<int>	iDate,
				int		iType
				);
			//�ֹ�ͳ��
			//��Ʊ����״��ͳ��


			//�زֹ�ͳ��
			bool StatStockHoldingTopTen(
				Tx::Core::Table_Indicator &resTable,
				std::vector<int>	iSecurityId,
				std::vector<int>	iDate,
				int		iType
				);

			//�زֹɼ��ж�ͳ��

			//�زֹɼ��ȱȽ�

			//�ֹ���ϸͳ��
			bool StatStockHolding(
				Tx::Core::Table_Indicator &resTable,
				std::vector<int>	iSecurityId,
				std::vector<int>	iDate,
				int		iType,
				int     &iTempCount
				);	
			//����������Ʊ
			bool StatBuyInSellOutStock(		
				Tx::Core::Table_Indicator &resTable,
				std::vector<int>	iSecurityId,
				std::vector<int>	iDate,
				int		iType
				);


			//��ծͳ��
			//�زֳ�ծͳ��T_BOND_HOLDING_TOP_FIVE_BOND
			bool StatBondHoldingTopFive(
				Tx::Core::Table_Indicator &resTable,
				std::vector<int>	iSecurityId,
				std::vector<int>	iDate,
				int		iType
				);

			//����ת���ڵĿ�תծT_BOND_HOLDING_TOP_FIVE_CONVERTIBLE
			bool StatConvertibleBond(
				Tx::Core::Table_Indicator &resTable,
				std::vector<int>	iSecurityId,
				std::vector<int>	iDate,
				int		iType
				);
			//���һ������ծȯ
			bool StatCurrencyFundHoldingBond(
				Tx::Core::Table_Indicator &resTable,
				std::vector<int> &iSecurityId,
				std::vector<int> &iDate,
				int iType
				);
			//add by lijw 2008-09-08
			//ȡ�÷��ʽ����ֵ������
			bool GetCloseFundNavRaise(
				std::vector<int>& iSecurityId,		//����ʵ��ID
				//int date,							//��������,0��ʾ���������
				Tx::Core::Table_Display& resTable,	//������ݱ�
				int& iDate,
				bool bPrice = false				//�ּ۱�־
				);
			//����ֵ�Ƿ�
			bool GetFundNavRaise(
				std::vector<int>& iSecurityId,			//����ʵ��ID
				int date,							//��������
				Tx::Core::Table_Display& resTable,//������ݱ�
				bool bPrice=false//�ּ۱�־
				);
			//����Ľ׶�����   add by lijw 2008-09-11
			bool FundPhaseMarket(std::vector<int> &samples,Tx::Core::Table_Indicator& resTable);
			//����ĳ����˽ṹ   add by lijw 2008-09-16
			bool HoldPersonStruct(std::vector<int> &iSecurityId,Tx::Core::Table_Indicator& resTable,std::vector<int> &vDate);
			//2008-09-08 add by wangych
			//ȡ�ÿ���ʽ����ֵ�Ƿ�
			bool GetOpenFundNavRaise(
				std::vector<int>& iSecurityId,		//����ʵ��ID
				//int date,							//��������
				Tx::Core::Table_Display& resTable,	//������ݱ�
				int& iDate,
				bool bPrice=false					//�ּ۱�־
				);


			//2008-08-07
			//ȡ�û���ֵ2days
			bool GetFundNav2Days(
				std::vector<int>& iSecurityId,		//����ʵ��ID
				Tx::Core::Table_Display& resTable,	//������ݱ�
				GridAdapterTable_Display& resAdapter//��ʾ��ʽ
				);

			//2008-09-08 add by wangych
			//ȡ�ÿ���ʽ����ֵ2days
			bool GetOpenFundNav2Days(
				std::vector<int>& iSecurityId,		//����ʵ��ID
				Tx::Core::Table_Display& resTable,	//������ݱ�
				GridAdapterTable_Display& resAdapter,//��ʾ��ʽ
				int& iDate1,						//������1
				int& iDate2							//������2
				);

			//add by lijw 2008-09-08
			//ȡ�û���ֵ2days
			bool GetCloseFundNav2Days(
				std::vector<int>& iSecurityId,		//����ʵ��ID
				Tx::Core::Table_Display& resTable,	//������ݱ�
				GridAdapterTable_Display& resAdapter,//��ʾ��ʽ
				int& iDate1,						//������1
				int& iDate2							//������2
				);

			bool GetFundCurrency2Days(
				std::vector<int>& iSecurityId,		//����ʵ��ID
				Tx::Core::Table_Display& resTable,	//������ݱ�
				GridAdapterTable_Display& resAdapter,//��ʾ��ʽ
				int& iDate1,						//������1
				int& iDate2							//������2
				);

			//2008-09-11 add by wangych
			//ȡ�÷��ʽ�������鱨��
			bool GetCloseFundQuotePrice(
				std::vector<int>& iSecurityId,		//����ʵ��ID
				int date,							//��������
				Tx::Core::Table_Display& resTable,	//������ݱ�
				GridAdapterTable_Display& resAdapter//��ʾ��ʽ
				);
			//ȡ����ҵ���� add by wangych 08.09.12
			bool TxFund::GetIndustryDistribute(
				std::vector<int>& iSecurityId,		//����ʵ��ID
				int date1,							//��������,��ѡ������
				int date2,							//������ǰ����ѡ�ϸ�������
				Tx::Core::Table_Display& resTable,	//������ݱ�
				GridAdapterTable_Display& resAdapter//��ʾ��ʽ
				);


			//ͬҵ����
			//����Ͷ�����

			//��ɫ��Ʊ

			//��ɫ��ҵ

			//�زֹ�Ʊ����

			//����Ӷ��
			bool StatTradeCommission(
				Tx::Core::Table_Indicator &resTable,
				std::vector<int>	iSecurityId,
				std::vector<int>	iDate,
				int		iType
				);
			//guanyd
			//һЩ����
		private:
			UINT varCfg[9];
			FundStyle	m_FundStype;			

			IndicatorWithParameter m_IWP;
			Tx::Business::IndicatorWithParameterArray m_IWPA;

			//��ʱ��Ŀǰ�����ڷ���ͳ����Ϊģ���αȫ�ֱ���ʹ��
			Tx::Core::Table_Indicator tmpTable;

			CString strTable;//�����������
			bool TransObjectToSecIns(
				std::vector<int>	sObjectId,		//����ʵ��id
				std::vector<int> &sSecInsId,		//ȯid�����id
				int iType						//id���ͣ�1:ȯid	2:����id
				);

			bool IdColToNameAndCode(
				Tx::Core::Table_Indicator &resTable,		//�����
				int iCol,									//�ƶ���
				int iIdType,								//��������
				int iMethodType=0
				);
			bool StatisticOnType(Tx::Core::Table_Indicator &resTable,int iClassifyType);

			bool StatisticsOnType(Tx::Core::Table_Indicator &resTable,int iClassifyTypes);

			//������ת��������
			// ��ʼ��ֹ�����뱨���ڵ�ת�� by 20101111 wanglm
			int RegportDateToReportID( std::vector<int> vecStartDate, std::vector<int> vecDate );  // by 20101112 wanglm

			//����+������-���������ڣ�����һ��
			bool TransReportDateToNormal(Tx::Core::Table_Indicator &tmpTable,int iCol);
			// add by lijw 2008-02-20,����ӿڹ��õĽӿڡ����������������һ��ר�õġ�tempTable���������ݣ�Ҳ�Ѵ���������ݱ�����tempTable��
			bool TransReportDateToNormal2(Tx::Core::Table_Indicator &tempTable,int iCol);
			//��������->CString���ͣ�tempTable���������ݣ�Ҳ�Ѵ���������ݱ�����tempTable�
			bool TransReportDateToNormal3(Tx::Core::Table_Indicator &tempTable,int iCol);
			//��������������ȥ�Ǳ�Ļ�������
			//���������ݴ�����ʵ������һ��������ԣ����Բ�����table�Ļ����������������Fund�µ�һ������
			//CopyRow����һ�����ݵ���һ�ű����һ�У�ʵ��Ҫ��֤�����Լ���������ȫ��ͬ
			//PlusRow���Ƕ������������ݣ�int ,double,float�������ۼӣ����������򲻹�
			bool CopyRow(Table_Indicator &resTable,Table_Indicator &sourceTable,int iResRow,int iSourceRow);
			bool PlusRow(Table_Indicator &resTable,Table_Indicator &sourceTable,int iResRow,int iSourceRow);
			//add by lijw 2009-2-13
			bool EliminateSample(std::vector<int> &TradeVS,std::vector<int> &TradeVR,std::vector<int> &FundV);
			bool EliminateSample(std::vector<int> &TradeVS,std::vector<int> &TradeVR,std::set<int> &FundSet);

			//
			bool FillColumn(
				Tx::Core::Table_Indicator &m_transTable,		//������
				UINT transIndexColumn,							//��������Ϊmap��������
				UINT addColumn,									//�����������ݵ��У�������ȡ�����ݷŵ�insertColumn��
				Tx::Core::Table_Indicator &m_resTableAddColumn,	//Դ��
				UINT resIndexColumn,								//Դ����Ϊmap��������
				UINT	insertColumn								//Ҫ�������ݵ���		
				);

			bool TransReportIdToFundId(Tx::Core::Table_Indicator &resTable,int iCol=0);


			bool GetCycleDates(int iStartDate,int iEndDate,int iTimeCycle,std::vector<int> &iDates,int iCustomCycle);

			bool InsertColOfNAV(Tx::Core::Table_Indicator &resTable,int iInsertCol,int iIdCol,int iDateCol);
			bool ChangeDateColToReportCol(Tx::Core::Table_Indicator &resTable,int iCol);

			bool GetIndexDat(int file_id, std::unordered_map<int,CString>& indexdat_map);
			//add by lijw 2008-03-25
			//��ע������ĺ�����ר�ż������ֵ���ۼӡ�Ϊ�����ķ���ͳ����׼��
			bool AddUpRow(Tx::Core::Table &resTable,		//��Ž����
				int iStyle,						//��������ķ�ʽ
				std::vector<int> ColVector,		//������Щ�н���ͳ��
				std::vector<int> IntCol,			//��Ҫ��ӵ�������
				std::vector<int> DoubleCol,	//��Ҫ��ӵ�double��
				std::vector<int> iDate,			//������
				int			  iCol				//���������ڵ��С�
				)	;
			bool AddUpRow(Tx::Core::Table &resTable,		//��Ž����
				int iStyle,						//��������ķ�ʽ
				std::vector<int> ColVector,		//������Щ�н���ͳ��
				std::vector<int> IntCol,			//��Ҫ��ӵ�������
				std::vector<int> DoubleCol,	//��Ҫ��ӵ�double��
				std::vector<int> iDate,			//������
				int			   iCol,				//���������ڵ��С�
				int			   iEquityCol,			//����ֵ���ڵ��С�
				int			   iFundIdCol,			//����ID���ڵ��С�
				int &			   iTradeId,		//��Ʊ����ծȯ�Ľ���ʵ��ID���ڵ��У��������ĺ�����������Ĵ�������ơ�
				int			   sortCol,		//������һ�н�������
				int			   pos,		//�������ڵ��С�
				bool             IsBuySell = false    //�ж��Ƿ������������ķ���ͳ�� add 2008-05-21
				)	;
			bool AddUpRow2(Tx::Core::Table &resTable,		//��Ž����
				std::vector<int> ColVector,		//������Щ�н���ͳ��
				std::vector<int> IntCol,			//��Ҫ��ӵ�������
				std::vector<int> DoubleCol,	//��Ҫ��ӵ�double��
				std::vector<int> iDate,			//������
				int			   iCol,				//���������ڵ��С�
				int &		   iTradeId,		//��Ʊ����ծȯ�Ľ���ʵ��ID���ڵ���
				int			   sortCol,		//������һ�н�������
				int			   pos,			//�������ڵ��С�
				bool            IsBuySell = false    //�ж��Ƿ������������ķ���ͳ��  add 2008-05-21
				)	;
			bool AddUpRow3(Tx::Core::Table &resTable,		//��Ž����
				//			  int iStyle,						//��������ķ�ʽ
				std::vector<int> ColVector,		//������Щ�н���ͳ��
				std::vector<int> IntCol,			//��Ҫ��ӵ�������
				std::vector<int> DoubleCol,	//��Ҫ��ӵ�double��
				std::vector<int> iDate,			//������
				int			   iCol,				//���������ڵ��С�
				int			   iEquityCol,				//����ֵ���ڵ��С�
				int			   iFundIdCol,			//����ID���ڵ��С�
				int 			   iTradeId,		//��Ʊ����ծȯ�Ľ���ʵ��ID���ڵ���
				int			   sortCol,		//������һ�н�������
				int			   pos,			//�������ڵ��С�
				bool             IsBuySell = false    //�ж��Ƿ������������ķ���ͳ��  add 2008-05-21
				)	;
			//������ר��Ϊ����Ӷ��ķ���ͳ��д�ĺ���2008-05-09
			bool AddUpRow(Tx::Core::Table &resTable,		//��Ž����
				int iStyle,						//��������ķ�ʽ
				std::vector<int> ColVector,		//������Щ�н���ͳ��
				std::vector<int> IntCol,			//��Ҫ��ӵ�������
				std::vector<int> DoubleCol,	//��Ҫ��ӵ�double��
				std::vector<int> iDate,			//������
				int			   iCol,				//���������ڵ��С�
				int &			   iTradeId,		//ȯ��ID���ڵ��У��������ĺ���������������ơ�
				int			   sortCol,		//������һ�н�������
				int			   pos			//�������ڵ��С�
				)	;
			bool AddUpRow4(Tx::Core::Table &resTable,		//��Ž����
				std::vector<int> ColVector,		//������Щ�н���ͳ��
				std::vector<int> IntCol,			//��Ҫ��ӵ�������
				std::vector<int> DoubleCol,	//��Ҫ��ӵ�double��
				std::vector<int> iDate,			//������
				int			   iCol,				//���������ڵ��С�
				int &		   iTradeId,		//ȯ��ID���ڵ���
				int			   sortCol,		//������һ�н�������
				int			   pos			//�������ڵ��С�
				);
			bool AddUpRow3(Tx::Core::Table &resTable,		//��Ž����
				std::vector<int> ColVector,		//������Щ�н���ͳ��
				std::vector<int> IntCol,			//��Ҫ��ӵ�������
				std::vector<int> DoubleCol,	//��Ҫ��ӵ�double��
				std::vector<int> iDate,			//������
				int			   iCol,				//���������ڵ��С�
				int 			   iTradeId,		//ȯ��ID���ڵ��У��������ĺ���������������ơ�
				int			   sortCol,		//������һ�н�������
				int			   pos			//�������ڵ��С�
				);
			bool AddUpRowEX(Tx::Core::Table &resTable,		//��Ž����
				int iStyle,						//��������ķ�ʽ
				std::vector<int> ColVector,		//������Щ�н���ͳ��  
				std::vector<int> IntCol,			//��Ҫ��ӵ�������
				std::vector<int> DoubleCol,	//��Ҫ��ӵ�double��
				std::vector<int> iDate,			//������
				int			   iCol,				//���������ڵ��С�
				int &			   iTradeCol,			//ȯ��ID���ڵ��У���������ǰ��������������ơ�
				int			   sortCol,		//������һ�н�������
				int			   pos			//�������ڵ��С�
				);	
			bool AddUpRowCompany(
				Tx::Core::Table &resTable,		//��Ž����
				std::vector<int> ColVector,		//������Щ�н���ͳ��  
				std::vector<int> IntCol,			//��Ҫ��ӵ�������
				std::vector<int> DoubleCol,	//��Ҫ��ӵ�double��
				std::vector<int> iDate,			//������
				int			  iCol,				//���������ڵ��С�
				int 		      iTradeCol,		//ȯ��ID���ڵ��У��������ĺ���������������ơ�
				int			   sortCol,		//������һ�н�������
				int			   pos			//�������ڵ��С�
				);	
			void AddUpAnalysis(Tx::Core::Table &resTable,		//��Ž�������
				std::vector<int> ColVector,		//������Щ�н���ͳ��
				bool	bCalculateValueRate,
				std::unordered_map<int,std::vector<int>> mapDate,
				Tx::Core::Table_Indicator & netValueTable
				);
			//��ȡ����==//30001232,	//������New��30001020,	//����˾ID��30001021	//�й�����ID
			//�ദ�ط��õ�����������������Ժ��޸�
			bool Get_Fund_Type_Manager_Company(Tx::Core::Table_Indicator &resTable,
				std::vector<int>	iSecurityId
				);
		public:
			//���������������� 2009-5-19
			bool FundSample( Tx::Core::Table_Display* pResTable,long lSecurityId);
			bool GetValueOfStock(Tx::Core::Table_Indicator &hbTable,long lSecurityId);
			//ͨ����ʼ��ֹ���ڻ�����䱨����,�������û�б����ڣ��ͷ�����ʼʱ��ǰ�ı����ڣ�
			void GetReportDateArr(std::vector<int>& VecDate,int iStart,int iEnd);
			double GetAvgNetValueReportDateArr(std::vector<int> vecDate, int SecId);
			//��netValueTableȡ��λ��ֵ
			double GetAvgNetValueReportDateArr(std::vector<int> vecDate, int SecId,Tx::Core::Table_Indicator & netValueTable);

		};//end of class TxFund
	}
}
#endif // !defined(AFX_TXFUND_H__5BA14E0A_27BC_4FD4_B281_1AEFC5CFB1D8__INCLUDED_)
