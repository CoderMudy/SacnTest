/**************************************************************
	Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
	File name:		TxWarrant.cpp
	Author:			�Ժ꿡
	Date:			2007-10-29
	Version:		1.0
	Description:	
					Ȩ֤ҵ������

***************************************************************/

#include "StdAfx.h"
#include "TxWarrant.h"
#include "MyIndicator.h"
#include "ComputingWarrant.h"

namespace Tx
{
	namespace Business
	{

TxWarrant::TxWarrant(void)
{
	//����
	m_dWarRatio = 1.25;
}

TxWarrant::~TxWarrant(void)
{
	//����
}
//��Ȩ��ʽ 30301139 tinyint short
short TxWarrant::GetPowerStyle(SecurityQuotation* pSecurity)
{
	if(pSecurity==NULL)
		return -1;
	WarrantBasicInfo* pWarrantBasicInfo = pSecurity->GetWarrantBasicInfo();
	if(pWarrantBasicInfo==NULL)
		return -1;
	//byte bytePowerStyle = pSecurity->GetIndicateByteValue(30301139);
	//return (short)bytePowerStyle;//pSecurity->GetIndicateShortValue(30301139);
	return (short)pWarrantBasicInfo->power_style;
}
//��ʽȨ֤
bool TxWarrant::IsAmericanStyle(int iSecurityId)
{
	GetSecurityNow(iSecurityId);
	return IsAmericanStyle(m_pSecurity);
}
bool TxWarrant::IsAmericanStyle(SecurityQuotation* pSecurity)
{
	if(GetPowerStyle(pSecurity)==0)
		return true;
	return false;
}
//ŷʽȨ֤
bool TxWarrant::IsEuropeanStyle(int iSecurityId)
{
	GetSecurityNow(iSecurityId);
	return IsEuropeanStyle(m_pSecurity);
}

bool TxWarrant::IsEuropeanStyle(SecurityQuotation* pSecurity)
{
	if(GetPowerStyle(pSecurity)==1)
		return true;
	return false;
}
//��Ľ��
bool TxWarrant::IsBermudaStyle(int iSecurityId)
{
	GetSecurityNow(iSecurityId);
	return IsBermudaStyle(m_pSecurity);
}
bool TxWarrant::IsBermudaStyle(SecurityQuotation* pSecurity)
{
	if(GetPowerStyle(pSecurity)==2)
		return true;
	return false;
}
//���֤ȯ���� 30301129 int
int TxWarrant::GetObjectSecurityId(int iSecurityId)
{
	GetSecurityNow(iSecurityId);
	return GetObjectSecurityId(m_pSecurity);
}
int TxWarrant::GetObjectSecurityId(SecurityQuotation* pSecurity)
{
	if(pSecurity==NULL)
		return Con_intInvalid;
	WarrantBasicInfo* pWarrantBasicInfo = pSecurity->GetWarrantBasicInfo();
	if(pWarrantBasicInfo==NULL)
		return Con_intInvalid;
	//return pSecurity->GetIndicateIntValue(30301257);
	return pWarrantBasicInfo->obj_id;
}

//��Ȩ���� 30301140 decimal double
double TxWarrant::GetPowerRatio(int iSecurityId)
{
	GetSecurityNow(iSecurityId);
	return GetPowerRatio(m_pSecurity);
}
double TxWarrant::GetPowerRatio(SecurityQuotation* pSecurity)
{
	if(pSecurity==NULL)
		return Con_doubleInvalid;
	WarrantBasicInfo* pWarrantBasicInfo = pSecurity->GetWarrantBasicInfo();
	if(pWarrantBasicInfo==NULL)
		return Con_doubleInvalid;
	//return pSecurity->GetIndicateDoubleValue(30301140);
	return pWarrantBasicInfo->power_ratio;
}

//��Ȩ�۸� 30301141  decimal double
double TxWarrant::GetPowerPrice(int iSecurityId)
{
	GetSecurityNow(iSecurityId);
	return GetPowerPrice(m_pSecurity);
}
double TxWarrant::GetPowerPrice(SecurityQuotation* pSecurity)
{
	if(pSecurity==NULL)
		return Con_doubleInvalid;
	WarrantBasicInfo* pWarrantBasicInfo = pSecurity->GetWarrantBasicInfo();
	if(pWarrantBasicInfo==NULL)
		return Con_doubleInvalid;
	//return pSecurity->GetIndicateDoubleValue(30301141);
	return pWarrantBasicInfo->power_price;
}
//������ֹ���� 30301135
//��Ȩ��ֹ����[Ȩ֤�ĵ�����] 30301143 int
int TxWarrant::GetPowerEndDate(int iSecurityId)
{
	GetSecurityNow(iSecurityId);
	return GetPowerEndDate(m_pSecurity);
}
int TxWarrant::GetPowerEndDate(SecurityQuotation* pSecurity)
{
	if(pSecurity==NULL)
		return Con_intInvalid;
	WarrantBasicInfo* pWarrantBasicInfo = pSecurity->GetWarrantBasicInfo();
	if(pWarrantBasicInfo==NULL)
		return Con_intInvalid;
	//return pSecurity->GetIndicateIntValue(30301143);
	return pWarrantBasicInfo->power_end_date;
}
//ȡ����ͣ�۸�
float	TxWarrant::GetUpper(int iSecurityId)
{
	SecurityQuotation* pSecurity = GetSecurityNow(iSecurityId);
	if(pSecurity==NULL)
		return Con_floatInvalid;
	return GetUpper(pSecurity);
}
float	TxWarrant::GetUpper(SecurityQuotation* pSecurity)
{
	//ȡ�ñ��֤ȯ
	int TartgetId = GetObjectSecurityId(pSecurity->GetId());
	//��Ȩ����
	double fPowerratio = GetPowerRatio(pSecurity->GetId());
	if(!(fPowerratio>0))
		return Con_floatInvalid;
	
	SecurityQuotation* pObjSecurity = GetSecurityNow(TartgetId);
	if(pObjSecurity==NULL)
		return Con_floatInvalid;
	//Ȩ֤ǰ�ռ�
	float fPreClose = pSecurity->GetPreClosePrice();
	if(fPreClose<0)
		return Con_floatInvalid;
	//���ǰ�ռ�
	float fTarPreClose = pObjSecurity->GetPreClosePrice();
	if(fTarPreClose<0)
		return Con_floatInvalid;
	//�����ͣ��
	//2008-03-03
	float ratio = 0.1f;
	if(pObjSecurity->InSTblock()==true)
		ratio = 0.05f;
	/*
	float fUpper = pObjSecurity->GetUpper(ratio);
	if(fUpper<0)
		return Con_floatInvalid;
	int iScale = 3;
	//���=3
	//�Ͻ���=2
	if(pSecurity->IsShanghai()==true)
		iScale = 2;
	int ifUpper = 0;
	int iSRup = 1;
	ifUpper = GetDataByScale(fUpper,iScale,iSRup);
	int ifPtclose= 0;
	int iSRptc = 1;
	ifPtclose = GetDataByScale(fTarPreClose,3,iSRptc);
	int ifPclose= 0;
	int iSRpc = 1;
	ifPclose = GetDataByScale(fPreClose,3,iSRpc);
	//Ȩ֤��ͣ��
	float fRaise = (float)(ifPclose/(double)iSRpc+(ifUpper/(double)iSRup-ifPtclose/(double)iSRptc)*m_dWarRatio*fPowerratio);
	//float fRaise = (float)(fPreClose+(fUpper-fTarPreClose)*m_dWarRatio*fPowerratio);
	//fRaise = GetDataByScale(fRaise,3);
	fRaise+=0.00005f;
	*/
	//�Ƿ�
	float fUpValue = fTarPreClose*ratio;
	int iScale = 3;
	//���=3λС����
	//�Ͻ���=2λС����
	if(pSecurity->IsShanghai()==true)
		iScale = 2;
	int iUp = 1;
	int ifUpValue = 0;
	ifUpValue = GetDataByScale(fUpValue,iScale,iUp);
	fUpValue = (float)(ifUpValue/(float)iUp);

	//Ȩ֤��ͣ��
	float fRaise = (float)(fPreClose + fUpValue*m_dWarRatio*fPowerratio);
	fRaise+=0.00005f;
	int iUpRaise = 1;
	int ifRaise = 0;
	ifRaise = GetDataByScale(fRaise,3,iUpRaise);
	fRaise = (float)(ifRaise/(float)iUpRaise);

	return fRaise;
}
//ȡ�õ�ͣ�۸�
float	TxWarrant::GetLower(int iSecurityId)
{
	SecurityQuotation* pSecurity = GetSecurityNow(iSecurityId);
	if(pSecurity==NULL)
		return Con_floatInvalid;
	return GetLower(pSecurity);
}
float	TxWarrant::GetLower(SecurityQuotation* pSecurity)
{
	//ȡ�ñ��֤ȯ
	int TartgetId = GetObjectSecurityId(pSecurity->GetId());
	//��Ȩ����
	double fPowerratio = GetPowerRatio(pSecurity->GetId());
	if(!(fPowerratio>0))
		return Con_floatInvalid;
	
	SecurityQuotation* pObjSecurity = GetSecurityNow(TartgetId);
	if(pObjSecurity==NULL)
		return Con_floatInvalid;
	//Ȩ֤ǰ�ռ�
	float fPreClose = pSecurity->GetPreClosePrice();
	if(fPreClose<0)
		return Con_floatInvalid;
	//���ǰ�ռ�
	float fTarPreClose = pObjSecurity->GetPreClosePrice();
	if(fTarPreClose<0)
		return Con_floatInvalid;

	//��ĵ�ͣ��
	//2008-03-03
	float ratio = 0.1f;
	if(pObjSecurity->InSTblock()==true)
		ratio = 0.05f;
/*
	float fLower = pObjSecurity->GetLower(ratio);
	if(fLower<0)
		return Con_floatInvalid;
	int iScale = 3;
	//���=3λС����
	//�Ͻ���=2λС����
	if(pSecurity->IsShanghai()==true)
		iScale = 2;
	int ifUpper = 0;
	int iSRup = 1;
	ifUpper = GetDataByScale(fLower,iScale,iSRup);

	int ifPtclose= 0;
	int iSRptc = 1;
	ifPtclose = GetDataByScale(fTarPreClose,3,iSRptc);
	int ifPclose= 0;
	int iSRpc = 1;
	ifPclose = GetDataByScale(fPreClose,3,iSRpc);

	//Ȩ֤��ͣ��
	float fRaise = (float)(ifPclose/(double)iSRpc-(ifPtclose/(double)iSRptc-ifUpper/(double)iSRup)*m_dWarRatio*fPowerratio);
	fRaise+=0.00005f;
*/
	//����
	float fLowValue = fTarPreClose*ratio;
	int iScale = 3;
	//���=3λС����
	//�Ͻ���=2λС����
	if(pSecurity->IsShanghai()==true)
		iScale = 2;
	int iLow = 1;
	int ifLowValue = 0;
	ifLowValue = GetDataByScale(fLowValue,iScale,iLow);
	fLowValue = (float)(ifLowValue/(float)iLow);

	//Ȩ֤��ͣ��
	float fRaise = (float)(fPreClose - fLowValue*m_dWarRatio*fPowerratio);
	fRaise+=0.00005f;
	int iLowRaise = 1;
	int ifRaise = 0;
	ifRaise = GetDataByScale(fRaise,3,iLowRaise);
	fRaise = (float)(ifRaise/(float)iLowRaise);
/*	double v = 0;
	v = GetDataByScale(fLower,iScale);

	fLower = v;//GetDataByScale(fLower,iScale);
	v = GetDataByScale(fTarPreClose,3);
	fTarPreClose =v;
	v = GetDataByScale(fPreClose,3);
	fPreClose = v;

	//Ȩ֤��ͣ��
	float fRaise = (float)(fPreClose-(fTarPreClose-fLower)*m_dWarRatio*fPowerratio);
	fRaise = GetDataByScale(fRaise,3);
	*/
	if(fRaise<0)
		fRaise = 0;
	return fRaise;
}
//ʣ������
int TxWarrant::GetRemainsDays(int iSecurityId,int iDate)
{
	SecurityQuotation* pSecurity = GetSecurityNow(iSecurityId);
	if(pSecurity==NULL)
		return Con_intInvalid;
	return GetRemainsDays(pSecurity,iDate);
}
int TxWarrant::GetRemainsDays(SecurityQuotation* pSecurity,int iDate)
{
	int iEndDate = 0;//GetPowerEndDate(pSecurity);
	//������ֹ���� 30301135
	//if(pSecurity->GetIndicatorValueAbsolute(30301135,iEndDate)==false)
	//	return Con_intInvalid;
	WarrantBasicInfo* pWarrantBasicInfo = pSecurity->GetWarrantBasicInfo();
	if(pWarrantBasicInfo==NULL)
		return Con_intInvalid;
	iEndDate = pWarrantBasicInfo->end_date;

	if(iEndDate==Con_intInvalid)
		return Con_intInvalid;
	ZkkLib::DateTime startDate(iDate);
	ZkkLib::DateTime endDate(iEndDate);
	return (int)(endDate-startDate).GetTotalDays();
}
//ȡ�������
//---------------------------------------------------------------
//�Ϲ�Ȩ֤�����=����Ȩ��+�Ϲ�Ȩ֤�۸�/��Ȩ����-���ɼۣ�/���ɼ� 
//�Ϲ�Ȩ֤�����=�����ɼ�+�Ϲ�Ȩ֤�۸�/��Ȩ����-��Ȩ�ۣ�/���ɼ�
//Ͷ������Ҫ�仯���ٰٷֱȣ��ſ���ʵ��ƽ��
//*******************************************************************
double	TxWarrant::GetPremiumRate(
		double dWPrice,		//Ȩ֤�۸�
		double dSPrice,		//��Ĺ�Ʊ�۸�
		double dExPrice,	//��Ȩ��
		bool	bFlag,		//�Ϲ�Ϊtrue,��֮Ϊfalse
		double	dRatio		//��Ȩ����
		)
{
	ComputingWarrant cw;
	return 100*cw.GetPremiumRate(
			dWPrice,		//Ȩ֤�۸�
			dSPrice,		//��Ĺ�Ʊ�۸�
			dExPrice,	//��Ȩ��
			bFlag,		//�Ϲ�Ϊtrue,��֮Ϊfalse
			dRatio		//��Ȩ����
		);

	//double dPre= 0.0;
	//if( bFlag )//�Ϲ�
	//{
	//	dPre = (dExPrice + dWPrice / dRatio)/dSPrice - 1; 
	//	//�Ϲ�Ȩ֤�����=����Ȩ��+�Ϲ�Ȩ֤�۸�/��Ȩ����-���ɼۣ�/���ɼ� 
	//	//dPre = (dExPrice + dWPrice/dRatio -  dSPrice)/dSPrice;
	//}
	//else		//�Ϲ�
	//{
	//	//�Ϲ�Ȩ֤�����=�����ɼ�+�Ϲ�Ȩ֤�۸�/��Ȩ����-��Ȩ�ۣ�/���ɼ�
	//	//dPre = 1- (dExPrice - dWPrice/dRatio )/ dSPrice;
	//	dPre = 1 + (dWPrice/dRatio - dExPrice )/ dSPrice;
	//}

	//dPre *= 100;
	//return dPre;
}
//ȡ�������
double		TxWarrant::GetPremiumRate(int iSecurityId)
{
	SecurityQuotation* pSecurity = GetSecurityNow(iSecurityId);
	if(pSecurity==NULL)
		return Con_intInvalid;
	return GetPremiumRate(pSecurity);
}
double		TxWarrant::GetPremiumRate(SecurityQuotation* pSecurity)
{
	//��Ȩ��
	double dPowerPrice = GetPowerPrice(pSecurity);
	if(dPowerPrice<0)
		return Con_doubleInvalid;
	//�ּ�
	float fPrice = pSecurity->GetClosePrice();
	if(fPrice<0)
		return Con_doubleInvalid;
	//�Ϲ�
	bool bBuy = pSecurity->IsWarrant_Buy();
	//ȡ�ñ��֤ȯ
	int TartgetId = GetObjectSecurityId(pSecurity->GetId());
	//��Ȩ����
	double fPowerratio = GetPowerRatio(pSecurity->GetId());
	if(!(fPowerratio>0))
		return Con_doubleInvalid;
	
	SecurityQuotation* pObjSecurity = GetSecurityNow(TartgetId);
	if(pObjSecurity==NULL)
		return Con_doubleInvalid;
	float fObjPrice = pObjSecurity->GetClosePrice();
	if(fObjPrice<0)
		return Con_doubleInvalid;
	return GetPremiumRate(fPrice,fObjPrice,dPowerPrice,bBuy,fPowerratio);
}
double		TxWarrant::GetPremiumRate(SecurityQuotation* pSecurity,int date)
{
	//��Ȩ��
	double dPowerPrice = GetPowerPrice(pSecurity);
	if(dPowerPrice<0)
		return Con_doubleInvalid;
	//�ּ�
	float fPrice = pSecurity->GetClosePrice(date,true);
	if(fPrice<0)
		return Con_doubleInvalid;
	//�Ϲ�
	bool bBuy = pSecurity->IsWarrant_Buy();
	//ȡ�ñ��֤ȯ
	int TartgetId = GetObjectSecurityId(pSecurity->GetId());
	//��Ȩ����
	double fPowerratio = GetPowerRatio(pSecurity->GetId());
	if(!(fPowerratio>0))
		return Con_doubleInvalid;
	
	SecurityQuotation* pObjSecurity = GetSecurityNow(TartgetId);
	if(pObjSecurity==NULL)
		return Con_doubleInvalid;
	float fObjPrice = pObjSecurity->GetClosePrice(date,true);
	if(fObjPrice<0)
		return Con_doubleInvalid;
	return GetPremiumRate(fPrice,fObjPrice,dPowerPrice,bBuy,fPowerratio);
}
//�ܸ˱���
double	TxWarrant::GetGearRate(	double	dSClose,	//����ʲ������̼�
										double	dWClose,	//Ȩ֤�����̼�
										double	dExRatio	//��Ȩ����
										)
{
	ComputingWarrant cw;
	return cw.GetGearRate(dSClose,dWClose,dExRatio);
	//double	dGear = 0.0;
	//dGear = dSClose/( dWClose / dExRatio );
	//return dGear;
}
double		TxWarrant::GetGearRate(int iSecurityId)
{
	SecurityQuotation* pSecurity = GetSecurityNow(iSecurityId);
	if(pSecurity==NULL)
		return Con_intInvalid;
	return GetGearRate(pSecurity);
}
double		TxWarrant::GetGearRate(SecurityQuotation* pSecurity)
{
	//�ּ�
	float fPrice = pSecurity->GetClosePrice();
	if(fPrice<0)
		return Con_doubleInvalid;
	//ȡ�ñ��֤ȯ
	int TartgetId = GetObjectSecurityId(pSecurity->GetId());
	//��Ȩ����
	double fPowerratio = GetPowerRatio(pSecurity->GetId());
	if(!(fPowerratio>0))
		return Con_doubleInvalid;
	
	SecurityQuotation* pObjSecurity = GetSecurityNow(TartgetId);
	if(pObjSecurity==NULL)
		return Con_doubleInvalid;
	float fObjPrice = pObjSecurity->GetClosePrice();
	if(fObjPrice<0)
		return Con_doubleInvalid;
	return GetGearRate(fObjPrice,fPrice,fPowerratio);
}
double		TxWarrant::GetGearRate(SecurityQuotation* pSecurity,int date)
{
	//�ּ�
	float fPrice = pSecurity->GetClosePrice(date,true);
	if(fPrice<0)
		return Con_doubleInvalid;
	//ȡ�ñ��֤ȯ
	int TartgetId = GetObjectSecurityId(pSecurity->GetId());
	//��Ȩ����
	double fPowerratio = GetPowerRatio(pSecurity->GetId());
	if(!(fPowerratio>0))
		return Con_doubleInvalid;

	SecurityQuotation* pObjSecurity = GetSecurityNow(TartgetId);
	if(pObjSecurity==NULL)
		return Con_doubleInvalid;
	float fObjPrice = pObjSecurity->GetClosePrice(date,true);
	if(fObjPrice<0)
		return Con_doubleInvalid;
	return GetGearRate(fObjPrice,fPrice,fPowerratio);
}
//ȡ������������
double	TxWarrant::GetSigma(	bool	bCall_Put,		//�Ϲ��Ϲ�	true�Ϲ�
									double	dStockPrice,//���֤ȯ�۸�
									double	dExPrice,	//��Ȩ�۸�
									double	dNoRiskInterest,//�޷�������
									double	dDelta,		
									double	dLastedYear,//ʣ�����ޣ��꣩
									double	dMarketValue//��Ȩ�г���ֵ
									)
{
	//-------------ȡ����Щ����---------------------------
	double	dSigma = 0.0;
	////����Ȩ֤ID�õ���������
	//bool	bCall_Put = false;	
	////����ID���ڵõ���Ʊ�۸�
	//double	dStockPrice = 1;
	////��Ȩִ�м۸�
	//double	dExPrice = 1.0;
	////�޷���������
	//double	dNoRiskInterest = 0.02;
	////��Ĺ�Ʊ�����������
	//double	dDelta = 0.0;
	////ʣ�����ޣ��꣩
	//double	dLastedYear = 1.0;
	////��Ȩ�г���ֵ
	//double	dMarketValue = 1.0;
	//***************************************************************
	CWarrantMath	WarrantMath;
	WarrantMath.implied_volatility(bCall_Put,dStockPrice,dExPrice,dNoRiskInterest,dDelta,dLastedYear,dMarketValue,&dSigma );
	
	return dSigma;
}
double		TxWarrant::GetSigma(int iSecurityId)
{
	SecurityQuotation* pSecurity = GetSecurityNow(iSecurityId);
	if(pSecurity==NULL)
		return Con_intInvalid;
	return GetSigma(pSecurity);
}
double		TxWarrant::GetSigma(SecurityQuotation* pSecurity)
{
	int iRenmainsgays = GetRemainsDays(pSecurity,ZkkLib::DateTime::getCurrentTime().GetDate().GetInt());
	double dLastYear = (double)iRenmainsgays/365;
	//��Ȩ��
	double dPowerPrice = GetPowerPrice(pSecurity);
	if(dPowerPrice<0)
		return Con_doubleInvalid;	
	
	//�ּ�
	float fPrice = pSecurity->GetClosePrice();
	if(fPrice<0)
		return Con_doubleInvalid;
	//�Ϲ�
	bool bBuy = pSecurity->IsWarrant_Buy();
	//ȡ�ñ��֤ȯ
	int TartgetId = GetObjectSecurityId(pSecurity->GetId());
	//��Ȩ����
	double fPowerratio = GetPowerRatio(pSecurity->GetId());
	if(!(fPowerratio>0))
		return Con_doubleInvalid;
	
	SecurityQuotation* pObjSecurity = GetSecurityNow(TartgetId);
	if(pObjSecurity==NULL)
		return Con_doubleInvalid;
	float fObjPrice = pObjSecurity->GetClosePrice();
	if(fObjPrice<0)
		return Con_doubleInvalid;
	double		dNoInterest = 0.018;
	
	double	dMarketValue=fPrice/fPowerratio;
	//��Ĺ�Ʊ�����������
	double	dDelta = 0.0;
	return GetSigma(!bBuy,fObjPrice,dPowerPrice,dNoInterest,dDelta,dLastYear,dMarketValue);
}
double		TxWarrant::GetSigma(SecurityQuotation* pSecurity,int date)
{
	int iRenmainsgays = GetRemainsDays(pSecurity,date);
	double dLastYear = (double)iRenmainsgays/365;
	//��Ȩ��
	double dPowerPrice = GetPowerPrice(pSecurity);
	if(dPowerPrice<0)
		return Con_doubleInvalid;
	
	//�ּ�
	float fPrice = pSecurity->GetClosePrice(date,true);
	if(fPrice<0)
		return Con_doubleInvalid;
	//�Ϲ�
	bool bBuy = pSecurity->IsWarrant_Buy();
	//ȡ�ñ��֤ȯ
	int TartgetId = GetObjectSecurityId(pSecurity->GetId());
	//��Ȩ����
	double fPowerratio = GetPowerRatio(pSecurity->GetId());
	if(!(fPowerratio>0))
		return Con_doubleInvalid;
	
	SecurityQuotation* pObjSecurity = GetSecurityNow(TartgetId);
	if(pObjSecurity==NULL)
		return Con_doubleInvalid;
	float fObjPrice = pObjSecurity->GetClosePrice(date,true);
	if(fObjPrice<0)
		return Con_doubleInvalid;
	double		dNoInterest = 0.018;
	
	double	dMarketValue=fPrice/fPowerratio;
	//��Ĺ�Ʊ�����������
	double	dDelta = 0.0;
	return GetSigma(!bBuy,fObjPrice,dPowerPrice,dNoInterest,dDelta,dLastYear,dMarketValue);
}
//����������ݽ��������
bool TxWarrant::GetBlockAnalysisCol(Table_Display& baTable,std::vector<int>& arrSamples,int& iSortCol)
{
	if(TxBusiness::GetBlockAnalysisCol(baTable,arrSamples,iSortCol)==false)
		return false;

	baTable.SetPrecise(5, 3);
	baTable.SetPrecise(6, 3);
	baTable.SetPrecise(7, 3);

	int nCol = baTable.GetColCount();
	int sCol = 12;
	if(nCol>sCol)
		baTable.DeleteCol(sCol,nCol-sCol);

	//11 ��������
	baTable.AddCol(Tx::Core::dtype_int4);
	//12 ʣ������
	baTable.AddCol(Tx::Core::dtype_val_string);
	//��������
	baTable.AddCol(Tx::Core::dtype_val_string);
	//���ɴ���
	baTable.AddCol(Tx::Core::dtype_val_string);
	//���ɼ۸�
	baTable.AddCol(Tx::Core::dtype_double);
	//13 ����
	baTable.AddCol(Tx::Core::dtype_double);
	//14 ���
	baTable.AddCol(Tx::Core::dtype_double);
	//15 �ܸ�
	baTable.AddCol(Tx::Core::dtype_double);

	nCol = 12;
	baTable.SetTitle(nCol, _T("��������"));
	baTable.SetFormatStyle(nCol, Tx::Core::fs_date);
	++nCol;
	baTable.SetTitle(nCol, _T("ʣ������"));
	baTable.SetPrecise(nCol, 2);
	++nCol;

	//��������
	baTable.SetTitle(nCol, _T("��������"));
	++nCol;
	//���ɴ���
	baTable.SetTitle(nCol, _T("���ɴ���"));
	++nCol;
	//���ɼ۸�
	baTable.SetTitle(nCol, _T("���ɼ۸�"));
	++nCol;

	baTable.SetTitle(nCol, _T("����"));
	baTable.SetPrecise(nCol, 3);
	++nCol;
	baTable.SetTitle(nCol, _T("���"));
	baTable.SetPrecise(nCol, 2);
	++nCol;
	baTable.SetTitle(nCol, _T("�ܸ�"));
	baTable.SetPrecise(nCol, 2);
#ifdef _DEBUG
	CString m_str=baTable.TableToString();
	Tx::Core::Commonality::String().StringToClipboard(m_str);
#endif
	return true;
}
//����������ݽ������
bool TxWarrant::SetBlockAnalysisCol(Table_Display& baTable,SecurityQuotation* psq,int& j,int ii)
{
	if(psq==NULL)
		return false;
	//11 ��������
		int iEndDate=Con_intInvalid;
		//������ֹ���� 30301135
		WarrantBasicInfo* pWarrantBasicInfo = psq->GetWarrantBasicInfo();
		if(pWarrantBasicInfo!=NULL)
		{
			iEndDate = pWarrantBasicInfo->end_date;
		}
		//if(psq->GetIndicatorValueAbsolute(30301135,iEndDate)==false)
		//	iEndDate=Con_intInvalid;
		baTable.SetCell(j,ii,iEndDate);
		j++;
	//12 ʣ������
		CString strDate;
		int iDate = psq->GetCurDataDateTime().GetDate().GetInt();
		int days = GetRemainsDays(psq,iDate);
		if((iEndDate>0 && iDate>iEndDate) || days<=0 )
			strDate = _T("�ѵ���");
		else
			strDate.Format(_T("%d��"),days);
		baTable.SetCell(j,ii,strDate);
		j++;

		//
		int iObjId = GetObjectSecurityId(psq);
		SecurityQuotation* pSecurityObj = GetSecurityNow(iObjId);

		if(pSecurityObj!=NULL)
			baTable.SetCell(j,ii,pSecurityObj->GetName());
		j++;
		if(pSecurityObj!=NULL)
			baTable.SetCell(j,ii,pSecurityObj->GetCode());
		j++;
		double m_dPrice = (double)pSecurityObj->GetClosePrice(true);
		if(m_dPrice < 0.01)
			m_dPrice = Con_doubleInvalid;
		baTable.SetCell(j,ii,m_dPrice);
		j++;

	//13 ����
		baTable.SetCell(j,ii,GetSigma(psq));
		j++;
	//14 ���
		baTable.SetCell(j,ii,GetPremiumRate(psq));
		j++;
	//15 �ܸ�
		baTable.SetCell(j,ii,GetGearRate(psq));
		j++;
#ifdef _DEBUG
		CString m_str=baTable.TableToString();
		Tx::Core::Commonality::String().StringToClipboard(m_str);
#endif
	return true;
}

	}
}