#include "stdafx.h"
#include "VarCalcParam.h"

#include "VarCalcGlobal.h"
#include "../AdapterTools/VARDateTimeAdapter.h"
namespace Tx
{
	namespace VAR
	{
CVarCalcParam::CVarCalcParam()
{
	m_iPLType = eLogProfitRate;
	m_iHoldingDays = 10;
	m_dAlpha = 0.05;
	m_dLmd = 0.83;
	m_iSimulationTimes = 1000;
	m_iSamplesCount = 250;
}

CVarCalcParam::CVarCalcParam(int iPLType, int iHoldingDays, double dAlpha,	double dLmd, int iSimulationTimes, int iSamplesCount)
{
	m_iPLType = iPLType;
	m_iHoldingDays = iHoldingDays;
	m_dAlpha = dAlpha;
	m_dLmd = dLmd;
	m_iSimulationTimes = iSimulationTimes;
	m_iSamplesCount = iSamplesCount;
}
CVarCalcParam::~CVarCalcParam()
{
}
CVarCalcParam CVarCalcParam::GetDefaultParam()
{
	CVarCalcParam paramDef;
	return paramDef;
}
bool CVarCalcParam::IsValid()
{
	if (this->m_dLmd < 0.00000001 || this->m_dLmd > 1.00)
	{
		return false;
	}
	if (this->m_iPLType != ePercentProfitRate && this->m_iPLType != eLogProfitRate)
	{
		return false;
	}
	if (this->m_dAlpha != 0.05 && this->m_dAlpha != 0.1 && this->m_dAlpha != 0.01)
	{
		return false;
	}
	if (this->m_iHoldingDays < 1 || this->m_iHoldingDays > 10)
	{
		return false;
	}
	return true;
}
int CVarCalcParam::GetPLType() const
{
	return this->m_iPLType;
}

int CVarCalcParam::GetHoldingDays() const
{
	return this->m_iHoldingDays;
}
double CVarCalcParam::GetAlpha() const
{
	return this->m_dAlpha;
}
double CVarCalcParam::GetLmd() const
{
	return this->m_dLmd;
}
int CVarCalcParam::GetSimulationDays() const
{
	return this->m_iSimulationTimes;
}
int CVarCalcParam::GetSamplesCount() const
{
	return this->m_iSamplesCount;
}
	}
}
