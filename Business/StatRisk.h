/**********************************************************
  Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
  File name:		StatRisk.h
  Author:			王志勇
  Version:			1.0
  Date:				2007-10-08
  Description:
					这是一个功能类，用来完成基金的业绩分析的各种算法的调用
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
			#define  JJM_YJPJ     10486    //基金业绩评价
			#define  JJ_PJ_RISK1  10487    //业绩风险分析
			#define  JJ_PJ_RISK2  10488    //风险调整收益评价
			#define  JJ_PJ_RISK3  10489    //业绩归因分析
			#define  JJ_PJ_TM	  10511    //T-M二次项模型
			#define  JJ_PJ_HM     10512    //H-M二次项模型
			#define  JJ_PJ_CL	  10513    //C-L二次项模型
			#define  JJ_PJ_GII    10514    //GII模型
			#define  JJ_PJ_TM3	  10515    //T-M三因素模型
			#define  JJ_PJ_HM3    10516    //H-M三因素模型
			#define  JJ_PJ_CL3	  10517    //C-L三因素模型
			#define  JJ_PJ_GII3   10518    //GII三因素模型
			#define  JJ_PJ_FAMA   10519    //FAMA分解
			#define  JJ_PJ_RISK4  10490    //业绩检验
			#define  JJ_PJ_JY_ZXG 10520    //自相关系数检验(短期持续性)
			#define  JJ_PJ_JY_JBL 10521    //交叉积比率检验(长期持续性)
		public:
			StatRisk();
			~StatRisk();
		public:
			int		m_nNowId;			//当前计算样本id
			int m_nType;				//业绩分析的类型
			std::vector<double> m_Result;	
										//存储单个基金分析结果的容器
			std::set<int> m_Sample;
										//存储基金样本ID的集合	
			//Tx::Core::Table_Indicator m_Table;
			Table_Display	m_Table;			
							//存储所有样本分析结果的表
			
			
			int		m_nTotal;			//样本个数
			Tx::Core::CStat_Math	*m_pMath;		
										//算法功能类
			
			//参数配置
			double	*m_fbuf;			//基金收益时间序列
			double	*m_nbuf;			//无风险收益率参照序列
			double	*m_mbuf;			//市场收益率参照序列	
			double	*m_buf1;			//计算每日市场时机把握带来的单位基金资产增加值
			double	*m_buf2;			//风格指数中，小盘指数收益率－大盘指数收益率
			double	*m_buf3;			//（大盘价值收益率＋小盘价值收益率） －（大盘成长收益率＋小盘成长收益率）
			int		m_LagPeriod;		//持续性
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
			// 设置参数为默认值
			void Set_Param_Default();
			// 读取参数
			BOOL Get_Param();
			// 检查参数合理性
			BOOL Check_Param();

			// 初始化值
			void Init_ResultData();
			// 初始化统计结果 ResultData
			BOOL Reset_Result_Buffer();

			// 将选中的样本显示到系统
			BOOL On_Init_Stock ();
			// 开始统计计算
			BOOL Start_Stat();
	/*	private:*/
		public:
			// 基金业绩风险分析
			void Init_Risk1_Data ();
			void On_Calc_Risk1 ();
			// 风险调整收益评价
			void Init_Risk2_Data ();
			void On_Calc_Risk2 ();
			// T-M二次项模型
			void Init_TM_Data ();
			void On_Calc_TM ();
			// H-M二次项模型
			void Init_HM_Data ();
			void On_Calc_HM ();
			// C-L二次项模型
			void Init_CL_Data ();
			void On_Calc_CL ();
			// GII模型
			void Init_GII_Data ();
			void On_Calc_GII ();
			
			// T-M三因素模型
			void Init_TM3_Data ();
			void On_Calc_TM3 ();
			// H-M三因素模型
			void Init_HM3_Data ();
			void On_Calc_HM3 ();
			// C-L三因素模型
			void Init_CL3_Data ();
			void On_Calc_CL3 ();
			// GII三因素模型
			void Init_GII3_Data ();
			void On_Calc_GII3 ();
			
			// FAMA分解
			void Init_FAMA_Data ();
			void On_Calc_FAMA ();	
			// Brinson分解
			void Init_Brinson_Data ();
			void On_Calc_Brinson ();

			//自相关系数检验(短期持续性)
			void Init_JY_ZXG ();
			void On_Calc_JY_ZXG ();
			//交叉积比率检验(长期持续性)
			void Init_JY_JBL ();
			void On_Calc_JY_JBL ();
			//Spearman秩相关检验(一致性检验)
			void Init_JY_SPE ();
			void On_Calc_JY_SPE ();

			//设置数据
			void Set_Result_Data ();
			//计算每日市场时机把握带来的单位基金资产增加值
			void GetGIIPmt( );
			
		};
	}
}