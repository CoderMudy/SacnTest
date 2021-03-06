/**************************************************************
	Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
	File name:		TxFundStat.cpp
	Author:			关耀东
	Date:			2007-09-17
	Version:		1.0
	Description:	基金-专项统计子模块功能实现集合
	
	
					修改人		修改类型		修改时间		内容梗概
	history:		
***************************************************************/
#include"stdafx.h"
//#include "vld.h"
#include"Fund.h"


#include "..\..\data\FunctionDataManager.h"
#include "..\..\data\SecurityTradeDate.h"
#include "..\..\data\SecurityBase.h"
#include "FundDeriveData.h"
// add by lijw 2008-02-26
#include "..\..\Core\Core\Table.h"
#include "..\..\Core\ZkkLibrary\Date.h"
#include <algorithm>
#include <unordered_map>
//#include"TxBusiness.h"

#include "../../Core/Driver/ClientFileEngine/base/HttpDnAndUp.h"
#include "../../Core/Driver/ClientFileEngine/base/zip/ZipWrapper.h"

#include "..\..\Data\SecurityExtentInfo.h"

namespace Tx
{
	namespace Business
	{

//modify by lijw 2008-08-06
//基金份额
bool TxFund::StatFundShare(
		Tx::Core::Table_Indicator	&resTable,
		std::vector<int>	&iObjectId,
		int		iStartDate,
		int		iEndDate,
		bool	bAllDate,
		UINT	uReason,
//		UINT	uStyle,
		UINT	uType
		)
{
		m_txTable.Clear();
		tmpTable.Clear();
		int nCountTest = 0;
		std::vector<CString> vReason;
		std::vector<int> vStyle,vType;
		int iBitFlag=1;
		CString strReason[5]={_T("基金设立"),_T("份额变动"),_T("基金上市"),_T("基金扩募"),_T("发起人份额流通")};
		for(int i=1;i<6;i++,iBitFlag=iBitFlag<<1)
		{
			if(uReason&iBitFlag)
				vReason.push_back(strReason[i-1]);			
		}		
		/*iBitFlag=1;
		for(int i=1;i<18;i++,iBitFlag=iBitFlag<<1)
		{
			if(uStyle&iBitFlag)
			{
				if(i<8)
					vStyle.push_back(i+1);
				else
					if(i<12)
						vStyle.push_back(i+2);
					else
							vStyle.push_back(i+3);
			}
		}		*/
		iBitFlag=1;
		for(int i=1;i<3;i++,iBitFlag=iBitFlag<<1)
		{
			if(uType&iBitFlag)
				if(i==1)
				{
					vType.push_back(1);			
					vType.push_back(2);			
					vType.push_back(3);			
					vType.push_back(4);			
				}
				else
					vType.push_back(0);			
		}

		m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//交易实体
		m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//startdate

		UINT arrParaIndex[2];
		arrParaIndex[0]=0;
		arrParaIndex[1]=1;
		m_txTable.AddIndicatorColumn(30301104,Tx::Core::dtype_int4,arrParaIndex,2);//公告日期
		m_txTable.AddIndicatorColumn(30301106,Tx::Core::dtype_val_string,arrParaIndex,2);//变动原因
		m_txTable.AddIndicatorColumn(30301111,Tx::Core::dtype_decimal,arrParaIndex,2);//变动后份额
		m_txTable.AddIndicatorColumn(30301112,Tx::Core::dtype_val_string,arrParaIndex,2);//信息来源
		m_txTable.AddIndicatorColumn(30301116,Tx::Core::dtype_val_string,arrParaIndex,2);//备注

		m_pLogicalBusiness->GetData(m_txTable,true);
#ifdef _DEBUG
		nCountTest = (int)m_txTable.GetRowCount();
		CString strt=m_txTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strt);
#endif
		m_txTable.InsertCol(2,Tx::Core::dtype_val_string);
		m_txTable.InsertCol(3,Tx::Core::dtype_val_string);

		m_txTable.InsertCol(6,Tx::Core::dtype_decimal);

		int curId,preId;
		double dTotalSharePre;
		for(UINT i=0;i<m_txTable.GetRowCount();i++)
		{
			
			if(i==0)
			{
				m_txTable.SetCell(6,i,Tx::Core::Con_doubleInvalid);
			}
			else
			{
				m_txTable.GetCell(0,i,curId);
				m_txTable.GetCell(0,i-1,preId);
				if(curId==preId)
				{
					m_txTable.GetCell(7,i-1,dTotalSharePre);
					m_txTable.SetCell(6,i,dTotalSharePre);
				}
				else
				{
					m_txTable.SetCell(6,i,Tx::Core::Con_doubleInvalid);
				}
			}
		}

		this->IdColToNameAndCode(resTable,0,1);
#ifdef _DEBUG
		nCountTest = (int)resTable.GetRowCount();
		strt=resTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strt);
#endif
		tmpTable.AddParameterColumn(Tx::Core::dtype_int4);//基金id
		UINT arrParaIndex1[1];
		arrParaIndex1[0]=0;


		tmpTable.AddIndicatorColumn(30001034,Tx::Core::dtype_int4,arrParaIndex1,1);//风格
		//tmpTable.AddIndicatorColumn(30001035,Tx::Core::dtype_int4,arrParaIndex1,1);//类型
		//modified by zhangxs 20091221---NewStyle
		tmpTable.AddIndicatorColumn(30001232,Tx::Core::dtype_int4,arrParaIndex1,1);//类型
		m_pLogicalBusiness->GetData(tmpTable,true);
		
#ifdef _DEBUG
		nCountTest = (int)tmpTable.GetRowCount();
		strt=tmpTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strt);
#endif
		Tx::Core::Table_Indicator TableDate;

		UINT nCol=m_txTable.GetColCount();
		UINT* nColArray=new UINT[nCol+10];
		for(UINT i=0;i<nCol+10;i++)
		{
			nColArray[i]=i;
		}

		if((bAllDate)&&(m_txTable.GetRowCount()!=0))
		{
			TableDate.Clone(m_txTable);
		}
		else
		{
			TableDate.CopyColumnInfoFrom(m_txTable);
			if(m_txTable.GetRowCount()!=0)
				m_txTable.Between(TableDate,nColArray,nCol,1,iStartDate,iEndDate,true,true);
		}
		std::unordered_map<int,int> mId;
		std::unordered_map<int,int>::iterator IdIter;
		int iId;
		for(UINT i=0;i<tmpTable.GetRowCount();i++)
		{
			tmpTable.GetCell(0,i,iId);
			mId.insert(std::make_pair(iId,i));
		}
		
		int iStyle,iType;
		
		TableDate.AddCol(Tx::Core::dtype_int4);
		TableDate.AddCol(Tx::Core::dtype_int4);
		CString tmpStr;


		std::unordered_map<int,CString> mType,mStyle;
		std::unordered_map<int,CString>::iterator iterTypeStyle;
#ifdef TYPE_FUND_TYPE_INDEX
		GetIndexDat(TYPE_FUND_TYPE_INDEX,mType);
#else
		GetIndexDat(23,mType);
#endif
//modified by zhangxs 20091221---NewStyle
//#ifdef TYPE_FUND_STYLE_INDEX
//		GetIndexDat(TYPE_FUND_STYLE_INDEX,mStyle);
//#else
//		GetIndexDat(24,mStyle);
//#endif
#ifdef TYPE_FUND_STYLE_INDEX_NEW
		GetIndexDat(TYPE_FUND_STYLE_INDEX_NEW,mStyle);
#else
		GetIndexDat(60,mStyle);
#endif


		for(UINT i=0;i<TableDate.GetRowCount();i++)
		{
			TableDate.GetCell(0,i,iId);
			TableDate.GetCell(5,i,tmpStr);
			if(tmpStr=="基金设立")
				TableDate.SetCell(6,i,Tx::Core::Con_doubleInvalid);
			if(GetSecurityNow(iId)!=NULL)
			{
				//modified by zhangxs 20101123
//#ifdef _SECURITYID_FOR_STAT_
				iId = m_pSecurity->GetId();
//#else
//				iId=m_pSecurity->GetSecurity1Id();
//#endif
				IdIter=mId.find(iId);
				if(IdIter!=mId.end())
				{
					tmpTable.GetCell(1,IdIter->second,iType);
					iterTypeStyle=mType.find(iType);
					if(iterTypeStyle!=mType.end())
					{
						tmpStr.Format(iterTypeStyle->second);
					}
					else
					{
						tmpStr = _T("未定义类型");
					}
					TableDate.SetCell(2,i,tmpStr);

					TableDate.SetCell(11,i,iType);
					tmpTable.GetCell(2,IdIter->second,iStyle);
					iterTypeStyle=mStyle.find(iStyle);
					if(iterTypeStyle!=mStyle.end())
					{
						tmpStr.Format(iterTypeStyle->second);
					}
					else
					{
						tmpStr = _T("未定义类型");
					}
					TableDate.SetCell(3,i,tmpStr);
					TableDate.SetCell(10,i,iStyle);
				}

			}

		}
#ifdef _DEBUG
		nCountTest = (int)TableDate.GetRowCount();
		strt=TableDate.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strt);
#endif
		Tx::Core::Table_Indicator TableReason,TableStyle,TableType;


		resTable.CopyColumnInfoFrom(TableDate);

		nCol=TableDate.GetColCount();
		if((vReason.size()==5)&&(TableDate.GetRowCount()!=0))
			TableReason.Clone(TableDate);
		else
		{
			TableReason.CopyColumnInfoFrom(TableDate);
			TableDate.EqualsAt(TableReason,nColArray,nCol,5,vReason);
		}
		//add by lijw 2005-12-25
		if(TableReason.GetRowCount() == 0)
			return false;
//#ifdef _DEBUG
//		strt=TableReason.TableToString();
//		Tx::Core::Commonality::String().StringToClipboard(strt);
//#endif
//		if((vStyle.size()==15)&&(TableReason.GetRowCount()!=0))
//			TableStyle.Clone(TableReason);
//		else 
//		{
//		TableStyle.CopyColumnInfoFrom(TableDate);
//			TableReason.EqualsAt(TableStyle,nColArray,nCol,10,vStyle);
//
//		}
#ifdef _DEBUG
		nCountTest = (int)TableStyle.GetRowCount();
		strt=TableStyle.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strt);
#endif
		if((vReason.size()==2)&&(TableReason.GetRowCount()!=0))
			TableType.Clone(TableReason);
		else 
		{
			TableType.CopyColumnInfoFrom(TableDate);
			TableReason.EqualsAt(TableType,nColArray,nCol,11,vType);
		}
		//add by lijw 2005-12-25
		if(TableType.GetRowCount() == 0)
			return false;
#ifdef _DEBUG
		nCountTest = (int)TableType.GetRowCount();
		strt=TableType.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strt);
#endif
		TableType.EqualsAt(resTable,nColArray,nCol,0,iObjectId);

#ifdef _DEBUG
		nCountTest = (int)resTable.GetRowCount();
		strt=resTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strt);
#endif
		resTable.DeleteCol(11);
		resTable.DeleteCol(10);
		double dShare=0;
		resTable.InsertCol(5,Tx::Core::dtype_int4);
#ifdef _DEBUG
		nCountTest = (int)resTable.GetRowCount();
		CString sss= resTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(sss);
#endif
		for(UINT i=0;i<resTable.GetRowCount();i++)
		{
			resTable.GetCell(7,i,dShare);
			if(dShare!=Tx::Core::Con_doubleInvalid)
			{
				resTable.SetCell(7,i,dShare/100000000);
			}

			resTable.GetCell(8,i,dShare);
			if(dShare!=Tx::Core::Con_doubleInvalid)
			{
				resTable.SetCell(8,i,dShare/100000000);
			}
			int datetemp;
			resTable.GetCell(1,i,datetemp);
			resTable.SetCell(5,i,datetemp);
		}
		resTable.DeleteCol(1);
		this->IdColToNameAndCode(resTable,0,1);
		
//		resTable.DeleteCol(0);
		delete nColArray;
		nColArray = NULL;
	return true;
}
		

	
//交易行情[高级]
	bool	TxFund::StatCycleRate(
		Tx::Core::Table_Indicator	&resTable,
		std::set<int>	&iSecurityId,
		int		iStartDate,
		int		iEndDate,
		bool bCutFirstDateRaise,			//剔除首日涨幅
		int	iFQLX							//复权类型
		)
	{
	return BlockCycleRateAdv(
		iSecurityId,		//交易实体ID
		iStartDate,			//起始日期
		iEndDate,			//终止日期
		bCutFirstDateRaise,	//剔除首日涨幅
		iFQLX,				//复权类型 0-不复权;1=后复权
		resTable,			//结果数据表
		3
		);
	}



//基金分红
	// modify by lijw 2008-02-03
	bool	TxFund::StatFundDividend(
		Tx::Core::Table_Indicator	&resTable,
		std::vector<int>	&iSecurityId,
		int		iStartDate,
		int		iEndDate,
		bool	bGetAllDate
		)
	{
		if(iSecurityId.size() == 0)
			return false;
		//step1
//		Tx::Core::ProgressWnd* pProgressWnd = Tx::Core::ProgressWnd::GetInstance();
		Tx::Core::ProgressWnd prw;
		//step2
		CString sProgressPrompt;
		sProgressPrompt =  _T("基金分红统计");
		UINT progId = prw.AddItem(1,sProgressPrompt, 0.0);
		//step3
		prw.Show(15);
		prw.EnableCancelButton(true);

		prw.SetPercent(progId, 0.05);
		if(prw.IsCanceled()==true)
		{
			prw.SetPercent(progId,1.0);
			prw.EnableCancelButton(false);
			resTable.DeleteRow(0,resTable.GetRowCount());
			return false;
		}
		//下面是统计的功能 modify by lijw 2008-02-03
		this->m_txTable.Clear();

		std::vector<int>	iFundId;
		//两个参数列分别是交易实体id，除息日
		m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
		//除权日
		m_txTable.AddParameterColumn(Tx::Core::dtype_int4);

		UINT arrParaIndex[2];
		arrParaIndex[0]=0;
		arrParaIndex[1]=1;

	/*	m_txTable.AddIndicatorColumn(30301010,Tx::Core::dtype_int4,arrParaIndex,2);
		m_txTable.AddIndicatorColumn(30301012,Tx::Core::dtype_val_string,arrParaIndex,2)*/;
		//分红年度
		m_txTable.AddIndicatorColumn(30301017,Tx::Core::dtype_int4,arrParaIndex,2);
		//登记日指标列
		m_txTable.AddIndicatorColumn(30301013,Tx::Core::dtype_int4,arrParaIndex,2);
		//分红比例分子
		m_txTable.AddIndicatorColumn(30301015,Tx::Core::dtype_decimal,arrParaIndex,2);
		//分红比例分母
		m_txTable.AddIndicatorColumn(30301014,Tx::Core::dtype_decimal,arrParaIndex,2);
//		m_txTable.AddIndicatorColumn(30301021,Tx::Core::dtype_decimal,arrParaIndex,2);

		prw.SetPercent(progId, 0.15);
		if(prw.IsCanceled()==true)
		{
			prw.SetPercent(progId,1.0);
			prw.EnableCancelButton(false);
			resTable.DeleteRow(0,resTable.GetRowCount());
			return false;
		}
		//取出全部的数据
		m_pLogicalBusiness->GetData(m_txTable,true);
#ifdef _DEBUG
		for(int it=0;it<(int)m_txTable.GetRowCount();it++)
		{
			int itDate = 0;
			m_txTable.GetCell(1,it,itDate);
			GlobalWatch::_GetInstance()->WatchHere(_T("zhaohj|| col2 = %d"),itDate);
		}
		CString strTable1=m_txTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable1);
#endif
		prw.SetPercent(progId, 0.45);
		if(prw.IsCanceled()==true)
		{
			prw.SetPercent(progId,1.0);
			prw.EnableCancelButton(false);
			resTable.DeleteRow(0,resTable.GetRowCount());
			return false;
		}
		resTable.Clone(m_txTable);

		UINT* nColArray = new UINT[resTable.GetColCount()];
		for(UINT i=0;i<resTable.GetColCount();i++)
			nColArray[i]=i;

		prw.SetPercent(progId, 0.65);
		if(prw.IsCanceled()==true)
		{
			prw.SetPercent(progId,1.0);
			prw.EnableCancelButton(false);
			resTable.DeleteRow(0,resTable.GetRowCount());
			delete nColArray;
			nColArray = NULL;
			return false;
		}

		Tx::Core::Table_Indicator	resTableDate;

		resTableDate.CopyColumnInfoFrom(resTable);

		//数据在resTable中做调整以及筛选
		if(resTable.GetRowCount()>0)
		{
		//日期筛选
			if(!bGetAllDate)
			{
				//std::set<int>::iterator s;
				//除权日期为筛选日期
				resTable.Between(resTableDate,nColArray,resTable.GetColCount(),1,iStartDate,iEndDate,true,true);
			}
			else
			{
				resTableDate.Clone(resTable);
			}

			resTable.Clear();
			resTable.CopyColumnInfoFrom(resTableDate);
		//样本筛选
			
			resTableDate.EqualsAt(resTable,nColArray,resTable.GetColCount(),0,iSecurityId);
			
		}
		delete nColArray;
		nColArray = NULL;		
		prw.SetPercent(progId, 0.85);
		if(prw.IsCanceled()==true)
		{
			prw.SetPercent(progId,1.0);
			prw.EnableCancelButton(false);
			resTable.DeleteRow(0,resTable.GetRowCount());
			return false;
		}
#ifdef _DEBUG
		CString strTable2=m_txTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable2);
#endif
		resTable.AddCol(Tx::Core::dtype_val_string);// 存放分配方案
		resTable.AddCol(Tx::Core::dtype_double);    // 流通分配
        resTable.AddCol(Tx::Core::dtype_double);    // 分配总额
//		resTable.AddCol(Tx::Core::dtype_val_string);// 注册省份
//		resTable.AddCol(Tx::Core::dtype_val_string);// 证监会行业代码
		double dShares,dRate;
		CString sRate;
		for(UINT i=0;i<resTable.GetRowCount();i++)
		{
			resTable.GetCell(4,i,dShares);
			resTable.GetCell(5,i,dRate);
			sRate.Format(_T("%2.0f∶%0.3f"),dRate,dShares);
			resTable.SetCell(6,i,sRate);
		}
	  
		////取基金的流通份额和总份额
		//  m_txTable.Clear();
		////基金ID
		//m_txTable.AddParameterColumn(Tx::Core::dtype_int4);

		//UINT varCfg[1];			//参数配置
		//int varCount=1;			//参数个数	
		//varCfg[0]=0;

		////交易提示 指标组
		//const int INDICATOR_INDEX=3;
		//long iIndicator[INDICATOR_INDEX]=
		//{			
		//	30001023,	//截止日期
		//	30001033,	//流通份额
		//	30001032,	//总份额
		//};
		//bool result;
		////设定指标列
		//for (int i = 0; i <	INDICATOR_INDEX; i++)
		//{
		//	GetIndicatorDataNow(iIndicator[i]);
		//	if(m_pIndicatorData == NULL)
		//		return false;
		//	result = m_pLogicalBusiness->SetIndicatorIntoTable(m_pIndicatorData,varCfg,varCount,m_txTable);
		//	if(result==false)
		//		return false;
		//}
		////把交易实体ID转化为券ID
		//std::vector<int> iFund_Id;
		////this->TransObjectToSecIns(iSecurityId,iFund_Id,1);
		//std::vector<int>::iterator iterTemp;
		//int tempId;
		//for(iterTemp = iSecurityId.begin();iterTemp != iSecurityId.end();++iterTemp)
		//{
		//	GetSecurityNow(*iterTemp);
		//	if(m_pSecurity==NULL)
		//		continue;
		//	tempId = m_pSecurity->GetSecurity1Id();
		//	if (find(iFund_Id.begin(),iFund_Id.end(),tempId) == iFund_Id.end())
		//	{
		//		iFund_Id.push_back(tempId);
		//	}
		//}
		//if(!bGetAllDate)
		//{
		//	std::vector<int> vecDate;
		//	vecDate.push_back(iStartDate);
		//	vecDate.push_back(iEndDate);
		//	//根据之前3个步骤的设置进行数据读取，结果数据存放在table中
		//	result=m_pLogicalBusiness->GetData(m_txTable,iFund_Id,vecDate,1,0);
		//	if(result==false)
		//		return false;
		//}
		//else
		//{
		//	result=m_pLogicalBusiness->GetData(m_txTable,true);
		//	if(result==false || m_txTable.GetRowCount() == 0)
		//		return false;
		//	UINT* nColArray2 = new UINT[m_txTable.GetColCount()];
		//	for(UINT j = 0;j < m_txTable.GetColCount();j++)
		//		nColArray2[j]=j;
		//	Tx::Core::Table_Indicator	TempTable;
		//	TempTable.CopyColumnInfoFrom(m_txTable);
		//	m_txTable.EqualsAt(TempTable,nColArray2,m_txTable.GetColCount(),0,iFund_Id);
		//	m_txTable.Clear();
		//	m_txTable.Clone(TempTable);
		//	if(nColArray2 != NULL)
		//	{
		//		delete nColArray2;
		//		nColArray2 = NULL;
		//	}
		//}
#ifdef _DEBUG
		CString strTable=m_txTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
		//把两个表连起来。
		//为了把两个表连起来，需要增加一列，用来存放券ID。
		resTable.InsertCol(1,Tx::Core::dtype_int4);
		//存放名称
		resTable.InsertCol(1,Tx::Core::dtype_val_string);
		//存放代码
		resTable.InsertCol(2,Tx::Core::dtype_val_string);
        //调换除权日的位置
		resTable.InsertCol(7,Tx::Core::dtype_int4);
		
		for(int j = 0;j < (int)resTable.GetRowCount();j++)
		{
			int transId;
			resTable.GetCell(0,j,transId);
			GetSecurityNow(transId);
			if(m_pSecurity==NULL)
				continue;

			//2008-05-04 zhaohj
			//取得除权日期
			int ifDate;
			resTable.GetCell(4,j,ifDate);
			resTable.SetCell(7,j,ifDate);
			//保存

			//取得基金ID
			int fund_id = (int)m_pSecurity->GetSecurity1Id();
			resTable.SetCell(3,j,fund_id);
			//根据交易实体ID取得样本的名称和外码；
			CString strName,strCode;
			strName = m_pSecurity->GetName();
			strCode = m_pSecurity->GetCode();
			//取得证监会行业代码和注册省份
//			CString CSRCode,province;
//			CSRCode = m_pSecurity->GetCSRCIndustryCode();
//			province = m_pSecurity->GetRegisterProvance();
			resTable.SetCell(1,j,strName);
			resTable.SetCell(2,j,strCode);
//			resTable.SetCell(13,j,province);//徐总建议把该列去掉
//			resTable.SetCell(13,j,CSRCode);
		  /*resTable.GetCell(5,j,ifDate);
			resTable.SetCell(5,j,ifDate);*/

			TxFundShareData *str_FundData = NULL;    
			double dAccount = 0.0, dCirculate = 0.0, deno = 0.0, molecule = 0.0;

			/* 20100812 wanglm 基金分红 取变动份额 */
			str_FundData = NULL;
			int ncout = m_pSecurity->GetTxFundShareDataCount();

			std::vector<int> vecTemp;
			vecTemp.clear();
			GetReportDateArr(vecTemp,iStartDate,iEndDate);
			int iRQtemp = vecTemp.at( vecTemp.size() -1 );    // 取最近变动份额的时间

			if(ncout > 0)
			{
				str_FundData = m_pSecurity->GetTxFundShareDataByDate(iRQtemp);
			}

			if(str_FundData != NULL)
			{
				dCirculate = str_FundData->TradeableShare;   // 取到的是变动“流通份额”数据
				dAccount = str_FundData->TotalShare;         // 取到的是变动“总份额”数据
			}

			resTable.GetCell(8,j,deno);               // 10
			resTable.GetCell(9,j,molecule);           // 0.5
#ifdef _DEBUG
			CString strTable = resTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif

			if(dCirculate <= 0.0)  // 流通份额
			{
				dCirculate = Tx::Core::Con_doubleInvalid;    
			}
			else
			{
				dCirculate = dCirculate*deno/molecule/10000;  
			}

			if(dAccount <= 0.0)  // 总份额
			{
				dAccount = Tx::Core::Con_doubleInvalid;
			}
			else
			{
				dAccount = dAccount*deno/molecule/10000;
			}

			resTable.SetCell(11,j,dCirculate);
			resTable.SetCell(12,j,dAccount);
		}



//		//把m_txTable券ID和resTable里的券ID对应起来。并且把流通分配和分配总额放到resTable表里。
//		for(int k = 0;k < (int)m_txTable.GetRowCount();k++)
//		{
//			TxFundShareData *str_FundData = NULL;    // 20100812 wanglm  取变动基金份额
//			double dRise = 0.0 ;
//			int icol = 2;
//
//			int fund_id;
//			m_txTable.GetCell(0,k,fund_id);
//			m_pSecurity = NULL;
//			GetSecurityNow(fund_id);
//			if(m_pSecurity == NULL)
//				continue;
//#ifdef _DEBUG
//			CString strTable=m_txTable.TableToString();
//			Tx::Core::Commonality::String().StringToClipboard(strTable);
//#endif
//
//			std::vector<UINT>::iterator iteID;
//			std::vector<UINT> vecInstiID;
//			resTable.Find(3,fund_id,vecInstiID);
//
//			std::vector<int>::iterator iteRet;
//			std::vector<int> vecTemp;
//			vecTemp.clear();
//			GetReportDateArr(vecTemp,iStartDate,iEndDate);
//            int iRQtemp = vecTemp.at( vecTemp.size() -1 );    // 取最近变动份额的时间
//
//			//for(iteRet = vecTemp.begin(); iteRet != vecTemp.end(); ++iteRet )
//			for ( iteID = vecInstiID.begin();iteID != vecInstiID.end();++iteID )
//			{
//				//icol++;
//				//得到流通分额和总分额
//				double dAccount,dCirculate,deno,molecule;
//				//m_txTable.GetCell(2,k,dCirculate);      // 取到的是初始“流通份额”数据
//				//m_txTable.GetCell(3,k,dAccount);        // 取到的是初始“总份额”数据
//
//                /* 20100812 wanglm 基金分红 取变动份额 */
//				str_FundData = NULL;
//				int ncout = m_pSecurity->GetTxFundShareDataCount();
//				if(ncout > 0)
//				{
//					//str_FundData = m_pSecurity->GetTxFundShareDataByIndex( *iteID );// *iteRet
//					str_FundData = m_pSecurity->GetTxFundShareDataByDate(iRQtemp);
//				}
//                //resTable.GetCell( icol, k, dRise );
//				if(str_FundData != NULL)
//				{
//					int iRQ = str_FundData->iDate;
//					dCirculate = str_FundData->TradeableShare;   // 取到的是变动“流通份额”数据
//					dAccount = str_FundData->TotalShare;         // 取到的是变动“总份额”数据
//				}
//
//				resTable.GetCell(8,*iteID,deno);
//				resTable.GetCell(9,*iteID,molecule);
//
//				if(dCirculate <= 0.0)  // 流通份额
//				{
//					dCirculate = Tx::Core::Con_doubleInvalid;     // wanglm test FH data
//				}
//				else
//				{
//					dCirculate = dCirculate*deno/molecule/10000;  
//					//strCirculate.Format(_T("%.2f"),dCirculate);
//				}
//
//				if(dAccount <= 0.0)  // 总份额
//				{
//					dAccount = Tx::Core::Con_doubleInvalid;
//				}
//				else
//				{
//					dAccount = dAccount*deno/molecule/10000;
//					//strAccount.Format(_T("%.2f"),dAccount);
//				}
//				resTable.SetCell(11,*iteID,dCirculate);
//				resTable.SetCell(12,*iteID,dAccount);
//			
//			}
//		}
	    resTable.DeleteCol(9);
		resTable.DeleteCol(8);
		resTable.DeleteCol(4);
		resTable.DeleteCol(3);
		//step5
		prw.SetPercent(progId, 1.0);
		sProgressPrompt += _T(",完成!");
		prw.SetText(progId,sProgressPrompt);
		prw.EnableCancelButton(false);
		return true;
	}//end of StatFundDividend

	//基金发行
	//基金发行分为开放式基金和封闭式基金。
	//封闭式基金有首发、增发两种类别；并称之为发行价（固定为1）发行量。发行总市值。募集资金要从发行总市值扣除发行费用
	//开放式资金则有基金净销售额（等同于发行总市值）。合计募集份额是基金净销售额加上除银行利息
	//通过分别调用两个模块，从数据库各自表格表中获得数据，然后整合
	//modify by lijw 2008-02-04
	bool	TxFund::StatFundIssue(
				Tx::Core::Table_Indicator	&resTable,
				std::vector<int>	&iSecurityId,
				int		iStartDate,
				int		iEndDate,
				bool	bGetAllDate,
				WORD	wIssueType,
				bool    FXRQ
				)
	{
		Tx::Core::Table_Indicator	resTableOpen;
		std::vector<int> vecF,vecK;
		std::vector<int>::iterator ite;
		int tempId;
		for(ite = iSecurityId.begin();ite != iSecurityId.end();++ite)
		{
			tempId = *ite;
			GetSecurityNow(tempId);
			if(m_pSecurity == NULL)
				continue;
			if (find(vecF.begin(),vecF.end(),tempId) == vecF.end() && find(vecK.begin(),vecK.end(),tempId) == vecK.end())
			{
				if(m_pSecurity->IsFund_Open())
				{
					vecK.push_back(tempId);
				}
				else
					vecF.push_back(tempId);
			}		
		}
		//添加进度条
		//step1
//		Tx::Core::ProgressWnd* pProgressWnd = Tx::Core::ProgressWnd::GetInstance();
		Tx::Core::ProgressWnd prw;
		//step2
		CString sProgressPrompt;
		sProgressPrompt =  _T("基金发行统计");
		UINT progId = prw.AddItem(1,sProgressPrompt, 0.0);
		//step3
		prw.Show(200);
		prw.EnableCancelButton(true);

		prw.SetPercent(progId, 0.05);
		if(prw.IsCanceled()==true)
		{
			prw.SetPercent(progId,1.0);
			prw.EnableCancelButton(false);
			resTable.DeleteRow(0,resTable.GetRowCount());
			return false;
		}
		bool result = false;
		if(vecK.size() > 0 && vecF.size() > 0)
		{		
			result = StatFundIssueClose(resTable,vecF,iStartDate,iEndDate,bGetAllDate,wIssueType,FXRQ);
			bool resultOpen = false;
			if (wIssueType >1)
			{
//#ifdef _SECURITYID_FOR_STAT_
				resultOpen = StatFundIssueOpen_New(resTableOpen,vecK,iStartDate,iEndDate,bGetAllDate,FXRQ);
//#else
//				resultOpen = StatFundIssueOpen(resTableOpen,vecK,iStartDate,iEndDate,bGetAllDate,FXRQ);
//#endif
				//resultOpen = StatFundIssueOpen(resTableOpen,vecK,iStartDate,iEndDate,bGetAllDate,FXRQ);			
				prw.SetPercent(progId, 0.6);
			}
			if(result == false && resultOpen == false)
			{
				prw.SetPercent(progId, 1.0);
				sProgressPrompt += _T(",完成!");
				prw.SetText(progId,sProgressPrompt);
				prw.EnableCancelButton(false);
				return false;
			}
			if(result == false && resultOpen == true)
			{
				resTable.Clear();
				resTable.CopyColumnInfoFrom(resTableOpen);
				resTable.Clone(resTableOpen);
				prw.SetPercent(progId, 1.0);
				sProgressPrompt += _T(",完成!");
				prw.SetText(progId,sProgressPrompt);
				prw.EnableCancelButton(false);
				return true;
			}
			if(result == true && resultOpen == false)
			{
				prw.SetPercent(progId, 1.0);
				sProgressPrompt += _T(",完成!");
				prw.SetText(progId,sProgressPrompt);
				prw.EnableCancelButton(false);
				return true;
			}	
		}
		else if(vecK.size() > 0 && vecF.size() == 0)
		{
			if (wIssueType >1)//开放式基金的类型全部按“首次发行”，所以要加上这一判断。
			{
//#ifdef _SECURITYID_FOR_STAT_
				result = StatFundIssueOpen_New(resTable,vecK,iStartDate,iEndDate,bGetAllDate,FXRQ);
//#else
//				result = StatFundIssueOpen(resTable,vecK,iStartDate,iEndDate,bGetAllDate,FXRQ);
//#endif
				
				if(result == false)
				{
					prw.SetPercent(progId, 1.0);
					return false;
				}
				prw.SetPercent(progId, 0.6);
			}
			else
			{
				prw.SetPercent(progId, 1.0);
				return false;
			}
		}
        else if(vecK.size() == 0 && vecF.size() > 0)
		{
			result = StatFundIssueClose(resTable,vecF,iStartDate,iEndDate,bGetAllDate,wIssueType,FXRQ);
			if(result == false)
			{
				prw.SetPercent(progId, 1.0);
				return false;
			}
			prw.SetPercent(progId, 0.6);
		}
		else
			return false;
        int iRCount = resTableOpen.GetRowCount();
		Tx::Core::VariantData id,name,code,type,IssueType,IssueDate;
        Tx::Core::VariantData MarketDate,IssuePrice,CollectFund;
		Tx::Core::VariantData IssueAccount,IssueValue;
		double DInvalidate;
		DInvalidate = Tx::Core::Con_doubleInvalid;
		int icount = resTable.GetRowCount();
		for(int i = 0;i < iRCount;i++)
		{
			resTable.AddRow();
			resTableOpen.GetCell(0,i,id);
			resTableOpen.GetCell(1,i,name);
			resTableOpen.GetCell(2,i,code);
			resTableOpen.GetCell(3,i,type);
			resTableOpen.GetCell(4,i,IssueType);
			resTableOpen.GetCell(5,i,IssueDate);
			resTableOpen.GetCell(6,i,MarketDate);
			resTableOpen.GetCell(7,i,IssuePrice);
			resTableOpen.GetCell(8,i,IssueAccount);
			resTableOpen.GetCell(9,i,IssueValue);
			resTableOpen.GetCell(10,i,CollectFund);
//			resTableOpen.GetCell(11,i,IssueFee);
			resTable.SetCell(0,i + icount,id);
			resTable.SetCell(1,i + icount,name);
			resTable.SetCell(2,i + icount,code);
			resTable.SetCell(3,i + icount,type);
			resTable.SetCell(4,i + icount,IssueType);
			resTable.SetCell(5,i + icount,IssueDate);
			resTable.SetCell(6,i + icount,MarketDate);
			resTable.SetCell(7,i + icount,IssuePrice);
			resTable.SetCell(8,i + icount,IssueAccount);
			resTable.SetCell(9,i + icount,IssueValue);
			resTable.SetCell(10,i + icount,CollectFund);
			resTable.SetCell(11,i + icount,DInvalidate);
		}
		//step5
		prw.SetPercent(progId, 1.0);
		sProgressPrompt += _T(",完成!");
		prw.SetText(progId,sProgressPrompt);
		prw.EnableCancelButton(false);
		return true;
	}//end of StatFundIssue


	//封闭式基金发行
	//modify by lijw 2008-02-04
	bool	TxFund::StatFundIssueClose(
		Tx::Core::Table_Indicator	&resTable,
		std::vector<int>	&iSecurityId,
		int		iStartDate,
		int		iEndDate,
		bool	bGetAllDate,
		WORD	wIssueType,			//wIssueType是发行类型：	0x0001首发、0x0002扩募
		bool    FXRQ
		)
	{
		//实体id-〉基金id
		std::vector<int>	iFundId;	//基金id
		std::vector<CString>	iType;		//解析存储数据类型标志位
//		this->TransObjectToSecIns(iSecurityId,iFundId,1);
		std::vector<int>::iterator iterV;
		int tempId;
		std::vector<int> tempTradeV;
		iterV = iSecurityId.begin();
		for (;iterV != iSecurityId.end();++iterV)
		{
			GetSecurityNow(*iterV);
			if(m_pSecurity != NULL)
			{
				tempId = m_pSecurity->GetSecurity1Id();
				if (find(iFundId.begin(),iFundId.end(),tempId) == iFundId.end())
				{
					iFundId.push_back(tempId);
					tempTradeV.push_back(*iterV);
				}
			}
		}
		iSecurityId.clear();
		iSecurityId.assign(tempTradeV.begin(),tempTradeV.end());
		//准备样本集=第一参数列:F_FUND_ID,int型
		resTable.AddParameterColumn(Tx::Core::dtype_int4);
		//公告日期
		resTable.AddParameterColumn(Tx::Core::dtype_int4);
		const int indicatorIndex = 9;
		long iIndicator[indicatorIndex] = 
		{
			30301059,		//类型
		    30301058,		//发行方式：
			30301056,		//发行日期：
			30301066,		//上市日期：
			30301062,		//发行价：
			30301060,		//发行量[股]：
			30301064,		//发行市值[元]：
			30301065,		//募集资金[元]：
			30301063,		//发行费用[元]：
		};
		UINT varCfg[2];			//参数配置
		int varCount=2;			//参数个数
		bool result;
		for (int i = 0; i < indicatorIndex; i++)
		{
			int tempIndicator = iIndicator[i];
			GetIndicatorDataNow(tempIndicator);
			if (m_pIndicatorData==NULL)
				return false; 
			varCfg[0]=0;
			varCfg[1]=1;
			result = m_pLogicalBusiness->SetIndicatorIntoTable(
				m_pIndicatorData,	//指标
				varCfg,				//参数配置
				varCount,			//参数个数
				resTable	//计算需要的参数传输载体以及计算后结果的载体
				);
			if(result==false)
			{
				return FALSE;
			}

		}
		result = m_pLogicalBusiness->GetData(resTable,true);
		if(result==false)
			return false;
#ifdef _DEBUG
		CString strTable1=resTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable1);
#endif
		UINT* nColArray = new UINT[resTable.GetColCount()];

		for(UINT i=0;i<resTable.GetColCount();i++)
			nColArray[i]=i;
		Tx::Core::Table_Indicator	resTableType,resTableDate;

		resTableType.CopyColumnInfoFrom(resTable);
		resTableDate.CopyColumnInfoFrom(resTable);
  		if(resTable.GetRowCount()>0)
		{
			if(0 < wIssueType && wIssueType<= 3)
			{
				//目前没有扩募的数据，而且有些基金是没有通过发行上市的，所以这里的扩募实际上是"清理规范"和未指定类型的数据
				if((wIssueType==3)&&(resTable.GetRowCount()!=0))
					resTableType.Clone(resTable);
				else
				{
					if(wIssueType&2)
						iType.push_back(_T("首次发行"));
					else//这是wIssueType&1
					{
						iType.push_back(_T(""));
						iType.push_back(_T("清理规范"));
					}

				   resTable.EqualsAt(resTableType,nColArray,resTable.GetColCount(),3,iType);
				}
				
			}
		
			//数据在resTable中做调整以及筛选
			if(resTableType.GetRowCount()>0)
			{
				//日期筛选
				if(!bGetAllDate)
				{
					if(FXRQ)
					{
						//按发行日期筛选
						resTableType.Between(resTableDate,nColArray,resTable.GetColCount(),4,iStartDate,iEndDate,true,true);
					}
					else
					{
						//按上市日期筛选
						resTableType.Between(resTableDate,nColArray,resTable.GetColCount(),5,iStartDate,iEndDate,true,true);
					}

				}
				else
				{
					resTableDate.Clone(resTableType);
				}

				//增列此列是为了增加交易实体ID
				resTableDate.InsertCol(1,Tx::Core::dtype_int4);
				//加入两列,是为了增加名称和外码.
				resTableDate.InsertCol(2,Tx::Core::dtype_val_string);
				resTableDate.InsertCol(3,Tx::Core::dtype_val_string);
				resTable.Clear();
				resTable.CopyColumnInfoFrom(resTableDate);
				LONG nColCount2 = resTableDate.GetColCount();
				UINT* nColArray2 = new UINT[nColCount2];
				for(int i = 0; i < nColCount2; i++)
				{
					nColArray2[i] = i;
				}
				std::vector<int>	iFundId;	//暂存基金id
				//在程序内部转换样本ID
				for(std::vector<int>::iterator iter=iSecurityId.begin();iter!=iSecurityId.end();iter++)
				{
					//取得交易实体ID
					int TradeID = *iter;
					GetSecurityNow(*iter);
					if(m_pSecurity==NULL)
						continue;
					//取得基金ID
					int tempInstitutionid = (int)m_pSecurity->GetSecurity1Id();
					iFundId.push_back(tempInstitutionid);
					//根据交易实体ID取得样本的名称和外码；
					CString strName,strCode;
					strName = m_pSecurity->GetName();
					strCode = m_pSecurity->GetCode();
					//把机构ID和交易实体ID对应起来。并且把交易实体ID放到表里。
					std::vector<UINT> vecInstiID;
					resTableDate.Find(0,tempInstitutionid,vecInstiID);
					std::vector<UINT>::iterator iteID;
					for(iteID = vecInstiID.begin();iteID != vecInstiID.end();++iteID)
					{

						resTableDate.SetCell(1,*iteID,TradeID);
						resTableDate.SetCell(2,*iteID,strName);
						resTableDate.SetCell(3,*iteID,strCode);
						//处理发行价的无效值。
						double IssuePrice;
						resTableDate.GetCell(9,*iteID,IssuePrice);
					}
				}
				resTableDate.EqualsAt(resTable,nColArray2,nColCount2,0,iFundId);
				delete nColArray2;
				nColArray2 = NULL;
			}
			else
			{
				delete nColArray;
				nColArray = NULL;
				return false;
			}
		}
		//之所以写下面的代码，是为了处理当没有取得数据时，不至于它与开放式基金的列数不一致
		else
		{
			delete nColArray;
			nColArray = NULL;
			return false;
		}
		delete nColArray;
		nColArray = NULL;
	    resTable.DeleteCol(4);
		resTable.DeleteCol(0);
		return true;
	}//end of StatFundIssueClose

//add by lijw
    //开放式基金发行
	bool TxFund::StatFundIssueOpen(
		Tx::Core::Table_Indicator	&resTable,
		std::vector<int>	&iSecurityId,
		int		iStartDate,
		int		iEndDate,
		bool	bGetAllDate,
		bool    FXRQ
		)
	{
		//下面是取基金基本表的数据
		//实体id-〉基金id
		std::vector<int>	iFundId;	//基金id
//		this->TransObjectToSecIns(iSecurityId,iFundId,1);
		std::vector<int>::iterator iterV;
		int tempId;
		std::vector<int> tempTradeV;
		iterV = iSecurityId.begin();
		for (;iterV != iSecurityId.end();++iterV)
		{
			GetSecurityNow(*iterV);
			if(m_pSecurity != NULL)
			{
				tempId = m_pSecurity->GetSecurity1Id(*iterV);
				if (find(iFundId.begin(),iFundId.end(),tempId) == iFundId.end())
				{
					iFundId.push_back(tempId);
					tempTradeV.push_back(*iterV);
				}
			}
		}

		iSecurityId.clear();
		iSecurityId.assign(tempTradeV.begin(),tempTradeV.end());
		Tx::Core::Table_Indicator	m_txTable,TempTable;
		//准备样本集=第一参数列:F_FUND_ID,int型
		m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
		const int indicatorIndex = 3;
		long iIndicator[indicatorIndex] = 
		{
			30001040,	//发行方式
			30001022,	//设立日期也就是界面上的上市日期，老系统就是那样
			30001038	//发行价
		};
		UINT varCfg[1];			//参数配置
		int varCount=1;			//参数个数
		bool result;
		for (int i = 0; i < indicatorIndex; i++)
		{
			int tempIndicator = iIndicator[i];
			GetIndicatorDataNow(tempIndicator);
			if (m_pIndicatorData==NULL)
				return false; 
			varCfg[0]=0;
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
		result = m_pLogicalBusiness->GetData(m_txTable,true);
		if(result==false)
			return false;
#ifdef _DEBUG
		CString strTable1=m_txTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable1);
#endif
		TempTable.CopyColumnInfoFrom(m_txTable);
		UINT icount = m_txTable.GetColCount();
		UINT* nColArray1 = new UINT[icount];
		for(UINT i=0;i<icount;i++)
			nColArray1[i]=i;
		if(m_txTable.GetRowCount()>0)
		{
			//日期筛选
			if(!bGetAllDate)
			{
				if(!FXRQ)
				{
					//按上市日期筛选
					m_txTable.Between(TempTable,nColArray1,icount,2,iStartDate,iEndDate,true,true);
				}
				else//当数据是按另外一张表里的发行日期进行统计时。
					TempTable.Clone(m_txTable);
			}
			//取得全部日期的数据。
			else
			{
				TempTable.Clone(m_txTable);
			}
			//类型筛选 //注释：基金的类型全部按首次发行
			m_txTable.Clear();
			m_txTable.CopyColumnInfoFrom(TempTable);
			//TempTable.EqualsAt(m_txTable,nColArray1,icount,0,iFundId);
			TempTable.EqualsAt(m_txTable,nColArray1,icount,0,iSecurityId);

		}
        delete nColArray1;
		nColArray1 = NULL;
		for (int j = 0;j < (int)m_txTable.GetRowCount();j++)
		{
			double iprice;
			m_txTable.GetCell(3,j,iprice);
			if (iprice < 0)
			{
				iprice = Tx::Core::Con_intInvalid;
				m_txTable.SetCell(3,j,iprice);
			}
		}
#ifdef _DEBUG
		CString strTable4=m_txTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable4);
#endif
		////从另外的一张表里取数据。
		//m_FundStype.clear();
		//m_IWPA.clear();
		//std::vector<CString>	iType;		//解析存储数据类型标志位
		////iPara是指标组，指标必须在同一数据库表中
		//std::vector<int> iParameter;
		//iParameter.push_back(30301080);		//发行日期
		////iParameter.push_back(30301082);		//基金净销售额
		//iParameter.push_back(30301084);		//合计募集金额

		////设定这里需要的指标
		//Tx::Business::IndicatorFile::GetInstance()->SetIWAP(m_IWPA,iParameter);
		//Tx::Business::IndicatorFile::GetInstance()->GetData(m_IWPA,iFundId,true);

		//Tx::Core::Table_Indicator	resTableDate;
		//if(m_IWPA.m_table_indicator.GetRowCount()!=0)
		//	resTable.Clone(m_IWPA.m_table_indicator);
		//else
		//	resTable.CopyColumnInfoFrom(m_IWPA.m_table_indicator);

		//resTableDate.CopyColumnInfoFrom(m_IWPA.m_table_indicator);

		//UINT* nColArray = new UINT[m_IWPA.m_table_indicator.GetColCount()];
		//for(UINT i=0;i<m_IWPA.m_table_indicator.GetColCount();i++)
		//	nColArray[i]=i;

		//从开放式基金表里取数据
		Tx::Core::Table_Indicator	resTableDate;//辅助表
		//准备样本集=第一参数列:F_FUND_ID,int型
		resTableDate.AddParameterColumn(Tx::Core::dtype_int4);
		//公告日期
		resTableDate.AddParameterColumn(Tx::Core::dtype_int4);
		
		const int indicatorIndex2 = 2;
		long iIndicator2[indicatorIndex2] = 
		{
			30301080,	//发行日期
			30301084,	//合计募集金额
		};
		UINT varCfg2[2];			//参数配置
		int varCount2=2;			//参数个数
		bool result2;
		for (int i = 0; i < indicatorIndex2; i++)
		{
			int tempIndicator = iIndicator2[i];
			GetIndicatorDataNow(tempIndicator);
			if (m_pIndicatorData==NULL)
				return false; 
			varCfg2[0]=0;
			varCfg2[1]=1;
			result2 = m_pLogicalBusiness->SetIndicatorIntoTable(
				m_pIndicatorData,	//指标
				varCfg2,				//参数配置
				varCount2,			//参数个数
				resTableDate	//计算需要的参数传输载体以及计算后结果的载体
				);
			if(result2==false)
			{
				return FALSE;
			}

		}
		result2 = m_pLogicalBusiness->GetData(resTableDate,true);
		if(result2==false)
			return false;
#ifdef _DEBUG
		CString strTable2=resTableDate.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable2);
#endif
		UINT icount2 = resTableDate.GetColCount();
		UINT* nColArray2 = new UINT[icount2];
		for(UINT i=0;i<icount2;i++)
			nColArray2[i]=i;   
		resTable.CopyColumnInfoFrom(resTableDate);
		if(resTableDate.GetRowCount()>0)
		{
			//日期筛选
			if(!bGetAllDate)
			{
				if(FXRQ)
				{
					//按发行日期筛选
					resTableDate.Between(resTable,nColArray2,icount2,2,iStartDate,iEndDate,true,true);
				}
				else//当数据是按另外一张表里的上市日期进行统计时。
					resTable.Clone(resTableDate);
			}
			else
			{
				resTable.Clone(resTableDate);
			}
			resTableDate.Clear();
			resTableDate.CopyColumnInfoFrom(resTable);
			//resTable.EqualsAt(resTableDate,nColArray2,icount2,0,iFundId);
			resTable.EqualsAt(resTableDate,nColArray2,icount2,0,iSecurityId);

		}
#ifdef _DEBUG
		CString strTableTmp=resTableDate.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTableTmp);
#endif
		if(resTableDate.GetRowCount() == 0)
		{
			delete nColArray2;
			nColArray2 = NULL;
			return false;
		}
		delete nColArray2;
		nColArray2 = NULL;
        resTable.Clear();
		resTable.CopyColumnInfoFrom(resTableDate);
		resTable.Clone(resTableDate);
		//把resTable和m_txTable连接起来。
		//增列此列是为了增加交易实体ID
		resTable.InsertCol(1,Tx::Core::dtype_int4);
		//加入两列,是为了增加名称和外码.
		resTable.InsertCol(2,Tx::Core::dtype_val_string);
		resTable.InsertCol(3,Tx::Core::dtype_val_string);
		//类型
		resTable.InsertCol(4,Tx::Core::dtype_val_string);
		//发行方式
		resTable.InsertCol(5,Tx::Core::dtype_val_string);
		//上市日期
		resTable.InsertCol(8,Tx::Core::dtype_int4);
		//发行价
		resTable.InsertCol(9,Tx::Core::dtype_double);
		//发行量
		resTable.InsertCol(10,Tx::Core::dtype_double);
		//发行市值
		resTable.InsertCol(11,Tx::Core::dtype_double);
		//发行费用
		resTable.AddCol(Tx::Core::dtype_val_string);
		for(int k= 0;k < (int)m_txTable.GetRowCount();++k)
		{

			int txID;
			m_txTable.GetCell(0,k,txID);
			std::vector<UINT> vecInstiID;
			resTable.Find(0,txID,vecInstiID);
			std::vector<UINT>::iterator iteID;
			CString strType;
			int MarketDate;
			double IssuePrice;
			for(iteID = vecInstiID.begin();iteID != vecInstiID.end();++iteID)
			{
				m_txTable.GetCell(1,k,strType);
				m_txTable.GetCell(2,k,MarketDate);
				m_txTable.GetCell(3,k,IssuePrice);
				if(IssuePrice < 0)
					IssuePrice = Tx::Core::Con_doubleInvalid;
				resTable.SetCell(5,*iteID,strType);
				resTable.SetCell(8,*iteID,MarketDate);
				resTable.SetCell(9,*iteID,IssuePrice);
               
			}
		}
		std::vector<int>::iterator iterTemp;
		for(iterTemp = iSecurityId.begin();iterTemp != iSecurityId.end();++iterTemp)
		{
			//取得交易实体ID
			int TradeID = *iterTemp;
			GetSecurityNow(TradeID);
			if(m_pSecurity==NULL)
				continue;
			//取得基金ID
			int tempInstitutionid = (int)m_pSecurity->GetSecurity1Id();
			//根据交易实体ID取得样本的名称和外码；
			CString strName,strCode;
			strName = m_pSecurity->GetName();
			strCode = m_pSecurity->GetCode();
            CString strInvidate = _T("-");
			std::vector<UINT> vecInstiID2;
			resTable.Find(0,tempInstitutionid,vecInstiID2);
			std::vector<UINT>::iterator iteID2;
			double dMuJi;
			CString m_strFXL;
			CString m_strFXSZ;
			CString m_strEnd;
			for(iteID2 = vecInstiID2.begin();iteID2 != vecInstiID2.end();++iteID2)
			{
				resTableDate.GetCell(3,*iteID2,dMuJi);
				resTable.SetCell(4,*iteID2,(CString)_T("首次发行"));
				resTable.SetCell(1,*iteID2,TradeID);
				resTable.SetCell(2,*iteID2,strName);
				resTable.SetCell(3,*iteID2,strCode);
				//if(dMuJi < 0.0 || dMuJi == Con_doubleInvalid)
				//{
				//	m_strFXSZ = strInvidate;
				//	m_strFXL = strInvidate;
				//}
				//else
				//{
				//	m_strEnd.Format("%f",dMuJi);
				//	int i = m_strEnd.GetLength();
				//	m_strFXL = m_strEnd.Mid(0,i-7);
				//	m_strFXSZ = m_strEnd.Mid(0,i-4);
				//	//m_strFXSZ.Format("%2f",dMuJi);
				//}
				resTable.SetCell(10,*iteID2,dMuJi);
				resTable.SetCell(11,*iteID2,dMuJi);
				resTable.SetCell(13,*iteID2,strInvidate);
			}
		}
		resTable.DeleteCol(6);
		resTable.DeleteCol(0);

		// add by wangyc 暂时不更改上面的过程  begin
		//增加处理，增加一个过滤的过程，针对上市日期和发行日期的过滤
		if(!bGetAllDate)
		{
			Tx::Core::Table_Indicator	resTableTemp;//辅助表
			UINT icount3 = resTable.GetColCount();
			UINT* nColArray3 = new UINT[icount3];
			for(UINT i=0;i<icount3;i++)
				nColArray3[i]=i;  
			resTableTemp.CopyColumnInfoFrom(resTable);
			resTableTemp.Clone(resTable);
			resTable.Clear();
			resTable.CopyColumnInfoFrom(resTableTemp);
			if (FXRQ)
			{
				resTableTemp.Between(resTable,nColArray3,icount3,5,iStartDate,iEndDate,true,true);
			} 
			else
			{
				resTableTemp.Between(resTable,nColArray3,icount3,6,iStartDate,iEndDate,true,true);
			}
		}
		// add by wangyc 暂时不更改上面的过程  end
		//增加处理，增加一个过滤的过程，针对上市日期和发行日期的过滤

		return true;
	}//end of StatFundIssueOpen

	//added by zhangxs 20101130
//开放式发行==新库
bool TxFund::StatFundIssueOpen_New(
								   Tx::Core::Table_Indicator	&resTable,
								   std::vector<int>	&iSecurityId,
									int		iStartDate,
									int		iEndDate,
									bool	bGetAllDate,
									bool    FXRQ 
								)
{
	//下面是取基金基本表的数据
	//step1 取交易实体对应的券id
	std::vector<int> iFundId;
	iFundId.clear();
	int tempId;
	std::vector<int>::iterator iterV;
	std::vector<int> tempTradeV;	//用于筛选非法交易实体
	iterV = iSecurityId.begin();
	for (;iterV != iSecurityId.end();++iterV)
	{
		GetSecurityNow(*iterV);
		if(m_pSecurity != NULL)
		{
			tempId = m_pSecurity->GetSecurity1Id(*iterV);
			if (find(iFundId.begin(),iFundId.end(),tempId) == iFundId.end())
			{
				iFundId.push_back(tempId);
				tempTradeV.push_back(*iterV);
			}
		}
	}

	iSecurityId.clear();
	iSecurityId.assign(tempTradeV.begin(),tempTradeV.end());
	//step2.1 取30001040,	//发行方式等指标，新库用的是交易实体id，老库使用的券id
	Tx::Core::Table_Indicator	m_txTable,TempTable;
	//准备样本集=第一参数列:F_FUND_ID,int型
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
	const int indicatorIndex = 3;
	long iIndicator[indicatorIndex] = 
	{
		30001040,	//发行方式
		30001022,	//设立日期也就是界面上的上市日期，老系统就是那样
		30001038	//发行价
	};
	UINT varCfg[1];			//参数配置
	int varCount=1;			//参数个数
	bool result;
	for (int i = 0; i < indicatorIndex; i++)
	{
		int tempIndicator = iIndicator[i];
		GetIndicatorDataNow(tempIndicator);
		if (m_pIndicatorData==NULL)
			return false; 
		varCfg[0]=0;
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
	result = m_pLogicalBusiness->GetData(m_txTable,true);
	if(result==false)
		return false;
	CString strTable1;
	CString strTable2;
#ifdef _DEBUG
	strTable1=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable1);
#endif
	//step2.2 取30001040,	//发行方式等指标筛选
	TempTable.CopyColumnInfoFrom(m_txTable);
	UINT icount = m_txTable.GetColCount();
	UINT* nColArray1 = new UINT[icount];
	for(UINT i=0;i<icount;i++)
		nColArray1[i]=i;
	if(m_txTable.GetRowCount()>0)
	{
		//日期筛选
		if(bGetAllDate || FXRQ )
		{
			TempTable.Clone(m_txTable);
		}
		else
		{
			//按上市日期筛选
			m_txTable.Between(TempTable,nColArray1,icount,2,iStartDate,iEndDate,true,true);
		}
		//类型筛选 //注释：基金的类型全部按首次发行
		m_txTable.Clear();
		m_txTable.CopyColumnInfoFrom(TempTable);
		TempTable.EqualsAt(m_txTable,nColArray1,icount,0,iSecurityId);

	}
	delete nColArray1;
	nColArray1 = NULL;
	for (int j = 0;j < (int)m_txTable.GetRowCount();j++)
	{
		double iprice;
		m_txTable.GetCell(3,j,iprice);
		if (iprice < 0)
		{
			iprice = Tx::Core::Con_intInvalid;
			m_txTable.SetCell(3,j,iprice);
		}
	}
#ifdef _DEBUG
	strTable1=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable1);
#endif
	//step3.1 30301080,	//发行日期等指标，使用的是券id
	//从开放式基金表里取数据
	Tx::Core::Table_Indicator	resTableDate;//辅助表
	//准备样本集=第一参数列:F_FUND_ID,int型
	resTableDate.AddParameterColumn(Tx::Core::dtype_int4);
	//公告日期
	resTableDate.AddParameterColumn(Tx::Core::dtype_int4);

	const int indicatorIndex2 = 2;
	long iIndicator2[indicatorIndex2] = 
	{
		30301080,	//发行日期
		30301084,	//合计募集金额
	};
	UINT varCfg2[2];			//参数配置
	int varCount2=2;			//参数个数
	bool result2;
	for (int i = 0; i < indicatorIndex2; i++)
	{
		int tempIndicator = iIndicator2[i];
		GetIndicatorDataNow(tempIndicator);
		if (m_pIndicatorData==NULL)
			return false; 
		varCfg2[0]=0;
		varCfg2[1]=1;
		result2 = m_pLogicalBusiness->SetIndicatorIntoTable(
			m_pIndicatorData,	//指标
			varCfg2,				//参数配置
			varCount2,			//参数个数
			resTableDate	//计算需要的参数传输载体以及计算后结果的载体
			);
		if(result2==false)
		{
			return FALSE;
		}
	}
	result2 = m_pLogicalBusiness->GetData(resTableDate,true);
	if(result2==false)
		return false;
#ifdef _DEBUG
	strTable2=resTableDate.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable2);
#endif
	//step3.2 30301080,	//发行日期等指标，筛选
	UINT icount2 = resTableDate.GetColCount();
	UINT* nColArray2 = new UINT[icount2];
	for(UINT i=0;i<icount2;i++)
		nColArray2[i]=i;   
	resTable.CopyColumnInfoFrom(resTableDate);
	if(resTableDate.GetRowCount()>0)
	{
		//日期筛选
		if(!bGetAllDate && FXRQ)
		{			
			//按发行日期筛选
			resTableDate.Between(resTable,nColArray2,icount2,2,iStartDate,iEndDate,true,true);
		}
		else
		{
			resTable.Clone(resTableDate);
		}
		resTableDate.Clear();
		resTableDate.CopyColumnInfoFrom(resTable);
		resTable.EqualsAt(resTableDate,nColArray2,icount2,0,iFundId);
	}
#ifdef _DEBUG
	strTable2=resTableDate.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable2);
#endif
	if(resTableDate.GetRowCount() == 0)
	{
		delete nColArray2;
		nColArray2 = NULL;
		return false;
	}
	delete nColArray2;
	nColArray2 = NULL;
	resTable.Clear();
	resTable.CopyColumnInfoFrom(resTableDate);
	resTable.Clone(resTableDate);
	//step4 拼接两个表
	//把resTable和m_txTable连接起来。
	//增列此列是为了增加交易实体ID
	resTable.InsertCol(1,Tx::Core::dtype_int4);
	//加入两列,是为了增加名称和外码.
	resTable.InsertCol(2,Tx::Core::dtype_val_string);
	resTable.InsertCol(3,Tx::Core::dtype_val_string);
	//类型
	resTable.InsertCol(4,Tx::Core::dtype_val_string);
	//发行方式
	resTable.InsertCol(5,Tx::Core::dtype_val_string);
	//上市日期
	resTable.InsertCol(8,Tx::Core::dtype_int4);
	//发行价
	resTable.InsertCol(9,Tx::Core::dtype_double);
	//发行量
	resTable.InsertCol(10,Tx::Core::dtype_double);
	//发行市值
	resTable.InsertCol(11,Tx::Core::dtype_double);
	//发行费用
	resTable.AddCol(Tx::Core::dtype_val_string);
	for(int k= 0;k < (int)m_txTable.GetRowCount();++k)
	{

		int txID;
		m_txTable.GetCell(0,k,txID);
		GetSecurityNow(txID);
		if(m_pSecurity==NULL)
			continue;
		int iFundIdTmp = m_pSecurity->GetSecurity1Id();
		std::vector<UINT> vecInstiID;
		resTable.Find(0,iFundIdTmp,vecInstiID);
		std::vector<UINT>::iterator iteID;
		CString strType;
		int MarketDate;
		double IssuePrice;
		for(iteID = vecInstiID.begin();iteID != vecInstiID.end();++iteID)
		{
			m_txTable.GetCell(1,k,strType);
			m_txTable.GetCell(2,k,MarketDate);
			m_txTable.GetCell(3,k,IssuePrice);
			if(IssuePrice < 0)
				IssuePrice = Tx::Core::Con_doubleInvalid;
			resTable.SetCell(5,*iteID,strType);
			resTable.SetCell(8,*iteID,MarketDate);
			resTable.SetCell(9,*iteID,IssuePrice);

		}
	}
	std::vector<int>::iterator iterTemp;
	for(iterTemp = iSecurityId.begin();iterTemp != iSecurityId.end();++iterTemp)
	{
		//取得交易实体ID
		int TradeID = *iterTemp;
		GetSecurityNow(TradeID);
		if(m_pSecurity==NULL)
			continue;
		int iFundIdTmp = m_pSecurity->GetSecurity1Id();
		//根据交易实体ID取得样本的名称和外码；
		CString strName,strCode;
		strName = m_pSecurity->GetName();
		strCode = m_pSecurity->GetCode();
		CString strInvidate = _T("-");
		double dMuJi;
		CString m_strFXL;
		CString m_strFXSZ;
		CString m_strEnd;

		std::vector<UINT> vecInstiID;
		resTableDate.Find(0,iFundIdTmp,vecInstiID);
		std::vector<UINT>::iterator iteID;
		for(iteID = vecInstiID.begin();iteID != vecInstiID.end();++iteID)
		{
			resTableDate.GetCell(3,*iteID,dMuJi);
			resTable.SetCell(4,*iteID,(CString)_T("首次发行"));
			resTable.SetCell(1,*iteID,TradeID);
			resTable.SetCell(2,*iteID,strName);
			resTable.SetCell(3,*iteID,strCode);
			resTable.SetCell(10,*iteID,dMuJi);
			resTable.SetCell(11,*iteID,dMuJi);
			resTable.SetCell(13,*iteID,strInvidate);
		}
	}
	resTable.DeleteCol(6);
	resTable.DeleteCol(0);

	// step5 暂时不更改上面的过程  begin
	//增加处理，增加一个过滤的过程，针对上市日期和发行日期的过滤
	if(!bGetAllDate)
	{
		Tx::Core::Table_Indicator	resTableTemp;//辅助表
		UINT icount3 = resTable.GetColCount();
		UINT* nColArray3 = new UINT[icount3];
		for(UINT i=0;i<icount3;i++)
			nColArray3[i]=i;  
		resTableTemp.CopyColumnInfoFrom(resTable);
		resTableTemp.Clone(resTable);
		resTable.Clear();
		resTable.CopyColumnInfoFrom(resTableTemp);
		if (FXRQ)
		{
			resTableTemp.Between(resTable,nColArray3,icount3,5,iStartDate,iEndDate,true,true);
		} 
		else
		{
			resTableTemp.Between(resTable,nColArray3,icount3,6,iStartDate,iEndDate,true,true);
		}
	}
	return true;
}
//add by lijw 2008-03-05
//基金公司概况
bool	TxFund::StatFundCompanySituation(
		Tx::Core::Table_Indicator	&resTable,
		std::vector<int>	&iSecurityId,	//传进来是公司ID
		int		iStartDate,
		int		iEndDate,
		bool	bGetAllDate		
		)	
{
	//添加进度条
//	ProgressWnd* pwd=Tx::Core::ProgressWnd::GetInstance();
	Tx::Core::ProgressWnd prw;
	UINT pid=prw.AddItem(1,_T("基金公司概况统计..."),0.0);
	prw.Show(1000);
	//默认的返回值状态
	bool result = true;

	//清空数据
	m_txTable.Clear();
	//准备样本集=第一参数列:基金公司ID,int型
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
	const int indicatorIndex = 16;
	long iIndicator[indicatorIndex] = 
	{
		30001231,	//基金公司简称
		30001003,	//成立时间	
		30001001,	//基金公司全称
		30001221,	//内资合资标志（也即是基金公司类型0：内资 1：合资）
		30001004,	//注册资本
		30001005,	//注册地址
		30001006,	//法人代表
		30001007,	//总经理
		30001014,	//经营范围
		30001008,	//办公地址
		30001009,	//公司网址
		30001010,	//Email
		30001011,	//咨询电话
		30001012,	//咨询传真
		30001013,	//咨询地址
		30001015	//公司简介
	};
	UINT varCfg[1];			//参数配置
	int varCount=1;			//参数个数
	for (int i = 0; i < indicatorIndex; i++)
	{
		int tempIndicator = iIndicator[i];

		GetIndicatorDataNow(tempIndicator);
		if (m_pIndicatorData==NULL)
		{ return false; }
		varCfg[0]=0;
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
		return false;
	//添加进度条
	prw.SetPercent(pid,0.3);
#ifdef _DEBUG
	CString strTable1=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable1);
#endif
	UINT iCol=m_txTable.GetColCount();
	//复制所有列信息
	resTable.CopyColumnInfoFrom(m_txTable);
	if(m_txTable.GetRowCount()==0)
	{
		//添加进度条
		prw.SetPercent(pid,1.0);
		return false;
	}
	UINT* nColArray = new UINT[iCol];
	for(UINT i=0;i<iCol;i++)
		nColArray[i]=i;
	//取得指定时间区间内的记录数据	
	if(m_txTable.GetRowCount()>0)
	{
		if(!bGetAllDate)
		{
			m_txTable.Between(resTable,nColArray,iCol,2,iStartDate,iEndDate,true,true);
		}
		else
		{
			resTable.Clone(m_txTable);	
		}
	}
	m_txTable.Clear();
	m_txTable.CopyColumnInfoFrom(resTable);
	std::vector<int>  iInstitutionId;
	std::vector<int>::iterator iter1;
	if(resTable.GetRowCount()>0)
	{
		resTable.EqualsAt(m_txTable,nColArray,iCol,0,iSecurityId);
	}
	if (m_txTable.GetRowCount() == 0)
	{
		delete nColArray;
		nColArray = NULL;
		//添加进度条
		prw.SetPercent(pid,1.0);
		return false;
	}
#ifdef _DEBUG
	strTable1=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable1);
#endif
	//为了把内资合资标志转化为公司类型而作准备
	m_txTable.InsertCol(5,Tx::Core::dtype_val_string);
	byte type;
	double EnrolCapital;
	for(iter1 = iSecurityId.begin();iter1!=iSecurityId.end();++iter1)
	{	
		//修改数据的展示方式
		std::vector<UINT> vecInstiID2;
		m_txTable.Find(0,*iter1,vecInstiID2);
		std::vector<UINT>::iterator iteID;
		int tempPosition;
		CString strType;
		for(iteID = vecInstiID2.begin();iteID != vecInstiID2.end();++iteID)
		{
			tempPosition = *iteID;
			m_txTable.GetCell(4,tempPosition,type);
			m_txTable.GetCell(6,tempPosition,EnrolCapital);
			EnrolCapital = EnrolCapital/100000000;
			m_txTable.SetCell(6,tempPosition,EnrolCapital);
			if(type == 0)
				strType = _T("内资");
			if(type == 1)
				strType = _T("合资");
            m_txTable.SetCell(5,tempPosition,strType);
		}

	}
	m_txTable.InsertCol(3,Tx::Core::dtype_int4);//为封闭式基金支数准备
	m_txTable.InsertCol(4,Tx::Core::dtype_double);//为封闭式基金最新规模准备
	m_txTable.InsertCol(5,Tx::Core::dtype_int4);//为开放式基金支数准备
	m_txTable.InsertCol(6,Tx::Core::dtype_double);//为开放式基金最新规模准备准备
	//从表T_FUND_COMPANY_BALANCE_SIZE取数据
	//清空数据
	resTable.Clear();
	//准备样本集=第一参数列:基金公司ID,int型
	resTable.AddParameterColumn(Tx::Core::dtype_int4);
	const int indicatorIndex2 = 4;
	long iIndicator2[indicatorIndex2] = 
	{
		30001229,	//封闭式基金只数
		30001227,	//封闭式基金规模
		30001230,	//开放式基金只数
		30001228	//开放式基金规模		
	};
	UINT varCfg2[1];			//参数配置
	int varCount2=1;			//参数个数
	for (int i = 0; i < indicatorIndex2; i++)
	{
		int tempIndicator = iIndicator2[i];

		GetIndicatorDataNow(tempIndicator);
		if (m_pIndicatorData==NULL)
		{
			delete nColArray;
			nColArray = NULL;
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false;
		}
		varCfg2[0]=0;
		result = m_pLogicalBusiness->SetIndicatorIntoTable(
			m_pIndicatorData,	//指标
			varCfg2,				//参数配置
			varCount2,			//参数个数
			resTable	//计算需要的参数传输载体以及计算后结果的载体
			);
		if(result==false)
		{
			delete nColArray;
			nColArray = NULL;
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false;
		}
	}
	//根据之前3个步骤的设置进行数据读取，结果数据存放在table中
	result = m_pLogicalBusiness->GetData(resTable,true);
	if(result==false)
	{
		delete nColArray;
		nColArray = NULL;
		//添加进度条
		prw.SetPercent(pid,1.0);
		return false;
	}
	iCol = resTable.GetColCount();
	nColArray = new UINT[iCol];
	for(UINT i=0;i<iCol;i++)
		nColArray[i]=i;
	Tx::Core::Table_Indicator tempTable;
	tempTable.CopyColumnInfoFrom(resTable);
	if(resTable.GetRowCount() != 0)
	{
		resTable.EqualsAt(tempTable,nColArray,iCol,0,iSecurityId);
#ifdef _DEBUG
		CString strTable1=resTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable1);
#endif
		int iRow = tempTable.GetRowCount();
		int  iData1,iData2;
		double dData3,dData4;
		int iCompanyId;
		std::vector<UINT> positionVector;
		std::vector<UINT>::iterator Uter;
		for(int i = 0;i < iRow;i++)
		{
			tempTable.GetCell(0,i,iCompanyId);
			tempTable.GetCell(1,i,iData1);
			tempTable.GetCell(2,i,dData3);
			dData3 = dData3/100000000;
			tempTable.GetCell(3,i,iData2);
			tempTable.GetCell(4,i,dData4);
			dData4 = dData4/100000000;
			if (!positionVector.empty())
			{
				positionVector.clear();
			}
			m_txTable.Find(0,iCompanyId,positionVector);
			for(Uter = positionVector.begin();Uter != positionVector.end();Uter++)
			{
				m_txTable.SetCell(3,*Uter,iData1);
				m_txTable.SetCell(4,*Uter,dData3);
				m_txTable.SetCell(5,*Uter,iData2);
				m_txTable.SetCell(6,*Uter,dData4);
			}			
		}
	}
#ifdef _DEBUG
	CString strTable3=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable3);
#endif
	m_txTable.DeleteCol(8);//删除内资合资标志
	resTable.Clear();
	resTable.Clone(m_txTable);
	delete nColArray;
	nColArray = NULL;
	//添加进度条
	prw.SetPercent(pid,1.0);
	return true;

}


// add by lijw 2008-03-05
//基金公司股东
bool	TxFund::StatFundHolder(
		Tx::Core::Table_Indicator	&resTable,
		std::vector<int>	&iSecurityId,          //传进来的就是基金公司的机构ID
		int		iStartDate,
		int		iEndDate,
		bool	bGetAllDate		
		)
{
	//添加进度条
//	ProgressWnd* pwd=Tx::Core::ProgressWnd::GetInstance();
	ProgressWnd prw;
	UINT pid=prw.AddItem(1,_T("基金公司股东统计..."),0.0);
	prw.Show(1000);
   	//默认的返回值状态
	bool result = true;
	//清空数据
	m_txTable.Clear();
	//准备样本集=第一参数列:基金公司ID,int型
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
	//日期参数=第二参数列;公告日期, int型
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
	//序号=第三参数列;F_No, int型
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
	const int indicatorIndex = 3;
	long iIndicator[indicatorIndex] = 
	{
		30601026,	//股东名称
		30601027,	//出资金额
		30601028	//股权比例
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
			return FALSE;
		}

	}
	//根据之前3个步骤的设置进行数据读取，结果数据存放在table中
	result = m_pLogicalBusiness->GetData(m_txTable,true);
	if(result==false)
		return false;
	//添加进度条
	prw.SetPercent(pid,0.3);
#ifdef _DEBUG
	CString strTable1=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable1);
#endif
	UINT iCol=m_txTable.GetColCount();
	//复制所有列信息
	resTable.CopyColumnInfoFrom(m_txTable);
	if(m_txTable.GetRowCount()==0)
	{
		//添加进度条
		prw.SetPercent(pid,1.0);
		return false;
	}

	UINT* nColArray = new UINT[iCol];
	for(UINT i=0;i<iCol;i++)
		nColArray[i]=i;

	//取得指定时间区间内的记录数据	
	if(m_txTable.GetRowCount()>0)
	{
		if(!bGetAllDate)
		{
			m_txTable.Between(resTable,nColArray,iCol,1,iStartDate,iEndDate,true,true);
		}
		else
		{
			resTable.Clone(m_txTable);	
		}
	}
	m_txTable.Clear();
	m_txTable.CopyColumnInfoFrom(resTable);
	//取得指定实体id(iSecurityId)的数据
	if(resTable.GetRowCount()>0)
	{
		resTable.EqualsAt(m_txTable,nColArray,iCol,0,iSecurityId);
	}
#ifdef _DEBUG
	strTable1=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable1);
#endif
	//插入这两列为了插入代码和名称。
	m_txTable.InsertCol(1,Tx::Core::dtype_val_string);//插入基金公司名称
	std::vector<int>::iterator iter1;
	std::unordered_map<int,CString>::iterator mapIter;
	std::unordered_map<int,CString> NameMap;
	TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_INSTITUTIONID_TO_SHORTNAME,NameMap);
//	double dData;
	std::vector<UINT> vecInstiID2;
	std::vector<UINT>::iterator iteID;
	for(iter1 = iSecurityId.begin();iter1!=iSecurityId.end();++iter1)
	{
		//取得公司机构ID
		int Institutionid1 = *iter1;
		//根据公司机构ID取得样本的名称
		CString strName;
		mapIter = NameMap.find(Institutionid1);
		if(mapIter != NameMap.end())
			strName = mapIter->second;
		if(!vecInstiID2.empty())
			vecInstiID2.clear();
		m_txTable.Find(0,Institutionid1,vecInstiID2);		
		for(iteID = vecInstiID2.begin();iteID != vecInstiID2.end();++iteID)
		{
			m_txTable.SetCell(1,*iteID,strName);
			/*m_txTable.GetCell(5,*iteID,dData);
			dData = dData/100000000;
			m_txTable.SetCell(5,*iteID,dData);*/
		}

	}
#ifdef _DEBUG
	CString strTable3=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable3);
#endif
	resTable.Clear();
	resTable.Clone(m_txTable);
	resTable.DeleteCol(3);//删除序号那一列；
	delete nColArray;
	nColArray = NULL;
	//添加进度条
	prw.SetPercent(pid,1.0);
	return true;

}


	// add by lijw 2008-03-04
	//基金经理
	bool	TxFund::StatFundManager(
		Tx::Core::Table_Indicator	&resTable,
		std::vector<int>	&iSecurityId,
		int		iStartDate,
		int		iEndDate,
		bool	bGetAllDate
		)
	{
		//添加进度条
		// ProgressWnd* pwd=Tx::Core::ProgressWnd::GetInstance();
		ProgressWnd prw;
		UINT pid=prw.AddItem(1,_T("基金经理统计..."),0.0);
		prw.Show(1000);

		//实体id-〉基金id
		/*std::vector<int>	iFundId;
		if(!TransObjectToSecIns(iSecurityId,iFundId,1))
			return false;		*/
		std::set<int> setSecurityId;
		std::set<int> SameFundId;
		std::set<int> DifferentID;
		std::vector<int>::iterator iter;
		for(iter = iSecurityId.begin();iter != iSecurityId.end();++iter)
		{
			GetSecurityNow(*iter);
			if(m_pSecurity == NULL)
				continue;
			int fundid =(int)m_pSecurity->GetSecurity1Id();
			if(setSecurityId.find(fundid) == setSecurityId.end())
			{
				setSecurityId.insert(fundid);
				DifferentID.insert(*iter);
			}
			else
				SameFundId.insert(*iter);
		}
		//默认的返回值状态。
		bool result = true;
		//清空数据
		m_txTable.Clear();

		//基金ID,int型
		m_txTable.AddParameterColumn(Tx::Core::dtype_int4);

		//公告日期 int型
		m_txTable.AddParameterColumn(Tx::Core::dtype_int4);

		// 基金经理的姓名，CString型
		m_txTable.AddParameterColumn(Tx::Core::dtype_val_string);

		UINT varCfg[3];			//参数配置
		int varCount=3;			//参数个数	交易实体id，公告日
		for(int i=0;i<3;i++)
			varCfg[i]=i;
		const int INDICATOR_INDEX=8;
		long iIndicator[INDICATOR_INDEX]=
		{			
			30601009,	//职位
			30601010,	//起始日期
			30601011,	//终止日期
			30601012,	//性别
			30601013,	//学历
			30601014,	//出生年份
			30601015,	//简历
			30601016	//备注
		};

		//设定指标列
		for (int i = 0; i <	INDICATOR_INDEX; i++)
		{
			GetIndicatorDataNow(iIndicator[i]);
			result = m_pLogicalBusiness->SetIndicatorIntoTable(m_pIndicatorData,varCfg,varCount,m_txTable);
			if(result==false)
				break;
		}
		if(result==false)
			return false;
		//根据之前3个步骤的设置进行数据读取，结果数据存放在table中
		result=m_pLogicalBusiness->GetData(m_txTable,true);
		if(result==false)
			return false;
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
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false;
		}

		UINT* nColArray = new UINT[iCol];
		for(UINT i=0;i<iCol;i++)
			nColArray[i]=i;

		//取得指定时间区间内的记录数据	
		if(m_txTable.GetRowCount()>0)
		{
			if(!bGetAllDate)
			{
				//m_txTable.Between(resTable,nColArray,iCol,1,iStartDate,iEndDate,true,true);
				m_txTable.Between(resTable,nColArray,iCol,4,iStartDate,iEndDate,true,true);//modified by zhangxs
			}
			else
			{
				resTable.Clone(m_txTable);	
			}
		}
		m_txTable.Clear();
		m_txTable.CopyColumnInfoFrom(resTable);
		//取得指定实体id(iSecurityId)
		if(resTable.GetRowCount()>0)
		{
			resTable.EqualsAt(m_txTable,nColArray,iCol,0,setSecurityId);
		}
#ifdef _DEBUG
		CString strTable1=m_txTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable1);
#endif
		//为插入交易实体ID做准备。
		m_txTable.InsertCol(0,Tx::Core::dtype_int4);//插入交易实体ID
		//插入这两列为了插入代码和名称。
		m_txTable.InsertCol(1,Tx::Core::dtype_val_string);//插入基金名称
		m_txTable.InsertCol(2,Tx::Core::dtype_val_string);//插入代码
//		Tx::Core::Table_Indicator	resTable1;
		std::set<int>::iterator iter1;
		for(iter1 = DifferentID.begin();iter1!=DifferentID.end();++iter1)
		{
			//取得交易实体ID
			int TradeID = *iter1;
			GetSecurityNow(*iter1);
			if(m_pSecurity==NULL)
				continue;
			//取得基金ID
			int tempInstitutionid1 = m_pSecurity->GetSecurity1Id();
			//根据交易实体ID取得样本的名称和外码；
			CString strName,strCode;
			strName = m_pSecurity->GetName();
			strCode = m_pSecurity->GetCode();
		    //把基金ID和交易实体ID对应起来。并且把交易实体ID放到表里。
			std::vector<UINT> vecInstiID2;
			m_txTable.Find(3,tempInstitutionid1,vecInstiID2);
			std::vector<UINT>::iterator iteID;
			for(iteID = vecInstiID2.begin();iteID != vecInstiID2.end();++iteID)
			{
				CString  sex;
				m_txTable.GetCell(9,*iteID,sex);
				CString str1 = _T("男");
				CString str2 = _T("女");
				if(sex == "1")
					m_txTable.SetCell(9,*iteID,str1);
				if(sex == "0")
					m_txTable.SetCell(9,*iteID,str2);
				m_txTable.SetCell(0,*iteID,TradeID);
				m_txTable.SetCell(1,*iteID,strName);
				m_txTable.SetCell(2,*iteID,strCode);
			}
			
		}
#ifdef _DEBUG
		CString strTable3=m_txTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable3);
#endif
		//为计算方便，以基金ID进行排序
		m_txTable.Sort(3);
		if(SameFundId.size() != 0)
		{
			//为不同的交易实体ID转化为相同的基金ID那样的样本填写数据，
			std::vector<UINT> vecFundID;
			std::set<int>::iterator iterSet,tempIter;
			for(iterSet = SameFundId.begin();iterSet != SameFundId.end();++iterSet)
			{
				int tempfundid ;
				GetSecurityNow(*iterSet);
				if(m_pSecurity == NULL)
					continue;
				tempfundid = m_pSecurity->GetSecurity1Id();//取得基金ID
                CString fundname,fundcode;
				fundname = m_pSecurity->GetName();
				fundcode = m_pSecurity->GetCode();
				m_txTable.Find(3,tempfundid,vecFundID);
				if(vecFundID.size() == 0)
					continue;
				//取得在表中最小的位置
				std::set<int> tempset(vecFundID.begin(),vecFundID.end());
				tempIter = tempset.begin();
				//增加相同的记录
				std::set<int>::size_type icount = tempset.size();
				m_txTable.InsertRow(*tempIter,icount);
				int istartdate,ienddate,birthday;
				CString name,strpost,sex,resume,remark,degree;
				int position1,position2;
                for(;tempIter != tempset.end();++tempIter)
				{
					position1 = *tempIter;
					position2 = position1 + icount;
                    m_txTable.SetCell(0,position1,*iterSet);
					m_txTable.SetCell(1,position1,fundname);
					m_txTable.SetCell(2,position1,fundcode);
                    //取得其他数据
					m_txTable.GetCell(5,position2,name);//基金经理的名称
                    m_txTable.GetCell(6,position2,strpost);//取得职位
					m_txTable.GetCell(7,position2,istartdate);//取得起始日期
					m_txTable.GetCell(8,position2,ienddate);//取得终止日期
					m_txTable.GetCell(9,position2,sex);//取得性别
					m_txTable.GetCell(10,position2,degree);//取得学历
					m_txTable.GetCell(11,position2,birthday);//取得出生年份
					m_txTable.GetCell(12,position2,resume);//取得简历
					m_txTable.GetCell(13,position2,remark);//取得备注
					//填充其他数据
					m_txTable.SetCell(5,position1,name);
					m_txTable.SetCell(6,position1,strpost);
					m_txTable.SetCell(7,position1,istartdate);
					m_txTable.SetCell(8,position1,ienddate);
					m_txTable.SetCell(9,position1,sex);
					m_txTable.SetCell(10,position1,degree);
					m_txTable.SetCell(11,position1,birthday);
					m_txTable.SetCell(12,position1,resume);
					m_txTable.SetCell(13,position1,remark);
				}
				vecFundID.clear();
				tempset.clear();
			}
		}
#ifdef _DEBUG
		CString strTable4=m_txTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable4);
#endif
		m_txTable.DeleteCol(3,2);//删除基金ID和公告日期
		resTable.Clear();
		resTable.Clone(m_txTable);
		delete nColArray;
		nColArray = NULL;
#ifdef _DEBUG
		strTable4=m_txTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable4);
#endif
		//添加进度条
		prw.SetPercent(pid,1.0);
		return true;

	}
	

//add by lijw 2008-03-05
//持有人结构
bool TxFund::StatFundHoldingStructure(
									  Tx::Core::Table_Indicator &resTable,
									  std::vector<int>	iSecurityId,
									  std::vector<int>	iDate,
									  int		iType
									  )
{
	//添加进度条
// 	ProgressWnd* pwd=Tx::Core::ProgressWnd::GetInstance();
	ProgressWnd prw;
	UINT pid=prw.AddItem(1,_T("持有人结构统计..."),0.0);
	prw.Show(1000);
	m_txTable.Clear();//这是引用别人的成员变量，
	//默认的返回值状态
	bool result = false;
	//清空数据
	m_txTable.Clear();
	//准备样本集参数列
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//基金交易实体ID
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//截止日期
	const int indicatorIndex = 4;
	long iIndicator[indicatorIndex] = 
	{
		30301118,	//期末持有人户数
		30301119,	//平均每户持有份额
		30301120,	//机构投资者持有份额
		30301121	//个人投资者持有份额
	};
	UINT varCfg[2];			//参数配置
	int varCount=2;			//参数个数
	for (int i = 0; i < indicatorIndex; i++)
	{
		int tempIndicator = iIndicator[i];

		GetIndicatorDataNow(tempIndicator);
		if (m_pIndicatorData==NULL)
		{ return false; }
		varCfg[0]=0;
		varCfg[1]=1;
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
	UINT iColCount = m_txTable.GetColCount();
	UINT* nColArray = new UINT[iColCount];
	for(int i = 0; i < (int)iColCount; i++)
	{
		nColArray[i] = i;
	}
	result = m_pLogicalBusiness->GetData(m_txTable,true);
	if(result == false || m_txTable.GetRowCount() == 0)
	{
		//添加进度条
		prw.SetPercent(pid,1.0);
		return false;
	}
#ifdef _DEBUG
	CString strTable=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	Tx::Core::Table_Indicator tempTable;
	tempTable.CopyColumnInfoFrom(m_txTable);
	//根据基金交易实体ID进行筛选
	m_txTable.EqualsAt(tempTable,nColArray,iColCount,0,iSecurityId);
	if(tempTable.GetRowCount() == 0)
	{ 
		delete nColArray;
		nColArray = NULL;
		//添加进度条
		prw.SetPercent(pid,1.0);
		return false;
	}
#ifdef _DEBUG
	strTable=tempTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	resTable.CopyColumnInfoFrom(tempTable);
	//进行报告期的筛选,它是以截止日期为基准的。
	tempTable.EqualsAt(resTable,nColArray,iColCount,1,iDate);
	if(resTable.GetRowCount() == 0)
	{ 
		delete nColArray;
		nColArray = NULL;
		//添加进度条
		prw.SetPercent(pid,1.0);
		return false;
	}
	delete nColArray;
	nColArray = NULL;
#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	if(iType == 1)
		prw.SetPercent(pid,0.6);//添加进度条

	resTable.InsertCol(1,Tx::Core::dtype_val_string);//基金的名称
	resTable.InsertCol(2,Tx::Core::dtype_val_string);//基金的代码
	resTable.InsertCol(4,Tx::Core::dtype_val_string);//基金的报告期
	resTable.InsertCol(5,Tx::Core::dtype_double);//份额合计
	resTable.AddCol(Tx::Core::dtype_double);//10机构投资比例
	resTable.AddCol(Tx::Core::dtype_double);//11 个人投资比例
	//增加基金的交易实体ID和名称，代码
	int date;
	CString strdate,year,Redate;
	double dPersonHold,dInstitutionHold,dTotalHold;
	double dInstiPro,dPerPro;
	std::vector<int>::iterator iterId;
	for(iterId = iSecurityId.begin();iterId != iSecurityId.end();++iterId)
	{
		int TradeId = *iterId ;
		CString strName,strCode;
		GetSecurityNow(TradeId);
		if(m_pSecurity == NULL)
			continue;
		strName = m_pSecurity->GetName();
		strCode = m_pSecurity->GetCode();
		std::vector<UINT> vecInstiID;
		resTable.Find(0,TradeId,vecInstiID);
		std::vector<UINT>::iterator iteID;
		for(iteID = vecInstiID.begin();iteID != vecInstiID.end();++iteID)
		{
			UINT position = *iteID;
//			resTable.SetCell(0,position,*iterId);
			resTable.SetCell(1,position,strName);
			resTable.SetCell(2,position,strCode);
			if(iType ==1)
			{
				//填写报告期
				resTable.GetCell(3,position,date);
				strdate.Format(_T("%d"),date);
				year = strdate.Left(4);
				Redate = strdate.Right(4);
				// if(Redate == "0630")
				if (_tcsicmp(Redate,_T("0630")) == 0)
					strdate = year+_T("年")+_T("中报");
				else
					strdate = year+_T("年")+_T("年报");
				resTable.SetCell(4,position,strdate);
			}			
			//计算比例与合计
			resTable.GetCell(8,position,dInstitutionHold);
			resTable.GetCell(9,position,dPersonHold);
			dTotalHold = dPersonHold + dInstitutionHold;
			resTable.SetCell(5,position,dTotalHold);
            dInstiPro = 100*dInstitutionHold/dTotalHold;
			dPerPro = 100*dPersonHold/dTotalHold;
			resTable.SetCell(10,position,dInstiPro);
			resTable.SetCell(11,position,dPerPro);
		}

	}
	if(iType ==1)
	{
		resTable.DeleteCol(3);//删除截止日期
	}
#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	int iType2 = iType;//保存原来的值.
	if(iType >1)
	{
		int iCount = resTable.GetRowCount();//累加之前resTable里的行数。
		//为了进行分类统计。所以增加一列基金ID。
		resTable.AddCol(Tx::Core::dtype_int4);//基金ID
		resTable.AddCol(Tx::Core::dtype_int4);//基金风格
		resTable.AddCol(Tx::Core::dtype_int4);//基金管理公司ID
		resTable.AddCol(Tx::Core::dtype_int4);//基金托管银行ID
		std::set<int> fundIdSet;
		int tempfundId,TradeId;
//		int icount = resTable.GetRowCount();
		for(int i = 0;i < iCount;i++)
		{
			resTable.GetCell(0,i,TradeId);
			GetSecurityNow(TradeId);
			if(m_pSecurity == NULL)
				continue;
//#ifdef _SECURITYID_FOR_STAT_
			resTable.SetCell(12,i,TradeId);
			fundIdSet.insert(TradeId);
//#else
//			tempfundId = m_pSecurity->GetSecurity1Id();
//			resTable.SetCell(12,i,tempfundId);
//			fundIdSet.insert(tempfundId);
//#endif
			
		}
		//添加进度条
		prw.SetPercent(pid,0.6);
		//这里增加样本分类选择
		m_txTable.Clear();
		tempTable.Clear();
		//准备样本集=第一参数列:F_FUND_ID,int型
		m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
		const int indicatorIndex1 = 3;
		long iIndicator1[indicatorIndex1] = 
		{
			//30001035,	//基金风格
			//modified by zhangxs 20091221---NewStyle
			30001232,	//基金风格New
			30001020,	//管理公司ID，
			30001021	//托管银行ID，
		};
		UINT varCfg1[1];			//参数配置
		int varCount1=1;			//参数个数
		for (int i = 0; i < indicatorIndex1; i++)
		{
			int tempIndicator = iIndicator1[i];
			GetIndicatorDataNow(tempIndicator);
			if (m_pIndicatorData==NULL)
				return false; 
			varCfg1[0]=0;
			result = m_pLogicalBusiness->SetIndicatorIntoTable(
				m_pIndicatorData,	//指标
				varCfg1,				//参数配置
				varCount1,			//参数个数
				m_txTable	//计算需要的参数传输载体以及计算后结果的载体
				);
			if(result==false)
			{
				return FALSE;
			}

		}
		result = m_pLogicalBusiness->GetData(m_txTable,true);
		if(result==false)
			return false;
#ifdef _DEBUG
		strTable=m_txTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
		UINT iCol2=m_txTable.GetColCount();
		//复制所有列信息
		tempTable.CopyColumnInfoFrom(m_txTable);
		if(m_txTable.GetRowCount()==0)
		{
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false;
		}
		UINT* nColArray2 = new UINT[iCol2];
		for(UINT i=0;i<iCol2;i++)
			nColArray2[i]=i;	
		m_txTable.EqualsAt(tempTable,nColArray2,iCol2,0,fundIdSet);
		delete nColArray2;
		nColArray2 = NULL;
		//把基金风格、公司ID、托管银行ID放到resTable
		std::vector<UINT> vecInstiID2;
		std::vector<UINT>::iterator iteID;
		int position;
		int iStyle,iCompanyId,ibankId;
		int icount = tempTable.GetRowCount();
		for(int j = 0;j < icount;j++)
		{
			tempTable.GetCell(0,j,tempfundId);
			tempTable.GetCell(1,j,iStyle);
			tempTable.GetCell(2,j,iCompanyId);
			tempTable.GetCell(3,j,ibankId);
			//把基金ID和交易实体ID对应起来。并且把数据放到表里。
			if(!(vecInstiID2.empty()))
				vecInstiID2.clear();
			resTable.Find(12,tempfundId,vecInstiID2);
			for(iteID = vecInstiID2.begin();iteID != vecInstiID2.end();++iteID)
			{
				position = *iteID;
				resTable.SetCell(13,position,iStyle);//基金风格
				resTable.SetCell(14,position,iCompanyId);//管理公司ID
				resTable.SetCell(15,position,ibankId);//托管银行ID
			}
		}
#ifdef _DEBUG
		strTable=m_txTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif		
		std::vector<int> ColVector;		//根据哪些列进行统计  
		if(!(iType & 1))
			iType += 1;//假设都选择选择了单只基金那一项分类方式。
		if(iType & 2)
			ColVector.push_back(14);
		if(iType & 4)
			ColVector.push_back(13);
		if(iType & 8)
			ColVector.push_back(15);
		std::vector<int> IntCol;			//需要相加的整型列
		IntCol.push_back(6);
		std::vector<int> DoubleCol;	//需要相加的double列
		DoubleCol.push_back(5);
		for(int j = 8;j < 10;j++)
		{
			DoubleCol.push_back(j);
		}
#ifdef _DEBUG
		strTable=resTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
		AddUpRow(resTable,iType,ColVector,IntCol,DoubleCol,iDate,3);
		std::vector<int>::size_type isize = iDate.size();
		int iCount1 = resTable.GetRowCount();//累加以后resTable里的行数。
		int iCount2,iCount3;
		iCount2 = iCount1 - iCount;//这是增加的行数 样本汇总和其他分类方式的行数。
		std::unordered_map<int,CString> CompanyMap;
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_INSTITUTIONID_TO_SHORTNAME,CompanyMap);
		std::unordered_map<int,CString> StyleMap;
		//modified by zhangxs 20091221---NewStyle
		//TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX,StyleMap);
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX_NEW,StyleMap);
		std::unordered_map<int,CString> BankMap;
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_BANK,BankMap);
		std::unordered_map<int,CString>::iterator iterMap;
		int id;
		CString Name;		
		if(iType < 17)//没有样本汇总的情况
		{
			for(int m = 0;m < iCount2;m++)
			{				
				resTable.GetCell(13,iCount+m,id);
				if(id == 0)
				{
					resTable.GetCell(14,iCount+m,id);
					if(id == 0)
					{
						resTable.GetCell(15,iCount+m,id);
						iterMap = BankMap.find(id);
						if(iterMap!= BankMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,iCount+m,Name);
						resTable.SetCell(2,iCount+m,(CString)(_T("托管银行")));
						continue;
					}
					else
					{
						iterMap = CompanyMap.find(id);
						if(iterMap!= CompanyMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,iCount+m,Name);
						resTable.SetCell(2,iCount+m,(CString)(_T("基金公司")));
						continue;
					}					
				}
				else
				{
					iterMap = StyleMap.find(id);
					if(iterMap!= StyleMap.end())
						Name = iterMap->second;
					else
						Name = _T("未知类型");
					resTable.SetCell(1,iCount+m,Name);
					resTable.SetCell(2,iCount+m,(CString)(_T("基金风格")));
				}
			}				
		}
		else//不但有单只基金和样本汇总，还有其他的分类方式
		{
			iCount3 = iCount2 - isize;//除了样本汇总以外的其他分类方式的行数
			if(iCount3 > 0)//表示除了样本汇总以外还有其他的分类的方式
			{
				//填充样本汇总的那些行
				for(int k = 0;k < (int)isize;k++)
				{
					resTable.SetCell(1,iCount+iCount3+k,(CString)(_T("样本汇总")));
					resTable.SetCell(2,iCount+iCount3+k,(CString)(_T("全部汇总")));
				}
				//填充除了样本汇总以外的其他分类方式的那些行。
				for(int i = 0;i < iCount3;i++)
				{
					resTable.GetCell(13,iCount+i,id);
					if(id == 0)
					{
						resTable.GetCell(14,iCount+i,id);
						if(id == 0)
						{
							resTable.GetCell(15,iCount+i,id);
							iterMap = BankMap.find(id);
							if(iterMap!= BankMap.end())
								Name = iterMap->second;
							else
								Name = Tx::Core::Con_strInvalid;
							resTable.SetCell(1,iCount+i,Name);
							resTable.SetCell(2,iCount+i,(CString)(_T("托管银行")));
							continue;
						}
						else
						{
							iterMap = CompanyMap.find(id);
							if(iterMap!= CompanyMap.end())
								Name = iterMap->second;
							else
								Name = Tx::Core::Con_strInvalid;
							resTable.SetCell(1,iCount+i,Name);
							resTable.SetCell(2,iCount+i,(CString)(_T("基金公司")));
							continue;
						}					
					}
					else
					{
						iterMap = StyleMap.find(id);
						if(iterMap!= StyleMap.end())
							Name = iterMap->second;
						else
							Name = _T("未知类型");
						resTable.SetCell(1,iCount+i,Name);
						resTable.SetCell(2,iCount+i,(CString)(_T("基金风格")));
					}
				}
			}
			else//只有单只基金和汇总的情况
			{
				for(int k = 0;k < (int)isize;k++)
				{
					resTable.SetCell(1,iCount+k,(CString)(_T("样本汇总")));
					resTable.SetCell(2,iCount+k,(CString)(_T("全部汇总")));
				}		
			}
		}	
		double proportion,totalShare,share;
		int tempCount;
		if(!(iType2 & 1))
		{
			resTable.DeleteRow(0,iCount);
			tempCount = iCount2;			
		}
		else
		{
			tempCount = iCount1;			
		}
		for(int j = 0;j < tempCount;j++)
		{
			//填写报告期
			resTable.GetCell(3,j,date);
			strdate.Format(_T("%d"),date);
			year = strdate.Left(4);
			Redate = strdate.Right(4);
			// if(Redate == "0630")
			if (_tcsicmp(Redate,_T("0630")) == 0)
				strdate = year+_T("年")+_T("中报");
			else
				strdate = year+_T("年")+_T("年报");
			resTable.SetCell(4,j,strdate);
			//计算比例
			resTable.GetCell(8,j,share);//机构投资者持有份额。
			resTable.GetCell(5,j,totalShare);
			if(totalShare > 0)
			{
				proportion = share*100/totalShare;
				if(proportion > 0)
					resTable.SetCell(10,j,proportion);
				else
					resTable.SetCell(10,j,Tx::Core::Con_doubleInvalid);
				//resTable.SetCell(10,j,proportion);
				resTable.GetCell(9,j,share);//个人投资者持有份额。
				proportion = share*100/totalShare;
				if(proportion > 0)
					resTable.SetCell(11,j,proportion);
				else
					resTable.SetCell(11,j,Tx::Core::Con_doubleInvalid);
				//计算平均份额
				int personCount;
				resTable.GetCell(6,j,personCount);
				proportion = totalShare/personCount;
				if(proportion > 0)
					resTable.SetCell(7,j,proportion);
				else
					resTable.SetCell(7,j,Tx::Core::Con_doubleInvalid);
			}
			else
			{
				resTable.SetCell(7,j,Tx::Core::Con_doubleInvalid);
				resTable.SetCell(10,j,Tx::Core::Con_doubleInvalid);
				resTable.SetCell(11,j,Tx::Core::Con_doubleInvalid);
			}
			
		}		
		resTable.DeleteCol(12,4);
		resTable.Arrange();
		MultiSortRule multisort;
		multisort.AddRule(3,false);
		multisort.AddRule(2,true);
		multisort.AddRule(5,false);			
		resTable.SortInMultiCol(multisort);
		resTable.Arrange();
		resTable.DeleteCol(3);
#ifdef _DEBUG
		strTable=resTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
		//添加进度条
		prw.SetPercent(pid,1.0);
	}
	else
	{		
		//添加进度条
		prw.SetPercent(pid,1.0);
	}
	/*if(iType2 > 17)
	{
		resTable.SetSortRange(0,resTable.GetRowCount()-2);
	}*/
	return true;
}

//add by lijw 2008-03-25
//备注：此函数是专门计算各行值得累加。为样本的分类统计作准备
bool TxFund::AddUpRow(Tx::Core::Table &resTable,		//存放结果表
			  int iStyle,						//样本分类的方式
			  std::vector<int> ColVector,		//根据哪些列进行统计  
			  std::vector<int> IntCol,			//需要相加的整型列
			  std::vector<int> DoubleCol,	//需要相加的double列
			  std::vector<int> iDate,			//报告期
			  int			  iCol				//报告期所在的列。
			  )	
{
	std::set<int>::iterator iterSet;
	std::set<int> iIdSet;	
	std::vector<UINT> PositionCollect;
	std::vector<UINT> TempCollect;
	std::vector<UINT> Intersection;
	std::vector<UINT>::iterator UterV;
	std::vector<int>::iterator iterV;
	std::vector<int>::iterator iterDate;
	std::vector<int>::iterator dterV;
	std::vector<int>::iterator iterCol;//参照列的专用变量
//	std::unordered_map<int,int>::iterator iterMap;
	//用于保存分类统计时的数据
	Tx::Core::Table tempTable;
	tempTable.CopyColumnInfoFrom(resTable);
	int Colpos;//程序中公共用的行变量。
	int iDateCol;//报告期的专用列号
	int iIdCol;//参照列的专用列号
	int iRow;//程序中公共用的行变量。
	int row = -1;//这是专门纪录tempTable里的行号。
	int icount;
	int iData = 0;
	double dData = 0;
	int itempdate;
	int itempId;
	std::unordered_map<int,int> iDataMap;//存放临时的整型值	
	std::unordered_map<int,double> dDataMap;//存放临时的double值
	//填充iDataMap和dDataMap
	for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
	{
		Colpos = *iterV;
		iDataMap.insert(std::make_pair(Colpos,0));
	}
	for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
	{
		Colpos = *dterV;
		dDataMap.insert(std::make_pair(Colpos,0));
	}
#if _DEBUG
    CString strTable;//为了测试用的
#endif 
	iDateCol = iCol;
	//之所以把它们分开计算，而没有一个个计算，是为了进行样本汇总时，方便计算
	//并且当按多个分类方式进行统计时，可以同时计算多个分类方式
	switch(iStyle)
	{
	case 3://管理公司ID
	case 5://基金风格
	case 7://管理公司ID和基金风格
	case 9://托管银行ID   	
	case 11://管理公司ID和托管银行ID
	case 13://基金风格和托管银行ID
	case 15://基金风格和托管银行ID和管理公司ID
		{
			//根据那个参数列进行相加
			for(iterCol = ColVector.begin();iterCol != ColVector.end();++iterCol)
			{				
				icount = resTable.GetRowCount();
				iIdCol = *iterCol;
				for(int i = 0;i < icount;i++)
				{
					resTable.GetCell(iIdCol,i,itempId);
					iIdSet.insert(itempId);
				}
				//根据ID进行相加
				for(iterSet = iIdSet.begin();iterSet != iIdSet.end();++iterSet)
				{					
					itempId = *iterSet;					
					PositionCollect.clear();
					resTable.Find(iIdCol,itempId,PositionCollect);//取得与公司ID相同的纪录的位置。
					if(PositionCollect.empty())
						continue;
					//把vector里的值放在set里，是为了用Set里的find方法
					std::set<UINT> positionSet(PositionCollect.begin(),PositionCollect.end());
//					std::set<UINT> positionSet(PositionCollect.begin(),PositionCollect.end());
					//根据报告期进行筛选
					for(iterDate = iDate.begin();iterDate != iDate.end();++iterDate)
					{
						itempdate = *iterDate;
						TempCollect.clear();
						resTable.Find(iDateCol,itempdate,TempCollect);//取得与报告期相同的纪录的位置。
						if(TempCollect.empty())
							continue;
						//把它们相同的值保存在Intersection
						Intersection.clear();
						for(UterV = TempCollect.begin();UterV != TempCollect.end();++UterV)
						{
							if(positionSet.find(*UterV) != positionSet.end())
								Intersection.push_back(*UterV);
						}
						if(Intersection.empty())
							continue;
						//统计一个报告期，tempTable表增加一行。
						tempTable.AddRow();
						row++;
						//把保存各列值得map初始化为零
						if(!IntCol.empty())
						{
							for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
							{
								Colpos = *iterV;
								iDataMap[Colpos] = 0;
							}
						}	
						if(!DoubleCol.empty())
						{
							for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
							{
								Colpos = *dterV;
								dDataMap[Colpos] = 0;
							}
						}						
						//把ID相同并且报告期相同的行中，所需要相加的列进行相加。
						for(UterV = Intersection.begin();UterV != Intersection.end();++UterV)
						{
							iRow = *UterV;
							//需要相加的整型列进行相加。
							if(!IntCol.empty())
							{
								for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
								{
									Colpos = *iterV;
									resTable.GetCell(Colpos,iRow,iData);
									if (iData < 0)
										continue;									
									iDataMap[Colpos] = iDataMap[Colpos] + iData;
								}
							}							
							//需要相加的double列进行相加。
							if(!DoubleCol.empty())
							{
								for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
								{
									Colpos = *dterV;
									resTable.GetCell(Colpos,iRow,dData);
									if (dData < 0)
										continue;									
									dDataMap[Colpos] = dDataMap[Colpos] + dData;
								}
							}						
						}
						//把各列的值放到tempTable里。
						//首先放整型列的值
						if(!IntCol.empty())
						{
							for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
							{
								Colpos = *iterV;						
								iData = iDataMap[Colpos];
								tempTable.SetCell(Colpos,row,iData);
							}
						}						
						//其次放double列的值
						if(!DoubleCol.empty())
						{
							for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
							{
								Colpos = *dterV;
								dData= dDataMap[Colpos] ;
								tempTable.SetCell(Colpos,row,dData);
							}
						}					
						//把报告期和ID放到tempTable里;
						tempTable.SetCell(iDateCol,row,itempdate);
						tempTable.SetCell(iIdCol,row,itempId);
					}
				}
			}
			//把tempTable里的数据放到resTable
			icount = tempTable.GetRowCount();
			int tempRow = resTable.GetRowCount();
			resTable.AddRow(icount);
			for(int k = 0;k < icount;k++)
			{
				//首先把报告期和ID添加到resTable里。
				tempTable.GetCell(iDateCol,k,iData);
				resTable.SetCell(iDateCol,tempRow+k,iData);
				for(iterV = ColVector.begin();iterV != ColVector.end();++iterV)
				{
					tempTable.GetCell(*iterV,k,iData);
					resTable.SetCell(*iterV,tempRow+k,iData);
				}
				//添加所累加的数据。
				if (!IntCol.empty())
				{
					for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
					{
						tempTable.GetCell(*iterV,k,iData);
						if (iData < 0)
						{
							iData = Tx::Core::Con_intInvalid;
						}
						resTable.SetCell(*iterV,tempRow+k,iData);
					}
				} 
				if(!DoubleCol.empty())
				{
					for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
					{
						tempTable.GetCell(*dterV,k,dData);
						if (dData < 0)
						{
							dData = Tx::Core::Con_doubleInvalid;								
						}
						resTable.SetCell(*dterV,tempRow+k,dData);
					}
				}
			}
		}
#ifdef _DEBUG
		strTable=resTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
		break;
    case 17://样本汇总
		{			
			//根据报告期进行筛选
			for(iterDate = iDate.begin();iterDate != iDate.end();++iterDate)
			{
				itempdate = *iterDate;
				//iDateCol = iterMap->first;
				//itempdate = iterMap->second;
				TempCollect.clear();
				resTable.Find(iDateCol,itempdate,TempCollect);//取得与报告期相同的纪录的位置。
				if(TempCollect.empty())
					continue;
				tempTable.AddRow();
				row++;	
				//把保存各列值得map初始化为零
				if(!IntCol.empty())
				{
					for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
					{
						Colpos = *iterV;
						iDataMap[Colpos] = 0;
					}
				}	
				if(!DoubleCol.empty())
				{
					for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
					{
						Colpos = *dterV;
						dDataMap[Colpos] = 0;
					}
				}					
				//把报告期相同的行中，所需要相加的列进行相加。
				for(UterV = TempCollect.begin();UterV != TempCollect.end();++UterV)
				{
					iRow = *UterV;
					//需要相加的整型列进行相加。
					if(!IntCol.empty())
					{
						for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
						{
							Colpos = *iterV;
							resTable.GetCell(Colpos,iRow,iData);
							if (iData < 0)
								continue;						
							iDataMap[Colpos] = iDataMap[Colpos] + iData;
						}
					}							
					//需要相加的double列进行相加。
					if(!DoubleCol.empty())
					{
						for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
						{
							Colpos = *dterV;
							resTable.GetCell(Colpos,iRow,dData);
							if (dData < 0)
								continue;							
							dDataMap[Colpos] = dDataMap[Colpos] + dData;
						}
					}						
				}
				//把各列的值放到tempTable里。
				//首先放整型列的值
				if(!IntCol.empty())
				{
					for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
					{
						Colpos = *iterV;						
						iData = iDataMap[Colpos];
						tempTable.SetCell(Colpos,row,iData);
					}
				}						
				//其次放double列的值
				if(!DoubleCol.empty())
				{
					for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
					{
						Colpos = *dterV;
						dData= dDataMap[Colpos] ;
						tempTable.SetCell(Colpos,row,dData);
					}
				}					
				//把报告期放到tempTable里; 此时得到的值也是样本汇总的统计。
				tempTable.SetCell(iDateCol,row,itempdate);
			}
#ifdef _DEBUG
			strTable=tempTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
			//把tempTable里的数据放到resTable
			icount = tempTable.GetRowCount();
			int tempRow = resTable.GetRowCount();
			resTable.AddRow(icount);
			for(int k = 0;k < icount;k++)
			{
				//首先把报告期和ID添加到resTable里。
				tempTable.GetCell(iDateCol,k,iData);
				resTable.SetCell(iDateCol,tempRow+k,iData);
				//添加所累加的数据。
				if (!IntCol.empty())
				{
					for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
					{
						tempTable.GetCell(*iterV,k,iData);
						if (iData < 0)
						{
							iData = Tx::Core::Con_intInvalid;
						}
						resTable.SetCell(*iterV,tempRow+k,iData);
					}
				} 
				if(!DoubleCol.empty())
				{
					for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
					{
						tempTable.GetCell(*dterV,k,dData);
						if (dData < 0)
						{
							dData = Tx::Core::Con_doubleInvalid;								
						}
						resTable.SetCell(*dterV,tempRow+k,dData);
					}
				}
			}
		}
#ifdef _DEBUG
		strTable=resTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
		break;
	case 19://管理公司ID和样本汇总
	case 21://基金风格和样本汇总
	case 25://样本汇总和托管银行ID
	case 23://基金风格和样本汇总和管理公司ID
	case 27://样本汇总和托管银行ID和管理公司ID
	case 29://基金风格和托管银行ID和样本汇总
	case 31://基金风格和托管银行ID和样本汇总和管理公司
		{
			//这是只有全部汇总分类方式的情况下
			int cutoff = 1;//为了保存全部汇总的数据的开关变量	
			Tx::Core::Table collectTable;
			collectTable.CopyColumnInfoFrom(resTable);
			//下面是不但有全部汇总分类方式进行统计，还有其他的方式。
			//根据那个参数列进行相加
			for(iterCol = ColVector.begin();iterCol != ColVector.end();++iterCol,cutoff++)
			{	
				icount = resTable.GetRowCount();
				iIdCol = *iterCol;
				for(int i = 0;i < icount;i++)
				{
					resTable.GetCell(iIdCol,i,itempId);
					iIdSet.insert(itempId);
				}
				//根据ID进行相加
				for(iterSet = iIdSet.begin();iterSet != iIdSet.end();++iterSet)
				{					
					itempId = *iterSet;					
					PositionCollect.clear();
					resTable.Find(iIdCol,itempId,PositionCollect);//取得与公司ID相同的纪录的位置。
					if(PositionCollect.empty())
						continue;
					//把vector里的值放在set里，是为了用Set里的find方法
					std::set<UINT> positionSet(PositionCollect.begin(),PositionCollect.end());
//					std::set<UINT> positionSet(PositionCollect.begin(),PositionCollect.end());
					//根据报告期进行筛选
					for(iterDate = iDate.begin();iterDate != iDate.end();++iterDate)
					{
						itempdate = *iterDate;
						TempCollect.clear();
						resTable.Find(iDateCol,itempdate,TempCollect);//取得与报告期相同的纪录的位置。
						if(TempCollect.empty())
							continue;
						//把它们相同的值保存在Intersection
						Intersection.clear();
						for(UterV = TempCollect.begin();UterV != TempCollect.end();++UterV)
						{
							if(positionSet.find(*UterV) != positionSet.end())
								Intersection.push_back(*UterV);
						}
						if(Intersection.empty())
							continue;
						tempTable.AddRow();
						row += 1;
						//把保存各列值得map初始化为零
						if(!IntCol.empty())
						{
							for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
							{
								Colpos = *iterV;
								iDataMap[Colpos] = 0;
							}
						}	
						if(!DoubleCol.empty())
						{
							for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
							{
								Colpos = *dterV;
								dDataMap[Colpos] = 0;
							}
						}					
						//把ID相同并且报告期相同的行中，所需要相加的列进行相加。
						for(UterV = Intersection.begin();UterV != Intersection.end();++UterV)
						{
							iRow = *UterV;
							//需要相加的整型列进行相加。
							if(!IntCol.empty())
							{
								for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
								{
									Colpos = *iterV;
									resTable.GetCell(Colpos,iRow,iData);
									if (iData < 0)
										continue;									
									iDataMap[Colpos] = iDataMap[Colpos] + iData;
								}
							}							
							//需要相加的double列进行相加。
							if(!DoubleCol.empty())
							{
								for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
								{
									Colpos = *dterV;
									resTable.GetCell(Colpos,iRow,dData);									
									if (dData < 0)
										continue;									
									dDataMap[Colpos] = dDataMap[Colpos] + dData;
								}
							}						
						}
						//把各列的值放到tempTable里。
						//首先放整型列的值
						if(!IntCol.empty())
						{
							for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
							{
								Colpos = *iterV;						
								iData = iDataMap[Colpos];
								tempTable.SetCell(Colpos,row,iData);
							}
						}						
						//其次放double列的值
						if(!DoubleCol.empty())
						{
							for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
							{
								Colpos = *dterV;
								dData= dDataMap[Colpos] ;
								tempTable.SetCell(Colpos,row,dData);
							}
						}					
						//把报告期和ID放到tempTable里;
						tempTable.SetCell(iDateCol,row,itempdate);
						tempTable.SetCell(iIdCol,row,itempId);
					}
#ifdef _DEBUG
					strTable=tempTable.TableToString();
					Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
				}
				if(cutoff == 1)
				{
					//根据报告期进行筛选
					int temprow = -1;
					for(iterDate = iDate.begin();iterDate != iDate.end();++iterDate)
					{
						itempdate = *iterDate;
						TempCollect.clear();
						tempTable.Find(iDateCol,itempdate,TempCollect);//取得与报告期相同的纪录的位置
						if(TempCollect.empty())
							continue;
						collectTable.AddRow();
						temprow++;
						if(!IntCol.empty())
						{
							for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
							{
								Colpos = *iterV;
								iDataMap[Colpos] = 0;
							}
						}	
						if(!DoubleCol.empty())
						{
							for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
							{
								Colpos = *dterV;
								dDataMap[Colpos] = 0;
							}
						}					
						for(UterV = TempCollect.begin();UterV != TempCollect.end();++UterV)
						{
							iRow = *UterV;							
							//需要相加的整型列进行相加。
							if(!IntCol.empty())
							{
								for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
								{
									Colpos = *iterV;
									tempTable.GetCell(Colpos,iRow,iData);
									iDataMap[Colpos] = iDataMap[Colpos] + iData;
								}
							}							
							//需要相加的double列进行相加。
							if(!DoubleCol.empty())
							{
								for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
								{
									Colpos = *dterV;
									tempTable.GetCell(Colpos,iRow,dData);
									dDataMap[Colpos] = dDataMap[Colpos] + dData;
								}
							}
						}
						//把各列的值放到collectTable里。
						//首先放整型列的值
						if(!IntCol.empty())
						{
							for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
							{
								Colpos = *iterV;						
								iData = iDataMap[Colpos];
								collectTable.SetCell(Colpos,temprow,iData);
							}
						}						
						//其次放double列的值
						if(!DoubleCol.empty())
						{
							for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
							{
								Colpos = *dterV;
								dData= dDataMap[Colpos] ;
								collectTable.SetCell(Colpos,temprow,dData);
							}
						}					
						//把报告期放到collectTable里; 此时得到的值也是样本汇总的统计。
						collectTable.SetCell(iDateCol,temprow,itempdate);
					}
				}
			}
			//把tempTable里的数据放到resTable
#ifdef _DEBUG
			strTable=tempTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
			icount = tempTable.GetRowCount();
			int tempRow = resTable.GetRowCount();
			resTable.AddRow(icount);
			for(int k = 0;k < icount;k++)
			{
				//首先把报告期和ID添加到resTable里。
				tempTable.GetCell(iDateCol,k,iData);
				resTable.SetCell(iDateCol,tempRow+k,iData);
				for(iterV = ColVector.begin();iterV != ColVector.end();++iterV)
				{
					tempTable.GetCell(*iterV,k,iData);
					resTable.SetCell(*iterV,tempRow+k,iData);
				}
				//添加所累加的数据。
				if (!IntCol.empty())
				{
					for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
					{
						tempTable.GetCell(*iterV,k,iData);
						if (iData < 0)
						{
							iData = Tx::Core::Con_intInvalid;
						}
						resTable.SetCell(*iterV,tempRow+k,iData);
					}
				} 
				if(!DoubleCol.empty())
				{
					for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
					{
						tempTable.GetCell(*dterV,k,dData);
						if (dData < 0)
						{
							dData = Tx::Core::Con_doubleInvalid;
						}
						resTable.SetCell(*dterV,tempRow+k,dData);
					}
				}
			}
#ifdef _DEBUG
			strTable=resTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
			//把collectTable里数据放到resTable;
			icount = collectTable.GetRowCount();
			tempRow = resTable.GetRowCount();
			resTable.AddRow(icount);
			for(int k = 0;k < icount;k++)
			{
				//首先把报告期和ID添加到resTable里。
				collectTable.GetCell(iDateCol,k,iData);
				resTable.SetCell(iDateCol,tempRow+k,iData);
				//添加所累加的数据。
				if (!IntCol.empty())
				{
					for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
					{
						collectTable.GetCell(*iterV,k,iData);
						if (iData < 0)
						{
							iData = Tx::Core::Con_intInvalid;
						}
						resTable.SetCell(*iterV,tempRow+k,iData);
					}
				} 
				if(!DoubleCol.empty())
				{
					for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
					{
						collectTable.GetCell(*dterV,k,dData);
						if (dData < 0)
						{
							dData = Tx::Core::Con_doubleInvalid;								
						}
						resTable.SetCell(*dterV,tempRow+k,dData);
					}
				}
			}
#ifdef _DEBUG
			strTable=resTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
		}
		break;
	}
	return true;
}
//add by lijw 2008-04-07
//备注：此函数是专门计算样本汇总统计作准备
bool TxFund::AddUpRow2(Tx::Core::Table &resTable,		//存放结果表
					  std::vector<int> ColVector,		//根据哪些列进行统计  
					  std::vector<int> IntCol,			//需要相加的整型列
					  std::vector<int> DoubleCol,	//需要相加的double列
					  std::vector<int> iDate,			//报告期
					  int			  iCol,				//报告期所在的列。
					  int &			  iTradeCol,			//股票或者债券的交易实体ID所在的列
					  int			   sortCol,		//根据那一列进行排序。
					  int			   pos,			//排名所在的列。
					  bool            IsBuySell   //判断是否是买入卖出的分组统计
					  )	
{
	std::set<int>::iterator iterSet,iterTradeSet;
	std::set<int>::iterator iterTemp;
	std::set<int> iTradeSet;
	int iTradeId;
	std::unordered_multimap<int,int> IdPosition;
	std::unordered_multimap<int,int>::iterator iterMulti;	
	std::vector<UINT> TempCollect;
	std::set<int> Intersection;
	std::vector<UINT>::iterator UterV;
	std::vector<int>::iterator iterV;
	std::vector<int>::iterator iterDate;
	std::vector<int>::iterator dterV;
	std::vector<int>::iterator iterCol;//参照列的专用变量

	std::set<int> deletePosition;
	//用于保存分类统计时的数据
	Tx::Core::Table tempTable;
	tempTable.CopyColumnInfoFrom(resTable);
	int Colpos;//程序中公共用的行变量。
	int iDateCol;//报告期的专用列号
	int iRow;//程序中公共用的行变量。
	int icount;
	int iData = 0;
	double dData = 0;
	int itempdate;
	std::unordered_map<int,int> iDataMap;//存放临时的整型值	
	std::unordered_map<int,double> dDataMap;//存放临时的double值
	//填充iDataMap和dDataMap
	for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
	{
		Colpos = *iterV;
		iDataMap.insert(std::make_pair(Colpos,0));
	}
	for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
	{
		Colpos = *dterV;
		dDataMap.insert(std::make_pair(Colpos,0));
	}
//#if _DEBUG
//	CString strTable;//为了测试用的
//#endif 
	iDateCol = iCol;
	if(tempTable.GetRowCount() != 0)
	{
		tempTable.Clear();
		tempTable.Clone(resTable);
	}
	else
		tempTable.Clone(resTable);
//#ifdef _DEBUG
//	strTable=tempTable.TableToString();
//	Tx::Core::Commonality::String().StringToClipboard(strTable);
//#endif
	//根据报告期进行筛选
	for(iterDate = iDate.begin();iterDate != iDate.end();++iterDate)
	{
		itempdate = *iterDate;
		TempCollect.clear();
		tempTable.Find(iDateCol,itempdate,TempCollect);//取得与报告期相同的纪录的位置。
		if(TempCollect.empty())
			continue;			
		//取得在那些相同的行中全部的交易实体	
		if(!iTradeSet.empty())
			iTradeSet.clear();
		if(!IdPosition.empty())
			IdPosition.clear();
		for(UterV = TempCollect.begin();UterV != TempCollect.end();++UterV)
		{
			tempTable.GetCell(iTradeCol,*UterV,iTradeId);
			iTradeSet.insert(iTradeId);
			IdPosition.insert(make_pair(iTradeId,*UterV));
			//如果是买入卖出的话，就把基金的支数全部写成1，如果后面有需要合并的，再改写它们，2008-05-23
			if(IsBuySell)
			{
				int fundCount = 1;
				tempTable.SetCell(iTradeCol+3,*UterV,fundCount);
			}
		}		
		for(iterTradeSet = iTradeSet.begin();iterTradeSet != iTradeSet.end();++iterTradeSet)
		{
			iTradeId = *iterTradeSet;
			if (IdPosition.count(iTradeId) > 1)
			{
				/*while ((iterMulti = IdPosition.find(iTradeId)) != IdPosition.end())
				{
					Intersection.insert(iterMulti->second);
					IdPosition.erase(iterMulti);
				}*/
				//用下面的方法更为简便
				if(!Intersection.empty())
					Intersection.clear();
				for(iterMulti = IdPosition.lower_bound(iTradeId);iterMulti != IdPosition.upper_bound(iTradeId);++iterMulti)
				{
					Intersection.insert(iterMulti->second);					
				}
				//把保存各列值得map初始化为零
				if(!IntCol.empty() || !DoubleCol.empty())
				{
					for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
					{
						Colpos = *iterV;
						iDataMap[Colpos] = 0;
					}
					for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
					{
						Colpos = *dterV;
						dDataMap[Colpos] = 0;
					}
				}							
				//把报告期相同的行中，所需要相加的列进行相加。
				for(iterTemp = Intersection.begin();iterTemp != Intersection.end();++iterTemp)
				{
					iRow = *iterTemp;
					//需要相加的整型列进行相加。
					if(!IntCol.empty())
					{
						for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
						{
							Colpos = *iterV;
							tempTable.GetCell(Colpos,iRow,iData);
							if (iData < 0)
								continue;									
							iDataMap[Colpos] = iDataMap[Colpos] + iData;
						}
					}							
					//需要相加的double列进行相加。
					if(!DoubleCol.empty())
					{
						for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
						{
							Colpos = *dterV;
							tempTable.GetCell(Colpos,iRow,dData);
							if (dData < 0)
								continue;									
							dDataMap[Colpos] = dDataMap[Colpos] + dData;
						}
					}						
				}
				iRow = *(Intersection.begin());
				//如果买入卖出有需要合并的，改写它们的值，2008-05-23
				int iSize = Intersection.size();
				if(IsBuySell && iSize >0)
				{					
					tempTable.SetCell(iTradeCol+3,iRow,iSize);
				}
				//把相加的各项的值放到tempTable里的其中一行当中,多余的行全部删掉.
				if(!IntCol.empty() || !DoubleCol.empty())
				{					
					//首先放整型列的值
					for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
					{
						Colpos = *iterV;						
						iData = iDataMap[Colpos];								
						tempTable.SetCell(Colpos,iRow,iData);
					}
					//其次放double列的值
					for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
					{
						Colpos = *dterV;
						dData= dDataMap[Colpos] ;
						tempTable.SetCell(Colpos,iRow,dData);
					}				
					//把多余的行删掉；
					for(iterTemp = --(Intersection.end());iterTemp != Intersection.begin();--iterTemp)
					{
						deletePosition.insert(*iterTemp);
					}
				}						
			} 				
		}		
	}
	//把要删除的行全部删掉。
	std::set<int>::reverse_iterator revIter;
	for(revIter = deletePosition.rbegin();revIter != deletePosition.rend();++revIter)
		tempTable.DeleteRow(*revIter);		
	tempTable.Arrange();
	//把tempTable里的数据放到resTable
	//为了对重仓债里的重仓股进行排序，所以对tempTable进行排序，分别按照报告期、债券的市值
	MultiSortRule multisort;
	multisort.AddRule(iDateCol,true);
	multisort.AddRule(sortCol,false);
	tempTable.SortInMultiCol(multisort);
	tempTable.Arrange();
//#ifdef _DEBUG
//	strTable=tempTable.TableToString();
//	Tx::Core::Commonality::String().StringToClipboard(strTable);
//#endif
	int NO;
	for(iterDate = iDate.begin();iterDate != iDate.end();++iterDate)
	{
		itempdate = *iterDate;					
		TempCollect.clear();
		tempTable.Find(iDateCol,itempdate,TempCollect);//取得与报告期相同的纪录的位置。
		if(TempCollect.empty())
			continue;
		NO = 0;
		for(UterV = TempCollect.begin();UterV != TempCollect.end();++UterV)
		{
			NO += 1;
			tempTable.SetCell(pos,*UterV,NO);
		}
	}
/*#ifdef _DEBUG
	strTable=tempTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif*/	
	icount = tempTable.GetRowCount();
	int tempRow = resTable.GetRowCount();
	resTable.AddRow(icount);	
	for(int k = 0;k < icount;k++)
	{
//		NO += 1;
		tempTable.GetCell(pos,k,NO);
		resTable.SetCell(pos,tempRow+k,NO);
		//添加报告期
		tempTable.GetCell(iDateCol,k,itempdate);
		resTable.SetCell(iDateCol,tempRow+k,itempdate);
		//添加交易实体ID
		int tempTradeId;
		tempTable.GetCell(iTradeCol,k,tempTradeId);		
		resTable.SetCell(iTradeCol,tempRow+k,tempTradeId);
		//添加债券的名称和代码
		CString strname,strcode;
		if(IsBuySell)
		{
			int fundcount;
			tempTable.GetCell(iTradeCol+3,k,fundcount);//add by lijw 2008-05-23
			resTable.SetCell(iTradeCol+3,tempRow+k,fundcount);//add by lijw 2008-05-23
		}		
		tempTable.GetCell(iTradeCol+1,k,strcode);
		tempTable.GetCell(iTradeCol+2,k,strname);		
		resTable.SetCell(iTradeCol+1,tempRow+k,strcode);
		resTable.SetCell(iTradeCol+2,tempRow+k,strname);
		//添加所累加的数据。
		if (!IntCol.empty())
		{
			for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
			{
				tempTable.GetCell(*iterV,k,iData);
				if (iData < 0)
				{
					iData = Tx::Core::Con_intInvalid;
				}
				resTable.SetCell(*iterV,tempRow+k,iData);
			}
		} 
		if(!DoubleCol.empty())
		{
			for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
			{
				tempTable.GetCell(*dterV,k,dData);
				if (dData < 0)
				{
					dData = Tx::Core::Con_doubleInvalid;								
				}
				resTable.SetCell(*dterV,tempRow+k,dData);
			}
		}		
	}		
/*#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif*/	
	iTradeCol = tempTable.GetRowCount();//这是为了返回样本汇总的行数。
	return true;
}
//add by lijw 2009-02-16
//备注：此函数是专门计算交易佣金的样本汇总统计作准备
bool TxFund::AddUpRow4(Tx::Core::Table &resTable,		//存放结果表
					   std::vector<int> ColVector,		//根据哪些列进行统计  
					   std::vector<int> IntCol,			//需要相加的整型列
					   std::vector<int> DoubleCol,	//需要相加的double列
					   std::vector<int> iDate,			//报告期
					   int			  iCol,				//报告期所在的列。
					   int &			  iTradeCol,			//股票或者债券的交易实体ID所在的列
					   int			   sortCol,		//根据那一列进行排序。
					   int			   pos			//排名所在的列。
					   )	
{
	std::set<int>::iterator iterSet,iterTradeSet;
	std::set<int>::iterator iterTemp;
	std::set<int> iTradeSet;
//	int iTradeId;
	std::unordered_multimap<int,int> IdPosition;
	std::unordered_multimap<int,int>::iterator iterMulti;	
	std::vector<UINT> TempCollect;
	std::set<int> Intersection;
	std::vector<UINT>::iterator UterV;
	std::vector<int>::iterator iterV;
	std::vector<int>::iterator iterDate;
	std::vector<int>::iterator dterV;
	std::vector<int>::iterator iterCol;//参照列的专用变量

	std::set<int> deletePosition;
	//用于保存分类统计时的数据
	Tx::Core::Table tempTable;
	tempTable.CopyColumnInfoFrom(resTable);
	int Colpos;//程序中公共用的行变量。
	int iDateCol;//报告期的专用列号
	int iRow;//程序中公共用的行变量。
	int icount;
	int iData = 0;
	double dData = 0;
	int itempdate;
	std::unordered_map<int,int> iDataMap;//存放临时的整型值	
	std::unordered_map<int,double> dDataMap;//存放临时的double值
	//填充iDataMap和dDataMap
	for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
	{
		Colpos = *iterV;
		iDataMap.insert(std::make_pair(Colpos,0));
	}
	for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
	{
		Colpos = *dterV;
		dDataMap.insert(std::make_pair(Colpos,0));
	}
	//#if _DEBUG
	//	CString strTable;//为了测试用的
	//#endif 
	iDateCol = iCol;
	if(tempTable.GetRowCount() != 0)
	{
		tempTable.Clear();
		tempTable.Clone(resTable);
	}
	else
		tempTable.Clone(resTable);
	//#ifdef _DEBUG
	//	strTable=tempTable.TableToString();
	//	Tx::Core::Commonality::String().StringToClipboard(strTable);
	//#endif
	////根据报告期进行筛选
	//for(iterDate = iDate.begin();iterDate != iDate.end();++iterDate)
	//{
	//	itempdate = *iterDate;
	//	TempCollect.clear();
	//	tempTable.Find(iDateCol,itempdate,TempCollect);//取得与报告期相同的纪录的位置。
	//	if(TempCollect.empty())
	//		continue;			
	//	//取得在那些相同的行中全部的交易实体	
	//	if(!iTradeSet.empty())
	//		iTradeSet.clear();
	//	if(!IdPosition.empty())
	//		IdPosition.clear();
	//	for(UterV = TempCollect.begin();UterV != TempCollect.end();++UterV)
	//	{
	//		tempTable.GetCell(iTradeCol,*UterV,iTradeId);
	//		iTradeSet.insert(iTradeId);
	//		IdPosition.insert(make_pair(iTradeId,*UterV));			
	//	}		
	//	for(iterTradeSet = iTradeSet.begin();iterTradeSet != iTradeSet.end();++iterTradeSet)
	//	{
	//		iTradeId = *iterTradeSet;
	//		if (IdPosition.count(iTradeId) > 1)
	//		{
	//			/*while ((iterMulti = IdPosition.find(iTradeId)) != IdPosition.end())
	//			{
	//			Intersection.insert(iterMulti->second);
	//			IdPosition.erase(iterMulti);
	//			}*/
	//			//用下面的方法更为简便
	//			if(!Intersection.empty())
	//				Intersection.clear();
	//			for(iterMulti = IdPosition.lower_bound(iTradeId);iterMulti != IdPosition.upper_bound(iTradeId);++iterMulti)
	//			{
	//				Intersection.insert(iterMulti->second);					
	//			}
	//			//把保存各列值得map初始化为零
	//			if(!IntCol.empty() || !DoubleCol.empty())
	//			{
	//				for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
	//				{
	//					Colpos = *iterV;
	//					iDataMap[Colpos] = 0;
	//				}
	//				for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
	//				{
	//					Colpos = *dterV;
	//					dDataMap[Colpos] = 0;
	//				}
	//			}							
	//			//把报告期相同的行中，所需要相加的列进行相加。
	//			for(iterTemp = Intersection.begin();iterTemp != Intersection.end();++iterTemp)
	//			{
	//				iRow = *iterTemp;
	//				//需要相加的整型列进行相加。
	//				if(!IntCol.empty())
	//				{
	//					for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
	//					{
	//						Colpos = *iterV;
	//						tempTable.GetCell(Colpos,iRow,iData);
	//						if (iData < 0)
	//							continue;									
	//						iDataMap[Colpos] = iDataMap[Colpos] + iData;
	//					}
	//				}							
	//				//需要相加的double列进行相加。
	//				if(!DoubleCol.empty())
	//				{
	//					for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
	//					{
	//						Colpos = *dterV;
	//						tempTable.GetCell(Colpos,iRow,dData);
	//						if (dData < 0)
	//							continue;									
	//						dDataMap[Colpos] = dDataMap[Colpos] + dData;
	//					}
	//				}						
	//			}
	//			iRow = *(Intersection.begin());
	//			////如果买入卖出有需要合并的，改写它们的值，2008-05-23
	//			//int iSize = Intersection.size();				
	//			//把相加的各项的值放到tempTable里的其中一行当中,多余的行全部删掉.
	//			if(!IntCol.empty() || !DoubleCol.empty())
	//			{					
	//				//首先放整型列的值
	//				for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
	//				{
	//					Colpos = *iterV;						
	//					iData = iDataMap[Colpos];								
	//					tempTable.SetCell(Colpos,iRow,iData);
	//				}
	//				//其次放double列的值
	//				for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
	//				{
	//					Colpos = *dterV;
	//					dData= dDataMap[Colpos] ;
	//					tempTable.SetCell(Colpos,iRow,dData);
	//				}				
	//				//把多余的行删掉；
	//				for(iterTemp = --(Intersection.end());iterTemp != Intersection.begin();--iterTemp)
	//				{
	//					deletePosition.insert(*iterTemp);
	//				}
	//			}						
	//		} 				
	//	}
	//}
	////把要删除的行全部删掉。
	//std::set<int>::reverse_iterator revIter;
	//for(revIter = deletePosition.rbegin();revIter != deletePosition.rend();++revIter)
	//	tempTable.DeleteRow(*revIter);		
	//tempTable.Arrange();
#ifdef _DEBUG
	strTable=tempTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	std::set<CString> strNameSet;
	std::unordered_multimap<CString,int> strNamePosition;
	std::unordered_multimap<CString,int>::iterator iterNameMulti;
	std::set<CString>::iterator iterNameSet;
	//根据报告期进行筛选
	for(iterDate = iDate.begin();iterDate != iDate.end();++iterDate)
	{
		itempdate = *iterDate;
		TempCollect.clear();
		tempTable.Find(iDateCol,itempdate,TempCollect);//取得与报告期相同的纪录的位置。
		if(TempCollect.empty())
			continue;
		if(!strNameSet.empty())
			strNameSet.clear();
		if(!strNamePosition.empty())
			strNamePosition.clear();
		CString strName;
		for(UterV = TempCollect.begin();UterV != TempCollect.end();++UterV)
		{
			strName = _T("");
			tempTable.GetCell(iTradeCol+2,*UterV,strName);
			strNameSet.insert(strName);
			strNamePosition.insert(make_pair(strName,*UterV));			
		}
		for(iterNameSet = strNameSet.begin();iterNameSet != strNameSet.end();++iterNameSet)
		{
			strName = *iterNameSet;
			if (strNamePosition.count(strName) > 1)
			{			
				if(!Intersection.empty())
					Intersection.clear();
				for(iterNameMulti = strNamePosition.lower_bound(strName);iterNameMulti != strNamePosition.upper_bound(strName);++iterNameMulti)
				{
					Intersection.insert(iterNameMulti->second);					
				}
				//把保存各列值得map初始化为零
				if(!IntCol.empty() || !DoubleCol.empty())
				{
					for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
					{
						Colpos = *iterV;
						iDataMap[Colpos] = 0;
					}
					for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
					{
						Colpos = *dterV;
						dDataMap[Colpos] = 0;
					}
				}							
				//把报告期相同的行中，所需要相加的列进行相加。
				for(iterTemp = Intersection.begin();iterTemp != Intersection.end();++iterTemp)
				{
					iRow = *iterTemp;
					//需要相加的整型列进行相加。
					if(!IntCol.empty())
					{
						for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
						{
							Colpos = *iterV;
							tempTable.GetCell(Colpos,iRow,iData);
							if (iData < 0)
								continue;									
							iDataMap[Colpos] = iDataMap[Colpos] + iData;
						}
					}							
					//需要相加的double列进行相加。
					if(!DoubleCol.empty())
					{
						for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
						{
							Colpos = *dterV;
							tempTable.GetCell(Colpos,iRow,dData);
							if (dData < 0)
								continue;									
							dDataMap[Colpos] = dDataMap[Colpos] + dData;
						}
					}						
				}
				iRow = *(Intersection.begin());
				////如果买入卖出有需要合并的，改写它们的值，2008-05-23
				//int iSize = Intersection.size();				
				//把相加的各项的值放到tempTable里的其中一行当中,多余的行全部删掉.
				if(!IntCol.empty() || !DoubleCol.empty())
				{					
					//首先放整型列的值
					for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
					{
						Colpos = *iterV;						
						iData = iDataMap[Colpos];								
						tempTable.SetCell(Colpos,iRow,iData);
					}
					//其次放double列的值
					for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
					{
						Colpos = *dterV;
						dData= dDataMap[Colpos] ;
						tempTable.SetCell(Colpos,iRow,dData);
					}				
					//把多余的行删掉；
					for(iterTemp = --(Intersection.end());iterTemp != Intersection.begin();--iterTemp)
					{
						deletePosition.insert(*iterTemp);
					}
				}						
			} 				
		}
	}
	//把要删除的行全部删掉。
	std::set<int>::reverse_iterator revIter;
	for(revIter = deletePosition.rbegin();revIter != deletePosition.rend();++revIter)
		tempTable.DeleteRow(*revIter);		
	tempTable.Arrange();
#ifdef _DEBUG
	strTable=tempTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//把tempTable里的数据放到resTable
	//为了对重仓债里的重仓股进行排序，所以对tempTable进行排序，分别按照报告期、债券的市值
	MultiSortRule multisort;
	multisort.AddRule(iDateCol,true);
	multisort.AddRule(sortCol,false);
	tempTable.SortInMultiCol(multisort);
	tempTable.Arrange();
#ifdef _DEBUG
		strTable=tempTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	int NO;
	for(iterDate = iDate.begin();iterDate != iDate.end();++iterDate)
	{
		itempdate = *iterDate;					
		TempCollect.clear();
		tempTable.Find(iDateCol,itempdate,TempCollect);//取得与报告期相同的纪录的位置。
		if(TempCollect.empty())
			continue;
		NO = 0;
		for(UterV = TempCollect.begin();UterV != TempCollect.end();++UterV)
		{
			NO += 1;
			tempTable.SetCell(pos,*UterV,NO);
		}
	}
	/*#ifdef _DEBUG
	strTable=tempTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
	#endif*/	
	icount = tempTable.GetRowCount();
	int tempRow = resTable.GetRowCount();
	resTable.AddRow(icount);	
	for(int k = 0;k < icount;k++)
	{
		//		NO += 1;
		tempTable.GetCell(pos,k,NO);
		resTable.SetCell(pos,tempRow+k,NO);
		//添加报告期
		tempTable.GetCell(iDateCol,k,itempdate);
		resTable.SetCell(iDateCol,tempRow+k,itempdate);
		//添加交易实体ID
		int tempTradeId;
		tempTable.GetCell(iTradeCol,k,tempTradeId);		
		resTable.SetCell(iTradeCol,tempRow+k,tempTradeId);
		//添加债券的名称和代码
		CString strname,strcode;
		//if(IsBuySell)
		//{
		//	int fundcount;
		//	tempTable.GetCell(iTradeCol+3,k,fundcount);//add by lijw 2008-05-23
		//	resTable.SetCell(iTradeCol+3,tempRow+k,fundcount);//add by lijw 2008-05-23
		//}		
		tempTable.GetCell(iTradeCol+1,k,strcode);
		tempTable.GetCell(iTradeCol+2,k,strname);		
		resTable.SetCell(iTradeCol+1,tempRow+k,strcode);
		resTable.SetCell(iTradeCol+2,tempRow+k,strname);
		//添加所累加的数据。
		if (!IntCol.empty())
		{
			for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
			{
				tempTable.GetCell(*iterV,k,iData);
				if (iData < 0)
				{
					iData = Tx::Core::Con_intInvalid;
				}
				resTable.SetCell(*iterV,tempRow+k,iData);
			}
		} 
		if(!DoubleCol.empty())
		{
			for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
			{
				tempTable.GetCell(*dterV,k,dData);
				if (dData < 0)
				{
					dData = Tx::Core::Con_doubleInvalid;								
				}
				resTable.SetCell(*dterV,tempRow+k,dData);
			}
		}		
	}		
	/*#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
	#endif*/	
	iTradeCol = tempTable.GetRowCount();//这是为了返回样本汇总的行数。
	return true;
}
//add by lijw 2008-04-07
//备注：此函数是专门计算没有样本汇总分类统计作准备
bool TxFund::AddUpRow3(
					  Tx::Core::Table &resTable,		//存放结果表
					  std::vector<int> ColVector,		//根据哪些列进行统计  
					  std::vector<int> IntCol,			//需要相加的整型列
					  std::vector<int> DoubleCol,	//需要相加的double列
					  std::vector<int> iDate,			//报告期
					  int			  iCol,				//报告期所在的列。
					  int			   iEquityCol,				//基金净值所在的列。
					  int			   iFundIdCol,			//基金净值所在的列。
					  int 		      iTradeCol,		//股票或者债券的交易实体ID所在的列
					  int			   sortCol,		//根据那一列进行排序。
					  int			   pos,		//排名所在的列。
					  bool            IsBuySell    //判断是否是买入卖出的分组统计
					  )	
{
	std::set<int>::iterator iterSet,iterTradeSet;
	std::set<int>::iterator iterTemp;
	std::set<int> iIdSet;
	std::set<int> iTradeSet;
	int iTradeId;
	std::vector<int> rowVector;
	std::unordered_multimap<int,int> IdPosition;
	std::unordered_multimap<int,int>::iterator iterMulti;
	std::vector<UINT> PositionCollect;
	std::vector<UINT> TempCollect;
	std::set<int> Intersection;
	std::vector<UINT>::iterator UterV;
	std::vector<int>::iterator iterV;
	std::vector<int>::iterator iterDate;
	std::vector<int>::iterator dterV;
	std::vector<int>::iterator iterCol;//参照列的专用变量

	std::set<int> deletePosition;
	////用于保存分类统计时的数据
	Tx::Core::Table tempTable,staticTable;	
	staticTable.CopyColumnInfoFrom(resTable);
	tempTable.CopyColumnInfoFrom(staticTable);
	staticTable.Clone(resTable);
	int Colpos;//程序中公共用的行变量。
	int iDateCol;//报告期的专用列号
	int iIdCol;//参照列的专用列号
	int iRow;//程序中公共用的行变量。
	//int row = -1;//这是专门纪录tempTable里的行号。
	int icount;
	int iData = 0;
	double dData = 0;
	int itempdate;
	int itempId;
	std::unordered_map<int,int> iDataMap;//存放临时的整型值	
	std::unordered_map<int,double> dDataMap;//存放临时的double值
	std::unordered_map<int,double> dEquityMap;//用于存放基金净值的值
	//填充iDataMap和dDataMap、dEquityMap
	for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
	{
		Colpos = *iterV;
		iDataMap.insert(std::make_pair(Colpos,0));
	}
	for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
	{
		Colpos = *dterV;
		dDataMap.insert(std::make_pair(Colpos,0));
	}
	dEquityMap.insert(std::make_pair(iEquityCol,0));
//#if _DEBUG
//	CString strTable;//为了测试用的
//#endif 
	iDateCol = iCol;
	//根据那个参数列进行相加
	for(iterCol = ColVector.begin();iterCol != ColVector.end();++iterCol)
	{
		if(tempTable.GetRowCount() != 0)
		{
			tempTable.Clear();
			tempTable.Clone(staticTable);
		}
		else
			tempTable.Clone(staticTable);
		icount = tempTable.GetRowCount();
		iIdCol = *iterCol;
		if(!iIdSet.empty())
			iIdSet.clear();
		for(int i = 0;i < icount;i++)
		{
			tempTable.GetCell(iIdCol,i,itempId);
			iIdSet.insert(itempId);
		}		
		//根据ID进行相加
		for(iterSet = iIdSet.begin();iterSet != iIdSet.end();++iterSet)
		{
			itempId = *iterSet;					
			PositionCollect.clear();
			tempTable.Find(iIdCol,itempId,PositionCollect);//取得与公司ID相同的纪录的位置。
			if(PositionCollect.empty())
				continue;			
			//把vector里的值放在set里，是为了用Set里的find方法
			std::set<UINT> positionSet(PositionCollect.begin(),PositionCollect.end());
			//根据报告期进行筛选
			for(iterDate = iDate.begin();iterDate != iDate.end();++iterDate)
			{
				itempdate = *iterDate;
				TempCollect.clear();
				tempTable.Find(iDateCol,itempdate,TempCollect);//取得与报告期相同的纪录的位置。
				if(TempCollect.empty())
					continue;
				//把它们相同的值保存在Intersection
				if(!rowVector.empty())
					rowVector.clear();
				for(UterV = TempCollect.begin();UterV != TempCollect.end();++UterV)
				{
					if(positionSet.find(*UterV) != positionSet.end())
						rowVector.push_back(*UterV);
				}
				if(rowVector.empty())
					continue;
				//把公司ID和报告期相同的，但是基金ID不同的记录的基金净值加起来。
				dEquityMap[iEquityCol] = 0;
				int tempid;
				std::vector<int>::iterator Id_itor;
				std::set<int> fundId;
				if(!fundId.empty())
					fundId.clear();
				for(Id_itor = rowVector.begin();Id_itor != rowVector.end();++Id_itor)
				{
					double equity;
					tempTable.GetCell(iFundIdCol,*Id_itor,tempid);
					if(fundId.find(tempid) == fundId.end())
					{
						fundId.insert(tempid);
						tempTable.GetCell(iEquityCol,*Id_itor,equity);
						dEquityMap[iEquityCol] += equity;
					}
				}
				//取得在那些相同的行中全部的交易实体
				if(!iTradeSet.empty())
					iTradeSet.clear();
				if(!IdPosition.empty())
					IdPosition.clear();
				for(iterV = rowVector.begin();iterV != rowVector.end();++iterV)
				{
					tempTable.GetCell(iTradeCol,*iterV,iTradeId);
					iTradeSet.insert(iTradeId);
					IdPosition.insert(make_pair(iTradeId,*iterV));
					//把公司ID和报告期相同的，但是基金ID不同的记录的基金净值相加后的结果放到tempTable里
					tempTable.SetCell(iEquityCol,*iterV,dEquityMap[iEquityCol]);
					//如果是买入卖出的话，就把基金的支数全部写成1，如果后面有需要合并的，再改写它们，2008-05-23
					if(IsBuySell)
					{
						int fundCount = 1;
						tempTable.SetCell(iTradeCol+3,*iterV,fundCount);
					}
				}
//#ifdef _DEBUG
//				strTable=tempTable.TableToString();
//				Tx::Core::Commonality::String().StringToClipboard(strTable);
//#endif
				for(iterTradeSet = iTradeSet.begin();iterTradeSet != iTradeSet.end();++iterTradeSet)
				{
					iTradeId = *iterTradeSet;
					if (IdPosition.count(iTradeId) > 1)
					{
						if(!Intersection.empty())
							Intersection.clear();
						for(iterMulti = IdPosition.lower_bound(iTradeId);iterMulti != IdPosition.upper_bound(iTradeId);++iterMulti)
						{
							Intersection.insert(iterMulti->second);					
						}
						//把保存各列值得map初始化为零
						if(!IntCol.empty() || !DoubleCol.empty())
						{
							for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
							{
								Colpos = *iterV;
								iDataMap[Colpos] = 0;
							}
							for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
							{
								Colpos = *dterV;
								dDataMap[Colpos] = 0;
							}
						}							
						//把ID相同并且报告期相同的行中，所需要相加的列进行相加。
						for(iterTemp = Intersection.begin();iterTemp != Intersection.end();++iterTemp)
						{
							iRow = *iterTemp;
							//需要相加的整型列进行相加。
							if(!IntCol.empty())
							{
								for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
								{
									Colpos = *iterV;
									tempTable.GetCell(Colpos,iRow,iData);
									if (iData < 0)
										continue;									
									iDataMap[Colpos] = iDataMap[Colpos] + iData;
								}
							}							
							//需要相加的double列进行相加。
							if(!DoubleCol.empty())
							{
								for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
								{
									Colpos = *dterV;
									tempTable.GetCell(Colpos,iRow,dData);
									if (dData < 0)
										continue;									
									dDataMap[Colpos] = dDataMap[Colpos] + dData;
								}
							}						
						}
						iRow = *(Intersection.begin());
						//如果买入卖出有需要合并的，改写它们的值，2008-05-23
						int iSize = (int)Intersection.size();
						if(IsBuySell && iSize >0)
						{							
							tempTable.SetCell(iTradeCol+3,iRow,iSize);
						}
						//把相加的各项的值放到tempTable里的其中一行里.多余的行全部删掉.
						if(!IntCol.empty() || !DoubleCol.empty())
						{
							//首先放整型列的值
							for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
							{
								Colpos = *iterV;						
								iData = iDataMap[Colpos];								
								tempTable.SetCell(Colpos,iRow,iData);
							}
							//其次放double列的值
							for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
							{
								Colpos = *dterV;
								dData= dDataMap[Colpos] ;
								tempTable.SetCell(Colpos,iRow,dData);
							}
							//把多余的行删掉；
							for(iterTemp = --(Intersection.end());iterTemp != Intersection.begin();--iterTemp)
							{
								deletePosition.insert(*iterTemp);								
							}							
						}						
					} 							
				}			
			}
		}
//#ifdef _DEBUG
//		strTable=tempTable.TableToString();
//		Tx::Core::Commonality::String().StringToClipboard(strTable);
//#endif
		//把要删除的行全部删掉。
		std::set<int>::reverse_iterator revIter;
		for(revIter = deletePosition.rbegin();revIter != deletePosition.rend();++revIter)
			tempTable.DeleteRow(*revIter);		
		tempTable.Arrange();
		//把tempTable里的数据放到resTable
//		int tempTradeId;
		if(iDate.size() > 1)
		{
			Tx::Core::Table singleDateTable;
			std::vector<UINT>::iterator u_iterator;
			singleDateTable.CopyColumnInfoFrom(tempTable);
			MultiSortRule multisort2;
			int position,i_data;
			double d_data;
			int irow;
		    for(iterDate = iDate.begin();iterDate != iDate.end();++iterDate)
			{
				if(singleDateTable.GetRowCount() != 0)
				{
					singleDateTable.Clear();
					singleDateTable.CopyColumnInfoFrom(tempTable);
				}
				if(!TempCollect.empty())
					TempCollect.clear();
				tempTable.Find(iDateCol,*iterDate,TempCollect);//取得与报告期相同的纪录的位置。
				if(TempCollect.empty())
					continue;
				//把报告期相同的记录放到singleDateTable里
				irow = 0;				
				for(u_iterator = TempCollect.begin();u_iterator !=TempCollect.end();++u_iterator,irow++)
				{
					singleDateTable.AddRow();
					position = *u_iterator;
					tempTable.GetCell(iEquityCol,position,d_data);
					singleDateTable.SetCell(iEquityCol,irow,d_data);
					for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
					{
						tempTable.GetCell(*dterV,position,d_data);
						singleDateTable.SetCell(*dterV,irow,d_data);
					}
					for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
					{
						tempTable.GetCell(*iterV,position,i_data);
						singleDateTable.SetCell(*iterV,irow,i_data);
					}
					//添加报告期
					tempTable.GetCell(iDateCol,position,itempdate);
					singleDateTable.SetCell(iDateCol,irow,itempdate);
					//添加根据那一列进行统计的ID
					tempTable.GetCell(iIdCol,position,itempId);
					singleDateTable.SetCell(iIdCol,irow,itempId);
					//添加交易实体ID
					int tempTradeId;
					tempTable.GetCell(iTradeCol,position,tempTradeId);
					singleDateTable.SetCell(iTradeCol,irow,tempTradeId);
					//添加债券的名称和代码
					CString strname,strcode;
					if(IsBuySell)
					{
						int fundcount;
						tempTable.GetCell(iTradeCol+3,position,fundcount);//add by lijw 2008-05-23
						singleDateTable.SetCell(iTradeCol+3,irow,fundcount);//add by lijw 2008-05-23
					}
					tempTable.GetCell(iTradeCol+1,position,strcode);
					tempTable.GetCell(iTradeCol+2,position,strname);
					singleDateTable.SetCell(iTradeCol+1,irow,strcode);
					singleDateTable.SetCell(iTradeCol+2,irow,strname);
				}				
				multisort2.AddRule(iIdCol,true);
				multisort2.AddRule(sortCol,false);
				singleDateTable.SortInMultiCol(multisort2);
				singleDateTable.Arrange();
				int NO ;
				for(iterSet = iIdSet.begin();iterSet != iIdSet.end();++iterSet)
				{
					itempId = *iterSet;					
					PositionCollect.clear();
					singleDateTable.Find(iIdCol,itempId,PositionCollect);//取得与公司ID相同的纪录的位置。
					if(PositionCollect.empty())
						continue;
					NO = 0;
					for(UterV = PositionCollect.begin();UterV != PositionCollect.end();++UterV)
					{
						NO += 1;
						singleDateTable.SetCell(pos,*UterV,NO);
					}
				}
//#ifdef _DEBUG
//				strTable=singleDateTable.TableToString();
//				Tx::Core::Commonality::String().StringToClipboard(strTable);
//#endif
				//把singleDateTable里的数据拷贝到resTable;
				resTable.AppendTableByRow(singleDateTable);
			}
		}
		else
		{
			//为了对重仓债里的重仓股进行排序，所以对tempTable进行排序，分别按照ID、债券的市值
			MultiSortRule multisort;
			multisort.AddRule(iIdCol,true);
			multisort.AddRule(sortCol,false);
			tempTable.SortInMultiCol(multisort);
			tempTable.Arrange();
			int NO ;
			for(iterSet = iIdSet.begin();iterSet != iIdSet.end();++iterSet)
			{
				itempId = *iterSet;					
				PositionCollect.clear();
				tempTable.Find(iIdCol,itempId,PositionCollect);//取得与公司ID相同的纪录的位置。
				if(PositionCollect.empty())
					continue;
				NO = 0;
				for(UterV = PositionCollect.begin();UterV != PositionCollect.end();++UterV)
				{
					NO += 1;
					tempTable.SetCell(pos,*UterV,NO);
				}
			}
//#ifdef _DEBUG
//			strTable=tempTable.TableToString();
//			Tx::Core::Commonality::String().StringToClipboard(strTable);
//#endif
			icount = tempTable.GetRowCount();
			int tempRow = resTable.GetRowCount();
			resTable.AddRow(icount);	
			for(int k = 0;k < icount;k++)
			{
				//添加基金净值
				double tempEquity;
				tempTable.GetCell(iEquityCol,k,tempEquity);
				resTable.SetCell(iEquityCol,tempRow+k,tempEquity);
				tempTable.GetCell(pos,k,NO);
				resTable.SetCell(pos,tempRow+k,NO);
				//添加所累加的数据。
				if (!IntCol.empty())
				{
					for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
					{
						tempTable.GetCell(*iterV,k,iData);
						if (iData < 0)
						{
							iData = Tx::Core::Con_intInvalid;
						}
						resTable.SetCell(*iterV,tempRow+k,iData);
					}
				} 
				if(!DoubleCol.empty())
				{
					for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
					{
						tempTable.GetCell(*dterV,k,dData);
						if (dData < 0)
						{
							dData = Tx::Core::Con_doubleInvalid;								
						}
						resTable.SetCell(*dterV,tempRow+k,dData);
					}
				}
				//添加报告期
				tempTable.GetCell(iDateCol,k,itempdate);
				resTable.SetCell(iDateCol,tempRow+k,itempdate);
				//添加根据那一列进行统计的ID
				tempTable.GetCell(iIdCol,k,itempId);
				resTable.SetCell(iIdCol,tempRow+k,itempId);
				//添加交易实体ID
				int tempTradeId;
				tempTable.GetCell(iTradeCol,k,tempTradeId);
				resTable.SetCell(iTradeCol,tempRow+k,tempTradeId);
				//添加债券的名称和代码
				CString strname,strcode;
				if(IsBuySell)
				{
					int fundcount;
					tempTable.GetCell(iTradeCol+3,k,fundcount);//add by lijw 2008-05-23
					resTable.SetCell(iTradeCol+3,tempRow+k,fundcount);//add by lijw 2008-05-23
				}
				tempTable.GetCell(iTradeCol+1,k,strcode);
				tempTable.GetCell(iTradeCol+2,k,strname);
				resTable.SetCell(iTradeCol+1,tempRow+k,strcode);
				resTable.SetCell(iTradeCol+2,tempRow+k,strname);
			}
		}	
	}

//#ifdef _DEBUG
//	strTable=resTable.TableToString();
//	Tx::Core::Commonality::String().StringToClipboard(strTable);
//#endif
	return true;
}
bool TxFund::AddUpRow3(
					   Tx::Core::Table &resTable,		//存放结果表
					   std::vector<int> ColVector,		//根据哪些列进行统计  
					   std::vector<int> IntCol,			//需要相加的整型列
					   std::vector<int> DoubleCol,	//需要相加的double列
					   std::vector<int> iDate,			//报告期
					   int			  iCol,				//报告期所在的列。
					   int 		      iTradeCol,		//券商ID所在的列，并且它的后面必须是它的名称。
					   int			   sortCol,		//根据那一列进行排序。
					   int			   pos			//排名所在的列。
					   )	
{
	std::set<int>::iterator iterSet,iterTradeSet;
	std::set<int>::iterator iterTemp;
	std::set<int> iIdSet;
	std::set<int> iTradeSet;
	int iTradeId;
	std::vector<int> rowVector;
	std::unordered_multimap<int,int> IdPosition;
	std::unordered_multimap<int,int>::iterator iterMulti;
	std::vector<UINT> PositionCollect;
	std::vector<UINT> TempCollect;
	std::set<int> Intersection;
	std::vector<UINT>::iterator UterV;
	std::vector<int>::iterator iterV;
	std::vector<int>::iterator iterDate;
	std::vector<int>::iterator dterV;
	std::vector<int>::iterator iterCol;//参照列的专用变量

	std::set<int> deletePosition;
	////用于保存分类统计时的数据
	Tx::Core::Table tempTable,staticTable;	
	staticTable.CopyColumnInfoFrom(resTable);
	tempTable.CopyColumnInfoFrom(staticTable);
	staticTable.Clone(resTable);
	int Colpos;//程序中公共用的行变量。
	int iDateCol;//报告期的专用列号
	int iIdCol;//参照列的专用列号
	int iRow;//程序中公共用的行变量。
	int icount;
	int iData = 0;
	double dData = 0;
	int itempdate;
	int itempId;
	std::unordered_map<int,int> iDataMap;//存放临时的整型值	
	std::unordered_map<int,double> dDataMap;//存放临时的double值
	//填充iDataMap和dDataMap
	for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
	{
		Colpos = *iterV;
		iDataMap.insert(std::make_pair(Colpos,0));
	}
	for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
	{
		Colpos = *dterV;
		dDataMap.insert(std::make_pair(Colpos,0));
	}
#if _DEBUG
	CString strTable;//为了测试用的
#endif 
	iDateCol = iCol;
	//根据那个参数列进行相加
	for(iterCol = ColVector.begin();iterCol != ColVector.end();++iterCol)
	{
		if(tempTable.GetRowCount() != 0)
		{
			tempTable.Clear();
			tempTable.Clone(staticTable);
		}
		else
			tempTable.Clone(staticTable);
		icount = tempTable.GetRowCount();
		iIdCol = *iterCol;
		if(!iIdSet.empty())
			iIdSet.clear();
		for(int i = 0;i < icount;i++)
		{
			tempTable.GetCell(iIdCol,i,itempId);
			iIdSet.insert(itempId);
		}		
		//根据ID进行相加
		for(iterSet = iIdSet.begin();iterSet != iIdSet.end();++iterSet)
		{
			itempId = *iterSet;					
			PositionCollect.clear();
			tempTable.Find(iIdCol,itempId,PositionCollect);//取得与公司ID相同的纪录的位置。
			if(PositionCollect.empty())
				continue;			
			//把vector里的值放在set里，是为了用Set里的find方法
			std::set<UINT> positionSet(PositionCollect.begin(),PositionCollect.end());
			//根据报告期进行筛选
			for(iterDate = iDate.begin();iterDate != iDate.end();++iterDate)
			{
				itempdate = *iterDate;
				TempCollect.clear();
				tempTable.Find(iDateCol,itempdate,TempCollect);//取得与报告期相同的纪录的位置。
				if(TempCollect.empty())
					continue;
				//把它们相同的值保存在Intersection
				if(!rowVector.empty())
					rowVector.clear();
				for(UterV = TempCollect.begin();UterV != TempCollect.end();++UterV)
				{
					if(positionSet.find(*UterV) != positionSet.end())
						rowVector.push_back(*UterV);
				}
				if(rowVector.empty())
					continue;			
				//取得在那些相同的行中全部的交易实体
				if(!iTradeSet.empty())
					iTradeSet.clear();
				if(!IdPosition.empty())
					IdPosition.clear();
				for(iterV = rowVector.begin();iterV != rowVector.end();++iterV)
				{
					tempTable.GetCell(iTradeCol,*iterV,iTradeId);
					iTradeSet.insert(iTradeId);
					IdPosition.insert(make_pair(iTradeId,*iterV));					
				}				
				for(iterTradeSet = iTradeSet.begin();iterTradeSet != iTradeSet.end();++iterTradeSet)
				{
					iTradeId = *iterTradeSet;
					if (IdPosition.count(iTradeId) > 1)
					{
						if(!Intersection.empty())
							Intersection.clear();
						for(iterMulti = IdPosition.lower_bound(iTradeId);iterMulti != IdPosition.upper_bound(iTradeId);++iterMulti)
						{
							Intersection.insert(iterMulti->second);					
						}
						//把保存各列值得map初始化为零
						if(!IntCol.empty() || !DoubleCol.empty())
						{
							for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
							{
								Colpos = *iterV;
								iDataMap[Colpos] = 0;
							}
							for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
							{
								Colpos = *dterV;
								dDataMap[Colpos] = 0;
							}
						}							
						//把ID相同并且报告期相同的行中，所需要相加的列进行相加。
						for(iterTemp = Intersection.begin();iterTemp != Intersection.end();++iterTemp)
						{
							iRow = *iterTemp;
							//需要相加的整型列进行相加。
							if(!IntCol.empty())
							{
								for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
								{
									Colpos = *iterV;
									tempTable.GetCell(Colpos,iRow,iData);
									if (iData < 0)
										continue;									
									iDataMap[Colpos] = iDataMap[Colpos] + iData;
								}
							}							
							//需要相加的double列进行相加。
							if(!DoubleCol.empty())
							{
								for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
								{
									Colpos = *dterV;
									tempTable.GetCell(Colpos,iRow,dData);
									if (dData < 0)
										continue;									
									dDataMap[Colpos] = dDataMap[Colpos] + dData;
								}
							}						
						}

						CString m_strCompany;
						tempTable.GetCell(7,iRow,m_strCompany);
						//把相加的各项的值放到tempTable里的其中一行里.多余的行全部删掉.
						if(!IntCol.empty() || !DoubleCol.empty())
						{
							iRow = *(Intersection.begin());
							//首先放整型列的值
							for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
							{
								Colpos = *iterV;						
								iData = iDataMap[Colpos];								
								tempTable.SetCell(Colpos,iRow,iData);
							}
							//其次放double列的值
							for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
							{
								Colpos = *dterV;
								dData= dDataMap[Colpos] ;
								tempTable.SetCell(Colpos,iRow,dData);
							}
							//把多余的行删掉；
							for(iterTemp = --(Intersection.end());iterTemp != Intersection.begin();--iterTemp)
							{
								deletePosition.insert(*iterTemp);								
							}							
						}						
					} 							
				}			
			}
		}
		//把要删除的行全部删掉。
		std::set<int>::reverse_iterator revIter;
		for(revIter = deletePosition.rbegin();revIter != deletePosition.rend();++revIter)
			tempTable.DeleteRow(*revIter);		
		tempTable.Arrange();
#ifdef _DEBUG
		strTable=tempTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
		//added by zhangxs 20090107
		std::set<int> setDeleteRow;
		//选中基金公司统计时
		setDeleteRow.clear();
		std::vector<UINT> m_vecPositionCol;
		for(iterSet = iIdSet.begin();iterSet != iIdSet.end();++iterSet)
		{
			itempId = *iterSet;					
			m_vecPositionCol.clear();
			tempTable.Find(iIdCol,itempId,m_vecPositionCol);//取得与公司ID相同的纪录的位置。
			if(m_vecPositionCol.empty())
				continue;			
			//根据报告期进行筛选
			std::vector<UINT> m_vecPositionColDate;
			for(iterDate = iDate.begin();iterDate != iDate.end();++iterDate)
			{
				itempdate = *iterDate;
				m_vecPositionColDate.clear();
				tempTable.Find(iDateCol,itempdate,m_vecPositionColDate);//取得与报告期相同的纪录的位置。
				if(m_vecPositionColDate.empty())
					continue;
				//统计名称相同的记录
				CString m_strName;
				CString m_strCompName;
				std::set<int> setRowNo; //保存券商全称和简称Id不同的行
				for(std::vector<UINT>::iterator m_iter = m_vecPositionCol.begin();m_iter!=m_vecPositionCol.end();m_iter++)
				{
					double m_dJyyj = 0.0;
					double m_dGpjyl = 0.0;
					double m_dZqjyl = 0.0;
					double m_dHgjyl = 0.0;
					int m_iReport = 0;
					/*	if(setRowNo.find(*m_iter)!= setRowNo.end())
					continue;*/
					if(setDeleteRow.find(*m_iter)!= setDeleteRow.end())
						continue;
					tempTable.GetCell(7,*m_iter,m_strName);
					if(m_strName.GetLength()<1)
						continue;
					tempTable.GetCell(4,*m_iter,m_iReport);
					if(m_iReport < 1 || m_iReport != itempdate)
						continue;
					tempTable.GetCell(8,*m_iter,m_dJyyj);
					tempTable.GetCell(9,*m_iter,m_dGpjyl);
					tempTable.GetCell(10,*m_iter,m_dZqjyl);
					tempTable.GetCell(11,*m_iter,m_dHgjyl);
					if(m_dJyyj <1e-6 || m_dJyyj == Con_doubleInvalid)
						m_dJyyj = 0.0;
					if(m_dGpjyl <1e-6 || m_dGpjyl == Con_doubleInvalid)
						m_dGpjyl = 0.0;
					if(m_dZqjyl <1e-6 || m_dZqjyl == Con_doubleInvalid)
						m_dZqjyl = 0.0;
					if(m_dHgjyl <1e-6 || m_dHgjyl == Con_doubleInvalid)
						m_dHgjyl = 0.0;
					for(std::vector<UINT>::iterator iter = m_iter+1;iter!=m_vecPositionCol.end();iter++)
					{
						if(setDeleteRow.find(*m_iter)!= setDeleteRow.end())
							continue;
						int m_iReportDate = 0;
						tempTable.GetCell(4,*iter,m_iReportDate);
						if(m_iReport < 1 || m_iReportDate != itempdate)
							continue;
						tempTable.GetCell(7,*iter,m_strCompName);
						if(!m_strName.Compare(m_strCompName))
						{
							double m_dJy = 0.0;
							double m_dGp = 0.0;
							double m_dZq = 0.0;
							double m_dHg = 0.0;
							tempTable.GetCell(8,*iter,m_dJy);
							tempTable.GetCell(9,*iter,m_dGp);
							tempTable.GetCell(10,*iter,m_dZq);
							tempTable.GetCell(11,*iter,m_dHg);
							if( m_dJy > 0.0 )
								m_dJyyj += m_dJy;
							if( m_dGp > 0.0 )
								m_dGpjyl += m_dGp;
							if( m_dZq > 0.0 )
								m_dZqjyl += m_dZq;
							if( m_dHg > 0.0 )
								m_dHgjyl += m_dHg;
							setRowNo.insert(*iter);
							setDeleteRow.insert(*iter);
						}
					}
					tempTable.SetCell(8,*m_iter,m_dJyyj);
					tempTable.SetCell(9,*m_iter,m_dGpjyl);
					tempTable.SetCell(10,*m_iter,m_dZqjyl);
					tempTable.SetCell(11,*m_iter,m_dHgjyl);
				}
			}		
		}
		//把要删除的行全部删掉。
		for(revIter = setDeleteRow.rbegin();revIter != setDeleteRow.rend();++revIter)
			tempTable.DeleteRow(*revIter);		
		tempTable.Arrange();
#ifdef _DEBUG
		strTable=tempTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
		//把tempTable里的数据放到resTable
		if(iDate.size() > 1)
		{
			Tx::Core::Table singleDateTable;
			std::vector<UINT>::iterator u_iterator;
			singleDateTable.CopyColumnInfoFrom(tempTable);
			MultiSortRule multisort2;
			int position,i_data;
			double d_data;
			int irow;
			for(iterDate = iDate.begin();iterDate != iDate.end();++iterDate)
			{
				if(singleDateTable.GetRowCount() != 0)
				{
					singleDateTable.Clear();
					singleDateTable.CopyColumnInfoFrom(tempTable);
				}
				if(!TempCollect.empty())
					TempCollect.clear();
				tempTable.Find(iDateCol,*iterDate,TempCollect);//取得与报告期相同的纪录的位置。
				if(TempCollect.empty())
					continue;
				//把报告期相同的记录放到singleDateTable里
				irow = 0;
				for(u_iterator = TempCollect.begin();u_iterator !=TempCollect.end();++u_iterator,irow++)
				{
					singleDateTable.AddRow();
					position = *u_iterator;				
					for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
					{
						tempTable.GetCell(*dterV,position,d_data);
						singleDateTable.SetCell(*dterV,irow,d_data);
					}
					for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
					{
						tempTable.GetCell(*iterV,position,i_data);
						singleDateTable.SetCell(*iterV,irow,i_data);
					}
					//添加报告期
					tempTable.GetCell(iDateCol,position,itempdate);
					singleDateTable.SetCell(iDateCol,irow,itempdate);
					//添加根据那一列进行统计的ID
					tempTable.GetCell(iIdCol,position,itempId);
					singleDateTable.SetCell(iIdCol,irow,itempId);
					//添加债券的名称和代码
					CString strname;
					tempTable.GetCell(iTradeCol+1,position,strname);
					singleDateTable.SetCell(iTradeCol+1,irow,strname);
				}				
				multisort2.AddRule(iIdCol,true);
				multisort2.AddRule(sortCol,false);
				singleDateTable.SortInMultiCol(multisort2);
				singleDateTable.Arrange();
				int NO ;
				for(iterSet = iIdSet.begin();iterSet != iIdSet.end();++iterSet)
				{
					itempId = *iterSet;					
					PositionCollect.clear();
					singleDateTable.Find(iIdCol,itempId,PositionCollect);//取得与公司ID相同的纪录的位置。
					if(PositionCollect.empty())
						continue;
					NO = 0;
					for(UterV = PositionCollect.begin();UterV != PositionCollect.end();++UterV)
					{
						NO += 1;
						singleDateTable.SetCell(pos,*UterV,NO);
					}
				}
#ifdef _DEBUG
				strTable=singleDateTable.TableToString();
				Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
				//把singleDateTable里的数据拷贝到resTable;
				resTable.AppendTableByRow(singleDateTable);
			}
		}
		else
		{
			//为了对重仓债里的重仓股进行排序，所以对tempTable进行排序，分别按照ID、债券的市值
			MultiSortRule multisort;
			multisort.AddRule(iIdCol,true);
			multisort.AddRule(sortCol,false);
			tempTable.SortInMultiCol(multisort);
			tempTable.Arrange();
			int NO ;
			for(iterSet = iIdSet.begin();iterSet != iIdSet.end();++iterSet)
			{
				itempId = *iterSet;					
				PositionCollect.clear();
				tempTable.Find(iIdCol,itempId,PositionCollect);//取得与公司ID相同的纪录的位置。
				if(PositionCollect.empty())
					continue;
				NO = 0;
				for(UterV = PositionCollect.begin();UterV != PositionCollect.end();++UterV)
				{
					NO += 1;
					tempTable.SetCell(pos,*UterV,NO);
				}
			}
#ifdef _DEBUG
			strTable=tempTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
			icount = tempTable.GetRowCount();
			int tempRow = resTable.GetRowCount();
			resTable.AddRow(icount);	
			for(int k = 0;k < icount;k++)
			{
				//添加基金净值
				tempTable.GetCell(pos,k,NO);
				resTable.SetCell(pos,tempRow+k,NO);
				//添加所累加的数据。
				if (!IntCol.empty())
				{
					for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
					{
						tempTable.GetCell(*iterV,k,iData);
						if (iData < 0)
						{
							iData = Tx::Core::Con_intInvalid;
						}
						resTable.SetCell(*iterV,tempRow+k,iData);
					}
				} 
				if(!DoubleCol.empty())
				{
					for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
					{
						tempTable.GetCell(*dterV,k,dData);
						if (dData < 0)
						{
							dData = Tx::Core::Con_doubleInvalid;								
						}
						resTable.SetCell(*dterV,tempRow+k,dData);
					}
				}
				//添加报告期
				tempTable.GetCell(iDateCol,k,itempdate);
				resTable.SetCell(iDateCol,tempRow+k,itempdate);
				//添加根据那一列进行统计的ID
				tempTable.GetCell(iIdCol,k,itempId);
				resTable.SetCell(iIdCol,tempRow+k,itempId);
				//添加债券的名称和代码
				CString strname;
				tempTable.GetCell(iTradeCol+1,k,strname);
				resTable.SetCell(iTradeCol+1,tempRow+k,strname);
			}
		}	
	}

#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	return true;
}
//add by lijw 2008-04-03
//备注：此函数是专门计算各行值得累加。为样本的分类统计作准备
bool TxFund::AddUpRow(Tx::Core::Table &resTable,		//存放结果表
					  int iStyle,						//样本分类的方式
					  std::vector<int> ColVector,		//根据哪些列进行统计  
					  std::vector<int> IntCol,			//需要相加的整型列
					  std::vector<int> DoubleCol,	//需要相加的double列
					  std::vector<int> iDate,			//报告期
					  int			   iCol,				//报告期所在的列。
					  int			   iEquityCol,				//基金净值所在的列。
					  int			   iFundIdCol,			//基金净值所在的列。
					  int &			   iTradeCol,			//股票或者债券的交易实体ID所在的列
					  int			   sortCol,		//根据那一列进行排序。
					  int			   pos,		//排名所在的列。
					  bool            IsBuySell    //判断是否是买入卖出的分组统计
					  )	
{
	
	//之所以把它们分开计算，而没有一个个计算，是为了进行样本汇总时，方便计算
	//并且当按多个分类方式进行统计时，可以同时计算多个分类方式
	switch(iStyle)
	{
	case 3://管理公司ID
	case 5://基金风格
	case 7://管理公司ID和基金风格
	case 9://托管银行ID   	
	case 11://管理公司ID和托管银行ID
	case 13://基金风格和托管银行ID
	case 15://基金风格和托管银行ID和管理公司ID
		AddUpRow3(resTable,ColVector,IntCol,DoubleCol,iDate,iCol,iEquityCol,iFundIdCol,iTradeCol,sortCol,pos,IsBuySell);
		break;
	case 17://样本汇总
		AddUpRow2(resTable,ColVector,IntCol,DoubleCol,iDate,iCol,iTradeCol,sortCol,pos,IsBuySell);
		break;
	case 19://管理公司ID和样本汇总
	case 21://基金风格和样本汇总
	case 25://样本汇总和托管银行ID
	case 23://基金风格和样本汇总和管理公司ID
	case 27://样本汇总和托管银行ID和管理公司ID
	case 29://基金风格和托管银行ID和样本汇总
	case 31://基金风格和托管银行ID和样本汇总和管理公司
		{
			Tx::Core::Table tempTable2;
			tempTable2.CopyColumnInfoFrom(resTable);
			tempTable2.Clone(resTable);
			AddUpRow3(resTable,ColVector,IntCol,DoubleCol,iDate,iCol,iEquityCol,iFundIdCol,iTradeCol,sortCol,pos,IsBuySell);
			AddUpRow2(tempTable2,ColVector,IntCol,DoubleCol,iDate,iCol,iTradeCol,sortCol,pos,IsBuySell);
			int icount = tempTable2.GetRowCount();
			int ideleteRow = icount - iTradeCol;
			tempTable2.DeleteRow(0,ideleteRow);
			tempTable2.Arrange();
			resTable.AppendTableByRow(tempTable2,false);
		}
		break;
	}
	return true;
}
// add by lijw 2008-05-09  下面的注释全部是它的重载函数的注释
bool TxFund::AddUpRow(Tx::Core::Table &resTable,		//存放结果表
					  int iStyle,						//样本分类的方式
					  std::vector<int> ColVector,		//根据哪些列进行统计  
					  std::vector<int> IntCol,			//需要相加的整型列
					  std::vector<int> DoubleCol,	//需要相加的double列
					  std::vector<int> iDate,			//报告期
					  int			   iCol,				//报告期所在的列。
					  int &			   iTradeCol,			//券商ID所在的列，并且它的前面必须是它的名称。
					  int			   sortCol,		//根据那一列进行排序。
					  int			   pos			//排名所在的列。
					  )	
{

	//之所以把它们分开计算，而没有一个个计算，是为了进行样本汇总时，方便计算
	//并且当按多个分类方式进行统计时，可以同时计算多个分类方式
	switch(iStyle)
	{
	case 3://管理公司ID
	case 5://基金风格
	case 7://管理公司ID和基金风格
	case 9://托管银行ID   	
	case 11://管理公司ID和托管银行ID
	case 13://基金风格和托管银行ID
	case 15://基金风格和托管银行ID和管理公司ID
		AddUpRow3(resTable,ColVector,IntCol,DoubleCol,iDate,iCol,iTradeCol,sortCol,pos);
		break;
	case 17://样本汇总
		{
			resTable.InsertCol(iTradeCol+1,Tx::Core::dtype_val_string);
			for(int i = 0;i < 4;i++)
				DoubleCol[i] = DoubleCol[i] + 1;
		    AddUpRow2(resTable,ColVector,IntCol,DoubleCol,iDate,iCol,iTradeCol,sortCol+1,pos);
			resTable.DeleteCol(7);//之所以这样做是为了不想再写AddUpRow2的重载函数。
		}
		break;
	case 19://管理公司ID和样本汇总
	case 21://基金风格和样本汇总
	case 25://样本汇总和托管银行ID
	case 23://基金风格和样本汇总和管理公司ID
	case 27://样本汇总和托管银行ID和管理公司ID
	case 29://基金风格和托管银行ID和样本汇总
	case 31://基金风格和托管银行ID和样本汇总和管理公司
		{
			Tx::Core::Table tempTable2;
			tempTable2.CopyColumnInfoFrom(resTable);
			tempTable2.Clone(resTable);
			AddUpRow3(resTable,ColVector,IntCol,DoubleCol,iDate,iCol,iTradeCol,sortCol,pos);
			int itempCol = iTradeCol+1;
			tempTable2.InsertCol(itempCol,Tx::Core::dtype_val_string);
			for(int i = 0;i < 4;i++)
				DoubleCol[i] = DoubleCol[i] + 1;
			int iTempCount = ColVector.size();
			for (int i = 0;i < iTempCount;i++)
			{
				ColVector[i] += 1;
			}
			sortCol = sortCol + 1;
			AddUpRow2(tempTable2,ColVector,IntCol,DoubleCol,iDate,iCol,iTradeCol,sortCol,pos);
			tempTable2.DeleteCol(itempCol);//之所以这样做是为了不想再写AddUpRow2的重载函数。
			int icount = tempTable2.GetRowCount();
			int ideleteRow = icount - iTradeCol;
			tempTable2.DeleteRow(0,ideleteRow);
			tempTable2.Arrange();
			resTable.AppendTableByRow(tempTable2,false);
		}
		break;
	}
	return true;
}
//--------------获取货币基金基金七日年化-----------------
bool	TxFund::GetMmfFundAcuNvr( std::vector< int > iSecurity,std::vector< int > iDates, Tx::Core::Table_Indicator* pResTable )
{
	if ( pResTable == NULL )
		return false;
	pResTable->Clear();
	//m_dMmfFundAcuNvrColMap.clear();
	//m_dMmfFundAcuNvrRowMap.clear();
	if ( 0 == iSecurity.size()|| 0 == iDates.size() )
		return false;
	//std::vector<int> iSecurity( nSecurity.begin(),nSecurity.end());
	//std::vector<int> iDates( nDates.begin(),nDates.end());

	pResTable->AddCol( Tx::Core::dtype_int4 );		//ID
	int nCol = 1;
	for ( std::vector< int >::iterator iter = iDates.begin(); iter != iDates.end(); ++iter )
	{
		pResTable->AddCol( Tx::Core::dtype_double );
		//m_dMmfFundAcuNvrColMap.insert( std::pair<int,int>( *iter,nCol++));
	}	
	int nRow = 0;
	for ( std::vector< int >::iterator iter = iSecurity.begin(); iter != iSecurity.end(); ++iter,++nRow)
	{
		pResTable->AddRow();
		pResTable->SetCell( 0,nRow,*iter );
		//m_dMmfFundAcuNvrRowMap.insert( std::pair<int,int>( *iter,nRow++));
	}	
	{
		// 必然选择了累计净值
		int iSize = sizeof(int) * ( 4 /*请求类型*/ + 1 /*券个数*/ + pResTable->GetRowCount() + 1 /*日期个数*/ + iDates.size());
		LPBYTE pBuffer = new BYTE [iSize];
		if (pBuffer == NULL)
			return false;
		LPBYTE pWrite = pBuffer;
		memset(pBuffer,0,iSize);
		int iType = 4;
		memcpy_s(pWrite,iSize,&iType,sizeof(int));
		pWrite += sizeof(int);
		int nSecSize = (int)pResTable->GetRowCount();
		memcpy_s(pWrite,iSize,&nSecSize,sizeof(int));
		pWrite += sizeof(int);
		for (int i=0;i<nSecSize;i++)
		{
			int iId;
			pResTable->GetCell(0,i,iId);
			memcpy_s(pWrite,iSize,&iId,sizeof(int));
			pWrite += sizeof(int);
		}

		nSecSize = (int)iDates.size();
		memcpy_s(pWrite,iSize,&nSecSize,sizeof(int));
		pWrite += sizeof(int);

		for (int i=0;i<nSecSize;i++)
		{
			memcpy_s(pWrite,iSize,&iDates[i],sizeof(int));
			pWrite += sizeof(int);
		}
		LPCTSTR lpUrl = CORE_GETSYSINFO->GetServerAddr(_T("File"),_T("FundMMF"));
        //LPCTSTR lpUrl = _T("http://192.168.5.1/MMFService/ServerHandler.ashx");
		Tx::Drive::Http::CSyncUpload upload;
		int iStart = ::GetTickCount();
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

			iStart = ::GetTickCount();
			LPBYTE pRecv = lpData;
			UINT nParseCount = pResTable->GetRowCount() * iDates.size();
			float fValue = 0.0;
			double dValue = 0.0;
			for (UINT i=0;i < nParseCount; i++)
			{
				memcpy_s(&fValue,sizeof(float),pRecv,sizeof(float));
				pRecv += sizeof(float);
				if (fabs(fValue - Tx::Core::Con_floatInvalid) < 0.00001)
					pResTable->SetCell(i%iDates.size() + 1,i/iDates.size(),Tx::Core::Con_doubleInvalid);
				else
				{
					dValue = (double)fValue;
					pResTable->SetCell(i%iDates.size() + 1,i/iDates.size(),dValue);
				}
			}
			delete []lpData;
			lpData = NULL;
			iEnd = ::GetTickCount();
			TRACE(_T("\r\nParse Result Cost Time %d(ms)\r\n"),iEnd-iStart);
		}
		delete[] pBuffer;
		pBuffer = NULL;
	}
#ifdef _DEBUG
	CString strTable1=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable1);
#endif
	return true;
}

//--------------获取货币基金万份收益-----------------
bool	TxFund::GetMmfFundNvr( std::vector< int > iSecurity,std::vector< int > iDates, Tx::Core::Table_Indicator* pResTable )
{
	if ( pResTable == NULL )
		return false;
	pResTable->Clear();
	//m_MmfFundNvrRowMap.clear();
	m_MmfFundNvrColMap.clear();

	if ( 0 == iSecurity.size()|| 0 == iDates.size())
		return false;
	//std::vector<int> iSecurity( nSecurity.begin(),nSecurity.end());
	//std::vector<int> iDates( nDates.begin(),nDates.end());
	pResTable->AddCol( Tx::Core::dtype_int4 );		//ID
	int nCol = 1;
	for ( std::vector< int >::iterator iter = iDates.begin(); iter != iDates.end(); ++iter )
	{
		pResTable->AddCol( Tx::Core::dtype_double );
		m_MmfFundNvrColMap.insert( std::pair<int,int>( nCol++,*iter));

	}	
	int nRow = 0;
	for ( std::vector< int >::iterator iter = iSecurity.begin(); iter != iSecurity.end(); ++iter,++nRow)
	{
		pResTable->AddRow();
		pResTable->SetCell( 0,nRow,*iter );
		//m_MmfFundNvrRowMap.insert( std::pair<int,int>( *iter,nRow++));
	}	
	{
		// 必然选择了单位净值
		int iSize = sizeof(int) * ( 1 /*请求类型*/ + 1 /*券个数*/ + pResTable->GetRowCount() + 1 /*日期个数*/ + iDates.size());
		LPBYTE pBuffer = new BYTE [iSize];
		if (pBuffer == NULL)
			return false;

		LPBYTE pWrite = pBuffer;
		memset(pBuffer,0,iSize);
		int iType = 3;
		memcpy_s(pWrite,iSize,&iType,sizeof(int));
		pWrite += sizeof(int);
		int nSecSize = (int)pResTable->GetRowCount();
		memcpy_s(pWrite,iSize,&nSecSize,sizeof(int));
		pWrite += sizeof(int);
		for (int i=0;i<nSecSize;i++)
		{
			int iId;
			pResTable->GetCell(0,i,iId);
			memcpy_s(pWrite,iSize,&iId,sizeof(int));
			pWrite += sizeof(int);
		}

		nSecSize = (int)iDates.size();
		memcpy_s(pWrite,iSize,&nSecSize,sizeof(int));
		pWrite += sizeof(int);

		for (int i=0;i<nSecSize;i++)
		{
			memcpy_s(pWrite,iSize,&iDates[i],sizeof(int));
			pWrite += sizeof(int);
		}
		LPCTSTR lpUrl = CORE_GETSYSINFO->GetServerAddr(_T("File"),_T("FundMMF"));
        //LPCTSTR lpUrl = _T("http://192.168.5.1/MMFService/ServerHandler.ashx");
		Tx::Drive::Http::CSyncUpload upload;
		int iStart = ::GetTickCount();
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
			iStart = ::GetTickCount();
			LPBYTE pRecv = lpData;
			UINT nParseCount = pResTable->GetRowCount() * iDates.size();
			float fValue = 0.0;
			double dValue = 0.0;
			for (UINT i=0;i < nParseCount; i++)
			{
				memcpy_s(&fValue,sizeof(float),pRecv,sizeof(float));
				pRecv += sizeof(float);
				if (fabs(fValue - Tx::Core::Con_floatInvalid) < 0.00001)
					pResTable->SetCell(i%iDates.size() + 1,i/iDates.size(),Tx::Core::Con_doubleInvalid);
				else
				{
					dValue = (double)fValue;
					pResTable->SetCell(i%iDates.size() + 1,i/iDates.size(),dValue);
				}
			}
			delete []lpData;
			lpData = NULL;
			iEnd = ::GetTickCount();
			TRACE(_T("\r\nParse Result Cost Time %d(ms)\r\n"),iEnd-iStart);
		}
		delete[] pBuffer;
		pBuffer = NULL;
	}
#ifdef _DEBUG
	CString strTable1=pResTable->TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable1);
#endif
	return true;		
}

	//add by lijw 2008-05-13
	//货币基金收益
		bool	TxFund::StatFundCurrencyIncome(
			Tx::Core::Table_Indicator	&resTable,
			std::vector<int>	&iSecurityId,
			int		iStartDate,
			int		iEndDate,
			int		iTimeCycle,			//时间周期:日周月季年区间
			int		iStatIndicator,		//统计指标:万份基金单位收益、最近七年化收益率
			bool	IsOut,				//是直接输出还是累计计算
			std::vector<CString> &vDates,
			std::vector<CString> &vColName
			)
		{
			ProgressWnd prw;
			UINT pid=prw.AddItem(1,_T("货币基金收益统计..."),0.0);
			prw.Show(1000);
			//默认的返回值状态
			bool result = true;
			if(iStatIndicator==0)
				return false;	
			//判断是否是货币基金		
			std::vector<int>::iterator iterV;
			std::vector<int> tempVector;
			for(iterV = iSecurityId.begin();iterV != iSecurityId.end();++iterV)
			{
				GetSecurityNow(*iterV);
				if(m_pSecurity != NULL)
				{
					//BUG:13241   2012-09-27    理财债券类似于货币基金
					Tx::Data::FundNewInfo *pFundNewInfo = m_pSecurity->GetFundNewInfo();
					if(pFundNewInfo == NULL)
						continue;
					if(m_pSecurity->IsFund_Currency() || pFundNewInfo->style_id == 21)
						tempVector.push_back(*iterV);
				}
			}
			iSecurityId.clear();
			iSecurityId = tempVector;
			
			if(iSecurityId.empty())
			{
				//添加进度条
				prw.SetPercent(pid,1.0);
				return false;
			}
			//清空数据
			m_txTable.Clear();
			//日期类型==>选择
			int dateB;
			switch(iTimeCycle)
			{
			case 1:
				{
					COleDateTime dttemp;
					dttemp.SetDate(iStartDate/10000,iStartDate%10000/100,iStartDate%100);
					int iDayofWeek = dttemp.GetDayOfWeek();
					int iSpan = (iDayofWeek + 1) % 7;
					COleDateTimeSpan dtspan(iSpan,0,0,0);
					dttemp -= dtspan;
					dateB = dttemp.GetYear()*10000 + dttemp.GetMonth()*100 + dttemp.GetDay();
					/*dateB = Tx::Core::TxDate::CalculateFridayOfWeek(iStartDate);
					dateB = Tx::Core::TxDate::CalculateDateOffsetDays (dateB,-7);
					dateB--;*/
				}			
				break;
			case 2:
				{
					dateB = Tx::Core::TxDate::CalculateEndOfMonth (iStartDate);
					dateB = Tx::Core::TxDate::CalculateDateOffsetMonths(dateB,-1);
					//zway//直接用--不对//dateB--;
					dateB = Tx::Core::TxDate::CalculateDateOffsetDays(dateB,-1);
				}			
				break;
			case 3:
				{
					dateB = Tx::Core::TxDate::CalculateEndOfQuarter (iStartDate);
					dateB = Tx::Core::TxDate::CalculateDateOffsetMonths(dateB,-3);
					//zway//直接用--不对//dateB--;
					dateB = Tx::Core::TxDate::CalculateDateOffsetDays(dateB,-1);
				}
				break;
			case 4:
				{
					dateB = Tx::Core::TxDate::CalculateEndOfYear(iStartDate);
					dateB = Tx::Core::TxDate::CalculateDateOffsetMonths(dateB,-12);
					//zway//直接用--不对//dateB--;
					dateB = Tx::Core::TxDate::CalculateDateOffsetDays(dateB,-1);
				}
				break;
			case 0:
			case 5:
				dateB = iStartDate;
				break;
			default:
				break;
			}
			std::vector<int> tempDateVector;
			for (int i = dateB;i <= iEndDate;)
			{
				tempDateVector.push_back(i);
				i = Tx::Core::TxDate::CalculateDateOffsetDays (i,1);
			}
            Tx::Core::Table_Indicator tempTable1,tempTable2;
            result = GetMmfFundNvr(iSecurityId,tempDateVector,&tempTable1);    //万份单位收益
			if (result == false)
			{
				//添加进度条
				prw.SetPercent(pid,1.0);
				return false;
			}
			result = GetMmfFundAcuNvr(iSecurityId,tempDateVector,&tempTable2);  //7日年化收益率
			if (result == false)
			{
				//添加进度条
				prw.SetPercent(pid,1.0);
				return false;
			}
			if (tempTable1.GetRowCount() != tempTable2.GetRowCount() || tempTable1.GetColCount() != tempTable2.GetColCount())
			{
				ASSERT(0);
				//添加进度条
				prw.SetPercent(pid,1.0);
				return false;
			}
			m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
			m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
			m_txTable.AddParameterColumn(Tx::Core::dtype_double);
			m_txTable.AddParameterColumn(Tx::Core::dtype_double);
			if (tempTable1.GetRowCount() < 0 || tempTable2.GetRowCount() < 0)
			{
				//添加进度条
				prw.SetPercent(pid,1.0);
				return false;
			}
			int icoc = tempTable1.GetRowCount();
			int icoc2 = (int)tempDateVector.size();
			int iTradeId;
			int irow3 = -1;
			double dData1,dData2;
			int idate;
			for (int j = 0; j < icoc;++j)
			{
				tempTable1.GetCell(0,j,iTradeId);
				for (int k = 1;k < icoc2 + 1;++k)
				{					
					tempTable1.GetCell(k,j,dData1);
					//tempTable1.GetCell(k,j,idate);
					idate = m_MmfFundNvrColMap[k];
					tempTable2.GetCell(k,j,dData2);					
					if (dData1 < 0 && dData2 < 0)
						continue;
					m_txTable.AddRow();
					irow3++;
					// add by wangyc 20100304
					//根据要求去掉负值的过滤，允许万份单位收益为负值
					//if (dData1 < 0)
					//{
					//	dData1 = Tx::Core::Con_doubleInvalid;
					//}
					if (dData2 < 0)
					{
						dData2 = Tx::Core::Con_doubleInvalid;
					}
					m_txTable.SetCell(0,irow3,iTradeId);
					m_txTable.SetCell(1,irow3,idate);
					m_txTable.SetCell(2,irow3,dData1);
					m_txTable.SetCell(3,irow3,dData2);
				}
			}
//			//准备样本集=第一参数列:基金交易实体ID,int型
//			m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
//			////日期参数=第二参数列;开始日期, int型
//			//m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
//			//序号=第三参数列;截至日期, int型
//			m_txTable.AddParameterColumn(Tx::Core::dtype_int4);		
//			int varCount=2;			//参数个数
//			if(iStatIndicator&1)	 
//			{
//				GetIndicatorDataNow(30401005);
//				result = m_pLogicalBusiness->SetIndicatorIntoTable(m_pIndicatorData,varCfg,varCount,m_txTable);
//				if(result==false)
//				{
//					//添加进度条
//					prw.SetPercent(pid,1.0);
//					return false;
//				}
//			}
//			if(iStatIndicator&2)
//			{
//				GetIndicatorDataNow(30401006);
//				result = m_pLogicalBusiness->SetIndicatorIntoTable(m_pIndicatorData,varCfg,varCount,m_txTable);
//				if(result==false)
//				{
//					//添加进度条
//					prw.SetPercent(pid,1.0);
//					return false;
//				}
//			}
//			//根据之前的设置进行数据读取，结果数据存放在table中
//			result = m_pLogicalBusiness->GetData(m_txTable,true);
//			if(result==false)
//			{
//				//添加进度条
//				prw.SetPercent(pid,1.0);
//				return false;
//			}
//			//添加进度条
//			prw.SetPercent(pid,0.3);
//#ifdef _DEBUG
//			CString strTable1=m_txTable.TableToString();
//			Tx::Core::Commonality::String().StringToClipboard(strTable1);
//#endif

			Tx::Core::Table_Indicator tempTable;
#ifdef _DEBUG
			CString strTable1=m_txTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable1);
#endif
			//把tempTable的数据放到resTable里
			resTable.AddCol(Tx::Core::dtype_int4);//为存放交易实体增加一列	
			resTable.AddCol(Tx::Core::dtype_val_string);//存放基金名称
			resTable.AddCol(Tx::Core::dtype_val_string);//存放基金代码		
			//添加进度条
			prw.SetPercent(pid,0.6);
#ifdef _DEBUG
			strTable1=m_txTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable1);
#endif
            //下面是把tempTable的数据放到resTable里。
			int icount,tradeId;
			std::vector<UINT> vecInstiID2;			
			std::vector<UINT>::iterator iteID,iteID2;
			//std::vector<CString>::iterator iterStr;
			CString strName,strCode;
			int enddate,irow = 0;
			//double dData1,dData2;//dData1表示每万份基金单位收益
			CString strDate;
			int tempCol;
			std::unordered_map<int,int> TitleToColMap;
			std::vector<int> DataColV;
			std::set<int> TempDateSet;
			std::set<int>::iterator iterSet;
            switch(iTimeCycle)
			{
			case 0://按时间周期为日（不管是直接输出还是累计计算都是下面的代码）
				{
					////为了取得相应的结果，所以对resTable进行排序，分别按照基金交易实体ID、截止日期
					MultiSortRule multisort;
					multisort.AddRule(0,true);
					multisort.AddRule(1,true);
					m_txTable.SortInMultiCol(multisort);
                    m_txTable.Arrange();
					if(!vecInstiID2.empty())
						vecInstiID2.clear();
					for (int i = 0;i < (int)m_txTable.GetRowCount();i++)
					{
						m_txTable.GetCell(1,i,enddate);
						iterSet = TempDateSet.find(enddate);
						if(iterSet == TempDateSet.end())
						{
							TempDateSet.insert(enddate);
						}					
					}
					int k = 1;
					for(iterSet = TempDateSet.begin();iterSet != TempDateSet.end();++iterSet,k++)
					{
						enddate = *iterSet;
						TitleToColMap.insert(std::make_pair(enddate,k));
						strDate.Format(_T("%d-%d-%d"),enddate/10000,enddate/100%100,enddate%100);
						vDates.push_back(strDate);
					}
					icount = static_cast<int>(vDates.size());
					if(iStatIndicator&1 && iStatIndicator&2)
						icount = icount*2;
					for(int i = 0;i < icount;i++)
						resTable.AddCol(Tx::Core::dtype_double);					
					int itempCount = 0;
					for(iterV = iSecurityId.begin(),irow = -1;iterV != iSecurityId.end();++iterV)
					{						
						if(!vecInstiID2.empty())
							vecInstiID2.clear();
						m_txTable.Find(0,*iterV,vecInstiID2);
						if (vecInstiID2.empty())
						{
							continue;
						}
						resTable.AddRow();
						irow++;
						itempCount = 0;
						if(!DataColV.empty())
							DataColV.clear();
						for(iteID = vecInstiID2.begin();iteID != vecInstiID2.end();++iteID,itempCount++)
						{
							if(itempCount == 0)
							{
								//先把样本的名称和代码添上。
								m_txTable.GetCell(0,*iteID,tradeId);
								GetSecurityNow(tradeId);
								if(m_pSecurity != NULL)
								{
									strName = m_pSecurity->GetName();
									strCode = m_pSecurity->GetCode();
									resTable.SetCell(1,irow,strName);
									resTable.SetCell(2,irow,strCode);
								}
								resTable.SetCell(0,irow,tradeId);
							}
							//添加数据
							if(iStatIndicator&1 && iStatIndicator&2)
							{
								m_txTable.GetCell(2,*iteID,dData1);
								m_txTable.GetCell(3,*iteID,dData2);
								m_txTable.GetCell(1,*iteID,enddate);
								DataColV.push_back(TitleToColMap[enddate]);
								tempCol = 2*TitleToColMap[enddate] + 1;
								resTable.SetCell(tempCol,irow,dData1);
								resTable.SetCell(tempCol+1,irow,dData2);
							}							
							//else
							//{
							//	m_txTable.GetCell(2,*iteID,dData1);
							//	m_txTable.GetCell(1,*iteID,enddate);
							//	DataColV.push_back(TitleToColMap[enddate]);
							//	tempCol = TitleToColMap[enddate] + 2;
							//	resTable.SetCell(tempCol,irow,dData1);
							//}
							///////////////////////////////////////////////////////////////////////////////
							//wangyc fix bug on 20090911  对于计算万份收益和7日年化收益做不同的处理
							else if(iStatIndicator&1)	//只计算万份收益
							{
								m_txTable.GetCell(2,*iteID,dData1);
								m_txTable.GetCell(1,*iteID,enddate);
								DataColV.push_back(TitleToColMap[enddate]);
								tempCol = TitleToColMap[enddate] + 2;
								resTable.SetCell(tempCol,irow,dData1);
							}
							else						//只计算7日年化收益率
							{
								m_txTable.GetCell(3,*iteID,dData1);
								m_txTable.GetCell(1,*iteID,enddate);
								DataColV.push_back(TitleToColMap[enddate]);
								tempCol = TitleToColMap[enddate] + 2;
								resTable.SetCell(tempCol,irow,dData1);
							}
							///////////////////////////////////////////////////////////////////////////////

						}
						std::vector<int>::iterator tempIter;
						for(int j = 1;j <= (int)vDates.size();j++)
						{
							tempIter = find(DataColV.begin(),DataColV.end(),j);
							if(tempIter == DataColV.end())
							{
								//添加数据
								if(iStatIndicator&1 && iStatIndicator&2)
								{
									tempCol = 2*j + 1;
									resTable.SetCell(tempCol,irow,Tx::Core::Con_doubleInvalid);
									resTable.SetCell(tempCol+1,irow,Tx::Core::Con_doubleInvalid);
								}
								else
								{
									tempCol = j + 2;
									resTable.SetCell(tempCol,irow,Tx::Core::Con_doubleInvalid);
								}
							}
						}						
					}						
				}
				break;
			case 1://按时间周期为周	
			case 2://按时间周期为月			
			case 3://按时间周期为季度		
			case 4://按时间周期为年				
				AddUpCalculator(resTable,m_txTable,iSecurityId,iStartDate,iEndDate,iStatIndicator,iTimeCycle,IsOut,vDates,vColName);
				break;
			case 5://按时间周期为区间
				{
					////为了取得相应的结果，所以对resTable进行排序，分别按照基金交易实体ID、截止日期
					MultiSortRule multisort;
					multisort.AddRule(0,true);
					multisort.AddRule(1,true);
					m_txTable.SortInMultiCol(multisort);
					m_txTable.Arrange();
#ifdef _DEBUG
					CString strTable3=m_txTable.TableToString();
					Tx::Core::Commonality::String().StringToClipboard(strTable3);
#endif
					strDate.Format(_T("%d-%d-%d"),iEndDate/10000,iEndDate/100%100,iEndDate%100);
					vDates.push_back(strDate);
					//根据徐总的要求，数据按照精确的日期来取，不取相近的日期的数据。
					if(IsOut)
					{
						icount = 1;
						if(iStatIndicator&1 && iStatIndicator&2)
							icount = icount*2;
						for(int i = 0;i < icount;i++)
							resTable.AddCol(Tx::Core::dtype_double);
						UINT iColCount2 = m_txTable.GetColCount();
						UINT* nColArray2 = new UINT[iColCount2];
						for(int i = 0; i < (int)iColCount2; i++)
						{
							nColArray2[i] = i;
						}
						std::vector<int> tempVector;
						tempVector.push_back(iEndDate);
						if(tempTable.GetRowCount() != 0)
							tempTable.Clear();
						tempTable.CopyColumnInfoFrom(m_txTable);
						m_txTable.EqualsAt(tempTable,nColArray2,iColCount2,1,tempVector);
#ifdef _DEBUG
						CString strTable2=tempTable.TableToString();
						Tx::Core::Commonality::String().StringToClipboard(strTable2);
#endif
						if(tempTable.GetRowCount() == 0)
						{
							//添加进度条
							prw.SetPercent(pid,1.0);
							delete [] nColArray2;
							nColArray2 = NULL;
							return false;
						}
						delete [] nColArray2;
						nColArray2 = NULL;
						double dTempData1 = 0,dTempData2 = 0;
						for(iterV = iSecurityId.begin(),irow = -1;iterV != iSecurityId.end();++iterV)
						{						
							if(!vecInstiID2.empty())
								vecInstiID2.clear();
							tempTable.Find(0,*iterV,vecInstiID2);
							if (vecInstiID2.empty())
							{
								continue;
							}
							resTable.AddRow();
							irow++;
							//先把样本的名称和代码添上。
							iteID = vecInstiID2.begin();
							tempTable.GetCell(0,*iteID,tradeId);
							GetSecurityNow(tradeId);
							if(m_pSecurity != NULL)
							{
								strName = m_pSecurity->GetName();
								strCode = m_pSecurity->GetCode();
								resTable.SetCell(1,irow,strName);
								resTable.SetCell(2,irow,strCode);
							}
							resTable.SetCell(0,irow,tradeId);													
							//if(iStatIndicator&1 && iStatIndicator&2)
							//{
							//	m_txTable.GetCell(2,*iteID,dData1);
							//	m_txTable.GetCell(3,*iteID,dData2);
							//}
							//else
							//{
							//	m_txTable.GetCell(2,*iteID,dData1);
							//}
							////添加数据						
							//if(iStatIndicator&1 && iStatIndicator&2)
							//{
							//	resTable.SetCell(3,irow,dData1);
							//	resTable.SetCell(4,irow,dData2);
							//}
							//else
							//{
							//	resTable.SetCell(3,irow,dData1);
							//}
							///////////////////////////////////////////////////////////////////////////////
							//wangyc fix bug on 20090911  对于计算万份收益和7日年化收益做不同的处理
							//if(iStatIndicator&1 && iStatIndicator&2)
							//{
							//	m_txTable.GetCell(2,*iteID,dData1);
							//	m_txTable.GetCell(3,*iteID,dData2);
							//}
							//else if(iStatIndicator&1)
							//{
							//	m_txTable.GetCell(2,*iteID,dData1);
							//}
							//else
							//{
							//	m_txTable.GetCell(3,*iteID,dData1);
							//}
							// zway 2010-08-04 for mantis #869, 上面代码用错变量了(m_txTable 是没有做 EqualsAt 的)
							if(iStatIndicator&1 && iStatIndicator&2)
							{
								tempTable.GetCell(2,*iteID,dData1);
								tempTable.GetCell(3,*iteID,dData2);
							}
							else if(iStatIndicator&1)
							{
								tempTable.GetCell(2,*iteID,dData1);
							}
							else
							{
								tempTable.GetCell(3,*iteID,dData1);
							}
							// zway end of for mantis #869
							//添加数据						
							if(iStatIndicator&1 && iStatIndicator&2)
							{
								resTable.SetCell(3,irow,dData1);
								resTable.SetCell(4,irow,dData2);
							}
							else
							{
								resTable.SetCell(3,irow,dData1);
							}
							///////////////////////////////////////////////////////////////////////////////
						}
					}
					else
					{						
						icount = 1;
						if(iStatIndicator&1 && iStatIndicator&2)
							icount = icount*2;
						for(int i = 0;i < icount;i++)
							resTable.AddCol(Tx::Core::dtype_double);
						double dTempData1 = 0,dTempData2 = 0;
						for(iterV = iSecurityId.begin(),irow = -1;iterV != iSecurityId.end();++iterV)
						{						
							if(!vecInstiID2.empty())
								vecInstiID2.clear();
							m_txTable.Find(0,*iterV,vecInstiID2);
							if (vecInstiID2.empty())
							{
								continue;
							}							
							resTable.AddRow();
							irow++;
							//先把样本的名称和代码添上。
							iteID = vecInstiID2.begin();
							m_txTable.GetCell(0,*iteID,tradeId);
							GetSecurityNow(tradeId);
							if(m_pSecurity != NULL)
							{
								strName = m_pSecurity->GetName();
								strCode = m_pSecurity->GetCode();
								resTable.SetCell(1,irow,strName);
								resTable.SetCell(2,irow,strCode);
							}
							resTable.SetCell(0,irow,tradeId);							
							dTempData1 = 0,dTempData2 = 0;
							//刘鹏 2011-10-11,BUG 8183,货币式基金阶段收益算法调整为：∏（1+每日万分收益/10000）-1 
							const int ciMoneyFundNumber = 10000;//万
							dData1 = 1,dData2 = 0;
							//添加数据
							for(iteID = vecInstiID2.begin();iteID != vecInstiID2.end();++iteID)
							{
								//添加数据						
								if(iStatIndicator&1 && iStatIndicator&2 || iStatIndicator == 1)
								{
									m_txTable.GetCell(2,*iteID,dTempData1);
									dData1 *= (1+dTempData1/ciMoneyFundNumber);
								}
							}
							dData1 = (dData1 - 1)*ciMoneyFundNumber;
							//添加数据						
							if(iStatIndicator&1 && iStatIndicator&2)
							{
								resTable.SetCell(3,irow,dData1);
								dData2 = Tx::Core::Con_doubleInvalid;
								resTable.SetCell(4,irow,dData2);
							}
							else if(iStatIndicator == 2)
							{
								dData1 = Tx::Core::Con_doubleInvalid;
								resTable.SetCell(3,irow,dData1);
							}
							else
							{
								resTable.SetCell(3,irow,dData1);
							}
						}
					}											
				}
				break;
			default:
				break;
			}				
			//添加进度条
			prw.SetPercent(pid,0.7);
			vColName.push_back(_T("名称"));
			vColName.push_back(_T("代码"));
			if(iStatIndicator&1 && iStatIndicator&2)
			{
				CString tmpStr;
				int isize = vDates.size();
				for(int j = 0;j < isize;j++)
				{					
					tmpStr.Format(_T("万元收益"));
					vColName.push_back(tmpStr);
					tmpStr.Format(_T("七日年化收益率"));
					vColName.push_back(tmpStr);
				}
			}
			
			//添加进度条
			prw.SetPercent(pid,1.0);
#ifdef _DEBUG
			strTable1=resTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable1);
#endif
			return true;
		}
//add by lijw 2008-05-16
//是专门为了计算货币基金收益里的累计计算。
		bool TxFund::AddUpCalculator(
			Tx::Core::Table_Indicator &resTable,   //存放结果的表
			Tx::Core::Table_Indicator &m_txTable2,  //传入要处理的数据的源表
			std::vector<int>	&iSecurityId,	  //样本的交易实体ID
			int		iStartDate,					  //起始日期
			int		iEndDate,					 //截止日期
			int		iStatIndicator,		        //统计指标:万份基金单位收益、最近七年化收益率
			int		iTimeCycle,		        	//时间周期:日周月季年区间
			bool	IsOut,				//是直接输出还是累计计算
			std::vector<CString> &vDates,
			std::vector<CString> &vColName
			)
		{
			int iCol,tradeId;
			std::vector<UINT> vecInstiID2;			
			std::vector<UINT>::iterator iteID,iteID2;
			std::vector<int>::iterator iterV;
			CString strName,strCode;
			int irow = -1;
			double dData1,dData2;//dData1表示每万份基金单位收益
			CString strDate;
			int tempCol;

			iCol = m_txTable.GetColCount();
			UINT* nColArray = new UINT[iCol];
			for(UINT i=0;i<(UINT)iCol;i++)
				nColArray[i]=i;				
			Tx::Core::Table_Indicator m_txTable;
			//m_txTable.Clear();
			m_txTable.Clone(m_txTable2);
			int dateB,dateE;
			//switch(iTimeCycle)
			//{
			//case 1:
			//	{
			//		dateB = Tx::Core::TxDate::CalculateFridayOfWeek(iStartDate);
			//		dateB = Tx::Core::TxDate::CalculateDateOffsetDays (dateB,-7);				
			//	}			
			//	break;
			//case 2:
			//	{
			//		dateB = Tx::Core::TxDate::CalculateEndOfMonth (iStartDate);
			//		dateB = Tx::Core::TxDate::CalculateDateOffsetMonths(dateB,-1);
			//	}			
			//	break;
			//case 3:
			//	{
			//		dateB = Tx::Core::TxDate::CalculateEndOfQuarter (iStartDate);
			//		dateB = Tx::Core::TxDate::CalculateDateOffsetMonths(dateB,-3);
			//	}
			//	break;
			//case 4:
			//	{
			//		dateB = Tx::Core::TxDate::CalculateEndOfYear(iStartDate);
			//		dateB = Tx::Core::TxDate::CalculateDateOffsetMonths(dateB,-12);
			//	}
			//	break;
			//default:
			//	break;
			//}
			////以截至日期为标准进行取区间的值。
			//m_txTable2.Between(m_txTable,nColArray,iCol,1,dateB-1,iEndDate,true,true);
			//if(m_txTable.GetRowCount() == 0)
			//{				
			//	delete [] nColArray;
			//	nColArray = NULL;
			//	return false;
			//}
			MultiSortRule multisort;
			multisort.AddRule(0,true);
			multisort.AddRule(1,true);
			m_txTable.SortInMultiCol(multisort);
			Tx::Core::Table_Indicator tempTable1,tempTable2;
			tempTable1.CopyColumnInfoFrom(m_txTable);
			tempTable2.CopyColumnInfoFrom(m_txTable);			
			std::unordered_map<int,int> TitleToCol;
			std::unordered_map<int,int>::iterator iterMap,iterMap2,iterMap3;
			int TitleCount = 0;
			std::vector<int> DataColV;
			std::unordered_map<int,int> DateRangeMap;
			for(iterV = iSecurityId.begin();iterV != iSecurityId.end();++iterV)
			{
				if(!vecInstiID2.empty())
					vecInstiID2.clear();
				m_txTable.Find(0,*iterV,vecInstiID2);
				if(vecInstiID2.empty())
					continue;
				if(!DataColV.empty())
					DataColV.clear();
				resTable.AddRow(1);
				irow++ ;
				if(tempTable1.GetRowCount() != 0 )
				{
					tempTable1.Clear();							
					tempTable1.CopyColumnInfoFrom(m_txTable);							
				}		
				iteID = vecInstiID2.begin();
				iteID2 = vecInstiID2.end() - 1;
				m_txTable.SubDrill(0,*iteID,m_txTable.GetColCount()-1,*iteID2,tempTable1);
#ifdef _DEBUG
				CString strTable1=tempTable1.TableToString();
				Tx::Core::Commonality::String().StringToClipboard(strTable1);
#endif				
				switch(iTimeCycle)
				{
				case 1:
					{
						dateB = Tx::Core::TxDate::CalculateFridayOfWeek(iStartDate);
						dateE = dateB;
						dateB = Tx::Core::TxDate::CalculateDateOffsetDays (dateB,-7);				
					}			
					break;
				case 2:
					{
						dateB = Tx::Core::TxDate::CalculateEndOfMonth (iStartDate);
						dateE = dateB;
						dateB = Tx::Core::TxDate::CalculateDateOffsetMonths(dateB,-1);
					}			
					break;
				case 3:
					{
						dateB = Tx::Core::TxDate::CalculateEndOfQuarter (iStartDate);
						dateE = dateB;
						dateB = Tx::Core::TxDate::CalculateDateOffsetMonths(dateB,-3);
					}
					break;
				case 4:
					{
						dateB = Tx::Core::TxDate::CalculateEndOfYear(iStartDate);
						dateE = dateB;
						dateB = Tx::Core::TxDate::CalculateDateOffsetMonths(dateB,-12);
					}
					break;
				default:
					break;
				}
				/*if(dateB > iEndDate)
				{
					dateB = iStartDate;
					dateE = iEndDate;
				}
				else
				{
					dateE = dateB;
					dateB = iStartDate-1;			
				}*/
				for (; dateE <= iEndDate;)
				{
					DateRangeMap.insert(std::make_pair(dateB,dateE));
					if(tempTable2.GetRowCount() != 0)
					{
						tempTable2.Clear();
						tempTable2.CopyColumnInfoFrom(tempTable1);
					}								
					//zway//直接用+1不对//tempTable1.Between(tempTable2,nColArray,iCol,1,dateB+1,dateE,true,true);
					tempTable1.Between(tempTable2,nColArray,iCol,1,Tx::Core::TxDate::CalculateDateOffsetDays(dateB,1),dateE,true,true);
#ifdef _DEBUG
					CString strTable5=tempTable2.TableToString();
					Tx::Core::Commonality::String().StringToClipboard(strTable5);
#endif
					if (tempTable2.GetRowCount() == 0)
					{
						dateB = dateE;
						//dateB = Tx::Core::TxDate::CalculateDateOffsetDays(dateB,1);
						CalculatorEndDate(iTimeCycle,dateE,dateB);
						continue;
					}
					if (IsOut)
					{			
						//首先添加交易实体和名称，代码。
						tempTable2.GetCell(0,0,tradeId);
						resTable.SetCell(0,irow,tradeId);
						GetSecurityNow(tradeId);
						if(m_pSecurity != NULL)
						{
							strName = m_pSecurity->GetName();
							strCode = m_pSecurity->GetCode();
							resTable.SetCell(1,irow,strName);
							resTable.SetCell(2,irow,strCode);
						}
						int tempEndDate;
						//tempTable2.GetCell(1,tempTable2.GetRowCount()-1,tempEndDate);
						tempEndDate = dateE;	// zway for mantis #1778
						iterMap = TitleToCol.find(tempEndDate);
						if (iterMap == TitleToCol.end())
						{
							TitleCount++;
							if (!TitleToCol.empty())
							{
								iterMap2 = TitleToCol.lower_bound(tempEndDate);
								if(iStatIndicator&1 && iStatIndicator&2)
								{									
									if (iterMap2 == TitleToCol.end())
									{
										resTable.AddCol(Tx::Core::dtype_double);
										resTable.AddCol(Tx::Core::dtype_double);									
										TitleToCol.insert(std::make_pair(tempEndDate,TitleCount));
									}
									else
									{
										int iColData = iterMap2->second;										
										iterMap3 = TitleToCol.insert(iterMap2,std::make_pair(tempEndDate,iColData));	
										resTable.InsertCol(2*iColData+1,Tx::Core::dtype_double);
										resTable.InsertCol(2*iColData+2,Tx::Core::dtype_double);
										for (iterMap3++;iterMap3 != TitleToCol.end();++iterMap3)
										{
											iColData++;
											iterMap3->second = iColData;
										}
									}					

								}
								else
								{									
									if (iterMap2 == TitleToCol.end())
									{
										resTable.AddCol(Tx::Core::dtype_double);					
										TitleToCol.insert(std::make_pair(tempEndDate,TitleCount));
									}
									else
									{
										int iColData = iterMap2->second;
										iterMap3 =TitleToCol.insert(iterMap2,std::make_pair(tempEndDate,iColData));										
										resTable.InsertCol(iColData+2,Tx::Core::dtype_double);
										for (iterMap3++;iterMap3 != TitleToCol.end();++iterMap3)
										{
											iColData++;
											iterMap3->second = iColData;;
										}
									}															
								}	
							} 
							else
							{
								if(iStatIndicator&1 && iStatIndicator&2)
								{
									resTable.AddCol(Tx::Core::dtype_double);
									resTable.AddCol(Tx::Core::dtype_double);									
									TitleToCol.insert(std::make_pair(tempEndDate,TitleCount));
								} 
								else
								{
									resTable.AddCol(Tx::Core::dtype_double);
									//resTable.AddCol(Tx::Core::dtype_double);									
									TitleToCol.insert(std::make_pair(tempEndDate,TitleCount));
								}						
							}
													
							/*strDate.Format(_T("%d-%d-%d"),tempEndDate/10000,tempEndDate/100%100,tempEndDate%100);
							vDates.push_back(strDate);*/							
							if (irow > 0)
							{
								for (int i = 0;i < irow;i++)
								{
									if(iStatIndicator&1 && iStatIndicator&2)
									{										
										tempCol = 2*TitleToCol[tempEndDate] + 1;
										resTable.SetCell(tempCol,i,Tx::Core::Con_doubleInvalid);
										resTable.SetCell(tempCol+1,i,Tx::Core::Con_doubleInvalid);
									}
									else
									{
										tempCol = TitleToCol[tempEndDate] + 2;
										resTable.SetCell(tempCol,i,Tx::Core::Con_doubleInvalid);
									}
								}
							}						
						}
#ifdef _DEBUG
						CString strTable3=resTable.TableToString();
						Tx::Core::Commonality::String().StringToClipboard(strTable3);
#endif
						//添加数据
						if(iStatIndicator&1 && iStatIndicator&2)
						{
							tempTable2.GetCell(2,tempTable2.GetRowCount()-1,dData1);
							tempTable2.GetCell(3,tempTable2.GetRowCount()-1,dData2);
							tempCol = 2*TitleToCol[tempEndDate] + 1;
							resTable.SetCell(tempCol,irow,dData1);
							resTable.SetCell(tempCol+1,irow,dData2);
							DataColV.push_back(tempCol);
							DataColV.push_back(tempCol+1);
						}
						//else
						//{
						//	tempTable2.GetCell(2,tempTable2.GetRowCount()-1,dData1);
						//	tempCol = TitleToCol[tempEndDate] + 2;
						//	resTable.SetCell(tempCol,irow,dData1);
						//	DataColV.push_back(tempCol);
						//}
						/////////////////////////////////////////////////////////////////////////////////////////////////
						//wangyc fix bug on 20090911 修改关于单独计算万份基金单位收益和七日年化收益计算结果都是万份收益的bug
						else if(iStatIndicator&1)	//只计算万份收益
						{
							tempTable2.GetCell(2,tempTable2.GetRowCount()-1,dData1);
							tempCol = TitleToCol[tempEndDate] + 2;
							resTable.SetCell(tempCol,irow,dData1);
							DataColV.push_back(tempCol);
						}
						else					//只计算7日年化收益
						{
							tempTable2.GetCell(3,tempTable2.GetRowCount()-1,dData1);
							tempCol = TitleToCol[tempEndDate] + 2;
							resTable.SetCell(tempCol,irow,dData1);
							DataColV.push_back(tempCol);
						}
						////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
						strTable=resTable.TableToString();
						Tx::Core::Commonality::String().StringToClipboard(strTable3);
#endif
					} 
					else
					{
						double dTotalData1 = 0,dTotalData2 = 0;
						int tempEndDate;
						for(int k = 0;k < (int)tempTable2.GetRowCount();k++)
						{				
							if(k == 0)
							{
								//首先添加交易实体和名称，代码。
								tempTable2.GetCell(0,0,tradeId);
								resTable.SetCell(0,irow,tradeId);
								GetSecurityNow(tradeId);
								if(m_pSecurity != NULL)
								{
									strName = m_pSecurity->GetName();
									strCode = m_pSecurity->GetCode();
									resTable.SetCell(1,irow,strName);
									resTable.SetCell(2,irow,strCode);
								}						
								//tempTable2.GetCell(1,tempTable2.GetRowCount()-1,tempEndDate);
								tempEndDate = dateE;	// zway for mantis #1778
								iterMap = TitleToCol.find(tempEndDate);								
								if (iterMap == TitleToCol.end())
								{
									TitleCount++;
									if (!TitleToCol.empty())
									{
										iterMap2 = TitleToCol.lower_bound(tempEndDate);
										if(iStatIndicator&1 && iStatIndicator&2)
										{											
											if (iterMap2 == TitleToCol.end())
											{
												resTable.AddCol(Tx::Core::dtype_double);
												resTable.AddCol(Tx::Core::dtype_double);									
												TitleToCol.insert(std::make_pair(tempEndDate,TitleCount));
											}
											else
											{
												int iColData = iterMap2->second;
												iterMap3 =TitleToCol.insert(iterMap2,std::make_pair(tempEndDate,iColData));												
												resTable.InsertCol(2*iColData+1,Tx::Core::dtype_double);
												resTable.InsertCol(2*iColData+2,Tx::Core::dtype_double);
												for (iterMap3++;iterMap3 != TitleToCol.end();++iterMap3)
												{
													iColData++;
													iterMap3->second = iColData;
												}
											}
										}
										else
										{											
											if (iterMap2 == TitleToCol.end())
											{
												resTable.AddCol(Tx::Core::dtype_double);					
												TitleToCol.insert(std::make_pair(tempEndDate,TitleCount));
											}
											else
											{
												int iColData = iterMap2->second;
												iterMap3 = TitleToCol.insert(iterMap2,std::make_pair(tempEndDate,iColData));												
												resTable.InsertCol(iColData+2,Tx::Core::dtype_double);
												for (iterMap3++;iterMap3 != TitleToCol.end();++iterMap3)
												{
													iColData++;
													iterMap3->second = iColData;
												}
											}
										}
									} 
									else
									{
										if(iStatIndicator&1 && iStatIndicator&2)
										{
											resTable.AddCol(Tx::Core::dtype_double);
											resTable.AddCol(Tx::Core::dtype_double);									
											TitleToCol.insert(std::make_pair(tempEndDate,TitleCount));
										}
										else
										{
											resTable.AddCol(Tx::Core::dtype_double);					
											TitleToCol.insert(std::make_pair(tempEndDate,TitleCount));
										}
									}									
									/*strDate.Format(_T("%d-%d-%d"),tempEndDate/10000,tempEndDate/100%100,tempEndDate%100);
									vDates.push_back(strDate);*/	
									////取数据
									//if(iStatIndicator&1 && iStatIndicator&2)
									//{
									//	tempTable2.GetCell(2,k,dData1);
									//	dTotalData1 += dData1;
									//	//tempTable2.GetCell(3,k,dData2);
									//	dTotalData2 = Tx::Core::Con_doubleInvalid;
									//}
									//else if(iStatIndicator == 2)
									//{
									//	dTotalData1 = Tx::Core::Con_doubleInvalid;
									//}
									//else
									//{
									//	tempTable2.GetCell(2,k,dData1);
									//	dTotalData1 += dData1;
									//}
									if (irow > 0)
									{
										for (int i = 0;i < irow;i++)
										{
											if(iStatIndicator&1 && iStatIndicator&2)
											{										
												tempCol = 2*TitleToCol[tempEndDate] + 1;
												resTable.SetCell(tempCol,i,Tx::Core::Con_doubleInvalid);
												resTable.SetCell(tempCol+1,i,Tx::Core::Con_doubleInvalid);
											}
											else
											{
												tempCol = TitleToCol[tempEndDate] + 2;
												resTable.SetCell(tempCol,i,Tx::Core::Con_doubleInvalid);
											}
										}
									}						
								}								
							}
							//取数据
							if(iStatIndicator&1 && iStatIndicator&2)
							{
								tempTable2.GetCell(2,k,dData1);
								dTotalData1 += dData1;
								dTotalData2 = Tx::Core::Con_doubleInvalid;
							}
							else if(iStatIndicator == 2)
							{
								dTotalData1 = Tx::Core::Con_doubleInvalid;
							}
							else
							{
								tempTable2.GetCell(2,k,dData1);
								dTotalData1 += dData1;
							}			
						}
						//tempTable2.GetCell(1,tempTable2.GetRowCount()-1,tempEndDate);
						tempEndDate = dateE;	// zway for mantis #1778
						//添加数据
						if(iStatIndicator&1 && iStatIndicator&2)
						{					
							tempCol = 2*TitleToCol[tempEndDate] + 1;
							resTable.SetCell(tempCol,irow,dTotalData1);
							resTable.SetCell(tempCol+1,irow,dTotalData2);
							DataColV.push_back(tempCol);
							DataColV.push_back(tempCol+1);
						}
						else
						{					
							tempCol = TitleToCol[tempEndDate] + 2;
							resTable.SetCell(tempCol,irow,dTotalData1);
							DataColV.push_back(tempCol);
						}
					}
					dateB = dateE;
					//dateB = Tx::Core::TxDate::CalculateDateOffsetDays(dateB,1);
					CalculatorEndDate(iTimeCycle,dateE,dateB);
				}
				if (iEndDate - dateB >0)
				{
					if(tempTable2.GetRowCount() != 0)
					{
						tempTable2.Clear();
						tempTable2.CopyColumnInfoFrom(tempTable1);
					}								
					//zway//直接用+1不对//tempTable1.Between(tempTable2,nColArray,iCol,1,dateB+1,iEndDate,true,true);
					tempTable1.Between(tempTable2,nColArray,iCol,1,Tx::Core::TxDate::CalculateDateOffsetDays(dateB,1),iEndDate,true,true);
					if (tempTable2.GetRowCount() != 0)
					{
						if (IsOut)
						{			
							//首先添加交易实体和名称，代码。
							tempTable2.GetCell(0,0,tradeId);
							resTable.SetCell(0,irow,tradeId);
							GetSecurityNow(tradeId);
							if(m_pSecurity != NULL)
							{
								strName = m_pSecurity->GetName();
								strCode = m_pSecurity->GetCode();
								resTable.SetCell(1,irow,strName);
								resTable.SetCell(2,irow,strCode);
							}
							int tempEndDate;
							//tempTable2.GetCell(1,tempTable2.GetRowCount()-1,tempEndDate);
							tempEndDate = dateE;	// zway for mantis #1778
							iterMap = TitleToCol.find(tempEndDate);
							if (iterMap == TitleToCol.end())
							{
								TitleCount++;
								if (!TitleToCol.empty())
								{
									iterMap2 = TitleToCol.lower_bound(tempEndDate);
									if(iStatIndicator&1 && iStatIndicator&2)
									{										
										if (iterMap2 == TitleToCol.end())
										{
											resTable.AddCol(Tx::Core::dtype_double);
											resTable.AddCol(Tx::Core::dtype_double);									
											TitleToCol.insert(std::make_pair(tempEndDate,TitleCount));
										}
										else
										{
											int iColData = iterMap2->second;
											iterMap3 = TitleToCol.insert(iterMap2,std::make_pair(tempEndDate,iColData));											
											resTable.InsertCol(2*iColData+1,Tx::Core::dtype_double);
											resTable.InsertCol(2*iColData+2,Tx::Core::dtype_double);
											for (iterMap3++;iterMap3 != TitleToCol.end();++iterMap3)
											{
												iColData++;
												iterMap3->second = iColData;
											}
										}
									}
									else
									{
										if (iterMap2 == TitleToCol.end())
										{
											resTable.AddCol(Tx::Core::dtype_double);					
											TitleToCol.insert(std::make_pair(tempEndDate,TitleCount));
										}
										else
										{
											int iColData = iterMap2->second;
											iterMap3 = TitleToCol.insert(iterMap2,std::make_pair(tempEndDate,iColData));											
											resTable.InsertCol(iColData+2,Tx::Core::dtype_double);
											for (iterMap3++;iterMap3 != TitleToCol.end();++iterMap3)
											{
												iColData++;
												iterMap3->second = iColData;
											}
										}
									}
								} 
								else
								{
									if(iStatIndicator&1 && iStatIndicator&2)
									{
										resTable.AddCol(Tx::Core::dtype_double);
										resTable.AddCol(Tx::Core::dtype_double);									
										TitleToCol.insert(std::make_pair(tempEndDate,TitleCount));
									}
									else
									{
										resTable.AddCol(Tx::Core::dtype_double);					
										TitleToCol.insert(std::make_pair(tempEndDate,TitleCount));
									}
								}																
								if (irow > 0)
								{
									for (int i = 0;i < irow;i++)
									{
										if(iStatIndicator&1 && iStatIndicator&2)
										{										
											tempCol = 2*TitleToCol[tempEndDate] + 1;
											resTable.SetCell(tempCol,i,Tx::Core::Con_doubleInvalid);
											resTable.SetCell(tempCol+1,i,Tx::Core::Con_doubleInvalid);
										}
										else
										{
											tempCol = TitleToCol[tempEndDate] + 2;
											resTable.SetCell(tempCol,i,Tx::Core::Con_doubleInvalid);
										}
									}
								}						
							}
#ifdef _DEBUG
							CString strTable4=resTable.TableToString();
							Tx::Core::Commonality::String().StringToClipboard(strTable4);
#endif
							//添加数据
							if(iStatIndicator&1 && iStatIndicator&2)
							{
								tempTable2.GetCell(2,tempTable2.GetRowCount()-1,dData1);
								tempTable2.GetCell(3,tempTable2.GetRowCount()-1,dData2);
								tempCol = 2*TitleToCol[tempEndDate] + 1;
								resTable.SetCell(tempCol,irow,dData1);
								resTable.SetCell(tempCol+1,irow,dData2);
								DataColV.push_back(tempCol);
								DataColV.push_back(tempCol+1);
							}
							//else
							//{
							//	tempTable2.GetCell(2,tempTable2.GetRowCount()-1,dData1);
							//	tempCol = TitleToCol[tempEndDate] + 2;
							//	resTable.SetCell(tempCol,irow,dData1);
							//	DataColV.push_back(tempCol);
							//}
							//////////////////////////////////////////////////////////////////
							//wangy fix bug 20090911 关于单独计算万份收益和7日年化收益 结果一致的修改
							else if(iStatIndicator&1)
							{
								tempTable2.GetCell(2,tempTable2.GetRowCount()-1,dData1);
								tempCol = TitleToCol[tempEndDate] + 2;
								resTable.SetCell(tempCol,irow,dData1);
								DataColV.push_back(tempCol);
							}
							else
							{
								tempTable2.GetCell(3,tempTable2.GetRowCount()-1,dData1);
								tempCol = TitleToCol[tempEndDate] + 2;
								resTable.SetCell(tempCol,irow,dData1);
								DataColV.push_back(tempCol);
							}
							///////////////////////////////////////////////////////////////////
#ifdef _DEBUG
							strTable4=resTable.TableToString();
							Tx::Core::Commonality::String().StringToClipboard(strTable4);
#endif
						} 
						else
						{
							double dTotalData1 = 0,dTotalData2 = 0;
							int tempEndDate;
							for(int k = 0;k < (int)tempTable2.GetRowCount();k++)
							{				
								if(k == 0)
								{
									//首先添加交易实体和名称，代码。
									tempTable2.GetCell(0,0,tradeId);
									resTable.SetCell(0,irow,tradeId);
									GetSecurityNow(tradeId);
									if(m_pSecurity != NULL)
									{
										strName = m_pSecurity->GetName();
										strCode = m_pSecurity->GetCode();
										resTable.SetCell(1,irow,strName);
										resTable.SetCell(2,irow,strCode);
									}						
									//tempTable2.GetCell(1,tempTable2.GetRowCount()-1,tempEndDate);
									tempEndDate = dateE;	// zway for mantis #1778
									iterMap = TitleToCol.find(tempEndDate);
									if (iterMap == TitleToCol.end())
									{
										TitleCount++;
										if (!TitleToCol.empty())
										{
											iterMap2 = TitleToCol.lower_bound(tempEndDate);
											if(iStatIndicator&1 && iStatIndicator&2)
											{												
												if (iterMap2 == TitleToCol.end())
												{
													resTable.AddCol(Tx::Core::dtype_double);
													resTable.AddCol(Tx::Core::dtype_double);									
													TitleToCol.insert(std::make_pair(tempEndDate,TitleCount));
												}
												else
												{
													int iColData = iterMap2->second;
													iterMap3 = TitleToCol.insert(iterMap2,std::make_pair(tempEndDate,iColData));													
													resTable.InsertCol(2*iColData+1,Tx::Core::dtype_double);
													resTable.InsertCol(2*iColData+2,Tx::Core::dtype_double);
													for (iterMap3++;iterMap3 != TitleToCol.end();++iterMap3)
													{
														iColData++;
														iterMap3->second = iColData;
													}
												}
											}
											else
											{
												if (iterMap2 == TitleToCol.end())
												{
													resTable.AddCol(Tx::Core::dtype_double);					
													TitleToCol.insert(std::make_pair(tempEndDate,TitleCount));
												}
												else
												{
													int iColData = iterMap2->second;
													iterMap3 = TitleToCol.insert(iterMap2,std::make_pair(tempEndDate,iColData));													
													resTable.InsertCol(iColData+2,Tx::Core::dtype_double);
													for (iterMap3++;iterMap3 != TitleToCol.end();++iterMap3)
													{
														iColData++;
														iterMap3->second = iColData;
													}
												}
											}
										} 
										else
										{
											if(iStatIndicator&1 && iStatIndicator&2)
											{
												resTable.AddCol(Tx::Core::dtype_double);
												resTable.AddCol(Tx::Core::dtype_double);									
												TitleToCol.insert(std::make_pair(tempEndDate,TitleCount));
											}
											else
											{
												resTable.AddCol(Tx::Core::dtype_double);					
												TitleToCol.insert(std::make_pair(tempEndDate,TitleCount));
											}
										}
										
										////取数据
										//if(iStatIndicator&1 && iStatIndicator&2)
										//{
										//	tempTable2.GetCell(2,k,dData1);
										//	dTotalData1 += dData1;
										//	dTotalData2 = Tx::Core::Con_doubleInvalid;
										//}
										//else if(iStatIndicator == 2)
										//{
										//	dTotalData1 = Tx::Core::Con_doubleInvalid;
										//}
										//else
										//{
										//	tempTable2.GetCell(2,k,dData1);
										//	dTotalData1 += dData1;
										//}
										if (irow > 0)
										{
											for (int i = 0;i < irow;i++)
											{
												if(iStatIndicator&1 && iStatIndicator&2)
												{										
													tempCol = 2*TitleToCol[tempEndDate] + 1;
													resTable.SetCell(tempCol,i,Tx::Core::Con_doubleInvalid);
													resTable.SetCell(tempCol+1,i,Tx::Core::Con_doubleInvalid);
												}
												else
												{
													tempCol = TitleToCol[tempEndDate] + 2;
													resTable.SetCell(tempCol,i,Tx::Core::Con_doubleInvalid);
												}
											}
										}						
									}									
								}
								//取数据
								if(iStatIndicator&1 && iStatIndicator&2)
								{
									tempTable2.GetCell(2,k,dData1);
									dTotalData1 += dData1;
									dTotalData2 = Tx::Core::Con_doubleInvalid;
								}
								else if(iStatIndicator == 2)
								{
									dTotalData1 = Tx::Core::Con_doubleInvalid;
								}
								else
								{
									tempTable2.GetCell(2,k,dData1);
									dTotalData1 += dData1;
								}			
							}
							//tempTable2.GetCell(1,tempTable2.GetRowCount()-1,tempEndDate);
							tempEndDate = dateE;	// zway for mantis #1778
							//添加数据
							if(iStatIndicator&1 && iStatIndicator&2)
							{					
								tempCol = 2*TitleToCol[tempEndDate] + 1;
								resTable.SetCell(tempCol,irow,dTotalData1);
								resTable.SetCell(tempCol+1,irow,dTotalData2);
								DataColV.push_back(tempCol);
								DataColV.push_back(tempCol+1);
							}
							else
							{					
								tempCol = TitleToCol[tempEndDate] + 2;
								resTable.SetCell(tempCol,irow,dTotalData1);
								DataColV.push_back(tempCol);
							}
						}
					}					
				}
				std::vector<int>::iterator iterTempV;
				for (int m = 1;m <= TitleCount;m++)
				{
					//添加数据
					if(iStatIndicator&1 && iStatIndicator&2)
					{					
						tempCol = 2*m + 1;
						iterTempV = find(DataColV.begin(),DataColV.end(),tempCol);
						if (iterTempV == DataColV.end())
						{
							resTable.SetCell(tempCol,irow,Tx::Core::Con_doubleInvalid);
							resTable.SetCell(tempCol+1,irow,Tx::Core::Con_doubleInvalid);
						}
						
					}
					else
					{					
						tempCol = m + 2;
						iterTempV = find(DataColV.begin(),DataColV.end(),tempCol);
						if (iterTempV == DataColV.end())
						{
							resTable.SetCell(tempCol,irow,Tx::Core::Con_doubleInvalid);
						}
					}
				}
#ifdef _DEBUG
				CString strTable3=resTable.TableToString();
				Tx::Core::Commonality::String().StringToClipboard(strTable3);
#endif
			}
			//std::unordered_map<int> CombinationColMap;
			std::set<int> CombinationTitleColSet;
			std::set<int>::iterator iterSet;
			int idate1,icol2,icol1;
			//int testCount = 1;
			for (iterMap = DateRangeMap.begin();iterMap != DateRangeMap.end();++iterMap)
			{
				if (!CombinationTitleColSet.empty())
				{
					CombinationTitleColSet.clear();
				}				
				for (iterMap2 = TitleToCol.begin();iterMap2 != TitleToCol.end();++iterMap2)
				{
					if (iterMap->first < iterMap2->first && iterMap2->first  <= iterMap->second)
					{
						CombinationTitleColSet.insert(iterMap2->first);
					}				
				}
				if ((int)CombinationTitleColSet.size() >= 2)
				{
					idate1 = *(CombinationTitleColSet.rbegin());
					tempCol = TitleToCol[idate1];
					for (int j = 0;j < (int)resTable.GetRowCount();j++)
					{
						for (iterSet = CombinationTitleColSet.begin();iterSet != CombinationTitleColSet.end();++iterSet)
						{	
							if (tempCol != TitleToCol[*iterSet])
							{
								icol2 = 2*TitleToCol[*iterSet] + 1;
								icol1 = 2*tempCol + 1;
								if(iStatIndicator&1 && iStatIndicator&2)
								{
									resTable.GetCell(icol2,j,dData1);
									resTable.GetCell(icol2+1,j,dData2);
									if (dData1 > 0)
									{
										resTable.SetCell(icol1,j,dData1);
									}
									if (dData2 > 0)
									{
										resTable.SetCell(icol1+1,j,dData2);
									}
								} 
								else
								{
									resTable.GetCell(icol2,j,dData1);
									if (dData1 > 0)
									{
										resTable.SetCell(icol1,j,dData1);
									}
								}									
							}
						}													
					}
					iterMap3 = TitleToCol.find(*(CombinationTitleColSet.begin()));
					tempCol = iterMap3->second;
					iterMap3++;
					for (;iterMap3 != TitleToCol.end();++iterMap3)
					{
						tempCol++;
						iterMap3->second = tempCol;
					}
					std::set<int>::reverse_iterator rIterSet;
					std::unordered_map<int,int>::iterator iterMap4;
					rIterSet = CombinationTitleColSet.rbegin();
					for (rIterSet++;rIterSet != CombinationTitleColSet.rend();++rIterSet)
					{
						idate1 = *rIterSet;
						iterMap4 = TitleToCol.find(idate1);
						if (iterMap4 != TitleToCol.end())
						{							
							icol2 = 2*iterMap4->second + 1;
							if(iStatIndicator&1 && iStatIndicator&2)
							{
								resTable.DeleteCol(icol2,2);
							} 
							else
							{
								resTable.DeleteCol(icol2);
							}
							TitleToCol.erase(iterMap4);
						}
					}
				}               
			}
			int tempDate;
			for (iterMap = TitleToCol.begin();iterMap != TitleToCol.end();++iterMap)
			{
				tempDate = iterMap->first;
				strDate.Format(_T("%d-%d-%d"),tempDate/10000,tempDate/100%100,tempDate%100);
				vDates.push_back(strDate);
			}
			delete [] nColArray;
			nColArray = NULL;
			return true;
		}
void TxFund::CalculatorEndDate(int iTimeCycle,int& dateE,int & dateB)
{
	int iTempDate;
	switch(iTimeCycle)
	{
	case 1:
		dateE = Tx::Core::TxDate::CalculateDateOffsetDays (dateB,7);
		break;
	case 2:
		{
			iTempDate = Tx::Core::TxDate::CalculateDateOffsetDays (dateB, 1);
			dateE = Tx::Core::TxDate::CalculateEndOfMonth (iTempDate);
		}			
		break;
	case 3:
		{
			iTempDate = Tx::Core::TxDate::CalculateDateOffsetDays (dateB, 1);
			dateE = Tx::Core::TxDate::CalculateEndOfQuarter (iTempDate);
		}
		break;
	case 4:
		{
			iTempDate = Tx::Core::TxDate::CalculateDateOffsetDays (dateB, 1);
			dateE = Tx::Core::TxDate::CalculateEndOfYear(iTempDate);
		}
		break;
	default:
		break;
	}
}
	//折溢价率
	bool	TxFund::StatOverflowValueRate(
		Tx::Core::Table_Indicator	&resTable,
		std::vector<int>	&iSecurityId,
		int		iStartDate,
		int		iEndDate,
		int		iTimeCycle,		//统计周期
		int		iCustomCycle,	//自定义周期
		std::vector<CString> &vColName
		)
	{
		ProgressWnd prw;
		UINT pid=prw.AddItem(1,_T("折溢价率统计"),0.0);
		prw.Show(2000);
		//判断是否是封闭式基金
		std::vector<int>::iterator iter;
		std::vector<int> CloseSecurity;
		for(iter = iSecurityId.begin();iter != iSecurityId.end();++iter)
		{
			GetSecurityNow(*iter);
			if(m_pSecurity != NULL)
			{
				//if(m_pSecurity->IsFund_Close() || m_pSecurity->IsFund_ETF() || m_pSecurity->IsFund_LOF())
				//2012-09-20    Bug:13047
				if(m_pSecurity->IsFund() && m_pSecurity->IsNormal())
					CloseSecurity.push_back(*iter);
			}			
		}
		Tx::Business::FundDeriveData FDD;	//净值增长率、折溢价率计算类
		std::vector<int> iDates;
		CTime temptime;
		temptime = CTime::GetCurrentTime();
		int tempDate = temptime.GetYear()*10000 + temptime.GetMonth()*100 + temptime.GetDay();
		if(tempDate == iEndDate)
		{
			//iEndDate = iEndDate - 1;
			CTimeSpan ts(1,0,0,0);
			CTime tend = temptime - ts;
			iEndDate = tend.GetYear()*10000 + tend.GetMonth()*100 + tend.GetDay();
		}
		GetCycleDates(iStartDate,iEndDate,iTimeCycle,iDates,iCustomCycle);
		if(iCustomCycle > 0 || iTimeCycle == 5)
			iDates.erase(iDates.begin());
		//添加进度条
		prw.SetPercent(pid,0.3);
		/*CTime temptime;
		temptime = CTime::GetCurrentTime();
		int tempDate = temptime.GetYear()*10000 + temptime.GetMonth()*100 + temptime.GetDay();
		if(tempDate == *(iDates.end()-1))
			iDates.erase(iDates.end()-1);*/
		//计算一组封闭式基金，某个时间序列的折溢价率
		bool result = FDD.CalcFundPremium(resTable,CloseSecurity,iDates);
		if(resTable.GetRowCount() == 0 ||result ==false)
		{
			prw.SetPercent(pid,1.0);
			return false;
		}
		//添加进度条
		prw.SetPercent(pid,0.6);
#ifdef _DEBUG
		CString strTable=resTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
		this->IdColToNameAndCode(resTable,0,0);
//		resTable.DeleteCol(0);

		vColName.push_back(_T("名称"));
		vColName.push_back(_T("代码"));
		CString tStr;
		for(int i=0;i<(int)iDates.size();i++)
		{
			tStr.Format(_T("%d-%02d-%02d"),iDates[i]/10000,iDates[i]/100%100,iDates[i]%100);
			vColName.push_back(tStr);
		}
		std::unordered_map<int,double> dtotalShare;
		std::unordered_map<int,double> dTempData;
		int icol = 2;
		double dRise = 0.0;
		int id;
		TxFundShareData* pTempFundShare = NULL;
		for(int i = 0;i < (int)resTable.GetRowCount();i++)
		{
			icol = 2;
			resTable.GetCell(0,i,id);
			GetSecurityNow(id);
			if(m_pSecurity != NULL)
			{
				for(iter = iDates.begin();iter != iDates.end();++iter)
				{
					icol++;
					pTempFundShare = NULL;
					pTempFundShare = m_pSecurity->GetTxFundShareDataByDate(*iter);
					//resTable.GetCell(icol,i,dRise);
					//if(pTempFundShare != NULL)
					//{
					//	if(pTempFundShare->TotalShare < 0)
					//		continue;							
					//	if(pTempFundShare->TotalShare > 0 && fabs(dRise - Tx::Core::Con_doubleInvalid) > 0.00001)
					//	{
					//		dTempData[*iter] += pTempFundShare->TotalShare*dRise;
					//		dtotalShare[*iter] += pTempFundShare->TotalShare;
					//	}
					//}
					if(pTempFundShare && pTempFundShare->TotalShare > 0.0)
					{
						//std::unordered_map<int,double>::iterator i2 = dtotalShare.find(*iter);
						//if (i2 != dtotalShare.end())
						//	i2->second += pTempFundShare->TotalShare;
						//else
						//	dtotalShare.insert(std::pair<int,double>(*iter,pTempFundShare->TotalShare)); 
						//修改：by wangzf  bug 10665   2012-2-15
						resTable.GetCell(icol,i,dRise);
						if(fabs(dRise - Tx::Core::Con_doubleInvalid) > 0.00001)
						{
							std::unordered_map<int,double>::iterator i2 = dtotalShare.find(*iter);
							if (i2 != dtotalShare.end())
								i2->second += pTempFundShare->TotalShare;
							else
								dtotalShare.insert(std::pair<int,double>(*iter,pTempFundShare->TotalShare)); 

							std::unordered_map<int,double>::iterator i1 = dTempData.find(*iter);
							if (i1 != dTempData.end())
								i1->second += pTempFundShare->TotalShare*dRise;
							else
								dTempData.insert(std::pair<int,double>(*iter,pTempFundShare->TotalShare*dRise));
						}
					}
				}
			}			
		}
		int icount = resTable.GetRowCount();
		resTable.AddRow();
		CString strTemp = _T("份额加权平均");
		resTable.SetCell(1,icount,strTemp);
		strTemp = Tx::Core::Con_strInvalid;
		resTable.SetCell(2,icount,strTemp);
		icol = 2;
		//double dData;
		//for(iter = iDates.begin();iter != iDates.end();++iter)
		//{
		//	icol++;
		//	if(dtotalShare.empty())
		//	{
		//		resTable.SetCell(icol,icount,Con_doubleInvalid);
		//		continue;
		//	}
		//	dData = dTempData[*iter]/dtotalShare[*iter];
		//	if(fabs(dData - Tx::Core::Con_doubleInvalid) < 0.00001)
		//		dData = Tx::Core::Con_doubleInvalid;			
		//	resTable.SetCell(icol,icount,dData);
		//}
		if(dtotalShare.empty() || dTempData.empty())
		{
			for(iter = iDates.begin();iter != iDates.end();++iter)
			{
				icol++;
				resTable.SetCell(icol,icount,Tx::Core::Con_doubleInvalid);
			}
		}
		else
		{
			for(iter = iDates.begin();iter != iDates.end();++iter)
			{
				icol++;
				double dData = Tx::Core::Con_doubleInvalid;
				std::unordered_map<int,double>::iterator i1 = dTempData.find(*iter);
				std::unordered_map<int,double>::iterator i2 = dtotalShare.find(*iter);
				if (i1 != dTempData.end() && i2 != dtotalShare.end())
				{
					dData = i1->second/i2->second;
				}
				resTable.SetCell(icol,icount,dData);
			}
		}
		//添加进度条
		prw.SetPercent(pid,1.0);
		return true;
	}
    
	//modified by lijw 2008-05-23
	//净值增长率
	bool	TxFund::StatFundNetValueRiseRate(
		Tx::Core::Table_Indicator	&resTable,
		std::vector<int>	&iSecurityId,
		int		iStartDate,
		int		iEndDate,
		int		iTimeCycle,		//统计周期
		int		iCustomCycle,	//自定义周期
		std::vector<CString> &vColName
		)
	{		
	/*	ProgressWnd prw;
		UINT pid=prw.AddItem(1,_T("净值增长率统计"),0.0);
		prw.Show(2000);*/

		resTable.Clear();
//		m_txTable.Clear();//delete by lijw 2008-03-18
		Tx::Business::FundDeriveData FDD;	//净值增长率、折溢价率计算类
		std::vector<int> iDates;
		std::set<std::pair<int,int>> date;		
		std::vector<int>::iterator CurrentDate,iterFirst,iterSecond;		
		iDates.clear();
		CTime temptime;
		temptime = CTime::GetCurrentTime();
		int tempDate = temptime.GetYear()*10000 + temptime.GetMonth()*100 + temptime.GetDay();
		if(tempDate == iEndDate)
			iEndDate = iEndDate - 1;
		GetCycleDates(iStartDate,iEndDate,iTimeCycle,iDates,iCustomCycle);	
		if(iDates.empty())
		{
			AfxMessageBox(_T("所选的周期跟指定日期不匹配"));
			return false;
		}
		int iCycleIndex;
		int iPreDate;
		CurrentDate=iDates.begin();
		if(iCustomCycle <= 0)
		{
			if(iTimeCycle != 5)
			{
				iCycleIndex=m_pShIndex->GetTradeDateIndex(*CurrentDate,0,iTimeCycle,true);
				iPreDate=m_pShIndex->GetTradeDateByIndex(iCycleIndex-1,iTimeCycle,false);	
				iDates.insert(iDates.begin(),iPreDate);
			}
		}
		
//		//说明是自定义的周期
//		if(iCustomCycle >0)
//		{
//	//		int tempDate = Tx::Core::TxDate::CalculateDateOffsetDays(iStartDate,-iCustomCycle);
//		/*	while ((iCycleIndex = m_pShIndex->GetTradeDateIndex(iStartDate,0,0,true)) == -1)
//			{
//				iStartDate = Tx::Core::TxDate::CalculateDateOffsetDays(iStartDate,1);
//			}
//			iPreDate=m_pShIndex->GetTradeDateByIndex(iCycleIndex-iCustomCycle,0,false);*/
//		}
//		else
//		{
//			if(iTimeCycle != 5)
//			{
//				iCycleIndex=m_pShIndex->GetTradeDateIndex(*CurrentDate,0,iTimeCycle,true);
//				iPreDate=m_pShIndex->GetTradeDateByIndex(iCycleIndex-1,iTimeCycle,false);				
//			}
//			else//周期为区间
//			{
////				int tempDate = Tx::Core::TxDate::CalculateDateOffsetDays(iStartDate,-iCustomCycle);
//				while ((iCycleIndex = m_pShIndex->GetTradeDateIndex(iStartDate,0,0,true)) == -1)
//				{
//					iStartDate = Tx::Core::TxDate::CalculateDateOffsetDays(iStartDate,-1);
//				}
//				iPreDate=m_pShIndex->GetTradeDateByIndex(iCycleIndex,0,false);
//			}
//		}
		/*CTime temptime;
		temptime = CTime::GetCurrentTime();
		int tempDate = temptime.GetYear()*10000 + temptime.GetMonth()*100 + temptime.GetDay();
		if(tempDate == *(iDates.end()-1))
			iDates.erase(iDates.end()-1);*/
		for(CurrentDate=iDates.begin();;)
		{
			iterFirst = CurrentDate;
			iterSecond = ++CurrentDate;	
			if(CurrentDate == iDates.end())
				break;
			Tx::Business::TxBusiness gDate;
			date.insert(pair<int,int>(*iterFirst,*iterSecond));			
		}
		std::vector<int>::iterator iterV;
		std::vector<int> tempVector;
		for(iterV = iSecurityId.begin();iterV != iSecurityId.end();++iterV)
		{
			GetSecurityNow(*iterV);			
			if(m_pSecurity != NULL)
			{
				//BUG:13241   2012-09-27
				Tx::Data::FundNewInfo *pFundNewInfo = m_pSecurity->GetFundNewInfo();
				if(pFundNewInfo == NULL || pFundNewInfo->style_id == 21)
					continue;
				if(!m_pSecurity->IsFund_Currency())
					tempVector.push_back(*iterV);
			}
		}
//#ifdef _DEBUG
//		for ( int i = 0; i< 1; i++)
//		{
//			bool Ret = FDD.CalcFundNvr (resTable, tempVector, date);
//		}
//#endif
		bool Ret = FDD.CalcFundNvr (resTable, tempVector, date);
		// 计算完成已经是%了,不要再乘100,会影响无效值的判断
		if(resTable.GetRowCount() == 0 || Ret == false)
		{
			return false;
		}
#ifdef _DEBUG
		CString strTable4=resTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable4);
#endif	
		this->IdColToNameAndCode(resTable,0,0);
		vColName.push_back(_T("名称"));
		vColName.push_back(_T("代码"));
		CString tStr;
		iDates.erase(iDates.begin());
		for(int i=0;i<(int)iDates.size();i++)
		{
			tStr.Format(_T("%d-%d-%d"),iDates[i]/10000,iDates[i]/100%100,iDates[i]%100);
			vColName.push_back(tStr);
		}
		////把净值增长率都乘100；
		//int icol = resTable.GetColCount();
		//int irow = resTable.GetRowCount();
		//double equity;
		//for(int j = 0;j < irow;j++)
		//{
		//	for(int i = 3;i < icol;i++)
		//	{
		//		resTable.GetCell(i,j,equity);
		//		equity = equity*100;
		//		resTable.SetCell(i,j,equity);
		//	}
		//}
#ifdef _DEBUG
		strTable4=resTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable4);
#endif
		//modified by zhangxs 20090807
		/*---------------------------
		金业绩加权权重设置有所修改：
		改为按照统计区间各次公告资产净值的平均值计算，
		如统计业绩的区间内无公告资产净值，
		按照最近一次公告的资产规模计算。
		-----------------------------------*/
		std::unordered_map<int,double> dtotalShare;
		std::unordered_map<int,double> dTempData;
		dtotalShare.clear();
		dTempData.clear();
		int icol = 2;
		double dRise = 0.0;
		int id;
		TxFundShareData* pTempFundShare = NULL;
		FundNetValueData* pTempFundNet = NULL;

		std::vector<int> vecTemp;
		vecTemp.clear();
		GetReportDateArr(vecTemp,iStartDate,iEndDate);
	
		for(int i = 0;i < (int)resTable.GetRowCount();i++)
		{
			icol = 2;
			resTable.GetCell(0,i,id);
			GetSecurityNow(id);
			if(m_pSecurity != NULL)
			{
				double dTotleNet = 0.0;
				dTotleNet = GetAvgNetValueReportDateArr(vecTemp,id);
				if(iTimeCycle == 5)
				{
					int iDateTemp = iDates[0];
					double dTempRaise = 0.0;
					int iSum = 0;
					icol = 3 ;
					resTable.GetCell(icol,i,dRise);
					
					if(fabs(dRise - Tx::Core::Con_doubleInvalid) > 0.00001)
					{
						dTempData[iDates[0]] += dTotleNet * dRise;
						dtotalShare[iDates[0]] += dTotleNet;
					}
				}
				else
				{
					for(iterV = iDates.begin();iterV != iDates.end();++iterV)
					{
						icol++;
						//BUG:13478   /  wangzf  /  2012-11-12
						resTable.GetCell(icol,i,dRise);
						if(fabs(dRise - Tx::Core::Con_doubleInvalid) > 0.00001)
						{
							dTempData[*iterV] += dTotleNet * dRise;
							dtotalShare[*iterV] += dTotleNet;
							TRACE3("%d -- %f -- %f\n",id,dTotleNet,dRise);
						}
						//pTempFundShare = NULL;
						//pTempFundNet = NULL;
						//pTempFundShare = m_pSecurity->GetTxFundShareDataByDate(*iterV);
						//if(pTempFundShare != NULL)
						//{
						//	if(pTempFundShare->TotalShare < 0 )
						//		continue;
						//	int m_iDateReport = pTempFundShare->iDate;
						//	pTempFundNet = m_pSecurity->GetFundNetValueDataByDate(m_iDateReport);
						//	resTable.GetCell(icol,i,dRise);
						//	if( pTempFundNet != NULL)
						//	{						
						//		if( pTempFundNet->fNetvalue < 0)
						//			continue;						
						//		if(pTempFundShare->TotalShare > 0 && pTempFundNet->fNetvalue > 0
						//			&& fabs(dRise - Tx::Core::Con_doubleInvalid) > 0.00001)
						//		{
						//			dTempData[*iterV] += (pTempFundShare->TotalShare)*(pTempFundNet->fNetvalue) * dRise;
						//			dtotalShare[*iterV] += (pTempFundShare->TotalShare)*(pTempFundNet->fNetvalue);
						//			TRACE3("%d -- %f -- %f\n",id,pTempFundShare->TotalShare,dRise);
						//		}
						//	}
						//}
					}
				}				
			}			
		}
		int icount = resTable.GetRowCount();
		resTable.AddRow();
		CString strTemp = _T("净值加权平均");
		resTable.SetCell(1,icount,strTemp);
		strTemp = Tx::Core::Con_strInvalid;
		resTable.SetCell(2,icount,strTemp);
		icol = 2;
		double dData = Con_doubleInvalid;
		for(iterV = iDates.begin();iterV != iDates.end();++iterV)
		{
			icol++;
			if(dtotalShare.empty())
			{
				resTable.SetCell(icol,icount,Con_doubleInvalid);
				continue;
			}
			if(dtotalShare[*iterV] > 0.01)
				dData = dTempData[*iterV]/dtotalShare[*iterV];
			if(fabs(dData - Tx::Core::Con_doubleInvalid) < 0.00001)
				dData = Tx::Core::Con_doubleInvalid;			
			resTable.SetCell(icol,icount,dData);
		}
		return true;
	}
//净值分析 add by lijw 2008-03-3-20
//分五部分  基金基本信息+最新净值信息+收盘价+折溢价率+最后历史行情
bool	TxFund::StatFundNetValueAnalyse(
		Tx::Core::Table_Indicator	&resTable,
		std::vector<int>	&iSecurityId,
		int		iEndDate,
		std::vector<CString> ExplainVec,
		std::vector<int> StartVec,
		std::vector<int> EndVec,
		std::unordered_map<int,double>	mapMarketSetting,
		bool	bCalculateValueRate,
		int		iType,
		std::vector<CString> &vColName,
		std::vector<CString> &vColHeaderName
		)
{
	ProgressWnd prw;
	UINT pid=prw.AddItem(1,_T("净值分析统计"),0.0);
	prw.Show(2000);

	if(iSecurityId.empty())
	{
		//添加进度条
		prw.SetPercent(pid,1.0);
		return false;
	}
	/*
	注：单只基金的单位净值、累计净值、收盘价、折溢价率、基准收益率、平均折溢价率等的数据提供方式改为Web服务
	Mantis:11523  2012-11-12  /  wangzf   / 
	*/
	std::vector<std::pair<int,int>> reqDates;//时间段集合
	std::vector<int> reqSecurityIds; 
	reqDates.clear();
	reqSecurityIds.clear();
	//添加表头文字--------------------------------------------------------------------------------
	vColName.clear();
	vColName.push_back(_T("名称"));
	vColName.push_back(_T("代码"));
	vColName.push_back(_T("基金风格"));
	vColName.push_back(_T("上市日期"));
	vColName.push_back(_T("份额[亿]"));
	vColName.push_back(_T("单位净值"));
	vColName.push_back(_T("累计净值"));
	vColName.push_back(_T("收盘价"));
	vColName.push_back(_T("折溢价率"));

    CString strdate1,strdate2,strhead1,strhead2;
	strdate1.Format(_T("%d-%d-%d"),iEndDate/10000,iEndDate%10000/100,iEndDate%10000%100);//这是所选的周期里最后一个周期的截止日期。
    strhead1 =_T("基本情况") + (CString)_T("[") + strdate1 + (CString)_T("]");
	vColHeaderName.push_back(strhead1);
	int tempdate1,tempdate2;
	std::vector<CString>::iterator iterStr;
	std::vector<int>::iterator iterStart,iterEnd;
	iterStart = StartVec.begin();
	iterEnd = EndVec.begin();
	for (iterStr = ExplainVec.begin();iterStr != ExplainVec.end();++iterStr,++iterStart,++iterEnd)
	{		
		tempdate1 = *iterStart;
		tempdate2 = *iterEnd;
		if(tempdate1 > 0)
		{
			strdate1.Format(_T("%d-%d-%d"),tempdate1/10000,tempdate1%10000/100,tempdate1%100);
		}
		else
		{
			strdate1 = iterStr->Left(9);
		}
		strdate2.Format(_T("%d-%d-%d"),tempdate2/10000,tempdate2%10000/100,tempdate2%100);
		strhead2 = *iterStr+ _T("[") +strdate1 + _T(" － ") + strdate2+ _T("]");
		vColHeaderName.push_back(strhead2);
		reqDates.push_back(std::make_pair(tempdate1,tempdate2)); //时间段集合
	}
	//添加表头文字--------------------------------------------------------------------------------

	//下面是从基金基本表取基金的基本信息的数据。并且把这个表作为结果表，
	//默认的返回值状态。
	bool result = true;
//	Tx::Core::Table_Indicator	m_txTable,TempTable;
	//准备样本集=第一参数列:交易实体ID,int型
	resTable.AddParameterColumn(Tx::Core::dtype_int4);
	//开始日期
	resTable.AddParameterColumn(Tx::Core::dtype_int4);
	//插入这两列为了插入代码和名称。
	resTable.InsertCol(1,Tx::Core::dtype_val_string);//插入基金名称
	resTable.InsertCol(2,Tx::Core::dtype_val_string);//插入代码
	resTable.InsertCol(3,Tx::Core::dtype_int4);//3,基金风格ID
	resTable.AddCol(Tx::Core::dtype_decimal); //基金份额
	resTable.AddCol(Tx::Core::dtype_decimal);//6,插入单位净值
	resTable.AddCol(Tx::Core::dtype_decimal);//7,插入累计净值
	resTable.AddCol(Tx::Core::dtype_double);//8,插入收盘价
	resTable.AddCol(Tx::Core::dtype_double);//9,插入折溢价率
	resTable.AddCol(Tx::Core::dtype_int4);//10,基金公司ID
	resTable.AddCol(Tx::Core::dtype_int4);//11,银行ID	
	
	std::vector<UINT> vecInstiID2;
	std::vector<UINT>::iterator iteID;
	int position;
	CString strName,strCode;	
	double ZYJL,fundShare;
	Tx::Business::FundDeriveData FDD;	//净值增长率、折溢价率计算类
	int TradeID;
	FundNewInfo *pFundNewInfo;
	int iInstitutionId;
	int m_iRow = 0;
	int ealiestSetupDate = 21001231;//最早上市日期

	auto dbset = _M_dbContext.get_total_share_db_set(iEndDate,true);
	ASSERT(dbset != nullptr);

	std::vector<int>::iterator iter1,iter2;
	for(iter1 = iSecurityId.begin();iter1!=iSecurityId.end();++iter1)
	{
		//取得交易实体ID
		TradeID = *iter1;
		GetSecurityNow(*iter1);
		if(m_pSecurity==NULL || m_pSecurity->IsFund_Currency())  //去掉货币基金
			continue;
		
		//基金份额
		fundShare = dbset->get_value(TradeID);

		ZYJL = Tx::Core::Con_doubleInvalid;
		// 判断不是封闭就行了....
		//IsOpenFund = !m_pSecurity->IsFund_Close();

		//根据交易实体ID取得样本的名称和外码；
		strName = m_pSecurity->GetName();
		strCode = m_pSecurity->GetCode();

		//基金基本信息--设立日期、上市日期、基金风格ID、银行ID
		pFundNewInfo = m_pSecurity->GetFundNewInfo();
		if (pFundNewInfo == NULL)
			continue;

		resTable.AddRow();
		resTable.SetCell(0,m_iRow,*iter1);
		//如果是开放式基金，就把设立日期放到上市日期的那一列上
		if(!m_pSecurity->IsFund_Close())
			resTable.SetCell(4,m_iRow,pFundNewInfo->setup_date);	
		else
			resTable.SetCell(4,m_iRow,pFundNewInfo->ipo_date);

 		if(pFundNewInfo->setup_date >0 && pFundNewInfo->setup_date < ealiestSetupDate)
			ealiestSetupDate = pFundNewInfo->setup_date;

		//添加收盘价
		//if(m_pSecurity->IsFund_Close() || m_pSecurity->IsFund_ETF() || m_pSecurity->IsFund_LOF())
		//	dOpenPrice = m_pSecurity->GetClosePrice(iEndDate,true);
		//else //部分特殊的开放基金或QDII基金有行情数据
		//	dOpenPrice = m_pSecurity->GetOpenFundClosePrice(iEndDate,false);
		//if(dOpenPrice < 0)
		//	dOpenPrice = Tx::Core::Con_doubleInvalid;
		//resTable.SetCell(8,m_iRow,dOpenPrice);

		resTable.SetCell(1,m_iRow,strName);
		resTable.SetCell(2,m_iRow,strCode);	
		//把基金的份额的单位换成亿
		if(fundShare < 0)
			fundShare = Tx::Core::Con_doubleInvalid;
		else
			fundShare = fundShare/100000000;
		resTable.SetCell(5,m_iRow,fundShare);

		//添加基金风格ID，基金公司ID，银行ID 
		resTable.SetCell(3,m_iRow,pFundNewInfo->style_id);
		iInstitutionId = m_pSecurity->GetInstitutionId();
		resTable.SetCell(10,m_iRow,iInstitutionId);
		resTable.SetCell(11,m_iRow,pFundNewInfo->trusteeship_bank);

		//请求样本集
		reqSecurityIds.push_back(TradeID);
		m_iRow++;
	}

#ifdef _DEBUG
	CString strTable1=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable1);
#endif

//--------------------------------------------------------------------------------
	//添加进度条
	//prw.SetPercent(pid,0.4);
    prw.SetPercent(pid,0.2);
	////下面的应该对相应的周期进行循环．并且在调试之前一定要把日期填写好   
	//这个地方要注意,自上市日期以来,还没有做,如果是求自上市日期以来,一定要先找它们的上市日期.
//自定义-------------------------------------------------------------------------------------
	Tx::Core::Table_Indicator TempDateTable;
	std::set< std::pair<int,int> > tempDate;
	int icount,iTradeId;
	/*std::set< std::pair<int,int> >::reverse_iterator iterPair;*/
	int tempCol = 12;	
	int iDate1 = 0,iDate2 = 0;
	int iDate3,iDate4;
	CString msg,msg1;
	std::vector<int> iDates;
	iterEnd = EndVec.begin();
	for(iterStart = StartVec.begin();iterStart != StartVec.end();++iterStart,++iterEnd)
	{
		msg.Format(_T("StatFundNetValueAnalyse::StartDate:%d"),*iterStart);
		//CLogRecorder::GetInstance()->WriteToLog(msg);

		if(bCalculateValueRate)
		{
			resTable.AddCol(Tx::Core::dtype_double);//tempCol,插入净值增长率
			resTable.AddCol(Tx::Core::dtype_int4);//tempCol+1,插入增长率排名
			resTable.AddCol(Tx::Core::dtype_double);//tempCol+2,插入同期市场比较
			resTable.AddCol(Tx::Core::dtype_double);//tempCol+3,插入业绩基准收益率
			resTable.AddCol(Tx::Core::dtype_double);//tempCol+4,插入同期业绩基准比较
			resTable.AddCol(Tx::Core::dtype_double);//tempCol+5,插入平均折溢价率
			vColName.push_back(_T("增长率"));
			vColName.push_back(_T("排名"));
			vColName.push_back(_T("同期市场比较"));
			vColName.push_back(_T("基准增长率"));
			vColName.push_back(_T("超越基准"));
			vColName.push_back(_T("平均折溢价率"));
		}
		else
		{
			resTable.AddCol(Tx::Core::dtype_double);//tempCol,插入净值增长率
			resTable.AddCol(Tx::Core::dtype_int4);//tempCol+1,插入增长率排名
			resTable.AddCol(Tx::Core::dtype_double);//tempCol+2,插入同期市场比较
			resTable.AddCol(Tx::Core::dtype_double);//tempCol+3,插入业绩基准收益率
			resTable.AddCol(Tx::Core::dtype_double);//tempCol+4,插入同期业绩基准比较
			vColName.push_back(_T("增长率"));
			vColName.push_back(_T("排名"));
			vColName.push_back(_T("同期市场比较"));
			vColName.push_back(_T("基准增长率"));
			vColName.push_back(_T("超越基准"));
		}
		iDate1 = *iterStart;
		iDate2 = *iterEnd;
		iDate3 = iDate1;
		iDate4 = iDate2;	

//-------------------------------------------------------------------------------------------
		//计算净值增长率
		double JZZL,dRate1=0,dRate2=0;
		int iCycleIndex;
		if (iDate1 !=0 )
		{
			// 不是上市以来
			//2012-04-12,刘鹏。区间的计算，不改变开始日期。bug 11578 -- 有的基金在上交所的非交易日也披露基金净值
			vector<int> tmpDate;
			GetCycleDates(iDate1,iDate1,5,tmpDate,0);
			iDate1 = tmpDate[0];
		}

		while((iCycleIndex = m_pShIndex->GetTradeDateIndex(iDate2,0,0,true)) == -1)
			iDate2 = Tx::Core::TxDate::CalculateDateOffsetDays(iDate2,-1);
		iDate2=m_pShIndex->GetTradeDateByIndex(iCycleIndex,0,false);

		//为了把当天的净值增长率值去掉，所以有以下判断
		CTime tempTime = CTime::GetCurrentTime();
		int idate = tempTime.GetYear()*10000 + tempTime.GetMonth()*100 + tempTime.GetDay();
		if(idate == iDate2)
		{
			iCycleIndex = m_pShIndex->GetTradeDateIndex(iDate2,0,0,true);//因为上面已经判断，所以此处不用判断
			iDate2=m_pShIndex->GetTradeDateByIndex(iCycleIndex-1,0,false);
		}

		icount = resTable.GetRowCount();
		if (iDate2 == iDate1)
		{
			JZZL = Tx::Core::Con_doubleInvalid;
			dRate2 = Tx::Core::Con_doubleInvalid;
			for (int j = 0;j < icount;j++)
			{
				resTable.SetCell(tempCol,j,JZZL);//是写成无效值，还是0，根据业务我认为应该是0；但是暂时写成无效值
				resTable.SetCell(tempCol+2,j,dRate2);
			}
			if (bCalculateValueRate)
			{
				tempCol += 6;
			} 
			else
			{
				tempCol +=5;
			}
			continue;
		}
		if(!tempDate.empty())
			tempDate.clear();
		tempDate.insert(std::make_pair(iDate1,iDate2));	
		m_txTable.Clear();

		FDD.CalcFundNvr (m_txTable, iSecurityId, tempDate);
#ifdef _DEBUG
		//mantis:15391 记录日志  2013-05-28

		for (std::vector<int>::iterator iter4 = iSecurityId.begin();iter4 != iSecurityId.end();iter4++)
		{
			msg.Format(_T("%d"),*iter4);
			CLogRecorder::GetInstance()->WriteToLog(msg);
		}
		for (std::set< std::pair<int,int> >::iterator iter3 = tempDate.begin();iter3 != tempDate.end();iter3++)
		{
			msg.Format(_T("%d,%d"),iter3->first,iter3->second);
			CLogRecorder::GetInstance()->WriteToLog(msg);
		}
		msg.Format(_T("StatFundNetValueAnalyse::CalcFundNvr-End"));
		CLogRecorder::GetInstance()->WriteToLog(msg);


		strTable1=m_txTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable1);
#endif		
		std::vector<double>::iterator dIter;
		std::unordered_map<int,double>::iterator iterMap;
		double dclosePrice1,dclosePrice2;		
		for(int i = 0;i < icount;i++)
		{
			resTable.GetCell(0,i,iTradeId);					
			//把m_txTable和resTable表中交易实体ID相等纪录对应起来。并且把净值增长率放到resTable。
			if(!(vecInstiID2.empty()))
				vecInstiID2.clear();
			m_txTable.Find(0,iTradeId,vecInstiID2);
			//如果没有该交易实体的净值增长率，就说明该日期范围内没有该值。
			if(vecInstiID2.empty())
			{
				JZZL = Tx::Core::Con_doubleInvalid;;//是写成无效值，还是0，根据业务我认为应该是0；但是暂时写成无效值
				dRate2 = Tx::Core::Con_doubleInvalid;;
			}
			else
			{
				//因为m_txTable表里的数据是以基金的交易实体为主键的，所以可以用下面的方法。
				iteID = vecInstiID2.begin();
				m_txTable.GetCell(1,*iteID,JZZL);
				//把基金净值的值乘以100
				//JZZL = JZZL*100;
				//把同期市场比较的值也添加到resTable里。
				dRate2 = 0;
				int iMarketDate;
				resTable.GetCell(4,i,iMarketDate);
				for(iterMap = mapMarketSetting.begin();iterMap != mapMarketSetting.end();++iterMap)
				{
					GetSecurityNow(iterMap->first);
					if(m_pSecurity == NULL)
					{
						dRate2 = Tx::Core::Con_doubleInvalid;
						break;
					}
					if(iMarketDate >iDate1)
					{
						//dclosePrice1 = m_pSecurity->GetPreClosePrice(iMarketDate);
						dclosePrice1 = m_pSecurity->GetClosePrice(iMarketDate);
					}
					else
					{
						//dclosePrice1 = m_pSecurity->GetPreClosePrice(iDate1);
						dclosePrice1 = m_pSecurity->GetClosePrice(iDate1);
					}
					//dclosePrice2 = m_pSecurity->GetClosePrice(iDate2);
					dclosePrice2 = m_pSecurity->GetClosePrice(iDate2);
					dRate1 = (dclosePrice2/dclosePrice1 - 1)*iterMap->second;
					dRate2 += dRate1;
				}
				if(fabs(JZZL) < 10000)
				{
					dRate2 = JZZL - dRate2;
					if(fabs(dRate2) > 10000)
						dRate2 = Tx::Core::Con_doubleInvalid;
				}
				else
				{
					JZZL = Tx::Core::Con_doubleInvalid;
					dRate2 = Tx::Core::Con_doubleInvalid;
				}

			}
			resTable.SetCell(tempCol,i,JZZL);
			resTable.SetCell(tempCol+2,i,dRate2);
		}
		if (bCalculateValueRate)
		{
			tempCol += 6;
		} 
		else
		{
			tempCol +=5;
		}
	}
	if(bCalculateValueRate)
		vColName.push_back(_T("6"));
	else
		vColName.push_back(_T("5"));
    //进度条
	prw.SetPercent(pid,0.4);
//自定义-------------------------------------------------------------------------------------

	Tx::Core::Table_Indicator tmpTable;
	int resTableRow,resTableCol;  //结果表行数、列数
	int tmpTableRow,tmpTableCol;  //临时表行数、列数

	FDD.CalFundData(tmpTable,reqSecurityIds,reqDates,iEndDate,bCalculateValueRate);

	if (tmpTable.GetColCount() == 0)
	{
		prw.SetPercent(pid,1.0);
		return false;
	}
	//判断返回数据中自定义时间段个数与参数中的是否一致
	int nTmp = 0;
	if(bCalculateValueRate)
		nTmp = (tmpTable.GetColCount()-5)/2;
	else
		nTmp = tmpTable.GetColCount()-5;
	if (nTmp != (int)StartVec.size())
	{
		//进度条
		prw.SetPercent(pid,1.0);
		return false;
	}
	//1.填充基本情况列数据--单位净值、累计净值、收盘价、折溢价率
	tmpTableRow = tmpTable.GetRowCount();
	double dValue2;
	for (int i=0;i<tmpTableRow;i++)
	{
		tmpTable.GetCell(0,i,TradeID);
		if(!(vecInstiID2.empty()))
			vecInstiID2.clear();
		resTable.Find(0,TradeID,vecInstiID2);
		if((int)vecInstiID2.size() == 0)
			continue;
		for (int icol=1;icol<5;icol++)
		{
			tmpTable.GetCell(icol,i,dValue2);
			for(iteID = vecInstiID2.begin();iteID != vecInstiID2.end();++iteID)
			{	
				position = *iteID;
				if(icol == 1) //单位净值
					resTable.SetCell(6,position,dValue2);
				else if(icol == 2)//累计净值
					resTable.SetCell(7,position,dValue2);
				else if(icol == 3)//收盘价
					resTable.SetCell(8,position,dValue2);
				else//折溢价率
					resTable.SetCell(9,position,dValue2);
			}
		}
	}

	//2.填充业绩基准收益率、平均折溢价率
	tmpTableCol = tmpTable.GetColCount();
	int tempCol1;
	if (bCalculateValueRate)
	{
		for (int row=0;row<tmpTableRow;row++)
		{
			tempCol1 = 15;//业绩基准收益率
			tmpTable.GetCell(0,row,TradeID);
			if(!(vecInstiID2.empty()))
				vecInstiID2.clear();
			resTable.Find(0,TradeID,vecInstiID2);
			if((int)vecInstiID2.size() == 0)
				continue;
			iteID = vecInstiID2.begin();
			position = *iteID;
			for (int icol=5;icol<tmpTableCol;)
			{
				tmpTable.GetCell(icol,row,dValue2);
			    resTable.SetCell(tempCol1,position,dValue2);
				icol += 2;
				tempCol1 += 6;
			}
			tempCol1 = 17;
			for (int icol=6;icol<tmpTableCol;)
			{
				tmpTable.GetCell(icol,row,dValue2);
				resTable.SetCell(tempCol1,position,dValue2);
				icol += 2;
				tempCol1 += 6;
			}
		}
	}
	else
	{
		for (int row=0;row<tmpTableRow;row++)
		{
			tempCol1 = 15;//业绩基准收益率
			tmpTable.GetCell(0,row,TradeID);
			if(!(vecInstiID2.empty()))
				vecInstiID2.clear();
			resTable.Find(0,TradeID,vecInstiID2);
			if((int)vecInstiID2.size() == 0)
				continue;
			iteID = vecInstiID2.begin();
			position = *iteID;
			for (int icol=5;icol<tmpTableCol;icol++)
			{
				tmpTable.GetCell(icol,row,dValue2);
				resTable.SetCell(tempCol1,position,dValue2);
				tempCol1 += 5;
			}
		}
	}

	//进度条
	prw.SetPercent(pid,0.6);
	//3.填充同期业绩基准比较
	//同期业绩基准比较 = 净值增长率-业绩基准收益率
	resTableRow = resTable.GetRowCount();
	resTableCol = resTable.GetColCount();
	double tmpJZZL = 0.0;
	for (int row=0;row<resTableRow;row++)
	{
		//2012-12-28  wangzf  bug:14203
		for (int col=12;col<resTableCol;)
		{
			resTable.GetCell(col,row,tmpJZZL);
			resTable.GetCell(col+3,row,dValue2);
			if(tmpJZZL == Tx::Core::Con_doubleInvalid || dValue2 == Tx::Core::Con_doubleInvalid)
				resTable.SetCell(col+4,row,Tx::Core::Con_doubleInvalid);
			else
			{
				dValue2 = tmpJZZL - dValue2;
				if (fabs(dValue2-Tx::Core::Con_doubleInvalid)<0.00001)
				{
					resTable.SetCell(col+4,row,Tx::Core::Con_doubleInvalid);
				}
				else
				    resTable.SetCell(col+4,row,dValue2);
			}
			if (bCalculateValueRate)
				col += 6;
			else
				col += 5;
		}
	}

	//填充净值增长率排名
	std::vector<int>::size_type size = StartVec.size();
	int iNo;	
	icount = resTable.GetColCount();
	int sortCol;
	if (bCalculateValueRate)
	{
		tempCol = icount - 5;//存放净值增长率的排名的第一列
		sortCol = icount - 6;//需要排序的那一列
	} 
	else
	{
		tempCol = icount - 4;//存放净值增长率的排名的第一列
		sortCol = icount - 5;//需要排序的那一列
	}
	double JZZLData1,JZZLData2;
	for(int k = 0;k < (int)size;k++)
	{		
		resTable.Sort(sortCol,false);
		resTable.Arrange();
#ifdef _DEBUG
		strTable1=resTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable1);
#endif
		iNo = 0;
		JZZLData1 = Tx::Core::Con_doubleInvalid;
		JZZLData2 = Tx::Core::Con_doubleInvalid;
		bool IsEqual = false;
		int EffectDataRow = 0;
		for (int m = 0;m < (int)resTable.GetRowCount();m++)
		{
			resTable.GetCell(sortCol,m,JZZLData1);
			if(fabs(JZZLData1 - Tx::Core::Con_doubleInvalid) > 0.00001)
				EffectDataRow++;
			else
			{
				resTable.SetCell(tempCol,m,Tx::Core::Con_intInvalid);
				continue;
			}
			if(JZZLData2 == JZZLData1)
			{				
				IsEqual = true;
			}
			else
			{
				if(IsEqual)
					iNo = EffectDataRow - 1;
				IsEqual = false;
			}
			if(!IsEqual)
				iNo++;
			resTable.SetCell(tempCol,m,iNo);
            JZZLData2 = JZZLData1;			
		}	
		if(bCalculateValueRate)
		{
			tempCol -= 6;
			sortCol -= 6;
		}
		else
		{
			tempCol -= 5;
			sortCol -= 5;
		}
	}
	std::unordered_map<int,CString> fundStyle;
	std::unordered_map<int,CString>::iterator iterMap;
	//modified by zhangxs 20091221---NewStyle
	//TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX,fundStyle);
	TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX_NEW,fundStyle);
	resTable.InsertCol(4,Tx::Core::dtype_val_string);//插入基金风格的str
	icount = resTable.GetRowCount();//没有分组统计之前resTable里的记录数
	int tempStyle;
    for(int j = 0;j < icount;j++)
	{
		resTable.GetCell(3,j,tempStyle);
		iterMap = fundStyle.find(tempStyle);
		if(iterMap != fundStyle.end())
			resTable.SetCell(4,j,iterMap->second);
	}

	//添加进度条
	prw.SetPercent(pid,0.8);
	//权重由份额改为期间资产净值平均值 ---zhangxs 20090813---
	//一次性取得所有券所有需要日期的单位净值 -- 2012-2-14，刘鹏
	int tmpEaliestStartDate,tmpLatestEndDate;
	//先算最小的起始日期
	tmpEaliestStartDate = *min_element(StartVec.begin(),StartVec.end());
	if(tmpEaliestStartDate <= 0)
		tmpEaliestStartDate = ealiestSetupDate;
	//再算最大的终止日期
	tmpLatestEndDate = *max_element(EndVec.begin(),EndVec.end());
	//调用接口进行数据处理
	Tx::Core::Table_Indicator netValueTable;
	vector<int> maxDates;//最大的日期序列
	GetReportDateArr(maxDates,tmpEaliestStartDate,tmpLatestEndDate);
	for(vector<int>::iterator iter=StartVec.begin();iter!=StartVec.end();iter++)
	{
		if(*iter<=0)
			continue;
		maxDates.push_back(*iter);
	}
	sort(maxDates.begin(),maxDates.end());
    if (!StatFundNetValueOutput(netValueTable,iSecurityId,maxDates))
    {
		netValueTable.Clear();
		return false;
    }
#ifdef _DEBUG
		int nCountTest = (int)netValueTable.GetRowCount();
		CString strt=netValueTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strt);
#endif
	std::unordered_map<int,std::vector<int>> mapDate;
	mapDate.clear();
	int m_iSizeOfDate = (int)StartVec.size();
	if( m_iSizeOfDate > 0 && m_iSizeOfDate == (int)EndVec.size())
	{
		for(int ix = 0; ix < m_iSizeOfDate; ix++)
		{
			if(StartVec[ix] == 0)
				continue;
			std::vector<int> vecTemp;
			vecTemp.clear();
			GetReportDateArr(vecTemp,StartVec[ix],EndVec[ix]);
			mapDate.insert(std::pair<int,std::vector<int>>(ix,vecTemp));
		}		
	}
	//分组统计
	if(iType >1)
	{
		std::vector<int> ColVector;		//根据哪些列进行统计  
		if(iType & 2)
			ColVector.push_back(11);
		if(iType & 4)
			ColVector.push_back(3);
		if(iType & 8)
			ColVector.push_back(12);
		AddUpAnalysis(resTable,ColVector,bCalculateValueRate,mapDate,netValueTable);
	}

	//为加权平均添加一行。
	resTable.AddRow();
	int icount2 = 0;
	if(iType >1)
		icount2 = resTable.GetRowCount();//分组统计之后resTable里的记录数
#ifdef _DEBUG
	strTable1=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable1);
#endif
	int iCol = (int)resTable.GetColCount();
	tempCol = 13;//该列是净值增长率
	double share,dRate;
	std::unordered_map<int,double> dData1,dData2;
	////初始化dData1,dData2;
	//dData1[1] = 0;//总份额1
	//dData1[2] = 0;//总份额2
	//dData1[3] = 0;//总份额3
	//dData1[4] = 0;//总份额4
	dData2[10] = 0;//折溢价率
	dData1[10] = 0;//折溢价率总份额
	if(bCalculateValueRate)
	{		
		for(int i = tempCol;i < iCol;)
		{
			dData1[i] = 0;
			dData1[i+2] = 0;
			dData1[i+3] = 0;
			dData1[i+4] = 0;
			dData1[i+5] = 0;

			dData2[i] = 0;
			dData2[i+2] = 0;
			dData2[i+3] = 0;
			dData2[i+4] = 0;
			dData2[i+5] = 0;
			i += 6;
		}
	}
	else
	{
		for(int i = tempCol;i < (int)iCol;)
		{
			dData1[i] = 0;
			dData1[i+2] = 0;
			dData1[i+3] = 0;
			dData1[i+4] = 0;

			dData2[i] = 0;
			dData2[i+2] = 0;
			dData2[i+3] = 0;
			dData2[i+4] = 0;
			i += 5;
		}
	}

	//------------------------------------
	int m_iStart = ::GetTickCount();

	// =================================
	// 取所有有效的日期，然后根据得到的日期集合
	// 更新本地缓存的所有基金的份额信息。
	{
		std::set<int> allDates;		

		for(int m = 0;m < icount;m++)
		{
			std::vector<int> vecDate;
			int iSecId;
			resTable.GetCell(0,m,iSecId);
			Security* pSec = (Security*)GetSecurity(iSecId);
			if(pSec == NULL)
				continue;

			int ixTemp = -1;
			if((int)StartVec.size() != (int)mapDate.size())
			{
				int m_iDateListed;
				m_iDateListed = pSec->GetFundEstablishDate();
				for(int ix = 0; ix < m_iSizeOfDate; ix++)
				{
					if(StartVec[ix] != 0)
						continue;

					std::vector<int> vecTemp;
					vecTemp.clear();
					GetReportDateArr(vecTemp,m_iDateListed,EndVec[ix]);
					mapDate.insert(std::pair<int,std::vector<int>>(ix,vecTemp));
					ixTemp = ix;
				}		
			}

			std::unordered_map<int,std::vector<int>>::iterator iter = mapDate.find( 0 );
			if ( iter != mapDate.end() )
				vecDate = iter->second;

			std::copy(vecDate.begin(),vecDate.end(),std::inserter(allDates,allDates.end()));

			if(bCalculateValueRate)
			{
				int ix = 0;
				for(int j = tempCol;j < (int)iCol;)
				{
					std::unordered_map<int,std::vector<int>>::iterator ite = mapDate.find( ix );
					if ( ite != mapDate.end() )
						vecDate = ite->second;

					std::copy(vecDate.begin(),vecDate.end(),std::inserter(allDates,allDates.end()));

					j += 6;
					ix++;
				}
			}
			else
			{
				int ix = 0;
				for(int j = tempCol;j < (int)iCol;)
				{
					std::unordered_map<int,std::vector<int>>::iterator ite = mapDate.find( ix );
					if ( ite != mapDate.end() )
						vecDate = ite->second;

					std::copy(vecDate.begin(),vecDate.end(),std::inserter(allDates,allDates.end()));

					j += 5;
					ix++;
				}
			}

			if(ixTemp >= 0)
				mapDate.erase(ixTemp);
		}

		for(auto it = allDates.begin();it!=allDates.end();it++)
		{
			_M_dbContext.get_total_share_db_set(*it,true);
		}
	}
	// 取所有有效的日期，然后根据得到的日期集合
	// 更新本地缓存的所有基金的份额信息。
	// ======================================

	for(int m = 0;m < icount;m++)
	{
		std::vector<int> vecDate;
		int iSecId;
		resTable.GetCell(0,m,iSecId);
		Security* pSec = (Security*)GetSecurity(iSecId);
		if(pSec == NULL)
			continue;

		int ixTemp = -1;
		if((int)StartVec.size() != (int)mapDate.size())
		{
			int m_iDateListed;
			m_iDateListed = pSec->GetFundEstablishDate();
			for(int ix = 0; ix < m_iSizeOfDate; ix++)
			{
				if(StartVec[ix] != 0)
					continue;

				std::vector<int> vecTemp;
				vecTemp.clear();
				GetReportDateArr(vecTemp,m_iDateListed,EndVec[ix]);
				mapDate.insert(std::pair<int,std::vector<int>>(ix,vecTemp));
				ixTemp = ix;
			}		
		}

		std::unordered_map<int,std::vector<int>>::iterator iter = mapDate.find( 0 );
		if ( iter != mapDate.end() )
			vecDate = iter->second;
		
		share = GetAvgNetValueReportDateArr(vecDate,iSecId,netValueTable);
		resTable.GetCell(10,m,dRate);
		if(fabs(dRate - Tx::Core::Con_doubleInvalid) > 0.00001)
		{
			dData2[10] += share*dRate;
			dData1[10] += share;
		}

		if(bCalculateValueRate)
		{
			int ix = 0;
			for(int j = tempCol;j < (int)iCol;)
			{
				std::unordered_map<int,std::vector<int>>::iterator ite = mapDate.find( ix );
				if ( ite != mapDate.end() )
					vecDate = ite->second;
				
				share = GetAvgNetValueReportDateArr(vecDate,iSecId,netValueTable);

				resTable.GetCell(j,m,dRate);//净值增长率
				if(fabs(dRate - Tx::Core::Con_doubleInvalid) > 0.00001)
				{
					dData2[j] += share*dRate;
					dData1[j] += share;
				}
				resTable.GetCell(j+2,m,dRate);//同期市场比较
				if(fabs(dRate - Tx::Core::Con_doubleInvalid) > 0.00001)
				{
					dData2[j+2] += share*dRate;
					dData1[j+2] += share;
				}
				resTable.GetCell(j+3,m,dRate);//业绩基准收益率
				if(fabs(dRate - Tx::Core::Con_doubleInvalid) > 0.00001)
				{
					dData2[j+3] += share*dRate;
					dData1[j+3] += share;
				}
				resTable.GetCell(j+4,m,dRate);//同期业绩基准比较
				if(fabs(dRate - Tx::Core::Con_doubleInvalid) > 0.00001)
				{
					dData2[j+4] += share*dRate;
					dData1[j+4] += share;
				}
				resTable.GetCell(j+5,m,dRate);//平均折溢价率
				if(fabs(dRate - Tx::Core::Con_doubleInvalid) > 0.00001)
				{
					dData2[j+5] += share*dRate;
					dData1[j+5] += share;
				}
				j += 6;
				ix++;
			}
		}
		else
		{
			int ix = 0;
			for(int j = tempCol;j < (int)iCol;)
			{
				std::unordered_map<int,std::vector<int>>::iterator ite = mapDate.find( ix );
				if ( ite != mapDate.end() )
					vecDate = ite->second;
				
				share = GetAvgNetValueReportDateArr(vecDate,iSecId,netValueTable);

				resTable.GetCell(j,m,dRate);
				if(fabs(dRate - Tx::Core::Con_doubleInvalid) > 0.00001)
				{
					dData2[j] += share*dRate;
					dData1[j] += share;
				}
				resTable.GetCell(j+2,m,dRate);
				if(fabs(dRate - Tx::Core::Con_doubleInvalid) > 0.00001)
				{
					dData2[j+2] += share*dRate;
					dData1[j+2] += share;
				}
				resTable.GetCell(j+3,m,dRate);//业绩基准收益率
				if(fabs(dRate - Tx::Core::Con_doubleInvalid) > 0.00001)
				{
					dData2[j+3] += share*dRate;
					dData1[j+3] += share;
				}
				resTable.GetCell(j+4,m,dRate);//同期业绩基准比较
				if(fabs(dRate - Tx::Core::Con_doubleInvalid) > 0.00001)
				{
					dData2[j+4] += share*dRate;
					dData1[j+4] += share;
				}
				j += 5;
				ix++;
			}
		}
		if(ixTemp >= 0)
			mapDate.erase(ixTemp);
	}

	int m_iEnd = ::GetTickCount();
	TRACE(_T("权改为资产净值---时间花费%d\r\n"),m_iEnd - m_iStart );
	
	//添加加权平均那一行的数据
	if(iType >1)
		position  = icount2 -1;//这是用来保存往resTable.里添加数据时的行数。
	else
		position = icount;//因为这是在resTable增加了一行以前，计算的resTable的行数，所以此时就不再减一了。
	
	//modified by zhangxs 20081224
	if (dData1[10] != 0 && iType == 1) //只有样本分类为单只基金时，才显示平均折溢价率
	{
		dData2[10] = dData2[10]/dData1[10];
		if(fabs(dData2[10] - Tx::Core::Con_doubleInvalid) < 0.000001)
		{
			double dTempData = Tx::Core::Con_doubleInvalid;
			resTable.SetCell(10,position,dTempData);
		}
		else
			resTable.SetCell(10,position,dData2[10]);
	} 
	else
	{
		resTable.SetCell(10,position,Con_doubleInvalid);
	}
	
	for(int k = tempCol;k < (int)iCol;)
	{
		if(bCalculateValueRate)
		{
			//净值增长率
			if(dData1[k] != 0)
			{
				dData2[k] = dData2[k]/dData1[k];
				if(fabs(dData2[k] - Tx::Core::Con_doubleInvalid) < 0.000001)
				{
					resTable.SetCell(k,position,Con_doubleInvalid);
				}
				else
					resTable.SetCell(k,position,dData2[k]);
			}
			else
				resTable.SetCell(k,position,Con_doubleInvalid);

			//同期市场比较
			if(dData1[k+2] != 0)
			{
				dData2[k+2] = dData2[k+2]/dData1[k+2];
				if(fabs(dData2[k+2] - Tx::Core::Con_doubleInvalid) < 0.000001)
				{
					resTable.SetCell(k+2,position,Tx::Core::Con_doubleInvalid);
				}
				else
					resTable.SetCell(k+2,position,dData2[k+2]);
			}
			else
				resTable.SetCell(k+2,position,Con_doubleInvalid);

			//业绩基准收益率
			if(dData1[k+3] != 0)
			{
				dData2[k+3] = dData2[k+3]/dData1[k+3];
				if(fabs(dData2[k+3] - Tx::Core::Con_doubleInvalid) < 0.000001)
				{
					resTable.SetCell(k+3,position,Tx::Core::Con_doubleInvalid);
				}
				else
					resTable.SetCell(k+3,position,dData2[k+3]);
			}
			else
				resTable.SetCell(k+3,position,Con_doubleInvalid);

			//同期业绩基准比较
			if(dData1[k+4] != 0)
			{
				dData2[k+4] = dData2[k+4]/dData1[k+4];
				if(fabs(dData2[k+4] - Tx::Core::Con_doubleInvalid) < 0.000001)
				{
					resTable.SetCell(k+4,position,Tx::Core::Con_doubleInvalid);
				}
				else
					resTable.SetCell(k+4,position,dData2[k+4]);
			}
			else
				resTable.SetCell(k+4,position,Con_doubleInvalid);

			//平均折溢价率
			if(dData1[k+5] != 0)
			{
				dData2[k+5] = dData2[k+5]/dData1[k+5];
				if(fabs(dData2[k+5] - Tx::Core::Con_doubleInvalid) < 0.000001)
				{
					resTable.SetCell(k+5,position,Tx::Core::Con_doubleInvalid);
				}
				else
					resTable.SetCell(k+5,position,dData2[k+5]);
			}
			else
				resTable.SetCell(k+5,position,Con_doubleInvalid);
			k += 6;
		}
		else
		{
			if(dData1[k] != 0)
			{
				dData2[k] = dData2[k]/dData1[k];
				if(fabs(dData2[k] - Tx::Core::Con_doubleInvalid) < 0.000001)
				{
					resTable.SetCell(k,position,Con_doubleInvalid);
				}
				else
					resTable.SetCell(k,position,dData2[k]);
			}
			else
				resTable.SetCell(k,position,Con_doubleInvalid);

			if(dData1[k+2] != 0)
			{
				dData2[k+2] = dData2[k+2]/dData1[k+2];
				if(fabs(dData2[k+2] - Tx::Core::Con_doubleInvalid) < 0.000001)
				{
					resTable.SetCell(k+2,position,Tx::Core::Con_doubleInvalid);
				}
				else
					resTable.SetCell(k+2,position,dData2[k+2]);
			}
			else
				resTable.SetCell(k+2,position,Con_doubleInvalid);
			//业绩基准收益率
			if(dData1[k+3] != 0)
			{
				dData2[k+3] = dData2[k+3]/dData1[k+3];
				if(fabs(dData2[k+3] - Tx::Core::Con_doubleInvalid) < 0.000001)
				{
					resTable.SetCell(k+3,position,Tx::Core::Con_doubleInvalid);
				}
				else
					resTable.SetCell(k+3,position,dData2[k+3]);
			}
			else
				resTable.SetCell(k+3,position,Con_doubleInvalid);

			//同期业绩基准比较
			if(dData1[k+4] != 0)
			{
				dData2[k+4] = dData2[k+4]/dData1[k+4];
				if(fabs(dData2[k+4] - Tx::Core::Con_doubleInvalid) < 0.000001)
				{
					resTable.SetCell(k+4,position,Tx::Core::Con_doubleInvalid);
				}
				else
					resTable.SetCell(k+4,position,dData2[k+4]);
			}
			else
				resTable.SetCell(k+4,position,Con_doubleInvalid);

			k += 5;
		}
	}
	CString strTemp = _T("-");
	resTable.SetCell(2,position,strTemp);
	resTable.SetCell(4,position,strTemp);
//	resTable.SetCell(6,position,Tx::Core::Con_doubleInvalid);
	resTable.SetCell(5,position,Tx::Core::Con_intInvalid);
	for (int i = 6;i < 10;i++)
	{
		resTable.SetCell(i,position,Tx::Core::Con_doubleInvalid);
	}
	//把增长率排名，也填充为“-”
	tempCol = 14;         //该列是增长率排名
	for(int j = tempCol;j < (int)iCol;)
	{
		resTable.SetCell(j,position,Tx::Core::Con_intInvalid);
		if(!(iType&1))
			resTable.SetCell(j+1,position,Tx::Core::Con_doubleInvalid);
		if(bCalculateValueRate)
			j += 6;
		else
			j +=5;
	}
	strTemp = _T("净值加权平均");
	resTable.SetCell(1,position,strTemp);
	//如果没有选单只基金的，要把那些数据删掉
	if(!(iType&1))
	{
		for(int j = icount-1;j >= 0 ;j--)
			resTable.DeleteRow(j);
	}	
	resTable.DeleteCol(11,2);//删除管理公司ID和托管银行ID
	resTable.DeleteCol(3);//删除基金风格int那一列；
//	resTable.SetSortRange(0,resTable.GetRowCount()-2);
	//添加进度条
	prw.SetPercent(pid,1.0);
	//CLogRecorder::GetInstance()->WriteToLog(_T("StatFundNetValueAnalyse::OK"));
	return true;

}
void TxFund::GetReportDateArr(std::vector<int>& VecDate,int iStart,int iEnd)
{
	VecDate.clear();
	if(iStart < 19900000 || iStart > 25001231 || iEnd < 19900000 || iEnd > 25001231)
		return;
	for(int iDay = iStart +1;iDay <= iEnd;iDay++)
	{
		int ix = iDay %10000;
		if(ix == 331|| ix == 630 || ix == 930 || ix == 1231)
			VecDate.push_back(iDay);
	}
	if((int)VecDate.size() < 1)
	{
		VecDate.push_back(iStart);
	}
}
double TxFund::GetAvgNetValueReportDateArr(std::vector<int> vecDate, int SecId)
{
	int iSum = 0;
	TxFundShareData* pTempFundShare = NULL;
	FundNetValueData* pTempFundNet = NULL;
	double AvgNetValue = 0.0;
	if((int)vecDate.size()<1)
		return  AvgNetValue;
	Security* pSec = (Security*)GetSecurity(SecId);
	if(pSec == NULL)
		return AvgNetValue;
	for(std::vector<int>::iterator ite = vecDate.begin();ite != vecDate.end(); ite++)
	{						
		pTempFundShare = NULL;
		pTempFundNet = NULL;
		pTempFundNet = pSec->GetFundNetValueDataByDate(*ite);
		//pTempFundShare = pSec->GetTxFundShareDataByDate(*ite);
		if(pTempFundNet != NULL)
		{
			if( pTempFundNet->fNetvalue < 0.001)
				continue;	
			int m_iDateReport = pTempFundNet->iDate;
			//pTempFundNet = pSec->GetFundNetValueDataByDate(m_iDateReport);
			pTempFundShare = pSec->GetTxFundShareDataByDate(m_iDateReport);
			if( pTempFundShare != NULL)
			{						
				if(pTempFundShare->TotalShare < 0.001 )
					continue;					
				if(pTempFundShare->TotalShare > 0 && pTempFundNet->fNetvalue > 0)
				{

					double dTemp = (pTempFundShare->TotalShare)*(pTempFundNet->fNetvalue);
					AvgNetValue += (pTempFundShare->TotalShare)*(pTempFundNet->fNetvalue);
					iSum++;
				}
			}
		}
	}
	if(iSum > 0)
		AvgNetValue /= iSum;
	return AvgNetValue;
}

double TxFund::GetAvgNetValueReportDateArr(std::vector<int> vecDate, int SecId,Tx::Core::Table_Indicator & netValueTable)
{
	int iSum = 0;	
	double AvgNetValue = 0.0;

	if((int)vecDate.size()<1)
		return  AvgNetValue;

	int rowCount = netValueTable.GetRowCount();
	int	rowIndex = -1;

	for(int i=0;i<rowCount;i++)
	{
		int tmpId;
		netValueTable.GetCell(0,i,tmpId);
		if(tmpId <= 0)
			break;
		if(tmpId == SecId)
		{
			rowIndex = i;
			break;
		}
	}

	if(rowIndex<0)
		return AvgNetValue;

	Security* pSec = (Security*)GetSecurity(SecId);
	if(pSec == NULL)
		return AvgNetValue;	

	for(std::vector<int>::iterator ite = vecDate.begin();ite != vecDate.end(); ite++)
	{						
		int colIndex = -1;

		//int maxDate = 21001231;//表里的最大日期		
		for(int i=0;i<rowCount;i++/*,g_i++*/)
		{
			int tmpDate;
			netValueTable.GetCell(1,i,tmpDate);
			if(tmpDate <= 0)
				break;
			
			if(tmpDate == *ite)
			{
				colIndex = i;
				break;
			}
		}

		if(colIndex<0)
			return AvgNetValue;
		else
			colIndex += 2;

		double netValue;//从表里取单位净值
		netValueTable.GetCell(colIndex,rowIndex,netValue);
		if( netValue < 0.001)
			continue;
		
		auto dbset = _M_dbContext.get_total_share_db_set(*ite,false);
		ASSERT(dbset != nullptr);

		double totalShare = dbset->get_value(SecId);

		if(totalShare < 0.001 )
			continue;

		AvgNetValue += (totalShare)*netValue;
		iSum++;
	}

	if(iSum > 0)
		AvgNetValue /= iSum;

	return AvgNetValue;
}

void TxFund::AddUpAnalysis(Tx::Core::Table &resTable,		//存放结果结果表
				   std::vector<int> ColVector,		//根据那些列进行统计
				   bool	bCalculateValueRate,
				   std::unordered_map<int,std::vector<int>> mapDate,
				   Tx::Core::Table_Indicator & netValueTable
				   )
{
	std::vector<int>::iterator iterCol;
	int iIdCol,itempId;
	std::set<int> iIdSet;
	std::set<int>::iterator iterSet;
	std::vector<UINT> PositionCollect;
	std::vector<UINT>::iterator UterV;
	double share,dRate;
	std::unordered_map<int,double> dData1,dData2;
	int tempCol = 13;//该列是净值增长率
	int iTempRow = resTable.GetRowCount();
	int iCol = resTable.GetColCount();
	Tx::Core::Table tempTable;
	tempTable.CopyColumnInfoFrom(resTable);	
	int iRow = -1;//用于保存分组统计的记录数。
	//根据那个参数列进行相加
#ifdef _DEBUG
	CString	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	for(iterCol = ColVector.begin();iterCol != ColVector.end();++iterCol)
	{
		iIdCol = *iterCol;
		for(int i = 0;i < iTempRow;i++)
		{
			resTable.GetCell(iIdCol,i,itempId);
			iIdSet.insert(itempId);
		}
		//根据ID进行相加
		for(iterSet = iIdSet.begin();iterSet != iIdSet.end();++iterSet)
		{			
			itempId = *iterSet;	
			if(!PositionCollect.empty())
				PositionCollect.clear();
			resTable.Find(iIdCol,itempId,PositionCollect);//取得与公司ID相同的纪录的位置。
			if(PositionCollect.empty())
				continue;
			//每根据一个ID相加，就增加一行。
			tempTable.AddRow();
			iRow++;
			//初始化dData1,dData2;
			//dData1[6] = 0;
			if(bCalculateValueRate)
			{		
				for(int i = tempCol;i < iCol;)
				{
					dData1[i] = 0;
					dData1[i+2] = 0;
					dData1[i+3] = 0;
					dData1[i+4] = 0;

					dData2[i] = 0;
					dData2[i+2] = 0;
					dData2[i+3] = 0;
					dData2[i+4] = 0;
					i += 6;
				}
			}
			else
			{
				for(int i = tempCol;i < iCol;)
				{
					dData1[i] = 0;
					dData1[i+2] = 0;
					dData1[i+3] = 0;
					dData1[i+4] = 0;

					dData2[i] = 0;
					dData2[i+2] = 0;
					dData2[i+3] = 0;
					dData2[i+4] = 0;
					i += 5;
				}
			}
			int position;
			std::vector<int> vecDate;
			for(UterV = PositionCollect.begin();UterV != PositionCollect.end();++UterV)
			{
				position = *UterV;
				//resTable.GetCell(6,position,share);//份额。
				int ix = 0;
				for(int j = tempCol;j < iCol;)
				{
					int iSecId = 0;
					resTable.GetCell(0,position,iSecId);
					std::unordered_map<int,std::vector<int>>::iterator ite = mapDate.find( ix );
					if ( ite != mapDate.end() )
						vecDate = ite->second;
					//share = GetAvgNetValueReportDateArr(vecDate,iSecId);
					share = GetAvgNetValueReportDateArr(vecDate,iSecId,netValueTable);
					ix++;
					resTable.GetCell(j,position,dRate);
					if(fabs(dRate - Tx::Core::Con_doubleInvalid) > 0.000001 && fabs(share - Tx::Core::Con_doubleInvalid) > 0.000001)
					{
						dData2[j] += share*dRate;
						dData1[j] += share;
					}
					resTable.GetCell(j+2,position,dRate);
					if(fabs(dRate - Tx::Core::Con_doubleInvalid) > 0.000001 && fabs(share - Tx::Core::Con_doubleInvalid) > 0.000001)
					{
						dData2[j+2] += share*dRate;	
						dData1[j+2] += share;
					}
					resTable.GetCell(j+3,position,dRate);
					if(fabs(dRate - Tx::Core::Con_doubleInvalid) > 0.000001 && fabs(share - Tx::Core::Con_doubleInvalid) > 0.000001)
					{
						dData2[j+3] += share*dRate;	
						dData1[j+3] += share;
					}
					resTable.GetCell(j+4,position,dRate);
					if(fabs(dRate - Tx::Core::Con_doubleInvalid) > 0.000001 && fabs(share - Tx::Core::Con_doubleInvalid) > 0.000001)
					{
						dData2[j+4] += share*dRate;	
						dData1[j+4] += share;
					}
					if(bCalculateValueRate)
						j += 6;
					else
						j += 5;		
					
				}		
				//dData1[6] += share;
			}		
			for(int k = tempCol;k < iCol;)
			{
				//因为在上面已经判断过是否是无效值，在这就不用判断了。
				// added by zhoup 2009.05.04
				// 分母时0.0就不用判断了????
				if (dData1[k] > 0.0)
					dData2[k] = dData2[k]/dData1[k];
				else
					dData2[k] = Tx::Core::Con_doubleInvalid;
				if(fabs(dData2[k] - Tx::Core::Con_doubleInvalid) < 0.000001)
				{
					tempTable.SetCell(k,iRow,Tx::Core::Con_doubleInvalid);
				}
				else
					tempTable.SetCell(k,iRow,dData2[k]);
				// added by zhoup 2009.05.04
				// 分母时0.0就不用判断了????
				if (dData1[k+2] > 0.0)
					dData2[k+2] = dData2[k+2]/dData1[k+2];
				else
					dData2[k+2] = Tx::Core::Con_doubleInvalid;
				if(fabs(dData2[k+2] - Tx::Core::Con_doubleInvalid) < 0.000001)
				{
					tempTable.SetCell(k+2,iRow,Tx::Core::Con_doubleInvalid);
				}
				else
					tempTable.SetCell(k+2,iRow,dData2[k+2]);		

				if (dData1[k+3] > 0.0)
					dData2[k+3] = dData2[k+3]/dData1[k+3];
				else
					dData2[k+3] = Tx::Core::Con_doubleInvalid;
				if(fabs(dData2[k+3] - Tx::Core::Con_doubleInvalid) < 0.000001)
				{
					tempTable.SetCell(k+3,iRow,Tx::Core::Con_doubleInvalid);
				}
				else
					tempTable.SetCell(k+3,iRow,dData2[k+3]);	

				if (dData1[k+4] > 0.0)
					dData2[k+4] = dData2[k+4]/dData1[k+4];
				else
					dData2[k+4] = Tx::Core::Con_doubleInvalid;
				if(fabs(dData2[k+4] - Tx::Core::Con_doubleInvalid) < 0.000001)
				{
					tempTable.SetCell(k+4,iRow,Tx::Core::Con_doubleInvalid);
				}
				else
					tempTable.SetCell(k+4,iRow,dData2[k+4]);	

				if(bCalculateValueRate)
					k += 6;
				else
					k += 5;				
			}
			//添加名称
			std::unordered_map<int,CString>::iterator iterMap;
			switch(iIdCol)
			{
			case 3:
				{
					std::unordered_map<int,CString> StyleMap;
					//modified by zhangxs 20091221---NewStyle
					//TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX,StyleMap);	
					TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX_NEW,StyleMap);
					iterMap = StyleMap.find(itempId);
					if(iterMap != StyleMap.end())
						tempTable.SetCell(1,iRow,iterMap->second);
					else
						tempTable.SetCell(1,iRow,(CString)_T("未知类型"));
				}
				break;
			case 11:
				{
					std::unordered_map<int,CString> CompanyMap;
					TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_INSTITUTIONID_TO_SHORTNAME,CompanyMap);
					iterMap = CompanyMap.find(itempId);
					if(iterMap != CompanyMap.end())
						tempTable.SetCell(1,iRow,iterMap->second);
				}
				break;
			case 12:
				{
					std::unordered_map<int,CString> BankMap;
					TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_BANK,BankMap);
					iterMap = BankMap.find(itempId);
					if(iterMap != BankMap.end())
						tempTable.SetCell(1,iRow,iterMap->second);
				}
				break;
			default:
				break;
			}
			tempTable.SetCell(2,iRow,Tx::Core::Con_strInvalid);
			tempTable.SetCell(4,iRow,Tx::Core::Con_strInvalid);
			tempTable.SetCell(5,iRow,Tx::Core::Con_intInvalid);
//			tempTable.SetCell(8,iRow,Tx::Core::Con_doubleInvalid);			
			for (int i = 6;i <= 10;i++)
			{
				tempTable.SetCell(i,iRow,Tx::Core::Con_doubleInvalid);
			}
			for(int j = tempCol+1;j < iCol;)
			{
				tempTable.SetCell(j,iRow,Tx::Core::Con_intInvalid);
				if(bCalculateValueRate)
				{
					tempTable.SetCell(j+4,iRow,Tx::Core::Con_doubleInvalid);
					j += 6;
				}
				else
					j +=5;
			}
		}
	}
#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//把singleDateTable里的数据拷贝到resTable;
	resTable.AppendTableByRow(tempTable);
#ifdef _DEBUG
	strTable=tempTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
}

bool TxFund::StatFundHoldingStatistics(	
		Tx::Core::Table_Indicator &resTable,
		std::vector<int>	iSecurityId,
		std::vector<int>	iDate,
		int		iType
		)
{
	//添加进度条
//	ProgressWnd* pwd=Tx::Core::ProgressWnd::GetInstance();
	ProgressWnd prw;
	UINT pid=prw.AddItem(1,_T("持仓结构统计..."),0.0);
	prw.Show(1000);
	m_txTable.Clear();//这是引用别人的成员变量，
	//从T_ASSET_ALLOCATION_twoyear里取基金定期报告ID，基金ID，报告年份，报告期
	//默认的返回值状态
	bool result = false;
	//清空数据
	m_txTable.Clear();
	//准备样本集参数列
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//基金ID
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//报告年份
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//报告期
	const int indicatorIndex = 4;
	long iIndicator[indicatorIndex] = 
	{
		30901140,	//资产净值
		30901132,	//债券合计
		30901131,	//股票市值
		30901343	//合并重仓投资市值
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
			return FALSE;
		}

	}
	UINT iColCount = m_txTable.GetColCount();
	UINT* nColArray = new UINT[iColCount];
	for(int i = 0; i < (int)iColCount; i++)
	{
		nColArray[i] = i;
	}
	result = m_pLogicalBusiness->GetData(m_txTable,true);
	if(result == false)
	{
		delete nColArray;
		nColArray = NULL;
		//添加进度条
		prw.SetPercent(pid,1.0);
		return false;
	}
	CString strTable;
	CString strTableTmp;
#ifdef _DEBUG
	strTable=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//把交易实体ID转化为基金ID
	std::vector<int> iSecurity1Id;
	std::vector<int>::iterator iterV;
	std::vector<int> tempTradeV;
	int tempId;
	for (iterV = iSecurityId.begin();iterV != iSecurityId.end();++iterV)
	{
		GetSecurityNow(*iterV);
		if (m_pSecurity != NULL)
		{
			tempId = m_pSecurity->GetSecurity1Id(*iterV);
//#ifdef _SECURITYID_FOR_STAT_
//			tempId = *iterV;
//#endif
			if (find(iSecurity1Id.begin(),iSecurity1Id.end(),tempId) == iSecurity1Id.end())
			{
				iSecurity1Id.push_back(tempId);
				tempTradeV.push_back(*iterV);
			}
		}
	}

	iSecurityId.clear();
	iSecurityId.assign(tempTradeV.begin(),tempTradeV.end());
	Tx::Core::Table_Indicator tempTable;
	tempTable.CopyColumnInfoFrom(m_txTable);
	//根据基金ID进行筛选
	m_txTable.EqualsAt(tempTable,nColArray,iColCount,0,iSecurity1Id);
	if (tempTable.GetRowCount() == 0)
	{
		//添加进度条
		prw.SetPercent(pid,1.0);
		return false;
	}
	//为了进行年度和报告期的筛选，所以把报告年份和报告期和为一列。
	TransReportDateToNormal2(tempTable,1);
	resTable.Clear();
	resTable.CopyColumnInfoFrom(tempTable);
	//进行年度和报告期的筛选
	tempTable.EqualsAt(resTable,nColArray,iColCount-1,1,iDate);
#ifdef _DEBUG
		strTable=resTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	delete nColArray ;
	nColArray = NULL;
    //添加进度条
	prw.SetPercent(pid,0.3);

	//为增加基金的交易实体ID和名称，代码作准备
	resTable.InsertCol(0,Tx::Core::dtype_int4);//基金交易实体ID
	resTable.InsertCol(1,Tx::Core::dtype_val_string);//基金名称
	resTable.InsertCol(2,Tx::Core::dtype_val_string);//基金代码
	resTable.InsertCol(7,Tx::Core::dtype_double);//债券占净值比例
	resTable.InsertCol(9,Tx::Core::dtype_double);//股票占净值比例
	resTable.AddCol(Tx::Core::dtype_double);//11重仓股占净值比
	resTable.AddCol(Tx::Core::dtype_double);//12持股集中度
	
	//增加基金的交易实体ID和名称，代码
	double Equity,bondEquity,stockEquity,stockRate,ZCEquity,ZCRate,dJZD;
	int position;
	std::vector<int>::iterator iterId;
//	if(iType >1)//在此我得出一个结论，那就是不能把定义变量的语句单独放在条件语句中。   
	std::set<int> fundIdSet;
	for(iterId = iSecurityId.begin();iterId != iSecurityId.end();++iterId)
	{
		int fundId  ;
		CString strName,strCode;
		GetSecurityNow(*iterId);
		if(m_pSecurity == NULL)
			continue;
//#ifdef _SECURITYID_FOR_STAT_
//		fundId = *iterId;
//		if(iType >1)
//			fundIdSet.insert(*iterId);
//#else
		fundId = m_pSecurity->GetSecurity1Id();
		if(iType >1)
			fundIdSet.insert(fundId);
//#endif
		
		strName = m_pSecurity->GetName();
		strCode = m_pSecurity->GetCode();
		std::vector<UINT> vecInstiID;
		resTable.Find(3,fundId,vecInstiID);
		std::vector<UINT>::iterator iteID;
		for(iteID = vecInstiID.begin();iteID != vecInstiID.end();++iteID)
		{
			position = *iteID;
			resTable.SetCell(0,position,*iterId);
			resTable.SetCell(1,position,strName);
			resTable.SetCell(2,position,strCode);
			//计算各个占净值的比例
            resTable.GetCell(5,position,Equity);//取出资产净值			
			resTable.GetCell(6,position,bondEquity);
			resTable.GetCell(8,position,stockEquity);			
			resTable.GetCell(10,position,ZCEquity);
			if (Equity == 0.0)
			{
				bondEquity = Tx::Core::Con_doubleInvalid;
				resTable.SetCell(7,position,bondEquity);
				stockRate = Tx::Core::Con_doubleInvalid;
				resTable.SetCell(9,position,stockRate);
				ZCRate = Tx::Core::Con_doubleInvalid;
				resTable.SetCell(11,position,ZCRate);
			} 
			else
			{
				//计算债券占净值比例
				bondEquity = bondEquity/Equity*100;
				if(bondEquity <= 0)
					bondEquity = Tx::Core::Con_doubleInvalid;
				resTable.SetCell(7,position,bondEquity);
				//计算股票占净值比例
				stockRate = stockEquity/Equity*100;
				if(stockRate <= 0)
					stockRate = Tx::Core::Con_doubleInvalid;
				resTable.SetCell(9,position,stockRate);
				//计算重仓股占净值比例
				ZCRate = ZCEquity/Equity*100;
				if(ZCRate <= 0)
					ZCRate = Tx::Core::Con_doubleInvalid;
				resTable.SetCell(11,position,ZCRate);
			}
			if (stockEquity == 0.0)
			{
				dJZD = Tx::Core::Con_doubleInvalid;
				resTable.SetCell(12,position,dJZD);
			} 
			else
			{
				//计算持股集中度
				dJZD =  ZCEquity/stockEquity*100;  
				if(dJZD <= 0)
					dJZD = Tx::Core::Con_doubleInvalid;
				resTable.SetCell(12,position,dJZD);
			}			
		}
	}
#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	int iType2 = iType;//保存原来的值.
	//这里增加样本分类选择
	if(iType >1)
	{
		int iCount = resTable.GetRowCount();//累加之前resTable里的行数。
		//为了进行分类统计。所以增加一列基金ID。
		resTable.AddCol(Tx::Core::dtype_int4);//基金风格
		resTable.AddCol(Tx::Core::dtype_int4);//基金管理公司ID
		resTable.AddCol(Tx::Core::dtype_int4);//基金托管银行ID
		int tempfundId;
//		int icount = resTable.GetRowCount();
		//添加进度条
		prw.SetPercent(pid,0.6);
		//这里增加样本分类选择
		m_txTable.Clear();
		tempTable.Clear();
		//准备样本集=第一参数列:F_FUND_ID,int型
		m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
		const int indicatorIndex1 = 3;
		long iIndicator1[indicatorIndex1] = 
		{
			//30001035,	//基金风格
			//modified by zhangxs 20091221---NewStyle
			30001232,	//基金风格New
			30001020,	//管理公司ID，
			30001021	//托管银行ID，
		};
		UINT varCfg1[1];			//参数配置
		int varCount1=1;			//参数个数
		for (int i = 0; i < indicatorIndex1; i++)
		{
			int tempIndicator = iIndicator1[i];
			GetIndicatorDataNow(tempIndicator);
			if (m_pIndicatorData==NULL)
			{
				//添加进度条
				prw.SetPercent(pid,1.0);
				return false;
			} 
			varCfg1[0]=0;
			result = m_pLogicalBusiness->SetIndicatorIntoTable(
				m_pIndicatorData,	//指标
				varCfg1,				//参数配置
				varCount1,			//参数个数
				m_txTable	//计算需要的参数传输载体以及计算后结果的载体
				);
			if(result==false)
			{
				//添加进度条
				prw.SetPercent(pid,1.0);
				return false;
			}

		}
		result = m_pLogicalBusiness->GetData(m_txTable,true);
		if(result==false)
			return false;
#ifdef _DEBUG
		strTableTmp=m_txTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTableTmp);
#endif
		UINT iCol2=m_txTable.GetColCount();
		//复制所有列信息
		tempTable.CopyColumnInfoFrom(m_txTable);
		if(m_txTable.GetRowCount()==0)
		{
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false;
		}
		UINT* nColArray2 = new UINT[iCol2];
		for(UINT i=0;i<iCol2;i++)
			nColArray2[i]=i;
//#ifdef _SECURITYID_FOR_STAT_
		m_txTable.EqualsAt(tempTable,nColArray2,iCol2,0,tempTradeV);
//#else
//		m_txTable.EqualsAt(tempTable,nColArray2,iCol2,0,fundIdSet);
//		
//#endif

#ifdef _DEBUG
		strTableTmp=tempTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTableTmp);
#endif
		delete nColArray2;
		nColArray2 = NULL;
		//把基金风格、公司ID、托管银行ID放到resTable
		std::vector<UINT> vecInstiID2;
		std::vector<UINT>::iterator iteID;
		int position;
		int iStyle,iCompanyId,ibankId;
		int icount = tempTable.GetRowCount();
		for(int m = 0;m < icount;m++)
		{
			tempTable.GetCell(0,m,tempfundId);
			tempTable.GetCell(1,m,iStyle);
			tempTable.GetCell(2,m,iCompanyId);
			tempTable.GetCell(3,m,ibankId);
			//把基金ID和交易实体ID对应起来。并且把数据放到表里。
			if(!(vecInstiID2.empty()))
				vecInstiID2.clear();
//#ifdef _SECURITYID_FOR_STAT_
			resTable.Find(0,tempfundId,vecInstiID2);
//#else
//			resTable.Find(3,tempfundId,vecInstiID2);
//#endif
			for(iteID = vecInstiID2.begin();iteID != vecInstiID2.end();++iteID)
			{
				position = *iteID;
				resTable.SetCell(13,position,iStyle);//基金风格
				resTable.SetCell(14,position,iCompanyId);//管理公司ID
				resTable.SetCell(15,position,ibankId);//托管银行ID
			}
		}
#ifdef _DEBUG
		strTable=resTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
		std::vector<int> ColVector;		//根据哪些列进行统计  
		if(!(iType & 1))
			iType += 1;//假设都选择选择了单只基金那一项分类方式。
		if(iType & 2)
			ColVector.push_back(14);
		if(iType & 4)
			ColVector.push_back(13);
		if(iType & 8)
			ColVector.push_back(15);
		std::vector<int> IntCol;			//需要相加的整型列
//		IntCol.push_back(6);
		std::vector<int> DoubleCol;	//需要相加的double列
		DoubleCol.push_back(5);
		for(int n = 6;n < 11;n++)
		{
			DoubleCol.push_back(n);
			n++;
		}
		AddUpRow(resTable,iType,ColVector,IntCol,DoubleCol,iDate,4);
		std::vector<int>::size_type isize = iDate.size();
		int iCount1 = resTable.GetRowCount();//累加以后resTable里的行数。
		int iCount2,iCount3;
		iCount2 = iCount1 - iCount;//这是增加的行数 样本汇总和其他分类方式的行数。
		std::unordered_map<int,CString> CompanyMap;		
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_INSTITUTIONID_TO_SHORTNAME,CompanyMap);
		std::unordered_map<int,CString> StyleMap;
		//modified by zhangxs 20091221---NewStyle
		//TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX,StyleMap);
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX_NEW,StyleMap);
		std::unordered_map<int,CString> BankMap;
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_BANK,BankMap);
		std::unordered_map<int,CString>::iterator iterMap;
		int id;
		CString Name;		
		if(iType < 17)//没有样本汇总的情况
		{
			for(int m = 0;m < iCount2;m++)
			{				
				resTable.GetCell(13,iCount+m,id);
				if(id == 0)
				{
					resTable.GetCell(14,iCount+m,id);
					if(id == 0)
					{
						resTable.GetCell(15,iCount+m,id);
						iterMap = BankMap.find(id);
						if(iterMap!= BankMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,iCount+m,Name);
						resTable.SetCell(2,iCount+m,(CString)(_T("托管银行")));
						continue;
					}
					else
					{
						iterMap = CompanyMap.find(id);
						if(iterMap!= CompanyMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,iCount+m,Name);
						resTable.SetCell(2,iCount+m,(CString)(_T("基金公司")));
						continue;
					}					
				}
				else
				{
					iterMap = StyleMap.find(id);
					if(iterMap!= StyleMap.end())
						Name = iterMap->second;
					else
						Name = Tx::Core::Con_strInvalid;
					resTable.SetCell(1,iCount+m,Name);
					resTable.SetCell(2,iCount+m,(CString)(_T("基金风格")));
				}
			}				
		}
		else//不但有单只基金和样本汇总，还有其他的分类方式
		{
			iCount3 = iCount2 - isize;//除了样本汇总以外的其他分类方式的行数
			if(iCount3 > 0)//表示除了样本汇总以外还有其他的分类的方式
			{
				//填充样本汇总的那些行
				for(int k = 0;k < (int)isize;k++)
				{
					resTable.SetCell(1,iCount+iCount3+k,(CString)(_T("样本汇总")));
					resTable.SetCell(2,iCount+iCount3+k,(CString)(_T("全部汇总")));
				}
				//填充除了样本汇总以外的其他分类方式的那些行。
				for(int i = 0;i < iCount3;i++)
				{
					resTable.GetCell(13,iCount+i,id);
					if(id == 0)
					{
						resTable.GetCell(14,iCount+i,id);
						if(id == 0)
						{
							resTable.GetCell(15,iCount+i,id);
							iterMap = BankMap.find(id);
							if(iterMap!= BankMap.end())
								Name = iterMap->second;
							else
								Name = Tx::Core::Con_strInvalid;
							resTable.SetCell(1,iCount+i,Name);
							resTable.SetCell(2,iCount+i,(CString)(_T("托管银行")));
							continue;
						}
						else
						{
							iterMap = CompanyMap.find(id);
							if(iterMap!= CompanyMap.end())
								Name = iterMap->second;
							else
								Name = Tx::Core::Con_strInvalid;
							resTable.SetCell(1,iCount+i,Name);
							resTable.SetCell(2,iCount+i,(CString)(_T("基金公司")));
							continue;
						}					
					}
					else
					{
						iterMap = StyleMap.find(id);
						if(iterMap!= StyleMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,iCount+i,Name);
						resTable.SetCell(2,iCount+i,(CString)(_T("基金风格")));
					}
				}
			}
			else//只有单只基金和汇总的情况
			{
				for(int k = 0;k < (int)isize;k++)
				{
					resTable.SetCell(1,iCount+k,(CString)(_T("样本汇总")));
					resTable.SetCell(2,iCount+k,(CString)(_T("全部汇总")));
				}		
			}
		}	
		//把报告年份和报告期转化成CString类型,所以增加一列
		resTable.InsertCol(5,Tx::Core::dtype_val_string);//str报告期	
		CString stryear,strdate;
		int tempdate,iyear ,itemp;
		int tempCount;
		int j;
		if(!(iType2 & 1))
		{
			resTable.DeleteRow(0,iCount);
			tempCount = iCount2;		
			j = 0;	
		}
		else
		{
			for(int i = 0;i < iCount;i++)
			{
				//填写报告期	
				resTable.GetCell(4,i,tempdate);	
				itemp = tempdate%10000;
				iyear = tempdate/10000;
				stryear.Format(_T("%d"),iyear);
				switch(itemp)
				{
				case 331:
					strdate = stryear + _T("年") + _T("一季报");
					break;
				case 630:
					strdate = stryear + _T("年") + _T("二季报");
					break;
				case 930:
					strdate = stryear + _T("年") + _T("三季报");
					break;
				case 1231:
					strdate = stryear + _T("年") + _T("四季报");
					break;
				}
				resTable.SetCell(5,i,strdate);
			}
			tempCount = iCount1;
			j = iCount;
		}
		for(;j < tempCount;j++)
		{
			//填写报告期	
			resTable.GetCell(4,j,tempdate);	
			itemp = tempdate%10000;
			iyear = tempdate/10000;
			stryear.Format(_T("%d"),iyear);
			switch(itemp)
			{
			case 331:
				strdate = stryear + _T("年") + _T("一季报");
				break;
			case 630:
				strdate = stryear + _T("年") + _T("二季报");
				break;
			case 930:
				strdate = stryear + _T("年") + _T("三季报");
				break;
			case 1231:
				strdate = stryear + _T("年") + _T("四季报");
				break;
			}
			resTable.SetCell(5,j,strdate);
			//计算比例
			resTable.GetCell(6,j,Equity);//取得基金净值
			resTable.GetCell(7,j,bondEquity);//债券市值				
			bondEquity = bondEquity*100/Equity;
			if(bondEquity < 0)
				bondEquity = Tx::Core::Con_doubleInvalid;
			resTable.SetCell(8,j,bondEquity);
			resTable.GetCell(9,j,stockEquity);
			if(stockEquity < 0)
				stockRate = Tx::Core::Con_doubleInvalid;
			else
				stockRate = stockEquity*100/Equity;
			resTable.SetCell(10,j,stockRate);
			resTable.GetCell(11,j,ZCEquity);
			if(ZCEquity < 0)
				ZCRate = Tx::Core::Con_doubleInvalid;
			else
				ZCRate = ZCEquity*100/Equity;
			resTable.SetCell(12,j,ZCRate);
			dJZD = ZCEquity*100/stockEquity;
			if(dJZD < 0)
				dJZD = Tx::Core::Con_doubleInvalid;				
			resTable.SetCell(13,j,dJZD);
		}
		resTable.DeleteCol(14,3);		
	}
	else
	{	
		//把报告年份和报告期转化成CString类型,所以增加一列
		resTable.InsertCol(5,Tx::Core::dtype_val_string);//str报告期	
		CString stryear,strdate;
		int tempdate,iyear ,itemp;
		int icount = resTable.GetRowCount();
		for(int j = 0;j < icount;j++)
		{			
			resTable.GetCell(4,j,tempdate);				
			itemp = tempdate%10000;
			iyear = tempdate/10000;
			stryear.Format(_T("%d"),iyear);
			switch(itemp)
			{
			case 331:
				strdate = stryear + _T("年") + _T("一季报");
				break;
			case 630:
				strdate = stryear + _T("年") + _T("二季报");
				break;
			case 930:
				strdate = stryear + _T("年") + _T("三季报");
				break;
			case 1231:
				strdate = stryear + _T("年") + _T("四季报");
				break;
			}
			resTable.SetCell(5,j,strdate);
		}
	}	
	resTable.DeleteCol(3);
	resTable.Arrange();
	MultiSortRule multisort;
	multisort.AddRule(3,false);
	multisort.AddRule(2,true);	
	multisort.AddRule(5,false);	
	resTable.SortInMultiCol(multisort);
	resTable.Arrange();
	resTable.DeleteCol(3);
	/*if(iType2 > 17)
	{
		resTable.SetSortRange(0,resTable.GetRowCount()-2);
	}*/
	//添加进度条
	prw.SetPercent(pid,1.0);
	return true;
}

//add by lijw 2008-03-14
//资产组合统计 t_asset_allocation
bool TxFund::StatAssetAllocation(
		Tx::Core::Table_Indicator &resTable,
		std::vector<int>	iSecurityId,
		std::vector<int>	iDate,
		int		iType
		)
{
	//添加进度条
//	ProgressWnd* pwd=Tx::Core::ProgressWnd::GetInstance();
	ProgressWnd prw;
	UINT pid=prw.AddItem(1,_T("资产组合统计..."),0.0);
	prw.Show(1000);
	m_txTable.Clear();//这是引用别人的成员变量，
	//从T_ASSET_ALLOCATION_twoyear里取基金定期报告ID，基金ID，报告年份，报告期
	//默认的返回值状态
	bool result = false;
	//清空数据
	m_txTable.Clear();
	//准备样本集参数列
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//基金ID
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//报告年份
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//报告期
	const int indicatorIndex = 8;
	long iIndicator[indicatorIndex] = 
	{
		30901141,	//资产总值
		30901131,	//股票市值
		30901132,	//债券合计
		30901133,	//银行存款及清算备付金
		30901134,	//其他资产
		30901135,	//买入返售证券
		30901136,	//应收证券清算款
		30901137	//应收证券清算款		
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
			return FALSE;
		}

	}
	UINT iColCount = m_txTable.GetColCount();
	UINT* nColArray = new UINT[iColCount];
	for(int i = 0; i < (int)iColCount; i++)
	{
		nColArray[i] = i;
	}
	result = m_pLogicalBusiness->GetData(m_txTable,true);
	CString strTable;
	CString strTableTmp;
#ifdef _DEBUG
	strTable = m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	if(result == false)
	{
		delete nColArray;
		nColArray = NULL;
		//添加进度条
		prw.SetPercent(pid,1.0);
		return false;
	}
	//把交易实体ID转化为基金ID
	std::vector<int> iSecurity1Id;
	std::vector<int>::iterator iterVector;
	int tempId;
	std::vector<int> tempTradeV;
	for (iterVector = iSecurityId.begin();iterVector != iSecurityId.end();++iterVector)
	{
		GetSecurityNow(*iterVector);
		if (m_pSecurity != NULL)
		{
			tempId = m_pSecurity->GetSecurity1Id();
			if (find(iSecurity1Id.begin(),iSecurity1Id.end(),tempId) == iSecurity1Id.end())
			{
				iSecurity1Id.push_back(tempId);
				tempTradeV.push_back(*iterVector);
			}
		}
	}
	iSecurityId.clear();
	iSecurityId.assign(tempTradeV.begin(),tempTradeV.end());
	Tx::Core::Table_Indicator tempTable;
	tempTable.CopyColumnInfoFrom(m_txTable);
	//根据基金ID进行筛选
	m_txTable.EqualsAt(tempTable,nColArray,iColCount,0,iSecurity1Id);
	//为了进行年度和报告期的筛选，所以把报告年份和报告期和为一列。
	TransReportDateToNormal2(tempTable,1);
	resTable.Clear();
	resTable.CopyColumnInfoFrom(tempTable);
	//进行年度和报告期的筛选
	tempTable.EqualsAt(resTable,nColArray,iColCount-1,1,iDate);
#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	delete nColArray;
	nColArray = NULL;
	//添加进度条
	prw.SetPercent(pid,0.3);

	//为增加基金的交易实体ID和名称，代码作准备
	resTable.InsertCol(0,Tx::Core::dtype_int4);//基金交易实体ID
	resTable.InsertCol(1,Tx::Core::dtype_val_string);//基金名称
	resTable.InsertCol(2,Tx::Core::dtype_val_string);//基金代码
	resTable.InsertCol(7,Tx::Core::dtype_double);//股票占总值比例
	resTable.InsertCol(9,Tx::Core::dtype_double);//债券占总值比例
	resTable.InsertCol(11,Tx::Core::dtype_double);//银行存款及清算备付金占总值比
	resTable.InsertCol(13,Tx::Core::dtype_double);//其他资产占总值比
	resTable.InsertCol(15,Tx::Core::dtype_double);//买入返售证券占总值比
	resTable.InsertCol(17,Tx::Core::dtype_double);//应收证券清算款占总值比
	resTable.AddCol(Tx::Core::dtype_double);//19权证占总值比

	//增加基金的交易实体ID和名称，代码
	double totalValue,bondRate,stockRate,bankRate,elseRate,buyRate,reckoning,warrantRate;
	int position;
	std::vector<int>::iterator iterId;
	std::set<int> fundIdSet;
	for(iterId = iSecurityId.begin();iterId != iSecurityId.end();++iterId)
	{
		int fundId  ;
		CString strName,strCode;
		GetSecurityNow(*iterId);
		if(m_pSecurity == NULL)
			continue;
//#ifdef _SECURITYID_FOR_STAT_
//		if(iType >1)
//			fundIdSet.insert(*iterId);
//#else
		fundId = m_pSecurity->GetSecurity1Id();
		if(iType >1)
			fundIdSet.insert(fundId);
//#endif
		
		strName = m_pSecurity->GetName();
		strCode = m_pSecurity->GetCode();
		std::vector<UINT> vecInstiID;
		resTable.Find(3,fundId,vecInstiID);
		std::vector<UINT>::iterator iteID;
		for(iteID = vecInstiID.begin();iteID != vecInstiID.end();++iteID)
		{
			position = *iteID;
			resTable.SetCell(0,position,*iterId);
			resTable.SetCell(1,position,strName);
			resTable.SetCell(2,position,strCode);
			//计算各个占总值的比例
			resTable.GetCell(5,position,totalValue);//取出资产总值
			//计算股票占总值比例
			resTable.GetCell(6,position,stockRate);
			stockRate = stockRate/totalValue*100;
			if(stockRate <= 0)
				stockRate = Tx::Core::Con_doubleInvalid;
			resTable.SetCell(7,position,stockRate);
            //计算债券占净值比例
			resTable.GetCell(8,position,bondRate);
			bondRate = bondRate/totalValue*100;
			if(bondRate <= 0)
				bondRate = Tx::Core::Con_doubleInvalid;
			resTable.SetCell(9,position,bondRate);
            //计算银行存款及清算备付金占总值比
			resTable.GetCell(10,position,bankRate);
			bankRate = bankRate/totalValue*100;
			if(bankRate <= 0)
				bankRate = Tx::Core::Con_doubleInvalid;
			resTable.SetCell(11,position,bankRate);
			//计算其他资产占总值比
			resTable.GetCell(12,position,elseRate);
			elseRate =  elseRate/totalValue*100;  
			if(elseRate <= 0)
				elseRate = Tx::Core::Con_doubleInvalid;
			resTable.SetCell(13,position,elseRate);
			//计算买入返售证券占总值比
			resTable.GetCell(14,position,buyRate);
			buyRate =  buyRate/totalValue*100;  
			if(buyRate <= 0)
				buyRate = Tx::Core::Con_doubleInvalid;
			resTable.SetCell(15,position,buyRate);
			//计算应收清算款占总值比
			resTable.GetCell(16,position,reckoning);
			reckoning =  reckoning/totalValue*100;  
			if(reckoning <= 0)
				reckoning = Tx::Core::Con_doubleInvalid;
			resTable.SetCell(17,position,reckoning);
			//计算权证占总值比
			resTable.GetCell(18,position,warrantRate);
			warrantRate =  warrantRate/totalValue*100;  
			if(warrantRate <= 0)
				warrantRate = Tx::Core::Con_doubleInvalid;
			resTable.SetCell(19,position,warrantRate);
		}
	}
#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	int iType2 = iType;//保存原来的值.
	//这里增加样本分类选择
	if(iType >1)
	{
		int iCount = resTable.GetRowCount();//累加之前resTable里的行数。
		//为了进行分类统计。所以增加一列基金ID。
		resTable.AddCol(Tx::Core::dtype_int4);//基金风格
		resTable.AddCol(Tx::Core::dtype_int4);//基金管理公司ID
		resTable.AddCol(Tx::Core::dtype_int4);//基金托管银行ID
		int tempfundId;
//		int icount = resTable.GetRowCount();
		//添加进度条
		prw.SetPercent(pid,0.6);
		//这里增加样本分类选择
		m_txTable.Clear();
		tempTable.Clear();
		//准备样本集=第一参数列:F_FUND_ID,int型
		m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
		const int indicatorIndex1 = 3;
		long iIndicator1[indicatorIndex1] = 
		{
			//30001035,	//基金风格
			//modified by zhangxs 20091221---NewStyle
			30001232,	//基金风格New
			30001020,	//管理公司ID，
			30001021	//托管银行ID，
		};
		UINT varCfg1[1];			//参数配置
		int varCount1=1;			//参数个数
		for (int i = 0; i < indicatorIndex1; i++)
		{
			int tempIndicator = iIndicator1[i];
			GetIndicatorDataNow(tempIndicator);
			if (m_pIndicatorData==NULL)
			{
				//添加进度条
				prw.SetPercent(pid,1.0);
				return false;
			}
			varCfg1[0]=0;
			result = m_pLogicalBusiness->SetIndicatorIntoTable(
				m_pIndicatorData,	//指标
				varCfg1,				//参数配置
				varCount1,			//参数个数
				m_txTable	//计算需要的参数传输载体以及计算后结果的载体
				);
			if(result == false)
			{
				//添加进度条
				prw.SetPercent(pid,1.0);
				return false;
			}
		}
		result = m_pLogicalBusiness->GetData(m_txTable,true);
		if(result == false)
		{
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false;
		}
#ifdef _DEBUG
		strTableTmp = m_txTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTableTmp);
#endif
		UINT iCol2=m_txTable.GetColCount();
		//复制所有列信息
		tempTable.CopyColumnInfoFrom(m_txTable);
		if(m_txTable.GetRowCount()==0)
		{
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false;
		}
		UINT* nColArray2 = new UINT[iCol2];
		for(UINT i=0;i<iCol2;i++)
			nColArray2[i]=i;
//#ifdef _SECURITYID_FOR_STAT_
		m_txTable.EqualsAt(tempTable,nColArray2,iCol2,0,iSecurityId);
//#else
//		m_txTable.EqualsAt(tempTable,nColArray2,iCol2,0,fundIdSet);
//#endif
		delete nColArray2;
		nColArray2 = NULL;
		//把基金风格、公司ID、托管银行ID放到resTable
		std::vector<UINT> vecInstiID2;
		std::vector<UINT>::iterator iteID;
		int position;
		int iStyle,iCompanyId,ibankId;
		int icount = tempTable.GetRowCount();
		for(int j = 0;j < icount;j++)
		{
			tempTable.GetCell(0,j,tempfundId);
			tempTable.GetCell(1,j,iStyle);
			tempTable.GetCell(2,j,iCompanyId);
			tempTable.GetCell(3,j,ibankId);
			//把基金ID和交易实体ID对应起来。并且把数据放到表里。
			if(!(vecInstiID2.empty()))
				vecInstiID2.clear();
//#ifdef _SECURITYID_FOR_STAT_
			resTable.Find(0,tempfundId,vecInstiID2);
//#else
//			resTable.Find(3,tempfundId,vecInstiID2);
//#endif
			for(iteID = vecInstiID2.begin();iteID != vecInstiID2.end();++iteID)
			{
				position = *iteID;
				resTable.SetCell(20,position,iStyle);//基金风格
				resTable.SetCell(21,position,iCompanyId);//管理公司ID
				resTable.SetCell(22,position,ibankId);//托管银行ID
			}
		}
#ifdef _DEBUG
		strTableTmp = resTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTableTmp);
#endif
		std::vector<int> ColVector;		//根据哪些列进行统计  
		if(!(iType & 1))
			iType += 1;//假设都选择选择了单只基金那一项分类方式。
		if(iType & 2)
			ColVector.push_back(21);
		if(iType & 4)
			ColVector.push_back(20);
		if(iType & 8)
			ColVector.push_back(22);
		std::vector<int> IntCol;			//需要相加的整型列
		//		IntCol.push_back(6);
		std::vector<int> DoubleCol;	//需要相加的double列
		DoubleCol.push_back(5);
		for(int j = 6;j < 19;j++)
		{
			DoubleCol.push_back(j);
			j++;
		}
		AddUpRow(resTable,iType,ColVector,IntCol,DoubleCol,iDate,4);
		std::vector<int>::size_type isize = iDate.size();
		int iCount1 = resTable.GetRowCount();//累加以后resTable里的行数。
		int iCount2,iCount3;
		iCount2 = iCount1 - iCount;//这是增加的行数 样本汇总和其他分类方式的行数。
		std::unordered_map<int,CString> CompanyMap;
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_INSTITUTIONID_TO_SHORTNAME,CompanyMap);
		std::unordered_map<int,CString> StyleMap;
		//modified by zhangxs 20091221---NewStyle
		//TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX,StyleMap);
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX_NEW,StyleMap);
		std::unordered_map<int,CString> BankMap;
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_BANK,BankMap);
		std::unordered_map<int,CString>::iterator iterMap;
		int id;
		CString Name;		
		if(iType < 17)//没有样本汇总的情况
		{
			for(int m = 0;m < iCount2;m++)
			{				
				resTable.GetCell(20,iCount+m,id);
				if(id == 0)
				{
					resTable.GetCell(21,iCount+m,id);
					if(id == 0)
					{
						resTable.GetCell(22,iCount+m,id);
						iterMap = BankMap.find(id);
						if(iterMap!= BankMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,iCount+m,Name);
						resTable.SetCell(2,iCount+m,(CString)(_T("托管银行")));
						continue;
					}
					else
					{
						iterMap = CompanyMap.find(id);
						if(iterMap!= CompanyMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,iCount+m,Name);
						resTable.SetCell(2,iCount+m,(CString)(_T("基金公司")));
						continue;
					}					
				}
				else
				{
					iterMap = StyleMap.find(id);
					if(iterMap!= StyleMap.end())
						Name = iterMap->second;
					else
						Name = Tx::Core::Con_strInvalid;
					resTable.SetCell(1,iCount+m,Name);
					resTable.SetCell(2,iCount+m,(CString)(_T("基金风格")));
				}
			}				
		}
		else//不但有单只基金和样本汇总，还有其他的分类方式
		{
			iCount3 = iCount2 - isize;//除了样本汇总以外的其他分类方式的行数
			if(iCount3 > 0)//表示除了样本汇总以外还有其他的分类的方式
			{
				//填充样本汇总的那些行
				for(int k = 0;k < (int)isize;k++)
				{
					resTable.SetCell(1,iCount+iCount3+k,(CString)(_T("样本汇总")));
					resTable.SetCell(2,iCount+iCount3+k,(CString)(_T("全部汇总")));
				}
				//填充除了样本汇总以外的其他分类方式的那些行。
				for(int i = 0;i < iCount3;i++)
				{
					resTable.GetCell(20,iCount+i,id);
					if(id == 0)
					{
						resTable.GetCell(21,iCount+i,id);
						if(id == 0)
						{
							resTable.GetCell(22,iCount+i,id);
							iterMap = BankMap.find(id);
							if(iterMap!= BankMap.end())
								Name = iterMap->second;
							else
								Name = Tx::Core::Con_strInvalid;
							resTable.SetCell(1,iCount+i,Name);
							resTable.SetCell(2,iCount+i,(CString)(_T("托管银行")));
							continue;
						}
						else
						{
							iterMap = CompanyMap.find(id);
							if(iterMap!= CompanyMap.end())
								Name = iterMap->second;
							else
								Name = Tx::Core::Con_strInvalid;
							resTable.SetCell(1,iCount+i,Name);
							resTable.SetCell(2,iCount+i,(CString)(_T("基金公司")));
							continue;
						}					
					}
					else
					{
						iterMap = StyleMap.find(id);
						if(iterMap!= StyleMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,iCount+i,Name);
						resTable.SetCell(2,iCount+i,(CString)(_T("基金风格")));
					}
				}
			}
			else//只有单只基金和汇总的情况
			{
				for(int k = 0;k < (int)isize;k++)
				{
					resTable.SetCell(1,iCount+k,(CString)(_T("样本汇总")));
					resTable.SetCell(2,iCount+k,(CString)(_T("全部汇总")));
				}		
			}
		}	
		//把报告年份和报告期转化成CString类型,所以增加一列
		resTable.InsertCol(5,Tx::Core::dtype_val_string);//str报告期	
		CString stryear,strdate;
		int tempdate,iyear ,itemp;
		int tempCount;
		if(!(iType2 & 1))
		{
			resTable.DeleteRow(0,iCount);
			tempCount = iCount2;			
		}
		else
		{
			tempCount = iCount1;			
		}
		for(int j = 0;j < tempCount;j++)
		{
			//填写报告期	
			resTable.GetCell(4,j,tempdate);	
			itemp = tempdate%10000;
			iyear = tempdate/10000;
			stryear.Format(_T("%d"),iyear);
			switch(itemp)
			{
			case 331:
				strdate = stryear + _T("年") + _T("一季报");
				break;
			case 630:
				strdate = stryear + _T("年") + _T("二季报");
				break;
			case 930:
				strdate = stryear + _T("年") + _T("三季报");
				break;
			case 1231:
				strdate = stryear + _T("年") + _T("四季报");
				break;
			}
			resTable.SetCell(5,j,strdate);
			//计算比例
			resTable.GetCell(6,j,totalValue);//取得资产总值

			resTable.GetCell(7,j,stockRate);
	    	stockRate = stockRate*100/totalValue;
			if(stockRate < 0)
				stockRate = Tx::Core::Con_doubleInvalid;
			resTable.SetCell(8,j,stockRate);

			resTable.GetCell(9,j,bondRate);//债券市值				
			bondRate = bondRate*100/totalValue;
			if(bondRate < 0)
				bondRate = Tx::Core::Con_doubleInvalid;
			resTable.SetCell(10,j,bondRate);
			
			resTable.GetCell(11,j,bankRate);
			bankRate = bankRate*100/totalValue;
			if(bankRate < 0)
				bankRate = Tx::Core::Con_doubleInvalid;
			resTable.SetCell(12,j,bankRate);
 
			resTable.GetCell(13,j,elseRate);
			elseRate = elseRate*100/totalValue;
			if(elseRate < 0)
				elseRate = Tx::Core::Con_doubleInvalid;				
			resTable.SetCell(14,j,elseRate);

			resTable.GetCell(15,j,buyRate);
			buyRate = buyRate*100/totalValue;
			if(buyRate < 0)
				buyRate = Tx::Core::Con_doubleInvalid;				
			resTable.SetCell(16,j,buyRate);

			resTable.GetCell(17,j,reckoning);
			reckoning = reckoning*100/totalValue;
			if(reckoning < 0)
				reckoning = Tx::Core::Con_doubleInvalid;				
			resTable.SetCell(18,j,reckoning);
			//计算权证占总值比
			resTable.GetCell(19,j,warrantRate);
			warrantRate =  warrantRate/totalValue*100;  
			if(warrantRate <= 0)
				warrantRate = Tx::Core::Con_doubleInvalid;
			resTable.SetCell(20,j,warrantRate);
		}
		resTable.DeleteCol(21,3);
	}
	else
	{	
		//把报告年份和报告期转化成CString类型,所以增加一列
		resTable.InsertCol(5,Tx::Core::dtype_val_string);//str报告期	
		CString stryear,strdate;
		int tempdate,iyear ,itemp;
		int icount = resTable.GetRowCount();
		for(int j = 0;j < icount;j++)
		{			
			resTable.GetCell(4,j,tempdate);				
			itemp = tempdate%10000;
			iyear = tempdate/10000;
			stryear.Format(_T("%d"),iyear);
			switch(itemp)
			{
			case 331:
				strdate = stryear + _T("年") + _T("一季报");
				break;
			case 630:
				strdate = stryear + _T("年") + _T("二季报");
				break;
			case 930:
				strdate = stryear + _T("年") + _T("三季报");
				break;
			case 1231:
				strdate = stryear + _T("年") + _T("四季报");
				break;
			}
			resTable.SetCell(5,j,strdate);
		}		
	}
	resTable.DeleteCol(3);
	resTable.Arrange();
	MultiSortRule multisort;
	multisort.AddRule(3,false);
	multisort.AddRule(2,true);
	multisort.AddRule(5,false);	
	resTable.SortInMultiCol(multisort);
	resTable.Arrange();
	resTable.DeleteCol(3);
	/*if(iType2 > 17)
	{
		resTable.SetSortRange(0,resTable.GetRowCount()-2);
	}*/
	//添加进度条
	prw.SetPercent(pid,1.0);
	return true;
}


//行业分布统计 add by lijw 2008-03-17
bool TxFund::StatIndustryDistribution(
									  Tx::Core::Table_Indicator &resTable,
									  std::vector<int>	iSecurityId,
									  std::vector<int>	iDate,
									  int		iType
									  )
{
	//添加进度条
//	ProgressWnd* pwd=Tx::Core::ProgressWnd::GetInstance();
	ProgressWnd prw;
	UINT pid=prw.AddItem(1,_T("行业分布统计..."),0.0);
	prw.Show(1000);
	m_txTable.Clear();//这是引用别人的成员变量，
	//从T_ASSET_ALLOCATION_twoyear里取基金定期报告ID，基金ID，报告年份，报告期
	//默认的返回值状态
	bool result = false;
	//把交易实体ID转化为基金ID
	std::vector<int> iFundId;
	std::vector<int>::iterator iterV;
	int tempId;

	std::vector<int> tempTradeV;
	for(iterV = iSecurityId.begin();iterV != iSecurityId.end();++iterV)
	{
		GetSecurityNow(*iterV);
		if(m_pSecurity != NULL)
		{
			tempId = m_pSecurity->GetSecurity1Id();
			if(find(iFundId.begin(),iFundId.end(),tempId) == iFundId.end())
			{
				iFundId.push_back(tempId);
				tempTradeV.push_back(*iterV);
			}
		}
	}
	iSecurityId.clear();
	iSecurityId.assign(tempTradeV.begin(),tempTradeV.end());
	//清空数据
	m_txTable.Clear();
	//准备样本集参数列
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//基金ID
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//报告年份
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//报告期
	const int indicatorIndex = 3;
	//取回基金券id
	long iIndicator[indicatorIndex] = 
	{
		30901332,	//基金定期报告ID//这是T_ASSET_ALLOCATION表里
		30901140,	//资产净值
		30901131	//股票市值
	};
	UINT varCfg[3];			//参数配置
	int varCount=3;			//参数个数
	for (int i = 0; i < indicatorIndex; i++)
	{
		int tempIndicator = iIndicator[i];

		GetIndicatorDataNow(tempIndicator);
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
	UINT iColCount = m_txTable.GetColCount();
	UINT* nColArray = new UINT[iColCount];
	for(int i = 0; i < (int)iColCount; i++)
	{
		nColArray[i] = i;
	}
	result = m_pLogicalBusiness->GetData(m_txTable,true);
	CString strTable;
	CString strTableTmp;
#ifdef _DEBUG
	strTable=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	
	iSecurityId.clear();
	iSecurityId.assign(tempTradeV.begin(),tempTradeV.end());
	Tx::Core::Table_Indicator tempTable;
	tempTable.CopyColumnInfoFrom(m_txTable);
	//根据基金ID进行筛选
	m_txTable.EqualsAt(tempTable,nColArray,iColCount,0,iFundId);
	//为了进行年度和报告期的筛选，所以把报告年份和报告期和为一列。
	TransReportDateToNormal2(tempTable,1);
#ifdef _DEBUG
	strTable=tempTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	m_txTable.Clear();
	m_txTable.CopyColumnInfoFrom(tempTable);
	//进行年度和报告期的筛选
	tempTable.EqualsAt(m_txTable,nColArray,iColCount-1,1,iDate);
#ifdef _DEBUG
	strTable=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//取出基金定期报告ID，把它放到vector里，为和从视图T_STOCK_HOLDING_INDUSTRY_DISTRIBUTION_View取得数据进行连接作准备。
	std::vector<int> ReportV;
	for(int i = 0;i < (int)m_txTable.GetRowCount();i++)
	{
		int reportid ;
		m_txTable.GetCell(2,i,reportid);
		ReportV.push_back(reportid);
	}
	delete nColArray;
	nColArray = NULL;

	Tx::Core::Table_Indicator TxTable;
	//准备样本集参数列
	TxTable.AddParameterColumn(Tx::Core::dtype_int4);//基金ID
	TxTable.AddParameterColumn(Tx::Core::dtype_int4);//报告年份
	TxTable.AddParameterColumn(Tx::Core::dtype_int4);//报告期
	const int indicatorIndex3 = 26;
	long iIndicator3[indicatorIndex3];
	//取回基金券id
	//行业分布的指标
	iIndicator3[0] = 30901348;//基金定期报告ID
	for(int i=0;i<23;i++)
	{
		iIndicator3[i+1] = 30901191 + i;
	}
	iIndicator3[24] = 30901349;//重仓行业前三名市值(不含制造业)
	iIndicator3[25] = 30901350;//占基金投资股票市值
	
	UINT varCfg3[3];			//参数配置
	int varCount3=3;			//参数个数
	for (int i = 0; i < indicatorIndex3; i++)
	{
		int tempIndicator = iIndicator3[i];

		GetIndicatorDataNow(tempIndicator);
		if (m_pIndicatorData==NULL)
		{ 
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false; 
		}
		varCfg3[0]=0;
		varCfg3[1]=1;
		varCfg3[2]=2;
		result = m_pLogicalBusiness->SetIndicatorIntoTable(
			m_pIndicatorData,	//指标
			varCfg3,				//参数配置
			varCount3,			//参数个数
			TxTable	//计算需要的参数传输载体以及计算后结果的载体
			);
		if(result==false)
		{ 
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false;
		}

	}

	result = m_pLogicalBusiness->GetData(TxTable,true);
	if (TxTable.GetRowCount() == 0)
	{
		//添加进度条
		prw.SetPercent(pid,1.0);
		return false;
	}
#ifdef _DEBUG
	strTable = TxTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	UINT iColCount3 = TxTable.GetColCount();
	UINT* nColArray3 = new UINT[iColCount3];
	for(int i = 0; i < (int)iColCount3; i++)
	{
		nColArray3[i] = i;
	}
	resTable.CopyColumnInfoFrom(TxTable);
	TxTable.EqualsAt(resTable,nColArray3,iColCount3,3,ReportV);
	delete nColArray3;
	nColArray3 = NULL;
#ifdef _DEBUG
	strTable = resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//添加进度条
	prw.SetPercent(pid,0.3);
	//为增加基金的交易实体ID和名称，代码作准备
	resTable.InsertCol(0,Tx::Core::dtype_int4);//基金交易实体ID
	resTable.InsertCol(1,Tx::Core::dtype_val_string);//基金名称
	resTable.InsertCol(2,Tx::Core::dtype_val_string);//基金代码
	//为把m_txTable的内容放到resTable表里作准备。
	resTable.InsertCol(7,Tx::Core::dtype_decimal);//资产净值
	resTable.InsertCol(8,Tx::Core::dtype_decimal);//股票市值
	//增加占比例的列,共增加了23列
	int iCol = 10;
	for(int j = 0;j < 23;j++)
	{
		resTable.InsertCol(iCol,Tx::Core::dtype_decimal);
		iCol += 2; 
	}
	int position;
	double equity2,stockmoney,dEquity;
	for(int m = 0;m < (int)m_txTable.GetRowCount();m++)
	{
		int reId2;		
		m_txTable.GetCell(2,m,reId2);
		m_txTable.GetCell(3,m,equity2);//基金的资产净值
		m_txTable.GetCell(4,m,stockmoney);//基金持股市值
		std::vector<UINT> vecInstiID;
		resTable.Find(6,reId2,vecInstiID);
		std::vector<UINT>::iterator iteID;
		for(iteID = vecInstiID.begin();iteID != vecInstiID.end();++iteID)
		{
			position = *iteID;
			resTable.SetCell(7,position,equity2);
			resTable.SetCell(8,position,stockmoney);
			//计算各个占净值比
			dEquity = 0;
			iCol = 9;
			for(int j = 0;j < 23;j++,iCol++)
			{
				resTable.GetCell(iCol,position,dEquity);
				if(dEquity < 0)
				{
					dEquity = Tx::Core::Con_doubleInvalid;
					resTable.SetCell(++iCol,position,dEquity);
					continue;
				}
				dEquity = dEquity*100/equity2;
				resTable.SetCell(++iCol,position,dEquity);
			}
		}

	}
#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//增加基金的交易实体ID和名称，代码
	std::vector<int>::iterator iterId;
	for(iterId = iSecurityId.begin();iterId != iSecurityId.end();++iterId)
	{
		int fundId  ;
		CString strName,strCode;
		GetSecurityNow(*iterId);
		if(m_pSecurity == NULL)
			continue;
//#ifdef _SECURITYID_FOR_STAT_
		fundId = m_pSecurity->GetSecurity1Id();		
		strName = m_pSecurity->GetName();
		strCode = m_pSecurity->GetCode();
		std::vector<UINT> vecInstiID;
		resTable.Find(3,fundId,vecInstiID);
		std::vector<UINT>::iterator iteID;
		for(iteID = vecInstiID.begin();iteID != vecInstiID.end();++iteID)
		{
			resTable.SetCell(0,*iteID,*iterId);
			resTable.SetCell(1,*iteID,strName);
			resTable.SetCell(2,*iteID,strCode);
		}

	}
	int iType2 = iType;//保存原来的值.
	//这里增加样本分类选择
	if(iType >1)
	{
		//为了进行年度和报告期的筛选，所以把报告年份和报告期和为一列。
		TransReportDateToNormal2(resTable,4);
		int iCount = resTable.GetRowCount();//累加之前resTable里的行数。
		//为了进行分类统计。所以增加一列基金ID。
		resTable.AddCol(Tx::Core::dtype_int4);//基金风格
		resTable.AddCol(Tx::Core::dtype_int4);//基金管理公司ID
		resTable.AddCol(Tx::Core::dtype_int4);//基金托管银行ID
		int tempfundId;
//		int icount = resTable.GetRowCount();
		//添加进度条
		prw.SetPercent(pid,0.6);
		//这里增加样本分类选择
		m_txTable.Clear();
		tempTable.Clear();
		//准备样本集=第一参数列:F_FUND_ID,int型
		m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
		const int indicatorIndex1 = 3;
		//取回交易实体id
		long iIndicator1[indicatorIndex1] = 
		{
			//30001035,	//基金风格
			//modified by zhangxs 20091221---NewStyle
			30001232,	//基金风格New
			30001020,	//管理公司ID，
			30001021	//托管银行ID，
		};
		UINT varCfg1[1];			//参数配置
		int varCount1=1;			//参数个数
		for (int i = 0; i < indicatorIndex1; i++)
		{
			int tempIndicator = iIndicator1[i];
			GetIndicatorDataNow(tempIndicator);
			if (m_pIndicatorData==NULL)
			{
				//添加进度条
				prw.SetPercent(pid,1.0);
				return false; 
			}
			varCfg1[0]=0;
			result = m_pLogicalBusiness->SetIndicatorIntoTable(
				m_pIndicatorData,	//指标
				varCfg1,				//参数配置
				varCount1,			//参数个数
				m_txTable	//计算需要的参数传输载体以及计算后结果的载体
				);
			if(result==false)
			{
				//添加进度条
				prw.SetPercent(pid,1.0);
				return false;
			}

		}
		result = m_pLogicalBusiness->GetData(m_txTable,true);
		if(result==false)
			return false;
#ifdef _DEBUG
		strTableTmp = m_txTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTableTmp);
#endif
		UINT iCol2=m_txTable.GetColCount();
		//复制所有列信息
		tempTable.CopyColumnInfoFrom(m_txTable);
		if(m_txTable.GetRowCount()==0)
		{
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false;
		}
		UINT* nColArray2 = new UINT[iCol2];
		for(UINT i=0;i<iCol2;i++)
			nColArray2[i]=i;
//#ifdef _SECURITYID_FOR_STAT_
		m_txTable.EqualsAt(tempTable,nColArray2,iCol2,0,iSecurityId);
//#else
//		m_txTable.EqualsAt(tempTable,nColArray2,iCol2,0,iFundId);
//#endif
		delete nColArray2;
		nColArray2 = NULL;
		//把基金风格、公司ID、托管银行ID放到resTable
		std::vector<UINT> vecInstiID2;
		std::vector<UINT>::iterator iteID;
		int position;
		int iStyle,iCompanyId,ibankId;
		int icount = tempTable.GetRowCount();
		for(int j = 0;j < icount;j++)
		{
			tempTable.GetCell(0,j,tempfundId);
			tempTable.GetCell(1,j,iStyle);
			tempTable.GetCell(2,j,iCompanyId);
			tempTable.GetCell(3,j,ibankId);
			//把基金ID和交易实体ID对应起来。并且把数据放到表里。
			if(!(vecInstiID2.empty()))
				vecInstiID2.clear();
//#ifdef _SECURITYID_FOR_STAT_
			resTable.Find(0,tempfundId,vecInstiID2);
//#else
//			resTable.Find(3,tempfundId,vecInstiID2);
//#endif
			for(iteID = vecInstiID2.begin();iteID != vecInstiID2.end();++iteID)
			{
				position = *iteID;
				resTable.SetCell(56,position,iStyle);//基金风格
				resTable.SetCell(57,position,iCompanyId);//管理公司ID
				resTable.SetCell(58,position,ibankId);//托管银行ID
			}
		}		
		std::vector<int> ColVector;		//根据哪些列进行统计  
		if(!(iType & 1))
			iType += 1;//假设都选择选择了单只基金那一项分类方式。
		if(iType & 2)
			ColVector.push_back(57);
		if(iType & 4)
			ColVector.push_back(56);
		if(iType & 8)
			ColVector.push_back(58);
		std::vector<int> IntCol;			//需要相加的整型列
		//		IntCol.push_back(6);
		std::vector<int> DoubleCol;	//需要相加的double列
		DoubleCol.push_back(7);
		for(int j = 6;j < 53;j++)
		{
			DoubleCol.push_back(j);
			j++;
		}
		AddUpRow(resTable,iType,ColVector,IntCol,DoubleCol,iDate,4);
		std::vector<int>::size_type isize = iDate.size();
		int iCount1 = resTable.GetRowCount();//累加以后resTable里的行数。
		int iCount2,iCount3;
		iCount2 = iCount1 - iCount;//这是增加的行数 样本汇总和其他分类方式的行数。
		std::unordered_map<int,CString> CompanyMap;
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_INSTITUTIONID_TO_SHORTNAME,CompanyMap);
		std::unordered_map<int,CString> StyleMap;
		//modified by zhangxs 20091221---NewStyle
		//TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX,StyleMap);
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX_NEW,StyleMap);
		std::unordered_map<int,CString> BankMap;
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_BANK,BankMap);
		std::unordered_map<int,CString>::iterator iterMap;
		int id;
		CString Name;		
		if(iType < 17)//没有样本汇总的情况
		{
			for(int m = 0;m < iCount2;m++)
			{				
				resTable.GetCell(56,iCount+m,id);
				if(id == 0)
				{
					resTable.GetCell(57,iCount+m,id);
					if(id == 0)
					{
						resTable.GetCell(58,iCount+m,id);
						iterMap = BankMap.find(id);
						if(iterMap!= BankMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,iCount+m,Name);
						resTable.SetCell(2,iCount+m,(CString)(_T("托管银行")));
						continue;
					}
					else
					{
						iterMap = CompanyMap.find(id);
						if(iterMap!= CompanyMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,iCount+m,Name);
						resTable.SetCell(2,iCount+m,(CString)(_T("基金公司")));
						continue;
					}					
				}
				else
				{
					iterMap = StyleMap.find(id);
					if(iterMap!= StyleMap.end())
						Name = iterMap->second;
					else
						Name = Tx::Core::Con_strInvalid;
					resTable.SetCell(1,iCount+m,Name);
					resTable.SetCell(2,iCount+m,(CString)(_T("基金风格")));
				}
			}				
		}
		else//不但有单只基金和样本汇总，还有其他的分类方式
		{
			iCount3 = iCount2 - isize;//除了样本汇总以外的其他分类方式的行数
			if(iCount3 > 0)//表示除了样本汇总以外还有其他的分类的方式
			{
				//填充样本汇总的那些行
				for(int k = 0;k < (int)isize;k++)
				{
					resTable.SetCell(1,iCount+iCount3+k,(CString)(_T("样本汇总")));
					resTable.SetCell(2,iCount+iCount3+k,(CString)(_T("全部汇总")));
				}
				//填充除了样本汇总以外的其他分类方式的那些行。
				for(int i = 0;i < iCount3;i++)
				{
					resTable.GetCell(56,iCount+i,id);
					if(id == 0)
					{
						resTable.GetCell(57,iCount+i,id);
						if(id == 0)
						{
							resTable.GetCell(58,iCount+i,id);
							iterMap = BankMap.find(id);
							if(iterMap!= BankMap.end())
								Name = iterMap->second;
							else
								Name = Tx::Core::Con_strInvalid;
							resTable.SetCell(1,iCount+i,Name);
							resTable.SetCell(2,iCount+i,(CString)(_T("托管银行")));
							continue;
						}
						else
						{
							iterMap = CompanyMap.find(id);
							if(iterMap!= CompanyMap.end())
								Name = iterMap->second;
							else
								Name = Tx::Core::Con_strInvalid;
							resTable.SetCell(1,iCount+i,Name);
							resTable.SetCell(2,iCount+i,(CString)(_T("基金公司")));
							continue;
						}					
					}
					else
					{
						iterMap = StyleMap.find(id);
						if(iterMap!= StyleMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,iCount+i,Name);
						resTable.SetCell(2,iCount+i,(CString)(_T("基金风格")));
					}
				}
			}
			else//只有单只基金和汇总的情况
			{
				for(int k = 0;k < (int)isize;k++)
				{
					resTable.SetCell(1,iCount+k,(CString)(_T("样本汇总")));
					resTable.SetCell(2,iCount+k,(CString)(_T("全部汇总")));
				}		
			}
		}	
		//把报告年份和报告期转化成CString类型,所以增加一列
		resTable.InsertCol(5,Tx::Core::dtype_val_string);//str报告期	
		CString stryear,strdate;
		int tempdate,iyear ,itemp;
		int tempCount;
		if(!(iType2 & 1))
		{
			resTable.DeleteRow(0,iCount);
			tempCount = iCount2;			
		}
		else
		{
			tempCount = iCount1;			
		}
		for(int j = 0;j < tempCount;j++)
		{
			//填写报告期	
			resTable.GetCell(4,j,tempdate);	
			itemp = tempdate%10000;
			iyear = tempdate/10000;
			stryear.Format(_T("%d"),iyear);
			switch(itemp)
			{
			case 331:
				strdate = stryear + _T("年") + _T("一季报");
				break;
			case 630:
				strdate = stryear + _T("年") + _T("二季报");
				break;
			case 930:
				strdate = stryear + _T("年") + _T("三季报");
				break;
			case 1231:
				strdate = stryear + _T("年") + _T("四季报");
				break;
			}
			resTable.SetCell(5,j,strdate);
			//计算比例
			resTable.GetCell(7,j,equity2);//基金的资产净值
			dEquity = 0;
			iCol = 9;
			for(int k = 0;k < 23;k++,iCol++)
			{
				resTable.GetCell(iCol,j,dEquity);
				if(dEquity < 0)
					dEquity = Tx::Core::Con_doubleInvalid;
				else
					dEquity = dEquity*100/equity2;
				resTable.SetCell(++iCol,j,dEquity);
			}
		}
		resTable.DeleteCol(57,3);
		resTable.DeleteCol(6);
		resTable.DeleteCol(3);
		resTable.Arrange();
		MultiSortRule multisort;
		multisort.AddRule(3,false);
		multisort.AddRule(2,true);
		multisort.AddRule(5,false);	
		resTable.SortInMultiCol(multisort);
		resTable.Arrange();
		resTable.DeleteCol(3);
	}
	else
	{	
		//下面的方法和上面的方法的思路不一样。
		//把报告年份和报告期转化成CString类型,所以增加一列
		resTable.InsertCol(6,Tx::Core::dtype_val_string);//str报告期
		//把报告年份和报告期转化为CString类型
		TransReportDateToNormal3(resTable,4);//减少两列，分别是报告年份、报告期（int）
		resTable.Arrange();
		MultiSortRule multisort;
		multisort.AddRule(4,false);
		multisort.AddRule(2,true);
		multisort.AddRule(7,false);	
		resTable.SortInMultiCol(multisort);
		resTable.Arrange();
		resTable.DeleteCol(4);

		resTable.DeleteCol(5);//基金定期报告ID
		resTable.DeleteCol(3);//删除基金ID
#ifdef _DEBUG
		strTable=resTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif	
	}
	//由于行业集中度没有数据，所以暂时删掉
	int DelCol = resTable.GetColCount();
	resTable.DeleteCol(DelCol-2,2);
	/*if(iType2 > 17)
	{
		resTable.SetSortRange(0,resTable.GetRowCount()-2);
	}*/
	//添加进度条
	prw.SetPercent(pid,1.0);
	return true;
}
//add by lijw 2008-04-02
//债券组合结构统计T_BOND_HOLDING_BOND_TYPE_DISTRIBUTION
bool TxFund::StatBondDistribution(
								  Tx::Core::Table_Indicator &resTable,
								  std::vector<int>	iSecurityId,
								  std::vector<int>	iDate,
								  int		iType
								  )
{
	//添加进度条
//	ProgressWnd* pwd=Tx::Core::ProgressWnd::GetInstance();
	ProgressWnd prw;
	UINT pid=prw.AddItem(1,_T("债券组合结构统计..."),0.0);
	prw.Show(1000);
	//默认的返回值状态。
	bool result = true;	
	std::set<int> setFundId;
	std::set<int> DifferentID;
	std::vector<int>::iterator iter;
	for(iter = iSecurityId.begin();iter != iSecurityId.end();++iter)
	{
		GetSecurityNow(*iter);
		if(m_pSecurity == NULL)
			continue;
		int fundid =(int)m_pSecurity->GetSecurity1Id();
		if(setFundId.find(fundid) == setFundId.end())
		{
			setFundId.insert(fundid);
			DifferentID.insert(*iter);
		}
	}
	iSecurityId.clear();
	iSecurityId.assign(DifferentID.begin(),DifferentID.end());
	resTable.AddCol(Tx::Core::dtype_int4);//基金ID
	resTable.AddCol(Tx::Core::dtype_int4);//财年
	resTable.AddCol(Tx::Core::dtype_int4);//财季	
	//参数个数	
	int varCount=3;	
	const int INDICATOR_INDEX = 7;
	//取回的是券id
	long iIndicator[INDICATOR_INDEX]=
	{
		30901351,	//基金定期报告ID
		30901221,	//债券合计
		30901215,	//国家债券
		30901216,	//金融债
		30901217,	//央行票据
		30901218,	//企业债
		30901219	//可转债
	};
	//设定指标列
	for (int i = 0;i <	INDICATOR_INDEX;i++)
	{
		GetIndicatorDataNow(iIndicator[i]);
		if (m_pIndicatorData==NULL)
		{ 
			//添加进度条
			prw.SetPercent(pid,0.3);
			return false;
		}
		result = m_pLogicalBusiness->SetIndicatorIntoTable(
			m_pIndicatorData,	//指标
			varCfg,				//参数配置
			varCount,			//参数个数
			resTable			//计算需要的参数传输载体以及计算后结果的载体
			);
		if(result==false)
			return false;
	}
	result=m_pLogicalBusiness->GetData(resTable,true);
	if(result == false)
	{
		//添加进度条
		prw.SetPercent(pid,0.3);
		return false;
	}
	CString strTable;
	CString strTableTmp;
#ifdef _DEBUG
	strTable = resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif 
	UINT iColCount = resTable.GetColCount();
	UINT* nColArray = new UINT[iColCount];
	for(int i = 0; i < (int)iColCount; i++)
	{
		nColArray[i] = i;
	}
	Tx::Core::Table_Indicator tempTable;
	tempTable.CopyColumnInfoFrom(resTable);
	//根据基金ID进行筛选
	resTable.EqualsAt(tempTable,nColArray,iColCount,0,setFundId);
	if(tempTable.GetRowCount() == 0)
	{
		delete nColArray;
		nColArray = NULL;
		//添加进度条
		prw.SetPercent(pid,0.3);
		return false;
	}
	//为了进行年度和报告期的筛选，所以把报告年份和报告期和为一列。
	TransReportDateToNormal2(tempTable,1);
#ifdef _DEBUG
	strTable = tempTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	resTable.Clear();
	resTable.CopyColumnInfoFrom(tempTable);
	//进行年度和报告期的筛选
	tempTable.EqualsAt(resTable,nColArray,iColCount-1,1,iDate);
	if(resTable.GetRowCount() == 0)
	{
		delete nColArray;
		nColArray = NULL;
		//添加进度条
		prw.SetPercent(pid,0.3);
		return false;
	}
#ifdef _DEBUG
	strTable = resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//取出基金定期报告ID，把它放到vector里，为和从视图T_STOCK_HOLDING_INDUSTRY_DISTRIBUTION_View取得数据进行连接作准备。
	std::vector<int> ReportV;
	int iCount = resTable.GetRowCount();
	int reportid ;
	for(int i = 0;i < iCount;i++)
	{	
		resTable.GetCell(2,i,reportid);
		ReportV.push_back(reportid);
	}
	delete nColArray;
	nColArray = NULL;

	//从T_ASSET_ALLOCATION_twoyear里取基金定期报告ID，基金ID，报告年份，报告期
	//清空数据
	m_txTable.Clear();
	//准备样本集参数列
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//基金ID
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//报告年份
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//报告期
	const int indicatorIndex2 = 2;
	//取回的是券id
	long iIndicator2[indicatorIndex2] = 
	{
		30901332,	//基金定期报告ID//这是T_ASSET_ALLOCATION表里
		30901140	//资产净值
	};
	UINT varCfg2[3];			//参数配置
	int varCount2=3;			//参数个数
	for (int i = 0; i < indicatorIndex2; i++)
	{
		GetIndicatorDataNow(iIndicator2[i]);
		if (m_pIndicatorData==NULL)
		{ return false; }
		varCfg2[0]=0;
		varCfg2[1]=1;
		varCfg2[2]=2;
		result = m_pLogicalBusiness->SetIndicatorIntoTable(
			m_pIndicatorData,	//指标
			varCfg2,				//参数配置
			varCount2,			//参数个数
			m_txTable	//计算需要的参数传输载体以及计算后结果的载体
			);
		if(result==false)
		{
			//添加进度条
			prw.SetPercent(pid,0.3);
			return false;
		}
	}
	UINT iColCount2 = m_txTable.GetColCount();
	UINT* nColArray2 = new UINT[iColCount2];
	for(int i = 0; i < (int)iColCount2; i++)
	{
		nColArray2[i] = i;
	}
	result = m_pLogicalBusiness->GetData(m_txTable,true);
#ifdef _DEBUG
	strTableTmp = m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTableTmp);
#endif
	tempTable.Clear();
	tempTable.CopyColumnInfoFrom(m_txTable);
	//根据基金ID进行筛选
	m_txTable.EqualsAt(tempTable,nColArray2,iColCount2,3,ReportV);
	if(tempTable.GetRowCount() == 0)
	{
		delete nColArray2;
		nColArray2 = NULL;
		//添加进度条
		prw.SetPercent(pid,0.3);
		return false;
	}
	delete nColArray2;
	nColArray2 = NULL;
#ifdef _DEBUG
	strTableTmp = tempTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTableTmp);
#endif
	//添加进度条
	prw.SetPercent(pid,0.3);
	//为增加基金的交易实体ID和名称，代码作准备
	resTable.InsertCol(0,Tx::Core::dtype_int4);//基金交易实体ID
	resTable.InsertCol(1,Tx::Core::dtype_val_string);//基金名称
	resTable.InsertCol(2,Tx::Core::dtype_val_string);//基金代码
	//为把tempTable的内容放到resTable表里作准备。
	resTable.InsertCol(6,Tx::Core::dtype_decimal);//资产净值
	//增加占比例的列,共增加了6列
	int iCol = 8;
	for(int j = 0;j < 5;j++)
	{
		resTable.InsertCol(iCol,Tx::Core::dtype_decimal);
		iCol += 2; 
	}
	resTable.AddCol(Tx::Core::dtype_decimal);
	int position;
	double equity2,dEquity;
	std::vector<UINT> vecInstiID;
	std::vector<UINT>::iterator iteID;
	for(int m = 0;m < (int)tempTable.GetRowCount();m++)
	{		
		tempTable.GetCell(3,m,reportid);
		tempTable.GetCell(4,m,equity2);//基金的资产净值	
		if(!vecInstiID.empty())
			vecInstiID.clear();
		resTable.Find(5,reportid,vecInstiID);		
		for(iteID = vecInstiID.begin();iteID != vecInstiID.end();++iteID)
		{
			position = *iteID;
			resTable.SetCell(6,position,equity2);
			//计算各个占净值比
			dEquity = 0;
			iCol = 7;
			for(int j = 0;j < 6;j++,iCol++)
			{
				resTable.GetCell(iCol,position,dEquity);
				if(dEquity < 0)
				{
					dEquity = Tx::Core::Con_doubleInvalid;
					resTable.SetCell(++iCol,position,dEquity);
					continue;
				}
				dEquity = dEquity*100/equity2;
				resTable.SetCell(++iCol,position,dEquity);
			}
		}

	}
	//增加基金的交易实体ID和名称，代码
	std::vector<int>::iterator iterId;
	int fundId ;
	CString strName,strCode;
	for(iterId = iSecurityId.begin();iterId != iSecurityId.end();++iterId)
	{		
		GetSecurityNow(*iterId);
		if(m_pSecurity == NULL)
			continue;
		fundId = m_pSecurity->GetSecurity1Id();
		strName = m_pSecurity->GetName();
		strCode = m_pSecurity->GetCode();
		if(!vecInstiID.empty())
			vecInstiID.clear();
		resTable.Find(3,fundId,vecInstiID);
		for(iteID = vecInstiID.begin();iteID != vecInstiID.end();++iteID)
		{
			position = *iteID;
			resTable.SetCell(0,position,*iterId);
			resTable.SetCell(1,position,strName);
			resTable.SetCell(2,position,strCode);
		}
	}
//	//为计算方便，以基金ID进行排序
//	resTable.Sort(3);
//	if(SameFundId.size() != 0)
//	{
//		//为不同的交易实体ID转化为相同的基金ID那样的样本填写数据，		
//		std::set<int>::iterator iterSet,tempIter;
//		int position1,position2;
//		for(iterSet = SameFundId.begin();iterSet != SameFundId.end();++iterSet)
//		{
//			GetSecurityNow(*iterSet);
//			if(m_pSecurity == NULL)
//				continue;
//			fundId = m_pSecurity->GetSecurity1Id();//取得基金ID
//			strName = m_pSecurity->GetName();
//			strCode = m_pSecurity->GetCode();
//			if(!vecInstiID.empty())
//				vecInstiID.clear();
//			resTable.Find(3,fundId,vecInstiID);
//			if(vecInstiID.size() == 0)
//				continue;
//			//取得在表中最小的位置
//			std::set<int> tempset(vecInstiID.begin(),vecInstiID.end());
//			tempIter = tempset.begin();
//			//增加相同的记录
//			std::set<int>::size_type icount = tempset.size();
//			resTable.InsertRow(*tempIter,icount);
//			int ReportDate;			
//			double dData;
//			for(;tempIter != tempset.end();++tempIter)
//			{
//				position1 = *tempIter;
//				position2 = position1 + icount;
//				resTable.SetCell(0,position1,*iterSet);
//				resTable.SetCell(1,position1,strName);
//				resTable.SetCell(2,position1,strCode);
//				//填充其他的数据
//				resTable.GetCell(3,position2,fundId);
//				resTable.GetCell(4,position2,ReportDate);
//				resTable.SetCell(3,position1,fundId);
//				resTable.SetCell(4,position1,ReportDate);
//				iCol = 6;
//				for(int i = 0;i < 13;i++,iCol++)
//				{
//					resTable.GetCell(iCol,position2,dData);
//					resTable.SetCell(iCol,position1,dData);
//				}			
//			}
//			vecInstiID.clear();
//			tempset.clear();
//		}
//	}
//#ifdef _DEBUG
//	strTable=resTable.TableToString();
//	Tx::Core::Commonality::String().StringToClipboard(strTable);
//#endif
	//这里增加样本分类选择
	int iType2 = iType;//保存原来的值.
	if(iType >1)
	{
		int iCount = resTable.GetRowCount();//累加之前resTable里的行数。
		//为了进行分类统计。所以增加一列基金ID。
		resTable.AddCol(Tx::Core::dtype_int4);//基金风格
		resTable.AddCol(Tx::Core::dtype_int4);//基金管理公司ID
		resTable.AddCol(Tx::Core::dtype_int4);//基金托管银行ID
		int tempfundId;
//		int icount = resTable.GetRowCount();
		//添加进度条
		prw.SetPercent(pid,0.6);
		//这里增加样本分类选择
		m_txTable.Clear();
		tempTable.Clear();
		//准备样本集=第一参数列:F_FUND_ID,int型
		m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
		const int indicatorIndex1 = 3;
		//取回的是交易实体id
		long iIndicator1[indicatorIndex1] = 
		{
			//30001035,	//基金风格
			//modified by zhangxs 20091221---NewStyle
			30001232,	//基金风格New
			30001020,	//管理公司ID，
			30001021	//托管银行ID，
		};
		UINT varCfg1[1];			//参数配置
		int varCount1=1;			//参数个数
		for (int i = 0; i < indicatorIndex1; i++)
		{
			int tempIndicator = iIndicator1[i];
			GetIndicatorDataNow(tempIndicator);
			if (m_pIndicatorData==NULL)
				return false; 
			varCfg1[0]=0;
			result = m_pLogicalBusiness->SetIndicatorIntoTable(
				m_pIndicatorData,	//指标
				varCfg1,				//参数配置
				varCount1,			//参数个数
				m_txTable	//计算需要的参数传输载体以及计算后结果的载体
				);
			if(result==false)
			{
				return FALSE;
			}

		}
		result = m_pLogicalBusiness->GetData(m_txTable,true);
		if(result==false)
			return false;
#ifdef _DEBUG
		strTableTmp = m_txTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
		UINT iCol2=m_txTable.GetColCount();
		//复制所有列信息
		tempTable.CopyColumnInfoFrom(m_txTable);
		if(m_txTable.GetRowCount()==0)
		{
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false;
		}
		UINT* nColArray2 = new UINT[iCol2];
		for(UINT i=0;i<iCol2;i++)
			nColArray2[i]=i;
//#ifdef _SECURITYID_FOR_STAT_
		m_txTable.EqualsAt(tempTable,nColArray2,iCol2,0,iSecurityId);
//#else
//		m_txTable.EqualsAt(tempTable,nColArray2,iCol2,0,setFundId);
//#endif
		delete nColArray2;
		nColArray2 = NULL;
		//把基金风格、公司ID、托管银行ID放到resTable
		std::vector<UINT> vecInstiID2;
		std::vector<UINT>::iterator iteID;
		int position;
		int iStyle,iCompanyId,ibankId;
		int icount = tempTable.GetRowCount();
		for(int j = 0;j < icount;j++)
		{
			tempTable.GetCell(0,j,tempfundId);
			tempTable.GetCell(1,j,iStyle);
			tempTable.GetCell(2,j,iCompanyId);
			tempTable.GetCell(3,j,ibankId);
			//把基金ID和交易实体ID对应起来。并且把数据放到表里。
			if(!(vecInstiID2.empty()))
				vecInstiID2.clear();
//#ifdef _SECURITYID_FOR_STAT_
			resTable.Find(0,tempfundId,vecInstiID2);
//#else
//			resTable.Find(3,tempfundId,vecInstiID2);
//#endif
			for(iteID = vecInstiID2.begin();iteID != vecInstiID2.end();++iteID)
			{
				position = *iteID;
				resTable.SetCell(19,position,iStyle);//基金风格
				resTable.SetCell(20,position,iCompanyId);//管理公司ID
				resTable.SetCell(21,position,ibankId);//托管银行ID
			}
		}		
		std::vector<int> ColVector;		//根据哪些列进行统计  
		if(!(iType & 1))
			iType += 1;//假设都选择选择了单只基金那一项分类方式。
		if(iType & 2)
			ColVector.push_back(20);
		if(iType & 4)
			ColVector.push_back(19);
		if(iType & 8)
			ColVector.push_back(21);
		std::vector<int> IntCol;			//需要相加的整型列
		//		IntCol.push_back(6);
		std::vector<int> DoubleCol;	//需要相加的double列
		DoubleCol.push_back(6);
		for(int j = 7;j < 18;j++)
		{
			DoubleCol.push_back(j);
			j++;
		}
		AddUpRow(resTable,iType,ColVector,IntCol,DoubleCol,iDate,4);
		std::vector<int>::size_type isize = iDate.size();
		int iCount1 = resTable.GetRowCount();//累加以后resTable里的行数。
		int iCount2,iCount3;
		iCount2 = iCount1 - iCount;//这是增加的行数 样本汇总和其他分类方式的行数。
		std::unordered_map<int,CString> CompanyMap;
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_INSTITUTIONID_TO_SHORTNAME,CompanyMap);
		std::unordered_map<int,CString> StyleMap;
		//modified by zhangxs 20091221---NewStyle
		//TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX,StyleMap);
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX_NEW,StyleMap);
		std::unordered_map<int,CString> BankMap;
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_BANK,BankMap);
		std::unordered_map<int,CString>::iterator iterMap;
		int id;
		CString Name;		
		if(iType < 17)//没有样本汇总的情况
		{
			for(int m = 0;m < iCount2;m++)
			{				
				resTable.GetCell(19,iCount+m,id);
				if(id == 0)
				{
					resTable.GetCell(20,iCount+m,id);
					if(id == 0)
					{
						resTable.GetCell(21,iCount+m,id);
						iterMap = BankMap.find(id);
						if(iterMap!= BankMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,iCount+m,Name);
						resTable.SetCell(2,iCount+m,(CString)(_T("托管银行")));
						continue;
					}
					else
					{
						iterMap = CompanyMap.find(id);
						if(iterMap!= CompanyMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,iCount+m,Name);
						resTable.SetCell(2,iCount+m,(CString)(_T("基金公司")));
						continue;
					}					
				}
				else
				{
					iterMap = StyleMap.find(id);
					if(iterMap!= StyleMap.end())
						Name = iterMap->second;
					else
						Name = Tx::Core::Con_strInvalid;
					resTable.SetCell(1,iCount+m,Name);
					resTable.SetCell(2,iCount+m,(CString)(_T("基金风格")));
				}
			}				
		}
		else//不但有单只基金和样本汇总，还有其他的分类方式
		{
			iCount3 = iCount2 - isize;//除了样本汇总以外的其他分类方式的行数
			if(iCount3 > 0)//表示除了样本汇总以外还有其他的分类的方式
			{
				//填充样本汇总的那些行
				for(int k = 0;k < (int)isize;k++)
				{
					resTable.SetCell(1,iCount+iCount3+k,(CString)(_T("样本汇总")));
					resTable.SetCell(2,iCount+iCount3+k,(CString)(_T("全部汇总")));
				}
				//填充除了样本汇总以外的其他分类方式的那些行。
				for(int i = 0;i < iCount3;i++)
				{
					resTable.GetCell(19,iCount+i,id);
					if(id == 0)
					{
						resTable.GetCell(20,iCount+i,id);
						if(id == 0)
						{
							resTable.GetCell(21,iCount+i,id);
							iterMap = BankMap.find(id);
							if(iterMap!= BankMap.end())
								Name = iterMap->second;
							else
								Name = Tx::Core::Con_strInvalid;
							resTable.SetCell(1,iCount+i,Name);
							resTable.SetCell(2,iCount+i,(CString)(_T("托管银行")));
							continue;
						}
						else
						{
							iterMap = CompanyMap.find(id);
							if(iterMap!= CompanyMap.end())
								Name = iterMap->second;
							else
								Name = Tx::Core::Con_strInvalid;
							resTable.SetCell(1,iCount+i,Name);
							resTable.SetCell(2,iCount+i,(CString)(_T("基金公司")));
							continue;
						}					
					}
					else
					{
						iterMap = StyleMap.find(id);
						if(iterMap!= StyleMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,iCount+i,Name);
						resTable.SetCell(2,iCount+i,(CString)(_T("基金风格")));
					}
				}
			}
			else//只有单只基金和汇总的情况
			{
				for(int k = 0;k < (int)isize;k++)
				{
					resTable.SetCell(1,iCount+k,(CString)(_T("样本汇总")));
					resTable.SetCell(2,iCount+k,(CString)(_T("全部汇总")));
				}		
			}
		}	
		int tempCount;
		if(!(iType2 & 1))
		{
			resTable.DeleteRow(0,iCount);
			tempCount = iCount2;			
		}
		else
		{
			tempCount = iCount1;			
		}
		for(int j = 0;j < tempCount;j++)
		{			
			//计算比例
			resTable.GetCell(6,j,equity2);//基金的资产净值
			dEquity = 0;
			iCol = 7;
			for(int k = 0;k < 6;k++,iCol++)
			{
				resTable.GetCell(iCol,j,dEquity);
				if(dEquity < 0)
					dEquity = Tx::Core::Con_doubleInvalid;
				else
					dEquity = dEquity*100/equity2;
				resTable.SetCell(++iCol,j,dEquity);
			}
		}
		resTable.DeleteCol(19,3);		
	}	
	//把报告年份和报告期转化成CString类型,所以增加一列
	resTable.InsertCol(5,Tx::Core::dtype_val_string);//str报告期	
	CString stryear,strdate;
	int tempdate,iyear ,itemp;
	iCount = resTable.GetRowCount();
	for(int j = 0;j < iCount;j++)
	{
		//填写报告期	
		resTable.GetCell(4,j,tempdate);	
		itemp = tempdate%10000;
		iyear = tempdate/10000;
		stryear.Format(_T("%d"),iyear);
		switch(itemp)
		{
		case 331:
			strdate = stryear + _T("年") + _T("一季报");
			break;
		case 630:
			strdate = stryear + _T("年") + _T("二季报");
			break;
		case 930:
			strdate = stryear + _T("年") + _T("三季报");
			break;
		case 1231:
			strdate = stryear + _T("年") + _T("四季报");
			break;
		}
		resTable.SetCell(5,j,strdate);
	}	
	resTable.DeleteCol(6);
	resTable.DeleteCol(3);
	resTable.Arrange();
	MultiSortRule multisort;
	multisort.AddRule(3,false);
	multisort.AddRule(2,true);
	multisort.AddRule(5,false);	
	resTable.SortInMultiCol(multisort);	
	resTable.Arrange();
	resTable.DeleteCol(3);
	/*if(iType2 > 17)
	{
		resTable.SetSortRange(0,resTable.GetRowCount()-2);
	}*/
	//添加进度条
	prw.SetPercent(pid,1.0);
	return true;
}

// 起始终止日期与报告期的转换 by 20101111 wanglm 20100430
int TxFund::RegportDateToReportID( std::vector<int> vecStartDate, std::vector<int> vecDate )
{
	int iRet = 0;
	std::vector<int> iReportID; //40040001 40040002 40040003 40040004 40040006 40040009
	for ( int i=0; i < (int)vecDate.size(); i++ )
	{
		int iStartDate = vecStartDate[i];
		int iEndDate = vecDate[i];

		iStartDate = iStartDate % 10000;
		iEndDate = iEndDate % 10000;

		//if ( 101 == iStartDate && 331 == iEndDate )
		//{
		//	iReportID.push_back(40040001);  // 一季报
		//}
		//else if ( 401 == iStartDate && 630 == iEndDate )
		//{
		//	iReportID.push_back(40040002);  // 二季报
		//}
		//else if ( 701 == iStartDate && 930 == iEndDate )
		//{
		//	iReportID.push_back(40040004);  // 三季报 
		//}
		//else if ( 1001 == iStartDate && 1231 == iEndDate )
		//{
		//	iReportID.push_back(40040006);  // 四季报
		//}

		if ( 101 == iStartDate && 630 == iEndDate )
		{
			iReportID.push_back(40040003);  // 中报
			iRet = 3;
		}
		else if ( 101 == iStartDate && 1231 == iEndDate )
		{
			iReportID.push_back(40040009);  // 年报
			iRet = 9;
		}
	}

	return iRet;
}

// add by lijw 2008-03-11
//天相行业分布统计
bool TxFund::StatIndustryDistributionTx(
		Tx::Core::Table_Indicator &resTable,
		std::vector<int>	iSecurityId,
		std::vector<int>    iStartData,
		std::vector<int>	iDate,
		int		iType
		)
{
	//添加进度条
//	ProgressWnd* pwd=Tx::Core::ProgressWnd::GetInstance();
	ProgressWnd prw;
	UINT pid=prw.AddItem(1,_T("天相行业分布统计..."),0.0);
	prw.Show(1000);
	m_txTable.Clear();//这是引用别人的成员变量，
	//从T_ASSET_ALLOCATION_twoyear里取基金定期报告ID，基金ID，报告年份，报告期
	//默认的返回值状态
	bool result = false;
	std::vector<int> iFundId;
	//	if(!TransObjectToSecIns(iSecurityId,iFundId,1))
	//		return false;	
	std::vector<int>::iterator iter;
	int tempFundId;
	std::vector<int> tempTradeV;
	iter = iSecurityId.begin();	
	for (;iter != iSecurityId.end();++iter)
	{
		GetSecurityNow(*iter);
		if (m_pSecurity != NULL)
		{
			tempFundId = m_pSecurity->GetSecurity1Id();
			if (find(iFundId.begin(),iFundId.end(),tempFundId) == iFundId.end())
			{
				iFundId.push_back(tempFundId);
				tempTradeV.push_back(*iter);
			}
		}
	}
	iSecurityId.clear();
	iSecurityId.assign(tempTradeV.begin(),tempTradeV.end());
	//清空数据
	m_txTable.Clear();
	//准备样本集参数列
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//基金ID
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//报告年份
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//报告期
	const int indicatorIndex = 3;
	int iRet = RegportDateToReportID( iStartData, iDate );
	//取回的是基金券id
	long iIndicator[indicatorIndex]=
	{
		30901332,	// 基金定期报告ID//这是T_ASSET_ALLOCATION表里
		30901140,	// 资产净值
		30901131  	// 股票市值    // Src 原始表碎文件ID “股票市值”iIndicator[2] = 30901130;
	};

	UINT varCfg[3];			//参数配置
	int varCount=3;			//参数个数
	for (int i = 0; i < indicatorIndex; i++)
	{
		int	tempIndicator = iIndicator[i];

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
			return FALSE;
		}

	}
	UINT iColCount = m_txTable.GetColCount();
	UINT* nColArray = new UINT[iColCount];
	for(int i = 0; i < (int)iColCount; i++)
	{
		nColArray[i] = i;
	}
	result = m_pLogicalBusiness->GetData(m_txTable,true);
	if (m_txTable.GetRowCount() == 0)
	{
		delete nColArray;
		nColArray = NULL;
		//添加进度条
		prw.SetPercent(pid,1.0);
		return false;
	}
#ifdef _DEBUG
	CString strTable=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//把交易实体ID转化为基金ID
	
	iSecurityId.clear();
	iSecurityId.assign(tempTradeV.begin(),tempTradeV.end());
	Tx::Core::Table_Indicator tempTable;
	tempTable.CopyColumnInfoFrom(m_txTable);
	//根据基金ID进行筛选
	m_txTable.EqualsAt(tempTable,nColArray,iColCount,0,iFundId);
	if (tempTable.GetRowCount() == 0)
	{
		delete nColArray;
		nColArray = NULL;
		//添加进度条
		prw.SetPercent(pid,1.0);
		delete nColArray;
		return false;
	}
	//为了进行年度和报告期的筛选，所以把报告年份和报告期和为一列。
	TransReportDateToNormal2(tempTable,1);         // wanglm 报告年份和报告期合为一列 -- 先去掉
#ifdef _DEBUG
	strTable=tempTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	m_txTable.Clear();
	m_txTable.CopyColumnInfoFrom(tempTable);
	//进行年度和报告期的筛选
	tempTable.EqualsAt(m_txTable,nColArray,iColCount-1,1,iDate); // lReportID
#ifdef _DEBUG
	strTable=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//取出基金定期报告ID，把它放到vector里，为和从视图T_STOCK_HOLDING_INDUSTRY_DISTRIBUTION_TX_View取得数据进行连接作准备。
	std::vector<int> ReportV;
	int itxCount = m_txTable.GetRowCount();   // (int)m_txTable.GetRowCount()
	         
	for(int i = 0;i < itxCount; i++)
	{
		int reportid ;
		m_txTable.GetCell(2,i,reportid);
		ReportV.push_back(reportid);
	}
	delete nColArray;
	nColArray = NULL;
//==========================================================================
	if ( 3 == iRet || 9 == iRet )
	{   // 当为3和9时，是中报和年报
		//从T_STOCK_HOLDING_TOP_TEN_twoyear取其他的数据
		Tx::Core::Table_Indicator hbTable;
		//准备样本集参数列
		hbTable.AddParameterColumn(Tx::Core::dtype_int4);//基金ID
		hbTable.AddParameterColumn(Tx::Core::dtype_int4);//报告年份
		hbTable.AddParameterColumn(Tx::Core::dtype_int4);//报告期
		hbTable.AddParameterColumn(Tx::Core::dtype_byte);//持股类型
		hbTable.AddParameterColumn(Tx::Core::dtype_int4);//序号
		const int indicatorIndex33 = 6;
		//取回的是基金券id
		long iIndicator33[indicatorIndex33] = 
		{
			30901320,	//市值排名
			30901128,	//股票ID
			30901130,	//股票市值
			30901129,	//持股数量
			30901321,	//所占比例（即是占净值比例）
			30901322	//占流通股比例（%）
		};
		UINT varCfg33[5];			//参数配置
		int varCount33=5;			//参数个数
		for (int i = 0; i < indicatorIndex33; i++)
		{
			int tempIndicator = iIndicator33[i];

			GetIndicatorDataNow(tempIndicator);
			if (m_pIndicatorData==NULL)
			{ return false; }
			varCfg33[0]=0;
			varCfg33[1]=1;
			varCfg33[2]=2;
			varCfg33[3]=3;
			varCfg33[4]=4;
			if(m_pLogicalBusiness ==NULL)
				continue;
			result = m_pLogicalBusiness->SetIndicatorIntoTable(
				m_pIndicatorData,	//指标
				varCfg33,				//参数配置
				varCount33,			//参数个数
				hbTable	//计算需要的参数传输载体以及计算后结果的载体
				);
			if(result==false)
			{
				//添加进度条
				prw.SetPercent(pid,1.0);
				return false;
			}
		}
		if(m_pLogicalBusiness ==NULL)
			return FALSE;
		result = m_pLogicalBusiness->GetData(hbTable,true);
		if(hbTable.GetRowCount() == 0)
		{
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false;
		}
		UINT iColCount4 = hbTable.GetColCount();
		UINT* nColArray4 = new UINT[iColCount4];
		for(int i = 0; i < (int)iColCount4; i++)
		{
			nColArray4[i] = i;
		}
		tempTable.Clear();
		tempTable.CopyColumnInfoFrom(hbTable);
		std::vector<byte> TypeVector;
		TypeVector.push_back(3);
		hbTable.EqualsAt(tempTable,nColArray4,iColCount4,3,TypeVector);
		delete nColArray4;
		nColArray4 = NULL;
		hbTable.Clear();
		hbTable.Clone(tempTable);//之所以这样写是为了不修改下面的代码
		hbTable.DeleteCol(3,2);//删除持股类型和序号列
#ifdef _DEBUG
		strTable=hbTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
#ifdef _DEBUG
		strTable=m_txTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
		double bCGSZ = 0.00;
	    int    iID_h = 0;
		int iHCount  = hbTable.GetRowCount();
		int iTXCount = m_txTable.GetRowCount();

		for( int i = 0; i < iTXCount; i++ )
		{
			int iID_tx;
			m_txTable.GetCell( 0, i, iID_tx );

			for ( int j = 0; j < iHCount; j++ )     
			{
				hbTable.GetCell( 0, j, iID_h );         // id

				if ( iID_h == iID_tx )  
				{
					hbTable.GetCell( 5, j, bCGSZ );     // 取股票市值	 
					m_txTable.SetCell( 4, i, bCGSZ );   // 写入股票市值
					break;
				}
				else
				{
					//AfxMessageBox( _T("没有找到相匹配的ID") );
				}
			}
		}
#ifdef _DEBUG
		strTable=m_txTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	}
//==================================================================================

	Tx::Core::Table_Indicator TxTable;
	//准备样本集参数列
	TxTable.AddParameterColumn(Tx::Core::dtype_int4);//基金ID
	TxTable.AddParameterColumn(Tx::Core::dtype_int4);//报告年份
	TxTable.AddParameterColumn(Tx::Core::dtype_int4);//报告期
	const int indicatorIndex3 = 41;//41个指标
	long iIndicator3[indicatorIndex3];
	//天相行业的指标
	//取回的是基金券id
	iIndicator3[0] = 30901340;//基金定期报告ID
	for(int i=0;i<15;i++)
	{
		iIndicator3[i+1] = 30901265 + i;
	}
	iIndicator3[16] = 30901284;
	for(int i=0;i<4;i++)
	{
		iIndicator3[i+17] = 30901280 + i;
	}
	for(int i=0;i<11;i++)
	{
		iIndicator3[i+21] = 30901285 + i;
	}
	for(int i=0;i<7;i++)
	{
		iIndicator3[i+32] = 30901297 + i;
	}
	iIndicator3[39] = 30901344;//行业前三名市值
    iIndicator3[40] = 30901345;//占股票投资比例指标
	UINT varCfg3[3];			//参数配置
	int varCount3=3;			//参数个数
	for (int i = 0; i < indicatorIndex3; i++)
	{
		int tempIndicator = iIndicator3[i];

		GetIndicatorDataNow(tempIndicator);
		if (m_pIndicatorData==NULL)
		{ return false; }
		varCfg3[0]=0;
		varCfg3[1]=1;
		varCfg3[2]=2;
		result = m_pLogicalBusiness->SetIndicatorIntoTable(
			m_pIndicatorData,	//指标
			varCfg3,				//参数配置
			varCount3,			//参数个数
			TxTable	//计算需要的参数传输载体以及计算后结果的载体
			);
		if(result==false)
		{ 
			return false;
		}

	}

	result = m_pLogicalBusiness->GetData(TxTable,true);
#ifdef _DEBUG
	strTable=TxTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	UINT iColCount3 = TxTable.GetColCount();
	UINT* nColArray3 = new UINT[iColCount3];
	for(int i = 0; i < (int)iColCount3; i++)
	{
		nColArray3[i] = i;
	}
	resTable.CopyColumnInfoFrom(TxTable);
	if ( (int)ReportV.size() <= 0 )
	{
		delete nColArray3;
		nColArray3 = NULL;
		return false;
	}
	TxTable.EqualsAt(resTable,nColArray3,iColCount3,3,ReportV);
	delete nColArray3;
	nColArray3 = NULL;
	if (resTable.GetRowCount() == 0)
	{
		//添加进度条
		prw.SetPercent(pid,1.0);
		return false;
	}
#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//添加进度条
	prw.SetPercent(pid,0.3);
	//为增加基金的交易实体ID和名称，代码作准备
	resTable.InsertCol(0,Tx::Core::dtype_int4);//基金交易实体ID
	resTable.InsertCol(1,Tx::Core::dtype_val_string);//基金名称
	resTable.InsertCol(2,Tx::Core::dtype_val_string);//基金代码
	//为把m_txTable的内容放到resTable表里作准备。
	resTable.InsertCol(7,Tx::Core::dtype_decimal);//资产净值
	resTable.InsertCol(8,Tx::Core::dtype_decimal);//股票市值
	//增加占比例的列,共增加了38列，
	int iCol = 10;
	for(int j = 0;j < 38;j++)
	{
		resTable.InsertCol(iCol,Tx::Core::dtype_decimal);
		iCol += 2; 
	}
//	resTable.AddCol(Tx::Core::dtype_decimal);
	int position;
	double equity2,stockmoney,dEquity;
	for(int m = 0;m < (int)m_txTable.GetRowCount();m++)
	{
		int reId2;		
		m_txTable.GetCell(2,m,reId2);
		m_txTable.GetCell(3,m,equity2);//基金的资产净值
		m_txTable.GetCell(4,m,stockmoney);//基金持股市值
		std::vector<UINT> vecInstiID;
		resTable.Find(6,reId2,vecInstiID);
		std::vector<UINT>::iterator iteID;
		for(iteID = vecInstiID.begin();iteID != vecInstiID.end();++iteID)
		{
			position = *iteID;
			resTable.SetCell(7,position,equity2);
			resTable.SetCell(8,position,stockmoney);
			//计算各个占净值比
			dEquity = 0;
			iCol = 9;
			for(int j = 0;j < 38;j++,iCol++)
			{
				resTable.GetCell(iCol,position,dEquity);
				if(dEquity < 0)
				{
					dEquity = Tx::Core::Con_doubleInvalid;
					resTable.SetCell(++iCol,position,dEquity);
					continue;
				}
				dEquity = dEquity*100/equity2;
				resTable.SetCell(++iCol,position,dEquity);
			}
		}

	}
#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//增加基金的交易实体ID和名称，代码
	//std::set<int> fundIdSet;
	std::vector<int>::iterator iterId;
	for(iterId = iSecurityId.begin();iterId != iSecurityId.end();++iterId)
	{
		int fundId  ;
		CString strName,strCode;
		GetSecurityNow(*iterId);
		if(m_pSecurity == NULL)
			continue;
//#ifdef _SECURITYID_FOR_STAT_
		fundId = m_pSecurity->GetSecurity1Id();		
		strName = m_pSecurity->GetName();
		strCode = m_pSecurity->GetCode();
		std::vector<UINT> vecInstiID;
		resTable.Find(3,fundId,vecInstiID);
		std::vector<UINT>::iterator iteID;
		for(iteID = vecInstiID.begin();iteID != vecInstiID.end();++iteID)
		{
			resTable.SetCell(0,*iteID,*iterId);
			resTable.SetCell(1,*iteID,strName);
			resTable.SetCell(2,*iteID,strCode);
		}
	}	
	int iType2 = iType;//保存原来的值.
	//这里增加样本分类选择
	if(iType >1)
	{
		//为了进行年度和报告期的筛选，所以把报告年份和报告期和为一列。
		TransReportDateToNormal2(resTable,4);
#ifdef _DEBUG
		strTable=resTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
		int iCount = resTable.GetRowCount();//累加之前resTable里的行数。
		//为了进行分类统计。所以增加一列基金ID。
		resTable.AddCol(Tx::Core::dtype_int4);//基金风格
		resTable.AddCol(Tx::Core::dtype_int4);//基金管理公司ID
		resTable.AddCol(Tx::Core::dtype_int4);//基金托管银行ID
		int tempfundId;
//		int icount = resTable.GetRowCount();
		//添加进度条
		prw.SetPercent(pid,0.6);
		//这里增加样本分类选择
		m_txTable.Clear();
		tempTable.Clear();
		//准备样本集=第一参数列:F_FUND_ID,int型
		m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
		const int indicatorIndex1 = 3;
		//取回的是基金交易实体id
		long iIndicator1[indicatorIndex1] = 
		{
			//30001035,	//基金风格
			//modified by zhangxs 20091221---NewStyle
			30001232,	//基金风格New
			30001020,	//管理公司ID，
			30001021	//托管银行ID，
		};
		UINT varCfg1[1];			//参数配置
		int varCount1=1;			//参数个数
		for (int i = 0; i < indicatorIndex1; i++)
		{
			int tempIndicator = iIndicator1[i];
			GetIndicatorDataNow(tempIndicator);
			if (m_pIndicatorData==NULL)
				return false; 
			varCfg1[0]=0;
			result = m_pLogicalBusiness->SetIndicatorIntoTable(
				m_pIndicatorData,	//指标
				varCfg1,				//参数配置
				varCount1,			//参数个数
				m_txTable	//计算需要的参数传输载体以及计算后结果的载体
				);
			if(result==false)
			{
				return FALSE;
			}

		}
		result = m_pLogicalBusiness->GetData(m_txTable,true);
		if(result==false)
			return false;
#ifdef _DEBUG
		strTable=m_txTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
		UINT iCol2=m_txTable.GetColCount();
		//复制所有列信息
		tempTable.CopyColumnInfoFrom(m_txTable);
		if(m_txTable.GetRowCount()==0)
		{
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false;
		}
		UINT* nColArray2 = new UINT[iCol2];
		for(UINT i=0;i<iCol2;i++)
			nColArray2[i]=i;
//#ifdef _SECURITYID_FOR_STAT_
		m_txTable.EqualsAt(tempTable,nColArray2,iCol2,0,iSecurityId);
//#else
//		m_txTable.EqualsAt(tempTable,nColArray2,iCol2,0,iFundId);
//#endif
		delete nColArray2;
		nColArray2 = NULL;
		//把基金风格、公司ID、托管银行ID放到resTable
		std::vector<UINT> vecInstiID2;
		std::vector<UINT>::iterator iteID;
		int position;
		int iStyle,iCompanyId,ibankId;
		int icount = tempTable.GetRowCount();
		for(int j = 0;j < icount;j++)
		{
			tempTable.GetCell(0,j,tempfundId);
			tempTable.GetCell(1,j,iStyle);
			tempTable.GetCell(2,j,iCompanyId);
			tempTable.GetCell(3,j,ibankId);
			//把基金ID和交易实体ID对应起来。并且把数据放到表里。
			if(!(vecInstiID2.empty()))
				vecInstiID2.clear();
//#ifdef _SECURITYID_FOR_STAT_
			resTable.Find(0,tempfundId,vecInstiID2);
//#else
//			resTable.Find(3,tempfundId,vecInstiID2);
//#endif
			for(iteID = vecInstiID2.begin();iteID != vecInstiID2.end();++iteID)
			{
				position = *iteID;
				resTable.SetCell(86,position,iStyle);//基金风格
				resTable.SetCell(87,position,iCompanyId);//管理公司ID
				resTable.SetCell(88,position,ibankId);//托管银行ID
			}
		}
#ifdef _DEBUG
		strTable=resTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif		
		std::vector<int> ColVector;		//根据哪些列进行统计  
		if(!(iType & 1))
			iType += 1;//假设都选择选择了单只基金那一项分类方式。
		if(iType & 2)
			ColVector.push_back(87);
		if(iType & 4)
			ColVector.push_back(86);
		if(iType & 8)
			ColVector.push_back(88);
		std::vector<int> IntCol;			//需要相加的整型列
		//		IntCol.push_back(6);
		std::vector<int> DoubleCol;	//需要相加的double列
		DoubleCol.push_back(7);
		for(int j = 6;j < 84;j++)
		{
			DoubleCol.push_back(j);
			j++;
		}
		AddUpRow(resTable,iType,ColVector,IntCol,DoubleCol,iDate,4);
#ifdef _DEBUG
		strTable=resTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
		std::vector<int>::size_type isize = iDate.size();
		int iCount1 = resTable.GetRowCount();//累加以后resTable里的行数。
		int iCount2,iCount3;
		iCount2 = iCount1 - iCount;//这是增加的行数 样本汇总和其他分类方式的行数。
		std::unordered_map<int,CString> CompanyMap;
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_INSTITUTIONID_TO_SHORTNAME,CompanyMap);
		std::unordered_map<int,CString> StyleMap;
		//modified by zhangxs 20091221---NewStyle
		//TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX,StyleMap);
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX_NEW,StyleMap);
		std::unordered_map<int,CString> BankMap;
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_BANK,BankMap);
		std::unordered_map<int,CString>::iterator iterMap;
		int id;
		CString Name;		
		if(iType < 17)//没有样本汇总的情况
		{
			for(int m = 0;m < iCount2;m++)
			{				
				resTable.GetCell(86,iCount+m,id);
				if(id == 0)
				{
					resTable.GetCell(87,iCount+m,id);
					if(id == 0)
					{
						resTable.GetCell(88,iCount+m,id);
						iterMap = BankMap.find(id);
						if(iterMap!= BankMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,iCount+m,Name);
						resTable.SetCell(2,iCount+m,(CString)(_T("托管银行")));
						continue;
					}
					else
					{
						iterMap = CompanyMap.find(id);
						if(iterMap!= CompanyMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,iCount+m,Name);
						resTable.SetCell(2,iCount+m,(CString)(_T("基金公司")));
						continue;
					}					
				}
				else
				{
					iterMap = StyleMap.find(id);
					if(iterMap!= StyleMap.end())
						Name = iterMap->second;
					else
						Name = Tx::Core::Con_strInvalid;
					resTable.SetCell(1,iCount+m,Name);
					resTable.SetCell(2,iCount+m,(CString)(_T("基金风格")));
				}
			}				
		}
		else//不但有单只基金和样本汇总，还有其他的分类方式
		{
			iCount3 = iCount2 - isize;//除了样本汇总以外的其他分类方式的行数
			if(iCount3 > 0)//表示除了样本汇总以外还有其他的分类的方式
			{
				//填充样本汇总的那些行
				for(int k = 0;k < (int)isize;k++)
				{
					resTable.SetCell(1,iCount+iCount3+k,(CString)(_T("样本汇总")));
					resTable.SetCell(2,iCount+iCount3+k,(CString)(_T("全部汇总")));
				}
				//填充除了样本汇总以外的其他分类方式的那些行。
				for(int i = 0;i < iCount3;i++)
				{
					resTable.GetCell(86,iCount+i,id);
					if(id == 0)
					{
						resTable.GetCell(87,iCount+i,id);
						if(id == 0)
						{
							resTable.GetCell(88,iCount+i,id);
							iterMap = BankMap.find(id);
							if(iterMap!= BankMap.end())
								Name = iterMap->second;
							else
								Name = Tx::Core::Con_strInvalid;
							resTable.SetCell(1,iCount+i,Name);
							resTable.SetCell(2,iCount+i,(CString)(_T("托管银行")));
							continue;
						}
						else
						{
							iterMap = CompanyMap.find(id);
							if(iterMap!= CompanyMap.end())
								Name = iterMap->second;
							else
								Name = Tx::Core::Con_strInvalid;
							resTable.SetCell(1,iCount+i,Name);
							resTable.SetCell(2,iCount+i,(CString)(_T("基金公司")));
							continue;
						}					
					}
					else
					{
						iterMap = StyleMap.find(id);
						if(iterMap!= StyleMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,iCount+i,Name);
						resTable.SetCell(2,iCount+i,(CString)(_T("基金风格")));
					}
				}
			}
			else//只有单只基金和汇总的情况
			{
				for(int k = 0;k < (int)isize;k++)
				{
					resTable.SetCell(1,iCount+k,(CString)(_T("样本汇总")));
					resTable.SetCell(2,iCount+k,(CString)(_T("全部汇总")));
				}		
			}
		}	
		//把报告年份和报告期转化成CString类型,所以增加一列
		resTable.InsertCol(5,Tx::Core::dtype_val_string);//str报告期	
		CString stryear,strdate;
		int tempdate,iyear ,itemp;
		int tempCount;
		if(!(iType2 & 1))
		{
			resTable.DeleteRow(0,iCount);
			tempCount = iCount2;			
		}
		else
		{
			tempCount = iCount1;			
		}
		for(int j = 0;j < tempCount;j++)
		{
			//填写报告期	
			resTable.GetCell(4,j,tempdate);	
			itemp = tempdate%10000;
			iyear = tempdate/10000;
			stryear.Format(_T("%d"),iyear);
			switch(itemp)
			{
			case 331:
				strdate = stryear + _T("年") + _T("一季报");
				break;
			case 630:
				strdate = stryear + _T("年") + _T("二季报");
				break;
			case 930:
				strdate = stryear + _T("年") + _T("三季报");
				break;
			case 1231:
				strdate = stryear + _T("年") + _T("四季报");
				break;
			}
			resTable.SetCell(5,j,strdate);
			//计算比例
			resTable.GetCell(7,j,equity2);//基金的资产净值
			dEquity = 0;
			iCol = 9;
			for(int k = 0;k < 38;k++,iCol++)
			{
				resTable.GetCell(iCol,j,dEquity);
				if(dEquity < 0)
					dEquity = Tx::Core::Con_doubleInvalid;
				else
					dEquity = dEquity*100/equity2;
				resTable.SetCell(++iCol,j,dEquity);
			}
		}
		resTable.DeleteCol(87,3);
		resTable.DeleteCol(6);
		resTable.DeleteCol(3);
		resTable.Arrange();
		MultiSortRule multisort;
		multisort.AddRule(3,false);
		multisort.AddRule(2,true);
		multisort.AddRule(5,false);	
		resTable.SortInMultiCol(multisort);
		resTable.Arrange();
		resTable.DeleteCol(3);
	}
	else
	{	
		//下面的方法和上面的方法的思路不一样。
		//把报告年份和报告期转化成CString类型,所以增加一列
		resTable.InsertCol(6,Tx::Core::dtype_val_string);//str报告期
		//把报告年份和报告期转化为CString类型
		tempTable.Clear();
		tempTable.Clone(resTable);
#ifdef _DEBUG
		strTable=tempTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
		TransReportDateToNormal3(tempTable,4);//减少两列，分别是报告年份、报告期（int）
#ifdef _DEBUG
		CString strTable1 = tempTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
		if ( iRet == 3 )
		{
			int iTexmpCount = tempTable.GetRowCount();
			for( int i = 0; i < iTexmpCount; i++ )
			{
				CString strDate;
				tempTable.GetCell( 5, i, strDate );
				CString str;
				str = strDate.Left(4);
				str = str + _T("中报");
				tempTable.SetCell( 5, i, str );
			}
		}
		else if ( iRet == 9 )
		{
			int iTexmpCount = tempTable.GetRowCount();
			for( int i = 0; i < iTexmpCount; i++ )
			{
				CString strDate;
				tempTable.GetCell( 5, i, strDate );
				CString str;
				str = strDate.Left(4);
				str = str + _T("年报");
				tempTable.SetCell( 5, i, str );
			}
		}

		resTable.Clear();
		resTable.Clone(tempTable);
		MultiSortRule multisort;
		multisort.AddRule(4,false);
		multisort.AddRule(2,true);
		multisort.AddRule(7,false);	
		resTable.SortInMultiCol(multisort);
		resTable.Arrange();
		resTable.DeleteCol(4);

		resTable.DeleteCol(5);//基金定期报告ID
		resTable.DeleteCol(3);//删除基金ID
	}
	//由于数据库中没有数据，所以暂时先把把行业集中度下面的那两列去掉，等以后有数据了再添加上。
	int EraseCol = resTable.GetColCount();
	resTable.DeleteCol(EraseCol - 1);
	resTable.DeleteCol(EraseCol - 2);
	/*if(iType2 > 17)
	{
		resTable.SetSortRange(0,resTable.GetRowCount()-2);
	}*/
	//添加进度条
	prw.SetPercent(pid,1.0);
	return true;
}
	//add by lijw 2008-02-19
bool TxFund::StatStockHoldingTopTen(
									Tx::Core::Table_Indicator &resTable,
									std::vector<int>	iSecurityId,
									std::vector<int>	iDate,
									int		iType
									)
{
	//添加进度条
//	ProgressWnd* pwd=Tx::Core::ProgressWnd::GetInstance();
	ProgressWnd prw;
	UINT pid=prw.AddItem(1,_T("重仓股统计..."),0.0);
	prw.Show(1000);
	m_txTable.Clear();//这是引用别人的成员变量，
	//从T_ASSET_ALLOCATION_twoyear里取基金定期报告ID，基金ID，报告年份，报告期
	//默认的返回值状态
	bool result = false;
	//把交易实体ID转化为基金ID
	std::vector<int> iFundId;
	/*if(!TransObjectToSecIns(iSecurityId,iSecurity1Id,1))
	return false;	*/
	std::vector<int>::iterator iterV;
	std::vector<int> tempTradeV;
	int tempId;
	for (iterV = iSecurityId.begin();iterV != iSecurityId.end();++iterV)
	{
		GetSecurityNow(*iterV);
		if (m_pSecurity != NULL)
		{
			tempId = m_pSecurity->GetSecurity1Id(*iterV);
			if (find(iFundId.begin(),iFundId.end(),tempId) == iFundId.end())
			{
				iFundId.push_back(tempId);
				tempTradeV.push_back(*iterV);
			}			
		}
	}
	iSecurityId.clear();
	iSecurityId.assign(tempTradeV.begin(),tempTradeV.end());
	//清空数据
	m_txTable.Clear();
	//准备样本集参数列
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//基金ID
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//报告年份
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//报告期
	const int indicatorIndex = 2;
	//取回的是基金券id
	long iIndicator[indicatorIndex] = 
	{
		30901332,	//基金定期报告ID//这是T_ASSET_ALLOCATION表里
		30901140	//资产净值
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
			return FALSE;
		}

	}
	UINT iColCount = m_txTable.GetColCount();
	UINT* nColArray = new UINT[iColCount];
	for(int i = 0; i < (int)iColCount; i++)
	{
		nColArray[i] = i;
	}
	result = m_pLogicalBusiness->GetData(m_txTable,true);
#ifdef _DEBUG
	CString strTable=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	
	Tx::Core::Table_Indicator tempTable;
	tempTable.CopyColumnInfoFrom(m_txTable);
	//根据基金ID进行筛选
	m_txTable.EqualsAt(tempTable,nColArray,iColCount,0,iFundId);
	//为了进行年度和报告期的筛选，所以把报告年份和报告期和为一列。
	TransReportDateToNormal2(tempTable,1);
#ifdef _DEBUG
	strTable=tempTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	m_txTable.Clear();
	m_txTable.CopyColumnInfoFrom(tempTable);
	//进行年度和报告期的筛选
	tempTable.EqualsAt(m_txTable,nColArray,iColCount-1,1,iDate);
	if (m_txTable.GetRowCount() == 0)
	{
		delete nColArray;
		nColArray = NULL;
		//添加进度条
		prw.SetPercent(pid,1.0);
		return false;
	}
#ifdef _DEBUG
	strTable=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//取出基金定期报告ID，把它放到vector里，为和从视图T_STOCK_HOLDING_TOP_TEN_twoyear取得数据进行连接作准备。
	std::vector<int> ReportV;
	for(int i = 0;i < (int)m_txTable.GetRowCount();i++)
	{
		int reportid ;
		m_txTable.GetCell(2,i,reportid);
		ReportV.push_back(reportid);
	}
	delete nColArray;
	nColArray = NULL;


	//从T_STOCK_HOLDING_TOP_TEN_twoyear取其他的数据
	Tx::Core::Table_Indicator hbTable;
	//准备样本集参数列
	hbTable.AddParameterColumn(Tx::Core::dtype_int4);//基金ID
	hbTable.AddParameterColumn(Tx::Core::dtype_int4);//报告年份
	hbTable.AddParameterColumn(Tx::Core::dtype_int4);//报告期
	hbTable.AddParameterColumn(Tx::Core::dtype_byte);//F1序号
	const int indicatorIndex3 = 7;
	//取回的是基金券id
	long iIndicator3[indicatorIndex3] = 
	{
		30901329,	//基金定期报告ID//这是T_STOCK_HOLDING_TOP_TEN表里
		30901310,	//市值排名
		30901238,	//股票ID
		30901239,	//股票市值
		30901311,	//持股数量
		30901240,	//所占比例（即是占净值比例）
		30901312	//占流通股比例（%）
//		30901328	//占基金持股市值比例（%）备注：由于分组统计时，取数据比较麻烦，所以把该字段去掉。
	};
	UINT varCfg3[4];			//参数配置
	int varCount3=4;			//参数个数
	for (int i = 0; i < indicatorIndex3; i++)
	{
		int tempIndicator = iIndicator3[i];

		GetIndicatorDataNow(tempIndicator);
		if (m_pIndicatorData==NULL)
		{ 
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false;
		}
		varCfg3[0]=0;
		varCfg3[1]=1;
		varCfg3[2]=2;
		varCfg3[3]=3;
		result = m_pLogicalBusiness->SetIndicatorIntoTable(
			m_pIndicatorData,	//指标
			varCfg3,				//参数配置
			varCount3,			//参数个数
			hbTable	//计算需要的参数传输载体以及计算后结果的载体
			);
		if(result==false)
		{ 
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false;
		}

	}
	
	result = m_pLogicalBusiness->GetData(hbTable,true);
	hbTable.DeleteCol(3);//删除序号列，之所以在删掉，是因为不想改下面的代码。
#ifdef _DEBUG
	strTable=hbTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	UINT iColCount3 = hbTable.GetColCount();
	UINT* nColArray3 = new UINT[iColCount3];
	for(int i = 0; i < (int)iColCount3; i++)
	{
		nColArray3[i] = i;
	}
	resTable.CopyColumnInfoFrom(hbTable);
	hbTable.EqualsAt(resTable,nColArray3,iColCount3,3,ReportV);
	if(resTable.GetRowCount() == 0)
	{
		delete nColArray3;
		nColArray3 = NULL;
		//添加进度条
		prw.SetPercent(pid,1.0);
		return false;
	}
	delete nColArray3;
	nColArray3 = NULL;
#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//添加进度条
	prw.SetPercent(pid,0.3);
  	//为增加基金的交易实体ID和名称，代码作准备
	resTable.InsertCol(0,Tx::Core::dtype_int4);//基金交易实体ID
	resTable.InsertCol(1,Tx::Core::dtype_val_string);//基金名称
	resTable.InsertCol(2,Tx::Core::dtype_val_string);//基金代码
	//为把m_txTable的内容放到resTable表里作准备。
	resTable.InsertCol(6,Tx::Core::dtype_decimal);//资产净值
	//把股票ID（也既是内码）转换成股票名称和代码
	resTable.InsertCol(10,Tx::Core::dtype_val_string);//股票strcode
	resTable.InsertCol(11,Tx::Core::dtype_val_string);//股票strname
	double equity2;
	double dEquity,dCircle;
    for(int m = 0;m < (int)m_txTable.GetRowCount();m++)
	{
		int reId2;		
		m_txTable.GetCell(2,m,reId2);
		m_txTable.GetCell(3,m,equity2);
		std::vector<UINT> vecInstiID;
		resTable.Find(7,reId2,vecInstiID);
		std::vector<UINT>::iterator iteID;
		for(iteID = vecInstiID.begin();iteID != vecInstiID.end();++iteID)
		{
			int stockId ;
			resTable.GetCell(9,*iteID,stockId);
			GetSecurityNow(stockId);
			if(m_pSecurity == NULL)
				continue;
			CString strname,strcode;
			strname = m_pSecurity->GetName();
			strcode = m_pSecurity->GetCode();
			//把占净值比，占流通股比，分别乘以100			
			resTable.GetCell(14,*iteID,dEquity);			
			if(fabs(dEquity - Tx::Core::Con_doubleInvalid) < 0.00001)
				dEquity = Tx::Core::Con_doubleInvalid;
			else
				dEquity = dEquity*100;
			resTable.SetCell(14,*iteID,dEquity);
			resTable.GetCell(15,*iteID,dCircle);
			if(fabs(dCircle - Tx::Core::Con_doubleInvalid) < 0.00001)
				dCircle = Tx::Core::Con_doubleInvalid;
			else
				dCircle = dCircle*100;			
			resTable.SetCell(15,*iteID,dCircle);
			//resTable.GetCell(16,*iteID,dValue);
			//dValue = dValue*100;
   //         resTable.SetCell(16,*iteID,dValue);//备注：把占基金持股市值比例去掉
			resTable.SetCell(10,*iteID,strcode);
			resTable.SetCell(11,*iteID,strname);
        	resTable.SetCell(6,*iteID,equity2);
		}

	}
#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//为了进行年度和报告期的筛选，所以把报告年份和报告期和为一列。
	TransReportDateToNormal2(resTable,4);
	//增加基金的交易实体ID和名称，代码
	std::vector<int>::iterator iterId;
	std::unordered_map<int,double> dDataMap;//存放临时的基金净值	
	std::vector<int>::iterator iterDate;
	for(iterId = iSecurityId.begin();iterId != iSecurityId.end();++iterId)
	{
		int fundId  ;
		CString strName,strCode;
		GetSecurityNow(*iterId);
		if(m_pSecurity == NULL)
			continue;
		fundId = m_pSecurity->GetSecurity1Id();
		strName = m_pSecurity->GetName();
		strCode = m_pSecurity->GetCode();
		std::vector<UINT> vecInstiID;
		resTable.Find(3,fundId,vecInstiID);		
		std::vector<UINT>::iterator iteID;
		for(iteID = vecInstiID.begin();iteID != vecInstiID.end();++iteID)
		{
			resTable.SetCell(0,*iteID,*iterId);
			resTable.SetCell(1,*iteID,strName);
			resTable.SetCell(2,*iteID,strCode);
		}
		if(iType >=16)
		{
			//把所有相同的报告期的基金净值加起来。这是为了样本汇总时作准备。			
			int itempdate;
			std::vector<UINT> TempCollect;
			//填充dDataMap，把各个报告期的基金净值设为零
			for(iterDate = iDate.begin();iterDate != iDate.end();++iterDate)
			{
				dDataMap.insert(std::make_pair(*iterDate,0));
			}
			for(iterDate = iDate.begin();iterDate != iDate.end();++iterDate)
			{
				itempdate = *iterDate;
				if(!TempCollect.empty())
					TempCollect.clear();
				resTable.Find(4,itempdate,TempCollect);//取得与报告期相同的纪录的位置。
				if(TempCollect.empty())
					continue;
				std::set<UINT> positionSet(vecInstiID.begin(),vecInstiID.end());
				iteID = TempCollect.begin();
				for(;iteID != TempCollect.end();++iteID)
				{
					if(positionSet.find(*iteID) != positionSet.end())
					{
						resTable.GetCell(5,*iteID,equity2);
						dDataMap[itempdate] += equity2; 
						break;
					}
					else
						continue;
				}
			}			
		}
	}
	MultiSortRule multisort;
	multisort.AddRule(0,true);
	multisort.AddRule(7,true);
	resTable.SortInMultiCol(multisort);
	int iType2 = iType;//保存原来的值.
	//这里增加样本分类选择
	if(iType >1)
	{	
		int iCount = resTable.GetRowCount();//累加之前resTable里的行数。		
		//为了进行分类统计。所以增加一列基金ID。
		resTable.AddCol(Tx::Core::dtype_int4);//基金风格
		resTable.AddCol(Tx::Core::dtype_int4);//基金管理公司ID
		resTable.AddCol(Tx::Core::dtype_int4);//基金托管银行ID		
		//添加进度条
		prw.SetPercent(pid,0.6);
		//这里增加样本分类选择
		m_txTable.Clear();
		tempTable.Clear();
		//准备样本集=第一参数列:F_FUND_ID,int型
		m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
		const int indicatorIndex1 = 3;
		//取回的是基金交易实体id
		long iIndicator1[indicatorIndex1] = 
		{
			//30001035,	//基金风格
			//modified by zhangxs 20091221---NewStyle
			30001232,	//基金风格New
			30001020,	//管理公司ID，
			30001021	//托管银行ID，
		};
		UINT varCfg1[1];			//参数配置
		int varCount1=1;			//参数个数
		for (int i = 0; i < indicatorIndex1; i++)
		{
			int tempIndicator = iIndicator1[i];
			GetIndicatorDataNow(tempIndicator);
			if (m_pIndicatorData==NULL)
				return false; 
			varCfg1[0]=0;
			result = m_pLogicalBusiness->SetIndicatorIntoTable(
				m_pIndicatorData,	//指标
				varCfg1,				//参数配置
				varCount1,			//参数个数
				m_txTable	//计算需要的参数传输载体以及计算后结果的载体
				);
			if(result==false)
			{
				return FALSE;
			}

		}
		result = m_pLogicalBusiness->GetData(m_txTable,true);
		if(result==false)
			return false;
#ifdef _DEBUG
		strTable=m_txTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
		UINT iCol2=m_txTable.GetColCount();
		//复制所有列信息
		tempTable.CopyColumnInfoFrom(m_txTable);
		if(m_txTable.GetRowCount()==0)
		{
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false;
		}
		UINT* nColArray2 = new UINT[iCol2];
		for(UINT i=0;i<iCol2;i++)
			nColArray2[i]=i;	
		m_txTable.EqualsAt(tempTable,nColArray2,iCol2,0,iSecurityId);
		delete nColArray2;
		nColArray2 = NULL;
		//把基金风格、公司ID、托管银行ID放到resTable
		std::vector<UINT> vecInstiID2;
		std::vector<UINT>::iterator iteID;		
		int iStyle,iCompanyId,ibankId;
		int tempfundId;
		int position;
		int icount = tempTable.GetRowCount();
		for(int j = 0;j < icount;j++)
		{
			tempTable.GetCell(0,j,tempfundId);
			tempTable.GetCell(1,j,iStyle);
			tempTable.GetCell(2,j,iCompanyId);
			tempTable.GetCell(3,j,ibankId);
			//把基金ID和交易实体ID对应起来。并且把数据放到表里。
			if(!(vecInstiID2.empty()))
				vecInstiID2.clear();
//#ifdef _SECURITYID_FOR_STAT_
			resTable.Find(0,tempfundId,vecInstiID2);
//#else
//			resTable.Find(3,tempfundId,vecInstiID2);
//#endif
			for(iteID = vecInstiID2.begin();iteID != vecInstiID2.end();++iteID)
			{
				position = *iteID;
				resTable.SetCell(15,position,iStyle);//基金风格
				resTable.SetCell(16,position,iCompanyId);//管理公司ID
				resTable.SetCell(17,position,ibankId);//托管银行ID
			}
		}		
		std::vector<int> ColVector;		//根据哪些列进行统计  
		if(!(iType & 1))
			iType += 1;//假设都选择选择了单只基金那一项分类方式。
		if(iType & 2)
			ColVector.push_back(16);
		if(iType & 4)
			ColVector.push_back(15);
		if(iType & 8)
			ColVector.push_back(17);
		std::vector<int> IntCol;			//需要相加的整型列
		//		IntCol.push_back(6);
		std::vector<int> DoubleCol;	//需要相加的double列
//		DoubleCol.push_back(5);
		DoubleCol.push_back(11);
		DoubleCol.push_back(12);
		/*if (iType2 < 17)
		DoubleCol.push_back(5);*/
		/*for(int j = 6;j < 17;j++)
		{
		DoubleCol.push_back(j);
		j++;
		}*/
		int iTradeId = 8;//之所以这样做，是为了让它带回汇总样本统计的纪录数。
		AddUpRow(resTable,iType,ColVector,IntCol,DoubleCol,iDate,4,5,3,iTradeId,11,7);
		//		std::vector<int>::size_type isize = iDate.size();
		int iCount1 = resTable.GetRowCount();//累加以后resTable里的行数。
		int iCount2,iCount3;
		iCount2 = iCount1 - iCount;//这是增加的行数 样本汇总和其他分类方式的行数。
		std::unordered_map<int,CString> CompanyMap;
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_INSTITUTIONID_TO_SHORTNAME,CompanyMap);
		std::unordered_map<int,CString> StyleMap;
		//modified by zhangxs 20091221---NewStyle
		//TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX,StyleMap);
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX_NEW,StyleMap);
		std::unordered_map<int,CString> BankMap;
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_BANK,BankMap);
		std::unordered_map<int,CString>::iterator iterMap;
		int id;
		CString Name;	
		if(iType < 17)//没有样本汇总的情况
		{
			for(int m = 0;m < iCount2;m++)
			{
				position = iCount+m;
				resTable.GetCell(15,position,id);
				if(id == 0)
				{
					resTable.GetCell(16,position,id);
					if(id == 0)
					{
						resTable.GetCell(17,position,id);
						iterMap = BankMap.find(id);
						if(iterMap!= BankMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,position,Name);
						resTable.SetCell(2,position,(CString)(_T("托管银行")));
						continue;
					}
					else
					{
						iterMap = CompanyMap.find(id);
						if(iterMap!= CompanyMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,position,Name);
						resTable.SetCell(2,position,(CString)(_T("基金公司")));
						continue;
					}					
				}
				else
				{
					iterMap = StyleMap.find(id);
					if(iterMap!= StyleMap.end())
						Name = iterMap->second;
					else
						Name = Tx::Core::Con_strInvalid;
					resTable.SetCell(1,position,Name);
					resTable.SetCell(2,position,(CString)(_T("基金风格")));
				}
			}				
		}
		else//不但有单只基金和样本汇总，还有其他的分类方式
		{
			int isize = iTradeId;//这是样本汇总的行数.
			iCount3 = iCount2 - isize;//除了样本汇总以外的其他分类方式的行数
			if(iCount3 > 0)//表示除了样本汇总以外还有其他的分类的方式
			{
				//填充样本汇总的那些行
				for(int k = 0;k < isize;k++)
				{
					resTable.SetCell(1,iCount+iCount3+k,(CString)(_T("样本汇总")));
					resTable.SetCell(2,iCount+iCount3+k,(CString)(_T("全部汇总")));
				}
				//填充除了样本汇总以外的其他分类方式的那些行。
				for(int i = 0;i < iCount3;i++)
				{
					position = iCount+i;
					resTable.GetCell(15,position,id);
					if(id == 0)
					{
						resTable.GetCell(16,position,id);
						if(id == 0)
						{
							resTable.GetCell(17,position,id);
							iterMap = BankMap.find(id);
							if(iterMap!= BankMap.end())
								Name = iterMap->second;
							else
								Name = Tx::Core::Con_strInvalid;
							resTable.SetCell(1,position,Name);
							resTable.SetCell(2,position,(CString)(_T("托管银行")));
							continue;
						}
						else
						{
							iterMap = CompanyMap.find(id);
							if(iterMap!= CompanyMap.end())
								Name = iterMap->second;
							else
								Name = Tx::Core::Con_strInvalid;
							resTable.SetCell(1,position,Name);
							resTable.SetCell(2,position,(CString)(_T("基金公司")));
							continue;
						}					
					}
					else
					{
						iterMap = StyleMap.find(id);
						if(iterMap!= StyleMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,position,Name);
						resTable.SetCell(2,position,(CString)(_T("基金风格")));						
					}			
				}
			}
			else//只有单只基金和汇总的情况
			{				
				for(int k = 0;k < isize;k++)
				{
					position = iCount+k;
					resTable.SetCell(1,position,(CString)(_T("样本汇总")));
					resTable.SetCell(2,position,(CString)(_T("全部汇总")));			
				}		
			}
		}	
		//把报告年份和报告期转化成CString类型,所以增加一列
		resTable.InsertCol(5,Tx::Core::dtype_val_string);//str报告期	
		CString stryear,strdate;
		int tempdate,iyear ,itemp;
		int tempCount;
		int n = 0;
		if(!(iType2 & 1))
		{
			resTable.DeleteRow(0,iCount);
			tempCount = iCount2;
			n = 0;
		}
		else
		{
			for(int i = 0;i < iCount;i++)
			{
				//填写报告期	
				resTable.GetCell(4,i,tempdate);	
				itemp = tempdate%10000;
				iyear = tempdate/10000;
				stryear.Format(_T("%d"),iyear);
				switch(itemp)
				{
				case 331:
					strdate = stryear + _T("年") + _T("一季报");
					break;
				case 630:
					strdate = stryear + _T("年") + _T("二季报");
					break;
				case 930:
					strdate = stryear + _T("年") + _T("三季报");
					break;
				case 1231:
					strdate = stryear + _T("年") + _T("四季报");
					break;
				}
				resTable.SetCell(5,i,strdate);			
			}
			tempCount = iCount1;	
			n = iCount;
		}
		double proportion,Value;
		std::unordered_map<int,double>::iterator iterMap2;
		int tempId;
		double dtradeShare,dCount;
		for(;n < tempCount;n++)
		{
			//填写报告期	
			resTable.GetCell(4,n,tempdate);	
			itemp = tempdate%10000;
			iyear = tempdate/10000;
			stryear.Format(_T("%d"),iyear);
			switch(itemp)
			{
			case 331:
				strdate = stryear + _T("年") + _T("一季报");
				break;
			case 630:
				strdate = stryear + _T("年") + _T("二季报");
				break;
			case 930:
				strdate = stryear + _T("年") + _T("三季报");
				break;
			case 1231:
				strdate = stryear + _T("年") + _T("四季报");
				break;
			}			
			//把样本汇总的基金净值放到resTable
			if(iType >=17 && n >= tempCount - iTradeId)
			{
				iterMap2 = dDataMap.find(tempdate);
				if(iterMap2 != dDataMap.end())
				{
					equity2 = iterMap2->second;
					resTable.SetCell(6,n,equity2);
					//计算比例					
					resTable.GetCell(12,n,Value);//取得股票的市值
					if(equity2 < 0 || Value < 0)
						proportion = Tx::Core::Con_doubleInvalid;
					proportion = Value*100/equity2;			
					resTable.SetCell(14,n,proportion);	
					//计算流通比例；
					resTable.GetCell(9,n,tempId);
					GetSecurityNow(tempId);
					if(m_pSecurity != NULL)
					{
						dtradeShare = m_pSecurity->GetTradableShare();
						resTable.GetCell(13,n,dCount);
						if(dtradeShare < 0 || dCount < 0)
							dtradeShare = Tx::Core::Con_doubleInvalid;
						else
							dtradeShare = dCount/dtradeShare;
						resTable.SetCell(15,n,dtradeShare);
					}
				}
			}
			else
			{
				//计算比例
				resTable.GetCell(6,n,equity2);//取得资产净值
				resTable.GetCell(12,n,Value);//取得股票的市值
				if(equity2 < 0 || Value < 0)
					proportion = Tx::Core::Con_doubleInvalid;
				proportion = Value*100/equity2;			
				resTable.SetCell(14,n,proportion);	
				//计算流通比例；
				resTable.GetCell(9,n,tempId);
				GetSecurityNow(tempId);
				if(m_pSecurity != NULL)
				{
					dtradeShare = m_pSecurity->GetTradableShare();
					resTable.GetCell(13,n,dCount);
					if(dtradeShare < 0 || dCount < 0)
						dtradeShare = Tx::Core::Con_doubleInvalid;
					else
						dtradeShare = dCount/dtradeShare;
					resTable.SetCell(15,n,dtradeShare);
				}
			}
			resTable.SetCell(5,n,strdate);
		}
		resTable.DeleteCol(16,3);
	}
	else
	{	
		//添加进度条
		prw.SetPercent(pid,0.7);
		//把报告年份和报告期转化成CString类型,所以增加一列
		resTable.InsertCol(5,Tx::Core::dtype_val_string);//str报告期	
		CString stryear,strdate;
		int tempdate,iyear,itemp;
		//填写报告期
		for(int k = 0;k < (int)resTable.GetRowCount();k++)
		{
			resTable.GetCell(4,k,tempdate);	
			itemp = tempdate%10000;
			iyear = tempdate/10000;
			stryear.Format(_T("%d"),iyear);
			switch(itemp)
			{
			case 331:
				strdate = stryear + _T("年") + _T("一季报");
				break;
			case 630:
				strdate = stryear + _T("年") + _T("二季报");
				break;
			case 930:
				strdate = stryear + _T("年") + _T("三季报");
				break;
			case 1231:
				strdate = stryear + _T("年") + _T("四季报");
				break;
			}
			resTable.SetCell(5,k,strdate);
		}
		
	}
	resTable.DeleteCol(9);//删除股票ID
	resTable.DeleteCol(7);//基金定期报告ID
	resTable.DeleteCol(3);//删除基金ID和报告期
	resTable.Arrange();
	MultiSortRule multisort2;
	multisort2.AddRule(3,false);
	multisort2.AddRule(1,true);	
	multisort2.AddRule(6,true);
	resTable.SortInMultiCol(multisort2);
	resTable.Arrange();
	resTable.DeleteCol(3);
	//添加进度条
	//if(iType2 > 17)
	//{
	//	resTable.SetSortRange(0,resTable.GetRowCount()-2);
	//}
	prw.SetPercent(pid,1.0);
	return true;
}
//获取基金==//30001232,	//基金风格New；30001020,	//管理公司ID；30001021	//托管银行ID
//多处地方用到，所以提出来便于以后修改
bool TxFund::Get_Fund_Type_Manager_Company(Tx::Core::Table_Indicator &resTable,
								   std::vector<int>	iSecurityId
								   )
{
	bool result = false;
	resTable.Clear();
	//为了进行分类统计。所以增加一列基金ID。
	resTable.AddCol(Tx::Core::dtype_int4);//基金风格
	resTable.AddCol(Tx::Core::dtype_int4);//基金管理公司ID
	resTable.AddCol(Tx::Core::dtype_int4);//基金托管银行ID		
	//添加进度条
	//这里增加样本分类选择
	m_txTable.Clear();
	//准备样本集=第一参数列:F_FUND_ID,int型
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
	const int indicatorIndex1 = 3;
	//取回的是基金交易实体id
	long iIndicator1[indicatorIndex1] = 
	{
		//30001035,	//基金风格
		//modified by zhangxs 20091221---NewStyle
		30001232,	//基金风格New
		30001020,	//管理公司ID，
		30001021	//托管银行ID，
	};
	UINT varCfg1[1];			//参数配置
	int varCount1=1;			//参数个数
	for (int i = 0; i < indicatorIndex1; i++)
	{
		int tempIndicator = iIndicator1[i];
		GetIndicatorDataNow(tempIndicator);
		if (m_pIndicatorData==NULL)
			return false; 
		varCfg1[0]=0;
		result = m_pLogicalBusiness->SetIndicatorIntoTable(
			m_pIndicatorData,	//指标
			varCfg1,				//参数配置
			varCount1,			//参数个数
			m_txTable	//计算需要的参数传输载体以及计算后结果的载体
			);
		if(!result)
		{
			return false;
		}

	}
	result = m_pLogicalBusiness->GetData(m_txTable,true);
	if(!result)
		return false;
#ifdef _DEBUG
	strTable=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	UINT iCol2=m_txTable.GetColCount();
	//复制所有列信息
	resTable.CopyColumnInfoFrom(m_txTable);
	if(m_txTable.GetRowCount()==0)
	{
		return false;
	}
	UINT* nColArray2 = new UINT[iCol2];
	for(UINT i=0;i<iCol2;i++)
		nColArray2[i]=i;	
	m_txTable.EqualsAt(resTable,nColArray2,iCol2,0,iSecurityId);
	delete nColArray2;
	nColArray2 = NULL;
	return true;
}
//add by lijw 2008-02-21
//持股明细统计
bool TxFund::StatStockHolding(
							  Tx::Core::Table_Indicator &resTable,
							  std::vector<int>	iSecurityId,
							  std::vector<int>	iDate,
							  int		iType,
							  int	    &iTempCount
							  )
{
	//添加进度条
//	ProgressWnd* pwd=Tx::Core::ProgressWnd::GetInstance();
	ProgressWnd prw;
	UINT pid=prw.AddItem(1,_T("持股明细统计..."),0.0);
	prw.Show(1000);
	m_txTable.Clear();//这是引用别人的成员变量，
	//从T_ASSET_ALLOCATION_twoyear里取基金定期报告ID，基金ID，报告年份，报告期
	//默认的返回值状态
	bool result = false;
	//把交易实体ID转化为基金ID
	std::vector<int> iFundId;
	std::vector<int>::iterator iterV;
	int fundId;
	std::vector<int> tempTradeV;
	for(iterV = iSecurityId.begin();iterV != iSecurityId.end();++iterV)
	{
		GetSecurityNow(*iterV);
		if(m_pSecurity != NULL)
		{
			fundId = m_pSecurity->GetSecurity1Id(*iterV);
			if(find(iFundId.begin(),iFundId.end(),fundId) == iFundId.end())
			{
				iFundId.push_back(fundId);
				tempTradeV.push_back(*iterV);
			}			
		}
	}
	iSecurityId.clear();
	iSecurityId.assign(tempTradeV.begin(),tempTradeV.end());
	//清空数据
	m_txTable.Clear();
	//准备样本集参数列
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//基金ID
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//报告年份
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//报告期
	const int indicatorIndex = 1;
	//取回的是基金券id
	long iIndicator[indicatorIndex] = 
	{
		30901140	//资产净值
	};
	UINT varCfg[3];			//参数配置
	int varCount=3;			//参数个数
	for (int i = 0; i < indicatorIndex; i++)
	{
		int tempIndicator = iIndicator[i];

		GetIndicatorDataNow(tempIndicator);
		if (m_pIndicatorData==NULL)
		{ 
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false; 
		}
		varCfg[0]=0;
		varCfg[1]=1;
		varCfg[2]=2;
		if(m_pLogicalBusiness ==NULL)
			continue;
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
			return FALSE;
		}

	}
	UINT iColCount = m_txTable.GetColCount();
	UINT* nColArray = new UINT[iColCount];
	for(int i = 0; i < (int)iColCount; i++)
	{
		nColArray[i] = i;
	}
	if(m_pLogicalBusiness ==NULL)
		return FALSE;
	result = m_pLogicalBusiness->GetData(m_txTable,true);
	if(result == false || m_txTable.GetRowCount() == 0)
	{
		delete nColArray;
		nColArray = NULL;
		//添加进度条
		prw.SetPercent(pid,1.0);
		return false;
	}
	CString strTable;
	CString strTableTmp;
#ifdef _DEBUG
	strTable=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif	
	
	Tx::Core::Table_Indicator tempTable;
	tempTable.CopyColumnInfoFrom(m_txTable);
	//根据基金ID进行筛选
	m_txTable.EqualsAt(tempTable,nColArray,iColCount,0,iFundId);
	if(tempTable.GetRowCount() == 0)
	{
		delete nColArray;
		nColArray = NULL;
		//添加进度条
		prw.SetPercent(pid,1.0);
		return false;
	}
	//为了进行年度和报告期的筛选，所以把报告年份和报告期和为一列。
	TransReportDateToNormal2(tempTable,1);
	m_txTable.Clear();
	m_txTable.CopyColumnInfoFrom(tempTable);
	//进行年度和报告期的筛选
	tempTable.EqualsAt(m_txTable,nColArray,iColCount-1,1,iDate);
	if(m_txTable.GetRowCount() == 0)
	{
		delete nColArray;
		nColArray = NULL;
		//添加进度条
		prw.SetPercent(pid,1.0);
		return false;
	}
#ifdef _DEBUG
	strTable=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif	
	delete nColArray;
	nColArray = NULL;


	//从T_STOCK_HOLDING_TOP_TEN_twoyear取其他的数据
	Tx::Core::Table_Indicator hbTable;
	//准备样本集参数列
	hbTable.AddParameterColumn(Tx::Core::dtype_int4);//基金ID
	hbTable.AddParameterColumn(Tx::Core::dtype_int4);//报告年份
	hbTable.AddParameterColumn(Tx::Core::dtype_int4);//报告期
	hbTable.AddParameterColumn(Tx::Core::dtype_byte);//持股类型
	hbTable.AddParameterColumn(Tx::Core::dtype_int4);//序号
	const int indicatorIndex3 = 6;
	//取回的是基金券id
	long iIndicator3[indicatorIndex3] = 
	{
		30901320,	//市值排名
		30901128,	//股票ID
		30901130,	//股票市值
		30901129,	//持股数量
		30901321,	//所占比例（即是占净值比例）
		30901322	//占流通股比例（%）
	};
	UINT varCfg3[5];			//参数配置
	int varCount3=5;			//参数个数
	for (int i = 0; i < indicatorIndex3; i++)
	{
		int tempIndicator = iIndicator3[i];

		GetIndicatorDataNow(tempIndicator);
		if (m_pIndicatorData==NULL)
		{ return false; }
		varCfg3[0]=0;
		varCfg3[1]=1;
		varCfg3[2]=2;
		varCfg3[3]=3;
		varCfg3[4]=4;
		if(m_pLogicalBusiness ==NULL)
			continue;
		result = m_pLogicalBusiness->SetIndicatorIntoTable(
			m_pIndicatorData,	//指标
			varCfg3,				//参数配置
			varCount3,			//参数个数
			hbTable	//计算需要的参数传输载体以及计算后结果的载体
			);
		if(result==false)
		{
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false;
		}
	}
	if(m_pLogicalBusiness ==NULL)
		return FALSE;
	result = m_pLogicalBusiness->GetData(hbTable,true);
	if(hbTable.GetRowCount() == 0)
	{
		//添加进度条
		prw.SetPercent(pid,1.0);
		return false;
	}
#ifdef _DEBUG
	strTable = hbTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	UINT iColCount4 = hbTable.GetColCount();
	UINT* nColArray4 = new UINT[iColCount4];
	for(int i = 0; i < (int)iColCount4; i++)
	{
		nColArray4[i] = i;
	}
	tempTable.Clear();
	tempTable.CopyColumnInfoFrom(hbTable);
	std::vector<byte> TypeVector;
	TypeVector.push_back(3);
	hbTable.EqualsAt(tempTable,nColArray4,iColCount4,3,TypeVector);
	delete nColArray4;
	nColArray4 = NULL;
	hbTable.Clear();
	hbTable.Clone(tempTable);//之所以这样写是为了不修改下面的代码
	hbTable.DeleteCol(3,2);//删除持股类型和序号列
#ifdef _DEBUG
	strTable=hbTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	UINT iColCount3 = hbTable.GetColCount();
	UINT* nColArray3 = new UINT[iColCount3];
	for(int i = 0; i < (int)iColCount3; i++)
	{
		nColArray3[i] = i;
	}
	tempTable.Clear();
	tempTable.CopyColumnInfoFrom(hbTable);

	hbTable.EqualsAt(tempTable,nColArray3,iColCount3,0,iFundId);

	//为了进行年度和报告期的筛选，所以把报告年份和报告期和为一列。
	TransReportDateToNormal2(tempTable,1);
    resTable.CopyColumnInfoFrom(tempTable);
	tempTable.EqualsAt(resTable,nColArray3,iColCount3 - 1,1,iDate);
	delete nColArray3;
	nColArray3 = NULL;
	if(resTable.GetRowCount() == 0)
	{		
		//添加进度条
		prw.SetPercent(pid,1.0);
		return false;
	}
#ifdef _DEBUG
	strTable=tempTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//添加进度条
	prw.SetPercent(pid,0.3);
	//为增加基金的交易实体ID和名称，代码作准备
	resTable.InsertCol(0,Tx::Core::dtype_int4);//基金交易实体ID
	resTable.InsertCol(1,Tx::Core::dtype_val_string);//基金名称
	resTable.InsertCol(2,Tx::Core::dtype_val_string);//基金代码
	//为把m_txTable的内容放到resTable表里作准备。
	resTable.InsertCol(5,Tx::Core::dtype_decimal);//资产净值
	//把股票ID（也既是内码）转换成股票名称和代码
	resTable.InsertCol(8,Tx::Core::dtype_val_string);//股票strcode
	resTable.InsertCol(9,Tx::Core::dtype_val_string);//股票strname
	//把资产净值添加到resTable里
	int fundid1,tempdate4,fundid5,tempdate5;
	fundid1 = tempdate4 = fundid5 = tempdate5 = 0;
	int stockId;
	double equity2;
	double dEquity,dCircle;
	CString strname,strcode;
	std::vector<UINT>vecID1,vecID2,vecID3;
	std::vector<UINT>::iterator iteID1,iteID2,iteID3;
	for(int m = 0;m < (int)resTable.GetRowCount();m++)
	{
		resTable.GetCell(3,m,fundid1);
		resTable.GetCell(4,m,tempdate4);
		if(fundid1 == fundid5 && tempdate4 == tempdate5)
		{
#ifdef _DEBUG
			if(vecID3.size() != 1)
			{
				AfxMessageBox(_T("数据出现错误"));
				//添加进度条
				prw.SetPercent(pid,1.0);
				return false;
			}
#endif
			iteID3 = vecID3.begin();
			m_txTable.GetCell(2,*iteID3,equity2);
			resTable.SetCell(5,m,equity2);
			//把占净值比，占流通股比，分别乘以100
			resTable.GetCell(12,m,dEquity);			
			if(fabs(dEquity - Tx::Core::Con_doubleInvalid) < 0.00001)
				dEquity = Tx::Core::Con_doubleInvalid;
			else
				dEquity = dEquity*100;
			resTable.SetCell(12,m,dEquity);
			resTable.GetCell(13,m,dCircle);			
			if(fabs(dCircle - Tx::Core::Con_doubleInvalid) < 0.00001)
				dCircle = Tx::Core::Con_doubleInvalid;
			else
				dCircle = dCircle*100;
			resTable.SetCell(13,m,dCircle);
			//添加名称和代码
			resTable.GetCell(7,m,stockId);
			GetSecurityNow(stockId);
			if(m_pSecurity == NULL)
				continue;
			strname = m_pSecurity->GetName();
			strcode = m_pSecurity->GetCode();
			resTable.SetCell(8,m,strcode);
			resTable.SetCell(9,m,strname);			
		}
		else
		{
			fundid5 = fundid1;
			tempdate5 = tempdate4;
			vecID1.clear();
			vecID2.clear();
			vecID3.clear();
			m_txTable.Find(0,fundid1,vecID1);//查找相等的fundid
			m_txTable.Find(1,tempdate4,vecID2);//查找相等的报告期（合并后的）
			if (vecID1.size() == 0 || vecID2.size() == 0)
			{
				fundid1 = tempdate4 = fundid5 = tempdate5 = 0;
				continue;
			}
			for(iteID1 = vecID1.begin();iteID1 != vecID1.end();++iteID1)
			{

				for(iteID2 = vecID2.begin();iteID2 != vecID2.end();++iteID2)
				{
					if(*iteID1 == *iteID2)
					{
						vecID3.push_back(*iteID1);
						break;
					}
				}
			}
			//当找不到符合的记录时。//add by lijw 2008-09-11
			if(vecID3.empty())
			{
				equity2 = Tx::Core::Con_doubleInvalid;
				resTable.SetCell(5,m,equity2);
				//把占净值比，占流通股比，分别乘以100
				resTable.GetCell(12,m,dEquity);
				dEquity = dEquity*100;
				resTable.SetCell(12,m,dEquity);
				resTable.GetCell(13,m,dCircle);
				dCircle = dCircle*100;
				resTable.SetCell(13,m,dCircle);
				//添加名称和代码
				resTable.GetCell(7,m,stockId);
				GetSecurityNow(stockId);
				if(m_pSecurity == NULL)
					continue;
				strname = m_pSecurity->GetName();
				strcode = m_pSecurity->GetCode();
				resTable.SetCell(8,m,strcode);
				resTable.SetCell(9,m,strname);			
				continue;
			}
#ifdef _DEBUG
			if(vecID3.size() != 1)
			{
				AfxMessageBox(_T("数据出现错误"));
				//添加进度条
				prw.SetPercent(pid,1.0);
				return false;
			}
#endif
			iteID3 = vecID3.begin();
			m_txTable.GetCell(2,*iteID3,equity2);
			resTable.SetCell(5,m,equity2);
			//把占净值比，占流通股比，分别乘以100
			resTable.GetCell(12,m,dEquity);
			dEquity = dEquity*100;
			resTable.SetCell(12,m,dEquity);
			resTable.GetCell(13,m,dCircle);
			dCircle = dCircle*100;
			resTable.SetCell(13,m,dCircle);
			//添加名称和代码
			resTable.GetCell(7,m,stockId);
			GetSecurityNow(stockId);
			if(m_pSecurity == NULL)
				continue;
			strname = m_pSecurity->GetName();
			strcode = m_pSecurity->GetCode();
			resTable.SetCell(8,m,strcode);
			resTable.SetCell(9,m,strname);			
		}
	}
//#ifdef _DEBUG
//	strTable=resTable.TableToString();
//	Tx::Core::Commonality::String().StringToClipboard(strTable);
//#endif
	//增加基金的交易实体ID和名称，代码
	std::unordered_map<int,double> dDataMap;//存放临时的基金净值	
	std::vector<int>::iterator iterDate;
	std::vector<UINT> vecInstiID;
	std::vector<UINT>::iterator iteID;
	for(iterV = iSecurityId.begin();iterV != iSecurityId.end();++iterV)
	{
		GetSecurityNow(*iterV);
		if(m_pSecurity == NULL)
			continue;
		fundId = m_pSecurity->GetSecurity1Id();
		strname = m_pSecurity->GetName();
		strcode = m_pSecurity->GetCode();
		if(!vecInstiID.empty())
			vecInstiID.clear();

		resTable.Find(3,fundId,vecInstiID);
		resTable.Find(3,fundId,vecInstiID);
		if(vecInstiID.empty())
			continue;
		for(iteID = vecInstiID.begin();iteID != vecInstiID.end();++iteID)
		{
			resTable.SetCell(0,*iteID,*iterV);
			resTable.SetCell(1,*iteID,strname);
			resTable.SetCell(2,*iteID,strcode);
		}
		if(iType >=16)
		{
			//把所有相同的报告期的基金净值加起来。这是为了样本汇总时作准备。			
			int itempdate;
			std::vector<UINT> TempCollect;
			//填充dDataMap，把各个报告期的基金净值设为零
			for(iterDate = iDate.begin();iterDate != iDate.end();++iterDate)
			{
				dDataMap.insert(std::make_pair(*iterDate,0));
			}
			for(iterDate = iDate.begin();iterDate != iDate.end();++iterDate)
			{
				itempdate = *iterDate;
				if(!TempCollect.empty())
					TempCollect.clear();
				resTable.Find(4,itempdate,TempCollect);//取得与报告期相同的纪录的位置。
				if(TempCollect.empty())
					continue;
				std::set<UINT> positionSet(vecInstiID.begin(),vecInstiID.end());
				iteID = TempCollect.begin();
				for(;iteID != TempCollect.end();++iteID)
				{
					if(positionSet.find(*iteID) != positionSet.end())
					{
						resTable.GetCell(5,*iteID,equity2);
						dDataMap[itempdate] += equity2; 
						break;
					}
					else
						continue;
				}
			}			
		}
	}
	//为了取得相应的结果，所以对resTable进行排序，分别按照基金ID、市值排名
	MultiSortRule multisort;
	multisort.AddRule(3,true);
	multisort.AddRule(6,true);
	resTable.SortInMultiCol(multisort);
	int iType2 = iType;//保存原来的值.
	//这里增加样本分类选择
	if(iType >1)
	{	
		int iCount = resTable.GetRowCount();//累加之前resTable里的行数。		
		//为了进行分类统计。所以增加一列基金ID。
		resTable.AddCol(Tx::Core::dtype_int4);//基金风格
		resTable.AddCol(Tx::Core::dtype_int4);//基金管理公司ID
		resTable.AddCol(Tx::Core::dtype_int4);//基金托管银行ID		
		//添加进度条
		prw.SetPercent(pid,0.6);
		//这里增加样本分类选择
		m_txTable.Clear();
		tempTable.Clear();
		//准备样本集=第一参数列:F_FUND_ID,int型
		m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
		const int indicatorIndex1 = 3;
		//取回的是基金交易实体id
		long iIndicator1[indicatorIndex1] = 
		{
			//30001035,	//基金风格
			//modified by zhangxs 20091221---NewStyle
			30001232,	//基金风格New
			30001020,	//管理公司ID，
			30001021	//托管银行ID，
		};
		UINT varCfg1[1];			//参数配置
		int varCount1=1;			//参数个数
		for (int i = 0; i < indicatorIndex1; i++)
		{
			int tempIndicator = iIndicator1[i];
			GetIndicatorDataNow(tempIndicator);
			if (m_pIndicatorData==NULL)
				return false; 
			varCfg1[0]=0;
			result = m_pLogicalBusiness->SetIndicatorIntoTable(
				m_pIndicatorData,	//指标
				varCfg1,				//参数配置
				varCount1,			//参数个数
				m_txTable	//计算需要的参数传输载体以及计算后结果的载体
				);
			if(result==false)
			{
				return FALSE;
			}

		}
		result = m_pLogicalBusiness->GetData(m_txTable,true);
		if(result==false)
			return false;
#ifdef _DEBUG
		strTable=m_txTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
		UINT iCol2=m_txTable.GetColCount();
		//复制所有列信息
		tempTable.CopyColumnInfoFrom(m_txTable);
		if(m_txTable.GetRowCount()==0)
		{
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false;
		}
		UINT* nColArray2 = new UINT[iCol2];
		for(UINT i=0;i<iCol2;i++)
			nColArray2[i]=i;	
		m_txTable.EqualsAt(tempTable,nColArray2,iCol2,0,iSecurityId);
		delete nColArray2;
		nColArray2 = NULL;
		//把基金风格、公司ID、托管银行ID放到resTable
		std::vector<UINT> vecInstiID2;
		std::vector<UINT>::iterator iteID;		
		int iStyle,iCompanyId,ibankId;
		int tempfundId;
		int position;
		int icount = tempTable.GetRowCount();
		for(int j = 0;j < icount;j++)
		{
			tempTable.GetCell(0,j,tempfundId);
			tempTable.GetCell(1,j,iStyle);
			tempTable.GetCell(2,j,iCompanyId);
			tempTable.GetCell(3,j,ibankId);
			//把基金ID和交易实体ID对应起来。并且把数据放到表里。
			if(!(vecInstiID2.empty()))
				vecInstiID2.clear();
//#ifdef _SECURITYID_FOR_STAT_
			resTable.Find(0,tempfundId,vecInstiID2);		
//#else
//			resTable.Find(3,tempfundId,vecInstiID2);
//#endif
			for(iteID = vecInstiID2.begin();iteID != vecInstiID2.end();++iteID)
			{
				position = *iteID;
				resTable.SetCell(14,position,iStyle);//基金风格
				resTable.SetCell(15,position,iCompanyId);//管理公司ID
				resTable.SetCell(16,position,ibankId);//托管银行ID
			}
		}		
		std::vector<int> ColVector;		//根据哪些列进行统计  
		if(!(iType & 1))
			iType += 1;//假设都选择选择了单只基金那一项分类方式。
		if(iType & 2)
			ColVector.push_back(15);
		if(iType & 4)
			ColVector.push_back(14);
		if(iType & 8)
			ColVector.push_back(16);
		std::vector<int> IntCol;			//需要相加的整型列
		//		IntCol.push_back(6);
		std::vector<int> DoubleCol;	//需要相加的double列
//		DoubleCol.push_back(5);
		DoubleCol.push_back(10);
		DoubleCol.push_back(11);
		/*if (iType2 < 17)
		DoubleCol.push_back(5);*/
		/*for(int j = 6;j < 17;j++)
		{
		DoubleCol.push_back(j);
		j++;
		}*/
		int iTradeId = 7;//之所以这样做，是为了让它带回汇总样本统计的纪录数。
		AddUpRow(resTable,iType,ColVector,IntCol,DoubleCol,iDate,4,5,3,iTradeId,10,6);
		//		std::vector<int>::size_type isize = iDate.size();
		int iCount1 = resTable.GetRowCount();//累加以后resTable里的行数。
		int iCount2,iCount3;
		iCount2 = iCount1 - iCount;//这是增加的行数 样本汇总和其他分类方式的行数。
		iTempCount = iCount2;
		std::unordered_map<int,CString> CompanyMap;
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_INSTITUTIONID_TO_SHORTNAME,CompanyMap);
		std::unordered_map<int,CString> StyleMap;
		//modified by zhangxs 20091221---NewStyle
		//TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX,StyleMap);
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX_NEW,StyleMap);
		std::unordered_map<int,CString> BankMap;
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_BANK,BankMap);
		std::unordered_map<int,CString>::iterator iterMap;
		int id;
		CString Name;	
		if(iType < 17)//没有样本汇总的情况
		{
			for(int m = 0;m < iCount2;m++)
			{
				position = iCount+m;
				resTable.GetCell(14,position,id);
				if(id == 0)
				{
					resTable.GetCell(15,position,id);
					if(id == 0)
					{
						resTable.GetCell(16,position,id);
						iterMap = BankMap.find(id);
						if(iterMap!= BankMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,position,Name);
						resTable.SetCell(2,position,(CString)(_T("托管银行")));
						continue;
					}
					else
					{
						iterMap = CompanyMap.find(id);
						if(iterMap!= CompanyMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,position,Name);
						resTable.SetCell(2,position,(CString)(_T("基金公司")));
						continue;
					}					
				}
				else
				{
					iterMap = StyleMap.find(id);
					if(iterMap!= StyleMap.end())
						Name = iterMap->second;
					else
						Name = Tx::Core::Con_strInvalid;
					resTable.SetCell(1,position,Name);
					resTable.SetCell(2,position,(CString)(_T("基金风格")));
				}
			}				
		}
		else//不但有单只基金和样本汇总，还有其他的分类方式
		{
			int isize = iTradeId;//这是样本汇总的行数.
			iCount3 = iCount2 - isize;//除了样本汇总以外的其他分类方式的行数
			if(iCount3 > 0)//表示除了样本汇总以外还有其他的分类的方式
			{
				//填充样本汇总的那些行
				for(int k = 0;k < isize;k++)
				{
					resTable.SetCell(1,iCount+iCount3+k,(CString)(_T("样本汇总")));
					resTable.SetCell(2,iCount+iCount3+k,(CString)(_T("全部汇总")));
				}
				//填充除了样本汇总以外的其他分类方式的那些行。
				for(int i = 0;i < iCount3;i++)
				{
					position = iCount+i;
					resTable.GetCell(14,position,id);
					if(id == 0)
					{
						resTable.GetCell(15,position,id);
						if(id == 0)
						{
							resTable.GetCell(16,position,id);
							iterMap = BankMap.find(id);
							if(iterMap!= BankMap.end())
								Name = iterMap->second;
							else
								Name = Tx::Core::Con_strInvalid;
							resTable.SetCell(1,position,Name);
							resTable.SetCell(2,position,(CString)(_T("托管银行")));
							continue;
						}
						else
						{
							iterMap = CompanyMap.find(id);
							if(iterMap!= CompanyMap.end())
								Name = iterMap->second;
							else
								Name = Tx::Core::Con_strInvalid;
							resTable.SetCell(1,position,Name);
							resTable.SetCell(2,position,(CString)(_T("基金公司")));
							continue;
						}					
					}
					else
					{
						iterMap = StyleMap.find(id);
						if(iterMap!= StyleMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,position,Name);
						resTable.SetCell(2,position,(CString)(_T("基金风格")));						
					}			
				}
			}
			else//只有单只基金和汇总的情况
			{				
				for(int k = 0;k < isize;k++)
				{
					position = iCount+k;
					resTable.SetCell(1,position,(CString)(_T("样本汇总")));
					resTable.SetCell(2,position,(CString)(_T("全部汇总")));			
				}		
			}
		}	
		//把报告年份和报告期转化成CString类型,所以增加一列
		resTable.InsertCol(5,Tx::Core::dtype_val_string);//str报告期	
		CString stryear,strdate;
		int tempdate,iyear;
		int tempCount;
		int n = 0;
		if(!(iType2 & 1))
		{
			resTable.DeleteRow(0,iCount);
			tempCount = iCount2;
			n = 0;
		}
		else
		{
			for(int i = 0;i < iCount;i++)
			{
				//填写报告期
				resTable.GetCell(4,i,tempdate);
				iyear = tempdate/10000;
				stryear.Format(_T("%d"),iyear);
				if(tempdate%10000 == 630)
					strdate = stryear + _T("年") + _T("中报");
				if(tempdate%10000 == 1231)
					strdate = stryear + _T("年") + _T("年报");

				resTable.SetCell(5,i,strdate);			
			}
			tempCount = iCount1;	
			n = iCount;
		}
		double proportion,Value;
		std::unordered_map<int,double>::iterator iterMap2;
		int tempId;
		double dtradeShare,dCount;
		for(;n < tempCount;n++)
		{
			//填写报告期
			resTable.GetCell(4,n,tempdate);
			iyear = tempdate/10000;
			stryear.Format(_T("%d"),iyear);
			if(tempdate%10000 == 630)
				strdate = stryear + _T("年") + _T("中报");
			if(tempdate%10000 == 1231)
				strdate = stryear + _T("年") + _T("年报");

			//把样本汇总的基金净值放到resTable
			if(iType >=17 && n >= tempCount - iTradeId)
			{
				iterMap2 = dDataMap.find(tempdate);
				if(iterMap2 != dDataMap.end())
				{
					equity2 = iterMap2->second;
					resTable.SetCell(6,n,equity2);
					//计算比例					
					resTable.GetCell(11,n,Value);//取得股票的市值
					if(equity2 < 0 || Value < 0)
						proportion = Tx::Core::Con_doubleInvalid;
					else 
						proportion = Value*100/equity2;			
					resTable.SetCell(13,n,proportion);	
					//计算流通市值比例；
					resTable.GetCell(8,n,tempId);
					GetSecurityNow(tempId);
					if(m_pSecurity != NULL)
					{
						dtradeShare = m_pSecurity->GetTradableShare();
						resTable.GetCell(12,n,dCount);
						if(dtradeShare < 0 || dCount < 0)
							dtradeShare = Tx::Core::Con_doubleInvalid;
						else
							dtradeShare = dCount/dtradeShare;
						resTable.SetCell(14,n,dtradeShare);
					}
				}
			}
			else
			{
				//计算比例
				resTable.GetCell(6,n,equity2);//取得资产净值
				resTable.GetCell(11,n,Value);//取得股票的市值
				if(equity2 < 0 || Value < 0)
					proportion = Tx::Core::Con_doubleInvalid;
				else 
					proportion = Value*100/equity2;			
				resTable.SetCell(13,n,proportion);	
				//计算流通比例；
				resTable.GetCell(8,n,tempId);
				GetSecurityNow(tempId);
				if(m_pSecurity != NULL)
				{
					dtradeShare = m_pSecurity->GetTradableShare();
					resTable.GetCell(12,n,dCount);
					if(dtradeShare <0 || dCount < 0)
						dtradeShare = Tx::Core::Con_doubleInvalid;
					else
						dtradeShare = dCount/dtradeShare;
					//dtradeShare = dCount/dtradeShare;
					resTable.SetCell(14,n,dtradeShare);
				}
			}
			resTable.SetCell(5,n,strdate);
		}
		resTable.DeleteCol(15,3);
	}
	else
	{	
		//添加进度条
		prw.SetPercent(pid,0.7);
		//把报告年份和报告期转化成CString类型,所以增加一列
		resTable.InsertCol(5,Tx::Core::dtype_val_string);//str报告期	
		CString stryear,strdate;
		int tempdate,iyear;
		//填写报告期
		for(int k = 0;k < (int)resTable.GetRowCount();k++)
		{
			resTable.GetCell(4,k,tempdate);
			iyear = tempdate/10000;
			stryear.Format(_T("%d"),iyear);
			if(tempdate%10000 == 630)
				strdate = stryear + _T("年") + _T("中报");
			if(tempdate%10000 == 1231)
				strdate = stryear + _T("年") + _T("年报");
			resTable.SetCell(5,k,strdate);
		}

	}
	resTable.DeleteCol(8);//删除股票ID
	resTable.DeleteCol(3);//删除基金ID和报告期
	resTable.Arrange();
	MultiSortRule multisort2;
	multisort2.AddRule(3,false);
	multisort2.AddRule(1,true);	
	multisort2.AddRule(6,true);
	resTable.SortInMultiCol(multisort2);
	resTable.Arrange();
	resTable.DeleteCol(3);
	
	////下面是后来加的，是通过持股类型筛选的。
	//std::set<byte> typeNo;
 //  	typeNo.insert(3);
	//int colCount = tempTable.GetColCount();
	//UINT* nColArray4 = new UINT[colCount];
	//for(int i = 0; i < colCount; i++)
	//{
	//	nColArray4[i] = i;
	//}
	//resTable.EqualsAt(tempTable,nColArray4,colCount,6,typeNo);
//#ifdef _DEBUG
//	strTable=tempTable.TableToString();
//	Tx::Core::Commonality::String().StringToClipboard(strTable);
//#endif
	//resTable.Clear();
	//resTable.CopyColumnInfoFrom(tempTable);
	//resTable.Clone(tempTable);
	//resTable.Sort(9);
	//resTable.Arrange();
	//resTable.DeleteCol(10);//删除股票ID                暂时删掉，以后还要用
	//resTable.DeleteCol(7);//基金定期报告ID
 //   resTable.DeleteCol(6);//持股类型                   暂时删掉，以后还要用
	//resTable.DeleteCol(4);//删除合并的报告年份和报告期
	//resTable.DeleteCol(3);//删除基金ID    

	//添加进度条
	/*if(iType2 > 17)
	{
		resTable.SetSortRange(0,resTable.GetRowCount()-2);
	}*/
	prw.SetPercent(pid,1.0);
	return true;
}

//买入卖出股票
//add by lijw 2008-02-25
bool TxFund::StatBuyInSellOutStock(		
								   Tx::Core::Table_Indicator &resTable,
								   std::vector<int>	iSecurityId,
								   std::vector<int>	iDate,
								   int		iType
								   )

{
	//添加进度条
//	ProgressWnd* pwd=Tx::Core::ProgressWnd::GetInstance();
	ProgressWnd prw;
	UINT pid=prw.AddItem(1,_T("买入卖出统计..."),0.0);
	prw.Show(1000);
	m_txTable.Clear();//这是引用别人的成员变量，
	//从T_FUND_NET_ASSET_CHANGE里取基金定期报告ID，基金ID，报告年份，报告期
	//默认的返回值状态
	bool result = false;
	//清空数据
	m_txTable.Clear();
	//准备样本集参数列
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//基金ID
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//报告年份
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//报告期
	const int indicatorIndex = 1;
	long iIndicator[indicatorIndex] = 
	{
		30901091	//期初基金净值
	};
	UINT varCfg[3];			//参数配置
	int varCount=3;			//参数个数
	for (int i = 0; i < indicatorIndex; i++)
	{
		int tempIndicator = iIndicator[i];

		GetIndicatorDataNow(tempIndicator);
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
			return FALSE;
		}
	}
	UINT iColCount = m_txTable.GetColCount();
	UINT* nColArray = new UINT[iColCount];
	for(int i = 0; i < (int)iColCount; i++)
	{
		nColArray[i] = i;
	}
	result = m_pLogicalBusiness->GetData(m_txTable,true);
#ifdef _DEBUG
	CString strTable=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//把交易实体ID转化为基金ID
	std::vector<int> iFundId;
	std::vector<int>::iterator iter;
	long fundId;
	std::vector<int> tempTradeV;
	for (iter = iSecurityId.begin();iter != iSecurityId.end();++iter)
	{
		GetSecurityNow(*iter);
		if (m_pSecurity != NULL)
		{
			fundId = m_pSecurity->GetSecurity1Id(*iter);
			if(find(iFundId.begin(),iFundId.end(),fundId)==iFundId.end())
			{
				iFundId.push_back(fundId);
				tempTradeV.push_back(*iter);
			}			
		}
	}
	iSecurityId.clear();
	iSecurityId.assign(tempTradeV.begin(),tempTradeV.end());
	Tx::Core::Table_Indicator tempTable;
	tempTable.CopyColumnInfoFrom(m_txTable);
	//根据基金ID进行筛选
	m_txTable.EqualsAt(tempTable,nColArray,iColCount,0,iFundId);
	//为了进行年度和报告期的筛选，所以把报告年份和报告期和为一列。
	TransReportDateToNormal2(tempTable,1);
#ifdef _DEBUG
	strTable=tempTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	m_txTable.Clear();
	m_txTable.CopyColumnInfoFrom(tempTable);
	//进行年度和报告期的筛选
	tempTable.EqualsAt(m_txTable,nColArray,iColCount-1,1,iDate);
	delete nColArray;
	nColArray = NULL;
	if (m_txTable.GetRowCount() == 0)
	{
		//添加进度条
		prw.SetPercent(pid,1.0);
		return false;
	}	
#ifdef _DEBUG
	strTable=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif

	//从T_buy_in_STOCK_twoyear取其他的数据
	Tx::Core::Table_Indicator BuyTable;
	//准备样本集参数列
	BuyTable.AddParameterColumn(Tx::Core::dtype_int4);//基金ID
	BuyTable.AddParameterColumn(Tx::Core::dtype_int4);//报告年份
	BuyTable.AddParameterColumn(Tx::Core::dtype_int4);//报告期
	BuyTable.AddParameterColumn(Tx::Core::dtype_int4);//序号
	const int indicatorIndex3 =3;
	long iIndicator3[indicatorIndex3] = 
	{
		30901334,	//基金定期报告ID//这是T_buy_in_STOCK表里
		30901107,	//股票ID
		30901108	//累计买入金额
	};
	UINT varCfg3[4];			//参数配置
	int varCount3=4;			//参数个数
	for (int i = 0; i < indicatorIndex3; i++)
	{
		int tempIndicator = iIndicator3[i];

		GetIndicatorDataNow(tempIndicator);
		if (m_pIndicatorData==NULL)
		{ return false; }
		varCfg3[0]=0;
		varCfg3[1]=1;
		varCfg3[2]=2;
		varCfg3[3]=3;
		result = m_pLogicalBusiness->SetIndicatorIntoTable(
			m_pIndicatorData,	//指标
			varCfg3,				//参数配置
			varCount3,			//参数个数
			BuyTable	//计算需要的参数传输载体以及计算后结果的载体
			);
		if(result==false)
		{
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false;
		}
	}
	result = m_pLogicalBusiness->GetData(BuyTable,true);
    if(BuyTable.GetRowCount() == 0 || result == false)
	{
		//添加进度条
		prw.SetPercent(pid,1.0);
		return false;
	}
#ifdef _DEBUG
	strTable=BuyTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	UINT iColCount3 = BuyTable.GetColCount();
	UINT* nColArray3 = new UINT[iColCount3];
	for(int i = 0; i < (int)iColCount3; i++)
	{
		nColArray3[i] = i;
	}
	tempTable.Clear();
    tempTable.CopyColumnInfoFrom(BuyTable);	
	BuyTable.EqualsAt(tempTable,nColArray3,iColCount3,0,iFundId);
	//为了后面的计算方便，所以把报告年份和报告期和为一列。
	TransReportDateToNormal2(tempTable,1);
	resTable.CopyColumnInfoFrom(tempTable);
	tempTable.EqualsAt(resTable,nColArray3,iColCount3-1,1,iDate);
	delete nColArray3;
	nColArray3 = NULL;
	if(resTable.GetRowCount() == 0)
	{
		//添加进度条
		prw.SetPercent(pid,1.0);
		return false;
	}	
#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//取出基金定期报告ID，把它放到vector里，为下面的表的连接作准备
	std::set<int> ReportV;
	for(int i = 0;i < (int)resTable.GetRowCount();i++)
	{
		int reportid ;
		resTable.GetCell(3,i,reportid);
		ReportV.insert(reportid);
	}	
	 //把resTable作为最终的结构表。
	//添加进度条
	prw.SetPercent(pid,0.3);
	//为增加基金的交易实体ID和名称，代码作准备
	resTable.InsertCol(0,Tx::Core::dtype_int4);//基金交易实体ID
	resTable.InsertCol(1,Tx::Core::dtype_val_string);//基金名称
	resTable.InsertCol(2,Tx::Core::dtype_val_string);//基金代码
	//为把m_txTable的内容放到resTable表里作准备。
	resTable.InsertCol(5,Tx::Core::dtype_decimal);//资产净值
	//把股票ID（也既是内码）转换成股票名称和代码
	resTable.InsertCol(9,Tx::Core::dtype_val_string);//股票strcode
	resTable.InsertCol(10,Tx::Core::dtype_val_string);//股票strname
	resTable.InsertCol(11,Tx::Core::dtype_int4);//基金只数
	resTable.AddCol(Tx::Core::dtype_decimal);//13（买入的）占期初净值比
	resTable.AddCol(Tx::Core::dtype_decimal);//14（卖出的）金额
	resTable.AddCol(Tx::Core::dtype_decimal);//15（卖出的）占期初净值比
	resTable.AddCol(Tx::Core::dtype_decimal);//16净买入金额
	//把期初基金净值加到resTable里
	int fundid1,tempdate4,fundid5,tempdate5;
	fundid1 = tempdate4 = fundid5 = tempdate5 = 0;
	int stockId;
	double equity2;
	double dBuyMoney,dProportion;
	CString strname,strcode;
	std::vector<UINT>vecID1,vecID2,vecID3;
	std::vector<UINT>::iterator iteID1,iteID2,iteID3;
	for(int m = 0;m < (int)resTable.GetRowCount();m++)
	{
		resTable.GetCell(3,m,fundid1);
		resTable.GetCell(4,m,tempdate4);
		if(fundid1 == fundid5 && tempdate4 == tempdate5)
		{
#ifdef _DEBUG
			if(vecID3.size() != 1)
			{
				AfxMessageBox(_T("数据出现错误"));
				//添加进度条
				prw.SetPercent(pid,1.0);
				return false;
			}
#endif
			iteID3 = vecID3.begin();
			m_txTable.GetCell(2,*iteID3,equity2);
			resTable.SetCell(5,m,equity2);
			//占期初净值比，
			resTable.GetCell(12,m,dBuyMoney);
			if(equity2 <= 0)
				dProportion = Tx::Core::Con_doubleInvalid;
			else
				dProportion = dBuyMoney/equity2*100;
			resTable.SetCell(13,m,dProportion);	
			//添加名称和代码
			resTable.GetCell(8,m,stockId);
			GetSecurityNow(stockId);
			if(m_pSecurity == NULL)
				continue;
			strname = m_pSecurity->GetName();
			strcode = m_pSecurity->GetCode();
			resTable.SetCell(9,m,strcode);
			resTable.SetCell(10,m,strname);			
		}
		else
		{
			fundid5 = fundid1;
			tempdate5 = tempdate4;
			vecID1.clear();
			vecID2.clear();
			vecID3.clear();
			m_txTable.Find(0,fundid1,vecID1);//查找相等的fundid
			m_txTable.Find(1,tempdate4,vecID2);//查找相等的报告期（合并后的）
			if (vecID1.size() == 0 || vecID2.size() == 0)
			{
				//该句代码是为了确保当查找的那两个字段，其中一个为空的情况。add by lijw 2008-09-09
				fundid1 = tempdate4 = fundid5 = tempdate5 = 0;
				continue;
			}
			for(iteID1 = vecID1.begin();iteID1 != vecID1.end();++iteID1)
			{

				for(iteID2 = vecID2.begin();iteID2 != vecID2.end();++iteID2)
				{
					if(*iteID1 == *iteID2)
					{
						vecID3.push_back(*iteID1);
						break;
					}
				}
			}
			//当找不到符合的记录时。
			if(vecID3.empty())
			{
				equity2 = Tx::Core::Con_doubleInvalid;
				resTable.SetCell(5,m,equity2);
				//占期初净值比，				
				dProportion = Tx::Core::Con_doubleInvalid;
				resTable.SetCell(13,m,dProportion);	
				//添加名称和代码
				resTable.GetCell(8,m,stockId);
				GetSecurityNow(stockId);
				if(m_pSecurity == NULL)
				{
					//该句代码是为了确保当查找的那两个字段，其中一个为空的情况。add by lijw 2008-09-09
					fundid1 = tempdate4 = fundid5 = tempdate5 = 0;
					continue;
				}
				strname = m_pSecurity->GetName();
				strcode = m_pSecurity->GetCode();
				resTable.SetCell(9,m,strcode);
				resTable.SetCell(10,m,strname);	
				//该句代码是为了确保当查找的那两个字段，其中一个为空的情况。add by lijw 2008-09-09
				fundid1 = tempdate4 = fundid5 = tempdate5 = 0;
				continue;
			}
#ifdef _DEBUG
			if(vecID3.size() != 1)
			{
				AfxMessageBox(_T("数据出现错误"));
				//添加进度条
				prw.SetPercent(pid,1.0);
				return false;
			}
#endif
			iteID3 = vecID3.begin();
			m_txTable.GetCell(2,*iteID3,equity2);
			resTable.SetCell(5,m,equity2);
			//占期初净值比，
			resTable.GetCell(12,m,dBuyMoney);
			if(equity2 <= 0)
				dProportion = Tx::Core::Con_doubleInvalid;
			else
				dProportion = dBuyMoney/equity2*100;
			resTable.SetCell(13,m,dProportion);	
			//添加名称和代码
			resTable.GetCell(8,m,stockId);
			GetSecurityNow(stockId);
			if(m_pSecurity == NULL)
				continue;
			strname = m_pSecurity->GetName();
			strcode = m_pSecurity->GetCode();
			resTable.SetCell(9,m,strcode);
			resTable.SetCell(10,m,strname);			
		}		
	}
#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//从T_sell_out_STOCK_twoyear取出数据
	Tx::Core::Table_Indicator SellTable,STempTable;
	//准备样本集参数列
	SellTable.AddParameterColumn(Tx::Core::dtype_int4);//基金ID
	SellTable.AddParameterColumn(Tx::Core::dtype_int4);//报告年份
	SellTable.AddParameterColumn(Tx::Core::dtype_int4);//报告期
	SellTable.AddParameterColumn(Tx::Core::dtype_int4);//序号
	const int indicatorIndex4 =3;
	long iIndicator4[indicatorIndex4] = 
	{
		30901335,	//基金定期报告ID//这是T_Sell_Ount_STOCK表里
		30901110,	//股票ID
		30901111	//累计卖出金额
	};
	UINT varCfg4[4];			//参数配置
	int varCount4=4;			//参数个数
	for (int i = 0; i < indicatorIndex4; i++)
	{
		int tempIndicator = iIndicator4[i];
		GetIndicatorDataNow(tempIndicator);
		if (m_pIndicatorData==NULL)
		{
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false; 
		}
		varCfg4[0]=0;
		varCfg4[1]=1;
		varCfg4[2]=2;
		varCfg4[3]=3;
		result = m_pLogicalBusiness->SetIndicatorIntoTable(
			m_pIndicatorData,	//指标
			varCfg4,				//参数配置
			varCount4,			//参数个数
			SellTable	//计算需要的参数传输载体以及计算后结果的载体
			);
		if(!result)
		{
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false;
		}
	}

	result = m_pLogicalBusiness->GetData(SellTable,true);

#ifdef _DEBUG
	strTable=SellTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	UINT iColCount4 = SellTable.GetColCount();
	UINT* nColArray4 = new UINT[iColCount4];
	for(int i = 0; i < (int)iColCount4; i++)
	{
		nColArray4[i] = i;
	}
	STempTable.CopyColumnInfoFrom(SellTable);
	SellTable.EqualsAt(STempTable,nColArray4,iColCount4,4,ReportV);
	
#ifdef _DEBUG
	strTable=STempTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//从T_sell_out_STOCK_twoyear找出与T_buy_in_STOCK_twoyear相对应的记录
	int jdID1,jdID2;//基金定期报告ID
	int stockTempId;//股票Id
	jdID1 = jdID2 = 0;
	double dequity3,dRate,dBuyTempMoney;
	double dSellMoney;
	std::vector<int> tempReV;
	Tx::Core::Table_Indicator TableOne;
	std::vector<UINT> vecStockD;
	std::vector<UINT>::iterator iteID;
	int iRemaintRow = 0;
	TableOne.CopyColumnInfoFrom(STempTable);
	//为了取得相应的结果，所以对resTable进行排序，分别按照基金定期报告ID、序号
	MultiSortRule multisort;
	multisort.AddRule(7,true);
	multisort.AddRule(6,true);
	resTable.SortInMultiCol(multisort);
#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	for(int j = 0;j < (int)resTable.GetRowCount();j++)
	{
		int No;
		resTable.GetCell(7,j,jdID1);
		if(jdID1 != jdID2)
		{
			if(TableOne.GetRowCount() != 0)
			{		
				resTable.GetCell(6,j-1,No);
				resTable.GetCell(5,j-1,dequity3);//取出基金净值
				iRemaintRow = TableOne.GetRowCount();
				//插入只有累计卖出金额的记录。
				TableOne.Sort(3);
				TableOne.Arrange();
				TransReportDateToNormal2(TableOne,1);
				int fundId,Rdate,tradeId;
                double dSellMoney2,dSellProportion;
                for(int k = 0;k < iRemaintRow;k++)
				{
					int pos = j + k;
					resTable.InsertRow(pos);
					TableOne.GetCell(0,k,fundId);
					TableOne.GetCell(1,k,Rdate);
					TableOne.GetCell(4,k,tradeId);
					//取出股票的名字和代码
					GetSecurityNow(tradeId);
					if(m_pSecurity == NULL)
						continue;
					strname = m_pSecurity->GetName();
					strcode = m_pSecurity->GetCode();
					TableOne.GetCell(5,k,dSellMoney2);//取出卖出金额
					resTable.SetCell(3,pos,fundId);
					resTable.SetCell(4,pos,Rdate);
					No = No + 1;
					resTable.SetCell(6,pos,No);
					resTable.SetCell(5,pos,dequity3);
					resTable.SetCell(8,pos,tradeId);
					resTable.SetCell(9,pos,strcode);
					resTable.SetCell(10,pos,strname);
					resTable.SetCell(14,pos,dSellMoney2);
					if(dequity3 <= 0)
						dSellProportion = Tx::Core::Con_doubleInvalid;
					else
						dSellProportion = dSellMoney2/dequity3*100;
					resTable.SetCell(15,pos,dSellProportion);
					resTable.SetCell(16,pos,-dSellMoney2);
				}
				j = j + iRemaintRow - 1;//把j跳过刚才添加的记录；
				jdID2 = 0;
				//把TableOne清空
				TableOne.Clear();
				TableOne.CopyColumnInfoFrom(STempTable);
				continue;
			}
			jdID2 = jdID1;
			resTable.GetCell(8,j,stockTempId);
			resTable.GetCell(5,j,dequity3);//取出基金净值
			resTable.GetCell(12,j,dBuyTempMoney);//取出累计买入金额
			tempReV.clear();
			tempReV.push_back(jdID1);
			STempTable.EqualsAt(TableOne,nColArray4,iColCount4,4,tempReV);
#ifdef _DEBUG
			strTable=TableOne.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
			std::vector<UINT> vecStockD;
			vecStockD.clear();
			TableOne.Find(5,stockTempId,vecStockD);
			std::vector<UINT>::iterator iteID; 
			//下面条件判断是为了说明基金定期报告ID相同的行中，没有两个相同的股票。
#ifdef _DEBUG
			if(vecStockD.size() > 1 )
			{
				AfxMessageBox(_T("数据1出现问题"));
				TRACE("%d",j);
				delete nColArray4;
				nColArray4 = NULL;
				//添加进度条
				prw.SetPercent(pid,1.0);
				return false;
			}
#endif
			//这是只有累计买入的数据，没有累计卖出的数据
			No = 1;
			if(vecStockD.size() == 0)
			{				
				resTable.GetCell(12,j,dBuyTempMoney);//取出买入金额
				resTable.SetCell(16,j,dBuyTempMoney);//填写净买入金额
				resTable.SetCell(6,j,No);
				continue;
			}
			//下面是既有累计买入的也有累计卖出的数据
			iteID = vecStockD.begin();
			TableOne.GetCell(6,*iteID,dSellMoney);//取出卖出金额
			resTable.SetCell(14,j,dSellMoney);
			if(dequity3 <= 0)
				dRate = Tx::Core::Con_doubleInvalid;
			else
				dRate = dSellMoney/dequity3*100;
			resTable.SetCell(15,j,dRate);
			resTable.SetCell(16,j,dBuyTempMoney - dSellMoney);
			resTable.SetCell(6,j,No);
			TableOne.DeleteRow(*iteID);
			TableOne.Arrange();			
		}
		else
		{
			resTable.GetCell(8,j,stockTempId);
			resTable.GetCell(6,j-1,No);
			No = No + 1;
			resTable.GetCell(5,j,dequity3);//取出基金净值
			resTable.GetCell(12,j,dBuyTempMoney);//取出累计买入金额
			vecStockD.clear();
			TableOne.Find(5,stockTempId,vecStockD);
#ifdef _DEBUG
			if(vecStockD.size() > 1 )
			{
				TRACE("%d",j);
				//添加进度条
				prw.SetPercent(pid,1.0);
				AfxMessageBox(_T("数据2出现问题"));
				delete nColArray4;
				nColArray4 = NULL;
				return false;
			}
#endif
			//这是只有累计买入的数据，没有累计卖出的数据
			if(vecStockD.size() == 0)
			{
				resTable.GetCell(12,j,dBuyTempMoney);//取出买入金额
				resTable.SetCell(16,j,dBuyTempMoney);//填写净买入金额
				resTable.SetCell(6,j,No);
				continue;
			}
			//下面是既有累计买入的也有累计卖出的数据
			iteID = vecStockD.begin();
			TableOne.GetCell(6,*iteID,dSellMoney);//取出卖出金额
			resTable.SetCell(14,j,dSellMoney);
			if(dequity3 <= 0)
				dRate = Tx::Core::Con_doubleInvalid;
			else
				dRate = dSellMoney/dequity3*100;
			resTable.SetCell(15,j,dRate);
			resTable.SetCell(16,j,dBuyTempMoney - dSellMoney);
			resTable.SetCell(6,j,No);
			TableOne.DeleteRow(*iteID);
			TableOne.Arrange();
		}

	}
	//这是判断resTable最后一行的记录所对应的只有累计卖出的情况
	//也即是TableOne剩下的那些记录。
	if(TableOne.GetRowCount() != 0)
	{
		int No,icount;
		icount = resTable.GetRowCount();
		resTable.GetCell(6,icount-1,No);
		resTable.GetCell(5,icount-1,dequity3);//取出基金净值
		iRemaintRow = TableOne.GetRowCount();
		//插入只有累计卖出金额的记录。
		TableOne.Sort(3);
		TableOne.Arrange();
		TransReportDateToNormal2(TableOne,1);
		int fundId,Rdate,tradeId;
		double dSellMoney2,dSellProportion;
		for(int k = 0;k < (int)TableOne.GetRowCount();k++)
		{
			resTable.AddRow();
			TableOne.GetCell(0,k,fundId);
			TableOne.GetCell(1,k,Rdate);
			TableOne.GetCell(4,k,tradeId);
			//取出股票的名字和代码
			GetSecurityNow(tradeId);
			if(m_pSecurity == NULL)
				continue;
			CString strname,strcode;
			strname = m_pSecurity->GetName();
			strcode = m_pSecurity->GetCode();
			TableOne.GetCell(5,k,dSellMoney2);//取出卖出金额
			resTable.SetCell(3,icount + k,fundId);
			resTable.SetCell(4,icount + k,Rdate);
			resTable.SetCell(5,icount + k,dequity3);
			No = No + 1;
			resTable.SetCell(6,icount + k,No);
			resTable.SetCell(8,icount + k,tradeId);
			resTable.SetCell(9,icount + k,strcode);
			resTable.SetCell(10,icount + k,strname);
			resTable.SetCell(14,icount + k,dSellMoney2);
			if(dequity3 <= 0)
				dSellProportion = Tx::Core::Con_doubleInvalid;
			else
				dSellProportion = dSellMoney2/dequity3*100;
			resTable.SetCell(15,icount + k,dSellProportion);
			resTable.SetCell(16,icount + k,-dSellMoney2);
		}
	}
	delete nColArray4;
	nColArray4 = NULL;

	//增加基金的交易实体ID和名称，代码
	std::vector<int>::iterator iterId;
	std::unordered_map<int,double> dDataMap;//存放临时的基金净值	
	std::vector<int>::iterator iterDate;
	for(iterId = iSecurityId.begin();iterId != iSecurityId.end();++iterId)
	{
		int icount = 1;
		int fundId;
		CString strName,strCode;
		GetSecurityNow(*iterId);
		if(m_pSecurity == NULL)
			continue;
		fundId = m_pSecurity->GetSecurity1Id();
		strName = m_pSecurity->GetName();
		strCode = m_pSecurity->GetCode();
		std::vector<UINT> vecInstiID;
		resTable.Find(3,fundId,vecInstiID);
		std::vector<UINT>::iterator iteID;
		for(iteID = vecInstiID.begin();iteID != vecInstiID.end();++iteID)
		{
			resTable.SetCell(0,*iteID,*iterId);
			resTable.SetCell(1,*iteID,strName);
			resTable.SetCell(2,*iteID,strCode);			
			resTable.SetCell(11,*iteID,icount);//基金支数。
		}
		if(iType >=16)
		{
			//把所有相同的报告期的基金净值加起来。这是为了样本汇总时作准备。			
			int itempdate;
			std::vector<UINT> TempCollect;
			//填充dDataMap，把各个报告期的基金净值设为零
			for(iterDate = iDate.begin();iterDate != iDate.end();++iterDate)
			{
				dDataMap.insert(std::make_pair(*iterDate,0));
			}
			for(iterDate = iDate.begin();iterDate != iDate.end();++iterDate)
			{
				itempdate = *iterDate;
				if(!TempCollect.empty())
					TempCollect.clear();
				resTable.Find(4,itempdate,TempCollect);//取得与报告期相同的纪录的位置。
				if(TempCollect.empty())
					continue;
				std::set<UINT> positionSet(vecInstiID.begin(),vecInstiID.end());
				iteID = TempCollect.begin();
				for(;iteID != TempCollect.end();++iteID)
				{
					if(positionSet.find(*iteID) != positionSet.end())
					{
						resTable.GetCell(5,*iteID,equity2);
						dDataMap[itempdate] += equity2; 
						break;
					}
					else
						continue;
				}
			}			
		}
	}
    int iType2 = iType;//保存原来的值.
	//这里增加样本分类选择
	if(iType >1)
	{	
		int iCount = resTable.GetRowCount();//累加之前resTable里的行数。		
		//为了进行分类统计。所以增加一列基金ID。
		resTable.AddCol(Tx::Core::dtype_int4);//基金风格
		resTable.AddCol(Tx::Core::dtype_int4);//基金管理公司ID
		resTable.AddCol(Tx::Core::dtype_int4);//基金托管银行ID		
		//添加进度条
		prw.SetPercent(pid,0.6);
		//这里增加样本分类选择
		m_txTable.Clear();
		tempTable.Clear();
		//准备样本集=第一参数列:F_FUND_ID,int型
		m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
		const int indicatorIndex1 = 3;
		long iIndicator1[indicatorIndex1] = 
		{
			//30001035,	//基金风格
			//modified by zhangxs 20091221---NewStyle
			30001232,	//基金风格New
			30001020,	//管理公司ID，
			30001021	//托管银行ID，
		};
		UINT varCfg1[1];			//参数配置
		int varCount1=1;			//参数个数
		for (int i = 0; i < indicatorIndex1; i++)
		{
			int tempIndicator = iIndicator1[i];
			GetIndicatorDataNow(tempIndicator);
			if (m_pIndicatorData==NULL)
			{
				//添加进度条
				prw.SetPercent(pid,1.0);
				return false; 
			}
			varCfg1[0]=0;
			result = m_pLogicalBusiness->SetIndicatorIntoTable(
				m_pIndicatorData,	//指标
				varCfg1,				//参数配置
				varCount1,			//参数个数
				m_txTable	//计算需要的参数传输载体以及计算后结果的载体
				);
			if(result==false)
			{
				//添加进度条
				prw.SetPercent(pid,1.0);
				return FALSE;
			}
		}
		result = m_pLogicalBusiness->GetData(m_txTable,true);
		if(result==false)
		{
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false;
		}
#ifdef _DEBUG
		strTable=m_txTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
		UINT iCol2=m_txTable.GetColCount();
		//复制所有列信息
		tempTable.CopyColumnInfoFrom(m_txTable);
		if(m_txTable.GetRowCount()==0)
		{
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false;
		}
		UINT* nColArray2 = new UINT[iCol2];
		for(UINT i=0;i<iCol2;i++)
			nColArray2[i]=i;
//#ifdef _SECURITYID_FOR_STAT_
		m_txTable.EqualsAt(tempTable,nColArray2,iCol2,0,iSecurityId);
//#else
//		m_txTable.EqualsAt(tempTable,nColArray2,iCol2,0,iFundId);
//#endif
		
		delete nColArray2;
		nColArray2 = NULL;
		//把基金风格、公司ID、托管银行ID放到resTable
		std::vector<UINT> vecInstiID2;
		std::vector<UINT>::iterator iteID;		
		int iStyle,iCompanyId,ibankId;
		int tempfundId;
		int position;
		int icount = tempTable.GetRowCount();
		for(int j = 0;j < icount;j++)
		{
			tempTable.GetCell(0,j,tempfundId);
			tempTable.GetCell(1,j,iStyle);
			tempTable.GetCell(2,j,iCompanyId);
			tempTable.GetCell(3,j,ibankId);
			//把基金ID和交易实体ID对应起来。并且把数据放到表里。
			if(!(vecInstiID2.empty()))
				vecInstiID2.clear();
//#ifdef _SECURITYID_FOR_STAT_
			resTable.Find(0,tempfundId,vecInstiID2);
//#else
//			resTable.Find(3,tempfundId,vecInstiID2);
//#endif
			for(iteID = vecInstiID2.begin();iteID != vecInstiID2.end();++iteID)
			{
				position = *iteID;
				resTable.SetCell(17,position,iStyle);//基金风格
				resTable.SetCell(18,position,iCompanyId);//管理公司ID
				resTable.SetCell(19,position,ibankId);//托管银行ID
			}
		}		
		std::vector<int> ColVector;		//根据哪些列进行统计  
		if(!(iType & 1))
			iType += 1;//假设都选择选择了单只基金那一项分类方式。
		if(iType & 2)
			ColVector.push_back(18);
		if(iType & 4)
			ColVector.push_back(17);
		if(iType & 8)
			ColVector.push_back(19);
		std::vector<int> IntCol;			//需要相加的整型列
		//		IntCol.push_back(6);
		std::vector<int> DoubleCol;	//需要相加的double列
//		DoubleCol.push_back(5);     //由于已经单独把基金净值的列，传进AddUpRow，并且在该函数中也进行了计算。
		DoubleCol.push_back(12);
		DoubleCol.push_back(14);
		/*if (iType2 < 17)
		DoubleCol.push_back(5);*/
		/*for(int j = 6;j < 17;j++)
		{
		DoubleCol.push_back(j);
		j++;
		}*/
		int iTradeId = 8;//之所以这样做，是为了让它带回汇总样本统计的纪录数。
		AddUpRow(resTable,iType,ColVector,IntCol,DoubleCol,iDate,4,5,3,iTradeId,12,6,true);
		//		std::vector<int>::size_type isize = iDate.size();
		int iCount1 = resTable.GetRowCount();//累加以后resTable里的行数。
		int iCount2,iCount3;
		iCount2 = iCount1 - iCount;//这是增加的行数 样本汇总和其他分类方式的行数。
		std::unordered_map<int,CString> CompanyMap;
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_INSTITUTIONID_TO_SHORTNAME,CompanyMap);
		std::unordered_map<int,CString> StyleMap;
		//modified by zhangxs 20091221---NewStyle
		//TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX,StyleMap);
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX_NEW,StyleMap);
		std::unordered_map<int,CString> BankMap;
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_BANK,BankMap);
		std::unordered_map<int,CString>::iterator iterMap;
		int id;
		CString Name;	
		if(iType < 17)//没有样本汇总的情况
		{
			for(int m = 0;m < iCount2;m++)
			{
				position = iCount+m;
				resTable.GetCell(17,position,id);
				if(id == 0)
				{
					resTable.GetCell(18,position,id);
					if(id == 0)
					{
						resTable.GetCell(19,position,id);
						iterMap = BankMap.find(id);
						if(iterMap!= BankMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,position,Name);
						resTable.SetCell(2,position,(CString)(_T("托管银行")));
						continue;
					}
					else
					{
						iterMap = CompanyMap.find(id);
						if(iterMap!= CompanyMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,position,Name);
						resTable.SetCell(2,position,(CString)(_T("基金公司")));
						continue;
					}					
				}
				else
				{
					iterMap = StyleMap.find(id);
					if(iterMap!= StyleMap.end())
						Name = iterMap->second;
					else
						Name = Tx::Core::Con_strInvalid;
					resTable.SetCell(1,position,Name);
					resTable.SetCell(2,position,(CString)(_T("基金风格")));
				}
			}				
		}
		else//不但有单只基金和样本汇总，还有其他的分类方式
		{
			int isize = iTradeId;//这是样本汇总的行数.
			iCount3 = iCount2 - isize;//除了样本汇总以外的其他分类方式的行数
			if(iCount3 > 0)//表示除了样本汇总以外还有其他的分类的方式
			{
				//填充样本汇总的那些行
				for(int k = 0;k < isize;k++)
				{
					resTable.SetCell(1,iCount+iCount3+k,(CString)(_T("样本汇总")));
					resTable.SetCell(2,iCount+iCount3+k,(CString)(_T("全部汇总")));
				}
				//填充除了样本汇总以外的其他分类方式的那些行。
				for(int i = 0;i < iCount3;i++)
				{
					position = iCount+i;
					resTable.GetCell(17,position,id);
					if(id == 0)
					{
						resTable.GetCell(18,position,id);
						if(id == 0)
						{
							resTable.GetCell(19,position,id);
							iterMap = BankMap.find(id);
							if(iterMap!= BankMap.end())
								Name = iterMap->second;
							else
								Name = Tx::Core::Con_strInvalid;
							resTable.SetCell(1,position,Name);
							resTable.SetCell(2,position,(CString)(_T("托管银行")));
							continue;
						}
						else
						{
							iterMap = CompanyMap.find(id);
							if(iterMap!= CompanyMap.end())
								Name = iterMap->second;
							else
								Name = Tx::Core::Con_strInvalid;
							resTable.SetCell(1,position,Name);
							resTable.SetCell(2,position,(CString)(_T("基金公司")));
							continue;
						}					
					}
					else
					{
						iterMap = StyleMap.find(id);
						if(iterMap!= StyleMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,position,Name);
						resTable.SetCell(2,position,(CString)(_T("基金风格")));						
					}			
				}
			}
			else//只有单只基金和汇总的情况
			{				
				for(int k = 0;k < isize;k++)
				{
					position = iCount+k;
					resTable.SetCell(1,position,(CString)(_T("样本汇总")));
					resTable.SetCell(2,position,(CString)(_T("全部汇总")));			
				}		
			}
		}	
		//把报告年份和报告期转化成CString类型,所以增加一列
		resTable.InsertCol(5,Tx::Core::dtype_val_string);//str报告期	
		CString stryear,strdate;
		int tempdate,iyear;
		int tempCount;
		int n = 0;
		if(!(iType2 & 1))
		{
			resTable.DeleteRow(0,iCount);
			tempCount = iCount2;
			n = 0;
		}
		else
		{
			for(int i = 0;i < iCount;i++)
			{
				//填写报告期
				resTable.GetCell(4,i,tempdate);
				iyear = tempdate/10000;
				stryear.Format(_T("%d"),iyear);
				if(tempdate%10000 == 630)
					strdate = stryear + _T("年") + _T("中报");
				if(tempdate%10000 == 1231)
					strdate = stryear + _T("年") + _T("年报");

				resTable.SetCell(5,i,strdate);			
			}
			tempCount = iCount1;	
			n = iCount;
		}
		double proportion1,proportion2,Value1,Value2,Value3;
		std::unordered_map<int,double>::iterator iterMap2;
		for(;n < tempCount;n++)
		{
			//填写报告期
			resTable.GetCell(4,n,tempdate);
			iyear = tempdate/10000;
			stryear.Format(_T("%d"),iyear);
			if(tempdate%10000 == 630)
				strdate = stryear + _T("年") + _T("中报");
			if(tempdate%10000 == 1231)
				strdate = stryear + _T("年") + _T("年报");

			//把样本汇总的基金净值放到resTable
			if(iType >=17 && n >= tempCount - iTradeId)
			{
				iterMap2 = dDataMap.find(tempdate);
				if(iterMap2 != dDataMap.end())
				{
					equity2 = iterMap2->second;
					resTable.SetCell(6,n,equity2);
					//计算比例					
					resTable.GetCell(13,n,Value1);//取得股票累计买入金额
					resTable.GetCell(15,n,Value2);//取得股票累计卖出金额
					if(equity2 <= 0 )
					{
						proportion1 = Tx::Core::Con_doubleInvalid;
						proportion2 = Tx::Core::Con_doubleInvalid;
						resTable.SetCell(14,n,proportion1);	
						resTable.SetCell(16,n,proportion2);	
						//计算净买入金额
						Value3 = Tx::Core::Con_doubleInvalid;
					}
					else
					{
						if(Value1 < 0 && Value2 >0)
						{
							proportion1 = Tx::Core::Con_doubleInvalid;
							resTable.SetCell(14,n,proportion1);	
							proportion2 = Value2*100/equity2;	
							resTable.SetCell(16,n,proportion2);	
							//计算净买入金额
							Value3 = -Value2;
						}
						else if(Value2 < 0 && Value1 >0)
						{
							proportion2 = Tx::Core::Con_doubleInvalid;
							resTable.SetCell(16,n,proportion2);	
							proportion1 = Value1*100/equity2;	
							resTable.SetCell(14,n,proportion1);	
							Value3 = Value1;
						}
						else
						{
							proportion2 = Value2*100/equity2;	
							resTable.SetCell(16,n,proportion2);	
							proportion1 = Value1*100/equity2;	
							resTable.SetCell(14,n,proportion1);	
							Value3 = Value1 - Value2;
						}
					}
					//填写净买入金额
					resTable.SetCell(17,n,Value3);
				}
			}
			else
			{
				//计算比例
				resTable.GetCell(6,n,equity2);//取得资产净值
				resTable.GetCell(13,n,Value1);//取得股票累计买入金额
				resTable.GetCell(15,n,Value2);//取得股票累计卖出金额
				if(equity2 <= 0 )
				{
					proportion1 = Tx::Core::Con_doubleInvalid;
					proportion2 = Tx::Core::Con_doubleInvalid;
					resTable.SetCell(14,n,proportion1);	
					resTable.SetCell(16,n,proportion2);	
					//计算净买入金额
					Value3 = Tx::Core::Con_doubleInvalid;
				}
				else
				{
					if(Value1 < 0 && Value2 >0)
					{
						proportion1 = Tx::Core::Con_doubleInvalid;
						resTable.SetCell(14,n,proportion1);	
						proportion2 = Value2*100/equity2;	
						resTable.SetCell(16,n,proportion2);	
						//计算净买入金额
						Value3 = -Value2;
					}
					else if(Value2 < 0 && Value1 >0)
					{
						proportion2 = Tx::Core::Con_doubleInvalid;
						resTable.SetCell(16,n,proportion2);	
						proportion1 = Value1*100/equity2;	
						resTable.SetCell(14,n,proportion1);	
						Value3 = Value1;
					}
					else
					{
						proportion2 = Value2*100/equity2;	
						resTable.SetCell(16,n,proportion2);	
						proportion1 = Value1*100/equity2;	
						resTable.SetCell(14,n,proportion1);	
						Value3 = Value1 - Value2;
					}
				}
				//填写净买入金额
				resTable.SetCell(17,n,Value3);
			}
			resTable.SetCell(5,n,strdate);
		}
		resTable.DeleteCol(18,3);
	}
	else
	{	
		//添加进度条
		prw.SetPercent(pid,0.7);
		//把报告年份和报告期转化成CString类型,所以增加一列
		resTable.InsertCol(5,Tx::Core::dtype_val_string);//str报告期
		//把报告年份和报告期转化为CString类型
		int tempdate2,tempdate3;
		CString strdate,stryear,strreport;
		for(int m = 0;m < (int)resTable.GetRowCount();m++)
		{
			resTable.GetCell(4,m,tempdate2);
			tempdate3 =(int)tempdate2/10000;
			stryear.Format(_T("%d"),tempdate3);
			tempdate2 = tempdate2%10000;
			if(tempdate2 == 630)
				strdate = stryear + _T("年") + _T("中报");
			else
				strdate = stryear + _T("年") + _T("年报");
			resTable.SetCell(5,m,strdate);
		}
	}
#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	resTable.DeleteCol(9);//删除股票ID
	resTable.DeleteCol(8);//基金定期报告ID
	resTable.DeleteCol(3);//删除基金ID和报告期
	resTable.Arrange();
	MultiSortRule multisort2;
	multisort2.AddRule(3,false);
	multisort2.AddRule(1,true);	
	multisort2.AddRule(6,true);
	resTable.SortInMultiCol(multisort2);
	resTable.Arrange();
	resTable.DeleteCol(3);
	/*if(iType2 > 17)
	{
		resTable.SetSortRange(0,resTable.GetRowCount()-2);
	}*/
	//添加进度条
	prw.SetPercent(pid,1.0);
	return true;


}


//持债统计 add by lijw 2008-02-27
//重仓持债统计T_BOND_HOLDING_TOP_FIVE_BOND
bool TxFund::StatBondHoldingTopFive(
									Tx::Core::Table_Indicator &resTable,
									std::vector<int>	iSecurityId,
									std::vector<int>	iDate,
									int		iType
									)
{
	//添加进度条
//	ProgressWnd* pwd=Tx::Core::ProgressWnd::GetInstance();
	ProgressWnd prw;
	UINT pid=prw.AddItem(1,_T("重仓债券统计..."),0.0);
	prw.Show(1000);
	m_txTable.Clear();//这是引用别人的成员变量，
	//从T_ASSET_ALLOCATION_twoyear里取基金定期报告ID，基金ID，报告年份，报告期
	//默认的返回值状态
	bool result = false;
	//清空数据
	m_txTable.Clear();
	//准备样本集参数列
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//基金ID
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//报告年份
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//报告期
	const int indicatorIndex = 2;
	long iIndicator[indicatorIndex] = 
	{
		30901332,	//基金定期报告ID//这是T_ASSET_ALLOCATION表里
		30901140	//资产净值
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
			return FALSE;
		}

	}
	UINT iColCount = m_txTable.GetColCount();
	UINT* nColArray = new UINT[iColCount];
	for(int i = 0; i < (int)iColCount; i++)
	{
		nColArray[i] = i;
	}
	result = m_pLogicalBusiness->GetData(m_txTable,true);
#ifdef _DEBUG
	CString strTable=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//把交易实体ID转化为基金ID
	std::vector<int> iSecurity1Id;
	std::vector<int>::iterator iterId;
	//if(!TransObjectToSecIns(iSecurityId,iSecurity1Id,1))
	//	return false;
	int tempFundId;
	std::vector<int> tempTradeV;
	for(iterId = iSecurityId.begin();iterId != iSecurityId.end();++iterId)
	{
		GetSecurityNow(*iterId);
		if(m_pSecurity != NULL)
		{
			tempFundId = m_pSecurity->GetSecurity1Id();
			if(find(iSecurity1Id.begin(),iSecurity1Id.end(),tempFundId) == iSecurity1Id.end())
			{
				iSecurity1Id.push_back(tempFundId);
				tempTradeV.push_back(*iterId);
			}
		}
	}
	iSecurityId.clear();
	iSecurityId.assign(tempTradeV.begin(),tempTradeV.end());
	Tx::Core::Table_Indicator tempTable;
	tempTable.CopyColumnInfoFrom(m_txTable);
	//根据基金ID进行筛选
	m_txTable.EqualsAt(tempTable,nColArray,iColCount,0,iSecurity1Id);
	//为了进行年度和报告期的筛选，所以把报告年份和报告期和为一列。
	TransReportDateToNormal2(tempTable,1);
#ifdef _DEBUG
	strTable=tempTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	m_txTable.Clear();
	m_txTable.CopyColumnInfoFrom(tempTable);
	//进行年度和报告期的筛选
	tempTable.EqualsAt(m_txTable,nColArray,iColCount-1,1,iDate);
#ifdef _DEBUG
	strTable=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//取出基金定期报告ID，把它放到vector里，为和从视图T_STOCK_HOLDING_TOP_TEN_twoyear取得数据进行连接作准备。
	std::vector<int> ReportV;
	int reportid ;
	for(int i = 0;i < (int)m_txTable.GetRowCount();i++)
	{		
		m_txTable.GetCell(2,i,reportid);
		ReportV.push_back(reportid);
	}
	delete nColArray;
	nColArray = NULL;
	//从T_BOND_HOLDING_TOP_FIVE_BOND_twoyear取其他的数据
	Tx::Core::Table_Indicator BondTable;
	//准备样本集参数列
	BondTable.AddParameterColumn(Tx::Core::dtype_int4);//基金ID
	BondTable.AddParameterColumn(Tx::Core::dtype_int4);//报告年份
	BondTable.AddParameterColumn(Tx::Core::dtype_int4);//报告期
	BondTable.AddParameterColumn(Tx::Core::dtype_int4);//序号
	// zway 2010-07-01 for mantis #1663//
	//const int indicatorIndex3 = 5;
	//long iIndicator3[indicatorIndex3] = 
	//{
	//	30901352,	//基金定期报告ID
	//	30901250,	//市值排名
	//	30901247,	//债券ID（此时是交易实体ID）
	//	30901248,	//债券市值
	//	30901249	//所占比例（即是占净值比例）
	//};
	const int indicatorIndex3 = 7;		// // zway 2010-07-01 for mantis #1663 //
	long iIndicator3[indicatorIndex3] = 
	{
		30901352,	//基金定期报告ID
		30901250,	//市值排名
		30901247,	//债券ID（此时是交易实体ID）
		30901248,	//债券市值
		30901249,	//所占比例（即是占净值比例）
		30901338,	//债券代码的指数Id
		30901339	//债券名称的指标Id
	};
	UINT varCfg3[4];			//参数配置
	int varCount3=4;			//参数个数
	for (int i = 0; i < indicatorIndex3; i++)
	{
		int tempIndicator = iIndicator3[i];

		GetIndicatorDataNow(tempIndicator);
		if (m_pIndicatorData==NULL)
		{ return false; }
		varCfg3[0]=0;
		varCfg3[1]=1;
		varCfg3[2]=2;
		varCfg3[3]=3;
		result = m_pLogicalBusiness->SetIndicatorIntoTable(
			m_pIndicatorData,	//指标
			varCfg3,				//参数配置
			varCount3,			//参数个数
			BondTable	//计算需要的参数传输载体以及计算后结果的载体
			);
		if(result==false)
		{ 
			return FALSE;
		}

	}

	result = m_pLogicalBusiness->GetData(BondTable,true);	
	UINT iColCount3 = BondTable.GetColCount();
	UINT* nColArray3 = new UINT[iColCount3];
	for(int i = 0; i < (int)iColCount3; i++)
	{
		nColArray3[i] = i;
	}
	resTable.CopyColumnInfoFrom(BondTable);
	BondTable.EqualsAt(resTable,nColArray3,iColCount3,4,ReportV);
	delete nColArray3 ;
	nColArray3 = NULL;
#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//为了把他们转化成CString类型，所以把报告年份和报告期和为一列。
	TransReportDateToNormal2(resTable,1);
	resTable.DeleteCol(2);
#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//添加进度条
	prw.SetPercent(pid,0.3);
	//为增加基金的交易实体ID和名称，代码作准备
	resTable.InsertCol(0,Tx::Core::dtype_int4);//基金交易实体ID
	resTable.InsertCol(1,Tx::Core::dtype_val_string);//基金名称
	resTable.InsertCol(2,Tx::Core::dtype_val_string);//基金代码
	//为把m_txTable的内容放到resTable表里作准备。
	resTable.InsertCol(5,Tx::Core::dtype_decimal);//资产净值
	//把债券ID（也既是内码）转换成股票名称和代码
	resTable.InsertCol(9,Tx::Core::dtype_val_string);//债券strcode
	resTable.InsertCol(10,Tx::Core::dtype_val_string);//债券strname
	//把资产净值添加到resTable里	
	int bondId=0;
	double equity2;
	CString strname,strcode;
	std::vector<UINT>vecID1;
	std::vector<UINT>::iterator iteID1;
	int position;
	for(int m = 0;m < (int)m_txTable.GetRowCount();m++)
	{
		if (!vecID1.empty())
		{
			vecID1.clear();
		}
		m_txTable.GetCell(2,m,reportid);
		m_txTable.GetCell(3,m,equity2);
		resTable.Find(6,reportid,vecID1);//查找相等的基金定期报告ID
		for(iteID1 = vecID1.begin();iteID1 != vecID1.end();++iteID1)
		{
			position = *iteID1;
			resTable.SetCell(5,position,equity2);
			resTable.GetCell(13,position,strcode);
			resTable.GetCell(14,position,strname);
			resTable.SetCell(9,position,strcode);
			resTable.SetCell(10,position,strname);
			// zway 2010-07-01 for mantis #1663//
			//resTable.GetCell(8,position,bondId);			
			//GetSecurityNow(bondId);
			//if(m_pSecurity != NULL)
			//{
			//	strname = m_pSecurity->GetName();
			//	strcode = m_pSecurity->GetCode();
			//	if (strname.GetLength() == 0)
			//	{
			//		TRACE (_T("\n\nName is empty (%d).\n"), m_pSecurity->GetId());
			//	}
			//	resTable.SetCell(9,position,strcode);
			//	resTable.SetCell(10,position,strname);
			//}
			// zway 2010-07-01 for mantis #1663//
		}			
	}
	resTable.DeleteCol(resTable.GetColCount() - 2, 2);// zway 2010-07-01 for mantis #1663//
#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//增加基金的交易实体ID和名称，代码	
	int fundId  ;
	CString strName,strCode;
	std::unordered_map<int,double> dDataMap;//存放临时的基金净值	
	std::vector<int>::iterator iterDate;
	for(iterId = iSecurityId.begin();iterId != iSecurityId.end();++iterId)
	{	
		GetSecurityNow(*iterId);
		if(m_pSecurity == NULL)
			continue;
		fundId = m_pSecurity->GetSecurity1Id();
		strName = m_pSecurity->GetName();
		strCode = m_pSecurity->GetCode();
		std::vector<UINT> vecInstiID;
		resTable.Find(3,fundId,vecInstiID);
		std::vector<UINT>::iterator iteID;
		for(iteID = vecInstiID.begin();iteID != vecInstiID.end();++iteID)
		{
			resTable.SetCell(0,*iteID,*iterId);
			resTable.SetCell(1,*iteID,strName);
			resTable.SetCell(2,*iteID,strCode);
		}
		if(iType >=16)
		{
            //把所有相同的报告期的基金净值加起来。这是为了样本汇总时作准备。			
			int itempdate;
			std::vector<UINT> TempCollect;
			//填充dDataMap，把各个报告期的基金净值设为零
			for(iterDate = iDate.begin();iterDate != iDate.end();++iterDate)
			{
				dDataMap.insert(std::make_pair(*iterDate,0));
			}
			for(iterDate = iDate.begin();iterDate != iDate.end();++iterDate)
			{
				itempdate = *iterDate;
				if(!TempCollect.empty())
					TempCollect.clear();
				resTable.Find(4,itempdate,TempCollect);//取得与报告期相同的纪录的位置。
				if(TempCollect.empty())
					continue;
				std::set<UINT> positionSet(vecInstiID.begin(),vecInstiID.end());
				iteID = TempCollect.begin();
				for(;iteID != TempCollect.end();++iteID)
				{
					if(positionSet.find(*iteID) != positionSet.end())
					{
						resTable.GetCell(5,*iteID,equity2);
						dDataMap[itempdate] += equity2; 
						break;
					}
					else
						continue;
				}
			}			
		}
	}
#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	int iType2 = iType;//保存原来的值.
	//这里增加样本分类选择
	if(iType >1)
	{
		int iCount = resTable.GetRowCount();//累加之前resTable里的行数。		
		//为了进行分类统计。所以增加一列基金ID。
		resTable.AddCol(Tx::Core::dtype_int4);//基金风格
		resTable.AddCol(Tx::Core::dtype_int4);//基金管理公司ID
		resTable.AddCol(Tx::Core::dtype_int4);//基金托管银行ID		
		//添加进度条
		prw.SetPercent(pid,0.6);
		//这里增加样本分类选择
		m_txTable.Clear();
		tempTable.Clear();
		//准备样本集=第一参数列:F_FUND_ID,int型
		m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
		const int indicatorIndex1 = 3;
		long iIndicator1[indicatorIndex1] = 
		{
			//30001035,	//基金风格
			//modified by zhangxs 20091221---NewStyle
			30001232,	//基金风格New
			30001020,	//管理公司ID，
			30001021	//托管银行ID，
		};
		UINT varCfg1[1];			//参数配置
		int varCount1=1;			//参数个数
		for (int i = 0; i < indicatorIndex1; i++)
		{
			int tempIndicator = iIndicator1[i];
			GetIndicatorDataNow(tempIndicator);
			if (m_pIndicatorData==NULL)
				return false; 
			varCfg1[0]=0;
			result = m_pLogicalBusiness->SetIndicatorIntoTable(
				m_pIndicatorData,	//指标
				varCfg1,				//参数配置
				varCount1,			//参数个数
				m_txTable	//计算需要的参数传输载体以及计算后结果的载体
				);
			if(result==false)
			{
				return FALSE;
			}

		}
		result = m_pLogicalBusiness->GetData(m_txTable,true);
		if(result==false)
			return false;
#ifdef _DEBUG
		strTable=m_txTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
		UINT iCol2=m_txTable.GetColCount();
		//复制所有列信息
		tempTable.CopyColumnInfoFrom(m_txTable);
		if(m_txTable.GetRowCount()==0)
		{
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false;
		}
		UINT* nColArray2 = new UINT[iCol2];
		for(UINT i=0;i<iCol2;i++)
			nColArray2[i]=i;
//#ifdef _SECURITYID_FOR_STAT_
		m_txTable.EqualsAt(tempTable,nColArray2,iCol2,0,iSecurityId);
//#else
//		m_txTable.EqualsAt(tempTable,nColArray2,iCol2,0,iSecurity1Id);
//#endif
		delete nColArray2;
		nColArray2 = NULL;
		//把基金风格、公司ID、托管银行ID放到resTable
		std::vector<UINT> vecInstiID2;
		std::vector<UINT>::iterator iteID;		
		int iStyle,iCompanyId,ibankId;
		int tempfundId;
		int icount = tempTable.GetRowCount();
		for(int j = 0;j < icount;j++)
		{
			tempTable.GetCell(0,j,tempfundId);
			tempTable.GetCell(1,j,iStyle);
			tempTable.GetCell(2,j,iCompanyId);
			tempTable.GetCell(3,j,ibankId);
			//把基金ID和交易实体ID对应起来。并且把数据放到表里。
			if(!(vecInstiID2.empty()))
				vecInstiID2.clear();
//#ifdef _SECURITYID_FOR_STAT_
			resTable.Find(0,tempfundId,vecInstiID2);
/*#else
			resTable.Find(3,tempfundId,vecInstiID2);
#endif	*/		
			for(iteID = vecInstiID2.begin();iteID != vecInstiID2.end();++iteID)
			{
				position = *iteID;
				resTable.SetCell(13,position,iStyle);//基金风格
				resTable.SetCell(14,position,iCompanyId);//管理公司ID
				resTable.SetCell(15,position,ibankId);//托管银行ID
			}
		}		
		std::vector<int> ColVector;		//根据哪些列进行统计  
		if(!(iType & 1))
			iType += 1;//假设都选择选择了单只基金那一项分类方式。
		if(iType & 2)
			ColVector.push_back(14);
		if(iType & 4)
			ColVector.push_back(13);
		if(iType & 8)
			ColVector.push_back(15);
		std::vector<int> IntCol;			//需要相加的整型列
		//		IntCol.push_back(6);
		std::vector<int> DoubleCol;	//需要相加的double列
		DoubleCol.push_back(11);
		/*if (iType2 < 17)
			DoubleCol.push_back(5);*/
		/*for(int j = 6;j < 17;j++)
		{
			DoubleCol.push_back(j);
			j++;
		}*/
		int iTradeId = 8;//之所以这样做，是为了让它带回汇总样本统计的纪录数。
		AddUpRow(resTable,iType,ColVector,IntCol,DoubleCol,iDate,4,5,3,iTradeId,11,7);
//		std::vector<int>::size_type isize = iDate.size();
		int iCount1 = resTable.GetRowCount();//累加以后resTable里的行数。
		int iCount2,iCount3;
		iCount2 = iCount1 - iCount;//这是增加的行数 样本汇总和其他分类方式的行数。
		std::unordered_map<int,CString> CompanyMap;
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_INSTITUTIONID_TO_SHORTNAME,CompanyMap);
		std::unordered_map<int,CString> StyleMap;
		//modified by zhangxs 20091221---NewStyle
		//TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX,StyleMap);
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX_NEW,StyleMap);
		std::unordered_map<int,CString> BankMap;
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_BANK,BankMap);
		std::unordered_map<int,CString>::iterator iterMap;
		int id;
		CString Name;	
		if(iType < 17)//没有样本汇总的情况
		{
			for(int m = 0;m < iCount2;m++)
			{
				position = iCount+m;
				resTable.GetCell(13,position,id);
				if(id == 0)
				{
					resTable.GetCell(14,position,id);
					if(id == 0)
					{
						resTable.GetCell(15,position,id);
						iterMap = BankMap.find(id);
						if(iterMap!= BankMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,position,Name);
						resTable.SetCell(2,position,(CString)(_T("托管银行")));
						continue;
					}
					else
					{
						iterMap = CompanyMap.find(id);
						if(iterMap!= CompanyMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,position,Name);
						resTable.SetCell(2,position,(CString)(_T("基金公司")));
						continue;
					}					
				}
				else
				{
					iterMap = StyleMap.find(id);
					if(iterMap!= StyleMap.end())
						Name = iterMap->second;
					else
						Name = Tx::Core::Con_strInvalid;
					resTable.SetCell(1,position,Name);
					resTable.SetCell(2,position,(CString)(_T("基金风格")));
				}
			}				
		}
		else//不但有单只基金和样本汇总，还有其他的分类方式
		{
			int isize = iTradeId;//这是样本汇总的行数.
			iCount3 = iCount2 - isize;//除了样本汇总以外的其他分类方式的行数
			if(iCount3 > 0)//表示除了样本汇总以外还有其他的分类的方式
			{
				//填充样本汇总的那些行
				for(int k = 0;k < isize;k++)
				{
					resTable.SetCell(1,iCount+iCount3+k,(CString)(_T("样本汇总")));
					resTable.SetCell(2,iCount+iCount3+k,(CString)(_T("全部汇总")));
				}
				//填充除了样本汇总以外的其他分类方式的那些行。
				for(int i = 0;i < iCount3;i++)
				{
					position = iCount+i;
					resTable.GetCell(13,position,id);
					if(id == 0)
					{
						resTable.GetCell(14,position,id);
						if(id == 0)
						{
							resTable.GetCell(15,position,id);
							iterMap = BankMap.find(id);
							if(iterMap!= BankMap.end())
								Name = iterMap->second;
							else
								Name = Tx::Core::Con_strInvalid;
							resTable.SetCell(1,position,Name);
							resTable.SetCell(2,position,(CString)(_T("托管银行")));
							continue;
						}
						else
						{
							iterMap = CompanyMap.find(id);
							if(iterMap!= CompanyMap.end())
								Name = iterMap->second;
							else
								Name = Tx::Core::Con_strInvalid;
							resTable.SetCell(1,position,Name);
							resTable.SetCell(2,position,(CString)(_T("基金公司")));
							continue;
						}					
					}
					else
					{
						iterMap = StyleMap.find(id);
						if(iterMap!= StyleMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,position,Name);
						resTable.SetCell(2,position,(CString)(_T("基金风格")));						
					}			
				}
			}
			else//只有单只基金和汇总的情况
			{				
				for(int k = 0;k < isize;k++)
				{
					position = iCount+k;
					resTable.SetCell(1,position,(CString)(_T("样本汇总")));
					resTable.SetCell(2,position,(CString)(_T("全部汇总")));			
				}		
			}
		}	
		//把报告年份和报告期转化成CString类型,所以增加一列
		resTable.InsertCol(5,Tx::Core::dtype_val_string);//str报告期	
		CString stryear,strdate;
		int tempdate,iyear ,itemp;
		int tempCount;
		int n = 0;
		if(!(iType2 & 1))
		{
			resTable.DeleteRow(0,iCount);
			tempCount = iCount2;
			n = 0;
		}
		else
		{
			for(int i = 0;i < iCount;i++)
			{
				//填写报告期	
				resTable.GetCell(4,i,tempdate);	
				itemp = tempdate%10000;
				iyear = tempdate/10000;
				stryear.Format(_T("%d"),iyear);
				switch(itemp)
				{
				case 331:
					strdate = stryear + _T("年") + _T("一季报");
					break;
				case 630:
					strdate = stryear + _T("年") + _T("二季报");
					break;
				case 930:
					strdate = stryear + _T("年") + _T("三季报");
					break;
				case 1231:
					strdate = stryear + _T("年") + _T("四季报");
					break;
				}
				resTable.SetCell(5,i,strdate);			
			}
			tempCount = iCount1;	
			n = iCount;
		}
		double proportion,Value;
		std::unordered_map<int,double>::iterator iterMap2;
		for(;n < tempCount;n++)
		{
			//填写报告期	
			resTable.GetCell(4,n,tempdate);	
			itemp = tempdate%10000;
			iyear = tempdate/10000;
			stryear.Format(_T("%d"),iyear);
			switch(itemp)
			{
			case 331:
				strdate = stryear + _T("年") + _T("一季报");
				break;
			case 630:
				strdate = stryear + _T("年") + _T("二季报");
				break;
			case 930:
				strdate = stryear + _T("年") + _T("三季报");
				break;
			case 1231:
				strdate = stryear + _T("年") + _T("四季报");
				break;
			}			
			//把样本汇总的基金净值放到resTable
			if(iType >=17 && n >= tempCount - iTradeId)
			{
				iterMap2 = dDataMap.find(tempdate);
				if(iterMap2 != dDataMap.end())
				{
					equity2 = iterMap2->second;
					resTable.SetCell(6,n,equity2);
					//计算比例					
					resTable.GetCell(12,n,Value);//取得债券的市值
					if(equity2 < 0 || Value < 0)
						proportion = Tx::Core::Con_doubleInvalid;
					proportion = Value*100/equity2;			
					resTable.SetCell(13,n,proportion);			
				}
			}
			else
			{
				//计算比例
				resTable.GetCell(6,n,equity2);//取得资产净值
				resTable.GetCell(12,n,Value);//取得债券的市值
				if(equity2 < 0 || Value < 0)
					proportion = Tx::Core::Con_doubleInvalid;
				proportion = Value*100/equity2;			
				resTable.SetCell(13,n,proportion);			
			}
			resTable.SetCell(5,n,strdate);
			
		}
		resTable.DeleteCol(14,3);
	}
	else
	{	
		//添加进度条
		prw.SetPercent(pid,0.7);
		//把报告年份和报告期转化成CString类型,所以增加一列
		resTable.InsertCol(5,Tx::Core::dtype_val_string);//str报告期
		//把报告年份和报告期转化为CString类型
		int tempdate,iyear,tempdate2;
		CString stryear,strdate;
		for(int k = 0;k < (int)resTable.GetRowCount();k++)
		{			
			resTable.GetCell(4,k,tempdate);			
			iyear = tempdate/10000;
			stryear.Format(_T("%d"),iyear);
			tempdate2 = tempdate%10000;
			switch(tempdate2)
			{
			case 331:
				strdate = stryear + _T("年") + _T("一季报");
				break;
			case 630:
				strdate = stryear + _T("年") + _T("二季报");
				break;
			case 930:
				strdate = stryear + _T("年") + _T("三季报");
				break;
			case 1231: 
				strdate = stryear + _T("年") + _T("四季报");
				break;
			}
			resTable.SetCell(5,k,strdate);
		}

	}
	resTable.DeleteCol(9);//删除债券交易实体ID    
	resTable.DeleteCol(7);
	resTable.DeleteCol(3);//删除基金ID
	resTable.Arrange();
	MultiSortRule multisort;
	multisort.AddRule(3,false);
	multisort.AddRule(1,true);	
	multisort.AddRule(6,true);
	resTable.SortInMultiCol(multisort);
	resTable.Arrange();
	resTable.DeleteCol(3);//删除合并的报告年份和报告期
	//添加进度条
	/*if(iType2 > 17)
	{
		resTable.SetSortRange(0,resTable.GetRowCount()-2);
	}*/
	prw.SetPercent(pid,1.0);
	return true;

}
//处于转股期的可转债T_BOND_HOLDING_TOP_FIVE_CONVERTIBLE
bool TxFund::StatConvertibleBond(
		Tx::Core::Table_Indicator &resTable,
		std::vector<int>	iSecurityId,
		std::vector<int>	iDate,
		int		iType
		)
{
	//添加进度条
//	ProgressWnd* pwd=Tx::Core::ProgressWnd::GetInstance();
	ProgressWnd prw;
	UINT pid=prw.AddItem(1,_T("处于转股期的可转债统计..."),0.0);
	prw.Show(1000);
	m_txTable.Clear();//这是引用别人的成员变量，
	//从T_ASSET_ALLOCATION_twoyear里取基金定期报告ID，基金ID，报告年份，报告期
	//默认的返回值状态
	bool result = false;
	//清空数据
	m_txTable.Clear();
	//准备样本集参数列
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//基金ID
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//报告年份
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//报告期
	const int indicatorIndex = 2;
	long iIndicator[indicatorIndex] = 
	{
		30901332,	//基金定期报告ID//这是T_ASSET_ALLOCATION表里
		30901140	//资产净值
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
			return FALSE;
		}

	}
	UINT iColCount = m_txTable.GetColCount();
	UINT* nColArray = new UINT[iColCount];
	for(int i = 0; i < (int)iColCount; i++)
	{
		nColArray[i] = i;
	}
	result = m_pLogicalBusiness->GetData(m_txTable,true);
#ifdef _DEBUG
	strTable=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//把交易实体ID转化为基金ID
	std::vector<int> iSecurity1Id;
	std::vector<int>::iterator iterId;
	int tempFundId;
	std::vector<int> tempTradeV;
	for(iterId = iSecurityId.begin();iterId != iSecurityId.end();++iterId)
	{
		GetSecurityNow(*iterId);
		if(m_pSecurity != NULL)
		{
			tempFundId = m_pSecurity->GetSecurity1Id();
			if(find(iSecurity1Id.begin(),iSecurity1Id.end(),tempFundId) == iSecurity1Id.end())
			{
				iSecurity1Id.push_back(tempFundId);
				tempTradeV.push_back(*iterId);
			}		
		}
	}
	iSecurityId.clear();
	iSecurityId.assign(tempTradeV.begin(),tempTradeV.end());
	/*if(!TransObjectToSecIns(iSecurityId,iSecurity1Id,1))
	{
		delete nColArray;
		nColArray = NULL;
		return false;	
	}*/
	Tx::Core::Table_Indicator tempTable;
	tempTable.CopyColumnInfoFrom(m_txTable);
	//根据基金ID进行筛选
	m_txTable.EqualsAt(tempTable,nColArray,iColCount,0,iSecurity1Id);
	//为了进行年度和报告期的筛选，所以把报告年份和报告期和为一列。
	TransReportDateToNormal2(tempTable,1);
	m_txTable.Clear();
	m_txTable.CopyColumnInfoFrom(tempTable);
	//进行年度和报告期的筛选
	tempTable.EqualsAt(m_txTable,nColArray,iColCount-1,1,iDate);
#ifdef _DEBUG
	strTable=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//取出基金定期报告ID，把它放到set里，为和从视图T_BOND_HOLDING_TOP_FIVE_CONVERTIBLE_twoyear取得数据进行连接作准备。
	std::set<int> ReportV;
	for(int i = 0;i < (int)m_txTable.GetRowCount();i++)
	{
		int reportid ;
		m_txTable.GetCell(2,i,reportid);
		ReportV.insert(reportid);
	}
	delete nColArray;
	nColArray = NULL;


	//从T_BOND_HOLDING_TOP_FIVE_CONVERTIBLE_twoyear取其他的数据
	Tx::Core::Table_Indicator zhzTable;
	//准备样本集参数列
	zhzTable.AddParameterColumn(Tx::Core::dtype_int4);//基金ID
	zhzTable.AddParameterColumn(Tx::Core::dtype_int4);//报告年份
	zhzTable.AddParameterColumn(Tx::Core::dtype_int4);//报告期
	zhzTable.AddParameterColumn(Tx::Core::dtype_int4);//F_NO序号
	const int indicatorIndex3 = 5;
	long iIndicator3[indicatorIndex3] = 
	{
		30901336,	//基金定期报告ID//这是T_BOND_HOLDING_TOP_FIVE_CONVERTIBLE_twoyear表里
		30901254,	//市值排名
		30901251,	//债券ID
		30901252,	//债券市值
		30901253	//所占比例（即是占净值比例）

	};
	UINT varCfg3[4];			//参数配置
	int varCount3=4;			//参数个数
	for (int i = 0; i < indicatorIndex3; i++)
	{
		int tempIndicator = iIndicator3[i];

		GetIndicatorDataNow(tempIndicator);
		if (m_pIndicatorData==NULL)
		{ return false; }
		varCfg3[0]=0;
		varCfg3[1]=1;
		varCfg3[2]=2;
		varCfg3[3]=3;
		result = m_pLogicalBusiness->SetIndicatorIntoTable(
			m_pIndicatorData,	//指标
			varCfg3,				//参数配置
			varCount3,			//参数个数
			zhzTable	//计算需要的参数传输载体以及计算后结果的载体
			);
		if(result==false)
		{ 
			return FALSE;
		}

	}

	result = m_pLogicalBusiness->GetData(zhzTable,true);
	zhzTable.DeleteCol(3);//删除序号列，之所以在删掉，是因为不想改下面的代码。
#ifdef _DEBUG
	strTable=zhzTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	UINT iColCount3 = zhzTable.GetColCount();
	UINT* nColArray3 = new UINT[iColCount3];
	for(int i = 0; i < (int)iColCount3; i++)
	{
		nColArray3[i] = i;
	}
	resTable.CopyColumnInfoFrom(zhzTable);
	zhzTable.EqualsAt(resTable,nColArray3,iColCount3,3,ReportV);
	//为了把他们转化成CString类型，所以把报告年份和报告期和为一列。
	TransReportDateToNormal2(resTable,1);
	delete nColArray3;
	nColArray3 = NULL;
#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif

	//添加进度条
	prw.SetPercent(pid,0.3);
	//为增加基金的交易实体ID和名称，代码作准备
	resTable.InsertCol(0,Tx::Core::dtype_int4);//基金交易实体ID
	resTable.InsertCol(1,Tx::Core::dtype_val_string);//基金名称
	resTable.InsertCol(2,Tx::Core::dtype_val_string);//基金代码
	//为把m_txTable的内容放到resTable表里作准备。
	resTable.InsertCol(5,Tx::Core::dtype_decimal);//资产净值
	//把债券ID（也既是内码）转换成债券名称和代码
	resTable.InsertCol(9,Tx::Core::dtype_val_string);//债券strcode
	resTable.InsertCol(10,Tx::Core::dtype_val_string);//债券strname
	double equity2;
	for(int m = 0;m < (int)m_txTable.GetRowCount();m++)
	{
		int reId2;		
		m_txTable.GetCell(2,m,reId2);
		m_txTable.GetCell(3,m,equity2);
		std::vector<UINT> vecInstiID;
		resTable.Find(6,reId2,vecInstiID);
		std::vector<UINT>::iterator iteID;
		for(iteID = vecInstiID.begin();iteID != vecInstiID.end();++iteID)
		{
			int bondId ;
			resTable.GetCell(8,*iteID,bondId);
			GetSecurityNow(bondId);
			if(m_pSecurity == NULL)
				continue;
			CString strname,strcode;
			strname = m_pSecurity->GetName();
			strcode = m_pSecurity->GetCode();
			//////把占净值比乘以100
			////double dEquity;
			////resTable.GetCell(12,*iteID,dEquity);
			////dEquity = dEquity*100;
			////resTable.SetCell(12,*iteID,dEquity);
		    resTable.SetCell(9,*iteID,strcode);
			resTable.SetCell(10,*iteID,strname);
			resTable.SetCell(5,*iteID,equity2);
		}

	}
#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//增加基金的交易实体ID和名称，代码
//	std::vector<int>::iterator iterId;
	std::unordered_map<int,double> dDataMap;//存放临时的基金净值	
	std::vector<int>::iterator iterDate;
	for(iterId = iSecurityId.begin();iterId != iSecurityId.end();++iterId)
	{
		int fundId;
		CString strName,strCode;
		GetSecurityNow(*iterId);
		if(m_pSecurity == NULL)
			continue;
		fundId = m_pSecurity->GetSecurity1Id();
		strName = m_pSecurity->GetName();
		strCode = m_pSecurity->GetCode();
		std::vector<UINT> vecInstiID;
		resTable.Find(3,fundId,vecInstiID);
		std::vector<UINT>::iterator iteID;
		for(iteID = vecInstiID.begin();iteID != vecInstiID.end();++iteID)
		{
			resTable.SetCell(0,*iteID,*iterId);
			resTable.SetCell(1,*iteID,strName);
			resTable.SetCell(2,*iteID,strCode);
		}
		if(iType >=16)
		{
			//把所有相同的报告期的基金净值加起来。这是为了样本汇总时作准备。			
			int itempdate;
			std::vector<UINT> TempCollect;
			//填充dDataMap，把各个报告期的基金净值设为零
			for(iterDate = iDate.begin();iterDate != iDate.end();++iterDate)
			{
				dDataMap.insert(std::make_pair(*iterDate,0));
			}
			for(iterDate = iDate.begin();iterDate != iDate.end();++iterDate)
			{
				itempdate = *iterDate;
				if(!TempCollect.empty())
					TempCollect.clear();
				resTable.Find(4,itempdate,TempCollect);//取得与报告期相同的纪录的位置。
				if(TempCollect.empty())
					continue;
				std::set<UINT> positionSet(vecInstiID.begin(),vecInstiID.end());
				iteID = TempCollect.begin();
				for(;iteID != TempCollect.end();++iteID)
				{
					if(positionSet.find(*iteID) != positionSet.end())
					{
						resTable.GetCell(5,*iteID,equity2);
						dDataMap[itempdate] += equity2; 
						break;
					}
					else
						continue;
				}
			}			
		}
	}
	int iType2 = iType;//保存原来的值.
	//这里增加样本分类选择  /*下面的注释都是按重仓债的统计写的，因为下面的代码是从哪里拷贝过来的*/
	if(iType >1)
	{
		int iCount = resTable.GetRowCount();//累加之前resTable里的行数。		
		//为了进行分类统计。所以增加一列基金ID。
		resTable.AddCol(Tx::Core::dtype_int4);//基金风格
		resTable.AddCol(Tx::Core::dtype_int4);//基金管理公司ID
		resTable.AddCol(Tx::Core::dtype_int4);//基金托管银行ID		
		//添加进度条
		prw.SetPercent(pid,0.6);
		//这里增加样本分类选择
		m_txTable.Clear();
		tempTable.Clear();
		//准备样本集=第一参数列:F_FUND_ID,int型
		m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
		const int indicatorIndex1 = 3;
		long iIndicator1[indicatorIndex1] = 
		{
			//30001035,	//基金风格
			//modified by zhangxs 20091221---NewStyle
			30001232,	//基金风格New
			30001020,	//管理公司ID，
			30001021	//托管银行ID，
		};
		UINT varCfg1[1];			//参数配置
		int varCount1=1;			//参数个数
		for (int i = 0; i < indicatorIndex1; i++)
		{
			int tempIndicator = iIndicator1[i];
			GetIndicatorDataNow(tempIndicator);
			if (m_pIndicatorData==NULL)
				return false; 
			varCfg1[0]=0;
			result = m_pLogicalBusiness->SetIndicatorIntoTable(
				m_pIndicatorData,	//指标
				varCfg1,				//参数配置
				varCount1,			//参数个数
				m_txTable	//计算需要的参数传输载体以及计算后结果的载体
				);
			if(result==false)
			{
				return FALSE;
			}

		}
		result = m_pLogicalBusiness->GetData(m_txTable,true);
		if(result==false)
			return false;
#ifdef _DEBUG
		strTable=m_txTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
		UINT iCol2=m_txTable.GetColCount();
		//复制所有列信息
		tempTable.CopyColumnInfoFrom(m_txTable);
		if(m_txTable.GetRowCount()==0)
		{
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false;
		}
		UINT* nColArray2 = new UINT[iCol2];
		for(UINT i=0;i<iCol2;i++)
			nColArray2[i]=i;
//#ifdef _SECURITYID_FOR_STAT_
		m_txTable.EqualsAt(tempTable,nColArray2,iCol2,0,iSecurityId);
//#else
//		m_txTable.EqualsAt(tempTable,nColArray2,iCol2,0,iSecurity1Id);
//#endif
		delete nColArray2;
		nColArray2 = NULL;
		//把基金风格、公司ID、托管银行ID放到resTable
		std::vector<UINT> vecInstiID2;
		std::vector<UINT>::iterator iteID;		
		int iStyle,iCompanyId,ibankId;
		int tempfundId;
		int icount = tempTable.GetRowCount();
		UINT position;
		for(int j = 0;j < icount;j++)
		{
			tempTable.GetCell(0,j,tempfundId);
			tempTable.GetCell(1,j,iStyle);
			tempTable.GetCell(2,j,iCompanyId);
			tempTable.GetCell(3,j,ibankId);
			//把基金ID和交易实体ID对应起来。并且把数据放到表里。
			if(!(vecInstiID2.empty()))
				vecInstiID2.clear();
//#ifdef _SECURITYID_FOR_STAT_
			resTable.Find(0,tempfundId,vecInstiID2);
/*#else
			resTable.Find(3,tempfundId,vecInstiID2);
#endif	*/		
			for(iteID = vecInstiID2.begin();iteID != vecInstiID2.end();++iteID)
			{
				position = *iteID;
				resTable.SetCell(13,position,iStyle);//基金风格
				resTable.SetCell(14,position,iCompanyId);//管理公司ID
				resTable.SetCell(15,position,ibankId);//托管银行ID
			}
		}		
		std::vector<int> ColVector;		//根据哪些列进行统计  
		if(!(iType & 1))
			iType += 1;//假设都选择选择了单只基金那一项分类方式。
		if(iType & 2)
			ColVector.push_back(14);
		if(iType & 4)
			ColVector.push_back(13);
		if(iType & 8)
			ColVector.push_back(15);
		std::vector<int> IntCol;			//需要相加的整型列
		//		IntCol.push_back(6);
		std::vector<int> DoubleCol;	//需要相加的double列
		DoubleCol.push_back(11);
		/*if (iType2 < 17)
		DoubleCol.push_back(5);*/
		/*for(int j = 6;j < 17;j++)
		{
		DoubleCol.push_back(j);
		j++;
		}*/
		int iTradeId = 8;//之所以这样做，是为了让它带回汇总样本统计的纪录数。
		AddUpRow(resTable,iType,ColVector,IntCol,DoubleCol,iDate,4,5,3,iTradeId,11,7);
		//		std::vector<int>::size_type isize = iDate.size();
		int iCount1 = resTable.GetRowCount();//累加以后resTable里的行数。
		int iCount2,iCount3;
		iCount2 = iCount1 - iCount;//这是增加的行数 样本汇总和其他分类方式的行数。
		std::unordered_map<int,CString> CompanyMap;
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_INSTITUTIONID_TO_SHORTNAME,CompanyMap);
		std::unordered_map<int,CString> StyleMap;
		//modified by zhangxs 20091221---NewStyle
		//TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX,StyleMap);
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX_NEW,StyleMap);
		std::unordered_map<int,CString> BankMap;
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_BANK,BankMap);
		std::unordered_map<int,CString>::iterator iterMap;
		int id;
		CString Name;	
		if(iType < 17)//没有样本汇总的情况
		{
			for(int m = 0;m < iCount2;m++)
			{
				position = iCount+m;
				resTable.GetCell(13,position,id);
				if(id == 0)
				{
					resTable.GetCell(14,position,id);
					if(id == 0)
					{
						resTable.GetCell(15,position,id);
						iterMap = BankMap.find(id);
						if(iterMap!= BankMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,position,Name);
						resTable.SetCell(2,position,(CString)(_T("托管银行")));
						continue;
					}
					else
					{
						iterMap = CompanyMap.find(id);
						if(iterMap!= CompanyMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,position,Name);
						resTable.SetCell(2,position,(CString)(_T("基金公司")));
						continue;
					}					
				}
				else
				{
					iterMap = StyleMap.find(id);
					if(iterMap!= StyleMap.end())
						Name = iterMap->second;
					else
						Name = Tx::Core::Con_strInvalid;
					resTable.SetCell(1,position,Name);
					resTable.SetCell(2,position,(CString)(_T("基金风格")));
				}
			}				
		}
		else//不但有单只基金和样本汇总，还有其他的分类方式
		{
			int isize = iTradeId;//这是样本汇总的行数.
			iCount3 = iCount2 - isize;//除了样本汇总以外的其他分类方式的行数
			if(iCount3 > 0)//表示除了样本汇总以外还有其他的分类的方式
			{
				//填充样本汇总的那些行
				for(int k = 0;k < isize;k++)
				{
					resTable.SetCell(1,iCount+iCount3+k,(CString)(_T("样本汇总")));
					resTable.SetCell(2,iCount+iCount3+k,(CString)(_T("全部汇总")));
				}
				//填充除了样本汇总以外的其他分类方式的那些行。
				for(int i = 0;i < iCount3;i++)
				{
					position = iCount+i;
					resTable.GetCell(13,position,id);
					if(id == 0)
					{
						resTable.GetCell(14,position,id);
						if(id == 0)
						{
							resTable.GetCell(15,position,id);
							iterMap = BankMap.find(id);
							if(iterMap!= BankMap.end())
								Name = iterMap->second;
							else
								Name = Tx::Core::Con_strInvalid;
							resTable.SetCell(1,position,Name);
							resTable.SetCell(2,position,(CString)(_T("托管银行")));
							continue;
						}
						else
						{
							iterMap = CompanyMap.find(id);
							if(iterMap!= CompanyMap.end())
								Name = iterMap->second;
							else
								Name = Tx::Core::Con_strInvalid;
							resTable.SetCell(1,position,Name);
							resTable.SetCell(2,position,(CString)(_T("基金公司")));
							continue;
						}					
					}
					else
					{
						iterMap = StyleMap.find(id);
						if(iterMap!= StyleMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,position,Name);
						resTable.SetCell(2,position,(CString)(_T("基金风格")));						
					}			
				}
			}
			else//只有单只基金和汇总的情况
			{				
				for(int k = 0;k < isize;k++)
				{
					position = iCount+k;
					resTable.SetCell(1,position,(CString)(_T("样本汇总")));
					resTable.SetCell(2,position,(CString)(_T("全部汇总")));			
				}		
			}
		}	
		//把报告年份和报告期转化成CString类型,所以增加一列
		resTable.InsertCol(5,Tx::Core::dtype_val_string);//str报告期	
		CString stryear,strdate;
		int tempdate,iyear ,itemp;
		int tempCount;
		int n = 0;
		if(!(iType2 & 1))
		{
			resTable.DeleteRow(0,iCount);
			tempCount = iCount2;
			n = 0;
		}
		else
		{
			for(int i = 0;i < iCount;i++)
			{
				//填写报告期	
				resTable.GetCell(4,i,tempdate);	
				itemp = tempdate%10000;
				iyear = tempdate/10000;
				stryear.Format(_T("%d"),iyear);
				switch(itemp)
				{
				case 331:
					strdate = stryear + _T("年") + _T("一季报");
					break;
				case 630:
					strdate = stryear + _T("年") + _T("二季报");
					break;
				case 930:
					strdate = stryear + _T("年") + _T("三季报");
					break;
				case 1231:
					strdate = stryear + _T("年") + _T("四季报");
					break;
				}
				resTable.SetCell(5,i,strdate);			
			}
			tempCount = iCount1;	
			n = iCount;
		}
		double proportion,Value;
		std::unordered_map<int,double>::iterator iterMap2;
		for(;n < tempCount;n++)
		{
			//填写报告期	
			resTable.GetCell(4,n,tempdate);	
			itemp = tempdate%10000;
			iyear = tempdate/10000;
			stryear.Format(_T("%d"),iyear);
			switch(itemp)
			{
			case 331:
				strdate = stryear + _T("年") + _T("一季报");
				break;
			case 630:
				strdate = stryear + _T("年") + _T("二季报");
				break;
			case 930:
				strdate = stryear + _T("年") + _T("三季报");
				break;
			case 1231:
				strdate = stryear + _T("年") + _T("四季报");
				break;
			}			
			//把样本汇总的基金净值放到resTable
			if(iType >=17 && n >= tempCount - iTradeId)
			{
				iterMap2 = dDataMap.find(tempdate);
				if(iterMap2 != dDataMap.end())
				{
					equity2 = iterMap2->second;
					resTable.SetCell(6,n,equity2);
					//计算比例					
					resTable.GetCell(12,n,Value);//取得债券的市值
					if(equity2 < 0 || Value < 0)
						proportion = Tx::Core::Con_doubleInvalid;
					proportion = Value*100/equity2;			
					resTable.SetCell(13,n,proportion);			
				}
			}
			else
			{
				//计算比例
				resTable.GetCell(6,n,equity2);//取得资产净值
				resTable.GetCell(12,n,Value);//取得债券的市值
				if(equity2 < 0 || Value < 0)
					proportion = Tx::Core::Con_doubleInvalid;
				proportion = Value*100/equity2;			
				resTable.SetCell(13,n,proportion);			
			}
			resTable.SetCell(5,n,strdate);

		}
		resTable.DeleteCol(14,3);
	}
	else
	{
		//添加进度条
		prw.SetPercent(pid,0.7);
		//把报告年份和报告期转化成CString类型,所以增加一列
		resTable.InsertCol(5,Tx::Core::dtype_val_string);//str报告期
		//把报告年份和报告期转化为CString类型
		int tempdate,iyear,tempdate2;
		CString stryear,strdate;
		for(int k = 0;k < (int)resTable.GetRowCount();k++)
		{			
			resTable.GetCell(4,k,tempdate);			
			iyear = tempdate/10000;
			stryear.Format(_T("%d"),iyear);
			tempdate2 = tempdate%10000;
			switch(tempdate2)
			{
			case 331:
				strdate = stryear + _T("年") + _T("一季报");
				break;
			case 630:
				strdate = stryear + _T("年") + _T("二季报");
				break;
			case 930:
				strdate = stryear + _T("年") + _T("三季报");
				break;
			case 1231: 
				strdate = stryear + _T("年") + _T("四季报");
				break;
			}
			resTable.SetCell(5,k,strdate);
		}

	}
	resTable.DeleteCol(9);//删除债券交易实体ID    
	resTable.DeleteCol(7);
	//resTable.DeleteCol(4);
	resTable.DeleteCol(3);//删除基金ID
	resTable.Arrange();
	MultiSortRule multisort;
	multisort.AddRule(3,false);
	multisort.AddRule(1,true);	
	multisort.AddRule(6,true);
	resTable.SortInMultiCol(multisort);
	resTable.Arrange();
	resTable.DeleteCol(3);//删除合并的报告年份和报告期
	//添加进度条
	/*if(iType2 > 17)
	{
		resTable.SetSortRange(0,resTable.GetRowCount()-2);
	}*/
	prw.SetPercent(pid,1.0);
	return true;
}


//add by lijw 2008-04-8
//货币基金持有债券T_BOND_HOLDING_TOP_FIVE_BOND
bool TxFund::StatCurrencyFundHoldingBond(
	Tx::Core::Table_Indicator &resTable,
	std::vector<int> &iSecurityId,
	std::vector<int> &iDate,
	int iType
	)
{
	//添加进度条
//	ProgressWnd* pwd=Tx::Core::ProgressWnd::GetInstance();
	ProgressWnd prw;
	UINT pid=prw.AddItem(1,_T("货币基金持有债券统计..."),0.0);
	prw.Show(1000);
	m_txTable.Clear();//这是引用别人的成员变量，
	//从T_ASSET_ALLOCATION_twoyear里取基金定期报告ID，基金ID，报告年份，报告期
	//默认的返回值状态
	bool result = false;
	//清空数据
	m_txTable.Clear();
	//准备样本集参数列
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//基金ID
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//报告年份
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//报告期
	const int indicatorIndex = 2;
	long iIndicator[indicatorIndex] = 
	{
		30901332,	//基金定期报告ID//这是T_ASSET_ALLOCATION表里
		30901140	//资产净值
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
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false;
		}

	}

	UINT iColCount = m_txTable.GetColCount();
	UINT* nColArray = new UINT[iColCount];
	for(int i = 0; i < (int)iColCount; i++)
	{
		nColArray[i] = i;
	}
	result = m_pLogicalBusiness->GetData(m_txTable,true);
	if (result == false)
	{
		//添加进度条
		prw.SetPercent(pid,1.0);
		return false;
	}
#ifdef _DEBUG
	strTable=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//把交易实体ID转化为基金ID
	std::set<int> iSecurity1Id;
	std::vector<int>::iterator iterV;
	std::vector<int> tempTradeV;
	int iFundId;
	for (iterV = iSecurityId.begin();iterV != iSecurityId.end();++iterV)
	{
		GetSecurityNow(*iterV);
		if (m_pSecurity != NULL)
		{
			iFundId = m_pSecurity->GetSecurity1Id();
			if(m_pSecurity->IsFund_Currency())
			{
				if(iSecurity1Id.find(iFundId) == iSecurity1Id.end())
				{
					iSecurity1Id.insert(m_pSecurity->GetSecurity1Id());
					tempTradeV.push_back(*iterV);
				}				
			}
		}
	}
	iSecurityId.clear();
	iSecurityId.assign(tempTradeV.begin(),tempTradeV.end());
	Tx::Core::Table_Indicator tempTable;
	tempTable.CopyColumnInfoFrom(m_txTable);
	//根据基金ID进行筛选
	m_txTable.EqualsAt(tempTable,nColArray,iColCount,0,iSecurity1Id);
	//为了进行年度和报告期的筛选，所以把报告年份和报告期和为一列。
	TransReportDateToNormal2(tempTable,1);
	m_txTable.Clear();
	m_txTable.CopyColumnInfoFrom(tempTable);
	//进行年度和报告期的筛选
	tempTable.EqualsAt(m_txTable,nColArray,iColCount-1,1,iDate);
	//取出基金定期报告ID，把它放到set里，为和从视图T_BOND_HOLDING_TOP_FIVE_CONVERTIBLE_twoyear取得数据进行连接作准备。
	std::set<int> ReportV;
	for(int i = 0;i < (int)m_txTable.GetRowCount();i++)
	{
		int reportid ;
		m_txTable.GetCell(2,i,reportid);
		ReportV.insert(reportid);
	}
	if (ReportV.empty())
	{
		//添加进度条
		prw.SetPercent(pid,1.0);
		return false;
	}
	delete nColArray;
	nColArray = NULL;
#ifdef _DEBUG
	strTable=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//从T_BOND_HOLDING_TOP_FIVE_BOND_twoyear取其他的数据
	Tx::Core::Table_Indicator BondTable;
	//准备样本集参数列
	BondTable.AddParameterColumn(Tx::Core::dtype_int4);//基金ID
	BondTable.AddParameterColumn(Tx::Core::dtype_int4);//报告年份
	BondTable.AddParameterColumn(Tx::Core::dtype_int4);//报告期
	BondTable.AddParameterColumn(Tx::Core::dtype_int4);//序号
	const int indicatorIndex3 = 8;
	long iIndicator3[indicatorIndex3] = 
	{
		30901352,	//基金的定期报告ID
		30901250,	//市值排名
		30901247,	//债券交易实体ID
		30901338,	//债券代码（也即时外码）
		30901339,	//债券的名称
		30901337,	//债券数量
		30901248,	//债券市值（也即是成本）
		30901249	//所占比例（即是占净值比例）		
	};
	UINT varCfg3[4];			//参数配置
	int varCount3=4;			//参数个数
	for (int i = 0; i < indicatorIndex3; i++)
	{
		int tempIndicator = iIndicator3[i];

		GetIndicatorDataNow(tempIndicator);
		if (m_pIndicatorData==NULL)
		{ return false; }
		varCfg3[0]=0;
		varCfg3[1]=1;
		varCfg3[2]=2;
		varCfg3[3]=3;
		result = m_pLogicalBusiness->SetIndicatorIntoTable(
			m_pIndicatorData,	//指标
			varCfg3,				//参数配置
			varCount3,			//参数个数
			BondTable	//计算需要的参数传输载体以及计算后结果的载体
			);
		if(result==false)
		{ 
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false;
		}
	}
	result = m_pLogicalBusiness->GetData(BondTable,true);
	if (result == false)
	{
		//添加进度条
		prw.SetPercent(pid,1.0);
		return false;
	}
#ifdef _DEBUG
	strTable=BondTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	UINT iColCount3 = BondTable.GetColCount();
	UINT* nColArray3 = new UINT[iColCount3];
	for(int i = 0; i < (int)iColCount3; i++)
	{
		nColArray3[i] = i;
	}
	resTable.CopyColumnInfoFrom(BondTable);
	BondTable.EqualsAt(resTable,nColArray3,iColCount3,4,ReportV);
	delete nColArray3;
	nColArray3 = NULL;
	if(resTable.GetRowCount() == 0)
	{
		//添加进度条
		prw.SetPercent(pid,1.0);
		return false;
	}
	//把序号那一列删掉
	resTable.DeleteCol(3);
	//为了把他们转化成CString类型，所以把报告年份和报告期和为一列。并且也为样本汇总时，合并基金净值作准备。
	TransReportDateToNormal2(resTable,1);
#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//添加进度条
	prw.SetPercent(pid,0.3);
	//为增加基金的交易实体ID和名称，代码作准备
	resTable.InsertCol(0,Tx::Core::dtype_int4);//基金交易实体ID
	resTable.InsertCol(1,Tx::Core::dtype_val_string);//基金名称
	resTable.InsertCol(2,Tx::Core::dtype_val_string);//基金代码
	//为把m_txTable的内容放到resTable表里作准备。
	resTable.InsertCol(5,Tx::Core::dtype_decimal);//资产净值
	////把债券ID（也既是内码）转换成债券名称和代码
	//resTable.InsertCol(9,Tx::Core::dtype_val_string);//债券strcode
	//resTable.InsertCol(10,Tx::Core::dtype_val_string);//债券strname
	double equity2;
	int reId2;
	std::vector<UINT>::iterator iteID;
	std::vector<UINT> vecInstiID;
	for(int m = 0;m < (int)m_txTable.GetRowCount();m++)
	{			
		m_txTable.GetCell(2,m,reId2);
		m_txTable.GetCell(3,m,equity2);
		if(!vecInstiID.empty())
			vecInstiID.clear();
		resTable.Find(6,reId2,vecInstiID);
//		int bondId;
		CString strCode;
		for(iteID = vecInstiID.begin();iteID != vecInstiID.end();++iteID)
		{
			/*resTable.GetCell(8,*iteID,bondId);
			GetSecurityNow(bondId);
			if(m_pSecurity != NULL)
			{
				strName = m_pSecurity->GetName();
				strCode = m_pSecurity->GetCode();
				resTable.SetCell(9,*iteID,strCode);
				resTable.SetCell(10,*iteID,strName);
			}*/
			resTable.GetCell(9,*iteID,strCode);
			strCode = strCode.Left(strCode.GetLength()-3);
			resTable.SetCell(9,*iteID,strCode);
			resTable.SetCell(5,*iteID,equity2);
		}
	}
#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//增加基金的交易实体ID和名称，代码
	std::vector<int>::iterator iterId;
	std::unordered_map<int,double> dDataMap;//存放临时的基金净值	
	std::vector<int>::iterator iterDate;
	for(iterId = iSecurityId.begin();iterId != iSecurityId.end();++iterId)
	{
		int fundId;
		CString strName,strCode;
		GetSecurityNow(*iterId);
		if(m_pSecurity == NULL)
			continue;
		fundId = m_pSecurity->GetSecurity1Id();
		strName = m_pSecurity->GetName();
		strCode = m_pSecurity->GetCode();
		if(!vecInstiID.empty())
			vecInstiID.clear();
		resTable.Find(3,fundId,vecInstiID);
		for(iteID = vecInstiID.begin();iteID != vecInstiID.end();++iteID)
		{
			resTable.SetCell(0,*iteID,*iterId);
			resTable.SetCell(1,*iteID,strName);
			resTable.SetCell(2,*iteID,strCode);
		}
		if(iType >=16)
		{
			//把所有相同的报告期的基金净值加起来。这是为了样本汇总时作准备。			
			int itempdate;
			std::vector<UINT> TempCollect;
			//填充dDataMap，把各个报告期的基金净值设为零
			for(iterDate = iDate.begin();iterDate != iDate.end();++iterDate)
			{
				dDataMap.insert(std::make_pair(*iterDate,0));
			}
			for(iterDate = iDate.begin();iterDate != iDate.end();++iterDate)
			{
				itempdate = *iterDate;
				if(!TempCollect.empty())
					TempCollect.clear();
				resTable.Find(4,itempdate,TempCollect);//取得与报告期相同的纪录的位置。
				if(TempCollect.empty())
					continue;
				std::set<UINT> positionSet(vecInstiID.begin(),vecInstiID.end());
				iteID = TempCollect.begin();
				for(;iteID != TempCollect.end();++iteID)
				{
					if(positionSet.find(*iteID) != positionSet.end())
					{
						resTable.GetCell(5,*iteID,equity2);
						dDataMap[itempdate] += equity2; 
						break;
					}
					else
						continue;
				}
			}			
		}
	}
    int iType2 = iType;//保存原来的值.
	//这里增加样本分类选择  /*下面的注释都是按重仓债的统计写的，因为下面的代码是从哪里拷贝过来的*/
	if(iType >1)
	{
		int iCount = resTable.GetRowCount();//累加之前resTable里的行数。		
		//为了进行分类统计。所以增加一列基金ID。
		resTable.AddCol(Tx::Core::dtype_int4);//基金风格
		resTable.AddCol(Tx::Core::dtype_int4);//基金管理公司ID
		resTable.AddCol(Tx::Core::dtype_int4);//基金托管银行ID		
		//添加进度条
		prw.SetPercent(pid,0.6);
		//这里增加样本分类选择
		m_txTable.Clear();
		tempTable.Clear();
		//准备样本集=第一参数列:F_FUND_ID,int型
		m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
		const int indicatorIndex1 = 3;
		long iIndicator1[indicatorIndex1] = 
		{
			//30001035,	//基金风格
			//modified by zhangxs 20091221---NewStyle
			30001232,	//基金风格New
			30001020,	//管理公司ID，
			30001021	//托管银行ID，
		};
		UINT varCfg1[1];			//参数配置
		int varCount1=1;			//参数个数
		for (int i = 0; i < indicatorIndex1; i++)
		{
			int tempIndicator = iIndicator1[i];
			GetIndicatorDataNow(tempIndicator);
			if (m_pIndicatorData==NULL)
				return false; 
			varCfg1[0]=0;
			result = m_pLogicalBusiness->SetIndicatorIntoTable(
				m_pIndicatorData,	//指标
				varCfg1,				//参数配置
				varCount1,			//参数个数
				m_txTable	//计算需要的参数传输载体以及计算后结果的载体
				);
			if(result==false)
			{
				return FALSE;
			}

		}
		result = m_pLogicalBusiness->GetData(m_txTable,true);
		if(result==false)
			return false;
#ifdef _DEBUG
		strTable=m_txTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
		UINT iCol2=m_txTable.GetColCount();
		//复制所有列信息
		tempTable.CopyColumnInfoFrom(m_txTable);
		if(m_txTable.GetRowCount()==0)
		{
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false;
		}
		UINT* nColArray2 = new UINT[iCol2];
		for(UINT i=0;i<iCol2;i++)
			nColArray2[i]=i;
//#ifdef _SECURITYID_FOR_STAT_
		m_txTable.EqualsAt(tempTable,nColArray2,iCol2,0,iSecurityId);
/*#else
		m_txTable.EqualsAt(tempTable,nColArray2,iCol2,0,iSecurity1Id);
#endif	*/	
		delete nColArray2;
		nColArray2 = NULL;
		//把基金风格、公司ID、托管银行ID放到resTable
		std::vector<UINT> vecInstiID2;
		std::vector<UINT>::iterator iteID;		
		int iStyle,iCompanyId,ibankId;
		int tempfundId;
		int icount = tempTable.GetRowCount();
		UINT position;
		for(int j = 0;j < icount;j++)
		{
			tempTable.GetCell(0,j,tempfundId);
			tempTable.GetCell(1,j,iStyle);
			tempTable.GetCell(2,j,iCompanyId);
			tempTable.GetCell(3,j,ibankId);
			//把基金ID和交易实体ID对应起来。并且把数据放到表里。
			if(!(vecInstiID2.empty()))
				vecInstiID2.clear();
//#ifdef _SECURITYID_FOR_STAT_
			resTable.Find(0,tempfundId,vecInstiID2);
//#else
//			resTable.Find(3,tempfundId,vecInstiID2);
//#endif
			
			for(iteID = vecInstiID2.begin();iteID != vecInstiID2.end();++iteID)
			{
				position = *iteID;
				resTable.SetCell(14,position,iStyle);//基金风格
				resTable.SetCell(15,position,iCompanyId);//管理公司ID
				resTable.SetCell(16,position,ibankId);//托管银行ID
			}
		}		
		std::vector<int> ColVector;		//根据哪些列进行统计  
		if(!(iType & 1))
			iType += 1;//假设都选择选择了单只基金那一项分类方式。
		if(iType & 2)
			ColVector.push_back(15);
		if(iType & 4)
			ColVector.push_back(14);
		if(iType & 8)
			ColVector.push_back(16);
		std::vector<int> IntCol;			//需要相加的整型列
//		IntCol.push_back(10);
		std::vector<int> DoubleCol;	//需要相加的double列
		DoubleCol.push_back(11);
		DoubleCol.push_back(12);
		/*if (iType2 < 17)
		DoubleCol.push_back(5);*/
		/*for(int j = 6;j < 17;j++)
		{
		DoubleCol.push_back(j);
		j++;
		}*/
		int iTradeId = 8;//之所以这样做，是为了让它带回汇总样本统计的纪录数。
		AddUpRow(resTable,iType,ColVector,IntCol,DoubleCol,iDate,4,5,3,iTradeId,12,7);
		//		std::vector<int>::size_type isize = iDate.size();
		int iCount1 = resTable.GetRowCount();//累加以后resTable里的行数。
		int iCount2,iCount3;
		iCount2 = iCount1 - iCount;//这是增加的行数 样本汇总和其他分类方式的行数。
		std::unordered_map<int,CString> CompanyMap;
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_INSTITUTIONID_TO_SHORTNAME,CompanyMap);
		std::unordered_map<int,CString> StyleMap;
		//modified by zhangxs 20091221---NewStyle
		//TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX,StyleMap);
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX_NEW,StyleMap);
		std::unordered_map<int,CString> BankMap;
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_BANK,BankMap);
		std::unordered_map<int,CString>::iterator iterMap;
		int id;
		CString Name;	
		if(iType < 17)//没有样本汇总的情况
		{
			for(int m = 0;m < iCount2;m++)
			{
				position = iCount+m;
				resTable.GetCell(14,position,id);
				if(id == 0)
				{
					resTable.GetCell(15,position,id);
					if(id == 0)
					{
						resTable.GetCell(16,position,id);
						iterMap = BankMap.find(id);
						if(iterMap!= BankMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,position,Name);
						resTable.SetCell(2,position,(CString)(_T("托管银行")));
						continue;
					}
					else
					{
						iterMap = CompanyMap.find(id);
						if(iterMap!= CompanyMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,position,Name);
						resTable.SetCell(2,position,(CString)(_T("基金公司")));
						continue;
					}					
				}
				else
				{
					iterMap = StyleMap.find(id);
					if(iterMap!= StyleMap.end())
						Name = iterMap->second;
					else
						Name = Tx::Core::Con_strInvalid;
					resTable.SetCell(1,position,Name);
					resTable.SetCell(2,position,(CString)(_T("基金风格")));
				}
			}				
		}
		else//不但有单只基金和样本汇总，还有其他的分类方式
		{
			int isize = iTradeId;//这是样本汇总的行数.
			iCount3 = iCount2 - isize;//除了样本汇总以外的其他分类方式的行数
			if(iCount3 > 0)//表示除了样本汇总以外还有其他的分类的方式
			{
				//填充样本汇总的那些行
				for(int k = 0;k < isize;k++)
				{
					resTable.SetCell(1,iCount+iCount3+k,(CString)(_T("样本汇总")));
					resTable.SetCell(2,iCount+iCount3+k,(CString)(_T("全部汇总")));
				}
				//填充除了样本汇总以外的其他分类方式的那些行。
				for(int i = 0;i < iCount3;i++)
				{
					position = iCount+i;
					resTable.GetCell(14,position,id);
					if(id == 0)
					{
						resTable.GetCell(15,position,id);
						if(id == 0)
						{
							resTable.GetCell(16,position,id);
							iterMap = BankMap.find(id);
							if(iterMap!= BankMap.end())
								Name = iterMap->second;
							else
								Name = Tx::Core::Con_strInvalid;
							resTable.SetCell(1,position,Name);
							resTable.SetCell(2,position,(CString)(_T("托管银行")));
							continue;
						}
						else
						{
							iterMap = CompanyMap.find(id);
							if(iterMap!= CompanyMap.end())
								Name = iterMap->second;
							else
								Name = Tx::Core::Con_strInvalid;
							resTable.SetCell(1,position,Name);
							resTable.SetCell(2,position,(CString)(_T("基金公司")));
							continue;
						}					
					}
					else
					{
						iterMap = StyleMap.find(id);
						if(iterMap!= StyleMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,position,Name);
						resTable.SetCell(2,position,(CString)(_T("基金风格")));						
					}			
				}
			}
			else//只有单只基金和汇总的情况
			{				
				for(int k = 0;k < isize;k++)
				{
					position = iCount+k;
					resTable.SetCell(1,position,(CString)(_T("样本汇总")));
					resTable.SetCell(2,position,(CString)(_T("全部汇总")));			
				}		
			}
		}	
		//把报告年份和报告期转化成CString类型,所以增加一列
		resTable.InsertCol(5,Tx::Core::dtype_val_string);//str报告期	
		CString stryear,strdate;
		int tempdate,iyear ,itemp;
		int tempCount;
		int n = 0;
		if(!(iType2 & 1))
		{
			resTable.DeleteRow(0,iCount);
			tempCount = iCount2;
			n = 0;
		}
		else
		{
			for(int i = 0;i < iCount;i++)
			{
				//填写报告期	
				resTable.GetCell(4,i,tempdate);	
				itemp = tempdate%10000;
				iyear = tempdate/10000;
				stryear.Format(_T("%d"),iyear);
				switch(itemp)
				{
				case 331:
					strdate = stryear + _T("年") + _T("一季报");
					break;
				case 630:
					strdate = stryear + _T("年") + _T("二季报");
					break;
				case 930:
					strdate = stryear + _T("年") + _T("三季报");
					break;
				case 1231:
					strdate = stryear + _T("年") + _T("四季报");
					break;
				}
				resTable.SetCell(5,i,strdate);			
			}
			tempCount = iCount1;	
			n = iCount;
		}
		double proportion,Value;
		std::unordered_map<int,double>::iterator iterMap2;
		for(;n < tempCount;n++)
		{
			//填写报告期	
			resTable.GetCell(4,n,tempdate);	
			itemp = tempdate%10000;
			iyear = tempdate/10000;
			stryear.Format(_T("%d"),iyear);
			switch(itemp)
			{
			case 331:
				strdate = stryear + _T("年") + _T("一季报");
				break;
			case 630:
				strdate = stryear + _T("年") + _T("二季报");
				break;
			case 930:
				strdate = stryear + _T("年") + _T("三季报");
				break;
			case 1231:
				strdate = stryear + _T("年") + _T("四季报");
				break;
			}			
			//把样本汇总的基金净值放到resTable
			if(iType >=17 && n >= tempCount - iTradeId)
			{
				iterMap2 = dDataMap.find(tempdate);
				if(iterMap2 != dDataMap.end())
				{
					equity2 = iterMap2->second;
					resTable.SetCell(6,n,equity2);
					//计算比例					
					resTable.GetCell(13,n,Value);//取得债券的市值
					if(equity2 < 0 || Value < 0)
						proportion = Tx::Core::Con_doubleInvalid;
					proportion = Value*100/equity2;			
					resTable.SetCell(14,n,proportion);			
				}
			}
			else
			{
				//计算比例
				resTable.GetCell(6,n,equity2);//取得资产净值
				resTable.GetCell(13,n,Value);//取得债券的市值
				if(equity2 < 0 || Value < 0)
					proportion = Tx::Core::Con_doubleInvalid;
				proportion = Value*100/equity2;			
				resTable.SetCell(14,n,proportion);			
			}
			resTable.SetCell(5,n,strdate);

		}
		resTable.DeleteCol(15,3);
	}
	else
	{
		//添加进度条
		prw.SetPercent(pid,0.7);
		//把报告年份和报告期转化成CString类型,所以增加一列
		resTable.InsertCol(5,Tx::Core::dtype_val_string);//str报告期
		//把报告年份和报告期转化为CString类型
		int tempdate,iyear,tempdate2;
		CString stryear,strdate;
		for(int k = 0;k < (int)resTable.GetRowCount();k++)
		{			
			resTable.GetCell(4,k,tempdate);			
			iyear = tempdate/10000;
			stryear.Format(_T("%d"),iyear);
			tempdate2 = tempdate%10000;
			switch(tempdate2)
			{
			case 331:
				strdate = stryear + _T("年") + _T("一季报");
				break;
			case 630:
				strdate = stryear + _T("年") + _T("二季报");
				break;
			case 930:
				strdate = stryear + _T("年") + _T("三季报");
				break;
			case 1231: 
				strdate = stryear + _T("年") + _T("四季报");
				break;
			}
			resTable.SetCell(5,k,strdate);
		}

	}
#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	resTable.DeleteCol(9);//删除债券交易实体ID  
	resTable.DeleteCol(7);
	//resTable.DeleteCol(4);
	resTable.DeleteCol(3);//删除基金ID
	resTable.Arrange();
	MultiSortRule multisort;
	multisort.AddRule(3,false);
	multisort.AddRule(1,true);	
	multisort.AddRule(6,true);
	resTable.SortInMultiCol(multisort);
	resTable.Arrange();
	resTable.DeleteCol(3);//删除合并的报告年份和报告期

#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif	
	/*if(iType2 > 17)
	{
		resTable.SetSortRange(0,resTable.GetRowCount()-2);
	}*/
	//添加进度条
	prw.SetPercent(pid,1.0);
	return true;
}



////交易佣金
//add by lijw 2008-02-28
bool TxFund::StatTradeCommission(
		Tx::Core::Table_Indicator &resTable,
		std::vector<int>	iSecurityId,
		std::vector<int>	iDate,
		int		iType
		)
{
	//添加进度条
//	ProgressWnd* pwd=Tx::Core::ProgressWnd::GetInstance();
	ProgressWnd prw;
	UINT pid=prw.AddItem(1,_T("交易佣金统计..."),0.0);
	prw.Show(1000);

	//默认的返回值状态
	bool result = false;
	//从T_FUND_TRADE_ACTIVITY取其他的数据
	Tx::Core::Table_Indicator TradeTable;
	//准备样本集参数列
	TradeTable.AddParameterColumn(Tx::Core::dtype_int4);//基金ID
	TradeTable.AddParameterColumn(Tx::Core::dtype_int4);//报告年份
	TradeTable.AddParameterColumn(Tx::Core::dtype_int4);//报告期
	TradeTable.AddParameterColumn(Tx::Core::dtype_int4);//序号
	const int indicatorIndex3 = 5;
	long iIndicator3[indicatorIndex3] = 
	{
		30901123,	//券商名称id
		30901125,	//债券佣金
		30901124,	//股票交易量
		30901126,	//债券交易量
		30901127	//回购交易量
	};
	UINT varCfg3[4];			//参数配置
	int varCount3=4;			//参数个数
	for (int i = 0; i < indicatorIndex3; i++)
	{
		int tempIndicator = iIndicator3[i];

		GetIndicatorDataNow(tempIndicator);
		if (m_pIndicatorData==NULL)
		{ 
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false;
		}
		varCfg3[0]=0;
		varCfg3[1]=1;
		varCfg3[2]=2;
		varCfg3[3]=3;
		result = m_pLogicalBusiness->SetIndicatorIntoTable(
			m_pIndicatorData,	//指标
			varCfg3,				//参数配置
			varCount3,			//参数个数
			TradeTable	//计算需要的参数传输载体以及计算后结果的载体
			);
		if(result==false)
		{ 
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false;
		}

	}

	result = m_pLogicalBusiness->GetData(TradeTable,true);
#ifdef _DEBUG
	UINT nColCount = TradeTable.GetColCount();
    UINT nRowCount = TradeTable.GetRowCount();
	CString strTable11 = TradeTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard( strTable11 );
#endif
	UINT iColCount3 = TradeTable.GetColCount();
	UINT* nColArray3 = new UINT[iColCount3];
	for(int i = 0; i < (int)iColCount3; i++)
	{
		nColArray3[i] = i;
	}
	Tx::Core::Table_Indicator tempTable;
	tempTable.CopyColumnInfoFrom(TradeTable);
	////把交易实体ID转化为基金ID
	std::set<int> iSecurity1Id;
	std::vector<int> tempTradeV;
	std::vector<int>::iterator iterV;
	int iFunId;
	for (iterV = iSecurityId.begin();iterV != iSecurityId.end();++iterV)
	{
		GetSecurityNow(*iterV);
		if (m_pSecurity != NULL)
		{
			iFunId = m_pSecurity->GetSecurity1Id();
			if(iSecurity1Id.find(iFunId) == iSecurity1Id.end())
			{
				iSecurity1Id.insert(iFunId);
				tempTradeV.push_back(*iterV);
			}			
		}
	}
	iSecurityId.clear();
	iSecurityId.assign(tempTradeV.begin(),tempTradeV.end());
	TradeTable.EqualsAt(tempTable,nColArray3,iColCount3,0,iSecurity1Id);
#ifdef _DEBUG
	CString strTable=tempTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	if(tempTable.GetRowCount() == 0)
	{
		delete nColArray3;
		nColArray3 = NULL;
		//添加进度条
		prw.SetPercent(pid,1.0);
		return false;
	}
	//为了进行年度和报告期的筛选，所以把报告年份和报告期和为一列。
	TransReportDateToNormal2(tempTable,1);
	resTable.CopyColumnInfoFrom(tempTable);
	tempTable.EqualsAt(resTable,nColArray3,iColCount3 - 1,1,iDate);
	delete nColArray3;
	nColArray3 = NULL;
	if(resTable.GetRowCount() == 0)
	{
		//添加进度条
		prw.SetPercent(pid,1.0);
		return false;
	}
#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//为了提高查找的速度，先在resTable 里找出所需全部的券商ID，为了后面以它为条件进行筛选作准备
	int bondSid3;
	std::set<int> bondidSet;
	for(int j = 0;j < (int)resTable.GetRowCount();j++)
	{
		resTable.GetCell(3,j,bondSid3);
        bondidSet.insert(bondSid3);
	}

	m_txTable.Clear();//这是引用别人的成员变量，
	//从券商信息索引里取券商ID

	//准备样本集参数列
	m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//券商ID
	const int indicatorIndex = 1;
	long iIndicator[indicatorIndex] = 
	{
		30001064	//券商名称
	};
	UINT varCfg[1];			//参数配置
	int varCount=1;			//参数个数
	for (int i = 0; i < indicatorIndex; i++)
	{
		int tempIndicator = iIndicator[i];

		GetIndicatorDataNow(tempIndicator);
		if (m_pIndicatorData==NULL)
		{ return false; }
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
	UINT iColCount = m_txTable.GetColCount();
	UINT* nColArray = new UINT[iColCount];
	for(int i = 0; i < (int)iColCount; i++)
	{
		nColArray[i] = i;
	}
	result = m_pLogicalBusiness->GetData(m_txTable,true);
#ifdef _DEBUG
	strTable=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
    tempTable.Clear();
	tempTable.CopyColumnInfoFrom(m_txTable);
	//通过券商ID进行筛选。
	m_txTable.EqualsAt(tempTable,nColArray,iColCount,0,bondidSet);
	delete nColArray;
	nColArray = NULL;
	m_txTable.Clear();
	m_txTable.CopyColumnInfoFrom(tempTable);
	m_txTable.Clone(tempTable);
#ifdef _DEBUG
	strTable=m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//添加进度条
	prw.SetPercent(pid,0.3);
	//为增加基金的交易实体ID和名称，代码作准备
	resTable.InsertCol(0,Tx::Core::dtype_int4);//基金交易实体ID
	resTable.InsertCol(1,Tx::Core::dtype_val_string);//基金名称
	resTable.InsertCol(2,Tx::Core::dtype_val_string);//基金代码
	//为把m_txTable的内容放到resTable表里作准备。
	resTable.InsertCol(7,Tx::Core::dtype_val_string);//券商名称
	//把券商名称添加到resTable里
	int bondSid1,bondSid5;
	bondSid1 = bondSid5 = 0;
	CString strname;
	std::vector<UINT>vecID1,vecID2,vecID3;
	std::vector<UINT>::iterator iteID1,iteID2,iteID3;
	for(int m = 0;m < (int)resTable.GetRowCount();m++)
	{
		resTable.GetCell(6,m,bondSid1);//取出券商ID
		if(bondSid1 != bondSid5)
		{
			bondSid5 = bondSid1;
			vecID1.clear();
			vecID2.clear();
			vecID3.clear();
			m_txTable.Find(0,bondSid1,vecID1);//查找相等的bondSid1
			if(vecID1.size() == 0)
			{
				bondSid1 = bondSid5 = 0;
				continue;
			}
#ifdef _DEBUG
			if(vecID1.size() != 1)
			{
				AfxMessageBox(_T("数据出现错误"));
				return false;
			}
#endif
			iteID1 = vecID1.begin();
			m_txTable.GetCell(1,*iteID1,strname);
			resTable.SetCell(7,m,strname);
		}
		else
		{
#ifdef _DEBUG
			if(vecID1.size() != 1)
			{
				AfxMessageBox(_T("数据出现错误"));
				return false;
			}
#endif
			iteID1 = vecID1.begin();
			m_txTable.GetCell(1,*iteID1,strname);
			resTable.SetCell(7,m,strname);
		}
	}

#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//增加基金的交易实体ID和名称，代码
	std::vector<int>::iterator iterId;
	int fundId  ;
	CString strName,strCode;
	std::vector<UINT> vecInstiID;
	std::vector<UINT> vecInstiID2;
	std::vector<UINT> vecInstiID3;
	std::vector<UINT>::iterator iteID,iteID4;
	std::vector<int>::iterator iterTemp;
	int icount = 1;//序号
	//为了添加序号，
	MultiSortRule multisort;
	multisort.AddRule(4,false);
	multisort.AddRule(3,true);
	multisort.AddRule(8,false);
	resTable.SortInMultiCol(multisort);
	resTable.Arrange();
#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//resTable.Sort(8,false);
	//resTable.Arrange();
	for(iterTemp = iDate.begin();iterTemp != iDate.end();++iterTemp)
	{
		if(!vecInstiID2.empty())
			vecInstiID2.clear();
		resTable.Find(4,*iterTemp,vecInstiID2);
		for(iterId = iSecurityId.begin();iterId != iSecurityId.end();++iterId)
		{
			GetSecurityNow(*iterId);
			if(m_pSecurity == NULL)
				continue;
			fundId = m_pSecurity->GetSecurity1Id();
			strName = m_pSecurity->GetName();
			strCode = m_pSecurity->GetCode();
			if(!vecInstiID3.empty())
				vecInstiID3.clear();
			resTable.Find(3,fundId,vecInstiID3);
			if(!vecInstiID.empty())
				vecInstiID.clear();
			for(iteID4 = vecInstiID3.begin();iteID4 != vecInstiID3.end();++iteID4)
			{			
				if(find(vecInstiID2.begin(),vecInstiID2.end(),*iteID4) != vecInstiID2.end())
				{					
					vecInstiID.push_back(*iteID4);
				}
			}
			icount = 1;//序号
			for(iteID = vecInstiID.begin();iteID != vecInstiID.end();++iteID)
			{
				resTable.SetCell(0,*iteID,*iterId);
				resTable.SetCell(1,*iteID,strName);
				resTable.SetCell(2,*iteID,strCode);
				resTable.SetCell(5,*iteID,icount++);
			}

		}
	}	
	int iType2 = iType;//保存原来的值.
	//这里增加样本分类选择  /*下面的注释都是按重仓债的统计写的，因为下面的代码是从哪里拷贝过来的*/
	if(iType >1)
	{
		int iCount = resTable.GetRowCount();//累加之前resTable里的行数。		
		//为了进行分类统计。所以增加一列基金ID。
		resTable.AddCol(Tx::Core::dtype_int4);//基金风格
		resTable.AddCol(Tx::Core::dtype_int4);//基金管理公司ID
		resTable.AddCol(Tx::Core::dtype_int4);//基金托管银行ID		
		//添加进度条
		prw.SetPercent(pid,0.6);
		//这里增加样本分类选择
		m_txTable.Clear();
		tempTable.Clear();
		//准备样本集=第一参数列:F_FUND_ID,int型
		m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
		const int indicatorIndex1 = 3;
		long iIndicator1[indicatorIndex1] = 
		{
			//30001035,	//基金风格
			//modified by zhangxs 20091221---NewStyle
			30001232,	//基金风格New
			30001020,	//管理公司ID，
			30001021	//托管银行ID，
		};
		UINT varCfg1[1];			//参数配置
		int varCount1=1;			//参数个数
		for (int i = 0; i < indicatorIndex1; i++)
		{
			int tempIndicator = iIndicator1[i];
			GetIndicatorDataNow(tempIndicator);
			if (m_pIndicatorData==NULL)
				return false; 
			varCfg1[0]=0;
			result = m_pLogicalBusiness->SetIndicatorIntoTable(
				m_pIndicatorData,	//指标
				varCfg1,				//参数配置
				varCount1,			//参数个数
				m_txTable	//计算需要的参数传输载体以及计算后结果的载体
				);
			if(result==false)
			{
				return FALSE;
			}

		}
		result = m_pLogicalBusiness->GetData(m_txTable,true);
		if(result==false)
			return false;
#ifdef _DEBUG
		strTable=m_txTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
		UINT iCol2=m_txTable.GetColCount();
		//复制所有列信息
		tempTable.CopyColumnInfoFrom(m_txTable);
		if(m_txTable.GetRowCount()==0)
		{
			//添加进度条
			prw.SetPercent(pid,1.0);
			return false;
		}
		UINT* nColArray2 = new UINT[iCol2];
		for(UINT i=0;i<iCol2;i++)
			nColArray2[i]=i;
//#ifdef _SECURITYID_FOR_STAT_
		m_txTable.EqualsAt(tempTable,nColArray2,iCol2,0,iSecurityId);
/*#else
		m_txTable.EqualsAt(tempTable,nColArray2,iCol2,0,iSecurity1Id);
#endif	*/	
		delete nColArray2;
		nColArray2 = NULL;
		//把基金风格、公司ID、托管银行ID放到resTable
		std::vector<UINT> vecInstiID2;
		std::vector<UINT>::iterator iteID;		
		int iStyle,iCompanyId,ibankId;
		int tempfundId;
		int icount = tempTable.GetRowCount();
		UINT position;
		for(int j = 0;j < icount;j++)
		{
			tempTable.GetCell(0,j,tempfundId);
			tempTable.GetCell(1,j,iStyle);
			tempTable.GetCell(2,j,iCompanyId);
			tempTable.GetCell(3,j,ibankId);
			//把基金ID和交易实体ID对应起来。并且把数据放到表里。
			if(!(vecInstiID2.empty()))
				vecInstiID2.clear();
//#ifdef _SECURITYID_FOR_STAT_
			resTable.Find(0,tempfundId,vecInstiID2);
//#else
//			resTable.Find(3,tempfundId,vecInstiID2);
//#endif
			
			for(iteID = vecInstiID2.begin();iteID != vecInstiID2.end();++iteID)
			{
				position = *iteID;
				resTable.SetCell(12,position,iStyle);//基金风格
				resTable.SetCell(13,position,iCompanyId);//管理公司ID
				resTable.SetCell(14,position,ibankId);//托管银行ID
			}
		}		
		std::vector<int> ColVector;		//根据哪些列进行统计  
		if(!(iType & 1))
			iType += 1;//假设都选择选择了单只基金那一项分类方式。
		if(iType & 2)
			ColVector.push_back(13);
		if(iType & 4)
			ColVector.push_back(12);
		if(iType & 8)
			ColVector.push_back(14);
		std::vector<int> IntCol;			//需要相加的整型列
		//		IntCol.push_back(6);
		std::vector<int> DoubleCol;	//需要相加的double列
//		DoubleCol.push_back(11);
		/*if (iType2 < 17)
		DoubleCol.push_back(5);*/
		for(int j = 8;j < 12;j++)
		{
			DoubleCol.push_back(j);
		}
		int iTradeId = 6;//之所以这样做，是为了让它带回汇总样本统计的纪录数。
		//AddUpRow(resTable,iType,ColVector,IntCol,DoubleCol,iDate,4,iTradeId,8,5);
		//调用新的接口，解决按基金公司统计全称和简称问题 modified by zhangxs 20080108
		AddUpRowEX(resTable,iType,ColVector,IntCol,DoubleCol,iDate,4,iTradeId,8,5);
		//		std::vector<int>::size_type isize = iDate.size();
		int iCount1 = resTable.GetRowCount();//累加以后resTable里的行数。
		int iCount2,iCount3;
		iCount2 = iCount1 - iCount;//这是增加的行数 样本汇总和其他分类方式的行数。
		std::unordered_map<int,CString> CompanyMap;
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_INSTITUTIONID_TO_SHORTNAME,CompanyMap);
		std::unordered_map<int,CString> StyleMap;
		//modified by zhangxs 20091221---NewStyle
		//TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX,StyleMap);
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX_NEW,StyleMap);
		std::unordered_map<int,CString> BankMap;
		TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_BANK,BankMap);
		std::unordered_map<int,CString>::iterator iterMap;
		int id;
		CString Name;	
		if(iType < 17)//没有样本汇总的情况
		{
			for(int m = 0;m < iCount2;m++)
			{
				position = iCount+m;
				resTable.GetCell(12,position,id);
				if(id == 0)
				{
					resTable.GetCell(13,position,id);
					if(id == 0)
					{
						resTable.GetCell(14,position,id);
						iterMap = BankMap.find(id);
						if(iterMap!= BankMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,position,Name);
						resTable.SetCell(2,position,(CString)(_T("托管银行")));
						continue;
					}
					else
					{
						iterMap = CompanyMap.find(id);
						if(iterMap!= CompanyMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,position,Name);
						resTable.SetCell(2,position,(CString)(_T("基金公司")));
						continue;
					}					
				}
				else
				{
					iterMap = StyleMap.find(id);
					if(iterMap!= StyleMap.end())
						Name = iterMap->second;
					else
						Name = Tx::Core::Con_strInvalid;
					resTable.SetCell(1,position,Name);
					resTable.SetCell(2,position,(CString)(_T("基金风格")));
				}
			}				
		}
		else//不但有单只基金和样本汇总，还有其他的分类方式
		{
			int isize = iTradeId;//这是样本汇总的行数.
			iCount3 = iCount2 - isize;//除了样本汇总以外的其他分类方式的行数
			if(iCount3 > 0)//表示除了样本汇总以外还有其他的分类的方式
			{
				//填充样本汇总的那些行
				for(int k = 0;k < isize;k++)
				{
					resTable.SetCell(1,iCount+iCount3+k,(CString)(_T("样本汇总")));
					resTable.SetCell(2,iCount+iCount3+k,(CString)(_T("全部汇总")));
				}
				//填充除了样本汇总以外的其他分类方式的那些行。
				for(int i = 0;i < iCount3;i++)
				{
					position = iCount+i;
					resTable.GetCell(12,position,id);
					if(id == 0)
					{
						resTable.GetCell(13,position,id);
						if(id == 0)
						{
							resTable.GetCell(14,position,id);
							iterMap = BankMap.find(id);
							if(iterMap!= BankMap.end())
								Name = iterMap->second;
							else
								Name = Tx::Core::Con_strInvalid;
							resTable.SetCell(1,position,Name);
							resTable.SetCell(2,position,(CString)(_T("托管银行")));
							continue;
						}
						else
						{
							iterMap = CompanyMap.find(id);
							if(iterMap!= CompanyMap.end())
								Name = iterMap->second;
							else
								Name = Tx::Core::Con_strInvalid;
							resTable.SetCell(1,position,Name);
							resTable.SetCell(2,position,(CString)(_T("基金公司")));
							continue;
						}					
					}
					else
					{
						iterMap = StyleMap.find(id);
						if(iterMap!= StyleMap.end())
							Name = iterMap->second;
						else
							Name = Tx::Core::Con_strInvalid;
						resTable.SetCell(1,position,Name);
						resTable.SetCell(2,position,(CString)(_T("基金风格")));						
					}			
				}
			}
			else//只有单只基金和汇总的情况
			{				
				for(int k = 0;k < isize;k++)
				{
					position = iCount+k;
					resTable.SetCell(1,position,(CString)(_T("样本汇总")));
					resTable.SetCell(2,position,(CString)(_T("全部汇总")));			
				}		
			}
		}	
		//把报告年份和报告期转化成CString类型,所以增加一列
		resTable.InsertCol(5,Tx::Core::dtype_val_string);//str报告期	
		CString stryear,strdate;
		int tempdate,iyear ,itemp;
		int tempCount;
		int n = 0;
		if(!(iType2 & 1))
		{
			resTable.DeleteRow(0,iCount);
			tempCount = iCount2;
			n = 0;
		}
		else
		{
			for(int i = 0;i < iCount;i++)
			{
				//填写报告期	
				resTable.GetCell(4,i,tempdate);	
				itemp = tempdate%10000;
				iyear = tempdate/10000;
				stryear.Format(_T("%d"),iyear);
				switch(itemp)
				{			
				case 630:
					strdate = stryear + _T("年") + _T("中报");
					break;			
				case 1231:
					strdate = stryear + _T("年") + _T("年报");
					break;
				}
				resTable.SetCell(5,i,strdate);			
			}
			tempCount = iCount1;	
			n = iCount;
		}
		std::unordered_map<int,double>::iterator iterMap2;
		for(;n < tempCount;n++)
		{
			//填写报告期	
			resTable.GetCell(4,n,tempdate);	
			itemp = tempdate%10000;
			iyear = tempdate/10000;
			stryear.Format(_T("%d"),iyear);
			switch(itemp)
			{
			case 630:
				strdate = stryear + _T("年") + _T("中报");
				break;			
			case 1231:
				strdate = stryear + _T("年") + _T("年报");
				break;
			}			
			resTable.SetCell(5,n,strdate);
		}
		resTable.DeleteCol(13,3);
	}
	else
	{
		//添加进度条
		prw.SetPercent(pid,0.7);
		//把报告年份和报告期转化成CString类型,所以增加一列
		resTable.InsertCol(5,Tx::Core::dtype_val_string);//str报告期
		//把报告年份和报告期转化为CString类型
		int tempdate,iyear,tempdate2;
		CString stryear,strdate;
		for(int k = 0;k < (int)resTable.GetRowCount();k++)
		{			
			resTable.GetCell(4,k,tempdate);			
			iyear = tempdate/10000;
			stryear.Format(_T("%d"),iyear);
			tempdate2 = tempdate%10000;
			switch(tempdate2)
			{
			case 630:
				strdate = stryear + _T("年") + _T("中报");
				break;			
			case 1231:
				strdate = stryear + _T("年") + _T("年报");
				break;
			}
			resTable.SetCell(5,k,strdate);
		}

	}
	resTable.DeleteCol(7);//券商名称ID   
	resTable.DeleteCol(3);//删除合并后的报告年份和报告期以及基金ID
	resTable.Arrange();
	MultiSortRule multisort2;
	multisort2.AddRule(3,false);
	multisort2.AddRule(1,true);	
	multisort2.AddRule(5,true);	
	resTable.SortInMultiCol(multisort2);
	resTable.Arrange();
	resTable.DeleteCol(3);
#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	/*if(iType2 > 17)
	{
		resTable.SetSortRange(0,resTable.GetRowCount()-2);
	}*/
	//添加进度条
	prw.SetPercent(pid,1.0);
	return true;
}
	
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
//上面是业务方法，下面是辅助方法
bool TxFund::TransObjectToSecIns(
		std::vector<int>	iObjectId,		//交易实体id
		std::vector<int> &iSecInsId,		//券id或机构id
		int iType						//id类型：1:券id	2:机构id
		)
{
	int ii = iObjectId.size();
	if(ii<=0)
		return false;

	std::vector<int>::iterator iter=iObjectId.begin();
	
	while(iter!=iObjectId.end())
	{
		if(*iter>=20000000)//错把交易实体放到公司id
		{
			iSecInsId.push_back(*iter);
		}
		else
		{
			if(GetSecurityNow(*iter++)!=NULL)
			{
				switch(iType)
				{
				case 1:
					iSecInsId.push_back( m_pSecurity->GetSecurity1Id());
					break;
				case 2:
					iSecInsId.push_back( m_pSecurity->GetInstitutionId());
					break;
				default:
					;
				}
			}
		}//end else
	}
	return true;
}//End Of TransObjectToSecIns


//id列转为基金名称代码
//在id列原位置上添加一列
bool TxFund::IdColToNameAndCode(Tx::Core::Table_Indicator &resTable,int iCol,int iIdType,int iMethodType)
{
	//iIdType标记传入列数据是何种类型。0为实体，1为基金，2为公司
	//可以实现不依靠标记自动识别，待修改
	//公司id转基金id尚未添加
	if(resTable.GetColCount()==0)
	{
		return true;
	}
	std::unordered_map<int,CString> iInstitutionMap;
	std::unordered_map<int,CString>::iterator iterMap;
	if(iIdType==2)
		Tx::Data::TypeMapManage::GetInstance()->GetTypeMap(27,iInstitutionMap);
	if(resTable.GetColCount()==1)
	{
		resTable.AddCol(Tx::Core::dtype_val_string);	//公司名称
		resTable.AddCol(Tx::Core::dtype_val_string);	//公司外码
	}
	else
	{
		resTable.InsertCol(iCol+1,Tx::Core::dtype_val_string);	//公司名称
		resTable.InsertCol(iCol+2,Tx::Core::dtype_val_string);	//公司外码
	}
	std::set <int> items;
	int iId=0;
	std::set<int>::iterator iter; 

	for(UINT i=0;i<resTable.GetRowCount();i++)
	{
		int iSecId;

		//switch(iIdType)
		//{
		//case 0:
		//	resTable.GetCell(iCol,i,iObjId);
		//	GetSecurityNow(iObjId);
		//	iSecId= m_pSecurity->GetSecurity1Id();
		//	break;
		//case 1:
		//	resTable.GetCell(iCol,i,iSecId);
		//	break;
		//case 2:
		//	resTable.GetCell(iCol,i,iSecId);
		//	break;
		//default:
		//	ASSERT(0);
		//	resTable.GetCell(iCol,i,iSecId);
		//}
		resTable.GetCell(iCol,i,iSecId);
			
		/*if(iSecId>10000000)
		{
			items.clear();
			m_pFunctionDataManager->GetItems(iSecId,items);
		
			if((items.size()!=0)&&(iSecId<20000000))
			{
				iter=items.begin();
				iSecId=*iter;
			}
			else
			{
				iterMap=iInstitutionMap.find(iSecId);
				if(iterMap!=iInstitutionMap.end())
				{
					resTable.SetCell(iCol+1,i,iterMap->second);
					resTable.SetCell(iCol+2,i,(CString)"--");

				}
				
				CString s1,s2;
				long i1;
				Tx::Data::Collection *pCollection,mCollection;
				pCollection=m_pFunctionDataManager->GetCollection(iSecId);
				if(pCollection!=NULL)
				{

					pCollection->GetChildCollection(mCollection);

					if(pCollection!=NULL)
					{
						s1=pCollection->GetName();
						i1=pCollection->GetId();

						CString tmpS;
						tmpS.Format(_T("%d"),s2);
						resTable.SetCell(iCol+1,i,s1);
						resTable.SetCell(iCol+2,i,tmpS);
					}
					else
					{
						resTable.SetCell(iCol+1,i,CString(_T("--")));
						resTable.SetCell(iCol+2,i,CString(_T("--")));
					}
				}
				continue;
			}
		}*/

		GetSecurityNow(iSecId);
		if(m_pSecurity!=NULL)
		{
			resTable.SetCell(iCol+1,i,m_pSecurity->GetSecurity1Name());
			resTable.SetCell(iCol+2,i,m_pSecurity->GetCode());
		}
		
		
	}//end of  for()
	return true;
}


//分类汇总：范式接口
//通过调用单项式接口StatisticOnType分别对类型统计,注意他们的名字与StatisticsOnType只相差一个字母
//0:基金1:基金公司	2:基金风格	3:托管公司	4:汇总
//为了提高功能的粒子度，每一次调用功能只做一次报告期的查询
//多报告期的查询要多次调用，在上面的功能调用时，先建立报告期vector，以及一张汇总的空表
//对于个报告期进行查询并续接入汇总表
bool TxFund::StatisticsOnType(Tx::Core::Table_Indicator &resTable,int iClassifyTypes)
{

#ifdef _DEBUG
	CString strTable1;
	strTable1= resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable1);
#endif
	Tx::Core::Table_Indicator TableInstitution,TableType,TableBank,TableStat;

	
	//m_txTable.Clear();
	tmpTable.Clear();//辅助表,存储 iFundId,iInstitutionId,iTypeId,iBankId;
	int tmpId;
	//m_txTable.Clone(resTable);

	TableInstitution.CopyColumnInfoFrom(resTable);
	TableType.CopyColumnInfoFrom(resTable);
	TableBank.CopyColumnInfoFrom(resTable);
	TableStat.CopyColumnInfoFrom(resTable);
	
	
	{	//增加一个辅助表，存储了公司名称，公司id，托管银行id，基金风格
		m_FundStype.clear();
		m_IWPA.clear();
		m_IWPA.m_table.Clear();
		m_IWPA.m_table_indicator.Clear();
		std::vector<int> iParameter;

							//类型
		iParameter.push_back(30001016);	//公司简称   备注//这是基金的中文简称。add "备注"by lijw 2008-02-18
		iParameter.push_back(30001020);	//管理公司id
		iParameter.push_back(30001021);	//托管银行id	
		//iParameter.push_back(30001035);	//基金风格
		//modified by zhangxs 20091221---NewStyle
		iParameter.push_back(30001232);	//基金风格New

		//设定这里需要的指标
		Tx::Business::IndicatorFile::GetInstance()->SetIWAP(m_IWPA,iParameter);
		Tx::Business::IndicatorFile::GetInstance()->GetData(m_IWPA,iParameter,true);

		if(m_IWPA.m_table_indicator.GetRowCount()!=0)
			tmpTable.Clone(m_IWPA.m_table_indicator);
		else
			tmpTable.CopyColumnInfoFrom(m_IWPA.m_table_indicator);

		tmpTable.DeleteCol(2);
		tmpTable.DeleteCol(0);
	}
#ifdef _DEBUG
	CString strTable;
	strTable = tmpTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	if(iClassifyTypes&1)		
	{
		resTable.Clear();
		if(m_txTable.GetRowCount()!=0)
			resTable.Clone(m_txTable);
		else
			resTable.CopyColumnInfoFrom(m_txTable);
		this->IdColToNameAndCode(resTable,0,1);
	}

	else
	{
		resTable.Clear();
		resTable.CopyColumnInfoFrom(m_txTable);
		resTable.InsertCol(1,Tx::Core::dtype_val_string);
		resTable.InsertCol(2,Tx::Core::dtype_val_string);
	}
	Collection* pC = NULL;
	CString sName;

	if(iClassifyTypes&2)		
	{
		TableInstitution.InsertCol(1,Tx::Core::dtype_int4);
#ifdef _DEBUG
		strTable = TableInstitution.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
		StatisticOnType(TableInstitution,1);
#ifdef _DEBUG
		strTable = TableInstitution.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
		TableInstitution.InsertCol(2,Tx::Core::dtype_val_string);
		TableInstitution.InsertCol(3,Tx::Core::dtype_val_string);
		
		for(UINT i=0;i<TableInstitution.GetRowCount();i++)
		{
			TableInstitution.GetCell(1,i,tmpId);//在这里把管理公司ID的值传进去，就会得到公司的名字。
			pC = m_pFunctionDataManager->GetCollection(tmpId);
			sName = "-";
		
			if(pC!=NULL)
			{
				sName=pC->GetName();
		
			}
			TableInstitution.SetCell(2,i,sName);
			TableInstitution.SetCell(3,i,(CString)"基金公司");
			/*TableInstitution.SetCell(3,i,sCode);*/
		}

		TableInstitution.DeleteCol(1);
		resTable.AppendTableByRow(TableInstitution);
	}
#ifdef _DEBUG
	strTable = resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	if(iClassifyTypes&4)		
	{
		//从基金表构建风格表
			TableType.InsertCol(1,Tx::Core::dtype_int4);
			StatisticOnType(TableType,2);
		TableType.InsertCol(2,Tx::Core::dtype_val_string);
		TableType.InsertCol(3,Tx::Core::dtype_val_string);

		std::set<int>::iterator iter; 
		std::set<int> items;
		
		for(UINT i=0;i<TableType.GetRowCount();i++)
		{

			TableType.GetCell(1,i,tmpId);
			m_pFunctionDataManager->GetItems(tmpId,items);

			if(items.size()!=0)
			{
				iter= items.begin();
				tmpId=*iter;
				GetSecurityNow(tmpId);
			}
			if(m_pSecurity!=NULL)
			{
			//	if(m_pSecurity->IsFund_Stock())
				//modified by zhangxs 20091221---NewStyle
				//===============================================
				/*if(	m_pSecurity->IsFundStyle_GrowUp()||m_pSecurity->IsFundStyle_Banlance()||m_pSecurity->IsFundStyle_Worth()||
					m_pSecurity->IsFundStyle_Index()||m_pSecurity->IsFundStyle_Industry()||
					m_pSecurity->IsFundStyle_StockArea()||m_pSecurity->IsFundStyle_StockOther()
					)*/
				if(m_pSecurity->IsFundStyle_StockActive() || m_pSecurity->IsFundStyle_StockSteady()||
					m_pSecurity->IsFundStyle_StockIndex() || m_pSecurity->IsFundStyle_StockIndexEnhance())
					TableType.SetCell(2,i,(CString)"股票型");
			//	if(m_pSecurity->IsFund_Intermix())
				//modified by zhangxs 20091221---NewStyle
				//===============================================
				/*if(	m_pSecurity->IsFundStyle_FlexibleCombination()||m_pSecurity->IsFundStyle_Config()||
					m_pSecurity->IsFundStyle_IntermixOther()||m_pSecurity->IsFundStyle_IntermixArea()
					)*/
				if(	m_pSecurity->IsFundStyle_FlexibleCombination()||m_pSecurity->IsFundStyle_FundCombination()||
					m_pSecurity->IsFundStyle_CareCombination()||m_pSecurity->IsFundStyle_PreserCombination()
					||m_pSecurity->IsFundStyle_SpecialCombination())
					TableType.SetCell(2,i,(CString)"混合型");
			//	if(m_pSecurity->IsFund_Bond())
				if(	m_pSecurity->IsFundStyle_PureBond()||m_pSecurity->IsFundStyle_LeanBond()
					)
					TableType.SetCell(2,i,(CString)"债券型");
				if(m_pSecurity->IsFund_Currency())
					TableType.SetCell(2,i,(CString)"货币型");
				if(m_pSecurity->IsFund_Breakeven())
					TableType.SetCell(2,i,(CString)"保本型");
				if(m_pSecurity->IsFundStyle_Unknown())
					TableType.SetCell(2,i,(CString)"未知类型");

			}
			TableType.SetCell(3,i,(CString)"基金风格");
		}
		TableType.DeleteCol(1);
		resTable.AppendTableByRow(TableType);
	}
	
	if(iClassifyTypes&8)		
	{		
		TableBank.InsertCol(1,Tx::Core::dtype_int4);
		StatisticOnType(TableBank,3);
		TableBank.InsertCol(2,Tx::Core::dtype_val_string);
		TableBank.InsertCol(3,Tx::Core::dtype_val_string);
		for(UINT i=0;i<TableBank.GetRowCount();i++)
		{
			TableBank.GetCell(1,i,tmpId);
			pC = m_pFunctionDataManager->GetCollection(tmpId);
			sName = "-";
			if(pC!=NULL)
				sName=pC->GetName();
			TableBank.SetCell(2,i,sName);
			TableBank.SetCell(3,i,(CString)"托管银行");
		}
		TableBank.DeleteCol(1);
		resTable.AppendTableByRow(TableBank);
	}
	
	if(iClassifyTypes&16)		
	{	
		//从基金表构建汇总表
		TableStat.InsertCol(1,Tx::Core::dtype_int4);
		StatisticOnType(TableStat,4);
		TableStat.InsertCol(2,Tx::Core::dtype_val_string);
		TableStat.InsertCol(3,Tx::Core::dtype_val_string);
		for(UINT i=0;i<TableStat.GetRowCount();i++)
		{
			TableStat.SetCell(2,i,(CString)"样本汇总");
			TableStat.SetCell(3,i,(CString)"全部样本");
		}

		TableStat.DeleteCol(1);
		resTable.AppendTableByRow(TableStat);


	}

return true;
}


//根据取得的基金id，分布按其公司，风格，托管银行，全部汇总再次统计；
//应该还需要知道统计的日期以及指标的数量、详细内容；
//还可能要指定id列，当前写定为0；

//建立三张map->公司id ->托管银行
//              |->风格类型

                 
//实现流程：
//实现的方案是顺序实现，但是使用实现过程中发现，对每个标志位写一个函数冗余成分太多
//
//
//后使用递归
//这样的话代码简洁度大大提升，而效率不会下降太多，因为总的存储空间需求很小
//相对复杂的事需要设定较之前更多的辅助位，略微增加程序易懂性

//再次修改
//发现数据库设计在此处有更好的针对性，递归的优势度下降，因为代码简洁度上比分支设计稍强，但是递归本身的复杂性在此得不偿失
//原先的设计，几个数据的获取是有部分有序的，关系树深度为3，但现在的数据库表中，几个数据的关系树深度为2
//这样增加了数据获取的一致度，所以做成通用方法更为方便
//有一点不足是增加了一个表的存储（空间=基金个数*所需指标个数）
//
//辅助表的优缺点：
//	1、虽然表中基金id与其他id关系是多对一的（即一个基金id向上只会找到唯一内容，不会有分歧）
//		但是对应的数据在表格这样的存储条件下是无法一次查询得到的。要想通过id直接查询其所在行,
//		还要添加一个map，使基金id与其行号关联。这样由于map存储结构的优势可以实现查询上的高效性,
//		这个额外的map实际是为了提高效率而使用的。
//	2、基金id-〉公司id 这样的对应关系其实是可以通过缓存数据得到的，但是设计的时候还是采用了统一的模式
//		还有基金id-〉基金风格，基金id-〉托管银行。

//



//分类汇总：单项式接口
//此接口通常只被范式分类汇总：范式接口调用
//用于具体分类方式处理
bool TxFund::StatisticOnType(Tx::Core::Table_Indicator &Table4Kind,int iClassifyType)
{
	CString strTable;//add by lijw 2008-02-15
#ifdef _DEBUG
	strTable = Table4Kind.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	std::unordered_map<int,int> m_iTmpId;
	std::unordered_map<int,int>::iterator iter_iParentId;
	int tmpFundId;
	int iRowNumber;		//当公司id已经出现在map中，则用此标记记录map->second,即表中的位置。
		TableOutput(tmpTable);
	m_txTable.InsertCol(1,Tx::Core::dtype_int4);
#ifdef _DEBUG
	strTable = m_txTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	if((iClassifyType==1)||(iClassifyType==2)||(iClassifyType==3))
	{	
		for(UINT i=0;i<m_txTable.GetRowCount();i++)
		{
			int iFundId=0;
			int tmpId;
			m_txTable.GetCell(0,i,iFundId);	

			UINT j;
			for(j=0;j<tmpTable.GetRowCount();j++)
			{
				
				//2是公司，3是托管银行，4是风格

				tmpTable.GetCell(0,j,tmpFundId);
				if(tmpFundId==iFundId)
				{
					switch(iClassifyType)
						{
						case 1:
							tmpTable.GetCell(2,j,tmpId);
						//	Table4Kind.SetCell(0,i,tmpId);
							break;
						case 2:
							tmpTable.GetCell(4,j,tmpId);
						//	Table4Kind.SetCell(0,i,tmpId);
							break;
						case 3:
							tmpTable.GetCell(3,j,tmpId);
							break;
						}
					break;
				
				}
					
			}
	
			if(j==tmpTable.GetRowCount())
			{
				TRACE("从t_fund_basic表中搜索基金id没有找到，有错误\n");
				ASSERT(0);
				return false;
			}

			iter_iParentId=m_iTmpId.find(tmpId);

			if(iter_iParentId!=m_iTmpId.end())
			{
				iRowNumber=iter_iParentId->second;
									
				//累加该行数据到查询行iRowNumber;
				this->PlusRow(Table4Kind,m_txTable,iRowNumber,i);
			}
			else
			{
			
				//新添一行，并添加到map
				Table4Kind.AddRow();
				CopyRow(Table4Kind,m_txTable,(int)(Table4Kind.GetRowCount()-1),i);
#ifdef _DEBUG
				strTable = Table4Kind.TableToString();
				Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
				Table4Kind.SetCell(1,Table4Kind.GetRowCount()-1,tmpId);	
#ifdef _DEBUG
				strTable = Table4Kind.TableToString();
				Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
		//?g?Table4Kind.SetCell(0,Table4Kind.GetRowCount()-1,tmpId);
		//?g?到这里前代码应该已经添加上了，对应老系统应该把相应的名称添上
				//make_pair(iInstitutionId,(int)Table4Kind.GetRowCount());
				m_iTmpId.insert(std::make_pair(tmpId,(int)Table4Kind.GetRowCount()-1));
			}
		}

	}
	else
		if(iClassifyType==4)
		{
			Table4Kind.AddRow();
			
			this->CopyRow(Table4Kind,m_txTable,0,0);
			for(UINT j=1;j<m_txTable.GetRowCount();j++)
			{
				this->PlusRow(Table4Kind,m_txTable,0,j);
			}

		}

	//几个数据的计算公式
	//顺序为：基金净值、债券市值、债券占比、股票市值、股票占比、重仓股、重仓股比
	//债券占比=债券市值/基金净值
	//股票占比=股票市值/基金净值
	//重仓占比=重仓股市值/基金净值
	//持股集中度=重仓股市值/股票市值
#ifdef _DEBUG
		strTable = m_txTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
		m_txTable.DeleteCol(1);
#ifdef _DEBUG
		strTable = m_txTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	return true;
}//end of myfunction

//从sourceTable中取出数据行放入resTable中
bool TxFund::CopyRow(Table_Indicator &resTable,Table_Indicator &sourceTable,int iResRow,int iSourceRow)
	{
		Tx::Core::VariantData vData;
		if(resTable.GetColCount()!=sourceTable.GetColCount())
		{	
			TRACE("两表宽度不同\n");
			ASSERT(0);
			return false;
		}
		for(UINT i=0;i<resTable.GetColCount();i++)
		{
			if(resTable.GetColType(i)!=sourceTable.GetColType(i))
			{
				TRACE("两表数据类型有不一致，存在潜在的危险\n");
				ASSERT(0);
				return false;
			};
		}

		if(iResRow>(int)resTable.GetRowCount()||iSourceRow>(int)sourceTable.GetRowCount())
		{
			TRACE("选择的行超出了行数范围\n");
			ASSERT(0);
			return false;
		}
		//以上是错误保护
		for(UINT j=0;j<resTable.GetColCount();j++)
		{
			sourceTable.GetCell(j,iSourceRow,vData);
			//modify by lijw 2008-02-18 把下面的语句delete
			/*if(sourceTable.GetColType(j)!=Tx::Core::dtype_int4)
			{
				continue;
			}*/
			
			if(sourceTable.GetColType(j)==Tx::Core::dtype_decimal)
			{
				if(vData.numeric_data.double_value<100.1)
					continue;
			}	
			
			if(sourceTable.GetColType(j)==Tx::Core::dtype_double)
			{
				if(vData.numeric_data.double_value<100.1)
					continue;
			}
			resTable.SetCell(j,iResRow,vData);
		}

		
		return true;
	}//end of CopyRow
	
bool TxFund::PlusRow(Table_Indicator &resTable,Table_Indicator &sourceTable,int iResRow,int iSourceRow)
	{
		Tx::Core::VariantData vResData,vSourceData;
		if(resTable.GetColCount()!=sourceTable.GetColCount())
		{	
			TRACE("两表宽度不同\n");
			ASSERT(0);
			return false;
		}
		for(UINT i=0;i<resTable.GetColCount();i++)
		{
			if(resTable.GetColType(i)!=sourceTable.GetColType(i))
			{
				TRACE("两表数据类型有不一致，存在潜在的危险\n");
				ASSERT(0);
				return false;
			};
		}

		if(iResRow>(int)resTable.GetRowCount()||iSourceRow>(int)sourceTable.GetRowCount())
		{
			TRACE("选择的行超出了行数范围\n");
			ASSERT(0);
			return false;
		}
		//以上是错误保护
//		int tmpI;
		double tmpD;
		for(UINT j=3;j<resTable.GetColCount();j++)
		{
			//if(sourceTable.GetColType(j)==Tx::Core::dtype_int4)
			//{
			//	sourceTable.GetCell(j,iSourceRow,vSourceData);
			//	resTable.GetCell(j,iResRow,vResData);
			//	if(vSourceData.numeric_data.int_value==vResData.numeric_data.int_value)
			//	{
			//		tmpI=numeric_data;
			//	}	
			//	else
			//	{
			//		tmpI=0;
			//	}
			//	resTable.SetCell(j,iResRow,tmpI);
			//}
			
			if(sourceTable.GetColType(j)==Tx::Core::dtype_decimal)
			{
				sourceTable.GetCell(j,iSourceRow,vSourceData);
				resTable.GetCell(j,iResRow,vResData);
				tmpD=vResData.numeric_data.double_value+vSourceData.numeric_data.double_value;
				resTable.SetCell(j,iResRow,tmpD);
			}	
			
			if(sourceTable.GetColType(j)==Tx::Core::dtype_double)
			{
				sourceTable.GetCell(j,iSourceRow,vSourceData);
				resTable.GetCell(j,iResRow,vResData);
				tmpD=vResData.numeric_data.double_value+vSourceData.numeric_data.double_value;
				resTable.SetCell(j,iResRow,tmpD);
			}
		}

		
		return true;
	}//end of CopyRow

//bool CopyColumn()
//{
//
//	return true;
//}//end of CopyRow


//-----------fund-----------//
bool TxFund::FillColumn(
			Tx::Core::Table_Indicator &m_transTable,		//关联表
			UINT transIndexColumn,							//关联表作为map索引的列
			UINT addColumn,									//关联表中数据的列，从这里取出数据放到insertColumn列
			Tx::Core::Table_Indicator &m_resTableAddColumn,	//源表
			UINT resIndexColumn,								//源表作为map索引的列
			UINT	insertColumn								//要插入数据的列		
		)
{
	//有表为空则操作无意义
	if((m_transTable.GetRowCount()==0)||(m_resTableAddColumn.GetRowCount()==0))
		return false;

	//列超界错误
	if((UINT)addColumn>m_transTable.GetColCount())
		return false;

	if (insertColumn>m_resTableAddColumn.GetColCount())
	{
		m_resTableAddColumn.AddCol(m_transTable.GetColumn(addColumn)->dtype);
		insertColumn=m_resTableAddColumn.GetRowCount();

		ASSERT(0);
		TRACE("数据列超出列数，默认添加到最后一样\n");
	}
	else	
		if(m_resTableAddColumn.GetColType(insertColumn)!=m_transTable.GetColType(addColumn))
		{
			//判断两列数据类型是否一致，否则不做操作。	
			ASSERT(0);
			return false;
		}

	//std::set<int> setEventId;
	//int	iEventId;
	//for(UINT k=0;k<m_resTableAddColumn.GetRowCount();k++)
	//{
	//	m_resTableAddColumn.GetCell(resIndexColumn,k,iEventId);
	//	setEventId.insert(iEventId);
	//}
	//
	
	std::unordered_map<int,int> mMap;
	int iTransTableCell,iResTableCell;
	for(UINT i=0;i<m_transTable.GetRowCount();i++)
	{
		m_transTable.GetCell(transIndexColumn,i,iTransTableCell);
		mMap.insert(std::make_pair(iTransTableCell,i));
	}

	for(UINT i=0;i<m_resTableAddColumn.GetRowCount();i++)
	{
		int p=(int)m_resTableAddColumn.GetColType(resIndexColumn);
		m_resTableAddColumn.GetCell(resIndexColumn,i,iResTableCell);
		std::unordered_map<int,int>::iterator iter=mMap.find(iResTableCell);
		if (iter!=mMap.end())
		{
			Tx::Core::VariantData val;
			val.data_type=m_transTable.GetColumn(1)->dtype;
			m_transTable.GetCell(addColumn,iter->second,val);
			m_resTableAddColumn.SetCell(insertColumn,i,val);
		}
		else
		{
			//没查到，理论上不可能，因为前面就会被中断掉了
		}
	}

	return true;
}//end of FillColumn

//将指定的列，从报告期id转为基金id
bool TxFund::TransReportIdToFundId(Tx::Core::Table_Indicator &resTable,int iCol)
{
	Tx::Core::Table_Indicator TransTable;
	Tx::Business::IndicatorWithParameterArray tmpIWPA;
	//从本地碎文件取得对应的表

	std::vector<int> iParameter;
	int iReportId=0;
	int	iFundId=0,iReportYear,iReportQuarter;

	iParameter.push_back(30901255);	//基金id
	iParameter.push_back(30901256);	//财年
	iParameter.push_back(30901257);	//报告期
	Tx::Business::IndicatorFile::GetInstance()->SetIWAP(tmpIWPA,iParameter);
	Tx::Business::IndicatorFile::GetInstance()->GetData(tmpIWPA,iParameter,true);

	std::unordered_map<int,int> IdPair;
	std::unordered_map<int,int>::iterator iter_IdPair;

	//建立报告期id-〉对应行数的map，这样从该行可以取得3个数据（基金id，财年，报告期）

	resTable.InsertCol(iCol,Tx::Core::dtype_int4);
	resTable.InsertCol(iCol,Tx::Core::dtype_int4);

	for(UINT i=0;i<tmpIWPA.m_table_indicator.GetRowCount();i++)
	{
		tmpIWPA.m_table_indicator.GetCell(2,i,iReportId);
		IdPair.insert(std::make_pair(iReportId,i));
	}
	
	int maxAssertCount=0;
	//根据索引将报告期id-〉转为基金Id
	for(UINT i=0;i<resTable.GetRowCount();i++)
	{
		
		int nRow;

		resTable.GetCell(iCol,i,iReportId);
		iter_IdPair=IdPair.find(iReportId);
		if(iter_IdPair!=IdPair.end())
		{
			nRow=iter_IdPair->second;
			
			tmpIWPA.m_table_indicator.GetCell(3,nRow,iFundId);
			tmpIWPA.m_table_indicator.GetCell(4,nRow,iReportYear);
			tmpIWPA.m_table_indicator.GetCell(5,nRow,iReportQuarter);
			
			resTable.SetCell(iCol,i,iFundId);
			resTable.SetCell(iCol+1,i,iReportYear);
			resTable.SetCell(iCol+2,i,iReportQuarter);

		}
		else
		{
			resTable.SetCell(iCol,i,0);

			maxAssertCount++;
			TRACE("基金-〉专项统计-〉辅助_报告期转基金：不存在这样此报告期Id:%d\n",iReportId);
			//ASSERT(0);
			if(maxAssertCount==10)
				break;
			//超过5个没有停止报告和继续
		}
	}

	return true;
}

bool TxFund::GetCycleDates(int iStartDate,int iEndDate,int iTimeCycle,std::vector<int> &iDates,int iCustomCycle)
{
	m_pShIndex->LoadHisTrade();
	m_pShIndex->LoadTradeDate();
	this->m_pShIndex->GetDate(iStartDate,iEndDate,iTimeCycle,iDates,iCustomCycle);
	//2012-04-12,刘鹏。区间的计算，不改变开始日期。bug 11578 -- 有的基金在上交所的非交易日也披露基金净值
	if(iTimeCycle == 5)
	{
		iDates[0] = iStartDate;
	}
	return true;
}

//将 财年+报告期-〉报告日期
bool TxFund::TransReportDateToNormal(Tx::Core::Table_Indicator &tmpTable,int iCol)
{
	if((UINT)iCol>=tmpTable.GetColCount())
		return false;
	for(UINT i=0;i<tmpTable.GetRowCount();i++)
		{
			int iYear,iYearQuarter;
			tmpTable.GetCell(iCol,i,iYear);
			tmpTable.GetCell(iCol+1,i,iYearQuarter);
			switch(iYearQuarter)
			{
			case 40040001:
				iYearQuarter=331;
				break;
			case 40040002:
				iYearQuarter=630;
				break;
			case 40040004:
				iYearQuarter=930;
				break;
			case 40040006:
				iYearQuarter=1231;
				break;
			case 40040003:
				iYearQuarter=630;
				break;
			case 40040009:
				iYearQuarter=1231;
				break;
			default:
				break;
			}

			tmpTable.SetCell(iCol,i,iYear*10000+iYearQuarter);
		}
		tmpTable.DeleteCol(iCol+1);

		return true;
}
//将 财年+报告期-〉报告日期   报告期和日期的合并
bool TxFund::TransReportDateToNormal2(Tx::Core::Table_Indicator &tempTable,int iCol)
{
	if((UINT)iCol>=tempTable.GetColCount())
		return false;
	int iYear,iYearQuarter;
	for(UINT i=0;i<tempTable.GetRowCount();i++)
	{		
		tempTable.GetCell(iCol,i,iYear);
		tempTable.GetCell(iCol+1,i,iYearQuarter);
		switch(iYearQuarter)
		{
		case 40040001:           // 一季报
			iYearQuarter=331;
			break;
		case 40040002:           // 二季报
			iYearQuarter=630;
			break;
		case 40040004:           // 三季报
			iYearQuarter=930;
			break;
		case 40040006:           // 四季报
			iYearQuarter=1231;
			break;
		case 40040003:           // 中报
			iYearQuarter=630;
			break;
		case 40040009:           // 年报
			iYearQuarter=1231;
			break;
		default:
			break;
		}

		tempTable.SetCell(iCol,i,iYear*10000+iYearQuarter);
	}
	tempTable.DeleteCol(iCol+1);

	return true;
}

//将 财年+报告期-〉报告日期（CString类型）
bool TxFund::TransReportDateToNormal3(Tx::Core::Table_Indicator &tempTable,int iCol)
{
	if((UINT)iCol>=tempTable.GetColCount())
		return false;
	int tempdate = 0;
	int reportdate = 0;
	for(UINT i=0;i<tempTable.GetRowCount();i++)
	{
		int iYear,iYearQuarter;
		CString strYear,strQuarter;
		tempTable.GetCell(iCol,i,iYear);
		strYear.Format(_T("%d"),iYear);
		tempTable.GetCell(iCol+1,i,iYearQuarter);
		switch(iYearQuarter)
		{
		case 40040001:
			strQuarter = _T("一季报");
			tempdate = 331;
			break;
		case 40040002:
			strQuarter = _T("二季报");
			tempdate = 630;
			break;
		case 40040004:
			strQuarter = _T("三季报");
			tempdate = 930;
			break;
		case 40040006:
			strQuarter = _T("四季报");
			tempdate = 1231;
			break;
		case 40040003:
			strQuarter = _T("中报");
			tempdate = 630;
			break;
		case 40040009:
			strQuarter = _T("年报");
			tempdate = 1231;
			break;
		default:
			break;
		}
        strYear = strYear + _T("年") + strQuarter;
		tempTable.SetCell(iCol+2,i,strYear);
		iYearQuarter %=10;
		reportdate = iYear*10000 + iYearQuarter;
		tempTable.SetCell(iCol+1,i,reportdate);
	}
	tempTable.DeleteCol(iCol);
	return true;
}
bool TxFund::InsertColOfNAV(Tx::Core::Table_Indicator &wholeTable,int iInsertCol,int iIdCol,int iDateCol)
{
		std::set<int> items;
		std::set<int>::iterator iter;	
		if((wholeTable.GetColCount()<(UINT)iInsertCol)||(wholeTable.GetColCount()<(UINT)iIdCol)||(wholeTable.GetColCount()<(UINT)iDateCol))
			return false;
		Tx::Core::Table_Indicator tmpAssetTable;
		//第一二三参数列:f_F_PERIODIC_REPORT_ID,int型
		tmpAssetTable.AddParameterColumn(Tx::Core::dtype_int4);
		tmpAssetTable.AddParameterColumn(Tx::Core::dtype_int4);
		tmpAssetTable.AddParameterColumn(Tx::Core::dtype_int4);

		int varCount=3;			

		UINT arrParaIndex[3];
		arrParaIndex[0]=0;
		arrParaIndex[1]=1;
		arrParaIndex[2]=2;
        //30901140是资产净值
		tmpAssetTable.AddIndicatorColumn(30901140,Tx::Core::dtype_decimal,arrParaIndex,3);
		std::unordered_map<__int64,double> tmpMap;
		//设定指标列


		bool result=m_pLogicalBusiness->GetData(tmpAssetTable,true);
		TransReportDateToNormal(tmpAssetTable,1);
		TableOutput(wholeTable);
		int iId,iDate;
		double dTotalNAV;
		__int64 iIdDate;
		for(UINT i=0;i<tmpAssetTable.GetRowCount();i++)
		{
			tmpAssetTable.GetCell(0,i,iId);
			tmpAssetTable.GetCell(1,i,iDate);
			iIdDate=(__int64)iId*100000000+iDate;
			
			tmpAssetTable.GetCell(2,i,dTotalNAV);
			tmpMap.insert(std::make_pair(iIdDate,dTotalNAV));
		}
		std::unordered_map<__int64,double>::iterator mIter;
		for(UINT i=0;i<wholeTable.GetRowCount();i++)
		{		
			int tmpId=0,tmpDate=0;
			wholeTable.GetCell(iIdCol,i,tmpId);
			wholeTable.GetCell(iDateCol,i,tmpDate);

			items.clear();

			if(tmpId<10000000)
			{
				this->GetSecurityNow(tmpId);			
				if(m_pSecurity!=NULL)
				{
					if(iId<20000000)
						tmpId=m_pSecurity->GetSecurity1Id();
					else
						tmpId=m_pSecurity->GetInstitutionId();
				}
				else
				{
					CString tmpStr;
					tmpStr.Format(_T("非法的交易实体号：%d！请查询"),tmpId);
					/*ASSERT(0);*/
					TRACE(tmpStr);
				}
			}
			iIdDate=(__int64)tmpId*100000000+tmpDate;
			mIter=tmpMap.find(iIdDate);
			if(mIter!=tmpMap.end())
				dTotalNAV=mIter->second;
			wholeTable.SetCell(iInsertCol,i,dTotalNAV);
			
		}
	return true;
}

bool TxFund::ChangeDateColToReportCol(Tx::Core::Table_Indicator &resTable,int iCol)
{
	
	int iYear,iMonthDay,iDate;
	for(UINT i=0;i<resTable.GetRowCount();i++)
	{
		resTable.GetCell(iCol,i,iDate);
		iYear=iDate/10000;
		iMonthDay=iDate%10000;
		if(((iMonthDay<0331)&&(iMonthDay>=0101))||(iMonthDay==1231))
		{
			iDate=(iYear-1)*10000+1231;
		}
		else 
			if((iMonthDay>=331)&&(iMonthDay<630))
			{
				iDate=iYear*10000+331;
			}
			else
				if((iMonthDay>=630)&&(iMonthDay<930))
				{
					iDate=iYear*10000+630;
				}
				else
					if((iMonthDay>=930)&&(iMonthDay<1231))
					{
						iDate=iYear*10000+930;
					}
		resTable.SetCell(iCol,i,iDate);
	}
	return true;
}//End Of ChangeDateToReportdate


bool TxFund::GetIndexDat(int file_id, std::unordered_map<int,CString>& indexdat_map)
{
	TypeMapManage::GetInstance()->GetTypeMap(file_id,indexdat_map);
	return true;
}
//added by zhangxs 20090106
bool TxFund::AddUpRowCompany(
							 Tx::Core::Table &resTable,		//存放结果表
							 std::vector<int> ColVector,		//根据哪些列进行统计  
							 std::vector<int> IntCol,			//需要相加的整型列
							 std::vector<int> DoubleCol,	//需要相加的double列
							 std::vector<int> iDate,			//报告期
							 int			  iCol,				//报告期所在的列。
							 int 		      iTradeCol,		//券商ID所在的列，并且它的后面必须是它的名称。
							 int			   sortCol,		//根据那一列进行排序。
							 int			   pos			//排名所在的列。
					   )	
{
	std::set<int>::iterator iterSet,iterTradeSet;
	std::set<int>::iterator iterTemp;
	std::set<int> iIdSet;
	std::set<int> iTradeSet;
	int iTradeId;
	std::vector<int> rowVector;
	std::unordered_multimap<int,int> IdPosition;
	std::unordered_multimap<int,int>::iterator iterMulti;
	std::vector<UINT> PositionCollect;
	std::vector<UINT> TempCollect;
	std::set<int> Intersection;
	std::vector<UINT>::iterator UterV;
	std::vector<int>::iterator iterV;
	std::vector<int>::iterator iterDate;
	std::vector<int>::iterator dterV;
	std::vector<int>::iterator iterCol;//参照列的专用变量

	std::set<int> deletePosition;
	////用于保存分类统计时的数据
	Tx::Core::Table tempTable,staticTable;	
	staticTable.CopyColumnInfoFrom(resTable);
	tempTable.CopyColumnInfoFrom(staticTable);
	staticTable.Clone(resTable);
	int Colpos;//程序中公共用的行变量。
	int iDateCol;//报告期的专用列号
	int iIdCol;//参照列的专用列号
	int iRow;//程序中公共用的行变量。
	int icount;
	int iData = 0;
	double dData = 0;
	int itempdate;
	int itempId;
	std::unordered_map<int,int> iDataMap;//存放临时的整型值	
	std::unordered_map<int,double> dDataMap;//存放临时的double值
	//填充iDataMap和dDataMap
	for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
	{
		Colpos = *iterV;
		iDataMap.insert(std::make_pair(Colpos,0));
	}
	for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
	{
		Colpos = *dterV;
		dDataMap.insert(std::make_pair(Colpos,0));
	}
#if _DEBUG
	CString strTable;//为了测试用的
#endif 
	iDateCol = iCol;
	//根据那个参数列进行相加
	for(iterCol = ColVector.begin();iterCol != ColVector.end();++iterCol)
	{
		if(tempTable.GetRowCount() != 0)
		{
			tempTable.Clear();
			tempTable.Clone(staticTable);
		}
		else
			tempTable.Clone(staticTable);
		icount = tempTable.GetRowCount();
		iIdCol = *iterCol;
		if(!iIdSet.empty())
			iIdSet.clear();
		for(int i = 0;i < icount;i++)
		{
			tempTable.GetCell(iIdCol,i,itempId);
			iIdSet.insert(itempId);
		}		
		//根据ID进行相加
		for(iterSet = iIdSet.begin();iterSet != iIdSet.end();++iterSet)
		{
			itempId = *iterSet;					
			PositionCollect.clear();
			tempTable.Find(iIdCol,itempId,PositionCollect);//取得与公司ID相同的纪录的位置。
			if(PositionCollect.empty())
				continue;			
			//把vector里的值放在set里，是为了用Set里的find方法
			std::set<UINT> positionSet(PositionCollect.begin(),PositionCollect.end());
			//根据报告期进行筛选

			for(iterDate = iDate.begin();iterDate != iDate.end();++iterDate)
			{
				itempdate = *iterDate;
				TempCollect.clear();
				tempTable.Find(iDateCol,itempdate,TempCollect);//取得与报告期相同的纪录的位置。
				if(TempCollect.empty())
					continue;
				//把它们相同的值保存在Intersection
				if(!rowVector.empty())
					rowVector.clear();
				for(UterV = TempCollect.begin();UterV != TempCollect.end();++UterV)
				{
					if(positionSet.find(*UterV) != positionSet.end())
						rowVector.push_back(*UterV);
				}
				if(rowVector.empty())
					continue;			
				//取得在那些相同的行中全部的交易实体
				if(!iTradeSet.empty())
					iTradeSet.clear();
				if(!IdPosition.empty())
					IdPosition.clear();
				for(iterV = rowVector.begin();iterV != rowVector.end();++iterV)
				{
					tempTable.GetCell(iTradeCol,*iterV,iTradeId);
					iTradeSet.insert(iTradeId);
					IdPosition.insert(make_pair(iTradeId,*iterV));					
				}				
				for(iterTradeSet = iTradeSet.begin();iterTradeSet != iTradeSet.end();++iterTradeSet)
				{
					iTradeId = *iterTradeSet;
					if (IdPosition.count(iTradeId) > 1)
					{
						if(!Intersection.empty())
							Intersection.clear();
						for(iterMulti = IdPosition.lower_bound(iTradeId);iterMulti != IdPosition.upper_bound(iTradeId);++iterMulti)
						{
							Intersection.insert(iterMulti->second);					
						}
						//把保存各列值得map初始化为零
						if(!IntCol.empty() || !DoubleCol.empty())
						{
							for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
							{
								Colpos = *iterV;
								iDataMap[Colpos] = 0;
							}
							for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
							{
								Colpos = *dterV;
								dDataMap[Colpos] = 0;
							}
						}							
						//把ID相同并且报告期相同的行中，所需要相加的列进行相加。
						for(iterTemp = Intersection.begin();iterTemp != Intersection.end();++iterTemp)
						{
							iRow = *iterTemp;
							//需要相加的整型列进行相加。
							if(!IntCol.empty())
							{
								for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
								{
									Colpos = *iterV;
									tempTable.GetCell(Colpos,iRow,iData);
									if (iData < 0)
										continue;									
									iDataMap[Colpos] = iDataMap[Colpos] + iData;
								}
							}							
							//需要相加的double列进行相加。
							if(!DoubleCol.empty())
							{
								for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
								{
									Colpos = *dterV;
									tempTable.GetCell(Colpos,iRow,dData);
									if (dData < 0)
										continue;									
									dDataMap[Colpos] = dDataMap[Colpos] + dData;
								}
							}						
						}
						//把相加的各项的值放到tempTable里的其中一行里.多余的行全部删掉.
						if(!IntCol.empty() || !DoubleCol.empty())
						{
							iRow = *(Intersection.begin());
							//首先放整型列的值
							for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
							{
								Colpos = *iterV;						
								iData = iDataMap[Colpos];								
								tempTable.SetCell(Colpos,iRow,iData);
							}
							//其次放double列的值
							for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
							{
								Colpos = *dterV;
								dData= dDataMap[Colpos] ;
								tempTable.SetCell(Colpos,iRow,dData);
							}
							//把多余的行删掉；
							for(iterTemp = --(Intersection.end());iterTemp != Intersection.begin();--iterTemp)
							{
								deletePosition.insert(*iterTemp);								
							}							
						}
					}
				}
			}		
		}
		//把要删除的行全部删掉。
		std::set<int>::reverse_iterator revIter;
		for(revIter = deletePosition.rbegin();revIter != deletePosition.rend();++revIter)
			tempTable.DeleteRow(*revIter);		
		tempTable.Arrange();

#ifdef _DEBUG
		strTable=tempTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
		//added by zhangxs 20090107
		std::set<int> setDeleteRow;
		//选中基金公司统计时
		/*if(*iterCol == 13)
		{*/
			setDeleteRow.clear();
			std::vector<UINT> m_vecPositionCol;
			for(iterSet = iIdSet.begin();iterSet != iIdSet.end();++iterSet)
			{
				itempId = *iterSet;					
				m_vecPositionCol.clear();
				tempTable.Find(iIdCol,itempId,m_vecPositionCol);//取得与公司ID相同的纪录的位置。
				if(m_vecPositionCol.empty())
					continue;			
				//根据报告期进行筛选
				std::vector<UINT> m_vecPositionColDate;
				for(iterDate = iDate.begin();iterDate != iDate.end();++iterDate)
				{
					itempdate = *iterDate;
					m_vecPositionColDate.clear();
					tempTable.Find(iDateCol,itempdate,m_vecPositionColDate);//取得与报告期相同的纪录的位置。
					if(m_vecPositionColDate.empty())
						continue;
					//统计名称相同的记录
					CString m_strName;
					CString m_strCompName;
					std::set<int> setRowNo; //保存券商全称和简称Id不同的行
					for(std::vector<UINT>::iterator m_iter = m_vecPositionCol.begin();m_iter!=m_vecPositionCol.end();m_iter++)
					{
						double m_dJyyj = 0.0;
						double m_dGpjyl = 0.0;
						double m_dZqjyl = 0.0;
						double m_dHgjyl = 0.0;
						int m_iReport = 0;
					/*	if(setRowNo.find(*m_iter)!= setRowNo.end())
							continue;*/
						if(setDeleteRow.find(*m_iter)!= setDeleteRow.end())
							continue;
						tempTable.GetCell(7,*m_iter,m_strName);
						if(m_strName.GetLength()<1)
							continue;
						tempTable.GetCell(4,*m_iter,m_iReport);
						if(m_iReport < 1 || m_iReport != itempdate)
							continue;
						tempTable.GetCell(8,*m_iter,m_dJyyj);
						tempTable.GetCell(9,*m_iter,m_dGpjyl);
						tempTable.GetCell(10,*m_iter,m_dZqjyl);
						tempTable.GetCell(11,*m_iter,m_dHgjyl);
						if(m_dJyyj <1e-6 || m_dJyyj == Con_doubleInvalid)
							m_dJyyj = 0.0;
						if(m_dGpjyl <1e-6 || m_dGpjyl == Con_doubleInvalid)
							m_dGpjyl = 0.0;
						if(m_dZqjyl <1e-6 || m_dZqjyl == Con_doubleInvalid)
							m_dZqjyl = 0.0;
						if(m_dHgjyl <1e-6 || m_dHgjyl == Con_doubleInvalid)
							m_dHgjyl = 0.0;
						for(std::vector<UINT>::iterator iter = m_iter+1;iter!=m_vecPositionCol.end();iter++)
						{
							if(setDeleteRow.find(*m_iter)!= setDeleteRow.end())
								continue;
							int m_iReportDate = 0;
							tempTable.GetCell(4,*iter,m_iReportDate);
							if(m_iReport < 1 || m_iReportDate != itempdate)
								continue;
							tempTable.GetCell(7,*iter,m_strCompName);
							if(!m_strName.Compare(m_strCompName))
							{
								double m_dJy = 0.0;
								double m_dGp = 0.0;
								double m_dZq = 0.0;
								double m_dHg = 0.0;
								tempTable.GetCell(8,*iter,m_dJy);
								tempTable.GetCell(9,*iter,m_dGp);
								tempTable.GetCell(10,*iter,m_dZq);
								tempTable.GetCell(11,*iter,m_dHg);
								if( m_dJy > 0.0 )
									m_dJyyj += m_dJy;
								if( m_dGp > 0.0 )
									m_dGpjyl += m_dGp;
								if( m_dZq > 0.0 )
									m_dZqjyl += m_dZq;
								if( m_dHg > 0.0 )
									m_dHgjyl += m_dHg;
								setRowNo.insert(*iter);
								setDeleteRow.insert(*iter);
							}
						}
						tempTable.SetCell(8,*m_iter,m_dJyyj);
						tempTable.SetCell(9,*m_iter,m_dGpjyl);
						tempTable.SetCell(10,*m_iter,m_dZqjyl);
						tempTable.SetCell(11,*m_iter,m_dHgjyl);
					}
				}		
			}
			//把要删除的行全部删掉。
			for(revIter = setDeleteRow.rbegin();revIter != setDeleteRow.rend();++revIter)
				tempTable.DeleteRow(*revIter);		
			tempTable.Arrange();
		//}
		
#ifdef _DEBUG
		strTable=tempTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif		
		//把tempTable里的数据放到resTable
		if(iDate.size() > 1)
		{
			Tx::Core::Table singleDateTable;
			std::vector<UINT>::iterator u_iterator;
			singleDateTable.CopyColumnInfoFrom(tempTable);
			MultiSortRule multisort2;
			int position,i_data;
			double d_data;
			int irow;
			for(iterDate = iDate.begin();iterDate != iDate.end();++iterDate)
			{
				if(singleDateTable.GetRowCount() != 0)
				{
					singleDateTable.Clear();
					singleDateTable.CopyColumnInfoFrom(tempTable);
				}
				if(!TempCollect.empty())
					TempCollect.clear();
				tempTable.Find(iDateCol,*iterDate,TempCollect);//取得与报告期相同的纪录的位置。
				if(TempCollect.empty())
					continue;
				//把报告期相同的记录放到singleDateTable里
				irow = 0;
				for(u_iterator = TempCollect.begin();u_iterator !=TempCollect.end();++u_iterator,irow++)
				{
					singleDateTable.AddRow();
					position = *u_iterator;				
					for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
					{
						tempTable.GetCell(*dterV,position,d_data);
						singleDateTable.SetCell(*dterV,irow,d_data);
					}
					for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
					{
						tempTable.GetCell(*iterV,position,i_data);
						singleDateTable.SetCell(*iterV,irow,i_data);
					}
					//添加报告期
					tempTable.GetCell(iDateCol,position,itempdate);
					singleDateTable.SetCell(iDateCol,irow,itempdate);
					//添加根据那一列进行统计的ID
					tempTable.GetCell(iIdCol,position,itempId);
					singleDateTable.SetCell(iIdCol,irow,itempId);
					//添加债券的名称和代码
					CString strname;
					tempTable.GetCell(iTradeCol+1,position,strname);
					singleDateTable.SetCell(iTradeCol+1,irow,strname);
				}				
				multisort2.AddRule(iIdCol,true);
				multisort2.AddRule(sortCol,false);
				singleDateTable.SortInMultiCol(multisort2);
				singleDateTable.Arrange();
				int NO ;
				for(iterSet = iIdSet.begin();iterSet != iIdSet.end();++iterSet)
				{
					itempId = *iterSet;					
					PositionCollect.clear();
					singleDateTable.Find(iIdCol,itempId,PositionCollect);//取得与公司ID相同的纪录的位置。
					if(PositionCollect.empty())
						continue;
					NO = 0;
					for(UterV = PositionCollect.begin();UterV != PositionCollect.end();++UterV)
					{
						NO += 1;
						singleDateTable.SetCell(pos,*UterV,NO);
					}
				}
#ifdef _DEBUG
				strTable=singleDateTable.TableToString();
				Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
				//把singleDateTable里的数据拷贝到resTable;
				resTable.AppendTableByRow(singleDateTable);
			}
		}
		else
		{
			//为了对重仓债里的重仓股进行排序，所以对tempTable进行排序，分别按照ID、债券的市值
			MultiSortRule multisort;
			multisort.AddRule(iIdCol,true);
			multisort.AddRule(sortCol,false);
			tempTable.SortInMultiCol(multisort);
			tempTable.Arrange();
			int NO ;
			for(iterSet = iIdSet.begin();iterSet != iIdSet.end();++iterSet)
			{
				itempId = *iterSet;					
				PositionCollect.clear();
				tempTable.Find(iIdCol,itempId,PositionCollect);//取得与公司ID相同的纪录的位置。
				if(PositionCollect.empty())
					continue;
				NO = 0;
				for(UterV = PositionCollect.begin();UterV != PositionCollect.end();++UterV)
				{
					NO += 1;
					tempTable.SetCell(pos,*UterV,NO);
				}
			}
#ifdef _DEBUG
			strTable=tempTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
			icount = tempTable.GetRowCount();
			int tempRow = resTable.GetRowCount();
			resTable.AddRow(icount);	
			for(int k = 0;k < icount;k++)
			{
				//添加基金净值
				tempTable.GetCell(pos,k,NO);
				resTable.SetCell(pos,tempRow+k,NO);
				//添加所累加的数据。
				if (!IntCol.empty())
				{
					for(iterV = IntCol.begin();iterV != IntCol.end();++iterV)
					{
						tempTable.GetCell(*iterV,k,iData);
						if (iData < 0)
						{
							iData = Tx::Core::Con_intInvalid;
						}
						resTable.SetCell(*iterV,tempRow+k,iData);
					}
				} 
				if(!DoubleCol.empty())
				{
					for(dterV = DoubleCol.begin();dterV != DoubleCol.end();++dterV)
					{
						tempTable.GetCell(*dterV,k,dData);
						if (dData < 0)
						{
							dData = Tx::Core::Con_doubleInvalid;								
						}
						resTable.SetCell(*dterV,tempRow+k,dData);
					}
				}
				//添加报告期
				tempTable.GetCell(iDateCol,k,itempdate);
				resTable.SetCell(iDateCol,tempRow+k,itempdate);
				//添加根据那一列进行统计的ID
				tempTable.GetCell(iIdCol,k,itempId);
				resTable.SetCell(iIdCol,tempRow+k,itempId);
				//添加债券的名称和代码
				CString strname;
				tempTable.GetCell(iTradeCol+1,k,strname);
				resTable.SetCell(iTradeCol+1,tempRow+k,strname);
			}
		}
#ifdef _DEBUG
		strTable=resTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	}
#ifdef _DEBUG
	strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	return true;
}
//解决按基金公司统计,公司名称全称和简称问题 added by zhangxs 20090107
bool TxFund::AddUpRowEX(Tx::Core::Table &resTable,		//存放结果表
					  int iStyle,						//样本分类的方式
					  std::vector<int> ColVector,		//根据哪些列进行统计  
					  std::vector<int> IntCol,			//需要相加的整型列
					  std::vector<int> DoubleCol,	//需要相加的double列
					  std::vector<int> iDate,			//报告期
					  int			   iCol,				//报告期所在的列。
					  int &			   iTradeCol,			//券商ID所在的列，并且它的前面必须是它的名称。
					  int			   sortCol,		//根据那一列进行排序。
					  int			   pos			//排名所在的列。
					  )	
{

	//之所以把它们分开计算，而没有一个个计算，是为了进行样本汇总时，方便计算
	//并且当按多个分类方式进行统计时，可以同时计算多个分类方式
	switch(iStyle)
	{
	case 3://管理公司ID
		/*AddUpRowCompany(resTable,ColVector,IntCol,DoubleCol,iDate,iCol,iTradeCol,sortCol,pos);
		break;*/
	case 7://管理公司ID和基金风格
	case 11://管理公司ID和托管银行ID
	case 15://基金风格和托管银行ID和管理公司ID
		/*AddUpRowCompany(resTable,ColVector,IntCol,DoubleCol,iDate,iCol,iTradeCol,sortCol,pos);
		break;*/
	case 5://基金风格
	case 9://托管银行ID   	
	case 13://基金风格和托管银行ID
		//AddUpRow3(resTable,ColVector,IntCol,DoubleCol,iDate,iCol,iTradeCol,sortCol,pos);
		AddUpRowCompany(resTable,ColVector,IntCol,DoubleCol,iDate,iCol,iTradeCol,sortCol,pos);
		break;
	case 17://样本汇总
		{
			resTable.InsertCol(iTradeCol+1,Tx::Core::dtype_val_string);
			for(int i = 0;i < 4;i++)
				DoubleCol[i] = DoubleCol[i] + 1;
			AddUpRow4(resTable,ColVector,IntCol,DoubleCol,iDate,iCol,iTradeCol,sortCol+1,pos);
			resTable.DeleteCol(7);//之所以这样做是为了不想再写AddUpRow2的重载函数。
		}
		break;
	case 19://管理公司ID和样本汇总
	case 21://基金风格和样本汇总
	case 25://样本汇总和托管银行ID
	case 23://基金风格和样本汇总和管理公司ID
	case 27://样本汇总和托管银行ID和管理公司ID
	case 29://基金风格和托管银行ID和样本汇总
	case 31://基金风格和托管银行ID和样本汇总和管理公司
		{
			Tx::Core::Table tempTable2;
			tempTable2.CopyColumnInfoFrom(resTable);
			tempTable2.Clone(resTable);
			//AddUpRow3(resTable,ColVector,IntCol,DoubleCol,iDate,iCol,iTradeCol,sortCol,pos);
			AddUpRowCompany(resTable,ColVector,IntCol,DoubleCol,iDate,iCol,iTradeCol,sortCol,pos);
			int itempCol = iTradeCol+1;
			tempTable2.InsertCol(itempCol,Tx::Core::dtype_val_string);
			for(int i = 0;i < 4;i++)
				DoubleCol[i] = DoubleCol[i] + 1;
			int iTempCount = ColVector.size();
			for (int i = 0;i < iTempCount;i++)
			{
				ColVector[i] += 1;
			}
			sortCol = sortCol + 1;
			AddUpRow4(tempTable2,ColVector,IntCol,DoubleCol,iDate,iCol,iTradeCol,sortCol,pos);
			tempTable2.DeleteCol(itempCol);//之所以这样做是为了不想再写AddUpRow2的重载函数。
			int icount = tempTable2.GetRowCount();
			int ideleteRow = icount - iTradeCol;
			tempTable2.DeleteRow(0,ideleteRow);
			tempTable2.Arrange();
			resTable.AppendTableByRow(tempTable2,false);
		}
		break;
	}
	return true;
}

//modify by zhoup 2009.02.03
//专项统计
//基金净值输出
bool	TxFund::StatFundNetValueOutput(
									   Tx::Core::Table_Indicator	&resTable,
									   std::vector<int>	&iSecurityId,
									   int		iStartDate,
									   int		iEndDate,
									   int		iTimeCycle,		//时间周期。日周月季年
									   int		iStatIndicator,	//基金单位净值，基金累计净值 0-none 1-单位,2-累计,3-both
									   std::vector<CString> &vDates,
									   std::vector<CString> &vColName
									   )
{
//	return false;
	if(iStatIndicator < 1 || iStatIndicator > 3)
		return false;

	if (iSecurityId.empty())
		return false;

	CTime temptime;
	temptime = CTime::GetCurrentTime();
	int tempDate = temptime.GetYear()*10000 + temptime.GetMonth()*100 + temptime.GetDay();
	if(tempDate == iEndDate)
		iEndDate = iEndDate - 1;

	std::vector<int> iDates;
	iDates.clear();
	//GetCycleDates(iStartDate,iEndDate,iTimeCycle,iDates,0);
	//modified by zhangxs 20100427 时期序列按照自然日
	//================================================================
	int dateB = 0;
	if(iTimeCycle == 1)
	{
		COleDateTime dttemp;
		dttemp.SetDate(iStartDate/10000,iStartDate%10000/100,iStartDate%100);
		int iDayofWeek = dttemp.GetDayOfWeek();
		int iSpan = (iDayofWeek + 1) % 7;
		COleDateTimeSpan dtspan(iSpan,0,0,0);
		dttemp -= dtspan;
		dateB = dttemp.GetYear()*10000 + dttemp.GetMonth()*100 + dttemp.GetDay();
		for (int i = dateB;i < iEndDate;)
		{
			if(i >= iStartDate)
				iDates.push_back(i);
			i = Tx::Core::TxDate::CalculateDateOffsetDays (i,7);			
		}
		iDates.push_back(iEndDate);
	}
	else if(iTimeCycle == 2)
	{
		dateB = Tx::Core::TxDate::CalculateEndOfMonth (iStartDate);
		for (int i = dateB;i < iEndDate;)
		{
			iDates.push_back(i);
			i = Tx::Core::TxDate::CalculateDateOffsetMonths(i,1);		
		}
		iDates.push_back(iEndDate);
	}			
	else if(iTimeCycle == 3)
	{
		dateB = Tx::Core::TxDate::CalculateEndOfQuarter (iStartDate);
		for (int i = dateB;i < iEndDate;)
		{
			iDates.push_back(i);
			i = Tx::Core::TxDate::CalculateDateOffsetMonths(i,3);		
		}
		iDates.push_back(iEndDate);
	}
	else if(iTimeCycle == 4)
	{
		dateB = Tx::Core::TxDate::CalculateEndOfYear(iStartDate);
		for (int i = dateB;i < iEndDate;)
		{
			iDates.push_back(i);
			i = Tx::Core::TxDate::CalculateDateOffsetMonths(i,12);		
		}
		iDates.push_back(iEndDate);
	}
	else if(iTimeCycle == 0 || iTimeCycle == 5)
	{
		dateB = iStartDate;
		for (int i = dateB;i <= iEndDate;)
		{
			iDates.push_back(i);
			i = Tx::Core::TxDate::CalculateDateOffsetDays (i,1);		
		}
	}	
	//===================================================================

	resTable.Clear();
	resTable.AddCol(Tx::Core::dtype_int4);	// 0列 不展示
	resTable.AddCol(Tx::Core::dtype_val_string);	// 名称
	resTable.AddCol(Tx::Core::dtype_val_string);	//代码

	if (iStatIndicator != 3)
	{
		for (UINT i=0;i<iDates.size();i++)
			resTable.AddCol(Tx::Core::dtype_double);
	}
	else
	{
		for (UINT i=0;i<2 * iDates.size();i++)
			resTable.AddCol(Tx::Core::dtype_double);
	}

	int iRow = 0;
	for (UINT i=0;i<iSecurityId.size();i++)
	{
		SecurityQuotation* p = (SecurityQuotation*)GetSecurity(iSecurityId[i]);
		if (p == NULL || p->IsFund_Currency())
			continue;
		resTable.AddRow();
		resTable.SetCell(0,iRow,iSecurityId[i]);
		resTable.SetCell(1,iRow,p->GetName());
		resTable.SetCell(2,iRow,p->GetCode());
		iRow++;
	}

	if (iStatIndicator != 2)
	{
		// 必然选择了单位净值
		int iSize = sizeof(int) * ( 1 /*请求类型*/ + 1 /*券个数*/ + resTable.GetRowCount() + 1 /*日期个数*/ + iDates.size());
		LPBYTE pBuffer = new BYTE [iSize];
		if (pBuffer == NULL)
			return false;

		LPBYTE pWrite = pBuffer;
		memset(pBuffer,0,iSize);
		int iType = 1;
		memcpy_s(pWrite,iSize,&iType,sizeof(int));
		pWrite += sizeof(int);
		int nSecSize = (int)resTable.GetRowCount();
		memcpy_s(pWrite,iSize,&nSecSize,sizeof(int));
		pWrite += sizeof(int);
		for (int i=0;i<nSecSize;i++)
		{
			int iId;
			resTable.GetCell(0,i,iId);
			memcpy_s(pWrite,iSize,&iId,sizeof(int));
			pWrite += sizeof(int);
		}

		nSecSize = (int)iDates.size();
		memcpy_s(pWrite,iSize,&nSecSize,sizeof(int));
		pWrite += sizeof(int);

		for (int i=0;i<nSecSize;i++)
		{
			memcpy_s(pWrite,iSize,&iDates[i],sizeof(int));
			pWrite += sizeof(int);
		}

		//LPCTSTR lpUrl = _T("http://192.168.5.87/FundNavSer/ServerHandler.ashx");
		//LPCTSTR lpUrl = _T("http://192.168.6.89/FundNavSerBinary/ServerHandler.ashx");
		//LPCTSTR lpUrl = _T("http://221.122.41.211/FundNavSer/ServerHandler.ashx");
		LPCTSTR lpUrl = CORE_GETSYSINFO->GetServerAddr(_T("File"),_T("FundNavRaiseRate"));

		Tx::Drive::Http::CSyncUpload upload;
		int iStart = ::GetTickCount();
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

			iStart = ::GetTickCount();
			LPBYTE pRecv = lpData;
			UINT nParseCount = resTable.GetRowCount() * iDates.size();
			float fValue = 0.0;
			double dValue = 0.0;
			for (UINT i=0;i < nParseCount; i++)
			{
				memcpy_s(&fValue,sizeof(float),pRecv,sizeof(float));
				pRecv += sizeof(float);
				if (fabs(fValue - Tx::Core::Con_floatInvalid) < 0.00001)
					// 无效值
					//resultTable.SetCell(i%date.size() + 1,i/date.size(),Tx::Core::Con_doubleInvalid);
				{
					if (iStatIndicator == 1)
						resTable.SetCell(i%iDates.size() + 3,i/iDates.size(),Tx::Core::Con_doubleInvalid);
					else
						resTable.SetCell(2 * (i%iDates.size()) + 3,i/iDates.size(),Tx::Core::Con_doubleInvalid);
				}
				else
				{
					dValue = (double)fValue;
					if (iStatIndicator == 1)
						resTable.SetCell(i%iDates.size() + 3,i/iDates.size(),dValue);
					else
						resTable.SetCell(2 * (i%iDates.size()) + 3,i/iDates.size(),dValue);
				}
			}
			delete []lpData;
			lpData = NULL;
			iEnd = ::GetTickCount();
			TRACE(_T("\r\nParse Result Cost Time %d(ms)\r\n"),iEnd-iStart);
		}
		delete[] pBuffer;
		pBuffer = NULL;
	}
	if (iStatIndicator != 1)
	{
		// 必然选择了累计净值
		int iSize = sizeof(int) * ( 1 /*请求类型*/ + 1 /*券个数*/ + resTable.GetRowCount() + 1 /*日期个数*/ + iDates.size());
		LPBYTE pBuffer = new BYTE [iSize];
		if (pBuffer == NULL)
			return false;

		LPBYTE pWrite = pBuffer;
		memset(pBuffer,0,iSize);
		int iType = 2;
		memcpy_s(pWrite,iSize,&iType,sizeof(int));
		pWrite += sizeof(int);
		int nSecSize = (int)resTable.GetRowCount();
		memcpy_s(pWrite,iSize,&nSecSize,sizeof(int));
		pWrite += sizeof(int);
		for (int i=0;i<nSecSize;i++)
		{
			int iId;
			resTable.GetCell(0,i,iId);
			memcpy_s(pWrite,iSize,&iId,sizeof(int));
			pWrite += sizeof(int);
		}

		nSecSize = (int)iDates.size();
		memcpy_s(pWrite,iSize,&nSecSize,sizeof(int));
		pWrite += sizeof(int);

		for (int i=0;i<nSecSize;i++)
		{
			memcpy_s(pWrite,iSize,&iDates[i],sizeof(int));
			pWrite += sizeof(int);
		}

		//LPCTSTR lpUrl = _T("http://192.168.5.87/FundNavSer/ServerHandler.ashx");
		//LPCTSTR lpUrl = _T("http://192.168.6.89/FundNavSerBinary/ServerHandler.ashx");
		//LPCTSTR lpUrl = _T("http://221.122.41.211/FundNavSer/ServerHandler.ashx");
		LPCTSTR lpUrl = CORE_GETSYSINFO->GetServerAddr(_T("File"),_T("FundNavRaiseRate"));

		Tx::Drive::Http::CSyncUpload upload;
		int iStart = ::GetTickCount();
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

			iStart = ::GetTickCount();
			LPBYTE pRecv = lpData;
			UINT nParseCount = resTable.GetRowCount() * iDates.size();
			float fValue = 0.0;
			double dValue = 0.0;
			for (UINT i=0;i < nParseCount; i++)
			{
				memcpy_s(&fValue,sizeof(float),pRecv,sizeof(float));
				pRecv += sizeof(float);
				if (fabs(fValue - Tx::Core::Con_floatInvalid) < 0.00001)
					// 无效值
					//resultTable.SetCell(i%date.size() + 1,i/date.size(),Tx::Core::Con_doubleInvalid);
				{
					if (iStatIndicator == 2)
						resTable.SetCell(i%iDates.size() + 3,i/iDates.size(),Tx::Core::Con_doubleInvalid);
					else
						resTable.SetCell(2 * (i%iDates.size()) + 4,i/iDates.size(),Tx::Core::Con_doubleInvalid);
				}
				else
				{
					dValue = (double)fValue;
					if (iStatIndicator == 2)
						resTable.SetCell(i%iDates.size() + 3,i/iDates.size(),dValue);
					else
						resTable.SetCell(2 * (i%iDates.size()) + 4,i/iDates.size(),dValue);
				}
			}
			delete []lpData;
			lpData = NULL;
			iEnd = ::GetTickCount();
			TRACE(_T("\r\nParse Result Cost Time %d(ms)\r\n"),iEnd-iStart);
		}
		delete[] pBuffer;
		pBuffer = NULL;
	}
	//作列名
	vColName.push_back(_T("名称"));
	vColName.push_back(_T("代码"));



	//iter_Date=iDates.begin();		
	CString tmpStr;
	for(UINT j=0;j<iDates.size();j++)
	{
		if(iStatIndicator==3)
		{
			tmpStr.Format(_T("单位净值"));
			vColName.push_back(tmpStr);
			tmpStr.Format(_T("累计净值"));
			vColName.push_back(tmpStr);
			tmpStr.Format(_T("%d-%d-%d"),iDates[j]/10000,iDates[j]/100%100,iDates[j]%100);
			vDates.push_back(tmpStr);
		}
		else 
		{
			tmpStr.Format(_T("%d-%d-%d"),iDates[j]/10000,iDates[j]/100%100,iDates[j]%100);
			vColName.push_back(tmpStr);
		}
//		iter_Date++;
	}
	return true;
}//end of StatFundNetValueOutput

bool TxFund::StatFundNetValueOutput(Tx::Core::Table_Indicator &resTable,std::vector<int> &iSecurityId,std::vector<int> iDates)
{
	if (iSecurityId.empty())
		return false;

	resTable.Clear();
	resTable.AddCol(Tx::Core::dtype_int4);	// 0列 id
	resTable.AddCol(Tx::Core::dtype_int4); // 1列 日期

	for (UINT i=0;i<iDates.size();i++)
	{
		resTable.AddCol(Tx::Core::dtype_double);
	}

	int iRow = 0;
	for (UINT i=0;i<iSecurityId.size();i++)
	{
		SecurityQuotation* p = (SecurityQuotation*)GetSecurity(iSecurityId[i]);
		if (p == NULL || p->IsFund_Currency())
			continue;
		resTable.AddRow();
		resTable.SetCell(0,iRow,iSecurityId[i]);
		iRow++;
	}
	for (UINT i=0;i<iDates.size();i++)
	{
		if(i >= resTable.GetRowCount())
			resTable.AddRow();

		resTable.SetCell(1,i,iDates[i]);
	}

	// 必然选择了单位净值
	int iSize = sizeof(int) * ( 1 /*请求类型*/ + 1 /*券个数*/ + resTable.GetRowCount() + 1 /*日期个数*/ + iDates.size());
	LPBYTE pBuffer = new BYTE [iSize];
	if (pBuffer == NULL)
		return false;

	LPBYTE pWrite = pBuffer;
	memset(pBuffer,0,iSize);
	int iType = 1;
	memcpy_s(pWrite,iSize,&iType,sizeof(int));
	pWrite += sizeof(int);
	int nSecSize = (int)resTable.GetRowCount();
	memcpy_s(pWrite,iSize,&nSecSize,sizeof(int));
	pWrite += sizeof(int);
	for (int i=0;i<nSecSize;i++)
	{
		int iId;
		resTable.GetCell(0,i,iId);
		memcpy_s(pWrite,iSize,&iId,sizeof(int));
		pWrite += sizeof(int);
	}

	nSecSize = (int)iDates.size();
	memcpy_s(pWrite,iSize,&nSecSize,sizeof(int));
	pWrite += sizeof(int);

	for (int i=0;i<nSecSize;i++)
	{
		memcpy_s(pWrite,iSize,&iDates[i],sizeof(int));
		pWrite += sizeof(int);
	}

	//LPCTSTR lpUrl = _T("http://192.168.5.87/FundNavSer/ServerHandler.ashx");
	//LPCTSTR lpUrl = _T("http://192.168.6.89/FundNavSerBinary/ServerHandler.ashx");
	//LPCTSTR lpUrl = _T("http://221.122.41.211/FundNavSer/ServerHandler.ashx");
	LPCTSTR lpUrl = CORE_GETSYSINFO->GetServerAddr(_T("File"),_T("FundNavRaiseRate"));

	Tx::Drive::Http::CSyncUpload upload;
	int iStart = ::GetTickCount();
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

		iStart = ::GetTickCount();
		LPBYTE pRecv = lpData;
		UINT nParseCount = resTable.GetRowCount() * iDates.size();
		float fValue = 0.0;
		double dValue = 0.0;
		for (UINT i=0;i < nParseCount; i++)
		{
			memcpy_s(&fValue,sizeof(float),pRecv,sizeof(float));
			pRecv += sizeof(float);
			if (fabs(fValue - Tx::Core::Con_floatInvalid) < 0.00001)
				// 无效值
				//resultTable.SetCell(i%date.size() + 1,i/date.size(),Tx::Core::Con_doubleInvalid);
			{
				resTable.SetCell(i%iDates.size() + 2,i/iDates.size(),Tx::Core::Con_doubleInvalid);
			}
			else
			{
				dValue = (double)fValue;
				resTable.SetCell(i%iDates.size() + 2,i/iDates.size(),dValue);
			}
		}
		delete []lpData;
		lpData = NULL;
		iEnd = ::GetTickCount();
		TRACE(_T("\r\nParse Result Cost Time %d(ms)\r\n"),iEnd-iStart);
	}
	delete[] pBuffer;
	pBuffer = NULL;

	return true;
}

bool TxFund::FundSample( Tx::Core::Table_Display* pResTable,long lSecurityId)
{
// #ifdef _DEBUG
// 	SYSTEMTIME st1, st2;
//  	GetLocalTime(&st1);
// #endif

	int idMaster = Tx::Data::FundNV2MasterMap::GetInstance()->GetMasterId((int)lSecurityId);
	if (idMaster > 0)
	{
		lSecurityId = (long)idMaster;
	}

	SecurityQuotation* pSecurityFund = GetSecurityNow(lSecurityId);
	if (NULL == pSecurityFund || !pSecurityFund->IsFund())
	{
		return false;
	}
	long lFundId = pSecurityFund->GetSecurity1Id();

	std::vector< int > nVec;
	std::vector<double> dHoldingShares;
	int iNewestReportDate = 0;
	FundCombineInvestmentVIPStock TempData;
	int icount = pSecurityFund->GetDataCount(Tx::Data::dt_FundCombineInvestmentVIPStock);
	if (icount < 1)
	{
		return false;
	}

	int iId = 0;
	for(int i = icount - 1;i >= icount - 10 && i >= 0;i--)
	{
		bool Res = pSecurityFund->GetDataByIndex(Tx::Data::dt_FundCombineInvestmentVIPStock,i,TempData);
		if(Res)
		{
			if(iId != 0 && iId != TempData.iId)
				break;

			iId = TempData.iId;
			nVec.push_back(TempData.f2);
			dHoldingShares.push_back(TempData.dFinancial[2]);
		}
	}
	
	if (0 == nVec.size())
	{
		return false;
	}

	pResTable->Clear();
	int col = 0;
	//表列的初始化指定

	//名称
	pResTable->AddCol(dtype_val_string);
	pResTable->SetTitle(col++, _T("名称"));

	//代码
	pResTable->AddCol(dtype_val_string);
	pResTable->SetTitle(col++, _T("代码"));

	//前收
	pResTable->AddCol(dtype_double);
	pResTable->SetTitle(col++, _T("前收"));

	//收盘
	pResTable->AddCol(dtype_double);
	pResTable->SetTitle(col++, _T("现价"));
	//20091118根据需求 “收盘”改“现价”
	//涨幅
	pResTable->AddCol(dtype_double);
	pResTable->SetTitle(col++,_T("涨幅(%)"));

	//成交量
	pResTable->AddCol(dtype_double);
	pResTable->SetTitle(col++,_T("成交量(手)"));

	//成交金额
	pResTable->AddCol(dtype_double);
	pResTable->SetTitle(col++,_T("成交金额(万元)"));

	//换手率
	pResTable->AddCol(dtype_double);
	pResTable->SetTitle(col++,_T("换手率(%)"));


	//贡献率
	pResTable->AddCol(dtype_double);
	pResTable->SetTitle(col++, _T("贡献率(%)"));

// #ifdef _DEBUG
// 	INT64 iTime1=0,iTime2=0;
// 	GetLocalTime(&st2);
// 	iTime1 = (1000 * (st1.wHour * 3600 + st1.wMinute * 60 + st1.wSecond) + st1.wMilliseconds);
// 	iTime2 = (1000 * (st2.wHour * 3600 + st2.wMinute * 60 + st2.wSecond) + st2.wMilliseconds);
// 	TRACE("\t\nPart1耗时 %I64d",(iTime2-iTime1));
// 	GetLocalTime(&st1);
// #endif

	//Tx::Core::Table_Indicator hbTable;
//     bool Res = GetValueOfStock(hbTable,lFundId);
// 	if (!Res)
// 	{
// 		hbTable.Clear();
// 	}
// #ifdef _DEBUG
// 	GetLocalTime(&st2);
// 	iTime1 = (1000 * (st1.wHour * 3600 + st1.wMinute * 60 + st1.wSecond) + st1.wMilliseconds);
// 	iTime2 = (1000 * (st2.wHour * 3600 + st2.wMinute * 60 + st2.wSecond) + st2.wMilliseconds);
// 	TRACE("\t\nPart2耗时 %I64d",(iTime2-iTime1));
// 	GetLocalTime(&st1);
// #endif
    std::vector<int>::iterator iter2;

	CTime time = CTime::GetCurrentTime();
	int current = time.GetYear()*10000 + time.GetMonth()*100 + time.GetDay();
	CTimeSpan timeSpan(1,0,0,0);
	CTime time2 = time - timeSpan;
	int yesterday = time2.GetYear()*10000 + time2.GetMonth()*100 + time2.GetDay();

	int row = 0;

	for( std::vector<int>::iterator iter = nVec.begin(); iter != nVec.end(); ++iter )
	{
		SecurityQuotation* pSecurityStock = GetSecurityNow( *iter );
		if (pSecurityStock == NULL || !pSecurityStock->IsStock())
		{
			continue;
		}

		pResTable->AddRow();

		col = 0;
		//名称
		CString str = pSecurityStock->GetName();
		pResTable->SetCell( col++,row,str );
		//代码
		str = pSecurityStock->GetCode();
		pResTable->SetCell( col++,row,str );
		//前收
		double dPreclose = pSecurityStock->GetPreClosePrice();
		if (dPreclose > 0.0)
		{
			pResTable->SetCell(col,row,dPreclose);
		}
		col++;

		//收盘
		double dClose = pSecurityStock->GetClosePrice( true );
		if (dClose > 0.0)
		{
			pResTable->SetCell(col,row,dClose);
		}
		col++;

		//涨幅
		double dPercent = 0.0;
		if ( dPreclose > 0.0 && dClose > 0.0 )
		{
			dPercent = (dClose - dPreclose) / dPreclose * 100;
			pResTable->SetCell(col ,row, dPercent);
		}
		col++;

		//成交量
		double dVolume = pSecurityStock->GetVolume();
		if( dVolume > 0.0 )
		{
			pResTable->SetCell(col, row, dVolume / 100);
		}
		col++;

		//成交金额
		double dAmount = pSecurityStock->GetAmount();
		if (dAmount > 0.0)
		{
			pResTable->SetCell(col,row,dAmount / 10000);
		}
		col++;

		//换手率
		double dExRatio = 0.0;
		if (dVolume > 0.0 && dAmount > 0.0)
		{
			dExRatio = pSecurityStock->GetTradeRate();
			pResTable->SetCell( col,row,dExRatio);
		}
		col++;

		//计算基金的资产净值
		double fundshare = (pSecurityFund->GetTxFundShareDataByDate(yesterday))->TotalShare;
		double fundNetValue = (pSecurityFund->GetFundNetValueDataByIndex(yesterday))->fNetvalue;
		double zcNetValue = fundshare*fundNetValue;
		//贡献率
		double dVotePercent = 0.0;
// 		if (hbTable.GetRowCount() > 0)
// 		{
// 			std::vector<UINT> ResVector;
// 			hbTable.Find(3,*iter,ResVector);
// 			
// 			if (!ResVector.empty())
// 			{
// 				hbTable.GetCell(4,*(ResVector.begin()),StockSZ);
// 			} 
// 			else if (static_cast<int>(ResVector.size()) > 1)
// 			{
// 				ASSERT(0);
// 			}
// 		}
		double dShares = dHoldingShares[row];
		if ( dClose > 0.0 && dPreclose > 0.0 && dShares > 0.0 && zcNetValue > 0.0 )
		{
			dVotePercent = (dClose  - dPreclose) * dShares/ zcNetValue * 100;
			pResTable->SetCell(col, row, dVotePercent);
		}
		col++;

		row++;
	}
// 	 #ifdef _DEBUG
// 	 	GetLocalTime(&st2);
// 	 	iTime1 = (1000 * (st1.wHour * 3600 + st1.wMinute * 60 + st1.wSecond) + st1.wMilliseconds);
// 	 	iTime2 = (1000 * (st2.wHour * 3600 + st2.wMinute * 60 + st2.wSecond) + st2.wMilliseconds);
// 	 	TRACE("\t\nPart3耗时 %I64d",(iTime2-iTime1));
// 	 	GetLocalTime(&st1);
// 	#endif

	return true;
}
bool TxFund::GetValueOfStock(Tx::Core::Table_Indicator &hbTable,long lSecurityId)
{
	//准备样本集参数列
	hbTable.AddParameterColumn(Tx::Core::dtype_int4);//基金ID
	hbTable.AddParameterColumn(Tx::Core::dtype_int4);//报告年份
	hbTable.AddParameterColumn(Tx::Core::dtype_int4);//报告期
	hbTable.AddParameterColumn(Tx::Core::dtype_byte);//F1序号

	const int indicatorIndex3 = 2;
	long iIndicator3[indicatorIndex3] = 
	{
		30901238,	//股票ID
		30901239,	//股票市值	
	};
	bool result = false;
	UINT varCfg3[4];			//参数配置
	int varCount3=4;			//参数个数
	for (int i = 0; i < indicatorIndex3; i++)
	{
		int tempIndicator = iIndicator3[i];

		GetIndicatorDataNow(tempIndicator);
		if (m_pIndicatorData==NULL)
		{ 
			return false;
		}
		varCfg3[0]=0;
		varCfg3[1]=1;
		varCfg3[2]=2;
		varCfg3[3]=3;
		result = m_pLogicalBusiness->SetIndicatorIntoTable(
			m_pIndicatorData,	//指标
			varCfg3,				//参数配置
			varCount3,			//参数个数
			hbTable	//计算需要的参数传输载体以及计算后结果的载体
			);
		if(result==false)
		{ 
			return false;
		}
	}
	result = m_pLogicalBusiness->GetData(hbTable,true);	
// #ifdef _DEBUG
// 	CString strTable=hbTable.TableToString();
// 	Tx::Core::Commonality::String().StringToClipboard(strTable);
// #endif
	UINT iColCount3 = hbTable.GetColCount();
	UINT* nColArray3 = new UINT[iColCount3];
	for(int i = 0; i < (int)iColCount3; i++)
	{
		nColArray3[i] = i;
	}	
	Tx::Core::Table_Indicator tempTable;
	tempTable.CopyColumnInfoFrom(hbTable);
	std::vector<int> iSecurity1Id;
	iSecurity1Id.push_back(lSecurityId);
	//根据基金ID进行筛选
	hbTable.EqualsAt(tempTable,nColArray3,iColCount3,0,iSecurity1Id);
	//为了进行年度和报告期的筛选，所以把报告年份和报告期和为一列。
	TransReportDateToNormal2(tempTable,1);
	MultiSortRule multisort;
	multisort.AddRule(1,false);
	multisort.AddRule(3,true);
	tempTable.SortInMultiCol(multisort);
	tempTable.Arrange();
// #ifdef _DEBUG
// 	strTable=tempTable.TableToString();
// 	Tx::Core::Commonality::String().StringToClipboard(strTable);
// #endif
	int idate;
	tempTable.GetCell(1,0,idate);
	std::vector<int> iDateV;
	iDateV.push_back(idate);
	hbTable.Clear();
	hbTable.CopyColumnInfoFrom(tempTable);
	//进行年度和报告期的筛选
	tempTable.EqualsAt(hbTable,nColArray3,iColCount3-1,1,iDateV);
	if(hbTable.GetRowCount() == 0)
	{
		delete nColArray3;
		nColArray3 = NULL;
		return false;
	}
// #ifdef _DEBUG
// 	strTable=hbTable.TableToString();
// 	Tx::Core::Commonality::String().StringToClipboard(strTable);
// #endif
	delete nColArray3;
	nColArray3 = NULL;
	return true;
}
	}//end namespace Business
}//end namespace Tx

