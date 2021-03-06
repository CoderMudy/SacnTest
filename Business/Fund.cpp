/**************************************************************
Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
File name:		Fund.cpp
Author:			赵宏俊
Date:			2007-07-09
Version:		1.0
Description:	基金业务功能类
***************************************************************/
#include "stdafx.h"
#include "Fund.h"
#include <algorithm>
#include "..\..\core\driver\ClientFileEngine\base/Zip/ZipWrapper.h"

#include <xmllite.h>
#pragma comment(lib,"xmllite.lib")

using namespace Tx::Drive::IO::Zip;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

namespace Tx
{
	//////////////////////////////////////////////////////////////////////
	// fund_total_share_db_set 实现
	//////////////////////////////////////////////////////////////////////
	fund_total_share_db_set::fund_total_share_db_set(int date)
		: _M_date(date)
	{	
	}

	fund_total_share_db_set::~fund_total_share_db_set()
	{
	}

	int fund_total_share_db_set::get_count() const
	{		
		return (int)_M_set.size();
	}

	// 获取基金总份额
	double fund_total_share_db_set::get_value(int id) const
	{
		auto it = _M_set.find(id);
		return it == _M_set.end() ? Con_doubleInvalid : it->second;
	}

	bool fund_total_share_db_set::_Download()
	{
		TCHAR szUri[MAX_PATH]={0};
		wchar_t requestBody[MAX_PATH]={0};

		// 读取 URL
		TCHAR szExecuteFolder[MAX_PATH] = {0};
		TCHAR szINIFilename[MAX_PATH] = {0};

		::GetModuleFileName(NULL,szExecuteFolder,MAX_PATH);
		::PathRemoveFileSpec(szExecuteFolder);
		::PathCombine(szINIFilename,szExecuteFolder,_T("Config\\Report\\Common\\config.ini"));

		::GetPrivateProfileString(_T("Svr"),_T("AddressFund"),_T("http://masterservice.chinaciia.com/XGWebService/WebHandler.ashx"),szUri,ARRAYSIZE(szUri),szINIFilename);

		// 创建 POST 数据
		int size = ::swprintf_s(requestBody,
			L"<?xml version=\"1.0\" encoding=\"utf-16\"?>" \
			L"<data>" \
			L"<indicator value=\"7243\" return=\"2\">" \
			L"<parameter type=\"1\" value=\"%d\"/>" \
			L"</indicator>" \
			L"</data>",
			_M_date);

		// 发送请求
		Tx::Drive::Http::CSyncUpload syncUpload;
		if(!syncUpload.Post(szUri,(LPBYTE)requestBody,size * sizeof(wchar_t),_T("Content-Type: text/xml; charset=utf-16")))
		{
			return false;
		}

		// 读取响应
		const auto& response = syncUpload.Rsp();
		if(!response.IsSuccess())
		{
			// 请求失败
			return false;
		}

		const auto& body = response.Body();
		if(body.IsEmpty())
		{
			// 返回数据包为空
			return false;
		}

		// 读取并解析响应
		if(!_Read_response(body.DataPtr(),body.Size()))
			return false;

		return true;
	}

	bool fund_total_share_db_set::_Read_response(unsigned char * _Buffer,unsigned int _NumberOfBytes)
	{	
		HRESULT hr = S_OK;
		CHeapPtr<unsigned char,CGlobalAllocator> buffer;		
		CComPtr<IStream> spXmlStream;
		CComPtr<IXmlReader> spReader;
		CComPtr<IXmlReaderInput> spReaderInput;		

		// 解压缩
		unsigned int nBytes = *((unsigned int*)_Buffer);
		if(nBytes == 0 || nBytes > 10 * 1024 * 1024) // 10 MB
		{
			// 数据异常
			return false;
		}

		if(!buffer.AllocateBytes(nBytes))
		{
			// E_OUTOFMEMORY
			return false;
		}

		if(!CZipWrapper::MemUnZip(buffer,nBytes,_Buffer + sizeof(unsigned int),_NumberOfBytes - sizeof(unsigned int)))
		{
			// 解压缩失败
			return false;
		}

		// 创建 XML 读取器
		if(FAILED(hr = ::CreateStreamOnHGlobal(buffer,FALSE,&spXmlStream)))		
		{
			// 创建内存流失败 E_OUTOFMEMORY
			wprintf(L"Error creating memory stream, error is %08.8lx",E_OUTOFMEMORY); 
			return false;
		}

		if(FAILED(hr = CreateXmlReader(__uuidof(IXmlReader),(void**)&spReader,NULL))) 
		{ 
			wprintf(L"Error creating xml reader, error is %08.8lx", hr); 
			return false;
		} 

		if(FAILED(hr = CreateXmlReaderInputWithEncodingName(spXmlStream,NULL,L"utf-16",TRUE,NULL,&spReaderInput))) 
		{ 
			wprintf(L"Error creating xml reader with encoding name, error is %08.8lx", hr); 
			return false;
		} 

		if (FAILED(hr = spReader->SetInput(spReaderInput))) 
		{ 
			wprintf(L"Error setting input for reader, error is %08.8lx", hr); 
			return false;
		} 

		if (FAILED(hr = spReader->SetProperty(XmlReaderProperty_DtdProcessing,DtdProcessing_Prohibit))) 
		{ 
			wprintf(L"Error setting dtd processing property in reader, error is %08.8lx", hr); 
			return false;
		}

		// 读取数据
		XmlNodeType nodetype;		
		const WCHAR* pwszLocalName;
		const WCHAR* pwszValue;
		UINT cwchValue=100;

		while(S_OK == (hr = spReader->Read(&nodetype)))
		{
			switch (nodetype)
			{
			case XmlNodeType_Element:
				{
					if (FAILED(hr = spReader->GetLocalName(&pwszLocalName, NULL))) 
					{
						wprintf(L"Error get element local name in reader, error is %08.8lx", hr); 
						return false;
					}

					if(::lstrcmpW(pwszLocalName,L"Content") == 0)
					{
						//  id
						if(FAILED(hr = spReader->MoveToFirstAttribute())
							|| FAILED(hr = spReader->GetValue(&pwszValue,&cwchValue)))
						{
							wprintf(L"Error get first(id) attribute value in reader, error is %08.8lx", hr); 
							return false;
						}

						if(cwchValue > 0)
						{
							auto result = _M_set.insert(std::unordered_map<int,double>::value_type(::_wtoi(pwszValue),Con_doubleInvalid));

							// value
							if(FAILED(hr = spReader->MoveToNextAttribute())
								|| FAILED(hr = spReader->GetValue(&pwszValue,&cwchValue)))
							{
								wprintf(L"Error get second(value) attribute value in reader, error is %08.8lx", hr); 
								return false;
							}

							if(cwchValue > 0)
								result.first->second = ::_wtof(pwszValue);
						}
					}

					break;
				}
			default:
				break;
			}
		}

		return true;
	}

	//
	// fund_db_context 实现
	//
	fund_db_context::fund_db_context()
	{
	}

	fund_db_context::~fund_db_context()
	{
	}

	std::shared_ptr<fund_total_share_db_set> fund_db_context::get_total_share_db_set(int date,bool update)
	{
		std::shared_ptr<fund_total_share_db_set> dbset;

		auto it = _M_totalShares.find(date);
		if(it != _M_totalShares.end())
		{
			dbset = it->second;
		}
		else
		{
			dbset = std::make_shared<fund_total_share_db_set>(date);
			_M_totalShares.insert(std::unordered_map<int,std::shared_ptr<fund_total_share_db_set>>::value_type(date,dbset));
		}

		if(update && dbset->get_count() == 0)
			dbset->_Download();
		
		return dbset;
	}

	namespace Business
	{

		IMPLEMENT_DYNCREATE(TxFund,TxBusiness)
		//////////////////////////////////////////////////////////////////////
		// Construction/Destruction
		//////////////////////////////////////////////////////////////////////
		__declspec(selectany) fund_db_context TxFund::_M_dbContext;

		TxFund::TxFund()
		{
			for(int i=0;i<9;i++)
				varCfg[i]=i;
		}

		TxFund::~TxFund()
		{
		}

		//2007-09-18
		//板块风险分析
		bool TxFund::BlockRiskIndicatorAdvFund(
			int iMenuID,						//功能ID
			Tx::Core::Table_Indicator& resTable,//结果数据表
			std::set<int>& iSecurityId,			//交易实体ID
			int iEndDate,						//截止日期
			long lRefoSecurityId,				//基准交易实体ID
			int iStartDate,						//起始日期
			int iTradeDaysCount,				//交易天数
			int iDuration,						//交易周期0=日；1=周；2=月；3=季；4=年
			bool bLogRatioFlag,					//计算比率的方式,true=指数比率,false=简单比率
			bool bClosePrice,					//使用收盘价
			int  iStdType,						//标准差类型1=有偏，否则无偏
			int  iOffset,						//偏移
			bool bAhead,						//复权标志,true=前复权,false=后复权
			bool bUserRate,						//用户自定义年利率,否则取一年定期基准利率
			double dUserRate,					//用户自定义年利率
			bool bDayRate,						//日利率
			bool bPrw,
			bool bMean
			)
		{
			int ii = iSecurityId.size();
			if(ii<=0)
				return false;

			bool bFirstDayInDuration = false;
			/*
			//
			IndicatorWithParameterArray arr;
			//根据功能ID建立指标信息
			IndicatorFile::GetInstance()->SetIWAP(arr, iMenuID);
			//根据指标信息建立table列
			arr.BuildTableIndicator();
			//准备提取数据
			resTable.CopyColumnInfoFrom(arr.m_table_indicator);
			*/
			RiskIndicatorAdv(resTable);
#ifdef _DEBUG
			CString strTable=resTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
			resTable.DeleteCol(2);
			resTable.DeleteCol(1);

			RiskIndicatorAdv(
				resTable,				//结果数据表
				iSecurityId,			//交易实体ID
				iEndDate,				//截止日期
				lRefoSecurityId,		//基准交易实体ID
				bFirstDayInDuration,	//默认周期期初
				iStartDate,				//起始日期
				iTradeDaysCount,		//交易天数
				iDuration,				//交易周期0=日；1=周；2=月；3=季；4=年
				bLogRatioFlag,			//计算比率的方式,true=指数比率,false=简单比率
				bClosePrice,			//使用收盘价
				iStdType,				//标准差类型1=有偏，否则无偏
				iOffset,				//偏移
				bAhead,					//复权标志,true=前复权,false=后复权
				bUserRate,				//用户自定义年利率,否则取一年定期基准利率
				dUserRate,				//用户自定义年利率,否则取一年定期基准利率
				bDayRate,				//日利率
				bPrw,
				bMean
				);

			resTable.DeleteCol(2);

			return true;
		}
		//板块阶段行情[涨幅]
		//[2007-07-13]测试通过
		/*bool TxFund::BlockCycleRate(
		std::set<int>& iSecurityId,		//交易实体ID
		int date,							//数据日期
		Tx::Core::Table_Indicator& resTable	//结果数据表
		)
		{
		int ii = iSecurityId.size();
		if(ii<=0)
		return false;
		//step1
		Tx::Core::ProgressWnd* pProgressWnd = Tx::Core::ProgressWnd::GetInstance();
		//step2
		CString sProgressPrompt;
		sProgressPrompt.Format(_T("阶段表现..."));
		UINT progId = prw.AddItem(1,sProgressPrompt, 0.0);
		//step3
		prw.Show(1500);
		//step4
		prw.SetPercent(progId, 0.1);

		//默认的返回值状态
		bool result = false;
		//清空数据
		resTable.Clear();
		int iCol = 0;

		//2007-08-17
		//样本名称
		resTable.AddParameterColumn(Tx::Core::dtype_val_string);
		//样本外码
		resTable.AddParameterColumn(Tx::Core::dtype_val_string);

		//准备样本集=第一参数列:F_Security_ID,int型
		resTable.AddParameterColumn(Tx::Core::dtype_int4);
		//根据样本数量添加记录数
		resTable.AddRow(ii);
		//添加券ID
		int j=0;
		for(std::set<int>::iterator iter = iSecurityId.begin();iter!=iSecurityId.end();iter++,j++)
		//resTable.SetCell(0,j,iSecurityId[j]);
		resTable.SetCell(2,j,*iter);

		//2007-09-18
		//取得上证指数的最近交易日期
		if(m_pShIndex!=NULL)
		date = m_pShIndex->GetTradeDateOffset(date,0);

		//数据日期参数=第二参数列;F_DATE, int型
		iCol++;
		resTable.AddParameterColumn(Tx::Core::dtype_int4,true);
		resTable.SetCell(3,0,date);

		prw.SetPercent(progId, 0.3);

		long iIndicator = 30300042;	//指标=本周以来涨幅
		UINT varCfg[2];			//参数配置
		int varCount=2;			//参数个数
		for (int i = 0; i < 14; i++)
		{
		GetIndicatorDataNow(iIndicator+i);
		if (m_pIndicatorData==NULL)
		{
		prw.SetPercent(progId, 1.0);
		return false;
		}
		varCfg[0]=2;
		varCfg[1]=3;
		result = m_pLogicalBusiness->SetIndicatorIntoTable(
		m_pIndicatorData,	//指标
		varCfg,				//参数配置
		varCount,			//参数个数
		resTable	//计算需要的参数传输载体以及计算后结果的载体
		);
		if(result==false)
		break;
		}
		prw.SetPercent(progId, 0.5);

		//根据之前3个步骤的设置进行数据读取，结果数据存放在table中
		result = m_pLogicalBusiness->GetData(resTable);
		if(result==false)
		{
		prw.SetPercent(progId, 1.0);
		return false;
		}

		//resTable.DeleteCol(2);
		j=0;
		for(std::set<int>::iterator iter = iSecurityId.begin();iter!=iSecurityId.end();iter++,j++)
		{
		GetSecurityNow(*iter);
		if(m_pSecurity!=NULL)
		{
		//填充名称
		resTable.SetCell(0,j,m_pSecurity->GetName());
		//填充外码
		resTable.SetCell(1,j,m_pSecurity->GetCode());
		resTable.SetCell(2,j,(int)m_pSecurity->GetId());
		}
		}
		prw.SetPercent(progId, 1.0);
		sProgressPrompt+=_T(",完成!");
		prw.SetText(progId, sProgressPrompt);

		return true;
		}
		*/
		//通用统计接口
		bool TxFund::StatCommon(
			int iMenuID,						//功能ID
			Tx::Core::Table_Indicator& resTable,//结果数据表
			std::vector<int>& iSecurityId,		//交易实体ID
			std::vector<Tx::Core::VariantData> arrParam,	//参数
			bool bFullData						//统计所有
			)
		{
			//清空
			//m_IWPA.clear();
			//
			////根据功能ID建立指标信息
			//IndicatorFile::GetInstance()->SetIWAP(m_IWPA, iMenuID);
			//if(bFullData==false)
			//{
			//	//设置参数
			//	for(int i=0;i<(int)m_IWPA.size();i++)
			//		IndicatorFile::GetInstance()->SetParameter(m_IWPA,i,arrParam);
			//}
			////提取数据
			//IndicatorFile::GetInstance()->GetData(m_IWPA,iSecurityId,bFullData);

			Tx::Core::Table_Indicator m_txTable;
			m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
			m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
			m_txTable.AddParameterColumn(Tx::Core::dtype_int4);

			const int INDICATOR_INDEX=8;
			int varCount=3;
			long iIndicator[INDICATOR_INDEX];

			for(int i=0;i<INDICATOR_INDEX;i++)
			{			
				iIndicator[i]=30901222+i;
			};
			bool result;
			//设定指标列
			for (int i = 0; i <	INDICATOR_INDEX; i++)
			{
				GetIndicatorDataNow(iIndicator[i]);
				result = m_pLogicalBusiness->SetIndicatorIntoTable(
					m_pIndicatorData,	//指标
					varCfg,				//参数配置
					varCount,			//参数个数
					m_txTable			//计算需要的参数传输载体以及计算后结果的载体
					);
				if(result==false)
					break;
			}

			result=m_pLogicalBusiness->GetData(m_txTable,true);
			//复制数据
			resTable.CopyColumnInfoFrom(m_txTable);
			resTable.Clone(m_txTable);
			this->IdColToNameAndCode(resTable,0,1);
			return true;
		}
		//货币基金期限配置
		//add by lijw 2008-03-28
		bool TxFund::StatCurrencyFundSurplusTerm(
			Tx::Core::Table_Indicator& resTable,//结果数据表
			std::vector<int>& iSecurityId,		//交易实体ID
			std::vector<int>  iDate
			)
		{	
			std::set<int> setSecurityId;
			std::set<int> SameFundId;
			std::set<int> DifferentID;
			std::vector<int>::iterator iter;
			std::vector<int> TradeV;
			bool Result = EliminateSample(iSecurityId,TradeV,setSecurityId);
			if(Result == false)
				return false;
			DifferentID.insert(TradeV.begin(),TradeV.end());
			/*for(iter = iSecurityId.begin();iter != iSecurityId.end();++iter)
			{
			GetSecurityNow(*iter);
			if(m_pSecurity == NULL)
			continue;
			int fundid =(int)m_pSecurity->GetSecurity1Id();
			if(setSecurityId.find(fundid) == setSecurityId.end())
			{
			setSecurityId.insert(fundid);
			DifferentID.insert(*iter);
			}
			else
			SameFundId.insert(*iter);
			}*/
			ProgressWnd prw;
			UINT pid=prw.AddItem(1,_T("货币基金期限配置..."),0.0);
			prw.Show(1000);

			//从T_ASSET_ALLOCATION_twoyear里取基金定期报告ID，基金ID，报告年份，报告期
			//默认的返回值状态
			bool result = false;
			//清空数据
			m_txTable.Clear();
			//准备样本集参数列
			m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//基金ID
			m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//报告年份
			m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//报告期
			const int indicatorIndex = 2;
			long iIndicator[indicatorIndex] = 
			{
				30901332,	//基金定期报告ID//这是T_ASSET_ALLOCATION表里
				30901140	//资产净值
			};
			UINT varCfg[3];			//参数配置
			int varCount=3;			//参数个数
			for (int i = 0; i < indicatorIndex; i++)
			{
				int tempIndicator = iIndicator[i];

				GetIndicatorDataNow(tempIndicator);
				if (m_pIndicatorData==NULL)
				{ return false; }
				varCfg[0]=0;
				varCfg[1]=1;
				varCfg[2]=2;
				result = m_pLogicalBusiness->SetIndicatorIntoTable(
					m_pIndicatorData,	//指标
					varCfg,				//参数配置
					varCount,			//参数个数
					m_txTable	//计算需要的参数传输载体以及计算后结果的载体
					);
				if(result==false)
				{
					return FALSE;
				}

			}
			UINT iColCount = m_txTable.GetColCount();
			UINT* nColArray = new UINT[iColCount];
			for(int i = 0; i < (int)iColCount; i++)
			{
				nColArray[i] = i;
			}
			result = m_pLogicalBusiness->GetData(m_txTable,true);
#ifdef _DEBUG
			CString strTable=m_txTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
			////把交易实体ID转化为基金ID
			//std::vector<int> iSecurity1Id;
			//if(!TransObjectToSecIns(iSecurityId,iSecurity1Id,1))
			//{
			//	delete nColArray;
			//	nColArray = NULL;
			//	return false;	
			//}
			Tx::Core::Table_Indicator tempTable;
			tempTable.CopyColumnInfoFrom(m_txTable);
			//根据基金ID进行筛选
			m_txTable.EqualsAt(tempTable,nColArray,iColCount,0,setSecurityId);
			//为了进行年度和报告期的筛选，所以把报告年份和报告期和为一列。
			TransReportDateToNormal2(tempTable,1);
#ifdef _DEBUG
			strTable=tempTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
			m_txTable.Clear();
			m_txTable.CopyColumnInfoFrom(tempTable);
			//进行年度和报告期的筛选
			tempTable.EqualsAt(m_txTable,nColArray,iColCount-1,1,iDate);
#ifdef _DEBUG
			strTable=m_txTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
			//取出基金定期报告ID，把它放到vector里，为和从视图T_STOCK_HOLDING_TOP_TEN_twoyear取得数据进行连接作准备。
			std::vector<int> ReportV;
			int reportid ;
			for(int i = 0;i < (int)m_txTable.GetRowCount();i++)
			{		
				m_txTable.GetCell(2,i,reportid);
				ReportV.push_back(reportid);
			}
			delete nColArray;
			nColArray = NULL;
			if (ReportV.empty())
			{
				//添加进度条
				prw.SetPercent(pid,1.0);
				return false;
			}

			tempTable.Clear();
			//从T_BOND_HOLDING_TERM_DISTRIBUTION取其他的数据
			//准备样本集参数列
			tempTable.AddParameterColumn(Tx::Core::dtype_int4);//基金ID
			tempTable.AddParameterColumn(Tx::Core::dtype_int4);//报告年份
			tempTable.AddParameterColumn(Tx::Core::dtype_int4);//报告期
			const int indicatorIndex3 = 7;
			long iIndicator3[indicatorIndex3] = 
			{
				30901353,//基金定期报告ID
				30901229,//平均剩余期限
				30901222,//30天以内
				30901223,//30(含)-60天
				30901224,//60(含)-90天
				30901225,//90(含)-180天
				30901226 //180天以上
			};
			UINT varCfg3[3];			//参数配置
			int varCount3=3;			//参数个数
			for (int i = 0; i < indicatorIndex3; i++)
			{
				int tempIndicator = iIndicator3[i];

				GetIndicatorDataNow(tempIndicator);
				if (m_pIndicatorData==NULL)
				{ return false; }
				varCfg3[0]=0;
				varCfg3[1]=1;
				varCfg3[2]=2;
				result = m_pLogicalBusiness->SetIndicatorIntoTable(
					m_pIndicatorData,	//指标
					varCfg3,				//参数配置
					varCount3,			//参数个数
					tempTable	//计算需要的参数传输载体以及计算后结果的载体
					);
				if(result==false)
				{ 
					return false;
				}
			}
			result = m_pLogicalBusiness->GetData(tempTable,true);
#ifdef _DEBUG
			strTable=tempTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
			UINT iColCount3 = tempTable.GetColCount();
			UINT* nColArray3 = new UINT[iColCount3];
			for(int i = 0; i < (int)iColCount3; i++)
			{
				nColArray3[i] = i;
			}
			resTable.CopyColumnInfoFrom(tempTable);
			tempTable.EqualsAt(resTable,nColArray3,iColCount3,3,ReportV);
			if(resTable.GetRowCount() == 0)
			{
				//添加进度条
				prw.SetPercent(pid,1.0);
				delete nColArray3;
				nColArray3 = NULL;
				return false;
			}
			delete nColArray3;
			nColArray3 = NULL;
			//为了把它转化为CString类型，所以把报告年份和报告期和为一列。
			TransReportDateToNormal2(resTable,1);	
#ifdef _DEBUG
			strTable=resTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif	
			//添加进度条
			prw.SetPercent(pid,0.6);
			//为增加基金的交易实体ID和名称，代码作准备
			resTable.InsertCol(0,Tx::Core::dtype_int4);//基金交易实体ID
			resTable.InsertCol(1,Tx::Core::dtype_val_string);//基金名称
			resTable.InsertCol(2,Tx::Core::dtype_val_string);//基金代码
			//为把m_txTable的内容放到resTable表里作准备。
			resTable.InsertCol(5,Tx::Core::dtype_decimal);//资产净值
			double equity2;
			std::vector<UINT>vecID1;
			std::vector<UINT>::iterator iteID1;
			int position;
			for(int m = 0;m < (int)m_txTable.GetRowCount();m++)
			{
				m_txTable.GetCell(2,m,reportid);
				m_txTable.GetCell(3,m,equity2);
				if(!vecID1.empty())
					vecID1.clear();
				resTable.Find(6,reportid,vecID1);//查找相等的基金定期报告ID
				for(iteID1 = vecID1.begin();iteID1 != vecID1.end();++iteID1)
				{
					position = *iteID1;
					resTable.SetCell(5,position,equity2);			
				}			
			}
#ifdef _DEBUG
			strTable=resTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
			//把报告年份和报告期转化成CString类型,所以增加一列
			resTable.InsertCol(5,Tx::Core::dtype_val_string);//str报告期
			//增加基金的交易实体ID和名称，代码
			int iRow,ReDate;
			CString strdate,strLeft,strRight;
			std::set<int>::iterator iterId;
			for(iterId = DifferentID.begin();iterId != DifferentID.end();++iterId)
			{
				int fundId  ;
				CString strName,strCode;
				GetSecurityNow(*iterId);
				if(m_pSecurity == NULL)
					continue;
				fundId = m_pSecurity->GetSecurity1Id();
				strName = m_pSecurity->GetName();
				strCode = m_pSecurity->GetCode();
				std::vector<UINT> vecInstiID;
				resTable.Find(3,fundId,vecInstiID);
				std::vector<UINT>::iterator iteID;
				for(iteID = vecInstiID.begin();iteID != vecInstiID.end();++iteID)
				{
					iRow = *iteID;
					resTable.SetCell(0,iRow,*iterId);
					resTable.SetCell(1,iRow,strName);
					resTable.SetCell(2,iRow,strCode);
					//得到日期把它们转化为报告期
					resTable.GetCell(4,iRow,ReDate);
					strdate.Format(_T("%d"),ReDate);
					strLeft = strdate.Left(4);
					strRight = strdate.Right(4);
					if(strRight == "0331")
						strdate = strLeft + _T("年") + _T("一季报");
					if(strRight == "0630")
						strdate = strLeft + _T("年") + _T("二季报");
					if(strRight == "0930")
						strdate = strLeft + _T("年") + _T("三季报");
					if(strRight == "1231")
						strdate = strLeft + _T("年") + _T("四季报");
					resTable.SetCell(5,iRow,strdate);
				}

			}
			//	//为计算方便，以基金ID进行排序
			//	resTable.Sort(3);
			//	if(SameFundId.size() != 0)
			//	{
			//		//为不同的交易实体ID转化为相同的基金ID那样的样本填写数据，
			//		std::vector<UINT> vecFundID;
			//		std::set<int>::iterator iterSet,tempIter;
			//		for(iterSet = SameFundId.begin();iterSet != SameFundId.end();++iterSet)
			//		{
			//			int tempfundid ;
			//			GetSecurityNow(*iterSet);
			//			if(m_pSecurity == NULL)
			//				continue;
			//			tempfundid = m_pSecurity->GetSecurity1Id();//取得基金ID
			//			CString fundname,fundcode;
			//			fundname = m_pSecurity->GetName();
			//			fundcode = m_pSecurity->GetCode();
			//			resTable.Find(3,tempfundid,vecFundID);
			//			if(vecFundID.size() == 0)
			//				continue;
			//			//取得在表中最小的位置
			//			std::set<int> tempset(vecFundID.begin(),vecFundID.end());
			//			tempIter = tempset.begin();
			//			//增加相同的记录
			//			std::set<int>::size_type icount = tempset.size();
			//			resTable.InsertRow(*tempIter,icount);
			//			CString reportdate;
			//			double data0,data1,data2,data3,data4,data5,data6;
			//			int position1,position2;
			//			for(;tempIter != tempset.end();++tempIter)
			//			{
			//				position1 = *tempIter;
			//				position2 = position1 + icount;
			//				resTable.SetCell(0,position1,*iterSet);
			//				resTable.SetCell(1,position1,fundname);
			//				resTable.SetCell(2,position1,fundcode);
			//				//取得其他数据
			//				resTable.GetCell(5,position2,reportdate);//报告期
			//				resTable.GetCell(6,position2,data0);//资产净值
			//				resTable.GetCell(8,position2,data1);//平均剩余期限
			//				resTable.GetCell(9,position2,data2);//30天以内
			//				resTable.GetCell(10,position2,data3);//30-60
			//				resTable.GetCell(11,position2,data4);//60-90
			//				resTable.GetCell(12,position2,data5);//90-180
			//				resTable.GetCell(13,position2,data6);//180
			//				//填充其他数据
			//				resTable.SetCell(5,position1,reportdate);
			//				resTable.SetCell(6,position1,data0);
			//				resTable.SetCell(8,position1,data1);
			//				resTable.SetCell(9,position1,data2);
			//				resTable.SetCell(10,position1,data3);
			//				resTable.SetCell(11,position1,data4);
			//				resTable.SetCell(12,position1,data5);
			//				resTable.SetCell(13,position1,data6);
			//			}
			//			vecFundID.clear();
			//			tempset.clear();
			//		}
			//	}
			//#ifdef _DEBUG
			//	CString strTable4=resTable.TableToString();
			//	Tx::Core::Commonality::String().StringToClipboard(strTable4);
			//#endif
			resTable.DeleteCol(7);
			resTable.DeleteCol(3);//删除基金ID和报告期(int)。
			resTable.Arrange();
			MultiSortRule multisort;
			multisort.AddRule(3,false);
			multisort.AddRule(2,true);
			multisort.AddRule(5,true);			
			resTable.SortInMultiCol(multisort);
			resTable.Arrange();
			resTable.DeleteCol(3);
			//添加进度条
			prw.SetPercent(pid,1.0);
			return true;
		}

		//add by lijw 2008-02-21
		//重仓股集中度统计
		bool TxFund::StatFundZCGjzd(
			int iMenuID,						//功能ID
			Tx::Core::Table_Indicator& resTable,//结果数据表
			std::vector<int>& iSecurityId,		//交易实体ID
			std::vector<int> vDate				//报告期
			)
		{
			//添加进度条
			// ProgressWnd* pwd=Tx::Core::ProgressWnd::GetInstance();
			ProgressWnd prw;
			UINT pid=prw.AddItem(1,_T("重仓股集中度统计..."),0.0);
			prw.Show(1000);
			m_txTable.Clear();//这是引用别人的成员变量，
			//从T_ASSET_ALLOCATION_twoyear里取基金定期报告ID，基金ID，报告年份，报告期
			//默认的返回值状态
			bool result = false;
			//清空数据
			m_txTable.Clear();
			//准备样本集参数列
			m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//基金ID
			m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//报告年份
			m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//报告期
			const int indicatorIndex = 2;
			long iIndicator[indicatorIndex] = 
			{
				30901332,	//基金定期报告ID//这是T_ASSET_ALLOCATION表里
				30901131	//股票市值
			};
			UINT varCfg[3];			//参数配置
			int varCount=3;			//参数个数
			for (int i = 0; i < indicatorIndex; i++)
			{
				int tempIndicator = iIndicator[i];

				GetIndicatorDataNow(tempIndicator);
				if (m_pIndicatorData==NULL)
				{ return false; }
				varCfg[0]=0;
				varCfg[1]=1;
				varCfg[2]=2;
				result = m_pLogicalBusiness->SetIndicatorIntoTable(
					m_pIndicatorData,	//指标
					varCfg,				//参数配置
					varCount,			//参数个数
					m_txTable	//计算需要的参数传输载体以及计算后结果的载体
					);
				if(result==false)
				{
					return FALSE;
				}

			}
			UINT iColCount = m_txTable.GetColCount();
			UINT* nColArray = new UINT[iColCount];
			for(int i = 0; i < (int)iColCount; i++)
			{
				nColArray[i] = i;
			}
			result = m_pLogicalBusiness->GetData(m_txTable,true);
#ifdef _DEBUG
			CString strTable=m_txTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
			//把交易实体ID转化为基金ID
			std::vector<int> iSecurity1Id;
			std::vector<int>::iterator iterVector;
			int tempId;
			std::vector<int> tempTradeV;
			//bool Result = EliminateSample(iSecurityId,tempTradeV,iSecurity1Id);
			//if(Result == false)
			//{
			//	//添加进度条
			//	prw.SetPercent(pid,1.0);
			//	return false;
			//}
			for (iterVector = iSecurityId.begin();iterVector != iSecurityId.end();++iterVector)
			{
				GetSecurityNow(*iterVector);
				if (m_pSecurity != NULL)
				{
					tempId = m_pSecurity->GetSecurity1Id();
					if(find(iSecurity1Id.begin(),iSecurity1Id.end(),tempId) == iSecurity1Id.end())
					{
						iSecurity1Id.push_back(tempId);
						tempTradeV.push_back(*iterVector);
					}
				}
			}
			iSecurityId.clear();
			iSecurityId.assign(tempTradeV.begin(),tempTradeV.end());
			Tx::Core::Table_Indicator tempTable;
			tempTable.CopyColumnInfoFrom(m_txTable);
			//根据基金ID进行筛选
			m_txTable.EqualsAt(tempTable,nColArray,iColCount,0,iSecurity1Id);
			if (tempTable.GetRowCount() == 0)
			{
				delete nColArray;
				nColArray = NULL;
				//添加进度条
				prw.SetPercent(pid,1.0);
				return false;
			}
			//为了进行年度和报告期的筛选，所以把报告年份和报告期和为一列。
			TransReportDateToNormal2(tempTable,1);
#ifdef _DEBUG
			strTable=tempTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
			m_txTable.Clear();
			m_txTable.CopyColumnInfoFrom(tempTable);
			//进行年度和报告期的筛选
			tempTable.EqualsAt(m_txTable,nColArray,iColCount-1,1,vDate);
			if (m_txTable.GetRowCount() == 0)
			{
				delete nColArray;
				nColArray = NULL;
				//添加进度条
				prw.SetPercent(pid,1.0);
				return false;
			}
#ifdef _DEBUG
			strTable=m_txTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
			//取出基金定期报告ID，把它放到vector里，为和从视图T_STOCK_HOLDING_TOP_TEN_twoyear取得数据进行连接作准备。
			std::set<int> ReportV;
			for(int i = 0;i < (int)m_txTable.GetRowCount();i++)
			{
				int reportid ;
				m_txTable.GetCell(2,i,reportid);
				ReportV.insert(reportid);
			}
			delete nColArray;
			nColArray = NULL;


			//从T_STOCK_HOLDING_TOP_TEN_twoyear取其他的数据
			Tx::Core::Table_Indicator hbTable;
			//准备样本集参数列
			hbTable.AddParameterColumn(Tx::Core::dtype_int4);//基金ID
			hbTable.AddParameterColumn(Tx::Core::dtype_int4);//报告年份
			hbTable.AddParameterColumn(Tx::Core::dtype_int4);//报告期
			hbTable.AddParameterColumn(Tx::Core::dtype_byte);//F1序号
			const int indicatorIndex3 = 4;
			long iIndicator3[indicatorIndex3] = 
			{
				30901329,	//基金定期报告ID//这是T_STOCK_HOLDING_TOP_TEN表里
				30901310,	//市值排名
				30901239,	//股票市值
				30901240	//所占比例（即是占净值比例）
			};
			UINT varCfg3[4];			//参数配置
			int varCount3=4;			//参数个数
			for (int i = 0; i < indicatorIndex3; i++)
			{
				int tempIndicator = iIndicator3[i];

				GetIndicatorDataNow(tempIndicator);
				if (m_pIndicatorData==NULL)
				{ return false; }
				varCfg3[0]=0;
				varCfg3[1]=1;
				varCfg3[2]=2;
				varCfg3[3]=3;
				result = m_pLogicalBusiness->SetIndicatorIntoTable(
					m_pIndicatorData,	//指标
					varCfg3,				//参数配置
					varCount3,			//参数个数
					hbTable	//计算需要的参数传输载体以及计算后结果的载体
					);
				if(result==false)
				{
					//添加进度条
					prw.SetPercent(pid,1.0);
					return false;
				}

			}

			result = m_pLogicalBusiness->GetData(hbTable,true);
			hbTable.DeleteCol(3);//删除序号列，之所以在删掉，是因为不想改下面的代码。
#ifdef _DEBUG
			strTable=hbTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
			UINT iColCount3 = hbTable.GetColCount();
			UINT* nColArray3 = new UINT[iColCount3];
			for(int i = 0; i < (int)iColCount3; i++)
			{
				nColArray3[i] = i;
			}
			resTable.CopyColumnInfoFrom(hbTable);
			hbTable.EqualsAt(resTable,nColArray3,iColCount3,3,ReportV);
			delete nColArray3;
			nColArray3 = NULL;
			if(resTable.GetRowCount() == 0)
			{
				//添加进度条
				prw.SetPercent(pid,0.3);
				return false;
			}
#ifdef _DEBUG
			strTable=resTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif

			//添加进度条
			prw.SetPercent(pid,0.3);
			//为增加基金的交易实体ID和名称，代码作准备
			resTable.InsertCol(0,Tx::Core::dtype_int4);//基金交易实体ID
			resTable.InsertCol(1,Tx::Core::dtype_val_string);//基金名称
			resTable.InsertCol(2,Tx::Core::dtype_val_string);//基金代码
			//为把m_txTable的内容放到resTable表里作准备。
			resTable.InsertCol(3,Tx::Core::dtype_decimal);//股票市值
			for(int m = 0;m < (int)m_txTable.GetRowCount();m++)
			{
				int reId2;
				double equity2;
				m_txTable.GetCell(2,m,reId2);
				m_txTable.GetCell(3,m,equity2);
				std::vector<UINT> vecInstiID;
				resTable.Find(7,reId2,vecInstiID);
				std::vector<UINT>::iterator iteID;
				for(iteID = vecInstiID.begin();iteID != vecInstiID.end();++iteID)
				{			
					//把占净值比例乘以100
					double dEquity;
					resTable.GetCell(10,*iteID,dEquity);
					dEquity = dEquity*100;
					resTable.SetCell(10,*iteID,dEquity);
					resTable.SetCell(3,*iteID,equity2);
				}

			}
#ifdef _DEBUG
			strTable=resTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
			//增加基金的交易实体ID和名称，代码
			std::vector<int>::iterator iterId;
			for(iterId = iSecurityId.begin();iterId != iSecurityId.end();++iterId)
			{
				int fundId  ;
				CString strName,strCode;
				GetSecurityNow(*iterId);
				if(m_pSecurity == NULL)
					continue;
				fundId = m_pSecurity->GetSecurity1Id();
				strName = m_pSecurity->GetName();
				strCode = m_pSecurity->GetCode();
				std::vector<UINT> vecInstiID;
				resTable.Find(4,fundId,vecInstiID);
				std::vector<UINT>::iterator iteID;
				for(iteID = vecInstiID.begin();iteID != vecInstiID.end();++iteID)
				{
					resTable.SetCell(0,*iteID,*iterId);
					resTable.SetCell(1,*iteID,strName);
					resTable.SetCell(2,*iteID,strCode);
				}

			}
			//把报告年份和报告期转化成CString类型,所以增加一列
			resTable.InsertCol(7,Tx::Core::dtype_val_string);//str报告期
			//把报告年份和报告期转化为CString类型
			TransReportDateToNormal3(resTable,5);//减少两列，分别是报告年份、报告期（int）
			resTable.DeleteCol(7);//基金定期报告ID
			resTable.DeleteCol(4);//因为已经把交易实体ID写在表里了，所以这里，可以把基金ID去掉。

			//把第五列报告期转化为日期 add by wangyc 20091104 begin
			for(UINT i=0;i<resTable.GetRowCount();i++)
			{
				int iReportDate,iYear,iDay;
				resTable.GetCell(4,i,iReportDate);
				iYear = iReportDate/10000;
				iReportDate %= 10;
				switch(iReportDate)
				{
				case 1:
					iDay=331;
					break;
				case 2:
					iDay=630;
					break;
				case 4:
					iDay=930;
					break;
				case 6:
					iDay=1231;
					break;
				case 3:
					iDay=630;
					break;
				case 9:
					iDay=1231;
					break;
				default:
					break;
				}

				resTable.SetCell(4,i,iYear*10000+iDay);
			}
			//把第五列报告期转化为日期 add by wangyc 20091104 begin

#ifdef _DEBUG
			strTable=resTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif

			UINT iColCount4 = resTable.GetColCount();
			UINT* nColArray4 = new UINT[iColCount4];
			for(int i = 0; i < (int)iColCount4; i++)
			{
				nColArray4[i] = i;
			}
			Tx::Core::Table_Indicator tempTable2,tempTable3,tempTable4;
			//	tempTable2.CopyColumnInfoFrom(resTable);
			//	tempTable3.CopyColumnInfoFrom(resTable);
			//因为tempTable4是存放临时结果，所以重新加列
			tempTable4.AddCol(Tx::Core::dtype_int4);//交易实体
			tempTable4.AddCol(Tx::Core::dtype_val_string);//基金名称
			tempTable4.AddCol(Tx::Core::dtype_val_string);//基金代码
			tempTable4.AddCol(Tx::Core::dtype_int4);//报告期
			tempTable4.AddCol(Tx::Core::dtype_val_string);//报告期
			tempTable4.AddCol(Tx::Core::dtype_decimal);//
			tempTable4.AddCol(Tx::Core::dtype_decimal);
			tempTable4.AddCol(Tx::Core::dtype_decimal);
			tempTable4.AddCol(Tx::Core::dtype_decimal);
			tempTable4.AddCol(Tx::Core::dtype_decimal);
			tempTable4.AddCol(Tx::Core::dtype_decimal);
			tempTable4.AddCol(Tx::Core::dtype_decimal);
			tempTable4.AddCol(Tx::Core::dtype_decimal);
			std::vector<int> iTradeId;
			//记录行数
			static int icount = -1;
			std::vector<int>::iterator tempDate;
			for (tempDate=vDate.begin();tempDate!= vDate.end();tempDate++)
			{
				std::vector<int> iReportDate;
				iReportDate.clear();
				iReportDate.push_back(*tempDate);

				Tx::Core::Table_Indicator tempTable5;
				tempTable5.Clear();
				tempTable5.CopyColumnInfoFrom(resTable);
				//根据基金ID进行筛选
				resTable.EqualsAt(tempTable5,nColArray4,iColCount4,4,iReportDate);

				//根据样本，对每一个样本都计算占股票投资市值比和占净值比。
				for(iterId = iSecurityId.begin();iterId != iSecurityId.end();++iterId)
				{
					tempTable2.Clear();
					tempTable2.CopyColumnInfoFrom(tempTable5);
					//是为了得到一个样本的记录。
					iTradeId.clear();
					iTradeId.push_back(*iterId);
					//通过交易实体进行筛选
					tempTable5.EqualsAt(tempTable2,nColArray4,iColCount4,0,iTradeId);
					//如果在前面把有交易实体的全部选出来时，下面的判断就可以不要。
					if(tempTable2.GetRowCount() == 0)
						continue;
#ifdef _DEBUG
					strTable=tempTable2.TableToString();
					Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
					//tempTable4增加一行
					tempTable4.AddRow();
					icount++;
					//股票市值
					double stockValue = 0;
					//--占净值的比例
					double jzbl = 0;

					for(int jj=0;jj<4;jj++)
					{
						int iStartIndex = 1;
						int iEndIndex = 1;
						switch(jj)
						{
						case 0:
							//前两大重仓股 = 股票投资[0]+股票投资[1]
							iEndIndex = 2;
							break;
						case 1:
							//前三大重仓股 = 前两大重仓股+股票投资[2];
							iEndIndex = 3;
							break;
						case 2:
							//前五大重仓股 = 前三大重仓股+股票投资[3]+股票投资[4]
							iEndIndex = 5;
							break;
						case 3:
							//前十大重仓股 = 前五大重仓股+股票投资[5]+...股票投资[9]
							iEndIndex = 10;
							break;
						}
						tempTable3.Clear();
						tempTable3.CopyColumnInfoFrom(tempTable2);
						if(tempTable2.GetRowCount()>0)
							tempTable2.Between(tempTable3,nColArray4,iColCount4,6,iStartIndex,iEndIndex,true,true);
#ifdef _DEBUG
						strTable=tempTable3.TableToString();
						Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
						//取每支基金中的股票市值
						double dEquity3;
						int id,tempdate;
						CString strName,strCode,strDate;
						//之所以这样取，是因为每支基金名称代码，股票在一支基金中的总值对于他们中的股票来说是一定的。
						tempTable3.GetCell(0,0,id);
						tempTable3.GetCell(1,0,strName);
						tempTable3.GetCell(2,0,strCode);
						tempTable3.GetCell(3,0,dEquity3);
						tempTable3.GetCell(4,0,tempdate);
						tempTable3.GetCell(5,0,strDate);
						//循环处理每条记录
						double vv = 0;
						jzbl = 0;
						stockValue = 0;
						for(int ii=0;ii<(int)tempTable3.GetRowCount();ii++)
						{
							//取得占净值比例
							tempTable3.GetCell(8,ii,vv);
							if((vv - Tx::Core::Con_doubleInvalid)>0.000001)
							{
								jzbl += vv;
							}
							else
							{
								TRACE(_T("\n市值比例:%s,%s,%.4f,%.3f"),strName,strCode,jzbl,vv);
							}

							//取得股票市值
							tempTable3.GetCell(7,ii,vv);
							if((vv - Tx::Core::Con_doubleInvalid)>0.000001)
							{
								stockValue += vv;
							}
							else
							{
								TRACE(_T("\n股票市值之和:%s,%s,%.4f,%.3f"),strName,strCode,stockValue,vv);
							}
						}

						//插入数据到table
						//添加进度条
						prw.SetPercent(pid,0.6);
						//计算比例
						tempTable4.SetCell(0,icount,id);
						tempTable4.SetCell(1,icount,strName);
						tempTable4.SetCell(2,icount,strCode);
						tempTable4.SetCell(3,icount,tempdate);
						tempTable4.SetCell(4,icount,strDate);

						int iCol = 5;
						int tempCol = iCol+jj*2;
						tempTable4.SetCell(tempCol,icount,jzbl);
						double dTemp = stockValue/dEquity3*100;
						tempTable4.SetCell(tempCol + 1,icount,dTemp);
					}

				}
			}

			resTable.Clear();
			resTable.CopyColumnInfoFrom(tempTable4);
			resTable.Clone(tempTable4);
			//这是保证第二次运行的时候，icount是从0开始的。
			icount = -1;
			delete nColArray4;
			nColArray4 = NULL;
			MultiSortRule multisort;
			multisort.AddRule(3,false);
			multisort.AddRule(2,true);	
			resTable.SortInMultiCol(multisort);
			resTable.Arrange();
			resTable.DeleteCol(3);
			//添加进度条
			prw.SetPercent(pid,1.0);
			return true;
		}
		////重仓股集中度统计
		//bool TxFund::StatFundZCGjzd(
		//							int iMenuID,						//功能ID
		//							Tx::Core::Table_Indicator& resTable,//结果数据表
		//							std::vector<int>& iSecurityId,		//交易实体ID
		//							std::vector<int> vDate				//报告期
		//							)
		//{
		//	//没有样本，立即返回
		//	if(iSecurityId.size()<=0)
		//		return false;
		//
		//	//由于不再需要计算比例，所以不用取得投资组合数据
		//	//取得合并重仓股
		//	Tx::Core::Table_Indicator resTable_hb;
		//	////交易实体名称
		//	//resTable_hb.AddParameterColumn(Tx::Core::dtype_val_string);
		//	////交易实体外码[代码]
		//	//resTable_hb.AddParameterColumn(Tx::Core::dtype_val_string);
		//
		//	//准备样本集=第1参数列:iSecurityId,int型
		//	resTable_hb.AddParameterColumn(Tx::Core::dtype_int4);
		//	//准备样本集=第2参数列:iFiscalYear,int型
		//	resTable_hb.AddParameterColumn(Tx::Core::dtype_int4);
		//	//准备样本集=第3参数列:iFiscalQuarter,int型
		//	resTable_hb.AddParameterColumn(Tx::Core::dtype_int4);
		//	//准备样本集=第4参数列:序号,int型
		//	resTable_hb.AddParameterColumn(Tx::Core::dtype_byte);
		//	//col=4
		//	//股票ID
		//	//col=5
		//	//股票市值
		//
		//	int iIndicator = 0;		//指标=
		//	UINT varCfg1[4];	//参数配置
		//	int varCount1=4;	//参数个数
		//
		//	//设置指标需要的参数列
		//	varCfg1[0]=0;
		//	varCfg1[1]=1;
		//	varCfg1[2]=2;
		//	varCfg1[3]=3;
		//
		//	//循环处理指标列
		//	bool result = false;
		//	int i=0;
		//	//合并
		//	iIndicator = 30901240;
		//
		//	//积极2007-12-07 涂暂时使用
		//	//	iIndicator = 30901234;//所占比例  备注：delete by lijw 2008-02-21
		//
		//	//所占市值比例
		//	for (i = 0; i < 1; i++)
		//	{
		//		GetIndicatorDataNow(iIndicator+i);
		//
		//		if (m_pIndicatorData==NULL)
		//			return false;
		//		result = m_pLogicalBusiness->SetIndicatorIntoTable(
		//			m_pIndicatorData,	//指标
		//			varCfg1,			//参数配置
		//			varCount1,			//参数个数
		//			resTable_hb	//计算需要的参数传输载体以及计算后结果的载体
		//			);
		//		if(result==false)
		//			break;
		//	}
		//	if(result==false)
		//		return false;
		//
		//	iIndicator = 30901310;
		//
		//	//积极2007-12-07 涂暂时使用
		//	//	iIndicator = 30901304;             备注：delete by lijw 2008-02-21
		//
		//	//占流通股比例
		//	for (i = 0; i < 3; i++)
		//	{
		//		if(i==1)
		//			continue;
		//		GetIndicatorDataNow(iIndicator+i);
		//
		//		if (m_pIndicatorData==NULL)
		//			return false;
		//		result = m_pLogicalBusiness->SetIndicatorIntoTable(
		//			m_pIndicatorData,	//指标
		//			varCfg1,			//参数配置
		//			varCount1,			//参数个数
		//			resTable_hb	//计算需要的参数传输载体以及计算后结果的载体
		//			);
		//		if(result==false)
		//			break;
		//	}
		//	if(result==false)
		//		return false;
		//
		//	//取得所有数据
		//	result = m_pLogicalBusiness->GetData(resTable_hb,true);
		//	if(result==false)
		//		return false;
		//#ifdef _DEBUG
		//	CString strTable=resTable_hb.TableToString();
		//	Tx::Core::Commonality::String().StringToClipboard(strTable);
		//#endif
		//	std::vector<int>::iterator iterDate;
		//	int iFiscalYear,iFiscalQuarter;
		//	Tx::Core::Table_Indicator dbTable,wholeTable;
		//	if(resTable_hb.GetRowCount()!=0)
		//		dbTable.Clone(resTable_hb);
		//	else
		//		dbTable.CopyColumnInfoFrom(resTable_hb);
		//	for(iterDate=vDate.begin();iterDate!=vDate.end();iterDate++)
		//	{
		//		if(dbTable.GetRowCount()!=0)
		//			resTable_hb.Clone(dbTable);
		//		else
		//			resTable_hb.CopyColumnInfoFrom(dbTable);
		//		iFiscalYear=*iterDate/10000;
		//		switch(*iterDate%10000)
		//		{
		//		case 331:
		//			iFiscalQuarter=40040001;
		//			break;
		//		case 630:
		//			iFiscalQuarter=40040002;
		//			break;
		//		case 930:
		//			iFiscalQuarter=40040004;
		//			break;
		//		case 1231:
		//			iFiscalQuarter=40040006;
		//			break;
		//		default:
		//			break;
		//		}
		//
		//		//step1
		//		Tx::Core::ProgressWnd* pProgressWnd = Tx::Core::ProgressWnd::GetInstance();
		//		//step2
		//		CString sProgressPrompt;
		//		sProgressPrompt.Format(_T("重仓股集中度统计..."));
		//		UINT progId = prw.AddItem(1,sProgressPrompt, 0.0);
		//		//step3
		//		prw.Show(15);
		//		//step4
		//		prw.SetPercent(progId, 0.1);
		//
		//		//股票代码
		//		//int iStockId = 4;
		//		//所占比例
		//		int iValue = 4;
		//		//市值排名
		//		int iNo = 5;
		//		//流通比例
		//		int iLt = 6;
		//
		//		Tx::Core::Table_Indicator resTable_hb1;
		//		Tx::Core::Table_Indicator resTable_hb2;
		//
		//		//复制列信息
		//		resTable_hb1.CopyColumnInfoFrom(resTable_hb);
		//		resTable_hb2.CopyColumnInfoFrom(resTable_hb);
		//
		//		//建立列
		//		std::vector<int> iSelect;
		//		int nColCount = resTable_hb.GetColCount();
		//		UINT * ColArr = new UINT[nColCount];
		//		for(int icol=0;icol<nColCount;icol++)
		//			ColArr[icol]=icol;
		//
		//		//筛选财年
		//		iSelect.push_back(iFiscalYear);
		//		if(resTable_hb.GetRowCount()>0)
		//			resTable_hb.EqualsAt(resTable_hb1,ColArr,nColCount,1,iSelect);
		//		if(resTable_hb1.GetRowCount()>0)
		//		{
		//			//筛选财季
		//			iSelect.clear();
		//			iSelect.push_back(iFiscalQuarter);
		//			resTable_hb1.EqualsAt(resTable_hb2,ColArr,nColCount,2,iSelect);
		//		}
		//		resTable_hb.Clear();
		//		resTable_hb1.Clear();
		//
		//		//按照基金样本筛选
		//		resTable_hb.CopyColumnInfoFrom(resTable_hb2);
		//		iSelect.clear();
		//		for(i=0;i<(int)iSecurityId.size();i++)
		//		{
		//			GetSecurityNow(iSecurityId[i]);
		//			if(m_pSecurity==NULL)
		//				continue;
		//			int iSecurity1Id = m_pSecurity->GetSecurity1Id();
		//			iSelect.push_back(iSecurity1Id);
		//		}
		//		if(resTable_hb2.GetRowCount()>0)
		//			resTable_hb2.EqualsAt(resTable_hb,ColArr,nColCount,0,iSelect);
		//#ifdef _DEBUG
		//		strTable=resTable_hb.TableToString();
		//		Tx::Core::Commonality::String().StringToClipboard(strTable);
		//#endif
		//
		//		/////////////////////////////////////
		//		//清空
		//		resTable.Clear();
		//		//基金名称
		//		resTable.AddParameterColumn(Tx::Core::dtype_val_string);
		//		//基金代码
		//		resTable.AddParameterColumn(Tx::Core::dtype_val_string);
		//		//报告期
		//		resTable.AddParameterColumn(Tx::Core::dtype_val_string);
		//
		//		resTable.AddParameterColumn(Tx::Core::dtype_double);
		//		resTable.AddParameterColumn(Tx::Core::dtype_double);
		//
		//		resTable.AddParameterColumn(Tx::Core::dtype_double);
		//		resTable.AddParameterColumn(Tx::Core::dtype_double);
		//
		//		resTable.AddParameterColumn(Tx::Core::dtype_double);
		//		resTable.AddParameterColumn(Tx::Core::dtype_double);
		//
		//		resTable.AddParameterColumn(Tx::Core::dtype_double);
		//		resTable.AddParameterColumn(Tx::Core::dtype_double);
		//
		//		//重仓股列表
		//		Tx::Core::Table_Indicator resTable_zc;
		//		//3 循环处理每个样本
		//		int iRow = iSecurityId.size();
		//		//if(iRow>0)
		//		//	resTable.AddRow(iRow);
		//
		//#ifdef _DEBUG
		//		int isid=0;
		//		resTable_hb2.GetCell(0,0,isid);
		//		TRACE(_T("\nid=%d"),isid);
		//#endif
		//
		//		int iSampleCount = 0;
		//		for(int k=0;k<iRow;k++)
		//		{
		//			prw.SetPercent(progId, (double)k/(double)iRow);
		//
		//			//3.1
		//			resTable_zc.Clear();
		//			//从T_FUND_HOLDING_STOCK_BREAKOUT=基金持股明细[持股明细统计]中取得十大重仓股的持股市值
		//			//合并投资部分重仓股[T_STOCK_HOLDING_TOP_TEN]
		//
		//			//3.2
		//			//取得当前记录的交易实体ID
		//			int iSecurity = iSecurityId[k];
		//			GetSecurityNow(iSecurity);
		//			if(m_pSecurity==NULL)
		//				continue;
		//			int iSecurity1Id = m_pSecurity->GetSecurity1Id();
		//			//复制合并重仓股的列信息
		//			resTable_zc.CopyColumnInfoFrom(resTable_hb);
		//			//
		//			iSelect.clear();
		//			iSelect.push_back(iSecurity1Id);
		//			if(resTable_hb.GetRowCount()>0)
		//				resTable_hb.EqualsAt(resTable_zc,ColArr,nColCount,0,iSelect);
		//
		//			if(resTable_zc.GetRowCount()<=0)
		//				continue;
		//
		//			resTable.AddRow();
		//
		//			CString sName;
		//			sName = m_pSecurity->GetName();
		//			CString sCode;
		//			sCode = m_pSecurity->GetCode();
		//			resTable.SetCell(0,iSampleCount,sName);
		//			resTable.SetCell(1,iSampleCount,sCode);
		//
		//			CString sReport;
		//			sReport.Format(_T("%d年%s"),
		//				iFiscalYear,					//报告期-财年
		//				//报告期-财务季度
		//				TypeMapManage::GetInstance()->GetDatByID(TYPE_FISCAL_YEAR_QUARTER,iFiscalQuarter)
		//				);
		//			resTable.SetCell(2,iSampleCount,sReport);
		//
		//
		//			//按照股票市值降序排序
		//			//resTable_zc.Sort(iNo);
		//
		//			//--占资产净值的比例
		//			double jzbl = 0;
		//			double jzbl2 = 0;
		//			double jzbl3 = 0;
		//			double jzbl5 = 0;
		//			double jzbl10 = 0;
		//
		//			//--占股票投资市值的比例
		//			double gpszbl = 0;
		//			double gpszbl2 = 0;
		//			double gpszbl3 = 0;
		//			double gpszbl5 = 0;
		//			double gpszbl10 = 0;
		//
		//			for(int jj=0;jj<4;jj++)
		//			{
		//				int iStartIndex = 1;
		//				int iEndIndex = 1;
		//				switch(jj)
		//				{
		//				case 0:
		//					//前两大重仓股 = 股票投资[0]+股票投资[1]
		//					iEndIndex = 2;
		//					break;
		//				case 1:
		//					//前三大重仓股 = 前两大重仓股+股票投资[2];
		//					iEndIndex = 3;
		//					break;
		//				case 2:
		//					//前五大重仓股 = 前三大重仓股+股票投资[3]+股票投资[4]
		//					iEndIndex = 5;
		//					break;
		//				case 3:
		//					//前十大重仓股 = 前五大重仓股+股票投资[5]+...股票投资[9]
		//					iEndIndex = 10;
		//					break;
		//				}
		//				resTable_hb2.Clear();
		//				resTable_hb2.CopyColumnInfoFrom(resTable_zc);
		//				iSelect.clear();
		//				iSelect.push_back(iSecurity1Id);
		//				if(resTable_zc.GetRowCount()>0)
		//					//resTable_zc.EqualsAt(resTable_hb2,ColArr,nColCount,iNo,iSelect);
		//					resTable_zc.Between(resTable_hb2,ColArr,nColCount,iNo,iStartIndex,iEndIndex,true,true);
		//#ifdef _DEBUG
		//				strTable=resTable_hb2.TableToString();
		//				Tx::Core::Commonality::String().StringToClipboard(strTable);
		//#endif
		//				//循环处理每个样本
		//				double vv = 0;
		//				jzbl = 0;
		//				gpszbl = 0;
		//				for(int ii=0;ii<(int)resTable_hb2.GetRowCount();ii++)
		//				{
		//					//取得市值比例
		//					resTable_zc.GetCell(iValue,ii,vv);
		//					if((vv-Tx::Core::Con_doubleInvalid)>0.000001)
		//					{
		//						jzbl += vv;
		//					}
		//					else
		//					{
		//						TRACE(_T("\n市值比例:%s,%s,%s %.4f,%.3f"),sName,sCode,sReport,jzbl,vv);
		//					}
		//
		//					//取得占流通股比例
		//					resTable_zc.GetCell(iLt,ii,vv);
		//					if((vv-Tx::Core::Con_doubleInvalid)>0.000001)
		//					{
		//						gpszbl += vv;
		//					}
		//					else
		//					{
		//						TRACE(_T("\n流通股比例:%s,%s,%s %.4f,%.3f"),sName,sCode,sReport,gpszbl,vv);
		//					}
		//				}
		//				//3.7
		//				//插入数据到table
		//				int iCol = 3;
		//				resTable.SetCell(iCol+jj*2,iSampleCount,jzbl*100);
		//				iCol++;
		//				resTable.SetCell(iCol+jj*2,iSampleCount,gpszbl*100);
		//			}
		//			iSampleCount++;
		//
		//			//3.7
		//			//插入数据到table
		//			/*
		//			int iCol = 3;
		//			resTable.SetCell(iCol++,k,jzbl2);
		//			resTable.SetCell(iCol++,k,gpszbl2);
		//
		//			resTable.SetCell(iCol++,k,jzbl3);
		//			resTable.SetCell(iCol++,k,gpszbl3);
		//
		//			resTable.SetCell(iCol++,k,jzbl5);
		//			resTable.SetCell(iCol++,k,gpszbl5);
		//
		//			resTable.SetCell(iCol++,k,jzbl10);
		//			resTable.SetCell(iCol++,k,gpszbl10);
		//			*/
		//		}
		//		delete ColArr;
		//
		//		prw.SetPercent(progId, 1.0);
		//		sProgressPrompt+=_T(",完成!");
		//		prw.SetText(progId, sProgressPrompt);
		//
		//
		//		if(wholeTable.GetColCount()==0)
		//			wholeTable.CopyColumnInfoFrom(resTable);
		//		wholeTable.AppendTableByRow(resTable);
		//	}
		//	if(wholeTable.GetRowCount()!=0)
		//		resTable.Clone(wholeTable);
		//	return true;
		//}
		//重仓股季度比较 modify by lijw 2008-03-14
		bool TxFund::StatFundZCGjdbj(
			int iMenuID,						//功能ID
			Tx::Core::Table_Indicator& resTable,//结果数据表
			std::vector<int>& iSecurityId,		//交易实体ID
			std::vector<int> vDate				//报告期
			)
		{
			//2007-10-23
			//徐勉：
			//1重仓股包括积极投资部分，指数投资部分和合并部分
			//2重仓股数据表的设计在原来[积极投资部分重仓股]的基础之上增加一项比例字段，增加后包括持股数量占流通股本的比例，股票市值占基金股票市值的比例
			//3如此则不需要提取投资组合的数据

			//添加进度条
			//ProgressWnd* pwd=Tx::Core::ProgressWnd::GetInstance();
			//ProgressWnd prw;
			//UINT pid=prw.AddItem(1,_T("天相行业分布统计..."),0.0);
			//prw.Show(1000);

			ProgressWnd prw;
			//step2
			CString sProgressPrompt;
			sProgressPrompt.Format(_T("重仓股季度比较..."));
			UINT progId = prw.AddItem(1,sProgressPrompt, 0.0);
			//step3
			prw.Show(15);
			//step4
			prw.SetPercent(progId, 0.1);

			//没有样本，立即返回
			if(iSecurityId.size()<=0)
				return false;

			//step1
			//取得合并重仓股
			Tx::Core::Table_Indicator resTable_cgmx;
			//0-准备样本集=第1参数列:iSecurityId,int型=基金券ID
			resTable_cgmx.AddParameterColumn(Tx::Core::dtype_int4);
			//1-准备样本集=第2参数列:iFiscalYear,int型=财年
			resTable_cgmx.AddParameterColumn(Tx::Core::dtype_int4);
			//2-准备样本集=第3参数列:iFiscalQuarter,int型=财季
			resTable_cgmx.AddParameterColumn(Tx::Core::dtype_int4);
			//3-序号
			resTable_cgmx.AddParameterColumn(Tx::Core::dtype_byte);
			//col=4
			//股票ID
			int iColStockSecurity = 4;
			//col=5
			//股票市值
			int iColStockValue = 5;
			//col=6
			//占资产净值比例
			int iColNetValue = 6;
			//col=7
			//市值排名
			int iColNo = 7;
			//col=8
			//持股量
			int iColHolding = 8;
			//col=9
			//占流通股比例
			int iColRateLT = 9;

			int iIndicator = 0;		//指标=代码
			UINT varCfg1[4];		//参数配置
			int varCount1=4;		//参数个数

			//step1.1
			//设置指标需要的参数列
			varCfg1[0]=0;
			varCfg1[1]=1;
			varCfg1[2]=2;
			varCfg1[3]=3;

			//step1.2
			//循环处理指标列
			bool result = false;
			int i=0;
			iIndicator = 30901238;	//指标=代码[应该取合并重仓股指标]      
			//积极2007-12-07 涂暂时使用
			//	iIndicator = 30901232;//指标=代码[应该取合并重仓股指标]    备注 deleter by lijw 2008-02-21
			for (i = 0; i < 3; i++)
			{
				GetIndicatorDataNow(iIndicator+i);

				if (m_pIndicatorData==NULL)
					return false;
				result = m_pLogicalBusiness->SetIndicatorIntoTable(
					m_pIndicatorData,	//指标
					varCfg1,			//参数配置
					varCount1,			//参数个数
					resTable_cgmx		//计算需要的参数传输载体以及计算后结果的载体
					);
				if(result==false)
					break;
			}
			if(result==false)
				return false;

			iIndicator = 30901310;	//指标=代码[应该取合并重仓股指标]
			//积极2007-12-07 涂暂时使用
			//	iIndicator = 30901304;              备注 delete by lijw 2008-02-21
			for (i = 0; i < 3; i++)
			{
				GetIndicatorDataNow(iIndicator+i);

				if (m_pIndicatorData==NULL)
					return false;
				result = m_pLogicalBusiness->SetIndicatorIntoTable(
					m_pIndicatorData,	//指标
					varCfg1,			//参数配置
					varCount1,			//参数个数
					resTable_cgmx		//计算需要的参数传输载体以及计算后结果的载体
					);
				if(result==false)
					break;
			}
			if(result==false)
				return false;
			//step1.3
			//取得所有数据
			result = m_pLogicalBusiness->GetData(resTable_cgmx,true);
			if(result==false)
				return false;

			std::vector<int>::iterator iterDate;
			int iFiscalYear,iFiscalQuarter;

			Tx::Core::Table_Indicator dbTable,wholeTable;
			if(resTable_cgmx.GetRowCount()!=0)
				dbTable.Clone(resTable_cgmx);
			else
				dbTable.CopyColumnInfoFrom(resTable_cgmx);
			for(iterDate=vDate.begin();iterDate!=vDate.end();iterDate++)
			{
				if(dbTable.GetRowCount()!=0)
					resTable_cgmx.Clone(dbTable);
				else
					resTable_cgmx.CopyColumnInfoFrom(dbTable);
				iFiscalYear=*iterDate/10000;
				switch(*iterDate%10000)
				{
				case 331:
					iFiscalQuarter=40040001;
					break;
				case 630:
					iFiscalQuarter=40040002;
					break;
				case 930:
					iFiscalQuarter=40040004;
					break;
				case 1231:
					iFiscalQuarter=40040006;
					break;
				default:
					break;
				}
				//step1
				// Tx::Core::ProgressWnd* pProgressWnd = Tx::Core::ProgressWnd::GetInstance();


				//step1.4
				//准备筛选财年
				Tx::Core::Table_Indicator resTable_cgmx1;
				Tx::Core::Table_Indicator resTable_cgmx2;

				//step1.5
				//复制列信息
				resTable_cgmx1.CopyColumnInfoFrom(resTable_cgmx);
				resTable_cgmx2.CopyColumnInfoFrom(resTable_cgmx);

				//step1.6
				//建立列
				std::vector<int> iSelect;
				iSelect.push_back(iFiscalYear);
				int nColCount = resTable_cgmx.GetColCount();
				UINT * ColArr = new UINT[nColCount];
				for(int icol=0;icol<nColCount;icol++)
					ColArr[icol]=icol;

				//step1.7
				//筛选财年
				if(resTable_cgmx.GetRowCount()>0)
					resTable_cgmx.EqualsAt(resTable_cgmx1,ColArr,nColCount,1,iSelect);

				//step1.8
				//筛选财季
				if(resTable_cgmx1.GetRowCount()>0)
				{
					iSelect.clear();
					iSelect.push_back(iFiscalQuarter);
					resTable_cgmx1.EqualsAt(resTable_cgmx2,ColArr,nColCount,2,iSelect);
				}
				resTable_cgmx1.Clear();
				resTable_cgmx.Clear();

				//按照基金样本筛选
				resTable_cgmx.CopyColumnInfoFrom(resTable_cgmx2);
				iSelect.clear();
				std::vector<int> tempTradeV;//add by lijw 2008-11-21
				for(i=0;i<(int)iSecurityId.size();i++)
				{
					GetSecurityNow(iSecurityId[i]);
					if(m_pSecurity==NULL)
						continue;
					int iSecurity1Id = m_pSecurity->GetSecurity1Id();
					iSelect.push_back(iSecurity1Id);
					tempTradeV.push_back(iSecurityId[i]);
				}
				iSecurityId.clear();
				iSecurityId.assign(tempTradeV.begin(),tempTradeV.end());
				if(resTable_cgmx2.GetRowCount()>0)
					resTable_cgmx2.EqualsAt(resTable_cgmx,ColArr,nColCount,0,iSelect);

				/////////////////////////////////////

				//添加进度条
				//	prw.SetPercent(pid,0.6);

				//step2
				//准备数据结果列信息
				//清空
				resTable.Clear();
				//add by lijw 2008-03-14
				//添加基金的交易实体ID
				resTable.AddParameterColumn(Tx::Core::dtype_int4);
				//名称
				resTable.AddParameterColumn(Tx::Core::dtype_val_string);
				//代码
				resTable.AddParameterColumn(Tx::Core::dtype_val_string);
				//报告期
				resTable.AddParameterColumn(Tx::Core::dtype_int4);
				//报告期
				resTable.AddParameterColumn(Tx::Core::dtype_val_string);
				for(int izcg=0;izcg<10;izcg++)
				{
					//名称
					resTable.AddParameterColumn(Tx::Core::dtype_val_string);
					//代码
					resTable.AddParameterColumn(Tx::Core::dtype_val_string);
					//市值
					resTable.AddParameterColumn(Tx::Core::dtype_double);
					//比例
					resTable.AddParameterColumn(Tx::Core::dtype_double);
					//股数
					resTable.AddParameterColumn(Tx::Core::dtype_double);
				}

				//step3
				//按照基金重仓股列表输出数据
				Tx::Core::Table_Indicator resTable_zc;
				//step3.1
				//循环处理每个样本
				int iRow = iSecurityId.size();
				//if(iRow>0)
				//	resTable.AddRow(iRow);

				int iSampleCount = 0;
				for(int k=0;k<iRow;k++)
				{
					prw.SetPercent(progId, (double)k/(double)iRow);
					//step3.2
					//清空当前基金重仓股数据表
					resTable_zc.Clear();
					//从T_FUND_HOLDING_STOCK_BREAKOUT=基金持股明细[持股明细统计]中取得十大重仓股的持股市值
					//合并投资部分重仓股[T_STOCK_HOLDING_TOP_TEN]

					//step3.3
					//取得当前记录的基金交易实体ID
					int iSecurity = iSecurityId[k];
					int iSecurity1Id = 0;
					CString sName;
					CString sCode;
					//resTable_zh.GetCell(0,k,iSecurity);
					GetSecurityNow(iSecurity);
					if(m_pSecurity==NULL)
						continue;
					//step3.7
					//取得基金券ID，用于循环处理重仓股数据
					iSecurity1Id = m_pSecurity->GetSecurity1Id();

					//step3.8
					//取得当前基金重仓股数据表
					//复制持股明细的列信息
					resTable_zc.CopyColumnInfoFrom(resTable_cgmx);
					iSelect.clear();
					iSelect.push_back(iSecurity1Id);
					if(resTable_cgmx.GetRowCount()>0)
						resTable_cgmx.EqualsAt(resTable_zc,ColArr,nColCount,0,iSelect);
#ifdef _DEBUG
					CString strTable = resTable_zc.TableToString();
					Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
					//step3.8
					//如果当前基金没有重仓股数据，则处理下一个基金
					if(resTable_zc.GetRowCount()<=0)
						continue;

					//step3.3
					//取得基金名称
					sName = m_pSecurity->GetName();
					//step3.4
					//取得基金代码
					sCode = m_pSecurity->GetCode();

					resTable.AddRow();
					//step3.5
					// 基金的交易实体ID add by lijw 2008-03-14
					resTable.SetCell(0,iSampleCount,iSecurity);
					//保存基金名称 modify by lijw 2008-03-14
					resTable.SetCell(1,iSampleCount,sName);
					//step3.6
					//保存基金代码
					resTable.SetCell(2,iSampleCount,sCode);

					//step3.7
					//处理报告期
					CString sReport;
					sReport.Format(_T("%d%s"),
						iFiscalYear,					//报告期-财年
						TypeMapManage::GetInstance()->GetDatByID(TYPE_FISCAL_YEAR_QUARTER,iFiscalQuarter)
						);
					resTable.SetCell(4,iSampleCount,sReport);
					int tempdate1,reportdate;
					switch(iFiscalQuarter)
					{
					case 40040001:
						tempdate1=331;
						break;
					case 40040002:
						tempdate1=630;
						break;
					case 40040004:
						tempdate1=930;
						break;
					case 40040006:
						tempdate1=1231;
						break;
					default:
						break;
					}
					reportdate = iFiscalYear*10000 + tempdate1;
					resTable.SetCell(3,iSampleCount,reportdate);
					//step3.9
					//按照股票市值降序排序
					resTable_zc.Sort(iColNo);
					resTable_zc.Arrange();
#ifdef _DEBUG
					strTable=resTable_zc.TableToString();
					Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
					//modify by lijw 2008-03-14
					int iCol = 5;
					//step3.10
					//处理前十大
					for(int jj=0;jj<10&&jj<(int)resTable_zc.GetRowCount();jj++)
					{
						//根据排名取得样本
						resTable_cgmx2.Clear();
						resTable_cgmx2.CopyColumnInfoFrom(resTable_zc);
						iSelect.clear();
						iSelect.push_back(jj+1);
						if(resTable_zc.GetRowCount()>0)
							resTable_zc.EqualsAt(resTable_cgmx2,ColArr,nColCount,iColNo,iSelect);
#ifdef _DEBUG
						strTable=resTable_cgmx2.TableToString();
						Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
						//modify by lijw 2008-03-14
						if(resTable_cgmx2.GetRowCount()<=0)
							continue;

						//循环处理每个样本
						double v = 0;
						//代码
						sCode = _T("");
						//名称
						sName = _T("");
						int iSampleSecurityId = 0;

						//取得当前基金重仓股的股票ID[交易实体ID]
						//始终取第一个样本，因为排名可能存在并列的样本
						resTable_cgmx2.GetCell(iColStockSecurity,0,iSampleSecurityId);

						GetSecurityNow(iSampleSecurityId);
						if(m_pSecurity!=NULL)
						{
							//取得股票名称
							sName = m_pSecurity->GetName();
							//取得股票代码
							sCode = m_pSecurity->GetCode();
						}
						//保存股票代码
						resTable.SetCell(iCol++,iSampleCount,sCode);
						//step3.15
						//保存股票名称
						resTable.SetCell(iCol++,iSampleCount,sName);

						//取得股票市值
						resTable_cgmx2.GetCell(iColStockValue,0,v);
						//保存股票市值
						resTable.SetCell(iCol++,iSampleCount,v);

						//占净值比例
						resTable_cgmx2.GetCell(iColNetValue,0,v);
						//占净值比例
						if(v>0)
							resTable.SetCell(iCol++,iSampleCount,v*100);
						else
							resTable.SetCell(iCol++,iSampleCount,Con_doubleInvalid);

						//取得持股数
						resTable_cgmx2.GetCell(iColHolding,0,v);
						if(v>0)
							resTable.SetCell(iCol++,iSampleCount,v);
						else
							resTable.SetCell(iCol++,iSampleCount,Con_doubleInvalid);
					}
					iSampleCount++;
				}
				delete ColArr;
				ColArr = NULL;

				if(wholeTable.GetColCount()==0)
					wholeTable.CopyColumnInfoFrom(resTable);
				wholeTable.AppendTableByRow(resTable);
			}
			if(wholeTable.GetRowCount()!=0)
				resTable.Clone(wholeTable);
#ifdef _DEBUG
			strTable = resTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
			MultiSortRule multisort;
			multisort.AddRule(3,false);
			multisort.AddRule(2,true);	
			resTable.SortInMultiCol(multisort);
			resTable.Arrange();
			resTable.DeleteCol(3);
			prw.SetPercent(progId, 1.0);
			sProgressPrompt+=_T(",完成!");
			prw.SetText(progId, sProgressPrompt);
			return true;
		}
		bool TxFund::StockHoldingTopTen(
			Tx::Core::Table_Indicator &resTable,
			std::vector<int>	iSecurityId,
			std::vector<int>	iDate
			)
		{
			//默认的返回值状态
			bool result = false;
			//把交易实体ID转化为基金ID
			std::vector<int> iSecurity1Id;
			std::vector<int>::iterator iterV,iterV2;
			int tempId;
			std::vector<int> tempTradeV;
			for (iterV = iSecurityId.begin();iterV != iSecurityId.end();++iterV)
			{
				GetSecurityNow(*iterV);
				if (m_pSecurity != NULL)
				{
					tempId = m_pSecurity->GetSecurity1Id(*iterV);
					iterV2 = iSecurity1Id.end();
					if (find(iSecurity1Id.begin(),iterV2,tempId) == iterV2)
					{
						iSecurity1Id.push_back(tempId);
						tempTradeV.push_back(*iterV);
					}			
				}
			}
			iSecurityId.clear();
			iSecurityId.assign(tempTradeV.begin(),tempTradeV.end());
			//从T_STOCK_HOLDING_TOP_TEN_twoyear取其他的数据
			Tx::Core::Table_Indicator hbTable;
			//准备样本集参数列
			hbTable.AddParameterColumn(Tx::Core::dtype_int4);//基金ID
			hbTable.AddParameterColumn(Tx::Core::dtype_int4);//报告年份
			hbTable.AddParameterColumn(Tx::Core::dtype_int4);//报告期
			hbTable.AddParameterColumn(Tx::Core::dtype_byte);//F1序号
			const int indicatorIndex3 = 5;
			long iIndicator3[indicatorIndex3] = 
			{
				30901238,	//股票ID
				30901239,	//股票市值
				30901311,	//持股数量
				30901312,	//占流通股比例（%）
				30901240	//所占比例（即是占净值比例）		
			};
			UINT varCfg3[4];			//参数配置
			int varCount3=4;			//参数个数
			for (int i = 0; i < indicatorIndex3; i++)
			{
				int tempIndicator = iIndicator3[i];

				GetIndicatorDataNow(tempIndicator);
				if (m_pIndicatorData==NULL)
				{ 
					return false;
				}
				varCfg3[0]=0;
				varCfg3[1]=1;
				varCfg3[2]=2;
				varCfg3[3]=3;
				result = m_pLogicalBusiness->SetIndicatorIntoTable(
					m_pIndicatorData,	//指标
					varCfg3,				//参数配置
					varCount3,			//参数个数
					hbTable	//计算需要的参数传输载体以及计算后结果的载体
					);
				if(result==false)
				{ 
					return false;
				}
			}
			result = m_pLogicalBusiness->GetData(hbTable,true);	
#ifdef _DEBUG
			strTable=hbTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
			UINT iColCount3 = hbTable.GetColCount();
			UINT* nColArray3 = new UINT[iColCount3];
			for(int i = 0; i < (int)iColCount3; i++)
			{
				nColArray3[i] = i;
			}	
			Tx::Core::Table_Indicator tempTable;
			tempTable.CopyColumnInfoFrom(hbTable);
			//根据基金ID进行筛选
			hbTable.EqualsAt(tempTable,nColArray3,iColCount3,0,iSecurity1Id);
			//为了进行年度和报告期的筛选，所以把报告年份和报告期和为一列。
			TransReportDateToNormal2(tempTable,1);
#ifdef _DEBUG
			strTable=tempTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
			resTable.CopyColumnInfoFrom(tempTable);
			//进行年度和报告期的筛选
			tempTable.EqualsAt(resTable,nColArray3,iColCount3-1,1,iDate);
			if(resTable.GetRowCount() == 0)
			{
				delete nColArray3;
				nColArray3 = NULL;
				return false;
			}
#ifdef _DEBUG
			strTable=resTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
			delete nColArray3;
			nColArray3 = NULL;

			//为增加基金的交易实体ID和名称，代码作准备
			resTable.InsertCol(0,Tx::Core::dtype_int4);//基金交易实体ID
			resTable.InsertCol(1,Tx::Core::dtype_val_string);//基金名称
			resTable.InsertCol(2,Tx::Core::dtype_val_string);//基金代码
			//增加基金的交易实体ID和名称，代码
			double dEquity,dCircle;	
			std::vector<int>::iterator iterId;
			int fundId  ;
			CString strName,strCode;
			int position;
			std::vector<UINT> vecInstiID;
			std::vector<UINT>::iterator iteID;
			for(iterId = iSecurityId.begin();iterId != iSecurityId.end();++iterId)
			{
				GetSecurityNow(*iterId);
				if(m_pSecurity == NULL)
					continue;
				fundId = m_pSecurity->GetSecurity1Id();
				strName = m_pSecurity->GetName();
				strCode = m_pSecurity->GetCode();
				if(!vecInstiID.empty())
					vecInstiID.clear();
				resTable.Find(3,fundId,vecInstiID);	
				if(vecInstiID.empty())
					continue;
				for(iteID = vecInstiID.begin();iteID != vecInstiID.end();++iteID)
				{
					position = *iteID;
					resTable.SetCell(0,position,*iterId);
					resTable.SetCell(1,position,strName);
					resTable.SetCell(2,position,strCode);
					//占流通股比，分别乘以100			
					resTable.GetCell(10,position,dEquity);
					dEquity = dEquity*100;
					resTable.SetCell(10,position,dEquity);
					resTable.GetCell(9,position,dCircle);
					dCircle = dCircle*100;
					resTable.SetCell(9,position,dCircle);
				}		
			}
			resTable.DeleteCol(5);//删除序号
			resTable.DeleteCol(3);//删除基金ID	
			if(resTable.GetRowCount() == 0)
				return false;
			return true;
		}

		//数据源为持股明细
		bool TxFund::StockHoldingList(
			Tx::Core::Table_Indicator &resTable,
			std::vector<int>	iSecurityId,
			std::vector<int>	iDate
			)
		{
			bool result = false;	
			//把交易实体ID转化为基金ID
			std::set<int> iSecurityIdSet;
			std::vector<int>::iterator iterV;
			int fundId;
			std::vector<int> tempTradeV;
			for(iterV = iSecurityId.begin();iterV != iSecurityId.end();++iterV)
			{
				GetSecurityNow(*iterV);
				if(m_pSecurity != NULL)
				{
					fundId = m_pSecurity->GetSecurity1Id(*iterV);
					if(iSecurityIdSet.find(fundId) == iSecurityIdSet.end())
					{
						iSecurityIdSet.insert(fundId);
						tempTradeV.push_back(*iterV);
					}
				}
			}
			iSecurityId.clear();
			iSecurityId.assign(tempTradeV.begin(),tempTradeV.end());
			Tx::Core::Table_Indicator tempTable;
			Tx::Core::Table_Indicator hbTable;
			//准备样本集参数列
			hbTable.AddParameterColumn(Tx::Core::dtype_int4);//基金ID
			hbTable.AddParameterColumn(Tx::Core::dtype_int4);//报告年份
			hbTable.AddParameterColumn(Tx::Core::dtype_int4);//报告期
			hbTable.AddParameterColumn(Tx::Core::dtype_byte);//持股类型
			hbTable.AddParameterColumn(Tx::Core::dtype_int4);//序号
			const int indicatorIndex3 = 5;
			long iIndicator3[indicatorIndex3] = 
			{
				30901128,	//股票ID
				30901130,	//股票市值
				30901129,	//持股数量		
				30901322,	//占流通股比例（%）
				30901321	//所占比例（即是占净值比例）
			};
			UINT varCfg3[5];			//参数配置
			int varCount3=5;			//参数个数
			for (int i = 0; i < indicatorIndex3; i++)
			{
				int tempIndicator = iIndicator3[i];

				GetIndicatorDataNow(tempIndicator);
				if (m_pIndicatorData==NULL)
				{ return false; }
				varCfg3[0]=0;
				varCfg3[1]=1;
				varCfg3[2]=2;
				varCfg3[3]=3;
				varCfg3[4]=4;
				result = m_pLogicalBusiness->SetIndicatorIntoTable(
					m_pIndicatorData,	//指标
					varCfg3,				//参数配置
					varCount3,			//参数个数
					hbTable	//计算需要的参数传输载体以及计算后结果的载体
					);
				if(result==false)
				{
					return false;
				}
			}
			result = m_pLogicalBusiness->GetData(hbTable,true);
			if(hbTable.GetRowCount() == 0)
			{
				return false;
			}
			UINT iColCount4 = hbTable.GetColCount();
			UINT* nColArray4 = new UINT[iColCount4];
			for(int i = 0; i < (int)iColCount4; i++)
			{
				nColArray4[i] = i;
			}
			tempTable.Clear();
			tempTable.CopyColumnInfoFrom(hbTable);
			std::vector<byte> TypeVector;
			TypeVector.push_back(3);
			hbTable.EqualsAt(tempTable,nColArray4,iColCount4,3,TypeVector);
			delete nColArray4;
			nColArray4 = NULL;
			hbTable.Clear();
			hbTable.Clone(tempTable);//之所以这样写是为了不修改下面的代码
#ifdef _DEBUG
			strTable=hbTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
			hbTable.DeleteCol(3,2);//删除持股类型和序号列
#ifdef _DEBUG
			strTable=hbTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
			UINT iColCount3 = hbTable.GetColCount();
			UINT* nColArray3 = new UINT[iColCount3];
			for(int i = 0; i < (int)iColCount3; i++)
			{
				nColArray3[i] = i;
			}
			tempTable.Clear();
			tempTable.CopyColumnInfoFrom(hbTable);

			hbTable.EqualsAt(tempTable,nColArray3,iColCount3,0,iSecurityIdSet);
#ifdef _DEBUG
			strTable=tempTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
			//为了进行年度和报告期的筛选，所以把报告年份和报告期和为一列。
			TransReportDateToNormal2(tempTable,1);
			resTable.CopyColumnInfoFrom(tempTable);
			tempTable.EqualsAt(resTable,nColArray3,iColCount3 - 1,1,iDate);
			delete nColArray3;
			nColArray3 = NULL;
			if(resTable.GetRowCount() == 0)
			{		
				return false;
			}
			//为增加基金的交易实体ID和名称，代码作准备
			resTable.InsertCol(0,Tx::Core::dtype_int4);//基金交易实体ID
			resTable.InsertCol(1,Tx::Core::dtype_val_string);//基金名称
			resTable.InsertCol(2,Tx::Core::dtype_val_string);//基金代码	
			double dEquity,dCircle;
			CString strname,strcode;	
			//增加基金的交易实体ID和名称，代码
			std::vector<UINT> vecInstiID;
			std::vector<UINT>::iterator iteID;
			int position;
			for(iterV = iSecurityId.begin();iterV != iSecurityId.end();++iterV)
			{
				GetSecurityNow(*iterV);
				if(m_pSecurity == NULL)
					continue;
				fundId = m_pSecurity->GetSecurity1Id();
				strname = m_pSecurity->GetName();
				strcode = m_pSecurity->GetCode();
				if(!vecInstiID.empty())
					vecInstiID.clear();
				resTable.Find(3,fundId,vecInstiID);
				if(vecInstiID.empty())
					continue;
				for(iteID = vecInstiID.begin();iteID != vecInstiID.end();++iteID)
				{
					position = *iteID;
					resTable.SetCell(0,position,*iterV);
					resTable.SetCell(1,position,strname);
					resTable.SetCell(2,position,strcode);
					//占流通股比，分别乘以100
					resTable.GetCell(9,position,dEquity);
					dEquity = dEquity*100;
					resTable.SetCell(9,position,dEquity);
					resTable.GetCell(8,position,dCircle);
					dCircle = dCircle*100;
					resTable.SetCell(8,position,dCircle);		
				}		
			}
			resTable.DeleteCol(3);//删除基金ID
			if(resTable.GetRowCount() == 0)
			{
				return false;
			}
			return true;
		}
		////股票持有情况统计
		//bool TxFund::StatFundStockHolding(
		//								    Tx::Core::Table_Indicator& sourceTable,	//为双击后保存数据。
		//									Tx::Core::Table_Indicator& resTableStock,	//股票结果数据表
		//									Tx::Core::Table_Indicator& resTableFund,	//基金结果数据表
		//									std::vector<int>& iFundSecurityId,		//基金交易实体ID
		//									std::vector<int>& iStockSecurityId,		//股票交易实体ID
		//									std::vector<int> vDate,				//报告期
		//									bool bIsZCG,							//数据源为重仓股；否则为持股明细
		//									//std::set<int> & TradeId,			//保存所取得sourceTable里的数据的全部的交易实体ID
		//									std::vector<CString> &vColName,
		//									std::vector<CString> &vColHeaderName
		//									)
		//{
		//	ProgressWnd prw;
		//	UINT pid=prw.AddItem(1,_T("股票持有情况统计..."),0.0);
		//	prw.Show(1000);
		//
		//    //从数据源取数据
		////	Tx::Core::Table_Indicator resTable;
		//	bool result = false;
		//	if(bIsZCG)
		//	{
		//		result = StockHoldingTopTen(sourceTable,iFundSecurityId,vDate);
		//		if(result == false)
		//		{
		//			//添加进度条
		//			prw.SetPercent(pid,1.0);
		//			return false;
		//		}
		//	}
		//	else
		//	{
		//		result = StockHoldingList(sourceTable,iFundSecurityId,vDate);
		//		if(result == false)
		//		{
		//			//添加进度条
		//			prw.SetPercent(pid,1.0);
		//			return false;
		//		}
		//	}
		//
		//	//添加进度条
		//	prw.SetPercent(pid,0.6);
		//#ifdef _DEBUG
		//	strTable=sourceTable.TableToString();
		//	Tx::Core::Commonality::String().StringToClipboard(strTable);
		//#endif
		//	//添加列的类型
		//	resTableStock.AddCol(Tx::Core::dtype_int4);//0交易实体ID
		//	resTableStock.AddCol(Tx::Core::dtype_val_string);//1股票名称
		//	resTableStock.AddCol(Tx::Core::dtype_val_string);//2股票代码
		//	resTableStock.AddCol(Tx::Core::dtype_int4);//3上市日期
		//	resTableStock.AddCol(Tx::Core::dtype_double);//4总股本
		//	resTableStock.AddCol(Tx::Core::dtype_double);//5流通股本
		//	resTableStock.AddCol(Tx::Core::dtype_val_string);//6证监会行业
		//	resTableStock.AddCol(Tx::Core::dtype_val_string);//7天相行业
		//
		//	//填充标题
		//	vColName.clear();
		//	vColName.push_back(_T("股票名称"));
		//	vColName.push_back(_T("股票代码"));	
		//	int isize = (int)vDate.size();
		//	/*std::set<int,greater<int> > ErasePosSet;
		//	std::set<int,greater<int> >::iterator EraseIter;*/
		//	int tempPosRow = -1;
		//	for(int k = 0;k < isize;k++)
		//	{		
		//		//填充标题
		//		CString stryear,strdate;
		//		int tempdate,iyear;
		//		tempdate = vDate[k];
		//		int itemp;
		//		itemp = tempdate%10000;
		//		iyear = tempdate/10000;
		//		stryear.Format(_T("%d"),iyear);
		//		switch(itemp)
		//		{
		//		case 331:
		//			strdate = stryear + _T("年") + _T("一季报");
		//			break;
		//		case 630:
		//			strdate = stryear + _T("年") + _T("二季报");
		//			break;
		//		case 930:
		//			strdate = stryear + _T("年") + _T("三季报");
		//			break;
		//		case 1231:
		//			strdate = stryear + _T("年") + _T("四季报");
		//			break;
		//		}
		//		//暂时先把下面的代码去掉，因为会出现标题显示不一致的问题，等以后，改变的话，可以直接用下面的代码
		//		//if(bIsZCG)
		//		//{
		//		//	int itemp;
		//		//	itemp = tempdate%10000;
		//		//	iyear = tempdate/10000;
		//		//	stryear.Format(_T("%d"),iyear);
		//		//	switch(itemp)
		//		//	{
		//		//	case 331:
		//		//		strdate = stryear + _T("年") + _T("一季报");
		//		//		break;
		//		//	case 630:
		//		//		strdate = stryear + _T("年") + _T("二季报");
		//		//		break;
		//		//	case 930:
		//		//		strdate = stryear + _T("年") + _T("三季报");
		//		//		break;
		//		//	case 1231:
		//		//		strdate = stryear + _T("年") + _T("四季报");
		//		//		break;
		//		//	}
		//		//}
		//		////暂时先把他们的报告期写为二、四季报。等需要时，
		//		////可以直接把他们改称中报和年报，无须修改程序。
		//		//else
		//		//{
		//		//	iyear = tempdate/10000;
		//		//	stryear.Format(_T("%d"),iyear);
		//		//	if(tempdate%10000 == 630)
		//		//		strdate = stryear + _T("年") + _T("二季报");
		//		//	if(tempdate%10000 == 1231)
		//		//		strdate = stryear + _T("年") + _T("四季报");
		//		//}
		//		vColHeaderName.push_back(strdate);
		//		vColName.push_back(_T("持有市值(元)"));
		//		vColName.push_back(_T("占流通股比例%"));
		//		vColName.push_back(_T("持股数(股)"));
		//		vColName.push_back(_T("持有该股基金数"));
		//		//添加相应的列
		//		tempPosRow++;
		//		resTableStock.InsertCol(3+tempPosRow*4,Tx::Core::dtype_double);
		//		resTableStock.InsertCol(4+tempPosRow*4,Tx::Core::dtype_double);
		//		resTableStock.InsertCol(5+tempPosRow*4,Tx::Core::dtype_double);
		//		resTableStock.InsertCol(6+tempPosRow*4,Tx::Core::dtype_int4);//基金支数
		//	}
		//	vColName.push_back(_T("上市日期"));
		//	vColName.push_back(_T("总股本"));
		//	vColName.push_back(_T("流通股本"));
		//	vColName.push_back(_T("证监会行业"));
		//	vColName.push_back(_T("天相行业"));
		//	////把需要删掉的日期删掉
		//	//std::vector<int>::iterator iterV;
		//	//for(EraseIter = ErasePosSet.begin();EraseIter != ErasePosSet.end();++EraseIter)
		//	//{
		//	//	iterV = find(vDate.begin(),vDate.end(),*EraseIter);
		//	//	if(iterV != vDate.end())
		//	//		vDate.erase(iterV);
		//	//}
		//    isize = (int)vDate.size();
		//    //填充resTableFund和resTableStock的数据
		//	resTableFund.CopyColumnInfoFrom(sourceTable);
		//	//首先把reaTable里的股票全部取出来
		//	int iRowCount = sourceTable.GetRowCount();
		//	std::set<int> TradeSet;
		//	int iTradeId;
		//	for(int i = 0;i < iRowCount;i++)
		//	{
		//		sourceTable.GetCell(4,i,iTradeId);
		//		TradeSet.insert(iTradeId);
		//	}
		////	TradeId = TradeSet;
		//	CString strData;
		//	double dData,dData2;
		//	std::vector<int>::iterator iterVector;
		//	int flag = -1;
		//	int position;
		//	iRowCount = -1;
		//	int iData;
		//	int iRow = -1;
		//	std::unordered_map<int,int> iDataMap;
		//	std::unordered_map<int,double> dDataMap;
		//	std::vector<UINT> vecInstiID,vecInstiID2,positionV;
		//	std::vector<UINT>::iterator iteID;
		//	for(iterVector = iStockSecurityId.begin();iterVector != iStockSecurityId.end();++iterVector)
		//	{
		//		if(TradeSet.find(*iterVector) != TradeSet.end())
		//		{
		//			//填充resTableStock			
		//			iData = *iterVector;			
		//			GetSecurityNow(iData);
		//			if(m_pSecurity == NULL)
		//				continue;
		//			resTableStock.AddRow();
		//			iRow++;
		//			//取得股票交易实体
		//			resTableStock.SetCell(0,iRow,iData);
		//			//股票名称
		//			strData = m_pSecurity->GetName();		
		//			resTableStock.SetCell(1,iRow,strData);
		//			//股票代码
		//			strData = m_pSecurity->GetCode();
		//			resTableStock.SetCell(2,iRow,strData);
		//			//上市日期
		//			iData = m_pSecurity->GetIPOListedDate();
		//			resTableStock.SetCell(3+isize*4,iRow,iData);
		//			//总股本
		//			dData = m_pSecurity->GetTotalInstitutionShare();
		//			if(dData <= 0)
		//				dData = Tx::Core::Con_doubleInvalid;
		//			resTableStock.SetCell(4+isize*4,iRow,dData);
		//			//流通股本
		//			double capitalization = m_pSecurity->GetTradableShare();
		//			if(capitalization <= 0)
		//				capitalization = Tx::Core::Con_doubleInvalid;
		//			resTableStock.SetCell(5+isize*4,iRow,capitalization);
		//			//证监会行业
		//			strData = m_pSecurity->GetCSRCIndustryName();		
		//			resTableStock.SetCell(6+isize*4,iRow,strData);
		//			//天相行业
		//			strData = m_pSecurity->GetTxSecIndustryName();
		//			resTableStock.SetCell(7+isize*4,iRow,strData);
		//			//用于记录该股票是否被显示
		//			int VisibleFlag = 0;
		//			//根据报告期和股票的交易实体ID取其他的数据;
		//			for (int i = 0;i < isize;i++)
		//			{
		//				if (!vecInstiID.empty())
		//					vecInstiID.clear();
		//				if(!vecInstiID2.empty())
		//					vecInstiID2.clear();
		//				sourceTable.Find(4,*iterVector,vecInstiID);
		//				sourceTable.Find(3,vDate[i],vecInstiID2);
		//				if(!positionV.empty())
		//					positionV.clear();
		//				for(iteID = vecInstiID.begin();iteID != vecInstiID.end();++iteID)
		//				{
		//					if(find(vecInstiID2.begin(),vecInstiID2.end(),*iteID) != vecInstiID2.end())
		//						positionV.push_back(*iteID);				
		//				}
		//				if(positionV.empty())
		//				{
		//					//股票市值
		//					dData = Tx::Core::Con_doubleInvalid;
		//					resTableStock.SetCell(3+i*4,iRow,dData);
		//					//持股数量
		//					dData2 = Tx::Core::Con_doubleInvalid;
		//					resTableStock.SetCell(5+i*4,iRow,dData2);					
		//					//占流通股比
		//					double dRate;	
		//					dRate = Tx::Core::Con_doubleInvalid;					
		//					resTableStock.SetCell(4+i*4,iRow,dRate);
		//					//基金支数
		//					int iFundCount = Tx::Core::Con_intInvalid;
		//					resTableStock.SetCell(6+i*4,iRow,iFundCount);
		//					VisibleFlag++;
		//					continue;
		//				}
		//				dData = dData2 = 0;				
		//				double dtemp1,dtemp2;
		//				for(iteID = positionV.begin();iteID != positionV.end();++iteID)
		//				{
		//					sourceTable.GetCell(5,*iteID,dtemp1);
		//					dData += dtemp1;
		//					sourceTable.GetCell(6,*iteID,dtemp2);
		//					dData2 += dtemp2;
		//				}
		//				//股票市值
		//				if(dData <= 0)
		//					dData = Tx::Core::Con_doubleInvalid;
		//				resTableStock.SetCell(3+i*4,iRow,dData);
		//				//持股数量
		//				if(dData2 <= 0)
		//					dData2 = Tx::Core::Con_doubleInvalid;
		//				resTableStock.SetCell(5+i*4,iRow,dData2);
		//				//取得离指定报告期最近的日期的股本数据
		//				TxShareData *pShareData = m_pSecurity->GetTxShareDataByDate(vDate[i]);
		//				//占流通股比
		//				double dRate;
		//				if(fabs(pShareData->TradeableShare - Tx::Core::Con_doubleInvalid) < 0.000001)
		//					dRate = Tx::Core::Con_doubleInvalid;
		//				else
		//					dRate = dData2/pShareData->TradeableShare*100;
		//				resTableStock.SetCell(4+i*4,iRow,dRate);
		//				//基金支数
		//				int iFundCount = (int)positionV.size();
		//				resTableStock.SetCell(6+i*4,iRow,iFundCount);
		//			}
		//			if(VisibleFlag == isize)
		//			{
		//				resTableStock.DeleteRow(iRow);
		//				iRow--;
		//				continue;
		//			}
		//            //填充resTableFund
		//			flag++;
		//			if(flag == 0)
		//			{
		//				//填充resTableFund
		//				if (!vecInstiID.empty())
		//					vecInstiID.clear();
		//				sourceTable.Find(4,*iterVector,vecInstiID);
		//				for(iteID = vecInstiID.begin();iteID != vecInstiID.end();++iteID)
		//				{
		//					resTableFund.AddRow();
		//					iRowCount++;
		//					position = *iteID;
		//					sourceTable.GetCell(0,position,iTradeId);
		//					resTableFund.SetCell(0,iRowCount,iTradeId);
		//					sourceTable.GetCell(3,position,iData);
		//					resTableFund.SetCell(3,iRowCount,iData);
		//					for(int i = 1;i < 3;i++)
		//					{
		//						sourceTable.GetCell(i,position,strData);
		//						resTableFund.SetCell(i,iRowCount,strData);
		//					}
		//					for(int j = 5;j < 9;j++)
		//					{
		//						sourceTable.GetCell(j,position,dData);
		//						resTableFund.SetCell(j,iRowCount,dData);
		//					}
		//				}
		//				//把报告年份和报告期转化成CString类型,所以增加一列
		//				resTableFund.InsertCol(4,Tx::Core::dtype_val_string);//str报告期	
		//				CString stryear,strdate;
		//				int tempdate,iyear;
		//				if(bIsZCG)
		//				{
		//					//填写报告期
		//					for(int k = 0;k < (int)resTableFund.GetRowCount();k++)
		//					{
		//						int itemp;
		//						resTableFund.GetCell(3,k,tempdate);	
		//						itemp = tempdate%10000;
		//						iyear = tempdate/10000;
		//						stryear.Format(_T("%d"),iyear);
		//						switch(itemp)
		//						{
		//						case 331:
		//							strdate = stryear + _T("年") + _T("一季报");
		//							break;
		//						case 630:
		//							strdate = stryear + _T("年") + _T("二季报");
		//							break;
		//						case 930:
		//							strdate = stryear + _T("年") + _T("三季报");
		//							break;
		//						case 1231:
		//							strdate = stryear + _T("年") + _T("四季报");
		//							break;
		//						}
		//						resTableFund.SetCell(4,k,strdate);
		//					}
		//				}
		//				else
		//				{
		//					//填写报告期  暂时先把他们的报告期写为二、四季报。等需要时，
		//					//可以直接把他们改称中报和年报，无须修改程序。
		//					for(int k = 0;k < (int)resTableFund.GetRowCount() ;k++)
		//					{
		//						resTableFund.GetCell(3,k,tempdate);
		//						iyear = tempdate/10000;
		//						stryear.Format(_T("%d"),iyear);
		//						if(tempdate%10000 == 630)
		//							strdate = stryear + _T("年") + _T("二季报");
		//						if(tempdate%10000 == 1231)
		//							strdate = stryear + _T("年") + _T("四季报");
		//						resTableFund.SetCell(4,k,strdate);
		//					}
		//				}
		//				resTableFund.DeleteCol(5);//
		//				resTableFund.DeleteCol(3);//删除报告期（int）
		//				resTableFund.Sort(2);
		//#ifdef _DEBUG
		//				strTable=resTableFund.TableToString();
		//				Tx::Core::Commonality::String().StringToClipboard(strTable);
		//#endif
		//			}			
		//		}		
		//	}
		//	if(resTableFund.GetRowCount() == 0)
		//	{
		//		//添加进度条
		//		prw.SetPercent(pid,1.0);
		//		return false;
		//	}
		//#ifdef _DEBUG
		//	strTable=resTableStock.TableToString();
		//	Tx::Core::Commonality::String().StringToClipboard(strTable);
		//#endif
		//
		//	//添加进度条
		//	prw.SetPercent(pid,1.0);
		//	return true;
		//}
		//某股票被基金持有的明细
		bool TxFund::StatFundStockDetail(
			Tx::Core::Table_Indicator& resTable,	//结果数据表
			Tx::Core::Table_Indicator& resTableMx,	//明细
			int iSecurityStockId	//股票交易实体ID
			)
		{
			//明细数据记录为空，则不处理
			if(resTableMx.GetRowCount()<=0)
				return false;

			//明细数据列数为0，则不处理
			int nColCount = resTableMx.GetColCount();
			if(nColCount<=0)
				return false;

			//准备复制
			UINT *ColArr= new UINT[nColCount];
			for(int i=0;i<nColCount;i++)
				ColArr[i]=i;

			//准备筛选股票的样本
			std::vector<int> iSelect;
			iSelect.push_back(iSecurityStockId);

			//复制列信息
			resTable.Clear();
			resTable.CopyColumnInfoFrom(resTableMx);

			//筛选
			resTableMx.EqualsAt(resTable,ColArr,nColCount,0,iSelect);

			delete ColArr;
			ColArr = NULL;

			return true;
		}
		//add by lijw 2008-02-29
		//基金申购赎回费率表
		bool TxFund::StatFundFeeRate(
			Tx::Core::Table_Indicator& resTable,	//结果数据表
			std::vector<int>& iFundSecurityId,		//基金交易实体ID
			int iStartDate,						//起始日期
			int iEndDate,							//终止日期
			int  FeeType,		                    //费率模式
			bool IsAppoint						   //是否是指定日期
			)
		{
			//添加进度条
			// ProgressWnd* pwd=Tx::Core::ProgressWnd::GetInstance();
			ProgressWnd prw;
			UINT pid=prw.AddItem(1,_T("申购赎回费率统计..."),0.0);
			prw.Show(1000);
			//默认的返回值状态。
			bool result = true;
			//清空数据
			m_txTable.Clear();

			//基金的交易实体id ，int型
			m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
			//公告日期参数F_disclosureDATE, int型
			m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
			//费率类型
			m_txTable.AddParameterColumn(Tx::Core::dtype_byte);
			//费率模式
			m_txTable.AddParameterColumn(Tx::Core::dtype_val_string);
			//序号参数f_no, int型
			m_txTable.AddParameterColumn(Tx::Core::dtype_byte);

			UINT varCfg[5];			//参数配置
			int varCount=5;			//参数个数	交易实体id，公告日
			const int INDICATOR_INDEX=6;
			long iIndicator[INDICATOR_INDEX]=
			{			
				31000001,	//开始日期
				31000002,	//截止日期
				31000003,	//申购金额条件
				31000004,	//持有时间条件
				31000006,	//费率
				31000007	//固定费用
			};

			//设定指标列
			for (int i = 0; i <	INDICATOR_INDEX; i++)
			{
				GetIndicatorDataNow(iIndicator[i]);

				varCfg[0]=0;
				varCfg[1]=1;
				varCfg[2]=2;
				varCfg[3]=3;
				varCfg[4]=4;
				result = m_pLogicalBusiness->SetIndicatorIntoTable(
					m_pIndicatorData,	//指标
					varCfg,				//参数配置
					varCount,			//参数个数
					m_txTable			//计算需要的参数传输载体以及计算后结果的载体
					);
				if(result==false)
					break;
			}
			if(result==false)
				return false;
			//根据之前3个步骤的设置进行数据读取，结果数据存放在table中
			result = m_pLogicalBusiness->GetData(m_txTable,true);
			if(result==false ||m_txTable.GetRowCount() == 0)
			{
				//添加进度条
				prw.SetPercent(pid,1.0);
				return false;
			}
			Tx::Core::Table_Indicator resTableType;
			UINT iCol=m_txTable.GetColCount();
			//复制所有列信息
			resTable.CopyColumnInfoFrom(m_txTable);
			resTableType.CopyColumnInfoFrom(m_txTable);
			UINT* nColArray = new UINT[iCol];
			for(UINT i=0;i<iCol;i++)
				nColArray[i]=i;
			//根据实体id取出数据
			if(m_txTable.GetRowCount()>0)
			{
				m_txTable.EqualsAt(resTable,nColArray,iCol,0,iFundSecurityId);
			}
			else
			{
				delete nColArray;
				nColArray = NULL;
				//添加进度条 
				prw.SetPercent(pid,1.0);
				return false;
			}
			if(resTable.GetRowCount() == 0)
			{
				delete nColArray;
				nColArray = NULL;
				//添加进度条
				prw.SetPercent(pid,1.0);
				return false;
			}
			CString strTable4;
			//	//当是最新日期时
			//	if(IsAppoint)
			//	{
			//		//对resTable表里的数据进行排序分别按照基金交易实体ID、开始日期
			//		MultiSortRule multisort;
			//		multisort.AddRule(0,true);
			//		multisort.AddRule(5,false);
			//		resTable.SortInMultiCol(multisort);
			//		resTable.Arrange();
			//#ifdef _DEBUG
			//		strTable4=resTable.TableToString();
			//		Tx::Core::Commonality::String().StringToClipboard(strTable4);
			//#endif
			//		//取出每个样本的最新日期的数据
			//		Tx::Core::Table_Indicator tempTable,resTableDate;
			//		tempTable.CopyColumnInfoFrom(resTable);
			//		resTableDate.CopyColumnInfoFrom(resTable);
			//		std::vector<int>::iterator iterV;
			//		std::vector<int> tempVector;
			//		int iRowCount;
			//		int date;
			//		std::vector<int> VecDate;
			//		for(iterV = iFundSecurityId.begin();iterV != iFundSecurityId.end();++iterV)
			//		{
			//			if(!tempVector.empty())
			//				tempVector.clear();
			//			tempVector.push_back(*iterV);
			//			//首先取出每个样本的数据
			//			iRowCount = tempTable.GetRowCount();
			//			if(iRowCount > 0)
			//			{
			//				tempTable.DeleteRow(0,iRowCount);
			//				tempTable.Arrange();
			//			}
			//			resTable.EqualsAt(tempTable,nColArray,iCol,0,tempVector);
			//			if(0 == tempTable.GetRowCount())
			//				continue;
			//			tempTable.GetCell(5,0,date);
			//			if(!VecDate.empty())
			//				VecDate.clear();
			//			VecDate.push_back(date);
			//			iRowCount = resTableDate.GetRowCount();
			//			if(iRowCount >0)
			//			{
			//				resTableDate.DeleteRow(0,iRowCount);
			//				resTableDate.Arrange();
			//			}
			//			//取得起始日期最大的那天的所有纪录。
			//			tempTable.EqualsAt(resTableDate,nColArray,iCol,5,VecDate);	
			//			resTableType.AppendTableByRow(resTableDate,false);
			//		}		
			//		if(resTableType.GetRowCount() == 0)
			//		{
			//			delete nColArray;
			//			nColArray = NULL;
			//			//添加进度条
			//			prw.SetPercent(pid,1.0);
			//			return false;
			//		}
			//#ifdef _DEBUG
			//		strTable4=resTableType.TableToString();
			//		Tx::Core::Commonality::String().StringToClipboard(strTable4);
			//#endif
			//		//添加进度条
			//		prw.SetPercent(pid,0.6);
			//	}
			//	//取得指定时间区间内的记录数据	
			//	else
			//	{		
			//		resTable.Between(resTableType,nColArray,iCol,5,iStartDate,iEndDate,true,true);
			//#ifdef _DEBUG
			//		strTable4=resTableType.TableToString();
			//		Tx::Core::Commonality::String().StringToClipboard(strTable4);
			//#endif
			//		//添加进度条
			//		prw.SetPercent(pid,0.6);
			//		if(resTableType.GetRowCount() == 0)
			//		{
			//			delete nColArray;
			//			nColArray = NULL;
			//			//添加进度条
			//			prw.SetPercent(pid,1.0);
			//			return false;
			//		}	
			//	}
			//modied by zhangxs 20090109
			//由取最新数据和指定日期数据改为-->取全部日期数据和指定日期数据
			if(!IsAppoint)
			{
				resTableType.Clear();
				resTableType.Clone(resTable);
#ifdef _DEBUG
				strTable4=resTableType.TableToString();
				Tx::Core::Commonality::String().StringToClipboard(strTable4);
#endif
				if(resTableType.GetRowCount() == 0)
				{
					delete nColArray;
					nColArray = NULL;
					//添加进度条
					prw.SetPercent(pid,1.0);
					return false;
				}
			}
			else
			{		
				resTable.Between(resTableType,nColArray,iCol,5,iStartDate,iEndDate,true,true);
#ifdef _DEBUG
				strTable4=resTableType.TableToString();
				Tx::Core::Commonality::String().StringToClipboard(strTable4);
#endif
				//添加进度条
				prw.SetPercent(pid,0.6);
				if(resTableType.GetRowCount() == 0)
				{
					delete nColArray;
					nColArray = NULL;
					//添加进度条
					prw.SetPercent(pid,1.0);
					return false;
				}	
			}	
			std::set<byte> typeSet;
			switch(FeeType)
			{
			case 1:
				typeSet.insert(0);//表示认购
				break;
			case 2:
				typeSet.insert(1);//表示申购
				break;
			case 4:
				typeSet.insert(2);//表示赎回
				break;
			case 3:
				typeSet.insert(0);
				typeSet.insert(1);
				break;
			case 5:
				typeSet.insert(0);
				typeSet.insert(2);
				break;
			case 6:
				typeSet.insert(1);
				typeSet.insert(2);
				break;
			case 7:
				typeSet.insert(0);
				typeSet.insert(1);
				typeSet.insert(2);
				break;
			default:
				AfxMessageBox(_T("费率类型出错"));
				break;
			}	
			resTable.Clear();
			resTable.CopyColumnInfoFrom(resTableType);
			//根据费率类型来取数据
			resTableType.EqualsAt(resTable,nColArray,iCol,2,typeSet);
			MultiSortRule multisort;
			multisort.AddRule(0,true);
			multisort.AddRule(1,false);
			resTable.SortInMultiCol(multisort);
			resTable.Arrange();
#ifdef _DEBUG
			strTable4=resTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable4);
#endif
			if (nColArray != NULL)
			{
				delete nColArray;
				nColArray = NULL;
			}	
			//插入这两列为了插入名称和代码。
			resTable.InsertCol(1,Tx::Core::dtype_val_string);//插入名称
			resTable.InsertCol(2,Tx::Core::dtype_val_string);//插入代码
			//插入费率类型和费率模式的CString列
			resTable.InsertCol(9,Tx::Core::dtype_val_string);//插入费率类型
			resTable.InsertCol(10,Tx::Core::dtype_val_string);//插入费率模式
			//插入为序号换列的一列
			resTable.InsertCol(11,Tx::Core::dtype_byte);//插入备用的序号列
			//为了把固定费用转化成符合规范的格式。
			resTable.AddCol(Tx::Core::dtype_val_string);
			//把基金的名称和代码添加到表里,以及把固定费用转化为CString类型
			int transactionId;
			double dFixFee;
			CString strFee;
			for(int i = 0;i < (int)resTable.GetRowCount();i++)
			{
				//取得交易实体ID
				resTable.GetCell(0,i,transactionId);
				GetSecurityNow(transactionId);
				if(m_pSecurity==NULL)
					continue;
				//根据交易实体ID取得样本的名称和外码；
				CString strName,strCode;
				strName = m_pSecurity->GetName();
				strCode = m_pSecurity->GetCode();
				resTable.SetCell(1,i,strName);
				resTable.SetCell(2,i,strCode);
				//取得固定费用的值
				resTable.GetCell(15,i,dFixFee);
				if(dFixFee == Tx::Core::Con_doubleInvalid || dFixFee == 0)
					resTable.SetCell(16,i,Tx::Core::Con_strInvalid);
				else
				{
					strFee.Format(_T("%.2f"),dFixFee);
					resTable.SetCell(16,i,strFee);
				}
				//移动序号的位置
				byte no;
				resTable.GetCell(6,i,no);
				resTable.SetCell(11,i,no);
				//取得费率类型和费率模式
				byte feetype;
				CString feemode;
				resTable.GetCell(4,i,feetype);
				resTable.GetCell(5,i,feemode);
				CString strFeeType,strFeeMode;
				switch(feetype)
				{
				case 0:
					strFeeType = _T("认购类型");
					resTable.SetCell(9,i,strFeeType);
					break;
				case 1:
					strFeeType = _T("申购类型");
					resTable.SetCell(9,i,strFeeType);
					break;
				case 2:
					strFeeType = _T("赎回类型");
					resTable.SetCell(9,i,strFeeType);
					break;
				}
				if(feemode == _T("f"))
				{
					strFeeMode = _T("前端收费");
					resTable.SetCell(10,i,strFeeMode);
				}
				if(feemode == _T("b"))
				{
					strFeeMode = _T("后端收费");
					resTable.SetCell(10,i,strFeeMode);
				}
			}
			resTable.DeleteCol(15);//删除固定费用的double列
			//删除费用模式的字符列（既是内容是“f”或“t”）//费用类型的int列和公告日期，序号列（作为参数时）
			resTable.DeleteCol(3,4);
#ifdef _DEBUG
			strTable4=resTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable4);
#endif
			//添加进度条
			prw.SetPercent(pid,1.0);
			return true;

		}
		//	//基金申购赎回费率表
		//bool TxFund::StatFundFeeRate(
		//			Tx::Core::Table_Indicator& resTable,	//结果数据表
		//			std::vector<int>& iFundSecurityId,		//基金交易实体ID
		//			int iStart,							//起始日期
		//			int iEnd,							//终止日期
		//			bool IsAppoint,						//是否是指定日期
		//			)
		//{
		//	int iMenuId = 10485;
		//
		//	m_IWPA.clear();
		//	//实体id-〉基金id
		//	std::vector<int>	iFundId;
		//
		//	Tx::Core::Table_Indicator resTable0,resTable1;	//指标参数
		//	//std::vector<int> iParameter;
		//
		//	////指标
		//	//for(int i=0;i<7;i++)
		//	//{
		//	//	iParameter.push_back(31000001+i);
		//	//}
		//
		//	////设定需要的指标
		//	//Tx::Business::IndicatorFile::GetInstance()->SetIWAP(m_IWPA,iParameter);
		//	////取得所有数据
		//	//Tx::Business::IndicatorFile::GetInstance()->GetData(m_IWPA,iFundId,true);
		//
		//
		//	////复制数据列信息
		//	//resTable0.CopyColumnInfoFrom(m_IWPA.m_table_indicator);
		//	////复制数据
		//	//resTable0.Clone(m_IWPA.m_table_indicator);
		//
		//
		//		resTable0.AddParameterColumn(Tx::Core::dtype_int4);
		//		resTable0.AddParameterColumn(Tx::Core::dtype_int4);
		//		resTable0.AddParameterColumn(Tx::Core::dtype_byte);
		//		resTable0.AddParameterColumn(Tx::Core::dtype_val_string);
		//		resTable0.AddParameterColumn(Tx::Core::dtype_byte);
		//
		//		
		//		const int INDICATOR_INDEX=7;
		//		int varCount=5;
		//		long iIndicator[INDICATOR_INDEX];
		//		
		//		for(int i=0;i<INDICATOR_INDEX;i++)
		//		{			
		//			iIndicator[i]=31000001+i;
		//		};
		//		bool result;
		//		//设定指标列
		//		for (int i = 0; i <	INDICATOR_INDEX; i++)
		//		{
		//			GetIndicatorDataNow(iIndicator[i]);
		//		
		//
		//
		//
		//			result = m_pLogicalBusiness->SetIndicatorIntoTable(
		//														m_pIndicatorData,	//指标
		//														varCfg,				//参数配置
		//														varCount,			//参数个数
		//														resTable0			//计算需要的参数传输载体以及计算后结果的载体
		//						);
		//			if(result==false)
		//				break;
		//		}
		//	result=m_pLogicalBusiness->GetData(resTable0,true);
		//	if(resTable0.GetRowCount()<=0)
		//		return false;
		//
		//	//创建列信息，准备筛选
		//	int nColCount = resTable0.GetColCount();
		//	UINT* nColArray = new UINT[nColCount];
		//	for(int i=0;i<nColCount;i++)
		//		nColArray[i]=i;
		//
		//	//筛选起始日期col=5
		//	resTable1.CopyColumnInfoFrom(resTable0);
		//	resTable0.Between(resTable1,nColArray,nColCount,5,iStart,iEnd);
		//
		//	//筛选样本col=0
		//	resTable.CopyColumnInfoFrom(resTable1);
		//	if(resTable1.GetRowCount()>0)
		//		resTable1.EqualsAt(resTable,nColArray,nColCount,0,iFundSecurityId);
		//	this->IdColToNameAndCode(resTable,0,0);
		//	delete nColArray;
		//	nColArray = NULL;
		//
		//	return true;
		//}
		//排序分析数据结果标题列
		bool TxFund::GetBlockAnalysisCol(Table_Display& baTable,std::vector<int>& arrSamples,int& iSortCol)
		{
			if(TxBusiness::GetBlockAnalysisCol(baTable,arrSamples,iSortCol)==false)
				return false;

			int nCol = 12;
			//2008-10-31
			//基金价格精度取3位
			baTable.SetPrecise(5, 3);
			baTable.SetPrecise(6, 3);
			//2008-11-17
			//基金涨跌精度取3位
			baTable.SetPrecise(7, 3);

			baTable.InsertCol(nCol,Tx::Core::dtype_double);
			baTable.SetTitle(nCol, _T("净值"));
			baTable.SetFormatStyle(nCol, Tx::Core::fs_finance);
			baTable.SetPrecise(nCol, 4);
			return true;
		}
		//排序分析数据结果设置
		bool TxFund::SetBlockAnalysisCol(Table_Display& baTable,SecurityQuotation* psq,int& j,int ii)
		{
			if(psq==NULL)
				return false;

			double dNAV = psq->GetFundNAV();

			int idate = psq->GetCurDataDate();
			float closePrice = psq->GetClosePrice(idate,true);
			//float closePrice = psq->GetClosePrice(true);
			double share = psq->GetTotalShare();
			double tradeableShare = psq->GetTradableShare();
			double dValue = Con_doubleInvalid;

			//净值
			baTable.SetCell(j,ii,dNAV);
			j++;
			//流通股本
			baTable.SetCell(j,ii,tradeableShare);
			j++;
			//总股本
			baTable.SetCell(j,ii,share);
			j++;
			//流通市值
			if(psq->HaveDetailData()==true)
			{

				if(tradeableShare>0 && closePrice>0)
					dValue = closePrice*tradeableShare;
				else
					//流通股本不可能小于0
					dValue = Tx::Core::Con_doubleInvalid;
			}
			else
			{
				if(tradeableShare>0 && dNAV>0)
					dValue = dNAV*tradeableShare;
				else
					//流通股本不可能小于0
					dValue = Tx::Core::Con_doubleInvalid;
			}
			baTable.SetCell(j,ii,dValue);
			j++;
			//总市值
			if(psq->IsFund_Estimate())
			{
				if(share>0 && dNAV>0)
					dValue = dNAV*share;
				else
					dValue = Tx::Core::Con_doubleInvalid;
			}
			else if(psq->HaveDetailData()==true)
			{
				if(share>0 && closePrice>0)
					dValue = closePrice*share;
				else
					dValue = Tx::Core::Con_doubleInvalid;
			}
			else
			{
				//
				if(share>0 && dNAV>0)
					dValue = dNAV*share;
				else
					dValue = Tx::Core::Con_doubleInvalid;
			}
			baTable.SetCell(j,ii,dValue);
			j++;
			return true;
		}
		//基金净值涨幅
		bool TxFund::GetFundNavRaise(
			std::vector<int>& iSecurityId,		//交易实体ID
			int date,						//数据日期,0表示最近交易日
			Tx::Core::Table_Display& resTable,//结果数据表
			bool bPrice
			)
		{
			//构建两市场交易日期序列
			if(GetHuShenTradeDate()==false)
			{
				AfxMessageBox(_T("大盘交易日序列数据错误！"));
				return false;
			}

			//4000208=证指数
			//取得上证指数的交易实体指针
			//Security* pBaseSecurity;
			//pBaseSecurity = m_pShIndex;
			int iMarketId = 0;
			if(iMarketId ==0)
				iMarketId = m_pFunctionDataManager->GetBourseId_ShangHai();

			int iToday = m_pFunctionDataManager->GetServerCurDateTime(iMarketId).GetDate().GetInt();

			//取得两市场的起始日期和最新日期
			int iMixAllCount = m_pMixTradeDay->GetTradeDayCount();
			int iBaseStartDate = m_pMixTradeDay->GetDateByIndex(0);
			int iBaseEndDate = m_pMixTradeDay->GetLatestDate();

			if(date<=0)
			{
				//取得最近上个交易日期
				if(iBaseEndDate == iToday)
				{
					//今天是交易日期
					//大盘已经完成收盘处理
					date = m_pMixTradeDay->GetDateByIndex(iMixAllCount-2);
				}
				else
				{
					//尚未完成收盘处理
					date = m_pMixTradeDay->GetDateByIndex(iMixAllCount-1);
				}
			}
			else
			{
				//指定了历史日期
				if(m_pMixTradeDay->GetIndexByDate(date)<0)
				{
					//取得最近上个交易日期
					if(iBaseEndDate == iToday)
					{
						//今天是交易日期
						//大盘已经完成收盘处理
						date = m_pMixTradeDay->GetDateByIndex(iMixAllCount-2);
					}
					else
					{
						//尚未完成收盘处理
						date = m_pMixTradeDay->GetDateByIndex(iMixAllCount-1);
					}
				}
			}

			int ii = iSecurityId.size();

			//step1
			//	Tx::Core::ProgressWnd* pProgressWnd = Tx::Core::ProgressWnd::GetInstance();
			Tx::Core::ProgressWnd prw;
			//step2
			CString sProgressPrompt;
			sProgressPrompt.Format(_T("基金净值涨幅..."));
			UINT progId = prw.AddItem(1,sProgressPrompt, 0.0);
			//step3
			prw.Show(15);
			//step4
			prw.SetPercent(progId, 0.1);

			//默认的返回值状态
			bool result = false;
			//清空数据
			resTable.Clear();
			int iCol = 0;

			//准备样本集=Security_ID,int型
			resTable.AddCol(Tx::Core::dtype_int4);
			iCol++;
			//样本名称
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.SetTitle(iCol,_T("名称"));
			iCol++;
			//样本外码
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.SetTitle(iCol,_T("代码"));
			iCol++;
			//数据日期
			resTable.AddCol(Tx::Core::dtype_int4);
			resTable.SetTitle(iCol,_T("日期"));
			resTable.SetFormatStyle(iCol, fs_date);
			iCol++;

			//最近日净值增长率double
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("最近日净值增长率"));
			iCol++;
			//最近一周净值增长率double
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("最近一周净值增长率"));
			iCol++;
			//最近一个月净值增长率double
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("最近一个月净值增长率"));
			iCol++;
			//最近三个月净值增长率double
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("最近三个月净值增长率"));
			iCol++;
			//最近六个月净值增长率double
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("最近六个月净值增长率"));
			iCol++;
			//最近一年净值增长率double
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("最近一年净值增长率"));
			iCol++;
			//最近二年净值增长率double
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("最近二年净值增长率"));
			iCol++;
			//今年以来净值增长率double
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("今年以来净值增长率"));
			iCol++;
			//设立以来净值增长率double
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("设立以来净值增长率"));
			iCol++;


			//添加券ID
			int j=0;
			for(std::vector<int>::iterator iter = iSecurityId.begin();iter!=iSecurityId.end();iter++)
			{
				SecurityQuotation* pSecurity = GetSecurityNow(*iter);
				if(pSecurity!=NULL)
				{
					j = resTable.GetRowCount();
					//根据样本数量添加记录数
					resTable.AddRow();
					int nCol = 0;
					//交易实体id
					resTable.SetCell(nCol,j,*iter);
					nCol++;
					//填充名称
					resTable.SetCell(nCol,j,pSecurity->GetName());
					nCol++;
					//填充外码
					resTable.SetCell(nCol,j,pSecurity->GetCode());
					nCol++;
					//数据日期
					resTable.SetCell(nCol,j,date);
					nCol++;

					FundNavRaise* pFundNavRaise = NULL;
					if(pSecurity->GetGlobalData(gdt_FundNavRaise,&pFundNavRaise)==false || pFundNavRaise==NULL)
					{
						double dValue = Con_doubleInvalid;
						//最近日净值增长率double
						resTable.SetCell(nCol,j,dValue);
						nCol++;
						//最近一周净值增长率double
						resTable.SetCell(nCol,j,dValue);
						nCol++;
						//最近一个月净值增长率double
						resTable.SetCell(nCol,j,dValue);
						nCol++;
						//最近三个月净值增长率double
						resTable.SetCell(nCol,j,dValue);
						nCol++;
						//最近六个月净值增长率double
						resTable.SetCell(nCol,j,dValue);
						nCol++;
						//最近一年净值增长率double
						resTable.SetCell(nCol,j,dValue);
						nCol++;
						//最近二年净值增长率double
						resTable.SetCell(nCol,j,dValue);
						nCol++;
						//今年以来净值增长率double
						resTable.SetCell(nCol,j,dValue);
						nCol++;
						//设立以来净值增长率double
						resTable.SetCell(nCol,j,dValue);
						nCol++;
						continue;
					}

					//最近日净值增长率double
					resTable.SetCell(nCol,j,pFundNavRaise->f_nav_return_cur_day);
					nCol++;
					//最近一周净值增长率double
					resTable.SetCell(nCol,j,pFundNavRaise->f_nav_return_latest_week);
					nCol++;
					//最近一个月净值增长率double
					resTable.SetCell(nCol,j,pFundNavRaise->f_nav_return_lastet_month);
					nCol++;
					//最近三个月净值增长率double
					resTable.SetCell(nCol,j,pFundNavRaise->f_nav_return_latest_3M);
					nCol++;
					//最近六个月净值增长率double
					resTable.SetCell(nCol,j,pFundNavRaise->f_nav_return_latest_6M);
					nCol++;
					//最近一年净值增长率double
					resTable.SetCell(nCol,j,pFundNavRaise->f_nav_return_latest_1Y);
					nCol++;
					//最近二年净值增长率double
					resTable.SetCell(nCol,j,pFundNavRaise->f_nav_return_latest_2Y);
					nCol++;
					//今年以来净值增长率double
					resTable.SetCell(nCol,j,pFundNavRaise->f_nav_return_latest_this_year);
					nCol++;
					//设立以来净值增长率double
					resTable.SetCell(nCol,j,pFundNavRaise->f_nav_return_from_setup);
					nCol++;
				}
				else
				{
#ifdef _DEBUG
					GlobalWatch::_GetInstance()->WatchHere(_T("bond security id = %d non-exist"),*iter);
#endif
				}
			}

			prw.SetPercent(progId, 1.0);
			sProgressPrompt+=_T(",完成!");
			prw.SetText(progId, sProgressPrompt);

			return true;
		}
		//基金的阶段行情   add by lijw 2008-09-11
		bool TxFund::FundPhaseMarket(std::vector<int> &samples,Tx::Core::Table_Indicator& resTable)

		{
			//添加进度条
			ProgressWnd prw;
			UINT pid=prw.AddItem(1,_T("基金的阶段行情..."),0.0);
			prw.Show(1000);
			//默认的返回值状态。
			bool result = true;
			//	Tx::Core::Table_Display m_txTable;
			m_txTable.Clear();
			//基金的交易实体id ，int型
			m_txTable.AddParameterColumn(Tx::Core::dtype_int4);
			////日期参数int型
			//m_txTable.AddParameterColumn(Tx::Core::dtype_int4);

			UINT varCfg[1];			//参数配置
			int varCount=1;			//参数个数	交易实体id，日期
			const int INDICATOR_INDEX = 5;
			long iIndicator[INDICATOR_INDEX]=
			{			
				30300044,	//过去一周涨幅
				30300045,	//过去一个月涨幅
				30300046,	//过去三个月涨幅
				30300047,	//过去六个月涨幅
				30300048,	//过去一年涨幅
			};

			//设定指标列
			for (int i = 0; i <	INDICATOR_INDEX; i++)
			{
				GetIndicatorDataNow(iIndicator[i]);

				varCfg[0]=0;
				result = m_pLogicalBusiness->SetIndicatorIntoTable(
					m_pIndicatorData,	//指标
					varCfg,				//参数配置
					varCount,			//参数个数
					m_txTable			//计算需要的参数传输载体以及计算后结果的载体
					);
				if(result==false)
					break;
			}
			if(result==false)
				return false;
			//根据之前3个步骤的设置进行数据读取，结果数据存放在table中
			result = m_pLogicalBusiness->GetData(m_txTable,true);
			if(result==false ||m_txTable.GetRowCount() == 0)
			{
				//添加进度条
				prw.SetPercent(pid,1.0);
				return false;
			}
			UINT iCol=m_txTable.GetColCount();
			//复制所有列信息
			resTable.CopyColumnInfoFrom(m_txTable);
			UINT* nColArray = new UINT[iCol];
			for(UINT i=0;i<iCol;i++)
				nColArray[i]=i;
			//根据实体id取出数据
			if(m_txTable.GetRowCount()>0)
			{
				m_txTable.EqualsAt(resTable,nColArray,iCol,0,samples);
			}
			else
			{
				//添加进度条 
				prw.SetPercent(pid,1.0);
				return false;
			}
			delete nColArray;
			if(resTable.GetRowCount() == 0)
			{
				//添加进度条
				prw.SetPercent(pid,1.0);
				return false;
			}
			CString strTable4;
#ifdef _DEBUG
			strTable4=resTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable4);
#endif
			//添加进度条
			prw.SetPercent(pid,0.6);	
			//插入这两列为了插入名称和代码。
			resTable.InsertCol(1,Tx::Core::dtype_val_string);//插入名称
			resTable.InsertCol(2,Tx::Core::dtype_val_string);//插入代码
			resTable.InsertCol(3,Tx::Core::dtype_double);//4现价
			resTable.InsertCol(4,Tx::Core::dtype_double);//5日涨幅
			resTable.AddCol(Tx::Core::dtype_int4);//11成立日期
			resTable.AddCol(Tx::Core::dtype_val_string);//12基金公司
			//把基金的名称和代码添加到表里
			std::unordered_map<int,CString> InStitutionIdToName;
			std::unordered_map<int,CString>::iterator iter;
			TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_INSTITUTIONID_TO_SHORTNAME,InStitutionIdToName);
			int transactionId;
			CString strName,strCode;
			FundNewInfo *pfundInfo;
			int date,iInstitutionId;
			double CurrentPrice,PreClosePrice;
			double dRise;
			for(int i = 0;i < (int)resTable.GetRowCount();i++)
			{
				//取得交易实体ID
				resTable.GetCell(0,i,transactionId);
				GetSecurityNow(transactionId);
				if(m_pSecurity==NULL)
					continue;
				//根据交易实体ID取得样本的名称和外码；		
				strName = m_pSecurity->GetName();
				strCode = m_pSecurity->GetCode();
				resTable.SetCell(1,i,strName);
				resTable.SetCell(2,i,strCode);
				PreClosePrice = m_pSecurity->GetPreClosePrice();
				CurrentPrice = m_pSecurity->GetClosePrice();
				if (CurrentPrice <= 0)
				{
					CurrentPrice = Tx::Core::Con_doubleInvalid;
				}
				resTable.SetCell(3,i,CurrentPrice);		
				if (CurrentPrice <= 0 || PreClosePrice <= 0)
					dRise = Tx::Core::Con_doubleInvalid;
				else
					dRise = (CurrentPrice - PreClosePrice)/PreClosePrice*100;
				resTable.SetCell(4,i,dRise);		
				pfundInfo = m_pSecurity->GetFundNewInfo();
				date = pfundInfo->setup_date;
				resTable.SetCell(10,i,date);
				iInstitutionId = m_pSecurity->GetInstitutionId();	
				iter = InStitutionIdToName.find(iInstitutionId);
				if(iter != InStitutionIdToName.end())
					resTable.SetCell(11,i,iter->second);
			}
#ifdef _DEBUG
			strTable4=resTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable4);
#endif
			//添加进度条
			prw.SetPercent(pid,1.0);
			return true;

		}
		//add by lijw 2008-09-08
		//取得封闭式基金净值增长率
		bool TxFund::GetCloseFundNavRaise(
			std::vector<int>& iSecurityId,		//交易实体ID
			//int date,						//数据日期,0表示最近交易日
			Tx::Core::Table_Display& resTable,//结果数据表
			int& iDate,
			bool bPrice
			)
		{
			//构建两市场交易日期序列
			if(GetHuShenTradeDate()==false)
			{
				AfxMessageBox(_T("大盘交易日序列数据错误！"));
				return false;
			}
			int iMarketId = 0;
			if(iMarketId ==0)
				iMarketId = m_pFunctionDataManager->GetBourseId_ShangHai();

			int iToday = m_pFunctionDataManager->GetServerCurDateTime(iMarketId).GetDate().GetInt();

			//取得两市场的起始日期和最新日期
			int iMixAllCount = m_pMixTradeDay->GetTradeDayCount();
			int iBaseStartDate = m_pMixTradeDay->GetDateByIndex(0);
			int iBaseEndDate = m_pMixTradeDay->GetLatestDate();

			if(iDate<=0)
			{
				//取得最近上个交易日期
				if(iBaseEndDate == iToday)
				{
					//今天是交易日期
					//大盘已经完成收盘处理
					iDate = m_pMixTradeDay->GetDateByIndex(iMixAllCount-2);
				}
				else
				{
					//尚未完成收盘处理
					iDate = m_pMixTradeDay->GetDateByIndex(iMixAllCount-1);
				}
			}
			else
			{
				//指定了历史日期
				if(m_pMixTradeDay->GetIndexByDate(iDate)<0)
				{
					//取得最近上个交易日期
					if(iBaseEndDate == iToday)
					{
						//今天是交易日期
						//大盘已经完成收盘处理
						iDate = m_pMixTradeDay->GetDateByIndex(iMixAllCount-2);
					}
					else
					{
						//尚未完成收盘处理
						iDate = m_pMixTradeDay->GetDateByIndex(iMixAllCount-1);
					}
				}
			}

			int ii = iSecurityId.size();

			//用于取当日基金单位和累计净值 
			DataFileNormal<blk_TxExFile_FileHead,FundNav2Day>* pFundNav2Day = new DataFileNormal<blk_TxExFile_FileHead,FundNav2Day>;
			if(pFundNav2Day==NULL)
				return false;
			//取得扩展数据目录id
			int iFundNav2DaysId = 
				Tx::Data::DataStatus::GetInstance()->GetExFileIdFromIni(
				Tx::Core::SystemPath::GetInstance()->GetSystemDataPath(),
				_T("fund_nav_2days"));

			int iFundNav2DaysFileId_last = -1;

			//step1
			//	Tx::Core::ProgressWnd* pProgressWnd = Tx::Core::ProgressWnd::GetInstance();
			Tx::Core::ProgressWnd prw;
			//step2
			CString sProgressPrompt;
			sProgressPrompt.Format(_T("基金净值增长率..."));
			UINT progId = prw.AddItem(1,sProgressPrompt, 0.0);
			//step3
			prw.Show(15);
			//step4
			prw.SetPercent(progId, 0.1);

			//默认的返回值状态
			bool result = false;
			//清空数据
			resTable.Clear();
			int iCol = 0;

			//准备样本集=Security_ID,int型
			resTable.AddCol(Tx::Core::dtype_int4);
			iCol++;
			//样本名称
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.SetTitle(iCol,_T("名称"));
			iCol++;
			//样本外码
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.SetTitle(iCol,_T("代码"));
			iCol++;

			//单位净值
			resTable.AddCol(Tx::Core::dtype_float);
			resTable.SetTitle(iCol,_T("单位净值"));
			resTable.SetPrecise(iCol,4);
			iCol++;

			//累计净值
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("累计净值"));
			resTable.SetPrecise(iCol,4);
			iCol++;

			////最近日净值增长率double
			//resTable.AddCol(Tx::Core::dtype_double);
			//resTable.SetTitle(iCol,_T("最新净值增长率%"));
			//iCol++;
			//最近一周净值增长率double
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("最近一周净值增长率%"));
			iCol++;
			//最近一个月净值增长率double
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("最近一个月净值增长率%"));
			iCol++;
			//最近三个月净值增长率double
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("最近三个月净值增长率%"));
			iCol++;
			//最近六个月净值增长率double
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("最近六个月净值增长率%"));
			iCol++;
			//最近一年净值增长率double
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("最近一年净值增长率%"));
			iCol++;
			//最近二年净值增长率double
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("最近二年净值增长率%"));
			iCol++;
			//今年以来净值增长率double
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("今年以来净值增长率%"));
			iCol++;
			//设立以来净值增长率double
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("设立以来净值增长率%"));
			iCol++;


			//添加券ID
			int j=0;
			for(std::vector<int>::iterator iter = iSecurityId.begin();iter!=iSecurityId.end();iter++)
			{
				SecurityQuotation* pSecurity = GetSecurityNow(*iter);
				if(pSecurity!=NULL)
				{
					CString sFileIdKey;
					if(pSecurity->IsFund_Close()==false)
					{
						continue;
					}
					sFileIdKey = _T("fund_close_nav_2days");	

					j = resTable.GetRowCount();
					//根据样本数量添加记录数
					resTable.AddRow();
					int nCol = 0;
					//交易实体id
					resTable.SetCell(nCol,j,*iter);
					nCol++;
					//填充名称
					resTable.SetCell(nCol,j,pSecurity->GetName());
					nCol++;
					//填充外码
					resTable.SetCell(nCol,j,pSecurity->GetCode());
					nCol++;


					//用于取当日基金单位和累计净值 
					int iFundNav2DaysFileId = 
						Tx::Data::DataStatus::GetInstance()->GetExFileIdFromIni(
						Tx::Core::SystemPath::GetInstance()->GetSystemDataPath(),
						sFileIdKey);
					if(iFundNav2DaysFileId_last != iFundNav2DaysFileId)
					{
						pFundNav2Day->Reset();
					}
					if(pFundNav2Day->Load(
						iFundNav2DaysFileId,//文件名=2020202.dat
						iFundNav2DaysId,//文件所在目录
						true)==false || pFundNav2Day->GetDataCount()<=1)
					{
						iFundNav2DaysFileId_last = iFundNav2DaysFileId;
						continue;
					}
					//记录上次文件id
					iFundNav2DaysFileId_last = iFundNav2DaysFileId;

					FundNav2Day* pFundNav2DayData = pFundNav2Day->GetDataByObj(*iter,false);

					if(pFundNav2DayData==NULL)
					{
						//单位净值
						resTable.SetCell(nCol,j,Con_floatInvalid);
						nCol++;
						//累计净值
						resTable.SetCell(nCol,j,Con_doubleInvalid);
						nCol++;
					}
					else
					{
						//单位净值
						resTable.SetCell(nCol,j,pFundNav2DayData->fNav1);
						nCol++;
						//累计净值
						resTable.SetCell(nCol,j,pFundNav2DayData->fSumNav1);
						nCol++;
					}

					FundNavRaise* pFundNavRaise = NULL;
					if(pSecurity->GetGlobalData(gdt_FundNavRaise,&pFundNavRaise)==false || pFundNavRaise==NULL)
					{
						double dValue = Con_doubleInvalid;
						////最近日净值增长率double
						//resTable.SetCell(nCol,j,dValue);
						//nCol++;
						//最近一周净值增长率double
						resTable.SetCell(nCol,j,dValue);
						nCol++;
						//最近一个月净值增长率double
						resTable.SetCell(nCol,j,dValue);
						nCol++;
						//最近三个月净值增长率double
						resTable.SetCell(nCol,j,dValue);
						nCol++;
						//最近六个月净值增长率double
						resTable.SetCell(nCol,j,dValue);
						nCol++;
						//最近一年净值增长率double
						resTable.SetCell(nCol,j,dValue);
						nCol++;
						//最近二年净值增长率double
						resTable.SetCell(nCol,j,dValue);
						nCol++;
						//今年以来净值增长率double
						resTable.SetCell(nCol,j,dValue);
						nCol++;
						//设立以来净值增长率double
						resTable.SetCell(nCol,j,dValue);
						nCol++;
						continue;
					}

					iDate = pFundNavRaise->f_trade_date;
					////最近日净值增长率double
					//resTable.SetCell(nCol,j,pFundNavRaise->f_nav_return_cur_day);
					//nCol++;
					//最近一周净值增长率double
					resTable.SetCell(nCol,j,pFundNavRaise->f_nav_return_latest_week);
					nCol++;
					//最近一个月净值增长率double
					resTable.SetCell(nCol,j,pFundNavRaise->f_nav_return_lastet_month);
					nCol++;
					//最近三个月净值增长率double
					resTable.SetCell(nCol,j,pFundNavRaise->f_nav_return_latest_3M);
					nCol++;
					//最近六个月净值增长率double
					resTable.SetCell(nCol,j,pFundNavRaise->f_nav_return_latest_6M);
					nCol++;
					//最近一年净值增长率double
					resTable.SetCell(nCol,j,pFundNavRaise->f_nav_return_latest_1Y);
					nCol++;
					//最近二年净值增长率double
					resTable.SetCell(nCol,j,pFundNavRaise->f_nav_return_latest_2Y);
					nCol++;
					//今年以来净值增长率double
					resTable.SetCell(nCol,j,pFundNavRaise->f_nav_return_latest_this_year);
					nCol++;
					//设立以来净值增长率double
					resTable.SetCell(nCol,j,pFundNavRaise->f_nav_return_from_setup);
					nCol++;
				}
				else
				{
#ifdef _DEBUG
					GlobalWatch::_GetInstance()->WatchHere(_T("bond security id = %d non-exist"),*iter);
#endif
				}
			}

			prw.SetPercent(progId, 1.0);
			sProgressPrompt+=_T(",完成!");
			prw.SetText(progId, sProgressPrompt);

			delete pFundNav2Day;
			pFundNav2Day = NULL;


			return true;
		}
		//add by lijw 2008-09-08
		//取得基金净值2days
		bool TxFund::GetCloseFundNav2Days(
			std::vector<int>& iSecurityId,		//交易实体ID
			Tx::Core::Table_Display& resTable,	//结果数据表
			GridAdapterTable_Display& resAdapter,//显示格式
			int& iDate1,						//交易日1
			int& iDate2							//交易日1
			)
		{
			int ii = iSecurityId.size();
			if(ii<=0)
				return false;

			DataFileNormal<blk_TxExFile_FileHead,FundNav2Day>* pFundNav2Day = new DataFileNormal<blk_TxExFile_FileHead,FundNav2Day>;
			if(pFundNav2Day==NULL)
				return false;
			//取得扩展数据目录id
			int iFundNav2DaysId = 
				Tx::Data::DataStatus::GetInstance()->GetExFileIdFromIni(
				Tx::Core::SystemPath::GetInstance()->GetSystemDataPath(),
				_T("fund_nav_2days"));

			int iFundNav2DaysFileId_last = -1;

			//step1
			Tx::Core::ProgressWnd prw;
			//step2
			CString sProgressPrompt;
			sProgressPrompt.Format(_T("基金净值..."));
			UINT progId = prw.AddItem(1,sProgressPrompt, 0.0);
			//step3
			prw.Show(15);
			//step4
			prw.SetPercent(progId, 0.1);

			//默认的返回值状态
			bool result = false;
			//清空数据
			resTable.Clear();
			int iCol = 0;

			//准备样本集=Security_ID,int型
			resTable.AddCol(Tx::Core::dtype_int4);
			iCol++;
			//样本名称
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.SetTitle(iCol,_T("名称"));
			iCol++;
			//样本外码
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.SetTitle(iCol,_T("代码"));
			iCol++;
			//单位净值1
			resTable.AddCol(Tx::Core::dtype_float);
			resTable.SetTitle(iCol,_T("单位净值"));
			//resTable.SetFormatStyle(iCol, fs_date);
			iCol++;

			//累计净值1
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("累计净值"));
			iCol++;

			//单位净值2
			resTable.AddCol(Tx::Core::dtype_float);
			resTable.SetTitle(iCol,_T("单位净值"));
			//resTable.SetFormatStyle(iCol, fs_date);
			iCol++;

			//累计净值2
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("累计净值"));
			iCol++;

			//周涨跌值
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("周涨跌值"));
			iCol++;

			//周涨跌幅
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("周涨跌幅%"));
			iCol++;

			//市值
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("市值"));
			iCol++;

			//折溢价率
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("折溢价率%"));
			iCol++;

			//到期日
			resTable.AddCol(Tx::Core::dtype_int4);
			resTable.SetTitle(iCol,_T("到期日"));
			iCol++;

			//基金经理
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.SetTitle(iCol,_T("基金经理"));
			iCol++;
			//添加券ID
			int j=0;
			bool bIsFundCurrency = false;
			for(std::vector<int>::iterator iter = iSecurityId.begin();iter!=iSecurityId.end();iter++)
			{
				SecurityQuotation* pSecurity = GetSecurityNow(*iter);
				if(pSecurity!=NULL)
				{
					CString sFileIdKey;
					if(pSecurity->IsFund_Close()==true)
					{
						sFileIdKey = _T("fund_close_nav_2days");
					}
					else if(pSecurity->IsFund_Open()==true)
					{
						sFileIdKey = _T("fund_open_nav_2days");
					}
					else if(pSecurity->IsFund_Currency()==true)
					{
						sFileIdKey = _T("fund_currency_nav_2days");
						bIsFundCurrency = true;
					}
					else
						continue;
					j = resTable.GetRowCount();
					//根据样本数量添加记录数
					resTable.AddRow();
					int nCol = 0;
					//交易实体id
					resTable.SetCell(nCol,j,*iter);
					nCol++;
					//填充名称
					resTable.SetCell(nCol,j,pSecurity->GetName());
					nCol++;
					//填充外码
					resTable.SetCell(nCol,j,pSecurity->GetCode());
					nCol++;

					int iFundNav2DaysFileId = 
						Tx::Data::DataStatus::GetInstance()->GetExFileIdFromIni(
						Tx::Core::SystemPath::GetInstance()->GetSystemDataPath(),
						sFileIdKey);
					if(iFundNav2DaysFileId_last != iFundNav2DaysFileId)
					{
						pFundNav2Day->Reset();
					}
					if(pFundNav2Day->Load(
						iFundNav2DaysFileId,//文件名=2020202.dat
						iFundNav2DaysId,//文件所在目录
						true)==false || pFundNav2Day->GetDataCount()<=1)
					{
						iFundNav2DaysFileId_last = iFundNav2DaysFileId;
						continue;
					}

					//处理最新加载的日期
					if(iFundNav2DaysFileId_last != iFundNav2DaysFileId)
					{
						//最后一条是数据日期
						struct FundNav2DaysDate
						{
							int id;
							int date1;
							int date2;
						};
						FundNav2Day* pFundNav2DayDate = pFundNav2Day->GetDataByIndex(pFundNav2Day->GetDataCount()-1);
						if(pFundNav2DayDate!=NULL)
						{
							FundNav2DaysDate* pFundNav2DaysDate1 = (FundNav2DaysDate*)pFundNav2DayDate;
#ifdef _DEBUG
							int iiiii=0;
							TRACE(_T("\nid=%d,date1=%d,date2=%d"),pFundNav2DaysDate1->id,pFundNav2DaysDate1->date1,pFundNav2DaysDate1->date2);
#endif
							//将日期写入adapter的双层表头
							//resAdapter
							iDate1 = pFundNav2DaysDate1->date1;
							iDate2 = pFundNav2DaysDate1->date2;
						}
					}

					//记录上次文件id
					iFundNav2DaysFileId_last = iFundNav2DaysFileId;

					FundNav2Day* pFundNav2DayData = pFundNav2Day->GetDataByObj(*iter,false);
					if(pFundNav2DayData==NULL)
						continue;

					//单位净值
					resTable.SetCell(nCol,j,pFundNav2DayData->fNav1);
					nCol++;
					//累计净值
					resTable.SetCell(nCol,j,pFundNav2DayData->fSumNav1);
					nCol++;

					//单位净值
					resTable.SetCell(nCol,j,pFundNav2DayData->fNav2);
					nCol++;
					//累计净值
					resTable.SetCell(nCol,j,pFundNav2DayData->fSumNav2);
					nCol++;

					double dValue = Con_doubleInvalid;
					if(pFundNav2DayData->fNav1>0 && pFundNav2DayData->fNav2>0)
					{
						dValue = pFundNav2DayData->fNav1 - pFundNav2DayData->fNav2;
					}
					//周涨跌幅
					resTable.SetCell(nCol,j,dValue);
					nCol++;
					if(fabs(dValue-Con_doubleInvalid)>0.000001)
					{
						if(pFundNav2DayData->fNav2 > 0)
						{
							dValue /= pFundNav2DayData->fNav2;
							dValue *= 100;
						}
						else
							dValue = Con_doubleInvalid;
					}
					// 周涨跌幅以-100%到100%为界限
					if(fabs(dValue)>100)
						dValue = Con_doubleInvalid;
					//周涨跌幅
					resTable.SetCell(nCol,j,dValue);
					nCol++;

					//市值
					double dprice = pSecurity->GetClosePrice();
					if(dprice<0)
						dprice = Tx::Core::Con_doubleInvalid;
					resTable.SetCell(nCol,j,dprice);
					nCol++;

					//折溢价率
					double dZYJ;
					if(dprice < 0)
						dZYJ = Tx::Core::Con_doubleInvalid;
					else
						dZYJ = (pFundNav2DayData->fNav1 - dprice)/pFundNav2DayData->fNav1*100;
					resTable.SetCell(nCol,j,dZYJ);
					nCol++;

					//到期日
					FundCloseIssueInfo * pFundCloseIssueInfo = pSecurity->GetFundCloseIssueInfo();
					if(pFundCloseIssueInfo != NULL)
					{
						int date;
						date = pFundCloseIssueInfo->end_date;
						resTable.SetCell(nCol,j,date);
						nCol++;
					}

					//基金经理
					FundNewInfo* pFundNewInfo = pSecurity->GetFundNewInfo();
					if(pFundNewInfo!=NULL)
					{
						CString sManager;
						sManager = pFundNewInfo->fund_manager;
						resTable.SetCell(nCol,j,sManager);
						nCol++;
					}
				}
				else
				{
#ifdef _DEBUG
					GlobalWatch::_GetInstance()->WatchHere(_T("fund security id = %d non-exist"),*iter);
#endif
				}
			}

			if(bIsFundCurrency==true)
			{
				iCol=3;
				//单位净值1
				resTable.AddCol(Tx::Core::dtype_float);
				resTable.SetTitle(iCol,_T("单位净值"));
				//resTable.SetFormatStyle(iCol, fs_date);
				iCol++;

				//累计净值1
				resTable.AddCol(Tx::Core::dtype_double);
				resTable.SetTitle(iCol,_T("累计净值"));
				iCol++;

				//单位净值2
				resTable.AddCol(Tx::Core::dtype_float);
				resTable.SetTitle(iCol,_T("单位净值"));
				//resTable.SetFormatStyle(iCol, fs_date);
				iCol++;

				//累计净值2
				resTable.AddCol(Tx::Core::dtype_double);
				resTable.SetTitle(iCol,_T("累计净值"));
				iCol++;
			}
			////add by lijw 2008-09-08
			//int icount = pTable->GetRowCount();
			//icount -= 2;
			//pTable->DeleteCol(icount,2);
			//for (int i = 0;i < 4;i++)
			//{
			//	pTable->AddCol(Tx::Core::dtype_double);
			//}
			//pTable->AddCol(Tx::Core::dtype_int4);
			//pTable->AddCol(Tx::Core::dtype_val_string);
			prw.SetPercent(progId, 1.0);
			sProgressPrompt+=_T(",完成!");
			prw.SetText(progId, sProgressPrompt);

			delete pFundNav2Day;
			pFundNav2Day = NULL;
			resTable.SetFormatStyle(11,fs_date);
			j = 3;
			for(;j < 8;j++)
			{
				resTable.SetPrecise(j,4);
			}
			resTable.SetPrecise(j,2);
			j++;
			resTable.SetPrecise(j,4);
			j++;
			resTable.SetPrecise(j,2);
			return true;
		}
		bool TxFund::GetFundNav2Days(
			std::vector<int>& iSecurityId,		//交易实体ID
			Tx::Core::Table_Display& resTable,	//结果数据表
			GridAdapterTable_Display& resAdapter//显示格式
			)
		{
			int ii = iSecurityId.size();
			if(ii<=0)
				return false;

			DataFileNormal<blk_TxExFile_FileHead,FundNav2Day>* pFundNav2Day = new DataFileNormal<blk_TxExFile_FileHead,FundNav2Day>;
			if(pFundNav2Day==NULL)
				return false;
			//取得扩展数据目录id
			int iFundNav2DaysId = 
				Tx::Data::DataStatus::GetInstance()->GetExFileIdFromIni(
				Tx::Core::SystemPath::GetInstance()->GetSystemDataPath(),
				_T("fund_nav_2days"));

			int iFundNav2DaysFileId_last = -1;

			//step1
			Tx::Core::ProgressWnd prw;
			//step2
			CString sProgressPrompt;
			sProgressPrompt.Format(_T("基金净值..."));
			UINT progId = prw.AddItem(1,sProgressPrompt, 0.0);
			//step3
			prw.Show(15);
			//step4
			prw.SetPercent(progId, 0.1);

			//默认的返回值状态
			bool result = false;
			//清空数据
			resTable.Clear();
			int iCol = 0;

			//准备样本集=Security_ID,int型
			resTable.AddCol(Tx::Core::dtype_int4);
			iCol++;
			//样本名称
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.SetTitle(iCol,_T("名称"));
			iCol++;
			//样本外码
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.SetTitle(iCol,_T("代码"));
			iCol++;
			//单位净值1
			resTable.AddCol(Tx::Core::dtype_float);
			resTable.SetTitle(iCol,_T("单位净值"));
			//resTable.SetFormatStyle(iCol, fs_date);
			iCol++;

			//累计净值1
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("累计净值"));
			iCol++;

			//单位净值2
			resTable.AddCol(Tx::Core::dtype_float);
			resTable.SetTitle(iCol,_T("单位净值"));
			//resTable.SetFormatStyle(iCol, fs_date);
			iCol++;

			//累计净值2
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("累计净值"));
			iCol++;

			//日涨跌值
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("日涨跌值"));
			iCol++;

			//日涨跌幅
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("日涨跌幅"));
			iCol++;

			//添加券ID
			int j=0;
			bool bIsFundCurrency = false;
			for(std::vector<int>::iterator iter = iSecurityId.begin();iter!=iSecurityId.end();iter++)
			{
				SecurityQuotation* pSecurity = GetSecurityNow(*iter);
				if(pSecurity!=NULL)
				{
					CString sFileIdKey;
					if(pSecurity->IsFund_Close()==true)
					{
						sFileIdKey = _T("fund_close_nav_2days");
					}
					else if(pSecurity->IsFund_Open()==true)
					{
						sFileIdKey = _T("fund_open_nav_2days");
					}
					else if(pSecurity->IsFund_Currency()==true)
					{
						sFileIdKey = _T("fund_currency_nav_2days");
						bIsFundCurrency = true;
					}
					else
						continue;
					j = resTable.GetRowCount();
					//根据样本数量添加记录数
					resTable.AddRow();
					int nCol = 0;
					//交易实体id
					resTable.SetCell(nCol,j,*iter);
					nCol++;
					//填充名称
					resTable.SetCell(nCol,j,pSecurity->GetName());
					nCol++;
					//填充外码
					resTable.SetCell(nCol,j,pSecurity->GetCode());
					nCol++;

					int iFundNav2DaysFileId = 
						Tx::Data::DataStatus::GetInstance()->GetExFileIdFromIni(
						Tx::Core::SystemPath::GetInstance()->GetSystemDataPath(),
						sFileIdKey);
					if(iFundNav2DaysFileId_last != iFundNav2DaysFileId)
					{
						pFundNav2Day->Reset();
					}
					if(pFundNav2Day->Load(
						iFundNav2DaysFileId,//文件名=2020202.dat
						iFundNav2DaysId,//文件所在目录
						true)==false || pFundNav2Day->GetDataCount()<=1)
					{
						iFundNav2DaysFileId_last = iFundNav2DaysFileId;
						continue;
					}

					//处理最新加载的日期
					if(iFundNav2DaysFileId_last != iFundNav2DaysFileId)
					{
						//最后一条是数据日期
						struct FundNav2DaysDate
						{
							int id;
							int date1;
							int date2;
						};
						FundNav2Day* pFundNav2DayDate = pFundNav2Day->GetDataByIndex(pFundNav2Day->GetDataCount()-1);
						if(pFundNav2DayDate!=NULL)
						{
							FundNav2DaysDate* pFundNav2DaysDate1 = (FundNav2DaysDate*)pFundNav2DayDate;
#ifdef _DEBUG
							int iiiii=0;
							TRACE(_T("\nid=%d,date1=%d,date2=%d"),pFundNav2DaysDate1->id,pFundNav2DaysDate1->date1,pFundNav2DaysDate1->date2);
#endif
							//将日期写入adapter的双层表头
							//resAdapter
						}
					}

					//记录上次文件id
					iFundNav2DaysFileId_last = iFundNav2DaysFileId;

					FundNav2Day* pFundNav2DayData = pFundNav2Day->GetDataByObj(*iter,false);
					if(pFundNav2DayData==NULL)
						continue;

					//单位净值
					resTable.SetCell(nCol,j,pFundNav2DayData->fNav1);
					nCol++;
					//累计净值
					resTable.SetCell(nCol,j,pFundNav2DayData->fSumNav1);
					nCol++;

					//单位净值
					resTable.SetCell(nCol,j,pFundNav2DayData->fNav2);
					nCol++;
					//累计净值
					resTable.SetCell(nCol,j,pFundNav2DayData->fSumNav2);
					nCol++;

					double dValue = Con_doubleInvalid;
					if(pFundNav2DayData->fNav1>0 && pFundNav2DayData->fNav2>0)
					{
						dValue = pFundNav2DayData->fNav1 - pFundNav2DayData->fNav2;
					}
					//日涨跌值
					resTable.SetCell(nCol,j,dValue);
					nCol++;
					if(fabs(dValue-Con_doubleInvalid)>0.000001)
					{
						dValue /= pFundNav2DayData->fNav2;
						dValue *= 100;
					}
					//日涨跌幅
					resTable.SetCell(nCol,j,dValue);
					nCol++;
				}
				else
				{
#ifdef _DEBUG
					GlobalWatch::_GetInstance()->WatchHere(_T("fund security id = %d non-exist"),*iter);
#endif
				}
			}

			if(bIsFundCurrency==true)
			{
				iCol=3;
				//单位净值1
				resTable.AddCol(Tx::Core::dtype_float);
				resTable.SetTitle(iCol,_T("单位净值"));
				//resTable.SetFormatStyle(iCol, fs_date);
				iCol++;

				//累计净值1
				resTable.AddCol(Tx::Core::dtype_double);
				resTable.SetTitle(iCol,_T("累计净值"));
				iCol++;

				//单位净值2
				resTable.AddCol(Tx::Core::dtype_float);
				resTable.SetTitle(iCol,_T("单位净值"));
				//resTable.SetFormatStyle(iCol, fs_date);
				iCol++;

				//累计净值2
				resTable.AddCol(Tx::Core::dtype_double);
				resTable.SetTitle(iCol,_T("累计净值"));
				iCol++;
			}
			prw.SetPercent(progId, 1.0);
			sProgressPrompt+=_T(",完成!");
			prw.SetText(progId, sProgressPrompt);

			delete pFundNav2Day;
			pFundNav2Day = NULL;

			return true;
		}
		//2008-08-07
		//取得货币型基金收益
		bool TxFund::GetFundCurrency2Days(
			std::vector<int>& iSecurityId,		//交易实体ID
			Tx::Core::Table_Display& resTable,	//结果数据表
			GridAdapterTable_Display& resAdapter,//显示格式
			int& iDate1,						//交易日1
			int& iDate2							//交易日2
			)
		{
			int ii = iSecurityId.size();
			if(ii<=0)
				return false;

			DataFileNormal<blk_TxExFile_FileHead,FundNav2Day>* pFundNav2Day = new DataFileNormal<blk_TxExFile_FileHead,FundNav2Day>;
			if(pFundNav2Day==NULL)
				return false;
			//取得扩展数据目录id
			int iFundNav2DaysId = 
				Tx::Data::DataStatus::GetInstance()->GetExFileIdFromIni(
				Tx::Core::SystemPath::GetInstance()->GetSystemDataPath(),
				_T("fund_nav_2days"));

			int iFundNav2DaysFileId_last = -1;

			//step1
			Tx::Core::ProgressWnd prw;
			//step2
			CString sProgressPrompt;
			sProgressPrompt.Format(_T("货币型基金收益..."));
			UINT progId = prw.AddItem(1,sProgressPrompt, 0.0);
			//step3
			prw.Show(15);
			//step4
			prw.SetPercent(progId, 0.1);

			//默认的返回值状态
			bool result = false;
			//清空数据
			resTable.Clear();
			int iCol = 0;

			//准备样本集=Security_ID,int型
			resTable.AddCol(Tx::Core::dtype_int4);
			iCol++;
			//样本名称
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.SetTitle(iCol,_T("名称"));
			iCol++;
			//样本外码
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.SetTitle(iCol,_T("代码"));
			iCol++;
			//万份单位收益1
			resTable.AddCol(Tx::Core::dtype_float);
			resTable.SetTitle(iCol,_T("万元收益"));
			resTable.SetPrecise(iCol,5);
			iCol++;

			//7日年平均收益率%1
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("7日年平均收益率%"));
			resTable.SetPrecise(iCol,4);
			iCol++;

			//万份单位收益2
			resTable.AddCol(Tx::Core::dtype_float);
			resTable.SetTitle(iCol,_T("万元收益"));
			resTable.SetPrecise(iCol,5);
			//resTable.SetFormatStyle(iCol, fs_date);
			iCol++;

			//7日年平均收益率%2
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("7日年平均收益率%"));
			resTable.SetPrecise(iCol,4);
			iCol++;

			//成立日期
			resTable.AddCol(Tx::Core::dtype_int4);
			resTable.SetTitle(iCol,_T("成立日期"));
			resTable.SetFormatStyle(iCol, fs_date);
			iCol++;

			//基金经理
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.SetTitle(iCol,_T("基金经理"));
			iCol++;

			//添加券ID
			int j=0;
			for(std::vector<int>::iterator iter = iSecurityId.begin();iter!=iSecurityId.end();iter++)
			{
				SecurityQuotation* pSecurity = GetSecurityNow(*iter);
				if(pSecurity!=NULL)
				{
					CString sFileIdKey;
					if(pSecurity->IsFund_Currency()==false)
						continue;
					sFileIdKey = _T("fund_currency_nav_2days");
					j = resTable.GetRowCount();
					//根据样本数量添加记录数
					resTable.AddRow();
					int nCol = 0;
					//交易实体id
					resTable.SetCell(nCol,j,*iter);
					nCol++;
					//填充名称
					resTable.SetCell(nCol,j,pSecurity->GetName());
					nCol++;
					//填充外码
					resTable.SetCell(nCol,j,pSecurity->GetCode());
					nCol++;

					int iFundNav2DaysFileId = 
						Tx::Data::DataStatus::GetInstance()->GetExFileIdFromIni(
						Tx::Core::SystemPath::GetInstance()->GetSystemDataPath(),
						sFileIdKey);
					if(iFundNav2DaysFileId_last != iFundNav2DaysFileId)
					{
						pFundNav2Day->Reset();
					}
					if(pFundNav2Day->Load(
						iFundNav2DaysFileId,//文件名=2020202.dat
						iFundNav2DaysId,//文件所在目录
						true)==false || pFundNav2Day->GetDataCount()<=1)
					{
						iFundNav2DaysFileId_last = iFundNav2DaysFileId;
						continue;
					}

					//处理最新加载的日期
					if(iFundNav2DaysFileId_last != iFundNav2DaysFileId)
					{
						//最后一条是数据日期
						struct FundNav2DaysDate
						{
							int id;
							int date1;
							int date2;
						};
						FundNav2Day* pFundNav2DayDate = pFundNav2Day->GetDataByIndex(pFundNav2Day->GetDataCount()-1);
						if(pFundNav2DayDate!=NULL)
						{
							FundNav2DaysDate* pFundNav2DaysDate1 = (FundNav2DaysDate*)pFundNav2DayDate;
#ifdef _DEBUG
							int iiiii=0;
							TRACE(_T("\nid=%d,date1=%d,date2=%d"),pFundNav2DaysDate1->id,pFundNav2DaysDate1->date1,pFundNav2DaysDate1->date2);
#endif
							//将日期写入adapter的双层表头
							//resAdapter
							iDate1 = pFundNav2DaysDate1->date1;
							iDate2 = pFundNav2DaysDate1->date2;
						}
					}

					//记录上次文件id
					iFundNav2DaysFileId_last = iFundNav2DaysFileId;

					FundNav2Day* pFundNav2DayData = pFundNav2Day->GetDataByObj(*iter,false);
					if(pFundNav2DayData==NULL)
						continue;

					//万份单位收益1
					resTable.SetCell(nCol,j,pFundNav2DayData->fNav1);
					nCol++;
					//7日年平均收益率%1
					if (-10.0 < pFundNav2DayData->fSumNav1 && pFundNav2DayData->fSumNav1 < 10.0)
						resTable.SetCell(nCol,j,pFundNav2DayData->fSumNav1);
					else
						resTable.SetCell(nCol,j,Tx::Core::Con_doubleInvalid);
					nCol++;

					//万份单位收益2
					resTable.SetCell(nCol,j,pFundNav2DayData->fNav2);
					nCol++;
					//7日年平均收益率%2
					if (-10.0 < pFundNav2DayData->fSumNav2 && pFundNav2DayData->fSumNav2 < 10.0)
						resTable.SetCell(nCol,j,pFundNav2DayData->fSumNav2);
					else
						resTable.SetCell(nCol,j,Tx::Core::Con_doubleInvalid);
					nCol++;

					FundNewInfo* pFundNewInfo = pSecurity->GetFundNewInfo();
					if(pFundNewInfo!=NULL)
					{
						//成立日期
						resTable.SetCell(nCol,j,pFundNewInfo->setup_date);
						nCol++;
						//基金经理
						CString sManager;
						sManager = pFundNewInfo->fund_manager;
						resTable.SetCell(nCol,j,sManager);
						nCol++;
					}
				}
				else
				{
#ifdef _DEBUG
					GlobalWatch::_GetInstance()->WatchHere(_T("fund security id = %d non-exist"),*iter);
#endif
				}
			}

			prw.SetPercent(progId, 1.0);
			sProgressPrompt+=_T(",完成!");
			prw.SetText(progId, sProgressPrompt);

			delete pFundNav2Day;
			pFundNav2Day = NULL;

			return true;
		}
		//2008-08-27
		//折溢价率
		double	TxFund::GetOverflowValueRate(
			int     iSecurityId,
			int		iDate
			)
		{
			//取得交易实体
			SecurityQuotation* pSecurity = GetSecurityNow(iSecurityId);
			if(pSecurity==NULL)
				return Con_doubleInvalid;

			//只有封闭式基金和etf,lof,等才计算
			if(!(pSecurity->IsFund_Close()==true || pSecurity->IsFund_Open_Normal()!=true))
				return Con_doubleInvalid;

			//取得指定日期的收盘价格
			float fClosePrice = pSecurity->GetClosePrice(iDate,true);
			if(!(fClosePrice>0))
				return Con_doubleInvalid;

			//取得指定日期的单位净值
			double dNav = pSecurity->GetFundNAV(iDate);
			if(!(dNav>0))
				return Con_doubleInvalid;

			return (fClosePrice-dNav)/dNav;
		}

		//2008-09-08  add by wangych
		//取得开发式基金净值2days
		bool TxFund::GetOpenFundNav2Days(
			std::vector<int>& iSecurityId,		//交易实体ID
			Tx::Core::Table_Display& resTable,	//结果数据表
			GridAdapterTable_Display& resAdapter,//显示格式
			int& iDate1,						//交易日1
			int& iDate2						//交易日2
			)
		{
			int ii = iSecurityId.size();
			if(ii<=0)
				return false;

			DataFileNormal<blk_TxExFile_FileHead,FundNav2Day>* pFundNav2Day = new DataFileNormal<blk_TxExFile_FileHead,FundNav2Day>;
			if(pFundNav2Day==NULL)
				return false;
			//取得扩展数据目录id
			int iFundNav2DaysId = 
				Tx::Data::DataStatus::GetInstance()->GetExFileIdFromIni(
				Tx::Core::SystemPath::GetInstance()->GetSystemDataPath(),
				_T("fund_nav_2days"));

			int iFundNav2DaysFileId_last = -1;

			//step1
			Tx::Core::ProgressWnd prw;
			//step2
			CString sProgressPrompt;
			sProgressPrompt.Format(_T("基金净值..."));
			UINT progId = prw.AddItem(1,sProgressPrompt, 0.0);
			//step3
			prw.Show(15);
			//step4
			prw.SetPercent(progId, 0.1);

			//默认的返回值状态
			bool result = false;
			//清空数据
			resTable.Clear();
			int iCol = 0;

			//准备样本集=Security_ID,int型
			resTable.AddCol(Tx::Core::dtype_int4);
			iCol++;
			//样本名称
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.SetTitle(iCol,_T("名称"));
			iCol++;
			//样本外码
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.SetTitle(iCol,_T("代码"));
			iCol++;
			//单位净值1
			resTable.AddCol(Tx::Core::dtype_float);
			resTable.SetTitle(iCol,_T("单位净值"));
			resTable.SetPrecise(iCol,4);
			//resTable.SetFormatStyle(iCol, fs_date);
			iCol++;

			//累计净值1
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("累计净值"));
			resTable.SetPrecise(iCol,4);
			iCol++;

			//单位净值2
			resTable.AddCol(Tx::Core::dtype_float);
			resTable.SetTitle(iCol,_T("单位净值"));
			resTable.SetPrecise(iCol,4);
			//resTable.SetFormatStyle(iCol, fs_date);
			iCol++;

			//累计净值2
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("累计净值"));
			resTable.SetPrecise(iCol,4);
			iCol++;

			//日涨跌值
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("日涨跌值"));
			resTable.SetPrecise(iCol,4);
			iCol++;

			//日涨跌幅
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("日涨跌幅%"));
			resTable.SetPrecise(iCol,2);
			iCol++;

			//申购状态
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.SetTitle(iCol,_T("申购状态"));
			iCol++;

			//基金经理
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.SetTitle(iCol,_T("基金经理"));
			iCol++;
			//备注
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.SetTitle(iCol,_T("备注"));
			iCol++;

			//添加券ID
			int j=0;
			bool bIsFundCurrency = false;
			for(std::vector<int>::iterator iter = iSecurityId.begin();iter!=iSecurityId.end();iter++)
			{
				SecurityQuotation* pSecurity = GetSecurityNow(*iter);
				if(pSecurity!=NULL)
				{
					CString sFileIdKey;
					if(pSecurity->IsFund_Close()==true)
					{
						sFileIdKey = _T("fund_close_nav_2days");
					}
					else if(pSecurity->IsFund_Open()==true)
					{
						sFileIdKey = _T("fund_open_nav_2days");	
					}
					else if(pSecurity->IsFund_Currency()==true)
					{
						sFileIdKey = _T("fund_currency_nav_2days");
						bIsFundCurrency = true;
					}
					else
						continue;
					j = resTable.GetRowCount();
					//根据样本数量添加记录数
					resTable.AddRow();
					int nCol = 0;
					//交易实体id
					resTable.SetCell(nCol,j,*iter);
					nCol++;
					//填充名称
					resTable.SetCell(nCol,j,pSecurity->GetName());
					nCol++;
					//填充外码
					resTable.SetCell(nCol,j,pSecurity->GetCode());
					nCol++;

					int iFundNav2DaysFileId = 
						Tx::Data::DataStatus::GetInstance()->GetExFileIdFromIni(
						Tx::Core::SystemPath::GetInstance()->GetSystemDataPath(),
						sFileIdKey);
					if(iFundNav2DaysFileId_last != iFundNav2DaysFileId)
					{
						pFundNav2Day->Reset();
					}
					if(pFundNav2Day->Load(
						iFundNav2DaysFileId,//文件名=2020202.dat
						iFundNav2DaysId,//文件所在目录
						true)==false || pFundNav2Day->GetDataCount()<=1)
					{
						iFundNav2DaysFileId_last = iFundNav2DaysFileId;
						continue;
					}

					//处理最新加载的日期
					if(iFundNav2DaysFileId_last != iFundNav2DaysFileId)
					{
						//最后一条是数据日期
						struct FundNav2DaysDate
						{
							int id;
							int date1;
							int date2;
						};
						FundNav2Day* pFundNav2DayDate = pFundNav2Day->GetDataByIndex(pFundNav2Day->GetDataCount()-1);
						if(pFundNav2DayDate!=NULL)
						{
							FundNav2DaysDate* pFundNav2DaysDate1 = (FundNav2DaysDate*)pFundNav2DayDate;
#ifdef _DEBUG
							int iiiii=0;
							TRACE(_T("\nid=%d,date1=%d,date2=%d"),pFundNav2DaysDate1->id,pFundNav2DaysDate1->date1,pFundNav2DaysDate1->date2);
#endif
							//将日期写入adapter的双层表头
							//resAdapter
							iDate1 = pFundNav2DaysDate1->date1;
							iDate2 = pFundNav2DaysDate1->date2;
						}
					}

					//记录上次文件id
					iFundNav2DaysFileId_last = iFundNav2DaysFileId;

					FundNav2Day* pFundNav2DayData = pFundNav2Day->GetDataByObj(*iter,false);
					if (pFundNav2DayData != NULL)
					{
						//单位净值
						resTable.SetCell(nCol,j,pFundNav2DayData->fNav1);
						nCol++;
						//累计净值
						resTable.SetCell(nCol,j,pFundNav2DayData->fSumNav1);
						nCol++;

						//单位净值
						resTable.SetCell(nCol,j,pFundNav2DayData->fNav2);
						nCol++;
						//累计净值
						resTable.SetCell(nCol,j,pFundNav2DayData->fSumNav2);
						nCol++;

						double dValue = Con_doubleInvalid;
						if(pFundNav2DayData->fNav1>0 && pFundNav2DayData->fNav2>0)
						{
							dValue = pFundNav2DayData->fNav1 - pFundNav2DayData->fNav2;
						}
						//日涨跌值
						resTable.SetCell(nCol,j,dValue);
						nCol++;
						if(fabs(dValue-Con_doubleInvalid)>0.000001)
						{
							if (pFundNav2DayData->fNav2 != 0)
							{
								dValue /= pFundNav2DayData->fNav2;
								dValue *= 100;
							}

						}
						//日涨跌幅
						if (fabs(dValue) <= 10)
						{
							resTable.SetCell(nCol,j,dValue);
						} 
						else
						{
							resTable.SetCell(nCol,j,Tx::Core::Con_doubleInvalid);
						}			
						nCol++;
					} 
					else
					{
						//单位净值
						resTable.SetCell(nCol,j,Tx::Core::Con_floatInvalid);
						nCol++;
						//累计净值
						resTable.SetCell(nCol,j,Tx::Core::Con_doubleInvalid);
						nCol++;

						//单位净值
						resTable.SetCell(nCol,j,Tx::Core::Con_floatInvalid);
						nCol++;
						//累计净值
						resTable.SetCell(nCol,j,Tx::Core::Con_doubleInvalid);
						nCol++;

						double dValue = Con_doubleInvalid;
						//日涨跌值
						resTable.SetCell(nCol,j,Tx::Core::Con_doubleInvalid);
						nCol++;

						//日涨跌幅
						resTable.SetCell(nCol,j,Tx::Core::Con_doubleInvalid);			
						nCol++;

					}

					//申购状态
					resTable.SetCell(nCol,j,Tx::Core::Con_strInvalid);
					nCol++;

					//基金经理
					FundNewInfo* pFundNewInfo = pSecurity->GetFundNewInfo();
					if(pFundNewInfo!=NULL)
					{
						//基金经理
						CString sManager;
						sManager = pFundNewInfo->fund_manager;
						//if (!sManager.IsEmpty())
						//{
						//	resTable.SetCell(nCol,j,sManager);
						//} 
						//else
						//{
						//	resTable.SetCell(nCol,j,Tx::Core::Con_strInvalid);
						//}
						resTable.SetCell(nCol,j,sManager);
						nCol++;
					}

					//备注
					resTable.SetCell(nCol,j,Tx::Core::Con_strInvalid);
					nCol++;
				}
				else
				{
#ifdef _DEBUG
					GlobalWatch::_GetInstance()->WatchHere(_T("fund security id = %d non-exist"),*iter);
#endif
				}
			}

			if(bIsFundCurrency==true)
			{
				iCol=3;
				//单位净值1
				resTable.AddCol(Tx::Core::dtype_float);
				resTable.SetTitle(iCol,_T("单位净值"));
				//resTable.SetFormatStyle(iCol, fs_date);
				iCol++;

				//累计净值1
				resTable.AddCol(Tx::Core::dtype_double);
				resTable.SetTitle(iCol,_T("累计净值"));
				iCol++;

				//单位净值2
				resTable.AddCol(Tx::Core::dtype_float);
				resTable.SetTitle(iCol,_T("单位净值"));
				//resTable.SetFormatStyle(iCol, fs_date);
				iCol++;

				//累计净值2
				resTable.AddCol(Tx::Core::dtype_double);
				resTable.SetTitle(iCol,_T("累计净值"));
				iCol++;
			}
			prw.SetPercent(progId, 1.0);
			sProgressPrompt+=_T(",完成!");
			prw.SetText(progId, sProgressPrompt);

			delete pFundNav2Day;
			pFundNav2Day = NULL;

			return true;
		}

		//2008-09-08 add by wangych
		//取得开放式基金净值涨幅
		bool TxFund::GetOpenFundNavRaise(
			std::vector<int>& iSecurityId,		//交易实体ID
			// int date,						//数据日期,0表示最近交易日
			Tx::Core::Table_Display& resTable,//结果数据表
			int& iDate,
			bool bPrice
			)
		{
			//构建两市场交易日期序列
			if(GetHuShenTradeDate()==false)
			{
				AfxMessageBox(_T("大盘交易日序列数据错误！"));
				return false;
			}

			//4000208=证指数
			//取得上证指数的交易实体指针
			//Security* pBaseSecurity;
			//pBaseSecurity = m_pShIndex;
			int iMarketId = 0;
			if(iMarketId ==0)
				iMarketId = m_pFunctionDataManager->GetBourseId_ShangHai();

			int iToday = m_pFunctionDataManager->GetServerCurDateTime(iMarketId).GetDate().GetInt();

			//取得两市场的起始日期和最新日期
			int iMixAllCount = m_pMixTradeDay->GetTradeDayCount();
			int iBaseStartDate = m_pMixTradeDay->GetDateByIndex(0);
			int iBaseEndDate = m_pMixTradeDay->GetLatestDate();

			if(iDate<=0)
			{
				//取得最近上个交易日期
				if(iBaseEndDate == iToday)
				{
					//今天是交易日期
					//大盘已经完成收盘处理
					iDate = m_pMixTradeDay->GetDateByIndex(iMixAllCount-2);
				}
				else
				{
					//尚未完成收盘处理
					iDate = m_pMixTradeDay->GetDateByIndex(iMixAllCount-1);
				}
			}
			else
			{
				//指定了历史日期
				if(m_pMixTradeDay->GetIndexByDate(iDate)<0)
				{
					//取得最近上个交易日期
					if(iBaseEndDate == iToday)
					{
						//今天是交易日期
						//大盘已经完成收盘处理
						iDate = m_pMixTradeDay->GetDateByIndex(iMixAllCount-2);
					}
					else
					{
						//尚未完成收盘处理
						iDate = m_pMixTradeDay->GetDateByIndex(iMixAllCount-1);
					}
				}
			}

			int ii = iSecurityId.size();

			//用于去当日基金单位和累计净值 and by wangych 08.09.10  begin
			DataFileNormal<blk_TxExFile_FileHead,FundNav2Day>* pFundNav2Day = new DataFileNormal<blk_TxExFile_FileHead,FundNav2Day>;
			if(pFundNav2Day==NULL)
				return false;
			//取得扩展数据目录id
			int iFundNav2DaysId = 
				Tx::Data::DataStatus::GetInstance()->GetExFileIdFromIni(
				Tx::Core::SystemPath::GetInstance()->GetSystemDataPath(),
				_T("fund_nav_2days"));

			int iFundNav2DaysFileId_last = -1;
			//用于去当日基金单位和累计净值 and by wangych 08.09.10  end

			//step1
			//	Tx::Core::ProgressWnd* pProgressWnd = Tx::Core::ProgressWnd::GetInstance();
			Tx::Core::ProgressWnd prw;
			//step2
			CString sProgressPrompt;
			sProgressPrompt.Format(_T("基金净值涨幅..."));
			UINT progId = prw.AddItem(1,sProgressPrompt, 0.0);
			//step3
			prw.Show(15);
			//step4
			prw.SetPercent(progId, 0.1);

			//默认的返回值状态
			bool result = false;
			//清空数据
			resTable.Clear();
			int iCol = 0;

			//准备样本集=Security_ID,int型
			resTable.AddCol(Tx::Core::dtype_int4);
			iCol++;
			//样本名称
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.SetTitle(iCol,_T("名称"));
			iCol++;
			//样本外码
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.SetTitle(iCol,_T("代码"));
			iCol++;

			//单位净值
			resTable.AddCol(Tx::Core::dtype_float);
			resTable.SetTitle(iCol,_T("单位净值"));
			resTable.SetPrecise(iCol,4);
			iCol++;

			//累计净值
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("累计净值"));
			resTable.SetPrecise(iCol,4);
			iCol++;

			//最近日净值增长率double
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("最新净值增长率%"));
			resTable.SetPrecise(iCol,2);
			iCol++;
			//最近一周净值增长率double
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("最近一周净值增长率%"));
			resTable.SetPrecise(iCol,2);
			iCol++;
			//最近一个月净值增长率double
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("最近一个月净值增长率%"));
			resTable.SetPrecise(iCol,2);
			iCol++;
			//最近三个月净值增长率double
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("最近三个月净值增长率%"));
			resTable.SetPrecise(iCol,2);
			iCol++;
			//最近六个月净值增长率double
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("最近六个月净值增长率%"));
			resTable.SetPrecise(iCol,2);
			iCol++;
			//最近一年净值增长率double
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("最近一年净值增长率%"));
			resTable.SetPrecise(iCol,2);
			iCol++;
			//最近二年净值增长率double
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("最近二年净值增长率%"));
			resTable.SetPrecise(iCol,2);
			iCol++;
			//今年以来净值增长率double
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("今年以来净值增长率%"));
			resTable.SetPrecise(iCol,2);
			iCol++;
			//设立以来净值增长率double
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("设立以来净值增长率%"));
			resTable.SetPrecise(iCol,2);
			iCol++;


			//添加券ID
			int j=0;
			for(std::vector<int>::iterator iter = iSecurityId.begin();iter!=iSecurityId.end();iter++)
			{
				SecurityQuotation* pSecurity = GetSecurityNow(*iter);
				if(pSecurity!=NULL)
				{
					CString sFileIdKey;
					if(pSecurity->IsFund_Open()==false)
					{
						continue;
					}
					sFileIdKey = _T("fund_open_nav_2days");	

					j = resTable.GetRowCount();
					//根据样本数量添加记录数
					resTable.AddRow();
					int nCol = 0;
					//交易实体id
					resTable.SetCell(nCol,j,*iter);
					nCol++;
					//填充名称
					resTable.SetCell(nCol,j,pSecurity->GetName());
					nCol++;
					//填充外码
					resTable.SetCell(nCol,j,pSecurity->GetCode());
					nCol++;


					//用于去当日基金单位和累计净值 and by wangych 08.09.10  begin
					int iFundNav2DaysFileId = 
						Tx::Data::DataStatus::GetInstance()->GetExFileIdFromIni(
						Tx::Core::SystemPath::GetInstance()->GetSystemDataPath(),
						sFileIdKey);
					if(iFundNav2DaysFileId_last != iFundNav2DaysFileId)
					{
						pFundNav2Day->Reset();
					}
					if(pFundNav2Day->Load(
						iFundNav2DaysFileId,//文件名=2020202.dat
						iFundNav2DaysId,//文件所在目录
						true)==false || pFundNav2Day->GetDataCount()<=1)
					{
						iFundNav2DaysFileId_last = iFundNav2DaysFileId;
						continue;
					}
					//用于去当日基金单位和累计净值 and by wangych 08.09.10  end

					//记录上次文件id
					iFundNav2DaysFileId_last = iFundNav2DaysFileId;

					FundNav2Day* pFundNav2DayData = pFundNav2Day->GetDataByObj(*iter,false);

					if(pFundNav2DayData==NULL)
					{
						//单位净值
						resTable.SetCell(nCol,j,Con_floatInvalid);
						nCol++;
						//累计净值
						resTable.SetCell(nCol,j,Con_doubleInvalid);
						nCol++;
					}
					else
					{
						//单位净值
						resTable.SetCell(nCol,j,pFundNav2DayData->fNav1);
						nCol++;
						//累计净值
						resTable.SetCell(nCol,j,pFundNav2DayData->fSumNav1);
						nCol++;
					}

					FundNavRaise* pFundNavRaise = NULL;
					if(pSecurity->GetGlobalData(gdt_FundNavRaise,&pFundNavRaise)==false || pFundNavRaise==NULL)
					{
						double dValue = Con_doubleInvalid;
						//最近日净值增长率double
						resTable.SetCell(nCol,j,dValue);
						nCol++;
						//最近一周净值增长率double
						resTable.SetCell(nCol,j,dValue);
						nCol++;
						//最近一个月净值增长率double
						resTable.SetCell(nCol,j,dValue);
						nCol++;
						//最近三个月净值增长率double
						resTable.SetCell(nCol,j,dValue);
						nCol++;
						//最近六个月净值增长率double
						resTable.SetCell(nCol,j,dValue);
						nCol++;
						//最近一年净值增长率double
						resTable.SetCell(nCol,j,dValue);
						nCol++;
						//最近二年净值增长率double
						resTable.SetCell(nCol,j,dValue);
						nCol++;
						//今年以来净值增长率double
						resTable.SetCell(nCol,j,dValue);
						nCol++;
						//设立以来净值增长率double
						resTable.SetCell(nCol,j,dValue);
						nCol++;
						continue;
					}

					iDate = pFundNavRaise->f_trade_date;
					//最近日净值增长率double
					resTable.SetCell(nCol,j,pFundNavRaise->f_nav_return_cur_day);
					nCol++;
					//最近一周净值增长率double
					resTable.SetCell(nCol,j,pFundNavRaise->f_nav_return_latest_week);
					nCol++;
					//最近一个月净值增长率double
					resTable.SetCell(nCol,j,pFundNavRaise->f_nav_return_lastet_month);
					nCol++;
					//最近三个月净值增长率double
					resTable.SetCell(nCol,j,pFundNavRaise->f_nav_return_latest_3M);
					nCol++;
					//最近六个月净值增长率double
					resTable.SetCell(nCol,j,pFundNavRaise->f_nav_return_latest_6M);
					nCol++;
					//最近一年净值增长率double
					resTable.SetCell(nCol,j,pFundNavRaise->f_nav_return_latest_1Y);
					nCol++;
					//最近二年净值增长率double
					resTable.SetCell(nCol,j,pFundNavRaise->f_nav_return_latest_2Y);
					nCol++;
					//今年以来净值增长率double
					resTable.SetCell(nCol,j,pFundNavRaise->f_nav_return_latest_this_year);
					nCol++;
					//设立以来净值增长率double
					resTable.SetCell(nCol,j,pFundNavRaise->f_nav_return_from_setup);
					nCol++;
				}
				else
				{
#ifdef _DEBUG
					GlobalWatch::_GetInstance()->WatchHere(_T("bond security id = %d non-exist"),*iter);
#endif
				}
			}

			prw.SetPercent(progId, 1.0);
			sProgressPrompt+=_T(",完成!");
			prw.SetText(progId, sProgressPrompt);

			delete pFundNav2Day;
			pFundNav2Day = NULL;


			return true;
		}
		//取得封闭式基金行情报价 add by wangych 08.09.11
		bool TxFund::GetCloseFundQuotePrice(
			std::vector<int>& iSecurityId,		//交易实体ID
			int date,							//数据日期,0表示最近交易日
			Tx::Core::Table_Display& resTable,	//结果数据表
			GridAdapterTable_Display& resAdapter//显示格式
			)
		{
			int ii = iSecurityId.size();
			if(ii<=0)
				return false;

			//step1
			//	Tx::Core::ProgressWnd prw;
			//step2
			CString sProgressPrompt;
			sProgressPrompt.Format(_T("基金净值..."));
			//	UINT progId = prw.AddItem(1,sProgressPrompt, 0.0);
			//step3
			//	prw.Show(15);
			//step4
			//	prw.SetPercent(progId, 0.1);

			//默认的返回值状态
			bool result = false;
			//清空数据
			resTable.Clear();
			int iCol = 0;

			//准备样本集=Security_ID,int型
			resTable.AddCol(Tx::Core::dtype_int4);
			iCol++;
			//样本名称
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.SetTitle(iCol,_T("名称"));
			iCol++;
			//样本外码
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.SetTitle(iCol,_T("代码"));
			iCol++;
			//现价
			resTable.AddCol(Tx::Core::dtype_float);
			resTable.SetPrecise(iCol,3);
			resTable.SetTitle(iCol,_T("现价"));
			iCol++;
			//日涨跌幅
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("日涨幅%"));
			iCol++;
			//日涨跌值
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("日涨跌"));
			resTable.SetPrecise(iCol,3);
			iCol++;
			//换手率
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("换手率%"));
			iCol++;
			//现量
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("现量(手)"));
			resTable.SetFormatStyle(iCol,fs_finance);
			iCol++;
			//成交量
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("成交量(手)"));
			resTable.SetFormatStyle(iCol,fs_finance);
			iCol++;
			//净值
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("净值"));
			resTable.SetPrecise(iCol,4);
			//resTable.SetFormatStyle(iCol, fs_date);
			iCol++;
			//总份额
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("总份额(亿份)"));
			resTable.SetOutputItemRatio(iCol,1e8);
			resTable.SetFormatStyle(iCol,fs_finance);
			iCol++;
			//最高价
			resTable.AddCol(Tx::Core::dtype_float);
			resTable.SetTitle(iCol,_T("最高"));
			resTable.SetPrecise(iCol,3);
			iCol++;
			//最低价
			resTable.AddCol(Tx::Core::dtype_float);
			resTable.SetTitle(iCol,_T("最低"));
			resTable.SetPrecise(iCol,3);
			iCol++;
			//前收价
			resTable.AddCol(Tx::Core::dtype_float);
			resTable.SetTitle(iCol,_T("昨收"));
			resTable.SetPrecise(iCol,3);
			iCol++;
			//开盘价
			resTable.AddCol(Tx::Core::dtype_float);
			resTable.SetTitle(iCol,_T("开盘"));
			resTable.SetPrecise(iCol,3);
			iCol++;
			//折价率
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("折价率%"));
			//resTable.SetFormatStyle(iCol, fs_date);
			iCol++;
			//成交金额
			resTable.AddCol(Tx::Core::dtype_double);
			resTable.SetTitle(iCol,_T("成交金额(万元)"));
			resTable.SetOutputItemRatio(iCol,1e4);
			resTable.SetFormatStyle(iCol,fs_finance);
			iCol++;


			//添加券ID
			int j=0;
			bool bIsFundCurrency = false;
			for(std::vector<int>::iterator iter = iSecurityId.begin();iter!=iSecurityId.end();iter++)
			{
				SecurityQuotation* pSecurity = GetSecurityNow(*iter);
				if(pSecurity!=NULL)
				{
					CString sFileIdKey;
					if(pSecurity->IsFund_Close()==true)
					{
						sFileIdKey = _T("fund_close_nav_2days");
					}
					else
						continue;
					j = resTable.GetRowCount();
					//根据样本数量添加记录数
					resTable.AddRow();
					int nCol = 0;
					//交易实体id
					resTable.SetCell(nCol,j,*iter);
					nCol++;
					//填充名称
					resTable.SetCell(nCol,j,pSecurity->GetName());
					nCol++;
					//填充外码
					resTable.SetCell(nCol,j,pSecurity->GetCode());
					nCol++;

					//现价
					float closeprice = pSecurity->GetClosePrice(true);
					if (closeprice >= 0)
					{
						resTable.SetCell(nCol,j,closeprice);
					} 
					else
					{
						resTable.SetCell(nCol,j,Con_floatInvalid);
					}
					nCol++;
					//日涨跌幅
					resTable.SetCell(nCol,j,pSecurity->GetRaise());
					nCol++;
					//日涨跌值
					resTable.SetCell(nCol,j,pSecurity->GetRaiseValue());
					nCol++;
					//换手率
					double tradeRate = pSecurity->GetTradeRate();
					if (tradeRate >= 0)
					{
						resTable.SetCell(nCol,j,tradeRate);
					} 
					else
					{
						resTable.SetCell(nCol,j,Con_doubleInvalid);
					}
					nCol++;
					//现量
					double volumeLatest = pSecurity->GetVolumeLatest(true);
					if (volumeLatest >= 0)
					{
						resTable.SetCell(nCol,j,volumeLatest);
					} 
					else
					{
						resTable.SetCell(nCol,j,Con_doubleInvalid);
					}
					nCol++;
					//成交量
					double volume = pSecurity->GetVolume(true);
					if (volume >= 0)
					{
						resTable.SetCell(nCol,j,volume);
					} 
					else
					{
						resTable.SetCell(nCol,j,Con_doubleInvalid);
					}
					nCol++;
					//净值
					double fundNAV = pSecurity->GetFundNAV();
					if (fundNAV >= 0)
					{
						resTable.SetCell(nCol,j,fundNAV);
					} 
					else
					{
						resTable.SetCell(nCol,j,Con_doubleInvalid);
					}
					//resTable.SetFormatStyle(iCol, fs_date);
					nCol++;
					//总份额
					double totalshare = pSecurity->GetTotalShare();
					if (totalshare >= 0)
					{
						resTable.SetCell(nCol,j,totalshare);
					} 
					else
					{
						resTable.SetCell(nCol,j,Con_doubleInvalid);
					}
					nCol++;
					//最高价
					float highprice = pSecurity->GetHighPrice();
					if (highprice >= 0)
					{
						resTable.SetCell(nCol,j,highprice);
					} 
					else
					{
						resTable.SetCell(nCol,j,Con_floatInvalid);
					}
					nCol++;
					//最低价
					float lowprice = pSecurity->GetLowPrice();
					if (lowprice >= 0)
					{
						resTable.SetCell(nCol,j,lowprice);
					} 
					else
					{
						resTable.SetCell(nCol,j,Con_floatInvalid);
					}
					nCol++;
					//前收价
					float precloseprice = pSecurity->GetPreClosePrice();
					if (precloseprice >= 0)
					{
						resTable.SetCell(nCol,j,precloseprice);
					} 
					else
					{
						resTable.SetCell(nCol,j,Con_floatInvalid);
					}
					nCol++;
					//开盘价
					float openprice = pSecurity->GetOpenPrice();
					if (openprice >= 0)
					{
						resTable.SetCell(nCol,j,openprice);
					} 
					else
					{
						resTable.SetCell(nCol,j,Con_floatInvalid);
					}
					nCol++;
					//折价率
					double overflowValue = GetOverflowValueRate(*iter,date);
					if (overflowValue != Con_doubleInvalid)
					{
						resTable.SetCell(nCol,j,overflowValue*100);
					}
					else
					{
						resTable.SetCell(nCol,j,Con_doubleInvalid);
					}
					nCol++;
					//成交金额
					double amount = pSecurity->GetAmount();
					if (amount >= 0)
					{
						resTable.SetCell(nCol,j,amount);
					} 
					else
					{
						resTable.SetCell(nCol,j,Con_doubleInvalid);
					}
					nCol++;
				}
				else
				{
#ifdef _DEBUG
					GlobalWatch::_GetInstance()->WatchHere(_T("fund security id = %d non-exist"),*iter);
#endif
				}
			}

			//	prw.SetPercent(progId, 1.0);
			sProgressPrompt+=_T(",完成!");
			//	prw.SetText(progId, sProgressPrompt);

			return true;
		}
		//取得行业配置 add by wangych 08.09.12
		bool TxFund::GetIndustryDistribute(
			std::vector<int>& iSecurityId,		//交易实体ID
			int date1,							//数据日期,所选报告期
			int date2,							//数据日前，所选上个报告期
			Tx::Core::Table_Display& resTable,	//结果数据表
			GridAdapterTable_Display& resAdapter//显示格式
			)
		{
			//step1
			//	Tx::Core::ProgressWnd* pProgressWnd = Tx::Core::ProgressWnd::GetInstance();
			Tx::Core::ProgressWnd prw;
			//step2
			CString sProgressPrompt;
			sProgressPrompt.Format(_T("基金行业分布..."));
			UINT progId = prw.AddItem(1,sProgressPrompt, 0.0);
			//step3
			prw.Show(15);
			//step4
			prw.SetPercent(progId, 0.1);

			//默认的返回值状态
			bool result = false;
			//清空数据
			resTable.Clear();
			int iCol = 0;

			//准备样本集=Security_ID,int型
			resTable.AddCol(Tx::Core::dtype_int4);
			iCol++;
			//样本名称
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.SetTitle(iCol,_T("名称"));
			iCol++;
			//样本外码
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.SetTitle(iCol,_T("代码"));
			iCol++;
			//投资类型
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.SetTitle(iCol,_T("投资类型"));
			iCol++;
			//样本外码
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.SetTitle(iCol,_T("投资风格"));
			iCol++;

			for (int i =0;i<23;i++)
			{
				//市值(万元)
				resTable.AddCol(Tx::Core::dtype_double);
				resTable.SetTitle(iCol,_T("市值(万元)"));
				resTable.SetOutputItemRatio(iCol,1e4);
				resTable.SetFormatStyle(iCol,fs_finance);
				resTable.SetPrecise(iCol,2);
				iCol++;
				//市值增减(万元)
				resTable.AddCol(Tx::Core::dtype_double);
				resTable.SetTitle(iCol,_T("市值增减(万元)"));
				resTable.SetOutputItemRatio(iCol,1e4);
				resTable.SetFormatStyle(iCol,fs_finance);
				resTable.SetPrecise(iCol,2);
				iCol++;
				//占净值比例%
				resTable.AddCol(Tx::Core::dtype_double);
				resTable.SetTitle(iCol,_T("占净值比例%"));
				resTable.SetPrecise(iCol,2);
				iCol++;
				//占净值比例增减%
				resTable.AddCol(Tx::Core::dtype_double);
				resTable.SetTitle(iCol,_T("占净值比例增减%"));
				resTable.SetPrecise(iCol,2);
				iCol++;
			}

			//成立日期
			resTable.AddCol(Tx::Core::dtype_int4);
			resTable.SetTitle(iCol,_T("成立日期"));
			iCol++;
			//基金公司
			resTable.AddCol(Tx::Core::dtype_val_string);
			resTable.SetTitle(iCol,_T("基金公司"));
			iCol++;

			//填入数据
			DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundStockInvestmentIndustryDistribute>* pFundInvDistributeDataFile1 = new DataFileNormal<blk_TxExFile_FileHead,
				Tx::Data::FundStockInvestmentIndustryDistribute>;
			pFundInvDistributeDataFile1->SetCheckLoadById(true);


			bool bLoaded1 = false;
			if(pFundInvDistributeDataFile1!=NULL)
				bLoaded1 = pFundInvDistributeDataFile1->Load(date1,30155,true);
			if ( !bLoaded1 )
				return false;

			DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundStockInvestmentIndustryDistribute>* pFundInvDistributeDataFile2 = new DataFileNormal<blk_TxExFile_FileHead,
				Tx::Data::FundStockInvestmentIndustryDistribute>;
			pFundInvDistributeDataFile2->SetCheckLoadById(true);


			bool bLoaded2 = false;
			if(pFundInvDistributeDataFile2!=NULL)
				bLoaded2 = pFundInvDistributeDataFile2->Load(date2,30155,true);
			if ( !bLoaded2 )
				return false;

			int j=0;
			for(std::vector<int>::iterator iter = iSecurityId.begin();iter!=iSecurityId.end();iter++)
			{
				SecurityQuotation* pSecurity = GetSecurityNow(*iter);
				if(pSecurity!=NULL)
				{			
					//券ID
					int  nAccountingID = 0;
					nAccountingID = pSecurity->GetSecurity1Id();

					Tx::Data::FundCombineInvestmentIndustryDistribute* pFundComInvDistribute1 = NULL ;//投资组合行业分布
					pFundComInvDistribute1 = pFundInvDistributeDataFile1->GetDataByObj(nAccountingID,false);
					//if (pFundComInvDistribute1 == NULL)
					//{
					//	continue;
					//}

					Tx::Data::FundCombineInvestmentIndustryDistribute* pFundComInvDistribute2 = NULL ;//投资组合行业分布
					pFundComInvDistribute2 = pFundInvDistributeDataFile2->GetDataByObj(nAccountingID,false);
					if (pFundComInvDistribute2 == NULL)
					{
						continue;
					}

					j = resTable.GetRowCount();
					//根据样本数量添加记录数
					resTable.AddRow();
					int nCol = 0;
					//交易实体id
					resTable.SetCell(nCol,j,*iter);
					nCol++;
					//填充名称
					resTable.SetCell(nCol,j,pSecurity->GetName());
					nCol++;
					//填充外码
					resTable.SetCell(nCol,j,pSecurity->GetCode());
					nCol++;
					//投资类型
					nCol++;
					//投资风格
					nCol++;

					for(int i = 0;i<23;i++)
					{
						double dValue2;
						if ( pFundComInvDistribute2!=NULL )
							dValue2 = pFundComInvDistribute2->dFinancial[i];
						else
							dValue2 = Con_doubleInvalid;
						resTable.SetCell(nCol,j,dValue2);
						nCol++;

						double dValue1,dAddReduceNum;;
						if ( pFundComInvDistribute1!=NULL )
						{
							dValue1 = pFundComInvDistribute1->dFinancial[i];
							dAddReduceNum = dValue2 -dValue1;
						}
						else
							dAddReduceNum = Con_doubleInvalid;
						resTable.SetCell(nCol,j,dAddReduceNum);
						nCol++;

						double percent2;
						if (pFundComInvDistribute2!= NULL)
						{
							percent2=dValue2/pFundComInvDistribute2->dFinancial[23]*100;
						} 
						else
						{
							percent2 = Con_doubleInvalid;
						}
						resTable.SetCell(nCol,j,percent2);
						nCol++;

						double percent1,dAddReducePercent;
						if ( pFundComInvDistribute1!= NULL)
						{
							percent1=dValue1/pFundComInvDistribute1->dFinancial[23]*100;
							dAddReducePercent = percent2 -percent1;
						} 
						else
						{
							dAddReducePercent = Con_doubleInvalid;
						}
						resTable.SetCell(nCol,j,dAddReducePercent);
						nCol++;
					}

					//显示基金的成立日期，基金公司
					//把基金的名称和代码添加到表里
					std::unordered_map<int,CString> InStitutionIdToName;
					std::unordered_map<int,CString>::iterator iter;
					TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_INSTITUTIONID_TO_SHORTNAME,InStitutionIdToName);
					FundNewInfo *pfundInfo;
					int date,iInstitutionId;

					pfundInfo = pSecurity->GetFundNewInfo();
					date = pfundInfo->setup_date;
					resTable.SetCell(nCol,j,date);
					nCol++;

					iInstitutionId = pSecurity->GetInstitutionId();	
					iter = InStitutionIdToName.find(iInstitutionId);
					if(iter != InStitutionIdToName.end())
						resTable.SetCell(nCol,j,iter->second);
					nCol++;

				}
				else
				{
#ifdef _DEBUG
					GlobalWatch::_GetInstance()->WatchHere(_T("fund security id = %d non-exist"),*iter);
#endif
				}
			}

			delete pFundInvDistributeDataFile1;
			pFundInvDistributeDataFile1 = NULL;
			delete pFundInvDistributeDataFile2;
			pFundInvDistributeDataFile2 = NULL;

			prw.SetPercent(progId, 1.0);
			sProgressPrompt+=_T(",完成!");
			prw.SetText(progId, sProgressPrompt);

			return true;
		}

		//基金的持有人结构   add by lijw 2008-09-16
		bool TxFund::HoldPersonStruct(std::vector<int> &iSecurityId,Tx::Core::Table_Indicator& resTable,
			std::vector<int> &vDate)

		{
			//添加进度条
			ProgressWnd prw;
			UINT pid=prw.AddItem(1,_T("持有人结构统计..."),0.0);
			prw.Show(1000);
			m_txTable.Clear();//这是引用别人的成员变量，
			//默认的返回值状态
			bool result = false;
			//清空数据
			m_txTable.Clear();
			//准备样本集参数列
			m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//基金交易实体ID
			m_txTable.AddParameterColumn(Tx::Core::dtype_int4);//截止日期
			const int indicatorIndex = 4;
			long iIndicator[indicatorIndex] = 
			{
				30301118,	//期末持有人户数
				30301119,	//平均每户持有份额
				30301120,	//机构投资者持有份额
				30301121	//个人投资者持有份额
			};
			UINT varCfg[2];			//参数配置
			int varCount=2;			//参数个数
			for (int i = 0; i < indicatorIndex; i++)
			{
				int tempIndicator = iIndicator[i];

				GetIndicatorDataNow(tempIndicator);
				if (m_pIndicatorData==NULL)
				{ return false; }
				varCfg[0]=0;
				varCfg[1]=1;
				result = m_pLogicalBusiness->SetIndicatorIntoTable(
					m_pIndicatorData,	//指标
					varCfg,				//参数配置
					varCount,			//参数个数
					m_txTable	//计算需要的参数传输载体以及计算后结果的载体
					);
				if(result==false)
				{
					return FALSE;
				}

			}
			UINT iColCount = m_txTable.GetColCount();
			UINT* nColArray = new UINT[iColCount];
			for(int i = 0; i < (int)iColCount; i++)
			{
				nColArray[i] = i;
			}
			result = m_pLogicalBusiness->GetData(m_txTable,true);
			if(result == false || m_txTable.GetRowCount() == 0)
			{
				//添加进度条
				prw.SetPercent(pid,1.0);
				return false;
			}
#ifdef _DEBUG
			CString strTable=m_txTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
			Tx::Core::Table_Indicator tempTable;
			tempTable.CopyColumnInfoFrom(m_txTable);
			//根据基金交易实体ID进行筛选
			m_txTable.EqualsAt(tempTable,nColArray,iColCount,0,iSecurityId);
			if(tempTable.GetRowCount() == 0)
			{ 
				delete nColArray;
				nColArray = NULL;
				//添加进度条
				prw.SetPercent(pid,1.0);
				return false;
			}
#ifdef _DEBUG
			strTable=tempTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
			resTable.CopyColumnInfoFrom(tempTable);
			std::vector<int> tempVec;
			tempVec.push_back(*vDate.rbegin());
			//进行报告期的筛选,它是以截止日期为基准的。取得最近日期的数据
			tempTable.EqualsAt(resTable,nColArray,iColCount,1,tempVec);
#ifdef _DEBUG
			strTable=resTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
			if(resTable.GetRowCount() == 0)
			{
				delete nColArray;
				nColArray = NULL;
				//添加进度条
				prw.SetPercent(pid,1.0);
				return false;
			}
			m_txTable.Clear();
			m_txTable.CopyColumnInfoFrom(tempTable);
			tempVec.clear();
			tempVec.push_back(*vDate.begin());
			//取得次近日期的数据
			tempTable.EqualsAt(m_txTable,nColArray,iColCount,1,tempVec);
#ifdef _DEBUG
			strTable=m_txTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
			delete nColArray;
			nColArray = NULL;
			//取得最近日期与次近日期总份额增减的值
			resTable.InsertCol(2,Tx::Core::dtype_double);//份额合计
			resTable.InsertCol(3,Tx::Core::dtype_double);//份额增减
			int icount = resTable.GetRowCount();
			double dPersonHold,dInstitutionHold,dData;
			int iTradeId;
			std::vector<UINT> vecInstiID;
			std::vector<UINT>::iterator iteID;
			for(int i = 0;i < icount;i++)
			{
				resTable.GetCell(6,i,dInstitutionHold);
				resTable.GetCell(7,i,dPersonHold);
				dData = dPersonHold + dInstitutionHold;
				resTable.SetCell(2,i,dData);
				resTable.GetCell(0,i,iTradeId);
				if(!vecInstiID.empty())
					vecInstiID.clear();
				m_txTable.Find(0,iTradeId,vecInstiID);
				if(vecInstiID.empty())
					continue;
				if(vecInstiID.size() != 1)//由于所取数据所在的表的唯一聚簇索引是交易实体ID和截止日期
					ASSERT(0);
				iteID = vecInstiID.begin();
				m_txTable.GetCell(4,*iteID,dInstitutionHold);
				m_txTable.GetCell(5,*iteID,dPersonHold);
				//计算份额增减
				dData = dData - dInstitutionHold - dPersonHold;
				resTable.SetCell(3,i,dData);
			}
#ifdef _DEBUG
			strTable=resTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
			//
			prw.SetPercent(pid,0.6);//添加进度条

			resTable.InsertCol(2,Tx::Core::dtype_val_string);//基金的名称
			resTable.InsertCol(3,Tx::Core::dtype_val_string);//基金的代码
			resTable.InsertCol(4,Tx::Core::dtype_val_string);//基金的类型
			resTable.InsertCol(5,Tx::Core::dtype_val_string);//基金的风格
			resTable.InsertCol(11,Tx::Core::dtype_double);//11机构投资比例
			resTable.AddCol(Tx::Core::dtype_double);//13 个人投资比例
			resTable.AddCol(Tx::Core::dtype_int4);//14成立日期
			resTable.AddCol(Tx::Core::dtype_val_string);//15基金公司
			std::unordered_map<int,CString> CompanyMap;
			TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_INSTITUTIONID_TO_SHORTNAME,CompanyMap);
			std::unordered_map<int,CString> TypeMap;
			TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_TYPE_INDEX,TypeMap);
			std::unordered_map<int,CString> StyleMap;
			//TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX,StyleMap);
			//modified by zhangxs 20091221---NewStyle
			TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX_NEW,StyleMap);
			std::unordered_map<int,CString>::iterator iterMap;
			//增加基金名称，代码
			double dTotalHold;
			double dInstiPro,dPerPro;
			int iInstitutionId;
			std::vector<int>::iterator iterId;
			FundNewInfo *pNewInfo;
			for(iterId = iSecurityId.begin();iterId != iSecurityId.end();++iterId)
			{
				int TradeId = *iterId ;
				CString strName,strCode;
				GetSecurityNow(TradeId);
				if(m_pSecurity == NULL)
					continue;
				strName = m_pSecurity->GetName();
				strCode = m_pSecurity->GetCode();
				pNewInfo = m_pSecurity->GetFundNewInfo();
				iInstitutionId = m_pSecurity->GetInstitutionId();
				if(!vecInstiID.empty())
					vecInstiID.clear();
				resTable.Find(0,TradeId,vecInstiID);
				for(iteID = vecInstiID.begin();iteID != vecInstiID.end();++iteID)
				{
					UINT position = *iteID;
					resTable.SetCell(2,position,strName);
					resTable.SetCell(3,position,strCode);
					//计算比例
					resTable.GetCell(10,position,dInstitutionHold);
					resTable.GetCell(12,position,dPersonHold);
					resTable.GetCell(6,position,dTotalHold);
					dInstiPro = 100*dInstitutionHold/dTotalHold;
					dPerPro = 100*dPersonHold/dTotalHold;
					resTable.SetCell(11,position,dInstiPro);
					resTable.SetCell(13,position,dPerPro);
					resTable.SetCell(14,position,pNewInfo->setup_date);
					//填入基金风格，基金类型，基金公司简称
					iterMap = TypeMap.find(pNewInfo->type_id);
					if(iterMap != TypeMap.end())
						resTable.SetCell(4,position,iterMap->second);
					iterMap = StyleMap.find(pNewInfo->style_id);
					if(iterMap != StyleMap.end())
						resTable.SetCell(5,position,iterMap->second);
					iterMap = CompanyMap.find(iInstitutionId);
					if(iterMap != CompanyMap.end())
						resTable.SetCell(15,position,iterMap->second);
				}

			}
#ifdef _DEBUG
			strTable=resTable.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
			resTable.DeleteCol(1);
			//添加进度条
			prw.SetPercent(pid,1.0);
			return true;
		}
		//2008-09-12
		//根据自然日期取得基金报告期日期
		int TxFund::GetFundReportDate(int date,bool bQuarter,int iMarketId)
		{
			if(iMarketId<=0)
				iMarketId = m_pFunctionDataManager->GetBourseId_ShangHai();
			//取得当日日期
			int curDate = m_pFunctionDataManager->GetServerCurDateTime(iMarketId).GetDate().GetInt();

			int iyear = date/10000;

			//
			bool bHis = false;
			if(iyear<(curDate/10000))
				bHis = true;
			else
				//按照当年日期判断
				bHis = false;
			return (iyear*10000+GetFundReportDateMD(date,bHis,bQuarter));
		}
		int TxFund::GetFundReportDateMD(int date,bool bHis,bool bQuarter)
		{
			int month_day = date%10000;
			if(bHis==true)
			{
				//历史日期
				if(bQuarter==true)
				{
					//季报
					if(month_day >= 101 && month_day<331)
						month_day = 1231;
					else if(month_day>=331 && month_day<630)
						month_day = 331;
					else if(month_day>=630 && month_day<930)
						month_day = 630;
					else if(month_day>=930 && month_day<1231)
						month_day = 930;
					else month_day = 0;
				}
				else
				{
					//中报年报
					if(month_day >= 101 && month_day<630)
						month_day = 1231;
					else if(month_day>=630 && month_day<=1231)
						month_day = 630;
					else month_day = 0;
				}
			}
			else
			{
				//当年日期
				if(bQuarter==true)
				{
					//季报
					if(month_day >= 120 && month_day<420)
						month_day = 1231;
					else if(month_day>=420 && month_day<720)
						month_day = 331;
					else if(month_day>=720 && month_day<1020)
						month_day = 630;
					else if((month_day>=1020 && month_day<=1231) ||(month_day>=101 && month_day<120))
						month_day = 930;
					else month_day = 0;
				}
				else
				{
					//中报年报
					if(month_day >= 320 && month_day<820)
						month_day = 1231;
					else if((month_day>=820 && month_day<=1231) || (month_day>=101 && month_day<320))
						month_day = 630;
					else month_day = 0;
				}
			}
			return month_day;
		}
		/*
		Case "一季报"
		nQuarter = 1
		Case "二季报"
		nQuarter = 2
		Case "中报"
		nQuarter = 3
		Case "三季报"
		nQuarter = 4
		Case "四季报"
		nQuarter = 6
		Case "年报"
		nQuarter = 9

		*/
		//2008-09-11
		//重仓债券=明细=前5名为重仓
		bool TxFund::GetMyBonds(std::vector<int> iSecurityId,Table_Display& baTable,int iYear,int iQuarter,int no)
		{
			if(no<=0)
				no = 5;

			int ii = iSecurityId.size();

			//step1
			//	Tx::Core::ProgressWnd* pProgressWnd = Tx::Core::ProgressWnd::GetInstance();
			Tx::Core::ProgressWnd prw;
			//step2
			CString sProgressPrompt;
			sProgressPrompt.Format(_T("基金重仓债券..."));
			UINT progId = prw.AddItem(1,sProgressPrompt, 0.0);
			//step3
			prw.Show(15);
			//step4
			prw.SetPercent(progId, 0.1);

			//默认的返回值状态
			bool result = false;
			//清空数据
			baTable.Clear();
			int iCol = 0;

			//准备样本集=Security_ID,int型
			baTable.AddCol(Tx::Core::dtype_int4);
			iCol++;
			//样本名称
			baTable.AddCol(Tx::Core::dtype_val_string);
			baTable.SetTitle(iCol,_T("名称"));
			iCol++;
			//样本外码
			baTable.AddCol(Tx::Core::dtype_val_string);
			baTable.SetTitle(iCol,_T("代码"));
			iCol++;

			//投资风格
			baTable.AddCol(Tx::Core::dtype_val_string);
			baTable.SetTitle(iCol,_T("投资风格"));
			iCol++;

			//投资类型
			baTable.AddCol(Tx::Core::dtype_val_string);
			baTable.SetTitle(iCol,_T("投资类型"));
			iCol++;

			//基金净值（万元）
			baTable.AddCol(Tx::Core::dtype_double);
			baTable.SetTitle(iCol,_T("基金净值(万元)"));
			baTable.SetPrecise(iCol,2);
			baTable.SetOutputItemRatio(iCol,10000);
			iCol++;
			//重仓债券顺序
			baTable.AddCol(Tx::Core::dtype_int4);
			baTable.SetTitle(iCol,_T("重仓债券顺序"));
			iCol++;
			//债券代码
			baTable.AddCol(Tx::Core::dtype_val_string);
			baTable.SetTitle(iCol,_T("债券代码"));
			baTable.SetPrecise(iCol,2);
			iCol++;
			//债券名称
			baTable.AddCol(Tx::Core::dtype_val_string);
			baTable.SetTitle(iCol,_T("债券名称"));
			baTable.SetPrecise(iCol,2);
			iCol++;
			//市值（万元）
			baTable.AddCol(Tx::Core::dtype_double);
			baTable.SetTitle(iCol,_T("市值(万元)"));
			baTable.SetPrecise(iCol,2);
			baTable.SetOutputItemRatio(iCol,10000);
			iCol++;
			//占净值比例%
			baTable.AddCol(Tx::Core::dtype_double);
			baTable.SetTitle(iCol,_T("占净值比例%"));
			baTable.SetPrecise(iCol,2);
			iCol++;
			//成立日期
			baTable.AddCol(Tx::Core::dtype_int4);
			baTable.SetTitle(iCol,_T("成立日期"));
			baTable.SetFormatStyle(iCol,fs_date);
			iCol++;
			//基金公司
			baTable.AddCol(Tx::Core::dtype_val_string);
			baTable.SetTitle(iCol,_T("基金公司"));
			iCol++;

			CString sFileIdKey = _T("fund_bond_detail_for_year");
			int iFileId = 
				Tx::Data::DataStatus::GetInstance()->GetExFileIdFromIni(
				Tx::Core::SystemPath::GetInstance()->GetSystemDataPath(),
				sFileIdKey);

			//取得季报报告日期
			bool bQuarte = true;
			int date = iYear*10000+iQuarter;//GetFundReportDate(date,bQuarte);

			DataFileNormal<blk_TxExFile_FileHead,FundBondDetail>* pFundBondDetail = new DataFileNormal<blk_TxExFile_FileHead,FundBondDetail>;
			if(pFundBondDetail==NULL)
				return false;
			pFundBondDetail->SetOverWriteMapKey(true);
			pFundBondDetail->SetCheckLoadById(true);
			if(pFundBondDetail->Load(
				date,//文件名=date.dat
				iFileId,//文件所在目录
				true)==false || pFundBondDetail->GetDataCount()<=1)
			{
				delete pFundBondDetail;
				return false;
			}

			//添加券ID
			int j=0;
			for(std::vector<int>::iterator iter = iSecurityId.begin();iter!=iSecurityId.end();iter++)
			{
				SecurityQuotation* pSecurity = GetSecurityNow(*iter);
				if(pSecurity!=NULL)
				{
					if(pSecurity->IsFund()==false)
					{
						continue;
					}
					int iSecurity1Id = pSecurity->GetSecurity1Id();
					FundBondDetail* pFundBondDetailData = pFundBondDetail->GetDataByObj(iSecurity1Id,false);
					if(pFundBondDetailData==NULL)
					{
						continue;
					}
					for(;;)
					{
#ifdef _DEBUG
						if( iSecurity1Id == pFundBondDetailData->GetMapObj())
#else
						if( iSecurity1Id == pFundBondDetailData->GetMapObj() && pFundBondDetailData->f_no<=no)
#endif
						{
							j = baTable.GetRowCount();
							//根据样本数量添加记录数
							baTable.AddRow();
							iCol = 0;
							//交易实体id
							baTable.SetCell(iCol,j,*iter);
							iCol++;
							//填充名称
							baTable.SetCell(iCol,j,pSecurity->GetName());
							iCol++;
							//填充外码
							baTable.SetCell(iCol,j,pSecurity->GetCode());
							iCol++;
							//投资风格
							baTable.SetCell(iCol,j,pSecurity->GetTypeName(1));
							iCol++;

							//投资类型
							baTable.SetCell(iCol,j,pSecurity->GetTypeName(2));
							iCol++;

							//基金净值（万元）
							if(pSecurity->GetFundNAV()>0 && pSecurity->GetTotalShare()>0)
								baTable.SetCell(iCol,j,pSecurity->GetFundNAV()*pSecurity->GetTotalShare());
							else
								baTable.SetCell(iCol,j,Con_doubleInvalid);
							iCol++;
							//重仓债券顺序
							baTable.SetCell(iCol,j,pFundBondDetailData->f_no);
							//baTable.SetCell(iCol,j,pFundBondDetailData->f_market_value_order);
							iCol++;

							SecurityQuotation* pBond = GetSecurityNow(pFundBondDetailData->f1);
							if(pBond!=NULL)
							{
								//债券代码
								baTable.SetCell(iCol,j,pBond->GetCode());
								iCol++;
								//债券名称
								baTable.SetCell(iCol,j,pBond->GetName());
								iCol++;
								//市值（万元）
								baTable.SetCell(iCol,j,pFundBondDetailData->dValue[0]);
								iCol++;
								//占净值比例%
								baTable.SetCell(iCol,j,pFundBondDetailData->dValue[1]);
								//baTable.SetCell(iCol,j,pFundBondDetailData->dValue[2]);
								iCol++;
							}
							else
							{
								//债券代码
								iCol++;
								//债券名称
								iCol++;
								//市值（万元）
								iCol++;
								//占净值比例%
								iCol++;
							}
							//成立日期
							baTable.SetCell(iCol,j,pSecurity->GetIPOListedDate());
							iCol++;
							//基金公司
							baTable.SetCell(iCol,j,pSecurity->GetInstitutionName());
							iCol++;
						}
						if(pFundBondDetailData->f_no<=1)
							break;
						pFundBondDetailData--;
						if(pFundBondDetailData==NULL)
							break;
					}
				}
				else
				{
#ifdef _DEBUG
					GlobalWatch::_GetInstance()->WatchHere(_T("bond security id = %d non-exist"),*iter);
#endif
				}
			}

			prw.SetPercent(progId, 1.0);
			sProgressPrompt+=_T(",完成!");
			prw.SetText(progId, sProgressPrompt);

			delete pFundBondDetail;
			pFundBondDetail = NULL;

			MultiSortRule msrCol;
			msrCol.AddRule(2);
			msrCol.AddRule(6);
			baTable.SortInMultiCol(msrCol);

			return true;
		}
		//added by zhangxs
		//股票持有情况统计
		bool TxFund::StatFundStockHolding(
			Tx::Core::Table_Indicator& sourceTable,	//为双击后保存数据。
			Tx::Core::Table_Indicator& resTableStock,	//股票结果数据表
			Tx::Core::Table_Indicator& resTableFund,	//基金结果数据表
			std::vector<int>& iFundSecurityId,		//基金交易实体ID
			std::vector<int>& iStockSecurityId,		//股票交易实体ID
			std::vector<int> vDate,				//报告期
			bool bIsZCG,							//数据源为重仓股；否则为持股明细
			//std::set<int> & TradeId,			//保存所取得sourceTable里的数据的全部的交易实体ID
			std::vector<CString> &vColName,
			std::vector<CString> &vColHeaderName,
			int& m_iFirstSampleId
			)
		{
			ProgressWnd prw;
			UINT pid=prw.AddItem(1,_T("股票持有情况统计..."),0.0);
			prw.Show(1000);
			//#ifdef _DEBUG
			//	strTable=sourceTable.TableToString();
			//	Tx::Core::Commonality::String().StringToClipboard(strTable);
			//#endif
			//添加列的类型
			resTableStock.AddCol(Tx::Core::dtype_int4);//0交易实体ID
			resTableStock.AddCol(Tx::Core::dtype_val_string);//1股票名称
			resTableStock.AddCol(Tx::Core::dtype_val_string);//2股票代码
			resTableStock.AddCol(Tx::Core::dtype_int4);//3上市日期
			resTableStock.AddCol(Tx::Core::dtype_double);//4总股本
			resTableStock.AddCol(Tx::Core::dtype_double);//5流通股本
			resTableStock.AddCol(Tx::Core::dtype_val_string);//6证监会行业
			resTableStock.AddCol(Tx::Core::dtype_val_string);//7天相行业

			//填充标题
			vColName.clear();
			vColName.push_back(_T("股票名称"));
			vColName.push_back(_T("股票代码"));	
			int isize = (int)vDate.size();
			std::set<int,greater<int> > ErasePosSet;
			std::set<int,greater<int> >::iterator EraseIter;
			int tempPosRow = -1;
			for(int k = 0;k < isize;k++)
			{		
				//填充标题
				CString stryear,strdate;
				int tempdate,iyear;
				tempdate = vDate[k];
				if(bIsZCG)
				{
					int itemp;
					itemp = tempdate%10000;
					iyear = tempdate/10000;
					stryear.Format(_T("%d"),iyear);
					switch(itemp)
					{
					case 331:
						strdate = stryear + _T("年") + _T("一季报");
						break;
					case 630:
						strdate = stryear + _T("年") + _T("二季报");
						break;
					case 930:
						strdate = stryear + _T("年") + _T("三季报");
						break;
					case 1231:
						strdate = stryear + _T("年") + _T("四季报");
						break;
					}
				}
				//暂时先把他们的报告期写为二、四季报。等需要时，
				//可以直接把他们改称中报和年报，无须修改程序。
				else
				{
					iyear = tempdate/10000;
					stryear.Format(_T("%d"),iyear);
					if(tempdate%10000 == 630)
						strdate = stryear + _T("年") + _T("二季报");
					if(tempdate%10000 == 1231)
						strdate = stryear + _T("年") + _T("四季报");
				}
				vColHeaderName.push_back(strdate);
				vColName.push_back(_T("持有市值(元)"));
				vColName.push_back(_T("占流通A股比例%"));
				vColName.push_back(_T("持股数(股)"));
				vColName.push_back(_T("持有该股基金总数")); //bug:10423  将"持有该股基金数"改为"持有该股基金总数" 2012-4-20
				//添加相应的列
				tempPosRow++;
				resTableStock.InsertCol(3+tempPosRow*4,Tx::Core::dtype_double);
				resTableStock.InsertCol(4+tempPosRow*4,Tx::Core::dtype_double);
				resTableStock.InsertCol(5+tempPosRow*4,Tx::Core::dtype_double);
				resTableStock.InsertCol(6+tempPosRow*4,Tx::Core::dtype_int4);//基金支数
			}
			vColName.push_back(_T("上市日期"));
			vColName.push_back(_T("A股总股本"));
			vColName.push_back(_T("流通股本"));
			vColName.push_back(_T("证监会行业"));
			vColName.push_back(_T("天相行业"));
			//把需要删掉的日期删掉
			std::vector<int>::iterator iterV;
			for(EraseIter = ErasePosSet.begin();EraseIter != ErasePosSet.end();++EraseIter)
			{
				iterV = find(vDate.begin(),vDate.end(),*EraseIter);
				if(iterV != vDate.end())
					vDate.erase(iterV);
			}
			isize = (int)vDate.size();
			//填充resTableFund和resTableStock的数据
			resTableFund.CopyColumnInfoFrom(sourceTable);
			//首先把reaTable里的股票全部取出来
			int iRowCount = sourceTable.GetRowCount();
			CString strData;
			double dData;
			std::vector<int>::iterator iterVector;
			int flag = -1;
			int iData;
			int iRow = -1;
			std::vector<int> iSecurityIdRecord;
			iSecurityIdRecord.clear();
			Tx::Data::StockHeldSum* pStockHeldSum = NULL;
			DataFileNormal<blk_TxExFile_FileHead,Tx::Data::StockHeldSum>* pStockHeldSumDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::StockHeldSum>;
			pStockHeldSumDataFile->SetCheckLoadById(true);
			int iDirId = 30122;
			int iDirIdDetail = 30119;
			int m_iTotal = (int)iStockSecurityId.size();
			int tempNo = 1;
			double m_dPid = 0.0;
			for(iterVector = iStockSecurityId.begin();iterVector != iStockSecurityId.end();++iterVector)
			{
				m_dPid = (double)tempNo/(double)m_iTotal*0.9; 
				prw.SetPercent(pid,m_dPid);
				//填充resTableStock			
				SecurityQuotation* pSecurity = GetSecurityNow(*iterVector);
				if( pSecurity == NULL )
				{
					tempNo++;
					continue;
				}
				resTableStock.AddRow();
				iRow++;
				//取得股票交易实体
				resTableStock.SetCell(0,iRow,*iterVector);
				//股票名称
				strData = pSecurity->GetName();		
				resTableStock.SetCell(1,iRow,strData);
				//股票代码
				strData = pSecurity->GetCode();
				resTableStock.SetCell(2,iRow,strData);
				//上市日期
				iData = pSecurity->GetIPOListedDate();
				resTableStock.SetCell(3+isize*4,iRow,iData);
				//总股本
				dData = pSecurity->GetTotalShare();
				if(dData <= 0)
					dData = Tx::Core::Con_doubleInvalid;
				resTableStock.SetCell(4+isize*4,iRow,dData);
				//流通股本
				double capitalization = pSecurity->GetTradableShare();
				if(capitalization <= 0)
					capitalization = Tx::Core::Con_doubleInvalid;
				resTableStock.SetCell(5+isize*4,iRow,capitalization);
				//证监会行业
				strData = pSecurity->GetCSRCIndustryName();		
				resTableStock.SetCell(6+isize*4,iRow,strData);
				//天相行业
				strData = pSecurity->GetTxSecIndustryName();
				resTableStock.SetCell(7+isize*4,iRow,strData);
				//用于记录该股票是否被显示
				int VisibleFlag = 0;
				//modified by zhangxs 
				int j = 0;
				int m_iNullRowCount = 0;
				if(bIsZCG)
				{
					for(std::vector<int>::iterator iterat = vDate.begin();iterat != vDate.end();iterat++,j++)
					{
						if(pStockHeldSumDataFile->Load(
							*iterat,//文件名=2020202.dat
							iDirId,//文件所在目录
							true)==false)// || pStockHeldSum->GetDataCount()<=1)
						{
							m_iNullRowCount++;
							continue;
						}
						int iFundholdvolum = Con_intInvalid;
						double dFundHoldShare = Con_doubleInvalid;
						double dLtShareRate = Con_doubleInvalid;
						double dFundHoldMarketValue = Con_doubleInvalid;
						pStockHeldSum = NULL;
						pStockHeldSum = pStockHeldSumDataFile->GetDataByObj(*iterVector,false);
						if(pStockHeldSum == NULL)
						{
							m_iNullRowCount++;
							continue;
						}
						int m_iSecurityId = pStockHeldSum->f_trans_obj_id;
						iFundholdvolum = pStockHeldSum->f_fund_holding_volume;
						dFundHoldShare = pStockHeldSum->f_fund_holding_share;
						dFundHoldMarketValue = pStockHeldSum->f_fund_holding_market_value;
						dLtShareRate = pStockHeldSum->f_percent_tradable_share;
						//持股市值(元)
						resTableStock.SetCell(3+j*4,iRow,dFundHoldMarketValue);
						//占流通股比例
						resTableStock.SetCell(4+j*4,iRow,dLtShareRate);
						//持股数
						resTableStock.SetCell(5+j*4,iRow,dFundHoldShare);
						//持有该基金数
						resTableStock.SetCell(6+j*4,iRow,iFundholdvolum);
						VisibleFlag++;
					}
					if( j == m_iNullRowCount )
					{
						resTableStock.DeleteRow(iRow);
						resTableStock.Arrange();
						iRow--;
					}
					else
						iSecurityIdRecord.push_back(*iterVector);

				}
				else
				{
					for(std::vector<int>::iterator iterat = vDate.begin();iterat != vDate.end();iterat++,j++)
					{
						if(pStockHeldSumDataFile->Load(
							*iterat,//文件名=2020202.dat
							iDirIdDetail,//文件所在目录
							true)==false)// || pStockHeldSum->GetDataCount()<=1)
						{
							m_iNullRowCount++;
							continue;
						}
						int iFundholdvolum = Con_intInvalid;
						double dFundHoldShare = Con_doubleInvalid;
						double dLtShareRate = Con_doubleInvalid;
						double dFundHoldMarketValue = Con_doubleInvalid;
						pStockHeldSum = NULL;
						pStockHeldSum = pStockHeldSumDataFile->GetDataByObj(*iterVector,false);
						if(pStockHeldSum == NULL)
						{
							m_iNullRowCount++;
							continue;
						}
						int m_iSecurityId = pStockHeldSum->f_trans_obj_id;
						iFundholdvolum = pStockHeldSum->f_fund_holding_volume;
						dFundHoldShare = pStockHeldSum->f_fund_holding_share;
						dFundHoldMarketValue = pStockHeldSum->f_fund_holding_market_value;
						dLtShareRate = pStockHeldSum->f_percent_tradable_share;
						//持股市值(元)
						resTableStock.SetCell(3+j*4,iRow,dFundHoldMarketValue);
						//占流通股比例
						resTableStock.SetCell(4+j*4,iRow,dLtShareRate);
						//持股数
						resTableStock.SetCell(5+j*4,iRow,dFundHoldShare);
						//持有该基金数
						resTableStock.SetCell(6+j*4,iRow,iFundholdvolum);
						VisibleFlag++;
					}
					if( j == m_iNullRowCount )
					{
						resTableStock.DeleteRow(iRow);
						resTableStock.Arrange();
						iRow--;
					}
					else
						iSecurityIdRecord.push_back(*iterVector);

				}
				tempNo++;
			}
			//添加进度条
			prw.SetPercent(pid,0.9);
			int iNormalSecurityId = 0;
			std::vector<int>::iterator iter = iSecurityIdRecord.begin();
			if(iter != iSecurityIdRecord.end())
				iNormalSecurityId = *iter;
			m_iFirstSampleId = iNormalSecurityId;
			//从数据源取数据
			bool result = false;
			if(bIsZCG)
			{
				result = GetHeldStockFromTopTen(sourceTable,iNormalSecurityId,iFundSecurityId,vDate);
				if(result == false )
				{
					//添加进度条
					if(pStockHeldSumDataFile != NULL)
					{
						delete pStockHeldSumDataFile;
						pStockHeldSumDataFile = NULL;
					}
					prw.SetPercent(pid,1.0);
					return false;
				}
			}
			else
			{
				result = GetHeldStockHeldDetail(sourceTable,iNormalSecurityId,iFundSecurityId,vDate);
				if(result == false )
				{
					//添加进度条
					if(pStockHeldSumDataFile != NULL)
					{
						delete pStockHeldSumDataFile;
						pStockHeldSumDataFile = NULL;
					}
					prw.SetPercent(pid,1.0);
					return false;
				}
			}
			resTableFund.Clear();
			resTableFund.Clone(sourceTable);
			resTableFund.DeleteCol(3);
			resTableFund.InsertCol(3,dtype_val_string);
			int m_nRowCount = 0;
			m_nRowCount = sourceTable.GetRowCount();
			for(int i = 0;i<m_nRowCount;i++)
			{
				int m_date = 0;
				CString m_strDate = _T("");
				CString m_strYear = _T("");
				sourceTable.GetCell(3,i,m_date);
				switch(m_date%10000)
				{
				case 331:
					{
						m_strYear.Format("%d",m_date/10000);
						m_strDate = m_strYear +_T("年第一季报");
					}
					break;
				case 630:
					{
						m_strYear.Format("%d",m_date/10000);
						m_strDate = m_strYear +_T("年第二季报");
					}
					break;
				case 930:
					{
						m_strYear.Format("%d",m_date/10000);
						m_strDate = m_strYear +_T("年第三季报");
					}
					break;
				case 1231:
					{
						m_strYear.Format("%d",m_date/10000);
						m_strDate = m_strYear +_T("年第四季报");
					}
					break;
				default:
					break;
				}
				resTableFund.SetCell(3,i,m_strDate);
			}
			/*resTableFund.Clear();
			resTableFund.Clone(sourceTable);*/
			//添加进度条
			prw.SetPercent(pid,1.0);
			if(pStockHeldSumDataFile != NULL)
			{
				delete pStockHeldSumDataFile;
				pStockHeldSumDataFile = NULL;
			}
			return true;		
		}
		//added by zhangxs
		//股票持有情况统计--用于输出所有明细
		bool TxFund::StatFundStockHolding(
			Tx::Core::Table_Indicator& sourceTable,	//为双击后保存数据。
			Tx::Core::Table_Indicator& resTableStock,	//股票结果数据表
			Tx::Core::Table_Indicator& resTableFund,	//基金结果数据表
			std::vector<int>& iFundSecurityId,		//基金交易实体ID
			std::vector<int>& iStockSecurityId,		//股票交易实体ID
			std::vector<int> vDate,				//报告期
			bool bIsZCG,							//数据源为重仓股；否则为持股明细
			//std::set<int> & TradeId,			//保存所取得sourceTable里的数据的全部的交易实体ID
			std::vector<CString> &vColName,
			std::vector<CString> &vColHeaderName
			)
		{
			ProgressWnd prw;
			UINT pid=prw.AddItem(1,_T("股票持有情况统计..."),0.0);
			prw.Show(1000);
			//#ifdef _DEBUG
			//	strTable=sourceTable.TableToString();
			//	Tx::Core::Commonality::String().StringToClipboard(strTable);
			//#endif
			//添加列的类型
			resTableStock.AddCol(Tx::Core::dtype_int4);//0交易实体ID
			resTableStock.AddCol(Tx::Core::dtype_val_string);//1股票名称
			resTableStock.AddCol(Tx::Core::dtype_val_string);//2股票代码
			resTableStock.AddCol(Tx::Core::dtype_int4);//3上市日期
			resTableStock.AddCol(Tx::Core::dtype_double);//4总股本
			resTableStock.AddCol(Tx::Core::dtype_double);//5流通股本
			resTableStock.AddCol(Tx::Core::dtype_val_string);//6证监会行业
			resTableStock.AddCol(Tx::Core::dtype_val_string);//7天相行业

			//填充标题
			vColName.clear();
			vColName.push_back(_T("股票名称"));
			vColName.push_back(_T("股票代码"));	
			int isize = (int)vDate.size();
			std::set<int,greater<int> > ErasePosSet;
			std::set<int,greater<int> >::iterator EraseIter;
			int tempPosRow = -1;
			for(int k = 0;k < isize;k++)
			{		
				//填充标题
				CString stryear,strdate;
				int tempdate,iyear;
				tempdate = vDate[k];
				if(bIsZCG)
				{
					int itemp;
					itemp = tempdate%10000;
					iyear = tempdate/10000;
					stryear.Format(_T("%d"),iyear);
					switch(itemp)
					{
					case 331:
						strdate = stryear + _T("年") + _T("一季报");
						break;
					case 630:
						strdate = stryear + _T("年") + _T("二季报");
						break;
					case 930:
						strdate = stryear + _T("年") + _T("三季报");
						break;
					case 1231:
						strdate = stryear + _T("年") + _T("四季报");
						break;
					}
				}
				//暂时先把他们的报告期写为二、四季报。等需要时，
				//可以直接把他们改称中报和年报，无须修改程序。
				else
				{
					iyear = tempdate/10000;
					stryear.Format(_T("%d"),iyear);
					if(tempdate%10000 == 630)
						strdate = stryear + _T("年") + _T("二季报");
					if(tempdate%10000 == 1231)
						strdate = stryear + _T("年") + _T("四季报");
				}
				vColHeaderName.push_back(strdate);
				vColName.push_back(_T("持有市值(元)"));
				vColName.push_back(_T("占流通A股比例%"));
				vColName.push_back(_T("持股数(股)"));
				vColName.push_back(_T("持有该股基金数"));
				//添加相应的列
				tempPosRow++;
				resTableStock.InsertCol(3+tempPosRow*4,Tx::Core::dtype_double);
				resTableStock.InsertCol(4+tempPosRow*4,Tx::Core::dtype_double);
				resTableStock.InsertCol(5+tempPosRow*4,Tx::Core::dtype_double);
				resTableStock.InsertCol(6+tempPosRow*4,Tx::Core::dtype_int4);//基金支数
			}
			vColName.push_back(_T("上市日期"));
			vColName.push_back(_T("A股总股本"));
			vColName.push_back(_T("流通股本"));
			vColName.push_back(_T("证监会行业"));
			vColName.push_back(_T("天相行业"));
			//把需要删掉的日期删掉
			std::vector<int>::iterator iterV;
			for(EraseIter = ErasePosSet.begin();EraseIter != ErasePosSet.end();++EraseIter)
			{
				iterV = find(vDate.begin(),vDate.end(),*EraseIter);
				if(iterV != vDate.end())
					vDate.erase(iterV);
			}
			isize = (int)vDate.size();
			//填充resTableFund和resTableStock的数据
			resTableFund.CopyColumnInfoFrom(sourceTable);
			//首先把reaTable里的股票全部取出来
			int iRowCount = sourceTable.GetRowCount();
			CString strData;
			double dData;
			std::vector<int>::iterator iterVector;
			int flag = -1;
			int iData;
			int iRow = -1;
			std::vector<int> iSecurityIdRecord;
			iSecurityIdRecord.clear();
			Tx::Data::StockHeldSum* pStockHeldSum = NULL;
			DataFileNormal<blk_TxExFile_FileHead,Tx::Data::StockHeldSum>* pStockHeldSumDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::StockHeldSum>;
			pStockHeldSumDataFile->SetCheckLoadById(true);
			int iDirId = 30122;
			int iDirIdDetail = 30119;
			int m_iTotal = (int)iStockSecurityId.size();
			int tempNo = 1;
			double m_dPid = 0.0;
			for(iterVector = iStockSecurityId.begin();iterVector != iStockSecurityId.end();++iterVector)
			{
				m_dPid = (double)tempNo/(double)m_iTotal*0.9; 
				prw.SetPercent(pid,m_dPid);
				//填充resTableStock			
				SecurityQuotation* pSecurity = GetSecurityNow(*iterVector);
				if( pSecurity == NULL )
				{
					tempNo++;
					continue;
				}
				resTableStock.AddRow();
				iRow++;
				//取得股票交易实体
				resTableStock.SetCell(0,iRow,*iterVector);
				//股票名称
				strData = pSecurity->GetName();		
				resTableStock.SetCell(1,iRow,strData);
				//股票代码
				strData = pSecurity->GetCode();
				resTableStock.SetCell(2,iRow,strData);
				//上市日期
				iData = pSecurity->GetIPOListedDate();
				resTableStock.SetCell(3+isize*4,iRow,iData);
				//总股本
				dData = pSecurity->GetTotalShare();
				if(dData <= 0)
					dData = Tx::Core::Con_doubleInvalid;
				resTableStock.SetCell(4+isize*4,iRow,dData);
				//流通股本
				double capitalization = pSecurity->GetTradableShare();
				if(capitalization <= 0)
					capitalization = Tx::Core::Con_doubleInvalid;
				resTableStock.SetCell(5+isize*4,iRow,capitalization);
				//证监会行业
				strData = pSecurity->GetCSRCIndustryName();		
				resTableStock.SetCell(6+isize*4,iRow,strData);
				//天相行业
				strData = pSecurity->GetTxSecIndustryName();
				resTableStock.SetCell(7+isize*4,iRow,strData);
				//用于记录该股票是否被显示
				int VisibleFlag = 0;
				//modified by zhangxs 
				int j = 0;
				int m_iNullRowCount = 0;
				if(bIsZCG)
				{
					for(std::vector<int>::iterator iterat = vDate.begin();iterat != vDate.end();iterat++,j++)
					{
						if(pStockHeldSumDataFile->Load(
							*iterat,//文件名=2020202.dat
							iDirId,//文件所在目录
							true)==false)// || pStockHeldSum->GetDataCount()<=1)
						{
							m_iNullRowCount++;
							continue;
						}
						int iFundholdvolum = Con_intInvalid;
						double dFundHoldShare = Con_doubleInvalid;
						double dLtShareRate = Con_doubleInvalid;
						double dFundHoldMarketValue = Con_doubleInvalid;
						pStockHeldSum = NULL;
						pStockHeldSum = pStockHeldSumDataFile->GetDataByObj(*iterVector,false);
						if(pStockHeldSum == NULL)
						{
							m_iNullRowCount++;
							continue;
						}
						int m_iSecurityId = pStockHeldSum->f_trans_obj_id;
						iFundholdvolum = pStockHeldSum->f_fund_holding_volume;
						dFundHoldShare = pStockHeldSum->f_fund_holding_share;
						dFundHoldMarketValue = pStockHeldSum->f_fund_holding_market_value;
						dLtShareRate = pStockHeldSum->f_percent_tradable_share;
						//持股市值(元)
						resTableStock.SetCell(3+j*4,iRow,dFundHoldMarketValue);
						//占流通股比例
						resTableStock.SetCell(4+j*4,iRow,dLtShareRate);
						//持股数
						resTableStock.SetCell(5+j*4,iRow,dFundHoldShare);
						//持有该基金数
						resTableStock.SetCell(6+j*4,iRow,iFundholdvolum);
						VisibleFlag++;
					}
					if( j == m_iNullRowCount )
					{
						resTableStock.DeleteRow(iRow);
						resTableStock.Arrange();
						iRow--;
					}
					else
						iSecurityIdRecord.push_back(*iterVector);

				}
				else
				{
					for(std::vector<int>::iterator iterat = vDate.begin();iterat != vDate.end();iterat++,j++)
					{
						if(pStockHeldSumDataFile->Load(
							*iterat,//文件名=2020202.dat
							iDirIdDetail,//文件所在目录
							true)==false)// || pStockHeldSum->GetDataCount()<=1)
						{
							m_iNullRowCount++;
							continue;
						}
						int iFundholdvolum = Con_intInvalid;
						double dFundHoldShare = Con_doubleInvalid;
						double dLtShareRate = Con_doubleInvalid;
						double dFundHoldMarketValue = Con_doubleInvalid;
						pStockHeldSum = NULL;
						pStockHeldSum = pStockHeldSumDataFile->GetDataByObj(*iterVector,false);
						if(pStockHeldSum == NULL)
						{
							m_iNullRowCount++;
							continue;
						}
						int m_iSecurityId = pStockHeldSum->f_trans_obj_id;
						iFundholdvolum = pStockHeldSum->f_fund_holding_volume;
						dFundHoldShare = pStockHeldSum->f_fund_holding_share;
						dFundHoldMarketValue = pStockHeldSum->f_fund_holding_market_value;
						dLtShareRate = pStockHeldSum->f_percent_tradable_share;
						//持股市值(元)
						resTableStock.SetCell(3+j*4,iRow,dFundHoldMarketValue);
						//占流通股比例
						resTableStock.SetCell(4+j*4,iRow,dLtShareRate);
						//持股数
						resTableStock.SetCell(5+j*4,iRow,dFundHoldShare);
						//持有该基金数
						resTableStock.SetCell(6+j*4,iRow,iFundholdvolum);
						VisibleFlag++;
					}
					if( j == m_iNullRowCount )
					{
						resTableStock.DeleteRow(iRow);
						resTableStock.Arrange();
						iRow--;
					}
					else
						iSecurityIdRecord.push_back(*iterVector);

				}
				tempNo++;
			}
			//添加进度条
			prw.SetPercent(pid,0.9);
			int iNormalSecurityId = 0;
			//从数据源取数据
			bool result = false;
			if(bIsZCG)
			{
				result = GetHeldStockFromTopTen(sourceTable,iSecurityIdRecord,iFundSecurityId,vDate);
				if(result == false )
				{
					//添加进度条
					if(pStockHeldSumDataFile != NULL)
					{
						delete pStockHeldSumDataFile;
						pStockHeldSumDataFile = NULL;
					}
					prw.SetPercent(pid,1.0);
					return false;
				}
			}
			else
			{
				result = GetHeldStockHeldDetail(sourceTable,iSecurityIdRecord,iFundSecurityId,vDate);
				if(result == false )
				{
					//添加进度条
					if(pStockHeldSumDataFile != NULL)
					{
						delete pStockHeldSumDataFile;
						pStockHeldSumDataFile = NULL;
					}
					prw.SetPercent(pid,1.0);
					return false;
				}
			}
			resTableFund.Clear();
			resTableFund.Clone(sourceTable);
#ifdef _DEBUG
			CString strTable=resTableFund.TableToString();
			Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
			resTableFund.DeleteCol(5);
			resTableFund.InsertCol(5,dtype_val_string);
			int m_nRowCount = 0;
			m_nRowCount = sourceTable.GetRowCount();
			for(int i = 0;i<m_nRowCount;i++)
			{
				int m_date = 0;
				CString m_strDate = _T("");
				CString m_strYear = _T("");
				sourceTable.GetCell(5,i,m_date);
				switch(m_date%10000)
				{
				case 331:
					{
						m_strYear.Format("%d",m_date/10000);
						m_strDate = m_strYear +_T("年第一季报");
					}
					break;
				case 630:
					{
						m_strYear.Format("%d",m_date/10000);
						m_strDate = m_strYear +_T("年第二季报");
					}
					break;
				case 930:
					{
						m_strYear.Format("%d",m_date/10000);
						m_strDate = m_strYear +_T("年第三季报");
					}
					break;
				case 1231:
					{
						m_strYear.Format("%d",m_date/10000);
						m_strDate = m_strYear +_T("年第四季报");
					}
					break;
				default:
					break;
				}
				resTableFund.SetCell(5,i,m_strDate);
			}
			/*resTableFund.Clear();
			resTableFund.Clone(sourceTable);*/
			//添加进度条
			prw.SetPercent(pid,1.0);
			if(pStockHeldSumDataFile != NULL)
			{
				delete pStockHeldSumDataFile;
				pStockHeldSumDataFile = NULL;
			}
			return true;		
		}
		//added by zhangxs 
		//取得股票持有情况数据源为持股明细
		bool TxFund::GetHeldStockHeldDetail(
			Tx::Core::Table_Indicator &resTable,
			int	iSecurityId,
			std::vector<int> ViFundSecurityId,
			std::vector<int>    ViDate)
		{
			ProgressWnd prw;
			UINT pid=prw.AddItem(1,_T("股票持有情况统计..."),0.0);
			prw.Show(1000);
			if(ViDate.size()<1)
				return false;
			if(ViFundSecurityId.size()<1)
				return false;
			Tx::Data::StockHeldDetail* pStockHeldDetail = NULL;
			DataFileNormal<blk_TxExFile_FileHead,StockHeldDetail>* pStockHeldDetailDataFile = new DataFileNormal<blk_TxExFile_FileHead,StockHeldDetail>;
			pStockHeldDetailDataFile->SetCheckLoadById(true);
			if(pStockHeldDetailDataFile==NULL )
			{
				prw.SetPercent(pid,1.0);
				return false;
			}
			//int iDirId = 30123;
			int iDirId = 30120;

			//默认的返回值状态
			bool result = false;
			//清空数据
			resTable.Clear();
			int iCol = 0;

			//准备样本集=Security_ID,int型
			resTable.AddCol(Tx::Core::dtype_int4);
			iCol++;
			//样本名称
			resTable.AddCol(Tx::Core::dtype_val_string);
			//resTable.SetTitle(iCol,_T("名称"));
			iCol++;
			//样本外码
			resTable.AddCol(Tx::Core::dtype_val_string);
			//resTable.SetTitle(iCol,_T("代码"));
			iCol++;
			//报告期
			resTable.AddCol(Tx::Core::dtype_int4);
			//resTable.SetTitle(iCol,_T("报告期"));
			iCol++;

			//持股市值(元)
			resTable.AddCol(Tx::Core::dtype_double);
			//resTable.SetTitle(iCol,_T("持股市值(元)"));
			//	resTable.SetPrecise(iCol,2);
			//resTable.SetFormatStyle(iCol,fs_finance);
			iCol++;

			//持股数
			resTable.AddCol(Tx::Core::dtype_int4);
			//resTable.SetTitle(iCol,_T("持股数(股)"));
			iCol++;

			//占流通股比例%
			resTable.AddCol(Tx::Core::dtype_double);
			//resTable.SetTitle(iCol,_T("占流通股比例%"));
			//resTable.SetPrecise(iCol,2);
			iCol++;

			//占基金净值比例(%)
			resTable.AddCol(Tx::Core::dtype_double);
			//resTable.SetTitle(iCol,_T("占基金净值比例(%)"));
			//resTable.SetPrecise(iCol,2);
			iCol++;
			int iRow = 0;
			// 可以添加其他列,存放中间结果,供弹出窗口使用
			if(pStockHeldDetailDataFile->Load(
				iSecurityId,//文件名=2020202.dat
				iDirId,//文件所在目录
				true)==false)// || pStockHeldSum->GetDataCount()<=1)
			{
				if( pStockHeldDetailDataFile != NULL)
				{
					delete pStockHeldDetailDataFile;
					pStockHeldDetail = NULL;
				}
				prw.SetPercent(pid,1.0);
				return false;
			}
			prw.SetPercent(pid,0.6);
			for(std::vector<int>::iterator iter = ViDate.begin();iter != ViDate.end();iter++)
			{
				int m_date =*iter;
				CString m_strReportDate = _T("");
				CString m_strYear = _T("");
				m_strYear.Format(_T("%d"),m_date/10000);
				if(m_date%10000 > 600 && m_date%10000 < 710)
					m_strReportDate = m_strYear + _T("年") + _T("二季报");
				if(m_date%10000 > 1200 )
					m_strReportDate = m_strYear + _T("年") + _T("四季报");
				pStockHeldDetail = NULL;
				pStockHeldDetail = pStockHeldDetailDataFile->GetDataByObj(m_date,false);
				if(pStockHeldDetail == NULL)
				{
					continue;
				}
				std::set<int> m_iSecurity1Id;//券id
				m_iSecurity1Id.clear();
				while( pStockHeldDetail->f_end_date == m_date )
				{
					int nAccountingID = pStockHeldDetail->f_fund_trans_obj_id;
					double iHeldNetValue = Tx::Core::Con_doubleInvalid;			// 持股市值
					int dShare = Tx::Core::Con_intInvalid;						// 持股数
					double dLtShareRate = Tx::Core::Con_doubleInvalid;			// 占流通股比例%
					double dNetValueRate = Tx::Core::Con_doubleInvalid;			// 占基金净值比例%
					SecurityQuotation* p = GetSecurityNow(pStockHeldDetail->f_fund_trans_obj_id);
					if (p == NULL)
					{
						pStockHeldDetail++;
						continue;
					}
					if(find(ViFundSecurityId.begin(),ViFundSecurityId.end(),pStockHeldDetail->f_fund_trans_obj_id)==ViFundSecurityId.end())
					{
						pStockHeldDetail++;
						continue;
					}
					int m_iSecurity1IdSingle = p->GetSecurity1Id(nAccountingID);
					if(m_iSecurity1Id.find(m_iSecurity1IdSingle) == m_iSecurity1Id.end())
						m_iSecurity1Id.insert(m_iSecurity1IdSingle);
					else
					{
						pStockHeldDetail++;
						continue;
					}
					iHeldNetValue = pStockHeldDetail->f4;
					dShare = (int)pStockHeldDetail->F3;
					dLtShareRate = pStockHeldDetail ->F_PERCENT_TRADABLE_SHARE;
					dNetValueRate = pStockHeldDetail ->F6;

					resTable.AddRow();
					int nCol = 0;
					//交易实体id
					resTable.SetCell(nCol,iRow,nAccountingID);
					nCol++;
					//填充名称
					resTable.SetCell(nCol,iRow,p->GetName());
					nCol++;
					//填充外码
					resTable.SetCell(nCol,iRow,p->GetCode());
					nCol++;
					//报告期
					resTable.SetCell(nCol,iRow,m_date);
					nCol++;
					//持股市值(元)
					resTable.SetCell(nCol,iRow,iHeldNetValue);
					nCol++;
					//持股数
					resTable.SetCell(nCol,iRow,dShare);
					nCol++;
					//占流通股比例
					resTable.SetCell(nCol,iRow,dLtShareRate);
					nCol++;
					//占基金净值比例
					resTable.SetCell(nCol,iRow,dNetValueRate);
					nCol++;
					iRow++;
					pStockHeldDetail++;
				}
			}
			if(resTable.GetRowCount() < 1)
			{
				if( pStockHeldDetailDataFile != NULL)
				{
					delete pStockHeldDetailDataFile;
					pStockHeldDetail = NULL;
				}
				prw.SetPercent(pid,1.0);
				return false;
			}
			//多列排序
			Tx::Core::MultiSortRule m_MultSort;
			m_MultSort.AddRule(3,false);
			m_MultSort.AddRule(4,false);
			resTable.SortInMultiCol(m_MultSort);
			resTable.Arrange();
			if( pStockHeldDetailDataFile != NULL)
			{
				delete pStockHeldDetailDataFile;
				pStockHeldDetail = NULL;
			}
			/******************************************************************************/
			prw.SetPercent(pid,1.0);
			return true;

		}

		//added by zhangxs 
		//取得股票持有情况数据源为重仓股
		bool TxFund::GetHeldStockFromTopTen(
			Tx::Core::Table_Indicator &resTable,
			int	iSecurityId,
			std::vector<int> ViFundSecurityId,
			std::vector<int>   ViDate)
		{
			ProgressWnd prw;
			UINT pid=prw.AddItem(1,_T("股票持有情况统计..."),0.0);
			prw.Show(1000);

			if(ViDate.size()<1)
				return false;
			if(ViFundSecurityId.size()<1)
				return false;
			Tx::Data::StockHeldDetail* pStockHeldDetail = NULL;
			DataFileNormal<blk_TxExFile_FileHead,StockHeldDetail>* pStockHeldDetailDataFile = new DataFileNormal<blk_TxExFile_FileHead,StockHeldDetail>;
			pStockHeldDetailDataFile->SetCheckLoadById(true);
			if(pStockHeldDetailDataFile==NULL )
			{
				prw.SetPercent(pid,1.0);
				return false;
			}
			int iDirId = 30123;

			//默认的返回值状态
			bool result = false;
			//清空数据
			resTable.Clear();
			int iCol = 0;

			//准备样本集=Security_ID,int型
			resTable.AddCol(Tx::Core::dtype_int4);
			iCol++;
			//样本名称
			resTable.AddCol(Tx::Core::dtype_val_string);
			//resTable.SetTitle(iCol,_T("名称"));
			iCol++;
			//样本外码
			resTable.AddCol(Tx::Core::dtype_val_string);
			//resTable.SetTitle(iCol,_T("代码"));
			iCol++;
			//报告期
			resTable.AddCol(Tx::Core::dtype_int4);
			//resTable.SetTitle(iCol,_T("报告期"));
			iCol++;

			//持股市值(元)
			resTable.AddCol(Tx::Core::dtype_double);
			//resTable.SetTitle(iCol,_T("持股市值(元)"));
			//	resTable.SetPrecise(iCol,2);
			//resTable.SetFormatStyle(iCol,fs_finance);
			iCol++;

			//持股数
			resTable.AddCol(Tx::Core::dtype_int4);
			//resTable.SetTitle(iCol,_T("持股数(股)"));
			iCol++;

			//占流通股比例%
			resTable.AddCol(Tx::Core::dtype_double);
			//resTable.SetTitle(iCol,_T("占流通股比例%"));
			//resTable.SetPrecise(iCol,2);
			iCol++;

			//占基金净值比例(%)
			resTable.AddCol(Tx::Core::dtype_double);
			//resTable.SetTitle(iCol,_T("占基金净值比例(%)"));
			//resTable.SetPrecise(iCol,2);
			iCol++;
			int iRow = 0;
			// 可以添加其他列,存放中间结果,供弹出窗口使用
			if(pStockHeldDetailDataFile->Load(
				iSecurityId,//文件名=2020202.dat
				iDirId,//文件所在目录
				true)==false)// || pStockHeldSum->GetDataCount()<=1)
			{
				if( pStockHeldDetailDataFile != NULL)
				{
					delete pStockHeldDetailDataFile;
					pStockHeldDetail = NULL;
				}
				prw.SetPercent(pid,1.0);
				return false;
			}
			//添加进度条
			prw.SetPercent(pid,0.6);
			for(std::vector<int>::iterator iter = ViDate.begin();iter != ViDate.end();iter++)
			{
				int m_date = *iter;
				CString m_strReportDate = _T("");
				CString m_strYear = _T("");
				m_strYear.Format(_T("%d"),m_date/10000);
				switch(m_date%10000)
				{
				case 331:
					m_strReportDate = m_strYear + _T("年一季报");
					break;
				case 630:
					m_strReportDate = m_strYear + _T("年二季报");
					break;
				case 930:
					m_strReportDate = m_strYear + _T("年三季报");
					break;
				case 1231:
					m_strReportDate = m_strYear + _T("年四季报");
					break;
				}
				pStockHeldDetail = NULL;
				pStockHeldDetail = pStockHeldDetailDataFile->GetDataByObj(m_date,false);
				if(pStockHeldDetail == NULL)
					continue;
				int nAccountingID = 0;
				std::set<int> m_iSecurity1Id;
				m_iSecurity1Id.clear();
				while( pStockHeldDetail->f_end_date ==m_date )
				{
					double iHeldNetValue = Tx::Core::Con_doubleInvalid;			// 持股市值
					int dShare = Tx::Core::Con_intInvalid;						// 持股数
					double dLtShareRate = Tx::Core::Con_doubleInvalid;			// 占流通股比例%
					double dNetValueRate = Tx::Core::Con_doubleInvalid;			// 占基金净值比例%
					nAccountingID = pStockHeldDetail->f_fund_trans_obj_id;
					SecurityQuotation* p = GetSecurityNow(nAccountingID);
					if (p == NULL)
					{
						pStockHeldDetail++;
						continue;
					}
					if(find(ViFundSecurityId.begin(),ViFundSecurityId.end(),pStockHeldDetail->f_fund_trans_obj_id)==ViFundSecurityId.end())
					{
						pStockHeldDetail++;
						continue;
					}
					int m_iSecurity1IdSingle = p->GetSecurity1Id(nAccountingID);
					if(m_iSecurity1Id.find(m_iSecurity1IdSingle) == m_iSecurity1Id.end())
						m_iSecurity1Id.insert(m_iSecurity1IdSingle);
					else
					{
						pStockHeldDetail++;
						continue;
					}
					iHeldNetValue = pStockHeldDetail->f4;
					dShare = (int)pStockHeldDetail->F3;
					dLtShareRate = pStockHeldDetail ->F_PERCENT_TRADABLE_SHARE;
					dNetValueRate = pStockHeldDetail ->F6;

					resTable.AddRow();
					int nCol = 0;
					//交易实体id
					resTable.SetCell(nCol,iRow,nAccountingID);
					nCol++;
					//填充名称
					resTable.SetCell(nCol,iRow,p->GetName());
					nCol++;
					//填充外码
					resTable.SetCell(nCol,iRow,p->GetCode());
					nCol++;
					//报告期
					resTable.SetCell(nCol,iRow,m_date);
					nCol++;
					//持股市值(元)
					resTable.SetCell(nCol,iRow,iHeldNetValue);
					nCol++;
					//持股数
					resTable.SetCell(nCol,iRow,dShare);
					nCol++;
					//占流通股比例
					resTable.SetCell(nCol,iRow,dLtShareRate);
					nCol++;
					//占基金净值比例
					resTable.SetCell(nCol,iRow,dNetValueRate);
					nCol++;
					iRow++;
					pStockHeldDetail++;
				}
			}
			if(resTable.GetRowCount() < 1)
			{
				if( pStockHeldDetailDataFile != NULL)
				{
					delete pStockHeldDetailDataFile;
					pStockHeldDetail = NULL;
				}
				prw.SetPercent(pid,1.0);
				return false;
			}
			//多列排序
			Tx::Core::MultiSortRule m_MultSort;
			m_MultSort.AddRule(3,false);
			m_MultSort.AddRule(4,false);
			resTable.SortInMultiCol(m_MultSort);
			resTable.Arrange();
			if( pStockHeldDetailDataFile != NULL)
				delete pStockHeldDetailDataFile;
			pStockHeldDetail = NULL;
			/******************************************************************************/
			prw.SetPercent(pid,1.0);
			return true;

		}
		//用于输出所有明细
		bool TxFund::GetHeldStockHeldDetail(
			Tx::Core::Table_Indicator &resTable,
			std::vector<int> TradeId,
			std::vector<int> ViFundSecurityId,
			std::vector<int>    ViDate)
		{
			ProgressWnd prw;
			UINT pid=prw.AddItem(1,_T("股票持有情况统计..."),0.0);
			prw.Show(1000);
			if(ViDate.size()<1)
				return false;
			if(ViFundSecurityId.size()<1)
				return false;
			Tx::Data::StockHeldDetail* pStockHeldDetail = NULL;
			DataFileNormal<blk_TxExFile_FileHead,StockHeldDetail>* pStockHeldDetailDataFile = new DataFileNormal<blk_TxExFile_FileHead,StockHeldDetail>;
			pStockHeldDetailDataFile->SetCheckLoadById(true);
			if(pStockHeldDetailDataFile==NULL )
			{
				prw.SetPercent(pid,1.0);
				return false;
			}
			//int iDirId = 30123;
			int iDirId = 30120;

			//默认的返回值状态
			bool result = false;
			//清空数据
			resTable.Clear();
			int iCol = 0;

			//准备样本集=Security_ID,int型
			resTable.AddCol(Tx::Core::dtype_int4);
			iCol++;
			//股票名称
			resTable.AddCol(Tx::Core::dtype_val_string);
			iCol++;
			//股票外码
			resTable.AddCol(Tx::Core::dtype_val_string);
			iCol++;
			//样本名称
			resTable.AddCol(Tx::Core::dtype_val_string);
			//resTable.SetTitle(iCol,_T("名称"));
			iCol++;
			//样本外码
			resTable.AddCol(Tx::Core::dtype_val_string);
			//resTable.SetTitle(iCol,_T("代码"));
			iCol++;
			//报告期
			resTable.AddCol(Tx::Core::dtype_int4);
			//resTable.SetTitle(iCol,_T("报告期"));
			iCol++;

			//持股市值(元)
			resTable.AddCol(Tx::Core::dtype_double);
			//resTable.SetTitle(iCol,_T("持股市值(元)"));
			//	resTable.SetPrecise(iCol,2);
			//resTable.SetFormatStyle(iCol,fs_finance);
			iCol++;

			//持股数
			resTable.AddCol(Tx::Core::dtype_int4);
			//resTable.SetTitle(iCol,_T("持股数(股)"));
			iCol++;

			//占流通股比例%
			resTable.AddCol(Tx::Core::dtype_double);
			//resTable.SetTitle(iCol,_T("占流通股比例%"));
			//resTable.SetPrecise(iCol,2);
			iCol++;

			//占基金净值比例(%)
			resTable.AddCol(Tx::Core::dtype_double);
			//resTable.SetTitle(iCol,_T("占基金净值比例(%)"));
			//resTable.SetPrecise(iCol,2);
			iCol++;
			int iRow = 0;
			// 可以添加其他列,存放中间结果,供弹出窗口使用
			std::vector<int>::iterator m_itre = TradeId.begin();
			for(;m_itre != TradeId.end();m_itre++)
			{
				int iStockId = *m_itre;
				SecurityQuotation* pstock = GetSecurityNow(iStockId);
				if (pstock == NULL)
					continue;
				if(pStockHeldDetailDataFile->Load(
					*m_itre,//文件名=2020202.dat
					iDirId,//文件所在目录
					true)==false)// || pStockHeldSum->GetDataCount()<=1)
				{
					continue;
				}
				prw.SetPercent(pid,0.6);
				for(std::vector<int>::iterator iter = ViDate.begin();iter != ViDate.end();iter++)
				{
					int m_date =*iter;
					CString m_strReportDate = _T("");
					CString m_strYear = _T("");
					m_strYear.Format(_T("%d"),m_date/10000);
					if(m_date%10000 > 600 && m_date%10000 < 710)
						m_strReportDate = m_strYear + _T("年") + _T("二季报");
					if(m_date%10000 > 1200 )
						m_strReportDate = m_strYear + _T("年") + _T("四季报");
					pStockHeldDetail = NULL;
					pStockHeldDetail = pStockHeldDetailDataFile->GetDataByObj(m_date,false);
					if(pStockHeldDetail == NULL)
					{
						continue;
					}
					std::set<int> m_iSecurity1Id;//券id
					m_iSecurity1Id.clear();
					while( pStockHeldDetail->f_end_date == m_date )
					{
						int nAccountingID = pStockHeldDetail->f_fund_trans_obj_id;
						double iHeldNetValue = Tx::Core::Con_doubleInvalid;			// 持股市值
						int dShare = Tx::Core::Con_intInvalid;						// 持股数
						double dLtShareRate = Tx::Core::Con_doubleInvalid;			// 占流通股比例%
						double dNetValueRate = Tx::Core::Con_doubleInvalid;			// 占基金净值比例%
						SecurityQuotation* p = GetSecurityNow(pStockHeldDetail->f_fund_trans_obj_id);
						if (p == NULL)
						{
							pStockHeldDetail++;
							continue;
						}
						if(find(ViFundSecurityId.begin(),ViFundSecurityId.end(),pStockHeldDetail->f_fund_trans_obj_id)==ViFundSecurityId.end())
						{
							pStockHeldDetail++;
							continue;
						}
						int m_iSecurity1IdSingle = p->GetSecurity1Id(nAccountingID);
						if(m_iSecurity1Id.find(m_iSecurity1IdSingle) == m_iSecurity1Id.end())
							m_iSecurity1Id.insert(m_iSecurity1IdSingle);
						else
						{
							pStockHeldDetail++;
							continue;
						}
						iHeldNetValue = pStockHeldDetail->f4;
						dShare = (int)pStockHeldDetail->F3;
						dLtShareRate = pStockHeldDetail ->F_PERCENT_TRADABLE_SHARE;
						dNetValueRate = pStockHeldDetail ->F6;

						resTable.AddRow();
						int nCol = 0;
						//交易实体id
						resTable.SetCell(nCol,iRow,nAccountingID);
						nCol++;
						//股票名称
						resTable.SetCell(nCol,iRow,pstock->GetName());
						nCol++;
						//股票外码
						resTable.SetCell(nCol,iRow,pstock->GetCode());
						nCol++;
						//填充名称
						resTable.SetCell(nCol,iRow,p->GetName());
						nCol++;
						//填充外码
						resTable.SetCell(nCol,iRow,p->GetCode());
						nCol++;
						//报告期
						resTable.SetCell(nCol,iRow,m_date);
						nCol++;
						//持股市值(元)
						resTable.SetCell(nCol,iRow,iHeldNetValue);
						nCol++;
						//持股数
						resTable.SetCell(nCol,iRow,dShare);
						nCol++;
						//占流通股比例
						resTable.SetCell(nCol,iRow,dLtShareRate);
						nCol++;
						//占基金净值比例
						resTable.SetCell(nCol,iRow,dNetValueRate);
						nCol++;
						iRow++;
						pStockHeldDetail++;
					}
				}		
			}
			if(resTable.GetRowCount() < 1)
			{
				if( pStockHeldDetailDataFile != NULL)
				{
					delete pStockHeldDetailDataFile;
					pStockHeldDetail = NULL;
				}
				prw.SetPercent(pid,1.0);
				return false;
			}
			//多列排序
			Tx::Core::MultiSortRule m_MultSort;
			m_MultSort.AddRule(2,false);
			m_MultSort.AddRule(5,false);
			m_MultSort.AddRule(6,false);
			resTable.SortInMultiCol(m_MultSort);
			resTable.Arrange();
			if( pStockHeldDetailDataFile != NULL)
			{
				delete pStockHeldDetailDataFile;
				pStockHeldDetail = NULL;
			}
			/******************************************************************************/
			prw.SetPercent(pid,1.0);
			return true;

		}

		//added by zhangxs 
		//取得股票持有情况数据源为重仓股 -- 用于输出所有明细
		bool TxFund::GetHeldStockFromTopTen(
			Tx::Core::Table_Indicator &resTable,
			std::vector<int> TradeId,
			std::vector<int> ViFundSecurityId,
			std::vector<int>   ViDate)
		{
			ProgressWnd prw;
			UINT pid=prw.AddItem(1,_T("股票持有情况统计..."),0.0);
			prw.Show(1000);

			if(ViDate.size()<1)
				return false;
			if(ViFundSecurityId.size()<1)
				return false;
			Tx::Data::StockHeldDetail* pStockHeldDetail = NULL;
			DataFileNormal<blk_TxExFile_FileHead,StockHeldDetail>* pStockHeldDetailDataFile = new DataFileNormal<blk_TxExFile_FileHead,StockHeldDetail>;
			pStockHeldDetailDataFile->SetCheckLoadById(true);
			if(pStockHeldDetailDataFile==NULL )
			{
				prw.SetPercent(pid,1.0);
				return false;
			}
			int iDirId = 30123;

			//默认的返回值状态
			bool result = false;
			//清空数据
			resTable.Clear();
			int iCol = 0;

			//准备样本集=Security_ID,int型
			resTable.AddCol(Tx::Core::dtype_int4);
			iCol++;
			//股票名称
			resTable.AddCol(Tx::Core::dtype_val_string);
			iCol++;
			//股票外码
			resTable.AddCol(Tx::Core::dtype_val_string);
			iCol++;
			//样本名称
			resTable.AddCol(Tx::Core::dtype_val_string);
			//resTable.SetTitle(iCol,_T("名称"));
			iCol++;
			//样本外码
			resTable.AddCol(Tx::Core::dtype_val_string);
			//resTable.SetTitle(iCol,_T("代码"));
			iCol++;
			//报告期
			resTable.AddCol(Tx::Core::dtype_int4);
			//resTable.SetTitle(iCol,_T("报告期"));
			iCol++;

			//持股市值(元)
			resTable.AddCol(Tx::Core::dtype_double);
			//resTable.SetTitle(iCol,_T("持股市值(元)"));
			//	resTable.SetPrecise(iCol,2);
			//resTable.SetFormatStyle(iCol,fs_finance);
			iCol++;

			//持股数
			resTable.AddCol(Tx::Core::dtype_int4);
			//resTable.SetTitle(iCol,_T("持股数(股)"));
			iCol++;

			//占流通股比例%
			resTable.AddCol(Tx::Core::dtype_double);
			//resTable.SetTitle(iCol,_T("占流通股比例%"));
			//resTable.SetPrecise(iCol,2);
			iCol++;

			//占基金净值比例(%)
			resTable.AddCol(Tx::Core::dtype_double);
			//resTable.SetTitle(iCol,_T("占基金净值比例(%)"));
			//resTable.SetPrecise(iCol,2);
			iCol++;
			int iRow = 0;
			// 可以添加其他列,存放中间结果,供弹出窗口使用
			std::vector<int>::iterator m_iter = TradeId.begin();
			for(;m_iter != TradeId.end();m_iter++)
			{
				int SecId = *m_iter;
				SecurityQuotation* pstock = GetSecurityNow(SecId);
				if(pstock == NULL)
					continue;
				if(pStockHeldDetailDataFile->Load(
					*m_iter,//文件名=2020202.dat
					iDirId,//文件所在目录
					true)==false)// || pStockHeldSum->GetDataCount()<=1)
				{
					continue;
				}
				//添加进度条
				prw.SetPercent(pid,0.6);
				for(std::vector<int>::iterator iter = ViDate.begin();iter != ViDate.end();iter++)
				{
					int m_date = *iter;
					CString m_strReportDate = _T("");
					CString m_strYear = _T("");
					m_strYear.Format(_T("%d"),m_date/10000);
					switch(m_date%10000)
					{
					case 331:
						m_strReportDate = m_strYear + _T("年一季报");
						break;
					case 630:
						m_strReportDate = m_strYear + _T("年二季报");
						break;
					case 930:
						m_strReportDate = m_strYear + _T("年三季报");
						break;
					case 1231:
						m_strReportDate = m_strYear + _T("年四季报");
						break;
					}
					pStockHeldDetail = NULL;
					pStockHeldDetail = pStockHeldDetailDataFile->GetDataByObj(m_date,false);
					if(pStockHeldDetail == NULL)
						continue;
					int nAccountingID = 0;
					std::set<int> m_iSecurity1Id;
					m_iSecurity1Id.clear();
					while( pStockHeldDetail->f_end_date ==m_date )
					{
						double iHeldNetValue = Tx::Core::Con_doubleInvalid;			// 持股市值
						int dShare = Tx::Core::Con_intInvalid;						// 持股数
						double dLtShareRate = Tx::Core::Con_doubleInvalid;			// 占流通股比例%
						double dNetValueRate = Tx::Core::Con_doubleInvalid;			// 占基金净值比例%
						nAccountingID = pStockHeldDetail->f_fund_trans_obj_id;
						SecurityQuotation* p = GetSecurityNow(nAccountingID);
						if (p == NULL)
						{
							pStockHeldDetail++;
							continue;
						}
						if(find(ViFundSecurityId.begin(),ViFundSecurityId.end(),pStockHeldDetail->f_fund_trans_obj_id)==ViFundSecurityId.end())
						{
							pStockHeldDetail++;
							continue;
						}
						int m_iSecurity1IdSingle = p->GetSecurity1Id(nAccountingID);
						if(m_iSecurity1Id.find(m_iSecurity1IdSingle) == m_iSecurity1Id.end())
							m_iSecurity1Id.insert(m_iSecurity1IdSingle);
						else
						{
							pStockHeldDetail++;
							continue;
						}
						iHeldNetValue = pStockHeldDetail->f4;
						dShare = (int)pStockHeldDetail->F3;
						dLtShareRate = pStockHeldDetail ->F_PERCENT_TRADABLE_SHARE;
						dNetValueRate = pStockHeldDetail ->F6;

						resTable.AddRow();
						int nCol = 0;
						//交易实体id
						resTable.SetCell(nCol,iRow,nAccountingID);
						nCol++;
						//股票名称
						resTable.SetCell(nCol,iRow,pstock->GetName());
						nCol++;
						//股票外码
						resTable.SetCell(nCol,iRow,pstock->GetCode());
						nCol++;
						//填充名称
						resTable.SetCell(nCol,iRow,p->GetName());
						nCol++;
						//填充外码
						resTable.SetCell(nCol,iRow,p->GetCode());
						nCol++;
						//报告期
						resTable.SetCell(nCol,iRow,m_date);
						nCol++;
						//持股市值(元)
						resTable.SetCell(nCol,iRow,iHeldNetValue);
						nCol++;
						//持股数
						resTable.SetCell(nCol,iRow,dShare);
						nCol++;
						//占流通股比例
						resTable.SetCell(nCol,iRow,dLtShareRate);
						nCol++;
						//占基金净值比例
						resTable.SetCell(nCol,iRow,dNetValueRate);
						nCol++;
						iRow++;
						pStockHeldDetail++;
					}
				}
			}	
			if(resTable.GetRowCount() < 1)
			{
				if( pStockHeldDetailDataFile != NULL)
				{
					delete pStockHeldDetailDataFile;
					pStockHeldDetail = NULL;
				}
				prw.SetPercent(pid,1.0);
				return false;
			}
			//多列排序
			Tx::Core::MultiSortRule m_MultSort;
			m_MultSort.AddRule(2,false);
			m_MultSort.AddRule(5,false);
			m_MultSort.AddRule(6,false);
			resTable.SortInMultiCol(m_MultSort);
			resTable.Arrange();
			if( pStockHeldDetailDataFile != NULL)
				delete pStockHeldDetailDataFile;
			pStockHeldDetail = NULL;
			/******************************************************************************/
			prw.SetPercent(pid,1.0);
			return true;

		}
		bool TxFund::EliminateSample(std::vector<int> &TradeVS,std::vector<int> &TradeVR,std::vector<int> &FundV)
		{
			if(TradeVS.empty())
				return false;
			std::unordered_map<int,int> FundIdToTradeId;
			std::unordered_map<int,int>::iterator iterMap;
			std::vector<int>::iterator iter;
			int fundid = 0;
			CString strName,strName2;
			//CString strCode1,strCode2;
			for(iter = TradeVS.begin();iter != TradeVS.end();++iter)
			{
				SecurityQuotation * pSecurity2 = GetSecurityNow(*iter);
				if(pSecurity2 == NULL)
					continue;
				fundid =(int)pSecurity2->GetSecurity1Id();
				iterMap = FundIdToTradeId.find(fundid);
				if(iterMap == FundIdToTradeId.end())
				{
					FundIdToTradeId.insert(std::make_pair(fundid,*iter));
				}
				else
				{
					SecurityQuotation * pSecurity = GetSecurityNow(iterMap->second);
					strName = pSecurity->GetName();
					//	strCode1 = pSecurity->GetCode();
					if (strName.Find(_T("A")) != -1 || strName.Find(_T("Ａ")) != -1)
					{
						continue;
					}
					else if(strName.Find(_T("B")) != -1 || strName.Find(_T("Ｂ")) != -1)
					{
						strName2 = pSecurity2->GetName();
						if(strName2.Find(_T("C")) != -1 || strName2.Find(_T("Ｃ")) != -1)
						{
							continue;
						}
						else
						{
							FundIdToTradeId.erase(iterMap);
							FundIdToTradeId.insert(std::make_pair(fundid,*iter));
						}
					}
					else
					{
						//strName2 = pSecurity2->GetName();
						//strCode2 = pSecurity2->GetCode();
						FundIdToTradeId.erase(iterMap);
						FundIdToTradeId.insert(std::make_pair(fundid,*iter));
					}
				}
			}
			for(iterMap = FundIdToTradeId.begin();iterMap != FundIdToTradeId.end();++iterMap)
			{
				FundV.push_back(iterMap->first);
				TradeVR.push_back(iterMap->second);
			}
			if(FundV.empty())
			{
				return false;
			}
			return true;
		}
		bool TxFund::EliminateSample(std::vector<int> &TradeVS,std::vector<int> &TradeVR,std::set<int> &FundSet)
		{
			if(TradeVS.empty())
				return false;
			std::unordered_map<int,int> FundIdToTradeId;
			std::unordered_map<int,int>::iterator iterMap;
			std::vector<int>::iterator iter;
			int fundid = 0;
			CString strName,strName2;
			for(iter = TradeVS.begin();iter != TradeVS.end();++iter)
			{
				GetSecurityNow(*iter);
				if(m_pSecurity == NULL)
					continue;
				fundid =(int)m_pSecurity->GetSecurity1Id();
				iterMap = FundIdToTradeId.find(fundid);
				if(iterMap == FundIdToTradeId.end())
				{
					FundIdToTradeId.insert(std::make_pair(fundid,*iter));
				}
				else
				{
					SecurityQuotation * pSecurity = GetSecurityNow(iterMap->second);
					strName = pSecurity->GetName();
					if (strName.Find(_T("A")) != -1 || strName.Find(_T("Ａ")) != -1)
					{
						continue;
					}
					else if(strName.Find(_T("B")) != -1 || strName.Find(_T("Ｂ")) != -1)
					{
						strName2 = m_pSecurity->GetName();
						if(strName2.Find(_T("C")) != -1 || strName2.Find(_T("Ｃ")) != -1)
						{
							continue;
						}
						else
						{
							FundIdToTradeId.erase(iterMap);
							FundIdToTradeId.insert(std::make_pair(fundid,*iter));
						}
					}
					else
					{
						FundIdToTradeId.erase(iterMap);
						FundIdToTradeId.insert(std::make_pair(fundid,*iter));
					}
				}
			}
			for(iterMap = FundIdToTradeId.begin();iterMap != FundIdToTradeId.end();++iterMap)
			{
				FundSet.insert(iterMap->first);
				TradeVR.push_back(iterMap->second);
			}
			if(FundSet.empty())
			{
				return false;
			}
			return true;
		}
	}
}
