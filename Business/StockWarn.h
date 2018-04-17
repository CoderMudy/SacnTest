/**********************************************************
  Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
  File name:	StockWarn.h
  Author:		王志勇
  Version:		1.0
  Date:			2007-08-08
  Description:	这个类用来表示预警股票的各个设置,并且实现对象串行化

***********************************************************/

#if !defined(AFX_STOCKWARN_H__09C4DD23_FEF9_4577_8E22_F05D084E2DC3__INCLUDED_)
#define AFX_STOCKWARN_H__09C4DD23_FEF9_4577_8E22_F05D084E2DC3__INCLUDED_
#include "TxBusiness.h"
#include "time.h"

#include "time.h"

namespace Tx
{
	namespace Business
	{

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

		struct  WarnCfgInfo
		{
			int		nInterval;	//预警间隔
			int		nLast;		//停留时间
			int		nSiren;		//声音预警
			void Serialize(CArchive& ar)		//实现串行化
			{
				if (ar.IsLoading())
				{
					ar >> nInterval;			
					ar >> nLast;		
					ar >> nSiren;			
				}
				else
				{
					ar << nInterval;			
					ar << nLast;		
					ar << nSiren;	
				}
			}
		};
		
		class BUSINESS_EXT CStockWarn  
		{
		public:
			CStockWarn();	
			virtual ~CStockWarn();
		public:
			int			m_StockID;				//券种ID
			double		m_HighPrice;			//最高价
			double		m_LowPrice;				//最低价
			double		m_ChangeExtent;			//涨跌幅
			double		m_TurnOver;				//成交量
			double		m_TurnOverRate;			//换手率
			double		m_TurnOverVol;			//成交金额
			double		m_TurnOverAmount;		//成交笔数
			int			m_Date;					//日期
			bool		m_flag;					//标志是否更改
			bool		m_delete_flag;			//是否删除
			bool		m_check_flag;			//是否查看
			//-------20080324-----------------------------------
			bool		m_bSiren;				//是否警报声音
			bool		m_bStop;				//停止预警
			int			m_nInterval;			//预警间隔，一个单位为20秒
			int			m_nLast;				//窗口停留时间
			//=====================================================
			tm			m_time;					//预警时间


			void Serialize(CArchive& ar)		//实现串行化
			{
				if (ar.IsLoading())
				{
					ar >> m_StockID;			
					ar >> m_HighPrice;		
					ar >> m_LowPrice;			
					ar >> m_ChangeExtent;		
					ar >> m_TurnOver;			
					ar >> m_TurnOverRate;		
					ar >> m_TurnOverVol;		
					ar >> m_TurnOverAmount;	
					ar >> m_Date;				
					ar >> m_flag;				
					ar >> m_delete_flag;		
					ar >> m_check_flag;
					ar >> m_bSiren;	
					ar >> m_bStop;
					ar >> m_nInterval;
					ar >> m_nLast;
					ar >> m_time.tm_wday;
					ar >> m_time.tm_year;
					ar >> m_time.tm_mon;
					ar >> m_time.tm_mday;
					ar >> m_time.tm_hour;
					ar >> m_time.tm_min;
					ar >> m_time.tm_isdst;
					ar >> m_time.tm_sec;
					ar >> m_time.tm_yday;
					ar >> m_bSiren;	
					ar >> m_bStop;
					ar >> m_nInterval;
					ar >> m_nLast;
					
				}
				else
				{
					ar << m_StockID;			
					ar << m_HighPrice;		
					ar << m_LowPrice;			
					ar << m_ChangeExtent;		
					ar << m_TurnOver;			
					ar << m_TurnOverRate;		
					ar << m_TurnOverVol;		
					ar << m_TurnOverAmount;	
					ar << m_Date;				
					ar << m_flag;				
					ar << m_delete_flag;		
					ar << m_check_flag;		
					ar << m_bSiren;	
					ar << m_bStop;
					ar << m_nInterval;
					ar << m_nLast;
					ar << m_time.tm_wday;
					ar << m_time.tm_year;
					ar << m_time.tm_mon;
					ar << m_time.tm_mday;
					ar << m_time.tm_hour;
					ar << m_time.tm_min;
					ar << m_time.tm_isdst;
					ar << m_time.tm_sec;
					ar << m_time.tm_yday;
					ar << m_bSiren;	
					ar << m_bStop;
					ar << m_nInterval;
					ar << m_nLast;

				}
			}	
		};
}
}
#endif // !defined(AFX_STOCKWARN_H__09C4DD23_FEF9_4577_8E22_F05D084E2DC3__INCLUDED_)

