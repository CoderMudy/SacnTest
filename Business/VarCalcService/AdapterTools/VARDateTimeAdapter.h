#pragma once
//#include <atltime.h>


//时间转化器类，时间字串格式为: yyyy-mm-dd hh:mm:ss
class CVARDateTimeAdapter
{
public:
	//CVARDateTimeAdapter();
	~CVARDateTimeAdapter();
	CVARDateTimeAdapter(long lDate, long lTime);
	CVARDateTimeAdapter(const CString &strDateTime);
	CVARDateTimeAdapter(const CTime &tmDateTime);

	static bool CheckAndAdjustFormat(CString &strDt);

	long GetDate()
	{
		return m_lDate;
	}
	long GetTime()
	{
		return m_lTime;
	}
	//long Get
	CString GetDateTimeStr(); 
	CTime GetDateTime();

	long GetYear(void)   const     { return m_lDate/10000; }
	long GetMonth(void)  const     { return m_lDate/100 - (m_lDate/10000) * 100; }
	long GetDay(void)    const     { return m_lDate - (m_lDate/100)*100; }

	long GetHour(void)   const     { return m_lTime/10000; }
	long GetMinute(void) const     { return m_lTime/100 - (m_lTime/10000) * 100; }  
	long GetSecond(void) const     {  return m_lTime - (m_lTime/100)*100; }

	void NextDay();
	void LastDay();
	void LastSecond();
	void NextSecond();
	void AddMonth(int iMonthAdd);
	void AddDay(int iDayAdd);
	void AddSecond(int iSecondAdd);
	long GetDayDiff(const long lDate);

	friend bool operator > (const CVARDateTimeAdapter& eventNew, const CVARDateTimeAdapter& eventOld);
	friend bool operator < (const CVARDateTimeAdapter& eventNew, const CVARDateTimeAdapter& eventOld);
	friend bool operator == (const CVARDateTimeAdapter& eventNew,const CVARDateTimeAdapter& eventOld);
	friend bool operator >= (const CVARDateTimeAdapter& eventNew,const CVARDateTimeAdapter& eventOld);
	friend bool operator <= (const CVARDateTimeAdapter& eventNew,const CVARDateTimeAdapter& eventOld);

private:
	long m_lDate;
	long m_lTime;
	CString m_strDateTime;
};
