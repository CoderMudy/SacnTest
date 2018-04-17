/**************************************************************
	Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
	File name:		TxWarrant.h
	Author:			�Ժ꿡
	Date:			2007-10-29
	Version:		1.0
	Description:	
					Ȩ֤ҵ������

***************************************************************/

#ifndef __TXWARRANT_H__
#define __TXWARRANT_H__

#include "..\..\Core\TxMath\CWarrantMath.h"
#include "TxBusiness.h"

#pragma once
namespace Tx
{
	namespace Business
	{

class BUSINESS_EXT TxWarrant :
	public TxBusiness
{
public:
	TxWarrant(void);
	virtual ~TxWarrant(void);

private:
	//�Ƿ����Ʊ���125%
	double m_dWarRatio;
private:
	//��Ȩ��ʽ 30301139 tinyint short
	short GetPowerStyle(SecurityQuotation* pSecurity);
public:
	//��ʽȨ֤
	bool IsAmericanStyle(int iSecurityId);
	bool IsAmericanStyle(SecurityQuotation* pSecurity);
	//ŷʽȨ֤
	bool IsEuropeanStyle(int iSecurityId);
	bool IsEuropeanStyle(SecurityQuotation* pSecurity);
	//��Ľ��
	bool IsBermudaStyle(int iSecurityId);
	bool IsBermudaStyle(SecurityQuotation* pSecurity);

	//���֤ȯ���� 30301129 int
	//���֤ȯ���� 30301257 int
	int GetObjectSecurityId(int iSecurityId);
	int GetObjectSecurityId(SecurityQuotation* pSecurity);

	//��Ȩ���� 30301140 decimal double
	double GetPowerRatio(int iSecurityId);
	double GetPowerRatio(SecurityQuotation* pSecurity);

	//��Ȩ�۸� 30301141  decimal double
	double GetPowerPrice(int iSecurityId);
	double GetPowerPrice(SecurityQuotation* pSecurity);

	//��Ȩ��ֹ����[Ȩ֤�ĵ�����] 30301143 int
	int GetPowerEndDate(int iSecurityId);
	int GetPowerEndDate(SecurityQuotation* pSecurity);

	//ʣ������
	int GetRemainsDays(int iSecurityId,int iDate);
	int GetRemainsDays(SecurityQuotation* pSecurity,int iDate);

	//ȡ����ͣ�۸�
	float	GetUpper(int iSecurityId);
	float	GetUpper(SecurityQuotation* pSecurity);
	//ȡ�õ�ͣ�۸�
	float	GetLower(int iSecurityId);
	float	GetLower(SecurityQuotation* pSecurity);
	//ȡ�������
	double		GetPremiumRate( double dWPrice,		//Ȩ֤�۸�
								double dSPrice,		//��Ĺ�Ʊ�۸�
								double dExPrice,	//��Ȩ��
								bool	bFlag,		//�Ϲ�Ϊtrue,��֮Ϊfalse
								double	dRatio		//��Ȩ����
								);
	//ȡ�������
	double		GetPremiumRate(int iSecurityId);
	double		GetPremiumRate(SecurityQuotation* pSecurity);
	double		GetPremiumRate(SecurityQuotation* pSecurity,int date);
	//ȡ�øܸ˱���
	double		GetGearRate(	double	dSClose,	//����ʲ������̼�
								double	dWClose,	//Ȩ֤�����̼�
								double	dExRatio	//��Ȩ����
								);
	double		GetGearRate(int iSecurityId);
	double		GetGearRate(SecurityQuotation* pSecurity);
	double		GetGearRate(SecurityQuotation* pSecurity,int date);
	//ȡ������������
	double		GetSigma(	bool	bCall_Put,		//�Ϲ��Ϲ�	true�Ϲ�
							double	dStockPrice,	//���֤ȯ�۸�
							double	dExPrice,		//��Ȩ�۸�
							double	dNoRiskInterest,//�޷�������
							double	dDelta,		
							double	dLastedYear,	//ʣ�����ޣ��꣩
							double	dMarketValue	//��Ȩ�г���ֵ
							);
	double		GetSigma(int iSecurityId);
	double		GetSigma(SecurityQuotation* pSecurity);
	double		GetSigma(SecurityQuotation* pSecurity,int date);

	//���=�������
	//����������ݽ��������
	bool GetBlockAnalysisCol(Table_Display& baTable,std::vector<int>& arrSamples,int& iSortCol);
	//����������ݽ������
	bool SetBlockAnalysisCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow);
};
	}
}
#endif