#ifndef _opfundPriceseries_h_1987192
#define _opfundPriceseries_h_1987192

#include "PriceSeriesGenerator.h"

class COPFundPriceSeries : public CPriceSeriesGenerator
{
public:
	COPFundPriceSeries();
	COPFundPriceSeries(long lLatestDate, int iSampleCount);
	virtual ~COPFundPriceSeries();
public:
	virtual double GetLatestPrice(long lEntityID);
	virtual int	   GetSeries(long lEntityID, std::vector<double> &arrSeries);
	virtual int	   GetSeries1(long lEntityID, std::vector<double> &arrSeries);
	virtual CString ErrorCodeToString(int iError);
};
#endif