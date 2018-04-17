/**************************************************************
	Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
	File name:		TxBond.h
	Author:			�Ժ꿡
	Date:			2007-10-29
	Version:		1.0
	Description:	
					ծȯҵ������

***************************************************************/

#ifndef __TXBOND_H__
#define __TXBOND_H__

#include "TxBusiness.h"
#include "..\..\Core\TxMath\YTM_curve.h"

#define MIN_JYS_QBOND_LIST_DATE	19961231 //�������г�������������

#pragma once
namespace Tx
{
	namespace Business
	{
//����תծ�׼���Ҫ�����ݽṹ
struct BondCompnySamples 
{
	double left_year;
	double ytm;
};
//2008-05-14 ծȯ����
//��ծ����
//��ծ����
//��ҵծ����
//��ϲ���
struct BOND_TEST_ITEM {
	char	NAME[31];//����
	double  MZH;	//��ֵ
	double  PMLL;	//Ʊ������
	int		FXNX;	//��������
	int     TYPE;	//����: 1��Ϣ/0��Ϣ
	int     GBOND;	//1��ծ/0��ծ
	int     FXPL;	//��ϢƵ��/0ÿ��/1ÿ����/2ÿ����/3����
	int     FXRQ;	//��������
	int     DATE;	//����
};
//������ծ����Ļ�׼�������ݽṹ
struct NEW_BOND_INTEREST
{
	int iDate;
	double dInterest;
public:
	int GetMapObj(int index=0) { return iDate; }
};
class BUSINESS_EXT TxBond :
	public TxBusiness
{
public:
	TxBond(void);
	virtual ~TxBond(void);
private:
	//	ծȯ��������
	double		m_dYTM;				// ����������
	double		m_dDURATION;		// �־���
	double		m_dCONVEXITY;		// ͹��
	double		m_dMDUR;			// �����־���
	float		m_fPRICE;			// ȫ�ۻ򾻼�[CalcPriceByYtm�ļ�����]

	//��Ϣ����
	double	m_dSpanDays;			//
	int		m_iTotalDaysOfYear;		//
	int		m_iNdays;				//���������ȥ������

	//2008-01-07
	//�Ƿ��Ż�YTM����ı�־
	//���ÿ�μ��㶼��ͬһ���YTM����������Ϊtrue,���������ڱ仯��Ҫ�������������ԣ�����Ϊfalse
	bool	m_bCalcYTMoneDay;
	//�ϴμ���M������
	int		m_iYtmDate;

	//����
	long		F_DATE;				//01 ����
	float		F_PRICE;			//02 ȫ�ۻ򾻼�
	bool		F_IS_NETPRICE;		//03 ���۱�־
	bool		F_HAVE_INTEREST;	//04 Ӧ����Ϣ�Ƿ�������
	float		F_INTEREST;			//05 Ӧ����Ϣ

	//2009-02-23
	//ծȯ����=�����Ӧ
	int	m_iBondType;//0=��ծ��1=��תծ��2=�ع�

private:
	//���������ڼ������
	//��ָ�����ڵ���Ϣ����֮�������
	float CalcPower(
		int iStart,	//��������
		int iEnd,	//�´θ�Ϣ��
		int iLip,	//���θ�Ϣ��
		float fFxpl,//��ϢƵ��
		int iFigureMethod
		);
	//����ָ�����ڻ���ʣ������[��+����]
	bool	GetRemnantsBasic(int iStart,int iEnd,int& iYear,int& iDays,int& iTotDays);
	//���㵥λΪ���ʣ������
	double	GetRemnantsYear(int iStart,int iEnd,bool bNewBond=false);

	//���㵥λΪ���ʣ������
	int GetRemnants2Days(int iStart,int iEnd);

	//ȡ�������ڼ������ [����2��29�ղ���]
	int Get_Span_Days_For_Bond(int start_date, int end_date);

private:
	//����YTM����۸�
	bool CalcPriceByYtm(int iSecurityId,float fYtm,int iDate,int iBondCount=1);
	bool CalcNewBondPriceByYtm(
		BondNewInfo* pBondNewInfo,//in,ģ��ծȯ�Ļ�����Ϣ
		DataFileNormal<blk_TxExFile_FileHead,BondCashFlowData>* pCashFlow,//in,ģ��ծȯ���ֽ���
		float fYtm,
		int iDate,
		int iBondCount=1
		);

	//ȡ��ָ��ʣ�����޵���ҵծ��ʣ�����޺�YTM�б�
	bool GetCompnyBondByLeftYear(CArray<BondCompnySamples>& sampleList, double left_years, int iDate);

public:
	//2009-02-23
	//����ծȯ����
	void SetBondType(int value) { m_iBondType = value; }

	//2008-05-19
	//�·�ƫ��
	int OffsetMonth(int date,int iMonth);

	//����ծȯ�ֽ���[ģ��]
	int GetCashFlow(BondNewInfo* pBondNewInfo,int date_lipd,double dInterest, float* pcash_flow);
	//�����ֽ���
	bool CreateCashFlow(DataFileNormal<blk_TxExFile_FileHead,BondCashFlowData>* pCashFlow,BondNewInfo* pBondNewInfo,DataFileNormal<blk_TxExFile_FileHead,NEW_BOND_INTEREST>* pNewBondInterest,int start_date,bool bIsNew);

	//���ȼ���
	bool CalcYTM(double& dYtm,double& dDuration,double& dMduration,double& dConvexity,int iSecurityId,int iDate,float fPrice=-1.0,int iBondCount=1);
	bool CalcNewBondYTM(
		double& dYtm,			//out
		double& dDuration,		//out
		double& dMduration,		//out
		double& dConvexity,		//out
		BondNewInfo* pBondNewInfo,//in,ģ��ծȯ�Ļ�����Ϣ
		DataFileNormal<blk_TxExFile_FileHead,BondCashFlowData>* pCashFlow,//in,ģ��ծȯ���ֽ���
		int iDate,	//in,����YTM������
		float fPrice,//in,����YTM��ծȯ�۸�[ȫ��]
		int iBondCount=1//in
		);

	//Ȼ���������3���ӿ�ȡ�ü�����
	bool	Calc(int iSecurityId,int iDate,float fPrice,int iBondCount=1);
	bool	Calc1(int iSecurityId,int iDate);
	//value = true��ʾֻ����һ���ֽ�������������
	//value = false��ʾÿ�μ��㶼Ҫ�����ֽ�������������
	void	SetYtmFlag(bool value)
	{
		m_bCalcYTMoneDay = value;
	}
	double	Get_YTM(int iSecurityId,int iDate,double dPrice=-1.0,int iBondCount=1);
	bool	Get_MDURATION_CONVEXITY(int iSecurityId,int iDate,double& dDuration,double& dMduration,double& dConvexity,int iBondCount=1);

	double	Get_YTM(void){return m_dYTM;}
	double	Get_DURATION(void)	{ return m_dDURATION; }
	double	Get_MDURATION(void)	{ return m_dMDUR; }
	double	Get_CONVEXITY(void) { return m_dCONVEXITY; }
	float	Get_PriceByYTM(int iSecurityId,float fYtm,int iDate,int iBondCount=1);
	float	Get_PriceByYTM(BondNewInfo* pBondNewInfo,DataFileNormal<blk_TxExFile_FileHead,BondCashFlowData>* pCashFlow,float fYtm,int iDate,int iBondCount=1);

	//ȡ��ծȯʣ������[����] NL/365
	int GetRemainsDays(int iStart,int iEnd);

	//����תծ�׼�
	double CalcFloor(int iSecurityId,int iDate,bool bIsNetPrice=false,double left_years=7);
	//����תծƽ��
	double CalcParity(int iSecurityId,int iDate=0);
	//�����תծ-ת�������
	double CalcPremium(int iSecurityId,int iDate=0);
	//����ʹ�ʽ
	static double CalcPremium(
		double dClosePrice,	//���̼�
		double dPrice		//=ƽ�ۣ��������Ϊƽ������ʣ�=�׼ۣ��������Ϊ�׼�����ʣ�
		);
	//==========================================================================================//by zhangxs 20080806
	//��תծȨ֤ȡ�����ɼ۸�
	double GetBondStockPrice(int iSecurityId,int iDate=0);
	//ȡ�������Ƿ�
	double GetBondStockRaiseRatio(int iSecurityId,int iDate=0);
	//==========================================================================================

	//ȡ��תծ�׼�
	double GetFloor(int iSecurityId);

public:
	//��ϢƵ�� 30001125 int
	int	GetPayInterstFrequency(int iSecurityId);
	int	GetPayInterstFrequency(SecurityQuotation* pSecurity);
	//2008-01-04
	//��ϢƵ�� 30001125 int
	CString GetPayInterstFrequencyString(int iSecurityId);
	CString GetPayInterstFrequencyString(SecurityQuotation* pSecurity);

	//2008-05-15
	//ȡ��ծȯ��Ϣ��ʽint=��Ϣ��ʽ1:����,2:����,3:��������,4:�۽�����,5:����
	CString GetBondInterestType(int iSecurityId);
	CString GetBondInterestType(SecurityQuotation* pSecurity);

	//�����������㷽�� 30001126 int
	//1:30/360,2:ACT/365,3:ACT/ACT,4:ACT/360,5:NL/365,6:30/365
	int GetFigureMethod(int iSecurityId);
	int GetFigureMethod(SecurityQuotation* pSecurity);

	//��������	30001209
	//�������� 30001128 int
	int GetEndDate(int iSecurityId);
	int GetEndDate(SecurityQuotation* pSecurity);

	//���й�ģ	30001204
	double	GetShare(int iSecurityId);
	double	GetShare(SecurityQuotation* pSecurity);
	//Ʊ������	30001205
	double GetParRate(int iSecurityId);
	double GetParRate(SecurityQuotation* pSecurity);
	//30001125		��ϢƵ��
	//��������	30001206
	double	GetHoldYear(int iSecurityId);
	double	GetHoldYear(SecurityQuotation* pSecurity);
	//������Ϣ����	30001208
	int	GetBeginDate(int iSecurityId);
	int	GetBeginDate(SecurityQuotation* pSecurity);

	//��Ϣ����=��һ���ֽ�������Ϣ����
	int	GetFirstBeginDate(int iSecurityId);
	int	GetFirstBeginDate(SecurityQuotation* pSecurity);

	//ʣ������
	double	GetRemnants(int iSecurityId,int iDate);
	double	GetRemnants(SecurityQuotation* pSecurity,int iDate);
	int	GetRemnantsDays(int iSecurityId,int iDate);
	int	GetRemnantsDays(SecurityQuotation* pSecurity,int iDate);
	//������ת��Ϊ�ַ���
	CString GetRemnants(int days);
	//����ϵͳ����һ��
	CString GetRemnantsString(int iStart,int iEnd);

	//��Ϣ����=
	int	GetInterestDays(int iSecurityId,int iDate);
	int	GetInterestDays(SecurityQuotation* pSecurity,int iDate);
	//������ծģ�����
	int	GetInterestDays(DataFileNormal<blk_TxExFile_FileHead,BondCashFlowData>* pCashFlow,int iDate);

	//Ӧ����Ϣ
	double	GetInterest(int iSecurityId,int iDate);
	double	GetInterest(SecurityQuotation* pSecurity,int iDate);
	//Ӧ����Ϣ_��  2012-7-3
	//bFlag   true--"�ռ�"����  �� false--"����"����
	double GetInterest_New(int iSecurityId,int iDate,bool bFlag=true);
	double GetInterest_New(SecurityQuotation* pSecurity,int iDate,bool bFlag);
	//������ծģ�����
	double	GetInterest(BondNewInfo* pBondNewInfo,DataFileNormal<blk_TxExFile_FileHead,BondCashFlowData>* pCashFlow,int iDate);
	BondCashFlowData*	GetBondCashFlowDataByDate(DataFileNormal<blk_TxExFile_FileHead,BondCashFlowData>* pCashFlow,int date);

	//30001207		��Ϣ��ʽ
	int	GetInterestType(int iSecurityId);
	int	GetInterestType(SecurityQuotation* pSecurity);
	//30001210		�������ڵĽ����� 
	//30001211		��������

	//30001115		ծȯ���
	//30001116		ծȯȫ��
	//30001117		��������
	//30001118		���û���ID
	//30001119		������ID
	//30001120		����������
	//30001121		ծȯ����
	//30001122		ծȯ��ʽ
	//30001123		��Ϣ��ʽ
	//30001139      ծȯ��Ӧ��Ʊ����
	//30001140		��Ʊ����ʵ��
	int GetMarkManner(int iSecurityId);
	int	GetMarkManner(SecurityQuotation* pSecurity);
	//ծȯ����
	CString GetBondType(int iSecurityId);
	CString GetBondType(SecurityQuotation* pSecurity);

	//ծȯ��ʽ
	CString GetBondForm(int iSecurityId);
	CString GetBondForm(SecurityQuotation* pSecurity);

	//ծȯ��Ϣ��ʽ
	CString GetInterestTypeName(int iSecurityId);
	CString GetInterestTypeName(SecurityQuotation* pSecurity);
	//30001124		��׼����
	//30001126		�����������㷽��
	//30001127		��ʼ����
	//30001128		��������
	//30001129		����Ҹ�����
	//30001130		����Ҹ��۸�
	//30001131		����
	//30001132		ծȯ������
	//30001133		������ʽ
	//30001134		��������
	//30001187		ծȯ��ֵ
	//30001188		ծȯ����
	//30001189		��׼�������

	//���=�������
	////baTable�Ѿ����������[����ʵ��]
	bool GetBlockAnalysis(Table_Display& baTable,std::vector<int>& arrSamples,int& iSortCol,int iMarketId=0,bool bNeedProgressWnd=false,int nCollID=-1);
	//bool GetBlockAnalysis(Table_Display& baTable,std::vector<int>& arrSamples,bool bNeedProgressWnd=false);
	//����������ݽ��������
	bool GetBlockAnalysisCol(Table_Display& baTable,std::vector<int>& arrSamples,int& iSortCol);
	//����������ݽ������
	bool SetBlockAnalysisCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow);
	//��������������ݽ������[������]
	bool SetBlockAnalysisHslCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow);

	//2008-01-28
	//������ʾ
	//flag==0 ��Ϣ����
	//flag==1 ��Ϣ����
	//flag==2 ��Ϣ����
	//flag==3 �Ǽ�����
	bool GetTradeInfo(std::vector<int> iSecurityId,Table_Display& baTable,int iStartDate,int iEndDate,bool bAllDate=false,int flag=3);
	//added by zhangxs 20081216
	//������ʾ --����Ϣͳ����
	//flag==0 ��Ϣ����
	//flag==1 ��Ϣ����
	//flag==2 ��Ϣ����
	//flag==3 �Ǽ�����
	bool TxBond::GetTradeInfoDivideBonus(std::vector<int> iSecurityId,
		Table_Display& baTable,
		int iStartDate,
		int iEndDate,
		bool bAllDate,
		int flag);

public:
//	bool TxBondPayInterest(Tx::Core::Table_Indicator& resTable,std::vector<int> iSecurityId,int iStartDate,int iEndDate,bool bAllDate);
//	bool TxBondSectionMarket(Tx::Core::Table_Indicator& AccountTable,std::vector<int> iSecurityId,int iStartDate,int iEndDate);
	//	The iMarketdate is deleted by lijw 2008-09-03 ������ͳ����û���õ��ò��������԰��������ȥ����
	bool TxBondIssue(Tx::Core::Table_Indicator& resTable,std::vector<int> iSecurityId,int iStartDate,int iEndDate,bool bAllDate);
	bool IdColToNameAndCode(Tx::Core::Table_Indicator &resTable,int iCol,int iIdType=0,int iMethodType=0);
	//ծȯ�Ľ׶�����[�Ƿ�][�߼�]
	bool TxBondCycleRateAdv(
		std::set<int>& iSecurityId,			//����ʵ��ID
		int startDate,						//��ʼ����
		int endDate,						//��ֹ����
		bool bCutFirstDateRaise,			//�޳������Ƿ�
		int	iFQLX,							//��Ȩ���� 0-����Ȩ;1=��Ȩ
		Tx::Core::Table_Indicator& resTable,//������ݱ�
		int iFlag=0							//��������0-Ĭ�ϣ�1-ծȯ[bCutFirstDateRaise=true��ʾȫ��ģʽ,bCutFirstDateRaise=false��ʾ����ģʽ]
		);

//ծȯ ר��ͳ�Ʋ���
protected:
	virtual double GetBondInterest(SecurityQuotation* pSecurity,int iDate);

public:
	//[��]ծȯYTM�����ڡ��������ڡ�͹��    2013-09-02
    void GetYTM(SecurityQuotation* pSecurity,int iDate,double& dYtm,double& dDuration,double& dMduration,double& dConvexity);
	void GetYTM(SecurityQuotation* pSecurity,double& dYtm,double& dDuration,double& dMduration,double& dConvexity);
	bool GetYTM(SecurityQuotation* pSecurity,int iDate,float fPrice, int nPriceType, double& dYtm,double& dDuration,double& dMduration,double& dConvexity);
	virtual void GetBondYTM(std::vector<int> nSecurityIds);
	virtual void GetBondYTM(int nSecurityId,double &dYtm,double &dMdur,double &dCon);

	float GetPriceByYtmNew(SecurityQuotation* pSecurity,int iDate,double dYtm);
public:
	std::unordered_map<int,BondYTM> m_BondYTMMap;
};


	}
}

#endif
