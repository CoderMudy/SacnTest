#include "stdafx.h"
#include "FileLoadPriceSeries.h"

#include "AdapterTools/VARTypeChange.h"
#include "AdapterTools/VARDateTimeAdapter.h"

#include<fstream>

CFileLoadPriceSeries::CFileLoadPriceSeries()
{
	m_lLatestDate = 19800101;
	m_iSampleCount = 100;
}

CFileLoadPriceSeries::CFileLoadPriceSeries(long lLatestDate, int iSampleCount) : CPriceSeriesGenerator(lLatestDate, iSampleCount)
{
}

CFileLoadPriceSeries::~CFileLoadPriceSeries()
{
}
CString CFileLoadPriceSeries::GetDataFilePath(long lEntityID)
{
	CString strPath = this->m_strDataDirPath;
	int iLastCharPos = strPath.GetLength() - 1;
	TCHAR cLast = strPath.GetAt(iLastCharPos);
	if (cLast != '\\' && cLast != '/')
	{
		strPath += _T("/"); 
	}	
	CString strFileName;
	strFileName.Format(_T("%d.txt"), lEntityID);
	strPath += strFileName;
	return strPath;
}

void CFileLoadPriceSeries::SetDataDirPath(CString &strPath)
{
	this->m_strDataDirPath = strPath;
}

double CFileLoadPriceSeries::GetLatestPrice(long lEntityID)
{
	double dLatestPrice = 0.0;

	std::ifstream ifs(this->GetDataFilePath(lEntityID));
    if(!ifs.is_open())
    {
		return dLatestPrice;
	}

	CString strSep = _T("\t");
	CVARTypeChange *pType = CVARTypeChange::GetInstance();

	CString strDate;
	CString str;
	int iDaysIdx = 0;
	while (!ifs.eof() && iDaysIdx < this->m_iSampleCount) 
	{
		ifs.getline(str.GetBuffer(1024), 1024);
		str.ReleaseBuffer();
		if (0 == str.Find('#'))
		{
			continue;
		}
		str.Replace(_T("\r\n"),_T(""));	
		str += strSep;

		CString strTemp;
		int iStart = 0;
		int iEnd = 0;
		vector<CString> arrContentStr;
		while(0 < (iEnd = str.Find(strSep, iStart)))
		{
			strTemp = str.Mid(iStart, iEnd - iStart);
			iStart = iEnd +1;
			if (strTemp.IsEmpty())
			{
				continue;
			}
			strTemp.Trim();
			arrContentStr.push_back(strTemp);
		}

		if (!this->CheckFileFormatPerLine(arrContentStr))
		{
			return false;
		}

		strDate = arrContentStr[0];
		CVARDateTimeAdapter dt(strDate);
		if (dt.GetDate() > this->m_lLatestDate)
		{
			continue;
		}

		dLatestPrice = pType->AM_StrToDouble(arrContentStr[1]);
		break;
	}

	ifs.close();

	return dLatestPrice;
}

int	CFileLoadPriceSeries::GetSeries(long lEntityID, std::vector<double> &arrSeries)
{
	arrSeries.clear();

	std::ifstream ifs(this->GetDataFilePath(lEntityID));
    if(!ifs.is_open())
    {
		return -eDATA_ERROR;
	}

	CString strSep = _T("\t");

	CVARTypeChange *pType = CVARTypeChange::GetInstance();

	CString strDate;
	double dPrice = 0.0;
	CString str;
	int iDaysIdx = 0;
	while (!ifs.eof() && iDaysIdx < this->m_iSampleCount) 
	{
		ifs.getline(str.GetBuffer(1024), 1024);
		str.ReleaseBuffer();
		if (0 == str.Find('#'))
		{
			continue;
		}
		str.Replace(_T("\r\n"),_T(""));	
		str += strSep;

		CString strTemp;
		int iStart = 0;
		int iEnd = 0;
		vector<CString> arrContentStr;
		while(0 < (iEnd = str.Find(strSep, iStart)))
		{
			strTemp = str.Mid(iStart, iEnd - iStart);
			iStart = iEnd +1;
			if (strTemp.IsEmpty())
			{
				continue;
			}
			strTemp.Trim();
			arrContentStr.push_back(strTemp);
		}

		if (!this->CheckFileFormatPerLine(arrContentStr))
		{
			return false;
		}

		strDate = arrContentStr[0];
		CVARDateTimeAdapter dt(strDate);
		if (dt.GetDate() > this->m_lLatestDate)
		{
			continue;
		}

		dPrice = pType->AM_StrToDouble(arrContentStr[1]);
		
		arrSeries.push_back(dPrice);
		iDaysIdx++;

	}

	ifs.close();

	if ((int)arrSeries.size() < this->m_iSampleCount)
	{
		return -eDATA_UNENOUGH;
	}

	return eSUCESS;
}

CString CFileLoadPriceSeries::ErrorCodeToString(int iError)
{
	return CPriceSeriesGenerator::ErrorCodeToString(iError);
}

bool CFileLoadPriceSeries::CheckFileFormatPerLine(std::vector<CString>& arrLineContents)
{
	if (arrLineContents.size() < 2)
	{
		return false;
	}
	CString strDate = arrLineContents.at(0);
	strDate += _T(" 00:00:00");
	if (!CVARDateTimeAdapter::CheckAndAdjustFormat(strDate))
	{
		return false;
	}

	CString strPrice = arrLineContents.at(1);
	if (strPrice.IsEmpty()) 
	{
		return false;
	}

	bool bValid = true;
	TCHAR chValue = 0;
	int iPointCount = 0;
	for (int i = 0; i < strPrice.GetLength() && bValid; i++)
	{
		chValue= strPrice.GetAt(i);
		chValue == 46 ? (++iPointCount > 1 ? bValid = false : 0) : ((chValue > 47 && chValue < 58) ? 0 : bValid = false);
	}

	return bValid;
}