/**********************************************************
  Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
  File name:		Warn_Cfg.h
  Author:			王志勇
  Version:			1.0
  Date:				2007-09-04
  Description:
					这是一个功能类，用来完成股票预警样本的加载，保存，
					比较预警，保存预警结果等功能
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
			std::vector<CStockWarn> m_pWarnArray;			//保存指向预警券种设置的数组的指针
			std::vector<CStockWarn> m_pWarnResult;			//保存预警结果
			int			m_nPreInterval;						//上次预警间隔，判断是否更改
			int			m_nRemainTime;						//离预警还有多久，用于自己调整预警间隔，每一次调用递减20，达到0时候预警
			bool Save();									//保存券种设置到dat文件
			bool Load();									//加载券种预警设置到指定容器
			bool Load( std::vector<CStockWarn> &pArray, CString strFileName );	//加载券种预警设置到制定的容器
			bool Save( std::vector<CStockWarn> pArray, CString strFileName  );	//将制定容器中的内容保存
			bool LoadResult(Tx::Core::Table& resTable);								//加载预警信息到一个Table
			//-----------------------------------
			WarnCfgInfo				m_warnInfo;				//预警配置	
			std::vector<int>		m_ItemVec;				//保存预警项目
			std::vector<CString>	m_ItemStrVec;			//保存预警项目字符串
			std::vector<CString>	m_TotalItem;			//所有的预警项目
			std::unordered_map<int,int>		m_i2iMap;				//预警项目--列
			CXmlNodeWrapper			m_itemNode;				//预警项目结点
			CXmlNodeWrapper			m_Nodes;				//预警股票节点
			Tx::Core::Table			m_showTable;			//供显示当时预警的
			Tx::Core::Table			m_hisTable;				//预警历史
			std::set<CString>		m_strSet;				//提供一个唯一标识
			int						m_nPause;				//是否暂停预警
			std::unordered_map<int,CString>	m_i2sMap;				//预警项目
			int						m_nToday;				//当天日期
			bool					m_bDoing;				//正在操作
			int						m_nLowFrequence;		//报警低频
			std::unordered_map< CString,int > m_s2iMap;				//预警标示2低频报警 
			std::set<int>			m_alertSound;			//预警声音
			int						m_nSoundOn;				//是否声音预警
			int						m_nAlertOnce;			// 是否仅报警一次
			std::unordered_map<CString,int>	
									m_s2nMap;				//预警项目与预警的声音设置

			std::set<CString>		m_setAlertOnce;			// SecuirtyId | nItem
			void SetAlertOnce(bool bOnce);

		private:
			bool		m_bNeedReload;						//标志
		public:
			bool DeleteSample( int nIndex );				//删除预警券种
			bool AddNewSample( int StockID);				//增加预警券种

			void NotifyChange(void);												//收到通知，参数已经修改
			//加载预警信息
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
			
			// 从excel中批量导入
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
