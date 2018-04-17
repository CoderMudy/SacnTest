#include "StdAfx.h"
#include "TxAlarm.h"

TxAlarm::TxAlarm(void)
{
	m_nAlarmId = 0;
	m_nType = 0;
	m_nDate = 0;
	m_nTime = 0;
	m_nSiren = 0;
	m_nValid = 0;
	m_strTip = _T("");
}

TxAlarm::~TxAlarm(void)
{
}

void TxAlarm::Serialize(CArchive& ar)
{
	if(ar.IsStoring())
	{
		ar<<m_nAlarmId;
		ar<<m_nType;
		ar<<m_nDate;
		ar<<m_nTime;
		ar<<m_nSiren;
		ar<<m_nValid;
		ar<<m_strTip;
	}
	else
	{
		ar>>m_nAlarmId;
		ar>>m_nType;
		ar>>m_nDate;
		ar>>m_nTime;
		ar>>m_nSiren;
		ar>>m_nValid;
		ar>>m_strTip;
	}	
}