#include "stdafx.h"
#include "TestLog.h"

#include "../AdapterTools/VARDateTimeAdapter.h"
#include <fstream>
#include <time.h>

CTestLog::CTestLog()
{
	m_strLogPath = _T("C://Var/result/result.txt");
}

CTestLog::CTestLog(CString &strPath)
{
	m_strLogPath = strPath;
}
CTestLog::~CTestLog()
{
}
void CTestLog::ClearLog()
{
	if (m_strLogPath.IsEmpty())
	{
		return;
	}

	std::ofstream of;
	of.open(m_strLogPath);
	if (of.is_open())
	{
		of << _T("");
		of.close();
	}
}
void CTestLog::WiriteToLogFile(CString& strText)
{
	if (m_strLogPath.IsEmpty())
	{
		return;
	}
	std::ofstream of;
	of.open(m_strLogPath, std::ios::app);
	if (of.is_open())
	{
		CVARDateTimeAdapter ds(CTime::GetCurrentTime());
		of << ds.GetDateTimeStr();
		of << _T("\r\n");
		of << strText;
		of << _T("\r\n");

		of.close();
	}
}






