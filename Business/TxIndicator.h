/*************************************************
Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
File name:	TxIndicator.h
Author:       �Ժ꿡
Version:		1.0
Date:			2008-06-10

Description:	
ָ�������
��ͼͳһ�ӿ�
*************************************************/
#pragma once

#include "Business.h"
#include "TxBusiness.h"
#include "TxBond.h"
#include "TxWarrant.h"
#include "..\..\ArrayStatistics\TxArrayStatistics.h"
#include "..\..\ArrayStatistics\TxFunction.h"
#include "..\..\Data\DataFileStruct.h"
#include "..\..\core\driver\ClientFileEngine\public\LogRecorder.h"
#include "..\..\Core\Control\Manage.h"
#include <map>
#include <unordered_map>
#include "FundDeriveData.h"
#include "..\..\Data\XmlNodeWrapper.h"
#include "../../Core/Driver/ClientFileEngine/base/HttpDnAndUp.h"
#include "../../Core/Driver/ClientFileEngine/base/zip/ZipWrapper.h"

using namespace std;

namespace Tx
{
	namespace Business
	{
		//������������ǲ��������еĲ������ͣ��������������ﶼ���Ա���Ϊ��ͬ�Ĳ���
		struct TXFuncReq			//��������ṹ
		{
			INT iFuncID;			//����ID
			INT iSubItemID;			//��������ID
			LONG nIndicatorID;		//ָ��ID
			LONG nSecurityID;		//����ʵ��ID
			INT nDate;				//����
			INT nStartDate;			//ʱ���
			INT nEndDate;			//
			INT nOffsetDays;		//ƫ��
			INT iReportType;		//�������ͣ��ϲ�\ĸ��˾ 0,1
			INT iAccountingPolicyType;//��ƺ������� ����ǰ\������0,1
			INT iValueType;			
			DOUBLE dFindValue;
			INT	iPriceType;			//�۸�����
			DOUBLE dPrice;			//�۸�
			INT iIRType;			//��������
			INT nFYear;				//����
			INT nFQuarter;			//�Ƽ�
			INT iINDEXSampleIdx;
			INT iSubjectID;
			INT iHolderNo;			//�ɶ����
			INT nIndustryLevel;		//��ҵ����
			INT iQuotationNo;		
			TCHAR cName[32];
			INT iIndexFlg;
			INT iSYPL;			//����Ƶ�� 0-�� 1-�� 2- �� 3- �� 4-���� 5-��
			INT iYJLX;          //0--�ռ����ݣ�1--��������
		};

		//��ʷָ�꺯���������
		struct HisIndCalc_Param		
		{
			LONG *lpnShCalcEvtCnt;		//ʵ������
			time_t *lpnRefreshTime;		//ˢ��ʱ��
			BOOL *lpbThreadRun;			//�߳��Ƿ���������
			BOOL *lpbThreadTerm;		//�߳��Ƿ���ֹ
			BOOL *lpbsetAIsBusy;		//Aæ
			BOOL *lpbAnyShCalcEvt;
			ReqMap *pmapReq;			//��������
			StrSet *psetReqA;			//���󼯺�A
			StrSet *psetReqB;			//���󼯺�B
			CCriticalSection *pCS;		//��
			Tx::Business::TxBusiness *pBusiness;
			BOOL *lpbCalculate;			//���ڼ���
		};

		//ʵʩָ�꺯���������
		struct RealTimeIndCalc_Param
		{
			BOOL *lpbThreadRun;
			BOOL *lpbThreadTerm;
			time_t *lpnRefreshTimeRealTime;
			ReqMap *pmapReqRealTime;
			CCriticalSection *pCSRealTime;
		};

		//�������Map�����ṹ
		struct ClearReqMap_Param
		{
			BOOL *lpbThreadRun;
			BOOL *lpbThreadTerminated;
			BOOL *lpbAnyShCalcEvt;
			BOOL *lpbReqMapCleared;
			time_t *lpnRefreshTime;
			BOOL *lpbCalculate;
		};

		//�������ݲ����ṹ
		struct LoadData_Param
		{
			TxArrayStatistics *pArrayStatistics;
			CWnd *pParentWnd;
		};

		//ָ�����ڽṹ
		struct Indicator_Dates
		{
			INT nSecurity;
			INT nFirstDate;
			INT nSecondDate;
			DOUBLE dValue;
			INT nRow;
		};


		struct Indicator_Date
		{
			INT nSecurity;
			INT nDate;
			DOUBLE dValue;
			INT nRow;
		};

		struct Indicator_ShareHolder
		{
			INT nSecurity;
			INT nDate;
			INT iHolerNo;
			INT nRow;
		};
		//---by zhangxs----20080821----
		struct Indicator_FundManager
		{
			INT nSecurity;
			INT nDate;
			TCHAR cPosts[32];
			INT nRow;
		};
		//------------------------------
		//-----����ͳ�ƽṹ----------------
		struct ParamStatics
		{
			INT	iFuncID;	//����ID
			INT iCount;		//����
		};
		//------wangzhy--20080526----------


		//-----Excel�������ýṹ------
		enum	InvalidFormat
		{
			hyphenFormat = 0,	//"-"
			zeroFormat = 1,		//"0.0"
			nullFormat = 2,		//""
		};
		enum	DateFormat
		{
			numFormat = 0,		//********
			standardFormat = 1,	//****-**-**
		};
		struct ExcelCfg
		{
			int			invalidFormat;	//��Ч����
			int			dateFormat;		//��������
			int			calculateStyle; //ˢ�·�ʽ
		};
		struct BondRating 
		{
			int		iBondId;	//
			byte	f_bondrating[10];
		public:
			int GetMapObj(int index=0) { return iBondId; }
		};
		//��¶���ڣ���Բ�������
		struct  PublishDate
		{
			int		iId;
			int		iExpectedDate;
			int		iDate;
		public:
			int GetMapObj(int index=0) { return iId; }
		};
		//�嵲������������
		struct Bond5MmpYtm
		{
			double d_data[5];
		};

		class BUSINESS_EXT TxIndicator
		{
			TxIndicator(void);
		public:
			static TxIndicator* GetInstance(void)
			{
				static TxIndicator _ins;
				return &_ins;
			}
			virtual ~TxIndicator(void);

		private:
			//�Ƿ��ʼ���ı�־
			bool m_bInited;
			//�Ƿ��ʼ�����ݱ�
			bool m_bInitTables;
			//�����¼������߳�ʵ�ּ�ʱ��
			HANDLE m_hEvent_timer;
			HANDLE m_hEvent_ever;

			CWinThread *m_pThread_HisIndCalc, *m_pThread_RealTimeIndCalc, *m_pThread_ClearReqMap, *m_pThread_LoadData;
			BOOL m_bRun_HisIndCalc, m_bTerm_HisIndCalc, m_bRun_RealTimeIndCalc, m_bTerm_RealTimeIndCalc, m_bRun_ClearReqMap, m_bTerm_ClearReqMap;
			time_t m_nRefreshTime;
			time_t m_nRefreshTime_RealTime;
			LONG m_nShCalcEvtCnt;
			BOOL m_bAnyShCalcEvt;
			BOOL m_bReqMapCleared;
			BOOL m_bCalculate;
			BOOL m_bSynFlag;

			Tx::Business::TxBusiness *m_pBusiness;

			typedef vector <Indicator_Dates> vector_IndDates;
			typedef vector_IndDates::iterator iter_IndDates;

			typedef vector <Indicator_Date> vector_IndDate;
			typedef vector_IndDate::iterator iter_IndDate;

			typedef vector <Indicator_ShareHolder> vector_IndShareHolder;
			typedef vector_IndShareHolder::iterator iter_IndShareHolder;
			typedef pair <CString, Tx::Core::VariantData> ReqVarPair;
			//-------����ʲ���ծ���ݲ���--------
			set< int >	nBalanceSecuritySet;	//�洢�������ж��ٹ�˾
			set< int >  nBalanceDateSet;		//�洢�������ж��ٲ�ͬ�Ƽ�
			BOOL bBalanceInstitution;// = TRUE;	//Ĭ�ϰ��չ�˾����
			set< int >  nFinancialSecuritySet;
			set< int >	nFinancialDateSet;
			BOOL bFinancialInstitution;// = TRUE;
			set< int >  nIncomeSecuritySet;
			set< int >	nIncomeDateSet;
			BOOL bIncomeInstitution;// = TRUE;
			set< int >  nCashflowSecuritySet;
			set< int >	nCashflowDateSet;
			BOOL bCashflowInstitution;// = TRUE;
			set< int >  nRevenueSecuritySet;
			set< int >  nRevenueDateSet;
			BOOL bRevenueInstitution;// = TRUE;
			set< int >	nAccountReceivableSecuritySet;
			set< int >  nAccountReceivableDateSet;
			BOOL bAccountInstitution;// = TRUE;
			set< int >	nGainsAndLossesSecuritySet;
			set< int >  nGainsAndLossesDateSet;
			BOOL bGainsAndLossesInstitution;// = TRUE;
			set< int >  nDepreciationSecuritySet;
			set< int >  nDepreciationDateSet;
			BOOL bDepreciationInstitution;// = TRUE;
			set< int >  nFinanceChargeSecuritySet;
			set< int >  nFinanceChargeDateSet;
			BOOL bFinanceChargeInstitution;// = TRUE;
			//------------------------------zhangxs------20080815--------
			set< int >  nFundFinancialSecuritySet;  //������Ҫ����ָ��
			set< int >	nFundFinancialDateSet;
			BOOL bFundFinancialInstitution;// = TRUE;
			set< int >	nFundBalanceSecuritySet;	  //�����ʲ���ծ
			set< int >  nFundBalanceDateSet;		
			BOOL bFundBalanceInstitution;// = TRUE;	
			set< int >  nFundAchievementSet;    //��Ӫҵ��
			set< int >	nFundAchievementDateSet;
			BOOL bFundAchievementInstitution;// = TRUE;
			set< int >  nFundRevenueSet;	//�������
			set< int >	nFundRevenueDateSet;
			BOOL bFundRevenueInstitution;// = TRUE;
			set< int >  nFundNavChangeSet;	//��ֵ�䶯
			set< int >	nFundNavChangeDateSet;
			BOOL bFundNavChangeInstitution;// = TRUE;
			set< int >  nFundManagerSet;	//������
			set< int >	nFundManagerDateSet;
			BOOL bFundManagerInstitution;// = TRUE;
			set< int >  nFundShareSet;	//����ݶ�
			set< int >	nFundShareDateSet;
			BOOL bFundShareInstitution;// = TRUE;
			set< int >  nFundCombineInvestmentIndustryDistributeSet;	//����ϲ�Ͷ�ʲ�����ҵ�ֲ���
			set< int >	nFundCombineInvestmentIndustryDistributeDateSet;
			BOOL bFundCombineIndustryDistribute;// = TRUE;
			set< int >  nFundStockInvestmentIndustryDistributeSet;	//�������Ͷ�ʲ�����ҵ�ֲ���
			set< int >	nFundStockInvestmentIndustryDistributeDateSet;
			BOOL bFundStockIndustryDistribute;// = TRUE;
			set< int >  nFundIndexInvestmentIndustryDistributeSet;	//����ָ��Ͷ�ʲ�����ҵ�ֲ���
			set< int >	nFundIndexInvestmentIndustryDistributeDateSet;
			BOOL bFundIndexIndustryDistribute;// = TRUE;
			set< int >  nFundTXInvestmentIndustryDistributeSet;	//���Ͷ��������ҵ�ֲ���
			set< int >	nFundTXInvestmentIndustryDistributeDateSet;
			BOOL bFundTXIndustryDistribute;// = TRUE;

			set< int >  nFundCombineVIPStockSet;	//�ϲ�Ͷ�ʲ����زֹ�
			set< int >	nFundCombineVIPStockDateSet;
			BOOL bFundCombineVIPStockInstitution;// = TRUE;
			set< int >  nFundStockVIPStockeSet;	//���������زֹ�
			set< int >	nFundStockVIPStockDateSet;
			BOOL bFundStockVIPStockInstitution;// = TRUE;
			set< int >  nFundIndexVIPStockSet;	//ָ���������زֹ�
			set< int >	nFundIndexVIPStockDateSet;
			BOOL bFundIndexVIPStockInstitution;// = TRUE;
			set< int > nFundInvesmentGroupSet;	//����Ͷ�����
			set< int > nFundInvesmentGroupDateSet;
			BOOL bFundInvesmentGroup;

			set< int > nFundHoldStockDetail;	//����ֹ���ϸ
			set< int > nFundHoldStockDetailSet;
			BOOL bFundHolderStockDetail;

			set< int > nFundPositionInfo;	//����ֲֽṹ
			set< int > nFundPositionInfoSet;
			BOOL bFundPositionInfo;

			set< int > nFundShareDataChange;		//����ݶ�䶯
			set< int > nFundShareDataChangeSet;
			BOOL bFundShareDataChange;

			set< int > nFundBuyTotStock;			//�����ۼ������Ʊ
			set< int > nFundBuyTotStockSet;
			BOOL bFundBuyTotStock;

			set< int > nFundSaleTotStock;			//�����ۼ�������Ʊ
			set< int > nFundSaleTotStockSet;
			BOOL bFundSaleTotStock;

			set< int > nFundTradeVolumeInfo;		//ȯ��ϯλ
			set< int > nFundTradeVolumeInfoSet;
			BOOL bFundTradeVolumeInfo;
			//-----------------wangzhy-------------------------
			set< int > nFundBondPortfolio;			//ծȯ���
			set< int > nFundBondPortfolioDateSet;	
			BOOL bFundBondPortfolio;

			set< int > nFundBondHoldDetail;			//��ծ��ϸ
			set< int > nFundBondHoldDetailDateSet;
			BOOL bFundBondHoldDetail;	

			set< int > nFundCBondHoldDetail;		//�ֿ�תծ��ϸ
			set< int > nFundCBondHoldDetailDateSet;
			BOOL bFundCBondHoldDetail;	

			set< int > nFundOtherAsset;				//�����ʲ�
			set< int > nFundOtherAssetDateSet;
			BOOL bnFundOtherAsset;	

			set< int > nFlowOfCapitalSet;			//�ʽ�����
			set< int > nFlowOfCapitalDateSet;
			BOOL bFlowOfCapital;

			set< int > nPEPBSet;					//pepb
			set< int > nPEPBDateSet;
			BOOL bPePb;
			//----------------20080829-------------------------------
			//-------wangzhy------20080557-------
			set< int >  nHQSecuritySet;
			set< int >  nHQDateSet;
			BOOL bHQSecurity;// = TRUE--20080813

			//----------------------------
			set< int > nBalanceYHSet;					//Balance_YH
			set< int > nBalanceYHDateSet;
			BOOL bBalanceYH;

			set< int > nBalanceBXSet;					//Balance_BX
			set< int > nBalanceBXDateSet;
			BOOL bBalanceBX;

			set< int > nBalanceZQSet;					//Balance_ZQ
			set< int > nBalanceZQDateSet;
			BOOL bBalanceZQ;

			set< int > nProfitYHSet;					//Profit_YH
			set< int > nProfitYHDateSet;
			BOOL bProfitYH;

			set< int > nProfitBXSet;					//Profit_BX
			set< int > nProfitBXDateSet;
			BOOL bProfitBX;

			set< int > nProfitZQSet;					//Profit_ZQ
			set< int > nProfitZQDateSet;
			BOOL bProfitZQ;

			set< int > nCashFlowYHSet;					//CashFlow_YH
			set< int > nCashFlowYHDateSet;
			BOOL bCashFlowYH;

			set< int > nCashFlowBXSet;					//CashFlow_BX
			set< int > nCashFlowBXDateSet;
			BOOL bCashFlowBX;

			set< int > nCashFlowZQSet;					//CashFlow_ZQ
			set< int > nCashFlowZQDateSet;
			BOOL bCashFlowZQ;

			//-----����ֵ�������޸�--------
			set<int>	m_nNvrSample;
			set<pair<int,int>>	 m_nNvrDates;
			Tx::Core::Table			m_nNvrTable;
			map<pair<int,int>,int>	m_d2cMap;
			//--------20090106---------
			set<int> nBalanceStat;
			set<int> nBalanceStatDate;
			BOOL bBalanceStat;
			set<int> nProfitStat;
			set<int> nProfitStatDate;
			BOOL bProfitStat;
			set<int> nCashFlowStat;
			set<int> nCashFlowStatDate;
			BOOL bCashFlowStat;
			TXFuncReq reqTXFunction;
			TXFuncReq reqRealTime;
			HisIndCalc_Param param_HisIndCalc;
			RealTimeIndCalc_Param param_RealTimeIndCalc;
			ClearReqMap_Param param_ClearReqMap;
			LoadData_Param param_LoadData;

			/*ĿǰVBA�嵵�����̺�����ȡ����ȱ�ݣ���һ��ȡ������ȷ��ֵ����������ڶ��β��ܵõ���Чֵ
			������VBA���󻺴�Map��ʽ��ֻ������һ�Σ�ֱ������������ٴ�����
			Mantis12390�Ĵ������ǣ�����õ�������Чֵ����������󣬽������嵵�����̡�����������
			Mantis12390�Ĵ���������ȱ�ݣ����ֵ������Чֵʱ��һֱ����
			��������������ʱ��һ����������������˶���򽫸�ֵ���뻺�棬Ҳ���ǲ������ٴ�����
			*/
			//2012-10-23
			std::unordered_map<CString,int> mapRequest_limit;

			//VBA���󻺴�Map
			ReqMap mapRequest;			
			//VBA������У���A��B�������У�һ�����ڻ���VBA����һ������ָ�����
			StrSet setReqA;				//VBA�������A
			StrSet setReqB;				//VBA�������B
			BOOL bsetAIsBusy;			//TRUE��setAæµ������ָ����㣬FALSE��setA���У����ڻ���VBA����
			CCriticalSection _critsect;	//�ٽ����������̻߳���

			//ʵʱָ���VBA���󻺴�Map
			ReqMap mapReqRealTime;

			//������߻���ʵʱָ��������ٽ����������̻߳���
			CCriticalSection _critsect_RealTime;
		public:
			//<iGroupID, TXEE_FuncGroup>
			TXEE_FuncGroup_Map mapTXEEFuncGroup;

			//<iFuncID, TXEE_Function>
			TXEE_Function_Map mapTXEEFunction;

			//<iFuncID * 100 + iSubItemID, TXEE_Indicator>
			TXEE_Indicator_Map mapTXEEIndicator;

			//<iParamID, TXEE_Param>
			TXEE_Param_Map mapTXEEParam;

			//<iFuncID * 100 + iParamIdx, iParamID>
			INT_Map mapTXEEParamOfFunc;

			//<iFuncID * 100 + iSubItemID, CString>
			INTSTR_Map mapTXEEItem;

			//<iFuncID, iIndicatorID, iIndustryID>
			map<INT,INT_Map> mmapIndicator2IndustryID;

		private:
			//<strReq, strValue>
			StrMap mapSheetReq;
			map <CString, Tx::Core::VariantData> mapSheetReqVariantData;

			//<Name��Code>
			StrMMap mmapNameCode;

			//<����ʵ��Code, ����ʵ��ID>
			STRLONG_Map mapCodeID;
			//2012-12-25
			//����<����ʵ��Code, ����ʵ��ID>
			STRLONG_Map mapFundCodeID;
			//ծȯ<����ʵ��Code, ����ʵ��ID>
			STRLONG_Map mapBondCodeID;

			//��ҵ�ֲ�-֤���(��)  2013-04-22
			//--30377/�Բ��ꡢ�Ƽ�Ϊ�ļ�����
			//<������,<"����ʵ��ID+��ҵID"����ҵ��ֵ>>
			//--30378/�Խ���ʵ��IDΪ�ļ���
			//<����ʵ��ID,<"������+��ҵID"����ҵ��ֵ>>
			//map<int,map<CString,double>> mapFundIndustryDistribute;

			map<int,map<int,map<int,double>>> mapFundIndustryDistribute;

			//CWnd *pExcelWnd;

			//0��ʮ��ɶ���1��ʮ����ͨ�ɶ���2�����ʽ�����ʮ������ˣ�3����תծ��ʮ�������
			Tx::Core::Table_Indicator tableShareHolder[4];

			//0����Ʊ����(�׷�/����)��1��Ȩ֤����ע����2����תծת�ɼ۸�3����תծ���ʣ�4����תծת��������5��ծȯ������Ϣ
			Tx::Core::Table_Indicator tableIndDate[6];

			//0������ֵ����
			Tx::Core::Table_Indicator tableIndDates[1];

			//0�������ʲ���ֵ��1����תծ������Ϣ��2����Ҫ����ָ�ꣻ3���ʲ���ծ��
			//4��������䣻5���ֽ�������6���ɱ��ṹ��7���ֹ�������8����Ӫҵ������ֲ���
			//9���ʲ���ֵ׼����10���Ǿ��������棻11��Ӧ���ʿ12���������
			Tx::Core::Table_Indicator tableIndID[13];

			//-----------------zhangxs--20080821---------
			Tx::Core::Table_Indicator m_tableFundManager;
			Tx::Core::Table_Indicator m_tableBondTmp;
			//--------------------------------------------
			//------------------wangzhy-----------------
			Tx::Core::Table_Indicator m_tableQS;
			//------------------20080829----------------
			//��ָ֤��
			INT iSHIndexID;// = 4000208;
			//���ڳ�ָ
			INT iSZIndexID;// = 4000001;
		public:
			//------wangzhy----------
			ExcelCfg m_Cfg;
			//-------20090403--------
		public:
			void InitTXEEFunctions();
			void InitTables(Tx::Business::TxBusiness *pBusiness, bool bDownAll = true );
			void ClearTables();

			//�������ݹ��̣���Ϊ�߳��ڲ���������
			UINT LoadData_Proc(LPVOID lpParam);
			//�������Map
			UINT ClearReqMap_Proc(LPVOID lpParam);
			//��ʷָ��������
			UINT HisIndCalc_Proc(LPVOID lpParam);
			UINT RealTimeIndCalc_Proc(LPVOID lpParam);
			void FindItemOfReq(TXFuncReq *preqTXFunction, CString & strRequest, int iItemFlg = 0);
			DOUBLE GetKLine(Tx::Business::TxBusiness *pBusiness, int nDate, int iType,int m_iSecId);
			DOUBLE GetKLineNew(Tx::Business::TxBusiness *pBusiness, int nDate, int iType,int m_iSecId);//20091215
			DOUBLE GetKLineEx(Tx::Business::TxBusiness *pBusiness, int nDate, int iType,int m_iSecId);//20080813
			DOUBLE GetKLineExNew(Tx::Business::TxBusiness *pBusiness, int nDate, int iType,int m_iSecId);//20091215
			int GetTradeDateByOffset(Tx::Business::TxBusiness *pBusiness, int nDate, int nOffsetDays);
			int GetTradeDays(Tx::Business::TxBusiness *pBusiness, int nStartDate, int nEndDate);
			DOUBLE GetSumVolume(Tx::Business::TxBusiness *pBusiness, int nStartDate, int nEndDate);
			DOUBLE GetSumAmount(Tx::Business::TxBusiness *pBusiness, int nStartDate, int nEndDate);
			DOUBLE GetSumExchangeRatio(Tx::Business::TxBusiness *pBusiness, int nStartDate, int nEndDate);
			DOUBLE GetMMFGrowth(Tx::Business::TxBusiness *pBusiness, int nStartDate, int nEndDate);
			DOUBLE GetDatesIndicator_Value(INT nSecurity, INT nFirstDate, INT nSecondDate, vector_IndDates *pvIndDates);
			INT GetDatesIndicator_Row(INT nSecurity, INT nFirstDate, INT nSecondDate, vector_IndDates *pvIndDates);
			void SetDatesIndicator(vector_IndDates *pvIndDates, int nTblIdx);
			DOUBLE GetDateIndicator_Value(INT nSecurity, INT nDate, vector_IndDate *pvIndDate);
			INT GetDateIndicator_Row(INT nSecurity, INT nDate, vector_IndDate *pvIndDate);
			void SetDateIndicator(vector_IndDate *pvIndDate, int nTblIdx);
			INT GetShareHolderIndicatorRow(INT nSecurity, INT nDate, INT iHolderNo, vector_IndShareHolder *pvIndShareHolder);
			void SetShareHolderIndicator(vector_IndShareHolder *pvIndShareHolder, int nTblIdx);
			int Get_Value_Occur_Date(Tx::Business::TxBusiness *pBusiness, DOUBLE dFindValue, int iValueType, int nStartDate, int nEndDate);
			int Get_Price_Occur_Date(Tx::Business::TxBusiness *pBusiness, DOUBLE dPrice, int iPriceType, int iIRType, int nStartDate, int nEndDate);
			int Get_Value_Extremum_Date(Tx::Business::TxBusiness *pBusiness, int iValueType, int nStartDate, int nEndDate, BOOL bMinimum);
			int Get_Price_Extremum_Date(Tx::Business::TxBusiness *pBusiness, int iPriceType, int nStartDate, int nEndDate, BOOL bMinimum);
			//-------------------------------------------
			bool	IsSampleOfIndex( int iSecurity, int iIndex );
			double	Get_PE_Basic( int iSecurity, int iDate, bool bSecurity = true );
			double	Get_PE_Basic_NEW( int iSecurity, int iDate, bool bSecurity = true );
			double	Get_Profit_PE_Basic( Tx::Business::TxBusiness *pBusiness, int& iDate, bool bSecurity = true);
			double  Get_PE_Static( int iSecurity, int iDate, bool bSecurity = true );    //��̬PE add by wangzf   2012-3-23
			double	Get_PE_Ltm( int iSecurity, int iDate, bool bSecurity = true );
			double	Get_PE_Ltm_NEW( int iSecurity, int iDate, bool bSecurity = true );
			double	Get_Profit_PE_Lmt( Tx::Business::TxBusiness *pBusiness, int& iDate, bool bSecurity = true);
			double	Get_PE_Yoy( int iSecurity, int iDate, bool bSecurity = true );
			double	Get_Profit_From_Date( Tx::Business::TxBusiness *pBusiness, int iYear,int iQuarter, bool bSecurity = true );
			double	Get_Profit_PE_Yoy( Tx::Business::TxBusiness *pBusiness, int& iDate, bool bSecurity = true );
			double	Get_PE_Yoy_NEW( int iSecurity, int iDate, bool bSecurity = true );
			double	Get_PB( int iSecurity, int iDate, bool bSecurity = true );
			double	Get_PE_Basic( std::set<int> iSet, int iDate, bool bSecurity = true );
			double	Get_PE_Ltm( std::set<int> iSet, int iDate, bool bSecurity = true );
			double	Get_PE_Yoy( std::set<int> iSet, int iDate, bool bSecurity = true );
			double	Get_PB( std::set<int> iSet, int iDate, bool bSecurity = true );
			int		Get_FinancialDate(int iYear, int iQuarter );
			int		Get_PreFinancailDate(int iYear, int iQuarter );
			double	Get_Earing_Basic( int iSecurity, int iDate );
			CString	Get_QS_Name( int nID );
			//��ȡ��ҵ�ֲ���ֵ(֤���(��))
			double  GetFundCombineIndustryDistribute(int nSecurityId,int nReport,int nIndustryId);
			//added by zhangxs 20090304
			CString Text2Date(CString m_str);
			CString Date2Text(CString m_str);
			//-------------------------------------------
			//����ʵ�壬��ʼ���ڣ���ֹ���ڣ����ڣ����ظ�ʽ
			CString GetDateSeries( int nSecurity, int nBegin, int nEnd, int nCycle = 0, int nStyle = 0 );
			//-----by zhangxs------20080821---
			//DOUBLE GetFundManagerIndicator_Value(INT nSecurity, INT nDate, vector_IndDate *pvIndDate);
			//INT GetFundManagerIndicator_Row(INT nSecurity, INT nDate, vector_IndDate *pvIndDate);
			//void SetFundManagerIndicator(vector_IndDate *pvIndDate, int nTblIdx);

			//----------------------------------

			CString GetRefreshTime(void);
			void SendShCalcMsg(void);
			CString GetRefreshTime_RealTime(void);
			LONG GetSecurityID(LPCTSTR lpszSecurity, int nType = 0 );

			/// <summary>
			///  ����ָ�����뷵��ָ�������롣
			/// </summary>
			LONG GetIndexSecurityID(LPCTSTR lpszSecurity);

			LONG GetSecurityIDEx(LPCTSTR lpszSecurity, int nType = 0 );   //��ȡծȯ����ʵ��ID  2012-12-25
			LONG GetFund_SecurityID(LPCTSTR lpszSecurity, int nType = 0 );//��ȡ������ʵ��ID  2012-12-25

			SHORT GetItemID(SHORT iFuncID, LPCTSTR lpszItem);
			LONG GetIndicatorID2IndustryID(SHORT iFuncID, LPCTSTR lpszItem);  //ͨ��ָ��ID��ȡ��ҵID
			SHORT GetReportType(LPCTSTR lpszReportType);
			SHORT GetAccountingPolicyType(LPCTSTR lpszAccountingPolicyType);
			SHORT GetIRType(LPCTSTR lpszIRType);
			SHORT GetPriceType(LPCTSTR lpszPriceType);
			SHORT GetValueType(LPCTSTR lpszValueType);
			SHORT GetPFADSubject(LPCTSTR lpszPFADSubject);

			SHORT Get_Fund_ReportType(LPCTSTR lpszReportType);  //��ò���Ƽ���Ϣ by zhangxs 20080808

			void StartHisCalcThread(void);
			void StartRealCalcThread(void);

			void EndHisCalcThread(void);
			void EndRealCalcThread(void);
		private:
			bool m_bIsHisThreadStarted;
			bool m_bIsRealThreadStarted;
		public:
			CString GetIndicator(LPCTSTR lpszRequest,Tx::Core::VariantData& vdReq = Tx::Core::VariantData(_T("")));
			CString GetRealTimeIndicator(LPCTSTR lpszRequest,Tx::Core::VariantData& vdReq = Tx::Core::VariantData(_T("")));
			//2008-06-16
			//ͬ��������ʷָ��
			CString GetIndicatorSyn(LPCTSTR lpszRequest,Tx::Core::VariantData& vdReq = Tx::Core::VariantData(_T("")));
			CString HisIndCalcProcSyn(LPCTSTR lpszRequest,Tx::Core::VariantData& vdReq = Tx::Core::VariantData(_T("")));


			//����Ϊ���Ż�ͬ���������ӵĳ�Ա����
			Tx::Data::Balance*	m_pBalance;	//---20080528	
			DataFileNormal<blk_TxExFile_FileHead,Tx::Data::Balance>* m_pBalanceDataFile;//20080529
			Tx::Data::Financial*	m_pFinancial;
			DataFileNormal<blk_TxExFile_FileHead,Tx::Data::Financial>* m_pFinancialDataFile;
			Tx::Data::CashFlow*	m_pCashFlow;
			DataFileNormal<blk_TxExFile_FileHead,Tx::Data::CashFlow>* m_pCashFlowDataFile;
			Tx::Data::Income*	m_pIncome;
			DataFileNormal<blk_TxExFile_FileHead,Tx::Data::Income>* m_pIncomeDataFile;
			//wangzhy-------------20080527
			Tx::Data::Holder*	m_pHolder;	//--wangzhy--20080603
			Tx::Data::TradeableHolder*	m_pTradeableHolder;	//--wangzhy--20080603
			Tx::Data::FundBondHolder*	m_pFundBondHolder;		//--wangzhy--20080604
			Tx::Data::PrimeOperatingRevenue*	m_pRevenue;	//--wangzhy--20080604
			DataFileNormal<blk_TxExFile_FileHead,Tx::Data::PrimeOperatingRevenue>* m_pRevenueDataFile;//20080605
			Tx::Data::AssetsDepreciationReserves*	m_pDepreciation;	//--wangzhy--20080604
			DataFileNormal<blk_TxExFile_FileHead,Tx::Data::AssetsDepreciationReserves>* m_pDepreciationDataFile;//20080610
			Tx::Data::NonRecurringGainsAndLosses*	m_pGainsAndLosses;	//--wangzhy--20080604
			DataFileNormal<blk_TxExFile_FileHead,Tx::Data::NonRecurringGainsAndLosses>* m_pGainsAndLossesDataFile;//20080610
			Tx::Data::AccountsReceivable*	m_pAccountReceivable;	//--wangzhy--20080604
			DataFileNormal<blk_TxExFile_FileHead,Tx::Data::AccountsReceivable>* m_pAccountsReceivableDataFile;//20080610
			Tx::Data::FinanceCharge*	m_pFinanceCharge;	//--wangzhy--20080604
			DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FinanceCharge>* m_pFinanceChargeDataFile;//20080610
			Tx::Data::HisTradeDataCustomStruct* m_pHisDetail;//20080711
			//zhangxs-----------------------20080822-------
			Tx::Data::FundAchievement*	m_pFundArchievement;
			DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundAchievement>* m_pFundArchievementDataFile;
			Tx::Data::FundFinancial*	m_pFundFinancial;
			DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundFinancial>* m_pFundFinancialDataFile;
			Tx::Data::FundBalance*		m_pFundBalance;
			DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundBalance>* m_pFundBalanceDataFile;
			Tx::Data::FundNavChange*	m_pFundNavChange;
			DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundNavChange>* m_pFundNavChangeDataFile;
			Tx::Data::FundRevenue*		m_pFundRevenue;
			DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundRevenue>* m_pFundRevenueDataFile;
			Tx::Data::FundBlockInvestmentTxIndustryDistribute*	m_pFundTxIndustryDistribute;
			DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundBlockInvestmentTxIndustryDistribute>* m_pFundTxIndustryDistributeDataFile;
			Tx::Data::FundCombineInvestmentIndustryDistribute*	m_pFundComIndustryDistribute;
			DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundCombineInvestmentIndustryDistribute>* m_pFundComIndustryDistributeDataFile;
			Tx::Data::FundIndexInvestmentIndustryDistribute*	m_pFundIndexIndustryDistribute;
			DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundIndexInvestmentIndustryDistribute>* m_pFundIndexIndustryDistributeDataFile;
			Tx::Data::FundStockInvestmentIndustryDistribute*	m_pFundActiveIndustryDistribute;
			DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundStockInvestmentIndustryDistribute>* m_pFundActiveIndustryDistributeDataFile;
			Tx::Data::FundInvesmentGroup* m_pFundInvesmentGroup;
			DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundInvesmentGroup>* m_pFundInvesmentGroupDataFile;
			Tx::Data::Fund_VIP_Stock*  m_pFundstockVipStock;
			DataFileNormal<blk_TxExFile_FileHead,Tx::Data::Fund_VIP_Stock>* m_pFundstockVipStockDataFile;
			Tx::Data::FundIndexInvestmentVIPStock* m_pFundIndexVIPStock;
			DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundIndexInvestmentVIPStock>* m_pFundIndexVIPStockDataFile;
			Tx::Data::FundCombineInvestmentVIPStock*	m_pFundCombinVIPStock;
			DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundCombineInvestmentVIPStock>* m_pFundCombinVIPStockDataFile;

			Tx::Data::AmountFlow*	m_pAmountFlow;
			//--ǿ�����¼��㣬������еļ�����--20080801--
			UINT ClearResult_Proc(void);
			//------����ֵ---------------
			Tx::Core::Table			m_dFundNvrTable;	//��λ��ֵ
			std::unordered_map< int,int >		m_FundNvrRowMap;	//������
			std::unordered_map< int,int >		m_FundNvrColMap;	//������

			Tx::Core::Table			m_dFundAcuNvrTable;	//�ۼƾ�ֵ
			std::unordered_map< int,int >		m_dFundAcuNvrRowMap;//������
			std::unordered_map< int,int >		m_dFundAcuNvrColMap;//������
			//-------20090306----------------
			//-------���һ���ֵ--------------------
			Tx::Core::Table			m_dMmfFundNvrTable;	//��λ��ֵ
			std::unordered_map< int,int >		m_MmfFundNvrRowMap;	//������
			std::unordered_map< int,int >		m_MmfFundNvrColMap;	//������

			Tx::Core::Table			m_dMmfFundAcuNvrTable;	//�ۼƾ�ֵ
			std::unordered_map< int,int >		m_dMmfFundAcuNvrRowMap;//������
			std::unordered_map< int,int >		m_dMmfFundAcuNvrColMap;//������
			//----------20090504------------------------------------

			//--------------��ȡ����λ��ֵ-----------------
			bool	GetFundNvr( std::set< int > nSecurity,std::set< int > nDates, Tx::Core::Table* pResTable = NULL );
			double	GetFundNvrVal( int iSecurity,int iDate );
			//--------------��ȡ�����ۼƾ�ֵ-----------------
			bool	GetFundAcuNvr( std::set< int > nSecurity,std::set< int > nDates, Tx::Core::Table* pResTable = NULL );
			double	GetFundAcuNvrVal( int iSecurity,int iDate );
			//--------------��ȡ���һ����������-------------
			bool	GetMmfFundNvr( std::set< int > nSecurity,std::set< int > nDates, Tx::Core::Table* pResTable = NULL );
			double	GetMmfFundNvrVal( int iSecurity,int iDate );
			//--------------��ȡ����7���껯------------------
			bool	GetMmfFundAcuNvr( std::set< int > nSecurity,std::set< int > nDates, Tx::Core::Table* pResTable = NULL );
			double	GetMmfFundAcuNvrVal( int iSecurity,int iDate );


			//------����ֵ---------------
			Tx::Core::Table			m_dFundNvrExtTable;	//��λ��ֵ
			std::unordered_map< int,int >		m_FundNvrRowExtMap;	//������
			std::unordered_map< int,int >		m_FundNvrColExtMap;	//������

			Tx::Core::Table			m_dFundAcuNvrExtTable;	//�ۼƾ�ֵ
			std::unordered_map< int,int >		m_dFundAcuNvrRowExtMap;//������
			std::unordered_map< int,int >		m_dFundAcuNvrColExtMap;//������
			//-------20090306----------------
			//-------���һ���ֵ--------------------
			Tx::Core::Table			m_dMmfFundNvrExtTable;	//��λ��ֵ
			std::unordered_map< int,int >		m_MmfFundNvrRowExtMap;	//������
			std::unordered_map< int,int >		m_MmfFundNvrColExtMap;	//������

			Tx::Core::Table			m_dMmfFundAcuNvrExtTable;	//�ۼƾ�ֵ
			std::unordered_map< int,int >		m_dMmfFundAcuNvrRowExtMap;//������
			std::unordered_map< int,int >		m_dMmfFundAcuNvrColExtMap;//������
			//----------20090504------------------------------------
			//--------------��ȡ����λ��ֵext-----------------
			bool	GetFundNvrExt( std::set< int > nSecurity,std::set< int > nDates, Tx::Core::Table* pResTable = NULL );
			double	GetFundNvrExtVal( int iSecurity,int iDate );
			//--------------��ȡ�����ۼƾ�ֵext-----------------
			bool	GetFundAcuNvrExt( std::set< int > nSecurity,std::set< int > nDates, Tx::Core::Table* pResTable = NULL );
			double	GetFundAcuNvrExtVal( int iSecurity,int iDate );
			//--------------��ȡ���һ����������ext-------------
			bool	GetMmfFundNvrExt( std::set< int > nSecurity,std::set< int > nDates, Tx::Core::Table* pResTable = NULL );
			double	GetMmfFundNvrExtVal( int iSecurity,int iDate );
			//--------------��ȡ����7���껯ext------------------
			bool	GetMmfFundAcuNvrExt( std::set< int > nSecurity,std::set< int > nDates, Tx::Core::Table* pResTable = NULL );
			double	GetMmfFundAcuNvrExtVal( int iSecurity,int iDate );

			//�޸�ͬ���첽��ʽ
			bool	SetSynStyle( bool bSynFalg );
			bool	GetSynStyle( void );
			//������ȡ�·�ʽ
			CString GetLatestReport( CString strRequest );
			CString	DownLoad( CString strReq,CString strAddress );
			//��ȡ�ɽ���ϸ����
			//��ȡ��ϸ��������
			int		GetTradeDetailCount( int iSecurity,int iDate );
			Tx::Data::TradeDetailData* GetTradeDetailData( int iSecurity,int iDate,int iSeq ); 
			CString Test( CString strReq );
			//��ȡ����
			bool	GetExcelConfig(void);
			//��������
			bool	SavaExcelConfig(void);
			//�ļ�����,���ص�������,������λ��,������λ����
			CString	GetXmlData( CString strFineName,CString strCol,CString strKey= _T(""),CString strCol1 = _T("") );
			std::set<int> m_setSWIndex;

			std::unordered_map<int,CString> FundNameMap;	//֤���淶������map

			//ծȯYTM[ʵʱ]
			std::unordered_map<int,BondYTM> m_AllYtmMap;
			void GetBondYTM(void); 
			void GetBondYTM(int nSecurityId,double &dytm,double &dDur,double &dMDur,double &dCon,bool bCur=true);

			//�����嵲�����̵���������
			std::unordered_map<int,double> m_bpytm[5];
			std::unordered_map<int,double> m_spytm[5];

			void GetMmpBondYtm(int no,bool mmp=true);  //no���̺�1,2,3,4,5��mmp:true-���̣�flase-���̣�
			double GetMmpBondYtm(int iSecurityId,int no,bool mmp=true );   //no���̺�1,2,3,4,5��mmp:true-���̣�flase-���̣�
		};
		class SecurityNamePred
		{
		public:
			SecurityNamePred(CString name)
			{
				m_name = format(name);
			}
			bool operator()(pair<CString,CString> iter)
			{
				CString input = format(iter.first);
				bool ret = input.CompareNoCase(m_name) == 0;
				return ret;
			}
		private:
			CString format(const CString & input)
			{
				CString tmp(input);
				//tmp.Remove(_T(' '));//���

				char out[255];
				memset(out,0,255);
				::LCMapString(LANG_SYSTEM_DEFAULT,LCMAP_FULLWIDTH,tmp,255,out,255);
				CString ret(out);
				ret.Remove(_T('��'));//ȫ��
				return ret;
			}
			CString m_name;
		};
	}
}