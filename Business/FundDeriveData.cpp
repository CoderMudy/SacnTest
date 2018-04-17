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

//����һ�����ÿ�����ʱ��ľ�ֵ�����ʵļ�Ȩֵ
bool FundDeriveData::CalcWeightedFundNvr (std::vector<double> & dWeighted, Tx::Core::Table& resultTable, std::vector<int> & iSecurityId, std::set< std::pair<int,int> >& date)
{
	//---------------------------------------------------------------------------------
	bool bRet = true;

	UINT varCfg[2];			//��������
	int varCount=2;			//��������
	varCfg[0]=0;
	varCfg[1]=1;

	UINT nArray[3];
	nArray[0]=0;
	nArray[1]=1;
	nArray[2]=2;

	//----------------------------------------------------------------------------------
	// 1: ȡ����ķݶ�
	Tx::Core::Table_Indicator shareTable;
	shareTable.AddParameterColumn(Tx::Core::dtype_int4);
	shareTable.AddParameterColumn(Tx::Core::dtype_int4);	//�ڶ�������:����
	shareTable.AddIndicatorColumn(30301111,Tx::Core::dtype_double,varCfg,2); // �ܷݶ�

	std::vector<int> dates;
	for (std::set< std::pair<int,int> >::iterator iter=date.begin();iter!=date.end();iter++)
		dates.push_back (iter->second);

	bool result = m_pLogicalBusiness->GetData(shareTable, iSecurityId, dates, 1, 0);
	if(result==false)
		return false;

	// 2: �����Ȩ�ľ�ֵ������
	for (int k=1; k<(int)resultTable.GetColCount (); k++)
	{
		double dRet = 0.0;
		for (int t=0; t<(int)resultTable.GetRowCount(); t++)
		{
			double dTemp1 = 0.0;
			double dTemp2 = 0.0;
			resultTable.GetCell (k, t, dTemp1);

			int z = t*(resultTable.GetRowCount()-1) + (k-1); // ע�⣺��ȡ�����ݴ���
			shareTable.GetCell (2, z, dTemp2);

			dRet += dTemp1 * dTemp2;
		}

		dWeighted.push_back (dRet);
	}

	return true;
}
//moldified by lijw 2008-05-26
//����һ�����ĳ�����ڣ�ÿ�����ʱ��ĵ�λ��ֵ������
bool FundDeriveData::CalcFundNvr (Tx::Core::Table& resultTable, std::vector<int> & iSecurityId, std::set< std::pair<int,int> >& date)
{
//	Tx::Core::ProgressWnd prw;
	//UINT	nProgId = prw.AddItem( 0, _T("��ȡ���������С���") );
	//prw.Show( 1 );
	bool bRet = true;

	resultTable.Clear();
	resultTable.AddRow (iSecurityId.size());
	resultTable.AddCol(Tx::Core::dtype_int4); // ����
	for (UINT i = 0;i<date.size();i++)
	{
		resultTable.AddCol(Tx::Core::dtype_double); // ���������ֵ
	}

	// �������µľ�ֵ�����ʵ��ýӿ�
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
//	// ȥ�����һ��','
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
	//// ���Ծ�ֵ
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

		//// ������� id1,value1,value2....,id2,value1,value2,
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
				// ��Чֵ
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
//	UINT varCfg[2];			//��������
//	int varCount=2;			//��������
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
//	// 1: ȡ��λ��ֵ
//	Tx::Core::Table_Indicator navTable;
//	navTable.AddParameterColumn(Tx::Core::dtype_int4);
//	navTable.AddParameterColumn(Tx::Core::dtype_int4);	//�ڶ�������:����
//	navTable.AddIndicatorColumn(30301001,Tx::Core::dtype_double,varCfg,2); // ��λ��ֵ
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
//	// 2: ��ȡ����ֵ����ϵ��
//	Tx::Core::Table_Indicator fundAdjustTable;
//	fundAdjustTable.AddParameterColumn(Tx::Core::dtype_int4);
//	fundAdjustTable.AddParameterColumn(Tx::Core::dtype_int4);
//	fundAdjustTable.AddIndicatorColumn(30301125,Tx::Core::dtype_double,varCfg,2);//����ǰ�ľ�ֵ
//	fundAdjustTable.AddIndicatorColumn(30301126,Tx::Core::dtype_double,varCfg,2);//������ľ�ֵ
//
//	result = m_pLogicalBusiness->GetData(fundAdjustTable,true);
//	if(result==false)
//		return false;
//#ifdef _DEBUG
//	strTable4=navTable.TableToString();
//	Tx::Core::Commonality::String().StringToClipboard(strTable4);
//#endif	
//	//----------------------------------------------------------------------------------
//	// 3: ���㾻ֵ������
//	int k=0;
//	int nCount = iSecurityId.size();
//	for (std::vector<int>::iterator iter = iSecurityId.begin(); iter != iSecurityId.end (); iter++,k++)
//	{
//		resultTable.SetCell (0, k, *iter); // ��һ��
//
//		// ��ȡĳ����ľ�ֵ����ϵ��
//		Tx::Core::Table_Indicator adjustTempTable1;
//		adjustTempTable1.CopyColumnInfoFrom(fundAdjustTable);
//		fundAdjustTable.Between(adjustTempTable1,nArray,4,0,*iter,*iter,true,true); // ĳֻ����
//		/*if(adjustTempTable1.GetRowCount() == 0)
//			continue;*/
//         //add by lijw 2008-08-20
//		if(adjustTempTable1.GetRowCount() == 0)
//		{
//			// ÿ������ľ�ֵ������
//			int t=0;
//			for (std::set< std::pair<int,int> >::iterator iter2=date.begin(); iter2 != date.end(); iter2++, t++)
//			{
//				// ----------------------------------------------------------------------
//				// ǰ��������ֵ
//				double begin = 1.0;
//				double end = 1.0;
//				int z = k*dates.size() + t; // ע�⣺��ȡ�����ݴ���
//				navTable.GetCell (2, z, begin);	//���ֵȡ������Ϊ0	wangzhy
//				navTable.GetCell (2, z+1, end);	//���ֵȡ������Ϊ0	wangzhy
//				double adjust = 1.0;				
//				// ----------------------------------------------------------------------
//				// ��ֵ������
//				double dRet = 0.0;
//				if ( begin != 0 )
//					dRet = end/begin*adjust - 1;
//				resultTable.SetCell (t+1, k, dRet);
//			}
//			continue;
//		}
//		// ----------------------------------------------------------------------
//		// ÿ������ľ�ֵ������
//		int t=0;
////		std::vector<int> tempVector;        //�����ʱ�����ڡ�
//		for (std::set< std::pair<int,int> >::iterator iter2=date.begin(); iter2 != date.end(); iter2++, t++)
//		{
//			// ----------------------------------------------------------------------
//			// ǰ��������ֵ
//			double begin = 1.0;
//			double end = 1.0;
//			int z = k*dates.size() + t; // ע�⣺��ȡ�����ݴ���
//			navTable.GetCell (2, z, begin);	//���ֵȡ������Ϊ0	wangzhy
//			navTable.GetCell (2, z+1, end);	//���ֵȡ������Ϊ0	wangzhy
//
//			// ----------------------------------------------------------------------
//			// ��ֵ��������
//			Tx::Core::Table_Indicator adjustTempTable2;
//			adjustTempTable2.CopyColumnInfoFrom(fundAdjustTable);
//			//modify  by lijw 2008-05-27 ��
//			adjustTempTable1.Between(adjustTempTable2,nArray,4,1,iter2->first,iter2->second,true,false); // ȡ����ʼ�ͽ�����֮��ľ�ֵ����ϵ��
//	/*		if(!tempVector.empty())
//				tempVector.clear();
//			tempVector.push_back(iter2->second);
//			adjustTempTable1.EqualsAt(adjustTempTable2,nArray,4,1,tempVector);*/
//			double adjust = 1.0;
//			for (UINT i=adjustTempTable2.GetRowCount();i>0;i--)//modify by lijw 2008-05-27 ��1��Ϊ0��
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
//			// ��ֵ������
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

//����һ�����ĳ�����ڣ�ÿ�����ʱ��ĵ�λ��ֵ������
/*
{
	bool bRet = true;

	CString strTable;
	resultTable.Clear();
	resultTable.AddRow (iSecurityId.size());
	resultTable.AddCol(Tx::Core::dtype_int4); // ����
	for (UINT i = 0;i<date.size();i++)
	{
		resultTable.AddCol(Tx::Core::dtype_double); // ���������ֵ
	}

	//---------------------------------------------------------------------------------
	UINT varCfg[2];			//��������
	int varCount=2;			//��������
	varCfg[0]=0;
	varCfg[1]=1;

	UINT nArray[3];
	nArray[0]=0;
	nArray[1]=1;
	nArray[2]=2;

	//----------------------------------------------------------------------------------
	// 1: ȡ��λ��ֵ
	Tx::Core::Table_Indicator gbTable;
	gbTable.AddParameterColumn(Tx::Core::dtype_int4);
	gbTable.AddParameterColumn(Tx::Core::dtype_int4);	//�ڶ�������:����

	gbTable.AddIndicatorColumn(30301001,Tx::Core::dtype_double,varCfg,2); // ��λ��ֵ
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
	gbTable.EqualsAt(tempTable1,nArray,3,1,dates); // ȡ����ʼ�ͽ����յĵ�λ��ֵ
				strTable=tempTable1.TableToString();						//###############
				Tx::Core::Commonality::String().StringToClipboard(strTable);//###############
	Tx::Core::Table_Indicator navTempTable;
	navTempTable.CopyColumnInfoFrom(tempTable1);
	tempTable1.EqualsAt(navTempTable,nArray,3,0,iSecurityId); // 
				strTable=navTempTable.TableToString();						//###############
				Tx::Core::Commonality::String().StringToClipboard(strTable);//###############
	//----------------------------------------------------------------------------------
	// 2: ��ȡ����ֵ����ϵ��
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
	// 3: ���㾻ֵ������
	int k=0;
	for (std::vector<int>::iterator iter = iSecurityId.begin(); iter != iSecurityId.end (); iter++,k++)
	{
		resultTable.SetCell (0, k, *iter); // ��һ��

		// ----------------------------------------------------------------------
		// ĳֻ�����ȫ����λ��ֵ
		Tx::Core::Table_Indicator navTempTable1;
		navTempTable1.CopyColumnInfoFrom(navTempTable);
		navTempTable.Between(navTempTable1,nArray,3,0,*iter,*iter,true,true); // ĳֻ����
				strTable=navTempTable1.TableToString();						//###############
				Tx::Core::Commonality::String().StringToClipboard(strTable);//###############
		// ----------------------------------------------------------------------
		// ÿ������ľ�ֵ������
		int t=0;
		for (std::set< std::pair<int,int> >::iterator iter2=date.begin(); iter2 != date.end(); iter2++, t++)
		{
			// ----------------------------------------------------------------------
			// ǰ��������ֵ
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
			// ��ֵ��������
			Tx::Core::Table_Indicator adjustTempTable2;
			adjustTempTable2.CopyColumnInfoFrom(adjustTempTable);
			adjustTempTable.Between(adjustTempTable2,nArray,3,1,iter2->first,iter2->second,true,true); // ȡ����ʼ�ͽ�����֮��ľ�ֵ����ϵ��

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
			// ��ֵ������
			double dRet = end/begin*adjust - 1;
			resultTable.SetCell (t+1, k, dRet);
		}
	}

	return bRet;
}
*/

//����һ��ʱ���������ĵ�λ��ֵ������
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
	UINT varCfg[2];			//��������
	int varCount=2;			//��������
	varCfg[0]=0;
	varCfg[1]=1;

	UINT nArray[3];
	nArray[0]=0;
	nArray[1]=1;
	nArray[2]=2;

	Tx::Core::Table_Indicator gbTable;// ȡ��λ��ֵ��table
	gbTable.AddParameterColumn(Tx::Core::dtype_int4);
	gbTable.AddParameterColumn(Tx::Core::dtype_int4);	//�ڶ�������:����

	gbTable.AddIndicatorColumn(30301001,Tx::Core::dtype_double,varCfg,2); // ��λ��ֵ
	//gbTable.AddIndicatorColumn(30301002,Tx::Core::dtype_double,varCfg,2); // �ۼƾ�ֵ

	//����֮ǰ3����������ý������ݶ�ȡ��������ݴ����table��
	bool result = m_pLogicalBusiness->GetData(gbTable,true);
	if(result==false)
		return false;

	Tx::Core::Table_Indicator tempTable1;
	tempTable1.CopyColumnInfoFrom(gbTable);
	gbTable.Between(tempTable1,nArray,3,0,iSecurityId,iSecurityId,true,true); // ĳֻ����

	//CString str1 = tempTable1.TableToString ();
	//str1 = tempTable1.ColToString (2);
	//str1 = gbTable.ColToString (2);

	std::vector<int> date;
	date.push_back (iBeginDate);
	date.push_back (iEndDate);
	Tx::Core::Table_Indicator tempTable2;
	tempTable2.CopyColumnInfoFrom(gbTable);
	tempTable1.EqualsAt(tempTable2,nArray,3,1,date); // ȡ����ʼ�ͽ����յĵ�λ��ֵ
	
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

	// ��ȡ����ֵ����ϵ��
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
	fundAdjustTable.Between(tempTable3,nArray,3,0,iSecurityId,iSecurityId,true,true); // ĳֻ����

	Tx::Core::Table_Indicator tempTable4;
	tempTable4.CopyColumnInfoFrom(fundAdjustTable);
	tempTable3.Between(tempTable4,nArray,3,1,iBeginDate,iEndDate,true,true); // ȡ����ʼ�ͽ�����֮��ľ�ֵ����ϵ��

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
	iParameter.push_back(30301001);	//����λ��ֵ
	iParameter.push_back(30301002);	//�����ۼƵ�λ��ֵ

	std::vector <int> iFundId;
	iFundId.push_back (iSecurityId);

	//�趨������Ҫ��ָ��
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
	// 0----��Ϊָ��,�޷��Զ�ָ��
	m_IWPA.m_table_indicator.EqualsAt(t_Indi,pColArray,nCount,0,iFundId);
	if (pColArray!=NULL)
	{
		delete pColArray;
		pColArray = NULL;
	}
//	tableIndicator.CopyColumnInfoFrom (m_IWPA.m_table_indicator, 
    */
}

//����һֻ����ĳ�׶������ڣ�ÿ�����ʱ��ľ�ֵ������
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

//�����ֻ������ĳ��ʱ�������ڣ���ֵ������
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

//����һ����ʽ����ĳ��ʱ�����е��������
bool FundDeriveData::CalcFundPremium (Tx::Core::Table& resultTable, std::vector<int> & iSecurityId, std::vector<int>& iDate)
{
	//---------------------------------------------------------------------------------
	bool bRet = true;

	if (iSecurityId.empty() || iDate.empty())
		return bRet;
	resultTable.Clear();
	resultTable.AddRow (iSecurityId.size());
	resultTable.AddCol(Tx::Core::dtype_int4); // ����
	for (UINT i = 0;i<iDate.size();i++)
	{
		resultTable.AddCol(Tx::Core::dtype_double); // ���������ֵ
	}

	////---------------------------------------------------------------------------------
	//UINT varCfg[2];			//��������
	//int varCount=2;			//��������
	//varCfg[0]=0;
	//varCfg[1]=1;

	//UINT nArray[3];
	//nArray[0]=0;
	//nArray[1]=1;
	//nArray[2]=2;

	//----------------------------------------------------------------------------------
	// 1: ȡ��ֵ
	Tx::Core::Table_Indicator navTable;
	navTable.AddCol(Tx::Core::dtype_int4);
	navTable.AddCol(Tx::Core::dtype_double);
//	navTable.AddParameterColumn(Tx::Core::dtype_int4);	//�ڶ�������:����
//	navTable.AddIndicatorColumn(30301001,Tx::Core::dtype_double,varCfg,2); // ��λ��ֵ
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
				// ��Чֵ
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
	// 2: �����������
	int k=0;
	for (std::vector<int>::iterator iter = iSecurityId.begin(); iter != iSecurityId.end (); iter++,k++)
	{
		resultTable.SetCell (0, k, *iter); // ��һ��
        //����GetTradeDataByNatureDate����ӿڲ����ϸ�ҵ���Ҫ�����԰����ĵ���deleted by lijw 2008-09-27
		//// ----------------------------------------------------------------------
		////ȡ��ָ������ʵ�����ʷ����
		//GetSecurityHisTrade(*iter);
		////����ʷ���������в�ֳ���������
		//GetSecurityTradeDate(m_pSecurity);
		//// ----------------------------------------------------------------------
		//add by lijw 2008-09-27
		GetSecurityNow(*iter);
		if (m_pSecurity == NULL)
			continue;
		// ÿ�����ڵ��������
		int t=0;
		double dRet = 0.0;
		//for (std::vector<int>::iterator iter2=iDate.begin(); iter2 != iDate.end(); iter2++, t++)
		for (UINT j=0;j<iDate.size();j++)
		{
			// ĳֻ����ָ�����ڵĵ�λ��ֵ
			double dNav = 1.0;
			//int z = k*iDate.size() + t; // ע�⣺��ȡ�����ݴ���
			//std::find(iDate.begin(),iDate.end(),*iter2);
			resultTable.GetCell (1+j, k, dNav);

			// �������̼۸�
			//double dClosePrice = m_pSecurity->IsTradeDate(iDate[j]) ? m_pSecurity->GetClosePrice(iDate[j],false) : Con_doubleInvalid;
			//2012-10-16
			double dClosePrice = Con_doubleInvalid;
			int tempDate;
			if (m_pSecurity->IsFund_Close())  //��ջ���ȡÿ��������̼�  BUG:13546  / by wangzf  /2012-11-09
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


			//����GetTradeDataByNatureDate����ӿڲ����ϸ�ҵ���Ҫ�����԰����ĵ���
			//Tx::Data::HisTradeData* p = m_pSecurity->GetTradeDataByNatureDate (*iter2);
			
			//// �������
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
	CString str = _T("��ȡ���������С���");
	UINT	nProgId = prw.AddItem( 0, str );
	prw.Show( 1 );
	bool bRet = true;

	resultTable.Clear();
	resultTable.AddRow (iSecurityId.size());
	resultTable.AddCol(Tx::Core::dtype_int4); // ����
	for (UINT i = 0;i<date.size();i++)
	{
		resultTable.AddCol(Tx::Core::dtype_double); // ���������ֵ
	}

	//---------------------------------------------------------------------------------
	UINT varCfg[2];			//��������
	int varCount=2;			//��������
	varCfg[0]=0;
	varCfg[1]=1;

	UINT nArray[4];
	nArray[0]=0;
	nArray[1]=1;
	nArray[2]=2;
	nArray[3]=3;

	//----------------------------------------------------------------------------------
	// 1: ȡ��λ��ֵ
	Tx::Core::Table_Indicator navTable;
	navTable.AddParameterColumn(Tx::Core::dtype_int4);
	navTable.AddParameterColumn(Tx::Core::dtype_int4);	//�ڶ�������:����
	navTable.AddIndicatorColumn(30301001,Tx::Core::dtype_double,varCfg,2); // ��λ��ֵ

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
	// 2: ��ȡ����ֵ����ϵ��
	Tx::Core::Table_Indicator fundAdjustTable;
	fundAdjustTable.AddParameterColumn(Tx::Core::dtype_int4);
	fundAdjustTable.AddParameterColumn(Tx::Core::dtype_int4);
	fundAdjustTable.AddIndicatorColumn(30301125,Tx::Core::dtype_double,varCfg,2);//����ǰ�ľ�ֵ
	fundAdjustTable.AddIndicatorColumn(30301126,Tx::Core::dtype_double,varCfg,2);//������ľ�ֵ

	result = m_pLogicalBusiness->GetData(fundAdjustTable,true);
	if(result==false)
		return false;

	//----------------------------------------------------------------------------------
	// 3: ���㾻ֵ������
	int k=0;
	int nCount = iSecurityId.size();
	bool bClosePrice = false;
	for (std::vector<int>::iterator iter = iSecurityId.begin(); iter != iSecurityId.end (); iter++,k++)
	{
		resultTable.SetCell (0, k, *iter); // ��һ��
		TxBusiness business;
		business.GetSecurityNow(*iter);
		if ( business.m_pSecurity == NULL)
			continue;
		bClosePrice = false;
		if ( bNva )
		{
			if (business.m_pSecurity->IsFund())
			{
				//���վ�ֵ
				bClosePrice = false;
			}
			else
			{
				//�������̼�
				bClosePrice = true;

			}
		}
		else
		{
			if (business.m_pSecurity->IsFund_Open_Normal() || business.m_pSecurity->IsFund_QDII())
			{
				//���վ�ֵ
				bClosePrice = false;
			}
			else
			{
				//��������
				bClosePrice = true;
			}
		}
		if ( bClosePrice )
		{
			int t=0;
			for (std::set< std::pair<int,int> >::iterator iter2=date.begin(); iter2 != date.end(); iter2++, t++)
			{
				// ----------------------------------------------------------------------
				// ǰ���������̼�
				double begin = 1.0;
				double end = 1.0;

				// ----------------------------------------------------------------------
				// ��Ȩ����
				begin = business.m_pSecurity->GetClosePrice(iter2->first);
				end = business.m_pSecurity->GetClosePrice(iter2->second);
				end = end * business.m_pSecurity->GetExdividendScale(iter2->first,iter2->second,false);
				// ��ֵ������
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
			// ��ȡĳ����ľ�ֵ����ϵ��
			Tx::Core::Table_Indicator adjustTempTable1;
			adjustTempTable1.CopyColumnInfoFrom(fundAdjustTable);
			fundAdjustTable.Between(adjustTempTable1,nArray,4,0,*iter,*iter,true,true); // ĳֻ����
			if(adjustTempTable1.GetRowCount() == 0)
				continue;
			// ----------------------------------------------------------------------
			// ÿ������ľ�ֵ������
			int t=0;
			std::vector<int> tempVector;        //�����ʱ�����ڡ�
			for (std::set< std::pair<int,int> >::iterator iter2=date.begin(); iter2 != date.end(); iter2++, t++)
			{
				// ----------------------------------------------------------------------
				// ǰ��������ֵ
				double begin = 1.0;
				double end = 1.0;
				int z = k*dates.size() + t; // ע�⣺��ȡ�����ݴ���
				navTable.GetCell (2, z, begin);	//���ֵȡ������Ϊ0	wangzhy
				navTable.GetCell (2, z+1, end);	//���ֵȡ������Ϊ0	wangzhy

				// ----------------------------------------------------------------------
				// ��ֵ��������
				Tx::Core::Table_Indicator adjustTempTable2;
				adjustTempTable2.CopyColumnInfoFrom(fundAdjustTable);
				if(!tempVector.empty())
					tempVector.clear();
				tempVector.push_back(iter2->second);
				//����ʹ��EqualAt���������⣬Ӧ��ʹ��Between--wangzhy-----
				//adjustTempTable1.EqualsAt(adjustTempTable2,nArray,4,1,tempVector);
				adjustTempTable1.Between(adjustTempTable2,nArray,4,1,iter2->first,iter2->second,true, false);
				double adjust = 1.0;
				for (UINT i=adjustTempTable2.GetRowCount();i>0;i--)//modify by lijw 2008-05-27 ��1��Ϊ0��
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
				// ��ֵ������
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


//����ĳһ��֤ȯ��ĳ��ʱ�����е����������У������ļ�ת��Ϊ�˺�̨����
bool FundDeriveData::CalFundNav_Ext(Tx::Core::Table& resultTable, std::vector<int> & iSecurityId, std::set< std::pair<int,int> >& date,bool bNav,bool bSim )
{
	Tx::Core::ProgressWnd prw;
	UINT	nProgId = prw.AddItem( 0, _T("��ȡ���������С���") );
	prw.Show( 1 );
	bool bRet = true;
	resultTable.Clear();

	std::vector<int>	vFund;
	std::vector<int>	vStock;

	Tx::Business::TxBusiness business;
	//���ｫ���������࣬һ����ͨ����̨�����ȡ��һ���������ļ�����
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
	//�����ȴӺ�̨��ȡ
	int nFundCount = (int)vFund.size();
	int nStockCount = (int)vStock.size();
	if ( nFundCount != 0 )
	{
		bool bFlag = CalcFundNvr( resultTable, vFund, date );
		if ( !bFlag)
			return false;
	}
	//���ؼ���	
	Tx::Core::Table stockTable;
	if ( nStockCount != 0 )
	{
		bool bFlag = CalStockRate( stockTable,vStock,date, bSim );
		if ( !bFlag )
			return false;	
	}
	//ƴ�ӽ��
	if ( !bSim )
	{
		double dRet = 0.0;
		int	   nCol = resultTable.GetColCount();
		if ( nCol == 0 )
		{
			resultTable.AddCol(Tx::Core::dtype_int4); // ����
			for (UINT i = 0;i<date.size();i++)
			{
				resultTable.AddCol(Tx::Core::dtype_double); // ���������ֵ
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
			resultTable.AddCol(Tx::Core::dtype_int4); // ����
			for (UINT i = 0;i<date.size();i++)
			{
				resultTable.AddCol(Tx::Core::dtype_double); // ���������ֵ
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

//�����Ʊĳ��ʱ���ڵ���������
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
			// ǰ���������̼�
			double begin = 1.0;
			double end = 1.0;
			// ----------------------------------------------------------------------
			// ��Ȩ����
			begin = stock.m_pSecurity->GetClosePrice(itr->first);
			end = stock.m_pSecurity->GetClosePrice(itr->second);
			end = end * stock.m_pSecurity->GetExdividendScale(itr->first,itr->second,false);
			// ������
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

//����ĳһ�����ĳ�����ڵĵ�λ��ֵ���ۼƾ�ֵ�����̼ۡ�������ʣ�ĳ��ʱ�������ڣ�ҵ����׼�����ʡ�ƽ���������
bool FundDeriveData::CalFundData(Tx::Core::Table& resultTable, std::vector<int>& iSecurityId, std::vector<std::pair<int,int>>& date,int iEndDate,bool bAvg)
{
	bool bRet = true;
	int nDateSize = (int)date.size();
	int nIdSize = (int)iSecurityId.size();
	int nAvgType;
	int reqSize = sizeof(int) * (1/*�Ƿ�����ƽ���������*/ + 1/*��������*/ + 1/*ʱ��θ���*/ +nDateSize*2/*ʱ��μ���*/ + 1/*��������*/ + nIdSize/*��������*/);
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
		// ������Ӧ�ã�nRetSize����Ҫ����sizeof(int)
		if (nRetSize <= 4)
		{
			delete lpBuffer;
			lpBuffer = NULL;
			return false;
		}

		INT32 nPreSize = *(reinterpret_cast<INT32*>(lpRes));  //���ݴ�С
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
		resultTable.AddCol(Tx::Core::dtype_int4);//����
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


