/**************************************************************
	Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
	File name:		TxPreferredStock.h
	Author:			�Ժ꿡
	Date:			2007-07-09
	Version:		1.0
	Description:	���������Ʊҵ��
***************************************************************/
#ifndef __TXPREFERREDSTOCK_H__
#define __TXPREFERREDSTOCK_H__
#include "TxStock.h"

#pragma once

namespace Tx
{
	//�����Ʊҵ����
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