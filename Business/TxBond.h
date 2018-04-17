/**************************************************************
	Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
	File name:		TxBond.h
	Author:			赵宏俊
	Date:			2007-10-29
	Version:		1.0
	Description:	
					债券业务功能类

***************************************************************/

#ifndef __TXBOND_H__
#define __TXBOND_H__

#include "TxBusiness.h"
#include "..\..\Core\TxMath\YTM_curve.h"

#define MIN_JYS_QBOND_LIST_DATE	19961231 //交易所市场最早上市日期

#pragma once
namespace Tx
{
	namespace Business
	{
//计算转债底价需要的数据结构
struct BondCompnySamples 
{
	double left_year;
	double ytm;
};
//2008-05-14 债券测算
//新债测算
//国债测算
//企业债测算
//组合测算
struct BOND_TEST_ITEM {
	char	NAME[31];//名称
	double  MZH;	//面值
	double  PMLL;	//票面利率
	int		FXNX;	//发行年限
	int     TYPE;	//类型: 1浮息/0附息
	int     GBOND;	//1国债/0企债
	int     FXPL;	//付息频率/0每年/1每半年/2每季度/3到期
	int     FXRQ;	//发行日期
	int     DATE;	//日期
};
//用于新债测算的基准利率数据结构
struct NEW_BOND_INTEREST
{
	int iDate;
	double dInterest;
public:
	int GetMapObj(int index=0) { return iDate; }
};
class BUSINESS_EXT TxBond :
	public TxBusiness
{
public:
	TxBond(void);
	virtual ~TxBond(void);
private:
	//	债券特征数据
	double		m_dYTM;				// 到期收益率
	double		m_dDURATION;		// 持久期
	double		m_dCONVEXITY;		// 凸性
	double		m_dMDUR;			// 修正持久期
	float		m_fPRICE;			// 全价或净价[CalcPriceByYtm的计算结果]

	//计息方法
	double	m_dSpanDays;			//
	int		m_iTotalDaysOfYear;		//
	int		m_iNdays;				//由于闰年减去的天数

	//2008-01-07
	//是否优化YTM计算的标志
	//如果每次计算都是同一天的YTM，可以设置为true,否则，有日期变化需要重新修正，所以，设置为false
	bool	m_bCalcYTMoneDay;
	//上次计算M的日期
	int		m_iYtmDate;

	//输入
	long		F_DATE;				//01 日期
	float		F_PRICE;			//02 全价或净价
	bool		F_IS_NETPRICE;		//03 净价标志
	bool		F_HAVE_INTEREST;	//04 应计利息是否已设置
	float		F_INTEREST;			//05 应计利息

	//2009-02-23
	//债券类型=与板块对应
	int	m_iBondType;//0=国债；1=可转债；2=回购

private:
	//计算两日期间的天数
	//从指定日期到付息日期之间的天数
	float CalcPower(
		int iStart,	//输入日期
		int iEnd,	//下次付息日
		int iLip,	//本次付息日
		float fFxpl,//付息频率
		int iFigureMethod
		);
	//计算指定日期基本剩余期限[年+天数]
	bool	GetRemnantsBasic(int iStart,int iEnd,int& iYear,int& iDays,int& iTotDays);
	//计算单位为年的剩余期限
	double	GetRemnantsYear(int iStart,int iEnd,bool bNewBond=false);

	//计算单位为年的剩余期限
	int GetRemnants2Days(int iStart,int iEnd);

	//取得两日期间的天数 [闰年2月29日不计]
	int Get_Span_Days_For_Bond(int start_date, int end_date);

private:
	//根据YTM计算价格
	bool CalcPriceByYtm(int iSecurityId,float fYtm,int iDate,int iBondCount=1);
	bool CalcNewBondPriceByYtm(
		BondNewInfo* pBondNewInfo,//in,模拟债券的基本信息
		DataFileNormal<blk_TxExFile_FileHead,BondCashFlowData>* pCashFlow,//in,模拟债券的现金流
		float fYtm,
		int iDate,
		int iBondCount=1
		);

	//取得指定剩余期限的企业债的剩余期限和YTM列表
	bool GetCompnyBondByLeftYear(CArray<BondCompnySamples>& sampleList, double left_years, int iDate);

public:
	//2009-02-23
	//设置债券类型
	void SetBondType(int value) { m_iBondType = value; }

	//2008-05-19
	//月份偏移
	int OffsetMonth(int date,int iMonth);

	//计算债券现金流[模拟]
	int GetCashFlow(BondNewInfo* pBondNewInfo,int date_lipd,double dInterest, float* pcash_flow);
	//创建现金流
	bool CreateCashFlow(DataFileNormal<blk_TxExFile_FileHead,BondCashFlowData>* pCashFlow,BondNewInfo* pBondNewInfo,DataFileNormal<blk_TxExFile_FileHead,NEW_BOND_INTEREST>* pNewBondInterest,int start_date,bool bIsNew);

	//首先计算
	bool CalcYTM(double& dYtm,double& dDuration,double& dMduration,double& dConvexity,int iSecurityId,int iDate,float fPrice=-1.0,int iBondCount=1);
	bool CalcNewBondYTM(
		double& dYtm,			//out
		double& dDuration,		//out
		double& dMduration,		//out
		double& dConvexity,		//out
		BondNewInfo* pBondNewInfo,//in,模拟债券的基本信息
		DataFileNormal<blk_TxExFile_FileHead,BondCashFlowData>* pCashFlow,//in,模拟债券的现金流
		int iDate,	//in,计算YTM的日期
		float fPrice,//in,计算YTM的债券价格[全价]
		int iBondCount=1//in
		);

	//然后调用下面3个接口取得计算结果
	bool	Calc(int iSecurityId,int iDate,float fPrice,int iBondCount=1);
	bool	Calc1(int iSecurityId,int iDate);
	//value = true表示只进行一次现金流的修正处理
	//value = false表示每次计算都要进行现金流的修正处理
	void	SetYtmFlag(bool value)
	{
		m_bCalcYTMoneDay = value;
	}
	double	Get_YTM(int iSecurityId,int iDate,double dPrice=-1.0,int iBondCount=1);
	bool	Get_MDURATION_CONVEXITY(int iSecurityId,int iDate,double& dDuration,double& dMduration,double& dConvexity,int iBondCount=1);

	double	Get_YTM(void){return m_dYTM;}
	double	Get_DURATION(void)	{ return m_dDURATION; }
	double	Get_MDURATION(void)	{ return m_dMDUR; }
	double	Get_CONVEXITY(void) { return m_dCONVEXITY; }
	float	Get_PriceByYTM(int iSecurityId,float fYtm,int iDate,int iBondCount=1);
	float	Get_PriceByYTM(BondNewInfo* pBondNewInfo,DataFileNormal<blk_TxExFile_FileHead,BondCashFlowData>* pCashFlow,float fYtm,int iDate,int iBondCount=1);

	//取得债券剩余期限[天数] NL/365
	int GetRemainsDays(int iStart,int iEnd);

	//计算转债底价
	double CalcFloor(int iSecurityId,int iDate,bool bIsNetPrice=false,double left_years=7);
	//计算转债平价
	double CalcParity(int iSecurityId,int iDate=0);
	//计算可转债-转换溢价率
	double CalcPremium(int iSecurityId,int iDate=0);
	//溢价率公式
	static double CalcPremium(
		double dClosePrice,	//收盘价
		double dPrice		//=平价，则计算结果为平价溢价率；=底价，则计算结果为底价溢价率；
		);
	//==========================================================================================//by zhangxs 20080806
	//可转债权证取得正股价格
	double GetBondStockPrice(int iSecurityId,int iDate=0);
	//取得正股涨幅
	double GetBondStockRaiseRatio(int iSecurityId,int iDate=0);
	//==========================================================================================

	//取得转债底价
	double GetFloor(int iSecurityId);

public:
	//付息频率 30001125 int
	int	GetPayInterstFrequency(int iSecurityId);
	int	GetPayInterstFrequency(SecurityQuotation* pSecurity);
	//2008-01-04
	//付息频率 30001125 int
	CString GetPayInterstFrequencyString(int iSecurityId);
	CString GetPayInterstFrequencyString(SecurityQuotation* pSecurity);

	//2008-05-15
	//取得债券付息方式int=记息方式1:单利,2:复利,3:浮动利率,4:累进利率,5:贴现
	CString GetBondInterestType(int iSecurityId);
	CString GetBondInterestType(SecurityQuotation* pSecurity);

	//计算天数计算方法 30001126 int
	//1:30/360,2:ACT/365,3:ACT/ACT,4:ACT/360,5:NL/365,6:30/365
	int GetFigureMethod(int iSecurityId);
	int GetFigureMethod(SecurityQuotation* pSecurity);

	//到期日期	30001209
	//到期日期 30001128 int
	int GetEndDate(int iSecurityId);
	int GetEndDate(SecurityQuotation* pSecurity);

	//发行规模	30001204
	double	GetShare(int iSecurityId);
	double	GetShare(SecurityQuotation* pSecurity);
	//票面利率	30001205
	double GetParRate(int iSecurityId);
	double GetParRate(SecurityQuotation* pSecurity);
	//30001125		付息频率
	//发行年限	30001206
	double	GetHoldYear(int iSecurityId);
	double	GetHoldYear(SecurityQuotation* pSecurity);
	//本期起息日期	30001208
	int	GetBeginDate(int iSecurityId);
	int	GetBeginDate(SecurityQuotation* pSecurity);

	//起息日期=第一笔现金流的起息日期
	int	GetFirstBeginDate(int iSecurityId);
	int	GetFirstBeginDate(SecurityQuotation* pSecurity);

	//剩余期限
	double	GetRemnants(int iSecurityId,int iDate);
	double	GetRemnants(SecurityQuotation* pSecurity,int iDate);
	int	GetRemnantsDays(int iSecurityId,int iDate);
	int	GetRemnantsDays(SecurityQuotation* pSecurity,int iDate);
	//将日期转换为字符串
	CString GetRemnants(int days);
	//与老系统保持一致
	CString GetRemnantsString(int iStart,int iEnd);

	//计息天数=
	int	GetInterestDays(int iSecurityId,int iDate);
	int	GetInterestDays(SecurityQuotation* pSecurity,int iDate);
	//用于新债模拟测算
	int	GetInterestDays(DataFileNormal<blk_TxExFile_FileHead,BondCashFlowData>* pCashFlow,int iDate);

	//应计利息
	double	GetInterest(int iSecurityId,int iDate);
	double	GetInterest(SecurityQuotation* pSecurity,int iDate);
	//应计利息_新  2012-7-3
	//bFlag   true--"日间"数据  ， false--"日终"数据
	double GetInterest_New(int iSecurityId,int iDate,bool bFlag=true);
	double GetInterest_New(SecurityQuotation* pSecurity,int iDate,bool bFlag);
	//用于新债模拟测算
	double	GetInterest(BondNewInfo* pBondNewInfo,DataFileNormal<blk_TxExFile_FileHead,BondCashFlowData>* pCashFlow,int iDate);
	BondCashFlowData*	GetBondCashFlowDataByDate(DataFileNormal<blk_TxExFile_FileHead,BondCashFlowData>* pCashFlow,int date);

	//30001207		付息方式
	int	GetInterestType(int iSecurityId);
	int	GetInterestType(SecurityQuotation* pSecurity);
	//30001210		上市所在的交易所 
	//30001211		上市日期

	//30001115		债券简称
	//30001116		债券全称
	//30001117		所属国家
	//30001118		所用货币ID
	//30001119		发行人ID
	//30001120		发行人类型
	//30001121		债券类型
	//30001122		债券形式
	//30001123		记息方式
	//30001139      债券对应股票代码
	//30001140		股票交易实体
	int GetMarkManner(int iSecurityId);
	int	GetMarkManner(SecurityQuotation* pSecurity);
	//债券类型
	CString GetBondType(int iSecurityId);
	CString GetBondType(SecurityQuotation* pSecurity);

	//债券形式
	CString GetBondForm(int iSecurityId);
	CString GetBondForm(SecurityQuotation* pSecurity);

	//债券记息方式
	CString GetInterestTypeName(int iSecurityId);
	CString GetInterestTypeName(SecurityQuotation* pSecurity);
	//30001124		基准利率
	//30001126		计算天数计算方法
	//30001127		起始日期
	//30001128		到期日期
	//30001129		早赎兑付日期
	//30001130		早赎兑付价格
	//30001131		利差
	//30001132		债券担保人
	//30001133		担保方式
	//30001134		天相类型
	//30001187		债券面值
	//30001188		债券期限
	//30001189		基准利率类别

	//板块=排序分析
	////baTable已经填充了样本[交易实体]
	bool GetBlockAnalysis(Table_Display& baTable,std::vector<int>& arrSamples,int& iSortCol,int iMarketId=0,bool bNeedProgressWnd=false,int nCollID=-1);
	//bool GetBlockAnalysis(Table_Display& baTable,std::vector<int>& arrSamples,bool bNeedProgressWnd=false);
	//排序分析数据结果标题列
	bool GetBlockAnalysisCol(Table_Display& baTable,std::vector<int>& arrSamples,int& iSortCol);
	//排序分析数据结果设置
	bool SetBlockAnalysisCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow);
	//排序分析基本数据结果设置[换手率]
	bool SetBlockAnalysisHslCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow);

	//2008-01-28
	//交易提示
	//flag==0 起息日期
	//flag==1 付息日期
	//flag==2 除息日期
	//flag==3 登记日期
	bool GetTradeInfo(std::vector<int> iSecurityId,Table_Display& baTable,int iStartDate,int iEndDate,bool bAllDate=false,int flag=3);
	//added by zhangxs 20081216
	//交易提示 --供派息统计用
	//flag==0 起息日期
	//flag==1 付息日期
	//flag==2 除息日期
	//flag==3 登记日期
	bool TxBond::GetTradeInfoDivideBonus(std::vector<int> iSecurityId,
		Table_Display& baTable,
		int iStartDate,
		int iEndDate,
		bool bAllDate,
		int flag);

public:
//	bool TxBondPayInterest(Tx::Core::Table_Indicator& resTable,std::vector<int> iSecurityId,int iStartDate,int iEndDate,bool bAllDate);
//	bool TxBondSectionMarket(Tx::Core::Table_Indicator& AccountTable,std::vector<int> iSecurityId,int iStartDate,int iEndDate);
	//	The iMarketdate is deleted by lijw 2008-09-03 由于在统计里没有用到该参数，所以把这个参数去掉。
	bool TxBondIssue(Tx::Core::Table_Indicator& resTable,std::vector<int> iSecurityId,int iStartDate,int iEndDate,bool bAllDate);
	bool IdColToNameAndCode(Tx::Core::Table_Indicator &resTable,int iCol,int iIdType=0,int iMethodType=0);
	//债券的阶段行情[涨幅][高级]
	bool TxBondCycleRateAdv(
		std::set<int>& iSecurityId,			//交易实体ID
		int startDate,						//起始日期
		int endDate,						//终止日期
		bool bCutFirstDateRaise,			//剔除首日涨幅
		int	iFQLX,							//复权类型 0-不复权;1=后复权
		Tx::Core::Table_Indicator& resTable,//结果数据表
		int iFlag=0							//计算类型0-默认；1-债券[bCutFirstDateRaise=true表示全价模式,bCutFirstDateRaise=false表示净价模式]
		);

//债券 专项统计部分
protected:
	virtual double GetBondInterest(SecurityQuotation* pSecurity,int iDate);

public:
	//[新]债券YTM、久期、修正久期、凸性    2013-09-02
    void GetYTM(SecurityQuotation* pSecurity,int iDate,double& dYtm,double& dDuration,double& dMduration,double& dConvexity);
	void GetYTM(SecurityQuotation* pSecurity,double& dYtm,double& dDuration,double& dMduration,double& dConvexity);
	bool GetYTM(SecurityQuotation* pSecurity,int iDate,float fPrice, int nPriceType, double& dYtm,double& dDuration,double& dMduration,double& dConvexity);
	virtual void GetBondYTM(std::vector<int> nSecurityIds);
	virtual void GetBondYTM(int nSecurityId,double &dYtm,double &dMdur,double &dCon);

	float GetPriceByYtmNew(SecurityQuotation* pSecurity,int iDate,double dYtm);
public:
	std::unordered_map<int,BondYTM> m_BondYTMMap;
};


	}
}

#endif
