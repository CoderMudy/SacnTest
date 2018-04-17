#pragma once

#include "../../../core/driver/clientfileengine/base/memSlice.h"

#include <vector>
#include <utility>
using std::vector;
using std::pair;

class CStatDataTransfer
{
public:
	CStatDataTransfer(const CString& strRequestPath);
	~CStatDataTransfer();
public:
	BOOL ReqExec(const CString& strReqXML, CString& strRspXML);
protected:
	BOOL OpRequestExec(const CString& strReqXML, CString& strResult);
	void FmtRspXml(CString &xml, LPBYTE lpData, UINT nSize);
	BOOL UnPacket(Tx::Drive::Mem::MemSlice &data) CONST;
protected:
	CString m_strRequestPath;
};


