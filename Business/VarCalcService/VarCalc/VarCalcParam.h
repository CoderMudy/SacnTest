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
	int m_iPLType;//0-收益，1-百分比收益率，2-对数收益率
	int m_iHoldingDays;//未来持有天数
	double m_dAlpha;//置信度,<0.5,比如0.05
	double m_dLmd;//公比，按天等比衰减权重, 0 < m_dLmd < 1 
	int m_iSimulationTimes;//Monte-Carlo随机模拟次数

	int m_iSamplesCount;//采样天数(工作日)
};
	}
}
#endif