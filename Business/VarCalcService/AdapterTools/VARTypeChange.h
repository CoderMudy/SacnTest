#include "../../Business.h"
class BUSINESS_EXT CVARTypeChange
{
private:
	CVARTypeChange();
public:
	~CVARTypeChange(void);
	static CVARTypeChange* GetInstance(void);

	DOUBLE AM_StrToDouble(const CString &strSrc);
	INT AM_StrToInt(const CString &strSrc);
	LONG AM_StrToLONG(const CString &strSrc);
	FLOAT AM_StrToFloat(const CString &strSrc);
	void FormatFloatNumStr(CString &strFloat, CONST SHORT iDigit = 2); //格式化字符串形式的浮点数, iDigit为小数点保留位数
};
