#ifndef _bondPriceseries_h_1987192
#define _bondPriceseries_h_1987192

#include "PriceSeriesGenerator.h"

 struct _VAR_BondInfo{
	 long m_lSecuritiesId;
	 double m_dParRate;
	 double m_dHoldYear;
	 int m_iInterestType;
	 long m_lBeginDate;
	 long m_lEndDate;
	 int m_iPayInterestFrequence;
	 double m_dParValue;
	 int m_iBondType;
	 double m_dIssuePrice;
};

typedef struct _VAR_BondInfo VAR_BondInfo;
class CBondPriceSeries : public CPriceSeriesGenerator
{
public:
	CBondPriceSeries();
	CBondPriceSeries(long lLatestDate, int iSampleCount);
	virtual ~CBondPriceSeries();
public:
	virtual double GetLatestPrice(long lEntityID);
	virtual int	   GetSeries(long lEntityID, std::vector<double> &arrSeries);
	virtual CString ErrorCodeToString(int iError);

protected:
	double GetBondTotalPrice(long lEntityID, long lDate);
	bool GetBondInfo(long lSecurities, VAR_BondInfo &infoBond);
	double CalcBondInterestPerShare(long lSecurities, long lDate);
	double GetBondCashFlowYearRate(long lSecurities, long lDate);
	double GetBondEndDayNetValue(long lSecurities);
	long GetBondNearestInterestPayDate(long lSecurities, long lDate);
};
#endif