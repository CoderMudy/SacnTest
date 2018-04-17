/*************************************************
  Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
  File name:	CompanyWebsite.cpp
  Author:       lil
  Version:		1.0
  Date:			2007-09-04
  
  Description:
				1. 从公司情况的XML文件中解析出公司主页的URL.
				2. 此XML分为头部分与数据部分
				3. 每一个XML对应源数据库中一条记录
				4. 头部包含列名,数据部分根据列名区分
				5. 获得公司网址的步骤:
				   5.1 在头部中查找某一列名对应的序号T
				   5.2 根据序号T在数据部分中查找对应的XML结点内容
				   5.3 此内容即公司网址
	
*************************************************/



#include "../../Data/XmlNodeWrapper.h"

using namespace Tx::Data;

class AFX_EXT_CLASS CCompanyWebSite
{
public:
	CCompanyWebSite(INT iSecurityId);
	~CCompanyWebSite();

	//得到公司主页的URL
	void GetURL(CString &strURL);

	//得到错误提示的字符串
	static void GetErrText(CString &strHtml);

protected:
	//得到xml数据文件的路径及查找字符串
	void GetParam(CString &strPath, 
		LPCTSTR &lpFind);

private:
	CXmlDocumentWrapper m_xmlDoc;
	//交易实体ID
	INT m_id;
};