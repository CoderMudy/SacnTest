/**************************************************************
	Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
	File name:		TxPreferredStock.h
	Author:			赵宏俊
	Date:			2007-07-09
	Version:		1.0
	Description:	处理特殊股票业务
***************************************************************/
#ifndef __TXPREFERREDSTOCK_H__
#define __TXPREFERREDSTOCK_H__
#include "TxStock.h"

#pragma once

namespace Tx
{
	//特殊股票业务类
	namespace Business
	{
class TxPreferredStock :
	public TxStock
{
public:
	TxPreferredStock(void);
public:
	virtual ~TxPreferredStock(void);
};
	}
}
#endif