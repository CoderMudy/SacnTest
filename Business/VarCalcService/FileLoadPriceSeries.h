#ifndef _fileloadpriceseries_h_1987192
#define _fileloadpriceseries_h_1987192

#include "PriceSeriesGenerator.h"
#include <vector>
using std::vector;

class CFileLoadPriceSeries : public CPriceSeriesGenerator
{
public:
	CFileLoadPriceSeries();
	CFileLoadPriceSeries(long lLatestDate, int iSampleCount);
	virtual ~CFileLoadPriceSeries();
public:
	virtual double GetLatestPrice(long lEntityID);
	virtual int	   GetSeries(long lEntityID, std::vector<double> &arrSeries);
	virtual CString ErrorCodeToString(int iError);

	void SetDataDirPath(CString &strPath);
protected:
	CString GetDataFilePath(long lEntityID);

	bool CheckFileFormatPerLine(std::vector<CString>& arrLineContents);
protected:
	CString m_strDataDirPath;
};
#endif