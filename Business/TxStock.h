/**************************************************************
	Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
	File name:		TxStock.h
	Author:			赵宏俊
	Date:			2007-07-09
	Version:		1.0
	Description:	处理股票类业务
					某些处理方法可以直接使用TxBusiness提供的方法
***************************************************************/
#ifndef __TXSTOCK_H__
#define __TXSTOCK_H__
#include "TxBusiness.h"
//#include "..\..\Data\DataFileStruct.h"
//#include "..\..\Data\DataFile.h"
#include "TxBusiness.h"
#pragma once
namespace Tx
{
	namespace Business
	{
		const CString IndexDatFileName[19]=
		{
			_T("security_type_index.dat"),												//0
			_T("country_region_index.dat"),
			_T("currency_index.dat"),/*该表没有ID字段，用F_SEQ代替，使用时请注意*/
			_T("exchange_index.dat"),
			_T("institution_type_index.dat"),
			_T("accounting_firm_index.dat"),											//5
			_T("industry_index.dat"),/*使用f_industry_id字段*/
			_T("transaction_object_type_index.dat"),
			_T("event_type_index.dat"),
			_T("ipo_pricing_issue_type_index.dat"),

			//2007-08-22
			//上市公司业绩预告结果类型索引
			_T("profit_forecast_result_type_index.dat"),								//10

			_T("fiscal_year_quarter_index.dat"),
			_T("shareholder_type_index.dat"),/*f_type_id为nchar(1)类型  暂时用f_seq代替*/
			_T("tradable_shareholder_type_index.dat"),/*f_type_id字段为nchar(1) 暂时先用f_seq代替*/
			_T("share_change_cause_type_index.dat"),									
			_T("disclosure_type_index.dat"),											//15
			_T("tx_research_type_index.dat"),
			_T("info_file_type.dat"),
			_T("event_dividend_statistics_indicator_security.dat")						//18
		};
		//股票业务类
		//处理所有股票相关的业务
		class BUSINESS_EXT TxStock :
			public TxBusiness
		{
			DECLARE_DYNCREATE(TxStock)
		public:
			TxStock(void);
			virtual ~TxStock(void);
		//add by lijw 2008-08-21
		/*	个股评级统计（10333）
				个股评级数据（10334）
				个股评级汇总（10335）
				个股评级变化（10336）
				个股评级推荐效果（10337）

				盈利预测统计（10338）
				个股盈利预测数据（10339）
				盈利预测数据汇总（10340）
				盈利预测数据变化（10341）
				盈利预测数据误差统计（10342）

				特色统计（10343）
				机构最新预测数据（10344）*/
		private:
			DataFileNormal<blk_TxExFile_FileHead,GradeForecastOfYear>* m_pEvaluateYear;
			DataFileNormal<blk_TxExFile_FileHead,GradeForecastOfSecurity>* m_pEvaluateSecurity;
			DataFileNormal<blk_TxExFile_FileHead,ProfitForecastOfYear>* m_pForcastYear;
			DataFileNormal<blk_TxExFile_FileHead,ProfitForecastOfSecurity>* m_pForcastSecurity;
			GradeForecastOfYear * m_pEvaluateYearData;
			GradeForecastOfSecurity * m_pEvaluateSecurityData;
			GradeForecastOfSecurity * m_pEvaluateSecurityData2;
			ProfitForecastOfYear * m_pForcastYearData;
			ProfitForecastOfSecurity * m_pForcastSecurityData;
			bool LoadEvaluateYear(int id,CString strName);//id表示文件名ID，。
			bool LoadEvaluateSecurity(int id,CString strName);
			bool LoadForcastYear(int id,CString strName);
			bool LoadForcastSecurity(int id,CString strName);
			bool LoadFinaningSecurityData();	//取融资融券数据 add by wangyc 20100301
			void CombineDate(int iStartDate,int iEndDate,std::set<int>& DateSet);
			//存储融资融券数据		add by wangyc 20100301
			std::unordered_map<int,float> m_pIdToFinaning;	
			struct FINANCING
			{
				int iID;
				float fFinancingData;
			public:
				int GetMapObj(int index=0) { return iID; }
			};
		public:
			//个股评级数据（10334）
			bool TxStock::StatEvaluteData(
				Tx::Core::Table_Indicator &resTable,	//结果数据表
				std::vector<int> & iInstitutionId,		//机构样本
				std::vector<int> & iStockId,			//股票样本
				int iStartDate,							//起始日期
				int iEndDate,						//结束日期				
				bool bIsAllDate=false					//全部日期
				);
			//个股评级变化（10336）
			bool TxStock::StatEvaluteVariety(
				Tx::Core::Table_Indicator &resTable,	//结果数据表
				std::vector<int> & iInstitutionId,		//机构样本
				std::vector<int> & iStockId,			//股票样本
				int iStartDate,							//起始日期
				int iEndDate,						//结束日期				
				bool bIsAllDate=false					//全部日期
				);
			//个股盈利预测数据（10339）
			bool TxStock::StatForcastData(
				Tx::Core::Table_Indicator &resTable,	//结果数据表
				std::vector<int> & iInstitutionId,		//机构样本
				std::vector<int> & iStockId,			//股票样本
				int iStartDate,							//起始日期
				int iEndDate,						//结束日期				
				bool bIsAllDate=false					//全部日期
				);
			/*天相个股盈利预测 -- 最新的预测净利润
			 *iStockId,股票id
			 *expectYear,年份
			 */
			double GetForcastNetProfit(int iStockId,int expectYear);
			//天相个股盈利预测 -- 最新的预测时间
			int GetForcastIssueDate(int iStockId);

		public:
			//专项统计
			//行情交易：阶段行情、新股首日
			//发行分配：融资预案、发行融资、分红送配、分配预案。
			//股本股东：股本变化、股东人数、十大股东
			//财务分析：报告日期、业绩预告、年报快递、损益表分析
			//人力资源：公司高管
			//PE/PB：市盈率、市净率。
			//盈利预测（优先级中）：个股评级预测数据、最新投资评级、最新盈利预测、评级预测汇总、预测效果。
			//热点数据（优先级低）：限售股流通、股权激励。

			//股本变化统计
			//将所有股本变化记录数据读取到本地内存中
			bool GetAllVariantShareData(void);
			//首先取出指定样本集指定日期区间指定指标的数据记录
			bool	StatVariantShare(
				bool	bReloadAllData,			//是否重新加载全部数据的标志
				int		iStartDate,				//起始日期,-1表示忽略起始日期
				int		iEndDate,				//终止日期,-1表示忽略终止日期,两个-1表示全部日期
				std::set<int>& iInstitutionId,//机构样本
				Tx::Core::Table_Indicator& resTable	//统计结果数据表
				);

			//十大股东统计
			bool GetAllTenShareHoldersData(void);
			//十大流通股东统计
			bool TxStock::GetAllTenTradeableShareHoldersData(void);

			//从报告中得到数据，所以建议采用截止日期
			//取到本地的数据，再进行股东名称筛选
			bool	StatTenShareHolders(
				bool	bReloadAllData,			//是否重新加载全部数据的标志
				int		iStartDate,				//起始日期,-1表示忽略起始日期
				int		iEndDate,				//终止日期,-1表示忽略终止日期,两个-1表示全部日期
				std::set<int>& iInstitutionId,  //机构样本
				Tx::Core::Table_Indicator& resTable,	//统计结果数据表
				bool	bTradeableShare=false,	//true统计十大流通股东;false统计十大股东
				bool	bAnnounceDate=false		//true按照公告日期检索,false按照截止日期检索
				);

			//业绩预告统计
			bool	StatYJYG(
				int iFiscalYear,					//报告期-财年
				int iFiscalQuarter,					//报告期-财务季度
				std::set<int>& iSecurityId,			//需要机构样本，传入交易实体ID
				std::set<int>& iTypeId,				//预告类型ID
				Tx::Core::Table_Indicator& resTable	//统计结果数据表
				);

			//公司高管T_SENIOR_OFFICER_HOLDING
			BOOL GetSeniorOfficer(//LiLi,得到全部数据后如何取得指定日期范围的数据？
								  std::set<int> &iSecurityId,		//交易实体
								  int iSpecifyDate,					   //指定日期
								  Tx::Core::Table_Indicator &resTable,
									bool bNotAllDate
									);//结果数据表
			////Add by lijw 2008-10-06
			////将 财年+报告期-〉报告日期
			//bool TxFund::TransReportDateToNormal2(Tx::Core::Table_Indicator &tempTable,int iCol);
			//Add by lijw 2008-10-06
			BOOL GetDateOfReport(
				                 Tx::Core::Table_Indicator &resTable,//结果数据表
								 std::set<int> &iSecurityId,		//交易实体
								 int iStartDate,					//起始日期
								 int iEndDate,						//终止日期
								 char uReportType = 15,				//报告类型, 从最后一位起, 按位依次是,一季报，中报，三季报，年报, 需要与否用0/1表示
								 BOOL bActualDate = true,			//是否为实际披露日期    该参数为以后修改程序时备用。             
								 bool bGetAllDate=false				//是否全部日期
								 );

			//新股首日
			BOOL GetFirstDateOfStock(//LiLi
                                     Tx::Core::Table_Indicator &resTable,//结果数据表
									std::set<int> &iSecurityId,		//交易实体
				                     //std::set<int> &iInstitutionId,   //机构样本
								     INT uStartDate,					 //起始日期
								     INT uEndDate,
									 bool bSpecialDate							//是否指定日期
									 );
			//年报快递T_FINANCIAL_REPORT_EXPRESS
			BOOL GetFinancialReport(//LiLi
									Tx::Core::Table_Indicator &resTable,//结果数据表
									std::set<int> &iSecurityId,		//交易实体
									INT uStartDate,					 //(公告日期的)起始日期
									INT uEndDate			    	 //(公告日期的)终止日期	
				);

			//十大股东(指标30600018-25)
			BOOL GetTopTenShareHolder(//LiLi
										Tx::Core::Table_Indicator &resTable,//结果数据表
										std::set<int> &iSecurityId,	    	//交易实体
										INT uStartDate,					    //(公告日期的)起始日期
										INT uEndDate,			    	    //(公告日期的)终止日期
										const CString &strKeyWord,          //关键字(股名名称)
										bool AllDate,
										BOOL bExactitude = FALSE		    //是否精确查询
 				);

			//十大流通股东(指标30600026-35)
			BOOL GetTopTenShareHolder_Tradable(//LiLi
										Tx::Core::Table_Indicator &resTable,//结果数据表
										std::set<int> &iSecurityId,	    	//交易实体
										INT uStartDate,					    //(公告日期的)起始日期
										INT uEndDate,			    	    //(公告日期的)终止日期
										const CString &strKeyWord,          //关键字(股名名称)
										bool AllDate,
										BOOL bExactitude = FALSE		    //是否精确查询

				);

		
			//发行融资
			//add by lijw		2007-08-01
			bool StatIssueFinancing(
				std::set<int> & iInstitutionId,			//机构样本
				int iStartDate,							//起始日期
				int iEndDate,						//结束日期
				Tx::Core::Table_Indicator &resTable,	//结果数据表
				int iFXRQ,						//是按发行日期
				bool bGetAllDate=false,					//是否全部日期
				UINT uFlagType=7						//标志位	1:首发	2:配股	3:增发	4:扩募
				);
			//分配预案
			//add by guanyd		2007-08-01
			bool StatDistributionPlan(
				std::set<int> & iInstitutionId,			//机构样本
				int iFinanceYear,						//年份
				int iReportDate	,						//报告期
				Tx::Core::Table_Indicator &resTable	//结果数据表
//				bool bCommitted=true					//是否实施
				);

			//分红派息
			//add by guanyd		2007-08-08
			bool StatDivident(
				std::set<int> iSecurityId,				//交易实体ID
				int iStartDate,							//起始日
				int iEndDate,							//终止日
				Tx::Core::Table_Indicator &resTable,	//结果数据表
				bool bGetAllDate=false,					//是否全部日期
				UINT uFlagType=15						//标志位	1:分红	2:派息	4:配股	8:送股
				);
			//融资预案
			//add by guanyd		2007-08-10
			bool TxStock::StatFinancingPlan(
				std::set<int> & iSecurityId,			//机构样本
				int iStartDate,							//起始日期
				int iEndDate	,						//结束日期
				Tx::Core::Table_Indicator &resTable,	//结果数据表
				bool bGetAllDate=false					//是否全部日期
				);


			//限售股流通
			//add by guanyd		2007-08-10
			bool TxStock::StatLimitedShare(
				std::set<int> & iSecurityId,			//机构样本
				int iStartDate,							//起始日期
				int iEndDate	,						//结束日期
				Tx::Core::Table_Indicator &resTable,	//结果数据表
				bool bGetAllDate=false					//全部日期
				);
			//股本变化
			//add by guanyd		2007-08-18
			bool StatShareChange(
				std::set<int> & iSecurityId,			//机构样本
				int iStartDate,							//起始日期
				int iEndDate	,						//结束日期
				Tx::Core::Table_Indicator &resTable,	//结果数据表
				bool bGetAllDate=false,						//是否全部日期
				UINT uFlagType=31						//标志位	1:首发	2:配股	4:增发	8:送股	16:股权分置改革
				);
			//股东人数 modify by lijw 2008-03-13
			//add by guanyd		2007-08-10
			bool StatHolderNumber(
				std::set<int> & iSecurityId,			//机构样本
				int iStartDate,							//起始日期
				int iEndDate	,						//结束日期
				Tx::Core::Table_Indicator &resTable,	//结果数据表
				bool IsEnd=true,								//判断是公告日期还是截止日期
				bool bIssueAfterStart=true,				//是否剔除在起始日期后上市
				int SpecifyCount=0						//指定的股东人数
				);

			//板块：交易提示
			bool BlockTradePrompt(
				std::set<int> iSecurityId,				//交易实体ID
				int iStartDate,							//起始日
				int iEndDate,							//终止日
				Tx::Core::Table_Indicator &resTable		//结果数据表
				);

			//综合：交易提示
			//Add by guanyd
			//分送配
			bool ColligationTradePromptFPS(
				std::set<int> iSecurityId,				//交易实体ID
				int iStartDate,							//起始日
				int iEndDate,							//终止日
				Tx::Core::Table_Indicator &resTable,	//结果数据表
				bool bGetAllDate=false					//是否全部日期
				);
			//发行增发
			bool ColligationTradePromptFXZF(
				std::set<int> iSecurityId,				//交易实体ID
				int iStartDate,							//起始日
				int iEndDate,							//终止日
				Tx::Core::Table_Indicator &resTable,	//结果数据表
				bool bGetAllDate=false					//是否全部日期
				);
			//发行上市
			bool ColligationTradePromptFXSS(
				std::set<int> iSecurityId,				//交易实体ID
				int iStartDate,							//起始日
				int iEndDate,							//终止日
				Tx::Core::Table_Indicator &resTable,	//结果数据表
				bool bGetAllDate=false					//是否全部日期
				);	
			//配股
			bool ColligationTradePromptPG(
				std::set<int> iSecurityId,				//交易实体ID
				int iStartDate,							//起始日
				int iEndDate,							//终止日
				Tx::Core::Table_Indicator &resTable,	//结果数据表
				bool bGetAllDate=false,				//是否全部日期
				bool bIsFXRQ = false,                    //是发行日期统计。
				bool bIsSSRQ = false						//是按上市日期统计
				);


			//板块风险分析[高级]
			bool BlockRiskIndicatorAdv(
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
				int bUserRate=0,					//0=忽略无风险收益率；1取一年定期基准利率计算无风险收益率；2用户自定义年利率计算无风险收益率
				double dUserRate=0.034,				//用户自定义年利率,否则取一年定期基准利率
				bool bDayRate=true,					//日利率
				bool bPrw = false,					//是否包含进度条
				bool bMean = false                  //收益均值
				);

			//bool GetPEValue(
			//	//Tx::Core::Table_Indicator& resTable,// 结果数据表
			//	Tx::Core::Table& resultTable,
			//	std::set<int> iSecurityId,			// 交易实体ID
			//	UINT nStartDate,					// 起始日期
			//	UINT nEndDate,						// 终止日期
			//	int iSJZQ,							// 时间周期 1-日,2-周,3-月,4-年
			//	int iHZYB,							// 汇总样本 1-全部,2-剔除亏损,3-剔除微利(元)
			//	double dwl,						// 剔除微利时的微利
			//	int iJQFS,							// 加权方式 1-总股本,2-流通股本
			//	int iCWSJ,							// 财务数据:报告期的选择 按位与 0xFFFF
			//										// 由高到低分别表示 指定报告期/选择报告期,年报,中报,季报
			//										// 指定报告期的报告期格式
			//	UINT nFiscalyear,					// 财年
			//	UINT nFiscalyearquater,				// 财季
			//	int iJSFF,							// 计算方法 1-简单,2-滚动,3-同比
			//	bool bClosePrice					// 使用收盘价(true),均价(false)
			//	);

			//// test for load when enter
			//bool GetPEValue(
			//	//Tx::Core::Table_Indicator& resTable,// 结果数据表
			//	Tx::Core::Table& resultTable,
			//	std::set<int> iSecurityId,			// 交易实体ID
			//	UINT nStartDate,					// 起始日期
			//	UINT nEndDate,						// 终止日期
			//	int iSJZQ,							// 时间周期 1-日,2-周,3-月,4-年
			//	int iHZYB,							// 汇总样本 1-全部,2-剔除亏损,3-剔除微利(元)
			//	double dwl,						// 剔除微利时的微利
			//	int iJQFS,							// 加权方式 1-总股本,2-流通股本
			//	int iCWSJ,							// 财务数据:报告期的选择 按位与 0xFFFF
			//	// 由高到低分别表示 指定报告期/选择报告期,年报,中报,季报
			//	// 指定报告期的报告期格式
			//	UINT nFiscalyear,					// 财年
			//	UINT nFiscalyearquater,				// 财季
			//	int iJSFF,							// 计算方法 1-简单,2-滚动,3-同比
			//	bool bClosePrice,					// 使用收盘价(true),均价(false)
			//	std::unordered_map<int,Tx::Core::Unique_Pos_Map*>* pid_posinfo_map = NULL
			//	);

			//bool GetPBValue(
			//	//Tx::Core::Table_Indicator& resTable,// 结果数据表
			//	Tx::Core::Table& resultTable,
			//	std::set<int> iSecurityId,			// 交易实体ID
			//	UINT nStartDate,					// 起始日期
			//	UINT nEndDate,						// 终止日期
			//	int iSJZQ,							// 时间周期 1-日,2-周,3-月,4-年
			//	int iHZYB,							// 汇总样本 1-全部,2-剔除亏损,3-剔除微利(元)
			//	double dwl,						// 剔除微利时的微利
			//	int iJQFS,							// 加权方式 1-总股本,2-流通股本
			//	int iCWSJ,							// 财务数据:报告期的选择 按位与 0xFFFF
			//										// 由高到低分别表示 指定报告期/选择报告期,年报,中报,季报
			//										// 指定报告期的报告期格式
			//	UINT nFiscalyear,					// 财年
			//	UINT nFiscalyearquater,				// 财季
			//	int iJSFF,							// 计算方法 1-简单,2-滚动,3-同比
			//	bool bClosePrice					// 使用收盘价(true),均价(false)
			//	);
			//bool GetPBValue(
			//	//Tx::Core::Table_Indicator& resTable,// 结果数据表
			//	Tx::Core::Table& resultTable,
			//	std::set<int> iSecurityId,			// 交易实体ID
			//	UINT nStartDate,					// 起始日期
			//	UINT nEndDate,						// 终止日期
			//	int iSJZQ,							// 时间周期 1-日,2-周,3-月,4-年
			//	int iHZYB,							// 汇总样本 1-全部,2-剔除亏损,3-剔除微利(元)
			//	double dwl,						// 剔除微利时的微利
			//	int iJQFS,							// 加权方式 1-总股本,2-流通股本
			//	int iCWSJ,							// 财务数据:报告期的选择 按位与 0xFFFF
			//	// 由高到低分别表示 指定报告期/选择报告期,年报,中报,季报
			//	// 指定报告期的报告期格式
			//	UINT nFiscalyear,					// 财年
			//	UINT nFiscalyearquater,				// 财季
			//	int iJSFF,							// 计算方法 1-简单,2-滚动,3-同比
			//	bool bClosePrice,					// 使用收盘价(true),均价(false)
			//	std::unordered_map<int,Tx::Core::Unique_Pos_Map*>* pid_posinfo_map = NULL
			//	);
			//2007-08-07
			//一般行业利润表(新会计准则)
			bool GetIncomeStatementCommercialIndustryActstd(
				Tx::Core::Table_Indicator& resTable,//结果数据表
				int iSecurityId,					//交易实体ID
				int iConsolidated,					//1=母公司;2=合并;3=母公司|合并
				int iFiscalYear,					//报告期-财年
				int iFiscalQuarter,					//报告期-财务季度
				int iAjustment						//0：既不是调整前也不是调整后 1：调整前 2：调整后  3：既是调整前也是调整后
				);



			//2007-08-09
			//add by guanyd
			//功能：首先将关联表添加到map，
			//根据源表指定列作为索引从map中获取新列添加到源表指定位置,被关联表默认为0列map
		//	template<class T>
			bool FillColumn(
				Tx::Core::Table_Indicator &m_transTable,		//关联表
				int addColumn,								//关联表中数据的列
				Tx::Core::Table_Indicator &m_resTableAddColumn,	//源表
				int indexColumn,							//源表作为map索引的列
				int	insertColumn							//要插入数据的列
			);
			
			//2007-09-04
			//guanyd
			//将交易实体id转为券id或者机构id
			bool TxStock::TransObjectToSecIns(
				std::set<int>	sObjectId,		//交易实体id
				std::set<int> &sSecInsId,		//券id或机构id
				int iType						//id类型：1:券id	2:机构id
				);
			bool TxStock::TransObjectToSecIns(
				std::set<int>	sObjectId,		//交易实体id
				std::vector<int> &sSecInsId,		//券id或机构id
				int iType						//id类型：1:券id	2:机构id
				);
			//guanyd
			//2007-09-04
			//将任意选定日期回溯到第一个报告日期
			bool TxStock::ChangeDateToReportdate(int& iStartDate,int& iEndDate);

			//Add by guanyd
			//2007-08-31
			//将两张表根据券id对齐，因为id的全集为1600左右，而且在本表中出现不会重复
			//
			bool TxStock::AlignRows(
					Tx::Core::Table_Indicator& resTableFir,		//较大的表
					int iFirCol,								//大表中扩展式保证数据一致的列
					Tx::Core::Table_Indicator& resTableSec,		//被扩展的表
					int iSecCol									//扩展表中保证数据一致的列
					);
			
			//主营业务产品收入
			bool GetIncomeMain(
				Tx::Core::Table_Indicator& resTable,//结果数据表
				int iSecurityId,					//交易实体ID
				int iFiscalYear,					//报告期-财年
				int iFiscalQuarter					//报告期-财务季度
				);
			
			public:
				//读取索引文件信息
				BOOL GetIndexDat(int file_id,	//索引文件标识即文件名在IndexDatFileName数组中的位置 
				std::unordered_map<int,CString>& indexdat_map //存放读取的信息
				);
			public:
				struct IndexStruct
				{
					int	id;				//参数描述ID
					char description[200];	//参数描述,数据表设计长度为，目前暂定
				};

				// added by zhoup 2007.12.20
				// 新的市盈率计算接口
				bool GetPEPBValue(
					//Tx::Core::Table_Indicator& resTable,// 结果数据表
					Tx::Core::Table& resultTable,
					std::set<int> iSecurityId,			// 交易实体ID
					UINT nStartDate,					// 起始日期
					UINT nEndDate,						// 终止日期
					int iSJZQ,							// 时间周期 1-日,2-周,3-月,4-年
					int iHZYB,							// 汇总样本 1-全部,2-剔除亏损,3-剔除微利(元)
					double dwl,						// 剔除微利时的微利
					int iJQFS,							// 加权方式 1-总股本,2-流通股本
					int iCWSJ,							// 财务数据:报告期的选择 按位与 0xFFFF
					// 由高到低分别表示 指定报告期/选择报告期,年报,中报,季报
					// 指定报告期的报告期格式
					UINT nFiscalyear,					// 财年
					UINT nFiscalyearquater,				// 财季
					int iJSFF,							// 计算方法 1-简单,2-滚动,3-同比,4-静态
					bool bClosePrice,					// 使用收盘价(true),均价(false)
					bool bPE,							// 是否计算PE
					bool bPB,							// 是否计算PB
					std::unordered_map<int,Tx::Core::Unique_Pos_Map*>* pid_posinfo_map = NULL
					);

				// added by zhoup 2009.04.28
				// 板块市盈率统计接口
				bool GetBlockPEPB(
					Tx::Core::Table& resultTable,
					std::set<int> setBlockId,			// 板块ID
					UINT nStartDate,					// 起始日期
					UINT nEndDate,						// 终止日期
					int iSJZQ,							// 时间周期 1-日,2-周,3-月,4-年
					int iJSFF,							//计算方法
					bool bPE,							// 是否计算PE
					bool bPB							// 是否计算PB
					);
				// added by zhangxs 2009.11.24
				// 板块资金流向统计接口
				bool GetBlockCashFlow(
					Tx::Core::Table_Display& resultTable,
					UINT nStartDate,					// 起始日期
					UINT nEndDate,						// 终止日期
					int iResqType=0,					// 请求类型
					bool bStat = true,
					bool bDurRaise = true,
					int iMarketid = 0
					);
				// added by zhangxs 2009.11.30
				// 个股资金流向统计接口
				bool GetSampleCashFlow(
					Tx::Core::Table_Display& resultTable,
					std::vector<int> vecBlockId,		// 板块ID
					UINT nStartDate,					// 起始日期
					UINT nEndDate,						// 终止日期
					int iResqType,						// 请求类型
					bool bAddSamplesOnly=false,			//添加样本不统计
					bool bStat = true,
					bool bDurRaise = true,
					int iMarketid = 0,
					bool bFocusSamples = true
					);
				//取得利润或者权益的数据
				//add by wangyc 20081224
				bool GetDataOfProfitAndRight(
					int iYear,							//财年
					int iQuarter,						//财季
					bool iIsProfit,						//是利润还是权益 true 利润 false 权益
					std::vector<int>& iSecurityId,			//机构的ID
					std::vector<double>* iDateVector	//获得利润或者权益数据
					);
				//2008-04-25
				//排序分析-增加列
				virtual bool GetBlockAnalysisCol(Table_Display& baTable,std::vector<int>& arrSamples,int& iSortCol);
				virtual bool SetBlockAnalysisBasicCol(Table_Display& baTable,SecurityQuotation* psq,int& j,int ii,int idate,int iSamplesCount);
				virtual bool SetBlockAnalysisCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow);
				//2008-11-03
				//AH股溢价率
				bool GetAHData(std::vector<int> iSecurityId,int startDate,int endDate,Table_Display& baTable);

				// 增加一个汇率的结构给PE/PB计算用
				struct ExRate 
				{
					int iDate;
					double iRatio;
				public:
					int GetMapObj(int index=0) { return iDate; }
				};
		};
	}
}
#endif