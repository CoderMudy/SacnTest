/*************************************************
  Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
  File name:	CompanyWebsite.cpp
  Author:       lil
  Version:		1.0
  Date:			2007-09-04
  
  Description:
				1. �ӹ�˾�����XML�ļ��н�������˾��ҳ��URL.
				2. ��XML��Ϊͷ���������ݲ���
				3. ÿһ��XML��ӦԴ���ݿ���һ����¼
				4. ͷ����������,���ݲ��ָ�����������
				5. ��ù�˾��ַ�Ĳ���:
				   5.1 ��ͷ���в���ĳһ������Ӧ�����T
				   5.2 �������T�����ݲ����в��Ҷ�Ӧ��XML�������
				   5.3 �����ݼ���˾��ַ
	
*************************************************/



#include "../../Data/XmlNodeWrapper.h"

using namespace Tx::Data;

class AFX_EXT_CLASS CCompanyWebSite
{
public:
	CCompanyWebSite(INT iSecurityId);
	~CCompanyWebSite();

	//�õ���˾��ҳ��URL
	void GetURL(CString &strURL);

	//�õ�������ʾ���ַ���
	static void GetErrText(CString &strHtml);

protected:
	//�õ�xml�����ļ���·���������ַ���
	void GetParam(CString &strPath, 
		LPCTSTR &lpFind);

private:
	CXmlDocumentWrapper m_xmlDoc;
	//����ʵ��ID
	INT m_id;
};