#ifndef _testlog_h_1987192
#define _testlog_h_1987192

class CTestLog
{
public:
	CTestLog();
	CTestLog(CString& strPath);
	virtual ~CTestLog();
public:
	void WiriteToLogFile(CString& strText);
	void ClearLog();
protected:
	CString m_strLogPath;
};

#endif