/**************************************************************
Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
File name:		TxBusiness.h
Author:			赵宏俊
Date:			2007-07-03
Version:		1.0
Description:	业务类的基类
集中处理各类交易实体具有共同属性的业务
为各类交易实体提供基本支持
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
		//业务数据类
		//从数据层取得数据
		//如果需要计算,队取得的数据根据指标要求进行计算
		//将计算结果提交给功能窗口

		//RUNTIME_CLASS( TxBusiness ) == this->GetRuntimeClass() 
		//IsKindOf(RUNTIME_CLASS( TxBusiness ))
		class BUSINESS_EXT TxBusiness : public CObject  
		{
			DECLARE_DYNCREATE(TxBusiness)
			//构造区
		public:
			TxBusiness(void);
			virtual ~TxBusiness(void);

			//内存区
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

			//业务区
		public:

			//取得用户名对应的公司名称。临时的东西，想来想去搁这最好了
			static CString GetUserCompanyName(void);
			//2009-06-25
			double GetBigBillValue(SecurityQuotation* p,int date);
			void StatDetailData(std::vector<int>& vSample,int iStart,int iEnd,CString sBlockName=_T("板块"));
			void StatDetailData(int iCollectionId,int iStart,int iEnd);

			//2009-07-01
			//周期资金流入流出
			bool GetAmountFlow(
				Table_Display& resTable,			//结果数据表
				std::vector<int>	&iSecurityId,	//输入样本集
				int		iStartDate,				//起始日期
				int		iEndDate,				//终止日期
				int		iTimeCycle,				//时间周期。日周月季年
				std::vector<CString> &vDates,	//日期标题
				std::vector<CString> &vColName	//数据列标题
				);

			//2009-06-29
			//周期资金流入流出
			bool GetAmountFlow(
				std::vector<int>& vecSecurityId,	//样本
				int istart_date,					//起始日期
				int iend_date,						//终止日期
				Table_Display& baTable,				//结果
				int iFlag,							//周期类型
				int iMarketid						//交易所
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
			//实时资金流入流出求和
			//取得指数实时资金流入流出
			bool GetAmountFlow(int iIndexId,double& dAmountIn,double& dAmountOut);
			//取得指定样本的实时资金流入流出并求和
			bool GetAmountFlow(std::vector<int>& vecSecurityId,double& dAmountIn,double& dAmountOut);
			//2008-09-05
			//历史资金流入流出
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
			//取得指数样本涨跌数量
			bool GetIndexSamplesStatus(int iIndexSecurityId,int& iUp,int& iDown,int& iEqual);
			bool GetIndexSamplesStatus(SecurityQuotation* p,int& iUp,int& iDown,int& iEqual);
			bool GetIndexSamplesStatus(SecurityQuotation* p,int& iUp,int& iDown,int& iEqual,int& iTop,int& iBottom,int& iHalt,bool bGetHalt=false);

			//历史行情

			//取得指定交易实体的历史行情数据
			bool GetSecurityHisTrade(int iSecurityId);
			bool GetSecurityHisTrade(Security* p);

			//取得指定交易实体的交易日期序列
			bool GetSecurityTradeDate(int iSecurityId);
			bool GetSecurityTradeDate(Security* p);

			//文件下载区
			//根据指定的交易实体内码和功能代码,下载XML文件到本地
			CString DownloadXmlFile(int iSecurityId,int iFunctionId);

			//指标
			//2007-10-24
			//根据其他开发人员的要求，满足各种使用习惯，特修改
			IndicatorData* GetIndicatorDataNow(long iIndicatorId)
			{
				m_pIndicatorData = (IndicatorData*)GetIndicatorData(iIndicatorId);
				return m_pIndicatorData;
			}

			//交易实体
			//2007-10-24
			//根据其他开发人员的要求，满足各种使用习惯，特修改
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
			//删除指定交易实体
			bool RemoveSecurityNow(int iSecurityId)
			{
				return RemoveSecurity(iSecurityId);
			}

			//交易实体券种判断
			//可以在子类中判断交易实体的券种类型
			//递归模式，完全支持集合树
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
			//查表模式，支持集合树里已经进行全排列的集合节点
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


			//取得集合样本
			//专门针对证监会行业和天相行业
			bool	GetCollectionItems(int iCollectionId,std::set<int>& items)
			{
				return m_pFunctionDataManager->GetItemsFromSecurity(iCollectionId,items);
			}
			bool	GetCollectionItems(CString sCollectionName,std::set<int>& items)
			{
				return m_pFunctionDataManager->GetItemsFromSecurity(sCollectionName,items);
			}
			//根据配置文件中也字节点的Key取得样本
			bool	GetLeafItems(CString sLeafName,std::vector<int>& items)
			{
				return m_pFunctionDataManager->GetLeafItems(items,sLeafName);
			}
			//2009-03-11
			//过滤样本
			bool SelectItems(
				SecurityQuotation* p,
				bool bSt,		//ST
				bool bStop,		//摘牌
				bool bHalt,		//长期停牌
				bool bIssued	//已发行未上市
				);
			bool SelectItems(
				std::vector<int>& src,
				std::vector<int>& dst,
				bool bSt,		//ST
				bool bStop,		//摘牌
				bool bHalt,		//长期停牌
				bool bIssued	//已发行未上市
				);
			bool SelectItems(
				std::set<int>& src,
				std::set<int>& dst,
				bool bSt,		//ST
				bool bStop,		//摘牌
				bool bHalt,		//长期停牌
				bool bIssued	//已发行未上市
				);
			bool SelectItems(
				std::vector<int>& src,
				std::set<int>& dst,
				bool bSt,		//ST
				bool bStop,		//摘牌
				bool bHalt,		//长期停牌
				bool bIssued	//已发行未上市
				);
			bool SelectItems(
				std::set<int>& src,
				std::vector<int>& dst,
				bool bSt,		//ST
				bool bStop,		//摘牌
				bool bHalt,		//长期停牌
				bool bIssued	//已发行未上市
				);


			//2008-09-25
			//取得指定机构id所属的样本
			//针对机构,券,交易实体三级映射关系专用接口
			//如果bIsSecurityId=true表示样本是交易实体,bIsSecurityId=false表示样本是券
			bool	GetItemsByInstitution(
				int iInstitutionId,			//机构id
				std::vector<int>& items,	//样本
				bool bIsSecurityId=true		//是否交易实体样本
				);
			//取得指定券id所属的交易实体样本
			//针对券,交易实体映射关系专用接口，所有券均可用
			bool	GetItemsBySecurity(
				int iSecurity1Id,			//券id
				std::vector<int>& items	//样本
				);
			//2008-10-17
			//代销机构
			bool GetSecurityBySaleAgency(int iSaleAgencyId,std::vector<int>& items);

			//统计准备=添加样本
			bool AddSampleBeforeStat(
				int&	iCol,				//列索引值
				int		iStartDate,			//统计时间区间的起始日期
				int		iEndDate,			//统计时间区间的终止日期
				std::set<int>& iSecurity	//样本集
				);
			//统计准备=添加指标
			bool AddIndicatorBeforeStat(
				int		iIndicator,			//统计指标
				int		iStartDateIndex,	//统计时间区间的起始日期的参数列所引值
				int		iEndDateIndex		//统计时间区间的终止日期的参数列所引值
				);

			//由于板块业务功能具有共性，所以在业务类的基类里实现
			/*
			2008-01-09
			GetBlockAnalysis	为积累里的框架接口，只负责排序分析的数据处理流程
			GetBlockAnalysisCol 负责排序分析的结果数据的标题列信息的建立,各交易实体特殊列信息在子类里处理
			SetBlockAnalysisBasicCol 负责各种交易实体共性指标的数据处理,各种交易实体的基本指标保持不变
			SetBlockAnalysisCol	负责各种交易实体特性指标的数据处理,在子类里处理

			//各接口已经进行默认数据的处理
			*/
			//板块=排序分析
			//baTable已经填充了样本[交易实体]
			//virtual bool GetBlockAnalysis(Table_Display& baTable,std::set<int>& arrSamples,bool bNeedProgressWnd=false);
			virtual bool GetBlockAnalysis(Table_Display& baTable,std::vector<int>& arrSamples,int& iSortCol,int iMarketId=0,bool bNeedProgressWnd=false,int nCollID=-1);
			//添加行
			virtual bool BlockAnalysisAddRow(Table_Display& baTable,SecurityQuotation* pSecurity,int idate);
			//排序分析数据结果标题列
			virtual bool GetBlockAnalysisCol(Table_Display& baTable,std::vector<int>& arrSamples,int& iSortCol);
			//排序分析基本数据结果设置
			virtual bool SetBlockAnalysisBasicCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow,int idate,int iSamplesCount);
			//排序分析基本数据结果设置[换手率]
			virtual bool SetBlockAnalysisHslCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow);
			//排序分析数据结果设置
			virtual bool SetBlockAnalysisCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow);

			//设置样本
			//bool SetBlockSamples(Table_Display& baTable,std::set<int>& arrSamples,int iCol);
			bool SetBlockSamples(Table_Display& baTable,std::vector<int>& arrSamples,int iCol,int iInitNo);

			//2008-07-29
			//行情数据
			virtual bool GetTradeData(Table_Display& baTable,int iSecurityId,int iFlag=0,bool bFirst=false,int iWeight=-1,bool bNeedProgressWnd=false);
			//设置数据表的列信息
			virtual bool SetTableCol(Table_Display& baTable);
			//行情数据结果
			virtual bool SetTableData(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow,int idate,int iFlag=0,bool bFirst=false,int iWeight=-1);
			//设置股票数据表的列信息
			bool SetStockTableCol(Table_Display& baTable,SecurityQuotation* pSecurity);
			//取得股票行情数据
			bool GetStockTradeData(Table_Display& baTable,
				int iSecurityId,
				int iStartdate,
				int iEndDate,
				int iFlag=0,
				bool bFirst=false,
				int iWeight=-1,
				bool bNeedProgressWnd=false);

			//取得基金净值数据
			bool GetFundNavData(Table_Display& baTable,
				int iSecurityId,
				int iStartdate,
				int iEndDate,
				int iFlag=0,
				bool bFirst=false,
				int iWeight=-1,
				bool bNeedProgressWnd=false);

			//设置指数数据表的列信息
			bool SetIndexTableCol(Table_Display& baTable);
			//取得指数行情数据
			bool GetIndexTradeData(Table_Display& baTable,
				int iSecurityId,
				int iStartdate,
				int iEndDate,
				int iFlag=0,
				bool bFirst=false,
				int iWeight=-1,
				bool bNeedProgressWnd=false);
			bool SetNormalTableCol(Table_Display& baTable,SecurityQuotation* pSecurity);
			//取得行情数据
			bool GetNormalTradeData(Table_Display& baTable,
				int iSecurityId,
				int iStartdate,
				int iEndDate,
				int iFlag=0,
				bool bFirst=false,
				int iWeight=-1,
				bool bNeedProgressWnd=false);
			bool SetBondTableCol(Table_Display& baTable, int nPrecise = 2);	// 2011-03-03 修改深市债券价格小数点位数
			//取得债券行情数据
			bool GetBondTradeData(Table_Display& baTable,
				int iSecurityId,
				int iStartdate,
				int iEndDate,
				int iFlag=0,
				bool bFirst=false,
				int iWeight=-1,
				bool bNeedProgressWnd=false,
				bool bIsFullPrice = false);
			//债券行情数据
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
			//根据周期来转换交易数据
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
			//股票行情数据
			bool SetStockTableData(Table_Display& baTable,
				SecurityQuotation* pSecurity,
				std::vector<HisTradeData> m_vecHisTrade,
				int iStartdate,
				int iEndDate,
				int iFlag,
				bool bFirst,
				int iWeight);
			//指数行情数据
			bool SetIndexTableData(Table_Display& baTable,
				SecurityQuotation* pSecurity,
				std::vector<HisTradeData> m_vecHisTrade,
				int iStartdate,
				int iEndDate,
				int iFlag,
				bool bFirst,
				int iWeight);
			//行情数据
			bool SetNormalTableData(Table_Display& baTable,
				SecurityQuotation* pSecurity,
				std::vector<HisTradeData> m_vecHisTrade,
				int iStartdate,
				int iEndDate,
				int iFlag,
				bool bFirst,
				int iWeight);
			//板块=定价特征
			//根据板块样本计算定价特征数据
			//每次计算一个指定的交所；如果lBourseId=则计算所有样本，不区分交易所
			bool GetBlockPricedFuture(
				std::set<int>& iSecurityId,	//板块样本集
				Tx::Core::Table& baTable,	//计算结果
				int icalc_type,				//计算方法
				//int iSecurityType			//板块样本的券种类型0=股票；1=债券；2=指数；3=权证
				bool bNeedPsw=true
				);

			//板块样本定价特征明细数据
			/*
			iItem使用说明
			0:市值
			1:流通市值 
			2:含亏损，总股本加权市盈率
			3:不含亏损，总股本加权市盈率
			4:含亏损，流通股本加权市盈率
			5:不含亏损，流通股本加权市盈率

			6:含亏损，总股本加权市净率
			7:不含亏损，总股本加权市净率
			8:含亏损，流通股本加权市净率
			9:不含亏损，流通股本加权市净率

			10:净资产收益率[含负净资产]
			11:净资产收益率[不含负净资产]

			12:样本算术平均价
			13:样本总股本加权平均价
			14:样本流通股本加权平均价

			15:加权平均每股收益[含亏损]
			16:加权平均每股收益[不含亏损]

			17:净资产
			18:国有净资产
			19:每股净资产

			20:总股本
			21:国有股本
			22:流通股本
			*/
			bool GetBlockPricedFuture(
				std::set<int>& iSecurityId,	//板块样本集
				Tx::Core::Table& resTable,	//定价特征计算结果
				long lBourseId,				//交易所ID,0表示所有样本
				int iCalcType,				//计算方法 0=静态;1=滚动;2=简单;3=同比
				//int iSecurityType			//板块样本的券种类型0=股票；1=债券；2=指数；3=权证
				bool bNeedPsw=true
				);
			//设置定价特征单元个数据
			bool TxBusiness::SetBlockPricedFutureCell(
				Tx::Core::Table& resTable,	//定价特征计算结果
				CString sName1,	//0=大类
				CString sName2,	//1=小类
				double dValue,	//2=指标数据[沪市，深市，合计]
				int iValueScale,//2.1指标数据的小数位数
				CString sDetail,//3=查看明细[在界面上查看明细]
				int tot_count,	//4=输入样本数=tot_count
				int countByBourse,	//5=本市场样本数=countByBourse
				int invalid_count,	//6=无效样本数量=invalid_count
				int invalid_tradestatus_count,//7=当日交易状态不正常数量=invalid_tradestatus_count
				int invalid_pepb_count,	//8=PEPB数据无效数量=invalid_pepb_count
				int sampleCount,		//9=实际参与运算的样本数=sampleCount
				int invalid_share,		//10=对应股本无效数据的样本数=invalid_share
				int invalid_insititutionshare,//11=公司总股本无效数据的样本数=invalid_insititutionshare
				int iLR,//12=亏损家数=iLR
				int iZC,//13=负净资产家数=iZC
				int invalid_tradeableshare,//14=流通股本无效数据样本数=invalid_tradeableshare
				int invalid_lr,//15无效利润数据的样本数=invalid_lr
				int invalid_equity,	//16无效股东权益数据的样本数=invalid_equity;
				double dShareHolder_Equity_Parent2,//17股东权益[不含负净资产]
				double dProfitCur1,	//18不含亏损+不含负净资产=利润
				double dShareHolder_Equity_Parent1,//19不含亏损+不含负净资产=净资产
				double dProfitCur,	//20 含亏损的利润
				double dShareHolder_Equity_Parent,//21 股东权益[含负净资产]
				double dProfitCur2	//22不含亏损利润
				);
			//取得指定计算方法的利润
			double GetLR(SecurityQuotation* psq,int calc_type,int* invalid_lr=NULL);

			bool GetBlockSamplesPricedFutureDetail(
				int iItem,							//指定输出数据项目
				std::set<int>& iSecurityId,			//板块样本集
				Tx::Core::Table_Indicator& resTable,//结果数据
				int calc_type //0,1,2,3
				);

			//取得单个指定交易实体的除权数据
			bool GetSecurityExdividend(
				int iSecurityId,					//交易实体ID
				Tx::Core::Table_Indicator& resTable	//取得iSecurityId的除权数据
				);
			//取得单个指定券的除权数据
			bool GetSecurity1Exdividend(
				int iSecurity1Id,					//券ID
				Tx::Core::Table_Indicator& resTable	//取得iInstitutionId的除权数据
				);
			//2007-08-23
			//板块热点分析
			bool BlockHotFocus(
				Tx::Core::Table& resTable,	//结果数据表
				std::vector<int> iBloackSamples//板块样本ID
				);
			//板块风险分析
			bool BlockRiskIndicator(
				int				iFunID,				//功能ID
				std::set<int>& iSecurityId,			//交易实体ID
				int date,							//数据日期
				Tx::Core::Table_Indicator& resTable,//结果数据表
				bool bPrice=false	//现价标志
				);
			//2009-05-05
			//创建板块风险分析[高级]数据表
			bool RiskIndicatorAdv(
				Tx::Core::Table_Indicator& resTable//结果数据表
				);
			//2007-09-19
			//板块风险分析[高级]
			bool RiskIndicatorAdv(
				Tx::Core::Table_Indicator& resTable,//结果数据表
				std::set<int>& lSecurityId,			//交易实体ID
				int iEndDate,						//截止日期
				long lRefoSecurityId,				//基准交易实体ID
				bool bFirstDayInDuration = false,	//默认周期期初
				int iStartDate=0,					//起始日期
				int iTradeDaysCount=20,				//交易天数
				int iDuration=0,					//交易周期0=日；1=周；2=月；3=季；4=年
				bool bLogRatioFlag=false,			//计算比率的方式,true=指数比率,false=简单比率
				bool bClosePrice=true,				//使用收盘价
				int  iStdType=0,					//标准差类型1=有偏，否则无偏
				int  iOffset=0,						//偏移
				bool bAhead=true,					//复权标志,true=前复权,false=后复权
				//bool bUserRate=true,				//用户自定义年利率,否则取一年定期基准利率
				int bUserRate=0,					//0=忽略无风险收益率；1取一年定期基准利率计算无风险收益率；2用户自定义年利率计算无风险收益率
				double dUserRate=0.034,				//用户自定义年利率,否则取一年定期基准利率
				bool bDayRate=true,					//日利率
				bool bPrw = false,					//进度条显示与否
				bool bMean = false                  //是否在结果表中加入收益均值数据
				);

			//2007-10-08
			//板块阶段行情[涨幅]
			bool BlockCycleRate(
				std::set<int>& iSecurityId,			//交易实体ID
				int date,							//数据日期
				Tx::Core::Table_Indicator& resTable,//结果数据表
				bool bPrice=false//现价标志
				);
			//板块阶段行情[涨幅][高级]
			bool BlockCycleRateAdv(
				std::set<int>& iSecurityId,			//交易实体ID
				int startDate,						//起始日期
				int endDate,						//终止日期
				bool bCutFirstDateRaise,			//剔除首日涨幅
				int	iFQLX,							//复权类型 0-不复权;1=后复权
				Tx::Core::Table_Indicator& resTable,//结果数据表
				int iPrecise=2,						//小数点位数
				int iFlag=0,						//计算类型0-默认；1-债券[bCutFirstDateRaise=true表示全价模式,bCutFirstDateRaise=false表示净价模式]
				int iMarketId=0,					//交易所ID
				bool bOnlyRaise=false,				//只计算涨幅
				bool bUseCurData=false				//是否使用当日数据
				);
			bool BlockCycleRateAdv(
				std::vector<int>& iSecurityId,			//交易实体ID
				int startDate,						//起始日期
				int endDate,						//终止日期
				bool bCutFirstDateRaise,			//剔除首日涨幅
				int	iFQLX,							//复权类型 0-不复权;1=后复权
				Tx::Core::Table_Indicator& resTable,//结果数据表
				int iPrecise=2,						//小数点位数
				int iFlag=0,						//计算类型0-默认；1-债券[bCutFirstDateRaise=true表示全价模式,bCutFirstDateRaise=false表示净价模式]
				int iMarketId=0,					//交易所ID
				bool bOnlyRaise=false,				//只计算涨幅
				bool bUseCurData=false				//是否使用当日数据
				);
			//综合屏阶段涨幅
			bool BlockCycleRateHome(
				std::vector<int>& iSecurityId,			//交易实体ID
				int date,							//数据日期
				Tx::Core::Table_Display& table, //结果数据表
				bool bHaveIndex = false  //是否包含自选股指数
				);
		protected:
			virtual double GetBondInterest(SecurityQuotation* pSecurity,int iDate);

		public:
			//2008-01-07
			//取得两市场交易日期序列
			bool GetHuShenTradeDate(void);

			//2007-09-18从TxStock移植过来
			//行情数据同步
			//以基准交易实体为参照,按照多剔少补的原则,将样本交易实体同基准交易实体进行同步
			bool SynTradeData(
				Security* pBase,				//基准交易实体
				Security* pSample,				//样本交易实体
				HisTradeData* pSampleSecurityTradeData,//同步后样本行情的数据
				int baseDate,					//截止日期
				int iDays,						//准备处理的行情日期天数
				bool bIsToday,					//截止日期是当日，且当日是交易日
				bool bSameDuration,				//截止日期是当日，且当日是交易日,与行情文件最后数据日期同周期
				int iDuration=0,				//交易周期0=日；1=周；2=月；3=季；4=年
				bool bFirstDayInDuration=true	//true=周期期初；false=周期期末
				);

			//计算比率
			double CalcRatio(double x,double y,bool bLogRatioFlag)
			{
				if(x == y || x == 0) return 0;
				if(bLogRatioFlag == true)//自然对数
				{
					if(y==0) return 0;
					return log(y) - log(x);
				}
				else//简单比率
					return (y - x)/x;
			}
			//add by guanyd 20071127
			void TableOutput(Tx::Core::Table_Indicator &resTable);
		public:
			//设置数据表的列信息
			void SetDisplayTableColInfo(Table_Display* pTable,int iCol,int iIndicatorId,bool bSetTitle=true);

		public:
			//处理实型数据小数位数
			int GetDataByScale(float fValue,int iScale,int& iRatio);
			//int GetDataByScale(int iScale);
			//2008-04-28
			//取得预测pe的年度
			int GetPeYear(void);
			//取得预测pe的年度
			int GetPeYear(std::vector<int> arrSamples);
		public:
			//仅仅使用
			//使用之前调用 GetIndicatorDataNow
			//取得指标数据
			IndicatorData*			m_pIndicatorData;

			//临时数据结果表
			Tx::Core::Table_Indicator m_txTable;
			//仅仅使用
			//使用之前调用 GetSecurityNow
			//取得交易实体相关操作
			SecurityQuotation*		m_pSecurity;

			//上证指数的指针4000228
			SecurityQuotation*		m_pShIndex;
			//深市成份指数4000001
			SecurityQuotation*		m_pSzIndex;
			//2008-01-08
			//沪深两市交易日期序列
			MixTradeDay*	m_pMixTradeDay;

		protected:
			//券种分类类型ID
			int						m_iSecurityTypeId;

			//美元兑人民币的汇率
			double					m_dDollar2RMB_ratio;

			//所有除权数据
			//建议加载一次
			Tx::Core::Table_Indicator m_AllExdividendDataTable;
			//取得pe公布的最新财年
			int						m_iYearPe;

			//私有成员区
		public:
			//取得集合[板块]的相关操作
			FunctionDataManager*	m_pFunctionDataManager;
			//取得指标的相关操作
			LogicalBusiness*		m_pLogicalBusiness;

			// added by zhoup 2008.12.01
			// 实现对A、B股的重复过滤
			// 增加一个样本过滤接口：当同时出现A、B股的时候，将B股过滤掉；只有A或者B股的时候，不过滤
			// 传入的参数即为最后的结果
			void InistitutionFilter(std::vector<int>& vecSecurity);

			// added by zhoup 2008.12.01
			// 实现对A、B股的重复过滤
			// 增加一个样本过滤接口：当同时出现A、B股的时候，将B股过滤掉；只有A或者B股的时候，不过滤
			// 传入的参数即为最后的结果
			void InistitutionFilter(std::set<int>& setSecurity);
		private:
			void CalStatusAboutToday(SecurityQuotation * psq, int date, bool & bNeedCheckToday, bool & bNeedAppendToday);
			void PushBackToday(bool bNeedAppendToday,SecurityQuotation * psq,std::vector<HisTradeData> & m_vecHisTrade);
		public:
			//获取YTM
			virtual void GetBondYTM(std::vector<int> nSecurityIds);
			virtual void GetBondYTM(int nSecurityId,double &dYtm,double &dMdur,double &dCon);
		};
	}
}

#endif