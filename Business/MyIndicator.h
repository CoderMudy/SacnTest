#pragma once
/*************************************************
  Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
  File name:	MyIndicator.h
  Author:       章玮
  Version:		1.0
  Date:			2007-09-04
  
  Description:
				实现带参数指标定义
				原理:
				这部分内容本应在数据或业务层实现,
				目前, 为实现数据输出, 由章玮在这里实现, 
				其中数据用的是 Tx::Data::IndicatorData 的指针
				根据实现应用进行了业务与数据存贮矛盾, 进行了相应的转换
				这种作法个人认为是不好的, 应从数据文件设计上解决相关问题.


				class FiscalNo;					// 财务报告期工具类
				class FiscalYearNo;				// 财务报告期类
				class IndicatorWithParameter;	// 带参数的指标类
				class IndicatorWithParameterArray;	// 一组带参数的指标(std::vector), 其中实现通用计算接口

*************************************************/

// MyIndicator.h
#include <vector>
#include "..\..\data\indicator.h"
#include "..\..\Business\Business\TxBusiness.h"
#include "..\..\data\LogicalBusiness.h"
#include "..\..\core\core\variantdata.h"
#include "..\..\core\driver\Table_indicator.h"
#include "..\..\core\driver\table_display.h"

// 财务报告期

namespace Tx
{
	namespace Business
	{

typedef enum BUSINESS_EXT tagEnumFiscalNo
{
	fn_none = 0,		// 未知
	fn_month_1_3,		// 一季报		40040001
	fn_month_1_6,		// 中报			40040003
	fn_month_1_9,		// 三季报		40040005
	fn_month_1_12,		// 年报			40040009

	fn_quarter1,		// 第一季度		40040001
	fn_quarter2,		// 第二季度		40040002
	fn_quarter3,		// 第三季度		40040004
	fn_quarter4,		// 第四季度		40040006
	fn_month_6_12,		// 下半年		40040007
	fn_last_12			// 最近十二个月	40040008
} EnumFiscalNo;

// 财务报告期工具类
class BUSINESS_EXT FiscalNo
{
public:
	// 返回报告期名称
	static CString Name(int nNo);
	// 返回与数据库对照的 F_FISCAL_YEAR_QUARTER_ID
	static int Id(int nNo);
	// 返回起始月份
	static int StartMonth(int nNo);
	// 返回截止月份
	static int EndMonth(int nNo);
	// 是否单季报
	static bool IsQuarterly(int nNo);
	// int 到 转换
	static EnumFiscalNo Int2Enum(int nNo);
	// Enum 到 顺序(为生成"会计核算ID")
	static int Enum2Index(int nNo);
	// Enum 到 数据库中的ID
	static int Enum2ID(int nNo);
private:
	FiscalNo()
	{
	}
};

// 财务报告期类
class BUSINESS_EXT FiscalYearNo
{
public:
	FiscalYearNo();
	FiscalYearNo(int nYear);
	FiscalYearNo(int nYear, EnumFiscalNo nNo);
	FiscalYearNo(const FiscalYearNo& src);
	virtual ~FiscalYearNo();

	const FiscalYearNo& operator=(const FiscalYearNo& src)
	{
		m_dwYearNo = src.m_dwYearNo;
		return *this;
	}
	bool operator==(const FiscalYearNo& src) const
	{
		return m_dwYearNo == src.m_dwYearNo;
	}
	bool operator!=(const FiscalYearNo& src) const
	{
		return m_dwYearNo != src.m_dwYearNo;
	}

	operator int() const
	{
		return m_dwYearNo;
	}

	// 返回财年
	int GetYear() const		{ return HIWORD(m_dwYearNo); }
	// 返回报告期
	int GetNo() const		{ return LOWORD(m_dwYearNo); }
	// 返回名称
	CString GetName() const;
	// 返回起始日期
	int GetStartDate() const;
	// 返回截止日期
	int GetEndDate() const;
	// 返回上年同期
	FiscalYearNo SameOfLastYear() const;
	// 返回上年年报
	FiscalYearNo LastYear() const;
	// 返回上一期
	FiscalYearNo Previous() const;

protected:
	DWORD m_dwYearNo;	// HIWORD = year, LOWORD = no
};

typedef std::vector<FiscalYearNo> FiscalYearNoArray;

// 带参数的指标类
class BUSINESS_EXT IndicatorWithParameter
{
public:
	IndicatorWithParameter();
	IndicatorWithParameter(int iid);
	virtual ~IndicatorWithParameter();

public:
	// 是否有效
	bool IsValid() const
	{
		return m_pIndicatorData != NULL;
	}
	// 指标ID
	int GetId() const
	{
		if (m_pIndicatorData)
			return m_pIndicatorData->id;
		else
			return 0;
	}
	// 指标名称
	CString GetName() const
	{
		if (m_pIndicatorData)
		{
			CString sValue(m_pIndicatorData->cn_name);
			return sValue;
		}
		else
			return _T("");
	}
	// 指标显示名称
	CString GetDisplayName() const
	{
		if (m_pIndicatorData)
		{
			CString sValue(m_pIndicatorData->sDisPlayName);
			return sValue;
		}
		else
			return _T("");
	}
	// 显示单位
	CString GetUnit() const
	{
		if (m_pIndicatorData)
		{
			CString sValue(m_pIndicatorData->sOutputUnit);
			return sValue;
		}
		else
			return _T("");
	}
	// 显示格式
	CString GetFormatString() const
	{
		if (m_pIndicatorData)
		{
			CString sValue(m_pIndicatorData->sOutputItemFormat);
			return sValue;
		}
		else
			return _T("");
	}
	// 小数点精度
	int GetPrecision() const
	{
		if (m_pIndicatorData)
			return m_pIndicatorData->byOutputItemDec;
		else
			return 0;
	}
	// 数据类型
	int GetDataType() const
	{
		if (m_pIndicatorData)
			return m_pIndicatorData->DataType;
		else
			return 99;
	}
	// 输出比例
	double GetRatio() const
	{
		if (m_pIndicatorData)
			return m_pIndicatorData->fOutputItemRatio;
		else
			return 1.0;
	}
	// 参数个数
	int GetParameterCount() const
	{
		return (int)m_ParametersStamp.size();
	}
	// 指定参数数据类型
	int GetParameterType(int nIndex) const
	{
		if (m_pIndicatorData)
		{
			if (nIndex >= 0 && nIndex < GetParameterCount())
				return HIWORD(*(m_ParametersStamp.begin() + nIndex));
			else
				return 99;
		}
		else
			return 99;
	}
	// 指定参数描述
	CString GetParameterDescription(int nIndex) const
	{
		if (m_pIndicatorData)
		{
			if (nIndex >= 0 && nIndex < GetParameterCount())
				return _GetParameterDescription(LOWORD(*(m_ParametersStamp.begin() + nIndex)));
			else
				return _T("");
		}
		else
			return _T("");
	}
	// 指定参数描述ID
	int GetParameterDescriptionId(int nIndex) const
	{
		if (m_pIndicatorData)
		{
			if (nIndex >= 0 && nIndex < GetParameterCount())
				return LOWORD(*(m_ParametersStamp.begin() + nIndex));
			else
				return 0;
		}
		else
			return 0;
	}
	// 返回参数特征值序列
	bool GetParametersStamp(std::vector<DWORD>& arr) const
	{
		if (!m_pIndicatorData) return false;
		arr.clear();
		arr.assign(m_ParametersStamp.begin(), m_ParametersStamp.end());
		return true;
	}
	// 测试是否具有相同参数表
	bool HasSameParameters(const IndicatorWithParameter& src, bool bCheckValue = false) const
	{
		bool bRet = false;
		if (m_pIndicatorData && src.m_pIndicatorData && m_ParametersStamp.size() == src.m_ParametersStamp.size())
		{
			bRet = m_ParametersStamp == src.m_ParametersStamp;
			if (bRet && bCheckValue)
				bRet = m_ParametersValue == src.m_ParametersValue;
		}
		return bRet;
	}
	// 设置参数为默认值
	void SetParametersToDefault();

	// 返回参数(值)数组
	void GetParameterValueArray(std::vector<Tx::Core::VariantData>& arr) const;

	// 设置参数(值)
	void SetParameterValue(const std::vector<Tx::Core::VariantData>& arr);

	// 复制参数
	void CopyParameters(const IndicatorWithParameter& src);

	// 返回参数值字符串
	CString GetParameterValueText(int nIndex) const;
	// 返回数据类型 enum 字符串
	static CString DataType2EnumText(int nType);
	static Tx::Core::Data_Type DataType2Enum(int nType);
	// 返回显示文本
	CString GetText1() const;
	CString GetText2(bool bInclude1st = true) const;

public:
	Tx::Data::LogicalBusiness m_business;					// 用于调用底层方法
	Tx::Data::IndicatorData* m_pIndicatorData;				// 指标数据指针
	std::vector<DWORD> m_ParametersStamp;					// 参数特征值序列
	std::vector<Tx::Core::VariantData> m_ParametersValue;	// 参数值列表
	int m_nDirty;											// 是否需要"特殊处理"

public:
	static Tx::Core::Data_Type Int2DataTypeEnum(int nType);

protected:
	// 指定(类型)参数描述
	CString _GetParameterDescription(int nType) const;
};

class BUSINESS_EXT IndicatorWithParameterArray : public std::vector<IndicatorWithParameter>
{
public:
	IndicatorWithParameterArray();
	virtual ~IndicatorWithParameterArray();

	void Stat(int nBlockId = 0);
	void Stat(std::vector<int>& arrSecurity);

	// added by zhoup 07.09.17
	void StatFull();

	// added by zhoup 07.09.28
	void BuildTableIndicator();
	
	Tx::Business::TxBusiness m_business;					// 用于调用底层方法
	Tx::Core::Table_Display m_table;
	Tx::Core::Table_Indicator m_table_indicator;
};

// singleton 单件
class BUSINESS_EXT IndicatorFile
{
protected:
	IndicatorFile();
public:
	virtual ~IndicatorFile();

	static IndicatorFile* GetInstance();

	// 只添加列,具体的参数还要通过IWAP中的每一个IWP的SetParameter方法填写
	void SetIWAP(IndicatorWithParameterArray& IWPA,int nMenuId = 0);
	void SetIWAP(IndicatorWithParameterArray& IWPA,std::vector<int>& arrIID);
	void SetParameter(IndicatorWithParameterArray& IWPA,UINT nIndex,std::vector<Tx::Core::VariantData>& arrParam);
	void GetData(IndicatorWithParameterArray& IWPA,std::vector<int>& arrSecurityId,bool bFull = false);
//	void BuildTaleIndicator();
};
}
}