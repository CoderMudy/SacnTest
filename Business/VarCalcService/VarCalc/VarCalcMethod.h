#ifndef _varcalcmethod_h_1987192
#define _varcalcmethod_h_1987192

#include <vector>
#include <utility>
using std::vector;
using std::pair;

#include "../../../../Core/TxMath/ArrayMatrix.h"

#include "VarCalcDataSource.h"
#include "VarCalcParam.h"
namespace Tx
{
	namespace VAR
	{
class CCommonMathMethod
{
public:
	CCommonMathMethod();
	virtual ~CCommonMathMethod();
public:
	static 	bool PercentileFromDencity(double dAlpha, Tx::Core::CArrayMatrix &matrixM2, double &dVar, double &dCVar);
	static 	bool GenerateNormalRand(Tx::Core::CArrayMatrix& matrixNormalRand);
};

class CVarCalcMethod
{
public:
	CVarCalcMethod(CVarCalcParam *pParam, CVarDataSource *pDataSrc);
	virtual ~CVarCalcMethod();
public:
	virtual bool Calculate(double& dVar, double& dCVar);
protected:
	bool Covar(Tx::Core::CArrayMatrix& matrixCov, Tx::Core::CArrayMatrix& matrixAvg);//avg加权，协方差也以同样方式加权，此种方法结果比较正常
	bool Covar2(Tx::Core::CArrayMatrix& matrixCov, Tx::Core::CArrayMatrix& matrixAvg);//avg不加权， 协方差加权
	bool Covar3(Tx::Core::CArrayMatrix& matrixCov, Tx::Core::CArrayMatrix& matrixAvg);//将样本按衰减因子加权

protected:
	CVarCalcParam* m_pVarCalcParam;
	CVarDataSource* m_pVarDataSource;
};

class CVarNormalMethod : public CVarCalcMethod
{
public:
	CVarNormalMethod(CVarCalcParam *pParam, CVarDataSource *pDataSrc);
	virtual ~CVarNormalMethod();
public:
	virtual bool Calculate(double& dVar, double& dCVar);
protected:
	double Percentile(double dAlpha);
};

class CHistorSimulationMethod : public CVarCalcMethod
{
public:
	CHistorSimulationMethod(CVarCalcParam *pParam, CVarDataSource *pDataSrc);
	virtual ~CHistorSimulationMethod();
public:
	virtual bool Calculate(double& dVar, double& dCVar);
};

class CMonteCarloMethod : public CVarCalcMethod
{
public:
	CMonteCarloMethod (CVarCalcParam *pParam, CVarDataSource *pDataSrc);
	virtual ~CMonteCarloMethod ();
public:
	virtual bool Calculate(double& dVar, double& dCVar);
protected:
	bool CovMatrixCholesky(Tx::Core::CArrayMatrix& matrixCov);
	bool GenerateNormalArrayWithCov(const Tx::Core::CArrayMatrix& matrixCov, Tx::Core::CArrayMatrix& matrixResult);
};
	
}
}
#endif