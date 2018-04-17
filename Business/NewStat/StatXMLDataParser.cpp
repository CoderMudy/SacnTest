#include "StdAfx.h"
#include "StatXMLDataParser.h"
#include "../../../Data/XmlNodeWrapper.h"
#include "../../../Core/Core/ProgressWnd.h"

CStatXMLDataParser::CStatXMLDataParser(CString& strDescrip)
{
	 m_strDescrip = strDescrip;
}
CStatXMLDataParser::~CStatXMLDataParser()
{
}

CString CStatXMLDataParser::XMLReqFormat(CStatRequest& reqStat)
{
	/*1 初始化*/
	CString strRequestXML = _T("");
	if (!reqStat.IsValid())
	{
		return FALSE;
	}
	OleInitialize(NULL);

	
	/*2 XML Format*/
	CString strFirst;
	strFirst.Format(_T("<?xml version=\"1.0\" encoding=\"gb2312\"?><operation></operation>"));
	Tx::Data::CXmlDocumentWrapper doc;
	doc.LoadXML(strFirst);

	CString strValue;
	Tx::Data::CXmlNodeWrapper nodeOp(doc.AsNode());
	nodeOp.SetValue(_T("operation_command"), reqStat.m_strCommand);
	nodeOp.SetValue(_T("operation_name"), reqStat.m_strCommandDescrip);
	strValue.Format(_T("%d"), reqStat.m_iReqSeqNumber);
	nodeOp.SetValue(_T("req_seq"),strValue);

	std::vector<CStatParam>::iterator iterParam = reqStat.m_arrStatParams.begin();
	for (int i = 0; iterParam != reqStat.m_arrStatParams.end(); iterParam++, i++)
	{
		Tx::Data::CXmlNodeWrapper nodeT(nodeOp.InsertNode(i, iterParam->m_strName));
		nodeT.SetText(iterParam->m_strValue);
	}

	strRequestXML = doc.GetXML();
/*
#ifdef _DEBUG
	CString strFilePath;
	strFilePath.Format(_T("C:\\big_deal_req.xml"));
	doc.Save(strFilePath);//使用时删掉
#endif
	*/
	OleUninitialize();

	return strRequestXML;
}

BOOL CStatXMLDataParser::XMLParse(const CString& strXML, CStatResponse& rspStat, Tx::Core::Table_Display &resTable)
{
	if (strXML.IsEmpty())
	{
		return FALSE;
	}

	rspStat.Empty();
	resTable.Clear();

	/*1 XML Generate*/
	OleInitialize(NULL);

	CString strPrompt;
	strPrompt.Format(_T("%s数据解析.."), this->m_strDescrip);
	UINT progId = 1;
	Tx::Core::ProgressWnd prw;
	progId = prw.AddItem(progId, strPrompt, 0.0);
	prw.Show(0);

	Tx::Data::CXmlDocumentWrapper xmlDoc;
	/*2 读取XML*/
	if (!xmlDoc.LoadXML(strXML))
	{
		prw.SetPercent(progId, 1.0);//进度条
		strPrompt += _T(",完成!");
		prw.SetText(progId, strPrompt);

		TRACE(_T("fail to load xml: %s.\r\n"), strXML);
		OleUninitialize();
		return FALSE;
	}
	
	prw.SetPercent(progId, 0.05);//进度条
	/*3得到根结点*/
	Tx::Data::CXmlNodeWrapper resultNode(xmlDoc.AsNode());
	if (_T("element") != resultNode.NodeType() || _T("result") != resultNode.Name().MakeLower())
	{
		prw.SetPercent(progId, 1.0);//进度条
		strPrompt += _T(",完成!");
		prw.SetText(progId, strPrompt);

		OleUninitialize();
		return FALSE;
	}
	//Result 属性
	CString arrAttrStr[] = {_T("operation_command"), _T("req_seq"), _T("result"), _T("start_date"), _T("end_date")};
	int iAttrCount = resultNode.NumAttributes();
	if (iAttrCount != 5)
	{
		prw.SetPercent(progId, 1.0);//进度条
		strPrompt += _T(",完成!");
		prw.SetText(progId, strPrompt);

		TRACE(_T("user attribute incorrect number.\r\n"));
		OleUninitialize();
		return FALSE;
	}

	CString strValue;
	CString strName;
	for(int iAttrIdx = 0; iAttrIdx < iAttrCount; iAttrIdx++)
	{
		strName = resultNode.GetAttribName(iAttrIdx).MakeLower();
		if(arrAttrStr[iAttrIdx] != strName)
		{
			break;
		}
		switch(iAttrIdx)
		{
		case 0:
			{
				rspStat.m_strCommand = resultNode.GetAttribVal(iAttrIdx);
			}
			break;
		case 1:
			{
				strValue = resultNode.GetAttribVal(iAttrIdx);
				rspStat.m_iReqSeqNumber = _ttoi(strValue);
			}
			break;
		case 2:
			{
				strValue = resultNode.GetAttribVal(iAttrIdx);
				rspStat.m_iResult = _ttoi(strValue);
			}
			break;
		case 3:
		case 4:
			{
				CStatParam param;
				param.m_strName = strName;
				param.m_strValue = resultNode.GetAttribVal(iAttrIdx);
				rspStat.m_arrToBeCheckedParams.push_back(param);
			}
			break;
		default:break;
		}
	}

	int iNodeCount = resultNode.NumNodes();
	if (1 != iNodeCount)
	{
		prw.SetPercent(progId, 1.0);//进度条
		strPrompt += _T(",完成!");
		prw.SetText(progId, strPrompt);

		TRACE(_T("Incorrect info count.\r\n"));
		OleUninitialize();
		return FALSE;
	}
	Tx::Data::CXmlNodeWrapper tableNode(resultNode.GetNode(0));
/*	strName = tableNode.GetAttribName(0).MakeLower();
	if( _T("datamean") != strName)
	{
		TRACE(_T("Incorrect table info.\r\n"));
		OleUninitialize();
		return FALSE;
	}
*/
	CString strFind;
	strFind.Format(_T("//thead"));
	Tx::Data::CXmlNodeWrapper xmlColNameNode(tableNode.FindNode(strFind));

	if (!xmlColNameNode.IsValid())//找不到则返回
	{
		prw.SetPercent(progId, 1.0);//进度条
		strPrompt += _T(",完成!");
		prw.SetText(progId, strPrompt);

		TRACE(_T("Incorrect table info.\r\n"));
		OleUninitialize();
		return FALSE;
	}
	int iColCount = xmlColNameNode.NumNodes();
	for(int iT = 0; iT < iColCount; iT++)
	{
		Tx::Data::CXmlNodeWrapper ColNode(xmlColNameNode.GetNode(iT));
		resTable.AddCol(Tx::Core::dtype_val_string);
		resTable.SetTitle(iT, ColNode.GetText());
	}

	for(int iTBody = 0; iTBody < tableNode.NumNodes(); iTBody++)
	{
		Tx::Data::CXmlNodeWrapper bodyNode(tableNode.GetNode(iTBody));
		//首先解析数据列，如果列信息不存在，则立即退出
		if (_T("element") != bodyNode.NodeType() || _T("tbody") != bodyNode.Name().MakeLower())
		{
			continue;
		}

		int iNodes = bodyNode.NumNodes();
		double dPercent = 0.1;
		double dStep =  (1.0 - dPercent)/iNodes;
		//处理tbody的子节点
		resTable.AddRow(bodyNode.NumNodes());
		for (int j = 0; j < bodyNode.NumNodes(); j++)
		{
			dPercent = dPercent + dStep;
			prw.SetPercent(progId, dPercent);//进度条

			//解析列信息
			Tx::Data::CXmlNodeWrapper item_tr_Node(bodyNode.GetNode(j));
			//tr
			if (_T("element") != item_tr_Node.NodeType() || _T("tr") != item_tr_Node.Name().MakeLower())
			{
				continue;
			}

			if (!item_tr_Node.IsValid())//找不到则返回
			{
				OleUninitialize();
				return FALSE;
			}
			for (int k = 0; k < item_tr_Node.NumNodes(); ++k)
			{
				Tx::Data::CXmlNodeWrapper item_tn_Node(item_tr_Node.GetNode(k));
				CString strValue = item_tn_Node.GetText();
				resTable.SetCell(k,j, strValue);
			}
		}
	}

	prw.SetPercent(progId, 1.0);//进度条
	strPrompt += _T(",完成!");
	prw.SetText(progId, strPrompt);
	//exit
	OleUninitialize();
	return TRUE;
}
