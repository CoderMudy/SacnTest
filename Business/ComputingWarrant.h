/**********************************************************
  Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
  File name:	ComputingWarrant.h
  Author:		��־��
  Version:		1.0
  Date:			2007-11-13
  Description:	Ȩ֤���㹤�ߵ��㷨ʵ��

***********************************************************/
#include "TxBusiness.h"
#include "..\..\Data\SecurityTradeDate.h"
#include "..\..\Data\HisTrade.h"
#include "..\..\core\driver\Table_Indicator.h"
#include "..\..\Core\TxMath\CWarrantMath.h"
#include "..\..\Core\Zkklibrary\Datetime.h"
#include "..\..\Core\Core\ProgressWnd.h"
#include "TxWarrant.h"
#include <set>
using namespace Tx::Data;
using namespace Tx::Core;

namespace Tx
{
	namespace Business
	{
		class	BUSINESS_EXT ComputingWarrant
		{
		public:
			ComputingWarrant();
			~ComputingWarrant();
		public:
			//-------------------------------------------------
			//����������B_Sģ�ͣ���֪�˲������Ժ���������ۼ۸�
			//****************************************************
			//ȡ��Ȩ֤ĳ�����ڵ����ۼ۸�
			double		GetTheoryPrice( bool	bFlag,		//�Ϲ������Ϲ�true�Ϲ�
										int		nStyle,		//Ȩ֤����0ŷʽ,1��ʽ,2��Ľ��
										double	dNoInterest,//�޷�������
										double	dSClose,	//��ǰ���֤ȯ�۸�
										double	dExPrice,	//��Ȩ�۸� 
										double	dExRatio,	//��Ȩ����
										double	dDelta,		//��Ĺ�Ʊ�����������
										double	dLasted,	//ʣ������
										double	dSigma		//��Ʊ���������������ʱ�׼��
										);
			//---------------------------------------------------------------
			//������㵱ǰ��Ȩ֤�����յ�����
			//***************************************************************
			//ȡ��Ȩ֤��ĳ�յ�ʣ������
			double		GetLastedPeriod( int nID, int nDate );
			//ȡ������������
			double		GetSigma(	bool	bCall_Put,		//�Ϲ��Ϲ�	true�Ϲ�
									double	dStockPrice,	//���֤ȯ�۸�
									double	dExPrice,		//��Ȩ�۸�
									double	dNoRiskInterest,//�޷�������
									double	dDelta,		
									double	dLastedYear,	//ʣ�����ޣ��꣩
									double	dMarketValue	//��Ȩ�г���ֵ
									);
			//---------------------------------------------------------------
			//�Ϲ�Ȩ֤�����=����Ȩ��+�Ϲ�Ȩ֤�۸�/��Ȩ����-���ɼۣ�/���ɼ� 
			//�Ϲ�Ȩ֤�����=�����ɼ�+�Ϲ�Ȩ֤�۸�/��Ȩ����-��Ȩ�ۣ�/���ɼ�
			//Ͷ������Ҫ�仯���ٰٷֱȣ��ſ���ʵ��ƽ��
			//*******************************************************************
			//ȡ�������
			double		GetPremiumRate( double dWPrice,		//Ȩ֤�۸�
										double dSPrice,		//��Ĺ�Ʊ�۸�
										double dExPrice,	//��Ȩ��
										bool	bFlag,		//�Ϲ�Ϊtrue,��֮Ϊfalse
										double	dRatio		//��Ȩ����
										);
			//--------------------------------------------------------------------
			//�ܸ˱��ʣ����֤ȯ�۸��(Ȩ֤�۸����Ȩ����)
			//*******************************************************************
			//ȡ�øܸ˱���
			double		GetGearRate(	double	dSClose,	//����ʲ������̼�
										double	dWClose,	//Ȩ֤�����̼�
										double	dExRatio	//��Ȩ����
										);
			//---------------------------------------------------------------------
			//Ȩ֤�ǣ��������۸�Ȩ֤ǰһ�����̼۸�������֤ȯ�����Ƿ��۸񣭱��֤ȯǰһ�����̼ۣ���125%����Ȩ����
			//**********************************************************************
			//ȡ����ͣ��
			double		GetRaiseLimit(	double	dWPrice,	//Ȩ֤ǰһ�����̼�
										//double	dSOpen,		//���֤ȯ���տ��̼۸�
										double	dSPreClose,	//���֤ȯǰһ�����̼�
										double	dRatio		//��Ȩ����
										);
										
			//ȡ�õ�ͣ��
			double		GetDropLimit(	double	dWPrice,	//Ȩ֤ǰһ�����̼�
										//double	dSOpen,		//���֤ȯ���տ��̼۸�
										double	dSPreClose,	//���֤ȯǰһ�����̼�
										double	dRatio		//��Ȩ����
										);
			//ȡ�ù�Ʊ��ָ������ǰһ��Ĳ�����,
			double		GetStockStd( int nID, int nEndDate, int nCycle );
			
			//����Ȩ֤���㹤���еĸ���ָ�꣬������뵽resTable�У���bInterestΪtrueʱ,dInterest��Ч,bStdΪtrue,dStd��Ч,����nCycle��Ч
			bool		bCalWarrant( Table &resTable,		//�����
									std::set< int > nID,	//Ȩ֤ID����
									int	nStartDate,			//��ʼʱ��
									int nEndDate,			//��ֹʱ��
									bool bInterest,			//��������			trueΪָ����falseΪһ��������
									double	dInterest,		//����ֵ
									bool	bStd,			//��Ʊ����������	trueΪָ����falseΪ����
									double	dStd,			//��Ʊ������ֵ
									int		nCycle			//��������			0��ʾ�գ�1��ʾ�ܣ�2��ʾ��
									);
			bool		bGetTradeDaySequence(	int	iSecurityId,	//����ʵ��ID
												int iEnd,			//��ֹ����
												int iStart,			//��ʼ����
												int iCycle,			//����
												std::vector< int > &iDates		//�����
												);
			bool		bGetHQByCycle(	int nSecurityId,	//����ʵ��Id 
										int nStartDate,		//��ʼ����
										int nEndDate,		//��ֹ����
										int	nCycle,			//����
										std::vector< HisTradeData >	&resVec	//�������
										);
			//ȡ���������ڵĽ�ֹ����		
			int			GetNextCycleDay( int nDate,			//��ʼ����
										int nCycle			//����	1 week; 2 month; 3 year
										);
			

		};

	}
}