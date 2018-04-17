#ifndef _stockPriceseries_h_1987192
#define _stockPriceseries_h_1987192

#include "PriceSeriesGenerator.h"

class CStockPriceSeries : public CPriceSeriesGenerator
{
public:
	CStockPriceSeries();
	CStockPriceSeries(long lLatestDate, int iSampleCount);
	virtual ~CStockPriceSeries();
public:
	virtual double GetLatestPrice(long lEntityID);
	virtual int	   GetSeries1(long lEntityID, std::vector<double> &arrSeries);
	virtual int	   GetSeries(long lEntityID, std::vector<double> &arrSeries);
	virtual CString ErrorCodeToString(int iError);
};
#endif