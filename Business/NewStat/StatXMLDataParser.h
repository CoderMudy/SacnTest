#pragma once

#include "..\..\..\Core\Driver\Table_Display.h"
#include "StatReqAndRsp.h"

class CStatXMLDataParser
{
public:
	CStatXMLDataParser(CString& strDescrip);
	virtual ~CStatXMLDataParser();
public:
	CString XMLReqFormat(CStatRequest& reqStat);
	BOOL XMLParse(const CString& strXML, CStatResponse& rspStat, Tx::Core::Table_Display &resTable);
protected:
	CString m_strDescrip;
};


