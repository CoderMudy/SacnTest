#include "StdAfx.h"
#include "StatReqAndRsp.h"

CStatRequest::CStatRequest()
{
	m_strCommand = _T("");
	m_strCommandDescrip = _T("");
	m_iReqSeqNumber = 0;
}

CStatRequest::~CStatRequest()
{
}

BOOL CStatRequest::IsValid()
{
	if (m_strCommand.IsEmpty() || m_iReqSeqNumber < 0)
	{
		return FALSE;
	}
	return TRUE;
}

CStatResponse::CStatResponse()
{
	Empty();
}
CStatResponse::~CStatResponse()
{
	this->m_arrToBeCheckedParams.clear();
}

void CStatResponse::Empty()
{
	this->m_arrToBeCheckedParams.clear();
	this->m_iResult = 0;
	this->m_strCommand = _T("");
	this->m_iReqSeqNumber = -1;
}