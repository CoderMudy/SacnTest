/**********************************************************
  Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
  File name:		Warn_Cfg.cpp
  Author:			��־��
  Version:			1.0
  Date:				2007-09-04
  Description:
					����һ�������࣬������ɹ�ƱԤ�������ļ��أ����棬
					�Ƚ�Ԥ��������Ԥ������ȹ���
***********************************************************/

#include "stdafx.h"
#include "Warn_Cfg.h"
#include <unordered_map>
#include "..\..\Core\Control\Manage.h"
#include "..\Business\TxBusiness.h"
#include "..\..\Data\SecurityIdMap.h"

namespace Tx
{
	namespace Business
	{

		CWarn_Cfg* CWarn_Cfg::GetInstance()
		{
			static CWarn_Cfg _instance;
			return &_instance;
		}	
		CWarn_Cfg::CWarn_Cfg()
		{
			OleInitialize(NULL);
			//m_flag			=	false;	//��ʾδ������
			m_warnInfo.nInterval = 5;		//Ԥ�����
			m_warnInfo.nLast = 5;			//ͣ��ʱ��
			m_warnInfo.nSiren = 0;			//Ԥ������
			m_nLowFrequence = 5;			//��ƵĬ��Ϊ5
			m_bNeedReload = true;
			m_nPause = 0;	
			m_bDoing = false;
			m_nSoundOn = 1;					//�Ƿ�����Ԥ��
			m_nAlertOnce = 0;				// �Ƿ������һ��
			//���ϸ�ʽת��Ϊ�¸�ʽ
			if ( UpdateCfg() )
				TRACE(_T("----Ԥ����ʽת���ɹ�----\r\n"));
			UpdateWarnItemName();
			m_nToday = COleDateTime::GetCurrentTime().GetYear()*10000 + COleDateTime::GetCurrentTime().GetMonth()*100 + COleDateTime::GetCurrentTime().GetDay();
			m_nPreInterval = m_warnInfo.nInterval;
			m_nRemainTime = m_warnInfo.nInterval;
			m_showTable.Clear();
			m_showTable.AddCol(Tx::Core::dtype_val_string);	//����
			m_showTable.AddCol(Tx::Core::dtype_val_string);	//����
			m_showTable.AddCol(Tx::Core::dtype_val_string);	//��ֵ
			m_showTable.AddCol(Tx::Core::dtype_val_string);	//����
			m_showTable.AddCol(Tx::Core::dtype_val_string);	//ʱ��
			m_showTable.AddCol(Tx::Core::dtype_val_string); //Ψһ��ʶ id-item-date-time
			m_hisTable.Clear();
			m_hisTable.AddCol(Tx::Core::dtype_val_string);	//����
			m_hisTable.AddCol(Tx::Core::dtype_val_string);	//����
			m_hisTable.AddCol(Tx::Core::dtype_val_string);	//��ֵ
			m_hisTable.AddCol(Tx::Core::dtype_val_string);	//����
			m_hisTable.AddCol(Tx::Core::dtype_val_string);	//ʱ��
			m_hisTable.AddCol(Tx::Core::dtype_val_string); //Ψһ��ʶ id-item-date-time
		}

		CWarn_Cfg::~CWarn_Cfg()
		{
			m_pWarnArray.clear();
			m_pWarnResult.clear();
			OleUninitialize();
		}
		
		//Ĭ�ϵļ��غ���,�������ļ����ص�ָ������
		bool CWarn_Cfg::Load()
		{
			CString strFileName = Tx::Core::Manage::GetInstance()->m_pUserInfo->GetWarningPath() + _T("\\warncfg.dat");
			CFile f;
			if (!f.Open(strFileName, CFile::modeRead|CFile::shareDenyRead)) return false;
			CArchive ar(&f, CArchive::load);	
			m_pWarnArray.clear();
			int count = 0;
			ar >> count;
			m_warnInfo.Serialize(ar);
			for (int i = 0; i < count; i++)
			{
				CStockWarn item;
				item.Serialize(ar);
				m_pWarnArray.push_back(item);
			}
			ar.Close();
			f.Close();
			return true;
		}	
		
		//���غ���������,��������Ԥ�����
		bool CWarn_Cfg::Load( std::vector<CStockWarn> &pArray, CString strFileName )
		{
			CString FileName = Tx::Core::Manage::GetInstance()->m_pUserInfo->GetWarningPath() + _T( strFileName );
			CFile f;
			if (!f.Open(FileName, CFile::modeRead|CFile::shareDenyRead)) return false;
			CArchive ar(&f, CArchive::load);
			
			pArray.clear();
			int count = 0;
			ar >> count;
			for (int i = 0; i < count; i++)
			{
				CStockWarn item;
				item.Serialize(ar);
				pArray.push_back(item);
			}
			ar.Close();
			f.Close();
			return true;
		}	
		
		//Ĭ�ϵı��溯��,��Ĭ���������ݱ��浽�ļ�
		bool CWarn_Cfg::Save()
		{
			//if( !m_flag )		//��־�Ƿ��������,��������Ϊtrue
			//	return false;

			CString strFileName = Tx::Core::Manage::GetInstance()->m_pUserInfo->GetWarningPath() + _T("\\warncfg.dat");
			CFile f;
			if (!f.Open(strFileName, CFile::modeCreate|CFile::modeWrite)) return false;
			CArchive ar(&f, CArchive::store);	
			int count = (int)m_pWarnArray.size();
			ar << count;
			m_warnInfo.Serialize(ar);
			for (int i = 0; i < count; i++)
			{
				CStockWarn& item = m_pWarnArray[i];
				item.Serialize(ar);
			}
			ar.Close();
			f.Close();
			//m_flag = false;
			return true;
		}
		
		//���溯��������,������ĳ�������е����ݱ��浽ָ���ļ�
		bool CWarn_Cfg::Save( std::vector<CStockWarn> pArray, CString strFileName )
		{
			//if( !m_flag )		//��־�Ƿ��������,��������Ϊtrue
			//	return false;
			CString FileName = Tx::Core::Manage::GetInstance()->m_pUserInfo->GetWarningPath() + _T(strFileName);
			CFile f;
			if (!f.Open(FileName, CFile::modeCreate|CFile::modeWrite)) return false;
			CArchive ar(&f, CArchive::store);
			
			int count = (int)pArray.size();
			ar << count;
			for (int i = 0; i < count; i++)
			{
				CStockWarn& item = pArray[i];
				item.Serialize(ar);
			}
			ar.Close();
			f.Close();
			///m_flag = false;
			return true;
		}
		void CWarn_Cfg::SetAlertOnce(bool bOnce)
		{
			m_nAlertOnce = bOnce ? 1 : 0;
			m_setAlertOnce.clear();
		}
		//���Ҽ��Ԥ������Ƿ��д˼�¼,û����׷��,����Ҫ�ж�ʱ����Ҫ����
		bool CWarn_Cfg::Alert()
		{
			CString strTest = _T("");
			if ( m_bDoing )
				return false;
			else
				m_bDoing = true;
			if( m_bNeedReload )
				LoadExt();
			if ( m_nPause )
			{
				m_bDoing = false;			
				return false;
			}
			int nStockCount = m_Nodes.NumNodes();
			if( nStockCount == 0 )
			{
				m_bDoing = false;			
				return false;
			}
			int nDate = COleDateTime::GetCurrentTime().GetYear()*10000 + COleDateTime::GetCurrentTime().GetMonth()*100 + COleDateTime::GetCurrentTime().GetDay();
			if ( m_nToday != nDate )
			{
				//������ʷ��¼
				CString s;
				s.Format(_T("%d.xml"),m_nToday);
				SaveResult(m_hisTable,0,s);
				m_nToday = nDate;
				m_strSet.clear();
			}
			COleDateTime tm = COleDateTime::GetCurrentTime();
			CString strDate = tm.Format(_T("%Y-%m-%d"));
			CString strTm = tm.Format(_T(" %H:%M:%S"));
			CString strFlag = _T("");
			//ÿ�����
			m_showTable.DeleteRow(0,m_showTable.GetRowCount());
			m_showTable.Arrange();
			//����Ԥ����Ŀ����
			int	nWarnCount = 0;	
			//����Ԥ��
			std::set<CString>		sSet;

			// �Ƿ������һ��
			bool bOnce = m_nAlertOnce == 1;
			bool bDoAlert = bOnce ? false : true;

			try
			{
				//ѭ������
				for(int i = 0; i < nStockCount; i++)
				{
					CXmlNodeWrapper stockNode(m_Nodes.GetNode(i));
					int nItemCount = stockNode.NumNodes();
					if( nItemCount == 0 )
						continue;
					int nSecurityId = _ttoi(stockNode.GetValue(_T("id")));
					int nStop = _ttoi(stockNode.GetValue(_T("stop")));
					Tx::Business::TxBusiness business;
					business.GetSecurityNow(nSecurityId);
					if ( business.m_pSecurity == NULL || nStop == 1 )
						continue;
					int iMarketId = business.m_pSecurity->GetInnerBourseId();
					bool bTrading = Tx::Data::DataStatus::GetInstance()->IsTrading( iMarketId );
					if ( !bTrading )
						continue;
					//ȡ�÷ֱ�����
					Tx::Data::TradeDetail* pDetail = NULL;
					business.m_pSecurity->GetTradeDetail();
					pDetail = business.m_pSecurity->GetTradeDetailPointer();
					if ( pDetail == NULL )
						continue;
					int nCount = pDetail->GetTrdaeDetailDataCount();
					Tx::Data::TradeDetailData* pNow = pDetail->GetTrdaeDetailData( nCount-1);
					Tx::Data::TradeDetailData* pPre = pDetail->GetTrdaeDetailData( nCount-2);
					if( pNow == NULL )
						continue;
					Tx::Data::TradeQuotation *pTradeQuotation;
					//��������
					CString strTime = _T("");
					bool bEtf = business.m_pSecurity->IsFund_ETF();
					double dPrice = pNow->fPrice;
					ZkkLib::Time tm = pNow->tTime;
					ZkkLib::Time stampTm = tm;
					
					//int nSecurityId = business.m_pSecurity->GetId();

					//ѭ����Ŀ
					strFlag = _T("");
					for( int j = 0; j < nItemCount; j++)
					{
						CXmlNodeWrapper itemNode(stockNode.GetNode(j));
						int nItem = _ttoi(itemNode.GetValue(_T("id")));
						CString strLimit = itemNode.GetValue(_T("limit"));
						CString strMusic = itemNode.GetValue(_T("music"));
						CString strInterval = itemNode.GetValue(_T("interval"));
						double dLimit = atof(strLimit);
						if ( dLimit == 0.0 )
							continue;
						double dVal = 0.0;
						CString strMsg = _T("");
						
						switch(nItem)
						{
							case 1:	//�۸����
								if ( dPrice > dLimit )
								{
									if ( bEtf )
										strMsg.Format(_T("���¼�:%.3f,%s:%.3f"),dPrice,m_TotalItem[0],dLimit);
									else
										strMsg.Format(_T("���¼�:%.2f,%s:%.2f"),dPrice,m_TotalItem[0],dLimit);
								}
								strFlag.Format(_T("%d"),tm.GetInt());
								break;
							case 2:	//�۸����
								if ( dPrice < dLimit)
								{
									//�����¼
									if ( bEtf )
										strMsg.Format(_T("���¼�:%.3f,%s:%.3f"),dPrice,m_TotalItem[1],dLimit);
									else
										strMsg.Format(_T("���¼�:%.2f,%s:%.2f"),dPrice,m_TotalItem[1],dLimit);
								}
								strFlag.Format(_T("%d"),tm.GetInt());
								break;	
							case 3:	//�ǵ�������
								dVal = abs((business.m_pSecurity->GetClosePrice()-business.m_pSecurity->GetPreClosePrice())/business.m_pSecurity->GetPreClosePrice()*100);
								if( dVal > dLimit )
								{
									if ( bEtf )
										strMsg.Format(_T("���¼�:%.3f,%s:%.2f"),dPrice,m_TotalItem[2],dVal);
									else
										strMsg.Format(_T("���¼�:%.2f,%s:%.2f"),dPrice,m_TotalItem[2],dVal);
								}
								strFlag.Format(_T("%d"),tm.GetInt());
								break;
							case 4:	//�ֱʼ۲��
								if( nCount > 1 )
								{
									if ( pPre != NULL )
									{
										dVal = abs((pNow->fPrice - pPre->fPrice)/pPre->fPrice * 100);
										if( dVal > dLimit )
										{
											//�����¼
											if ( bEtf )
												strMsg.Format(_T("���¼�:%.3f,%s:%.2f"),dPrice,m_TotalItem[3],dVal);
											else
												strMsg.Format(_T("���¼�:%.2f,%s:%.2f"),dPrice,m_TotalItem[3],dVal);
										}
									}

								}
								strFlag.Format(_T("%d"),tm.GetInt());
								break;
							case 5:	//��ʳɽ�
								if ( nCount > 1 )
								{
									if ( pPre != NULL )
									{
										dVal =( pNow->dVolume - pPre->dVolume ) / 100;
										if(dVal > dLimit )
										{
											//�����¼
											if ( bEtf )
												strMsg.Format(_T("���¼�:%.3f,%s:%.0f"),dPrice,m_TotalItem[4],dVal);
											else
												strMsg.Format(_T("���¼�:%.2f,%s:%.0f"),dPrice,m_TotalItem[4],dVal);
										}	
									}
								}
								strFlag.Format(_T("%d"),tm.GetInt());
								break;
							case 6:	//���ʹҵ�(��)����(��)
								{
									pTradeQuotation = business.m_pSecurity->GetTradeQuotationPointer();
									double dVolume = 0.0;
									ZkkLib::Time tmTemp;
									double dValue = 0.0;
									for ( int i = 1; i <= 5; i++ )
									{
										float fPrice = 0.0;
										pTradeQuotation->GetBuyData(i,tmTemp,fPrice,dVolume);
										dValue += dVolume;
										if ( dVolume >= dVal )
											dVal = dVolume;
									}
									strFlag.Format(_T("%0.f"),dValue );
									dVal = dVal/100;
									if ( dVal > dLimit )
									{
										//�����¼
										if ( bEtf )
											strMsg.Format(_T("���¼�:%.3f,%s:%.0f"),dPrice,m_TotalItem[5],dVal);
										else
											strMsg.Format(_T("���¼�:%.2f,%s:%.0f"),dPrice,m_TotalItem[5],dVal);
									}
								}
								break;
							case 7:	//���ʹҵ�(����)����(��)
								{
									pTradeQuotation = business.m_pSecurity->GetTradeQuotationPointer();
									double dVolume = 0.0;
									ZkkLib::Time tmTemp;
									double dValue = 0.0;
									for ( int i = 1; i <= 5; i++ )
									{
										float fPrice = 0.0;
										pTradeQuotation->GetSaleData(i,tmTemp,fPrice,dVolume);
										dValue += dVolume;
										if ( dVolume >= dVal )
										{
											dVal = dVolume;
										}
									}
									strFlag.Format(_T("%0.f"),dValue);
									dVal = dVal/100;
									if ( dVal > dLimit )
									{
										//�����¼
										if ( bEtf )
											strMsg.Format(_T("���¼�:%.3f,%s:%.0f"),dPrice,m_TotalItem[6],dVal);
										else
											strMsg.Format(_T("���¼�:%.2f,%s:%.0f"),dPrice,m_TotalItem[6],dVal);
									}
								}
								break;
							default:
								break;			
						}
						if( strMsg.GetLength() != 0 )
						{
							// �����б���
							// �����˽�����һ��
							if (bOnce)
							{
								CString strOnce;
								strOnce.Format(_T("%d|%d"), nSecurityId, nItem);
								// ȷ����ֻ��Ʊ����������Ƿ��Ѿ�������
								std::set<CString>::iterator iter_once = m_setAlertOnce.find(strOnce);
								bool bFind = iter_once != m_setAlertOnce.end();
								if (!bFind)
								{
									m_setAlertOnce.insert(strOnce);
									bDoAlert = true;
								}
							}

							CString strStamp = _T("");	
							strStamp.Format(_T("%d|%d|%d|%s|%0.f"),nSecurityId,nItem,nDate,strFlag,dVal);
							if ( m_strSet.count(strStamp) == 0)
							{
								m_strSet.insert(strStamp);
								m_showTable.AddRow();
								m_showTable.SetCell( 0,m_showTable.GetRowCount()-1,business.m_pSecurity->GetName());
								m_showTable.SetCell( 1,m_showTable.GetRowCount()-1,strMsg );
								m_showTable.SetCell( 2,m_showTable.GetRowCount()-1,strLimit);
								m_showTable.SetCell( 3,m_showTable.GetRowCount()-1,strDate );
								m_showTable.SetCell( 4,m_showTable.GetRowCount()-1,strTm );
								m_showTable.SetCell( 5,m_showTable.GetRowCount()-1,strStamp );
								nWarnCount++;
								m_hisTable.AddRow();
								m_hisTable.SetCell( 0,m_hisTable.GetRowCount()-1,business.m_pSecurity->GetName());
								m_hisTable.SetCell( 1,m_hisTable.GetRowCount()-1,strMsg );
								m_hisTable.SetCell( 2,m_hisTable.GetRowCount()-1,strLimit);
								m_hisTable.SetCell( 3,m_hisTable.GetRowCount()-1,strDate );
								m_hisTable.SetCell( 4,m_hisTable.GetRowCount()-1,strTm );
								m_hisTable.SetCell( 5,m_hisTable.GetRowCount()-1,strStamp );
								//�����Ƿ�����Ԥ��
								CString strAlert;
								strAlert.Format(_T("%d|%d|%s|%d"),nSecurityId,nItem,strLimit,1 );
								sSet.insert(strAlert);
							}
							else
							{
								CString strAlert;
								strAlert.Format(_T("%d|%d|%s|%d"),nSecurityId,nItem,strLimit,0 );
								sSet.insert(strAlert);
							}
						}
					}	
				}
			}
			catch (_com_error& e)
			{
				CString ErrorStr;
				_bstr_t bstrSource(e.Source());
				_bstr_t bstrDescription(e.Description());
				ErrorStr.Format( _T("Excel Error\n\tCode = %08lx\n\tCode meaning = %s\n\tSource = %s\n\tDescription = %s\n"),
					e.Error(), e.ErrorMessage(), (LPCSTR)bstrSource, (LPCSTR)bstrDescription);
				AfxMessageBox(ErrorStr, MB_OK | MB_ICONERROR );
			}	
			LowFrequency(sSet,m_s2iMap,m_alertSound);
			AlertSound( m_alertSound );
			m_alertSound.clear();
			m_bDoing = false;
			if ( nWarnCount != 0 )
			{
				if (bOnce)
				{
					return bDoAlert;
				}
				else
				{
					return true;
				}
			}
			else
				return false;
		}
		//����µ�Ԥ����Ʊ,����Ѵ���,�򲻲���
		bool CWarn_Cfg::AddNewSample(int StockID)
		{
			//if( !m_flag )
			//	m_flag = true;
				
			//��������Լ�������ֵ�Ĵ��룬
			CStockWarn t;
			bool bFlag = false;;
			t.m_StockID	= StockID;
			int nCount = m_pWarnArray.size();
			for( int i = 0; i <nCount; i++ )
			{
				if( StockID != m_pWarnArray[i].m_StockID )
					continue;
				else
				{
					bFlag = true;
					break;
				}
			}
				
			if( !bFlag )
			{
				m_pWarnArray.push_back(t);
				return true;
			}
			else
				return false;
		}

		//ɾ��Ԥ����Ʊ
		bool CWarn_Cfg::DeleteSample( int nIndex )
		{
			//m_pWarnArray.erase(m_pWarnArray.begin() + nIndex);
			//�ж�һ��nIndex�Ƿ�Ϸ�,С����Ļ�����Խ��
			if( nIndex < 0 )
				return false;
			m_pWarnArray.erase(m_pWarnArray.begin() + nIndex);
			//m_flag = true;
			return true;
		}
		//����Ԥ�������Ϣ��һ������
		bool CWarn_Cfg::LoadResult(Tx::Core::Table& resTable)
		{
			//��ʽ��table
			resTable.Clear();
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.AddRow();
	
			
			CString sMsg;
			sMsg = "��Ʊ����";
			resTable.SetCell(0,0,sMsg);
			sMsg = "��ƱID";
			resTable.SetCell(1,0,sMsg);
			sMsg = "��߼�";
			resTable.SetCell(2,0,sMsg);
			sMsg = "��ͼ�";
			resTable.SetCell(3,0,sMsg);
			sMsg = "�ǵ���";
			resTable.SetCell(4,0,sMsg);
			sMsg ="�ɽ���";
			resTable.SetCell(5,0,sMsg);
			sMsg = "������";
			resTable.SetCell(6,0,sMsg);
			sMsg = "�ɽ����";
			resTable.SetCell(7,0,sMsg);
			sMsg = "�ɽ�����";
			resTable.SetCell(8,0,sMsg);
			sMsg = "����";
			resTable.SetCell(9,0,sMsg);
			sMsg = "ʱ��";
			resTable.SetCell(10,0,sMsg);

			CString FileName = Tx::Core::Manage::GetInstance()->m_pUserInfo->GetWarningPath() +_T("\\warnresult.dat");
			CFile f;
			if (!f.Open(FileName, CFile::modeRead|CFile::shareDenyRead)) return false;
			CArchive ar(&f, CArchive::load);
			
			m_pWarnResult.clear();
			int count = 0;
			ar >> count;
			for (int i = 0; i < count; i++)
			{
				CStockWarn item;
				item.Serialize(ar);
				m_pWarnResult.push_back(item);

			}

			ar.Close();
			f.Close();

			for( int n=0; n<count; n++ )
			{
				CStockWarn p = m_pWarnResult[n];
				resTable.AddRow();

				SecurityQuotation* se = (SecurityQuotation*)GetSecurity(p.m_StockID);	
				sMsg = se->GetName();
				resTable.SetCell(0,n+1,sMsg);
				
				sMsg.Format(_T("%s"),se->GetCode());
				resTable.SetCell(1,n+1,sMsg);

				sMsg.Format(_T("%.3f"),p.m_HighPrice);
				resTable.SetCell(2,n+1,sMsg);

				sMsg.Format(_T("%.3f"),p.m_LowPrice);
				resTable.SetCell(3,n+1,sMsg);

				sMsg.Format(_T("%.3f"),p.m_ChangeExtent);
				resTable.SetCell(4,n+1,sMsg);

				sMsg.Format(_T("%.3f"),p.m_TurnOver);
				resTable.SetCell(5,n+1,sMsg);

				sMsg.Format(_T("%.3f"),p.m_TurnOverRate);
				resTable.SetCell(6,n+1,sMsg);

				sMsg.Format(_T("%.3f"),p.m_TurnOverVol);
				resTable.SetCell(7,n+1,sMsg);

				sMsg.Format(_T("%.3f"),p.m_TurnOverAmount);
				resTable.SetCell(8,n+1,sMsg);

				sMsg.Format(_T("%.d"),p.m_Date);
				resTable.SetCell(9,n+1,sMsg);
				int iYr = p.m_time.tm_wday;
				int iMn = p.m_time.tm_year;
				int iDy = p.m_time.tm_mon;
				int iHr = p.m_time.tm_mday;
				int iMt = p.m_time.tm_hour;
				int iSe = p.m_time.tm_min;
				sMsg.Format(_T("%d/%d/%d %d:%d:%d"),iYr,iMn,iDy,iHr,iMt,iSe);
				resTable.SetCell(10,n+1,sMsg);
				
			}
			return true;
		}
		void CWarn_Cfg::NotifyChange(void)
		{
			m_bNeedReload = true;
		}

		CString CWarn_Cfg::LoadExt( CString strFileName )
		{
			m_bNeedReload = false;
			CString strRet  = _T("");
			//OleInitialize(NULL);
			CXmlDocumentWrapper doc;
			if ( doc.Load((LPCTSTR)strFileName ))
			{
				CXmlNodeWrapper root(doc.AsNode());
				//Ԥ������
				CXmlNodeWrapper configNode(root.GetNode(_T("Config")));
				if ( configNode.IsValid())
				{
					CString strInterval = configNode.GetValue(_T("interval"));
					CString strSound = configNode.GetValue(_T("sound"));
					CString strLast = configNode.GetValue(_T("last"));
					CString strPause = configNode.GetValue(_T("pause"));
					CString strLowFre = configNode.GetValue(_T("lowFrequency"));
					CString	strSoundOn = configNode.GetValue(_T("soundon"));
					CString strAlertOnce = configNode.GetValue(_T("AlertOnce"));
					m_warnInfo.nSiren = _ttoi( strSound);
					m_warnInfo.nInterval = _ttoi( strInterval);
					m_warnInfo.nLast = _ttoi( strLast );
					m_nPause = _ttoi(strPause);
					m_nLowFrequence = _ttoi( strLowFre );
					m_nSoundOn = _ttoi( strSoundOn );
					m_nAlertOnce = _ttoi(strAlertOnce);
					if( m_nLowFrequence <= 0 )
						m_nLowFrequence = 5;
				}
				else
				{
					m_warnInfo.nSiren = 0;
					m_warnInfo.nInterval = 5;
					m_warnInfo.nLast = 5;
					m_nLowFrequence = 5;
					m_nSoundOn = 0;
					m_nAlertOnce = 0;
				}
				//----Ԥ����Ŀ��ʱ���-------
				m_itemNode = root.FindNode(_T("Item"));
				if ( m_itemNode.IsValid())
				{
					m_ItemVec.clear();
					m_ItemStrVec.clear();
					m_i2iMap.clear();
					int count = m_itemNode.NumNodes();
					for ( int i = 0; i < count; i++ )
					{
						CXmlNodeWrapper node( m_itemNode.GetNode(i));
						int id = _ttoi(node.GetValue(_T("val")));
						CString str = node.GetValue(_T("name"));
						m_ItemVec.push_back(id);
						m_ItemStrVec.push_back(str);
						m_i2iMap.insert(std::pair<int,int>(id,i));
					}
				}
				else
				{
					CXmlDocumentWrapper doc1;
					doc1.LoadXML(_T("<?xml version=\"1.0\" encoding=\"utf-8\"?><Item></Item>"));
					m_itemNode = doc1.AsNode();
					m_ItemVec.clear();
					m_ItemStrVec.clear();
					m_i2iMap.clear();
				}
				m_Nodes = root.FindNode(_T("Warn"));
				if ( !m_Nodes.IsValid())
				{
					CXmlDocumentWrapper doc2;
					doc2.LoadXML(_T("<?xml version=\"1.0\" encoding=\"utf-8\"?><Warn></Warn>"));
					m_Nodes = doc2.AsNode();
				}
				strRet = m_Nodes.GetXML();
			}
			else
			{

				CXmlDocumentWrapper doc1;
				doc1.LoadXML(_T("<?xml version=\"1.0\" encoding=\"utf-8\"?><Item></Item>"));
				m_itemNode = doc1.AsNode();
				m_ItemVec.clear();
				m_ItemStrVec.clear();
				m_i2iMap.clear();

				CXmlDocumentWrapper doc2;
				doc2.LoadXML(_T("<?xml version=\"1.0\" encoding=\"utf-8\"?><Warn></Warn>"));
				m_Nodes = doc2.AsNode();

			}
			GetSoundCfg();
			//OleUninitialize();
			return strRet;
		}
		CString CWarn_Cfg::LoadExt()
		{
			CString strFileName = Tx::Core::Manage::GetInstance()->m_pUserInfo->GetWarningPath() + _T("\\warncfg.xml");
			return LoadExt(strFileName);
		}
		bool CWarn_Cfg::SaveExt( CString strFileName )
		{
			bool bRet = false;
			//OleInitialize(NULL);
			CXmlDocumentWrapper doc;
			if ( doc.Load((LPCTSTR)strFileName) )
			{
				CXmlNodeWrapper root(doc.AsNode());
				CXmlNodeWrapper cfgNode(root.FindNode(_T("Config")));
				cfgNode.SetValue(_T("interval"),m_warnInfo.nInterval);
				cfgNode.SetValue(_T("sound"),m_warnInfo.nSiren );				
				cfgNode.SetValue(_T("last"),m_warnInfo.nLast );
				cfgNode.SetValue(_T("pause"),m_nPause);
				cfgNode.SetValue(_T("lowFrequency"),m_nLowFrequence );
				cfgNode.SetValue(_T("soundon"),m_nSoundOn );
				cfgNode.SetValue(_T("AlertOnce"),m_nAlertOnce );
				//---Ԥ����Ŀ��ʱ���޸�
				CXmlNodeWrapper itemNode(root.FindNode(_T("Item")));
				if ( itemNode.IsValid())
					root.ReplaceNode( itemNode.Interface(),m_itemNode.Interface());
				//---����Ԥ����Ʊ
				CXmlNodeWrapper warnNode(root.FindNode(_T("Warn")));
				if ( warnNode.IsValid())
				{
					int nCount = m_Nodes.NumNodes();
					for ( int i = 0; i < nCount;i++ )
					{
						CXmlNodeWrapper oldNode(m_Nodes.GetNode(i));
						CString strId = oldNode.GetValue(_T("id"));
						int nItem = oldNode.NumNodes();
						for ( int j = 0; j < nItem;j++ )
						{
							CXmlNodeWrapper oldItemNode(oldNode.GetNode(j));
							CString sId = oldItemNode.GetValue(_T("id"));
							CString sLimit = oldItemNode.GetValue(_T("limit"));
							CString s;
							s.Format(_T("%s|%s|%s"),strId,sId,sLimit);
							std::unordered_map<CString,int>::iterator iter = m_s2nMap.find(s);
							if ( iter != m_s2nMap.end())
							{
								oldItemNode.SetValue(_T("music"),(int)iter->second/1000);
								oldItemNode.SetValue(_T("interval"),(int)iter->second%1000);
							}
							else
							{
								oldItemNode.SetValue(_T("music"),0);
								oldItemNode.SetValue(_T("interval"),5);
							}
						}
					}
					root.ReplaceNode( warnNode.Interface(),m_Nodes.Interface());
				}
				m_bNeedReload = true;
				bRet = doc.Save((LPCTSTR)strFileName)==TRUE?true:false;
			}
			else
			{
				if (doc.LoadXML(_T("<?xml version=\"1.0\" encoding=\"utf-8\"?><Data></Data>")))
				{
					CXmlNodeWrapper root(doc.AsNode());
					CXmlNodeWrapper cfgNode(root.InsertNode(0,_T("Config")));
					cfgNode.SetValue(_T("interval"),m_warnInfo.nInterval);
					cfgNode.SetValue(_T("sound"),m_warnInfo.nSiren );
					cfgNode.SetValue(_T("last"),m_warnInfo.nLast );
					cfgNode.SetValue(_T("pause"),m_nPause);
					cfgNode.SetValue(_T("soundon"),m_nSoundOn );
					cfgNode.SetValue(_T("AlertOnce"),m_nAlertOnce );
					//---Ԥ����Ŀ��ʱ���޸�
					CXmlNodeWrapper itemNode(root.InsertNode(1,_T("Item")));
					if ( itemNode.IsValid())
					{
						int nCount = m_itemNode.NumNodes();
						for ( int i = 0; i < nCount;i++ )
						{
							CXmlNodeWrapper oldNode(m_itemNode.GetNode(i));
							CString strVal = oldNode.GetValue(_T("val"));
							CString strName = oldNode.GetValue(_T("name"));
							CXmlNodeWrapper newNode(itemNode.InsertNode(i,_T("Id")));
							itemNode.SetValue(_T("val"),strVal);
							itemNode.SetValue(_T("name"),strName);
						}
					}
					//---����Ԥ����Ʊ
					CXmlNodeWrapper warnNode(root.InsertNode(2,_T("Warn")));
					if ( warnNode.IsValid())
					{
						int nCount = m_Nodes.NumNodes();
						for ( int i = 0; i < nCount;i++ )
						{
							CXmlNodeWrapper oldNode(m_itemNode.GetNode(i));
							CString strId = oldNode.GetValue(_T("id"));
							CString strStop = oldNode.GetValue(_T("stop"));
							CXmlNodeWrapper newNode(itemNode.InsertNode(i,_T("Id")));
							itemNode.SetValue(_T("id"),strId);
							itemNode.SetValue(_T("stop"),strStop);
							int nItem = oldNode.NumNodes();
							for ( int j = 0; j < nItem;j++ )
							{
								CXmlNodeWrapper oldItemNode(oldNode.GetNode(j));
								CString sId = oldItemNode.GetValue(_T("id"));
								CString sLimit = oldItemNode.GetValue(_T("limit"));
								CXmlNodeWrapper newItemNode(newNode.InsertNode(j,_T("Item")));
								newItemNode.SetValue(_T("id"),sId);
								newItemNode.SetValue(_T("limit"),sLimit);
								CString s;
								s.Format(_T("%s|%s|%s"),strId,sId,sLimit);
								std::unordered_map<CString,int>::iterator iter = m_s2nMap.find(s);
								if ( iter != m_s2nMap.end())
								{
									newItemNode.SetValue(_T("music"),(int)iter->second/1000);
									newItemNode.SetValue(_T("interval"),(int)iter->second%1000);
								}
								else
								{
									newItemNode.SetValue(_T("music"),0);
									newItemNode.SetValue(_T("interval"),5);
								}
							}
						}
					}
					m_bNeedReload = true;
					bRet = doc.Save((LPCTSTR)strFileName)==TRUE?true:false;
				}
			}
			//OleUninitialize();
			return bRet; 
		}

		int CWarn_Cfg::excel_format_data::GetStockId() const
		{
			Tx::Data::SecurityCodeMap* pMap = Tx::Data::SecurityCodeMap::GetInstance();
			// ��һ�� ���ͣ�1����Ʊ��2��ָ����3������4��ծȯ
			int iType = _ttoi(arrData[0]);
			// �ڶ��� ����
			CString strCode = arrData[1];
			strCode.MakeLower();		
			switch (iType)
			{
			case 1:	// stock
				{
					auto iter = pMap->StockCodeMap().find(strCode);
					if (iter != pMap->StockCodeMap().end())
					{
						return iter->second;
					}	
					iter = pMap->HKStockCodeMap().find(strCode);
					if (iter != pMap->HKStockCodeMap().end())
					{
						return iter->second;
					}
				}
				break;
			case 2:	// index
				{
					auto iter = pMap->IndexCodeMap().find(strCode);
					if (iter != pMap->IndexCodeMap().end())
					{
						return iter->second;
					}					
				}
				break;
			case 3:	// fund
				{
					auto iter = pMap->FundCodeMap().find(strCode);
					if (iter != pMap->FundCodeMap().end())
					{
						return iter->second;
					}					
				}
				break;
			case 4:	// bond
				{
					auto iter = pMap->BondCodeMap().find(strCode);
					if (iter != pMap->BondCodeMap().end())
					{
						return iter->second;
					}					
				}
				break;
			default: 
				break;
			}
			return 0;
		}
		
		CString CWarn_Cfg::excel_format_data::GetLimit(int nId) const
		{
			CString strLimit = _T("");
			int nCol = nId + 2;
			if (nId >= 2 && nId <= 8)
			{
				strLimit = arrData[nCol];
			}
			return strLimit;
		}

		bool CWarn_Cfg::LoadFrom_excel(std::vector<CWarn_Cfg::excel_format_data>& arrSamples, std::vector<CString>& arrUnknow)
		{
			arrSamples.clear();
			arrUnknow.clear();

			CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, _T("Excel�ĵ�(*.xls)|*.xls|CSV(���ŷָ�)|*.csv||"));
			if (dlg.DoModal() != IDOK) 
				return false;
			CString strFileName = dlg.GetPathName();

			::Excel::_ApplicationPtr excelApp = NULL;
			if(!SUCCEEDED(excelApp.CreateInstance(_T("Excel.Application"))))
				return false;

			::Excel::WorkbooksPtr excelBooks = excelApp->GetWorkbooks();
			if (excelBooks == NULL)
			{
				excelApp.Release();
				return false;
			}
			::Excel::_WorkbookPtr excelBook = excelBooks->Open(_bstr_t(strFileName));
			if (excelBook == NULL)
			{
				excelBooks.Release();
				excelApp.Release();
				return false;
			}

			::Excel::SheetsPtr excelSheets = excelBook->GetSheets();
			if (excelSheets == NULL)
			{
				excelBook.Release();
				excelBooks.Release();
				excelApp.Release();
				return false;
			}

			::Excel::_WorksheetPtr excelSheet = excelSheets->GetItem(1);
			if (excelSheet == NULL)
			{
				excelSheets.Release();
				excelBook.Release();
				excelBooks.Release();
				excelApp.Release();
				return false;
			}

			// m_pRange = NULL;
			std::vector<CString> arrCol;
			arrCol.push_back(_T("A"));
			arrCol.push_back(_T("B"));
			arrCol.push_back(_T("C"));
			arrCol.push_back(_T("D"));
			arrCol.push_back(_T("E"));
			arrCol.push_back(_T("F"));
			arrCol.push_back(_T("G"));
			arrCol.push_back(_T("H"));
			arrCol.push_back(_T("I"));

			int nRow = 2;	// �ӵڶ��п�ʼ
			while (true)
			{
				CString s;
				s.Format(_T("%s%d"), _T("A"), nRow);
				::Excel::RangePtr excelRange = excelSheet->GetRange(_variant_t(s), _variant_t(s));
				_variant_t vItem = excelRange->GetItem(_variant_t(1), _variant_t(1));
				CString strCell = CString(vItem);
				// ��һ�� Ϊ������ѭ��
				if (strCell.GetLength() == 0)
					break; 
				// type
				CWarn_Cfg::excel_format_data data;
				data.arrData[0] = strCell;
				excelRange.Release();
				// id
				s.Format(_T("%s%d"), _T("B"), nRow);
				excelRange = excelSheet->GetRange(_variant_t(s), _variant_t(s));
				vItem = excelRange->GetItem(_variant_t(1), _variant_t(1));
				strCell = CString(vItem);
				data.arrData[1] = strCell;
				excelRange.Release();
				// limits
				for (int c = 0; c < 7; c++)
				{
					s.Format(_T("%s%d"), arrCol[c+2], nRow);
					excelRange = excelSheet->GetRange(_variant_t(s), _variant_t(s));
					vItem = excelRange->GetItem(_variant_t(1), _variant_t(1));
					strCell = CString(vItem);
					data.arrData[c+2] = strCell;
					excelRange.Release();
				}
				// ��ȷ��У��
				if (data.GetStockId() != 0)
					arrSamples.push_back(data);
				else
				{
					CString strU;
					strU.Format(_T("(%s)%s"), data.arrData[1], data.arrData[1]);
					arrUnknow.push_back(strU);	// δʶ�����
				}
				nRow++;
			}
			
			//CString s;
			//s.Format(_T("%s%d"), _T("A"), 1);
			//::Excel::RangePtr excelRange = excelSheet->GetRange(_variant_t(s), _variant_t(s));
			//_variant_t vItem = excelRange->GetItem(_variant_t(1), _variant_t(1));
			//CString strCell = CString(vItem);
	
			//AfxMessageBox(strCell);

			excelSheet.Release();
			excelSheets.Release();
			excelBook->Close();
			excelBook.Release();
			excelBooks.Release();
			excelApp.Release();

			return true;
		}

		bool CWarn_Cfg::SaveExt_excel ()
		{
			CString strFileName = Tx::Core::Manage::GetInstance()->m_pUserInfo->GetWarningPath() + _T("\\warncfg.xml");
			return SaveExt_excel(strFileName);
		}


		// ��excel����������
		bool CWarn_Cfg::SaveExt_excel (const CString& strFileName )
		{
			// �����ֲ�����
			WarnCfgInfo x_warnInfo;						//Ԥ������	
			x_warnInfo.nInterval = m_warnInfo.nInterval;//Ԥ�����
			x_warnInfo.nLast = m_warnInfo.nLast;		//ͣ��ʱ��
			x_warnInfo.nSiren = m_warnInfo.nSiren;		//Ԥ������
			int x_nPause = m_nPause;					//�Ƿ���ͣԤ��
			int x_nLowFrequence = m_nLowFrequence;		//������Ƶ
			int x_nSoundOn = m_nSoundOn;				//�Ƿ�����Ԥ��
			int x_nAlertOnce = m_nAlertOnce;			// �Ƿ������һ��

			std::vector<CWarn_Cfg::excel_format_data> arrSamples;
			std::vector<CString> arrUnknow;

			if(FAILED(::CoInitialize(NULL)))
				return false;
			bool bLoadOk = LoadFrom_excel(arrSamples, arrUnknow);
			::CoUninitialize();

			if (!bLoadOk || arrSamples.empty())
				return false;

			if (!arrUnknow.empty())
			{
				CString str;
				str.Format(_T("����%d�д���δ��ʶ��:\r\n"), (int)arrUnknow.size());
				for (int i = 0; i < (int)arrUnknow.size(); i++)
				{
					str += arrUnknow[i] + _T(", ");
				}
				str += _T("\r\n�Ƿ���ɱ��β�����");
				if (AfxMessageBox(str, MB_YESNO) == IDNO)
					return false;
			}

			//CXmlNodeWrapper x_itemNode;				//Ԥ����Ŀ���
			
			std::vector<CString> x_arrItems;
			x_arrItems.push_back(_T("�۸����"));
			x_arrItems.push_back(_T("�۸����"));
			x_arrItems.push_back(_T("�ǵ���(%)"));
			x_arrItems.push_back(_T("�ֱʼ۲��(%)"));
			x_arrItems.push_back(_T("�󵥳ɽ�(��)"));
			x_arrItems.push_back(_T("�󵥹���(��)"));
			x_arrItems.push_back(_T("�󵥹���(��)"));

			bool bRet = false;
			//OleInitialize(NULL);
			CXmlDocumentWrapper doc;
			if ( doc.Load((LPCTSTR)strFileName) )
			{
				CXmlNodeWrapper root(doc.AsNode());
				CXmlNodeWrapper cfgNode(root.FindNode(_T("Config")));
				if (cfgNode.IsValid())
				{
					root.RemoveNode(root.FindNode(_T("Config")));
				}
				
				//---ɾ��ԭ�е�Ԥ����Ŀ
				CXmlNodeWrapper itemNode(root.FindNode(_T("Item")));
				if ( itemNode.IsValid())
				{
					root.RemoveNode(root.FindNode(_T("Item")));
				}
				//---ɾ��ԭ�еľ���Ԥ����Ʊ
				CXmlNodeWrapper warnNode(root.FindNode(_T("Warn")));
				if ( warnNode.IsValid())
				{
					root.RemoveNode(root.FindNode(_T("Warn")));
				}
			}
			else
			{
				if (!doc.LoadXML(_T("<?xml version=\"1.0\" encoding=\"utf-8\"?><Data></Data>")))
				{
					return false;
				}
			}

			m_bNeedReload = true;

			// д���ò���
			CXmlNodeWrapper root(doc.AsNode());
			CXmlNodeWrapper cfgNode(root.InsertNode(0,_T("Config")));
			if (!cfgNode.IsValid())
				return false;
			cfgNode.SetValue(_T("interval"),x_warnInfo.nInterval);
			cfgNode.SetValue(_T("sound"),x_warnInfo.nSiren );
			cfgNode.SetValue(_T("last"),x_warnInfo.nLast );
			cfgNode.SetValue(_T("pause"),x_nPause);
			cfgNode.SetValue(_T("soundon"),x_nSoundOn );
			cfgNode.SetValue(_T("AlertOnce"),x_nAlertOnce );

	
			//--- д Ԥ����Ŀ
			CXmlNodeWrapper itemNode(root.InsertNode(1,_T("Item")));
			if ( itemNode.IsValid())
			{
				CString sValue;
				for (int i = 0; i < (int)x_arrItems.size(); i++)
				{
					CXmlNodeWrapper newNode(itemNode.InsertNode(i,_T("Id")));
					if (newNode.IsValid())
					{
						sValue.Format(_T("%d"),i+1);
						newNode.SetValue(_T("name"),x_arrItems[i]);
						newNode.SetValue(_T("val"),sValue);
					}
				}
			}
			
			//---����Ԥ����Ʊ
			CXmlNodeWrapper warnNode(root.InsertNode(2,_T("Warn")));
			if ( warnNode.IsValid())
			{
				int nCount = (int)arrSamples.size();
				CString strId, strStop;
				for (int i = 0; i < nCount; i++)
				{
					const CWarn_Cfg::excel_format_data& stock_item = arrSamples[i];
					if (stock_item.GetStockId() == 0) continue;
					CXmlNodeWrapper newNode(warnNode.InsertNode(i,_T("Stock")));
					if (!newNode.IsValid()) continue;
					strStop = _T("0");
					strId.Format(_T("%d"), stock_item.GetStockId());	// ֤ȯid
					newNode.SetValue(_T("id"),strId);
					newNode.SetValue(_T("stop"),strStop);

					ASSERT(stock_item.arrData.size() == 2 + x_arrItems.size());

					for (int j = 0; j < (int)x_arrItems.size(); j++)
					{
						const CString &strLimits = stock_item.arrData[j + 2];
						if (strLimits.GetLength() == 0) continue;
						CXmlNodeWrapper stockNode(newNode.InsertNode(i,_T("Item")));
						if (!stockNode.IsValid()) continue;
						stockNode.SetValue(_T("interval"),x_warnInfo.nInterval);
						stockNode.SetValue(_T("id"),j+1);	// ?
						stockNode.SetValue(_T("music"),0);
						stockNode.SetValue(_T("limit"), strLimits);
					}
				}
			}
			////OleUninitialize();
			bRet = doc.Save((LPCTSTR)strFileName)==TRUE?true:false;
			return bRet; 
		}

		bool CWarn_Cfg::SaveExt()
		{
			CString strFileName = Tx::Core::Manage::GetInstance()->m_pUserInfo->GetWarningPath() + _T("\\warncfg.xml");
			return SaveExt(strFileName);
		}
		bool	CWarn_Cfg::LoadRes( Tx::Core::Table& resTable,CString strFileName )
		{
			resTable.Clear();
			resTable.AddCol(Tx::Core::dtype_val_string);		//����
			resTable.AddCol(Tx::Core::dtype_val_string);		//������Ϣ
			resTable.AddCol(Tx::Core::dtype_val_string);		//��ֵ
			resTable.AddCol(Tx::Core::dtype_val_string);		//����
			resTable.AddCol(Tx::Core::dtype_val_string);		//ʱ��
			CString s;
			if ( strFileName.GetLength() == 0 )
				strFileName.Format(_T("\\%d.xml"),COleDateTime::GetCurrentTime().GetYear()*10000 + COleDateTime::GetCurrentTime().GetMonth()*100 + COleDateTime::GetCurrentTime().GetDay());
			s.Format(_T("\\%s"),strFileName);
			CString strPath = Tx::Core::Manage::GetInstance()->m_pUserInfo->GetWarningPath() + s;
			//OleInitialize(NULL);
			CXmlDocumentWrapper doc;
			if ( doc.Load((LPCTSTR)strPath))
			{
				CXmlNodeWrapper root(doc.AsNode());
				if ( root.IsValid())
				{
					int nCount = root.NumNodes();
					for ( int i = 0; i < nCount; i++ )
					{
						CXmlNodeWrapper node(root.GetNode(i));
						CString strName = node.GetValue(_T("name"));
						int nSecurityId = _ttoi(node.GetValue(_T("id")));
						CString strDate = node.GetValue(_T("date"));
						CString strTime = node.GetValue(_T("time"));
						CString strDes = node.GetValue(_T("description"));
						CString strLimit = node.GetValue(_T("limit"));
						resTable.AddRow();
						resTable.SetCell(0,resTable.GetRowCount()-1,strName);
						resTable.SetCell(1,resTable.GetRowCount()-1,strDes);
						resTable.SetCell(2,resTable.GetRowCount()-1,strLimit);
						resTable.SetCell(3,resTable.GetRowCount()-1,strDate);
						resTable.SetCell(4,resTable.GetRowCount()-1,strTime);
					}
				}
				//OleUninitialize();
				return true;
			}
			//OleUninitialize();
			return false;
		}
		//bool	CWarn_Cfg::SaveRes( Tx::Core::Table& resTable,int nDeleteCount,int nWarnItem )
		//{
		//	bool bRet = false;
		//	//OleInitialize(NULL);
		//		if ( nDeleteCount == 0 )
		//		{
		//			int nCount = resTable.GetRowCount();
		//			if ( nCount <= 0 )
		//				return true;
		//			CString strFileName = Tx::Core::Manage::GetInstance()->m_pUserInfo->GetWarningPath() + _T("\\warnresult.xml");
		//			CXmlDocumentWrapper doc;
		//			Tx::Core::Op_File file;
		//			BOOL bLoad = FALSE;
		//			if ( !file.IsFileExist(strFileName))
		//				bLoad = doc.LoadXML(_T("<?xml version=\"1.0\" encoding=\"utf-8\"?><Data></Data>"));
		//			else
		//				bLoad = doc.Load((LPCTSTR)strFileName);
		//			if ( !bLoad )
		//			{
		//				Tx::Log::CLogRecorder::GetInstance()->WriteToLog(_T("��ƱԤ����ʷ�ļ�����ʧ��"));	
		//				file.TXDeleteFile(strFileName);
		//				return false;
		//			}
		//			CXmlNodeWrapper root(doc.AsNode());
		//			if ( !root.IsValid())
		//				return false;
		//			if ( nWarnItem != -1 )
		//			{
		//				nWarnItem = nCount-nWarnItem;
		//			}
		//			for ( int i = nWarnItem ; i < nCount; i++ )
		//			{
		//				CString strName;
		//				CString strDes;
		//				CString strLimit;
		//				CString strDate;
		//				CString strTime;
		//				resTable.GetCell(0,i,strName);
		//				resTable.GetCell(1,i,strDes);
		//				resTable.GetCell(2,i,strLimit);
		//				resTable.GetCell(3,i,strDate);
		//				resTable.GetCell(4,i,strTime);
		//				CXmlNodeWrapper node(root.InsertNode(0,_T("Record")));
		//				node.SetValue(_T("name"),strName);
		//				node.SetValue(_T("description"),strDes);
		//				node.SetValue(_T("date"),strDate);
		//				node.SetValue(_T("time"),strTime);
		//				node.SetValue(_T("limit"),strLimit);
		//			}
		//			bRet = doc.Save(strFileName);
		//		}
		//		else
		//		{
		//			CString strFileName = Tx::Core::Manage::GetInstance()->m_pUserInfo->GetWarningPath() + _T("\\warnresult.xml");
		//			CXmlDocumentWrapper doc;
		//			Tx::Core::Op_File file;
		//			BOOL bLoad = FALSE;
		//			if ( !file.IsFileExist(strFileName))
		//				bLoad = doc.LoadXML(_T("<?xml version=\"1.0\" encoding=\"utf-8\"?><Data></Data>"));
		//			else
		//				bLoad = doc.Load((LPCTSTR)strFileName);
		//			if ( !bLoad )
		//			{
		//				Tx::Log::CLogRecorder::GetInstance()->WriteToLog(_T("��ƱԤ����ʷ�ļ�����ʧ��"));	
		//				file.TXDeleteFile(strFileName);
		//				return false;
		//			}
		//			CXmlNodeWrapper root(doc.AsNode());
		//			if ( !root.IsValid())
		//				return false;
		//			int nItemCount = root.NumNodes();
		//			nItemCount = nItemCount-nDeleteCount;
		//			CXmlDocumentWrapper doc1;
		//			doc1.LoadXML(_T("<?xml version=\"1.0\" encoding=\"utf-8\"?><Data></Data>"));
		//			CXmlNodeWrapper root1(doc1.AsNode());
		//			if ( nItemCount >= 1)
		//			{
		//				for ( int k = 0; k < nItemCount;k++)
		//				{
		//					CXmlNodeWrapper node(root.GetNode(k));
		//					CString strName = node.GetValue(_T("name"));
		//					CString strDes = node.GetValue(_T("description"));
		//					CString strLimit = node.GetValue(_T("limit"));
		//					CString strDate = node.GetValue(_T("date"));
		//					CString strTime = node.GetValue(_T("time"));
		//					CXmlNodeWrapper node1(root1.InsertNode(k,_T("Record")));
		//					node1.SetValue(_T("name"),strName);
		//					node1.SetValue(_T("description"),strDes);
		//					node1.SetValue(_T("date"),strDate);
		//					node1.SetValue(_T("time"),strTime);
		//					node1.SetValue(_T("limit"),strLimit);
		//				}
		//			}
		//			int nCount = resTable.GetRowCount();
		//			if ( nCount > 0 )
		//			{
		//				if ( nWarnItem != -1 )
		//				{
		//					nWarnItem = nCount-nWarnItem;
		//				}
		//				for ( int i =0 ; i < nCount; i++ )
		//				{
		//					CString strName;
		//					CString strDes;
		//					CString strLimit;
		//					CString strDate;
		//					CString strTime;
		//					resTable.GetCell(0,i,strName);
		//					resTable.GetCell(1,i,strDes);
		//					resTable.GetCell(2,i,strLimit);
		//					resTable.GetCell(3,i,strDate);
		//					resTable.GetCell(4,i,strTime);
		//					CXmlNodeWrapper node(root.InsertNode(0,_T("Record")));
		//					node.SetValue(_T("name"),strName);
		//					node.SetValue(_T("description"),strDes);
		//					node.SetValue(_T("date"),strDate);
		//					node.SetValue(_T("time"),strTime);
		//					node.SetValue(_T("limit"),strLimit);
		//				}
		//			}
		//			bRet =  doc1.Save(strFileName);
		//		}	
		//	//OleUninitialize();
		//	return bRet;
		//}
		bool	CWarn_Cfg::UpdateCfg()
		{
			bool bRet = false;
			//�ж��Ƿ��Ѿ�����
			CString strOld = Tx::Core::Manage::GetInstance()->m_pUserInfo->GetWarningPath() + _T("\\warncfg.dat");
			CString strNew = Tx::Core::Manage::GetInstance()->m_pUserInfo->GetWarningPath() + _T("\\warncfg.xml");
			Tx::Core::Op_File file;
			if ( file.IsFileExist(strNew))
				return true;
			//OleInitialize(NULL);
			if ( !file.IsFileExist(strOld))
			{
				CXmlDocumentWrapper doc1;
				doc1.LoadXML(_T("<?xml version=\"1.0\" encoding=\"utf-8\"?><Data><Config interval=\"5\" sound=\"False\" last=\"5\" pause=\"0\"/><Item><Id val=\"1\" name=\"��߼�\"/><Id val=\"2\" name=\"��ͼ�\"/></Item><Warn></Warn></Data>"));
				doc1.Save((LPCTSTR)strNew);
				return true;
			}
			Load();
			CXmlDocumentWrapper doc;
			doc.LoadXML(_T("<?xml version=\"1.0\" encoding=\"utf-8\"?><Data></Data>"));
			CXmlNodeWrapper root(doc.AsNode());
			CXmlNodeWrapper configNode(root.InsertNode(0,_T("Config")));
			configNode.SetValue(_T("interval"),5);
			configNode.SetValue(_T("last"),5);
			configNode.SetValue(_T("pause"),0);
			configNode.SetValue(_T("sound"),m_warnInfo.nSiren);
			configNode.SetValue(_T("soundon"),0);

			CXmlNodeWrapper itemNode(root.InsertNode(1,_T("Item")));
			std::vector<CString> itemVec;
			GetWarnItem(itemVec);
			for ( int i = 0; i < (int)itemVec.size(); i++ )
			{
				CXmlNodeWrapper node(itemNode.InsertNode(i,_T("Id")));
				node.SetValue(_T("val"),i+1);
				node.SetValue(_T("name"),(LPCTSTR)itemVec[i]);
			}
			CXmlNodeWrapper warnNode( root.InsertNode(2,_T("Warn")));
			for ( int i = 0; i < (int)m_pWarnArray.size(); i++)
			{
				CXmlNodeWrapper stockNode(warnNode.InsertNode(i,_T("Stock")));
				if ( m_pWarnArray[i].m_bStop )
					stockNode.SetValue(_T("stop"),1);
				else
					stockNode.SetValue(_T("stop"),0);
				stockNode.SetValue(_T("id"),m_pWarnArray[i].m_StockID);
				CString str;
				if ( m_pWarnArray[i].m_HighPrice != 0.0 )
				{
					CXmlNodeWrapper node(stockNode.InsertNode(0,_T("Item")));
					node.SetValue(_T("id"),1);
					str.Format(_T("%.2f"),m_pWarnArray[i].m_HighPrice);
					node.SetValue(_T("limit"),str);
				}
				if ( m_pWarnArray[i].m_LowPrice != 0.0 )
				{
					CXmlNodeWrapper node(stockNode.InsertNode(0,_T("Item")));
					node.SetValue(_T("id"),2);
					str.Format(_T("%.2f"),m_pWarnArray[i].m_LowPrice);
					node.SetValue(_T("limit"),str);
				}
				if ( m_pWarnArray[i].m_ChangeExtent != 0.0 )
				{
					CXmlNodeWrapper node(stockNode.InsertNode(0,_T("Item")));
					node.SetValue(_T("id"),3);
					str.Format(_T("%.2f"),m_pWarnArray[i].m_ChangeExtent);
					node.SetValue(_T("limit"),str);
				}
				if ( m_pWarnArray[i].m_TurnOver != 0.0 )
				{
					CXmlNodeWrapper node(stockNode.InsertNode(0,_T("Item")));
					node.SetValue(_T("id"),4);
					str.Format(_T("%.2f"),m_pWarnArray[i].m_TurnOver);
					node.SetValue(_T("limit"),str);
				}
				if ( m_pWarnArray[i].m_TurnOverRate != 0.0 )
				{
					CXmlNodeWrapper node(stockNode.InsertNode(0,_T("Item")));
					node.SetValue(_T("id"),5);
					str.Format(_T("%.2f"),m_pWarnArray[i].m_TurnOverRate);
					node.SetValue(_T("limit"),str);
				}
			}
			bRet =  doc.Save((LPCTSTR)strNew)==TRUE?true:false;
			//OleUninitialize();
			return bRet;
		}
		bool	CWarn_Cfg::GetWarnItem(std::vector<CString>& itemVec )
		{
			itemVec.clear();
			m_TotalItem.clear();
			CString strFileName = Tx::Core::Manage::GetInstance()->m_pSystemPath->GetConfigPath() + _T("\\warnitem.xml");
			//OleInitialize(NULL);
			CXmlDocumentWrapper doc;
			if (doc.Load((LPCTSTR)strFileName) )
			{
				CXmlNodeWrapper root(doc.AsNode());
				int nCount = root.NumNodes();
				for ( int i = 0; i < nCount; i++ )
				{
					CXmlNodeWrapper node(root.GetNode(i));
					itemVec.push_back(node.GetValue(_T("name")));
					m_TotalItem.push_back(node.GetValue(_T("name")));
				}
			}
			//OleUninitialize();
			return true;
		}

		bool	CWarn_Cfg::ResetWarnItem(std::vector<int>& itemIdVec )
		{
			//OleInitialize(NULL);
			std::vector<CString> itemVec;
			GetWarnItem(itemVec);
			int nSeq = 0;
			m_ItemVec.clear();
			m_ItemStrVec.clear();
			m_i2iMap.clear();	
			m_itemNode.RemoveNodes(_T("Id"));
			try
			{
				CString str = _T("");
				for( std::vector<int>::iterator iter = itemIdVec.begin(); iter != itemIdVec.end(); ++iter )
				{
					CXmlNodeWrapper node(m_itemNode.InsertNode(nSeq,_T("Id")));
					int nId = *iter-1;
					str = itemVec[nId];
					node.SetValue(_T("val"),*iter);
					node.SetValue(_T("name"),str);
					m_ItemVec.push_back(*iter);
					m_ItemStrVec.push_back(str);
					m_i2iMap.insert(std::pair<int,int>(*iter,nSeq));
					nSeq++;
				}
			}
			catch (_com_error& e)
			{
				CString ErrorStr;
				_bstr_t bstrSource(e.Source());
				_bstr_t bstrDescription(e.Description());
				ErrorStr.Format( _T("Excel Error\n\tCode = %08lx\n\tCode meaning = %s\n\tSource = %s\n\tDescription = %s\n"),
					e.Error(), e.ErrorMessage(), (LPCSTR)bstrSource, (LPCSTR)bstrDescription);
				AfxMessageBox(ErrorStr, MB_OK | MB_ICONERROR );
			}
			Tx::Business::CWarn_Cfg::GetInstance()->SaveExt();
			//OleUninitialize();
			return  true;
		}
		bool	CWarn_Cfg::UpdateWarnItemName()
		{
			std::vector<CString> itemVec;
			if ( !GetWarnItem(itemVec))
				return false;
			int nSize = (int)itemVec.size();
			if(nSize < 1)
				return false;
			CString strFileName = Tx::Core::Manage::GetInstance()->m_pUserInfo->GetWarningPath() + _T("\\warncfg.xml");
			CXmlDocumentWrapper doc;
			if ( doc.Load((LPCTSTR)strFileName ))
			{
				CXmlNodeWrapper root(doc.AsNode());
				CXmlNodeWrapper nodeItem = root.FindNode(_T("Item"));
				if ( nodeItem.IsValid())
				{
					int nCount = nodeItem.NumNodes();
					for ( int i = 0; i < nCount; i++ )
					{
						CXmlNodeWrapper node = nodeItem.GetNode(i);
						int nId = _ttoi(node.GetValue(_T("val")));
						if(nId > 0 && nId <= nSize)
							node.SetValue(_T("name"),itemVec[nId-1]);
					}
				}
				doc.Save(strFileName);
				m_bNeedReload = true;
				return true;
			}
			else
				return false;	
		}
		bool	CWarn_Cfg::SaveResult( Tx::Core::Table& resTable,int nCount, CString strFileName )
		{
			int nRowCount = resTable.GetRowCount();
			if ( nCount == 0 )
				nCount = nRowCount;
			if ( nCount > nRowCount )
				nCount = nRowCount;
			CString strPath = Tx::Core::Manage::GetInstance()->m_pUserInfo->GetWarningPath();
			if ( strFileName.GetLength()== 0 )
				strFileName.Format(_T("\\%d.xml"),COleDateTime::GetCurrentTime().GetYear()*10000 + COleDateTime::GetCurrentTime().GetMonth()*100 + COleDateTime::GetCurrentTime().GetDay());
			strPath += strFileName;
			bool bRet = false;
			try
			{				
				CXmlDocumentWrapper doc;
				if(!::PathFileExists(strPath))
				{
					if(!doc.LoadXML(_T("<?xml version=\"1.0\" encoding=\"utf-8\"?><Data></Data>")))
						return false;
				}
				else
				{
					if(!doc.Load((LPCTSTR)strPath))
					{
						::DeleteFile(strPath);

						if(!doc.LoadXML(_T("<?xml version=\"1.0\" encoding=\"utf-8\"?><Data></Data>")))
							return false;
					}
				}

				CXmlNodeWrapper root(doc.AsNode());
				if ( !root.IsValid())
					return false;
				for ( int i = 0 ; i < nCount; i++ )
				{
					CString strName;
					CString strDes;
					CString strLimit;
					CString strDate;
					CString strTime;
					resTable.GetCell(0,i,strName);
					resTable.GetCell(1,i,strDes);
					resTable.GetCell(2,i,strLimit);
					resTable.GetCell(3,i,strDate);
					resTable.GetCell(4,i,strTime);
					CXmlNodeWrapper node(root.InsertNode(0,_T("Record")));
					node.SetValue(_T("name"),strName);
					node.SetValue(_T("description"),strDes);
					node.SetValue(_T("date"),strDate);
					node.SetValue(_T("time"),strTime);
					node.SetValue(_T("limit"),strLimit);
				}
				resTable.DeleteRow(0,nCount);
				bRet = doc.Save((LPCTSTR)strPath)==TRUE?true:false;
			}
			catch (_com_error& e)
			{
				CString ErrorStr;
				_bstr_t bstrSource(e.Source());
				_bstr_t bstrDescription(e.Description());
				ErrorStr.Format( _T("Excel Error\n\tCode = %08lx\n\tCode meaning = %s\n\tSource = %s\n\tDescription = %s\n"),
					e.Error(), e.ErrorMessage(), (LPCSTR)bstrSource, (LPCSTR)bstrDescription);
				AfxMessageBox(ErrorStr, MB_OK | MB_ICONERROR );
			}
			return bRet;
		}

		bool	CWarn_Cfg::SaveResult()
		{
			return SaveResult(m_hisTable);
		}
		bool	CWarn_Cfg::LowFrequency( std::set<CString>& strSet, std::unordered_map<CString,int>& s2iMap, std::set<int>& iSet )
		{
			std::unordered_map<CString,int> tempMap(s2iMap);
			s2iMap.clear();
			for ( std::set<CString>::iterator itr = strSet.begin(); itr != strSet.end(); ++itr )
			{
				CString sStr = *itr;
				CString sStamp = sStr.Left( sStr.GetLength()-2);
				CString sFlag = sStr.Right( 1 );
				std::unordered_map<CString,int>::iterator ir = m_s2nMap.find(sStamp);
				int nLowFrequence = 5;
				int nMusic = 0;
				if ( ir != m_s2nMap.end())
				{
					int nVal = ir->second;
					nLowFrequence = nVal % 1000;
					nMusic = nVal/ 1000;
				}	
				std::unordered_map<CString,int>::iterator iter = tempMap.find(sStamp);
				//����ϴ�Ԥ��
				if ( iter != tempMap.end())
				{
					/*
					int nVal = iter->second + 5;
					if ( nVal > m_nLowFrequence )
					{
						nVal = 0;
						std::vector<CString> arrText;
						Tx::Core::Commonality::String().Split(sStamp,arrText,_T("-"));
						int nV = _ttoi(arrText[1]);
						if ( _ttoi(sFlag) == 1 )
							iSet.insert(nV);
					}
					s2iMap.insert(std::pair<CString,int>(sStamp,nVal));
					*/
					if ( _ttoi( sFlag ) == 0  )
					{
						int nVal = iter->second + 5;
						m_s2iMap.insert(std::pair<CString,int>(sStamp,nVal));
					}
					else
					{
						int nVal = iter->second + 5;
						if ( nVal >= nLowFrequence )
						{
							nVal = nMusic;
							//std::vector<CString> arrText;
							//Tx::Core::Commonality::String().Split(sStamp,arrText,_T("-"));
							//int nV = _ttoi(arrText[1]);
							iter->second = 0;
							if ( nVal > 0 )
								iSet.insert(nVal);

						}
						s2iMap.insert(std::pair<CString,int>(sStamp,nVal));
					}		
				}
				//�״�Ԥ��
				else
				{
					s2iMap.insert(std::pair<CString,int>(sStamp,0));
					//std::vector<CString> arrText;
					//Tx::Core::Commonality::String().Split(sStamp,arrText,_T("-"));
					//int nVal = _ttoi(arrText[1]);
					int nVal = nMusic;
					if ( nVal > 0 )
						iSet.insert(nVal);
				}
			}
			return true;
		}
		void	CWarn_Cfg::GetSoundCfg()
		{
			m_s2nMap.clear();
			int nStockCount = m_Nodes.NumNodes();
			if ( nStockCount <= 0 )
				return;
			try
			{
				//ѭ������
				for(int i = 0; i < nStockCount; i++)
				{
					CXmlNodeWrapper stockNode(m_Nodes.GetNode(i));
					int nItemCount = stockNode.NumNodes();
					if( nItemCount == 0 )
						continue;
					int nSecurityId = _ttoi(stockNode.GetValue(_T("id")));
					for( int j = 0; j < nItemCount; j++)
					{
						CXmlNodeWrapper itemNode(stockNode.GetNode(j));
						int nItem = _ttoi(itemNode.GetValue(_T("id")));
						CString strLimit = itemNode.GetValue(_T("limit"));
						CString strMusic = itemNode.GetValue(_T("music"));
						CString strInterval = itemNode.GetValue(_T("interval"));
						double dLimit = atof(strLimit);
						if ( dLimit == 0.0 )
							continue;
						CString s;
						s.Format(_T("%d|%d|%s"),nSecurityId,nItem,strLimit);
						int	nVal = _ttoi(strMusic)*1000 + _ttoi(strInterval);
						m_s2nMap.insert(std::pair<CString,int>(s,nVal));
					}
				}
			}
			catch (_com_error& e)
			{
				CString ErrorStr;
				_bstr_t bstrSource(e.Source());
				_bstr_t bstrDescription(e.Description());
				ErrorStr.Format( _T("Excel Error\n\tCode = %08lx\n\tCode meaning = %s\n\tSource = %s\n\tDescription = %s\n"),
					e.Error(), e.ErrorMessage(), (LPCSTR)bstrSource, (LPCSTR)bstrDescription);
				AfxMessageBox(ErrorStr, MB_OK | MB_ICONERROR );
			}

		}
		bool	CWarn_Cfg::AlertSound( std::set<int>& nSet )
		{
			if ( m_nSoundOn == 0 || nSet.size() == 0 )
				return false;
			CString sPath;
			CString s = Tx::Core::Manage::GetInstance()->m_pSystemPath->GetConfigPath() + _T("\\Sound");
			for ( std::set<int>::iterator iter = nSet.begin(); iter != nSet.end(); ++iter )
			{
				sPath.Format(_T("%s\\warn%d.wav"),s ,*iter );
				PlaySound(sPath,NULL,SND_SYNC);
			}
			//sPath.Format(_T("%s\\warn%d.wav"),s ,m_warnInfo.nSiren + 1 );
			//PlaySound(sPath,NULL,SND_SYNC);
			//for ( std::set<int>::iterator iter = nSet.begin(); iter != nSet.end(); ++iter )
			//{
			//	CString sPath;
			//	sPath.Format(_T("C:\\warn%d.wav"),*iter);
			//	PlaySound(sPath,NULL,SND_SYNC);
			//}
			return true;
		}
}

}