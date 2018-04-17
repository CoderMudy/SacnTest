/**************************************************************
	Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
	File name:		TxPreferredStock.cpp
	Author:			赵宏俊
	Date:			2007-07-09
	Version:		1.0
	Description:	处理特殊股票业务
***************************************************************/
#include "StdAfx.h"
#include "TxPreferredStock.h"
namespace Tx
{
	namespace Business
	{

TxPreferredStock::TxPreferredStock(void)
{
	//40000003(特别股)
	m_iSecurityTypeId = 40000003;
}

TxPreferredStock::~TxPreferredStock(void)
{
}
	}
}