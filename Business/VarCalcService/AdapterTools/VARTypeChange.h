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
	void FormatFloatNumStr(CString &strFloat, CONST SHORT iDigit = 2); //��ʽ���ַ�����ʽ�ĸ�����, iDigitΪС���㱣��λ��
};
