/**************************************************************
	Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
	File name:		TxBond.cpp
	Author:			guanyd
	Date:			2007-12-13
	Version:		1.0
	Description:	
					ծȯҵ������-ר��ͳ�Ʋ���

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

////3000129 10002306  05��ծ12
////��������Ĺ�������Ϣͳ�����ͳ��
//bool TxBond::TxBondPayInterest(
//	Tx::Core::Table_Indicator& resTable,
//	std::vector<int> iSecurityId,
//	int iStartDate,
//	int iEndDate,
//	bool bAllDate
//	)
//{
//	
//	//Ĭ�ϵķ���ֵ״̬��
//	bool result = true;
//
//	//ObjectId,int��
//	resTable.AddParameterColumn(Tx::Core::dtype_int4);
//	//DisclousreDate, int��
//	resTable.AddParameterColumn(Tx::Core::dtype_int4);
//
//
//
//	UINT varCfg[2];			//��������
//	int varCount=2;			//��������	����ʵ��id��������
//   //�����ָ����Ȼ����Ҫ�õ��������ڣ����ǰ������ϸ��ĳ�Ϣ���ڼ���
//	//������ʾ ָ����
//	//item_id: 30300081-30300101
//	const int INDICATOR_INDEX= 5 ;
//	long iIndicator[INDICATOR_INDEX]=
//	{	
//		30301217,	//��Ϣ���
//		30301219,	//��Ϣ��
//		30301220,	//��Ϣ��
//		30301222,	//��Ϣ����
//		30301223,	//��Ϣ���
//	};
//
//	
//	//�趨ָ����
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
//	//����֮ǰ3����������ý������ݶ�ȡ��������ݴ����table��
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
//	resTable.InsertCol(1,Tx::Core::dtype_val_string);//�������
//	resTable.InsertCol(2,Tx::Core::dtype_val_string);//��Ӵ���
//	resTable.InsertCol(8,Tx::Core::dtype_val_string);//��Ϣ����CString;
//    //��������ܶ
//	CString strName,strCode;
//	int subscript;
//	double dPay;
//	CString sRate;//��������Ӧ���Ƿ��䷽����
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
//		////��������ܶ�
//		////first ȡ��������
//		//BondNewInfo * pBondInfo1 = p1->GetBondNewInfo();
//		//double Account = pBondInfo1->share;
//		int icount = vecSecuID.size();
//		for(int k = 0;k < icount;k++)
//		{
//			subscript = vecSecuID[k];
//			//��������ܶ�����ŵ�resTable�            
//			resTable.GetCell(7,subscript,dPay); 
//			if(dPay>0)
//			{
//				sRate.Format("100:%.3f",dPay);//�Ұ���С��������������2λ��Ϊ3λ��
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
	//��ӽ�����
//	ProgressWnd* pwd=Tx::Core::ProgressWnd::GetInstance();
	ProgressWnd prw;
	UINT pid=prw.AddItem(1,_T("��������ͳ��..."),0.0);
	prw.Show(1000);
	//Ĭ�ϵķ���ֵ״̬��
	bool result = true;
	//�õ�ȫ�����������ںͰѽ���ʵ��IDת��ΪȯID��
	std::vector<int> secVector;
	std::unordered_map<int,int> MMarkDate;//�洢�������ڡ�
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
	//ObjectId,int��
	resTable.AddParameterColumn(Tx::Core::dtype_int4);
	//DisclousreDate, int��
	resTable.AddParameterColumn(Tx::Core::dtype_int4);

	UINT varCfg[2];			//��������
	int varCount=2;			//��������	����ʵ��id��������

	//������ʾ ָ����
	//item_id: 30300081-30300101
	/*��������ֻ��Ҫ�õ�һ���������ǽ���ʵ��ID�����Ķ���ȯid��dtype_int4���ͣ��͹�������*/
	const int INDICATOR_INDEX=6;
	long iIndicator[INDICATOR_INDEX]=
	{	
		30301179,	//���з�ʽ
		30301182,	//��������
		30301174,	//���м�
		30301176,	//ʵ�ʷ��н��
		30301178,	//���ʽ��
		30301177,	//���з���
	};

	//�趨ָ����
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

	//����ȫ���ķ�������ȡ����
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
	//��ӽ�����
	prw.SetPercent(pid,0.6);
	resTable.InsertCol(1,Tx::Core::dtype_val_string);//�������
	resTable.InsertCol(2,Tx::Core::dtype_val_string);//��Ӵ���
	resTable.InsertCol(5,Tx::Core::dtype_val_string);//��ӷ��з�ʽ��CString
	resTable.InsertCol(7,Tx::Core::dtype_int4);//��������
	//resTable.InsertCol(9,Tx::Core::dtype_double);//������ֵ
	std::unordered_map<int,CString>::iterator mapIter;
	std::unordered_map<int,CString> StyleMap;
	TypeMapManage::GetInstance()->GetTypeMap(TYPE_BOND_IPO_TYPE,StyleMap);
   //����������ںͽ���ʵ��ID
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
			////���㷢����ֵ
			//resTable.GetCell(10,position,dValue);
			//resTable.GetCell(11,position,dfee);
			//if(dfee >0 && dValue >0)
			//	dValue = dValue - dfee;
			//resTable.SetCell(9,position,dValue);
			//�ѷ��з��з�ʽCString��
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
	resTable.DeleteCol(3,2);//ɾ����������
	//��ӽ�����
	prw.SetPercent(pid,1.0);
	return true;
}
//
bool TxBond::IdColToNameAndCode(Tx::Core::Table_Indicator &resTable,int iCol,int iIdType,int iMethodType)
{
	//iIdType��Ǵ����������Ǻ������͡�0Ϊʵ�壬1Ϊ����2Ϊ��˾
	//����ʵ�ֲ���������Զ�ʶ�𣬴��޸�
	//��˾idת����id��δ���
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
		resTable.AddCol(Tx::Core::dtype_val_string);	//��˾����
		resTable.AddCol(Tx::Core::dtype_val_string);	//��˾����
	}
	else
	{
		resTable.InsertCol(iCol+1,Tx::Core::dtype_val_string);	//��˾����
		resTable.InsertCol(iCol+2,Tx::Core::dtype_val_string);	//��˾����
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
		std::set<int>& iSecurityId,			//����ʵ��ID
		int startDate,						//��ʼ����
		int endDate,						//��ֹ����
		bool bCutFirstDateRaise,			//�޳������Ƿ�
		int	iFQLX,							//��Ȩ���� 0-����Ȩ;1=��Ȩ
		Tx::Core::Table_Indicator& resTable,//������ݱ�
		int iFlag							//��������0-Ĭ�ϣ�1-ծȯ[bCutFirstDateRaise=true��ʾȫ��ģʽ,bCutFirstDateRaise=false��ʾ����ģʽ]
		)
{
	TxBusiness TempTxBusiness;
	int iPrecise=2;						//С����λ��
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
//	//4000208=ָ֤��
//	//ȡ��ծȯ�Ľ���ʵ��ָ��
//	Security* pBaseSecurity;
//	pBaseSecurity = m_pShIndex;
//
//	//ȡ����ʼ����=��������
//	int iStartDate = pBaseSecurity->GetTradeDateOffset(startDate,0);
//	if(iStartDate<0)
//		return false;
//	//ȡ����ֹ����=��������
//	int iEndDate = pBaseSecurity->GetTradeDateOffset(endDate,0);
//	if(iEndDate<0)
//		iEndDate = pBaseSecurity->GetTradeDateLatest();
//	if(iEndDate<0)
//		return false;
//
//	//���㽻����������
//	int iDays = 0;
//	int iStartIndex = pBaseSecurity->GetTradeDateIndex(iStartDate);
//	int iEndIndex = pBaseSecurity->GetTradeDateIndex(iEndDate);
//	iDays = iEndIndex-iStartIndex+1;
//
//	//ȡ����ʼ���ڵ�����
//	OneDayTradeData* pOneDayTradeData = new OneDayTradeData;
//	if(pOneDayTradeData==NULL)
//		return false;
//	if(pOneDayTradeData->Load(iStartDate)==false)
//	{
//		delete pOneDayTradeData;
//		return false;
//	}
//
//	//ȡ����ֹ���ڵ�����
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
//	//��ձ�
//	resTable.Clear();
//
//	//�������������
//	//name
//	resTable.AddCol(Tx::Core::dtype_val_string);
//	//extcode����
//	resTable.AddCol(Tx::Core::dtype_val_string);
//	//�׶ο��̼ۣ�=Open
//	resTable.AddCol(Tx::Core::dtype_float);
//	//�׶����̼ۣ�=Close
//	resTable.AddCol(Tx::Core::dtype_float);
//	//�׶γɽ�������ɣ���= SumVolume2- SumVolume1
//	resTable.AddCol(Tx::Core::dtype_double);
//	//�׶γɽ����Ԫ����= SumAmount2- SumAmount1
//	resTable.AddCol(Tx::Core::dtype_double);
//	//�׶��ǵ�����������= (���̼� - ǰ�ռ�) / ǰ�ռ� * 100
//	resTable.AddCol(Tx::Core::dtype_double);
//	//�׶ξ��ۣ�= �ɽ���� / �ɽ���
//	resTable.AddCol(Tx::Core::dtype_double);
//	//�վ��ɽ�������ɣ���=�׶γɽ���/����������
//	resTable.AddCol(Tx::Core::dtype_double);
//	//�վ��ɽ���ڣ���=�׶γɽ���/����������
//	resTable.AddCol(Tx::Core::dtype_double);
//
//	//�¹���������
//	DAY_HQ_ITEM* pFirstDateData = new DAY_HQ_ITEM;
//
//	//��ʼ��������
//	int i=0;
//	//step1
//	Tx::Core::ProgressWnd* p = Tx::Core::ProgressWnd::GetInstance();
//	//step2
//	CString sProgressPrompt;
//	sProgressPrompt =  _T("�׶�����ͳ��");
//	UINT progId = p->AddItem(1,sProgressPrompt, 0.0);
//	//step3
//	p->Show(15);
//
//	//ѭ����������
//	for(std::set<int>::iterator iter=iSecurityId.begin();iter!=iSecurityId.end();iter++)
//	{
//		//step4 ������λ��
//		p->SetPercent(progId, (double)i/(double)ii);
//
//		//ȡ�õ�ǰ������ָ��
//		GetSecurityNow(*iter);
//		if(m_pSecurity!=NULL)
//		{
//			//ȡ����ʼ���ڵ���������
//			DAY_HQ_ITEM* pStart = NULL;
//			pStart = pOneDayTradeData->GetData(*iter);
//			if(pStart==NULL)
//			{
//				//���û���ҵ���������
//				//ȡ���¹���������
//				//bFirstDate = true;
//				pFirstDateData->Code=*iter;
//
//				double dfPreClose = m_pSecurity->GetFirstDateIssuePrice();//02 ǰ��
//				double dfOpen = m_pSecurity->GetFirstDateOpen();			//03 ����
//				double dfHigh = m_pSecurity->GetFirstDateHigh();			//04 ���
//				double dfLow = m_pSecurity->GetFirstDateLow();				//05 ���
//				double dfClose = m_pSecurity->GetFirstDateClose();			//06 ����
//
//				pFirstDateData->Preclose = dfPreClose<0 ? Con_floatInvalid:(float)dfPreClose;
//				pFirstDateData->Open = dfOpen<0 ? Con_floatInvalid:(float)dfOpen;
//				pFirstDateData->High = dfHigh<0? Con_floatInvalid:(float)dfHigh;
//				pFirstDateData->Low = dfLow<0?Con_floatInvalid:(float)dfLow;
//				pFirstDateData->Close = dfClose<0?Con_floatInvalid:(float)dfClose;
//				pFirstDateData->Price = pFirstDateData->Close;					//07 ǰ��Ȩ��
//
//				pFirstDateData->lSumTradeDays=1;
//				pFirstDateData->Volume=m_pSecurity->GetFirstDateVolume();		//08 �ɽ���
//				pFirstDateData->Amount=m_pSecurity->GetFirstDateAmount();		//09 �ɽ����
//				pFirstDateData->SumVolume=pFirstDateData->Volume;				//10 �ۼƳɽ���
//				pFirstDateData->SumAmount=pFirstDateData->Amount;				//11 �ۼƳɽ����
//				pStart = pFirstDateData;
//			}
//
//			//���ҵ�ǰ������ֹ���ڵ�����
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
//			//����
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
//				//���Ǹ�Ȩ����
//				//�����Ƿ��޳������Ƿ�
//				double preClose = 0;
//				double nowClose = 0;
//				preClose = pStart->Preclose;
//				nowClose = pEnd->Close;
//				if(iFQLX==0)//����Ȩ
//				{
//					if(bCutFirstDateRaise==true)
//					{
//						//�޳������Ƿ�
//						if(pStart->Close<0)
//							preClose = Con_doubleInvalid;
//						else
//							preClose = pStart->Close;
//					}
//				}
//				else if(iFQLX==1)//��Ȩ
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
//						//�޳������Ƿ�
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
//			else if(iFlag==1)//ծȯ
//			{
//				if(bCutFirstDateRaise==true)
//				{
//					//ȫ��
//				}
//				else
//				{
//					//����
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
//				//���Ǹ�Ȩ����
//				//�����Ƿ��޳������Ƿ�
//				double preClose = 0;
//				double nowClose = 0;
//				preClose = pStart->Preclose;
//				nowClose = pEnd->Close;
//				if(iFQLX==0)//����Ȩ
//				{
//					//2007-12-04
//					/*���أ�ծȯû���޳������Ƿ�ѡ��
//					if(bCutFirstDateRaise==true)
//					{
//						//�޳������Ƿ�
//						if(pStart->Close<0)
//							preClose = Con_doubleInvalid;
//						else
//							preClose = pStart->Close;
//					}
//					*/
//				}
//				else if(iFQLX==1)//��Ȩ
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
//						//�޳������Ƿ�
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
//	sProgressPrompt += _T(",���!");
//	p->SetText(progId,sProgressPrompt);
//
//	//�ͷ�ÿ�����������ڴ�
//	delete pOneDayTradeData;
//	delete pOneDayTradeData1;
//
//	//�ͷ��ڴ�
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