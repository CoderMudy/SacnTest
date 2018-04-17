#include "stdafx.H"
#include "CompanyWebsite.h"
#include "..\..\core\core\commonality.h"
#include "txbusiness.h"


CCompanyWebSite::CCompanyWebSite(INT iSecurityId)
: m_id(iSecurityId)
{
}



CCompanyWebSite::~CCompanyWebSite()
{
}



//得到公司主页的URL
void CCompanyWebSite::GetURL(CString &strURL)
{
	strURL = _T("");
	//先得到XML文件路径
	CString strPath;
	LPCTSTR lpFind = NULL;
	GetParam(strPath, lpFind);
	if ( strPath.IsEmpty() || lpFind == NULL )
		return;
	//读取XML
	if ( !m_xmlDoc.Load(strPath) )
	{
		TRACE(_T("fail to load xml: %s.\r\n"),
			strPath);
		return;
	}
	//得到根结点
	CXmlNodeWrapper rootNode(m_xmlDoc.AsNode());

	//查找字段名所在的结点
	ASSERT(lpFind != NULL);
	CXmlNodeWrapper xmlNode(
		rootNode.FindNode(lpFind) );
	//找不到则返回
	if ( !xmlNode.IsValid() )
		return;
	//得到结点名称
	strPath = xmlNode.Name();
	//格式化下一次查找的字符串
	strPath.Insert(0, _T("//tbody//"));

	//查找目标结点
	CXmlNodeWrapper destNode(
		rootNode.FindNode(strPath) );
	if ( !destNode.IsValid() )
		return;

	//复制结点的字符串
	strURL = destNode.GetText();
}


//得到xml数据文件的路径及查找字符串
void CCompanyWebSite::GetParam(CString &strPath, 
		LPCTSTR &lpFind)
{	
	strPath = _T("");
	lpFind = NULL;

	Tx::Business::TxBusiness busObj;
	//判断交易实体类型
	Tx::Data::Security* pSecurity = 
		busObj.GetSecurityNow(m_id);
	if ( pSecurity == NULL )
		return;
	//得到数据文件
	//根据股票类型取得不同的XML文件
	//需要两个参数: 
	//1. 功能ID
	//2. 机构ID
	INT iFnID = 0,//功能ID
	    iFileID = 0;

	if ( pSecurity->IsStock() )
	{
		//股票
		iFnID = 10260;
		//文件ID是机构ID
		iFileID = 
			pSecurity->GetInstitutionId();
		lpFind = 
			_T("//ColumnName//*[@fieldName='f_Company_WebSite']");
	}
	else if ( pSecurity->IsFund() )
	{
		//基金
		iFnID = 10524;
		//文件ID是券ID
		iFileID = pSecurity->GetSecurity1Id();;

		lpFind = 
			_T("//ColumnName//*[@fieldName='f_company_website']");
	}
	else
	{
		ASSERT(FALSE);
		return;
	}

	//先得到数据文件
	strPath = 
		busObj.DownloadXmlFile(iFileID, iFnID);
}

//得到错误提示的字符串
void CCompanyWebSite::GetErrText(CString &strHtml)
{
	strHtml = _T("");
	strHtml.Append("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\
						   <HTML><HEAD><TITLE>无法找到该页</TITLE>\
						   <META HTTP-EQUIV=\"Content-Type\" Content=\"text/html; charset=GB2312\">\
						   <STYLE type=\"text/css\">\
						   BODY { font: 9pt/12pt 宋体 }\
						   H1 { font: 12pt/15pt 宋体 }\
						   H2 { font: 9pt/12pt 宋体 }\
						   A:link { color: red }\
						   A:visited { color: maroon }\
						   </STYLE>\
						   </HEAD><BODY><TABLE width=500 border=0 cellspacing=10><TR><TD>\
						   <h1>无法找到该公司的主页</h1>\
						   此公司主页可能不存在或者已经删除、更名或暂时不可用。<hr>\
						   </TD></TR></TABLE></BODY></HTML>");
}