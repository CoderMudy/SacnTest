#ifndef _vardataprovider_h_1987192
#define _vardataprovider_h_1987192

#include <vector>
#include <utility>
using std::vector;
using std::pair;
#include "VarCalc/VarCalcDataSource.h"

#include "PriceSeriesGenerator.h"

class CVarDataProvider
{
public:
	CVarDataProvider();
	virtual ~CVarDataProvider();
public:
	void Clear();
	bool AssetSetting();

	bool AssertPricesSeries(long lEntityID);

	void SetParams(int iDate, int iSamplesDays);
	bool AddInvestItem(long lEntityID, double dHodingNumber);

	bool UpdateDS(Tx::VAR::CVarDataSource* pDs);

	bool UpdateDSFromFile(CString& strPath, Tx::VAR::CVarDataSource* pDs);

protected:
	bool GetPriceArrayAndLatestPrice(long lEntityID, vector<double>& arrPrice, double &dLatestPrice);
	bool GetPriceArrayFromFile(CString &strPath, long lEntityID, vector<double>& arrPriceRate);

	CPriceSeriesGenerator* CreatePLRateSeriesObj(long lEntityID);
protected:
	int m_iDate;
	int m_iSamplesDays;
	std::vector<std::pair<long, double> > m_arrPortfolio;
};
#endif