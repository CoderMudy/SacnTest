/**************************************************************
	Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
	File name:		TxStock.cpp
	Author:			赵宏俊
	Date:			2007-07-09
	Version:		1.0
	Description:	处理股票类业务
					某些处理方法可以直接使用TxBusiness提供的方法
***************************************************************/
#include "StdAfx.h"
#include "TxStock.h"
#include "..\..\core\core\SystemPath.h"
#include "..\..\core\core\Commonality.h"
#include "..\..\core\core\userinfo.h"
#include "..\..\core\driver\TxFileUpdateEngine.h"
#include "MyIndicator.h"
#include "../../Core/Driver/ClientFileEngine/base/HttpDnAndUp.h"
#include "../../Core/Driver/ClientFileEngine/base/zip/ZipWrapper.h"
#include "..\..\Data\HotBlockInfo.h"

#ifdef _DEBUG
#include "..\..\core\driver\structs_updatefile.h"
#endif

namespace Tx
{
	namespace Business
	{
		IMPLEMENT_DYNCREATE(TxStock,TxBusiness)
			TxStock::TxStock(void)
		{
			//40000001(股票)
			m_iSecurityTypeId = 40000001;
			//2007-10-29
			//暂时停止该类型的判断
			m_iSecurityTypeId = 0;
			m_pEvaluateYear = NULL;
			m_pEvaluateSecurity = NULL;
			m_pForcastYear = NULL;
			m_pForcastSecurity = NULL;

			m_pEvaluateYearData = NULL;
			m_pEvaluateSecurityData = NULL;
			m_pEvaluateSecurityData2 = NULL;
			m_pForcastYearData = NULL;
			m_pForcastSecurityData = NULL;
			
			CTime tm = CTime::GetCurrentTime();
			m_iYearPe = tm.GetYear()%100;
		}

		TxStock::~TxStock(void)
		{
			if(m_pEvaluateYear)
			{
				delete m_pEvaluateYear;
				m_pEvaluateYear = NULL;
			}
			if(m_pEvaluateSecurity)
			{
				delete m_pEvaluateSecurity;
				m_pEvaluateSecurity = NULL;
			}
			if(m_pForcastYear)
			{
				delete m_pForcastYear;
				m_pForcastYear = NULL;
			}
			if(m_pForcastSecurity)
			{
				delete m_pForcastSecurity;
				m_pForcastSecurity = NULL;
			}		
		}

		template<class T1,class T2>
		void ReDefColType(Tx::Core::Table_Indicator& table,int col,Tx::Core::Data_Type dtype,T1 sbuf,T2 dbuf)
		{
			UINT tlen=table.GetColCount();
			UINT rlen=table.GetRowCount();
			if (tlen<=0||col<0||col>=tlen||rlen<=0)
				return;
			table.AddCol(table.GetColType(col));
			table.CopyColumnData(table,col,table.GetColCount()-1);
			table.DeleteCol(col);
			table.InsertCol(col,dtype);
			UINT ncol=table.GetColCount()-1;
			UINT c=table.GetRowCount();
			for (int i=0;i<(int)c;i++)
			{
				table.GetCell(ncol,i,sbuf);
				dbuf=(T2)sbuf;
				table.SetCell(col,i,dbuf);
			}
			table.DeleteCol(ncol);
		}	

		void ReDefColTypeDoubleToInt(Tx::Core::Table_Indicator& table,int col,bool bGbit=true)
		{
			UINT tlen=table.GetColCount();
			UINT rlen=table.GetRowCount();
			if ((int)tlen<=0||col<0||col>=(int)tlen||(int)rlen<=0)
				return;
			double sbuf;
			int	dbuf;
			table.AddCol(table.GetColType(col));
			table.CopyColumnData(table,col,table.GetColCount()-1);
			table.DeleteCol(col);
			table.InsertCol(col,Tx::Core::dtype_int4);
			UINT ncol=table.GetColCount()-1;
			UINT c=table.GetRowCount();
			for (int i=0;i<(int)c;i++)
			{
				table.GetCell(ncol,i,sbuf);
				if (bGbit)
				{
					dbuf=(int)(sbuf+0.5);
				}
				table.SetCell(col,i,dbuf);
			}
			table.DeleteCol(ncol);
		}
		void ReDefColTypeDecmalToInt(Tx::Core::Table_Indicator& table,int col,bool bGbit=true)
		{
			UINT tlen=table.GetColCount();
			UINT rlen=table.GetRowCount();
			if ((int)tlen<=0||col<0||col>=(int)tlen||(int)rlen<=0)
				return;
			double sbuf;
			int	dbuf;
			table.AddCol(Tx::Core::dtype_decimal);
			table.CopyColumnData(table,col,table.GetColCount()-1);
			table.DeleteCol(col);
			table.InsertCol(col,Tx::Core::dtype_int4);
			UINT ncol=table.GetColCount()-1;
			UINT c=table.GetRowCount();
			for (int i=0;i<(int)c;i++)
			{
				table.GetCell(ncol,i,sbuf);
				if (sbuf==Tx::Core::Con_doubleInvalid)
				{
					dbuf=Tx::Core::Con_intInvalid;
				}
				else if (bGbit)
				{
					dbuf=(int)(sbuf+0.5);
				}
				table.SetCell(col,i,dbuf);
			}
			table.DeleteCol(ncol);
		}

//股本变化统计
//对于首发、增发、配股和送股的条件，可以在已经取得的结果中筛选，避免反复频繁调用底层，减少文件io
//附加指标
//如果需要附加其他指标,且该指标已经缓存,可以在输出的时候直接显示
//如果该指标未缓存,不建议附加
bool TxStock::GetAllVariantShareData(void)
{
	//默认的返回值状态
	bool result = false;
	//清空数据
	m_txTable.Clear();
	int iCol = 0;

	//准备样本集和日期参数
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);

	//if(AddSampleBeforeStat(iCol,iStartDate,iEndDate,iSecurity)==false)
	//	return false;

	int iIndicator = 30300014;	//指标=流通股
	UINT varCfg[2];			//参数配置
	int varCount=2;			//参数个数
	GetIndicatorDataNow(iIndicator);
	if(m_pIndicatorData==NULL)
		return false;
	varCfg[0]=0;
	varCfg[1]=1;
	result = m_pLogicalBusiness->SetIndicatorIntoTable(
		m_pIndicatorData,	//指标
		varCfg,				//参数配置
		varCount,			//参数个数
		m_txTable	//计算需要的参数传输载体以及计算后结果的载体
		);
	if(result==false)
		return false;

	iIndicator = 30300019;	//指标=总股本
	GetIndicatorDataNow(iIndicator);
	if(m_pIndicatorData==NULL)
		return false;
	varCfg[0]=0;
	varCfg[1]=1;
	result = m_pLogicalBusiness->SetIndicatorIntoTable(
		m_pIndicatorData,	//指标
		varCfg,				//参数配置
		varCount,			//参数个数
		m_txTable	//计算需要的参数传输载体以及计算后结果的载体
		);
	if(result==false)
		return false;

	//股本变动原因指标尚未定义
	//iIndicator = 30300019;	//指标=股本变动原因
	//GetIndicatorDataNow(iIndicator);
	//if(m_pIndicatorData==NULL)
	//	return false;
	//varCfg[0]=0;
	//varCfg[1]=1;
	//result = m_pLogicalBusiness->SetIndicatorIntoTable(
	//	m_pIndicatorData,	//指标
	//	varCfg,				//参数配置
	//	varCount,			//参数个数
	//	m_txTable	//计算需要的参数传输载体以及计算后结果的载体
	//	);
	//if(result==false)
	//	return false;

	//step4-GetData
	//根据之前3个步骤的设置进行数据读取，结果数据存放在table中
	result = m_pLogicalBusiness->GetData(m_txTable,true);
	if(result==false)
		return false;

	//double iTest=0;
	//m_txTable.GetCell(2,0,iTest);
	////between
	//Tx::Core::Table_Indicator resTable;
	//resTable.AddParameterColumn(Tx::Core::dtype_int4);
	//resTable.AddParameterColumn(Tx::Core::dtype_int4);
	//resTable.AddParameterColumn(Tx::Core::dtype_double);
	//resTable.AddParameterColumn(Tx::Core::dtype_double);
	//UINT nColArray[4];
	//nColArray[0]=0;
	//nColArray[1]=1;
	//nColArray[2]=2;
	//nColArray[3]=3;
	//m_txTable.Between(
	//	resTable,		//结果表(仅添加了列，行数必须为0)
	//	nColArray,		//结果列数组首址
	//	4,				//结果列个数
	//	1,				//源表中的条件列序号
	//	iStartDate,		//区间下限
	//	iEndDate,		//区间上限
	//	true,			//是否小于等于上限
	//	true			//是否大于等于下限
	//	);
			
	//注：table既是参数传递的载体，又是存放结果数据的载体
	return true;
}

bool TxStock::StatVariantShare(
	bool	bReloadAllData,			//是否重新加载全部数据的标志
	int		iStartDate,				//起始日期,-1表示忽略起始日期
	int		iEndDate,				//终止日期,-1表示忽略终止日期,两个-1表示全部日期
	std::set<int>& iInstitutionId,//交易实体样本
	Tx::Core::Table_Indicator& resTable	//统计结果数据表
	)
{
	//日期区间非法
	if(iStartDate>iEndDate)
		return false;

	//取得全部数据
	if(bReloadAllData==true)
		GetAllVariantShareData();

	//如果全部数据的记录数为0，则返回
	if(m_txTable.GetRowCount()<=0)
		return false;

	//double iTest=0;
	//m_txTable.GetCell(2,0,iTest);

	int nColCount = m_txTable.GetColCount();
	int theColIndex = 1;

	//取得指定时间区间内的记录数据
	UINT* nColArray = new UINT[nColCount];
	Tx::Core::Table_Indicator resTableDate;
	Tx::Core::Table_Indicator* ptable = &m_txTable;
	Tx::Core::Table_Indicator* ptable_return = &m_txTable;

	//复制所有列信息
	resTableDate.CopyColumnInfoFrom(m_txTable);

	for(int i=0;i<nColCount;i++)
		nColArray[i]=i;

	ptable->Between(
		resTableDate,	//结果表(仅添加了列，行数必须为0)
		nColArray,		//结果列数组首址
		nColCount,		//结果列个数
		theColIndex,	//源表中的条件列序号
		iStartDate,		//区间下限
		iEndDate,		//区间上限
		true,			//是否小于等于上限
		true			//是否大于等于下限
		);
	ptable = &resTableDate;
	ptable_return = &resTableDate;

	//取得指定交易实体的数据记录
	Tx::Core::Table_Indicator resTableId;
	if(iInstitutionId.size()>0)
	{
		theColIndex = 0;
		//复制所有列信息
		resTableId.CopyColumnInfoFrom(m_txTable);
		ptable->EqualsAt(
			resTableId,		//结果表(仅添加了列，行数必须为0)
			nColArray,		//结果列数组首址
			nColCount,		//结果列个数
			theColIndex,	//源表中的条件列序号
			iInstitutionId	//交易实体样本集
			);
		ptable_return = &resTableId;
	}

	resTable.Clone(*ptable_return);

	delete nColArray;
	
	return true;
}


//十大股东统计
//取到本地的数据，再进行股东名称,变动原因,持股数的筛选
bool TxStock::GetAllTenShareHoldersData(void)
{
	//默认的返回值状态
	bool result = false;
	//清空数据
	m_txTable.Clear();
	//清空数据
	m_txTable.Clear();
	int iCol = 0;

	//准备样本集和日期参数
	//样本ID
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
	//截止日期
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);

	int iIndicator = 0;
	UINT varCfg[2];			//参数配置
	int varCount=2;			//参数个数

	//1
	iIndicator = 30100019;	//指标=公告日期
	GetIndicatorDataNow(iIndicator);
	if(m_pIndicatorData==NULL)
		return false;
	varCfg[0]=0;
	varCfg[1]=1;
	result = m_pLogicalBusiness->SetIndicatorIntoTable(
		m_pIndicatorData,	//指标
		varCfg,				//参数配置
		varCount,			//参数个数
		m_txTable	//计算需要的参数传输载体以及计算后结果的载体
		);
	if(result==false)
		return false;

	//2变动原因
	iIndicator = 30100014;	//指标=变动原因
	GetIndicatorDataNow(iIndicator);
	if(m_pIndicatorData==NULL)
		return false;
	varCfg[0]=0;
	varCfg[1]=1;
	result = m_pLogicalBusiness->SetIndicatorIntoTable(
		m_pIndicatorData,	//指标
		varCfg,				//参数配置
		varCount,			//参数个数
		m_txTable	//计算需要的参数传输载体以及计算后结果的载体
		);
	if(result==false)
		return false;

	//3
	iIndicator = 30100011;	//指标=次序
	GetIndicatorDataNow(iIndicator);
	if(m_pIndicatorData==NULL)
		return false;
	varCfg[0]=0;
	varCfg[1]=1;
	result = m_pLogicalBusiness->SetIndicatorIntoTable(
		m_pIndicatorData,	//指标
		varCfg,				//参数配置
		varCount,			//参数个数
		m_txTable	//计算需要的参数传输载体以及计算后结果的载体
		);
	if(result==false)
		return false;

	//4
	iIndicator = 30100012;	//指标=股东名称
	GetIndicatorDataNow(iIndicator);
	if(m_pIndicatorData==NULL)
		return false;
	varCfg[0]=0;
	varCfg[1]=1;
	result = m_pLogicalBusiness->SetIndicatorIntoTable(
		m_pIndicatorData,	//指标
		varCfg,				//参数配置
		varCount,			//参数个数
		m_txTable	//计算需要的参数传输载体以及计算后结果的载体
		);
	if(result==false)
		return false;

	//5
	iIndicator = 30100013;	//指标=持股数
	GetIndicatorDataNow(iIndicator);
	if(m_pIndicatorData==NULL)
		return false;
	varCfg[0]=0;
	varCfg[1]=1;
	result = m_pLogicalBusiness->SetIndicatorIntoTable(
		m_pIndicatorData,	//指标
		varCfg,				//参数配置
		varCount,			//参数个数
		m_txTable	//计算需要的参数传输载体以及计算后结果的载体
		);
	if(result==false)
		return false;

	//6股东性质
	iIndicator = 30100022;	//指标=股东性质
	GetIndicatorDataNow(iIndicator);
	if(m_pIndicatorData==NULL)
		return false;
	varCfg[0]=0;
	varCfg[1]=1;
	result = m_pLogicalBusiness->SetIndicatorIntoTable(
		m_pIndicatorData,	//指标
		varCfg,				//参数配置
		varCount,			//参数个数
		m_txTable	//计算需要的参数传输载体以及计算后结果的载体
		);
	if(result==false)
		return false;

	//7占总股本%
	iIndicator = 30100023;	//指标=占总股本%
	GetIndicatorDataNow(iIndicator);
	if(m_pIndicatorData==NULL)
		return false;
	varCfg[0]=0;
	varCfg[1]=1;
	result = m_pLogicalBusiness->SetIndicatorIntoTable(
		m_pIndicatorData,	//指标
		varCfg,				//参数配置
		varCount,			//参数个数
		m_txTable	//计算需要的参数传输载体以及计算后结果的载体
		);
	if(result==false)
		return false;

	//8
	iIndicator = 30300019;	//指标=总股本
	GetIndicatorDataNow(iIndicator);
	if(m_pIndicatorData==NULL)
		return false;
	varCfg[0]=0;
	varCfg[1]=1;
	result = m_pLogicalBusiness->SetIndicatorIntoTable(
		m_pIndicatorData,	//指标
		varCfg,				//参数配置
		varCount,			//参数个数
		m_txTable	//计算需要的参数传输载体以及计算后结果的载体
		);
	if(result==false)
		return false;

	//9
	iIndicator = 30300014;	//指标=流通股
	GetIndicatorDataNow(iIndicator);
	if(m_pIndicatorData==NULL)
		return false;
	varCfg[0]=0;
	varCfg[1]=1;
	result = m_pLogicalBusiness->SetIndicatorIntoTable(
		m_pIndicatorData,	//指标
		varCfg,				//参数配置
		varCount,			//参数个数
		m_txTable	//计算需要的参数传输载体以及计算后结果的载体
		);
	if(result==false)
		return false;

	//step4-GetData
	//根据之前3个步骤的设置进行数据读取，结果数据存放在table中
	result = m_pLogicalBusiness->GetData(m_txTable,true);
	if(result==false)
		return false;

	
	//注：table既是参数传递的载体，又是存放结果数据的载体
	return true;
}
//十大[流通]股东统计
bool TxStock::StatTenShareHolders(
	bool	bReloadAllData,			//是否重新加载全部数据的标志
	int		iStartDate,				//起始日期,-1表示忽略起始日期
	int		iEndDate,				//终止日期,-1表示忽略终止日期,两个-1表示全部日期
	std::set<int>& iInstitutionId,//机构样本
	Tx::Core::Table_Indicator& resTable,//统计结果数据表
	bool	bTradeableShare,		//true统计十大流通股东;false统计十大股东
	bool	bAnnounceDate			//true按照公告日期检索,false按照截止日期检索
	)
{
	//日期区间非法
	if(iStartDate>iEndDate)
		return false;

	//取得全部数据
	if(bReloadAllData==true)
	{
		if(bTradeableShare==false)
			GetAllTenShareHoldersData();
		else
			GetAllTenTradeableShareHoldersData();
	}

	//如果全部数据的记录数为0，则返回
	if(m_txTable.GetRowCount()<=0)
		return false;

	//double iTest=0;
	//m_txTable.GetCell(2,0,iTest);

	int nColCount = m_txTable.GetColCount();
	int theColIndex = 1;
 
	if(bAnnounceDate==true)
		theColIndex = 2;

	//取得指定时间区间内的记录数据
	UINT* nColArray = new UINT[nColCount];
	Tx::Core::Table_Indicator resTableDate;
	Tx::Core::Table_Indicator* ptable = &m_txTable;
	Tx::Core::Table_Indicator* ptable_return = &m_txTable;

	//复制所有列信息
	resTableDate.CopyColumnInfoFrom(m_txTable);

	for(int i=0;i<nColCount;i++)
		nColArray[i]=i;

	ptable->Between(
		resTableDate,	//结果表(仅添加了列，行数必须为0)
		nColArray,		//结果列数组首址
		nColCount,		//结果列个数
		theColIndex,	//源表中的条件列序号
		iStartDate,		//区间下限
		iEndDate,		//区间上限
		true,			//是否小于等于上限
		true			//是否大于等于下限
		);
	ptable = &resTableDate;
	ptable_return = &resTableDate;

	//取得指定交易实体的数据记录
	Tx::Core::Table_Indicator resTableId;
	if(iInstitutionId.size()>0)
	{
		theColIndex = 0;
		//复制所有列信息
		resTableId.CopyColumnInfoFrom(m_txTable);
		ptable->EqualsAt(
			resTableId,		//结果表(仅添加了列，行数必须为0)
			nColArray,		//结果列数组首址
			nColCount,		//结果列个数
			theColIndex,	//源表中的条件列序号
			iInstitutionId	//交易实体样本集
			);
		ptable_return = &resTableId;
	}

	resTable.Clone(*ptable_return);

	delete nColArray;
	
	return true;
}

//十大流通股东统计
//从报告中得到数据，所以建议采用截止日期
//取到本地的数据，再进行股东名称,变动原因,持股数的筛选
bool TxStock::GetAllTenTradeableShareHoldersData(void)
{
	//默认的返回值状态
	bool result = false;
	//清空数据
	m_txTable.Clear();
	//清空数据
	m_txTable.Clear();
	int iCol = 0;

	//准备样本集和日期参数
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);

	int iIndicator = 30300014;	//指标=流通股
	UINT varCfg[2];			//参数配置
	int varCount=2;			//参数个数

	//1
	iIndicator = 30100025;	//指标=公告日期
	GetIndicatorDataNow(iIndicator);
	if(m_pIndicatorData==NULL)
		return false;
	varCfg[0]=0;
	varCfg[1]=1;
	result = m_pLogicalBusiness->SetIndicatorIntoTable(
		m_pIndicatorData,	//指标
		varCfg,				//参数配置
		varCount,			//参数个数
		m_txTable	//计算需要的参数传输载体以及计算后结果的载体
		);
	if(result==false)
		return false;

	//2变动原因
	iIndicator = 30100018;	//指标=流通股变动原因
	GetIndicatorDataNow(iIndicator);
	if(m_pIndicatorData==NULL)
		return false;
	varCfg[0]=0;
	varCfg[1]=1;
	result = m_pLogicalBusiness->SetIndicatorIntoTable(
		m_pIndicatorData,	//指标
		varCfg,				//参数配置
		varCount,			//参数个数
		m_txTable	//计算需要的参数传输载体以及计算后结果的载体
		);
	if(result==false)
		return false;

	//3
	iIndicator = 30100015;	//指标=流通股次序
	GetIndicatorDataNow(iIndicator);
	if(m_pIndicatorData==NULL)
		return false;
	varCfg[0]=0;
	varCfg[1]=1;
	result = m_pLogicalBusiness->SetIndicatorIntoTable(
		m_pIndicatorData,	//指标
		varCfg,				//参数配置
		varCount,			//参数个数
		m_txTable	//计算需要的参数传输载体以及计算后结果的载体
		);
	if(result==false)
		return false;

	//4
	iIndicator = 30100016;	//指标=流通股股东名称
	GetIndicatorDataNow(iIndicator);
	if(m_pIndicatorData==NULL)
		return false;
	varCfg[0]=0;
	varCfg[1]=1;
	result = m_pLogicalBusiness->SetIndicatorIntoTable(
		m_pIndicatorData,	//指标
		varCfg,				//参数配置
		varCount,			//参数个数
		m_txTable	//计算需要的参数传输载体以及计算后结果的载体
		);
	if(result==false)
		return false;

	//5
	iIndicator = 30100017;	//指标=流通股持股数
	GetIndicatorDataNow(iIndicator);
	if(m_pIndicatorData==NULL)
		return false;
	varCfg[0]=0;
	varCfg[1]=1;
	result = m_pLogicalBusiness->SetIndicatorIntoTable(
		m_pIndicatorData,	//指标
		varCfg,				//参数配置
		varCount,			//参数个数
		m_txTable	//计算需要的参数传输载体以及计算后结果的载体
		);
	if(result==false)
		return false;

	//6股东性质
	iIndicator = 30100028;	//指标=股东性质
	GetIndicatorDataNow(iIndicator);
	if(m_pIndicatorData==NULL)
		return false;
	varCfg[0]=0;
	varCfg[1]=1;
	result = m_pLogicalBusiness->SetIndicatorIntoTable(
		m_pIndicatorData,	//指标
		varCfg,				//参数配置
		varCount,			//参数个数
		m_txTable	//计算需要的参数传输载体以及计算后结果的载体
		);
	if(result==false)
		return false;

	//7占总股本%
	iIndicator = 30100029;	//指标=占总股本%
	GetIndicatorDataNow(iIndicator);
	if(m_pIndicatorData==NULL)
		return false;
	varCfg[0]=0;
	varCfg[1]=1;
	result = m_pLogicalBusiness->SetIndicatorIntoTable(
		m_pIndicatorData,	//指标
		varCfg,				//参数配置
		varCount,			//参数个数
		m_txTable	//计算需要的参数传输载体以及计算后结果的载体
		);
	if(result==false)
		return false;

	//8占流通股本%
	iIndicator = 30100024;	//指标=占流通股本%
	GetIndicatorDataNow(iIndicator);
	if(m_pIndicatorData==NULL)
		return false;
	varCfg[0]=0;
	varCfg[1]=1;
	result = m_pLogicalBusiness->SetIndicatorIntoTable(
		m_pIndicatorData,	//指标
		varCfg,				//参数配置
		varCount,			//参数个数
		m_txTable	//计算需要的参数传输载体以及计算后结果的载体
		);
	if(result==false)
		return false;

	//9
	iIndicator = 30300019;	//指标=总股本
	GetIndicatorDataNow(iIndicator);
	if(m_pIndicatorData==NULL)
		return false;
	varCfg[0]=0;
	varCfg[1]=1;
	result = m_pLogicalBusiness->SetIndicatorIntoTable(
		m_pIndicatorData,	//指标
		varCfg,				//参数配置
		varCount,			//参数个数
		m_txTable	//计算需要的参数传输载体以及计算后结果的载体
		);
	if(result==false)
		return false;

	//10
	iIndicator = 30300014;	//指标=流通股
	GetIndicatorDataNow(iIndicator);
	if(m_pIndicatorData==NULL)
		return false;
	varCfg[0]=0;
	varCfg[1]=1;
	result = m_pLogicalBusiness->SetIndicatorIntoTable(
		m_pIndicatorData,	//指标
		varCfg,				//参数配置
		varCount,			//参数个数
		m_txTable	//计算需要的参数传输载体以及计算后结果的载体
		);
	if(result==false)
		return false;

	//step4-GetData
	//根据之前3个步骤的设置进行数据读取，结果数据存放在table中
	result = m_pLogicalBusiness->GetData(m_txTable,true);
	if(result==false)
		return false;
	
	//注：table既是参数传递的载体，又是存放结果数据的载体
	return true;
}

//业绩预告统计
bool TxStock::StatYJYG(
	int iFiscalYear,					//报告期-财年
	int iFiscalQuarter,					//报告期-财务季度
	std::set<int>& iSecurityId,			//需要机构样本，传入交易实体ID
	std::set<int>& iTypeId,				//预告类型ID
	Tx::Core::Table_Indicator& resTable	//统计结果数据表
	)
{
	//step1
//	Tx::Core::ProgressWnd* pProgressWnd = Tx::Core::ProgressWnd::GetInstance();
	Tx::Core::ProgressWnd prw;
	//step2
	CString sProgressPrompt;
	sProgressPrompt.Format(_T("业绩预告统计..."));
	UINT progId = prw.AddItem(1,sProgressPrompt, 0.0);
	//step3
	prw.Show(15);
	//step4
	prw.SetPercent(progId, 0.1);

	//取得客户端配置文件目录
	CString appCfgPath;
	appCfgPath = 	Tx::Core::SystemPath::GetInstance()->GetConfigPath();

	//默认的返回值状态
	bool result = false;
	Tx::Core::Table_Indicator txTable;	//统计结果数据表
	//清空数据
	txTable.Clear();
	int iCol = 0;

	int i=0;
	//准备样本集=第一参数列;不添加样本集
	txTable.AddParameterColumn(Tx::Core::dtype_int4);

	//日期参数=第二参数列;不添加公告日期
	iCol++;
	txTable.AddParameterColumn(Tx::Core::dtype_int4);

	//报告期-财年参数=第三参数列,不添项
	iCol++;
	txTable.AddParameterColumn(Tx::Core::dtype_int4);
	//m_txTable.SetCell(iCol,0,iFiscalYear);

	//报告期-财务季度参数=第四参数列,不添项
	iCol++;
	txTable.AddParameterColumn(Tx::Core::dtype_int4);

	prw.SetPercent(progId, 0.2);

	//int iIndicator = 30600003;	//指标=业绩预告类型
	int iIndicator = 30700003;	//指标=业绩预告类型
	UINT varCfg[4];			//参数配置
	int varCount=4;			//参数个数
	varCfg[0]=0;
	varCfg[1]=1;
	varCfg[2]=2;
	varCfg[3]=3;
	for(UINT i=0;i<= 3;i++)
	{
		if ( i < 3 )
		{
			GetIndicatorDataNow(iIndicator+i);
			if(m_pIndicatorData==NULL)
			{
				prw.SetPercent(progId, 1.0);
				return false;
			}
			result = m_pLogicalBusiness->SetIndicatorIntoTable(
				m_pIndicatorData,	//指标
				varCfg,				//参数配置
				varCount,			//参数个数
				txTable	//计算需要的参数传输载体以及计算后结果的载体
				);
			if(result==false)
			{
				prw.SetPercent(progId, 1.0);
				return false;
			}
		}

		if (i == 3)
		{
			//iIndicator = 30700013;
			GetIndicatorDataNow(30700013);
			result = m_pLogicalBusiness->SetIndicatorIntoTable(
				m_pIndicatorData,	//指标
				varCfg,				//参数配置
				varCount,			//参数个数
				txTable	//计算需要的参数传输载体以及计算后结果的载体
				);

		}

	}

	//根据之前3个步骤的设置进行数据读取，结果数据存放在table中
	result = m_pLogicalBusiness->GetData(txTable,true);
	if(result==false)
	{
		prw.SetPercent(progId, 1.0);
		return false;
	}

	prw.SetPercent(progId, 0.5);

	//add by lijw 2007-08-05
	std::unordered_map<int,int> InstuToTrade;

	int nColCount = txTable.GetColCount();
	if(nColCount>0)
	{
		UINT* nColArray = new UINT[nColCount];
		for(int i=0;i<nColCount;i++)
			nColArray[i]=i;

		// add by lijw 2007-07-07
		//在程序内部转换样本ID
		std::set<int> iInstitutionId;
		int tempId;
		for(std::set<int>::iterator iter=iSecurityId.begin();iter!=iSecurityId.end();iter++)
		{
			GetSecurityNow(*iter);
			if(m_pSecurity==NULL)
				continue;
            //把不同的交易实体ID转化为相同的机构ID的那些样本去掉
			tempId = (int)m_pSecurity->GetInstitutionId();
			if(iInstitutionId.find(tempId) == iInstitutionId.end())
			{
				iInstitutionId.insert(tempId);
				InstuToTrade.insert(std::make_pair(tempId,*iter));
			}
		}

		Tx::Core::Table_Indicator resTable1;
		resTable1.CopyColumnInfoFrom(txTable);
		txTable.EqualsAt(resTable1,nColArray,nColCount,0,iInstitutionId);

		if(resTable1.GetRowCount()>0)
		{
			//财年col=2
			Tx::Core::Table_Indicator resTable2;
			resTable2.CopyColumnInfoFrom(resTable1);
			resTable1.Between(resTable2, nColArray, nColCount, 2, iFiscalYear, iFiscalYear, TRUE, TRUE);
			if(resTable2.GetRowCount()>0)
			{
				//财季col=3
				Tx::Core::Table_Indicator resTable3;
				resTable3.CopyColumnInfoFrom(resTable2);
				resTable2.Between(resTable3, nColArray, nColCount, 3, iFiscalQuarter, iFiscalQuarter, TRUE, TRUE);

				//2007-09-07
				//筛选预告类型col=8
				resTable.CopyColumnInfoFrom(resTable3);
				if(iTypeId.size()>0)
				{
					if(resTable3.GetRowCount()>0)
					{
						//转换类型
						std::set<byte> byteTypeId;
						for(std::set<int>::iterator iter=iTypeId.begin();iter!=iTypeId.end();iter++)
							byteTypeId.insert((byte)*iter);

						resTable3.EqualsAt(resTable,nColArray,nColCount,4,byteTypeId);
					}
				}
			}
		}

		delete nColArray;
		nColArray = NULL;
	}
	prw.SetPercent(progId, 0.8);

	//2007-09-05
	if(resTable.GetColCount()<=0)
		resTable.CopyColumnInfoFrom(txTable);

	if(resTable.GetColCount()>=4)
	{
		//2007-08-22
		//业绩预告类型
		resTable.InsertCol(3,Tx::Core::dtype_val_string);
		//2007-08-17
		//报告期-财务季度名称,在取得数据之后通过iFiscalQuarter填充此列
		resTable.InsertCol(3,Tx::Core::dtype_val_string);
		//样本名称
		resTable.InsertCol(0,Tx::Core::dtype_val_string);
		//外码
		resTable.InsertCol(0,Tx::Core::dtype_val_string);
		//交易实体
		resTable.InsertCol(0,Tx::Core::dtype_int4);
	}
	prw.SetPercent(progId, 0.9);
#ifdef _DEBUG
	CString strTable1=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable1);
#endif
    //modify by lijw 2008-08-05
	CString sFiscalQuarter;
	sFiscalQuarter = "";
	CString sName;
	int iInstuId;
	int iTradeId;
	std::unordered_map<int,int>::iterator iterMap;
	for(UINT i=0;i<resTable.GetRowCount();i++)
	{
		sFiscalQuarter = TypeMapManage::GetInstance()->GetDatByID(TYPE_FISCAL_YEAR_QUARTER,iFiscalQuarter);
		resTable.SetCell(6,i,sFiscalQuarter);
		//处理机构名称
		//通过机构ID取得名称，由于机构没有缓存，所以无法取得		
		resTable.GetCell(3,i,iInstuId);
        iterMap = InstuToTrade.find(iInstuId);
		if(iterMap != InstuToTrade.end())
			iTradeId = iterMap->second;
		else
			continue;
		resTable.SetCell(0,i,iTradeId);
		this->GetSecurityNow(iTradeId);
		if (m_pSecurity == NULL)
		{
			continue;
		}
		CString sCode=m_pSecurity->GetCode();
		sName = m_pSecurity->GetName();

		resTable.SetCell(1,i,sName);

		resTable.SetCell(2,i,sCode);

		//处理业绩预告类型
		byte byteTyep=0;
		resTable.GetCell(9,i,byteTyep);
		sName = TypeMapManage::GetInstance()->GetDatByID(TYPE_PROFIT_FORECAST_RESULT,(int)byteTyep);
		/*
		sName = "-";
		if(bYjygType==TRUE)
		{
			std::unordered_map<int,CString>::iterator pos;
			pos = mapYjyg.find((int)byteTyep);
			sName = pos->second;
		}
		*/
		resTable.SetCell(7,i,sName);
	}
#ifdef _DEBUG
	strTable1=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable1);
#endif
	//删除不需要展示的列
	resTable.DeleteCol(9);
	resTable.DeleteCol(8);
	resTable.DeleteCol(3);
#ifdef _DEBUG
	strTable1=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable1);
#endif
	prw.SetPercent(progId, 1.0);
	sProgressPrompt+=_T(",完成!");
	prw.SetText(progId, sProgressPrompt);

	return true;
}

//add by lijw 2007-01-20
BOOL TxStock::GetSeniorOfficer(
								  std::set<int> &iSecurityId,		//交易实体
				                  int iSpecifyDate,
								  Tx::Core::Table_Indicator &resTable,
								  bool bAllDate
								  )
{
	//添加进度条
//	ProgressWnd* pwd=Tx::Core::ProgressWnd::GetInstance();
	Tx::Core::ProgressWnd prw;
	UINT pid=prw.AddItem(1,_T("公司高管统计..."),0.0);
	prw.Show(1000);
	//默认的返回值状态
	bool result = false;
	TxBusiness txbusiness;
	txbusiness.InistitutionFilter(iSecurityId);
	if(iSecurityId.empty())
	{
		prw.SetPercent(pid,1.0);
		return false;
	}
	//清空数据
	m_txTable.Clear();
//	LONG iCol = 0;

	//准备样本集=第一参数列:F_INSTITUTION_ID,int型
//	LONG i=0;
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);

	//日期参数=第二参数列;F_END_DATE, int型
    //	iCol++;
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);

	//高管序号=第三参数列;F1, int型
//	iCol++;
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);

	
    const int indicatorIndex = 12;
	long iIndicator[indicatorIndex] = 
	{
		30400223,	//指标=公告日期
        30400224,	//起始日期
		30400226,	//姓名
		30400228,	//职务
		30400227,	//所持股数
		30400234,	//年薪
		30400229,	//出生年份
		30400230,	//身份证号码
		30400231,	//性别
		30400232,	//学历
		30400233,	//职称
		30400235	//信息来源
	};
	UINT varCfg[3];			//参数配置
	int varCount=3;			//参数个数
	for (int i = 0; i < indicatorIndex; i++)
	{
		int tempIndicator = iIndicator[i];

		GetIndicatorDataNow(tempIndicator);
		if (m_pIndicatorData==NULL)
		{ return false; }
		varCfg[0]=0;
		varCfg[1]=1;
		varCfg[2]=2;
		result = m_pLogicalBusiness->SetIndicatorIntoTable(
			m_pIndicatorData,	//指标
			varCfg,				//参数配置
			varCount,			//参数个数
			m_txTable	//计算需要的参数传输载体以及计算后结果的载体
			);
		if(result==false)
		{
			prw.SetPercent(pid,1.0);
			return false;
		}

	}
	//取得全部日期的数据。
	if(bAllDate)
	{
		result = m_pLogicalBusiness->GetData(m_txTable,true);	
		if(result==false)
		{
			prw.SetPercent(pid,1.0);
			return false;
		}

		//添加进度条
		prw.SetPercent(pid,0.3);
#ifdef _DEBUG
		CString strTable1=m_txTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable1);
#endif

		LONG nColCount = m_txTable.GetColCount();
		//	resTable.Clone(m_txTable);
		if(nColCount>0)
		{
			//增列此列是为了增加交易实体ID
			m_txTable.InsertCol(1,Tx::Core::dtype_int4);
			//加入两列,是为了增加名称和外码.
			m_txTable.InsertCol(2,Tx::Core::dtype_val_string);
			m_txTable.InsertCol(3,Tx::Core::dtype_val_string);
			//是为了给截止日期调换位置。
			m_txTable.InsertCol(8,Tx::Core::dtype_int4);
			m_txTable.InsertCol(9,Tx::Core::dtype_int4);
			LONG nColCount2 = m_txTable.GetColCount();
			UINT* nColArray = new UINT[nColCount2];
			for(int i = 0; i < nColCount2; i++)
			{
				nColArray[i] = i;
			}
			//在程序内部转换样本ID
			std::set<int> iInstitutionId;
			for(std::set<int>::iterator iter=iSecurityId.begin();iter!=iSecurityId.end();iter++)
			{
				////给截止日期调换位置
				//resTable.GetCell(4,)
				//取得交易实体ID
				int TradeID = *iter;
				GetSecurityNow(*iter);
				if(m_pSecurity==NULL)
					continue;
				//取得机构ID
				int tempInstitutionid = (int)m_pSecurity->GetInstitutionId();
				iInstitutionId.insert(tempInstitutionid);
				//根据交易实体ID取得样本的名称和外码；
				CString strName,strCode;
				strName = m_pSecurity->GetName();
				strCode = m_pSecurity->GetCode();
				//把机构ID和交易实体ID对应起来。并且把交易实体ID放到表里。
				std::vector<UINT> vecInstiID;
				m_txTable.Find(0,tempInstitutionid,vecInstiID);
				std::vector<UINT>::iterator iteID;
				for(iteID = vecInstiID.begin();iteID != vecInstiID.end();++iteID)
				{
					int tempdate1;
					m_txTable.GetCell(4,*iteID,tempdate1);
					if(tempdate1 == 99999999)
						tempdate1 = Tx::Core::Con_intInvalid;
					m_txTable.SetCell(8,*iteID,tempdate1);
					int iNo;
					m_txTable.GetCell(5,*iteID,iNo);
					m_txTable.SetCell(9,*iteID,iNo);
					m_txTable.SetCell(1,*iteID,TradeID);
					m_txTable.SetCell(2,*iteID,strName);
					m_txTable.SetCell(3,*iteID,strCode);
				}
			}

			resTable.CopyColumnInfoFrom(m_txTable);
			m_txTable.EqualsAt(resTable,nColArray,nColCount2,0,iInstitutionId);
			//添加进度条
			prw.SetPercent(pid,0.6);
#ifdef _DEBUG
			CString strTable3=resTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable3);
#endif
			delete nColArray;
			nColArray =NULL;
			
		}
		else
		{
			resTable.Clone(m_txTable);
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false;
		}	////给截止日期调换位置
		//
		//for(int j = 0;j < (int)resTable.GetRowCount();j++)
		//{
		//	int tempdate1;
		//	resTable.GetCell(4,j,tempdate1);
		//	if(tempdate1 == 99999999)
		//		tempdate1 = Tx::Core::Con_intInvalid;
		//	resTable.SetCell(8,j,tempdate1);
		//}
	
		resTable.DeleteCol(0);
		resTable.DeleteCol(3,2);
	}
	//取得指定日期的数据。
	else
	{
		std::vector<int> tempVec;
		tempVec.push_back(iSpecifyDate);
		std::vector<int> iInstitutionId;
		std::set<int>::iterator iter1;
		std::vector<int>::iterator iter2;
		for(iter1 = iSecurityId.begin();iter1 != iSecurityId.end();++iter1)
		{
			//取得交易实体ID
			int TradeID = *iter1;
			GetSecurityNow(*iter1);
			if(m_pSecurity==NULL)
				continue;
			//取得机构ID
			int tempInstitutionid = (int)m_pSecurity->GetInstitutionId();
			iInstitutionId.push_back(tempInstitutionid);
		}
		result = m_pLogicalBusiness->GetData(m_txTable,true);	
		if(result==false)
		{
			prw.SetPercent(pid,1.0);
			return false;
		}
		LONG nColCount3 = m_txTable.GetColCount();
		UINT* nColArray3 = new UINT[nColCount3];
		for(int i = 0; i < nColCount3; i++)
		{
			nColArray3[i] = i;
		}
		Tx::Core::Table_Indicator tempTable;
		tempTable.CopyColumnInfoFrom(m_txTable);
		m_txTable.EqualsAt(tempTable,nColArray3,nColCount3,0,iInstitutionId);
#ifdef _DEBUG
		CString strTable3=tempTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable3);
#endif
		//取得离截至日期最近的那天的所有样本的纪录。
		resTable.Clear();
		resTable.CopyColumnInfoFrom(tempTable);
        tempTable.EqualsAt(resTable,nColArray3,nColCount3,1,tempVec);
//		tempTable.Between(resTable,nColArray3,nColCount3,1,iSpecifyDate,iSpecifyDate,true,true);
#ifdef _DEBUG		
		CString strTable4;
#endif
		if(resTable.GetRowCount() == 0)
		{
			//对resTable表里的数据进行排序分别按照基金交易实体ID、截止日期
			MultiSortRule multisort;
			multisort.AddRule(0,true);
			multisort.AddRule(1,false);
			tempTable.SortInMultiCol(multisort);
			tempTable.Arrange();
#ifdef _DEBUG
			strTable4=tempTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable4);
#endif
			Tx::Core::Table_Indicator tempTable2,resTableDate;
			tempTable2.CopyColumnInfoFrom(tempTable);
			resTableDate.CopyColumnInfoFrom(tempTable);
//			std::vector<int>::iterator iterV;
			std::vector<int> tempVector;
			int iRowCount;
			int date;
			std::vector<int> VecDate;
			for(iter2 = iInstitutionId.begin();iter2 != iInstitutionId.end();++iter2)
			{
				if(!tempVector.empty())
					tempVector.clear();
				tempVector.push_back(*iter2);
				//首先取出每个样本的数据
				iRowCount = tempTable2.GetRowCount();
				if(iRowCount > 0)
				{
					tempTable2.DeleteRow(0,iRowCount);
					tempTable2.Arrange();
				}
				tempTable.EqualsAt(tempTable2,nColArray3,nColCount3,0,tempVector);
				iRowCount = tempTable2.GetRowCount();
				if(0 == iRowCount)
					continue;
				for(int i = 0;i < iRowCount;i++)
				{
					tempTable2.GetCell(1,i,date);
					if(date < iSpecifyDate)
					{
						if(!VecDate.empty())
							VecDate.clear();
						VecDate.push_back(date);
						break;
					}
				}
				iRowCount = resTableDate.GetRowCount();
				if(iRowCount >0)
				{
					resTableDate.DeleteRow(0,iRowCount);
					resTableDate.Arrange();
				}
				//取得起始日期最大的那天的所有纪录。
				tempTable2.EqualsAt(resTableDate,nColArray3,nColCount3,1,VecDate);	
				resTable.AppendTableByRow(resTableDate,false);
			}		
			if(resTable.GetRowCount() == 0)
			{
				delete nColArray3;
				nColArray3 = NULL;	
				//添加进度条
				prw.SetPercent(pid,1.0);
				return false;
			}
		}	
		delete nColArray3;
		nColArray3 = NULL;			
#ifdef _DEBUG
		strTable4=resTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable4);
#endif
		//增列此列是为了增加交易实体ID
		resTable.InsertCol(1,Tx::Core::dtype_int4);
		//加入两列,是为了增加名称和外码.
		resTable.InsertCol(2,Tx::Core::dtype_val_string);
		resTable.InsertCol(3,Tx::Core::dtype_val_string);
		//是为了给截止日期调换位置。
		resTable.InsertCol(8,Tx::Core::dtype_int4);
		resTable.InsertCol(9,Tx::Core::dtype_int4);
		for(iter1 = iSecurityId.begin();iter1!=iSecurityId.end();++iter1)
		{
			
			//取得交易实体ID
			int TradeID = *iter1;
			GetSecurityNow(*iter1);
			if(m_pSecurity==NULL)
				continue;
			//取得机构ID
			int tempInstitutionid1 = m_pSecurity->GetInstitutionId();
//			iInstitutionId.insert(tempInstitutionid);
			//根据交易实体ID取得样本的名称和外码；
			CString strName,strCode;
			strName = m_pSecurity->GetName();
			strCode = m_pSecurity->GetCode();
			//把机构ID和交易实体ID对应起来。并且把交易实体ID放到表里。
			std::vector<UINT> vecInstiID2;
			resTable.Find(0,tempInstitutionid1,vecInstiID2);
			std::vector<UINT>::iterator iteID;
			for(iteID = vecInstiID2.begin();iteID != vecInstiID2.end();++iteID)
			{

				resTable.SetCell(1,*iteID,TradeID);
				resTable.SetCell(2,*iteID,strName);
				resTable.SetCell(3,*iteID,strCode);
				int tempdate2;
				resTable.GetCell(4,*iteID,tempdate2);
				if(tempdate2 == 99999999)
					tempdate2 = Tx::Core::Con_intInvalid;
				resTable.SetCell(8,*iteID,tempdate2);
				int iNo;
				resTable.GetCell(5,*iteID,iNo);
				resTable.SetCell(9,*iteID,iNo);
			}
		}
		resTable.DeleteCol(0);
		resTable.DeleteCol(3,2);
	}
    ReDefColTypeDecmalToInt(resTable,9);
	ReDefColTypeDecmalToInt(resTable,10);
	/*MultiSortRule multisort;
	multisort.AddRule(0,true);
	multisort.AddRule(4,false);
	multisort.AddRule(3,true);
	resTable.SortInMultiCol(multisort);*/
#ifdef _DEBUG
	CString strTable2=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable2);
#endif	
	//添加进度条
	prw.SetPercent(pid,1.0);
	return true;
}
////Add by lijw 2008-10-06
////将 财年+报告期-〉报告日期
//bool TxFund::TransReportDateToNormal2(Tx::Core::Table_Indicator &tempTable,int iCol)
//{
//	if((UINT)iCol>=tempTable.GetColCount())
//		return false;
//	int iYear,iYearQuarter;
//	for(UINT i=0;i<tempTable.GetRowCount();i++)
//	{		
//		tempTable.GetCell(iCol,i,iYear);
//		tempTable.GetCell(iCol+1,i,iYearQuarter);
//		switch(iYearQuarter)
//		{
//		case 40040001:
//			iYearQuarter=331;
//			break;
//		case 40040002:
//			iYearQuarter=630;
//			break;
//		case 40040004:
//			iYearQuarter=930;
//			break;
//		case 40040006:
//			iYearQuarter=1231;
//			break;
//		case 40040003:
//			iYearQuarter=630;
//			break;
//		case 40040009:
//			iYearQuarter=1231;
//			break;
//		default:
//			break;
//		}
//
//		tempTable.SetCell(iCol,i,iYear*10000+iYearQuarter);
//	}
//	tempTable.DeleteCol(iCol+1);
//
//	return true;
//}

//Add by lijw 2008-10-06
//报告日期
BOOL TxStock::GetDateOfReport(
  					 Tx::Core::Table_Indicator &resTable,//结果数据表
					 std::set<int> &iSecurityId,		//交易实体
					 int iStartDate,					 //起始日期
					 int iEndDate,					     //终止日期
					 char chReportType,					 //报告类型, 从最后一位起, 依次是,一季报，中报，三季报，年报
					 BOOL bActualDate,			//是否为实际披露日期  ，该参数为以后修改程序时备用。               
					 bool bGetAllDate				//是否全部日期              
					 )
{
	if(iStartDate >= iEndDate)
	{
		AfxMessageBox(_T("请重新选择日期范围"));
		return false;
	}
	//添加进度条
	Tx::Core::ProgressWnd prw;
	UINT pid=prw.AddItem(1,_T("报告日期统计..."),0.0);
	prw.Show(1000);

	//ASSERT(iStartDate <= iEndDate);
	//默认的返回值状态
	bool result = false;
	//清空数据
	m_txTable.Clear();

	//准备样本集=第一参数列:机构ID(F_INSTITUTION_ID),int型
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
	//日期参数=第二参数列:财年(F_FISCAL_YEAR), int型
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);

	//日期参数=第三参数列:报告日期(F_FISCAL_YEAR_QUARTER), int型
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
	long iIndicator[6] = 
	{
		30500001,//截至日期
		30500003,//第一次预计披露日期
		30500004,//第二次预计披露日期
		30500005,//第三次预计披露日期
		30500006,//第四次预计披露日期
		30500002 //实际披露日期
	};
	UINT varCfg[3];			//参数配置
	int varCount=3;			//参数个数
	for (int i = 0; i < 6; i++)
	{
		GetIndicatorDataNow(iIndicator[i]);
		if (m_pIndicatorData==NULL)
		{
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false; 
		}
		varCfg[0]=0;
		varCfg[1]=1;
		varCfg[2]=2;
		result = m_pLogicalBusiness->SetIndicatorIntoTable(
													m_pIndicatorData,	//指标
													varCfg,				//参数配置
													varCount,			//参数个数
													m_txTable	//计算需要的参数传输载体以及计算后结果的载体
					                                      );
		if(result==false)
		{
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false; 
		}
	}
    //根据之前3个步骤的设置进行数据读取，结果数据存放在table中

	result = m_pLogicalBusiness->GetData(m_txTable,true);
	if(result==false)
	{
		//添加进度条
		prw.SetPercent(pid,1.0);
		return false; 
	}
	//添加进度条
	prw.SetPercent(pid,0.3);

	int nColCount = m_txTable.GetColCount();
	std::set<int> iInstitutionId;
	std::set<int>::iterator iterSet;
	std::set<int> DifferentId;
	if(nColCount>0)
	{
		UINT* nColArray = new UINT[nColCount];
		for(int i = 0; i < nColCount; i++)
		{
			nColArray[i] = i;
		}
		resTable.CopyColumnInfoFrom(m_txTable);
		
		for(iterSet = iSecurityId.begin();iterSet != iSecurityId.end();iterSet++)
		{
			GetSecurityNow(*iterSet);
			if(m_pSecurity != NULL)
			{
				iInstitutionId.insert((int)m_pSecurity->GetInstitutionId());
				//把不同的交易实体ID转化为相同的机构ID的样本去掉
				DifferentId.insert(*iterSet);
			}			
		}
		m_txTable.EqualsAt(resTable,nColArray,nColCount,0,iInstitutionId);
#ifdef _DEBUG
		CString strTable2=resTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable2);
#endif
		m_txTable.Clear();
		m_txTable.CopyColumnInfoFrom(resTable);	
		std::vector<int> ReportDateVec;
        if(chReportType & 1)
			ReportDateVec.push_back(40040001);
		if(chReportType & 2)
			ReportDateVec.push_back(40040003);
		if(chReportType & 4)
			ReportDateVec.push_back(40040005);
		if(chReportType & 8)
			ReportDateVec.push_back(40040009);
		//根据报告期取数据
		resTable.EqualsAt(m_txTable,nColArray,nColCount,2,ReportDateVec);
#ifdef _DEBUG
		CString strTable3=m_txTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable3);
#endif
		//添加进度条
		prw.SetPercent(pid,0.6);
		resTable.Clear();
		resTable.CopyColumnInfoFrom(m_txTable);	
		if(!bGetAllDate)
		{
			//if(bActualDate)//按实际披露日期
			//	m_txTable.Between(resTable, nColArray, nColCount,8,uStartDate,uEndDate,TRUE,TRUE);
			//else//按第一次预计披露日期
			//	m_txTable.Between(resTable, nColArray, nColCount,4,uStartDate,uEndDate,TRUE,TRUE);
			//按截至日期进行统计
			m_txTable.Between(resTable, nColArray, nColCount,3,iStartDate,iEndDate,TRUE,TRUE);
		}
		else
		{
			resTable.Clone(m_txTable);
		}
#ifdef _DEBUG
		strTable3=resTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable3);
#endif
		delete nColArray;
		nColArray = NULL;
	}
	else
	{
		//添加进度条
		prw.SetPercent(pid,1.0);
		return false; 
	}
	//名称
	resTable.InsertCol(1, Tx::Core::dtype_val_string);
	//外码
	resTable.InsertCol(2,Tx::Core::dtype_val_string);
    //报告期(str)
	resTable.InsertCol(5,Tx::Core::dtype_val_string);
	int tempInstitutionId;
	std::vector<UINT> PositionV;
	std::vector<UINT>::iterator iterU;
	CString strName,strCode;
	int iReportId;
	CString strReportDate;
	for(iterSet = DifferentId.begin();iterSet != DifferentId.end();++iterSet)
	{
		GetSecurityNow(*iterSet);
		if(m_pSecurity==NULL)
			continue;
		tempInstitutionId = (int)m_pSecurity->GetInstitutionId();
		if(!PositionV.empty())
			PositionV.clear();
		resTable.Find(0,tempInstitutionId,PositionV);
		if(PositionV.empty())
			continue;
		strName = m_pSecurity->GetName();
		strCode = m_pSecurity->GetCode();
		for(iterU = PositionV.begin();iterU != PositionV.end();++iterU)
		{
			//把交易实体ID放在机构ID那一列
			resTable.SetCell(0,*iterU,*iterSet);
			resTable.SetCell(1,*iterU,strName);
			resTable.SetCell(2,*iterU,strCode);
			//把报告期转化CString
			resTable.GetCell(4,*iterU,iReportId);
			switch(iReportId)
			{
			case 40040001:
				strReportDate = _T("一季报");
				break;
			case 40040003:
				strReportDate = _T("中报");
				break;
			case 40040005:
				strReportDate = _T("三季报");
				break;
			case 40040009:
				strReportDate = _T("年报");
				break;
			default:
				break;
			}
			resTable.SetCell(5,*iterU,strReportDate);		
		}
	}
	resTable.DeleteCol(3,2);//删除财年和报告期（int）	
#ifdef _DEBUG
	CString strTable7=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable7);
#endif


	//添加进度条
	prw.SetPercent(pid,1.0);
	return TRUE;
}
//新股首日
//modify by lijw 2008-1-20
BOOL TxStock::GetFirstDateOfStock(
                         Tx::Core::Table_Indicator &resTable,//结果数据表
						std::set<int> &iSecurityId,		//交易实体
                         //std::set<int> &iInstitutionId,   //机构样本
					     INT uStartDate,					 //起始日期
					     INT uEndDate,
						 bool bSpecialDate						//是否指定日期区间。否则取全部数据
				        )
{
	//添加进度条
//	ProgressWnd* pwd=Tx::Core::ProgressWnd::GetInstance();
	Tx::Core::ProgressWnd prw;
	UINT pid=prw.AddItem(1,_T("新股首日统计..."),0.0);
	prw.Show(1000);
	//ASSERT(uStartDate > 0 && uEndDate > 0 && uEndDate >= uStartDate);我已 在外面判断过了。

	//默认的返回值状态
	bool result = false;
	//清空数据
	m_txTable.Clear();
	LONG iCol = 0;

	
	//=第一参数列:f_trans_obj_id,int型
	LONG i=0;
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);


	LONG iIndicator = 30000061;	//指标
	UINT varCfg[1];			//参数配置
	int varCount=1;			//参数个数
	for (i = 0; i < 13; i++, iIndicator++)
	{
		GetIndicatorDataNow(iIndicator);
		if (m_pIndicatorData==NULL)
		{ 
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false; 
		}
		varCfg[0]=0;
		result = m_pLogicalBusiness->SetIndicatorIntoTable(
				m_pIndicatorData,	//指标
				varCfg,				//参数配置
				varCount,			//参数个数
				m_txTable	//计算需要的参数传输载体以及计算后结果的载体
				);
		if(result==false)
		{
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false;
		}
	}
	//根据之前3个步骤的设置进行数据读取，结果数据存放在table中
	result = m_pLogicalBusiness->GetData(m_txTable,true);
	if(result==false)
	{
		//添加进度条
		prw.SetPercent(pid,1.0);
		return false;
	}

	std::set<int>::iterator iter;
	std::set<int> tempS;
	for(iter = iSecurityId.begin();iter != iSecurityId.end();++iter)
	{
		GetSecurityNow(*iter);
		if(m_pSecurity != NULL)
		{
			if(!m_pSecurity->InNoListedBlock())
				tempS.insert(*iter);
		}
	}
	iSecurityId.clear();
	iSecurityId.insert(tempS.begin(),tempS.end());
	if(iSecurityId.empty())
	{
		//添加进度条
		prw.SetPercent(pid,0.6);
		return false;
	}
	//添加进度条
	prw.SetPercent(pid,0.6);
#ifdef _DEBUG
	CString strTable=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	LONG nColCount = m_txTable.GetColCount();
	if(nColCount>0)
	{
		UINT* nColArray = new UINT[nColCount];
		for(i = 0; i < nColCount; i++)
		{
			nColArray[i] = i;
		}
		resTable.CopyColumnInfoFrom(m_txTable);

		m_txTable.EqualsAt(resTable,nColArray,nColCount,0,iSecurityId);
		if(resTable.GetRowCount() == 0)
		{
			delete nColArray; 
			nColArray = NULL;
			//添加进度条
			prw.SetPercent(pid,0.6);
			return false;
		}
		m_txTable.Clear();

		m_txTable.CopyColumnInfoFrom(resTable);
		//resTable.Between(m_txTable, nColArray, nColCount, nColIndex, uStartDate, nEndDate, TRUE, TRUE);
		if(!bSpecialDate)
			resTable.Between(m_txTable, nColArray, nColCount, 2, uStartDate, uEndDate, TRUE, TRUE);
		else
			m_txTable.Clone(resTable);
		int iRow = m_txTable.GetRowCount(), iValue = 0;//modify by lijw 2008-11-28,把这句代码从下面移动到这。
		if(0 == iRow)
		{
			delete nColArray; 
			nColArray = NULL;
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false;
		}
		//增加一列为了添加证监会行业代码
		m_txTable.AddCol(Tx::Core::dtype_val_string);
		//转换交易实体ID 为 外码,新增一列(第二列)
		m_txTable.InsertCol(2, Tx::Core::dtype_val_string);
        //为取成交量和成交金额的值而定义的两个变量。
		double traden,tradem;
		//为取涨幅和换手率的值而定义的两个变量。
		double dChangeRate,dSwitchRate;
		//std::unordered_map<int,int> mapIntInt;
		//std::unordered_map<int,int>::iterator iterIntInt;
		////取得证券会的行业代码数据
		//Tx::Data::TypeMapManage::GetInstance()->GetTypeMapITI(TYPE_CSRC_INDUSTRY, mapIntInt);
		//int iRow = m_txTable.GetRowCount(), iValue = 0;
		CString strCode;
		for (int i = 0; i < iRow; i++)
		{
			//取第一列的实易实体ID
			m_txTable.GetCell(0, i, iValue); 
			GetSecurityNow(iValue);
			//取得外码
			if(m_pSecurity == NULL)
				continue;
			m_txTable.SetCell(2, i, m_pSecurity->GetCode());
            strCode = m_pSecurity->GetCSRCIndustryCode();
			if (strCode == Tx::Core::Con_strInvalid)
			{
				 strCode = m_pSecurity->GetCSRCIndustryCode(1);
			}
			m_txTable.SetCell(15,i,strCode);
            //取换手率的值
			m_txTable.GetCell(13,i,dSwitchRate);
			if(dSwitchRate>-10000000.0)
				m_txTable.SetCell(13,i,dSwitchRate*100);
            //取成交量的值
			m_txTable.GetCell(11,i,traden);
			if (traden!=Tx::Core::Con_doubleInvalid)
			{
				traden=traden/100;
			}
			m_txTable.SetCell(11,i,traden);
            //取涨跌幅的值
			m_txTable.GetCell(10,i,dChangeRate);
			if(dChangeRate>-10000000.0)
				m_txTable.SetCell(10,i,dChangeRate*100);

            //取成交金额的值
			m_txTable.GetCell(12,i,tradem);
			if (tradem!=Tx::Core::Con_doubleInvalid)
			{
				tradem=tradem/10000;
			}
			m_txTable.SetCell(12,i,tradem);
		}
		
		//把成交量后面的小数点去掉。
		ReDefColTypeDecmalToInt(m_txTable,11);

		resTable.Clear();
		resTable.Clone(m_txTable);
		delete nColArray; 
		nColArray = NULL;
	}
	else
	{
		resTable.Clone(m_txTable);
	}
#ifdef _DEBUG
	CString strTable2=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable2);
#endif

	//添加进度条
	prw.SetPercent(pid,1.0);
	return true;
}

//年报快递T_FINANCIAL_REPORT_EXPRESS
BOOL TxStock::GetFinancialReport(//LiLi
									Tx::Core::Table_Indicator &resTable,//结果数据表
									std::set<int> &iSecurityId,		//交易实体
									INT uStartDate,					 //(公告日期的)起始日期
									INT uEndDate					 //(公告日期的)结束日期	
				)
{
	//添加进度条 add by lijw 2008-02-01
//	ProgressWnd* pwd=Tx::Core::ProgressWnd::GetInstance();
	Tx::Core::ProgressWnd prw;
	UINT pid=prw.AddItem(1,_T("业绩快递统计..."),0.0);
	prw.Show(1000);

	ASSERT(uStartDate > 0 && uEndDate > 0 && uEndDate >= uStartDate);
	//取得客户端配置文件目录
	CString appCfgPath;
	appCfgPath = 	Tx::Core::SystemPath::GetInstance()->GetConfigPath();

	//默认的返回值状态
	bool result = false;
	//清空数据
	m_txTable.Clear();
	LONG iCol = 0;
	
	//=第一参数列:F_INSTITUTION_ID,int型
	LONG i=0;
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);

	//第二参数列:F_DISCLOSURE_DATE,int型
	iCol++;
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);


	//第三参数列:F_END_DATE,int型
	iCol++;
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
    //add by lijw 2008-11-29
	const int indicatorIndex = 16;
	long indicator[indicatorIndex] = 
	{
		30600001,
		30600002,
		30600011,
		30600012,//
		30600013,
		30600014,//
		30600015,
		30600003,
		30600005,
		30600004,
		30600006,
		30600007,
		30600008,
		30600009,
		30600010,
		30600017
	};
	LONG iIndicator = 30600001;	//指标
	UINT varCfg[3];			//参数配置
	int varCount=3;			//参数个数
	for (i = 0; i < indicatorIndex; i++)
	{
		GetIndicatorDataNow(indicator[i]);
		
		if (m_pIndicatorData==NULL)
		{ 
			prw.SetPercent(pid,1.0);
			return false; 
		}
		varCfg[0]=0;
		varCfg[1]=1;
		varCfg[2]=2;
		result = m_pLogicalBusiness->SetIndicatorIntoTable(
				m_pIndicatorData,	//指标
				varCfg,				//参数配置
				varCount,			//参数个数
				m_txTable	//计算需要的参数传输载体以及计算后结果的载体
				);
		if(result==false)
		{
			prw.SetPercent(pid,1.0);
			return false; 
		}
	}
	//根据之前3个步骤的设置进行数据读取，结果数据存放在table中
	result = m_pLogicalBusiness->GetData(m_txTable,true);
#ifdef _DEBUG
	CString strTable=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	if(result==false)
	{
		prw.SetPercent(pid,1.0);
		return false; 
	}
	//添加进度条
	prw.SetPercent(pid,0.6);

	//add by lijw 2008-08-05
	std::unordered_map<int,int> InstuToTrade;
	std::unordered_map<int,int>::iterator iterMap;
	LONG nColCount = m_txTable.GetColCount();
	if(nColCount>0)
	{
		UINT* nColArray = new UINT[nColCount];
		for(i = 0; i < nColCount; i++)
		{
			nColArray[i] = i;
		}
		resTable.CopyColumnInfoFrom(m_txTable);
		//2007-08-07
		//在程序内部转换样本ID
		std::set<int> iInstitutionId;
		int tempId;
		for(std::set<int>::iterator iter=iSecurityId.begin();iter!=iSecurityId.end();iter++)
		{
			GetSecurityNow(*iter);
			if(m_pSecurity==NULL)
				continue;
            //把不同的交易实体ID转化为相同的机构ID的那些样本去掉
			tempId = (int)m_pSecurity->GetInstitutionId();
			if(iInstitutionId.find(tempId) == iInstitutionId.end())
			{
				iInstitutionId.insert(tempId);
				InstuToTrade.insert(std::make_pair(tempId,*iter));
			}
		}
		m_txTable.EqualsAt(resTable,nColArray,nColCount,0,iInstitutionId);
#ifdef _DEBUG
		CString strTable2=resTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable2);
#endif
		m_txTable.Clear();

		m_txTable.CopyColumnInfoFrom(resTable);
		//resTable.Between(m_txTable, nColArray, nColCount, nColIndex, uStartDate, nEndDate, TRUE, TRUE);
		resTable.Between(m_txTable, nColArray, nColCount, 1, uStartDate, uEndDate, TRUE, TRUE);
		if(m_txTable.GetRowCount() == 0)
		{
			delete nColArray; 
			nColArray = NULL;
			prw.SetPercent(pid,1.0);
			return false; 
		}
		//将m_txTable中的第一列(机构ID)转成公司名称20070817
		////modify by lijw 2008-08-05
		int iTradeId;
		CString sCode,strName;
		{
			//先加入一列
			m_txTable.InsertCol(1, Tx::Core::dtype_val_string);
			m_txTable.InsertCol(2, Tx::Core::dtype_val_string);
			m_txTable.InsertCol(9, Tx::Core::dtype_double);
			m_txTable.InsertCol(12, Tx::Core::dtype_double);
			double dData1,dData2,dRiseRate;
			INT i = 0, iRow = m_txTable.GetRowCount(), iValue = 0;
			Collection* pC = NULL;
			for (; i < iRow; i++)
			{
				m_txTable.GetCell(0, i, iValue);
				iterMap = InstuToTrade.find(iValue);
				if(iterMap != InstuToTrade.end())
				{
					iTradeId = iterMap->second;
				}
				m_txTable.SetCell(0,i,iTradeId);
				this->GetSecurityNow(iTradeId);
				if(m_pSecurity != NULL)
				{
					sCode = m_pSecurity->GetCode();
					strName = m_pSecurity->GetName();
					m_txTable.SetCell(1,i,strName);
					m_txTable.SetCell(2,i,sCode);
				}
				m_txTable.GetCell(7, i, dData1);
				m_txTable.GetCell(8, i, dData2);
				if(dData1 <= 0 || dData2 <= 0)
				{
					dRiseRate = Tx::Core::Con_doubleInvalid;
				}
				else
				{
					dRiseRate = (dData1 - dData2)/dData2*100;
					if(fabs(dRiseRate - Tx::Core::Con_doubleInvalid) < 0.000001)
						dRiseRate = Tx::Core::Con_doubleInvalid;
				}				
				m_txTable.SetCell(9, i, dRiseRate);
				m_txTable.GetCell(10, i, dData1);
				m_txTable.GetCell(11, i, dData2);
				if(dData1 <= 0 || dData2 <= 0)
				{
					dRiseRate = Tx::Core::Con_doubleInvalid;
				}
				else
				{
					dRiseRate = (dData1 - dData2)/dData2*100;
					if(fabs(dRiseRate - Tx::Core::Con_doubleInvalid) < 0.000001)
						dRiseRate = Tx::Core::Con_doubleInvalid;
				}				
				m_txTable.SetCell(12, i, dRiseRate);
			}
			////再删除第一列
			//m_txTable.DeleteCol(0);
		}
		resTable.Clear();
		resTable.Clone(m_txTable);
		delete nColArray; 
		nColArray = NULL;
	}
	else
	{
		resTable.Clone(m_txTable);
	}
#ifdef _DEBUG
	CString strTable3=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable3);
#endif
	//添加进度条
	prw.SetPercent(pid,1.0);
	return TRUE;
}

//add by lijw 2008-1-22
//十大股东
BOOL TxStock::GetTopTenShareHolder(
										Tx::Core::Table_Indicator &resTable,//结果数据表
										std::set<int> &iSecurityId,	    	//交易实体
										INT uStartDate,					    //(公告日期的)起始日期
										INT uEndDate,			    	    //(公告日期的)终止日期
										const CString &strKeyWord,          //关键字(股名名称)
										bool bAllDate,						//是否是全部的日期
										BOOL bExactitude		    //是否精确查询
									
										)
{
	//添加进度条
//	ProgressWnd* pwd=Tx::Core::ProgressWnd::GetInstance();
	Tx::Core::ProgressWnd prw;
	UINT pid=prw.AddItem(1,_T("十大股东统计..."),0.0);
	prw.Show(1000);
	//十大股东30600018-25

	//十大流通股东30600026-35
	ASSERT(uStartDate > 0 && uEndDate > 0 && uEndDate >= uStartDate);

	//若同时含ab股将b股过滤, changed by chenyh at 2008-12-23
	TxBusiness::InistitutionFilter(iSecurityId);

	//默认的返回值状态
	bool result = false;
	//清空数据
	m_txTable.Clear();
	LONG iCol = 0;

	//=第一参数列:F_INSTITUTION_ID,int型
	LONG i=0;
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);

	//=第二参数列:F_END_DATE,int型
	iCol++;
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);

	//=第三参数列:F1(序号),int型
	iCol++;
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);

	LONG iIndicator = 30600018;	//指标, 前几个依次是公告日期, F2(十大股东序号), F3(股东名称)
	UINT varCfg[3];			//参数配置
	int varCount=3;			//参数个数
	for (i = 0; i < 8; i++, iIndicator++)
	{
		if(iIndicator == 30600022)
			continue;
		else if (iIndicator == 30600021)
		{
			GetIndicatorDataNow(iIndicator+2);
		}
		else if (iIndicator == 30600023)
		{
			GetIndicatorDataNow(iIndicator-2);
		}
		else
			GetIndicatorDataNow(iIndicator);
		if (m_pIndicatorData==NULL)
		{ return false; }
		varCfg[0]=0;
		varCfg[1]=1;
		varCfg[2]=2;
		result = m_pLogicalBusiness->SetIndicatorIntoTable(
				m_pIndicatorData,	//指标
				varCfg,				//参数配置
				varCount,			//参数个数
				m_txTable	//计算需要的参数传输载体以及计算后结果的载体
				);
		if(result==false)
		{
			return false;
		}
	}


	
	//根据之前3个步骤的设置进行数据读取，结果数据存放在table中
	result = m_pLogicalBusiness->GetData(m_txTable,true);
#ifdef _DEBUG
	CString strTable1=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable1);
#endif
	if(result==false)
		return false;
    //为暂时存储机构ID而作准备的。
    std::vector<int> iInstitutionId;
	LONG nColCount = m_txTable.GetColCount();
	UINT* nColArray = new UINT[nColCount];
	for(i = 0; i < nColCount; i++)
	{
		nColArray[i] = i;
	}
	//在程序内部转换样本ID

	for(std::set<int>::iterator iter=iSecurityId.begin();iter!=iSecurityId.end();iter++)
	{
		GetSecurityNow(*iter);
		if(m_pSecurity==NULL)
			continue;

		iInstitutionId.push_back((int)m_pSecurity->GetInstitutionId());
	}
	resTable.Clear();
	resTable.CopyColumnInfoFrom(m_txTable);
	m_txTable.EqualsAt(resTable,nColArray,nColCount,0,iInstitutionId);
	m_txTable.Clear();
	m_txTable.CopyColumnInfoFrom(resTable);
	if(bAllDate)
	{	
		resTable.Between(m_txTable,nColArray,nColCount,3,uStartDate,uEndDate,true,true);
		resTable.Clear();
		resTable.Clone(m_txTable);
	}
	delete	nColArray;
	nColArray = NULL;
	if (0 == resTable.GetRowCount())
	{
		//添加进度条
		prw.SetPercent(pid,1.0);
		return false;
	}
	/*else
	{
		resTable.Clone(m_txTable);
		return FALSE;
	}*/
#ifdef _DEBUG
	CString strTable2=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable2);
#endif
	CString strKw(strKeyWord);
	strKw.Trim();
	//添加进度条
	prw.SetPercent(pid,0.6);
	//如果关键字为空或表无记录，不用筛选
	if (!(strKw.IsEmpty() || resTable.GetRowCount() < 1))
	{	
		std::set<LONG> tRecArray;	
		
		if (bExactitude)
		{//精确查询
			LONG k = 0, iCount = static_cast<LONG>(resTable.GetRowCount());
			CString strValue(_T(""));
			for (k = 0; k < iCount; k++)
			{
				resTable.GetCellString(5, k, strValue);
				if (strValue.Compare(strKw) == 0)
				{
					tRecArray.insert(k);
				}
			}
		}
		else
		{//模糊查询, 可能有多个关键字，以空格隔开		
			while (strKw.Replace(_T("  "), _T(" ")) > 0)
			{
			}
			CStringArray tKeyArray;
			CString strSec(strKw);
			long iPos = strSec.Find(_T(" "));
			if (iPos < 0)
			{
				tKeyArray.Add(strSec);
			}
			else
			{
				while (iPos > 0)
				{
					tKeyArray.Add(strSec.Left(iPos));
					strSec = strSec.Right(strSec.GetLength() - iPos - 1);
					iPos = strSec.Find(_T(" "));
				}
				tKeyArray.Add(strSec);
			}
			//已取到关键字数组， 开始筛选
#ifdef _DEBUG
			{
				INT i = 0;
				for (i = 0; i < tKeyArray.GetSize(); i++)
				{
					TRACE(_T("%s\r\n"), tKeyArray.GetAt(i));
				}
			}
#endif
			LONG j = 0, k = 0, iSize = tKeyArray.GetSize(), iCount = static_cast<LONG>(resTable.GetRowCount());
			CString strValue(_T(""));
			std::set<LONG>::iterator iter_find;
			for (j = 0; j < iSize; j++)
			{
				for (k = 0; k < iCount; k++)
				{
					resTable.GetCellString(5, k, strValue);
					if (j != 0)
					{
						if (-1 == strValue.Find(tKeyArray.GetAt(j)))
						{
							iter_find = tRecArray.find(k);
							if (iter_find != tRecArray.end())
							{
								tRecArray.erase(iter_find);
							}						
						}
					}
					else
					{//第一个关键字
						if (-1 != strValue.Find(tKeyArray.GetAt(j)))
						{
							tRecArray.insert(k);
						}
					}
				}
			}
		}

		m_txTable.Clear();

		m_txTable.CopyColumnInfoFrom(resTable);

		//至此，tRecArray中保存的是符合条件的记录行索引
		if(tRecArray.empty())
		{
			resTable.Clear();
			return false;
		}
		else
		{
			std::set<LONG>::iterator iter = tRecArray.begin();
			std::set<LONG>::iterator iter_end = tRecArray.end();
			LONG j = 0, k = 0, iColCount = resTable.GetColCount();
			m_txTable.AddRow(tRecArray.size());
			Tx::Core::VariantData tVar;
			while (iter != iter_end)
			{
				for (k = 0; k < iColCount; k++)
				{
					tVar.data_type = resTable.GetColType(k);
					resTable.GetCell(k, *iter, tVar);
					m_txTable.SetCell(k, j, tVar);
				}
				j++, iter++;
			}

		}
	}
	else
	{
		m_txTable.CopyColumnInfoFrom(resTable);
		if (resTable.GetRowCount() != 0)
		{
			m_txTable.Clone(resTable);
		}		
	}


	//为插入交易实体ID做准备。
	m_txTable.InsertCol(1,Tx::Core::dtype_int4);//插入交易实体ID
	//插入这两列为了插入代码和名称。
	m_txTable.InsertCol(2,Tx::Core::dtype_val_string);//插入代码
	m_txTable.InsertCol(3,Tx::Core::dtype_val_string);//插入券名称
	
	if(iSecurityId.size() != iInstitutionId.size())
	{
#ifdef _DEBUG
		AfxMessageBox(_T("机构ID和交易实体ID的数量不一样"));
#endif
		return false;
	}
	int iInsCount = 0;
	std::set<int>::iterator iter1;
	for(iter1 = iSecurityId.begin();iter1!=iSecurityId.end();++iter1,iInsCount++)
	{

		//取得交易实体ID
		int TradeID = *iter1;
		GetSecurityNow(*iter1);
		if(m_pSecurity==NULL)
			continue;
		//取得机构ID
		int tempInstitutionid1 = iInstitutionId[iInsCount];
		//			iInstitutionId.insert(tempInstitutionid);
		//根据交易实体ID取得样本的名称和外码；
		CString strName,strCode;
		strName = m_pSecurity->GetName();
		strCode = m_pSecurity->GetCode();
		//把机构ID和交易实体ID对应起来。并且把交易实体ID放到表里。
		std::vector<UINT> vecInstiID2;
		m_txTable.Find(0,tempInstitutionid1,vecInstiID2);
		std::vector<UINT>::iterator iteID;
		for(iteID = vecInstiID2.begin();iteID != vecInstiID2.end();++iteID)
		{
			m_txTable.SetCell(1,*iteID,TradeID);
			m_txTable.SetCell(2,*iteID,strName);
			m_txTable.SetCell(3,*iteID,strCode);
		}
	}
	std::unordered_map<int,CString> indexdat_map;
	TypeMapManage::GetInstance()->GetTypeMap(12,indexdat_map);
	std::unordered_map<int,CString>::iterator iterMap;
	byte tempdate;
	double vproportion1,vproportion2;
	CString strMark;
	int imark;
	//把股东序号放到公告日期的那一列。
	int enddate;
	for(int j = 0;j < (int)m_txTable.GetRowCount();j++)
	{
		m_txTable.GetCell(6,j,enddate);
		m_txTable.SetCell(5,j,enddate);
		m_txTable.GetCell(7,j,tempdate);
		int tempdate1 = (int)tempdate;
		m_txTable.SetCell(6,j,tempdate1);		
		m_txTable.GetCell(12,j,vproportion1);
		if (abs(vproportion1 - Tx::Core::Con_doubleInvalid) < 0.000001)
			vproportion2 = Tx::Core::Con_doubleInvalid;
		else
			vproportion2 = vproportion1 * 100;
		m_txTable.SetCell(12,j,vproportion2);
		m_txTable.GetCell(10,j,strMark);
		imark = _ttoi(strMark);
		iterMap = indexdat_map.find(imark);
		if(iterMap != indexdat_map.end())
			m_txTable.SetCell(10,j,iterMap->second);
		else
			m_txTable.SetCell(10,j,Tx::Core::Con_strInvalid);
	}
#ifdef _DEBUG
	CString strTable4=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable4);
#endif
	m_txTable.DeleteCol(7);
	m_txTable.DeleteCol(0);
	resTable.Clear();
	resTable.Clone(m_txTable);
	ReDefColTypeDecmalToInt(resTable,7);
//	//为了添加序号，首先按照基金的交易实体ID和佣金量对resTable进行排序。
//	MultiSortRule multisort;
//	multisort.AddRule(0,true);
//	multisort.AddRule(4,false);	
//	multisort.AddRule(3,true);	
//	resTable.SortInMultiCol(multisort);
//	resTable.Arrange();
//#ifdef _DEBUG
//	CString strTable3=resTable.TableToString();
//	Tx::Core::Commonality::String().StringToClipboard(strTable3);
//#endif
	//添加进度条
	prw.SetPercent(pid,1.0);
	return true;
}



//add by lijw 2008-1-20
//十大流通股东GetTOpTenSHAREHOLDER_TRADABLE
BOOL TxStock::GetTopTenShareHolder_Tradable(
										Tx::Core::Table_Indicator &resTable,//结果数据表
										std::set<int> &iSecurityId,	    	//交易实体
										INT uStartDate,					    //(公告日期的)起始日期
										INT uEndDate,			    	    //(公告日期的)终止日期
										const CString &strKeyWord,          //关键字(股名名称)
										bool bAllDate,						//是否是全部的日期
										BOOL bExactitude		    //是否精确查询

										)
{
	//添加进度条
	Tx::Core::ProgressWnd prw;
	UINT pid=prw.AddItem(1,_T("十大流通股东统计..."),0.0);
	prw.Show(1000);
	//十大流通股东30600026-35
	ASSERT(uStartDate > 0 && uEndDate > 0 && uEndDate >= uStartDate);

	//若同时含ab股将b股过滤, changed by chenyh at 2008-12-23
	TxBusiness::InistitutionFilter(iSecurityId);

	//默认的返回值状态
	bool result = false;
	//清空数据
	m_txTable.Clear();
	LONG iCol = 0;

	//=第一参数列:F_INSTITUTION_ID,int型
	LONG i=0;
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);

	//=第二参数列:F_END_DATE,int型
	iCol++;
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);

	//=第三参数列:F1(序号),int型
	iCol++;
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);

	LONG iIndicator = 30600026;	//指标, 前几个依次是公告日期, F2(十大股东序号), F3(股东名称)
	UINT varCfg[3];			//参数配置
	int varCount=3;			//参数个数
	for (i = 0; i < 10; i++, iIndicator++)
	{
		if(iIndicator == 30600029 || iIndicator == 30600032 || iIndicator == 30600034)
			continue;		
		GetIndicatorDataNow(iIndicator);
		if (m_pIndicatorData==NULL)
		{ return false; }
		varCfg[0]=0;
		varCfg[1]=1;
		varCfg[2]=2;
		result = m_pLogicalBusiness->SetIndicatorIntoTable(
				m_pIndicatorData,	//指标
				varCfg,				//参数配置
				varCount,			//参数个数
				m_txTable	//计算需要的参数传输载体以及计算后结果的载体
				);
		if(result==false)
		{
			return FALSE;
		}
	}

	//根据之前3个步骤的设置进行数据读取，结果数据存放在table中
	result = m_pLogicalBusiness->GetData(m_txTable,true);
	if(result==false)
	{
		return false;
	}

#ifdef _DEBUG
	CString strTable5=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable5);
#endif

	std::vector<int> iInstitutionId;
	LONG nColCount = m_txTable.GetColCount();
	UINT* nColArray = new UINT[nColCount];
	for(i = 0; i < nColCount; i++)
	{
		nColArray[i] = i;
	}

	//在程序内部转换样本ID
	for(std::set<int>::iterator iter=iSecurityId.begin();iter!=iSecurityId.end();iter++)
	{
		GetSecurityNow(*iter);
		if(m_pSecurity==NULL)
		{
			continue;
		}
		iInstitutionId.push_back((int)m_pSecurity->GetInstitutionId());
	}
	resTable.Clear();
	resTable.CopyColumnInfoFrom(m_txTable);
	m_txTable.EqualsAt(resTable,nColArray,nColCount,0,iInstitutionId);
#ifdef _DEBUG
	strTable5=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable5);
#endif
	m_txTable.Clear();
	m_txTable.CopyColumnInfoFrom(resTable);
	if(bAllDate)
	{	
		resTable.Between(m_txTable,nColArray,nColCount,3,uStartDate,uEndDate,true,true);
		resTable.Clear();
		resTable.Clone(m_txTable);
	}
#ifdef _DEBUG
	strTable5=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable5);
#endif
	delete nColArray;
	nColArray = NULL;
	if (0 == resTable.GetRowCount())
	{
		//添加进度条
		prw.SetPercent(pid,1.0);
		return false;
	}

	CString strKw(strKeyWord);
	strKw.Trim();
	//如果关键字为空或表无记录，不用筛选
	if (!(strKw.IsEmpty() || resTable.GetRowCount() < 1))
	{
		std::set<LONG> tRecArray;	
		//目前第三个指标列是股东名称, 对应是结果表的第6列
		if (bExactitude)
		{//精确查询
			LONG k = 0, iCount = static_cast<LONG>(resTable.GetRowCount());
			CString strValue(_T(""));
			for (k = 0; k < iCount; k++)
			{
				resTable.GetCellString(5, k, strValue);
				if (strValue.Compare(strKw) == 0)
				{
					tRecArray.insert(k);
				}
			}
		}
		else
		{//模糊查询, 可能有多个关键字，以空格隔开		
			while (strKw.Replace(_T("  "), _T(" ")) > 0)
			{
			}
			CStringArray tKeyArray;
			CString strSec(strKw);
			LONG iPos = strSec.Find(_T(" "));
			if (iPos < 0)
			{
				tKeyArray.Add(strSec);
			}
			else
			{
				while (iPos > 0)
				{
					tKeyArray.Add(strSec.Left(iPos));
					strSec = strSec.Right(strSec.GetLength() - iPos - 1);
					iPos = strSec.Find(_T(" "));
				}
				tKeyArray.Add(strSec);
			}
			//已取到关键字数组， 开始筛选
#ifdef _DEBUG
			{
				INT i = 0;
				for (i = 0; i < tKeyArray.GetSize(); i++)
				{
					TRACE(_T("%s\r\n"), tKeyArray.GetAt(i));
				}
			}
#endif
			LONG j = 0, k = 0, iSize = tKeyArray.GetSize(), iCount = static_cast<LONG>(resTable.GetRowCount());
			CString strValue(_T(""));
			std::set<LONG>::iterator iter_find;
			for (j = 0; j < iSize; j++)
			{
				for (k = 0; k < iCount; k++)
				{
					resTable.GetCellString(5, k, strValue);
					if (j != 0)
					{
						if (-1 == strValue.Find(tKeyArray.GetAt(j)))
						{
							iter_find = tRecArray.find(k);
							if (iter_find != tRecArray.end())
							{
								tRecArray.erase(iter_find);
							}						
						}
					}
					else
					{//第一个关键字
						if (-1 != strValue.Find(tKeyArray.GetAt(j)))
						{
							tRecArray.insert(k);
						}
					}
				}
			}
		}

		m_txTable.Clear();

		m_txTable.CopyColumnInfoFrom(resTable);

		//至此，tRecArray中保存的是符合条件的记录行索引
		if(tRecArray.empty())
		{
			resTable.Clear();
			return false;
		}
		else
		{
			std::set<LONG>::iterator iter = tRecArray.begin();
			std::set<LONG>::iterator iter_end = tRecArray.end();
			LONG j = 0, k = 0, iColCount = resTable.GetColCount();
			m_txTable.AddRow(tRecArray.size());
			Tx::Core::VariantData tVar;
			while (iter != iter_end)
			{
				for (k = 0; k < iColCount; k++)
				{
					tVar.data_type = resTable.GetColType(k);
					resTable.GetCell(k, *iter, tVar);
					m_txTable.SetCell(k, j, tVar);
				}
				j++;
				iter++;
			}
		}
	}
	else
	{
		m_txTable.CopyColumnInfoFrom(resTable);
		if (resTable.GetRowCount() != 0)
		{
			m_txTable.Clone(resTable);
		}	
	}
	
	//添加进度条
	prw.SetPercent(pid,0.6);

	//为插入交易实体ID做准备。
	m_txTable.InsertCol(1,Tx::Core::dtype_int4);//插入交易实体ID
	//插入这两列为了插入代码和名称。
	m_txTable.InsertCol(2,Tx::Core::dtype_val_string);//插入代码
	m_txTable.InsertCol(3,Tx::Core::dtype_val_string);//插入券名称

	if(iSecurityId.size() != iInstitutionId.size())
	{
#ifdef _DEBUG
		AfxMessageBox(_T("机构ID和交易实体ID的数量不一样"));
#endif
		return false;
	}
	int iInsCount = 0;
	std::set<int>::iterator iter1;
	for(iter1 = iSecurityId.begin();iter1!=iSecurityId.end();++iter1,iInsCount++)
	{
		//取得交易实体ID
		int TradeID = *iter1;
		GetSecurityNow(*iter1);
		if(m_pSecurity==NULL)
			continue;
		//取得机构ID
		int tempInstitutionid1 = iInstitutionId[iInsCount];
		//			iInstitutionId.insert(tempInstitutionid);
		//根据交易实体ID取得样本的名称和外码；
		CString strName,strCode;
		strName = m_pSecurity->GetName();
		strCode = m_pSecurity->GetCode();
		//把机构ID和交易实体ID对应起来。并且把交易实体ID放到表里。
		std::vector<UINT> vecInstiID2;
		m_txTable.Find(0,tempInstitutionid1,vecInstiID2);
		std::vector<UINT>::iterator iteID;
		for(iteID = vecInstiID2.begin();iteID != vecInstiID2.end();++iteID)
		{
			m_txTable.SetCell(1,*iteID,TradeID);
			m_txTable.SetCell(2,*iteID,strName);
			m_txTable.SetCell(3,*iteID,strCode);
		}
	}
	byte tempdate;
	double  vProportion10,temp10,vProportion11,temp11;
	/*std::unordered_map<int,CString> indexdat_map;
	TypeMapManage::GetInstance()->GetTypeMap(13,indexdat_map);
	std::unordered_map<int,CString>::iterator iterMap;*/
	CString strMark;
	//把股东序号放到公告日期的那一列和把持股比例和流通比例各乘以100。
	int endDate;
	for(int j = 0;j < (int)m_txTable.GetRowCount();j++)
	{
		m_txTable.GetCell(6,j,endDate);
		m_txTable.SetCell(5,j,endDate);
		m_txTable.GetCell(7,j,tempdate);
		int tempdate1 = (int) tempdate;
     	m_txTable.SetCell(6,j,tempdate1);		
        m_txTable.GetCell(11,j,temp10);
		m_txTable.GetCell(12,j,temp11);
		if (abs(temp10 - Tx::Core::Con_doubleInvalid) < 0.000001)
		{
			vProportion10 = Tx::Core::Con_doubleInvalid;
		} 
		else
		{
			vProportion10 = temp10 * 100;
			
		}
		if (abs(temp11 - Tx::Core::Con_doubleInvalid) < 0.000001)
		{
			vProportion11 = Tx::Core::Con_doubleInvalid;
		} 
		else
		{			
			vProportion11 = temp11 * 100;			
		}
		m_txTable.SetCell(11,j,vProportion10);
		m_txTable.SetCell(12,j,vProportion11);
		m_txTable.GetCell(10,j,strMark);
		strMark = strMark + _T("股");
		m_txTable.SetCell(10,j,strMark);
	}
#ifdef _DEBUG
	CString strTable4=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable4);
#endif
	m_txTable.DeleteCol(7);
	m_txTable.DeleteCol(0);
	resTable.Clear();
	resTable.Clone(m_txTable);
	ReDefColTypeDecmalToInt(resTable,7);	
//#ifdef _DEBUG
//	CString strTable3=resTable.TableToString();
//	Tx::Core::Commonality::String().StringToClipboard(strTable3);
//#endif
//	//为了添加序号，首先按照基金的交易实体ID和佣金量对resTable进行排序。
//	MultiSortRule multisort;
//	multisort.AddRule(0,true);
//	multisort.AddRule(4,false);	
//	multisort.AddRule(3,true);		
//	resTable.SortInMultiCol(multisort);
//	resTable.Arrange();
	//添加进度条
	prw.SetPercent(pid,1.0);
	return true;	
}

//板块风险分析
bool TxStock::BlockRiskIndicatorAdv(

	int iMenuID,						//功能ID
	Tx::Core::Table_Indicator& resTable,//结果数据表
	std::set<int>& iSecurityId,			//交易实体ID
	int iEndDate,						//截止日期
	long lRefoSecurityId,				//基准交易实体ID
	int iStartDate,						//起始日期
	int iTradeDaysCount,				//交易天数
	int iDuration,						//交易周期0=日；1=周；2=月；3=季；4=年
	bool bLogRatioFlag,					//计算比率的方式,true=指数比率,false=简单比率
	bool bClosePrice,					//使用收盘价
	int  iStdType,						//标准差类型1=有偏，否则无偏
	int  iOffset,						//偏移
	bool bAhead,						//复权标志,true=前复权,false=后复权
	int bUserRate,						//用户自定义年利率,否则取一年定期基准利率
	double dUserRate,					//用户自定义年利率
	bool bDayRate,						//日利率
	bool bPrw,							//是否包含进度条
	bool bMean
								 )
{
	int ii = iSecurityId.size();
	if(ii<=0)
		return false;

	bool bFirstDayInDuration = false;
	//

	/*
	IndicatorWithParameterArray arr;
	//根据功能ID建立指标信息
	IndicatorFile::GetInstance()->SetIWAP(arr, iMenuID);
	//根据指标信息建立table列
	arr.BuildTableIndicator();
	//准备提取数据
	resTable.CopyColumnInfoFrom(arr.m_table_indicator);
	*/
	RiskIndicatorAdv(resTable);

	resTable.DeleteCol(2);
	resTable.DeleteCol(1);

	return 
	RiskIndicatorAdv(
		resTable,				//结果数据表
		iSecurityId,			//交易实体ID
		iEndDate,				//截止日期
		lRefoSecurityId,		//基准交易实体ID
		bFirstDayInDuration,	//默认周期期初
		iStartDate,				//起始日期
		iTradeDaysCount,		//交易天数
		iDuration,				//交易周期0=日；1=周；2=月；3=季；4=年
		bLogRatioFlag,			//计算比率的方式,true=指数比率,false=简单比率
		bClosePrice,			//使用收盘价
		iStdType,				//标准差类型1=有偏，否则无偏
		iOffset,				//偏移
		bAhead,					//复权标志,true=前复权,false=后复权
		bUserRate,				//用户自定义年利率,否则取一年定期基准利率
		dUserRate,				//用户自定义年利率,否则取一年定期基准利率
		bDayRate,				//日利率
		bPrw,					//是否包含进度条
		bMean
		);
	return true;
}

//板块交易提示
bool TxStock:: ColligationTradePromptFPS(
			std::set<int> iSecurityId,				//交易实体ID
			int iStartDate,							//起始日
			int iEndDate,							//终止日
			Tx::Core::Table_Indicator &resTable,	//结果数据表
			bool bGetAllDate						//是否选择全部日期
				)
{	
	std::set<int> iSecurity1Id;
	if(!TransObjectToSecIns(iSecurityId,iSecurity1Id,1))
		return false;	
	//默认的返回值状态
	bool result = true;
	//清空数据
	m_txTable.Clear();
	//事件id=第一参数列:F_event_ID,int型
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
	//对象类型参数=第二参数列;int型
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);


	UINT varCfg[2];			//参数配置
	int varCount=2;//参数个数 交易实体id,公告日

	//交易提示 指标组
	//item_id: 30300081-30300101
	const int INDICATOR_INDEX=6;
	long iIndicator[INDICATOR_INDEX]=
	{			
			30300159,	//券id
			30300160,	//分红方式
			30300161,	//除权日
			30300164,	//登记日
			30300171,	//权益比率：每股（派现/送股）
			30300172	//派现金数
						//备注'公告日'+date	+'派息日'+派息日
		
	};

	
	//设定指标列
	for (int i = 0; i <INDICATOR_INDEX; i++)
	{
	    GetIndicatorDataNow(iIndicator[i]);
	
		varCfg[0]=0;
		varCfg[1]=1;

		result = m_pLogicalBusiness->SetIndicatorIntoTable(
													m_pIndicatorData,	//指标
													varCfg,				//参数配置
													varCount,			//参数个数
													m_txTable			//计算需要的参数传输载体以及计算后结果的载体
					);
		if(result==false)
			break;
	}
	if(result==false)
		return false;
	//根据之前3个步骤的设置进行数据读取，结果数据存放在table中
	result=m_pLogicalBusiness->GetData(m_txTable,true);
	if(result==false)
		return false;

	UINT	iCol=m_txTable.GetColCount();


	Tx::Core::Table_Indicator resTableDate;

	//复制所有列信息
	resTableDate.CopyColumnInfoFrom(m_txTable);
	resTable.CopyColumnInfoFrom(m_txTable);
	if(m_txTable.GetRowCount()==0)
		return true;


	UINT* nColArray = new UINT[iCol];
	for(UINT i=0;i<iCol;i++)
		nColArray[i]=i;

		
	//取得指定时间区间内的记录数据
	if(m_txTable.GetRowCount()>0)
	{
		if(!bGetAllDate)
		{
			//m_txTable.Between(resTableDate,nColArray,iCol,4,iStartDate,iEndDate,true,true);
			//2008-06-05
			//按照登记日期进行筛选
			m_txTable.Between(resTableDate,nColArray,iCol,5,iStartDate,iEndDate,true,true);
		}
		else
		{
			resTableDate.Clone(m_txTable);
		}
	}

	if(resTableDate.GetRowCount()>0)
	{
		resTableDate.EqualsAt(resTable,nColArray,iCol,2,iSecurity1Id);

	}
	delete nColArray;
	nColArray = NULL;
	return true;		
}

//综合：交易提示：发行增发
	bool TxStock::ColligationTradePromptFXZF(
			std::set<int> iSecurityId,		//交易实体ID
			int iStartDate,						//起始日
			int iEndDate,						//终止日
			Tx::Core::Table_Indicator &resTable,	//结果数据表
			bool bGetAllDate
		)
{	
	//要取回全表然后剔除无用数据
	std::set<int> iSecurity1Id;
	if(!TransObjectToSecIns(iSecurityId,iSecurity1Id,1))
		return false;	
	

	//默认的返回值状态。
	bool result = true;
	//清空数据
	m_txTable.Clear();

	//事件id=第一参数列:f_event_id,int型
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
	//券id=第二参数列;f_security_id, int型
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);




	UINT varCfg[2];			//参数配置
	int varCount=2;			//参数个数	交易实体id，公告日

	//交易提示 指标组
	//item_id: 30300081-30300101
	const int INDICATOR_INDEX=5;
	long iIndicator[INDICATOR_INDEX]=
	{			
			30300135,	//发行量
			30300136,	//发行价
			30300145,	//发行类型
			30300146,	//发行日	//疑问:似乎应该使用公告日
			30300147	//上市日
					//备注
	};

	
	//设定指标列
	for (int i = 0; i <INDICATOR_INDEX; i++)
	{
	    GetIndicatorDataNow(iIndicator[i]);
	
		varCfg[0]=0;
		varCfg[1]=1;

		result = m_pLogicalBusiness->SetIndicatorIntoTable(
													m_pIndicatorData,	//指标
													varCfg,				//参数配置
													varCount,			//参数个数
													m_txTable			//计算需要的参数传输载体以及计算后结果的载体
					);
		if(result==false)
			break;
	}
	if(result==false)
		return false;
	//根据之前3个步骤的设置进行数据读取，结果数据存放在table中
	result=m_pLogicalBusiness->GetData(m_txTable,true);
	if(result==false)
		return false;

	UINT	iCol=m_txTable.GetColCount();

#ifdef _DEBUG
//2008-06-05 by zhaohj
	int iiRow = m_txTable.FindValueIndex(7000057);//由交易实体确定
	GlobalWatch::_GetInstance()->WatchHere(_T("zhaohj|| 7000057 , row=%d"),iiRow);
	iiRow = m_txTable.FindValueIndex(10006010);//由交易实体确定
	GlobalWatch::_GetInstance()->WatchHere(_T("zhaohj|| 10006010 , row=%d"),iiRow);
	iiRow = m_txTable.FindValueIndex(20046798);//由交易实体确定
	GlobalWatch::_GetInstance()->WatchHere(_T("zhaohj|| 20046798 , row=%d"),iiRow);
#endif
	Tx::Core::Table_Indicator resTableDate,resTableType;

	//复制所有列信息
	resTableDate.CopyColumnInfoFrom(m_txTable);
	resTableType.CopyColumnInfoFrom(m_txTable);
	resTable.CopyColumnInfoFrom(m_txTable);
	if(m_txTable.GetRowCount()==0)
		return true;


	UINT* nColArray = new UINT[iCol];
	for(UINT i=0;i<iCol;i++)
		nColArray[i]=i;

		set<int> sType;
	if(m_txTable.GetRowCount()>0)
	{
		//确定类型为首发/增发

		sType.insert(1);
		sType.insert(15);

		m_txTable.EqualsAt(
			resTableType,	//结果表，注意列数相同，行数为0
			nColArray,		//结果列首地址
			iCol,			//结果列个数
			4,				//类型
			sType			//1为首发，15为增发
			);


	}
	//取得指定时间区间内的记录数据	
	if(resTableType.GetRowCount()>0)
	{
		if(!bGetAllDate)
		{
			resTableType.Between(resTableDate,nColArray,iCol,5,iStartDate,iEndDate,true,true);
		}
		else
		{
			resTableDate.Clone(resTableType);
		}
	}
	if(resTableDate.GetRowCount()>0)
	{
		resTableDate.EqualsAt(resTable,nColArray,iCol,1,iSecurity1Id);
	}


	delete nColArray;
	nColArray = NULL;
	return true;		
}		

//综合：交易提示：发行上市
	bool TxStock::ColligationTradePromptFXSS(
			std::set<int> iSecurityId,		//交易实体ID
			int iStartDate,						//起始日
			int iEndDate,						//终止日
			Tx::Core::Table_Indicator &resTable,	//结果数据表
			bool bGetAllDate					//全部日期
		)
{	
	//要取回全表然后剔除无用数据
	std::set<int> iSecurity1Id;
	if(!TransObjectToSecIns(iSecurityId,iSecurity1Id,1))
		return false;	
	

	//默认的返回值状态。
	bool result = true;
	//清空数据
	m_txTable.Clear();

	//事件id=第一参数列:F_Security_ID,int型
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
	//券id=第二参数列;F_DATE, int型
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);




	UINT varCfg[2];			//参数配置
	int varCount=2;			//参数个数	交易实体id，公告日

	//交易提示 指标组
	//item_id: 30300081-30300101
	const int INDICATOR_INDEX=5;
	long iIndicator[INDICATOR_INDEX]=
	{			
			30300135,	//发行量
			30300136,	//发行价
			30300145,	//发行类型
			30300146,	//发行日	//疑问:似乎应该使用公告日
			30300147	//上市日
					//备注
	};

	
	//设定指标列
	for (int i = 0; i <INDICATOR_INDEX; i++)
	{
	    GetIndicatorDataNow(iIndicator[i]);
	
		varCfg[0]=0;
		varCfg[1]=1;

		result = m_pLogicalBusiness->SetIndicatorIntoTable(
													m_pIndicatorData,	//指标
													varCfg,				//参数配置
													varCount,			//参数个数
													m_txTable			//计算需要的参数传输载体以及计算后结果的载体
					);
		if(result==false)
			break;
	}
	if(result==false)
		return false;
	//根据之前3个步骤的设置进行数据读取，结果数据存放在table中
	result=m_pLogicalBusiness->GetData(m_txTable,true);
	if(result==false)
		return false;

	UINT	iCol=m_txTable.GetColCount();


	Tx::Core::Table_Indicator resTableDate,resTableType;

	//复制所有列信息
	resTableDate.CopyColumnInfoFrom(m_txTable);
	resTableType.CopyColumnInfoFrom(m_txTable);
	resTable.CopyColumnInfoFrom(m_txTable);
	if(m_txTable.GetRowCount()==0)
		return true;


	UINT* nColArray = new UINT[iCol];
	for(UINT i=0;i<iCol;i++)
		nColArray[i]=i;

		set<int> sType;
	if(m_txTable.GetRowCount()>0)
	{
		//确定类型为上市

		sType.insert(2);
		m_txTable.EqualsAt(resTableType,nColArray,iCol,4,sType);
	}
	//取得指定时间区间内的记录数据	
	if(resTableType.GetRowCount()>0)
	{
		if(!bGetAllDate)
		{
			resTableType.Between(resTableDate,nColArray,iCol,6,iStartDate,iEndDate,true,true);
		}
		else
		{
			resTableDate.Clone(resTableType);	
		}
	}

	if(resTableDate.GetRowCount()>0)
	{
		resTableDate.EqualsAt(resTable,nColArray,iCol,1,iSecurity1Id);

	}
	delete nColArray;
	nColArray = NULL;
	return true;		
}		
//综合：交易提示：配股
	bool TxStock::ColligationTradePromptPG(
			std::set<int> iSecurityId,		//交易实体ID
			int iStartDate,						//起始日
			int iEndDate,						//终止日
			Tx::Core::Table_Indicator &resTable,	//结果数据表
			bool bGetAllDate,			//全部日期
			bool bIsFXRQ,				//按发行日期统计
			bool bIsSSRQ				//按上市日期统计
			)
{
	//要取回全表然后剔除无用数据
	std::set<int> iSecurity1Id;
	if(!TransObjectToSecIns(iSecurityId,iSecurity1Id,1))
		return false;	
	


	//默认的返回值状态。
	bool result = true;
	//清空数据
	m_txTable.Clear();

	//事件id,f_event_id,int型
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
	//券id,f_security_id,int型
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);




	UINT varCfg[2];			//参数配置
	int varCount=2;			//参数个数	交易实体id，公告日

	
	const int INDICATOR_INDEX=9;
	long iIndicator[INDICATOR_INDEX]=
	{			
			30300156,	//modify by lijw 2008-12-5		
			30300150,	//除权日
			30300151,	//募集资金
			30300152,	//实际配股数
			30300153,	//配股价
			30300155,	//配股方式
			30300158,	//配股费用合计
			30300198,	//配股比例
            30300157	//配股的上市日期
	};

	

	
	//设定指标列
	for (int i = 0; i <	INDICATOR_INDEX; i++)
	{
	    GetIndicatorDataNow(iIndicator[i]);
	
		varCfg[0]=0;
		varCfg[1]=1;

		result = m_pLogicalBusiness->SetIndicatorIntoTable(
													m_pIndicatorData,	//指标
													varCfg,				//参数配置
													varCount,			//参数个数
													m_txTable			//计算需要的参数传输载体以及计算后结果的载体
					);
		if(result==false)
			break;
	}
	if(result==false)
		return false;
	//根据之前3个步骤的设置进行数据读取，结果数据存放在table中
	result=m_pLogicalBusiness->GetData(m_txTable,true);
	if(result==false)
		return false;

	UINT	iCol=m_txTable.GetColCount();

	Tx::Core::Table_Indicator resTableDate;

	//复制所有列信息
	resTableDate.CopyColumnInfoFrom(m_txTable);
	resTable.CopyColumnInfoFrom(m_txTable);
	if(m_txTable.GetRowCount()==0)
		return true;


	UINT* nColArray = new UINT[iCol];
	for(UINT i=0;i<iCol;i++)
		nColArray[i]=i;
	

	
	//取得指定时间区间内的记录数据	
	if(m_txTable.GetRowCount()>0)
	{
		if(!bGetAllDate)
		{
			if(bIsFXRQ)
			{
				m_txTable.Between(resTableDate,nColArray,iCol,2,iStartDate,iEndDate,true,true);
			}
			else if(bIsSSRQ)
			{
				m_txTable.Between(resTableDate,nColArray,iCol,10,iStartDate,iEndDate,true,true);
			}
			else//是按除权日计算的。
				m_txTable.Between(resTableDate,nColArray,iCol,3,iStartDate,	iEndDate,true,true);
			
		}
		else
		{
			resTableDate.Clone(m_txTable);
		}
		
	}
			
	//取得指定实体id(iSecurityId)
	if(resTableDate.GetRowCount()>0)
	{
		resTableDate.EqualsAt(resTable,nColArray,iCol,1,iSecurity1Id);
	}

	delete nColArray;
	nColArray = NULL;
	return true;		
}

//专项统计：发行融资
//Modify by lijw 		2007-01-28
	bool TxStock::StatIssueFinancing(
			std::set<int> & iSecurityId,			//机构样本
			int iStartDate,							//起始日期
			int iEndDate,						//结束日期
			Tx::Core::Table_Indicator &resTable,	//结果数据表
			int iFXRQ,
			bool bGetAllDate,						//是否全部日期
			UINT uFlagType					//标志位	1:首发	2:配股	4:增发	8:网下定价，网上发行（界面上还没有此参数，是否使用还待考虑）
			)
{
	//添加进度条
//	ProgressWnd* pwd=Tx::Core::ProgressWnd::GetInstance();
	Tx::Core::ProgressWnd prw;
	UINT pid=prw.AddItem(1,_T("发行融资统计..."),0.0);
	prw.Show(1000);

	//要取回全表然后剔除无用数据
	std::set<int> iSecurity1Id;	
	if(!TransObjectToSecIns(iSecurityId,iSecurity1Id,1))
		return false;	

	//默认的返回值状态。
	bool result = true;
	//清空数据
	m_txTable.Clear();

	//事件idf_event_id=int型
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
	//券idF_Security_ID,int型,
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);

	UINT varCfg[2];			//参数配置
	int varCount=2;			//参数个数	
    //添加指标
	const int INDICATOR_INDEX=8;
	long iIndicator[INDICATOR_INDEX]=
	{	
		30300145,	//发行类型
		30300144,	//发行方式
		30300146,	//发行日期
		30300147,	//上市日期
		30300136,	//发行价
		30300135,	//发行量
		30300138,	//发行市值
		30300137,	//发行费用
	};

	
	//设定指标列
	for (int i = 0; i <	INDICATOR_INDEX; i++)
	{
	    GetIndicatorDataNow(iIndicator[i]);
	
		varCfg[0]=0;
		varCfg[1]=1;

		result = m_pLogicalBusiness->SetIndicatorIntoTable(
													m_pIndicatorData,	//指标
													varCfg,				//参数配置
													varCount,			//参数个数
													m_txTable			//计算需要的参数传输载体以及计算后结果的载体
					);
		if(result==false)
			break;
	}
	if(result==false)
		return false;
	//根据之前3个步骤的设置进行数据读取，结果数据存放在table中
	result=m_pLogicalBusiness->GetData(m_txTable,true);
	if(result==false)
		return false;

	UINT iCol=m_txTable.GetColCount();


	Tx::Core::Table_Indicator resTableDate,resTableType;

	//复制所有列信息
	resTableType.CopyColumnInfoFrom(m_txTable);
	resTableDate.CopyColumnInfoFrom(m_txTable);
	resTable.CopyColumnInfoFrom(m_txTable);
	
	UINT* nColArray = new UINT[iCol];
	for(UINT i=0;i<iCol;i++)
		nColArray[i]=i;

	//取得指定时间区间内的记录数据	
	if(m_txTable.GetRowCount()>0)
	{
		if(!bGetAllDate)
		{
			if(iFXRQ == 0)//按发行日期统计
			{
				m_txTable.Between(resTableDate,	nColArray,iCol,4,iStartDate,iEndDate,true,true);
			}
			else//按上市日期统计
			{
                m_txTable.Between(resTableDate,	nColArray,iCol,5,iStartDate,iEndDate,true,true);
			}
		}
		else
		{
			resTableDate.Clone(m_txTable);
		}
	}

	//根据界面给定类型条件剔除
	////标志位	1:首发	2:配股	4:增发//这些是从界面上得到的.
    //modify by lijw 2008-1-28

	if(uFlagType < 7)
	{
		std::set<int>	setType;

		if(uFlagType&1)
		{
			setType.insert(1);
			setType.insert(2);//因为网下旬价、网上定价也是首发的子类，所以当选首发时，也把它显示出来。
		}
		if(uFlagType&4)
			setType.insert(15);

		if(setType.size()!=0)
			resTableDate.EqualsAt(	
			resTableType,	//结果表(仅添加了列，行数必须为0)
			nColArray,		//结果列数组首址
			iCol,			//结果列个数
			2,				//这里是发行类型列索引
			setType
			);

	}


	if(uFlagType==7)
	{
		resTableType.Clone(resTableDate);
	}


	//取得指定实体id(iSecurityId)
	if(resTableType.GetRowCount()>0)
	{
		//取得指定实体id(iSecurityId)
			resTableType.EqualsAt(
				resTable,	//结果表，注意列数相同，行数为0
				nColArray,		//结果列首地址
				iCol,			//结果列个数
				1,				//条件列号
				iSecurity1Id		//
			);

	}
	resTable.InsertCol(2,Tx::Core::dtype_val_string);	//2名称
	resTable.InsertCol(3,Tx::Core::dtype_val_string);	//3代码
	resTable.InsertCol(5,Tx::Core::dtype_val_string);	//5将发行类型变为名称
	resTable.InsertCol(7,Tx::Core::dtype_val_string);	//7将发行方式变为名称
	resTable.InsertCol(13,Tx::Core::dtype_decimal);	//13募集资金
	double price1,price2,price3;//发行费用，发行市值，募集资金
	for(UINT k=0;k<resTable.GetRowCount();k++)
	{
		resTable.GetCell(12,k,price2);
		resTable.GetCell(14,k,price1);
		if(price1>=0)
			price3=price2-price1;
		else
			price3=price2;
		resTable.SetCell(13,k,price3);
	}
	//下面的代码是为了取发行方式和发行类型的值作准备的.
	TypeMapManage * pType = TypeMapManage::GetInstance();
	//加入配股表
	if(uFlagType&2)	
	{
		Tx::Core::Table_Indicator resTablePG;
		if(iFXRQ == 0)
			this->ColligationTradePromptPG(iSecurityId,iStartDate,iEndDate,resTablePG,bGetAllDate,true,false);
		else
			this->ColligationTradePromptPG(iSecurityId,iStartDate,iEndDate,resTablePG,bGetAllDate,false,true);
		//当前时候，结果表-分红送股表的长度、配股表的长度
		UINT iResRowCount=resTable.GetRowCount();	
		UINT iPGRowCount=resTablePG.GetRowCount();

		//将pg表数据添加到分红送股表
		resTable.AddRow(iPGRowCount);
		for(UINT i=0;i<iPGRowCount;i++)
		{
			int iPGId,iRegDate,iPGExDate;
			double dPGCollect,dPGActual,dPGFee,dPGPrice;
			CString sIssueType;
			resTablePG.GetCell(1,i,iPGId);//券id
			resTablePG.GetCell(2,i,iRegDate);//登记日
			resTablePG.GetCell(10,i,iPGExDate);//上市日期//modify by lijw 2008-1-28
			//resTablePG.GetCell(8,i,dPGRate);
			resTablePG.GetCell(6,i,dPGPrice);//配股价
			resTablePG.GetCell(4,i,dPGCollect);//募集资金
			resTablePG.GetCell(5,i,dPGActual);//实际配股数
			resTablePG.GetCell(8,i,dPGFee);//配股费用合计
			resTablePG.GetCell(7,i,sIssueType);//配股方式

			resTable.SetCell(1,i+iResRowCount,iPGId);//券id
//			CString str = _T("配股");
			//取得配股的发行类型的中文值。
			CString str = pType->GetDatByID(8,14);
			resTable.SetCell(5,i+iResRowCount,str);
			resTable.SetCell(8,i+iResRowCount,iRegDate);//登记日期
			resTable.SetCell(9,i+iResRowCount,iPGExDate);
			resTable.SetCell(11,i+iResRowCount,dPGActual);
			resTable.SetCell(10,i+iResRowCount,dPGPrice);
			resTable.SetCell(14,i+iResRowCount,dPGFee);
			resTable.SetCell(7,i+iResRowCount,sIssueType);
			////填充发行市值
			if(dPGActual<0)
				resTable.SetCell(12,i+iResRowCount,Tx::Core::Con_doubleInvalid);//此时无效数据
			//else if(dPGCollect > dPGActual*dPGPrice)
			//	resTable.SetCell(12,i+iResRowCount,dPGCollect);
			else
				resTable.SetCell(12,i+iResRowCount,dPGActual*dPGPrice);
			//填充募集资金
			resTable.SetCell(13,i+iResRowCount,dPGCollect);

		}
	}		

	resTable.AddCol(Tx::Core::dtype_val_string);		//15注册省份
	resTable.AddCol(Tx::Core::dtype_val_string);		//16行业代码
	//添加进度条
	prw.SetPercent(pid,0.6);
	//在程序内部转换样本ID
//	std::set<int> isecuId;
	for(std::set<int>::iterator iter=iSecurityId.begin();iter!=iSecurityId.end();iter++)
	{
		//取得交易实体ID
		int TradeID = *iter;
		GetSecurityNow(*iter);
		if(m_pSecurity==NULL)
			continue;
		//取得券ID
		int isecuId = (int)m_pSecurity->GetSecurity1Id();
//		isecuId.insert(tempInstitutionid);
		//根据交易实体ID取得样本的名称和外码；
		CString strName,strCode;
		strName = m_pSecurity->GetName();
		strCode = m_pSecurity->GetCode();
		//取得证监会行业代码和注册省份
		CString CSRCode = m_pSecurity->GetCSRCIndustryCode();
		if(CSRCode == _T("-"))
		{
			CSRCode = m_pSecurity->GetCSRCIndustryCode(1);
		}
		CString province = m_pSecurity->GetRegisterProvance();
		//把券ID和交易实体ID对应起来。并且把交易实体ID放到表里。
		std::vector<UINT> vecInstiID;
		resTable.Find(1,isecuId,vecInstiID);
		std::vector<UINT>::iterator iteID;
		for(iteID = vecInstiID.begin();iteID != vecInstiID.end();++iteID)
		{

			resTable.SetCell(1,*iteID,TradeID);
			resTable.SetCell(2,*iteID,strName);
			resTable.SetCell(3,*iteID,strCode);
			resTable.SetCell(15,*iteID,province);
			resTable.SetCell(16,*iteID,CSRCode);
		}
	}

	//    ReDefColTypeDecmalToInt(resTable,3);
	//不能用上面的那句代码，因为有的发行量的值会超过21亿，int的最大值是就是21亿，它会越界。
	resTable.InsertCol(12,Tx::Core::dtype_int8);
	//为了把发行量后面的小数去掉。以及把发行方式和发行类型中文化。
	//add by lijw 2008-12-3
	resTable.InsertCol(14,Tx::Core::dtype_int8);
	resTable.InsertCol(16,Tx::Core::dtype_int8);
	resTable.InsertCol(18,Tx::Core::dtype_int8);
	std::unordered_map<int,CString> IssueMode,IssueType;
//	TypeMapManage * pType = TypeMapManage::GetInstance();
	pType->GetTypeMap(9,IssueMode);
	pType->GetTypeMap(8,IssueType);
	std::unordered_map<int,CString>::iterator iter1,iter2;
	//CString strAccount;
	CString strMode,strType;
	double dData;
	INT64 iData;
	for(int j= 0 ;j < (int)resTable.GetRowCount();j++)
	{		
		resTable.GetCell(11,j,dData);
		if(dData < 0)
		{
			iData = Tx::Core::Con_int64Invalid;
		}
		else
		{
			iData = static_cast<INT64>(dData + 0.5);
		}
		resTable.SetCell(12,j,iData);
		resTable.GetCell(13,j,dData);
		if(dData < 0)
		{
			iData = Tx::Core::Con_int64Invalid;
		}
		else
		{
			iData = static_cast<INT64>(dData + 0.5);
		}
		resTable.SetCell(14,j,iData);

		resTable.GetCell(15,j,dData);
		if(dData < 0)
		{
			iData = Tx::Core::Con_int64Invalid;
		}
		else
		{
			iData = static_cast<INT64>(dData + 0.5);
		}
		resTable.SetCell(16,j,iData);
		resTable.GetCell(17,j,dData);
		if(dData < 0)
		{
			iData = Tx::Core::Con_int64Invalid;
		}
		else
		{
			iData = static_cast<INT64>(dData + 0.5);
		}
		resTable.SetCell(18,j,iData);

		resTable.GetCell(5,j,strType);
		if(_T("") == strType)
		{
			int flag2;//flag2暂存发行类型
			resTable.GetCell(4,j,flag2);
			iter2 = IssueType.begin();
			iter2 = IssueType.find(flag2);
			if(iter2 != IssueType.end())
			{
				strType = iter2->second;
				resTable.SetCell(5,j,strType);
			}
			else
			{
				CString strType = _T("-");
				resTable.SetCell(5,j,strType);
			}
		}
		resTable.GetCell(7,j,strMode);
		if(_T("") == strMode)
		{
			int flag1;//flag1暂存发行方式
			resTable.GetCell(6,j,flag1);	
			iter1 = IssueMode.begin();
			iter1= IssueMode.find(flag1);
			if(iter1 != IssueMode.end())
			{
				strMode = iter1->second;
				resTable.SetCell(7,j,strMode);
			}
			else
			{
				CString strMode = _T("-");
				resTable.SetCell(7,j,strMode);
			}			
		}
	}

#ifdef _DEBUG
	CString strTable3=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable3);
#endif
	resTable.DeleteCol(17);
	resTable.DeleteCol(15);
	resTable.DeleteCol(13);
    resTable.DeleteCol(11);
	resTable.DeleteCol(6);
	resTable.DeleteCol(4);
	resTable.DeleteCol(0);
	delete nColArray;
	nColArray = NULL;
	//添加进度条
	prw.SetPercent(pid,1.0);
	return true;		
}
//分配预案
//add by guanyd		2007-08-01
//modify by lijw 2008-01-31
bool TxStock::StatDistributionPlan(
			std::set<int> & iSecurityId,			//机构样本
			int iYear,							//年份
			int iReport,							//报告期
			Tx::Core::Table_Indicator &resTable	//结果数据表
//			bool bCommitted							//是否实施
			)
{
	//添加进度条
	Tx::Core::ProgressWnd prw;
	UINT pid=prw.AddItem(1,_T("分配预案统计..."),0.0);
	prw.Show(1000);

	//要取回全表然后剔除无用数据
	std::set<int> iInstitutionId;
	if(!TransObjectToSecIns(iSecurityId,iInstitutionId,2))
	{
		return false;	
	}
	
	bool result = true;//默认的返回值状态。
	m_txTable.Clear();//清空数据
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//准备样本集=第一参数列:F_Security_ID,int型
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//公告日期参数=第二参数列;F_DATE, int型

	UINT varCfg[2];			//参数配置
	int varCount=2;			//参数个数	交易实体id，公告日
	varCfg[0]=0;
	varCfg[1]=1;

	const int INDICATOR_INDEX=11;//交易提示 指标组
	long iIndicator[INDICATOR_INDEX]=
	{			
			30300067,	//财年//把这值填到分配年度那一年
			30300068,	//报告期
			30300069,	//截止日期//把这个日期填到基准日期里。
			30300072,	//分配方案类型，0:不分现金也不送股: 1:送现金，2:送股，3:现金+送股
			30300073,	//送股数	
			30300074,	//送红股数
			30300075,	//公积金转增
			30300076,	//派现金数
			30300077,	//派现金总额
//			30300078,	//基准股本日
			30300080,	//备注
			30300079	//数据来源
	};
	//设定指标列
	for (int i = 0; i <	INDICATOR_INDEX; i++)
	{

		GetIndicatorDataNow(iIndicator[i]);

		result = m_pLogicalBusiness->SetIndicatorIntoTable(
													m_pIndicatorData,	//指标
													varCfg,				//参数配置
													varCount,			//参数个数
													m_txTable);			//计算需要的参数传输载体以及计算后结果的载体
					
		if(result==false)
		{
			return false;
		}
	}

	prw.SetPercent(pid,0.1);//添加进度条

	//根据之前3个步骤的设置进行数据读取，结果数据存放在table中
	result=m_pLogicalBusiness->GetData(m_txTable,true);
	if(result==false)
	{
		return false;
	}
	
	prw.SetPercent(pid,0.4);//添加进度条

	Tx::Core::Table_Indicator resTableDate;
	resTableDate.CopyColumnInfoFrom(m_txTable);//复制所有列信息
	resTable.CopyColumnInfoFrom(m_txTable);

	UINT iCol=m_txTable.GetColCount();
	UINT* nColArray = new UINT[iCol];
	for(UINT i=0;i<iCol;i++)
	{
		nColArray[i]=i;
	}

	//过滤掉类型0
	std::set<BYTE> setType;//0:不分现金也不送股: 1:送现金，2:送股，3:现金+送股
	setType.insert(1);
	setType.insert(2);
	setType.insert(3);
	if (m_txTable.GetRowCount() > 0)
	{
		m_txTable.EqualsAt(resTableDate, nColArray, iCol, 5, setType);
	}
	
	prw.SetPercent(pid,0.6);//添加进度条

	////取得指定时间区间内的记录数据	
	//if(m_txTable.GetRowCount()>0)
	//{
	//	//2007-08-07
	//	iStartDate*=10000;
	//	iStartDate+=iEndDate;

	//	std::set<int> iDeadDate;
	//	iDeadDate.insert(iStartDate);
	//	//按照截止日期进行过滤
	//	m_txTable.EqualsAt(resTableDate,nColArray,iCol,3,iDeadDate);
	//}
    //根据报告期和分配年度进行过滤
	std::set<int> setYear;
	std::set<int> setReport;
	setYear.insert(iYear);
	if(iReport == 1231)
	{
		setReport.insert(40040009);
	}
	if(iReport == 630)
	{
		setReport.insert(40040003);
	}

	//根据分配年度进行过滤
	m_txTable.Clear();
	m_txTable.CopyColumnInfoFrom(resTableDate);
	if(resTableDate.GetRowCount()>0)
	{
		resTableDate.EqualsAt(m_txTable,nColArray,iCol,2,setYear);
	}

	//根据报告期进行过滤
	resTableDate.Clear();
	resTableDate.CopyColumnInfoFrom(m_txTable);
	if(m_txTable.GetRowCount()>0)
	{
		m_txTable.EqualsAt(resTableDate,nColArray,iCol,3,setReport);
	}
	
	prw.SetPercent(pid,0.7);//添加进度条

	//按照样本集进行过滤
	if(resTableDate.GetRowCount()>0)
	{
		resTableDate.EqualsAt(resTable,nColArray,iCol,0,iInstitutionId);
		m_txTable.Clear();
		m_txTable.Clone(resTable);

	}
	prw.SetPercent(pid,0.9);//添加进度条

	//填充数据
	resTable.InsertCol(1,Tx::Core::dtype_val_string);	//1名称
	resTable.InsertCol(2,Tx::Core::dtype_val_string);	//2代码
	resTable.AddCol(Tx::Core::dtype_val_string);		//14注册省份
	resTable.AddCol(Tx::Core::dtype_val_string);		//15行业代码
    std::set<int>::iterator iter;
	CString CSRCode,province;
	for(iter=iSecurityId.begin();iter!=iSecurityId.end();++iter)
	{
		//取得交易实体ID
		int TradeID = *iter;
		GetSecurityNow(*iter);
		if(m_pSecurity==NULL)
			continue;
		//取得券ID
		int isecuId = (int)m_pSecurity->GetInstitutionId();
		//根据交易实体ID取得样本的名称和外码；
		CString strName,strCode;
		strName = m_pSecurity->GetName();
		strCode = m_pSecurity->GetCode();
		//取得证监会行业代码和注册省份
		CSRCode = m_pSecurity->GetCSRCIndustryCode();
		if(CSRCode == _T("-"))
		{
			CSRCode = m_pSecurity->GetCSRCIndustryCode(1);
		}
		province = m_pSecurity->GetRegisterProvance();
		//把券ID和交易实体ID对应起来。并且把交易实体ID放到表里。
		std::vector<UINT> vecInstiID;
		resTable.Find(0,isecuId,vecInstiID);
		std::vector<UINT>::iterator iteID;
		for(iteID = vecInstiID.begin();iteID != vecInstiID.end();++iteID)
		{
			resTable.SetCell(0,*iteID,TradeID);
			resTable.SetCell(1,*iteID,strName);
			resTable.SetCell(2,*iteID,strCode);
			resTable.SetCell(15,*iteID,province);
			resTable.SetCell(16,*iteID,CSRCode);
		}
	}


	//清除工作
	delete nColArray;
	nColArray = NULL;

 	resTable.DeleteCol(7);
 	resTable.DeleteCol(5);
     resTable.DeleteCol(3);
	//添加进度条
	prw.SetPercent(pid,1.0);

	return true;		
}

//bool TxStock::GetPEValue(
//						 Tx::Core::Table& resultTable,// 结果数据表
//						 std::set<int> iSecurityId,			// 交易实体ID
//						 UINT nStartDate,					// 起始日期
//						 UINT nEndDate,						// 终止日期
//						 int iSJZQ,							// 时间周期 0-日,1-周,2-月,3-年
//						 int iHZYB,							// 汇总样本 0-全部,1-剔除亏损,2-剔除微利(元)
//						 double dwl,						// 剔除微利时的微利
//						 int iJQFS,							// 加权方式 0-总股本,1-流通股本
//						 int iCWSJ,							// 财务数据:报告期的选择 按位与 0xFFFF
//						 // 由高到低分别表示 指定报告期/选择报告期,年报,中报,季报
//						 // 指定报告期的报告期格式
//						 UINT nFiscalyear,					// 财年
//						 UINT nFiscalyearquater,			// 财季
//						 int iJSFF,							// 计算方法 1-简单,2-滚动,3-同比
//						 bool bClosePrice,					// 使用收盘价(true),均价(false)
//						 std::unordered_map<int,Tx::Core::Unique_Pos_Map*>* pid_posinfo_map
//						 )
//{
//	//////////////////////////////////////////////////////////////////////////
//	// 与zway约定,resultTable的第一行,除前2列外,将日期转为double值,写入
//	// 前2列分别为券id,券名称
//	/************************************************************************/
//	/* PE=每股市价*总股本/净利润                                            */
//	/* 股价---根据日期,从历史行情取收盘价(或使用均价---暂不考虑)			*/
//	/* 总股本-----总股本(股本结构变化表)(每天)---已可取得					*/
//	/* 净利润-----归属于母公司所有者的净利润(一般行业利润表)				*/
//	/************************************************************************/
//	//return false;
//	if (iSecurityId.empty())
//		return false;
//	if (nEndDate<nStartDate)
//		return false;
//	if (iSecurityId.empty())
//		return false;
//	resultTable.Clear();
//	// resultTable的列数要根据时间基准序列的个数来定
//	// 但行数可以确定,iSecurityid的个数+1(加权)
//	resultTable.AddCol(Tx::Core::dtype_val_string);//券id--000001.sz
//	resultTable.AddCol(Tx::Core::dtype_val_string);//券名称
//	Tx::Core::TxDateTime starttime=Tx::Core::TxDateTime::getCurrentTime();
//	//默认的返回值状态
//	bool result = false;
//	int iCol = 0;
//
//	double dpro=0.0;
//	Tx::Core::Table calcuTable;
////	Tx::Core::ProgressWnd* pProgressWnd = Tx::Core::ProgressWnd::GetInstance();
//	Tx::Core::ProgressWnd prw;
//	UINT pId=prw.AddItem(1,_T("市盈率计算中"));
//	prw.Show(1000);
//
//	//取时间基准
//	m_pShIndex->LoadHisTrade();
//	m_pShIndex->LoadTradeDate();
//	int iStandard=m_pShIndex->GetTradeDataCount();
//	int iStandardStartIndex=m_pShIndex->GetTradeDateIndex(nStartDate);
//	int iiDATE=m_pShIndex->GetTradeDateByIndex(iStandardStartIndex);
//	if (iiDATE<(int)nStartDate)
//		iStandardStartIndex++;
//	int iStandardEndIndex=m_pShIndex->GetTradeDateIndex(nEndDate);
//	if (-1 == iStandardStartIndex)
//	{
//		int iFirstDate = m_pShIndex->GetTradeDateOffset(nStartDate,0);
//		iStandardStartIndex = m_pShIndex->GetTradeDateIndex(iFirstDate);
//		iStandardStartIndex++;
//	}
//	if (-1 == iStandardEndIndex)
//	{
//		int iEndDate = m_pShIndex->GetTradeDateOffset(nEndDate,0);
//		if (-1 == iEndDate)
//			iStandardEndIndex = m_pShIndex->GetTradeDateIndex(m_pShIndex->GetTradeDateLatest());
//		else
//			iStandardEndIndex = m_pShIndex->GetTradeDateIndex(iEndDate);
//	}
//	if (iStandardEndIndex < iStandardStartIndex)
//	{
//		prw.SetPercent(pId,1.0);
//		return false;
//	}
//
//	//////////////////////////////////////////////////////////////////////////
//	//在此处添加时间周期选择的代码
//	//////////////////////////////////////////////////////////////////////////
//	std::vector<int> indexvec;
//	if (1 == iSJZQ)//周
//	{
//		for (int i=iStandardStartIndex;i<iStandardEndIndex;i++)
//		{
//			int iDate=m_pShIndex->GetTradeDateByIndex(i);
//			if (5 == Tx::Core::TxDateTime::GetDayOfWeek(iDate/10000,(iDate%10000)/100,iDate%100))
//				indexvec.push_back(i);
//		}
//	}
//	else if (2 == iSJZQ)//月
//	{
//		for (int i=iStandardStartIndex;i<iStandardEndIndex;i++)
//		{
//			int iDate=m_pShIndex->GetTradeDateByIndex(i);
//			if (Tx::Core::TxDateTime::IsLeapYear(iDate/10000))//闰年
//			{
//				if (131 == iDate%10000 || 229 == iDate%10000 || 331 == iDate%10000
//					|| 430 == iDate%10000 || 531 == iDate%10000 || 630 == iDate%10000 || 731 == iDate%10000
//					|| 831 == iDate%10000 || 930 == iDate%10000 || 1031 == iDate%10000 || 1130 == iDate%10000
//					|| 1231 == iDate%10000)
//					indexvec.push_back(i);
//			}
//			else
//			{
//				if (131 == iDate%10000 || 228 == iDate%10000 || 331 == iDate%10000
//					|| 430 == iDate%10000 || 531 == iDate%10000 || 630 == iDate%10000 || 731 == iDate%10000
//					|| 831 == iDate%10000 || 930 == iDate%10000 || 1031 == iDate%10000 || 1130 == iDate%10000
//					|| 1231 == iDate%10000)
//					indexvec.push_back(i);
//			}
//		}
//	}
//	else if (3 == iSJZQ)//年
//	{
//		for (int i=iStandardStartIndex;i<iStandardEndIndex;i++)
//		{
//			int iDate=m_pShIndex->GetTradeDateByIndex(i);
//			if (1231 == iDate%10000)
//				indexvec.push_back(i);
//		}
//	}
//	else
//	{
//		for (int i=iStandardStartIndex;i<iStandardEndIndex;i++)
//			indexvec.push_back(i);
//	}
//
//	// 不管何种方式,都把当天加入计算
//	indexvec.push_back(iStandardEndIndex);
//	// 列数确定
//	resultTable.AddRow();// 日期
//	for (UINT i = 0;i<indexvec.size();i++)
//	{
//		resultTable.AddCol(Tx::Core::dtype_double);
//		resultTable.SetCell(2+i,0,static_cast<double>(m_pShIndex->GetTradeDateByIndex(indexvec[i])));
//	}
//
//	//2007-11-27 cenxw add 支持双击转入行情界面
//	resultTable.AddCol(Tx::Core::dtype_int4);
//
//	std::vector<double> fz_weight(indexvec.size()),fm_weight(indexvec.size());// 分子分母加权值
//	UINT iRow=1;
//
//	// 取一次所有股本,所有利润的数据就足够了...
//	// 不用每次循环都去取一次
//	// 每次0.4秒,太耗时了
//	Tx::Core::Table_Indicator gbTable;// 取股本的table
//	gbTable.AddParameterColumn(Tx::Core::dtype_int4);
//
//	//第二参数列:日期
//	iCol++;
//	gbTable.AddParameterColumn(Tx::Core::dtype_int4);
//	//resTable.SetCell(iCol,0,20070807);
//	UINT varCfg[5];			//参数配置
//	int varCount=2;			//参数个数
//	varCfg[0]=0;
//	varCfg[1]=1;
//	varCfg[2]=2;
//	varCfg[3]=3;
//	varCfg[4]=4;
//	int igb=2;
//	int igbCol=3;
//	int iIndicator = 30300019;	//指标=总股本
//	gbTable.AddIndicatorColumn(30300019,Tx::Core::dtype_double,varCfg,2);
//	if (1 == iJQFS) // 按流通股本加权
//	{
//		gbTable.AddIndicatorColumn(30300014,Tx::Core::dtype_double,varCfg,2);
//		igb=3;
//		igbCol=4;
//	}
//	//根据之前3个步骤的设置进行数据读取，结果数据存放在table中
//	result = m_pLogicalBusiness->GetData(gbTable,true,pid_posinfo_map);
//	if(result==false)
//		return false;
//
//	UINT nArray[5];
//	nArray[0]=0;
//	nArray[1]=1;
//	nArray[2]=2;
//	nArray[3]=3;
//	nArray[4]=4;
//
//	Tx::Core::Table_Indicator AlllrTable;
//	AlllrTable.AddParameterColumn(Tx::Core::dtype_int8);
//	AlllrTable.AddIndicatorColumn(30900089,Tx::Core::dtype_decimal,nArray,1);
//	
//	result = m_pLogicalBusiness->GetData(AlllrTable,true,pid_posinfo_map);
//
//	if(result==false)
//		return false;
//
//
//	for (std::set<int>::iterator iter=iSecurityId.begin();iter!=iSecurityId.end();iter++)
//	{// for begin
//		//starttime=Tx::Core::TxDateTime::getCurrentTime();
//		GetSecurityNow(*iter);
//		if (NULL == m_pSecurity)
//			continue;
//		int qid=m_pSecurity->GetInstitutionId();
//		calcuTable.Clear();
//
//		Tx::Core::Table_Indicator tempTable;
//		tempTable.CopyColumnInfoFrom(gbTable);
//		gbTable.Between(tempTable,nArray,igbCol,0,qid,qid,true,true);
//#ifdef _DEBUG
//		CString strTable = tempTable.TableToString();
//		Tx::Core::Commonality::String().StringToClipboard(strTable);
//#endif
//		Tx::Core::MultiSortRule msr;
//		msr.AddRule(1);
//		msr.AddRule(igb);
//		tempTable.SortInMultiCol(msr);
//		bool bSuc=m_pSecurity->LoadHisTrade();
//		bSuc=m_pSecurity->LoadTradeDate();// 取得交易日期
//		
//		calcuTable.AddCol(Tx::Core::dtype_int4,true);// 券id---测试只有一个20000002
//		calcuTable.AddCol(Tx::Core::dtype_int4);	// 时间序列---从20070401到20070501的每一个交易日
//		calcuTable.AddCol(Tx::Core::dtype_double);	// 股本---对应时间序列的每个值有一个()
//		calcuTable.AddCol(Tx::Core::dtype_double);	// 流通股本---对应时间序列的每个值有一个()
//		calcuTable.AddCol(Tx::Core::dtype_float);	// 前收价--对应时间序列的每个值有一个(行情)
//		calcuTable.AddCol(Tx::Core::dtype_double);	// 净利润--对应时间序列的每个值有一个(取最近的报告期的数据)
//		calcuTable.AddCol(Tx::Core::dtype_double);	// PE值--- 股本*前收/利润
//		// 只要第1,2,6列
//
//		// 按时间序列有序
//		int j=0;
//		for (UINT i=0;i<indexvec.size();i++)
//		{
//			int iTradeDate = m_pShIndex->GetTradeDateByIndex(indexvec[i]);
//// 			// 此券可能在这个日期是停牌的,取之前的收盘价
//			int iIndex = m_pSecurity->GetTradeDateIndex(iTradeDate);
//			if (-1 == iIndex)
//			{
//				int iValidDate = m_pSecurity->GetTradeDateOffset(iTradeDate,0);
//				if (-1 == iValidDate)
//					iValidDate=m_pSecurity->GetTradeDateLatest();
//				iIndex = m_pSecurity->GetTradeDateIndex(iValidDate);
//				//if (-1==iIndex)//日期下限仍为停牌时间
//				//	iIndex = m_pSecurity->GetTradeDateIndex(iValidDate);
//
//			}
//
//
//			HisTradeData* pHisTradeData=m_pSecurity->GetTradeDataByIndex(iIndex);
//			if (NULL == pHisTradeData)
//				continue;
//			calcuTable.AddRow();
//			calcuTable.SetCell(0,j,qid);
//			calcuTable.SetCell(1,j,iTradeDate);
//			//calcuTable.SetCell(2,j,gb);//股本单独填
//			if (bClosePrice)
//				calcuTable.SetCell(4,j,pHisTradeData->Close);//收盘价
//			else
//				calcuTable.SetCell(4,j,static_cast<float>(pHisTradeData->Amount/pHisTradeData->Volume));//均价--当天的成交金额/成交量
//			//calcuTable.SetCell(5,j,jlr);//净利润 单独填
//			pHisTradeData=NULL;
//			j++;
//		}
//		std::unordered_map<UINT,double> tempGBMap;
//		std::unordered_map<UINT,double> tempLTGBMap;
//		for (UINT i=tempTable.GetRowCount();i>0;i--)
//		{
//			int iDate;
//			double dzGb=0.0;
//			double dltGb=0.0;
//			tempTable.GetCell(1,i-1,iDate);
//			tempTable.GetCell(igb,i-1,dzGb);
//			if (1 == iJQFS)
//				tempTable.GetCell(igb-1,i-1,dltGb);
//			for (UINT k=0;k<calcuTable.GetRowCount();k++)
//			{
//				int icDate;
//				calcuTable.GetCell(1,k,icDate);
//				if (icDate>=iDate)
//				{
//					tempGBMap.insert(std::make_pair(k,dzGb));//从k的位置开始,填iGb的值
//					if (1 == iJQFS)
//						tempLTGBMap.insert(std::make_pair(k,dltGb));
//					break;
//				}
//			}
//		}
//		// 填股本
//		std::unordered_map<UINT,double>::iterator itermap=tempGBMap.end();
//		for (itermap=tempGBMap.begin();itermap!=tempGBMap.end();itermap++)
//		{
//			for (UINT i=(*itermap).first;i<calcuTable.GetRowCount();i++)
//			{
//				calcuTable.SetCell(2,i,(*itermap).second);
//			}
//		}
//
//		// 填流通股本
//		itermap=tempLTGBMap.end();
//		for (itermap=tempLTGBMap.begin();itermap!=tempLTGBMap.end();itermap++)
//		{
//			for (UINT i=(*itermap).first;i<calcuTable.GetRowCount();i++)
//			{
//				calcuTable.SetCell(3,i,(*itermap).second);
//			}
//		}
//
//		///////////////////////////////////////////////////////////////////////////
//		// 取净利润
//		// 直接取
//		// 跟计算方法有关 简单,滚动,同比
//		// (f_institution_id)*10000000+f_fiscal_year*1000+f_fiscal_year_quarter%10*100+f_consolidated_nonconsolidated*10+f_consolidated_nonconsolidated
//		// 按calcutable的第二列(时间序列),判断取哪个报告期.........f_consolidated_nonconsolidated=2,3,f_consolidated_nonconsolidated=2,3
//		// f_fiscal_year_quarter= 1,3,5,9
//		// 先按照最大最小日期做一次between,以此为基准,做equalsat
//
//		INT64 min_date=0,max_date=0;
//		int iminyear=0;
//		int imaxyear=0;
//
//		iminyear = nStartDate/10000;
//		iminyear -= 2; // 方便滚动,同比等计算
//		imaxyear = nEndDate/10000;
//
//		min_date=(INT64)((INT64)qid*(INT64)10000000)+(INT64)(iminyear*1000)+(INT64)(1*100)+(INT64)(0*10)+(INT64)(0);
//		max_date=(INT64)((INT64)qid*(INT64)10000000)+(INT64)(imaxyear*1000)+(INT64)(9*100)+(INT64)(1*10)+(INT64)(1);
//
//		// 会有多余的调整或者母公司字段为1的记录
//		Tx::Core::Table_Indicator lrTable;
//		lrTable.CopyColumnInfoFrom(AlllrTable);
//		AlllrTable.Between(lrTable,nArray,2,0,min_date,max_date,true,true);
//#ifdef _DEBUG
//		strTable = lrTable.TableToString();
//		Tx::Core::Commonality::String().StringToClipboard(strTable);
//#endif
////		std::set<INT64> condiset;
//
//		// 简单的算法
//		if (1==iJSFF)
//		{
//			int k=0;
//			for (UINT i=0;i<calcuTable.GetRowCount();i++)
//			{
//				// 最新财务数据的终止日期
////				condiset.clear();
//
//				tempTable.Clear();
//				tempTable.CopyColumnInfoFrom(lrTable);
//				//std::vector<INT64> v_index;
//				//INT64 i64date[4];
//				INT64 i64date;
//				int iReportDate;
//				calcuTable.GetCell(1,i,iReportDate);
//				int iyear=0,imon=0,iquarter=0;
//				iyear = iReportDate/10000;
//				imon = iReportDate%10000;
//
//				if (1==iCWSJ>>3)//1XXX
//				{
//					iyear=nFiscalyear;
//					iquarter=nFiscalyearquater%4004000;
//				}
//				else if (4!=(iCWSJ&4)&&(2!=(iCWSJ&2))&&(1==(iCWSJ&1))) //0001--季报
//				{
//					// 季报只有1 3 季报,1月31日之前取上一年的三季报
//					if(imon<331)
//					{
//						iquarter = 5;
//						iyear-=1;
//					}
//					else if (imon<930)
//						iquarter = 1;
//					else
//						iquarter = 5;
//				}
//				else if (4!=(iCWSJ&4)&&(2==(iCWSJ&2))&&(1!=(iCWSJ&1))) //0010--中报
//				{
//					iquarter = 3;
//					if (imon<630)
//						iyear-=1;			
//				}
//				else if (4==(iCWSJ&4)&&(2!=(iCWSJ&2))&&(1!=(iCWSJ&1))) //0100--年报
//				{
//					iquarter = 9;
//					if (imon!=1231)
//						iyear -= 1;
//				}
//				else if (4!=(iCWSJ&4)&&(2==(iCWSJ&2))&&(1==(iCWSJ&1))) //0011--季报,中报 
//				{
//					if (imon<331)
//					{
//						iquarter = 5;
//						iyear -= 1;
//					}
//					else if (imon<630)
//						iquarter = 1;
//					else if (imon<930)
//						iquarter = 3;
//					else
//						iquarter = 5;
//				}
//				else if (4==(iCWSJ&4)&&(2!=(iCWSJ&2))&&(1==(iCWSJ&1))) //0101--年报,季报
//				{
//					if (imon<331)
//					{
//						iquarter = 9;
//						iyear -= 1;
//					}
//					else if (i<930)
//						iquarter = 1;
//					else if (i<1231)
//						iquarter = 5;
//					else
//						iquarter = 9;
//				}
//				else if (4==(iCWSJ&4)&&(2==(iCWSJ&2))&&(1!=(iCWSJ&1))) //0110--年报,中报
//				{
//					if (imon<630)
//					{
//						iquarter = 9;
//						iyear -= 1;
//					}
//					else if (imon<1231)
//						iquarter = 3;
//					else
//						iquarter = 9;
//				}
//				else if (4==(iCWSJ&4)&&(2==(iCWSJ&2))&&(1==(iCWSJ&1))) //0111--年报,中报,季报
//				{
//					if (imon<331)
//					{
//						iquarter = 9;
//						iyear -= 1;
//					}
//					else if (imon<630)
//						iquarter = 1;
//					else if (imon<930)
//						iquarter = 3;
//					else if (imon<1231)
//						iquarter = 5;
//					else
//						iquarter = 9;
//				}
//
//				i64date=(INT64)((INT64)qid*(INT64)10000000)+(INT64)(iyear*1000)+(INT64)(iquarter*100)+(INT64)(1*10)+(INT64)(1);
///*				i64date[0]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)(iyear*1000)+(INT64)(iquarter*100)+(INT64)(2*10)+(INT64)(2);
//				i64date[1]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)(iyear*1000)+(INT64)(iquarter*100)+(INT64)(2*10)+(INT64)(3);
//				i64date[2]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)(iyear*1000)+(INT64)(iquarter*100)+(INT64)(3*10)+(INT64)(2);
//				i64date[3]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)(iyear*1000)+(INT64)(iquarter*100)+(INT64)(3*10)+(INT64)(3);
//
//
//				condiset.insert(i64date[0]);
//				condiset.insert(i64date[1]);
//				condiset.insert(i64date[2]);
//				condiset.insert(i64date[3]);
//				lrTable.EqualsAt(tempTable,nArray,2,0,condiset);*/
//				lrTable.Between(tempTable,nArray,2,0,i64date,i64date,true,true);
//#ifdef _DEBUG
//				strTable = tempTable.TableToString();
//				Tx::Core::Commonality::String().StringToClipboard(strTable);
//#endif				
//				if (tempTable.GetRowCount()>1)
//				{
//					ASSERT(0);
//					TRACE(_T("\r\n财务数据不对,请检查\r\n"));
//				}
//				double dlr=0.0;
//				if (tempTable.GetRowCount() > 0)
//				{
//					tempTable.GetCell(1,tempTable.GetRowCount()-1,dlr);
//					switch(iquarter)
//					{
//					case 1://X4
//						dlr*=4;
//						break;
//					case 3:
//						dlr*=2;
//						break;//X2
//					case 5:///X4/3
//						dlr=dlr*4/3;
//						break;
//					case 9:
//						break;
//					}
//				}
//
//				// 存入vector
//				double dzgb=0.0;
//				double dltgb=0.0;
//				float fprice=0.0;
//				calcuTable.GetCell(2,i,dzgb);//总股本
//				//根据iHZYB选择剔除
//				if (1==iHZYB && dlr<0.0)//剔除亏损
//						dlr = 0.0;
//				else if (2==iHZYB && dlr<dwl*dzgb)//剔除微利
//					dlr = 0.0;
//
//				calcuTable.SetCell(5,i,dlr);
//				
//				if (fabs(dlr-0.0) > 0.000001)
//				{
//					calcuTable.GetCell(3,i,dltgb);//流通股本
//					calcuTable.GetCell(4,i,fprice);//收盘
//				}
//
//				if (0 == iJQFS)
//				{
//					fz_weight[k]+=dzgb*fprice;
//					fm_weight[k]+=dlr;
//				}
//				else if ( 1== iJQFS)
//				{
//					fz_weight[k]+=dltgb*fprice;
//					if (fabs(dzgb-0.0) > 0.000001) // 总股本不为0,总股本没数据时,取出来是0
//						fm_weight[k]+=(dlr*dltgb/dzgb);
//				}
//
//				k++;
//			}
//		}
//		else if (2==iJSFF) // 滚动,对于一个日期,取3个记录,当前(同简单的取法),去年年报,去年同期
//		{
//			int k=0;
//			for (UINT i=0;i<calcuTable.GetRowCount();i++)
//			{
//				// 最新财务数据的终止日期
////				condiset.clear();
//
//				tempTable.Clear();//
//				tempTable.CopyColumnInfoFrom(lrTable);
//				//std::vector<INT64> v_index;
//				//INT64 i64date[4];
//				INT64 i64date;
//				int iReportDate;
//				calcuTable.GetCell(1,i,iReportDate);
//				int iyear=0,imon=0,iquarter=0;
//				iyear = iReportDate/10000;
//				imon = iReportDate%10000;
//
//				if (1==iCWSJ>>3)//1XXX
//				{
//					iyear=nFiscalyear;
//					iquarter=nFiscalyearquater%4004000;
//				}
//				else if (4!=(iCWSJ&4)&&(2!=(iCWSJ&2))&&(1==(iCWSJ&1))) //0001--季报
//				{
//					// 季报只有1 3 季报,1月31日之前取上一年的三季报
//					if(imon<331)
//					{
//						iquarter = 5;
//						iyear-=1;
//					}
//					else if (imon<930)
//						iquarter = 1;
//					else
//						iquarter = 5;
//				}
//				else if (4!=(iCWSJ&4)&&(2==(iCWSJ&2))&&(1!=(iCWSJ&1))) //0010--中报
//				{
//					iquarter = 3;
//					if (imon<630)
//						iyear-=1;			
//				}
//				else if (4==(iCWSJ&4)&&(2!=(iCWSJ&2))&&(1!=(iCWSJ&1))) //0100--年报
//				{
//					iquarter = 9;
//					if (imon!=1231)
//						iyear -= 1;
//				}
//				else if (4!=(iCWSJ&4)&&(2==(iCWSJ&2))&&(1==(iCWSJ&1))) //0011--季报,中报 
//				{
//					if (imon<331)
//					{
//						iquarter = 5;
//						iyear -= 1;
//					}
//					else if (imon<630)
//						iquarter = 1;
//					else if (imon<930)
//						iquarter = 3;
//					else
//						iquarter = 5;
//				}
//				else if (4==(iCWSJ&4)&&(2!=(iCWSJ&2))&&(1==(iCWSJ&1))) //0101--年报,季报
//				{
//					if (imon<331)
//					{
//						iquarter = 9;
//						iyear -= 1;
//					}
//					else if (i<930)
//						iquarter = 1;
//					else if (i<1231)
//						iquarter = 5;
//					else
//						iquarter = 9;
//				}
//				else if (4==(iCWSJ&4)&&(2==(iCWSJ&2))&&(1!=(iCWSJ&1))) //0110--年报,中报
//				{
//					if (imon<630)
//					{
//						iquarter = 9;
//						iyear -= 1;
//					}
//					else if (imon<1231)
//						iquarter = 3;
//					else
//						iquarter = 9;
//				}
//				else if (4==(iCWSJ&4)&&(2==(iCWSJ&2))&&(1==(iCWSJ&1))) //0111--年报,中报,季报
//				{
//					if (imon<331)
//					{
//						iquarter = 9;
//						iyear -= 1;
//					}
//					else if (imon<630)
//						iquarter = 1;
//					else if (imon<930)
//						iquarter = 3;
//					else if (imon<1231)
//						iquarter = 5;
//					else
//						iquarter = 9;
//				}
//
//				// 现期
//				i64date=(INT64)((INT64)qid*(INT64)10000000)+(INT64)(iyear*1000)+(INT64)(iquarter*100)+(INT64)(1*10)+(INT64)(1);
///*				i64date[0]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)(iyear*1000)+(INT64)(iquarter*100)+(INT64)(2*10)+(INT64)(2);
//				i64date[1]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)(iyear*1000)+(INT64)(iquarter*100)+(INT64)(2*10)+(INT64)(3);
//				i64date[2]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)(iyear*1000)+(INT64)(iquarter*100)+(INT64)(3*10)+(INT64)(2);
//				i64date[3]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)(iyear*1000)+(INT64)(iquarter*100)+(INT64)(3*10)+(INT64)(3);
//				condiset.insert(i64date[0]);
//				condiset.insert(i64date[1]);
//				condiset.insert(i64date[2]);
//				condiset.insert(i64date[3]);
//				lrTable.EqualsAt(tempTable,nArray,2,0,condiset);*/
//				lrTable.Between(tempTable,nArray,2,0,i64date,i64date,true,true);
//#ifdef _DEBUG
//				strTable = tempTable.TableToString();
//				Tx::Core::Commonality::String().StringToClipboard(strTable);
//#endif	
//				// 理论上每个情况只有一条
//				if (tempTable.GetRowCount()>1)
//				{
//					ASSERT(0);
//					TRACE(_T("\r\n财务数据不对,请检查\r\n"));
//				}
//
//				double dlrnow=0.0,dlrlastyear=0.0,dlrlastquarter=0.0;
//				if (tempTable.GetRowCount() > 0)
//				{
//					tempTable.GetCell(1,tempTable.GetRowCount()-1,dlrnow);
//					tempTable.DeleteRow(0,tempTable.GetRowCount());
//					tempTable.Arrange();
//				}
//				//condiset.clear();
//
//				// 去年年报
//				i64date=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(9*100)+(INT64)(1*10)+(INT64)(1);
///*				i64date[0]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(9*100)+(INT64)(2*10)+(INT64)(2);
//				i64date[1]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(9*100)+(INT64)(2*10)+(INT64)(3);
//				i64date[2]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(9*100)+(INT64)(3*10)+(INT64)(2);
//				i64date[3]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(9*100)+(INT64)(3*10)+(INT64)(3);
//				condiset.insert(i64date[0]);
//				condiset.insert(i64date[1]);
//				condiset.insert(i64date[2]);
//				condiset.insert(i64date[3]);
//				lrTable.EqualsAt(tempTable,nArray,2,0,condiset);*/
//				lrTable.Between(tempTable,nArray,2,0,i64date,i64date,true,true);
//#ifdef _DEBUG
//				strTable = tempTable.TableToString();
//				Tx::Core::Commonality::String().StringToClipboard(strTable);
//#endif	
//				// 理论上每个情况只有一条
//				if (tempTable.GetRowCount()>1)
//				{
//					ASSERT(0);
//					TRACE(_T("\r\n财务数据不对,请检查\r\n"));
//				}
//
//				//double dlrnow=0.0,dlrlastyear=0.0,dlrlastquarter=0.0;
//				if (tempTable.GetRowCount() > 0)
//				{
//					tempTable.GetCell(1,tempTable.GetRowCount()-1,dlrlastyear);
//					tempTable.DeleteRow(0,tempTable.GetRowCount());
//					tempTable.Arrange();
//				}
//				//condiset.clear();
//				//去年同期
//				i64date=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(iquarter*100)+(INT64)(1*10)+(INT64)(1);
///*				i64date[0]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(iquarter*100)+(INT64)(2*10)+(INT64)(2);
//				i64date[1]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(iquarter*100)+(INT64)(2*10)+(INT64)(3);
//				i64date[2]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(iquarter*100)+(INT64)(3*10)+(INT64)(2);
//				i64date[3]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(iquarter*100)+(INT64)(3*10)+(INT64)(3);
//				condiset.insert(i64date[0]);
//				condiset.insert(i64date[1]);
//				condiset.insert(i64date[2]);
//				condiset.insert(i64date[3]);
//				lrTable.EqualsAt(tempTable,nArray,2,0,condiset);*/
//				lrTable.Between(tempTable,nArray,2,0,i64date,i64date,true,true);
//#ifdef _DEBUG
//				strTable = tempTable.TableToString();
//				Tx::Core::Commonality::String().StringToClipboard(strTable);
//#endif	
//				// 理论上每个情况只有一条
//				if (tempTable.GetRowCount()>1)
//				{
//					ASSERT(0);
//					TRACE(_T("\r\n财务数据不对,请检查\r\n"));
//				}
//
//				//double dlrnow=0.0,dlrlastyear=0.0,dlrlastquarter=0.0;
//				if (tempTable.GetRowCount() > 0)
//				{
//					tempTable.GetCell(1,tempTable.GetRowCount()-1,dlrlastquarter);
//				}
//
//				double dlr=0.0;
//				if (fabs(dlrnow-0.0)>0.000001 && fabs(dlrlastyear-0.0)>0.000001 && fabs(dlrlastquarter-0.0)>0.000001)
//					dlr=dlrnow+dlrlastyear-dlrlastquarter;
//
//				calcuTable.SetCell(5,i,dlr);
//
//				// 存入vector
//				double dzgb=0.0;
//				double dltgb=0.0;
//				float fprice=0.0;
//				calcuTable.GetCell(2,i,dzgb);//总股本
//
//				//根据iHZYB选择剔除
//				if (1==iHZYB && dlr<0.0)//剔除亏损
//					dlr = 0.0;
//				else if (2==iHZYB && dlr<dwl*dzgb)//剔除微利
//					dlr = 0.0;
//
//				calcuTable.SetCell(5,i,dlr);
//
//				if (fabs(dlr-0.0) > 0.000001)
//				{
//					calcuTable.GetCell(3,i,dltgb);//流通股本
//					calcuTable.GetCell(4,i,fprice);//收盘
//				}
//
//				if (0 == iJQFS)
//				{
//					fz_weight[k]+=dzgb*fprice;
//					fm_weight[k]+=dlr;
//				}
//				else if ( 1== iJQFS)
//				{
//					fz_weight[k]+=dltgb*fprice;
//					if (fabs(dzgb-0.0) > 0.000001) // 总股本不为0,总股本没数据时,取出来是0
//						fm_weight[k]+=(dlr*dltgb/dzgb);
//				}
//				k++;
//			}
//		}
//		else if (3 == iJSFF) // 同比
//		{
//			int k=0;
//			for (UINT i=0;i<calcuTable.GetRowCount();i++)
//			{
//				// 最新财务数据的终止日期
//				//condiset.clear();
//
//				tempTable.Clear();//
//				tempTable.CopyColumnInfoFrom(lrTable);
//				//std::vector<INT64> v_index;
//				//INT64 i64date[4];
//				INT64 i64date;
//				int iReportDate;
//				calcuTable.GetCell(1,i,iReportDate);
//				int iyear=0,imon=0,iquarter=0;
//				iyear = iReportDate/10000;
//				imon = iReportDate%10000;
//
//				if (1==iCWSJ>>3)//1XXX
//				{
//					iyear=nFiscalyear;
//					iquarter=nFiscalyearquater%4004000;
//				}
//				else if (4!=(iCWSJ&4)&&(2!=(iCWSJ&2))&&(1==(iCWSJ&1))) //0001--季报
//				{
//					// 季报只有1 3 季报,1月31日之前取上一年的三季报
//					if(imon<331)
//					{
//						iquarter = 5;
//						iyear-=1;
//					}
//					else if (imon<930)
//						iquarter = 1;
//					else
//						iquarter = 5;
//				}
//				else if (4!=(iCWSJ&4)&&(2==(iCWSJ&2))&&(1!=(iCWSJ&1))) //0010--中报
//				{
//					iquarter = 3;
//					if (imon<630)
//						iyear-=1;			
//				}
//				else if (4==(iCWSJ&4)&&(2!=(iCWSJ&2))&&(1!=(iCWSJ&1))) //0100--年报
//				{
//					iquarter = 9;
//					if (imon!=1231)
//						iyear -= 1;
//				}
//				else if (4!=(iCWSJ&4)&&(2==(iCWSJ&2))&&(1==(iCWSJ&1))) //0011--季报,中报 
//				{
//					if (imon<331)
//					{
//						iquarter = 5;
//						iyear -= 1;
//					}
//					else if (imon<630)
//						iquarter = 1;
//					else if (imon<930)
//						iquarter = 3;
//					else
//						iquarter = 5;
//				}
//				else if (4==(iCWSJ&4)&&(2!=(iCWSJ&2))&&(1==(iCWSJ&1))) //0101--年报,季报
//				{
//					if (imon<331)
//					{
//						iquarter = 9;
//						iyear -= 1;
//					}
//					else if (i<930)
//						iquarter = 1;
//					else if (i<1231)
//						iquarter = 5;
//					else
//						iquarter = 9;
//				}
//				else if (4==(iCWSJ&4)&&(2==(iCWSJ&2))&&(1!=(iCWSJ&1))) //0110--年报,中报
//				{
//					if (imon<630)
//					{
//						iquarter = 9;
//						iyear -= 1;
//					}
//					else if (imon<1231)
//						iquarter = 3;
//					else
//						iquarter = 9;
//				}
//				else if (4==(iCWSJ&4)&&(2==(iCWSJ&2))&&(1==(iCWSJ&1))) //0111--年报,中报,季报
//				{
//					if (imon<331)
//					{
//						iquarter = 9;
//						iyear -= 1;
//					}
//					else if (imon<630)
//						iquarter = 1;
//					else if (imon<930)
//						iquarter = 3;
//					else if (imon<1231)
//						iquarter = 5;
//					else
//						iquarter = 9;
//				}
//
//				i64date=(INT64)((INT64)qid*(INT64)10000000)+(INT64)(iyear*1000)+(INT64)(iquarter*100)+(INT64)(1*10)+(INT64)(1);
//				// 本期
///*				i64date[0]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)(iyear*1000)+(INT64)(iquarter*100)+(INT64)(2*10)+(INT64)(2);
//				i64date[1]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)(iyear*1000)+(INT64)(iquarter*100)+(INT64)(2*10)+(INT64)(3);
//				i64date[2]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)(iyear*1000)+(INT64)(iquarter*100)+(INT64)(3*10)+(INT64)(2);
//				i64date[3]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)(iyear*1000)+(INT64)(iquarter*100)+(INT64)(3*10)+(INT64)(3);
//				condiset.insert(i64date[0]);
//				condiset.insert(i64date[1]);
//				condiset.insert(i64date[2]);
//				condiset.insert(i64date[3]);
//				lrTable.EqualsAt(tempTable,nArray,2,0,condiset);*/
//				lrTable.Between(tempTable,nArray,2,0,i64date,i64date,true,true);
//#ifdef _DEBUG
//				strTable = tempTable.TableToString();
//				Tx::Core::Commonality::String().StringToClipboard(strTable);
//#endif	
//				// 理论上每个情况只有一条
//				if (tempTable.GetRowCount()>1)
//				{
//					ASSERT(0);
//					TRACE(_T("\r\n财务数据不对,请检查\r\n"));
//				}
//
//				double dlrnow=0.0,dlrlastyear=0.0,dlrlastquarter=0.0;
//				if (tempTable.GetRowCount() > 0)
//				{
//					tempTable.GetCell(1,tempTable.GetRowCount()-1,dlrnow);
//					tempTable.DeleteRow(0,tempTable.GetRowCount());
//					tempTable.Arrange();
//				}
//
//				i64date=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(9*100)+(INT64)(1*10)+(INT64)(1);
///*				condiset.clear();
//
//				// 去年年报
//				i64date[0]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(9*100)+(INT64)(2*10)+(INT64)(2);
//				i64date[1]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(9*100)+(INT64)(2*10)+(INT64)(3);
//				i64date[2]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(9*100)+(INT64)(3*10)+(INT64)(2);
//				i64date[3]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(9*100)+(INT64)(3*10)+(INT64)(3);
//				condiset.insert(i64date[0]);
//				condiset.insert(i64date[1]);
//				condiset.insert(i64date[2]);
//				condiset.insert(i64date[3]);
//				lrTable.EqualsAt(tempTable,nArray,2,0,condiset);*/
//				lrTable.Between(tempTable,nArray,2,0,i64date,i64date,true,true);
//#ifdef _DEBUG
//				strTable = tempTable.TableToString();
//				Tx::Core::Commonality::String().StringToClipboard(strTable);
//#endif	
//				// 理论上每个情况只有一条
//				if (tempTable.GetRowCount()>1)
//				{
//					ASSERT(0);
//					TRACE(_T("\r\n财务数据不对,请检查\r\n"));
//				}
//
//				//double dlrnow=0.0,dlrlastyear=0.0,dlrlastquarter=0.0;
//				if (tempTable.GetRowCount() > 0)
//				{
//					tempTable.GetCell(1,tempTable.GetRowCount()-1,dlrlastyear);
//					tempTable.DeleteRow(0,tempTable.GetRowCount());
//					tempTable.Arrange();
//				}
//
//				i64date=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(iquarter*100)+(INT64)(1*10)+(INT64)(1);
///*				condiset.clear();
//				//去年同期
//				i64date[0]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(iquarter*100)+(INT64)(2*10)+(INT64)(2);
//				i64date[1]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(iquarter*100)+(INT64)(2*10)+(INT64)(3);
//				i64date[2]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(iquarter*100)+(INT64)(3*10)+(INT64)(2);
//				i64date[3]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(iquarter*100)+(INT64)(3*10)+(INT64)(3);
//				condiset.insert(i64date[0]);
//				condiset.insert(i64date[1]);
//				condiset.insert(i64date[2]);
//				condiset.insert(i64date[3]);
//				lrTable.EqualsAt(tempTable,nArray,2,0,condiset);*/
//				lrTable.Between(tempTable,nArray,2,0,i64date,i64date,true,true);
//#ifdef _DEBUG
//				strTable = tempTable.TableToString();
//				Tx::Core::Commonality::String().StringToClipboard(strTable);
//#endif	
//				// 理论上每个情况只有一条
//				if (tempTable.GetRowCount()>1)
//				{
//					ASSERT(0);
//					TRACE(_T("\r\n财务数据不对,请检查\r\n"));
//				}
//
//				//double dlrnow=0.0,dlrlastyear=0.0,dlrlastquarter=0.0;
//				if (tempTable.GetRowCount() > 0)
//				{
//					tempTable.GetCell(1,tempTable.GetRowCount()-1,dlrlastquarter);
//				}
//
//				double dlr=0.0;
//				if (fabs(dlrnow-0.0)>0.000001&&fabs(dlrlastyear-0.0)>0.000001&&fabs(dlrlastquarter-0.0)>0.000001)
//					dlr=dlrnow*dlrlastyear/dlrlastquarter;
//
//				calcuTable.SetCell(5,i,0.0);
//
//				// 存入vector
//				double dzgb=0.0;
//				double dltgb=0.0;
//				float fprice=0.0;
//				calcuTable.GetCell(2,i,dzgb);//总股本
//
//				//根据iHZYB选择剔除
//				if (1==iHZYB && dlr<0.0)//剔除亏损
//					dlr = 0.0;
//				else if (2==iHZYB && dlr<dwl*dzgb)//剔除微利
//					dlr = 0.0;
//
//				calcuTable.SetCell(5,i,dlr);
//
//				if (fabs(dlr-0.0) > 0.000001)
//				{
//					calcuTable.GetCell(3,i,dltgb);//流通股本
//					calcuTable.GetCell(4,i,fprice);//收盘
//				}
//
//				if (0 == iJQFS)
//				{
//					fz_weight[k]+=dzgb*fprice;
//					fm_weight[k]+=dlr;
//				}
//				else if ( 1== iJQFS)
//				{
//					fz_weight[k]+=dltgb*fprice;
//					if (fabs(dzgb-0.0) > 0.000001) // 总股本不为0,总股本没数据时,取出来是0
//						fm_weight[k]+=(dlr*dltgb/dzgb);
//				}
//				k++;
//			}
//		}
//
//		lrTable.Clear();
//		
//
//
//		// 计算
//		// 当利润不为0.0时,做除法(第2列*第三列/第四列)
//		// 按照汇总条件,将每天的结果,计入汇总值
//		resultTable.AddRow();
//		for (UINT i=0;i<calcuTable.GetRowCount();i++)
//		{
//			double dlr=0.0;
//			double dgb=0.0;
//			float fpclose=0.0;
//			double dpe=-DBL_MAX;//无效值
//			calcuTable.GetCell(4,i,fpclose);
//			calcuTable.GetCell(2,i,dgb);
//			calcuTable.GetCell(5,i,dlr);
//			if (fabs(dlr-0.0)>0.000000001)
//			{
//				dpe=dgb*(double)fpclose/dlr;
//			}
//			calcuTable.SetCell(6,i,dpe);
//
//			// 每算一个填一个
//			resultTable.SetCell(2+i,iRow,dpe);
//		}
//		CString strName=m_pSecurity->GetName();
//		resultTable.SetCell(1,iRow,m_pSecurity->GetCode());
//		resultTable.SetCell(0,iRow,strName);
//		resultTable.SetCell(resultTable.GetColCount()-1,iRow,(int)m_pSecurity->GetId());
//
//		iRow++;
//
//		dpro+=1.0;
//		prw.SetPercent(pId,dpro/iSecurityId.size());
//	}// for end
//
//	// 加权
//	resultTable.AddRow();
//	CString strTitle;
//	if (0 == iJQFS)
//		strTitle=_T("总股本加权");
//	else if (1 == iJQFS)
//		strTitle=_T("流通股本加权");
//	resultTable.SetCell(1,iRow,strTitle);
//	resultTable.SetCell(0,iRow,Tx::Core::Con_strInvalid);
//	for (UINT i=0;i<fz_weight.size();i++)
//	{
//		if (fabs(fz_weight[i]-0.0)>0.000001 && fabs(fm_weight[i]-0.0)>0.000001)
//			resultTable.SetCell(2+i,iRow,fz_weight[i]/fm_weight[i]);
//		else
//			resultTable.SetCell(2+i,iRow,Tx::Core::Con_doubleInvalid);
//	}
//	prw.SetPercent(pId,1.0);
//	calcuTable.Clear();
//	Tx::Core::TxDateTime endtime=Tx::Core::TxDateTime::getCurrentTime();
//	TRACE("\n--------------------------------------------------------------------------------------------------------------------------------\n");
//	TRACE("统计%d只股票(PE,%d--%d)用时:%.6f",iSecurityId.size(),nStartDate,nEndDate,(endtime-starttime).GetTotalSeconds());
//	TRACE("\n--------------------------------------------------------------------------------------------------------------------------------\n");
//#ifdef _DEBUG
//	CString strTable = resultTable.TableToString();
//	Tx::Core::Commonality::String().StringToClipboard(strTable);
//#endif
//	return true;
//}
//
//bool TxStock::GetPBValue(
//						 Tx::Core::Table& resultTable,// 结果数据表
//						 std::set<int> iSecurityId,			// 交易实体ID
//						 UINT nStartDate,					// 起始日期
//						 UINT nEndDate,						// 终止日期
//						 int iSJZQ,							// 时间周期 1-日,2-周,3-月,4-年
//						 int iHZYB,							// 汇总样本 1-全部,2-剔除亏损,3-剔除微利(元)
//						 double dwl,						// 剔除微利时的微利
//						 int iJQFS,							// 加权方式 1-总股本,2-流通股本
//						 int iCWSJ,							// 财务数据:报告期的选择 按位与 0xFFFF
//															// 由高到低分别表示 指定报告期/选择报告期,年报,中报,季报
//															// 指定报告期的报告期格式
//						 UINT nFiscalyear,					// 财年
//						 UINT nFiscalyearquater,			// 财季
//						 int iJSFF,							// 计算方法 1-简单,2-滚动,3-同比
//						 bool bClosePrice,					// 使用收盘价(true),均价(false)
//						 std::unordered_map<int,Tx::Core::Unique_Pos_Map*>* pid_posinfo_map
//						 )
//{
//	if (iSecurityId.empty())
//	return false;
//	if (nEndDate<nStartDate)
//	return false;
//	if (iSecurityId.empty())
//	return false;
//	resultTable.Clear();
//	// resultTable的列数要根据时间基准序列的个数来定
//	// 但行数可以确定,iSecurityid的个数+1(加权)
//	resultTable.AddCol(Tx::Core::dtype_val_string);//券id--000001.sz
//	resultTable.AddCol(Tx::Core::dtype_val_string);//券名称
//	Tx::Core::TxDateTime starttime=Tx::Core::TxDateTime::getCurrentTime();
//	//默认的返回值状态
//	bool result = false;
//	int iCol = 0;
//	//double dCount=static_cast<double>(iSecurityId.size());
//	double dpro=0.0;
//	Tx::Core::Table calcuTable;
////	Tx::Core::ProgressWnd* pProgressWnd = Tx::Core::ProgressWnd::GetInstance();
//	Tx::Core::ProgressWnd prw;
//	UINT pId=prw.AddItem(1,_T("市净率计算中"));
//	prw.Show(1000);
//
//	//取时间基准
//	m_pShIndex->LoadHisTrade();
//	m_pShIndex->LoadTradeDate();
//	int iStandard=m_pShIndex->GetTradeDataCount();
//	int iStandardStartIndex=m_pShIndex->GetTradeDateIndex(nStartDate);
//	int iiDATE=m_pShIndex->GetTradeDateByIndex(iStandardStartIndex);
//	if (iiDATE<(int)nStartDate)
//		iStandardStartIndex++;
//	int iStandardEndIndex=m_pShIndex->GetTradeDateIndex(nEndDate);
//	if (-1 == iStandardStartIndex)
//	{
//		int iFirstDate = m_pShIndex->GetTradeDateOffset(nStartDate,0);
//		iStandardStartIndex = m_pShIndex->GetTradeDateIndex(iFirstDate);
//		iStandardStartIndex++;
//	}
//	if (-1 == iStandardEndIndex)
//	{
//		int iEndDate = m_pShIndex->GetTradeDateOffset(nEndDate,0);
//		if (-1 == iEndDate)
//			iStandardEndIndex = m_pShIndex->GetTradeDateIndex(m_pShIndex->GetTradeDateLatest());
//		else
//			iStandardEndIndex = m_pShIndex->GetTradeDateIndex(iEndDate);
//	}
//	if (iStandardEndIndex < iStandardStartIndex)
//	return false;
//
//	std::vector<int> indexvec;
//	if (1 == iSJZQ)//周
//	{
//		for (int i=iStandardStartIndex;i<iStandardEndIndex;i++)
//		{
//			int iDate=m_pShIndex->GetTradeDateByIndex(i);
//			if (5 == Tx::Core::TxDateTime::GetDayOfWeek(iDate/10000,(iDate%10000)/100,iDate%100))
//				indexvec.push_back(i);
//		}
//	}
//	else if (2 == iSJZQ)//月
//	{
//		for (int i=iStandardStartIndex;i<iStandardEndIndex;i++)
//		{
//			int iDate=m_pShIndex->GetTradeDateByIndex(i);
//			if (Tx::Core::TxDateTime::IsLeapYear(iDate/10000))//闰年
//			{
//				if (131 == iDate%10000 || 229 == iDate%10000 || 331 == iDate%10000
//					|| 430 == iDate%10000 || 531 == iDate%10000 || 630 == iDate%10000 || 731 == iDate%10000
//					|| 831 == iDate%10000 || 930 == iDate%10000 || 1031 == iDate%10000 || 1130 == iDate%10000
//					|| 1231 == iDate%10000)
//					indexvec.push_back(i);
//			}
//			else
//			{
//				if (131 == iDate%10000 || 228 == iDate%10000 || 331 == iDate%10000
//					|| 430 == iDate%10000 || 531 == iDate%10000 || 630 == iDate%10000 || 731 == iDate%10000
//					|| 831 == iDate%10000 || 930 == iDate%10000 || 1031 == iDate%10000 || 1130 == iDate%10000
//					|| 1231 == iDate%10000)
//					indexvec.push_back(i);
//			}
//		}
//	}
//	else if (3 == iSJZQ)//年
//	{
//		for (int i=iStandardStartIndex;i<iStandardEndIndex;i++)
//		{
//			int iDate=m_pShIndex->GetTradeDateByIndex(i);
//			if (1231 == iDate%10000)
//				indexvec.push_back(i);
//		}
//	}
//	else
//	{
//		for (int i=iStandardStartIndex;i<iStandardEndIndex;i++)
//			indexvec.push_back(i);
//	}
//	
//	indexvec.push_back(iStandardEndIndex);
//	// 列数确定
//	resultTable.AddRow();// 日期
//	for (UINT i = 0;i<indexvec.size();i++)
//	{
//		resultTable.AddCol(Tx::Core::dtype_double);
//		resultTable.SetCell(2+i,0,static_cast<double>(m_pShIndex->GetTradeDateByIndex(indexvec[i])));
//	}
//	
//	//2007-11-27 cenxw add table ID col 
//	resultTable.AddCol(Tx::Core::dtype_int4);
//
//	std::vector<double> fz_weight(indexvec.size()),fm_weight(indexvec.size());// 分子分母加权值
//	UINT iRow=1;
//
//
//	Tx::Core::Table_Indicator gbTable;// 取股本的table
//	//准备样本=第一参数列:券id
//	gbTable.AddParameterColumn(Tx::Core::dtype_int4);
//
//	//第二参数列:日期
//	iCol++;
//	gbTable.AddParameterColumn(Tx::Core::dtype_int4);
//	//resTable.SetCell(iCol,0,20070807);
//	UINT varCfg[5];			//参数配置
//	int varCount=2;			//参数个数
//	varCfg[0]=0;
//	varCfg[1]=1;
//	varCfg[2]=2;
//	varCfg[3]=3;
//	varCfg[4]=4;
//
//	//取股本-------第三列是股本----全部取回,不按时间做between,只按id做
//	//若按流通股本加权还需取回流通股本
//	int igb=2;
//	int igbCol=3;
//	int iIndicator = 30300019;	//指标=总股本
//	gbTable.AddIndicatorColumn(30300019,Tx::Core::dtype_double,varCfg,2);
//	if (1 == iJQFS) // 按流通股本加权
//	{
//		gbTable.AddIndicatorColumn(30300014,Tx::Core::dtype_double,varCfg,2);
//		igb=3;
//		igbCol=4;
//	}
//
//	//根据之前3个步骤的设置进行数据读取，结果数据存放在table中
//	result = m_pLogicalBusiness->GetData(gbTable,true,pid_posinfo_map);
//	if(result==false)
//		return false;
//
//
//	UINT nArray[5];
//	nArray[0]=0;
//	nArray[1]=1;
//	nArray[2]=2;
//	nArray[3]=3;
//	nArray[4]=4;
//	Tx::Core::Table_Indicator AllqyTable;
//	AllqyTable.AddParameterColumn(Tx::Core::dtype_int8);
//	AllqyTable.AddIndicatorColumn(30900067,Tx::Core::dtype_decimal,nArray,1);
//	result = m_pLogicalBusiness->GetData(AllqyTable,true,pid_posinfo_map);
//	if(result==false)
//		return false;
//
//	for (std::set<int>::iterator iter=iSecurityId.begin();iter!=iSecurityId.end();iter++)
//	{// for begin
//		GetSecurityNow(*iter);
//		if (NULL == m_pSecurity)
//			continue;
//		int qid=m_pSecurity->GetInstitutionId();
//		calcuTable.Clear();
//
//
//		// 按照界面输入做between 取得总股本序列
//		Tx::Core::Table_Indicator tempTable;
//
//		tempTable.CopyColumnInfoFrom(gbTable);
//		gbTable.Between(tempTable,nArray,igbCol,0,qid,qid,true,true);
//
//		// 多排tempTable
//		Tx::Core::MultiSortRule msr;
//		msr.AddRule(1);
//		msr.AddRule(igb);
//		tempTable.SortInMultiCol(msr);
//
//		////以下是取股价
//		////每个券都要进行下列操作
//		////还要根据日周月年来取
//		////好像还需要一个时间基准,用来统一不同的券的时间序列
//		////m_pShIndex---上证指数的指针
//		m_pSecurity->LoadHisTrade();
//		m_pSecurity->LoadTradeDate();// 取得交易日期
//
//		calcuTable.AddCol(Tx::Core::dtype_int4,true);// 券id---测试只有一个20000002
//		calcuTable.AddCol(Tx::Core::dtype_int4);	// 时间序列---从20070401到20070501的每一个交易日
//		calcuTable.AddCol(Tx::Core::dtype_double);	// 股本---对应时间序列的每个值有一个
//		calcuTable.AddCol(Tx::Core::dtype_double);	// 流通股本---对应时间序列的每个值有一个
//		calcuTable.AddCol(Tx::Core::dtype_float);	// 前收价--对应时间序列的每个值有一个(行情)
//		calcuTable.AddCol(Tx::Core::dtype_double);	// 净资产--对应时间序列的每个值有一个(取最近的报告期的数据)
//		calcuTable.AddCol(Tx::Core::dtype_double);	// PB值--- 股本*前收/利润
//		// 只要第1,2,6列
//
//		// 按时间序列有序
//		int j=0;
//		for (UINT i=0;i<indexvec.size();i++)
//		{
//			int iTradeDate = m_pShIndex->GetTradeDateByIndex(indexvec[i]);
//			// 			// 此券可能在这个日期是停牌的,取之前的收盘价
//			int iIndex = m_pSecurity->GetTradeDateIndex(iTradeDate);
//			if (-1 == iIndex)
//			{
//				int iValidDate = m_pSecurity->GetTradeDateOffset(iTradeDate,0);
//				iIndex = m_pSecurity->GetTradeDateIndex(iValidDate);
//				//iIndex++;
//			}
//
//
//			HisTradeData* pHisTradeData=m_pSecurity->GetTradeDataByIndex(iIndex);
//			calcuTable.AddRow();
//			calcuTable.SetCell(0,j,qid);
//			calcuTable.SetCell(1,j,iTradeDate);
//			if (bClosePrice)
//				calcuTable.SetCell(4,j,pHisTradeData->Close);//收盘价
//			else
//				calcuTable.SetCell(4,j,static_cast<float>(pHisTradeData->Amount/pHisTradeData->Volume));//均价--当天的成交金额/成交量
//			pHisTradeData=NULL;
//			j++;
//		}
//		std::unordered_map<UINT,double> tempGBMap;
//		std::unordered_map<UINT,double> tempLTGBMap;
//		for (UINT i=tempTable.GetRowCount();i>0;i--)
//		{
//			int iDate;
//			double dzGb=0.0;
//			double dltGb=0.0;
//			tempTable.GetCell(1,i-1,iDate);
//			tempTable.GetCell(igb,i-1,dzGb);
//			if (1 == iJQFS)
//				tempTable.GetCell(igb-1,i-1,dltGb);
//			for (UINT k=0;k<calcuTable.GetRowCount();k++)
//			{
//				int icDate;
//				calcuTable.GetCell(1,k,icDate);
//				if (icDate>=iDate)
//				{
//					tempGBMap.insert(std::make_pair(k,dzGb));//从k的位置开始,填iGb的值
//					if (1 == iJQFS)
//						tempLTGBMap.insert(std::make_pair(k,dltGb));
//					break;
//				}
//			}
//		}
//		// 填股本
//		std::unordered_map<UINT,double>::iterator itermap=tempGBMap.end();
//		for (itermap=tempGBMap.begin();itermap!=tempGBMap.end();itermap++)
//		{
//			for (UINT i=(*itermap).first;i<calcuTable.GetRowCount();i++)
//			{
//				calcuTable.SetCell(2,i,(*itermap).second);
//			}
//		}
//
//		// 填流通股本
//		itermap=tempLTGBMap.end();
//		for (itermap=tempLTGBMap.begin();itermap!=tempLTGBMap.end();itermap++)
//		{
//			for (UINT i=(*itermap).first;i<calcuTable.GetRowCount();i++)
//			{
//				calcuTable.SetCell(3,i,(*itermap).second);
//			}
//		}
//
//		///////////////////////////////////////////////////////////////////////////
//		// 取净资产
//		// 直接取
//		// 跟计算方法有关 简单,滚动,同比
//		// (f_institution_id)*10000000+f_fiscal_year*1000+f_fiscal_year_quarter%10*100+f_consolidated_nonconsolidated*10+f_consolidated_nonconsolidated
//		// 按calcutable的第二列(时间序列),判断取哪个报告期.........f_consolidated_nonconsolidated=2,3,f_consolidated_nonconsolidated=2,3
//		// f_fiscal_year_quarter= 1,3,5,9
//		// 先按照最大最小日期做一次between,以此为基准,做equalsat
//
//		INT64 min_date=0,max_date=0;
//		int iminyear=0;
//		int imaxyear=0;
//
//		iminyear = nStartDate/10000;
//		iminyear -= 2; // 方便滚动,同比等计算
//		imaxyear = nEndDate/10000;
//
//		//min_date=(INT64)((INT64)qid*(INT64)10000000)+(INT64)(iminyear*1000)+(INT64)(1*100)+(INT64)(2*10)+(INT64)(2);
//		//max_date=(INT64)((INT64)qid*(INT64)10000000)+(INT64)(imaxyear*1000)+(INT64)(9*100)+(INT64)(3*10)+(INT64)(3);
//		min_date=(INT64)((INT64)qid*(INT64)10000000)+(INT64)(iminyear*1000)+(INT64)(1*100)+(INT64)(1*10)+(INT64)(1);
//		max_date=(INT64)((INT64)qid*(INT64)10000000)+(INT64)(imaxyear*1000)+(INT64)(9*100)+(INT64)(1*10)+(INT64)(1);
//
//		// 会有多余的调整或者母公司字段为1的记录
//
//		Tx::Core::Table_Indicator qyTable;
//		qyTable.CopyColumnInfoFrom(AllqyTable);
//		AllqyTable.Between(qyTable,nArray,2,0,min_date,max_date,true,true);
////		std::set<INT64> condiset;
//
//		// 简单的算法
//		if (1==iJSFF)
//		{
//			int k=0;
//			for (UINT i=0;i<calcuTable.GetRowCount();i++)
//			{
//				// 最新财务数据的终止日期
//				//condiset.clear();
//
//				tempTable.Clear();
//				tempTable.CopyColumnInfoFrom(qyTable);
//				//std::vector<INT64> v_index;
//				//INT64 i64date[4];
//				INT64 i64date;
//				int iReportDate;
//				calcuTable.GetCell(1,i,iReportDate);
//				int iyear=0,imon=0,iquarter=0;
//				iyear = iReportDate/10000;
//				imon = iReportDate%10000;
//
//				if (1==iCWSJ>>3)//1XXX
//				{
//					iyear=nFiscalyear;
//					iquarter=nFiscalyearquater%4004000;
//				}
//				else if (4!=(iCWSJ&4)&&(2!=(iCWSJ&2))&&(1==(iCWSJ&1))) //0001--季报
//				{
//					// 季报只有1 3 季报,1月31日之前取上一年的三季报
//					if(imon<331)
//					{
//						iquarter = 5;
//						iyear-=1;
//					}
//					else if (imon<930)
//						iquarter = 1;
//					else
//						iquarter = 5;
//				}
//				else if (4!=(iCWSJ&4)&&(2==(iCWSJ&2))&&(1!=(iCWSJ&1))) //0010--中报
//				{
//					iquarter = 3;
//					if (imon<630)
//						iyear-=1;			
//				}
//				else if (4==(iCWSJ&4)&&(2!=(iCWSJ&2))&&(1!=(iCWSJ&1))) //0100--年报
//				{
//					iquarter = 9;
//					if (imon!=1231)
//						iyear -= 1;
//				}
//				else if (4!=(iCWSJ&4)&&(2==(iCWSJ&2))&&(1==(iCWSJ&1))) //0011--季报,中报 
//				{
//					if (imon<331)
//					{
//						iquarter = 5;
//						iyear -= 1;
//					}
//					else if (imon<630)
//						iquarter = 1;
//					else if (imon<930)
//						iquarter = 3;
//					else
//						iquarter = 5;
//				}
//				else if (4==(iCWSJ&4)&&(2!=(iCWSJ&2))&&(1==(iCWSJ&1))) //0101--年报,季报
//				{
//					if (imon<331)
//					{
//						iquarter = 9;
//						iyear -= 1;
//					}
//					else if (i<930)
//						iquarter = 1;
//					else if (i<1231)
//						iquarter = 5;
//					else
//						iquarter = 9;
//				}
//				else if (4==(iCWSJ&4)&&(2==(iCWSJ&2))&&(1!=(iCWSJ&1))) //0110--年报,中报
//				{
//					if (imon<630)
//					{
//						iquarter = 9;
//						iyear -= 1;
//					}
//					else if (imon<1231)
//						iquarter = 3;
//					else
//						iquarter = 9;
//				}
//				else if (4==(iCWSJ&4)&&(2==(iCWSJ&2))&&(1==(iCWSJ&1))) //0111--年报,中报,季报
//				{
//					if (imon<331)
//					{
//						iquarter = 9;
//						iyear -= 1;
//					}
//					else if (imon<630)
//						iquarter = 1;
//					else if (imon<930)
//						iquarter = 3;
//					else if (imon<1231)
//						iquarter = 5;
//					else
//						iquarter = 9;
//				}
//
//				i64date=(INT64)((INT64)qid*(INT64)10000000)+(INT64)(iyear*1000)+(INT64)(iquarter*100)+(INT64)(1*10)+(INT64)(1);
///*				i64date[0]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)(iyear*1000)+(INT64)(iquarter*100)+(INT64)(2*10)+(INT64)(2);
//				i64date[1]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)(iyear*1000)+(INT64)(iquarter*100)+(INT64)(2*10)+(INT64)(3);
//				i64date[2]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)(iyear*1000)+(INT64)(iquarter*100)+(INT64)(3*10)+(INT64)(2);
//				i64date[3]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)(iyear*1000)+(INT64)(iquarter*100)+(INT64)(3*10)+(INT64)(3);
//
//
//				condiset.insert(i64date[0]);
//				condiset.insert(i64date[1]);
//				condiset.insert(i64date[2]);
//				condiset.insert(i64date[3]);
//				qyTable.EqualsAt(tempTable,nArray,2,0,condiset);*/
//				qyTable.Between(tempTable,nArray,2,0,i64date,i64date,true,true);
//				if (tempTable.GetRowCount()>1)
//				{
//					ASSERT(0);
//					TRACE(_T("\r\n财务数据不对,请检查\r\n"));
//				}
//
//				double dqy=0.0;
//				if (tempTable.GetRowCount() > 0)
//					tempTable.GetCell(1,tempTable.GetRowCount()-1,dqy);
//
//
//				// 存入vector
//				double dzgb=0.0;
//				double dltgb=0.0;
//				float fprice=0.0;
//				calcuTable.GetCell(2,i,dzgb);//总股本
//				//根据iHZYB选择剔除
//				if (1==iHZYB && dqy<0.0)//剔除亏损
//					dqy = 0.0;
//				else if (2==iHZYB && dqy<dwl*dzgb)//剔除微利
//					dqy = 0.0;
//
//				calcuTable.SetCell(5,i,dqy);
//
//				if (fabs(dqy-0.0) > 0.000001)
//				{
//					calcuTable.GetCell(3,i,dltgb);//流通股本
//					calcuTable.GetCell(4,i,fprice);//收盘
//				}
//
//				if (0 == iJQFS)
//				{
//					fz_weight[k]+=dzgb*fprice;
//					fm_weight[k]+=dqy;
//				}
//				else if ( 1== iJQFS)
//				{
//					fz_weight[k]+=dltgb*fprice;
//					if (fabs(dzgb-0.0) > 0.000001) // 总股本不为0,总股本没数据时,取出来是0
//						fm_weight[k]+=(dqy*dltgb/dzgb);
//				}
//				k++;
//			}
//		}
//		else if (2==iJSFF) // 滚动,对于一个日期,取3个记录,当前(同简单的取法),去年年报,去年同期
//		{
//			int k=0;
//			for (UINT i=0;i<calcuTable.GetRowCount();i++)
//			{
//				// 最新财务数据的终止日期
//				//condiset.clear();
//
//				tempTable.Clear();//
//				tempTable.CopyColumnInfoFrom(qyTable);
//				//std::vector<INT64> v_index;
//				//INT64 i64date[4];
//				INT64 i64date;
//				int iReportDate;
//				calcuTable.GetCell(1,i,iReportDate);
//				int iyear=0,imon=0,iquarter=0;
//				iyear = iReportDate/10000;
//				imon = iReportDate%10000;
//
//				if (1==iCWSJ>>3)//1XXX
//				{
//					iyear=nFiscalyear;
//					iquarter=nFiscalyearquater%4004000;
//				}
//				else if (4!=(iCWSJ&4)&&(2!=(iCWSJ&2))&&(1==(iCWSJ&1))) //0001--季报
//				{
//					// 季报只有1 3 季报,1月31日之前取上一年的三季报
//					if(imon<331)
//					{
//						iquarter = 5;
//						iyear-=1;
//					}
//					else if (imon<930)
//						iquarter = 1;
//					else
//						iquarter = 5;
//				}
//				else if (4!=(iCWSJ&4)&&(2==(iCWSJ&2))&&(1!=(iCWSJ&1))) //0010--中报
//				{
//					iquarter = 3;
//					if (imon<630)
//						iyear-=1;			
//				}
//				else if (4==(iCWSJ&4)&&(2!=(iCWSJ&2))&&(1!=(iCWSJ&1))) //0100--年报
//				{
//					iquarter = 9;
//					if (imon!=1231)
//						iyear -= 1;
//				}
//				else if (4!=(iCWSJ&4)&&(2==(iCWSJ&2))&&(1==(iCWSJ&1))) //0011--季报,中报 
//				{
//					if (imon<331)
//					{
//						iquarter = 5;
//						iyear -= 1;
//					}
//					else if (imon<630)
//						iquarter = 1;
//					else if (imon<930)
//						iquarter = 3;
//					else
//						iquarter = 5;
//				}
//				else if (4==(iCWSJ&4)&&(2!=(iCWSJ&2))&&(1==(iCWSJ&1))) //0101--年报,季报
//				{
//					if (imon<331)
//					{
//						iquarter = 9;
//						iyear -= 1;
//					}
//					else if (i<930)
//						iquarter = 1;
//					else if (i<1231)
//						iquarter = 5;
//					else
//						iquarter = 9;
//				}
//				else if (4==(iCWSJ&4)&&(2==(iCWSJ&2))&&(1!=(iCWSJ&1))) //0110--年报,中报
//				{
//					if (imon<630)
//					{
//						iquarter = 9;
//						iyear -= 1;
//					}
//					else if (imon<1231)
//						iquarter = 3;
//					else
//						iquarter = 9;
//				}
//				else if (4==(iCWSJ&4)&&(2==(iCWSJ&2))&&(1==(iCWSJ&1))) //0111--年报,中报,季报
//				{
//					if (imon<331)
//					{
//						iquarter = 9;
//						iyear -= 1;
//					}
//					else if (imon<630)
//						iquarter = 1;
//					else if (imon<930)
//						iquarter = 3;
//					else if (imon<1231)
//						iquarter = 5;
//					else
//						iquarter = 9;
//				}
//
//				// 现期
//				i64date=(INT64)((INT64)qid*(INT64)10000000)+(INT64)(iyear*1000)+(INT64)(iquarter*100)+(INT64)(1*10)+(INT64)(1);
///*				i64date[0]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)(iyear*1000)+(INT64)(iquarter*100)+(INT64)(2*10)+(INT64)(2);
//				i64date[1]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)(iyear*1000)+(INT64)(iquarter*100)+(INT64)(2*10)+(INT64)(3);
//				i64date[2]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)(iyear*1000)+(INT64)(iquarter*100)+(INT64)(3*10)+(INT64)(2);
//				i64date[3]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)(iyear*1000)+(INT64)(iquarter*100)+(INT64)(3*10)+(INT64)(3);
//				condiset.insert(i64date[0]);
//				condiset.insert(i64date[1]);
//				condiset.insert(i64date[2]);
//				condiset.insert(i64date[3]);
//				qyTable.EqualsAt(tempTable,nArray,2,0,condiset);*/
//				qyTable.Between(tempTable,nArray,2,0,i64date,i64date,true,true);
//				// 理论上每个情况只有一条
//				if (tempTable.GetRowCount()>1)
//				{
//					ASSERT(0);
//					TRACE(_T("\r\n财务数据不对,请检查\r\n"));
//				}
//
//				double dqynow=0.0,dqylastyear=0.0,dqylastquarter=0.0;
//				if (tempTable.GetRowCount() > 0)
//				{
//					tempTable.GetCell(1,tempTable.GetRowCount()-1,dqynow);
//					tempTable.DeleteRow(0,tempTable.GetRowCount());
//					tempTable.Arrange();
//				}
//				//condiset.clear();
//
//				// 去年年报
//				i64date=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(9*100)+(INT64)(1*10)+(INT64)(1);
///*				i64date[0]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(9*100)+(INT64)(2*10)+(INT64)(2);
//				i64date[1]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(9*100)+(INT64)(2*10)+(INT64)(3);
//				i64date[2]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(9*100)+(INT64)(3*10)+(INT64)(2);
//				i64date[3]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(9*100)+(INT64)(3*10)+(INT64)(3);
//				condiset.insert(i64date[0]);
//				condiset.insert(i64date[1]);
//				condiset.insert(i64date[2]);
//				condiset.insert(i64date[3]);
//				qyTable.EqualsAt(tempTable,nArray,2,0,condiset);*/
//				qyTable.Between(tempTable,nArray,2,0,i64date,i64date,true,true);
//				// 理论上每个情况只有一条
//				if (tempTable.GetRowCount()>1)
//				{
//					ASSERT(0);
//					TRACE(_T("\r\n财务数据不对,请检查\r\n"));
//				}
//
//				//double dlrnow=0.0,dlrlastyear=0.0,dlrlastquarter=0.0;
//				if (tempTable.GetRowCount() > 0)
//				{
//					tempTable.GetCell(1,tempTable.GetRowCount()-1,dqylastyear);
//					tempTable.DeleteRow(0,tempTable.GetRowCount());
//					tempTable.Arrange();
//				}
//
//				i64date=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(iquarter*100)+(INT64)(1*10)+(INT64)(1);
///*				condiset.clear();
//				//去年同期
//				i64date[0]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(iquarter*100)+(INT64)(2*10)+(INT64)(2);
//				i64date[1]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(iquarter*100)+(INT64)(2*10)+(INT64)(3);
//				i64date[2]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(iquarter*100)+(INT64)(3*10)+(INT64)(2);
//				i64date[3]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(iquarter*100)+(INT64)(3*10)+(INT64)(3);
//				condiset.insert(i64date[0]);
//				condiset.insert(i64date[1]);
//				condiset.insert(i64date[2]);
//				condiset.insert(i64date[3]);
//				qyTable.EqualsAt(tempTable,nArray,2,0,condiset);*/
//				qyTable.Between(tempTable,nArray,2,0,i64date,i64date,true,true);
//				// 理论上每个情况只有一条
//				if (tempTable.GetRowCount()>1)
//				{
//					ASSERT(0);
//					TRACE(_T("\r\n财务数据不对,请检查\r\n"));
//				}
//
//				//double dlrnow=0.0,dlrlastyear=0.0,dlrlastquarter=0.0;
//				if (tempTable.GetRowCount() > 0)
//				{
//					tempTable.GetCell(1,tempTable.GetRowCount()-1,dqylastquarter);
//				}
//
//				double dqy=0.0;
//				if (fabs(dqynow-0.0)>0.000001 && fabs(dqylastyear-0.0)>0.000001 && fabs(dqylastquarter-0.0)>0.000001)
//					dqy=dqynow+dqylastyear-dqylastquarter;
//
//				// 存入vector
//				double dzgb=0.0;
//				double dltgb=0.0;
//				float fprice=0.0;
//				calcuTable.GetCell(2,i,dzgb);//总股本
//				//根据iHZYB选择剔除
//				if (1==iHZYB && dqy<0.0)//剔除亏损
//					dqy = 0.0;
//				else if (2==iHZYB && dqy<dwl*dzgb)//剔除微利
//					dqy = 0.0;
//
//				calcuTable.SetCell(5,i,dqy);
//
//				if (fabs(dqy-0.0) > 0.000001)
//				{
//					calcuTable.GetCell(3,i,dltgb);//流通股本
//					calcuTable.GetCell(4,i,fprice);//收盘
//				}
//
//				if (0 == iJQFS)
//				{
//					fz_weight[k]+=dzgb*fprice;
//					fm_weight[k]+=dqy;
//				}
//				else if ( 1== iJQFS)
//				{
//					fz_weight[k]+=dltgb*fprice;
//					if (fabs(dzgb-0.0) > 0.000001) // 总股本不为0,总股本没数据时,取出来是0
//						fm_weight[k]+=(dqy*dltgb/dzgb);
//				}
//
//				k++;
//			}
//		}
//		else if (3 == iJSFF) // 同比
//		{
//			int k=0;
//			for (UINT i=0;i<calcuTable.GetRowCount();i++)
//			{
//				// 最新财务数据的终止日期
//				//condiset.clear();
//
//				tempTable.Clear();//
//				tempTable.CopyColumnInfoFrom(qyTable);
//				//std::vector<INT64> v_index;
//				//INT64 i64date[4];
//				INT64 i64date;
//				int iReportDate;
//				calcuTable.GetCell(1,i,iReportDate);
//				int iyear=0,imon=0,iquarter=0;
//				iyear = iReportDate/10000;
//				imon = iReportDate%10000;
//
//				if (1==iCWSJ>>3)//1XXX
//				{
//					iyear=nFiscalyear;
//					iquarter=nFiscalyearquater%4004000;
//				}
//				else if (4!=(iCWSJ&4)&&(2!=(iCWSJ&2))&&(1==(iCWSJ&1))) //0001--季报
//				{
//					// 季报只有1 3 季报,1月31日之前取上一年的三季报
//					if(imon<331)
//					{
//						iquarter = 5;
//						iyear-=1;
//					}
//					else if (imon<930)
//						iquarter = 1;
//					else
//						iquarter = 5;
//				}
//				else if (4!=(iCWSJ&4)&&(2==(iCWSJ&2))&&(1!=(iCWSJ&1))) //0010--中报
//				{
//					iquarter = 3;
//					if (imon<630)
//						iyear-=1;
//				}
//				else if (4==(iCWSJ&4)&&(2!=(iCWSJ&2))&&(1!=(iCWSJ&1))) //0100--年报
//				{
//					iquarter = 9;
//					if (imon!=1231)
//						iyear -= 1;
//				}
//				else if (4!=(iCWSJ&4)&&(2==(iCWSJ&2))&&(1==(iCWSJ&1))) //0011--季报,中报 
//				{
//					if (imon<331)
//					{
//						iquarter = 5;
//						iyear -= 1;
//					}
//					else if (imon<630)
//						iquarter = 1;
//					else if (imon<930)
//						iquarter = 3;
//					else
//						iquarter = 5;
//				}
//				else if (4==(iCWSJ&4)&&(2!=(iCWSJ&2))&&(1==(iCWSJ&1))) //0101--年报,季报
//				{
//					if (imon<331)
//					{
//						iquarter = 9;
//						iyear -= 1;
//					}
//					else if (i<930)
//						iquarter = 1;
//					else if (i<1231)
//						iquarter = 5;
//					else
//						iquarter = 9;
//				}
//				else if (4==(iCWSJ&4)&&(2==(iCWSJ&2))&&(1!=(iCWSJ&1))) //0110--年报,中报
//				{
//					if (imon<630)
//					{
//						iquarter = 9;
//						iyear -= 1;
//					}
//					else if (imon<1231)
//						iquarter = 3;
//					else
//						iquarter = 9;
//				}
//				else if (4==(iCWSJ&4)&&(2==(iCWSJ&2))&&(1==(iCWSJ&1))) //0111--年报,中报,季报
//				{
//					if (imon<331)
//					{
//						iquarter = 9;
//						iyear -= 1;
//					}
//					else if (imon<630)
//						iquarter = 1;
//					else if (imon<930)
//						iquarter = 3;
//					else if (imon<1231)
//						iquarter = 5;
//					else
//						iquarter = 9;
//				}
//
//				// 本期
//				i64date=(INT64)((INT64)qid*(INT64)10000000)+(INT64)(iyear*1000)+(INT64)(iquarter*100)+(INT64)(1*10)+(INT64)(1);
///*				i64date[0]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)(iyear*1000)+(INT64)(iquarter*100)+(INT64)(2*10)+(INT64)(2);
//				i64date[1]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)(iyear*1000)+(INT64)(iquarter*100)+(INT64)(2*10)+(INT64)(3);
//				i64date[2]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)(iyear*1000)+(INT64)(iquarter*100)+(INT64)(3*10)+(INT64)(2);
//				i64date[3]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)(iyear*1000)+(INT64)(iquarter*100)+(INT64)(3*10)+(INT64)(3);
//				condiset.insert(i64date[0]);
//				condiset.insert(i64date[1]);
//				condiset.insert(i64date[2]);
//				condiset.insert(i64date[3]);
//				qyTable.EqualsAt(tempTable,nArray,2,0,condiset);*/
//				qyTable.Between(tempTable,nArray,2,0,i64date,i64date,true,true);
//				// 理论上每个情况只有一条
//				if (tempTable.GetRowCount()>1)
//				{
//					ASSERT(0);
//					TRACE(_T("\r\n财务数据不对,请检查\r\n"));
//				}
//
//				double dqynow=0.0,dqylastyear=0.0,dqylastquarter=0.0;
//				if (tempTable.GetRowCount() > 0)
//				{
//					tempTable.GetCell(1,tempTable.GetRowCount()-1,dqynow);
//					tempTable.DeleteRow(0,tempTable.GetRowCount());
//					tempTable.Arrange();
//				}
//
//				i64date=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(9*100)+(INT64)(1*10)+(INT64)(1);
///*				condiset.clear();
//
//				// 去年年报
//				i64date[0]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(9*100)+(INT64)(2*10)+(INT64)(2);
//				i64date[1]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(9*100)+(INT64)(2*10)+(INT64)(3);
//				i64date[2]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(9*100)+(INT64)(3*10)+(INT64)(2);
//				i64date[3]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(9*100)+(INT64)(3*10)+(INT64)(3);
//				condiset.insert(i64date[0]);
//				condiset.insert(i64date[1]);
//				condiset.insert(i64date[2]);
//				condiset.insert(i64date[3]);
//				qyTable.EqualsAt(tempTable,nArray,2,0,condiset);*/
//				qyTable.Between(tempTable,nArray,2,0,i64date,i64date,true,true);
//				// 理论上每个情况只有一条
//				if (tempTable.GetRowCount()>1)
//				{
//					ASSERT(0);
//					TRACE(_T("\r\n财务数据不对,请检查\r\n"));
//				}
//
//				//double dqynow=0.0,dqylastyear=0.0,dqylastquarter=0.0;
//				if (tempTable.GetRowCount() > 0)
//				{
//					tempTable.GetCell(1,tempTable.GetRowCount()-1,dqylastyear);
//					tempTable.DeleteRow(0,tempTable.GetRowCount());
//					tempTable.Arrange();
//				}
//
//				i64date=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(iquarter*100)+(INT64)(1*10)+(INT64)(1);
///*				condiset.clear();
//				//去年同期
//				i64date[0]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(iquarter*100)+(INT64)(2*10)+(INT64)(2);
//				i64date[1]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(iquarter*100)+(INT64)(2*10)+(INT64)(3);
//				i64date[2]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(iquarter*100)+(INT64)(3*10)+(INT64)(2);
//				i64date[3]=(INT64)((INT64)qid*(INT64)10000000)+(INT64)((iyear-1)*1000)+(INT64)(iquarter*100)+(INT64)(3*10)+(INT64)(3);
//				condiset.insert(i64date[0]);
//				condiset.insert(i64date[1]);
//				condiset.insert(i64date[2]);
//				condiset.insert(i64date[3]);
//				qyTable.EqualsAt(tempTable,nArray,2,0,condiset);*/
//				qyTable.Between(tempTable,nArray,2,0,i64date,i64date,true,true);
//				// 理论上每个情况只有一条
//				if (tempTable.GetRowCount()>1)
//				{
//					ASSERT(0);
//					TRACE(_T("\r\n财务数据不对,请检查\r\n"));
//				}
//
//				//double dqynow=0.0,dqylastyear=0.0,dqylastquarter=0.0;
//				if (tempTable.GetRowCount() > 0)
//				{
//					tempTable.GetCell(1,tempTable.GetRowCount()-1,dqylastquarter);
//				}
//				double dqy=0.0;
//				if (dqynow>0.0&&dqylastyear>0.0&&dqylastquarter>0.0)
//					dqy=dqynow*dqylastyear/dqylastquarter;
//
//				// 存入vector
//				double dzgb=0.0;
//				double dltgb=0.0;
//				float fprice=0.0;
//				calcuTable.GetCell(2,i,dzgb);//总股本
//				//根据iHZYB选择剔除
//				if (1==iHZYB && dqy<0.0)//剔除亏损
//					dqy = 0.0;
//				else if (2==iHZYB && dqy<dwl*dzgb)//剔除微利
//					dqy = 0.0;
//
//				calcuTable.SetCell(5,i,dqy);
//
//				if (fabs(dqy-0.0) > 0.000001)
//				{
//					calcuTable.GetCell(3,i,dltgb);//流通股本
//					calcuTable.GetCell(4,i,fprice);//收盘
//				}
//
//				if (0 == iJQFS)
//				{
//					fz_weight[k]+=dzgb*fprice;
//					fm_weight[k]+=dqy;
//				}
//				else if ( 1== iJQFS)
//				{
//					fz_weight[k]+=dltgb*fprice;
//					if (fabs(dzgb-0.0) > 0.000001) // 总股本不为0,总股本没数据时,取出来是0
//						fm_weight[k]+=(dqy*dltgb/dzgb);
//				}
//
//				k++;
//			}
//		}
//
//		qyTable.Clear();
//		// 计算
//		// 当利润不为0.0时,做除法(第2列*第三列/第四列)
//		// 按照汇总条件,将每天的结果,计入汇总值
//		resultTable.AddRow();
//		for (UINT i=0;i<calcuTable.GetRowCount();i++)
//		{
//			double dqy=0.0;
//			double dgb=0.0;
//			float fpclose=0.0;
//			double dpe=-DBL_MAX;//无效值
//			calcuTable.GetCell(4,i,fpclose);
//			calcuTable.GetCell(2,i,dgb);
//			calcuTable.GetCell(5,i,dqy);
//			if (fabs(dqy-0.0)>0.000000001)
//			{
//				dpe=dgb*(double)fpclose/dqy;
//			}
//			calcuTable.SetCell(6,i,dpe);
//
//			// 每算一个填一个
//			resultTable.SetCell(2+i,iRow,dpe);
//		}
//		CString strName=m_pSecurity->GetName();
//		resultTable.SetCell(1,iRow,m_pSecurity->GetCode());
//		resultTable.SetCell(0,iRow,strName);
//		resultTable.SetCell(resultTable.GetColCount()-1,iRow,(int)m_pSecurity->GetId());
//		iRow++;
//
//		dpro+=1.0;
//		prw.SetPercent(pId,dpro/iSecurityId.size());
//	}// for end
//
//	// 加权
//	resultTable.AddRow();
//	CString strTitle;
//	if (0 == iJQFS)
//		strTitle=_T("总股本加权");
//	else if (1 == iJQFS)
//		strTitle=_T("流通股本加权");
//	resultTable.SetCell(1,iRow,strTitle);
//	resultTable.SetCell(0,iRow,Tx::Core::Con_strInvalid);
//	for (UINT i=0;i<fz_weight.size();i++)
//	{
//		if (fabs(fz_weight[i]-0.0)>0.000001 && fabs(fm_weight[i]-0.0)>0.000001)
//			resultTable.SetCell(2+i,iRow,fz_weight[i]/fm_weight[i]);
//		else
//			resultTable.SetCell(2+i,iRow,Tx::Core::Con_doubleInvalid);
//	}
//	prw.SetPercent(pId,1.0);
//	calcuTable.Clear();
//	Tx::Core::TxDateTime endtime=Tx::Core::TxDateTime::getCurrentTime();
//	TRACE("\n--------------------------------------------------------------------------------------------------------------------------------\n");
//	TRACE("统计%d只股票(PB,%d--%d)用时:%.6f",iSecurityId.size(),nStartDate,nEndDate,(endtime-starttime).GetTotalSeconds());
//	TRACE("\n--------------------------------------------------------------------------------------------------------------------------------\n");
//	return true;
//}
			//一般行业利润表(新会计准则)
bool TxStock::GetIncomeStatementCommercialIndustryActstd(
				Tx::Core::Table_Indicator& resTable,//结果数据表
				int iSecurityId,					//交易实体ID
				int iConsolidated,					//1=母公司;2=合并;3=母公司|合并
				int iFiscalYear,					//报告期-财年
				int iFiscalQuarter,					//报告期-财务季度
				int iAjustment						//0：既不是调整前也不是调整后 1：调整前 2：调整后  3：既是调整前也是调整后
				)
{
	/*
	//默认的返回值状态
	bool result = false;
	//清空数据
	m_txTable.Clear();
	int iCol = 0;

	//第一步从会计核算表取出会计核算ID
	//准备样本=第一参数列;添加样本-1个样本
	int i=0;
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
	m_txTable.AddRow();
	m_txTable.SetCell(iCol,0,iSecurityId);

	//第二参数列：1=母公司;2=合并;3=母公司|合并
	iCol++;
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
	m_txTable.SetCell(iCol,0,iConsolidated);

	//报告期-财年参数=第三参数列
	iCol++;
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
	m_txTable.SetCell(iCol,0,iFiscalYear);

	//报告期-财务季度参数=第四参数列
	iCol++;
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
	m_txTable.SetCell(iCol,0,iFiscalQuarter);

	//第五参数列：0：既不是调整前也不是调整后 1：调整前 2：调整后  3：既是调整前也是调整后
	iCol++;
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
	m_txTable.SetCell(iCol,0,iAjustment);

	//2007-08-07
	//指标正在填充
	int iIndicator = 30900212;	//指标=会计核算表
	UINT varCfg[5];			//参数配置
	int varCount=5;			//参数个数
	varCfg[0]=0;
	varCfg[1]=1;
	varCfg[2]=2;
	varCfg[3]=3;
	varCfg[4]=4;
	GetIndicatorDataNow(iIndicator);
	if(m_pIndicatorData==NULL)
		return false;
	result = m_pLogicalBusiness->SetIndicatorIntoTable(
		m_pIndicatorData,	//指标
		varCfg,				//参数配置
		varCount,			//参数个数
		m_txTable	//计算需要的参数传输载体以及计算后结果的载体
		);
	if(result==false)
		return false;
	//根据之前3个步骤的设置进行数据读取，结果数据存放在table中
	result = m_pLogicalBusiness->GetData(m_txTable);
	if(result==false)
		return false;

	//第二步从一般行业利润表取得数据
	//复制样本
	resTable.AppendColumn(m_txTable,0,0);
	//复制参数-eventid
	resTable.AddParameterColumn(Tx::Core::dtype_int4);
	resTable.CopyColumnData(m_txTable,5,1);

	iIndicator = 30900071;	//指标=一般行业利润表
	UINT varCfg1[1];			//参数配置
	varCfg1[0]=1;
	int varCount1=1;			//参数个数
	for(int i=0;i<47;i++)
	{
	GetIndicatorDataNow(iIndicator+i);
	if(m_pIndicatorData==NULL)
		return false;
	result = m_pLogicalBusiness->SetIndicatorIntoTable(
		m_pIndicatorData,	//指标
		varCfg1,				//参数配置
		varCount1,			//参数个数
		resTable	//计算需要的参数传输载体以及计算后结果的载体
		);
	if(result==false)
		return false;
	}
	//根据之前3个步骤的设置进行数据读取，结果数据存放在table中
	result = m_pLogicalBusiness->GetData(resTable);
	if(result==false)
		return false;
*/
	//默认的返回值状态
	bool result = false;
	//清空数据
	resTable.Clear();
	int iCol = 0;

	//特殊处理
	//将参数拼成一个INT64
	int i=0;
	INT64 Iindex = 0;
	int iInstitutionId = 0;

	GetSecurityNow(iSecurityId);
	if(m_pSecurity==NULL)
		return false;
	iInstitutionId = m_pSecurity->GetInstitutionId();
//convert(bigint,f_institution_id)*10000000+f_fiscal_year*1000+f_fiscal_year_quarter%10*100+f_consolidated_nonconsolidated*10+f_adjustment_flag as f_financial_accounting_id

	iFiscalQuarter %=40040000;
	Iindex = (INT64)iInstitutionId*10000000+(INT64)(iFiscalYear*1000)+(INT64)(iFiscalQuarter%10*100)+(INT64)(iConsolidated*10)+(INT64)iAjustment;

	resTable.AddParameterColumn(Tx::Core::dtype_int8);
	resTable.AddRow();
	resTable.SetCell(iCol,0,Iindex);

	//2007-08-07
	//指标正在填充
	int iIndicator = 30900071;	//指标=一般行业利润表
	UINT varCfg1[1];			//参数配置
	int varCount1=1;			//参数个数
	varCfg1[0]=0;
	for(int i=0;i<47;i++)
	{
	GetIndicatorDataNow(iIndicator+i);
	if(m_pIndicatorData==NULL)
		return false;
	result = m_pLogicalBusiness->SetIndicatorIntoTable(
		m_pIndicatorData,	//指标
		varCfg1,				//参数配置
		varCount1,			//参数个数
		resTable	//计算需要的参数传输载体以及计算后结果的载体
		);
	if(result==false)
		return false;
	}
	//根据之前3个步骤的设置进行数据读取，结果数据存放在table中
	result = m_pLogicalBusiness->GetData(resTable);
	if(result==false)
		return false;
	return true;
}
//主营业务产品收入
bool TxStock::GetIncomeMain(
				Tx::Core::Table_Indicator& resTable,//结果数据表
				int iSecurityId,					//交易实体ID
				int iFiscalYear,					//报告期-财年
				int iFiscalQuarter					//报告期-财务季度
				)
{
	int iConsolidated=1;					//2=合并;3=母公司|合并
	int iAjustment=1;						//2：调整后  3：既是调整前也是调整后
	//默认的返回值状态
	bool result = false;
	//清空数据
	m_txTable.Clear();
	int iCol = 0;

	//第一步从会计核算表取出会计核算ID
	//准备样本=第一参数列;添加样本-1个样本
	int i=0;
	INT64 Iindex = 0;
	INT64 Iindex1 = 0,Iindex2 = 0,Iindex3 = 0,Iindex4 = 0;
	int iInstitutionId = 0;

	GetSecurityNow(iSecurityId);
	if(m_pSecurity==NULL)
		return false;
	iInstitutionId = m_pSecurity->GetInstitutionId();
//convert(bigint,f_institution_id)*10000000+f_fiscal_year*1000+f_fiscal_year_quarter%10*100+f_consolidated_nonconsolidated*10+f_adjustment_flag as f_financial_accounting_id

	iFiscalQuarter %=40040000;
	Iindex = (INT64)iInstitutionId*10000000+(INT64)(iFiscalYear*1000)+(INT64)(iFiscalQuarter%10*100)+(INT64)(iConsolidated*10)+(INT64)iAjustment;

	/*
	iConsolidated=3;					//2=合并;3=母公司|合并
	iAjustment=3;						//2：调整后  3：既是调整前也是调整后
	Iindex1 = (INT64)iInstitutionId*10000000+(INT64)(iFiscalYear*1000)+(INT64)(iFiscalQuarter%10*100)+(INT64)(iConsolidated*10)+(INT64)iAjustment;

	iConsolidated=3;					//2=合并;3=母公司|合并
	iAjustment=2;						//2：调整后  3：既是调整前也是调整后
	Iindex2 = (INT64)iInstitutionId*10000000+(INT64)(iFiscalYear*1000)+(INT64)(iFiscalQuarter%10*100)+(INT64)(iConsolidated*10)+(INT64)iAjustment;

	iConsolidated=2;					//2=合并;3=母公司|合并
	iAjustment=2;						//2：调整后  3：既是调整前也是调整后
	Iindex3 = (INT64)iInstitutionId*10000000+(INT64)(iFiscalYear*1000)+(INT64)(iFiscalQuarter%10*100)+(INT64)(iConsolidated*10)+(INT64)iAjustment;

	iConsolidated=2;					//2=合并;3=母公司|合并
	iAjustment=3;						//2：调整后  3：既是调整前也是调整后
	Iindex4 = (INT64)iInstitutionId*10000000+(INT64)(iFiscalYear*1000)+(INT64)(iFiscalQuarter%10*100)+(INT64)(iConsolidated*10)+(INT64)iAjustment;
	//Iindex = (INT64)(iInstitutionId*10000000)+iFiscalYear*1000+iFiscalQuarter%10*100+iConsolidated*10+iAjustment;
	*/

	//组合ID
	m_txTable.AddParameterColumn(Tx::Core::dtype_int8);
	//产品分类
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);

	//resTable.AddRow();
	//resTable.SetCell(iCol,0,Iindex);


	int iIndicator = 30900214;	//指标
	UINT varCfg1[2];		//参数配置
	varCfg1[0]=0;
	varCfg1[1]=1;
	int varCount1=2;		//参数个数
	for(int i=0;i<6;i++)
	{
	GetIndicatorDataNow(iIndicator+i);
	if(m_pIndicatorData==NULL)
		return false;
	result = m_pLogicalBusiness->SetIndicatorIntoTable(
		m_pIndicatorData,	//指标
		varCfg1,				//参数配置
		varCount1,			//参数个数
		m_txTable	//计算需要的参数传输载体以及计算后结果的载体
		);
	if(result==false)
		return false;
	}
	//根据之前3个步骤的设置进行数据读取，结果数据存放在table中
	result = m_pLogicalBusiness->GetData(m_txTable,true);
	if(result==false)
		return false;

	int nColCount = m_txTable.GetColCount();
	if(nColCount>0)
	{
		UINT* nColArray = new UINT[nColCount];
		for(int i=0;i<nColCount;i++)
			nColArray[i]=i;

		//财年col=2
		resTable.CopyColumnInfoFrom(m_txTable);
		/*
		std::set<INT64> isetIndex;
		isetIndex.insert(Iindex1);
		isetIndex.insert(Iindex2);
		isetIndex.insert(Iindex3);
		isetIndex.insert(Iindex4);
		m_txTable.EqualsAt(resTable,nColArray,nColCount,0,isetIndex);
		*/
		m_txTable.Between(resTable, nColArray, nColCount, 0, Iindex, Iindex, TRUE, TRUE);

		delete nColArray; 
		nColArray = NULL;
	}
	else
		return false;

	return true;
}


//专项统计：分红派息（分红送配）
//add by lijw	2008-01-29
bool Tx::Business::TxStock::StatDivident(
		std::set<int> iSecurityId,				//交易实体ID
		int iStartDate,							//起始日
		int iEndDate,							//终止日
		Tx::Core::Table_Indicator &resTable,	//结果数据表
		bool bGetAllDate,							//是否全部日期
		UINT uFlagType							//标志位	1:分红	2:配股	4:送股
		)
{
	//添加进度条
//	ProgressWnd* pwd=Tx::Core::ProgressWnd::GetInstance();
	Tx::Core::ProgressWnd prw;
	UINT pid=prw.AddItem(1,_T("分红送配统计..."),0.0);
	prw.Show(1000);

	//获得汇率信息
	double dUSDRatio = Tx::Core::Con_doubleInvalid;
	DataFileNormal<blk_TxExFile_FileHead,ExRate>* pUSDRatio = new DataFileNormal<blk_TxExFile_FileHead,ExRate>;
	if (NULL != pUSDRatio)
	{
		if(pUSDRatio->Load(30253, 30007, true)==false || pUSDRatio->GetDataCount()<1)
		{
			delete pUSDRatio;
			pUSDRatio = NULL;
			return false;
		}
	}

	//添加列
	resTable.AddCol(Tx::Core::dtype_int4);//交易实体ID
	resTable.AddCol(Tx::Core::dtype_val_string);//名称
	resTable.AddCol(Tx::Core::dtype_val_string);//代码
    resTable.AddCol(Tx::Core::dtype_val_string);//类型
	resTable.AddCol(Tx::Core::dtype_val_string);//年度
    resTable.AddCol(Tx::Core::dtype_int4);//登记日
	resTable.AddCol(Tx::Core::dtype_int4);//除权日
	resTable.AddCol(Tx::Core::dtype_val_string);//分配方案
	resTable.AddCol(Tx::Core::dtype_val_string);//流通分配
	resTable.AddCol(Tx::Core::dtype_val_string);//分配总额
	resTable.AddCol(Tx::Core::dtype_val_string);//注册省份
	resTable.AddCol(Tx::Core::dtype_val_string);//证监会行业代码
	std::set<int>::iterator iter;
	StockBonusData * pStockB;
	StockBonusData StockTemp;
	StockRightIssue * pIssue;
	StockRightIssue Issue;
	CString strName,strCode,strproject,strType,stryear;
	CString strCirculate,strAccount,strPro,CSRCode;
	double dTemp;
	static int test = 0;//是为了增加多少行而准备的。
	if(uFlagType == 1 || uFlagType == 2 || uFlagType == 4)
	{
		
		switch(uFlagType)
		{
		case 1://分红
			for(iter = iSecurityId.begin();iter != iSecurityId.end();++iter)
			{
				GetSecurityNow(*iter);
				if (m_pSecurity == NULL)
				{
					continue;
				}
				strName = m_pSecurity->GetName();
				strCode = m_pSecurity->GetCode();
				strType = _T("分红");
				//取得证监会行业代码和注册省份
				CSRCode = m_pSecurity->GetCSRCIndustryCode();
				if(CSRCode == _T("-"))
				{
					CSRCode = m_pSecurity->GetCSRCIndustryCode(1);
				}
				strPro = m_pSecurity->GetRegisterProvance();
				int count = m_pSecurity->GetStockBonusDataCount();
				//按分红进行统计

				for(int i = 0;i < count;i++)
				{
					pStockB = m_pSecurity->GetStockBonusDataByIndex(i);
					if(pStockB->pxshu > 0)
					{
						if (!bGetAllDate)
						{
							if(!(iStartDate <= pStockB->excluding_dividend_date && pStockB->excluding_dividend_date <= iEndDate))
							continue;
						}
						//StockTemp.dividend_type = pStockB->dividend_type;//分红方式
						StockTemp.year = pStockB->year;//分红年度
						StockTemp.register_date = pStockB->register_date;//登记日
						StockTemp.excluding_dividend_date = pStockB->excluding_dividend_date;//除权日

						StockTemp.pxshu = pStockB->pxshu;//派现金数
						if (NULL != pUSDRatio && m_pSecurity->IsStockB() && m_pSecurity->IsShanghai())
						{
							ExRate* pRatio = NULL;
							pRatio = pUSDRatio->GetDataByObj(StockTemp.register_date);
							if (NULL != pRatio)
							{
								StockTemp.pxshu *= pRatio->iRatio;
							}
						}

						TxShareData*  pTxShareData =  m_pSecurity->GetTxShareDataByDate(StockTemp.register_date);//pStockB->b_share_register;
						//cyh modified at 2008-12-12
						//b 股的股本
						if (NULL != pTxShareData)
						{
							//StockTemp.b_share_register = pTxShareData->TradeableShare;
							//流通A股的股本
							StockTemp.a_share_register = pTxShareData->TradeableShare;///pStockB->a_share_register;
							//总股本
							StockTemp.total_share_register = pTxShareData->TotalShare;//pStockB->total_share_register;
							if (m_pSecurity->IsStockB())
							{
								StockTemp.total_share_register = pTxShareData->TradeableShare;
							}
						}

						resTable.AddRow(1);
						resTable.SetCell(0,test,*iter);
						resTable.SetCell(1,test,strName);
						resTable.SetCell(2,test,strCode);
						resTable.SetCell(3,test,strType);
						if(StockTemp.year < 0)
							StockTemp.year = 0;
						stryear.Format(_T("%d"),StockTemp.year);
						stryear = stryear.Left(4);
						if(stryear == _T("0"))
							stryear = Tx::Core::Con_strInvalid;
						resTable.SetCell(4,test,stryear);
						resTable.SetCell(5,test,StockTemp.register_date);
						resTable.SetCell(6,test,StockTemp.excluding_dividend_date);
						strproject.Format(_T("1∶%.3f"),StockTemp.pxshu);
						resTable.SetCell(7,test,strproject);

						//if (m_pSecurity->IsStockA())
						{
							if (StockTemp.a_share_register < 0)
							{
								strCirculate = _T("-");
							} 
							else
							{
								dTemp = StockTemp.pxshu * StockTemp.a_share_register/10000;
								strCirculate.Format(_T("%.2f"),dTemp);
							}
						}
// 						else if (m_pSecurity->IsStockB())
// 						{
// 							if (StockTemp.b_share_register < 0)
// 							{
// 								strCirculate = _T("-");
// 							} 
// 							else
// 							{
// 								dTemp = StockTemp.pxshu * StockTemp.b_share_register/10000;
// 								strCirculate.Format(_T("%.2f"),dTemp);
// 							}
// 						}

						resTable.SetCell(8,test,strCirculate);
						dTemp = 0;
						if (StockTemp.total_share_register < 0)
						{
							strAccount = _T("-");

						} 
						else
						{
							dTemp = StockTemp.pxshu * StockTemp.total_share_register/10000;
							strAccount.Format(_T("%.2f"),dTemp);
						}
						resTable.SetCell(9,test,strAccount);
						resTable.SetCell(10,test,strPro);
						resTable.SetCell(11,test,CSRCode);
						test++;//是为了计算已经增加了多少行。
					}

				}

			}
			break;

		case 4://送股
			for(iter = iSecurityId.begin();iter != iSecurityId.end();++iter)
			{
				GetSecurityNow(*iter);
				if (m_pSecurity == NULL)
				{
					continue;
				}
				strName = m_pSecurity->GetName();
				strCode = m_pSecurity->GetCode();
				strType = _T("送股");
				//取得证监会行业代码和注册省份
				CSRCode = m_pSecurity->GetCSRCIndustryCode();
				if(CSRCode == _T("-"))
				{
					CSRCode = m_pSecurity->GetCSRCIndustryCode(1);
				}
				strPro = m_pSecurity->GetRegisterProvance();
				int count = m_pSecurity->GetStockBonusDataCount();
				//按送股进行统计
				for(int i = 0;i < count;i++)
				{
					pStockB = m_pSecurity->GetStockBonusDataByIndex(i);
					if(pStockB->sgshu > 0)
					{
						if (!bGetAllDate)
						{
							if(!(iStartDate <= pStockB->excluding_dividend_date && pStockB->excluding_dividend_date <= iEndDate))
							continue;
						}
						//StockTemp.dividend_type = pStockB->dividend_type;//分红方式
						StockTemp.year = pStockB->year;//分红年度
						StockTemp.register_date = pStockB->register_date;//登记日
						StockTemp.excluding_dividend_date = pStockB->excluding_dividend_date;//除权日
						StockTemp.sgshu = pStockB->sgshu;//送股数

						resTable.AddRow(1);
						resTable.SetCell(0,test,*iter);
						resTable.SetCell(1,test,strName);
						resTable.SetCell(2,test,strCode);
						resTable.SetCell(3,test,strType);
						if(StockTemp.year < 0)
							StockTemp.year = 0;
						stryear.Format(_T("%d"),StockTemp.year);
						stryear = stryear.Left(4);
						if(stryear == _T("0"))
							stryear = Tx::Core::Con_strInvalid;
						resTable.SetCell(4,test,stryear);
						resTable.SetCell(5,test,StockTemp.register_date);//登记日
						resTable.SetCell(6,test,StockTemp.excluding_dividend_date);//除权日
						strproject.Format(_T("1∶%.3f"),StockTemp.sgshu);
						resTable.SetCell(7,test,strproject);//分配方案
						strCirculate = _T("-");
						resTable.SetCell(8,test,strCirculate);
						strAccount = _T("-");
						resTable.SetCell(9,test,strAccount);
						resTable.SetCell(10,test,strPro);
						resTable.SetCell(11,test,CSRCode);
						test++;
					}

				}

			}
			break;

		case 2://按配股进行统计

			for(iter = iSecurityId.begin();iter != iSecurityId.end();++iter)
			{
				GetSecurityNow(*iter);
				if (m_pSecurity == NULL)
				{
					continue;
				}
				strName = m_pSecurity->GetName();
				strCode = m_pSecurity->GetCode();
				//年度
				stryear = _T("-");
				//取得证监会行业代码和注册省份
				CSRCode = m_pSecurity->GetCSRCIndustryCode();
				if(CSRCode == _T("-"))
				{
					CSRCode = m_pSecurity->GetCSRCIndustryCode(1);
				}
				strPro = m_pSecurity->GetRegisterProvance();
				int icount = m_pSecurity->GetStockRightIssueCount();
				for(int j = 0;j < icount;j++)
				{
					pIssue = m_pSecurity->GetStockRightIssueByIndex(j);
					if(!bGetAllDate)
					{
						if(!(iStartDate <= pIssue->ex_date && pIssue->ex_date <= iEndDate))
						continue;
					}
					Issue.year = pIssue->year;
					Issue.register_date = pIssue->register_date;//登记日
					Issue.ex_date = pIssue->ex_date;//除权日
					strType  = pIssue->pgfs;//配股方式。
					Issue.right_issue_per = pIssue->right_issue_per;//配股比例

					resTable.AddRow(1);
					resTable.SetCell(0,test,*iter);
					resTable.SetCell(1,test,strName);
					resTable.SetCell(2,test,strCode);
					resTable.SetCell(3,test,strType);
					if(Issue.year < 0)
						Issue.year = 0;
					stryear.Format(_T("%d"),Issue.year);
					stryear = stryear.Left(4);
					if(stryear == _T("0"))
						stryear = Tx::Core::Con_strInvalid;
					resTable.SetCell(4,test,stryear);
					resTable.SetCell(5,test,Issue.register_date);//登记日
					resTable.SetCell(6,test,Issue.ex_date);//除权日
					strproject.Format(_T("1∶%.3f"),Issue.right_issue_per);
					resTable.SetCell(7,test,strproject);//分配方案
					strCirculate = _T("-");
					resTable.SetCell(8,test,strCirculate);
					strAccount = _T("-");
					resTable.SetCell(9,test,strAccount);
					resTable.SetCell(10,test,strPro);
					resTable.SetCell(11,test,CSRCode);
					test++;
				}
			}
			break;

		}

	}
	//添加进度条
	prw.SetPercent(pid,0.6);
	if(uFlagType == 3 || uFlagType == 5 || uFlagType == 6|| uFlagType == 7)
	{
		switch(uFlagType)
		{
		case 3://分红和配股的统计
			for(iter = iSecurityId.begin();iter != iSecurityId.end();++iter)
			{
				//分红的统计
				GetSecurityNow(*iter);
				if (m_pSecurity == NULL)
				{
					continue;
				}
				strName = m_pSecurity->GetName();
				strCode = m_pSecurity->GetCode();
				strType = _T("分红");
				//取得证监会行业代码和注册省份
				CSRCode = m_pSecurity->GetCSRCIndustryCode();
				if(CSRCode == _T("-"))
				{
					CSRCode = m_pSecurity->GetCSRCIndustryCode(1);
				}
				strPro = m_pSecurity->GetRegisterProvance();
				int count = m_pSecurity->GetStockBonusDataCount();
			
				for(int i = 0;i < count;i++)
				{
					pStockB = m_pSecurity->GetStockBonusDataByIndex(i);
					if(pStockB->pxshu > 0)
					{
						if (!bGetAllDate)
						{
							if(!(iStartDate <= pStockB->excluding_dividend_date && pStockB->excluding_dividend_date <= iEndDate))
							continue;
						}
						//StockTemp.dividend_type = pStockB->dividend_type;//分红方式
						StockTemp.year = pStockB->year;//分红年度
						StockTemp.register_date = pStockB->register_date;//登记日
						StockTemp.excluding_dividend_date = pStockB->excluding_dividend_date;//除权日
						StockTemp.pxshu = pStockB->pxshu;//派现金数
						if (NULL != pUSDRatio && m_pSecurity->IsStockB() && m_pSecurity->IsShanghai())
						{
							ExRate* pRatio = NULL;
							pRatio = pUSDRatio->GetDataByObj(StockTemp.register_date);
							if (NULL != pRatio)
							{
								StockTemp.pxshu *= pRatio->iRatio;
							}
						}
// 						//流通A股的股本
// 						StockTemp.a_share_register = pStockB->a_share_register;
// 						//总股本
// 						StockTemp.total_share_register = pStockB->total_share_register;


						TxShareData*  pTxShareData =  m_pSecurity->GetTxShareDataByDate(StockTemp.register_date);//pStockB->b_share_register;
						//cyh modified at 2008-12-12
						//b 股的股本
						if (NULL != pTxShareData)
						{
							//StockTemp.b_share_register = pTxShareData->TradeableShare;
							//流通A股的股本
							StockTemp.a_share_register = pTxShareData->TradeableShare;///pStockB->a_share_register;
							//总股本
							StockTemp.total_share_register = pTxShareData->TotalShare;//pStockB->total_share_register;
							if (m_pSecurity->IsStockB())
							{
								StockTemp.total_share_register = pTxShareData->TradeableShare;
							}
						}

						resTable.AddRow(1);
						resTable.SetCell(0,test,*iter);
						resTable.SetCell(1,test,strName);
						resTable.SetCell(2,test,strCode);
						resTable.SetCell(3,test,strType);
						if(StockTemp.year < 0)
							StockTemp.year = 0;
						stryear.Format(_T("%d"),StockTemp.year);
						stryear = stryear.Left(4);
						if(stryear == _T("0"))
							stryear = Tx::Core::Con_strInvalid;
						resTable.SetCell(4,test,stryear);
						resTable.SetCell(5,test,StockTemp.register_date);
						resTable.SetCell(6,test,StockTemp.excluding_dividend_date);
						strproject.Format(_T("1∶%.3f"),StockTemp.pxshu);
						resTable.SetCell(7,test,strproject);
						if (StockTemp.a_share_register < 0)
						{
							strCirculate = _T("-");

						} 
						else
						{
							dTemp = StockTemp.pxshu * StockTemp.a_share_register/10000;
							strCirculate.Format(_T("%.2f"),dTemp);
						}
						resTable.SetCell(8,test,strCirculate);
						dTemp = 0;
						if (StockTemp.total_share_register < 0)
						{
							strAccount = _T("-");

						} 
						else
						{
							dTemp = StockTemp.pxshu * StockTemp.total_share_register/10000;
							strAccount.Format(_T("%.2f"),dTemp);
						}
						resTable.SetCell(9,test,strAccount);
						resTable.SetCell(10,test,strPro);
						resTable.SetCell(11,test,CSRCode);
						test++;
					}

				}
				//配股的统计
				int icount = m_pSecurity->GetStockRightIssueCount();
				//年度
				stryear = _T("-");
				CString strpgfs = _T("");//为了不与上面的重复，所以又定义了保存配股方式的变量。
				for(int j = 0;j < icount;j++)
				{
					pIssue = m_pSecurity->GetStockRightIssueByIndex(j);
					if(!bGetAllDate)
					{
						if(!(iStartDate <= pIssue->ex_date && pIssue->ex_date <= iEndDate))
						continue;
					}	
					Issue.year = pIssue->year;
					Issue.register_date = pIssue->register_date;//登记日
					Issue.ex_date = pIssue->ex_date;//除权日
					strpgfs  = pIssue->pgfs;//配股方式。
					Issue.right_issue_per = pIssue->right_issue_per;//配股比例

					resTable.AddRow(1);
					resTable.SetCell(0,test,*iter);
					resTable.SetCell(1,test,strName);
					resTable.SetCell(2,test,strCode);
					resTable.SetCell(3,test,strpgfs);
					if(Issue.year < 0)
					{
						Issue.year = 0;
					}
					stryear.Format(_T("%d"),Issue.year);
					stryear = stryear.Left(4);
					if(stryear == _T("0"))
					{
						stryear = Tx::Core::Con_strInvalid;
					}
					resTable.SetCell(4,test,stryear);
					resTable.SetCell(5,test,Issue.register_date);//登记日
					resTable.SetCell(6,test,Issue.ex_date);//除权日
					strproject.Format(_T("1∶%.3f"),Issue.right_issue_per);
					resTable.SetCell(7,test,strproject);//分配方案
					strCirculate = _T("-");
					resTable.SetCell(8,test,strCirculate);
					strAccount = _T("-");
					resTable.SetCell(9,test,strAccount);
					resTable.SetCell(10,test,strPro);
					resTable.SetCell(11,test,CSRCode);
                    test++;
				}
			}
			break;
		case 5://分红+送股
			for(iter = iSecurityId.begin();iter != iSecurityId.end();++iter)
			{
				GetSecurityNow(*iter);
				if (m_pSecurity == NULL)
				{
					continue;
				}
				strName = m_pSecurity->GetName();
				strCode = m_pSecurity->GetCode();
				//取得证监会行业代码和注册省份
				CSRCode = m_pSecurity->GetCSRCIndustryCode();
				if(CSRCode == _T("-"))
				{
					CSRCode = m_pSecurity->GetCSRCIndustryCode(1);
				}
				strPro = m_pSecurity->GetRegisterProvance();
				int count = m_pSecurity->GetStockBonusDataCount();
				//按分红和送股进行统计
				for(int i = 0;i < count;i++)
				{
					pStockB = m_pSecurity->GetStockBonusDataByIndex(i);
					if (!bGetAllDate)
					{
						if(!(iStartDate <= pStockB->excluding_dividend_date && pStockB->excluding_dividend_date <= iEndDate))
						continue;
					}
					StockTemp.dividend_type = pStockB->dividend_type;//分红方式
					StockTemp.year = pStockB->year;//分红年度
					StockTemp.register_date = pStockB->register_date;//登记日
					StockTemp.excluding_dividend_date = pStockB->excluding_dividend_date;//除权日
					StockTemp.pxshu = pStockB->pxshu;//派现金数
					StockTemp.sgshu = pStockB->sgshu;//送股数

					if (NULL != pUSDRatio && m_pSecurity->IsStockB() && m_pSecurity->IsShanghai())
					{
						ExRate* pRatio = NULL;
						pRatio = pUSDRatio->GetDataByObj(StockTemp.register_date);
						if (NULL != pRatio)
						{
							StockTemp.pxshu *= pRatio->iRatio;
						}
					}
// 					//流通A股的股本
// 					StockTemp.a_share_register = pStockB->a_share_register;
// 					//总股本
// 					StockTemp.total_share_register = pStockB->total_share_register;


					TxShareData*  pTxShareData =  m_pSecurity->GetTxShareDataByDate(StockTemp.register_date);//pStockB->b_share_register;
					//cyh modified at 2008-12-12
					//b 股的股本
					if (NULL != pTxShareData)
					{
						//StockTemp.b_share_register = pTxShareData->TradeableShare;
						//流通A股的股本
						StockTemp.a_share_register = pTxShareData->TradeableShare;///pStockB->a_share_register;
						//总股本
						StockTemp.total_share_register = pTxShareData->TotalShare;//pStockB->total_share_register;
						if (m_pSecurity->IsStockB())
						{
							StockTemp.total_share_register = pTxShareData->TradeableShare;
						}
					}

					//既分红又送股的特殊情况处理
					if(StockTemp.sgshu > 0 && StockTemp.pxshu > 0)
					{
						//要把它写成两条记录。
						for(int j = 0;j < 2;j++)
						{
							resTable.AddRow(1);
							resTable.SetCell(0,test,*iter);
							resTable.SetCell(1,test,strName);
							resTable.SetCell(2,test,strCode);
							if(j == 0)//第一条记录。
							{
								strType = _T("送股");
								resTable.SetCell(3,test,strType);
								strproject.Format(_T("1∶%.3f"),StockTemp.sgshu);
								resTable.SetCell(7,test,strproject);//分配方案
								strCirculate = _T("-");
								resTable.SetCell(8,test,strCirculate);
								strAccount = _T("-");
								resTable.SetCell(9,test,strAccount);

							}
							else//第二条记录。
							{
								strType = _T("分红");
								resTable.SetCell(3,test,strType);
								strproject.Format(_T("1∶%.3f"),StockTemp.pxshu);
								resTable.SetCell(7,test,strproject);
								if (StockTemp.a_share_register < 0)
								{
									strCirculate = _T("-");
							
								} 
								else
								{
									dTemp = StockTemp.pxshu * StockTemp.a_share_register/10000;
									strCirculate.Format(_T("%.2f"),dTemp);
								}
								resTable.SetCell(8,test,strCirculate);
								dTemp = 0;
								if (StockTemp.total_share_register < 0)
								{
									strAccount = _T("-");
								
								} 
								else
								{
									dTemp = StockTemp.pxshu * StockTemp.total_share_register/10000;
									strAccount.Format(_T("%.2f"),dTemp);
								}
								resTable.SetCell(9,test,strAccount);
							}
							if(StockTemp.year < 0)
								StockTemp.year = 0;
							stryear.Format(_T("%d"),StockTemp.year);
							stryear = stryear.Left(4);
							if(stryear == "0")
								stryear = Tx::Core::Con_strInvalid;
							resTable.SetCell(4,test,stryear);
							resTable.SetCell(5,test,StockTemp.register_date);
							resTable.SetCell(6,test,StockTemp.excluding_dividend_date);
							resTable.SetCell(10,test,strPro);
							resTable.SetCell(11,test,CSRCode);
							test++;
						}
                     
					}
					//下面是正常情况的统计。
					if (StockTemp.pxshu > 0 || StockTemp.sgshu > 0)
					{
						
						if(StockTemp.pxshu > 0 && StockTemp.sgshu <= 0)
						{
							resTable.AddRow(1);
							resTable.SetCell(0,test,*iter);
							resTable.SetCell(1,test,strName);
							resTable.SetCell(2,test,strCode);
							strType = _T("分红");
							resTable.SetCell(3,test,strType);
							strproject.Format(_T("1∶%.3f"),StockTemp.pxshu);
							resTable.SetCell(7,test,strproject);
							if (StockTemp.a_share_register < 0)
							{
								strCirculate = _T("-");
			
							} 
							else
							{
								dTemp = StockTemp.pxshu * StockTemp.a_share_register/10000;
								strCirculate.Format(_T("%.2f"),dTemp);
							}
							resTable.SetCell(8,test,strCirculate);
							dTemp = 0;
							if (StockTemp.total_share_register < 0)
							{
								strAccount = _T("-");
							
							} 
							else
							{
								dTemp = StockTemp.pxshu * StockTemp.total_share_register/10000;
								strAccount.Format(_T("%.2f"),dTemp);
							}
							resTable.SetCell(9,test,strAccount);
							if(StockTemp.year < 0)
								StockTemp.year = 0;
							stryear.Format(_T("%d"),StockTemp.year);
							stryear = stryear.Left(4);
							if(stryear == "0")
								stryear = Tx::Core::Con_strInvalid;
							resTable.SetCell(4,test,stryear);
							resTable.SetCell(5,test,StockTemp.register_date);
							resTable.SetCell(6,test,StockTemp.excluding_dividend_date);
							resTable.SetCell(10,test,strPro);
							resTable.SetCell(11,test,CSRCode);
							test++;
						}
						if(StockTemp.sgshu > 0 && StockTemp.pxshu <= 0)
						{
							resTable.AddRow(1);
							resTable.SetCell(0,test,*iter);
							resTable.SetCell(1,test,strName);
							resTable.SetCell(2,test,strCode);
							strType = _T("送股");
							resTable.SetCell(3,test,strType);
							strproject.Format(_T("1∶%.3f"),StockTemp.sgshu);
							resTable.SetCell(7,test,strproject);//分配方案
							strCirculate = _T("-");
							resTable.SetCell(8,test,strCirculate);
							strAccount = _T("-");
							resTable.SetCell(9,test,strAccount);
							if(StockTemp.year < 0)
								StockTemp.year = 0;
							stryear.Format(_T("%d"),StockTemp.year);
							stryear = stryear.Left(4);
							if(stryear == _T("0"))
								stryear = Tx::Core::Con_strInvalid;
							resTable.SetCell(4,test,stryear);
							resTable.SetCell(5,test,StockTemp.register_date);
							resTable.SetCell(6,test,StockTemp.excluding_dividend_date);
							resTable.SetCell(10,test,strPro);
							resTable.SetCell(11,test,CSRCode);
							test++;

						}

						
					}
					
				}
			}
			break;
		case 6://送股+配股
			for(iter = iSecurityId.begin();iter != iSecurityId.end();++iter)
			{
				GetSecurityNow(*iter);
				if (m_pSecurity == NULL)
				{
					continue;
				}
				strName = m_pSecurity->GetName();
				strCode = m_pSecurity->GetCode();
				//取得证监会行业代码和注册省份
				CSRCode = m_pSecurity->GetCSRCIndustryCode();
				if(CSRCode == _T("-"))
				{
					CSRCode = m_pSecurity->GetCSRCIndustryCode(1);
				}
				strPro = m_pSecurity->GetRegisterProvance();
				int count = m_pSecurity->GetStockBonusDataCount();
				//按送股进行统计
				strType = _T("送股");
				for(int i = 0;i < count;i++)
				{
					pStockB = m_pSecurity->GetStockBonusDataByIndex(i);
					if(pStockB->sgshu > 0)
					{
						if (!bGetAllDate)
						{
							if(!(iStartDate <= pStockB->excluding_dividend_date && pStockB->excluding_dividend_date <= iEndDate))
							continue;
						}
						//StockTemp.dividend_type = pStockB->dividend_type;//分红方式
						StockTemp.year = pStockB->year;//分红年度

						StockTemp.register_date = pStockB->register_date;//登记日
						StockTemp.excluding_dividend_date = pStockB->excluding_dividend_date;//除权日
						StockTemp.sgshu = pStockB->sgshu;//送股数

						resTable.AddRow(1);
						resTable.SetCell(0,test,*iter);
						resTable.SetCell(1,test,strName);
						resTable.SetCell(2,test,strCode);
						resTable.SetCell(3,test,strType);
						if(StockTemp.year < 0)
							StockTemp.year = 0;
						stryear.Format(_T("%d"),StockTemp.year);
						stryear = stryear.Left(4);
						if(stryear == _T("0"))
							stryear = Tx::Core::Con_strInvalid;
						resTable.SetCell(4,test,stryear);
						resTable.SetCell(5,test,StockTemp.register_date);//登记日
						resTable.SetCell(6,test,StockTemp.excluding_dividend_date);//除权日
						strproject.Format(_T("1∶%.3f"),StockTemp.sgshu);
						resTable.SetCell(7,test,strproject);//分配方案
						strCirculate = _T("-");
						resTable.SetCell(8,test,strCirculate);
						strAccount = _T("-");
						resTable.SetCell(9,test,strAccount);
						resTable.SetCell(10,test,strPro);
						resTable.SetCell(11,test,CSRCode);
						test++;
					}

				}
				//配股的统计
				//年度
				stryear = _T("-");
				CString strpgfs = _T("");//为了不与上面的重复，所以又定义了保存配股方式的变量。
				int icount = m_pSecurity->GetStockRightIssueCount();
				for(int j = 0;j < icount;j++)
				{
					pIssue = m_pSecurity->GetStockRightIssueByIndex(j);
					if(!bGetAllDate)
					{
						if(!(iStartDate <= pIssue->ex_date && pIssue->ex_date <= iEndDate))
						continue;
					}
					Issue.year = pIssue->year;
					Issue.register_date = pIssue->register_date;//登记日
					Issue.ex_date = pIssue->ex_date;//除权日
					CString strpgfs;
					strpgfs  = pIssue->pgfs;//配股方式。
					Issue.right_issue_per = pIssue->right_issue_per;//配股比例

					resTable.AddRow(1);
					resTable.SetCell(0,test,*iter);
					resTable.SetCell(1,test,strName);
					resTable.SetCell(2,test,strCode);
					resTable.SetCell(3,test,strpgfs);
					if(Issue.year < 0)
					{
						Issue.year = 0;
					}
					stryear.Format(_T("%d"),Issue.year);
					stryear = stryear.Left(4);
					if(stryear == _T("0"))
					{
						stryear = Tx::Core::Con_strInvalid;
					}
					resTable.SetCell(4,test,stryear);
					resTable.SetCell(5,test,Issue.register_date);//登记日
					resTable.SetCell(6,test,Issue.ex_date);//除权日
					strproject.Format(_T("1∶%.3f"),Issue.right_issue_per);
					resTable.SetCell(7,test,strproject);//分配方案
					strCirculate = _T("-");
					resTable.SetCell(8,test,strCirculate);
					strAccount = _T("-");
					resTable.SetCell(9,test,strAccount);
					resTable.SetCell(10,test,strPro);
					resTable.SetCell(11,test,CSRCode);
					test++;
				}

			}
			break;
		case 7://分红+送配+配股
			for(iter = iSecurityId.begin();iter != iSecurityId.end();++iter)
			{
				GetSecurityNow(*iter);
				if (m_pSecurity == NULL)
				{
					continue;
				}
				strName = m_pSecurity->GetName();
				strCode = m_pSecurity->GetCode();
				//取得证监会行业代码和注册省份
				CSRCode = m_pSecurity->GetCSRCIndustryCode();
				if(CSRCode == _T("-"))
				{
					CSRCode = m_pSecurity->GetCSRCIndustryCode(1);
				}
				strPro = m_pSecurity->GetRegisterProvance();
				//分红+送股的专项统计
				int count = m_pSecurity->GetStockBonusDataCount();
				
				for(int i = 0;i < count;i++)
				{
					pStockB = m_pSecurity->GetStockBonusDataByIndex(i);
					if (!bGetAllDate)
					{
						if(!(iStartDate <= pStockB->excluding_dividend_date && pStockB->excluding_dividend_date <= iEndDate))
						continue;
					}					
					StockTemp.dividend_type = pStockB->dividend_type;//分红方式
					StockTemp.year = pStockB->year;//分红年度
					StockTemp.register_date = pStockB->register_date;//登记日
					StockTemp.excluding_dividend_date = pStockB->excluding_dividend_date;//除权日
					StockTemp.pxshu = pStockB->pxshu;//派现金数
					StockTemp.sgshu = pStockB->sgshu;//送股数

					if (NULL != pUSDRatio && m_pSecurity->IsStockB() && m_pSecurity->IsShanghai())
					{
						ExRate* pRatio = NULL;
						pRatio = pUSDRatio->GetDataByObj(StockTemp.register_date);
						if (NULL != pRatio)
						{
							StockTemp.pxshu *= pRatio->iRatio;
						}
					}
// 					//流通A股的股本
// 					StockTemp.a_share_register = pStockB->a_share_register;
// 					//总股本
// 					StockTemp.total_share_register = pStockB->total_share_register;

					TxShareData*  pTxShareData =  m_pSecurity->GetTxShareDataByDate(StockTemp.register_date);//pStockB->b_share_register;
					//cyh modified at 2008-12-12
					//b 股的股本
					if (NULL != pTxShareData)
					{
						//StockTemp.b_share_register = pTxShareData->TradeableShare;
						//流通A股的股本
						StockTemp.a_share_register = pTxShareData->TradeableShare;///pStockB->a_share_register;
						//总股本
						StockTemp.total_share_register = pTxShareData->TotalShare;//pStockB->total_share_register;
						if (m_pSecurity->IsStockB())
						{
							StockTemp.total_share_register = pTxShareData->TradeableShare;
						}
					}

					//既分红又送股的特殊情况处理
					if(StockTemp.sgshu > 0 && StockTemp.pxshu > 0)
					{
						//要把它写成两条记录。
						for(int j = 0;j < 2;j++)
						{
							resTable.AddRow(1);
							resTable.SetCell(0,test,*iter);
							resTable.SetCell(1,test,strName);
							resTable.SetCell(2,test,strCode);
							if(j == 0)//第一条记录。
							{
								strType = _T("送股");
								resTable.SetCell(3,test,strType);
								strproject.Format(_T("1∶%.3f"),StockTemp.sgshu);
								resTable.SetCell(7,test,strproject);//分配方案
								strCirculate = _T("-");
								resTable.SetCell(8,test,strCirculate);
								strAccount = _T("-");
								resTable.SetCell(9,test,strAccount);

							}
							else//第二条记录。
							{
								strType = _T("分红");
								resTable.SetCell(3,test,strType);
								strproject.Format(_T("1∶%.3f"),StockTemp.pxshu);
								resTable.SetCell(7,test,strproject);
								if (StockTemp.a_share_register < 0)
								{
									strCirculate = _T("-");
								
								}
								else
								{
									dTemp = StockTemp.pxshu * StockTemp.a_share_register/10000;
									strCirculate.Format(_T("%.2f"),dTemp);
								}
								
								resTable.SetCell(8,test,strCirculate);
								dTemp = 0;
								if (StockTemp.total_share_register < 0)
								{
									strAccount = _T("-");
					
								} 
								else
								{
									dTemp = StockTemp.pxshu * StockTemp.total_share_register/10000;
									strAccount.Format(_T("%.2f"),dTemp);
								}
								
								resTable.SetCell(9,test,strAccount);
							}
							if(StockTemp.year < 0)
								StockTemp.year = 0;
							stryear.Format(_T("%d"),StockTemp.year);
							stryear = stryear.Left(4);
							if(stryear == _T("0"))
								stryear = Tx::Core::Con_strInvalid;
							resTable.SetCell(4,test,stryear);
							resTable.SetCell(5,test,StockTemp.register_date);
							resTable.SetCell(6,test,StockTemp.excluding_dividend_date);
							resTable.SetCell(10,test,strPro);
							resTable.SetCell(11,test,CSRCode);
							test++;
						}

					}
					//下面是正常情况的统计。
					if (StockTemp.pxshu > 0 || StockTemp.sgshu > 0)
					{
						
						if(StockTemp.pxshu > 0 && StockTemp.sgshu <= 0)
						{
							resTable.AddRow(1);
							resTable.SetCell(0,test,*iter);
							resTable.SetCell(1,test,strName);
							resTable.SetCell(2,test,strCode);
							strType = _T("分红");
							resTable.SetCell(3,test,strType);
							strproject.Format(_T("1∶%.3f"),StockTemp.pxshu);
							resTable.SetCell(7,test,strproject);
							if (StockTemp.a_share_register < 0)
							{
								strCirculate = _T("-");

							}
							else
							{
								dTemp = StockTemp.pxshu * StockTemp.a_share_register/10000;
								strCirculate.Format(_T("%.2f"),dTemp);
							}
							resTable.SetCell(8,test,strCirculate);
							dTemp = 0;
							if (StockTemp.total_share_register < 0)
							{
								strAccount = _T("-");

							} 
							else
							{
								dTemp = StockTemp.pxshu * StockTemp.total_share_register/10000;
								strAccount.Format(_T("%.2f"),dTemp);
							}
							resTable.SetCell(9,test,strAccount);
							if(StockTemp.year < 0)
								StockTemp.year = 0;
							stryear.Format(_T("%d"),StockTemp.year);
							stryear = stryear.Left(4);
							if(stryear == "0")
								stryear = Tx::Core::Con_strInvalid;
							resTable.SetCell(4,test,stryear);
							resTable.SetCell(5,test,StockTemp.register_date);
							resTable.SetCell(6,test,StockTemp.excluding_dividend_date);
							resTable.SetCell(10,test,strPro);
							resTable.SetCell(11,test,CSRCode);
							test++;
						}
						if(StockTemp.sgshu > 0 && StockTemp.pxshu <= 0)
						{
							resTable.AddRow(1);
							resTable.SetCell(0,test,*iter);
							resTable.SetCell(1,test,strName);
							resTable.SetCell(2,test,strCode);
							strType = _T("送股");
							resTable.SetCell(3,test,strType);
							strproject.Format(_T("1∶%.3f"),StockTemp.sgshu);
							resTable.SetCell(7,test,strproject);//分配方案
							strCirculate = _T("-");
							resTable.SetCell(8,test,strCirculate);
							strAccount = _T("-");
							resTable.SetCell(9,test,strAccount);
							if(StockTemp.year < 0)
								StockTemp.year = 0;
							stryear.Format(_T("%d"),StockTemp.year);
							stryear = stryear.Left(4);
							if(stryear == _T("0"))
								stryear = Tx::Core::Con_strInvalid;
							resTable.SetCell(4,test,stryear);
							resTable.SetCell(5,test,StockTemp.register_date);
							resTable.SetCell(6,test,StockTemp.excluding_dividend_date);
							resTable.SetCell(10,test,strPro);
							resTable.SetCell(11,test,CSRCode);
							test++;

						}

					}
					
				}
				//按配股进行统计
				//配股的统计
				//年度
				stryear = _T("-");
				CString strpgfs = _T("");//为了不与上面的重复，所以又定义了保存配股方式的变量。
				int icount = m_pSecurity->GetStockRightIssueCount();
				for(int j = 0;j < icount;j++)
				{
					pIssue = m_pSecurity->GetStockRightIssueByIndex(j);
					if(!bGetAllDate)
					{
						if(!(iStartDate <= pIssue->ex_date && pIssue->ex_date <= iEndDate))
						continue;
					}
					Issue.year = pIssue->year;
					Issue.register_date = pIssue->register_date;//登记日
					Issue.ex_date = pIssue->ex_date;//除权日
					CString strpgfs;
					strpgfs  = pIssue->pgfs;//配股方式。
					Issue.right_issue_per = pIssue->right_issue_per;//配股比例

					resTable.AddRow(1);
					resTable.SetCell(0,test,*iter);
					resTable.SetCell(1,test,strName);
					resTable.SetCell(2,test,strCode);
					resTable.SetCell(3,test,strpgfs);
					if(Issue.year < 0)
					{
						Issue.year = 0;
					}
					stryear.Format(_T("%d"),Issue.year);
					stryear = stryear.Left(4);
					if(stryear == _T("0"))
					{
						stryear = Tx::Core::Con_strInvalid;
					}
					resTable.SetCell(4,test,stryear);
					resTable.SetCell(5,test,Issue.register_date);//登记日
					resTable.SetCell(6,test,Issue.ex_date);//除权日
					strproject.Format(_T("1∶%.3f"),Issue.right_issue_per);
					resTable.SetCell(7,test,strproject);//分配方案
					strCirculate = _T("-");
					resTable.SetCell(8,test,strCirculate);
					strAccount = _T("-");
					resTable.SetCell(9,test,strAccount);
					resTable.SetCell(10,test,strPro);
					resTable.SetCell(11,test,CSRCode);
					test++;
				}
			}
			break;
		}
	}
	test = 0;


	//删除利率信息
	if (NULL != pUSDRatio)
	{
		delete pUSDRatio;
		pUSDRatio = NULL;
	}

	//添加进度条
	prw.SetPercent(pid,1.0);
	return true;	
}



//专项统计：融资预案
//modify by lijw		2007-01-29
//功能：融资预案表的获得
	bool TxStock::StatFinancingPlan(
			std::set<int> & iSecurityId,			//样本ID
			int iStartDate,							//起始日期
			int iEndDate	,						//结束日期
			Tx::Core::Table_Indicator &resTable,	//结果数据表
			bool bGetAllDate
			)
{
	//添加进度条
//	ProgressWnd* pwd=Tx::Core::ProgressWnd::GetInstance();
	Tx::Core::ProgressWnd prw;
	UINT pid=prw.AddItem(1,_T("融资预案统计..."),0.0);
	prw.Show(1000);

	//要取回全表然后剔除无用数据
	std::vector<int> iInstitutionId;
	/*if(!TransObjectToSecIns(iSecurityId,iInstitutionId,2))
		return false;		*/
	Tx::Business::TxBusiness txbusiness;
	txbusiness.InistitutionFilter(iSecurityId);
	std::set<int>::iterator iterSet;
	for(iterSet = iSecurityId.begin();iterSet != iSecurityId.end();++iterSet)
	{
		GetSecurityNow(*iterSet);
		if(m_pSecurity != NULL)
		{
			iInstitutionId.push_back((int)m_pSecurity->GetInstitutionId());
		}
	}	
	//默认的返回值状态。
	bool result = true;
	//清空数据
	m_txTable.Clear();

	//机构id=第一参数列:F_insntitution_ID,int型
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
	//公告日期参数=第二参数列;F_disclosureDATE, int型
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
	//序号参数=第三参数列;f_no, int型
	m_txTable.AddParameterColumn(Tx::Core::dtype_byte);




	UINT varCfg[3];			//参数配置
	int varCount=3;			//参数个数	交易实体id，公告日

	//交易提示 指标组
	//item_id: 30300081-30300101
	const int INDICATOR_INDEX=14;
	long iIndicator[INDICATOR_INDEX]=
	{			
		30600036,	//拟融资方式
		30600037,	//发行价格（文字描述）
		30600038,	//拟发行规模（文字描述）
		30600039,	//拟发行股份数（文字描述）
		30600040,	//拟融资金额（文字描述）
		30600041,	//拟融资金额
		30600042,	//募集资金用途（文字描述）
		30600043,	//董事会议案是否通过
		30600044,	//股东大会审议通过否
		30600045,	//股东大会审议日期
		30600046,	//融资方案的有效期
		30600047,	//证监会审核情况
		30600048,	//融资进展情况
		30600049,	//方案变更情况
//		30600050	//记录的有效性
	//	30300151	//募集资金
					//备注'公告日'+date	+'派息日'+派息日
	};

	
	//设定指标列
	for (int i = 0; i <	INDICATOR_INDEX; i++)
	{
	    GetIndicatorDataNow(iIndicator[i]);
	
		varCfg[0]=0;
		varCfg[1]=1;
		varCfg[2]=2;

		result = m_pLogicalBusiness->SetIndicatorIntoTable(
													m_pIndicatorData,	//指标
													varCfg,				//参数配置
													varCount,			//参数个数
													m_txTable			//计算需要的参数传输载体以及计算后结果的载体
					);
		if(result==false)
			break;
	}
	if(result==false)
		return false;
	//根据之前3个步骤的设置进行数据读取，结果数据存放在table中
	result=m_pLogicalBusiness->GetData(m_txTable,true);
	if(result==false)
		return false;

	UINT	iCol=m_txTable.GetColCount();


	Tx::Core::Table_Indicator resTableDate;

	//复制所有列信息
	resTableDate.CopyColumnInfoFrom(m_txTable);
	resTable.CopyColumnInfoFrom(m_txTable);

	UINT* nColArray = new UINT[iCol];
	for(UINT i=0;i<iCol;i++)
		nColArray[i]=i;
	
	//取得指定时间区间内的记录数据	
	if(m_txTable.GetRowCount()>0)
	{
		if(!bGetAllDate)
		{
			m_txTable.Between(resTableDate,nColArray,iCol,1,iStartDate,iEndDate,true,true);
		}
		else
		{
			resTableDate.Clone(m_txTable);
		}
	}
	//添加进度条
	prw.SetPercent(pid,0.6);
	//取得指定实体id(iSecurityId)
	if(resTableDate.GetRowCount()>0)
	{
		resTableDate.EqualsAt(resTable,nColArray,iCol,0,iInstitutionId);
	}
	delete nColArray;
	nColArray = NULL;
	////为插入交易实体ID做准备。
	//m_txTable.InsertCol(1,Tx::Core::dtype_int4);//插入交易实体ID
	//插入这两列为了插入代码和名称。
	resTable.InsertCol(1,Tx::Core::dtype_val_string);//插入代码
	resTable.InsertCol(2,Tx::Core::dtype_val_string);//插入券名称
	//为了把发行股数的格式转化成符合规范的格式。
    resTable.InsertCol(9,Tx::Core::dtype_val_string);
	//为把董事会议案是否通过的值变成描述而准备。
	resTable.InsertCol(14,Tx::Core::dtype_val_string);
	//为把股东大会审议通过否的值变成描述而准备；
	resTable.InsertCol(16,Tx::Core::dtype_val_string);
	////插入当前解禁流通市值
	//m_txTable.InsertCol(11,Tx::Core::dtype_val_string);

	if(iSecurityId.size() != iInstitutionId.size())
	{
#ifdef _DEBUG
		AfxMessageBox(_T("机构ID和交易实体ID的数量不一样"));
#endif
		return false;
	}
	int iInsCount = 0;
	std::set<int>::iterator iter1;
	for(iter1 = iSecurityId.begin();iter1!=iSecurityId.end();++iter1,iInsCount++)
	{

		//取得交易实体ID
		int TradeID = *iter1;
		GetSecurityNow(*iter1);
		if(m_pSecurity==NULL)
			continue;
		//取得机构ID
		int tempInstitutionid1 = iInstitutionId[iInsCount];
		//根据交易实体ID取得样本的名称和外码；
		CString strName,strCode;
		strName = m_pSecurity->GetName();
		strCode = m_pSecurity->GetCode();
		
		//把机构ID和交易实体ID对应起来。并且把交易实体ID放到表里。
		std::vector<UINT> vecInstiID2;
		resTable.Find(0,tempInstitutionid1,vecInstiID2);
		std::vector<UINT>::iterator iteID;
		for(iteID = vecInstiID2.begin();iteID != vecInstiID2.end();++iteID)
		{

			resTable.SetCell(0,*iteID,TradeID);
			resTable.SetCell(1,*iteID,strName);
			resTable.SetCell(2,*iteID,strCode);
			//把董事会的表决和股东大会的表决由数字变成文字描述
			byte flag1,flag2;//flag1表示董事会的表决，flag2表示股东大会的表决。
			resTable.GetCell(13,*iteID,flag1);
			resTable.GetCell(15,*iteID,flag2);
			CString strFlag1,strFlag2;
			//董事会通过与否？
			strFlag1 = (flag1 == 1)?"通过":"未表决";
			//股东大会通过与否？
			strFlag2 = (flag2 == 1)?"通过":"未表决";
            resTable.SetCell(14,*iteID,strFlag1);
			resTable.SetCell(16,*iteID,strFlag2);
			//把发行股数的格式转化成符合规范的格式
			double dAccount;
			CString strAccount;
			resTable.GetCell(8,*iteID,dAccount);
			if(dAccount <= 0)
			{
				strAccount = Tx::Core::Con_strInvalid;
				resTable.SetCell(9,*iteID,strAccount);
				continue;
			}
            dAccount = dAccount/10000;
			strAccount.Format(_T("%.0f"),dAccount);
			resTable.SetCell(9,*iteID,strAccount);
		}
	}
	resTable.DeleteCol(15);
	resTable.DeleteCol(13);
	resTable.DeleteCol(8);
	resTable.DeleteCol(4);
#ifdef _DEBUG
	CString strTable4=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable4);
#endif

	//添加进度条
	prw.SetPercent(pid,1.0);
	return true;		
}//end of StatFinancingPlan


//限售股流通
//add by ljw	2007-1-22
//限售股流通表的获得
	bool TxStock::StatLimitedShare(
			std::set<int> & iSecurityId,			//样本ID
			int iStartDate,							//起始日期
			int iEndDate	,						//结束日期
			Tx::Core::Table_Indicator &resTable,	//结果数据表
			bool bGetAllDate
			)
{
	//默认的返回值状态。
	bool result = true;

	//添加进度条
	Tx::Core::ProgressWnd prw;
	UINT pid=prw.AddItem(1,_T("限售股流通统计..."),0.0);
	prw.Show(1000);

	std::set<int> setSecurityId;
	std::set<int> SameFundId;//为了不改变代码，这里存放的是交易实体ID
	std::set<int> DifferentID;
	std::set<int>::iterator iter = iSecurityId.begin();
	for(;iter != iSecurityId.end();++iter)
	{
		GetSecurityNow(*iter);
		if (m_pSecurity == NULL)
		{
			continue;
		}
		if (m_pSecurity->IsStockB())
		{
			continue;
		}
		int fundid =(int)m_pSecurity->GetInstitutionId();
		if(setSecurityId.find(fundid) == setSecurityId.end())
		{
			setSecurityId.insert(fundid);
			DifferentID.insert(*iter);
		}
		else
		{
			SameFundId.insert(*iter);
		}
	}

	if (0 == setSecurityId.size())
	{
		prw.SetPercent(pid,1.0);
		return false;
	}

	m_txTable.Clear();

	//公司代码:f_institution_id,int型
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
	//有效开始日期:f_start_date, int型
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
	//上市批次:f_ ,int型
	m_txTable.AddParameterColumn(Tx::Core::dtype_byte);
	//预计上市日期:f_ _date,int型
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);

	UINT varCfg[4];			//参数配置
	int varCount=4;			//参数个数	
	for(int i=0; i<4; i++)
	{
		varCfg[i]=i;
	}

	//交易提示 指标组
	//item_id: 30300081-30300101
	const int INDICATOR_INDEX=6;
	long iIndicator[INDICATOR_INDEX]=
	{
		30700010,	//实际上市日期
		30700006,	//公告日期
		30700008,	//解禁上市数量
		30700009,	//限售价格		
		30700007,	//有效截止日期
		30700012	//限售股份原因类型
	};

	
	//设定指标列
	for (int i = 0; i <	INDICATOR_INDEX; i++)
	{
	    GetIndicatorDataNow(iIndicator[i]);
		result = m_pLogicalBusiness->SetIndicatorIntoTable(m_pIndicatorData,varCfg,varCount,m_txTable);
		if(result==false)
		{
			break;
		}
	}
	if(result==false)
	{
		prw.SetPercent(pid,1.0);
		return false;
	}
	//根据之前3个步骤的设置进行数据读取，结果数据存放在table中
	result=m_pLogicalBusiness->GetData(m_txTable,true);
	if(result==false)
	{
		prw.SetPercent(pid,1.0);
		return false;
	}
	//添加进度条
	prw.SetPercent(pid,0.6);
#ifdef _DEBUG
	CString strTable=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	UINT iCol=m_txTable.GetColCount();
	//复制所有列信息
	resTable.CopyColumnInfoFrom(m_txTable);
	if(m_txTable.GetRowCount()==0)
	{
		prw.SetPercent(pid,1.0);
		return false;
	}

	UINT* nColArray = new UINT[iCol];
	for(UINT i=0;i<iCol;i++)
	{
		nColArray[i]=i;
	}
	
	//取得指定时间区间内的记录数据	
	if(m_txTable.GetRowCount()>0)
	{
		if(!bGetAllDate)
		{
			m_txTable.Between(resTable,nColArray,iCol,3,iStartDate,iEndDate,true,true);
		}
		else
		{
			resTable.Clone(m_txTable);	
		}
	}

    m_txTable.Clear();
	m_txTable.CopyColumnInfoFrom(resTable);
	if(resTable.GetRowCount()>0)
	{
		resTable.EqualsAt(m_txTable,nColArray,iCol,0,setSecurityId);
	}

#ifdef _DEBUG
	CString strTable1=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable1);
#endif
	if (m_txTable.GetRowCount() <= 0)
	{
		prw.SetPercent(pid,1.0);
		return false;
	}
    //为插入交易实体ID做准备。
	m_txTable.InsertCol(1,Tx::Core::dtype_int4);//插入交易实体ID
	//插入这两列为了插入代码和名称。
	m_txTable.InsertCol(2,Tx::Core::dtype_val_string);//插入名称
	m_txTable.InsertCol(3,Tx::Core::dtype_val_string);//插入代码
	//为调解解禁数量的小数点后面的位数而准备。
	m_txTable.InsertCol(10,Tx::Core::dtype_double);
	//插入当前价格一列；
	m_txTable.InsertCol(11,Tx::Core::dtype_double);
	//插入当前解禁流通市值
	m_txTable.InsertCol(12,Tx::Core::dtype_double);
	//插入限售股类型（CString）
	m_txTable.AddCol(Tx::Core::dtype_val_string);//16

	std::set<int>::iterator iter1;
	int imarketDate,iEndDate;
	int iPreDate;
	std::set<int,greater<int> > DelRowSet;
	std::set<int,greater<int> >::iterator iter2;
	for(iter1 = DifferentID.begin();iter1!=DifferentID.end();++iter1)
	{
		//取得交易实体ID
		int TradeID = *iter1;
		GetSecurityNow(*iter1);
		if(m_pSecurity==NULL)
		{
			continue;
		}
		//取得机构ID
		int tempInstitutionid1 = m_pSecurity->GetInstitutionId();
		//根据交易实体ID取得样本的名称和外码；
		CString strName = m_pSecurity->GetName();
		CString strCode = m_pSecurity->GetCode();
		double currentprice = m_pSecurity->GetClosePrice();
		//把机构ID和交易实体ID对应起来。并且把交易实体ID放到表里。
		std::vector<UINT> vecInstiID2;
		m_txTable.Find(0,tempInstitutionid1,vecInstiID2);
		std::vector<UINT>::iterator iteID;
		for(iteID = vecInstiID2.begin();iteID != vecInstiID2.end();++iteID)
		{
			//根据业务的要求，把没有上市日期的数据且有有效截至日期的数据去掉
			m_txTable.GetCell(7,*iteID,imarketDate);
			m_txTable.GetCell(14,*iteID,iEndDate);
			if(imarketDate <= 0 && iEndDate > 0)
			{
				DelRowSet.insert(*iteID);
				continue;
			}
			m_txTable.SetCell(1,*iteID,TradeID);
			m_txTable.SetCell(2,*iteID,strName);
			m_txTable.SetCell(3,*iteID,strCode);
			double circuAccount;
			double Account;
			double dData;
			if(currentprice < 0)
			{
				m_txTable.GetCell(9,*iteID,Account);
				if(Account < 0)
					Account = Tx::Core::Con_doubleInvalid;
				else
				    m_txTable.SetCell(10,*iteID,Account/10000);
				dData = Tx::Core::Con_doubleInvalid;
				m_txTable.SetCell(11,*iteID,dData);
				m_txTable.SetCell(12,*iteID,dData);
			}
			else
			{			
				m_txTable.SetCell(11,*iteID,currentprice);
				//往当前解禁流通市值这列填充数据
				m_txTable.GetCell(9,*iteID,Account);
				if(Account < 0)
				{
					Account = Tx::Core::Con_doubleInvalid;
				    m_txTable.SetCell(10,*iteID,Account);
					m_txTable.SetCell(12,*iteID,Account);
				}
				else
				{
					m_txTable.SetCell(10,*iteID,Account/10000);
					circuAccount = currentprice*Account/10000;
					m_txTable.SetCell(12,*iteID,circuAccount);
				}				
			}
			//把预计上市日期是99999999的过滤掉  2008-09-12			
			m_txTable.GetCell(6,*iteID,iPreDate);
			if(iPreDate == 99999999)
				m_txTable.SetCell(6,*iteID,Tx::Core::Con_intInvalid);	
			//添加限售股类型
			byte iData;
			CString  strType;
			m_txTable.GetCell(15,*iteID,iData);
			switch(iData)
			{
			case 1:
				strType = (_T("股改限售股份"));
				break;
			case 2:
				strType = (_T("IPO限售股份"));
				break;
			case 3:
				strType = (_T("增发限售股份"));
				break;
			case 4:
				strType = (_T("高管持股"));
				break;
			default:
				break;
			}
			m_txTable.SetCell(16,*iteID,strType);
		}
	}
	for (iter2 = DelRowSet.begin();iter2 != DelRowSet.end();++iter2)
	{
		m_txTable.DeleteRow(*iter2);
	}
	m_txTable.Arrange();
#ifdef _DEBUG
	CString strTable4=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable4);
#endif
	//下面这句时后来加的，我把第10列和16删掉，是为了不修改下面的代码。
	m_txTable.DeleteCol(15);
	m_txTable.DeleteCol(9);
	////为调整有效开始日期作准备
	//m_txTable.InsertCol(13,Tx::Core::dtype_int4);
	for(int k = 0;k < (int)m_txTable.GetRowCount();k++)
	{
		int date2;
		m_txTable.GetCell(8,k,date2);
		m_txTable.SetCell(4,k,date2);
	}
	m_txTable.DeleteCol(8);//删除公告日期所在的那一列

	//为不同的交易实体ID转化为相同的机构ID那样的样本填写数据，
	if(SameFundId.size() != 0)
	{
		//为计算方便，以机构ID进行排序
		m_txTable.Sort(0);
		//为不同的交易实体ID转化为相同的机构ID那样的样本填写数据，
		std::vector<UINT> vecFundID;
		std::set<int>::iterator iterSet,tempIter;
		for(iterSet = SameFundId.begin();iterSet != SameFundId.end();++iterSet)
		{
			int tempfundid ;
			GetSecurityNow(*iterSet);
			if (m_pSecurity == NULL)
			{
				continue;
			}
			tempfundid = m_pSecurity->GetInstitutionId();//取得机构ID
			CString fundname,fundcode;
			fundname = m_pSecurity->GetName();
			fundcode = m_pSecurity->GetCode();
			m_txTable.Find(0,tempfundid,vecFundID);
			if(vecFundID.size() == 0)
				continue;
			//取得在表中最小的位置
			std::set<int> tempset(vecFundID.begin(),vecFundID.end());
			tempIter = tempset.begin();
			//增加相同的记录
			std::set<int>::size_type icount = tempset.size();
			m_txTable.InsertRow(*tempIter,icount);
			int date1,date2,date3;
			double dprice;
			byte sspc;
            double dData1,dData2,dData3;
			int position1,position2;
			for(;tempIter != tempset.end();++tempIter)
			{
				position1 = *tempIter;
				position2 = position1 + icount;
				m_txTable.SetCell(1,position1,*iterSet);//交易实体ID
				m_txTable.SetCell(2,position1,fundname);
				m_txTable.SetCell(3,position1,fundcode);
				//取得其他数据
				m_txTable.GetCell(4,position2,date1);//公告日期
				m_txTable.GetCell(5,position2,sspc);//上市批次
				m_txTable.GetCell(6,position2,date2);//预计上市日期
				m_txTable.GetCell(7,position2,date3);//实际上市日期

				m_txTable.GetCell(8,position2,dData1);//解禁上市数量
				m_txTable.GetCell(9,position2,dData2);//当前价格
				m_txTable.GetCell(10,position2,dData3);//当前解禁流通市值
				m_txTable.GetCell(11,position2,dprice);//限售价格
//				m_txTable.GetCell(12,position2,date5);//有效截止日期
				CString strTemp;
				m_txTable.GetCell(13,position2,strTemp);//限售股类型
				//填充其他数据
			    m_txTable.SetCell(4,position1,date1);//公告日期
				m_txTable.SetCell(5,position1,sspc);//上市批次
				//把预计上市日期是99999999的过滤掉				
				if(date2 == 99999999)
				{
					m_txTable.SetCell(6,position1,Tx::Core::Con_doubleInvalid);
				}
				else
				{
					m_txTable.SetCell(6,position1,date2);//预计上市日期
				}
				m_txTable.SetCell(7,position1,date3);//实际上市日期
				m_txTable.SetCell(8,position1,dData1);//解禁上市数量
				m_txTable.SetCell(9,position1,dData2);//当前价格
				m_txTable.SetCell(10,position1,dData3);//当前解禁流通市值
				m_txTable.SetCell(11,position1,dprice);//限售价格
//				m_txTable.SetCell(12,position1,date5);//有效截止日期
				m_txTable.SetCell(13,position1,strTemp);//限售股类型
			}
			vecFundID.clear();
			tempset.clear();
		}
	}
	//删除有效截止日期和机构ID
	m_txTable.DeleteCol(12);	//删除有效截止日期
	m_txTable.DeleteCol(0);	//删除机构ID
    
	resTable.Clone(m_txTable);
	delete nColArray;
	nColArray = NULL;

#ifdef _DEBUG
	CString strTable3=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable3);
#endif
	//添加进度条
	prw.SetPercent(pid,1.0);
	return true;
}//end of StatLimitedShare


//股本变化
//add by guanyd		2007-08-18
//modify by lijw 2008-01-01
bool TxStock::StatShareChange(
			std::set<int> & iSecurityId,			//机构样本
			int iStartDate,							//起始日期
			int iEndDate,						//结束日期
			Tx::Core::Table_Indicator &resTable,	//结果数据表
			bool bGetAllDate,						//是否取得全部日期
			UINT uFlagType							//标志位	1:首发	2:配股	4:增发	8:送股	16:股权分置改革
			)
{
	//添加进度条
	Tx::Core::ProgressWnd prw;
	UINT pid=prw.AddItem(1,_T("股本变化统计..."),0.0);
	prw.Show(1000);

	//1 交易实体ID到机构ID
	std::set<int> iInstitutionId;
	if(!TransObjectToSecIns(iSecurityId,iInstitutionId,2))
	{
		return false;
	}

	/*2 参数准备*/
	bool result = true;
	m_txTable.Clear();	
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//公司代码:f_institution_id,int型
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//公告日期;f_basic_date, int型

	UINT varCfg[2];			//参数配置
	int varCount=2;			//参数个数	交易实体id，公告日
	varCfg[0]=0;
	varCfg[1]=1;

	const int INDICATOR_INDEX=8;	//交易提示 指标组
	long iIndicator[INDICATOR_INDEX]=
	{			
		30300173,	//事件类型
		30300174,	//发行日期
		30300175,	//上市日期
		30300177,	//流通股（变动前）
		30300176,	//流通股（变动后）
		30300179,	//总股本（变动前）
		30300178,	//总股本（变动后）
		30300180	//备注
	};
	
	for (int i = 0; i <	INDICATOR_INDEX; i++)	//设定指标列
	{
	    GetIndicatorDataNow(iIndicator[i]);
		result = m_pLogicalBusiness->SetIndicatorIntoTable(
													m_pIndicatorData,	//指标
													varCfg,				//参数配置
													varCount,			//参数个数
													m_txTable);			//计算需要的参数传输载体以及计算后结果的载体
												
		if(result==false)
		{
			break;
		}
	}
	if(result==false)
	{
		return false;
	}

	/*3根据之前3个步骤的设置进行数据读取，结果数据存放在table中*/
	result=m_pLogicalBusiness->GetData(m_txTable,true);
	if(result==false)
	{
		return false;
	}
#ifdef _DEBUG
	CString strTable=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	Tx::Core::Table_Indicator resTableDate;
	Tx::Core::Table_Indicator resTableType;

	//复制所有列信息
	resTableDate.CopyColumnInfoFrom(m_txTable);
	resTableType.CopyColumnInfoFrom(m_txTable);
	resTable.CopyColumnInfoFrom(m_txTable);

	if(m_txTable.GetRowCount()==0)
	{
		return true;
	}

	UINT	iCol=m_txTable.GetColCount();
	UINT* nColArray = new UINT[iCol];
	for(UINT i=0;i<iCol;i++)
	{
		nColArray[i]=i;
	}
	
	/*4取得指定时间区间内的记录数据*/
	if(!bGetAllDate)
	{
		m_txTable.Between(resTableDate,nColArray,iCol,4,iStartDate,iEndDate,true,true);
	}
	else
	{
		resTableDate.Clone(m_txTable);	
	}
	/*5根据界面给定类型条件剔除*///1:首发	2:配股	4:增发	8:送股	16:股权分置改革
	//modified by zhangxs 20101229==bug4718
	/*5根据界面给定类型条件剔除*///1,2:首发	3:送股	14:配股	15:增发
	if(uFlagType<=0)
	{
		return true;
	}
	if(uFlagType<32)//1，2，3，4在数据库里分别代表发行，增发，送股，配股
	{
		//modified by zhangxs 20101229
		/*std::set<int>	setType;
		if(uFlagType&1)
		{
			setType.insert(1);
		}
		if(uFlagType&2)
		{
			setType.insert(4);
		}
		if(uFlagType&4)
		{
			setType.insert(2);
		}
		if(uFlagType&8)
		{
			setType.insert(3);
		}*/
		/*	if(uFlagType&16)
		setType.insert(16);*/

		std::set<int>	setType;
		if(uFlagType&1 )
		{
			setType.insert(1);
			setType.insert(2);
		}
		if(uFlagType&4)
		{
			setType.insert(15);
		}
		if(uFlagType&8)
		{
			setType.insert(3);
		}
		if(uFlagType&2)
		{
			setType.insert(14);
		}
		if(setType.size()!=0)
		{
			resTableDate.EqualsAt(	
			resTableType,	//结果表(仅添加了列，行数必须为0)
			nColArray,		//结果列数组首址
			iCol,			//结果列个数
			2,				//条件列号，这里是类型id
			setType
			);
		}
	}

	/*6筛选样本集所有公司的结果*/
	if(resTableType.GetRowCount()>0)
	{
		resTableType.EqualsAt(resTable,nColArray,iCol,0,iInstitutionId);
	}
#ifdef _DEBUG
	strTable=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//添加进度条
	prw.SetPercent(pid,0.6);

	/*7填充其他数据*/
	resTable.InsertCol(1,Tx::Core::dtype_val_string);//插入券名称
	resTable.InsertCol(2,Tx::Core::dtype_val_string);//插入代码
	resTable.InsertCol(5,Tx::Core::dtype_val_string);//把事件类型转化为文字描述
    resTable.AddCol(Tx::Core::dtype_val_string);//插入省份
	resTable.AddCol(Tx::Core::dtype_val_string);//插入证监会行业代码

// 	std::unordered_map<int,CString> mapType;
// 	std::unordered_map<int,CString>::iterator itermap;
// 	TypeMapManage::GetInstance()->GetTypeMap(8,mapType);
	//暂时写死
	//modified by zhangxs 20101229
	//const CString strSHARE_CHANGE_TYPE[] = {_T(""),_T("首发"),_T("增发"),_T("送股"),_T("配股")};//1发行，2增发，3送股，4配股
	std::set<int>::iterator iter;
	CString CSRCode,province;
	for(iter=iSecurityId.begin();iter!=iSecurityId.end();++iter)
	{
		//取得交易实体ID
		int TradeID = *iter;
		GetSecurityNow(*iter);
		if(m_pSecurity==NULL)
		{
			continue;
		}
		//取得机构ID
		int isecuId = (int)m_pSecurity->GetInstitutionId();
		//根据交易实体ID取得样本的名称和外码；
		CString strName = m_pSecurity->GetName();
		CString strCode = m_pSecurity->GetCode();
		//取得证监会行业代码和注册省份
		CSRCode = m_pSecurity->GetCSRCIndustryCode();
		if(CSRCode == _T("-"))
			CSRCode = m_pSecurity->GetCSRCIndustryCode(1);
		province = m_pSecurity->GetRegisterProvance();
		//把券ID和交易实体ID对应起来。并且把交易实体ID放到表里。
		std::vector<UINT> vecInstiID;
		resTable.Find(0,isecuId,vecInstiID);
		std::vector<UINT>::iterator iteID;
		int thingId;
		for(iteID = vecInstiID.begin();iteID != vecInstiID.end();++iteID)
		{

			resTable.SetCell(0,*iteID,TradeID);
			resTable.SetCell(1,*iteID,strName);
			resTable.SetCell(2,*iteID,strCode);
			resTable.SetCell(13,*iteID,province);
			resTable.SetCell(14,*iteID,CSRCode);
			//得到事件类型的文字描述
            resTable.GetCell(4,*iteID,thingId);
			
			//itermap = mapType.find(thingId);
			//if((m_pSecurity != NULL)&&(thingId > 0 && thingId < 5))//(itermap!=mapType.end()))
			//{
			//	resTable.SetCell(5,*iteID, strSHARE_CHANGE_TYPE[thingId]);//itermap->second);
			//}
			//else
			//{
			//	resTable.SetCell(5, *iteID, CString(_T("-")));
			//}
			CString strType = _T("-");
			if(thingId == 1 || thingId == 2)
			{
				strType = _T("首发");
			}
			else if(thingId == 3)
			{
				strType = _T("送股");
			}
			else if(thingId == 14)
			{
				strType = _T("配股");
			}
			else if(thingId == 15)
			{
				strType = _T("增发");
			}
			resTable.SetCell(5,*iteID, strType);
		}
	}

	//表中的4列由单位[元]变为[万元]
	double dValue = Tx::Core::Con_doubleInvalid;
	for(int i=0;i<(int)resTable.GetRowCount();i++)
	{
		resTable.GetCell(8,i,dValue);
		if(fabs(dValue -Tx::Core::Con_doubleInvalid) > 0.000000001)
		{
			resTable.SetCell(8,i,dValue/10000);
		}

		resTable.GetCell(9,i,dValue);

		if(fabs(dValue -Tx::Core::Con_doubleInvalid) > 0.000000001)
		{
			resTable.SetCell(9,i,dValue/10000);
		}
		resTable.GetCell(10,i,dValue);
		if(fabs(dValue -Tx::Core::Con_doubleInvalid) > 0.000000001)
		{
			resTable.SetCell(10,i,dValue/10000);
		}
		resTable.GetCell(11,i,dValue);
		if(fabs(dValue -Tx::Core::Con_doubleInvalid) > 0.000000001)
		{
			resTable.SetCell(11,i,dValue/10000);
		}
	}
	resTable.DeleteCol(3,2);	//
	if (NULL != nColArray)
	{
		delete nColArray;
		nColArray = NULL;
	}

	prw.SetPercent(pid,1.0);

	return true;		
}//end of StatShareChange

//专项统计：股东人数
//add by lijw 2008-03-13
bool TxStock::StatHolderNumber(
							   std::set<int> & iSecurityId,			    //样本ID
							   int iStartDate,		 					//起始日期
							   int iEndDate,							//结束日期
							   Tx::Core::Table_Indicator &resTable,	    //结果数据表
							   bool IsEnd,								//判断是公告日期还是结束日期
							   bool bIssueAfterStart,					//是否剔除在起始日期后上市
							   int SpecifyCount					       //指定的股东人数
							   )
{
	Tx::Core::ProgressWnd prw;
	UINT pid=prw.AddItem(1,_T("股东人数统计..."),0.0);
	prw.Show(1000);

	if (iSecurityId.size() == 0)
	{
		prw.SetPercent(pid,1.0);
		return false;
	}
	
	/*1 在样本集中排除起始日期以后上市的样本，并把具有重复机构id的交易实体提炼出来*/
	std::set<int> setSecurityId;
	std::set<int> DifferentID;//为了不改变代码，这里存放的是交易实体ID
	std::set<int> SameFundId;//已出现过的重复机构id的交易实体ID

	//std::set<int> setUnIPOAfterStartID;
	int testDate = 0;
	std::set<int>::iterator iter = iSecurityId.begin();
	for(;iter != iSecurityId.end();iter++)
	{
		GetSecurityNow(*iter);
		if (m_pSecurity == NULL)
		{
			continue;
		}
		if (!m_pSecurity->IsStockA() && !m_pSecurity->IsStockB())
		{
			continue;
		}
		int fundid =(int)m_pSecurity->GetInstitutionId();
		
		testDate = m_pSecurity->GetIPOListedDate();
		if (testDate < 0 || testDate > iEndDate)//剔除截止期时未上市的股票
		{
			continue;
		}

		if (bIssueAfterStart && testDate > iStartDate)
		{
			continue;
		}

		if(setSecurityId.find(fundid) == setSecurityId.end())
		{
// 			if (testDate > iStartDate)//设置上市时间晚于起始日期的公司ID
// 			{
// 				setUnIPOAfterStartID.insert(fundid);
// 			}
			setSecurityId.insert(fundid);
			DifferentID.insert(*iter);
		}
		else
		{
			SameFundId.insert(*iter);
		}
	}

	if(DifferentID.size() == 0)
	{
		prw.SetPercent(pid,1.0);
		return false;
	}

	//默认的返回值状态。
	bool result = true;

	//2 配置取数据所需参数
	Tx::Core::Table_Indicator endTable;
	//f_institution_id,int型
	resTable.AddParameterColumn(Tx::Core::dtype_int4);
	//终止日期:f_start_date, int型
	resTable.AddParameterColumn(Tx::Core::dtype_int4);

	//f_institution_id,int型
	endTable.AddParameterColumn(Tx::Core::dtype_int4);
	//终止日期:f_start_date, int型
	endTable.AddParameterColumn(Tx::Core::dtype_int4);

	UINT varCfg[2];			//参数配置
	int varCount=2;			//参数个数	交易实体id，终止日期
	for(int i=0;i<2;i++)
	{
		varCfg[i]=i;
	}

	const int INDICATOR_INDEX=3;
	long iIndicator[INDICATOR_INDEX]=
	{			
		30300181,	//公告日期
		30300185,	//股东人数
		30300190	//平均持股
	};

	//设定指标列
	for (int i = 0; i <	INDICATOR_INDEX; i++)
	{
		GetIndicatorDataNow(iIndicator[i]);
		result = m_pLogicalBusiness->SetIndicatorIntoTable(m_pIndicatorData,varCfg,varCount,resTable);
		if(result==false)
		{
			break;
		}
		result = m_pLogicalBusiness->SetIndicatorIntoTable(m_pIndicatorData,varCfg,varCount,endTable);
		if(result==false)
		{
			break;
		}
	}

	std::vector<int> isecurityId;
	for(iter = setSecurityId.begin();iter != setSecurityId.end();++iter)
	{
		isecurityId.push_back(*iter);
	}

	/*3	根据是按公告日期还是按截止日期取数据*/

	std::vector<int> startDate;
	std::vector<int> endDate;
	startDate.push_back(iStartDate);
	endDate.push_back(iEndDate);
	int iColIdx = 2;//公告日期
	if(IsEnd)
	{
		iColIdx = 1;//截止日期
	}
		//根据开始日期取数据（备注：这里取出的数据是每个机构ID对应一条记录）
	result=m_pLogicalBusiness->GetData(resTable,isecurityId,startDate,iColIdx,0);
	if(result==false)
	{
			return false;
	}
		//根据终止日期取数据（备注：这里取出的数据是每个机构ID对应一条记录）
	result=m_pLogicalBusiness->GetData(endTable,isecurityId,endDate,iColIdx,0);
	if(result==false)
	{
		return false;
	}
		

#ifdef _DEBUG
	CString strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
#ifdef _DEBUG
	CString strTableend =endTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTableend);
#endif
#ifdef _DEBUG
    if(resTable.GetRowCount() != endTable.GetRowCount())
	{
		AfxMessageBox(_T("数据出现错误"));
		return false;
	}
#endif
	//因为这个地方的交易实体和机构ID是一一对应的，并且每个机构ID对应一条记录，
	//所以resTable,endTable里的记录条数应该是相等的，并且可以用下面的方法把这两个表链起来。
	//添加进度条
	prw.SetPercent(pid,0.6);
	//为插入交易实体ID做准备。

	resTable.InsertCol(0,Tx::Core::dtype_int4);//插入交易实体ID
	//插入这两列为了插入代码和名称。
	resTable.InsertCol(1,Tx::Core::dtype_val_string);//插入名称
	resTable.InsertCol(2,Tx::Core::dtype_val_string);//插入代码
	//为添加终止日期的股东人数作准备。
	resTable.InsertCol(7,Tx::Core::dtype_double);
	//为添加股东人数的增幅作准备。
	resTable.InsertCol(8,Tx::Core::dtype_double);
	//添加终止日期的平均持股做准备
	resTable.AddCol(Tx::Core::dtype_double);//10
	//添加平均持股的增幅做准备
	resTable.AddCol(Tx::Core::dtype_double);//11
#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//4把startTable和endTable这两个表连起来
	int iInstuId,position;
	std::set<int> iPositionSet;//为了保存少于给定的那个股东人数
	double dmanCount1,dmanCount2,average1,average2;
	double dManIncreasePercent = Tx::Core::Con_doubleInvalid;
	double dAverageIncreasePercent = Tx::Core::Con_doubleInvalid;
	int icount = endTable.GetRowCount();
	for(int i = 0;i < icount;i++)
	{
		endTable.GetCell(0,i,iInstuId);
		std::vector<UINT> vecInstiID2;
		resTable.Find(3,iInstuId,vecInstiID2);
		std::vector<UINT>::iterator iteID;
		for(iteID = vecInstiID2.begin();iteID != vecInstiID2.end();++iteID)
		{
			position = *iteID;
			endTable.GetCell(4,i,average1);
			endTable.GetCell(3,i,dmanCount1);

			if (average1 < 0.001 )
			{
				average1 = Tx::Core::Con_doubleInvalid;
			}

			if (dmanCount1 < 0.001)
			{
				dmanCount1 = Tx::Core::Con_doubleInvalid;
			}

// 			//如果开始表中公司在开始日期之后上市,将其设为无效数据
// 			if (setUnIPOAfterStartID.end() != setUnIPOAfterStartID.find(iInstuId))
// 			{
// 				resTable.SetCell(6, position, Tx::Core::Con_doubleInvalid);
// 				resTable.SetCell(9, position, Tx::Core::Con_doubleInvalid);
// 			}

			resTable.GetCell(6,position,dmanCount2);
			resTable.GetCell(9,position,average2);


			if(dmanCount1 < SpecifyCount && dmanCount2 < SpecifyCount)
			{
				iPositionSet.insert(position);
			}	

			//计算增幅
			if (dmanCount2 > 0.001 && dmanCount1 != Tx::Core::Con_doubleInvalid)
			{
				dManIncreasePercent = (dmanCount1 - dmanCount2)/dmanCount2*100;
			}
			else
			{
				dManIncreasePercent = Tx::Core::Con_doubleInvalid;
			}
			
			if (average2 > 0.001 && average1 != Tx::Core::Con_doubleInvalid)
			{
				dAverageIncreasePercent = (average1 - average2)/average2*100;
			}
			else
			{
				dAverageIncreasePercent = Tx::Core::Con_doubleInvalid;
			}

			//0 改为-
			if (average2 < 0.001 )
			{
				resTable.SetCell(9,i, Tx::Core::Con_doubleInvalid);
			}

			if (dmanCount2 < 0.001)
			{
				resTable.SetCell(6,i, Tx::Core::Con_doubleInvalid);
			}

			resTable.SetCell(7,position,dmanCount1);//终止日期的股东人数
			resTable.SetCell(8,position,dManIncreasePercent);//股东人数的的增幅
			resTable.SetCell(10,position,average1);//终止日期的平均持股
			resTable.SetCell(11,position,dAverageIncreasePercent);//平均持股的增幅
		}
	}
#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//5 设置不重复样本的样本名称和外码
	std::set<int>::iterator iter1;
	for(iter1 = DifferentID.begin();iter1!=DifferentID.end();++iter1)
	{
		//取得交易实体ID
		int TradeID = *iter1;
		GetSecurityNow(*iter1);
		if(m_pSecurity==NULL)
			continue;
		//取得机构ID
		int tempInstitutionid1 = m_pSecurity->GetInstitutionId();
	    //根据交易实体ID取得样本的名称和外码；
		CString strName,strCode;
		strName = m_pSecurity->GetName();
		strCode = m_pSecurity->GetCode();
		//把机构ID和交易实体ID对应起来。并且把交易实体ID放到表里。
		std::vector<UINT> vecInstiID3;
		resTable.Find(3,tempInstitutionid1,vecInstiID3);
		std::vector<UINT>::iterator iteID3;
		for(iteID3 = vecInstiID3.begin();iteID3 != vecInstiID3.end();++iteID3)
		{
			position = *iteID3;
			resTable.SetCell(0,position,TradeID);
			resTable.SetCell(1,position,strName);
			resTable.SetCell(2,position,strCode);
		}
	}
#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//6为不同的交易实体ID转化为相同的机构ID那样的样本填写数据，
	if(SameFundId.size() != 0)
	{
		//为计算方便，以机构ID进行排序
		resTable.Sort(3);
		//为不同的交易实体ID转化为相同的机构ID那样的样本填写数据，
		std::vector<UINT> vecFundID;
		std::set<int>::iterator iterSet,tempIter;
		for(iterSet = SameFundId.begin();iterSet != SameFundId.end();++iterSet)
		{
			GetSecurityNow(*iterSet);
			if (m_pSecurity == NULL)
			{
				continue;
			}
			int tempfundid = m_pSecurity->GetInstitutionId();//取得机构ID
			CString fundname = m_pSecurity->GetName();
			CString fundcode = m_pSecurity->GetCode();
			resTable.Find(3,tempfundid,vecFundID);
			if(vecFundID.size() == 0)
			{
				continue;
			}
			//取得在表中最小的位置
			std::set<int> tempset(vecFundID.begin(),vecFundID.end());
			tempIter = tempset.begin();
			//增加相同的记录
			std::set<int>::size_type icount = tempset.size();
			resTable.InsertRow(*tempIter,icount);
			double dstartman,dendman,damplitudeman;
			double dstartAve,dendAve,damplitudeAve;
			int position1,position2;
			for(;tempIter != tempset.end();++tempIter)
			{
				position1 = *tempIter;
				position2 = position1 + icount;
				resTable.SetCell(0,position1,*iterSet);
				resTable.SetCell(1,position1,fundname);
				resTable.SetCell(2,position1,fundcode);
				//取得其他数据
				resTable.GetCell(6,position2,dstartman);//起始日期的股东人数
				resTable.GetCell(7,position2,dendman);//终止日期的股东人数
				resTable.GetCell(8,position2,damplitudeman);//股东人数的增幅
				resTable.GetCell(9,position2,dstartAve);//起始日期平均持股
				resTable.GetCell(10,position2,dendAve);//终止日期平均持股
				resTable.GetCell(11,position2,damplitudeAve);//平均持股增幅
				//填充其他数据
				resTable.SetCell(6,position1,dstartman);//起始日期的股东人数
				resTable.SetCell(7,position1,dendman);//终止日期的股东人数
				resTable.SetCell(8,position1,damplitudeman);//股东人数的增幅
				resTable.SetCell(9,position1,dstartAve);//起始日期平均持股
				resTable.SetCell(10,position1,dendAve);//终止日期平均持股
				resTable.SetCell(11,position1,damplitudeAve);//平均持股增幅
			}
			vecFundID.clear();
			tempset.clear();
		}
	}

	//7 清理工作
	//删除起始日期的股东人数小于指定的股东人数的那些记录
	std::set<int>::const_reverse_iterator iterR;
	for(iterR = iPositionSet.rbegin();iterR != iPositionSet.rend();++iterR)
	{
		resTable.DeleteRow(*iterR);
	}
    //删除终止日期
	resTable.DeleteCol(5);
	//删除公告日期
	resTable.DeleteCol(4);
	//删除机构ID
	resTable.DeleteCol(3);	
#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//添加进度条
	prw.SetPercent(pid,1.0);
	return true;
}

//专项统计：股东人数
//add by guanyd		2007-08-10
//modify by lijw 2008-02-01
//bool TxStock::StatHolderNumber(
//			std::set<int> & iSecurityId,			//机构样本
//			int iStartDate,		 					//起始日期
//			int iEndDate,							//结束日期
//			Tx::Core::Table_Indicator &resTable,	//结果数据表
//			bool IsEnd,								//判断是公告日期还是结束日期
//			bool bIssueAfterStart,					//是否剔除在起始日期后上市
//			int iBelowHolderNumber					//是否剔除股东人数小于这个数目的样本
//			)
//{
////	Tx::Core::TxDateTime starttime=Tx::Core::TxDateTime::getCurrentTime();
//	ProgressWnd* pwd=Tx::Core::ProgressWnd::GetInstance();
//	UINT pid=prw.AddItem(1,_T("股东人数统计..."),0.0);
//	prw.Show(1000);
//	//将任意日期i回溯到上一个报告日期
//	//ChangeDateToReportdate(iStartDate,iEndDate);
//    if (iSecurityId.size() == 0)
//    {
//		prw.SetPercent(pid,1.0);
//		return false;
//    }
//    
//	//要取回全表然后剔除无用数据
//	std::vector<int> iInstitutionId;
//	if(!TransObjectToSecIns(iSecurityId,iInstitutionId,2))
//		return false;	
//
//	//默认的返回值状态。
//	bool result = true;
//	//清空数据
//	
//	Tx::Core::Table_Indicator m_txTable_Start,m_txTable_End;
//		
//	//公司代码:f_institution_id,int型
//	m_txTable_Start.AddParameterColumn(Tx::Core::dtype_int4);
//	//截止日期;f_end_date, int型		--实际是统计的起始日期
//	//统计初始年
//	m_txTable_Start.AddParameterColumn(Tx::Core::dtype_int4);	
//
//	//公司代码:f_institution_id,int型	
//	m_txTable_End.AddParameterColumn(Tx::Core::dtype_int4);
//	//截止日期;f_end_date, int型		--实际是统计的结束日期
//	//统计初始年
//	m_txTable_End.AddParameterColumn(Tx::Core::dtype_int4);	
//
//
//
//
//	UINT varCfg[2];			//参数配置
//	int varCount=2;			//参数个数	交易实体id，公告日
//
//	//交易提示 指标组
//
//	const int INDICATOR_INDEX=3;
//	long iIndicator[INDICATOR_INDEX]=
//	{			
//		30300181,	//公告日期
//		30300185,	//股东人数
//		30300190,	//平均持股
//
//	};
//
//	prw.SetPercent(pid,0.1);	
//	//设定指标列
//	for (int i = 0; i <	3; i++)
//	{
//	    GetIndicatorDataNow(iIndicator[i]);
//	
//		varCfg[0]=0;
//		varCfg[1]=1;
//
//		result = m_pLogicalBusiness->SetIndicatorIntoTable(m_pIndicatorData,varCfg,varCount,m_txTable_Start);
//		if(result==false)
//			break;
//
//		result = m_pLogicalBusiness->SetIndicatorIntoTable(m_pIndicatorData,varCfg,varCount,m_txTable_End);
//		if(result==false)
//			break;
//	}
//	if(result==false)
//		return false;
//
//	//根据之前3个步骤的设置进行数据读取，结果数据存放在table中
//	std::vector<int> vStartDate,vEndDate;
//	vStartDate.push_back(iStartDate);
//	vEndDate.push_back(iEndDate);
//	result=m_pLogicalBusiness->GetData<int>(m_txTable_Start,iInstitutionId,vStartDate,1,0);
//	if(result==false)
//		return false;
//#ifdef _DEBUG
//	CString strTable=m_txTable_Start.TableToString();
//	Tx::Core::Commonality::String().StringToClipboard(strTable);
//#endif
//	result=m_pLogicalBusiness->GetData<int>(m_txTable_End,iInstitutionId,vEndDate,1,0);
//	if(result==false)
//		return false;
//#ifdef _DEBUG
//	strTable=m_txTable_End.TableToString();
//	Tx::Core::Commonality::String().StringToClipboard(strTable);
//#endif
//	if((m_txTable_Start.GetRowCount()==0)&&(m_txTable_Start.GetRowCount()==0))
//		return true;
//	std::vector<int>::iterator iterId;
//	int i=0;
//	for(iterId=iInstitutionId.begin();iterId!=iInstitutionId.end();iterId++,i++)
//	{
//		m_txTable_Start.SetCell(0,i,*iterId);
//		m_txTable_End.SetCell(0,i,*iterId);
//	}
//	prw.SetPercent(pid,0.6);
//	//XXXX-两年数据不一致时，将较少数据年扩充到一致-XXXX
//	//这是错误的逻辑，即使数据行相同，也有可能是前面几行是已经摘牌的公司，后面同样行数的上市公司
//	//所以每一行都要验证
//	AlignRows(m_txTable_Start,0,m_txTable_End,0);
//
//	resTable.AddCol(Tx::Core::dtype_val_string);			//0公司外码
//	resTable.AddCol(Tx::Core::dtype_val_string);			//1公司名称
//	
//	resTable.AddCol(m_txTable_Start.GetColType(3));		//2人数起始数据
//	resTable.AddCol(m_txTable_End.GetColType(3));		//3人数截止数据
//	
//	//resTable.AddCol(Tx::Core::dtype_val_string);		//4人数变化百分比表达式
//	resTable.AddCol(Tx::Core::dtype_double);		//4人数变化百分比表达式
//
//	resTable.AddCol(m_txTable_Start.GetColType(4));		//5平均持股起始数据
//	resTable.AddCol(m_txTable_End.GetColType(4));		//6平均持股截止数据
//	
//	//resTable.AddCol(Tx::Core::dtype_val_string);		//7平局持股变化百分比表达式
//	resTable.AddCol(Tx::Core::dtype_double);		//7平局持股变化百分比表达式
//
//		
//	resTable.AddRow(m_txTable_Start.GetRowCount());
//	for(UINT i=0;i<m_txTable_Start.GetRowCount();i++)
//	{
//			int id=0;
//		m_txTable_Start.GetCell(0,i,id);
//
//		std::set <int> items;
//		m_pFunctionDataManager->GetItems(id,items);
//
//		std::set<int>::iterator iter = items.begin();
//		if(!items.empty())
//		{
//			id=*iter;
//
//			this->GetSecurityNow(id);
//
//			 if(m_pSecurity==0)
//				 continue;
//
//			CString sCode=m_pSecurity->GetCode();
//			CString sName;
//			sName = "-";
//			//假设券的每个交易实体均属于同一个行业
//
//
//			sName=m_pSecurity->GetSecurity1Name();
//			resTable.SetCell(0,i,sName);	
//			resTable.SetCell(1,i,sCode);
//		}
//	}
//
//	
//
//
//	resTable.CopyColumnData(m_txTable_Start,3,2);
//	resTable.CopyColumnData(m_txTable_End,3,3);
//
//	resTable.CopyColumnData(m_txTable_Start,4,5);
//	resTable.CopyColumnData(m_txTable_End,4,6);
//
//	std::set<int> setNoStart;
//	for(UINT k=0;k<resTable.GetRowCount();k++)
//	{
//		double iHolderStart,iHolderEnd;
//		double iShareStart,iShareEnd;
//		
//		resTable.GetCell(2,k,iHolderStart);
//		resTable.GetCell(3,k,iHolderEnd);
//		resTable.GetCell(5,k,iShareStart);
//		resTable.GetCell(6,k,iShareEnd);
//
//
//		double dfHolderRate,dfShareRate;
//		
//		if(bIssueAfterStart)//如果要剔除在起始日期后上市的数据的话则，则统计空数据即可
//		{
//			if((iHolderStart==0)||(iHolderStart==INT_MIN))
//				setNoStart.insert(k);
//		}
//		/*
//		CString  fHolderRate,fShareRate;
//		if((iHolderStart>0)&&(iHolderEnd>0))
//		{
//			//fHolderRate.Format("%.2f%%",(iHolderEnd/iHolderStart-1)*100);
//			
//		}
//		else
//		{
//			fHolderRate.Format("-");
//		}
//
//		if((iShareStart>0)&&(iShareEnd>0))
//		{
//			fShareRate.Format("%.2f%%",(iShareEnd/iShareStart-1)*100);
//		}
//		else
//		{
//			fShareRate.Format("-");
//		}
//		*/
//		if((iHolderEnd==0)||(iHolderStart==0)||iHolderEnd==iHolderStart)
//			dfHolderRate=Tx::Core::Con_doubleInvalid;
//		else
//			dfHolderRate=(iHolderEnd/iHolderStart-1)*100;
//
//		if((iShareEnd==0)||(iShareStart==0)||iShareEnd==iShareStart)
//			dfShareRate=Tx::Core::Con_doubleInvalid;
//		else
//			dfShareRate=(iShareEnd/iShareStart-1)*100;
//		resTable.SetCell(4,k,dfHolderRate);
//		resTable.SetCell(7,k,dfShareRate);
//	}
//
//	if(bIssueAfterStart)
//	{
//		for(std::set<int>::iterator iterNoStart=setNoStart.end();iterNoStart!=setNoStart.end();iterNoStart++)
//		{
//			
//			resTable.DeleteRow(*iterNoStart);
//		}
//	}
//	
//	//不为0则设置了剔除条件
//	//将结果表中起始人数和截止人数小于设定值的数据剔除
//	if(iBelowHolderNumber!=0)
//	{	
//		UINT iCol=resTable.GetColCount();
//		UINT* nColArray = new UINT[iCol];
//		for(UINT i=0;i<iCol;i++)
//			nColArray[i]=i;
//
//		if(iBelowHolderNumber<0)
//			return true;
//		Tx::Core::Table_Indicator TableCutLowNumber;
//		TableCutLowNumber.CopyColumnInfoFrom(resTable);
//
//		
//		resTable.Between(TableCutLowNumber,nColArray,iCol,2,(double)iBelowHolderNumber,9999999999.0);
//		resTable.Clear();
//		resTable.CopyColumnInfoFrom(TableCutLowNumber);
//
//		TableCutLowNumber.Between(resTable,nColArray,iCol,3,(double)iBelowHolderNumber,9999999999.0);
//		
//		delete nColArray;
//	}
//	
//	resTable.Sort(1);
//	ReDefColTypeDoubleToInt(resTable,2);
//	ReDefColTypeDoubleToInt(resTable,3);
//	ReDefColTypeDoubleToInt(resTable,5);
//	ReDefColTypeDoubleToInt(resTable,6);
//
//	Tx::Core::TxDateTime endtime=Tx::Core::TxDateTime::getCurrentTime();
//
//	TRACE("\n--------------------------------------------------------------------------------------------------------------------------------\n");
//	TRACE("统计%d只股票(股东人数)用时:%.6f",iSecurityId.size(),(endtime-starttime).GetTotalSeconds());
//	TRACE("\n--------------------------------------------------------------------------------------------------------------------------------\n");
//	
//	prw.SetPercent(pid,1.0);
//	return true;		
//}//end of StatHolderNumber

//板块：交易提示
bool TxStock::BlockTradePrompt(
				std::set<int> iSecurityId,		//交易实体ID
				int iStartDate,						//起始日
				int iEndDate,						//终止日
				Tx::Core::Table_Indicator &resTable	//结果数据表
				)
{
	Tx::Core::Table_Indicator resTableFPS,resTableFXZF,resTableFXSS,resTablePG;
	
	resTable.AddCol(Tx::Core::dtype_val_string);	//外码
	resTable.AddCol(Tx::Core::dtype_val_string);	//名称
	resTable.AddCol(Tx::Core::dtype_val_string);	//类型
	resTable.AddCol(Tx::Core::dtype_val_string);	//信息
	
	//2007-11-28 cenxw add
	resTable.AddCol(Tx::Core::dtype_int4); //ID

	//step1
//	Tx::Core::ProgressWnd* pProgressWnd = Tx::Core::ProgressWnd::GetInstance();
	Tx::Core::ProgressWnd prw;
	//step2
	CString sProgressPrompt;
	sProgressPrompt.Format(_T("交易提示..."));
	UINT progId = prw.AddItem(1,sProgressPrompt, 0.0);
	//step3
	prw.Show(15);
	prw.EnableCancelButton(true);

	int iRowCount=0;
	//step4
	prw.SetPercent(progId,0.1);

	this->ColligationTradePromptFPS(iSecurityId,iStartDate,iEndDate,resTableFPS);
	{

		//处理分送配表

		for(UINT k=0;k<resTableFPS.GetRowCount();k++)
		{			
			if(prw.IsCanceled()==true)
			{
				resTable.DeleteRow(0,resTable.GetRowCount());
				prw.SetPercent(progId, 1.0);
				prw.SetText(progId, sProgressPrompt);
				prw.Hide();
				prw.EnableCancelButton(false);
				return false;
			}
			resTable.AddRow();

			int id=0;//记录实体id
			int iTypeId;
			resTableFPS.GetCell(2,k,id);

			std::set <int> items;//用于查找同券的实体
			std::set<int>::iterator s_itType;

			std::unordered_map<int,CString> mType;//用于缓存类型索引
			std::unordered_map<int,CString>::iterator m_itType;

			//分别存储除权日，登记日，比率，派现数额
			int iExDate,iRegDate;
			double dRate,dPX;
			
			//外码和名称
			CString sCode,sName;

			//分别存储除权日，登记日，比率，派现数额，整合数据
			CString sExDate,sRegDate,sRate,sPX,sCon;

			

			m_pFunctionDataManager->GetItems(id,items);
			//items()
			if(!items.empty())
			{
				s_itType = items.begin();
				id=*s_itType;
	
				this->GetSecurityNow(id);
				if(m_pSecurity==0)
					continue;
	
	
				sCode=m_pSecurity->GetCode();
				sName=m_pSecurity->GetSecurity1Name();
				resTable.SetCell(0,iRowCount+k,sName);	
				resTable.SetCell(1,iRowCount+k,sCode);
				resTable.SetCell(4,iRowCount+k,(int)m_pSecurity->GetId());
			}
			
			resTableFPS.GetCell(3,k,iTypeId);
			this->GetIndexDat(18,mType);
			m_itType=mType.find(iTypeId);
			if (m_itType!=mType.end())
				resTable.SetCell(2,iRowCount+k,m_itType->second);

			resTableFPS.GetCell(4,k,iExDate);
				if((iExDate<19000000)||(iExDate>29990000))
					sExDate.Format(_T("-"));
				else
					sExDate.Format(_T("%d-%.2d-%.2d"),iExDate/10000,iExDate%10000/100,iExDate%100 );
			resTableFPS.GetCell(5,k,iRegDate);
				if((iRegDate<19000000)||(iRegDate>29990000))					
					sRegDate.Format(_T("-"));
				else
					sRegDate.Format(_T("%d-%.2d-%.2d"),iRegDate/10000,iRegDate%10000/100,iRegDate%100);
			resTableFPS.GetCell(6,k,dRate);
				if((dRate<0)||(dRate>9999999))					
					sRate.Format(_T("-"));
				else
					sRate.Format(_T("%.2f"),dRate);
			resTableFPS.GetCell(7,k,dPX);				
				if((dPX<0)||(dPX>999999))					
					sPX.Format(_T("-"));
				else
					sPX.Format(_T("%.2f"),dPX);
			
			sCon.Format(_T("除权日:%s, 登记日:%s, 送股比率:%s, 派现数额:%s"),sExDate,sRegDate,sRate,sPX);
			resTable.SetCell(3,iRowCount+k,sCon);
		}
	}
	prw.SetPercent(progId,0.3);
	if(prw.IsCanceled()==true)
	{
		resTable.DeleteRow(0,resTable.GetRowCount());
		prw.SetPercent(progId, 1.0);
		prw.SetText(progId, sProgressPrompt);
		prw.Hide();
		prw.EnableCancelButton(false);
		return false;
	}

	iRowCount+=resTableFPS.GetRowCount();
	this->ColligationTradePromptFXZF(iSecurityId,iStartDate,iEndDate,resTableFXZF);
	{
		//处理上市增发

		for(UINT k=0;k<resTableFXZF.GetRowCount();k++)
		{	
			if(prw.IsCanceled()==true)
			{
				resTable.DeleteRow(0,resTable.GetRowCount());
				prw.SetPercent(progId, 1.0);
				prw.SetText(progId, sProgressPrompt);
				prw.Hide();
				prw.EnableCancelButton(false);
				return false;
			}
			resTable.AddRow();

			int id=0;//记录实体id
			int iTypeId;
			resTableFXZF.GetCell(1,k,id);

			std::set <int> items;//用于查找同券的实体
			std::set<int>::iterator s_itType;

			std::unordered_map<int,CString> mType;//用于缓存类型索引
			std::unordered_map<int,CString>::iterator m_itType;

			//分别存储发行量，发行价，发行日，上市日
			double dIssueNumber,dIssuePrice;
			int iIssueDate,iListingDate;

			//外码和名称
			CString sCode,sName;

			//分别存储发行量，发行价，发行日，上市日，整合数据
			CString sIssueNumber,sIssuePrice,sIssueDate,sListingDate,sCon;


			m_pFunctionDataManager->GetItems(id,items);
			//items()
			if(!items.empty())
			{
				s_itType = items.begin();
				id=*s_itType;

				this->GetSecurityNow(id);
				if(m_pSecurity==0)
					continue;

				sCode=m_pSecurity->GetCode();
	
				sName=m_pSecurity->GetSecurity1Name();
				resTable.SetCell(0,iRowCount+k,sName);	
				resTable.SetCell(1,iRowCount+k,sCode);
				resTable.SetCell(4,iRowCount+k,(int)m_pSecurity->GetId());
			}
			
			resTableFXZF.GetCell(4,k,iTypeId);
			this->GetIndexDat(8,mType);//event_type_index.dat
			m_itType=mType.find(iTypeId);
			if(m_itType!=mType.end())
				resTable.SetCell(2,iRowCount+k,m_itType->second);
			else
				resTable.SetCell(2,iRowCount+k,CString(_T("增发")));

			resTableFXZF.GetCell(2,k,dIssueNumber);
				if(dIssueNumber<0)
					sIssueNumber.Format(_T("-"));
				else
					sIssueNumber.Format(_T("%.2f"),dIssueNumber);
			resTableFXZF.GetCell(3,k,dIssuePrice);
				if(dIssuePrice<0)			
					sIssuePrice.Format(_T("-"));
				else
					sIssuePrice.Format(_T("%.2f"),dIssuePrice);
			resTableFXZF.GetCell(5,k,iIssueDate);
				if((iIssueDate<0)||(iIssueDate>29990000))					
					sIssueDate.Format(_T("-"));
				else
					sIssueDate.Format(_T("%d-%.2d-%.2d"),iIssueDate/10000,iIssueDate%10000/100,iIssueDate%100 );
			resTableFXZF.GetCell(6,k,iListingDate);				
				if((iListingDate<0)||(iListingDate>29990000))					
					sListingDate.Format(_T("-"));
				else
					sListingDate.Format(_T("%d-%.2d-%.2d"),iListingDate/10000,iListingDate%10000/100,iListingDate%100 );
			
			sCon.Format(_T("发行量:%s, 发行价:%s, 发行日:%s, 上市日:%s"),sIssueNumber,sIssuePrice,sIssueDate,sListingDate);
			resTable.SetCell(3,iRowCount+k,sCon);
		}
	}
	prw.SetPercent(progId,0.6);
	if(prw.IsCanceled()==true)
	{
		resTable.DeleteRow(0,resTable.GetRowCount());
		prw.SetPercent(progId, 1.0);
		prw.SetText(progId, sProgressPrompt);
		prw.Hide();
		prw.EnableCancelButton(false);
		return false;
	}

	iRowCount+=resTableFXZF.GetRowCount();
	this->ColligationTradePromptFXSS(iSecurityId,iStartDate,iEndDate,resTableFXSS);
	{
		//处理上市增发

		for(UINT k=0;k<resTableFXSS.GetRowCount();k++)
		{	
			if(prw.IsCanceled()==true)
			{
				resTable.DeleteRow(0,resTable.GetRowCount());
				prw.SetPercent(progId, 1.0);
				prw.SetText(progId, sProgressPrompt);
				prw.Hide();
				prw.EnableCancelButton(false);
				return false;
			}
			resTable.AddRow();

			int id=0;//记录实体id
			int iTypeId;
			resTableFXSS.GetCell(1,k,id);

			std::set <int> items;//用于查找同券的实体
			std::set<int>::iterator s_itType;

			std::unordered_map<int,CString> mType;//用于缓存类型索引
			std::unordered_map<int,CString>::iterator m_itType;

			//分别存储发行量，发行价，发行日，上市日
			double dIssueNumber,dIssuePrice;
			int iIssueDate,iListingDate;

			//外码和名称
			
			CString sCode,sName;

			//分别存储发行量，发行价，发行日，上市日，整合数据
			CString sIssueNumber,sIssuePrice,sIssueDate,sListingDate,sCon;



			m_pFunctionDataManager->GetItems(id,items);
			//items()
			if(!items.empty())
			{
				s_itType = items.begin();
				id=*s_itType;
	
				this->GetSecurityNow(id);
				if(m_pSecurity==0)
					continue;
	
				sCode=m_pSecurity->GetCode();
	
				sName=m_pSecurity->GetSecurity1Name();
				resTable.SetCell(0,iRowCount+k,sName);	
				resTable.SetCell(1,iRowCount+k,sCode);
				resTable.SetCell(4,iRowCount+k,(int)m_pSecurity->GetId());
			}
			
			resTableFXSS.GetCell(4,k,iTypeId);
			this->GetIndexDat(8,mType);//event_type_index.dat
			m_itType=mType.find(iTypeId);
			if(m_itType!=mType.end())
				resTable.SetCell(2,iRowCount+k,m_itType->second);
			else
				resTable.SetCell(2,iRowCount+k,CString(_T("增发")));

			resTableFXSS.GetCell(2,k,dIssueNumber);
				if(dIssueNumber<0)
					sIssueNumber.Format(_T("-"));
				else
					sIssueNumber.Format(_T("%.2f"),dIssueNumber);
			resTableFXSS.GetCell(3,k,dIssuePrice);
				if(dIssuePrice<0)			
					sIssuePrice.Format(_T("-"));
				else
					sIssuePrice.Format(_T("%.2f"),dIssuePrice);
			resTableFXSS.GetCell(5,k,iIssueDate);
				if((iIssueDate<0)||(iIssueDate>29990000))					
					sIssueDate.Format(_T("-"));
				else
					sIssueDate.Format(_T("%d-%.2d-%.2d"),iIssueDate/10000,iIssueDate%10000/100,iIssueDate%100 );
			resTableFXSS.GetCell(6,k,iListingDate);				
				if((iListingDate<0)||(iListingDate>29990000))					
					sListingDate.Format(_T("-"));
				else
					sListingDate.Format(_T("%d-%.2d-%.2d"),iListingDate/10000,iListingDate%10000/100,iListingDate%100 );
			
			sCon.Format(_T("发行量:%s, 发行价:%s, 发行日:%s, 上市日:%s"),sIssueNumber,sIssuePrice,sIssueDate,sListingDate);
			resTable.SetCell(3,iRowCount+k,sCon);
		}
	}
	prw.SetPercent(progId,0.9);
	if(prw.IsCanceled()==true)
	{
		resTable.DeleteRow(0,resTable.GetRowCount());
		prw.SetPercent(progId, 1.0);
		prw.SetText(progId, sProgressPrompt);
		prw.Hide();
		prw.EnableCancelButton(false);
		return false;
	}

	iRowCount+=resTableFXSS.GetRowCount();
	this->ColligationTradePromptPG(iSecurityId,iStartDate,iEndDate,resTablePG);
	{

		//处理分送配表

		for(UINT k=0;k<resTablePG.GetRowCount();k++)
		{			
			if(prw.IsCanceled()==true)
			{
				resTable.DeleteRow(0,resTable.GetRowCount());
				prw.SetPercent(progId, 1.0);
				prw.SetText(progId, sProgressPrompt);
				prw.Hide();
				prw.EnableCancelButton(false);
				return false;
			}
			resTable.AddRow();

			int id=0;//记录实体id

			resTablePG.GetCell(1,k,id);

			std::set <int> items;//用于查找同券的实体
			std::set<int>::iterator s_itType;

			//分别存储除权日，登记日，比率，派现数额
			int iExDate,iRegDate;
			double dRate,dPGPrice;
			
			//外码和名称
			CString sCode,sName;

			//分别存储除权日，登记日，比率，派现数额，整合数据
			CString sExDate,sRegDate,sRate,sPGPrice,sCon;

			

			m_pFunctionDataManager->GetItems(id,items);
			//items()
			if(!items.empty())
			{
				s_itType = items.begin();
				id=*s_itType;
	
				this->GetSecurityNow(id);
				if(m_pSecurity==0)
					 continue;
	
	
				sCode=m_pSecurity->GetCode();
				sName=m_pSecurity->GetSecurity1Name();
				resTable.SetCell(0,iRowCount+k,sName);	
				resTable.SetCell(1,iRowCount+k,sCode);
				resTable.SetCell(4,iRowCount+k,(int)m_pSecurity->GetId());
			}
				
			CString sTemp=_T("配股");
			resTable.SetCell(2,iRowCount+k,sTemp);

			resTablePG.GetCell(3,k,iExDate);
				if((iExDate<19000000)||(iExDate>29990000))
					sExDate.Format(_T("-"));
				else
					sExDate.Format(_T("%d-%.2d-%.2d"),iExDate/10000,iExDate%10000/100,iExDate%100 );
			resTablePG.GetCell(2,k,iRegDate);
				if((iRegDate<19000000)||(iRegDate>29990000))					
					sRegDate.Format(_T("-"));
				else
					sRegDate.Format(_T("%d-%.2d-%.2d"),iRegDate/10000,iRegDate%10000/100,iRegDate%100 );
			resTablePG.GetCell(9,k,dRate);
				if((dRate<0)||(dRate>9999999))					
					sRate.Format(_T("-"));
				else
					sRate.Format(_T("%.2f"),dRate);
			resTablePG.GetCell(6,k,dPGPrice);				
				if((dPGPrice<0)||(dPGPrice>999999))					
					sPGPrice.Format(_T("-"));
				else
					sPGPrice.Format(_T("%.2f"),dPGPrice);
			
			sCon.Format(_T("除权日:%s, 登记日:%s, 送股比率:%s, 配股价格:%s"),sExDate,sRegDate,sRate,sPGPrice);
			resTable.SetCell(3,iRowCount+k,sCon);
		}
	}
	iRowCount+=resTablePG.GetRowCount();
	prw.SetPercent(progId, 1.0);
	prw.SetText(progId, sProgressPrompt);
	prw.EnableCancelButton(false);

	return true;
}//End of BlockTradePrompt






//////////////////////////////////////局部方法//////////////////////////////
////////////////此部分方法不是直接的业务方法，而是业务方法辅助//////////////
//guanyd
//2007-09-04
//将任意选定日期回溯到第一个报告日期
bool TxStock::ChangeDateToReportdate(int& iStartDate,int& iEndDate)
{
	int n1=iStartDate%10000,n2=iEndDate%10000;
	int y1=iStartDate/10000,y2=iEndDate/10000;
	if(((n1<0331)&&(n1>=0101))||(n1==1231))
	{

		iStartDate=(y1-1)*10000+1231;
	}
	else 
		if((n1>=331)&&(n1<630))
		{
			iStartDate=y1*10000+331;
		}
		else
			if((n1>=630)&&(n1<930))
			{
				iStartDate=y1*10000+630;
			}
			else
				if((n1>=930)&&(n1<1231))
				{
					iStartDate=y1*10000+930;
				}
	if(((n2<0331)&&(n2>=0101))||(n2==1231))
	{

		iEndDate=(y2-1)*10000+1231;
	}
	else 
		if((n2>=331)&&(n2<630))
		{
			iEndDate=y2*10000+331;
		}
		else
			if((n2>=630)&&(n2<930))
			{
				iEndDate=y2*10000+630;
			}
			else
				if((n2>=930)&&(n2<1231))
				{
					iEndDate=y2*10000+930;
				}
	return true;
}//End Of ChangeDateToReportdate

//guanyd
//2007-09-04
//将交易实体id转为券id或者机构id
bool TxStock::TransObjectToSecIns(
		std::set<int>	sObjectId,		//交易实体id
		std::set<int> &sSecInsId,		//券id或机构id
		int iType						//id类型：1:券id	2:机构id
		)
{
	int ii = sObjectId.size();
	if(ii<=0)
		return false;

	std::set<int>::iterator iter=sObjectId.begin();
	
	while(iter!=sObjectId.end())
	{
		GetSecurityNow(*iter++);
		if(m_pSecurity!=NULL)
			switch(iType)
			{
			case 1:
				sSecInsId.insert( m_pSecurity->GetSecurity1Id());
				break;
			case 2:
				sSecInsId.insert( m_pSecurity->GetInstitutionId());
				break;
			default:
				;
			}
	}
	return true;
}//End Of TransObjectToSecIns
bool TxStock::TransObjectToSecIns(
		std::set<int>	sObjectId,		//交易实体id
		std::vector<int> &sSecInsId,		//券id或机构id
		int iType						//id类型：1:券id	2:机构id
		)
{
	int ii = sObjectId.size();
	if(ii<=0)
		return false;

	std::set<int>::iterator iter=sObjectId.begin();
	
	while(iter!=sObjectId.end())
	{
		GetSecurityNow(*iter++);
		if(m_pSecurity!=NULL)
			switch(iType)
			{
			case 1:
				sSecInsId.push_back( m_pSecurity->GetSecurity1Id());
				break;
			case 2:
				sSecInsId.push_back( m_pSecurity->GetInstitutionId());
				break;
			default:
				;
			}
	}
	return true;
}//End Of TransObjectToSecIns
//Add by Guanyd
//2007-08-28
//将两张表根据券id对齐，因为id的全集为1600左右，而且在本表中出现不会重复
bool TxStock::AlignRows(
					Tx::Core::Table_Indicator& resTableFir,		//较大的表
					int iFirCol,								//大表中扩展式保证数据一致的列
					Tx::Core::Table_Indicator& resTableSec,		//被扩展的表
					int iSecCol									//扩展表中保证数据一致的列
					)

{
	UINT iFirCount=resTableFir.GetRowCount(),iSecCount=resTableSec.GetRowCount();
	for(UINT i=0,j=0;(i<iFirCount)||(j<iSecCount);)
	{
		int iFirId,iSecId;
		resTableFir.GetCell(iFirCol,i,iFirId);
		resTableSec.GetCell(iSecCol,j,iSecId);
		if(iFirId!=iSecId)
		{
			if(iFirId>iSecId)
			{
				//第二张表数据落后，补齐
				j++;
				if(j<resTableSec.GetRowCount())
				{
					resTableSec.InsertRow(j);
				}
				else
				{
					resTableSec.AddRow();
				}
				resTableSec.SetCell(0,j,iFirId);
				resTableSec.SetCell(1,j,0);
				resTableSec.SetCell(2,j,0);
				resTableSec.SetCell(3,j,0.0);
				resTableSec.SetCell(4,j,0.0);
			}
			else
			{
				//第一张表数据落后，补齐
				i++;
				if(i<resTableFir.GetRowCount())
				{
					resTableFir.InsertRow(i);
				}
				else
				{
					resTableFir.AddRow();
				}
				resTableFir.SetCell(0,i,iSecId);
				resTableFir.SetCell(1,i,0);
				resTableFir.SetCell(2,i,0);
				resTableFir.SetCell(3,i,0.0);
				resTableFir.SetCell(4,i,0.0);
			}
		}
		else
		{
			//两行目前对齐可一起向下
			i++;
			j++;
		}
	
	}
	return true;
}//End Of AlignRows

//cenxw
//获取本地类型文件，读入map
BOOL TxStock::GetIndexDat(int file_id, std::unordered_map<int,CString>& indexdat_map)
{
	TypeMapManage::GetInstance()->GetTypeMap(file_id,indexdat_map);
	return true;


	CString fileName=Tx::Core::SystemPath::GetInstance()->GetSystemDataPath()+_T("\\TypeMap\\")+IndexDatFileName[file_id];
	struct IndexStruct isd_d;
	CFile wf;
	try
	{
		if (!wf.Open(fileName,CFile::modeRead))
		{
			DWORD ecode=::GetLastError();
			TRACE("Open %s Error ecode %d",fileName,ecode);
			return FALSE;
		}
	}
	catch (CException* e)
	{
		TCHAR estr[255];
		e->GetErrorMessage(estr,255);
		e->Delete();
		TRACE("%s",estr);
		return FALSE;
	}
	long len = (long)wf.GetLength();
	long rec_count = len/sizeof(IndexStruct);
	for (int i=0;i<rec_count;i++)
	{
		CString cname;
		wf.Read(&isd_d,sizeof(IndexStruct));
		cname.Format(_T("%s"),isd_d.description);
		indexdat_map.insert(std::make_pair(isd_d.id,cname));
	}
	return TRUE;
}

//根据指定表为索引，从指定的列关联添加列数据
bool TxStock::FillColumn(
				Tx::Core::Table_Indicator &m_transTable,		//关联表
				int addColumn,						//关联表中数据的列
				Tx::Core::Table_Indicator &m_resTableAddColumn,	//源表
				int indexColumn,					//源表作为map索引的列
				int	insertColumn					//要插入数据的列		
				)
{
	//有表为空则操作无意义
	if((m_transTable.GetRowCount()==0)||(m_resTableAddColumn.GetRowCount()==0))
		return false;

	//列超界错误
	if((UINT)addColumn>m_transTable.GetColCount())
		return false;


	std::set<int> setEventId;
	int	iEventId;
	for(UINT k=0;k<m_resTableAddColumn.GetRowCount();k++)
	{
		m_resTableAddColumn.GetCell(0,k,iEventId);
		setEventId.insert(iEventId);
	}
	//	m_transTable.CopyColumnData(m_resTableAddColumn,0,0);
	std::set<int>::iterator iterEventId=setEventId.begin();
	int k=0;
	while(iterEventId!=setEventId.end())
	{

		m_transTable.AddRow(1);
		int t=*iterEventId;
		m_transTable.SetCell(0,k,*iterEventId);
		iterEventId++;
		k++;
	}

	bool result=m_pLogicalBusiness->GetData(m_transTable);


	if((m_transTable.GetActualSize()==0)||(m_transTable.GetColCount()!=2))
		return false;	
	std::unordered_map<int,int> mMap;
	int iTransTableCell,iResTableCell;
	for(UINT i=0;i<m_transTable.GetRowCount();i++)
	{
		m_transTable.GetCell(0,i,iTransTableCell);
		mMap.insert(std::make_pair(iTransTableCell,i));
	}

	if ((UINT)insertColumn>=m_resTableAddColumn.GetColCount())
	{
		m_resTableAddColumn.AddCol(m_transTable.GetColumn(1)->dtype);
	}
	else	
		if(m_resTableAddColumn.GetColType(insertColumn)!=m_transTable.GetColType(addColumn))
		{
			//判断两列数据类型是否一致，否则不做操作。	
			ASSERT(0);
			return false;
		}



	for(UINT i=0;i<m_resTableAddColumn.GetRowCount();i++)
	{
		int p=(int)m_resTableAddColumn.GetColType(indexColumn);
		m_resTableAddColumn.GetCell(indexColumn,i,iResTableCell);
		std::unordered_map<int,int>::iterator iter=mMap.find(iResTableCell);
		if (iter!=mMap.end())
		{
			Tx::Core::VariantData val;
			val.data_type=m_transTable.GetColumn(1)->dtype;
			m_transTable.GetCell(1,iter->second,val);
			m_resTableAddColumn.SetCell(insertColumn,i,val);
		}
	}

	return true;
}//end of FillColumn

bool TxStock::GetPEPBValue(Tx::Core::Table& resultTable, std::set<int> iSecurityId, UINT nStartDate, UINT nEndDate, int iSJZQ, int iHZYB, double dwl, int iJQFS, int iCWSJ, UINT nFiscalyear, UINT nFiscalyearquater, int iJSFF, bool bClosePrice,bool bPE, bool bPB, std::unordered_map<int,Tx::Core::Unique_Pos_Map*>* pid_posinfo_map /* = NULL  */)
{
	//////////////////////////////////////////////////////////////////////////
	// 与zway约定,resultTable的第一行,除前2列外,将日期转为double值,写入
	// 前2列分别为券id,券名称
	/************************************************************************/
	/* PE=每股市价*总股本/净利润                                            */
	/* 股价---根据日期,从历史行情取收盘价(或使用均价---暂不考虑)			*/
	/* 总股本-----总股本(股本结构变化表)(每天)---已可取得					*/
	/* 净利润-----归属于母公司所有者的净利润(一般行业利润表)				*/
	/************************************************************************/
	// modified by zhoup 2008.11.18
	//1.股本数据从TxShareData结构取得,不再使用碎文件模式
	//	TxShareData *pShareData = m_pSecurity->GetTxShareDataByDate(iDate);
	//2.利润做调整,要使用对应股本对应的利润
	//3.增加对当日的PE/PB统计
	if (iSecurityId.empty())
		return false;
	if (nEndDate<nStartDate)
		return false;
	if (iSecurityId.empty())
		return false;
	resultTable.Clear();
	// resultTable的列数要根据时间基准序列的个数来定
	// 但行数可以确定,iSecurityid的个数+1(加权)
	resultTable.AddCol(Tx::Core::dtype_val_string);//券id--000001.sz
	resultTable.AddCol(Tx::Core::dtype_val_string);//券名称
#ifdef _DEBUG
	Tx::Core::TxDateTime starttime=Tx::Core::TxDateTime::getCurrentTime();
#endif
	//默认的返回值状态
	bool result = false;
	int iCol = 0;

	double dpro=0.0;
	Tx::Core::Table calcuTable;
	//	Tx::Core::ProgressWnd* pProgressWnd = Tx::Core::ProgressWnd::GetInstance();

	//取时间基准
	m_pShIndex->LoadHisTrade();
	m_pShIndex->LoadTradeDate();
	int iStandard=m_pShIndex->GetTradeDataCount();
	int iStandardStartIndex=m_pShIndex->GetTradeDateIndex(nStartDate);
	int iStandardEndIndex = m_pShIndex->GetTradeDateIndex(nEndDate);
	/////////////////////////////////////////////
	//防止越界  wangyc begin 20081202
	////////////////////////////////////////////
	//如果开始日期后越界或者截止日前越界
	//if ( 0 == iStandardEndIndex || -1 == iStandardStartIndex)
	//{
	//	AfxMessageBox(_T("对不起,您输入的日期内没有交易日,请重新输入!"));
	//	return false;
	//}
	//如果开始日期前越界，把开始日期置为上海行情的第一个交易日
	if (0 == iStandardStartIndex)
	{
		nStartDate = m_pShIndex->GetTradeDateOffset(nStartDate,0);
	}
	////如果截止日期后越界，把截止日期置为上海行情的最新交易日
	//if (-1 == iStandardEndIndex)
	//{
	//	nEndDate = m_pShIndex->GetTradeDateLatest();
	//}

	/////////////////////////////////////////////
	//防止越界  wangyc end  20081202
	////////////////////////////////////////////

	//if (iStandardStartIndex == -1) // 后越界, 前越界 返回值是0
	//	iStandardStartIndex = m_pShIndex->GetTradeDateIndex(m_pShIndex->GetTradeDateLatest());
	////int iiDATE=m_pShIndex->GetTradeDateByIndex(iStandardStartIndex);
	////if (iiDATE<(int)nStartDate)
	////	iStandardStartIndex++;
	//int iStandardEndIndex=m_pShIndex->GetTradeDateIndex(nEndDate);
	////if (-1 == iStandardStartIndex)
	////{
	////	int iFirstDate = m_pShIndex->GetTradeDateOffset(nStartDate,0);
	////	iStandardStartIndex = m_pShIndex->GetTradeDateIndex(iFirstDate);
	////	iStandardStartIndex++;
	////}
	//if (-1 == iStandardEndIndex)
	//{
	//	//int iEndDate = m_pShIndex->GetTradeDateOffset(nEndDate,0);
	//	//if (-1 == iEndDate)
	//		iStandardEndIndex = m_pShIndex->GetTradeDateIndex(m_pShIndex->GetTradeDateLatest());
	//	//else
	//	//	iStandardEndIndex = m_pShIndex->GetTradeDateIndex(iEndDate);
	//}
	//if (iStandardEndIndex < iStandardStartIndex)
	//{
	//	prw.SetPercent(pId,1.0);
	//	return false;
	//}

	//////////////////////////////////////////////////////////////////////////
	//在此处添加时间周期选择的代码 wangyc fixed
	//////////////////////////////////////////////////////////////////////////
	std::vector<int> iDateVec;
	iDateVec.clear();
	int iStartDate = nStartDate, iEndDate = nEndDate;
	if (1 == iSJZQ)	//周
	{
		iStartDate = Tx::Core::TxDate::CalculateFridayOfWeek(iStartDate);
		iEndDate = Tx::Core::TxDate::CalculateFridayOfWeek(iEndDate);
		//最后一天算出的周五不在日期的范围内，则舍去，日期截止日往前推一周
		if (iEndDate > (int)nEndDate)
		{
			iEndDate = Tx::Core::TxDate::CalculateDateOffsetDays(iEndDate,-7);
		}
		while (iStartDate <= iEndDate)
		{
			int iTempDate;
			if (m_pShIndex->IsTradeDate(iStartDate))
			{
				iDateVec.push_back(iStartDate);
			} 
			else
			{
				iTempDate = m_pShIndex->GetTradeDateOffset(iStartDate,0);
				//往前找前一个交易日，如果不在一周的范围内，说明之前的一周没有有效交易日，则此日期不计入Vect
				//防止因放假交易所停止交易超出一周的情况而造成的数据冗余
				if (iTempDate > Tx::Core::TxDate::CalculateDateOffsetDays(iStartDate,-7))
				{
					iDateVec.push_back(iTempDate);
				}
			}
			iStartDate = Tx::Core::TxDate::CalculateDateOffsetDays(iStartDate,7);
		}
	} 
	else if (2 == iSJZQ)	//月
	{
		int iTempDate = Tx::Core::TxDate::CalculateEndOfMonth(iStartDate);
		while (iTempDate <= iEndDate)
		{
			if (m_pShIndex->IsTradeDate(iTempDate))
			{
				iDateVec.push_back(iTempDate);
			} 
			else
			{
				//bug:12349      2012-08-07
				int iDateEx = m_pShIndex->GetTradeDateOffset(iTempDate,0);
				while (iDateEx == -1)
				{
					iTempDate = Tx::Core::TxDate::CalculateDateOffsetDays(iTempDate,-1);
					iDateEx = iTempDate;
				}
                //---------------------------------------------------------------------
				iDateVec.push_back(m_pShIndex->GetTradeDateOffset(iTempDate,0));
			}
			iTempDate = Tx::Core::TxDate::CalculateDateOffsetMonths(iTempDate,1);
		}
	}
	else if (3 == iSJZQ)	//年
	{
		int iTempDate = Tx::Core::TxDate::CalculateEndOfYear(iStartDate);
		while (iTempDate <= iEndDate)
		{
			if (m_pShIndex->IsTradeDate(iTempDate))
			{
				iDateVec.push_back(iTempDate);
			} 
			else
			{
				iDateVec.push_back(m_pShIndex->GetTradeDateOffset(iTempDate,0));
			}
			iTempDate = Tx::Core::TxDate::CalculateDateOffsetYears(iTempDate,1);
		}
	}
	else					//日
	{
		while (iStartDate <= iEndDate)
		{
			if (m_pShIndex->IsTradeDate(iStartDate))
			{
				iDateVec.push_back(iStartDate);
			}
			iStartDate = Tx::Core::TxDate::CalculateDateOffsetDays(iStartDate,1);
		}
		if (iDateVec.size() < 1)
		{
			if (-1 == m_pShIndex->GetTradeDateOffset(iEndDate,0))
			{
				if (iEndDate == m_pShIndex->GetCurDataDate())
				{
					iDateVec.push_back(iEndDate);
				}
			}
			else
			{
				iDateVec.push_back(m_pShIndex->GetTradeDateOffset(iEndDate,0));
			}
		}
	}
	if (iDateVec.size() < 1)
	{
		AfxMessageBox(_T("您所选的日期区间内没有符合要求的交易日,请重新输入!"));
		resultTable.Clear();
		iDateVec.clear();
		return false;
	}


	// 列数确定
	resultTable.AddRow();// 日期

	if(bPE && bPB)	//计算PE和PB
	{
		for (UINT i = 0;i<iDateVec.size();i++)
		{
			resultTable.AddCol(Tx::Core::dtype_double);
			resultTable.AddCol(Tx::Core::dtype_double);
			resultTable.SetCell(2+2*i,0,static_cast<double>(iDateVec[i]));
			resultTable.SetCell(3+2*i,0,static_cast<double>(iDateVec[i]));
		}
	}
	else if (bPE || bPB)	//只计算PE或者PB
	{
		for (UINT i = 0;i<iDateVec.size();i++)
		{
			resultTable.AddCol(Tx::Core::dtype_double);
			resultTable.SetCell(2+i,0,static_cast<double>(iDateVec[i]));
		}
	} 
	else
	{
		AfxMessageBox(_T("请选择PE,PB至少一个计算指标"));
		resultTable.Clear();
		iDateVec.clear();
		return false;
	}


	//2007-11-27 cenxw add 支持双击转入行情界面
	resultTable.AddCol(Tx::Core::dtype_int4);

	std::vector<double> fzpe_weight(iDateVec.size()),fzpb_weight(iDateVec.size()),fmpe_weight(iDateVec.size()),fmpb_weight(iDateVec.size());// 分子分母加权值
	UINT iRow=1;

	UINT nArray[5];
	nArray[0]=0;
	nArray[1]=1;
	nArray[2]=2;
	nArray[3]=3;
	nArray[4]=4;

	// modified by zhoup 2007.12.24
	// tugl说归属母公司所有者的净利润可能为0,要用净利润-少数股东损益
	// 30900088 - 30900090
	// 后来又改回归属母公司所有者的净利润
	////Tx::Core::Table_Indicator AlllrTable;
	////AlllrTable.AddParameterColumn(Tx::Core::dtype_int8);
	////AlllrTable.AddIndicatorColumn(30900089,Tx::Core::dtype_decimal,nArray,1);

	////CString strTable;
	////result = m_pLogicalBusiness->GetData(AlllrTable,true,pid_posinfo_map);
	//#ifdef _DEBUG
	//	CString strTable = AlllrTable.TableToString();
	//	Tx::Core::Commonality::String().StringToClipboard(strTable);
	//#endif
	////if(result==false)
	////	return false;

	////Tx::Core::Table_Indicator AllqyTable;
	////AllqyTable.AddParameterColumn(Tx::Core::dtype_int8);
	////AllqyTable.AddIndicatorColumn(30900067,Tx::Core::dtype_decimal,nArray,1);

	////if (bPB)
	////{
	////	result = m_pLogicalBusiness->GetData(AllqyTable,true,pid_posinfo_map);
	////	if(result==false)
	////		return false;
	////}

	// 美元汇率---沪市
	DataFileNormal<blk_TxExFile_FileHead,ExRate>* pUSDRatio = new DataFileNormal<blk_TxExFile_FileHead,ExRate>;
	if(pUSDRatio->Load(
		30253,
		30007,
		true)==false || pUSDRatio->GetDataCount()<1)
	{
		delete pUSDRatio;
		return false;
	}
	// 港币汇率---深市
	DataFileNormal<blk_TxExFile_FileHead,ExRate>* pHKDRatio = new DataFileNormal<blk_TxExFile_FileHead,ExRate>;
	if(pHKDRatio->Load(
		30254,
		30007,
		true)==false || pHKDRatio->GetDataCount()<1)
	{
		delete pUSDRatio;
		delete pHKDRatio;
		return false;
	}

	//显示市盈率计算的进度条
	Tx::Core::ProgressWnd prw;
	UINT pId=prw.AddItem(1,_T("市盈率计算过程中……"));
	prw.Show(1000);

	//从交易实体ID转为公司ID
	std::vector<int> iCompanyId;
	for (std::set<int>::iterator iter = iSecurityId.begin();iter != iSecurityId.end();iter++)
	{
		//把不同的交易实体ID转化为相同的机构ID的那些样本去掉
		GetSecurityNow(*iter);
		if(m_pSecurity!=NULL)
			iCompanyId.push_back((int)m_pSecurity->GetInstitutionId());
	}

	//建立存储利润和权益的结构
	//选择单个报告期的数据
	std::vector<double>  dProfitVector;									//选择单个报告期的利润
	std::vector<double>  dRightVector;									//选择单个报告期的权益
	std::unordered_map<int,int> iIndexToReportData;								//存储报告期和索引的对应
	Tx::Core::Table profitTable;										//存储利润的数据
	Tx::Core::Table rightTable;											//存储权益的数据
	////选择单个报告期
	//if (1==iCWSJ>>3)
	//{
	//	int iYear = nFiscalyear;
	//	int iFiscal = nFiscalyearquater%10;
	//	//if( 9 != iFiscal && 1 == iJSFF)
	//	//{
	//	//	AfxMessageBox(_T("计算静态市盈率，请选择有效的年报数据"));
	//	//}
	//	int iQuarter = 0;
	//	switch(iFiscal)
	//	{
	//	case 1:
	//		iQuarter =1;
	//		break;
	//	case 3:
	//		iQuarter =2;
	//		break;
	//	case 5:
	//		iQuarter =3;
	//	    break;
	//	case 9:
	//		iQuarter =4;
	//	    break;
	//	default:
	//	    break;
	//	}
	//	//取得权益
	//	if (bPB)
	//	{
	//		if (!GetDataOfProfitAndRight(iYear,iQuarter,false,iCompanyId,&dRightVector))
	//		{
	//			AfxMessageBox(_T("未取到所选报告期权益数据"));
	//			return false;
	//		}
	//	}
	//	//取得利润
	//	if (!GetDataOfProfitAndRight(iYear,iQuarter,true,iCompanyId,&dProfitVector))
	//	{
	//		AfxMessageBox(_T("未取到所选报告期利润数据"));
	//		return false;
	//	}

	//} 
	//选择多个报告期
	//else
	{
		int bgnYear = 0;
		int endYear = 0;
		if (1==iCWSJ>>3)
		{
			bgnYear = nFiscalyear-2;
			endYear = nFiscalyear;
		}
		else
		{
			bgnYear = nStartDate/10000-2;
			endYear = nEndDate/10000;
		}

		if(bPB)
		{
			for (std::vector<int>::iterator iter = iCompanyId.begin();iter != iCompanyId.end();iter++)
			{
				rightTable.AddCol(Tx::Core::dtype_double);
			}
		}
		if (bPE)
		{
			for (std::vector<int>::iterator iter = iCompanyId.begin();iter != iCompanyId.end();iter++)
			{
				profitTable.AddCol(Tx::Core::dtype_double);
			}
		}


		//if( 1==iCWSJ>>3 )
		//{
		//	int i=0;
		//	for (int year=bgnYear;year <= endYear;year++)
		//	{
		//		iIndexToReportData.insert(make_pair(year*10000+1,i++));
		//		iIndexToReportData.insert(make_pair(year*10000+2,i++));
		//		iIndexToReportData.insert(make_pair(year*10000+3,i++));
		//		iIndexToReportData.insert(make_pair(year*10000+4,i++));
		//	}
		//}
		//else if (4!=(iCWSJ&4)&&(2!=(iCWSJ&2))&&(1==(iCWSJ&1))) //0001--季报
		//{
		//	// 季报只有1 3 季报,1月31日之前取上一年的三季报
		//	int i=0;
		//	for (int year=bgnYear;year <= endYear;year++)
		//	{
		//		iIndexToReportData.insert(make_pair(year*10000+1,i++));
		//		iIndexToReportData.insert(make_pair(year*10000+3,i++));
		//		iIndexToReportData.insert(make_pair(year*10000+4,i++));
		//	}
		//}
		//else if (4!=(iCWSJ&4)&&(2==(iCWSJ&2))&&(1!=(iCWSJ&1))) //0010--中报
		//{	
		//	int i=0;
		//	for (int year=bgnYear;year <= endYear;year++)
		//	{
		//		iIndexToReportData.insert(make_pair(year*10000+2,i++));
		//		iIndexToReportData.insert(make_pair(year*10000+4,i++));
		//	}
		//}
		//else if (4==(iCWSJ&4)&&(2!=(iCWSJ&2))&&(1!=(iCWSJ&1))) //0100--年报
		//{
		//	int i=0;
		//	for (int year=bgnYear;year <= endYear;year++)
		//	{
		//		iIndexToReportData.insert(make_pair(year*10000+4,i++));
		//	}
		//}
		//else if (4!=(iCWSJ&4)&&(2==(iCWSJ&2))&&(1==(iCWSJ&1))) //0011--季报,中报 
		//{
		//	int i=0;
		//	for (int year=bgnYear;year <= endYear;year++)
		//	{
		//		iIndexToReportData.insert(make_pair(year*10000+1,i++));
		//		iIndexToReportData.insert(make_pair(year*10000+2,i++));
		//		iIndexToReportData.insert(make_pair(year*10000+3,i++));
		//		iIndexToReportData.insert(make_pair(year*10000+4,i++));
		//	}
		//}
		//else if (4==(iCWSJ&4)&&(2!=(iCWSJ&2))&&(1==(iCWSJ&1))) //0101--年报,季报
		//{
		//	int i=0;
		//	for (int year=bgnYear;year <= endYear;year++)
		//	{
		//		iIndexToReportData.insert(make_pair(year*10000+1,i++));
		//		iIndexToReportData.insert(make_pair(year*10000+3,i++));
		//		iIndexToReportData.insert(make_pair(year*10000+4,i++));
		//	}
		//}
		//else if (4==(iCWSJ&4)&&(2==(iCWSJ&2))&&(1!=(iCWSJ&1))) //0110--年报,中报
		//{
		//	int i=0;
		//	for (int year=bgnYear;year <= endYear;year++)
		//	{
		//		iIndexToReportData.insert(make_pair(year*10000+2,i++));
		//		iIndexToReportData.insert(make_pair(year*10000+4,i++));
		//	}
		//}
		//else if (4==(iCWSJ&4)&&(2==(iCWSJ&2))&&(1==(iCWSJ&1))) //0111--年报,中报,季报
		//{
		//	int i=0;
		//	for (int year=bgnYear;year <= endYear;year++)
		//	{
		//		iIndexToReportData.insert(make_pair(year*10000+1,i++));
		//		iIndexToReportData.insert(make_pair(year*10000+2,i++));
		//		iIndexToReportData.insert(make_pair(year*10000+3,i++));
		//		iIndexToReportData.insert(make_pair(year*10000+4,i++));
		//	}
		//}
		//为了计算市净率，则每年四个报告期的数据都取
		int i=0;
		for (int year=bgnYear;year <= endYear;year++)
		{
			iIndexToReportData.insert(make_pair(year*10000+1,i++));
			iIndexToReportData.insert(make_pair(year*10000+2,i++));
			iIndexToReportData.insert(make_pair(year*10000+3,i++));
			iIndexToReportData.insert(make_pair(year*10000+4,i++));
		}

		//取利润权益的进度条
		Tx::Core::ProgressWnd prw1;
		UINT pId1=prw1.AddItem(1,_T("正在从服务器取利润和权益数据……"));
		prw1.Show(1000);

		int bgntime = ::GetTickCount();
		//取权益和利润的数据  填到表的对应项中
		int iProgressBar = 0;
		for (std::unordered_map<int,int>::iterator iter=iIndexToReportData.begin();iter!= iIndexToReportData.end();iter++,iProgressBar++)
		{
			prw1.SetPercent(pId1,(float)iProgressBar/iIndexToReportData.size());

			std::vector<double> dProfitVector;
			std::vector<double> dRightVector;
			int iyear = iter->first/10000;
			int iquarter =iter->first%10000;
			if (bPB)
			{
				if (GetDataOfProfitAndRight(iyear,iquarter,false,iCompanyId,&dRightVector))
				{
					rightTable.AddRow();
					int i =rightTable.GetRowCount();
					for (int j=0;j<(int)iSecurityId.size();j++)
					{
						rightTable.SetCell(j,i-1,dRightVector[j]);
					}
				}
				else
				{
					rightTable.AddRow();
					iter->second = -1;
				}
			}
			if(bPE)
			{
				if (GetDataOfProfitAndRight(iyear,iquarter,true,iCompanyId,&dProfitVector))
				{
					profitTable.AddRow();
					int i =profitTable.GetRowCount();
					for (int j=0;j<(int)iSecurityId.size();j++)
					{
						profitTable.SetCell(j,i-1,dProfitVector[j]);
					}
				}
				else
				{
					profitTable.AddRow();
					iter->second = -1;
				}
			}

		}
		int endtime = ::GetTickCount();
		int usedtime = endtime-bgntime;
		TRACE(_T("取数据花费时间：%ds"),usedtime);
	}
	CString strProfit = profitTable.TableToString();
	CString strRight = rightTable.TableToString();

	ExRate* pRatio = NULL;
	int iIndexOfColumn = 0;		//列的索引
	for (std::set<int>::iterator iter=iSecurityId.begin();iter!=iSecurityId.end();iter++,iIndexOfColumn++)
	{// for begin
		//starttime=Tx::Core::TxDateTime::getCurrentTime();
		GetSecurityNow(*iter);
		if (NULL == m_pSecurity )
			continue;
		if (m_pSecurity->IsStockA() || m_pSecurity->IsStockB())
		{
			int qid=m_pSecurity->GetInstitutionId();
			calcuTable.Clear();

			bool bSuc=m_pSecurity->LoadHisTrade();
			bSuc=m_pSecurity->LoadTradeDate();// 取得交易日期

			calcuTable.AddCol(Tx::Core::dtype_int4,true);// 券id---测试只有一个20000002													0
			calcuTable.AddCol(Tx::Core::dtype_int4);	// 时间序列---从20070401到20070501的每一个交易日								1
			calcuTable.AddCol(Tx::Core::dtype_double);	// 股本---对应时间序列的每个值有一个()-----对应股本("总股本")					2
			calcuTable.AddCol(Tx::Core::dtype_double);	// 流通股本---对应时间序列的每个值有一个()										3
			calcuTable.AddCol(Tx::Core::dtype_double);	// 总股本----公司股本(加权时用)													4
			calcuTable.AddCol(Tx::Core::dtype_float);	// 前收价--对应时间序列的每个值有一个(行情)	
			calcuTable.AddCol(Tx::Core::dtype_double);	// 净利润--对应时间序列的每个值有一个(取最近的报告期的数据) * 对应股本 / 总股本	6
			calcuTable.AddCol(Tx::Core::dtype_double);	// PE值--- 股本*前收/利润														7
			if (bPB)
			{
				calcuTable.AddCol(Tx::Core::dtype_double);	// 净资产																	8
				calcuTable.AddCol(Tx::Core::dtype_double);	// PB																		9
			}
			// 只要第1,2,6列

			// 按时间序列有序
			int j=0;
			for (UINT i=0;i<iDateVec.size();i++)
			{
				//int date = iDateVec[i];
				//从表中取对应利润和权益
				double dProfit = Tx::Core::Con_doubleInvalid;
				double dRight = Tx::Core::Con_doubleInvalid;
				//取基准的报告期，如果是指定报告期,基准报告期为指定报告期
				//如果为选择多个报告期，则取日期相应的最新的报告期为基准报告期
				//int reportData = Tx::Core::Con_intInvalid;				//基准报告期
				std::unordered_map<int,int>::iterator latestReport;				//基准报告期对应记录
				int iyear = 0;
				int iquarter = 0;
				//根据不同的市盈率类型和所选报告期的不同折算不同的利润数据 fix by wangyc 20091111
				if(bPE)
				{
					if ( 4 == iJSFF)	//计算静态适应率
					{
						iyear = iDateVec[i]/10000 - 1;
						iquarter = 4;
						int reportData = iyear*10000+iquarter;
						latestReport = iIndexToReportData.find(reportData);
						//if (latestReport == iIndexToReportData.end() || -1 == latestReport->second)
						//{
						//	AfxMessageBox(_T("未取到所选报告期数据"));
						//	return false;
						//}
					}  
					else
					{
						if (1==iCWSJ>>3)
						{
							//如果是指定报告期,基准报告期为指定报告期
							iyear = nFiscalyear;
							int iFiscal = nFiscalyearquater%10;
							//if( 9 != iFiscal && 1 == iJSFF)
							//{
							//	AfxMessageBox(_T("计算静态市盈率，请选择有效的年报数据"));
							//}
							switch(iFiscal)
							{
							case 1:
								iquarter =1;
								break;
							case 3:
								iquarter =2;
								break;
							case 5:
								iquarter =3;
								break;
							case 9:
								iquarter =4;
								break;
							default:
								break;
							}
							int reportData = iyear*10000+iquarter;
							//std::unordered_map<int,int>::iterator latestReport;
							latestReport = iIndexToReportData.find(reportData);
							if (latestReport == iIndexToReportData.end() || -1 == latestReport->second)
							{
								AfxMessageBox(_T("未取到所选报告期数据"));
								return false;
							}
						}
						else
						{
							//如果为选择多个报告期，则取日期相应的最新的报告期为基准报告期
							iyear = iDateVec[i]/10000;
							int imon = iDateVec[i]%10000;
							iquarter = 0;
							if (4!=(iCWSJ&4)&&(2!=(iCWSJ&2))&&(1==(iCWSJ&1))) //0001--季报
							{
								// 季报只有1 3 季报,1月31日之前取上一年的三季报
								if(imon<=331)
								{
									iquarter = 3;
									iyear-=1;
								}
								else if (imon<=930)
									iquarter = 1;
								else
									iquarter = 3;
							}
							else if (4!=(iCWSJ&4)&&(2==(iCWSJ&2))&&(1!=(iCWSJ&1))) //0010--中报
							{
								iquarter = 2;
								if (imon<=630)
									iyear-=1;			
							}
							else if (4==(iCWSJ&4)&&(2!=(iCWSJ&2))&&(1!=(iCWSJ&1))) //0100--年报
							{
								iquarter = 4;
								if (imon <= 1231)
									iyear -= 1;
							}
							else if (4!=(iCWSJ&4)&&(2==(iCWSJ&2))&&(1==(iCWSJ&1))) //0011--季报,中报 
							{
								if (imon<=331)
								{
									iquarter = 3;
									iyear -= 1;
								}
								else if (imon<=630)
									iquarter = 1;
								else if (imon<=930)
									iquarter = 2;
								else
									iquarter = 3;
							}
							else if (4==(iCWSJ&4)&&(2!=(iCWSJ&2))&&(1==(iCWSJ&1))) //0101--年报,季报
							{
								if (imon<=331)
								{
									iquarter = 4;
									iyear -= 1;
								}
								else if (i<=930)
									iquarter = 1;
								else if (i<=1231)
									iquarter = 3;
								else
									iquarter = 4;
							}
							else if (4==(iCWSJ&4)&&(2==(iCWSJ&2))&&(1!=(iCWSJ&1))) //0110--年报,中报
							{
								if (imon<=630)
								{
									iquarter = 4;
									iyear -= 1;
								}
								else if (imon<=1231)
									iquarter = 2;
								else
									iquarter = 4;
							}
							else if (4==(iCWSJ&4)&&(2==(iCWSJ&2))&&(1==(iCWSJ&1))) //0111--年报,中报,季报
							{
								if (imon<=331)
								{
									iquarter = 4;
									iyear -= 1;
								}
								else if (imon<=630)
									iquarter = 1;
								else if (imon<=930)
									iquarter = 2;
								else if (imon<=1231)
									iquarter = 3;
								else
									iquarter = 4;
							}

							int reportData = iyear*10000+iquarter;
							//std::unordered_map<int,int>::iterator latestReport;
							latestReport = iIndexToReportData.find(reportData);
							double dRight_temp = Tx::Core::Con_doubleInvalid;
							double dProfit_temp = Tx::Core::Con_doubleInvalid;
							if (latestReport != iIndexToReportData.end())
							{
								if (latestReport->second >= 0)
								{
									//////if (bPB)
									//////{
									//////	rightTable.GetCell(iIndexOfColumn,latestReport->second,dRight_temp);
									//////}
									if (bPE)
									{
										profitTable.GetCell(iIndexOfColumn,latestReport->second,dProfit_temp);

									}
								}
							}

							//该报告期的数据没有取到，取之前的报告期
							//或者该报告期取到数据，但是该报告期的该样本数据无效（即公布报告期有早有晚的情况）
							while(latestReport != iIndexToReportData.end() && (latestReport->second == -1 || (fabs(dRight_temp-0.0) > 1e14 && fabs(dProfit_temp-0.0)>1e14)))
							{
								if (4!=(iCWSJ&4)&&(2!=(iCWSJ&2))&&(1==(iCWSJ&1))) //0001--季报
								{
									// 季报只有1 3 季报,1月31日之前取上一年的三季报
									if(3 == iquarter)
									{
										iquarter = 1;
									}
									else if (1 == iquarter)
									{
										iyear--;
										iquarter = 3;
									}
								}
								else if (4!=(iCWSJ&4)&&(2==(iCWSJ&2))&&(1!=(iCWSJ&1))) //0010--中报
								{
									iyear--;			
								}
								else if (4==(iCWSJ&4)&&(2!=(iCWSJ&2))&&(1!=(iCWSJ&1))) //0100--年报
								{
									iyear--;
								}
								else if (4!=(iCWSJ&4)&&(2==(iCWSJ&2))&&(1==(iCWSJ&1))) //0011--季报,中报 
								{
									if (3 == iquarter)
									{
										iquarter = 2;

									} 
									else if (2 == iquarter)
									{
										iquarter =1;
									}
									else if(1 == iquarter)
									{
										iyear--;
										iquarter = 3;
									}
								}
								else if (4==(iCWSJ&4)&&(2!=(iCWSJ&2))&&(1==(iCWSJ&1))) //0101--年报,季报
								{
									if (4 == iquarter)
									{
										iquarter = 3;
									} 
									else if (3 == iquarter)
									{
										iquarter =1;
									}
									else if(1 == iquarter)
									{
										iyear--;
										iquarter = 4;
									}
								}
								else if (4==(iCWSJ&4)&&(2==(iCWSJ&2))&&(1!=(iCWSJ&1))) //0110--年报,中报
								{
									if (4 == iquarter)
									{
										iquarter = 2;
									} 
									else if(2 == iquarter)
									{
										iyear--;
										iquarter = 4;
									}
								}
								else if (4==(iCWSJ&4)&&(2==(iCWSJ&2))&&(1==(iCWSJ&1))) //0111--年报,中报,季报
								{
									if (4 == iquarter)
									{
										iquarter = 3;
									} 
									else if (3 == iquarter)
									{
										iquarter = 2;

									} 
									else if (2 == iquarter)
									{
										iquarter =1;
									}
									else if(1 == iquarter)
									{
										iyear--;
										iquarter = 4;
									};
								}

								int reportData = iyear*10000+iquarter;
								//std::unordered_map<int,int>::iterator latestReport;
								// find 以后 一定要判断返回值的!!!
								latestReport = iIndexToReportData.find(reportData);
								//if (latestReport == iIndexToReportData.end())
								//{
								//	TRACE(_T(""));
								//	AfxMessageBox(_T("未取到利润和权益数据"));
								//	return false;
								//}
								if (latestReport != iIndexToReportData.end())
								{
									if (latestReport->second >= 0)
									{
										//////if (bPB)
										//////{
										//////	rightTable.GetCell(iIndexOfColumn,latestReport->second,dRight_temp);
										//////}
										if (bPE)
										{
											profitTable.GetCell(iIndexOfColumn,latestReport->second,dProfit_temp);

										}
									}
								}
							}
							//if (latestReport == iIndexToReportData.end())
							//{
							//	//AfxMessageBox(_T("未取到利润和权益数据"));
							//	//return false;
							//}

						}
					}
				}


				//取需要的利润 开始
				if(bPE)
				{
					if (latestReport != iIndexToReportData.end())
					{
						if ( 4 == iJSFF)	//静态市盈率
						{
							if (latestReport != iIndexToReportData.end() && -1 != latestReport->second)
							{
								//if (bPB)
								//{
								//	rightTable.GetCell(iIndexOfColumn,latestReport->second,dRight);
								//}
								if (bPE)
								{
									profitTable.GetCell(iIndexOfColumn,latestReport->second,dProfit);
								}
							}
							else
							{
								dProfit = Tx::Core::Con_doubleInvalid;
								//dRight = Tx::Core::Con_doubleInvalid;
							}
						}
						else if (4 == latestReport->first%10 && -1 != latestReport->second)
						{
							//如果最新报告期为年报，则直接取年报数据的利润和权益
							//if (bPB)
							//{
							//	rightTable.GetCell(iIndexOfColumn,latestReport->second,dRight);
							//}
							if (bPE)
							{
								profitTable.GetCell(iIndexOfColumn,latestReport->second,dProfit);
							}

						} 
						else
						{
							//如果最新报告期不是年报

							//是否简单市盈率
							if(1 == iJSFF)			
							{
								//存储需要报告期的在利润表中的列的对应
								int iIndexOfReport1 = Tx::Core::Con_intInvalid;
								iIndexOfReport1 = latestReport->second;
								//if (bPB)
								//{
								//	rightTable.GetCell(iIndexOfColumn,iIndexOfReport1,dRight);
								//}
								if (bPE)
								{
									profitTable.GetCell(iIndexOfColumn,iIndexOfReport1,dProfit);
								}
								switch(latestReport->first%10)
								{
								case 1:
									dProfit*=4;
									break;
								case 2:
									dProfit*=2;
									break;
								case 3:
									dProfit= dProfit*4/3;
									break;
								case 4:
									break;
								default:
									break;
								}
							}
							else
							{
								//存储需要的三个报告期的在利润表中的列的对应
								int iIndexOfReport1 = Tx::Core::Con_intInvalid;
								int iIndexOfReport2 = Tx::Core::Con_intInvalid;
								int iIndexOfReport3 = Tx::Core::Con_intInvalid;
								iIndexOfReport1 = latestReport->second;
								int iSameReportLast = (iyear-1)*10000 + iquarter;
								latestReport = iIndexToReportData.find(iSameReportLast);
								if (latestReport != iIndexToReportData.end())
								{
									iIndexOfReport2 = latestReport->second;
								}
								else
								{
									iIndexOfReport2 = -1;
								}

								int iReportLastYear = (iyear-1)*10000 +4;
								latestReport = iIndexToReportData.find(iReportLastYear);
								if (latestReport != iIndexToReportData.end())
								{
									iIndexOfReport3 = latestReport->second;
								}
								else
								{
									iIndexOfReport3 = -1;
								}
								//if (bPB)
								//{
								//	rightTable.GetCell(iIndexOfColumn,iIndexOfReport1,dRight);
								//}
								if (bPE)
								{
									if(2 == iJSFF)		//滚动
									{
										//如果上年同期和上年年报有效							
										if(-1 != iIndexOfReport2 && -1 != iIndexOfReport3 )
										{
											double profit1 = Tx::Core::Con_doubleInvalid;
											double profit2 = Tx::Core::Con_doubleInvalid;
											double profit3 = Tx::Core::Con_doubleInvalid;
											profitTable.GetCell(iIndexOfColumn,iIndexOfReport1,profit1);
											profitTable.GetCell(iIndexOfColumn,iIndexOfReport2,profit2);
											profitTable.GetCell(iIndexOfColumn,iIndexOfReport3,profit3);
											if(fabs(profit1-0.0)<1e14 && fabs(profit2-0.0) < 1e14 && fabs(profit3-0.0) < 1e14)
											{
												dProfit = profit1 - profit2 + profit3;
											}
											else if(fabs(profit3-0.0) < 1e14)
											{
												dProfit = profit3;
											}
											else
											{
												dProfit =  Tx::Core::Con_doubleInvalid;
											}

										}
										else if(-1 != iIndexOfReport3 )
											//如果没有上年同期，直接使用上个年报
										{
											double profit = Tx::Core::Con_doubleInvalid;
											profitTable.GetCell(iIndexOfColumn,iIndexOfReport3,profit);
											if(fabs(profit-0.0) < 1e14)
											{
												profitTable.GetCell(iIndexOfColumn,iIndexOfReport3,dProfit);
											}
											else
											{
												dProfit =  Tx::Core::Con_doubleInvalid;
											}
										}
										else
										{
											//AfxMessageBox(_T("未取到所需利润数据，请联系我们！"));
											//return false;
											//未取到上年年报，无法计算滚动，置无效值
											//dRight = Tx::Core::Con_doubleInvalid;
											dProfit = Tx::Core::Con_doubleInvalid;
										}

									}
									else					//同比
									{
										if(-1 != iIndexOfReport2 && -1 != iIndexOfReport3)
										{
											double profit1 = Tx::Core::Con_doubleInvalid;
											double profit2 = Tx::Core::Con_doubleInvalid;
											double profit3 = Tx::Core::Con_doubleInvalid;
											profitTable.GetCell(iIndexOfColumn,iIndexOfReport1,profit1);
											profitTable.GetCell(iIndexOfColumn,iIndexOfReport2,profit2);
											profitTable.GetCell(iIndexOfColumn,iIndexOfReport3,profit3);
											if (profit1 > 0.0 && profit2 > 0.0 && profit3 > 0.0)
											{
												dProfit = profit1/profit2*profit3;
											} 
											else if (profit1 > 0.0)
											{
												//上年同期或者年报有一个为负值，按简单计算
												latestReport = iIndexToReportData.begin();
												for(int i = 0; i <iIndexOfReport1;i++)
												{
													latestReport++;
												}
												if (latestReport != iIndexToReportData.end())
												{
													switch(latestReport->first%10)
													{
													case 1:
														dProfit=4*profit1;
														break;
													case 2:
														dProfit=2*profit1;
														break;
													case 3:
														dProfit=4*profit1/3;
														break;
													case 4:
														break;
													default:
														break;
													}
												} // end for if (latestReport != iIndexToReportData.end())

											} //end for  else if (profit1 > 0.0)

										} // end for if(-1 != iIndexOfReport2 && -1 != iIndexOfReport3)

									} //end for if(2 == iJSFF)		//滚动

								} //end for if (bPE)

							} //end for //是否简单市盈率
							// if(1 == iJSFF)	


						} // end for 取需要的利润和权益 开始
						//取需要的利润和权益 结束

					}
				}//取需要的利润 结束


				//取所有者权益数据 fix by wangyc 20091106 begin
				//不管选择什么参数，始终选择最新报告期的所有者权益数据
				if (bPB)
				{
					dRight = Tx::Core::Con_doubleInvalid;

					iyear = iDateVec[i]/10000;
					int imon = iDateVec[i]%10000;
					if (imon<331)
					{
						iquarter = 4;
						iyear -= 1;
					}
					else if (imon<630)
						iquarter = 1;
					else if (imon<930)
						iquarter = 2;
					else if (imon<1231)
						iquarter = 3;
					else
						iquarter = 4;
					int iReportData = iyear*10000 + iquarter;
					std::unordered_map<int,int>::iterator RightReportDate;		//权益数据的报告期数据
					RightReportDate = iIndexToReportData.find(iReportData);
					while(RightReportDate != iIndexToReportData.end())
					{
						rightTable.GetCell(iIndexOfColumn,RightReportDate->second,dRight);
						if (fabs(dRight-0.0)>1e14)
						{
							RightReportDate--;
							continue;
						}
						break;

					}
					if (fabs(dRight-0.0)>1e14)
						dRight = Tx::Core::Con_doubleInvalid;
				}
				//取所有者权益数据 fix by wangyc 20091106 end






				////}

				//int iTradeDate = m_pShIndex->GetTradeDateByIndex(indexvec[i]);
				// 			// 此券可能在这个日期是停牌的,取之前的收盘价
				//判断是否当前日期，如果是当前日期，直接取价格股本，如果不是当前日期，取历史信息
				int iCurDate = m_pSecurity->GetCurDataDate();
				if ( iCurDate == iDateVec[i] && m_pSecurity->IsTrading())
				{
					//if ( m_pSecurity->GetVolume() <0.1)
					//	continue;
					calcuTable.AddRow();
					calcuTable.SetCell(0,j,qid);
					calcuTable.SetCell(1,j,iDateVec[i]);

					TxShareData *pShareData = m_pSecurity->GetTxShareDataByDate(iDateVec[i]);
					if (pShareData == NULL)
					{
						calcuTable.SetCell(2,j,Tx::Core::Con_doubleInvalid);
						calcuTable.SetCell(3,j,Tx::Core::Con_doubleInvalid);
						calcuTable.SetCell(4,j,Tx::Core::Con_doubleInvalid);
					}
					else
					{
						calcuTable.SetCell(2,j,pShareData->TheShare);
						calcuTable.SetCell(3,j,pShareData->TradeableShare);
						calcuTable.SetCell(4,j,pShareData->TotalShare);
					}

					if (bClosePrice)
					{
						if (m_pSecurity->IsStockB())
						{
							if (m_pSecurity->IsShanghai())
								pRatio = pUSDRatio->GetDataByObj(iDateVec[i]);
							else
								pRatio = pHKDRatio->GetDataByObj(iDateVec[i]);

							if (pRatio != NULL && m_pSecurity->GetClosePrice() > 0.0)
								calcuTable.SetCell(5,j,static_cast<float>(m_pSecurity->GetClosePrice() * pRatio->iRatio));//收盘价
							else
								calcuTable.SetCell(5,j,Tx::Core::Con_floatInvalid);//收盘价
						}
						else if (m_pSecurity->IsStockA() && m_pSecurity->GetClosePrice() > 0.0)
							calcuTable.SetCell(5,j,m_pSecurity->GetClosePrice());//收盘价
						else	// 非A,B股
							calcuTable.SetCell(5,j,Tx::Core::Con_floatInvalid);//收盘价
					}
					else
					{
						if (m_pSecurity->IsStockB())
						{
							if (m_pSecurity->IsShanghai())
								pRatio = pUSDRatio->GetDataByObj(iDateVec[i]);
							else
								pRatio = pHKDRatio->GetDataByObj(iDateVec[i]);

							if (pRatio != NULL && m_pSecurity->GetAmount() > 0.0 && m_pSecurity->GetVolume() > 0.0)
								calcuTable.SetCell(5,j,static_cast<float>(m_pSecurity->GetAmount()/m_pSecurity->GetVolume() * pRatio->iRatio));//收盘价
							else
								calcuTable.SetCell(5,j,Tx::Core::Con_floatInvalid);//均价
						}
						else if (m_pSecurity->IsStockA() && m_pSecurity->GetAmount() > 0.0 && m_pSecurity->GetVolume() > 0.0)
							calcuTable.SetCell(5,j,static_cast<float>(m_pSecurity->GetAmount()/m_pSecurity->GetVolume()));//均价
						else	// 非A,B股
							calcuTable.SetCell(5,j,Tx::Core::Con_floatInvalid);//收盘价
						//calcuTable.SetCell(5,j,static_cast<float>(pHisTradeData->Amount/pHisTradeData->Volume));//均价--当天的成交金额/成交量
					}
				} 
				else
				{
					int iIndex = m_pSecurity->GetTradeDateIndex(iDateVec[i]);
					if (-1 == iIndex)
					{
						int iValidDate = m_pSecurity->GetTradeDateOffset(iDateVec[i],0);
						if (-1 == iValidDate)
							iValidDate=m_pSecurity->GetTradeDateLatest();
						iIndex = m_pSecurity->GetTradeDateIndex(iValidDate);
						//////////////////////////////////////////////////////////
						// 处理发行未成功的样本，显示-- 20090519 add by wangyc
						if (iIndex < 0 )
						{
							calcuTable.AddRow();
							j++;
							continue;
						}
						///////////////////////////////////////////////////////
						//if (-1==iIndex)//日期下限仍为停牌时间
						//	iIndex = m_pSecurity->GetTradeDateIndex(iValidDate);

					}

					HisTradeData* pHisTradeData=m_pSecurity->GetTradeDataByIndex(iIndex);
					if (NULL == pHisTradeData)
						continue;

					calcuTable.AddRow();

					//索引为0，且日期不是股票上市当天的日期，表示该股票未上市
					if ( iIndex == 0 && pHisTradeData->Date != iDateVec[i])
					{
						j++;
						continue;
					}
						

					calcuTable.SetCell(0,j,qid);
					calcuTable.SetCell(1,j,iDateVec[i]);
					//calcuTable.SetCell(2,j,gb);//股本单独填
					// 不再单独填,可以一起搞定
					TxShareData *pShareData = m_pSecurity->GetTxShareDataByDate(iDateVec[i]);
					if (pShareData == NULL)
					{
						calcuTable.SetCell(2,j,Tx::Core::Con_doubleInvalid);
						calcuTable.SetCell(3,j,Tx::Core::Con_doubleInvalid);
						calcuTable.SetCell(4,j,Tx::Core::Con_doubleInvalid);
					}
					else
					{
						calcuTable.SetCell(2,j,pShareData->TheShare);
						calcuTable.SetCell(3,j,pShareData->TradeableShare);
						calcuTable.SetCell(4,j,pShareData->TotalShare);
					}
					if (bClosePrice)
					{
						if (m_pSecurity->IsStockB())
						{
							if (m_pSecurity->IsShanghai())
								pRatio = pUSDRatio->GetDataByObj(iDateVec[i]);
							else
								pRatio = pHKDRatio->GetDataByObj(iDateVec[i]);

							if (pRatio != NULL && pHisTradeData->Close > 0.0)
								calcuTable.SetCell(5,j,static_cast<float>(pHisTradeData->Close * pRatio->iRatio));//收盘价
							else
								calcuTable.SetCell(5,j,Tx::Core::Con_floatInvalid);//收盘价
						}
						else if (m_pSecurity->IsStockA() && pHisTradeData->Close > 0.0)
							calcuTable.SetCell(5,j,pHisTradeData->Close);//收盘价
						else	// 非A,B股
							calcuTable.SetCell(5,j,Tx::Core::Con_floatInvalid);//收盘价
					}
					else
					{
						if (m_pSecurity->IsStockB())
						{
							if (m_pSecurity->IsShanghai())
								pRatio = pUSDRatio->GetDataByObj(iDateVec[i]);
							else
								pRatio = pHKDRatio->GetDataByObj(iDateVec[i]);

							if (pRatio != NULL && pHisTradeData->Amount > 0.0 && pHisTradeData->Volume > 0.0)
								calcuTable.SetCell(5,j,static_cast<float>(pHisTradeData->Amount/pHisTradeData->Volume * pRatio->iRatio));//收盘价
							else
								calcuTable.SetCell(5,j,Tx::Core::Con_floatInvalid);//收盘价
						}
						else if (m_pSecurity->IsStockA() && pHisTradeData->Amount > 0.0 && pHisTradeData->Volume > 0.0)
							calcuTable.SetCell(5,j,static_cast<float>(pHisTradeData->Amount/pHisTradeData->Volume));//收盘价
						else	// 非A,B股
							calcuTable.SetCell(5,j,Tx::Core::Con_floatInvalid);//收盘价
						//calcuTable.SetCell(5,j,static_cast<float>(pHisTradeData->Amount/pHisTradeData->Volume));//均价--当天的成交金额/成交量
					}
					pHisTradeData=NULL;
				}

				//calcuTable.SetCell(5,j,jlr);//净利润 单独填
				if(bPB)
				{
					calcuTable.SetCell(8,j,dRight);
				}
				if (bPE)
				{
					calcuTable.SetCell(6,j,dProfit);
				}				
				j++;
			}

			// pbpe的合计的计算器
			if (bPB)
			{
				////Tx::Core::Table_Indicator qyTable;
				////qyTable.CopyColumnInfoFrom(AllqyTable);
				////AllqyTable.Between(qyTable,nArray,2,0,min_date+2,max_date,true,true);
				int k=0;
				for (UINT i=0;i<calcuTable.GetRowCount();i++)
				{
					// 最新财务数据的终止日期
					double dqy=0.0;
					calcuTable.GetCell(8,i,dqy);

					//汇总剔除无效值
					if (dqy > 1e14 ||dqy <-1e14 )
					{
						k++;
						continue;
					}

					double ddygb = 0.0;
					double dzgb=0.0;
					double dltgb=0.0;
					float fprice=0.0;
					calcuTable.GetCell(2,i,ddygb);//对应股本
					calcuTable.GetCell(3,i,dltgb);//流通本
					calcuTable.GetCell(4,i,dzgb);//总股本
					//根据iHZYB选择剔除
					if (1==iHZYB && dqy<0.0)//剔除亏损
						dqy = 0.0;
					else if (2==iHZYB && dqy * ddygb / dzgb <dwl*ddygb)//剔除微利
						dqy = 0.0;

					// modified by zhoup 2008.11.18
					if (ddygb > 0.0 && dzgb > 0.0)
						calcuTable.SetCell(8,i,dqy * ddygb / dzgb);

					if (fabs(dqy-0.0) > 0.000001)
					{
						calcuTable.GetCell(3,i,dltgb);//流通股本
						calcuTable.GetCell(5,i,fprice);//收盘
					}

					if (0 == iJQFS && ddygb > 0.0 && dzgb > 0.0)
					{
						fzpb_weight[k]+=ddygb*fprice;
						fmpb_weight[k]+=dqy * ddygb / dzgb;
					}
					else if ( 1== iJQFS && dltgb > 0.0 && dzgb > 0.0)
					{
						fzpb_weight[k]+=dltgb*fprice;
						//if (fabs(dzgb-0.0) > 0.000001) // 总股本不为0,总股本没数据时,取出来是0
						fmpb_weight[k]+=(dqy*dltgb/dzgb);
					}
					k++;
				}
			}
			if (bPE)
			{
				{
					int k=0;
					for (UINT i=0;i<calcuTable.GetRowCount();i++)
					{

						double dlr=0.0;
						calcuTable.GetCell(6,i,dlr);
						//汇总剔除无效值
						if (dlr > 1e14 ||dlr <-1e14 )
						{
							k++;
							continue;
						}

						// 存入vector
						double ddygb = 0.0;
						double dzgb=0.0;
						double dltgb=0.0;
						float fprice=0.0;
						calcuTable.GetCell(2,i,ddygb);//对应股本
						calcuTable.GetCell(3,i,dltgb);//流通股本
						calcuTable.GetCell(4,i,dzgb);//总股本
						//根据iHZYB选择剔除
						if (1==iHZYB && dlr<0.0)//剔除亏损
							dlr = 0.0;
						else if (2==iHZYB && dlr * ddygb / dzgb <dwl*ddygb)//剔除微利
							dlr = 0.0;

						if (ddygb > 0.0 && dzgb > 0.0)
							calcuTable.SetCell(6,i,dlr * ddygb / dzgb);
						else
							calcuTable.SetCell(6,i,Tx::Core::Con_doubleInvalid);

						if (fabs(dlr-0.0) > 0.000001)
						{
							calcuTable.GetCell(3,i,dltgb);//流通股本
							calcuTable.GetCell(5,i,fprice);//收盘
						}

						if (0 == iJQFS && ddygb > 0.0 && dzgb > 0.0)
						{
							fzpe_weight[k]+=ddygb*fprice;
							if (fabs(dzgb-0.0) > 0.000001) 
							{
								fmpe_weight[k]+=dlr * ddygb / dzgb;
							}
						}
						else if ( 1== iJQFS && dltgb > 0.0 && dzgb > 0.0)
						{
							fzpe_weight[k]+=dltgb*fprice;
							if (fabs(dzgb-0.0) > 0.000001) // 总股本不为0,总股本没数据时,取出来是0
								fmpe_weight[k]+=(dlr*dltgb/dzgb);
						}

						k++;
					}
				}

			}

			//CString temp = calcuTable.TableToString();

			// 计算
			// 当利润不为0.0时,做除法(第2列*第三列/第四列)
			// 按照汇总条件,将每天的结果,计入汇总值
			//往resultTalbe中填入个股PEPB数据
			resultTable.AddRow();
			if (bPE && bPB)
			{
				for (UINT i=0;i<calcuTable.GetRowCount();i++)
				{
					//int date1=-1;
					//calcuTable.GetCell(1,i,date1);
					double dlr=0.0;
					double ddygb=0.0;
					float fpclose=0.0;
					double dqy = 0.0;
					double dpb = -DBL_MAX;
					double dpe=-DBL_MAX;//无效值
					calcuTable.GetCell(5,i,fpclose);
					//修正错误，算个股市盈率不管是选择总股本加权还是流通股本加权都是用总股本*价格/利润
					////或者是对应股本*价格/对应利润
					//if (0 == iJQFS )	//取对应股本
					//{
					//	calcuTable.GetCell(2,i,ddygb);
					//}
					//else if ( 1== iJQFS )	//取流通股本
					//{
					//	calcuTable.GetCell(3,i,ddygb);
					//}
					calcuTable.GetCell(2,i,ddygb);
					//calcuTable.GetCell(2,i,ddygb);
					calcuTable.GetCell(6,i,dlr);
					calcuTable.GetCell(8,i,dqy);
					if (fabs(dlr-0.0)>0.000000001)
					{
						dpe=ddygb * (double)fpclose/dlr;
					}
					if (fabs(dqy-0.0)>0.000000001)
					{
						dpb= ddygb *(double)fpclose/dqy;
					}
					TRACE(_T("\r\n股票[%s(%d)],市盈率[%.2f]\r\n"),m_pSecurity->GetName(),m_pSecurity->GetId(),dpe);
					calcuTable.SetCell(7,i,dpe);
					calcuTable.SetCell(9,i,dpb);

					// 每算一个填一个
					//权益无效，剔除无效值
					if (dqy > 1e14 ||dqy <-1e14 )
					{
						resultTable.SetCell(3+2*i,iRow,Tx::Core::Con_doubleInvalid);
					}
					else
					{
						//if (fabs(dqy-0.0)>0.000000001)
						//{
						//	resultTable.SetCell(3+2*i,iRow,dpb);
						//}
						resultTable.SetCell(3+2*i,iRow,dpb);

					}

					//利润无效，剔除无效值
					if (dlr > 1e14 ||dlr <-1e14 )
					{
						resultTable.SetCell(2+2*i,iRow,Tx::Core::Con_doubleInvalid);
					}
					else
					{
						//if (fabs(dlr-0.0)>0.000000001)
						//{
						//	resultTable.SetCell(2+2*i,iRow,dpe);
						//}
						resultTable.SetCell(2+2*i,iRow,dpe);

					}


				}

				CString strName=m_pSecurity->GetName();
				resultTable.SetCell(1,iRow,m_pSecurity->GetCode());
				resultTable.SetCell(0,iRow,strName);
				resultTable.SetCell(resultTable.GetColCount()-1,iRow,(int)m_pSecurity->GetId());

				////临时增加测试
				//CString cal = calcuTable.TableToString();
				//CString res = resultTable.TableToString();

				iRow++;

				dpro+=1.0;
				prw.SetPercent(pId,dpro/iSecurityId.size());
			}
			else if (bPE)
			{
				for (UINT i=0;i<calcuTable.GetRowCount();i++)
				{
					double dlr=0.0;
					double ddygb=0.0;
					float fpclose=0.0;
					double dpe=-DBL_MAX;//无效值
					calcuTable.GetCell(5,i,fpclose);
					////修正错误，算个股市盈率不管是选择总股本加权还是流通股本加权都是用总股本*价格/利润
					////或者是对应股本*价格/对应利润
					//if (0 == iJQFS )	//取对应股本
					//{
					//	calcuTable.GetCell(2,i,ddygb);
					//}
					//else if ( 1== iJQFS )	//取流通股本
					//{
					//	calcuTable.GetCell(3,i,ddygb);
					//}
					calcuTable.GetCell(2,i,ddygb);
					//calcuTable.GetCell(2,i,ddygb);
					calcuTable.GetCell(6,i,dlr);

					//利润无效，剔除无效值
					if (dlr > 1e14 ||dlr <-1e14 )
					{
						resultTable.SetCell(2+i,iRow,Tx::Core::Con_doubleInvalid);
					}
					else
					{
						if (fabs(dlr-0.0)>0.000000001)
						{
							dpe=ddygb*(double)fpclose/dlr;
						}
						calcuTable.SetCell(7,i,dpe);

						TRACE(_T("\r\n股票[%s(%d)],市盈率[%.2f]\r\n"),m_pSecurity->GetName(),m_pSecurity->GetId(),dpe);
						// 每算一个填一个
						resultTable.SetCell(2+i,iRow,dpe);
					}


				}
				CString strName=m_pSecurity->GetName();
				resultTable.SetCell(1,iRow,m_pSecurity->GetCode());
				resultTable.SetCell(0,iRow,strName);
				resultTable.SetCell(resultTable.GetColCount()-1,iRow,(int)m_pSecurity->GetId());

				iRow++;

				dpro+=1.0;
				prw.SetPercent(pId,dpro/iSecurityId.size());
			}
			else
			{
				for (UINT i=0;i<calcuTable.GetRowCount();i++)
				{
					double ddygb=0.0;
					float fpclose=0.0;
					double dqy = 0.0;
					double dpb = -DBL_MAX;//无效值
					calcuTable.GetCell(5,i,fpclose);
					////修正错误，算个股市盈率不管是选择总股本加权还是流通股本加权都是用总股本*价格/利润
					////或者是对应股本*价格/对应利润
					//if (0 == iJQFS )	//取对应股本
					//{
					//	calcuTable.GetCell(2,i,ddygb);
					//}
					//else if ( 1== iJQFS )	//取流通股本
					//{
					//	calcuTable.GetCell(3,i,ddygb);
					//}
					calcuTable.GetCell(2,i,ddygb);
					//calcuTable.GetCell(2,i,ddygb);
					calcuTable.GetCell(8,i,dqy);
					//if (fabs(dqy-0.0)>0.000000001)
					//{
					//	dpb= ddygb *(double)fpclose/dqy;
					//}
					//TRACE(_T("\r\n股票[%s(%d)],市盈率[%.2f]\r\n"),m_pSecurity->GetName(),m_pSecurity->GetId(),dpe);
					//calcuTable.SetCell(9,i,dpb);

					// 每算一个填一个
					//权益无效，剔除无效值
					if (dqy > 1e14 ||dqy <-1e14 )
					{
						resultTable.SetCell(2+i,iRow,Tx::Core::Con_doubleInvalid);
					}
					else
					{
						if (fabs(dqy-0.0)>0.000000001)
						{
							dpb=ddygb*(double)fpclose/dqy;
						}
						calcuTable.SetCell(7,i,dpb);
						resultTable.SetCell(2+i,iRow,dpb);
					}


				}
				CString strName=m_pSecurity->GetName();
				resultTable.SetCell(1,iRow,m_pSecurity->GetCode());
				resultTable.SetCell(0,iRow,strName);
				resultTable.SetCell(resultTable.GetColCount()-1,iRow,(int)m_pSecurity->GetId());

				iRow++;

				dpro+=1.0;
				prw.SetPercent(pId,dpro/iSecurityId.size());
			}
		} // end for if (m_pSecurity->IsStockA() || m_pSecurity->IsStockB())
	}// for end
	// end for (std::set<int>::iterator iter=iSecurityId.begin();iter!=iSecurityId.end();iter++,iIndexOfColumn++)
	//当没有有效的个股样本时，不予计算
	CString strResult = resultTable.TableToString();
	if(0 == resultTable.GetRowCount())
	{
		AfxMessageBox(_T("对不起，没有取到有效的样本，请确认你的样本包括个股样本！"));
		return false;

	}

	if (pUSDRatio != NULL)
	{
		delete pUSDRatio;
		pUSDRatio = NULL;
	}
	if (pHKDRatio != NULL)
	{
		delete pHKDRatio;
		pHKDRatio = NULL;
	}

	// 加权
	resultTable.AddRow();
	CString strTitle;
	if (0 == iJQFS)
		strTitle=_T("总股本加权");
	else if (1 == iJQFS)
		strTitle=_T("流通股本加权");
	resultTable.SetCell(1,iRow,strTitle);
	//resultTable.SetCell(0,iRow,Tx::Core::Con_strInvalid);
	resultTable.SetCell(0,iRow,CString(_T("合计"))); //modified by zhangxs 20081218

	if (bPE && bPB)
	{
		for (UINT i=0;i<fzpb_weight.size();i++)
		{
			if (fabs(fzpb_weight[i]-0.0)>0.000001 && fabs(fmpb_weight[i]-0.0)>0.000001)
				resultTable.SetCell(3+2*i,iRow,fzpb_weight[i]/fmpb_weight[i]);
			else
				resultTable.SetCell(3+2*i,iRow,Tx::Core::Con_doubleInvalid);
		}
		for (UINT i=0;i<fzpe_weight.size();i++)
		{
			TRACE(_T("\r\n汇总,fz[%.4f],fm[%.4f],市盈率[%.4f]\r\n"),fzpe_weight[i],fmpe_weight[i],fzpe_weight[i]/fmpe_weight[i]);
			if (fabs(fzpe_weight[i]-0.0)>0.000001 && fabs(fmpe_weight[i]-0.0)>0.000001)
				resultTable.SetCell(2+2*i,iRow,fzpe_weight[i]/fmpe_weight[i]);
			else
				resultTable.SetCell(2+2*i,iRow,Tx::Core::Con_doubleInvalid);
		}
	}
	else if (bPE)
	{
		for (UINT i=0;i<fzpe_weight.size();i++)
		{
			TRACE(_T("\r\n汇总,fz[%.4f],fm[%.4f],市盈率[%.4f]\r\n"),fzpe_weight[i],fmpe_weight[i],fzpe_weight[i]/fmpe_weight[i]);
			if (fabs(fzpe_weight[i]-0.0)>0.000001 && fabs(fmpe_weight[i]-0.0)>0.000001)
				resultTable.SetCell(2+i,iRow,fzpe_weight[i]/fmpe_weight[i]);
			else
				resultTable.SetCell(2+i,iRow,Tx::Core::Con_doubleInvalid);
		}
	}
	else
	{
		for (UINT i=0;i<fzpb_weight.size();i++)
		{
			if (fabs(fzpb_weight[i]-0.0)>0.000001 && fabs(fmpb_weight[i]-0.0)>0.000001)
				resultTable.SetCell(2+i,iRow,fzpb_weight[i]/fmpb_weight[i]);
			else
				resultTable.SetCell(2+i,iRow,Tx::Core::Con_doubleInvalid);
		}
	}

	prw.SetPercent(pId,1.0);
	calcuTable.Clear();
#ifdef _DEBUG
	Tx::Core::TxDateTime endtime=Tx::Core::TxDateTime::getCurrentTime();
	TRACE("\n--------------------------------------------------------------------------------------------------------------------------------\n");
	TRACE("统计%d只股票(PE,%d--%d)用时:%.6f",iSecurityId.size(),nStartDate,nEndDate,(endtime-starttime).GetTotalSeconds());
	TRACE("\n--------------------------------------------------------------------------------------------------------------------------------\n");
	//strTable = resultTable.TableToString();
	//Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	int iSortLine = resultTable.GetRowCount();
	resultTable.SetSortRange(0,resultTable.GetRowCount() - 3);
	return true;
}

bool TxStock::GetDataOfProfitAndRight(
									  int iYear,							//财年
									  int iQuarter,						//财季
									  bool iIsProfit,					//是利润还是权益 true 利润 false 权益
									  std::vector<int>& iSecurityId,		//机构的ID
									  std::vector<double>* iDateVector	//获得利润或者权益数据
									  )
{
	bool bRet = false;


	// Binary
	int iSize = sizeof(int) * (1+1+1+1+1+iSecurityId.size());
	LPBYTE pBuffer = new BYTE [iSize];
	if (pBuffer == NULL)
		return false;
	LPBYTE pWrite = pBuffer;
	memset(pBuffer,0,iSize);
	//add by wangyc 20090504
	//写入是取权益利润还是直接取板块PEPB 和板块市盈率走统一接口
	int iLrFlag = 0;
	memcpy_s(pWrite,iSize,&iLrFlag,sizeof(int));
	pWrite += sizeof(int);
	//写入权益利润的标志位
	int ilr = 0;
	if (iIsProfit)
	{
		ilr = 0;
	} 
	else
	{
		ilr = 1;
	}
	memcpy_s(pWrite,iSize,&ilr,sizeof(int));
	pWrite += sizeof(int);
	//写入财年财季和"利润或者收益"的标志
	memcpy_s(pWrite,iSize,&iYear,sizeof(int));
	pWrite += sizeof(int);
	memcpy_s(pWrite,iSize,&iQuarter,sizeof(int));
	pWrite += sizeof(int);

	//写入机构的ID
	int nSecuritySize = (int)iSecurityId.size();
	memcpy_s(pWrite,iSize,&nSecuritySize,sizeof(int));
	pWrite += sizeof(int);
	//for (UINT i=0;i<nSecuritySize;i++)
	//{
	//	memcpy_s(pWrite,iSize,&iSecurityId[i],sizeof(int));
	//	pWrite += sizeof(int);
	//}

	for (std::vector<int>::iterator iter = iSecurityId.begin();iter!=iSecurityId.end();iter++)
	{
		int iID = *iter;
		memcpy_s(pWrite,iSize,&iID,sizeof(int));
		pWrite += sizeof(int);
	}

	//LPCTSTR lpUrl = _T("http://221.122.41.211/StockPEPBSer/Handler.ashx");
	//LPCTSTR lpUrl = _T("http://192.168.5.87/StockPEPBSer/Handler.ashx");
	LPCTSTR lpUrl = CORE_GETSYSINFO->GetServerAddr(_T("File"),_T("InstitutionRightsAndProfits"));
	Tx::Drive::Http::CSyncUpload upload;
	int iStart = ::GetTickCount();
	//	if ( upload.Post(lpUrl, (LPBYTE)lpwstr, nSize) )
	if ( upload.Post(lpUrl, pBuffer, iSize) )
	{
		int iEnd = ::GetTickCount();
		TRACE(_T("\r\nURL Cost Time %d(ms)\r\n"),iEnd-iStart);
		CONST Tx::Drive::Mem::MemSlice &data = upload.Rsp().Body();
		LPBYTE lpRes = data.DataPtr();
		UINT nRetSize = data.Size();
		if (nRetSize <= 0)
		{
			delete pBuffer;
			pBuffer = NULL;
			return false;
		}

		UINT nPreSize = *(reinterpret_cast<UINT*>(lpRes));
		//增加判断返回数据的有效性
		if (nPreSize/sizeof(float) != nSecuritySize)
		{
			delete pBuffer;
			pBuffer = NULL;
			return false;
		}

		LPBYTE lpData = new BYTE[nPreSize];
		if ( lpData == NULL )
		{
			delete pBuffer;
			pBuffer = NULL;
			return false;
		}
		if ( !Tx::Drive::IO::Zip::CZipWrapper::MemUnZip(lpData, 
			nPreSize, lpRes + sizeof(UINT), nRetSize - sizeof(UINT)) )
		{
			delete []lpData;
			delete[] pBuffer;
			pBuffer = NULL;
			return false;
		}

		// 解析结果 value1,value2....,id2,value1,value2,
		iStart = ::GetTickCount();
		LPBYTE pRecv = lpData;
		float fValue = 0.0;
		double dValue = 0.0;
		for (UINT i=0;i < (UINT)nSecuritySize; i++)
		{
			memcpy_s(&fValue,sizeof(float),pRecv,sizeof(float));
			pRecv += sizeof(float);
			if (fabs(fValue - Tx::Core::Con_floatInvalid) < 0.00001)
				// 无效值
				iDateVector->push_back(Tx::Core::Con_doubleInvalid);
			//resultTable.SetCell(i%date.size() + 1,i/date.size(),Tx::Core::Con_doubleInvalid);
			else
			{
				dValue = (double)fValue;
				iDateVector->push_back(dValue);
				//resultTable.SetCell(i%date.size() + 1,i/date.size(),dValue);
				bRet = true;	//只要有一个有效值就返回true
			}
		}
		delete []lpData;
		lpData = NULL;
		iEnd = ::GetTickCount();
		TRACE(_T("\r\nParse Result Cost Time %d(ms)\r\n"),iEnd-iStart);
	}
	delete[] pBuffer;
	pBuffer = NULL;

	return bRet;

}

//默认按照股票处理
bool TxStock::GetBlockAnalysisCol(Table_Display& baTable,std::vector<int>& arrSamples,int& iSortCol)
{
	iSortCol = 2;
	//如果UI没有输入列信息，自行添加
	if(baTable.GetColCount()<=0)
	{
		//加入样本for test
		//0交易实体ID
		baTable.AddCol(Tx::Core::dtype_int4);
		//1交易实体名称
		baTable.AddCol(Tx::Core::dtype_val_string);
		//2交易实体外码
		baTable.AddCol(Tx::Core::dtype_val_string);
		//2008-02-19
		//3样本原始顺序
		baTable.AddCol(Tx::Core::dtype_int4);

		//2007-08-23
		//增加 现量=最新成交量
		//加入列for test
		//前收、现价、涨跌、涨幅、换手率、现量、成交量、成交金额、流通股本、总股本、流通市值、总股市值
		//007：排序分析中，顺利为：涨幅、现价、钱收、涨跌 2007-12-12
		//4
		baTable.AddCol(Tx::Core::dtype_double);
		//5
		baTable.AddCol(Tx::Core::dtype_float);
		//6
		baTable.AddCol(Tx::Core::dtype_double);

		//7
		baTable.AddCol(Tx::Core::dtype_double);
		//8 buy1
		baTable.AddCol(Tx::Core::dtype_float);
		//2008-12-23
		//9 sell1
		baTable.AddCol(Tx::Core::dtype_float);
		//10
		baTable.AddCol(Tx::Core::dtype_double);

		//11
		baTable.AddCol(Tx::Core::dtype_double);
		//12
		baTable.AddCol(Tx::Core::dtype_double);

		//13
		baTable.AddCol(Tx::Core::dtype_double);//徐：资金流向2008-09-01,单位万元
		//14
		baTable.AddCol(Tx::Core::dtype_double);//换手率
		//add by wangyc 20100301 融资融券最高折算率 
		baTable.AddCol(Tx::Core::dtype_double);//一年滚动
		//15
		baTable.AddCol(Tx::Core::dtype_double);//一年滚动
		//16
		baTable.AddCol(Tx::Core::dtype_double);//预测pe
		//baTable.AddCol(Tx::Core::dtype_int4);//年度

		//17
		baTable.AddCol(Tx::Core::dtype_float);
		//18
		baTable.AddCol(Tx::Core::dtype_val_string);
		//19
		baTable.AddCol(Tx::Core::dtype_val_string);
		//20
		baTable.AddCol(Tx::Core::dtype_float);

		//21天相行业分类名称
		baTable.AddCol(Tx::Core::dtype_double);
		//22天相行业分类代码
		baTable.AddCol(Tx::Core::dtype_double);
		//23 din
		//baTable.AddCol(Tx::Core::dtype_double);
		//24dout
		//baTable.AddCol(Tx::Core::dtype_double);

		int nCol=1;
		baTable.SetTitle(nCol, _T("名称"));
		++nCol;
		baTable.SetTitle(nCol, _T("代码"));
		++nCol;
		baTable.SetTitle(nCol, _T("自选号"));
		++nCol;
		baTable.SetTitle(nCol, _T("涨幅"));
		baTable.SetKeyCol(nCol);
		++nCol;
		baTable.SetTitle(nCol, _T("现价"));
		++nCol;
		baTable.SetTitle(nCol, _T("涨跌"));
		++nCol;
		baTable.SetTitle(nCol, _T("现量(手)"));
		baTable.SetPrecise(nCol, 0);
		//baTable.SetOutputItemRatio(nCol, 100);
		baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
		++nCol;
		//2008-12-23
		baTable.SetTitle(nCol, _T("买一价"));
		++nCol;
		baTable.SetTitle(nCol, _T("卖一价"));
		++nCol;
		baTable.SetTitle(nCol, _T("总股市值(亿)"));
		//小数点
		baTable.SetPrecise(nCol, 2);
		//倍数
		baTable.SetOutputItemRatio(nCol,100000000);
		baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
		++nCol;
		baTable.SetTitle(nCol, _T("一年滚动PE"));
		baTable.SetPrecise(nCol, 1);
		++nCol;
		//baTable.SetTitle(nCol, _T("预测PE  "));
		CString sPeTitle;
		m_iYearPe = GetPeYear(arrSamples);
		sPeTitle.Format(_T("预测PE[%02d]"),m_iYearPe);
		baTable.SetTitle(nCol, sPeTitle);
		baTable.SetPrecise(nCol, 1);

		++nCol;
		baTable.SetTitle(nCol, _T("换手率"));
		++nCol;
		baTable.SetTitle(nCol, _T("成交量(手)"));
		baTable.SetPrecise(nCol, 0);
		//baTable.SetOutputItemRatio(nCol, 100);
		baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
		++nCol;
		baTable.SetTitle(nCol, _T("成交金额(万元)"));
		baTable.SetPrecise(nCol, 0);//cyh80 2009-08-19 改为保留0位小数(2)
		baTable.SetOutputItemRatio(nCol, 10000);
		baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
		++nCol;
		
		baTable.SetTitle(nCol, _T("流通市值(亿)"));
		//小数点
		baTable.SetPrecise(nCol, 2);
		//倍数
		baTable.SetOutputItemRatio(nCol,100000000);
		baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
		++nCol;
	
		//2009-09-01
		//徐：资金流向2008-09-01,单位万元
		baTable.SetTitle(nCol, _T("资金流向(万元)"));
		baTable.SetPrecise(nCol, 0);//cyh80 2009-9-2 改为保留0位小数(2)
		baTable.SetOutputItemRatio(nCol, 10000);
		baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
		++nCol;
		baTable.SetTitle(nCol, _T("前收"));
		++nCol;
		baTable.SetTitle(nCol, _T("天相行业分类名称"));
		++nCol;
		baTable.SetTitle(nCol, _T("天相行业分类代码"));
		++nCol;
		//2008-04-25 从现量后移来

		//add by wangyc 20100301 融资融券最高折算率 
		baTable.SetTitle(nCol, _T("最高折算率%"));
		baTable.SetPrecise(nCol, 0);
		++nCol;



		//baTable.SetTitle(nCol, _T("预测PE年度"));
		//++nCol;

		//股票流通股本
		//SetDisplayTableColInfo(&baTable,nCol,30000050);
		baTable.SetTitle(nCol, _T("流通股本(亿)"));
		//小数点
		baTable.SetPrecise(nCol, 2);
		//倍数
		baTable.SetOutputItemRatio(nCol,100000000);
		baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
		++nCol;
		//股票总股本
		//SetDisplayTableColInfo(&baTable,nCol,30000049);
		baTable.SetTitle(nCol, _T("总股本(亿)"));
		//小数点
		baTable.SetPrecise(nCol, 2);
		//倍数
		baTable.SetOutputItemRatio(nCol,100000000);
		baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
		++nCol;
		//SetDisplayTableColInfo(&baTable,nCol,30000050,false);
		//SetDisplayTableColInfo(&baTable,nCol,30000050,false);

		//baTable.SetTitle(nCol, _T("资金流入"));
		//++nCol;
		//baTable.SetTitle(nCol, _T("资金流出"));
		//++nCol;
	}
	return true;
}
//排序分析基本数据结果设置
bool TxStock::SetBlockAnalysisBasicCol(Table_Display& baTable,SecurityQuotation* psq,int& j,int ii,int idate,int iSamplesCount)
{
	float prePrice	= Con_floatInvalid;
	float closePrice	= Con_floatInvalid;
	float openPrice	= Con_floatInvalid;
	float lowPrice	= Con_floatInvalid;
	float highPrice	= Con_floatInvalid;
	double amount		= Con_doubleInvalid;
	double volume		= Con_doubleInvalid;
	double dRaiseValue = Con_doubleInvalid;
	double dRaise = Con_doubleInvalid;
	double dHsl = Con_doubleInvalid;
#ifdef _DEBUG
	//int iTestId = psq->GetId();
	//bool bn = psq->IsNormal();
	//bool bs = psq->IsStop();
	//bool bh = psq->IsHalt();
	//bool bhl = psq->IsHaltLong();

#endif
	//int idate = psq->GetCurDataDate();
	prePrice	= psq->GetPreClosePrice();
	double tradeableShare = psq->GetTradableShare();
	double share = psq->GetTotalShare();
	double dValue = Con_doubleInvalid;
	//int idate = psq->GetCurDataDate();
	closePrice = psq->GetClosePrice(true);
	//if(psq->IsValid()==true)
	{
		//closePrice	= psq->GetClosePrice(true);
		openPrice	= psq->GetOpenPrice();
		//lowPrice	= psq->GetLowPrice();
		//highPrice	= psq->GetHighPrice();
		amount		= psq->GetAmount();
		volume		= psq->GetVolume(true);

		//2009-04-21 集合竞价期间涨幅和涨跌采取买一价来计算
		if(TX_DATA_TRADE_STATUS_BEFORE0 == DataStatus::GetInstance()->GetStatusInt(Tx::Data::tag_DATA_TRADE_TIME_STATUS,psq->GetInnerBourseId()))
		{
			TradeQuotation* pTradeQuotation = psq->GetFirstTradeQuotationPointer();
			if(pTradeQuotation!=NULL)
			{
				//get preclose
				//get buy price 1
				TradeQuotationData* pTradeQuotationData = pTradeQuotation->GetBuyData(1);
				if(pTradeQuotationData!=NULL)
				{
					//2009-04-22 徐按照王倩的建议决定如此
					if(pTradeQuotationData->fPrice>0)
						closePrice = pTradeQuotationData->fPrice;
					//2009-05-22
					if(!(closePrice>0))
						closePrice = prePrice;
					if(prePrice>0 && closePrice>0)
					{
						dRaiseValue = closePrice-prePrice;
						dRaise = dRaiseValue/prePrice*100;
					}
				}
			}
		}
		else
		{
			dRaise = psq->GetRaise();
			dRaiseValue = psq->GetRaiseValue();
		}
		//dHsl = psq->GetTradeRate();
	}
	float buyPrice1	= 0;
	float salePrice1	= 0;
	TradeQuotation* pTradeQuotation = NULL;
	if(Tx::Core::UserInfo::GetInstance()->GetNetType()==1)
	{
		if(iSamplesCount<=50)
			pTradeQuotation = psq->GetTradeQuotationPointer();
		else
			pTradeQuotation = psq->GetFirstTradeQuotationPointer();
	}
	else
		pTradeQuotation = psq->GetFirstTradeQuotationPointer();

	if(pTradeQuotation!=NULL)
	{
		TradeQuotationData* pTradeQuotationData = pTradeQuotation->GetBuyData(1);
		if(pTradeQuotationData!=NULL)
			buyPrice1 = pTradeQuotationData->fPrice;
		pTradeQuotationData = pTradeQuotation->GetSaleData(1);
		if(pTradeQuotationData!=NULL)
			salePrice1 = pTradeQuotationData->fPrice;
	}

	//现量
	double dVolumeNow = Con_doubleInvalid;
	double dAmount = Con_doubleInvalid;

	double dValuein = Con_doubleInvalid;
	double dValueout = Con_doubleInvalid;
	if(psq->IsStop()==false)
	{
		dVolumeNow = psq->GetVolumeLatest(true);
		if(!(dVolumeNow>0))
			dVolumeNow = Con_doubleInvalid;
		
		if(psq->IsIndex()==false)
		{
			dValuein = psq->GetBuyAmount();
			dValueout = psq->GetSaleAmount();
		}
		else
		{
			GetAmountFlow(psq->GetId(),dValuein,dValueout);
		}
		if(dValuein>-0.001 && dValueout>-0.001)
		{
			dAmount = dValuein - dValueout;
		}
		
		if(fabs(dAmount)<0.000001)
			dAmount = Con_doubleInvalid;
//TRACE("\n\nid=%d, in=%.4f,out=%.4f[%.4f],amount=%.4f",psq->GetId(),dValuein,dValueout,dValuein+dValueout,amount);
		/*
		if(psq->GetBuyAmount()>-0.001 && psq->GetSaleAmount()>-0.001)
		{
			dAmount = psq->GetBuyAmount() - psq->GetSaleAmount();
		}
		if(fabs(dAmount)<0.000001)
			dAmount = Con_doubleInvalid;
		*/
	}

	/*
	int iDetailCount = 0;
	iDetailCount = psq->GetTrdaeDetailDataCount();
	if(iDetailCount<=0)
		dVolumeNow = Con_doubleInvalid;
	else
	{
		if(iDetailCount==1)
			dVolumeNow = volume;
		else
			dVolumeNow = volume - psq->GetTrdaeDetailData(iDetailCount-2)->dVolume;
	}
	*/

	CString sName;
	CString sExtCode;
	sName = Con_strInvalid;
	sExtCode = Con_strInvalid;
	sName = psq->GetName();
	sExtCode = psq->GetCode();

	//设置名称
		baTable.SetCell(j,ii,sName);
		j++;
	//设置外码
		baTable.SetCell(j,ii,sExtCode);
		j++;
		//2008-02-19
		//原始顺序
		j++;
	//涨幅
		baTable.SetCell(j,ii,dRaise);
		j++;
	//现价
		if(psq->IsFund() || (psq->IsStockB() && psq->IsShanghai()) )
			baTable.SetPrecise(j, ii, 3);
		baTable.SetCell(j,ii,closePrice);
		j++;
	//涨跌
		if(psq->IsFund() || (psq->IsStockB() && psq->IsShanghai()) )
			baTable.SetPrecise(j, ii, 3);
		baTable.SetCell(j,ii,dRaiseValue);
		j++;
	//现量
		baTable.SetCell(j,ii,dVolumeNow);
		j++;
//2009-05-14
if(prePrice>0 && closePrice>0 && dRaise<-100)
TRACE("\n\r Err name=%s,code=%s,raise %.4f,raiseValue=%.4f,preprice=%.4f,closeprice=%.4f",
	  sName,sExtCode,
	  dRaise,dRaiseValue,prePrice,closePrice
	  );
if(prePrice<0 && closePrice<0)
TRACE("\n\r Err name=%s,code=%s,raise %.4f,raiseValue=%.4f,preprice=%.4f,closeprice=%.4f",
	  sName,sExtCode,
	  dRaise,dRaiseValue,prePrice,closePrice
	  );
	//买一价
		if(psq->IsFund() || (psq->IsStockB() && psq->IsShanghai()) )
			baTable.SetPrecise(j, ii, 3);
		baTable.SetCell(j,ii,buyPrice1);
		j++;
	//卖一价
		if(psq->IsFund() || (psq->IsStockB() && psq->IsShanghai()) )
			baTable.SetPrecise(j, ii, 3);
		baTable.SetCell(j,ii,salePrice1);
		j++;
		//总市值
		if(share>0 && closePrice>0)
			dValue = closePrice*share;
		else
			//流通股本不可能小于0
			dValue = Tx::Core::Con_doubleInvalid;
		baTable.SetCell(j,ii,dValue);
		j++;
				// added by zhoup 2008.04.25
		// 市盈率的计算
		double dLtmPE = Con_doubleInvalid,dEPS = Con_doubleInvalid;
		double dDynamicPE = Con_doubleInvalid,dDynamicEPS = Con_doubleInvalid;

		int iYear = COleDateTime::GetCurrentTime().GetYear()%100;

		//double dclose = psq->GetClosePrice(true);

		if (psq != NULL)
		{
			SecurityEPS_PE* pSecurityEPS_PE = psq->GetSecurityEPS_PE();			
			/*if(pSecurityEPS_PE!=NULL && (pSecurityEPS_PE->f_year == psq->GetCurDataDateTime().GetYear()
				||pSecurityEPS_PE->f_year+1 == psq->GetCurDataDateTime().GetYear()))
			{
				if(pSecurityEPS_PE->f_Now_EPS > 0.0 && closePrice>0)
				{
					dLtmPE = closePrice / pSecurityEPS_PE->f_Now_EPS;
				}
				if(pSecurityEPS_PE->f_Next_EPS > 0.0 && closePrice>0)
				{
					dDynamicPE = closePrice / pSecurityEPS_PE->f_Next_EPS;
				}
			}*/
			//modified by zhangxs 20110107
			//一年滚动pe不受时间限制，预测pe通过样本，年报披露最新财年来算，如果和披露最新财年相等的则展示；
			if(pSecurityEPS_PE != NULL)
			{
				if(pSecurityEPS_PE->f_Now_EPS > 0.0 && closePrice>0)
				{
					dLtmPE = closePrice / pSecurityEPS_PE->f_Now_EPS;
				}
				if(pSecurityEPS_PE->f_year%100 == m_iYearPe-1 && pSecurityEPS_PE->f_Next_EPS > 0.0 && closePrice>0)
				{
					dDynamicPE = closePrice / pSecurityEPS_PE->f_Next_EPS;
				}			
			}
			/*
			iYear = psq->GetCurDataDateTime().GetYear();
			// 利润,权益
			InstitutionNewInfo* pInstitutionInfo = psq->GetInstitutionNewInfo();
			if (pInstitutionInfo != NULL)
			{
				// 价格
				PePb* pPePb = psq->GetPePbDataLatest();
				if (pPePb != NULL)
				{
					// 股本
					StockNewInfo* pStockNewInfo = psq->GetStockNewInfo();
					if (pStockNewInfo != NULL)
					{
						// 一年滚动pe
						dEPS = pPePb->ltm_eps;
						double dclose = psq->GetClosePrice(true);
						if (pPePb->ltm_eps < -100.0 || dclose < 0.0)
							dLtmPE = Con_doubleInvalid;
						else
							dLtmPE = dclose / dEPS;
//						TRACE(_T("\r\n股票名称:%s\r\n滚动EPS:%f\r\n实时价格:%.2f\r\n滚动PE:%f\r\n"),p->GetName(),dEPS,dclose,dLtmPE);
						//}

						// 得到年报的年份
						int iFiscalYear=0;
						if (pInstitutionInfo->fiscal_year_quarter == 40040009)
							iFiscalYear = pInstitutionInfo->fiscal_year;
						else
							iFiscalYear = pInstitutionInfo->fiscal_year-1;

						// 已披露年报pe
						if (iFiscalYear == iYear - 1)
						{
							//2008-10-22
							//原来使用前收价，现在使用现价
							if (pInstitutionInfo->dynamic_eps1 < -100.0 || dclose < 0.0)
								dDynamicPE = Con_doubleInvalid;
							else
								dDynamicPE = dclose / pInstitutionInfo->dynamic_eps1;
							dDynamicEPS = pInstitutionInfo->dynamic_eps1;
						}
						else
						{
							iYear--;
							// 价格或者EPS无效,PE无效
							//2008-10-22
							//原来使用前收价，现在使用现价
							if (pInstitutionInfo->dynamic_eps2 < -100.0 || dclose < 0.0)
								dDynamicPE = Con_doubleInvalid;
							else
								dDynamicPE = dclose / pInstitutionInfo->dynamic_eps2;
							dDynamicEPS = pInstitutionInfo->dynamic_eps2;
						}
					}
				}
			}
			*/
		}


		///////////////////////////////////////////////////////////////////////////////
		//一年滚动 PE
		baTable.SetCell(j,ii,dLtmPE);
		j++;

		//预测 PE
		baTable.SetCell(j,ii,dDynamicPE);
		j++;
		//换手率
		SetBlockAnalysisHslCol(baTable,psq,j,ii);
		j++;
	//成交量
		baTable.SetCell(j,ii,volume);
		j++;
	//成交金额
		baTable.SetCell(j,ii,amount);
		j++;
	
		//流通市值
		if(tradeableShare>0 && closePrice>0)
			dValue = closePrice*tradeableShare;
		else
			//流通股本不可能小于0
			dValue = Tx::Core::Con_doubleInvalid;
		baTable.SetCell(j,ii,dValue);
		j++;

		//2008-09-01
		//资金流向
		baTable.SetCell(j,ii,dAmount);
		j++;
			//前收
		if(psq->IsFund() || (psq->IsStockB() && psq->IsShanghai()) )
			baTable.SetPrecise(j, ii, 3);
		baTable.SetCell(j,ii,prePrice);
		j++;

		CString sTemp;
		//21
		sTemp = psq->GetTxSecIndustryName(2);
		if(sTemp == _T("-"))
			sTemp = psq->GetTxSecIndustryName(1);
		baTable.SetCell(j,ii,sTemp);
		j++;
		//22
		sTemp = psq->GetTxSecIndustryCode();
		if(sTemp == _T("-"))
			sTemp = psq->GetTxSecIndustryCode(1);
		baTable.SetCell(j,ii,sTemp);
		j++;

		//融资融券数据
		//加载融资融券数据	add by wangyc 20100301
		LoadFinaningSecurityData();

		std::unordered_map<int,float>::iterator iterData = m_pIdToFinaning.find(psq->GetId());
		if (iterData != m_pIdToFinaning.end())
		{
			baTable.SetCell(j,ii,iterData->second);
		}
		else
		{
			baTable.SetCell(j,ii,Con_floatInvalid);
		}
		j++;
		//流通股本
		baTable.SetCell(j,ii,tradeableShare);
		j++;
		//总股本
		baTable.SetCell(j,ii,share);
		j++;
	

		//baTable.SetCell(23,ii,dValuein);
		//baTable.SetCell(24,ii,dValueout);

	return true;
}

bool TxStock::SetBlockAnalysisCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow)
{
	return true;
}

//add by lijw 2008-08-21 下面是有关个股评级的统计
//个股评级数据（10334）
bool TxStock::StatEvaluteData(
							  Tx::Core::Table_Indicator &resTable,	//结果数据表
							  std::vector<int> & iInstitutionId,		//机构样本
							  std::vector<int> & iStockId,			//股票样本
							  int iStartDate,							//起始日期
							  int iEndDate,						//结束日期				
							  bool bIsAllDate				//全部日期
							  )
{
	ProgressWnd prw;
	UINT pid=prw.AddItem(1,_T("个股评级数据统计..."),0.0);
	prw.Show(1000);

	//添加列信息
	resTable.AddCol(Tx::Core::dtype_int4);//股票的交易实体0
	resTable.AddCol(Tx::Core::dtype_val_string);//股票名称1
	resTable.AddCol(Tx::Core::dtype_val_string);//股票代码2
//	resTable.AddCol(Tx::Core::dtype_int4);//研究机构ID 3
	resTable.AddCol(Tx::Core::dtype_val_string);//研究机构名称3
	resTable.AddCol(Tx::Core::dtype_int4);//推荐日期4
	resTable.AddCol(Tx::Core::dtype_val_string);//投资评级5
	resTable.AddCol(Tx::Core::dtype_float);//评级系数6
	resTable.AddCol(Tx::Core::dtype_float);//推荐价格7

	std::unordered_map<int,CString> mapIdToName;
	std::unordered_map<int,CString>::iterator iterMap;
	TypeMapManage::GetInstance()->GetTypeMap(50,mapIdToName);	

	int iRow = -1;
	int iTradeId;
	CString strName;
	CString strCode;
	CString strLevel;
	//判断股票样本的个数
	int icount = (int)iStockId.size();
	if(icount <= 10 || bIsAllDate)
	{		
		//下载按个股拆分的数据文件。
		for(int i = 0;i < icount;i++)
		{
			iTradeId = iStockId[i];
			SecurityQuotation* pSecurity = GetSecurityNow(iTradeId);
			if(NULL == pSecurity)
			{
				return false;//assert(0);
			}
			strName = pSecurity->GetName();
			strCode = pSecurity->GetCode();
			bool Ret = LoadEvaluateSecurity(iTradeId,_T("grade_forecast_of_security"));
			if (Ret == false)
			{
				continue;
			}
			int icountTemp = m_pEvaluateSecurity->GetDataCount();
			for(int j = 0;j < icountTemp;j++)
			{				
				m_pEvaluateSecurityData = m_pEvaluateSecurity->GetDataByIndex(j);
				//判断是否在所选的机构里
				if(iInstitutionId.end() == find(iInstitutionId.begin(),iInstitutionId.end(),m_pEvaluateSecurityData->f_provider_id))
				{
					continue;		
				}
				
				if(bIsAllDate || (m_pEvaluateSecurityData->f_issue_date >= iStartDate && m_pEvaluateSecurityData->f_issue_date <= iEndDate))
				{
					iRow++;
					resTable.AddRow();
					//添加数据
					resTable.SetCell(0,iRow,iTradeId);
					resTable.SetCell(1,iRow,strName);
					resTable.SetCell(2,iRow,strCode);
					//resTable.SetCell(3,iRow,m_pEvaluateSecurityData->f_provider_id);
					resTable.SetCell(4,iRow,m_pEvaluateSecurityData->f_issue_date);
					strLevel.Format(_T("%s"),m_pEvaluateSecurityData->f_provider_grade);
					resTable.SetCell(5,iRow,strLevel);
					resTable.SetCell(6,iRow,m_pEvaluateSecurityData->f_grade_power);
					resTable.SetCell(7,iRow,m_pEvaluateSecurityData->f_then_price);
					iterMap = mapIdToName.find(m_pEvaluateSecurityData->f_provider_id);
					if(iterMap != mapIdToName.end())
					{
						resTable.SetCell(3,iRow,iterMap->second);
					}
				}
			}
		}	
		//添加进度条
		prw.SetPercent(pid,0.7);
	}
	else
	{
		std::set<int> DateSet;
		std::set<int>::iterator iterSet;
		this->CombineDate(iStartDate,iEndDate,DateSet);
		for(iterSet = DateSet.begin();iterSet != DateSet.end();++iterSet)
		{			
			//下载按年份拆分的数据文件。
			bool Ret = LoadEvaluateYear(*iterSet,_T("grade_forecast_of_year"));
			if (Ret == false)
			{
				continue;
			}
			int icountTemp = m_pEvaluateYear->GetDataCount();
			for(int j = 0;j < icountTemp;j++)
			{				
				m_pEvaluateYearData = m_pEvaluateYear->GetDataByIndex(j);					

				//判断是否在所选的日期范围内。
				if(m_pEvaluateYearData->f_issue_date >= iStartDate && m_pEvaluateYearData->f_issue_date <= iEndDate)
				{
					//判断是否在所选的机构里
					if(iInstitutionId.end() == find(iInstitutionId.begin(),iInstitutionId.end(),m_pEvaluateYearData->f_provider_id))
					{
							continue;
					}
						//判断是否在所选的股票里
						iTradeId = m_pEvaluateYearData->f_stock_id;
						if(iStockId.end() == find(iStockId.begin(),iStockId.end(),iTradeId))
					{
						continue;
					}
					iRow++;
					resTable.AddRow();
					//添加数据
					SecurityQuotation* pSecurity = GetSecurityNow(iTradeId);
					if(NULL == pSecurity)
					{
						return false;
						//assert(0);
					}
					strName = pSecurity->GetName();
					strCode = pSecurity->GetCode();
					resTable.SetCell(0,iRow,iTradeId);
					resTable.SetCell(1,iRow,strName);
					resTable.SetCell(2,iRow,strCode);
					//resTable.SetCell(3,iRow,m_pEvaluateYearData->f_provider_id);
					resTable.SetCell(4,iRow,m_pEvaluateYearData->f_issue_date);
					strLevel.Format(_T("%s"),m_pEvaluateYearData->f_provider_grade);
					resTable.SetCell(5,iRow,strLevel);
					resTable.SetCell(6,iRow,m_pEvaluateYearData->f_grade_power);
					resTable.SetCell(7,iRow,m_pEvaluateYearData->f_then_price);
					iterMap = mapIdToName.find(m_pEvaluateYearData->f_provider_id);
					if(iterMap != mapIdToName.end())
					{
						resTable.SetCell(3,iRow,iterMap->second);
					}
				}
			}
		}
		//添加进度条
		prw.SetPercent(pid,0.7);
	}
//	resTable.DeleteCol(3);
	MultiSortRule multisort;
	multisort.AddRule(0,true);
	multisort.AddRule(4,false);
	resTable.SortInMultiCol(multisort);
	resTable.Arrange();
#ifdef _DEBUG
	CString strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//添加进度条
	prw.SetPercent(pid,1.0);
	return true;

}
//add by lijw 2008-10-20 
//个股评级变化（10336）
bool TxStock::StatEvaluteVariety(
								 Tx::Core::Table_Indicator &resTable,	//结果数据表
								 std::vector<int> & iInstitutionId,		//机构样本
								 std::vector<int> & iStockId,			//股票样本
								 int iStartDate,							//起始日期
								 int iEndDate,						//结束日期				
								 bool bIsAllDate					//全部日期
								 )

{
	ProgressWnd prw;
	UINT pid=prw.AddItem(1,_T("个股评级变化..."),0.0);
	prw.Show(1000);
	//添加列信息
	resTable.AddCol(Tx::Core::dtype_int4);//股票的交易实体0
	resTable.AddCol(Tx::Core::dtype_val_string);//股票名称1
	resTable.AddCol(Tx::Core::dtype_val_string);//股票代码2
	resTable.AddCol(Tx::Core::dtype_val_string);//变化状态3	
	resTable.AddCol(Tx::Core::dtype_val_string);//本次评级4
	resTable.AddCol(Tx::Core::dtype_val_string);//上次评级5
	resTable.AddCol(Tx::Core::dtype_val_string);//研究机构6
	resTable.AddCol(Tx::Core::dtype_int4);//推荐日期7

	std::unordered_map<int,CString> mapIdToName;
	std::unordered_map<int,CString>::iterator iterMap;
	TypeMapManage::GetInstance()->GetTypeMap(TYPE_TX_INSTITUTIONID_TO_NAME,mapIdToName);	
	int icount;
	int iRow = -1;
	int iTradeId;
	CString strName,strCode;
	CString strLastLevel,strCurrentLevel;
	CString strVarietyState;
	float fFirstModulus,fSecondModulus;
	//股票样本的个数
	icount = (int)iStockId.size();
	//下载按个股拆分的数据文件。
	for(int i = 0;i < icount;i++)
	{
		//添加进度条
		prw.SetPercent(pid,((double)(i+1))/icount);
		iTradeId = iStockId[i];
		GetSecurityNow(iTradeId);
		if(NULL == m_pSecurity)
			assert(0);
		strName = m_pSecurity->GetName();
		strCode = m_pSecurity->GetCode();
		bool Ret = LoadEvaluateSecurity(iTradeId,_T("grade_forecast_of_security"));
		if (Ret == false)
		{
			continue;
		}
		int icountTemp = m_pEvaluateSecurity->GetDataCount();
		strLastLevel = strCurrentLevel = Tx::Core::Con_strInvalid;
		fFirstModulus = fSecondModulus = Tx::Core::Con_floatInvalid;
		for(int j = 0;j < icountTemp;j++)
		{				
			m_pEvaluateSecurityData = m_pEvaluateSecurity->GetDataByIndex(j);
			//判断是否在所选的机构里
			if(iInstitutionId.end() == find(iInstitutionId.begin(),iInstitutionId.end(),m_pEvaluateSecurityData->f_provider_id))
				continue;				
			//表示是全部日期
			if(bIsAllDate)
			{
				iRow++;
				resTable.AddRow();
				//添加数据
				resTable.SetCell(0,iRow,iTradeId);
				resTable.SetCell(1,iRow,strName);
				resTable.SetCell(2,iRow,strCode);
				fFirstModulus = fSecondModulus;
				fSecondModulus = m_pEvaluateSecurityData->f_grade_power;
				if(fFirstModulus == Tx::Core::Con_floatInvalid || fSecondModulus == Tx::Core::Con_floatInvalid)
					strVarietyState = Tx::Core::Con_strInvalid;
				else
				{
					if(fFirstModulus > fSecondModulus)
						strVarietyState = _T("调低");
					else if(fFirstModulus < fSecondModulus)
						strVarietyState = _T("调高");
					else
						strVarietyState = _T("不变");
				}
				resTable.SetCell(3,iRow,strVarietyState);
				strLastLevel = strCurrentLevel;
				strCurrentLevel = m_pEvaluateSecurityData->f_provider_grade;
				if(strCurrentLevel.IsEmpty())
					strCurrentLevel = Tx::Core::Con_strInvalid;
				resTable.SetCell(4,iRow,strCurrentLevel);
				resTable.SetCell(5,iRow,strLastLevel);
				iterMap = mapIdToName.find(m_pEvaluateSecurityData->f_provider_id);
				if(iterMap != mapIdToName.end())
					resTable.SetCell(6,iRow,iterMap->second);
				//strLevel.Format(_T("%s"),m_pEvaluateSecurityData->f_provider_grade);
				resTable.SetCell(7,iRow,m_pEvaluateSecurityData->f_issue_date);
				
			}
			//表示是指定日期
			else
			{
				//判断是否在所选的日期范围内。
				if(m_pEvaluateSecurityData->f_issue_date >= iStartDate && m_pEvaluateSecurityData->f_issue_date <= iEndDate)
				{
					iRow++;
					resTable.AddRow();
					//添加数据
					resTable.SetCell(0,iRow,iTradeId);
					resTable.SetCell(1,iRow,strName);
					resTable.SetCell(2,iRow,strCode);
					if(0 == iRow)
					{
						m_pEvaluateSecurityData2 = m_pEvaluateSecurity->GetDataByIndex(j - 1);
						fSecondModulus = m_pEvaluateSecurityData2->f_grade_power;
						strCurrentLevel = m_pEvaluateSecurityData2->f_provider_grade;
					}
					fFirstModulus = fSecondModulus;
					fSecondModulus = m_pEvaluateSecurityData->f_grade_power;
					if(fFirstModulus == Tx::Core::Con_floatInvalid || fSecondModulus == Tx::Core::Con_floatInvalid)
						strVarietyState = Tx::Core::Con_strInvalid;
					else
					{
						if(fFirstModulus > fSecondModulus)
							strVarietyState = _T("调低");
						else if(fFirstModulus < fSecondModulus)
							strVarietyState = _T("调高");
						else
							strVarietyState = _T("xiangdeng");
					}
					resTable.SetCell(3,iRow,strVarietyState);
    				strLastLevel = strCurrentLevel;
					strCurrentLevel = m_pEvaluateSecurityData->f_provider_grade;
					if(strCurrentLevel.IsEmpty())
						strCurrentLevel = Tx::Core::Con_strInvalid;
					resTable.SetCell(4,iRow,strCurrentLevel);
					resTable.SetCell(5,iRow,strLastLevel);
					iterMap = mapIdToName.find(m_pEvaluateSecurityData->f_provider_id);
					if(iterMap != mapIdToName.end())
						resTable.SetCell(6,iRow,iterMap->second);
					resTable.SetCell(7,iRow,m_pEvaluateSecurityData->f_issue_date);
				}				
			}

		}
	}
	//	resTable.DeleteCol(3);
	MultiSortRule multisort;
	multisort.AddRule(2,true);//按代码排序
	multisort.AddRule(6,false);//按机构排序。
	resTable.SortInMultiCol(multisort);
	resTable.Arrange();
#ifdef _DEBUG
	CString strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//添加进度条
	prw.SetPercent(pid,1.0);
	return true;

}
//天相个股盈利预测 -- 最新的预测净利润
double TxStock::GetForcastNetProfit(int iStockId,int expectYear)
{
	SecurityQuotation* pSecurity = GetSecurityNow(iStockId);
	if(NULL == pSecurity)
	{
		return Tx::Core::Con_doubleInvalid;
	}
	if(!LoadForcastSecurity(iStockId,_T("profit_forecast_of_security")))
		return Tx::Core::Con_doubleInvalid;
	int icountTemp = m_pForcastSecurity->GetDataCount();
	int maxIssueDate = -1;
	double retNetProfit = Tx::Core::Con_doubleInvalid;

	for(int j = 0;j < icountTemp;j++)
	{
		m_pForcastSecurityData = m_pForcastSecurity->GetDataByIndex(j);
		if(m_pForcastSecurityData->f_provider_id == 1 && m_pForcastSecurityData->f_expect_year == expectYear)
		{
			if(m_pForcastSecurityData->f_issue_date > maxIssueDate)
			{
				maxIssueDate = m_pForcastSecurityData->f_issue_date;
				retNetProfit = m_pForcastSecurityData->f_net_profit;
			}
		}
	}
	return retNetProfit != Tx::Core::Con_doubleInvalid ? retNetProfit*1000000 : retNetProfit;
}
//天相个股盈利预测 -- 最新的预测时间
int TxStock::GetForcastIssueDate(int iStockId)
{
	SecurityQuotation* pSecurity = GetSecurityNow(iStockId);
	if(NULL == pSecurity)
	{
		return Tx::Core::Con_intInvalid;
	}
	if(!LoadForcastSecurity(iStockId,_T("profit_forecast_of_security")))
		return Tx::Core::Con_intInvalid;
	int icountTemp = m_pForcastSecurity->GetDataCount();
	int maxIssueDate = Tx::Core::Con_intInvalid;
	for(int j = 0;j < icountTemp;j++)
	{
		m_pForcastSecurityData = m_pForcastSecurity->GetDataByIndex(j);
		if(m_pForcastSecurityData->f_provider_id == 1)
		{
			if(m_pForcastSecurityData->f_issue_date > maxIssueDate)
				maxIssueDate = m_pForcastSecurityData->f_issue_date;
		}
	}
	return maxIssueDate;
}
//add by lijw 2008-08-22 下面是有关盈利预测的统计
//个股盈利预测数据（10339）
bool TxStock::StatForcastData(
							  Tx::Core::Table_Indicator &resTable,	//结果数据表
							  std::vector<int> & iInstitutionId,		//机构样本
							  std::vector<int> & iStockId,			//股票样本
							  int iStartDate,							//起始日期
							  int iEndDate,						//结束日期				
							  bool bIsAllDate				//全部日期
							  )
{
	ProgressWnd prw;
	UINT pid=prw.AddItem(1,_T("个股盈利预测数据统计..."),0.0);
	prw.Show(1000);

	//添加列信息
	resTable.AddCol(Tx::Core::dtype_int4);//股票的交易实体0
	resTable.AddCol(Tx::Core::dtype_val_string);//股票名称1
	resTable.AddCol(Tx::Core::dtype_val_string);//股票代码2
	resTable.AddCol(Tx::Core::dtype_val_string);//研究机构名称3
	resTable.AddCol(Tx::Core::dtype_int4);//推荐日期4
	resTable.AddCol(Tx::Core::dtype_int4);//预测年份5
	resTable.AddCol(Tx::Core::dtype_float);//每股收益6
	resTable.AddCol(Tx::Core::dtype_float);//市盈率7
	resTable.AddCol(Tx::Core::dtype_float);//净资产收益率8
	resTable.AddCol(Tx::Core::dtype_double);//主营业务收入9
	resTable.AddCol(Tx::Core::dtype_double);//主营业务利润10
	resTable.AddCol(Tx::Core::dtype_double);//净利润11

	std::unordered_map<int,CString> mapIdToName;
	std::unordered_map<int,CString>::iterator iterMap;
	TypeMapManage::GetInstance()->GetTypeMap(50,mapIdToName);	

	int iRow = -1;
	int iTradeId;
	CString strName;
	CString strCode;
	//判断股票样本的个数
	int icount = (int)iStockId.size();
	if(icount <= 10 || bIsAllDate)
	{		
		//下载按个股拆分的数据文件。
		for(int i = 0;i < icount;i++)
		{
			iTradeId = iStockId[i];
			SecurityQuotation* pSecurity = GetSecurityNow(iTradeId);
			if(NULL == pSecurity)
			{
				return false;//assert(0);
			}
			strName = pSecurity->GetName();
			strCode = pSecurity->GetCode();
			bool Ret = LoadForcastSecurity(iTradeId,_T("profit_forecast_of_security"));
			if (Ret == false)
			{
				continue;
			}
			int icountTemp = m_pForcastSecurity->GetDataCount();
			for(int j = 0;j < icountTemp;j++)
			{				
				m_pForcastSecurityData = m_pForcastSecurity->GetDataByIndex(j);
				//判断是否在所选的机构里
				if(iInstitutionId.end() == find(iInstitutionId.begin(),iInstitutionId.end(),m_pForcastSecurityData->f_provider_id))
				{
					continue;
				}
				//是否在所选日期内
				if(bIsAllDate || (m_pForcastSecurityData->f_issue_date >= iStartDate && m_pForcastSecurityData->f_issue_date <= iEndDate ))
				{
					iRow++;
					resTable.AddRow();
					//添加数据
					resTable.SetCell(0,iRow,iTradeId);
					resTable.SetCell(1,iRow,strName);
					resTable.SetCell(2,iRow,strCode);
					iterMap = mapIdToName.find(m_pForcastSecurityData->f_provider_id);
					if(iterMap != mapIdToName.end())
					{
						resTable.SetCell(3,iRow,iterMap->second);
					}
					resTable.SetCell(4,iRow,m_pForcastSecurityData->f_issue_date);
					resTable.SetCell(5,iRow,m_pForcastSecurityData->f_expect_year);					
					resTable.SetCell(6,iRow,m_pForcastSecurityData->f_eps);
					resTable.SetCell(7,iRow,m_pForcastSecurityData->f_pe);
					resTable.SetCell(8,iRow,m_pForcastSecurityData->f_roe);
					resTable.SetCell(9,iRow,m_pForcastSecurityData->f_main_income);
					resTable.SetCell(10,iRow,m_pForcastSecurityData->f_main_profit);
					resTable.SetCell(11,iRow,m_pForcastSecurityData->f_net_profit);
				}
			}
		}
		//添加进度条
		prw.SetPercent(pid,0.7);
	}
	else
	{
		std::set<int> DateSet;
		this->CombineDate(iStartDate,iEndDate,DateSet);
		std::set<int>::iterator iterSet = DateSet.begin();
		for (;iterSet != DateSet.end();++iterSet)
		{			
			//下载按年份拆分的数据文件。
			bool Ret = LoadForcastYear(*iterSet,_T("profit_forecast_of_year"));			
			if (Ret == false)
			{
				continue;
			}
			int icountTemp = m_pForcastYear->GetDataCount();
			for(int j = 0; j < icountTemp; j++)
			{				
				m_pForcastYearData = m_pForcastYear->GetDataByIndex(j);					

				//判断是否在所选的日期范围内。
				if(m_pForcastYearData->f_issue_date >= iStartDate && m_pForcastYearData->f_issue_date <= iEndDate)
				{
					//判断是否在所选的机构里
					if(iInstitutionId.end() == find(iInstitutionId.begin(),iInstitutionId.end(),m_pForcastYearData->f_provider_id))
					{
							continue;
					}
						//判断是否在所选的股票里
						iTradeId = m_pForcastYearData->f_stock_id;
						if(iStockId.end() == find(iStockId.begin(),iStockId.end(),iTradeId))
					{
						continue;
					}
					iRow++;
					resTable.AddRow();
					//添加数据
					SecurityQuotation* pSecurity = GetSecurityNow(iTradeId);
					if(NULL == pSecurity)
					{
						return false;//assert(0);
					}
					strName = pSecurity->GetName();
					strCode = pSecurity->GetCode();
					resTable.SetCell(0,iRow,iTradeId);
					resTable.SetCell(1,iRow,strName);
					resTable.SetCell(2,iRow,strCode);
					iterMap = mapIdToName.find(m_pForcastYearData->f_provider_id);
					if(iterMap != mapIdToName.end())
					{
						resTable.SetCell(3,iRow,iterMap->second);
					}
					resTable.SetCell(4,iRow,m_pForcastYearData->f_issue_date);
					resTable.SetCell(5,iRow,m_pForcastYearData->f_expect_year);					
					resTable.SetCell(6,iRow,m_pForcastYearData->f_eps);
					resTable.SetCell(7,iRow,m_pForcastYearData->f_pe);
					resTable.SetCell(8,iRow,m_pForcastYearData->f_roe);
					resTable.SetCell(9,iRow,m_pForcastYearData->f_main_income);
					resTable.SetCell(10,iRow,m_pForcastYearData->f_main_profit);
					resTable.SetCell(11,iRow,m_pForcastYearData->f_net_profit);
				}
			}
		}
		//添加进度条
		prw.SetPercent(pid,0.7);
	}
	MultiSortRule multisort;
	multisort.AddRule(0,true);
	multisort.AddRule(5,false);
	resTable.SortInMultiCol(multisort);
	resTable.Arrange();
#ifdef _DEBUG
	CString strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//添加进度条
	prw.SetPercent(pid,1.0);
	return true;
}

//chenyh changed at 2008-12-02
void TxStock::CombineDate(int iStartDate,int iEndDate,std::set<int>& DateSet)
{
	//根据日期拼数据文件名
	int date1 = iStartDate/10000;
	int date2 = iEndDate/10000;	
	
	int date4 = iStartDate/10000;
	int monthDay1 = iStartDate%10000;

	int date3 = 0;
	for(int j = 0;j < 2;j++)
	{
		if(monthDay1 >= 101 && monthDay1 <= 331)
		{
			date3 = date4*10000 + 331;
			DateSet.insert(date3);
		}
		else if(monthDay1 >= 401 && monthDay1 <= 630)
		{
			date3 = date4*10000 + 630;
			DateSet.insert(date3);
		}
		else if(monthDay1 >= 701 && monthDay1 <= 930)
		{
			date3 = date4*10000 + 930;
			DateSet.insert(date3);
		}
		else if(monthDay1 >= 1001 && monthDay1 <= 1231)
		{
			date3 = date4*10000 + 1231;
			DateSet.insert(date3);
		}
		date4 = iEndDate/10000;
		monthDay1 = iEndDate%10000;
	}	

	int itempCount = date2 - date1 - 1;
	for(int i = 0;i < itempCount;i++)
	{
		date1++;
		int tempDate = date1*10000 + 331;
		DateSet.insert(tempDate);
		tempDate = date1*10000 + 630;
		DateSet.insert(tempDate);
		tempDate = date1*10000 + 930;
		DateSet.insert(tempDate);
		tempDate = date1*10000 + 1231;
		DateSet.insert(tempDate);
	}	
}
bool TxStock::LoadEvaluateYear(int id,CString strName)
{	
	//取得数据文件id
	int iFuturesExFileId = 
		Tx::Data::DataStatus::GetInstance()->GetExFileIdFromIni(
		Tx::Core::SystemPath::GetInstance()->GetSystemDataPath(),
		strName);
	if(NULL == m_pEvaluateYear)
		m_pEvaluateYear = new DataFileNormal<blk_TxExFile_FileHead,GradeForecastOfYear>;
	m_pEvaluateYear->SetCheckLoadById(true);
	return m_pEvaluateYear->Load(
								id,//文件名=2020202.dat
								iFuturesExFileId,//文件所在目录
								true);
}
bool TxStock::LoadEvaluateSecurity(int id,CString strName)
{
	//取得数据文件id
	int iFuturesExFileId = 
		Tx::Data::DataStatus::GetInstance()->GetExFileIdFromIni(
		Tx::Core::SystemPath::GetInstance()->GetSystemDataPath(),
		strName);
	if(NULL == m_pEvaluateSecurity)
		m_pEvaluateSecurity = new DataFileNormal<blk_TxExFile_FileHead,GradeForecastOfSecurity>;
	m_pEvaluateSecurity->SetCheckLoadById(true);
	return m_pEvaluateSecurity->Load(
								id,//文件名=2020202.dat
								iFuturesExFileId,//文件所在目录
								true);
}
bool TxStock::LoadForcastYear(int id,CString strName)
{
	//取得数据文件id
	int iFuturesExFileId = 
		Tx::Data::DataStatus::GetInstance()->GetExFileIdFromIni(
		Tx::Core::SystemPath::GetInstance()->GetSystemDataPath(),
		strName);
	if(NULL == m_pForcastYear)
		m_pForcastYear = new DataFileNormal<blk_TxExFile_FileHead,ProfitForecastOfYear>;
	m_pForcastYear->SetCheckLoadById(true);
	return m_pForcastYear->Load(
		id,//文件名=2020202.dat
		iFuturesExFileId,//文件所在目录
		true);
}
bool TxStock::LoadForcastSecurity(int id,CString strName)
{
	//取得数据文件id
	int iFuturesExFileId = 
		Tx::Data::DataStatus::GetInstance()->GetExFileIdFromIni(
		Tx::Core::SystemPath::GetInstance()->GetSystemDataPath(),
		strName);
	if(NULL == m_pForcastSecurity)
		m_pForcastSecurity = new DataFileNormal<blk_TxExFile_FileHead,ProfitForecastOfSecurity>;
	m_pForcastSecurity->SetCheckLoadById(true);
	return m_pForcastSecurity->Load(
		id,//文件名=2020202.dat
		iFuturesExFileId,//文件所在目录
		true);
}
bool TxStock::LoadFinaningSecurityData()
{
	if(m_pIdToFinaning.size() > 0)
		return true;

	//获得融资融券信息
	DataFileNormal<blk_TxExFile_FileHead,FINANCING>* pFinancingData = new DataFileNormal<blk_TxExFile_FileHead,FINANCING>;
	if (NULL != pFinancingData)
	{
		if(pFinancingData->Load(30323, 30007, true)==false || pFinancingData->GetDataCount()<1)
		{
			delete pFinancingData;
			pFinancingData = NULL;
			return false ;
		}
	}
	FINANCING* pFinancing = NULL;
	for (int i = 0; i< pFinancingData->GetDataCount();i++)
	{
		pFinancing = pFinancingData->GetDataByIndex(i);
		if (pFinancing->fFinancingData >=0 && pFinancing->fFinancingData <=1)	//这是个小数存的是百分比数据
		{
			m_pIdToFinaning.insert(std::make_pair<int,float>(pFinancing->iID,pFinancing->fFinancingData*100));
		}

	}

	delete pFinancingData;
	pFinancingData = NULL;

	return true;
}
//2008-11-03
//AH股溢价率
bool TxStock::GetAHData(std::vector<int> iSecurityId,int startDate,int endDate,Table_Display& baTable)
{
	baTable.Clear();

	int iCol = 0;
	//A股交易实体id
	baTable.AddCol(Tx::Core::dtype_int4);
	baTable.SetTitle(iCol,_T("A股交易实体id"));
	iCol++;
	//H股交易实体id
	//baTable.AddCol(Tx::Core::dtype_int4);
	//baTable.SetTitle(iCol,_T("H股交易实体id"));
	//iCol++;

	//A股交易实体名称
	baTable.AddCol(Tx::Core::dtype_val_string);
	baTable.SetTitle(iCol,_T("名称"));
	iCol++;
	//H股交易实体名称
	//baTable.AddCol(Tx::Core::dtype_val_string);
	//baTable.SetTitle(iCol,_T("H股交易实体名称"));
	//iCol++;

	//数据日期
	baTable.AddCol(Tx::Core::dtype_int4);
	baTable.SetTitle(iCol,_T("数据日期"));
	baTable.SetFormatStyle(iCol,Tx::Core::fs_date);
	iCol++;

	//A股交易实体价格
	baTable.AddCol(Tx::Core::dtype_double);
	baTable.SetTitle(iCol,_T("A股价格"));
	baTable.SetPrecise(iCol,2);
	iCol++;
	//H股交易实体价格
	baTable.AddCol(Tx::Core::dtype_double);
	baTable.SetTitle(iCol,_T("H股价格"));
	baTable.SetPrecise(iCol,2);
	iCol++;

	//溢价率
	baTable.AddCol(Tx::Core::dtype_double);
	baTable.SetTitle(iCol,_T("溢价率[(A-H)/H*100]"));
	baTable.SetPrecise(iCol,2);
	iCol++;

	int Count = (int)iSecurityId.size();
	if(Count<=0)
		return false;
	if(startDate>endDate)
		return false;

	//循环样本
	for(int i=0;i<Count;i++)
	{
		int iId = iSecurityId[i];
		SecurityQuotation* p = (SecurityQuotation*)GetSecurityNow(iId);
		if(p==NULL) continue;

		if(p->GetDataFromUrl(dt_AH_line,_T("ah_line"))==false)
			continue;

		int iCount = p->GetDataCount(dt_AH_line);
		if (iCount <= 0)
			continue;

		CString sName;
		CString sCode;
		sName = p->GetName();
		sCode = p->GetCode();
		//get h stock id

		//循环处理符合日期范围要求的数据
		AH_line struAH_line;
		for (int i=0;i<iCount;i++)
		{
			memset(&struAH_line,0,sizeof(AH_line));
			if (!p->GetDataByIndex(dt_AH_line,i,struAH_line))
				continue;

			//检查日期
			if(struAH_line.iDate>=startDate && struAH_line.iDate<=endDate)
			{
				int iRow = (int)baTable.GetRowCount();
				iCol = 0;

				//添加记录
				baTable.AddRow();
				//A股交易实体id
				baTable.SetCell(iCol,iRow,iId);
				iCol++;

				//baTable.SetCell(iCol,iRow,iId);
				//iCol++;

				////A股交易实体名称
				baTable.SetCell(iCol,iRow,sName);
				iCol++;

				//baTable.SetCell(iCol,iRow,sName);
				//iCol++;

				//数据日期
				baTable.SetCell(iCol,iRow,struAH_line.iDate);
				iCol++;

				//A股交易实体价格
				baTable.SetCell(iCol,iRow,struAH_line.dAPrice);
				iCol++;

				//H股交易实体价格
				baTable.SetCell(iCol,iRow,struAH_line.dHPrice);
				iCol++;

				//溢价率
				double dRatio = Con_doubleInvalid;
				if(struAH_line.dHPrice>0)
					dRatio = (struAH_line.dAPrice-struAH_line.dHPrice)/struAH_line.dHPrice*100;
				baTable.SetCell(iCol,iRow,dRatio);
				iCol++;
			}
		}
	}

	return true;
}

// added by zhangxs 2009.11.24
// 板块资金流向统计接口
bool TxStock::GetBlockCashFlow(Tx::Core::Table_Display& resultTable,
							   UINT nStartDate,					// 起始日期
							   UINT nEndDate,					// 终止日期
							   int iResqType,					// 请求类型
							   bool bStat,
							   bool bDurRaise,
							   int iMarketid
					  )

{
	if(nStartDate > nEndDate)
	{
		AfxMessageBox(_T("您所选的日期区间错误,请重新输入!"));
		resultTable.Clear();
		return false;
	}
	resultTable.Clear();
	std::vector<int> vecResSampleId;
	vecResSampleId.clear();
	vecResSampleId.push_back(4000366);//天相流通
	vecResSampleId.push_back(4000367);//沪A
	vecResSampleId.push_back(4000368);//深A
	//vecResSampleId.push_back(4000006);//中小企业板
	vecResSampleId.push_back(4000358);//替换中小企业板4000358
	vecResSampleId.push_back(4001079);//创业板
	vecResSampleId.push_back(4000223);//hs300
#ifdef _DEBUG
	Tx::Core::TxDateTime starttime=Tx::Core::TxDateTime::getCurrentTime();
#endif
	//默认的返回值状态
	bool result = false;
	int iCol = 0;
	int iRow = 0;
	//step1 添加表头
	//交易实体id
	resultTable.AddCol(dtype_int4);
	resultTable.SetTitle(iCol++, _T("交易实体id"));

	//名称
	resultTable.AddCol(dtype_val_string);
	resultTable.SetTitle(iCol++, _T("名称"));

	//代码
	resultTable.AddCol(dtype_int4);
	resultTable.SetTitle(iCol++, _T("样本只数"));

	//涨幅
	resultTable.AddCol(dtype_float);
	resultTable.SetTitle(iCol++, _T("板块涨幅(%)"));

	//流入/流出
	resultTable.AddCol(dtype_double);
	resultTable.SetPrecise(iCol, 3);
	resultTable.SetTitle(iCol++, _T("流入/流出"));

	//净流入
	resultTable.AddCol(dtype_double);
	resultTable.SetOutputItemRatio(iCol, 10000);
	resultTable.SetFormatStyle(iCol, Tx::Core::fs_finance);
	resultTable.SetTitle(iCol++, _T("净流入(万元)"));

	//流入
	resultTable.AddCol(dtype_double);
	resultTable.SetOutputItemRatio(iCol, 10000);
	resultTable.SetFormatStyle(iCol, Tx::Core::fs_finance);
	resultTable.SetTitle(iCol++, _T("流入(万元)"));

	//流出
	resultTable.AddCol(dtype_double);
	resultTable.SetOutputItemRatio(iCol, 10000);
	resultTable.SetFormatStyle(iCol, Tx::Core::fs_finance);
	resultTable.SetTitle(iCol++, _T("流出(万元)"));

	int iSamCount = 0;
	int iSecId = 0;
	float fRatio = 0.0;
	double dInSum = 0.0;
	double dOutSum = 0.0;
	//step2 资金流向统计
	Tx::Core::ProgressWnd prw;
	CString sProgressPrompt;
	sProgressPrompt =  _T("资金流向数据请求...");
	UINT progId = prw.AddItem(1,sProgressPrompt, 0.0);
	prw.Show(15);
	resultTable.AddRow(6);
	int m_iHead = sizeof(byte) + sizeof(int)*2; //byte isFlag  //0:正常 1：错误 ,int size //大小 ,int colSize  //列大小
	if( 0 == iResqType)	//样本为板块Id
	{
		// 获得资金流向
		// 传递二进制
		// int -- 功能类型 1--板块 0--个股
		// int -- 交易实体个数
		// n*int -- 交易实体序列
		// int -- 起始日期
		// int -- 截止日期
		//交易实体Id
		int iParamSize = 1 + 1 + 1;//功能类型 1,起始日期,截止日期
		int* pBufferParam = NULL;
		pBufferParam = new int [iParamSize];
		if (pBufferParam == NULL)
			return false;

		int* pBuffer = pBufferParam;
		memset(pBuffer,0,iParamSize * sizeof(int));

		// 功能类型 1--板块资金流向
		*pBuffer = iResqType;
		pBuffer++;

		// 起始日期
		*pBuffer = nStartDate;
		pBuffer++;

		// 截止日期
		*pBuffer = nEndDate;
		pBuffer++;
		//LPCTSTR lpUrl = _T("http://192.168.5.77/CashFlow/CashFlow.aspx");
		LPCTSTR lpUrl = Tx::Core::SystemInfo::GetInstance()->GetServerAddr(_T("File"), _T("BlockFundStream"));

		Tx::Drive::Http::CSyncUpload upload;
		int iStart = ::GetTickCount();
		if ( upload.Post(lpUrl, (LPBYTE)pBufferParam, iParamSize * sizeof(int)) )
		{
			int iEnd = ::GetTickCount();
			TRACE(_T("\r\nURL Cost Time %d(ms)\r\n"),iEnd-iStart);
			CONST Tx::Drive::Mem::MemSlice &data = upload.Rsp().Body();
			LPBYTE lpRes = data.DataPtr();
			UINT nRetSize = data.Size();
			if (nRetSize <= 0)
			{
				prw.SetPercent(progId,0.5);
				AfxMessageBox(_T("数据下载失败！"));
				delete pBufferParam;
				pBufferParam = NULL;
				return false;
			}

			UINT nPreSize = *(reinterpret_cast<UINT*>(lpRes));
			UINT m_iSize = sizeof(int)+sizeof(int)+sizeof(float)+sizeof(double)+sizeof(double); //int 板块Id,int 样本个数,double 阶段流入,double 阶段流出
			UINT m_iCount = (nPreSize - m_iHead )/m_iSize;
			if((nPreSize - m_iHead )%m_iSize != 0 || m_iCount > 3000)
			{
				CString m_strTip;
				m_strTip.Format(_T("下载数据格式不对，文件大小为：[%d - %d]；每条大小为：%d"),nPreSize,m_iHead,m_iSize); 
				AfxMessageBox(m_strTip);
				delete []pBufferParam;
				pBufferParam = NULL;
				return false;
			}
			if(m_iCount <1)
			{
				AfxMessageBox(_T("未获取到数据，请检查参数"));
				delete []pBufferParam;
				pBufferParam = NULL;

				return false;
			}
			LPBYTE lpData = new BYTE[nPreSize];
			if ( lpData == NULL )
			{
				delete pBufferParam;
				pBufferParam = NULL;
				return false;
			}
			if ( !Tx::Drive::IO::Zip::CZipWrapper::MemUnZip(lpData, 
				nPreSize, lpRes + sizeof(UINT), nRetSize - sizeof(UINT)) )
			{
				delete []lpData;
				lpData = NULL;
				delete []pBufferParam;
				pBufferParam = NULL;
				return false;
			}
			LPBYTE pRecv = lpData;
			//UINT m_iSize = sizeof(int)+sizeof(int)+sizeof(double)+sizeof(double); //int 板块Id,int 样本个数,double 阶段流入,double 阶段流出
			//UINT m_iCount = (nPreSize - 8 )/m_iSize;
			//if((nPreSize - 8 )%m_iSize != 0)
			//{
			//	AfxMessageBox(_T("下载数据格式不对！"));
			//	delete []lpData;
			//	lpData = NULL;
			//	delete []pBufferParam;
			//	pBufferParam = NULL;
			//	return false;
			//}
			BYTE isFlag = 1;
			memcpy_s(&isFlag,sizeof(BYTE),pRecv,sizeof(BYTE));
			pRecv += sizeof(BYTE);

			double dValue = 0.0;
			pRecv += sizeof(int)*2;
			
			for (UINT i=0;i < m_iCount; i++)
			{
				bool bIsFixedIndex = false;
				//板块Id
				memcpy_s(&iSecId,sizeof(int),pRecv,sizeof(int));
				pRecv += sizeof(int);
				//样本支数
				memcpy_s(&iSamCount,sizeof(int),pRecv,sizeof(int));
				pRecv += sizeof(int);
				//阶段涨幅
				memcpy_s(&fRatio,sizeof(float),pRecv,sizeof(float));
				pRecv += sizeof(float);
				//累积流入
				memcpy_s(&dInSum,sizeof(double),pRecv,sizeof(double));
				pRecv += sizeof(double);
				//累积流出
				memcpy_s(&dOutSum,sizeof(double),pRecv,sizeof(double));
				pRecv += sizeof(double);
				
				SecurityQuotation* pSec = (SecurityQuotation*)GetSecurityNow(iSecId);
				if(pSec==NULL) continue;

				if( 4000366== iSecId || 4000367== iSecId || 4000368 == iSecId
					|| 4000358== iSecId|| 4001079 == iSecId|| 4000223== iSecId )
					bIsFixedIndex = true;
				if(!bIsFixedIndex)
				{
					if(4000358 == iSecId || 4000006 == iSecId)
						continue;
					vecResSampleId.push_back(iSecId);
					iRow = (int)resultTable.GetRowCount();
					//添加一行记录
					resultTable.AddRow();

					//板块Id
					resultTable.SetCell(0,iRow,iSecId);
					//板块名称
					CString m_str = _T("");
					if( 4000366== iSecId)
						m_str = _T("全部A股");
					else if( 4000367== iSecId)
						m_str = _T("沪A");
					else if( 4000368== iSecId)
						m_str = _T("深A");
					else if(4001079 == iSecId)
						m_str = _T("创业板");
					else if( 4000358 == iSecId)
						m_str = _T("中小板");
					else if(4000223 == iSecId)
						m_str = _T("沪深300");
					else
						m_str = pSec->GetName();
					resultTable.SetCell(1,iRow,m_str);
					//样本支数
					resultTable.SetCell(2,iRow,iSamCount);
					//阶段涨幅
					resultTable.SetCell(3,iRow,fRatio);
					//流入/流出
					if (fabs(dOutSum - Tx::Core::Con_doubleInvalid) < 0.1 
						|| fabs(dInSum - Tx::Core::Con_doubleInvalid) < 0.1
						|| fabs(dOutSum) - 0.1 < 0.1)
						resultTable.SetCell(4,iRow,Tx::Core::Con_doubleInvalid);
					else
						resultTable.SetCell(4,iRow,dInSum /dOutSum);
					//净流入
					if (fabs(dOutSum - Tx::Core::Con_doubleInvalid) < 0.1 || fabs(dInSum - Tx::Core::Con_doubleInvalid) < 0.1)
						resultTable.SetCell(5,iRow,Tx::Core::Con_doubleInvalid);
					else
						resultTable.SetCell(5,iRow,dInSum - dOutSum);
					//累积流入
					if (fabs(dInSum - Tx::Core::Con_doubleInvalid) < 0.1)
						resultTable.SetCell(6,iRow,Tx::Core::Con_doubleInvalid);
					else
						resultTable.SetCell(6,iRow,dInSum);
					//累积流出
					if (fabs(dOutSum - Tx::Core::Con_doubleInvalid) < 0.1)
						resultTable.SetCell(7,iRow,Tx::Core::Con_doubleInvalid);
					else
						resultTable.SetCell(7,iRow,dOutSum);
				}
				else
				{
					int m_iRow = 0;
					if( 4000367 == iSecId)
						m_iRow = 1;
					else if(4000368 == iSecId)
						m_iRow = 2;
					else if(4000358 == iSecId)
						m_iRow = 3;
					else if(4001079 == iSecId)
						m_iRow = 4;
					else if(4000223 == iSecId)
						m_iRow = 5;
					//板块Id
					resultTable.SetCell(0,m_iRow,iSecId);
					//板块名称
					CString m_str = _T("");
					if( 4000366== iSecId)
						m_str = _T("全部A股");
					else if( 4000367== iSecId)
						m_str = _T("沪A");
					else if( 4000368== iSecId)
						m_str = _T("深A");
					else if(4001079 == iSecId)
						m_str = _T("创业板");
					else if( 4000358 == iSecId)
						m_str = _T("中小板");
					else if(4000223 == iSecId)
						m_str = _T("沪深300");
					else
						m_str = pSec->GetName();
					resultTable.SetCell(1,m_iRow,m_str);
					//样本支数
					resultTable.SetCell(2,m_iRow,iSamCount);
					//阶段涨幅
					resultTable.SetCell(3,m_iRow,fRatio);
					//流入/流出
					if (fabs(dOutSum - Tx::Core::Con_doubleInvalid) < 0.1 
						|| fabs(dInSum - Tx::Core::Con_doubleInvalid) < 0.1
						|| fabs(dOutSum) - 0.1 < 0.1)
						resultTable.SetCell(4,m_iRow,Tx::Core::Con_doubleInvalid);
					else
						resultTable.SetCell(4,m_iRow,dInSum /dOutSum);
					//净流入
					if (fabs(dOutSum - Tx::Core::Con_doubleInvalid) < 0.1 || fabs(dInSum - Tx::Core::Con_doubleInvalid) < 0.1)
						resultTable.SetCell(5,m_iRow,Tx::Core::Con_doubleInvalid);
					else
						resultTable.SetCell(5,m_iRow,dInSum - dOutSum);
					//累积流入
					if (fabs(dInSum - Tx::Core::Con_doubleInvalid) < 0.1)
						resultTable.SetCell(6,m_iRow,Tx::Core::Con_doubleInvalid);
					else
						resultTable.SetCell(6,m_iRow,dInSum);
					//累积流出
					if (fabs(dOutSum - Tx::Core::Con_doubleInvalid) < 0.1)
						resultTable.SetCell(7,m_iRow,Tx::Core::Con_doubleInvalid);
					else
						resultTable.SetCell(7,m_iRow,dOutSum);
				}
			}
			delete []lpData;
			lpData = NULL;
			iEnd = ::GetTickCount();
			TRACE(_T("\r\nParse Result Cost Time %d(ms)\r\n"),iEnd-iStart);
		}
		delete []pBufferParam;
		pBufferParam = NULL;
		prw.SetPercent(progId,0.6);
	}
	else
	{
		prw.SetPercent(progId,0.3);
		AfxMessageBox(_T("数据下载失败！"));		
		return false;
	}
#ifdef _DEBUG
	CString strTable=resultTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif

//	//step3 添加阶段涨幅
//	int m_iStart = ::GetTickCount();
//	if((int)vecResSampleId.size() < 1)
//	{
//		AfxMessageBox(_T("提取样本为空！"));
//		return false;
//	}
//	Tx::Core::Table_Indicator durRaiseTable;
//	if(bDurRaise == true)
//	{
//		if(BlockCycleRateAdv(
//			vecResSampleId,	//交易实体ID
//			nStartDate,		//起始日期
//			nEndDate,		//终止日期
//			true,			//剔除首日涨幅
//			//0,				//复权类型 0:不复权;1:前复权;2:后复权;3:全复权+后复权;4:全复权+前复权
//			2,//2009-05-11 徐勉：资金流向需要复权，后复权
//			durRaiseTable,	//结果数据表
//			2,
//			0,				//计算类型0-默认；1-债券[bCutFirstDateRaise=true表示全价模式,bCutFirstDateRaise=false表示净价模式]
//			iMarketid,		//交易所ID
//			true,			//true只计算涨幅,false计算所有
//			false
//			)==false)
//		{
//			return false;
//		}
//		durRaiseTable.MakeReference(0);
//	}
//#ifdef _DEBUG
//	strTable=durRaiseTable.TableToString();
//	Tx::Core::Commonality::String().StringToClipboard(strTable);
//#endif
//	prw.SetPercent(progId,0.9);
//	//step4 拼接资金流向表和阶段涨幅表
//	for(int ix = 0;ix<(int)durRaiseTable.GetRowCount();ix++)
//	{
//		int secIdRaise = 0;
//		double m_dRate = Con_doubleInvalid;
//		durRaiseTable.GetCell(0,ix,secIdRaise);
//		durRaiseTable.GetCell(3,ix,m_dRate);
//		for(int jx = 0;jx < (int)resultTable.GetRowCount();jx++)
//		{
//			int tempId = 0;
//			resultTable.GetCell(0,jx,tempId);
//			if(tempId > 0 && tempId == secIdRaise)
//				resultTable.SetCell(3,jx,m_dRate);
//		}		
//	}
//	int m_iEnd = ::GetTickCount();
//	TRACE(_T("\r\nCycleRate cost time: %d\r\n"),m_iEnd- m_iStart);
	//全部A股没有阶段涨幅
	//resultTable.SetCell(3,0,Con_doubleInvalid);
	prw.SetPercent(progId,1.0);
#ifdef _DEBUG
	strTable=resultTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif

	return true;
}
// added by zhangxs 2009.11.24
// 个股资金流向统计接口
bool TxStock::GetSampleCashFlow(Tx::Core::Table_Display& resultTable,
								std::vector<int> vecBlockId,		// 板块ID
								UINT nStartDate,					// 起始日期
								UINT nEndDate,						// 终止日期
								int iResqType,						// 请求类型
								bool bAddSamplesOnly,			//添加样本不统计
								bool bStat,
								bool bDurRaise,
								int iMarketid,
								bool bFocusSamples
							   )
{
	if(nStartDate > nEndDate)
	{
		AfxMessageBox(_T("您所选的日期区间错误,请重新输入!"));
		resultTable.Clear();
		return false;
	}
	if (vecBlockId.empty())
		return false;
	resultTable.Clear();
	std::vector<int> vecResSampleId;
	vecResSampleId.clear();
#ifdef _DEBUG
	Tx::Core::TxDateTime starttime=Tx::Core::TxDateTime::getCurrentTime();
#endif
	//默认的返回值状态
	bool result = false;
	int iCol = 0;
	int iRow = 0;
	//step1 添加表头
	//交易实体id
	resultTable.AddCol(dtype_int4);
	resultTable.SetTitle(iCol++, _T("交易实体id"));

	//名称
	resultTable.AddCol(dtype_val_string);
	resultTable.SetTitle(iCol++, _T("名称"));

	//代码
	resultTable.AddCol(dtype_val_string);
	resultTable.SetTitle(iCol++, _T("代码"));

	//涨幅
	resultTable.AddCol(dtype_float);
	resultTable.SetTitle(iCol++, _T("个股涨幅(%)"));

	//流入/流出
	resultTable.AddCol(dtype_double);
	resultTable.SetPrecise(iCol, 3);
	resultTable.SetTitle(iCol++, _T("流入/流出"));

	//净流入
	resultTable.AddCol(dtype_double);
	resultTable.SetOutputItemRatio(iCol, 10000);
	resultTable.SetFormatStyle(iCol, Tx::Core::fs_finance);
	resultTable.SetTitle(iCol++, _T("净流入(万元)"));

	//流入
	resultTable.AddCol(dtype_double);
	resultTable.SetOutputItemRatio(iCol, 10000);
	resultTable.SetFormatStyle(iCol, Tx::Core::fs_finance);
	resultTable.SetTitle(iCol++, _T("流入(万元)"));

	//流出
	resultTable.AddCol(dtype_double);
	resultTable.SetOutputItemRatio(iCol, 10000);
	resultTable.SetFormatStyle(iCol, Tx::Core::fs_finance);
	resultTable.SetTitle(iCol++, _T("流出(万元)"));
	//只是添加样本
	if(bAddSamplesOnly)
	{
		int iSecurityCount = (int)vecBlockId.size();
		//循环样本
		for(int i=0;i<iSecurityCount;i++)
		{
			//样本id
			int iSecurityId = vecBlockId[i];
			//样本对象
			SecurityQuotation* pSecurityQuotation = GetSecurityNow(iSecurityId);
			if(pSecurityQuotation==NULL)
				continue;
			if(pSecurityQuotation->IsHK_Market()==true)
				continue;

			iRow = (int)resultTable.GetRowCount();
			iCol = 0;
			//添加一行记录
			resultTable.AddRow();
			//securityid
			resultTable.SetCell(iCol++,iRow,iSecurityId);
			//name
			resultTable.SetCell(iCol++,iRow,pSecurityQuotation->GetName());
			//code
			resultTable.SetCell(iCol++,iRow,pSecurityQuotation->GetCode());
		}
		return true;
	}
	int iSamCount = 0;
	int iSecId = 0;
	float fRatio = 0.0;
	double dInSum = 0.0;
	double dOutSum = 0.0;
	//step2 资金流向统计
	Tx::Core::Table calcuTable;
	Tx::Core::ProgressWnd prw;
	CString sProgressPrompt;
	sProgressPrompt =  _T("资金流向数据请求...");
	UINT progId = prw.AddItem(1,sProgressPrompt, 0.0);
	prw.Show(15);
	std::vector<int> m_vecRes;	//判断个股请求，有样本没有数据
	m_vecRes.clear();
	m_vecRes.assign(vecBlockId.begin(),vecBlockId.end());
	if(1 == iResqType)	//个股资金流向
	{
		// 获得资金流向
		// 传递二进制
		// int -- 功能类型 0--板块 1--板块下个股 2--样本集
		// int -- 起始日期
		// int -- 截止日期
		// int -- 交易实体个数
		// n*int -- 交易实体序列	
		//交易实体Id
		int iParamSize = 1 + 1 + 1 + vecBlockId.size(); //功能类型,起始日期,截止日期,板块指数交易实体id
		int* pBufferParam = NULL;
		pBufferParam = new int [iParamSize];
		if (pBufferParam == NULL)
			return false;
		int* pBuffer = pBufferParam;
		memset(pBuffer,0,iParamSize * sizeof(int));
		// 功能类型
		*pBuffer = iResqType;
		pBuffer++;
		// 起始日期
		*pBuffer = nStartDate;
		pBuffer++;
		// 截止日期
		*pBuffer = nEndDate;
		pBuffer++;
		// 板块指数交易实体id
		for (std::vector<int>::iterator iter = vecBlockId.begin(); iter!=vecBlockId.end();iter++)
		{
			*pBuffer = *iter;
			pBuffer++;
		}
		//LPCTSTR lpUrl = _T("http://192.168.5.77/CashFlow/CashFlow.aspx");
		LPCTSTR lpUrl = Tx::Core::SystemInfo::GetInstance()->GetServerAddr(_T("File"), _T("BlockFundStream"));

		Tx::Drive::Http::CSyncUpload upload;
		int iStart = ::GetTickCount();
		int m_iHead = sizeof(int)*2 +1;	//byte,int,int
		if ( upload.Post(lpUrl, (LPBYTE)pBufferParam, iParamSize * sizeof(int)) )
		{
			int iEnd = ::GetTickCount();
			TRACE(_T("\r\nURL Cost Time %d(ms)\r\n"),iEnd-iStart);
			CONST Tx::Drive::Mem::MemSlice &data = upload.Rsp().Body();
			LPBYTE lpRes = data.DataPtr();
			UINT nRetSize = data.Size();
			if (nRetSize <= 0)
			{
				prw.SetPercent(progId,0.5);
				AfxMessageBox(_T("数据下载失败！"));
				delete []pBufferParam;
				pBufferParam = NULL;

				return false;
			}

			UINT nPreSize = *(reinterpret_cast<UINT*>(lpRes));
			UINT m_iSize = sizeof(int)+sizeof(float)+sizeof(double)+sizeof(double);//int 样本Id;double 阶段流入;double 阶段流出
			UINT m_iCount = (nPreSize - m_iHead )/m_iSize;
			if((nPreSize - m_iHead )%m_iSize != 0 || m_iCount > 3000)
			{
				CString m_strTip;
				m_strTip.Format(_T("下载数据格式不对，文件大小为：[%d - %d]；每条大小为：%d"),nPreSize,m_iHead,m_iSize); 
				AfxMessageBox(m_strTip);
				delete []pBufferParam;
				pBufferParam = NULL;

				return false;
			}
			if(m_iCount <1)
			{
				AfxMessageBox(_T("未获取到数据，请检查参数"));
				delete []pBufferParam;
				pBufferParam = NULL;

				return false;
			}
			LPBYTE lpData = new BYTE[nPreSize];
			if ( lpData == NULL )
			{
				delete []pBufferParam;
				pBufferParam = NULL;
				return false;
			}
			if ( !Tx::Drive::IO::Zip::CZipWrapper::MemUnZip(lpData, 
				nPreSize, lpRes + sizeof(UINT), nRetSize - sizeof(UINT)) )
			{
				delete []lpData;
				lpData = NULL;
				delete []pBufferParam;
				pBufferParam = NULL;

				return false;
			}
			LPBYTE pRecv = lpData;
			int iStart = ::GetTickCount();
			//UINT m_iSize = sizeof(int)+sizeof(double)+sizeof(double);//int 样本Id;double 阶段流入;double 阶段流出
			//UINT m_iCount = (nPreSize - 8 )/m_iSize;
			//if((nPreSize - 8 )%m_iSize != 0)
			//{
			//	AfxMessageBox(_T("下载数据格式不对！"));
			//	delete []lpData;
			//	lpData = NULL;
			//	delete []pBufferParam;
			//	pBufferParam = NULL;
			//	return false;
			//}
			double dValue = 0.0;
			BYTE isFlag = 1;
			memcpy_s(&isFlag,sizeof(BYTE),pRecv,sizeof(BYTE));
			pRecv += sizeof(BYTE);
			if(isFlag == 1)
			{
				delete []lpData;
				lpData = NULL;
				delete []pBufferParam;
				pBufferParam = NULL;

				return false;
			}
			pRecv += sizeof(int)*2;
			for (UINT i=0;i < m_iCount; i++)
			{
				//板块Id
				memcpy_s(&iSecId,sizeof(int),pRecv,sizeof(int));
				pRecv += sizeof(int);
				//阶段涨幅
				memcpy_s(&fRatio,sizeof(float),pRecv,sizeof(float));
				pRecv += sizeof(float);
				//累积流入
				memcpy_s(&dInSum,sizeof(double),pRecv,sizeof(double));
				pRecv += sizeof(double);
				//累积流出
				memcpy_s(&dOutSum,sizeof(double),pRecv,sizeof(double));
				pRecv += sizeof(double);

				SecurityQuotation* pSec = (SecurityQuotation*)GetSecurityNow(iSecId);
				if(pSec==NULL) continue;
				vecResSampleId.push_back(iSecId);

				iRow = (int)resultTable.GetRowCount();
				//添加一行记录
				resultTable.AddRow();

				resultTable.SetCell(0,iRow,iSecId);
				resultTable.SetCell(1,iRow,pSec->GetName());
				resultTable.SetCell(2,iRow,pSec->GetCode());
				resultTable.SetCell(3,iRow,fRatio);
				//流入/流出
				if (fabs(dOutSum - Tx::Core::Con_doubleInvalid) < 0.1 
					|| fabs(dInSum - Tx::Core::Con_doubleInvalid) < 0.1
					|| fabs(dOutSum) - 0.1 < 0.1)
					resultTable.SetCell(4,iRow,Tx::Core::Con_doubleInvalid);
				else
					resultTable.SetCell(4,iRow,dInSum /dOutSum);
				//净流入
				if (fabs(dOutSum - Tx::Core::Con_doubleInvalid) < 0.1 
					|| fabs(dInSum - Tx::Core::Con_doubleInvalid) < 0.1)
					resultTable.SetCell(5,iRow,Tx::Core::Con_doubleInvalid);
				else
					resultTable.SetCell(5,iRow,dInSum - dOutSum);
				//累积流入
				if (fabs(dInSum - Tx::Core::Con_doubleInvalid) < 0.1)
					resultTable.SetCell(6,iRow,Tx::Core::Con_doubleInvalid);
				else
					resultTable.SetCell(6,iRow,dInSum);
				//累积流出
				if (fabs(dOutSum - Tx::Core::Con_doubleInvalid) < 0.1)
					resultTable.SetCell(7,iRow,Tx::Core::Con_doubleInvalid);
				else
					resultTable.SetCell(7,iRow,dOutSum);
			}
			delete []lpData;
			lpData = NULL;
			if(m_iCount != (int)(vecResSampleId.size()))
				AfxMessageBox(_T("本地码表和服务器码表不一致，样本统计有出入！"));
			iEnd = ::GetTickCount();
			TRACE(_T("\r\nParse Result Cost Time %d(ms)\r\n"),iEnd-iStart);
			if((int)vecResSampleId.size() != (int)vecBlockId.size())
			{

			}
			prw.SetPercent(progId,0.6);
		}	
		delete []pBufferParam;
		pBufferParam = NULL;
	}
	else if(2 == iResqType)
	{
		// 获得资金流向
		// 传递二进制
		// int -- 功能类型 0--板块 1--板块下个股 2--样本集
		// int -- 起始日期
		// int -- 截止日期
		// int -- 交易实体个数
		// n*int -- 交易实体序列
		//交易实体Id
		int iParamSize = 1 + 1 + 1 + vecBlockId.size();	//功能类型,起始日期,截止日期,交易实体id
		int* pBufferParam = NULL;
		pBufferParam = new int [iParamSize];
		if (pBufferParam == NULL)
			return false;

		int* pBuffer = pBufferParam;
		memset(pBuffer,0,iParamSize * sizeof(int));

		// 功能类型 
		*pBuffer = iResqType;
		pBuffer++;
		// 起始日期
		*pBuffer = nStartDate;
		pBuffer++;
		// 截止日期
		*pBuffer = nEndDate;
		pBuffer++;
		// 交易实体id
		for (std::vector<int>::iterator iter = vecBlockId.begin(); iter!=vecBlockId.end();iter++)
		{
			*pBuffer = *iter;
			pBuffer++;
		}		
		//LPCTSTR lpUrl = _T("http://192.168.5.77/CashFlow/CashFlow.aspx");
		LPCTSTR lpUrl = Tx::Core::SystemInfo::GetInstance()->GetServerAddr(_T("File"), _T("BlockFundStream"));

		Tx::Drive::Http::CSyncUpload upload;
		int m_iHead = sizeof(int)*2 +1;	//byte,int,int
		int iStart = ::GetTickCount();
		if ( upload.Post(lpUrl, (LPBYTE)pBufferParam, iParamSize * sizeof(int)) )
		{
			int iEnd = ::GetTickCount();
			TRACE(_T("\r\nURL Cost Time %d(ms)\r\n"),iEnd-iStart);
			CONST Tx::Drive::Mem::MemSlice &data = upload.Rsp().Body();
			LPBYTE lpRes = data.DataPtr();
			UINT nRetSize = data.Size();
			if (nRetSize <= 0)
			{
				prw.SetPercent(progId,0.5);
				AfxMessageBox(_T("数据下载失败！"));
				delete []pBufferParam;
				pBufferParam = NULL;

				return false;
			}
			UINT nPreSize = *(reinterpret_cast<UINT*>(lpRes));
			UINT m_iSize = sizeof(int) + sizeof(float)+sizeof(double) + sizeof(double);	//int 样本Id;double 阶段流入;double 阶段流出
			UINT m_iCount = (nPreSize - m_iHead )/m_iSize;
			if((nPreSize - m_iHead )%m_iSize != 0 || m_iCount > 3000)
			{
				CString m_strTip;
				m_strTip.Format(_T("下载数据格式不对，文件大小为：[%d - %d]；每条大小为：%d"),nPreSize,m_iHead,m_iSize); 
				AfxMessageBox(m_strTip);
				delete []pBufferParam;
				pBufferParam = NULL;
				return false;
			}
			/*if(m_iCount <1)
			{
				AfxMessageBox(_T("未获取到数据，请检查参数"));
				delete []pBufferParam;
				pBufferParam = NULL;

				return false;
			}*/
			LPBYTE lpData = new BYTE[nPreSize];
			if ( lpData == NULL )
			{
				delete []pBufferParam;
				pBufferParam = NULL;
				return false;
			}
			if ( !Tx::Drive::IO::Zip::CZipWrapper::MemUnZip(lpData, 
				nPreSize, lpRes + sizeof(UINT), nRetSize - sizeof(UINT)) )
			{
				delete []lpData;
				lpData = NULL;
				delete []pBufferParam;
				pBufferParam = NULL;

				return false;
			}
			LPBYTE pRecv = lpData;
			int iStart = ::GetTickCount();
			BYTE isFlag = 1;
			memcpy_s(&isFlag,sizeof(BYTE),pRecv,sizeof(BYTE));
			pRecv += sizeof(BYTE);
			if(isFlag == 1)
			{
				AfxMessageBox(_T("下载数据无效，请稍后再试！"));
				delete []lpData;
				lpData = NULL;
				delete []pBufferParam;
				pBufferParam = NULL;

				return false;
			}
			pRecv += sizeof(int)*2;
			for (UINT i=0;i < m_iCount; i++)
			{
				//交易实体Id
				memcpy_s(&iSecId,sizeof(int),pRecv,sizeof(int));
				pRecv += sizeof(int);
				//阶段涨幅
				memcpy_s(&fRatio,sizeof(float),pRecv,sizeof(float));
				pRecv += sizeof(float);
				//累积流入
				memcpy_s(&dInSum,sizeof(double),pRecv,sizeof(double));
				pRecv += sizeof(double);				
				//累积流出
				memcpy_s(&dOutSum,sizeof(double),pRecv,sizeof(double));
				pRecv += sizeof(double);

				SecurityQuotation* pSec = (SecurityQuotation*)GetSecurityNow(iSecId);
				if(pSec==NULL) continue;
				vecResSampleId.push_back(iSecId);
				for(std::vector<int>::iterator m_iter = m_vecRes.begin(); m_iter!=m_vecRes.end();m_iter++)
				{
					if(*m_iter == iSecId)
					{
						m_vecRes.erase(m_iter);
						break;
					}					
				}
				iRow = (int)resultTable.GetRowCount();
				//添加一行记录
				resultTable.AddRow();

				//交易实体Id
				resultTable.SetCell(0,iRow,iSecId);
				//交易实体名称
				resultTable.SetCell(1,iRow,pSec->GetName());
				resultTable.SetCell(2,iRow,pSec->GetCode());
				//阶段涨幅
				resultTable.SetCell(3,iRow,fRatio);
				//流入/流出
				if (fabs(dOutSum - Tx::Core::Con_doubleInvalid) < 0.1 
					|| fabs(dInSum - Tx::Core::Con_doubleInvalid) < 0.1
					|| fabs(dOutSum) - 0.1 < 0.1)
					resultTable.SetCell(4,iRow,Tx::Core::Con_doubleInvalid);
				else
					resultTable.SetCell(4,iRow,dInSum /dOutSum);
				//净流入
				if (fabs(dOutSum - Tx::Core::Con_doubleInvalid) < 0.1 || fabs(dInSum - Tx::Core::Con_doubleInvalid) < 0.1)
					resultTable.SetCell(5,iRow,Tx::Core::Con_doubleInvalid);
				else
					resultTable.SetCell(5,iRow,dInSum - dOutSum);
				//累积流入
				if (fabs(dInSum - Tx::Core::Con_doubleInvalid) < 0.1)
					resultTable.SetCell(6,iRow,Tx::Core::Con_doubleInvalid);
				else
					resultTable.SetCell(6,iRow,dInSum);
				//累积流出
				if (fabs(dOutSum - Tx::Core::Con_doubleInvalid) < 0.1)
					resultTable.SetCell(7,iRow,Tx::Core::Con_doubleInvalid);
				else
					resultTable.SetCell(7,iRow,dOutSum);
				iRow++;
			
			}
			delete	[]lpData;
			lpData = NULL;
			if(m_iCount != (int)(vecResSampleId.size()))
				AfxMessageBox(_T("本地码表和服务器码表不一致，样本统计有出入！"));

			for(std::vector<int>::iterator m_iter = m_vecRes.begin(); m_iter!=m_vecRes.end();m_iter++)
			{
				iRow = (int)resultTable.GetRowCount();
				resultTable.InsertRow(iRow);
				iSecId = *m_iter;
				SecurityQuotation* pSec = (SecurityQuotation*)GetSecurityNow(iSecId);
				if(pSec==NULL) continue;
				//交易实体Id
				resultTable.SetCell(0,iRow,iSecId);
				//交易实体名称
				resultTable.SetCell(1,iRow,pSec->GetName());
				resultTable.SetCell(2,iRow,pSec->GetCode());
				//阶段涨幅
				resultTable.SetCell(3,iRow,Tx::Core::Con_floatInvalid);
				//流入/流出
				resultTable.SetCell(4,iRow,Tx::Core::Con_doubleInvalid);
				//净流入
				resultTable.SetCell(5,iRow,Tx::Core::Con_doubleInvalid);				
				//累积流入
					resultTable.SetCell(6,iRow,Tx::Core::Con_doubleInvalid);
				//累积流出
				resultTable.SetCell(7,iRow,Tx::Core::Con_doubleInvalid);
			}
			iEnd = ::GetTickCount();
			TRACE(_T("\r\nParse Result Cost Time %d(ms)\r\n"),iEnd-iStart);
		}
		delete []pBufferParam;
		pBufferParam = NULL;
		prw.SetPercent(progId,0.6);
	}
	else
	{
		prw.SetPercent(progId,0.3);
		AfxMessageBox(_T("资金流向数据下载失败！"));
		return false;
	}

	/*if((int)vecResSampleId.size() < 1)
	{
		AfxMessageBox(_T("提取的样本为空！"));
		return false;
	}*/

#ifdef _DEBUG
	CString strTable=resultTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
//	int m_iStart = ::GetTickCount();
//	//step3 添加阶段涨幅
//	Tx::Core::Table_Indicator durRaiseTable;
//	if(bDurRaise == true)
//	{
//		if(BlockCycleRateAdv(
//			vecResSampleId,	//交易实体ID
//			nStartDate,		//起始日期
//			nEndDate,		//终止日期
//			true,			//剔除首日涨幅
//			//0,				//复权类型 0:不复权;1:前复权;2:后复权;3:全复权+后复权;4:全复权+前复权
//			2,//2009-05-11 徐勉：资金流向需要复权，后复权
//			durRaiseTable,	//结果数据表
//			2,
//			0,				//计算类型0-默认；1-债券[bCutFirstDateRaise=true表示全价模式,bCutFirstDateRaise=false表示净价模式]
//			iMarketid,		//交易所ID
//			true,			//true只计算涨幅,false计算所有
//			false
//			)==false)
//		{
//			return false;
//		}
//		durRaiseTable.MakeReference(0);
//	}
//#ifdef _DEBUG
//	strTable=durRaiseTable.TableToString();
//	Tx::Core::Commonality::String().StringToClipboard(strTable);
//#endif
//	//step4 拼接资金流向表和阶段涨幅表
//	for(int ix = 0;ix<(int)durRaiseTable.GetRowCount();ix++)
//	{
//		int secIdRaise = 0;
//		double m_dRate = Con_doubleInvalid;
//		durRaiseTable.GetCell(0,ix,secIdRaise);
//		durRaiseTable.GetCell(3,ix,m_dRate);
//		for(int jx = 0;jx < (int)resultTable.GetRowCount();jx++)
//		{
//			int tempId = 0;
//			resultTable.GetCell(0,jx,tempId);
//			if(tempId > 0 && tempId == secIdRaise)
//				resultTable.SetCell(3,jx,m_dRate);
//		}		
//	}
	/*for(int ix = 0;ix<(int)vecResSampleId.size();ix++)
	{
		double m_dRate = Con_doubleInvalid;
		durRaiseTable.GetCell(3,ix,m_dRate);
		resultTable.SetCell(3,ix,m_dRate);
	}*/
	//int m_iEnd = ::GetTickCount();
	//TRACE(_T("\r\nCycleRate cost time: %d\r\n"),m_iEnd- m_iStart);
	prw.SetPercent(progId,0.9);
	//step5 个股统计添加汇总
	if( resultTable.GetRowCount()>0)
	{
		double dValuein = 0;
		double dValueout = 0;
		double dValue = 0;
		double dRatio = 0;
		iCol = 0;
		double curValue = 0;
		iRow = (int)resultTable.GetRowCount();
		//添加一行记录
		resultTable.AddRow();

		//securityid
		resultTable.SetCell(iCol++,iRow,Con_intInvalid);
		//name
		CString sStat(_T("汇总"));
		resultTable.SetCell(iCol++,iRow,sStat);
		//code
		resultTable.SetCell(iCol++,iRow,Con_strInvalid);
		//raise
		//涨幅不需要统计
		resultTable.SetCell(iCol++,iRow,Con_floatInvalid);

		//统计
		int stat_col = iCol;
		for(int i=0;i<(int)resultTable.GetRowCount();i++)
		{
			stat_col = iCol;
			stat_col++;
			//netflow
			resultTable.GetCell(stat_col,i,curValue);
			if(fabs(curValue-Con_doubleInvalid)>0.000001)
				dValue+=curValue;
			stat_col++;
			//inflow
			resultTable.GetCell(stat_col,i,curValue);
			if(curValue>0)
				dValuein+=curValue;
			stat_col++;
			//outflow
			resultTable.GetCell(stat_col,i,curValue);
			if(curValue>0)
				dValueout+=curValue;
			stat_col++;
		}

		//ratio
		if(dValuein>0 && dValueout>0)
			resultTable.SetCell(iCol++,iRow,dValuein/dValueout);
		else
			resultTable.SetCell(iCol++,iRow,Con_doubleInvalid);

		if(!(dValuein>0)) dValuein = Con_doubleInvalid;
		if(!(dValueout>0)) dValueout = Con_doubleInvalid;
		if(dValueout<0 || dValuein<0) dValue = Con_doubleInvalid;

		curValue = 0;
		//netflow
		resultTable.SetCell(iCol++,iRow,dValue);
		//inflow
		resultTable.SetCell(iCol++,iRow,dValuein);
		//outflow
		resultTable.SetCell(iCol++,iRow,dValueout);
	}
	prw.SetPercent(progId,1.0);
#ifdef _DEBUG
	strTable=resultTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif

	return true;
}

// added by zhoup 2009.04.29
// 板块市盈率统计接口
bool TxStock::GetBlockPEPB(Tx::Core::Table& resultTable, std::set<int> setBlockId, UINT nStartDate, UINT nEndDate, int iSJZQ, int iJSFF, bool bPE, bool bPB )
{
	//// 临时的做法
	std::unordered_map<int,CString> BlockIdNameMap;

	//modified by zhangxs 20101129;
	BlockIdNameMap.clear();
	Tx::Data::HotBlockInfoMap::GetHotBlockInfoMap()->GetMapAllSampleMap_PePb(BlockIdNameMap);

	if (setBlockId.empty())
		return false;
	if (nEndDate<nStartDate)
		return false;
	if (iJSFF < 0)
		return false;
	resultTable.Clear();
	// 不做跳转
	// resultTable的列数要根据时间基准序列的个数来定
	resultTable.AddCol(Tx::Core::dtype_val_string);	// 板块名称

#ifdef _DEBUG
	Tx::Core::TxDateTime starttime=Tx::Core::TxDateTime::getCurrentTime();
#endif

	//默认的返回值状态
	bool result = false;
	int iCol = 0;

	double dpro=0.0;
	Tx::Core::Table calcuTable;

	//取时间基准
	m_pShIndex->LoadHisTrade();
	m_pShIndex->LoadTradeDate();
	int iStandard=m_pShIndex->GetTradeDataCount();
	int iStandardStartIndex=m_pShIndex->GetTradeDateIndex(nStartDate);
	int iStandardEndIndex = m_pShIndex->GetTradeDateIndex(nEndDate);

	if (0 == iStandardStartIndex)
	{
		nStartDate = m_pShIndex->GetTradeDateOffset(nStartDate,0);
	}
	std::vector<int> iDateVec;
	iDateVec.clear();
	int iStartDate = nStartDate, iEndDate = nEndDate;
	if (1 == iSJZQ)	//周
	{
		iStartDate = Tx::Core::TxDate::CalculateFridayOfWeek(iStartDate);
		iEndDate = Tx::Core::TxDate::CalculateFridayOfWeek(iEndDate);
		//最后一天算出的周五不在日期的范围内，则舍去，日期截止日往前推一周
		if (iEndDate > (int)nEndDate)
		{
			iEndDate = Tx::Core::TxDate::CalculateDateOffsetDays(iEndDate,-7);
		}
		while (iStartDate <= iEndDate)
		{
			int iTempDate;
			if (m_pShIndex->IsTradeDate(iStartDate))
			{
				iDateVec.push_back(iStartDate);
			} 
			else
			{
				iTempDate = m_pShIndex->GetTradeDateOffset(iStartDate,0);
				//往前找前一个交易日，如果不在一周的范围内，说明之前的一周没有有效交易日，则此日期不计入Vect
				//防止因放假交易所停止交易超出一周的情况而造成的数据冗余
				if (iTempDate > Tx::Core::TxDate::CalculateDateOffsetDays(iStartDate,-7))
				{
					iDateVec.push_back(iTempDate);
				}
			}
			iStartDate = Tx::Core::TxDate::CalculateDateOffsetDays(iStartDate,7);
		}
	} 
	else if (2 == iSJZQ)	//月
	{
		int iTempDate = Tx::Core::TxDate::CalculateEndOfMonth(iStartDate);
		while (iTempDate <= iEndDate)
		{
			if (m_pShIndex->IsTradeDate(iTempDate))
			{
				iDateVec.push_back(iTempDate);
			} 
			else
			{
				iDateVec.push_back(m_pShIndex->GetTradeDateOffset(iTempDate,0));
			}
			iTempDate = Tx::Core::TxDate::CalculateDateOffsetMonths(iTempDate,1);
		}
	}
	else if (3 == iSJZQ)	//年
	{
		int iTempDate = Tx::Core::TxDate::CalculateEndOfYear(iStartDate);
		while (iTempDate <= iEndDate)
		{
			if (m_pShIndex->IsTradeDate(iTempDate))
			{
				iDateVec.push_back(iTempDate);
			} 
			else
			{
				iDateVec.push_back(m_pShIndex->GetTradeDateOffset(iTempDate,0));
			}
			iTempDate = Tx::Core::TxDate::CalculateDateOffsetYears(iTempDate,1);
		}
	}
	else					//日
	{
		while (iStartDate <= iEndDate)
		{
			if (m_pShIndex->IsTradeDate(iStartDate))
			{
				iDateVec.push_back(iStartDate);
			}
			iStartDate = Tx::Core::TxDate::CalculateDateOffsetDays(iStartDate,1);
		}
		if (iDateVec.size() < 1)
		{
			if (-1 == m_pShIndex->GetTradeDateOffset(iEndDate,0))
			{
				if (iEndDate == m_pShIndex->GetCurDataDate())
				{
					iDateVec.push_back(iEndDate);
				}
			}
			else
			{
				iDateVec.push_back(m_pShIndex->GetTradeDateOffset(iEndDate,0));
			}
		}
	}
	if (iDateVec.size() < 1)
	{
		AfxMessageBox(_T("您所选的日期区间内没有符合要求的交易日,请重新输入!"));
		resultTable.Clear();
		iDateVec.clear();
		return false;

	}


	// 列数确定
	resultTable.AddRow(setBlockId.size() + 1);// 日期

	////设置名字
	UINT nRow = 1;
	for (std::set<int>::iterator iter = setBlockId.begin();iter != setBlockId.end();iter++)
	{
		std::unordered_map<int,CString>::iterator iterName = BlockIdNameMap.find(*iter);
		if (iterName != BlockIdNameMap.end())
		{
			resultTable.SetCell(0,nRow++,iterName->second);
		}
		else
			resultTable.SetCell(0,nRow++,Tx::Core::Con_strInvalid);
	}

	if(bPE && bPB)	//计算PE和PB
	{
		for (UINT i = 0;i<iDateVec.size();i++)
		{
			resultTable.AddCol(Tx::Core::dtype_double);
			resultTable.AddCol(Tx::Core::dtype_double);
			resultTable.SetCell(1+2*i,0,static_cast<double>(iDateVec[i]));
			resultTable.SetCell(2+2*i,0,static_cast<double>(iDateVec[i]));
		}
	}
	else if (bPE || bPB)	//只计算PE或者PB
	{
		for (UINT i = 0;i<iDateVec.size();i++)
		{
			resultTable.AddCol(Tx::Core::dtype_double);
			resultTable.SetCell(1+i,0,static_cast<double>(iDateVec[i]));
		}
	} 
	else
	{
		AfxMessageBox(_T("请选择PE,PB至少一个计算指标"));
		resultTable.Clear();
		iDateVec.clear();
		return false;
	}

	int iStat= -1;
	if (bPE)
	{
		iStat = iJSFF;
		// 获得PB
		// 传递二进制
		// int -- 功能类型 1--板块
		// int -- 计算方法 0--PB,1--一年滚动,2--同比,3--静态,4--简单
		// int -- 交易实体个数
		// n*int -- 交易实体序列
		// int -- 日期个数
		// m*int -- 日期序列

		int iParamSize = 4 + setBlockId.size() + iDateVec.size();
		int* pBufferParam = NULL;
		pBufferParam = new int [iParamSize];
		if (pBufferParam == NULL)
			return false;

		int* pBuffer = pBufferParam;

		memset(pBuffer,0,iParamSize * sizeof(int));

		// 功能类型 1--板块市盈率
		int iType = 1;
		//memcpy_s(pBufferParam,iParamSize,&iType,sizeof(int));
		*pBuffer = iType;
		pBuffer++;

		// int -- 计算方法 0--PB,1--一年滚动,2--同比,3--静态,4--简单
		//memcpy_s(pBufferParam + sizeof(int),iParamSize,&iType,sizeof(int));
		*pBuffer = iStat;
		pBuffer++;

		// 样本个数
		*pBuffer = setBlockId.size();
		pBuffer++;

		// 交易实体id
		for (std::set<int>::iterator iter = setBlockId.begin(); iter!=setBlockId.end();iter++)
		{
			*pBuffer = *iter;
			pBuffer++;
		}

		// 日期个数
		*pBuffer = iDateVec.size();
		pBuffer++;

		// 日期序列
		for (UINT i=0;i<iDateVec.size();i++)
		{
			*pBuffer = iDateVec[i];
			pBuffer++;
		}

		//LPCTSTR lpUrl = _T("http://192.168.5.87/StockPEPBSer/Handler.ashx");
		LPCTSTR lpUrl = CORE_GETSYSINFO->GetServerAddr(_T("File"),_T("InstitutionRightsAndProfits"));

		Tx::Drive::Http::CSyncUpload upload;
		int iStart = ::GetTickCount();
		if ( upload.Post(lpUrl, (LPBYTE)pBufferParam, iParamSize * sizeof(int)) )
		{
			int iEnd = ::GetTickCount();
			TRACE(_T("\r\nURL Cost Time %d(ms)\r\n"),iEnd-iStart);
			CONST Tx::Drive::Mem::MemSlice &data = upload.Rsp().Body();
			LPBYTE lpRes = data.DataPtr();
			UINT nRetSize = data.Size();
			if (nRetSize <= 0)
			{
				delete pBufferParam;
				pBufferParam = NULL;
				return false;
			}
			// 返回格式 n*m的序列
			// 交易实体1的m个日期的值
			// 交易实体2的m个日期的值
			UINT nPreSize = *(reinterpret_cast<UINT*>(lpRes));
			LPBYTE lpData = new BYTE[nPreSize];
			if ( lpData == NULL )
			{
				delete pBufferParam;
				pBufferParam = NULL;
				return false;
			}
			if ( !Tx::Drive::IO::Zip::CZipWrapper::MemUnZip(lpData, 
				nPreSize, lpRes + sizeof(UINT), nRetSize - sizeof(UINT)) )
			{
				delete []lpData;
				delete[] pBufferParam;
				pBufferParam = NULL;
				return false;
			}
			int iStart = ::GetTickCount();
			LPBYTE pRecv = lpData;
			UINT nParseCount = setBlockId.size() * iDateVec.size();
			float fValue = 0.0;
			double dValue = 0.0;
			for (UINT i=0;i < nParseCount; i++)
			{
				memcpy_s(&fValue,sizeof(float),pRecv,sizeof(float));
				pRecv += sizeof(float);
				// 返回就是有效和无效,不再判断
				if (bPB)
				{
					if (fabs(fValue - Tx::Core::Con_floatInvalid) < 0.1)
						resultTable.SetCell(1+2*(i%iDateVec.size()),1+i/iDateVec.size(),Tx::Core::Con_doubleInvalid);
					else
						resultTable.SetCell(1+2*(i%iDateVec.size()),1+i/iDateVec.size(),static_cast<double>(fValue));
				}
				else
				{
					if (fabs(fValue - Tx::Core::Con_floatInvalid) < 0.1)
						resultTable.SetCell(1+i%iDateVec.size(),1+i/iDateVec.size(),Tx::Core::Con_doubleInvalid);
					else
						resultTable.SetCell(1+i%iDateVec.size(),1+i/iDateVec.size(),static_cast<double>(fValue));
				}
			}
			delete []lpData;
			lpData = NULL;
			iEnd = ::GetTickCount();
			TRACE(_T("\r\nParse Result Cost Time %d(ms)\r\n"),iEnd-iStart);
		}
		if (pBufferParam != NULL)
		{
			delete pBufferParam;
			pBufferParam = NULL;
		}
	}
	if (bPB)
	{
		iStat = 0;
		// 获得PB
		// 传递二进制
		// int -- 功能类型 1--板块
		// int -- 计算方法 0--PB,1--一年滚动,2--同比,3--静态,4--简单
		// int -- 交易实体个数
		// n*int -- 交易实体序列
		// int -- 日期个数
		// m*int -- 日期序列

		int iParamSize = 4 + setBlockId.size() + iDateVec.size();
		int* pBufferParam = NULL;
		pBufferParam = new int [iParamSize];
		if (pBufferParam == NULL)
			return false;

		int* pBuffer = pBufferParam;

		memset(pBuffer,0,iParamSize * sizeof(int));

		// 功能类型 1--板块市盈率
		int iType = 1;
		//memcpy_s(pBufferParam,iParamSize,&iType,sizeof(int));
		*pBuffer = iType;
		pBuffer++;

		// int -- 计算方法 0--PB,1--一年滚动,2--同比,3--静态,4--简单
		//memcpy_s(pBufferParam + sizeof(int),iParamSize,&iType,sizeof(int));
		*pBuffer = iStat;
		pBuffer++;

		// 样本个数
		*pBuffer = setBlockId.size();
		pBuffer++;

		// 交易实体id
		for (std::set<int>::iterator iter = setBlockId.begin(); iter!=setBlockId.end();iter++)
		{
			*pBuffer = *iter;
			pBuffer++;
		}

		// 日期个数
		*pBuffer = iDateVec.size();
		pBuffer++;

		// 日期序列
		for (UINT i=0;i<iDateVec.size();i++)
		{
			*pBuffer = iDateVec[i];
			pBuffer++;
		}

		//LPCTSTR lpUrl = _T("http://007webdata.chinaciia.com/StockPEPBSerBeta/Handler.ashx");
		LPCTSTR lpUrl = CORE_GETSYSINFO->GetServerAddr(_T("File"),_T("InstitutionRightsAndProfits"));

		Tx::Drive::Http::CSyncUpload upload;
		int iStart = ::GetTickCount();
		if ( upload.Post(lpUrl, (LPBYTE)pBufferParam, iParamSize * sizeof(int)) )
		{
			int iEnd = ::GetTickCount();
			TRACE(_T("\r\nURL Cost Time %d(ms)\r\n"),iEnd-iStart);
			CONST Tx::Drive::Mem::MemSlice &data = upload.Rsp().Body();
			LPBYTE lpRes = data.DataPtr();
			UINT nRetSize = data.Size();
			if (nRetSize <= 0)
			{
				delete pBufferParam;
				pBufferParam = NULL;
				return false;
			}
			// 返回格式 n*m的序列
			// 交易实体1的m个日期的值
			// 交易实体2的m个日期的值
			UINT nPreSize = *(reinterpret_cast<UINT*>(lpRes));
			LPBYTE lpData = new BYTE[nPreSize];
			if ( lpData == NULL )
			{
				delete pBufferParam;
				pBufferParam = NULL;
				return false;
			}
			if ( !Tx::Drive::IO::Zip::CZipWrapper::MemUnZip(lpData, 
				nPreSize, lpRes + sizeof(UINT), nRetSize - sizeof(UINT)) )
			{
				delete []lpData;
				delete[] pBufferParam;
				pBufferParam = NULL;
				return false;
			}
			int iStart = ::GetTickCount();
			LPBYTE pRecv = lpData;
			UINT nParseCount = setBlockId.size() * iDateVec.size();
			float fValue = 0.0;
			double dValue = 0.0;
			for (UINT i=0;i < nParseCount; i++)
			{
				memcpy_s(&fValue,sizeof(float),pRecv,sizeof(float));
				pRecv += sizeof(float);
				// 返回就是有效和无效,不再判断
				if (bPE)
				{
					if (fabs(fValue - Tx::Core::Con_floatInvalid) < 0.1)
					{
						// 无效值
						resultTable.SetCell(2+2*(i%iDateVec.size()),1+i/iDateVec.size(),Tx::Core::Con_doubleInvalid);
					}
					else
						resultTable.SetCell(2+2*(i%iDateVec.size()),1+i/iDateVec.size(),static_cast<double>(fValue));
				}
				else
				{
					if (fabs(fValue - Tx::Core::Con_floatInvalid) < 0.1)
						resultTable.SetCell(1+i%iDateVec.size(),1+i/iDateVec.size(),Tx::Core::Con_doubleInvalid);
					else
						resultTable.SetCell(1+i%iDateVec.size(),1+i/iDateVec.size(),static_cast<double>(fValue));
				}
			}
			delete []lpData;
			lpData = NULL;
			iEnd = ::GetTickCount();
			TRACE(_T("\r\nParse Result Cost Time %d(ms)\r\n"),iEnd-iStart);
		}
		if (pBufferParam != NULL)
		{
			delete pBufferParam;
			pBufferParam = NULL;
		}
	}
#ifdef _DEBUG
	CString strTable=resultTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif

	//	int iStart = ::GetTickCount();
	//	LPBYTE pRecv = lpData;
	//	UINT nParseCount = resTable.GetRowCount() * iDates.size();
	//	float fValue = 0.0;
	//	double dValue = 0.0;
	//	for (UINT i=0;i < nParseCount; i++)
	//	{
	//		memcpy_s(&fValue,sizeof(float),pRecv,sizeof(float));
	//		pRecv += sizeof(float);
	//		if (fabs(fValue - Tx::Core::Con_floatInvalid) < 0.00001)
	//			// 无效值
	//			//resultTable.SetCell(i%date.size() + 1,i/date.size(),Tx::Core::Con_doubleInvalid);
	//		{
	//			if (iStatIndicator == 1)
	//				resTable.SetCell(i%iDates.size() + 3,i/iDates.size(),Tx::Core::Con_doubleInvalid);
	//			else
	//				resTable.SetCell(2 * (i%iDates.size()) + 3,i/iDates.size(),Tx::Core::Con_doubleInvalid);
	//		}
	//		else
	//		{
	//			dValue = (double)fValue;
	//			if (iStatIndicator == 1)
	//				resTable.SetCell(i%iDates.size() + 3,i/iDates.size(),dValue);
	//			else
	//				resTable.SetCell(2 * (i%iDates.size()) + 3,i/iDates.size(),dValue);
	//		}
	//	}
	//	delete []lpData;
	//	lpData = NULL;
	//	int iEnd = ::GetTickCount();
	//	TRACE(_T("\r\nParse Result Cost Time %d(ms)\r\n"),iEnd-iStart);
	//}

	return true;
}
	}//end of business
}//end of tx


