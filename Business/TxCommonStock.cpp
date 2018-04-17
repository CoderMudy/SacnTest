/**************************************************************
	Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
	File name:		TxCommonStock.cpp
	Author:			赵宏俊
	Date:			2007-07-09
	Version:		1.0
	Description:	处理普通股票业务
***************************************************************/
#include "StdAfx.h"
#include "TxCommonStock.h"
namespace Tx
{
	namespace Business
	{

TxCommonStock::TxCommonStock(void)
{
	//50010018(普通股票)
	m_iSecurityTypeId = 50010018;
}

TxCommonStock::~TxCommonStock(void)
{
}
	}
}