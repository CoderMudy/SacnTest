#pragma once

#include <utility>
#include <vector>
using std::vector;
using std::pair;

class CStatParam
{
public:
	CStatParam(){};
	virtual ~CStatParam(){};
public:
	CString m_strName;
	CString m_strValue;
};

class CStatRequest
{
public:
	CStatRequest();
	virtual ~CStatRequest();
public:
	BOOL IsValid();
public:
	CString m_strCommand;
	CString m_strCommandDescrip;
	int m_iReqSeqNumber;
	std::vector<CStatParam> m_arrStatParams;
};

class CStatResponse
{
public:
	CStatResponse();
	virtual ~CStatResponse();
public:
	void Empty();
public:
	int m_iResult;
	CString m_strCommand;
	int m_iReqSeqNumber;

	std::vector<CStatParam> m_arrToBeCheckedParams;
};


