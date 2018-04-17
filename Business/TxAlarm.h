#pragma once

class AFX_EXT_CLASS TxAlarm
{
public:
	TxAlarm(void);
public:
	~TxAlarm(void);
private:
	int 	m_nAlarmId;
	int		m_nType;
	int		m_nDate;
	int 	m_nTime;
	int		m_nSiren;
	int		m_nValid;
	CString	m_strTip;
public:
	virtual void Serialize(CArchive& ar);
public:
	int		SetAlarmId( int nId ){ return m_nAlarmId = nId;}
	int		SetAlarmType( int nType ){ return m_nType = nType; }
	int		SetAlarmDate( int nDate ){ return m_nDate = nDate; }
	int		SetAlarmTime( int nTime ){ return m_nTime = nTime; }
	int		SetAlarmSiren( int nSiren ){ return m_nSiren = nSiren; }
	int		SetAlarmValid( int nValid ){ return m_nValid = nValid; }
	CString	SetAlarmTip( CString strTip ){ return m_strTip = strTip; }
	int		GetAlarmId(){ return m_nAlarmId;}
	int		GetAlarmType(){ return m_nType; }
	int		GetAlarmDate(){ return m_nDate; }
	int		GetAlarmTime(){ return m_nTime; }
	int		GetAlarmSiren(){ return m_nSiren; }
	int		GetAlarmValid(){ return m_nValid; }
	CString	GetAlarmTip(){ return m_strTip; }
public:
	TxAlarm	operator= ( const TxAlarm& Alarm )
	{
		this->m_nAlarmId = Alarm.m_nAlarmId;
		this->m_nType = Alarm.m_nType;
		this->m_nDate = Alarm.m_nDate;
		this->m_nTime = Alarm.m_nTime;
		this->m_nSiren = Alarm.m_nSiren;
		this->m_nValid = Alarm.m_nValid;
		this->m_strTip = Alarm.m_strTip;
		return *this;
	}
};
