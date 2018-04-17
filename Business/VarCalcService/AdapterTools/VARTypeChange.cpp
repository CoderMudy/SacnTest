#include "StdAfx.h"
#include "VARTypeChange.h"

CString StrToInt(const CString& sIn, int& ret)
{
	size_t sizeIn = strlen(sIn) + 1;
	size_t sizeOut = 0;
	wchar_t wcstring[1024];
	mbstowcs_s(&sizeOut, wcstring, sizeIn, sIn, _TRUNCATE);
	wchar_t *endptr = NULL;
	ret = _wcstol_l(wcstring, &endptr, 0, 0);
	return CString(endptr);
}

CString StrToInt64(const CString& sIn, __int64& ret)
{
	size_t sizeIn = strlen(sIn) + 1;
	size_t sizeOut = 0;
	wchar_t wcstring[1024];
	mbstowcs_s(&sizeOut, wcstring, sizeIn, sIn, _TRUNCATE);
	wchar_t *endptr = NULL;
	ret = _wcstoi64_l(wcstring, &endptr, 0, 0);
	return CString(endptr);
}

CString StrToDouble(const CString& sIn, double& ret)
{
	size_t sizeIn = strlen(sIn) + 1;
	size_t sizeOut = 0;
	wchar_t wcstring[1024];
	mbstowcs_s(&sizeOut, wcstring, sizeIn, sIn, _TRUNCATE);
	wchar_t *endptr = NULL;
	ret = wcstod(wcstring, &endptr);
	return CString(endptr);
}

CVARTypeChange::CVARTypeChange(void)
{
}

CVARTypeChange::~CVARTypeChange(void)
{
}

CVARTypeChange* CVARTypeChange::GetInstance(void)
{
	static CVARTypeChange tInstance;
	return &tInstance;
}

DOUBLE CVARTypeChange::AM_StrToDouble(const CString &strSrc)
{
	DOUBLE dDst = 0.000000f;
	StrToDouble(strSrc, dDst);
	return  dDst;
}

INT CVARTypeChange::AM_StrToInt(const CString &strSrc)
{
	INT iDst = 0;
	StrToInt(strSrc, iDst);
	return  iDst;
}
LONG CVARTypeChange::AM_StrToLONG(const CString &strSrc)
{
	INT lDst = 0;
	StrToInt(strSrc, lDst);
	return (LONG)lDst;
}

FLOAT CVARTypeChange::AM_StrToFloat(const CString &strSrc)
{
	DOUBLE dDst = 0.000f;
	StrToDouble(strSrc, dDst);
	return  (FLOAT)dDst;
}

void CVARTypeChange::FormatFloatNumStr(CString &strFloat, CONST SHORT iDigit)
{	
	TCHAR szFormat[32];
	sprintf_s(szFormat, _countof(szFormat), _T("%%.%dlf"), iDigit);
	DOUBLE dValue = atof(strFloat);
	strFloat.Format(szFormat, dValue);
}
