/**************************************************************
Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
File name:		Fund.h
Author:			赵宏俊
Date:			2007-07-09
Version:		1.0
Description:	
基金业务功能类

2007-09-17
由于基金和股票的板块排序分析功能类似
只要不处理开放式基金就可以了；
所以基金板块排序分析和股票板块排序分析共用TxBusiness::GetBlockAnalysis

板块定价特征除开放式基金外，算法与股票相同使用TxBusiness::GetBlockPricedFuture

热点分析和分时同列已经在底层提供数据接口，在UI绘制图形和提取数据时直接调用

板块风险分析由于指标ID不同，所以在TxFund里处理基金的风险分析

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

		// 获取基金总份额
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
		// key 为日期
		std::unordered_map<int,std::shared_ptr<fund_total_share_db_set>> _M_totalShares;
	};

	namespace Business
	{
		//guanyd
		//基金风格
		//基金风格的内容数据
		struct	FundStyle
		{
			//基金风格基础	
			//0x0001股票型	0x0002混合型	0x0004债券型	0x0008货币型	0x0010保本型
			WORD	wBasicFundStyle;

			//股票型详细
			//0x0001成长型	0x0002平衡型	0x0004价值型	0x0008指数型	0x0010行业型	0x0020区域型	0x0040其他型
			WORD	wShareDetail;
			//混合型详细
			//0x0001灵活组合型	0x0002配置型	0x0004其他型
			WORD	wMixDetail;
			//债券型详细
			//0x0001纯债型	0x0002偏债型
			WORD	wBondDetail;
			//货币型详细
			WORD	wCurrencyDetail;
			//保本型详细
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
		//基金业务功能类
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
			//根据自然日期取得基金报告期日期
			int GetFundReportDate(int date,bool bQuarter,int iMarketId=0);
			int GetFundReportDateMD(int date,bool bHis,bool bQuarter);

			//2008-09-11
			//重仓债券=明细=前5名为重仓
			bool GetMyBonds(std::vector<int> iSecurityId,Table_Display& baTable,int iYear,int iQuarte,int no=5);

			//2008-08-27
			//折溢价率
			double	GetOverflowValueRate(
				int     iSecurityId,
				int		iDate
				);

			//排序分析数据结果标题列
			bool GetBlockAnalysisCol(Table_Display& baTable,std::vector<int>& arrSamples,int& iSortCol);
			//排序分析数据结果设置
			bool SetBlockAnalysisCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow);

			//2007-09-17
			//zhaohj
			//板块风险分析[高级]
			bool BlockRiskIndicatorAdvFund(
				int iMenuID,						//功能ID
				Tx::Core::Table_Indicator& resTable,//结果数据表
				std::set<int>& lSecurityId,			//交易实体ID
				int iEndDate,						//截止日期
				long lRefoSecurityId,				//基准交易实体ID
				int iStartDate=0,					//起始日期
				int iTradeDaysCount=20,				//交易天数
				int iDuration=0,					//交易周期0=日；1=周；2=月；3=季；4=年
				bool bLogRatioFlag=false,			//计算比率的方式,true=指数比率,false=简单比率
				bool bClosePrice=true,				//使用收盘价
				int  iStdType=0,					//标准差类型1=有偏，否则无偏
				int  iOffset=0,						//偏移
				bool bAhead=true,					//复权标志,true=前复权,false=后复权
				bool bUserRate=true,				//用户自定义年利率,否则取一年定期基准利率
				double dUserRate=0.034,				//用户自定义年利率,否则取一年定期基准利率
				bool bDayRate=true,					//日利率
				bool bPrw = false,					//是否添加进度条
				bool bMean = false                  //是否计算收益均值
				);
			//2007-10-15
			//
			/*bool BlockCycleRate(
			std::set<int>& iSecurityId,			//交易实体ID
			int date,							//数据日期
			Tx::Core::Table_Indicator& resTable	//结果数据表
			);
			*/
			//2007-10-15
			//货币基金期限配置
			bool StatCurrencyFundSurplusTerm(
				//		int iMenuID,						//功能ID
				Tx::Core::Table_Indicator& resTable,//结果数据表
				std::vector<int>& iSecurityId,		//交易实体ID
				std::vector<int>  iDate				//报告期
				);
			//重仓股集中度统计
			bool StatFundZCGjzd(
				int iMenuID,						//功能ID
				Tx::Core::Table_Indicator& resTable,//结果数据表
				std::vector<int>& iSecurityId,		//交易实体ID
				std::vector<int> vDate				//报告期
				);
			//重仓股季度比较
			bool StatFundZCGjdbj(
				int iMenuID,						//功能ID
				Tx::Core::Table_Indicator& resTable,//结果数据表
				std::vector<int>& iSecurityId,		//交易实体ID
				std::vector<int> vDate				//报告期
				);

			//modified by zhangxs
			bool TxFund::StatFundStockHolding(
				Tx::Core::Table_Indicator& sourceTable,	//为双击后保存数据。
				Tx::Core::Table_Indicator& resTableStock,	//股票结果数据表
				Tx::Core::Table_Indicator& resTableFund,	//基金结果数据表
				std::vector<int>& iFundSecurityId,		//基金交易实体ID
				std::vector<int>& iStockSecurityId,		//股票交易实体ID
				std::vector<int> vDate,				//报告期
				bool bIsZCG,							//数据源为重仓股；否则为持股明细
				//std::set<int> & TradeId,			//保存所取得sourceTable里的数据的全部的交易实体ID
				std::vector<CString> &vColName,
				std::vector<CString> &vColHeaderName,
				int& m_iFirstSampleId
				);
			//用于输出所有明细
			//modified by zhangxs
			bool TxFund::StatFundStockHolding(
				Tx::Core::Table_Indicator& sourceTable,	//为双击后保存数据。
				Tx::Core::Table_Indicator& resTableStock,	//股票结果数据表
				Tx::Core::Table_Indicator& resTableFund,	//基金结果数据表
				std::vector<int>& iFundSecurityId,		//基金交易实体ID
				std::vector<int>& iStockSecurityId,		//股票交易实体ID
				std::vector<int> vDate,				//报告期
				bool bIsZCG,							//数据源为重仓股；否则为持股明细
				//std::set<int> & TradeId,			//保存所取得sourceTable里的数据的全部的交易实体ID
				std::vector<CString> &vColName,
				std::vector<CString> &vColHeaderName
				);
			//股票持有状况统计--数据源为明细
			bool GetHeldStockHeldDetail(
				Tx::Core::Table_Indicator &resTable,
				std::vector<int> TradeId,
				std::vector<int> ViFundSecurityId,
				std::vector<int> iDate
				);
			//added by zhangxs
			//股票持有状况统计--数据源为重仓股

			bool TxFund::GetHeldStockFromTopTen(
				Tx::Core::Table_Indicator &resTable,
				std::vector<int> TradeId,
				std::vector<int> ViFundSecurityId,
				std::vector<int>   iDate);
			//专门为股票持有情况增加两个函数，

			//added by zhangxs 20081120
			//股票持有状况统计--数据源为明细
			bool GetHeldStockHeldDetail(
				Tx::Core::Table_Indicator &resTable,
				int	iSecurityId,
				std::vector<int> ViFundSecurityId,
				std::vector<int> iDate
				);
			//added by zhangxs
			//股票持有状况统计--数据源为重仓股

			bool TxFund::GetHeldStockFromTopTen(
				Tx::Core::Table_Indicator &resTable,
				int	iSecurityId,
				std::vector<int> ViFundSecurityId,
				std::vector<int>   iDate);

			//数据源为重仓股
			bool StockHoldingTopTen(
				Tx::Core::Table_Indicator &resTable,
				std::vector<int>	iSecurityId,
				std::vector<int>	iDate
				);

			//数据源为持股明细
			bool StockHoldingList(
				Tx::Core::Table_Indicator &resTable,
				std::vector<int>	iSecurityId,
				std::vector<int>	iDate
				);	
			//某股票被基金持有的明细
			bool StatFundStockDetail(
				Tx::Core::Table_Indicator& resTable,	//结果数据表
				Tx::Core::Table_Indicator& resTableMx,	//结果数据表明细
				int iSecurityStockId	//股票交易实体ID
				);
			//基金申购赎回费率表
			bool StatFundFeeRate(
				Tx::Core::Table_Indicator& resTable,	//结果数据表
				std::vector<int>& iFundSecurityId,		//基金交易实体ID
				int iStartDate,							//起始日期
				int iEndDate,							//终止日期
				int FeeType,							//费率模式
				bool IsAppoint = true					//是否是最新日期
				);
		private:
			//2007-10-16
			//bFullData = false =>精确统计=统计结果的记录数和样本数量保持一致[一对一]
			//bFullData = true =>统计=统计结果的记录数和样本数量不一致[一对多]
			bool StatCommon(
				int iMenuID,							//功能ID
				Tx::Core::Table_Indicator& resTable,	//结果数据表
				std::vector<int>& iSecurityId,		//交易实体ID
				std::vector<Tx::Core::VariantData> arrParam,	//参数
				bool bFullData=false					//统计所有
				);



		public:
			//guanyd 
			//专项统计
			//2007-09-17

			//阶段行情
			//交易行情
			bool	StatCycleRate(
				Tx::Core::Table_Indicator	&resTable,
				std::set<int>	&iSecurityId,
				int		iStartDate,
				int		iEndDate,
				bool bCutFirstDateRaise,			//剔除首日涨幅
				int	iFQLX							//复权类型
				);

			//市场情况
			//基金份额
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

			//基金分红
			bool	StatFundDividend(
				Tx::Core::Table_Indicator	&resTable,
				std::vector<int>	&iSecurityId,
				int		iStartDate,
				int		iEndDate,
				bool	bGetAllDate
				);

			//基金发行
			//wIssueType是发行类型：	0x0001首发、0x0002扩募
			bool	StatFundIssue(
				Tx::Core::Table_Indicator	&resTable,
				std::vector<int>	&iSecurityId,
				int		iStartDate,
				int		iEndDate,
				bool	bGetAllDate,
				WORD	wIssueType,
				bool    FXRQ = true
				);
			//封闭式发行
			bool	StatFundIssueClose(
				Tx::Core::Table_Indicator	&resTable,
				std::vector<int>	&iSecurityId,
				int		iStartDate,
				int		iEndDate,
				bool	bGetAllDate,
				WORD	wIssueType,	
				bool    FXRQ 
				);
			//开放式发行
			bool	StatFundIssueOpen(
				Tx::Core::Table_Indicator	&resTable,
				std::vector<int>	&iSecurityId,
				int		iStartDate,
				int		iEndDate,
				bool	bGetAllDate,
				bool    FXRQ 
				);
			//开放式发行==新库
			bool	StatFundIssueOpen_New(
				Tx::Core::Table_Indicator	&resTable,
				std::vector<int>	&iSecurityId,
				int		iStartDate,
				int		iEndDate,
				bool	bGetAllDate,
				bool    FXRQ 
				);
			//基金公司概况
			bool	StatFundCompanySituation(
				Tx::Core::Table_Indicator	&resTable,
				std::vector<int>	&iSecurityId,
				int		iStartDate,
				int		iEndDate,
				bool	bGetAllDate	
				);

			//基金公司股东
			bool	StatFundHolder(
				Tx::Core::Table_Indicator	&resTable,
				std::vector<int>	&iSecurityId,
				int		iStartDate,
				int		iEndDate,
				bool	bGetAllDate		
				);

			//基金经理
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
			//业绩分析
			//基金净值输出
			bool	StatFundNetValueOutput(
				Tx::Core::Table_Indicator	&resTable,
				std::vector<int>	&iSecurityId,
				int		iStartDate,
				int		iEndDate,
				int		iTimeCycle,		//时间周期。日周月季年
				int		iStatIndicator,	//基金单位净值，基金累计净值
				std::vector<CString> &vDates,
				std::vector<CString> &vColName
				);
			//基金净值 -- 多券多日期--日期点，不是区间
			bool	StatFundNetValueOutput(Tx::Core::Table_Indicator &resTable,std::vector<int>	&iSecurityId,std::vector<int> iDates);

			bool	GetMmfFundNvr( std::vector< int > iSecurity,std::vector< int > iDates, Tx::Core::Table_Indicator* pResTable );
			//--------------获取货币基金基金七日年化-----------------
			bool	GetMmfFundAcuNvr( std::vector< int > iSecurity,std::vector< int > iDates, Tx::Core::Table_Indicator* pResTable );

			std::unordered_map< int,int >		m_MmfFundNvrColMap;	//列索引

			//货币基金收益
			bool	StatFundCurrencyIncome(
				Tx::Core::Table_Indicator	&resTable,
				std::vector<int>	&iSecurityId,
				int		iStartDate,
				int		iEndDate,
				int		iTimeCycle,			//时间周期:日周月季年区间
				int		iStatIndicator,		//统计指标:万份基金单位收益、最近七年化收益率
				bool    IsOut,				//是直接输出还是累计计算。
				std::vector<CString> &vDates,
				std::vector<CString> &vColName
				);
			//add by lijw 2008-05-16
			//是专门为了计算货币基金收益里的累计计算。
			bool AddUpCalculator(
				Tx::Core::Table_Indicator &resTable,   //存放结果的表
				Tx::Core::Table_Indicator &m_txTable,  //传入要处理的数据的源表
				std::vector<int>	&iSecurityId,	  //样本的交易实体ID
				int		iStartDate,					  //起始日期
				int		iEndDate,					 //截止日期
				int		iStatIndicator,		        //统计指标:万份基金单位收益、最近七年化收益率
				int		iTimeCycle,			        //时间周期:日周月季年区间
				bool	IsOut,				//是直接输出还是累计计算
				std::vector<CString> &vDates,
				std::vector<CString> &vColName
				);
			void CalculatorEndDate(int iTimeCycle,int& dateE,int & dateB);
			//折溢价率
			bool	StatOverflowValueRate(
				Tx::Core::Table_Indicator	&resTable,
				std::vector<int>	&iSecurityId,
				int		iStartDate,
				int		iEndDate,
				int		iTimeCycle,		//统计周期
				int		iCustomCycle,	//自定义周期
				std::vector<CString> &vColName
				);
			//净值增长率
			bool	StatFundNetValueRiseRate(
				Tx::Core::Table_Indicator	&resTable,
				std::vector<int>	&iSecurityId,
				int		iStartDate,
				int		iEndDate,
				int		iTimeCycle,		//统计周期
				int		iCustomCycle,	//自定义周期
				std::vector<CString> &vColName
				);
			//净值分析
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
			//投资组合分析
			//持仓结构统计
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
			//资产组合统计 t_asset_allocation
			bool StatAssetAllocation(
				Tx::Core::Table_Indicator &resTable,
				std::vector<int>	iSecurityId,
				std::vector<int>	iDate,
				int		iType
				);

			//行业分布统计
			//		T_STOCK_HOLDING_INDUSTRY_DISTRIBUTION_ACTIVE	积极投资部分行业分布表
			//		T_STOCK_HOLDING_INDUSTRY_DISTRIBUTION_INACTIVE	指数化投资部分行业分布表
			//		T_STOCK_HOLDING_INDUSTRY_DISTRIBUTION	合并投资部分行业分布表
			bool StatIndustryDistribution(
				Tx::Core::Table_Indicator &resTable,
				std::vector<int>	iSecurityId,
				std::vector<int>	iDate,
				int		iType
				);
			//债券组合结构统计T_BOND_HOLDING_BOND_TYPE_DISTRIBUTION
			bool StatBondDistribution(
				Tx::Core::Table_Indicator &resTable,
				std::vector<int>	iSecurityId,
				std::vector<int>	iDate,
				int		iType
				);
			//天相行业分布统计
			bool StatIndustryDistributionTx(
				Tx::Core::Table_Indicator &resTable,
				std::vector<int>	iSecurityId,
				std::vector<int>    iStartData,
				std::vector<int>	iDate,
				int		iType
				);
			//持股统计
			//股票持有状况统计


			//重仓股统计
			bool StatStockHoldingTopTen(
				Tx::Core::Table_Indicator &resTable,
				std::vector<int>	iSecurityId,
				std::vector<int>	iDate,
				int		iType
				);

			//重仓股集中度统计

			//重仓股季度比较

			//持股明细统计
			bool StatStockHolding(
				Tx::Core::Table_Indicator &resTable,
				std::vector<int>	iSecurityId,
				std::vector<int>	iDate,
				int		iType,
				int     &iTempCount
				);	
			//买入卖出股票
			bool StatBuyInSellOutStock(		
				Tx::Core::Table_Indicator &resTable,
				std::vector<int>	iSecurityId,
				std::vector<int>	iDate,
				int		iType
				);


			//持债统计
			//重仓持债统计T_BOND_HOLDING_TOP_FIVE_BOND
			bool StatBondHoldingTopFive(
				Tx::Core::Table_Indicator &resTable,
				std::vector<int>	iSecurityId,
				std::vector<int>	iDate,
				int		iType
				);

			//处于转股期的可转债T_BOND_HOLDING_TOP_FIVE_CONVERTIBLE
			bool StatConvertibleBond(
				Tx::Core::Table_Indicator &resTable,
				std::vector<int>	iSecurityId,
				std::vector<int>	iDate,
				int		iType
				);
			//货币基金持有债券
			bool StatCurrencyFundHoldingBond(
				Tx::Core::Table_Indicator &resTable,
				std::vector<int> &iSecurityId,
				std::vector<int> &iDate,
				int iType
				);
			//add by lijw 2008-09-08
			//取得封闭式基金净值增长率
			bool GetCloseFundNavRaise(
				std::vector<int>& iSecurityId,		//交易实体ID
				//int date,							//数据日期,0表示最近交易日
				Tx::Core::Table_Display& resTable,	//结果数据表
				int& iDate,
				bool bPrice = false				//现价标志
				);
			//基金净值涨幅
			bool GetFundNavRaise(
				std::vector<int>& iSecurityId,			//交易实体ID
				int date,							//数据日期
				Tx::Core::Table_Display& resTable,//结果数据表
				bool bPrice=false//现价标志
				);
			//基金的阶段行情   add by lijw 2008-09-11
			bool FundPhaseMarket(std::vector<int> &samples,Tx::Core::Table_Indicator& resTable);
			//基金的持有人结构   add by lijw 2008-09-16
			bool HoldPersonStruct(std::vector<int> &iSecurityId,Tx::Core::Table_Indicator& resTable,std::vector<int> &vDate);
			//2008-09-08 add by wangych
			//取得开放式基金净值涨幅
			bool GetOpenFundNavRaise(
				std::vector<int>& iSecurityId,		//交易实体ID
				//int date,							//数据日期
				Tx::Core::Table_Display& resTable,	//结果数据表
				int& iDate,
				bool bPrice=false					//现价标志
				);


			//2008-08-07
			//取得基金净值2days
			bool GetFundNav2Days(
				std::vector<int>& iSecurityId,		//交易实体ID
				Tx::Core::Table_Display& resTable,	//结果数据表
				GridAdapterTable_Display& resAdapter//显示格式
				);

			//2008-09-08 add by wangych
			//取得开放式基金净值2days
			bool GetOpenFundNav2Days(
				std::vector<int>& iSecurityId,		//交易实体ID
				Tx::Core::Table_Display& resTable,	//结果数据表
				GridAdapterTable_Display& resAdapter,//显示格式
				int& iDate1,						//交易日1
				int& iDate2							//交易日2
				);

			//add by lijw 2008-09-08
			//取得基金净值2days
			bool GetCloseFundNav2Days(
				std::vector<int>& iSecurityId,		//交易实体ID
				Tx::Core::Table_Display& resTable,	//结果数据表
				GridAdapterTable_Display& resAdapter,//显示格式
				int& iDate1,						//交易日1
				int& iDate2							//交易日2
				);

			bool GetFundCurrency2Days(
				std::vector<int>& iSecurityId,		//交易实体ID
				Tx::Core::Table_Display& resTable,	//结果数据表
				GridAdapterTable_Display& resAdapter,//显示格式
				int& iDate1,						//交易日1
				int& iDate2							//交易日2
				);

			//2008-09-11 add by wangych
			//取得封闭式基金行情报价
			bool GetCloseFundQuotePrice(
				std::vector<int>& iSecurityId,		//交易实体ID
				int date,							//数据日期
				Tx::Core::Table_Display& resTable,	//结果数据表
				GridAdapterTable_Display& resAdapter//显示格式
				);
			//取得行业配置 add by wangych 08.09.12
			bool TxFund::GetIndustryDistribute(
				std::vector<int>& iSecurityId,		//交易实体ID
				int date1,							//数据日期,所选报告期
				int date2,							//数据日前，所选上个报告期
				Tx::Core::Table_Display& resTable,	//结果数据表
				GridAdapterTable_Display& resAdapter//显示格式
				);


			//同业分析
			//总体投资情况

			//特色股票

			//特色行业

			//重仓股票特征

			//交易佣金
			bool StatTradeCommission(
				Tx::Core::Table_Indicator &resTable,
				std::vector<int>	iSecurityId,
				std::vector<int>	iDate,
				int		iType
				);
			//guanyd
			//一些辅助
		private:
			UINT varCfg[9];
			FundStyle	m_FundStype;			

			IndicatorWithParameter m_IWP;
			Tx::Business::IndicatorWithParameterArray m_IWPA;

			//临时表，目前用于在分类统计作为模块间伪全局变量使用
			Tx::Core::Table_Indicator tmpTable;

			CString strTable;//用于数据输出
			bool TransObjectToSecIns(
				std::vector<int>	sObjectId,		//交易实体id
				std::vector<int> &sSecInsId,		//券id或机构id
				int iType						//id类型：1:券id	2:机构id
				);

			bool IdColToNameAndCode(
				Tx::Core::Table_Indicator &resTable,		//结果表
				int iCol,									//制定列
				int iIdType,								//数据类型
				int iMethodType=0
				);
			bool StatisticOnType(Tx::Core::Table_Indicator &resTable,int iClassifyType);

			bool StatisticsOnType(Tx::Core::Table_Indicator &resTable,int iClassifyTypes);

			//报告期转报告日期
			// 起始终止日期与报告期的转换 by 20101111 wanglm
			int RegportDateToReportID( std::vector<int> vecStartDate, std::vector<int> vecDate );  // by 20101112 wanglm

			//财年+报告期-〉报告日期，减少一列
			bool TransReportDateToNormal(Tx::Core::Table_Indicator &tmpTable,int iCol);
			// add by lijw 2008-02-20,这个接口公用的接口。是区别于上面的那一个专用的。tempTable即接受数据，也把处理过的数据保存在tempTable里
			bool TransReportDateToNormal2(Tx::Core::Table_Indicator &tempTable,int iCol);
			//报告日期->CString类型，tempTable即接受数据，也把处理过的数据保存在tempTable里，
			bool TransReportDateToNormal3(Tx::Core::Table_Indicator &tempTable,int iCol);
			//这两个方法看上去是表的基础操作
			//但是在数据处理上实际是有一定的针对性，所以不放在table的基础类里面而是做成Fund下的一个操作
			//CopyRow复制一行数据到另一张表的另一行，实现要保证列数以及列类型完全相同
			//PlusRow则是对数字类型数据（int ,double,float）进行累加，其他类型则不管
			bool CopyRow(Table_Indicator &resTable,Table_Indicator &sourceTable,int iResRow,int iSourceRow);
			bool PlusRow(Table_Indicator &resTable,Table_Indicator &sourceTable,int iResRow,int iSourceRow);
			//add by lijw 2009-2-13
			bool EliminateSample(std::vector<int> &TradeVS,std::vector<int> &TradeVR,std::vector<int> &FundV);
			bool EliminateSample(std::vector<int> &TradeVS,std::vector<int> &TradeVR,std::set<int> &FundSet);

			//
			bool FillColumn(
				Tx::Core::Table_Indicator &m_transTable,		//关联表
				UINT transIndexColumn,							//关联表作为map索引的列
				UINT addColumn,									//关联表中数据的列，从这里取出数据放到insertColumn列
				Tx::Core::Table_Indicator &m_resTableAddColumn,	//源表
				UINT resIndexColumn,								//源表作为map索引的列
				UINT	insertColumn								//要插入数据的列		
				);

			bool TransReportIdToFundId(Tx::Core::Table_Indicator &resTable,int iCol=0);


			bool GetCycleDates(int iStartDate,int iEndDate,int iTimeCycle,std::vector<int> &iDates,int iCustomCycle);

			bool InsertColOfNAV(Tx::Core::Table_Indicator &resTable,int iInsertCol,int iIdCol,int iDateCol);
			bool ChangeDateColToReportCol(Tx::Core::Table_Indicator &resTable,int iCol);

			bool GetIndexDat(int file_id, std::unordered_map<int,CString>& indexdat_map);
			//add by lijw 2008-03-25
			//备注：下面的函数是专门计算各行值得累加。为样本的分类统计作准备
			bool AddUpRow(Tx::Core::Table &resTable,		//存放结果表
				int iStyle,						//样本分类的方式
				std::vector<int> ColVector,		//根据那些列进行统计
				std::vector<int> IntCol,			//需要相加的整型列
				std::vector<int> DoubleCol,	//需要相加的double列
				std::vector<int> iDate,			//报告期
				int			  iCol				//报告期所在的列。
				)	;
			bool AddUpRow(Tx::Core::Table &resTable,		//存放结果表
				int iStyle,						//样本分类的方式
				std::vector<int> ColVector,		//根据那些列进行统计
				std::vector<int> IntCol,			//需要相加的整型列
				std::vector<int> DoubleCol,	//需要相加的double列
				std::vector<int> iDate,			//报告期
				int			   iCol,				//报告期所在的列。
				int			   iEquityCol,			//基金净值所在的列。
				int			   iFundIdCol,			//基金ID所在的列。
				int &			   iTradeId,		//股票或者债券的交易实体ID所在的列，并且它的后面必须是它的代码和名称。
				int			   sortCol,		//根据那一列进行排序。
				int			   pos,		//排名所在的列。
				bool             IsBuySell = false    //判断是否是买入卖出的分组统计 add 2008-05-21
				)	;
			bool AddUpRow2(Tx::Core::Table &resTable,		//存放结果表
				std::vector<int> ColVector,		//根据那些列进行统计
				std::vector<int> IntCol,			//需要相加的整型列
				std::vector<int> DoubleCol,	//需要相加的double列
				std::vector<int> iDate,			//报告期
				int			   iCol,				//报告期所在的列。
				int &		   iTradeId,		//股票或者债券的交易实体ID所在的列
				int			   sortCol,		//根据那一列进行排序。
				int			   pos,			//排名所在的列。
				bool            IsBuySell = false    //判断是否是买入卖出的分组统计  add 2008-05-21
				)	;
			bool AddUpRow3(Tx::Core::Table &resTable,		//存放结果表
				//			  int iStyle,						//样本分类的方式
				std::vector<int> ColVector,		//根据那些列进行统计
				std::vector<int> IntCol,			//需要相加的整型列
				std::vector<int> DoubleCol,	//需要相加的double列
				std::vector<int> iDate,			//报告期
				int			   iCol,				//报告期所在的列。
				int			   iEquityCol,				//基金净值所在的列。
				int			   iFundIdCol,			//基金ID所在的列。
				int 			   iTradeId,		//股票或者债券的交易实体ID所在的列
				int			   sortCol,		//根据那一列进行排序。
				int			   pos,			//排名所在的列。
				bool             IsBuySell = false    //判断是否是买入卖出的分组统计  add 2008-05-21
				)	;
			//下面是专门为交易佣金的分组统计写的函数2008-05-09
			bool AddUpRow(Tx::Core::Table &resTable,		//存放结果表
				int iStyle,						//样本分类的方式
				std::vector<int> ColVector,		//根据那些列进行统计
				std::vector<int> IntCol,			//需要相加的整型列
				std::vector<int> DoubleCol,	//需要相加的double列
				std::vector<int> iDate,			//报告期
				int			   iCol,				//报告期所在的列。
				int &			   iTradeId,		//券商ID所在的列，并且它的后面必须是它的名称。
				int			   sortCol,		//根据那一列进行排序。
				int			   pos			//排名所在的列。
				)	;
			bool AddUpRow4(Tx::Core::Table &resTable,		//存放结果表
				std::vector<int> ColVector,		//根据那些列进行统计
				std::vector<int> IntCol,			//需要相加的整型列
				std::vector<int> DoubleCol,	//需要相加的double列
				std::vector<int> iDate,			//报告期
				int			   iCol,				//报告期所在的列。
				int &		   iTradeId,		//券商ID所在的列
				int			   sortCol,		//根据那一列进行排序。
				int			   pos			//排名所在的列。
				);
			bool AddUpRow3(Tx::Core::Table &resTable,		//存放结果表
				std::vector<int> ColVector,		//根据那些列进行统计
				std::vector<int> IntCol,			//需要相加的整型列
				std::vector<int> DoubleCol,	//需要相加的double列
				std::vector<int> iDate,			//报告期
				int			   iCol,				//报告期所在的列。
				int 			   iTradeId,		//券商ID所在的列，并且它的后面必须是它的名称。
				int			   sortCol,		//根据那一列进行排序。
				int			   pos			//排名所在的列。
				);
			bool AddUpRowEX(Tx::Core::Table &resTable,		//存放结果表
				int iStyle,						//样本分类的方式
				std::vector<int> ColVector,		//根据哪些列进行统计  
				std::vector<int> IntCol,			//需要相加的整型列
				std::vector<int> DoubleCol,	//需要相加的double列
				std::vector<int> iDate,			//报告期
				int			   iCol,				//报告期所在的列。
				int &			   iTradeCol,			//券商ID所在的列，并且它的前面必须是它的名称。
				int			   sortCol,		//根据那一列进行排序。
				int			   pos			//排名所在的列。
				);	
			bool AddUpRowCompany(
				Tx::Core::Table &resTable,		//存放结果表
				std::vector<int> ColVector,		//根据哪些列进行统计  
				std::vector<int> IntCol,			//需要相加的整型列
				std::vector<int> DoubleCol,	//需要相加的double列
				std::vector<int> iDate,			//报告期
				int			  iCol,				//报告期所在的列。
				int 		      iTradeCol,		//券商ID所在的列，并且它的后面必须是它的名称。
				int			   sortCol,		//根据那一列进行排序。
				int			   pos			//排名所在的列。
				);	
			void AddUpAnalysis(Tx::Core::Table &resTable,		//存放结果结果表
				std::vector<int> ColVector,		//根据那些列进行统计
				bool	bCalculateValueRate,
				std::unordered_map<int,std::vector<int>> mapDate,
				Tx::Core::Table_Indicator & netValueTable
				);
			//获取基金==//30001232,	//基金风格New；30001020,	//管理公司ID；30001021	//托管银行ID
			//多处地方用到，所以提出来便于以后修改
			bool Get_Fund_Type_Manager_Company(Tx::Core::Table_Indicator &resTable,
				std::vector<int>	iSecurityId
				);
		public:
			//计算基金的样本行情 2009-5-19
			bool FundSample( Tx::Core::Table_Display* pResTable,long lSecurityId);
			bool GetValueOfStock(Tx::Core::Table_Indicator &hbTable,long lSecurityId);
			//通过起始截止日期获得区间报告期,如果区间没有报告期，就返回起始时间前的报告期，
			void GetReportDateArr(std::vector<int>& VecDate,int iStart,int iEnd);
			double GetAvgNetValueReportDateArr(std::vector<int> vecDate, int SecId);
			//从netValueTable取单位净值
			double GetAvgNetValueReportDateArr(std::vector<int> vecDate, int SecId,Tx::Core::Table_Indicator & netValueTable);

		};//end of class TxFund
	}
}
#endif // !defined(AFX_TXFUND_H__5BA14E0A_27BC_4FD4_B281_1AEFC5CFB1D8__INCLUDED_)
