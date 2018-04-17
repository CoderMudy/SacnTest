#include "StdAfx.h"
#include "StatDataTransfer.h"
#include "../../../core/driver/clientfileengine/base/httpDnAndUp.h"
#include "../../../core/driver/clientfileengine/base/zip/zipWrapper.h"

CStatDataTransfer::CStatDataTransfer(const CString& strRequestPath)
{
	this->m_strRequestPath = strRequestPath;
}
CStatDataTransfer::~CStatDataTransfer()
{
}
BOOL CStatDataTransfer::ReqExec(const CString& strReqXML, CString& strRspXML)
{
	if (!OpRequestExec(strReqXML,strRspXML))
	{
		return FALSE;
	}
	return TRUE;
}
BOOL CStatDataTransfer::OpRequestExec(const CString& strReqXML, CString& strResult)
{
	CString strReqXMLFormated = _T("");
	if (strReqXML.IsEmpty())
	{
		return FALSE;
	}

	LPBYTE lpPost = NULL;
	UINT postSize = 0;
#ifndef _UNICODE
	CStringW wXml(strReqXML);	
	LPCWSTR src = wXml;
	lpPost = (LPBYTE)src;
	postSize = wXml.GetLength() * sizeof(WCHAR);
#else
	LPCTSTR src = strReqXML;
	lpPost = (LPBYTE)src;
	postSize = strXML.GetLength() * sizeof(WCHAR);
#endif


	CString url = this->m_strRequestPath;
	CONST INT iTimeOut = 5000;

	Tx::Drive::Http::CSyncUpload upLoad;
	int nRetry = 3;
	while (nRetry > 0)
	{
		if (upLoad.Post(url, lpPost, postSize, NULL, iTimeOut))
		{
			break;
		}
		nRetry--;
	}
	if (nRetry <= 0)
	{
		return FALSE;
	}

	Tx::Drive::Http::CHttpRsp rsp = upLoad.Rsp();
	Tx::Drive::Mem::MemSlice data(rsp.Body());
	this->UnPacket(data);//解压
	LPBYTE lpData = data.DataPtr();
	UINT nSize = data.Size();

	CString xml;
	this->FmtRspXml(xml, lpData, nSize);//将返回结果转成字符串
	if (xml.IsEmpty())
	{
		return FALSE;
	}

	strResult = xml;
	return TRUE;
}
BOOL CStatDataTransfer::UnPacket(Tx::Drive::Mem::MemSlice &data) CONST
{
	UINT nSize = data.Size();
	LPBYTE lpData = data.DataPtr();
	//头四个字节是原包大小
	UINT nTotal = *(reinterpret_cast<UINT*>(lpData));
	Tx::Drive::Mem::MemSlice temp(nTotal);
	LPBYTE lpTemp = temp.DataPtr();	
	if ( lpTemp == NULL )
	{
		Tx::Drive::IO::CLog::Write(_T("fail to alloc mem before unpacket kline-data."));
		return FALSE;
	}
	if ( !Tx::Drive::IO::Zip::CZipWrapper::MemUnZip(
		lpTemp, nTotal, lpData + sizeof(UINT), nSize - sizeof(UINT)) )
	{
		Tx::Drive::IO::CLog::Write(_T("fail to unzip kline-data."));
		return FALSE;
	}
	data = temp;

	return TRUE;
}
//格式化Xml字符串
void CStatDataTransfer::FmtRspXml(CString &xml, LPBYTE lpData, UINT nSize)
{
	//ASSERT( lpData != NULL && nSize != 0 );
	if (lpData == NULL)
	{
		return;
	}
	UINT count = nSize / sizeof(WCHAR);
	if ( count == 0 )
	{
		xml = _T("");
		return;
	}
	nSize = count * sizeof(WCHAR);
#ifdef _UNICODE
	LPTSTR buf = xml.GetBuffer(count + 1);
	LPCTSTR src = reinterpret_cast<LPCTSTR>(lpData);
	_tcsncpy_s(buf, count + 1, src, count);
	xml.ReleaseBuffer(count);	
#else
	/*CStringW wXml;
	LPWSTR dst = wXml.GetBuffer(count + 1);
	LPCWSTR src = reinterpret_cast<LPCWSTR>(lpData);
	wcsncpy_s(dst, count + 1, src, count);
	wXml.ReleaseBuffer(count);
	CString strXml(wXml);
	xml = strXml;*/
	
	/*LPCWSTR src = reinterpret_cast<LPCWSTR>(lpData);
	int lengthOfDst = WideCharToMultiByte(CP_ACP, 0, src, -1, NULL, 0, NULL, NULL);
	CString strXml;
	LPSTR dst = strXml.GetBuffer(lengthOfDst);
	WideCharToMultiByte(CP_ACP, 0, src, count+1, dst, lengthOfDst, NULL, NULL);   
	DWORD dw = GetLastError(); 
	strXml.ReleaseBuffer(lengthOfDst);
	xml = strXml;
	*/


	LPBYTE lpDataDst = new BYTE[nSize + sizeof(WCHAR)];
	memcpy_s(lpDataDst, nSize +2,  lpData, nSize);
	lpDataDst[nSize] = lpDataDst[nSize+1] = 0;
	LPCWSTR src = reinterpret_cast<LPCWSTR>(lpDataDst);

	int lengthOfMbs = WideCharToMultiByte( CP_ACP, 0, src, -1, NULL, 0, NULL, NULL);
	CString strXml;
    lengthOfMbs = WideCharToMultiByte( CP_ACP, 0, src, -1, strXml.GetBuffer(lengthOfMbs), lengthOfMbs, NULL, NULL);
	strXml.ReleaseBuffer();
    xml = strXml;
	delete lpDataDst;
    lpDataDst = NULL;
#endif
}