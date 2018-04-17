/**********************************************************
  Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
  File name:		Warn_Cfg.h
  Author:			��־��
  Version:			1.0
  Date:				2007-09-04
  Description:
					����һ�������࣬������ɹ�ƱԤ�������ļ��أ����棬
					�Ƚ�Ԥ��������Ԥ������ȹ���
***********************************************************/

#if !defined(AFX_WARNCFG_H__312599AA_AAED_42AD_82D9_24CE7AEF3B20__INCLUDED_)
#define AFX_WARNCFG_H__312599AA_AAED_42AD_82D9_24CE7AEF3B20__INCLUDED_


#include "StockWarn.h"
#include <Afxtempl.h>
#include "Business.h"
#include "TxBusiness.h"
#include "..\..\Data\XmlNodeWrapper.h"
#include   <mmsystem.h>  
#pragma   comment(lib,   "winmm.lib")  
#include <unordered_map>
#include <vector>
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
using namespace Tx::Data;
namespace Tx
{
	namespace Business
	{
		class BUSINESS_EXT CWarn_Cfg  
		{
		public:
			static CWarn_Cfg* GetInstance();
			virtual ~CWarn_Cfg();
		private:
			CWarn_Cfg();
		public:
			//-----------------------------------
			std::vector<CStockWarn> m_pWarnArray;			//����ָ��Ԥ��ȯ�����õ������ָ��
			std::vector<CStockWarn> m_pWarnResult;			//����Ԥ�����
			int			m_nPreInterval;						//�ϴ�Ԥ��������ж��Ƿ����
			int			m_nRemainTime;						//��Ԥ�����ж�ã������Լ�����Ԥ�������ÿһ�ε��õݼ�20���ﵽ0ʱ��Ԥ��
			bool Save();									//����ȯ�����õ�dat�ļ�
			bool Load();									//����ȯ��Ԥ�����õ�ָ������
			bool Load( std::vector<CStockWarn> &pArray, CString strFileName );	//����ȯ��Ԥ�����õ��ƶ�������
			bool Save( std::vector<CStockWarn> pArray, CString strFileName  );	//���ƶ������е����ݱ���
			bool LoadResult(Tx::Core::Table& resTable);								//����Ԥ����Ϣ��һ��Table
			//-----------------------------------
			WarnCfgInfo				m_warnInfo;				//Ԥ������	
			std::vector<int>		m_ItemVec;				//����Ԥ����Ŀ
			std::vector<CString>	m_ItemStrVec;			//����Ԥ����Ŀ�ַ���
			std::vector<CString>	m_TotalItem;			//���е�Ԥ����Ŀ
			std::unordered_map<int,int>		m_i2iMap;				//Ԥ����Ŀ--��
			CXmlNodeWrapper			m_itemNode;				//Ԥ����Ŀ���
			CXmlNodeWrapper			m_Nodes;				//Ԥ����Ʊ�ڵ�
			Tx::Core::Table			m_showTable;			//����ʾ��ʱԤ����
			Tx::Core::Table			m_hisTable;				//Ԥ����ʷ
			std::set<CString>		m_strSet;				//�ṩһ��Ψһ��ʶ
			int						m_nPause;				//�Ƿ���ͣԤ��
			std::unordered_map<int,CString>	m_i2sMap;				//Ԥ����Ŀ
			int						m_nToday;				//��������
			bool					m_bDoing;				//���ڲ���
			int						m_nLowFrequence;		//������Ƶ
			std::unordered_map< CString,int > m_s2iMap;				//Ԥ����ʾ2��Ƶ���� 
			std::set<int>			m_alertSound;			//Ԥ������
			int						m_nSoundOn;				//�Ƿ�����Ԥ��
			int						m_nAlertOnce;			// �Ƿ������һ��
			std::unordered_map<CString,int>	
									m_s2nMap;				//Ԥ����Ŀ��Ԥ������������

			std::set<CString>		m_setAlertOnce;			// SecuirtyId | nItem
			void SetAlertOnce(bool bOnce);

		private:
			bool		m_bNeedReload;						//��־
		public:
			bool DeleteSample( int nIndex );				//ɾ��Ԥ��ȯ��
			bool AddNewSample( int StockID);				//����Ԥ��ȯ��

			void NotifyChange(void);												//�յ�֪ͨ�������Ѿ��޸�
			//����Ԥ����Ϣ
			CString LoadExt( CString strFileName );
			CString LoadExt();
			bool	SaveExt( CString StrFileName );
			bool	SaveExt();

			class excel_format_data
			{
			public:
				excel_format_data()
				{
					arrData.assign(9, _T(""));
				}
				~excel_format_data()
				{
					arrData.clear();
				}
				std::vector<CString> arrData;
				int GetStockId() const;
				CString GetLimit(int nId) const;
			};
			
			// ��excel����������
			bool SaveExt_excel ();
			bool SaveExt_excel (const CString& strFileName);
			bool LoadFrom_excel(std::vector<CWarn_Cfg::excel_format_data>& arrSamples, std::vector<CString>& arrUnknow);
		
			bool	Alert();
			bool	LoadRes( Tx::Core::Table& resTable, CString strFileName = _T(""));
			//bool	SaveRes( Tx::Core::Table& resTable,int	nDeleteCount = 0,int nWarnItem = -1 );
			bool	UpdateCfg();
			bool	GetWarnItem(std::vector<CString>& itemVec );
			bool	ResetWarnItem(std::vector<int>& itemIdVec );
			bool	UpdateWarnItemName();
			bool	SaveResult( Tx::Core::Table& resTable,int nCount = 0, CString strFileName = _T(""));
			bool	SaveResult();
			bool	AlertSound( std::set<int>& nSet );
		private:
			bool	LowFrequency( std::set<CString>& strSet, std::unordered_map<CString,int>& s2iMap, std::set<int>& iSet );
			void	GetSoundCfg();
		};
	}
}
#endif // !defined(AFX_WARNCFG_H__312599AA_AAED_42AD_82D9_24CE7AEF3B20__INCLUDED_)
