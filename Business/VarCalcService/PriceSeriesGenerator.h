#ifndef _Priceseriesgenerator_h_1987192
#define _Priceseriesgenerator_h_1987192

#include <vector>
using std::vector;

class CPriceSeriesGenerator
{
public:
	CPriceSeriesGenerator();
	CPriceSeriesGenerator(long lLatestDate,int iSampleCount);
	virtual ~CPriceSeriesGenerator();

public:
	enum emError
	{
		eSUCESS = 0,
		eDATA_UNENOUGH,
		eLOGICAL_ERROR,
		eDATA_ERROR,
		eERROR_END,
	};
	virtual double GetLatestPrice(long lEntityID);
	virtual int	   GetSeries(long lEntityID, std::vector<double> &arrSeries);
	virtual CString ErrorCodeToString(int iError);
protected:
	//根据样本最后日期和样本数取对应上证指数交易日序列
	bool GetShIndexTradeDateSeries(std::vector<int>& arrTradeDates);
protected:
	long m_lLatestDate;
	int m_iSampleCount;
};

#endif