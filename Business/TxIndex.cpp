/**************************************************************
Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
File name:		TxIndex.cpp
Author:			�Ժ꿡
Date:			2007-10-29
Version:		1.0
Description:	
ָ��ҵ������

***************************************************************/
#include "StdAfx.h"
#include "TxIndex.h"
#include "MyIndicator.h"
#include "..\..\Data\SecurityQuotation.h"
#include "..\..\Core\Core\ProgressWnd.h"
#include "..\..\Data\DataFileStruct.h"
#include "..\..\Data\TypeMapManage.h"
#include "TxStock.h"
#include "..\..\core\driver\ClientFileEngine\public\LogRecorder.h"

#include "..\..\Business\ModIndex\IndexSampleManager.h"
#include "..\..\Core\control\Manage.h"
namespace Tx
{
	namespace Business
	{

		TxIndex::TxIndex(void)
		{
			Tx::Data::HotBlockInfoMap::GetHotBlockInfoMap()->GetMapTxShareDataEx(m_mapTxShareDataEx);
		}

		TxIndex::~TxIndex(void)
		{
			//����
		}
		//�����շ���
		bool TxIndex::BlockRiskIndicatorAdv(
			int iMenuID,						//����ID
			Tx::Core::Table_Indicator& resTable,//������ݱ�
			std::set<int>& iSecurityId,			//����ʵ��ID
			int iEndDate,						//��ֹ����
			long lRefoSecurityId,				//��׼����ʵ��ID
			int iStartDate,						//��ʼ����
			int iTradeDaysCount,				//��������
			int iDuration,						//��������0=�գ�1=�ܣ�2=�£�3=����4=��
			bool bLogRatioFlag,					//������ʵķ�ʽ,true=ָ������,false=�򵥱���
			bool bClosePrice,					//ʹ�����̼�
			int  iStdType,						//��׼������1=��ƫ��������ƫ
			int  iOffset,						//ƫ��
			bool bAhead,						//��Ȩ��־,true=ǰ��Ȩ,false=��Ȩ
			bool bUserRate,						//�û��Զ���������,����ȡһ�궨�ڻ�׼����
			double dUserRate,					//�û��Զ���������
			bool bDayRate						//������
			)
		{
			int ii = iSecurityId.size();
			if(ii<=0)
				return false;

			bool bFirstDayInDuration = false;
			//

			/*
			IndicatorWithParameterArray arr;
			//���ݹ���ID����ָ����Ϣ
			IndicatorFile::GetInstance()->SetIWAP(arr, iMenuID);
			//����ָ����Ϣ����table��
			arr.BuildTableIndicator();
			//׼����ȡ����
			resTable.CopyColumnInfoFrom(arr.m_table_indicator);
			*/
			RiskIndicatorAdv(resTable);

			resTable.DeleteCol(2);
			resTable.DeleteCol(1);

			RiskIndicatorAdv(
				resTable,				//������ݱ�
				iSecurityId,			//����ʵ��ID
				iEndDate,				//��ֹ����
				lRefoSecurityId,		//��׼����ʵ��ID
				bFirstDayInDuration,	//Ĭ�������ڳ�
				iStartDate,				//��ʼ����
				iTradeDaysCount,		//��������
				iDuration,				//��������0=�գ�1=�ܣ�2=�£�3=����4=��
				bLogRatioFlag,			//������ʵķ�ʽ,true=ָ������,false=�򵥱���
				bClosePrice,			//ʹ�����̼�
				iStdType,				//��׼������1=��ƫ��������ƫ
				iOffset,				//ƫ��
				bAhead,					//��Ȩ��־,true=ǰ��Ȩ,false=��Ȩ
				bUserRate,				//�û��Զ���������,����ȡһ�궨�ڻ�׼����
				dUserRate,				//�û��Զ���������,����ȡһ�궨�ڻ�׼����
				bDayRate				//������
				);

			resTable.DeleteCol(2);
			return true;
		}

		//ָ�������仯
		bool TxIndex::SampleChange(
			Tx::Core::Table_Display& resTable,//������ݱ�
			long lSecurityId	//����ʵ��ID
			)
		{
			resTable.AddCol(Tx::Core::dtype_int4);			//ID
			resTable.AddCol(Tx::Core::dtype_val_string);	//name
			resTable.AddCol(Tx::Core::dtype_val_string);	//sid
			resTable.AddCol(Tx::Core::dtype_val_string);	//date
			resTable.AddCol(Tx::Core::dtype_val_string);	//status
			resTable.AddCol(Tx::Core::dtype_val_string);	//cause
			TxBusiness business;
			business.GetSecurityNow(lSecurityId);
			if(business.m_pSecurity == NULL)
				return false;
			//�Զ���ָ��
			if(lSecurityId > 4998600 && lSecurityId < 4999000)
			{
				ReadExIndexFile(lSecurityId,resTable);
				resTable.Sort(3,false);
				return true;
			}
			std::vector<int> lSid; 
			int count= business.m_pSecurity->GetIndexSampleBreakoutDataCount();
			if(count<=0)
				return true;
			ProgressWnd prw;
			UINT pid=prw.AddItem(1,_T("�����仯���ݻ�ȡ..."),0.0);
			prw.Show(2000);
			CString causeStr;
			int nRow = 0;
			for (int i=0;i<count;i++)
			{
				IndexSampleBreakoutData* isb=business.m_pSecurity->GetIndexSampleBreakoutDataByIndex(i);
				if( NULL == isb )
					continue;
				GetSecurityNow( isb->obj_id );
				if ( NULL == m_pSecurity )
				{	
					CString sLog;
					sLog.Format(_T("%d:������������%dΪ��Ч����ʵ�����"),lSecurityId,isb->obj_id);
					Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);	
					continue;
				}
				if ( isb->date <= 0 )
					continue;
				resTable.AddRow();
				resTable.SetCell(0,nRow,(int)m_pSecurity->GetId());
				resTable.SetCell(1,nRow,m_pSecurity->GetName());
				resTable.SetCell(2,nRow,m_pSecurity->GetCode());
				causeStr.Format(_T("%04d-%02d-%02d"),isb->date/10000,isb->date%10000/100,isb->date%100);
				resTable.SetCell(3,nRow,causeStr);
				if(isb->is_in_or_out == 0)
					causeStr = _T("����");
				else
					causeStr = _T("�޳�");
				resTable.SetCell(4,nRow,causeStr);
				//�õ������仯��ԭ��
				causeStr = _T("");
				causeStr = Tx::Data::TypeMapManage::GetInstance()->GetDatByID(TYPE_ID_INDEX_TYPE_NAME,isb->reason_id);
				resTable.SetCell(5,nRow,causeStr);
				prw.SetPercent( pid,double(nRow)/count );		
				nRow++;
			}
#ifdef _DEBUG
			CString strTable = resTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
			resTable.Sort(3,false);
			prw.SetPercent(pid,1.0);
			return true;
		}

		void TxIndex::ReadExIndexFile(int iSecId,Tx::Core::Table_Display& resTable)
		{
			resTable.Clear();
			resTable.AddCol(Tx::Core::dtype_int4);			//ID
			resTable.AddCol(Tx::Core::dtype_val_string);	//name
			resTable.AddCol(Tx::Core::dtype_val_string);	//sid
			resTable.AddCol(Tx::Core::dtype_val_string);	//date
			resTable.AddCol(Tx::Core::dtype_val_string);	//status
			resTable.AddCol(Tx::Core::dtype_val_string);	//cause
			CString		strFileName;
			//strFileName.Format( "%d.hs", iSecId);
			TxBusiness businessTmp;
			businessTmp.GetSecurityNow(iSecId);
			if(businessTmp.m_pSecurity != NULL )
				strFileName.Format(_T("%s.hs"),businessTmp.m_pSecurity->GetCode());
			CFile	HisFile;
			CString  strPath = Tx::Core::Manage::GetInstance()->m_pUserInfo->GetExtIndexPath() + _T("/") + strFileName;
			if ( !HisFile.Open( strPath, CFile::modeRead))
				return;
			int iLen = (int)HisFile.GetLength();
			int iSize = sizeof(IndexSampleHisExt);
			if(iLen < 1 || iLen % iSize != 0)
				return;
			int iCount = iLen/iSize;
			int  ix = 0;
			for (int i = 0 ;i < iCount ; i++)
			{
				IndexSampleHisExt InSamHisExt;
				HisFile.Read(&InSamHisExt,iSize);		
				CString strTmp = _T("");
				TxBusiness business;
				business.GetSecurityNow(InSamHisExt.nID);
				if(business.m_pSecurity == NULL)
				{				
					continue;
				}
				resTable.AddRow();
				resTable.SetCell(0,ix,InSamHisExt.nID);
				strTmp = business.m_pSecurity->GetName();
				resTable.SetCell(1,ix,strTmp);		//name;
				strTmp = business.m_pSecurity->GetCode();
				resTable.SetCell(2,ix,strTmp);		//sid
				int iDate = InSamHisExt.nDate;
				strTmp.Format(_T("%04d-%02d-%02d"),iDate/10000,iDate%10000/100,iDate%100);		
				resTable.SetCell(3,ix,strTmp);		//date
				if(InSamHisExt.nAction == 1)
					strTmp = _T("����");
				else if(InSamHisExt.nAction == -1)
					strTmp = _T("�޳�");
				else
					strTmp = _T("����");
				resTable.SetCell(4,ix,strTmp);		//status
				strTmp = InSamHisExt.nAction == 0? strTmp: _T("ָ��") + strTmp;
				resTable.SetCell(5,ix,strTmp);		//cause
				ix++;
			}	
			HisFile.Close();	
#ifdef _DEBUG
			CString strTable = resTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
		}

		//added by zhangxs 20100607
		void TxIndex::ReadExIndexFile(int iSecId,Tx::Core::Table_Display& resTable,int nStart,int nEnd)
		{
			resTable.Clear();
			resTable.AddCol(Tx::Core::dtype_int4);			//ID
			resTable.AddCol(Tx::Core::dtype_val_string);	//name
			resTable.AddCol(Tx::Core::dtype_val_string);	//sid
			resTable.AddCol(Tx::Core::dtype_val_string);	//date
			resTable.AddCol(Tx::Core::dtype_val_string);	//status
			resTable.AddCol(Tx::Core::dtype_val_string);	//cause
			CString		strFileName;
			//strFileName.Format( "%d.hs", iSecId);
			TxBusiness businessTmp;
			businessTmp.GetSecurityNow(iSecId);
			if(businessTmp.m_pSecurity != NULL )
				strFileName.Format(_T("%s.hs"),businessTmp.m_pSecurity->GetCode());
			CFile	HisFile;
			CString  strPath = Tx::Core::Manage::GetInstance()->m_pUserInfo->GetExtIndexPath() + _T("/") + strFileName;
			if ( !HisFile.Open( strPath, CFile::modeRead))
				return;
			int iLen = (int)HisFile.GetLength();
			int iSize = sizeof(IndexSampleHisExt);
			if(iLen < 1 || iLen % iSize != 0)
				return;
			int iCount = iLen/iSize;
			int  ix = 0;
			for (int i = 0 ;i < iCount ; i++)
			{
				IndexSampleHisExt InSamHisExt;
				HisFile.Read(&InSamHisExt,iSize);		
				CString strTmp = _T("");
				TxBusiness business;
				business.GetSecurityNow(InSamHisExt.nID);
				if(business.m_pSecurity == NULL)
				{				
					continue;
				}
				if(InSamHisExt.nDate > nEnd || InSamHisExt.nDate < nStart)
					continue;
				resTable.AddRow();
				resTable.SetCell(0,ix,InSamHisExt.nID);
				strTmp = business.m_pSecurity->GetName();
				resTable.SetCell(1,ix,strTmp);		//name;
				strTmp = business.m_pSecurity->GetCode();
				resTable.SetCell(2,ix,strTmp);		//sid
				int iDate = InSamHisExt.nDate;
				strTmp.Format(_T("%04d-%02d-%02d"),iDate/10000,iDate%10000/100,iDate%100);		
				resTable.SetCell(3,ix,strTmp);		//date
				if(InSamHisExt.nAction == 1)
					strTmp = _T("����");
				else if(InSamHisExt.nAction == -1)
					strTmp = _T("�޳�");
				else
					strTmp = _T("����");
				resTable.SetCell(4,ix,strTmp);		//status
				strTmp = InSamHisExt.nAction == 0? strTmp: _T("ָ��") + strTmp;
				resTable.SetCell(5,ix,strTmp);		//cause
				ix++;
			}	
			HisFile.Close();	
#ifdef _DEBUG
			CString strTable = resTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
		}
		//����11606
		double TxIndex::GetIndexMV(long lSecurityId,TypeMv type)
		{
			double retMV = 0.0;

			GetSecurityNow(lSecurityId);								//��ȡָ������ʵ��
			if(m_pSecurity == NULL)
				return retMV;
			int count= m_pSecurity->GetIndexConstituentDataCount();		//ȡ����������
			if(count<=0)
				return Con_doubleInvalid;
			Tx::Business::TxBusiness business;
			for (int i=0;i<count;i++)
			{
				double share = Con_doubleInvalid;
				IndexConstituentData * isd=m_pSecurity->GetIndexConstituentDataByIndex(i);
				business.GetSecurityNow( isd->iSecurityId);
				if ( business.m_pSecurity == NULL )
					continue;

				SecurityQuotation* psq = business.m_pSecurity;
				float fClose = psq->GetClosePrice( true );
				StockNewShare * pStkNewShare = psq->GetStockNewShare();
				if(pStkNewShare == NULL)
					continue;
				switch(type)
				{
				case typeFree:
					share = pStkNewShare->f_free_share;
					//share = psq->GetTradableShare();
					break;
				case typeTradable:
					share = pStkNewShare->tradable_share;
					//share = psq->GetTotalInstitutionShare();
					break;
				case typeTotal:
					share = pStkNewShare->total_share;
					//share = psq->GetTotalShare();
					break;
				}
				if ( fClose != Con_floatInvalid && share != Con_doubleInvalid )
					retMV += fClose * share;
			}
			return retMV;
		}
		bool TxIndex::IndexSample(Tx::Core::Table_Display& resTable,
			long lSecurityId,
			int iSortCol,
			bool bAscend
			)
			/*
			{
			resTable.AddCol(Tx::Core::dtype_int4);			//ID
			resTable.AddCol(Tx::Core::dtype_val_string);	//name
			resTable.AddCol(Tx::Core::dtype_val_string);	//sid
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.AddCol(Tx::Core::dtype_double);		//�ɽ���
			resTable.AddCol(Tx::Core::dtype_double);		//�ɽ����	
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetPrecise( 6, 4 );
			resTable.SetPrecise( 7, 4 );
			resTable.SetPrecise( 8, 0 );
			resTable.SetPrecise( 9, 0 );

			double dTotalMV = 0.0;										//����ֵ����λΪ��
			double dTradeableMV = 0.0;									//��ͨ��ֵ

			double dZMv = 0.0;																	//��ǰ����ֵ
			double dLtMV= 0.0;																	//��ǰ��ͨ��ֵ
			GetSecurityNow(lSecurityId);														//��ȡָ������ʵ��
			std::set<int> lSid; 
			int count= m_pSecurity->GetIndexConstituentDataCount();								//ȡ����������
			double dIndexValue = m_pSecurity->GetPreClosePrice();								//ȡ��ָ����ǰ�ռ�
			if(count<=0)
			{	
			//resTable.DeleteCol(6,2);
			return false;
			}
			//---------------�ж��Ƿ��Զ���ָ��,true�Զ���-----------------
			int		bUserIndex = m_pSecurity->IsIndexCustomByCode();
			double	dRealTotalMV = 0.0;
			double	dRealTradeableMV = 0.0;
			Tx::Business::TxBusiness business;
			for (int i=0;i<count;i++)											//��ȡ������lSid
			{
			IndexConstituentData * isd=m_pSecurity->GetIndexConstituentDataByIndex(i);
			business.GetSecurityNow( isd->iSecurityId);
			if ( business.m_pSecurity == NULL )
			continue;
			lSid.insert(isd->iSecurityId);	
			double dPreclose = business.m_pSecurity->GetPreClosePrice();
			double dClose = business.m_pSecurity->GetClosePrice( true );
			double dLtshare = 0.0;
			double dToShare = 0.0;
			if ( business.m_pSecurity->IsStock())
			{
			Tx::Data::TxShareData* pData = NULL;
			pData = business.m_pSecurity->GetTxShareDataByDate(business.m_pSecurity->GetTradeDateLatest());
			if ( pData != NULL)
			{	
			dLtshare = pData->TradeableShare;
			dToShare = pData->TotalShare;
			}
			}
			if ( dPreclose > 0.0 && dLtshare > 0.0 )
			dTradeableMV += dPreclose * dLtshare;
			if ( dPreclose > 0.0 && dToShare > 0.0 )
			dTotalMV += dPreclose * dToShare;
			if ( dClose > 0.0 )
			{
			dRealTotalMV += dClose * business.m_pSecurity->GetTotalShare();
			dRealTradeableMV += dClose * business.m_pSecurity->GetTradableShare();
			}
			}
			double dRealMV = 0.0;
			bool	bReal = true;		//ʵʱ����
			if (bUserIndex)
			{
			CString sPath = Tx::Core::Manage::GetInstance()->m_pUserInfo->GetExtIndexPath()+_T("\\");
			CString strFileName;
			strFileName.Format("%s%d.ini",sPath,lSecurityId-4000000);
			int		nWeightStyle = 0;	//��Ȩ��ʽ
			nWeightStyle = GetPrivateProfileInt( "parameter_set","weight_style",nWeightStyle, strFileName );
			bReal = GetPrivateProfileInt( "parameter_revise","real_calculate",bReal, strFileName );
			if ( nWeightStyle == 0 )
			dRealMV = dRealTotalMV;
			else if ( nWeightStyle == 1 )
			dRealMV = dRealTradeableMV;
			else
			dRealMV = 0.0;
			}
			else
			dRealMV = dRealTradeableMV;
			int nRow = 0;
			for ( std::set< int >::iterator iter = lSid.begin();iter != lSid.end(); ++iter )
			{
			GetSecurityNow( *iter );
			if ( m_pSecurity == NULL )
			continue;
			resTable.AddRow();
			nRow = resTable.GetRowCount()-1;
			resTable.SetCell(0,nRow,*iter);
			resTable.SetCell(1,nRow,m_pSecurity->GetName());
			resTable.SetCell(2,nRow,m_pSecurity->GetCode());	

			double	dClose = m_pSecurity->GetClosePrice( true );		//���̼۸�
			double	dPreClose = m_pSecurity->GetPreClosePrice();		//ǰ��
			double dPercent;
			if (dClose==Tx::Core::Con_floatInvalid||dPreClose==Tx::Core::Con_floatInvalid)
			dPercent=Tx::Core::Con_doubleInvalid;
			else			
			dPercent = (dClose-dPreClose) / dPreClose;				//�Ƿ�

			double dTradableShare = m_pSecurity->GetTradableShare();	//������ͨ�ɱ�
			double dTotalShare = m_pSecurity->GetTotalShare();			//�����ܹɱ�

			double dVolume = m_pSecurity->GetVolume();					//�ɽ���
			double dAmount = m_pSecurity->GetAmount();					//�ɽ����
			double dDevote = Tx::Core::Con_doubleInvalid;				//����ֵ
			double dDevoteP = Tx::Core::Con_doubleInvalid;				//������
			double dLtWeight = Tx::Core::Con_doubleInvalid;				//��ͨȨ��
			double dTotalWeight =  Tx::Core::Con_doubleInvalid;			//��Ȩ��
			double dPE = Tx::Core::Con_doubleInvalid;					//PE

			double dHSL = Tx::Core::Con_doubleInvalid;					//������
			if ( m_pSecurity->GetTradeRate() > 0.0 )
			dHSL = m_pSecurity->GetTradeRate();


			double dLT; 												//��ͨ��ֵ
			if(fabs(dTradableShare)<0.0000001)
			dLT=Tx::Core::Con_doubleInvalid;				
			else
			if(dClose==Tx::Core::Con_floatInvalid)
			dLT=Tx::Core::Con_doubleInvalid;
			else
			if(dClose==Tx::Core::Con_floatInvalid)
			dLT= Tx::Core::Con_doubleInvalid;
			else
			dLT= dTradableShare * dClose;
			double dTotal;												//����ֵ
			if(dTotalShare==Tx::Core::Con_doubleInvalid)
			dTotal= Tx::Core::Con_doubleInvalid;				
			else
			if(dClose==Tx::Core::Con_floatInvalid)
			dTotal=Tx::Core::Con_doubleInvalid;
			else
			dTotal= dTotalShare * dClose;	
			if ( dClose > 0.0 && dClose != Con_floatInvalid && dPreClose > 0.0 && dPreClose != Con_floatInvalid && dTotalShare != Con_doubleInvalid && dTradableShare != Con_doubleInvalid )
			{
			//-----------------------------------------------------------
			//���㹱�����빱��ֵ
			// 1 ���������ͨ��ֵ������ֵ�ı仯

			double	dLtMvCh = ( dClose - dPreClose ) * dTradableShare;
			double	dToMvCh = (  dClose - dPreClose ) * dTotalShare;
			//dDevoteP = dToMvCh / dTotal;
			if ( dTradeableMV != 0.0 && dTradeableMV != Con_doubleInvalid)
			{
			dDevoteP = dLtMvCh / dTradeableMV;	//--wangzhy--20080723--
			//2 ���㹱��ֵ
			if ( dIndexValue > 0.0 && dIndexValue != Con_floatInvalid )
			{	dDevote = dDevoteP * dIndexValue;
			//�ٷ�������λС��
			dDevoteP = 0.0001 * int (dDevoteP * 1000*10000);
			dDevote = 0.0001 * int( 10000* dDevote);
			}
			else
			dDevoteP = 0.0001 * int (dDevoteP * 100*10000);
			}
			//===================================================================
			}

			if ( dRealTradeableMV > 0.0 && dRealTotalMV > 0.0 )
			{
			dLtWeight = dLT / dRealTradeableMV;
			dTotalWeight = dTotal / dRealTotalMV;
			}
			if ( m_pSecurity->IsStock() && m_pSecurity->GetPePbDataLatest() != NULL )
			dPE = m_pSecurity->GetPePbDataLatest()->ltm_pe;
			else
			dPE = Con_doubleInvalid;

			if ( dPreClose <= 0.0 )
			dPreClose = Tx::Core::Con_doubleInvalid;
			resTable.SetCell(3,nRow,dPreClose);						//ǰ��
			if ( dClose <= 0.0 )
			dClose = Tx::Core::Con_doubleInvalid;
			resTable.SetCell(4,nRow,dClose);						//����
			if ( dPercent != Con_doubleInvalid)
			dPercent *= 100;
			resTable.SetCell(5,nRow,dPercent);						//�Ƿ�
			resTable.SetCell(6,nRow,dDevoteP);						//������

			resTable.SetCell(7,nRow,dDevote);						//����ֵ
			if ( dVolume <= 0.0 )
			dVolume = Con_doubleInvalid;
			else
			dVolume /= 100;
			resTable.SetCell(8,nRow,dVolume );						//�ɽ���
			if ( dAmount<=0 )
			dAmount = Con_doubleInvalid;
			else
			dAmount /= 10000;
			resTable.SetCell(9,nRow,dAmount );						//�ɽ����
			if ( dHSL <= 0.0 )
			dHSL = Con_doubleInvalid;
			resTable.SetCell(10,nRow,dHSL);							//������
			resTable.SetCell(11,nRow,dPE);							//PE
			double temp = dLT/100000000;
			if ( temp <= 0.0 )
			temp = Con_doubleInvalid;
			resTable.SetCell(12,nRow,temp);							//��ͨ��ֵ
			temp = dTotal/100000000;
			if ( temp <= 0.0 )
			temp = Con_doubleInvalid;
			resTable.SetCell(13,nRow,temp);							//����ֵ	
			temp = 100*dLtWeight;
			if ( temp <= 0.0 )
			temp = Con_doubleInvalid;
			resTable.SetCell(14,nRow,temp);							//��ͨȨ��
			temp = 100*dTotalWeight;
			if (temp <= 0.0)
			temp = Con_doubleInvalid;
			resTable.SetCell(15,nRow,temp );							//��Ȩ��
			}
			if( iSortCol > -1 )
			resTable.Sort( iSortCol,bAscend );
			return true;
			}
			*/

		{
			resTable.AddCol(Tx::Core::dtype_int4);			//ID
			resTable.AddCol(Tx::Core::dtype_val_string);	//name
			resTable.AddCol(Tx::Core::dtype_val_string);	//sid
			resTable.AddCol(Tx::Core::dtype_float);			//preclose
			resTable.AddCol(Tx::Core::dtype_float);			//close
			resTable.AddCol(Tx::Core::dtype_double);		//raise percent
			resTable.AddCol(Tx::Core::dtype_double);		//devotep
			resTable.AddCol(Tx::Core::dtype_double);		//devote
			resTable.AddCol(Tx::Core::dtype_double);		//�ɽ���
			resTable.AddCol(Tx::Core::dtype_double);		//�ɽ����	
			resTable.AddCol(Tx::Core::dtype_double);		//rate
			resTable.AddCol(Tx::Core::dtype_double);		//pe
			resTable.AddCol(Tx::Core::dtype_double);		//tradeablemv
			resTable.AddCol(Tx::Core::dtype_double);		//totalMv
			resTable.AddCol(Tx::Core::dtype_double);		//tradeableweight
			resTable.AddCol(Tx::Core::dtype_double);		//totalweight
			resTable.SetPrecise( 6, 4 );
			resTable.SetPrecise( 7, 4 );
			resTable.SetPrecise( 8, 0 );
			resTable.SetPrecise( 9, 0 );
			resTable.SetPrecise( 14, 4 );
			resTable.SetPrecise( 15, 4 );
			//--�������⴦�������ָ�������ľ��ȣ�Ҫ��3λbug_no 2033
			bool bHavFundSample = false;
			double dPreTotalMV = 0.0;									//����ֵ����λΪ��
			double dPreTradeableMV = 0.0;								//��ͨ��ֵ
			double dRealTotalMV = 0.0;
			double dRealTradeableMV = 0.0;
			GetSecurityNow(lSecurityId);								//��ȡָ������ʵ��
			std::set<int> lSid; 
			int count= m_pSecurity->GetIndexConstituentDataCount();		//ȡ����������
			float fIndexValue = m_pSecurity->GetPreClosePrice();		//ȡ��ָ����ǰ�ռ�
			if(count<=0)
				return false;
			//modified by zhangxs 20110303 �⼸��ָ����������������ͨ���ܹɱ���ݶ����
			bool bIsSpecialId = false;
			if(lSecurityId == 4000363 || lSecurityId == 4000364 || lSecurityId == 4000365
				|| lSecurityId == 4000369
				|| lSecurityId == 4000373||lSecurityId == 4000371||lSecurityId == 4000372 || lSecurityId == 4000374)
				bIsSpecialId = true;
			//---------------�ж��Ƿ��Զ���ָ��,true�Զ���-----------------
			int		bUserIndex = m_pSecurity->IsIndexCustomByCode();
			Tx::Business::TxBusiness business;
			int nRow = 0;
			for (int i=0;i<count;i++)											//��ȡ������lSid
			{
				IndexConstituentData * isd=m_pSecurity->GetIndexConstituentDataByIndex(i);
				business.GetSecurityNow( isd->iSecurityId);
				if ( business.m_pSecurity == NULL )
					continue;
				if ( business.m_pSecurity->IsFund())
					bHavFundSample = true;
				int iSecIdTmp = isd->iSecurityId;
				lSid.insert(isd->iSecurityId);	
				resTable.AddRow();
				nRow = resTable.GetRowCount()-1;
				resTable.SetCell(0,nRow,isd->iSecurityId);									//id
				resTable.SetCell(1,nRow,business.m_pSecurity->GetName());				//����
				resTable.SetCell(2,nRow,business.m_pSecurity->GetCode());				//����
				float fPreclose = business.m_pSecurity->GetPreClosePrice();
				resTable.SetCell(3,nRow,fPreclose );									//ǰ��
				float fClose = business.m_pSecurity->GetClosePrice( true );
				resTable.SetCell(4,nRow,fClose );									//����
				double dChangeRate = 0.0;
				if ( fClose == Con_floatInvalid || fPreclose == Con_floatInvalid )
					dChangeRate = Con_doubleInvalid;
				else
					dChangeRate = (fClose - fPreclose)/ fPreclose * 100;
				resTable.SetCell(5,nRow,dChangeRate);								//�Ƿ�
				double dVolume = business.m_pSecurity->GetVolume();	
				if ( dVolume != Con_doubleInvalid )	                              //�ɽ���
				{
					if (business.m_pSecurity->IsBond())
						resTable.SetCell(8,nRow,dVolume/10);   // mantis:15083   2013-04-16
					else
						resTable.SetCell(8,nRow,dVolume/100);
				}
				else
					resTable.SetCell(8,nRow,Con_doubleInvalid); 
				double dAmount = business.m_pSecurity->GetAmount();
				if (dAmount != Con_doubleInvalid )								//�ɽ����
					resTable.SetCell(9,nRow,dAmount/10000 );
				else
					resTable.SetCell(9,nRow,Con_doubleInvalid);
				double dHSL = business.m_pSecurity->GetTradeRate();
				resTable.SetCell(10,nRow,dHSL );										//������
				double	dPE = Con_doubleInvalid;

				{
					SecurityQuotation* psq = business.m_pSecurity;
					if (psq != NULL)
					{
						int iYear = psq->GetCurDataDateTime().GetYear();
						// ����,Ȩ��
						InstitutionNewInfo* pInstitutionInfo = psq->GetInstitutionNewInfo();
						if (pInstitutionInfo != NULL)
						{
							// �۸�
							PePb* pPePb = psq->GetPePbDataLatest();
							if (pPePb != NULL)
							{
								// �ɱ�
								//StockNewInfo* pStockNewInfo = psq->GetStockNewInfo();
								//if (pStockNewInfo != NULL)
								{
									double dEPS = pPePb->ltm_eps;
									double dLtmPE;
									double dclose = psq->GetClosePrice(true);
									if (pPePb->ltm_eps < -100.0 || dclose < 0.0)
										dLtmPE = Con_doubleInvalid;
									else
										dLtmPE = dclose / dEPS;
									dPE = dLtmPE;
									//int iFiscalYear=0;
									//if (pInstitutionInfo->fiscal_year_quarter == 40040009)
									//	iFiscalYear = pInstitutionInfo->fiscal_year;
									//else
									//	iFiscalYear = pInstitutionInfo->fiscal_year-1;

									//double dDynamicPE;
									//if (iFiscalYear == iYear - 1)
									//{
									//	if (pInstitutionInfo->dynamic_eps1 < -100.0 || dclose < 0.0)
									//		dDynamicPE = Con_doubleInvalid;
									//	else
									//		dDynamicPE = dclose / pInstitutionInfo->dynamic_eps1;
									//	//dPE = dDynamicPE;
									//}
									//else
									//{
									//	iYear--;
									//	if (pInstitutionInfo->dynamic_eps1 < -100.0 || dclose < 0.0)
									//		dDynamicPE = Con_doubleInvalid;
									//	else
									//		dDynamicPE = dclose / pInstitutionInfo->dynamic_eps1;
									//	//dPE = dDynamicPE;
									//}
								}
							}
						}
					}

				}
				resTable.SetCell(11,nRow,dPE );
				double dLtshare = Con_doubleInvalid;
				double dToShare = Con_doubleInvalid;
				//if ( business.m_pSecurity->IsStock())
				{
					dLtshare = business.m_pSecurity->GetTradableShare();
					dToShare = business.m_pSecurity->GetTotalShare();			
				}
				resTable.SetCell(12,nRow,dLtshare);							//�ݴ�
				resTable.SetCell(13,nRow,dToShare);							//�ݴ�
				if ( fPreclose != Con_floatInvalid && dLtshare != Con_doubleInvalid )
					dPreTradeableMV += fPreclose * dLtshare;
				if ( fPreclose != Con_floatInvalid && dToShare != Con_doubleInvalid )
					dPreTotalMV += fPreclose * dToShare;
				if ( fClose != Con_floatInvalid && dLtshare != Con_doubleInvalid )
				{
					dRealTradeableMV += fClose * dLtshare;
				}
				if ( fClose != Con_floatInvalid && dToShare != Con_doubleInvalid )
				{
					dRealTotalMV += fClose * dToShare;
				}
			}
			double dRealMV = 0.0;
			bool	bReal = true;		//ʵʱ����
			int		nWeightStyle = 0;	//��Ȩ��ʽ
			if (bUserIndex)
			{
				CString sPath = Tx::Core::Manage::GetInstance()->m_pUserInfo->GetExtIndexPath()+_T("\\");
				CString strFileName;
				strFileName.Format("%s%d.ini",sPath,lSecurityId-4000000);
				nWeightStyle = GetPrivateProfileInt( "parameter_set","weight_style",nWeightStyle, strFileName );
				bReal = GetPrivateProfileInt( "parameter_revise","real_calculate",bReal, strFileName ) > 0;
				if ( nWeightStyle == 0 )
					dRealMV = dRealTotalMV;
				else if ( nWeightStyle == 1 )
					dRealMV = dRealTradeableMV;
				else
					dRealMV = 0.0;
			}
			else
				dRealMV = dRealTradeableMV;
			int nItemCount = resTable.GetRowCount();
			ASSERT( nItemCount == (int)lSid.size());
			//Modified by zhangxs ָ�������������ʣ�ͨ��������ɱ����м���
			double dTotalRelyedMv = 0;//��������������ֵ
			double dTotalLtRelyMv = 0;//��������������ֵ
			for ( int ix = 0; ix < nItemCount; ix++ )
			{
				int nSecurityId = 0;
				resTable.GetCell(0,ix,nSecurityId);
				float fClose = Con_floatInvalid;
				double dRelyedShare = Con_doubleInvalid;
				double dLTRelyedShare = Con_doubleInvalid;
				float fPreclose = Con_floatInvalid;
				resTable.GetCell(3,ix,fPreclose);
				resTable.GetCell(4,ix,fClose);
				std::unordered_map<int,Tx::Data::TxShareDataEx>::iterator m_iter = m_mapTxShareDataEx.find(nSecurityId);
				if(m_iter != m_mapTxShareDataEx.end())
				{
					Tx::Data::TxShareDataEx tmp;
					tmp = m_iter->second;
					dRelyedShare = tmp.f_by_free_share;
					dLTRelyedShare = tmp.f_free_share;
				}
				if(fClose > 0.01 && dRelyedShare > 0.01)
					dTotalRelyedMv += fPreclose*dRelyedShare;
				if(fClose > 0.01 && dLTRelyedShare > 0.01)
					dTotalLtRelyMv += fClose*dLTRelyedShare;
			}
			for ( int i = 0; i < nItemCount; i++ )
			{
				int nSecurityId = 0;
				resTable.GetCell(0,i,nSecurityId);
				float fClose = Con_floatInvalid;
				float fPreclose = Con_floatInvalid;
				double dLtShare = Con_doubleInvalid;
				double dTotalShare = Con_doubleInvalid;
				resTable.GetCell(3,i,fPreclose);
				resTable.GetCell(4,i,fClose);
				resTable.GetCell(12,i,dLtShare);
				resTable.GetCell(13,i,dTotalShare);
				double dSamPreTotalMv = Con_doubleInvalid;
				double dSamPreLtMv = Con_doubleInvalid;
				double dSamTotalMv = Con_doubleInvalid;
				double dSamLtMv = Con_doubleInvalid;
				double dLtWeight = Con_doubleInvalid;
				double dTotalWeight = Con_doubleInvalid;
				double dDevote = Con_doubleInvalid;
				double dDevoteP = Con_doubleInvalid;

				if ( fClose != Con_floatInvalid && dLtShare != Con_doubleInvalid && dTotalShare != Con_doubleInvalid )
				{
					dSamLtMv = fClose * dLtShare;
					dSamTotalMv = fClose * dTotalShare;
					dSamPreLtMv = fPreclose * dLtShare;
					dSamPreTotalMv = fPreclose * dTotalShare;
					resTable.SetCell(12,i,dSamLtMv/100000000 );
					resTable.SetCell(13,i,dSamTotalMv/100000000 );
					if ( dRealTotalMV > 0.0 && dRealTradeableMV > 0.0 )
					{
						dLtWeight = dSamLtMv / dRealTradeableMV * 100;
						dTotalWeight = dSamTotalMv / dRealTotalMV * 100;
					}
					resTable.SetCell(14,i,dLtWeight );
					resTable.SetCell(15,i,dTotalWeight );
				}
				else if( fClose != Con_floatInvalid && dLtShare != Con_doubleInvalid )
				{
					dSamLtMv = fClose * dLtShare;			
					dSamPreLtMv = fPreclose * dLtShare;
					resTable.SetCell(12,i,dSamLtMv/100000000 );
					resTable.SetCell(13,i,dSamTotalMv );
					if ( dRealTotalMV > 0.0 && dRealTradeableMV > 0.0 )
					{
						dLtWeight = dSamLtMv / dRealTradeableMV * 100;
					}
					resTable.SetCell(14,i,dLtWeight );
					resTable.SetCell(15,i,dTotalWeight );
				}
				else if( fClose != Con_floatInvalid && dTotalShare != Con_doubleInvalid)
				{
					dSamTotalMv = fClose * dTotalShare;
					dSamPreTotalMv = fPreclose * dTotalShare;
					resTable.SetCell(12,i,dSamLtMv );
					resTable.SetCell(13,i,dSamTotalMv/100000000);
					if ( dRealTotalMV > 0.0 && dRealTradeableMV > 0.0 )
					{
						dTotalWeight = dSamTotalMv / dRealTotalMV * 100;
					}
					resTable.SetCell(14,i,dLtWeight );
					resTable.SetCell(15,i,dTotalWeight );
				}
				else
				{
					resTable.SetCell(12,i,dSamLtMv );
					resTable.SetCell(13,i,dSamTotalMv );
					resTable.SetCell(14,i,dLtWeight );
					resTable.SetCell(15,i,dTotalWeight );
				}
				/*if ( bUserIndex )
				{
				if ( nWeightStyle == 0)
				{
				if ( dSamPreTotalMv != Con_doubleInvalid && dSamTotalMv != Con_doubleInvalid &&  dRealTotalMV != dPreTotalMV )
				dDevoteP = (dSamTotalMv - dSamPreTotalMv) / dPreTotalMV * 1000;
				}
				else if ( nWeightStyle == 1)
				{
				if ( dSamPreLtMv != Con_doubleInvalid && dSamLtMv != Con_doubleInvalid &&  dRealTradeableMV != dPreTradeableMV )
				dDevoteP = (dSamLtMv - dSamPreLtMv) / dPreTradeableMV * 1000;
				}
				}
				else
				{
				if ( dSamPreLtMv != Con_doubleInvalid && dSamLtMv != Con_doubleInvalid &&  dRealTradeableMV != dPreTradeableMV )
				dDevoteP = (dSamLtMv - dSamPreLtMv) / dPreTradeableMV * 1000;
				}
				if ( fIndexValue != Con_floatInvalid && dDevoteP != Con_doubleInvalid )
				dDevote = fIndexValue * dDevoteP / 1000;*/
				//modified by zhangxs 20110302
				double dSamRelyedMv = Con_doubleInvalid;
				double dSamRelyedPreMv = Con_doubleInvalid;
				double dSamRelyedShare = Con_doubleInvalid;
				double dSamLTRelyedMv = Con_doubleInvalid;
				double dSamLTRelyedPreMv = Con_doubleInvalid;
				double dSamLTRelyedShare = Con_doubleInvalid;
				std::unordered_map<int,Tx::Data::TxShareDataEx>::iterator m_iter = m_mapTxShareDataEx.find(nSecurityId);
				if(m_iter != m_mapTxShareDataEx.end())
				{
					Tx::Data::TxShareDataEx tmp;
					tmp = m_iter->second;
					dSamRelyedShare = tmp.f_by_free_share;
					dSamLTRelyedShare = tmp.f_free_share;
				}
#ifdef _DEBUG
				CString strLog = _T("");
				business.GetSecurityNow( nSecurityId);
				if ( business.m_pSecurity != NULL )
				{
					strLog.Format(_T("%s--Kd:%.02f --LT:%.02f\r\n"),business.m_pSecurity->GetName(),dSamRelyedShare,dSamLTRelyedShare);
				}		
				//TRACE(strLog);
#endif

				if(dSamRelyedShare >0.001 && fClose >0.001 && fPreclose >0.001)
				{
					dSamRelyedMv = dSamRelyedShare*fClose;
					dSamRelyedPreMv = dSamRelyedShare*fPreclose;
				}
				if(dSamLTRelyedShare >0.001 && fClose >0.001 && fPreclose >0.001)
				{
					dSamLTRelyedMv = dSamLTRelyedShare*fClose;
					dSamLTRelyedPreMv = dSamLTRelyedShare*fPreclose;
				}
				if(bIsSpecialId)
				{
					if ( dSamTotalMv != Con_doubleInvalid && dSamPreTotalMv != Con_doubleInvalid &&  dPreTotalMV >0.001 )
						dDevoteP = (dSamTotalMv - dSamPreTotalMv) / dPreTotalMV * 10000;
#ifdef _DEBUG
					CString strLogTmp = _T("");
					business.GetSecurityNow( nSecurityId);
					if ( business.m_pSecurity != NULL )
					{
						strLogTmp.Format(_T("%s--������ֵ:%.02f --������ֵ:%.02f--ָ����ֵ:%0.2f--������:%.02f\r\n"),
							business.m_pSecurity->GetName(),dSamTotalMv,dSamPreTotalMv,dPreTotalMV,dDevoteP);
					}		
					TRACE(strLogTmp);
#endif
				}
				else
				{
					if ( bUserIndex )
					{
						if ( nWeightStyle == 0)
						{
							if ( dSamRelyedMv >0.001 && dSamRelyedPreMv >0.001 &&  dTotalRelyedMv >0.001 )
								dDevoteP = (dSamRelyedMv - dSamRelyedPreMv)/ dTotalRelyedMv * 10000;
						}
						else if ( nWeightStyle == 1)
						{
							if ( dSamLTRelyedMv >0.001 && dSamLTRelyedPreMv >0.001 &&  dTotalLtRelyMv >0.001 )
								dDevoteP = (dSamLTRelyedMv - dSamLTRelyedPreMv)/ dTotalLtRelyMv * 10000;
						}
					}
					else
					{
						if ( dSamRelyedMv >0.001 && dSamRelyedPreMv >0.001 &&  dTotalRelyedMv >0.001 )
							dDevoteP = (dSamRelyedMv - dSamRelyedPreMv)/ dTotalRelyedMv * 10000;
					}
				}
				if(fabs(dDevoteP - 0.000001)<0.00001)
					dDevoteP = Con_doubleInvalid;

				if ( fIndexValue != Con_floatInvalid && dDevoteP != Con_doubleInvalid )
					dDevote = fIndexValue * dDevoteP / 10000;
				if(fabs(dDevote - 0.000001)<0.00001)
					dDevote = Con_doubleInvalid;
				resTable.SetCell(6,i,dDevoteP );
				resTable.SetCell(7,i,dDevote );	
			}
			if ( bHavFundSample == true )
			{
				resTable.SetPrecise( 3, 3 );
				resTable.SetPrecise( 4, 3 );
			}
			if( iSortCol > -1 )
				resTable.Sort( iSortCol,bAscend );
			return true;
		}



		bool TxIndex::EtfSample( Tx::Core::Table_Display* pResTable,
			long lSecurityId
			)
		{
			TxBusiness business;
			business.GetSecurityNow( lSecurityId );
			if ( business.m_pSecurity == NULL )
				return false;
			if ( !business.m_pSecurity->IsFund_ETF())
				return false;
			int nCode = _ttoi( business.m_pSecurity->GetCode());
			TxStock *p;
			p = new TxStock;
			std::vector< int > nVec;
			//����50ETF 510050
			//�׻�100ETF 159901
			//����100ETF 510180
			//������С��  159902
			switch( nCode )
			{
			case 510050:
				//��֤50 000016
				p->GetLeafItems( _T("hu50_index"),nVec );
				break;
			case 159901:
				//��֤100�۸�ָ�� 399004
				p->GetLeafItems( _T("s100"),nVec );
				break;
			case 510180:
				//��֤180
				p->GetLeafItems( _T("hu180_index"),nVec );
				break;
			case 159902:
				//��С��100
				p->GetLeafItems( _T("ms100_index"),nVec );
				break;
			case 510880:
				//��֤����ָ������
				p->GetLeafItems( _T("interest_index"),nVec );
				break;
			}
			if ( p != NULL)
			{
				delete p;
				p = NULL;
			}
			if ( nVec.size() == 0 )
				return false;
			pResTable->Clear();
			int col = 0;
			int row = 0;
			//���еĳ�ʼ��ָ��

			//����
			pResTable->AddCol(dtype_val_string);
			pResTable->SetTitle(col++, _T("����"));

			//����
			pResTable->AddCol(dtype_val_string);
			pResTable->SetTitle(col++, _T("����"));

			//ǰ��
			pResTable->AddCol(dtype_double);
			pResTable->SetTitle(col++, _T("ǰ��"));

			//����
			pResTable->AddCol(dtype_double);
			pResTable->SetTitle(col++, _T("�ּ�"));		//�������󣬡����̡���Ϊ���ּۡ�

			//�Ƿ�
			pResTable->AddCol(dtype_double);
			pResTable->SetTitle(col++,_T("�Ƿ�(%)"));

			//�ɽ���
			pResTable->AddCol(dtype_double);
			pResTable->SetTitle(col++,_T("�ɽ���(��)"));

			//�ɽ����
			pResTable->AddCol(dtype_double);
			pResTable->SetTitle(col++,_T("�ɽ����(��Ԫ)"));

			//������
			pResTable->AddCol(dtype_double);
			pResTable->SetTitle(col++,_T("������(%)"));


			//������
			pResTable->AddCol(dtype_double);
			pResTable->SetTitle(col++, _T("������(%)"));

			//����ֵ
			//pResTable->AddCol(dtype_double);
			//pResTable->SetTitle(col++, _T("����ֵ"));

			//���ȼ���������ֵ
			double	dMV = 0.0;
			for( std::vector<int>::iterator iter = nVec.begin(); iter != nVec.end(); ++iter )
			{
				business.GetSecurityNow( *iter );
				if ( business.m_pSecurity == NULL || !business.m_pSecurity->IsStock())
					continue;
				double dPrice = business.m_pSecurity->GetPreClosePrice();
				double dShare = business.m_pSecurity->GetTxShareDataByDate( business.m_pSecurity->GetTradeDateLatest())->TradeableShare;
				if ( dPrice > 0.0 && dShare > 0.0 )
					dMV += dPrice * dShare;
			}
			for( std::vector<int>::iterator iter = nVec.begin(); iter != nVec.end(); ++iter )
			{
				business.GetSecurityNow( *iter );
				if ( business.m_pSecurity == NULL || !business.m_pSecurity->IsStock())
					continue;
				col = 0;
				pResTable->AddRow();
				//����
				CString str = business.m_pSecurity->GetName();
				pResTable->SetCell( col++,row,str );
				//����
				str = business.m_pSecurity->GetCode();
				pResTable->SetCell( col++,row,str );
				//ǰ��
				double dPreclose = business.m_pSecurity->GetPreClosePrice();
				if ( dPreclose > 0.0 )
					pResTable->SetCell( col++,row,dPreclose);
				else
					col++;
				//����
				double dClose = business.m_pSecurity->GetClosePrice( true );
				if ( dClose > 0.0 )
					pResTable->SetCell( col++,row,dClose);
				else
					col++;
				//�Ƿ�
				double dPercent = 0.0;
				if ( dPreclose > 0.0 && dClose > 0.0 )
				{
					dPercent = (dClose - dPreclose) / dPreclose * 100;
					pResTable->SetCell( col++ ,row, dPercent );
				}
				else
					col++;
				//�ɽ���
				double dVolume = business.m_pSecurity->GetVolume();
				if( dVolume > 0.0 )
					pResTable->SetCell( col++, row, dVolume / 100 );
				else
					col++;
				//�ɽ����
				double dAmount = business.m_pSecurity->GetAmount();
				if ( dAmount > 0.0 )
					pResTable->SetCell( col++,row,dAmount / 10000);
				else
					col++;
				//������
				double dExRatio = 0.0;
				if ( dVolume > 0.0 && dAmount > 0.0 )
				{
					dExRatio = business.m_pSecurity->GetTradeRate();
					pResTable->SetCell( col++,row,dExRatio);
				}
				else
					col++;
				//������
				double dVotePercent = 0.0;
				double dShare1 = business.m_pSecurity->GetTxShareDataByDate( business.m_pSecurity->GetTradeDateLatest())->TradeableShare;
				double dShare2 = business.m_pSecurity->GetTradableShare();
				if ( dClose > 0.0 && dPreclose > 0.0 && dShare2 > 0.0 && dShare1 > 0.0 && dMV > 0.0 )
				{
					dVotePercent = (dClose * dShare2 - dPreclose * dShare1)/ dMV * 100;
					pResTable->SetCell( col++, row, dVotePercent );
				}
				else
					col++;
				row++;
			}
			return true;
		}
		//--��δ����--wangzhy---
		bool TxIndex::GetWeightSample( int nSecurityId, std::vector< int >& nVec )
		{
			nVec.clear();
			TxBusiness business;
			business.GetSecurityNow( nSecurityId );
			if ( business.m_pSecurity == NULL )
				return false;
			if ( !business.m_pSecurity->IsFund_ETF())
			{
				int nCode = _ttoi( business.m_pSecurity->GetCode());
				TxStock *p;
				p = new TxStock;
				//����50ETF 510050
				//�׻�100ETF 159901
				//����100ETF 510180
				//������С��  159902
				switch( nCode )
				{
				case 510050:
					//��֤50 000016
					p->GetLeafItems( _T("hu50_index"),nVec );
					break;
				case 159901:
					//��֤100�۸�ָ�� 399004
					p->GetLeafItems( _T("s100"),nVec );
					break;
				case 510180:
					//��֤180
					p->GetLeafItems( _T("hu180_index"),nVec );
					break;
				case 159902:
					//��С��100
					p->GetLeafItems( _T("ms100_index"),nVec );
					break;
				case 510880:
					//��֤����ָ������
					p->GetLeafItems( _T("interest_index"),nVec );
					break;
				}
				return true;
			}
			else if( business.m_pSecurity->IsFund())
			{
				Tx::Data::FundIndexInvestmentVIPStock* pFundIndexVIPStock;
				int nCount = business.m_pSecurity->GetDataCount( Tx::Data::dt_FundIndexInvestmentVIPStock );
				business.m_pSecurity->GetDataByIndex( Tx::Data::dt_FundIndexInvestmentVIPStock,nCount-1,&pFundIndexVIPStock);
				if (NULL ==pFundIndexVIPStock )
					return false;
				int nSeq = pFundIndexVIPStock->f1;
				pFundIndexVIPStock -= nSeq-1;
				while( nSeq-- >= 1 )
				{
					nVec.push_back( pFundIndexVIPStock->f2 );
					pFundIndexVIPStock++;
				}
				return true;
			}
			else
				return false;
		}
		//���=�������
		//baTable�Ѿ����������[����ʵ��]
		bool TxIndex::GetBlockAnalysis(Table_Display& baTable,std::vector<int>& arrSamples,int& iSortCol,int iMarketId,bool bNeedProgressWnd,int nCollID)
		{
			TxBusiness::GetBlockAnalysis(baTable,arrSamples,iSortCol,iMarketId,bNeedProgressWnd,nCollID);
			//ֻ������300��ҵָ����ʾȨ��
			if(nCollID != 50010668 && nCollID != 50010669)
				baTable.DeleteCol(16,3);
			//2007-12-29
			//������
			baTable.DeleteCol(12,4);
			baTable.DeleteCol(8);
			return true;
		}

		bool TxIndex::GetBlockAnalysisCol(Table_Display& baTable,std::vector<int>& arrSamples,int& iSortCol)
		{
			if(TxBusiness::GetBlockAnalysisCol(baTable,arrSamples,iSortCol)==false)
				return false;

			int nCol = 16;
			baTable.AddCol(Tx::Core::dtype_double);
			baTable.AddCol(Tx::Core::dtype_double);
			baTable.AddCol(Tx::Core::dtype_double);

			baTable.SetTitle(nCol, _T("������ͨȨ��(%)"));
			//С����
			baTable.SetPrecise(nCol, 4);
			baTable.SetFormatStyle(nCol, Tx::Core::fs_normal);
			++nCol;

			baTable.SetTitle(nCol, _T("��ͨȨ��(%)"));
			//С����
			baTable.SetPrecise(nCol, 4);
			baTable.SetFormatStyle(nCol, Tx::Core::fs_normal);
			++nCol;

			baTable.SetTitle(nCol, _T("�ܹ�Ȩ��(%)"));
			//С����
			baTable.SetPrecise(nCol, 4);
			baTable.SetFormatStyle(nCol, Tx::Core::fs_normal);
			++nCol;
			return true;
		}

		//��������������ݽ������
		bool TxIndex::SetBlockAnalysisCol(Table_Display& baTable,SecurityQuotation* psq,int& j,int ii)
		{
			if(psq==NULL)
				return false;
			//��ʼ��ʱ��������¹�Ʊ��Ϣ
			double tradeableShare = Con_doubleInvalid;//psq->GetTradableShare();
			double share = Con_doubleInvalid;//psq->GetTotalShare();
			double dValue = Con_doubleInvalid;
			//int idate = psq->GetCurDataDate();
			float closePrice = Con_floatInvalid;//psq->GetClosePrice(idate,true);
			double dFreeWeight = Con_doubleInvalid;//������ͨȨ��
			double dTradableWeight = Con_doubleInvalid;//��ͨȨ��
			double dTotalWeight = Con_doubleInvalid;//�ܹ�Ȩ��

			//��ͨ�ɱ�
			baTable.SetCell(j,ii,tradeableShare);
			j++;
			//�ܹɱ�
			baTable.SetCell(j,ii,share);
			j++;
			//��ͨ��ֵ
			//if(tradeableShare>0 && closePrice>0)
			//	dValue = closePrice*tradeableShare;
			//else
			//��ͨ�ɱ�������С��0
			dValue = Tx::Core::Con_doubleInvalid;
			baTable.SetCell(j,ii,dValue);
			j++;
			//����ֵ
			//if(share>0 && closePrice>0)
			//	dValue = closePrice*share;
			//else
			//��ͨ�ɱ�������С��0
			dValue = Tx::Core::Con_doubleInvalid;
			baTable.SetCell(j,ii,dValue);
			j++;
			//Ȩ��
			if(psq->IsIndex_TX())
			{
				TxIndex txIndex;
				long secId = psq->GetId();
				double dFreeValue = txIndex.GetIndexMV(secId,TxIndex::typeFree);
				double dTradableValue = txIndex.GetIndexMV(secId,TxIndex::typeTradable);
				double dTotalValue = txIndex.GetIndexMV(secId,TxIndex::typeTotal);

				double dHs300FreeValue = txIndex.GetIndexMV(4000032,TxIndex::typeFree);
				double dHs300TradableValue = txIndex.GetIndexMV(4000032,TxIndex::typeTradable);
				double dHs300TotalValue = txIndex.GetIndexMV(4000032,TxIndex::typeTotal);
				if(dFreeValue>0.0 && dHs300FreeValue>0.0)
					dFreeWeight = dFreeValue/dHs300FreeValue *100;
				if(dTradableValue>0.0 && dHs300TradableValue>0.0)
					dTradableWeight = dTradableValue/dHs300TradableValue *100;
				if(dTotalValue>0.0 && dHs300TotalValue>0.0)
					dTotalWeight = dTotalValue/dHs300TotalValue *100;
			}

			baTable.SetCell(j,ii,dFreeWeight);
			j++;
			baTable.SetCell(j,ii,dTradableWeight);
			j++;
			baTable.SetCell(j,ii,dTotalWeight);
			j++;
			return true;
		}
		//��������������ݽ������[������]
		bool TxIndex::SetBlockAnalysisHslCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow)
		{
			double dHsl = Con_doubleInvalid;
			//dHsl = pSecurity->GetTradeRate();
			//������
			baTable.SetCell(nCol,nRow,dHsl);
			return true;
		}

		bool TxIndex::GetPeriodHq( Table_Display& resTable, std::set< int>& arrSample,int nStart, int nEnd )
		{

			if ( nStart > nEnd || nEnd > ZkkLib::DateTime::getCurrentTime().GetDate().GetInt())
			{
				AfxMessageBox(_T("�������ô���!"));
				return false;
			}
			resTable.Clear();
			resTable.AddCol(Tx::Core::dtype_int4);			//ID
			resTable.AddCol(Tx::Core::dtype_val_string);	//name
			resTable.AddCol(Tx::Core::dtype_val_string);	//sid


			resTable.AddCol(Tx::Core::dtype_double);		//preclose
			resTable.AddCol(Tx::Core::dtype_double);		//open
			resTable.AddCol(Tx::Core::dtype_double);		//close
			resTable.AddCol(Tx::Core::dtype_double);			//�ɽ���
			resTable.AddCol(Tx::Core::dtype_double);	  //�ɽ����	
			resTable.AddCol(Tx::Core::dtype_double);	//�ǵ�		
			resTable.AddCol(Tx::Core::dtype_double);	//�ǵ���

			resTable.AddCol(Tx::Core::dtype_int4);		//��������
			//resTable.AddCol(Tx::Core::dtype_double);	//����
			resTable.AddCol(Tx::Core::dtype_double);	//�վ��ɽ���
			resTable.AddCol(Tx::Core::dtype_double);	//�վ��ɽ����
			//resTable.AddCol(Tx::Core::dtype_double);
			//resTable.AddCol(Tx::Core::dtype_double);



			if ( arrSample.size() == 0 )
				return true;
			HisTradeData*	pHisTrade = NULL;
			int	nRow = 0;
			for ( std::set< int >::iterator iter = arrSample.begin(); iter != arrSample.end(); ++iter )
			{		
				GetSecurityNow(*iter);
				if( this->m_pSecurity == NULL || this->m_pSecurity->IsIndex() == false )
					continue;
				//1 ����
				m_pSecurity->LoadHisTrade();
				m_pSecurity->LoadTradeDate();
				//2	�ҳ���Ч����
				int	nIndexBegin = m_pSecurity->GetTradeDateIndex( nStart );
				int nIndexEnd = m_pSecurity->GetTradeDateIndex( nEnd );
				//3 ȡ�ÿ�ʼ��������
				double	dPreclose = Con_doubleInvalid;
				double	dOpen = Con_doubleInvalid;
				pHisTrade = m_pSecurity->GetTradeDataByIndex( nIndexBegin );
				if ( pHisTrade != NULL )
				{
					dPreclose = pHisTrade->Preclose;
					dOpen = pHisTrade->Open;
				}
				//4 ȡ�ý�����������
				double	dClose = Con_doubleInvalid;
				pHisTrade = m_pSecurity->GetTradeDataByIndex( nIndexEnd);
				if ( pHisTrade != NULL)
					dClose = pHisTrade->Close;
				//5 �ǵ�
				double dRaise = Con_doubleInvalid;
				double dRaiseRate = Con_doubleInvalid;
				if ( dPreclose != Con_doubleInvalid && dClose != Con_doubleInvalid )
				{
					dRaise = dClose - dPreclose;
					dRaiseRate = dRaise / dPreclose * 100;
				}
				//6 ������
				int nDays = Con_intInvalid;
				nDays = nIndexEnd - nIndexBegin;
				//7 �ɽ���,�ɽ����
				double	dAmount = 0.0;
				double 	dVolume = 0.0;
				for ( int i = nIndexBegin; i <= nIndexEnd; i++ )
				{
					pHisTrade = m_pSecurity->GetTradeDataByIndex( i );
					if( pHisTrade )
					{
						dAmount += pHisTrade->Amount;
						dVolume += pHisTrade->Volume;
					}
				}
				dAmount /= 10000;
				/*int nVolume = (int )dVolume;*/
				//8 ����
				//double dAveragePrice = Con_doubleInvalid;
				//if ( dAmount > 0.0 && dVolume > 0.0 )
				//	dAveragePrice = dAmount / dVolume;
				//9 �վ��ɽ��� ���վ��ɽ����
				double dDayAmount = Con_doubleInvalid;
				double dDayVolume = Con_doubleInvalid;
				if ( dAmount > 0.0 && dVolume > 0.0 && nDays > 0 )
				{
					dDayAmount = dAmount / nDays;
					dDayVolume = dVolume / nDays;
				}
				//���ñ�����ֵ
				// ȡ�����ƴ���
				CString strName = m_pSecurity->GetName();
				CString strCode = m_pSecurity->GetCode();
				int nID = nRow+1;
				resTable.AddRow();
				resTable.SetCell( 0,nRow,nID);		//���
				resTable.SetCell( 1, nRow,strName); //����
				resTable.SetCell( 2, nRow,strCode); //����
				resTable.SetCell( 3, nRow, dPreclose);
				resTable.SetCell( 4,nRow,dOpen);
				resTable.SetCell( 5,nRow,dClose);
				resTable.SetCell( 6,nRow,dVolume);
				resTable.SetCell(7,nRow,dAmount);
				resTable.SetCell(8,nRow,dRaise);
				resTable.SetCell(9,nRow,dRaiseRate);
				resTable.SetCell(10,nRow,nDays);
				/*resTable.SetCell(11,nRow,dAveragePrice);*/
				resTable.SetCell( 11,nRow,dDayVolume);
				resTable.SetCell( 12,nRow,dDayAmount);
				nRow++;
			}
			return true;
		}
	}
}