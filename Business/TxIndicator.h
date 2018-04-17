/*************************************************
Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
File name:	TxIndicator.h
Author:       赵宏俊
Version:		1.0
Date:			2008-06-10

Description:	
指标计算类
试图统一接口
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
		//这里包含了我们参数中所有的参数类型，各种请求在这里都可以表现为不同的参数
		struct TXFuncReq			//函数请求结构
		{
			INT iFuncID;			//函数ID
			INT iSubItemID;			//函数返回ID
			LONG nIndicatorID;		//指标ID
			LONG nSecurityID;		//交易实体ID
			INT nDate;				//日期
			INT nStartDate;			//时间段
			INT nEndDate;			//
			INT nOffsetDays;		//偏移
			INT iReportType;		//报告类型，合并\母公司 0,1
			INT iAccountingPolicyType;//会计核算类型 调整前\调整后0,1
			INT iValueType;			
			DOUBLE dFindValue;
			INT	iPriceType;			//价格类型
			DOUBLE dPrice;			//价格
			INT iIRType;			//返回类型
			INT nFYear;				//财年
			INT nFQuarter;			//财季
			INT iINDEXSampleIdx;
			INT iSubjectID;
			INT iHolderNo;			//股东序号
			INT nIndustryLevel;		//行业分类
			INT iQuotationNo;		
			TCHAR cName[32];
			INT iIndexFlg;
			INT iSYPL;			//收益频率 0-日 1-周 2- 月 3- 季 4-半年 5-年
			INT iYJLX;          //0--日间数据，1--日终数据
		};

		//历史指标函数计算参数
		struct HisIndCalc_Param		
		{
			LONG *lpnShCalcEvtCnt;		//实例个数
			time_t *lpnRefreshTime;		//刷新时间
			BOOL *lpbThreadRun;			//线程是否正在运行
			BOOL *lpbThreadTerm;		//线程是否终止
			BOOL *lpbsetAIsBusy;		//A忙
			BOOL *lpbAnyShCalcEvt;
			ReqMap *pmapReq;			//结果缓存表
			StrSet *psetReqA;			//请求集合A
			StrSet *psetReqB;			//请求集合B
			CCriticalSection *pCS;		//锁
			Tx::Business::TxBusiness *pBusiness;
			BOOL *lpbCalculate;			//正在计算
		};

		//实施指标函数计算参数
		struct RealTimeIndCalc_Param
		{
			BOOL *lpbThreadRun;
			BOOL *lpbThreadTerm;
			time_t *lpnRefreshTimeRealTime;
			ReqMap *pmapReqRealTime;
			CCriticalSection *pCSRealTime;
		};

		//清空请求Map参数结构
		struct ClearReqMap_Param
		{
			BOOL *lpbThreadRun;
			BOOL *lpbThreadTerminated;
			BOOL *lpbAnyShCalcEvt;
			BOOL *lpbReqMapCleared;
			time_t *lpnRefreshTime;
			BOOL *lpbCalculate;
		};

		//下载数据参数结构
		struct LoadData_Param
		{
			TxArrayStatistics *pArrayStatistics;
			CWnd *pParentWnd;
		};

		//指标日期结构
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
		//-----参数统计结构----------------
		struct ParamStatics
		{
			INT	iFuncID;	//函数ID
			INT iCount;		//个数
		};
		//------wangzhy--20080526----------


		//-----Excel引擎设置结构------
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
			int			invalidFormat;	//无效设置
			int			dateFormat;		//日期设置
			int			calculateStyle; //刷新方式
		};
		struct BondRating 
		{
			int		iBondId;	//
			byte	f_bondrating[10];
		public:
			int GetMapObj(int index=0) { return iBondId; }
		};
		//披露日期，针对财务数据
		struct  PublishDate
		{
			int		iId;
			int		iExpectedDate;
			int		iDate;
		public:
			int GetMapObj(int index=0) { return iId; }
		};
		//五挡买卖盘收益率
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
			//是否初始化的标志
			bool m_bInited;
			//是否初始化数据表
			bool m_bInitTables;
			//利用事件控制线程实现计时器
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
			//-------针对资产负债数据测试--------
			set< int >	nBalanceSecuritySet;	//存储参数中有多少公司
			set< int >  nBalanceDateSet;		//存储参数中有多少不同财季
			BOOL bBalanceInstitution;// = TRUE;	//默认按照公司加载
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
			set< int >  nFundFinancialSecuritySet;  //基金主要财务指标
			set< int >	nFundFinancialDateSet;
			BOOL bFundFinancialInstitution;// = TRUE;
			set< int >	nFundBalanceSecuritySet;	  //基金资产负债
			set< int >  nFundBalanceDateSet;		
			BOOL bFundBalanceInstitution;// = TRUE;	
			set< int >  nFundAchievementSet;    //经营业绩
			set< int >	nFundAchievementDateSet;
			BOOL bFundAchievementInstitution;// = TRUE;
			set< int >  nFundRevenueSet;	//收益分配
			set< int >	nFundRevenueDateSet;
			BOOL bFundRevenueInstitution;// = TRUE;
			set< int >  nFundNavChangeSet;	//净值变动
			set< int >	nFundNavChangeDateSet;
			BOOL bFundNavChangeInstitution;// = TRUE;
			set< int >  nFundManagerSet;	//基金经理
			set< int >	nFundManagerDateSet;
			BOOL bFundManagerInstitution;// = TRUE;
			set< int >  nFundShareSet;	//基金份额
			set< int >	nFundShareDateSet;
			BOOL bFundShareInstitution;// = TRUE;
			set< int >  nFundCombineInvestmentIndustryDistributeSet;	//基金合并投资部分行业分布表
			set< int >	nFundCombineInvestmentIndustryDistributeDateSet;
			BOOL bFundCombineIndustryDistribute;// = TRUE;
			set< int >  nFundStockInvestmentIndustryDistributeSet;	//基金积极投资部分行业分布表
			set< int >	nFundStockInvestmentIndustryDistributeDateSet;
			BOOL bFundStockIndustryDistribute;// = TRUE;
			set< int >  nFundIndexInvestmentIndustryDistributeSet;	//基金指数投资部分行业分布表
			set< int >	nFundIndexInvestmentIndustryDistributeDateSet;
			BOOL bFundIndexIndustryDistribute;// = TRUE;
			set< int >  nFundTXInvestmentIndustryDistributeSet;	//组合投资天相行业分布表
			set< int >	nFundTXInvestmentIndustryDistributeDateSet;
			BOOL bFundTXIndustryDistribute;// = TRUE;

			set< int >  nFundCombineVIPStockSet;	//合并投资部分重仓股
			set< int >	nFundCombineVIPStockDateSet;
			BOOL bFundCombineVIPStockInstitution;// = TRUE;
			set< int >  nFundStockVIPStockeSet;	//积极部分重仓股
			set< int >	nFundStockVIPStockDateSet;
			BOOL bFundStockVIPStockInstitution;// = TRUE;
			set< int >  nFundIndexVIPStockSet;	//指数化部分重仓股
			set< int >	nFundIndexVIPStockDateSet;
			BOOL bFundIndexVIPStockInstitution;// = TRUE;
			set< int > nFundInvesmentGroupSet;	//基金投资组合
			set< int > nFundInvesmentGroupDateSet;
			BOOL bFundInvesmentGroup;

			set< int > nFundHoldStockDetail;	//基金持股明细
			set< int > nFundHoldStockDetailSet;
			BOOL bFundHolderStockDetail;

			set< int > nFundPositionInfo;	//基金持仓结构
			set< int > nFundPositionInfoSet;
			BOOL bFundPositionInfo;

			set< int > nFundShareDataChange;		//基金份额变动
			set< int > nFundShareDataChangeSet;
			BOOL bFundShareDataChange;

			set< int > nFundBuyTotStock;			//基金累计买入股票
			set< int > nFundBuyTotStockSet;
			BOOL bFundBuyTotStock;

			set< int > nFundSaleTotStock;			//基金累计卖出股票
			set< int > nFundSaleTotStockSet;
			BOOL bFundSaleTotStock;

			set< int > nFundTradeVolumeInfo;		//券商席位
			set< int > nFundTradeVolumeInfoSet;
			BOOL bFundTradeVolumeInfo;
			//-----------------wangzhy-------------------------
			set< int > nFundBondPortfolio;			//债券组合
			set< int > nFundBondPortfolioDateSet;	
			BOOL bFundBondPortfolio;

			set< int > nFundBondHoldDetail;			//持债明细
			set< int > nFundBondHoldDetailDateSet;
			BOOL bFundBondHoldDetail;	

			set< int > nFundCBondHoldDetail;		//持可转债明细
			set< int > nFundCBondHoldDetailDateSet;
			BOOL bFundCBondHoldDetail;	

			set< int > nFundOtherAsset;				//其他资产
			set< int > nFundOtherAssetDateSet;
			BOOL bnFundOtherAsset;	

			set< int > nFlowOfCapitalSet;			//资金流向
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

			//-----基金净值增长率修改--------
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

			/*目前VBA五档买卖盘函数提取存在缺陷，第一次取不到正确的值，必须请求第二次才能得到有效值
			而现在VBA请求缓存Map方式就只能请求一次，直到清理缓存才能再次请求
			Mantis12390的处理方法是：如果得到的是无效值，则继续请求，仅限于五档买卖盘、买卖量函数
			Mantis12390的处理方法存在缺陷：如果值就是无效值时会一直请求
			针对上述情况，暂时加一条件，如果已请求了多次则将该值存入缓存，也就是不让其再次请求
			*/
			//2012-10-23
			std::unordered_map<CString,int> mapRequest_limit;

			//VBA请求缓存Map
			ReqMap mapRequest;			
			//VBA请求队列，分A、B两个队列，一个用于缓存VBA请求，一个用于指标计算
			StrSet setReqA;				//VBA请求队列A
			StrSet setReqB;				//VBA请求队列B
			BOOL bsetAIsBusy;			//TRUE：setA忙碌，用于指标计算，FALSE：setA空闲，用于缓存VBA请求
			CCriticalSection _critsect;	//临界区，用于线程互斥

			//实时指标的VBA请求缓存Map
			ReqMap mapReqRealTime;

			//计算或者缓存实时指标请求的临界区，用于线程互斥
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

			//<Name，Code>
			StrMMap mmapNameCode;

			//<交易实体Code, 交易实体ID>
			STRLONG_Map mapCodeID;
			//2012-12-25
			//基金<交易实体Code, 交易实体ID>
			STRLONG_Map mapFundCodeID;
			//债券<交易实体Code, 交易实体ID>
			STRLONG_Map mapBondCodeID;

			//行业分布-证监会(新)  2013-04-22
			//--30377/以财年、财季为文件名则
			//<报告期,<"交易实体ID+行业ID"，行业市值>>
			//--30378/以交易实体ID为文件名
			//<交易实体ID,<"报告期+行业ID"，行业市值>>
			//map<int,map<CString,double>> mapFundIndustryDistribute;

			map<int,map<int,map<int,double>>> mapFundIndustryDistribute;

			//CWnd *pExcelWnd;

			//0：十大股东；1：十大流通股东；2：封闭式基金的十大持有人；3：可转债的十大持有人
			Tx::Core::Table_Indicator tableShareHolder[4];

			//0：股票发行(首发/增发)；1：权证创设注销；2：可转债转股价格；3：可转债利率；4：可转债转换数量；5：债券发行信息
			Tx::Core::Table_Indicator tableIndDate[6];

			//0：基金净值调整
			Tx::Core::Table_Indicator tableIndDates[1];

			//0：基金资产净值；1：可转债基本信息；2：主要财务指标；3：资产负债；
			//4：利润分配；5：现金流量；6：股本结构；7：持股人数；8：主营业务收入分布；
			//9：资产减值准备；10：非经常性损益；11：应收帐款；12：财务费用
			Tx::Core::Table_Indicator tableIndID[13];

			//-----------------zhangxs--20080821---------
			Tx::Core::Table_Indicator m_tableFundManager;
			Tx::Core::Table_Indicator m_tableBondTmp;
			//--------------------------------------------
			//------------------wangzhy-----------------
			Tx::Core::Table_Indicator m_tableQS;
			//------------------20080829----------------
			//上证指数
			INT iSHIndexID;// = 4000208;
			//深圳成指
			INT iSZIndexID;// = 4000001;
		public:
			//------wangzhy----------
			ExcelCfg m_Cfg;
			//-------20090403--------
		public:
			void InitTXEEFunctions();
			void InitTables(Tx::Business::TxBusiness *pBusiness, bool bDownAll = true );
			void ClearTables();

			//下载数据过程，作为线程内部函数调用
			UINT LoadData_Proc(LPVOID lpParam);
			//清空请求Map
			UINT ClearReqMap_Proc(LPVOID lpParam);
			//历史指标计算过程
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
			double  Get_PE_Static( int iSecurity, int iDate, bool bSecurity = true );    //静态PE add by wangzf   2012-3-23
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
			//获取行业分布市值(证监会(新))
			double  GetFundCombineIndustryDistribute(int nSecurityId,int nReport,int nIndustryId);
			//added by zhangxs 20090304
			CString Text2Date(CString m_str);
			CString Date2Text(CString m_str);
			//-------------------------------------------
			//交易实体，开始日期，截止日期，周期，返回格式
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
			///  根据指数代码返回指数的内码。
			/// </summary>
			LONG GetIndexSecurityID(LPCTSTR lpszSecurity);

			LONG GetSecurityIDEx(LPCTSTR lpszSecurity, int nType = 0 );   //获取债券交易实体ID  2012-12-25
			LONG GetFund_SecurityID(LPCTSTR lpszSecurity, int nType = 0 );//获取基金交易实体ID  2012-12-25

			SHORT GetItemID(SHORT iFuncID, LPCTSTR lpszItem);
			LONG GetIndicatorID2IndustryID(SHORT iFuncID, LPCTSTR lpszItem);  //通过指标ID获取行业ID
			SHORT GetReportType(LPCTSTR lpszReportType);
			SHORT GetAccountingPolicyType(LPCTSTR lpszAccountingPolicyType);
			SHORT GetIRType(LPCTSTR lpszIRType);
			SHORT GetPriceType(LPCTSTR lpszPriceType);
			SHORT GetValueType(LPCTSTR lpszValueType);
			SHORT GetPFADSubject(LPCTSTR lpszPFADSubject);

			SHORT Get_Fund_ReportType(LPCTSTR lpszReportType);  //获得财年财季信息 by zhangxs 20080808

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
			//同步计算历史指标
			CString GetIndicatorSyn(LPCTSTR lpszRequest,Tx::Core::VariantData& vdReq = Tx::Core::VariantData(_T("")));
			CString HisIndCalcProcSyn(LPCTSTR lpszRequest,Tx::Core::VariantData& vdReq = Tx::Core::VariantData(_T("")));


			//这里为了优化同步函数增加的成员变量
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
			//--强制重新计算，清掉已有的计算结果--20080801--
			UINT ClearResult_Proc(void);
			//------基金净值---------------
			Tx::Core::Table			m_dFundNvrTable;	//单位净值
			std::unordered_map< int,int >		m_FundNvrRowMap;	//行索引
			std::unordered_map< int,int >		m_FundNvrColMap;	//列索引

			Tx::Core::Table			m_dFundAcuNvrTable;	//累计净值
			std::unordered_map< int,int >		m_dFundAcuNvrRowMap;//行索引
			std::unordered_map< int,int >		m_dFundAcuNvrColMap;//列索引
			//-------20090306----------------
			//-------货币基金净值--------------------
			Tx::Core::Table			m_dMmfFundNvrTable;	//单位净值
			std::unordered_map< int,int >		m_MmfFundNvrRowMap;	//行索引
			std::unordered_map< int,int >		m_MmfFundNvrColMap;	//列索引

			Tx::Core::Table			m_dMmfFundAcuNvrTable;	//累计净值
			std::unordered_map< int,int >		m_dMmfFundAcuNvrRowMap;//行索引
			std::unordered_map< int,int >		m_dMmfFundAcuNvrColMap;//列索引
			//----------20090504------------------------------------

			//--------------获取基金单位净值-----------------
			bool	GetFundNvr( std::set< int > nSecurity,std::set< int > nDates, Tx::Core::Table* pResTable = NULL );
			double	GetFundNvrVal( int iSecurity,int iDate );
			//--------------获取基金累计净值-----------------
			bool	GetFundAcuNvr( std::set< int > nSecurity,std::set< int > nDates, Tx::Core::Table* pResTable = NULL );
			double	GetFundAcuNvrVal( int iSecurity,int iDate );
			//--------------获取货币基金万份收益-------------
			bool	GetMmfFundNvr( std::set< int > nSecurity,std::set< int > nDates, Tx::Core::Table* pResTable = NULL );
			double	GetMmfFundNvrVal( int iSecurity,int iDate );
			//--------------获取基金7日年化------------------
			bool	GetMmfFundAcuNvr( std::set< int > nSecurity,std::set< int > nDates, Tx::Core::Table* pResTable = NULL );
			double	GetMmfFundAcuNvrVal( int iSecurity,int iDate );


			//------基金净值---------------
			Tx::Core::Table			m_dFundNvrExtTable;	//单位净值
			std::unordered_map< int,int >		m_FundNvrRowExtMap;	//行索引
			std::unordered_map< int,int >		m_FundNvrColExtMap;	//列索引

			Tx::Core::Table			m_dFundAcuNvrExtTable;	//累计净值
			std::unordered_map< int,int >		m_dFundAcuNvrRowExtMap;//行索引
			std::unordered_map< int,int >		m_dFundAcuNvrColExtMap;//列索引
			//-------20090306----------------
			//-------货币基金净值--------------------
			Tx::Core::Table			m_dMmfFundNvrExtTable;	//单位净值
			std::unordered_map< int,int >		m_MmfFundNvrRowExtMap;	//行索引
			std::unordered_map< int,int >		m_MmfFundNvrColExtMap;	//列索引

			Tx::Core::Table			m_dMmfFundAcuNvrExtTable;	//累计净值
			std::unordered_map< int,int >		m_dMmfFundAcuNvrRowExtMap;//行索引
			std::unordered_map< int,int >		m_dMmfFundAcuNvrColExtMap;//列索引
			//----------20090504------------------------------------
			//--------------获取基金单位净值ext-----------------
			bool	GetFundNvrExt( std::set< int > nSecurity,std::set< int > nDates, Tx::Core::Table* pResTable = NULL );
			double	GetFundNvrExtVal( int iSecurity,int iDate );
			//--------------获取基金累计净值ext-----------------
			bool	GetFundAcuNvrExt( std::set< int > nSecurity,std::set< int > nDates, Tx::Core::Table* pResTable = NULL );
			double	GetFundAcuNvrExtVal( int iSecurity,int iDate );
			//--------------获取货币基金万份收益ext-------------
			bool	GetMmfFundNvrExt( std::set< int > nSecurity,std::set< int > nDates, Tx::Core::Table* pResTable = NULL );
			double	GetMmfFundNvrExtVal( int iSecurity,int iDate );
			//--------------获取基金7日年化ext------------------
			bool	GetMmfFundAcuNvrExt( std::set< int > nSecurity,std::set< int > nDates, Tx::Core::Table* pResTable = NULL );
			double	GetMmfFundAcuNvrExtVal( int iSecurity,int iDate );

			//修改同步异步方式
			bool	SetSynStyle( bool bSynFalg );
			bool	GetSynStyle( void );
			//数据提取新方式
			CString GetLatestReport( CString strRequest );
			CString	DownLoad( CString strReq,CString strAddress );
			//获取成交明细数据
			//获取明细数据条数
			int		GetTradeDetailCount( int iSecurity,int iDate );
			Tx::Data::TradeDetailData* GetTradeDetailData( int iSecurity,int iDate,int iSeq ); 
			CString Test( CString strReq );
			//读取配置
			bool	GetExcelConfig(void);
			//保存配置
			bool	SavaExcelConfig(void);
			//文件名称,返回的数据列,搜索定位列,搜索定位条件
			CString	GetXmlData( CString strFineName,CString strCol,CString strKey= _T(""),CString strCol1 = _T("") );
			std::set<int> m_setSWIndex;

			std::unordered_map<int,CString> FundNameMap;	//证监会规范基金简称map

			//债券YTM[实时]
			std::unordered_map<int,BondYTM> m_AllYtmMap;
			void GetBondYTM(void); 
			void GetBondYTM(int nSecurityId,double &dytm,double &dDur,double &dMDur,double &dCon,bool bCur=true);

			//最新五挡买卖盘到期收益率
			std::unordered_map<int,double> m_bpytm[5];
			std::unordered_map<int,double> m_spytm[5];

			void GetMmpBondYtm(int no,bool mmp=true);  //no：盘号1,2,3,4,5；mmp:true-买盘，flase-买盘；
			double GetMmpBondYtm(int iSecurityId,int no,bool mmp=true );   //no：盘号1,2,3,4,5；mmp:true-买盘，flase-买盘；
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
				//tmp.Remove(_T(' '));//半角

				char out[255];
				memset(out,0,255);
				::LCMapString(LANG_SYSTEM_DEFAULT,LCMAP_FULLWIDTH,tmp,255,out,255);
				CString ret(out);
				ret.Remove(_T('　'));//全角
				return ret;
			}
			CString m_name;
		};
	}
}