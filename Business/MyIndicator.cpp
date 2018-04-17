/*************************************************
  Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
  File name:	MyIndicator.cpp
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
#include "stdafx.h"
#include "MyIndicator.h"
#include "..\..\core\core\HourglassWnd.h"
#include "..\..\Data\MenuItemMap.h"
// ���񱨸��ڹ����� class FiscalNo �� static function
// ���ر���������
namespace Tx
{
	namespace Business
	{
CString FiscalNo::Name(int nNo)
{
	CString str = _T("");
	switch (nNo)
	{
	case fn_none:				 
		str = _T("δ֪");
		break;
	case fn_month_1_3:		
		str = _T("һ����");
		break;
	case fn_month_1_6:
		str = _T("�б�");
		break;
	case fn_month_1_9:
		str = _T("������");
		break;
	case fn_month_1_12:
		str = _T("�걨");
		break;
	case fn_quarter1:
		str = _T("��һ����");
		break;
	case fn_quarter2:
		str = _T("�ڶ�����");
		break;
	case fn_quarter3:
		str = _T("��������");
		break;
	case fn_quarter4:
		str = _T("���ļ���");
		break;
	case fn_month_6_12:
		str = _T("�°���");
		break;
	case fn_last_12:
		str = _T("���ʮ������");
		break;
	default:
		break;
	}
	return str;
}

// ���������ݿ���յ� F_FISCAL_YEAR_QUARTER_ID
int FiscalNo::Id(int nNo)
{
	int nId = 0;
	switch (nNo)
	{
	case fn_none:			// δ֪
		break;
	case fn_month_1_3:		// һ����
		nId = 40040001;
		break;
	case fn_month_1_6:		// �б�
		nId = 40040003;
		break;
	case fn_month_1_9:		// ������
		nId = 40040005;
		break;
	case fn_month_1_12:		// �걨
		nId = 40040009;
		break;
	case fn_quarter1:		// ��һ����
		nId = 40040001;
		break;
	case fn_quarter2:		// �ڶ�����
		nId = 40040002;
		break;
	case fn_quarter3:		// ��������
		nId = 40040004;
		break;
	case fn_quarter4:		// ���ļ���
		nId = 40040006;
		break;
	case fn_month_6_12:		// �°���
		nId = 40040007;
		break;
	case fn_last_12:		// ���ʮ������
		nId = 40040008;
		break;
	default:
		break;
	}
	return nId;
}

// ������ʼ�·�
int FiscalNo::StartMonth(int nNo)
{
	int nStart = 0;
	switch (nNo)
	{
	case fn_month_1_3:		// һ����
	case fn_month_1_6:		// �б�
	case fn_month_1_9:		// ������
	case fn_month_1_12:		// �걨
	case fn_quarter1:		// ��һ����
		nStart = 1;
		break;
	case fn_quarter2:		// �ڶ�����
	case fn_month_6_12:		// �°���
		nStart = 4;
		break;
	case fn_quarter3:		// ��������
		nStart = 7;
		break;
	case fn_quarter4:		// ���ļ���
		nStart = 10;
		break;
	//case fn_none:			// δ֪
	//case fn_last_12:		// ���ʮ������
	default:
		break;
	}
	return nStart;
}

// ���ؽ�ֹ�·�
int FiscalNo::EndMonth(int nNo)
{
	int nEnd = 0;
	switch (nNo)
	{
	case fn_month_1_3:		// һ����
	case fn_quarter1:		// ��һ����
		nEnd = 3;
		break;
	case fn_month_1_6:		// �б�
	case fn_quarter2:		// �ڶ�����
		nEnd = 6;
		break;
	case fn_month_1_9:		// ������
	case fn_quarter3:		// ��������
		nEnd = 9;
		break;
	case fn_month_1_12:		// �걨
	case fn_month_6_12:		// �°���
	case fn_quarter4:		// ���ļ���
		nEnd = 12;
		break;
	//case fn_none:			// δ֪
	//case fn_last_12:		// ���ʮ������
	default:
		break;
	}
	return nEnd;
}

// �Ƿ񵥼���
bool FiscalNo::IsQuarterly(int nNo)
{
	switch (nNo)
	{
	case fn_quarter1:		// ��һ����
	case fn_quarter2:		// �ڶ�����
	case fn_quarter3:		// ��������
	case fn_quarter4:		// ���ļ���
		return true;
	//case fn_month_6_12:		// �°���
	//case fn_month_1_3:		// һ����
	//case fn_month_1_6:		// �б�
	//case fn_month_1_9:		// ������
	//case fn_month_1_12:		// �걨
	//case fn_none:			// δ֪
	//case fn_last_12:		// ���ʮ������
	default:
		return false;
	}
}

// int �� ת��
EnumFiscalNo FiscalNo::Int2Enum(int nNo)
{
	switch (nNo)
	{
	case fn_none:			// δ֪
		return fn_none;
	case fn_month_1_3:		// һ����
		return fn_month_1_3;
	case fn_month_1_6:		// �б�
		return fn_month_1_6;
	case fn_month_1_9:		// ������
		return fn_month_1_9;
	case fn_month_1_12:		// �걨
		return fn_month_1_12;
	case fn_quarter1:		// ��һ����
		return fn_quarter1;
	case fn_quarter2:		// �ڶ�����
		return fn_quarter2;
	case fn_quarter3:		// ��������
		return fn_quarter3;
	case fn_quarter4:		// ���ļ���
		return fn_quarter4;
	case fn_month_6_12:		// �°���
		return fn_month_6_12;
	case fn_last_12:		// ���ʮ������
		return fn_last_12;
	default:
		return fn_none;
	}
}

// Enum �� ˳��(Ϊ����"��ƺ���ID")
int FiscalNo::Enum2Index(int nNo)
{
	switch (nNo)
	{
	case fn_none:			// δ֪
		return 0;
	case fn_month_1_3:		// һ����
		return 1;
	case fn_month_1_6:		// �б�
		return 3;
	case fn_month_1_9:		// ������
		return 5;
	case fn_month_1_12:		// �걨
		return 9;
	case fn_quarter1:		// ��һ����
		return 1;
	case fn_quarter2:		// �ڶ�����
		return 2;
	case fn_quarter3:		// ��������
		return 4;
	case fn_quarter4:		// ���ļ���
		return 6;
	case fn_month_6_12:		// �°���
		return 7;
	case fn_last_12:		// ���ʮ������
		return 8;
	default:
		return 0;
	}
}

// Enum �� ���ݿ��е�ID
int FiscalNo::Enum2ID(int nNo)
{
	switch (nNo)
	{
	case fn_none:			// δ֪
		return 0;
	case fn_month_1_3:		// һ����
		return 40040001;
	case fn_month_1_6:		// �б�
		return 40040003;
	case fn_month_1_9:		// ������
		return 40040005;
	case fn_month_1_12:		// �걨
		return 40040009;
	case fn_quarter1:		// ��һ����
		return 40040001;
	case fn_quarter2:		// �ڶ�����
		return 40040002;
	case fn_quarter3:		// ��������
		return 40040004;
	case fn_quarter4:		// ���ļ���
		return 40040006;
	case fn_month_6_12:		// �°���
		return 40040007;
	case fn_last_12:		// ���ʮ������
		return 40040008;
	default:
		return 0;
	}
}

// ���񱨸����� class FiscalYearNo
FiscalYearNo::FiscalYearNo()
{
	// HIWORD = year, LOWORD = no
	COleDateTime tmp = COleDateTime::GetCurrentTime();
	switch (tmp.GetMonth())
	{
	case 1:
	case 2:
	case 3:		// �걨(��һ��)
		m_dwYearNo = MAKELONG(fn_month_1_12, tmp.GetYear() - 1);
		break;
	case 4:
	case 5:
	case 6:		// һ����
		m_dwYearNo = MAKELONG(fn_month_1_3, tmp.GetYear());
		break;
	case 7:
	case 8:
	case 9:		// �б�
		m_dwYearNo = MAKELONG(fn_month_1_6, tmp.GetYear());
		break;
	case 10:
	case 11:
	case 12:	// ������
		m_dwYearNo = MAKELONG(fn_month_1_6, tmp.GetYear());
		break;
	}
}

FiscalYearNo::FiscalYearNo(int nYear)
{
	COleDateTime tmp = COleDateTime::GetCurrentTime();
	switch (tmp.GetMonth())
	{
	case 1:
	case 2:
	case 3:		// �걨(��һ��)
		m_dwYearNo = MAKELONG(fn_month_1_12, nYear - 1);
		break;
	case 4:
	case 5:
	case 6:		// һ����
		m_dwYearNo = MAKELONG(fn_month_1_3, nYear);
		break;
	case 7:
	case 8:
	case 9:		// �б�
		m_dwYearNo = MAKELONG(fn_month_1_6, nYear);
		break;
	case 10:
	case 11:
	case 12:	// ������
		m_dwYearNo = MAKELONG(fn_month_1_6, nYear);
		break;
	}
}

FiscalYearNo::FiscalYearNo(int nYear, EnumFiscalNo nNo)
{
	m_dwYearNo = MAKELONG(nNo, nYear);
}

FiscalYearNo::FiscalYearNo(const FiscalYearNo& src)
{
	m_dwYearNo = src.m_dwYearNo;
}

FiscalYearNo::~FiscalYearNo()
{
}

// ��������
CString FiscalYearNo::GetName() const
{
	CString str;
	str.Format(_T("%d%s"), GetYear(), FiscalNo::Name(GetNo()));
	return str;
}

// ������ʼ����
int FiscalYearNo::GetStartDate() const
{
	int y, m, d;
	y = GetYear();
	m = FiscalNo::StartMonth(GetNo());
	d = 1;
	ASSERT(m != 0);
	return 10000 * y + 100 * m + d;
}

// ���ؽ�ֹ����
int FiscalYearNo::GetEndDate() const
{
	int y, m, d;
	y = GetYear();
	m = FiscalNo::EndMonth(GetNo());
	d = 1;
	ASSERT(m != 0);
	if (m == 12)
	{
		y++;
		m = 1;
	}
	else
	{
		m++;
	}
	COleDateTime tmp(y, m, d, 0, 0, 0);		// �õ���һ��1��
	tmp -= 1;	// ��ǰһ��(Ŀ���Ǳ����µ�)
	return 10000 * tmp.GetYear() + 100 * tmp.GetMonth() + tmp.GetDay();
}

// ��������ͬ��
FiscalYearNo FiscalYearNo::SameOfLastYear() const
{
	return FiscalYearNo(GetYear() - 1, FiscalNo::Int2Enum(GetNo()));
}

// ���������걨
FiscalYearNo FiscalYearNo::LastYear() const
{
	return FiscalYearNo(GetYear() - 1, fn_month_1_12);
}

// ������һ��
FiscalYearNo FiscalYearNo::Previous() const
{
	switch (GetNo())
	{
	case fn_month_1_3:		// һ����
		return FiscalYearNo(GetYear() - 1, fn_month_1_12);
	case fn_month_1_6:		// �б�
		return FiscalYearNo(GetYear(), fn_month_1_3);
	case fn_month_1_9:		// ������
		return FiscalYearNo(GetYear(), fn_month_1_6);
	case fn_month_1_12:		// �걨
		return FiscalYearNo(GetYear(), fn_month_1_9);

	case fn_quarter1:		// ��һ����
		return FiscalYearNo(GetYear() - 1, fn_quarter4);
	case fn_quarter2:		// �ڶ�����
		return FiscalYearNo(GetYear(), fn_quarter1);
	case fn_quarter3:		// ��������
		return FiscalYearNo(GetYear(), fn_quarter2);
	case fn_quarter4:		// ���ļ���
		return FiscalYearNo(GetYear(), fn_quarter3);
	//case fn_month_6_12:		// �°���
	//case fn_last_12:		// ���ʮ������
	default:
		ASSERT (0);
		return FiscalYearNo(*this);
	}
}

// static 
Tx::Core::Data_Type IndicatorWithParameter::Int2DataTypeEnum(int nType)
{
	switch (nType)
	{
	case Tx::Core::dtype_bool:               return Tx::Core::dtype_bool;
	case Tx::Core::dtype_char:               return Tx::Core::dtype_char;
	case Tx::Core::dtype_byte:               return Tx::Core::dtype_byte;
	case Tx::Core::dtype_int2:               return Tx::Core::dtype_int2;
	case Tx::Core::dtype_uint2:              return Tx::Core::dtype_uint2;
	case Tx::Core::dtype_int4:               return Tx::Core::dtype_int4;
	case Tx::Core::dtype_uint4:              return Tx::Core::dtype_uint4;
	case Tx::Core::dtype_int8:               return Tx::Core::dtype_int8;
	case Tx::Core::dtype_uint8:              return Tx::Core::dtype_uint8;
	case Tx::Core::dtype_float:              return Tx::Core::dtype_float;
	case Tx::Core::dtype_double:             return Tx::Core::dtype_double;
	case Tx::Core::dtype_decimal:            return Tx::Core::dtype_decimal;
	case Tx::Core::dtype_guid:               return Tx::Core::dtype_guid;
	case Tx::Core::dtype_rowversion:         return Tx::Core::dtype_rowversion;
	case Tx::Core::dtype_txdate:             return Tx::Core::dtype_txdate;
	case Tx::Core::dtype_txtime:             return Tx::Core::dtype_txtime;
	case Tx::Core::dtype_txdatetime:         return Tx::Core::dtype_txdatetime;
	case Tx::Core::dtype_val_binary:         return Tx::Core::dtype_val_binary;
	case Tx::Core::dtype_val_string:         return Tx::Core::dtype_val_string;
	case Tx::Core::dtype_val_ansi_string:    return Tx::Core::dtype_val_ansi_string;
	case Tx::Core::dtype_val_unic_string:    return Tx::Core::dtype_val_unic_string;
	default:
		{
			ASSERT(0);
			return Tx::Core::dtype_bool;
		}
	}
}

// IndicatorWithParameter
IndicatorWithParameter::IndicatorWithParameter() : m_pIndicatorData(NULL), m_nDirty(0)
{
}

IndicatorWithParameter::IndicatorWithParameter(int iid) : m_nDirty(0)
{
	m_pIndicatorData = m_business.m_pIndicator->GetIndicator(iid);
	if (m_pIndicatorData)
	{
		if (m_pIndicatorData->ParaDescType[0] == 18)
		{
			// ���⴦��"��ƺ���ID"
			m_nDirty = 1;
			//ASSERT(m_pIndicatorData->GetParamCount() == 1);
			// ����ID(3)
			m_ParametersStamp.push_back(MAKELONG(3, Tx::Core::dtype_int4));
			m_ParametersValue.push_back(Tx::Core::VariantData(Tx::Core::dtype_int4));
			// ������(1001)
			m_ParametersStamp.push_back(MAKELONG(1001, Tx::Core::dtype_int4));
			m_ParametersValue.push_back(Tx::Core::VariantData(Tx::Core::dtype_int4));
			// �ϲ�/ĸ��˾(1002)
			m_ParametersStamp.push_back(MAKELONG(1002, Tx::Core::dtype_bool));
			m_ParametersValue.push_back(Tx::Core::VariantData(Tx::Core::dtype_bool));
			// ����ǰ/������(1003)
			m_ParametersStamp.push_back(MAKELONG(1003, Tx::Core::dtype_bool));
			m_ParametersValue.push_back(Tx::Core::VariantData(Tx::Core::dtype_bool));
			if (m_pIndicatorData->GetParamCount() > 1)
			{
				for (int i = 1; i < m_pIndicatorData->GetParamCount(); i++)
				{
					m_ParametersStamp.push_back(MAKELONG(m_pIndicatorData->ParaDescType[i], m_pIndicatorData->ParaType[i]));
					m_ParametersValue.push_back(Tx::Core::VariantData(IndicatorWithParameter::Int2DataTypeEnum(m_pIndicatorData->ParaType[i])));
				}
			}
		}
		else
		{
			for (int i = 0; i < m_pIndicatorData->GetParamCount(); i++)
			{
				m_ParametersStamp.push_back(MAKELONG(m_pIndicatorData->ParaDescType[i], m_pIndicatorData->ParaType[i]));
				m_ParametersValue.push_back(Tx::Core::VariantData(IndicatorWithParameter::Int2DataTypeEnum(m_pIndicatorData->ParaType[i])));
			}
		}
		// ����������ΪĬ��ֵ
		SetParametersToDefault();
	}
}

IndicatorWithParameter::~IndicatorWithParameter()
{
	m_ParametersStamp.clear();
	m_ParametersValue.clear();
	m_pIndicatorData = NULL;
}

// ָ��(����)��������
CString IndicatorWithParameter::_GetParameterDescription(int nType) const
{
	switch (m_nDirty)
	{
	case 1:			// ���⴦��"��ƺ���ID"
		switch (nType)
		{
		case 1001:
			return _T("������");
		case 1002:
			return _T("ʹ�úϲ�����");
		case 1003:
			return _T("ʹ�õ�������");
		default:
			break;
		}
		break;
	default:		// ����
		return m_business.m_pIndicator->GetIndicatorParamDesc(nType);
	}
	return _T("");
}

// ���ò���ΪĬ��ֵ
void IndicatorWithParameter::SetParametersToDefault()
{
	if (m_pIndicatorData == NULL) return;
	ASSERT (m_ParametersStamp.size() == m_ParametersValue.size());
	for (UINT i = 0; i < (UINT)m_ParametersStamp.size(); i++)
	{
		int nType, nDesc;
		nType = HIWORD(m_ParametersStamp[i]);
		nDesc = LOWORD(m_ParametersStamp[i]);
		// ����ʵ��ID(1), ȯID(2), ����ID(3), ��������(4), ��ֹ����(11), ���(15), ��ƺ���id(18), ��׼����(23)
		switch (nDesc)
		{
		case 1:		// ����ʵ��ID
		case 2:		// ȯID
		case 3:		// ����ID
			if (nType == Tx::Core::dtype_int4)
			{
				m_ParametersValue[i] = (int)0;
			}
			else
			{
				//ASSERT(0);
				TRACE(_T("\n(%d)DataType is: %s"), nDesc, IndicatorWithParameter::DataType2EnumText(nType));
			}
			break;
		case 4:		// ��������
		case 11:	// ��ֹ����
		case 23:	// ��׼����
			if (nType == Tx::Core::dtype_int4)
			{
				COleDateTime tmp = COleDateTime::GetCurrentTime();
				m_ParametersValue[i] = (int)(tmp.GetYear() * 10000 + tmp.GetMonth() * 100 + tmp.GetDay());
			}
			else
			{
				//ASSERT(0);
				TRACE(_T("\n(%d)DataType is: %s"), nDesc, IndicatorWithParameter::DataType2EnumText(nType));
			}
			break;
		case 15:	// ���
			if (nType == Tx::Core::dtype_int2)
			{
				m_ParametersValue[i] = (short)1;
			}
			else if (nType == Tx::Core::dtype_int4)
			{
				m_ParametersValue[i] = (int)1;
			}
			else
			{
				//ASSERT(0);
				TRACE(_T("\n(%d)DataType is: %s"), nDesc, IndicatorWithParameter::DataType2EnumText(nType));
			}
			break;
		case 1001:	// ������
			if (nType == Tx::Core::dtype_int4)
			{
				m_ParametersValue[i] = (int)(FiscalYearNo());
			}
			else
			{
				ASSERT(0);
			}
			break;
		case 1002:	// �ϲ�
		case 1003:	// ����
			if (nType == Tx::Core::dtype_bool)
			{
				m_ParametersValue[i] = true;
			}
			else
			{
				//ASSERT(0);
				TRACE(_T("\n(%d)DataType is: %s"), nDesc, IndicatorWithParameter::DataType2EnumText(nType));
			}
			break;
		default:
			TRACE(_T("\nDescriptionId = %d(%s), %s"), nDesc, _GetParameterDescription(nDesc), GetName());
			//ASSERT (0);
			break;
		}
	}
}

// ���ز���(ֵ)����
void IndicatorWithParameter::GetParameterValueArray(std::vector<Tx::Core::VariantData>& arr) const
{
	arr.clear();
	if (m_pIndicatorData && !m_ParametersValue.empty())
	{
		arr.assign(m_ParametersValue.begin(),m_ParametersValue.end());
	}
}

// ���ò���(ֵ) ֻ����������ͼ���, û�ж�������Ч�Լ��Լ���, �������������"����ȫ"��, ��Ҫ�ڵ���ʱ�������ݵĿɿ���!!!
void IndicatorWithParameter::SetParameterValue(const std::vector<Tx::Core::VariantData>& arr)
{
	if (arr.size() == m_ParametersValue.size())
	{
		for (UINT i = 0; i < (UINT)arr.size(); i++)
		{
			if (arr[i].data_type != m_ParametersValue[i].data_type)
			{
				ASSERT(0);
				return;
			}
		}
		m_ParametersValue.clear();
		m_ParametersValue.assign(arr.begin(), arr.end());
	}
	else
	{
		ASSERT(0);
	}
}

// ���Ʋ���
void IndicatorWithParameter::CopyParameters(const IndicatorWithParameter& src)
{
	if (src.IsValid() && HasSameParameters(src, false))
	{
		SetParameterValue(src.m_ParametersValue);
	}
}

// ���ز���ֵ�ַ���
CString IndicatorWithParameter::GetParameterValueText(int nIndex) const
{
	CString str = _T("");
	if (m_pIndicatorData == NULL || GetParameterCount() == 0)
	{
		return str;
	}
	if (nIndex < 0 || nIndex >= GetParameterCount())
	{
		ASSERT (0);
		return str;
	}
	int nType, nDesc;
	nType = HIWORD(m_ParametersStamp[nIndex]);
	nDesc = LOWORD(m_ParametersStamp[nIndex]);

	switch (nDesc)
	{
	case 1:		// ����ʵ��ID
	case 2:		// ȯID
	case 3:		// ����ID
		str = _T("��ǰ����");
		break;
	case 4:		// ��������
	case 11:	// ��ֹ����
	case 23:	// ��׼����
		if (nType == Tx::Core::dtype_int4)
		{
			int val = (int)m_ParametersValue[nIndex];
			int y;
			div_t tmp = div(val, 10000);
			y = tmp.quot;
			tmp = div(tmp.rem, 100);
			str.Format(_T("%d-%02d-%02d"), y, tmp.quot, tmp.rem);
		}
		else
		{
			ASSERT(0);
		}
		break;
	case 15:	// ���
		if (nType == Tx::Core::dtype_int2)
		{
			short val = (short)m_ParametersValue[nIndex];
			str.Format(_T("%d"), val);
		}
		else if (nType == Tx::Core::dtype_int4)
		{
			int val = (int)m_ParametersValue[nIndex];
			str.Format(_T("%d"), val);
		}
		else
		{
			ASSERT(0);
		}
		break;
	case 1001:	// ������
		if (nType == Tx::Core::dtype_int4)
		{
			int val = (int)m_ParametersValue[nIndex];
			int year = HIWORD(val);
			int no = LOWORD(val);
			FiscalYearNo tmp(year, FiscalNo::Int2Enum(no));
			str = tmp.GetName();
		}
		else
		{
			ASSERT(0);
		}
		break;
	case 1002:	// ����
	case 1003:	// �ϲ�
		if (nType == Tx::Core::dtype_bool)
		{
			bool val = (bool)m_ParametersValue[nIndex];
			str = val ? _T("true") : _T("false");
		}
		else
		{
			ASSERT(0);
		}
		break;
	default:
		ASSERT (0);
		break;
	}
	return str;
}

// static
CString IndicatorWithParameter::DataType2EnumText(int nType)
{
	switch(nType)
	{
	case Tx::Core::dtype_bool:				return _T("dtype_bool");
	case Tx::Core::dtype_char:				return _T("dtype_char");
	case Tx::Core::dtype_byte:				return _T("dtype_byte");
	case Tx::Core::dtype_int2:				return _T("dtype_int2");
	case Tx::Core::dtype_uint2:				return _T("dtype_uint2");
	case Tx::Core::dtype_int4:				return _T("dtype_int4");
	case Tx::Core::dtype_uint4:				return _T("dtype_uint4");
	case Tx::Core::dtype_int8:				return _T("dtype_int8");
	case Tx::Core::dtype_uint8:				return _T("dtype_uint8");
	case Tx::Core::dtype_float:				return _T("dtype_float");
	case Tx::Core::dtype_double:			return _T("dtype_double");
	case Tx::Core::dtype_decimal:			return _T("dtype_decimal");
	case Tx::Core::dtype_guid:				return _T("dtype_guid");
	case Tx::Core::dtype_rowversion:		return _T("dtype_rowversion");
	case Tx::Core::dtype_txdate:			return _T("dtype_txdate");
	case Tx::Core::dtype_txtime:			return _T("dtype_txtime");
	case Tx::Core::dtype_txdatetime:		return _T("dtype_txdatetime");
	case Tx::Core::dtype_val_binary:		return _T("dtype_val_binary");
	case Tx::Core::dtype_val_string:		return _T("dtype_val_string");
	case Tx::Core::dtype_val_ansi_string:	return _T("dtype_val_ansi_string");
	case Tx::Core::dtype_val_unic_string:	return _T("dtype_val_unic_string");
	default:								return _T("unknow");
	}
}

Tx::Core::Data_Type IndicatorWithParameter::DataType2Enum(int nType)
{
	switch(nType)
	{
    case Tx::Core::dtype_bool:              return Tx::Core::dtype_bool;
    case Tx::Core::dtype_char:              return Tx::Core::dtype_char;
    case Tx::Core::dtype_byte:              return Tx::Core::dtype_byte;
    case Tx::Core::dtype_int2:              return Tx::Core::dtype_int2;
    case Tx::Core::dtype_uint2:             return Tx::Core::dtype_uint2;
    case Tx::Core::dtype_int4:              return Tx::Core::dtype_int4;
    case Tx::Core::dtype_uint4:             return Tx::Core::dtype_uint4;
    case Tx::Core::dtype_int8:              return Tx::Core::dtype_int8;
    case Tx::Core::dtype_uint8:             return Tx::Core::dtype_uint8;
    case Tx::Core::dtype_float:             return Tx::Core::dtype_float;
    case Tx::Core::dtype_double:            return Tx::Core::dtype_double;
    case Tx::Core::dtype_decimal:           return Tx::Core::dtype_decimal;
    case Tx::Core::dtype_guid:              return Tx::Core::dtype_guid;
    case Tx::Core::dtype_rowversion:        return Tx::Core::dtype_rowversion;
    case Tx::Core::dtype_txdate:            return Tx::Core::dtype_txdate;
    case Tx::Core::dtype_txtime:            return Tx::Core::dtype_txtime;
    case Tx::Core::dtype_txdatetime:        return Tx::Core::dtype_txdatetime;
    case Tx::Core::dtype_val_binary:        return Tx::Core::dtype_val_binary;
    case Tx::Core::dtype_val_string:        return Tx::Core::dtype_val_string;
    case Tx::Core::dtype_val_ansi_string:   return Tx::Core::dtype_val_ansi_string;
    case Tx::Core::dtype_val_unic_string:   return Tx::Core::dtype_val_unic_string;
	default:								
		ASSERT (0);
		return Tx::Core::dtype_int4;;
	}
}

// ������ʾ�ı�, ���ش���ʽ������
// �ܹɱ�(����ID[dtype_int4], ��ֹ����[dtype_int4])
CString IndicatorWithParameter::GetText1() const
{
	CString str = _T("");
	if (m_pIndicatorData)
	{
		str += GetName() + _T("(");
		for (int i = 0; i < GetParameterCount(); i++)
		{
			str += _GetParameterDescription(LOWORD(*(m_ParametersStamp.begin() + i))) + _T("[");
			str += IndicatorWithParameter::DataType2EnumText(HIWORD(*(m_ParametersStamp.begin() + i))) + _T("], ");
		}
		str.TrimRight(_T(", "));
		str += _T(")");
	}
	return str;
}

// ˰������(��ǰ����, 2007�걨, true, true)
CString IndicatorWithParameter::GetText2(bool bInclude1st) const
{
	CString str = _T("");
	if (m_pIndicatorData)
	{
		str += GetName() + _T("(");
		for (int i = bInclude1st ? 0 : 1; i < GetParameterCount(); i++)
		{
			str += GetParameterValueText(i) + _T(", ");
		}
		str.TrimRight(_T(", "));
		str += _T(")");
	}
	return str;
}

// class IndicatorWithParameterArray : public std::vector<IndicatorWithParameter>
IndicatorWithParameterArray::IndicatorWithParameterArray()
{
}
IndicatorWithParameterArray::~IndicatorWithParameterArray()
{
	clear();
}

void get_security_array(int id, std::vector<int>& arrOut)
{
	arrOut.clear();
	Tx::Business::TxBusiness p;

	std::set<int> items;
	if (id == 0)
	{
		// ȡ��ǰ���, Ŀǰ�����ڰ��
		p.GetCollectionItems(60000454, items);
	}
	else
	{
		p.GetCollectionItems(id, items);
	}
	for (std::set<int>::iterator iter = items.begin(); iter != items.end(); iter++)
	{
		arrOut.push_back(*iter);
	}
}

void IndicatorWithParameterArray::Stat(int nBlockId)
{
	std::vector<int> arrSecurity;
	get_security_array(nBlockId, arrSecurity);
	Stat(arrSecurity);
}

void IndicatorWithParameterArray::Stat(std::vector<int>& arrSecurity)
{
	Tx::Core::HourglassWnd hourglass;
	hourglass.Show(_T("����ͳ��..."));

	m_table_indicator.Clear();
	m_table.Clear();
	UINT nIndex = 0;
	// ��������ID������
	m_table_indicator.AddCol(Tx::Core::dtype_int4);
	m_table_indicator.AddCol(Tx::Core::dtype_int4);
	m_table_indicator.AddCol(Tx::Core::dtype_int4);
	
	if (arrSecurity.empty()) 
	{
		// ��׷��һ�������
		m_table_indicator.AddRow(1);
	}
	else
	{
		m_table_indicator.AddRow((UINT)arrSecurity.size());
		for (UINT i = 0; i < (UINT)arrSecurity.size(); i++)
		{
			m_business.GetSecurityNow((long)arrSecurity[i]);
			if (m_business.m_pSecurity)
			{
				m_table_indicator.SetCell(0, i, (int)m_business.m_pSecurity->GetId());
				m_table_indicator.SetCell(1, i, (int)m_business.m_pSecurity->GetSecurity1Id());
				m_table_indicator.SetCell(2, i, (int)m_business.m_pSecurity->GetInstitutionId());
			}
		}
	}
	m_table.AddRow(m_table_indicator.GetRowCount());
	// 
	std::vector<DWORD> arrParaPos;
	nIndex = 3;
	for (/*const_*/iterator iter = begin(); iter != end(); iter++)
	{
		/*const */IndicatorWithParameter& item = *iter;
		if (item.m_nDirty == 1)
		{
			DWORD dw = MAKELONG(nIndex, nIndex);
			arrParaPos.push_back(dw);
			m_table_indicator.AddParameterColumn(Tx::Core::dtype_int8, false);
			for (UINT r = 0;  r < m_table_indicator.GetRowCount(); r++)
			{
				__int64 id64;
				int institutionid;
				m_table_indicator.GetCell(2, r, institutionid);
				int val = (int)item.m_ParametersValue[1];
				bool b1, b2;
				b1 = (bool)item.m_ParametersValue[2];
				b2 = (bool)item.m_ParametersValue[3];
				id64 = (__int64)institutionid * (__int64)10000000 
					+ (__int64)HIWORD(val) * (__int64)1000
					+ (__int64)FiscalNo::Enum2Index(LOWORD(val)) * (__int64)100
					+ (__int64)(b1 ? 10 : 0)
					+ (__int64)(b2 ? 1 : 0);
				//CString sss;
				//sss.Format(_T("%I64d"),id64);
				//AfxMessageBox(sss);
				m_table_indicator.SetCell(nIndex, r, id64);
			}
			nIndex++;
		}
		else
		{
			if (item.GetParameterCount() > 1)
			{
				if (item.GetParameterCount() == 1)
					arrParaPos.push_back(0);
				else
				{
					DWORD dw = MAKELONG(nIndex, nIndex + item.GetParameterCount() - 1 - 1);
					arrParaPos.push_back(dw);
				}
				for (std::vector<Tx::Core::VariantData>::/*const_*/iterator iter_param = item.m_ParametersValue.begin() + 1; iter_param != item.m_ParametersValue.end(); iter_param++)
				{
					/*const */Tx::Core::VariantData& param = *iter_param;
					m_table_indicator.AddParameterColumn(param.data_type, true);
					m_table_indicator.SetCell(nIndex, 0, param);
					nIndex++;
				}
			}
			else
			{
				arrParaPos.push_back((DWORD)0);
			}
		}
	}
	ASSERT (arrParaPos.size() == size());
	// �������ָ����
	for (UINT i = 0; i < (UINT)size(); i++)
	{
		const IndicatorWithParameter& item = *(begin() + i);
		if (item.GetParameterCount() < 1)
		{
			ASSERT (0);
			continue;
		}
		UINT nParamCount = 0;
		UINT * pIndex = NULL;
		if (item.m_nDirty == 1)
		{
			nParamCount = 1;
			pIndex = new UINT [nParamCount];
			DWORD dw = arrParaPos[i];
			UINT nBegin, nEnd;
			nBegin = LOWORD(dw);
			nEnd = HIWORD(dw);
			ASSERT (nBegin == nEnd);
			pIndex[0] = nBegin;
		}
		else
		{
			nParamCount = item.GetParameterCount();
			pIndex = new UINT [nParamCount];
			pIndex[0] = item.m_pIndicatorData->Belong2Entity;
			DWORD dw = arrParaPos[i];
			if (dw)
			{
				UINT nBegin, nEnd;
				nBegin = LOWORD(dw);
				nEnd = HIWORD(dw);
				ASSERT (nBegin <= nEnd);
				for (UINT i = nBegin; i <= nEnd; i++)
				{
					pIndex[1 + i - nBegin] = i;
				}
			}
		}
		m_table_indicator.AddIndicatorColumn(item.GetId(), IndicatorWithParameter::DataType2Enum(item.GetDataType()), pIndex, nParamCount);
		if (pIndex) delete pIndex;
	}

	m_table.AttachColumn(m_table_indicator, m_table_indicator.GetColCount() - (int)size(), m_table_indicator.GetColCount() - 1);
	for (UINT i = 0; i < (UINT)size(); i++)
	{
		m_table.SetTitle(i, (*(begin() + i)).GetName());
	}

	Tx::Data::LogicalBusiness business;
	business.GetData(m_table_indicator);

	hourglass.Hide();
}

void IndicatorWithParameterArray::StatFull()
{
	Tx::Core::HourglassWnd hourglass;
	hourglass.Show(_T("����ͳ��..."));

	m_table_indicator.Clear();
	m_table.Clear();
	UINT nIndex = 3;
	//// ��������ID������
	m_table_indicator.AddCol(Tx::Core::dtype_int4);
	m_table_indicator.AddCol(Tx::Core::dtype_int4);
	m_table_indicator.AddCol(Tx::Core::dtype_int4);

	//if (arrSecurity.empty()) 
	//{
	//	// ��׷��һ�������
	//	m_table_indicator.AddRow(1);
	//}
	//else
	//{
	//	m_table_indicator.AddRow((UINT)arrSecurity.size());
	//	for (UINT i = 0; i < (UINT)arrSecurity.size(); i++)
	//	{
	//		m_business.GetSecurityNow((long)arrSecurity[i]);
	//		if (m_business.m_pSecurity)
	//		{
	//			m_table_indicator.SetCell(0, i, (int)m_business.m_pSecurity->GetId());
	//			m_table_indicator.SetCell(1, i, (int)m_business.m_pSecurity->GetSecurity1Id());
	//			m_table_indicator.SetCell(2, i, (int)m_business.m_pSecurity->GetInstitutionId());
	//		}
	//	}
	//}
	//m_table.AddRow(m_table_indicator.GetRowCount());
	// 
	std::vector<DWORD> arrParaPos;
	nIndex = 3;
	//for (/*const_*/iterator iter = begin(); iter != end(); iter++)
	//{
	iterator iter = begin();
		/*const */IndicatorWithParameter& item = *iter;
		if (item.m_nDirty == 1)
		{
			DWORD dw = MAKELONG(nIndex, nIndex);
			for (/*const_*/iterator iter = begin(); iter != end(); iter++)
				arrParaPos.push_back(dw);
			m_table_indicator.AddParameterColumn(Tx::Core::dtype_int8, false);
			//for (UINT r = 0;  r < m_table_indicator.GetRowCount(); r++)
			//{
			//	__int64 id64;
			//	int institutionid;
			//	m_table_indicator.GetCell(2, r, institutionid);
			//	int val = (int)item.m_ParametersValue[1];
			//	bool b1, b2;
			//	b1 = (bool)item.m_ParametersValue[2];
			//	b2 = (bool)item.m_ParametersValue[3];
			//	id64 = (__int64)institutionid * (__int64)10000000 
			//		+ (__int64)HIWORD(val) * (__int64)1000
			//		+ (__int64)FiscalNo::Enum2Index(LOWORD(val)) * (__int64)100
			//		+ (__int64)(b1 ? 10 : 0)
			//		+ (__int64)(b2 ? 1 : 0);
			//	//CString sss;
			//	//sss.Format(_T("%I64d"),id64);
			//	//AfxMessageBox(sss);
			//	m_table_indicator.SetCell(nIndex, r, id64);
			//}
			//nIndex++;
		}
		else
		{
			for (/*const_*/iterator iter = begin(); iter != end(); iter++)
			{
				if (item.GetParameterCount() == 1)
					arrParaPos.push_back(0);
				else
				{
					DWORD dw = MAKELONG(nIndex, nIndex + item.GetParameterCount() - 1 - 1);
					arrParaPos.push_back(dw);
				}
			}
			for (int i=1;i<item.GetParameterCount();i++)
			{
				m_table_indicator.AddParameterColumn((Tx::Core::Data_Type)item.GetParameterType(i));

			}
			//for (std::vector<Tx::Core::VariantData>::/*const_*/iterator iter_param = item.m_ParametersValue.begin() + 1; iter_param != item.m_ParametersValue.end(); iter_param++)
			//{
			//	/*const */Tx::Core::VariantData& param = *iter_param;
			//	m_table_indicator.AddParameterColumn(param.data_type);
			////	m_table_indicator.SetCell(nIndex, 0, param);
			////	nIndex++;
			//}
		}
	//}
	ASSERT (arrParaPos.size() == size());
	// �������ָ����
	for (UINT i = 0; i < (UINT)size(); i++)
	{
		const IndicatorWithParameter& item = *(begin() + i);
		if (item.GetParameterCount() < 1)
		{
			ASSERT (0);
			continue;
		}
		UINT nParamCount = 0;
		UINT * pIndex = NULL;
		if (item.m_nDirty == 1)
		{
			nParamCount = 1;
			pIndex = new UINT [nParamCount];
			DWORD dw = arrParaPos[i];
			UINT nBegin, nEnd;
			nBegin = LOWORD(dw);
			nEnd = HIWORD(dw);
			ASSERT (nBegin == nEnd);
			pIndex[0] = nBegin;
		}
		else
		{
			nParamCount = item.GetParameterCount();
			pIndex = new UINT [nParamCount];
			pIndex[0] = item.m_pIndicatorData->Belong2Entity;
			DWORD dw = arrParaPos[i];
			if (dw)
			{
				UINT nBegin, nEnd;
				nBegin = LOWORD(dw);
				nEnd = HIWORD(dw);

				ASSERT (nBegin <= nEnd);
				for (UINT i = nBegin; i <= nEnd; i++)
				{
					pIndex[1 + i - nBegin] = i;
				}
			}
		}
		m_table_indicator.AddIndicatorColumn(item.GetId(), IndicatorWithParameter::DataType2Enum(item.GetDataType()), pIndex, nParamCount);
		if (pIndex) delete pIndex;
	}

	m_table.AttachColumn(m_table_indicator, m_table_indicator.GetColCount() - (int)size(), m_table_indicator.GetColCount() - 1);
	for (UINT i = 0; i < (UINT)size(); i++)
	{
		m_table.SetTitle(i, (*(begin() + i)).GetName());
	}

	Tx::Data::LogicalBusiness business;
	business.GetData(m_table_indicator,true);

	hourglass.Hide();
}

void IndicatorWithParameterArray::BuildTableIndicator()
{
	m_table_indicator.Clear();
	m_table.Clear();
	UINT nIndex = 3;
	//// ��������ID������
	m_table_indicator.AddCol(Tx::Core::dtype_int4);
	m_table_indicator.AddCol(Tx::Core::dtype_int4);
	m_table_indicator.AddCol(Tx::Core::dtype_int4);

	std::vector<DWORD> arrParaPos;
	nIndex = 3;

	iterator iter = begin();
	/*const */IndicatorWithParameter& item = *iter;
	if (item.m_nDirty == 1)
	{
		DWORD dw = MAKELONG(nIndex, nIndex);
		for (/*const_*/iterator iter = begin(); iter != end(); iter++)
			arrParaPos.push_back(dw);
		m_table_indicator.AddParameterColumn(Tx::Core::dtype_int8, false);

	}
	else
	{
		for (/*const_*/iterator iter = begin(); iter != end(); iter++)
		{
			DWORD dw = MAKELONG(nIndex, nIndex + item.GetParameterCount() - 1-1);
			arrParaPos.push_back(dw);
		}
		for (int i=0;i<item.GetParameterCount()-1;i++)
		{
			m_table_indicator.AddParameterColumn((Tx::Core::Data_Type)item.GetParameterType(i));

		}
	}
	//}
	ASSERT (arrParaPos.size() == size());
	// �������ָ����
	for (UINT i = 0; i < (UINT)size(); i++)
	{
		const IndicatorWithParameter& item = *(begin() + i);
		if (item.GetParameterCount() < 1)
		{
			ASSERT (0);
			continue;
		}
		UINT nParamCount = 0;
		UINT * pIndex = NULL;
		if (item.m_nDirty == 1)
		{
			nParamCount = 1;
			pIndex = new UINT [nParamCount];
			DWORD dw = arrParaPos[i];
			UINT nBegin, nEnd;
			nBegin = LOWORD(dw);
			nEnd = HIWORD(dw);
			ASSERT (nBegin == nEnd);
			pIndex[0] = nBegin;
		}
		else
		{
			nParamCount = item.GetParameterCount();
			pIndex = new UINT [nParamCount];
			pIndex[0] = item.m_pIndicatorData->Belong2Entity;
			DWORD dw = arrParaPos[i];
			if (dw)
			{
				UINT nBegin, nEnd;
				nBegin = LOWORD(dw);
				nEnd = HIWORD(dw);
				//ASSERT (nBegin <= nEnd);
				for (UINT i = nBegin; i <= nEnd; i++)
				{
					pIndex[1 + i - nBegin] = i;
				}
			}
		}
		m_table_indicator.AddIndicatorColumn(item.GetId(), IndicatorWithParameter::DataType2Enum(item.GetDataType()), pIndex, nParamCount);
		if (pIndex) delete pIndex;
	}

	m_table.AttachColumn(m_table_indicator, m_table_indicator.GetColCount() - (int)size(), m_table_indicator.GetColCount() - 1);
	for (UINT i = 0; i < (UINT)size(); i++)
	{
		m_table.SetTitle(i, (*(begin() + i)).GetName());
	}
}
//void SetIWAP(int nBlockId = 0);
//void SetIWAP(std::vector<int>& arrIID);
//void GetData(IndicatorWithParameterArray& IWPA,bool bFull = false);
IndicatorFile::IndicatorFile(void)
{
}

IndicatorFile::~IndicatorFile(void)
{
}

IndicatorFile* IndicatorFile::GetInstance()
{
	static IndicatorFile inst;
	return &inst;
}

void IndicatorFile::SetIWAP(IndicatorWithParameterArray& IWPA,int nMenuId /* = 0 */)
{
	std::vector<int> arrIID;
	Tx::Data::MenuItemMapData* pMenuItemMapData=Tx::Data::MenuItemMapData::GetInstance();
	pMenuItemMapData->GetIndicatorByMenuItemId(arrIID,nMenuId);
	SetIWAP(IWPA,arrIID);
	pMenuItemMapData=NULL;
}

void IndicatorFile::SetIWAP(Tx::Business::IndicatorWithParameterArray &IWPA, std::vector<int> &arrIID)
{
	UINT nSize=arrIID.size();
	for (UINT i=0;i<nSize;i++)
		IWPA.push_back(IndicatorWithParameter(arrIID[i]));	//with default parameter
															//you should change the parameter manually after this
}

void IndicatorFile::SetParameter(IndicatorWithParameterArray& IWPA,UINT nIndex,std::vector<Tx::Core::VariantData>& arrParam)
{
	IWPA[nIndex].SetParameterValue(arrParam);
}

void IndicatorFile::GetData(IndicatorWithParameterArray& IWPA,std::vector<int>& arrSecurityId,bool bFull /* = false */)
{
	if (bFull)
		IWPA.StatFull();
	else
	{
		//if (arrSecurityId.size()!=IWPA.size())
		//{
		//	ASSERT(0);
		//	return;
		//}
		IWPA.Stat(arrSecurityId);
	}
}

}
}