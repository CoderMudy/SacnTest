/********************************************************************
	FileName:	FundDeriveData.cpp
	Created:	2007/09/25
	Project:	Business
	Author:		xum
	Version:	v1.0

	Purpose:	use to calculate the derived data of fund 
	
	History:	

*********************************************************************
	Copyright (C) 2007 - All Rights Reserved
*********************************************************************/

#include "StdAfx.h"
#include "FundDeriveData.h"
#include "..\..\core\core\Commonality.h"
#include "TxBusiness.h"
#include <math.h>
#include "MyIndicator.h"
//#include "..\..\Data\XmlNodeWrapper.h"
#include "../../Core/Driver/ClientFileEngine/base/HttpDnAndUp.h"
#include "../../Core/Driver/ClientFileEngine/base/zip/ZipWrapper.h"


namespace Tx
{
	namespace Business
	{

FundDeriveData::FundDeriveData(void)
{
}

FundDeriveData::~FundDeriveData(void)
{
}

//计算一组基金，每个间隔时间的净值增长率的加权值
bool FundDeriveData::CalcWeightedFundNvr (std::vector<double> & dWeighted, Tx::Core::Table& resultTable, std::vector<int> & iSecurityId, std::set< std::pair<int,int> >& date)
{
	//---------------------------------------------------------------------------------
	bool bRet = true;

	UINT varCfg[2];			//参数配置
	int varCount=2;			//参数个数
	varCfg[0]=0;
	varCfg[1]=1;

	UINT nArray[3];
	nArray[0]=0;
	nArray[1]=1;
	nArray[2]=2;

	//----------------------------------------------------------------------------------
	// 1: 取基金的份额
	Tx::Core::Table_Indicator shareTable;
	shareTable.AddParameterColumn(Tx::Core::dtype_int4);
	shareTable.AddParameterColumn(Tx::Core::dtype_int4);	//第二参数列:日期
	shareTable.AddIndicatorColumn(30301111,Tx::Core::dtype_double,varCfg,2); // 总份额

	std::vector<int> dates;
	for (std::set< std::pair<int,int> >::iterator iter=date.begin();iter!=date.end();iter++)
		dates.push_back (iter->second);

	bool result = m_pLogicalBusiness->GetData(shareTable, iSecurityId, dates, 1, 0);
	if(result==false)
		return false;

	// 2: 计算加权的净值增长率
	for (int k=1; k<(int)resultTable.GetColCount (); k++)
	{
		double dRet = 0.0;
		for (int t=0; t<(int)resultTable.GetRowCount(); t++)
		{
			double dTemp1 = 0.0;
			double dTemp2 = 0.0;
			resultTable.GetCell (k, t, dTemp1);

			int z = t*(resultTable.GetRowCount()-1) + (k-1); // 注意：提取的数据次序
			shareTable.GetCell (2, z, dTemp2);

			dRet += dTemp1 * dTemp2;
		}

		dWeighted.push_back (dRet);
	}

	return true;
}
//moldified by lijw 2008-05-26
//计算一组基金，某区间内，每个间隔时间的单位净值增长率
bool FundDeriveData::CalcFundNvr (Tx::Core::Table& resultTable, std::vector<int> & iSecurityId, std::set< std::pair<int,int> >& date)
{
//	Tx::Core::ProgressWnd prw;
	//UINT	nProgId = prw.AddItem( 0, _T("获取收益率序列……") );
	//prw.Show( 1 );
	bool bRet = true;

	resultTable.Clear();
	resultTable.AddRow (iSecurityId.size());
	resultTable.AddCol(Tx::Core::dtype_int4); // 内码
	for (UINT i = 0;i<date.size();i++)
	{
		resultTable.AddCol(Tx::Core::dtype_double); // 各个区间的值
	}

	// 以下是新的净值增长率调用接口
//	CString strTransId,strStartDate,strEndDate;
//	CString strLong = _T("id,");
//	for (UINT i=0;i<iSecurityId.size();i++)
//	{
//		strTransId.Format(_T("%d,"),iSecurityId[i]);
//		strLong += strTransId;
//		resultTable.SetCell(0,i,iSecurityId[i]);
//	}
//
//	strLong += _T("start,");
//	for (std::set< std::pair<int,int> >::iterator iter = date.begin();iter != date.end();iter++)
//	{
//		strStartDate.Format(_T("%d,"),iter->first);
//		strLong += strStartDate;
//	}
//
//	strLong += _T("end,");
//	for (std::set< std::pair<int,int> >::iterator iter = date.begin();iter != date.end();iter++)
//	{
//		strEndDate.Format(_T("%d,"),iter->second);
//		strLong += strEndDate;
//	}
//	// 去掉最后一个','
//	strLong = strLong.Left(strLong.GetLength() - 1);
//
//#ifndef _UNICODE
//	USES_CONVERSION;
//	LPWSTR lpwstr = A2W(strLong);
//#else
//	LPCTSTR lpwstr = strLong;
//#endif
//	UINT nSize = strLong.GetLength() * sizeof(wchar_t);

	// Binary
	int iSize = sizeof(int) * (1+1+1+iSecurityId.size()+date.size()+date.size()) + 4;
	LPBYTE pBuffer = new BYTE [iSize];
	if (pBuffer == NULL)
		return false;
	LPBYTE pWrite = pBuffer + 4;
	memset(pBuffer,0,iSize);
//	memcpy_s(pWrite,iSize,&nSecSize,sizeof(int));
	int nSecSize = (int)iSecurityId.size();
	memcpy_s(pWrite,iSize,&nSecSize,sizeof(int));
	pWrite += sizeof(int);
	for (UINT i=0;i<(UINT)nSecSize;i++)
	{
		//strTransId.Format(_T("%d,"),iSecurityId[i]);
		//strLong += strTransId;
		//resultTable.SetCell(0,i,iSecurityId[i]);
		memcpy_s(pWrite,iSize,&iSecurityId[i],sizeof(int));
		pWrite += sizeof(int);
		resultTable.SetCell(0,i,iSecurityId[i]);
	}

	nSecSize = (int)date.size();
	memcpy_s(pWrite,iSize,&nSecSize,sizeof(int));
	pWrite += sizeof(int);

	for (std::set< std::pair<int,int> >::iterator iter = date.begin();iter != date.end();iter++)
	{
		//strStartDate.Format(_T("%d,"),iter->first);
		//strLong += strStartDate;
		memcpy_s(pWrite,iSize,&iter->first,sizeof(int));
		pWrite += sizeof(int);
	}

	nSecSize = (int)date.size();
	memcpy_s(pWrite,iSize,&nSecSize,sizeof(int));
	pWrite += sizeof(int);

	for (std::set< std::pair<int,int> >::iterator iter = date.begin();iter != date.end();iter++)
	{
		//strEndDate.Format(_T("%d,"),iter->second);
		//strLong += strEndDate;
		memcpy_s(pWrite,iSize,&iter->second,sizeof(int));
		pWrite += sizeof(int);
	}
	//// 测试净值
	//int iSize = sizeof(int) * (1+1+1+1+iSecurityId.size()+date.size());
	//LPBYTE pBuffer = new BYTE [iSize];
	//if (pBuffer == NULL)
	//	return false;
	//LPBYTE pWrite = pBuffer;
	//memset(pBuffer,0,iSize);
	//int i=2;
	//memcpy_s(pWrite,iSize,&i,sizeof(int));
	//pWrite += sizeof(int);
	//int nSecSize = (int)iSecurityId.size();
	//memcpy_s(pWrite,iSize,&nSecSize,sizeof(int));
	//pWrite += sizeof(int);
	//for (UINT i=0;i<nSecSize;i++)
	//{
	//	//strTransId.Format(_T("%d,"),iSecurityId[i]);
	//	//strLong += strTransId;
	//	//resultTable.SetCell(0,i,iSecurityId[i]);
	//	memcpy_s(pWrite,iSize,&iSecurityId[i],sizeof(int));
	//	pWrite += sizeof(int);
	//	resultTable.SetCell(0,i,iSecurityId[i]);
	//}

	//nSecSize = (int)date.size();
	//memcpy_s(pWrite,iSize,&nSecSize,sizeof(int));
	//pWrite += sizeof(int);

	//for (std::set< std::pair<int,int> >::iterator iter = date.begin();iter != date.end();iter++)
	//{
	//	//strStartDate.Format(_T("%d,"),iter->first);
	//	//strLong += strStartDate;
	//	memcpy_s(pWrite,iSize,&iter->first,sizeof(int));
	//	pWrite += sizeof(int);
	//}

	//nSecSize = (int)date.size();
	//memcpy_s(pWrite,iSize,&nSecSize,sizeof(int));
	//pWrite += sizeof(int);

	//for (std::set< std::pair<int,int> >::iterator iter = date.begin();iter != date.end();iter++)
	//{
	//	//strEndDate.Format(_T("%d,"),iter->second);
	//	//strLong += strEndDate;
	//	memcpy_s(pWrite,iSize,&iter->second,sizeof(int));
	//	pWrite += sizeof(int);
	//}

	//LPCTSTR lpUrl = _T("http://192.168.5.87/FundNavSer/ServerHandler.ashx");
	//LPCTSTR lpUrl = _T("http://192.168.6.89/FundNavSerBinary/ServerHandler.ashx");
	//LPCTSTR lpUrl = _T("http://221.122.41.211/FundNavSer/ServerHandler.ashx");
	LPCTSTR lpUrl = CORE_GETSYSINFO->GetServerAddr(_T("File"),_T("FundNavRaiseRate"));
	Tx::Drive::Http::CSyncUpload upload;
	int iStart = ::GetTickCount();
//	if ( upload.Post(lpUrl, (LPBYTE)lpwstr, nSize) )
	if ( upload.Post(lpUrl, pBuffer, iSize,_T("Content-Type: application/octet-stream")) )
	{
		int iEnd = ::GetTickCount();
		TRACE(_T("\r\nURL Cost Time %d(ms)\r\n"),iEnd-iStart);
		CONST Tx::Drive::Mem::MemSlice &data = upload.Rsp().Body();
		LPBYTE lpRes = data.DataPtr();
		UINT nRetSize = data.Size();
		if (nRetSize <= 0)
		{
			delete pBuffer;
			pBuffer = NULL;
			return false;
		}

		UINT nPreSize = *(reinterpret_cast<UINT*>(lpRes));
		LPBYTE lpData = new BYTE[nPreSize];
		if ( lpData == NULL )
		{
			delete pBuffer;
			pBuffer = NULL;
			return false;
		}
		if ( !Tx::Drive::IO::Zip::CZipWrapper::MemUnZip(lpData, 
			   nPreSize, lpRes + sizeof(UINT), nRetSize - sizeof(UINT)) )
		{
			delete []lpData;
			delete[] pBuffer;
			pBuffer = NULL;
			return false;
		}

		//UINT nCount = nPreSize / sizeof(wchar_t);
		//CStringW strTemp;
		//LPWSTR lpTemp = strTemp.GetBuffer(nPreSize + sizeof(wchar_t));
		//memcpy_s(lpTemp, nPreSize, lpData, nPreSize);
		//lpTemp[nCount] = 0;
		//strTemp.ReleaseBuffer();
		//delete []lpData;
		//lpData = NULL;


		//CString strRet(lpTemp);

		//// 解析结果 id1,value1,value2....,id2,value1,value2,
		//iStart = ::GetTickCount();
		//UINT nParseCount = date.size() + 1;
		//UINT i=1;

		//TCHAR sep[] = _T(",");
		//TCHAR *temptoken=NULL; 
		//TCHAR* token = _tcstok_s(strRet.GetBuffer(),sep,&temptoken);
		//while (token != NULL)
		//{
		//	token = _tcstok_s(NULL,sep,&temptoken);
		//	if (++i % nParseCount != 1)
		//	{
		//		if (0 == _stricmp(token,_T("--")))
		//			resultTable.SetCell((i - 1)%nParseCount,(i - 1)/nParseCount,Tx::Core::Con_doubleInvalid);
		//		else
		//			resultTable.SetCell((i - 1)%nParseCount,(i - 1)/nParseCount,_tstof(token));
		//		//TRACE(_T("%s,"),token);
		//	}
		//}
		//strRet.ReleaseBuffer();
		//iEnd = ::GetTickCount();
		//TRACE(_T("\r\nParse Result Cost Time %d(ms)\r\n"),iEnd-iStart);
		iStart = ::GetTickCount();
		LPBYTE pRecv = lpData;
		UINT nParseCount = date.size() * iSecurityId.size();
		float fValue = 0.0;
		double dValue = 0.0;
		for (UINT i=0;i < nParseCount; i++)
		{
			memcpy_s(&fValue,sizeof(float),pRecv,sizeof(float));
			pRecv += sizeof(float);
			if (fabs(fValue - Tx::Core::Con_floatInvalid) < 0.00001)
				// 无效值
				resultTable.SetCell(i%date.size() + 1,i/date.size(),Tx::Core::Con_doubleInvalid);
			else
			{
				dValue = (double)fValue;
				resultTable.SetCell(i%date.size() + 1,i/date.size(),dValue);
			}
		}
		delete []lpData;
		lpData = NULL;
		iEnd = ::GetTickCount();
		TRACE(_T("\r\nParse Result Cost Time %d(ms)\r\n"),iEnd-iStart);
	}
	delete[] pBuffer;
	pBuffer = NULL;

//	//---------------------------------------------------------------------------------
//	UINT varCfg[2];			//参数配置
//	int varCount=2;			//参数个数
//	varCfg[0]=0;
//	varCfg[1]=1;
//
//	UINT nArray[4];
//	nArray[0]=0;
//	nArray[1]=1;
//	nArray[2]=2;
//	nArray[3]=3;
//
//	//----------------------------------------------------------------------------------
//	// 1: 取单位净值
//	Tx::Core::Table_Indicator navTable;
//	navTable.AddParameterColumn(Tx::Core::dtype_int4);
//	navTable.AddParameterColumn(Tx::Core::dtype_int4);	//第二参数列:日期
//	navTable.AddIndicatorColumn(30301001,Tx::Core::dtype_double,varCfg,2); // 单位净值
//
//	std::vector<int> dates;
//	std::set< std::pair<int,int> >::iterator iter;
//	//modified by lijw 2008-05-26
//	for (iter = date.begin();iter!=date.end();iter++)
//	{
//		dates.push_back (iter->first);
////   	dates.push_back (iter->second);
//	}
//    dates.push_back((--iter)->second);//add by lijw 2008-05-26
//	bool result = m_pLogicalBusiness->GetData(navTable, iSecurityId, dates, 1, 0);
//	if(result==false)
//		return false;
//#ifdef _DEBUG
//	CString strTable4=navTable.TableToString();
//	Tx::Core::Commonality::String().StringToClipboard(strTable4);
//#endif	
//	//----------------------------------------------------------------------------------
//	// 2: 提取出净值调整系数
//	Tx::Core::Table_Indicator fundAdjustTable;
//	fundAdjustTable.AddParameterColumn(Tx::Core::dtype_int4);
//	fundAdjustTable.AddParameterColumn(Tx::Core::dtype_int4);
//	fundAdjustTable.AddIndicatorColumn(30301125,Tx::Core::dtype_double,varCfg,2);//调整前的净值
//	fundAdjustTable.AddIndicatorColumn(30301126,Tx::Core::dtype_double,varCfg,2);//调整后的净值
//
//	result = m_pLogicalBusiness->GetData(fundAdjustTable,true);
//	if(result==false)
//		return false;
//#ifdef _DEBUG
//	strTable4=navTable.TableToString();
//	Tx::Core::Commonality::String().StringToClipboard(strTable4);
//#endif	
//	//----------------------------------------------------------------------------------
//	// 3: 计算净值增长率
//	int k=0;
//	int nCount = iSecurityId.size();
//	for (std::vector<int>::iterator iter = iSecurityId.begin(); iter != iSecurityId.end (); iter++,k++)
//	{
//		resultTable.SetCell (0, k, *iter); // 第一列
//
//		// 提取某基金的净值调整系数
//		Tx::Core::Table_Indicator adjustTempTable1;
//		adjustTempTable1.CopyColumnInfoFrom(fundAdjustTable);
//		fundAdjustTable.Between(adjustTempTable1,nArray,4,0,*iter,*iter,true,true); // 某只基金
//		/*if(adjustTempTable1.GetRowCount() == 0)
//			continue;*/
//         //add by lijw 2008-08-20
//		if(adjustTempTable1.GetRowCount() == 0)
//		{
//			// 每个区间的净值增长率
//			int t=0;
//			for (std::set< std::pair<int,int> >::iterator iter2=date.begin(); iter2 != date.end(); iter2++, t++)
//			{
//				// ----------------------------------------------------------------------
//				// 前后两个净值
//				double begin = 1.0;
//				double end = 1.0;
//				int z = k*dates.size() + t; // 注意：提取的数据次序
//				navTable.GetCell (2, z, begin);	//这个值取道可能为0	wangzhy
//				navTable.GetCell (2, z+1, end);	//这个值取道可能为0	wangzhy
//				double adjust = 1.0;				
//				// ----------------------------------------------------------------------
//				// 净值增长率
//				double dRet = 0.0;
//				if ( begin != 0 )
//					dRet = end/begin*adjust - 1;
//				resultTable.SetCell (t+1, k, dRet);
//			}
//			continue;
//		}
//		// ----------------------------------------------------------------------
//		// 每个区间的净值增长率
//		int t=0;
////		std::vector<int> tempVector;        //存放临时的日期。
//		for (std::set< std::pair<int,int> >::iterator iter2=date.begin(); iter2 != date.end(); iter2++, t++)
//		{
//			// ----------------------------------------------------------------------
//			// 前后两个净值
//			double begin = 1.0;
//			double end = 1.0;
//			int z = k*dates.size() + t; // 注意：提取的数据次序
//			navTable.GetCell (2, z, begin);	//这个值取道可能为0	wangzhy
//			navTable.GetCell (2, z+1, end);	//这个值取道可能为0	wangzhy
//
//			// ----------------------------------------------------------------------
//			// 净值调整因子
//			Tx::Core::Table_Indicator adjustTempTable2;
//			adjustTempTable2.CopyColumnInfoFrom(fundAdjustTable);
//			//modify  by lijw 2008-05-27 ；
//			adjustTempTable1.Between(adjustTempTable2,nArray,4,1,iter2->first,iter2->second,true,false); // 取出起始和截至日之间的净值调整系数
//	/*		if(!tempVector.empty())
//				tempVector.clear();
//			tempVector.push_back(iter2->second);
//			adjustTempTable1.EqualsAt(adjustTempTable2,nArray,4,1,tempVector);*/
//			double adjust = 1.0;
//			for (UINT i=adjustTempTable2.GetRowCount();i>0;i--)//modify by lijw 2008-05-27 把1改为0；
//			{
//				double temp1 = 1.0;
//				double temp2 = 1.0;
//				adjustTempTable2.GetCell (2, i-1, temp1);
//				adjustTempTable2.GetCell (3, i-1, temp2);
//				if ( temp2 <= 0 )
//					adjust = 1.0;
//				else
//					adjust *= temp1/temp2;	
//			}
//
//			// ----------------------------------------------------------------------
//			// 净值增长率
//			double dRet = 0.0;
//			if ( begin != 0 )
//				dRet = end/begin*adjust - 1;
//			resultTable.SetCell (t+1, k, dRet);
//		}
//		prw.SetPercent( nProgId, double( k )/ nCount );
//	}
//#ifdef _DEBUG
//	strTable4=resultTable.TableToString();
//	Tx::Core::Commonality::String().StringToClipboard(strTable4);
//#endif	
//	prw.SetPercent( nProgId, 1.0 );
	return bRet;
}

//计算一组基金，某区间内，每个间隔时间的单位净值增长率
/*
{
	bool bRet = true;

	CString strTable;
	resultTable.Clear();
	resultTable.AddRow (iSecurityId.size());
	resultTable.AddCol(Tx::Core::dtype_int4); // 内码
	for (UINT i = 0;i<date.size();i++)
	{
		resultTable.AddCol(Tx::Core::dtype_double); // 各个区间的值
	}

	//---------------------------------------------------------------------------------
	UINT varCfg[2];			//参数配置
	int varCount=2;			//参数个数
	varCfg[0]=0;
	varCfg[1]=1;

	UINT nArray[3];
	nArray[0]=0;
	nArray[1]=1;
	nArray[2]=2;

	//----------------------------------------------------------------------------------
	// 1: 取单位净值
	Tx::Core::Table_Indicator gbTable;
	gbTable.AddParameterColumn(Tx::Core::dtype_int4);
	gbTable.AddParameterColumn(Tx::Core::dtype_int4);	//第二参数列:日期

	gbTable.AddIndicatorColumn(30301001,Tx::Core::dtype_double,varCfg,2); // 单位净值
	bool result = m_pLogicalBusiness->GetData(gbTable,true);
	if(result==false)
		return false;

	std::set<int> dates;
	for (std::set< std::pair<int,int> >::iterator iter=date.begin();iter!=date.end();iter++)
	{
		dates.insert (iter->first);
		dates.insert (iter->second);
	}

	Tx::Core::Table_Indicator tempTable1;
	tempTable1.CopyColumnInfoFrom(gbTable);
	gbTable.EqualsAt(tempTable1,nArray,3,1,dates); // 取出起始和截至日的单位净值
				strTable=tempTable1.TableToString();						//###############
				Tx::Core::Commonality::String().StringToClipboard(strTable);//###############
	Tx::Core::Table_Indicator navTempTable;
	navTempTable.CopyColumnInfoFrom(tempTable1);
	tempTable1.EqualsAt(navTempTable,nArray,3,0,iSecurityId); // 
				strTable=navTempTable.TableToString();						//###############
				Tx::Core::Commonality::String().StringToClipboard(strTable);//###############
	//----------------------------------------------------------------------------------
	// 2: 提取出净值调整系数
	Tx::Core::Table_Indicator fundAdjustTable;
	fundAdjustTable.AddParameterColumn(Tx::Core::dtype_int4);
	fundAdjustTable.AddParameterColumn(Tx::Core::dtype_int4);
	fundAdjustTable.AddIndicatorColumn(30301125,Tx::Core::dtype_double,varCfg,2);
	fundAdjustTable.AddIndicatorColumn(30301126,Tx::Core::dtype_double,varCfg,2);
	
	result = m_pLogicalBusiness->GetData(fundAdjustTable,true);
	if(result==false)
		return false;
	
	Tx::Core::Table_Indicator adjustTempTable;
	adjustTempTable.CopyColumnInfoFrom(fundAdjustTable);
	fundAdjustTable.EqualsAt(adjustTempTable,nArray,3,0,iSecurityId); // 
				strTable=adjustTempTable.TableToString();						//###############
				Tx::Core::Commonality::String().StringToClipboard(strTable);//###############
	//----------------------------------------------------------------------------------
	// 3: 计算净值增长率
	int k=0;
	for (std::vector<int>::iterator iter = iSecurityId.begin(); iter != iSecurityId.end (); iter++,k++)
	{
		resultTable.SetCell (0, k, *iter); // 第一列

		// ----------------------------------------------------------------------
		// 某只基金的全部单位净值
		Tx::Core::Table_Indicator navTempTable1;
		navTempTable1.CopyColumnInfoFrom(navTempTable);
		navTempTable.Between(navTempTable1,nArray,3,0,*iter,*iter,true,true); // 某只基金
				strTable=navTempTable1.TableToString();						//###############
				Tx::Core::Commonality::String().StringToClipboard(strTable);//###############
		// ----------------------------------------------------------------------
		// 每个区间的净值增长率
		int t=0;
		for (std::set< std::pair<int,int> >::iterator iter2=date.begin(); iter2 != date.end(); iter2++, t++)
		{
			// ----------------------------------------------------------------------
			// 前后两个净值
			Tx::Core::Table_Indicator navTempTable2;


			navTempTable2.CopyColumnInfoFrom(navTempTable1);
			navTempTable1.Between(navTempTable2,nArray,3,1,iter2->first,iter2->second,true,true);

				strTable=navTempTable2.TableToString();						//###############
				Tx::Core::Commonality::String().StringToClipboard(strTable);//###############
			double begin = 1.0;
			double end = 1.0;
			navTempTable2.GetCell (2, 0, begin);
			navTempTable2.GetCell (2, 1, end);

			// ----------------------------------------------------------------------
			// 净值调整因子
			Tx::Core::Table_Indicator adjustTempTable2;
			adjustTempTable2.CopyColumnInfoFrom(adjustTempTable);
			adjustTempTable.Between(adjustTempTable2,nArray,3,1,iter2->first,iter2->second,true,true); // 取出起始和截至日之间的净值调整系数

				strTable=adjustTempTable.TableToString();					//###############
				Tx::Core::Commonality::String().StringToClipboard(strTable);//###############

				strTable=adjustTempTable2.TableToString();					//###############
				Tx::Core::Commonality::String().StringToClipboard(strTable);//###############
			double adjust = 1.0;
			for (UINT i=adjustTempTable2.GetRowCount();i>0;i--)
			{
				double temp1 = 1.0;
				double temp2 = 1.0;
				adjustTempTable2.GetCell (2, i, temp1);
				adjustTempTable2.GetCell (3, i, temp2);
				adjust *= temp2/temp1;
			}

			// ----------------------------------------------------------------------
			// 净值增长率
			double dRet = end/begin*adjust - 1;
			resultTable.SetCell (t+1, k, dRet);
		}
	}

	return bRet;
}
*/

//计算一段时间区间基金的单位净值增长率
double FundDeriveData::CalcFundNvr_OneFund (int iSecurityId, int iBeginDate, int iEndDate, BOOL bFlag_Sryl /*= FALSE*/)
{
	std::vector<int> SecurityId;
	SecurityId.push_back (iSecurityId);

	std::set< std::pair<int,int> > date;
	date.insert (pair<int,int>(iBeginDate, iEndDate));

	Tx::Core::Table resultTable;
	bool bRet = CalcFundNvr (resultTable, SecurityId, date);

	double dRet = 0.0;
	resultTable.GetCell (1, 0, dRet);
	return dRet;
	
	/*
	UINT varCfg[2];			//参数配置
	int varCount=2;			//参数个数
	varCfg[0]=0;
	varCfg[1]=1;

	UINT nArray[3];
	nArray[0]=0;
	nArray[1]=1;
	nArray[2]=2;

	Tx::Core::Table_Indicator gbTable;// 取单位净值的table
	gbTable.AddParameterColumn(Tx::Core::dtype_int4);
	gbTable.AddParameterColumn(Tx::Core::dtype_int4);	//第二参数列:日期

	gbTable.AddIndicatorColumn(30301001,Tx::Core::dtype_double,varCfg,2); // 单位净值
	//gbTable.AddIndicatorColumn(30301002,Tx::Core::dtype_double,varCfg,2); // 累计净值

	//根据之前3个步骤的设置进行数据读取，结果数据存放在table中
	bool result = m_pLogicalBusiness->GetData(gbTable,true);
	if(result==false)
		return false;

	Tx::Core::Table_Indicator tempTable1;
	tempTable1.CopyColumnInfoFrom(gbTable);
	gbTable.Between(tempTable1,nArray,3,0,iSecurityId,iSecurityId,true,true); // 某只基金

	//CString str1 = tempTable1.TableToString ();
	//str1 = tempTable1.ColToString (2);
	//str1 = gbTable.ColToString (2);

	std::vector<int> date;
	date.push_back (iBeginDate);
	date.push_back (iEndDate);
	Tx::Core::Table_Indicator tempTable2;
	tempTable2.CopyColumnInfoFrom(gbTable);
	tempTable1.EqualsAt(tempTable2,nArray,3,1,date); // 取出起始和截至日的单位净值
	
	double begin = 1.0;
	double end = 1.0;
	tempTable2.GetCell (2, 0, begin);
	tempTable2.GetCell (2, 1, end);
	
	//
	//CString str = tempTable2.ColToString (0);
	//str = tempTable2.ColToString (1);
	//str = tempTable2.ColToString (2);
	//str = tempTable2.RowToString (0);
	//

	// 提取出净值调整系数
	Tx::Core::Table_Indicator fundAdjustTable;
	fundAdjustTable.AddParameterColumn(Tx::Core::dtype_int4);
	fundAdjustTable.AddParameterColumn(Tx::Core::dtype_int4);
	fundAdjustTable.AddIndicatorColumn(30301125,Tx::Core::dtype_double,varCfg,2);
	fundAdjustTable.AddIndicatorColumn(30301126,Tx::Core::dtype_double,varCfg,2);
	
	result = m_pLogicalBusiness->GetData(fundAdjustTable,true);
	if(result==false)
		return false;

	Tx::Core::Table_Indicator tempTable3;
	tempTable3.CopyColumnInfoFrom(fundAdjustTable);
	fundAdjustTable.Between(tempTable3,nArray,3,0,iSecurityId,iSecurityId,true,true); // 某只基金

	Tx::Core::Table_Indicator tempTable4;
	tempTable4.CopyColumnInfoFrom(fundAdjustTable);
	tempTable3.Between(tempTable4,nArray,3,1,iBeginDate,iEndDate,true,true); // 取出起始和截至日之间的净值调整系数

	double adjust = 1.0;
	for (UINT i=tempTable4.GetRowCount();i>0;i--)
	{
		double temp1 = 1.0;
		double temp2 = 1.0;
		tempTable4.GetCell (2, i, temp1);
		tempTable4.GetCell (3, i, temp2);
		adjust *= temp2/temp1;
	}

	double fRet = end/begin*adjust - 1;
	*/

	/*
	std::vector<int> iParameter;
	iParameter.push_back(30301001);	//基金单位净值
	iParameter.push_back(30301002);	//基金累计单位净值

	std::vector <int> iFundId;
	iFundId.push_back (iSecurityId);

	//设定这里需要的指标
	Tx::Business::IndicatorWithParameterArray m_IWPA;
	Tx::Business::IndicatorFile::GetInstance()->SetIWAP(m_IWPA,iParameter);
	Tx::Business::IndicatorFile::GetInstance()->GetData(m_IWPA,iFundId,true);
	//int i = m_IWPA.m_table_indicator.GetRowCount();
	//i = m_IWPA.m_table_indicator.GetColCount();

	Table_Indicator t_Indi;
	t_Indi.CopyColumnInfoFrom(m_IWPA.m_table_indicator);
	UINT* pColArray = new UINT [m_IWPA.m_table_indicator.GetColCount()];
	UINT nCount = 0;
	for (;nCount<m_IWPA.m_table_indicator.GetColCount();nCount++)
	{
		pColArray[nCount]=nCount;
	}
	// 0----人为指定,无法自动指定
	m_IWPA.m_table_indicator.EqualsAt(t_Indi,pColArray,nCount,0,iFundId);
	if (pColArray!=NULL)
	{
		delete pColArray;
		pColArray = NULL;
	}
//	tableIndicator.CopyColumnInfoFrom (m_IWPA.m_table_indicator, 
    */
}

//计算一只基金某阶段区间内，每个间隔时间的净值增长率
bool FundDeriveData::CalcFundNvr_OneFund (std::vector<double>& dNvr, int iSecurityId, std::set<int> & dates, int iBeginDate)
{
	std::vector<int> SecurityId;
	SecurityId.push_back (iSecurityId);

	std::set< std::pair<int,int> > date;
	for (std::set<int>::iterator iter = dates.begin(); iter != dates.end(); iter ++)
	{
		date.insert (pair<int,int>(iBeginDate, *iter));
		iBeginDate = *iter;
	}

	Tx::Core::Table resultTable;
	bool bRet = CalcFundNvr (resultTable, SecurityId, date);

	for (int i=0; i<(int)dates.size(); i++)
	{
		double dRet = 0.0;
		resultTable.GetCell (1+i, 0, dRet);
		dNvr.push_back(dRet);
	}

	return true;
}

//计算多只基金在某个时间区间内，净值增长率
bool FundDeriveData::CalcFundNvr_Funds (std::vector<double>& dNvr, std::vector<int> & iSecurityId, int iEndDate, int iBeginDate)
{
	std::set< std::pair<int,int> > date;
	date.insert (pair<int,int>(iBeginDate, iEndDate));

	Tx::Core::Table resultTable;
	bool bRet = CalcFundNvr (resultTable, iSecurityId, date);

	for (int i=0; i<(int)iSecurityId.size(); i++)
	{
		double dRet = 0.0;
		resultTable.GetCell (1, i, dRet);
		dNvr.push_back (dRet);
	}

	return true;
}

//计算一组封闭式基金，某个时间序列的折溢价率
bool FundDeriveData::CalcFundPremium (Tx::Core::Table& resultTable, std::vector<int> & iSecurityId, std::vector<int>& iDate)
{
	//---------------------------------------------------------------------------------
	bool bRet = true;

	if (iSecurityId.empty() || iDate.empty())
		return bRet;
	resultTable.Clear();
	resultTable.AddRow (iSecurityId.size());
	resultTable.AddCol(Tx::Core::dtype_int4); // 内码
	for (UINT i = 0;i<iDate.size();i++)
	{
		resultTable.AddCol(Tx::Core::dtype_double); // 各个区间的值
	}

	////---------------------------------------------------------------------------------
	//UINT varCfg[2];			//参数配置
	//int varCount=2;			//参数个数
	//varCfg[0]=0;
	//varCfg[1]=1;

	//UINT nArray[3];
	//nArray[0]=0;
	//nArray[1]=1;
	//nArray[2]=2;

	//----------------------------------------------------------------------------------
	// 1: 取净值
	Tx::Core::Table_Indicator navTable;
	navTable.AddCol(Tx::Core::dtype_int4);
	navTable.AddCol(Tx::Core::dtype_double);
//	navTable.AddParameterColumn(Tx::Core::dtype_int4);	//第二参数列:日期
//	navTable.AddIndicatorColumn(30301001,Tx::Core::dtype_double,varCfg,2); // 单位净值
//
//	bool result = m_pLogicalBusiness->GetData(navTable, iSecurityId, iDate, 1, 0);
//#ifdef _DEBUG
//	CString strTable=navTable.TableToString();
//	Tx::Core::Commonality::String().StringToClipboard(strTable);
//#endif
//	if(result==false)
//		return false;

	int iSize = sizeof(int) * (1+1+1+iSecurityId.size() + iDate.size());
	LPBYTE pBuffer = new BYTE [iSize];
	if (pBuffer == NULL)
		return false;

	LPBYTE pWrite = pBuffer;
	memset(pBuffer,0,iSize);
	int i=1;
	memcpy_s(pWrite,iSize,&i,sizeof(int));
	pWrite += sizeof(int);
	int nSecSize = (int)iSecurityId.size();
	memcpy_s(pWrite,iSize,&nSecSize,sizeof(int));
	pWrite += sizeof(int);
	for (UINT i=0;i<(UINT)nSecSize;i++)
	{
		memcpy_s(pWrite,iSize,&iSecurityId[i],sizeof(int));
		pWrite += sizeof(int);
	}

	nSecSize = iDate.size();
	memcpy_s(pWrite,iSize,&nSecSize,sizeof(int));
	pWrite += sizeof(int);

	for (UINT i=0;i<(UINT)nSecSize;i++)
	{
		memcpy_s(pWrite,iSize,&iDate[i],sizeof(int));
		pWrite += sizeof(int);
	}
	//memcpy_s(pWrite,iSize,&iEndDate,sizeof(int));

	//LPCTSTR lpUrl = _T("http://192.168.5.87/FundNavSer/ServerHandler.ashx");
	//LPCTSTR lpUrl = _T("http://192.168.6.89/FundNavSerBinary/ServerHandler.ashx");
	//LPCTSTR lpUrl = _T("http://221.122.41.211/FundNavSer/ServerHandler.ashx");
	LPCTSTR lpUrl = CORE_GETSYSINFO->GetServerAddr(_T("File"),_T("FundNavRaiseRate"));

	std::vector<double> dLjNav,dDwNav;
	Tx::Drive::Http::CSyncUpload upload;
	int iStart = ::GetTickCount();
	//	if ( upload.Post(lpUrl, (LPBYTE)lpwstr, nSize) )
	if ( upload.Post(lpUrl, pBuffer, iSize,_T("Content-Type: application/octet-stream")) )
	{
		int iEnd = ::GetTickCount();
		TRACE(_T("\r\nURL Cost Time %d(ms)\r\n"),iEnd-iStart);
		CONST Tx::Drive::Mem::MemSlice &data = upload.Rsp().Body();
		LPBYTE lpRes = data.DataPtr();
		UINT nRetSize = data.Size();
		if (nRetSize <= 0)
		{
			delete pBuffer;
			pBuffer = NULL;
			return false;
		}

		UINT nPreSize = *(reinterpret_cast<UINT*>(lpRes));
		LPBYTE lpData = new BYTE[nPreSize];
		if ( lpData == NULL )
		{
			delete pBuffer;
			pBuffer = NULL;
			return false;
		}
		if ( !Tx::Drive::IO::Zip::CZipWrapper::MemUnZip(lpData, 
			nPreSize, lpRes + sizeof(UINT), nRetSize - sizeof(UINT)) )
		{
			delete []lpData;
			delete[] pBuffer;
			pBuffer = NULL;
			return false;
		}

		iStart = ::GetTickCount();
		LPBYTE pRecv = lpData;
		UINT nParseCount = iSecurityId.size() * iDate.size();
		float fValue = 0.0;
		double dValue = 0.0;
		for (UINT i=0;i < nParseCount; i++)
		{
			memcpy_s(&fValue,sizeof(float),pRecv,sizeof(float));
			pRecv += sizeof(float);
			if (fabs(fValue - Tx::Core::Con_floatInvalid) < 0.00001)
				// 无效值
				resultTable.SetCell(i%iDate.size() + 1,i/iDate.size(),Tx::Core::Con_doubleInvalid);
			else
			{
				dValue = (double)fValue;
				resultTable.SetCell(i%iDate.size() + 1,i/iDate.size(),dValue);
			}
		}
		delete []lpData;
		lpData = NULL;
		iEnd = ::GetTickCount();
		TRACE(_T("\r\nParse Result Cost Time %d(ms)\r\n"),iEnd-iStart);
	}
	delete[] pBuffer;
	pBuffer = NULL;

	//----------------------------------------------------------------------------------
	// 2: 计算折溢价率
	int k=0;
	for (std::vector<int>::iterator iter = iSecurityId.begin(); iter != iSecurityId.end (); iter++,k++)
	{
		resultTable.SetCell (0, k, *iter); // 第一列
        //由于GetTradeDataByNatureDate这个接口不符合该业务的要求，所以把它改掉。deleted by lijw 2008-09-27
		//// ----------------------------------------------------------------------
		////取得指定交易实体的历史行情
		//GetSecurityHisTrade(*iter);
		////从历史行情数据中拆分出交易日期
		//GetSecurityTradeDate(m_pSecurity);
		//// ----------------------------------------------------------------------
		//add by lijw 2008-09-27
		GetSecurityNow(*iter);
		if (m_pSecurity == NULL)
			continue;
		// 每个日期的折溢价率
		int t=0;
		double dRet = 0.0;
		//for (std::vector<int>::iterator iter2=iDate.begin(); iter2 != iDate.end(); iter2++, t++)
		for (UINT j=0;j<iDate.size();j++)
		{
			// 某只基金指定日期的单位净值
			double dNav = 1.0;
			//int z = k*iDate.size() + t; // 注意：提取的数据次序
			//std::find(iDate.begin(),iDate.end(),*iter2);
			resultTable.GetCell (1+j, k, dNav);

			// 当日收盘价格
			//double dClosePrice = m_pSecurity->IsTradeDate(iDate[j]) ? m_pSecurity->GetClosePrice(iDate[j],false) : Con_doubleInvalid;
			//2012-10-16
			double dClosePrice = Con_doubleInvalid;
			int tempDate;
			if (m_pSecurity->IsFund_Close())  //封闭基金取每周五的收盘价  BUG:13546  / by wangzf  /2012-11-09
			{
				tempDate = Tx::Core::TxDate::CalculateFridayOfWeek(iDate[j]);
				if(tempDate == iDate[j])
					dClosePrice = m_pSecurity->IsTradeDate(iDate[j]) ? m_pSecurity->GetClosePrice(tempDate,false) : Con_doubleInvalid;
				else
				{
					tempDate = Tx::Core::TxDate::CalculateDateOffsetDays(tempDate,-7);
					while((m_pShIndex->GetTradeDateIndex(tempDate,0,0,true)) == -1)
						tempDate = Tx::Core::TxDate::CalculateDateOffsetDays(tempDate,-7);
					dClosePrice = m_pSecurity->IsTradeDate(iDate[j]) ? m_pSecurity->GetClosePrice(tempDate,false) : Con_doubleInvalid;
				}
			}
			else if(m_pSecurity->IsFund_ETF() || m_pSecurity->IsFund_LOF())
				dClosePrice = m_pSecurity->IsTradeDate(iDate[j]) ? m_pSecurity->GetClosePrice(iDate[j],false) : Con_doubleInvalid;
			else
				dClosePrice = m_pSecurity->IsTradeDate(iDate[j]) ? m_pSecurity->GetOpenFundClosePrice(iDate[j],false) : Con_doubleInvalid;


			//由于GetTradeDataByNatureDate这个接口不符合该业务的要求，所以把它改掉。
			//Tx::Data::HisTradeData* p = m_pSecurity->GetTradeDataByNatureDate (*iter2);
			
			//// 折溢价率
			//double dRet = 0.0;
			//if(p!=NULL)
			//	dRet= ((p->Close - dNav)/dNav)*100;
			//else
			//	dRet = Tx::Core::Con_doubleInvalid;
       
			//add by lijw 2008-09-27
			if(dClosePrice < 0.0 || dNav <= 0.0)
				dRet = Tx::Core::Con_doubleInvalid;
			else
			{
				dRet = (dClosePrice - dNav)/dNav*100;
			}
			t++;
			resultTable.SetCell (t, k, dRet);
		}
	}
	//add by lijw 2008-03-11
//#ifdef _DEBUG 
//	strTable=resultTable.TableToString();
//	Tx::Core::Commonality::String().StringToClipboard(strTable);
//#endif
	return true;
}

bool FundDeriveData::CalCloseFundNav(Tx::Core::Table& resultTable, std::vector<int> & iSecurityId, std::set< std::pair<int,int> >& date, bool bNva,bool bSim )
{
	Tx::Core::ProgressWnd prw;
	CString str = _T("获取收益率序列……");
	UINT	nProgId = prw.AddItem( 0, str );
	prw.Show( 1 );
	bool bRet = true;

	resultTable.Clear();
	resultTable.AddRow (iSecurityId.size());
	resultTable.AddCol(Tx::Core::dtype_int4); // 内码
	for (UINT i = 0;i<date.size();i++)
	{
		resultTable.AddCol(Tx::Core::dtype_double); // 各个区间的值
	}

	//---------------------------------------------------------------------------------
	UINT varCfg[2];			//参数配置
	int varCount=2;			//参数个数
	varCfg[0]=0;
	varCfg[1]=1;

	UINT nArray[4];
	nArray[0]=0;
	nArray[1]=1;
	nArray[2]=2;
	nArray[3]=3;

	//----------------------------------------------------------------------------------
	// 1: 取单位净值
	Tx::Core::Table_Indicator navTable;
	navTable.AddParameterColumn(Tx::Core::dtype_int4);
	navTable.AddParameterColumn(Tx::Core::dtype_int4);	//第二参数列:日期
	navTable.AddIndicatorColumn(30301001,Tx::Core::dtype_double,varCfg,2); // 单位净值

	std::vector<int> dates;
	std::set< std::pair<int,int> >::iterator iter;
	for (iter = date.begin();iter!=date.end();iter++)
	{
		dates.push_back (iter->first);
	}
	dates.push_back((--iter)->second);//add by lijw 2008-05-26
	bool result = m_pLogicalBusiness->GetData(navTable, iSecurityId, dates, 1, 0);
	if(result==false)
		return false;

	//----------------------------------------------------------------------------------
	// 2: 提取出净值调整系数
	Tx::Core::Table_Indicator fundAdjustTable;
	fundAdjustTable.AddParameterColumn(Tx::Core::dtype_int4);
	fundAdjustTable.AddParameterColumn(Tx::Core::dtype_int4);
	fundAdjustTable.AddIndicatorColumn(30301125,Tx::Core::dtype_double,varCfg,2);//调整前的净值
	fundAdjustTable.AddIndicatorColumn(30301126,Tx::Core::dtype_double,varCfg,2);//调整后的净值

	result = m_pLogicalBusiness->GetData(fundAdjustTable,true);
	if(result==false)
		return false;

	//----------------------------------------------------------------------------------
	// 3: 计算净值增长率
	int k=0;
	int nCount = iSecurityId.size();
	bool bClosePrice = false;
	for (std::vector<int>::iterator iter = iSecurityId.begin(); iter != iSecurityId.end (); iter++,k++)
	{
		resultTable.SetCell (0, k, *iter); // 第一列
		TxBusiness business;
		business.GetSecurityNow(*iter);
		if ( business.m_pSecurity == NULL)
			continue;
		bClosePrice = false;
		if ( bNva )
		{
			if (business.m_pSecurity->IsFund())
			{
				//按照净值
				bClosePrice = false;
			}
			else
			{
				//按照收盘价
				bClosePrice = true;

			}
		}
		else
		{
			if (business.m_pSecurity->IsFund_Open_Normal() || business.m_pSecurity->IsFund_QDII())
			{
				//按照净值
				bClosePrice = false;
			}
			else
			{
				//按照收盘
				bClosePrice = true;
			}
		}
		if ( bClosePrice )
		{
			int t=0;
			for (std::set< std::pair<int,int> >::iterator iter2=date.begin(); iter2 != date.end(); iter2++, t++)
			{
				// ----------------------------------------------------------------------
				// 前后两个收盘价
				double begin = 1.0;
				double end = 1.0;

				// ----------------------------------------------------------------------
				// 除权因子
				begin = business.m_pSecurity->GetClosePrice(iter2->first);
				end = business.m_pSecurity->GetClosePrice(iter2->second);
				end = end * business.m_pSecurity->GetExdividendScale(iter2->first,iter2->second,false);
				// 净值增长率
				double dRet = 0.0;
				if ( begin != 0 && end!= 0 && begin != Con_floatInvalid && end != Con_floatInvalid )
				{
					if ( bSim)
						dRet = end/begin - 1;	
					else
						dRet = log(end) -log(begin);
				}
				resultTable.SetCell (t+1, k, dRet);
			}
		}
		else
		{
			// 提取某基金的净值调整系数
			Tx::Core::Table_Indicator adjustTempTable1;
			adjustTempTable1.CopyColumnInfoFrom(fundAdjustTable);
			fundAdjustTable.Between(adjustTempTable1,nArray,4,0,*iter,*iter,true,true); // 某只基金
			if(adjustTempTable1.GetRowCount() == 0)
				continue;
			// ----------------------------------------------------------------------
			// 每个区间的净值增长率
			int t=0;
			std::vector<int> tempVector;        //存放临时的日期。
			for (std::set< std::pair<int,int> >::iterator iter2=date.begin(); iter2 != date.end(); iter2++, t++)
			{
				// ----------------------------------------------------------------------
				// 前后两个净值
				double begin = 1.0;
				double end = 1.0;
				int z = k*dates.size() + t; // 注意：提取的数据次序
				navTable.GetCell (2, z, begin);	//这个值取道可能为0	wangzhy
				navTable.GetCell (2, z+1, end);	//这个值取道可能为0	wangzhy

				// ----------------------------------------------------------------------
				// 净值调整因子
				Tx::Core::Table_Indicator adjustTempTable2;
				adjustTempTable2.CopyColumnInfoFrom(fundAdjustTable);
				if(!tempVector.empty())
					tempVector.clear();
				tempVector.push_back(iter2->second);
				//这里使用EqualAt可能有问题，应该使用Between--wangzhy-----
				//adjustTempTable1.EqualsAt(adjustTempTable2,nArray,4,1,tempVector);
				adjustTempTable1.Between(adjustTempTable2,nArray,4,1,iter2->first,iter2->second,true, false);
				double adjust = 1.0;
				for (UINT i=adjustTempTable2.GetRowCount();i>0;i--)//modify by lijw 2008-05-27 把1改为0；
				{
					double temp1 = 1.0;
					double temp2 = 1.0;
					adjustTempTable2.GetCell (2, i-1, temp1);
					adjustTempTable2.GetCell (3, i-1, temp2);
					if ( temp2 <= 0 )
						adjust = 1.0;
					else
						adjust *= temp1/temp2;	
				}					
				// 净值增长率
				double dRet = 0.0;
				if ( begin != 0 && end!= 0 )
				{
					if ( bSim )
						dRet = end/begin*adjust - 1;
					else
						dRet = log(end*adjust) - log(begin);
				}					
				resultTable.SetCell (t+1, k, dRet);
			}
		}
		prw.SetPercent( nProgId, double( k )/ nCount );
	}
	prw.SetPercent( nProgId, 1.0 );
	return bRet;
}


//计算某一组证券，某个时间序列的增长率序列，将碎文件转换为了后台计算
bool FundDeriveData::CalFundNav_Ext(Tx::Core::Table& resultTable, std::vector<int> & iSecurityId, std::set< std::pair<int,int> >& date,bool bNav,bool bSim )
{
	Tx::Core::ProgressWnd prw;
	UINT	nProgId = prw.AddItem( 0, _T("获取收益率序列……") );
	prw.Show( 1 );
	bool bRet = true;
	resultTable.Clear();

	std::vector<int>	vFund;
	std::vector<int>	vStock;

	Tx::Business::TxBusiness business;
	//这里将样本分两类，一部分通过后台计算读取，一部分利用文件计算
	if ( bNav )
	{
		for(std::vector<int>::iterator iter = iSecurityId.begin();iter != iSecurityId.end(); ++iter )
		{
			business.GetSecurityNow( *iter );
			ASSERT( business.m_pSecurity != NULL );
			if ( business.m_pSecurity == NULL )
				continue;
			if ( business.m_pSecurity->IsFund())
				vFund.push_back( *iter );
			else
				vStock.push_back( *iter );
		}
	}
	else
	{
		for(std::vector<int>::iterator iter = iSecurityId.begin();iter != iSecurityId.end(); ++iter )
		{
			business.GetSecurityNow( *iter );
			ASSERT( business.m_pSecurity != NULL );
			if ( business.m_pSecurity == NULL )
				continue;
			if ( business.m_pSecurity->IsFund() && business.m_pSecurity->IsFund_Open() )
				vFund.push_back( *iter );
			else
				vStock.push_back( *iter );
		}
	}
	//这里先从后台读取
	int nFundCount = (int)vFund.size();
	int nStockCount = (int)vStock.size();
	if ( nFundCount != 0 )
	{
		bool bFlag = CalcFundNvr( resultTable, vFund, date );
		if ( !bFlag)
			return false;
	}
	//本地计算	
	Tx::Core::Table stockTable;
	if ( nStockCount != 0 )
	{
		bool bFlag = CalStockRate( stockTable,vStock,date, bSim );
		if ( !bFlag )
			return false;	
	}
	//拼接结果
	if ( !bSim )
	{
		double dRet = 0.0;
		int	   nCol = resultTable.GetColCount();
		if ( nCol == 0 )
		{
			resultTable.AddCol(Tx::Core::dtype_int4); // 内码
			for (UINT i = 0;i<date.size();i++)
			{
				resultTable.AddCol(Tx::Core::dtype_double); // 各个区间的值
			}
		}
		for ( int i = 0; i < nFundCount; i++ )
		{
			for ( int j = 1; j < nCol;j++)
			{
				resultTable.GetCell(j,i,dRet);
				if ( fabs(dRet- Con_doubleInvalid) < 0.00001 )
					resultTable.SetCell(j,i,Con_doubleInvalid );
				else
					resultTable.SetCell(j,i,log(dRet/100+1));
			}
		}
	}
	else
	{
		double dRet = 0.0;
		int	   nCol = resultTable.GetColCount();
		if ( nCol == 0 )
		{
			resultTable.AddCol(Tx::Core::dtype_int4); // 内码
			for (UINT i = 0;i<date.size();i++)
			{
				resultTable.AddCol(Tx::Core::dtype_double); // 各个区间的值
			}
		}
#ifdef _DEBUG
		CString strTable=resultTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
		TRACE(_T("--------------------------\n"));
		for ( int i = 0; i < nFundCount; i++ )
		{
			for ( int j = 1; j < nCol;j++)
			{
				resultTable.GetCell(j,i,dRet);
				if ( fabs(dRet- Con_doubleInvalid) < 0.00001 )
					resultTable.SetCell(j,i,Con_doubleInvalid );
				else
				{				
					resultTable.SetCell(j,i,dRet/100);
					CString sTmp;
					sTmp.Format(_T("%f\n"),dRet/100);
					TRACE( sTmp );
				}

			}
		}
	}
#ifdef _DEBUG
	CString strTable=resultTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif

	if ( nStockCount != 0 )
	{
		int nStockRetCount = stockTable.GetRowCount();
		for ( int i = 0; i < nStockRetCount; i++ )
		{
			resultTable.AddRow();
			int nResultRowCount = resultTable.GetRowCount();
			int nResultColCount = resultTable.GetColCount();
			int nId = 0;
			double dRet = 0;
			stockTable.GetCell(0,i,nId );
			resultTable.SetCell(0,nResultRowCount-1,nId );
			for ( int j = 1; j < nResultColCount; j++ )
			{
				stockTable.GetCell(j,i,dRet );
				resultTable.SetCell(j,nResultRowCount-1,dRet );
			}
		}
	}
#ifdef _DEBUG
	strTable=resultTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	return true;
}

//计算股票某段时间内的增长序列
bool FundDeriveData::CalStockRate( Tx::Core::Table& resultTable, std::vector<int> & iSecurityId, std::set< std::pair<int,int> >& date,bool bSim )
{
	resultTable.Clear();
	int nCount = (int)iSecurityId.size();
	if ( nCount <= 0 )
		return false;
	
	resultTable.AddCol(Tx::Core::dtype_int4);
	for ( std::set< std::pair<int,int> >::iterator iter = date.begin(); iter  != date.end(); ++iter )
		resultTable.AddCol(Tx::Core::dtype_double);

	int nRow = 0;
	int nCol = 0;
	Tx::Business::TxBusiness stock;
	for ( std::vector<int>::iterator iter = iSecurityId.begin(); iter != iSecurityId.end(); ++iter )
	{
		stock.GetSecurityNow(*iter);
		if ( stock.m_pSecurity == NULL )
			continue;
		resultTable.AddRow();
		nCol = 0;
		resultTable.SetCell( nCol++,nRow,*iter);
		for ( std::set< std::pair<int,int> >::iterator itr = date.begin(); itr  != date.end(); ++itr )
		{
			// ----------------------------------------------------------------------
			// 前后两个收盘价
			double begin = 1.0;
			double end = 1.0;
			// ----------------------------------------------------------------------
			// 除权因子
			begin = stock.m_pSecurity->GetClosePrice(itr->first);
			end = stock.m_pSecurity->GetClosePrice(itr->second);
			end = end * stock.m_pSecurity->GetExdividendScale(itr->first,itr->second,false);
			// 增长率
			double dRet = 0.0;
			if ( begin != 0 && end!= 0 && begin != Con_floatInvalid && end != Con_floatInvalid )
			{
				if ( bSim)
					dRet = end/begin - 1;	
				else
					dRet = log(end) -log(begin);
			}
			resultTable.SetCell (nCol++, nRow, dRet);
		}
		nRow++;
	}
	return true;
}

//计算某一组基金，某个日期的单位净值、累计净值、收盘价、折溢价率，某个时间区间内，业绩基准收益率、平均折溢价率
bool FundDeriveData::CalFundData(Tx::Core::Table& resultTable, std::vector<int>& iSecurityId, std::vector<std::pair<int,int>>& date,int iEndDate,bool bAvg)
{
	bool bRet = true;
	int nDateSize = (int)date.size();
	int nIdSize = (int)iSecurityId.size();
	int nAvgType;
	int reqSize = sizeof(int) * (1/*是否请求平均折溢价率*/ + 1/*最新日期*/ + 1/*时间段个数*/ +nDateSize*2/*时间段集合*/ + 1/*样本个数*/ + nIdSize/*样本集合*/);
	LPBYTE lpBuffer = new BYTE[reqSize];
	if(lpBuffer == NULL)
		return false;
	LPBYTE lpWrite = lpBuffer;
	memset(lpBuffer,0,reqSize);
	if(bAvg)
		nAvgType = 1;
	else
		nAvgType = 0;
	memcpy_s(lpWrite,reqSize,&nAvgType,sizeof(int));
	lpWrite += sizeof(int);
	memcpy_s(lpWrite,reqSize,&iEndDate,sizeof(int));
	lpWrite += sizeof(int);
	memcpy_s(lpWrite,reqSize,&nDateSize,sizeof(int));
	lpWrite += sizeof(int);
	for (UINT i=0;i<(UINT)nDateSize;i++)
	{
		memcpy_s(lpWrite,reqSize,&date[i],sizeof(int)*2);
		lpWrite += sizeof(int)*2;
	}
	memcpy_s(lpWrite,reqSize,&nIdSize,sizeof(int));
	lpWrite += sizeof(int);
	for (UINT i=0;i<(UINT)nIdSize;i++)
	{
		memcpy_s(lpWrite,reqSize,&iSecurityId[i],sizeof(int));
		lpWrite += sizeof(int);
	}

	
	CString strServer = _T("");
	CString strSvrList;
	CString strAddress;
	strSvrList.Format(_T("%sServerList%d.ini"),Tx::Core::SystemPath::GetInstance()->GetExePath(),Tx::Core::UserInfo::GetInstance()->GetNetId());
	TCHAR tempch[200];
	memset(tempch,0,200);
	if (GetPrivateProfileString(_T("StatServer"), _T("FundStat"), strServer, tempch, 200, strSvrList) > 0) 
		strServer.Format(_T("%s"), tempch);
	else
		return false;

	strAddress.Format(_T("%s/%d.aspx"), strServer, 10528);
	LPCTSTR lpUrl = strAddress;

	Tx::Drive::Http::CSyncUpload upload;
	int iReqStart = ::GetTickCount();
	if (upload.Post(lpUrl,lpBuffer,reqSize,_T("Content-Type: application/octet-stream")))
	{
		int iReqEnd = ::GetTickCount();
		TRACE(_T("\r\nJZFX URL Cost Time %d(ms)\r\n"),iReqEnd-iReqStart);
		CONST Tx::Drive::Mem::MemSlice &mData = upload.Rsp().Body();
		LPBYTE lpRes = mData.DataPtr();
		UINT nRetSize = mData.Size();
		// zhangw 2014-01-06
		// 针对这个应用，nRetSize至少要大于sizeof(int)
		if (nRetSize <= 4)
		{
			delete lpBuffer;
			lpBuffer = NULL;
			return false;
		}

		INT32 nPreSize = *(reinterpret_cast<INT32*>(lpRes));  //数据大小
		if(nPreSize<=0)
			return false;
		LPBYTE lpData = new BYTE[nPreSize];
		if (lpData == NULL)
		{
			delete lpData;
			lpData = NULL;
			return false;
		}
		if ( !Tx::Drive::IO::Zip::CZipWrapper::MemUnZip(lpData, 
			(UINT&)nPreSize, lpRes + sizeof(UINT), nRetSize - sizeof(UINT)) )
		{
			delete []lpData;
			delete[] lpBuffer;
			lpBuffer = NULL;
			return false;
		}
		iReqStart = ::GetTickCount();
		LPBYTE pRecv = lpData;
		UINT nParseRow = nIdSize;
		UINT nParseCol = 5+(UINT)nDateSize; 
		if(bAvg)
			nParseCol = 5+nDateSize*2;

		float fValue1 = 0.0;
		double dValue1 = 0.0;
		int nValue = 0;

		resultTable.Clear();
		resultTable.AddRow(nParseRow);
		resultTable.AddCol(Tx::Core::dtype_int4);//内码
		for (UINT i=1;i<5;i++)
		{
			resultTable.AddCol(Tx::Core::dtype_double);
		}

		for (UINT i=5;i<nParseCol;i++)
		{
			if (bAvg)
			{
				resultTable.AddCol(Tx::Core::dtype_double);
				i++;
				resultTable.AddCol(Tx::Core::dtype_double);
			}
			else
			{
				resultTable.AddCol(Tx::Core::dtype_double);
			}
		}
		for (UINT r=0;r<nParseRow;r++)
		{
			for (UINT c=0;c<nParseCol;c++)
			{
				if (c == 0)
				{
					memcpy_s(&nValue,sizeof(int),pRecv,sizeof(int));
					if(nValue==Tx::Core::Con_intInvalid)
						resultTable.SetCell(c,r,Tx::Core::Con_intInvalid);
					else
						resultTable.SetCell(c,r,nValue);
					pRecv += sizeof(int);
				}
				else
				{
					memcpy_s(&fValue1,sizeof(float),pRecv,sizeof(float));
					if(fabs(fValue1-Tx::Core::Con_floatInvalid) < 0.00001)
					{
						resultTable.SetCell(c,r,Tx::Core::Con_doubleInvalid);
					}
					else
					{
						dValue1 = (double)fValue1;
						resultTable.SetCell(c,r,dValue1);
					}
					pRecv += sizeof(float);
				}
			}
		}
		delete []lpData;
		lpData = NULL;
		delete []lpBuffer;
		lpBuffer = NULL;
		iReqEnd = ::GetTickCount();
		TRACE(_T("\r\nZZFX Parse Result Cost Time %d(ms)\r\n"),iReqEnd-iReqStart);
#ifdef _DEBUG
		CString strTable1=resultTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(strTable1);
#endif
		return bRet;
	}
	return false;
}
}
}


