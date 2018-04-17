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



//�õ���˾��ҳ��URL
void CCompanyWebSite::GetURL(CString &strURL)
{
	strURL = _T("");
	//�ȵõ�XML�ļ�·��
	CString strPath;
	LPCTSTR lpFind = NULL;
	GetParam(strPath, lpFind);
	if ( strPath.IsEmpty() || lpFind == NULL )
		return;
	//��ȡXML
	if ( !m_xmlDoc.Load(strPath) )
	{
		TRACE(_T("fail to load xml: %s.\r\n"),
			strPath);
		return;
	}
	//�õ������
	CXmlNodeWrapper rootNode(m_xmlDoc.AsNode());

	//�����ֶ������ڵĽ��
	ASSERT(lpFind != NULL);
	CXmlNodeWrapper xmlNode(
		rootNode.FindNode(lpFind) );
	//�Ҳ����򷵻�
	if ( !xmlNode.IsValid() )
		return;
	//�õ��������
	strPath = xmlNode.Name();
	//��ʽ����һ�β��ҵ��ַ���
	strPath.Insert(0, _T("//tbody//"));

	//����Ŀ����
	CXmlNodeWrapper destNode(
		rootNode.FindNode(strPath) );
	if ( !destNode.IsValid() )
		return;

	//���ƽ����ַ���
	strURL = destNode.GetText();
}


//�õ�xml�����ļ���·���������ַ���
void CCompanyWebSite::GetParam(CString &strPath, 
		LPCTSTR &lpFind)
{	
	strPath = _T("");
	lpFind = NULL;

	Tx::Business::TxBusiness busObj;
	//�жϽ���ʵ������
	Tx::Data::Security* pSecurity = 
		busObj.GetSecurityNow(m_id);
	if ( pSecurity == NULL )
		return;
	//�õ������ļ�
	//���ݹ�Ʊ����ȡ�ò�ͬ��XML�ļ�
	//��Ҫ��������: 
	//1. ����ID
	//2. ����ID
	INT iFnID = 0,//����ID
	    iFileID = 0;

	if ( pSecurity->IsStock() )
	{
		//��Ʊ
		iFnID = 10260;
		//�ļ�ID�ǻ���ID
		iFileID = 
			pSecurity->GetInstitutionId();
		lpFind = 
			_T("//ColumnName//*[@fieldName='f_Company_WebSite']");
	}
	else if ( pSecurity->IsFund() )
	{
		//����
		iFnID = 10524;
		//�ļ�ID��ȯID
		iFileID = pSecurity->GetSecurity1Id();;

		lpFind = 
			_T("//ColumnName//*[@fieldName='f_company_website']");
	}
	else
	{
		ASSERT(FALSE);
		return;
	}

	//�ȵõ������ļ�
	strPath = 
		busObj.DownloadXmlFile(iFileID, iFnID);
}

//�õ�������ʾ���ַ���
void CCompanyWebSite::GetErrText(CString &strHtml)
{
	strHtml = _T("");
	strHtml.Append("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\
						   <HTML><HEAD><TITLE>�޷��ҵ���ҳ</TITLE>\
						   <META HTTP-EQUIV=\"Content-Type\" Content=\"text/html; charset=GB2312\">\
						   <STYLE type=\"text/css\">\
						   BODY { font: 9pt/12pt ���� }\
						   H1 { font: 12pt/15pt ���� }\
						   H2 { font: 9pt/12pt ���� }\
						   A:link { color: red }\
						   A:visited { color: maroon }\
						   </STYLE>\
						   </HEAD><BODY><TABLE width=500 border=0 cellspacing=10><TR><TD>\
						   <h1>�޷��ҵ��ù�˾����ҳ</h1>\
						   �˹�˾��ҳ���ܲ����ڻ����Ѿ�ɾ������������ʱ�����á�<hr>\
						   </TD></TR></TABLE></BODY></HTML>");
}