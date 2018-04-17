/**************************************************************
	Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
	File name:		TxBusinessStat.cpp
	Author:			赵宏俊
	Date:			2007-07-03
	Version:		1.0
	Description:	
					集中处理各类统计业务
***************************************************************/
#include "StdAfx.h"
#include "TxBusiness.h"
#include "..\..\Core\Driver\TxFileUpdateEngine.h"

namespace Tx
{
	namespace Business
	{

//统计准备=添加样本
bool TxBusiness::AddSampleBeforeStat(
	int&	iCol,
	int		iStartDate,
	int		iEndDate,
	std::set<int>& iSecurity
	)
{
	//默认的返回值状态
	bool result = false;

	//step1-SetSecurityIntoTable
	//将用户选择的交易实体代码集合作为参数设置到table
	//分步操作1.1-设置交易实体col=0
	result = m_pLogicalBusiness->SetSecurityIntoTable(
		iCol,		//参数列索引值
		iSecurity,	//交易实体集合
		m_txTable	//计算需要的参数传输载体以及计算后结果的载体
		);
	if(result==false)
		return false;
	//同时添加机构和券的id
	//1.2;col=1,col=2
	result = m_pLogicalBusiness->SetSecurityIntoTable(iCol,m_txTable);
	if(result==false)
		return false;

	//step2-SetVariantIntoTable
	//设置单参数列,允许多个参数
	int count = 1;	//参数个数
	//起始日期col=3
	result = m_pLogicalBusiness->SetVariantIntoTable(
		iCol,									//参数列索引值
		&(Tx::Core::VariantData(iStartDate)),	//VariantData参数
		count,									//参数个数
		m_txTable	//计算需要的参数传输载体以及计算后结果的载体
		);
	if(result==false)
		return false;
	////终止日期col=4
	//result = m_pLogicalBusiness->SetVariantIntoTable(
	//	iCol,								//参数列索引值
	//	&(Tx::Core::VariantData(iEndDate)),	//VariantData参数
	//	count,								//参数个数
	//	m_txTable	//计算需要的参数传输载体以及计算后结果的载体
	//	);
	//if(result==false)
	//	return false;
	return true;
}

//统计准备=添加指标
bool TxBusiness::AddIndicatorBeforeStat(
	int		iIndicator,
	int		iStartDateIndex,
	int		iEndDateIndex
	)
{
	//step3-SetIndicatorIntoTable
	//根据已经设置的参数列，配置指标参数，根据指标信息表，设置返回数据类型
	//允许多个指标
	//int iIndicator = 30300014;	//指标=流通股
	UINT varCfg[3];			//参数配置
	int varCount=2;			//参数个数

	GetIndicatorDataNow(iIndicator);
	if(m_pIndicatorData==NULL)
		return false;

	//机构id
	//varCfg[0]=m_pIndicatorData->Belong2Entity;
	varCfg[0]=0;
	//
	varCfg[1]=iStartDateIndex;
	//varCfg[2]=iEndDateIndex;
	bool result = m_pLogicalBusiness->SetIndicatorIntoTable(
		m_pIndicatorData,	//指标
		varCfg,				//参数配置
		varCount,			//参数个数
		m_txTable	//计算需要的参数传输载体以及计算后结果的载体
		);
	if(result==false)
		return false;
	return true;
}
	}
}