/**********************************************************
  Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
  File name:	StockWarn.cpp
  Author:		��־��
  Version:		1.0
  Date:			2007-08-08
  Description:	�����������ʾԤ����Ʊ�ĸ�������,����ʵ�ֶ����л�

***********************************************************/

#include "stdafx.h"
#include "StockWarn.h"
namespace Tx
{
	namespace Business
	{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CStockWarn::CStockWarn()
{
			//Ԥ��ʱ��
			m_time.tm_wday	= 0;
			m_time.tm_year	= 0;
			m_time.tm_mon	= 0;
			m_time.tm_mday	= 0;
			m_time.tm_hour	= 0;
			m_time.tm_min	= 0;
			m_time.tm_yday	= 0;
			m_time.tm_isdst = 0;
			m_time.tm_sec	= 0;
			
			m_StockID		=0;			//ȯ��ID
			m_HighPrice		=0;			//��߼�
			m_LowPrice		=0;			//��ͼ�
			m_ChangeExtent	=0;			//�ǵ���
			m_TurnOver		=0;			//�ɽ���
			m_TurnOverRate	=0;			//������
			m_TurnOverVol	=0;			//�ɽ����
			m_TurnOverAmount=0;			//�ɽ�����
			m_Date			=0;				//����
			m_flag			=false;			//��־�Ƿ����
											//false��ʾû�и���
			m_delete_flag	=false;			//�Ƿ�ɾ��
											//false ��ʾû��ɾ��
			m_check_flag	=false;			//�Ƿ�鿴
											//false��ʾû�в鿴
			

}

CStockWarn::~CStockWarn()
{

}
}
}