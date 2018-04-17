#ifndef _varcalcparam_h_1987192
#define _varcalcparam_h_1987192

#include "../../Business.h"

namespace Tx
{
	namespace VAR
	{
class BUSINESS_EXT CVarCalcParam
{
public:
	CVarCalcParam();
	CVarCalcParam(int iPLType, int iHoldingDays, double dAlpha,	double dLmd, int iSimulationTimes, int iSamplesCount);
	virtual ~CVarCalcParam();
public:
	bool IsValid();
	CVarCalcParam GetDefaultParam();

	int GetPLType() const;
	int GetHoldingDays() const;
	double GetAlpha() const;
	double GetLmd() const;
	int GetSimulationDays() const;

	int GetSamplesCount() const;

protected:
	bool Load();
private:
	int m_iPLType;//0-���棬1-�ٷֱ������ʣ�2-����������
	int m_iHoldingDays;//δ����������
	double m_dAlpha;//���Ŷ�,<0.5,����0.05
	double m_dLmd;//���ȣ�����ȱ�˥��Ȩ��, 0 < m_dLmd < 1 
	int m_iSimulationTimes;//Monte-Carlo���ģ�����

	int m_iSamplesCount;//��������(������)
};
	}
}
#endif