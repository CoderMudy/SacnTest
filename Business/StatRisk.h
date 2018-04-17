/**********************************************************
  Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
  File name:		StatRisk.h
  Author:			��־��
  Version:			1.0
  Date:				2007-10-08
  Description:
					����һ�������࣬������ɻ����ҵ�������ĸ����㷨�ĵ���
***********************************************************/
#pragma once
#include "TxBusiness.h"
#include "..\..\Core\Control\Manage.h"
#include "FundDeriveData.h"
#include "FundDeriveData.h"
#include "..\..\Core\Core\Table.h"
#include "..\..\Core\Zkklibrary\DateTime.h"
#include "..\..\core\driver\Table_Display.h"
#include "..\..\Core\Core\ProgressWnd.h"
#include <vector>
#include <set>

//**************************************


//****************************************************************

namespace Tx
{	
	namespace Business
	{
		class BUSINESS_EXT StatRisk
		{
			#define  JJM_YJPJ     10486    //����ҵ������
			#define  JJ_PJ_RISK1  10487    //ҵ�����շ���
			#define  JJ_PJ_RISK2  10488    //���յ�����������
			#define  JJ_PJ_RISK3  10489    //ҵ���������
			#define  JJ_PJ_TM	  10511    //T-M������ģ��
			#define  JJ_PJ_HM     10512    //H-M������ģ��
			#define  JJ_PJ_CL	  10513    //C-L������ģ��
			#define  JJ_PJ_GII    10514    //GIIģ��
			#define  JJ_PJ_TM3	  10515    //T-M������ģ��
			#define  JJ_PJ_HM3    10516    //H-M������ģ��
			#define  JJ_PJ_CL3	  10517    //C-L������ģ��
			#define  JJ_PJ_GII3   10518    //GII������ģ��
			#define  JJ_PJ_FAMA   10519    //FAMA�ֽ�
			#define  JJ_PJ_RISK4  10490    //ҵ������
			#define  JJ_PJ_JY_ZXG 10520    //�����ϵ������(���ڳ�����)
			#define  JJ_PJ_JY_JBL 10521    //��������ʼ���(���ڳ�����)
		public:
			StatRisk();
			~StatRisk();
		public:
			int		m_nNowId;			//��ǰ��������id
			int m_nType;				//ҵ������������
			std::vector<double> m_Result;	
										//�洢��������������������
			std::set<int> m_Sample;
										//�洢��������ID�ļ���	
			//Tx::Core::Table_Indicator m_Table;
			Table_Display	m_Table;			
							//�洢����������������ı�
			
			
			int		m_nTotal;			//��������
			Tx::Core::CStat_Math	*m_pMath;		
										//�㷨������
			
			//��������
			double	*m_fbuf;			//��������ʱ������
			double	*m_nbuf;			//�޷��������ʲ�������
			double	*m_mbuf;			//�г������ʲ�������	
			double	*m_buf1;			//����ÿ���г�ʱ�����մ����ĵ�λ�����ʲ�����ֵ
			double	*m_buf2;			//���ָ���У�С��ָ�������ʣ�����ָ��������
			double	*m_buf3;			//�����̼�ֵ�����ʣ�С�̼�ֵ�����ʣ� �������̳ɳ������ʣ�С�̳ɳ������ʣ�
			int		m_LagPeriod;		//������
			int		iEndDate;
			int		iStartDate;
			int		m_algo_type;
			int		m_nData_type;
			int		m_std_type;
			int		m_NoRiskRet_Type;
			int		m_item_count;
			int		m_nDate_Type;
			int		m_nCycle;
			double	m_fUserDefineNrValue;		
			char	m_AvrRetCode[255];				
		public:
			// ���ò���ΪĬ��ֵ
			void Set_Param_Default();
			// ��ȡ����
			BOOL Get_Param();
			// ������������
			BOOL Check_Param();

			// ��ʼ��ֵ
			void Init_ResultData();
			// ��ʼ��ͳ�ƽ�� ResultData
			BOOL Reset_Result_Buffer();

			// ��ѡ�е�������ʾ��ϵͳ
			BOOL On_Init_Stock ();
			// ��ʼͳ�Ƽ���
			BOOL Start_Stat();
	/*	private:*/
		public:
			// ����ҵ�����շ���
			void Init_Risk1_Data ();
			void On_Calc_Risk1 ();
			// ���յ�����������
			void Init_Risk2_Data ();
			void On_Calc_Risk2 ();
			// T-M������ģ��
			void Init_TM_Data ();
			void On_Calc_TM ();
			// H-M������ģ��
			void Init_HM_Data ();
			void On_Calc_HM ();
			// C-L������ģ��
			void Init_CL_Data ();
			void On_Calc_CL ();
			// GIIģ��
			void Init_GII_Data ();
			void On_Calc_GII ();
			
			// T-M������ģ��
			void Init_TM3_Data ();
			void On_Calc_TM3 ();
			// H-M������ģ��
			void Init_HM3_Data ();
			void On_Calc_HM3 ();
			// C-L������ģ��
			void Init_CL3_Data ();
			void On_Calc_CL3 ();
			// GII������ģ��
			void Init_GII3_Data ();
			void On_Calc_GII3 ();
			
			// FAMA�ֽ�
			void Init_FAMA_Data ();
			void On_Calc_FAMA ();	
			// Brinson�ֽ�
			void Init_Brinson_Data ();
			void On_Calc_Brinson ();

			//�����ϵ������(���ڳ�����)
			void Init_JY_ZXG ();
			void On_Calc_JY_ZXG ();
			//��������ʼ���(���ڳ�����)
			void Init_JY_JBL ();
			void On_Calc_JY_JBL ();
			//Spearman����ؼ���(һ���Լ���)
			void Init_JY_SPE ();
			void On_Calc_JY_SPE ();

			//��������
			void Set_Result_Data ();
			//����ÿ���г�ʱ�����մ����ĵ�λ�����ʲ�����ֵ
			void GetGIIPmt( );
			
		};
	}
}