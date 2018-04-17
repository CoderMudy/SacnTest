#include "StdAfx.h"
#include "BigDealStatService.h"

#include "StatXMLDataParser.h"
#include "StatReqAndRsp.h"
#include "StatDataTransfer.h"
#include "..\..\..\Data\TableDataManager.h"
#include "..\..\..\Data\SecurityQuotation.h"
#include "../../../Core/Core/ProgressWnd.h"

const CString strCOMMAND = _T("0001");
const CString strCOMMAN_DESCRIP = _T("BigDealStatDataReq");

CBigDealStatService::CBigDealStatService()
{
	m_iSecuritiesType = eSTOCK_AND_BOND;
	m_bAllSample = true;
}
CBigDealStatService::CBigDealStatService(int iSecuritiesType,bool bAllSample)
{
	if (iSecuritiesType < eSTOCK_AND_BOND || iSecuritiesType >=eSECURITIES_TYPE_COUNT)
	{
		m_iSecuritiesType = eSTOCK_AND_BOND;
	}
	else
	{
		m_iSecuritiesType = iSecuritiesType;
	}

	m_bAllSample = bAllSample;
}

CBigDealStatService::~CBigDealStatService()
{
}

BOOL CBigDealStatService::UpdateDataCache(long lStartDate, long lEndDate)
{
	if (lStartDate < 19800101 || lEndDate < 19800101)
	{
		return FALSE;
	}

	CString strDescrip = _T("大宗交易");

	CString strPrompt;
	strPrompt.Format(_T("%s数据传输.."), strDescrip);
	UINT progId = 1;
	Tx::Core::ProgressWnd prw;
	progId = prw.AddItem(progId, strPrompt, 0.0);
	prw.Show(0);

	//lStartDate = 20100130;
	CStatRequest reqBigDealStat;
	reqBigDealStat.m_strCommand = strCOMMAND;
	reqBigDealStat.m_strCommandDescrip = strCOMMAN_DESCRIP;
	reqBigDealStat.m_iReqSeqNumber = 0;

	CStatParam param;

	param.m_strName = _T("StartDate");
	param.m_strValue.Format(_T("%ld"), lStartDate);
	reqBigDealStat.m_arrStatParams.push_back(param);

	param.m_strName = _T("EndDate");
	param.m_strValue.Format(_T("%ld"), lEndDate);
	reqBigDealStat.m_arrStatParams.push_back(param);

	param.m_strName = _T("SecuritiesType");
	param.m_strValue.Format(_T("%ld"), this->m_iSecuritiesType);
	reqBigDealStat.m_arrStatParams.push_back(param);

	CStatXMLDataParser parserStat(strDescrip);
	CString strReqXML = parserStat.XMLReqFormat(reqBigDealStat);

	prw.SetPercent(progId, 0.3);//进度条

	CString strRspXML;
	CString strURL = Tx::Core::SystemInfo::GetInstance()->GetServerAddr(_T("File"),_T("DZJYInfoService"));
	//CString strURL =_T("http://192.168.5.77/DZJY/DZJYService.aspx");//chenyh to change
	CStatDataTransfer loadBigDealData(strURL);
	if (!loadBigDealData.ReqExec(strReqXML, strRspXML))
	{
		prw.SetPercent(progId, 1.0);//进度条
		strPrompt += _T(",完成!");
		prw.SetText(progId, strPrompt);

		return FALSE;
	}

	prw.SetPercent(progId, 1.0);//进度条
	strPrompt += _T(",完成!");
	prw.SetText(progId, strPrompt);
	prw.Hide();

	Tx::Core::Table_Display tabResult;
	CStatResponse rspStat;
	parserStat.XMLParse(strRspXML, rspStat, tabResult);

	if (!AppendUpdatedTabToCache(tabResult))
	{
		return FALSE;
	}

	this->m_lStartCacheDate = lStartDate;
	this->m_lEndCacheDate = lEndDate;

	return TRUE;
}
BOOL CBigDealStatService::AppendUpdatedTabToCache(Tx::Core::Table_Display& tabResult)
{
	if (0 == this->m_tabCache.GetRowCount())
	{
		m_tabCache.Clone(tabResult);
	}
	else
	{
		UINT iCacheColCount = this->m_tabCache.GetColCount();
		if (iCacheColCount != tabResult.GetColCount())
		{
			m_tabCache.Clone(tabResult);
		}
		else
		{
			CString strValue;
			int iCacheRowCount = this->m_tabCache.GetRowCount();
			for (UINT iRow = 0; iRow < tabResult.GetRowCount(); iRow++)
			{
				this->m_tabCache.AddRow();
				for (UINT iCol = 0; iCol < tabResult.GetColCount(); iCol++)
				{
					tabResult.GetCellString(iCol, iRow, strValue);
					this->m_tabCache.SetCell(iCol, iCacheRowCount, strValue);
				}
				iCacheRowCount++;
			}
		}
	}

	return TRUE;
}
BOOL CBigDealStatService::GetDataFromCache(long lStartDate, long lEndDate, Tx::Core::Table_Display& tabResult)
{
	tabResult.Clear();
	if (lStartDate < this->m_lStartCacheDate || lEndDate > this->m_lEndCacheDate)
	{
		return FALSE;
	}
	if (lStartDate == this->m_lStartCacheDate && lEndDate == this->m_lEndCacheDate)
	{
		tabResult.Clone(this->m_tabCache);// temp code
	}
	else
	{
		CString strValue;
		int iCacheRowCount = this->m_tabCache.GetRowCount();
		for (UINT iRow = 0; iRow < tabResult.GetRowCount(); iRow++)
		{
			this->m_tabCache.AddRow();
			for (UINT iCol = 0; iCol < tabResult.GetColCount(); iCol++)
			{
				tabResult.GetCellString(iCol, iRow, strValue);
				this->m_tabCache.SetCell(iCol, iCacheRowCount, strValue);
			}
			iCacheRowCount++;
		}
	}
	return TRUE;
}

BOOL CBigDealStatService::GetShowData(vector<int> vSamples, long lStartDate, long lEndDate, Tx::Core::Table_Display& tabResult)
{
	Tx::Core::Table_Display tabTemp;
	if (!this->GetData(lStartDate, lEndDate,tabTemp))
	{
		return FALSE;
	}

	UINT iColCount = tabTemp.GetColCount();
	if (iColCount != 9)
	{
		return FALSE;
	}

	tabResult.Clear();


	CString sColName[]=	{
						_T(""), _T("名称"),_T("代码"),_T("交易日期"),
						_T("成交价(元)"),_T("成交量(万股)"),_T("成交金额(万元)"), 
						_T("买入营业部"),_T("卖出营业部"),_T("是否专场")
						};
	Tx::Core::Data_Type iColType[] ={
						Tx::Core::dtype_int4, Tx::Core::dtype_val_string, Tx::Core::dtype_val_string, Tx::Core::dtype_val_string, 
						Tx::Core::dtype_double, Tx::Core::dtype_double, Tx::Core::dtype_double, 
						Tx::Core::dtype_val_string, Tx::Core::dtype_val_string, Tx::Core::dtype_val_string
					};

	UINT iShowColCount = 10;
	for (UINT iCol = 0; iCol < iShowColCount; iCol++)
	{
		tabResult.AddCol(iColType[iCol]);
		if(sColName[iCol].GetLength() > 0)
		{
			tabResult.SetTitle(iCol, sColName[iCol]);
		}
	}

	tabResult.SetPrecise(4, 2);

	tabResult.SetFormatStyle(5, Tx::Core::fs_finance);//设置显示风格
	tabResult.SetPrecise(5, 2);

	tabResult.SetFormatStyle(5, Tx::Core::fs_finance);//设置显示风格
	tabResult.SetPrecise(6, 2);

	int iEntityID = 0;
	CString strName;
	CString strCode;
	CString strValue;
	double dValue = 0.0;


	CString strDescrip = _T("大宗交易");
	CString strPrompt;
	strPrompt.Format(_T("%s数据转换.."), strDescrip);
	UINT progId = 1;
	Tx::Core::ProgressWnd prw;
	progId = prw.AddItem(progId, strPrompt, 0.05);
	prw.Show(0);

	UINT iRowCount = tabTemp.GetRowCount();
	int iRealRow = -1;

	double dPercent = 0.05;
	double dStep = (1- dPercent) / iRowCount;
	for (UINT i = 0; i < iRowCount; i++)
	{
		dPercent += dStep;
		prw.SetPercent(progId, dPercent);

		tabTemp.GetCellString(0,i, strValue);
		iEntityID = _ttoi(strValue);
		Tx::Data::SecurityQuotation* p = (Tx::Data::SecurityQuotation*)GetSecurity(iEntityID);
		if(p==NULL)
		{
			continue;
		}
		if (!m_bAllSample && std::find(vSamples.begin(), vSamples.end(), iEntityID) == vSamples.end())
		{
			continue;
		}
		tabResult.AddRow();
		iRealRow++;

		tabResult.SetCell(0, iRealRow, iEntityID);
		strName = p->GetName();

		strCode = p->GetCode();
		int iPos = strCode.Find('.');
		if (iPos > 0)
		{
			strCode = strCode.Left(iPos);	
		}

		tabResult.SetCell(1, iRealRow, strName);
		tabResult.SetCell(2, iRealRow, strCode);

		tabTemp.GetCellString(1, i, strValue);
		tabResult.SetCell(3, iRealRow, strValue);

		tabTemp.GetCellString(2, i, strValue);
		dValue = (double)_tstof(strValue);
		tabResult.SetCell(4, iRealRow, dValue);

		tabTemp.GetCellString(3, i, strValue);
		dValue = (double)_tstof(strValue);
		tabResult.SetCell(5, iRealRow, dValue);

		tabTemp.GetCellString(4, i, strValue);
		dValue = (double)_tstof(strValue);
		tabResult.SetCell(6, iRealRow, dValue);


		tabTemp.GetCellString(6, i, strValue);
		tabResult.SetCell(7, iRealRow, strValue);

		tabTemp.GetCellString(7, i, strValue);
		tabResult.SetCell(8, iRealRow, strValue);

		tabTemp.GetCellString(5, i, strValue);
		if (0 == strValue.Compare(_T("0")))
		{
			strValue = _T("是");
		}
		else if (0 == strValue.Compare(_T("1")))
		{
			strValue = _T("否");
		}
		else
		{
			strValue = _T("--");
		}
		tabResult.SetCell(9, iRealRow, strValue);
	}
	tabResult.Sort(3, false);

	prw.SetPercent(progId, 1.0);//进度条
	strPrompt += _T(",完成!");
	prw.SetText(progId, strPrompt);

	return TRUE;
}

