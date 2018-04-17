/**************************************************************
	Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
	File name:		TxStock.h
	Author:			�Ժ꿡
	Date:			2007-07-09
	Version:		1.0
	Description:	�����Ʊ��ҵ��
					ĳЩ����������ֱ��ʹ��TxBusiness�ṩ�ķ���
***************************************************************/
#ifndef __TXSTOCK_H__
#define __TXSTOCK_H__
#include "TxBusiness.h"
//#include "..\..\Data\DataFileStruct.h"
//#include "..\..\Data\DataFile.h"
#include "TxBusiness.h"
#pragma once
namespace Tx
{
	namespace Business
	{
		const CString IndexDatFileName[19]=
		{
			_T("security_type_index.dat"),												//0
			_T("country_region_index.dat"),
			_T("currency_index.dat"),/*�ñ�û��ID�ֶΣ���F_SEQ���棬ʹ��ʱ��ע��*/
			_T("exchange_index.dat"),
			_T("institution_type_index.dat"),
			_T("accounting_firm_index.dat"),											//5
			_T("industry_index.dat"),/*ʹ��f_industry_id�ֶ�*/
			_T("transaction_object_type_index.dat"),
			_T("event_type_index.dat"),
			_T("ipo_pricing_issue_type_index.dat"),

			//2007-08-22
			//���й�˾ҵ��Ԥ������������
			_T("profit_forecast_result_type_index.dat"),								//10

			_T("fiscal_year_quarter_index.dat"),
			_T("shareholder_type_index.dat"),/*f_type_idΪnchar(1)����  ��ʱ��f_seq����*/
			_T("tradable_shareholder_type_index.dat"),/*f_type_id�ֶ�Ϊnchar(1) ��ʱ����f_seq����*/
			_T("share_change_cause_type_index.dat"),									
			_T("disclosure_type_index.dat"),											//15
			_T("tx_research_type_index.dat"),
			_T("info_file_type.dat"),
			_T("event_dividend_statistics_indicator_security.dat")						//18
		};
		//��Ʊҵ����
		//�������й�Ʊ��ص�ҵ��
		class BUSINESS_EXT TxStock :
			public TxBusiness
		{
			DECLARE_DYNCREATE(TxStock)
		public:
			TxStock(void);
			virtual ~TxStock(void);
		//add by lijw 2008-08-21
		/*	��������ͳ�ƣ�10333��
				�����������ݣ�10334��
				�����������ܣ�10335��
				���������仯��10336��
				���������Ƽ�Ч����10337��

				ӯ��Ԥ��ͳ�ƣ�10338��
				����ӯ��Ԥ�����ݣ�10339��
				ӯ��Ԥ�����ݻ��ܣ�10340��
				ӯ��Ԥ�����ݱ仯��10341��
				ӯ��Ԥ���������ͳ�ƣ�10342��

				��ɫͳ�ƣ�10343��
				��������Ԥ�����ݣ�10344��*/
		private:
			DataFileNormal<blk_TxExFile_FileHead,GradeForecastOfYear>* m_pEvaluateYear;
			DataFileNormal<blk_TxExFile_FileHead,GradeForecastOfSecurity>* m_pEvaluateSecurity;
			DataFileNormal<blk_TxExFile_FileHead,ProfitForecastOfYear>* m_pForcastYear;
			DataFileNormal<blk_TxExFile_FileHead,ProfitForecastOfSecurity>* m_pForcastSecurity;
			GradeForecastOfYear * m_pEvaluateYearData;
			GradeForecastOfSecurity * m_pEvaluateSecurityData;
			GradeForecastOfSecurity * m_pEvaluateSecurityData2;
			ProfitForecastOfYear * m_pForcastYearData;
			ProfitForecastOfSecurity * m_pForcastSecurityData;
			bool LoadEvaluateYear(int id,CString strName);//id��ʾ�ļ���ID����
			bool LoadEvaluateSecurity(int id,CString strName);
			bool LoadForcastYear(int id,CString strName);
			bool LoadForcastSecurity(int id,CString strName);
			bool LoadFinaningSecurityData();	//ȡ������ȯ���� add by wangyc 20100301
			void CombineDate(int iStartDate,int iEndDate,std::set<int>& DateSet);
			//�洢������ȯ����		add by wangyc 20100301
			std::unordered_map<int,float> m_pIdToFinaning;	
			struct FINANCING
			{
				int iID;
				float fFinancingData;
			public:
				int GetMapObj(int index=0) { return iID; }
			};
		public:
			//�����������ݣ�10334��
			bool TxStock::StatEvaluteData(
				Tx::Core::Table_Indicator &resTable,	//������ݱ�
				std::vector<int> & iInstitutionId,		//��������
				std::vector<int> & iStockId,			//��Ʊ����
				int iStartDate,							//��ʼ����
				int iEndDate,						//��������				
				bool bIsAllDate=false					//ȫ������
				);
			//���������仯��10336��
			bool TxStock::StatEvaluteVariety(
				Tx::Core::Table_Indicator &resTable,	//������ݱ�
				std::vector<int> & iInstitutionId,		//��������
				std::vector<int> & iStockId,			//��Ʊ����
				int iStartDate,							//��ʼ����
				int iEndDate,						//��������				
				bool bIsAllDate=false					//ȫ������
				);
			//����ӯ��Ԥ�����ݣ�10339��
			bool TxStock::StatForcastData(
				Tx::Core::Table_Indicator &resTable,	//������ݱ�
				std::vector<int> & iInstitutionId,		//��������
				std::vector<int> & iStockId,			//��Ʊ����
				int iStartDate,							//��ʼ����
				int iEndDate,						//��������				
				bool bIsAllDate=false					//ȫ������
				);
			/*�������ӯ��Ԥ�� -- ���µ�Ԥ�⾻����
			 *iStockId,��Ʊid
			 *expectYear,���
			 */
			double GetForcastNetProfit(int iStockId,int expectYear);
			//�������ӯ��Ԥ�� -- ���µ�Ԥ��ʱ��
			int GetForcastIssueDate(int iStockId);

		public:
			//ר��ͳ��
			//���齻�ף��׶����顢�¹�����
			//���з��䣺����Ԥ�����������ʡ��ֺ����䡢����Ԥ����
			//�ɱ��ɶ����ɱ��仯���ɶ�������ʮ��ɶ�
			//����������������ڡ�ҵ��Ԥ�桢�걨��ݡ���������
			//������Դ����˾�߹�
			//PE/PB����ӯ�ʡ��о��ʡ�
			//ӯ��Ԥ�⣨���ȼ��У�����������Ԥ�����ݡ�����Ͷ������������ӯ��Ԥ�⡢����Ԥ����ܡ�Ԥ��Ч����
			//�ȵ����ݣ����ȼ��ͣ������۹���ͨ����Ȩ������

			//�ɱ��仯ͳ��
			//�����йɱ��仯��¼���ݶ�ȡ�������ڴ���
			bool GetAllVariantShareData(void);
			//����ȡ��ָ��������ָ����������ָ��ָ������ݼ�¼
			bool	StatVariantShare(
				bool	bReloadAllData,			//�Ƿ����¼���ȫ�����ݵı�־
				int		iStartDate,				//��ʼ����,-1��ʾ������ʼ����
				int		iEndDate,				//��ֹ����,-1��ʾ������ֹ����,����-1��ʾȫ������
				std::set<int>& iInstitutionId,//��������
				Tx::Core::Table_Indicator& resTable	//ͳ�ƽ�����ݱ�
				);

			//ʮ��ɶ�ͳ��
			bool GetAllTenShareHoldersData(void);
			//ʮ����ͨ�ɶ�ͳ��
			bool TxStock::GetAllTenTradeableShareHoldersData(void);

			//�ӱ����еõ����ݣ����Խ�����ý�ֹ����
			//ȡ�����ص����ݣ��ٽ��йɶ�����ɸѡ
			bool	StatTenShareHolders(
				bool	bReloadAllData,			//�Ƿ����¼���ȫ�����ݵı�־
				int		iStartDate,				//��ʼ����,-1��ʾ������ʼ����
				int		iEndDate,				//��ֹ����,-1��ʾ������ֹ����,����-1��ʾȫ������
				std::set<int>& iInstitutionId,  //��������
				Tx::Core::Table_Indicator& resTable,	//ͳ�ƽ�����ݱ�
				bool	bTradeableShare=false,	//trueͳ��ʮ����ͨ�ɶ�;falseͳ��ʮ��ɶ�
				bool	bAnnounceDate=false		//true���չ������ڼ���,false���ս�ֹ���ڼ���
				);

			//ҵ��Ԥ��ͳ��
			bool	StatYJYG(
				int iFiscalYear,					//������-����
				int iFiscalQuarter,					//������-���񼾶�
				std::set<int>& iSecurityId,			//��Ҫ�������������뽻��ʵ��ID
				std::set<int>& iTypeId,				//Ԥ������ID
				Tx::Core::Table_Indicator& resTable	//ͳ�ƽ�����ݱ�
				);

			//��˾�߹�T_SENIOR_OFFICER_HOLDING
			BOOL GetSeniorOfficer(//LiLi,�õ�ȫ�����ݺ����ȡ��ָ�����ڷ�Χ�����ݣ�
								  std::set<int> &iSecurityId,		//����ʵ��
								  int iSpecifyDate,					   //ָ������
								  Tx::Core::Table_Indicator &resTable,
									bool bNotAllDate
									);//������ݱ�
			////Add by lijw 2008-10-06
			////�� ����+������-����������
			//bool TxFund::TransReportDateToNormal2(Tx::Core::Table_Indicator &tempTable,int iCol);
			//Add by lijw 2008-10-06
			BOOL GetDateOfReport(
				                 Tx::Core::Table_Indicator &resTable,//������ݱ�
								 std::set<int> &iSecurityId,		//����ʵ��
								 int iStartDate,					//��ʼ����
								 int iEndDate,						//��ֹ����
								 char uReportType = 15,				//��������, �����һλ��, ��λ������,һ�������б������������걨, ��Ҫ�����0/1��ʾ
								 BOOL bActualDate = true,			//�Ƿ�Ϊʵ����¶����    �ò���Ϊ�Ժ��޸ĳ���ʱ���á�             
								 bool bGetAllDate=false				//�Ƿ�ȫ������
								 );

			//�¹�����
			BOOL GetFirstDateOfStock(//LiLi
                                     Tx::Core::Table_Indicator &resTable,//������ݱ�
									std::set<int> &iSecurityId,		//����ʵ��
				                     //std::set<int> &iInstitutionId,   //��������
								     INT uStartDate,					 //��ʼ����
								     INT uEndDate,
									 bool bSpecialDate							//�Ƿ�ָ������
									 );
			//�걨���T_FINANCIAL_REPORT_EXPRESS
			BOOL GetFinancialReport(//LiLi
									Tx::Core::Table_Indicator &resTable,//������ݱ�
									std::set<int> &iSecurityId,		//����ʵ��
									INT uStartDate,					 //(�������ڵ�)��ʼ����
									INT uEndDate			    	 //(�������ڵ�)��ֹ����	
				);

			//ʮ��ɶ�(ָ��30600018-25)
			BOOL GetTopTenShareHolder(//LiLi
										Tx::Core::Table_Indicator &resTable,//������ݱ�
										std::set<int> &iSecurityId,	    	//����ʵ��
										INT uStartDate,					    //(�������ڵ�)��ʼ����
										INT uEndDate,			    	    //(�������ڵ�)��ֹ����
										const CString &strKeyWord,          //�ؼ���(��������)
										bool AllDate,
										BOOL bExactitude = FALSE		    //�Ƿ�ȷ��ѯ
 				);

			//ʮ����ͨ�ɶ�(ָ��30600026-35)
			BOOL GetTopTenShareHolder_Tradable(//LiLi
										Tx::Core::Table_Indicator &resTable,//������ݱ�
										std::set<int> &iSecurityId,	    	//����ʵ��
										INT uStartDate,					    //(�������ڵ�)��ʼ����
										INT uEndDate,			    	    //(�������ڵ�)��ֹ����
										const CString &strKeyWord,          //�ؼ���(��������)
										bool AllDate,
										BOOL bExactitude = FALSE		    //�Ƿ�ȷ��ѯ

				);

		
			//��������
			//add by lijw		2007-08-01
			bool StatIssueFinancing(
				std::set<int> & iInstitutionId,			//��������
				int iStartDate,							//��ʼ����
				int iEndDate,						//��������
				Tx::Core::Table_Indicator &resTable,	//������ݱ�
				int iFXRQ,						//�ǰ���������
				bool bGetAllDate=false,					//�Ƿ�ȫ������
				UINT uFlagType=7						//��־λ	1:�׷�	2:���	3:����	4:��ļ
				);
			//����Ԥ��
			//add by guanyd		2007-08-01
			bool StatDistributionPlan(
				std::set<int> & iInstitutionId,			//��������
				int iFinanceYear,						//���
				int iReportDate	,						//������
				Tx::Core::Table_Indicator &resTable	//������ݱ�
//				bool bCommitted=true					//�Ƿ�ʵʩ
				);

			//�ֺ���Ϣ
			//add by guanyd		2007-08-08
			bool StatDivident(
				std::set<int> iSecurityId,				//����ʵ��ID
				int iStartDate,							//��ʼ��
				int iEndDate,							//��ֹ��
				Tx::Core::Table_Indicator &resTable,	//������ݱ�
				bool bGetAllDate=false,					//�Ƿ�ȫ������
				UINT uFlagType=15						//��־λ	1:�ֺ�	2:��Ϣ	4:���	8:�͹�
				);
			//����Ԥ��
			//add by guanyd		2007-08-10
			bool TxStock::StatFinancingPlan(
				std::set<int> & iSecurityId,			//��������
				int iStartDate,							//��ʼ����
				int iEndDate	,						//��������
				Tx::Core::Table_Indicator &resTable,	//������ݱ�
				bool bGetAllDate=false					//�Ƿ�ȫ������
				);


			//���۹���ͨ
			//add by guanyd		2007-08-10
			bool TxStock::StatLimitedShare(
				std::set<int> & iSecurityId,			//��������
				int iStartDate,							//��ʼ����
				int iEndDate	,						//��������
				Tx::Core::Table_Indicator &resTable,	//������ݱ�
				bool bGetAllDate=false					//ȫ������
				);
			//�ɱ��仯
			//add by guanyd		2007-08-18
			bool StatShareChange(
				std::set<int> & iSecurityId,			//��������
				int iStartDate,							//��ʼ����
				int iEndDate	,						//��������
				Tx::Core::Table_Indicator &resTable,	//������ݱ�
				bool bGetAllDate=false,						//�Ƿ�ȫ������
				UINT uFlagType=31						//��־λ	1:�׷�	2:���	4:����	8:�͹�	16:��Ȩ���øĸ�
				);
			//�ɶ����� modify by lijw 2008-03-13
			//add by guanyd		2007-08-10
			bool StatHolderNumber(
				std::set<int> & iSecurityId,			//��������
				int iStartDate,							//��ʼ����
				int iEndDate	,						//��������
				Tx::Core::Table_Indicator &resTable,	//������ݱ�
				bool IsEnd=true,								//�ж��ǹ������ڻ��ǽ�ֹ����
				bool bIssueAfterStart=true,				//�Ƿ��޳�����ʼ���ں�����
				int SpecifyCount=0						//ָ���Ĺɶ�����
				);

			//��飺������ʾ
			bool BlockTradePrompt(
				std::set<int> iSecurityId,				//����ʵ��ID
				int iStartDate,							//��ʼ��
				int iEndDate,							//��ֹ��
				Tx::Core::Table_Indicator &resTable		//������ݱ�
				);

			//�ۺϣ�������ʾ
			//Add by guanyd
			//������
			bool ColligationTradePromptFPS(
				std::set<int> iSecurityId,				//����ʵ��ID
				int iStartDate,							//��ʼ��
				int iEndDate,							//��ֹ��
				Tx::Core::Table_Indicator &resTable,	//������ݱ�
				bool bGetAllDate=false					//�Ƿ�ȫ������
				);
			//��������
			bool ColligationTradePromptFXZF(
				std::set<int> iSecurityId,				//����ʵ��ID
				int iStartDate,							//��ʼ��
				int iEndDate,							//��ֹ��
				Tx::Core::Table_Indicator &resTable,	//������ݱ�
				bool bGetAllDate=false					//�Ƿ�ȫ������
				);
			//��������
			bool ColligationTradePromptFXSS(
				std::set<int> iSecurityId,				//����ʵ��ID
				int iStartDate,							//��ʼ��
				int iEndDate,							//��ֹ��
				Tx::Core::Table_Indicator &resTable,	//������ݱ�
				bool bGetAllDate=false					//�Ƿ�ȫ������
				);	
			//���
			bool ColligationTradePromptPG(
				std::set<int> iSecurityId,				//����ʵ��ID
				int iStartDate,							//��ʼ��
				int iEndDate,							//��ֹ��
				Tx::Core::Table_Indicator &resTable,	//������ݱ�
				bool bGetAllDate=false,				//�Ƿ�ȫ������
				bool bIsFXRQ = false,                    //�Ƿ�������ͳ�ơ�
				bool bIsSSRQ = false						//�ǰ���������ͳ��
				);


			//�����շ���[�߼�]
			bool BlockRiskIndicatorAdv(
				int iMenuID,						//����ID
				Tx::Core::Table_Indicator& resTable,//������ݱ�
				std::set<int>& lSecurityId,			//����ʵ��ID
				int iEndDate,						//��ֹ����
				long lRefoSecurityId,				//��׼����ʵ��ID
				int iStartDate=0,					//��ʼ����
				int iTradeDaysCount=20,				//��������
				int iDuration=0,					//��������0=�գ�1=�ܣ�2=�£�3=����4=��
				bool bLogRatioFlag=false,			//������ʵķ�ʽ,true=ָ������,false=�򵥱���
				bool bClosePrice=true,				//ʹ�����̼�
				int  iStdType=0,					//��׼������1=��ƫ��������ƫ
				int  iOffset=0,						//ƫ��
				bool bAhead=true,					//��Ȩ��־,true=ǰ��Ȩ,false=��Ȩ
				int bUserRate=0,					//0=�����޷��������ʣ�1ȡһ�궨�ڻ�׼���ʼ����޷��������ʣ�2�û��Զ��������ʼ����޷���������
				double dUserRate=0.034,				//�û��Զ���������,����ȡһ�궨�ڻ�׼����
				bool bDayRate=true,					//������
				bool bPrw = false,					//�Ƿ����������
				bool bMean = false                  //�����ֵ
				);

			//bool GetPEValue(
			//	//Tx::Core::Table_Indicator& resTable,// ������ݱ�
			//	Tx::Core::Table& resultTable,
			//	std::set<int> iSecurityId,			// ����ʵ��ID
			//	UINT nStartDate,					// ��ʼ����
			//	UINT nEndDate,						// ��ֹ����
			//	int iSJZQ,							// ʱ������ 1-��,2-��,3-��,4-��
			//	int iHZYB,							// �������� 1-ȫ��,2-�޳�����,3-�޳�΢��(Ԫ)
			//	double dwl,						// �޳�΢��ʱ��΢��
			//	int iJQFS,							// ��Ȩ��ʽ 1-�ܹɱ�,2-��ͨ�ɱ�
			//	int iCWSJ,							// ��������:�����ڵ�ѡ�� ��λ�� 0xFFFF
			//										// �ɸߵ��ͷֱ��ʾ ָ��������/ѡ�񱨸���,�걨,�б�,����
			//										// ָ�������ڵı����ڸ�ʽ
			//	UINT nFiscalyear,					// ����
			//	UINT nFiscalyearquater,				// �Ƽ�
			//	int iJSFF,							// ���㷽�� 1-��,2-����,3-ͬ��
			//	bool bClosePrice					// ʹ�����̼�(true),����(false)
			//	);

			//// test for load when enter
			//bool GetPEValue(
			//	//Tx::Core::Table_Indicator& resTable,// ������ݱ�
			//	Tx::Core::Table& resultTable,
			//	std::set<int> iSecurityId,			// ����ʵ��ID
			//	UINT nStartDate,					// ��ʼ����
			//	UINT nEndDate,						// ��ֹ����
			//	int iSJZQ,							// ʱ������ 1-��,2-��,3-��,4-��
			//	int iHZYB,							// �������� 1-ȫ��,2-�޳�����,3-�޳�΢��(Ԫ)
			//	double dwl,						// �޳�΢��ʱ��΢��
			//	int iJQFS,							// ��Ȩ��ʽ 1-�ܹɱ�,2-��ͨ�ɱ�
			//	int iCWSJ,							// ��������:�����ڵ�ѡ�� ��λ�� 0xFFFF
			//	// �ɸߵ��ͷֱ��ʾ ָ��������/ѡ�񱨸���,�걨,�б�,����
			//	// ָ�������ڵı����ڸ�ʽ
			//	UINT nFiscalyear,					// ����
			//	UINT nFiscalyearquater,				// �Ƽ�
			//	int iJSFF,							// ���㷽�� 1-��,2-����,3-ͬ��
			//	bool bClosePrice,					// ʹ�����̼�(true),����(false)
			//	std::unordered_map<int,Tx::Core::Unique_Pos_Map*>* pid_posinfo_map = NULL
			//	);

			//bool GetPBValue(
			//	//Tx::Core::Table_Indicator& resTable,// ������ݱ�
			//	Tx::Core::Table& resultTable,
			//	std::set<int> iSecurityId,			// ����ʵ��ID
			//	UINT nStartDate,					// ��ʼ����
			//	UINT nEndDate,						// ��ֹ����
			//	int iSJZQ,							// ʱ������ 1-��,2-��,3-��,4-��
			//	int iHZYB,							// �������� 1-ȫ��,2-�޳�����,3-�޳�΢��(Ԫ)
			//	double dwl,						// �޳�΢��ʱ��΢��
			//	int iJQFS,							// ��Ȩ��ʽ 1-�ܹɱ�,2-��ͨ�ɱ�
			//	int iCWSJ,							// ��������:�����ڵ�ѡ�� ��λ�� 0xFFFF
			//										// �ɸߵ��ͷֱ��ʾ ָ��������/ѡ�񱨸���,�걨,�б�,����
			//										// ָ�������ڵı����ڸ�ʽ
			//	UINT nFiscalyear,					// ����
			//	UINT nFiscalyearquater,				// �Ƽ�
			//	int iJSFF,							// ���㷽�� 1-��,2-����,3-ͬ��
			//	bool bClosePrice					// ʹ�����̼�(true),����(false)
			//	);
			//bool GetPBValue(
			//	//Tx::Core::Table_Indicator& resTable,// ������ݱ�
			//	Tx::Core::Table& resultTable,
			//	std::set<int> iSecurityId,			// ����ʵ��ID
			//	UINT nStartDate,					// ��ʼ����
			//	UINT nEndDate,						// ��ֹ����
			//	int iSJZQ,							// ʱ������ 1-��,2-��,3-��,4-��
			//	int iHZYB,							// �������� 1-ȫ��,2-�޳�����,3-�޳�΢��(Ԫ)
			//	double dwl,						// �޳�΢��ʱ��΢��
			//	int iJQFS,							// ��Ȩ��ʽ 1-�ܹɱ�,2-��ͨ�ɱ�
			//	int iCWSJ,							// ��������:�����ڵ�ѡ�� ��λ�� 0xFFFF
			//	// �ɸߵ��ͷֱ��ʾ ָ��������/ѡ�񱨸���,�걨,�б�,����
			//	// ָ�������ڵı����ڸ�ʽ
			//	UINT nFiscalyear,					// ����
			//	UINT nFiscalyearquater,				// �Ƽ�
			//	int iJSFF,							// ���㷽�� 1-��,2-����,3-ͬ��
			//	bool bClosePrice,					// ʹ�����̼�(true),����(false)
			//	std::unordered_map<int,Tx::Core::Unique_Pos_Map*>* pid_posinfo_map = NULL
			//	);
			//2007-08-07
			//һ����ҵ�����(�»��׼��)
			bool GetIncomeStatementCommercialIndustryActstd(
				Tx::Core::Table_Indicator& resTable,//������ݱ�
				int iSecurityId,					//����ʵ��ID
				int iConsolidated,					//1=ĸ��˾;2=�ϲ�;3=ĸ��˾|�ϲ�
				int iFiscalYear,					//������-����
				int iFiscalQuarter,					//������-���񼾶�
				int iAjustment						//0���Ȳ��ǵ���ǰҲ���ǵ����� 1������ǰ 2��������  3�����ǵ���ǰҲ�ǵ�����
				);



			//2007-08-09
			//add by guanyd
			//���ܣ����Ƚ���������ӵ�map��
			//����Դ��ָ������Ϊ������map�л�ȡ������ӵ�Դ��ָ��λ��,��������Ĭ��Ϊ0��map
		//	template<class T>
			bool FillColumn(
				Tx::Core::Table_Indicator &m_transTable,		//������
				int addColumn,								//�����������ݵ���
				Tx::Core::Table_Indicator &m_resTableAddColumn,	//Դ��
				int indexColumn,							//Դ����Ϊmap��������
				int	insertColumn							//Ҫ�������ݵ���
			);
			
			//2007-09-04
			//guanyd
			//������ʵ��idתΪȯid���߻���id
			bool TxStock::TransObjectToSecIns(
				std::set<int>	sObjectId,		//����ʵ��id
				std::set<int> &sSecInsId,		//ȯid�����id
				int iType						//id���ͣ�1:ȯid	2:����id
				);
			bool TxStock::TransObjectToSecIns(
				std::set<int>	sObjectId,		//����ʵ��id
				std::vector<int> &sSecInsId,		//ȯid�����id
				int iType						//id���ͣ�1:ȯid	2:����id
				);
			//guanyd
			//2007-09-04
			//������ѡ�����ڻ��ݵ���һ����������
			bool TxStock::ChangeDateToReportdate(int& iStartDate,int& iEndDate);

			//Add by guanyd
			//2007-08-31
			//�����ű����ȯid���룬��Ϊid��ȫ��Ϊ1600���ң������ڱ����г��ֲ����ظ�
			//
			bool TxStock::AlignRows(
					Tx::Core::Table_Indicator& resTableFir,		//�ϴ�ı�
					int iFirCol,								//�������չʽ��֤����һ�µ���
					Tx::Core::Table_Indicator& resTableSec,		//����չ�ı�
					int iSecCol									//��չ���б�֤����һ�µ���
					);
			
			//��Ӫҵ���Ʒ����
			bool GetIncomeMain(
				Tx::Core::Table_Indicator& resTable,//������ݱ�
				int iSecurityId,					//����ʵ��ID
				int iFiscalYear,					//������-����
				int iFiscalQuarter					//������-���񼾶�
				);
			
			public:
				//��ȡ�����ļ���Ϣ
				BOOL GetIndexDat(int file_id,	//�����ļ���ʶ���ļ�����IndexDatFileName�����е�λ�� 
				std::unordered_map<int,CString>& indexdat_map //��Ŷ�ȡ����Ϣ
				);
			public:
				struct IndexStruct
				{
					int	id;				//��������ID
					char description[200];	//��������,���ݱ���Ƴ���Ϊ��Ŀǰ�ݶ�
				};

				// added by zhoup 2007.12.20
				// �µ���ӯ�ʼ���ӿ�
				bool GetPEPBValue(
					//Tx::Core::Table_Indicator& resTable,// ������ݱ�
					Tx::Core::Table& resultTable,
					std::set<int> iSecurityId,			// ����ʵ��ID
					UINT nStartDate,					// ��ʼ����
					UINT nEndDate,						// ��ֹ����
					int iSJZQ,							// ʱ������ 1-��,2-��,3-��,4-��
					int iHZYB,							// �������� 1-ȫ��,2-�޳�����,3-�޳�΢��(Ԫ)
					double dwl,						// �޳�΢��ʱ��΢��
					int iJQFS,							// ��Ȩ��ʽ 1-�ܹɱ�,2-��ͨ�ɱ�
					int iCWSJ,							// ��������:�����ڵ�ѡ�� ��λ�� 0xFFFF
					// �ɸߵ��ͷֱ��ʾ ָ��������/ѡ�񱨸���,�걨,�б�,����
					// ָ�������ڵı����ڸ�ʽ
					UINT nFiscalyear,					// ����
					UINT nFiscalyearquater,				// �Ƽ�
					int iJSFF,							// ���㷽�� 1-��,2-����,3-ͬ��,4-��̬
					bool bClosePrice,					// ʹ�����̼�(true),����(false)
					bool bPE,							// �Ƿ����PE
					bool bPB,							// �Ƿ����PB
					std::unordered_map<int,Tx::Core::Unique_Pos_Map*>* pid_posinfo_map = NULL
					);

				// added by zhoup 2009.04.28
				// �����ӯ��ͳ�ƽӿ�
				bool GetBlockPEPB(
					Tx::Core::Table& resultTable,
					std::set<int> setBlockId,			// ���ID
					UINT nStartDate,					// ��ʼ����
					UINT nEndDate,						// ��ֹ����
					int iSJZQ,							// ʱ������ 1-��,2-��,3-��,4-��
					int iJSFF,							//���㷽��
					bool bPE,							// �Ƿ����PE
					bool bPB							// �Ƿ����PB
					);
				// added by zhangxs 2009.11.24
				// ����ʽ�����ͳ�ƽӿ�
				bool GetBlockCashFlow(
					Tx::Core::Table_Display& resultTable,
					UINT nStartDate,					// ��ʼ����
					UINT nEndDate,						// ��ֹ����
					int iResqType=0,					// ��������
					bool bStat = true,
					bool bDurRaise = true,
					int iMarketid = 0
					);
				// added by zhangxs 2009.11.30
				// �����ʽ�����ͳ�ƽӿ�
				bool GetSampleCashFlow(
					Tx::Core::Table_Display& resultTable,
					std::vector<int> vecBlockId,		// ���ID
					UINT nStartDate,					// ��ʼ����
					UINT nEndDate,						// ��ֹ����
					int iResqType,						// ��������
					bool bAddSamplesOnly=false,			//���������ͳ��
					bool bStat = true,
					bool bDurRaise = true,
					int iMarketid = 0,
					bool bFocusSamples = true
					);
				//ȡ���������Ȩ�������
				//add by wangyc 20081224
				bool GetDataOfProfitAndRight(
					int iYear,							//����
					int iQuarter,						//�Ƽ�
					bool iIsProfit,						//��������Ȩ�� true ���� false Ȩ��
					std::vector<int>& iSecurityId,			//������ID
					std::vector<double>* iDateVector	//����������Ȩ������
					);
				//2008-04-25
				//�������-������
				virtual bool GetBlockAnalysisCol(Table_Display& baTable,std::vector<int>& arrSamples,int& iSortCol);
				virtual bool SetBlockAnalysisBasicCol(Table_Display& baTable,SecurityQuotation* psq,int& j,int ii,int idate,int iSamplesCount);
				virtual bool SetBlockAnalysisCol(Table_Display& baTable,SecurityQuotation* pSecurity,int& nCol,int nRow);
				//2008-11-03
				//AH�������
				bool GetAHData(std::vector<int> iSecurityId,int startDate,int endDate,Table_Display& baTable);

				// ����һ�����ʵĽṹ��PE/PB������
				struct ExRate 
				{
					int iDate;
					double iRatio;
				public:
					int GetMapObj(int index=0) { return iDate; }
				};
		};
	}
}
#endif