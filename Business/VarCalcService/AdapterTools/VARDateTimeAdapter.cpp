#include "stdafx.h"
#include "VARDateTimeAdapter.h"
//TXAM_NAMESPACE_DEF_TXAM_GL_HEADER

CVARDateTimeAdapter::~CVARDateTimeAdapter()
{
}

CVARDateTimeAdapter::CVARDateTimeAdapter(long lDate, long lTime)
{
	m_lDate = lDate;
	m_lTime =lTime;
	m_strDateTime = GetDateTimeStr();
}
CVARDateTimeAdapter::CVARDateTimeAdapter(const CString &strDateTime)
{
	m_lDate = 0;
	m_lTime = 0;
	CString strDateTime1 = strDateTime;
	if (CheckAndAdjustFormat(strDateTime1))
	{
		m_strDateTime = strDateTime1;
		long lYear = _ttol(m_strDateTime.Mid(0,4));
		long lMonth = _ttol(m_strDateTime.Mid(5,2));
		long lDay = _ttol(m_strDateTime.Mid(8,2));
		long lHour = _ttol(m_strDateTime.Mid(11,2));
		long lMinute = _ttol(m_strDateTime.Mid(14,2));
		long lSecond = _ttol(m_strDateTime.Mid(17,2));

		m_lDate = lYear*10000 + lMonth*100 + lDay;
		m_lTime = lHour*10000 + lMinute*100 + lSecond;
	}
}

CVARDateTimeAdapter::CVARDateTimeAdapter(const CTime &tmDateTime)
{
	m_lDate = (long)(tmDateTime.GetYear()*10000 + tmDateTime.GetMonth()*100 + tmDateTime.GetDay());
	m_lTime = (long)(tmDateTime.GetHour()*10000 + tmDateTime.GetMinute()*100 + tmDateTime.GetSecond());
	m_strDateTime = GetDateTimeStr(); 
}

CString CVARDateTimeAdapter::GetDateTimeStr()
{ 
	CString str;
	if (0 == m_lDate && 0 == m_lTime)
	{
		str = _T("");
	}
	else
	{
		str.Format(_T("%04ld-%02ld-%02ld %02ld:%02ld:%02ld"), 
		GetYear(), GetMonth(), GetDay(),
		GetHour(), GetMinute(), GetSecond());
	}
	return str;
}

CTime CVARDateTimeAdapter::GetDateTime()
{
	CTime time( GetYear(), GetMonth(), GetDay(), GetHour(), GetMinute(), GetSecond(), 0);
	return time;
}

//ÒÔºóÔÙ²¹³ä
bool CVARDateTimeAdapter::CheckAndAdjustFormat(CString &strDt)
{
	if (strDt.GetLength() != _tcslen(_T("2007-10-01 00:00:00")))
	{
		int iPos1 = strDt.Find(_T("-"));
		int iPos2 = strDt.Find(_T("-"), iPos1 + 1);
		int iPos3 = strDt.Find(_T(" "));
		if (2 == (iPos2 - iPos1))
		{
			strDt.Insert(iPos1 + 1, '0');
			iPos2 += 1;
			iPos3 += 1; 
		}

		if (2 == iPos3 - iPos2)
		{
			strDt.Insert(iPos2 + 1, '0');
		}

		if (strDt.GetLength() == _tcslen(_T("2007-10-01 00:00:00")))
		{
			return true;
		}
		iPos1 = strDt.Find(_T(" "));
		iPos2 = strDt.Find(_T(":"));
		if (2 == iPos2 - iPos1)
		{
			strDt.Insert(iPos1 +1, '0');
		}
		if (strDt.GetLength() == _tcslen(_T("2007-10-01 00:00:00")))
		{
			return true;
		}

		return false;
	}
	return true;
}

void CVARDateTimeAdapter::NextDay()
{
	AddDay(1);
}
void CVARDateTimeAdapter::LastDay()
{
	AddDay(-1);
}

void CVARDateTimeAdapter::NextSecond()
{
	AddSecond(1);
}

void CVARDateTimeAdapter::LastSecond()
{
	AddSecond(-1);
}

void CVARDateTimeAdapter::AddMonth(int iMonthAdd)
{
	long lNewMonth = GetMonth() + iMonthAdd;
	long lNewYear = GetYear();
	if (lNewMonth > 12)
	{
		lNewYear = GetYear() +1;
		lNewMonth -= 12 ;
	}
	else if (lNewMonth < 1)
	{
		lNewYear = GetYear() -1;
		lNewMonth += 12;
	}
	

	int iDaysOfMount[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	//GetMonthDaysOfYear
	if (0 == (lNewYear - 2012)%4)
	{
		iDaysOfMount[1] = 29;
	}
	int iDayOfMonth = iDaysOfMount[lNewMonth - 1];
	int iNewDay = GetDay();
	if (iNewDay > iDayOfMonth)
	{
		iNewDay = iDayOfMonth;
	}
	m_lDate = lNewYear * 10000 + lNewMonth * 100 + iNewDay;


	m_strDateTime = GetDateTimeStr(); 
}

void CVARDateTimeAdapter::AddDay(int iDayAdd)
{
	CTime time = GetDateTime();
	CTimeSpan ts( iDayAdd, 0, 0, 0);
	time += ts;
	m_lDate = (long)(time.GetYear()*10000 + time.GetMonth()*100 + time.GetDay());
	m_lTime = (long)(time.GetHour()*10000 + time.GetMinute()*100 + time.GetSecond());
	m_strDateTime = GetDateTimeStr(); 
}
void CVARDateTimeAdapter::AddSecond(int iSecondAdd)
{
	CTime time = GetDateTime();
	CTimeSpan ts( 0, 0, 0, iSecondAdd);
	time += ts;
	m_lDate = (long)(time.GetYear()*10000 + time.GetMonth()*100 + time.GetDay());
	m_lTime = (long)(time.GetHour()*10000 + time.GetMinute()*100 + time.GetSecond());
	m_strDateTime = GetDateTimeStr(); 
}

long CVARDateTimeAdapter::GetDayDiff(const long lDate)
{
	CVARDateTimeAdapter dt(lDate,0);
	CTime timeIn = dt.GetDateTime();
	CTime time = GetDateTime();
	CTimeSpan ts = time - timeIn;

	return (long)ts.GetDays();

}

bool operator > (const CVARDateTimeAdapter& DTNew, const CVARDateTimeAdapter& DTOld)
{
	if (DTNew.m_lDate > DTOld.m_lDate)
	{
		return true;
	}
	else if (DTNew.m_lDate < DTOld.m_lDate)
	{
		return false;
	}

	return DTNew.m_lTime > DTOld.m_lTime;
}

bool operator == (const CVARDateTimeAdapter& DTNew, const CVARDateTimeAdapter& DTOld)
{
	return (DTNew.m_lDate == DTOld.m_lDate) && (DTNew.m_lTime == DTOld.m_lTime) ;

}
bool operator < (const CVARDateTimeAdapter& DTNew, const CVARDateTimeAdapter& DTOld)
{
	return !((DTNew > DTOld) || (DTNew == DTOld));
}
bool operator <= (const CVARDateTimeAdapter& DTNew, const CVARDateTimeAdapter& DTOld)
{
	return !(DTNew > DTOld) ;
}
bool operator >= (const CVARDateTimeAdapter& DTNew, const CVARDateTimeAdapter& DTOld)
{
	return !(DTNew < DTOld);
}
//TXAM_NAMESPACE_DEF_TXAM_GL_END