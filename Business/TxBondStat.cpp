/**************************************************************
	Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
	File name:		TxBond.cpp
	Author:			guanyd
	Date:			2007-12-13
	Version:		1.0
	Description:	
					债券业务功能类-专项统计部分

***************************************************************/

#include "stdAfx.h"
#include "TxBond.h"
#include "MyIndicator.h"
# include <stdlib.h>

#include "..\..\core\core\Commonality.h"
//2008-1-11 lijw add
#include "TxBusiness.h"
namespace Tx
{
	namespace Business
	{

////3000129 10002306  05国债12
////这个函数的功能是派息统计里的统计
//bool TxBond::TxBondPayInterest(
//	Tx::Core::Table_Indicator& resTable,
//	std::vector<int> iSecurityId,
//	int iStartDate,
//	int iEndDate,
//	bool bAllDate
//	)
//{
//	
//	//默认的返回值状态。
//	bool result = true;
//
//	//ObjectId,int型
//	resTable.AddParameterColumn(Tx::Core::dtype_int4);
//	//DisclousreDate, int型
//	resTable.AddParameterColumn(Tx::Core::dtype_int4);
//
//
//
//	UINT varCfg[2];			//参数配置
//	int varCount=2;			//参数个数	交易实体id，公告日
//   //下面的指标虽然都需要用到公告日期，但是按界面上给的除息日期计算
//	//交易提示 指标组
//	//item_id: 30300081-30300101
//	const int INDICATOR_INDEX= 5 ;
//	long iIndicator[INDICATOR_INDEX]=
//	{	
//		30301217,	//付息年度
//		30301219,	//付息日
//		30301220,	//除息日
//		30301222,	//付息利率
//		30301223,	//付息金额
//	};
//
//	
//	//设定指标列
//	for (int i = 0; i <	INDICATOR_INDEX; i++)
//	{
//	    GetIndicatorDataNow(iIndicator[i]);
//	
//		varCfg[0]=0;
//		varCfg[1]=1;
//
//		result = m_pLogicalBusiness->SetIndicatorIntoTable(m_pIndicatorData,varCfg,varCount,resTable);
//		if(result==false)
//			break;
//	}
//	if(result==false)
//		return false;
//	//根据之前3个步骤的设置进行数据读取，结果数据存放在table中
//	result=m_pLogicalBusiness->GetData(resTable,true);
//	if(result==false)
//		return false;
//
//	Tx::Core::Table_Indicator tmpTable;
//	tmpTable.CopyColumnInfoFrom(resTable);
//
//	int iCol=resTable.GetColCount();
//	UINT *nColArray =new UINT[iCol];
//	for(int i=0;i<iCol;i++)
//	{
//		nColArray[i]=i;
//	}
//	
//	if(resTable.GetRowCount()>0)
//	{
//		if(!bAllDate)
//		{
//#ifdef _DEBUG
//			CString str=resTable.TableToString();
//#endif
//			resTable.Between(tmpTable,nColArray,iCol,4,iStartDate,iEndDate,true,true);
//#ifdef _DEBUG
//			CString tstr3=tmpTable.TableToString();
//#endif
//		}
//		else
//		{
//			tmpTable.Clone(resTable);
//		}
//		resTable.Clear();
//		resTable.CopyColumnInfoFrom(tmpTable);
//		if(tmpTable.GetRowCount()>0)
//		{
//
//			tmpTable.EqualsAt(resTable,nColArray,iCol,0,iSecurityId);
//#ifdef _DEBUG
//			CString tstr=resTable.TableToString();
//#endif
//		}	
//	}
//	resTable.InsertCol(1,Tx::Core::dtype_val_string);//添加名称
//	resTable.InsertCol(2,Tx::Core::dtype_val_string);//添加代码
//	resTable.InsertCol(8,Tx::Core::dtype_val_string);//付息利率CString;
//    //计算分配总额。
//	CString strName,strCode;
//	int subscript;
//	double dPay;
//	CString sRate;//这个代表的应该是分配方案。
//	int isecCount = iSecurityId.size();
//	for (int j = 0;j < isecCount;j++)
//	{
//		std::vector<UINT> vecSecuID;
//		int isecuID = iSecurityId[j];
//		GetSecurityNow(isecuID);
//		if(m_pSecurity == NULL)
//			continue;
//		strName = m_pSecurity->GetName();
//		strCode = m_pSecurity->GetCode();
//		SecurityQuotation* p1 = (SecurityQuotation*)GetSecurity(isecuID);
//		if (p1 == NULL) 
//			continue;		
//		resTable.Find(0,isecuID,vecSecuID);	
//		////计算分配总额
//		////first 取发行数量
//		//BondNewInfo * pBondInfo1 = p1->GetBondNewInfo();
//		//double Account = pBondInfo1->share;
//		int icount = vecSecuID.size();
//		for(int k = 0;k < icount;k++)
//		{
//			subscript = vecSecuID[k];
//			//计算分配总额并把它放到resTable里。            
//			resTable.GetCell(7,subscript,dPay); 
//			if(dPay>0)
//			{
//				sRate.Format("100:%.3f",dPay);//我把它小数点后面的数字由2位改为3位。
//			}
//			else
//			{
//				sRate = "-";
//			}
//			resTable.SetCell(8,k,sRate);
//			/*IssueAccount = Account*dPay/1000000;
//			resTable.SetCell(9,subscript,IssueAccount);*/
//			resTable.SetCell(1,subscript,strName);
//			resTable.SetCell(2,subscript,strCode);			
//		}
//	}			
//	resTable.DeleteCol(7);
//	resTable.DeleteCol(3);
//#ifdef _DEBUG
//	CString strTable=resTable.TableToString();
//	Tx::Core::Commonality::String().StringToClipboard(strTable);
//#endif
//	delete nColArray;
//	return true;
//}

//modify by lijw 2008-04-25
bool TxBond::TxBondIssue(
	Tx::Core::Table_Indicator& resTable,
	std::vector<int> iSecurityId,
	int iStartDate,
	int iEndDate,
	bool bAllDate
//    bool iMarketdate
	)
{
	//添加进度条
//	ProgressWnd* pwd=Tx::Core::ProgressWnd::GetInstance();
	ProgressWnd prw;
	UINT pid=prw.AddItem(1,_T("发行融资统计..."),0.0);
	prw.Show(1000);
	//默认的返回值状态。
	bool result = true;
	//得到全部的上市日期和把交易实体ID转化为券ID；
	std::vector<int> secVector;
	std::unordered_map<int,int> MMarkDate;//存储上市日期。
	std::vector<int>::iterator iter=iSecurityId.begin();
	int markdate;
	int fundId;
	while(iter!=iSecurityId.end())
	{
		SecurityQuotation* p = (SecurityQuotation*)GetSecurity(*iter++);
		if(p==NULL)
			return false;
		BondNewInfo * pBondInfo = p->GetBondNewInfo();
		if(pBondInfo == NULL)
			continue;
		markdate = pBondInfo->ipo_date;
		fundId = p->GetSecurity1Id();
		secVector.push_back(fundId);
		MMarkDate.insert(std::make_pair(fundId,markdate));
	}
	//ObjectId,int型
	resTable.AddParameterColumn(Tx::Core::dtype_int4);
	//DisclousreDate, int型
	resTable.AddParameterColumn(Tx::Core::dtype_int4);

	UINT varCfg[2];			//参数配置
	int varCount=2;			//参数个数	交易实体id，公告日

	//交易提示 指标组
	//item_id: 30300081-30300101
	/*上市日期只需要用到一个参数就是交易实体ID其他的都是券id（dtype_int4类型）和公告日期*/
	const int INDICATOR_INDEX=6;
	long iIndicator[INDICATOR_INDEX]=
	{	
		30301179,	//发行方式
		30301182,	//发行日期
		30301174,	//发行价
		30301176,	//实际发行金额
		30301178,	//集资金额
		30301177,	//发行费用
	};

	//设定指标列
	for (int i = 0; i <	INDICATOR_INDEX; i++)
	{
		GetIndicatorDataNow(iIndicator[i]);

		varCfg[0]=0;
		varCfg[1]=1;

		result = m_pLogicalBusiness->SetIndicatorIntoTable(m_pIndicatorData,varCfg,varCount,resTable);
		if(result==false)
			break;
	}
	if(result==false)
		return false;

	//根据全部的发行日期取数据
	result=m_pLogicalBusiness->GetData(resTable,true);
	if(result==false)
		return false;
	int iCol=resTable.GetColCount();
	UINT *nColArray =new UINT[iCol];
	for(int i=0;i<iCol;i++)
	{
		nColArray[i]=i;
	}
	Tx::Core::Table_Indicator tmpTable;
	tmpTable.CopyColumnInfoFrom(resTable);
	if(resTable.GetRowCount() > 0)
	{
		if(!bAllDate)
		{
			resTable.Between(tmpTable,nColArray,iCol,3,iStartDate,iEndDate,true,true);
		}
		else
			tmpTable.Clone(resTable);
	}
	else
		return false;
	resTable.Clear();
	resTable.CopyColumnInfoFrom(tmpTable);
	if(tmpTable.GetRowCount()>0)
	{
		tmpTable.EqualsAt(resTable,nColArray,iCol,0,secVector);
	}
	else
		return false;
#ifdef _DEBUG
	CString strTable1=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable1);
#endif
	//添加进度条
	prw.SetPercent(pid,0.6);
	resTable.InsertCol(1,Tx::Core::dtype_val_string);//添加名称
	resTable.InsertCol(2,Tx::Core::dtype_val_string);//添加代码
	resTable.InsertCol(5,Tx::Core::dtype_val_string);//添加发行方式的CString
	resTable.InsertCol(7,Tx::Core::dtype_int4);//上市日期
	//resTable.InsertCol(9,Tx::Core::dtype_double);//发行市值
	std::unordered_map<int,CString>::iterator mapIter;
	std::unordered_map<int,CString> StyleMap;
	TypeMapManage::GetInstance()->GetTypeMap(TYPE_BOND_IPO_TYPE,StyleMap);
   //添加上市日期和交易实体ID
	std::vector<UINT> positionVec;
	std::vector<UINT>::iterator Uter;
	CString strName,strCode;
//	double dValue,dfee;
	int position,style;
	for(iter = iSecurityId.begin();iter != iSecurityId.end();++iter)
	{
		GetSecurityNow(*iter);
		if(m_pSecurity == NULL)
			continue;
		fundId = m_pSecurity->GetSecurity1Id();
		markdate = MMarkDate[fundId];
        strName = m_pSecurity->GetName();
		strCode = m_pSecurity->GetCode();
		if(!positionVec.empty())
			positionVec.clear();
		resTable.Find(0,fundId,positionVec);
		for(Uter = positionVec.begin();Uter != positionVec.end();++Uter)
		{
			position = *Uter;
			resTable.SetCell(0,position,*iter);
			resTable.SetCell(1,position,strName);
			resTable.SetCell(2,position,strCode);
			resTable.SetCell(7,position,markdate);
			////计算发行市值
			//resTable.GetCell(10,position,dValue);
			//resTable.GetCell(11,position,dfee);
			//if(dfee >0 && dValue >0)
			//	dValue = dValue - dfee;
			//resTable.SetCell(9,position,dValue);
			//把发行发行方式CString化
			resTable.GetCell(4,position,style);
			mapIter = StyleMap.find(style);
			if(mapIter != StyleMap.end())
				resTable.SetCell(5,position,mapIter->second);
		}
	}
#ifdef _DEBUG
	CString strTable=resTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
    delete nColArray;
	resTable.DeleteCol(3,2);//删除公告日期
	//添加进度条
	prw.SetPercent(pid,1.0);
	return true;
}
//
bool TxBond::IdColToNameAndCode(Tx::Core::Table_Indicator &resTable,int iCol,int iIdType,int iMethodType)
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

		resTable.GetCell(iCol,i,iSecId);
			
		if(iSecId>10000000)
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
						tmpS.Format("%d",s2);
						resTable.SetCell(iCol+1,i,s1);
						resTable.SetCell(iCol+2,i,tmpS);
					}
					else
					{
						resTable.SetCell(iCol+1,i,CString("--"));
						resTable.SetCell(iCol+2,i,CString("--"));
					}
				}
				continue;
			}
		}

		GetSecurityNow(iSecId);
		if(m_pSecurity!=NULL)
		{
			resTable.SetCell(iCol+1,i,m_pSecurity->GetSecurity1Name());
			resTable.SetCell(iCol+2,i,m_pSecurity->GetCode());
		}
		
		
	}//end of  for()
	return true;
}

bool TxBond::TxBondCycleRateAdv(
		std::set<int>& iSecurityId,			//交易实体ID
		int startDate,						//起始日期
		int endDate,						//终止日期
		bool bCutFirstDateRaise,			//剔除首日涨幅
		int	iFQLX,							//复权类型 0-不复权;1=后复权
		Tx::Core::Table_Indicator& resTable,//结果数据表
		int iFlag							//计算类型0-默认；1-债券[bCutFirstDateRaise=true表示全价模式,bCutFirstDateRaise=false表示净价模式]
		)
{
	TxBusiness TempTxBusiness;
	int iPrecise=2;						//小数点位数
	return TempTxBusiness.BlockCycleRateAdv(iSecurityId,startDate,endDate,bCutFirstDateRaise,iFQLX,resTable,iPrecise,1);
	
//	int ii= iSecurityId.size();
//	if(ii<=0)
//		return false;
//	if(endDate<startDate)
//		return false;
//
//	if(m_pShIndex==NULL)
//		return false;
//
//	//4000208=证指数
//	//取得债券的交易实体指针
//	Security* pBaseSecurity;
//	pBaseSecurity = m_pShIndex;
//
//	//取得起始日期=交易日期
//	int iStartDate = pBaseSecurity->GetTradeDateOffset(startDate,0);
//	if(iStartDate<0)
//		return false;
//	//取得终止日期=交易日期
//	int iEndDate = pBaseSecurity->GetTradeDateOffset(endDate,0);
//	if(iEndDate<0)
//		iEndDate = pBaseSecurity->GetTradeDateLatest();
//	if(iEndDate<0)
//		return false;
//
//	//计算交易日期天数
//	int iDays = 0;
//	int iStartIndex = pBaseSecurity->GetTradeDateIndex(iStartDate);
//	int iEndIndex = pBaseSecurity->GetTradeDateIndex(iEndDate);
//	iDays = iEndIndex-iStartIndex+1;
//
//	//取得起始日期的数据
//	OneDayTradeData* pOneDayTradeData = new OneDayTradeData;
//	if(pOneDayTradeData==NULL)
//		return false;
//	if(pOneDayTradeData->Load(iStartDate)==false)
//	{
//		delete pOneDayTradeData;
//		return false;
//	}
//
//	//取得终止日期的数据
//	OneDayTradeData* pOneDayTradeData1 = new OneDayTradeData;
//	if(pOneDayTradeData1==NULL)
//	{
//		delete pOneDayTradeData;
//		return false;
//	}
//	if(pOneDayTradeData1->Load(iEndDate)==false)
//	{
//		delete pOneDayTradeData;
//		delete pOneDayTradeData1;
//		return false;
//	}
//
//	//清空表
//	resTable.Clear();
//
//	//创建结果数据列
//	//name
//	resTable.AddCol(Tx::Core::dtype_val_string);
//	//extcode外码
//	resTable.AddCol(Tx::Core::dtype_val_string);
//	//阶段开盘价：=Open
//	resTable.AddCol(Tx::Core::dtype_float);
//	//阶段收盘价：=Close
//	resTable.AddCol(Tx::Core::dtype_float);
//	//阶段成交量（万股）：= SumVolume2- SumVolume1
//	resTable.AddCol(Tx::Core::dtype_double);
//	//阶段成交额（亿元）：= SumAmount2- SumAmount1
//	resTable.AddCol(Tx::Core::dtype_double);
//	//阶段涨跌幅（％）：= (收盘价 - 前收价) / 前收价 * 100
//	resTable.AddCol(Tx::Core::dtype_double);
//	//阶段均价：= 成交金额 / 成交量
//	resTable.AddCol(Tx::Core::dtype_double);
//	//日均成交量（万股）：=阶段成交量/交易日天数
//	resTable.AddCol(Tx::Core::dtype_double);
//	//日均成交额（亿）：=阶段成交额/交易日天数
//	resTable.AddCol(Tx::Core::dtype_double);
//
//	//新股首日数据
//	DAY_HQ_ITEM* pFirstDateData = new DAY_HQ_ITEM;
//
//	//初始化进度条
//	int i=0;
//	//step1
//	Tx::Core::ProgressWnd* p = Tx::Core::ProgressWnd::GetInstance();
//	//step2
//	CString sProgressPrompt;
//	sProgressPrompt =  _T("阶段行情统计");
//	UINT progId = p->AddItem(1,sProgressPrompt, 0.0);
//	//step3
//	p->Show(15);
//
//	//循环处理样本
//	for(std::set<int>::iterator iter=iSecurityId.begin();iter!=iSecurityId.end();iter++)
//	{
//		//step4 进度条位置
//		p->SetPercent(progId, (double)i/(double)ii);
//
//		//取得当前样本的指针
//		GetSecurityNow(*iter);
//		if(m_pSecurity!=NULL)
//		{
//			//取得起始日期的行情数据
//			DAY_HQ_ITEM* pStart = NULL;
//			pStart = pOneDayTradeData->GetData(*iter);
//			if(pStart==NULL)
//			{
//				//如果没有找到样本数据
//				//取得新股首日数据
//				//bFirstDate = true;
//				pFirstDateData->Code=*iter;
//
//				double dfPreClose = m_pSecurity->GetFirstDateIssuePrice();//02 前收
//				double dfOpen = m_pSecurity->GetFirstDateOpen();			//03 开盘
//				double dfHigh = m_pSecurity->GetFirstDateHigh();			//04 最高
//				double dfLow = m_pSecurity->GetFirstDateLow();				//05 最低
//				double dfClose = m_pSecurity->GetFirstDateClose();			//06 收盘
//
//				pFirstDateData->Preclose = dfPreClose<0 ? Con_floatInvalid:(float)dfPreClose;
//				pFirstDateData->Open = dfOpen<0 ? Con_floatInvalid:(float)dfOpen;
//				pFirstDateData->High = dfHigh<0? Con_floatInvalid:(float)dfHigh;
//				pFirstDateData->Low = dfLow<0?Con_floatInvalid:(float)dfLow;
//				pFirstDateData->Close = dfClose<0?Con_floatInvalid:(float)dfClose;
//				pFirstDateData->Price = pFirstDateData->Close;					//07 前复权价
//
//				pFirstDateData->lSumTradeDays=1;
//				pFirstDateData->Volume=m_pSecurity->GetFirstDateVolume();		//08 成交量
//				pFirstDateData->Amount=m_pSecurity->GetFirstDateAmount();		//09 成交金额
//				pFirstDateData->SumVolume=pFirstDateData->Volume;				//10 累计成交量
//				pFirstDateData->SumAmount=pFirstDateData->Amount;				//11 累计成交金额
//				pStart = pFirstDateData;
//			}
//
//			//查找当前样本终止日期的数据
//			DAY_HQ_ITEM* pEnd = NULL;
//			pEnd = pOneDayTradeData1->GetData(*iter);
//
//			resTable.AddRow();
//			int iCol=0;
//			//resTable.SetCell(iCol,i,*iter);
//			resTable.SetCell(iCol,i,m_pSecurity->GetName());
//			iCol++;
//
//			CString sExtCode;
//
//			sExtCode = m_pSecurity->GetCode();
//			//计算
//			
//			resTable.SetCell(iCol,i,sExtCode);
//			iCol++;
//
//
//			if(pEnd==NULL)
//			{
//				i++;
//				continue;
//			}
//
//
//			if(iFlag==0)
//			{
//				resTable.SetCell(iCol,i,pStart->Open);
//				iCol++;
//
//				resTable.SetCell(iCol,i,pEnd->Close);
//				iCol++;
//
//				double v = Con_doubleInvalid;
//				if(pEnd->SumVolume>0 && pStart->SumVolume>0)
//				{
//					v = pEnd->SumVolume-pStart->SumVolume;
//					if(pStart->Volume>0)
//						v += pStart->Volume;
//				}
//				resTable.SetCell(iCol,i,v);
//				iCol++;
//
//				double a = Con_doubleInvalid;
//				if(pEnd->SumAmount>0 && pStart->SumAmount>0)
//				{
//					a = pEnd->SumAmount-pStart->SumAmount;
//					if(pStart->Amount>0)
//						a+=pStart->Amount;
//				}
//				resTable.SetCell(iCol,i,a);
//				iCol++;
//
//				//2007-09-10
//				//考虑复权类型
//				//考虑是否剔除首日涨幅
//				double preClose = 0;
//				double nowClose = 0;
//				preClose = pStart->Preclose;
//				nowClose = pEnd->Close;
//				if(iFQLX==0)//不复权
//				{
//					if(bCutFirstDateRaise==true)
//					{
//						//剔除首日涨幅
//						if(pStart->Close<0)
//							preClose = Con_doubleInvalid;
//						else
//							preClose = pStart->Close;
//					}
//				}
//				else if(iFQLX==1)//后复权
//				{
//					if(pStart->Price<0)
//						preClose = Con_doubleInvalid;
//					else
//						preClose = pStart->Price;
//					if(pEnd->Price<0)
//						nowClose = Con_doubleInvalid;
//					else
//						nowClose = pEnd->Price;
//
//					/*
//					if(bCutFirstDateRaise==true)
//					{
//						//剔除首日涨幅
//						preClose = pStart->Close;
//					}
//					*/
//				}
//
//				double tValue = Con_doubleInvalid;
//				if(pEnd->Close>0 && pStart->Preclose>0)
//					resTable.SetCell(iCol,i,(double)((pEnd->Close-pStart->Preclose)/pStart->Preclose*100));
//				else
//					resTable.SetCell(iCol,i,tValue);
//				iCol++;
//
//				if(v>0 && a>0)
//					resTable.SetCell(iCol,i,a/v);
//				else
//					resTable.SetCell(iCol,i,tValue);
//				iCol++;
//
//				int lDays = pEnd->lSumTradeDays-pStart->lSumTradeDays;
//				if(pEnd->lSumTradeDays>0 && pStart->lSumTradeDays>0 && v>0 && a>0)
//				{
//					resTable.SetCell(iCol,i,v/lDays);
//					iCol++;
//					resTable.SetCell(iCol,i,a/lDays);
//					iCol++;
//				}
//				else
//				{
//					resTable.SetCell(iCol,i,tValue);
//					iCol++;
//					resTable.SetCell(iCol,i,tValue);
//					iCol++;
//				}
//			}
//			else if(iFlag==1)//债券
//			{
//				if(bCutFirstDateRaise==true)
//				{
//					//全价
//				}
//				else
//				{
//					//净价
//				}
//
//				resTable.SetCell(iCol,i,pStart->Open);
//				iCol++;
//
//				resTable.SetCell(iCol,i,pEnd->Close);
//				iCol++;
//
//				double v = Con_doubleInvalid;
//				if(pEnd->SumVolume>0 && pStart->SumVolume>0)
//				{
//					v = pEnd->SumVolume-pStart->SumVolume;
//					if(pStart->Volume>0)
//						v += pStart->Volume;
//				}
//				resTable.SetCell(iCol,i,v);
//				iCol++;
//
//				double a = Con_doubleInvalid;
//				if(pEnd->SumAmount>0 && pStart->SumAmount>0)
//				{
//					a = pEnd->SumAmount-pStart->SumAmount;
//					if(pStart->Amount>0)
//						a+=pStart->Amount;
//				}
//				resTable.SetCell(iCol,i,a);
//				iCol++;
//
//				//2007-09-10
//				//考虑复权类型
//				//考虑是否剔除首日涨幅
//				double preClose = 0;
//				double nowClose = 0;
//				preClose = pStart->Preclose;
//				nowClose = pEnd->Close;
//				if(iFQLX==0)//不复权
//				{
//					//2007-12-04
//					/*王霖：债券没有剔除首日涨幅选项
//					if(bCutFirstDateRaise==true)
//					{
//						//剔除首日涨幅
//						if(pStart->Close<0)
//							preClose = Con_doubleInvalid;
//						else
//							preClose = pStart->Close;
//					}
//					*/
//				}
//				else if(iFQLX==1)//后复权
//				{
//					if(pStart->Price<0)
//						preClose = Con_doubleInvalid;
//					else
//						preClose = pStart->Price;
//					if(pEnd->Price<0)
//						nowClose = Con_doubleInvalid;
//					else
//						nowClose = pEnd->Price;
//
//					/*
//					if(bCutFirstDateRaise==true)
//					{
//						//剔除首日涨幅
//						preClose = pStart->Close;
//					}
//					*/
//				}
//
//				double tValue = Con_doubleInvalid;
//				if(pEnd->Close>0 && pStart->Preclose>0)
//					resTable.SetCell(iCol,i,(double)((pEnd->Close-pStart->Preclose)/pStart->Preclose*100));
//				else
//					resTable.SetCell(iCol,i,tValue);
//				iCol++;
//
//				if(v>0 && a>0)
//					resTable.SetCell(iCol,i,a/v);
//				else
//					resTable.SetCell(iCol,i,tValue);
//				iCol++;
//
//				int lDays = pEnd->lSumTradeDays-pStart->lSumTradeDays;
//				if(pEnd->lSumTradeDays>0 && pStart->lSumTradeDays>0 && v>0 && a>0)
//				{
//					resTable.SetCell(iCol,i,v/lDays);
//					iCol++;
//					resTable.SetCell(iCol,i,a/lDays);
//					iCol++;
//				}
//				else
//				{
//					resTable.SetCell(iCol,i,tValue);
//					iCol++;
//					resTable.SetCell(iCol,i,tValue);
//					iCol++;
//				}
//			}
//			i++;
//		}
//	}
//
//	//step5
//	p->SetPercent(progId, 1.0);
//	sProgressPrompt += _T(",完成!");
//	p->SetText(progId,sProgressPrompt);
//
//	//释放每日行情数据内存
//	delete pOneDayTradeData;
//	delete pOneDayTradeData1;
//
//	//释放内存
//	delete pFirstDateData;
//
//	//delete pStartDateData;
//	//delete pEndDateData;
//	double dValue;
//	for(UINT i=0;i<resTable.GetRowCount();i++)
//	{
//		resTable.GetCell(4,i,dValue);
//		if(dValue>-10000000)
//			resTable.SetCell(4,i,dValue/10000);
//		resTable.GetCell(5,i,dValue);
//		if(dValue>-10000000)
//			resTable.SetCell(5,i,dValue/10000);
//		resTable.GetCell(8,i,dValue);
//		if(dValue>-10000000)
//			resTable.SetCell(8,i,dValue/10000);
//		resTable.GetCell(9,i,dValue);
//		if(dValue>-10000000)
//			resTable.SetCell(9,i,dValue/10000);
//	}
//	return true;
}



	}
}