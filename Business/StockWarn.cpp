/**********************************************************
  Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
  File name:	StockWarn.cpp
  Author:		王志勇
  Version:		1.0
  Date:			2007-08-08
  Description:	这个类用来表示预警股票的各个设置,并且实现对象串行化

***********************************************************/

#include "stdafx.h"
#include "StockWarn.h"
namespace Tx
{
	namespace Business
	{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CStockWarn::CStockWarn()
{
			//预警时间
			m_time.tm_wday	= 0;
			m_time.tm_year	= 0;
			m_time.tm_mon	= 0;
			m_time.tm_mday	= 0;
			m_time.tm_hour	= 0;
			m_time.tm_min	= 0;
			m_time.tm_yday	= 0;
			m_time.tm_isdst = 0;
			m_time.tm_sec	= 0;
			
			m_StockID		=0;			//券种ID
			m_HighPrice		=0;			//最高价
			m_LowPrice		=0;			//最低价
			m_ChangeExtent	=0;			//涨跌幅
			m_TurnOver		=0;			//成交量
			m_TurnOverRate	=0;			//换手率
			m_TurnOverVol	=0;			//成交金额
			m_TurnOverAmount=0;			//成交笔数
			m_Date			=0;				//日期
			m_flag			=false;			//标志是否更改
											//false表示没有更改
			m_delete_flag	=false;			//是否删除
											//false 表示没有删除
			m_check_flag	=false;			//是否查看
											//false表示没有查看
			

}

CStockWarn::~CStockWarn()
{

}
}
}