/**********************************************************
  Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
  File name:		StatRisk.cpp
  Author:			��־��
  Version:			1.0
  Date:				2007-10-08
  Description:
					����һ�������࣬������ɻ����ҵ�������ĸ����㷨�ĵ���
***********************************************************/

#include "StdAfx.h"

//#include "vld.h"
#include "StatRisk.h"
#include "..\..\Data\SecurityBase.h"
#include "..\..\Data\BasicInterrest.h"
#include "..\..\Data\HisTrade.h"
#include "../../Core/Core/DataType.h"

namespace Tx
{
	namespace Business
	{
		StatRisk::StatRisk()
		{
			m_fbuf = NULL;
			m_nbuf = NULL;
			m_mbuf = NULL;
			m_buf1 = NULL;
			m_buf2 = NULL;
			m_buf3 = NULL;
			m_nType = JJ_PJ_RISK1;
			m_pMath = new Tx::Core::CStat_Math();
			m_pMath->m_offset = 0;
			m_Table.AddCol( Core::dtype_int4 );
			m_Table.AddCol( Core::dtype_int4 );
	
		}

		StatRisk::~StatRisk()
		{
			if ( m_pMath != NULL )
			{
				delete m_pMath;
				m_pMath = NULL;
			}
			if ( m_nbuf != NULL )
			{
				delete []m_nbuf;
				m_nbuf = NULL;
			}
			if ( m_mbuf != NULL )
			{
				delete []m_mbuf;
				m_mbuf = NULL;
			}
			if ( m_fbuf != NULL )
			{
				delete []m_fbuf;
				m_fbuf = NULL;
			}
			if ( m_buf1 != NULL )
			{
				delete []m_buf1;
				m_buf1 = NULL;
			}
			if ( m_buf2 != NULL )
			{
				delete []m_buf2;
				m_buf2 = NULL;
			}
			if ( m_buf3 != NULL )
			{
				delete []m_buf3;
				m_buf3 = NULL;
			}
			m_Table.Clear();
		}

		void	StatRisk::Set_Param_Default()
		{
			CString sz = "fund";
			CString str;
			CString szPath =  Tx::Core::Manage::GetInstance()->m_pUserInfo->GetUserMathIni();
			str.Format("%d",2 );
			WritePrivateProfileString( sz, "data_cycle",str,szPath );
		}
		// ��ȡ�����ļ��в�������������ͨ��UI�ĶԻ������
		BOOL StatRisk::Get_Param()
		{
			Tx::Business::TxBusiness	shBusiness;
		
			CString sz = "fund";
			CString szPath =  Tx::Core::Manage::GetInstance()->m_pUserInfo->GetUserMathIni();
			iEndDate = shBusiness.m_pShIndex->GetTradeDateLatest();
			iEndDate = min( iEndDate,GetPrivateProfileInt("share","end_date",iEndDate,szPath));
			iStartDate = ( iEndDate / 10000 -1 )*10000 + iEndDate % 10000;
			iStartDate = GetPrivateProfileInt("share","start_date",iStartDate,szPath);
			m_nDate_Type = 0;
			m_nDate_Type = GetPrivateProfileInt("share","date_style",m_nDate_Type,szPath);
			m_algo_type = 1;//���� 2011-12-13,bug 934,�޸�Ĭ�ϲ���
			m_algo_type = GetPrivateProfileInt( sz,"algo_type",m_algo_type,szPath);
			m_nData_type = 2;
			m_nData_type = GetPrivateProfileInt( sz,"data_type",m_nData_type,szPath);
			m_std_type = 0;
			m_std_type = GetPrivateProfileInt(sz,"std_type",m_std_type,szPath);
			m_NoRiskRet_Type = 1;
			m_NoRiskRet_Type =  GetPrivateProfileInt(sz,"NoRiskRet_Type",m_NoRiskRet_Type,szPath);
			m_item_count = 20;
			m_item_count = GetPrivateProfileInt(sz,"item_count",m_item_count,szPath);
			m_nCycle = 2;
			m_nCycle = GetPrivateProfileInt(sz,"day_cycle",m_nCycle,szPath);
			char temp[8];
			GetPrivateProfileString(sz,"UserDefineNrValue","0",temp,7,szPath);
			m_fUserDefineNrValue = atof(temp);
			GetPrivateProfileString(sz,"AvrRet_Code","4000208",m_AvrRetCode,255,szPath);
			return TRUE;	
		}

		// ��ʼ��ҵ�������Ľ��棬��Բ�ͬ��ҵ���������ò�ͬ����
		void StatRisk::Init_ResultData()
		{
			if ( m_nType == JJ_PJ_JY_JBL )  //��������ʼ���(���ڳ�����)
			{
				Init_JY_JBL ();
				return;
			}
			switch( m_nType )
			{
				case JJ_PJ_RISK1:		//ҵ�����շ���
					Init_Risk1_Data ();
					break;

				case JJ_PJ_RISK2:		//���յ�����������
					Init_Risk2_Data ();
					break;

				case JJ_PJ_TM:		//T-M������ģ��
					Init_TM_Data ();
					break;

				case JJ_PJ_HM:		//H-M������ģ��
					Init_HM_Data ();
					break;

				case JJ_PJ_CL:		//C-L������ģ��
					Init_CL_Data ();
					break;

				case JJ_PJ_GII:		//GIIģ��
					Init_GII_Data ();
					break;

				case JJ_PJ_TM3:		//T-M������ģ��
					Init_TM3_Data ();
					break;
				
				case JJ_PJ_HM3:		//H-M������ģ��
					Init_HM3_Data ();
					break;
				
				case JJ_PJ_CL3:		//C-L������ģ��
					Init_CL3_Data ();
					break;
				
				case JJ_PJ_GII3:		//GII������ģ��
					Init_GII3_Data ();
					break;
				case JJ_PJ_FAMA:		//FAMA�ֽ�
					Init_FAMA_Data ();
					break;
				case JJ_PJ_JY_ZXG:		//�����ϵ������(���ڳ�����)
					Init_JY_ZXG ();
					break;

				default:
					break;
			}
		}

		// ��ʼ��ͳ�ƽ�� ResultData
		BOOL StatRisk::Reset_Result_Buffer()
		{
			return TRUE;
		}

		// ��ʼͳ�Ƽ���,������ʱû�мӽ�����
		BOOL StatRisk::Start_Stat()
		{		
			//�Ϸ��Լ���
			if( m_item_count <= 0 && m_nData_type != 1 )
				return FALSE;
			ProgressWnd prw;
			CString sProgress = _T("ҵ����������");
			UINT progId = prw.AddItem(1,sProgress, 0.0);
			prw.Show( 500 );			
			//��ʼ������
			Init_ResultData ();

			if ( m_nType == JJ_PJ_JY_JBL )  //��������ʼ���(���ڳ�����)
			{
				if ( m_Sample.size() <= 1)
				{
					AfxMessageBox("�����������������Ӧ�ô���1��");
					prw.SetPercent( progId, 1.0 );
					return FALSE;
				}
				On_Calc_JY_JBL ();
				prw.SetPercent( progId, 1.0 );
				return TRUE;
			}
			Tx::Business::FundDeriveData	data;
			//���������������,���ݸ��������ڷ�ʽ���������Ҫ�Ľ��������У�����Ĭ��ȥÿ�����ڵ����һ��������
			//------------------------------------------------------------------------------------------------
			//����Ļ�׼ȡ��֤
			Business::TxBusiness		shBusiness;
			shBusiness.m_pShIndex->LoadHisTrade();
			shBusiness.m_pShIndex->LoadTradeDate();
			switch( m_nDate_Type )
			{
			//��ǰ������Ϊ��������
			case 0:
				{
					int nIndex = shBusiness.m_pShIndex->GetTradeDataCount()-1;
					iEndDate = shBusiness.m_pShIndex->GetTradeDateByIndex( nIndex );
					if( nIndex > -1 )
					{
						int offset = m_item_count;
						if( m_nCycle == 1 )
						{
							iStartDate = shBusiness.m_pShIndex->GetTradeDateByIndex( max( nIndex - offset, 0 ) );
						}
						if( m_nCycle == 2 )
						{
							int nWeekIndex = shBusiness.m_pShIndex->GetTradeDateIndex( iEndDate, 0, 1 );
							iStartDate = shBusiness.m_pShIndex->GetTradeDateByIndex( max( 0, nWeekIndex - offset), 1 );	
						}
						if( m_nCycle == 3 )
						{
							int nMonthIndex = shBusiness.m_pShIndex->GetTradeDateIndex( iEndDate, 0, 2 );
							iStartDate = shBusiness.m_pShIndex->GetTradeDateByIndex( max( nMonthIndex - offset, 0),2 );	
						}
						if( m_nCycle == 4 )
						{
							int nQuarterIndex =shBusiness.m_pShIndex->GetTradeDateIndex( iEndDate, 0, 3 );
							iStartDate = shBusiness.m_pShIndex->GetTradeDateByIndex( max( nQuarterIndex - offset, 0), 3 );	
						}
						if ( m_nCycle == 5)//����
						{
							int nQuarterIndex =shBusiness.m_pShIndex->GetTradeDateIndex( iEndDate, 0, 5 );
							iStartDate = shBusiness.m_pShIndex->GetTradeDateByIndex( max( nQuarterIndex - offset, 0), 5 );	
							//��������ģʽ��Ϊ����
							//AfxMessageBox("��������ģʽ��δ����");
							//prw.SetPercent(progId,1.0);
							//return FALSE;
						}
						if( m_nCycle == 6 )
						{
							int nYearIndex = shBusiness.m_pShIndex->GetTradeDateIndex( iEndDate, 0, 4 );
							iStartDate = shBusiness.m_pShIndex->GetTradeDateByIndex( max( 0, nYearIndex - offset),4 );	
						}
					}
				}
				break;
			//ָ����ʼ��ֹ
			case 1:
				{
					iStartDate = max( iStartDate, 19901219 );
					iEndDate = min( iEndDate, shBusiness.m_pShIndex->GetTradeDateLatest() );
				}
				break;
			//ָ��������Ϊ��ֹ����
			case 2:
				{
					int nIndex = shBusiness.m_pShIndex->GetTradeDateIndex( iEndDate );
					iEndDate = shBusiness.m_pShIndex->GetTradeDateByIndex( nIndex );
					if( nIndex > -1 )
					{
						int offset = m_item_count;
						if( m_nCycle == 1 )
						{
							iStartDate = shBusiness.m_pShIndex->GetTradeDateByIndex( max( nIndex - offset , 0 ) );
						}
						if( m_nCycle == 2 )
						{
							int nWeekIndex = shBusiness.m_pShIndex->GetTradeDateIndex( iEndDate, 0, 1 );
							iStartDate = shBusiness.m_pShIndex->GetTradeDateByIndex( max( 0, nWeekIndex - offset), 1 );	
						}
						if( m_nCycle == 3 )
						{
							int nMonthIndex = shBusiness.m_pShIndex->GetTradeDateIndex( iEndDate, 0, 2 );
							iStartDate = shBusiness.m_pShIndex->GetTradeDateByIndex( max( 0, nMonthIndex - offset), 2 );	
						}
						if( m_nCycle == 4 )
						{
							int nQuarterIndex =shBusiness.m_pShIndex->GetTradeDateIndex( iEndDate, 0, 3 );
							iStartDate = shBusiness.m_pShIndex->GetTradeDateByIndex( max( 0, nQuarterIndex - offset ), 3 );	
						}
						if ( m_nCycle == 5)//����
						{
							int nQuarterIndex =shBusiness.m_pShIndex->GetTradeDateIndex( iEndDate, 0, 5 );
							iStartDate = shBusiness.m_pShIndex->GetTradeDateByIndex( max( nQuarterIndex - offset , 0), 5 );	
							//��������ģʽ��Ϊ����
							//AfxMessageBox("��������ģʽ��δ����");
							//prw.SetPercent(progId,1.0);
							//return FALSE;
						}
						if( m_nCycle == 6 )
						{
							int nYearIndex = shBusiness.m_pShIndex->GetTradeDateIndex( iEndDate, 0, 4 );
							iStartDate = shBusiness.m_pShIndex->GetTradeDateByIndex( max( 0, nYearIndex - offset ), 4 );	
						}
					}	
				}
				break;
			default:
				break;
			}
			//*****************************************************************
			
			//���ｫ�û����õĻ��������������ʼ���������ŵ�����
			//--------------------------------------------------------------------------
			CString sz;
			sz.Format("%s", m_AvrRetCode);
			//������Ҫȷ���Ǵ洢������
			int nCode = _ttoi( sz );

			Core::Table	table;
			//Core::Table subTable;
			std::vector< int >	iDates;
			std::vector< int >		iVec( m_Sample.begin(), m_Sample.end() );
			//���ｫ���׻�׼����ʵ��Ҳ�Ž�ȥ
			iVec.push_back( nCode );
			iVec.push_back(4000384);	//С��ָ��
			iVec.push_back(4000382);	//����ָ��
			iVec.push_back(4000390);	//С�̼�ֵ
			iVec.push_back(4000386);	//���̼�ֵ
			iVec.push_back(4000389);	//С�̳ɳ�
			iVec.push_back(4000385);	//���̳ɳ�
			int nCycle = 1;
			if ( m_nCycle == 5 )
				nCycle = 6;
			else
				if ( m_nCycle == 6 )
					nCycle = 4;
				else
					nCycle = m_nCycle-1;

			shBusiness.m_pShIndex->GetDate( iStartDate, iEndDate, nCycle, iDates, 0 );
			//mantis:15454  2013-07-16
			if (nCycle == 0 && iDates.size() > 0)
			{
				int iDateIndex = shBusiness.m_pShIndex->GetTradeDateIndex(iDates[0],0,0,true);
				int iPreDate = shBusiness.m_pShIndex->GetTradeDateByIndex(iDateIndex-1,0,false);
				if (iPreDate > 0)
				{
					iDates.insert(iDates.begin(),iPreDate);
				}
			}

			sort( iDates.begin(), iDates.end());
			std::set< std::pair< int , int > > dates;
			if ( iDates.size() <= 1 )
			{
				AfxMessageBox("������ָ�����ڻ����޷�ʽ�µ�����������");
				return FALSE;
			}
			for( std::vector< int >::iterator iter = iDates.begin() + 1; iter != iDates.end(); iter++ )
			{
				dates.insert( pair< int, int >( *(iter-1), *iter));
			}
			if ( dates.size() < 2 )
			{
				AfxMessageBox("������ָ�����ڻ����޷�ʽ�µ�����������");
				return FALSE;
			}	
			//data.CalcFundNvr( table, iVec, dates );
			if ( m_nData_type == 2 )
			{
				//��ֵ
				if ( m_algo_type == 0 )
					data.CalFundNav_Ext( table, iVec, dates,true, false );
				else
					data.CalFundNav_Ext( table, iVec, dates );	
			}
			else
			{
				//����
				if ( m_algo_type == 0 )
					data.CalFundNav_Ext( table, iVec, dates,false, false );
				else
					data.CalFundNav_Ext( table, iVec, dates,false );	
			}

			
			//*********************************************************************

			//��ȡ��׼�������У���ȡ��m_mbuf��,����ĳ��Ȱ���table�еĳ���			
			//-------------------------------------------------------------------------
			Tx::Business::TxBusiness	Business;
			//�������iDate�е����ڼ����׼������������
			int nTemp = iDates.size() - 1;
			m_mbuf = new double[nTemp];
			//Tx::Data::HisTradeData	*pTradeData;
			double	dClose = 0.0;
			double	dPreClose = 0.0;
			double  dVal = 0.0;
			int nRowIndex = 0;
			std::vector<UINT> vRowIndex;
			vRowIndex.clear();
			table.Find(0,nCode,vRowIndex);
			if ( vRowIndex.size() != 1 )
			{
				//delete []m_mbuf;
				//m_mbuf = NULL;
				return FALSE;
			}
			nRowIndex = *(vRowIndex.begin());
			TRACE(_T("---------------��׼��������----------------"));
			for ( int n = 0; n < nTemp; n++ )
			{
				table.GetCell(n+1,nRowIndex,dVal);
				m_mbuf[n] = dVal;
				TRACE(_T("%f\n"),dVal);
			}
			//*********************************************************************************

			//��ȡ�޷����������У���ȡ��m_nbuf�У�
			//-----------------------------------------------------------------------------------
			m_nbuf = new double[nTemp];
			int nType = 1;		//��Ӧ�����ʷ�ʽ
			double dNrisk = 0.0;
			switch(m_NoRiskRet_Type)
			{	
			case 0:
				//����
				dNrisk = 0.0;
				for ( int i = 0; i < nTemp; i++ )
				{
					m_nbuf[i] = dNrisk;
				}
				break;
			case 1:
				{
					int nSeq = 0;
					Data::BasicInterestData*	p;
					for( std::set<std::pair<int,int>>::iterator iter = dates.begin();iter != dates.end(); ++iter,nSeq++ )
					{
						int nStart = iter->first;
						int nEnd = iter->second;
						double dInterest = 0.0;
						while( nStart < nEnd )
						{	
							//ȡ�õ��յĻ�׼����,
							int index = Data::BasicInterest::GetInstance()->GetIndexByDate( nStart, 8, 1 );
							p = Data::BasicInterest::GetInstance()->GetDataByIndex( index );


							if( p!= NULL)
							{
								if( nStart/10000%400==0 || (nStart/10000%4==0 && nStart/10000%100!= 0))
									dInterest += p->dInterest/100/366;
								else
									dInterest += p->dInterest/100/365;
							}
							COleDateTime begin( nStart/10000,nStart%10000/100,nStart%100,0,0,0);
							begin += COleDateTimeSpan( 1,0,0,0);
							nStart = begin.GetYear()*10000 + begin.GetMonth()*100 + begin.GetDay();
						}
						m_nbuf[nSeq] = dInterest; 
						TRACE(_T("%f\n"),dInterest);
					}	
					/*int index = Data::BasicInterest::GetInstance()->GetIndexByDate( iStartDate, 8, 1 );
					Data::BasicInterestData*	p;
					p = Data::BasicInterest::GetInstance()->GetDataByIndex( index );
					if ( p != NULL)
					{
						if ( 2 == m_nCycle )
							nType = 7;
						if ( 3 == m_nCycle )
							nType = 30;
						dNrisk = ( p->dInterest / 100 ) / 365 * nType;
					}
					*/
				}
				break;
			case 2:
				{
					int nSeq = 0;
					for( std::set<std::pair<int,int>>::iterator iter = dates.begin();iter != dates.end(); ++iter,nSeq++ )
					{
						int nStart = iter->first;
						int nEnd = iter->second;
						double dInterest = 0.0;
						while( nStart < nEnd )
						{	
							if( nStart/10000%400==0 || (nStart/10000%4==0 && nStart/10000%100!= 0))
								dInterest += m_fUserDefineNrValue/100/366;
							else
								dInterest += m_fUserDefineNrValue/100/365;
							COleDateTime begin( nStart/10000,nStart%10000/100,nStart%100,0,0,0);
							begin += COleDateTimeSpan( 1,0,0,0);
							nStart = begin.GetYear()*10000 + begin.GetMonth()*100 + begin.GetDay();
						}
						m_nbuf[nSeq] = dInterest; 
					}
				}
				break;
			default:
				break;
			}
			//************************************************************************************
			
			//if ( m_nType == JJ_PJ_JY_JBL )  //��������ʼ���(���ڳ�����)
			//{
			//	On_Calc_JY_JBL ();
			//	return TRUE;
			//}
			
			//��ʼ�ۼ��������м���
			//------------------------------------------------------------

			//��m_mbuf��m_nbuf���б��ݣ���������ʱ���������ݣ��´�������Ҫ�ָ�ԭ���ݣ��������ּ������ 
			//2013-02-27 wangzf
			bool bRecover = false;    //�ж��Ƿ�Ҫ�ָ�ԭ����m_mbuf��m_nbuf
			double* nbuf = new double[nTemp];
			double* mbuf = new double[nTemp];
			memset( nbuf,0,nTemp * sizeof(double));
			memset( mbuf,0,nTemp * sizeof(double));

			for (int i=0;i<nTemp;i++)
			{
				mbuf[i] = m_mbuf[i];
				nbuf[i] = m_nbuf[i];
			}

			
			TxBusiness	*pFund = new TxBusiness();
			int nRowCount = m_Sample.size();
			int	nTh = 0;
			int nTotal = m_Sample.size()+1;
			m_fbuf = new double[nTemp];
			m_buf1 = new double[nTemp];
			m_buf2 = new double[nTemp];
			m_buf3 = new double[nTemp];

			for( std::set<int>::iterator iter = m_Sample.begin(); iter != m_Sample.end(); ++iter) 
			{
				pFund->GetSecurityNow( *iter );
				if ( pFund->m_pSecurity == NULL)
					continue;
				if(pFund->m_pSecurity->IsFund_Currency()) //�޳������ͻ���
					continue;
				if (!pFund->m_pSecurity->IsFund())
					continue;
				//�õ��û��������������,��ȡ��m_fbuf�У�
				//int iCode;
				std::vector<UINT>	vRow;
				table.Find( 0, *iter,vRow );
				ASSERT( (int)vRow.size() == 1 );
				int nRow = *vRow.begin();
				
				m_Table.AddRow();
				m_Table.SetCell(0,m_Table.GetRowCount()-1,*iter);
				CString str;
				str = pFund->m_pSecurity->GetName();
				m_Table.SetCell( 1, m_Table.GetRowCount()-1, str );
				str = pFund->m_pSecurity->GetCode();
				m_Table.SetCell( 2, m_Table.GetRowCount()-1,str );
				memset( m_fbuf,0,nTemp * sizeof(double));
				for( int n = 0; n < nTemp; n++ )
				{
					double dTemp = 0.0;
					table.GetCell( n+1, nRow, dTemp );
					if ( dTemp == Con_doubleInvalid )
						dTemp = 0;
					m_fbuf[n] = dTemp;
				}
				/*
				for( int i = 0; i < nRowCount; i++ )
				{
					table.GetCell( 0, i, iCode );
					if( iCode == *iter )
					//����һ�����ݶ���m_fbuf;
					{
						m_Table.AddRow();
						m_Table.SetCell(0,m_Table.GetRowCount()-1,*iter);
						CString str;
						str = pFund->m_pSecurity->GetName();
						m_Table.SetCell( 1, m_Table.GetRowCount()-1, str );
						str = pFund->m_pSecurity->GetCode();
						m_Table.SetCell( 2, m_Table.GetRowCount()-1,str );

						if ( pFund->m_pSecurity->IsFund_Currency())
						{
							//------------wangzhy----------------
						}
						m_fbuf = new double[nTemp];
						for( int n = 0; n < nTemp; n++ )
						{
							double dTemp = 0.0;
							table.GetCell( n+1, i, dTemp );
							if ( dTemp == Con_doubleInvalid )
								dTemp = 0;
							m_fbuf[n] = dTemp;
						}
						break;
					}
					else
						continue;
				}
				*/
				//������ݲ�ͬ��ҵ���������ͣ�����m_buf1,m_buf2,m_buf3
				//m_buf1	����ÿ���г�ʱ�����մ����ĵ�λ�����ʲ�����ֵ
				//m_buf2	���ָ���У�С��ָ�������ʣ�����ָ��������
				//m_buf3	�����̼�ֵ�����ʣ�С�̼�ֵ�����ʣ� �������̳ɳ������ʣ�С�̳ɳ������ʣ�
				//-----------------------------------
				//�������m_buf1
				if ( m_nType == JJ_PJ_GII || m_nType == JJ_PJ_GII3 )
				{
					//m_buf1 = new double[nTemp];
					memset( m_buf1,0,nTemp * sizeof(double));
					for ( int i = 0; i < nTemp; i++ )
					{
						m_buf1[i] = 1.0;
						HisTradeData*	pTradeData = NULL;
						HisTradeData*	pTradeData1 = NULL;
						//�������m_buf1[i]��ֵ
						int	nBegin = iDates[i];
						int nEnd = iDates[i+1];
						//����������ʱ����ڵĽ���������
						int	iIndexBegin	=	shBusiness.m_pShIndex->GetTradeDateIndex( nBegin );
						int iIndexEnd = shBusiness.m_pShIndex->GetTradeDateIndex( nEnd );
						double*	buf1 = new double[iIndexEnd-iIndexBegin];
						for ( int j = iIndexBegin; j < iIndexEnd; j++ )
						{
							double	dTemp1 = 0.0;
							double	dTemp2 = 0.0;
							int iToday = shBusiness.m_pShIndex->GetTradeDateByIndex( j );
							int iNextDay = shBusiness.m_pShIndex->GetTradeDateByIndex( j+1 );
							pTradeData = pFund->m_pSecurity->GetTradeDataByNatureDate( iToday );	
							pTradeData1 = pFund->m_pSecurity->GetTradeDataByNatureDate( iNextDay );
							if ( pTradeData != NULL && pTradeData1 != NULL )
							{
								if ( pTradeData->Close != 0.0 )
									dTemp1 = ( pTradeData1->Close - pTradeData->Close ) / pTradeData->Close;								
							}
							pTradeData = Business.m_pSecurity->GetTradeDataByNatureDate( iToday );
							pTradeData1 = Business.m_pSecurity->GetTradeDataByNatureDate( iNextDay );
							if ( pTradeData != NULL && pTradeData1 != NULL )
							{
								if ( pTradeData->Close != 0.0 )
									dTemp2 = ( pTradeData1->Close - pTradeData->Close ) / pTradeData->Close;								
							}
							buf1[j-iIndexBegin] = max( 1+dTemp2,1+dTemp1 );
							m_buf1[i] *= buf1[j-iIndexBegin];
						}
						m_buf1[i] -= 1+m_nbuf[i];
						delete	[]buf1;
					}
				}
				//�������m_buf2,m_buf3
				if ( m_nType == JJ_PJ_GII3 || m_nType == JJ_PJ_TM3 || m_nType ==JJ_PJ_HM3 || m_nType == JJ_PJ_CL3 || m_nType == JJ_PJ_FAMA )
				{
					//m_buf2 = new double[nTemp];
					//m_buf3 = new double[nTemp];
					memset( m_buf2,0,nTemp * sizeof(double));
					memset( m_buf3,0,nTemp * sizeof(double));
					double* buf1 = new double[nTemp];
					double* buf2 = new double[nTemp];
					double* buf3 = new double[nTemp];
					double* buf4 = new double[nTemp];
					double* buf5 = new double[nTemp];
					double* buf6 = new double[nTemp];
					Tx::Business::TxBusiness*	pBusiness = new Tx::Business::TxBusiness();
					//С��ָ��
					vRowIndex.clear();
					table.Find(0,4000384,vRowIndex);
					if ( vRowIndex.size() != 1 )
					{

						delete	[]buf1;
						delete	[]buf2;
						delete	[]buf3;
						delete	[]buf4;
						delete	[]buf5;
						delete	[]buf6;
						delete	pBusiness;
						if ( pFund)
							delete pFund;
						//delete	[]m_buf2;
						//delete	[]m_buf3;
						//delete  []m_fbuf;	
						//delete []m_mbuf;
						//delete []m_nbuf;
						return FALSE;
					}
					nRowIndex = *(vRowIndex.begin());
					for ( int i = 0; i < nTemp; i++ )
					{
						table.GetCell(i+1,nRowIndex,dVal);
						buf1[i] = dVal;
					}
					//����ָ��
					vRowIndex.clear();
					table.Find(0,4000382,vRowIndex);
					if ( vRowIndex.size() != 1 )
					{

						delete	[]buf1;
						delete	[]buf2;
						delete	[]buf3;
						delete	[]buf4;
						delete	[]buf5;
						delete	[]buf6;
						delete	pBusiness;
						if ( pFund)
							delete pFund;
						//delete  []m_fbuf;
						//delete	[]m_buf2;
						//delete	[]m_buf3;
						//delete []m_mbuf;
						//delete []m_nbuf;
						return FALSE;
					}
					nRowIndex = *(vRowIndex.begin());
					for ( int i = 0; i < nTemp; i++ )
					{
						table.GetCell(i+1,nRowIndex,dVal);
						buf2[i] = dVal;
					}
					//���̼�ֵ
					vRowIndex.clear();
					table.Find(0,4000386,vRowIndex);
					if ( vRowIndex.size() != 1 )
					{

						delete	[]buf1;
						delete	[]buf2;
						delete	[]buf3;
						delete	[]buf4;
						delete	[]buf5;
						delete	[]buf6;

						delete	pBusiness;
						if ( pFund)
							delete pFund;
						//delete  []m_fbuf;
						//delete	[]m_buf2;
						//delete	[]m_buf3;
						//delete []m_mbuf;
						//delete []m_nbuf;
						return FALSE;
					}
					nRowIndex = *(vRowIndex.begin());
					for ( int i = 0; i < nTemp; i++ )
					{
						table.GetCell(i+1,nRowIndex,dVal);
						buf3[i] = dVal;
					}
					//С�̼�ֵ
					vRowIndex.clear();
					table.Find(0,4000390 ,vRowIndex);
					if ( vRowIndex.size() != 1 )
					{

						delete	[]buf1;
						delete	[]buf2;
						delete	[]buf3;
						delete	[]buf4;
						delete	[]buf5;
						delete	[]buf6;

						delete	pBusiness;
						if ( pFund)
							delete pFund;
						//delete  []m_fbuf;	
						//delete	[]m_buf2;
						//delete	[]m_buf3;						
						//delete []m_mbuf;
						//delete []m_nbuf;
						return FALSE;
					}
					nRowIndex = *(vRowIndex.begin());
					for ( int i = 0; i < nTemp; i++ )
					{
						table.GetCell(i+1,nRowIndex,dVal);
						buf4[i] = dVal;
					}
					//���̳ɳ�
					vRowIndex.clear();
					table.Find(0,4000385 ,vRowIndex);
					if ( vRowIndex.size() != 1 )
					{

						delete	[]buf1;
						delete	[]buf2;
						delete	[]buf3;
						delete	[]buf4;
						delete	[]buf5;
						delete	[]buf6;
						delete	pBusiness;
						if ( pFund)
							delete pFund;
						//delete  []m_fbuf;
						//delete	[]m_buf2;
						//delete	[]m_buf3;
						//delete []m_mbuf;
						//delete []m_nbuf;
						return FALSE;
					}
					nRowIndex = *(vRowIndex.begin());
					for ( int i = 0; i < nTemp; i++ )
					{
						table.GetCell(i+1,nRowIndex,dVal);
						buf5[i] = dVal;
					}
					//С�̳ɳ�
					vRowIndex.clear();
					table.Find(0,4000389 ,vRowIndex);
					if ( vRowIndex.size() != 1 )
					{

						delete	[]buf1;
						delete	[]buf2;
						delete	[]buf3;
						delete	[]buf4;
						delete	[]buf5;
						delete	[]buf6;
						delete	pBusiness;
						if ( pFund)
							delete pFund;
						//delete  []m_fbuf;
						//delete	[]m_buf2;
						//delete	[]m_buf3;
						//delete []m_mbuf;
						//delete []m_nbuf;
						return FALSE;
					}
					nRowIndex = *(vRowIndex.begin());
					for ( int i = 0; i < nTemp; i++ )
					{
						table.GetCell(i+1,nRowIndex,dVal);
						buf6[i] = dVal;
					}
					for ( int i = 0; i < nTemp; i++ )
					{
						m_buf2[i] = buf1[i] - buf2[i];
						m_buf3[i] = buf3[i] + buf4[i] - buf5[i] - buf6[i];
					}
					delete	[]buf1;
					delete	[]buf2;
					delete	[]buf3;
					delete	[]buf4;
					delete	[]buf5;
					delete	[]buf6;
					delete	pBusiness;
				}
				//��ȡ����ʱ��  
				if ( m_nType == JJ_PJ_JY_ZXG )
				{
					;
				}
				
				//2013-02-27  �ָ�m_mbuf��m_nbuf������  ���mantis:14631
				if (bRecover)
				{
					memset(m_mbuf,0,nTemp*sizeof(double));
					memset(m_nbuf,0,nTemp*sizeof(double));
					for (int i=0;i<nTemp;i++)
					{
						m_mbuf[i] = mbuf[i];
						m_nbuf[i] = nbuf[i];
					}
					bRecover = false;
				}
				int nTemp2 = nTemp;

				//Bug:13995  / wangzf / 2012-12-18	

				Tx::Data::FundNewInfo* pData = pFund->m_pSecurity->GetFundNewInfo();
				if ( pData != NULL && iStartDate < pData->setup_date)
				{
					//bug:14989  2013-07-16
					//iDates.clear();
					//iStartDate = pData->setup_date;
					//if(iStartDate >= iEndDate) continue;
					//shBusiness.m_pShIndex->GetDate( iStartDate, iEndDate, nCycle, iDates, 0 );
					std::vector<int> iTempDates;
					if(pData->setup_date >= iEndDate) continue;
					shBusiness.m_pShIndex->GetDate( pData->setup_date, iEndDate, nCycle, iTempDates, 0 );
					int nTemp1 = (int)iTempDates.size() - 1;
					if (nTemp1 <= 0) continue;

					double* buf1 = new double[nTemp1];
					double* buf2 = new double[nTemp1];
					double* buf3 = new double[nTemp1];
					double* buf4 = new double[nTemp1];
					double* buf5 = new double[nTemp1];
					double* buf6 = new double[nTemp1];
					//���ݽ�ȡ��ʼλ��
					int nPos = nTemp-nTemp1; 
					for (int i=0;i<nTemp1;i++)
					{
						buf1[i] = m_mbuf[nPos+i];
						buf2[i] = m_nbuf[nPos+i];
						buf3[i] = m_fbuf[nPos+i];
					}
					memset(m_mbuf,0,nTemp*sizeof(double));
					memset(m_nbuf,0,nTemp*sizeof(double));
					memset(m_fbuf,0,nTemp*sizeof(double));
					for (int i=0;i<nTemp1;i++)
					{
						m_mbuf[i] = buf1[i];
						m_nbuf[i] = buf2[i];
						m_fbuf[i] = buf3[i];
					}
					if ( m_nType == JJ_PJ_GII || m_nType == JJ_PJ_GII3 )
					{
						for (int i=0;i<nTemp1;i++)
						{
							buf4[i] = m_buf1[nPos+i];
						}
						memset(m_buf1,0,nTemp*sizeof(double));
						for (int i=0;i<nTemp1;i++)
						{
							m_buf1[i] = buf4[i];
						}
					}
					if ( m_nType == JJ_PJ_GII3 || m_nType == JJ_PJ_TM3 || m_nType ==JJ_PJ_HM3 || m_nType == JJ_PJ_CL3 || m_nType == JJ_PJ_FAMA )
					{
						for (int i=0;i<nTemp1;i++)
						{
							buf5[i] = m_buf2[nPos+i];
							buf6[i] = m_buf3[nPos+i];
						}
						memset(m_buf2,0,nTemp*sizeof(double));
						memset(m_buf3,0,nTemp*sizeof(double));
						for (int i=0;i<nTemp1;i++)
						{
							m_buf2[i] = buf5[i];
							m_buf3[i] = buf6[i];
						}
					}
					delete []buf1;
					delete []buf2;
					delete []buf3;
					delete []buf4;
					delete []buf5;
					delete []buf6;

					nTemp = nTemp1;
					bRecover = true;
				}
				//**************************************
				m_item_count = nTemp;
				nTemp = nTemp2;
				switch( m_nType )
				{
				case JJ_PJ_RISK1:		//ҵ�����շ���
					On_Calc_Risk1 ();
					break;
				case JJ_PJ_RISK2:		//���յ�����������
					On_Calc_Risk2 ();
					break;
				case JJ_PJ_TM:			//T-M������ģ��
					On_Calc_TM ();
					break;
				case JJ_PJ_HM:			//H-M������ģ��
					On_Calc_HM ();
					break;
				case JJ_PJ_CL:			//C-L������ģ��
					On_Calc_CL ();
					break;
				case JJ_PJ_GII:			//GIIģ��
					On_Calc_GII ();
					break;
				case JJ_PJ_TM3:			//T-M������ģ��
					On_Calc_TM3 ();
					break;			
				case JJ_PJ_HM3:			//H-M������ģ��
					On_Calc_HM3 ();	
					break;			
				case JJ_PJ_CL3:			//C-L������ģ��
					On_Calc_CL3 ();
					break;				
				case JJ_PJ_GII3:		//GII������ģ��
					On_Calc_GII3 ();
					break;				
				case JJ_PJ_FAMA:		//FAMA�ֽ�
					On_Calc_FAMA ();
					break;
				case JJ_PJ_JY_ZXG:		//�����ϵ������(���ڳ�����)
					On_Calc_JY_ZXG ();
					break;
				default: 
					break;
				}
				nTh++;
				prw.SetPercent( progId, double( nTh) / nTotal );	
			}
			prw.SetPercent( progId, 1.0 );
			delete []m_fbuf;
			m_fbuf = NULL;
			delete []m_nbuf;
			m_nbuf = NULL;
			delete []m_fbuf;
			m_fbuf = NULL;
			delete m_buf1;
			m_buf1 = NULL;
			delete m_buf2;
			m_buf2 = NULL;
			delete m_buf3;
			m_buf3 = NULL;
			delete []mbuf;
			delete []nbuf;
			//delete m_fbuf;
			//m_fbuf = NULL;
			if ( pFund)
				delete pFund;
			m_Table.Sort( 2 );
			return TRUE;
		}
		
		// ����ҵ�����շ�����ʼ������
		void StatRisk::Init_Risk1_Data ()
		{
			//�����ʼ��
			m_Table.Clear();
			m_Table.AddCol(Tx::Core::dtype_val_string);
			m_Table.AddCol(Tx::Core::dtype_val_string);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.SetTitle( 0, _T("����"));
			m_Table.SetTitle( 1, _T("����"));
			m_Table.SetTitle( 2, _T("����ƽ��������(%)"));
			m_Table.SetTitle( 3, _T("����ƽ��������(%)"));			
			m_Table.SetTitle( 4, _T("����ƽ����������(%)"));			
			m_Table.SetTitle( 5, _T("����ƽ����������(%)"));
			m_Table.SetTitle( 6, _T("�����ʱ�׼��"));
			m_Table.SetTitle( 7, _T("�������£��룩��׼��"));
			m_Table.SetTitle( 8, _T("��ϵͳ����"));
			m_Table.SetTitle( 9, _T("����ϵ��"));
			m_Table.SetTitle( 10, _T("�������"));
			m_Table.InsertCol(0,Tx::Core::dtype_int4 );
			m_Table.SetPrecise( 7, 4 );
			m_Table.SetPrecise( 8, 4 );
			m_Table.SetPrecise( 9, 4 );
			m_Table.SetPrecise( 10, 4 );
			m_Table.SetPrecise( 11, 4 );
		}

		//����ҵ�����շ�������
		void StatRisk::On_Calc_Risk1 ()
		{
			m_Result.clear();
			//������һ�����������ʻ�ȡ����������ȫ��Ϊ�㣬����ʾ������
			double	dSum = 0.0;
			for ( int i = 0; i < m_item_count; i++ )
			{
				if ( m_fbuf[i] == Con_doubleInvalid )
					continue;
				else
					dSum += m_fbuf[i];
			}
			if ( dSum==0.0 )
			{
				for( int n = 3; n < 12; n++)
				{
					m_Table.SetCell( n, m_Table.GetRowCount()-1, Con_doubleInvalid );
				}
				return;
			}	
			//2������ƽ��������
			double dValue = m_pMath->AveReturn( m_item_count, m_fbuf );
			m_Result.push_back ( dValue );
			//3������ƽ��������
			dValue = m_pMath->GeoAveReturn( m_item_count, m_fbuf );
			m_Result.push_back ( dValue );
			//4������ƽ����������
			dValue = m_pMath->AveRiskReturn( m_item_count, m_fbuf, m_nbuf );
			if ( dValue == Con_doubleInvalid )
				m_Result.push_back ( dValue );
			else
				m_Result.push_back ( 100 * dValue );
			//5������ƽ����������
			dValue = m_pMath->GeoAveRiskReturn( m_item_count, m_fbuf, m_nbuf );
			if ( dValue == Con_doubleInvalid )
				m_Result.push_back ( dValue );
			else
				m_Result.push_back ( 100 * dValue );
			//6�������ʱ�׼��
			dValue = m_pMath->Std( m_item_count, m_fbuf );
			m_Result.push_back ( dValue );
			//7���������£��룩��׼��
			dValue = m_pMath->BelowStd( m_item_count, m_fbuf );
			m_Result.push_back ( dValue );
			//8����ϵͳ����
			double beta = m_pMath->Beta( m_item_count, m_fbuf, m_nbuf, m_mbuf );
			double alpha = m_pMath->Alpha( m_item_count, beta, m_fbuf, m_nbuf, m_mbuf );
			dValue = m_pMath->NoSystemRisk( m_item_count,beta,alpha, m_fbuf, m_mbuf );
			if (dValue== 0)
				dValue = Con_doubleInvalid;
			m_Result.push_back ( dValue );
			//9������ϵ��
			dValue = m_pMath->DecisionCoeff( m_item_count, m_fbuf, m_mbuf );
			m_Result.push_back ( dValue );
			//10���������
			dValue = m_pMath->TrackError( m_item_count, m_fbuf, m_mbuf );
			m_Result.push_back ( dValue );
			//�������н�����뵽����
			//m_Table.AddRow();
			CString str;			
			double dTmp = m_Result[0];
			if( dTmp != Con_doubleInvalid )
				dTmp  *= 100;
			m_Table.SetCell( 3, m_Table.GetRowCount()-1, dTmp );	

			dTmp = m_Result[1];
			if ( dTmp != Con_doubleInvalid )
				dTmp *= 100;
			m_Table.SetCell( 4, m_Table.GetRowCount()-1, dTmp );

			dTmp = m_Result[2];
			m_Table.SetCell( 5, m_Table.GetRowCount()-1, dTmp );
			
			dTmp = m_Result[3];
			m_Table.SetCell( 6, m_Table.GetRowCount()-1, dTmp );
			
			dTmp = m_Result[4];
			m_Table.SetCell( 7, m_Table.GetRowCount()-1, dTmp );
			
			dTmp = m_Result[5];
			m_Table.SetCell( 8, m_Table.GetRowCount()-1, dTmp );
			
			dTmp = m_Result[6];
			m_Table.SetCell( 9, m_Table.GetRowCount()-1, dTmp );
			
			dTmp = m_Result[7];
			m_Table.SetCell( 10, m_Table.GetRowCount()-1, dTmp );
			
			dTmp = m_Result[8];
			m_Table.SetCell( 11, m_Table.GetRowCount()-1, dTmp );
		}
		
		// ���յ����������۳�ʼ������
		void	StatRisk::Init_Risk2_Data()
		{
			m_Table.Clear();
			m_Table.AddCol(Tx::Core::dtype_val_string);
			m_Table.AddCol(Tx::Core::dtype_val_string);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.SetTitle( 0, _T("����"));
			m_Table.SetTitle( 1, _T("����"));
			m_Table.SetTitle( 2, _T("Betaϵ��"));
			m_Table.SetTitle( 3, _T("Treynorϵ��"));
			m_Table.SetTitle( 4, _T("Sharpeϵ��"));
			m_Table.SetTitle( 5, _T("Jensenϵ��"));
			m_Table.SetTitle( 6, _T("��Ϣ���ʣ����۱��ʣ�")) ;
			m_Table.SetTitle( 7, _T("Mƽ�����"));
			m_Table.SetTitle( 8, _T("���յ�������Cֵ"));
			m_Table.InsertCol(0,Tx::Core::dtype_int4 );
			m_Table.SetPrecise( 3, 4);
			m_Table.SetPrecise( 4, 8);
			m_Table.SetPrecise( 5, 4);
			m_Table.SetPrecise( 6, 8);
			m_Table.SetPrecise( 7, 4);
			m_Table.SetPrecise( 8, 8);
			m_Table.SetPrecise( 9, 4);
		}
		
		//���յ����������ۼ���
		void	StatRisk::On_Calc_Risk2()
		{
			m_Result.clear();
			//������һ�����������ʻ�ȡ����������ȫ��Ϊ�㣬����ʾ������
			double	dSum = 0.0;
			for ( int i = 0; i < m_item_count; i++ )
			{
				dSum += m_fbuf[i];
			}
			if ( dSum==0.0 )
			{
				for( int n = 3; n < 10; n++)
				{
					m_Table.SetCell( n, m_Table.GetRowCount()-1, Con_doubleInvalid );
				}
				return;
			}
			//Beta
			double dValue = m_pMath->Beta( m_item_count, m_fbuf, m_nbuf, m_mbuf );
			double beta = dValue;
			m_Result.push_back( dValue );
			//Treynor
			dValue = m_pMath->Treynor( m_item_count, beta, m_fbuf, m_nbuf );
			m_Result.push_back( dValue );
			// Sharpeϵ��
			double std = m_pMath->Std( m_item_count, m_fbuf );
			dValue = m_pMath->Sharpe( m_item_count, std, m_fbuf, m_nbuf );
			m_Result.push_back( dValue );
			// Jensenϵ��
			dValue = m_pMath->Jensen( m_item_count, beta, m_fbuf, m_nbuf, m_mbuf );
			m_Result.push_back( dValue );
			// ��Ϣ���ʣ����۱��ʣ�
			double trackerror = m_pMath->TrackError( m_item_count, m_fbuf, m_mbuf );
			dValue = m_pMath->IFRatio( m_item_count, trackerror, m_fbuf, m_mbuf );
			m_Result.push_back( dValue );
			// Mƽ�����
			dValue = m_pMath->MSquare( m_item_count, m_fbuf, m_nbuf, m_mbuf );
			m_Result.push_back( dValue );
			// ���յ�������Cֵ
			dValue = m_pMath->CValue( m_item_count, m_fbuf, m_nbuf, m_mbuf );
			m_Result.push_back( dValue );		
			//�������н�����뵽����
			//m_Table.AddRow();
			for( int n = 3; n < 10; n++)
			{
				double dTmp = m_Result[n-3];
				m_Table.SetCell( n, m_Table.GetRowCount()-1, dTmp );
			}
		}
		
		// T-M������ģ�ͽ����ʼ��
		void	StatRisk::Init_TM_Data()
		{
			m_Table.Clear();
			m_Table.AddCol(Tx::Core::dtype_val_string);
			m_Table.AddCol(Tx::Core::dtype_val_string);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.SetTitle( 0, _T("����"));
			m_Table.SetTitle( 1, _T("����"));
			m_Table.SetTitle( 2, _T("Alphaֵ"));
			m_Table.SetTitle( 3, _T("Beta1ϵ��"));			
			m_Table.SetTitle( 4, _T("Beta2ϵ��"));			
			m_Table.SetTitle( 5, _T("Alpha�ı�׼��"));
			m_Table.SetTitle( 6, _T("Beta1�ı�׼��"));
			m_Table.SetTitle( 7, _T("Beta2�ı�׼��"));
			m_Table.SetTitle( 8, _T("Alpha��t����ֵ"));
			m_Table.SetTitle( 9, _T("Beta1��t����ֵ"));
			m_Table.SetTitle( 10, _T("Beta2��t����ֵ"));
			m_Table.SetTitle( 11, _T("�����ж�ϵ��"));
			m_Table.InsertCol(0,Tx::Core::dtype_int4 );
			m_Table.SetPrecise( 3, 8);
			m_Table.SetPrecise( 4, 4);
			m_Table.SetPrecise( 5, 4);
			m_Table.SetPrecise( 6, 8);
			m_Table.SetPrecise( 7, 4);
			m_Table.SetPrecise( 8, 4);
			m_Table.SetPrecise( 9, 4);
			m_Table.SetPrecise( 10, 4);
			m_Table.SetPrecise( 11, 4);
			m_Table.SetPrecise( 12, 4);
				
		}
		//T-M������ģ�ͼ���
		void	StatRisk::On_Calc_TM ()
		{
			//������һ�����������ʻ�ȡ����������ȫ��Ϊ�㣬����ʾ������
			double	dSum = 0.0;
			for ( int i = 0; i < m_item_count; i++ )
			{
				//if ( (fabs(m_fbuf[i]) - Con_doubleInvalid ) < 0.000001)
				if ( m_fbuf[i] == Con_doubleInvalid )
					continue;
				else
					dSum += m_fbuf[i];
			}
			if ( dSum==0.0 )
			{
				for( int n = 3; n < 13; n++)
				{
					m_Table.SetCell( n, m_Table.GetRowCount()-1, Con_doubleInvalid );
				}
				return;
			}
			//modified by zhangxs 20081225 ��TM������Ϊbool�ͣ�����ѡ��������������С��ָ��ֵ��û�ж����ݴ��������
			if(!m_pMath->TM( m_item_count, m_fbuf, m_nbuf, m_mbuf ))
			{
				for( int n = 3; n < 13; n++ )
				{
					m_Table.SetCell( n, m_Table.GetRowCount()-1, Con_doubleInvalid );
				}
			}
			//��m_pMath->m_ResultData�е����ݶ���table
			//m_Table.AddRow();
			else
			{
				for( int n = 3; n < 13; n++ )
				{
					double	dTemp = m_pMath->m_ResultData[n-3];
					m_Table.SetCell( n, m_Table.GetRowCount()-1, dTemp );
				}
			}
		}

		//H-M������ģ�ͳ�ʼ��
		void	StatRisk::Init_HM_Data()
		{
			Init_TM_Data ();
		}

		//H-M������ģ�ͼ���
		void	StatRisk::On_Calc_HM()
		{
			//������һ�����������ʻ�ȡ����������ȫ��Ϊ�㣬����ʾ������
			double	dSum = 0.0;
			for ( int i = 0; i < m_item_count; i++ )
			{
				if ( m_fbuf[i] == Con_doubleInvalid )
					continue;
				else
					dSum += m_fbuf[i];
			}
			if ( dSum==0.0 )
			{
				for( int n = 3; n < 13; n++)
				{
					m_Table.SetCell( n, m_Table.GetRowCount()-1, Con_doubleInvalid );
				}
				return;
			}
			//modified by zhangxs 20081225 ��HM������Ϊbool�ͣ�����ѡ��������������С��ָ��ֵ��û�ж����ݴ��������
			if(!m_pMath->HM( m_item_count, m_fbuf, m_nbuf, m_mbuf ))
			{
				for( int n = 3; n < 13; n++ )
				{
					m_Table.SetCell( n, m_Table.GetRowCount()-1, Con_doubleInvalid );
				}
			}
			//��m_pMath->m_ResultData�е����ݶ���table
			//m_Table.AddRow();
			else
			{
				for( int n = 3; n < 13; n++ )
				{
					double dTemp = m_pMath->m_ResultData[n-3];
					m_Table.SetCell( n, m_Table.GetRowCount()-1, dTemp );
				}
			}
		}
		
		// C-L������ģ�ͽ����ʼ��
		void	StatRisk::Init_CL_Data()
		{
			Init_TM_Data ();
		}

		// C-L������ģ�ͼ���
		void	StatRisk::On_Calc_CL ()
		{
			//������һ�����������ʻ�ȡ����������ȫ��Ϊ�㣬����ʾ������
			double	dSum = 0.0;
			for ( int i = 0; i < m_item_count; i++ )
			{
				if ( m_fbuf[i] == Con_doubleInvalid )
					continue;
				else
					dSum += m_fbuf[i];
			}
			if ( dSum==0.0 )
			{
				for( int n = 3; n < 13; n++)
				{
					m_Table.SetCell( n, m_Table.GetRowCount()-1, Con_doubleInvalid );
				}
				return;
			}
			//modified by zhangxs 20081225 ��CL������Ϊbool�ͣ�����ѡ��������������С��ָ��ֵ��û�ж����ݴ��������
			if(!m_pMath->CL( m_item_count, m_fbuf, m_nbuf, m_mbuf ))
			{
				for( int n = 3; n < 13; n++ )
				{
					m_Table.SetCell( n, m_Table.GetRowCount()-1, Con_doubleInvalid );
				}
			}
			//��m_pMath->m_ResultData�е����ݶ���table
			//m_Table.AddRow();
			else
			{
				for( int n = 3; n < 13; n++ )
				{
					double dTemp = m_pMath->m_ResultData[n-3];
					m_Table.SetCell( n, m_Table.GetRowCount()-1, dTemp );
				}
			}
		}
			
		//GIIģ�ͽ����ʼ��
		void	StatRisk::Init_GII_Data()
		{
			Init_TM_Data ();
		}

		//GIIģ�ͼ���
		void	StatRisk::On_Calc_GII()
		{	
			double	dSum = 0.0;
			for ( int i = 0; i < m_item_count; i++ )
			{
				if ( m_fbuf[i] == Con_doubleInvalid )
					continue;
				else
					dSum += m_fbuf[i];
			}
			if ( dSum==0.0 )
			{
				for( int n = 3; n < 13; n++)
				{
					m_Table.SetCell( n, m_Table.GetRowCount()-1, Con_doubleInvalid );
				}
				return;
			}
			//m_buf1����ÿ���г�ʱ�����մ����ĵ�λ�����ʲ�����ֵ
			//modified by zhangxs 20081225 ��GII������Ϊbool�ͣ�����ѡ��������������С��ָ��ֵ��û�ж����ݴ��������
			if(!m_pMath->GII( m_item_count, m_fbuf, m_nbuf, m_mbuf, m_buf1 ))
			{
				for( int n = 3; n < 13; n++ )
					m_Table.SetCell( n, m_Table.GetRowCount()-1, Con_doubleInvalid );
			}
			//��m_pMath->m_ResultData�е����ݶ���table
			//m_Table.AddRow();
			else
			{
				for( int n = 3; n < 13; n++ )
				{
					double dTemp = m_pMath->m_ResultData[n-3];
					m_Table.SetCell( n, m_Table.GetRowCount()-1, dTemp );
				}
			}
		}

		//T-M������ģ�ͽ����ʼ��
		void	StatRisk::Init_TM3_Data()
		{
			m_Table.Clear();
			m_Table.AddCol(Tx::Core::dtype_val_string);
			m_Table.AddCol(Tx::Core::dtype_val_string);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);		
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.SetTitle( 0, _T("����"));
			m_Table.SetTitle( 1, _T("����"));
			m_Table.SetTitle( 2, _T("Alphaֵ"));
			m_Table.SetTitle( 3, _T("Beta1ϵ��"));			
			m_Table.SetTitle( 4, _T("Beta2ϵ��"));			
			m_Table.SetTitle( 5, _T("Sϵ��"));
			m_Table.SetTitle( 6, _T("Hϵ��"));
			m_Table.SetTitle( 7, _T("Alpha�ı�׼��"));
			m_Table.SetTitle( 8, _T("Beta1�ı�׼��"));
			m_Table.SetTitle( 9, _T("Beta2�ı�׼��"));
			m_Table.SetTitle( 10, _T("S�ı�׼��"));
			m_Table.SetTitle( 11, _T("H�ı�׼��"));
			m_Table.SetTitle( 12, _T("Alpha��t����ֵ"));
			m_Table.SetTitle( 13, _T("Beta1��t����ֵ"));
			m_Table.SetTitle( 14, _T("Beta2��t����ֵ"));
			m_Table.SetTitle( 15, _T("S��t����ֵ"));
			m_Table.SetTitle( 16, _T("H��t����ֵ"));
			m_Table.SetTitle( 17, _T("�����ж�ϵ��R��ƽ��"));
			m_Table.InsertCol(0,Tx::Core::dtype_int4 );
			m_Table.SetPrecise( 3, 8);
			m_Table.SetPrecise( 4, 4);
			m_Table.SetPrecise( 5, 4);
			m_Table.SetPrecise( 6, 4);
			m_Table.SetPrecise( 7, 4);
			m_Table.SetPrecise( 8, 8);
			m_Table.SetPrecise( 9, 4);
			m_Table.SetPrecise( 10, 4);
			m_Table.SetPrecise( 11, 4);
			m_Table.SetPrecise( 12, 4);
			m_Table.SetPrecise( 13, 4);
			m_Table.SetPrecise( 14, 4);
			m_Table.SetPrecise( 15, 4);
			m_Table.SetPrecise( 16, 4);
			m_Table.SetPrecise( 17, 4);
			m_Table.SetPrecise( 18, 4);
		}

		//T-M������ģ�ͼ���
		void	StatRisk::On_Calc_TM3()
		{
			double	dSum = 0.0;
			for ( int i = 0; i < m_item_count; i++ )
			{
				if ( m_fbuf[i] == Con_doubleInvalid )
					continue;
				else
					dSum += m_fbuf[i];
			}
			if ( dSum==0.0 )
			{
				for( int n = 3; n < 19; n++)
				{
					m_Table.SetCell( n, m_Table.GetRowCount()-1, Con_doubleInvalid );
				}
				return;
			}
			/*	buf2 ���ָ���У�С��ָ�������ʣ�����ָ��������	*/
			/*	buf3 �����̼�ֵ�����ʣ�С�̼�ֵ�����ʣ� �������̳ɳ������ʣ�С�̳ɳ������ʣ�	*/
			//modified by zhangxs 20081225 ��TM_3Factor������Ϊbool�ͣ�����ѡ��������������С��ָ��ֵ��û�ж����ݴ��������
			if(!m_pMath->TM_3Factor( m_item_count, m_fbuf, m_nbuf, m_mbuf, m_buf2, m_buf3 ))
			{
				for( int n = 3; n < 19; n++ )
					m_Table.SetCell( n, m_Table.GetRowCount()-1, Con_doubleInvalid );
			}
			//��m_pMath->m_ResultData�е����ݶ���table
			//m_Table.AddRow();
			else
			{
				for( int n = 3; n < 19; n++ )
				{
					double dTemp = m_pMath->m_ResultData[n-3];
					m_Table.SetCell( n, m_Table.GetRowCount()-1, dTemp );
				}
			}
		}

		//H-M������ģ��
		void	StatRisk::Init_HM3_Data()
		{
			Init_TM3_Data ();
		}
		void	StatRisk::On_Calc_HM3()
		{
			double	dSum = 0.0;
			for ( int i = 0; i < m_item_count; i++ )
			{
				if ( m_fbuf[i] == Con_doubleInvalid )
					continue;
				else
					dSum += m_fbuf[i];
			}
			if ( dSum==0.0 )
			{
				for( int n = 3; n < 19; n++)
				{
					m_Table.SetCell( n, m_Table.GetRowCount()-1, Con_doubleInvalid );
				}
				return;
			}
			/*	buf2 ���ָ���У�С��ָ�������ʣ�����ָ��������	*/
			/*	buf3 �����̼�ֵ�����ʣ�С�̼�ֵ�����ʣ� �������̳ɳ������ʣ�С�̳ɳ������ʣ�	*/
			//modified by zhangxs 20081225 ��HM_3Factor������Ϊbool�ͣ�����ѡ��������������С��ָ��ֵ��û�ж����ݴ��������
			if(!m_pMath->HM_3Factor( m_item_count, m_fbuf, m_nbuf, m_mbuf, m_buf2, m_buf3 ))
			{
				for( int n = 3; n < 19; n++ )
					m_Table.SetCell( n, m_Table.GetRowCount()-1, Con_doubleInvalid );
			}
			//��m_pMath->m_ResultData�е����ݶ���table	
			//m_Table.AddRow();
			else
			{
				for( int n = 3; n < 19; n++ )
				{
					double dTemp = m_pMath->m_ResultData[n-3];
					m_Table.SetCell( n, m_Table.GetRowCount()-1, dTemp );
				}
			}
		}
	
		//C-L������ģ��
		void	StatRisk::Init_CL3_Data()
		{
			Init_TM3_Data ();
		}
		void	StatRisk::On_Calc_CL3()
		{
			double	dSum = 0.0;
			for ( int i = 0; i < m_item_count; i++ )
			{
				if ( m_fbuf[i] == Con_doubleInvalid )
					continue;
				else
					dSum += m_fbuf[i];
			}
			if ( dSum==0.0 )
			{
				for( int n = 3; n < 19; n++)
				{
					m_Table.SetCell( n, m_Table.GetRowCount()-1, Con_doubleInvalid );
				}
				return;
			}
			/*	buf2 ���ָ���У�С��ָ�������ʣ�����ָ��������	*/
			/*	buf3 �����̼�ֵ�����ʣ�С�̼�ֵ�����ʣ� �������̳ɳ������ʣ�С�̳ɳ������ʣ�	*/
			//modified by zhangxs 20081225 ��CL_3Factor������Ϊbool�ͣ�����ѡ��������������С��ָ��ֵ��û�ж����ݴ��������
			if(!m_pMath->CL_3Factor( m_item_count, m_fbuf, m_nbuf, m_mbuf, m_buf2, m_buf3 ))
			{
				for( int n = 3; n < 19; n++ )
					m_Table.SetCell( n, m_Table.GetRowCount()-1, Con_doubleInvalid );
			}
			//��m_pMath->m_ResultData�е����ݶ���table			
			//m_Table.AddRow();
			else
			{
				for( int n = 3; n < 19; n++ )
				{
					double dTemp = m_pMath->m_ResultData[n-3];
					m_Table.SetCell( n, m_Table.GetRowCount()-1, dTemp );
				}
			}
		}
		
		//GII������ģ��
		void	StatRisk::Init_GII3_Data()
		{
			Init_TM3_Data ();
		}
		void	StatRisk::On_Calc_GII3()
		{
			double	dSum = 0.0;
			for ( int i = 0; i < m_item_count; i++ )
			{
				if ( m_fbuf[i] == Con_doubleInvalid )
					continue;
				else
					dSum += m_fbuf[i];
			}
			if ( dSum==0.0 )
			{
				for( int n = 3; n < 19; n++)
				{
					m_Table.SetCell( n, m_Table.GetRowCount()-1, Con_doubleInvalid );
				}
				return;
			}
			//buf1 ���ָ���У�С��ָ�������ʣ�����ָ��������
			//buf2 �����̼�ֵ�����ʣ�С�̼�ֵ�����ʣ� �������̳ɳ������ʣ�С�̳ɳ������ʣ�
			//buf3	����ÿ���г�ʱ�����մ����ĵ�λ�����ʲ�����ֵ
			//modified by zhangxs 20081225 ��GII_3Factor������Ϊbool�ͣ�����ѡ��������������С��ָ��ֵ��û�ж����ݴ��������
			if(!m_pMath->GII_3Factor( m_item_count, m_fbuf, m_nbuf, m_mbuf, m_buf1, m_buf2, m_buf3 ))
			{
				for( int n = 3; n < 19; n++ )
				{
					m_Table.SetCell( n, m_Table.GetRowCount()-1, Con_doubleInvalid );
				}
			}
			//��m_pMath->m_ResultData�е����ݶ���table		
			//m_Table.AddRow();
			else
			{
				for( int n = 3; n < 19; n++ )
				{
					double dTemp = m_pMath->m_ResultData[n-3];
					m_Table.SetCell( n, m_Table.GetRowCount()-1, dTemp );
				}
			}
		}

		// FAMA�ֽ�
		void	StatRisk::Init_FAMA_Data()
		{
			m_Table.Clear();
			m_Table.AddCol(Tx::Core::dtype_val_string);
			m_Table.AddCol(Tx::Core::dtype_val_string);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);	
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.SetTitle( 0, _T("����"));
			m_Table.SetTitle( 1, _T("����"));
			m_Table.SetTitle( 2, _T("Alphaֵ"));
			m_Table.SetTitle( 3, _T("Beta1ϵ��"));			
			m_Table.SetTitle( 4, _T("Sϵ��"));
			m_Table.SetTitle( 5, _T("Hϵ��"));
			m_Table.SetTitle( 6, _T("Alpha�ı�׼��"));
			m_Table.SetTitle( 7, _T("Beta1�ı�׼��"));
			m_Table.SetTitle( 8, _T("S�ı�׼��"));
			m_Table.SetTitle( 9, _T("H�ı�׼��"));
			m_Table.SetTitle( 10, _T("Alpha��t����ֵ"));
			m_Table.SetTitle( 11, _T("Beta1��t����ֵ"));
			m_Table.SetTitle( 12, _T("S��t����ֵ"));
			m_Table.SetTitle( 13, _T("H��t����ֵ"));
			m_Table.SetTitle( 14, _T("�����ж�ϵ��R��ƽ��"));
			m_Table.InsertCol(0,Tx::Core::dtype_int4 );
			m_Table.SetPrecise( 3, 8);
			m_Table.SetPrecise( 4, 4);
			m_Table.SetPrecise( 5, 4);
			m_Table.SetPrecise( 6, 4);
			m_Table.SetPrecise( 7, 8);
			m_Table.SetPrecise( 8, 4);
			m_Table.SetPrecise( 9, 4);
			m_Table.SetPrecise( 10, 4);
			m_Table.SetPrecise( 11, 4);
			m_Table.SetPrecise( 12, 4);
			m_Table.SetPrecise( 13, 4);
			m_Table.SetPrecise( 14, 4);
			m_Table.SetPrecise( 15, 4);
		}
		void	StatRisk::On_Calc_FAMA()
		{
			double	dSum = 0.0;
			for ( int i = 0; i < m_item_count; i++ )
			{
				if ( m_fbuf[i] == Con_doubleInvalid )
					continue;
				else
					dSum += m_fbuf[i];
			}
			if ( dSum==0.0 )
			{
				for( int n = 3; n < 16; n++)
				{
					m_Table.SetCell( n, m_Table.GetRowCount()-1, Con_doubleInvalid );
				}
				return;
			}
			//buf2 ���ָ���У�С��ָ�������ʣ�����ָ��������
			//buf3 �����̼�ֵ�����ʣ�С�̼�ֵ�����ʣ� �������̳ɳ������ʣ�С�̳ɳ������ʣ�
			//modified by zhangxs 20081225 ��FAMA������Ϊbool�ͣ�����ѡ��������������С��ָ��ֵ��û�ж����ݴ��������
			if(!m_pMath->FAMA( m_item_count, m_fbuf, m_nbuf, m_mbuf, m_buf2, m_buf3 ))
			{
				for( int n = 3; n < 16; n++ )
				{
					m_Table.SetCell( n, m_Table.GetRowCount()-1, Con_doubleInvalid );
				}
			}
			//��m_pMath->m_ResultData�е����ݶ���table	
			//m_Table.AddRow();
			else
			{
				for( int n = 3; n < 16; n++ )
				{
					double dTemp = m_pMath->m_ResultData[n-3];
					m_Table.SetCell( n, m_Table.GetRowCount()-1, dTemp );
				}
			}
		}

		//�����ϵ������(���ڳ�����)�����ʼ��
		void	StatRisk::Init_JY_ZXG()
		{
			m_Table.Clear();

			m_Table.AddCol(Tx::Core::dtype_val_string);
			m_Table.AddCol(Tx::Core::dtype_val_string);
			m_Table.SetTitle( 0, _T("����"));
			m_Table.SetTitle( 1, _T("����"));
			CString str;
			for ( int i = 1; i <= m_LagPeriod; i++ )
			{
				m_Table.AddCol(Tx::Core::dtype_double );
				str.Format("�ͺ�%d�������ϵ��",i );
				m_Table.SetTitle( 1+i,str ); 
			}
			m_Table.AddCol(Tx::Core::dtype_double );
			m_Table.SetTitle( m_LagPeriod+2, _T("Qͳ����"));
			//m_Table.SetTitle( 2, _T("�ͺ�1�������ϵ��"));
			//m_Table.SetTitle( 3, _T("�ͺ�2�������ϵ��"));
			//m_Table.SetTitle( 4, _T("�ͺ�3�������ϵ��"));
			//m_Table.SetTitle( 5, _T("�ͺ�4�������ϵ��"));
			//m_Table.SetTitle( 6, _T("Qͳ����"));
			m_Table.InsertCol(0,Tx::Core::dtype_int4 );
			for( int i = 3; i < (int)m_Table.GetColCount(); i++)
				m_Table.SetPrecise( i, 4);
		}

		//�����ϵ������(���ڳ�����)����
		void	StatRisk::On_Calc_JY_ZXG()
		{
			double	dSum = 0.0;
			for ( int i = 0; i < m_item_count; i++ )
			{
				if ( m_fbuf[i] == Con_doubleInvalid )
					continue;
				else
					dSum += m_fbuf[i];
			}
			if ( dSum==0.0 )
			{
				for( int n = 3; n < m_LagPeriod+4; n++)
				{
					m_Table.SetCell( n, m_Table.GetRowCount()-1, Con_doubleInvalid );
				}
				return;
			}
			//modified by zhangxs 20081225 ��JY_ZXG������Ϊbool�ͣ�����ѡ��������������С��ָ��ֵ��û�ж����ݴ��������
			if(!m_pMath->JY_ZXG( m_item_count, m_fbuf, m_nbuf, m_mbuf, m_LagPeriod ))
			{
				for( int n = 3; n < m_LagPeriod+4; n++ )
				{
					m_Table.SetCell( n, m_Table.GetRowCount()-1, Con_doubleInvalid );
				}
			}
			//��m_pMath->m_ResultData�е����ݶ���table	
			//m_Table.AddRow();
			else
			{
				for( int n = 3; n < m_LagPeriod+4; n++ )
				{
					double dTemp = m_pMath->m_ResultData[n-3];
					m_Table.SetCell( n, m_Table.GetRowCount()-1, dTemp );
				}
			}
		}

		//��������ʼ���(���ڳ�����)�����ʼ��
		void	StatRisk::Init_JY_JBL()
		{
			m_Table.Clear();
			m_Table.AddCol(Tx::Core::dtype_val_string);
			m_Table.AddCol(Tx::Core::dtype_val_string);
			m_Table.AddCol(Tx::Core::dtype_int4);
			m_Table.AddCol(Tx::Core::dtype_int4);
			m_Table.AddCol(Tx::Core::dtype_int4);
			m_Table.AddCol(Tx::Core::dtype_int4);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.AddCol(Tx::Core::dtype_double);
			m_Table.SetTitle( 0, _T("����"));
			m_Table.SetTitle( 1, _T("����"));
			m_Table.SetTitle( 2, _T("WW(��)��"));
			m_Table.SetTitle( 3, _T("LL(��)��"));
			m_Table.SetTitle( 4, _T("WL(��)��"));
			m_Table.SetTitle( 5, _T("LW(��)��"));
			m_Table.SetTitle( 6, _T("CPRͳ����"));
			m_Table.SetTitle( 7, _T("Zͳ����"));

		}

		//��������ʼ���(���ڳ�����)����
		//
		void	StatRisk::On_Calc_JY_JBL()
		{
			Tx::Business::FundDeriveData	data;
			//���������������,���ݸ��������ڷ�ʽ���������Ҫ�Ľ��������У�����Ĭ��ȥÿ�����ڵ����һ��������
			//------------------------------------------------------------------------------------------------
			//����Ļ�׼ȡ��֤
			TxBusiness		shBusiness;
			shBusiness.m_pShIndex->LoadHisTrade();
			shBusiness.m_pShIndex->LoadTradeDate();
			switch( m_nData_type )
			{
			//��ǰ������Ϊ��������
			case 0:
				{
					int nIndex = shBusiness.m_pShIndex->GetTradeDataCount()-1;
					iEndDate = shBusiness.m_pShIndex->GetTradeDateByIndex( nIndex );
					if( nIndex > -1 )
					{
						int offset = m_item_count;
						if( m_nCycle == 1 )
						{
							iStartDate = shBusiness.m_pShIndex->GetTradeDateByIndex( max( nIndex - offset, 0 ) );
						}
						if( m_nCycle == 2 )
						{
							int nWeekIndex = shBusiness.m_pShIndex->GetTradeDateIndex( iEndDate, 0, 1 );
							iStartDate = shBusiness.m_pShIndex->GetTradeDateByIndex( max( 0, nWeekIndex - offset), 1 );	
						}
						if( m_nCycle == 3 )
						{
							int nMonthIndex = shBusiness.m_pShIndex->GetTradeDateIndex( iEndDate, 0, 2 );
							iStartDate = shBusiness.m_pShIndex->GetTradeDateByIndex( max( nMonthIndex - offset, 0),2 );	
						}
						if( m_nCycle == 4 )
						{
							int nQuarterIndex =shBusiness.m_pShIndex->GetTradeDateIndex( iEndDate, 0, 3 );
							iStartDate = shBusiness.m_pShIndex->GetTradeDateByIndex( max( nQuarterIndex - offset, 0), 3 );	
						}
						if ( m_nCycle == 5)//����
						{
							int nQuarterIndex =shBusiness.m_pShIndex->GetTradeDateIndex( iEndDate, 0, 5 );
							iStartDate = shBusiness.m_pShIndex->GetTradeDateByIndex( max( nQuarterIndex - offset, 0), 5 );	
						}
						if( m_nCycle == 6 )
						{
							int nYearIndex = shBusiness.m_pShIndex->GetTradeDateIndex( iEndDate, 0, 4 );
							iStartDate = shBusiness.m_pShIndex->GetTradeDateByIndex( max( 0, nYearIndex - offset),4 );	
						}
					}
				}
				break;
			//ָ����ʼ��ֹ
			case 1:
				{
					iStartDate = max( iStartDate, 19901219 );
					iEndDate = min( iEndDate, shBusiness.m_pShIndex->GetTradeDateLatest() );
				}
				break;
			//ָ��������Ϊ��ֹ����
			case 2:
				{
					int nIndex = shBusiness.m_pShIndex->GetTradeDateIndex( iEndDate );
					iEndDate = shBusiness.m_pShIndex->GetTradeDateByIndex( nIndex );
					if( nIndex > -1 )
					{
						int offset = m_item_count;
						if( m_nCycle == 1 )
						{
							iStartDate = shBusiness.m_pShIndex->GetTradeDateByIndex( max( nIndex - offset, 0 ) );
						}
						if( m_nCycle == 2 )
						{
							int nWeekIndex = shBusiness.m_pShIndex->GetTradeDateIndex( iEndDate, 0, 1 );
							iStartDate = shBusiness.m_pShIndex->GetTradeDateByIndex( max( 0, nWeekIndex - offset), 1 );	
						}
						if( m_nCycle == 3 )
						{
							int nMonthIndex = shBusiness.m_pShIndex->GetTradeDateIndex( iEndDate, 0, 2 );
							iStartDate = shBusiness.m_pShIndex->GetTradeDateByIndex( max( 0, nMonthIndex - offset), 2 );	
						}
						if( m_nCycle == 4 )
						{
							int nQuarterIndex =shBusiness.m_pShIndex->GetTradeDateIndex( iEndDate, 0, 3 );
							iStartDate = shBusiness.m_pShIndex->GetTradeDateByIndex( max( 0, nQuarterIndex - offset), 3 );	
						}
						if ( m_nCycle == 5)//����
						{
							int nQuarterIndex =shBusiness.m_pShIndex->GetTradeDateIndex( iEndDate, 0, 5 );
							iStartDate = shBusiness.m_pShIndex->GetTradeDateByIndex( max( nQuarterIndex - offset, 0), 5 );	
						}
						if( m_nCycle == 6 )
						{
							int nYearIndex = shBusiness.m_pShIndex->GetTradeDateIndex( iEndDate, 0, 4 );
							iStartDate = shBusiness.m_pShIndex->GetTradeDateByIndex( max( 0, nYearIndex - offset), 4 );	
						}
					}	
				}
				break;
			default:
				break;
			}
			//*****************************************************************

			//���ｫ�û����õĻ��������������ʼ���������ŵ�����
			//--------------------------------------------------------------------------
			CString sz;
			sz.Format("%s", m_AvrRetCode);
			//������Ҫȷ���Ǵ洢������
			int nCode = _ttoi( sz );	
			Core::Table	table;
			std::vector< int >	iDates;
			std::vector< int >		iVec( m_Sample.begin(), m_Sample.end() );
			iVec.push_back( nCode );
			iVec.push_back(4000384);	//С��ָ��
			iVec.push_back(4000382);	//����ָ��
			iVec.push_back(4000390);	//С�̼�ֵ
			iVec.push_back(4000386);	//���̼�ֵ
			iVec.push_back(4000389);	//С�̳ɳ�
			iVec.push_back(4000385);	//���̳ɳ�
			shBusiness.m_pShIndex->GetDate( iStartDate, iEndDate, m_nCycle-1, iDates, 0 );
			sort( iDates.begin(), iDates.end());
			std::set< std::pair< int , int > > dates;
			for( std::vector< int >::iterator iter = iDates.begin() + 1; iter != iDates.end(); iter++ )
			{
				dates.insert( pair< int, int >( *(iter-1), *iter));
			}
			if ( m_algo_type == 0 )
				data.CalFundNav_Ext( table, iVec, dates, false );
			else
				data.CalFundNav_Ext( table, iVec, dates );
			//*********************************************************************

			//��ȡ��׼�������У���ȡ��m_mbuf��,����ĳ��Ȱ���table�еĳ���			
			//-------------------------------------------------------------------------
			//�������iDate�е����ڼ����׼������������
			int nTemp = iDates.size() - 1;
			m_mbuf = new double[nTemp];
			//Tx::Data::HisTradeData	*pTradeData;
			double	dClose = 0.0;
			double	dPreClose = 0.0;
			double  dVal = 0.0;
			int nRowIndex = 0;
			std::vector<UINT> vRowIndex;
			vRowIndex.clear();
			table.Find(0,4000208,vRowIndex);
			if ( vRowIndex.size() != 1 )
			{	
				//delete []m_mbuf;
				return;
			}
			nRowIndex = *(vRowIndex.begin());
			for ( int n = 0; n < nTemp; n++ )
			{
				table.GetCell(n+1,nRowIndex,dVal);
				m_mbuf[n] = dVal;
			}
			//*********************************************************************************

			//��ȡ�޷����������У���ȡ��m_nbuf�У�
			//-----------------------------------------------------------------------------------

			double dNrisk = 0.0;
			switch(m_NoRiskRet_Type)
			{	
			case 0:
				//����
				dNrisk = 0;
				break;
			case 1:
				{
					int index = Data::BasicInterest::GetInstance()->GetIndexByDate( iStartDate, 8, 1 );
					Data::BasicInterestData*	p;
					p = Data::BasicInterest::GetInstance()->GetDataByIndex( index );
					if ( p != NULL)
					{
						dNrisk = p->dInterest;
					}
				}
				break;
			case 2:
				dNrisk = m_fUserDefineNrValue;
				break;
			default:
				break;
			}
			m_nbuf = new double[nTemp];
			for( int i = 0; i< nTemp; i++ )
			{
				m_nbuf[i] = dNrisk;
			}
			//����ѭ��
			int nLL(0), nWW(0), nLW(0) ,nWL(0) ;
			m_fbuf = new double[nTemp];			
			for ( int i = 0; i < (int)m_Sample.size(); i++ )
			{
				//ȡ�û�������������
				memset(m_fbuf,0,nTemp*sizeof(double));
				int iCode  = 1;
				table.GetCell( 0, i, iCode );
				for( int j = 0; j < nTemp; j++ )
				{
					table.GetCell( j+1, i, m_fbuf[j] );
				}
				//���õ�ֻ���𽻲�����ʼ���
				//��������Ӧ�ý�m_item_countת��ΪnTemp,��Ϊ���ܺܶ඼����Ч����
				//m_pMath->JY_JBL( m_item_count, m_fbuf, m_nbuf, m_mbuf );
				m_pMath->JY_JBL( nTemp, m_fbuf, m_nbuf, m_mbuf );
				if ( m_pMath->m_nResultData == 1 )
					nWW++;
				if ( m_pMath->m_nResultData == 2 )
					nLL++;
				if ( m_pMath->m_nResultData == 3 )
					nWL++;
				if ( m_pMath->m_nResultData == 4 )
					nLW++;
			}

			m_Table.AddRow();
			CString str;
			str = "ȫ����������";
			m_Table.SetCell( 0,  0, str );
			str.Format(_T("-"));
			m_Table.SetCell( 1, 0, str );
			m_Table.SetCell( 2, 0, nWW );
			m_Table.SetCell( 3, 0, nLL );
			m_Table.SetCell( 4, 0, nWL );
			m_Table.SetCell( 5, 0, nLW );
			
			// ����ͳ������ֵ
			double fCPR;
			double fCov;
			double fZValue;
			if ( nWW*nLL*nWL*nLW == 0)
			{
				
				//str = "-";
				//m_Table.SetCell( 6, 0, str );
				//m_Table.SetCell( 7, 0, str );
				//modified by zhangxs 20081225 6.7��Ϊdouble�������string�г�ͻ
				m_Table.SetCell( 6, 0, Con_doubleInvalid );
				m_Table.SetCell( 7, 0, Con_doubleInvalid );
				//m_Table.SetCell( 5, m_Table.GetRowCount()-1, str );
			}
			else
			{
				fCPR = (double)nWW*nLL/((double)nWL*nLW);
				m_Table.SetCell( 6, 0, fCPR );
				fCov = sqrt (1.00/nWW + 1.00/nLL + 1.00/nWL + 1.00/nLW);
				m_Table.SetCell( 7, 0, fCov );
				fZValue = log10(fCPR)/fCov;
/*				str.Format( "%f",fZValue );
				m_Table.SetCell( 7, m_Table.GetRowCount()-1, str );	*/	
			}
			delete []m_mbuf;
			m_mbuf = NULL;
			delete []m_nbuf;
			m_nbuf = NULL;
			delete []m_fbuf;
			m_fbuf = NULL;
		}		
	}
}