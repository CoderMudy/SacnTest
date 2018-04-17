/**********************************************************
  Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
  File name:	StockWarn.h
  Author:		��־��
  Version:		1.0
  Date:			2007-08-08
  Description:	�����������ʾԤ����Ʊ�ĸ�������,����ʵ�ֶ����л�

***********************************************************/

#if !defined(AFX_STOCKWARN_H__09C4DD23_FEF9_4577_8E22_F05D084E2DC3__INCLUDED_)
#define AFX_STOCKWARN_H__09C4DD23_FEF9_4577_8E22_F05D084E2DC3__INCLUDED_
#include "TxBusiness.h"
#include "time.h"

#include "time.h"

namespace Tx
{
	namespace Business
	{

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

		struct  WarnCfgInfo
		{
			int		nInterval;	//Ԥ�����
			int		nLast;		//ͣ��ʱ��
			int		nSiren;		//����Ԥ��
			void Serialize(CArchive& ar)		//ʵ�ִ��л�
			{
				if (ar.IsLoading())
				{
					ar >> nInterval;			
					ar >> nLast;		
					ar >> nSiren;			
				}
				else
				{
					ar << nInterval;			
					ar << nLast;		
					ar << nSiren;	
				}
			}
		};
		
		class BUSINESS_EXT CStockWarn  
		{
		public:
			CStockWarn();	
			virtual ~CStockWarn();
		public:
			int			m_StockID;				//ȯ��ID
			double		m_HighPrice;			//��߼�
			double		m_LowPrice;				//��ͼ�
			double		m_ChangeExtent;			//�ǵ���
			double		m_TurnOver;				//�ɽ���
			double		m_TurnOverRate;			//������
			double		m_TurnOverVol;			//�ɽ����
			double		m_TurnOverAmount;		//�ɽ�����
			int			m_Date;					//����
			bool		m_flag;					//��־�Ƿ����
			bool		m_delete_flag;			//�Ƿ�ɾ��
			bool		m_check_flag;			//�Ƿ�鿴
			//-------20080324-----------------------------------
			bool		m_bSiren;				//�Ƿ񾯱�����
			bool		m_bStop;				//ֹͣԤ��
			int			m_nInterval;			//Ԥ�������һ����λΪ20��
			int			m_nLast;				//����ͣ��ʱ��
			//=====================================================
			tm			m_time;					//Ԥ��ʱ��


			void Serialize(CArchive& ar)		//ʵ�ִ��л�
			{
				if (ar.IsLoading())
				{
					ar >> m_StockID;			
					ar >> m_HighPrice;		
					ar >> m_LowPrice;			
					ar >> m_ChangeExtent;		
					ar >> m_TurnOver;			
					ar >> m_TurnOverRate;		
					ar >> m_TurnOverVol;		
					ar >> m_TurnOverAmount;	
					ar >> m_Date;				
					ar >> m_flag;				
					ar >> m_delete_flag;		
					ar >> m_check_flag;
					ar >> m_bSiren;	
					ar >> m_bStop;
					ar >> m_nInterval;
					ar >> m_nLast;
					ar >> m_time.tm_wday;
					ar >> m_time.tm_year;
					ar >> m_time.tm_mon;
					ar >> m_time.tm_mday;
					ar >> m_time.tm_hour;
					ar >> m_time.tm_min;
					ar >> m_time.tm_isdst;
					ar >> m_time.tm_sec;
					ar >> m_time.tm_yday;
					ar >> m_bSiren;	
					ar >> m_bStop;
					ar >> m_nInterval;
					ar >> m_nLast;
					
				}
				else
				{
					ar << m_StockID;			
					ar << m_HighPrice;		
					ar << m_LowPrice;			
					ar << m_ChangeExtent;		
					ar << m_TurnOver;			
					ar << m_TurnOverRate;		
					ar << m_TurnOverVol;		
					ar << m_TurnOverAmount;	
					ar << m_Date;				
					ar << m_flag;				
					ar << m_delete_flag;		
					ar << m_check_flag;		
					ar << m_bSiren;	
					ar << m_bStop;
					ar << m_nInterval;
					ar << m_nLast;
					ar << m_time.tm_wday;
					ar << m_time.tm_year;
					ar << m_time.tm_mon;
					ar << m_time.tm_mday;
					ar << m_time.tm_hour;
					ar << m_time.tm_min;
					ar << m_time.tm_isdst;
					ar << m_time.tm_sec;
					ar << m_time.tm_yday;
					ar << m_bSiren;	
					ar << m_bStop;
					ar << m_nInterval;
					ar << m_nLast;

				}
			}	
		};
}
}
#endif // !defined(AFX_STOCKWARN_H__09C4DD23_FEF9_4577_8E22_F05D084E2DC3__INCLUDED_)

