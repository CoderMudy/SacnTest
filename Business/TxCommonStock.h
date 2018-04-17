/**************************************************************
	Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
	File name:		TxCommonStock.h
	Author:			赵宏俊
	Date:			2007-07-09
	Version:		1.0
	Description:	处理普通股票业务
***************************************************************/
#ifndef __TXCOMMONSTOCK_H__
#define __TXCOMMONSTOCK_H__
#include "TxStock.h"


#pragma once
namespace Tx
{
	namespace Business
	{
//普通股票业务类
class TxCommonStock :
	public TxStock
{
public:
	TxCommonStock(void);
public:
	virtual ~TxCommonStock(void);
};
	}
}

#endif