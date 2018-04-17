/**********************************************************
  Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
  File name:	ComputingWarrant.h
  Author:		王志勇
  Version:		1.0
  Date:			2007-11-13
  Description:	权证测算工具的算法实现

***********************************************************/
#include "TxBusiness.h"
#include "..\..\Data\SecurityTradeDate.h"
#include "..\..\Data\HisTrade.h"
#include "..\..\core\driver\Table_Indicator.h"
#include "..\..\Core\TxMath\CWarrantMath.h"
#include "..\..\Core\Zkklibrary\Datetime.h"
#include "..\..\Core\Core\ProgressWnd.h"
#include "TxWarrant.h"
#include <set>
using namespace Tx::Data;
using namespace Tx::Core;

namespace Tx
{
	namespace Business
	{
		class	BUSINESS_EXT ComputingWarrant
		{
		public:
			ComputingWarrant();
			~ComputingWarrant();
		public:
			//-------------------------------------------------
			//这里利用了B_S模型，已知了波动率以后反向求的理论价格
			//****************************************************
			//取得权证某个日期的理论价格
			double		GetTheoryPrice( bool	bFlag,		//认购还是认沽true认购
										int		nStyle,		//权证类型0欧式,1美式,2百慕大
										double	dNoInterest,//无风险利率
										double	dSClose,	//当前标的证券价格
										double	dExPrice,	//行权价格 
										double	dExRatio,	//行权比例
										double	dDelta,		//标的股票年股利收益率
										double	dLasted,	//剩余年限
										double	dSigma		//股票连续复利年收益率标准差
										);
			//---------------------------------------------------------------
			//这里计算当前到权证到期日的年限
			//***************************************************************
			//取得权证的某日的剩余年限
			double		GetLastedPeriod( int nID, int nDate );
			//取得隐含波动率
			double		GetSigma(	bool	bCall_Put,		//认购认沽	true认购
									double	dStockPrice,	//标的证券价格
									double	dExPrice,		//行权价格
									double	dNoRiskInterest,//无风险利率
									double	dDelta,		
									double	dLastedYear,	//剩余期限（年）
									double	dMarketValue	//期权市场价值
									);
			//---------------------------------------------------------------
			//认购权证溢价率=（行权价+认购权证价格/行权比例-正股价）/正股价 
			//认沽权证溢价率=（正股价+认沽权证价格/行权比例-行权价）/正股价
			//投资者需要变化多少百分比，才可以实现平本
			//*******************************************************************
			//取得溢价率
			double		GetPremiumRate( double dWPrice,		//权证价格
										double dSPrice,		//标的股票价格
										double dExPrice,	//行权价
										bool	bFlag,		//认购为true,反之为false
										double	dRatio		//行权比例
										);
			//--------------------------------------------------------------------
			//杠杆比率＝标的证券价格÷(权证价格×行权比例)
			//*******************************************************************
			//取得杠杆比率
			double		GetGearRate(	double	dSClose,	//相关资产的收盘价
										double	dWClose,	//权证的收盘价
										double	dExRatio	//行权比例
										);
			//---------------------------------------------------------------------
			//权证涨（跌）幅价格＝权证前一日收盘价格±（标的证券当日涨幅价格－标的证券前一日收盘价）×125%×行权比例
			//**********************************************************************
			//取得涨停价
			double		GetRaiseLimit(	double	dWPrice,	//权证前一日收盘价
										//double	dSOpen,		//标的证券当日开盘价格
										double	dSPreClose,	//标的证券前一日收盘价
										double	dRatio		//行权比例
										);
										
			//取得跌停价
			double		GetDropLimit(	double	dWPrice,	//权证前一日收盘价
										//double	dSOpen,		//标的证券当日开盘价格
										double	dSPreClose,	//标的证券前一日收盘价
										double	dRatio		//行权比例
										);
			//取得股票的指定日期前一年的波动率,
			double		GetStockStd( int nID, int nEndDate, int nCycle );
			
			//计算权证测算工具中的各项指标，结果放入到resTable中，当bInterest为true时,dInterest有效,bStd为true,dStd有效,否则nCycle有效
			bool		bCalWarrant( Table &resTable,		//结果表
									std::set< int > nID,	//权证ID集合
									int	nStartDate,			//开始时间
									int nEndDate,			//截止时间
									bool bInterest,			//利率类型			true为指定，false为一年期利率
									double	dInterest,		//利率值
									bool	bStd,			//股票波动率类型	true为指定，false为计算
									double	dStd,			//股票波动率值
									int		nCycle			//日期类型			0表示日，1表示周，2表示月
									);
			bool		bGetTradeDaySequence(	int	iSecurityId,	//交易实体ID
												int iEnd,			//截止日期
												int iStart,			//开始日期
												int iCycle,			//周期
												std::vector< int > &iDates		//结果集
												);
			bool		bGetHQByCycle(	int nSecurityId,	//交易实体Id 
										int nStartDate,		//开始日期
										int nEndDate,		//截止日期
										int	nCycle,			//周期
										std::vector< HisTradeData >	&resVec	//结果集合
										);
			//取下现在周期的截止日期		
			int			GetNextCycleDay( int nDate,			//开始日期
										int nCycle			//周期	1 week; 2 month; 3 year
										);
			

		};

	}
}