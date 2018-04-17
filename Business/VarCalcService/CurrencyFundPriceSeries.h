#ifndef _currencyfundPriceseries_h_1987192
#define _currencyfundPriceseries_h_1987192

#include "PriceSeriesGenerator.h"

class CCurrencyFundPriceSeries : public CPriceSeriesGenerator
{
public:
	CCurrencyFundPriceSeries();
	CCurrencyFundPriceSeries(long lLatestDate, int iSampleCount);
	virtual ~CCurrencyFundPriceSeries();
public:
	virtual double GetLatestPrice(long lEntityID);
	virtual int	   GetSeries(long lEntityID, std::vector<double> &arrSeries);
	virtual CString ErrorCodeToString(int iError);
protected:
	double GetCFundProfitPerShare(long lEntityId, long lDate);
};
#endif