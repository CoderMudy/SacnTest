/**************************************************************
	Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
	File name:		TxCommonStock.h
	Author:			�Ժ꿡
	Date:			2007-07-09
	Version:		1.0
	Description:	������ͨ��Ʊҵ��
***************************************************************/
#ifndef __TXCOMMONSTOCK_H__
#define __TXCOMMONSTOCK_H__
#include "TxStock.h"


#pragma once
namespace Tx
{
	namespace Business
	{
//��ͨ��Ʊҵ����
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