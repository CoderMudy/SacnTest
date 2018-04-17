#pragma once
/*************************************************
  Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
  File name:	MyIndicator.h
  Author:       ����
  Version:		1.0
  Date:			2007-09-04
  
  Description:
				ʵ�ִ�����ָ�궨��
				ԭ��:
				�ⲿ�����ݱ�Ӧ�����ݻ�ҵ���ʵ��,
				Ŀǰ, Ϊʵ���������, ������������ʵ��, 
				���������õ��� Tx::Data::IndicatorData ��ָ��
				����ʵ��Ӧ�ý�����ҵ�������ݴ���ì��, ��������Ӧ��ת��
				��������������Ϊ�ǲ��õ�, Ӧ�������ļ�����Ͻ���������.


				class FiscalNo;					// ���񱨸��ڹ�����
				class FiscalYearNo;				// ���񱨸�����
				class IndicatorWithParameter;	// ��������ָ����
				class IndicatorWithParameterArray;	// һ���������ָ��(std::vector), ����ʵ��ͨ�ü���ӿ�

*************************************************/

// MyIndicator.h
#include <vector>
#include "..\..\data\indicator.h"
#include "..\..\Business\Business\TxBusiness.h"
#include "..\..\data\LogicalBusiness.h"
#include "..\..\core\core\variantdata.h"
#include "..\..\core\driver\Table_indicator.h"
#include "..\..\core\driver\table_display.h"

// ���񱨸���

namespace Tx
{
	namespace Business
	{

typedef enum BUSINESS_EXT tagEnumFiscalNo
{
	fn_none = 0,		// δ֪
	fn_month_1_3,		// һ����		40040001
	fn_month_1_6,		// �б�			40040003
	fn_month_1_9,		// ������		40040005
	fn_month_1_12,		// �걨			40040009

	fn_quarter1,		// ��һ����		40040001
	fn_quarter2,		// �ڶ�����		40040002
	fn_quarter3,		// ��������		40040004
	fn_quarter4,		// ���ļ���		40040006
	fn_month_6_12,		// �°���		40040007
	fn_last_12			// ���ʮ������	40040008
} EnumFiscalNo;

// ���񱨸��ڹ�����
class BUSINESS_EXT FiscalNo
{
public:
	// ���ر���������
	static CString Name(int nNo);
	// ���������ݿ���յ� F_FISCAL_YEAR_QUARTER_ID
	static int Id(int nNo);
	// ������ʼ�·�
	static int StartMonth(int nNo);
	// ���ؽ�ֹ�·�
	static int EndMonth(int nNo);
	// �Ƿ񵥼���
	static bool IsQuarterly(int nNo);
	// int �� ת��
	static EnumFiscalNo Int2Enum(int nNo);
	// Enum �� ˳��(Ϊ����"��ƺ���ID")
	static int Enum2Index(int nNo);
	// Enum �� ���ݿ��е�ID
	static int Enum2ID(int nNo);
private:
	FiscalNo()
	{
	}
};

// ���񱨸�����
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

	// ���ز���
	int GetYear() const		{ return HIWORD(m_dwYearNo); }
	// ���ر�����
	int GetNo() const		{ return LOWORD(m_dwYearNo); }
	// ��������
	CString GetName() const;
	// ������ʼ����
	int GetStartDate() const;
	// ���ؽ�ֹ����
	int GetEndDate() const;
	// ��������ͬ��
	FiscalYearNo SameOfLastYear() const;
	// ���������걨
	FiscalYearNo LastYear() const;
	// ������һ��
	FiscalYearNo Previous() const;

protected:
	DWORD m_dwYearNo;	// HIWORD = year, LOWORD = no
};

typedef std::vector<FiscalYearNo> FiscalYearNoArray;

// ��������ָ����
class BUSINESS_EXT IndicatorWithParameter
{
public:
	IndicatorWithParameter();
	IndicatorWithParameter(int iid);
	virtual ~IndicatorWithParameter();

public:
	// �Ƿ���Ч
	bool IsValid() const
	{
		return m_pIndicatorData != NULL;
	}
	// ָ��ID
	int GetId() const
	{
		if (m_pIndicatorData)
			return m_pIndicatorData->id;
		else
			return 0;
	}
	// ָ������
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
	// ָ����ʾ����
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
	// ��ʾ��λ
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
	// ��ʾ��ʽ
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
	// С���㾫��
	int GetPrecision() const
	{
		if (m_pIndicatorData)
			return m_pIndicatorData->byOutputItemDec;
		else
			return 0;
	}
	// ��������
	int GetDataType() const
	{
		if (m_pIndicatorData)
			return m_pIndicatorData->DataType;
		else
			return 99;
	}
	// �������
	double GetRatio() const
	{
		if (m_pIndicatorData)
			return m_pIndicatorData->fOutputItemRatio;
		else
			return 1.0;
	}
	// ��������
	int GetParameterCount() const
	{
		return (int)m_ParametersStamp.size();
	}
	// ָ��������������
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
	// ָ����������
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
	// ָ����������ID
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
	// ���ز�������ֵ����
	bool GetParametersStamp(std::vector<DWORD>& arr) const
	{
		if (!m_pIndicatorData) return false;
		arr.clear();
		arr.assign(m_ParametersStamp.begin(), m_ParametersStamp.end());
		return true;
	}
	// �����Ƿ������ͬ������
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
	// ���ò���ΪĬ��ֵ
	void SetParametersToDefault();

	// ���ز���(ֵ)����
	void GetParameterValueArray(std::vector<Tx::Core::VariantData>& arr) const;

	// ���ò���(ֵ)
	void SetParameterValue(const std::vector<Tx::Core::VariantData>& arr);

	// ���Ʋ���
	void CopyParameters(const IndicatorWithParameter& src);

	// ���ز���ֵ�ַ���
	CString GetParameterValueText(int nIndex) const;
	// ������������ enum �ַ���
	static CString DataType2EnumText(int nType);
	static Tx::Core::Data_Type DataType2Enum(int nType);
	// ������ʾ�ı�
	CString GetText1() const;
	CString GetText2(bool bInclude1st = true) const;

public:
	Tx::Data::LogicalBusiness m_business;					// ���ڵ��õײ㷽��
	Tx::Data::IndicatorData* m_pIndicatorData;				// ָ������ָ��
	std::vector<DWORD> m_ParametersStamp;					// ��������ֵ����
	std::vector<Tx::Core::VariantData> m_ParametersValue;	// ����ֵ�б�
	int m_nDirty;											// �Ƿ���Ҫ"���⴦��"

public:
	static Tx::Core::Data_Type Int2DataTypeEnum(int nType);

protected:
	// ָ��(����)��������
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
	
	Tx::Business::TxBusiness m_business;					// ���ڵ��õײ㷽��
	Tx::Core::Table_Display m_table;
	Tx::Core::Table_Indicator m_table_indicator;
};

// singleton ����
class BUSINESS_EXT IndicatorFile
{
protected:
	IndicatorFile();
public:
	virtual ~IndicatorFile();

	static IndicatorFile* GetInstance();

	// ֻ�����,����Ĳ�����Ҫͨ��IWAP�е�ÿһ��IWP��SetParameter������д
	void SetIWAP(IndicatorWithParameterArray& IWPA,int nMenuId = 0);
	void SetIWAP(IndicatorWithParameterArray& IWPA,std::vector<int>& arrIID);
	void SetParameter(IndicatorWithParameterArray& IWPA,UINT nIndex,std::vector<Tx::Core::VariantData>& arrParam);
	void GetData(IndicatorWithParameterArray& IWPA,std::vector<int>& arrSecurityId,bool bFull = false);
//	void BuildTaleIndicator();
};
}
}