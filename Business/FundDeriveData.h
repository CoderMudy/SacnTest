/********************************************************************
	FileName:	FundDeriveData.h
	Created:	2007/09/25
	Project:	Business
	Author:		xum
	Version:	v1.0

	Purpose:	use to calculate the derived data of fund 
	
	History:	

*********************************************************************
	Copyright (C) 2007 - All Rights Reserved
*********************************************************************/

#pragma once

#include "TxBusiness.h"

namespace Tx
{
	namespace Business
	{

class BUSINESS_EXT FundDeriveData : 
			public TxBusiness
{
public:
	FundDeriveData(void);
	~FundDeriveData(void);

public:
	//����һ�����ÿ�����ʱ��ľ�ֵ�����ʵļ�Ȩֵ
	bool CalcWeightedFundNvr (std::vector<double> & dWeighted, Tx::Core::Table& resultTable, std::vector<int> & iSecurityId, std::set< std::pair<int,int> >& date);

	//����һ�����ĳ�����ڣ�ÿ�����ʱ��ĵ�λ��ֵ������
	bool CalcFundNvr (Tx::Core::Table& resultTable, std::vector<int> & iSecurityId, std::set< std::pair<int,int> >& date);

	//����һֻ����һ��ʱ������ľ�ֵ������
	double CalcFundNvr_OneFund (int iSecurityId, int iBeginDate, int iEndDate, BOOL bFlag_Sryl = FALSE);

	//����һֻ����ĳ�׶������ڣ�ÿ�����ʱ��ľ�ֵ������
	bool CalcFundNvr_OneFund (std::vector<double>& dNvr, int iSecurityId, std::set<int> & dates, int iBeginDate);

	//�����ֻ������ĳ��ʱ�������ڣ���ֵ������
	bool CalcFundNvr_Funds (std::vector<double>& dNvr, std::vector<int> & iSecurityId, int iEndDate, int iBeginDate);

	//����һ����ʽ����ĳ��ʱ�����е��������
	bool CalcFundPremium (Tx::Core::Table& resultTable, std::vector<int> & iSecurityId, std::vector<int>& date);

	//����ĳһ��֤ȯ��ĳ��ʱ�����е�����������
	bool CalCloseFundNav(Tx::Core::Table& resultTable, std::vector<int> & iSecurityId, std::set< std::pair<int,int> >& date,bool bNav = true,bool bSim = true);

	//�����Ʊĳ��ʱ���ڵ���������
	bool CalStockRate( Tx::Core::Table& resultTable, std::vector<int> & iSecurityId, std::set< std::pair<int,int> >& date,bool bSim = true ); 
	//����ĳһ��֤ȯ��ĳ��ʱ�����е����������У������ļ�ת��Ϊ�˺�̨����
	bool CalFundNav_Ext(Tx::Core::Table& resultTable, std::vector<int> & iSecurityId, std::set< std::pair<int,int> >& date,bool bNav = true,bool bSim = true);

	//����ĳһ�����ĳ�����ڵĵ�λ��ֵ���ۼƾ�ֵ�����̼ۡ�������ʣ�ĳ��ʱ�������ڣ�ҵ����׼�����ʡ�ƽ���������
	bool CalFundData(Tx::Core::Table& resultTable, std::vector<int>& iSecurityId, std::vector<std::pair<int,int>>& date,int iEndDate,bool bAvg = true);


};

	}
}
