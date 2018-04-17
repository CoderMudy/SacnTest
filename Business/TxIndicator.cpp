/*************************************************
  Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
  File name:	TxIndicator.cpp
  Author:       �Ժ꿡
  Version:		1.0
  Date:			2008-06-10
  
  Description:	
				ָ�������
				��ͼͳһ�ӿ�
*************************************************/

#include "StdAfx.h"
#include "TxIndicator.h"
#include "..\..\Business\Business\ComputingWarrant.h"
#include "Markup.h"

#include "..\..\Business\Business\TxStock.h"
#include "..\..\business\business\Fund.h"
#include "..\..\Data\TypeMapManage.h"
#include "..\..\Data\SecurityExtentInfo.h"

//��鲢�ȴ��߳��Ƿ��˳�
void WaitThreadExit(CWinThread* pThread)
{
	if(pThread==NULL)
		return;

	DWORD ExitCode;
	while(GetExitCodeThread(pThread->m_hThread,&ExitCode)!=FALSE)
	{
		MSG msg;
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != FALSE)
			::SendMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
		if(ExitCode == STILL_ACTIVE)
			continue;

		break;
	}
}

namespace Tx
{
	namespace Business
	{
		struct CashFlow_YH
		{
			//�ļ�λ�ã�30263\  iId;--����ʵ��
			//�ļ�λ�ã�30264\  iId;--����Ƽ�
			int iId;
			double dCashFlow[76];
		public:
			int GetMapObj(int index=0) { return iId; }
		};

		struct CashFlow_BX
		{
			//�ļ�λ�ã�30267\  iId;--����ʵ��
			//�ļ�λ�ã�30268\  iId;--����Ƽ�
			int iId;
			double dCashFlow[79];
		public:
			int GetMapObj(int index=0) { return iId; }
		};

		struct CashFlow_ZQ
		{
			//�ļ�λ�ã�30265\ iId;--����ʵ��
			//�ļ�λ�ã�30266\ iId;--����Ƽ�
			int iId;
			double dCashFlow[71];
		public:
			int GetMapObj(int index=0) { return iId; }
		};
		//��������
		struct Profit_YH
		{
			//�ļ�λ�ã�30269 iId;--����ʵ��
			//�ļ�λ�ã�30270 iId;--����Ƽ�
			int iId;
			double dProfit[30];
		public:
			int GetMapObj(int index=0) { return iId; }
		};

		struct Profit_BX
		{
			//�ļ�λ�ã�30271\ iId;--����ʵ��
			//�ļ�λ�ã�30272\ iId;--����Ƽ�
			int iId;
			double dProfit[40];
		public:
			int GetMapObj(int index=0) { return iId; }
		};

		struct Profit_ZQ
		{
			//�ļ�λ�ã�30273\ iId;--����ʵ��
			//�ļ�λ�ã�30274\ iId;--����Ƽ�
			int iId;
			double dProfit[29];
		public:
			int GetMapObj(int index=0) { return iId; }
		};

		//�ʲ���ծ��
		struct Balance_YH 
		{
			//�ļ�λ�ã�30275\ iId;--����ʵ��
			//�ļ�λ�ã�30276\ iId;--����Ƽ�
			int iId;
			//��������
			double dBalance[85];

		public:
			int GetMapObj(int index=0) { return iId; }
		};
		struct Balance_BX 
		{
			//�ļ�λ�ã�30277\  iId;--����ʵ��
			//�ļ�λ�ã�30278\  iId;--����Ƽ�
			int iId;
			//��������
			double dBalance[99];

		public:
			int GetMapObj(int index=0) { return iId; }
		};
		struct Balance_ZQ 
		{
			//�ļ�λ�ã�30279\  iId;--����ʵ��
			//�ļ�λ�ã�30280\  iId;--����Ƽ�
			int iId;
			//��������
			double dBalance[84];

		public:
			int GetMapObj(int index=0) { return iId; }
		};

		//-------------������һЩ�̶��Ĳ���-------------------------wangzhy
		const char * cReportTypes[2] = {"ĸ��˾", "�ϲ�"};
		const char * cAccountingPolicyTypes[2] = {"����ǰ", "������"};
		const char * cIRTypes[2] = {"ǰ��Ȩ", "��Ȩ"};
		const char * cPriceTypes[6] = {"���̼�", "ǰ�ռ�", "��߼�", "��ͼ�", "���̼�", "�վ���"};
		const char * cValueTypes[8] = {"�ɽ���", "�ɽ����", "������", "��ͨ��ֵ", "������ֵ", "����ֵ", "������", "������"};
		const char * cPFADSubjects[4] = {"������", "����������", "����ת����", "��ĩ���"};
		const char * cIncomeDistribution[7] = {"����", "����ռ��(%)", "�ɱ�", "ë��", "ë��ռ��(%)", "ë����(%)", "ë���ʱ仯(%)"};
		const char * cAccountsReceivable[4] = {"Ӧ���˿���", "Ӧ���˿�ռ��(%)","����׼��ռ��(%)" , "����׼�����"};
		const char * cCWBriefString[3] = {"���ʦ���", "��ƻ��ʦ", "���ʦ������"};
		const char * cFundReportTypes[6] = {"һ����","������","�б�","������","�ļ���","�걨"};      //by zhangxs 20080808

TxIndicator::TxIndicator(void)
{
	m_bInited = false;
	m_bInitTables = false;

	m_hEvent_ever = NULL;
	m_hEvent_timer = NULL;

	//��ָ֤��
	iSHIndexID = 4000208;
	//���ڳ�ָ
	iSZIndexID = 4000001;

	bBalanceInstitution = TRUE;	//Ĭ�ϰ��չ�˾����
	bFinancialInstitution = TRUE;
	bIncomeInstitution = TRUE;
	bCashflowInstitution = TRUE;
	bRevenueInstitution = TRUE;
	bAccountInstitution = TRUE;
	bGainsAndLossesInstitution = TRUE;
	bDepreciationInstitution = TRUE;
	bFinanceChargeInstitution = TRUE;
	bHQSecurity = TRUE;

	m_pBusiness = new Tx::Business::TxBusiness();
	bsetAIsBusy = FALSE;
	m_nRefreshTime = m_nRefreshTime_RealTime = 0;
	m_bCalculate = FALSE;
	CString sPath = Tx::Core::Manage::GetInstance()->m_pUserInfo->GetUserIni();
	int nSyn = GetPrivateProfileInt(_T("Excel"),_T("Syn"),0,sPath );
	if ( nSyn == 0 )
		m_bSynFlag = FALSE;
	else
		m_bSynFlag = TRUE;
	GetExcelConfig();
	m_nShCalcEvtCnt = 0;		//ʵ������Ϊ0
	m_bAnyShCalcEvt = FALSE;
	m_bReqMapCleared = FALSE;
	m_bRun_HisIndCalc = TRUE;
	m_bRun_RealTimeIndCalc = TRUE;
	m_bRun_ClearReqMap = TRUE;
	m_bTerm_HisIndCalc = FALSE;
	m_bTerm_RealTimeIndCalc = FALSE;
	m_bTerm_ClearReqMap = FALSE;

	param_HisIndCalc.lpbThreadRun = &m_bRun_HisIndCalc;
	param_HisIndCalc.lpbThreadTerm = &m_bTerm_HisIndCalc;
	param_HisIndCalc.lpnRefreshTime = &m_nRefreshTime;
	param_HisIndCalc.lpbCalculate = &m_bCalculate;
	param_HisIndCalc.lpnShCalcEvtCnt = &m_nShCalcEvtCnt;
	param_HisIndCalc.lpbsetAIsBusy = &bsetAIsBusy;
	param_HisIndCalc.lpbAnyShCalcEvt = &m_bAnyShCalcEvt;
	param_HisIndCalc.pCS = &_critsect;
	param_HisIndCalc.pBusiness = m_pBusiness;
	param_HisIndCalc.pmapReq = &mapRequest;
	param_HisIndCalc.psetReqA = &setReqA;
	param_HisIndCalc.psetReqB = &setReqB;

	param_RealTimeIndCalc.lpbThreadRun = &m_bRun_RealTimeIndCalc;
	param_RealTimeIndCalc.lpbThreadTerm = &m_bTerm_RealTimeIndCalc;
	param_RealTimeIndCalc.lpnRefreshTimeRealTime = &m_nRefreshTime_RealTime;
	param_RealTimeIndCalc.pCSRealTime = &_critsect_RealTime;
	param_RealTimeIndCalc.pmapReqRealTime = &mapReqRealTime;

	param_ClearReqMap.lpbReqMapCleared = &m_bReqMapCleared;
	param_ClearReqMap.lpbAnyShCalcEvt = &m_bAnyShCalcEvt;
	param_ClearReqMap.lpbThreadRun = &m_bRun_ClearReqMap;
	param_ClearReqMap.lpbThreadTerminated = &m_bTerm_ClearReqMap;
	param_ClearReqMap.lpnRefreshTime = &m_nRefreshTime;
	param_ClearReqMap.lpbCalculate = &m_bCalculate;

	m_bIsHisThreadStarted = false;
	m_bIsRealThreadStarted = false;

	//--wangzhy--20080626
	//����Ϊ���Ż�ͬ���������ӵĳ�Ա����
	m_pBalance = NULL;	//---20080528	
	m_pFinancial = NULL;
	m_pCashFlow =NULL;
	m_pIncome = NULL;
	//wangzhy-------------20080527
	m_pHolder		 = NULL;	//--wangzhy--20080603
	m_pTradeableHolder = NULL;	//--wangzhy--20080603
	m_pFundBondHolder = NULL;		//--wangzhy--20080604
	m_pRevenue	= NULL;	//--wangzhy--20080604
	m_pDepreciation = NULL;	//--wangzhy--20080604
	m_pGainsAndLosses = NULL;	//--wangzhy--20080604
	m_pAccountReceivable = NULL;	//--wangzhy--20080604
	m_pFinanceCharge = NULL;	//--wangzhy--20080604
	m_pIncomeDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::Income>;
	m_pIncomeDataFile->SetCheckLoadById(true);
	m_pCashFlowDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::CashFlow>;	
	m_pCashFlowDataFile->SetCheckLoadById(true);
	m_pFinancialDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::Financial>;	
	m_pFinancialDataFile->SetCheckLoadById(true);
	m_pBalanceDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::Balance>;//20080529	
	m_pBalanceDataFile->SetCheckLoadById(true);
	m_pRevenueDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::PrimeOperatingRevenue>;//20080605
	m_pRevenueDataFile->SetCheckLoadById(true);
	m_pDepreciationDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::AssetsDepreciationReserves>;//20080610
	m_pDepreciationDataFile->SetCheckLoadById(true);
	m_pGainsAndLossesDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::NonRecurringGainsAndLosses>;//20080610
	m_pGainsAndLossesDataFile->SetCheckLoadById(true);
	m_pAccountsReceivableDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::AccountsReceivable>;//20080610
	m_pAccountsReceivableDataFile->SetCheckLoadById(true);
	m_pFinanceChargeDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FinanceCharge>;//20080610
	m_pFinanceChargeDataFile->SetCheckLoadById(true);
	m_pHisDetail = NULL;
	//-----------zhangxs---20080822------
	m_pFundArchievement		= NULL;
	m_pFundArchievementDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundAchievement>;
	m_pFundArchievementDataFile->SetCheckLoadById(true);
	m_pFundFinancial		= NULL;
	m_pFundFinancialDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundFinancial>;
	m_pFundFinancialDataFile->SetCheckLoadById(true);
	m_pFundBalance			= NULL;
	m_pFundBalanceDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundBalance>;
	m_pFundBalanceDataFile->SetCheckLoadById(true);
	m_pFundNavChange		= NULL;
	m_pFundNavChangeDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundNavChange>;
	m_pFundNavChangeDataFile->SetCheckLoadById(true);
	m_pFundRevenue			= NULL;
	m_pFundRevenueDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundRevenue>;
	m_pFundRevenueDataFile->SetCheckLoadById(true);
	m_pFundTxIndustryDistribute	= NULL;
	m_pFundTxIndustryDistributeDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundBlockInvestmentTxIndustryDistribute>;
	m_pFundTxIndustryDistributeDataFile->SetCheckLoadById(true);
	m_pFundComIndustryDistribute	= NULL;
	m_pFundComIndustryDistributeDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundCombineInvestmentIndustryDistribute>;
	m_pFundComIndustryDistributeDataFile->SetCheckLoadById(true);
	m_pFundIndexIndustryDistribute	= NULL;
	m_pFundIndexIndustryDistributeDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundIndexInvestmentIndustryDistribute>;
	m_pFundIndexIndustryDistributeDataFile->SetCheckLoadById(true);
	m_pFundActiveIndustryDistribute	= NULL;
	m_pFundActiveIndustryDistributeDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundStockInvestmentIndustryDistribute>;
	m_pFundActiveIndustryDistributeDataFile->SetCheckLoadById(true);
	m_pFundInvesmentGroup = NULL;
	m_pFundInvesmentGroupDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundInvesmentGroup>;
	m_pFundInvesmentGroupDataFile->SetCheckLoadById(true);
	m_pFundstockVipStock = NULL;
	m_pFundstockVipStockDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::Fund_VIP_Stock>;
	m_pFundstockVipStockDataFile->SetCheckLoadById(true);
	m_pFundIndexVIPStock =NULL;
	m_pFundIndexVIPStockDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundIndexInvestmentVIPStock>;
	m_pFundIndexVIPStockDataFile->SetCheckLoadById(true);
	m_pFundCombinVIPStock = NULL;
	m_pFundCombinVIPStockDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundCombineInvestmentVIPStock>;
	m_pFundCombinVIPStockDataFile->SetCheckLoadById(true);

	m_pAmountFlow = NULL;

	m_setSWIndex.clear();
	Tx::Business::TxStock* m_pStock;
	Tx::Data::Collection* m_pCol = NULL;
	m_pStock = new Tx::Business::TxStock();
	//����ȫ��A��Ʊ������
	bool result = m_pStock->m_pFunctionDataManager->LoadLeafCollectionSample(50010329);
	if (result)
	{
		std::set<int> m_setSWIdTemp;		
		for(int ix = 50010330; ix <=50010338;ix++ )
		{
			m_pCol = NULL;
			m_pCol = m_pStock->m_pFunctionDataManager->GetCollection(ix); 
			m_setSWIdTemp.clear();
			if(m_pCol!=NULL)
			{
				m_pCol->GetCollectionItems(m_setSWIdTemp);
				std::set<int>::iterator m_iter = m_setSWIdTemp.begin();
				for(;m_iter != m_setSWIdTemp.end();m_iter++)
				{
					int m_id = *m_iter;
					if(m_id > 0)
						m_setSWIndex.insert(m_id);
				}
			}
		}
		m_pCol = NULL;
	}	
	delete m_pStock;
	//------------------------------------
	FundNameMap.clear();
	TypeMapManage::GetInstance()->GetTypeMap(59,FundNameMap);
}

TxIndicator::~TxIndicator(void)
{
	if(m_pBusiness!=NULL)
		delete m_pBusiness;

	ClearTables();
	//mapSheetReq.clear();
	mapRequest.clear();
	mapReqRealTime.clear();
	if(setReqA.size() > 0)
	{
		setReqA.clear();
	}
	if(setReqB.size() > 0)
	{
		setReqB.clear();
	}
	//if ( m_pHolder )
	//	delete m_pHolder;
	//if ( m_pTradeableHolder )
	//	delete m_pTradeableHolder;
	//if (m_pFundBondHolder)
	//	delete m_pFundBondHolder
	//if (m_pRevenue)
	//	delete m_pRevenue;
	//if (m_pDepreciation)
	//	delete m_pDepreciation;
	//if (m_pGainsAndLosses)
	//	delete m_pGainsAndLosses;
	//if (m_pAccountReceivable)
	//	delete m_pAccountReceivable;
	//if (m_pFinanceCharge)
	//	delete m_pFinanceCharge;
	if(m_pIncomeDataFile)
		delete	m_pIncomeDataFile;
	if(m_pCashFlowDataFile)
		delete m_pCashFlowDataFile;
	if (m_pFinancialDataFile)
		delete	m_pFinancialDataFile;
	if (m_pBalanceDataFile)
		delete m_pBalanceDataFile;
	if (m_pRevenueDataFile)
		delete m_pRevenueDataFile;
	if (m_pDepreciationDataFile)
		delete m_pDepreciationDataFile;
	if (m_pGainsAndLossesDataFile)
		delete m_pGainsAndLossesDataFile;
	if (m_pAccountsReceivableDataFile)
		delete m_pAccountsReceivableDataFile;
	if (m_pFinanceChargeDataFile)
		delete m_pFinanceChargeDataFile;
	//-----------zhangxs----20080822--
	if (m_pFundArchievementDataFile)
		delete m_pFundArchievementDataFile;
	if (m_pFundFinancialDataFile)
		delete m_pFundFinancialDataFile;
	if (m_pFundBalanceDataFile)
		delete m_pFundBalanceDataFile;
	if (m_pFundNavChangeDataFile)
		delete m_pFundNavChangeDataFile;
	if (m_pFundRevenueDataFile)
		delete m_pFundRevenueDataFile;
	if (m_pFundTxIndustryDistributeDataFile)
		delete m_pFundTxIndustryDistributeDataFile;
	if ( m_pFundComIndustryDistributeDataFile )
		delete m_pFundComIndustryDistributeDataFile;
	if ( m_pFundIndexIndustryDistributeDataFile )
		delete m_pFundIndexIndustryDistributeDataFile;
	if ( m_pFundActiveIndustryDistributeDataFile )
		delete m_pFundActiveIndustryDistributeDataFile;
	if ( m_pFundInvesmentGroupDataFile )
		delete m_pFundInvesmentGroupDataFile;
	if ( m_pFundstockVipStockDataFile )
		delete m_pFundstockVipStockDataFile;
	if ( m_pFundIndexVIPStockDataFile )
		delete m_pFundIndexVIPStockDataFile;
	if (m_pFundCombinVIPStockDataFile)
		delete m_pFundCombinVIPStockDataFile;

}


//2008-06-10
//ȡ�������е���Ŀ--------------wangzhy-------------------------
void TxIndicator::FindItemOfReq(TXFuncReq *preqTXFunction, CString & strRequest, int iItemFlg)
{
	int i, iFirst, iLast;
	TXEE_Indicator_Iter iterIndicator;

	memset(preqTXFunction->cName, 0, 32);
	iFirst = 0;
	switch(iItemFlg)
	{
	case 0:		//FuncID��SubItemID��IndicatorID
		preqTXFunction->nIndicatorID = 0;
		iLast = strRequest.Find(_T(';'), iFirst);
		preqTXFunction->iFuncID = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
		iFirst = iLast + 1;
		iLast = strRequest.Find(_T(';'), iFirst);
		preqTXFunction->iSubItemID = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
		iterIndicator = mapTXEEIndicator.find(preqTXFunction->iFuncID * 100 + preqTXFunction->iSubItemID);
		if(iterIndicator == mapTXEEIndicator.end())
		{
			preqTXFunction->nIndicatorID = 0;
		}else
		{
			preqTXFunction->nIndicatorID = iterIndicator->second.nIndicatorID;
		}
		break;
	case 1:		//Security
		for(i = 0; i < 2; i++)
		{	//FuncID��SubItemID
			iLast = strRequest.Find(_T(';'), iFirst);
			iFirst = iLast + 1;
		}
		iLast = strRequest.Find(_T(';'), iFirst);
		if(iLast > 0)
		{
			preqTXFunction->nSecurityID = _ttol(strRequest.Mid(iFirst, iLast - iFirst));
		}else
		{
			preqTXFunction->nSecurityID = _ttol(strRequest.Mid(iFirst));
		}
		break;
	case 2:		//FuncID��SubItemID��Security
		iLast = strRequest.Find(_T(';'), iFirst);
		preqTXFunction->iFuncID = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
		iFirst = iLast + 1;
		iLast = strRequest.Find(_T(';'), iFirst);
		preqTXFunction->iSubItemID = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
		iFirst = iLast + 1;
		iLast = strRequest.Find(_T(';'), iFirst);
		if(iLast > 0)
		{
			preqTXFunction->nSecurityID = _ttol(strRequest.Mid(iFirst, iLast - iFirst));
			if(preqTXFunction->nSecurityID == 0)
			{
				iFirst = iLast + 1;
				iLast = strRequest.Find(_T(';'), iFirst);
				strcpy_s(preqTXFunction->cName, 32, strRequest.Mid(iFirst, iLast - iFirst));
				iFirst = iLast + 1;
				preqTXFunction->iIndexFlg = _ttoi(strRequest.Mid(iFirst));

			}
		}else
		{
			preqTXFunction->nSecurityID = _ttol(strRequest.Mid(iFirst));
		}
		break;
	case 3:		//Date
	case 4:		//��������
	case 20:	//�������
	case 22:	//��ҵ�㼶
	case 23:	//���������
		for(i = 0; i < 3; i++)
		{	//FuncID��SubItemID��Security
			iLast = strRequest.Find(_T(';'), iFirst);
			iFirst = iLast + 1;
		}
		iLast = strRequest.Find(_T(';'), iFirst);
		switch(iItemFlg)
		{
		case 3:		//Date
			if(iLast > 0)
			{
				preqTXFunction->nDate = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
			}else
			{
				preqTXFunction->nDate = _ttoi(strRequest.Mid(iFirst));
			}
			break;
		case 4:		//��������
			if(iLast > 0)
			{
				preqTXFunction->iINDEXSampleIdx = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
			}else
			{
				preqTXFunction->iINDEXSampleIdx = _ttoi(strRequest.Mid(iFirst));
			}
			break;
		case 20:	//�������
			if(iLast > 0)
			{
				preqTXFunction->nFYear = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
			}else
			{
				preqTXFunction->nFYear = _ttoi(strRequest.Mid(iFirst));
			}
			break;
		case 22:	//��ҵ�㼶
			if(iLast > 0)
			{
				preqTXFunction->nIndustryLevel = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
			}else
			{
				preqTXFunction->nIndustryLevel = _ttoi(strRequest.Mid(iFirst));
			}
			break;
		default:	//���������
			if(iLast > 0)
			{
				preqTXFunction->iQuotationNo = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
			}else
			{
				preqTXFunction->iQuotationNo = _ttoi(strRequest.Mid(iFirst));
			}
			break;
		}
		break;
	case 5:		//StartDate��EndDate
		for(i = 0; i < 3; i++)
		{	//FuncID��SubItemID��Security
			iLast = strRequest.Find(_T(';'), iFirst);
			iFirst = iLast + 1;
		}
		iLast = strRequest.Find(_T(';'), iFirst);
		preqTXFunction->nStartDate = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
		iFirst = iLast + 1;
		iLast = strRequest.Find(_T(';'), iFirst);
		if(iLast > 0)
		{
			preqTXFunction->nEndDate = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
		}else
		{
			preqTXFunction->nEndDate = _ttoi(strRequest.Mid(iFirst));
		}
		break;
	case 6:		//Date��OffsetDays
		for(i = 0; i < 3; i++)
		{	//FuncID��SubItemID��Security
			iLast = strRequest.Find(_T(';'), iFirst);
			iFirst = iLast + 1;
		}
		iLast = strRequest.Find(_T(';'), iFirst);
		preqTXFunction->nDate = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
		iFirst = iLast + 1;
		iLast = strRequest.Find(_T(';'), iFirst);
		if(iLast > 0)
		{
			preqTXFunction->nOffsetDays = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
		}else
		{
			preqTXFunction->nOffsetDays = _ttoi(strRequest.Mid(iFirst));
		}
		break;
	case 7:		//StartDate��EndDate����������
	case 8:		//StartDate��EndDate���۸�����
	case 21:	//StartDate��EndDate���������
		for(i = 0; i < 3; i++)
		{	//FuncID��SubItemID��Security
			iLast = strRequest.Find(_T(';'), iFirst);
			iFirst = iLast + 1;
		}
		iLast = strRequest.Find(_T(';'), iFirst);
		preqTXFunction->nStartDate = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
		iFirst = iLast + 1;
		iLast = strRequest.Find(_T(';'), iFirst);
		preqTXFunction->nEndDate = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
		iFirst = iLast + 1;
		switch(iItemFlg)
		{
		case 7:		//��������
			preqTXFunction->iValueType = _ttoi(strRequest.Mid(iFirst));
			break;
		case 8:		//�۸�����
			preqTXFunction->iPriceType = _ttoi(strRequest.Mid(iFirst));
			break;
		case 21:	//�������
			preqTXFunction->nFYear = _ttoi(strRequest.Mid(iFirst));
			break;
		}
		break;
	case 9:		//Date��StartDate��EndDate����Ȩ����
		for(i = 0; i < 3; i++)
		{	//FuncID��SubItemID��Security
			iLast = strRequest.Find(_T(';'), iFirst);
			iFirst = iLast + 1;
		}
		iLast = strRequest.Find(_T(';'), iFirst);
		preqTXFunction->nDate = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
		iFirst = iLast + 1;
		iLast = strRequest.Find(_T(';'), iFirst);
		preqTXFunction->nStartDate = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
		iFirst = iLast + 1;
		iLast = strRequest.Find(_T(';'), iFirst);
		preqTXFunction->nEndDate = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
		iFirst = iLast + 1;
		preqTXFunction->iIRType = _ttoi(strRequest.Mid(iFirst));
		break;
	case 10:		//StartDate��EndDate���������͡���������
		for(i = 0; i < 3; i++)
		{	//FuncID��SubItemID��Security
			iLast = strRequest.Find(_T(';'), iFirst);
			iFirst = iLast + 1;
		}
		iLast = strRequest.Find(_T(';'), iFirst);
		preqTXFunction->nStartDate = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
		iFirst = iLast + 1;
		iLast = strRequest.Find(_T(';'), iFirst);
		preqTXFunction->nEndDate = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
		iFirst = iLast + 1;
		iLast = strRequest.Find(_T(';'), iFirst);
		preqTXFunction->iValueType = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
		iFirst = iLast + 1;
		preqTXFunction->dFindValue = atof(strRequest.Mid(iFirst));
		break;
	case 11:	//StartDate��EndDate���۸����͡���Ȩ���͡��۸�
		for(i = 0; i < 3; i++)
		{	//FuncID��SubItemID��Security
			iLast = strRequest.Find(_T(';'), iFirst);
			iFirst = iLast + 1;
		}
		iLast = strRequest.Find(_T(';'), iFirst);
		preqTXFunction->nStartDate = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
		iFirst = iLast + 1;
		iLast = strRequest.Find(_T(';'), iFirst);
		preqTXFunction->nEndDate = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
		iFirst = iLast + 1;
		iLast = strRequest.Find(_T(';'), iFirst);
		preqTXFunction->iPriceType = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
		iFirst = iLast + 1;
		iLast = strRequest.Find(_T(';'), iFirst);
		preqTXFunction->iIRType = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
		iFirst = iLast + 1;
		preqTXFunction->dPrice = atof(strRequest.Mid(iFirst));
		break;
	case 12:	//������ȡ��Ƽ�����
		for(i = 0; i < 3; i++)
		{	//FuncID��SubItemID��Security
			iLast = strRequest.Find(_T(';'), iFirst);
			iFirst = iLast + 1;
		}
		iLast = strRequest.Find(_T(';'), iFirst);
		preqTXFunction->nFYear = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
		iFirst = iLast + 1;
		preqTXFunction->nFQuarter = _ttoi(strRequest.Mid(iFirst));
		break;
	case 13:	//������ȡ��Ƽ����ȡ��ϲ�/ĸ��˾������ǰ/��
		for(i = 0; i < 3; i++)
		{	//FuncID��SubItemID��Security
			iLast = strRequest.Find(_T(';'), iFirst);
			iFirst = iLast + 1;
		}
		iLast = strRequest.Find(_T(';'), iFirst);
		preqTXFunction->nFYear = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
		iFirst = iLast + 1;
		iLast = strRequest.Find(_T(';'), iFirst);
		preqTXFunction->nFQuarter = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
		iFirst = iLast + 1;
		iLast = strRequest.Find(_T(';'), iFirst);
		preqTXFunction->iReportType = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
		iFirst = iLast + 1;
		preqTXFunction->iAccountingPolicyType = _ttoi(strRequest.Mid(iFirst));
		break;
	case 14:	//FuncID��SubItemID��IndicatorID��������ȡ��Ƽ����ȡ���Ŀ���
		iLast = strRequest.Find(_T(';'), iFirst);
		preqTXFunction->iFuncID = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
		iFirst = iLast + 1;
		iLast = strRequest.Find(_T(';'), iFirst);
		preqTXFunction->iSubItemID = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
		iterIndicator = mapTXEEIndicator.find(preqTXFunction->iFuncID * 100 + preqTXFunction->iSubItemID);
		if(iterIndicator == mapTXEEIndicator.end())
		{
			preqTXFunction->nIndicatorID = 0;
		}else
		{
			preqTXFunction->nIndicatorID = iterIndicator->second.nIndicatorID;
		}
		for(i = 0; i < 2; i++)
		{	//Security���������
			iFirst = iLast + 1;
			iLast = strRequest.Find(_T(';'), iFirst);
		}
		preqTXFunction->nFYear = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
		iFirst = iLast + 1;
		iLast = strRequest.Find(_T(';'), iFirst);
		preqTXFunction->nFQuarter = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
		iFirst = iLast + 1;
		preqTXFunction->iSubjectID = _ttoi(strRequest.Mid(iFirst));
		break;
	case 15:	//Security��StartDate��EndDate
		for(i = 0; i < 2; i++)
		{	//FuncID��SubItemID
			iLast = strRequest.Find(_T(';'), iFirst);
			iFirst = iLast + 1;
		}
		iLast = strRequest.Find(_T(';'), iFirst);
		preqTXFunction->nSecurityID = _ttol(strRequest.Mid(iFirst, iLast - iFirst));
		iFirst = iLast + 1;
		iLast = strRequest.Find(_T(';'), iFirst);
		preqTXFunction->nStartDate = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
		iFirst = iLast + 1;
		preqTXFunction->nEndDate = _ttoi(strRequest.Mid(iFirst));
		break;
	case 16:	//Security��Date
		for(i = 0; i < 2; i++)
		{	//FuncID��SubItemID
			iLast = strRequest.Find(_T(';'), iFirst);
			iFirst = iLast + 1;
		}
		iLast = strRequest.Find(_T(';'), iFirst);
		preqTXFunction->nSecurityID = _ttol(strRequest.Mid(iFirst, iLast - iFirst));
		iFirst = iLast + 1;
		preqTXFunction->nDate = _ttoi(strRequest.Mid(iFirst));
		break;
	case 17:	//Date��Price
	case 18:	//Date��iHolderNo
		for(i = 0; i < 3; i++)
		{	//FuncID��SubItemID��Security
			iLast = strRequest.Find(_T(';'), iFirst);
			iFirst = iLast + 1;
		}
		iLast = strRequest.Find(_T(';'), iFirst);
		preqTXFunction->nDate = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
		iFirst = iLast + 1;
		iLast = strRequest.Find(_T(';'), iFirst);
		if(iItemFlg == 17)
		{	//Date��Price
			if(iLast > 0)
			{
				preqTXFunction->dPrice = atof(strRequest.Mid(iFirst, iLast - iFirst));
			}else
			{
				preqTXFunction->dPrice = atof(strRequest.Mid(iFirst));
			}
		}else
		{	//Date��iHolderNo
			if(iLast > 0)
			{
				preqTXFunction->iHolderNo = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
			}else
			{
				preqTXFunction->iHolderNo = _ttoi(strRequest.Mid(iFirst));
			}
		}
		break;
	case 19:	//Security��Date��iHolderNo
		for(i = 0; i < 2; i++)
		{	//FuncID��SubItemID
			iLast = strRequest.Find(_T(';'), iFirst);
			iFirst = iLast + 1;
		}
		iLast = strRequest.Find(_T(';'), iFirst);
		preqTXFunction->nSecurityID = _ttol(strRequest.Mid(iFirst, iLast - iFirst));
		iFirst = iLast + 1;
		iLast = strRequest.Find(_T(';'), iFirst);
		preqTXFunction->nDate = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
		iFirst = iLast + 1;
		preqTXFunction->iHolderNo = _ttoi(strRequest.Mid(iFirst));
		break;
	case 24:	//FuncID��security�������С�����Ƽ�
		iLast = strRequest.Find(_T(";"),iFirst);
		preqTXFunction->iFuncID = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
        iFirst = iLast + 1;
		iLast = strRequest.Find(_T(";"),iFirst);
		preqTXFunction->iSubItemID = _ttol(strRequest.Mid(iFirst,iLast - iFirst));
		iFirst = iLast + 1;
		iLast = strRequest.Find (_T(";"),iFirst);
		preqTXFunction->nSecurityID = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
		iFirst = iLast + 1;
		preqTXFunction->nFYear = _ttoi(strRequest.Mid( iFirst))/10000;
		preqTXFunction->nFQuarter = _ttoi(strRequest.Mid( iFirst))%10000;
		break;
	case 25:
		for(i = 0; i < 3; i++)
		{	//FuncID��SubItemID��Security
			iLast = strRequest.Find(_T(';'), iFirst);
			iFirst = iLast + 1;
		}
		iLast = strRequest.Find(_T(';'), iFirst);
		preqTXFunction->nFYear = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
		iFirst = iLast + 1;
		iLast = strRequest.Find(_T(';'), iFirst);
		preqTXFunction->nFQuarter = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
		iFirst = iLast + 1;
		iLast = strRequest.Find(_T(';'), iFirst);
		preqTXFunction->iReportType = _ttoi(strRequest.Mid(iFirst, iLast - iFirst));
		break;
	case 26://securityId startdate enddate ����Ƶ��
		{
			for(i = 0; i < 3; i++)
			{	//FuncID��SubItemID��Security
				iLast = strRequest.Find(_T(';'), iFirst);
				iFirst = iLast + 1;
			}
			iLast = strRequest.Find(_T(';'), iFirst);
			preqTXFunction->nStartDate = _ttoi(strRequest.Mid(iFirst,iLast - iFirst));
			iFirst = iLast + 1;
			iLast = strRequest.Find(_T(';'), iFirst);
			preqTXFunction->nEndDate = _ttoi(strRequest.Mid(iFirst,iLast - iFirst));
			iFirst = iLast + 1;
			//iLast = strRequest.Find(_T(';'), iFirst);
			preqTXFunction->iSYPL= _ttoi(strRequest.Mid(iFirst));
		}
		break;
	case 27: //FuncId ��date
		{
			iLast = strRequest.Find(_T(';'), iFirst);
			preqTXFunction->iFuncID = _ttoi(strRequest.Mid(iFirst,iLast - iFirst));
			iFirst = iLast + 1;
			iLast = strRequest.Find(_T(';'), iFirst);
			preqTXFunction->nDate = _ttoi(strRequest.Mid(iFirst));
		}
		break;
	case 39: //Ӧ����Ϣ��־  2012-07-23
		{
			for (i = 0;i<4;i++)
			{
				iLast = strRequest.Find(_T(";"),iFirst);
				iFirst = iLast + 1;
			}
			preqTXFunction->iYJLX = _ttoi(strRequest.Mid(iFirst));
		}
		break;
	default:
		break;
	}
}


DOUBLE TxIndicator::GetDatesIndicator_Value(INT nSecurity, INT nFirstDate, INT nSecondDate, vector_IndDates *pvIndDates)
{
	iter_IndDates iterIndDates;

	for(iterIndDates = pvIndDates->begin(); iterIndDates != pvIndDates->end(); iterIndDates++)
	{
		if(iterIndDates->nSecurity == nSecurity && iterIndDates->nFirstDate == nFirstDate && iterIndDates->nSecondDate == nSecondDate)
		{
			return iterIndDates->dValue;
		}
	}

	return 0;
}

INT TxIndicator::GetDatesIndicator_Row(INT nSecurity, INT nFirstDate, INT nSecondDate, vector_IndDates *pvIndDates)
{
	iter_IndDates iterIndDates;

	for(iterIndDates = pvIndDates->begin(); iterIndDates != pvIndDates->end(); iterIndDates++)
	{
		if(iterIndDates->nSecurity == nSecurity && iterIndDates->nFirstDate == nFirstDate && iterIndDates->nSecondDate == nSecondDate)
		{
			return iterIndDates->nRow;
		}
	}

	return -1;
}

void TxIndicator::SetDatesIndicator(vector_IndDates *pvIndDates, int nTblIdx)
{
	iter_IndDates iterIndDates;
	INT nEndDateIdx, nStartDate, nEndDate;
	Tx::Business::TxBusiness business;
	Tx::Core::Table_Indicator subtable_Security, subtable_Period;
	Tx::Data::FundNetValueData *pFundNetValueData;
	DOUBLE dValue1 = 0, dValue2 = 0, dCoefficient = 1, dValue3 = 0, dValue4 = 0;
	UINT i, array_Col[4] = {0, 1, 2, 3};

	for(iterIndDates = pvIndDates->begin(); iterIndDates != pvIndDates->end(); iterIndDates++)
	{
		switch(nTblIdx)
		{
		case 0:		//����ֵ����
			dValue1 = dValue2 = 0;
			business.GetSecurityNow((LONG)(iterIndDates->nSecurity));
			if(iterIndDates->nFirstDate == 0)
			{
				nStartDate = business.m_pSecurity->GetFundEstablishDate();
			}else
			{
				nStartDate = iterIndDates->nFirstDate;
			}
			pFundNetValueData = business.m_pSecurity->GetFundNetValueDataByDate(nStartDate);
			if(pFundNetValueData != NULL)
			{
				dValue1 = pFundNetValueData->fNetvalue;
			}
			nEndDate = iterIndDates->nSecondDate;
			pFundNetValueData = business.m_pSecurity->GetFundNetValueDataByDate(nEndDate);
			if(pFundNetValueData == NULL)
			{
				nEndDateIdx = business.m_pSecurity->GetFundNetValueDataCount() - 1;
				if(nEndDateIdx >= 0)
				{
					pFundNetValueData = business.m_pSecurity->GetFundNetValueDataByIndex(nEndDateIdx);
				}
			}
			if(pFundNetValueData != NULL)
			{
				dValue2 = pFundNetValueData->fNetvalue;
			}

			if(dValue1 > 0 && dValue2 > 0)
			{
				dCoefficient = 1;
				//------------wangzhy-----------------20080417----------------
				if ( tableIndDates[nTblIdx].GetRowCount() == 0 )
					business.m_pLogicalBusiness->GetData( tableIndDates[nTblIdx],true );
				//------------------------------------------------------------

				if(tableIndDates[nTblIdx].GetRowCount() > 0)
				{
					//��ѯSecurity(����ʵ��ID)
					subtable_Security.Clear();
					subtable_Security.CopyColumnInfoFrom(tableIndDates[nTblIdx]);
					tableIndDates[nTblIdx].Between(subtable_Security, array_Col, 4, 0, iterIndDates->nSecurity, iterIndDates->nSecurity, true, true);
					//��ѯDate(��Ȩ��)
					subtable_Period.Clear();
					subtable_Period.CopyColumnInfoFrom(subtable_Security);
					subtable_Security.Between(subtable_Period, array_Col, 4, 1, nStartDate, nEndDate, true, true);
					//�������ֵ����ϵ��
					for(i = 0; i < subtable_Period.GetRowCount(); i++)
					{
						subtable_Period.GetCell(2, i, dValue3);	//����ǰ����ֵ
						subtable_Period.GetCell(3, i, dValue4);	//���������ֵ
						if(dValue3 > 0 && dValue4 > 0)
						{
							dCoefficient = dCoefficient * (dValue3 / dValue4);	//����ǰ����ֵ>���������ֵ
						}
					}
				}
				iterIndDates->dValue = (dValue2 * dCoefficient / dValue1 - 1);	//���ǻ���ֵ����ϵ��
			}

			break;
		default:
			break;
		}
	}
}

DOUBLE TxIndicator::GetDateIndicator_Value(INT nSecurity, INT nDate, vector_IndDate *pvIndDate)
{
	iter_IndDate iterIndDate;

	for(iterIndDate = pvIndDate->begin(); iterIndDate != pvIndDate->end(); iterIndDate++)
	{
		if(iterIndDate->nSecurity == nSecurity && iterIndDate->nDate == nDate)
		{
			return iterIndDate->dValue;
		}
	}

	return 0;
}

INT TxIndicator::GetDateIndicator_Row(INT nSecurity, INT nDate, vector_IndDate *pvIndDate)
{
	iter_IndDate iterIndDate;

	for(iterIndDate = pvIndDate->begin(); iterIndDate != pvIndDate->end(); iterIndDate++)
	{
		if(iterIndDate->nSecurity == nSecurity && iterIndDate->nDate == nDate)
		{
			return iterIndDate->nRow;
		}
	}

	return -1;
}



void TxIndicator::SetDateIndicator(vector_IndDate *pvIndDate, int nTblIdx)
{
	iter_IndDate iterIndDate;
	Tx::Business::TxBusiness business;
	Tx::Core::Table_Indicator subtable_Security, subtable_Date;
	int nStartDate, nEndDate, iSecurity1ID, nRow, nDate;
	DOUBLE dSumValue, dValue;
	vector <UINT> vRow;
	UINT i, array_Col[4] = {0, 1, 2, 3};

	if(tableIndDate[nTblIdx].GetRowCount() == 0)
	{
		//----------------------�����Ϊ�գ���ȥ����----------------
		business.m_pLogicalBusiness->GetData(tableIndDate[nTblIdx], true);
		//-----------------wangzhy-20080417--------------------------
		//return;
	}
#ifdef _DEBUG
	CString strTableIndDate = tableIndDate[nTblIdx].TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTableIndDate);
#endif

	for(iterIndDate = pvIndDate->begin(); iterIndDate != pvIndDate->end(); iterIndDate++)
	{
		business.GetSecurityNow((LONG)(iterIndDate->nSecurity));
		
		subtable_Security.Clear();
		subtable_Security.CopyColumnInfoFrom(tableIndDate[nTblIdx]);
		switch(nTblIdx)
		{
		case 0:		//��Ʊ����
			//ȯid
			iSecurity1ID = (INT)(business.m_pSecurity->GetSecurity1Id());
			tableIndDate[nTblIdx].Between(subtable_Security, array_Col, 4, 1, iSecurity1ID, iSecurity1ID, true, true);
			break;
		case 1:		//Ȩ֤����ע��
			//����ʵ��id
			tableIndDate[nTblIdx].Between(subtable_Security, array_Col, 4, 2, iterIndDate->nSecurity, iterIndDate->nSecurity, true, true);
			break;
		case 2:		//��תծת�ɼ۸�
			//ȯid
			iSecurity1ID = (INT)(business.m_pSecurity->GetSecurity1Id());
			tableIndDate[nTblIdx].Between(subtable_Security, array_Col, 3, 0, iSecurity1ID, iSecurity1ID, true, true);
			break;
		case 3:		//��תծ����
			//ȯid
			iSecurity1ID = (INT)(business.m_pSecurity->GetSecurity1Id());
			tableIndDate[nTblIdx].Between(subtable_Security, array_Col, 4, 0, iSecurity1ID, iSecurity1ID, true, true);
			break;
		case 4:		//��תծת������
			//ȯid
			iSecurity1ID = (INT)(business.m_pSecurity->GetSecurity1Id());
			vRow.clear();
			tableIndDate[nTblIdx].Find(0, iSecurity1ID, vRow);
			if(vRow.empty())
			{
				break;
			}
			//��ֹ����(ȡ��ӽ�ָ�����ڵĽ�ֹ����)
			for(i = 0; i < vRow.size(); i++)
			{
				tableIndDate[nTblIdx].GetCell(1, vRow[i], nDate);
				if(nDate > iterIndDate->nDate)
				{
					break;
				}
			}
			if(i > 0)
			{
				nRow = vRow[i - 1];
				iterIndDate->nRow = nRow;
			}
			break;
		case 5:		//ծȯ������Ϣ
			//ȯid
			iSecurity1ID = (INT)(business.m_pSecurity->GetSecurity1Id());
			vRow.clear();
			tableIndDate[nTblIdx].Find(0, iSecurity1ID, vRow);
			if(vRow.empty())
			{
				break;
			}
			//��������(ȡ���һ��)
			nRow = vRow[vRow.size() - 1];
			iterIndDate->nRow = nRow;
			break;
		default:
			break;
		}

		if(subtable_Security.GetRowCount() == 0)
		{
			continue;
		}

		switch(nTblIdx)
		{
		case 0:		//��Ʊ����
			//��������
			subtable_Security.Sort(2);
			if(iterIndDate->nDate == 0)
			{
				nRow = 0;
			}else
			{
				for(i = 0; i < subtable_Security.GetRowCount(); i++)
				{
					subtable_Security.GetCell(2, i, nDate);
					if(nDate >= iterIndDate->nDate)
					{
						break;
					}
				}
				nRow = i;
				if(nRow == subtable_Security.GetRowCount())
				{
					nRow --;
				}
			}
			//���м۸�
			if(nRow >= 0)
			{
				subtable_Security.GetCell(3, (UINT)nRow, dValue);
				if(dValue > 0)
				{
					iterIndDate->dValue = dValue;
				}
			}
			break;
		case 1:		//Ȩ֤����ע��
			//Ȩ֤�Ĵ����ע������
			nStartDate = business.m_pSecurity->GetIPOListedDate();
			if(nStartDate < 0)
			{
				nStartDate = 0;
			}
			nEndDate = iterIndDate->nDate;
			subtable_Date.Clear();
			subtable_Date.CopyColumnInfoFrom(subtable_Security);
			subtable_Security.Between(subtable_Date, array_Col, 4, 1, nStartDate, nEndDate, true, true);
			//��ֹ���ڵ�Ȩ֤ʣ�ഴ�����
			dSumValue = 0;
			for(i = 0; i < subtable_Date.GetRowCount(); i++)
			{
				subtable_Date.GetCell(3, i, dValue);
				if(dValue != Tx::Core::Con_doubleInvalid)
				{
					dSumValue += dValue;
				}
			}
			iterIndDate->dValue = dSumValue;
			break;
		case 2:		//��תծת�ɼ۸�
			//��Ч����
			subtable_Security.Sort(1);
			nRow = subtable_Security.ApproximativeFind(1, iterIndDate->nDate);
			//ת�ɼ۸�
			if(nRow >= 0)
			{
				subtable_Security.GetCell(2, (UINT)nRow, dValue);
				if(dValue > 0)
				{
					iterIndDate->dValue = dValue;
				}
			}
			break;
		case 3:		//��תծ����
			//��ʼ����(ȡ��ӽ�ָ�����ڵ���ʼ����)
			subtable_Security.Sort(1);
			nRow = subtable_Security.ApproximativeFind(1, iterIndDate->nDate);
			if(nRow >= 0)
			{
				subtable_Security.GetCell(1, (UINT)nRow, nDate);
				subtable_Date.Clear();
				subtable_Date.CopyColumnInfoFrom(subtable_Security);
				subtable_Security.Between(subtable_Date, array_Col, 4, 1, nDate, nDate, true, true);
				if(subtable_Date.GetRowCount() > 0)
				{
					//��������(ȡ��ӽ�ָ�����ڵĹ�������)
					subtable_Date.Sort(2);
					nRow = subtable_Date.ApproximativeFind(2, iterIndDate->nDate);
					if(nRow >= 0)
					{
						subtable_Date.GetCell(3, nRow, dValue);
						if(dValue > 0)
						{
							iterIndDate->dValue = dValue;
						}
					}
				}
			}
			break;
		default:
			break;
		}
	}
}

INT TxIndicator::GetShareHolderIndicatorRow(INT nSecurity, INT nDate, INT iHolderNo, vector_IndShareHolder *pvIndShareHolder)
{
	iter_IndShareHolder iterIndHolder;

	for(iterIndHolder = pvIndShareHolder->begin(); iterIndHolder != pvIndShareHolder->end(); iterIndHolder++)
	{
		if(iterIndHolder->nSecurity == nSecurity && iterIndHolder->nDate == nDate && iterIndHolder->iHolerNo == iHolderNo)
		{
			return iterIndHolder->nRow;
		}
	}

	return -1;
}

void TxIndicator::SetShareHolderIndicator(vector_IndShareHolder *pvIndShareHolder, int nTblIdx)
{
	set <INT> setDate;
	set <INT> ::iterator iterDate;
	vector <UINT> vRow_Security, vRow_Date;
	iter_IndShareHolder iterIndShareHolder;
	Tx::Business::TxBusiness business;
	Tx::Core::Table_Indicator subtable_Security, subtable_Date;
	int nID, nDate, iHolderNo, nReportDate;
	BYTE cHolderNo;
	UINT i, array_Col[3] = {0, 1, 2};

	if(tableShareHolder[nTblIdx].GetRowCount() == 0)
	{
		//�����Ϊ��,��������ļ�����,����Ļ�ȥʮ��ɱ��Ĺ��ܲ�����----20080512-wangzhy
		Tx::Business::TxBusiness business;
		business.m_pLogicalBusiness->GetData(tableShareHolder[nTblIdx], true);
		//return;
	}

	for(iterIndShareHolder = pvIndShareHolder->begin(); iterIndShareHolder != pvIndShareHolder->end(); iterIndShareHolder++)
	{
		if(business.GetSecurityNow((LONG)(iterIndShareHolder->nSecurity)))
		{
			switch(nTblIdx)
			{
			case 0:		//ʮ��ɶ�
			case 1:		//ʮ����ͨ�ɶ�
				nID = (INT)(business.m_pSecurity->GetInstitutionId());		//����ID
				break;
			default:	//ʮ���������ˡ�ʮ���תծ������
				nID = (INT)(business.m_pSecurity->GetSecurity1Id());		//ȯID
				break;
			}
			if(nID <= 0)
			{
				continue;
			}
			//ȡ��ָ������ID/ȯID��������
			vRow_Security.clear();
			tableShareHolder[nTblIdx].Find(0, nID, vRow_Security);
			//ȡ��ָ������ID/ȯID���ӱ�
			subtable_Security.Clear();
			subtable_Security.CopyColumnInfoFrom(tableShareHolder[nTblIdx], array_Col, 3);
			tableShareHolder[nTblIdx].Between(subtable_Security, array_Col, 3, 0, nID, nID, true, true);
			//ȡ��ָ������ID/ȯID�����н�ֹ����
			setDate.clear();
			for(i = 0; i < subtable_Security.GetRowCount(); i++)
			{
				subtable_Security.GetCell(1, i, nDate);
				setDate.insert(nDate);
			}
			//ȡ��ָ������ID/ȯID����ӽ�ָ�����ڵĽ�ֹ����
			nReportDate = 0;
			for(iterDate = setDate.begin(); iterDate != setDate.end(); iterDate++)
			{
				if((*iterDate) <= iterIndShareHolder->nDate)
				{
					nReportDate = (*iterDate);
				}else
				{
					break;
				}
			}
			//ȡ�ý�ֹ����֮��
			if(nReportDate > 0)
			{
				if(iterIndShareHolder->iHolerNo == 0)
				{	//ȡ�ý�ֹ�����ӱ�
					subtable_Date.Clear();
					subtable_Date.CopyColumnInfoFrom(tableShareHolder[nTblIdx], array_Col, 3);
					subtable_Security.Between(subtable_Date, array_Col, 3, 1, nReportDate, nReportDate, true, true);
					iterIndShareHolder->nRow = (INT)(subtable_Date.GetRowCount());
					continue;
				}
				//ȡ�ý�ֹ���ڵ�������
				vRow_Date.clear();
				for(i = 0; i < vRow_Security.size(); i++)
				{
					tableShareHolder[nTblIdx].GetCell(1, vRow_Security[i], nDate);
					if(nDate == nReportDate)
					{
						vRow_Date.push_back(vRow_Security[i]);
					}
				}
				if(vRow_Date.size() == 0)
				{
					continue;
				}
				for(i = 0; i < vRow_Date.size(); i++)
				{
					if(nTblIdx == 2)
					{	//ʮ����������
						tableShareHolder[nTblIdx].GetCell(2, vRow_Date[i], cHolderNo);
						if((INT)cHolderNo == iterIndShareHolder->iHolerNo)
						{
							iterIndShareHolder->nRow = (INT)(vRow_Date[i]);
							break;
						}
					}else
					{	//ʮ��ɶ���ʮ����ͨ�ɶ���ʮ���תծ������
						tableShareHolder[nTblIdx].GetCell(2, vRow_Date[i], iHolderNo);
						if(iHolderNo == iterIndShareHolder->iHolerNo)
						{
							iterIndShareHolder->nRow = (INT)(vRow_Date[i]);
							break;
						}
					}
				}
			}
		}
	}
}



//����ʵʩָ��Ĺ���-------------------------wangzhy---------------------
UINT TxIndicator::RealTimeIndCalc_Proc(LPVOID lpParam)
{
	Tx::Business::TxBusiness *pBusiness;
	Tx::Business::TxBusiness business;
	Tx::Business::TxBond bond;
	Tx::Business::TxWarrant warrant;
	Tx::Data::TxShareData *pTxShareData;
	Tx::Data::TxFundShareData *pTxFundShareData;
	Tx::Data::BondNewInfo *pBondNewInfo;
	Tx::Data::BondNotChangeAmount *pCBondAmount;
	Tx::Data::TradeQuotation *pTradeQuotation;
	Tx::Data::TradeQuotationData *pTradeQuotationData;
	ReqMapIter iterReqRealTime;
	CTime tmLast, tmCurrent;
	CTimeSpan tsRefresh;
	CString strReq, strResult, strValue;
	DOUBLE dValue, dValue1, dValue2, dValue3;
	INT nSecurityID, nDataCount, nDate, nStartDate, nEndDate, nTime, nTradeDays;
	LONG nNewReqCnt;
	INT_Map mapIntInt;
	INT_Map_Iter iterIntInt;
	//RealTimeIndCalc_Param *lpParam_RealTimeIndCalc = (RealTimeIndCalc_Param *)lpParam;
	RealTimeIndCalc_Param *lpParam_RealTimeIndCalc = &param_RealTimeIndCalc;

	CSingleLock singleLock(lpParam_RealTimeIndCalc->pCSRealTime);

	tmLast = 0;
	pBusiness = new Tx::Business::TxBusiness();
	
	//
	CTime tYtmLast;
	CTime tBYYtm[5]; //����
	CTime tSPYtm[5]; //����

	while((*(lpParam_RealTimeIndCalc->lpbThreadRun)) == TRUE)
	{
		tmCurrent = CTime::GetCurrentTime();
		nTime = tmCurrent.GetHour() * 100 + tmCurrent.GetMinute();
		if(!(nTime <= 1500))
		{
			singleLock.Lock();	//�̻߳���
			if(lpParam_RealTimeIndCalc->pmapReqRealTime->size() > 0)
			{
				lpParam_RealTimeIndCalc->pmapReqRealTime->clear();
			}
			singleLock.Unlock();
			::Sleep(50);
			continue;
		}

		if(tmLast > 0)
		{
			tsRefresh = tmCurrent - tmLast;
		}

		if((tmLast == 0 && lpParam_RealTimeIndCalc->pmapReqRealTime->size() > 0) || 
			(tmLast > 0 && tsRefresh.GetTotalSeconds() > 10))
		{
			singleLock.Lock();	//�̻߳���
			nNewReqCnt = 0;
			if(lpParam_RealTimeIndCalc->pmapReqRealTime->size() > 0)
			{
				TRACE(_T("\n--ʵʱָ���VBA���������ʼʱ�䣺%4d-%02d-%02d %02d:%02d:%02d--\n"), 
					tmCurrent.GetYear(), tmCurrent.GetMonth(), tmCurrent.GetDay(), 
					tmCurrent.GetHour(), tmCurrent.GetMinute(), tmCurrent.GetSecond());
			}

			//Tx::Log::CLogRecorder::GetInstance()->WriteToLog(_T("Excel����ʵʱ�������"));
			for(iterReqRealTime = lpParam_RealTimeIndCalc->pmapReqRealTime->begin(); iterReqRealTime != lpParam_RealTimeIndCalc->pmapReqRealTime->end(); iterReqRealTime++)
			{
				if(iterReqRealTime->second.nCalculateTime == 0)
				{
					nNewReqCnt ++;
				}

				strReq = iterReqRealTime->first;
				FindItemOfReq(&reqRealTime, strReq, 2);			//FuncID��SubItemID��SecurityID
				
				if(!pBusiness->GetSecurityNow(reqRealTime.nSecurityID))
					continue;
				strResult = _T("");
				
				//2008-06-12 by zhaohj
				//֧�ַ���������
				Tx::Core::VariantData vdReq;
				
				switch(reqRealTime.iFuncID)
				{
				case 14:	//��������
					vdReq.data_type = dtype_int4;
					FindItemOfReq(&reqRealTime, strReq, 5);	//StartDate��EndDate
					nTradeDays = GetTradeDays(pBusiness, reqRealTime.nStartDate, reqRealTime.nEndDate);
					strResult.Format(_T("%d"), nTradeDays);
					break;
				case 15:	//���̼�
					vdReq.data_type = dtype_float;
					FindItemOfReq(&reqRealTime, strReq, 3);	//Date
					dValue = GetKLine(pBusiness, reqRealTime.nDate, 0,reqRealTime.nSecurityID);
					if(dValue <= 0)
					{
						break;
					}
					//bug:11640  2012-09-25
					//if(pBusiness->m_pSecurity->IsFundTradeInMarket()||(pBusiness->m_pSecurity->IsShanghai()&&pBusiness->m_pSecurity->IsStockB()))
					if(pBusiness->m_pSecurity->IsFundTradeInMarket() || pBusiness->m_pSecurity->IsFund_Bond() || pBusiness->m_pSecurity->IsFund_Intermix() || 
						pBusiness->m_pSecurity->IsFund_ETF() || pBusiness->m_pSecurity->IsFund_LOF() || pBusiness->m_pSecurity->IsFund_QDII() || 
						pBusiness->m_pSecurity->IsFund_Stock() || (pBusiness->m_pSecurity->IsStockB() && pBusiness->m_pSecurity->IsShanghai())||
						(pBusiness->m_pSecurity->IsBond_National() && pBusiness->m_pSecurity->IsShenzhen()) || pBusiness->m_pSecurity->IsBond_Change() ||
						pBusiness->m_pSecurity->IsWarrant() || pBusiness->m_pSecurity->IsRepurchase()
					  )
					{
						strResult.Format(_T("%.3f"), dValue);
						/*CString strlog;
						strlog.Format( _T("Excel���������̼ۡ�--cost 15 = %s"), strResult );
						Tx::Log::CLogRecorder::GetInstance()->WriteToLog(strlog);*/
					}else
					{
						strResult.Format(_T("%.2f"), dValue);
						/*		CString strlog;
						strlog.Format( _T("Excel���������̼ۡ�--cost 15 = %s"), strResult );
						Tx::Log::CLogRecorder::GetInstance()->WriteToLog(strlog);*/
					}
					break;
				case 56:	//���̼�[��Ȩ]
					vdReq.data_type = dtype_float;
					FindItemOfReq(&reqRealTime, strReq, 9);	//Date��StartDate��EndDate����Ȩ����
					dValue1 = GetKLine(pBusiness, reqRealTime.nDate, 0,reqRealTime.nSecurityID);
					if(dValue1 <= 0)
					{
						break;
					}
					if(reqRealTime.iIRType == 0)
					{	//ǰ��Ȩ
						//dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nDate, reqTXFunction.nEndDate, true);
						dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqRealTime.nStartDate, reqRealTime.nEndDate, true);
					}else
					{	//��Ȩ
						if ( reqRealTime.nStartDate == 0 )
							reqTXFunction.nStartDate = pBusiness->m_pSecurity->GetTradeDateByIndex(0);	
						//dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nDate, false);
						dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqRealTime.nStartDate, reqRealTime.nEndDate, false);
					}
					//bug:11640  2012-09-25
					//if(pBusiness->m_pSecurity->IsFundTradeInMarket()||(pBusiness->m_pSecurity->IsShanghai()&&pBusiness->m_pSecurity->IsStockB()))
					if(pBusiness->m_pSecurity->IsFundTradeInMarket() || pBusiness->m_pSecurity->IsFund_Bond() || pBusiness->m_pSecurity->IsFund_Intermix() || 
						pBusiness->m_pSecurity->IsFund_ETF() || pBusiness->m_pSecurity->IsFund_LOF() || pBusiness->m_pSecurity->IsFund_QDII() || 
						pBusiness->m_pSecurity->IsFund_Stock() || (pBusiness->m_pSecurity->IsStockB() && pBusiness->m_pSecurity->IsShanghai())||
						(pBusiness->m_pSecurity->IsBond_National() && pBusiness->m_pSecurity->IsShenzhen()) || pBusiness->m_pSecurity->IsBond_Change() ||
						pBusiness->m_pSecurity->IsWarrant() || pBusiness->m_pSecurity->IsRepurchase())
					{
						strResult.Format(_T("%.3f"), dValue1 * dValue2);
					}else
					{
						strResult.Format(_T("%.2f"), dValue1 * dValue2);
					}
					break;
				case 16:	//ǰ�ռ�
					vdReq.data_type = dtype_float;
					FindItemOfReq(&reqRealTime, strReq, 3);	//Date
					dValue = GetKLine(pBusiness, reqRealTime.nDate, 1,reqRealTime.nSecurityID);
					if(dValue <= 0)
					{
						break;
					}
					//bug:11640  2012-09-25
					//if(pBusiness->m_pSecurity->IsFundTradeInMarket()||(pBusiness->m_pSecurity->IsShanghai()&&pBusiness->m_pSecurity->IsStockB()))
					if(pBusiness->m_pSecurity->IsFundTradeInMarket() || pBusiness->m_pSecurity->IsFund_Bond() || pBusiness->m_pSecurity->IsFund_Intermix() || 
						pBusiness->m_pSecurity->IsFund_ETF() || pBusiness->m_pSecurity->IsFund_LOF() || pBusiness->m_pSecurity->IsFund_QDII() || 
						pBusiness->m_pSecurity->IsFund_Stock() || (pBusiness->m_pSecurity->IsStockB() && pBusiness->m_pSecurity->IsShanghai())||
						(pBusiness->m_pSecurity->IsBond_National() && pBusiness->m_pSecurity->IsShenzhen()) || pBusiness->m_pSecurity->IsBond_Change() ||
						pBusiness->m_pSecurity->IsWarrant() || pBusiness->m_pSecurity->IsRepurchase())
					{
						strResult.Format(_T("%.3f"), dValue);
						/*	CString strlog;
						strlog.Format( _T("Excel������ǰ�ռۡ�--cost 16 = %s"), strResult );
						Tx::Log::CLogRecorder::GetInstance()->WriteToLog(strlog);*/
					}else
					{
						strResult.Format(_T("%.2f"), dValue);
						/*	CString strlog;
						strlog.Format( _T("Excel������ǰ�ռۡ�--cost 16 = %s"), strResult );
						Tx::Log::CLogRecorder::GetInstance()->WriteToLog(strlog);*/
					}
					break;
				case 57:	//ǰ�ռ�[��Ȩ]
					vdReq.data_type = dtype_float;
					FindItemOfReq(&reqRealTime, strReq, 9);	//Date��StartDate��EndDate����Ȩ����
					dValue1 = GetKLine(pBusiness, reqRealTime.nDate, 1,reqRealTime.nSecurityID);
					if(dValue1 <= 0)
					{
						break;
					}
					if(reqRealTime.iIRType == 0)
					{	//ǰ��Ȩ
						//dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nDate, reqTXFunction.nEndDate, true);
						dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqRealTime.nStartDate, reqRealTime.nEndDate, true);
					}else
					{	//��Ȩ
						if ( reqRealTime.nStartDate == 0 )
							reqRealTime.nStartDate = pBusiness->m_pSecurity->GetTradeDateByIndex(0);	
						//dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nDate, false);
						dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqRealTime.nStartDate, reqRealTime.nEndDate, false);
					}
					//bug:11640  2012-09-25
					//if(pBusiness->m_pSecurity->IsFundTradeInMarket()||(pBusiness->m_pSecurity->IsShanghai()&&pBusiness->m_pSecurity->IsStockB()))
					if(pBusiness->m_pSecurity->IsFundTradeInMarket() || pBusiness->m_pSecurity->IsFund_Bond() || pBusiness->m_pSecurity->IsFund_Intermix() || 
						pBusiness->m_pSecurity->IsFund_ETF() || pBusiness->m_pSecurity->IsFund_LOF() || pBusiness->m_pSecurity->IsFund_QDII() || 
						pBusiness->m_pSecurity->IsFund_Stock() || (pBusiness->m_pSecurity->IsStockB() && pBusiness->m_pSecurity->IsShanghai())||
						(pBusiness->m_pSecurity->IsBond_National() && pBusiness->m_pSecurity->IsShenzhen()) || pBusiness->m_pSecurity->IsBond_Change() ||
						pBusiness->m_pSecurity->IsWarrant() || pBusiness->m_pSecurity->IsRepurchase())
					{
						strResult.Format(_T("%.3f"), dValue1 * dValue2);
					}else
					{
						strResult.Format(_T("%.2f"), dValue1 * dValue2);
					}
					break;
				case 17:	//��߼�
					vdReq.data_type = dtype_float;
					FindItemOfReq(&reqRealTime, strReq, 3);		//Date
					dValue = GetKLine(pBusiness, reqRealTime.nDate, 2,reqRealTime.nSecurityID);
					if(dValue <= 0)
					{
						break;
					}
					//bug:11640  2012-09-25
					//if(pBusiness->m_pSecurity->IsFundTradeInMarket()||(pBusiness->m_pSecurity->IsShanghai()&&pBusiness->m_pSecurity->IsStockB()))
					if(pBusiness->m_pSecurity->IsFundTradeInMarket() || pBusiness->m_pSecurity->IsFund_Bond() || pBusiness->m_pSecurity->IsFund_Intermix() || 
						pBusiness->m_pSecurity->IsFund_ETF() || pBusiness->m_pSecurity->IsFund_LOF() || pBusiness->m_pSecurity->IsFund_QDII() || 
						pBusiness->m_pSecurity->IsFund_Stock() || (pBusiness->m_pSecurity->IsStockB() && pBusiness->m_pSecurity->IsShanghai())||
						(pBusiness->m_pSecurity->IsBond_National() && pBusiness->m_pSecurity->IsShenzhen()) || pBusiness->m_pSecurity->IsBond_Change() ||
						pBusiness->m_pSecurity->IsWarrant() || pBusiness->m_pSecurity->IsRepurchase())
					{
						strResult.Format(_T("%.3f"), dValue);
					}else
					{
						strResult.Format(_T("%.2f"), dValue);
					}
					break;
				case 58:	//��߼�[��Ȩ]
					vdReq.data_type = dtype_float;
					FindItemOfReq(&reqRealTime, strReq, 9);		//Date��StartDate��EndDate����Ȩ����
					dValue1 = GetKLine(pBusiness, reqRealTime.nDate, 2,reqRealTime.nSecurityID);
					if(dValue1 <= 0)
					{
						break;
					}
					if(reqRealTime.iIRType == 0)
					{	//ǰ��Ȩ
						//dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nDate, reqTXFunction.nEndDate, true);
						dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqRealTime.nStartDate, reqRealTime.nEndDate, true);
					}else
					{	//��Ȩ
						if ( reqRealTime.nStartDate == 0 )
							reqRealTime.nStartDate = pBusiness->m_pSecurity->GetTradeDateByIndex(0);	
						//dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nDate, false);
						dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqRealTime.nStartDate, reqRealTime.nEndDate, false);
					}
					//bug:11640  2012-09-25
					//if(pBusiness->m_pSecurity->IsFundTradeInMarket()||(pBusiness->m_pSecurity->IsShanghai()&&pBusiness->m_pSecurity->IsStockB()))
					if(pBusiness->m_pSecurity->IsFundTradeInMarket() || pBusiness->m_pSecurity->IsFund_Bond() || pBusiness->m_pSecurity->IsFund_Intermix() || 
						pBusiness->m_pSecurity->IsFund_ETF() || pBusiness->m_pSecurity->IsFund_LOF() || pBusiness->m_pSecurity->IsFund_QDII() || 
						pBusiness->m_pSecurity->IsFund_Stock() || (pBusiness->m_pSecurity->IsStockB() && pBusiness->m_pSecurity->IsShanghai())||
						(pBusiness->m_pSecurity->IsBond_National() && pBusiness->m_pSecurity->IsShenzhen()) || pBusiness->m_pSecurity->IsBond_Change() ||
						pBusiness->m_pSecurity->IsWarrant() || pBusiness->m_pSecurity->IsRepurchase())
					{
						strResult.Format(_T("%.3f"), dValue1 * dValue2);
					}else
					{
						strResult.Format(_T("%.2f"), dValue1 * dValue2);
					}
					break;
				case 18:	//��ͼ�
					vdReq.data_type = dtype_float;
					FindItemOfReq(&reqRealTime, strReq, 3);		//Date
					dValue = GetKLine(pBusiness, reqRealTime.nDate, 3,reqRealTime.nSecurityID);
					if(dValue <= 0)
					{
						break;
					}
					//bug:11640  2012-09-25
					//if(pBusiness->m_pSecurity->IsFundTradeInMarket()||(pBusiness->m_pSecurity->IsShanghai()&&pBusiness->m_pSecurity->IsStockB()))
					if(pBusiness->m_pSecurity->IsFundTradeInMarket() || pBusiness->m_pSecurity->IsFund_Bond() || pBusiness->m_pSecurity->IsFund_Intermix() || 
						pBusiness->m_pSecurity->IsFund_ETF() || pBusiness->m_pSecurity->IsFund_LOF() || pBusiness->m_pSecurity->IsFund_QDII() || 
						pBusiness->m_pSecurity->IsFund_Stock() || (pBusiness->m_pSecurity->IsStockB() && pBusiness->m_pSecurity->IsShanghai())||
						(pBusiness->m_pSecurity->IsBond_National() && pBusiness->m_pSecurity->IsShenzhen()) || pBusiness->m_pSecurity->IsBond_Change() ||
						pBusiness->m_pSecurity->IsWarrant() || pBusiness->m_pSecurity->IsRepurchase()
					  )
					{
						strResult.Format(_T("%.3f"), dValue);
					}else
					{
						strResult.Format(_T("%.2f"), dValue);
					}
					break;
				case 59:	//��ͼ�[��Ȩ]
					vdReq.data_type = dtype_float;
					FindItemOfReq(&reqRealTime, strReq, 9);		//Date��StartDate��EndDate����Ȩ����
					dValue1 = GetKLine(pBusiness, reqRealTime.nDate, 3,reqRealTime.nSecurityID);
					if(dValue1 <= 0)
					{
						break;
					}
					if(reqRealTime.iIRType == 0)
					{	//ǰ��Ȩ
						//dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nDate, reqTXFunction.nEndDate, true);
						dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqRealTime.nStartDate, reqRealTime.nEndDate, true);
					}else
					{	//��Ȩ
						if ( reqRealTime.nStartDate == 0 )
							reqRealTime.nStartDate = pBusiness->m_pSecurity->GetTradeDateByIndex(0);	
						//dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nDate, false);
						dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqRealTime.nStartDate, reqRealTime.nEndDate, false);
					}
					//bug:11640  2012-09-25
					//if(pBusiness->m_pSecurity->IsFundTradeInMarket()||(pBusiness->m_pSecurity->IsShanghai()&&pBusiness->m_pSecurity->IsStockB()))
					if(pBusiness->m_pSecurity->IsFundTradeInMarket() || pBusiness->m_pSecurity->IsFund_Bond() || pBusiness->m_pSecurity->IsFund_Intermix() || 
						pBusiness->m_pSecurity->IsFund_ETF() || pBusiness->m_pSecurity->IsFund_LOF() || pBusiness->m_pSecurity->IsFund_QDII() || 
						pBusiness->m_pSecurity->IsFund_Stock() || (pBusiness->m_pSecurity->IsStockB() && pBusiness->m_pSecurity->IsShanghai())||
						(pBusiness->m_pSecurity->IsBond_National() && pBusiness->m_pSecurity->IsShenzhen()) || pBusiness->m_pSecurity->IsBond_Change() ||
						pBusiness->m_pSecurity->IsWarrant() || pBusiness->m_pSecurity->IsRepurchase())
					{
						strResult.Format(_T("%.3f"), dValue1 * dValue2);
					}else
					{
						strResult.Format(_T("%.2f"), dValue1 * dValue2);
					}
					break;
				case 19:	//���̼�
					vdReq.data_type = dtype_float;
					FindItemOfReq(&reqRealTime, strReq, 3);		//Date
					dValue = GetKLine(pBusiness, reqRealTime.nDate, 4,reqRealTime.nSecurityID);
					if(dValue <= 0)
					{
						break;
					}
					//bug:11640    2012-09-25
					//if(pBusiness->m_pSecurity->IsFundTradeInMarket()||(pBusiness->m_pSecurity->IsShanghai()&&pBusiness->m_pSecurity->IsStockB()))
					if(pBusiness->m_pSecurity->IsFundTradeInMarket() || pBusiness->m_pSecurity->IsFund_Bond() || pBusiness->m_pSecurity->IsFund_Intermix() || 
						pBusiness->m_pSecurity->IsFund_ETF() || pBusiness->m_pSecurity->IsFund_LOF() || pBusiness->m_pSecurity->IsFund_QDII() || 
						pBusiness->m_pSecurity->IsFund_Stock() || (pBusiness->m_pSecurity->IsStockB() && pBusiness->m_pSecurity->IsShanghai())||
						(pBusiness->m_pSecurity->IsBond_National() && pBusiness->m_pSecurity->IsShenzhen()) || pBusiness->m_pSecurity->IsBond_Change() ||
						pBusiness->m_pSecurity->IsWarrant() || pBusiness->m_pSecurity->IsRepurchase()
					  )
					{
						strResult.Format(_T("%.3f"), dValue);
						/*		CString strlog;
						strlog.Format( _T("Excel���������̼ۡ�--cost 19 = %s"), strResult );
						Tx::Log::CLogRecorder::GetInstance()->WriteToLog(strlog);*/
					}else
					{
						strResult.Format(_T("%.2f"), dValue);
			/*			CString strlog;
						strlog.Format( _T("Excel���������̼ۡ�--cost 19 = %s"), strResult );
						Tx::Log::CLogRecorder::GetInstance()->WriteToLog(strlog);*/
					}
					break;
				case 60:	//���̼�[��Ȩ]
					vdReq.data_type = dtype_float;
					FindItemOfReq(&reqRealTime, strReq, 9);		//Date��StartDate��EndDate����Ȩ����
					dValue1 = GetKLine(pBusiness, reqRealTime.nDate, 4,reqRealTime.nSecurityID);
					if(dValue1 <= 0)
					{
						break;
					}
					if(reqRealTime.iIRType == 0)
					{	//ǰ��Ȩ
						//dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nDate, reqTXFunction.nEndDate, true);
						dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqRealTime.nStartDate, reqRealTime.nEndDate, true);
					}else
					{	//��Ȩ
						if ( reqRealTime.nStartDate == 0 )
							reqRealTime.nStartDate = pBusiness->m_pSecurity->GetTradeDateByIndex(0);	
						//dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nDate, false);
						dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqRealTime.nStartDate, reqRealTime.nEndDate, false);
					}
					//bug:11640  2012-09-25
					//if(pBusiness->m_pSecurity->IsFundTradeInMarket()||(pBusiness->m_pSecurity->IsShanghai()&&pBusiness->m_pSecurity->IsStockB()))
					if(pBusiness->m_pSecurity->IsFundTradeInMarket() || pBusiness->m_pSecurity->IsFund_Bond() || pBusiness->m_pSecurity->IsFund_Intermix() || 
						pBusiness->m_pSecurity->IsFund_ETF() || pBusiness->m_pSecurity->IsFund_LOF() || pBusiness->m_pSecurity->IsFund_QDII() || 
						pBusiness->m_pSecurity->IsFund_Stock() || (pBusiness->m_pSecurity->IsStockB() && pBusiness->m_pSecurity->IsShanghai())||
						(pBusiness->m_pSecurity->IsBond_National() && pBusiness->m_pSecurity->IsShenzhen()) || pBusiness->m_pSecurity->IsBond_Change() ||
						pBusiness->m_pSecurity->IsWarrant() || pBusiness->m_pSecurity->IsRepurchase())
					{
						strResult.Format(_T("%.3f"), dValue1 * dValue2);
					}else
					{
						strResult.Format(_T("%.2f"), dValue1 * dValue2);
					}
					break;
				case 20:	//�վ���
					vdReq.data_type = dtype_float;
					FindItemOfReq(&reqRealTime, strReq, 3);		//Date
					//dValue1 = GetKLine(pBusiness, reqRealTime.nDate, 5);
					//dValue2 = GetKLine(pBusiness, reqRealTime.nDate, 6);
					//if(dValue1 > 0 && dValue2 > 0)
					//{	//�ɽ����/�ɽ���
					//	dValue = dValue1 / dValue2;
					//}else
					//{	//ǰ�ռ�
					//	dValue = GetKLine(pBusiness, reqRealTime.nDate, 1);
					//}
					dValue = GetKLineExNew(pBusiness,reqRealTime.nDate,9,reqRealTime.nSecurityID);
					if(dValue < 0.1)
						dValue = GetKLine(pBusiness, reqRealTime.nDate, 1,reqRealTime.nSecurityID);
					if(dValue <= 0)
					{
						break;
					}
					if(pBusiness->m_pSecurity->IsFundTradeInMarket()||(pBusiness->m_pSecurity->IsShanghai()&&pBusiness->m_pSecurity->IsStockB()))
					{
						strResult.Format(_T("%.3f"), dValue);
						/*		CString strlog;
						strlog.Format( _T("Excel�������վ��ۡ�--cost 20 = %s"), strResult );
						Tx::Log::CLogRecorder::GetInstance()->WriteToLog(strlog);*/
					}else
					{
						strResult.Format(_T("%.2f"), dValue);
				/*		CString strlog;
						strlog.Format( _T("Excel�������վ��ۡ�--cost 20 = %s"), strResult );
						Tx::Log::CLogRecorder::GetInstance()->WriteToLog(strlog);*/
					}
					break;
				case 61:	//�վ���[��Ȩ]
					vdReq.data_type = dtype_float;
					FindItemOfReq(&reqRealTime, strReq, 9);		//Date��StartDate��EndDate����Ȩ����
					dValue1 = GetKLine(pBusiness, reqRealTime.nDate, 5,reqRealTime.nSecurityID);
					dValue2 = GetKLine(pBusiness, reqRealTime.nDate, 6,reqRealTime.nSecurityID);
					if(reqRealTime.iIRType == 0)
					{	//ǰ��Ȩ
						//dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nDate, reqTXFunction.nEndDate, true);
						dValue3 = pBusiness->m_pSecurity->GetExdividendScale(reqRealTime.nStartDate, reqRealTime.nEndDate, true);
					}else
					{	//��Ȩ
						if ( reqRealTime.nStartDate == 0 )
							reqRealTime.nStartDate = pBusiness->m_pSecurity->GetTradeDateByIndex(0);	
						//dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nDate, false);
						dValue3 = pBusiness->m_pSecurity->GetExdividendScale(reqRealTime.nStartDate, reqRealTime.nEndDate, false);
					}
					if(dValue1 > 0 && dValue2 > 0)
					{	//�ɽ����/�ɽ���
						dValue = dValue1 / dValue2;
					}else
					{	//ǰ�ռ�
						dValue = GetKLine(pBusiness, reqRealTime.nDate, 1,reqRealTime.nSecurityID);
					}
					if(dValue <= 0)
					{
						break;
					}
					if(pBusiness->m_pSecurity->IsFundTradeInMarket()||(pBusiness->m_pSecurity->IsShanghai()&&pBusiness->m_pSecurity->IsStockB()))
					{
						strResult.Format(_T("%.3f"), dValue * dValue3);
					}else
					{
						strResult.Format(_T("%.2f"), dValue * dValue3);
					}
					break;
				case 21:	//�ɽ���
					vdReq.data_type = dtype_double;
					FindItemOfReq(&reqRealTime, strReq, 3);	//Date
					dValue = GetKLine(pBusiness, reqRealTime.nDate, 6,reqRealTime.nSecurityID);
					if(dValue > 0)
					{
						strResult.Format(_T("%.0f"), dValue);
					}
					break;
				case 22:	//�ɽ����
					vdReq.data_type = dtype_double;
					FindItemOfReq(&reqRealTime, strReq, 3);		//Date
					dValue = GetKLine(pBusiness, reqRealTime.nDate, 5,reqRealTime.nSecurityID);
					if(dValue > 0)
					{
						if(pBusiness->m_pSecurity->IsFundTradeInMarket())
						{
							strResult.Format(_T("%.3f"), dValue);
						}else
						{
							strResult.Format(_T("%.2f"), dValue);
						}
					}
					break;
				case 23:	//������
					vdReq.data_type = dtype_double;
					FindItemOfReq(&reqRealTime, strReq, 3);		//Date
					dValue1 = GetKLine(pBusiness, reqRealTime.nDate, 6,reqRealTime.nSecurityID);
					dValue2 = 0;
					if(pBusiness->m_pSecurity->IsStock())
					{	//��Ʊ
						pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(reqRealTime.nDate);
						if(pTxShareData != NULL)
						{
							dValue2 = pTxShareData->TradeableShare;
						}
					}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
					{	//����
						pTxFundShareData = pBusiness->m_pSecurity->GetTxFundShareDataByDate(reqRealTime.nDate);
						if(pTxFundShareData != NULL)
						{
							dValue2 = pTxFundShareData->TradeableShare;
						}
					}else if(pBusiness->m_pSecurity->IsBond_Change())
					{	//��תծ
						pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
						nDataCount = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
						if(nDataCount > 0)
						{
							pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex();
							if(reqRealTime.nDate < pCBondAmount->end_date)
							{	//��תծ������
								dValue2 = pBondNewInfo->share / 100;
							}else
							{	//δת��ծȯ���
								pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(nDataCount - 1);
								if(reqRealTime.nDate >= pCBondAmount->end_date)
								{
									dValue2 = pCBondAmount->not_change_bond_amount / 100;
								}else
								{
									pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByDate(reqRealTime.nDate);
									dValue2 = pCBondAmount->not_change_bond_amount / 100;
								}
							}
						}else
						{	//��תծ������
							dValue2 = pBondNewInfo->share / 100 ;
						}
					}
					if(dValue1 > 0 && dValue2 > 0)
					{
						strResult.Format(_T("%.2f"), dValue1*100 / dValue2);
					}
					break;
				case 24:	//�ۼƳɽ���
					vdReq.data_type = dtype_double;
					FindItemOfReq(&reqRealTime, strReq, 5);		//StartDate��EndDate
					dValue = GetSumVolume(pBusiness, reqRealTime.nStartDate, reqRealTime.nEndDate);
					if(dValue > 0)
					{
						strResult.Format(_T("%.0f"), dValue);
					}
					break;
				case 25:	//�ۼƳɽ����
					vdReq.data_type = dtype_double;
					FindItemOfReq(&reqRealTime, strReq, 5);		//StartDate��EndDate
					dValue = GetSumAmount(pBusiness, reqRealTime.nStartDate, reqRealTime.nEndDate);
					if(dValue > 0)
					{
						if(pBusiness->m_pSecurity->IsFundTradeInMarket())
						{
							strResult.Format(_T("%.3f"), dValue);
						}else
						{
							strResult.Format(_T("%.2f"), dValue);
						}
					}
					break;
				case 26:	//�ۼƻ�����
					vdReq.data_type = dtype_double;
					FindItemOfReq(&reqRealTime, strReq, 5);		//StartDate��EndDate
					dValue = GetSumExchangeRatio(pBusiness, reqRealTime.nStartDate, reqRealTime.nEndDate);
					if(dValue > 0)
					{
						strResult.Format(_T("%.2f"), 100 * dValue);
					}
					break;
				case 27:	//��ͨ��ֵ
					vdReq.data_type = dtype_double;
					FindItemOfReq(&reqRealTime, strReq, 3);		//Date
					dValue1 = GetKLine(pBusiness, reqRealTime.nDate, 4,reqRealTime.nSecurityID);
					dValue2 = 0;
					if(pBusiness->m_pSecurity->IsStock())
					{	//��Ʊ
						pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(reqRealTime.nDate);
						if(pTxShareData != NULL)
						{
							dValue2 = pTxShareData->TradeableShare;
						}
					}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
					{	//����
						pTxFundShareData = pBusiness->m_pSecurity->GetTxFundShareDataByDate(reqRealTime.nDate);
						if(pTxFundShareData != NULL)
						{
							dValue2 = pTxFundShareData->TradeableShare;
						}
					}else if (pBusiness->m_pSecurity->IsIndex_TX())
					{
						Tx::Data::IndexShareData* pIndexShare = pBusiness->m_pSecurity->GetIndexShareDataByDate(reqTXFunction.nDate,false);
						if ( pIndexShare != NULL )
							dValue2 = pIndexShare->TradeableValue;
					}
					else if(pBusiness->m_pSecurity->IsBond_Change())
					{	//��תծ
						pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
						nDataCount = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
						if(nDataCount > 0)
						{
							pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex();
							if(reqRealTime.nDate < pCBondAmount->end_date)
							{	//��תծ������
								dValue2 = pBondNewInfo->share / 100;
							}else
							{	//δת��ծȯ���
								pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(nDataCount - 1);
								if(reqRealTime.nDate >= pCBondAmount->end_date)
								{
									dValue2 = pCBondAmount->not_change_bond_amount / 100;
								}else
								{
									pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByDate(reqRealTime.nDate);
									dValue2 = pCBondAmount->not_change_bond_amount / 100;
								}
							}
						}else
						{	//��תծ������
							dValue2 = pBondNewInfo->share / 100 ;
						}
					}
					else if(pBusiness->m_pSecurity->IsIndex())
					{
						Tx::Data::IndexShareData* m_pIndexShareData = NULL;
						DataFileNormal<blk_TxExFile_FileHead,IndexShareData>* pDataFile = new DataFileNormal<blk_TxExFile_FileHead,IndexShareData>;
						if(pDataFile!=NULL)
						{
							bool bLoaded = true;
							bLoaded = pDataFile->Load(reqRealTime.nSecurityID,30036,true);
							if(bLoaded)
								m_pIndexShareData = pDataFile->GetDataByObj(reqRealTime.nDate,false);
							if(m_pIndexShareData != NULL)
								dValue = m_pIndexShareData->TradeableValue;
							if(dValue < 0)
								dValue = Con_doubleInvalid;
							if(dValue > 0 &&dValue != Con_doubleInvalid)
								strResult.Format(_T("%.2f"), dValue);
						}
						if(pDataFile != NULL)
							delete pDataFile;
						pDataFile = NULL;
					}
					if(dValue1 > 0 && dValue2 > 0)
					{
						if(pBusiness->m_pSecurity->IsStock())
						{
							strValue.Format(_T("%.2f"), dValue1);
						}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
						{
							strValue.Format(_T("%.3f"), dValue1);
						}else if(pBusiness->m_pSecurity->IsBond_Change())
						{
							strValue.Format(_T("%.2f"), dValue1);
						}
						dValue1 = atof(strValue);
						dValue2 = (ULONGLONG)dValue2;
						dValue = dValue1 * dValue2;
						if(pBusiness->m_pSecurity->IsFundTradeInMarket())
						{
							strResult.Format(_T("%.3f"), dValue);
						}else if( pBusiness->m_pSecurity->IsIndex_TX())
						{
							strResult.Format(_T("%.2f"), dValue2);
						}
						else
						{
							strResult.Format(_T("%.2f"), dValue);
						}
					}
					break;
				case 28:	//������ֵ(A)
				case 279:   //������ֵ(A+B)
					//vdReq.data_type = dtype_double;
					//FindItemOfReq(&reqRealTime, strReq, 3);		//Date
					//dValue1 = GetKLine(pBusiness, reqRealTime.nDate, 4,reqRealTime.nSecurityID);
					//dValue2 = 0;
					//if(pBusiness->m_pSecurity->IsStock())
					//{	//��Ʊ
					//	pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(reqRealTime.nDate);
					//	if(pTxShareData != NULL)
					//	{
					//		if (reqTXFunction.iFuncID == 28 && pBusiness->m_pSecurity->IsStockA())
					//			dValue2 = pTxShareData->TradeableShare;
					//		else
					//		    dValue2 = pTxShareData->TheShare;
					//	}
					//}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
					//{	//����
					//	pTxFundShareData = pBusiness->m_pSecurity->GetTxFundShareDataByDate(reqRealTime.nDate);
					//	if(pTxFundShareData != NULL)
					//	{
					//		dValue2 = pTxFundShareData->TotalShare;
					//	}
					//}else if(pBusiness->m_pSecurity->IsBond_Change())
					//{	//��תծ
					//	pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
					//	nDataCount = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
					//	if(nDataCount > 0)
					//	{
					//		pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex();
					//		if(reqRealTime.nDate < pCBondAmount->end_date)
					//		{	//��תծ������
					//			dValue2 = pBondNewInfo->share / 100;
					//		}else
					//		{	//δת��ծȯ���
					//			pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(nDataCount - 1);
					//			if(reqRealTime.nDate >= pCBondAmount->end_date)
					//			{
					//				dValue2 = pCBondAmount->not_change_bond_amount / 100;
					//			}else
					//			{
					//				pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByDate(reqRealTime.nDate);
					//				dValue2 = pCBondAmount->not_change_bond_amount / 100;
					//			}
					//		}
					//	}else
					//	{	//��תծ������
					//		dValue2 = pBondNewInfo->share / 100 ;
					//	}
					//}
					//else if(pBusiness->m_pSecurity->IsIndex())
					//{
					//	Tx::Data::IndexShareData* m_pIndexShareData = NULL;
					//	DataFileNormal<blk_TxExFile_FileHead,IndexShareData>* pDataFile = new DataFileNormal<blk_TxExFile_FileHead,IndexShareData>;
					//	if(pDataFile!=NULL)
					//	{
					//		bool bLoaded = true;
					//		bLoaded = pDataFile->Load(reqRealTime.nSecurityID,30036,true);
					//		if(bLoaded)
					//			m_pIndexShareData = pDataFile->GetDataByObj(reqRealTime.nDate,false);
					//		if(m_pIndexShareData != NULL)
					//			dValue = m_pIndexShareData->TotalValue;
					//		if(dValue < 0)
					//			dValue = Con_doubleInvalid;
					//		if(dValue > 0 &&dValue != Con_doubleInvalid)
					//			strResult.Format(_T("%.2f"), dValue);
					//	}
					//	if(pDataFile != NULL)
					//		delete pDataFile;
					//	pDataFile = NULL;
					//}
					//if(dValue1 > 0 && dValue2 > 0)
					//{
					//	if(pBusiness->m_pSecurity->IsStock())
					//	{
					//		strValue.Format(_T("%.2f"), dValue1);
					//	}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
					//	{
					//		strValue.Format(_T("%.3f"), dValue1);
					//	}else if(pBusiness->m_pSecurity->IsBond_Change())
					//	{
					//		strValue.Format(_T("%.2f"), dValue1);
					//	}
					//	dValue1 = atof(strValue);
					//	dValue2 = (ULONGLONG)dValue2;
					//	dValue = dValue1 * dValue2;
					//	if(pBusiness->m_pSecurity->IsFundTradeInMarket())
					//	{
					//		strResult.Format(_T("%.3f"), dValue);
					//	}else
					//	{
					//		strResult.Format(_T("%.2f"), dValue);
					//	}
					//}
					break;
				case 284:   //������ֵ(B)
					break;
				case 29:	//����ֵ
					vdReq.data_type = dtype_double;
					FindItemOfReq(&reqRealTime, strReq, 3);		//Date
					dValue1 = GetKLine(pBusiness, reqRealTime.nDate, 4,reqRealTime.nSecurityID);
					dValue2 = 0;
					if(pBusiness->m_pSecurity->IsStock())
					{	//��Ʊ
						pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(reqRealTime.nDate);
						if(pTxShareData != NULL)
						{
							dValue2 = pTxShareData->TotalShare;
						}
					}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
					{	//����
						pTxFundShareData = pBusiness->m_pSecurity->GetTxFundShareDataByDate(reqRealTime.nDate);
						if(pTxFundShareData != NULL)
						{
							dValue2 = pTxFundShareData->TotalShare;
						}
					}else if (pBusiness->m_pSecurity->IsIndex_TX())
					{
						Tx::Data::IndexShareData* pIndexShare = pBusiness->m_pSecurity->GetIndexShareDataByDate(reqTXFunction.nDate,false);
						if ( pIndexShare != NULL )
							dValue2 = pIndexShare->TotalValue;
					}else if(pBusiness->m_pSecurity->IsBond_Change())
					{	//��תծ
						pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
						nDataCount = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
						if(nDataCount > 0)
						{
							pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex();
							if(reqRealTime.nDate < pCBondAmount->end_date)
							{	//��תծ������
								dValue2 = pBondNewInfo->share / 100;
							}else
							{	//δת��ծȯ���
								pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(nDataCount - 1);
								if(reqRealTime.nDate >= pCBondAmount->end_date)
								{
									dValue2 = pCBondAmount->not_change_bond_amount / 100;
								}else
								{
									pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByDate(reqRealTime.nDate);
									dValue2 = pCBondAmount->not_change_bond_amount / 100;
								}
							}
						}else
						{	//��תծ������
							dValue2 = pBondNewInfo->share / 100 ;
						}
					}
					else if(pBusiness->m_pSecurity->IsIndex())
					{
						Tx::Data::IndexShareData* m_pIndexShareData = NULL;
						DataFileNormal<blk_TxExFile_FileHead,IndexShareData>* pDataFile = new DataFileNormal<blk_TxExFile_FileHead,IndexShareData>;
						if(pDataFile!=NULL)
						{
							bool bLoaded = true;
							bLoaded = pDataFile->Load(reqRealTime.nSecurityID,30036,true);
							if(bLoaded)
								m_pIndexShareData = pDataFile->GetDataByObj(reqRealTime.nDate,false);
							if(m_pIndexShareData != NULL)
								dValue = m_pIndexShareData->TotalValue;
							if(dValue < 0)
								dValue = Con_doubleInvalid;
							if(dValue > 0 &&dValue != Con_doubleInvalid)
								strResult.Format(_T("%.2f"), dValue);
						}
						if(pDataFile != NULL)
							delete pDataFile;
						pDataFile = NULL;
					}
					if(dValue1 > 0 && dValue2 > 0)
					{
						if(pBusiness->m_pSecurity->IsStock())
						{
							strValue.Format(_T("%.2f"), dValue1);
						}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
						{
							strValue.Format(_T("%.3f"), dValue1);
						}else if(pBusiness->m_pSecurity->IsBond_Change())
						{
							strValue.Format(_T("%.2f"), dValue1);
						}
						dValue1 = atof(strValue);
						dValue2 = (ULONGLONG)dValue2;
						dValue = dValue1 * dValue2;
						if(pBusiness->m_pSecurity->IsFundTradeInMarket())
						{
							strResult.Format(_T("%.3f"), dValue);
						}else if( pBusiness->m_pSecurity->IsIndex_TX())
						{
							strResult.Format(_T("%.2f"), dValue2);
						}
						else 
						{
							strResult.Format(_T("%.2f"), dValue);
						}
					}
					break;
				case 30:	//��������ӽ�ָ���۸�(<=)�ĵ�һ������
					vdReq.data_type = dtype_int4;
					FindItemOfReq(&reqRealTime, strReq, 11);	//StartDate��EndDate���۸����͡���Ȩ���͡��۸�
					nDate = Get_Price_Occur_Date(pBusiness, reqRealTime.dPrice, reqRealTime.iPriceType, reqRealTime.iIRType, reqRealTime.nStartDate, reqRealTime.nEndDate);
					if(nDate > 0)
					{
						//�޸�Ϊ8λ��--20080708--
						strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
						//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
					}
					break;
				case 31:	//��������ӽ�ָ����������(<=)�ĵ�һ������
					vdReq.data_type = dtype_int4;
					FindItemOfReq(&reqRealTime, strReq, 10);	//StartDate��EndDate���������͡���������
					nDate = Get_Value_Occur_Date(pBusiness, reqRealTime.dFindValue, reqRealTime.iValueType, reqRealTime.nStartDate, reqRealTime.nEndDate);
					if(nDate > 0)
					{
						//�޸�Ϊ8λ��--20080708--
						strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
						//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
					}
					break;
				case 100:	//����������������Сֵ�ĵ�һ������
					vdReq.data_type = dtype_int4;
					FindItemOfReq(&reqRealTime, strReq, 7);		//StartDate��EndDate����������
					nDate = Get_Value_Extremum_Date(pBusiness, reqRealTime.iValueType, reqRealTime.nStartDate, reqRealTime.nEndDate, TRUE);
					if(nDate > 0)
					{
						//�޸�Ϊ8λ��--20080708--
						strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
						//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
					}
					break;
				case 101:	//�����������������ֵ�ĵ�һ������
					vdReq.data_type = dtype_int4;
					FindItemOfReq(&reqRealTime, strReq, 7);		//StartDate��EndDate����������
					nDate = Get_Value_Extremum_Date(pBusiness, reqRealTime.iValueType, reqRealTime.nStartDate, reqRealTime.nEndDate, FALSE);
					if(nDate > 0)
					{
						//�޸�Ϊ8λ��--20080708--
						strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
						//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
					}
					break;
				case 102:	//�����ڼ۸���Сֵ�ĵ�һ������
					vdReq.data_type = dtype_int4;
					FindItemOfReq(&reqRealTime, strReq, 8);		//StartDate��EndDate���۸�����
					nDate = Get_Price_Extremum_Date(pBusiness, reqRealTime.iPriceType, reqRealTime.nStartDate, reqRealTime.nEndDate, TRUE);
					if(nDate > 0)
					{
						//�޸�Ϊ8λ��--20080708--
						strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
						//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
					}
					break;
				case 103:	//�����ڼ۸����ֵ�ĵ�һ������
					vdReq.data_type = dtype_int4;
					FindItemOfReq(&reqRealTime, strReq, 8);		//StartDate��EndDate���۸�����
					nDate = Get_Price_Extremum_Date(pBusiness, reqRealTime.iPriceType, reqRealTime.nStartDate, reqRealTime.nEndDate, FALSE);
					if(nDate > 0)
					{
						//�޸�Ϊ8λ��--20080708--
						strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
						//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
					}
					break;
				case 32:	//�׶��Ƿ�
					vdReq.data_type = dtype_double;
					FindItemOfReq(&reqRealTime, strReq, 5);		//StartDate��EndDate
					//��StartDateΪ��׼��EndDate���С���Ȩ��
					dValue3 = GetKLine(pBusiness, reqRealTime.nStartDate, 1,reqRealTime.nSecurityID);	//�״η��м۸�==��������ǰ�ռ�
					dValue1 = GetKLine(pBusiness, reqRealTime.nEndDate, 4,reqRealTime.nSecurityID);
					dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqRealTime.nStartDate, reqRealTime.nEndDate, false);
					if(dValue1 > 0 && dValue2 > 0 && dValue3 > 0)
					{
						dValue = (dValue1 * dValue2) / dValue3 - 1;
						strResult.Format(_T("%.2f"), 100*dValue);
					}
					break;
				case 321:	//�׶��Ƿ�(������ʼ���ڵ����Ƿ�)
					vdReq.data_type = dtype_double;
					FindItemOfReq(&reqRealTime, strReq, 5);		//StartDate��EndDate
					//��StartDateΪ��׼��EndDate���С���Ȩ��
					dValue3 = GetKLine(pBusiness, reqRealTime.nStartDate, 4,reqRealTime.nSecurityID);	//�״η��м۸�==��������ǰ�ռ�
					dValue1 = GetKLine(pBusiness, reqRealTime.nEndDate, 4,reqRealTime.nSecurityID);
					dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqRealTime.nStartDate, reqRealTime.nEndDate, false);
					if(dValue1 > 0 && dValue2 > 0 && dValue3 > 0)
					{
						dValue = (dValue1 * dValue2) / dValue3 - 1;
						strResult.Format(_T("%.2f"), 100*dValue);
					}
					break;
				case 322:	//�����������Ƿ�
					vdReq.data_type = dtype_double;
					FindItemOfReq(&reqTXFunction, strReq, 3);	
					dValue3 = GetKLine(pBusiness, pBusiness->m_pSecurity->GetTradeDateByIndex(0), 0,reqTXFunction.nSecurityID);	
					dValue1 = GetKLine(pBusiness, reqTXFunction.nDate, 4,reqTXFunction.nSecurityID);
					dValue2 = pBusiness->m_pSecurity->GetExdividendScale(pBusiness->m_pSecurity->GetTradeDateByIndex(0), reqTXFunction.nDate, false);
					if(dValue1 > 0 && dValue2 > 0 && dValue3 > 0)
					{
						//��Ϊ�ٷ���20080710
						dValue = (dValue1 * dValue2) / dValue3 - 1;
						strResult.Format(_T("%.2f"), 100*dValue);
					}
					break;
				case 33:	//������
					vdReq.data_type = dtype_double;
					{
						dValue = 0.0;
						FindItemOfReq(&reqRealTime,strReq,3 );
						//dValue = pBusiness->m_pSecurity->GetBuyVolume( false );
						//TradeQuotation* pTQ = NULL;						
						if(pBusiness->m_pSecurity!=NULL)
						{
							//�õ�ǰ�ռ۸񣬺�����������
							//pTQ = pBusiness->m_pSecurity->GetTradeQuotationPointer();
							pBusiness->m_pSecurity->RequestQuotation();
							dValue = pBusiness->m_pSecurity->GetBuyVolume( false );
						}
						if ( dValue > 0.0 )
							strResult.Format(_T("%.2f"),dValue);
					}
					break;
				case 34:	//������
					vdReq.data_type = dtype_double;
					{
						dValue = 0.0;
						FindItemOfReq(&reqRealTime,strReq,3 );
						TradeQuotation* pTQ = NULL;						
						if(pBusiness->m_pSecurity!=NULL)
						{
							//�õ�ǰ�ռ۸񣬺�����������
							pTQ = pBusiness->m_pSecurity->GetTradeQuotationPointer();
							dValue = pBusiness->m_pSecurity->GetSaleVolume( false );
						}
						if ( dValue > 0.0 )
							strResult.Format(_T("%.2f"),dValue);
					}
					break;
				case 144:	//���5�����̼�
						vdReq.data_type = dtype_float;
						FindItemOfReq(&reqRealTime, strReq, 23);	//���������
						if(pBusiness == NULL || pBusiness->m_pSecurity == NULL)
							break;
						pBusiness->m_pSecurity->RequestQuotation();
						pTradeQuotation = pBusiness->m_pSecurity->GetTradeQuotationPointer();
						pTradeQuotation = pBusiness->m_pSecurity->GetTradeQuotationPointer();
						if(pTradeQuotation != NULL)
						{
							nDataCount = pTradeQuotation->GetTradeQuotationDataCount();
							if(nDataCount == 10)
							{
								pTradeQuotationData = pTradeQuotation->GetBuyData(reqRealTime.iQuotationNo);
								if(pTradeQuotationData != NULL)
								{
									dValue = pTradeQuotationData->fPrice;
									if(pBusiness->m_pSecurity->IsFundTradeInMarket())
									{
										strResult.Format(_T("%.3f"), dValue);
									}else
									{
										strResult.Format(_T("%.2f"), dValue);
									}
								}
							}
						}
					break;
				case 145:	//���5�����̼�
					vdReq.data_type = dtype_float;
					FindItemOfReq(&reqRealTime, strReq, 23);	//���������
						if(pBusiness == NULL || pBusiness->m_pSecurity == NULL)
							break;
						pBusiness->m_pSecurity->RequestQuotation();
					pTradeQuotation = pBusiness->m_pSecurity->GetTradeQuotationPointer();
					if(pTradeQuotation != NULL)
					{
						nDataCount = pTradeQuotation->GetTradeQuotationDataCount();
						if(nDataCount == 10)
						{
							pTradeQuotationData = pTradeQuotation->GetSaleData(reqRealTime.iQuotationNo);
							if(pTradeQuotationData != NULL)
							{
								dValue = pTradeQuotationData->fPrice;
								if(pBusiness->m_pSecurity->IsFundTradeInMarket())
								{
									strResult.Format(_T("%.3f"), dValue);
								}else
								{
									strResult.Format(_T("%.2f"), dValue);
								}
							}
						}
					}
					break;
				case 146:	//���5��������
					vdReq.data_type = dtype_double;
					FindItemOfReq(&reqRealTime, strReq, 23);	//���������
						if(pBusiness == NULL || pBusiness->m_pSecurity == NULL)
							break;
						pBusiness->m_pSecurity->RequestQuotation();
					pTradeQuotation = pBusiness->m_pSecurity->GetTradeQuotationPointer();
					if(pTradeQuotation != NULL)
					{
						nDataCount = pTradeQuotation->GetTradeQuotationDataCount();
						if(nDataCount == 10)
						{
							pTradeQuotationData = pTradeQuotation->GetBuyData(reqRealTime.iQuotationNo);
							if(pTradeQuotationData != NULL)
							{
								dValue = pTradeQuotationData->dVolume;
								strResult.Format(_T("%.0f"), dValue);
							}
						}
					}
					break;
				case 147:	//���5��������

					vdReq.data_type = dtype_double;
					FindItemOfReq(&reqRealTime, strReq, 23);	//���������
						if(pBusiness == NULL || pBusiness->m_pSecurity == NULL)
							break;
						pBusiness->m_pSecurity->RequestQuotation();
					pTradeQuotation = pBusiness->m_pSecurity->GetTradeQuotationPointer();
					if(pTradeQuotation != NULL)
					{
						nDataCount = pTradeQuotation->GetTradeQuotationDataCount();
						if(nDataCount == 10)
						{
							pTradeQuotationData = pTradeQuotation->GetSaleData(reqRealTime.iQuotationNo);
							if(pTradeQuotationData != NULL)
							{
								dValue = pTradeQuotationData->dVolume;
								strResult.Format(_T("%.0f"), dValue);
							}
						}
					}
					break;
				case 148:	//ծȯ���5�����̼۶�Ӧ�ĵ���������
					vdReq.data_type = dtype_double;
					if(pBusiness->m_pSecurity->IsBond())
					{
						FindItemOfReq(&reqRealTime, strReq, 23);	//���������

						if (CTime::GetCurrentTime()-tBYYtm[reqRealTime.iQuotationNo-1] > CTimeSpan(0,0,0,5))
						{
							GetMmpBondYtm(reqRealTime.iQuotationNo);
							tBYYtm[reqRealTime.iQuotationNo-1] = CTime::GetCurrentTime();
						}

						dValue = GetMmpBondYtm(reqRealTime.nSecurityID,reqRealTime.iQuotationNo);
						if (dValue > 0)
						{
							strResult.Format(_T("%.5f"), dValue);
						}
					}
					break;

					//vdReq.data_type = dtype_double;
					//if(pBusiness->m_pSecurity->IsBond())
					//{
					//	FindItemOfReq(&reqRealTime, strReq, 23);	//���������
					//	pTradeQuotation = pBusiness->m_pSecurity->GetTradeQuotationPointer();
					//	if(pTradeQuotation != NULL)
					//	{
					//		nDataCount = pTradeQuotation->GetTradeQuotationDataCount();
					//		if(nDataCount == 10)
					//		{
					//			pTradeQuotationData = pTradeQuotation->GetBuyData(reqRealTime.iQuotationNo);
					//			if(pTradeQuotationData != NULL)
					//			{
					//				nDate = pBusiness->m_pSecurity->GetCurDataDate();
					//				dValue1 = pTradeQuotationData->fPrice;
					//				if(pBusiness->m_pSecurity->IsBond_National())
					//				{	//��ծʵ�о��۽��ף����Ǽ��㵽��������ʱծȯ�۸�Ϊȫ��
					//					//dValue2 = bond.GetInterest((INT)(reqRealTime.nSecurityID), nDate);
					//					//2012-7-16  Ӧ����Ϣ(��)
					//					dValue2 = bond.GetInterest_New((INT)(reqRealTime.nSecurityID), nDate,true);
					//					if(dValue2 < 0)
					//					{
					//						break;
					//					}
					//					dValue1 += dValue2;
					//				}
					//				bond.Calc((INT)(reqRealTime.nSecurityID), nDate, (FLOAT)dValue1);
					//				dValue = bond.Get_YTM();
					//				if(dValue > 0)
					//				{
					//					strResult.Format(_T("%.5f"), dValue);
					//				}
					//			}
					//		}
					//	}
					//}
					//break;
				case 149:	//ծȯ���5�����̼۶�Ӧ�ĵ���������
					vdReq.data_type = dtype_double;
					if(pBusiness->m_pSecurity->IsBond())
					{
						FindItemOfReq(&reqRealTime, strReq, 23);	//���������

						if (CTime::GetCurrentTime()-tSPYtm[reqRealTime.iQuotationNo-1] > CTimeSpan(0,0,0,5))
						{
							GetMmpBondYtm(reqRealTime.iQuotationNo,false);
							tSPYtm[reqRealTime.iQuotationNo-1] = CTime::GetCurrentTime();
						}

						dValue = GetMmpBondYtm(reqRealTime.nSecurityID,reqRealTime.iQuotationNo,false);
						if (dValue > 0)
						{
							strResult.Format(_T("%.5f"), dValue);
						}
					}
					break;

					//vdReq.data_type = dtype_double;
					//if(pBusiness->m_pSecurity->IsBond())
					//{
					//	FindItemOfReq(&reqRealTime, strReq, 23);	//���������
					//	pTradeQuotation = pBusiness->m_pSecurity->GetTradeQuotationPointer();
					//	if(pTradeQuotation != NULL)
					//	{
					//		nDataCount = pTradeQuotation->GetTradeQuotationDataCount();
					//		if(nDataCount == 10)
					//		{
					//			pTradeQuotationData = pTradeQuotation->GetSaleData(reqRealTime.iQuotationNo);
					//			if(pTradeQuotationData != NULL)
					//			{
					//				nDate = pBusiness->m_pSecurity->GetCurDataDate();
					//				dValue1 = pTradeQuotationData->fPrice;
					//				if(pBusiness->m_pSecurity->IsBond_National())
					//				{	//��ծʵ�о��۽��ף����Ǽ��㵽��������ʱծȯ�۸�Ϊȫ��
					//					//dValue2 = bond.GetInterest((INT)(reqRealTime.nSecurityID), nDate);
					//					//2012-7-16  Ӧ����Ϣ(��)
					//					dValue2 = bond.GetInterest_New((INT)(reqRealTime.nSecurityID), nDate,true);
					//					if(dValue2 < 0)
					//					{
					//						break;
					//					}
					//					dValue1 += dValue2;
					//				}
					//				bond.Calc((INT)(reqRealTime.nSecurityID), nDate, (FLOAT)dValue1);
					//				dValue = bond.Get_YTM();
					//				if(dValue > 0)
					//				{
					//					strResult.Format(_T("%.5f"), dValue);
					//				}
					//			}
					//		}
					//	}
					//}
					//break;		
				case 81:	//IOPV(ETF����LOF����)
				case 286:   //IOPV(ETF����LOF����)
					vdReq.data_type = dtype_double;
					if(!(pBusiness->m_pSecurity->IsFund_ETF() || pBusiness->m_pSecurity->IsFund_LOF()))
					{
						break;
					}
					/*
					dValue = pBusiness->m_pSecurity->GetIOPV();
					if(dValue > 0)
					{
						strResult.Format(_T("%.4f"), dValue);
					}
					*/
					mapIntInt.clear();
					Tx::Data::TypeMapManage::GetInstance()->GetTypeMapITI(TYPE_IOPV_ID_ID, mapIntInt);
					iterIntInt = mapIntInt.find((INT)(reqRealTime.nSecurityID));
					if(iterIntInt != mapIntInt.end())
					{
						if(business.GetSecurityNow((LONG)(iterIntInt->second)) != NULL)
						{
							if(pBusiness->m_pSecurity->IsValid())
							{
								dValue = business.m_pSecurity->GetClosePrice(true);
							}else
							{
								dValue = business.m_pSecurity->GetClosePrice(pBusiness->m_pSecurity->GetTradeDateLatest());
							}
							if(dValue > 0)
							{
								strResult.Format(_T("%.4f"), dValue);
							}
						}
					}
					break;
				case 62:	//Ȩ֤-����������

					vdReq.data_type = dtype_double;
					FindItemOfReq(&reqRealTime, strReq, 3);		//Date
					nSecurityID = warrant.GetObjectSecurityId((INT)(reqRealTime.nSecurityID));
					nStartDate = pBusiness->m_pSecurity->GetIPOListedDate();
					nEndDate = warrant.GetPowerEndDate((INT)(reqRealTime.nSecurityID));
					if(nSecurityID > 0 && nStartDate > 0 && nEndDate > 0 && reqRealTime.nDate >= nStartDate && 
						reqRealTime.nDate <= nEndDate)
					{
						double dPowerRatio = warrant.GetPowerRatio((INT)(reqRealTime.nSecurityID));
						double dPrice = pBusiness->m_pSecurity->GetClosePrice( reqRealTime.nDate );
						if( dPowerRatio != 0.0 )
							dValue3 = dPrice/dPowerRatio;
						business.GetSecurityNow((LONG)nSecurityID);
						dValue1 = GetKLine(&business, reqRealTime.nDate, 4,reqRealTime.nSecurityID);
						dValue2 = (DOUBLE)(warrant.GetRemainsDays((INT)(reqRealTime.nSecurityID), reqRealTime.nDate)) / 365;
						//pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(reqRealTime.nDate);
						//if(pTxShareData != NULL)
						//{
							//dValue3 = GetKLine(pBusiness, reqRealTime.nDate, 4) * pTxShareData->TotalShare;
							dValue = warrant.GetSigma(!pBusiness->m_pSecurity->IsWarrant_Buy(), dValue1, 
														warrant.GetPowerPrice((INT)(reqRealTime.nSecurityID)), 
														0.018, 0.0, dValue2, dValue3);
							strResult.Format(_T("%.3f"), dValue);
						//}
					}
					break;
				case 63:	//Ȩ֤-�����
					vdReq.data_type = dtype_double;
					FindItemOfReq(&reqRealTime, strReq, 3);		//Date
					nSecurityID = warrant.GetObjectSecurityId((INT)(reqRealTime.nSecurityID));
					nStartDate = pBusiness->m_pSecurity->GetIPOListedDate();
					nEndDate = warrant.GetPowerEndDate((INT)(reqRealTime.nSecurityID));
					if(nSecurityID > 0 && nStartDate > 0 && nEndDate > 0 && reqRealTime.nDate >= nStartDate && 
						reqRealTime.nDate <= nEndDate)
					{
						business.GetSecurityNow((LONG)nSecurityID);
						dValue1 = GetKLine(pBusiness, reqRealTime.nDate, 4,reqRealTime.nSecurityID);
						dValue2 = GetKLine(&business, reqRealTime.nDate, 4,reqRealTime.nSecurityID);
						dValue = warrant.GetPremiumRate(dValue1, dValue2, 
														warrant.GetPowerPrice((INT)(reqRealTime.nSecurityID)),
														pBusiness->m_pSecurity->IsWarrant_Buy(), 
														warrant.GetPowerRatio((INT)(reqRealTime.nSecurityID)));
						strResult.Format(_T("%.2f"), dValue);
					}
					break;
				case 64:	//Ȩ֤-�ܸ˱���
					vdReq.data_type = dtype_double;
					FindItemOfReq(&reqRealTime, strReq, 3);		//Date
					nSecurityID = warrant.GetObjectSecurityId((INT)(reqRealTime.nSecurityID));
					nStartDate = pBusiness->m_pSecurity->GetIPOListedDate();
					nEndDate = warrant.GetPowerEndDate((INT)(reqRealTime.nSecurityID));
					if(nSecurityID > 0 && nStartDate > 0 && nEndDate > 0 && reqRealTime.nDate >= nStartDate && 
						reqRealTime.nDate <= nEndDate)
					{
						business.GetSecurityNow((LONG)nSecurityID);
						dValue1 = GetKLine(&business, reqRealTime.nDate, 4,reqRealTime.nSecurityID);
						dValue2 = GetKLine(pBusiness, reqRealTime.nDate, 4,reqRealTime.nSecurityID);
						dValue = warrant.GetGearRate(dValue1, dValue2, warrant.GetPowerRatio((INT)(reqRealTime.nSecurityID)));
						strResult.Format(_T("%.2f"), dValue);
					}
					break;
				case 98:	//ծȯYTM
				case 104:	//ծȯ��������
				case 105:	//ծȯ͹��
					{

						vdReq.data_type = dtype_double;
						if (!pBusiness->m_pSecurity->IsBond())
							break;
						if (CTime::GetCurrentTime()-tYtmLast > CTimeSpan(0,0,0,5))
						{
							GetBondYTM();
							tYtmLast = CTime::GetCurrentTime();
						}
						double dYtm,dDur,dMDur,dCon;
						GetBondYTM(reqRealTime.nSecurityID,dYtm,dDur,dMDur,dCon);

						switch(reqRealTime.iFuncID)
						{
						case 98:
							dValue =dYtm;
							if(dValue != Tx::Core::Con_doubleInvalid)
							{
								strResult.Format(_T("%.6f"), dValue*100);
							}
							break;
						case 104:
							dValue = dMDur;
							if(dValue != Tx::Core::Con_doubleInvalid)
							{
								strResult.Format(_T("%.6f"), dValue);
							}
							break;
						case 105:
							dValue = dCon;
							if(dValue != Tx::Core::Con_doubleInvalid)
							{
								strResult.Format(_T("%.6f"), dValue);
							}
							break;
						default:
							break;
						}
					}
					break;

					//////////////////////////////////////////////////////////////////////////

					//vdReq.data_type = dtype_double;
					//FindItemOfReq(&reqRealTime, strReq, 17);	//Date��Price
					//FindItemOfReq(&reqRealTime, strReq, 8);    //iPriceType
					//if(reqRealTime.dPrice == 0)
					//{
					//	reqRealTime.dPrice = GetKLine(pBusiness, reqRealTime.nDate, 4,reqRealTime.nSecurityID);
					//}
					//if(reqRealTime.dPrice <= 0)
					//{
					//	break;
					//}
					////if(pBusiness->m_pSecurity->IsBond_National())
					//if(reqRealTime.iPriceType == 0)
					//{	//��ծʵ�о��۽��ף����Ǽ��㵽��������ʱծȯ�۸�Ϊȫ��
					//	//dValue = bond.GetInterest((INT)(reqRealTime.nSecurityID), reqRealTime.nDate);
					//	//2012-7-16  Ӧ����Ϣ(��)
					//	dValue = bond.GetInterest_New((INT)(reqRealTime.nSecurityID), reqRealTime.nDate,true);
					//	if(dValue < 0)
					//	{
					//		break;
					//	}
					//	reqRealTime.dPrice += dValue;
					//}
					//bond.Calc((INT)(reqRealTime.nSecurityID), reqRealTime.nDate, (FLOAT)(reqRealTime.dPrice));
					//switch(reqRealTime.iFuncID)
					//{
					//case 98:
					//	dValue = bond.Get_YTM();
					//	if(dValue != Tx::Core::Con_doubleInvalid)
					//	{
					//		strResult.Format(_T("%.6f"), dValue*100);
					//	}
					//	break;
					//case 104:
					//	dValue = bond.Get_MDURATION();
					//	if(dValue != Tx::Core::Con_doubleInvalid)
					//	{
					//		strResult.Format(_T("%.6f"), dValue);
					//	}
					//	break;
					//case 105:
					//	dValue = bond.Get_CONVEXITY();
					//	if(dValue != Tx::Core::Con_doubleInvalid)
					//	{
					//		strResult.Format(_T("%.6f"), dValue);
					//	}
					//	break;
					//default:
					//	break;
					//}
					//break;

					//////////////////////////////////////////////////////////////////////////

				case 113:	//��תծ-ת��ƽ��
					vdReq.data_type = dtype_double;
					FindItemOfReq(&reqRealTime, strReq, 3);		//Date
					dValue = bond.CalcParity((INT)(reqRealTime.nSecurityID), reqRealTime.nDate);
					if(dValue > 0)
					{
						strResult.Format(_T("%.2f"), dValue);
					}
					break;
				case 114:	//��תծ-ת�������
					vdReq.data_type = dtype_double;
					FindItemOfReq(&reqRealTime, strReq, 3);		//Date
					dValue = bond.CalcPremium((INT)(reqRealTime.nSecurityID), reqRealTime.nDate);
					if(dValue != Tx::Core::Con_doubleInvalid)
					{
						strResult.Format(_T("%.4f"), dValue);
					}
					break;
				case 115:	//��תծ-ת���׼�
					vdReq.data_type = dtype_double;
					FindItemOfReq(&reqRealTime, strReq, 3);		//Date
					dValue = bond.GetFloor((INT)(reqRealTime.nSecurityID));
					if(dValue > 0)
					{
						strResult.Format(_T("%.2f"), dValue);
					}
					break;
				case 116:	//��תծ-�׼������
					vdReq.data_type = dtype_double;
					FindItemOfReq(&reqRealTime, strReq, 3);		//Date
					//��תծ-ת���׼�
					dValue1 = bond.GetFloor((INT)(reqRealTime.nSecurityID));
					dValue2 = GetKLine(pBusiness, reqRealTime.nDate, 4,reqRealTime.nSecurityID);
					if(dValue1 > 0 && dValue2 > 0)
					{
						dValue = dValue2 / dValue1 - 1;
						strResult.Format(_T("%.4f"), dValue);
					}
					break;
				case 151:	//��ʷ��ʱ�ļ۸�
				case 152:	//��ʷ��ʱ�ĳɽ���
				case 153:	//��ʷ��ʱ�ĳɽ����
					{
						strResult = Con_strInvalid;
						long nCount = 0;
						FindItemOfReq(&reqRealTime, strReq, 19);	//Security��Date��iNo
						pBusiness->m_pSecurity->Request241(false);
						Tx::Data::HisTradeData* pHisTrade = pBusiness->m_pSecurity->Get241TradeData(reqRealTime.iHolderNo-1);
						if ( pHisTrade == NULL )
							break;
						CString strFormatType = _T("%.2f");
						if( pBusiness->m_pSecurity->IsFund() || (pBusiness->m_pSecurity->IsStockB() && pBusiness->m_pSecurity->IsShanghai()) )
							strFormatType = _T("%.3f");
						switch(reqRealTime.iFuncID )		
						{
						case 151:
							dValue = pHisTrade->Close;
							if ( dValue >= 0.0)
								strResult.Format(strFormatType,dValue);
							break;
						case 152:
							dValue = pHisTrade->Volume;
							if ( dValue >= 0.0)
								strResult.Format(strFormatType,dValue);
							break;
						case 153:
							dValue = pHisTrade->Amount;
							if ( dValue >= 0.0)
								strResult.Format(strFormatType,dValue);
							break;
						default:
							break;
						/*
						m_pHisDetail = pBusiness->m_pSecurity->GetHisTradeDataCustom( reqTXFunction.nDate, nCount);
						if ( m_pHisDetail == NULL || nCount <= 0 )
							break;
						if ( reqTXFunction.iHolderNo <= nCount )
						{
							switch(reqTXFunction.iFuncID )		
							{
							case 151:
								dValue = m_pHisDetail[reqTXFunction.iHolderNo-1].Close;
								if ( dValue >= 0.0)
									strResult.Format(_T("%.2f"),dValue);
								break;
							case 152:
								dValue = m_pHisDetail[reqTXFunction.iHolderNo-1].Volume;
								if ( dValue >= 0.0)
									strResult.Format(_T("%.2f"),dValue);
								break;
							case 153:
								dValue = m_pHisDetail[reqTXFunction.iHolderNo-1].Amount;
								if ( dValue >= 0.0)
									strResult.Format(_T("%.2f"),dValue);
								break;
							default:
								break;
							}
						}
						*/
						}
					}
					break;
				case 200:	//�ʽ�����
					{
						strResult = Con_strInvalid;
						FindItemOfReq(&reqTXFunction, strReq, 16);
						if ( pBusiness->m_pSecurity->IsTodayTradeDate())
							dValue = pBusiness->m_pSecurity->GetBuyAmount();
						else
							break;
/*						else
						{
							pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_AmountFlow,pBusiness->m_pSecurity->GetTradeDateLatest(),&m_pAmountFlow,false);
							if ( m_pAmountFlow == NULL )
								break;
							else
								dValue = m_pAmountFlow->f_inflow;	
						}	*/					
						if ( dValue!=Con_doubleInvalid && dValue > 0.0 )
							strResult.Format(_T("%.2f"),dValue );
					}
					break;
				case 201:
					{
						strResult = Con_strInvalid;
						FindItemOfReq(&reqTXFunction, strReq, 16);
						if ( pBusiness->m_pSecurity->IsTodayTradeDate())
							dValue = pBusiness->m_pSecurity->GetSaleAmount();
						else
							break;
						//else
						//{
						//	pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_AmountFlow,pBusiness->m_pSecurity->GetTradeDateLatest(),&m_pAmountFlow,false);
						//	if ( m_pAmountFlow == NULL )
						//		break;
						//	else
						//		dValue = m_pAmountFlow->f_outFlow;	
						//}
						if ( dValue!=Con_doubleInvalid )
							strResult.Format(_T("%.2f"),dValue );
					}
					break;
				case 202:
					{
						strResult = Con_strInvalid;
						FindItemOfReq(&reqTXFunction, strReq, 16);
						if ( pBusiness->m_pSecurity->IsTodayTradeDate())
						{
							dValue1 = pBusiness->m_pSecurity->GetBuyAmount();
							dValue2 = pBusiness->m_pSecurity->GetSaleAmount();
						}
						else
							break;
						//else
						//{
						//	pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_AmountFlow,pBusiness->m_pSecurity->GetTradeDateLatest(),&m_pAmountFlow,false);
						//	if ( m_pAmountFlow == NULL )
						//		break;
						//	dValue1 = m_pAmountFlow->f_inflow;
						//	dValue2 = m_pAmountFlow->f_outFlow;	
						//}
						if ( dValue1 != Con_doubleInvalid  && dValue2 != Con_doubleInvalid )
							strResult.Format(_T("%.2f"),dValue1 - dValue2 );
					}
					break;
				case 207:
					{
						strResult = Con_strInvalid;
						FindItemOfReq(&reqTXFunction, strReq, 16);
						if ( pBusiness->m_pSecurity->IsTodayTradeDate())
						{
							dValue1 = pBusiness->m_pSecurity->GetBuyAmount();
							dValue2 = pBusiness->m_pSecurity->GetSaleAmount();
						}
						else
							break;
						//else
						//{
						//	pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_AmountFlow,pBusiness->m_pSecurity->GetTradeDateLatest(),&m_pAmountFlow,false);
						//	if ( m_pAmountFlow == NULL )
						//		break;
						//	dValue1 = m_pAmountFlow->f_inflow;
						//	dValue2 = m_pAmountFlow->f_outFlow;	
						//}
						if ( dValue1 != Con_doubleInvalid  && dValue2 != Con_doubleInvalid && dValue2 != 0)
							strResult.Format(_T("%.2f"),dValue1 /dValue2 );						
					}
					break;
				case 232:
					{
						FindItemOfReq(&reqRealTime, strReq, 3);	//Date
						dValue = GetKLine(pBusiness, reqRealTime.nDate, 7,reqRealTime.nSecurityID);
						if(fabs( Con_doubleInvalid-dValue) <0.00001)
							break;
						if(pBusiness->m_pSecurity->IsFundTradeInMarket()||(pBusiness->m_pSecurity->IsShanghai()&&pBusiness->m_pSecurity->IsStockB()))
						{
							strResult.Format(_T("%.3f"), dValue);
						}else
						{
							strResult.Format(_T("%.2f"), dValue);
						}
						break;
					}
					break;
				case 233:
					{
						FindItemOfReq(&reqRealTime, strReq, 3);	//Date
						dValue = GetKLine(pBusiness, reqRealTime.nDate, 8,reqRealTime.nSecurityID);
						if(fabs( Con_doubleInvalid-dValue) <0.00001)
							break;
						if(pBusiness->m_pSecurity->IsFundTradeInMarket()||(pBusiness->m_pSecurity->IsShanghai()&&pBusiness->m_pSecurity->IsStockB()))
						{
							strResult.Format(_T("%.3f"), dValue);
						}else
						{
							strResult.Format(_T("%.2f"), dValue);
						}
						break;
					}
					break;
				default:
					break;
				}


				//CString sLog;
				//sLog.Format(_T("wangzf|| request=%s,result=%s"),strReq,strResult);
				//Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);

				if(strResult.GetLength() > 0)
				{
					iterReqRealTime->second.strValue = strResult;
					//2008-06-12
					vdReq.FromString(strResult);
					iterReqRealTime->second.vdReq = vdReq;
				}
				iterReqRealTime->second.nCalculateTime = tmCurrent.GetTime();
			}

			tmCurrent = CTime::GetCurrentTime();
			if(lpParam_RealTimeIndCalc->pmapReqRealTime->size() > 0)
			{
				/*			CString sLog;
				sLog.Format(_T("\n--ʵʱָ���VBA�����������ʱ�䣺%4d-%02d-%02d %02d:%02d:%02d--\n"), 
				tmCurrent.GetYear(), tmCurrent.GetMonth(), tmCurrent.GetDay(), 
				tmCurrent.GetHour(), tmCurrent.GetMinute(), tmCurrent.GetSecond());
				Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);*/

				TRACE(_T("\n--ʵʱָ���VBA�����������ʱ�䣺%4d-%02d-%02d %02d:%02d:%02d--\n"), 
					tmCurrent.GetYear(), tmCurrent.GetMonth(), tmCurrent.GetDay(), 
					tmCurrent.GetHour(), tmCurrent.GetMinute(), tmCurrent.GetSecond());
			}
			if(nNewReqCnt > 0)
			{
				(*(lpParam_RealTimeIndCalc->lpnRefreshTimeRealTime)) = tmCurrent.GetTime();
			}
			tmLast = tmCurrent;

			singleLock.Unlock();
		}
		::WaitForSingleObject(m_hEvent_timer, 1000);
	}

	(*(lpParam_RealTimeIndCalc->lpbThreadTerm)) = TRUE;
	delete pBusiness;
	//Tx::Log::CLogRecorder::GetInstance()->WriteToLog(_T("Excel�˳�ʵʱ�������"));
	return 0;
}
CString TxIndicator::HisIndCalcProcSyn(LPCTSTR lpszRequest,Tx::Core::VariantData& vdReq)
{
	//Tx::Log::CLogRecorder::GetInstance()->WriteToLog(_T("Excel������ʷ����1"));
	CString strReq, strResult, strValue,strValue1;
	strReq = lpszRequest;
	if(strReq.GetLength()<=0)
		return _T("");
	LONG nIndustryID;
	Tx::Core::Table_Indicator subtable_ID, subtable_Year, subtable_Quarter;
	Tx::Data::TxShareData *pTxShareData;
	Tx::Data::TxFundShareData *pTxFundShareData;
	Tx::Data::BondNotChangeAmount *pCBondAmount;
	Tx::Data::IndexConstituentData *pIndexConstituentData;
	Tx::Data::SecurityQuotation *pSecurity;
	Tx::Data::FundNewInfo *pFundNewInfo;
	Tx::Data::BondNewInfo *pBondNewInfo;
	Tx::Data::BondCashFlowData *pBondCashFlowData;
	Tx::Data::FundNetValueData *pFundNetValueData;
	Tx::Data::FundGainData *pFundGainData;
	Tx::Data::StockBonusData *pStockBonusData;
	Tx::Data::FundBonusData *pFundBonusData;
	Tx::Data::BondGeneral *pBondGeneral;
	Tx::Business::TxWarrant warrant;
	Tx::Business::TxBond bond;
	Tx::Business::TxBusiness business;
	Tx::Data::TradeQuotation *pTradeQuotation;
	Tx::Data::TradeQuotationData *pTradeQuotationData;

	//added by zhangxs 20090218
	Balance_YH* pBalance_YH = NULL;
	DataFileNormal<blk_TxExFile_FileHead,Balance_YH>* m_pBalanceYHDataFile = new DataFileNormal<blk_TxExFile_FileHead,Balance_YH>;
	m_pBalanceYHDataFile->SetCheckLoadById(true);
	Balance_BX* pBalance_BX = NULL;
	DataFileNormal<blk_TxExFile_FileHead,Balance_BX>* m_pBalanceBXDataFile = new DataFileNormal<blk_TxExFile_FileHead,Balance_BX>;
	m_pBalanceBXDataFile->SetCheckLoadById(true);
	Balance_ZQ* pBalance_ZQ = NULL;
	DataFileNormal<blk_TxExFile_FileHead,Balance_ZQ>* m_pBalanceZQDataFile = new DataFileNormal<blk_TxExFile_FileHead,Balance_ZQ>;
	m_pBalanceZQDataFile->SetCheckLoadById(true);
	Profit_YH* pProfit_YH = NULL;
	DataFileNormal<blk_TxExFile_FileHead,Profit_YH>* m_pProfitYHDataFile = new DataFileNormal<blk_TxExFile_FileHead,Profit_YH>;
	m_pProfitYHDataFile->SetCheckLoadById(true);
	Profit_BX* pProfit_BX = NULL;
	DataFileNormal<blk_TxExFile_FileHead,Profit_BX>* m_pProfitBXDataFile = new DataFileNormal<blk_TxExFile_FileHead,Profit_BX>;
	m_pProfitBXDataFile->SetCheckLoadById(true);
	Profit_ZQ* pProfit_ZQ = NULL;
	DataFileNormal<blk_TxExFile_FileHead,Profit_ZQ>* m_pProfitZQDataFile = new DataFileNormal<blk_TxExFile_FileHead,Profit_ZQ>;
	m_pProfitZQDataFile->SetCheckLoadById(true);
	CashFlow_YH* pCashFlow_YH = NULL;
	DataFileNormal<blk_TxExFile_FileHead,CashFlow_YH>* m_pCashFlowYHDataFile = new DataFileNormal<blk_TxExFile_FileHead,CashFlow_YH>;
	m_pCashFlowYHDataFile->SetCheckLoadById(true);
	CashFlow_BX* pCashFlow_BX = NULL;
	DataFileNormal<blk_TxExFile_FileHead,CashFlow_BX>* m_pCashFlowBXDataFile = new DataFileNormal<blk_TxExFile_FileHead,CashFlow_BX>;
	m_pCashFlowBXDataFile->SetCheckLoadById(true);
	CashFlow_ZQ* pCashFlow_ZQ = NULL;
	DataFileNormal<blk_TxExFile_FileHead,CashFlow_ZQ>* m_pCashFlowZQDataFile = new DataFileNormal<blk_TxExFile_FileHead,CashFlow_ZQ>;
	m_pCashFlowZQDataFile->SetCheckLoadById(true);
	INT64 nAccountingID;
	INT nDate, nTradeDays, nInstitution, /*iTblIdx, */iVal, iIndexObjID, nDataCount, nSecurity1ID, iTransObjID, nStartDate, nEndDate, iRowNo, nCount;
	ReqMapIter iterReq;
	StrSetIter iterReqCalc;
	StrMapIter iterSheetReq, iterIndicatorReq;
	StrMMapIter iterNameCode;
	CTime tmStart, tmEnd, tmFrom, tmTo;
	CTimeSpan tsInterVal;
	DOUBLE dValue, dValue1, dValue2, dValue3,dValue4;
	UINT i, /*nCol, nRow,*/ array_Col[4] = {0, 1, 2, 3};
	//StrSet *psetReqCalc;
	vector <int> vInstitution, vDate;
	vector <UINT> vRow;
	Indicator_Dates indDates;
	Indicator_Date indDate;
	Indicator_ShareHolder indShareHolder;
	//��Ʊ���м۸�Ȩ֤ʣ�ഴ���������תծת�ɼ۸񡢿�תծ���ʡ���תծת��������ծȯ������Ϣ
	vector_IndDate vStockIssue, vWarrantDV, vCBondPrice, vCBondInterest, vCBondAmount, vBondIPOInfo;
	vector_IndDates vFundNAVGrowth;	//����ֵ������
	vector_IndShareHolder vTopTenShareHolder, vTopTenCShareHolder, vTopTenFundHolder, vTopTenCBondHolder;
	//byte cSubject;
	INT_Map mapIntInt;
	INT_Map_Iter iterIntInt;
	INTSTR_Map mapIntStr;
	INTSTR_Map_Iter iterIntStr;
	bool bLoaded = true;
	//����--------------------------
	int iSubItemID;					//---20080527
	HisIndCalc_Param *lpParam_HisIndCalc = &param_HisIndCalc;

	Tx::Business::TxBusiness *pBusiness = lpParam_HisIndCalc->pBusiness;
	//ReqMap *pmapReq = lpParam_HisIndCalc->pmapReq;
	
			tmStart = (CTime::GetCurrentTime()).GetTime();
			TRACE(_T("\n--��ʷָ���VBA���������ʼʱ�䣺%4d-%02d-%02d %02d:%02d:%02d--\n"), tmStart.GetYear(), tmStart.GetMonth(), tmStart.GetDay(), tmStart.GetHour(), tmStart.GetMinute(), tmStart.GetSecond());
			//-----------------------------wangzhy-20080526----------------------
			FindItemOfReq(&reqTXFunction, strReq);	//FuncID��SubItemID��IndicatorID
			switch(reqTXFunction.iFuncID)
			{
			case 1:		//֤ȯ����(���г���׺)
			case 2:		//֤ȯ����(���г���׺)
			case 3:		//֤ȯ���
			case 4:		//A�ɴ���
			case 5:		//B�ɴ���
			case 6:		//ע�����
			case 7:		//֤�����ҵ
			case 281:   //֤�����ҵ[��]
			case 8:		//������ҵ
			case 218:	//֤�����ҵ����
			case 280:   //֤�����ҵ����[��]  2013-04-19
			case 219:	//������ҵ����
			case 256:   //��֤��ҵ����   2012-10-10
			case 257:   //��֤��ҵ����
			case 258:   //������ҵ����
			case 259:   //������ҵ����
			case 260:   //������ҵ����
			case 261:   //������ҵ����
			case 262:   //�޳���ҵ����
			case 263:   //�޳���ҵ����
			case 10:	//��˾ȫ��
			case 11:	//IPO��������
			case 12:	//IPO��������
			case 13:	//��������
			case 14:	//��������
			case 15:	//���̼�
			case 16:	//ǰ�ռ�
			case 17:	//��߼�
			case 18:	//��ͼ�
			case 19:	//���̼�
			case 20:	//�վ���
			case 21:	//�ɽ���
			case 22:	//�ɽ����
			case 23:	//������
			case 24:	//�ۼƳɽ���
			case 25:	//�ۼƳɽ����
			case 26:	//�ۼƻ�����
			case 27:	//��ͨ��ֵ
			case 28:	//������ֵ(A)
			case 284:   //������ֵ(B)
			case 279:   //������ֵ(A+B)
			case 29:	//����ֵ
			case 30:	//��������ӽ�ָ���۸�(<=)�ĵ�һ������
			case 31:	//��������ӽ�ָ����������(<=)�ĵ�һ������
			case 32:	//�׶��Ƿ�
			case 321:	//�׶��Ƿ�(������ʼ���ڵ����Ƿ�)
			case 322://�����������Ƿ�
			case 33:	//������
			case 34:	//������
			case 144:	//���5�����̼�
			case 145:	//���5�����̼�
			case 146:	//���5��������
			case 147:	//���5��������
			case 148:	//ծȯ���5�����̼۶�Ӧ�ĵ���������
			case 149:	//ծȯ���5�����̼۶�Ӧ�ĵ���������
			case 35:	//��ͨ�ɱ�
			case 36:	//�ܹɱ�
			case 37:	//���ڹɱ�(A)
			case 285:   //���ڹɱ�(B)
			case 278:   //���ڹɱ�(A+B)
			case 38:	//�ֹ�����
			case 39:	//���й�
			case 99:	//����ͨ��
			case 40:	//��Ҫ����ָ���
			case 41:	//�ʲ���ծ��
			case 42:	//��������
			case 43:	//�ֽ�������
			case 55:	//��Ҫ����ָ��������ָ��
			case 75:	//�����ʲ���ֵ
			case 112:	//��תծ��Ӧ��Ʊ����
			case 44:	//��Ӫҵ������ҵ��ֲ�����Ŀ������
			case 45:	//��Ӫҵ������ҵ��ֲ�����Ŀ����
			case 46:	//�ʲ���ֵ׼��������
			case 47:	//�Ǿ������������Ŀ����
			case 48:	//�Ǿ������������Ŀ���
			case 49:	//Ӧ���ʿ������
			case 50:	//Ӧ���ʿ������
			case 51:	//������ñ���Ŀ����
			case 52:	//������ñ���Ŀ���
			case 56:	//���̼�[��Ȩ]
			case 57:	//ǰ�ռ�[��Ȩ]
			case 58:	//��߼�[��Ȩ]
			case 59:	//��ͼ�[��Ȩ]
			case 60:	//���̼�[��Ȩ]
			case 61:	//�վ���[��Ȩ]
			case 100:	//����������������Сֵ�ĵ�һ������
			case 101:	//�����������������ֵ�ĵ�һ������
			case 102:	//�����ڼ۸���Сֵ�ĵ�һ������
			case 103:	//�����ڼ۸����ֵ�ĵ�һ������
			case 117:	//ָ������������
			case 118:	//��Ʊ��֤�����ҵָ������
			case 119:	//��Ʊ��������ҵָ������
			case 120:	//�����ھ���
			case 121:	//��������ȷֺ��ܶ�
			case 133:	//�����ڷֺ��ܶ�
			case 134:	//��ȷֺ��ܶ�
			case 135:	//�����������Ϣ�ܶ�
			case 136:	//��������Ϣ�ܶ�
			case 137:	//�����Ϣ�ܶ�
			case 62:	//Ȩ֤-����������
			case 63:	//Ȩ֤-�����
			case 64:	//Ȩ֤-�ܸ˱���
			case 65:	//Ȩ֤-��Ȩ�۸�
			case 66:	//Ȩ֤��Ȩ��ֹ����
			case 67:	//Ȩ֤-ʣ����
			case 68:	//Ȩ֤-���ɴ���
			case 69:	//Ȩ֤-��Ȩ����
			case 71:	//��������
			case 122:	//������
			case 72:	//���������
			case 73:	//����λ��ֵ
			case 74:	//�����ۼƵ�λ��ֵ
			case 76:	//�����г��������ÿ��ݻ�������
			case 77:	//�����������
			case 78:	//����ʣ������
			case 79:	//�����г���������7���껯������
			case 81:	//IOPV(ETF����LOF����)
			case 286:   //IOPV(ETF����LOF����)
			case 82:	//�����й���
			case 83:	//�����г�����������
			case 84:	//������������
			case 85:	//ծȯ����
			case 123:	//ծȯ��ʽ
			case 87:	//ծȯ��ֵ
			case 89:	//ծȯ��ϢƵ��
			case 86:	//ծȯ������
			case 88:	//ծȯƱ������
			case 90:	//ծȯ��������
			case 91:	//ծȯ��Ϣ����
			case 92:	//ծȯ��������
			case 93:	//ծȯ������Ϣ��
			case 94:	//ծȯ���ڸ�Ϣ��
			case 95:	//ծȯӦ����Ϣ
			case 96:	//ծȯ��Ϣ����
			case 97:	//ծȯʣ������
			case 98:	//ծȯYTM
			case 104:	//ծȯ��������
			case 105:	//ծȯ͹��
			case 113:	//��תծ-ת��ƽ��
			case 114:	//��תծ-ת�������
			case 115:	//��תծ-ת���׼�
			case 116:	//��תծ-�׼������
			case 154:	//��������Ϣ
			case 220:	//�����ʲ���ծ��
			case 221:	//�����ʲ���ծ��
			case 222:	//֤ȯ�ʲ���ծ��
			case 223:	//���������
			case 224:	//���������
			case 225:	//֤ȯ�����
			case 226:	//�����ֽ�������
			case 227:	//�����ֽ�������
			case 228:	//֤ȯ��˾�ֽ�������
			case 229:	//���ڸ�ʽת��Text2Date
			case 230:	//���ڸ�ʽת��Date2Text
			case 232:
			case 233:
			case 234:
			case 237:	//����Ԥ��
			case 238:	//������
			case 242:	//ȡ�ùɱ���ϸ����
			case 243:	//֤���淶������
			case 249:   //ծȯ�׷��۸�
			case 250:   //ծȯ��׼ȯ������
			case 251:   //ծȯ���±�׼ȯ�����ʿ�ʼ������
			case 252:   //ծȯ���±�׼ȯ�����ʽ���������
			case 255:   //�����ڷݶ�
			case 264:   //�������ӻ�����
			case 265:   //ծȯ����
			case 266:   //ծȯ��������
			case 267:   //ծȯ��������
			case 268:   //��ծ��������
			case 269:   //��ծ������������
			case 270:   //��ծ������������
			case 271:   //ծȯ���¶Ҹ�������
			case 272:   //ծȨ�Ǽ���
			case 273:   //ծȯ�Ҹ���ʼ��
			case 274:   //ծȯ��һ�θ�Ϣ�Ľ�ֹ����
			case 275:   //ÿ��Ԫ�ֽ���
			case 276:   //��Ϣ��
			case 277:   //����Ȩ��ֵ
				//��ʼ��mapSheetReq(<strReq, strValue>)
				//mapSheetReq.insert(StrPair(strReq, _T("")));
				//mapSheetReqVariantData.insert(ReqVarPair(strReq,Tx::Core::VariantData(_T(""))));
				break;
			case 9:		//��Ʊ���м۸�
			case 70:	//Ȩ֤-����ʣ������
			case 106:	//��תծת�ɼ۸�
			case 107:	//��תծ����
			case 109:	//��תծת������
				//��ʼ��mapSheetReq(<strReq, strValue>)
				//mapSheetReq.insert(StrPair(strReq, _T("")));
				//mapSheetReqVariantData.insert(ReqVarPair(strReq,Tx::Core::VariantData(_T(""))));
				//�������������
				FindItemOfReq(&reqTXFunction, strReq, 16);	//Security��Date
				indDate.nRow = -1;
				indDate.dValue = 0;
				indDate.nSecurity = reqTXFunction.nSecurityID;
				indDate.nDate = reqTXFunction.nDate;
				switch(reqTXFunction.iFuncID)
				{
				case 9:
					vStockIssue.push_back(indDate);
					break;
				case 70:
					vWarrantDV.push_back(indDate);
					break;
				case 106:
					vCBondPrice.push_back(indDate);
					break;
				case 107:
					vCBondInterest.push_back(indDate);
					break;
				case 109:
					vCBondAmount.push_back(indDate);
					break;
				default:
					break;
				}
				break;
			case 80:	//����ֵ������
				//��ʼ��mapSheetReq(<strReq, strValue>)
				//mapSheetReq.insert(StrPair(strReq, _T("")));
				//mapSheetReqVariantData.insert(ReqVarPair(strReq,Tx::Core::VariantData(_T(""))));
				//�������������
				FindItemOfReq(&reqTXFunction, strReq, 15);	//Security��StartDate��EndDate
				indDates.nRow = -1;
				indDates.dValue = 0;
				indDates.nSecurity = reqTXFunction.nSecurityID;
				indDates.nFirstDate = reqTXFunction.nStartDate;
				indDates.nSecondDate = reqTXFunction.nEndDate;
				vFundNAVGrowth.push_back(indDates);
				break;
			case 110:	//��תծ��������
				//��ʼ��mapSheetReq(<strReq, strValue>)
				//mapSheetReq.insert(StrPair(strReq, _T("")));
				//mapSheetReqVariantData.insert(ReqVarPair(strReq,Tx::Core::VariantData(_T(""))));
				//�������������
				FindItemOfReq(&reqTXFunction, strReq, 1);	//Security
				indDate.nRow = -1;
				indDate.dValue = 0;
				indDate.nDate = 0;
				indDate.nSecurity = reqTXFunction.nSecurityID;
				vBondIPOInfo.push_back(indDate);
				break;
			case 108:	//��תծ-����
				//��ʼ��mapSheetReq(<strReq, strValue>)
				//mapSheetReq.insert(StrPair(strReq, _T("")));
				//mapSheetReqVariantData.insert(ReqVarPair(strReq,Tx::Core::VariantData(_T(""))));
				if(reqTXFunction.iSubItemID == 1)
				{	//������
					//�������������
					FindItemOfReq(&reqTXFunction, strReq, 1);	//Security
					indDate.nRow = -1;
					indDate.dValue = 0;
					indDate.nDate = 0;
					indDate.nSecurity = reqTXFunction.nSecurityID;
					vBondIPOInfo.push_back(indDate);
				}
				break;
			case 111:	//��תծ������Ϣ
				//��ʼ��mapSheetReq(<strReq, strValue>)
				//mapSheetReq.insert(StrPair(strReq, _T("")));
				//mapSheetReqVariantData.insert(ReqVarPair(strReq,Tx::Core::VariantData(_T(""))));
				if(reqTXFunction.iSubItemID != 2)
				{	//������id֮���ָ��
					//�������������
					FindItemOfReq(&reqTXFunction, strReq, 1);	//Security
					indDate.nRow = -1;
					indDate.dValue = 0;
					indDate.nDate = 0;
					indDate.nSecurity = reqTXFunction.nSecurityID;
					vBondIPOInfo.push_back(indDate);
				}
				break;
			case 124:	//ʮ��ɶ�����
			case 125:	//ʮ��ɶ��ֹ���
			case 126:	//ʮ��ɶ��ֹɱ���
			case 127:	//ʮ��ɶ��ֹ�����
			case 128:	//ʮ����ͨ�ɶ�����
			case 129:	//ʮ����ͨ�ɶ��ֹ���
			case 130:	//ʮ����ͨ�ɶ��ܹɱ���
			case 131:	//ʮ����ͨ�ɶ���ͨ����
			case 132:	//ʮ����ͨ�ɶ��ֹ�����
			case 138:	//���ʽ����ʮ�����������
			case 139:	//���ʽ����ʮ������˻���ݶ�
			case 140:	//���ʽ����ʮ������˻���ݶ����
			case 141:	//��תծʮ�����������
			case 142:	//��תծʮ�������ծȯ���
			case 143:	//��תծʮ�������ծȯ������
				//��ʼ��mapSheetReq(<strReq, strValue>)
				//mapSheetReq.insert(StrPair(strReq, _T("")));
				//mapSheetReqVariantData.insert(ReqVarPair(strReq,Tx::Core::VariantData(_T(""))));
				FindItemOfReq(&reqTXFunction, strReq, 19);	//Security��Date��iHolderNo
				indShareHolder.nRow = -1;
				indShareHolder.iHolerNo = reqTXFunction.iHolderNo;
				indShareHolder.nDate = reqTXFunction.nDate;
				indShareHolder.nSecurity = (INT)(reqTXFunction.nSecurityID);
				switch(reqTXFunction.iFuncID)
				{
				case 124:	//ʮ��ɶ�����
				case 125:	//ʮ��ɶ��ֹ���
				case 126:	//ʮ��ɶ��ֹɱ���
				case 127:	//ʮ��ɶ��ֹ�����
					vTopTenShareHolder.push_back(indShareHolder);
					break;
				case 128:	//ʮ����ͨ�ɶ�����
				case 129:	//ʮ����ͨ�ɶ��ֹ���
				case 130:	//ʮ����ͨ�ɶ��ܹɱ���
				case 131:	//ʮ����ͨ�ɶ���ͨ����
				case 132:	//ʮ����ͨ�ɶ��ֹ�����
					vTopTenCShareHolder.push_back(indShareHolder);
					break;
				case 138:	//���ʽ����ʮ�����������
				case 139:	//���ʽ����ʮ������˻���ݶ�
				case 140:	//���ʽ����ʮ������˻���ݶ����
					vTopTenFundHolder.push_back(indShareHolder);
					break;
				default:	//��תծʮ����������ơ�ծȯ��ծȯ������
					vTopTenCBondHolder.push_back(indShareHolder);
					break;
				}
				break;
			case 198:
			case 206:
				FindItemOfReq(&reqTXFunction, strReq,20);
				break;
			default:
				//mapSheetReq.insert(StrPair(strReq, _T("")));
				//mapSheetReqVariantData.insert(ReqVarPair(strReq,Tx::Core::VariantData(_T(""))));
				break;
			}

			//�����Ʊ���м۸�
			if(vStockIssue.size() > 0)
			{
				SetDateIndicator(&vStockIssue, 0);
			}

			//����Ȩ֤ʣ�ഴ�����
			if(vWarrantDV.size() > 0)
			{
				SetDateIndicator(&vWarrantDV, 1);
			}

			//�����תծת�ɼ۸�
			if(vCBondPrice.size() > 0)
			{
				SetDateIndicator(&vCBondPrice, 2);
			}

			//�����תծ����
			if(vCBondInterest.size() > 0)
			{
				SetDateIndicator(&vCBondInterest, 3);
			}

			//�����תծת������
			if(vCBondAmount.size() > 0)
			{
				SetDateIndicator(&vCBondAmount, 4);
			}

			//����ծȯ������Ϣ
			if(vBondIPOInfo.size() > 0)
			{
				SetDateIndicator(&vBondIPOInfo, 5);
			}

			//�������ֵ������
			if(vFundNAVGrowth.size() > 0)
			{
				SetDatesIndicator(&vFundNAVGrowth, 0);
			}

			//map <CString, Tx::Core::VariantData>::iterator iterVdReq = mapSheetReqVariantData.begin();
			//iterSheetReq = mapSheetReq.begin();

			{
				//dValue = Con_doubleInvalid;
				//iterSheetReq = mapSheetReq.find(strReq);

				strResult = _T("-");
				FindItemOfReq(&reqTXFunction, strReq, 2);	//FuncID��SubItemID��Security
				FindItemOfReq( &reqTXFunction, strReq, 26 );
				if(reqTXFunction.iFuncID == 1 || reqTXFunction.iFuncID == 2)
				{
					//�ַ���
					vdReq.data_type = dtype_val_string;

					//֤ȯ����
					if(strlen(reqTXFunction.cName) > 0)
					{
						strValue.Format(_T("%s"), reqTXFunction.cName);
						strValue.Trim();

						SecurityNamePred pred(strValue);
						StrMMapIter iter = find_if(mmapNameCode.begin(),mmapNameCode.end(),pred);
						int matchCount = count_if(mmapNameCode.begin(),mmapNameCode.end(),pred);

						for(int i=0;i<matchCount;iter++,i++)
						{
							iTransObjID = (INT)GetSecurityId(iter->second);
							if(iTransObjID > 0 && business.GetSecurityNow((LONG)iTransObjID))
							{
								if( (reqTXFunction.iSubItemID == 0 && business.m_pSecurity->IsCN_Market()) ||
								    (reqTXFunction.iSubItemID == 1 && business.m_pSecurity->IsHongkong()) )
								{
									if( (reqTXFunction.iIndexFlg == 0 && !business.m_pSecurity->IsIndex()) ||
										(reqTXFunction.iIndexFlg != 0 && business.m_pSecurity->IsIndex()))
									{
										strResult = iter->second;
										break;
									}
								}
							}
						}
					}

					if(strResult.GetLength() > 0)
					{
						if(reqTXFunction.iFuncID == 1)
						{	//֤ȯ����(���г���׺)
							strResult = strResult.Left(strResult.GetLength() - 3);
						}
					}
				}else if(pBusiness->GetSecurityNow(reqTXFunction.nSecurityID))
				{
					if(pBusiness->m_pSecurity != NULL)
					{
						switch(reqTXFunction.iFuncID)
						{
						case 3:		//֤ȯ���
							vdReq.data_type = dtype_val_string;
							strResult = pBusiness->m_pSecurity->GetName();
							break;
						case 4:		//B�ɴ���-->A�ɴ���
							vdReq.data_type = dtype_val_string;
							if(!pBusiness->m_pSecurity->IsStockB())
							{
								break;
							}
							mapIntInt.clear();
							Tx::Data::TypeMapManage::GetInstance()->GetTypeMapITI(TYPE_STOCK_TRANSOBJECT_BA, mapIntInt);
							iterIntInt = mapIntInt.find((INT)(reqTXFunction.nSecurityID));
							if(iterIntInt != mapIntInt.end())
							{
								if(business.GetSecurityNow(iterIntInt->second) != NULL)
								{
									strResult = business.m_pSecurity->GetCode();
								}
							}
							break;
						case 5:		//A�ɴ���-->B�ɴ���
							vdReq.data_type = dtype_val_string;
							if(!pBusiness->m_pSecurity->IsStockA())
							{
								break;
							}
							mapIntInt.clear();
							Tx::Data::TypeMapManage::GetInstance()->GetTypeMapITI(TYPE_STOCK_TRANSOBJECT_AB, mapIntInt);
							iterIntInt = mapIntInt.find((INT)(reqTXFunction.nSecurityID));
							if(iterIntInt != mapIntInt.end())
							{
								if(business.GetSecurityNow(iterIntInt->second) != NULL)
								{
									strResult = business.m_pSecurity->GetCode();
								}
							}
							break;
						case 6:		//ע�����
							vdReq.data_type = dtype_val_string;
							strResult = pBusiness->m_pSecurity->GetRegisterProvance();
							break;
						case 10:	//����ȫ��
						case 72:	//���������
							vdReq.data_type = dtype_val_string;
							nInstitution = (INT)pBusiness->m_pSecurity->GetInstitutionId();
							if(nInstitution > 0)
							{
								mapIntStr.clear();
								Tx::Data::TypeMapManage::GetInstance()->GetTypeMap(TYPE_INSTITUTION_CHINALONGNAME, mapIntStr);
								iterIntStr = mapIntStr.find(nInstitution);
								if(iterIntStr != mapIntStr.end())
								{
									strValue = iterIntStr->second;
									if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
									{
										strResult = strValue;
									}
								}
							}
							break;
						case 7:		//֤�����ҵ
							{
								vdReq.data_type = dtype_val_string;
								FindItemOfReq(&reqTXFunction, strReq, 22);	//��ҵ�㼶
								nIndustryID = pBusiness->m_pSecurity->GetCSRC_IndustryId();
								iVal = 0;
								std::vector<CString> vTemp;
								vTemp.clear();
								while(nIndustryID > 0)
								{
									strValue = TypeMapManage::GetInstance()->GetDatByID(TYPE_CSRC_INDUSTRY_NAME, (INT)nIndustryID);
									vTemp.push_back( strValue );
									nIndustryID = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRY_PARENT, (INT)nIndustryID);
								}
								if ( reqTXFunction.nIndustryLevel == 0 )
								{
									for( std::vector<CString>::iterator iter = vTemp.begin(); iter != vTemp.end(); ++iter )
									{
										if ( strResult.GetLength() > 0 )
											strResult = *iter + _T(" - ") + strResult;
										else
											strResult = *iter;
									}	
								}
								else
								{
									int nCount = (int)vTemp.size();
									if(nCount < 1)
										strResult = _T("-");
									else
									{
										if ( reqTXFunction.nIndustryLevel >= nCount)
											strResult = *vTemp.begin();
										else
											strResult = vTemp[nCount-reqTXFunction.nIndustryLevel];
									}
								}
							}

							break;
						case 281:   
							break;
						case 8:		//������ҵ
							vdReq.data_type = dtype_val_string;
							FindItemOfReq(&reqTXFunction, strReq, 22);	//��ҵ�㼶
							if ( reqTXFunction.nIndustryLevel == 1 )
								reqTXFunction.nIndustryLevel = 2;
							else if ( reqTXFunction.nIndustryLevel == 2 )
								reqTXFunction.nIndustryLevel  = 1;
							nIndustryID = pBusiness->m_pSecurity->GetTxSec_IndustryId();
							iVal = 0;
							while(nIndustryID > 0)
							{
								strValue = TypeMapManage::GetInstance()->GetDatByID(TYPE_TX_INDUSTRY_NAME, (INT)nIndustryID);
								if(reqTXFunction.nIndustryLevel == 0)
								{	//����ҵ�㼶
									if(iVal == 0)
									{
										strResult = strValue;
									}else
									{
										strResult = strValue + _T("-") + strResult;
									}
								}else
								{	//����ҵ�㼶
									strResult = strValue;
									if(reqTXFunction.nIndustryLevel == iVal + 1)
									{
										break;
									}
								}
								iVal ++;
								nIndustryID = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRY_PARENT, (INT)nIndustryID);
							}
							break;
						case 257:  //��֤
						case 259:  //����
						case 261:  //����
						case 263:  //�޳�
							break;
						case 218:		//֤�����ҵ����
							{
								vdReq.data_type = dtype_val_string;
								FindItemOfReq(&reqTXFunction, strReq, 22);	//��ҵ�㼶
								//strResult = pBusiness->m_pSecurity->GetCSRCIndustryCode(reqTXFunction.nIndustryLevel);
								int iIndustryId = 0;
								iIndustryId = pBusiness->m_pSecurity->GetCSRC_IndustryId();

								//step1 get level1id
								int l1id = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRYID_TO_LEVEL1ID,iIndustryId);
								//step2 get level2id
								int l2id = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRYID_TO_LEVEL2ID,iIndustryId);
								//step3
								//get level1name
								CString sl1Name = TypeMapManage::GetInstance()->GetDatByID(TYPE_INDUSTRYID_TO_CODE,l1id);
								//step4
								//get level2name
								CString sl2Name = TypeMapManage::GetInstance()->GetDatByID(TYPE_INDUSTRYID_TO_CODE,l2id);

								int lid = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRYID_TO_LEVEL,iIndustryId);
								CString sl3Name = _T("");
								CString sl4Name = _T("");
								if( lid == 3)
									sl3Name = TypeMapManage::GetInstance()->GetDatByID(TYPE_INDUSTRYID_TO_CODE,iIndustryId);
								if( lid == 4)
									sl4Name = TypeMapManage::GetInstance()->GetDatByID(TYPE_INDUSTRYID_TO_CODE,iIndustryId);
								switch(reqTXFunction.nIndustryLevel)
								{
								case 0:
									if(lid == 4)
										strResult = sl4Name;
									else if(lid == 2)
										strResult = sl2Name;
									else if(lid == 3)
										strResult = sl3Name;
									else
										strResult = sl1Name;
									break;
								case 1:
									strResult = sl1Name;
									break;
								case 2:
									strResult = sl2Name;
									break;
								case 3:
									strResult = sl3Name;
									break;
								case 4:
									strResult = sl4Name;
									break;
								default:
									strResult = _T("-");
									break;
								}
							}
							break;
						case 280:       //֤�����ҵ����[��]
							break;
						case 219:		//������ҵ����
							{
								vdReq.data_type = dtype_val_string;
								FindItemOfReq(&reqTXFunction, strReq, 22);	//��ҵ�㼶
								int iIndustryId = 0;
								iIndustryId = pBusiness->m_pSecurity->GetTxSec_IndustryId();

								//step1 get level1id
								int l1id = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRYID_TO_LEVEL1ID,iIndustryId);
								//step2 get level2id
								int l2id = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRYID_TO_LEVEL2ID,iIndustryId);
								//step3
								//get level1name
								CString sl1Name = TypeMapManage::GetInstance()->GetDatByID(TYPE_INDUSTRYID_TO_CODE,l1id);
								//step4
								//get level2name
								CString sl2Name = TypeMapManage::GetInstance()->GetDatByID(TYPE_INDUSTRYID_TO_CODE,l2id);

								switch(reqTXFunction.nIndustryLevel)
								{
								case 0:
									if(l2id < 0)
										strResult = sl1Name;
									else
										strResult = sl2Name;
									break;
								case 1:
									strResult = sl1Name;
									break;
								case 2:
									strResult = sl2Name;
									break;
								default:
									strResult = _T("-");
									break;
								}
							}
							break;
						case 256:  //��֤
						case 258:  //����
						case 260:  //����
						case 262:  //�޳�
							break;
						case 9:		//���м۸�
							vdReq.data_type = dtype_float;
							//--����һ��򿪣�����������һЩû���׷�--20080703
							dValue = pBusiness->m_pSecurity->GetFirstDateIssuePrice();	//�״η��м۸�
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							dValue = GetDateIndicator_Value((INT)(reqTXFunction.nSecurityID), reqTXFunction.nDate, &vStockIssue);
							if(dValue > 0)
							{
								strResult.Format(_T("%.2f"), dValue);
							}
							break;
						case 11:	//IPO��������
							vdReq.data_type = dtype_int4;
							nDate = pBusiness->m_pSecurity->GetIPOListedDate();
							if(nDate > 0)
							{
								//�޸�Ϊ8λ��--20080708--
								strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
							}
							break;
						case 12:	//IPO��������
							vdReq.data_type = dtype_int4;
							nDate = pBusiness->m_pSecurity->GetIPOIssueDate();
							if(nDate > 0)
							{
								//�޸�Ϊ8λ��--20080708--
								strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
							}
							break;
						case 13:	//��������
							vdReq.data_type = dtype_int4;
							FindItemOfReq(&reqTXFunction, strReq, 6);	//Date��OffsetDays
							nDate = GetTradeDateByOffset(pBusiness, reqTXFunction.nDate, reqTXFunction.nOffsetDays);
							if(nDate > 0)
							{
								//�޸�Ϊ8λ��--20080708--
								strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
							}
							break;
						case 14:	//��������
							vdReq.data_type = dtype_int4;
							FindItemOfReq(&reqTXFunction, strReq, 5);	//StartDate��EndDate
							nTradeDays = GetTradeDays(pBusiness, reqTXFunction.nStartDate, reqTXFunction.nEndDate);
							strResult.Format(_T("%d"), nTradeDays);
							break;
						case 15:	//���̼�
							vdReq.data_type = dtype_float;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							//dValue = GetKLine(pBusiness, reqTXFunction.nDate, 0);
							dValue = GetKLineEx(pBusiness, reqTXFunction.nDate, 0,reqTXFunction.nSecurityID);
							if(dValue <= 0)
							{
								break;
							}
							//bug:11640    2012-09-25
							//if(pBusiness->m_pSecurity->IsFundTradeInMarket())
							if(pBusiness->m_pSecurity->IsFundTradeInMarket() || pBusiness->m_pSecurity->IsFund_Bond() || pBusiness->m_pSecurity->IsFund_Intermix() || 
								pBusiness->m_pSecurity->IsFund_ETF() || pBusiness->m_pSecurity->IsFund_LOF() || pBusiness->m_pSecurity->IsFund_QDII() || 
								pBusiness->m_pSecurity->IsFund_Stock() || (pBusiness->m_pSecurity->IsStockB() && pBusiness->m_pSecurity->IsShanghai())||
								(pBusiness->m_pSecurity->IsBond_National() && pBusiness->m_pSecurity->IsShenzhen()) || pBusiness->m_pSecurity->IsBond_Change() ||
								pBusiness->m_pSecurity->IsWarrant() || pBusiness->m_pSecurity->IsRepurchase()
							  )
							{
								strResult.Format(_T("%.3f"), dValue);
								/*		CString strlog;
								strlog.Format( _T("Excel���������̼ۡ�--cost 16 = %s"), strResult );
								Tx::Log::CLogRecorder::GetInstance()->WriteToLog(strlog);*/
							}else
							{
								strResult.Format(_T("%.2f"), dValue);
						/*		CString strlog;
								strlog.Format( _T("Excel���������̼ۡ�--cost 16 = %s"), strResult );
								Tx::Log::CLogRecorder::GetInstance()->WriteToLog(strlog);*/
							}
							break;
						case 56:	//���̼�[��Ȩ]
							vdReq.data_type = dtype_float;
							FindItemOfReq(&reqTXFunction, strReq, 9);	//Date��StartDate��EndDate����Ȩ����
							//dValue1 = GetKLine(pBusiness, reqTXFunction.nDate, 0);
							dValue1 = GetKLineEx(pBusiness, reqTXFunction.nDate, 0,reqTXFunction.nSecurityID);
							if(dValue1 <= 0)
							{
								break;
							}
							if(reqTXFunction.iIRType == 0)
							{	//ǰ��Ȩ
								//dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nDate, reqTXFunction.nEndDate, true);
								dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nEndDate, true);
							}else
							{	//��Ȩ
								if ( reqTXFunction.nStartDate == 0 )
									reqTXFunction.nStartDate = pBusiness->m_pSecurity->GetTradeDateByIndex(0);	
								//dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nDate, false);
								dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nEndDate, false);
							}
							//if(pBusiness->m_pSecurity->IsFundTradeInMarket())
							//bug:11640  2012-09-25
							if(pBusiness->m_pSecurity->IsFundTradeInMarket() || pBusiness->m_pSecurity->IsFund_Bond() || pBusiness->m_pSecurity->IsFund_Intermix() || 
								pBusiness->m_pSecurity->IsFund_ETF() || pBusiness->m_pSecurity->IsFund_LOF() || pBusiness->m_pSecurity->IsFund_QDII() || 
								pBusiness->m_pSecurity->IsFund_Stock() || (pBusiness->m_pSecurity->IsStockB() && pBusiness->m_pSecurity->IsShanghai())||
								(pBusiness->m_pSecurity->IsBond_National() && pBusiness->m_pSecurity->IsShenzhen()) || pBusiness->m_pSecurity->IsBond_Change() ||
								pBusiness->m_pSecurity->IsWarrant() || pBusiness->m_pSecurity->IsRepurchase())
							{
								strResult.Format(_T("%.3f"), dValue1 * dValue2);
							}else
							{
								strResult.Format(_T("%.2f"), dValue1 * dValue2);
							}
							break;
						case 16:	//ǰ�ռ�
							vdReq.data_type = dtype_float;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							//dValue = GetKLine(pBusiness, reqTXFunction.nDate, 1);
							dValue = GetKLineEx(pBusiness, reqTXFunction.nDate, 1,reqTXFunction.nSecurityID);
							if(dValue <= 0)
							{
								break;
							}
							//if(pBusiness->m_pSecurity->IsFundTradeInMarket())
							//bug:11640  2012-09-25
							
							if(pBusiness->m_pSecurity->IsFundTradeInMarket() || pBusiness->m_pSecurity->IsFund_Bond() || pBusiness->m_pSecurity->IsFund_Intermix() || 
								pBusiness->m_pSecurity->IsFund_ETF() || pBusiness->m_pSecurity->IsFund_LOF() || pBusiness->m_pSecurity->IsFund_QDII() || 
								pBusiness->m_pSecurity->IsFund_Stock() || (pBusiness->m_pSecurity->IsStockB() && pBusiness->m_pSecurity->IsShanghai())||
								(pBusiness->m_pSecurity->IsBond_National() && pBusiness->m_pSecurity->IsShenzhen()) || pBusiness->m_pSecurity->IsBond_Change() ||
								pBusiness->m_pSecurity->IsWarrant() || pBusiness->m_pSecurity->IsRepurchase())
							{
								strResult.Format(_T("%.3f"), dValue);
								/*				CString strlog;
								strlog.Format( _T("Excel������ǰ�ռۡ�--cost 16 = %s"), strResult );
								Tx::Log::CLogRecorder::GetInstance()->WriteToLog(strlog);*/
							}else
							{
								strResult.Format(_T("%.2f"), dValue);
					/*			CString strlog;
								strlog.Format( _T("Excel������ǰ�ռۡ�--cost 16 = %s"), strResult );
								Tx::Log::CLogRecorder::GetInstance()->WriteToLog(strlog);*/
							}
							break;
						case 57:	//ǰ�ռ�[��Ȩ]
							vdReq.data_type = dtype_float;
							FindItemOfReq(&reqTXFunction, strReq, 9);	//Date��StartDate��EndDate����Ȩ����
							//dValue1 = GetKLine(pBusiness, reqTXFunction.nDate, 1);
							dValue1 = GetKLineEx(pBusiness, reqTXFunction.nDate, 1,reqTXFunction.nSecurityID);
							if(dValue1 <= 0)
							{
								break;
							}
							if(reqTXFunction.iIRType == 0)
							{	//ǰ��Ȩ
								//dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nDate, reqTXFunction.nEndDate, true);
								dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nEndDate, true);
							}else
							{	//��Ȩ
								if ( reqTXFunction.nStartDate == 0 )
									reqTXFunction.nStartDate = pBusiness->m_pSecurity->GetTradeDateByIndex(0);	
								//dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nDate, false);
								dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nEndDate, false);
							}
							//if(pBusiness->m_pSecurity->IsFundTradeInMarket())
							//bug:11640  2012-09-25
							if(pBusiness->m_pSecurity->IsFundTradeInMarket() || pBusiness->m_pSecurity->IsFund_Bond() || pBusiness->m_pSecurity->IsFund_Intermix() || 
								pBusiness->m_pSecurity->IsFund_ETF() || pBusiness->m_pSecurity->IsFund_LOF() || pBusiness->m_pSecurity->IsFund_QDII() || 
								pBusiness->m_pSecurity->IsFund_Stock() || (pBusiness->m_pSecurity->IsStockB() && pBusiness->m_pSecurity->IsShanghai())||
								(pBusiness->m_pSecurity->IsBond_National() && pBusiness->m_pSecurity->IsShenzhen()) || pBusiness->m_pSecurity->IsBond_Change() ||
								pBusiness->m_pSecurity->IsWarrant() || pBusiness->m_pSecurity->IsRepurchase())
							{
								strResult.Format(_T("%.3f"), dValue1 * dValue2);
							}else
							{
								strResult.Format(_T("%.2f"), dValue1 * dValue2);
							}
							break;
						case 17:	//��߼�
							vdReq.data_type = dtype_float;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							//dValue = GetKLine(pBusiness, reqTXFunction.nDate, 2);
							dValue = GetKLineEx(pBusiness, reqTXFunction.nDate, 2,reqTXFunction.nSecurityID);
							if(dValue <= 0)
							{
								break;
							}
							//if(pBusiness->m_pSecurity->IsFundTradeInMarket())
							//bug:11640  2012-09-25
							if(pBusiness->m_pSecurity->IsFundTradeInMarket() || pBusiness->m_pSecurity->IsFund_Bond() || pBusiness->m_pSecurity->IsFund_Intermix() || 
								pBusiness->m_pSecurity->IsFund_ETF() || pBusiness->m_pSecurity->IsFund_LOF() || pBusiness->m_pSecurity->IsFund_QDII() || 
								pBusiness->m_pSecurity->IsFund_Stock() || (pBusiness->m_pSecurity->IsStockB() && pBusiness->m_pSecurity->IsShanghai())||
								(pBusiness->m_pSecurity->IsBond_National() && pBusiness->m_pSecurity->IsShenzhen()) || pBusiness->m_pSecurity->IsBond_Change() ||
								pBusiness->m_pSecurity->IsWarrant() || pBusiness->m_pSecurity->IsRepurchase())
							{
								strResult.Format(_T("%.3f"), dValue);
								/*		CString strlog;
								strlog.Format( _T("Excel��������߼ۡ�--cost 17 = %s"), strResult );
								Tx::Log::CLogRecorder::GetInstance()->WriteToLog(strlog);*/
							}else
							{
								strResult.Format(_T("%.2f"), dValue);
							/*	CString strlog;
								strlog.Format( _T("Excel��������߼ۡ�--cost 17 = %s"), strResult );
								Tx::Log::CLogRecorder::GetInstance()->WriteToLog(strlog);*/
							}
							break;
						case 58:	//��߼�[��Ȩ]
							vdReq.data_type = dtype_float;
							FindItemOfReq(&reqTXFunction, strReq, 9);	//Date��StartDate��EndDate����Ȩ����
							//dValue1 = GetKLine(pBusiness, reqTXFunction.nDate, 2);
							dValue1 = GetKLineEx(pBusiness, reqTXFunction.nDate, 2,reqTXFunction.nSecurityID);
							if(dValue1 <= 0)
							{
								break;
							}
							if(reqTXFunction.iIRType == 0)
							{	//ǰ��Ȩ
								//dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nDate, reqTXFunction.nEndDate, true);
								dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nEndDate, true);
							}else
							{	//��Ȩ
								if ( reqTXFunction.nStartDate == 0 )
									reqTXFunction.nStartDate = pBusiness->m_pSecurity->GetTradeDateByIndex(0);	
								//dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nDate, false);
								dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nEndDate, false);
							}
							//bug:11640  2012-09-25
							//if(pBusiness->m_pSecurity->IsFundTradeInMarket()||(pBusiness->m_pSecurity->IsShanghai()&&pBusiness->m_pSecurity->IsStockB()))
							if(pBusiness->m_pSecurity->IsFundTradeInMarket() || pBusiness->m_pSecurity->IsFund_Bond() || pBusiness->m_pSecurity->IsFund_Intermix() || 
								pBusiness->m_pSecurity->IsFund_ETF() || pBusiness->m_pSecurity->IsFund_LOF() || pBusiness->m_pSecurity->IsFund_QDII() || 
								pBusiness->m_pSecurity->IsFund_Stock() || (pBusiness->m_pSecurity->IsStockB() && pBusiness->m_pSecurity->IsShanghai())||
								(pBusiness->m_pSecurity->IsBond_National() && pBusiness->m_pSecurity->IsShenzhen()) || pBusiness->m_pSecurity->IsBond_Change() ||
								pBusiness->m_pSecurity->IsWarrant() || pBusiness->m_pSecurity->IsRepurchase())
							{
								strResult.Format(_T("%.3f"), dValue1 * dValue2);
							}else
							{
								strResult.Format(_T("%.2f"), dValue1 * dValue2);
							}
							break;
						case 18:	//��ͼ�
							vdReq.data_type = dtype_float;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							//dValue = GetKLine(pBusiness, reqTXFunction.nDate, 3);
							dValue = GetKLineEx(pBusiness, reqTXFunction.nDate, 3,reqTXFunction.nSecurityID);
							if(dValue <= 0)
							{
								break;
							}
							//if(pBusiness->m_pSecurity->IsFundTradeInMarket())
							//bug:11640  2012-09-25
							if(pBusiness->m_pSecurity->IsFundTradeInMarket() || pBusiness->m_pSecurity->IsFund_Bond() || pBusiness->m_pSecurity->IsFund_Intermix() || 
								pBusiness->m_pSecurity->IsFund_ETF() || pBusiness->m_pSecurity->IsFund_LOF() || pBusiness->m_pSecurity->IsFund_QDII() || 
								pBusiness->m_pSecurity->IsFund_Stock() || (pBusiness->m_pSecurity->IsStockB() && pBusiness->m_pSecurity->IsShanghai())||
								(pBusiness->m_pSecurity->IsBond_National() && pBusiness->m_pSecurity->IsShenzhen()) || pBusiness->m_pSecurity->IsBond_Change() ||
								pBusiness->m_pSecurity->IsWarrant() || pBusiness->m_pSecurity->IsRepurchase())
							{
								strResult.Format(_T("%.3f"), dValue);
								/*		CString strlog;
								strlog.Format( _T("Excel��������ͼۡ�--cost 18 = %s"), strResult );
								Tx::Log::CLogRecorder::GetInstance()->WriteToLog(strlog);*/
							}else
							{
								strResult.Format(_T("%.2f"), dValue);
				/*				CString strlog;
								strlog.Format( _T("Excel��������ͼۡ�--cost 18 = %s"), strResult );
								Tx::Log::CLogRecorder::GetInstance()->WriteToLog(strlog);*/
							}
							break;
						case 59:	//��ͼ�[��Ȩ]
							vdReq.data_type = dtype_float;
							FindItemOfReq(&reqTXFunction, strReq, 9);	//Date��StartDate��EndDate����Ȩ����
							//dValue1 = GetKLine(pBusiness, reqTXFunction.nDate, 3);
							dValue1 = GetKLineEx(pBusiness, reqTXFunction.nDate, 3,reqTXFunction.nSecurityID);
							if(dValue1 <= 0)
							{
								break;
							}
							if(reqTXFunction.iIRType == 0)
							{	//ǰ��Ȩ
								//dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nDate, reqTXFunction.nEndDate, true);
								dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nEndDate, true);
							}else
							{	//��Ȩ
								if ( reqTXFunction.nStartDate == 0 )
									reqTXFunction.nStartDate = pBusiness->m_pSecurity->GetTradeDateByIndex(0);	
								//dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nDate, false);
								dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nEndDate, false);
							}
							//if(pBusiness->m_pSecurity->IsFundTradeInMarket())
							//bug:11640  2012-09-25
							if(pBusiness->m_pSecurity->IsFundTradeInMarket() || pBusiness->m_pSecurity->IsFund_Bond() || pBusiness->m_pSecurity->IsFund_Intermix() || 
								pBusiness->m_pSecurity->IsFund_ETF() || pBusiness->m_pSecurity->IsFund_LOF() || pBusiness->m_pSecurity->IsFund_QDII() || 
								pBusiness->m_pSecurity->IsFund_Stock() || (pBusiness->m_pSecurity->IsStockB() && pBusiness->m_pSecurity->IsShanghai())||
								(pBusiness->m_pSecurity->IsBond_National() && pBusiness->m_pSecurity->IsShenzhen()) || pBusiness->m_pSecurity->IsBond_Change() ||
								pBusiness->m_pSecurity->IsWarrant() || pBusiness->m_pSecurity->IsRepurchase())
							{
								strResult.Format(_T("%.3f"), dValue1 * dValue2);
							}else
							{
								strResult.Format(_T("%.2f"), dValue1 * dValue2);
							}
							break;
						case 19:	//���̼�
							vdReq.data_type = dtype_float;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							//dValue = GetKLine(pBusiness, reqTXFunction.nDate, 4);
							dValue = GetKLineEx(pBusiness, reqTXFunction.nDate, 4,reqTXFunction.nSecurityID);
							if(dValue <= 0)
							{
								break;
							}
							if(pBusiness->m_pSecurity->IsFundTradeInMarket() || pBusiness->m_pSecurity->IsFund_Bond() || pBusiness->m_pSecurity->IsFund_Intermix() || 
								pBusiness->m_pSecurity->IsFund_ETF() || pBusiness->m_pSecurity->IsFund_LOF() || pBusiness->m_pSecurity->IsFund_QDII() || 
								pBusiness->m_pSecurity->IsFund_Stock() || (pBusiness->m_pSecurity->IsStockB() && pBusiness->m_pSecurity->IsShanghai())||
								(pBusiness->m_pSecurity->IsBond_National() && pBusiness->m_pSecurity->IsShenzhen()) || pBusiness->m_pSecurity->IsBond_Change() ||
								pBusiness->m_pSecurity->IsWarrant() || pBusiness->m_pSecurity->IsRepurchase())
							{
								strResult.Format(_T("%.3f"), dValue);
								/*	CString strlog;
								strlog.Format( _T("Excel���������̼ۡ�--cost 19 = %s"), strResult );
								Tx::Log::CLogRecorder::GetInstance()->WriteToLog(strlog);*/
							}else
							{
								strResult.Format(_T("%.2f"), dValue);
							/*	CString strlog;
								strlog.Format( _T("Excel���������̼ۡ�--cost 19 = %s"), strResult );
								Tx::Log::CLogRecorder::GetInstance()->WriteToLog(strlog);*/
							}
							break;
						case 60:	//���̼�[��Ȩ]
							vdReq.data_type = dtype_float;
							FindItemOfReq(&reqTXFunction, strReq, 9);	//Date��StartDate��EndDate����Ȩ����
							//dValue1 = GetKLine(pBusiness, reqTXFunction.nDate, 4);
							dValue1 = GetKLineEx(pBusiness, reqTXFunction.nDate, 4,reqTXFunction.nSecurityID);
							if(dValue1 <= 0)
							{
								break;
							}
							if(reqTXFunction.iIRType == 0)
							{	//ǰ��Ȩ
								//  dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nDate, reqTXFunction.nEndDate, true);
								dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nEndDate, true);
							}else
							{	//��Ȩ
								if ( reqTXFunction.nStartDate == 0 )
									reqTXFunction.nStartDate = pBusiness->m_pSecurity->GetTradeDateByIndex(0);
								//dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nDate, false);
								dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nEndDate, false);
							}
							//if(pBusiness->m_pSecurity->IsFundTradeInMarket())
							//bug:11640  2012-09-25
							if(pBusiness->m_pSecurity->IsFundTradeInMarket() || pBusiness->m_pSecurity->IsFund_Bond() || pBusiness->m_pSecurity->IsFund_Intermix() || 
								pBusiness->m_pSecurity->IsFund_ETF() || pBusiness->m_pSecurity->IsFund_LOF() || pBusiness->m_pSecurity->IsFund_QDII() || 
								pBusiness->m_pSecurity->IsFund_Stock() || (pBusiness->m_pSecurity->IsStockB() && pBusiness->m_pSecurity->IsShanghai())||
								(pBusiness->m_pSecurity->IsBond_National() && pBusiness->m_pSecurity->IsShenzhen()) || pBusiness->m_pSecurity->IsBond_Change() ||
								pBusiness->m_pSecurity->IsWarrant() || pBusiness->m_pSecurity->IsRepurchase())
							{
								strResult.Format(_T("%.3f"), dValue1 * dValue2);
							}else
							{
								strResult.Format(_T("%.2f"), dValue1 * dValue2);
							}
							break;
						case 20:	//�վ���
							vdReq.data_type = dtype_float;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							//dValue1 = GetKLine(pBusiness, reqTXFunction.nDate, 5);
							//dValue2 = GetKLine(pBusiness, reqTXFunction.nDate, 6);
							/*dValue1 = GetKLineEx(pBusiness, reqTXFunction.nDate, 5);
							dValue2 = GetKLineEx(pBusiness, reqTXFunction.nDate, 6);*/
							dValue = GetKLineExNew(pBusiness, reqTXFunction.nDate, 9,reqTXFunction.nSecurityID);
							//if(dValue1 > 0 && dValue2 > 0)
							//{	//�ɽ����/�ɽ���
							//	dValue = dValue1 / dValue2;
							//}else
							//{	//ǰ�ռ�
							//	//dValue = GetKLine(pBusiness, reqTXFunction.nDate, 1);
							//	dValue = GetKLineEx(pBusiness, reqTXFunction.nDate, 1);
							//}
							if(dValue < 0.1)		//ǰ�ռ�
								dValue = GetKLineExNew(pBusiness, reqTXFunction.nDate, 1,reqTXFunction.nSecurityID);
							if(dValue <= 0)
							{
								break;
							}
							if(pBusiness->m_pSecurity->IsFundTradeInMarket())
							{
								strResult.Format(_T("%.3f"), dValue);
								/*				CString strlog;
								strlog.Format( _T("Excel�������վ��ۡ�--cost 19 = %s"), strResult );
								Tx::Log::CLogRecorder::GetInstance()->WriteToLog(strlog);*/
							}else
							{
								strResult.Format(_T("%.2f"), dValue);
				/*				CString strlog;
								strlog.Format( _T("Excel�������վ��ۡ�--cost 19 = %s"), strResult );
								Tx::Log::CLogRecorder::GetInstance()->WriteToLog(strlog);*/
							}
							break;
						case 61:	//�վ���[��Ȩ]
							vdReq.data_type = dtype_float;
							FindItemOfReq(&reqTXFunction, strReq, 9);	//Date��StartDate��EndDate����Ȩ����
							//dValue1 = GetKLine(pBusiness, reqTXFunction.nDate, 5);
							//dValue2 = GetKLine(pBusiness, reqTXFunction.nDate, 6);
							dValue1 = GetKLineEx(pBusiness, reqTXFunction.nDate, 5,reqTXFunction.nSecurityID);
							dValue2 = GetKLineEx(pBusiness, reqTXFunction.nDate, 6,reqTXFunction.nSecurityID);
							if(reqTXFunction.iIRType == 0)
							{	//ǰ��Ȩ
								//dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nDate, reqTXFunction.nEndDate, true);
								dValue3 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nEndDate, true);
							}else
							{	//��Ȩ
								if ( reqTXFunction.nStartDate == 0 )
									reqTXFunction.nStartDate = pBusiness->m_pSecurity->GetTradeDateByIndex(0);	
								//dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nDate, false);
								dValue3 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nEndDate, false);
							}
							if(dValue1 > 0 && dValue2 > 0)
							{	//�ɽ����/�ɽ���
								dValue = dValue1 / dValue2;
							}else
							{	//ǰ�ռ�
								//dValue = GetKLine(pBusiness, reqTXFunction.nDate, 1);
								dValue = GetKLineEx(pBusiness, reqTXFunction.nDate, 1,reqTXFunction.nSecurityID);
							}
							if(dValue <= 0)
							{
								break;
							}
							if(pBusiness->m_pSecurity->IsFundTradeInMarket())
							{
								strResult.Format(_T("%.3f"), dValue * dValue3);
							}else
							{
								strResult.Format(_T("%.2f"), dValue * dValue3);
							}
							break;
						case 21:	//�ɽ���
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							//dValue = GetKLine(pBusiness, reqTXFunction.nDate, 6);
							dValue = GetKLineEx(pBusiness, reqTXFunction.nDate, 6,reqTXFunction.nSecurityID);
							if(dValue > 0)
							{
								strResult.Format(_T("%.0f"), dValue);
							}
							break;
						case 22:	//�ɽ����
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							//dValue = GetKLine(pBusiness, reqTXFunction.nDate, 5);
							dValue = GetKLineEx(pBusiness, reqTXFunction.nDate, 5,reqTXFunction.nSecurityID);
							if(dValue > 0)
							{
								if(pBusiness->m_pSecurity->IsFundTradeInMarket())
								{
									strResult.Format(_T("%.3f"), dValue);
								}else
								{
									strResult.Format(_T("%.2f"), dValue);
								}
							}
							break;
						case 23:	//������
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							//dValue1 = GetKLine(pBusiness, reqTXFunction.nDate, 6);
							dValue1 = GetKLineEx(pBusiness, reqTXFunction.nDate, 6,reqTXFunction.nSecurityID);
							dValue2 = 0;
							if(pBusiness->m_pSecurity->IsStock())
							{	//��Ʊ
								pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(reqTXFunction.nDate);
								if(pTxShareData != NULL)
								{
									dValue2 = pTxShareData->TradeableShare;
								}
							}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
							{	//����
								pTxFundShareData = pBusiness->m_pSecurity->GetTxFundShareDataByDate(reqTXFunction.nDate);
								if(pTxFundShareData != NULL)
								{
									dValue2 = pTxFundShareData->TradeableShare;
								}
							}else if(pBusiness->m_pSecurity->IsBond_Change())
							{	//��תծ
								pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
								nDataCount = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
								if(nDataCount > 0)
								{
									pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex();
									if(reqTXFunction.nDate < pCBondAmount->end_date)
									{	//��תծ������
										dValue2 = pBondNewInfo->share / 100;
									}else
									{	//δת��ծȯ���
										pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(nDataCount - 1);
										if(reqTXFunction.nDate >= pCBondAmount->end_date)
										{
											dValue2 = pCBondAmount->not_change_bond_amount / 100;
										}else
										{
											pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByDate(reqTXFunction.nDate);
											dValue2 = pCBondAmount->not_change_bond_amount / 100;
										}
									}
								}else
								{	//��תծ������
									dValue2 = pBondNewInfo->share / 100 ;
								}
							}
							if(dValue1 > 0 && dValue2 > 0)
							{
								strResult.Format(_T("%.2f"), dValue1 * 100 / dValue2);
							}
							break;
						case 24:	//�ۼƳɽ���
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 5);	//StartDate��EndDate
							dValue = GetSumVolume(pBusiness, reqTXFunction.nStartDate, reqTXFunction.nEndDate);
							if(dValue > 0)
							{
								strResult.Format(_T("%.0f"), dValue);
							}
							break;
						case 25:	//�ۼƳɽ����
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 5);	//StartDate��EndDate
							dValue = GetSumAmount(pBusiness, reqTXFunction.nStartDate, reqTXFunction.nEndDate);
							if(dValue > 0)
							{
								if(pBusiness->m_pSecurity->IsFundTradeInMarket())
								{
									strResult.Format(_T("%.3f"), dValue);
								}else
								{
									strResult.Format(_T("%.2f"), dValue);
								}
							}
							break;
						case 26:	//�ۼƻ�����
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 5);	//StartDate��EndDate
							dValue = GetSumExchangeRatio(pBusiness, reqTXFunction.nStartDate, reqTXFunction.nEndDate);
							if(dValue > 0)
							{
								strResult.Format(_T("%.2f"), 100*dValue);
							}
							break;
						case 27:	//��ͨ��ֵ
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							dValue1 = GetKLine(pBusiness, reqTXFunction.nDate, 4,reqTXFunction.nSecurityID);
							dValue2 = 0;
							if(pBusiness->m_pSecurity->IsStock())
							{	//��Ʊ
								pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(reqTXFunction.nDate);
								if(pTxShareData != NULL)
								{
									dValue2 = pTxShareData->TradeableShare;
								}
							}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
							{	//����
								pTxFundShareData = pBusiness->m_pSecurity->GetTxFundShareDataByDate(reqTXFunction.nDate);
								if(pTxFundShareData != NULL)
								{
									dValue2 = pTxFundShareData->TradeableShare;
								}
							}else if (pBusiness->m_pSecurity->IsIndex_TX())
							{
								Tx::Data::IndexShareData* pIndexShare = pBusiness->m_pSecurity->GetIndexShareDataByDate(reqTXFunction.nDate,false);
								if ( pIndexShare != NULL )
									dValue2 = pIndexShare->TradeableValue;
							}
							else if(pBusiness->m_pSecurity->IsBond_Change())
							{	//��תծ
								pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
								nDataCount = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
								if(nDataCount > 0)
								{
									pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex();
									if(reqTXFunction.nDate < pCBondAmount->end_date)
									{	//��תծ������
										dValue2 = pBondNewInfo->share / 100;
									}else
									{	//δת��ծȯ���
										pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(nDataCount - 1);
										if(reqTXFunction.nDate >= pCBondAmount->end_date)
										{
											dValue2 = pCBondAmount->not_change_bond_amount / 100;
										}else
										{
											pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByDate(reqTXFunction.nDate);
											dValue2 = pCBondAmount->not_change_bond_amount / 100;
										}
									}
								}else
								{	//��תծ������
									dValue2 = pBondNewInfo->share / 100 ;
								}
							}
							else if(pBusiness->m_pSecurity->IsIndex())
							{
								Tx::Data::IndexShareData* m_pIndexShareData = NULL;
								DataFileNormal<blk_TxExFile_FileHead,IndexShareData>* pDataFile = new DataFileNormal<blk_TxExFile_FileHead,IndexShareData>;
								if(pDataFile!=NULL)
								{
									bool bLoaded = true;
									bLoaded = pDataFile->Load(reqTXFunction.nSecurityID,30036,true);
									if(bLoaded)
										m_pIndexShareData = pDataFile->GetDataByObj(reqTXFunction.nDate,false);
									if(m_pIndexShareData != NULL)
										dValue = m_pIndexShareData->TradeableValue;
									if(dValue < 0)
										dValue = Con_doubleInvalid;
									if(dValue > 0 &&dValue != Con_doubleInvalid)
										strResult.Format(_T("%.2f"), dValue);
								}
								if(pDataFile != NULL)
									delete pDataFile;
								pDataFile = NULL;
							}
							if(dValue1 > 0 && dValue2 > 0)
							{
								if(pBusiness->m_pSecurity->IsStock()||pBusiness->m_pSecurity->IsIndex_TX())
								{
									strValue.Format(_T("%.2f"), dValue1);
								}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
								{
									strValue.Format(_T("%.3f"), dValue1);
								}else if(pBusiness->m_pSecurity->IsBond_Change())
								{
									strValue.Format(_T("%.2f"), dValue1);
								}
								dValue1 = atof(strValue);
								dValue2 = (ULONGLONG)dValue2;
								dValue = dValue1 * dValue2;
								if(pBusiness->m_pSecurity->IsFundTradeInMarket())
								{
									strResult.Format(_T("%.3f"), dValue);
								}else if( pBusiness->m_pSecurity->IsIndex_TX() )
								{
									strResult.Format(_T("%.2f"), dValue2);
								}
								else
								{
									strResult.Format(_T("%.2f"), dValue);
								}
							}
							break;
						case 28:	//������ֵ(A)
						case 279:   //������ֵ(A+B)
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							dValue1 = GetKLine(pBusiness, reqTXFunction.nDate, 4,reqTXFunction.nSecurityID);
							dValue2 = 0;
							dValue3 = 0;
							dValue4 = 0;
							dValue = 0;
							// ��Ʊ   dValue = dValue1*dValue2 + dValue3*dValue4
							if(pBusiness->m_pSecurity->IsStock())
							{	//��Ʊ
								if (reqTXFunction.iFuncID == 28)  //������ֵ(A)
								{
									pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(reqTXFunction.nDate);
									if(pTxShareData != NULL)
										dValue2 = pTxShareData->TheShare;
								}
								else if (reqTXFunction.iFuncID == 279) //������ֵ(A+B)
								{
									pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(reqTXFunction.nDate);
									if(pTxShareData != NULL)
										dValue2 = pTxShareData->TheShare;
									//
									//�ж���A�ɻ���B��
									if (pBusiness->m_pSecurity->IsStockA())
									{
										//�����A��,�����Ƿ���B��
										mapIntInt.clear();
										Tx::Data::TypeMapManage::GetInstance()->GetTypeMapITI(TYPE_STOCK_TRANSOBJECT_AB, mapIntInt);
									}
									//----------------------------------StockA---------------------------------------
									else if (pBusiness->m_pSecurity->IsStockB())
									{
										//�����B�ɣ������Ƿ���A��
										mapIntInt.clear();
										Tx::Data::TypeMapManage::GetInstance()->GetTypeMapITI(TYPE_STOCK_TRANSOBJECT_BA, mapIntInt);
									}
									//----------------------------------StockB---------------------------------------
									//�����A�ɣ�ͬʱ����B�ɣ���B�ɹɱ����ϣ������B�ɣ�ͬʱ����A�ɣ���A�ɹɱ�����
									iterIntInt = mapIntInt.find((INT)(reqTXFunction.nSecurityID));
									if(iterIntInt != mapIntInt.end())
									{
										if(pBusiness->GetSecurityNow(iterIntInt->second) != NULL)
										{
											dValue3 = GetKLine(pBusiness, reqTXFunction.nDate, 4,iterIntInt->second);

											pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(reqTXFunction.nDate);
											if(pTxShareData != NULL)
												dValue4 = pTxShareData->TheShare;
										}
									}
								}

							}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
							{	//����
								pTxFundShareData = pBusiness->m_pSecurity->GetTxFundShareDataByDate(reqTXFunction.nDate);
								if(pTxFundShareData != NULL)
								{
									dValue2 = pTxFundShareData->TotalShare;
								}
							}else if(pBusiness->m_pSecurity->IsBond_Change())
							{	//��תծ
								pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
								nDataCount = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
								if(nDataCount > 0)
								{
									pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex();
									if(reqTXFunction.nDate < pCBondAmount->end_date)
									{	//��תծ������
										dValue2 = pBondNewInfo->share / 100;
									}else
									{	//δת��ծȯ���
										pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(nDataCount - 1);
										if(reqTXFunction.nDate >= pCBondAmount->end_date)
										{
											dValue2 = pCBondAmount->not_change_bond_amount / 100;
										}else
										{
											pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByDate(reqTXFunction.nDate);
											dValue2 = pCBondAmount->not_change_bond_amount / 100;
										}
									}
								}else
								{	//��תծ������
									dValue2 = pBondNewInfo->share / 100 ;
								}
							}
							else if(pBusiness->m_pSecurity->IsIndex())
							{
								Tx::Data::IndexShareData* m_pIndexShareData = NULL;
								DataFileNormal<blk_TxExFile_FileHead,IndexShareData>* pDataFile = new DataFileNormal<blk_TxExFile_FileHead,IndexShareData>;
								if(pDataFile!=NULL)
								{
									bool bLoaded = true;
									bLoaded = pDataFile->Load(reqTXFunction.nSecurityID,30036,true);
									if(bLoaded)
										m_pIndexShareData = pDataFile->GetDataByObj(reqTXFunction.nDate,false);
									if(m_pIndexShareData != NULL)
										dValue = m_pIndexShareData->TotalValue;
									if(dValue < 0)
										dValue = Con_doubleInvalid;
									if(dValue > 0 &&dValue != Con_doubleInvalid)
										strResult.Format(_T("%.2f"), dValue);
								}
								if(pDataFile != NULL)
									delete pDataFile;
								pDataFile = NULL;
							}
							if (pBusiness->m_pSecurity->IsStock())
							{
								if(pBusiness->m_pSecurity->IsStockA())
								{
									strValue.Format(_T("%.2f"), dValue1);
									strValue1.Format(_T("%.3f"), dValue3);
								}
								else if(pBusiness->m_pSecurity->IsStockB())
								{
									strValue.Format(_T("%.3f"), dValue1);
									strValue1.Format(_T("%.2f"), dValue3);
								}

								if (dValue1 > 0 && dValue2 > 0)
								{
									dValue1 = atof(strValue);
									dValue2 = (ULONGLONG)dValue2;
									dValue = dValue1*dValue2;
								}
								if (dValue3 > 0 && dValue4 > 0)
								{
									dValue3 = atof(strValue1);
									dValue4 = (ULONGLONG)dValue4;
									dValue += dValue3*dValue4;
								}
							}
							else
							{
								if(dValue1 > 0 && dValue2 > 0)
								{
									if(pBusiness->m_pSecurity->IsFundTradeInMarket())
										strValue.Format(_T("%.3f"), dValue1);
									else if(pBusiness->m_pSecurity->IsBond_Change())
										strValue.Format(_T("%.2f"), dValue1);
									//����
									dValue1 = atof(strValue);
									dValue2 = (ULONGLONG)dValue2;
									dValue = dValue1 * dValue2;
								}
							}
							if (dValue > 0)
							{
								if(pBusiness->m_pSecurity->IsFundTradeInMarket())
									strResult.Format(_T("%.3f"), dValue);
								else
									strResult.Format(_T("%.2f"), dValue);
							}
							break;
						case 29:	//����ֵ
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							dValue1 = GetKLine(pBusiness, reqTXFunction.nDate, 4,reqTXFunction.nSecurityID);
							dValue2 = 0;
							if(pBusiness->m_pSecurity->IsStock())
							{	//��Ʊ
								pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(reqTXFunction.nDate);
								if(pTxShareData != NULL)
								{
									dValue2 = pTxShareData->TotalShare;
								}
							}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
							{	//����
								pTxFundShareData = pBusiness->m_pSecurity->GetTxFundShareDataByDate(reqTXFunction.nDate);
								if(pTxFundShareData != NULL)
								{
									dValue2 = pTxFundShareData->TotalShare;
								}
							}else if (pBusiness->m_pSecurity->IsIndex_TX())
							{
								Tx::Data::IndexShareData* pIndexShare = pBusiness->m_pSecurity->GetIndexShareDataByDate(reqTXFunction.nDate,false);
								if ( pIndexShare != NULL )
									dValue2 = pIndexShare->TotalValue;
							}else if(pBusiness->m_pSecurity->IsBond_Change())
							{	//��תծ
								pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
								nDataCount = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
								if(nDataCount > 0)
								{
									pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex();
									if(reqTXFunction.nDate < pCBondAmount->end_date)
									{	//��תծ������
										dValue2 = pBondNewInfo->share / 100;
									}else
									{	//δת��ծȯ���
										pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(nDataCount - 1);
										if(reqTXFunction.nDate >= pCBondAmount->end_date)
										{
											dValue2 = pCBondAmount->not_change_bond_amount / 100;
										}else
										{
											pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByDate(reqTXFunction.nDate);
											dValue2 = pCBondAmount->not_change_bond_amount / 100;
										}
									}
								}else
								{	//��תծ������
									dValue2 = pBondNewInfo->share / 100 ;
								}
							}
							else if(pBusiness->m_pSecurity->IsIndex())
							{
								Tx::Data::IndexShareData* m_pIndexShareData = NULL;
								DataFileNormal<blk_TxExFile_FileHead,IndexShareData>* pDataFile = new DataFileNormal<blk_TxExFile_FileHead,IndexShareData>;
								if(pDataFile!=NULL)
								{
									bool bLoaded = true;
									bLoaded = pDataFile->Load(reqTXFunction.nSecurityID,30036,true);
									if(bLoaded)
										m_pIndexShareData = pDataFile->GetDataByObj(reqTXFunction.nDate,false);
									if(m_pIndexShareData != NULL)
										dValue = m_pIndexShareData->TotalValue;
									if(dValue < 0)
										dValue = Con_doubleInvalid;
									if(dValue > 0 &&dValue != Con_doubleInvalid)
										strResult.Format(_T("%.2f"), dValue);
								}
								if(pDataFile != NULL)
									delete pDataFile;
								pDataFile = NULL;
							}
							if(dValue1 > 0 && dValue2 > 0)
							{
								if(pBusiness->m_pSecurity->IsStock()||pBusiness->m_pSecurity->IsIndex_TX())
								{
									strValue.Format(_T("%.2f"), dValue1);
								}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
								{
									strValue.Format(_T("%.3f"), dValue1);
								}else if(pBusiness->m_pSecurity->IsBond_Change())
								{
									strValue.Format(_T("%.2f"), dValue1);
								}
								dValue1 = atof(strValue);
								dValue2 = (ULONGLONG)dValue2;
								dValue = dValue1 * dValue2;
								if(pBusiness->m_pSecurity->IsFundTradeInMarket())
								{
									strResult.Format(_T("%.3f"), dValue);
								}else if( pBusiness->m_pSecurity->IsIndex_TX() )
								{
									strResult.Format(_T("%.2f"), dValue2);
								}
								else
								{
									strResult.Format(_T("%.2f"), dValue);
								}
							}
							break;
						case 30:	//������<=ָ���۸�ĵ�һ������
							vdReq.data_type = dtype_int4;
							FindItemOfReq(&reqTXFunction, strReq, 11);	//StartDate��EndDate���۸����͡���Ȩ���͡��۸�
							nDate = Get_Price_Occur_Date(pBusiness, reqTXFunction.dPrice, reqTXFunction.iPriceType, reqTXFunction.iIRType, reqTXFunction.nStartDate, reqTXFunction.nEndDate);
							if(nDate > 0)
							{
								//�޸�Ϊ8λ��--20080708--
								strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
							}
							break;
						case 31:	//������<=ָ���������ݵĵ�һ������
							vdReq.data_type = dtype_int4;
							FindItemOfReq(&reqTXFunction, strReq, 10);	//StartDate��EndDate���������͡���������
							nDate = Get_Value_Occur_Date(pBusiness, reqTXFunction.dFindValue, reqTXFunction.iValueType, reqTXFunction.nStartDate, reqTXFunction.nEndDate);
							if(nDate > 0)
							{
								//�޸�Ϊ8λ��--20080708--
								strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
							}							
							break;
						case 100:	//����������������Сֵ�ĵ�һ������
							vdReq.data_type = dtype_int4;
							FindItemOfReq(&reqTXFunction, strReq, 7);	//StartDate��EndDate����������
							nDate = Get_Value_Extremum_Date(pBusiness, reqTXFunction.iValueType, reqTXFunction.nStartDate, reqTXFunction.nEndDate, TRUE);
							if(nDate > 0)
							{
								//�޸�Ϊ8λ��--20080708--
								strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
							}	 
							break;
						case 101:	//�����������������ֵ�ĵ�һ������
							vdReq.data_type = dtype_int4;
							FindItemOfReq(&reqTXFunction, strReq, 7);	//StartDate��EndDate����������
							nDate = Get_Value_Extremum_Date(pBusiness, reqTXFunction.iValueType, reqTXFunction.nStartDate, reqTXFunction.nEndDate, FALSE);
							if(nDate > 0)
							{
								//�޸�Ϊ8λ��--20080708--
								strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
							}							
							break;
						case 102:	//�����ڼ۸���Сֵ�ĵ�һ������
							vdReq.data_type = dtype_int4;
							FindItemOfReq(&reqTXFunction, strReq, 8);	//StartDate��EndDate���۸�����
							nDate = Get_Price_Extremum_Date(pBusiness, reqTXFunction.iPriceType, reqTXFunction.nStartDate, reqTXFunction.nEndDate, TRUE);
							if(nDate > 0)
							{
								//�޸�Ϊ8λ��--20080708--
								strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
							}
							break;
						case 103:	//�����ڼ۸����ֵ�ĵ�һ������
							vdReq.data_type = dtype_int4;
							FindItemOfReq(&reqTXFunction, strReq, 8);	//StartDate��EndDate���۸�����
							nDate = Get_Price_Extremum_Date(pBusiness, reqTXFunction.iPriceType, reqTXFunction.nStartDate, reqTXFunction.nEndDate, FALSE);
							if(nDate > 0)
							{
								//�޸�Ϊ8λ��--20080708--
								strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
							}
							break;
						case 32:	//�׶��Ƿ�
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 5);	//StartDate��EndDate
							//��StartDateΪ��׼��EndDate���С���Ȩ��
							dValue3 = GetKLine(pBusiness, reqTXFunction.nStartDate, 1,reqTXFunction.nSecurityID);	//�״η��м۸�==��������ǰ�ռ�
							dValue1 = GetKLine(pBusiness, reqTXFunction.nEndDate, 4,reqTXFunction.nSecurityID);
							dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nEndDate, false);
							if(dValue1 > 0 && dValue2 > 0 && dValue3 > 0)
							{
								dValue = (dValue1 * dValue2) / dValue3 - 1;
								strResult.Format(_T("%.4f"), dValue);
							}
							break;
						case 321:	//�׶��Ƿ�(������ʼ���ڵ����Ƿ�)
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 5);	//StartDate��EndDate
							//��StartDateΪ��׼��EndDate���С���Ȩ��
							dValue3 = GetKLine(pBusiness, reqTXFunction.nStartDate, 4,reqTXFunction.nSecurityID);	//�״η��м۸�==��������ǰ�ռ�
							dValue1 = GetKLine(pBusiness, reqTXFunction.nEndDate, 4,reqTXFunction.nSecurityID);
							dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nEndDate, false);
							if(dValue1 > 0 && dValue2 > 0 && dValue3 > 0)
							{
								dValue = (dValue1 * dValue2) / dValue3 - 1;
								strResult.Format(_T("%.4f"), dValue);
							}
							break;
						case 322:	//�����������Ƿ�
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 3);	
							dValue3 = GetKLine(pBusiness, pBusiness->m_pSecurity->GetTradeDateByIndex(0), 0,reqTXFunction.nSecurityID);	
							dValue1 = GetKLine(pBusiness, reqTXFunction.nDate, 4,reqTXFunction.nSecurityID);
							dValue2 = pBusiness->m_pSecurity->GetExdividendScale(pBusiness->m_pSecurity->GetTradeDateByIndex(0), reqTXFunction.nDate, false);
							if(dValue1 > 0 && dValue2 > 0 && dValue3 > 0)
							{
								//��Ϊ�ٷ���20080710
								dValue = (dValue1 * dValue2) / dValue3 - 1;
								strResult.Format(_T("%.2f"), 100*dValue);
							}
							break;
						case 33:	//������
							vdReq.data_type = dtype_double;
							break;
						case 34:	//������
							vdReq.data_type = dtype_double;
							break;
						case 144:	//���5�����̼�
							vdReq.data_type = dtype_float;
							FindItemOfReq(&reqTXFunction, strReq, 23);	//���������
							if(pBusiness == NULL || pBusiness->m_pSecurity == NULL)
								break;
							pBusiness->m_pSecurity->RequestQuotation();
							pTradeQuotation = pBusiness->m_pSecurity->GetTradeQuotationPointer();
							if(pTradeQuotation != NULL)
							{
								nDataCount = pTradeQuotation->GetTradeQuotationDataCount();
								if(nDataCount == 10)
								{
									pTradeQuotationData = pTradeQuotation->GetBuyData(reqTXFunction.iQuotationNo);
									if(pTradeQuotationData != NULL)
									{
										dValue = pTradeQuotationData->fPrice;
										if(pBusiness->m_pSecurity->IsFundTradeInMarket())
										{
											strResult.Format(_T("%.3f"), dValue);
										}else
										{
											strResult.Format(_T("%.2f"), dValue);
										}
									}
								}
							}
							break;
						case 145:	//���5�����̼�
							vdReq.data_type = dtype_float;
							FindItemOfReq(&reqTXFunction, strReq, 23);	//���������
						if(pBusiness == NULL || pBusiness->m_pSecurity == NULL)
							break;
						pBusiness->m_pSecurity->RequestQuotation();
							pTradeQuotation = pBusiness->m_pSecurity->GetTradeQuotationPointer();
							if(pTradeQuotation != NULL)
							{
								nDataCount = pTradeQuotation->GetTradeQuotationDataCount();
								if(nDataCount == 10)
								{
									pTradeQuotationData = pTradeQuotation->GetSaleData(reqTXFunction.iQuotationNo);
									if(pTradeQuotationData != NULL)
									{
										dValue = pTradeQuotationData->fPrice;
										if(pBusiness->m_pSecurity->IsFundTradeInMarket())
										{
											strResult.Format(_T("%.3f"), dValue);
										}else
										{
											strResult.Format(_T("%.2f"), dValue);
										}
									}
								}
							}
							break;
						case 146:	//���5��������
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 23);	//���������
						if(pBusiness == NULL || pBusiness->m_pSecurity == NULL)
							break;
						pBusiness->m_pSecurity->RequestQuotation();
							pTradeQuotation = pBusiness->m_pSecurity->GetTradeQuotationPointer();
							if(pTradeQuotation != NULL)
							{
								nDataCount = pTradeQuotation->GetTradeQuotationDataCount();
								if(nDataCount == 10)
								{
									pTradeQuotationData = pTradeQuotation->GetBuyData(reqTXFunction.iQuotationNo);
									if(pTradeQuotationData != NULL)
									{
										dValue = pTradeQuotationData->dVolume;
										strResult.Format(_T("%.0f"), dValue);
									}
								}
							}
							break;
						case 147:	//���5��������
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 23);	//���������
						if(pBusiness == NULL || pBusiness->m_pSecurity == NULL)
							break;
						pBusiness->m_pSecurity->RequestQuotation();
							pTradeQuotation = pBusiness->m_pSecurity->GetTradeQuotationPointer();
							if(pTradeQuotation != NULL)
							{
								nDataCount = pTradeQuotation->GetTradeQuotationDataCount();
								if(nDataCount == 10)
								{
									pTradeQuotationData = pTradeQuotation->GetSaleData(reqTXFunction.iQuotationNo);
									if(pTradeQuotationData != NULL)
									{
										dValue = pTradeQuotationData->dVolume;
										strResult.Format(_T("%.0f"), dValue);
									}
								}
							}
							break;
						case 148:	//ծȯ���5�����̼۶�Ӧ�ĵ���������
							vdReq.data_type = dtype_double;
							if(pBusiness->m_pSecurity->IsBond())
							{
								FindItemOfReq(&reqTXFunction, strReq, 23);	//���������
								pTradeQuotation = pBusiness->m_pSecurity->GetTradeQuotationPointer();
								if(pTradeQuotation != NULL)
								{
									nDataCount = pTradeQuotation->GetTradeQuotationDataCount();
									if(nDataCount == 10)
									{
										pTradeQuotationData = pTradeQuotation->GetBuyData(reqTXFunction.iQuotationNo);
										if(pTradeQuotationData != NULL)
										{
											nDate = pBusiness->m_pSecurity->GetCurDataDate();
											dValue1 = pTradeQuotationData->fPrice;
											if(pBusiness->m_pSecurity->IsBond_National())
											{	//��ծʵ�о��۽��ף����Ǽ��㵽��������ʱծȯ�۸�Ϊȫ��
												//dValue2 = bond.GetInterest((INT)(reqTXFunction.nSecurityID), nDate);
												//2012-7-16  Ӧ����Ϣ(��)
												dValue2 = bond.GetInterest_New((INT)(reqTXFunction.nSecurityID), nDate,true);
												if(dValue2 < 0)
												{
													break;
												}
												dValue1 += dValue2;
											}
											bond.Calc((INT)(reqTXFunction.nSecurityID), nDate, (FLOAT)dValue1);
											dValue = bond.Get_YTM();
											if(dValue != 0)
											{
												strResult.Format(_T("%.5f"), dValue);
											}
										}
									}
								}
							}
							break;
						case 149:	//ծȯ���5�����̼۶�Ӧ�ĵ���������
							vdReq.data_type = dtype_double;
							if(pBusiness->m_pSecurity->IsBond())
							{
								FindItemOfReq(&reqTXFunction, strReq, 23);	//���������
								pTradeQuotation = pBusiness->m_pSecurity->GetTradeQuotationPointer();
								if(pTradeQuotation != NULL)
								{
									nDataCount = pTradeQuotation->GetTradeQuotationDataCount();
									if(nDataCount == 10)
									{
										pTradeQuotationData = pTradeQuotation->GetSaleData(reqTXFunction.iQuotationNo);
										if(pTradeQuotationData != NULL)
										{
											nDate = pBusiness->m_pSecurity->GetCurDataDate();
											dValue1 = pTradeQuotationData->fPrice;
											if(pBusiness->m_pSecurity->IsBond_National())
											{	//��ծʵ�о��۽��ף����Ǽ��㵽��������ʱծȯ�۸�Ϊȫ��
												//dValue2 = bond.GetInterest((INT)(reqTXFunction.nSecurityID), nDate);
												//2012-7-16  Ӧ����Ϣ(��)
												dValue2 = bond.GetInterest_New((INT)(reqTXFunction.nSecurityID), nDate,true);
												if(dValue2 < 0)
												{
													break;
												}
												dValue1 += dValue2;
											}
											bond.Calc((INT)(reqTXFunction.nSecurityID), nDate, (FLOAT)dValue1);
											dValue = bond.Get_YTM();
											if(dValue != 0)
											{
												strResult.Format(_T("%.5f"), dValue);
											}
										}
									}
								}
							}
							break;
						case 35:	//��ͨ�ɱ�
							vdReq.data_type = dtype_int4;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							dValue = 0;
							if(pBusiness->m_pSecurity->IsStock())
							{	//��Ʊ
								pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(reqTXFunction.nDate);
								if(pTxShareData != NULL)
								{
									dValue = pTxShareData->TradeableShare;
								}
							}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
							{	//����
								pTxFundShareData = pBusiness->m_pSecurity->GetTxFundShareDataByDate(reqTXFunction.nDate);
								if(pTxFundShareData != NULL)
								{
									dValue = pTxFundShareData->TradeableShare;
								}
							}else if(pBusiness->m_pSecurity->IsBond_Change())
							{	//��תծ
								pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
								nDataCount = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
								if(nDataCount > 0)
								{
									pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex();
									if(reqTXFunction.nDate < pCBondAmount->end_date)
									{	//��תծ������
										dValue = pBondNewInfo->share / 100;
									}else
									{	//δת��ծȯ���
										pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(nDataCount - 1);
										if(reqTXFunction.nDate >= pCBondAmount->end_date)
										{
											dValue = pCBondAmount->not_change_bond_amount / 100;
										}else
										{
											pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByDate(reqTXFunction.nDate);
											dValue = pCBondAmount->not_change_bond_amount / 100;
										}
									}
								}else
								{	//��תծ������
									dValue = pBondNewInfo->share / 100 ;
								}
							}
							if(dValue > 0)
							{
								strResult.Format(_T("%.0f"), dValue);
							}
							break;
						case 36:	//�ܹɱ�
							vdReq.data_type = dtype_int4;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							dValue = 0;
							if(pBusiness->m_pSecurity->IsStock())
							{	//��Ʊ
								pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(reqTXFunction.nDate);
								if(pTxShareData != NULL)
								{
									dValue = pTxShareData->TotalShare;
								}
							}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
							{	//����
								pTxFundShareData = pBusiness->m_pSecurity->GetTxFundShareDataByDate(reqTXFunction.nDate);
								if(pTxFundShareData != NULL)
								{
									dValue = pTxFundShareData->TotalShare;
								}
							}else if(pBusiness->m_pSecurity->IsBond_Change())
							{	//��תծ
								pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
								nDataCount = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
								if(nDataCount > 0)
								{
									pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex();
									if(reqTXFunction.nDate < pCBondAmount->end_date)
									{	//��תծ������
										dValue = pBondNewInfo->share / 100;
									}else
									{	//δת��ծȯ���
										pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(nDataCount - 1);
										if(reqTXFunction.nDate >= pCBondAmount->end_date)
										{
											dValue = pCBondAmount->not_change_bond_amount / 100;
										}else
										{
											pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByDate(reqTXFunction.nDate);
											dValue = pCBondAmount->not_change_bond_amount / 100;
										}
									}
								}else
								{	//��תծ������
									dValue = pBondNewInfo->share / 100 ;
								}
							}
							if(dValue > 0)
							{
								strResult.Format(_T("%.0f"), dValue);
							}
							break;
						case 37:	//���ڹɱ�(A)
						case 278:   //���ڹɱ�(A+B)
							vdReq.data_type = dtype_int4;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							dValue = 0;
							if(pBusiness->m_pSecurity->IsStock())
							{	//��Ʊ
								if (reqTXFunction.iFuncID == 37) //���ڹɱ�(A)
								{
									pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(reqTXFunction.nDate);
									if(pTxShareData != NULL)
										dValue = pTxShareData->TheShare;
								}
								else if(reqTXFunction.iFuncID == 278)//���ڹɱ�(A+B)
								{
									pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(reqTXFunction.nDate);
									if(pTxShareData != NULL)
										dValue = pTxShareData->TheShare;
									if (dValue < 0)  dValue = 0;
									//�ж���A�ɻ���B��
									if (pBusiness->m_pSecurity->IsStockA())
									{
										//�����A��,�����Ƿ���B��
										mapIntInt.clear();
										Tx::Data::TypeMapManage::GetInstance()->GetTypeMapITI(TYPE_STOCK_TRANSOBJECT_AB, mapIntInt);
									}
									//----------------------------------StockA---------------------------------------
									else if (pBusiness->m_pSecurity->IsStockB())
									{
										//�����B�ɣ������Ƿ���A��
										mapIntInt.clear();
										Tx::Data::TypeMapManage::GetInstance()->GetTypeMapITI(TYPE_STOCK_TRANSOBJECT_BA, mapIntInt);
									}
									//----------------------------------StockB---------------------------------------
									//�����A�ɣ�ͬʱ����B�ɣ���B�ɹɱ����ϣ������B�ɣ�ͬʱ����A�ɣ���A�ɹɱ�����
									iterIntInt = mapIntInt.find((INT)(reqTXFunction.nSecurityID));
									if(iterIntInt != mapIntInt.end())
									{
										if(pBusiness->GetSecurityNow(iterIntInt->second) != NULL)
										{
											pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(reqTXFunction.nDate);
											if(pTxShareData != NULL)
												dValue += pTxShareData->TheShare;
										}
									}
								}
								//------------------------------------���ڹɱ�(A+B)----------------------------------
							}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
							{	//����
								pTxFundShareData = pBusiness->m_pSecurity->GetTxFundShareDataByDate(reqTXFunction.nDate);
								if(pTxFundShareData != NULL)
								{
									dValue = pTxFundShareData->TotalShare;
								}
							}else if(pBusiness->m_pSecurity->IsBond_Change())
							{	//��תծ
								pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
								nDataCount = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
								if(nDataCount > 0)
								{
									pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex();
									if(reqTXFunction.nDate < pCBondAmount->end_date)
									{	//��תծ������
										dValue = pBondNewInfo->share / 100;
									}else
									{	//δת��ծȯ���
										pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(nDataCount - 1);
										if(reqTXFunction.nDate >= pCBondAmount->end_date)
										{
											dValue = pCBondAmount->not_change_bond_amount / 100;
										}else
										{
											pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByDate(reqTXFunction.nDate);
											dValue = pCBondAmount->not_change_bond_amount / 100;
										}
									}
								}else
								{	//��תծ������
									dValue = pBondNewInfo->share / 100 ;
								}
							}
							if(dValue > 0)
							{
								strResult.Format(_T("%.0f"), dValue);
							}
							break;
						case 285: //���ڹɱ�(B)
							break;
						case 38:	//�ֹ�����
							vdReq.data_type = dtype_int4;
							if(tableIndID[7].GetRowCount() == 0)
							{
								//--------------�漰�����ļ����أ��ж�--------------------
								pBusiness->m_pLogicalBusiness->GetData(tableIndID[7], true);
								//--------------wangzhy----20080417----------------------
								//break;
							}
							dValue = 0;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							nInstitution = (INT)pBusiness->m_pSecurity->GetInstitutionId();
							vRow.clear();
							tableIndID[7].Find(0, nInstitution, vRow);
							if(vRow.size() == 0)
							{
								break;
							}
							for(i = 0; i < vRow.size(); i++)
							{
								tableIndID[7].GetCell(1, vRow[i], nDate);
								if(nDate > reqTXFunction.nDate)
								{
									break;
								}
							}
							if(i > 0)
							{
								tableIndID[7].GetCell(2, vRow[i - 1], dValue);
							}
							if(dValue > 0)
							{
								strResult.Format(_T("%.0f"), dValue);
							}
							break;
						case 39:	//���й�
						case 99:	//����ͨ��
							vdReq.data_type = dtype_int4;
							if(tableIndID[6].GetRowCount() == 0)
							{
								//--------------�漰�����ļ����أ��ж�--------------------
								pBusiness->m_pLogicalBusiness->GetData(tableIndID[6], true);
								//--------------wangzhy----20080417----------------------
								//break;
							}
							dValue = 0;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							nInstitution = (INT)pBusiness->m_pSecurity->GetInstitutionId();
							vRow.clear();
							tableIndID[6].Find(0, nInstitution, vRow);
							if(vRow.size() == 0)
							{
								break;
							}
							for(i = 0; i < vRow.size(); i++)
							{
								tableIndID[6].GetCell(1, vRow[i], nDate);
								if(nDate > reqTXFunction.nDate)
								{
									break;
								}
							}
							if(i > 0)
							{
								if(reqTXFunction.iFuncID == 99)
								{	//����ͨ��
									tableIndID[6].GetCell(2, vRow[i - 1], dValue);
								}else
								{	//���й�
									tableIndID[6].GetCell(3, vRow[i - 1], dValue);
								}

							}
							if(dValue >= 0)
							{
								strResult.Format(_T("%.0f"), dValue);
							}
							break;
						case 40:
						case 41:
						case 42:
						case 43:
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 13);	//������ȡ����񼾶ȡ��ϲ�/ĸ��˾������ǰ/��
							nInstitution = (INT)pBusiness->m_pSecurity->GetInstitutionId();
							iSubItemID = (int)reqTXFunction.iSubItemID-1;
							dValue = Tx::Core::Con_doubleInvalid;
							switch( reqTXFunction.iFuncID )
							{
							case 40:
								{
									nAccountingID = nInstitution * 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);
									//�����������ļ�	
									bLoaded = true;
									if(m_pFinancialDataFile!=NULL)
										bLoaded = m_pFinancialDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30073,true);
									if ( bLoaded )
										m_pFinancial = m_pFinancialDataFile->GetDataByObj(nAccountingID,false);
									if ( m_pFinancial!=NULL)
									{
										dValue = m_pFinancial->dFinancial[iSubItemID];
										if ( iSubItemID ==7||iSubItemID==8)
											dValue/=100;
									}
									if(dValue != Tx::Core::Con_doubleInvalid && dValue != Tx::Core::Con_floatInvalid)
									{
										if(iSubItemID == 1)
											strResult.Format(_T("%.3f"),dValue);
										else
											strResult.Format(_T("%.2f"), dValue);
									}
								}
								break;
							case 41:
								{	
									nAccountingID = nInstitution * 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);
									//�����������ļ�	
									bLoaded = true;
									if(m_pBalanceDataFile!=NULL)
										bLoaded = m_pBalanceDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30067,true);
									if ( bLoaded )
										m_pBalance = m_pBalanceDataFile->GetDataByObj(nAccountingID,false);

									if ( m_pBalance!=NULL)
										dValue = m_pBalance->dBalance[iSubItemID];
									if(dValue != Tx::Core::Con_doubleInvalid && dValue != Tx::Core::Con_floatInvalid)
									{
										strResult.Format(_T("%.2f"), dValue);
									}
								}
								break;
							case 43:
								{
									nAccountingID = nInstitution * 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);
									//�����������ļ�	
									bLoaded = true;
									if(m_pCashFlowDataFile!=NULL)
										bLoaded = m_pCashFlowDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30069,true);
									if ( bLoaded )
										m_pCashFlow = m_pCashFlowDataFile->GetDataByObj(nAccountingID,false);

									if ( m_pCashFlow!=NULL)
										dValue = m_pCashFlow->dCashFlow[iSubItemID];
									if(dValue != Tx::Core::Con_doubleInvalid && dValue != Tx::Core::Con_floatInvalid)
									{
										strResult.Format(_T("%.2f"), dValue);
									}
								}
								break;
							case 42:
								{
									nAccountingID = nInstitution * 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);
									//�����������ļ�	
									bLoaded = true;
									if(m_pIncomeDataFile!=NULL)
										bLoaded = m_pIncomeDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30071,true);
									if ( bLoaded )
										m_pIncome = m_pIncomeDataFile->GetDataByObj(nAccountingID,false);
		
									if ( m_pIncome!=NULL)
										dValue = m_pIncome->dIncome[iSubItemID];
									if(dValue != Tx::Core::Con_doubleInvalid && dValue != Tx::Core::Con_floatInvalid)
									{
										strResult.Format(_T("%.2f"), dValue);
									}
								}
								break;
							}
							
							break;
							//------------------------wangzhy-----------------20080527------------------------------
						case 55:	//��Ҫ����ָ��������ָ��
							vdReq.data_type = dtype_val_string;
							if(tableIndID[2].GetRowCount() == 0)
							{
								//--------------�漰�����ļ����أ��ж�--------------------
								pBusiness->m_pLogicalBusiness->GetData(tableIndID[2], true);
								//--------------wangzhy----20080417----------------------
								//break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 13);	//������ȡ����񼾶ȡ��ϲ�/ĸ��˾������ǰ/��
							nInstitution = (INT)pBusiness->m_pSecurity->GetInstitutionId();
							nAccountingID = (INT64)nInstitution * 10000000 + (INT64)reqTXFunction.nFYear * 1000 + (INT64)reqTXFunction.nFQuarter * 100 + (INT64)reqTXFunction.iReportType * 10 + (INT64)reqTXFunction.iAccountingPolicyType;
							vRow.clear();
							tableIndID[2].Find(0, nAccountingID, vRow);
							if(vRow.size() != 1)
							{
								break;
							}
							switch(reqTXFunction.iSubItemID)
							{
							case 1:		//������
							case 2:		//���ʦ����
								tableIndID[2].GetCell(tableIndID[2].GetColCount() - 4 + (UINT)(reqTXFunction.iSubItemID), vRow[0], strValue);
								if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
								{
									strResult = strValue;
								}
								break;
							default:	//���������id
								tableIndID[2].GetCell(tableIndID[2].GetColCount() - 4 + (UINT)(reqTXFunction.iSubItemID), vRow[0], nInstitution);
								if(nInstitution > 0)
								{
									strResult = Tx::Data::TypeMapManage::GetInstance()->GetDatByID(TYPE_ACCOUNTING_FIRM, nInstitution);
								}
								break;
							}
							break;
						case 75:	//�����ʲ���ֵ
							/*
							vdReq.data_type = dtype_double;							
							if(tableIndID[0].GetRowCount() == 0)
							{
								//--------------�漰�����ļ����أ��ж�--------------------
								pBusiness->m_pLogicalBusiness->GetData(tableIndID[0], true);
								//--------------wangzhy----20080417----------------------
								//break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 12);	//������ȡ����񼾶�
							nSecurity1ID = (INT)pBusiness->m_pSecurity->GetSecurity1Id();
							subtable_ID.Clear();
							subtable_ID.CopyColumnInfoFrom(tableIndID[0]);
							tableIndID[0].Between(subtable_ID, array_Col, 4, 0, nSecurity1ID, nSecurity1ID, true, true);
							if(subtable_ID.GetRowCount() == 0)
							{
								break;
							}
							subtable_Year.Clear();
							subtable_Year.CopyColumnInfoFrom(subtable_ID);
							subtable_ID.Between(subtable_Year, array_Col, 4, 1, reqTXFunction.nFYear, reqTXFunction.nFYear, true, true);
							if(subtable_Year.GetRowCount() == 0)
							{
								break;
							}
							subtable_Quarter.Clear();
							subtable_Quarter.CopyColumnInfoFrom(subtable_Year);
							subtable_Year.Between(subtable_Quarter, array_Col, 4, 2, 40040000 + reqTXFunction.nFQuarter, 40040000 + reqTXFunction.nFQuarter, true, true);
							if(subtable_Quarter.GetRowCount() != 1)
							{
								break;
							}
							subtable_Quarter.GetCell(3, 0, dValue);
							if(dValue != Tx::Core::Con_doubleInvalid && dValue != Tx::Core::Con_floatInvalid)
							{
								if(dValue != 0)
								{
									strResult.Format(_T("%.2f"), dValue);
								}
							}
							*/
							{
								vdReq.data_type = dtype_double;
								FindItemOfReq(&reqTXFunction, strReq, 3);
								//Mantis:13263    2012-10-24
								//�ʲ���ֵȡ���������ֱ����¶�ı��缾��ĩ�ʲ���ֵ����Ӧ������¶���ʲ���ֵ�����û����¶���ܵ��ʲ���ֵ�������ܷݶ�*��λ�ʲ���ֵ�����㡣 
								//---------------------------------------------------------------------------------------------------------------
								int nTemp = (int)((reqTXFunction.nDate % 10000) / 100) * 100 + (int)(reqTXFunction.nDate % 100);
								if (nTemp == 331 || nTemp == 630 || nTemp == 930 || nTemp == 1231)
								{
									if(nTemp == 331)
										nTemp = 1;
									else if(nTemp == 630)
										nTemp = 2;
									else if(nTemp == 930)
										nTemp = 4;
									else
										nTemp = 6;
									Tx::Data::FundInvesmentGroup* pFundInvesmentGroup = NULL;
									nAccountingID = (INT64)((reqTXFunction.nDate / 10000) * 10000) + (INT64)nTemp;
									pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_FundInvesmentGroup,nAccountingID,&pFundInvesmentGroup,false);

									if (pFundInvesmentGroup == NULL)
									{

										pTxFundShareData = pBusiness->m_pSecurity->GetTxFundShareDataByDate( reqTXFunction.nDate );
										if ( pTxFundShareData != NULL)
										{
											if( pBusiness->m_pSecurity->IsFund_Currency())
											{
												strResult.Format(_T("%.2f"),pTxFundShareData->TotalShare);
											}
											else
											{
												dValue = GetFundNvrVal( reqTXFunction.nSecurityID,reqTXFunction.nDate);
												if ( dValue != Con_doubleInvalid )
												{
													CString sTemp = _T("");
													sTemp.Format(_T("%.4f"),dValue);
													dValue = atof(sTemp);
													strResult.Format(_T("%.2f"),pTxFundShareData->TotalShare*dValue);
												}
											}
										}
									}
									else
									{
										dValue = pFundInvesmentGroup->dValue[9];
										if(dValue != Con_doubleInvalid)
											strResult.Format(_T("%.2f"),dValue);
									}
								}
								//-----------------------------------------------------------------------------------------------------------------
								else
								{
									pTxFundShareData = pBusiness->m_pSecurity->GetTxFundShareDataByDate( reqTXFunction.nDate );
									if ( pTxFundShareData != NULL)
									{
										if( pBusiness->m_pSecurity->IsFund_Currency())
										{
											strResult.Format(_T("%.2f"),pTxFundShareData->TotalShare);
										}
										else
										{
											dValue = GetFundNvrVal( reqTXFunction.nSecurityID,reqTXFunction.nDate);
											if ( dValue != Con_doubleInvalid )
												strResult.Format(_T("%.2f"),pTxFundShareData->TotalShare*dValue);
										}
									}
								}
							}
							break;
						case 108:	//��תծ-����
							vdReq.data_type = dtype_int4;
							switch(reqTXFunction.iSubItemID)
							{
							case 1:		//������
								iRowNo = GetDateIndicator_Row((INT)(reqTXFunction.nSecurityID), 0, &vBondIPOInfo);
								if(iRowNo >= 0)
								{
									//ծȯ������ʼ����
									tableIndDate[5].GetCell(6, iRowNo, nDate);
									if(nDate > 0)
									{
										//�޸�Ϊ8λ��--20080708--
										strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
										//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
									}
								}
								break;
							case 2:		//��Ϣ��
								pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
								if(pBondNewInfo != NULL)
								{
									nDate = pBondNewInfo->begin_date;
									//�޸�Ϊ8λ��--20080708--
									strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
									//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								}
								break;
							case 3:		//������
								nDate = pBusiness->m_pSecurity->GetIPOListedDate();
								if(nDate > 0)
								{
									//�޸�Ϊ8λ��--20080708--
									strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
									//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								}
								break;
							case 8:		//������
								pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
								if(pBondNewInfo != NULL)
								{
									nDate = pBondNewInfo->end_date;
									//�޸�Ϊ8λ��--20080708--
									strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
									//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								}
								break;
							default:
								if(tableIndID[1].GetRowCount() == 0)
								{
									//--------------�漰�����ļ����أ��ж�--------------------
									pBusiness->m_pLogicalBusiness->GetData(tableIndID[1], true);
									//--------------wangzhy----20080417----------------------
									//break;
								}
								nSecurity1ID = (INT)pBusiness->m_pSecurity->GetSecurity1Id();
								vRow.clear();
								tableIndID[1].Find(0, nSecurity1ID, vRow);
								if(vRow.size() != 1)
								{
									break;
								}
								switch(reqTXFunction.iSubItemID)
								{
								case 4:		//ת����ʼ��
									tableIndID[1].GetCell(2, vRow[0], nDate);
									break;
								case 5:		//���ۿ�ʼ��
									tableIndID[1].GetCell(5, vRow[0], nDate);
									break;
								case 6:		//�����ʼ��
									tableIndID[1].GetCell(4, vRow[0], nDate);
									break;
								default:	//ת��������
									tableIndID[1].GetCell(3, vRow[0], nDate);
									break;
								}
								if(nDate > 0)
								{
									//�޸�Ϊ8λ��--20080708--
									strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
									//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								}
								break;
							}
							break;
						case 111:	//��תծ������Ϣ
							{
								vdReq.data_type = dtype_val_string;
								strResult = _T("");
								if(reqTXFunction.iSubItemID == 2)
								{
									//m_tableBondTmp//������id
									if(m_tableBondTmp.GetRowCount() == 0)
									{
										//--------------�漰�����ļ����أ��ж�--------------------
										pBusiness->m_pLogicalBusiness->GetData(m_tableBondTmp, true);
									}
#ifdef _DEBUG
									CString strTable = m_tableBondTmp.TableToString();
									Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
									nSecurity1ID = (INT)pBusiness->m_pSecurity->GetSecurity1Id();
									vRow.clear();
									m_tableBondTmp.Find(0, nSecurity1ID, vRow);
									if(vRow.size() != 1)
									{
										break;
									}
									m_tableBondTmp.GetCell(1, vRow[0], strResult);
									break;
								}
								iRowNo = GetDateIndicator_Row((INT)(reqTXFunction.nSecurityID), 0, &vBondIPOInfo);
								if(iRowNo >= 0)
								{
									switch(reqTXFunction.iSubItemID)
									{
									case 1:		//���з�ʽ
										tableIndDate[5].GetCell(5, iRowNo, iVal);
										if(iVal > 0)
										{
											strResult = Tx::Data::TypeMapManage::GetInstance()->GetDatByID(TYPE_BOND_IPO_TYPE, iVal);
										}
										break;
									case 3:		//������
										tableIndDate[5].GetCell(11, iRowNo, strValue);
										if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
										{
											strResult = strValue;
										}
										break;
									case 4:		//������ʽ
										tableIndDate[5].GetCell(12, iRowNo, strValue);
										if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
										{
											strResult = strValue;
										}
										break;
									case 5:		//�����Ƽ���
										tableIndDate[5].GetCell(13, iRowNo, strValue);
										if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
										{
											strResult = strValue;
										}
										break;
									case 6:		//��������
										tableIndDate[5].GetCell(9, iRowNo, strValue);
										if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
										{
											strResult = strValue;
										}
										break;
									case 7:		//ծȯ���õȼ�
										tableIndDate[5].GetCell(8, iRowNo, strValue);
										if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
										{
											strResult = strValue;
										}
										break;
									case 8:		//������������
										tableIndDate[5].GetCell(10, iRowNo, strValue);
										if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
										{
											strResult = strValue;
										}
										break;
									case 9:
									case 10:
									case 11:
									case 12:
										{
											CString strPath = pBusiness->DownloadXmlFile( pBusiness->m_pSecurity->GetSecurity1Id(),11004);
											//CString strDataMean = _T("ծȯ_������Ϣ_��תծ");
											//Tx::Core::Table resTable;
											//int m_iSecId = pBusiness->m_pSecurity->GetId();
											//pBusiness->DownloadXmlFile(reqTXFunction.nSecurityID,11004);
											//CString strPath = Tx::Core::Manage::GetInstance()->m_pSystemPath->GetXMLPath() + _T("\\11004\\");
											//CString strFileName = _T("");
											//strFileName.Format(_T("%d.xml"),pBusiness->m_pSecurity->GetSecurity1Id());
											//strPath += strFileName;
											switch( reqTXFunction.iSubItemID )
											{	
											case 9:
												strResult = GetXmlData( strPath,_T("T42"));
												break;
											case 10:
												strResult = GetXmlData( strPath,_T("T43"));
												break;
											case 11:
												strResult = GetXmlData( strPath,_T("T44"));
												break;
											case 12:
												strResult = GetXmlData( strPath,_T("T45"));
												break;
											}	
										}
										break;
									default:
										break;
									}
								}
							}
							break;
							//vdReq.data_type = dtype_val_string;
							//if(reqTXFunction.iSubItemID == 2)
							//{	//������id
							//	if(tableIndID[1].GetRowCount() == 0)
							//	{
							//		//--------------�漰�����ļ����أ��ж�--------------------
							//		pBusiness->m_pLogicalBusiness->GetData(tableIndID[1], true);
							//		//--------------wangzhy----20080417----------------------
							//		//break;
							//	}
							//	nSecurity1ID = (INT)pBusiness->m_pSecurity->GetSecurity1Id();
							//	vRow.clear();
							//	tableIndID[1].Find(0, nSecurity1ID, vRow);
							//	if(vRow.size() != 1)
							//	{
							//		break;
							//	}
							//	tableIndID[1].GetCell(6, vRow[0], nInstitution);
							//	if(nInstitution > 0)
							//	{
							//		mapIntStr.clear();
							//		Tx::Data::TypeMapManage::GetInstance()->GetTypeMap(TYPE_INSTITUTION_CHINALONGNAME, mapIntStr);
							//		iterIntStr = mapIntStr.find(nInstitution);
							//		if(iterIntStr != mapIntStr.end())
							//		{
							//			strValue = iterIntStr->second;
							//			if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
							//			{
							//				strResult = strValue;
							//			}
							//		}
							//	}
							//	break;
							//}
							//iRowNo = GetDateIndicator_Row((INT)(reqTXFunction.nSecurityID), 0, &vBondIPOInfo);
							//if(iRowNo >= 0)
							//{
							//	switch(reqTXFunction.iSubItemID)
							//	{
							//	case 1:		//���з�ʽ
							//		tableIndDate[5].GetCell(5, iRowNo, iVal);
							//		if(iVal > 0)
							//		{
							//			strResult = Tx::Data::TypeMapManage::GetInstance()->GetDatByID(TYPE_BOND_IPO_TYPE, iVal);
							//		}
							//		break;
							//	case 3:		//������
							//		tableIndDate[5].GetCell(11, iRowNo, strValue);
							//		if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
							//		{
							//			strResult = strValue;
							//		}
							//		break;
							//	case 4:		//������ʽ
							//		tableIndDate[5].GetCell(12, iRowNo, strValue);
							//		if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
							//		{
							//			strResult = strValue;
							//		}
							//		break;
							//	case 5:		//�����Ƽ���
							//		tableIndDate[5].GetCell(13, iRowNo, strValue);
							//		if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
							//		{
							//			strResult = strValue;
							//		}
							//		break;
							//	case 6:		//��������
							//		tableIndDate[5].GetCell(9, iRowNo, strValue);
							//		if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
							//		{
							//			strResult = strValue;
							//		}
							//		break;
							//	case 7:		//ծȯ���õȼ�
							//		tableIndDate[5].GetCell(8, iRowNo, strValue);
							//		if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
							//		{
							//			strResult = strValue;
							//		}
							//		break;
							//	case 8:		//������������
							//		tableIndDate[5].GetCell(10, iRowNo, strValue);
							//		if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
							//		{
							//			strResult = strValue;
							//		}
							//		break;
							//	case 9:
							//	case 10:
							//	case 11:
							//	case 12:
							//		{			
							//			CString strPath = pBusiness->DownloadXmlFile( pBusiness->m_pSecurity->GetSecurity1Id(),11004);
							//			//CString strPath = Tx::Core::Manage::GetInstance()->m_pSystemPath->GetXMLPath() + _T("\\11004\\");
							//			//CString strFileName = _T("");
							//			//strFileName.Format(_T("%d.xml"),pBusiness->m_pSecurity->GetSecurity1Id());
							//			//strPath += strFileName;
							//			switch( reqTXFunction.iSubItemID )
							//			{	
							//				case 9:
							//					strResult = GetXmlData( strPath,_T("T42"));
							//					break;
							//				case 10:
							//					strResult = GetXmlData( strPath,_T("T43"));
							//					break;
							//				case 11:
							//					strResult = GetXmlData( strPath,_T("T44"));
							//					break;
							//				case 12:
							//					strResult = GetXmlData( strPath,_T("T45"));
							//					break;

							//			}	
							//		}
							//		break;
							//	default:
							//		break;
							//	}
							//}
							//break;
						case 112:	//��תծ��Ӧ��Ʊ����
							vdReq.data_type = dtype_val_string;
							if(tableIndID[1].GetRowCount() == 0)
							{
								//--------------�漰�����ļ����أ��ж�--------------------
								pBusiness->m_pLogicalBusiness->GetData(tableIndID[1], true);
								//--------------wangzhy----20080417----------------------
								//break;
							}
							nSecurity1ID = (INT)pBusiness->m_pSecurity->GetSecurity1Id();
							vRow.clear();
							tableIndID[1].Find(0, nSecurity1ID, vRow);
							if(vRow.size() > 0)
							{
								tableIndID[1].GetCell(1, vRow[0], iTransObjID);
								if(iTransObjID > 0)
								{
									pSecurity = (Tx::Data::SecurityQuotation *)GetSecurity((LONG)iTransObjID);
									if(pSecurity != NULL)
									{
										strResult = pSecurity->GetCode(true);
									}
								}
							}
							break;
							//--wangzhy--20080604
						case 44:	//��Ӫҵ������ҵ��ֲ�������
						case 45:	//��Ӫҵ������ҵ��ֲ�����Ŀ����
						case 46:	//�ʲ���ֵ׼��������
						case 47:	//�Ǿ������������Ŀ����
						case 48:	//�Ǿ������������Ŀ���
						case 49:	//Ӧ���ʿ������
						case 50:	//Ӧ���ʿ������
						case 51:	//������ñ���Ŀ����
						case 52:	//������ñ���Ŀ���
							FindItemOfReq(&reqTXFunction, strReq, 14);	//FuncID��SubItemID��IndicatorID��������ȡ��Ƽ����ȡ���Ŀ���
							nInstitution = (INT)pBusiness->m_pSecurity->GetInstitutionId();
							switch( reqTXFunction.iFuncID )
							{
							case 44:	//��Ӫҵ������ҵ��ֲ�������
							case 45:	//��Ӫҵ������ҵ��ֲ�����Ŀ����
								{
								if ( reqTXFunction.iFuncID == 45 )
									vdReq.data_type = dtype_val_string;
								else
									vdReq.data_type = dtype_double;

								m_pRevenue = NULL;
								strResult = Con_strInvalid;
								nAccountingID = nInstitution * 100 + (INT64)reqTXFunction.iSubjectID;
								//�����������ļ�	
								bLoaded = true;
								if(m_pRevenueDataFile!=NULL)
									bLoaded = m_pRevenueDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30079,true);
								if ( bLoaded )
									m_pRevenue = m_pRevenueDataFile->GetDataByObj(nAccountingID,false);
								if ( m_pRevenue == NULL )
									break;
								if ( reqTXFunction.iFuncID == 45 )
									strResult = m_pRevenue->bName;
								else
								{
									double dValue = 0.0;
									double dSumValue = 0.0;
									switch( reqTXFunction.iSubItemID )
									{
									case 1:
										if ( m_pRevenue->dSR != Con_doubleInvalid)
											strResult.Format(_T("%.2f"),m_pRevenue->dSR);
										break;
									case 2:
										{
										dValue = m_pRevenue->dSR;
										nAccountingID = nInstitution * 100 + 1;
										bool bLoaded = true;
										if(m_pRevenueDataFile!=NULL)
											bLoaded = m_pRevenueDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30079,true);
										if ( bLoaded )
										{
											m_pRevenue = m_pRevenueDataFile->GetDataByObj(nAccountingID,false);
											while ( m_pRevenue->iDate / 100 == nInstitution )
											{
												dSumValue += m_pRevenue->dSR;
												m_pRevenue++;
											}
										}	
										strResult.Format(_T("%.2f"),dValue*100/dSumValue);
										}
										break;
									case 3:
										if ( m_pRevenue->dCB != Con_doubleInvalid)
											strResult.Format(_T("%.2f"),m_pRevenue->dCB);
										break;
									case 4:
										if ( m_pRevenue->dML != Con_doubleInvalid)
											strResult.Format(_T("%.2f"),m_pRevenue->dML);
										break;
									case 5:
										{

										dValue = m_pRevenue->dML;
										nAccountingID = nInstitution * 100 + 1;
										bool bLoaded = true;
										if(m_pRevenueDataFile!=NULL)
											bLoaded = m_pRevenueDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30079,true);
										if ( bLoaded )
										{
											m_pRevenue = m_pRevenueDataFile->GetDataByObj(nAccountingID,false);
											while ( m_pRevenue->iDate / 100 == nInstitution )
											{
												dSumValue += m_pRevenue->dML;
												m_pRevenue++;
											}
										}	
										strResult.Format(_T("%.2f"),dValue*100/dSumValue);
										}
										break;
									case 6:
										if ( m_pRevenue->dMLL != Con_doubleInvalid)
											strResult.Format(_T("%.2f"),m_pRevenue->dMLL);
										break;
									case 7:
										if ( m_pRevenue->dMLLBH != Con_doubleInvalid)
											strResult.Format(_T("%.2f"),m_pRevenue->dMLLBH);
										break;
									default:
										break;
									}
								}
								}
								break;
							case 46:	//�ʲ���ֵ׼��������
								{
								vdReq.data_type = dtype_double;
								//nAccountingID = (INT64)reqTXFunction.nFYear * 1000000 + (INT64)reqTXFunction.nFQuarter*100 + (INT64)reqTXFunction.iSubItemID;
								iSubItemID = (int)reqTXFunction.iSubItemID-1;
								m_pDepreciation = NULL;
								//strResult = Con_strInvalid;
								nAccountingID = nInstitution * 100 + (INT64)reqTXFunction.iSubjectID;
								//�����������ļ�	
								bool bLoaded = true;
								if(m_pDepreciationDataFile!=NULL)
									bLoaded = m_pDepreciationDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30081,true);
								if ( bLoaded )
									m_pDepreciation = m_pDepreciationDataFile->GetDataByObj(nAccountingID,false);
								if ( m_pDepreciation == NULL )
									break;
								if ( m_pDepreciation->dAsset[iSubItemID] != Con_doubleInvalid )
									strResult.Format(_T("%.2f"),m_pDepreciation->dAsset[iSubItemID]);
								}
								break;
							case 47:	//�Ǿ������������Ŀ����
							case 48:	//�Ǿ������������Ŀ���
								{

								if ( reqTXFunction.iFuncID == 47 )
									vdReq.data_type = dtype_val_string;
								else
									vdReq.data_type = dtype_double;
								//--������ƴ������Ҫ�����ݵ�����
								//nAccountingID = (INT64)reqTXFunction.nFYear * 1000000 + (INT64)reqTXFunction.nFQuarter * 100 + (INT64)reqTXFunction.iSubjectID;
								//iSubItemID = (int)reqTXFunction.iSubItemID-1;
								m_pGainsAndLosses = NULL;
								//strResult = Con_strInvalid;

								nAccountingID = nInstitution * 100 + (INT64)reqTXFunction.iSubjectID;
								bool bLoaded = true;
								if(m_pGainsAndLossesDataFile!=NULL)
									bLoaded = m_pGainsAndLossesDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30086,true);
								if ( bLoaded )
									m_pGainsAndLosses = m_pGainsAndLossesDataFile->GetDataByObj(nAccountingID,false);

								if ( m_pGainsAndLosses == NULL )
									break;
								if ( reqTXFunction.iFuncID == 47 )
									strResult = m_pGainsAndLosses->bName;
								else
									strResult.Format(_T("%.2f"),m_pGainsAndLosses->dLosses);
								}
								break;
							case 49:	//Ӧ���ʿ������
							case 50:	//Ӧ���ʿ������
								{
								if ( reqTXFunction.iFuncID == 49 )
									vdReq.data_type = dtype_val_string;
								else
									vdReq.data_type = dtype_double;
								iSubItemID = (int)reqTXFunction.iSubItemID-1;
								m_pAccountReceivable = NULL;
								//strResult = Con_strInvalid;
								nAccountingID = nInstitution * 100 + (INT64)reqTXFunction.iSubjectID;
								bLoaded = true;
								if(m_pAccountsReceivableDataFile!=NULL)
									bLoaded = m_pAccountsReceivableDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30087,true);
								if ( bLoaded )
									m_pAccountReceivable = m_pAccountsReceivableDataFile->GetDataByObj(nAccountingID,false);
								if ( m_pAccountReceivable == NULL )
									break;
								if ( reqTXFunction.iFuncID == 49 )
									strResult = m_pAccountReceivable->bName;
								else
									if ( m_pAccountReceivable->dAccountsReceivable[iSubItemID+4] != Con_doubleInvalid )
										strResult.Format(_T("%.2f"),m_pAccountReceivable->dAccountsReceivable[iSubItemID+4]);
								}
								break;
							case 51:	//������ñ���Ŀ����
							case 52:	//������ñ���Ŀ���
								{
									if ( reqTXFunction.iFuncID == 51 )
										vdReq.data_type = dtype_val_string;
									else
										vdReq.data_type = dtype_double;
									iSubItemID = (int)reqTXFunction.iSubItemID;
									m_pFinanceCharge = NULL;	
									
									nAccountingID = nInstitution * 100 + (INT64)reqTXFunction.iSubjectID;
									bLoaded = true;
									if(m_pFinanceChargeDataFile!=NULL)
										bLoaded = m_pFinanceChargeDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30088,true);
									if ( bLoaded )
										m_pFinanceCharge = m_pFinanceChargeDataFile->GetDataByObj(nAccountingID,false);
								
									if ( m_pFinanceCharge == NULL )
										break;
									if ( reqTXFunction.iFuncID == 51 )
										strResult = m_pFinanceCharge->bName;
									else
									{
										if ( m_pFinanceCharge->dFinanceCharge[iSubItemID] != Con_doubleInvalid )
											strResult.Format(_T("%.2f"),m_pFinanceCharge->dFinanceCharge[iSubItemID]);
									}
								}
								break;
							default:
								break;
							}
							break;
							//------------------					
						case 117:	//ָ������
							vdReq.data_type = dtype_val_string;
							FindItemOfReq(&reqTXFunction, strReq, 4);	//ָ�����롢��������
							if(reqTXFunction.iINDEXSampleIdx == 0)
							{
								strResult.Format(_T("%d"), pBusiness->m_pSecurity->GetIndexConstituentDataCount());
							}else
							{
								pIndexConstituentData = pBusiness->m_pSecurity->GetIndexConstituentDataByIndex(reqTXFunction.iINDEXSampleIdx - 1);
								if(pIndexConstituentData != NULL && pIndexConstituentData->iSecurityId > 0 && pBusiness->m_pSecurity->GetIndexConstituentDataCount() >= reqTXFunction.iINDEXSampleIdx )
								{
									pSecurity = (Tx::Data::SecurityQuotation *)GetSecurity((LONG)(pIndexConstituentData->iSecurityId));
									if(pSecurity != NULL)
									{
										strResult = pSecurity->GetCode();
									}
								}
							}
							break;
						case 118:	//��Ʊ��֤�����ҵ����
							vdReq.data_type = dtype_val_string;
							FindItemOfReq(&reqTXFunction, strReq, 22);	//��ҵ�㼶
							nIndustryID = pBusiness->m_pSecurity->GetCSRC_IndustryId();
							iVal = 0;
							iIndexObjID = 0;
							while(nIndustryID > 0)
							{
								iTransObjID = iIndexObjID;
								iIndexObjID = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDEX_INDUSTRY, (INT)nIndustryID);
								if(iIndexObjID < 0)
								{
									iIndexObjID = iTransObjID;
								}
								if(reqTXFunction.nIndustryLevel > 0 && reqTXFunction.nIndustryLevel == iVal + 1)
								{
									break;
								}
								iVal ++;
								nIndustryID = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRY_PARENT, (INT)nIndustryID);
							}
							if(iIndexObjID > 0)
							{
								pSecurity = (Tx::Data::SecurityQuotation *)GetSecurity((LONG)iIndexObjID);
								if(pSecurity != NULL)
								{
									strResult = pSecurity->GetCode();
								}
							}
							break;
						case 119:	//��Ʊ��������ҵ����
							vdReq.data_type = dtype_val_string;
							FindItemOfReq(&reqTXFunction, strReq, 22);	//��ҵ�㼶
							nIndustryID = pBusiness->m_pSecurity->GetTxSec_IndustryId();
							iVal = 0;
							iIndexObjID = 0;
							while(nIndustryID > 0)
							{
								iTransObjID = iIndexObjID;
								iIndexObjID = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDEX_INDUSTRY, (INT)nIndustryID);
								if(iIndexObjID < 0)
								{
									iIndexObjID = iTransObjID;
								}
								if(reqTXFunction.nIndustryLevel > 0 && reqTXFunction.nIndustryLevel == iVal + 1)
								{
									break;
								}
								iVal ++;
								nIndustryID = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRY_PARENT, (INT)nIndustryID);
							}
							if(iIndexObjID > 0)
							{
								pSecurity = (Tx::Data::SecurityQuotation *)GetSecurity((LONG)iIndexObjID);
								if(pSecurity != NULL)
								{
									strResult = pSecurity->GetCode();
								}
							}
							break;
						case 120:	//�����ھ���
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 5);	//StartDate��EndDate
							dValue1 = GetSumAmount(pBusiness, reqTXFunction.nStartDate, reqTXFunction.nEndDate);
							dValue2 = GetSumVolume(pBusiness, reqTXFunction.nStartDate, reqTXFunction.nEndDate);
							if(dValue1 > 0 && dValue2 > 0)    
							{
								strResult.Format(_T("%.2f"), dValue1 / dValue2);
							}
							break;
						case 121:	//��������ȷֺ��ܶ�
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 21);	//StartDate��EndDate���������
							dValue = 0;
							if(pBusiness->m_pSecurity->IsStock())
							{	//��Ʊ�ֺ�
								for(i = 0; (INT)i < pBusiness->m_pSecurity->GetStockBonusDataCount(); i++)
								{
									pStockBonusData = pBusiness->m_pSecurity->GetStockBonusDataByIndex((INT)i);
									if(reqTXFunction.nStartDate <= pStockBonusData->register_date && 
										reqTXFunction.nEndDate >= pStockBonusData->register_date)
									{
										if(reqTXFunction.nFYear > 0 && reqTXFunction.nFYear != (pStockBonusData->year / 10000))
										{
											continue;
										}
										pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(pStockBonusData->register_date);
										if(pTxShareData != NULL)
										{
											dValue1 = pTxShareData->TotalShare;
										}else
										{
											dValue1 = 0;
										}
										dValue2 = pStockBonusData->pxshu;
										if(pStockBonusData->pxshu > 0)
										{
											dValue2 = pStockBonusData->pxshu;
										}else
										{
											dValue2 = 0;
										}
										dValue += dValue1 * dValue2;
									}
								}
							}else if(pBusiness->m_pSecurity->IsFund())
							{	//����ֺ�
								for(i = 0; (INT)i < pBusiness->m_pSecurity->GetFundBonusDataCount(); i++)
								{
									pFundBonusData = pBusiness->m_pSecurity->GetFundBonusDataByIndex((INT)i);
									if(reqTXFunction.nStartDate <= pFundBonusData->book_date && 
										reqTXFunction.nEndDate >= pFundBonusData->book_date)
									{
										if(reqTXFunction.nFYear > 0 && reqTXFunction.nFYear != pFundBonusData->year)
										{
											continue;
										}
										if(pFundBonusData->fact > 0)
										{
											dValue += pFundBonusData->fact;
										}
									}
								}
							}
							if(dValue > 0)
							{
								strResult.Format(_T("%.2f"), dValue);
							}
							break;
						case 135:	//�����������Ϣ�ܶ�
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 21);	//StartDate��EndDate���������
							dValue = 0;
							if(pBusiness->m_pSecurity->IsBond())
							{
								pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
								if(pBondNewInfo != NULL && pBondNewInfo->share > 0)
								{
									for(i = 0; (INT)i < pBusiness->m_pSecurity->GetBondCashFlowDataCount(); i++)
									{
										pBondCashFlowData = pBusiness->m_pSecurity->GetBondCashFlowDataByIndex((INT)i);
										if(reqTXFunction.nStartDate <= pBondCashFlowData->F_BOND_BOOK_DATE && 
											reqTXFunction.nEndDate >= pBondCashFlowData->F_BOND_BOOK_DATE)
										{
											if(reqTXFunction.nFYear > 0 && reqTXFunction.nFYear != pBondCashFlowData->F_YEAR)
											{
												continue;
											}
											if(pBondCashFlowData->F_CASH > 0)
											{
												dValue += pBondNewInfo->share * pBondCashFlowData->F_CASH / 100;
											}
										}
									}
								}
							}
							if(dValue > 0)
							{
								strResult.Format(_T("%.2f"), dValue);
							}
							break;
						case 133:	//�����ڷֺ��ܶ�
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 5);	//StartDate��EndDate
							dValue = 0;
							if(pBusiness->m_pSecurity->IsStock())
							{	//��Ʊ�ֺ�
								for(i = 0; (INT)i < pBusiness->m_pSecurity->GetStockBonusDataCount(); i++)
								{
									pStockBonusData = pBusiness->m_pSecurity->GetStockBonusDataByIndex((INT)i);
									if(reqTXFunction.nStartDate <= pStockBonusData->register_date && 
										reqTXFunction.nEndDate >= pStockBonusData->register_date)
									{
										pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(pStockBonusData->register_date);
										if(pTxShareData != NULL)
										{
											dValue1 = pTxShareData->TotalShare;
										}else
										{
											dValue1 = 0;
										}
										if(pStockBonusData->pxshu > 0)
										{
											dValue2 = pStockBonusData->pxshu;
										}else
										{
											dValue2 = 0;
										}
										dValue += dValue1 * dValue2;
									}
								}
							}else if(pBusiness->m_pSecurity->IsFund())
							{	//����ֺ�
								for(i = 0; (INT)i < pBusiness->m_pSecurity->GetFundBonusDataCount(); i++)
								{
									pFundBonusData = pBusiness->m_pSecurity->GetFundBonusDataByIndex((INT)i);
									if(reqTXFunction.nStartDate <= pFundBonusData->book_date && 
										reqTXFunction.nEndDate >= pFundBonusData->book_date)
									{
										if(pFundBonusData->fact > 0)
										{
											dValue += pFundBonusData->fact;
										}
									}
								}
							}
							if(dValue > 0)
							{
								strResult.Format(_T("%.2f"), dValue);
							}
							break;
						case 136:	//��������Ϣ�ܶ�
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 5);	//StartDate��EndDate
							dValue = 0;
							if(pBusiness->m_pSecurity->IsBond())
							{
								pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
								if(pBondNewInfo != NULL && pBondNewInfo->share > 0)
								{
									for(i = 0; (INT)i < pBusiness->m_pSecurity->GetBondCashFlowDataCount(); i++)
									{
										pBondCashFlowData = pBusiness->m_pSecurity->GetBondCashFlowDataByIndex((INT)i);
										if(reqTXFunction.nStartDate <= pBondCashFlowData->F_BOND_BOOK_DATE && 
											reqTXFunction.nEndDate >= pBondCashFlowData->F_BOND_BOOK_DATE)
										{
											if(pBondCashFlowData->F_CASH > 0)
											{
												dValue += pBondNewInfo->share * pBondCashFlowData->F_CASH / 100;
											}
										}
									}
								}
							}
							if(dValue > 0)
							{
								strResult.Format(_T("%.2f"), dValue);
							}
							break;
						case 134:	//��ȷֺ��ܶ�
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 20);	//�������
							dValue = 0;
							if(pBusiness->m_pSecurity->IsStock())
							{	//��Ʊ�ֺ�
								for(i = 0; (INT)i < pBusiness->m_pSecurity->GetStockBonusDataCount(); i++)
								{
									pStockBonusData = pBusiness->m_pSecurity->GetStockBonusDataByIndex((INT)i);
									if(pStockBonusData->register_date <= 0)
									{
										continue;
									}
									if(reqTXFunction.nFYear != (pStockBonusData->year / 10000))
									{
										continue;
									}
									pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(pStockBonusData->register_date);
									if(pTxShareData != NULL)
									{
										dValue1 = pTxShareData->TotalShare;
									}else
									{
										dValue1 = 0;
									}
									if(pStockBonusData->pxshu > 0)
									{
										dValue2 = pStockBonusData->pxshu;
									}else
									{
										dValue2 = 0;
									}
									dValue += dValue1 * dValue2;
								}
							}else if(pBusiness->m_pSecurity->IsFund())
							{	//����ֺ�
								for(i = 0; (INT)i < pBusiness->m_pSecurity->GetFundBonusDataCount(); i++)
								{
									pFundBonusData = pBusiness->m_pSecurity->GetFundBonusDataByIndex((INT)i);
									if(pFundBonusData->book_date <= 0)
									{
										continue;
									}
									if(reqTXFunction.nFYear != pFundBonusData->year)
									{
										continue;
									}
									if(pFundBonusData->fact > 0)
									{
										dValue += pFundBonusData->fact;
									}
								}
							}
							if(dValue > 0)
							{
								strResult.Format(_T("%.2f"), dValue);
							}
							break;
						case 137:	//�����Ϣ�ܶ�
							vdReq.data_type = dtype_double;
							if(!pBusiness->m_pSecurity->IsBond())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 20);	//�������
							dValue = 0;
							if(pBusiness->m_pSecurity->IsBond())
							{
								pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
								if(pBondNewInfo != NULL && pBondNewInfo->share > 0)
								{
									for(i = 0; (INT)i < pBusiness->m_pSecurity->GetBondCashFlowDataCount(); i++)
									{
										pBondCashFlowData = pBusiness->m_pSecurity->GetBondCashFlowDataByIndex((INT)i);
										if(pBondCashFlowData->F_BOND_BOOK_DATE <= 0)
										{
											continue;
										}
										if(reqTXFunction.nFYear != pBondCashFlowData->F_YEAR)
										{
											continue;
										}
										if(pBondCashFlowData->F_CASH > 0)
										{
											dValue += pBondNewInfo->share * pBondCashFlowData->F_CASH / 100;
										}
									}
								}
							}
							if(dValue > 0)
							{
								strResult.Format(_T("%.2f"), dValue);
							}
							break;
						case 62:	//Ȩ֤-����������
							vdReq.data_type = dtype_double;
							if(!pBusiness->m_pSecurity->IsWarrant())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							iTransObjID = warrant.GetObjectSecurityId((INT)(reqTXFunction.nSecurityID));
							nStartDate = pBusiness->m_pSecurity->GetIPOListedDate();
							nEndDate = warrant.GetPowerEndDate((INT)(reqTXFunction.nSecurityID));
							if(iTransObjID > 0 && nStartDate > 0 && nEndDate > 0 && reqTXFunction.nDate >= nStartDate && reqTXFunction.nDate <= nEndDate)
							{
								business.GetSecurityNow((LONG)iTransObjID);
								dValue1 = GetKLine(&business, reqTXFunction.nDate, 4,reqTXFunction.nSecurityID);
								Tx::Business::ComputingWarrant computingWarrant;
								dValue2 = computingWarrant.GetLastedPeriod( reqTXFunction.nSecurityID,reqTXFunction.nDate );
								//dValue2 = (DOUBLE)(warrant.GetRemainsDays((INT)(reqTXFunction.nSecurityID), reqTXFunction.nDate)) / 365;
								//Ȩ֤�۸�
								dValue3 = pBusiness->m_pSecurity->GetClosePrice( reqTXFunction.nDate );
								//��ǰ��Ȩ����
								double dDivid = 1.0;
								int nNow = COleDateTime::GetCurrentTime().GetYear()*10000 + COleDateTime::GetCurrentTime().GetMonth()*100 + COleDateTime::GetCurrentTime().GetDay();
								dDivid = business.m_pSecurity->GetExdividendScale( reqTXFunction.nDate,nNow,true );
								double dExRatio = warrant.GetPowerRatio( reqTXFunction.nSecurityID );
								dExRatio *= dDivid;
								double dMarketValue = dValue3 / dExRatio;
								dValue = warrant.GetSigma(!pBusiness->m_pSecurity->IsWarrant_Buy(), dValue1, warrant.GetPowerPrice((INT)(reqTXFunction.nSecurityID)), 0.018, 0.0, dValue2, dMarketValue);
								strResult.Format(_T("%.3f"), dValue);
							}
							break;
						case 63:	//Ȩ֤-�����
							vdReq.data_type = dtype_double;
							if(!pBusiness->m_pSecurity->IsWarrant())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							iTransObjID = warrant.GetObjectSecurityId((INT)(reqTXFunction.nSecurityID));
							nStartDate = pBusiness->m_pSecurity->GetIPOListedDate();
							nEndDate = warrant.GetPowerEndDate((INT)(reqTXFunction.nSecurityID));
							if(iTransObjID > 0 && nStartDate > 0 && nEndDate > 0 && reqTXFunction.nDate >= nStartDate && reqTXFunction.nDate <= nEndDate)
							{
								business.GetSecurityNow((LONG)iTransObjID);
								dValue1 = GetKLine(pBusiness, reqTXFunction.nDate, 4,reqTXFunction.nSecurityID);
								dValue2 = GetKLine(&business, reqTXFunction.nDate, 4,reqTXFunction.nSecurityID);
								dValue = warrant.GetPremiumRate(dValue1, dValue2, 
									warrant.GetPowerPrice((INT)(reqTXFunction.nSecurityID)),
									pBusiness->m_pSecurity->IsWarrant_Buy(), 
									warrant.GetPowerRatio((INT)(reqTXFunction.nSecurityID)));
								strResult.Format(_T("%.2f"), dValue);
							}
							break;
						case 64:	//Ȩ֤-�ܸ˱���
							vdReq.data_type = dtype_double;
							if(!pBusiness->m_pSecurity->IsWarrant())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							iTransObjID = warrant.GetObjectSecurityId((INT)(reqTXFunction.nSecurityID));
							nStartDate = pBusiness->m_pSecurity->GetIPOListedDate();
							nEndDate = warrant.GetPowerEndDate((INT)(reqTXFunction.nSecurityID));
							if(iTransObjID > 0 && nStartDate > 0 && nEndDate > 0 && reqTXFunction.nDate >= nStartDate && reqTXFunction.nDate <= nEndDate)
							{
								business.GetSecurityNow((LONG)iTransObjID);
								dValue1 = GetKLine(&business, reqTXFunction.nDate, 4,reqTXFunction.nSecurityID);
								dValue2 = GetKLine(pBusiness, reqTXFunction.nDate, 4,reqTXFunction.nSecurityID);
								dValue = warrant.GetGearRate(dValue1, dValue2, warrant.GetPowerRatio((INT)(reqTXFunction.nSecurityID)));
								strResult.Format(_T("%.2f"), dValue);
							}
							break;
						case 65:	//Ȩ֤-��Ȩ�۸�
							vdReq.data_type = dtype_float;
							if(!pBusiness->m_pSecurity->IsWarrant())
							{
								break;
							}
							dValue = warrant.GetPowerPrice((INT)(reqTXFunction.nSecurityID));
							if(dValue > 0)
							{
								strResult.Format(_T("%.2f"), dValue);
							}
							break;
						case 66:	//Ȩ֤-������
							vdReq.data_type = dtype_int4;
							if(!pBusiness->m_pSecurity->IsWarrant())
							{
								break;
							}
							nDate = warrant.GetPowerEndDate((INT)(reqTXFunction.nSecurityID));
							if(nDate > 0)
							{
								//�޸�Ϊ8λ��--20080708--
								strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
							}
							break;
						case 67:	//Ȩ֤-ʣ����
							vdReq.data_type = dtype_int4;
							if(!pBusiness->m_pSecurity->IsWarrant())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							nDate = warrant.GetRemainsDays((INT)(reqTXFunction.nSecurityID), reqTXFunction.nDate);
							if(nDate > 0)
							{
								strResult.Format(_T("%d"), nDate);
							}
							break;
						case 68:	//Ȩ֤-���ɴ���
							vdReq.data_type = dtype_val_string;
							if(!pBusiness->m_pSecurity->IsWarrant())
							{
								break;
							}
							iTransObjID = warrant.GetObjectSecurityId((INT)(reqTXFunction.nSecurityID));
							if(iTransObjID > 0)
							{
								pSecurity = (Tx::Data::SecurityQuotation *)GetSecurity((LONG)iTransObjID);
								if(pSecurity != NULL)
								{
									strResult = pSecurity->GetCode();
								}
							}
							break;
						case 69:	//Ȩ֤-��Ȩ����
							vdReq.data_type = dtype_double;
							if(!pBusiness->m_pSecurity->IsWarrant())
							{
								break;
							}
							dValue = warrant.GetPowerRatio((INT)(reqTXFunction.nSecurityID));
							if(dValue > 0)
							{
								strResult.Format(_T("%.4f"), dValue);
							}
							break;
						case 70:	//Ȩ֤-����ʣ������
							vdReq.data_type = dtype_double;
							if(!pBusiness->m_pSecurity->IsWarrant())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							dValue = GetDateIndicator_Value((INT)(reqTXFunction.nSecurityID), reqTXFunction.nDate, &vWarrantDV);
							if(dValue > 0)
							{
								strResult.Format(_T("%.0f"), dValue);
							}
							break;
						case 71:	//��������
							vdReq.data_type = dtype_val_string;
							strResult = _T("");
							if(!pBusiness->m_pSecurity->IsFund())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 2);	//secId subitemId
							if(reqTXFunction.iSubItemID == 0 || reqTXFunction.iSubItemID == 1)
							{
								if(pBusiness->m_pSecurity->IsFund_LOF())
									strResult = _T("LOF");
								else if(pBusiness->m_pSecurity->IsFund_ETF())
									strResult = _T("ETF");
								else if(pBusiness->m_pSecurity->IsFund_Open())
									strResult = _T("����ʽ");
								else if(pBusiness->m_pSecurity->IsFund_Close())
									strResult = _T("���ʽ");
							}
							else if(reqTXFunction.iSubItemID == 2)
							{
								//if(pBusiness->m_pSecurity->IsFund_Stock())
								//{
								//	strResult = _T("��Ʊ��");
								//	if(pBusiness->m_pSecurity->IsFundStyle_StockActive())
								//		strResult += _T("->������");
								//	else if(pBusiness->m_pSecurity->IsFundStyle_StockSteady())
								//		strResult += _T("->�Ƚ���");
								//	else if(pBusiness->m_pSecurity->IsFundStyle_StockIndex())
								//		strResult += _T("->��ָ����");
								//	else if(pBusiness->m_pSecurity->IsFundStyle_StockIndexEnhance())
								//		strResult += _T("->��ǿָ����");
								//}
								//else if(pBusiness->m_pSecurity->IsFund_Intermix())
								//{
								//	strResult = _T("�����");
								//	if(pBusiness->m_pSecurity->IsFundStyle_FlexibleCombination())
								//		strResult += _T("->���������");
								//	else if(pBusiness->m_pSecurity->IsFundStyle_FundCombination())
								//		strResult += _T("->����������");
								//	else if(pBusiness->m_pSecurity->IsFundStyle_CareCombination())
								//		strResult += _T("->����������");
								//	else if(pBusiness->m_pSecurity->IsFundStyle_PreserCombination())
								//		strResult += _T("->���������");
								//	else if(pBusiness->m_pSecurity->IsFundStyle_SpecialCombination())
								//		strResult += _T("->�ض�������");
								//}
								//else if(pBusiness->m_pSecurity->IsFund_Bond())
								//{
								//	strResult = _T("ծȯ��");
								//	if(pBusiness->m_pSecurity->IsFundStyle_PureBond())
								//		strResult += _T("->��ծ��");
								//	else if(pBusiness->m_pSecurity->IsFundStyle_LeanBond())
								//		strResult += _T("->ƫծ��");
								//}
								//else if(pBusiness->m_pSecurity->IsFund_Currency())
								//{
								//	strResult = _T("�����г�����");
								//}
								//else if(pBusiness->m_pSecurity->IsFund_QDII())
								//	strResult = _T("QDII");

								//bug:12439    2012-08-07
								pFundNewInfo = pBusiness->m_pSecurity->GetFundNewInfo();
								if(pFundNewInfo != NULL)
								{
									mapIntStr.clear();
									Tx::Data::TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX_NEW, mapIntStr);
									iterIntStr = mapIntStr.find(pFundNewInfo->style_id);
									if(iterIntStr != mapIntStr.end())
									{
										strValue = iterIntStr->second;
										if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
										{
											strResult = strValue;
										}
									}
								}
							}
							/*pFundNewInfo = pBusiness->m_pSecurity->GetFundNewInfo();
							if(pFundNewInfo != NULL)
							{
								mapIntStr.clear();
								Tx::Data::TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_TYPE_INDEX, mapIntStr);
								iterIntStr = mapIntStr.find(pFundNewInfo->type_id);
								if(iterIntStr != mapIntStr.end())
								{
									strValue = iterIntStr->second;
									if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
									{
										strResult = strValue;
									}
								}
							}*/
							break;
						case 122:	//������
							vdReq.data_type = dtype_val_string;
							if(!pBusiness->m_pSecurity->IsFund())
							{
								break;
							}
							pFundNewInfo = pBusiness->m_pSecurity->GetFundNewInfo();
							if(pFundNewInfo != NULL)
							{
								mapIntStr.clear();
								//modified by zhangxs 20091221---NewStyle
								//Tx::Data::TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX, mapIntStr);
								Tx::Data::TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX_NEW, mapIntStr);
								iterIntStr = mapIntStr.find(pFundNewInfo->style_id);
								if(iterIntStr != mapIntStr.end())
								{
									strValue = iterIntStr->second;
									if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
									{
										strResult = strValue;
									}
								}
							}
							break;
						case 73:	//����λ��ֵ
							vdReq.data_type = dtype_double;
							if(pBusiness->m_pSecurity->IsFund_Currency())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate < reqTXFunction.nDate)
							{
								break;
							}
							pFundNetValueData = pBusiness->m_pSecurity->GetFundNetValueDataByDate(reqTXFunction.nDate);
							if(pFundNetValueData == NULL)
							{
								nDataCount = pBusiness->m_pSecurity->GetFundNetValueDataCount();
								if(nDataCount > 0)
								{
									pFundNetValueData = pBusiness->m_pSecurity->GetFundNetValueDataByIndex(nDataCount - 1);
								}
							}
							if(pFundNetValueData != NULL)
							{
								if(pFundNetValueData->fNetvalue > 0)
								{
									strResult.Format(_T("%.4f"), pFundNetValueData->fNetvalue);
								}
							}
							break;
						case 74:	//�����ۼƵ�λ��ֵ
							vdReq.data_type = dtype_double;
							if(pBusiness->m_pSecurity->IsFund_Currency())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate < reqTXFunction.nDate)
							{
								break;
							}
							pFundNetValueData = pBusiness->m_pSecurity->GetFundNetValueDataByDate(reqTXFunction.nDate);
							if(pFundNetValueData == NULL)
							{
								nDataCount = pBusiness->m_pSecurity->GetFundNetValueDataCount();
								if(nDataCount > 0)
								{
									pFundNetValueData = pBusiness->m_pSecurity->GetFundNetValueDataByIndex(nDataCount - 1);
								}
							}
							if(pFundNetValueData != NULL)
							{
								if(pFundNetValueData->fNetvalueAcu > 0)
								{
									strResult.Format(_T("%.4f"), pFundNetValueData->fNetvalueAcu);
								}
							}
							break;
						case 76:	//�����г��������ÿ��ݻ�������
							vdReq.data_type = dtype_double;
							if(!pBusiness->m_pSecurity->IsFund_Currency())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate < reqTXFunction.nDate)
							{
								break;
							}
							iVal = pBusiness->m_pSecurity->GetFundGainDataIndexByDate(reqTXFunction.nDate);
							if(iVal >= 0)
							{
								if(iVal > 0)
								{
									iVal -- ;
								}
								pFundGainData = pBusiness->m_pSecurity->GetFundGainDataByIndex(iVal);
								if(pFundGainData != NULL)
								{
									if(pFundGainData->gain_ten_thousand_share > 0)
									{
										strResult.Format(_T("%.5f"), pFundGainData->gain_ten_thousand_share);
									}
								}
							}
							break;
						case 79:	//�����г���������7���껯������
							vdReq.data_type = dtype_double;
							if(!pBusiness->m_pSecurity->IsFund_Currency())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate < reqTXFunction.nDate)
							{
								break;
							}
							iVal = pBusiness->m_pSecurity->GetFundGainDataIndexByDate(reqTXFunction.nDate);
							if(iVal >= 0)
							{
								if(iVal > 0)
								{
									iVal -- ;
								}
								pFundGainData = pBusiness->m_pSecurity->GetFundGainDataByIndex(iVal);
								if(pFundGainData != NULL)
								{
									if(pFundGainData->year_yield_last_week > 0)
									{
										strResult.Format(_T("%.4f"), pFundGainData->year_yield_last_week);
									}
								}
							}
							break;
						case 77:	//�����������
							vdReq.data_type = dtype_double;
							//if(!pBusiness->m_pSecurity->IsFundTradeInMarket())
							//2012-09-20   Bug:13047 
							if(!pBusiness->m_pSecurity->IsFund())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							pFundNewInfo = pBusiness->m_pSecurity->GetFundNewInfo();
							if (pFundNewInfo == NULL)
							{
								break;
							}
							if(reqTXFunction.nDate < pBusiness->m_pSecurity->GetFundNewInfo()->ipo_date)//��û���̼�
								break;
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate < reqTXFunction.nDate)
							{
								break;
							}
							pFundNetValueData = pBusiness->m_pSecurity->GetFundNetValueDataByDate(reqTXFunction.nDate);
							if(pFundNetValueData != NULL)
							{
								if(reqTXFunction.nDate == pFundNetValueData->iDate)
								{	//��ֹ�����л���ֵ
									//dValue1 = pBusiness->m_pSecurity->GetClosePrice(reqTXFunction.nDate);
									//2012-10-16
									if(pBusiness->m_pSecurity->IsFund_Close() || pBusiness->m_pSecurity->IsFund_ETF() || pBusiness->m_pSecurity->IsFund_ETF())
										dValue1 = pBusiness->m_pSecurity->GetClosePrice(reqTXFunction.nDate);
									else
										dValue1 = pBusiness->m_pSecurity->GetOpenFundClosePrice(reqTXFunction.nDate);
									dValue2 = pFundNetValueData->fNetvalue;
									if(dValue1 > 0 && dValue2 > 0)
									{
										dValue = 100 * (dValue1 - dValue2) / dValue2;
										strResult.Format(_T("%.4f"), dValue);
									}
								}
							}
							break;
						case 78:	//����ʣ������(����)
							vdReq.data_type = dtype_int4;
							if(!pBusiness->m_pSecurity->IsFund_Close())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate <= reqTXFunction.nDate)
							{
								break;
							}
							pFundNewInfo = pBusiness->m_pSecurity->GetFundNewInfo();
							if(pFundNewInfo != NULL)
							{
								if(pFundNewInfo->hold_year > 0 && pFundNewInfo->setup_date > 0)
								{
									tmFrom = CTime(reqTXFunction.nDate / 10000, (reqTXFunction.nDate % 10000) / 100, reqTXFunction.nDate % 100, 0, 0, 0);
									tmTo = CTime(pFundNewInfo->setup_date / 10000 + pFundNewInfo->hold_year, (pFundNewInfo->setup_date % 10000) / 100, pFundNewInfo->setup_date % 100, 0, 0, 0);
									if(tmTo > tmFrom)
									{
										tsInterVal = tmTo - tmFrom;
										strResult.Format(_T("%d"), tsInterVal.GetDays());
									}
								}
							}
							break;
						case 80:	//����ֵ������
							vdReq.data_type = dtype_double;
							if(!pBusiness->m_pSecurity->IsFund())
							{
								break;
							}
							//BUG:13241   2012-09-27
							pFundNewInfo = pBusiness->m_pSecurity->GetFundNewInfo();
							if (pFundNewInfo == NULL)
							{
								break;
							}
							if(pBusiness->m_pSecurity->IsFund_Currency() || pFundNewInfo->style_id == 21)
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 5);	//StartDate��EndDate
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate < reqTXFunction.nEndDate)
							{
								break;
							}
							dValue = GetDatesIndicator_Value((INT)(reqTXFunction.nSecurityID), reqTXFunction.nStartDate, reqTXFunction.nEndDate, &vFundNAVGrowth);
							if(dValue != 0)
							{
								strResult.Format(_T("%.4f"), dValue);
							}
							break;
						case 81:	//IOPV(ETF����LOF����)
						case 286:   //IOPV(ETF����LOF����)
							vdReq.data_type = dtype_double;
							if(!(pBusiness->m_pSecurity->IsFund_ETF() || pBusiness->m_pSecurity->IsFund_LOF()))
							{
								break;
							}
							/*
							dValue = pBusiness->m_pSecurity->GetIOPV();
							if(dValue > 0)
							{
							strResult.Format(_T("%.4f"), dValue);
							}
							*/
							mapIntInt.clear();
							Tx::Data::TypeMapManage::GetInstance()->GetTypeMapITI(TYPE_IOPV_ID_ID, mapIntInt);
							iterIntInt = mapIntInt.find((INT)(reqTXFunction.nSecurityID));
							if(iterIntInt != mapIntInt.end())
							{
								if(business.GetSecurityNow((LONG)(iterIntInt->second)) != NULL)
								{
									if(pBusiness->m_pSecurity->IsValid())
									{
										dValue = business.m_pSecurity->GetClosePrice(true);
									}else
									{
										dValue = business.m_pSecurity->GetClosePrice(pBusiness->m_pSecurity->GetTradeDateLatest());
									}
									if(dValue > 0)
									{
										strResult.Format(_T("%.4f"), dValue);
									}
								}
							}
							break;
						case 82:	//�����й���
							vdReq.data_type = dtype_val_string;
							if(!pBusiness->m_pSecurity->IsFund())
							{
								break;
							}
							pFundNewInfo = pBusiness->m_pSecurity->GetFundNewInfo();
							if(pFundNewInfo != NULL)
							{
								mapIntStr.clear();
								Tx::Data::TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_BANK, mapIntStr);
								iterIntStr = mapIntStr.find(pFundNewInfo->trusteeship_bank);
								if(iterIntStr != mapIntStr.end())
								{
									strValue = iterIntStr->second;
									if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
									{
										strResult = strValue;
									}
								}
							}
							break;
						case 83:	//�����г�����������
							{
								vdReq.data_type = dtype_val_string;
								//BUG:13241   2012-09-27
								//if(!pBusiness->m_pSecurity->IsFund_Currency())
								pFundNewInfo = pBusiness->m_pSecurity->GetFundNewInfo();
								if (pFundNewInfo == NULL)
								{
									break;
								}
								if(pBusiness->m_pSecurity->IsFund_Currency() || pFundNewInfo->style_id == 21)
								{
								}
								else
									break;
								FindItemOfReq(&reqTXFunction, strReq, 5);	//StartDate��EndDate
								int nDateList = pBusiness->m_pSecurity->GetFundEstablishDate();
								if(nDateList > 0 && nDateList > reqTXFunction.nEndDate)
									break;
								nDate = pBusiness->m_pSecurity->GetStopedLastDate();
								if(nDate > 0 && nDate < reqTXFunction.nEndDate)
									break;
								dValue = GetMMFGrowth(pBusiness, reqTXFunction.nStartDate, reqTXFunction.nEndDate);
								if(dValue > 0)
									strResult.Format(_T("%.8f"), dValue);
							}							
							break;
						case 84:	//������������
							vdReq.data_type = dtype_int4;
							if(!pBusiness->m_pSecurity->IsFund())
							{
								break;
							}
							nDate = pBusiness->m_pSecurity->GetFundEstablishDate();
							if(nDate > 0)
							{
								//�޸�Ϊ8λ��--20080708--
								strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
							}
							break;
						case 85:	//ծȯ����
							vdReq.data_type = dtype_val_string;
							if(!pBusiness->m_pSecurity->IsBond())
							{
								break;
							}
							pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
							if(pBondNewInfo != NULL)
							{
								iVal = pBondNewInfo->bond_type;
								strResult = Tx::Data::TypeMapManage::GetInstance()->GetDatByID(TYPE_BOND_TYPE, iVal);
							}
							break;
						case 123:	//ծȯ��ʽ
							vdReq.data_type = dtype_val_string;
							if(!pBusiness->m_pSecurity->IsBond())
							{
								break;
							}
							pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
							if(pBondNewInfo != NULL)
							{
								iVal = pBondNewInfo->bond_form;
								strResult = Tx::Data::TypeMapManage::GetInstance()->GetDatByID(TYPE_BOND_FORM, iVal);
							}
							break;
						case 86:	//ծȯ������
							vdReq.data_type = dtype_int4;
							if(!pBusiness->m_pSecurity->IsBond())
							{
								break;
							}
							pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
							if(pBondNewInfo != NULL)
							{
								dValue = pBondNewInfo->share;
								strResult.Format(_T("%.0f"), dValue);
							}
							break;
						case 87:	//ծȯ��ֵ
							vdReq.data_type = dtype_double;
							if(!pBusiness->m_pSecurity->IsBond())
							{
								break;
							}
							pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
							if(pBondNewInfo != NULL)
							{
								dValue = pBondNewInfo->par_val;
								strResult.Format(_T("%.0f"), dValue);
							}
							break;
						case 88:	//ծȯƱ������
							{
								vdReq.data_type = dtype_double;
								if(!pBusiness->m_pSecurity->IsBond())
								{
									break;
								}
								CTime tCurrentDate = CTime::GetCurrentTime();
								int nCurrentDate = tCurrentDate.GetYear()*10000+tCurrentDate.GetMonth()*100+tCurrentDate.GetDay();
								FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
								if (nCurrentDate == (int)reqTXFunction.nDate)
								{
									pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
									if(pBondNewInfo != NULL)
									{
										dValue = pBondNewInfo->par_rate;
										if ( dValue >= 0.0 )
											strResult.Format(_T("%.4f"), dValue / 100);
									}
								}
								else
								{
									int count = pBusiness->m_pSecurity->GetBondPmllCount();
									float payInterst = Con_floatInvalid;
									if(count <= 0) 
										break;
									Tx::Data::BondPmll* pBondPmll = NULL;
									for (int i=0;i<count;i++)
									{	
										pBondPmll = pBusiness->m_pSecurity->GetBondPmllByIndex(i);
										if(pBondPmll == NULL)
											continue;
										if ((int)reqTXFunction.nDate >= pBondPmll->sdate)
										{
											payInterst = pBondPmll->payInterest;
											continue;
										}
										else
											break;
									}
									dValue = payInterst;
									if(dValue > Con_floatInvalid)
										strResult.Format(_T("%.4f"), dValue/100);
								}
							}
							break;
						case 89:	//ծȯ��ϢƵ��
							vdReq.data_type = dtype_double;
							if(!pBusiness->m_pSecurity->IsBond())
							{
								break;
							}
							pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
							if(pBondNewInfo != NULL)
							{
								iVal = pBondNewInfo->pay_interest_frequence;
								strResult.Format(_T("%d��1��"), iVal);
							}
							break;
						case 90:	//ծȯ��������
							vdReq.data_type = dtype_int4;
							if(!pBusiness->m_pSecurity->IsBond())
							{
								break;
							}
							pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
							if(pBondNewInfo != NULL)
							{
								dValue = pBondNewInfo->hold_year;
								strResult.Format(_T("%.0f"), dValue);
							}
							break;
						case 91:	//ծȯ��Ϣ����
							vdReq.data_type = dtype_int4;
							if(!pBusiness->m_pSecurity->IsBond())
							{
								break;
							}
							pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
							if(pBondNewInfo != NULL)
							{
								nDate = pBondNewInfo->begin_date;
								//�޸�Ϊ8λ��--20080708--
								strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
							}
							break;
						case 92:	//ծȯ��������
							vdReq.data_type = dtype_int4;
							if(!pBusiness->m_pSecurity->IsBond())
							{
								break;
							}
							pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
							if(pBondNewInfo != NULL)
							{
								nDate = pBondNewInfo->end_date;
								//�޸�Ϊ8λ��--20080708--
								strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
							}
							break;
						case 93:	//ծȯ������Ϣ��
							vdReq.data_type = dtype_int4;
							if(!pBusiness->m_pSecurity->IsBond())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate < reqTXFunction.nDate)
							{
								break;
							}
							pBondCashFlowData = pBusiness->m_pSecurity->GetBondCashFlowDataByDate(reqTXFunction.nDate);
							if(pBondCashFlowData != NULL)
							{
								nDate = pBondCashFlowData->F_LIPD;
								//�޸�Ϊ8λ��--20080708--
								strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
							}
							break;
						case 94:	//ծȯ���ڸ�Ϣ��
							vdReq.data_type = dtype_int4;
							if(!pBusiness->m_pSecurity->IsBond())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate < reqTXFunction.nDate)
							{
								break;
							}
							pBondCashFlowData = pBusiness->m_pSecurity->GetBondCashFlowDataByDate(reqTXFunction.nDate);
							if(pBondCashFlowData != NULL)
							{
								nDate = pBondCashFlowData->F_NIPD;
								//�޸�Ϊ8λ��--20080708--
								strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
							}
							break;
						case 95:	//ծȯӦ����Ϣ
							{
								vdReq.data_type = dtype_double;
								if(!pBusiness->m_pSecurity->IsBond())
								{
									break;
								}
								FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
								FindItemOfReq(&reqTXFunction, strReq, 39);  //Ӧ����Ϣ��־  2012-07-23

								bool bFlag;
								if ((int)reqTXFunction.iYJLX == 0)
									bFlag = true;
								else if ((int)reqTXFunction.iYJLX == 1)
								    bFlag = false;
								else
									break;
								nDate = pBusiness->m_pSecurity->GetStopedLastDate();
								if(nDate > 0 && nDate < reqTXFunction.nDate)
								{
									break;
								}
								//dValue = bond.GetInterest((INT)(reqTXFunction.nSecurityID), reqTXFunction.nDate);
								//2012-7-16
								dValue = bond.GetInterest_New((INT)(reqTXFunction.nSecurityID), reqTXFunction.nDate,bFlag);
								if(dValue >= 0)
								{
									strResult.Format(_T("%.4f"), dValue);
								}
							}
							break;
						case 96:	//ծȯ��Ϣ����
							vdReq.data_type = dtype_int4;
							if(!pBusiness->m_pSecurity->IsBond())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate < reqTXFunction.nDate)
							{
								break;
							}
							nDate = bond.GetInterestDays((INT)(reqTXFunction.nSecurityID), reqTXFunction.nDate);
							if(nDate > 0)
							{
								strResult.Format(_T("%d"), nDate);
							}
							break;
						case 97:	//ծȯʣ������
							vdReq.data_type = dtype_val_string;
							if(!pBusiness->m_pSecurity->IsBond())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate < reqTXFunction.nDate)
							{
								break;
							}
							pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
							if(pBondNewInfo != NULL)
							{
								nStartDate = reqTXFunction.nDate;
								nEndDate = pBondNewInfo->end_date;
								strValue = bond.GetRemnantsString(nStartDate, nEndDate);
								if(strValue.Compare(_T("-")) != 0)
								{
									strResult = strValue;
								}
							}
							break;
						case 98:	//ծȯYTM
						case 104:	//ծȯ��������
						case 105:	//ծȯ͹��
							{
								vdReq.data_type = dtype_double;
								if(!pBusiness->m_pSecurity->IsBond()) 
									break;
								FindItemOfReq(&reqTXFunction, strReq, 17);	//Date��Price
								FindItemOfReq(&reqTXFunction, strReq, 8);    //iPriceType
								nDate = pBusiness->m_pSecurity->GetStopedLastDate();
								if(nDate > 0 && nDate < reqTXFunction.nDate)
									break;
								CTime time = CTime::GetCurrentTime();
								int iTime = time.GetYear()*10000 + time.GetMonth()*100 + time.GetDay();		
								//��ʷYTM���������ڡ�͹��
								if (iTime == reqTXFunction.nDate)
								{
									double dYtm,dDur,dMDur,dCon;
									dYtm = Con_doubleInvalid;
									dDur = Con_doubleInvalid;
									dMDur = Con_doubleInvalid;
									dCon = Con_doubleInvalid;
									GetBondYTM(reqRealTime.nSecurityID,dYtm,dDur,dMDur,dCon,false);
									switch(reqTXFunction.iFuncID)
									{
									case 98:
										dValue = dYtm;
										if(dValue != Tx::Core::Con_doubleInvalid)
										{
											strResult.Format(_T("%.6f"), dValue*100);
										}
										break;
									case 104:
										dValue = dMDur;
										if(dValue != Tx::Core::Con_doubleInvalid)
										{
											strResult.Format(_T("%.6f"), dValue);
										}
										break;
									case 105:
										dValue = dCon;
										if(dValue != Tx::Core::Con_doubleInvalid)
										{
											strResult.Format(_T("%.6f"), dValue);
										}
										break;
									default:
										break;
									}
								}
								else
								{
									if(reqTXFunction.dPrice <= 0)
									{
										BondYTM* pBondYTM = pBusiness->m_pSecurity->GetBondYTMByDate(reqTXFunction.nDate);
										if (pBondYTM != NULL)
										{
											switch(reqTXFunction.iFuncID)
											{
											case 98:
												dValue = pBondYTM->fYtm;
												if(dValue != Tx::Core::Con_doubleInvalid)
												{
													strResult.Format(_T("%.6f"), dValue*100);
												}
												break;
											case 104:
												dValue = pBondYTM->fADur;
												if(dValue != Tx::Core::Con_doubleInvalid)
												{
													strResult.Format(_T("%.6f"), dValue);
												}
												break;
											case 105:
												dValue = pBondYTM->fCon;
												if(dValue != Tx::Core::Con_doubleInvalid)
												{
													strResult.Format(_T("%.6f"), dValue);
												}
												break;
											default:
												break;
											}
										}
									}
									else
									{
										//////////////////////////////////////////////////////////////////////////
										// 20131011 ���Ӽ۸����ͼ��۸���ж� xujf
										Tx::Business::TxBond  TxBond;			
										double dDur = 0.0, dMDur = 0.0, dCon = 0.0, dYTM = 0.0;
										TxBond.GetYTM(pBusiness->m_pSecurity, reqTXFunction.nDate, (float)reqTXFunction.dPrice, reqTXFunction.iPriceType, dYTM, dDur, dMDur, dCon);
										if(dYTM <= -100 || dDur <= -100 || dMDur < -100 || dCon <= -100)
											strResult.Format(_T("-"));
										else
										{
											switch(reqTXFunction.iFuncID)
											{
											case 98:
												strResult.Format(_T("%.6f"), dYTM*100);
												break;
											case 104:
												strResult.Format(_T("%.6f"), dDur);
												break;
											case 105:
												strResult.Format(_T("%.6f"), dCon);
												break;
											default:
												break;
											}
										}
									}
								}

							}
							break;

							//////////////////////////////////////////////////////////////////////////
							//FindItemOfReq(&reqTXFunction, strReq, 17);	//Date��Price
							//FindItemOfReq(&reqTXFunction, strReq, 8);    //iPriceType

							//if(reqTXFunction.dPrice == 0)
							//{
							//	pBondGeneral = pBusiness->m_pSecurity->GetBondGeneralByDate(reqTXFunction.nDate);
							//	if(pBondGeneral == NULL)
							//	{
							//		nDate = CTime::GetCurrentTime().GetYear() * 10000 + CTime::GetCurrentTime().GetMonth() * 100 + CTime::GetCurrentTime().GetDay();
							//		if(reqTXFunction.nDate == nDate && pBusiness->m_pSecurity->IsValid())
							//		{
							//			reqTXFunction.dPrice = GetKLine(pBusiness, reqTXFunction.nDate, 4,reqTXFunction.nSecurityID);
							//		}else
							//		{
							//			break;
							//		}
							//	}else
							//	{
							//		switch(reqTXFunction.iFuncID)
							//		{
							//		case 98:
							//			dValue = pBondGeneral->dYtm;
							//			if(dValue != Tx::Core::Con_doubleInvalid)
							//			{
							//				strResult.Format(_T("%.6f"), dValue );
							//			}
							//			break;
							//		case 104:
							//			dValue = pBondGeneral->dDuration;
							//			if(dValue != Tx::Core::Con_doubleInvalid)
							//			{
							//				strResult.Format(_T("%.6f"), dValue);
							//			}
							//			break;
							//		case 105:
							//			dValue = pBondGeneral->dConvexity;
							//			if(dValue != Tx::Core::Con_doubleInvalid)
							//			{
							//				strResult.Format(_T("%.6f"), dValue);
							//			}
							//			break;
							//		default:
							//			break;
							//		}
							//		break;
							//	}
							//}
							//if(reqTXFunction.dPrice <= 0)
							//{
							//	break;
							//}
							////if(pBusiness->m_pSecurity->IsBond_National())
							//if(reqTXFunction.iPriceType == 0) //Ŀǰծ���Ǿ��۽��ף����㵽�������ʼ۸�Ϊȫ��
							//{	//��ծʵ�о��۽��ף����Ǽ��㵽��������ʱծȯ�۸�Ϊȫ��
							//	//dValue = bond.GetInterest((INT)(reqTXFunction.nSecurityID), reqTXFunction.nDate);
							//	//2012-7-16  Ӧ����Ϣ(��)
							//	dValue = bond.GetInterest_New((INT)(reqTXFunction.nSecurityID), reqTXFunction.nDate,true);
							//	if(dValue < 0)
							//	{
							//		break;
							//	}
							//	reqTXFunction.dPrice += dValue;
							//}
							//bond.Calc((INT)(reqTXFunction.nSecurityID), reqTXFunction.nDate, (FLOAT)(reqTXFunction.dPrice));
							//switch(reqTXFunction.iFuncID)
							//{
							//case 98:
							//	dValue = bond.Get_YTM();
							//	if(dValue != Tx::Core::Con_doubleInvalid)
							//	{
							//		strResult.Format(_T("%.6f"), dValue*100);
							//	}
							//	break;
							//case 104:
							//	dValue = bond.Get_MDURATION();
							//	if(dValue != Tx::Core::Con_doubleInvalid)
							//	{
							//		strResult.Format(_T("%.6f"), dValue);
							//	}
							//	break;
							//case 105:
							//	dValue = bond.Get_CONVEXITY();
							//	if(dValue != Tx::Core::Con_doubleInvalid)
							//	{
							//		strResult.Format(_T("%.6f"), dValue);
							//	}
							//	break;
							//default:
							//	break;
							//}
							//break;

							//////////////////////////////////////////////////////////////////////////


						case 124:	//ʮ��ɶ�����
						case 125:	//ʮ��ɶ��ֹ���
						case 126:	//ʮ��ɶ��ֹɱ���
						case 127:	//ʮ��ɶ��ֹ�����

							switch( reqTXFunction.iFuncID)
							{
							case 125:	//ʮ��ɶ��ֹ���
							case 126:	//ʮ��ɶ��ֹɱ���
								vdReq.data_type = dtype_double;
								break;
							case 124:	//ʮ��ɶ�����
							case 127:	//ʮ��ɶ��ֹ�����
								vdReq.data_type = dtype_val_string;
								break;
							}

							if(!(pBusiness->m_pSecurity->IsStockA() || pBusiness->m_pSecurity->IsStockB()))
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 18);	//Date��iHolderNo
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate < reqTXFunction.nDate)
							{
								break;
							}
							//--�������������µ�����ģʽ--wangzhy--20080603	
							//maybe bug			��һЩ���ݲ���ʮ���ɶ�,���ҿ��ܲ��йɶ����
							m_pHolder = NULL;
							pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_Holder,reqTXFunction.nDate,&m_pHolder,true);							
							if ( m_pHolder == NULL || reqTXFunction.iHolderNo > 10 )
							{
								//strResult = Con_strInvalid;
								break;
							}
							//ȡ��Ŀǰ����ȡ�ü�¼�Ĺɶ����,���ͨ���������ҵ������ʱ����ߵ����һ����¼,��������Ҫ�����û�Ҫ��ID�ڽ���ѡ��
							nAccountingID = m_pHolder->bNo;		//�û�Ҫ��ʮ��ɶ����
							if ( nAccountingID < reqTXFunction.iHolderNo )
							{
								//strResult = Con_strInvalid;
								break;
							}
							if( nAccountingID > reqTXFunction.iHolderNo )
								m_pHolder = m_pHolder + reqTXFunction.iHolderNo - nAccountingID;  
							switch( reqTXFunction.iFuncID)
							{
							case 124:	//ʮ��ɶ�����
								strResult = m_pHolder->bName;
								break;
							case 125:	//ʮ��ɶ��ֹ���
								strResult.Format(_T("%.0f"),m_pHolder->dVolume);
								break;
							case 126:	//ʮ��ɶ��ֹɱ���
								strResult.Format(_T("%.4f"),m_pHolder->dPercent);
								break;
							case 127:	//ʮ��ɶ��ֹ�����
								strResult = Tx::Data::TypeMapManage::GetInstance()->GetDatByID(TYPE_SHAREHOLDER_TYPE, m_pHolder->iFlag);
								break;
							default:
								break;
							}
							//---------------------------------------------
							break;
						case 128:	//ʮ����ͨ�ɶ�����
						case 129:	//ʮ����ͨ�ɶ��ֹ���
						case 130:	//ʮ����ͨ�ɶ��ܹɱ���
						case 131:	//ʮ����ͨ�ɶ���ͨ����
						case 132:	//ʮ����ͨ�ɶ��ֹ�����
							switch( reqTXFunction.iFuncID)
							{
							case 128:	//ʮ����ͨ�ɶ�����
							case 132:	//ʮ����ͨ�ɶ��ֹ�����
								vdReq.data_type = dtype_val_string;
								break;
							case 129:	//ʮ����ͨ�ɶ��ֹ���
								vdReq.data_type = dtype_int4;
								break;
							case 130:	//ʮ����ͨ�ɶ��ܹɱ���
							case 131:	//ʮ����ͨ�ɶ���ͨ����
								vdReq.data_type = dtype_double;
								break;
							}
							if(!(pBusiness->m_pSecurity->IsStockA() || pBusiness->m_pSecurity->IsStockB()))
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 18);	//Date��iHolderNo
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate < reqTXFunction.nDate)
							{
								break;
							}
							//--wangzhy--20080604--
							m_pTradeableHolder = NULL;
							pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_TradeableHolder,reqTXFunction.nDate,&m_pTradeableHolder,true);							
							if ( m_pTradeableHolder == NULL || reqTXFunction.iHolderNo > 10 )
							{
								//strResult = Con_strInvalid;
								break;
							}
							//ȡ��Ŀǰ����ȡ�ü�¼�Ĺɶ����,���ͨ���������ҵ������ʱ����ߵ����һ����¼,��������Ҫ�����û�Ҫ��ID�ڽ���ѡ��
							nAccountingID = m_pTradeableHolder->bNo;		//�û�Ҫ��ʮ��ɶ����
							if ( nAccountingID < reqTXFunction.iHolderNo )
							{
								//strResult = Con_strInvalid;
								break;
							}
							if( nAccountingID > reqTXFunction.iHolderNo )
								m_pTradeableHolder = m_pTradeableHolder + reqTXFunction.iHolderNo - nAccountingID;  
							//while( nAccountingID > reqTXFunction.iHolderNo )
							//{
							//	nAccountingID--;
							//	pTradeableHolder--;
							//}
							switch( reqTXFunction.iFuncID)
							{
							case 128:	//ʮ����ͨ�ɶ�����
								strResult = m_pTradeableHolder->bName;
								break;
							case 129:	//ʮ����ͨ�ɶ��ֹ���
								strResult.Format(_T("%.0f"),m_pTradeableHolder->dVolume);
								break;
							case 130:	//ʮ����ͨ�ɶ��ܹɱ���
								strResult.Format(_T("%.4f"),m_pTradeableHolder->dPercent);
								break;
							case 131:	//ʮ����ͨ�ɶ���ͨ����
								strResult.Format(_T("%.4f"),m_pTradeableHolder->dTradeablePercent);
								break;
							case 132:	//ʮ����ͨ�ɶ��ֹ�����
								strResult = Tx::Data::TypeMapManage::GetInstance()->GetDatByID(TYPE_TRADABLE_SHAREHOLDER, m_pTradeableHolder->iFlag);
								break;
							default:
								break;
							}
							//---------------------
							break;
							//--wangzhy--20080604--
						case 138:	//���ʽ����ʮ�����������
						case 139:	//���ʽ����ʮ������˻���ݶ�
						case 140:	//���ʽ����ʮ������˻���ݶ����
							switch(reqTXFunction.iFuncID)
							{
							case 138:	//���ʽ����ʮ�����������
								vdReq.data_type = dtype_val_string;
								break;
							case 139:	//���ʽ����ʮ������˻���ݶ�
							case 140:	//���ʽ����ʮ������˻���ݶ����
								vdReq.data_type = dtype_double;
								break;
							}
							if(!pBusiness->m_pSecurity->IsFund_Close())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 18);	//Date��iHolderNo
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate < reqTXFunction.nDate)
							{
								break;
							}
							m_pFundBondHolder = NULL;
							pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_FundHoder,reqTXFunction.nDate,&m_pFundBondHolder,true);							
							if ( m_pFundBondHolder == NULL || reqTXFunction.iHolderNo > 10 )
							{
								//strResult = Con_strInvalid;
								break;
							}
							//ȡ��Ŀǰ����ȡ�ü�¼�Ĺɶ����,���ͨ���������ҵ������ʱ����ߵ����һ����¼,��������Ҫ�����û�Ҫ��ID�ڽ���ѡ��
							nAccountingID = m_pFundBondHolder->iNo;		//�û�Ҫ��ʮ��ɶ����
							if ( nAccountingID < reqTXFunction.iHolderNo )
							{
								//strResult = Con_strInvalid;
								break;
							}
							if( nAccountingID > reqTXFunction.iHolderNo )
								m_pFundBondHolder = m_pFundBondHolder + reqTXFunction.iHolderNo - nAccountingID;  
							//while( nAccountingID > reqTXFunction.iHolderNo )
							//{
							//	nAccountingID--;
							//	pFundBondHolder--;
							//}
							switch(reqTXFunction.iFuncID)
							{
							case 138:	//���ʽ����ʮ�����������
								strResult = m_pFundBondHolder->bName;
								break;
							case 139:	//���ʽ����ʮ������˻���ݶ�
								strResult.Format(_T("%.0f"), m_pFundBondHolder->dVolume);
								break;
							case 140:	//���ʽ����ʮ������˻���ݶ����
								strResult.Format(_T("%.4f"), m_pFundBondHolder->dPercent);
								break;
							default:
								break;
							}
							break;
						case 141:	//��תծʮ�����������
						case 142:	//��תծʮ�������ծȯ���
						case 143:	//��תծʮ�������ծȯ������
							switch(reqTXFunction.iFuncID)
							{
							case 141:	//��תծʮ�����������
								vdReq.data_type = dtype_val_string;
								break;
							case 142:	//��תծʮ�������ծȯ���
							case 143:	//��תծʮ�������ծȯ������
								vdReq.data_type = dtype_double;
								break;
							}
							if(!pBusiness->m_pSecurity->IsBond_Change())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 18);	//Date��iHolderNo
							if(nDate > 0 && nDate < reqTXFunction.nDate)
							{
								break;
							}
							m_pFundBondHolder = NULL;
							pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_BondHoder,reqTXFunction.nDate,&m_pFundBondHolder,true);							
							if ( m_pFundBondHolder == NULL || reqTXFunction.iHolderNo > 10 )
							{
								//strResult = Con_strInvalid;
								break;
							}
							//ȡ��Ŀǰ����ȡ�ü�¼�Ĺɶ����,���ͨ���������ҵ������ʱ����ߵ����һ����¼,��������Ҫ�����û�Ҫ��ID�ڽ���ѡ��
							nAccountingID = m_pFundBondHolder->iNo;		//�û�Ҫ��ʮ��ɶ����
							if ( nAccountingID < reqTXFunction.iHolderNo )
							{
								//strResult = Con_strInvalid;
								break;
							}
							if( nAccountingID > reqTXFunction.iHolderNo )
								m_pFundBondHolder = m_pFundBondHolder + reqTXFunction.iHolderNo - nAccountingID;  
							//while( nAccountingID > reqTXFunction.iHolderNo )
							//{
							//	nAccountingID--;
							//	pFundBondHolder--;
							//}
							switch(reqTXFunction.iFuncID)
							{
							case 141:	//��תծʮ�����������
								strResult = m_pFundBondHolder->bName;
								break;
							case 142:	//��תծʮ�������ծȯ���
								strResult.Format(_T("%.0f"), m_pFundBondHolder->dVolume);
								break;
							case 143:	//��תծʮ�������ծȯ������
								if (m_pFundBondHolder->dPercent != Con_doubleInvalid  )				
									strResult.Format(_T("%.4f"), m_pFundBondHolder->dPercent);
								break;
							default:
								break;
							}
							break;
							//---------------------
						case 106:	//��תծת�ɼ۸�
							vdReq.data_type = dtype_double;
							if(!pBusiness->m_pSecurity->IsBond_Change())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate < reqTXFunction.nDate)
							{
								break;
							}
							dValue = GetDateIndicator_Value((INT)(reqTXFunction.nSecurityID), reqTXFunction.nDate, &vCBondPrice);
							if(dValue > 0)
							{
								strResult.Format(_T("%.2f"), dValue);
							}
							break;
						case 107:	//��תծ����
							vdReq.data_type = dtype_double;
							if(!pBusiness->m_pSecurity->IsBond_Change())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate < reqTXFunction.nDate)
							{
								break;
							}
							dValue = GetDateIndicator_Value((INT)(reqTXFunction.nSecurityID), reqTXFunction.nDate, &vCBondInterest);
							if(dValue > 0)
							{
								strResult.Format(_T("%.4f"), dValue / 100);
							}
							break;
						case 109:	//��תծת������
							vdReq.data_type = dtype_double;
							if(!pBusiness->m_pSecurity->IsBond_Change())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate < reqTXFunction.nDate)
							{
								break;
							}
							iRowNo = GetDateIndicator_Row((INT)(reqTXFunction.nSecurityID), reqTXFunction.nDate, &vCBondAmount);
							if(iRowNo >= 0)
							{
								dValue = 0;
								switch(reqTXFunction.iSubItemID)
								{
								case 1:		//��ת��ծȯ���
									tableIndDate[4].GetCell(2, (UINT)iRowNo, dValue);
									break;
								case 2:		//�ۼ�ת������
									tableIndDate[4].GetCell(3, (UINT)iRowNo, dValue);
									break;
								case 3:		//δת��ծȯ���
									tableIndDate[4].GetCell(4, (UINT)iRowNo, dValue);
									break;
								default:
									break;
								}
								if(dValue > 0)
								{
									strResult.Format(_T("%.2f"), dValue);
								}
							}
							if(dValue < 0.01 && reqTXFunction.iSubItemID == 3)
							{
								CString strDataPath = _T("");
								int id = m_pBusiness->m_pSecurity->GetSecurity1Id();
								strDataPath = m_pBusiness->DownloadXmlFile(id, 11004);
								strResult = GetXmlData( strDataPath,_T("T17"));
							}
							break;
						case 110:	//��תծ��������
							vdReq.data_type = dtype_double;
							if(!pBusiness->m_pSecurity->IsBond_Change())
							{
								break;
							}
							if(reqTXFunction.iSubItemID == 1)
							{	//���й�ģ
								pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
								if(pBondNewInfo != NULL)
								{
									dValue = pBondNewInfo->share;
									strResult.Format(_T("%.0f"), dValue);
								}
							}else
							{
								iRowNo = GetDateIndicator_Row((INT)(reqTXFunction.nSecurityID), 0, &vBondIPOInfo);
								if(iRowNo >= 0)
								{
									switch(reqTXFunction.iSubItemID)
									{
									case 2:		//���з���
										tableIndDate[5].GetCell(3, iRowNo, dValue);
										if(dValue > 0)
										{
											strResult.Format(_T("%.0f"), dValue);
										}
										break;
									case 3:		//���ʽ��
										tableIndDate[5].GetCell(4, iRowNo, dValue);
										if(dValue > 0)
										{
											strResult.Format(_T("%.0f"), dValue);
										}
										break;
									case 4:		//������ǩ��
										tableIndDate[5].GetCell(16, iRowNo, dValue);
										if(dValue > 0)
										{
											strResult.Format(_T("%.4f"), dValue / 100);
										}
										break;
									case 5:		//�������۱���
										tableIndDate[5].GetCell(18, iRowNo, dValue);
										if(dValue > 0)
										{
											strResult.Format(_T("%.4f"), dValue / 100);
										}
										break;
									case 6:		//�������۷�����
										tableIndDate[5].GetCell(15, iRowNo, dValue);
										if(dValue > 0)
										{
											strResult.Format(_T("%.0f"), dValue);
										}
										break;
									case 7:		//���»���������
										tableIndDate[5].GetCell(17, iRowNo, dValue);
										if(dValue > 0)
										{
											strResult.Format(_T("%.0f"), dValue);
										}
										break;
									case 8:		//�������̰�����
										tableIndDate[5].GetCell(19, iRowNo, dValue);
										if(dValue > 0)
										{
											strResult.Format(_T("%.0f"), dValue);
										}
										break;
									case 9:		//�ɶ����������
										tableIndDate[5].GetCell(14, iRowNo, dValue);
										if(dValue > 0)
										{
											strResult.Format(_T("%.0f"), dValue);
										}
										break;
									default:
										break;
									}
								}
							}
							break;
						case 113:	//��תծ-ת��ƽ��
							vdReq.data_type = dtype_double;
							if(!pBusiness->m_pSecurity->IsBond_Change())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate < reqTXFunction.nDate)
							{
								break;
							}
							pBondGeneral = pBusiness->m_pSecurity->GetBondGeneralByDate(reqTXFunction.nDate);
							if(pBondGeneral == NULL)
							{
								nDate = CTime::GetCurrentTime().GetYear() * 10000 + CTime::GetCurrentTime().GetMonth() * 100 + CTime::GetCurrentTime().GetDay();
								if(reqTXFunction.nDate == nDate && pBusiness->m_pSecurity->IsValid())
								{
									dValue = bond.CalcParity((INT)(reqTXFunction.nSecurityID), reqTXFunction.nDate);
									if(dValue > 0)
									{
										strResult.Format(_T("%.2f"), dValue);
									}
								}
							}else
							{
								dValue = pBondGeneral->dParity;
								if(dValue > 0)
								{
									strResult.Format(_T("%.2f"), dValue);
								}
							}
							break;
						case 114:	//��תծ-ת�������
							vdReq.data_type = dtype_double;
							if(!pBusiness->m_pSecurity->IsBond_Change())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate < reqTXFunction.nDate)
							{
								break;
							}
							pBondGeneral = pBusiness->m_pSecurity->GetBondGeneralByDate(reqTXFunction.nDate);
							if(pBondGeneral == NULL)
							{
								nDate = CTime::GetCurrentTime().GetYear() * 10000 + CTime::GetCurrentTime().GetMonth() * 100 + CTime::GetCurrentTime().GetDay();
								if(reqTXFunction.nDate == nDate && pBusiness->m_pSecurity->IsValid())
								{
									dValue = bond.CalcPremium((INT)(reqTXFunction.nSecurityID), reqTXFunction.nDate);
									if(dValue != Tx::Core::Con_doubleInvalid)
									{
										strResult.Format(_T("%.4f"), dValue);
									}
								}
							}else
							{
								dValue1 = pBondGeneral->dParity;
								dValue2 = GetKLine(pBusiness, reqTXFunction.nDate, 4,reqTXFunction.nSecurityID);
								if(dValue1 > 0 && dValue2 > 0)
								{
									dValue = dValue2 / dValue1 - 1;
									strResult.Format(_T("%.4f"), dValue);
								}
							}
							break;
						case 115:	//��תծ-ת���׼�
							vdReq.data_type = dtype_double;
							if(!pBusiness->m_pSecurity->IsBond_Change())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate < reqTXFunction.nDate)
							{
								break;
							}
							pBondGeneral = pBusiness->m_pSecurity->GetBondGeneralByDate(reqTXFunction.nDate);
							if(pBondGeneral == NULL)
							{
								nDate = CTime::GetCurrentTime().GetYear() * 10000 + CTime::GetCurrentTime().GetMonth() * 100 + CTime::GetCurrentTime().GetDay();
								if(reqTXFunction.nDate == nDate && pBusiness->m_pSecurity->IsValid())
								{
									dValue = bond.GetFloor((INT)(reqTXFunction.nSecurityID));
									if(dValue != Tx::Core::Con_doubleInvalid)
									{
										strResult.Format(_T("%.2f"), dValue);
									}
								}
							}else
							{
								dValue = pBondGeneral->dFloor;
								if(dValue > 0)
								{
									strResult.Format(_T("%.2f"), dValue);
								}
							}
							break;
						case 116:	//��תծ-�׼������
							vdReq.data_type = dtype_double;
							if(!pBusiness->m_pSecurity->IsBond_Change())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate < reqTXFunction.nDate)
							{
								break;
							}
							//��תծ-ת���׼�
							pBondGeneral = pBusiness->m_pSecurity->GetBondGeneralByDate(reqTXFunction.nDate);
							if(pBondGeneral == NULL)
							{
								nDate = CTime::GetCurrentTime().GetYear() * 10000 + CTime::GetCurrentTime().GetMonth() * 100 + CTime::GetCurrentTime().GetDay();
								if(reqTXFunction.nDate == nDate && pBusiness->m_pSecurity->IsValid())
								{
									dValue1 = bond.GetFloor((INT)(reqTXFunction.nSecurityID));
									dValue2 = GetKLine(pBusiness, reqTXFunction.nDate, 4,reqTXFunction.nSecurityID);
									if(dValue1 > 0 && dValue2 > 0)
									{
										dValue = dValue2 / dValue1 - 1;
										strResult.Format(_T("%.4f"), dValue);
									}
								}
							}else
							{
								dValue1 = pBondGeneral->dFloor;
								dValue2 = GetKLine(pBusiness, reqTXFunction.nDate, 4,reqTXFunction.nSecurityID);
								if(dValue1 > 0 && dValue2 > 0)
								{
									dValue = dValue2 / dValue1 - 1;
									strResult.Format(_T("%.4f"), dValue);
								}
							}
							break;
							//---zhangxs---20080822---------------------------------------------------
						case 154:	//������	
							{
								strResult = _T("");
								FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
								if ( !pBusiness->m_pSecurity->IsFund())
									break;
								if(m_tableFundManager.GetRowCount() == 0)
									pBusiness->m_pLogicalBusiness->GetData(m_tableFundManager, true);
								vector <UINT> vRow;
								int iIndex = pBusiness->m_pSecurity->GetSecurity1Id();
								m_tableFundManager.Find(0, iIndex, vRow);
								if(vRow.size() ==0 )
									break;
								int nBegin = 0;
								int nEnd = 0;
								iSubItemID = (int)reqTXFunction.iSubItemID-1;
								CString strName = _T("");
								CString	str = _T("");
								int nValue = 0;
								int i = 0;
								for( vector<UINT>::iterator iter = vRow.begin(); iter != vRow.end(); ++iter,i++ )
								{
									m_tableFundManager.GetCell( 4,*iter,nBegin);
									m_tableFundManager.GetCell( 5,*iter,nEnd );
									if( reqTXFunction.nDate >= nBegin && (reqTXFunction.nDate <= nEnd || nEnd == Con_intInvalid))
									{
										m_tableFundManager.GetCell( 2,*iter,strName);
										switch( iSubItemID )
										{
										case 0:		//����������
											str = strName;
											break;
										case 1:		//������ְλ
											m_tableFundManager.GetCell( 3,*iter,str);
											break;
										case 2:		//��ʼ����
											m_tableFundManager.GetCell( 4,*iter,nValue);
											str.Format(_T("%d"),nValue);
											break;
										case 3:		//��������
											m_tableFundManager.GetCell( 5,*iter,nValue);
											if(nValue ==Con_intInvalid )
												str = _T("����");
											else
												str.Format(_T("%d"),nValue);
											break;
										case 4:		//�Ա�
											m_tableFundManager.GetCell( 6,*iter,str);
											nValue = _ttoi(str.GetBuffer(0));
											if(nValue == 1)
												str = _T("��");
											else
												str = _T("Ů");
											break;
										case 5:		//ѧ��
											m_tableFundManager.GetCell( 7,*iter,str);
											break;
										case 6:		//����
											m_tableFundManager.GetCell( 8,*iter,nValue);
											if(nValue ==Con_intInvalid )
												str = _T("δ�ṩ");
											else 
												str.Format(_T("%d"),nValue);
											break;
										case 7:		//����
											m_tableFundManager.GetCell( 9,*iter,str);
											break;
										default:
											break;

										}
										//strResult += strName + _T(":") + str;
										if(i == vRow.size() -1)
											strResult += str;
										else
											strResult += (str + _T("��"));
									}
								}
							}
							break;
						case 155:	//���ʽ��������Ϣ
							strResult = Con_strInvalid;
							FindItemOfReq(&reqTXFunction, strReq,24);
							if( pBusiness->m_pSecurity->IsFund_Close())
							{
								FundCloseIssueInfo *pIssueInfo = pBusiness->m_pSecurity->GetFundCloseIssueInfo();
								if( pIssueInfo == NULL )
									break;
								iSubItemID = (int)reqTXFunction.iSubItemID-1;
								switch(iSubItemID)
								{
								case 0:		//��ʼ����
									strResult.Format( _T("%d"),pIssueInfo->start_date);
									break;
								case 1:		//��������
									strResult.Format( _T("%d"),pIssueInfo->end_date);
									break;
								case 2:		//���з�ʽ
									if( (CString)pIssueInfo->issue_mode != Con_strInvalid)
										strResult.Format( _T("%s"), (CString)pIssueInfo->issue_mode);
									break;
								case 3:		//��������
									if((CString)pIssueInfo->issue_type != Con_strInvalid)
										strResult.Format( _T("%s"),(CString)pIssueInfo->issue_type);
									break;
								case 4:		//������
									if(pIssueInfo->dValue[0] != Con_doubleInvalid)
										strResult.Format( _T("%d"),pIssueInfo->dValue[0]);
									break;
								case 5:		//��ֵ
									if(pIssueInfo->dValue[1] != Con_doubleInvalid)
										strResult.Format( _T("%.2f"),pIssueInfo->dValue[1]);
									break;
								case 6:		//���м�
									if(pIssueInfo->dValue[2] != Con_doubleInvalid)
										strResult.Format( _T("%.2f"),pIssueInfo->dValue[2]);
									break;
								case 7:		//���з���
									if(pIssueInfo->dValue[3] != Con_doubleInvalid)
										strResult.Format( _T("%.2f"),pIssueInfo->dValue[3]);
									break;
								case 8:		//��������ֵ
									if(pIssueInfo->dValue[4] != Con_doubleInvalid)
										strResult.Format( _T("%.2f"),pIssueInfo->dValue[4]);
									break;
								case 9:		//���ʽ��
									if(pIssueInfo->dValue[5] != Con_doubleInvalid)
										strResult.Format( _T("%.2f"),pIssueInfo->dValue[5]);
									break;
								case 10:	//��������
									strResult.Format( _T("%d"),pIssueInfo->list_date);
									break;
								case 11:	//���ο���ͨ�ݶ�
									if(pIssueInfo->dValue[6] != Con_doubleInvalid)
										strResult.Format( _T("%.2f"),pIssueInfo->dValue[6]);
									break;
								case 12:	//��Ч�깺����
									strResult.Format( _T("%d"),pIssueInfo->dValue[7]);
									break;
								case 13:	//��Ч�깺����
									if(pIssueInfo->dValue[8] != Con_doubleInvalid)
										strResult.Format( _T("%.2f"),pIssueInfo->dValue[8]);
									break;
								case 14:	//�����Ϲ�����
									if(pIssueInfo->dValue[9] != Con_doubleInvalid)
										strResult.Format( _T("%.2f"),pIssueInfo->dValue[9]);
									break;
								case 15:	//��ǩ��
									if(pIssueInfo->dValue[10] != Con_doubleInvalid)
										strResult.Format( _T("%.2f"),pIssueInfo->dValue[10]);
									break;
								case 16:		//�깺����
									strResult.Format( _T("%d"),pIssueInfo->applyid);
									break;
								case 17:	//����������
									if(pIssueInfo->dValue[11] != Con_doubleInvalid)
										strResult.Format( _T("%.2f"),pIssueInfo->dValue[11]);
									break;
								case 18:	//���ۻ���������
									strResult.Format( _T("%d"),pIssueInfo->list_date_add);
									break;
								case 19:	//��ļ����¶����
									strResult.Format( _T("%s"),pIssueInfo->enlist_info_on_newspaper);
									break;
								case 20:	//��������¶����
									strResult.Format( _T("%.2f"),pIssueInfo->list_info_on_newspaper);
									break;
								case 21:	//��������������
									if(pIssueInfo->dValue[12] != Con_doubleInvalid)
										strResult.Format( _T("%.2f"),pIssueInfo->dValue[12]);
									break;
								case 22:	//����������

									strResult.Format( _T("%d"),pIssueInfo->issue_date);
									break;
								case 23:	//������������
									if(pIssueInfo->dValue[13] != Con_doubleInvalid)
										strResult.Format( _T("%.2f"),pIssueInfo->dValue[13]);
									break;
								default:
									break;
								}
							}
							break;
						case 156:	//����ʽ��������Ϣ
							strResult = Con_strInvalid;
							FindItemOfReq(&reqTXFunction, strReq,24);
							if( pBusiness->m_pSecurity->IsFund_Open())
							{
								FundOpenIssueInfo *pIssueInfo = pBusiness->m_pSecurity->GetFundOpenIssueInfo();
								if( pIssueInfo == NULL )
									break;
								iSubItemID = (int)reqTXFunction.iSubItemID-1;
								switch(iSubItemID)
								{
								case 0:		//��ʼ����
									strResult.Format( _T("%d"),pIssueInfo->start_date);
									break;
								case 1:		//��������
									strResult.Format( _T("%d"),pIssueInfo->end_date);
									break;
								case 2:		//�������۶�
									if(pIssueInfo->dValue[0] != Con_doubleInvalid)
										strResult.Format( _T("%.2f"),pIssueInfo->dValue[0]);
									break;
								case 3:		//������Ϣ
									if(pIssueInfo->dValue[1] != Con_doubleInvalid)
										strResult.Format( _T("%.2f"),pIssueInfo->dValue[1]);
									break;
								case 4:		//�ϼ�ļ���ݶ�
									if(pIssueInfo->dValue[2] != Con_doubleInvalid)
										strResult.Format( _T("%.2f"),pIssueInfo->dValue[2]);
									break;
								case 5:		//��Ч�Ϲ�����
									if(pIssueInfo->dValue[3] != Con_doubleInvalid)
										strResult.Format( _T("%.2f"),pIssueInfo->dValue[3]);
									break;
								case 6:		//�깺��ؿ�ʼ��
									strResult.Format( _T("%d"),pIssueInfo->redeem_date);
									break;
								default:
									break;
								}
							}
							break;
						case 157:   //�����ʲ����
							{
								FindItemOfReq(&reqTXFunction, strReq, 24);	//FuncID��SubItemID��IndicatorID��������ȡ��Ƽ�����
								nInstitution = (INT)pBusiness->m_pSecurity->GetInstitutionId();
								m_pFundInvesmentGroup = NULL;
								nAccountingID = nInstitution *100 +(INT64)reqTXFunction.iSubjectID;
								strResult = Con_strInvalid;
								bLoaded = true;
								iSubItemID = (int)reqTXFunction.iSubItemID-1;
								nAccountingID = (INT)pBusiness->m_pSecurity->GetSecurity1Id();
								if(m_pFundInvesmentGroupDataFile!=NULL)
								{
									int iExFileId=Tx::Data::DataStatus::GetInstance()->GetExFileIdFromIni(Tx::Core::SystemPath::GetInstance()->GetSystemDataPath(),_T("fund_invesment_group"));
									bLoaded = m_pFundInvesmentGroupDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,iExFileId,true);
								}
								if ( bLoaded )
									m_pFundInvesmentGroup = m_pFundInvesmentGroupDataFile->GetDataByObj(nAccountingID,false);
								if ( m_pFundInvesmentGroup == NULL )
									break;
								if ( m_pFundInvesmentGroup!=NULL )
									dValue = m_pFundInvesmentGroup->dValue[iSubItemID];
								if( dValue !=Con_doubleInvalid )
									strResult.Format( _T("%.2f"),dValue);

							}
							break;
						case 158:	//����ݶ�䶯
						case 159:	//����ݶ�
							break;
						case 160:	//�����������ҵ�ֲ�
						case 161:	//����ָ������ҵ�ֲ�
						case 162:	//����ϲ�����ҵ�ֲ�
						case 189:	//����������ҵ�ֲ�
							FindItemOfReq(&reqTXFunction, strReq, 24);	//FuncID��SubItemID��IndicatorID��������ȡ��Ƽ�����
							nInstitution = (INT)pBusiness->m_pSecurity->GetInstitutionId();
							iSubItemID = (int)reqTXFunction.iSubItemID-1;
							switch( reqTXFunction.iFuncID )
							{
							case 160:	//�����������ҵ�ֲ�
								{
									m_pFundActiveIndustryDistribute = NULL;
									strResult = Con_strInvalid;
									nAccountingID = nInstitution *100 +(INT64)reqTXFunction.iSubjectID;
									bLoaded = true;
									if(m_pFundActiveIndustryDistributeDataFile!=NULL)
										bLoaded = m_pFundActiveIndustryDistributeDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30157,true);
									if ( bLoaded )
										m_pFundActiveIndustryDistribute = m_pFundActiveIndustryDistributeDataFile->GetDataByObj(nAccountingID,false);
									if ( m_pFundActiveIndustryDistribute == NULL )
										break;
									if ( m_pFundActiveIndustryDistribute!=NULL )
										dValue = m_pFundActiveIndustryDistribute->dFinancial[iSubItemID];
									if( dValue !=Con_doubleInvalid )
										strResult.Format( _T("%.2f"),dValue);
								}
								break;
							case 161:	//����ָ������ҵ�ֲ�
								{
									m_pFundIndexIndustryDistribute = NULL;
									strResult = Con_strInvalid;
									nAccountingID = nInstitution *100 +(INT64)reqTXFunction.iSubjectID;
									bLoaded = true;
									if(m_pFundActiveIndustryDistributeDataFile!=NULL)
										bLoaded = m_pFundIndexIndustryDistributeDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30159,true);
									if ( bLoaded )
										m_pFundIndexIndustryDistribute = m_pFundIndexIndustryDistributeDataFile->GetDataByObj(nAccountingID,false);
									if ( m_pFundIndexIndustryDistribute == NULL )
										break;
									if ( m_pFundIndexIndustryDistribute!=NULL )
										dValue = m_pFundIndexIndustryDistribute->dFinancial[iSubItemID];
									if( dValue !=Con_doubleInvalid )
										strResult.Format( _T("%.2f"),dValue);
								}
								break;
							case 162:	//����ϲ�����ҵ�ֲ�
								{
									m_pFundComIndustryDistribute = NULL;
									strResult = Con_strInvalid;
									nAccountingID = nInstitution *100 +(INT64)reqTXFunction.iSubjectID;
									bLoaded = true;
									if(m_pFundComIndustryDistribute!=NULL)
										bLoaded = m_pFundComIndustryDistributeDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30155,true);
									if ( bLoaded )
										m_pFundComIndustryDistribute = m_pFundComIndustryDistributeDataFile->GetDataByObj(nAccountingID,false);
									if ( m_pFundComIndustryDistribute == NULL )
										break;
									if ( m_pFundComIndustryDistribute!=NULL )
										dValue = m_pFundComIndustryDistribute->dFinancial[iSubItemID];
									if( dValue !=Con_doubleInvalid )
										strResult.Format( _T("%.2f"),dValue);
								}
								break;
							case 189:	//����������ҵ�ֲ�
								{
									m_pFundTxIndustryDistribute = NULL;
									strResult = Con_strInvalid;
									nAccountingID = nInstitution *100 +(INT64)reqTXFunction.iSubjectID;
									bLoaded = true;
									if(m_pFundTxIndustryDistribute!=NULL)
										bLoaded = m_pFundTxIndustryDistributeDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30161,true);
									if ( bLoaded )
										m_pFundTxIndustryDistribute = m_pFundTxIndustryDistributeDataFile->GetDataByObj(nAccountingID,false);
									if ( m_pFundTxIndustryDistribute == NULL )
										break;
									if ( m_pFundTxIndustryDistribute!=NULL )
										dValue = m_pFundTxIndustryDistribute->dFinancial[iSubItemID];
									if( dValue !=Con_doubleInvalid )
										strResult.Format( _T("%.2f"),dValue);
								}
								break;
							}
							break;
						case 166:	//�ֲֽṹ
							//FindItemOfReq(&reqTXFunction, strReq, 24);	//FuncID��SubItemID��IndicatorID��������ȡ��Ƽ�����
							//nInstitution = (INT)pBusiness->m_pSecurity->GetInstitutionId();
							//iSubItemID = (int)reqTXFunction.iSubItemID-1;
							break;
						case 167:	//����ֵ�䶯
						case 168:	//������Ҫ����ָ��
						case 169:	//�����ʲ���ծ��
						case 170:	//����Ӫҵ����
						case 171:	//�������
							FindItemOfReq(&reqTXFunction, strReq, 24);	//FuncID��SubItemID��IndicatorID��������ȡ��Ƽ�����
							nInstitution = (INT)pBusiness->m_pSecurity->GetInstitutionId();
							iSubItemID = (int)reqTXFunction.iSubItemID-1;
							switch( reqTXFunction.iFuncID )
							{
							case 167:	//����ֵ�䶯
								{
									m_pFundNavChange = NULL;
									strResult = Con_strInvalid;
									nAccountingID = nInstitution * 100 + (INT64)reqTXFunction.iSubjectID;
									//�����������ļ�	
									bLoaded = true;
									if(m_pFundNavChangeDataFile!=NULL)
										bLoaded = m_pFundNavChangeDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30151,true);
									if ( bLoaded )
										m_pFundNavChange = m_pFundNavChangeDataFile->GetDataByObj(nAccountingID,false);
									if ( m_pFundNavChange == NULL )
										break;
									if ( m_pFundNavChange!=NULL )
										dValue = m_pFundNavChange->dFinancial[iSubItemID];
									if( dValue !=Con_doubleInvalid )
										strResult.Format( _T("%.2f"),dValue);
								}
								break;
							case 168:	//������Ҫ����ָ��
								{
									m_pFundFinancial = NULL;
									strResult = Con_strInvalid;
									nAccountingID = nInstitution * 100 + (INT64)reqTXFunction.iSubjectID;
									//�����������ļ�	
									bLoaded = true;
									if(m_pFundFinancialDataFile!=NULL)
										bLoaded = m_pFundFinancialDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30145,true);
									if ( bLoaded )
										m_pFundFinancial = m_pFundFinancialDataFile->GetDataByObj(nAccountingID,false);
									if ( m_pFundFinancial == NULL )
										break;
									if ( m_pFundFinancial!=NULL )
										dValue = m_pFundFinancial->dFinancial[iSubItemID];
									if( dValue !=Con_doubleInvalid )
										strResult.Format( _T("%.2f"),dValue);
								}
								break;
							case 169:	//�����ʲ���ծ��
								{
									m_pFundBalance = NULL;
									strResult = Con_strInvalid;
									nAccountingID = nInstitution * 100 + (INT64)reqTXFunction.iSubjectID;
									//�����������ļ�	
									bLoaded = true;
									if(m_pFundBalanceDataFile!=NULL)
										bLoaded = m_pFundBalanceDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30147,true);
									if ( bLoaded )
										m_pFundBalance = m_pFundBalanceDataFile->GetDataByObj(nAccountingID,false);
									if ( m_pFundBalance == NULL )
										break;
									if ( m_pFundBalance!=NULL )
										dValue = m_pFundBalance->dFinancial[iSubItemID];
									if( dValue !=Con_doubleInvalid )
										strResult.Format( _T("%.2f"),dValue);
								}
								break;
							case 170:	//����Ӫҵ����
								{
									m_pFundArchievement = NULL;
									strResult = Con_strInvalid;
									nAccountingID = nInstitution * 100 + (INT64)reqTXFunction.iSubjectID;
									//�����������ļ�	
									bLoaded = true;
									if(m_pFundArchievementDataFile!=NULL)
										bLoaded = m_pFundArchievementDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30149,true);
									if ( bLoaded )
										m_pFundArchievement = m_pFundArchievementDataFile->GetDataByObj(nAccountingID,false);
									if ( m_pFundArchievement == NULL )
										break;
									if ( m_pFundArchievement!=NULL )
										dValue = m_pFundArchievement->dFinancial[iSubItemID];
									if( dValue !=Con_doubleInvalid )
										strResult.Format( _T("%.2f"),dValue);
								}
								break;
							case 171:	//�������
								{
									m_pFundRevenue = NULL;
									strResult = Con_strInvalid;
									nAccountingID = nInstitution * 100 + (INT64)reqTXFunction.iSubjectID;
									//�����������ļ�	
									bLoaded = true;
									if(m_pFundRevenueDataFile!=NULL)
										bLoaded = m_pFundRevenueDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30079,true);
									if ( bLoaded )
										m_pFundRevenue = m_pFundRevenueDataFile->GetDataByObj(nAccountingID,false);
									if ( m_pFundRevenue == NULL )
										break;
									if ( m_pFundRevenue!=NULL )
										dValue = m_pFundRevenue->dFinancial[iSubItemID];
									if( dValue !=Con_doubleInvalid )
										strResult.Format( _T("%.2f"),dValue);
								}
								break;
							default :
								break;
							}
							break;
						case 163:	//�������زֹɽ���ʵ��
						case 164:	//�زֹ���ֵ
						case 165:	//�زֹ���ռ����
						case 172:	//�زֹɳֹ���
						case 173:	//�زֹ�ռ��ͨ�ɱ�
							switch(reqTXFunction.iFuncID)
							{
							case 163:	//�������زֹɽ���ʵ��
								vdReq.data_type = dtype_val_string;
								break;
							case 164:	//�������زֹ���ֵ
							case 165:	//�زֹ���ռ����
							case 172:	//�زֹɳֹ���
							case 173:	//�زֹ�ռ��ͨ�ɱ�
								vdReq.data_type = dtype_double;
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 18);	//Date��iHolderNo
							FindItemOfReq(&reqTXFunction, strReq, 24);
							m_pFundstockVipStock = NULL;

							//����ʵ��
							nAccountingID = (INT64)reqTXFunction.nFYear * 10000 +  (INT64)reqTXFunction.nFQuarter;
							m_pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_FundStockInvestmentVIPStock,nAccountingID,&m_pFundstockVipStock,false);
											
							if ( m_pFundstockVipStock == NULL || reqTXFunction.iHolderNo > 10 )
							{
								break;
							}
							nAccountingID = m_pFundstockVipStock->iMmarketValueOrder;		//ʮ���زֹ����
							if ( nAccountingID < reqTXFunction.iHolderNo )
							{
								break;
							}
							if( nAccountingID > reqTXFunction.iHolderNo )
								m_pFundstockVipStock = m_pFundstockVipStock + reqTXFunction.iHolderNo - nAccountingID;  

							switch(reqTXFunction.iFuncID)
							{
							case 163:	//�����ͻ���ʮ���زֹ�����
								{
									int iSecurity = m_pFundstockVipStock ->f2;
									Tx::Business::TxBusiness business;
									business.GetSecurityNow(iSecurity);
									strResult = business.m_pSecurity->GetName();
								}
								break;
							case 164:	//�����ͻ����زֹ���ֵ
								{
									if( m_pFundstockVipStock ->dFinancial[0] != Con_doubleInvalid )
										strResult.Format(_T("%.2f"), m_pFundstockVipStock ->dFinancial[0]);
								}
								break;
							case 165:	//�����ͻ����ز���ռ����			
								strResult.Format(_T("%.2f"), m_pFundstockVipStock ->dFinancial[1]);
								break;
							case 172:	//�����ͻ����زֳֹ���			
								strResult.Format(_T("%.2f"), m_pFundstockVipStock ->dFinancial[2]);
								break;
							case 173:	//�����ͻ����ز�ռ��ͨ�ɱ�			
								strResult.Format(_T("%.2f"), m_pFundstockVipStock ->dFinancial[3]);
								break;
							default:
								break;
							}
							break;
						case 174:
						case 175:
						case 176:
						case 177:
						case 178:
							switch(reqTXFunction.iFuncID)
							{
							case 174:	//�������زֹɽ���ʵ��
								vdReq.data_type = dtype_val_string;
								break;
							case 175:	//ָ�����زֹ���ֵ
							case 176:	//�زֹ���ռ����
							case 177:	//�زֹɳֹ���
							case 178:	//�زֹ�ռ��ͨ�ɱ�
								vdReq.data_type = dtype_double;
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 18);	//Date��iHolderNo
							FindItemOfReq(&reqTXFunction, strReq, 24);
							m_pFundIndexVIPStock = NULL;

							//����ʵ��
							nAccountingID = (INT64)reqTXFunction.nFYear * 10000 +  (INT64)reqTXFunction.nFQuarter;
							m_pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_FundIndexInvestmentVIPStock,nAccountingID,&m_pFundIndexVIPStock,false);
											
							if ( m_pFundIndexVIPStock == NULL || reqTXFunction.iHolderNo > 10 )
							{
								break;
							}
							nAccountingID = m_pFundIndexVIPStock->iMmarketValueOrder;		//ʮ���زֹ����
							if ( nAccountingID < reqTXFunction.iHolderNo )
							{
								break;
							}
							if( nAccountingID > reqTXFunction.iHolderNo )
								m_pFundIndexVIPStock = m_pFundIndexVIPStock + reqTXFunction.iHolderNo - nAccountingID;  

							switch(reqTXFunction.iFuncID)
							{
							case 174:	//ָ���ͻ���ʮ���زֹ�����
								{
									int iSecurity = m_pFundIndexVIPStock ->f2;
									Tx::Business::TxBusiness business;
									business.GetSecurityNow(iSecurity);
									strResult = business.m_pSecurity->GetName();
								}
								break;
							case 175:	//ָ���ͻ����زֹ���ֵ
								{
									if( m_pFundstockVipStock ->dFinancial[0] != Con_doubleInvalid )
										strResult.Format(_T("%.2f"), m_pFundIndexVIPStock ->dFinancial[0]);
								}
								break;
							case 176:	//ָ���ͻ����ز���ռ����			
								strResult.Format(_T("%.2f"), m_pFundIndexVIPStock ->dFinancial[1]);
								break;
							case 177:	//ָ���ͻ����زֳֹ���			
								strResult.Format(_T("%.2f"), m_pFundIndexVIPStock ->dFinancial[2]);
								break;
							case 178:	//ָ���ͻ����ز�ռ��ͨ�ɱ�			
								strResult.Format(_T("%.2f"), m_pFundIndexVIPStock ->dFinancial[3]);
								break;
							default:
								break;
							}
							break;
						case 179:
						case 180:
						case 181:
						case 182:
						case 183:
							switch(reqTXFunction.iFuncID)
							{
							case 179:	//�ϲ����زֹɽ���ʵ��
								vdReq.data_type = dtype_val_string;
								break;
							case 180:	//�ϲ����زֹ���ֵ
							case 181:	//�زֹ���ռ����
							case 182:	//�زֹɳֹ���
							case 183:	//�زֹ�ռ��ͨ�ɱ�
								vdReq.data_type = dtype_double;
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 18);	//Date��iHolderNo
							FindItemOfReq(&reqTXFunction, strReq, 24);
							m_pFundCombinVIPStock = NULL;

							//����ʵ��
							nAccountingID = (INT64)reqTXFunction.nFYear * 10000 +  (INT64)reqTXFunction.nFQuarter;
							m_pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_FundIndexInvestmentVIPStock,nAccountingID,&m_pFundCombinVIPStock,false);
											
							if ( m_pFundCombinVIPStock == NULL || reqTXFunction.iHolderNo > 10 )
							{
								break;
							}
							nAccountingID = m_pFundCombinVIPStock->iMmarketValueOrder;		//ʮ���زֹ����
							if ( nAccountingID < reqTXFunction.iHolderNo )
							{
								break;
							}
							if( nAccountingID > reqTXFunction.iHolderNo )
								m_pFundCombinVIPStock = m_pFundCombinVIPStock + reqTXFunction.iHolderNo - nAccountingID;  

							switch(reqTXFunction.iFuncID)
							{
							case 179:	//�ϲ��ͻ���ʮ���زֹ�����
								{
									int iSecurity = m_pFundCombinVIPStock ->f2;
									Tx::Business::TxBusiness business;
									business.GetSecurityNow(iSecurity);
									strResult = business.m_pSecurity->GetName();
								}
								break;
							case 180:	//�ϲ��ͻ����زֹ���ֵ
								{
									if( m_pFundCombinVIPStock ->dFinancial[0] != Con_doubleInvalid )
										strResult.Format(_T("%.2f"), m_pFundCombinVIPStock ->dFinancial[0]);
								}
								break;
							case 181:	//�ϲ��ͻ����ز���ռ����			
								strResult.Format(_T("%.2f"), m_pFundCombinVIPStock ->dFinancial[1]);
								break;
							case 182:	//�ϲ��ͻ����زֳֹ���			
								strResult.Format(_T("%.2f"), m_pFundCombinVIPStock ->dFinancial[2]);
								break;
							case 183:	//�ϲ��ͻ����ز�ռ��ͨ�ɱ�			
								strResult.Format(_T("%.2f"), m_pFundCombinVIPStock ->dFinancial[3]);
								break;
							default:
								break;
							}
							break;
						case 212:	//���շ���--��׼��
						case 213:	//���շ���--Alpha
						case 214:	//���շ���--Beta
						case 215:	//���շ���--Sharpeϵ��
						case 216:	//���շ���--Treynorϵ��
						case 217:	//���շ���--�������
						case 287:   //�����ֵ
							{
								strResult = Con_strInvalid;
								FindItemOfReq( &reqTXFunction, strReq, 26 );
								Tx::Core::Table_Indicator table;
								std::set<int> iset;
								iset.clear();
								//ֻ��дһ������
								int id;
								id = reqTXFunction.nSecurityID;
								iset.insert( id );
								//************************
								int iEndDate = 20070730;						//��ֹ����
								long lSecurityId = 0;					//��׼����ʵ��ID
								int iTradeDaysCount=20;				//��������
								int iStartDate=0;
								bool bLogRatioFlag=false;			//������ʵķ�ʽ,true=ָ������,false=�򵥱���
								bool bClosePrice=true;				//ʹ�����̼�
								int  iStdType=0;					//��׼������1=��ƫ��������ƫ
								int  iOffset=0;						//ƫ��
								bool bAhead=true;					//��Ȩ��־,true=ǰ��Ȩ,false=��Ȩ
								bool bUserRate=false;				//�û��Զ���������,����ȡһ�궨�ڻ�׼����
								bool bDayRate=true;					//������
								int iDuration = 1;					//��������0=�գ�1=�ܣ�2=�£�3=����5=��
								double dUserRate = 0.034;
								//���������ļ���߶�Ӧ���Ǵ�1��ʼ��,���õ�ʱ���1
								int iDateType = 0;					//0��ֹ����Ϊ���½����գ�1ָ����ֹ���ڣ�2ָ����ֹ����
								long lRefoSecurityId = 4000208;
								iEndDate  = reqTXFunction.nEndDate;
								iStartDate = reqTXFunction.nStartDate;
								iDuration = reqTXFunction.iSYPL;

								Tx::Business::TxBusiness	Business;
								Business.GetSecurityNow( lRefoSecurityId );
								Business.m_pSecurity->LoadTradeDate();
								Security* psec = (Security*)GetSecurity(id);
								if(psec == NULL)
									return NULL;
								double std,alpha,beta,sharp,treynor,trackerror;
								std=alpha=beta=sharp=treynor=trackerror=0.0;
								double dMean = 0.0;
								if(psec->IsFund())
								{
									Tx::Business::TxFund txfund;
									txfund.BlockRiskIndicatorAdvFund(
										10228,								//����ID		
										table,								//������ݱ�
										iset,								//����ʵ��ID
										iEndDate,							//��ֹ����
										lRefoSecurityId,				//��׼����ʵ��ID
										iStartDate,						//��ʼ����
										iTradeDaysCount,				//��������
										iDuration,						//��������0=�գ�1=�ܣ�2=�£�3=����4=��
										bLogRatioFlag,					//������ʵķ�ʽ,true=ָ������,false=�򵥱���
										bClosePrice,					//ʹ�����̼�
										0,							//��׼������1=��ƫ��������ƫ
										0,							//ƫ��
										true,						//��Ȩ��־,true=ǰ��Ȩ,false=��Ȩ
										1,							//�û��Զ���������,����ȡһ�궨�ڻ�׼����
										dUserRate,					//�û��Զ���������
										iDuration==0?true:false,	//������
										true,
										true
						                );					
									if (table.GetRowCount() == 1)
									{
										int iCol = 2;
										table.GetCell(iCol,0,std);
										iCol++;
										table.GetCell(iCol,0,alpha);
										iCol++;
										table.GetCell(iCol,0,beta);
										iCol++;
										table.GetCell(iCol,0,sharp);
										iCol++;
										table.GetCell(iCol,0,treynor);
										iCol++;
										table.GetCell(iCol,0,trackerror);
										iCol++;
										table.GetCell(iCol,0,dMean);
										iCol++;
									}
								}
								else
								{
									Tx::Business::TxStock txstock;
									txstock.BlockRiskIndicatorAdv(
										10228,
										table,
										iset,
										iEndDate,
										lRefoSecurityId,
										iStartDate,
										iTradeDaysCount,
										iDuration,						//��������0=�գ�1=�ܣ�2=�£�3=����4=��
										bLogRatioFlag,						//������ʵķ�ʽ,true=ָ������,false=�򵥱���
										bClosePrice,						//ʹ�����̼�
										0,							//��׼������1=��ƫ��������ƫ
										0,									//ƫ��
										true,								//��Ȩ��־,true=ǰ��Ȩ,false=��Ȩ
										1,							//�û��Զ���������,����ȡһ�궨�ڻ�׼����
										dUserRate,
										iDuration==0?true:false,								//������ 2008-11-24 by zhaohj
										true,
										true
										);
									if (table.GetRowCount() == 1)
									{
										int iCol = 3;
										table.GetCell(iCol,0,std);
										iCol++;
										table.GetCell(iCol,0,alpha);
										iCol++;
										table.GetCell(iCol,0,beta);
										iCol++;
										table.GetCell(iCol,0,sharp);
										iCol++;
										table.GetCell(iCol,0,treynor);
										iCol++;
										table.GetCell(iCol,0,trackerror);
										iCol++;
										table.GetCell(iCol,0,dMean);
										iCol++;
									}
								}	
								//Tx::Business::TxStock txstock;
								//txstock.BlockRiskIndicatorAdv(
								//	10228,
								//	table,
								//	iset,
								//	iEndDate,
								//	lRefoSecurityId,
								//	iStartDate,
								//	iTradeDaysCount,
								//	iDuration,						//��������0=�գ�1=�ܣ�2=�£�3=����4=��
								//	bLogRatioFlag,						//������ʵķ�ʽ,true=ָ������,false=�򵥱���
								//	bClosePrice,						//ʹ�����̼�
								//	0,							//��׼������1=��ƫ��������ƫ
								//	0,									//ƫ��
								//	true,								//��Ȩ��־,true=ǰ��Ȩ,false=��Ȩ
								//	false,							//�û��Զ���������,����ȡһ�궨�ڻ�׼����
								//	dUserRate,
								//	iDuration==0?true:false,								//������ 2008-11-24 by zhaohj
								//	true
								//	);	
								
								switch( reqTXFunction.iFuncID )
								{
								case 212:
									dValue = std;
									break;
								case 213:
									dValue = alpha;
									break;
								case 214:
									dValue = beta;
									break;
								case 215:
									dValue = sharp;
									break;
								case 216:
									dValue = treynor;
									break;
								case 217:
									dValue = trackerror;
									break;
								case 287:
									dValue = dMean;
									break;
								default:
									dValue = 0.0;
									break;
								}
								if ( dValue != Con_doubleInvalid )
									strResult.Format(_T("%.6f"),dValue);
							}
							break;
						case 220:
						case 221:
						case 222:
						case 223:
						case 224:
						case 225:
						case 226:
						case 227:
						case 228:
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 13);	//������ȡ����񼾶ȡ��ϲ�/ĸ��˾������ǰ/��
							nInstitution = (INT)pBusiness->m_pSecurity->GetInstitutionId();
							iSubItemID = (int)reqTXFunction.iSubItemID-1;
							dValue = Tx::Core::Con_doubleInvalid;
							switch( reqTXFunction.iFuncID )
							{
							case 220:
								{
									nAccountingID = nInstitution * 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);
									//�����������ļ�								
									if(m_pBalanceYHDataFile!=NULL)
									{
										int m_instution = (INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter;
										if( m_pBalanceYHDataFile->Load(m_instution,30269,true) == true)
										{
											pBalance_YH = m_pBalanceYHDataFile->GetDataByObj(nAccountingID,false);
											if ( pBalance_YH!=NULL)
												dValue = pBalance_YH->dBalance[iSubItemID];
										}
									}
								}
								break;
							case 221:
								{
									nAccountingID = nInstitution * 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);
									//�����������ļ�									
									if(m_pBalanceBXDataFile!=NULL)
									{
										int m_instution = (INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter;
										if(m_pBalanceBXDataFile->Load(m_instution,30271,true) == true)
										{
											pBalance_BX = m_pBalanceBXDataFile->GetDataByObj(nAccountingID,false);
											if ( pBalance_BX!=NULL)
												dValue = pBalance_BX->dBalance[iSubItemID];
										}
									}									
								}
								break;
							case 222:
								{
									nAccountingID = nInstitution * 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);
									//�����������ļ�	
									if(m_pBalanceZQDataFile!=NULL)
									{
										int m_instution = (INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter;
										if(m_pBalanceZQDataFile->Load(m_instution,30273,true) == true)
										{
											pBalance_ZQ = m_pBalanceZQDataFile->GetDataByObj(nAccountingID,false);
											if ( pBalance_ZQ!=NULL)
												dValue = pBalance_ZQ->dBalance[iSubItemID];
										}
									}	
								}
								break;
							case 223:
								{
									nAccountingID = nInstitution * 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);
									//�����������ļ�	
									if(m_pProfitYHDataFile!=NULL)
									{
										int m_instution = (INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter;
										if(m_pProfitYHDataFile->Load(m_instution,30275,true) == true)
										{
											pProfit_YH = m_pProfitYHDataFile->GetDataByObj(nAccountingID,false);
											if ( pProfit_YH!=NULL)
												dValue = pProfit_YH->dProfit[iSubItemID];
											else
											{
												CString sLog;
												sLog.Format(_T("��������[ %s ]��������û�ж�Ӧ����\r\n"),pBusiness->m_pSecurity->GetName());
												Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
											}
										}
										else
										{
											CString sLog;
											sLog.Format(_T("��������[ %s ]���������ļ�����ʧ��\r\n"),pBusiness->m_pSecurity->GetName());
											Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
										}
									}	
								}
								break;
							case 224:
								{
									nAccountingID = nInstitution * 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);
									//�����������ļ�	
									if(m_pProfitBXDataFile!=NULL)
									{
										int m_instution = (INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter;
										if(m_pProfitBXDataFile->Load(m_instution,30277,true) == true)
										{
											pProfit_BX = m_pProfitBXDataFile->GetDataByObj(nAccountingID,false);
											if ( pProfit_BX!=NULL)
												dValue = pProfit_BX->dProfit[iSubItemID];
										}
									}	
								}
								break;
							case 225:
								{
									nAccountingID = nInstitution * 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);
									//�����������ļ�
									if(m_pProfitZQDataFile!=NULL)
									{
										int m_instution = (INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter;
										if(m_pProfitZQDataFile->Load(m_instution,30279,true) == true)
										{
											pProfit_ZQ = m_pProfitZQDataFile->GetDataByObj(nAccountingID,false);
											if ( pProfit_ZQ!=NULL)
												dValue = pProfit_ZQ->dProfit[iSubItemID];
										}
									}
								}
								break;
							case 226:
								{
									nAccountingID = nInstitution * 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);
									//�����������ļ�	
									bLoaded = true;
									if(m_pCashFlowYHDataFile!=NULL)
									{
										int m_instution = (INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter;
										if(m_pCashFlowYHDataFile->Load(m_instution,30263,true) == true)
										{
											pCashFlow_YH = m_pCashFlowYHDataFile->GetDataByObj(nAccountingID,false);
											if ( pCashFlow_YH!=NULL)
												dValue = pCashFlow_YH->dCashFlow[iSubItemID];
										}
									}
								}
								break;
							case 227:
								{
									nAccountingID = nInstitution * 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);
									//�����������ļ�	
									bLoaded = true;
									if(m_pCashFlowBXDataFile!=NULL)
									{
										int m_instution = (INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter;
										if(m_pCashFlowBXDataFile->Load(m_instution,30265,true) == true)
										{
											pCashFlow_BX = m_pCashFlowBXDataFile->GetDataByObj(nAccountingID,false);
											if ( pCashFlow_BX!=NULL)
												dValue = pCashFlow_BX->dCashFlow[iSubItemID];
										}
									}	
								}
								break;
							case 228:
								{
									nAccountingID = nInstitution * 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);
									//�����������ļ�	
									bLoaded = true;
									if(m_pCashFlowZQDataFile!=NULL)
									{
										int m_instution = (INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter;
										if(m_pCashFlowZQDataFile->Load(m_instution,30267,true) == true)
										{
											pCashFlow_ZQ = m_pCashFlowZQDataFile->GetDataByObj(nAccountingID,false);
											if ( pCashFlow_ZQ!=NULL)
												dValue = pCashFlow_ZQ->dCashFlow[iSubItemID];
										}
									}
								}
								break;
							default:
								break;
							}
							break;
							case 229:
								{
									vdReq.data_type = dtype_double;
									FindItemOfReq(&reqTXFunction, strReq, 27);
									CString m_strTempDate = _T("-");
									m_strTempDate.Format(_T("%d"),reqTXFunction.nDate);
									strResult = _T("-");
									strResult = Text2Date(m_strTempDate);
								}								
								break;
							case 230:
								{
									vdReq.data_type = dtype_double;
									FindItemOfReq(&reqTXFunction, strReq, 27);
									/*CString m_strTempDate = _T("-");
									m_strTempDate.Format(_T("%d"),reqTXFunction.nDate);
									strResult = _T("-");
									strResult = Date2Text(m_strTempDate);*/
									strResult = _T("-");
									strResult.Format(_T("%d"),reqTXFunction.nDate);
								}	
							case 232:
								{

								}
								break;
							case 233:
								{
									
								}
								break;
							case 234:
								{
									CString str = pBusiness->m_pSecurity->GetCode( true );
									int nTmp = str.Find(_T("."));
									if ( nTmp == -1 )
										break;
									str = str.Mid( nTmp+1 );
									if ( str.Compare(_T("SZ")))
										strResult = _T("����");
									else
										if ( str.Compare(_T("SH")))
											strResult = _T("�Ϻ�");
										else
											if ( str.Compare(_T("TX")))
												strResult = _T("����");
											else
												if ( str.Compare(_T("HK")))
													strResult = _T("���");
												else    /* 20100708 wanglm ָ���С��б꣬�����¸����������г� */ 
													if ( str.Compare(_T("SW")) == 0 )
														strResult = _T("����");
													else
														if ( str.Compare(_T("ZX")) == 0 )
															strResult = _T("����");
								}
								break;
							case 237:
								{
									FindItemOfReq(&reqTXFunction, strReq, 3);	// ndate
									vdReq.data_type = dtype_val_string;
									CString strPath = pBusiness->DownloadXmlFile(pBusiness->m_pSecurity->GetInstitutionId(),10269);
									OleInitialize(NULL);
									CXmlDocumentWrapper doc;
									if ( !doc.Load((LPCTSTR)strPath ))
										break;
									CString lpFind;
									lpFind.Format(_T("//tr[T6='%d']"),reqTXFunction.nDate);
									CXmlNodeWrapper root(doc.AsNode());
									//CString lpFind = 
									//	_T("//tr[*[name() = name(//ColumnName//*[@fieldName='f_end_date'])]='20071231']");
									//CXmlNodeWrapper configNode(root.FindNode(lpFind));
									CXmlNodeWrapper destNode(root.FindNode( lpFind ));
									strResult = _T("");
									if ( destNode.IsValid())
									{
										CString strText = destNode.GetText();

										CString sNodeItem = NULL;
										switch(iSubItemID)
										{
										case 0:
											sNodeItem = _T("T9");
											break;
										case 1:
											sNodeItem = _T("T10");
											break;
										case 2:
											sNodeItem = _T("T11");
											break;
										case 3:
											sNodeItem = _T("T12");
											break;
										case 4:
											sNodeItem = _T("T3");
											break;
										case 5:
											sNodeItem = _T("T13");
											break;
										case 6:
											sNodeItem = _T("T14");
											break;
										}

										CXmlNodeWrapper ValueNode(destNode.FindNode(sNodeItem));
										if( ValueNode.IsValid() )
										{
											strResult = ValueNode.GetText();
											if(iSubItemID == 3)
											{
												//ÿʮ�����ֽ��������ÿ�����ֽ���
												float fTmp = atof(strResult.GetBuffer());
												fTmp /= 10;
												strResult.ReleaseBuffer();
												strResult.Format(_T("%f"),fTmp);
											}
										}
									}
									OleUninitialize();
								}								
								break;
							case 238:		//������
								{
									vdReq.data_type = dtype_val_string;
									strResult = pBusiness->m_pSecurity->GetName();
								}								
								break;
							case 243:		//֤���淶������
								{
									vdReq.data_type = dtype_val_string;
									strResult = _T("-");
									int iSecId = pBusiness->m_pSecurity->GetId();
									std::unordered_map<int,CString>::iterator iterTmp;
									iterTmp = FundNameMap.find(iSecId);
									if(iterTmp != FundNameMap.end())
									{
										strResult = iterTmp->second;
									}
								}								
								break;
						//----------------------------------------------------------------------------
							case 198:
							case 206:
								{
									strResult = Con_strInvalid;
									Tx::Data::EarningsEstimateTx*	pEpsEstimate = NULL;
									FindItemOfReq(&reqTXFunction, strReq,20);
									pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_EarningsEstimateTx,reqTXFunction.nFYear,&pEpsEstimate,false);
									if ( pEpsEstimate == NULL )
										break;
									switch( reqTXFunction.iFuncID )
									{
									case 198:
										dValue = pEpsEstimate->f_tx_eps;
										break;
									case 206:
										dValue = pEpsEstimate->f_tx_ave;
										break;
									default:
										dValue = Con_doubleInvalid;
										break;
									}
									if ( dValue != Con_doubleInvalid )
										strResult.Format( _T("%.2f"), dValue );	
								}
								break;
							case 242:	//ȡ�ùɱ���ϸ����
								{
									dValue = 1.0;
									vdReq.data_type = dtype_int4;
									FindItemOfReq(&reqTXFunction, strReq, 5);	//StartDate��EndDate
									pStockBonusData = NULL;
									int ix = 0;
									int m_iCount = pBusiness->m_pSecurity->GetStockBonusDataCount();
									if(reqTXFunction.nStartDate > reqTXFunction.nEndDate)
									{
										strResult = _T("-");
									}
									else
									{
										for(ix = 0; ix < m_iCount; ix++)
										{
											pStockBonusData = pBusiness->m_pSecurity->GetStockBonusDataByIndex((INT)ix);
											if(pStockBonusData == NULL)
												continue;
											if(reqTXFunction.nStartDate < pStockBonusData->register_date && 
												reqTXFunction.nEndDate >= pStockBonusData->register_date)
											{
												if(reqTXFunction.nFYear > 0 && reqTXFunction.nFYear != (pStockBonusData->year / 10000))
													continue;
												if(pStockBonusData->sgshu > 0.0 && pStockBonusData->sgshu != Con_doubleInvalid)
													dValue *= 1+ pStockBonusData->sgshu;											
											}
										}
										strResult.Format(_T("%02f"), dValue);
									}
								}
								break;
							case 244:
								{
									strResult = Con_strInvalid;
									FindItemOfReq(&reqTXFunction, strReq,1);
									FindItemOfReq(&reqTXFunction, strReq,20);
									TxStock *pStock = new TxStock;
									dValue = pStock->GetForcastNetProfit(reqTXFunction.nSecurityID,reqTXFunction.nFYear);
									if ( dValue != Con_doubleInvalid )
										strResult.Format( _T("%.2f"), dValue );
								}
								break;
							case 245:
								{
									strResult = Con_strInvalid;
									FindItemOfReq(&reqTXFunction, strReq,1);
									TxStock *pStock = new TxStock;
									nDate = pStock->GetForcastIssueDate(reqTXFunction.nSecurityID);
									if(nDate > 0)
									{
										strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
									}
								}
								break;
							case 249:     //ծȯ�׷��۸�
								{
									vdReq.data_type = dtype_double;
									if (!pBusiness->m_pSecurity->IsBond())
									{
										break;
									}
									pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
									if (pBondNewInfo != NULL)
									{
										dValue = pBondNewInfo->issue_price;
										if (dValue != Con_doubleInvalid)
										{
											strResult.Format(_T("%.3f"),dValue);
										}
									}
								}
								break;
							case 250:     //ծȯ��׼ȯ������
								{
									// mantis:15006  2013-04-16
									if(!pBusiness->m_pSecurity->IsBond() && !pBusiness->m_pSecurity->IsFund())
									{
										break;
									}
									FindItemOfReq(&reqTXFunction, strReq,3);
									BondConvRate* pBondConvRate = NULL;
									BondConversionRatePast* pBondConversionRatePast = NULL;
									if ((int)reqTXFunction.nDate == 0)
									{
										pBondConvRate = pBusiness->m_pSecurity->GetBondConvRate();
										if (pBondConvRate != NULL)
										{
											dValue = pBondConvRate->f_rate;
											if(dValue > Con_doubleInvalid)
												strResult.Format(_T("%.2f"),dValue);
										}
									}
									else
									{
										int count = pBusiness->m_pSecurity->GetBondConversionRatePastCount();
										float fRate = Con_floatInvalid;
										if(count <= 0)
											break;
										for (int i=0;i<count;i++)
										{
											pBondConversionRatePast = pBusiness->m_pSecurity->GetBondConversionRatePastByIndex(i);
											if(pBondConversionRatePast == NULL)
												continue;
											if (reqTXFunction.nDate < pBondConversionRatePast->f_start)
												break;
											if ((int)reqTXFunction.nDate>=pBondConversionRatePast->f_start && (int)reqTXFunction.nDate <= pBondConversionRatePast->f_end)
											{
												fRate = pBondConversionRatePast->f_rate;
											}
										}
										dValue = fRate;
										if(dValue > Con_floatInvalid)
											strResult.Format(_T("%.2f"),dValue);
									}
									
								}
								break;
							case 251:     //ծȯ���±�׼ȯ�����ʿ�ʼ������
								{
									if(!pBusiness->m_pSecurity->IsBond() && !pBusiness->m_pSecurity->IsFund())
									{
										break;
									}
									FindItemOfReq(&reqTXFunction, strReq,3);
									BondConvRate* pBondConvRate = NULL;
									pBondConvRate = pBusiness->m_pSecurity->GetBondConvRate();
									if (pBondConvRate != NULL)
									{
										nDate = pBondConvRate->f_start;
										if (nDate > 0)
										{
											strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
										}
									}
								}
								break;
							case 252:     //ծȯ���±�׼ȯ�����ʽ���������
								{
									if(!pBusiness->m_pSecurity->IsBond() && !pBusiness->m_pSecurity->IsFund())
									{
										break;
									}
									FindItemOfReq(&reqTXFunction, strReq,3);
									BondConvRate* pBondConvRate = NULL;
									pBondConvRate = pBusiness->m_pSecurity->GetBondConvRate();
									if (pBondConvRate != NULL)
									{
										nDate = pBondConvRate->f_end;
										if (nDate > 0)
										{
											strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
										}
									}
								}
								break;
							case 255:
								{
									strResult = _T("");
									FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
									if ( !pBusiness->m_pSecurity->IsFund())
										break;
									Tx::Data::FundShareChangeNei* pFundShareChangeNei = pBusiness->m_pSecurity->GetFundShareChangNeiByDate((int)reqTXFunction.nDate);
									if(pFundShareChangeNei == NULL)
										break;
									if(pFundShareChangeNei->totalShare > 0)
										strResult.Format(_T("%0.2f"),pFundShareChangeNei->totalShare);
								}
								break;
							case 264:   //�������ӷ��
								{
									vdReq.data_type = dtype_val_string;
									if(!pBusiness->m_pSecurity->IsFund())
									{
										break;
									}
									//1.ͨ������ʵ��ID��ȡ���ӷ��ID
									mapIntInt.clear();
									Tx::Data::TypeMapManage::GetInstance()->GetTypeMapITI(TYPE_YHFUND_STYLE_ID_TO_INDEX, mapIntInt);
									iterIntInt = mapIntInt.find(reqTXFunction.nSecurityID);
									if (iterIntInt != mapIntInt.end())
									{
										//2.��ȡ���ӷ��str
										mapIntStr.clear();
										Tx::Data::TypeMapManage::GetInstance()->GetTypeMap(TYPE_YHFUND_STYLE_INDEX, mapIntStr);
										iterIntStr = mapIntStr.find(iterIntInt->second);
										if(iterIntStr != mapIntStr.end())
										{
											strValue = iterIntStr->second;
											if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
											{
												strResult = strValue;
											}
										}
									}
								}
								break;
							case 265:   //ծȯ����
								strResult = _T("ծȯ����");
								break;
							case 266:   //ծȯ��������
								strResult = _T("ծȯ����");
								break;
							case 267:   //ծȯ��������
								strResult = _T("ծȯ����");
								break;
							case 268:   //��ծ��������
								strResult = _T("ծȯ����");
								break;
							case 269:   //��ծ������������
								strResult = _T("ծȯ����");
								break;
							case 270:   //��ծ������������
								strResult = _T("ծȯ����");
								break;
							case 271:   //ծȯ���¶Ҹ�������
							case 272:   //ծȨ�Ǽ���
							case 273:   //ծȯ�Ҹ���ʼ��
							case 274:   //ծȯ��һ�θ�Ϣ�Ľ�ֹ����
							case 275:   //ÿ��Ԫ�ֽ���
							case 276:   //��Ϣ��
								break;
							case 277:   //����Ȩ��ֵ
								break;
							case 282:   //��ҵ�ֲ�(��)
								break;
							case 283:   //��ҵ�ֲ�ռ��(��)
								break;
						   //---------------------------------------------------------------
							default:
								break;
						}
					}
				}

				//if(strResult.GetLength() > 0)
				//{
				//	iterSheetReq->second = strResult;
				//	//2008-06-12 by zhaohj
				//	//vdReq
				//	vdReq.FromString(strResult);
				//	iterVdReq->second = vdReq;
				//}
			}
			if( m_pBalanceYHDataFile != NULL)
			{
				delete m_pBalanceYHDataFile;
				m_pBalanceYHDataFile = NULL;
			}
			if( m_pBalanceBXDataFile != NULL )
			{
				delete m_pBalanceBXDataFile;
				m_pBalanceBXDataFile = NULL;
			}
			if( m_pBalanceZQDataFile != NULL)
			{
				delete m_pBalanceZQDataFile;
				m_pBalanceZQDataFile = NULL;
			}
			if( m_pProfitYHDataFile != NULL )
			{
				delete m_pProfitYHDataFile;
				m_pProfitYHDataFile = NULL;
			}
			if( m_pProfitBXDataFile != NULL)
			{
				delete m_pProfitBXDataFile;
				m_pProfitBXDataFile = NULL;
			}
			if( m_pProfitZQDataFile != NULL)
			{
				delete m_pProfitZQDataFile;
				m_pProfitZQDataFile = NULL;
			}
			if(m_pCashFlowYHDataFile != NULL)
			{
				delete m_pCashFlowYHDataFile;
				m_pCashFlowYHDataFile = NULL;
			}
			if( m_pCashFlowBXDataFile != NULL)
			{
				delete m_pCashFlowBXDataFile;
				m_pCashFlowBXDataFile = NULL;
			}
			if( m_pCashFlowZQDataFile != NULL)
			{
				delete m_pCashFlowZQDataFile;
				m_pCashFlowZQDataFile = NULL;
			}

			tmEnd = CTime::GetCurrentTime();

			//singleLock.Lock();	//�̻߳���
			//ˢ��mapReq
			//for(iterSheetReq = mapSheetReq.begin(); iterSheetReq != mapSheetReq.end(); iterSheetReq ++)
			//{
			//	if(iterSheetReq->second.GetLength() > 0)
			//	{
			//		iterReq = pmapReq->find(iterSheetReq->first);
			//		if(iterReq != pmapReq->end())
			//		{
			//			iterReq->second.strValue = iterSheetReq->second;

			//			//2008-06-12 by zhaohj
			//			iterVdReq = mapSheetReqVariantData.find(iterSheetReq->first);
			//			iterReq->second.vdReq = iterVdReq->second;

			//			iterReq->second.nCalculateTime = tmEnd.GetTime();
			//		}
			//	}
			//}
			//singleLock.Unlock();

			//mapSheetReq.clear();
			//mapSheetReqVariantData.clear();//2008-06-12 by zhaohj
			vTopTenShareHolder.clear();
			vTopTenCShareHolder.clear();
			vTopTenFundHolder.clear();
			vTopTenCBondHolder.clear();
			vStockIssue.clear();
			vWarrantDV.clear();
			vCBondPrice.clear();
			vCBondInterest.clear();
			vCBondAmount.clear();
			vBondIPOInfo.clear();
			vFundNAVGrowth.clear();

			tmEnd = CTime::GetCurrentTime();
			TRACE(_T("\n--��ʷָ���VBA�����������ʱ�䣺%4d-%02d-%02d %02d:%02d:%02d--\n"), 
				tmEnd.GetYear(), tmEnd.GetMonth(), tmEnd.GetDay(), 
				tmEnd.GetHour(), tmEnd.GetMinute(), tmEnd.GetSecond());


	//------------wanzhy-----------
	//--------------20080529-------------
	//*(lpParam_HisIndCalc->lpbThreadTerm) = TRUE;
	//Tx::Log::CLogRecorder::GetInstance()->WriteToLog(_T("Excel�˳���ʷ����1,�������"));
	return strResult;
}

//---------�����漰��ʷ��ָ�����----------------------------wangzhy---------------
UINT TxIndicator::HisIndCalc_Proc(LPVOID lpParam)
{
	//Ŀǰָ�꣺1 - 288
	CString sLog;
	sLog = _T("Excel������ʷ�߳��ڲ�");
	//Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
	CString strReq, strResult, strValue,strValue1;
	LONG nIndustryID;
	Tx::Core::Table_Indicator subtable_ID, subtable_Year, subtable_Quarter;
	Tx::Data::TxShareData *pTxShareData;
	Tx::Data::TxFundShareData *pTxFundShareData;
	Tx::Data::BondNotChangeAmount *pCBondAmount;
	Tx::Data::IndexConstituentData *pIndexConstituentData;
	Tx::Data::SecurityQuotation *pSecurity;
	Tx::Data::FundNewInfo *pFundNewInfo;
	Tx::Data::BondNewInfo *pBondNewInfo;
	Tx::Data::BondCashFlowData *pBondCashFlowData;
	Tx::Data::FundNetValueData *pFundNetValueData;
	Tx::Data::FundGainData *pFundGainData;
	Tx::Data::StockBonusData *pStockBonusData;
	Tx::Data::FundBonusData *pFundBonusData;
	Tx::Data::BondGeneral *pBondGeneral;
	Tx::Business::TxWarrant warrant;
	Tx::Business::TxBond bond;
	Tx::Business::TxBusiness business;
	Tx::Data::TradeQuotation *pTradeQuotation;
	Tx::Data::TradeQuotationData *pTradeQuotationData;
	Tx::Data::HisTradeDataCustomStruct* pHisDetail = NULL;
	INT64 nAccountingID;
	INT nDate, nTradeDays, nInstitution, iVal, iIndexObjID, nDataCount, nSecurity1ID, iTransObjID, nStartDate, nEndDate, iRowNo, nCount;
	ReqMapIter iterReq;
	StrSetIter iterReqCalc;
	StrMapIter iterSheetReq, iterIndicatorReq;
	StrMMapIter iterNameCode;
	CTime tmStart, tmEnd, tmFrom, tmTo;
	CTimeSpan tsInterVal;
	DOUBLE dValue, dValue1, dValue2, dValue3,dValue4;
	UINT i, array_Col[4] = {0, 1, 2, 3};
	StrSet *psetReqCalc;
	vector <int> vInstitution, vDate;
	vector <UINT> vRow;
	Indicator_Dates indDates;
	Indicator_Date indDate;
	Indicator_ShareHolder indShareHolder;
	//��Ʊ���м۸�Ȩ֤ʣ�ഴ���������תծת�ɼ۸񡢿�תծ���ʡ���תծת��������ծȯ������Ϣ
	vector_IndDate vStockIssue, vWarrantDV, vCBondPrice, vCBondInterest, vCBondAmount, vBondIPOInfo;
	vector_IndDates vFundNAVGrowth;	//����ֵ������
	vector_IndShareHolder vTopTenShareHolder, vTopTenCShareHolder, vTopTenFundHolder, vTopTenCBondHolder;
	//byte cSubject;
	INT_Map mapIntInt;
	INT_Map_Iter iterIntInt;
	INTSTR_Map mapIntStr;
	INTSTR_Map_Iter iterIntStr;

	//����--------------------------
	int iSubItemID;					//---20080527
	Tx::Data::Balance*	pBalance = NULL;	//---20080528	
	DataFileNormal<blk_TxExFile_FileHead,Tx::Data::Balance>* pBalanceDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::Balance>;//20080529
	pBalanceDataFile->SetCheckLoadById(true);
	Tx::Data::Financial*	pFinancial = NULL;
	DataFileNormal<blk_TxExFile_FileHead,Tx::Data::Financial>* pFinancialDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::Financial>;
	pFinancialDataFile->SetCheckLoadById(true);
	Tx::Data::CashFlow*	pCashFlow =NULL;
	DataFileNormal<blk_TxExFile_FileHead,Tx::Data::CashFlow>* pCashFlowDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::CashFlow>;
	pCashFlowDataFile->SetCheckLoadById(true);
	Tx::Data::Income*	pIncome = NULL;
	DataFileNormal<blk_TxExFile_FileHead,Tx::Data::Income>* pIncomeDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::Income>;
	pIncomeDataFile->SetCheckLoadById(true);

	//wangzhy-------------20080527
	Tx::Data::Holder*	pHolder		 = NULL;	//--wangzhy--20080603
	Tx::Data::TradeableHolder*	pTradeableHolder = NULL;	//--wangzhy--20080603
	Tx::Data::FundBondHolder*	pFundBondHolder = NULL;		//--wangzhy--20080604
	
	Tx::Data::PrimeOperatingRevenue*	pRevenue	= NULL;	//--wangzhy--20080604
	DataFileNormal<blk_TxExFile_FileHead,Tx::Data::PrimeOperatingRevenue>* pRevenueDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::PrimeOperatingRevenue>;//20080605
	pRevenueDataFile->SetCheckLoadById(true);
	
	Tx::Data::AssetsDepreciationReserves*	pDepreciation = NULL;	//--wangzhy--20080604
	DataFileNormal<blk_TxExFile_FileHead,Tx::Data::AssetsDepreciationReserves>* pDepreciationDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::AssetsDepreciationReserves>;//20080610
	pDepreciationDataFile->SetCheckLoadById(true);
	Tx::Data::NonRecurringGainsAndLosses*	pGainsAndLosses = NULL;	//--wangzhy--20080604
	DataFileNormal<blk_TxExFile_FileHead,Tx::Data::NonRecurringGainsAndLosses>* pGainsAndLossesDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::NonRecurringGainsAndLosses>;//20080610
	pGainsAndLossesDataFile->SetCheckLoadById(true);
	Tx::Data::AccountsReceivable*	pAccountReceivable = NULL;	//--wangzhy--20080604
	DataFileNormal<blk_TxExFile_FileHead,Tx::Data::AccountsReceivable>* pAccountsReceivableDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::AccountsReceivable>;//20080610
	pAccountsReceivableDataFile->SetCheckLoadById(true);
	Tx::Data::FinanceCharge*	pFinanceCharge = NULL;	//--wangzhy--20080604
	DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FinanceCharge>* pFinanceChargeDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FinanceCharge>;//20080610
	pFinanceChargeDataFile->SetCheckLoadById(true);
	//--------------����ָ��ṹ-----zhangxs200815---------------
	Tx::Data::FundBalance*	pFundBalance = NULL;	
	DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundBalance>* pFundBalanceDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundBalance>;
	pFundBalanceDataFile->SetCheckLoadById(true);
	Tx::Data::FundFinancial*	pFundFinancial = NULL;
	DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundFinancial>* pFundFinancialDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundFinancial>;
	pFundFinancialDataFile->SetCheckLoadById(true);
	Tx::Data::FundAchievement*	pFundAchievement =NULL;
	DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundAchievement>* pFundAchievementDataFile = new DataFileNormal<blk_TxExFile_FileHead,
		Tx::Data::FundAchievement>;
	pFundAchievementDataFile->SetCheckLoadById(true);
	Tx::Data::FundRevenue*	pFundRevenue = NULL;
	DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundRevenue>* pFundRevenueDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundRevenue>;
	pFundRevenueDataFile->SetCheckLoadById(true);
	Tx::Data::FundNavChange*	pFundNavChange = NULL;
	DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundNavChange>* pFundNavChangeDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundNavChange>;
	pFundNavChangeDataFile->SetCheckLoadById(true);
	Tx::Data::FundInvesmentGroup*	pFundInvesmentGroup = NULL;						//Ͷ�����
	DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundInvesmentGroup>* pFundInvesmentGroupDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundInvesmentGroup>;
	pFundInvesmentGroupDataFile->SetCheckLoadById(true);
	Tx::Data::FundCombineInvestmentIndustryDistribute* pFundComInvDistribute = NULL ;//�ϲ���Ͷ�������ҵ�ֲ�
	DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundCombineInvestmentIndustryDistribute>* pFundComInvDistributeDataFile = new DataFileNormal<blk_TxExFile_FileHead,
		Tx::Data::FundCombineInvestmentIndustryDistribute>;
	pFundComInvDistributeDataFile->SetCheckLoadById(true);
	Tx::Data::FundStockInvestmentIndustryDistribute*	pFundStockInvDistribute = NULL; //����Ͷ�������ҵ�ֲ�
	DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundStockInvestmentIndustryDistribute>* pFundStockInvDistributeDataFile = new DataFileNormal<blk_TxExFile_FileHead,
		Tx::Data::FundStockInvestmentIndustryDistribute>;
	pFundStockInvDistributeDataFile->SetCheckLoadById(true);
	Tx::Data::FundIndexInvestmentIndustryDistribute*	pFundIndixInvDistribution = NULL;//ָ����Ͷ�������ҵ�ֲ�
	DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundIndexInvestmentIndustryDistribute>* pFundIndixInvDistributionDataFile = new DataFileNormal<blk_TxExFile_FileHead,
		Tx::Data::FundIndexInvestmentIndustryDistribute>;
	pFundIndixInvDistributionDataFile->SetCheckLoadById(true);
	Tx::Data::FundBlockInvestmentTxIndustryDistribute*	pFundTXInvDistribution = NULL;//����Ͷ�������ҵ�ֲ�
	DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundBlockInvestmentTxIndustryDistribute>* pFundTXInvDistributionDataFile = new DataFileNormal<blk_TxExFile_FileHead,
		Tx::Data::FundBlockInvestmentTxIndustryDistribute>;
	pFundTXInvDistributionDataFile->SetCheckLoadById(true);

	Tx::Data::FundCombineInvestmentVIPStock*	pFundCombineVipStock = NULL;			//�ϲ�Ͷ�ʲ����زֹ�
	DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundCombineInvestmentVIPStock>* pFundCombineVipStockDataFile = new DataFileNormal<blk_TxExFile_FileHead,
		Tx::Data::FundCombineInvestmentVIPStock>;
	pFundCombineVipStockDataFile->SetCheckLoadById(true);
	Tx::Data::FundStockInvestmentVIPStock*	pFundstockVipStock = NULL;		//���������زֹ�
	DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundStockInvestmentVIPStock>* pFundstockVipStockDataFile = new DataFileNormal<blk_TxExFile_FileHead,
		Tx::Data::FundStockInvestmentVIPStock>;
	pFundstockVipStockDataFile->SetCheckLoadById(true);
	Tx::Data::FundIndexInvestmentVIPStock*	pFundIndexVipStock = NULL;		//ָ���������زֹ�
	DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundIndexInvestmentVIPStock>* pFundIndexVipStockDataFile = new DataFileNormal<blk_TxExFile_FileHead,
		Tx::Data::FundIndexInvestmentVIPStock>;
	pFundIndexVipStockDataFile->SetCheckLoadById(true);

	Tx::Data::FundHoldStockDetail*	pFundHoldStockDetail = NULL;	//�ֹ���ϸ
	DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundHoldStockDetail>* pFundHoldStockDetailDataFile = new DataFileNormal<blk_TxExFile_FileHead,
		Tx::Data::FundHoldStockDetail>;
	pFundHoldStockDetailDataFile->SetCheckLoadById(true);
	pFundHoldStockDetailDataFile->SetOverWriteMapKey(true);

	Tx::Data::FundPositionInfo*	pFundPositionInfo = NULL;	//�ֲֽṹ
	DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundPositionInfo>* pFundPositionInfoDataFile = new DataFileNormal<blk_TxExFile_FileHead,
		Tx::Data::FundPositionInfo>;
	pFundPositionInfoDataFile->SetCheckLoadById(true);
	pFundPositionInfoDataFile->SetOverWriteMapKey(true);

	Tx::Data::FundShareDataChange*	pFundShareDataChange = NULL;	//�ݶ�䶯
	DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundShareDataChange>* pFundShareDataChangeDataFile = new DataFileNormal<blk_TxExFile_FileHead,
		Tx::Data::FundShareDataChange>;
	pFundShareDataChangeDataFile->SetCheckLoadById(true);
	pFundShareDataChangeDataFile->SetOverWriteMapKey(true);

	Tx::Data::FundBuyTotStock*	pFundBuyTotStock = NULL;	//�ۼ������Ʊ
	DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundBuyTotStock>* pFundBuyTotStockDataFile = new DataFileNormal<blk_TxExFile_FileHead,
		Tx::Data::FundBuyTotStock>;
	pFundBuyTotStockDataFile->SetCheckLoadById(true);
	pFundBuyTotStockDataFile->SetOverWriteMapKey(true);


	Tx::Data::FundSaleTotStockForSecurity*	pFundSaleTotStock = NULL;	//�ۼ�������Ʊ
	DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundSaleTotStockForSecurity>* pFundSaleTotStockDataFile = new DataFileNormal<blk_TxExFile_FileHead,
		Tx::Data::FundSaleTotStockForSecurity>;
	pFundSaleTotStockDataFile->SetCheckLoadById(true);
	pFundSaleTotStockDataFile->SetOverWriteMapKey(true);

	Tx::Data::FundTradeVolumeInfo*	pFundTradeVolumeInfo = NULL;	//ȯ��ϯλ
	DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundTradeVolumeInfo>* pFundTradeVolumeInfoDataFile = new DataFileNormal<blk_TxExFile_FileHead,
		Tx::Data::FundTradeVolumeInfo>;
	pFundTradeVolumeInfoDataFile->SetCheckLoadById(true);
	pFundTradeVolumeInfoDataFile->SetOverWriteMapKey(true);

	//---------------------------wangzhy--------------------------------------------
	Tx::Data::FundBondGroup*	pFundBondGroup = NULL;				//����ծȯ���
	DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundBondGroup>* pFundBondGroupDataFile = new DataFileNormal<blk_TxExFile_FileHead,
		Tx::Data::FundBondGroup>;

	Tx::Data::FundBondDetail*	pFundBondDetail	= NULL;				//�����ծ��ϸ
	DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundBondDetail>* pFundBondDetailDataFile = new DataFileNormal<blk_TxExFile_FileHead,
		Tx::Data::FundBondDetail>;
	pFundBondDetailDataFile->SetCheckLoadById(true);
	pFundBondDetailDataFile->SetOverWriteMapKey(true);
	Tx::Data::FundBondChangeDetail*	pFundCBondDetail = NULL;		//�����תծ��ϸ
	DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundBondChangeDetail>* pFundCBondDetailDataFile = new DataFileNormal<blk_TxExFile_FileHead,
		Tx::Data::FundBondChangeDetail>;
	pFundCBondDetailDataFile->SetCheckLoadById(true);
	pFundCBondDetailDataFile->SetOverWriteMapKey(true);

	Tx::Data::FundOtherAsset*	pFundOtherAsset = NULL;				//���������ʲ�
	DataFileNormal<blk_TxExFile_FileHead,Tx::Data::FundOtherAsset>* pFundOtherAssetDataFile = new DataFileNormal<blk_TxExFile_FileHead,
		Tx::Data::FundOtherAsset>;
	//---------------------------20080829-------------------------------------------
	Tx::Data::EarningsEstimateTx*	pEpsEstimate = NULL;			//Ԥ��eps
	
	Tx::Data::AmountFlow*	pAmountFlow = NULL;
	DataFileNormal<blk_TxExFile_FileHead,Tx::Data::AmountFlow>* pAmountFlowDataFile = new DataFileNormal<blk_TxExFile_FileHead,
		Tx::Data::AmountFlow>;
	pAmountFlowDataFile->SetCheckLoadById(true);
	//--------------------------------20080812--------------
	Tx::Data::GeneralBalanceForInsdutry*	pBalanceStat = NULL;
	DataFileNormal<blk_TxExFile_FileHead,Tx::Data::GeneralBalanceForInsdutry>* pBalanceStatDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::GeneralBalanceForInsdutry>;
	pBalanceStatDataFile->SetCheckLoadById(true);

	Tx::Data::GeneralBriefForInsdutry*	pProfitStat = NULL;
	DataFileNormal<blk_TxExFile_FileHead,Tx::Data::GeneralBriefForInsdutry>* pProfitStatDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::GeneralBriefForInsdutry>;
	pProfitStatDataFile->SetCheckLoadById(true);	

	Tx::Data::GeneralCashflowForInsdutry*	pCashFlowStat = NULL;
	DataFileNormal<blk_TxExFile_FileHead,Tx::Data::GeneralCashflowForInsdutry>* pCashFlowStatDataFile = new DataFileNormal<blk_TxExFile_FileHead,Tx::Data::GeneralCashflowForInsdutry>;
	pCashFlowStatDataFile->SetCheckLoadById(true);

	Tx::Data::HisTradeLatestDataOfaDayStruct* pDataOfaDay = NULL;

	//added by zhangxs 20090218
	Balance_YH* pBalance_YH = NULL;
	DataFileNormal<blk_TxExFile_FileHead,Balance_YH>* m_pBalanceYHDataFile = new DataFileNormal<blk_TxExFile_FileHead,Balance_YH>;
	m_pBalanceYHDataFile->SetCheckLoadById(true);
	Balance_BX* pBalance_BX = NULL;
	DataFileNormal<blk_TxExFile_FileHead,Balance_BX>* m_pBalanceBXDataFile = new DataFileNormal<blk_TxExFile_FileHead,Balance_BX>;
	m_pBalanceBXDataFile->SetCheckLoadById(true);
	Balance_ZQ* pBalance_ZQ = NULL;
	DataFileNormal<blk_TxExFile_FileHead,Balance_ZQ>* m_pBalanceZQDataFile = new DataFileNormal<blk_TxExFile_FileHead,Balance_ZQ>;
	m_pBalanceZQDataFile->SetCheckLoadById(true);
	Profit_YH* pProfit_YH = NULL;
	DataFileNormal<blk_TxExFile_FileHead,Profit_YH>* m_pProfitYHDataFile = new DataFileNormal<blk_TxExFile_FileHead,Profit_YH>;
	m_pProfitYHDataFile->SetCheckLoadById(true);
	Profit_BX* pProfit_BX = NULL;
	DataFileNormal<blk_TxExFile_FileHead,Profit_BX>* m_pProfitBXDataFile = new DataFileNormal<blk_TxExFile_FileHead,Profit_BX>;
	m_pProfitBXDataFile->SetCheckLoadById(true);
	Profit_ZQ* pProfit_ZQ = NULL;
	DataFileNormal<blk_TxExFile_FileHead,Profit_ZQ>* m_pProfitZQDataFile = new DataFileNormal<blk_TxExFile_FileHead,Profit_ZQ>;
	m_pProfitZQDataFile->SetCheckLoadById(true);
	CashFlow_YH* pCashFlow_YH = NULL;
	DataFileNormal<blk_TxExFile_FileHead,CashFlow_YH>* m_pCashFlowYHDataFile = new DataFileNormal<blk_TxExFile_FileHead,CashFlow_YH>;
	m_pCashFlowYHDataFile->SetCheckLoadById(true);
	CashFlow_BX* pCashFlow_BX = NULL;
	DataFileNormal<blk_TxExFile_FileHead,CashFlow_BX>* m_pCashFlowBXDataFile = new DataFileNormal<blk_TxExFile_FileHead,CashFlow_BX>;
	m_pCashFlowBXDataFile->SetCheckLoadById(true);
	CashFlow_ZQ* pCashFlow_ZQ = NULL;
	DataFileNormal<blk_TxExFile_FileHead,CashFlow_ZQ>* m_pCashFlowZQDataFile = new DataFileNormal<blk_TxExFile_FileHead,CashFlow_ZQ>;
	m_pCashFlowZQDataFile->SetCheckLoadById(true);

	//-------------------------
	std::set<int>	nSetNvrSecurity;
	std::set<int>	nSetNvrDate;

	std::set<int>	nSetAcuNvrSecurity;
	std::set<int>	nSetAcuNvrDate;
	//-------------------------
	std::set<int>	nSetMmfNvrSecurity;
	std::set<int>	nSetMmfNvrDate;

	std::set<int>	nSetMmfAcuNvrSecurity;
	std::set<int>	nSetMmfAcuNvrDate;
	//---------------------------
	
	//-------------------------
	std::set<int>	nSetNvrSecurityExt;
	std::set<int>	nSetNvrDateExt;

	std::set<int>	nSetAcuNvrSecurityExt;
	std::set<int>	nSetAcuNvrDateExt;
	//-------------------------
	std::set<int>	nSetMmfNvrSecurityExt;
	std::set<int>	nSetMmfNvrDateExt;

	std::set<int>	nSetMmfAcuNvrSecurityExt;
	std::set<int>	nSetMmfAcuNvrDateExt;
	//---------------------------

	bool bReqYtm = false;
	
	//������ǿתΪ��ʷָ��������
	//HisIndCalc_Param *lpParam_HisIndCalc = (HisIndCalc_Param *)lpParam;
	HisIndCalc_Param *lpParam_HisIndCalc = &param_HisIndCalc;

	Tx::Business::TxBusiness *pBusiness = lpParam_HisIndCalc->pBusiness;
	ReqMap *pmapReq = lpParam_HisIndCalc->pmapReq;
	CSingleLock singleLock(lpParam_HisIndCalc->pCS);
	int	m_inDate = 0; //��¼VBA��������Date��nDate��ƥ��zhangxs-----20080818

	//�嵲�������������Ƿ����
	bool bBpYtmLoad[5];
	bool bSpYtmLoad[5];
	for(int i=0;i<5;i++)
	{
		bSpYtmLoad[i] = false;
		bBpYtmLoad[i] = false;
	}

	//��¼�߳�����״̬
#if _DEBUG
	if (*(lpParam_HisIndCalc->lpbThreadRun) == TRUE)
		Tx::Log::CLogRecorder::GetInstance()->WriteToLog(_T("Excel�߳�����״̬:True"));
	else
		Tx::Log::CLogRecorder::GetInstance()->WriteToLog(_T("Excel�߳�����״̬:False"));
#endif

	//��lEvtCntFlag��lEvtCnt���Ƚ������ֵ��ͬ�򲻼�����־��Ϊ�����ظ���¼
	LONG lEvtCntFlag = -1;
	BOOL bCalculateFlag = 3;
	//�߳�������
	while(*(lpParam_HisIndCalc->lpbThreadRun) == TRUE)
	{
		//��ΪҪʵ��������,û��Ҫ�жϷ���ֵ
		::WaitForSingleObject(m_hEvent_ever, 1000 );
		singleLock.Lock();	//�̻߳���
		/*if((*(lpParam_HisIndCalc->lpnShCalcEvtCnt) == 0)&&(lpParam_HisIndCalc->psetReqB != NULL || lpParam_HisIndCalc->psetReqA != NULL))
		{
			*(lpParam_HisIndCalc->lpnShCalcEvtCnt) = 1;
		}*/

		//��¼����ʵ������
		CString sLog1;
		LONG lEvtCnt = *(lpParam_HisIndCalc->lpnShCalcEvtCnt);
		if (lEvtCntFlag != lEvtCnt)
		{
			lEvtCntFlag = lEvtCnt;
#if _DEBUG
			sLog1.Format(_T("Excel����ʵ��������%d"),lEvtCnt);
			Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog1);
#endif
		}


		if(*(lpParam_HisIndCalc->lpnShCalcEvtCnt) > 0)
		{
			//ʵ��������1
			*(lpParam_HisIndCalc->lpnShCalcEvtCnt) = (*(lpParam_HisIndCalc->lpnShCalcEvtCnt)) - 1;
			//���Aæ�����B����֮����A
			if(*(lpParam_HisIndCalc->lpbsetAIsBusy))
			{
				psetReqCalc = lpParam_HisIndCalc->psetReqB;
#if _DEBUG
				sLog1.Format(_T("Excel�߳�B,reqCount=%d"),psetReqCalc->size());
				Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog1);
#endif
			}else
			{
				psetReqCalc = lpParam_HisIndCalc->psetReqA;
#if _DEBUG
				sLog1.Format(_T("Excel�߳�A,reqCount=%d"),psetReqCalc->size());
				Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog1);
#endif
			}
			if(psetReqCalc->size() > 0)
			{
				(*(lpParam_HisIndCalc->lpbCalculate)) = TRUE;
				*(lpParam_HisIndCalc->lpbsetAIsBusy) = !(*(lpParam_HisIndCalc->lpbsetAIsBusy));
			}else
			{
				if(*(lpParam_HisIndCalc->lpnShCalcEvtCnt) == 0)
				{
					*(lpParam_HisIndCalc->lpbAnyShCalcEvt) = FALSE;
				}
			}
		}
		singleLock.Unlock();


		//��¼ָ�����״̬ ,Ϊ�˱����ظ���¼
		BOOL bCalculate = *(lpParam_HisIndCalc->lpbCalculate);
		if (bCalculate != bCalculateFlag)
		{
#if _DEBUG
			if((*(lpParam_HisIndCalc->lpbCalculate)) == TRUE)
				Tx::Log::CLogRecorder::GetInstance()->WriteToLog(_T("Excel�̼߳���״̬:True"));
			else
				Tx::Log::CLogRecorder::GetInstance()->WriteToLog(_T("Excel�̼߳���״̬:False"));
#endif
			bCalculateFlag = bCalculate;
		}

		if((*(lpParam_HisIndCalc->lpbCalculate)) == TRUE)
		{	//Excel����SheetCalculate�¼���Ի����VBA������м���
			tmStart = (CTime::GetCurrentTime()).GetTime();
			CString sLog;
#if _DEBUG
			sLog.Format(_T("\n--��ʷָ���VBA���������ʼʱ�䣺%4d-%02d-%02d %02d:%02d:%02d--\n"), 
				tmStart.GetYear(), tmStart.GetMonth(), tmStart.GetDay(), 
				tmStart.GetHour(), tmStart.GetMinute(), tmStart.GetSecond());
			Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog,TRUE);	
#endif			
			nBalanceSecuritySet.clear();	
			nBalanceDateSet.clear();		
			nCashflowSecuritySet.clear();
			nCashflowDateSet.clear();
			nFinancialSecuritySet.clear();
			nFinancialDateSet.clear();
			nIncomeSecuritySet.clear();
			nIncomeDateSet.clear();
			nRevenueSecuritySet.clear();
			nRevenueDateSet.clear();
			nAccountReceivableSecuritySet.clear();
			nAccountReceivableDateSet.clear();
			nGainsAndLossesSecuritySet.clear();
			nGainsAndLossesDateSet.clear();
			nDepreciationSecuritySet.clear();
			nDepreciationDateSet.clear();
			nFinanceChargeSecuritySet.clear();
			nFinanceChargeDateSet.clear();
			//------------------zhangxs-----20080818-----------
			nFundFinancialSecuritySet.clear();  //������Ҫ����ָ��
			nFundFinancialDateSet.clear();
			nFundBalanceSecuritySet.clear();	  //�����ʲ���ծ
			nFundBalanceDateSet.clear();		
			nFundAchievementSet.clear();    //��Ӫҵ��
			nFundAchievementDateSet.clear();
			nFundRevenueSet.clear();	//�������
			nFundRevenueDateSet.clear();
			nFundNavChangeSet.clear();	//��ֵ�䶯
			nFundNavChangeDateSet.clear();
			//nFundAssetAllocationSet.clear();	//Ͷ�����
			//nFundAssetAllocationDateSet.clear();
			////nFundCloseIssuseInfoSet.clear();	//���ʽ��������Ϣ
			////nFundCloseIssuseInfoDateSet.clear();
			////nFundOpenIssuseInfoSet.clear();	//����ʽ��������Ϣ
			////nFundOpenIssuseInfoDateSet.clear();
			nFundManagerSet.clear();	//������
			nFundManagerDateSet.clear();
			nFundShareSet.clear();	//����ݶ�
			nFundShareDateSet.clear();
			nFundCombineInvestmentIndustryDistributeSet.clear();	//����ϲ�Ͷ�ʲ�����ҵ�ֲ���
			nFundCombineInvestmentIndustryDistributeDateSet.clear();
			nFundStockInvestmentIndustryDistributeSet.clear();	//�������Ͷ�ʲ�����ҵ�ֲ���
			nFundStockInvestmentIndustryDistributeDateSet.clear();
			nFundIndexInvestmentIndustryDistributeSet.clear();	//����ָ��Ͷ�ʲ�����ҵ�ֲ���
			nFundIndexInvestmentIndustryDistributeDateSet.clear();
			nFundIndexInvestmentIndustryDistributeSet.clear();	//Ͷ�����������ҵ�ֲ���
			nFundTXInvestmentIndustryDistributeDateSet.clear();
			nFundCombineVIPStockSet.clear();	//�ϲ�Ͷ�ʲ����زֹ�
			nFundCombineVIPStockDateSet.clear();
			nFundStockVIPStockeSet.clear();	//���������زֹ�
			nFundStockVIPStockDateSet.clear();
			nFundIndexVIPStockSet.clear();	//ָ���������زֹ�
			nFundIndexVIPStockDateSet.clear();	
			nFundBondPortfolio.clear();			//ծȯ���
			nFundBondPortfolioDateSet.clear();	
			nFundBondHoldDetail.clear();			//��ծ��ϸ
			nFundBondHoldDetailDateSet.clear();
			nFundCBondHoldDetail.clear();		//�ֿ�תծ��ϸ
			nFundCBondHoldDetailDateSet.clear();
			nFundOtherAsset.clear();				//�����ʲ�
			nFundOtherAssetDateSet.clear();
			nFlowOfCapitalSet.clear();				//�ʽ�����
			nFlowOfCapitalDateSet.clear();
			nBalanceStat.clear();					//
			nPEPBSet.clear();
			nBalanceStatDate.clear();
			nProfitStat.clear();
			nProfitStatDate.clear();
			nCashFlowStat.clear();
			nCashFlowStatDate.clear();

			nBalanceYHSet.clear();					//Balance_YH
			nBalanceYHDateSet.clear();
			nBalanceBXSet.clear();					//Balance_BX
			nBalanceBXDateSet.clear();
			nBalanceZQSet.clear();					//Balance_ZQ
			nBalanceZQDateSet.clear();
			nProfitYHSet.clear();					//Profit_YH
			nProfitYHDateSet.clear();		
			nProfitBXSet.clear();					//Profit_BX
			nProfitBXDateSet.clear();		
			nProfitZQSet.clear();					//Profit_ZQ
			nProfitZQDateSet.clear();			
			nCashFlowYHSet.clear();					//CashFlow_YH
			nCashFlowYHDateSet.clear();
			nCashFlowBXSet.clear();					//CashFlow_BX
			nCashFlowBXDateSet.clear();			
			nCashFlowZQSet.clear();					//CashFlow_ZQ
			nCashFlowZQDateSet.clear();
			//-------����ֵ������-------
			m_nNvrDates.clear();
			m_nNvrSample.clear();
			m_nNvrTable.Clear();
			m_d2cMap.clear();
			//-------20090106-------

			nSetNvrSecurity.clear();
			nSetNvrDate.clear();

			nSetAcuNvrSecurity.clear();
			nSetAcuNvrDate.clear();
			
			nSetMmfNvrSecurity.clear();
			nSetMmfNvrDate.clear();

			nSetMmfAcuNvrSecurity.clear();
			nSetMmfAcuNvrDate.clear();


			nSetNvrSecurityExt.clear();
			nSetNvrDateExt.clear();

			nSetAcuNvrSecurityExt.clear();
			nSetAcuNvrDateExt.clear();
			
			nSetMmfNvrSecurityExt.clear();
			nSetMmfNvrDateExt.clear();

			nSetMmfAcuNvrSecurityExt.clear();
			nSetMmfAcuNvrDateExt.clear();

			//-----------------------------wangzhy-20080526----------------------
			singleLock.Lock();	//�̻߳���
			for(iterReqCalc = psetReqCalc->begin(); iterReqCalc != psetReqCalc->end(); iterReqCalc ++)
			{
				// Ŀǰָ��ID  1 -  
				strReq = (*iterReqCalc);
				FindItemOfReq(&reqTXFunction, strReq);	//FuncID��SubItemID��IndicatorID
				switch(reqTXFunction.iFuncID)
				{
				case 1:		//֤ȯ����(���г���׺)
				case 2:		//֤ȯ����(���г���׺)
				case 3:		//֤ȯ���
				case 4:		//A�ɴ���
				case 5:		//B�ɴ���
				case 6:		//ע�����
				case 7:		//֤�����ҵ
				case 281:   //֤�����ҵ[��]
				case 8:		//������ҵ
				case 218:	//֤�����ҵ����
				case 280:   //֤�����ҵ����[��]
				case 219:	//������ҵ����
				case 256:   //��֤��ҵ����   2012-10-10
				case 257:   //��֤��ҵ����
				case 258:   //������ҵ����
				case 259:   //������ҵ����
				case 260:   //������ҵ����
				case 261:   //������ҵ����
				case 262:   //�޳���ҵ����
				case 263:   //�޳���ҵ����
				case 10:	//��˾ȫ��
				case 11:	//IPO��������
				case 12:	//IPO��������
				case 13:	//��������
				case 14:	//��������
				case 15:	//���̼�
				case 16:	//ǰ�ռ�
				case 17:	//��߼�
				case 18:	//��ͼ�
				case 19:	//���̼�
				case 20:	//�վ���
				case 21:	//�ɽ���
				case 22:	//�ɽ����
				case 23:	//������
				case 24:	//�ۼƳɽ���
				case 25:	//�ۼƳɽ����
				case 26:	//�ۼƻ�����
				case 27:	//��ͨ��ֵ
				case 28:	//������ֵ(A)
				case 284:   //������ֵ(B)
				case 279:   //������ֵ(A+B)
				case 29:	//����ֵ
				case 30:	//��������ӽ�ָ���۸�(<=)�ĵ�һ������
				case 31:	//��������ӽ�ָ����������(<=)�ĵ�һ������
				case 32:	//�׶��Ƿ�
				case 321:	//�׶��Ƿ�(������ʼ���ڵ����Ƿ�)
				case 322://�����������Ƿ�
				case 33:	//������
				case 34:	//������
				case 144:	//���5�����̼�
				case 145:	//���5�����̼�
				case 146:	//���5��������
				case 147:	//���5��������
				case 148:	//ծȯ���5�����̼۶�Ӧ�ĵ���������
				case 149:	//ծȯ���5�����̼۶�Ӧ�ĵ���������
				case 35:	//��ͨ�ɱ�
				case 36:	//�ܹɱ�
				case 37:	//���ڹɱ�(A)
				case 285:   //���ڹɱ�(B)
				case 278:   //���ڹɱ�(A+B)
				case 38:	//�ֹ�����
				case 39:	//���й�
				case 99:	//����ͨ��
				case 40:	//��Ҫ����ָ���
				case 41:	//�ʲ���ծ��
				case 42:	//��������
				case 43:	//�ֽ�������
				case 55:	//��Ҫ����ָ��������ָ��
				case 75:	//�����ʲ���ֵ
				case 112:	//��תծ��Ӧ��Ʊ����
				case 44:	//��Ӫҵ������ҵ��ֲ�����Ŀ������
				case 45:	//��Ӫҵ������ҵ��ֲ�����Ŀ����
				case 46:	//�ʲ���ֵ׼��������
				case 47:	//�Ǿ������������Ŀ����
				case 48:	//�Ǿ������������Ŀ���
				case 49:	//Ӧ���ʿ������
				case 50:	//Ӧ���ʿ������
				case 51:	//������ñ���Ŀ����
				case 52:	//������ñ���Ŀ���
				case 56:	//���̼�[��Ȩ]
				case 57:	//ǰ�ռ�[��Ȩ]
				case 58:	//��߼�[��Ȩ]
				case 59:	//��ͼ�[��Ȩ]
				case 60:	//���̼�[��Ȩ]
				case 61:	//�վ���[��Ȩ]
				case 100:	//����������������Сֵ�ĵ�һ������
				case 101:	//�����������������ֵ�ĵ�һ������
				case 102:	//�����ڼ۸���Сֵ�ĵ�һ������
				case 103:	//�����ڼ۸����ֵ�ĵ�һ������
				case 117:	//ָ������������
				case 118:	//��Ʊ��֤�����ҵָ������
				case 119:	//��Ʊ��������ҵָ������
				case 120:	//�����ھ���
				case 121:	//��������ȷֺ��ܶ�
				case 133:	//�����ڷֺ��ܶ�
				case 134:	//��ȷֺ��ܶ�
				case 135:	//�����������Ϣ�ܶ�
				case 136:	//��������Ϣ�ܶ�
				case 137:	//�����Ϣ�ܶ�
				case 62:	//Ȩ֤-����������
				case 63:	//Ȩ֤-�����
				case 64:	//Ȩ֤-�ܸ˱���
				case 65:	//Ȩ֤-��Ȩ�۸�
				case 66:	//Ȩ֤��Ȩ��ֹ����
				case 67:	//Ȩ֤-ʣ����
				case 68:	//Ȩ֤-���ɴ���
				case 69:	//Ȩ֤-��Ȩ����
				case 71:	//��������
				case 122:	//������
				case 72:	//���������
				case 73:	//����λ��ֵ
				case 74:	//�����ۼƵ�λ��ֵ
				case 76:	//�����г��������ÿ��ݻ�������
				case 77:	//�����������
				case 78:	//����ʣ������
				case 79:	//�����г���������7���껯������
				case 81:	//IOPV(ETF����LOF����)
				case 286:   //IOPV(ETF����LOF����)
				case 82:	//�����й���
				case 83:	//�����г�����������
				case 84:	//������������
				case 85:	//ծȯ����
				case 123:	//ծȯ��ʽ
				case 87:	//ծȯ��ֵ
				case 89:	//ծȯ��ϢƵ��
				case 86:	//ծȯ������
				case 88:	//ծȯƱ������
				case 90:	//ծȯ��������
				case 91:	//ծȯ��Ϣ����
				case 92:	//ծȯ��������
				case 93:	//ծȯ������Ϣ��
				case 94:	//ծȯ���ڸ�Ϣ��
				case 95:	//ծȯӦ����Ϣ
				case 96:	//ծȯ��Ϣ����
				case 97:	//ծȯʣ������
				case 113:	//��תծ-ת��ƽ��
				case 114:	//��תծ-ת�������
				case 115:	//��תծ-ת���׼�
				case 116:	//��תծ-�׼������
				case 229:	//���ڸ�ʽת��text2Date
				case 230:	//���ڸ�ʽת��Date2Text
				case 231:	//������뵽�г�����
				case 232:	//�ǵ�ֵ
				case 233:	//�ǵ���
				case 234:	//�����г�
				case 238:	//������
				case 243:	//֤���淶������
				case 242:	//ȡ�ùɱ���ϸ����
				case 249:   //ծȯ�׷��۸�
				case 250:   //ծȯ��׼ȯ������
				case 251:   //ծȯ���±�׼ȯ�����ʿ�ʼ������
				case 252:   //ծȯ���±�׼ȯ�����ʽ���������
				case 255:   //�����ڽ���
				case 264:   //�������ӷ��
				case 265:   //ծȯ����
				case 266:   //ծȯ��������
				case 267:   //ծȯ��������
				case 268:   //��ծ��������
				case 269:   //��ծ������������
				case 270:   //��ծ������������
				case 271:   //ծȯ���¶Ҹ�������
				case 272:   //ծȨ�Ǽ���
				case 273:   //ծȯ�Ҹ���ʼ��
				case 274:   //ծȯ��һ�θ�Ϣ�Ľ�ֹ����
				case 275:   //ÿ��Ԫ�ֽ���
				case 276:   //��Ϣ��
				case 277:   //����Ȩ��ֵ
				case 282:   //�ϲ���ҵ�ֲ�-֤���(��)
				case 283:   //�ϲ���ҵ�ֲ�ռ��-֤���(��)
				case 288:   //�ݶֵ��׼��
					//��ʼ��mapSheetReq(<strReq, strValue>)
					mapSheetReq.insert(StrPair(strReq, _T("")));
					mapSheetReqVariantData.insert(ReqVarPair(strReq,Tx::Core::VariantData(_T(""))));
					//-----------������ʲ���ծ���������������ҳ����еĹ�˾id�����Ƽ�
					switch( reqTXFunction.iFuncID )
					{
					case 41:
						FindItemOfReq(&reqTXFunction,strReq,1);		//��������ȷ�Ͻ���ʵ��
						FindItemOfReq(&reqTXFunction,strReq,12);	//ȷ�ϲƼ�
						nBalanceSecuritySet.insert( reqTXFunction.nSecurityID );
						nDate = reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter;
						nBalanceDateSet.insert( nDate );
						break;
					case 40:
						FindItemOfReq(&reqTXFunction,strReq,1);		//��������ȷ�Ͻ���ʵ��
						FindItemOfReq(&reqTXFunction,strReq,12);	//ȷ�ϲƼ�
						nFinancialSecuritySet.insert( reqTXFunction.nSecurityID );
						nDate = reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter;
						nFinancialDateSet.insert( nDate );
						break;
					case 42:
						FindItemOfReq(&reqTXFunction,strReq,1);		//��������ȷ�Ͻ���ʵ��
						FindItemOfReq(&reqTXFunction,strReq,12);	//ȷ�ϲƼ�
						nIncomeSecuritySet.insert( reqTXFunction.nSecurityID );
						nDate = reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter;
						nIncomeDateSet.insert( nDate );
						break;
					case 43:
						FindItemOfReq(&reqTXFunction,strReq,1);		//��������ȷ�Ͻ���ʵ��
						FindItemOfReq(&reqTXFunction,strReq,12);	//ȷ�ϲƼ�
						nCashflowSecuritySet.insert( reqTXFunction.nSecurityID );
						nDate = reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter;
						nCashflowDateSet.insert( nDate );
						break;
					case 44:
					case 45:
						FindItemOfReq(&reqTXFunction,strReq,1);		//��������ȷ�Ͻ���ʵ��
						FindItemOfReq(&reqTXFunction,strReq,12);	//ȷ�ϲƼ�
						nRevenueSecuritySet.insert( reqTXFunction.nSecurityID );
						nDate = reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter;
						nRevenueDateSet.insert( nDate );
						break;
					case 46:
						FindItemOfReq(&reqTXFunction,strReq,1);		//��������ȷ�Ͻ���ʵ��
						FindItemOfReq(&reqTXFunction,strReq,12);	//ȷ�ϲƼ�
						nDepreciationSecuritySet.insert( reqTXFunction.nSecurityID );
						nDate = reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter;
						nDepreciationDateSet.insert( nDate );
					case 47:
					case 48:
						FindItemOfReq(&reqTXFunction,strReq,1);		//��������ȷ�Ͻ���ʵ��
						FindItemOfReq(&reqTXFunction,strReq,12);	//ȷ�ϲƼ�
						nGainsAndLossesSecuritySet.insert( reqTXFunction.nSecurityID );
						nDate = reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter;
						nGainsAndLossesDateSet.insert( nDate );
						break;
					case 49:
					case 50:
						FindItemOfReq(&reqTXFunction,strReq,1);		//��������ȷ�Ͻ���ʵ��
						FindItemOfReq(&reqTXFunction,strReq,12);	//ȷ�ϲƼ�
						nAccountReceivableSecuritySet.insert( reqTXFunction.nSecurityID );
						nDate = reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter;
						nAccountReceivableDateSet.insert( nDate );
						break;
					case 51:
					case 52:
						FindItemOfReq(&reqTXFunction,strReq,1);		//��������ȷ�Ͻ���ʵ��
						FindItemOfReq(&reqTXFunction,strReq,12);	//ȷ�ϲƼ�
						nFinanceChargeSecuritySet.insert( reqTXFunction.nSecurityID );
						nDate = reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter;
						nFinancialDateSet.insert( nDate );
						break;
					case 15:	//���̼�
					case 16:	//ǰ�ռ�
					case 17:	//��߼�
					case 18:	//��ͼ�
					case 19:	//���̼�
					case 20:	//�վ���
					case 21:	//�ɽ���
					case 22:	//�ɽ����
					case 23:	//������
					case 232:
					case 233:
						FindItemOfReq(&reqTXFunction,strReq,1);
						FindItemOfReq(&reqTXFunction,strReq,3);
						nHQSecuritySet.insert( reqTXFunction.nSecurityID );
						nHQDateSet.insert( reqTXFunction.nDate );
						break;
					case 56:	//���̼�[��Ȩ]
					case 57:	//ǰ�ռ�[��Ȩ]
					case 58:	//��߼�[��Ȩ]
					case 59:	//��ͼ�[��Ȩ]
					case 60:	//���̼�[��Ȩ]
					case 61:	//�վ���[��Ȩ]
						FindItemOfReq(&reqTXFunction,strReq,1);
						FindItemOfReq(&reqTXFunction,strReq,9);
						nHQSecuritySet.insert( reqTXFunction.nSecurityID );
						nHQDateSet.insert( reqTXFunction.nDate );
						break;
					case 73:
						FindItemOfReq(&reqTXFunction,strReq,1);
						FindItemOfReq(&reqTXFunction,strReq,3);
						if ( reqTXFunction.iSubItemID)
						{
							nSetNvrSecurityExt.insert( reqTXFunction.nSecurityID );
							nSetNvrDateExt.insert( reqTXFunction.nDate );	
						}
						else
						{
							nSetNvrSecurity.insert( reqTXFunction.nSecurityID );
							nSetNvrDate.insert( reqTXFunction.nDate );	
						}
						break;
					case 76:
						FindItemOfReq(&reqTXFunction,strReq,1);
						FindItemOfReq(&reqTXFunction,strReq,3);
						if ( reqTXFunction.iSubItemID)
						{
							nSetMmfNvrSecurityExt.insert( reqTXFunction.nSecurityID );
							nSetMmfNvrDateExt.insert( reqTXFunction.nDate );	
						}
						else
						{
							nSetMmfNvrSecurity.insert( reqTXFunction.nSecurityID );
							nSetMmfNvrDate.insert( reqTXFunction.nDate );	
						}
						break;
					case 74:
						FindItemOfReq(&reqTXFunction,strReq,1);
						FindItemOfReq(&reqTXFunction,strReq,3);
						if ( reqTXFunction.iSubItemID )
						{
							nSetAcuNvrSecurityExt.insert( reqTXFunction.nSecurityID );
							nSetAcuNvrDateExt.insert( reqTXFunction.nDate );
						}
						else
						{
							nSetAcuNvrSecurity.insert( reqTXFunction.nSecurityID );
							nSetAcuNvrDate.insert( reqTXFunction.nDate );
						}
						break;
					case 79:
						FindItemOfReq(&reqTXFunction,strReq,1);
						FindItemOfReq(&reqTXFunction,strReq,3);
						if ( reqTXFunction.iSubItemID)
						{
							nSetMmfAcuNvrSecurityExt.insert( reqTXFunction.nSecurityID );
							nSetMmfAcuNvrDateExt.insert( reqTXFunction.nDate );	
						}
						else
						{
							nSetMmfAcuNvrSecurity.insert( reqTXFunction.nSecurityID );
							nSetMmfAcuNvrDate.insert( reqTXFunction.nDate );	
						}			
						break;
					case 75:
						FindItemOfReq(&reqTXFunction,strReq,1);
						FindItemOfReq(&reqTXFunction,strReq,3);
						nSetNvrSecurity.insert( reqTXFunction.nSecurityID );
						nSetNvrDate.insert( reqTXFunction.nDate );	
						break;
					}
					break;
				case 9:		//��Ʊ���м۸�
				case 70:	//Ȩ֤-����ʣ������
				case 106:	//��תծת�ɼ۸�
				case 107:	//��תծ����
				case 109:	//��תծת������
					//��ʼ��mapSheetReq(<strReq, strValue>)
					mapSheetReq.insert(StrPair(strReq, _T("")));
					mapSheetReqVariantData.insert(ReqVarPair(strReq,Tx::Core::VariantData(_T(""))));
					//�������������
					FindItemOfReq(&reqTXFunction, strReq, 16);	//Security��Date
					indDate.nRow = -1;
					indDate.dValue = 0;
					indDate.nSecurity = reqTXFunction.nSecurityID;
					indDate.nDate = reqTXFunction.nDate;
					switch(reqTXFunction.iFuncID)
					{
					case 9:
						vStockIssue.push_back(indDate);
						break;
					case 70:
						vWarrantDV.push_back(indDate);
						break;
					case 106:
						vCBondPrice.push_back(indDate);
						break;
					case 107:
						vCBondInterest.push_back(indDate);
						break;
					case 109:
						vCBondAmount.push_back(indDate);
						break;
					default:
						break;
					}
					break;
				case 80:	//����ֵ������
					//��ʼ��mapSheetReq(<strReq, strValue>)
					mapSheetReq.insert(StrPair(strReq, _T("")));
					mapSheetReqVariantData.insert(ReqVarPair(strReq,Tx::Core::VariantData(_T(""))));
					//�������������
					FindItemOfReq(&reqTXFunction, strReq, 15);	//Security��StartDate��EndDate
					m_nNvrSample.insert(reqTXFunction.nSecurityID);
					m_nNvrDates.insert(std::pair<int,int>(reqTXFunction.nStartDate,reqTXFunction.nEndDate));
					//indDates.nRow = -1;
					//indDates.dValue = 0;
					//indDates.nSecurity = reqTXFunction.nSecurityID;
					//indDates.nFirstDate = reqTXFunction.nStartDate;
					//indDates.nSecondDate = reqTXFunction.nEndDate;
					//vFundNAVGrowth.push_back(indDates);
					break;
				case 98:	//ծȯYTM
				case 104:	//ծȯ��������
				case 105:	//ծȯ͹��
					{
						mapSheetReq.insert(StrPair(strReq, _T("")));
						mapSheetReqVariantData.insert(ReqVarPair(strReq,Tx::Core::VariantData(_T(""))));
						CTime time = CTime::GetCurrentTime();
						int iDate = time.GetYear()*10000 + time.GetMonth()*100 + time.GetDay();
						int iTime = time.GetHour()*100 + time.GetMinute();
						if (iDate == reqTXFunction.nDate)
							bReqYtm = true;
					}
					break;
				case 110:	//��תծ��������
					//��ʼ��mapSheetReq(<strReq, strValue>)
					mapSheetReq.insert(StrPair(strReq, _T("")));
					mapSheetReqVariantData.insert(ReqVarPair(strReq,Tx::Core::VariantData(_T(""))));
					//�������������
					FindItemOfReq(&reqTXFunction, strReq, 1);	//Security
					indDate.nRow = -1;
					indDate.dValue = 0;
					indDate.nDate = 0;
					indDate.nSecurity = reqTXFunction.nSecurityID;
					vBondIPOInfo.push_back(indDate);
					break;
				case 108:	//��תծ-����
					//��ʼ��mapSheetReq(<strReq, strValue>)
					mapSheetReq.insert(StrPair(strReq, _T("")));
					mapSheetReqVariantData.insert(ReqVarPair(strReq,Tx::Core::VariantData(_T(""))));
					if(reqTXFunction.iSubItemID == 1)
					{	//������
						//�������������
						FindItemOfReq(&reqTXFunction, strReq, 1);	//Security
						indDate.nRow = -1;
						indDate.dValue = 0;
						indDate.nDate = 0;
						indDate.nSecurity = reqTXFunction.nSecurityID;
						vBondIPOInfo.push_back(indDate);
					}
					break;
				case 111:	//��תծ������Ϣ
					//��ʼ��mapSheetReq(<strReq, strValue>)
					mapSheetReq.insert(StrPair(strReq, _T("")));
					mapSheetReqVariantData.insert(ReqVarPair(strReq,Tx::Core::VariantData(_T(""))));
					if(reqTXFunction.iSubItemID != 2)
					{	//������id֮���ָ��
						//�������������
						FindItemOfReq(&reqTXFunction, strReq, 1);	//Security
						indDate.nRow = -1;
						indDate.dValue = 0;
						indDate.nDate = 0;
						indDate.nSecurity = reqTXFunction.nSecurityID;
						vBondIPOInfo.push_back(indDate);
					}
					break;
				case 124:	//ʮ��ɶ�����
				case 125:	//ʮ��ɶ��ֹ���
				case 126:	//ʮ��ɶ��ֹɱ���
				case 127:	//ʮ��ɶ��ֹ�����
				case 128:	//ʮ����ͨ�ɶ�����
				case 129:	//ʮ����ͨ�ɶ��ֹ���
				case 130:	//ʮ����ͨ�ɶ��ܹɱ���
				case 131:	//ʮ����ͨ�ɶ���ͨ����
				case 132:	//ʮ����ͨ�ɶ��ֹ�����
				case 138:	//���ʽ����ʮ�����������
				case 139:	//���ʽ����ʮ������˻���ݶ�
				case 140:	//���ʽ����ʮ������˻���ݶ����
				case 141:	//��תծʮ�����������
				case 142:	//��תծʮ�������ծȯ���
				case 143:	//��תծʮ�������ծȯ������
					//��ʼ��mapSheetReq(<strReq, strValue>)
					mapSheetReq.insert(StrPair(strReq, _T("")));
					mapSheetReqVariantData.insert(ReqVarPair(strReq,Tx::Core::VariantData(_T(""))));
					FindItemOfReq(&reqTXFunction, strReq, 19);	//Security��Date��iHolderNo
					indShareHolder.nRow = -1;
					indShareHolder.iHolerNo = reqTXFunction.iHolderNo;
					indShareHolder.nDate = reqTXFunction.nDate;
					indShareHolder.nSecurity = (INT)(reqTXFunction.nSecurityID);
					switch(reqTXFunction.iFuncID)
					{
					case 124:	//ʮ��ɶ�����
					case 125:	//ʮ��ɶ��ֹ���
					case 126:	//ʮ��ɶ��ֹɱ���
					case 127:	//ʮ��ɶ��ֹ�����
						vTopTenShareHolder.push_back(indShareHolder);
						break;
					case 128:	//ʮ����ͨ�ɶ�����
					case 129:	//ʮ����ͨ�ɶ��ֹ���
					case 130:	//ʮ����ͨ�ɶ��ܹɱ���
					case 131:	//ʮ����ͨ�ɶ���ͨ����
					case 132:	//ʮ����ͨ�ɶ��ֹ�����
						vTopTenCShareHolder.push_back(indShareHolder);
						break;
					case 138:	//���ʽ����ʮ�����������
					case 139:	//���ʽ����ʮ������˻���ݶ�
					case 140:	//���ʽ����ʮ������˻���ݶ����
						vTopTenFundHolder.push_back(indShareHolder);
						break;
					default:	//��תծʮ����������ơ�ծȯ��ծȯ������
						vTopTenCBondHolder.push_back(indShareHolder);
						break;
					}
					break;
					//--wangzhy--20080710--added
				case 151:
				case 152:
				case 153:
					mapSheetReq.insert(StrPair(strReq, _T("")));
					mapSheetReqVariantData.insert(ReqVarPair(strReq,Tx::Core::VariantData(_T(""))));
					break;
				//-----zhangxs---20080813---
				case 154:		//�����������Ϣ
					mapSheetReq.insert(StrPair(strReq, _T("")));
					mapSheetReqVariantData.insert(ReqVarPair(strReq,Tx::Core::VariantData(_T(""))));
					//nFundIPOInfoSet.insert(reqTXFunction.nSecurityID);
					nDate = reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter;
					//nFundIPOInfoDateSet.insert( nDate );
					break;
				case 157:		//Ͷ�����
					mapSheetReq.insert(StrPair(strReq, _T("")));
					mapSheetReqVariantData.insert(ReqVarPair(strReq,Tx::Core::VariantData(_T(""))));
					nFundInvesmentGroupSet.insert(reqTXFunction.nSecurityID);
					nDate = reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter;
					nFundInvesmentGroupSet.insert( nDate );
					break;
				case 160:	//�����������ҵ�ֲ�
				case 161:	//����ָ������ҵ�ֲ�
				case 162:	//����ϲ�����ҵ�ֲ�
				case 189:
					mapSheetReq.insert(StrPair(strReq, _T("")));
					mapSheetReqVariantData.insert(ReqVarPair(strReq,Tx::Core::VariantData(_T(""))));
					switch( reqTXFunction.iFuncID )
					{
					case 160:
						FindItemOfReq(&reqTXFunction,strReq,24);
						nFundCombineInvestmentIndustryDistributeSet.insert(reqTXFunction.nSecurityID);
						nDate = reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter;
						nFundCombineInvestmentIndustryDistributeDateSet.insert( nDate );
						break;
					case 161:
						FindItemOfReq(&reqTXFunction,strReq,24);
						nFundCombineInvestmentIndustryDistributeSet.insert(reqTXFunction.nSecurityID);
						nDate = reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter;
						nFundCombineInvestmentIndustryDistributeDateSet.insert( nDate );
						break;
					case 162:
						FindItemOfReq(&reqTXFunction,strReq,24);
						nFundCombineInvestmentIndustryDistributeSet.insert(reqTXFunction.nSecurityID);
						nDate = reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter;
						nFundCombineInvestmentIndustryDistributeDateSet.insert( nDate );
						break;
					case 189:
						FindItemOfReq(&reqTXFunction,strReq,24);
						nFundTXInvestmentIndustryDistributeSet.insert(reqTXFunction.nSecurityID); 
						nDate = reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter;
						nFundTXInvestmentIndustryDistributeDateSet.insert( nDate );  
						break;
					}
					break;
				case 247:       //������ҵ��ֵռ����ֵ��   2012-3-23
					mapSheetReq.insert(StrPair(strReq,_T("")));
					mapSheetReqVariantData.insert(ReqVarPair(strReq,Tx::Core::VariantData(_T())));
					FindItemOfReq(&reqTXFunction,strReq,24);
					nFundTXInvestmentIndustryDistributeSet.insert(reqTXFunction.nSecurityID); 
					nDate = reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter;
					nFundTXInvestmentIndustryDistributeDateSet.insert( nDate );  
					break;
				case 248:       //�ϲ���ҵ��ֵռ����ֵ��   2012-3-23
					mapSheetReq.insert(StrPair(strReq,_T("")));
					mapSheetReqVariantData.insert(ReqVarPair(strReq,Tx::Core::VariantData(_T())));
					FindItemOfReq(&reqTXFunction,strReq,24);
					nFundCombineInvestmentIndustryDistributeSet.insert(reqTXFunction.nSecurityID);
					nDate = reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter;
					nFundCombineInvestmentIndustryDistributeDateSet.insert( nDate );
					break;
				case 163:
				case 164:
				case 165:
				case 172:
				case 173:
				case 174:
				case 175:
				case 176:
				case 177:
				case 178:
				case 179:
				case 180:
				case 181:
				case 182:
				case 183:
					mapSheetReq.insert(StrPair(strReq, _T("")));
					mapSheetReqVariantData.insert(ReqVarPair(strReq,Tx::Core::VariantData(_T(""))));
					switch( reqTXFunction.iFuncID )
					{
					case 163:	//������Ͷ���زֹ�
					case 164:
					case 165:
					case 172:
					case 173:
						FindItemOfReq(&reqTXFunction,strReq,18);
						nFundStockVIPStockeSet.insert(reqTXFunction.nSecurityID);
						nDate = reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter;
						nFundStockVIPStockDateSet.insert( nDate );
						FindItemOfReq(&reqTXFunction,strReq,18);
						nFundCombineVIPStockSet.insert(reqTXFunction.nSecurityID);
						nDate = reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter;
						nFundCombineVIPStockDateSet.insert( nDate );
						break;
					case 174:	//ָ����Ͷ���زֹ�
					case 175:
					case 176:
					case 177:
					case 178:
						FindItemOfReq(&reqTXFunction,strReq,24);
						nFundIndexVIPStockSet.insert(reqTXFunction.nSecurityID);
						nDate = reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter;
						nFundIndexVIPStockDateSet.insert( nDate );
						break;
					case 179:	//�ϲ���Ͷ���زֹ�
					case 180:
					case 181:
					case 182:
					case 183:
						FindItemOfReq(&reqTXFunction,strReq,18);
						nFundCombineVIPStockSet.insert(reqTXFunction.nSecurityID);
						nDate = reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter;
						nFundCombineVIPStockDateSet.insert( nDate );
						break;
					}
					break;
				case 167:
				case 168:
				case 169:
				case 170:
				case 171:
					mapSheetReq.insert(StrPair(strReq, _T("")));
					mapSheetReqVariantData.insert(ReqVarPair(strReq,Tx::Core::VariantData(_T(""))));
					switch(reqTXFunction.iFuncID)
					{
					case 167:
						FindItemOfReq(&reqTXFunction,strReq,24);
						nFundNavChangeSet.insert(reqTXFunction.nSecurityID);
						nDate = reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter;
						nFundNavChangeDateSet.insert( nDate );
						break;
					case 168:
						FindItemOfReq(&reqTXFunction,strReq,24);
						nFundFinancialSecuritySet.insert(reqTXFunction.nSecurityID);
						nDate = reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter;
						nFundFinancialDateSet.insert( nDate );
						break;
					case 169:
						FindItemOfReq(&reqTXFunction,strReq,24);
						nFundBalanceSecuritySet.insert(reqTXFunction.nSecurityID);
						nDate = reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter;
						nFundBalanceDateSet.insert( nDate );
						break;
					case 170:
						FindItemOfReq(&reqTXFunction,strReq,24);
						nFundAchievementSet.insert(reqTXFunction.nSecurityID);
						nDate = reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter;
						nFundAchievementDateSet.insert( nDate );
						break;
					case 171:
						FindItemOfReq(&reqTXFunction,strReq,24);
						nFundRevenueSet.insert(reqTXFunction.nSecurityID);
						nDate = reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter;
						nFundRevenueDateSet.insert( nDate );
						break;
					}
					break;
				case 192:		//ծȯ���
				case 193:		//��ծ��ϸ
				case 194:		//�ֿ�תծ��ϸ1
				case 195:		//�����ʲ�
					mapSheetReq.insert(StrPair(strReq, _T("")));
					mapSheetReqVariantData.insert(ReqVarPair(strReq,Tx::Core::VariantData(_T(""))));
					FindItemOfReq(&reqTXFunction,strReq,24);
					switch(reqTXFunction.iFuncID)
					{
					case 192:
						nFundBondPortfolio.insert( reqTXFunction.nSecurityID);
						nFundBondPortfolioDateSet.insert( reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter );
						break;
					case 193:
						nFundBondHoldDetail.insert(reqTXFunction.nSecurityID);
						nFundBondHoldDetailDateSet.insert( reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter );
						break;
					case 194:
						nFundCBondHoldDetail.insert( reqTXFunction.nSecurityID);
						nFundCBondHoldDetailDateSet.insert( reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter );
						break;
					case 195:
						nFundOtherAsset.insert( reqTXFunction.nSecurityID);;				
						nFundOtherAssetDateSet.insert( reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter );
					}
					break;
				case 198:
				case 206:
				case 244:
				case 245:
					mapSheetReq.insert(StrPair(strReq, _T("")));
					mapSheetReqVariantData.insert(ReqVarPair(strReq,Tx::Core::VariantData(_T(""))));
					break;
				case 199:
					mapSheetReq.insert(StrPair(strReq, _T("")));
					mapSheetReqVariantData.insert(ReqVarPair(strReq,Tx::Core::VariantData(_T(""))));
					{
						nIncomeSecuritySet.insert( reqTXFunction.nSecurityID );
						nDate = reqTXFunction.nFYear*10 + 9;
						nIncomeDateSet.insert( nDate );
					}
					break;
				case 200:
				case 201:
				case 202:
				case 207:
					mapSheetReq.insert(StrPair(strReq, _T("")));
					mapSheetReqVariantData.insert(ReqVarPair(strReq,Tx::Core::VariantData(_T(""))));
					FindItemOfReq(&reqTXFunction,strReq,16);
					{
						nFlowOfCapitalSet.insert( reqTXFunction.nSecurityID );
						nFlowOfCapitalDateSet.insert( reqTXFunction.nDate );
					}
					break;
				case 203:
				case 204:
				case 205:
				case 208:
					mapSheetReq.insert(StrPair(strReq, _T("")));
					mapSheetReqVariantData.insert(ReqVarPair(strReq,Tx::Core::VariantData(_T(""))));
					FindItemOfReq(&reqTXFunction,strReq,15);
					break;
				case 185:
				case 186:
				case 187:
				case 188:
					mapSheetReq.insert(StrPair(strReq, _T("")));
					mapSheetReqVariantData.insert(ReqVarPair(strReq,Tx::Core::VariantData(_T(""))));
					FindItemOfReq(&reqTXFunction,strReq,16);
					nPEPBSet.insert(reqTXFunction.nSecurityID);
					nPEPBDateSet.insert(reqTXFunction.nDate);
					break;
				case 212:
				case 213:
				case 214:
				case 215:
				case 216:
				case 217:
				case 287:
					mapSheetReq.insert(StrPair(strReq, _T("")));
					mapSheetReqVariantData.insert(ReqVarPair(strReq,Tx::Core::VariantData(_T(""))));
					FindItemOfReq( &reqTXFunction, strReq,2);
					FindItemOfReq(&reqTXFunction,strReq,26);
					break;
				case 220:
				case 221:
				case 222:
				case 223:
				case 224:
				case 225:
				case 226:
				case 227:
				case 228:
					mapSheetReq.insert(StrPair(strReq, _T("")));
					mapSheetReqVariantData.insert(ReqVarPair(strReq,Tx::Core::VariantData(_T(""))));
					//-----------������ʲ���ծ���������������ҳ����еĹ�˾id�����Ƽ�
					switch( reqTXFunction.iFuncID )
					{
					case 220:
						FindItemOfReq(&reqTXFunction,strReq,1);		//��������ȷ�Ͻ���ʵ��
						FindItemOfReq(&reqTXFunction, strReq, 12);	//ȷ�ϲƼ�
						nBalanceYHSet.insert( reqTXFunction.nSecurityID );
						nDate = reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter;
						nBalanceYHDateSet.insert( nDate );
						break;
					case 221:
						FindItemOfReq(&reqTXFunction,strReq,1);		//��������ȷ�Ͻ���ʵ��
						FindItemOfReq(&reqTXFunction, strReq, 12);	//ȷ�ϲƼ�
						nBalanceBXSet.insert( reqTXFunction.nSecurityID );
						nDate = reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter;
						nBalanceBXDateSet.insert( nDate );
						break;
					case 222:
						FindItemOfReq(&reqTXFunction,strReq,1);		//��������ȷ�Ͻ���ʵ��
						FindItemOfReq(&reqTXFunction, strReq, 12);	//ȷ�ϲƼ�
						nBalanceZQSet.insert( reqTXFunction.nSecurityID );
						nDate = reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter;
						nBalanceZQDateSet.insert( nDate );
						break;
					case 223:
						FindItemOfReq(&reqTXFunction,strReq,1);		//��������ȷ�Ͻ���ʵ��
						FindItemOfReq(&reqTXFunction, strReq, 12);	//ȷ�ϲƼ�
						nProfitYHSet.insert( reqTXFunction.nSecurityID );
						nDate = reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter;
						nProfitYHDateSet.insert( nDate );
						break;
					case 224:
						FindItemOfReq(&reqTXFunction,strReq,1);		//��������ȷ�Ͻ���ʵ��
						FindItemOfReq(&reqTXFunction, strReq, 12);	//ȷ�ϲƼ�
						nProfitBXSet.insert( reqTXFunction.nSecurityID );
						nDate = reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter;
						nProfitBXDateSet.insert( nDate );
						break;
					case 225:
						FindItemOfReq(&reqTXFunction,strReq,1);		//��������ȷ�Ͻ���ʵ��
						FindItemOfReq(&reqTXFunction, strReq, 12);	//ȷ�ϲƼ�
						nProfitZQSet.insert( reqTXFunction.nSecurityID );
						nDate = reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter;
						nProfitZQDateSet.insert( nDate );
						break;
					case 226:
						FindItemOfReq(&reqTXFunction,strReq,1);		//��������ȷ�Ͻ���ʵ��
						FindItemOfReq(&reqTXFunction, strReq, 12);	//ȷ�ϲƼ�
						nCashFlowYHSet.insert( reqTXFunction.nSecurityID );
						nDate = reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter;
						nCashFlowYHDateSet.insert( nDate );
						break;
					case 227:
						FindItemOfReq(&reqTXFunction,strReq,1);		//��������ȷ�Ͻ���ʵ��
						FindItemOfReq(&reqTXFunction, strReq, 12);	////ȷ�ϲƼ�
						nCashFlowBXSet.insert( reqTXFunction.nSecurityID );
						nDate = reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter;
						nCashFlowBXDateSet.insert( nDate );
						break;
					case 228:
						FindItemOfReq(&reqTXFunction,strReq,1);		//��������ȷ�Ͻ���ʵ��
						FindItemOfReq(&reqTXFunction, strReq, 12);	///ȷ�ϲƼ�
						nCashFlowZQSet.insert( reqTXFunction.nSecurityID );
						nDate = reqTXFunction.nFYear*10 + reqTXFunction.nFQuarter;
						nCashFlowZQDateSet.insert( nDate );
						break;					
					}
					break;
				case 237:
					mapSheetReq.insert(StrPair(strReq, _T("")));
					mapSheetReqVariantData.insert(ReqVarPair(strReq,Tx::Core::VariantData(_T(""))));
					FindItemOfReq(&reqTXFunction,strReq,2);
					FindItemOfReq(&reqTXFunction,strReq,3);
					//nFundNavChangeSet.insert(reqTXFunction.nSecurityID);
					nDate = reqTXFunction.nDate;
					//nFundNavChangeDateSet.insert( nDate );
					break;

				//-------------------------------------------------------------------------------------------
				default:
					mapSheetReq.insert(StrPair(strReq, _T("")));
					mapSheetReqVariantData.insert(ReqVarPair(strReq,Tx::Core::VariantData(_T(""))));
					break;
				}
			}
			psetReqCalc->clear();
			singleLock.Unlock();

			//�����Ʊ���м۸�
			if(vStockIssue.size() > 0)
			{
				SetDateIndicator(&vStockIssue, 0);
			}

			//����Ȩ֤ʣ�ഴ�����
			if(vWarrantDV.size() > 0)
			{
				SetDateIndicator(&vWarrantDV, 1);
			}

			//�����תծת�ɼ۸�
			if(vCBondPrice.size() > 0)
			{
				SetDateIndicator(&vCBondPrice, 2);
			}

			//�����תծ����
			if(vCBondInterest.size() > 0)
			{
				SetDateIndicator(&vCBondInterest, 3);
			}

			//�����תծת������
			if(vCBondAmount.size() > 0)
			{
				SetDateIndicator(&vCBondAmount, 4);
			}

			//����ծȯ������Ϣ
			if(vBondIPOInfo.size() > 0)
			{
				SetDateIndicator(&vBondIPOInfo, 5);
			}

			//�������ֵ������
			if(vFundNAVGrowth.size() > 0)
			{
				SetDatesIndicator(&vFundNAVGrowth, 0);
			}

			//�µĻ���ֵ������
			if ( m_nNvrSample.size() > 0 && m_nNvrDates.size() > 0 )
			{
				Tx::Business::FundDeriveData fundNvr;
				std::vector<int> vecSamples;
				vecSamples.assign( m_nNvrSample.begin(),m_nNvrSample.end());
				int n = 1;
				for ( set<pair<int,int>>::iterator iter = m_nNvrDates.begin(); iter != m_nNvrDates.end();++iter )
				{
					m_d2cMap.insert(pair<pair<int,int>,int>(*iter,n++));
				}
				fundNvr.CalcFundNvr( m_nNvrTable,vecSamples,m_nNvrDates );
			}
			//-----------------------
			GetFundNvr( nSetNvrSecurity,nSetNvrDate);
			GetFundAcuNvr(nSetAcuNvrSecurity,nSetAcuNvrDate);
			//-----------------------
			GetMmfFundNvr( nSetMmfNvrSecurity,nSetMmfNvrDate);
			GetMmfFundAcuNvr(nSetMmfAcuNvrSecurity,nSetMmfAcuNvrDate);
			//-----------------------
			GetFundNvrExt( nSetNvrSecurityExt,nSetNvrDateExt);
			GetFundAcuNvrExt(nSetAcuNvrSecurityExt,nSetAcuNvrDateExt);
			//-----------------------
			GetMmfFundNvrExt( nSetMmfNvrSecurityExt,nSetMmfNvrDateExt);
			GetMmfFundAcuNvrExt(nSetMmfAcuNvrSecurityExt,nSetMmfAcuNvrDateExt);

			if (bReqYtm)
			{
				GetBondYTM();
				bReqYtm = false;
			}

			//-------������ڲ�����������������������ʹ�����ֲ������ݼ���ģʽ
			//���չ�˾������ݴ�С��ͨ����СΪ70*8*2*10*4=44800�ֽڣ�
			//���ղƼ���֣�ͨ����СΪ1.6M
			
			if ( nBalanceSecuritySet.size() > 100 )
				bBalanceInstitution = FALSE;
			else 
				bBalanceInstitution = TRUE;
			if ( nFinancialSecuritySet.size() > 100 )
				bFinancialInstitution = FALSE;
			else
				bFinancialInstitution = TRUE;
			if ( nCashflowSecuritySet.size() > 100 )
				bCashflowInstitution = FALSE;
			else
				bCashflowInstitution = TRUE;
			if ( nIncomeSecuritySet.size() > 100 )
				bIncomeInstitution = FALSE;
			else
				bIncomeInstitution = TRUE;
			if( nRevenueSecuritySet.size() > 100)
				bRevenueInstitution = FALSE;
			else
				bRevenueInstitution = TRUE;
			if( nAccountReceivableSecuritySet.size() > 100 )
				bAccountInstitution = FALSE;
			else
				bAccountInstitution = TRUE;
			if( nGainsAndLossesSecuritySet.size() > 100 )
				bGainsAndLossesInstitution = FALSE;
			else 
				bGainsAndLossesInstitution = TRUE;
			if ( nDepreciationSecuritySet.size() > 100 )
				bDepreciationInstitution = FALSE;
			else
				bDepreciationInstitution = TRUE;
			if ( nFinanceChargeSecuritySet.size() > 100 )
				bFinanceChargeInstitution = FALSE;
			else
				bFinanceChargeInstitution = TRUE;
			if ( nHQSecuritySet.size() > 100 )
				bHQSecurity = FALSE;
			else
				bHQSecurity = TRUE;
			//----------------zhangxs--20080813-------------------
			if ( nFundFinancialSecuritySet.size() > 100 )
				bFundFinancialInstitution = FALSE;
			else
				bFundFinancialInstitution = TRUE;	
			if ( nFundBalanceSecuritySet.size() > 100 )
				bFundBalanceInstitution = FALSE;
			else
				bFundBalanceInstitution = TRUE;
			if ( nFundAchievementSet.size() > 100 )
				bFundAchievementInstitution = FALSE;
			else
				bFundAchievementInstitution = TRUE;
			if ( nFundRevenueSet.size() > 100 )
				bFundRevenueInstitution = FALSE;
			else
				bFundRevenueInstitution = TRUE;
			if ( nFundNavChangeSet.size() > 100 )
				bFundNavChangeInstitution = FALSE;
			else
				bFundNavChangeInstitution =TRUE;

			if ( nFundCombineInvestmentIndustryDistributeSet.size() > 100 )
				bFundCombineIndustryDistribute = FALSE;
			else
				bFundCombineIndustryDistribute = TRUE;

			if ( nFundStockInvestmentIndustryDistributeSet.size() > 100 )
				bFundStockIndustryDistribute = FALSE;
			else
				bFundStockIndustryDistribute = TRUE;
			if ( nFundIndexInvestmentIndustryDistributeSet.size() > 100 )
				bFundIndexIndustryDistribute = FALSE;
			else
				bFundIndexIndustryDistribute = TRUE;
			if ( nFundCombineVIPStockSet.size() > 100 )
				bFundCombineVIPStockInstitution = FALSE;
			else
				bFundCombineVIPStockInstitution = TRUE;
			if ( nFundStockVIPStockeSet.size() > 100 )
				bFundStockVIPStockInstitution = FALSE;
			else
				bFundStockVIPStockInstitution = TRUE;
			if ( nFundIndexVIPStockSet.size() > 100 )
				bFundIndexVIPStockInstitution = FALSE;
			else 
				bFundIndexVIPStockInstitution = TRUE;
			if ( nFundStockVIPStockeSet.size() > 100 )
				bFundStockVIPStockInstitution = FALSE;
			else
				bFundStockVIPStockInstitution = TRUE;
			if ( nFundHoldStockDetailSet.size() > 100 )
				bFundHolderStockDetail = FALSE;
			else
				bFundHolderStockDetail = TRUE;
			if ( nFundPositionInfoSet.size() > 100 )
				bFundPositionInfo = FALSE;
			else
				bFundPositionInfo = TRUE;
			if ( nFundShareDataChangeSet.size() > 100 )
				bFundShareDataChange = FALSE;
			else
				bFundShareDataChange = TRUE;
			if ( nFundBuyTotStockSet.size() > 100 )
				bFundBuyTotStock = FALSE;
			else
				bFundBuyTotStock = TRUE;
			if ( nFundSaleTotStockSet.size() > 100 )
				bFundSaleTotStock = FALSE;
			else
				bFundSaleTotStock = TRUE;

			if ( nFundTradeVolumeInfoSet.size() > 100 )
				bFundTradeVolumeInfo = FALSE;
			else
				bFundTradeVolumeInfo = TRUE;

			if (nFundTXInvestmentIndustryDistributeDateSet.size() > 100 )
				bFundTXIndustryDistribute = FALSE;
			else
				bFundTXIndustryDistribute = TRUE;
			//-------------wangzhy------------------------
			if (nFundBondPortfolioDateSet.size() > 100 )
				bFundBondPortfolio = FALSE;
			else
				bFundBondPortfolio = TRUE;

			if (nFundBondHoldDetailDateSet.size() > 100 )
				bFundBondHoldDetail = FALSE;
			else
				bFundBondHoldDetail = TRUE;

			if (nFundCBondHoldDetailDateSet.size() > 100 )
				bFundCBondHoldDetail = FALSE;
			else
				bFundCBondHoldDetail = TRUE;

			if (nFundOtherAssetDateSet.size() > 100 )
				bnFundOtherAsset = FALSE;
			else
				bnFundOtherAsset = TRUE;

			if ( nFlowOfCapitalSet.size() > 100 )
				bFlowOfCapital = FALSE;
			else
				bFlowOfCapital = TRUE;

			if ( nPEPBSet.size() > 100 )
				bPePb = FALSE;
			else
				bPePb = TRUE;

			if ( nBalanceStat.size() > 100 )
				bBalanceStat = FALSE;
			else
				bBalanceStat = TRUE;

			if( nProfitStat.size() > 100 )
				bProfitStat = FALSE;
			else
				bProfitStat = TRUE;

			if ( nCashFlowStat.size() > 100 )
				bCashFlowStat = FALSE;
			else
				bCashFlowStat = TRUE;

			if( nBalanceYHSet.size() >100 )
				bBalanceYH = FALSE;
			else
				bBalanceYH = TRUE;
			
			if( nBalanceBXSet.size() >100 )
				bBalanceBX = FALSE;
			else
				bBalanceBX = TRUE;

			if( nBalanceZQSet.size() > 100 )
				bBalanceZQ = FALSE;
			else
				bBalanceZQ = TRUE;

			if( nProfitYHSet.size() > 100)
				bProfitYH = FALSE;
			else
				bProfitYH = TRUE;
			
			if( nProfitBXSet.size() > 100 )
				bProfitBX = FALSE;
			else
				bProfitBX = TRUE;

			if( nProfitZQSet.size() > 100)
				bProfitZQ = FALSE;
			else
				bProfitZQ = TRUE;

			if( nCashFlowYHSet.size() > 100 )
				bCashFlowYH = FALSE;
			else
				bCashFlowYH = TRUE;
			
			if( nCashFlowBXSet.size() > 100 )
				bCashFlowBX = FALSE;
			else
				bCashFlowBX = TRUE;
			
			if( nCashFlowZQSet.size() > 100 )
				bCashFlowZQ = FALSE;
			else
				bCashFlowZQ = TRUE;
			//-------------20080829------------------------------------

			//----------------------------------------------

			//------wangzhy---------20080527-----------------------------------

			map <CString, Tx::Core::VariantData>::iterator iterVdReq = mapSheetReqVariantData.begin();

			for(iterSheetReq = mapSheetReq.begin(); iterSheetReq != mapSheetReq.end(); iterSheetReq ++,iterVdReq++)
			{
				strReq = iterSheetReq->first;
				dValue = Tx::Core::Con_doubleInvalid;
				//2008-06-13 by zhaohj
				strResult = iterSheetReq->second;
				if(strResult.GetLength()>0)
				{
					TRACE(_T("\n �����ظ�����excel"));
					continue;
				}

				strResult = _T("-");
				//dValue = 0.0;
				dValue = Con_doubleInvalid;
				dValue1 = 0.0;
				dValue2 = 0.0;
				dValue3 = 0.0;

				//2008-06-12 by zhaohj
				//֧�ַ���������
				Tx::Core::VariantData vdReq;

				FindItemOfReq(&reqTXFunction, strReq, 26);	//startdate��enddate����������
				FindItemOfReq(&reqTXFunction, strReq, 2);	//FuncID��SubItemID��Security
				if(reqTXFunction.iFuncID == 1 || reqTXFunction.iFuncID == 2)
				{
					//�ַ���
					vdReq.data_type = dtype_val_string;

					//֤ȯ����
					if(strlen(reqTXFunction.cName) > 0)
					{
						strValue.Format(_T("%s"), reqTXFunction.cName);
						strValue.Trim();

						SecurityNamePred pred(strValue);
						StrMMapIter iter = find_if(mmapNameCode.begin(),mmapNameCode.end(),pred);
						int matchCount = count_if(mmapNameCode.begin(),mmapNameCode.end(),pred);

						for(int i=0;i<matchCount;iter++,i++)
						{
							iTransObjID = (INT)GetSecurityId(iter->second);
							if(iTransObjID > 0 && business.GetSecurityNow((LONG)iTransObjID))
							{
								if( (reqTXFunction.iSubItemID == 0 && business.m_pSecurity->IsCN_Market()) ||
								    (reqTXFunction.iSubItemID == 1 && business.m_pSecurity->IsHongkong()) )
								{
									if( (reqTXFunction.iIndexFlg == 0 && !business.m_pSecurity->IsIndex()) ||
										(reqTXFunction.iIndexFlg != 0 && business.m_pSecurity->IsIndex()))
									{
										strResult = iter->second;
										break;
									}
								}
							}
						}
					}

					if(strResult.GetLength() > 0)
					{
						if(reqTXFunction.iFuncID == 1)
						{	//֤ȯ����(���г���׺)
							strResult = strResult.Left(strResult.GetLength() - 3);
						}
					}
				}else if ( reqTXFunction.iFuncID >= 209 && reqTXFunction.iFuncID <= 211 )
				{
					//��������
					FindItemOfReq(&reqTXFunction, strReq, 25);
					iSubItemID = reqTXFunction.iSubItemID-1;
					CString strTemp;
					strTemp.Format(_T("%d"),reqTXFunction.nSecurityID);
					int nSecurityId = (int)GetSecurityID(strTemp, 7 );
					pBusiness->GetSecurityNow(nSecurityId);
					if ( pBusiness->m_pSecurity != NULL )
					{
						if ( !pBusiness->m_pSecurity->InTxIndustryIndexBlock())
							continue;
						reqTXFunction.nSecurityID = TypeMapManage::GetInstance()->GetIDByValueITI(TYPE_INDEX_INDUSTRY,nSecurityId);
						//reqTXFunction.nSecurityID = pBusiness->m_pSecurity->GetTxSec_IndustryId();
						if ( reqTXFunction.iReportType == 0)
							reqTXFunction.iReportType = 2;
						else if ( reqTXFunction.iReportType == 1)
							reqTXFunction.iReportType = 1;
						switch( reqTXFunction.iFuncID )
						{
						case 209:
							pBalanceStat = NULL;
							//����ʵ��
							if ( bBalanceStat )
							{
								nInstitution = reqTXFunction.nSecurityID;
								nAccountingID = reqTXFunction.nFYear * 100000 + reqTXFunction.nFQuarter * 10 + reqTXFunction.iReportType;
								bool bLoad = false;
								bLoad = pBalanceStatDataFile->Load( nInstitution,30228,true);
								if ( bLoad )
									pBalanceStat = pBalanceStatDataFile->GetDataByObj( nAccountingID,false );
							}
							else
							{
								nInstitution = reqTXFunction.nFYear * 10000 + reqTXFunction.nFQuarter;
								nAccountingID = reqTXFunction.nSecurityID * 10 + reqTXFunction.iReportType;
								bool bLoad = false;
								bLoad = pBalanceStatDataFile->Load( nInstitution, 30229,true );
								if ( bLoad)
									pBalanceStat = pBalanceStatDataFile->GetDataByObj( nAccountingID,false );
							}
							if ( pBalanceStat == NULL )
								break;
							if ( iSubItemID == 0 )
							{
								int nCount = pBalanceStat->iCompanyCount;
								if ( nCount >= 0 )
									strResult.Format(_T("%d"),nCount);
							}
							else
							{
								iSubItemID--;
								if ( iSubItemID >=0 && iSubItemID <= 69 )
								{
									dValue = pBalanceStat->dValue[iSubItemID];
									if ( dValue != Con_doubleInvalid )
										strResult.Format(_T("%.2f"),dValue);
								}
							}
							break;
						case 210:
							pProfitStat = NULL;
							{
								if ( bProfitStat )
								{
									nInstitution = reqTXFunction.nSecurityID;
									nAccountingID = reqTXFunction.nFYear * 100000 + reqTXFunction.nFQuarter * 10 + reqTXFunction.iReportType;
									bool bLoad = false;
									bLoad = pProfitStatDataFile->Load( nInstitution,30230,true);
									if ( bLoad )
										pProfitStat = pProfitStatDataFile->GetDataByObj( nAccountingID,false );
								}
								else
								{
									nInstitution = reqTXFunction.nFYear * 10000 + reqTXFunction.nFQuarter;
									nAccountingID = reqTXFunction.nSecurityID * 10 + reqTXFunction.iReportType;
									bool bLoad = false;
									bLoad = pProfitStatDataFile->Load( nInstitution, 30231,true );
									if ( bLoad)
										pProfitStat = pProfitStatDataFile->GetDataByObj( nAccountingID,false );
								}
								if ( pProfitStat == NULL )
									break;
								if ( iSubItemID == 0 )
								{
									int nCount = pProfitStat->iCompanyCount;
									if ( nCount >= 0 )
										strResult.Format(_T("%d"),nCount);
								}
								else
								{
									iSubItemID--;
									if ( iSubItemID >=0 && iSubItemID <= 46 )
									{
										dValue = pProfitStat->dValue[iSubItemID];
										if ( dValue != Con_doubleInvalid )
											strResult.Format(_T("%.2f"),dValue);
									}
								}
							}
							break;
						case 211:
							pCashFlowStat = NULL;
							if ( bCashFlowStat )
							{
								nInstitution = reqTXFunction.nSecurityID;
								nAccountingID = reqTXFunction.nFYear * 100000 + reqTXFunction.nFQuarter * 10 + reqTXFunction.iReportType;
								bool bLoad = false;
								bLoad = pCashFlowStatDataFile->Load( nInstitution,30232,true);
								if ( bLoad )
									pCashFlowStat = pCashFlowStatDataFile->GetDataByObj( nAccountingID,false );
							}
							else
							{
								nInstitution = reqTXFunction.nFYear * 10000 + reqTXFunction.nFQuarter;
								nAccountingID = reqTXFunction.nSecurityID * 10 + reqTXFunction.iReportType;
								bool bLoad = false;
								bLoad = pCashFlowStatDataFile->Load( nInstitution, 30233,true );
								if ( bLoad)
									pCashFlowStat = pCashFlowStatDataFile->GetDataByObj( nAccountingID,false );
							}
							if ( pCashFlowStat == NULL )
								break;
							if ( iSubItemID == 0 )
							{
								int nCount = pCashFlowStat->iCompanyCount;
								if ( nCount >= 0 )
									strResult.Format(_T("%d"),nCount);
							}
							else
							{
								iSubItemID--;
								if ( iSubItemID >=0 && iSubItemID <= 69 )
								{
									dValue = pCashFlowStat->dValue[iSubItemID];
									if ( dValue != Con_doubleInvalid )
										strResult.Format(_T("%.2f"),dValue);
								}
							}
							break;
						}
					}
					//���㺯��
				}else if(pBusiness->GetSecurityNow(reqTXFunction.nSecurityID))
				{
					if(pBusiness->m_pSecurity != NULL)
					{
						switch(reqTXFunction.iFuncID)
						{
						case 3:		//֤ȯ���
							vdReq.data_type = dtype_val_string;
							strResult = pBusiness->m_pSecurity->GetName();
							break;
						case 4:		//B�ɴ���-->A�ɴ���
							vdReq.data_type = dtype_val_string;
							if(!pBusiness->m_pSecurity->IsStockB())
							{
								break;
							}
							mapIntInt.clear();
							Tx::Data::TypeMapManage::GetInstance()->GetTypeMapITI(TYPE_STOCK_TRANSOBJECT_BA, mapIntInt);
							iterIntInt = mapIntInt.find((INT)(reqTXFunction.nSecurityID));
							if(iterIntInt != mapIntInt.end())
							{
								if(business.GetSecurityNow(iterIntInt->second) != NULL)
								{
									strResult = business.m_pSecurity->GetCode();
								}
							}
							break;
						case 5:		//A�ɴ���-->B�ɴ���
							vdReq.data_type = dtype_val_string;
							if(!pBusiness->m_pSecurity->IsStockA())
							{
								break;
							}
							mapIntInt.clear();
							Tx::Data::TypeMapManage::GetInstance()->GetTypeMapITI(TYPE_STOCK_TRANSOBJECT_AB, mapIntInt);
							iterIntInt = mapIntInt.find((INT)(reqTXFunction.nSecurityID));
							if(iterIntInt != mapIntInt.end())
							{
								if(business.GetSecurityNow(iterIntInt->second) != NULL)
								{
									strResult = business.m_pSecurity->GetCode();
								}
							}
							break;
						case 6:		//ע�����
							vdReq.data_type = dtype_val_string;
							strResult = pBusiness->m_pSecurity->GetRegisterProvance();
							break;
						case 10:	//����ȫ��
						case 72:	//���������
							vdReq.data_type = dtype_val_string;
							nInstitution = (INT)pBusiness->m_pSecurity->GetInstitutionId();
							if(nInstitution > 0)
							{
								mapIntStr.clear();
								Tx::Data::TypeMapManage::GetInstance()->GetTypeMap(TYPE_INSTITUTION_CHINALONGNAME, mapIntStr);
								iterIntStr = mapIntStr.find(nInstitution);
								if(iterIntStr != mapIntStr.end())
								{
									strValue = iterIntStr->second;
									if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
									{
										strResult = strValue;
									}
								}
							}
							break;
						case 7:		//֤�����ҵ
						case 281:   //֤�����ҵ[��]
							{
								vdReq.data_type = dtype_val_string;
								FindItemOfReq(&reqTXFunction, strReq, 22);	//��ҵ�㼶
								if(reqTXFunction.iFuncID == 7)
								    nIndustryID = pBusiness->m_pSecurity->GetCSRC_IndustryId();
								else
									nIndustryID = pBusiness->m_pSecurity->GetNewCSRC_IndustryId();

								iVal = 0;
								std::vector<CString> vTemp;
								vTemp.clear();
								while(nIndustryID > 0)
								{
									strValue = TypeMapManage::GetInstance()->GetDatByID(TYPE_CSRC_INDUSTRY_NAME, (INT)nIndustryID);
									vTemp.push_back( strValue );
									nIndustryID = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRY_PARENT, (INT)nIndustryID);
								}
								if ( reqTXFunction.nIndustryLevel == 0 )
								{
									for( std::vector<CString>::iterator iter = vTemp.begin(); iter != vTemp.end(); ++iter )
									{
										if ( strResult.GetLength() > 0 )
											strResult = *iter + _T(" - ") + strResult;
										else
											strResult = *iter;
									}	
								}
								else
								{
									int nCount = (int)vTemp.size();
									if(nCount < 1)
										strResult = _T("-");
									else
									{
										if ( reqTXFunction.nIndustryLevel >= nCount)
											strResult = *vTemp.begin();
										else
											strResult = vTemp[nCount-reqTXFunction.nIndustryLevel];
									}
									
								}
							//while(nIndustryID > 0)
							//{
							//	strValue = TypeMapManage::GetInstance()->GetDatByID(TYPE_CSRC_INDUSTRY_NAME, (INT)nIndustryID);
							//	if(reqTXFunction.nIndustryLevel == 0)
							//	{	//����ҵ�㼶
							//		if(iVal == 0)
							//		{
							//			strResult = strValue;
							//		}else
							//		{
							//			strResult = strValue + _T("-") + strResult;
							//		}
							//	}else
							//	{	//����ҵ�㼶
							//		strResult = strValue;
							//		if(reqTXFunction.nIndustryLevel == iVal + 1)
							//		{
							//			break;
							//		}
							//	}
							//	iVal ++;
							//	nIndustryID = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRY_PARENT, (INT)nIndustryID);
							//}
							}
							break;
						case 8:		//������ҵ
							vdReq.data_type = dtype_val_string;
							FindItemOfReq(&reqTXFunction, strReq, 22);	//��ҵ�㼶
							if ( reqTXFunction.nIndustryLevel == 1 )
								reqTXFunction.nIndustryLevel = 2;
							else if ( reqTXFunction.nIndustryLevel == 2 )
								reqTXFunction.nIndustryLevel  = 1;
							nIndustryID = pBusiness->m_pSecurity->GetTxSec_IndustryId();
							iVal = 0;
							while(nIndustryID > 0)
							{
								strValue = TypeMapManage::GetInstance()->GetDatByID(TYPE_TX_INDUSTRY_NAME, (INT)nIndustryID);
								if(reqTXFunction.nIndustryLevel == 0)
								{	//����ҵ�㼶
									if(iVal == 0)
									{
										strResult = strValue;
									}else
									{
										strResult = strValue + _T("-") + strResult;
									}
								}else
								{	//����ҵ�㼶
									strResult = strValue;
									if(reqTXFunction.nIndustryLevel == iVal + 1)
									{
										break;
									}
								}
								iVal ++;
								nIndustryID = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRY_PARENT, (INT)nIndustryID);
							}
							break;
						case 257:      //��֤��ҵ����
							{
								vdReq.data_type = dtype_val_string;
								FindItemOfReq(&reqTXFunction, strReq, 22);	//��ҵ�㼶
								nIndustryID = pBusiness->m_pSecurity->GetZZ_IndustryId();
								iVal = 0;
								std::vector<CString> vTemp;
								vTemp.clear();
								strResult = _T("");
								while(nIndustryID > 0)
								{
									strValue = TypeMapManage::GetInstance()->GetDatByID(TYPE_ZSZJ_INDUSTRY_NAME, (INT)nIndustryID);
									vTemp.push_back( strValue );
									nIndustryID = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRY_PARENT, (INT)nIndustryID);
								}
								if ( reqTXFunction.nIndustryLevel == 0 )
								{
									for( std::vector<CString>::iterator iter = vTemp.begin(); iter != vTemp.end(); ++iter )
									{
										if ( strResult.GetLength() > 0 )
											strResult = *iter + _T(" - ") + strResult;
										else
											strResult = *iter;
									}	
								}
								else
								{
									int nCount = (int)vTemp.size();
									if(nCount < 1)
										strResult = _T("-");
									else
									{
										if ( reqTXFunction.nIndustryLevel == nCount)
											strResult = *vTemp.begin();
										else if(reqTXFunction.nIndustryLevel > nCount)
											strResult = _T("-");
										else
											strResult = vTemp[nCount-reqTXFunction.nIndustryLevel];
									}

								}
							}
							break;
						case 259:      //������ҵ����
							{
								vdReq.data_type = dtype_val_string;
								FindItemOfReq(&reqTXFunction, strReq, 22);	//��ҵ�㼶
								nIndustryID = pBusiness->m_pSecurity->GetZX_IndustryId();
								iVal = 0;
								std::vector<CString> vTemp;
								vTemp.clear();
								strResult = _T("");
								while(nIndustryID > 0)
								{
									strValue = TypeMapManage::GetInstance()->GetDatByID(TYPE_ZSZJ_INDUSTRY_NAME, (INT)nIndustryID);
									vTemp.push_back( strValue );
									nIndustryID = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRY_PARENT, (INT)nIndustryID);
								}
								if ( reqTXFunction.nIndustryLevel == 0 )
								{
									for( std::vector<CString>::iterator iter = vTemp.begin(); iter != vTemp.end(); ++iter )
									{
										if ( strResult.GetLength() > 0 )
											strResult = *iter + _T(" - ") + strResult;
										else
											strResult = *iter;
									}	
								}
								else
								{
									int nCount = (int)vTemp.size();
									if(nCount < 1)
										strResult = _T("-");
									else
									{
										if ( reqTXFunction.nIndustryLevel >= nCount)
											strResult = *vTemp.begin();
										else
											strResult = vTemp[nCount-reqTXFunction.nIndustryLevel];
									}
								}
							}
							break;
						case 261:      //������ҵ����
							{
								vdReq.data_type = dtype_val_string;
								FindItemOfReq(&reqTXFunction, strReq, 22);	//��ҵ�㼶
								nIndustryID = pBusiness->m_pSecurity->GetSW_IndustryId();
								iVal = 0;
								std::vector<CString> vTemp;
								vTemp.clear();
								strResult = _T("");
								while(nIndustryID > 0)
								{
									strValue = TypeMapManage::GetInstance()->GetDatByID(TYPE_ZSZJ_INDUSTRY_NAME, (INT)nIndustryID);
									vTemp.push_back( strValue );
									nIndustryID = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRY_PARENT, (INT)nIndustryID);
								}
								if ( reqTXFunction.nIndustryLevel == 0 )
								{
									for( std::vector<CString>::iterator iter = vTemp.begin(); iter != vTemp.end(); ++iter )
									{
										if ( strResult.GetLength() > 0 )
											strResult = *iter + _T(" - ") + strResult;
										else
											strResult = *iter;
									}	
								}
								else
								{
									int nCount = (int)vTemp.size();
									if(nCount < 1)
										strResult = _T("-");
									else
									{
										if ( reqTXFunction.nIndustryLevel >= nCount)
											strResult = *vTemp.begin();
										else
											strResult = vTemp[nCount-reqTXFunction.nIndustryLevel];
									}
								}
							}
							break;
						case 263:      //�޳���ҵ����
							{
								vdReq.data_type = dtype_val_string;
								FindItemOfReq(&reqTXFunction, strReq, 22);	//��ҵ�㼶
								nIndustryID = pBusiness->m_pSecurity->GetJC_IndustryId();
								iVal = 0;
								std::vector<CString> vTemp;
								vTemp.clear();
								strResult = _T("");
								while (nIndustryID > 0)
								{
									strValue = TypeMapManage::GetInstance()->GetDatByID(TYPE_ZSZJ_INDUSTRY_NAME, (INT)nIndustryID);
									vTemp.push_back(strValue);
									nIndustryID = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRY_PARENT, (INT)nIndustryID);
								}
								if (reqTXFunction.nIndustryLevel == 0)
								{
									for (std::vector<CString>::iterator iter = vTemp.begin(); iter != vTemp.end(); ++iter)
									{
										if(strResult.GetLength() > 0)
											strResult = *iter + _T(" - ") + strResult;
										else
											strResult = *iter;
									}
								}
								else
								{
									int nCount = (int)vTemp.size();
									if(nCount < 1)
										strResult = _T("-");
									else
									{
										if(reqTXFunction.nIndustryLevel >= nCount)
											strResult = *vTemp.begin();
										else
											strResult = vTemp[nCount - reqTXFunction.nIndustryLevel];
									}
								}
							}
							break;
						case 218:		//֤�����ҵ����
						case 280:       //֤�����ҵ����[��]
							{
								vdReq.data_type = dtype_val_string;
								FindItemOfReq(&reqTXFunction, strReq, 22);	//��ҵ�㼶
								//strResult = pBusiness->m_pSecurity->GetCSRCIndustryCode(reqTXFunction.nIndustryLevel);
								int iIndustryId = 0;
								if (reqTXFunction.iFuncID == 218)
								    iIndustryId = pBusiness->m_pSecurity->GetCSRC_IndustryId();
								else
									iIndustryId = pBusiness->m_pSecurity->GetNewCSRC_IndustryId();

								int lid = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRYID_TO_LEVEL,iIndustryId);
								if(reqTXFunction.nIndustryLevel == 0)
									reqTXFunction.nIndustryLevel = lid;
								switch(reqTXFunction.nIndustryLevel)
								{
								case 1:
									{
										int l1id = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRYID_TO_LEVEL1ID,iIndustryId);
										strResult = TypeMapManage::GetInstance()->GetDatByID(TYPE_INDUSTRYID_TO_CODE,l1id);
									}
									break;
								case 2:
									{
										int l2id = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRYID_TO_LEVEL2ID,iIndustryId);
										strResult = TypeMapManage::GetInstance()->GetDatByID(TYPE_INDUSTRYID_TO_CODE,l2id);
									}
									break;
								case 3:
									{
										int l3id = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRYID_TO_LEVEL3ID,iIndustryId);
										strResult = TypeMapManage::GetInstance()->GetDatByID(TYPE_INDUSTRYID_TO_CODE,l3id);
									}
									break;
								case 4:
									{
										if (lid < 4)
										{
											strResult = _T("-");
											break;
										}
										strResult = TypeMapManage::GetInstance()->GetDatByID(TYPE_INDUSTRYID_TO_CODE,iIndustryId);
									}
									break;
								default:
									strResult = _T("-");
									break;
								}
								////step1 get level1id
								//int l1id = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRYID_TO_LEVEL1ID,iIndustryId);
								////step2 get level2id
								//int l2id = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRYID_TO_LEVEL2ID,iIndustryId);
								////step3
								////get level1name
								//CString sl1Name = TypeMapManage::GetInstance()->GetDatByID(TYPE_INDUSTRYID_TO_CODE,l1id);
								////step4
								////get level2name
								//CString sl2Name = TypeMapManage::GetInstance()->GetDatByID(TYPE_INDUSTRYID_TO_CODE,l2id);

								//int lid = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRYID_TO_LEVEL,iIndustryId);
								//CString sl3Name = _T("");
								//CString sl4Name = _T("");
								//if( lid == 3)
								//	sl3Name = TypeMapManage::GetInstance()->GetDatByID(TYPE_INDUSTRYID_TO_CODE,iIndustryId);
								//if( lid == 4)
								//	sl4Name = TypeMapManage::GetInstance()->GetDatByID(TYPE_INDUSTRYID_TO_CODE,iIndustryId);
								//switch(reqTXFunction.nIndustryLevel)
								//{
								//case 0:
								//	if(lid == 4)
								//		strResult = sl4Name;
								//	else if(lid == 2)
								//		strResult = sl2Name;
								//	else if(lid == 3)
								//		strResult = sl3Name;
								//	else
								//		strResult = sl1Name;
								//	break;
								//case 1:
								//	strResult = sl1Name;
								//	break;
								//case 2:
								//	strResult = sl2Name;
								//	break;
								//case 3:
								//	strResult = sl3Name;
								//	break;
								//case 4:
								//	strResult = sl4Name;
								//	break;
								//default:
								//	strResult = _T("-");
								//	break;
								//}
							}
							break;
						
						case 219:		//������ҵ����
							//vdReq.data_type = dtype_val_string;
							//FindItemOfReq(&reqTXFunction, strReq, 22);	//��ҵ�㼶
							//strResult = pBusiness->m_pSecurity->GetTxSecIndustryCode(reqTXFunction.nIndustryLevel);
							{
								vdReq.data_type = dtype_val_string;
								FindItemOfReq(&reqTXFunction, strReq, 22);	//��ҵ�㼶
								int iIndustryId = 0;
								iIndustryId = pBusiness->m_pSecurity->GetTxSec_IndustryId();

								if(reqTXFunction.nIndustryLevel == 0)
									reqTXFunction.nIndustryLevel = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRYID_TO_LEVEL,iIndustryId);
								switch(reqTXFunction.nIndustryLevel)
								{
								case 1:
									{
										int l1id = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRYID_TO_LEVEL1ID,iIndustryId);
										strResult = TypeMapManage::GetInstance()->GetDatByID(TYPE_INDUSTRYID_TO_CODE,l1id);
									}
									break;
								case 2:
									{
										int l2id = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRYID_TO_LEVEL2ID,iIndustryId);
										strResult = TypeMapManage::GetInstance()->GetDatByID(TYPE_INDUSTRYID_TO_CODE,l2id);
									}
									break;
								default:
									strResult = _T("-");
									break;
								}
								////step1 get level1id
								//int l1id = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRYID_TO_LEVEL1ID,iIndustryId);
								////step2 get level2id
								//int l2id = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRYID_TO_LEVEL2ID,iIndustryId);
								////step3
								////get level1name
								//CString sl1Name = TypeMapManage::GetInstance()->GetDatByID(TYPE_INDUSTRYID_TO_CODE,l1id);
								////step4
								////get level2name
								//CString sl2Name = TypeMapManage::GetInstance()->GetDatByID(TYPE_INDUSTRYID_TO_CODE,l2id);

								//switch(reqTXFunction.nIndustryLevel)
								//{
								//case 0:
								//	if(l2id < 0)
								//		strResult = sl1Name;
								//	else
								//		strResult = sl2Name;
								//	break;
								//case 1:
								//	strResult = sl1Name;
								//	break;
								//case 2:
								//	strResult = sl2Name;
								//	break;
								//default:
								//	strResult = _T("-");
								//	break;
								//}
							}
							break;
						case 256:   //��֤��ҵ����
							{
								vdReq.data_type = dtype_val_string;
								FindItemOfReq(&reqTXFunction, strReq, 22);	//��ҵ�㼶
								int iIndustryId = 0;
								iIndustryId = pBusiness->m_pSecurity->GetZZ_IndustryId();

								int lid = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRYID_TO_LEVEL,iIndustryId);
								if(reqTXFunction.nIndustryLevel == 0)
									reqTXFunction.nIndustryLevel = lid;
								switch(reqTXFunction.nIndustryLevel)
								{
								case 1:
									{
										int l1id = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRYID_TO_LEVEL1ID,iIndustryId);
										strResult = TypeMapManage::GetInstance()->GetDatByID(TYPE_INDUSTRYID_TO_CODE,l1id);
									}
									break;
								case 2:
									{
										int l2id = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRYID_TO_LEVEL2ID,iIndustryId);
										strResult = TypeMapManage::GetInstance()->GetDatByID(TYPE_INDUSTRYID_TO_CODE,l2id);
									}
									break;
								case 3:
									{
										int l3id = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRYID_TO_LEVEL3ID,iIndustryId);
										strResult = TypeMapManage::GetInstance()->GetDatByID(TYPE_INDUSTRYID_TO_CODE,l3id);
									}
									break;
								case 4:
									{
										if(lid < 4)
										{
											strResult = _T("-");
											break;
										}
										strResult = TypeMapManage::GetInstance()->GetDatByID(TYPE_INDUSTRYID_TO_CODE,iIndustryId);
									}
									break;
								default:
									strResult = _T("-");
									break;
								}
							}
							break;
						case 258:   //������ҵ����
							{
								vdReq.data_type = dtype_val_string;
								FindItemOfReq(&reqTXFunction, strReq, 22);	//��ҵ�㼶
								int iIndustryId = 0;
								iIndustryId = pBusiness->m_pSecurity->GetZX_IndustryId();

								int lid = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRYID_TO_LEVEL,iIndustryId);
								if(reqTXFunction.nIndustryLevel == 0)
									reqTXFunction.nIndustryLevel = lid;
								switch(reqTXFunction.nIndustryLevel)
								{
								case 1:
									{
										int l1id = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRYID_TO_LEVEL1ID,iIndustryId);
										strResult = TypeMapManage::GetInstance()->GetDatByID(TYPE_INDUSTRYID_TO_CODE,l1id);
									}
									break;
								case 2:
									{
										int l2id = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRYID_TO_LEVEL2ID,iIndustryId);
										strResult = TypeMapManage::GetInstance()->GetDatByID(TYPE_INDUSTRYID_TO_CODE,l2id);
									}
									break;
								case 3:
									{
										if (lid < 3)
										{
											strResult = _T("-");
											break;
										}
										strResult = TypeMapManage::GetInstance()->GetDatByID(TYPE_INDUSTRYID_TO_CODE,iIndustryId);
									}
									break;
								default:
									strResult = _T("-");
									break;
								}
							}
							break;
						case 260:   //������ҵ����
							{
								vdReq.data_type = dtype_val_string;
								FindItemOfReq(&reqTXFunction, strReq, 22);	//��ҵ�㼶
								int iIndustryId = 0;
								iIndustryId = pBusiness->m_pSecurity->GetSW_IndustryId();

								int lid = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRYID_TO_LEVEL,iIndustryId);
								if(reqTXFunction.nIndustryLevel == 0)
									reqTXFunction.nIndustryLevel = lid;
								switch(reqTXFunction.nIndustryLevel)
								{
								case 1:
									{
										int l1id = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRYID_TO_LEVEL1ID,iIndustryId);
										strResult = TypeMapManage::GetInstance()->GetDatByID(TYPE_INDUSTRYID_TO_CODE,l1id);
									}
									break;
								case 2:
									{
										int l2id = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRYID_TO_LEVEL2ID,iIndustryId);
										strResult = TypeMapManage::GetInstance()->GetDatByID(TYPE_INDUSTRYID_TO_CODE,l2id);
									}
									break;
								case 3:
									{
										if (lid < 3)
										{
											strResult = _T("-");
											break;
										}
										strResult = TypeMapManage::GetInstance()->GetDatByID(TYPE_INDUSTRYID_TO_CODE,iIndustryId);
									}
									break;
								default:
									strResult = _T("-");
									break;
								}
							}
							break;
						case 262:   //�޳���ҵ����
							{
								vdReq.data_type = dtype_val_string;
								FindItemOfReq(&reqTXFunction, strReq, 22);	//��ҵ�㼶
								int iIndustryId = 0;
								iIndustryId = pBusiness->m_pSecurity->GetJC_IndustryId();

								int lid = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRYID_TO_LEVEL,iIndustryId);
								if(reqTXFunction.nIndustryLevel == 0)
									reqTXFunction.nIndustryLevel = lid;
								switch(reqTXFunction.nIndustryLevel)
								{
								case 1:
									{
										int l1id = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRYID_TO_LEVEL1ID,iIndustryId);
										strResult = TypeMapManage::GetInstance()->GetDatByID(TYPE_INDUSTRYID_TO_CODE,l1id);
									}
									break;
								case 2:
									{
										int l2id = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRYID_TO_LEVEL2ID,iIndustryId);
										strResult = TypeMapManage::GetInstance()->GetDatByID(TYPE_INDUSTRYID_TO_CODE,l2id);
									}
									break;
								case 3:
									{
										int l3id = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRYID_TO_LEVEL3ID,iIndustryId);
										strResult = TypeMapManage::GetInstance()->GetDatByID(TYPE_INDUSTRYID_TO_CODE,l3id);
									}
									break;
								case 4:
									{
										if (lid < 4)
										{
											strResult = _T("-");
											break;
										}
										strResult = TypeMapManage::GetInstance()->GetDatByID(TYPE_INDUSTRYID_TO_CODE,iIndustryId);
									}
									break;
								default:
									strResult = _T("-");
									break;
								}
							}
							break;
						case 9:		//���м۸�
							vdReq.data_type = dtype_float;
							//--����һ��򿪣�����������һЩû���׷�
							dValue = pBusiness->m_pSecurity->GetFirstDateIssuePrice();	//�״η��м۸�
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							dValue = GetDateIndicator_Value((INT)(reqTXFunction.nSecurityID), reqTXFunction.nDate, &vStockIssue);
							if(dValue > 0)
							{
								strResult.Format(_T("%.2f"), dValue);
							}
							break;
						case 11:	//IPO��������
							vdReq.data_type = dtype_int4;
							nDate = pBusiness->m_pSecurity->GetIPOListedDate();
							if(nDate > 0)
							{
								//�޸�Ϊ8λ��--20080708--
								strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
							}
							break;
						case 12:	//IPO��������
							vdReq.data_type = dtype_int4;
							nDate = pBusiness->m_pSecurity->GetIPOIssueDate();
							if(nDate > 0)
							{
								//�޸�Ϊ8λ��--20080708--
								strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
							}
							break;
						case 13:	//��������
							vdReq.data_type = dtype_int4;
							FindItemOfReq(&reqTXFunction, strReq, 6);	//Date��OffsetDays
							nDate = GetTradeDateByOffset(pBusiness, reqTXFunction.nDate, reqTXFunction.nOffsetDays);
							if(nDate > 0)
							{
								//�޸�Ϊ8λ��--20080708--
								strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
							}
							break;
						case 14:	//��������
							vdReq.data_type = dtype_int4;
							FindItemOfReq(&reqTXFunction, strReq, 5);	//StartDate��EndDate
							nTradeDays = GetTradeDays(pBusiness, reqTXFunction.nStartDate, reqTXFunction.nEndDate);
							strResult.Format(_T("%d"), nTradeDays);
							break;
						case 15:	//���̼�
							vdReq.data_type = dtype_float;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							if ( bHQSecurity )
								dValue = GetKLineEx(pBusiness, reqTXFunction.nDate, 0,reqTXFunction.nSecurityID);
							else
								dValue = GetKLine(pBusiness, reqTXFunction.nDate, 0,reqTXFunction.nSecurityID);
							if(dValue <= 0)
							{
								break;
							}
							//bug:11640   2012-09-25
							//if(pBusiness->m_pSecurity->IsFundTradeInMarket()||(pBusiness->m_pSecurity->IsShanghai()&&pBusiness->m_pSecurity->IsStockB()))
							if(pBusiness->m_pSecurity->IsFundTradeInMarket() || pBusiness->m_pSecurity->IsFund_Bond() || pBusiness->m_pSecurity->IsFund_Intermix() || 
								pBusiness->m_pSecurity->IsFund_ETF() || pBusiness->m_pSecurity->IsFund_LOF() || pBusiness->m_pSecurity->IsFund_QDII() || 
								pBusiness->m_pSecurity->IsFund_Stock() || (pBusiness->m_pSecurity->IsStockB() && pBusiness->m_pSecurity->IsShanghai())||
								(pBusiness->m_pSecurity->IsBond_National() && pBusiness->m_pSecurity->IsShenzhen()) || pBusiness->m_pSecurity->IsBond_Change() ||
								pBusiness->m_pSecurity->IsWarrant() || pBusiness->m_pSecurity->IsRepurchase())
							{
								strResult.Format(_T("%.3f"), dValue);
								/*			CString strlog;
								strlog.Format( _T("Excel���������̼ۡ�--cost 15 = %s"), strResult );
								Tx::Log::CLogRecorder::GetInstance()->WriteToLog(strlog);*/
							}else
							{
								strResult.Format(_T("%.2f"), dValue);
						/*		CString strlog;
								strlog.Format( _T("Excel���������̼ۡ�--cost 15 = %s"), strResult );
								Tx::Log::CLogRecorder::GetInstance()->WriteToLog(strlog);*/
							}
							break;
						case 56:	//���̼�[��Ȩ]
							vdReq.data_type = dtype_float;
							FindItemOfReq(&reqTXFunction, strReq, 9);	//Date��StartDate��EndDate����Ȩ����
							if ( bHQSecurity )
								dValue1 = GetKLineEx(pBusiness, reqTXFunction.nDate, 0,reqTXFunction.nSecurityID);
							else
								dValue1 = GetKLine(pBusiness, reqTXFunction.nDate, 0 ,reqTXFunction.nSecurityID);
							if(dValue1 <= 0)
							{
								break;
							}
							if(reqTXFunction.iIRType == 0)
							{	//ǰ��Ȩ     // 20100810 by wanglm ��Ȩ�۸��޸� ��56 - 61
								dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nDate, reqTXFunction.nEndDate, true);
								//dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nEndDate, true);
							}else
							{	//��Ȩ
								if ( reqTXFunction.nStartDate == 0 )
									reqTXFunction.nStartDate = pBusiness->m_pSecurity->GetTradeDateByIndex(0);	
								dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nDate, false);
								//dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nEndDate, false);
							}
							//if(pBusiness->m_pSecurity->IsFundTradeInMarket())
							//bug:11640  2012-09-25
							if(pBusiness->m_pSecurity->IsFundTradeInMarket() || pBusiness->m_pSecurity->IsFund_Bond() || pBusiness->m_pSecurity->IsFund_Intermix() || 
								pBusiness->m_pSecurity->IsFund_ETF() || pBusiness->m_pSecurity->IsFund_LOF() || pBusiness->m_pSecurity->IsFund_QDII() || 
								pBusiness->m_pSecurity->IsFund_Stock() || (pBusiness->m_pSecurity->IsStockB() && pBusiness->m_pSecurity->IsShanghai())||
								(pBusiness->m_pSecurity->IsBond_National() && pBusiness->m_pSecurity->IsShenzhen()) || pBusiness->m_pSecurity->IsBond_Change() ||
								pBusiness->m_pSecurity->IsWarrant() || pBusiness->m_pSecurity->IsRepurchase())
							{
								strResult.Format(_T("%.3f"), dValue1 * dValue2||(pBusiness->m_pSecurity->IsShanghai()&&pBusiness->m_pSecurity->IsStockB()));
							}else
							{
								strResult.Format(_T("%.2f"), dValue1 * dValue2);
							}
							break;
						case 16:	//ǰ�ռ�
							vdReq.data_type = dtype_float;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							if ( bHQSecurity )
								dValue = GetKLineEx(pBusiness, reqTXFunction.nDate, 1,reqTXFunction.nSecurityID);
							else
								dValue = GetKLine(pBusiness, reqTXFunction.nDate, 1,reqTXFunction.nSecurityID);
							if(dValue <= 0)
							{
								break;
							}
							//bug:11640  2012-09-25
							//if(pBusiness->m_pSecurity->IsFundTradeInMarket()||(pBusiness->m_pSecurity->IsShanghai()&&pBusiness->m_pSecurity->IsStockB()))
							if(pBusiness->m_pSecurity->IsFundTradeInMarket() || pBusiness->m_pSecurity->IsFund_Bond() || pBusiness->m_pSecurity->IsFund_Intermix() || 
								pBusiness->m_pSecurity->IsFund_ETF() || pBusiness->m_pSecurity->IsFund_LOF() || pBusiness->m_pSecurity->IsFund_QDII() || 
								pBusiness->m_pSecurity->IsFund_Stock() || (pBusiness->m_pSecurity->IsStockB() && pBusiness->m_pSecurity->IsShanghai())||
								(pBusiness->m_pSecurity->IsBond_National() && pBusiness->m_pSecurity->IsShenzhen()) || pBusiness->m_pSecurity->IsBond_Change() ||
								pBusiness->m_pSecurity->IsWarrant() || pBusiness->m_pSecurity->IsRepurchase())
							{
								strResult.Format(_T("%.3f"), dValue);
								/*			CString strlog;
								strlog.Format( _T("Excel������ǰ�ռۡ�--cost 16 = %s"), strResult );
								Tx::Log::CLogRecorder::GetInstance()->WriteToLog(strlog);*/
							}else
							{
								strResult.Format(_T("%.2f"), dValue);
							/*	CString strlog;
								strlog.Format( _T("Excel������ǰ�ռۡ�--cost 16 = %s"), strResult );
								Tx::Log::CLogRecorder::GetInstance()->WriteToLog(strlog);*/
							}
							break;
						case 57:	//ǰ�ռ�[��Ȩ]
							vdReq.data_type = dtype_float;
							FindItemOfReq(&reqTXFunction, strReq, 9);	//Date��StartDate��EndDate����Ȩ����
							if ( bHQSecurity )
								dValue1 = GetKLineEx(pBusiness, reqTXFunction.nDate, 1,reqTXFunction.nSecurityID);
							else
								dValue1 = GetKLine(pBusiness, reqTXFunction.nDate, 1,reqTXFunction.nSecurityID);
							if(dValue1 <= 0)
							{
								break;
							}
							if(reqTXFunction.iIRType == 0)
							{	//ǰ��Ȩ    // 20100810 by wanglm ��Ȩ�۸��޸�
								dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nDate, reqTXFunction.nEndDate, true);
								//dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nEndDate, true);
							}else
							{	//��Ȩ
								if ( reqTXFunction.nStartDate == 0 )
									reqTXFunction.nStartDate = pBusiness->m_pSecurity->GetTradeDateByIndex(0);	
								dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nDate, false);
								//dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nEndDate, false);
							}
							//bug:11640  2012-09-25
							//if(pBusiness->m_pSecurity->IsFundTradeInMarket()||(pBusiness->m_pSecurity->IsShanghai()&&pBusiness->m_pSecurity->IsStockB()))
							if(pBusiness->m_pSecurity->IsFundTradeInMarket() || pBusiness->m_pSecurity->IsFund_Bond() || pBusiness->m_pSecurity->IsFund_Intermix() || 
								pBusiness->m_pSecurity->IsFund_ETF() || pBusiness->m_pSecurity->IsFund_LOF() || pBusiness->m_pSecurity->IsFund_QDII() || 
								pBusiness->m_pSecurity->IsFund_Stock() || (pBusiness->m_pSecurity->IsStockB() && pBusiness->m_pSecurity->IsShanghai())||
								(pBusiness->m_pSecurity->IsBond_National() && pBusiness->m_pSecurity->IsShenzhen()) || pBusiness->m_pSecurity->IsBond_Change() ||
								pBusiness->m_pSecurity->IsWarrant() || pBusiness->m_pSecurity->IsRepurchase())
							{
								strResult.Format(_T("%.3f"), dValue1 * dValue2);
							}else
							{
								strResult.Format(_T("%.2f"), dValue1 * dValue2);
							}
							break;
						case 17:	//��߼�
							vdReq.data_type = dtype_float;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							if ( bHQSecurity )
								dValue = GetKLineEx(pBusiness, reqTXFunction.nDate, 2,reqTXFunction.nSecurityID);
							else
								dValue = GetKLine(pBusiness, reqTXFunction.nDate, 2,reqTXFunction.nSecurityID);
							if(dValue <= 0)
							{
								break;
							}
							//bug:11640  2012-09-25
							//if(pBusiness->m_pSecurity->IsFundTradeInMarket()||(pBusiness->m_pSecurity->IsShanghai()&&pBusiness->m_pSecurity->IsStockB()))
							if(pBusiness->m_pSecurity->IsFundTradeInMarket() || pBusiness->m_pSecurity->IsFund_Bond() || pBusiness->m_pSecurity->IsFund_Intermix() || 
								pBusiness->m_pSecurity->IsFund_ETF() || pBusiness->m_pSecurity->IsFund_LOF() || pBusiness->m_pSecurity->IsFund_QDII() || 
								pBusiness->m_pSecurity->IsFund_Stock() || (pBusiness->m_pSecurity->IsStockB() && pBusiness->m_pSecurity->IsShanghai())||
								(pBusiness->m_pSecurity->IsBond_National() && pBusiness->m_pSecurity->IsShenzhen()) || pBusiness->m_pSecurity->IsBond_Change() ||
								pBusiness->m_pSecurity->IsWarrant() || pBusiness->m_pSecurity->IsRepurchase())
							{
								strResult.Format(_T("%.3f"), dValue);
								/*		CString strlog;
								strlog.Format( _T("Excel��������߼ۡ�--cost 17 = %s"), strResult );
								Tx::Log::CLogRecorder::GetInstance()->WriteToLog(strlog);*/
							}else
							{
								strResult.Format(_T("%.2f"), dValue);
							/*	CString strlog;
								strlog.Format( _T("Excel��������߼ۡ�--cost 17 = %s"), strResult );
								Tx::Log::CLogRecorder::GetInstance()->WriteToLog(strlog);*/
							}
							break;
						case 58:	//��߼�[��Ȩ]
							vdReq.data_type = dtype_float;
							FindItemOfReq(&reqTXFunction, strReq, 9);	//Date��StartDate��EndDate����Ȩ����
							if ( bHQSecurity )
								dValue1 = GetKLineEx(pBusiness, reqTXFunction.nDate, 2,reqTXFunction.nSecurityID);
							else
								dValue1 = GetKLine(pBusiness, reqTXFunction.nDate, 2,reqTXFunction.nSecurityID);
							if(dValue1 <= 0)
							{
								break;
							}
							if(reqTXFunction.iIRType == 0)
							{	//ǰ��Ȩ   // 20100810 by wanglm ��Ȩ�۸��޸�
								dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nDate, reqTXFunction.nEndDate, true);
								//dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nEndDate, true);
							}else
							{	//��Ȩ
								if ( reqTXFunction.nStartDate == 0 )
									reqTXFunction.nStartDate = pBusiness->m_pSecurity->GetTradeDateByIndex(0);	
								dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nDate, false);
								//dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nEndDate, false);
							}
							//bug:11640  2012-09-25
							//if(pBusiness->m_pSecurity->IsFundTradeInMarket()||(pBusiness->m_pSecurity->IsShanghai()&&pBusiness->m_pSecurity->IsStockB()))
							if(pBusiness->m_pSecurity->IsFundTradeInMarket() || pBusiness->m_pSecurity->IsFund_Bond() || pBusiness->m_pSecurity->IsFund_Intermix() || 
								pBusiness->m_pSecurity->IsFund_ETF() || pBusiness->m_pSecurity->IsFund_LOF() || pBusiness->m_pSecurity->IsFund_QDII() || 
								pBusiness->m_pSecurity->IsFund_Stock() || (pBusiness->m_pSecurity->IsStockB() && pBusiness->m_pSecurity->IsShanghai())||
								(pBusiness->m_pSecurity->IsBond_National() && pBusiness->m_pSecurity->IsShenzhen()) || pBusiness->m_pSecurity->IsBond_Change() ||
								pBusiness->m_pSecurity->IsWarrant() || pBusiness->m_pSecurity->IsRepurchase())
							{
								strResult.Format(_T("%.3f"), dValue1 * dValue2);
							}else
							{
								strResult.Format(_T("%.2f"), dValue1 * dValue2);
							}
							break;
						case 18:	//��ͼ�
							vdReq.data_type = dtype_float;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							if ( bHQSecurity )
								dValue = GetKLineEx(pBusiness, reqTXFunction.nDate, 3,reqTXFunction.nSecurityID);
							else
								dValue = GetKLine(pBusiness, reqTXFunction.nDate, 3,reqTXFunction.nSecurityID);
							if(dValue <= 0)
							{
								break;
							}
							//bug:11640  2012-09-25
							//if(pBusiness->m_pSecurity->IsFundTradeInMarket()||(pBusiness->m_pSecurity->IsShanghai()&&pBusiness->m_pSecurity->IsStockB()))
							if(pBusiness->m_pSecurity->IsFundTradeInMarket() || pBusiness->m_pSecurity->IsFund_Bond() || pBusiness->m_pSecurity->IsFund_Intermix() || 
								pBusiness->m_pSecurity->IsFund_ETF() || pBusiness->m_pSecurity->IsFund_LOF() || pBusiness->m_pSecurity->IsFund_QDII() || 
								pBusiness->m_pSecurity->IsFund_Stock() || (pBusiness->m_pSecurity->IsStockB() && pBusiness->m_pSecurity->IsShanghai())||
								(pBusiness->m_pSecurity->IsBond_National() && pBusiness->m_pSecurity->IsShenzhen()) || pBusiness->m_pSecurity->IsBond_Change() ||
								pBusiness->m_pSecurity->IsWarrant() || pBusiness->m_pSecurity->IsRepurchase())
							{
								strResult.Format(_T("%.3f"), dValue);
								/*	CString strlog;
								strlog.Format( _T("Excel��������ͼۡ�--cost 18 = %s"), strResult );
								Tx::Log::CLogRecorder::GetInstance()->WriteToLog(strlog);*/
							}else
							{
								strResult.Format(_T("%.2f"), dValue);
							/*	CString strlog;
								strlog.Format( _T("Excel��������ͼۡ�--cost 18 = %s"), strResult );
								Tx::Log::CLogRecorder::GetInstance()->WriteToLog(strlog);*/
							}
							break;
						case 59:	//��ͼ�[��Ȩ]
							vdReq.data_type = dtype_float;
							FindItemOfReq(&reqTXFunction, strReq, 9);	//Date��StartDate��EndDate����Ȩ����
							if ( bHQSecurity )
								dValue1 = GetKLineEx(pBusiness, reqTXFunction.nDate, 3,reqTXFunction.nSecurityID);
							else
								dValue1 = GetKLine(pBusiness, reqTXFunction.nDate, 3,reqTXFunction.nSecurityID);
							if(dValue1 <= 0)
							{
								break;
							}
							if(reqTXFunction.iIRType == 0)
							{	//ǰ��Ȩ   // 20100810 by wanglm ��Ȩ�۸��޸�
								dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nDate, reqTXFunction.nEndDate, true);
								//dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nEndDate, true);
							}else
							{	//��Ȩ
								if ( reqTXFunction.nStartDate == 0 )
									reqTXFunction.nStartDate = pBusiness->m_pSecurity->GetTradeDateByIndex(0);	
								dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nDate, false);
								//dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nEndDate, false);
							}
							//if(pBusiness->m_pSecurity->IsFundTradeInMarket())
							//bug:11640  2012-09-25
							if(pBusiness->m_pSecurity->IsFundTradeInMarket() || pBusiness->m_pSecurity->IsFund_Bond() || pBusiness->m_pSecurity->IsFund_Intermix() || 
								pBusiness->m_pSecurity->IsFund_ETF() || pBusiness->m_pSecurity->IsFund_LOF() || pBusiness->m_pSecurity->IsFund_QDII() || 
								pBusiness->m_pSecurity->IsFund_Stock() || (pBusiness->m_pSecurity->IsStockB() && pBusiness->m_pSecurity->IsShanghai())||
								(pBusiness->m_pSecurity->IsBond_National() && pBusiness->m_pSecurity->IsShenzhen()) || pBusiness->m_pSecurity->IsBond_Change() ||
								pBusiness->m_pSecurity->IsWarrant() || pBusiness->m_pSecurity->IsRepurchase())
							{
								strResult.Format(_T("%.3f"), dValue1 * dValue2||(pBusiness->m_pSecurity->IsShanghai()&&pBusiness->m_pSecurity->IsStockB()));
							}else
							{
								strResult.Format(_T("%.2f"), dValue1 * dValue2);
							}
							break;
						case 19:	//���̼�
							vdReq.data_type = dtype_float;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							if ( bHQSecurity )
								dValue = GetKLineEx(pBusiness, reqTXFunction.nDate, 4,reqTXFunction.nSecurityID);
							else
								dValue = GetKLine(pBusiness, reqTXFunction.nDate, 4,reqTXFunction.nSecurityID);
							if(dValue <= 0)
							{
								break;
							}
							//bug:11640   2012-09-25
							//if(pBusiness->m_pSecurity->IsFundTradeInMarket()||(pBusiness->m_pSecurity->IsShanghai()&&pBusiness->m_pSecurity->IsStockB()))
							if(pBusiness->m_pSecurity->IsFundTradeInMarket() || pBusiness->m_pSecurity->IsFund_Bond() || pBusiness->m_pSecurity->IsFund_Intermix() || 
								pBusiness->m_pSecurity->IsFund_ETF() || pBusiness->m_pSecurity->IsFund_LOF() || pBusiness->m_pSecurity->IsFund_QDII() || 
								pBusiness->m_pSecurity->IsFund_Stock() || (pBusiness->m_pSecurity->IsStockB() && pBusiness->m_pSecurity->IsShanghai())||
								(pBusiness->m_pSecurity->IsBond_National() && pBusiness->m_pSecurity->IsShenzhen()) || pBusiness->m_pSecurity->IsBond_Change() ||
								pBusiness->m_pSecurity->IsWarrant() || pBusiness->m_pSecurity->IsRepurchase() || pBusiness->m_pSecurity->IsIndex()
							  )
							{
								strResult.Format(_T("%.3f"), dValue);
					/*			CString strlog;
								strlog.Format( _T("Excel���������̼ۡ�--cost 19 = %s"), strResult );
								Tx::Log::CLogRecorder::GetInstance()->WriteToLog(strlog);*/
							}else
							{
								strResult.Format(_T("%.2f"), dValue);
					/*			CString strlog;
								strlog.Format( _T("Excel���������̼ۡ�--cost 19 = %s"), strResult );
								Tx::Log::CLogRecorder::GetInstance()->WriteToLog(strlog);*/
							}
							break;
						case 60:	//���̼�[��Ȩ]
							vdReq.data_type = dtype_float;
							FindItemOfReq(&reqTXFunction, strReq, 9);	//Date��StartDate��EndDate����Ȩ����
							if ( bHQSecurity )
								dValue1 = GetKLineEx(pBusiness, reqTXFunction.nDate, 4,reqTXFunction.nSecurityID);
							else
								dValue1 = GetKLine(pBusiness, reqTXFunction.nDate, 4,reqTXFunction.nSecurityID);
							if(dValue1 <= 0)
							{
								break;
							}
							if(reqTXFunction.iIRType == 0)
							{	//ǰ��Ȩ    // 20100810 by wanglm ��Ȩ�۸��޸� 
								dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nDate, reqTXFunction.nEndDate, true);    
								//dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nEndDate, true);
							}else
							{	//��Ȩ
								if ( reqTXFunction.nStartDate == 0 )
									reqTXFunction.nStartDate = pBusiness->m_pSecurity->GetTradeDateByIndex(0);	

								dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nDate, false);   
								//dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nEndDate, false);
							}
							//bug:11640  2012-09-25
							//if(pBusiness->m_pSecurity->IsFundTradeInMarket()||(pBusiness->m_pSecurity->IsShanghai()&&pBusiness->m_pSecurity->IsStockB()))
							if(pBusiness->m_pSecurity->IsFundTradeInMarket() || pBusiness->m_pSecurity->IsFund_Bond() || pBusiness->m_pSecurity->IsFund_Intermix() || 
								pBusiness->m_pSecurity->IsFund_ETF() || pBusiness->m_pSecurity->IsFund_LOF() || pBusiness->m_pSecurity->IsFund_QDII() || 
								pBusiness->m_pSecurity->IsFund_Stock() || (pBusiness->m_pSecurity->IsStockB() && pBusiness->m_pSecurity->IsShanghai())||
								(pBusiness->m_pSecurity->IsBond_National() && pBusiness->m_pSecurity->IsShenzhen()) || pBusiness->m_pSecurity->IsBond_Change() ||
								pBusiness->m_pSecurity->IsWarrant() || pBusiness->m_pSecurity->IsRepurchase())
							{
								strResult.Format(_T("%.3f"), dValue1 * dValue2);
							}else
							{
								strResult.Format(_T("%.2f"), dValue1 * dValue2);
							}
							break;
						case 20:	//�վ���
							vdReq.data_type = dtype_float;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							if ( bHQSecurity)
							{	
								/*dValue1 = GetKLineEx(pBusiness, reqTXFunction.nDate, 5);
								dValue2 = GetKLineEx(pBusiness, reqTXFunction.nDate, 6);*/
								dValue = GetKLineExNew(pBusiness, reqTXFunction.nDate, 9,reqTXFunction.nSecurityID);
							}
							else
							{
								/*dValue1 = GetKLine(pBusiness, reqTXFunction.nDate, 5);
								dValue2 = GetKLine(pBusiness, reqTXFunction.nDate, 6);*/
								dValue = GetKLine(pBusiness, reqTXFunction.nDate, 9,reqTXFunction.nSecurityID);
							}
							//if(dValue1 > 0 && dValue2 > 0)
							//{	//�ɽ����/�ɽ���
							//	dValue = dValue1 / dValue2;
							//}else
							//{	//ǰ�ռ�
							//	dValue = GetKLine(pBusiness, reqTXFunction.nDate, 1);
							//}
							if(dValue < 0.1)
								dValue = GetKLine(pBusiness, reqTXFunction.nDate, 1,reqTXFunction.nSecurityID);
							if(dValue <= 0)
							{
								break;
							}
							if(pBusiness->m_pSecurity->IsFundTradeInMarket()||(pBusiness->m_pSecurity->IsShanghai()&&pBusiness->m_pSecurity->IsStockB()))
							{
								strResult.Format(_T("%.3f"), dValue);
								/*			CString strlog;
								strlog.Format( _T("Excel�������վ��ۡ�--cost 21 = %s"), strResult );
								Tx::Log::CLogRecorder::GetInstance()->WriteToLog(strlog);*/
							}else
							{
								strResult.Format(_T("%.2f"), dValue);
					/*			CString strlog;
								strlog.Format( _T("Excel�������վ��ۡ�--cost 21 = %s"), strResult );
								Tx::Log::CLogRecorder::GetInstance()->WriteToLog(strlog);*/
							}
							break;
						case 61:	//�վ���[��Ȩ]
							vdReq.data_type = dtype_float;
							FindItemOfReq(&reqTXFunction, strReq, 9);	//Date��StartDate��EndDate����Ȩ����
							if ( bHQSecurity)
							{	
								dValue1 = GetKLineEx(pBusiness, reqTXFunction.nDate, 5,reqTXFunction.nSecurityID);
								dValue2 = GetKLineEx(pBusiness, reqTXFunction.nDate, 6,reqTXFunction.nSecurityID);
							}
							else
							{
								dValue1 = GetKLine(pBusiness, reqTXFunction.nDate, 5,reqTXFunction.nSecurityID);
								dValue2 = GetKLine(pBusiness, reqTXFunction.nDate, 6,reqTXFunction.nSecurityID);
							}
							if(reqTXFunction.iIRType == 0)
							{	//ǰ��Ȩ   // 20100810 by wanglm ��Ȩ�۸��޸�
								dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nDate, reqTXFunction.nEndDate, true);    
								//dValue3 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nEndDate, true);
							}else
							{	//��Ȩ
								if ( reqTXFunction.nStartDate == 0 )
									reqTXFunction.nStartDate = pBusiness->m_pSecurity->GetTradeDateByIndex(0);	
								dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nDate, false); 
								//dValue3 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nEndDate, false);
							}
							if(dValue1 > 0 && dValue2 > 0)
							{	//�ɽ����/�ɽ���
								dValue = dValue1 / dValue2;
							}else
							{	//ǰ�ռ�
								dValue = GetKLine(pBusiness, reqTXFunction.nDate, 1,reqTXFunction.nSecurityID);
							}
							if(dValue <= 0)
							{
								break;
							}
							if(pBusiness->m_pSecurity->IsFundTradeInMarket()||(pBusiness->m_pSecurity->IsShanghai()&&pBusiness->m_pSecurity->IsStockB()))
							{
								strResult.Format(_T("%.3f"), dValue * dValue3);
							}else
							{
								strResult.Format(_T("%.2f"), dValue * dValue3);
							}
							break;
						case 21:	//�ɽ���
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							if ( bHQSecurity )
								dValue = GetKLineEx(pBusiness, reqTXFunction.nDate, 6,reqTXFunction.nSecurityID);
							else
								dValue = GetKLine(pBusiness, reqTXFunction.nDate, 6,reqTXFunction.nSecurityID);
							if(dValue > 0)
							{
								strResult.Format(_T("%.0f"), dValue);
							}
							break;
						case 22:	//�ɽ����
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							if ( bHQSecurity )
								dValue = GetKLineEx(pBusiness, reqTXFunction.nDate, 5,reqTXFunction.nSecurityID);
							else
								dValue = GetKLine(pBusiness, reqTXFunction.nDate, 5,reqTXFunction.nSecurityID);
							if(dValue > 0)
							{
								if(pBusiness->m_pSecurity->IsFundTradeInMarket())
								{
									strResult.Format(_T("%.3f"), dValue);
								}else
								{
									strResult.Format(_T("%.2f"), dValue);
								}
							}
							break;
						case 23:	//������
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							if ( bHQSecurity )
								dValue1 = GetKLineEx(pBusiness, reqTXFunction.nDate, 6,reqTXFunction.nSecurityID);
							else
								dValue1 = GetKLine(pBusiness, reqTXFunction.nDate, 6,reqTXFunction.nSecurityID);
							dValue2 = 0;
							if(pBusiness->m_pSecurity->IsStock())
							{	//��Ʊ
								pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(reqTXFunction.nDate);
								if(pTxShareData != NULL)
								{
									dValue2 = pTxShareData->TradeableShare;
								}
							}
							//������ʱ���޸�--wangzhy----
							else if( pBusiness->m_pSecurity->IsIndex_TX())
							{
								if ( bHQSecurity )
									dValue1 = GetKLineEx(pBusiness, reqTXFunction.nDate, 5 ,reqTXFunction.nSecurityID);
								else
									dValue1 = GetKLine(pBusiness, reqTXFunction.nDate, 5 ,reqTXFunction.nSecurityID);
								Tx::Data::IndexShareData* p = pBusiness->m_pSecurity->GetIndexShareDataByDate( reqTXFunction.nDate,false );
								if ( p != NULL )
									dValue2 = p->TradeableValue;
							}
							else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
							{	//����
								pTxFundShareData = pBusiness->m_pSecurity->GetTxFundShareDataByDate(reqTXFunction.nDate);
								if(pTxFundShareData != NULL)
								{
									dValue2 = pTxFundShareData->TradeableShare;
								}
							}else if(pBusiness->m_pSecurity->IsBond_Change())
							{	//��תծ
								pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
								nDataCount = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
								if(nDataCount > 0)
								{
									pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex();
									if(reqTXFunction.nDate < pCBondAmount->end_date)
									{	//��תծ������
										dValue2 = pBondNewInfo->share / 100;
									}else
									{	//δת��ծȯ���
										pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(nDataCount - 1);
										if(reqTXFunction.nDate >= pCBondAmount->end_date)
										{
											dValue2 = pCBondAmount->not_change_bond_amount / 100;
										}else
										{
											pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByDate(reqTXFunction.nDate);
											dValue2 = pCBondAmount->not_change_bond_amount / 100;
										}
									}
								}else
								{	//��תծ������
									dValue2 = pBondNewInfo->share / 100 ;
								}
							}
							if(dValue1 > 0 && dValue2 > 0)
							{
								strResult.Format(_T("%.2f"), dValue1 * 100 / dValue2);
							}
							break;
						case 24:	//�ۼƳɽ���
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 5);	//StartDate��EndDate
							dValue = GetSumVolume(pBusiness, reqTXFunction.nStartDate, reqTXFunction.nEndDate);
							if(dValue > 0)
							{
								strResult.Format(_T("%.0f"), dValue);
							}
							break;
						case 25:	//�ۼƳɽ����
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 5);	//StartDate��EndDate
							dValue = GetSumAmount(pBusiness, reqTXFunction.nStartDate, reqTXFunction.nEndDate);
							if(dValue > 0)
							{
								if(pBusiness->m_pSecurity->IsFundTradeInMarket())
								{
									strResult.Format(_T("%.3f"), dValue);
								}else
								{
									strResult.Format(_T("%.2f"), dValue);
								}
							}
							break;
						case 26:	//�ۼƻ�����
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 5);	//StartDate��EndDate
							dValue = GetSumExchangeRatio(pBusiness, reqTXFunction.nStartDate, reqTXFunction.nEndDate);
							if(dValue > 0)
							{
								strResult.Format(_T("%.2f"), 100 * dValue);
							}
							break;
						case 27:	//��ͨ��ֵ
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							dValue1 = GetKLine(pBusiness, reqTXFunction.nDate, 4,reqTXFunction.nSecurityID);
							dValue2 = 0;
							if(pBusiness->m_pSecurity->IsStock())
							{	//��Ʊ
								pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(reqTXFunction.nDate);
								if(pTxShareData != NULL)
								{
									dValue2 = pTxShareData->TradeableShare;
								}
							}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
							{	//����
								pTxFundShareData = pBusiness->m_pSecurity->GetTxFundShareDataByDate(reqTXFunction.nDate);
								if(pTxFundShareData != NULL)
								{
									dValue2 = pTxFundShareData->TradeableShare;
								}
							}else if (pBusiness->m_pSecurity->IsIndex_TX())
							{
								Tx::Data::IndexShareData* pIndexShare = pBusiness->m_pSecurity->GetIndexShareDataByDate(reqTXFunction.nDate,false);
								if ( pIndexShare != NULL )
									dValue2 = pIndexShare->TradeableValue;
							}
							else if(pBusiness->m_pSecurity->IsBond_Change())
							{	//��תծ
								pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
								nDataCount = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
								if(nDataCount > 0)
								{
									pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex();
									if(reqTXFunction.nDate < pCBondAmount->end_date)
									{	//��תծ������
										dValue2 = pBondNewInfo->share / 100;
									}else
									{	//δת��ծȯ���
										pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(nDataCount - 1);
										if(reqTXFunction.nDate >= pCBondAmount->end_date)
										{
											dValue2 = pCBondAmount->not_change_bond_amount / 100;
										}else
										{
											pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByDate(reqTXFunction.nDate);
											dValue2 = pCBondAmount->not_change_bond_amount / 100;
										}
									}
								}else
								{	//��תծ������
									dValue2 = pBondNewInfo->share / 100 ;
								}
							}
							else if(pBusiness->m_pSecurity->IsIndex())
							{
								Tx::Data::IndexShareData* m_pIndexShareData = NULL;
								DataFileNormal<blk_TxExFile_FileHead,IndexShareData>* pDataFile = new DataFileNormal<blk_TxExFile_FileHead,IndexShareData>;
								if(pDataFile!=NULL)
								{
									bool bLoaded = true;
									bLoaded = pDataFile->Load(reqTXFunction.nSecurityID,30036,true);
									if(bLoaded)
										m_pIndexShareData = pDataFile->GetDataByObj(reqTXFunction.nDate,false);
									if(m_pIndexShareData != NULL)
										dValue = m_pIndexShareData->TradeableValue;
									if(dValue < 0)
										dValue = Con_doubleInvalid;
									if(dValue > 0 &&dValue != Con_doubleInvalid)
										strResult.Format(_T("%.2f"), dValue);
								}
								if(pDataFile != NULL)
									delete pDataFile;
								pDataFile = NULL;
							}
							if(dValue1 > 0 && dValue2 > 0)
							{
								if(pBusiness->m_pSecurity->IsStock()||pBusiness->m_pSecurity->IsIndex_TX())
								{
									strValue.Format(_T("%.2f"), dValue1);
								}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
								{
									strValue.Format(_T("%.3f"), dValue1);
								}else if(pBusiness->m_pSecurity->IsBond_Change())
								{
									strValue.Format(_T("%.2f"), dValue1);
								}
								dValue1 = atof(strValue);
								dValue2 = (ULONGLONG)dValue2;
								dValue = dValue1 * dValue2;
								if(pBusiness->m_pSecurity->IsFundTradeInMarket())
								{
									strResult.Format(_T("%.3f"), dValue);
								}else if( pBusiness->m_pSecurity->IsIndex_TX())
								{
									strResult.Format(_T("%.2f"), dValue2);
								}
								else
								{
									strResult.Format(_T("%.2f"), dValue);
								}
							}
							break;
						case 28:	//������ֵ(A)
						case 279:   //������ֵ(A+B)
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							dValue1 = GetKLine(pBusiness, reqTXFunction.nDate, 4,reqTXFunction.nSecurityID);
							dValue2 = 0;
							dValue3 = 0;
							dValue4 = 0;
							dValue = 0;
							// ��Ʊ   dValue = dValue1*dValue2 + dValue3*dValue4
							if(pBusiness->m_pSecurity->IsStock())
							{	//��Ʊ
								if (reqTXFunction.iFuncID == 28)  //������ֵ(A)
								{
									if (pBusiness->m_pSecurity->IsStockA())
									{
										pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(reqTXFunction.nDate);
										if(pTxShareData != NULL)
											dValue2 = pTxShareData->TheShare;
									}
									// �����B�ɣ�תΪA��
									else if (pBusiness->m_pSecurity->IsStockB())
									{
										//�����B�ɣ������Ƿ���A��
										mapIntInt.clear();
										Tx::Data::TypeMapManage::GetInstance()->GetTypeMapITI(TYPE_STOCK_TRANSOBJECT_BA, mapIntInt);

										//�����A�ɣ�ͬʱ����B�ɣ���B�ɹɱ����ϣ������B�ɣ�ͬʱ����A�ɣ���A�ɹɱ�����
										iterIntInt = mapIntInt.find((INT)(reqTXFunction.nSecurityID));
										if(iterIntInt != mapIntInt.end())
										{
											if(pBusiness->GetSecurityNow(iterIntInt->second) != NULL)
											{
												dValue1 = GetKLine(pBusiness, reqTXFunction.nDate, 4,iterIntInt->second);
												pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(reqTXFunction.nDate);
												if(pTxShareData != NULL)
													dValue2 = pTxShareData->TheShare;
											}
										}
									}
									//----------------------------------StockB---------------------------------------

								}
								else if (reqTXFunction.iFuncID == 279) //������ֵ(A+B)
								{
									pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(reqTXFunction.nDate);
									if(pTxShareData != NULL)
										dValue2 = pTxShareData->TheShare;
									//
									//�ж���A�ɻ���B��
									if (pBusiness->m_pSecurity->IsStockA())
									{
										//�����A��,�����Ƿ���B��
										mapIntInt.clear();
										Tx::Data::TypeMapManage::GetInstance()->GetTypeMapITI(TYPE_STOCK_TRANSOBJECT_AB, mapIntInt);
									}
									//----------------------------------StockA---------------------------------------
									else if (pBusiness->m_pSecurity->IsStockB())
									{
										//�����B�ɣ������Ƿ���A��
										mapIntInt.clear();
										Tx::Data::TypeMapManage::GetInstance()->GetTypeMapITI(TYPE_STOCK_TRANSOBJECT_BA, mapIntInt);
									}
									//----------------------------------StockB---------------------------------------
									//�����A�ɣ�ͬʱ����B�ɣ���B�ɹɱ����ϣ������B�ɣ�ͬʱ����A�ɣ���A�ɹɱ�����
									iterIntInt = mapIntInt.find((INT)(reqTXFunction.nSecurityID));
									if(iterIntInt != mapIntInt.end())
									{
										if(pBusiness->GetSecurityNow(iterIntInt->second) != NULL)
										{
											dValue3 = GetKLine(pBusiness, reqTXFunction.nDate, 4,iterIntInt->second);

											pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(reqTXFunction.nDate);
											if(pTxShareData != NULL)
												dValue4 = pTxShareData->TheShare;
										}
									}
								}

							}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
							{	//����
								pTxFundShareData = pBusiness->m_pSecurity->GetTxFundShareDataByDate(reqTXFunction.nDate);
								if(pTxFundShareData != NULL)
								{
									dValue2 = pTxFundShareData->TotalShare;
								}
							}else if(pBusiness->m_pSecurity->IsBond_Change())
							{	//��תծ
								pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
								nDataCount = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
								if(nDataCount > 0)
								{
									pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex();
									if(reqTXFunction.nDate < pCBondAmount->end_date)
									{	//��תծ������
										dValue2 = pBondNewInfo->share / 100;
									}else
									{	//δת��ծȯ���
										pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(nDataCount - 1);
										if(reqTXFunction.nDate >= pCBondAmount->end_date)
										{
											dValue2 = pCBondAmount->not_change_bond_amount / 100;
										}else
										{
											pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByDate(reqTXFunction.nDate);
											dValue2 = pCBondAmount->not_change_bond_amount / 100;
										}
									}
								}else
								{	//��תծ������
									dValue2 = pBondNewInfo->share / 100 ;
								}
							}
							else if(pBusiness->m_pSecurity->IsIndex())
							{
								Tx::Data::IndexShareData* m_pIndexShareData = NULL;
								DataFileNormal<blk_TxExFile_FileHead,IndexShareData>* pDataFile = new DataFileNormal<blk_TxExFile_FileHead,IndexShareData>;
								if(pDataFile!=NULL)
								{
									bool bLoaded = true;
									bLoaded = pDataFile->Load(reqTXFunction.nSecurityID,30036,true);
									if(bLoaded)
										m_pIndexShareData = pDataFile->GetDataByObj(reqTXFunction.nDate,false);
									if(m_pIndexShareData != NULL)
										dValue = m_pIndexShareData->TotalValue;
									if(dValue < 0)
										dValue = Con_doubleInvalid;
									if(dValue > 0 &&dValue != Con_doubleInvalid)
										strResult.Format(_T("%.2f"), dValue);
								}
								if(pDataFile != NULL)
									delete pDataFile;
								pDataFile = NULL;
							}
							if (pBusiness->m_pSecurity->IsStock())
							{
								if(pBusiness->m_pSecurity->IsStockA())
								{
									strValue.Format(_T("%.2f"), dValue1);
									strValue1.Format(_T("%.3f"), dValue3);
								}
								else if(pBusiness->m_pSecurity->IsStockB())
								{
									strValue.Format(_T("%.3f"), dValue1);
									strValue1.Format(_T("%.2f"), dValue3);
								}

								if (dValue1 > 0 && dValue2 > 0)
								{
									dValue1 = atof(strValue);
									dValue2 = (ULONGLONG)dValue2;
									dValue = dValue1*dValue2;
								}
								if (dValue3 > 0 && dValue4 > 0)
								{
									dValue3 = atof(strValue1);
									dValue4 = (ULONGLONG)dValue4;
									dValue += dValue3*dValue4;
								}
							}
							else
							{
								if(dValue1 > 0 && dValue2 > 0)
								{
                                    if(pBusiness->m_pSecurity->IsFundTradeInMarket())
										strValue.Format(_T("%.3f"), dValue1);
									else if(pBusiness->m_pSecurity->IsBond_Change())
										strValue.Format(_T("%.2f"), dValue1);
									//����
									dValue1 = atof(strValue);
									dValue2 = (ULONGLONG)dValue2;
									dValue = dValue1 * dValue2;
								}
							}
							if (dValue > 0)
							{
								if(pBusiness->m_pSecurity->IsFundTradeInMarket())
									strResult.Format(_T("%.3f"), dValue);
								else
									strResult.Format(_T("%.2f"), dValue);
							}
							break;
						case 284: // ������ֵ(B)
							{
								vdReq.data_type = dtype_double;
								FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
								// ��Ʊ   dValue = dValue1*dValue2
								if(pBusiness->m_pSecurity->IsStock())
								{	//��Ʊ
									if (pBusiness->m_pSecurity->IsStockB())
									{
										dValue1 = GetKLine(pBusiness, reqTXFunction.nDate, 4,reqTXFunction.nSecurityID);
										pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(reqTXFunction.nDate);
										if(pTxShareData != NULL)
											dValue2 = pTxShareData->TheShare;
									}
									// �����A�ɣ�תΪB��
									else if (pBusiness->m_pSecurity->IsStockA())
									{
										//�����B�ɣ������Ƿ���A��
										mapIntInt.clear();
										Tx::Data::TypeMapManage::GetInstance()->GetTypeMapITI(TYPE_STOCK_TRANSOBJECT_AB, mapIntInt);
										iterIntInt = mapIntInt.find((INT)(reqTXFunction.nSecurityID));
										if(iterIntInt != mapIntInt.end())
										{
											if(pBusiness->GetSecurityNow(iterIntInt->second) != NULL)
											{
												dValue1 = GetKLine(pBusiness, reqTXFunction.nDate, 4,iterIntInt->second);
												pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(reqTXFunction.nDate);
												if(pTxShareData != NULL)
													dValue2 = pTxShareData->TheShare;
											}
										}
									}

									if (dValue1>0 && dValue2>0)
									{
										strValue.Format(_T("%.3f"),dValue1);
										dValue1 = atof(strValue);
										dValue2 = (ULONGLONG)dValue2;
										dValue = dValue1*dValue2;
									}
									if (dValue > 0)
										strResult.Format(_T("%.2f"),dValue);
								}
							}
							break;
						case 29:	//����ֵ
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							dValue1 = GetKLine(pBusiness, reqTXFunction.nDate, 4,reqTXFunction.nSecurityID);
							dValue2 = 0;
							if(pBusiness->m_pSecurity->IsStock())
							{	//��Ʊ
								pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(reqTXFunction.nDate);
								if(pTxShareData != NULL)
								{
									dValue2 = pTxShareData->TotalShare;
								}
							}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
							{	//����
								pTxFundShareData = pBusiness->m_pSecurity->GetTxFundShareDataByDate(reqTXFunction.nDate);
								if(pTxFundShareData != NULL)
								{
									dValue2 = pTxFundShareData->TotalShare;
								}
							}else if (pBusiness->m_pSecurity->IsIndex_TX())
							{
								Tx::Data::IndexShareData* pIndexShare = pBusiness->m_pSecurity->GetIndexShareDataByDate(reqTXFunction.nDate,false);
								if ( pIndexShare != NULL )
									dValue2 = pIndexShare->TotalValue;
							}
							else if(pBusiness->m_pSecurity->IsBond_Change())
							{	//��תծ
								pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
								nDataCount = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
								if(nDataCount > 0)
								{
									pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex();
									if(reqTXFunction.nDate < pCBondAmount->end_date)
									{	//��תծ������
										dValue2 = pBondNewInfo->share / 100;
									}else
									{	//δת��ծȯ���
										pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(nDataCount - 1);
										if(reqTXFunction.nDate >= pCBondAmount->end_date)
										{
											dValue2 = pCBondAmount->not_change_bond_amount / 100;
										}else
										{
											pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByDate(reqTXFunction.nDate);
											dValue2 = pCBondAmount->not_change_bond_amount / 100;
										}
									}
								}else
								{	//��תծ������
									dValue2 = pBondNewInfo->share / 100 ;
								}
							}
							else if(pBusiness->m_pSecurity->IsIndex())
							{
								Tx::Data::IndexShareData* m_pIndexShareData = NULL;
								DataFileNormal<blk_TxExFile_FileHead,IndexShareData>* pDataFile = new DataFileNormal<blk_TxExFile_FileHead,IndexShareData>;
								if(pDataFile!=NULL)
								{
									bool bLoaded = true;
									bLoaded = pDataFile->Load(reqTXFunction.nSecurityID,30036,true);
									if(bLoaded)
										m_pIndexShareData = pDataFile->GetDataByObj(reqTXFunction.nDate,false);
									if(m_pIndexShareData != NULL)
										dValue = m_pIndexShareData->TotalValue;
									if(dValue < 0)
										dValue = Con_doubleInvalid;
									if(dValue > 0 &&dValue != Con_doubleInvalid)
										strResult.Format(_T("%.2f"), dValue);
								}
								if(pDataFile != NULL)
									delete pDataFile;
								pDataFile = NULL;
							}
							if(dValue1 > 0 && dValue2 > 0)
							{
								if(pBusiness->m_pSecurity->IsStock()||pBusiness->m_pSecurity->IsIndex_TX())
								{
									strValue.Format(_T("%.2f"), dValue1);
								}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
								{
									strValue.Format(_T("%.3f"), dValue1);
								}else if(pBusiness->m_pSecurity->IsBond_Change())
								{
									strValue.Format(_T("%.2f"), dValue1);
								}
								dValue1 = atof(strValue);
								dValue2 = (ULONGLONG)dValue2;
								dValue = dValue1 * dValue2;
								if(pBusiness->m_pSecurity->IsFundTradeInMarket())
								{
									strResult.Format(_T("%.3f"), dValue);
								}else if( pBusiness->m_pSecurity->IsIndex_TX())
								{
									strResult.Format(_T("%.2f"), dValue2);
								}
								else
								{
									strResult.Format(_T("%.2f"), dValue);
								}
							}
							break;
						case 30:	//������<=ָ���۸�ĵ�һ������
							vdReq.data_type = dtype_int4;
							FindItemOfReq(&reqTXFunction, strReq, 11);	//StartDate��EndDate���۸����͡���Ȩ���͡��۸�
							nDate = Get_Price_Occur_Date(pBusiness, reqTXFunction.dPrice, reqTXFunction.iPriceType, reqTXFunction.iIRType, reqTXFunction.nStartDate, reqTXFunction.nEndDate);
							if(nDate > 0)
							{
								//�޸�Ϊ8λ��--20080708--
								strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
							}
							break;
						case 31:	//������<=ָ���������ݵĵ�һ������
							vdReq.data_type = dtype_int4;
							FindItemOfReq(&reqTXFunction, strReq, 10);	//StartDate��EndDate���������͡���������
							nDate = Get_Value_Occur_Date(pBusiness, reqTXFunction.dFindValue, reqTXFunction.iValueType, reqTXFunction.nStartDate, reqTXFunction.nEndDate);
							if(nDate > 0)
							{
								//�޸�Ϊ8λ��--20080708--
								strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
							}							
							break;
						case 100:	//����������������Сֵ�ĵ�һ������
							vdReq.data_type = dtype_int4;
							FindItemOfReq(&reqTXFunction, strReq, 7);	//StartDate��EndDate����������
							nDate = Get_Value_Extremum_Date(pBusiness, reqTXFunction.iValueType, reqTXFunction.nStartDate, reqTXFunction.nEndDate, TRUE);
							if(nDate > 0)
							{
								//�޸�Ϊ8λ��--20080708--
								strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
							}	 
							break;
						case 101:	//�����������������ֵ�ĵ�һ������
							vdReq.data_type = dtype_int4;
							FindItemOfReq(&reqTXFunction, strReq, 7);	//StartDate��EndDate����������
							nDate = Get_Value_Extremum_Date(pBusiness, reqTXFunction.iValueType, reqTXFunction.nStartDate, reqTXFunction.nEndDate, FALSE);
							if(nDate > 0)
							{
								//�޸�Ϊ8λ��--20080708--
								strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
							}							
							break;
						case 102:	//�����ڼ۸���Сֵ�ĵ�һ������
							vdReq.data_type = dtype_int4;
							FindItemOfReq(&reqTXFunction, strReq, 8);	//StartDate��EndDate���۸�����
							nDate = Get_Price_Extremum_Date(pBusiness, reqTXFunction.iPriceType, reqTXFunction.nStartDate, reqTXFunction.nEndDate, TRUE);
							if(nDate > 0)
							{
								//�޸�Ϊ8λ��--20080708--
								strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
							}
							break;
						case 103:	//�����ڼ۸����ֵ�ĵ�һ������
							vdReq.data_type = dtype_int4;
							FindItemOfReq(&reqTXFunction, strReq, 8);	//StartDate��EndDate���۸�����
							nDate = Get_Price_Extremum_Date(pBusiness, reqTXFunction.iPriceType, reqTXFunction.nStartDate, reqTXFunction.nEndDate, FALSE);
							if(nDate > 0)
							{
								//�޸�Ϊ8λ��--20080708--
								strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
							}
							break;
						case 32:	//�׶��Ƿ�
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 5);	//StartDate��EndDate
							//��StartDateΪ��׼��EndDate���С���Ȩ��
							dValue3 = GetKLine(pBusiness, reqTXFunction.nStartDate, 1,reqTXFunction.nSecurityID);	//�״η��м۸�==��������ǰ�ռ�
							dValue1 = GetKLine(pBusiness, reqTXFunction.nEndDate, 4,reqTXFunction.nSecurityID);
							dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nEndDate, false);
							if(dValue1 > 0 && dValue2 > 0 && dValue3 > 0)
							{
								//��Ϊ�ٷ���20080710
								dValue = (dValue1 * dValue2) / dValue3 - 1;
								strResult.Format(_T("%.2f"), 100*dValue);
							}
							break;
						case 321:	//�׶��Ƿ�(������ʼ���ڵ����Ƿ�)
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 5);	//StartDate��EndDate
							//��StartDateΪ��׼��EndDate���С���Ȩ��
							dValue3 = GetKLine(pBusiness, reqTXFunction.nStartDate, 4,reqTXFunction.nSecurityID);	//�״η��м۸�==��������ǰ�ռ�
							dValue1 = GetKLine(pBusiness, reqTXFunction.nEndDate, 4,reqTXFunction.nSecurityID);
							dValue2 = pBusiness->m_pSecurity->GetExdividendScale(reqTXFunction.nStartDate, reqTXFunction.nEndDate, false);
							if(dValue1 > 0 && dValue2 > 0 && dValue3 > 0)
							{
								//��Ϊ�ٷ���20080710
								dValue = (dValue1 * dValue2) / dValue3 - 1;
								strResult.Format(_T("%.2f"), 100*dValue);
							}
							break;
						case 322:	//�����������Ƿ�
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 3);	
							dValue3 = GetKLine(pBusiness, pBusiness->m_pSecurity->GetTradeDateByIndex(0), 0,reqTXFunction.nSecurityID);	
							dValue1 = GetKLine(pBusiness, reqTXFunction.nDate, 4,reqTXFunction.nSecurityID);
							dValue2 = pBusiness->m_pSecurity->GetExdividendScale(pBusiness->m_pSecurity->GetTradeDateByIndex(0), reqTXFunction.nDate, false);
							if(dValue1 > 0 && dValue2 > 0 && dValue3 > 0)
							{
								//��Ϊ�ٷ���20080710
								dValue = (dValue1 * dValue2) / dValue3 - 1;
								strResult.Format(_T("%.2f"), 100*dValue);
							}
							break;
						case 33:	//������
							{
								vdReq.data_type = dtype_double;
								FindItemOfReq( &reqTXFunction,strReq,3 );
								nDate = reqTXFunction.nDate;
								if( nDate == CTime::GetCurrentTime().GetYear() * 10000 + CTime::GetCurrentTime().GetMonth() * 100 + CTime::GetCurrentTime().GetDay())
									dValue = pBusiness->m_pSecurity->GetBuyVolume( false );
								else
								{
									pDataOfaDay = pBusiness->m_pSecurity->GetHisTradeLatestDataOfaDay( nDate );
									if ( pDataOfaDay == NULL )
										break;
									double* pData = (double*)pDataOfaDay->DataBuf;
									dValue = *pData;
								}
								if ( dValue > 0.0 )
									strResult.Format(_T("%.2f"),dValue );
							}
							break;
						case 34:	//������
							vdReq.data_type = dtype_double;
							{
								FindItemOfReq( &reqTXFunction,strReq,3 );
								nDate = reqTXFunction.nDate;
								if ( nDate == CTime::GetCurrentTime().GetYear() * 10000 + CTime::GetCurrentTime().GetMonth() * 100 + CTime::GetCurrentTime().GetDay() )
									dValue = pBusiness->m_pSecurity->GetSaleVolume( false );
								else
								{
									pDataOfaDay = pBusiness->m_pSecurity->GetHisTradeLatestDataOfaDay( nDate );
									if ( pDataOfaDay == NULL )
										break;
									double* pData = (double*)pDataOfaDay->DataBuf;
									pData++;
									dValue = *pData;
								}
								if ( dValue > 0.0 )
									strResult.Format(_T("%.2f"),dValue );
							}
							break;
						case 144:	//���5�����̼�
							vdReq.data_type = dtype_float;
							FindItemOfReq(&reqTXFunction, strReq, 23);	//���������
							if(pBusiness == NULL || pBusiness->m_pSecurity == NULL)
								break;
							pBusiness->m_pSecurity->RequestQuotation();
							pTradeQuotation = pBusiness->m_pSecurity->GetTradeQuotationPointer();
							pTradeQuotation = pBusiness->m_pSecurity->GetTradeQuotationPointer();
							if(pTradeQuotation != NULL)
							{
								nDataCount = pTradeQuotation->GetTradeQuotationDataCount();
								if(nDataCount == 10)
								{
									pTradeQuotationData = pTradeQuotation->GetBuyData(reqTXFunction.iQuotationNo);
									if(pTradeQuotationData != NULL)
									{
										dValue = pTradeQuotationData->fPrice;
										if(pBusiness->m_pSecurity->IsFundTradeInMarket())
										{
											strResult.Format(_T("%.3f"), dValue);
										}else
										{
											strResult.Format(_T("%.2f"), dValue);
										}
									}
								}
							}
							break;
						case 145:	//���5�����̼�
							vdReq.data_type = dtype_float;
							FindItemOfReq(&reqTXFunction, strReq, 23);	//���������
						if(pBusiness == NULL || pBusiness->m_pSecurity == NULL)
							break;
						pBusiness->m_pSecurity->RequestQuotation();
							pTradeQuotation = pBusiness->m_pSecurity->GetTradeQuotationPointer();
							if(pTradeQuotation != NULL)
							{
								nDataCount = pTradeQuotation->GetTradeQuotationDataCount();
								if(nDataCount == 10)
								{
									pTradeQuotationData = pTradeQuotation->GetSaleData(reqTXFunction.iQuotationNo);
									if(pTradeQuotationData != NULL)
									{
										dValue = pTradeQuotationData->fPrice;
										if(pBusiness->m_pSecurity->IsFundTradeInMarket())
										{
											strResult.Format(_T("%.3f"), dValue);
										}else
										{
											strResult.Format(_T("%.2f"), dValue);
										}
									}
								}
							}
							break;
						case 146:	//���5��������
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 23);	//���������
						if(pBusiness == NULL || pBusiness->m_pSecurity == NULL)
							break;
						pBusiness->m_pSecurity->RequestQuotation();
							pTradeQuotation = pBusiness->m_pSecurity->GetTradeQuotationPointer();
							if(pTradeQuotation != NULL)
							{
								nDataCount = pTradeQuotation->GetTradeQuotationDataCount();
								if(nDataCount == 10)
								{
									pTradeQuotationData = pTradeQuotation->GetBuyData(reqTXFunction.iQuotationNo);
									if(pTradeQuotationData != NULL)
									{
										dValue = pTradeQuotationData->dVolume;
										strResult.Format(_T("%.0f"), dValue);
									}
								}
							}
							break;
						case 147:	//���5��������
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 23);	//���������
						if(pBusiness == NULL || pBusiness->m_pSecurity == NULL)
							break;
						pBusiness->m_pSecurity->RequestQuotation();
							pTradeQuotation = pBusiness->m_pSecurity->GetTradeQuotationPointer();
							if(pTradeQuotation != NULL)
							{
								nDataCount = pTradeQuotation->GetTradeQuotationDataCount();
								if(nDataCount == 10)
								{
									pTradeQuotationData = pTradeQuotation->GetSaleData(reqTXFunction.iQuotationNo);
									if(pTradeQuotationData != NULL)
									{
										dValue = pTradeQuotationData->dVolume;
										strResult.Format(_T("%.0f"), dValue);
									}
								}
							}
							break;
						case 148:	//ծȯ���5�����̼۶�Ӧ�ĵ���������
							vdReq.data_type = dtype_double;
							if(pBusiness->m_pSecurity->IsBond())
							{
								FindItemOfReq(&reqTXFunction, strReq, 23);	//���������
								if (!bBpYtmLoad[reqTXFunction.iQuotationNo-1])
								{
									GetMmpBondYtm(reqTXFunction.iQuotationNo);
									bBpYtmLoad[reqTXFunction.iQuotationNo-1] = true;
								}
								
								dValue = GetMmpBondYtm(reqTXFunction.nSecurityID,reqTXFunction.iQuotationNo);
								if (dValue > 0)
								{
									strResult.Format(_T("%.5f"), dValue);
								}
							}
							break;

							//vdReq.data_type = dtype_double;
							//if(pBusiness->m_pSecurity->IsBond())
							//{
							//	FindItemOfReq(&reqTXFunction, strReq, 23);	//���������
							//	pTradeQuotation = pBusiness->m_pSecurity->GetTradeQuotationPointer();
							//	if(pTradeQuotation != NULL)
							//	{
							//		nDataCount = pTradeQuotation->GetTradeQuotationDataCount();
							//		if(nDataCount == 10)
							//		{
							//			pTradeQuotationData = pTradeQuotation->GetBuyData(reqTXFunction.iQuotationNo);
							//			if(pTradeQuotationData != NULL)
							//			{
							//				nDate = pBusiness->m_pSecurity->GetCurDataDate();
							//				dValue1 = pTradeQuotationData->fPrice;
							//				if(pBusiness->m_pSecurity->IsBond_National())
							//				{	//��ծʵ�о��۽��ף����Ǽ��㵽��������ʱծȯ�۸�Ϊȫ��
							//					//dValue2 = bond.GetInterest((INT)(reqTXFunction.nSecurityID), nDate);
							//					//2012-7-16  Ӧ����Ϣ(��)
							//					dValue2 = bond.GetInterest_New((INT)(reqTXFunction.nSecurityID), nDate,true);
							//					if(dValue2 < 0)
							//					{
							//						break;
							//					}
							//					dValue1 += dValue2;
							//				}
							//				bond.Calc((INT)(reqTXFunction.nSecurityID), nDate, (FLOAT)dValue1);
							//				dValue = bond.Get_YTM();
							//				if(dValue != 0)
							//				{
							//					strResult.Format(_T("%.5f"), dValue);
							//				}
							//			}
							//		}
							//	}
							//}
							//break;
						case 149:	//ծȯ���5�����̼۶�Ӧ�ĵ���������
							vdReq.data_type = dtype_double;
							if(pBusiness->m_pSecurity->IsBond())
							{
								FindItemOfReq(&reqTXFunction, strReq, 23);	//���������

								if (!bSpYtmLoad[reqTXFunction.iQuotationNo-1])
								{
									GetMmpBondYtm(reqTXFunction.iQuotationNo,false);
									bSpYtmLoad[reqTXFunction.iQuotationNo-1] = true;
								}

								dValue = GetMmpBondYtm(reqTXFunction.nSecurityID,reqTXFunction.iQuotationNo,false);
								if (dValue > 0)
								{
									strResult.Format(_T("%.5f"), dValue);
								}
							}
							break;

							//vdReq.data_type = dtype_double;
							//if(pBusiness->m_pSecurity->IsBond())
							//{
							//	FindItemOfReq(&reqTXFunction, strReq, 23);	//���������
							//	pTradeQuotation = pBusiness->m_pSecurity->GetTradeQuotationPointer();
							//	if(pTradeQuotation != NULL)
							//	{
							//		nDataCount = pTradeQuotation->GetTradeQuotationDataCount();
							//		if(nDataCount == 10)
							//		{
							//			pTradeQuotationData = pTradeQuotation->GetSaleData(reqTXFunction.iQuotationNo);
							//			if(pTradeQuotationData != NULL)
							//			{
							//				nDate = pBusiness->m_pSecurity->GetCurDataDate();
							//				dValue1 = pTradeQuotationData->fPrice;
							//				if(pBusiness->m_pSecurity->IsBond_National())
							//				{	//��ծʵ�о��۽��ף����Ǽ��㵽��������ʱծȯ�۸�Ϊȫ��
							//					//dValue2 = bond.GetInterest((INT)(reqTXFunction.nSecurityID), nDate);
							//					//2012-7-16  Ӧ����Ϣ(��)
							//					dValue2 = bond.GetInterest_New((INT)(reqTXFunction.nSecurityID), nDate,true);
							//					if(dValue2 < 0)
							//					{
							//						break;
							//					}
							//					dValue1 += dValue2;
							//				}
							//				bond.Calc((INT)(reqTXFunction.nSecurityID), nDate, (FLOAT)dValue1);
							//				dValue = bond.Get_YTM();
							//				if(dValue != 0)
							//				{
							//					strResult.Format(_T("%.5f"), dValue);
							//				}
							//			}
							//		}
							//	}
							//}
							//break;
						case 35:	//��ͨ�ɱ�
							vdReq.data_type = dtype_int4;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							dValue = 0;
							if(pBusiness->m_pSecurity->IsStock())
							{	//��Ʊ
								pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(reqTXFunction.nDate);
								if(pTxShareData != NULL)
								{
									dValue = pTxShareData->TradeableShare;
								}
							}else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
							{	//����
								pTxFundShareData = pBusiness->m_pSecurity->GetTxFundShareDataByDate(reqTXFunction.nDate);
								if(pTxFundShareData != NULL)
								{
									dValue = pTxFundShareData->TradeableShare;
								}
							}else if(pBusiness->m_pSecurity->IsBond_Change())
							{	//��תծ
								pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
								nDataCount = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
								if(nDataCount > 0)
								{
									pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex();
									if(reqTXFunction.nDate < pCBondAmount->end_date)
									{	//��תծ������
										dValue = pBondNewInfo->share / 100;
									}else
									{	//δת��ծȯ���
										pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(nDataCount - 1);
										if(reqTXFunction.nDate >= pCBondAmount->end_date)
										{
											dValue = pCBondAmount->not_change_bond_amount / 100;
										}else
										{
											pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByDate(reqTXFunction.nDate);
											dValue = pCBondAmount->not_change_bond_amount / 100;
										}
									}
								}else
								{	//��תծ������
									dValue = pBondNewInfo->share / 100 ;
								}
							}
							if(dValue > 0)
							{
								strResult.Format(_T("%.0f"), dValue);
							}
							break;
						case 36:	//�ܹɱ�  wanglm
							vdReq.data_type = dtype_int4;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							dValue = 0;
							if(pBusiness->m_pSecurity->IsStock())
							{	//��Ʊ
								pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(reqTXFunction.nDate);
								if(pTxShareData != NULL)
								{
									dValue = pTxShareData->TotalShare;
								}
							}else if( pBusiness->m_pSecurity->IsFund() )  // IsFundTradeInMarket()
							{	//����
								pTxFundShareData = pBusiness->m_pSecurity->GetTxFundShareDataByDate(reqTXFunction.nDate);
								if(pTxFundShareData != NULL)
								{
									dValue = pTxFundShareData->TotalShare;
								}
							}else if(pBusiness->m_pSecurity->IsBond_Change())
							{	//��תծ
								pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
								nDataCount = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
								if(nDataCount > 0)
								{
									pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex();
									if(reqTXFunction.nDate < pCBondAmount->end_date)
									{	//��תծ������
										dValue = pBondNewInfo->share / 100;
									}else
									{	//δת��ծȯ���
										pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(nDataCount - 1);
										if(reqTXFunction.nDate >= pCBondAmount->end_date)
										{
											dValue = pCBondAmount->not_change_bond_amount / 100;
										}else
										{
											pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByDate(reqTXFunction.nDate);
											dValue = pCBondAmount->not_change_bond_amount / 100;
										}
									}
								}else
								{	//��תծ������
									dValue = pBondNewInfo->share / 100 ;
								}
							}
							if(dValue > 0)
							{
								strResult.Format(_T("%.0f"), dValue);
							}
							break;
						case 37:	//���ڹɱ�(A)
						case 278:   //���ڹɱ�(A+B)    mantis:15102
							vdReq.data_type = dtype_int4;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							dValue = 0;
							if(pBusiness->m_pSecurity->IsStock())
							{	//��Ʊ
								if (reqTXFunction.iFuncID == 37) //���ڹɱ�(A)
								{
									if (pBusiness->m_pSecurity->IsStockA())
									{
										pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(reqTXFunction.nDate);
										if(pTxShareData != NULL)
											dValue = pTxShareData->TheShare;
									}
									else if (pBusiness->m_pSecurity->IsStockB())
									{
										//תΪA��
										mapIntInt.clear();
										Tx::Data::TypeMapManage::GetInstance()->GetTypeMapITI(TYPE_STOCK_TRANSOBJECT_BA, mapIntInt);
										iterIntInt = mapIntInt.find((INT)(reqTXFunction.nSecurityID));
										if(iterIntInt != mapIntInt.end())
										{
											if(pBusiness->GetSecurityNow(iterIntInt->second) != NULL)
											{
												pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(reqTXFunction.nDate);
												if(pTxShareData != NULL)
													dValue = pTxShareData->TheShare;
											}
										}
									}
								}
								else if(reqTXFunction.iFuncID == 278)//���ڹɱ�(A+B)
								{
									pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(reqTXFunction.nDate);
									if(pTxShareData != NULL)
										dValue = pTxShareData->TheShare;
									if (dValue < 0)  dValue = 0;
									//�ж���A�ɻ���B��
									if (pBusiness->m_pSecurity->IsStockA())
									{
										//�����A��,�����Ƿ���B��
										mapIntInt.clear();
										Tx::Data::TypeMapManage::GetInstance()->GetTypeMapITI(TYPE_STOCK_TRANSOBJECT_AB, mapIntInt);
									}
									//----------------------------------StockA---------------------------------------
									else if (pBusiness->m_pSecurity->IsStockB())
									{
										//�����B�ɣ������Ƿ���A��
										mapIntInt.clear();
										Tx::Data::TypeMapManage::GetInstance()->GetTypeMapITI(TYPE_STOCK_TRANSOBJECT_BA, mapIntInt);
									}
									//----------------------------------StockB---------------------------------------
									//�����A�ɣ�ͬʱ����B�ɣ���B�ɹɱ����ϣ������B�ɣ�ͬʱ����A�ɣ���A�ɹɱ�����
									iterIntInt = mapIntInt.find((INT)(reqTXFunction.nSecurityID));
									if(iterIntInt != mapIntInt.end())
									{
										if(pBusiness->GetSecurityNow(iterIntInt->second) != NULL)
										{
											pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(reqTXFunction.nDate);
											if(pTxShareData != NULL)
												dValue += pTxShareData->TheShare;
										}
									}
								}
							}
							else if(pBusiness->m_pSecurity->IsFundTradeInMarket())
							{	//����
								pTxFundShareData = pBusiness->m_pSecurity->GetTxFundShareDataByDate(reqTXFunction.nDate);
								if(pTxFundShareData != NULL)
								{
									dValue = pTxFundShareData->TotalShare;
								}
							}else if(pBusiness->m_pSecurity->IsBond_Change())
							{	//��תծ
								pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
								nDataCount = pBusiness->m_pSecurity->GetBondNotChangeAmountCount();
								if(nDataCount > 0)
								{
									pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex();
									if(reqTXFunction.nDate < pCBondAmount->end_date)
									{	//��תծ������
										dValue = pBondNewInfo->share / 100;
									}else
									{	//δת��ծȯ���
										pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByIndex(nDataCount - 1);
										if(reqTXFunction.nDate >= pCBondAmount->end_date)
										{
											dValue = pCBondAmount->not_change_bond_amount / 100;
										}else
										{
											pCBondAmount = pBusiness->m_pSecurity->GetBondNotChangeAmountByDate(reqTXFunction.nDate);
											dValue = pCBondAmount->not_change_bond_amount / 100;
										}
									}
								}else
								{	//��תծ������
									dValue = pBondNewInfo->share / 100 ;
								}
							}
							if(dValue > 0)
							{
								strResult.Format(_T("%.0f"), dValue);
							}
							break;
						case 285: //���ڹɱ�(B)
							{
								vdReq.data_type = dtype_int4;
								FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
								dValue = 0;
								if (pBusiness->m_pSecurity->IsStockB())
								{
									pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(reqTXFunction.nDate);
									if(pTxShareData != NULL)
										dValue = pTxShareData->TheShare;
								}
								else if (pBusiness->m_pSecurity->IsStockA())
								{
									//תΪB��
									mapIntInt.clear();
									Tx::Data::TypeMapManage::GetInstance()->GetTypeMapITI(TYPE_STOCK_TRANSOBJECT_AB, mapIntInt);
									iterIntInt = mapIntInt.find((INT)(reqTXFunction.nSecurityID));
									if(iterIntInt != mapIntInt.end())
									{
										if(pBusiness->GetSecurityNow(iterIntInt->second) != NULL)
										{
											pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(reqTXFunction.nDate);
											if(pTxShareData != NULL)
												dValue = pTxShareData->TheShare;
										}
									}
								}
								if (dValue > 0)
								{
									strResult.Format(_T("%.0f"), dValue);
								}
							}
							break;
						case 38:	//�ֹ�����
							vdReq.data_type = dtype_int4;
							if(tableIndID[7].GetRowCount() == 0)
							{
								//--------------�漰�����ļ����أ��ж�--------------------
								pBusiness->m_pLogicalBusiness->GetData(tableIndID[7], true);
								//--------------wangzhy----20080417----------------------
								//break;
							}
							dValue = 0;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							nInstitution = (INT)pBusiness->m_pSecurity->GetInstitutionId();
							vRow.clear();
							tableIndID[7].Find(0, nInstitution, vRow);
							if(vRow.size() == 0)
							{
								break;
							}
							for(i = 0; i < vRow.size(); i++)
							{
								tableIndID[7].GetCell(1, vRow[i], nDate);
								if(nDate > reqTXFunction.nDate)
								{
									break;
								}
							}
							if(i > 0)
							{
								tableIndID[7].GetCell(2, vRow[i - 1], dValue);
							}
							if(dValue > 0)
							{
								strResult.Format(_T("%.0f"), dValue);
							}
							break;
						case 39:	//���й�
						case 99:	//����ͨ��
							vdReq.data_type = dtype_int4;
							if(tableIndID[6].GetRowCount() == 0)
							{
								//--------------�漰�����ļ����أ��ж�--------------------
								pBusiness->m_pLogicalBusiness->GetData(tableIndID[6], true);
								//--------------wangzhy----20080417----------------------
								//break;
							}
							dValue = 0;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							nInstitution = (INT)pBusiness->m_pSecurity->GetInstitutionId();
							vRow.clear();
							tableIndID[6].Find(0, nInstitution, vRow);
							if(vRow.size() == 0)
							{
								break;
							}
							for(i = 0; i < vRow.size(); i++)
							{
								tableIndID[6].GetCell(1, vRow[i], nDate);
								if(nDate > reqTXFunction.nDate)
								{
									break;
								}
							}
							if(i > 0)
							{
								if(reqTXFunction.iFuncID == 99)
								{	//����ͨ��
									tableIndID[6].GetCell(2, vRow[i - 1], dValue);
								}else
								{	//���й�
									tableIndID[6].GetCell(3, vRow[i - 1], dValue);
								}
								
							}
							if(dValue >= 0)
							{
								strResult.Format(_T("%.0f"), dValue);
							}
							break;
						case 40:
						case 41:
						case 42:
						case 43:
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 13);	//������ȡ����񼾶ȡ��ϲ�/ĸ��˾������ǰ/��
							nInstitution = (INT)pBusiness->m_pSecurity->GetInstitutionId();
							iSubItemID = (int)reqTXFunction.iSubItemID-1;
							dValue = Tx::Core::Con_doubleInvalid;
							switch( reqTXFunction.iFuncID )
							{
							case 40:
								{
									pFinancial = NULL;
									if ( bFinancialInstitution )
									{
										nAccountingID = (INT64)reqTXFunction.nFYear * 100000 +  (INT64)reqTXFunction.nFQuarter* 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);
										pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_Financial,nAccountingID,&pFinancial,false);
									}
									else
									{
										nAccountingID = nInstitution * 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);
										//�����������ļ�	
										bool bLoaded = true;

										if(pFinancialDataFile!=NULL)
										{
											pFinancialDataFile->Reset();
											bLoaded = pFinancialDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30073,true);
										}
										if ( bLoaded )
											pFinancial = pFinancialDataFile->GetDataByObj(nAccountingID,false);
									}
									if ( pFinancial!=NULL )
									{
										dValue = pFinancial->dFinancial[iSubItemID];
										if ( iSubItemID==7||iSubItemID==8)
											dValue /= 100;
									}
									if(dValue != Tx::Core::Con_doubleInvalid && dValue != Tx::Core::Con_floatInvalid)
									{
										if(iSubItemID == 1)
											strResult.Format(_T("%.3f"),dValue);
										else
											strResult.Format(_T("%.2f"), dValue);
									}
								}								
								break;
							case 41:
								{
									pBalance = NULL;	
									if ( bBalanceInstitution )
									{
										nAccountingID = (INT64)reqTXFunction.nFYear * 100000 +  (INT64)reqTXFunction.nFQuarter* 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);
										pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_Balance,nAccountingID,&pBalance,false);
									}
									else
									{
										nAccountingID = nInstitution * 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);
										//�����������ļ�	
										bool bLoaded = true;

										if(pBalanceDataFile!=NULL)
										{	
											bLoaded = pBalanceDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30067,true);
											pBalanceDataFile->Reset();	
										}
										if ( bLoaded )
											pBalance = pBalanceDataFile->GetDataByObj(nAccountingID,false);
									}
									if ( pBalance!=NULL)
										dValue = pBalance->dBalance[iSubItemID];
									if(dValue != Tx::Core::Con_doubleInvalid && dValue != Tx::Core::Con_floatInvalid)
									{
										strResult.Format(_T("%.2f"), dValue);
									}
								}								
								break;
							case 43:
								{
									pCashFlow = NULL;	
									if ( bCashflowInstitution )
									{
										nAccountingID = (INT64)reqTXFunction.nFYear * 100000 +  (INT64)reqTXFunction.nFQuarter* 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);
										pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_CashFlow,nAccountingID,&pCashFlow,false);
									}
									else
									{
										nAccountingID = nInstitution * 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);
										//�����������ļ�	
										bool bLoaded = true;

										if(pCashFlowDataFile!=NULL)
										{	
											pCashFlowDataFile->Reset();
											bLoaded = pCashFlowDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30069,true);
										}
										if ( bLoaded )
											pCashFlow = pCashFlowDataFile->GetDataByObj(nAccountingID,false);
									}
									if ( pCashFlow!=NULL)
										dValue = pCashFlow->dCashFlow[iSubItemID];
									if(dValue != Tx::Core::Con_doubleInvalid && dValue != Tx::Core::Con_floatInvalid)
									{
										strResult.Format(_T("%.2f"), dValue);
									}
								}								
								break;
							case 42:
								{
									pIncome = NULL;	
									if ( bIncomeInstitution )
									{
										nAccountingID = (INT64)reqTXFunction.nFYear * 100000 +  (INT64)reqTXFunction.nFQuarter* 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);
										pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_Income,nAccountingID,&pIncome,false);
									}
									else
									{
										nAccountingID = nInstitution * 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);
										//�����������ļ�	
										bool bLoaded = true;
										//��ջ���

										if(pIncomeDataFile!=NULL)
										{
											bLoaded = pIncomeDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30071,true);
											pIncomeDataFile->Reset();
										}
										if ( bLoaded )
											pIncome = pIncomeDataFile->GetDataByObj(nAccountingID,false);
									}
									if ( pIncome!=NULL)
										dValue = pIncome->dIncome[iSubItemID];
									if(dValue != Tx::Core::Con_doubleInvalid && dValue != Tx::Core::Con_floatInvalid)
									{
										strResult.Format(_T("%.2f"), dValue);
									}
								}								
								break;
							}
							break;
						//------------------------wangzhy-----------------20080527------------------------------
						case 55:	//��Ҫ����ָ��������ָ��
							vdReq.data_type = dtype_val_string;
							if(tableIndID[2].GetRowCount() == 0)
							{
								//--------------�漰�����ļ����أ��ж�--------------------
								pBusiness->m_pLogicalBusiness->GetData(tableIndID[2], true);
								//--------------wangzhy----20080417----------------------
								//break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 13);	//������ȡ����񼾶ȡ��ϲ�/ĸ��˾������ǰ/��
							nInstitution = (INT)pBusiness->m_pSecurity->GetInstitutionId();
							nAccountingID = (INT64)nInstitution * 10000000 + (INT64)reqTXFunction.nFYear * 1000 + (INT64)reqTXFunction.nFQuarter * 100 + (INT64)reqTXFunction.iReportType * 10 + (INT64)reqTXFunction.iAccountingPolicyType;
							vRow.clear();
							tableIndID[2].Find(0, nAccountingID, vRow);
							if(vRow.size() != 1)
							{
								break;
							}
							switch(reqTXFunction.iSubItemID)
							{
							case 1:		//������
							case 2:		//���ʦ����
								tableIndID[2].GetCell(tableIndID[2].GetColCount() - 4 + (UINT)(reqTXFunction.iSubItemID), vRow[0], strValue);
								if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
								{
									strResult = strValue;
								}
								break;
							default:	//���������id
								tableIndID[2].GetCell(tableIndID[2].GetColCount() - 4 + (UINT)(reqTXFunction.iSubItemID), vRow[0], nInstitution);
								if(nInstitution > 0)
								{
									strResult = Tx::Data::TypeMapManage::GetInstance()->GetDatByID(TYPE_ACCOUNTING_FIRM, nInstitution);
								}
								break;
							}
							break;
						case 75:	//�����ʲ���ֵ	
							/*
							vdReq.data_type = dtype_double;
								if(tableIndID[0].GetRowCount() == 0)
								{
									//--------------�漰�����ļ����أ��ж�--------------------
									pBusiness->m_pLogicalBusiness->GetData(tableIndID[0], true);
									//--------------wangzhy----20080417----------------------
									//break;
								}
								FindItemOfReq(&reqTXFunction, strReq, 12);	//������ȡ����񼾶�
								nSecurity1ID = (INT)pBusiness->m_pSecurity->GetSecurity1Id();
								subtable_ID.Clear();
								subtable_ID.CopyColumnInfoFrom(tableIndID[0]);
								tableIndID[0].Between(subtable_ID, array_Col, 4, 0, nSecurity1ID, nSecurity1ID, true, true);
								if(subtable_ID.GetRowCount() == 0)
								{
									break;
								}
								subtable_Year.Clear();
								subtable_Year.CopyColumnInfoFrom(subtable_ID);
								subtable_ID.Between(subtable_Year, array_Col, 4, 1, reqTXFunction.nFYear, reqTXFunction.nFYear, true, true);
								if(subtable_Year.GetRowCount() == 0)
								{
									break;
								}
								subtable_Quarter.Clear();
								subtable_Quarter.CopyColumnInfoFrom(subtable_Year);
								subtable_Year.Between(subtable_Quarter, array_Col, 4, 2, 40040000 + reqTXFunction.nFQuarter, 40040000 + reqTXFunction.nFQuarter, true, true);
								if(subtable_Quarter.GetRowCount() != 1)
								{
									break;
								}
								subtable_Quarter.GetCell(3, 0, dValue);
								if(dValue != Tx::Core::Con_doubleInvalid && dValue != Tx::Core::Con_floatInvalid)
								{
									if(dValue != 0)
									{
										strResult.Format(_T("%.2f"), dValue);
									}
								}
								*/
							{
								vdReq.data_type = dtype_double;
								FindItemOfReq(&reqTXFunction, strReq, 3);
								//Mantis:13263    2012-10-24
								//�ʲ���ֵȡ���������ֱ����¶�ı��缾��ĩ�ʲ���ֵ����Ӧ������¶���ʲ���ֵ�����û����¶���ܵ��ʲ���ֵ�������ܷݶ�*��λ�ʲ���ֵ�����㡣 
								//---------------------------------------------------------------------------------------------------------------
								int nTemp = (int)((reqTXFunction.nDate % 10000) / 100) * 100 + (int)(reqTXFunction.nDate % 100);
								if (nTemp == 331 || nTemp == 630 || nTemp == 930 || nTemp == 1231)
								{
									if(nTemp == 331)
										nTemp = 1;
									else if(nTemp == 630)
										nTemp = 2;
									else if(nTemp == 930)
										nTemp = 4;
									else
										nTemp = 6;
									pFundInvesmentGroup = NULL;
									nAccountingID = (INT64)((reqTXFunction.nDate / 10000) * 10000) + (INT64)nTemp;
									pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_FundInvesmentGroup,nAccountingID,&pFundInvesmentGroup,false);

									if (pFundInvesmentGroup == NULL)
									{

										pTxFundShareData = pBusiness->m_pSecurity->GetTxFundShareDataByDate( reqTXFunction.nDate );
										if ( pTxFundShareData != NULL)
										{
											if( pBusiness->m_pSecurity->IsFund_Currency())
											{
												strResult.Format(_T("%.2f"),pTxFundShareData->TotalShare);
											}
											else
											{
												dValue = GetFundNvrVal( reqTXFunction.nSecurityID,reqTXFunction.nDate);
												if ( dValue != Con_doubleInvalid )
													strResult.Format(_T("%.2f"),pTxFundShareData->TotalShare*dValue);
											}
										}
									}
									else
									{
										dValue = pFundInvesmentGroup->dValue[9];
										if(dValue != Con_doubleInvalid)
											strResult.Format(_T("%.2f"),dValue);
									}
								}
								//-----------------------------------------------------------------------------------------------------------------
								else    //BUG:13606  / by wangzf  / 2012-11-09
								{
									pTxFundShareData = pBusiness->m_pSecurity->GetTxFundShareDataByDate( reqTXFunction.nDate );
									if ( pTxFundShareData != NULL)
									{
										if( pBusiness->m_pSecurity->IsFund_Currency())
										{
											strResult.Format(_T("%.2f"),pTxFundShareData->TotalShare);
										}
										else
										{
											dValue = GetFundNvrVal( reqTXFunction.nSecurityID,reqTXFunction.nDate);
											if ( dValue != Con_doubleInvalid )
											{
												CString sTemp = _T("");
												sTemp.Format(_T("%.4f"),dValue);
												dValue = atof(sTemp);
												strResult.Format(_T("%.2f"),pTxFundShareData->TotalShare*dValue);
											}
										}
									}
								}
							}
							break;
						case 108:	//��תծ-����
							vdReq.data_type = dtype_int4;
							switch(reqTXFunction.iSubItemID)
							{
							case 1:		//������
								iRowNo = GetDateIndicator_Row((INT)(reqTXFunction.nSecurityID), 0, &vBondIPOInfo);
								if(iRowNo >= 0)
								{
									//ծȯ������ʼ����
									tableIndDate[5].GetCell(6, iRowNo, nDate);
									if(nDate > 0)
									{
										//�޸�Ϊ8λ��--20080708--
										strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
										//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
									}
								}
								break;
							case 2:		//��Ϣ��
								pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
								if(pBondNewInfo != NULL)
								{
									nDate = pBondNewInfo->begin_date;
									//�޸�Ϊ8λ��--20080708--
									strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
									//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								}
								break;
							case 3:		//������
								nDate = pBusiness->m_pSecurity->GetIPOListedDate();
								if(nDate > 0)
								{
									//�޸�Ϊ8λ��--20080708--
									strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
									//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								}
								break;
							case 8:		//������
								pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
								if(pBondNewInfo != NULL)
								{
									nDate = pBondNewInfo->end_date;
									//�޸�Ϊ8λ��--20080708--
									strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
									//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								}
								break;
							default:
								if(tableIndID[1].GetRowCount() == 0)
								{
									//--------------�漰�����ļ����أ��ж�--------------------
									pBusiness->m_pLogicalBusiness->GetData(tableIndID[1], true);
									//--------------wangzhy----20080417----------------------
									//break;
								}
								nSecurity1ID = (INT)pBusiness->m_pSecurity->GetSecurity1Id();
								vRow.clear();
								tableIndID[1].Find(0, nSecurity1ID, vRow);
								if(vRow.size() != 1)
								{
									break;
								}
								switch(reqTXFunction.iSubItemID)
								{
								case 4:		//ת����ʼ��
									tableIndID[1].GetCell(2, vRow[0], nDate);
									break;
								case 5:		//���ۿ�ʼ��
									tableIndID[1].GetCell(5, vRow[0], nDate);
									break;
								case 6:		//�����ʼ��
									tableIndID[1].GetCell(4, vRow[0], nDate);
									break;
								default:	//ת��������
									tableIndID[1].GetCell(3, vRow[0], nDate);
									break;
								}
								if(nDate > 0)
								{
									//�޸�Ϊ8λ��--20080708--
									strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
									//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								}
								break;
							}
							break;
						case 111:	//��תծ������Ϣ
							{
								vdReq.data_type = dtype_val_string;
								strResult = _T("");
								if(reqTXFunction.iSubItemID == 2)
								{
									//m_tableBondTmp//������id
									if(m_tableBondTmp.GetRowCount() == 0)
									{
										//--------------�漰�����ļ����أ��ж�--------------------
										pBusiness->m_pLogicalBusiness->GetData(m_tableBondTmp, true);
									}
#ifdef _DEBUG
CString strTable = m_tableBondTmp.TableToString();
Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
									nSecurity1ID = (INT)pBusiness->m_pSecurity->GetSecurity1Id();
									vRow.clear();
									m_tableBondTmp.Find(0, nSecurity1ID, vRow);
									if(vRow.size() != 1)
									{
										break;
									}
									m_tableBondTmp.GetCell(1, vRow[0], strResult);
									break;
								}
								iRowNo = GetDateIndicator_Row((INT)(reqTXFunction.nSecurityID), 0, &vBondIPOInfo);
								if(iRowNo >= 0)
								{
									switch(reqTXFunction.iSubItemID)
									{
									case 1:		//���з�ʽ
										tableIndDate[5].GetCell(5, iRowNo, iVal);
										if(iVal > 0)
										{
											strResult = Tx::Data::TypeMapManage::GetInstance()->GetDatByID(TYPE_BOND_IPO_TYPE, iVal);
										}
										break;
									case 3:		//������
										tableIndDate[5].GetCell(11, iRowNo, strValue);
										if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
										{
											strResult = strValue;
										}
										break;
									case 4:		//������ʽ
										tableIndDate[5].GetCell(12, iRowNo, strValue);
										if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
										{
											strResult = strValue;
										}
										break;
									case 5:		//�����Ƽ���
										tableIndDate[5].GetCell(13, iRowNo, strValue);
										if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
										{
											strResult = strValue;
										}
										break;
									case 6:		//��������
										tableIndDate[5].GetCell(9, iRowNo, strValue);
										if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
										{
											strResult = strValue;
										}
										break;
									case 7:		//ծȯ���õȼ�
										tableIndDate[5].GetCell(8, iRowNo, strValue);
										if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
										{
											strResult = strValue;
										}
										break;
									case 8:		//������������
										tableIndDate[5].GetCell(10, iRowNo, strValue);
										if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
										{
											strResult = strValue;
										}
										break;
									case 9:
									case 10:
									case 11:
									case 12:
										{
											CString strPath = pBusiness->DownloadXmlFile( pBusiness->m_pSecurity->GetSecurity1Id(),11004);
											//CString strDataMean = _T("ծȯ_������Ϣ_��תծ");
											//Tx::Core::Table resTable;
											//int m_iSecId = pBusiness->m_pSecurity->GetId();
											//pBusiness->DownloadXmlFile(reqTXFunction.nSecurityID,11004);
											//CString strPath = Tx::Core::Manage::GetInstance()->m_pSystemPath->GetXMLPath() + _T("\\11004\\");
											//CString strFileName = _T("");
											//strFileName.Format(_T("%d.xml"),pBusiness->m_pSecurity->GetSecurity1Id());
											//strPath += strFileName;
											switch( reqTXFunction.iSubItemID )
											{	
											case 9:
												strResult = GetXmlData( strPath,_T("T42"));
												break;
											case 10:
												strResult = GetXmlData( strPath,_T("T43"));
												break;
											case 11:
												strResult = GetXmlData( strPath,_T("T44"));
												break;
											case 12:
												strResult = GetXmlData( strPath,_T("T45"));
												break;
											}	
										}
										break;
									default:
										break;
									}
								}
							}
							break;
							//vdReq.data_type = dtype_val_string;
							//if(reqTXFunction.iSubItemID == 2)
							//{	//������id
							//	if(tableIndID[1].GetRowCount() == 0)
							//	{
							//		//--------------�漰�����ļ����أ��ж�--------------------
							//		pBusiness->m_pLogicalBusiness->GetData(tableIndID[1], true);
							//		//--------------wangzhy----20080417----------------------
							//		//break;
							//	}
							//	nSecurity1ID = (INT)pBusiness->m_pSecurity->GetSecurity1Id();
							//	vRow.clear();
							//	tableIndID[1].Find(0, nSecurity1ID, vRow);
							//	if(vRow.size() != 1)
							//	{
							//		break;
							//	}
							//	tableIndID[1].GetCell(6, vRow[0], nInstitution);
							//	if(nInstitution > 0)
							//	{
							//		mapIntStr.clear();
							//		Tx::Data::TypeMapManage::GetInstance()->GetTypeMap(TYPE_INSTITUTION_CHINALONGNAME, mapIntStr);
							//		iterIntStr = mapIntStr.find(nInstitution);
							//		if(iterIntStr != mapIntStr.end())
							//		{
							//			strValue = iterIntStr->second;
							//			if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
							//			{
							//				strResult = strValue;
							//			}
							//		}
							//	}
							//	break;
							//}
							//iRowNo = GetDateIndicator_Row((INT)(reqTXFunction.nSecurityID), 0, &vBondIPOInfo);
							//if(iRowNo >= 0)
							//{
							//	switch(reqTXFunction.iSubItemID)
							//	{
							//	case 1:		//���з�ʽ
							//		tableIndDate[5].GetCell(5, iRowNo, iVal);
							//		if(iVal > 0)
							//		{
							//			strResult = Tx::Data::TypeMapManage::GetInstance()->GetDatByID(TYPE_BOND_IPO_TYPE, iVal);
							//		}
							//		break;
							//	case 3:		//������
							//		tableIndDate[5].GetCell(11, iRowNo, strValue);
							//		if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
							//		{
							//			strResult = strValue;
							//		}
							//		break;
							//	case 4:		//������ʽ
							//		tableIndDate[5].GetCell(12, iRowNo, strValue);
							//		if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
							//		{
							//			strResult = strValue;
							//		}
							//		break;
							//	case 5:		//�����Ƽ���
							//		tableIndDate[5].GetCell(13, iRowNo, strValue);
							//		if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
							//		{
							//			strResult = strValue;
							//		}
							//		break;
							//	case 6:		//��������
							//		tableIndDate[5].GetCell(9, iRowNo, strValue);
							//		if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
							//		{
							//			strResult = strValue;
							//		}
							//		break;
							//	case 7:		//ծȯ���õȼ�
							//		tableIndDate[5].GetCell(8, iRowNo, strValue);
							//		if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
							//		{
							//			strResult = strValue;
							//		}
							//		break;
							//	case 8:		//������������
							//		tableIndDate[5].GetCell(10, iRowNo, strValue);
							//		if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
							//		{
							//			strResult = strValue;
							//		}
							//		break;
							//	case 9:
							//	case 10:
							//	case 11:
							//	case 12:
							//		{
							//			CString strPath = pBusiness->DownloadXmlFile( pBusiness->m_pSecurity->GetSecurity1Id(),11004);
							//			//CString strDataMean = _T("ծȯ_������Ϣ_��תծ");
							//			//Tx::Core::Table resTable;
							//			//int m_iSecId = pBusiness->m_pSecurity->GetId();
							//			//pBusiness->DownloadXmlFile(reqTXFunction.nSecurityID,11004);
							//			//CString strPath = Tx::Core::Manage::GetInstance()->m_pSystemPath->GetXMLPath() + _T("\\11004\\");
							//			//CString strFileName = _T("");
							//			//strFileName.Format(_T("%d.xml"),pBusiness->m_pSecurity->GetSecurity1Id());
							//			//strPath += strFileName;
							//			switch( reqTXFunction.iSubItemID )
							//			{	
							//			case 9:
							//				strResult = GetXmlData( strPath,_T("T42"));
							//				break;
							//			case 10:
							//				strResult = GetXmlData( strPath,_T("T43"));
							//				break;
							//			case 11:
							//				strResult = GetXmlData( strPath,_T("T44"));
							//				break;
							//			case 12:
							//				strResult = GetXmlData( strPath,_T("T45"));
							//				break;
							//			}	
							//		}
							//		break;
							//	default:
							//		break;
							//	}
							//}
							//break;
						case 112:	//��תծ��Ӧ��Ʊ����
							vdReq.data_type = dtype_val_string;
							if(tableIndID[1].GetRowCount() == 0)
							{
								//--------------�漰�����ļ����أ��ж�--------------------
								pBusiness->m_pLogicalBusiness->GetData(tableIndID[1], true);
								//--------------wangzhy----20080417----------------------
								//break;
							}
							nSecurity1ID = (INT)pBusiness->m_pSecurity->GetSecurity1Id();
							vRow.clear();
							tableIndID[1].Find(0, nSecurity1ID, vRow);
							if(vRow.size() > 0)
							{
								tableIndID[1].GetCell(1, vRow[0], iTransObjID);
								if(iTransObjID > 0)
								{
									pSecurity = (Tx::Data::SecurityQuotation *)GetSecurity((LONG)iTransObjID);
									if(pSecurity != NULL)
									{
										strResult = pSecurity->GetCode(true);
									}
								}
							}
							break;
						//--wangzhy--20080604
						case 44:	//��Ӫҵ������ҵ��ֲ�������
						case 45:	//��Ӫҵ������ҵ��ֲ�����Ŀ����
						case 46:	//�ʲ���ֵ׼��������
						case 47:	//�Ǿ������������Ŀ����
						case 48:	//�Ǿ������������Ŀ���
						case 49:	//Ӧ���ʿ������
						case 50:	//Ӧ���ʿ������
						case 51:	//������ñ���Ŀ����
						case 52:	//������ñ���Ŀ���
							FindItemOfReq(&reqTXFunction, strReq, 14);	//FuncID��SubItemID��IndicatorID��������ȡ��Ƽ����ȡ���Ŀ���
							nInstitution = (INT)pBusiness->m_pSecurity->GetInstitutionId();
							switch( reqTXFunction.iFuncID )
							{
							case 44:	//��Ӫҵ������ҵ��ֲ�������
							case 45:	//��Ӫҵ������ҵ��ֲ�����Ŀ����
								if ( reqTXFunction.iFuncID == 45 )
									vdReq.data_type = dtype_val_string;
								else
									vdReq.data_type = dtype_double;

								//--������ƴ������Ҫ�����ݵ�����
								//nAccountingID = (INT64)reqTXFunction.nFYear * 1000000 + (INT64)reqTXFunction.nFQuarter * 100 + (INT64)reqTXFunction.iSubjectID;
								pRevenue = NULL;
								strResult = Con_strInvalid;
								if ( bRevenueInstitution )
								{
									nAccountingID = (INT64)reqTXFunction.nFYear * 1000000 + (INT64)reqTXFunction.nFQuarter * 100 + (INT64)reqTXFunction.iSubjectID;
									pBusiness->m_pSecurity->GetDataByObj( dt_PrimeOperatingRevenue1,nAccountingID,&pRevenue,false );
								}
								else
								{
									nAccountingID = nInstitution * 100 + (INT64)reqTXFunction.iSubjectID;
									//�����������ļ�	
									bool bLoaded = true;
									if(pRevenueDataFile!=NULL)
										bLoaded = pRevenueDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30079,true);
									if ( bLoaded )
										pRevenue = pRevenueDataFile->GetDataByObj(nAccountingID,false);
								}

								if ( pRevenue == NULL )
									break;
								if ( reqTXFunction.iFuncID == 45 )
									strResult = pRevenue->bName;
								else
								{
									double dValue = 0.0;
									double dSumValue = 0.0;
									switch( reqTXFunction.iSubItemID )
									{
									case 1:
										if ( pRevenue->dSR != Con_doubleInvalid)
											strResult.Format(_T("%.2f"),pRevenue->dSR);
										break;
									case 2:
										dValue = pRevenue->dSR;
										if ( bRevenueInstitution )
										{
											nAccountingID = (INT64)reqTXFunction.nFYear * 1000000 + (INT64)reqTXFunction.nFQuarter * 100 + 1;
											pBusiness->m_pSecurity->GetDataByObj( dt_PrimeOperatingRevenue1,nAccountingID,&pRevenue,false );
											while ( pRevenue->iDate / 100 == ((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter ))
											{
												dSumValue += pRevenue->dSR;
												pRevenue++;
											}
										}
										else
										{
											nAccountingID = nInstitution * 100 + 1;
											bool bLoaded = true;
											if(pRevenueDataFile!=NULL)
												bLoaded = pRevenueDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30079,true);
											if ( bLoaded )
											{
												pRevenue = pRevenueDataFile->GetDataByObj(nAccountingID,false);
												while ( pRevenue->iDate / 100 == nInstitution )
												{
													dSumValue += pRevenue->dSR;
													pRevenue++;
												}
											}	

										}
										strResult.Format(_T("%.2f"),dValue*100/dSumValue);
										break;
									case 3:
										if ( pRevenue->dCB != Con_doubleInvalid)
											strResult.Format(_T("%.2f"),pRevenue->dCB);
										break;
									case 4:
										if ( pRevenue->dML != Con_doubleInvalid)
											strResult.Format(_T("%.2f"),pRevenue->dML);
										break;
									case 5:
										dValue = pRevenue->dML;
										if ( bRevenueInstitution)
										{
											nAccountingID = (INT64)reqTXFunction.nFYear * 1000000 + (INT64)reqTXFunction.nFQuarter * 100 + 1;
											pBusiness->m_pSecurity->GetDataByObj( dt_PrimeOperatingRevenue1,nAccountingID,&pRevenue,false );
											while ( pRevenue->iDate / 100 == ((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter ))
											{
												dSumValue += pRevenue->dML;
												pRevenue++;
											}
										}
										else
										{
											nAccountingID = nInstitution * 100 + 1;
											bool bLoaded = true;
											if(pRevenueDataFile!=NULL)
												bLoaded = pRevenueDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30079,true);
											if ( bLoaded )
											{
												pRevenue = pRevenueDataFile->GetDataByObj(nAccountingID,false);
												while ( pRevenue->iDate / 100 == nInstitution )
												{
													dSumValue += pRevenue->dML;
													pRevenue++;
												}
											}	
										}
										strResult.Format(_T("%.2f"),dValue*100/dSumValue);
										break;
									case 6:
										if ( pRevenue->dMLL != Con_doubleInvalid)
											strResult.Format(_T("%.2f"),pRevenue->dMLL);
										break;
									case 7:
										if ( pRevenue->dMLLBH != Con_doubleInvalid)
											strResult.Format(_T("%.2f"),pRevenue->dMLLBH);
										break;
									default:
										break;
									}
								}
								break;
							case 46:	//�ʲ���ֵ׼��������
								vdReq.data_type = dtype_double;
								strResult = Con_strInvalid;
								//nAccountingID = (INT64)reqTXFunction.nFYear * 1000000 + (INT64)reqTXFunction.nFQuarter*100 + (INT64)reqTXFunction.iSubItemID;
								iSubItemID = (int)reqTXFunction.iSubItemID-1;
								pDepreciation = NULL;
								//strResult = Con_strInvalid;
								if( bDepreciationInstitution )
								{
									nAccountingID = (INT64)reqTXFunction.nFYear * 1000000 + (INT64)reqTXFunction.nFQuarter*100 + (INT64)reqTXFunction.iSubjectID;
									pBusiness->m_pSecurity->GetDataByObj( dt_AssetsDepreciationReserves1,nAccountingID,&pDepreciation,false );	
								}
								else
								{
									nAccountingID = nInstitution * 100 + (INT64)reqTXFunction.iSubjectID;
									//�����������ļ�	
									bool bLoaded = true;
									if(pDepreciationDataFile!=NULL)
										bLoaded = pDepreciationDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30081,true);
									if ( bLoaded )
										pDepreciation = pDepreciationDataFile->GetDataByObj(nAccountingID,false);
									
								}
								if ( pDepreciation == NULL )
									break;
								if ( pDepreciation->dAsset[iSubItemID] != Con_doubleInvalid )
									strResult.Format(_T("%.2f"),pDepreciation->dAsset[iSubItemID]);
								break;
							case 47:	//�Ǿ������������Ŀ����
							case 48:	//�Ǿ������������Ŀ���
								if ( reqTXFunction.iFuncID == 47 )
									vdReq.data_type = dtype_val_string;
								else
									vdReq.data_type = dtype_double;
								//--������ƴ������Ҫ�����ݵ�����
								//nAccountingID = (INT64)reqTXFunction.nFYear * 1000000 + (INT64)reqTXFunction.nFQuarter * 100 + (INT64)reqTXFunction.iSubjectID;
								//iSubItemID = (int)reqTXFunction.iSubItemID-1;
								pGainsAndLosses = NULL;
								//strResult = Con_strInvalid;
								if ( bGainsAndLossesInstitution )
								{
									nAccountingID = (INT64)reqTXFunction.nFYear * 1000000 + (INT64)reqTXFunction.nFQuarter * 100 + (INT64)reqTXFunction.iSubjectID;
									pBusiness->m_pSecurity->GetDataByObj( dt_NonRecurringGainsAndLosses,nAccountingID,&pGainsAndLosses,false );
								}
								else
								{	
									nAccountingID = nInstitution * 100 + (INT64)reqTXFunction.iSubjectID;
									bool bLoaded = true;
									if(pGainsAndLossesDataFile!=NULL)
										bLoaded = pGainsAndLossesDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30086,true);
									if ( bLoaded )
										pGainsAndLosses = pGainsAndLossesDataFile->GetDataByObj(nAccountingID,false);
								}
								if ( pGainsAndLosses == NULL )
									break;
								if ( reqTXFunction.iFuncID == 47 )
									strResult = pGainsAndLosses->bName;
								else
									strResult.Format(_T("%.2f"),pGainsAndLosses->dLosses);
								break;
							case 49:	//Ӧ���ʿ������
							case 50:	//Ӧ���ʿ������
								if ( reqTXFunction.iFuncID == 49 )
									vdReq.data_type = dtype_val_string;
								else
									vdReq.data_type = dtype_double;
								iSubItemID = (int)reqTXFunction.iSubItemID-1;
								pAccountReceivable = NULL;
								//strResult = Con_strInvalid;
								if( bAccountInstitution )
								{
									nAccountingID = (INT64)reqTXFunction.nFYear * 1000000 + (INT64)reqTXFunction.nFQuarter * 100 + (INT64)reqTXFunction.iSubjectID;
									pBusiness->m_pSecurity->GetDataByObj( dt_AccountsReceivable,nAccountingID,&pAccountReceivable,false );
								}
								else
								{
									nAccountingID = nInstitution * 100 + (INT64)reqTXFunction.iSubjectID;
									bool bLoaded = true;
									if(pAccountsReceivableDataFile!=NULL)
										bLoaded = pAccountsReceivableDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30087,true);
									if ( bLoaded )
										pAccountReceivable = pAccountsReceivableDataFile->GetDataByObj(nAccountingID,false);
								}
								if ( pAccountReceivable == NULL )
									break;
								if ( reqTXFunction.iFuncID == 49 )
									strResult = pAccountReceivable->bName;
								else
									if ( pAccountReceivable->dAccountsReceivable[iSubItemID+4] != Con_doubleInvalid )
										strResult.Format(_T("%.2f"),pAccountReceivable->dAccountsReceivable[iSubItemID+4]);
								break;
							case 51:	//������ñ���Ŀ����
							case 52:	//������ñ���Ŀ���
								if ( reqTXFunction.iFuncID == 51 )
									vdReq.data_type = dtype_val_string;
								else
									vdReq.data_type = dtype_double;
								iSubItemID = (int)reqTXFunction.iSubItemID;
								pFinanceCharge = NULL;	
								//strResult = Con_strInvalid;
								if ( bFinanceChargeInstitution )
								{	
									nAccountingID = (INT64)reqTXFunction.nFYear * 1000000 + (INT64)reqTXFunction.nFQuarter * 100 + (INT64)reqTXFunction.iSubjectID;
									pBusiness->m_pSecurity->GetDataByObj( dt_FinanceCharge,nAccountingID,&pFinanceCharge,false );	
								}
								else
								{
									nAccountingID = nInstitution * 100 + (INT64)reqTXFunction.iSubjectID;
									bool bLoaded = true;
									if(pFinanceChargeDataFile!=NULL)
										bLoaded = pFinanceChargeDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30088,true);
									if ( bLoaded )
										pFinanceCharge = pFinanceChargeDataFile->GetDataByObj(nAccountingID,false);
								}
								if ( pFinanceCharge == NULL )
									break;
								if ( reqTXFunction.iFuncID == 51 )
									strResult = pFinanceCharge->bName;
								else
								{
									if ( pFinanceCharge->dFinanceCharge[iSubItemID] != Con_doubleInvalid )
										strResult.Format(_T("%.2f"),pFinanceCharge->dFinanceCharge[iSubItemID]);
								}
								break;
							default:
								break;
							}
							break;
						case 117:	//ָ������
							vdReq.data_type = dtype_val_string;
							FindItemOfReq(&reqTXFunction, strReq, 4);	//ָ�����롢��������
							if(reqTXFunction.iINDEXSampleIdx == 0)
							{
								strResult.Format(_T("%d"), pBusiness->m_pSecurity->GetIndexConstituentDataCount());
							}else
							{
								pIndexConstituentData = pBusiness->m_pSecurity->GetIndexConstituentDataByIndex(reqTXFunction.iINDEXSampleIdx - 1);
								if(pIndexConstituentData != NULL && pIndexConstituentData->iSecurityId > 0&&pBusiness->m_pSecurity->GetIndexConstituentDataCount() >= reqTXFunction.iINDEXSampleIdx)
								{
									pSecurity = (Tx::Data::SecurityQuotation *)GetSecurity((LONG)(pIndexConstituentData->iSecurityId));
									if(pSecurity != NULL)
									{
										strResult = pSecurity->GetCode();
									}
								}
							}
							break;
						case 118:	//��Ʊ��֤�����ҵ����
							vdReq.data_type = dtype_val_string;
							FindItemOfReq(&reqTXFunction, strReq, 22);	//��ҵ�㼶
							nIndustryID = pBusiness->m_pSecurity->GetCSRC_IndustryId();
							iVal = 0;
							iIndexObjID = 0;
							
							//bug:14343  ʹ��excel������ȡ֤�����ҵ����ʱ����ҵ�����д��1���᲻������  2013-01-15
							if (reqTXFunction.nIndustryLevel==1)
							{
								reqTXFunction.nIndustryLevel = 2;
							}
							while(nIndustryID > 0)
							{
								iTransObjID = iIndexObjID;
								iIndexObjID = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDEX_INDUSTRY, (INT)nIndustryID);
								if(iIndexObjID < 0)
								{
									iIndexObjID = iTransObjID;
								}
								if(reqTXFunction.nIndustryLevel > 0 && reqTXFunction.nIndustryLevel == iVal + 1)
								{
									break;
								}
								iVal ++;
								nIndustryID = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRY_PARENT, (INT)nIndustryID);
							}
							if(iIndexObjID > 0)
							{
								pSecurity = (Tx::Data::SecurityQuotation *)GetSecurity((LONG)iIndexObjID);
								if(pSecurity != NULL)
								{
									strResult = pSecurity->GetCode();
								}
							}
							break;
						case 119:	//��Ʊ��������ҵ����
							vdReq.data_type = dtype_val_string;
							FindItemOfReq(&reqTXFunction, strReq, 22);	//��ҵ�㼶
							nIndustryID = pBusiness->m_pSecurity->GetTxSec_IndustryId();
							iVal = 0;
							iIndexObjID = 0;
							while(nIndustryID > 0)
							{
								iTransObjID = iIndexObjID;
								iIndexObjID = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDEX_INDUSTRY, (INT)nIndustryID);
								if(iIndexObjID < 0)
								{
									iIndexObjID = iTransObjID;
								}
								if(reqTXFunction.nIndustryLevel > 0 && reqTXFunction.nIndustryLevel == iVal + 1)
								{
									break;
								}
								iVal ++;
								nIndustryID = TypeMapManage::GetInstance()->GetValueByIDITI(TYPE_INDUSTRY_PARENT, (INT)nIndustryID);
							}
							if(iIndexObjID > 0)
							{
								pSecurity = (Tx::Data::SecurityQuotation *)GetSecurity((LONG)iIndexObjID);
								if(pSecurity != NULL)
								{
									strResult = pSecurity->GetCode();
								}
							}
							break;
						case 120:	//�����ھ���
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 5);	//StartDate��EndDate
							dValue1 = GetSumAmount(pBusiness, reqTXFunction.nStartDate, reqTXFunction.nEndDate);
							dValue2 = GetSumVolume(pBusiness, reqTXFunction.nStartDate, reqTXFunction.nEndDate);
							if(dValue1 > 0 && dValue2 > 0)
							{
								strResult.Format(_T("%.2f"), dValue1 / dValue2);
							}
							break;
						case 121:	//��������ȷֺ��ܶ�
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 21);	//StartDate��EndDate���������
							dValue = 0;
							if(pBusiness->m_pSecurity->IsStock())
							{	//��Ʊ�ֺ�
								for(i = 0; (INT)i < pBusiness->m_pSecurity->GetStockBonusDataCount(); i++)
								{
									pStockBonusData = pBusiness->m_pSecurity->GetStockBonusDataByIndex((INT)i);
									if(reqTXFunction.nStartDate <= pStockBonusData->register_date && 
										reqTXFunction.nEndDate >= pStockBonusData->register_date)
									{
										if(reqTXFunction.nFYear > 0 && reqTXFunction.nFYear != (pStockBonusData->year / 10000))
										{
											continue;
										}
										pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(pStockBonusData->register_date);
										if(pTxShareData != NULL)
										{
											dValue1 = pTxShareData->TotalShare;
										}else
										{
											dValue1 = 0;
										}
										dValue2 = pStockBonusData->pxshu;
										if(pStockBonusData->pxshu > 0)
										{
											dValue2 = pStockBonusData->pxshu;
										}else
										{
											dValue2 = 0;
										}
										dValue += dValue1 * dValue2;
									}
								}
							}else if(pBusiness->m_pSecurity->IsFund())
							{	//����ֺ�
								for(i = 0; (INT)i < pBusiness->m_pSecurity->GetFundBonusDataCount(); i++)
								{
									pFundBonusData = pBusiness->m_pSecurity->GetFundBonusDataByIndex((INT)i);
									if(reqTXFunction.nStartDate <= pFundBonusData->book_date && 
										reqTXFunction.nEndDate >= pFundBonusData->book_date)
									{
										if(reqTXFunction.nFYear > 0 && reqTXFunction.nFYear != pFundBonusData->year)
										{
											continue;
										}
										if(pFundBonusData->fact > 0)
										{
											dValue += pFundBonusData->fact;
										}
									}
								}
							}
							if(dValue > 0)
							{
								strResult.Format(_T("%.2f"), dValue);
							}
							break;
						case 135:	//�����������Ϣ�ܶ�
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 21);	//StartDate��EndDate���������
							dValue = 0;
							if(pBusiness->m_pSecurity->IsBond())
							{
								pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
								if(pBondNewInfo != NULL && pBondNewInfo->share > 0)
								{
									for(i = 0; (INT)i < pBusiness->m_pSecurity->GetBondCashFlowDataCount(); i++)
									{
										pBondCashFlowData = pBusiness->m_pSecurity->GetBondCashFlowDataByIndex((INT)i);
										if(reqTXFunction.nStartDate <= pBondCashFlowData->F_BOND_BOOK_DATE && 
											reqTXFunction.nEndDate >= pBondCashFlowData->F_BOND_BOOK_DATE)
										{
											if(reqTXFunction.nFYear > 0 && reqTXFunction.nFYear != pBondCashFlowData->F_YEAR)
											{
												continue;
											}
											if(pBondCashFlowData->F_CASH > 0)
											{
												dValue += pBondNewInfo->share * pBondCashFlowData->F_CASH / 100;
											}
										}
									}
								}
							}
							if(dValue > 0)
							{
								strResult.Format(_T("%.2f"), dValue);
							}
							break;
						case 133:	//�����ڷֺ��ܶ�
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 5);	//StartDate��EndDate
							dValue = 0;
							if(pBusiness->m_pSecurity->IsStock())
							{	//��Ʊ�ֺ�
								for(i = 0; (INT)i < pBusiness->m_pSecurity->GetStockBonusDataCount(); i++)
								{
									pStockBonusData = pBusiness->m_pSecurity->GetStockBonusDataByIndex((INT)i);
									if(reqTXFunction.nStartDate <= pStockBonusData->register_date && 
										reqTXFunction.nEndDate >= pStockBonusData->register_date)
									{
										pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(pStockBonusData->register_date);
										if(pTxShareData != NULL)
										{
											dValue1 = pTxShareData->TotalShare;
										}else
										{
											dValue1 = 0;
										}
										if(pStockBonusData->pxshu > 0)
										{
											dValue2 = pStockBonusData->pxshu;
										}else
										{
											dValue2 = 0;
										}
										dValue += dValue1 * dValue2;
									}
								}
							}else if(pBusiness->m_pSecurity->IsFund())  //����ֺ�
							{	
								int iLen = strReq.GetLength();
								CString strTemp, strTemp1, strTemp2;
								int iDirc = strReq.ReverseFind(';');
								strTemp = strReq.Right( iLen - iDirc - 1 );
								strTemp1 = strReq.Left( iDirc );
								int iLen1 = strTemp1.GetLength();
								int iCount1 = strTemp1.ReverseFind(';');
								strTemp2 = strTemp1.Right( iLen1 - iCount1 - 1 ) ;
								int iEtime = atoi( strTemp );     // end
								int iBtime = atoi( strTemp2 );    // begin

								std::vector<double> vecDVal;
								std::vector<int> vecTemp;
								vecTemp.clear();
								TxFund fund;
								double iNum;
		
		 					    fund.GetReportDateArr(vecTemp, iBtime, iEtime);
								int iRQtemp = vecTemp.at( vecTemp.size() -1 );   
								int iRetCount = vecTemp.size();

								//int iCount = pBusiness->m_pSecurity->GetFundBonusDataCount();
                                
								for(i = 0; (INT)i < pBusiness->m_pSecurity->GetFundBonusDataCount(); i++)
								{
									// by wanglm Excel ���� ����ֺ�
									TxFundShareData *str_FundData = NULL;    

									str_FundData   = pBusiness->m_pSecurity->GetTxFundShareDataByDate( iRQtemp );  
									pFundBonusData = pBusiness->m_pSecurity->GetFundBonusDataByIndex((INT)i);
                                    vecDVal.push_back( pFundBonusData->numerator );
																		                                    
									//if(reqTXFunction.nStartDate <= pFundBonusData->book_date && reqTXFunction.nEndDate >= pFundBonusData->book_date)
									//{
										//if(pFundBonusData->fact > 0)    // pFundBonusData->fact 
										//{
										//	dValue += pFundBonusData->fact;     //  fact
										//}
                                        //////dValue = str_FundData->TotalShare * pFundBonusData->numerator / pFundBonusData->denominator / 10000;
									//}.
									if ( pFundBonusData->divid_date <= iEtime )
									{
	                                  dValue = str_FundData->TotalShare * pFundBonusData->numerator / pFundBonusData->denominator / 10000;  
									   iNum = pFundBonusData->numerator;
									}

									else
									{
										dValue = str_FundData->TotalShare * iNum / pFundBonusData->denominator / 10000;  
									}

								}
							}
							if(dValue > 0)
							{
								strResult.Format(_T("%.2f"), dValue);
							}
							break;
						case 136:	//��������Ϣ�ܶ�
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 5);	//StartDate��EndDate
							dValue = 0;
							if(pBusiness->m_pSecurity->IsBond())
							{
								pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
								if(pBondNewInfo != NULL && pBondNewInfo->share > 0)
								{
									for(i = 0; (INT)i < pBusiness->m_pSecurity->GetBondCashFlowDataCount(); i++)
									{
										pBondCashFlowData = pBusiness->m_pSecurity->GetBondCashFlowDataByIndex((INT)i);
										if(reqTXFunction.nStartDate <= pBondCashFlowData->F_BOND_BOOK_DATE && 
											reqTXFunction.nEndDate >= pBondCashFlowData->F_BOND_BOOK_DATE)
										{
											if(pBondCashFlowData->F_CASH > 0)
											{
												dValue += pBondNewInfo->share * pBondCashFlowData->F_CASH / 100;
											}
										}
									}
								}
							}
							if(dValue > 0)
							{
								strResult.Format(_T("%.2f"), dValue);
							}
							break;
						case 134:	//��ȷֺ��ܶ�
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 20);	//�������
							dValue = 0;
							if(pBusiness->m_pSecurity->IsStock())
							{	//��Ʊ�ֺ�
								for(i = 0; (INT)i < pBusiness->m_pSecurity->GetStockBonusDataCount(); i++)
								{
									pStockBonusData = pBusiness->m_pSecurity->GetStockBonusDataByIndex((INT)i);
									if(pStockBonusData->register_date <= 0)
									{
										continue;
									}
									if(reqTXFunction.nFYear != (pStockBonusData->year / 10000))
									{
										continue;
									}
									pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(pStockBonusData->register_date);
									if(pTxShareData != NULL)
									{
										dValue1 = pTxShareData->TotalShare;
									}else
									{
										dValue1 = 0;
									}
									if(pStockBonusData->pxshu > 0)
									{
										dValue2 = pStockBonusData->pxshu;
									}else
									{
										dValue2 = 0;
									}
									dValue += dValue1 * dValue2;
								}
							}else if(pBusiness->m_pSecurity->IsFund())
							{	//����ֺ�
								for(i = 0; (INT)i < pBusiness->m_pSecurity->GetFundBonusDataCount(); i++)
								{
									pFundBonusData = pBusiness->m_pSecurity->GetFundBonusDataByIndex((INT)i);
									if(pFundBonusData->book_date <= 0)
									{
										continue;
									}
									if(reqTXFunction.nFYear != pFundBonusData->year)
									{
										continue;
									}
									if(pFundBonusData->fact > 0)
									{
										dValue += pFundBonusData->fact;
									}
								}
							}
							if(dValue > 0)
							{
								strResult.Format(_T("%.2f"), dValue);
							}
							break;
						case 137:	//�����Ϣ�ܶ�
							vdReq.data_type = dtype_double;
							if(!pBusiness->m_pSecurity->IsBond())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 20);	//�������
							dValue = 0;
							if(pBusiness->m_pSecurity->IsBond())
							{
								pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
								if(pBondNewInfo != NULL && pBondNewInfo->share > 0)
								{
									for(i = 0; (INT)i < pBusiness->m_pSecurity->GetBondCashFlowDataCount(); i++)
									{
										pBondCashFlowData = pBusiness->m_pSecurity->GetBondCashFlowDataByIndex((INT)i);
										if(pBondCashFlowData->F_BOND_BOOK_DATE <= 0)
										{
											continue;
										}
										if(reqTXFunction.nFYear != pBondCashFlowData->F_YEAR)
										{
											continue;
										}
										if(pBondCashFlowData->F_CASH > 0)
										{
											dValue += pBondNewInfo->share * pBondCashFlowData->F_CASH / 100;
										}
									}
								}
							}
							if(dValue > 0)
							{
								strResult.Format(_T("%.2f"), dValue);
							}
							break;
						case 62:	//Ȩ֤-����������
							vdReq.data_type = dtype_double;
							if(!pBusiness->m_pSecurity->IsWarrant())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							iTransObjID = warrant.GetObjectSecurityId((INT)(reqTXFunction.nSecurityID));
							nStartDate = pBusiness->m_pSecurity->GetIPOListedDate();
							nEndDate = warrant.GetPowerEndDate((INT)(reqTXFunction.nSecurityID));
							if(iTransObjID > 0 && nStartDate > 0 && nEndDate > 0 && reqTXFunction.nDate >= nStartDate && reqTXFunction.nDate <= nEndDate)
							{
								business.GetSecurityNow((LONG)iTransObjID);
								dValue1 = GetKLine(&business, reqTXFunction.nDate, 4,reqTXFunction.nSecurityID);
								Tx::Business::ComputingWarrant computingWarrant;
								dValue2 = computingWarrant.GetLastedPeriod( reqTXFunction.nSecurityID,reqTXFunction.nDate );
								//dValue2 = (DOUBLE)(warrant.GetRemainsDays((INT)(reqTXFunction.nSecurityID), reqTXFunction.nDate)) / 365;
								//Ȩ֤�۸�
								dValue3 = pBusiness->m_pSecurity->GetClosePrice( reqTXFunction.nDate );
								//��ǰ��Ȩ����
								double dDivid = 1.0;
								int nNow = COleDateTime::GetCurrentTime().GetYear()*10000 + COleDateTime::GetCurrentTime().GetMonth()*100 + COleDateTime::GetCurrentTime().GetDay();
								dDivid = business.m_pSecurity->GetExdividendScale( reqTXFunction.nDate,nNow,true );
								double dExRatio = warrant.GetPowerRatio( reqTXFunction.nSecurityID );
								dExRatio *= dDivid;
								double dMarketValue = dValue3 / dExRatio;
								dValue = warrant.GetSigma(!pBusiness->m_pSecurity->IsWarrant_Buy(), dValue1, warrant.GetPowerPrice((INT)(reqTXFunction.nSecurityID)), 0.018, 0.0, dValue2, dMarketValue);
								strResult.Format(_T("%.3f"), dValue);
								//pTxShareData = pBusiness->m_pSecurity->GetTxShareDataByDate(reqTXFunction.nDate);
								//if(pTxShareData != NULL)
								//{
								//	dValue3 = GetKLine(pBusiness, reqTXFunction.nDate, 4) * pTxShareData->TotalShare;
								//	dValue = warrant.GetSigma(pBusiness->m_pSecurity->IsWarrant_Buy(), dValue1, warrant.GetPowerPrice((INT)(reqTXFunction.nSecurityID)), 0.018, 0.0, dValue2, dValue3);
								//	strResult.Format(_T("%.3f"), dValue);
								//}
							}
							break;
						case 63:	//Ȩ֤-�����
							vdReq.data_type = dtype_double;
							if(!pBusiness->m_pSecurity->IsWarrant())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							iTransObjID = warrant.GetObjectSecurityId((INT)(reqTXFunction.nSecurityID));
							nStartDate = pBusiness->m_pSecurity->GetIPOListedDate();
							nEndDate = warrant.GetPowerEndDate((INT)(reqTXFunction.nSecurityID));
							if(iTransObjID > 0 && nStartDate > 0 && nEndDate > 0 && reqTXFunction.nDate >= nStartDate && reqTXFunction.nDate <= nEndDate)
							{
								business.GetSecurityNow((LONG)iTransObjID);
								dValue1 = GetKLine(pBusiness, reqTXFunction.nDate, 4,reqTXFunction.nSecurityID);
								dValue2 = GetKLine(&business, reqTXFunction.nDate, 4,reqTXFunction.nSecurityID);
								dValue = warrant.GetPremiumRate(dValue1, dValue2, 
																warrant.GetPowerPrice((INT)(reqTXFunction.nSecurityID)),
																pBusiness->m_pSecurity->IsWarrant_Buy(), 
																warrant.GetPowerRatio((INT)(reqTXFunction.nSecurityID)));
								strResult.Format(_T("%.2f"), dValue);
							}
							break;
						case 64:	//Ȩ֤-�ܸ˱���
							vdReq.data_type = dtype_double;
							if(!pBusiness->m_pSecurity->IsWarrant())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							iTransObjID = warrant.GetObjectSecurityId((INT)(reqTXFunction.nSecurityID));
							nStartDate = pBusiness->m_pSecurity->GetIPOListedDate();
							nEndDate = warrant.GetPowerEndDate((INT)(reqTXFunction.nSecurityID));
							if(iTransObjID > 0 && nStartDate > 0 && nEndDate > 0 && reqTXFunction.nDate >= nStartDate && reqTXFunction.nDate <= nEndDate)
							{
								business.GetSecurityNow((LONG)iTransObjID);
								dValue1 = GetKLine(&business, reqTXFunction.nDate, 4,reqTXFunction.nSecurityID);
								dValue2 = GetKLine(pBusiness, reqTXFunction.nDate, 4,reqTXFunction.nSecurityID);
								dValue = warrant.GetGearRate(dValue1, dValue2, warrant.GetPowerRatio((INT)(reqTXFunction.nSecurityID)));
								strResult.Format(_T("%.2f"), dValue);
							}
							break;
						case 65:	//Ȩ֤-��Ȩ�۸�
							vdReq.data_type = dtype_float;
							if(!pBusiness->m_pSecurity->IsWarrant())
							{
								break;
							}
							dValue = warrant.GetPowerPrice((INT)(reqTXFunction.nSecurityID));
							if(dValue > 0)
							{
								strResult.Format(_T("%.2f"), dValue);
							}
							break;
						case 66:	//Ȩ֤-������
							vdReq.data_type = dtype_int4;
							if(!pBusiness->m_pSecurity->IsWarrant())
							{
								break;
							}
							nDate = warrant.GetPowerEndDate((INT)(reqTXFunction.nSecurityID));
							if(nDate > 0)
							{
								//�޸�Ϊ8λ��--20080708--
								strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
							}
							break;
						case 67:	//Ȩ֤-ʣ����
							vdReq.data_type = dtype_int4;
							if(!pBusiness->m_pSecurity->IsWarrant())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							nDate = warrant.GetRemainsDays((INT)(reqTXFunction.nSecurityID), reqTXFunction.nDate);
							if(nDate > 0)
							{
								strResult.Format(_T("%d"), nDate);
							}
							break;
						case 68:	//Ȩ֤-���ɴ���
							vdReq.data_type = dtype_val_string;
							if(!pBusiness->m_pSecurity->IsWarrant())
							{
								break;
							}
							iTransObjID = warrant.GetObjectSecurityId((INT)(reqTXFunction.nSecurityID));
							if(iTransObjID > 0)
							{
								pSecurity = (Tx::Data::SecurityQuotation *)GetSecurity((LONG)iTransObjID);
								if(pSecurity != NULL)
								{
									strResult = pSecurity->GetCode();
								}
							}
							break;
						case 69:	//Ȩ֤-��Ȩ����
							vdReq.data_type = dtype_double;
							if(!pBusiness->m_pSecurity->IsWarrant())
							{
								break;
							}
							dValue = warrant.GetPowerRatio((INT)(reqTXFunction.nSecurityID));
							if(dValue > 0)
							{
								strResult.Format(_T("%.4f"), dValue);
							}
							break;
						case 70:	//Ȩ֤-����ʣ������
							vdReq.data_type = dtype_double;
							if(!pBusiness->m_pSecurity->IsWarrant())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							dValue = GetDateIndicator_Value((INT)(reqTXFunction.nSecurityID), reqTXFunction.nDate, &vWarrantDV);
							if(dValue > 0)
							{
								strResult.Format(_T("%.0f"), dValue);
							}
							break;
						case 71:	//��������
							vdReq.data_type = dtype_val_string;
							strResult = _T("-");
							if(!pBusiness->m_pSecurity->IsFund())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 2);	//secId subitemId
							if(reqTXFunction.iSubItemID == 0 || reqTXFunction.iSubItemID == 1)
							{
								if(pBusiness->m_pSecurity->IsFund_LOF())
									strResult = _T("LOF");
								else if(pBusiness->m_pSecurity->IsFund_ETF())
									strResult = _T("ETF");
								else if(pBusiness->m_pSecurity->IsFund_Open())
									strResult = _T("����ʽ");
								else if(pBusiness->m_pSecurity->IsFund_Close())
									strResult = _T("���ʽ");
							}
							else if(reqTXFunction.iSubItemID == 2)
							{
								//if(pBusiness->m_pSecurity->IsFund_Stock())
								//{
								//	strResult = _T("��Ʊ��");
								//	if(pBusiness->m_pSecurity->IsFundStyle_StockActive())
								//		strResult += _T("->������");
								//	else if(pBusiness->m_pSecurity->IsFundStyle_StockSteady())
								//		strResult += _T("->�Ƚ���");
								//	else if(pBusiness->m_pSecurity->IsFundStyle_StockIndex())
								//		strResult += _T("->��ָ����");
								//	else if(pBusiness->m_pSecurity->IsFundStyle_StockIndexEnhance())
								//		strResult += _T("->��ǿָ����");
								//}
								//else if(pBusiness->m_pSecurity->IsFund_Intermix())
								//{
								//	strResult = _T("�����");
								//	if(pBusiness->m_pSecurity->IsFundStyle_FlexibleCombination())
								//		strResult += _T("->���������");
								//	else if(pBusiness->m_pSecurity->IsFundStyle_FundCombination())
								//		strResult += _T("->����������");
								//	else if(pBusiness->m_pSecurity->IsFundStyle_CareCombination())
								//		strResult += _T("->����������");
								//	else if(pBusiness->m_pSecurity->IsFundStyle_PreserCombination())
								//		strResult += _T("->���������");
								//	else if(pBusiness->m_pSecurity->IsFundStyle_SpecialCombination())
								//		strResult += _T("->�ض�������");
								//}
								//else if(pBusiness->m_pSecurity->IsFund_Bond())
								//{
								//	strResult = _T("ծȯ��");
								//	if(pBusiness->m_pSecurity->IsFundStyle_PureBond())
								//		strResult += _T("->��ծ��");
								//	else if(pBusiness->m_pSecurity->IsFundStyle_LeanBond())
								//		strResult += _T("->ƫծ��");
								//}
								//else if(pBusiness->m_pSecurity->IsFund_Currency())
								//{
								//	strResult = _T("�����г�����");
								//}
								//else if(pBusiness->m_pSecurity->IsFund_QDII())
								//	strResult = _T("QDII");
								
								//bug:12439    2012-08-07
								pFundNewInfo = pBusiness->m_pSecurity->GetFundNewInfo();
								if(pFundNewInfo != NULL)
								{
									mapIntStr.clear();
									Tx::Data::TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX_NEW, mapIntStr);
									iterIntStr = mapIntStr.find(pFundNewInfo->style_id);
									if(iterIntStr != mapIntStr.end())
									{
										strValue = iterIntStr->second;
										if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
										{
											strResult = strValue;
										}
									}
								}
							}
							/*pFundNewInfo = pBusiness->m_pSecurity->GetFundNewInfo();
							if(pFundNewInfo != NULL)
							{
								mapIntStr.clear();
								Tx::Data::TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_TYPE_INDEX, mapIntStr);
								iterIntStr = mapIntStr.find(pFundNewInfo->type_id);
								if(iterIntStr != mapIntStr.end())
								{
									strValue = iterIntStr->second;
									if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
									{
										strResult = strValue;
									}
								}
							}*/
							break;
						case 122:	//������
							vdReq.data_type = dtype_val_string;
							if(!pBusiness->m_pSecurity->IsFund())
							{
								break;
							}
							pFundNewInfo = pBusiness->m_pSecurity->GetFundNewInfo();
							if(pFundNewInfo != NULL)
							{
								mapIntStr.clear();
								//modified by zhangxs 20091221---NewStyle
								//Tx::Data::TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX, mapIntStr);
								Tx::Data::TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_STYLE_INDEX_NEW, mapIntStr);
								iterIntStr = mapIntStr.find(pFundNewInfo->style_id);
								if(iterIntStr != mapIntStr.end())
								{
									strValue = iterIntStr->second;
									if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
									{
										strResult = strValue;
									}
								}
							}
							break;
						case 73:	//����λ��ֵ
							vdReq.data_type = dtype_double;
							if(pBusiness->m_pSecurity->IsFund_Currency())
								break;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate < reqTXFunction.nDate)
								break;
							if ( reqTXFunction.iSubItemID )
								dValue = GetFundNvrExtVal( reqTXFunction.nSecurityID,reqTXFunction.nDate);
							else
								dValue = GetFundNvrVal( reqTXFunction.nSecurityID,reqTXFunction.nDate);
							if ( Con_doubleInvalid != dValue )
								strResult.Format(_T("%.4f"),dValue );
							/*
							pFundNetValueData = pBusiness->m_pSecurity->GetFundNetValueDataByDate(reqTXFunction.nDate);
							if(pFundNetValueData == NULL)
							{
								nDataCount = pBusiness->m_pSecurity->GetFundNetValueDataCount();
								if(nDataCount > 0)
								{
									pFundNetValueData = pBusiness->m_pSecurity->GetFundNetValueDataByIndex(nDataCount - 1);
								}
							}
							if(pFundNetValueData != NULL)
							{
								if(pFundNetValueData->fNetvalue > 0)
								{
									strResult.Format(_T("%.4f"), pFundNetValueData->fNetvalue);
								}
							}
							*/
							break;
						case 74:	//�����ۼƵ�λ��ֵ
							vdReq.data_type = dtype_double;
							if(pBusiness->m_pSecurity->IsFund_Currency())
								break;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate < reqTXFunction.nDate)
								break;
							if( reqTXFunction.iSubItemID )
								dValue = GetFundAcuNvrExtVal( reqTXFunction.nSecurityID,reqTXFunction.nDate);
							else
								dValue = GetFundAcuNvrVal( reqTXFunction.nSecurityID,reqTXFunction.nDate);
							if ( Con_doubleInvalid != dValue )
								strResult.Format(_T("%.4f"),dValue );
							/*
							pFundNetValueData = pBusiness->m_pSecurity->GetFundNetValueDataByDate(reqTXFunction.nDate);
							if(pFundNetValueData == NULL)
							{
								nDataCount = pBusiness->m_pSecurity->GetFundNetValueDataCount();
								if(nDataCount > 0)
								{
									pFundNetValueData = pBusiness->m_pSecurity->GetFundNetValueDataByIndex(nDataCount - 1);
								}
							}
							if(pFundNetValueData != NULL)
							{
								if(pFundNetValueData->fNetvalueAcu > 0)
								{
									strResult.Format(_T("%.4f"), pFundNetValueData->fNetvalueAcu);
								}
							}
							*/
							break;
						case 76:	//�����г��������ÿ��ݻ�������
							{
								vdReq.data_type = dtype_double;
								if(!pBusiness->m_pSecurity->IsFund_Currency())
									break;
								FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
								nDate = pBusiness->m_pSecurity->GetStopedLastDate();
								if(nDate > 0 && nDate < reqTXFunction.nDate)
									break;
								if ( reqTXFunction.iSubItemID)
									dValue = GetMmfFundNvrExtVal( reqTXFunction.nSecurityID,reqTXFunction.nDate);
								else
									dValue = GetMmfFundNvrVal( reqTXFunction.nSecurityID,reqTXFunction.nDate);
								// by 20101029 wanglm ������������ʱ���ء�-��
								CString str("");
								str.Format( "%d", reqTXFunction.nDate );
								int iYear = atoi( str.Left(4) );
								int iMon  = atoi( str.Mid(4, 2) );
								int iDay  = atoi( str.Right(2) );
                                
								CTime m_time(iYear,iMon,iDay,12,0,0);
								int weekday=m_time.GetDayOfWeek();
								if ( weekday == 7 )
								{
									//MessageBox("����������");
									strResult = _T("-");
								} 
								else
								{
									if ( dValue > 0.0 )
										strResult.Format(_T("%.5f"), dValue );
								}
							break;
							}
							/*
							iVal = pBusiness->m_pSecurity->GetFundGainDataIndexByDate(reqTXFunction.nDate);
							if(iVal >= 0)
							{
								if(iVal > 0)
								{
									iVal -- ;
								}
								pFundGainData = pBusiness->m_pSecurity->GetFundGainDataByIndex(iVal);
								//������һ�����ʱ���޸�--wangzhy---
								if(pFundGainData != NULL)
								{
									if(pFundGainData->gain_ten_thousand_share > 0)
									{
										strResult.Format(_T("%.5f"), pFundGainData->gain_ten_thousand_share);
									}
								}
							}
							*/
						case 79:	//�����г���������7���껯������
							vdReq.data_type = dtype_double;
							if(!pBusiness->m_pSecurity->IsFund_Currency())
								break;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate < reqTXFunction.nDate)
								break;
							if ( reqTXFunction.iSubItemID )
								dValue = GetMmfFundAcuNvrExtVal( reqTXFunction.nSecurityID,reqTXFunction.nDate);
							else
								dValue = GetMmfFundAcuNvrVal( reqTXFunction.nSecurityID,reqTXFunction.nDate);
							if ( dValue > 0.0 )
								strResult.Format(_T("%.4f"), dValue );
							/*
							iVal = pBusiness->m_pSecurity->GetFundGainDataIndexByDate(reqTXFunction.nDate);
							if(iVal >= 0)
							{
								if(iVal > 0)
								{
									iVal -- ;
								}
								pFundGainData = pBusiness->m_pSecurity->GetFundGainDataByIndex(iVal);
								if(pFundGainData != NULL)
								{
									if(pFundGainData->year_yield_last_week > 0)
									{
										strResult.Format(_T("%.4f"), pFundGainData->year_yield_last_week);
									}
								}
							}
							*/
							break;
						case 77:	//�����������
							vdReq.data_type = dtype_double;
							//if(!pBusiness->m_pSecurity->IsFundTradeInMarket())
							// 2012-09-20   Bug:13047
							if (!pBusiness->m_pSecurity->IsFund())
								break;
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							pFundNewInfo = pBusiness->m_pSecurity->GetFundNewInfo();
							if (pFundNewInfo == NULL)
							{
								break;
							}
							if(reqTXFunction.nDate < pBusiness->m_pSecurity->GetFundNewInfo()->ipo_date)//��û���̼�
								break;
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate < reqTXFunction.nDate)
								break;
							pFundNetValueData = pBusiness->m_pSecurity->GetFundNetValueDataByDate(reqTXFunction.nDate);
							if(pFundNetValueData != NULL)
							{
								//--bugfixed--�����������ÿ�ܸ�һ��������Ӧ�������һ��
						
								//��ֹ�����л���ֵ
								//dValue1 = pBusiness->m_pSecurity->GetClosePrice(pFundNetValueData->iDate,true);
								if(pBusiness->m_pSecurity->IsFund_Close() || pBusiness->m_pSecurity->IsFund_ETF() || pBusiness->m_pSecurity->IsFund_LOF())
									dValue1 = pBusiness->m_pSecurity->GetClosePrice(pFundNetValueData->iDate,true);
								else
									dValue1 = pBusiness->m_pSecurity->GetOpenFundClosePrice(pFundNetValueData->iDate,true);
								dValue2 = pFundNetValueData->fNetvalue;
								if(dValue1 > 0 && dValue2 > 0)
								{
									dValue = 100 * (dValue1 - dValue2) / dValue2;
									strResult.Format(_T("%.4f"), dValue);
								}

								/*
								if(reqTXFunction.nDate == pFundNetValueData->iDate)
								{	//��ֹ�����л���ֵ
									dValue1 = pBusiness->m_pSecurity->GetClosePrice(reqTXFunction.nDate);
									dValue2 = pFundNetValueData->fNetvalue;
									if(dValue1 > 0 && dValue2 > 0)
									{
										dValue = 100 * (dValue1 - dValue2) / dValue2;
										strResult.Format(_T("%.2f"), dValue);
									}
								}
								*/
							}
							break;
						case 78:	//����ʣ������(����)
							vdReq.data_type = dtype_int4;
							if(!pBusiness->m_pSecurity->IsFund_Close())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate <= reqTXFunction.nDate)
							{
								break;
							}
							pFundNewInfo = pBusiness->m_pSecurity->GetFundNewInfo();
							if(pFundNewInfo != NULL)
							{
								if(pFundNewInfo->hold_year > 0 && pFundNewInfo->setup_date > 0)
								{
									tmFrom = CTime(reqTXFunction.nDate / 10000, (reqTXFunction.nDate % 10000) / 100, reqTXFunction.nDate % 100, 0, 0, 0);
									tmTo = CTime(pFundNewInfo->setup_date / 10000 + pFundNewInfo->hold_year, (pFundNewInfo->setup_date % 10000) / 100, pFundNewInfo->setup_date % 100, 0, 0, 0);
									if(tmTo > tmFrom)
									{
										tsInterVal = tmTo - tmFrom;
										strResult.Format(_T("%d"), tsInterVal.GetDays());
									}
								}
							}
							break;
						case 80:	//����ֵ������
							vdReq.data_type = dtype_double;
							{
								if(!pBusiness->m_pSecurity->IsFund())
								{
									break;
								}
								//if(pBusiness->m_pSecurity->IsFund_Currency())
								//{
								//	 
								//}
								//BUG:13241    2012-09-27
								pFundNewInfo = pBusiness->m_pSecurity->GetFundNewInfo();
								if (pFundNewInfo == NULL)
								{
									break;
								}
								if(pBusiness->m_pSecurity->IsFund_Currency() || pFundNewInfo->style_id == 21)
								{
									break;
								}
								FindItemOfReq(&reqTXFunction, strReq, 5);	//StartDate��EndDate
								nDate = pBusiness->m_pSecurity->GetStopedLastDate();
								if(nDate > 0 && nDate < reqTXFunction.nEndDate)
								{
									break;
								}
								vRow.clear();
								m_nNvrTable.Find(0,reqTXFunction.nSecurityID,vRow);
								if ( vRow.size()!= 1 )
									break;
								map<pair<int,int>,int>::iterator itr = m_d2cMap.find(pair<int,int>(reqTXFunction.nStartDate,reqTXFunction.nEndDate));
								if ( itr == m_d2cMap.end())
									break;
								m_nNvrTable.GetCell(itr->second,vRow[0],dValue );
								if ( dValue != Con_doubleInvalid )
								{
									//double dRet = 0.000;
									//if ( dRet >= dValue )
									//{
									//	strResult = _T("-");
									//}
									//else
									{
										strResult.Format(_T("%.4f"), dValue);
									}
								}
							}

							//dValue = GetDatesIndicator_Value((INT)(reqTXFunction.nSecurityID), reqTXFunction.nStartDate, reqTXFunction.nEndDate, &vFundNAVGrowth);
							//if(dValue != 0)
							//{
							//	strResult.Format(_T("%.4f"), 100 * dValue);
							//}
							break;
						case 81:	//IOPV(ETF����LOF����)
						case 286:   //IOPV(ETF����LOF����)
							vdReq.data_type = dtype_double;
							if(!(pBusiness->m_pSecurity->IsFund_ETF() || pBusiness->m_pSecurity->IsFund_LOF()))
							{
								break;
							}
							/*
							dValue = pBusiness->m_pSecurity->GetIOPV();
							if(dValue > 0)
							{
								strResult.Format(_T("%.4f"), dValue);
							}
							*/
							mapIntInt.clear();
							Tx::Data::TypeMapManage::GetInstance()->GetTypeMapITI(TYPE_IOPV_ID_ID, mapIntInt);
							iterIntInt = mapIntInt.find((INT)(reqTXFunction.nSecurityID));
							if(iterIntInt != mapIntInt.end())
							{
								if(business.GetSecurityNow((LONG)(iterIntInt->second)) != NULL)
								{
									if(pBusiness->m_pSecurity->IsValid())
									{
										dValue = business.m_pSecurity->GetClosePrice(true);
									}else
									{
										dValue = business.m_pSecurity->GetClosePrice(pBusiness->m_pSecurity->GetTradeDateLatest());
									}
									if(dValue > 0)
									{
										strResult.Format(_T("%.4f"), dValue);
									}
								}
							}
							break;
						case 82:	//�����й���
							vdReq.data_type = dtype_val_string;
							if(!pBusiness->m_pSecurity->IsFund())
							{
								break;
							}
							pFundNewInfo = pBusiness->m_pSecurity->GetFundNewInfo();
							if(pFundNewInfo != NULL)
							{
								mapIntStr.clear();
								Tx::Data::TypeMapManage::GetInstance()->GetTypeMap(TYPE_FUND_BANK, mapIntStr);
								iterIntStr = mapIntStr.find(pFundNewInfo->trusteeship_bank);
								if(iterIntStr != mapIntStr.end())
								{
									strValue = iterIntStr->second;
									if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
									{
										strResult = strValue;
									}
								}
							}
							break;
						case 83:	//�����г�����������
							{
								vdReq.data_type = dtype_val_string;
								//BUG:13241    2012-09-27
								//if(!pBusiness->m_pSecurity->IsFund_Currency())
								pFundNewInfo = pBusiness->m_pSecurity->GetFundNewInfo();
								if (pFundNewInfo == NULL)
								{
									break;
								}
								if(pBusiness->m_pSecurity->IsFund_Currency() || pFundNewInfo->style_id == 21)
								{
								}
								else
									break;
								FindItemOfReq(&reqTXFunction, strReq, 5);	//StartDate��EndDate
								int nDateList = pBusiness->m_pSecurity->GetFundEstablishDate();
								if(nDateList > 0 && nDateList > reqTXFunction.nEndDate)
									break;
								nDate = pBusiness->m_pSecurity->GetStopedLastDate();
								if(nDate > 0 && nDate < reqTXFunction.nEndDate)
									break;
								dValue = GetMMFGrowth(pBusiness, reqTXFunction.nStartDate, reqTXFunction.nEndDate);
								if(dValue > 0)
									strResult.Format(_T("%.8f"), dValue);
							}
							break;
						case 84:	//������������
							vdReq.data_type = dtype_int4;
							if(!pBusiness->m_pSecurity->IsFund())
							{
								break;
							}
							nDate = pBusiness->m_pSecurity->GetFundEstablishDate();
							if(nDate > 0)
							{
								//�޸�Ϊ8λ��--20080708--
								strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
							}
							break;
						case 85:	//ծȯ����
							vdReq.data_type = dtype_val_string;
							if(!pBusiness->m_pSecurity->IsBond())
							{
								break;
							}
							pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
							if(pBondNewInfo != NULL)
							{
								iVal = pBondNewInfo->bond_type;
								strResult = Tx::Data::TypeMapManage::GetInstance()->GetDatByID(TYPE_BOND_TYPE, iVal);
							}
							break;
						case 123:	//ծȯ��ʽ
							vdReq.data_type = dtype_val_string;
							if(!pBusiness->m_pSecurity->IsBond())
							{
								break;
							}
							pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
							if(pBondNewInfo != NULL)
							{
								iVal = pBondNewInfo->bond_form;
								strResult = Tx::Data::TypeMapManage::GetInstance()->GetDatByID(TYPE_BOND_FORM, iVal);
							}
							break;
						case 86:	//ծȯ������
							vdReq.data_type = dtype_int4;
							if(!pBusiness->m_pSecurity->IsBond())
							{
								break;
							}
							pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
							if(pBondNewInfo != NULL)
							{
								dValue = pBondNewInfo->share;
								if ( dValue > 0.0 )
									strResult.Format(_T("%.0f"), dValue);
							}
							break;
						case 87:	//ծȯ��ֵ
							vdReq.data_type = dtype_double;
							if(!pBusiness->m_pSecurity->IsBond())
							{
								break;
							}
							pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
							if(pBondNewInfo != NULL)
							{
								dValue = pBondNewInfo->par_val;
								if ( dValue > 0.0 )
									strResult.Format(_T("%.0f"), dValue);
							}
							break;
						case 88:	//ծȯƱ������
							{
								vdReq.data_type = dtype_double;
								if(!pBusiness->m_pSecurity->IsBond())
								{
									break;
								}
								CTime tCurrentDate = CTime::GetCurrentTime();
								int nCurrentDate = tCurrentDate.GetYear()*10000+tCurrentDate.GetMonth()*100+tCurrentDate.GetDay();
								FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
								if (nCurrentDate == (int)reqTXFunction.nDate)
								{
									pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
									if(pBondNewInfo != NULL)
									{
										dValue = pBondNewInfo->par_rate;
										if ( dValue >= 0.0 )
											strResult.Format(_T("%.4f"), dValue / 100);
									}
								}
								else
								{
									int count = pBusiness->m_pSecurity->GetBondPmllCount();
									float payInterst = Con_floatInvalid;
									if(count <= 0) 
										break;
									Tx::Data::BondPmll* pBondPmll = NULL;
									for (int i=0;i<count;i++)
									{	
										pBondPmll = pBusiness->m_pSecurity->GetBondPmllByIndex(i);
										if(pBondPmll == NULL)
											continue;
										if ((int)reqTXFunction.nDate >= pBondPmll->sdate)
										{
											payInterst = pBondPmll->payInterest;
											continue;
										}
										else
											break;
									}
									dValue = payInterst;
									if(dValue > Con_floatInvalid)
										strResult.Format(_T("%.4f"), dValue/100);
								}
							}
							break;
						case 89:	//ծȯ��ϢƵ��
							vdReq.data_type = dtype_double;
							if(!pBusiness->m_pSecurity->IsBond())
							{
								break;
							}
							pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
							if(pBondNewInfo != NULL)
							{
								iVal = pBondNewInfo->pay_interest_frequence;
								if ( iVal > 0 )
									strResult.Format(_T("%d��1��"), iVal);
							}
							break;
						case 90:	//ծȯ��������
							vdReq.data_type = dtype_int4;
							if(!pBusiness->m_pSecurity->IsBond())
							{
								break;
							}
							pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
							if(pBondNewInfo != NULL)
							{
								dValue = pBondNewInfo->hold_year;
								if ( dValue >= 0.0 )
									strResult.Format(_T("%.0f"), dValue);
							}
							break;
						case 91:	//ծȯ��Ϣ����
							vdReq.data_type = dtype_int4;
							if(!pBusiness->m_pSecurity->IsBond())
							{
								break;
							}
							pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
							if(pBondNewInfo != NULL)
							{
								nDate = pBondNewInfo->begin_date;
								if ( nDate > 0 )
									strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
							}
							break;
						case 92:	//ծȯ��������
							vdReq.data_type = dtype_int4;
							if(!pBusiness->m_pSecurity->IsBond())
							{
								break;
							}
							pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
							if(pBondNewInfo != NULL)
							{
								nDate = pBondNewInfo->end_date;
								//�޸�Ϊ8λ��--20080708--
								if ( nDate > 0 )
									strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
							}
							break;
						case 93:	//ծȯ������Ϣ��
							vdReq.data_type = dtype_int4;
							if(!pBusiness->m_pSecurity->IsBond())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate < reqTXFunction.nDate)
							{
								break;
							}
							pBondCashFlowData = pBusiness->m_pSecurity->GetBondCashFlowDataByDate(reqTXFunction.nDate);
							if(pBondCashFlowData != NULL)
							{
								nDate = pBondCashFlowData->F_LIPD;
								//�޸�Ϊ8λ��--20080708--
								if ( nDate > 0 )
									strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
							}
							break;
						case 94:	//ծȯ���ڸ�Ϣ��
							vdReq.data_type = dtype_int4;
							if(!pBusiness->m_pSecurity->IsBond())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate < reqTXFunction.nDate)
							{
								break;
							}
							pBondCashFlowData = pBusiness->m_pSecurity->GetBondCashFlowDataByDate(reqTXFunction.nDate);
							if(pBondCashFlowData != NULL)
							{
								nDate = pBondCashFlowData->F_NIPD;
								//�޸�Ϊ8λ��--20080708--
								if ( nDate > 0 )
									strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								//strResult.Format(_T("%04d-%02d-%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
							}
							break;
						case 95:	//ծȯӦ����Ϣ
							{
								vdReq.data_type = dtype_double;
								if(!pBusiness->m_pSecurity->IsBond())
								{
									break;
								}
								FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
								FindItemOfReq(&reqTXFunction, strReq, 39);  //Ӧ����Ϣ��־ 0--����Ӧ����Ϣ(�ռ�����)��1--����Ӧ����Ϣ(��������) 2012-07-23

								bool bFlag;
								if((int)reqTXFunction.iYJLX == 0)
								     bFlag = true;
								else if((int)reqTXFunction.iYJLX == 1)
							         bFlag = false;
								else
									break;

								nDate = pBusiness->m_pSecurity->GetStopedLastDate();
								if(nDate > 0 && nDate < reqTXFunction.nDate)
								{
									break;
								}
								//dValue = bond.GetInterest((INT)(reqTXFunction.nSecurityID), reqTXFunction.nDate);
								//2012-7-16  Ӧ����Ϣ(��)
								dValue = bond.GetInterest_New((INT)(reqTXFunction.nSecurityID),reqTXFunction.nDate,bFlag);
								if(dValue >= 0)
								{
									strResult.Format(_T("%.4f"), dValue);
								}
							}
							break;
						case 96:	//ծȯ��Ϣ����
							vdReq.data_type = dtype_int4;
							if(!pBusiness->m_pSecurity->IsBond())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate < reqTXFunction.nDate)
							{
								break;
							}
							nDate = bond.GetInterestDays((INT)(reqTXFunction.nSecurityID), reqTXFunction.nDate);
							if(nDate > 0)
							{
								strResult.Format(_T("%d"), nDate);
							}
							break;
						case 97:	//ծȯʣ������
							vdReq.data_type = dtype_val_string;
							if(!pBusiness->m_pSecurity->IsBond())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate < reqTXFunction.nDate)
							{
								break;
							}
							pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
							if(pBondNewInfo != NULL)
							{
								nStartDate = reqTXFunction.nDate;
								nEndDate = pBondNewInfo->end_date;
								strValue = bond.GetRemnantsString(nStartDate, nEndDate);
								if(strValue.Compare(_T("-")) != 0)
								{
									strResult = strValue;
								}
							}
							break;
						case 98:	//ծȯYTM
						case 104:	//ծȯ��������
						case 105:	//ծȯ͹��
							{
								vdReq.data_type = dtype_double;
								if(!pBusiness->m_pSecurity->IsBond()) 
									break;
								FindItemOfReq(&reqTXFunction, strReq, 17);	//Date��Price
								FindItemOfReq(&reqTXFunction, strReq, 8);    //iPriceType
								nDate = pBusiness->m_pSecurity->GetStopedLastDate();
								if(nDate > 0 && nDate < reqTXFunction.nDate)
									break;
								CTime time = CTime::GetCurrentTime();
								int iTime = time.GetYear()*10000 + time.GetMonth()*100 + time.GetDay();		
								//��ʷYTM���������ڡ�͹��
								if (iTime == reqTXFunction.nDate)
								{
									double dYtm,dDur,dMDur,dCon;
									dYtm = Con_doubleInvalid;
									dDur = Con_doubleInvalid;
									dMDur = Con_doubleInvalid;
									dCon = Con_doubleInvalid;
									GetBondYTM(reqTXFunction.nSecurityID,dYtm,dDur,dMDur,dCon);
									switch(reqTXFunction.iFuncID)
									{
									case 98:
										dValue = dYtm;
										if(dValue != Tx::Core::Con_doubleInvalid)
										{
											strResult.Format(_T("%.6f"), dValue*100);
										}
										break;
									case 104:
										dValue = dMDur;
										if(dValue != Tx::Core::Con_doubleInvalid)
										{
											strResult.Format(_T("%.6f"), dValue);
										}
										break;
									case 105:
										dValue = dCon;
										if(dValue != Tx::Core::Con_doubleInvalid)
										{
											strResult.Format(_T("%.6f"), dValue);
										}
										break;
									default:
										break;
									}
								}
								else
								{
									if(reqTXFunction.dPrice <= 0.0)
									{
										BondYTM* pBondYTM = pBusiness->m_pSecurity->GetBondYTMByDate(reqTXFunction.nDate);
										if (pBondYTM != NULL)
										{
											switch(reqTXFunction.iFuncID)
											{
											case 98:
												dValue = pBondYTM->fYtm;
												if(dValue != Tx::Core::Con_doubleInvalid)
												{
													strResult.Format(_T("%.6f"), dValue*100);
												}
												break;
											case 104:
												dValue = pBondYTM->fADur;
												if(dValue != Tx::Core::Con_doubleInvalid)
												{
													strResult.Format(_T("%.6f"), dValue);
												}
												break;
											case 105:
												dValue = pBondYTM->fCon;
												if(dValue != Tx::Core::Con_doubleInvalid)
												{
													strResult.Format(_T("%.6f"), dValue);
												}
												break;
											default:
												break;
											}
										}
									}
									else
									{
										//////////////////////////////////////////////////////////////////////////
										// 20131011 ���Ӽ۸����ͼ��۸���ж� xujf
										Tx::Business::TxBond  TxBond;			
										double dDur = 0.0, dMDur = 0.0, dCon = 0.0, dYTM = 0.0;
										TxBond.GetYTM(pBusiness->m_pSecurity, reqTXFunction.nDate, (float)reqTXFunction.dPrice, reqTXFunction.iPriceType, dYTM, dDur, dMDur, dCon);
										if(dYTM <= -100 || dDur <= -100 || dMDur < -100 || dCon <= -100)
											strResult.Format(_T("-"));
										else
										{
											switch(reqTXFunction.iFuncID)
											{
											case 98:
												strResult.Format(_T("%.6f"), dYTM*100);
												break;
											case 104:
												strResult.Format(_T("%.6f"), dDur);
												break;
											case 105:
												strResult.Format(_T("%.6f"), dCon);
												break;
											default:
												break;
											}
										}
									}
								}
							}
							break;

							// -----------------------------------------------------------------

							//FindItemOfReq(&reqTXFunction, strReq, 17);	//Date��Price
							//FindItemOfReq(&reqTXFunction, strReq, 8);    //iPriceType
							//if(reqTXFunction.dPrice == 0)
							//{
							//	pBondGeneral = pBusiness->m_pSecurity->GetBondGeneralByDate(reqTXFunction.nDate);
							//	if(pBondGeneral == NULL)
							//	{
							//		nDate = CTime::GetCurrentTime().GetYear() * 10000 + CTime::GetCurrentTime().GetMonth() * 100 + CTime::GetCurrentTime().GetDay();
							//		if(reqTXFunction.nDate == nDate && pBusiness->m_pSecurity->IsValid())
							//		{
							//			reqTXFunction.dPrice = GetKLine(pBusiness, reqTXFunction.nDate, 4,reqTXFunction.nSecurityID);
							//		}else
							//		{
							//			break;
							//		}
							//	}else
							//	{
							//		switch(reqTXFunction.iFuncID)
							//		{
							//		case 98:
							//			dValue = pBondGeneral->dYtm;
							//			if(dValue != Tx::Core::Con_doubleInvalid)
							//			{
							//				strResult.Format(_T("%.6f"), dValue);
							//			}
							//			break;
							//		case 104:
							//			dValue = pBondGeneral->dDuration;
							//			if(dValue != Tx::Core::Con_doubleInvalid)
							//			{
							//				strResult.Format(_T("%.6f"), dValue);
							//			}
							//			break;
							//		case 105:
							//			dValue = pBondGeneral->dConvexity;
							//			if(dValue != Tx::Core::Con_doubleInvalid)
							//			{
							//				strResult.Format(_T("%.6f"), dValue);
							//			}
							//			break;
							//		default:
							//			break;
							//		}
							//		break;
							//	}
							//}
							//if(reqTXFunction.dPrice <= 0)
							//{
							//	break;
							//}
							////if(pBusiness->m_pSecurity->IsBond_National())
							//if(reqTXFunction.iPriceType == 0)  //����  2012-09-11
							//{	//��ծʵ�о��۽��ף����Ǽ��㵽��������ʱծȯ�۸�Ϊȫ��
							//	//dValue = bond.GetInterest((INT)(reqTXFunction.nSecurityID), reqTXFunction.nDate);
							//	//2012-7-16  Ӧ����Ϣ(��)
							//	dValue = bond.GetInterest_New((INT)(reqTXFunction.nSecurityID), reqTXFunction.nDate,true);
							//	if(dValue < 0)
							//	{
							//		break;
							//	}
							//	reqTXFunction.dPrice += dValue;
							//}
							//bond.Calc((INT)(reqTXFunction.nSecurityID), reqTXFunction.nDate, (FLOAT)(reqTXFunction.dPrice));
							//switch(reqTXFunction.iFuncID)
							//{
							//case 98:
							//	dValue = bond.Get_YTM();
							//	if(dValue != Tx::Core::Con_doubleInvalid)
							//	{
							//		strResult.Format(_T("%.6f"), dValue*100); //modified by zhangxs ����ϵͳ��ʾ���ݲ�һ���Ҿ��Ȳ�һ��
							//	}
							//	break;
							//case 104:
							//	dValue = bond.Get_MDURATION();
							//	if(dValue != Tx::Core::Con_doubleInvalid)
							//	{
							//		strResult.Format(_T("%.6f"), dValue);
							//	}
							//	break;
							//case 105:
							//	dValue = bond.Get_CONVEXITY();
							//	if(dValue != Tx::Core::Con_doubleInvalid)
							//	{
							//		strResult.Format(_T("%.6f"), dValue);
							//	}
							//	break;
							//default:
							//	break;
							//}
							//break;

							// ----------------------------------------------------
						case 124:	//ʮ��ɶ�����
						case 125:	//ʮ��ɶ��ֹ���
						case 126:	//ʮ��ɶ��ֹɱ���
						case 127:	//ʮ��ɶ��ֹ�����

							switch( reqTXFunction.iFuncID)
							{
							case 125:	//ʮ��ɶ��ֹ���
							case 126:	//ʮ��ɶ��ֹɱ���
								vdReq.data_type = dtype_double;
								break;
							case 124:	//ʮ��ɶ�����
							case 127:	//ʮ��ɶ��ֹ�����
								vdReq.data_type = dtype_val_string;
								break;
							}

							if(!(pBusiness->m_pSecurity->IsStockA() || pBusiness->m_pSecurity->IsStockB()))
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 18);	//Date��iHolderNo
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate < reqTXFunction.nDate)
							{
								break;
							}
							//--�������������µ�����ģʽ--wangzhy--20080603	
							//maybe bug			��һЩ���ݲ���ʮ���ɶ�,���ҿ��ܲ��йɶ����
							pHolder = NULL;
							pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_Holder,reqTXFunction.nDate,&pHolder,true);							
							if ( pHolder == NULL || reqTXFunction.iHolderNo > 10 )
							{
								//strResult = Con_strInvalid;
								break;
							}
							//ȡ��Ŀǰ����ȡ�ü�¼�Ĺɶ����,���ͨ���������ҵ������ʱ����ߵ����һ����¼,��������Ҫ�����û�Ҫ��ID�ڽ���ѡ��
							nAccountingID = pHolder->bNo;		//�û�Ҫ��ʮ��ɶ����
							if ( nAccountingID < reqTXFunction.iHolderNo )
							{
								//strResult = Con_strInvalid;
								break;
							}
							if( nAccountingID > reqTXFunction.iHolderNo )
								pHolder = pHolder + reqTXFunction.iHolderNo - nAccountingID;  
							//while( nAccountingID > reqTXFunction.iHolderNo )
							//{
							//	nAccountingID--;
							//	pHolder--;
							//}
							switch( reqTXFunction.iFuncID)
							{
							case 124:	//ʮ��ɶ�����
								strResult = pHolder->bName;
								break;
							case 125:	//ʮ��ɶ��ֹ���
								strResult.Format(_T("%.0f"),pHolder->dVolume);
								break;
							case 126:	//ʮ��ɶ��ֹɱ���
								strResult.Format(_T("%.4f"),pHolder->dPercent);
								break;
							case 127:	//ʮ��ɶ��ֹ�����
								strResult = Tx::Data::TypeMapManage::GetInstance()->GetDatByID(TYPE_SHAREHOLDER_TYPE, pHolder->iFlag);
								break;
							default:
								break;
							}
							//---------------------------------------------
							/*						
							iRowNo = GetShareHolderIndicatorRow((INT)(reqTXFunction.nSecurityID), reqTXFunction.nDate, reqTXFunction.iHolderNo, &vTopTenShareHolder);
							if(iRowNo >= 0)
							{
								switch(reqTXFunction.iFuncID)
								{
								case 124:	//ʮ��ɶ�����
									if(reqTXFunction.iHolderNo == 0)
									{	//ȡ��ʮ��ɶ�����
										strResult.Format(_T("%d"), iRowNo);
										break;
									}
									tableShareHolder[0].GetCell(3, (UINT)iRowNo, strValue);
									if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
									{
										strResult = strValue;
									}
									break;
								case 125:	//ʮ��ɶ��ֹ���
									tableShareHolder[0].GetCell(4, (UINT)iRowNo, dValue);
									if(dValue > 0)
									{
										strResult.Format(_T("%.0f"), dValue);
									}
									break;
								case 126:	//ʮ��ɶ��ֹɱ���
									tableShareHolder[0].GetCell(5, (UINT)iRowNo, dValue);
									if(dValue > 0)
									{
										strResult.Format(_T("%.4f"), dValue);
									}
									break;
								case 127:	//ʮ��ɶ��ֹ�����
									tableShareHolder[0].GetCell(6, (UINT)iRowNo, strValue);
									if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
									{
										iVal = _ttoi(strValue);
										strResult = Tx::Data::TypeMapManage::GetInstance()->GetDatByID(TYPE_SHAREHOLDER_TYPE, iVal);
									}
									break;
								default:
									break;
								}
							}
							*/
							break;
						case 128:	//ʮ����ͨ�ɶ�����
						case 129:	//ʮ����ͨ�ɶ��ֹ���
						case 130:	//ʮ����ͨ�ɶ��ܹɱ���
						case 131:	//ʮ����ͨ�ɶ���ͨ����
						case 132:	//ʮ����ͨ�ɶ��ֹ�����
							switch( reqTXFunction.iFuncID)
							{
							case 128:	//ʮ����ͨ�ɶ�����
							case 132:	//ʮ����ͨ�ɶ��ֹ�����
								vdReq.data_type = dtype_val_string;
								break;
							case 129:	//ʮ����ͨ�ɶ��ֹ���
								vdReq.data_type = dtype_int4;
								break;
							case 130:	//ʮ����ͨ�ɶ��ܹɱ���
							case 131:	//ʮ����ͨ�ɶ���ͨ����
								vdReq.data_type = dtype_double;
								break;
							}
							if(!(pBusiness->m_pSecurity->IsStockA() || pBusiness->m_pSecurity->IsStockB()))
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 18);	//Date��iHolderNo
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate < reqTXFunction.nDate)
							{
								break;
							}
							//--wangzhy--20080604--
							pTradeableHolder = NULL;
							pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_TradeableHolder,reqTXFunction.nDate,&pTradeableHolder,true);							
							if ( pTradeableHolder == NULL || reqTXFunction.iHolderNo > 10 )
							{
								//strResult = Con_strInvalid;
								break;
							}
							//ȡ��Ŀǰ����ȡ�ü�¼�Ĺɶ����,���ͨ���������ҵ������ʱ����ߵ����һ����¼,��������Ҫ�����û�Ҫ��ID�ڽ���ѡ��
							nAccountingID = pTradeableHolder->bNo;		//�û�Ҫ��ʮ��ɶ����
							if ( nAccountingID < reqTXFunction.iHolderNo )
							{
								//strResult = Con_strInvalid;
								break;
							}
							if( nAccountingID > reqTXFunction.iHolderNo )
								pTradeableHolder = pTradeableHolder + reqTXFunction.iHolderNo - nAccountingID;  
							//while( nAccountingID > reqTXFunction.iHolderNo )
							//{
							//	nAccountingID--;
							//	pTradeableHolder--;
							//}
							switch( reqTXFunction.iFuncID)
							{
							case 128:	//ʮ����ͨ�ɶ�����
								strResult = pTradeableHolder->bName;
								break;
							case 129:	//ʮ����ͨ�ɶ��ֹ���
								strResult.Format(_T("%.0f"),pTradeableHolder->dVolume);
								break;
							case 130:	//ʮ����ͨ�ɶ��ܹɱ���
								strResult.Format(_T("%.4f"),pTradeableHolder->dPercent);
								break;
							case 131:	//ʮ����ͨ�ɶ���ͨ����
								strResult.Format(_T("%.4f"),pTradeableHolder->dTradeablePercent);
								break;
							case 132:	//ʮ����ͨ�ɶ��ֹ�����
								strResult = Tx::Data::TypeMapManage::GetInstance()->GetDatByID(TYPE_TRADABLE_SHAREHOLDER, pTradeableHolder->iFlag);
								break;
							default:
								break;
							}
							//---------------------
							/*
							iRowNo = GetShareHolderIndicatorRow((INT)(reqTXFunction.nSecurityID), reqTXFunction.nDate, reqTXFunction.iHolderNo, &vTopTenCShareHolder);
							if(iRowNo >= 0)
							{
								switch(reqTXFunction.iFuncID)
								{
								case 128:	//ʮ����ͨ�ɶ�����
									if(reqTXFunction.iHolderNo == 0)
									{	//ȡ��ʮ����ͨ�ɶ�����
										strResult.Format(_T("%d"), iRowNo);
										break;
									}
									tableShareHolder[1].GetCell(3, (UINT)iRowNo, strValue);
									if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
									{
										strResult = strValue;
									}
									break;
								case 129:	//ʮ����ͨ�ɶ��ֹ���
									tableShareHolder[1].GetCell(4, (UINT)iRowNo, dValue);
									if(dValue > 0)
									{
										strResult.Format(_T("%.0f"), dValue);
									}
									break;
								case 130:	//ʮ����ͨ�ɶ��ܹɱ���
									tableShareHolder[1].GetCell(5, (UINT)iRowNo, dValue);
									if(dValue > 0)
									{
										strResult.Format(_T("%.4f"), dValue);
									}
									break;
								case 131:	//ʮ����ͨ�ɶ���ͨ����
									tableShareHolder[1].GetCell(6, (UINT)iRowNo, dValue);
									if(dValue > 0)
									{
										strResult.Format(_T("%.4f"), dValue);
									}
									break;
								case 132:	//ʮ����ͨ�ɶ��ֹ�����
									tableShareHolder[1].GetCell(7, (UINT)iRowNo, strValue);
									if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
									{
										strResult.Format(_T("%s��"), strValue);
									}
									break;
								default:
									break;
								}
							}
							*/
							break;
						//--wangzhy--20080604--
						case 138:	//���ʽ����ʮ�����������
						case 139:	//���ʽ����ʮ������˻���ݶ�
						case 140:	//���ʽ����ʮ������˻���ݶ����
							switch(reqTXFunction.iFuncID)
							{
							case 138:	//���ʽ����ʮ�����������
								vdReq.data_type = dtype_val_string;
								break;
							case 139:	//���ʽ����ʮ������˻���ݶ�
							case 140:	//���ʽ����ʮ������˻���ݶ����
								vdReq.data_type = dtype_double;
								break;
							}
							if(!pBusiness->m_pSecurity->IsFund_Close())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 18);	//Date��iHolderNo
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate < reqTXFunction.nDate)
							{
								break;
							}
							pFundBondHolder = NULL;
							pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_FundHoder,reqTXFunction.nDate,&pFundBondHolder,true);							
							if ( pFundBondHolder == NULL || reqTXFunction.iHolderNo > 10 )
							{
								//strResult = Con_strInvalid;
								break;
							}
							//ȡ��Ŀǰ����ȡ�ü�¼�Ĺɶ����,���ͨ���������ҵ������ʱ����ߵ����һ����¼,��������Ҫ�����û�Ҫ��ID�ڽ���ѡ��
							nAccountingID = pFundBondHolder->iNo;		//�û�Ҫ��ʮ��ɶ����
							if ( nAccountingID < reqTXFunction.iHolderNo )
							{
								//strResult = Con_strInvalid;
								break;
							}
							if( nAccountingID > reqTXFunction.iHolderNo )
								pFundBondHolder = pFundBondHolder + reqTXFunction.iHolderNo - nAccountingID;  
							//while( nAccountingID > reqTXFunction.iHolderNo )
							//{
							//	nAccountingID--;
							//	pFundBondHolder--;
							//}
							switch(reqTXFunction.iFuncID)
							{
							case 138:	//���ʽ����ʮ�����������
								strResult = pFundBondHolder->bName;
								break;
							case 139:	//���ʽ����ʮ������˻���ݶ�
								strResult.Format(_T("%.0f"), pFundBondHolder->dVolume);
								break;
							case 140:	//���ʽ����ʮ������˻���ݶ����
								strResult.Format(_T("%.4f"), pFundBondHolder->dPercent);
								break;
							default:
								break;
							}
							break;
						case 141:	//��תծʮ�����������
						case 142:	//��תծʮ�������ծȯ���
						case 143:	//��תծʮ�������ծȯ������
							switch(reqTXFunction.iFuncID)
							{
							case 141:	//��תծʮ�����������
								vdReq.data_type = dtype_val_string;
								break;
							case 142:	//��תծʮ�������ծȯ���
							case 143:	//��תծʮ�������ծȯ������
								vdReq.data_type = dtype_double;
								break;
							}
							if(!pBusiness->m_pSecurity->IsBond_Change())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 18);	//Date��iHolderNo
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate < reqTXFunction.nDate)
							{
								break;
							}
							pFundBondHolder = NULL;
							pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_BondHoder,reqTXFunction.nDate,&pFundBondHolder,true);							
							if ( pFundBondHolder == NULL || reqTXFunction.iHolderNo > 10 )
							{
								//strResult = Con_strInvalid;
								break;
							}
							//ȡ��Ŀǰ����ȡ�ü�¼�Ĺɶ����,���ͨ���������ҵ������ʱ����ߵ����һ����¼,��������Ҫ�����û�Ҫ��ID�ڽ���ѡ��
							nAccountingID = pFundBondHolder->iNo;		//�û�Ҫ��ʮ��ɶ����
							if ( nAccountingID < reqTXFunction.iHolderNo )
							{
								//strResult = Con_strInvalid;
								break;
							}
							if( nAccountingID > reqTXFunction.iHolderNo )
								pFundBondHolder = pFundBondHolder + reqTXFunction.iHolderNo - nAccountingID;  
							//while( nAccountingID > reqTXFunction.iHolderNo )
							//{
							//	nAccountingID--;
							//	pFundBondHolder--;
							//}
							switch(reqTXFunction.iFuncID)
							{
							case 141:	//��תծʮ�����������
								strResult = pFundBondHolder->bName;
								break;
							case 142:	//��תծʮ�������ծȯ���
								strResult.Format(_T("%.0f"), pFundBondHolder->dVolume);
								break;
							case 143:	//��תծʮ�������ծȯ������
								if (pFundBondHolder->dPercent != Con_doubleInvalid  )				
									strResult.Format(_T("%.4f"), pFundBondHolder->dPercent);
								break;
							default:
								break;
							}
							break;
						case 106:	//��תծת�ɼ۸�
							vdReq.data_type = dtype_double;
							if(!pBusiness->m_pSecurity->IsBond_Change())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate < reqTXFunction.nDate)
							{
								break;
							}
							dValue = GetDateIndicator_Value((INT)(reqTXFunction.nSecurityID), reqTXFunction.nDate, &vCBondPrice);
							if(dValue > 0)
							{
								strResult.Format(_T("%.2f"), dValue);
							}
							break;
						case 107:	//��תծ����
							vdReq.data_type = dtype_double;
							if(!pBusiness->m_pSecurity->IsBond_Change())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate < reqTXFunction.nDate)
							{
								break;
							}
							dValue = GetDateIndicator_Value((INT)(reqTXFunction.nSecurityID), reqTXFunction.nDate, &vCBondInterest);
							if(dValue > 0)
							{
								strResult.Format(_T("%.4f"), dValue / 100);
							}
							break;
						case 109:	//��תծת������
							vdReq.data_type = dtype_double;
							if(!pBusiness->m_pSecurity->IsBond_Change())
							{
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
							nDate = pBusiness->m_pSecurity->GetStopedLastDate();
							if(nDate > 0 && nDate < reqTXFunction.nDate)
							{
								break;
							}
							iRowNo = GetDateIndicator_Row((INT)(reqTXFunction.nSecurityID), reqTXFunction.nDate, &vCBondAmount);
							if(iRowNo >= 0)
							{
								dValue = 0;
								switch(reqTXFunction.iSubItemID)
								{
								case 1:		//��ת��ծȯ���
									tableIndDate[4].GetCell(2, (UINT)iRowNo, dValue);
									break;
								case 2:		//�ۼ�ת������
									tableIndDate[4].GetCell(3, (UINT)iRowNo, dValue);
									break;
								case 3:		//δת��ծȯ���
									tableIndDate[4].GetCell(4, (UINT)iRowNo, dValue);
									break;
								default:
									break;
								}
								if(dValue > 0)
								{
									strResult.Format(_T("%.2f"), dValue);
								}
							}
							if(dValue < 0.01 && reqTXFunction.iSubItemID == 3)
							{
								CString strDataPath = _T("");
								int id = m_pBusiness->m_pSecurity->GetSecurity1Id();
								strDataPath = m_pBusiness->DownloadXmlFile(id, 11004);
								strResult = GetXmlData( strDataPath,_T("T17"));
								///*XML Generate*/
								//OleInitialize(NULL);
								//Tx::Data::CXmlDocumentWrapper xmlDoc;
								////��ȡ����XML
								//if ( xmlDoc.Load((LPCTSTR)strDataPath))
								//{
								//	//�õ������
								//	Tx::Data::CXmlNodeWrapper rootNode(xmlDoc.AsNode());
								//	if(rootNode.IsValid())
								//	{
								//		Tx::Data::CXmlNodeWrapper leafNode(rootNode.FindNode(_T("tbody")));
								//		if(leafNode.IsValid())
								//		{
								//			strResult = leafNode.GetText();
								//		}
								//	}
								//}
								//OleUninitialize();
							}
							break;
						case 110:	//��תծ��������
							vdReq.data_type = dtype_double;
							if(!pBusiness->m_pSecurity->IsBond_Change())
							{
								break;
							}
							if(reqTXFunction.iSubItemID == 1)
							{	//���й�ģ
								pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
								if(pBondNewInfo != NULL)
								{
									dValue = pBondNewInfo->share;
									strResult.Format(_T("%.0f"), dValue);
								}
							}else
							{
								iRowNo = GetDateIndicator_Row((INT)(reqTXFunction.nSecurityID), 0, &vBondIPOInfo);
								if(iRowNo >= 0)
								{
									switch(reqTXFunction.iSubItemID)
									{
									case 2:		//���з���
										tableIndDate[5].GetCell(3, iRowNo, dValue);
										if(dValue > 0)
										{
											strResult.Format(_T("%.0f"), dValue);
										}
										break;
									case 3:		//���ʽ��
										tableIndDate[5].GetCell(4, iRowNo, dValue);
										if(dValue > 0)
										{
											strResult.Format(_T("%.0f"), dValue);
										}
										break;
									case 4:		//������ǩ��
										tableIndDate[5].GetCell(16, iRowNo, dValue);
										if(dValue > 0)
										{
											strResult.Format(_T("%.4f"), dValue / 100);
										}
										break;
									case 5:		//�������۱���
										tableIndDate[5].GetCell(18, iRowNo, dValue);
										if(dValue > 0)
										{
											strResult.Format(_T("%.4f"), dValue / 100);
										}
										break;
									case 6:		//�������۷�����
										tableIndDate[5].GetCell(15, iRowNo, dValue);
										if(dValue > 0)
										{
											strResult.Format(_T("%.0f"), dValue);
										}
										break;
									case 7:		//���»���������
										tableIndDate[5].GetCell(17, iRowNo, dValue);
										if(dValue > 0)
										{
											strResult.Format(_T("%.0f"), dValue);
										}
										break;
									case 8:		//�������̰�����
										tableIndDate[5].GetCell(19, iRowNo, dValue);
										if(dValue > 0)
										{
											strResult.Format(_T("%.0f"), dValue);
										}
										break;
									case 9:		//�ɶ����������
										tableIndDate[5].GetCell(14, iRowNo, dValue);
										if(dValue > 0)
										{
											strResult.Format(_T("%.0f"), dValue);
										}
										break;
									default:
										break;
									}
								}
							}
							break;
						case 113:	//��תծ-ת��ƽ��
							{
								vdReq.data_type = dtype_double;
								if(!pBusiness->m_pSecurity->IsBond_Change())
								{
									break;
								}
								FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
								nDate = pBusiness->m_pSecurity->GetStopedLastDate();
								if(nDate > 0 && nDate < reqTXFunction.nDate)
								{
									break;
								}
								//�޸ģ�2013-09-25
								BondParity* pBondParity = pBusiness->m_pSecurity->GetBondParityByDate(reqTXFunction.nDate);
								//pBondGeneral = pBusiness->m_pSecurity->GetBondGeneralByDate(reqTXFunction.nDate);
								if(pBondParity == NULL)
								{
									nDate = CTime::GetCurrentTime().GetYear() * 10000 + CTime::GetCurrentTime().GetMonth() * 100 + CTime::GetCurrentTime().GetDay();
									if(reqTXFunction.nDate == nDate && pBusiness->m_pSecurity->IsValid())
									{
										dValue = bond.CalcParity((INT)(reqTXFunction.nSecurityID), reqTXFunction.nDate);
										if(dValue > 0)
										{
											strResult.Format(_T("%.2f"), dValue);
										}
									}
								}else
								{
									//dValue = pBondGeneral->dParity;
									dValue = pBondParity->parity;
									if(dValue > 0)
									{
										strResult.Format(_T("%.2f"), dValue);
									}
								}
							}
							break;
						case 114:	//��תծ-ת�������
							{
								vdReq.data_type = dtype_double;
								if(!pBusiness->m_pSecurity->IsBond_Change())
								{
									break;
								}
								FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
								nDate = pBusiness->m_pSecurity->GetStopedLastDate();
								if(nDate > 0 && nDate < reqTXFunction.nDate)
								{
									break;
								}
								//�޸ģ�2013-05-29
								BondParity* pBondParity = pBusiness->m_pSecurity->GetBondParityByDate(reqTXFunction.nDate);
								//pBondGeneral = pBusiness->m_pSecurity->GetBondGeneralByDate(reqTXFunction.nDate);
								if(pBondParity == NULL)
								{
									nDate = CTime::GetCurrentTime().GetYear() * 10000 + CTime::GetCurrentTime().GetMonth() * 100 + CTime::GetCurrentTime().GetDay();
									if(reqTXFunction.nDate == nDate && pBusiness->m_pSecurity->IsValid())
									{
										dValue = bond.CalcPremium((INT)(reqTXFunction.nSecurityID), reqTXFunction.nDate);
										if(dValue != Tx::Core::Con_doubleInvalid)
										{
											strResult.Format(_T("%.4f"), dValue);
										}
									}
								}else
								{
									//dValue1 = pBondGeneral->dParity;
									dValue = pBondParity->parityPremium;
									//dValue2 = GetKLine(pBusiness, reqTXFunction.nDate, 4,reqTXFunction.nSecurityID);
									//if(dValue1 > 0 && dValue2 > 0)
									//{
									//	dValue = dValue2 / dValue1 - 1;
									if(dValue != Tx::Core::Con_doubleInvalid)
										strResult.Format(_T("%.4f"), dValue);
									//}
								}
							}
							break;
						case 115:	//��תծ-ת���׼�
							{
								vdReq.data_type = dtype_double;
								if(!pBusiness->m_pSecurity->IsBond_Change())
								{
									break;
								}
								FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
								nDate = pBusiness->m_pSecurity->GetStopedLastDate();
								if(nDate > 0 && nDate < reqTXFunction.nDate)
								{
									break;
								}
								BondParity* pBondParity = pBusiness->m_pSecurity->GetBondParityByDate(reqTXFunction.nDate);
								//pBondGeneral = pBusiness->m_pSecurity->GetBondGeneralByDate(reqTXFunction.nDate);
								if(pBondParity == NULL)
								{
									nDate = CTime::GetCurrentTime().GetYear() * 10000 + CTime::GetCurrentTime().GetMonth() * 100 + CTime::GetCurrentTime().GetDay();
									if(reqTXFunction.nDate == nDate && pBusiness->m_pSecurity->IsValid())
									{
										dValue = bond.GetFloor((INT)(reqTXFunction.nSecurityID));
										if(dValue != Tx::Core::Con_doubleInvalid)
										{
											strResult.Format(_T("%.2f"), dValue);
										}
									}
								}else
								{
									//dValue = pBondGeneral->dFloor;
									dValue = pBondParity->basePrice;
									if(dValue > 0)
									{
										strResult.Format(_T("%.2f"), dValue);
									}
								}
							}
							break;
						case 116:	//��תծ-�׼������
							{
								vdReq.data_type = dtype_double;
								if(!pBusiness->m_pSecurity->IsBond_Change())
								{
									break;
								}
								FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
								nDate = pBusiness->m_pSecurity->GetStopedLastDate();
								if(nDate > 0 && nDate < reqTXFunction.nDate)
								{
									break;
								}
								//��תծ-ת���׼�
								BondParity* pBondParity = pBusiness->m_pSecurity->GetBondParityByDate(reqTXFunction.nDate);
								//pBondGeneral = pBusiness->m_pSecurity->GetBondGeneralByDate(reqTXFunction.nDate);
								if(pBondParity == NULL)
								{
									nDate = CTime::GetCurrentTime().GetYear() * 10000 + CTime::GetCurrentTime().GetMonth() * 100 + CTime::GetCurrentTime().GetDay();
									if(reqTXFunction.nDate == nDate && pBusiness->m_pSecurity->IsValid())
									{
										dValue1 = bond.GetFloor((INT)(reqTXFunction.nSecurityID));
										dValue2 = GetKLine(pBusiness, reqTXFunction.nDate, 4,reqTXFunction.nSecurityID);
										if(dValue1 != Tx::Core::Con_floatInvalid && dValue2 != Tx::Core::Con_floatInvalid)
										{
											dValue = dValue2 / dValue1 - 1;
											strResult.Format(_T("%.4f"), dValue);
										}
									}
								}else
								{
									//dValue1 = pBondGeneral->dFloor;
									dValue1 = pBondParity->basePricePremium;
									//dValue2 = GetKLine(pBusiness, reqTXFunction.nDate, 4,reqTXFunction.nSecurityID);
									//if(dValue1 > 0 && dValue2 > 0)
									//{
									//	dValue = dValue2 / dValue1 - 1;
									if(dValue1 != Tx::Core::Con_floatInvalid)
										strResult.Format(_T("%.4f"), dValue1);
									//}
								}
							}
							break;
						case 151:	//��ʷ��ʱ�ļ۸�
						case 152:	//��ʷ��ʱ�ĳɽ���
						case 153:	//��ʷ��ʱ�ĳɽ����
							{
								strResult = Con_strInvalid;
								long nCount = 0;
								FindItemOfReq(&reqTXFunction, strReq, 19);	//Security��Date��iNo
								if ( reqTXFunction.nDate == CTime::GetCurrentTime().GetYear() * 10000 + CTime::GetCurrentTime().GetMonth() * 100 + CTime::GetCurrentTime().GetDay())
								{
									bool bRet = pBusiness->m_pSecurity->Request241();
									if ( !bRet )
										break;
									Tx::Data::HisTradeData* pHisTrade = pBusiness->m_pSecurity->Get241TradeData(reqTXFunction.iHolderNo-1);
									if ( pHisTrade == NULL )
										break;
									CString strFormatType = _T("%.2f");
									if( pBusiness->m_pSecurity->IsFund() || (pBusiness->m_pSecurity->IsStockB() && pBusiness->m_pSecurity->IsShanghai()) )
										strFormatType = _T("%.3f");
									switch(reqTXFunction.iFuncID )		
									{
									case 151:
										dValue = pHisTrade->Close;
										if ( dValue >= 0.0)
											strResult.Format(strFormatType,dValue);
										break;
									case 152:
										dValue = pHisTrade->Volume;
										if ( dValue >= 0.0)
											strResult.Format(strFormatType,dValue);
										break;
									case 153:
										dValue = pHisTrade->Amount;
										if ( dValue >= 0.0)
											strResult.Format(strFormatType,dValue);
										break;
									default:
										break;
									}

								}
								else
								{
									pHisDetail = pBusiness->m_pSecurity->GetHisTradeDataCustom( reqTXFunction.nDate, nCount);
									if ( pHisDetail == NULL || nCount <= 0 )
										break;
									CString strFormatType = _T("%.2f");
									if( pBusiness->m_pSecurity->IsFund() || (pBusiness->m_pSecurity->IsStockB() && pBusiness->m_pSecurity->IsShanghai()) )
										strFormatType = _T("%.3f");
									if ( reqTXFunction.iHolderNo <= nCount )
									{
										switch(reqTXFunction.iFuncID )		
										{
										case 151:
											dValue = pHisDetail[reqTXFunction.iHolderNo-1].Close;
											if ( dValue >= 0.0)
												strResult.Format(strFormatType,dValue);
											break;
										case 152:
											dValue = pHisDetail[reqTXFunction.iHolderNo-1].Volume;
											if ( dValue >= 0.0)
												strResult.Format(strFormatType,dValue);
											break;
										case 153:
											dValue = pHisDetail[reqTXFunction.iHolderNo-1].Amount;
											if ( dValue >= 0.0)
												strResult.Format(strFormatType,dValue);
											break;
										default:
											break;
										}
									}
								}
							}
							break;
						case 184:
							{
								//��ҵ���Ĭ��Ϊ0�����ش�/��/С��:998050/998051/998051��ȡ1ʱ�����ؼ�ֵ/�ɳ�:998065/998066��ȡ2ʱ�����ش�/С��ֵ:998063/998064
								strResult = Con_strInvalid;
								FindItemOfReq(&reqTXFunction, strReq, 22);	//��ҵ�㼶
								int iLevel = reqTXFunction.nIndustryLevel;
								if ( iLevel < 0 || iLevel > 2 )
									break;
								switch(iLevel)
								{
								case 0:
									if (IsSampleOfIndex( reqTXFunction.nSecurityID,4000382))
										strResult = _T("����");
									else
										if ( IsSampleOfIndex( reqTXFunction.nSecurityID,4000383))
											strResult = _T("����");
										else
											if ( IsSampleOfIndex( reqTXFunction.nSecurityID,4000384))
												strResult = _T("С��");
									break;
								case 1:
									if (IsSampleOfIndex( reqTXFunction.nSecurityID,4000397))
										strResult = _T("��ֵ");
									else
										if (IsSampleOfIndex( reqTXFunction.nSecurityID,4000398))
											strResult = _T("�ɳ�");
									break;
								case 2:
									if (IsSampleOfIndex( reqTXFunction.nSecurityID,4000395))
										strResult = _T("����ֵ");
									else
										if (IsSampleOfIndex( reqTXFunction.nSecurityID,4000396))
											strResult = _T("С��ֵ");
									break;
								}
							}
							break;
						case 185:
							{
								strResult = Con_strInvalid;
								FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
								if ( bPePb )
									dValue = Get_PE_Basic_NEW( reqTXFunction.nSecurityID, reqTXFunction.nDate );
								else
									dValue = Get_PE_Basic_NEW( reqTXFunction.nSecurityID, reqTXFunction.nDate ,false );
								if ( dValue != Con_doubleInvalid )
									strResult.Format("%.2f",dValue);
							}
							break;
						case 186:
							{
								strResult = Con_strInvalid;
								FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
								if ( bPePb )
									dValue = Get_PE_Ltm_NEW( reqTXFunction.nSecurityID, reqTXFunction.nDate );
								else
									dValue = Get_PE_Ltm_NEW( reqTXFunction.nSecurityID, reqTXFunction.nDate, false );
								if ( dValue != Con_doubleInvalid )
									strResult.Format("%.2f",dValue);
							}
							break;
						case 187:
							{
								strResult = Con_strInvalid;
								FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
								if( bPePb )
									dValue = Get_PE_Yoy_NEW( reqTXFunction.nSecurityID, reqTXFunction.nDate );
								else
									dValue = Get_PE_Yoy_NEW( reqTXFunction.nSecurityID, reqTXFunction.nDate, false );
								if ( dValue != Con_doubleInvalid )
									strResult.Format("%.2f",dValue);
							}
							break;
						case 188:
							{
								strResult = Con_strInvalid;
								FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
								if ( bPePb)
									dValue = Get_PB( reqTXFunction.nSecurityID, reqTXFunction.nDate );
								else
									dValue = Get_PB( reqTXFunction.nSecurityID, reqTXFunction.nDate, false );
								if ( dValue != Con_doubleInvalid )
									strResult.Format("%.2f",dValue);
							}
							break;
						//-----zhangxs--------20080808-------------------------
						case 154:	//������
							{
								strResult = _T("");
								FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
								if ( !pBusiness->m_pSecurity->IsFund())
									break;
								if(m_tableFundManager.GetRowCount() == 0)
									pBusiness->m_pLogicalBusiness->GetData(m_tableFundManager, true);
								vector <UINT> vRow;
								int iIndex = pBusiness->m_pSecurity->GetSecurity1Id();
								m_tableFundManager.Find(0, iIndex, vRow);
								if(vRow.size() ==0 )
									break;
								int nBegin = 0;
								int nEnd = 0;
								iSubItemID = (int)reqTXFunction.iSubItemID-1;
								CString strName = _T("");
								CString	str = _T("");
								int nValue = 0;
								int nQId = pBusiness->m_pSecurity->GetSecurity1Id();
								int i = 0;
								for( vector<UINT>::iterator iter = vRow.begin(); iter != vRow.end(); ++iter,i++ )
								{
									m_tableFundManager.GetCell( 4,*iter,nBegin);
									m_tableFundManager.GetCell( 5,*iter,nEnd );
									if( reqTXFunction.nDate >= nBegin && (reqTXFunction.nDate <= nEnd || nEnd == Con_intInvalid))
									{
										m_tableFundManager.GetCell( 2,*iter,strName);
										switch( iSubItemID )
										{
										case 0:		//����������
											str = strName;
											break;
										case 1:		//������ְλ
											m_tableFundManager.GetCell( 3,*iter,str);
											break;
										case 2:		//��ʼ����
											m_tableFundManager.GetCell( 4,*iter,nValue);
											str.Format(_T("%d"),nValue);
											break;
										case 3:		//��������
											m_tableFundManager.GetCell( 5,*iter,nValue);
											if(nValue ==Con_intInvalid )
												str = _T("����");
											else
												str.Format(_T("%d"),nValue);
											break;
										case 4:		//�Ա�
											m_tableFundManager.GetCell( 6,*iter,str);
											nValue = _ttoi(str.GetBuffer(0));
											//m_tableFundManager.GetCell( 6,*iter,nValue);
											if(nValue == 1)
												str = _T("��");
											else
												str = _T("Ů");
											break;
										case 5:		//ѧ��
											m_tableFundManager.GetCell( 7,*iter,str);
											break;
										case 6:		//����
											m_tableFundManager.GetCell( 8,*iter,nValue);
											if(nValue ==Con_intInvalid )
												str = _T("δ�ṩ");
											else 
												str.Format(_T("%d"),nValue);
											break;
										case 7:		//����
											//m_tableFundManager.GetCell( 9,*iter,str);
											{
												vdReq.data_type = dtype_val_string;
												//CString strDataMean = _T("����_��������");
												//Tx::Core::Table resTable;
												//pBusiness->m_pFunctionDataManager->GetDataFromXML( strDataMean,10410,pBusiness->m_pSecurity->GetId(),resTable);
												//strPath = Tx::Core::Manage::GetInstance()->m_pSystemPath->GetXMLPath() + _T("\\10410\\");
												CString strPath = pBusiness->DownloadXmlFile(pBusiness->m_pSecurity->GetSecurity1Id(),10410);
												//CString strFileName = _T("");
												//strFileName.Format(_T("%d.xml"),nQId);
												//strPath += strFileName;
												CMarkup xml;
												bool bload = false;
												bload = xml.Load( strPath );
												if ( bload)
												{
													if ( !xml.FindElem("data"))
														break;
													xml.IntoElem();
													if ( !xml.FindChildElem("tbody"))
														break;
													xml.IntoElem();
													while ( xml.FindChildElem("tr"))
													{
														xml.IntoElem();
														if(xml.FindChildElem("T7"))
														{
															str = xml.GetChildData();
															if ( atoi(str ) == nBegin )
															{
																xml.FindChildElem( "T9");
																str = xml.GetChildData();
																break;
															}
														}
														xml.OutOfElem();
													}
												}
											}
											break;
										default:
											break;
										}
										//strResult += strName + _T(":") + str;
										if(i == vRow.size()-1)
											strResult += str;
										else
											strResult += (str + _T("��"));
									}
								}
							}
							break;								
						case 155:	//���ʽ��������Ϣ
							strResult = Con_strInvalid;
							FindItemOfReq(&reqTXFunction, strReq,24);
							if( pBusiness->m_pSecurity->IsFund_Close())
							{
								FundCloseIssueInfo *pIssueInfo = pBusiness->m_pSecurity->GetFundCloseIssueInfo();
								if( pIssueInfo == NULL )
									break;
								iSubItemID = (int)reqTXFunction.iSubItemID-1;
								switch(iSubItemID)
								{
								case 0:		//��ʼ����
									strResult.Format( _T("%d"),pIssueInfo->start_date);
									break;
								case 1:		//��������
									strResult.Format( _T("%d"),pIssueInfo->end_date);
									break;
								case 2:		//���з�ʽ
									if( (CString)pIssueInfo->issue_mode != Con_strInvalid)
										strResult.Format( _T("%s"), (CString)pIssueInfo->issue_mode);
									break;
								case 3:		//��������
									if((CString)pIssueInfo->issue_type != Con_strInvalid)
										strResult.Format( _T("%s"),(CString)pIssueInfo->issue_type);
									break;
								case 4:		//������
									if(pIssueInfo->dValue[0] != Con_doubleInvalid)
										strResult.Format( _T("%d"),pIssueInfo->dValue[0]);
									break;
								case 5:		//��ֵ
									if(pIssueInfo->dValue[1] != Con_doubleInvalid)
										strResult.Format( _T("%.2f"),pIssueInfo->dValue[1]);
									break;
								case 6:		//���м�
									if(pIssueInfo->dValue[2] != Con_doubleInvalid)
										strResult.Format( _T("%.2f"),pIssueInfo->dValue[2]);
									break;
								case 7:		//���з���
									if(pIssueInfo->dValue[3] != Con_doubleInvalid)
										strResult.Format( _T("%.2f"),pIssueInfo->dValue[3]);
									break;
								case 8:		//��������ֵ
									if(pIssueInfo->dValue[4] != Con_doubleInvalid)
										strResult.Format( _T("%.2f"),pIssueInfo->dValue[4]);
									break;
								case 9:		//���ʽ��
									if(pIssueInfo->dValue[5] != Con_doubleInvalid)
										strResult.Format( _T("%.2f"),pIssueInfo->dValue[5]);
									break;
								case 10:	//��������
									strResult.Format( _T("%d"),pIssueInfo->list_date);
									break;
								case 11:	//���ο���ͨ�ݶ�
									if(pIssueInfo->dValue[6] != Con_doubleInvalid)
										strResult.Format( _T("%.2f"),pIssueInfo->dValue[6]);
									break;
								case 12:	//��Ч�깺����
									if ( pIssueInfo->dValue[7] != Con_doubleInvalid && pIssueInfo->dValue[7] > 0.0 )
										strResult.Format( _T("%.0f"),pIssueInfo->dValue[7]);
									break;
								case 13:	//��Ч�깺����
									if(pIssueInfo->dValue[8] != Con_doubleInvalid && pIssueInfo->dValue[8] > 0)
										strResult.Format( _T("%.0f"),pIssueInfo->dValue[8]);
									break;
								case 14:	//�����Ϲ�����
									if(pIssueInfo->dValue[9] != Con_doubleInvalid)
										strResult.Format( _T("%.2f"),pIssueInfo->dValue[9]);
									break;
								case 15:	//��ǩ��
									if(pIssueInfo->dValue[10] != Con_doubleInvalid)
										strResult.Format( _T("%.2f"),pIssueInfo->dValue[10]);
									break;
								case 16:		//�깺����
									strResult.Format( _T("%d"),pIssueInfo->applyid);
									break;
								case 17:	//����������
									if(pIssueInfo->dValue[11] != Con_doubleInvalid)
										strResult.Format( _T("%.2f"),pIssueInfo->dValue[11]);
									break;
								case 18:	//���ۻ���������
									if ( pIssueInfo->list_date_add != Con_intInvalid &&pIssueInfo->list_date_add > 0 )
										strResult.Format( _T("%d"),pIssueInfo->list_date_add);
									break;
								case 19:	//��ļ����¶����
									strResult.Format( _T("%s"),pIssueInfo->enlist_info_on_newspaper);
									break;
								case 20:	//��������¶����
									strResult.Format( _T("%s"),pIssueInfo->list_info_on_newspaper);
									break;
								case 21:	//��������������
									if(pIssueInfo->dValue[12] != Con_doubleInvalid)
										strResult.Format( _T("%.2f"),pIssueInfo->dValue[12]);
									break;
								case 22:	//����������
									
									strResult.Format( _T("%d"),pIssueInfo->issue_date);
									break;
								case 23:	//������������
									if(pIssueInfo->dValue[13] != Con_doubleInvalid)
										strResult.Format( _T("%.2f"),pIssueInfo->dValue[13]);
									break;
								default:
									break;
								}
							}
							break;
						case 156:	//����ʽ��������Ϣ
							strResult = Con_strInvalid;
							FindItemOfReq(&reqTXFunction, strReq,24);
							if( pBusiness->m_pSecurity->IsFund_Open())
							{
								FundOpenIssueInfo *pIssueInfo = pBusiness->m_pSecurity->GetFundOpenIssueInfo();
								if( pIssueInfo == NULL )
									break;
								iSubItemID = (int)reqTXFunction.iSubItemID-1;
								switch(iSubItemID)
								{
								case 0:		//��ʼ����
									strResult.Format( _T("%d"),pIssueInfo->start_date);
									break;
								case 1:		//��������
									strResult.Format( _T("%d"),pIssueInfo->end_date);
									break;
								case 2:		//�������۶�
									if(pIssueInfo->dValue[0] != Con_doubleInvalid)
										strResult.Format( _T("%.2f"),pIssueInfo->dValue[0]);
									break;
								case 3:		//������Ϣ
									if(pIssueInfo->dValue[1] != Con_doubleInvalid)
										strResult.Format( _T("%.2f"),pIssueInfo->dValue[1]);
									break;
								case 4:		//�ϼ�ļ���ݶ�
									if(pIssueInfo->dValue[2] != Con_doubleInvalid)
										strResult.Format( _T("%.2f"),pIssueInfo->dValue[2]);
									break;
								case 5:		//��Ч�Ϲ�����
									if(pIssueInfo->dValue[3] != Con_doubleInvalid)
										strResult.Format( _T("%.0f"),pIssueInfo->dValue[3]);
									break;
								case 6:		//�깺��ؿ�ʼ��
									if ( pIssueInfo->redeem_date != Con_intInvalid && pIssueInfo->redeem_date > 0 )
										strResult.Format( _T("%d"),pIssueInfo->redeem_date);
									break;
								default:
									break;
								}
							}
							break;
						case 157:	//�ʲ����
							strResult = Con_strInvalid;
							FindItemOfReq(&reqTXFunction, strReq,24);
							nInstitution = (INT)pBusiness->m_pSecurity->GetSecurity1Id();
							iSubItemID = (int)reqTXFunction.iSubItemID-1;
							pFundInvesmentGroup = NULL;
							nAccountingID = (INT64)reqTXFunction.nFYear * 10000 +  (INT64)reqTXFunction.nFQuarter;
							pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_FundInvesmentGroup,nAccountingID,&pFundInvesmentGroup,false);
							if ( pFundInvesmentGroup == NULL )
								break;
							dValue = pFundInvesmentGroup->dValue[iSubItemID];
							if( dValue !=Con_doubleInvalid )
									strResult.Format( _T("%.2f"),dValue);
							break;
						case 158:	//����ݶ�䶯
							strResult = Con_strInvalid;
							FindItemOfReq(&reqTXFunction, strReq,24);
							nInstitution = (INT)pBusiness->m_pSecurity->GetSecurity1Id();
							iSubItemID = (int)reqTXFunction.iSubItemID-1;
							pFundShareDataChange = NULL;
							if ( bFundShareDataChange )
							{
								nAccountingID = (INT64)reqTXFunction.nFYear * 10000 +  (INT64)reqTXFunction.nFQuarter;
								pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_FundShareDataChange,nAccountingID,&pFundShareDataChange,false);
							}
							else
							{
								nAccountingID = nInstitution;
								//�����������ļ�	
								bool bLoaded = false;
								if(pFundShareDataChangeDataFile!=NULL)
									bLoaded = pFundShareDataChangeDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30192,true);
								if ( bLoaded )
									pFundShareDataChange = pFundShareDataChangeDataFile->GetDataByObj(nAccountingID,false);
							}
							if(pFundShareDataChange == NULL )
								break;
							switch(iSubItemID)
							{
							case 0:	//�ڳ��ܷݶ�
								//strResult.Format( _T("%.2f"),pFundShareDataChange->F_SHARE_BEGIN);
								dValue = pFundShareDataChange->F_SHARE_BEGIN;
								if( dValue !=Con_doubleInvalid )
									strResult.Format( _T("%.2f"),dValue);
								break;
							case 1:	//�ڼ����깺�ݶ�								
								dValue = pFundShareDataChange->F_SHARE_SUBSCRIBE;
								if( dValue !=Con_doubleInvalid )
									strResult.Format( _T("%.2f"),dValue);
								break;
							case 2:	//�ڼ�����طݶ�								
								dValue = pFundShareDataChange->F_SHARE_CALL_BACK;
								if( dValue !=Con_doubleInvalid )
									strResult.Format( _T("%.2f"),dValue);
								break;
							case 3:	//��ĩ�ܷݶ�
								dValue = pFundShareDataChange->F_SHARE_END;
								if( dValue !=Con_doubleInvalid )
									strResult.Format( _T("%.2f"),dValue);
								break;
							default:
								break;
							}
							break;
						case 160:
						case 161:
						case 162:
						case 189:
						case 282:
							strResult = Con_strInvalid;
							FindItemOfReq(&reqTXFunction, strReq,24);
							nInstitution = (INT)pBusiness->m_pSecurity->GetSecurity1Id();
							iSubItemID = (int)reqTXFunction.iSubItemID-1;
							switch(reqTXFunction.iFuncID)
							{
							case 160:  //������ҵ�ֲ�
								pFundStockInvDistribute = NULL;
								if ( bFundStockIndustryDistribute )
								{
									nAccountingID = (INT64)reqTXFunction.nFYear * 10000 +  (INT64)reqTXFunction.nFQuarter;
									pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_FundStockInvestmentIndustryDistribute,nAccountingID,&pFundStockInvDistribute,false);
								}
								else
								{
									nAccountingID = nInstitution;
									//�����������ļ�	
									bool bLoaded = false;
									if(pFundStockInvDistributeDataFile!=NULL)
										bLoaded = pFundStockInvDistributeDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30157,true);
									if ( bLoaded )
										pFundStockInvDistribute = pFundStockInvDistributeDataFile->GetDataByObj(nAccountingID,false);
								}
								if ( pFundStockInvDistribute!=NULL )
									dValue = pFundStockInvDistribute->dFinancial[iSubItemID];
								if( dValue !=Con_doubleInvalid )
									strResult.Format( _T("%.2f"),dValue);
								break;
							case 161://ָ����ҵ�ֲ�
								pFundIndixInvDistribution = NULL;
								if ( bFundIndexIndustryDistribute )
								{
									nAccountingID = (INT64)reqTXFunction.nFYear * 10000 +  (INT64)reqTXFunction.nFQuarter;
									pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_FundIndexInvestmentIndustryDistribute,nAccountingID,&pFundIndixInvDistribution,false);
								}
								else
								{
									nAccountingID = nInstitution;
									//�����������ļ�	
									bool bLoaded = false;
									if(pFundIndixInvDistributionDataFile!=NULL)
										bLoaded = pFundIndixInvDistributionDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30159,true);
									if ( bLoaded )
										pFundIndixInvDistribution = pFundIndixInvDistributionDataFile->GetDataByObj(nAccountingID,false);
								}
								if ( pFundIndixInvDistribution!=NULL )
									dValue = pFundIndixInvDistribution->dFinancial[iSubItemID];
								if( dValue !=Con_doubleInvalid )
									strResult.Format( _T("%.2f"),dValue);
								break;
							case 162://�ϲ���ҵ�ֲ�
								pFundComInvDistribute = NULL;
								if ( bFundCombineIndustryDistribute )
								{
									nAccountingID = (INT64)reqTXFunction.nFYear * 10000 +  (INT64)reqTXFunction.nFQuarter;
									pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_FundCombineInvestmentIndustryDistribute,nAccountingID,&pFundComInvDistribute,false);
								}
								else
								{
									nAccountingID = nInstitution;
									//�����������ļ�	
									bool bLoaded = false;
									if(pFundComInvDistributeDataFile!=NULL)
										bLoaded = pFundComInvDistributeDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30155,true);
									if ( bLoaded )
										pFundComInvDistribute = pFundComInvDistributeDataFile->GetDataByObj(nAccountingID,false);
								}
								if ( pFundComInvDistribute!=NULL )
									dValue = pFundComInvDistribute->dFinancial[iSubItemID];
								if( dValue != Con_doubleInvalid )
									strResult.Format( _T("%.2f"),dValue);
								break;
							case 282:
								{
									//FindItemOfReq(&reqTXFunction, strReq,24);
									nDate = reqTXFunction.nFYear * 10000 + reqTXFunction.nFQuarter;
									// ����ʵ��ID �������ڡ���ҵID
									int iIndustryID = reqTXFunction.iSubItemID;
									dValue = GetFundCombineIndustryDistribute(nInstitution,nDate,iIndustryID);
									if (dValue != Con_doubleInvalid)
									{
										strResult.Format(_T("%.2f"),dValue);
									}
								}
								break;
							case 189:
								pFundTXInvDistribution = NULL;
								if ( bFundTXIndustryDistribute )
								{
									nAccountingID = (INT64)reqTXFunction.nFYear * 10000 +  (INT64)reqTXFunction.nFQuarter;
									pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_FundBlockInvestmentTxIndustryDistribute,nAccountingID,&pFundTXInvDistribution,false);
								}
								else
								{
									nAccountingID = nInstitution;
									//�����������ļ�	
									bool bLoaded = false;
									if(pFundTXInvDistributionDataFile!=NULL)
										bLoaded = pFundTXInvDistributionDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30161,true);
									if ( bLoaded )
										pFundTXInvDistribution = pFundTXInvDistributionDataFile->GetDataByObj(nAccountingID,false);
								}
								if ( pFundTXInvDistribution!=NULL )
									dValue = pFundTXInvDistribution->dFinancial[iSubItemID];
								if( dValue != Con_doubleInvalid )
									strResult.Format( _T("%.2f"),dValue);
								break;
							}
							break;

						case 166://�ֲֽṹ
							strResult = Con_strInvalid;
							FindItemOfReq(&reqTXFunction, strReq,24);					
							iSubItemID = (int)reqTXFunction.iSubItemID-1;
							pFundPositionInfo = NULL;
							switch ( reqTXFunction.nFQuarter )
							{
							case 3:	//�б�
								nAccountingID = (INT64)reqTXFunction.nFYear * 10000 + 630;
								break;
							case 9:	//�걨
								nAccountingID = (INT64)reqTXFunction.nFYear * 10000 + 1231;
								break;
							default:
								nAccountingID = 0;
								break;
							}
							//nAccountingID = (INT64)reqTXFunction.nFYear * 10000 +  (INT64)reqTXFunction.nFQuarter;
							pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_FundPositionInfo,nAccountingID,&pFundPositionInfo,false);
							if( pFundPositionInfo == NULL )
								break;
							switch (iSubItemID)
							{
							case 0:	//��������
								//strResult.Format( _T("%d"),pFundPositionInfo->f_end_date);
								dValue = pFundPositionInfo->f_end_date;
								if( dValue !=Con_doubleInvalid )
									strResult.Format( _T("%d"),(int)dValue);
								break;
							case 1:	//��ĩ�����˻���
								dValue = pFundPositionInfo->f_shareholder_num;
								if( dValue !=Con_doubleInvalid )
									strResult.Format( _T("%d"),(int)dValue);
								break;
							case 2:	//ƽ��ÿ�����зݶ�
								dValue = pFundPositionInfo->f_avg_share_per_person;
								if( dValue !=Con_doubleInvalid )
									strResult.Format( _T("%.2f"),dValue);
								break;
							case 3:	//����Ͷ���߳��зݶ�
								dValue = pFundPositionInfo->f_share_belong_to_institution;
								if( dValue !=Con_doubleInvalid )
									strResult.Format( _T("%.2f"),dValue);
								break;
							case 4:	//����Ͷ���߳��зݶ�
								dValue = pFundPositionInfo->f_share_belong_to_person;
								if( dValue !=Con_doubleInvalid )
									strResult.Format( _T("%.2f"),dValue);
								break;
							case 5:	//���˳��л���
								dValue = pFundPositionInfo->f_shareholder_num_person;
								if( dValue !=Con_doubleInvalid )
									strResult.Format( _T("%.2f"),dValue);
								break;
							case 6:	//�������л���
								dValue = pFundPositionInfo->f_shareholder_num_institution;
								if( dValue !=Con_doubleInvalid )
									strResult.Format( _T("%.2f"),dValue);
								break;
							default:
								break;
							}
							break;
						case 167:
						case 168:
						case 169:
						case 170:
						case 171:						
							strResult = Con_strInvalid;
							FindItemOfReq(&reqTXFunction, strReq,24);
							//nInstitution = (INT)pBusiness->m_pSecurity->GetId();
							iSubItemID = (int)reqTXFunction.iSubItemID-1;
							switch( reqTXFunction.iFuncID )
							{
							case 168://������Ҫ����ָ��
								pFundFinancial = NULL;
								if ( bFundFinancialInstitution )
								{
									nAccountingID = (INT64)reqTXFunction.nFYear * 10000 +  (INT64)reqTXFunction.nFQuarter;
									pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_FundFinancial,nAccountingID,&pFundFinancial,false);
								}
								else
								{
									nAccountingID = pBusiness->m_pSecurity->GetId();
									//�����������ļ�	
									bool bLoaded = false;
									if(pFundFinancialDataFile!=NULL)
										bLoaded = pFundFinancialDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30145,true);
									if ( bLoaded )
										pFundFinancial = pFundFinancialDataFile->GetDataByObj(nAccountingID,false);
								}
								if ( pFundFinancial!=NULL )
									dValue = pFundFinancial->dFinancial[iSubItemID];
								if( dValue !=Con_doubleInvalid )
									strResult.Format( _T("%.2f"),dValue);
								break;
							case 169://�����ʲ���ծ
								pFundBalance = NULL;
								if ( bFundBalanceInstitution )
								{
									nAccountingID = (INT64)reqTXFunction.nFYear * 10000 +  (INT64)reqTXFunction.nFQuarter;
									pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_FundBalance,nAccountingID,&pFundBalance,false);
								}
								else
								{
									nAccountingID = pBusiness->m_pSecurity->GetSecurity1Id();
									//�����������ļ�	
									bool bLoaded = false;
									if(pFundBalanceDataFile!=NULL)
										bLoaded = pFundBalanceDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30147,true);
									if ( bLoaded )
										pFundBalance = pFundBalanceDataFile->GetDataByObj(nAccountingID,false);
								}
								if ( pFundBalance!=NULL)
									dValue = pFundBalance->dFinancial[iSubItemID];
								if( dValue !=Con_doubleInvalid )
									strResult.Format( _T("%.2f"),dValue);
								break;
							case 170://����Ӫҵ��
								pFundAchievement = NULL;
								if( bFundAchievementInstitution )
								{
									//����ʵ��
									nAccountingID = (INT64)reqTXFunction.nFYear * 10000 +  (INT64)reqTXFunction.nFQuarter;
									pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_FundAchievement,nAccountingID,&pFundAchievement,false);
								}
								else
								{
									//����
									nAccountingID = pBusiness->m_pSecurity->GetSecurity1Id();
									//�����������ļ�	
									bool bLoaded = false;
									if(pFundAchievementDataFile!=NULL)
										bLoaded = pFundAchievementDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30149,true);
									if ( bLoaded )
										pFundAchievement = pFundAchievementDataFile->GetDataByObj(nAccountingID,false);
								}
								if ( pFundAchievement!=NULL)
									dValue = pFundAchievement->dFinancial[iSubItemID];
								if( dValue !=Con_doubleInvalid )
									strResult.Format( _T("%.2f"),dValue);
								break;
							case 171://�����������
								pFundRevenue = NULL;
								if( bFundRevenueInstitution )
								{
									//����ʵ��
									nAccountingID = (INT64)reqTXFunction.nFYear * 10000 +  (INT64)reqTXFunction.nFQuarter;
									pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_FundRevenue,nAccountingID,&pFundRevenue,false);
								}
								else
								{
									//����
									nAccountingID = pBusiness->m_pSecurity->GetSecurity1Id();
									//�����������ļ�	
									bool bLoaded = false;
									if(pFundRevenueDataFile!=NULL)
										bLoaded = pFundRevenueDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30151,true);
									if ( bLoaded )
										pFundRevenue = pFundRevenueDataFile->GetDataByObj(nAccountingID,false);
								}
								if ( pFundRevenue!=NULL)
									dValue = pFundRevenue->dFinancial[iSubItemID];
								if( dValue !=Con_doubleInvalid )
									strResult.Format( _T("%.2f"),dValue);
								break;
							case 167://����ֵ�䶯
								pFundNavChange = NULL;
								if( bFundNavChangeInstitution )
								{
									//����ʵ��
									nAccountingID = (INT64)reqTXFunction.nFYear * 10000 +  (INT64)reqTXFunction.nFQuarter;
									pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_FundNavChange,nAccountingID,&pFundNavChange,false);
								}
								else
								{
									//����
									nAccountingID = pBusiness->m_pSecurity->GetSecurity1Id();
									//�����������ļ�	
									bool bLoaded = false;
									if(pFundNavChangeDataFile!=NULL)
										bLoaded = pFundNavChangeDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30153,true);
									if ( bLoaded )
										pFundNavChange = pFundNavChangeDataFile->GetDataByObj(nAccountingID,false);
								}
								if ( pFundNavChange!=NULL)
									dValue = pFundNavChange->dFinancial[iSubItemID];
								if( dValue !=Con_doubleInvalid )
									strResult.Format( _T("%.2f"),dValue);
								break;
								
							default:
								strResult = strReq;
								break;
							}
							break;
							//-----------------------zhangxs----20080816--------
						case 163:	//�������زֹɽ���ʵ��
						case 164:	//�زֹ���ֵ
						case 165:	//�زֹ���ռ����
						case 172:	//�زֹɳֹ���
						case 173:	//�زֹ�ռ��ͨ�ɱ�
							switch(reqTXFunction.iFuncID)
							{
							case 163:	//�������زֹɽ���ʵ��
								vdReq.data_type = dtype_val_string;
								break;
							case 164:	//�������زֹ���ֵ
							case 165:	//�زֹ���ռ����
							case 172:	//�زֹɳֹ���
							case 173:	//�زֹ�ռ��ͨ�ɱ�
								vdReq.data_type = dtype_double;
								break;
							}
							strResult = Con_strInvalid;
							FindItemOfReq(&reqTXFunction, strReq, 18);	//Date��iHolderNo
							FindItemOfReq(&reqTXFunction, strReq, 24);
							pFundstockVipStock = NULL;
							if( bFundStockVIPStockInstitution )
							{
								//����ʵ��
								nAccountingID = (INT64)reqTXFunction.nFYear * 10000 +  (INT64)reqTXFunction.nFQuarter;
								pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_FundStockInvestmentVIPStock,nAccountingID,&pFundstockVipStock,false);
							}
							else
							{
								//����
								nAccountingID = nInstitution;
								//�����������ļ�	
								bool bLoaded = false;
								if(pFundstockVipStockDataFile!=NULL)
									bLoaded = pFundstockVipStockDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30165,true);
								if ( bLoaded )
									pFundstockVipStock = pFundstockVipStockDataFile->GetDataByObj(nAccountingID,false);
							}											
							if ( pFundstockVipStock == NULL || reqTXFunction.iHolderNo > 10 )
								break;
							//ȡ��Ŀǰ����ȡ�ü�¼�Ĺɶ����,���ͨ���������ҵ������ʱ����ߵ����һ����¼,��������Ҫ�����û�Ҫ��ID�ڽ���ѡ��
							nAccountingID = pFundstockVipStock->iMmarketValueOrder;		//ʮ���زֹ����
							if ( nAccountingID < reqTXFunction.iHolderNo )
								break;
							if( nAccountingID > reqTXFunction.iHolderNo )
								pFundstockVipStock = pFundstockVipStock + reqTXFunction.iHolderNo - nAccountingID;  
							switch(reqTXFunction.iFuncID)
							{
							case 163:	//�����ͻ���ʮ���زֹ�����
								{
									int iSecurity = pFundstockVipStock ->f2;
									Tx::Business::TxBusiness business;
									business.GetSecurityNow(iSecurity);
									strResult = business.m_pSecurity->GetName();
								}
								break;
							case 164:	//�����ͻ����زֹ���ֵ
								dValue = pFundstockVipStock ->dFinancial[0];
								if ( dValue != Con_doubleInvalid && dValue > 0.0 )
									strResult.Format(_T("%.2f"),dValue);
								break;
							case 165:	//�����ͻ����ز���ռ����			
								dValue = pFundstockVipStock ->dFinancial[1];
								if ( dValue != Con_doubleInvalid && dValue > 0.0 )
									strResult.Format(_T("%.2f"),100 * dValue);
								break;
							case 172:	//�����ͻ����زֳֹ���			
								dValue = pFundstockVipStock ->dFinancial[2];
								if ( dValue != Con_doubleInvalid && dValue > 0.0 )
									strResult.Format(_T("%.2f"),dValue);
								break;
							case 173:	//�����ͻ����ز�ռ��ͨ�ɱ�			
								dValue = pFundstockVipStock ->dFinancial[3];
								if ( dValue != Con_doubleInvalid && dValue > 0.0 )
									strResult.Format(_T("%.2f"),100 * dValue);
								break;
							default:
								break;
							}
							break;
						case 174:	//ָ�����زֹɽ���ʵ��
						case 175:	//�زֹ���ֵ
						case 176:	//�زֹ���ռ����
						case 177:	//�زֹɳֹ���
						case 178:	//�زֹ�ռ��ͨ�ɱ�
							switch(reqTXFunction.iFuncID)
							{
							case 174:	//�������زֹɽ���ʵ��
								vdReq.data_type = dtype_val_string;
								break;
							case 175:	//�������زֹ���ֵ
							case 176:	//�زֹ���ռ����
							case 177:	//�زֹɳֹ���
							case 178:	//�زֹ�ռ��ͨ�ɱ�
								vdReq.data_type = dtype_double;
								break;
							}
							FindItemOfReq(&reqTXFunction, strReq, 18);	//Date��iHolderNo
							FindItemOfReq(&reqTXFunction, strReq, 24);  //��������Ƽ�
							m_inDate = reqTXFunction.nDate/10 + reqTXFunction.nDate%10;
							strResult = Con_strInvalid;
							pFundIndexVipStock = NULL;
							if( bFundIndexVIPStockInstitution )
							{
								//����ʵ��
								nAccountingID = (INT64)reqTXFunction.nFYear * 10000 +  (INT64)reqTXFunction.nFQuarter;
								pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_FundIndexInvestmentVIPStock,nAccountingID,&pFundIndexVipStock,false);
							}
							else
							{
								//����
								nAccountingID = nInstitution;
								//�����������ļ�	
								bool bLoaded = false;
								if(pFundIndexVipStockDataFile!=NULL)
									bLoaded = pFundIndexVipStockDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30167,true);
								if ( bLoaded )
									pFundIndexVipStock = pFundIndexVipStockDataFile->GetDataByObj(nAccountingID,false);
							}											
							if ( pFundIndexVipStock == NULL || reqTXFunction.iHolderNo > 10 )
								break;
							//ȡ��Ŀǰ����ȡ�ü�¼�Ĺɶ����,���ͨ���������ҵ������ʱ����ߵ����һ����¼,��������Ҫ�����û�Ҫ��ID�ڽ���ѡ��
							nAccountingID = pFundIndexVipStock->iMmarketValueOrder;		//ʮ���زֹ����
							if ( nAccountingID < reqTXFunction.iHolderNo )
								break;
							if( nAccountingID > reqTXFunction.iHolderNo )
								pFundIndexVipStock = pFundIndexVipStock + reqTXFunction.iHolderNo - nAccountingID;  
							switch(reqTXFunction.iFuncID)
							{
							case 174:	//ָ���ͻ���ʮ���زֹ�����
								{
									int iSecurity = pFundIndexVipStock ->f2;
									Tx::Business::TxBusiness business;
									business.GetSecurityNow(iSecurity);
									strResult = business.m_pSecurity->GetName();
								}
								break;
							case 175:	//ָ���ͻ����زֹ���ֵ
								dValue = pFundIndexVipStock ->dFinancial[0];
								if ( dValue != Con_doubleInvalid && dValue > 0.0  )
									strResult.Format(_T("%.2f"), dValue );
								break;
							case 176:	//ָ���ͻ����ز���ռ����			
								dValue = pFundIndexVipStock ->dFinancial[1];
								if ( dValue != Con_doubleInvalid && dValue > 0.0  )
									strResult.Format(_T("%.2f"),100 * dValue );
								break;
							case 177:	//ָ���ͻ����زֳֹ���		
								dValue = pFundIndexVipStock ->dFinancial[2];
								if ( dValue != Con_doubleInvalid && dValue > 0.0  )
									strResult.Format(_T("%.2f"),dValue );
								break;
							case 178:	//ָ���ͻ����ز�ռ��ͨ�ɱ�	
								dValue = pFundIndexVipStock ->dFinancial[3];
								if ( dValue != Con_doubleInvalid && dValue > 0.0  )
									strResult.Format(_T("%.2f"), 100 * dValue );
								break;
							default:
								break;
							}
							break;
						case 179:	//�ϲ����زֹɽ���ʵ��
						case 180:	//�زֹ���ֵ
						case 181:	//�زֹ���ռ����
						case 182:	//�زֹɳֹ���
						case 183:	//�زֹ�ռ��ͨ�ɱ�
							switch(reqTXFunction.iFuncID)
							{
							case 179:	//�������زֹɽ���ʵ��
								vdReq.data_type = dtype_val_string;
								break;
							case 180:	//�ϲ����زֹ���ֵ
							case 181:	//�زֹ���ռ����
							case 182:	//�زֹɳֹ���
							case 183:	//�زֹ�ռ��ͨ�ɱ�
								vdReq.data_type = dtype_double;
								break;
							}
							strResult = Con_strInvalid;
							FindItemOfReq(&reqTXFunction, strReq, 18);	//Date��iHolderNo
							FindItemOfReq(&reqTXFunction, strReq, 24);  //��������Ƽ�
							m_inDate = reqTXFunction.nDate/10 + reqTXFunction.nDate%10;
							pFundCombineVipStock = NULL;
							if( bFundCombineVIPStockInstitution )
							{
								//����ʵ��
								nAccountingID = (INT64)reqTXFunction.nFYear * 10000 +  (INT64)reqTXFunction.nFQuarter;
								pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_FundCombineInvestmentVIPStock,nAccountingID,&pFundCombineVipStock,false);
							}
							else
							{
								//����
								nAccountingID = nInstitution;
								//�����������ļ�	
								bool bLoaded = false;
								if(pFundCombineVipStockDataFile!=NULL)
									bLoaded = pFundCombineVipStockDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30163,true);
								if ( bLoaded )
									pFundCombineVipStock = pFundCombineVipStockDataFile->GetDataByObj(nAccountingID,false);
							}												
							if ( pFundCombineVipStock == NULL || reqTXFunction.iHolderNo > 10 )
								break;
							//ȡ��Ŀǰ����ȡ�ü�¼�Ĺɶ����,���ͨ���������ҵ������ʱ����ߵ����һ����¼,��������Ҫ�����û�Ҫ��ID�ڽ���ѡ��
							nAccountingID = pFundCombineVipStock->iMmarketValueOrder;		//ʮ���زֹ����
							if ( nAccountingID < reqTXFunction.iHolderNo )
								break;
							if( nAccountingID >= reqTXFunction.iHolderNo )
								pFundCombineVipStock = pFundCombineVipStock + reqTXFunction.iHolderNo - nAccountingID;  
							switch(reqTXFunction.iFuncID)
							{
							case 179:	//�ϲ��ͻ���ʮ���زֹ�����
								{
									int iSecurity = pFundCombineVipStock ->f2;
									Tx::Business::TxBusiness m_business;
									m_business.GetSecurityNow(iSecurity);
									strResult = m_business.m_pSecurity->GetName();
								}
								break;
							case 180:	//�ϲ��ͻ����زֹ���ֵ
								dValue = pFundCombineVipStock ->dFinancial[0];
								if ( dValue != Con_doubleInvalid && dValue > 0.0  )
									strResult.Format(_T("%.2f"), dValue );
								break;
							case 181:	//�ϲ��ͻ����ز���ռ����			
								dValue = pFundCombineVipStock ->dFinancial[1];
								if ( dValue != Con_doubleInvalid && dValue > 0.0 )
									strResult.Format(_T("%.2f"),100*dValue );
								break;
							case 182:	//�ϲ��ͻ����زֳֹ���			
								dValue = pFundCombineVipStock ->dFinancial[2];
								if ( dValue != Con_doubleInvalid && dValue > 0.0 )
									strResult.Format(_T("%.2f"),dValue );
								break;
							case 183:	//�ϲ��ͻ����ز�ռ��ͨ�ɱ�			
								dValue = pFundCombineVipStock ->dFinancial[3];
								if ( dValue != Con_doubleInvalid && dValue > 0.0 )
									strResult.Format(_T("%.2f"), 100 * dValue );
								break;
							default:
								break;
							}
							break;
						case 190:	//ȯ��ϯλ
							{
								strResult = Con_strInvalid;
								FindItemOfReq(&reqTXFunction, strReq,19);//������iHolderNO
								FindItemOfReq(&reqTXFunction, strReq,24);
								nInstitution = (INT)pBusiness->m_pSecurity->GetSecurity1Id();
								iSubItemID = (int)reqTXFunction.iSubItemID-1;
								pFundTradeVolumeInfo = NULL;
								if( bFundTradeVolumeInfo )
								{
									//����ʵ��
									nAccountingID = (INT64)reqTXFunction.nFYear * 10000 +  (INT64)reqTXFunction.nFQuarter;
									pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_FundTradeVolumeInfo,nAccountingID,&pFundTradeVolumeInfo,false);
								}
								else
								{
									//����
									nAccountingID = nInstitution;
									//�����������ļ�	
									bool bLoaded = false;
									if(pFundTradeVolumeInfoDataFile!=NULL)
										bLoaded = pFundTradeVolumeInfoDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30187,true);
									if ( bLoaded )
										pFundTradeVolumeInfo = pFundTradeVolumeInfoDataFile->GetDataByObj(nAccountingID,false);
								}
								if ( pFundTradeVolumeInfo == NULL)
									break;
								nAccountingID = pFundTradeVolumeInfo->f1;
								if ( nAccountingID < reqTXFunction.iHolderNo )
									break;							
								pFundTradeVolumeInfo = pFundTradeVolumeInfo + reqTXFunction.iHolderNo - nAccountingID; 
								if(iSubItemID == 0 )
								{	
									//���������ļ���ȡȯ������
									strResult = Get_QS_Name( pFundTradeVolumeInfo->f2 );
								}
								else
								{
									dValue = pFundTradeVolumeInfo ->dValue[iSubItemID-1];
									if( dValue !=Con_doubleInvalid )
										strResult.Format( _T("%.2f"),dValue);
								}

							}
							break;
						case 191:	//�ֹ���ϸ
							{
								strResult = Con_strInvalid;
								FindItemOfReq(&reqTXFunction, strReq,19);//������iHolderNO
								FindItemOfReq(&reqTXFunction, strReq,24);
								nInstitution = (INT)pBusiness->m_pSecurity->GetSecurity1Id();
								iSubItemID = (int)reqTXFunction.iSubItemID-1;
								pFundHoldStockDetail = NULL;
								if( bFundHolderStockDetail )
								{
									//����ʵ��
									nAccountingID = (INT64)reqTXFunction.nFYear * 10000 +  (INT64)reqTXFunction.nFQuarter;
									pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_FundHoldStockDetail,nAccountingID,&pFundHoldStockDetail,false);
								}
								else
								{
									//����
									nAccountingID = nInstitution;
									//�����������ļ�	
									bool bLoaded = false;
									if(pFundHoldStockDetailDataFile!=NULL)
										bLoaded = pFundHoldStockDetailDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30189,true);
									if ( bLoaded )
										pFundHoldStockDetail = pFundHoldStockDetailDataFile->GetDataByObj(nAccountingID,false);
								}
								if ( pFundHoldStockDetail == NULL)
									break;
								nAccountingID = pFundHoldStockDetail ->f_no;
								if ( nAccountingID < reqTXFunction.iHolderNo )
									break;							
								if( nAccountingID >= reqTXFunction.iHolderNo )
									pFundHoldStockDetail = pFundHoldStockDetail + reqTXFunction.iHolderNo - nAccountingID; 
								switch(iSubItemID)
								{
								case 0:	//����ʵ��ID
									pBusiness->GetSecurityNow(pFundHoldStockDetail ->f2);
									strResult = pBusiness->m_pSecurity->GetName();
									break;
								case 1:	//�ֹ�����
									strResult.Format(_T("%.2f"), pFundHoldStockDetail ->dValue[0]);
									dValue = pFundHoldStockDetail ->dValue[0];
									if( dValue !=Con_doubleInvalid )
										strResult.Format( _T("%.2f"),dValue);
									break;
								case 2:	//�ֹ���ֵ
									dValue = pFundHoldStockDetail ->dValue[1];
									if( dValue !=Con_doubleInvalid )
										strResult.Format( _T("%.2f"),dValue);
									break;
								case 3:	//��ֵ����
									strResult.Format(_T("%.2f"), pFundHoldStockDetail ->f_market_value_order);
									dValue = pFundHoldStockDetail ->f_market_value_order;
									if( dValue !=Con_doubleInvalid )
										strResult.Format( _T("%d"),(int)dValue);
									break;
								case 4:	//��ռ����
									dValue = pFundHoldStockDetail ->dValue[2];
									if( dValue !=Con_doubleInvalid )
										strResult.Format( _T("%.2f"),100 * dValue);
									break;
								case 5:	//ռ��ͨ�ɱ���
									dValue = pFundHoldStockDetail ->dValue[3];
									if( dValue !=Con_doubleInvalid )
										strResult.Format( _T("%.2f"),100 * dValue);
									break;
								default:
									break;
								}
							}
							break;
						case 192:	//ծȯ���
							{
								strResult = Con_strInvalid;
								nInstitution = (INT)pBusiness->m_pSecurity->GetSecurity1Id();
								iSubItemID = (int)reqTXFunction.iSubItemID-1;
								pFundBondGroup = NULL;
								if ( bFundBondPortfolio)
								{
									//����ʵ��
									nAccountingID = (INT64)reqTXFunction.nFYear * 10000 +  (INT64)reqTXFunction.nFQuarter;
									pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_FundBondGroup,nAccountingID,&pFundBondGroup,false);
								}
								else
								{
									//����
									nAccountingID = nInstitution;
									//�����������ļ�	
									bool bLoaded = false;
									if(pFundBondGroupDataFile!=NULL)
										bLoaded = pFundBondGroupDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30194,true);
									if ( bLoaded )
										pFundBondGroup = pFundBondGroupDataFile->GetDataByObj(nAccountingID,false);
								}
								if ( pFundBondGroup == NULL)
									break;
								dValue = pFundBondGroup->dValue[iSubItemID];
								if (dValue != Con_doubleInvalid)
									strResult.Format( _T("%.2f"),dValue);
							}
							break;
						case 193:	//��ծ��ϸ
							{
								strResult = Con_strInvalid;
								FindItemOfReq(&reqTXFunction, strReq,19);//������iHolderNO
								nInstitution = (INT)pBusiness->m_pSecurity->GetSecurity1Id();
								iSubItemID = (int)reqTXFunction.iSubItemID-1;
								pFundBondDetail = NULL;
								if ( bFundBondHoldDetail )
								{
									//����ʵ��
									nAccountingID = (INT64)reqTXFunction.nFYear * 10000 +  (INT64)reqTXFunction.nFQuarter;
									pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_FundBondDetail,nAccountingID,&pFundBondDetail,false);
								}
								else
								{
									//����
									nAccountingID = nInstitution;
									//�����������ļ�	
									bool bLoaded = false;
									if(pFundBondDetailDataFile!=NULL)
										bLoaded = pFundBondDetailDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30196,true);
									if ( bLoaded )
										pFundBondDetail = pFundBondDetailDataFile->GetDataByObj(nAccountingID,false);
								}
								if ( pFundBondDetail == NULL)
									break;
								nAccountingID = pFundBondDetail->f_no;
								if ( reqTXFunction.iHolderNo > nAccountingID )
									break;
								pFundBondDetail = pFundBondDetail + reqTXFunction.iHolderNo - nAccountingID;
								switch( iSubItemID )
								{
								case 0:		//����
									pBusiness->GetSecurityNow( pFundBondDetail->f1 );
									if ( pBusiness->m_pSecurity != NULL )
										strResult = pBusiness->m_pSecurity->GetName();
									break;
								case 1:		//��ֵ
									if ( pFundBondDetail->dValue[0] != Con_doubleInvalid && pFundBondDetail->dValue[0] > 0.0 )
										strResult.Format(_T("%.2f"),pFundBondDetail->dValue[0]);
									break;
								case 2:		//����
									if ( pFundBondDetail->dValue[1] != Con_doubleInvalid && pFundBondDetail->dValue[1] > 0.0 && pFundBondDetail->dValue[1] <= 100.0)
										strResult.Format(_T("%.2f"),pFundBondDetail->dValue[1]);
									break;
								case 3:		//��ֵ����
									if ( pFundBondDetail->f_market_value_order > 0)
										strResult.Format(_T("%d"),pFundBondDetail->f_market_value_order);
									break;
								}
							}
							break;
						case 194:	//��תծ��ϸ
							{
								strResult = Con_strInvalid;
								FindItemOfReq(&reqTXFunction, strReq,19);//������iHolderNO
								nInstitution = (INT)pBusiness->m_pSecurity->GetSecurity1Id();
								iSubItemID = (int)reqTXFunction.iSubItemID-1;
								pFundCBondDetail = NULL;
								if ( bFundCBondHoldDetail )
								{
									//����ʵ��
									nAccountingID = (INT64)reqTXFunction.nFYear * 10000 +  (INT64)reqTXFunction.nFQuarter;
									pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_FundBondChangeDetail,nAccountingID,&pFundCBondDetail,false);
								}
								else
								{
									//����
									nAccountingID = nInstitution;
									//�����������ļ�	
									bool bLoaded = false;
									if(pFundCBondDetailDataFile!=NULL)
										bLoaded = pFundCBondDetailDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30198,true);
									if ( bLoaded )
										pFundCBondDetail = pFundCBondDetailDataFile->GetDataByObj(nAccountingID,false);
								}
								if ( pFundCBondDetail == NULL)
									break;
								nAccountingID = pFundCBondDetail->f_no;
								if ( reqTXFunction.iHolderNo > nAccountingID )
									break;
								pFundCBondDetail = pFundCBondDetail + reqTXFunction.iHolderNo - nAccountingID;
								switch( iSubItemID )
								{
								case 0:		//����
									pBusiness->GetSecurityNow( pFundCBondDetail->f1 );
									if ( pBusiness->m_pSecurity != NULL )
										strResult = pBusiness->m_pSecurity->GetName();
									break;
								case 1:		//��ֵ
									if ( pFundCBondDetail->f2 != Con_doubleInvalid && pFundCBondDetail->f2 > 0.0 )
										strResult.Format(_T("%.2f"),pFundCBondDetail->f2 );
									break;
								case 2:		//����    20100701 wanglm ��Щ������ʾ��������ԭ�����ж�������������  // && pFundCBondDetail->f3 <= 1.0
									if ( pFundCBondDetail->f3 != Con_doubleInvalid && pFundCBondDetail->f3 > 0.0 )   
									{
										double dVal = 0;
										dVal = pFundCBondDetail->f3;
										strResult.Format(_T("%.2f"), dVal );								
									}
									break;
								case 3:		//��ֵ����
									if ( pFundCBondDetail->f_market_value_order > 0)
										strResult.Format(_T("%d"),pFundCBondDetail->f_market_value_order);
									break;
								}
							}
							break;
						case 195:	//�����ʲ�,����ط������ݽ���Map��ʱ��ͬǰ�ߵĲ�һ��,
							{
								strResult = Con_strInvalid;
								FindItemOfReq(&reqTXFunction, strReq,19);//������iHolderNO
								nInstitution = (INT)pBusiness->m_pSecurity->GetSecurity1Id();
								iSubItemID = (int)reqTXFunction.iSubItemID-1;
								pFundOtherAsset = NULL;
								if ( bnFundOtherAsset)
								{
									nAccountingID = (INT64)reqTXFunction.nFYear * 10000 +  (INT64)reqTXFunction.nFQuarter;
									pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_FundOtherAsset,nAccountingID,&pFundOtherAsset,false);
								}
								else
								{
									//����
									nAccountingID = nInstitution;
									//�����������ļ�	
									bool bLoaded = false;
									if(pFundOtherAssetDataFile!=NULL)
										bLoaded = pFundOtherAssetDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30200,true);
									if ( bLoaded )
										pFundOtherAsset = pFundOtherAssetDataFile->GetDataByObj(nAccountingID,false);
								}
								if ( pFundOtherAsset == NULL  )
									break;
								Tx::Data::FundOtherAsset* pTemp = pFundOtherAsset;
								nAccountingID = 0;
								while( pTemp->iIndex == pFundOtherAsset->iIndex )
								{
									pTemp++;
									nAccountingID++;
								}
								if ( reqTXFunction.iHolderNo > nAccountingID )
									break;
								pFundOtherAsset = pFundOtherAsset + reqTXFunction.iHolderNo -1;
								//������ʱ��֧���ʲ����ͣ��ȴ����������Ϊָ���ʱ�����
								switch( iSubItemID )
								{
								case 0:		//��ֵ
									if ( pFundOtherAsset->f2 != Con_doubleInvalid && pFundOtherAsset->f2 > 0.0)
										strResult.Format(_T("%.2f"),pFundOtherAsset->f2);
									break;
								case 1:		//ռ��
									if ( pFundOtherAsset->f3 != Con_doubleInvalid && pFundOtherAsset->f3 > 0.0 && pFundOtherAsset->f3 < 1.0)
										strResult.Format(_T("%.2f"),pFundOtherAsset->f3);
									break;
								}
							}
							break;
						case 196:	//�ۼ������Ʊ
							strResult = Con_strInvalid;
							FindItemOfReq(&reqTXFunction, strReq,19);//������iHolderNO
							FindItemOfReq(&reqTXFunction, strReq,24);
							nInstitution = (INT)pBusiness->m_pSecurity->GetSecurity1Id();
							iSubItemID = (int)reqTXFunction.iSubItemID-1;
							pFundBuyTotStock = NULL;
							if( bFundBuyTotStock )
							{
								//����ʵ��
								nAccountingID = (INT64)reqTXFunction.nFYear * 10000 +  (INT64)reqTXFunction.nFQuarter;
								pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_FundBuyTotStock,nAccountingID,&pFundBuyTotStock,false);
							}
							else
							{
								//����
								nAccountingID = nInstitution;
								//�����������ļ�	
								bool bLoaded = false;
								if(pFundBuyTotStockDataFile!=NULL)
									bLoaded = pFundBuyTotStockDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30177,true);
								if ( bLoaded )
									pFundBuyTotStock = pFundBuyTotStockDataFile->GetDataByObj(nAccountingID,false);
							}
							if ( pFundBuyTotStock == NULL)
								break;
							nAccountingID = pFundBuyTotStock ->no;
							if ( nAccountingID < reqTXFunction.iHolderNo )
								break;							
							if( nAccountingID >= reqTXFunction.iHolderNo )
							{
								pFundBuyTotStock = pFundBuyTotStock + reqTXFunction.iHolderNo - nAccountingID; 
								if(iSubItemID == 0)
								{
									pBusiness->GetSecurityNow(pFundBuyTotStock ->security_id);
									strResult = pBusiness->m_pSecurity->GetName();
								}
								else if(iSubItemID == 1 )
								{
									dValue = pFundBuyTotStock->amount;
									if( dValue !=Con_doubleInvalid )
										strResult.Format( _T("%.2f"),dValue);
								}
								else if(iSubItemID == 2 )
								{
									dValue = pFundBuyTotStock->ratio;
									if( dValue !=Con_doubleInvalid )
										strResult.Format( _T("%.2f"),dValue);
								}
							}
							break;
						case 197://�ۼ�������Ʊ
							strResult = Con_strInvalid;
							FindItemOfReq(&reqTXFunction, strReq,19);//������iHolderNO
							FindItemOfReq(&reqTXFunction, strReq,24);
							nInstitution = (INT)pBusiness->m_pSecurity->GetSecurity1Id();
							iSubItemID = (int)reqTXFunction.iSubItemID-1;
							pFundSaleTotStock = NULL;
							if( bFundSaleTotStock )
							{
								//����ʵ��
								nAccountingID = (INT64)reqTXFunction.nFYear * 10000 +  (INT64)reqTXFunction.nFQuarter;
								pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_FundSaleTotStockForSecurity,nAccountingID,&pFundSaleTotStock,false);
							}
							else
							{
								//����
								nAccountingID = nInstitution;
								//�����������ļ�	
								bool bLoaded = false;
								if(pFundSaleTotStockDataFile!=NULL)
									bLoaded = pFundSaleTotStockDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter,30177,true);
								if ( bLoaded )
									pFundSaleTotStock = pFundSaleTotStockDataFile->GetDataByObj(nAccountingID,false);
							}
							if ( pFundSaleTotStock == NULL)
								break;
							nAccountingID = pFundSaleTotStock ->no;
							if ( nAccountingID < reqTXFunction.iHolderNo )
								break;							
							if( nAccountingID >= reqTXFunction.iHolderNo )
							{
								pFundSaleTotStock = pFundSaleTotStock + reqTXFunction.iHolderNo - nAccountingID; 
								if (iSubItemID < 3)
								{
									
									if(iSubItemID == 0)
									{
										pBusiness->GetSecurityNow(pFundSaleTotStock ->security_id);
										strResult = pBusiness->m_pSecurity->GetName();
									}
									else if(iSubItemID == 1 )
									{
										dValue = pFundSaleTotStock->amount;
										if( dValue !=Con_doubleInvalid )
											strResult.Format( _T("%.2f"),dValue);
									}
									else if(iSubItemID == 2 )
									{
										dValue = pFundSaleTotStock->ratio;
										if( dValue !=Con_doubleInvalid )
											strResult.Format( _T("%.2f"),dValue);
									}
								}
							}
							break;
						case 198:	//����Ԥ��EPS
						case 206:	//�г�Ԥ��EPS
							{
								strResult = Con_strInvalid;
								pEpsEstimate = NULL;
								FindItemOfReq(&reqTXFunction, strReq,20);
								pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_EarningsEstimateTx,reqTXFunction.nFYear,&pEpsEstimate,false);
								if ( pEpsEstimate == NULL )
									break;
								switch( reqTXFunction.iFuncID )
								{
								case 198:
									dValue = pEpsEstimate->f_tx_eps;
									break;
								case 206:
									dValue = pEpsEstimate->f_tx_ave;
									break;
								default:
									dValue = Con_doubleInvalid;
									break;
								}
								if ( dValue != Con_doubleInvalid )
									strResult.Format( _T("%.2f"), dValue );
							}
							break;
						case 244:
							{
								strResult = Con_strInvalid;
								FindItemOfReq(&reqTXFunction, strReq,1);
								FindItemOfReq(&reqTXFunction, strReq,20);
								TxStock *pStock = new TxStock;
								dValue = pStock->GetForcastNetProfit(reqTXFunction.nSecurityID,reqTXFunction.nFYear);
								if ( dValue != Con_doubleInvalid )
									strResult.Format( _T("%.2f"), dValue );
							}
							break;
						case 245:
							{
								strResult = Con_strInvalid;
								FindItemOfReq(&reqTXFunction, strReq,1);
								TxStock *pStock = new TxStock;
								nDate = pStock->GetForcastIssueDate(reqTXFunction.nSecurityID);
								if(nDate > 0)
								{
									strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
								}
							}
							break;
						case 199:
							{
								strResult = Con_strInvalid;
								FindItemOfReq(&reqTXFunction, strReq,20);
								nInstitution = (INT)pBusiness->m_pSecurity->GetInstitutionId();
								pIncome = NULL;	
								if ( bIncomeInstitution )
								{
									nAccountingID = (INT64)reqTXFunction.nFYear * 100000 +  93 ;
									pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_Income,nAccountingID,&pIncome,false);
								}
								else
								{
									nAccountingID = nInstitution * 10 + 3;
									//�����������ļ�	
									bool bLoaded = true;
									if(pIncomeDataFile!=NULL)
										bLoaded = pIncomeDataFile->Load((INT64)reqTXFunction.nFYear * 10000 + 9,30071,true);
									if ( bLoaded )
										pIncome = pIncomeDataFile->GetDataByObj(nAccountingID,false);
								}
								if ( pIncome == NULL)
									break;
								dValue = pBusiness->m_pSecurity->GetTotalInstitutionShare();
								if ( dValue > 0 && pIncome->dIncome[18] != Con_doubleInvalid )
									strResult.Format(_T("%.2f"),pIncome->dIncome[18]/dValue);
							}
							break;
						case 200:
						case 201:
						case 202:
						case 207:
							{
								strResult = Con_strInvalid;
								dValue = 0.0;
								nDate = CTime::GetCurrentTime().GetYear() * 10000 + CTime::GetCurrentTime().GetMonth() * 100 + CTime::GetCurrentTime().GetDay();
								FindItemOfReq( &reqTXFunction,strReq,16);
								if ( !pBusiness->m_pSecurity->IsTradeDate( reqTXFunction.nDate) && nDate != reqTXFunction.nDate )
									break;
								if ( nDate == reqTXFunction.nDate )
								{
									switch( reqTXFunction.iFuncID )
									{
									case 200:
										dValue = pBusiness->m_pSecurity->GetBuyAmount();
										break;
									case 201:
										dValue = pBusiness->m_pSecurity->GetSaleAmount();
										break;
									case 202:
										dValue = pBusiness->m_pSecurity->GetBuyAmount() - pBusiness->m_pSecurity->GetSaleAmount();
										break;
									case 207:
										dValue1 = pBusiness->m_pSecurity->GetBuyAmount();
										dValue2 = pBusiness->m_pSecurity->GetSaleAmount();
										if ( dValue2 != 0 )
											dValue = dValue1/dValue2;
										break;
									}
								}
								else
								{
									pAmountFlow = NULL;
									if (bFlowOfCapital)
									{
										nAccountingID = reqTXFunction.nDate;
										pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_AmountFlow,nAccountingID,&pAmountFlow,false);
									}
									else
									{
										nAccountingID = reqTXFunction.nSecurityID;
										bool bLoaded = true;
										if( pAmountFlowDataFile != NULL )
											bLoaded = pAmountFlowDataFile->Load( reqTXFunction.nDate,30201,true);
										if ( bLoaded )
											pAmountFlow = pAmountFlowDataFile->GetDataByObj( reqTXFunction.nSecurityID,false);							
									}
									if ( pAmountFlow == NULL )
										break;
									switch( reqTXFunction.iFuncID )
									{
									case 200:
										dValue = pAmountFlow->f_inflow;
										break;
									case 201:
										dValue = pAmountFlow->f_outFlow;
										break;
									case 202:
										dValue = pAmountFlow->f_inflow - pAmountFlow->f_outFlow;
										break;
									case 207:
										dValue1 = pAmountFlow->f_inflow;
										dValue2 = pAmountFlow->f_outFlow;
										if ( dValue2 != 0 )
											dValue = dValue1 / dValue2;
										break;
									}
								}
								if ( dValue != Con_doubleInvalid )
									strResult.Format( _T("%.2f"),dValue );
							}
							break;
						case 203:
						case 204:
						case 205:
						case 208:
							{
								strResult = Con_strInvalid;
								FindItemOfReq( &reqTXFunction, strReq, 5 );
								pAmountFlow = NULL;
								int m_iStartDate = reqTXFunction.nStartDate;
								int m_iEndDate = reqTXFunction.nEndDate;
								if ( m_iStartDate > m_iEndDate || pBusiness == NULL || pBusiness->m_pSecurity == NULL )
									break;							
								int m_iCurDate = CTime::GetCurrentTime().GetYear() * 10000 + CTime::GetCurrentTime().GetMonth() * 100 + CTime::GetCurrentTime().GetDay();
																
								if(m_iEndDate > m_iCurDate)
									m_iEndDate = m_iCurDate;

								if ( m_iStartDate == m_iEndDate )
								{									
									if ( !pBusiness->m_pSecurity->IsTodayTradeDate() )
										break;
									if ( m_iCurDate == m_iStartDate )
									{
										if( reqTXFunction.iFuncID  == 203)
											dValue = pBusiness->m_pSecurity->GetBuyAmount();
										else if( reqTXFunction.iFuncID  == 204)
											dValue = pBusiness->m_pSecurity->GetSaleAmount();
										else if( reqTXFunction.iFuncID  == 205)
											dValue = pBusiness->m_pSecurity->GetBuyAmount() - pBusiness->m_pSecurity->GetSaleAmount();
										else if( reqTXFunction.iFuncID  == 208)
										{
											dValue1 = pBusiness->m_pSecurity->GetBuyAmount();
											dValue2 = pBusiness->m_pSecurity->GetSaleAmount();
											if ( dValue2 != 0 )
												dValue = dValue1/dValue2;											
										}
									}
									else
									{
										pAmountFlow = NULL;
										if (bFlowOfCapital)
										{
											nAccountingID = m_iStartDate;
											pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_AmountFlow,nAccountingID,&pAmountFlow,false);
										}
										else
										{
											nAccountingID = reqTXFunction.nSecurityID;
											bool bLoaded = true;
											if( pAmountFlowDataFile != NULL )
												bLoaded = pAmountFlowDataFile->Load( m_iStartDate,30201,true);
											if ( bLoaded )
												pAmountFlow = pAmountFlowDataFile->GetDataByObj( reqTXFunction.nSecurityID,false);							
										}
										if ( pAmountFlow == NULL )
											break;
										if( reqTXFunction.iFuncID  == 203)
											dValue = pAmountFlow->f_inflow;
										else if( reqTXFunction.iFuncID  == 204)
											dValue = pAmountFlow->f_outFlow;
										else if( reqTXFunction.iFuncID  == 205)
											dValue = pAmountFlow->f_inflow - pAmountFlow->f_outFlow;
										else if( reqTXFunction.iFuncID  == 208)
										{
											dValue1 = pAmountFlow->f_inflow;
											dValue2 = pAmountFlow->f_outFlow;
											if ( dValue2 != 0 )
												dValue = dValue1 / dValue2;											
										}
									}								
								}
								else
								{
									pBusiness->m_pSecurity->LoadHisTrade(true);
									pBusiness->m_pSecurity->LoadTradeDate();
									HisTradeData* pHisTradeData = NULL;									
									pHisTradeData = pBusiness->m_pSecurity->GetTradeDataByNatureDate(reqTXFunction.nStartDate);
									if(pHisTradeData != NULL)
										m_iStartDate = pHisTradeData->Date;

									pHisTradeData = NULL;								
									pHisTradeData = pBusiness->m_pSecurity->GetTradeDataByNatureDate(reqTXFunction.nEndDate);
									if(pHisTradeData != NULL)
										m_iEndDate = pHisTradeData->Date;

									// �������������ʽ�����ʱû�п�������������Ч�ԵĴ��󡣡���2011��7��6�� ����
									if(reqTXFunction.nStartDate > m_iEndDate || reqTXFunction.nEndDate < m_iStartDate)
										break;

									pAmountFlow = NULL;
									bool bLoaded = false;
									if( pAmountFlowDataFile == NULL)
										break;								
									pAmountFlowDataFile->Reset();
									bLoaded = pAmountFlowDataFile->Load( m_iStartDate,30201,true);								
									if ( bLoaded )
										pAmountFlow = pAmountFlowDataFile->GetDataByObj( reqTXFunction.nSecurityID,false);
									if ( pAmountFlow == NULL)
										break;
									Tx::Data::AmountFlow amountTemp;
									memcpy( &amountTemp, pAmountFlow,sizeof(Tx::Data::AmountFlow));
									Tx::Data::AmountFlow*	pAmountFlow1 = NULL;
									pAmountFlowDataFile->Reset();
									bLoaded = false;
									int nHqEnd = pBusiness->m_pSecurity->GetTradeDateLatest();
									m_iEndDate = m_iEndDate > nHqEnd? nHqEnd:m_iEndDate;								
									bLoaded = pAmountFlowDataFile->Load(m_iEndDate,30201,true);
									if ( bLoaded )
										pAmountFlow1 = pAmountFlowDataFile->GetDataByObj( reqTXFunction.nSecurityID,false);
									if ( pAmountFlow1 == NULL )
										break;
									if( reqTXFunction.iFuncID  == 203)								
										dValue = pAmountFlow1->f_Suminflow - amountTemp.f_Suminflow +  amountTemp.f_inflow;
									else if( reqTXFunction.iFuncID  == 204)
										dValue = pAmountFlow1->f_SumoutFlow - amountTemp.f_SumoutFlow + amountTemp.f_outFlow;
									else if( reqTXFunction.iFuncID  == 205)
										dValue = pAmountFlow1->f_Suminflow - amountTemp.f_Suminflow  + amountTemp.f_inflow - pAmountFlow1->f_SumoutFlow + amountTemp.f_SumoutFlow - amountTemp.f_outFlow; 
									else if( reqTXFunction.iFuncID  == 208)
									{
										dValue1 = pAmountFlow1->f_Suminflow - amountTemp.f_Suminflow  +  amountTemp.f_inflow;
										dValue2 = pAmountFlow1->f_SumoutFlow - amountTemp.f_SumoutFlow + amountTemp.f_outFlow;
									}
									//���뵱����ʽ�����
									if(reqTXFunction.nEndDate >= m_iCurDate && m_iEndDate != m_iCurDate)
									{
										if ( pBusiness->m_pSecurity->IsTodayTradeDate() )
										{
											if( reqTXFunction.iFuncID  == 203)
												dValue += pBusiness->m_pSecurity->GetBuyAmount();
											else if( reqTXFunction.iFuncID  == 204)
												dValue += pBusiness->m_pSecurity->GetSaleAmount();
											else if( reqTXFunction.iFuncID  == 205)
												dValue += pBusiness->m_pSecurity->GetBuyAmount() - pBusiness->m_pSecurity->GetSaleAmount();
											else if( reqTXFunction.iFuncID  == 208)
											{
												dValue1 += pBusiness->m_pSecurity->GetBuyAmount();
												dValue2 += pBusiness->m_pSecurity->GetSaleAmount();
																						
											}
										}										
									}
									if ( reqTXFunction.iFuncID  == 208 && dValue2 != 0 )
										dValue = dValue1/dValue2;									
								}
								if ( dValue != Con_doubleInvalid )
									strResult.Format(_T("%.2f"),dValue);								
							}
							break;
							//�ʲ���ծ��ҵͳ��
						case 209:
							{

							}
							break;
							//���������ҵͳ��
						case 210:
							{

							}
							break;
							//�Ƚ�������ҵͳ��
						case 211:
							{

							}
							break;
						case 212:	//���շ���--��׼��
						case 213:	//���շ���--Alpha
						case 214:	//���շ���--Beta
						case 215:	//���շ���--Sharpeϵ��
						case 216:	//���շ���--Treynorϵ��
						case 217:	//���շ���--�������
						case 287:   //�����ֵ
							{
								strResult = Con_strInvalid;
								FindItemOfReq( &reqTXFunction, strReq, 26 );
								Tx::Core::Table_Indicator table;
								std::set<int> iset;
								iset.clear();
								//ֻ��дһ������
								int id;
								id = reqTXFunction.nSecurityID;
								iset.insert( id );
								//************************
								int iEndDate = 20070730;						//��ֹ����
								long lSecurityId = 0;					//��׼����ʵ��ID
								int iTradeDaysCount=20;				//��������
								int iStartDate=0;
								bool bLogRatioFlag=false;			//������ʵķ�ʽ,true=ָ������,false=�򵥱���
								bool bClosePrice=true;				//ʹ�����̼�
								int  iStdType=0;					//��׼������1=��ƫ��������ƫ
								int  iOffset=0;						//ƫ��
								bool bAhead=true;					//��Ȩ��־,true=ǰ��Ȩ,false=��Ȩ
								bool bUserRate=false;				//�û��Զ���������,����ȡһ�궨�ڻ�׼����
								bool bDayRate=true;					//������
								int iDuration = 1;					//��������0=�գ�1=�ܣ�2=�£�3=����5=��
								double dUserRate = 0.0;
								//���������ļ���߶�Ӧ���Ǵ�1��ʼ��,���õ�ʱ���1
								int iDateType = 0;					//0��ֹ����Ϊ���½����գ�1ָ����ֹ���ڣ�2ָ����ֹ����
								long lRefoSecurityId = 4000208;
								iEndDate  = reqTXFunction.nEndDate;
								iStartDate = reqTXFunction.nStartDate;
								iDuration = reqTXFunction.iSYPL;
								if(iDuration == 4)
									iDuration = 5;
								else if(iDuration == 5)
									iDuration = 4;
								Tx::Business::TxBusiness	Business;
								Business.GetSecurityNow( lRefoSecurityId );
								Business.m_pSecurity->LoadTradeDate();
								Security* psec = (Security*)GetSecurity(id);
								if(psec == NULL)
									return FALSE;
								double std,alpha,beta,sharp,treynor,trackerror;
								std=alpha=beta=sharp=treynor=trackerror=0.0;
								double dMean = 0.0;
								if(psec->IsFund())
								{
									Tx::Business::TxFund txfund;
									txfund.BlockRiskIndicatorAdvFund(
										10228,
										table,
										iset,
										iEndDate,
										lRefoSecurityId,
										iStartDate,
										iTradeDaysCount, 
										iDuration,						//��������0=�գ�1=�ܣ�2=�£�3=����4=��
										bLogRatioFlag,						//������ʵķ�ʽ,true=ָ������,false=�򵥱���
										bClosePrice,						//ʹ�����̼�
										0,							//��׼������1=��ƫ��������ƫ
										0,									//ƫ��
										true,								//��Ȩ��־,true=ǰ��Ȩ,false=��Ȩ
										1,							//�û��Զ���������,����ȡһ�궨�ڻ�׼����
										dUserRate,
										iDuration==0?true:false,							//������ 2008-11-24 by zhaohj
										true,
										true
										);
									if (table.GetRowCount() == 1)
									{
										int iCol = 2;
										table.GetCell(iCol,0,std);
										iCol++;
										table.GetCell(iCol,0,alpha);
										iCol++;
										table.GetCell(iCol,0,beta);
										iCol++;
										table.GetCell(iCol,0,sharp);
										iCol++;
										table.GetCell(iCol,0,treynor);
										iCol++;
										table.GetCell(iCol,0,trackerror);
										iCol++;
										table.GetCell(iCol,0,dMean);
										iCol++;
									}
								}
								else
								{
									Tx::Business::TxStock txstock;
									txstock.BlockRiskIndicatorAdv(
										10228,
										table,
										iset,
										iEndDate,
										lRefoSecurityId,
										iStartDate,
										iTradeDaysCount,
										iDuration,						//��������0=�գ�1=�ܣ�2=�£�3=����4=��
										bLogRatioFlag,						//������ʵķ�ʽ,true=ָ������,false=�򵥱���
										bClosePrice,						//ʹ�����̼�
										0,							//��׼������1=��ƫ��������ƫ
										0,									//ƫ��
										true,								//��Ȩ��־,true=ǰ��Ȩ,false=��Ȩ
										1,							//�û��Զ���������,����ȡһ�궨�ڻ�׼����
										dUserRate,
										iDuration==0?true:false,								//������ 2008-11-24 by zhaohj
										true,
										true
										);
									if (table.GetRowCount() == 1)
									{
										int iCol = 3;
										table.GetCell(iCol,0,std);
										iCol++;
										table.GetCell(iCol,0,alpha);
										iCol++;
										table.GetCell(iCol,0,beta);
										iCol++;
										table.GetCell(iCol,0,sharp);
										iCol++;
										table.GetCell(iCol,0,treynor);
										iCol++;
										table.GetCell(iCol,0,trackerror);
										iCol++;
										table.GetCell(iCol,0,dMean);
										iCol++;
									}
								}								
								switch( reqTXFunction.iFuncID )
								{
								case 212:
									dValue = std;
									break;
								case 213:
									dValue = alpha;
									break;
								case 214:
									dValue = beta;
									break;
								case 215:
									dValue = sharp;
									break;
								case 216:
									dValue = treynor;
									break;
								case 217:
									dValue = trackerror;
									break;
								case 287:
									dValue = dMean;
									break;
								default:
									dValue = 0.0;
									break;
								}
								if ( dValue != Con_doubleInvalid )
									strResult.Format(_T("%.6f"),dValue);
							}
							break;
						case 220:
						case 221:
						case 222:
						case 223:
						case 224:
						case 225:
						case 226:
						case 227:
						case 228:													
							vdReq.data_type = dtype_double;
							FindItemOfReq(&reqTXFunction, strReq, 13);	//������ȡ����񼾶ȡ��ϲ�/ĸ��˾������ǰ/��
							nInstitution = (INT)pBusiness->m_pSecurity->GetInstitutionId();
							iSubItemID = (int)reqTXFunction.iSubItemID-1;
							dValue = Tx::Core::Con_doubleInvalid;
							switch( reqTXFunction.iFuncID )
							{								
							case 220:
								pBalance_YH = NULL;
								if ( bBalanceYH )
								{
									nAccountingID = (INT64)reqTXFunction.nFYear * 100000 +  (INT64)reqTXFunction.nFQuarter* 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);
									//nAccountingID = (INT64)reqTXFunction.nFYear * 10000 +  (INT64)reqTXFunction.nFQuarter;
									if(m_pBalanceYHDataFile!=NULL)
									{
										if(m_pBalanceYHDataFile->Load(nInstitution,30270,true)==true)
										{
											pBalance_YH = m_pBalanceYHDataFile->GetDataByObj(nAccountingID,false);
											if ( pBalance_YH!=NULL )
												dValue = pBalance_YH->dBalance[iSubItemID];
										}
									}
								}
								else
								{
									nAccountingID = nInstitution * 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);
									if(m_pBalanceYHDataFile!=NULL)
									{
										int m_Institution = reqTXFunction.nFYear * 10000 + reqTXFunction.nFQuarter;
										if(m_pCashFlowBXDataFile->Load(m_Institution,30269,true)==true)											
										{
											pBalance_YH = m_pBalanceYHDataFile->GetDataByObj(nAccountingID,false);
											if ( pBalance_YH!=NULL )
												dValue = pBalance_YH->dBalance[iSubItemID];
										}												
									}
								}
								break;
							case 221:									
								pBalance_BX = NULL;
								if ( bBalanceBX )
								{
									nAccountingID = (INT64)reqTXFunction.nFYear * 100000 +  (INT64)reqTXFunction.nFQuarter* 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);
									if(m_pBalanceBXDataFile!=NULL)
									{
										if(m_pBalanceBXDataFile->Load(nInstitution,30272,true)==true)
										{
											pBalance_BX = m_pBalanceBXDataFile->GetDataByObj(nAccountingID,false);
											if ( pBalance_BX!=NULL )
												dValue = pBalance_BX->dBalance[iSubItemID];
										}
									}
								}
								else
								{
									nAccountingID = nInstitution * 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);											
									if(m_pBalanceBXDataFile!=NULL)
									{
										int m_Institution = reqTXFunction.nFYear * 10000 + reqTXFunction.nFQuarter;
										if(m_pCashFlowBXDataFile->Load(m_Institution,30271,true)==true)											
										{
											pBalance_BX= m_pBalanceBXDataFile->GetDataByObj(nAccountingID,false);
											if ( pBalance_BX!=NULL )
												dValue = pBalance_BX->dBalance[iSubItemID];
										}
									}
								}									
								break;
							case 222:
								pBalance_ZQ = NULL;
								if ( bBalanceZQ )
								{											
									if(m_pBalanceZQDataFile!=NULL)
									{
										nAccountingID = (INT64)reqTXFunction.nFYear * 100000 +  (INT64)reqTXFunction.nFQuarter* 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);
										if(m_pBalanceZQDataFile->Load(nInstitution,30274,true)==true)
										{
											pBalance_ZQ = m_pBalanceZQDataFile->GetDataByObj(nAccountingID,false);
											if ( pBalance_ZQ!=NULL )
												dValue = pBalance_ZQ->dBalance[iSubItemID];
										}												
									}
								}
								else
								{
									nAccountingID = nInstitution * 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);										
									if(m_pBalanceZQDataFile!=NULL)
									{
										int m_Institution = reqTXFunction.nFYear * 10000 + reqTXFunction.nFQuarter;
										if(m_pCashFlowBXDataFile->Load(m_Institution,30273,true)==true)											
										{
											pBalance_ZQ = m_pBalanceZQDataFile->GetDataByObj(nAccountingID,false);
											if ( pBalance_ZQ!=NULL )
												dValue = pBalance_ZQ->dBalance[iSubItemID];
										}
									}
								}
								break;
							case 223:
								pProfit_YH = NULL;
								if ( bProfitYH )
								{											
									if(m_pProfitYHDataFile!=NULL)
									{
										nAccountingID = (INT64)reqTXFunction.nFYear * 100000 +  (INT64)reqTXFunction.nFQuarter* 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);
										if(m_pProfitYHDataFile->Load(nInstitution,30276,true)==true)
										{
											pProfit_YH= m_pProfitYHDataFile->GetDataByObj(nAccountingID,false);													
											if ( pProfit_YH!=NULL )
												dValue = pProfit_YH->dProfit[iSubItemID];
											else
											{
												CString sLog;
												sLog.Format(_T("��������[ %s ]��������û��ȡ����Ӧ����\r\n"),pBusiness->m_pSecurity->GetName());
												Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
											}
										}
										else
										{
											CString sLog;
											sLog.Format(_T("��������[ %s ]���������ļ�����ʧ��\r\n"),pBusiness->m_pSecurity->GetName());
											Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
										}
									}
								}
								else
								{
									nAccountingID = nInstitution * 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);
									if(m_pProfitYHDataFile!=NULL)
									{
										int m_Institution = reqTXFunction.nFYear * 10000 + reqTXFunction.nFQuarter;
										if(m_pCashFlowBXDataFile->Load(m_Institution,30275,true)==true)											
										{
											pProfit_YH = m_pProfitYHDataFile->GetDataByObj(nAccountingID,false);
											if ( pProfit_YH!=NULL )
												dValue = pProfit_YH->dProfit[iSubItemID];
											else
											{
												CString sLog;
												sLog.Format(_T("��������[ %s ]��������û��ȡ����Ӧ����\r\n"),pBusiness->m_pSecurity->GetName());
												Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
											}
										}
									}
									else
									{
										CString sLog;
										sLog.Format(_T("��������[ %s ]���������ļ�����ʧ��\r\n"),pBusiness->m_pSecurity->GetName());
										Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
									}
								}
								break;
							case 224:
								pProfit_BX = NULL;
								if ( bProfitBX )
								{										
									if(m_pProfitBXDataFile!=NULL)
									{
										nAccountingID = (INT64)reqTXFunction.nFYear * 100000 +  (INT64)reqTXFunction.nFQuarter* 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);
										if(m_pProfitBXDataFile->Load(nInstitution,30278,true)==true)
										{
											pProfit_BX = m_pProfitBXDataFile->GetDataByObj(nAccountingID,false);
											if ( pProfit_BX!=NULL )
												dValue = pProfit_BX->dProfit[iSubItemID];
										}											
									}
								}
								else
								{
									nAccountingID = nInstitution * 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);										
									if(m_pProfitBXDataFile!=NULL)
									{
										int m_Institution = reqTXFunction.nFYear * 10000 + reqTXFunction.nFQuarter;
										if(m_pCashFlowBXDataFile->Load(m_Institution,30277,true)==true)											
										{
											pProfit_BX = m_pProfitBXDataFile->GetDataByObj(nAccountingID,false);
											if ( pProfit_BX!=NULL )
												dValue = pProfit_BX->dProfit[iSubItemID];
										}

									}
								}
								break;
							case 225:
								{
									pProfit_ZQ = NULL;
									BOOL bType1 = TRUE;
									BOOL bType2 = TRUE;
									BOOL bType3 = TRUE;
									if (reqTXFunction.iReportType == 0 && reqTXFunction.iAccountingPolicyType == 0) //ĸ��˾������ǰ
									{
										reqTXFunction.iAccountingPolicyType = 1;
										bType1 = FALSE;
									}
									else if (reqTXFunction.iReportType == 0 && reqTXFunction.iAccountingPolicyType == 1 && bType1) //ĸ��˾��������
									{
										reqTXFunction.iReportType = 1;
										reqTXFunction.iAccountingPolicyType = 0;
										bType2 = FALSE;
									}
									else if (reqTXFunction.iReportType == 1 && reqTXFunction.iAccountingPolicyType == 0 && bType2) //�ϲ�������ǰ
									{
										reqTXFunction.iAccountingPolicyType = 1;
										bType3 = FALSE;
									}
									else if (reqTXFunction.iReportType == 1 && reqTXFunction.iAccountingPolicyType == 1 && bType3)
									{
										reqTXFunction.iReportType = 2;
										reqTXFunction.iAccountingPolicyType = 0;
									}
									if ( bProfitZQ )
									{										
										if(m_pProfitZQDataFile!=NULL)
										{
											nAccountingID = (INT64)reqTXFunction.nFYear * 100000 +  (INT64)reqTXFunction.nFQuarter* 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);
											if(m_pProfitZQDataFile->Load(nInstitution,30280,true)==true)
											{
												pProfit_ZQ = m_pProfitZQDataFile->GetDataByObj(nAccountingID,false);
												if ( pProfit_ZQ!=NULL )
													dValue = pProfit_ZQ->dProfit[iSubItemID];
											}
										}
									}
									else
									{
										nAccountingID = nInstitution * 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);										
										if(m_pProfitZQDataFile!=NULL)
										{
											int m_Institution = reqTXFunction.nFYear * 10000 + reqTXFunction.nFQuarter;
											if(m_pCashFlowBXDataFile->Load(m_Institution,30279,true)==true)
											{
												pProfit_ZQ = m_pProfitZQDataFile->GetDataByObj(nAccountingID,false);
												if ( pProfit_ZQ!=NULL )
													dValue = pProfit_ZQ->dProfit[iSubItemID];
											}
										}
									}
								}
								break;
							case 226:
								pCashFlow_YH = NULL;
								if ( bCashFlowYH )
								{
									if(m_pCashFlowYHDataFile!=NULL)
									{
										nAccountingID = (INT64)reqTXFunction.nFYear * 100000 +  (INT64)reqTXFunction.nFQuarter* 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);
										if(m_pCashFlowYHDataFile->Load(nInstitution,30264,true)==true)
										{
											pCashFlow_YH = m_pCashFlowYHDataFile->GetDataByObj(nAccountingID,false);
											if ( pCashFlow_YH!=NULL )
												dValue = pCashFlow_YH->dCashFlow[iSubItemID];
										}												
									}
								}
								else
								{
									nAccountingID = nInstitution * 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);
									if(m_pCashFlowYHDataFile!=NULL)
									{
										int m_Institution = reqTXFunction.nFYear * 10000 + reqTXFunction.nFQuarter;
										if(m_pCashFlowBXDataFile->Load(m_Institution,30263,true)==true)											
										{
											pCashFlow_YH= m_pCashFlowYHDataFile->GetDataByObj(nAccountingID,false);
											if ( pCashFlow_YH!=NULL )
												dValue = pCashFlow_YH->dCashFlow[iSubItemID];
										}
									}
								}
								break;
							case 227:
								pCashFlow_BX = NULL;
								if ( bCashFlowBX )
								{									
									if(m_pCashFlowBXDataFile!=NULL)
									{
										nAccountingID = (INT64)reqTXFunction.nFYear * 100000 +  (INT64)reqTXFunction.nFQuarter* 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);
										if(m_pCashFlowBXDataFile->Load(nInstitution,30268,true)==true)
										{
											pCashFlow_BX = m_pCashFlowBXDataFile->GetDataByObj(nAccountingID,false);
											if ( pCashFlow_BX!=NULL )
												dValue = pCashFlow_BX->dCashFlow[iSubItemID];
										}
										
									}
								}
								else
								{
									nAccountingID = nInstitution * 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);
									
									if(m_pCashFlowBXDataFile!=NULL)
									{
										int m_Institution = reqTXFunction.nFYear * 10000 + reqTXFunction.nFQuarter;
										if(m_pCashFlowBXDataFile->Load(m_Institution,30267,true)==true)
										{
											pCashFlow_BX = m_pCashFlowBXDataFile->GetDataByObj(nAccountingID,false);
											if ( pCashFlow_BX!=NULL )
												dValue = pCashFlow_BX->dCashFlow[iSubItemID];
										}
										
									}
								}
								break;
							case 228:
								pCashFlow_ZQ = NULL;
								if ( bCashFlowZQ )
								{									
									if(m_pCashFlowZQDataFile!=NULL)
									{
										nAccountingID = (INT64)reqTXFunction.nFYear * 100000 +  (INT64)reqTXFunction.nFQuarter* 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);
										if(m_pCashFlowZQDataFile->Load(nInstitution,30266,true)==true)
										{
											pCashFlow_ZQ = m_pCashFlowZQDataFile->GetDataByObj(nAccountingID,false);
											if ( pCashFlow_ZQ!=NULL )
												dValue = pCashFlow_ZQ->dCashFlow[iSubItemID];
										}											
									}
								}
								else
								{
									nAccountingID = nInstitution * 10 + (((INT64)reqTXFunction.iReportType*2)|(INT64)reqTXFunction.iAccountingPolicyType);										
									if(m_pBalanceYHDataFile!=NULL)
									{
										int m_Institution = reqTXFunction.nFYear * 10000 + reqTXFunction.nFQuarter;
										if(m_pBalanceYHDataFile->Load(m_Institution,30265,true)==true)
										{
											pCashFlow_ZQ = m_pCashFlowZQDataFile->GetDataByObj(nAccountingID,false);
											if ( pCashFlow_ZQ!=NULL )
												dValue = pCashFlow_ZQ->dCashFlow[iSubItemID];
										}											
									}
								}
							break;
							default:
								break;
							}
							if(dValue != Tx::Core::Con_doubleInvalid && dValue != Tx::Core::Con_floatInvalid)
							{
								strResult.Format(_T("%.2f"), dValue);
							}
							break;
							case 229:
								{
									vdReq.data_type = dtype_double;
									FindItemOfReq(&reqTXFunction, strReq, 27);
									CString m_strTempDate = _T("-");
									m_strTempDate.Format(_T("%d"),reqTXFunction.nDate);
									strResult = _T("-");
									strResult = Text2Date(m_strTempDate);
								}
								break;
							case 230:
								{
									vdReq.data_type = dtype_double;
									FindItemOfReq(&reqTXFunction, strReq, 27);
									//CString m_strTempDate = _T("-");
									//m_strTempDate.Format(_T("%d"),reqTXFunction.nDate);
									//strResult = _T("-");
									//strResult = Date2Text(m_strTempDate);
									strResult.Format(_T("%d"),reqTXFunction.nDate);
								}								
								break;
							case 231:
								{
									if ( pBusiness->m_pSecurity->IsFund())
										strResult = pBusiness->m_pSecurity->GetCode();
									else
										strResult = Con_strInvalid;
								}
								break;
							case 232:
								{
									FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
									if ( bHQSecurity )
										dValue = GetKLineEx(pBusiness, reqTXFunction.nDate, 7,reqTXFunction.nSecurityID);
									else
										dValue = GetKLine(pBusiness, reqTXFunction.nDate, 7,reqTXFunction.nSecurityID);
									if ( fabs(dValue - Con_doubleInvalid ) < 0.00001 )
										break;
									if(pBusiness->m_pSecurity->IsFundTradeInMarket()||(pBusiness->m_pSecurity->IsShanghai()&&pBusiness->m_pSecurity->IsStockB()))
									{
										strResult.Format(_T("%.3f"), dValue);
									}else
									{
										strResult.Format(_T("%.2f"), dValue);
									}
									break;
								}
								break;
							case 233:
								{
									FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
									if ( bHQSecurity )
										dValue = GetKLineEx(pBusiness, reqTXFunction.nDate, 8,reqTXFunction.nSecurityID);
									else
										dValue = GetKLine(pBusiness, reqTXFunction.nDate, 8,reqTXFunction.nSecurityID);
									if ( fabs(dValue - Con_doubleInvalid ) < 0.00001 )
										break;
									if(pBusiness->m_pSecurity->IsFundTradeInMarket()||(pBusiness->m_pSecurity->IsShanghai()&&pBusiness->m_pSecurity->IsStockB()))
									{
										strResult.Format(_T("%.3f"), dValue);
									}else
									{
										strResult.Format(_T("%.2f"), dValue);
									}
									break;
								}
								break;
							case 234:
								{
									CString str = pBusiness->m_pSecurity->GetCode( true );
									int nTmp = str.Find(_T("."));
									if ( nTmp == -1 )
										break;
									str = str.Mid( nTmp+1 );
									if ( str.Compare(_T("SZ")) == 0 )
										strResult = _T("����");
									else
										if ( str.Compare(_T("SH")) == 0 )
											strResult = _T("�Ϻ�");
										else
											if ( str.Compare(_T("TX")) == 0 )
												strResult = _T("����");
											else
												if ( str.Compare(_T("HK")) == 0 )
													strResult = _T("���");
												else    /* 20100708 wanglm ָ���С��б꣬�����¸����������г� */ 
													if ( str.Compare(_T("SW")) == 0 )
													   strResult = _T("����");
													else
														if ( str.Compare(_T("ZX")) == 0 )
															strResult = _T("����");
								}
								break;
							case 237:
								{
									FindItemOfReq(&reqTXFunction, strReq, 3);	// ndate
									iSubItemID = (int)reqTXFunction.iSubItemID-1;
									vdReq.data_type = dtype_val_string;
									CString strPath = pBusiness->DownloadXmlFile(pBusiness->m_pSecurity->GetInstitutionId(),10269);
									OleInitialize(NULL);
									CXmlDocumentWrapper doc;
									if ( !doc.Load((LPCTSTR)strPath ))
										break;
									CString lpFind;
									lpFind.Format(_T("//tr[T6='%d']"),reqTXFunction.nDate);
									CXmlNodeWrapper root(doc.AsNode());
									//CString lpFind = 
									//	_T("//tr[*[name() = name(//ColumnName//*[@fieldName='f_end_date'])]='20071231']");
									//CXmlNodeWrapper configNode(root.FindNode(lpFind));
									CXmlNodeWrapper destNode(root.FindNode( lpFind ));
									strResult = _T("");
									if ( destNode.IsValid())
									{
										CString strText = destNode.GetText();

										CString sNodeItem = NULL;
										switch(iSubItemID)
										{
										case 0:
											sNodeItem = _T("T9");
											break;
										case 1:
											sNodeItem = _T("T10");
											break;
										case 2:
											sNodeItem = _T("T11");
											break;
										case 3:
											sNodeItem = _T("T12");
											break;
										case 4:
											sNodeItem = _T("T3");
											break;
										case 5:
											sNodeItem = _T("T13");
											break;
										case 6:
											sNodeItem = _T("T14");
											break;
										}

										CXmlNodeWrapper ValueNode(destNode.FindNode(sNodeItem));
										if( ValueNode.IsValid() )
										{
											strResult = ValueNode.GetText();
											if(iSubItemID == 3)
											{
												//ÿʮ�����ֽ��������ÿ�����ֽ���
												float fTmp = atof(strResult.GetBuffer());
												fTmp /= 10;
												strResult.ReleaseBuffer();
												strResult.Format(_T("%f"),fTmp);
											}
										}
									}
									OleUninitialize();
								}
								break;
							case 238:		//������
								{
									vdReq.data_type = dtype_val_string;
									strResult = pBusiness->m_pSecurity->GetName();
								}								
								break;
							case 243:	//֤���淶������
								{
									vdReq.data_type = dtype_val_string;
									strResult = _T("-");
									int iSecId = pBusiness->m_pSecurity->GetId();
									std::unordered_map<int,CString>::iterator iterTmp;
									iterTmp = FundNameMap.find(iSecId);
									if(iterTmp != FundNameMap.end())
									{
										strResult = iterTmp->second;
									}
								}
								break;
							case 239:		//ծȯ����
								vdReq.data_type = dtype_val_string;
								{
									BondRating* pBondRating = NULL;
									DataFileNormal<blk_TxExFile_FileHead,BondRating>* pBondRatingDataFile = new DataFileNormal<blk_TxExFile_FileHead,BondRating>;
									if ( pBondRatingDataFile == NULL )
										break;
									pBondRatingDataFile->SetCheckLoadById(true);
									bool bLoaded = false;
									if(pBondRatingDataFile!=NULL)
										bLoaded = pBondRatingDataFile->Load(30316,30007,true);
									if ( bLoaded )
										pBondRating = pBondRatingDataFile->GetDataByObj(reqTXFunction.nSecurityID,false);
									if ( pBondRating != NULL)
										strResult = CString(pBondRating->f_bondrating);
									delete pBondRatingDataFile;
								}
								break;
							case 240:		//Ԥ����¶����
								{
									FindItemOfReq(&reqTXFunction, strReq, 13);	//������ȡ����񼾶ȡ��ϲ�/ĸ��˾������ǰ/��
									nInstitution = reqTXFunction.nFYear*10000+reqTXFunction.nFQuarter;
									PublishDate*	pDate = NULL;
									DataFileNormal<blk_TxExFile_FileHead,PublishDate>* pPublishDateDataFile = new DataFileNormal<blk_TxExFile_FileHead,PublishDate>;
									if ( pPublishDateDataFile == NULL )
										break;
									pPublishDateDataFile->SetCheckLoadById(true);
									bool bLoaded = false;
									if(pPublishDateDataFile!=NULL)
										bLoaded = pPublishDateDataFile->Load(nInstitution,30317,true);
									if ( bLoaded )
										pDate = pPublishDateDataFile->GetDataByObj(reqTXFunction.nSecurityID,false);
									if ( pDate != NULL)
									{
										if ( pDate->iExpectedDate > 0 )
											strResult.Format(_T("%d"),pDate->iExpectedDate);
									}
									delete pPublishDateDataFile;
								}
								break;
							case 241:		//ʵ����¶����
								{
									FindItemOfReq(&reqTXFunction, strReq, 13);	//������ȡ����񼾶ȡ��ϲ�/ĸ��˾������ǰ/��
									nInstitution = reqTXFunction.nFYear*10000+reqTXFunction.nFQuarter;
									PublishDate*	pDate = NULL;
									DataFileNormal<blk_TxExFile_FileHead,PublishDate>* pPublishDateDataFile = new DataFileNormal<blk_TxExFile_FileHead,PublishDate>;
									if ( pPublishDateDataFile == NULL )
										break;
									pPublishDateDataFile->SetCheckLoadById(true);
									bool bLoaded = false;
									if(pPublishDateDataFile!=NULL)
										bLoaded = pPublishDateDataFile->Load(nInstitution,30317,true);
									if ( bLoaded )
										pDate = pPublishDateDataFile->GetDataByObj(reqTXFunction.nSecurityID,false);
									if ( pDate != NULL)
									{
										if ( pDate->iDate > 0 )
											strResult.Format(_T("%d"),pDate->iDate);
									}
									delete pPublishDateDataFile;
								}
								break;
							case 242:	//ȡ�ùɱ���ϸ����
								{
									dValue = 1.0;
									vdReq.data_type = dtype_int4;
									FindItemOfReq(&reqTXFunction, strReq, 5);	//StartDate��EndDate
									pStockBonusData = NULL;
									int ix = 0;
									int m_iCount = pBusiness->m_pSecurity->GetStockBonusDataCount();
									if(reqTXFunction.nStartDate > reqTXFunction.nEndDate)
									{
										strResult = _T("-");
									}
									else
									{
										for(ix = 0; ix < m_iCount; ix++)
										{
											pStockBonusData = pBusiness->m_pSecurity->GetStockBonusDataByIndex((INT)ix);
											if(pStockBonusData == NULL)
												continue;
											if(reqTXFunction.nStartDate < pStockBonusData->register_date && 
												reqTXFunction.nEndDate >= pStockBonusData->register_date)
											{
												if(reqTXFunction.nFYear > 0 && reqTXFunction.nFYear != (pStockBonusData->year / 10000))
													continue;
												if(pStockBonusData->sgshu > 0.0 && pStockBonusData->sgshu != Con_doubleInvalid)
													dValue *= 1+ pStockBonusData->sgshu;											
											}
										}
										strResult.Format(_T("%02f"), dValue);
									}									
								}
								break;
							case 246:     //��̬PE    2012-3-23
								{
									strResult = Con_strInvalid;
									FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
									if ( bPePb )
										dValue = Get_PE_Static(reqTXFunction.nSecurityID,reqTXFunction.nDate);
									else
										dValue = Get_PE_Static(reqTXFunction.nSecurityID,reqTXFunction.nDate);
									if ( dValue != Con_doubleInvalid )
										strResult.Format("%.2f",dValue);
								}
								break;
							case 247:    //������ҵ��ֵռ����ֵ��   2012-3-23
								{
									strResult = Con_strInvalid;
									FindItemOfReq(&reqTXFunction,strReq,24);  //FuncID��security�������С�����Ƽ�
									nInstitution = (INT)pBusiness->m_pSecurity->GetSecurity1Id();
									iSubItemID = (int)reqTXFunction.iSubItemID-1;
									pFundInvesmentGroup = NULL;
									pFundTXInvDistribution = NULL;
									if (reqTXFunction.nFQuarter == 3)
									{
										reqTXFunction.nFQuarter = 2;
										nAccountingID = (INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter;
										reqTXFunction.nFQuarter = 3;
									}
									if (reqTXFunction.nFQuarter == 9)
									{
										reqTXFunction.nFQuarter = 6;
										nAccountingID = (INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter;
 										reqTXFunction.nFQuarter = 9;
									}
									pBusiness->m_pSecurity->GetDataByObj(Tx::Data::dt_FundInvesmentGroup,nAccountingID,&pFundInvesmentGroup,false);
									if (pFundInvesmentGroup == NULL)
									    break;
									dValue = pFundInvesmentGroup->dValue[9];   //��ȡ����ֵ
									dValue1 = dValue;
									dValue = Con_doubleInvalid;
								

									nAccountingID = (INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter;
									if ( bFundTXIndustryDistribute )
									{
										pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_FundBlockInvestmentTxIndustryDistribute,nAccountingID,&pFundTXInvDistribution,false);
									}
									else
									{
										nAccountingID = nInstitution;
										//�����������ļ�	
										bool bLoaded = false;
										if(pFundTXInvDistributionDataFile!=NULL)
											bLoaded = pFundTXInvDistributionDataFile->Load(nAccountingID,30161,true);
										if ( bLoaded )
											pFundTXInvDistribution = pFundTXInvDistributionDataFile->GetDataByObj(nAccountingID,false);
									}
									if ( pFundTXInvDistribution!=NULL )
										dValue = pFundTXInvDistribution->dFinancial[iSubItemID];//��ȡ��ֵ
									if (dValue1 != Con_doubleInvalid && dValue != Con_doubleInvalid && dValue >0.0)
									{
										dValue = dValue/dValue1;
                                        strResult.Format("%.4f",dValue);
									}
								}
								break;
							case 248:    //�ϲ���ҵ��ֵռ����ֵ��   2012-3-23
								{
									strResult = Con_strInvalid;
									FindItemOfReq(&reqTXFunction,strReq,24);  //FuncID��security�������С�����Ƽ�
									nInstitution = (INT)pBusiness->m_pSecurity->GetSecurity1Id();
									iSubItemID = (int)reqTXFunction.iSubItemID-1;
									pFundInvesmentGroup = NULL;
									nAccountingID = (INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter;
									pBusiness->m_pSecurity->GetDataByObj(Tx::Data::dt_FundInvesmentGroup,nAccountingID,&pFundInvesmentGroup,false);
									if (pFundInvesmentGroup != NULL)
									    dValue = pFundInvesmentGroup->dValue[9];   //��ȡ����ֵ
									dValue1 = dValue;
									dValue = Con_doubleInvalid;

									pFundComInvDistribute = NULL;
									if ( bFundCombineIndustryDistribute )
									{
										pBusiness->m_pSecurity->GetDataByObj( Tx::Data::dt_FundCombineInvestmentIndustryDistribute,nAccountingID,&pFundComInvDistribute,false);
									}
									else
									{
										nAccountingID = nInstitution;
										//�����������ļ�	
										bool bLoaded = false;
										if(pFundComInvDistributeDataFile!=NULL)
											bLoaded = pFundComInvDistributeDataFile->Load(nAccountingID,30155,true);
										if ( bLoaded )
											pFundComInvDistribute = pFundComInvDistributeDataFile->GetDataByObj(nAccountingID,false);
									}
									if ( pFundComInvDistribute!=NULL )
										dValue = pFundComInvDistribute->dFinancial[iSubItemID];    //��ȡ��ֵ
									if( dValue != Con_doubleInvalid && dValue1 != Con_doubleInvalid && dValue >0.0)
									{
										dValue = dValue/dValue1;
										strResult.Format("%.4f",dValue);
									}		
								}
								break;
							case 283://�ϲ���ҵ��ֵռ����ֵ��(��)
								{
									strResult = Con_strInvalid;
									FindItemOfReq(&reqTXFunction,strReq,24);  //FuncID��security�������С�����Ƽ�
									nInstitution = (INT)pBusiness->m_pSecurity->GetSecurity1Id();
									pFundInvesmentGroup = NULL;
									nAccountingID = (INT64)reqTXFunction.nFYear * 10000 + (INT64)reqTXFunction.nFQuarter;
									pBusiness->m_pSecurity->GetDataByObj(Tx::Data::dt_FundInvesmentGroup,nAccountingID,&pFundInvesmentGroup,false);
									if (pFundInvesmentGroup != NULL)
										dValue = pFundInvesmentGroup->dValue[9];   //��ȡ����ֵ
									int iIndustryID = reqTXFunction.iSubItemID;
									double dIndustry = GetFundCombineIndustryDistribute(nInstitution,(int)nAccountingID,iIndustryID);
									if (dValue != Con_doubleInvalid && dIndustry > 0.0)
									{
										dIndustry = dIndustry/dValue;
										strResult.Format("%.4f",dIndustry);
									}
								}
								break;
							case 249:     //ծȯ�׷��۸�
								{
									vdReq.data_type = dtype_double;
									if (!pBusiness->m_pSecurity->IsBond())
									{
										break;
									}
									pBondNewInfo = pBusiness->m_pSecurity->GetBondNewInfo();
									if (pBondNewInfo != NULL)
									{
										dValue = pBondNewInfo->issue_price;
										if (dValue != Con_doubleInvalid)
										{
											strResult.Format(_T("%.3f"),dValue);
										}
									}
								}
								break;
							case 250:     //ծȯ��׼ȯ������
								{
									if(!pBusiness->m_pSecurity->IsBond() && !pBusiness->m_pSecurity->IsFund())
									{
										break;
									}
									FindItemOfReq(&reqTXFunction, strReq,3);
									BondConvRate* pBondConRate = NULL;
									BondConversionRatePast* pBondConversionRatePast = NULL;
									if ((int)reqTXFunction.nDate == 0)
									{
										pBondConRate = pBusiness->m_pSecurity->GetBondConvRate();
										if (pBondConRate != NULL)
										{
											dValue = pBondConRate->f_rate;
											if(dValue > Con_doubleInvalid)
												strResult.Format(_T("%.2f"),dValue);
										}
									}
									else
									{
										int count = pBusiness->m_pSecurity->GetBondConversionRatePastCount();
										float fRate = Con_floatInvalid;
										if(count <= 0)
											break;
										for (int i=0;i<count;i++)
										{
											pBondConversionRatePast = pBusiness->m_pSecurity->GetBondConversionRatePastByIndex(i);
											if(pBondConversionRatePast == NULL)
												continue;
											if (reqTXFunction.nDate < pBondConversionRatePast->f_start)
												break;
											if ((int)reqTXFunction.nDate>=pBondConversionRatePast->f_start && (int)reqTXFunction.nDate <= pBondConversionRatePast->f_end)
											{
												fRate = pBondConversionRatePast->f_rate;
											}
										}
										dValue = fRate;
										if(dValue > Con_floatInvalid)
											strResult.Format(_T("%.2f"),dValue);
									}
								}
								break;
							case 251:     //ծȯ���±�׼ȯ�����ʿ�ʼ������
								{
									if(!pBusiness->m_pSecurity->IsBond() && !pBusiness->m_pSecurity->IsFund())
									{
										break;
									}
									FindItemOfReq(&reqTXFunction, strReq,3);
									BondConvRate* pBondConvRate = NULL;
									pBondConvRate = pBusiness->m_pSecurity->GetBondConvRate();
									if (pBondConvRate != NULL)
									{
										nDate = pBondConvRate->f_start;
										if (nDate > 0)
										{
											strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
										}
									}
								}
								break;
							case 252:     //ծȯ���±�׼ȯ�����ʽ���������
								{
									if(!pBusiness->m_pSecurity->IsBond() && !pBusiness->m_pSecurity->IsFund())
									{
										break;
									}
									FindItemOfReq(&reqTXFunction, strReq,3);
									BondConvRate* pBondConvRate = NULL;
									pBondConvRate = pBusiness->m_pSecurity->GetBondConvRate();
									if (pBondConvRate != NULL)
									{
										nDate = pBondConvRate->f_end;
										if (nDate > 0)
										{
											strResult.Format(_T("%04d%02d%02d"), nDate / 10000, (nDate % 10000) / 100, nDate % 100);
										}
									}
								}
								break;

							case 255:   //�����ڷݶ�
								{
									strResult = _T("");
									FindItemOfReq(&reqTXFunction, strReq, 3);	//Date
									if ( !pBusiness->m_pSecurity->IsFund())
										break;
									Tx::Data::FundShareChangeNei* pFundShareChangeNei = pBusiness->m_pSecurity->GetFundShareChangNeiByDate((int)reqTXFunction.nDate);
									if(pFundShareChangeNei == NULL)
										break;

									if(pFundShareChangeNei->totalShare > 0)
										strResult.Format(_T("%0.2f"),pFundShareChangeNei->totalShare);
								}
								break;
							case 264:   //�������ӷ��
								{
									vdReq.data_type = dtype_val_string;
									if(!pBusiness->m_pSecurity->IsFund())
									{
										break;
									}
									//1.ͨ������ʵ��ID��ȡ���ӷ��ID
									mapIntInt.clear();
									Tx::Data::TypeMapManage::GetInstance()->GetTypeMapITI(TYPE_YHFUND_STYLE_ID_TO_INDEX, mapIntInt);
									iterIntInt = mapIntInt.find(reqTXFunction.nSecurityID);
									if (iterIntInt != mapIntInt.end())
									{
										//2.��ȡ���ӷ��str
										mapIntStr.clear();
										Tx::Data::TypeMapManage::GetInstance()->GetTypeMap(TYPE_YHFUND_STYLE_INDEX, mapIntStr);
										iterIntStr = mapIntStr.find(iterIntInt->second);
										if(iterIntStr != mapIntStr.end())
										{
											strValue = iterIntStr->second;
											if(strValue.Compare(Tx::Core::Con_strInvalid) != 0)
											{
												strResult = strValue;
											}
										}
									}
								}
								break;
							case 265:   //ծȯ����
								{
									FindItemOfReq(&reqTXFunction, strReq, 23);	//Date
									int count = pBusiness->m_pSecurity->GetBondRatingCount();
									if(count <= 0)
										break;
									Tx::Data::BondRating* pBondRating = NULL;
									int temp;
									temp = (int)reqTXFunction.iQuotationNo;
									if(temp == 0)
									{
										pBondRating = pBusiness->m_pSecurity->GetBondRatingByIndex(0);
										if(pBondRating == NULL)
											break;
										strResult = pBondRating->rating;
									}
									else if(temp == 1)
									{
										pBondRating = pBusiness->m_pSecurity->GetBondRatingByIndex(count-1);
										if(pBondRating == NULL)
											break;
										strResult = pBondRating->rating;
									}
									else
									{
										CString str1=_T("");
										CString	str2=_T("");
										for (int i=count-1;i>=0;i--)
										{
											pBondRating = pBusiness->m_pSecurity->GetBondRatingByIndex(i);
											if(pBondRating == NULL)
												continue;
											str1 = pBondRating->rating;
											if(str2.IsEmpty())
												str2 = str1;
											else
											{
												str2 += _T(",");
												str2 += str1;
											}
										}
										strResult = str2;
									}
								}
								break;
							case 266:   //ծȯ��������
								{
									FindItemOfReq(&reqTXFunction, strReq, 23);	//Date
									int count = pBusiness->m_pSecurity->GetBondRatingCount();
									if(count <= 0)
										break;
									Tx::Data::BondRating* pBondRating = NULL;
									CString strTemp = _T("");
									int temp;
									temp = (int)reqTXFunction.iQuotationNo;
									if(temp == 0)
									{
										pBondRating = pBusiness->m_pSecurity->GetBondRatingByIndex(0);
										if(pBondRating == NULL)
											break;
									}
									else if(temp == 1)
									{
										pBondRating = pBusiness->m_pSecurity->GetBondRatingByIndex(count-1);
										if(pBondRating == NULL)
											break;
									}
									else
										break;
									strTemp = TypeMapManage::GetInstance()->GetDatByID(TYPE_INSTITUTION_CHINALONGNAME,pBondRating->institutionId);
									if(!strTemp.IsEmpty())
										strResult = strTemp;
								}
								break;
							case 267:   //ծȯ��������
								{
									FindItemOfReq(&reqTXFunction, strReq, 23);	//Date
									int count = pBusiness->m_pSecurity->GetBondRatingCount();
									if(count <= 0)
										break;
									Tx::Data::BondRating* pBondRating = NULL;
									int temp;
									temp = (int)reqTXFunction.iQuotationNo;
									if(temp == 0)
									{
										pBondRating = pBusiness->m_pSecurity->GetBondRatingByIndex(0);
										if(pBondRating == NULL)
											break;
										strResult.Format(_T("%d"),pBondRating->date);
									}
									else if(temp == 1)
									{
										pBondRating = pBusiness->m_pSecurity->GetBondRatingByIndex(count-1);
										if(pBondRating == NULL)
											break;
										strResult.Format(_T("%d"),pBondRating->date);
									}
									else
										break;
								}
								break;
							case 268:   //��ծ��������
								{
									FindItemOfReq(&reqTXFunction, strReq, 23);	//Date
									int count = pBusiness->m_pSecurity->GetBondIsserRatingCount();
									if(count <= 0)
										break;
									Tx::Data::BondRating* pBondRating = NULL;
									int temp;
									temp = (int)reqTXFunction.iQuotationNo;
									if(temp == 0)
									{
										pBondRating = pBusiness->m_pSecurity->GetBondIsserRatingByIndex(0);
										if(pBondRating == NULL)
											break;
										strResult = pBondRating->rating;
									}
									else if(temp == 1)
									{
										pBondRating = pBusiness->m_pSecurity->GetBondIsserRatingByIndex(count-1);
										if(pBondRating == NULL)
											break;
										strResult = pBondRating->rating;
									}
									else
									{
										CString str1=_T("");
										CString	str2=_T("");
										for (int i=count-1;i>=0;i--)
										{
											pBondRating = pBusiness->m_pSecurity->GetBondIsserRatingByIndex(i);
											if(pBondRating == NULL)
												continue;
											str1 = pBondRating->rating;
											if(str2.IsEmpty())
												str2 = str1;
											else
											{
												str2 += _T(",");
												str2 += str1;
											}
										}
										strResult = str2;
									}
								}
								break;
							case 269:   //��ծ������������
								{
									FindItemOfReq(&reqTXFunction, strReq, 23);	//Date
									int count = pBusiness->m_pSecurity->GetBondIsserRatingCount();
									if(count <= 0)
										break;
									Tx::Data::BondRating* pBondRating = NULL;
									int temp;
									temp = (int)reqTXFunction.iQuotationNo;
									if(temp == 0)
									{
										pBondRating = pBusiness->m_pSecurity->GetBondIsserRatingByIndex(0);
										if(pBondRating == NULL)
											break;
									}
									else if(temp == 1)
									{
										pBondRating = pBusiness->m_pSecurity->GetBondIsserRatingByIndex(count-1);
										if(pBondRating == NULL)
											break;
									}
									else
										break;
									CString strTemp = _T("");
									strTemp = TypeMapManage::GetInstance()->GetDatByID(TYPE_INSTITUTION_CHINALONGNAME,pBondRating->institutionId);
									if(!strTemp.IsEmpty())
										strResult = strTemp;
								}
								break;
							case 270:   //��ծ������������
								{
									FindItemOfReq(&reqTXFunction, strReq, 23);	//Date
									int count = pBusiness->m_pSecurity->GetBondIsserRatingCount();
									if(count <= 0)
										break;
									Tx::Data::BondRating* pBondRating = NULL;
									int temp;
									temp = (int)reqTXFunction.iQuotationNo;
									if(temp == 0)
									{
										pBondRating = pBusiness->m_pSecurity->GetBondIsserRatingByIndex(0);
										if(pBondRating == NULL)
											break;
										strResult.Format(_T("%d"),pBondRating->date);
									}
									else if(temp == 1)
									{
										pBondRating = pBusiness->m_pSecurity->GetBondIsserRatingByIndex(count-1);
										if(pBondRating == NULL)
											break;
										strResult.Format(_T("%d"),pBondRating->date);
									}
									else
										break;
								}
								break;
							case 271:   //ծȯ���¶Ҹ�������
								{
									if(!pBusiness->m_pSecurity->IsBond())
										break;
									BondCashing* pBondCashing = NULL;
									pBondCashing = pBusiness->m_pSecurity->GetBondCashingByIndex();
									int count = pBusiness->m_pSecurity->GetBondCashingCount();
									if (pBondCashing != NULL)
									{
										int iDate = pBondCashing->f_disclosure_date;
										if (iDate > 0)
										{
											strResult.Format(_T("%04d%02d%02d"), iDate / 10000, (iDate % 10000) / 100, iDate % 100);
										}
									}
								}
								break;
							case 272:   //ծȨ�Ǽ���
								{
									if(!pBusiness->m_pSecurity->IsBond())
										break;
									BondCashing* pBondCashing = NULL;
									pBondCashing = pBusiness->m_pSecurity->GetBondCashingByIndex();
									if (pBondCashing != NULL)
									{
										int iDate = pBondCashing->f_register_date;
										if (iDate > 0)
										{
											strResult.Format(_T("%04d%02d%02d"), iDate / 10000, (iDate % 10000) / 100, iDate % 100);
										}
									}
								}
								break;
							case 273:   //ծȯ�Ҹ���ʼ��
								{
									if(!pBusiness->m_pSecurity->IsBond())
										break;
									BondCashing* pBondCashing = NULL;
									pBondCashing = pBusiness->m_pSecurity->GetBondCashingByIndex();
									if (pBondCashing != NULL)
									{
										int iDate = pBondCashing->f_date_interest;
										if (iDate > 0)
										{
											strResult.Format(_T("%04d%02d%02d"), iDate / 10000, (iDate % 10000) / 100, iDate % 100);
										}
									}
								}
								break;
							case 274:   //ծȯ��һ�θ�Ϣ�Ľ�ֹ����
								{
									if(!pBusiness->m_pSecurity->IsBond())
										break;
									BondCashing* pBondCashing = NULL;
									pBondCashing = pBusiness->m_pSecurity->GetBondCashingByIndex();
									if (pBondCashing != NULL)
									{
										int iDate = pBondCashing->f_date_interestEnd;
										if (iDate > 0)
										{
											strResult.Format(_T("%04d%02d%02d"), iDate / 10000, (iDate % 10000) / 100, iDate % 100);
										}
									}
								}
								break;
							case 275:   //ÿ��Ԫ�ֽ���
								{
									FindItemOfReq(&reqTXFunction, strReq, 23);	//Date
									if(!pBusiness->m_pSecurity->IsBond())
										break;
									BondCashing* pBondCashing = NULL;
									float ftemp = 0.0;
									int count = pBusiness->m_pSecurity->GetBondCashingCount();
									int temp = (int)reqTXFunction.iQuotationNo;
									if (count <= 0)
										break;
									if(count == 2)
									{
										if (temp == 0)
										{
											pBondCashing = pBusiness->m_pSecurity->GetBondCashingByIndex(0);
											if(pBondCashing != NULL)
											{
												if (pBondCashing->btype == 1)
												{
													if(pBondCashing->f_interest_amount != Con_floatInvalid)
														ftemp = pBondCashing->f_interest_amount;
												}
												else
												{
													pBondCashing = pBusiness->m_pSecurity->GetBondCashingByIndex(1);
													if (pBondCashing != NULL&&pBondCashing->btype == 1 && pBondCashing->f_interest_amount != Con_floatInvalid)
													{
														ftemp = pBondCashing->f_interest_amount;
													}
												}
											}
										}
										else //����
										{
											pBondCashing = pBusiness->m_pSecurity->GetBondCashingByIndex(0);
											if(pBondCashing != NULL)
											{
												int itmp = pBondCashing->btype;
												if(pBondCashing->f_interest_amount != Con_floatInvalid)
													ftemp = pBondCashing->f_interest_amount;
												pBondCashing = pBusiness->m_pSecurity->GetBondCashingByIndex(1);
												if(pBondCashing != NULL&&itmp != pBondCashing->btype&& pBondCashing->f_interest_amount != Con_floatInvalid)
												{
													ftemp += pBondCashing->f_interest_amount;
												}
											}
										}
									}
									else
									{
										pBondCashing = pBusiness->m_pSecurity->GetBondCashingByIndex();
										if (pBondCashing != NULL && pBondCashing->f_interest_amount != Con_floatInvalid)
										{
											//��Ϣ
											if (temp == 0 && pBondCashing->btype == 1)
											{
												ftemp = pBondCashing->f_interest_amount;
											}
											//����
											if (temp == 1 && pBondCashing->btype == 0)
											{
												ftemp = pBondCashing->f_interest_amount;
											}
										}
									}

									if (ftemp != 0.0 && ftemp != Con_floatInvalid)
									{
										strResult.Format(_T("%0.3f"),ftemp);
									}
								}
								break;
							case 276:   //��Ϣ��
								{
									if(!pBusiness->m_pSecurity->IsBond())
										break;
									BondCashing* pBondCashing = NULL;
									pBondCashing = pBusiness->m_pSecurity->GetBondCashingByIndex();
									if (pBondCashing != NULL)
									{
										int iDate = pBondCashing->f_ex_dividend_date;
										if (iDate > 0)
										{
											strResult.Format(_T("%04d%02d%02d"), iDate / 10000, (iDate % 10000) / 100, iDate % 100);
										}
									}
								}
								break;
							case 277:   //����Ȩ��ֵ
								{
									if(!pBusiness->m_pSecurity->IsFund())
										break;
									FindItemOfReq(&reqTXFunction, strReq, 19);	//Date
									FundAdjUnitValue* pData = pBusiness->m_pSecurity->GetFundAdjUnitValueByDate((int)reqTXFunction.nDate);
									if(pData == NULL || pData->fNav == Con_floatInvalid)
										break;
									if (reqTXFunction.iHolderNo == 1)
									{
										strResult.Format(_T("%0.2f"),pData->fNav);
									}
									else
									{
										if(pData->nDate == (int)reqTXFunction.nDate)
										    strResult.Format(_T("%0.4f"),pData->fNav);
									}
								}
								break;
							case 288:   //�ݶֵ��׼��
								{
									if(!pBusiness->m_pSecurity->IsFund()) break;
									FundNVStandardize* pData = pBusiness->m_pSecurity->GetFundNVStandardize();
									if (pData != NULL)
									{
										int iStandardize = pData->flag;
										if(iStandardize == 0)
											strResult = _T("��");
										else if(iStandardize == 1)
											strResult = _T("��");
									}
								}
								break;
							//-------------------------------------------------------------------------
						default:
							break;
						}
					}
				}
#ifdef _DEBUG
				GlobalWatch::_GetInstance()->WatchHere(_T("wangzy|| request=%s,result=%s"),strReq,strResult);


				sLog.Format(_T("wangzf|| request=%s,result=%s"),strReq,strResult);
				Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
#endif
				if(strResult.GetLength() > 0)
				{
					iterSheetReq->second = strResult;
					//2008-06-12 by zhaohj
					//vdReq
					vdReq.FromString(strResult);
					iterVdReq->second = vdReq;
				}
#ifdef _DEBUG
				else
				{
					GlobalWatch::_GetInstance()->WatchHere(_T("wangzy|| request=%s,none result"),strReq);
				}
#endif
			}
			
			tmEnd = CTime::GetCurrentTime();

			singleLock.Lock();	//�̻߳���
			//ˢ��mapReq
			if(mapSheetReq.size() > 0)
			{
				for(iterSheetReq = mapSheetReq.begin(); iterSheetReq != mapSheetReq.end(); iterSheetReq ++)
				{
					if(iterSheetReq->second.GetLength() > 0)
					{
						iterReq = pmapReq->find(iterSheetReq->first);
						if(iterReq != pmapReq->end())
						{
							iterReq->second.strValue = iterSheetReq->second;

							//2008-06-12 by zhaohj
							iterVdReq = mapSheetReqVariantData.find(iterSheetReq->first);
							iterReq->second.vdReq = iterVdReq->second;

							iterReq->second.nCalculateTime = tmEnd.GetTime();
						}
					}
				}
			}
			
			singleLock.Unlock();

			mapSheetReq.clear();
			mapSheetReqVariantData.clear();//2008-06-12 by zhaohj
			vTopTenShareHolder.clear();
			vTopTenCShareHolder.clear();
			vTopTenFundHolder.clear();
			vTopTenCBondHolder.clear();
			vStockIssue.clear();
			vWarrantDV.clear();
			vCBondPrice.clear();
			vCBondInterest.clear();
			vCBondAmount.clear();
			vBondIPOInfo.clear();
			vFundNAVGrowth.clear();
			
			tmEnd = CTime::GetCurrentTime();
			//sLog.Format(_T("\n--��ʷָ���VBA�����������ʱ�䣺%4d-%02d-%02d %02d:%02d:%02d--\n"), 
			//	tmEnd.GetYear(), tmEnd.GetMonth(), tmEnd.GetDay(), 
			//	tmEnd.GetHour(), tmEnd.GetMinute(), tmEnd.GetSecond());
			//Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);	
			TRACE(_T("\n--��ʷָ���VBA�����������ʱ�䣺%4d-%02d-%02d %02d:%02d:%02d--\n"), 
				tmEnd.GetYear(), tmEnd.GetMonth(), tmEnd.GetDay(), 
				tmEnd.GetHour(), tmEnd.GetMinute(), tmEnd.GetSecond());

			singleLock.Lock();	//�̻߳���
			*(lpParam_HisIndCalc->lpnRefreshTime) = tmEnd.GetTime();
			if(*(lpParam_HisIndCalc->lpnShCalcEvtCnt) == 0)
			{
				*(lpParam_HisIndCalc->lpbAnyShCalcEvt) = FALSE;
			}
			(*(lpParam_HisIndCalc->lpbCalculate)) = FALSE;
			singleLock.Unlock();
		}
		//::Sleep(50);
	}
	//------------wanzhy-----------
	if ( pBalanceDataFile )
		delete pBalanceDataFile;
	if ( pFinancialDataFile )
		delete pFinancialDataFile;
	if( pCashFlowDataFile )
		delete pCashFlowDataFile;
	if ( pIncomeDataFile )
		delete pIncomeDataFile;
	if ( pRevenueDataFile )
		delete pRevenueDataFile;
	if ( pDepreciationDataFile )
		delete pDepreciationDataFile;
	if ( pGainsAndLossesDataFile )
		delete pGainsAndLossesDataFile;
	if ( pAccountsReceivableDataFile )
		delete pAccountsReceivableDataFile;
	if ( pFinanceChargeDataFile )
		delete pFinanceChargeDataFile;
	//---------------------zhangxs 20080822-------------
	if ( pFundTXInvDistributionDataFile )
		delete pFundTXInvDistributionDataFile;
	if( pFundHoldStockDetailDataFile )
		delete pFundHoldStockDetailDataFile;
	if ( pFundPositionInfoDataFile )
		delete pFundPositionInfoDataFile;
	if ( pFundShareDataChangeDataFile )
		delete pFundShareDataChangeDataFile;
	if ( pFundBuyTotStockDataFile )
		delete pFundBuyTotStockDataFile;
	if ( pFundSaleTotStockDataFile )
		delete pFundSaleTotStockDataFile;
	if ( pFundTradeVolumeInfoDataFile )
		delete pFundTradeVolumeInfoDataFile;

	//--------------20080529-------------
	if ( pFundIndexVipStockDataFile )
		delete pFundIndexVipStockDataFile;
	if ( pFundstockVipStockDataFile )
		delete pFundstockVipStockDataFile;
	if (pFundCombineVipStockDataFile)
		delete pFundCombineVipStockDataFile;
	if (pFundIndixInvDistributionDataFile)
		delete pFundIndixInvDistributionDataFile;
	if (pFundStockInvDistributeDataFile)
		delete pFundStockInvDistributeDataFile;
	if (pFundComInvDistributeDataFile)
		delete pFundComInvDistributeDataFile;
	if (pFundNavChangeDataFile)
		delete pFundNavChangeDataFile;
	if (pFundRevenueDataFile)
		delete pFundRevenueDataFile;
	if (pFundAchievementDataFile)
		delete pFundAchievementDataFile;
	if (pFundFinancialDataFile)
		delete pFundFinancialDataFile;
	if (pFundBalanceDataFile)
		delete pFundBalanceDataFile;
	if (pFundBondGroupDataFile)
		delete pFundBondGroupDataFile;
	if (pFundBondDetailDataFile)
		delete pFundBondDetailDataFile;
	if (pFundCBondDetailDataFile)
		delete pFundCBondDetailDataFile;
	if ( pFundOtherAssetDataFile)
		delete pFundOtherAssetDataFile;
	if ( pAmountFlowDataFile )
	{
		delete pAmountFlowDataFile;
		pAmountFlowDataFile = NULL;
	}
	if ( pBalanceStatDataFile )
	{
		delete pBalanceStatDataFile;
		pBalanceStatDataFile = NULL;
	}
	if ( pProfitStatDataFile )
	{
		delete pProfitStatDataFile;
		pProfitStatDataFile = NULL;
	}
	if ( pCashFlowStatDataFile)
	{
		delete pCashFlowStatDataFile;
		pCashFlowStatDataFile = NULL;
	}
	if ( pFundInvesmentGroupDataFile )
	{
		delete pFundInvesmentGroupDataFile;
		pFundInvesmentGroupDataFile = NULL;
	}
	//added by zhangxs 20090218---------------------
	if( m_pBalanceYHDataFile )
	{
		delete m_pBalanceYHDataFile;
		m_pBalanceYHDataFile = NULL;
	}
	if( m_pBalanceBXDataFile )
	{
		delete m_pBalanceBXDataFile;
		m_pBalanceBXDataFile = NULL;
	}
	if( m_pBalanceZQDataFile )
	{
		delete m_pBalanceZQDataFile;
		m_pBalanceZQDataFile = NULL;
	}
	if( m_pProfitYHDataFile )
	{
		delete m_pProfitYHDataFile;
		m_pProfitYHDataFile = NULL;
	}
	if( m_pProfitBXDataFile )
	{
		delete m_pProfitBXDataFile;
		m_pProfitBXDataFile = NULL;
	}
	if( m_pProfitZQDataFile )
	{
		delete m_pProfitZQDataFile;
		m_pProfitZQDataFile = NULL;
	}
	if( m_pCashFlowYHDataFile)
	{
		delete m_pCashFlowYHDataFile;
		m_pCashFlowYHDataFile = NULL;
	}
	if( m_pCashFlowBXDataFile )
	{
		delete m_pCashFlowBXDataFile;
		m_pCashFlowBXDataFile = NULL;
	}
	if( m_pCashFlowZQDataFile )
	{
		delete m_pCashFlowZQDataFile;
		m_pCashFlowZQDataFile = NULL;
	}
	*(lpParam_HisIndCalc->lpbThreadTerm) = TRUE;

	sLog = _T("Excel�˳���ʷ����");
	//Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);

	return 0;
}

UINT TxIndicator::ClearReqMap_Proc(LPVOID lpParam)
{
	CTime tmNow, tmRefresh, tmReq;
	CTimeSpan tsRefresh, tsReq;
	ReqMapIter iterReq;
	StrSet setDeprecatedReq;
	StrSetIter iterDeprecatedReq;
	time_t nLashRefreshTime = 0;
	ClearReqMap_Param *lpClearReqMapParam = (ClearReqMap_Param *)lpParam;
	CSingleLock singleLock(&_critsect);

	while(*(lpClearReqMapParam->lpbThreadRun))
	{
		tmNow = CTime::GetCurrentTime();
		
		singleLock.Lock();
		if(*(lpClearReqMapParam->lpnRefreshTime) > nLashRefreshTime)
		{	//�������
			nLashRefreshTime = *(lpClearReqMapParam->lpnRefreshTime);
		}else if(nLashRefreshTime > 0 && (*(lpClearReqMapParam->lpbCalculate)) == FALSE)
		{	//δ����
			tmRefresh = CTime(nLashRefreshTime);
			tsRefresh = tmNow - tmRefresh;

			if(tsRefresh.GetTotalSeconds() > 600 && 
				!(*(lpClearReqMapParam->lpbReqMapCleared)) && 
				!(*(lpClearReqMapParam->lpbAnyShCalcEvt)))
			{	//�������ָ���VBA����
				setDeprecatedReq.clear();

				for(iterReq = mapRequest.begin(); iterReq != mapRequest.end(); iterReq ++)
				{
					if(iterReq->second.nReqTime > 0)
					{
						tmReq = CTime(iterReq->second.nReqTime);
						tsReq = tmNow - tmReq;
						if(tsReq.GetTotalSeconds() > 1800)
						{
							setDeprecatedReq.insert(iterReq->first);
						}
					}
				}

				for(iterDeprecatedReq = setDeprecatedReq.begin(); iterDeprecatedReq != setDeprecatedReq.end(); iterDeprecatedReq++)
				{
					mapRequest.erase(*iterDeprecatedReq);
				}

				*(lpClearReqMapParam->lpbReqMapCleared) = TRUE;
			}
		}
		singleLock.Unlock();

		::Sleep(50);
	}

	*(lpClearReqMapParam->lpbThreadTerminated) = TRUE;
	return 0;
}

UINT TxIndicator::ClearResult_Proc(void)
{
	HisIndCalc_Param *lpParam_HisIndCalc = &param_HisIndCalc;
	ReqMap *pmapReq = lpParam_HisIndCalc->pmapReq;
	CSingleLock singleLock(&_critsect);
	singleLock.Lock();
	pmapReq->clear();
	singleLock.Unlock();
	//Tx::Log::CLogRecorder::GetInstance()->WriteToLog(_T("-----------������----------"),TRUE);
	return 0;
}

UINT TxIndicator::LoadData_Proc(LPVOID lpParam)
{
	LoadData_Param *lpParam_LoadData = (LoadData_Param *)lpParam;
	//lpParam_LoadData->pArrayStatistics->ShowProgress(lpParam_LoadData->pParentWnd);

	return 0;
}

void TxIndicator::InitTXEEFunctions()
{
	if(m_bInited == true) return;
	m_bInited = true;

	CFile BinFile;
	TXEE_FuncGroup item_TXEE_FuncGroup;
	TXEE_Function item_TXEE_Function;
	TXEE_Indicator item_TXEE_Indicator;
	TXEE_Param item_TXEE_Param;
	TXEE_Indicator_Iter IterIndicator;
	INT i, nCount, nID, iParamID;
	CString strBinFilePath, strItem;
	Tx::Data::IndicatorData *pIndicatorData;

	mapTXEEFuncGroup.clear();
	strBinFilePath = Tx::Core::SystemPath::GetInstance()->GetConfigExcelPath() + _T("\\TXEE_FuncGroup.bin");
	BinFile.Open(strBinFilePath, CFile::modeRead);
	nCount = BinFile.GetLength() / (sizeof(INT) + sizeof(TXEE_FuncGroup));
	for(i = 0; i < nCount; i++)
	{
		BinFile.Read(&nID, sizeof(INT));
		BinFile.Read(&item_TXEE_FuncGroup, sizeof(TXEE_FuncGroup));
		mapTXEEFuncGroup.insert(TXEE_FuncGroup_Pair(nID, item_TXEE_FuncGroup));
	}
	BinFile.Close();

	mapTXEEFunction.clear();
	strBinFilePath = Tx::Core::SystemPath::GetInstance()->GetConfigExcelPath() + _T("\\TXEE_Function.bin");
	BinFile.Open(strBinFilePath, CFile::modeRead);
	nCount = BinFile.GetLength() / (sizeof(INT) + sizeof(TXEE_Function));
	for(i = 0; i < nCount; i++)
	{
		BinFile.Read(&nID, sizeof(INT));
		BinFile.Read(&item_TXEE_Function, sizeof(TXEE_Function));
		mapTXEEFunction.insert(TXEE_Function_Pair(nID, item_TXEE_Function));
	}
	BinFile.Close();

	mapTXEEIndicator.clear();
	strBinFilePath = Tx::Core::SystemPath::GetInstance()->GetConfigExcelPath() + _T("\\TXEE_Indicator.bin");
	BinFile.Open(strBinFilePath, CFile::modeRead);
	nCount = BinFile.GetLength() / (sizeof(INT) + sizeof(TXEE_Indicator));
	for(i = 0; i < nCount; i++)
	{
		BinFile.Read(&nID, sizeof(INT));
		BinFile.Read(&item_TXEE_Indicator, sizeof(TXEE_Indicator));
		mapTXEEIndicator.insert(TXEE_Indicator_Pair(nID, item_TXEE_Indicator));
	}
	BinFile.Close();

	mapTXEEParam.clear();
	strBinFilePath = Tx::Core::SystemPath::GetInstance()->GetConfigExcelPath() + _T("\\TXEE_Param.bin");
	BinFile.Open(strBinFilePath, CFile::modeRead);
	nCount = BinFile.GetLength() / (sizeof(INT) + sizeof(TXEE_Param));
	for(i = 0; i < nCount; i++)
	{
		BinFile.Read(&nID, sizeof(INT));
		BinFile.Read(&item_TXEE_Param, sizeof(TXEE_Param));
		mapTXEEParam.insert(TXEE_Param_Pair(nID, item_TXEE_Param));
	}
	BinFile.Close();

	mapTXEEParamOfFunc.clear();
	strBinFilePath = Tx::Core::SystemPath::GetInstance()->GetConfigExcelPath()+ _T("\\TXEE_ParamOfFunc.bin");
	BinFile.Open(strBinFilePath, CFile::modeRead);
	nCount = BinFile.GetLength() / (sizeof(INT) + sizeof(INT));
	for(i = 0; i < nCount; i++)
	{
		BinFile.Read(&nID, sizeof(INT));
		BinFile.Read(&iParamID, sizeof(INT));
		mapTXEEParamOfFunc.insert(INT_Pair(nID, iParamID));
	}
	BinFile.Close();

	mapTXEEItem.clear();
	for(IterIndicator = mapTXEEIndicator.begin(); IterIndicator != mapTXEEIndicator.end(); IterIndicator++)
	{
		if(IterIndicator->second.nIndicatorID > 0)
		{
			if((IterIndicator->first / 100) == 44)
			{	//��Ʒ����ֲ�
				strItem.Format(_T("%s"), cIncomeDistribution[IterIndicator->first % 100 - 1]);
				mapTXEEItem.insert(INTSTR_Pair(IterIndicator->first, strItem));
				continue;
			}
			if((IterIndicator->first / 100) == 50)
			{	//Ӧ���ʿ�
				strItem.Format(_T("%s"), cAccountsReceivable[IterIndicator->first % 100 - 1]);
				mapTXEEItem.insert(INTSTR_Pair(IterIndicator->first, strItem));
				continue;
			}
			if((IterIndicator->first / 100) == 55)
			{	//��Ҫ����ָ���������ָ��
				strItem.Format(_T("%s"), cCWBriefString[IterIndicator->first % 100 - 1]);
				mapTXEEItem.insert(INTSTR_Pair(IterIndicator->first, strItem));
				continue;
			}
			if((IterIndicator->first / 100) == 108 && (IterIndicator->first % 100) == 1)
			{	//ծȯ������Ϣ��������ʼ����-->��������
				strItem = _T("��������");
				mapTXEEItem.insert(INTSTR_Pair(IterIndicator->first, strItem));
				continue;
			}
			if((IterIndicator->first / 100) == 110 && (IterIndicator->first % 100) == 1)
			{	//ծȯ������Ϣ��ծȯ��������-->���н��
				strItem = _T("���й�ģ");
				mapTXEEItem.insert(INTSTR_Pair(IterIndicator->first, strItem));
				continue;
			}
			if((IterIndicator->first / 100) == 111 && (IterIndicator->first % 100) == 2)
			{	//ծȯ�����ˣ�����ȫ��-->������
				strItem = _T("������");
				mapTXEEItem.insert(INTSTR_Pair(IterIndicator->first, strItem));
				continue;
			}
			if((IterIndicator->first / 100) == 111 && (IterIndicator->first % 100) == 7)
			{	//ծȯ������Ϣ�����������õȼ�-->ծȯ���õȼ�
				strItem = _T("ծȯ���õȼ�");
				mapTXEEItem.insert(INTSTR_Pair(IterIndicator->first, strItem));
				continue;
			}
			if ( IterIndicator->first == 19101 || IterIndicator->first == 19601 || IterIndicator->first == 19701)
			{
				strItem = _T("��Ʊ����");
				mapTXEEItem.insert(INTSTR_Pair(IterIndicator->first, strItem));
				continue;
			}
			if (  IterIndicator->first == 19301 || IterIndicator->first == 19401 )
			{
				strItem = _T("ծȯ����");
				mapTXEEItem.insert(INTSTR_Pair(IterIndicator->first, strItem));
				continue;
			}
			if ( IterIndicator->first == 19001 )
			{
				strItem = _T("ȯ������");
				mapTXEEItem.insert(INTSTR_Pair(IterIndicator->first, strItem));
				continue;
			}
			//����ָ�꺯��
			pIndicatorData = m_pBusiness->GetIndicatorDataNow(IterIndicator->second.nIndicatorID);
			if(pIndicatorData != NULL)
			{
				strItem.Format(_T("%s"), (CString)pIndicatorData->cn_name);
				strItem.Trim();
				mapTXEEItem.insert(INTSTR_Pair(IterIndicator->first, strItem));
			}
		}
	}

	INT iIndustryID, iIndicatorID;
	INT_Map intMap;
	map<INT,INT_Map>::iterator iterMMap;

	mmapIndicator2IndustryID.clear();
	strBinFilePath = Tx::Core::SystemPath::GetInstance()->GetConfigExcelPath()+ _T("\\TXEE_ParamID2IndustryID.bin");
	BinFile.Open(strBinFilePath, CFile::modeRead);
	nCount = BinFile.GetLength() / (sizeof(INT)*3);
	for(i = 0; i < nCount; i++)
	{
		BinFile.Read(&nID, sizeof(INT));
		BinFile.Read(&iIndicatorID, sizeof(INT));
		BinFile.Read(&iIndustryID, sizeof(INT));
		iterMMap = mmapIndicator2IndustryID.find(nID);
		if (iterMMap == mmapIndicator2IndustryID.end())
		{
			intMap.clear();
			intMap.insert(INT_Pair(iIndicatorID,iIndustryID));
			mmapIndicator2IndustryID.insert(std::pair<INT,INT_Map>(nID,intMap));
		}
		else
		{
			iterMMap->second.insert(INT_Pair(iIndicatorID,iIndustryID));
		}
	}
	BinFile.Close();
}

void TxIndicator::ClearTables()
{
	if ( m_bInitTables == false)
		return;

	int i;
	for(i = 0; i < 1; i++)
	{
		tableIndDates[i].Clear();
	}
	for(i = 0; i < 6; i++)
	{
		tableIndDate[i].Clear();
	}
	for(i = 0; i < 4; i++)
	{
		tableShareHolder[i].Clear();
	}
	for(i = 0; i < 13; i++)
	{
		tableIndID[i].Clear();
	}
	m_tableQS.Clear();
	m_tableFundManager.Clear();
	mmapNameCode.clear();
	mapCodeID.clear();
	//2012-12-25
	mapFundCodeID.clear();
	mapBondCodeID.clear();

	m_tableBondTmp.Clear();
	m_bInitTables = false;
}


//�����ʼ���������ļ��ı�-------------wangzhy------------------------
void TxIndicator::InitTables(Tx::Business::TxBusiness *pBusiness, bool bDownAll )
{
	if(m_bInitTables == true)
		return;
	m_bInitTables = true;
	BOOL bResult = FALSE;
	INT iSecurityID, iFuncID;
	TXEE_Indicator_Iter iterIndicator;
	Tx::Core::Table_Indicator *pSecurityTable = NULL;
	Tx::Business::TxBusiness business;
	UINT i, array_ParamCol[3] = {0, 1, 2};

	//����ֵ����
	tableIndDates[0].AddParameterColumn(Tx::Core::dtype_int4);		//����ʵ��ID
	tableIndDates[0].AddParameterColumn(Tx::Core::dtype_int4);		//��Ȩ��
	tableIndDates[0].AddIndicatorColumn(30301125, Tx::Core::dtype_double, array_ParamCol, 2);	//����ǰ��ֵ
	tableIndDates[0].AddIndicatorColumn(30301126, Tx::Core::dtype_double, array_ParamCol, 2);	//������ֵ
	//ȡ�û���ֵ������������
	if ( bDownAll )
		bResult = pBusiness->m_pLogicalBusiness->GetData(tableIndDates[0], true);
	else
		bResult = FALSE;
	if(!bResult)
	{
		tableIndDates[0].Clear();
	}
	//m_pArrayStatistics->MakeProgress();

	//��Ʊ����
	tableIndDate[0].AddParameterColumn(Tx::Core::dtype_int4);	//�¼�ID
	tableIndDate[0].AddParameterColumn(Tx::Core::dtype_int4);	//ȯID
	tableIndDate[0].AddIndicatorColumn(30300146, Tx::Core::dtype_int4, array_ParamCol, 2);		//��������
	tableIndDate[0].AddIndicatorColumn(30300136, Tx::Core::dtype_decimal, array_ParamCol, 2);	//���м۸�
	//ȡ�ù�Ʊ������������
	if ( bDownAll )
		bResult = pBusiness->m_pLogicalBusiness->GetData(tableIndDate[0], true);
	else
		bResult = FALSE;
	//if(!bResult)
	//{
	//	tableIndDate[0].DeleteRow( 0, tableIndDate[0].GetRowCount());
	//	tableIndDate[0].Arrange();
	//	//tableIndDate[0].Clear();
	//}
	//m_pArrayStatistics->MakeProgress();

	//Ȩ֤ʣ�ഴ�����
	tableIndDate[1].AddParameterColumn(Tx::Core::dtype_val_string);	//ȯ��id
	tableIndDate[1].AddParameterColumn(Tx::Core::dtype_int4);		//�����ע������
	tableIndDate[1].AddParameterColumn(Tx::Core::dtype_int4);		//����ʵ��id
	tableIndDate[1].AddIndicatorColumn(30601060, Tx::Core::dtype_decimal, array_ParamCol, 3);	//Ȩ֤�����ע����
	//ȡ��Ȩ֤����/ע����������
	if ( bDownAll )
		bResult = pBusiness->m_pLogicalBusiness->GetData(tableIndDate[1], true);
	else
		bResult = FALSE;
	//if(!bResult)
	//{
	//	tableIndDate[1].Clear();
	//}
	//m_pArrayStatistics->MakeProgress();

	//��תծת�ɼ۸�
	tableIndDate[2].AddParameterColumn(Tx::Core::dtype_int4);	//ȯID
	tableIndDate[2].AddParameterColumn(Tx::Core::dtype_int4);	//��Ч����
	tableIndDate[2].AddIndicatorColumn(30301170, Tx::Core::dtype_decimal, array_ParamCol, 2);		//ת�ɼ۸�
	//ȡ��ת�ɼ۸���������
	if ( bDownAll )
		bResult = pBusiness->m_pLogicalBusiness->GetData(tableIndDate[2], true);
	else
		bResult = FALSE;
	//if(!bResult)
	//{
	//	tableIndDate[2].Clear();
	//}
	//m_pArrayStatistics->MakeProgress();

	//��תծ����
	tableIndDate[3].AddParameterColumn(Tx::Core::dtype_int4);	//ȯID
	tableIndDate[3].AddParameterColumn(Tx::Core::dtype_int4);	//��ʼ����
	tableIndDate[3].AddParameterColumn(Tx::Core::dtype_int4);	//��������
	tableIndDate[3].AddIndicatorColumn(30301159, Tx::Core::dtype_decimal, array_ParamCol, 3);		//˰ǰ����
	//ȡ��תծ������������
	if ( bDownAll )
		bResult = pBusiness->m_pLogicalBusiness->GetData(tableIndDate[3], true);
	else
		bResult = FALSE;
	//if(!bResult)
	//{
	//	tableIndDate[3].Clear();
	//}
	//m_pArrayStatistics->MakeProgress();

	//��תծת������
	tableIndDate[4].AddParameterColumn(Tx::Core::dtype_int4);	//ȯID
	tableIndDate[4].AddParameterColumn(Tx::Core::dtype_int4);	//��ֹ����
	tableIndDate[4].AddIndicatorColumn(30301162, Tx::Core::dtype_decimal, array_ParamCol, 2);		//��ת��ծȯ���
	tableIndDate[4].AddIndicatorColumn(30301163, Tx::Core::dtype_decimal, array_ParamCol, 2);		//�ۼ�ת������
	tableIndDate[4].AddIndicatorColumn(30301164, Tx::Core::dtype_decimal, array_ParamCol, 2);		//δת��ծȯ���
	//ȡ�ÿ�תծת��������������
	if ( bDownAll )
		bResult = pBusiness->m_pLogicalBusiness->GetData(tableIndDate[4], true);
	else
		bResult = FALSE;
#ifdef _DEBUG
	CString strTableIndDate4 = tableIndDate[4].TableToString();
	Tx::Core::Commonality::String().StringToClipboard(strTableIndDate4);
#endif
	//if(!bResult)
	//{
	//	tableIndDate[4].Clear();
	//}
	//m_pArrayStatistics->MakeProgress();

	//ծȯ������Ϣ
	tableIndDate[5].AddParameterColumn(Tx::Core::dtype_int4);	//ȯID
	tableIndDate[5].AddParameterColumn(Tx::Core::dtype_int4);	//��������
	tableIndDate[5].AddIndicatorColumn(30301174, Tx::Core::dtype_decimal, array_ParamCol, 2);		//���м۸�
	tableIndDate[5].AddIndicatorColumn(30301177, Tx::Core::dtype_decimal, array_ParamCol, 2);		//���з���
	tableIndDate[5].AddIndicatorColumn(30301178, Tx::Core::dtype_decimal, array_ParamCol, 2);		//���ʽ��
	tableIndDate[5].AddIndicatorColumn(30301179, Tx::Core::dtype_int4, array_ParamCol, 2);			//���з�ʽ(id)
	tableIndDate[5].AddIndicatorColumn(30301182, Tx::Core::dtype_int4, array_ParamCol, 2);			//������ʼ����
	tableIndDate[5].AddIndicatorColumn(30301183, Tx::Core::dtype_int4, array_ParamCol, 2);			//������ֹ����
	tableIndDate[5].AddIndicatorColumn(30301184, Tx::Core::dtype_val_string, array_ParamCol, 2);	//ծȯ���õȼ�
	tableIndDate[5].AddIndicatorColumn(30301186, Tx::Core::dtype_val_string, array_ParamCol, 2);	//��������
	tableIndDate[5].AddIndicatorColumn(30301215, Tx::Core::dtype_val_string, array_ParamCol, 2);	//������������
	tableIndDate[5].AddIndicatorColumn(30301216, Tx::Core::dtype_val_string, array_ParamCol, 2);	//������
	tableIndDate[5].AddIndicatorColumn(30301249, Tx::Core::dtype_val_string, array_ParamCol, 2);	//������ʽ
	tableIndDate[5].AddIndicatorColumn(30301250, Tx::Core::dtype_val_string, array_ParamCol, 2);	//�����Ƽ���
	tableIndDate[5].AddIndicatorColumn(30301251, Tx::Core::dtype_decimal, array_ParamCol, 2);		//�ɶ�����������
	tableIndDate[5].AddIndicatorColumn(30301252, Tx::Core::dtype_decimal, array_ParamCol, 2);		//�������۷�����
	tableIndDate[5].AddIndicatorColumn(30301253, Tx::Core::dtype_decimal, array_ParamCol, 2);		//������ǩ��
	tableIndDate[5].AddIndicatorColumn(30301254, Tx::Core::dtype_decimal, array_ParamCol, 2);		//���»���������
	tableIndDate[5].AddIndicatorColumn(30301255, Tx::Core::dtype_decimal, array_ParamCol, 2);		//�������۱���
	tableIndDate[5].AddIndicatorColumn(30301256, Tx::Core::dtype_decimal, array_ParamCol, 2);		//�������̰�����
	//ȡ��ծȯ������Ϣ��������
	if ( bDownAll )
		bResult = pBusiness->m_pLogicalBusiness->GetData(tableIndDate[5], true);
	else
		bResult = FALSE;
	//if(!bResult)
	//{
	//	tableIndDate[5].Clear();
	//}
	//m_pArrayStatistics->MakeProgress();

	//ʮ��ɶ�
	tableShareHolder[0].AddParameterColumn(Tx::Core::dtype_int4);	//����ID
	tableShareHolder[0].AddParameterColumn(Tx::Core::dtype_int4);	//��ֹ����
	tableShareHolder[0].AddParameterColumn(Tx::Core::dtype_int4);	//���
	tableShareHolder[0].AddIndicatorColumn(30600020, Tx::Core::dtype_val_string, array_ParamCol, 3);	//�ɶ�����
	tableShareHolder[0].AddIndicatorColumn(30600023, Tx::Core::dtype_decimal, array_ParamCol, 3);		//�ֹ���
	tableShareHolder[0].AddIndicatorColumn(30600025, Tx::Core::dtype_decimal, array_ParamCol, 3);		//�ֹɱ���
	tableShareHolder[0].AddIndicatorColumn(30600021, Tx::Core::dtype_val_string, array_ParamCol, 3);	//�ɶ���־(�ֹ�����)
	//ȡ��ʮ��ɶ���������
	if ( bDownAll )
		bResult = pBusiness->m_pLogicalBusiness->GetData(tableShareHolder[0], true);
	else
		bResult = FALSE;
	//if(!bResult)
	//{
	//	tableShareHolder[0].Clear();
	//}
	//m_pArrayStatistics->MakeProgress();

	//ʮ����ͨ�ɶ�
	tableShareHolder[1].AddParameterColumn(Tx::Core::dtype_int4);	//����ID
	tableShareHolder[1].AddParameterColumn(Tx::Core::dtype_int4);	//��ֹ����
	tableShareHolder[1].AddParameterColumn(Tx::Core::dtype_int4);	//���
	tableShareHolder[1].AddIndicatorColumn(30600028, Tx::Core::dtype_val_string, array_ParamCol, 3);	//�ɶ�����
	tableShareHolder[1].AddIndicatorColumn(30600030, Tx::Core::dtype_decimal, array_ParamCol, 3);		//�ֹ���
	tableShareHolder[1].AddIndicatorColumn(30600033, Tx::Core::dtype_decimal, array_ParamCol, 3);		//�ֹɱ���
	tableShareHolder[1].AddIndicatorColumn(30600035, Tx::Core::dtype_decimal, array_ParamCol, 3);		//��ͨ����
	tableShareHolder[1].AddIndicatorColumn(30600031, Tx::Core::dtype_val_string, array_ParamCol, 3);	//�ֹ�����
	//ȡ��ʮ����ͨ�ɶ���������
	if ( bDownAll )
		bResult = pBusiness->m_pLogicalBusiness->GetData(tableShareHolder[1], true);
	else
		bResult = FALSE;
	//if(!bResult)
	//{
	//	tableShareHolder[1].Clear();
	//}
	//m_pArrayStatistics->MakeProgress();

	//���ʽ����ʮ�������
	tableShareHolder[2].AddParameterColumn(Tx::Core::dtype_int4);	//����ID
	tableShareHolder[2].AddParameterColumn(Tx::Core::dtype_int4);	//��ֹ����
	tableShareHolder[2].AddParameterColumn(Tx::Core::dtype_byte);	//���
	tableShareHolder[2].AddIndicatorColumn(30601003, Tx::Core::dtype_val_string, array_ParamCol, 3);	//�ɶ�����
	tableShareHolder[2].AddIndicatorColumn(30601006, Tx::Core::dtype_decimal, array_ParamCol, 3);		//�ֹ���
	tableShareHolder[2].AddIndicatorColumn(30601008, Tx::Core::dtype_decimal, array_ParamCol, 3);		//�ֹɱ���
	//ȡ�÷��ʽ����ʮ���������������
	if ( bDownAll )
		bResult = pBusiness->m_pLogicalBusiness->GetData(tableShareHolder[2], true);
	else
		bResult = FALSE;
	//if(!bResult)
	//{
	//	tableShareHolder[2].Clear();
	//}
	//m_pArrayStatistics->MakeProgress();

	//��תծʮ�������
	tableShareHolder[3].AddParameterColumn(Tx::Core::dtype_int4);	//ȯID
	tableShareHolder[3].AddParameterColumn(Tx::Core::dtype_int4);	//��ֹ����
	tableShareHolder[3].AddParameterColumn(Tx::Core::dtype_int4);	//���
	tableShareHolder[3].AddIndicatorColumn(30601034, Tx::Core::dtype_val_string, array_ParamCol, 3);	//������
	tableShareHolder[3].AddIndicatorColumn(30601035, Tx::Core::dtype_decimal, array_ParamCol, 3);		//��������
	tableShareHolder[3].AddIndicatorColumn(30601062, Tx::Core::dtype_decimal, array_ParamCol, 3);		//���б���
	//ȡ�ÿ�תծʮ���������������
	if ( bDownAll )
		bResult = pBusiness->m_pLogicalBusiness->GetData(tableShareHolder[3], true);
	else
		bResult = FALSE;
	//if(!bResult)
	//{
	//	tableShareHolder[3].Clear();
	//}
	//m_pArrayStatistics->MakeProgress();

	//�����ʲ���ֵ
	tableIndID[0].AddParameterColumn(Tx::Core::dtype_int4);			//ȯID
	tableIndID[0].AddParameterColumn(Tx::Core::dtype_int4);			//�������
	tableIndID[0].AddParameterColumn(Tx::Core::dtype_int4);			//���񼾶�(40040000 + FQuarter)
	tableIndID[0].AddIndicatorColumn(30901140, Tx::Core::dtype_decimal, array_ParamCol, 3);				//�����ʲ���ֵ
	//ȡ�û����ʲ���ֵ��������
	if ( bDownAll )
		bResult = pBusiness->m_pLogicalBusiness->GetData(tableIndID[0], true);
	else
		bResult = FALSE;
	//if(!bResult)
	//{
	//	tableIndID[0].Clear();
	//}
	//m_pArrayStatistics->MakeProgress();

	//��תծ������Ϣ
	tableIndID[1].AddParameterColumn(Tx::Core::dtype_int4);			//ȯID
	tableIndID[1].AddIndicatorColumn(30001140, Tx::Core::dtype_int4, array_ParamCol, 1);				//��Ʊ����ʵ��
	tableIndID[1].AddIndicatorColumn(30001143, Tx::Core::dtype_int4, array_ParamCol, 1);				//ת����ʼ��
	tableIndID[1].AddIndicatorColumn(30001144, Tx::Core::dtype_int4, array_ParamCol, 1);				//ת����ֹ��
	tableIndID[1].AddIndicatorColumn(30001151, Tx::Core::dtype_int4, array_ParamCol, 1);				//�����ʼ��
	tableIndID[1].AddIndicatorColumn(30001154, Tx::Core::dtype_int4, array_ParamCol, 1);				//���ۿ�ʼ��
	tableIndID[1].AddIndicatorColumn(30001119, Tx::Core::dtype_int4, array_ParamCol, 1);				//������id
	//ȡ�ÿ�תծ������Ϣ��������
	if ( bDownAll )
		bResult = pBusiness->m_pLogicalBusiness->GetData(tableIndID[1], true);
	else
		bResult = FALSE;
		//��תծ������Ϣ
	m_tableBondTmp.AddParameterColumn(Tx::Core::dtype_int4);			//ȯID
	m_tableBondTmp.AddIndicatorColumn(30001145, Tx::Core::dtype_val_string, array_ParamCol, 1);				//������id
	if ( bDownAll )
		bResult = pBusiness->m_pLogicalBusiness->GetData(m_tableBondTmp, true);
	else
		bResult = FALSE;
#ifdef _DEBUG
CString strTable = m_tableBondTmp.TableToString();
Tx::Core::Commonality::String().StringToClipboard(strTable);
#endif
	//if(!bResult)
	//{
	//	tableIndID[1].Clear();
	//}
	//m_pArrayStatistics->MakeProgress();

	//��Ҫ����ָ���
	tableIndID[2].AddParameterColumn(Tx::Core::dtype_int8);			//��ƺ���ID
	iFuncID = 40;	//��Ҫ����ָ���������Ŀ
	iterIndicator = mapTXEEIndicator.find(iFuncID * 100 + 1);
	while(iterIndicator != mapTXEEIndicator.end() && (iterIndicator->first / 100) == iFuncID)
	{
		tableIndID[2].AddIndicatorColumn((UINT)(iterIndicator->second.nIndicatorID), Tx::Core::dtype_decimal, array_ParamCol, 1);
		iterIndicator ++ ;
	}
	iFuncID = 55;	//��Ҫ����ָ����������Ŀ
	iterIndicator = mapTXEEIndicator.find(iFuncID * 100 + 1);
	while(iterIndicator != mapTXEEIndicator.end() && (iterIndicator->first / 100) == iFuncID)
	{
		if(iterIndicator->first % 100 == 3)
		{	//���������id
			tableIndID[2].AddIndicatorColumn((UINT)(iterIndicator->second.nIndicatorID), Tx::Core::dtype_int4, array_ParamCol, 1);
		}else
		{
			tableIndID[2].AddIndicatorColumn((UINT)(iterIndicator->second.nIndicatorID), Tx::Core::dtype_val_string, array_ParamCol, 1);
		}
		iterIndicator ++ ;
	}
	//ȡ����Ҫ����ָ�����������
	if ( bDownAll )
		bResult = pBusiness->m_pLogicalBusiness->GetData(tableIndID[2], true);
	else
		bResult = FALSE;
	//if(!bResult)
	//{
	//	tableIndID[2].Clear();
	//}
	//m_pArrayStatistics->MakeProgress();

	//�ʲ���ծ��
	tableIndID[3].AddParameterColumn(Tx::Core::dtype_int8);			//��ƺ���ID
	iFuncID = 41;	//�ʲ���ծ����Ŀ
	iterIndicator = mapTXEEIndicator.find(iFuncID * 100 + 1);
	while(iterIndicator != mapTXEEIndicator.end() && (iterIndicator->first / 100) == iFuncID)
	{
		tableIndID[3].AddIndicatorColumn((UINT)(iterIndicator->second.nIndicatorID), Tx::Core::dtype_decimal, array_ParamCol, 1);
		iterIndicator ++ ;
	}
	//ȡ���ʲ���ծ����������
	if ( bDownAll )
		bResult = pBusiness->m_pLogicalBusiness->GetData(tableIndID[3], true);
	else
		bResult = FALSE;
	//if(!bResult)
	//{
	//	tableIndID[3].Clear();
	//}
	//m_pArrayStatistics->MakeProgress();

	//��������
	tableIndID[4].AddParameterColumn(Tx::Core::dtype_int8);			//��ƺ���ID
	iFuncID = 42;	//����������Ŀ
	iterIndicator = mapTXEEIndicator.find(iFuncID * 100 + 1);
	while(iterIndicator != mapTXEEIndicator.end() && (iterIndicator->first / 100) == iFuncID)
	{
		tableIndID[4].AddIndicatorColumn((UINT)(iterIndicator->second.nIndicatorID), Tx::Core::dtype_decimal, array_ParamCol, 1);
		iterIndicator ++ ;
	}
	//ȡ������������������
	if ( bDownAll )
		bResult = pBusiness->m_pLogicalBusiness->GetData(tableIndID[4], true);
	else
		bResult = FALSE;
	//if(!bResult)
	//{
	//	tableIndID[4].Clear();
	//}
	//m_pArrayStatistics->MakeProgress();

	//�ֽ�������
	tableIndID[5].AddParameterColumn(Tx::Core::dtype_int8);			//��ƺ���ID
	iFuncID = 43;	//�ֽ���������Ŀ
	iterIndicator = mapTXEEIndicator.find(iFuncID * 100 + 1);
	while(iterIndicator != mapTXEEIndicator.end() && (iterIndicator->first / 100) == iFuncID)
	{
		tableIndID[5].AddIndicatorColumn((UINT)(iterIndicator->second.nIndicatorID), Tx::Core::dtype_decimal, array_ParamCol, 1);
		iterIndicator ++ ;
	}
	//ȡ���ֽ���������������
	if ( bDownAll )
		bResult = pBusiness->m_pLogicalBusiness->GetData(tableIndID[5], true);
	else
		bResult = FALSE;
	//if(!bResult)
	//{
	//	tableIndID[5].Clear();
	//}
	//m_pArrayStatistics->MakeProgress();

	//�ɱ��ṹ
	tableIndID[6].AddParameterColumn(Tx::Core::dtype_int4);			//����ID
	tableIndID[6].AddParameterColumn(Tx::Core::dtype_int4);			//��������
	tableIndID[6].AddIndicatorColumn(30300001, Tx::Core::dtype_double, array_ParamCol, 2);				//����ͨ��
	tableIndID[6].AddIndicatorColumn(30300003, Tx::Core::dtype_double, array_ParamCol, 2);				//���й�
	//ȡ�ùɱ��ṹ��������
	if ( bDownAll )
		bResult = pBusiness->m_pLogicalBusiness->GetData(tableIndID[6], true);
	else
		bResult = FALSE;
	//if(!bResult)
	//{
	//	tableIndID[6].Clear();
	//}
//	m_pArrayStatistics->MakeProgress();

	//�ֹ�����
	tableIndID[7].AddParameterColumn(Tx::Core::dtype_int4);			//����ID
	tableIndID[7].AddParameterColumn(Tx::Core::dtype_int4);			//��ֹ����
	tableIndID[7].AddIndicatorColumn(30300185, Tx::Core::dtype_decimal, array_ParamCol, 2);				//�ɶ�����
	//ȡ�ùɶ�������������
	if ( bDownAll )
		bResult = pBusiness->m_pLogicalBusiness->GetData(tableIndID[7], true);
	else
		bResult = FALSE;
	//if(!bResult)
	//{
	//	tableIndID[7].Clear();
	//}
	//m_pArrayStatistics->MakeProgress();

	//��Ӫҵ������ֲ�
	tableIndID[8].AddParameterColumn(Tx::Core::dtype_int8);			//��ƺ���ID
	tableIndID[8].AddParameterColumn(Tx::Core::dtype_byte);			//���
	iFuncID = 44;	//��Ӫҵ������ֲ�������
	iterIndicator = mapTXEEIndicator.find(iFuncID * 100 + 1);
	while(iterIndicator != mapTXEEIndicator.end() && (iterIndicator->first / 100) == iFuncID)
	{
		tableIndID[8].AddIndicatorColumn((UINT)(iterIndicator->second.nIndicatorID), Tx::Core::dtype_decimal, array_ParamCol, 2);
		iterIndicator ++ ;
	}
	iFuncID = 45;	//��Ӫҵ������ֲ���Ŀ����
	iterIndicator = mapTXEEIndicator.find(iFuncID * 100);
	if(iterIndicator != mapTXEEIndicator.end())
	{
		tableIndID[8].AddIndicatorColumn((UINT)(iterIndicator->second.nIndicatorID), Tx::Core::dtype_val_string, array_ParamCol, 2);
	}
	//ȡ����Ӫҵ������ֲ���������
	if ( bDownAll )
		bResult = pBusiness->m_pLogicalBusiness->GetData(tableIndID[8], true);
	else
		bResult = FALSE;
	//if(!bResult)
	//{
	//	tableIndID[8].Clear();
	//}
	//m_pArrayStatistics->MakeProgress();

	//�ʲ���ֵ׼����
	tableIndID[9].AddParameterColumn(Tx::Core::dtype_int8);			//��ƺ���ID
	tableIndID[9].AddParameterColumn(Tx::Core::dtype_byte);			//���
	iFuncID = 46;	//��Ӫҵ������ֲ�������
	iterIndicator = mapTXEEIndicator.find(iFuncID * 100 + 1);
	while(iterIndicator != mapTXEEIndicator.end() && (iterIndicator->first / 100) == iFuncID)
	{
		tableIndID[9].AddIndicatorColumn((UINT)(iterIndicator->second.nIndicatorID), Tx::Core::dtype_decimal, array_ParamCol, 2);
		iterIndicator ++ ;
	}
	//ȡ���ʲ���ֵ׼������������
	if ( bDownAll )
		bResult = pBusiness->m_pLogicalBusiness->GetData(tableIndID[9], true);
	else
		bResult = FALSE;
	//if(!bResult)
	//{
	//	tableIndID[9].Clear();
	//}
	//m_pArrayStatistics->MakeProgress();

	//�Ǿ����������
	tableIndID[10].AddParameterColumn(Tx::Core::dtype_int8);			//��ƺ���ID
	tableIndID[10].AddParameterColumn(Tx::Core::dtype_byte);			//���
	iFuncID = 47;	//�Ǿ������������Ŀ����
	iterIndicator = mapTXEEIndicator.find(iFuncID * 100);
	if(iterIndicator != mapTXEEIndicator.end())
	{
		tableIndID[10].AddIndicatorColumn((UINT)(iterIndicator->second.nIndicatorID), Tx::Core::dtype_val_string, array_ParamCol, 2);
	}
	iFuncID = 48;	//�Ǿ������������ϸ��Ŀ���
	iterIndicator = mapTXEEIndicator.find(iFuncID * 100);
	if(iterIndicator != mapTXEEIndicator.end())
	{
		tableIndID[10].AddIndicatorColumn((UINT)(iterIndicator->second.nIndicatorID), Tx::Core::dtype_decimal, array_ParamCol, 2);
	}
	//ȡ�÷Ǿ������������������
	if ( bDownAll )
		bResult = pBusiness->m_pLogicalBusiness->GetData(tableIndID[10], true);
	else
		bResult = FALSE;
	//if(!bResult)
	//{
	//	tableIndID[10].Clear();
	//}
	//m_pArrayStatistics->MakeProgress();

	//Ӧ���ʿ��
	tableIndID[11].AddParameterColumn(Tx::Core::dtype_int8);			//��ƺ���ID
	tableIndID[11].AddParameterColumn(Tx::Core::dtype_byte);			//���
	iFuncID = 49;	//Ӧ���ʿ������
	iterIndicator = mapTXEEIndicator.find(iFuncID * 100);
	if(iterIndicator != mapTXEEIndicator.end())
	{
		tableIndID[11].AddIndicatorColumn((UINT)(iterIndicator->second.nIndicatorID), Tx::Core::dtype_val_string, array_ParamCol, 2);
	}
	iFuncID = 50;	//Ӧ���ʿ������
	iterIndicator = mapTXEEIndicator.find(iFuncID * 100 + 1);
	while(iterIndicator != mapTXEEIndicator.end() && (iterIndicator->first / 100) == iFuncID)
	{
		tableIndID[11].AddIndicatorColumn((UINT)(iterIndicator->second.nIndicatorID), Tx::Core::dtype_decimal, array_ParamCol, 2);
		iterIndicator ++ ;
	}
	//ȡ��Ӧ���ʿ����������
	if ( bDownAll )
		bResult = pBusiness->m_pLogicalBusiness->GetData(tableIndID[11], true);
	else
		bResult = FALSE;
	//if(!bResult)
	//{
	//	tableIndID[11].Clear();
	//}
	//m_pArrayStatistics->MakeProgress();

	//������ñ�
	tableIndID[12].AddParameterColumn(Tx::Core::dtype_int8);			//��ƺ���ID
	tableIndID[12].AddParameterColumn(Tx::Core::dtype_byte);			//���
	iFuncID = 51;	//������ñ���Ŀ����
	iterIndicator = mapTXEEIndicator.find(iFuncID * 100);
	if(iterIndicator != mapTXEEIndicator.end())
	{
		tableIndID[12].AddIndicatorColumn((UINT)(iterIndicator->second.nIndicatorID), Tx::Core::dtype_val_string, array_ParamCol, 2);
	}
	iFuncID = 52;	//������ñ���Ŀ���
	iterIndicator = mapTXEEIndicator.find(iFuncID * 100);
	if(iterIndicator != mapTXEEIndicator.end())
	{
		tableIndID[12].AddIndicatorColumn((UINT)(iterIndicator->second.nIndicatorID), Tx::Core::dtype_decimal, array_ParamCol, 2);
	}
	//ȡ�ò�����ñ���������
	if ( bDownAll )
		bResult = pBusiness->m_pLogicalBusiness->GetData(tableIndID[12], true);
	else
		bResult = FALSE;
	m_tableFundManager.AddParameterColumn(Tx::Core::dtype_int4);		//������	����ID
	m_tableFundManager.AddParameterColumn(Tx::Core::dtype_int4);		//������	��¶����
	m_tableFundManager.AddParameterColumn(Tx::Core::dtype_val_string);	//������	����������
	m_tableFundManager.AddIndicatorColumn(30601009, Tx::Core::dtype_val_string,array_ParamCol, 3);	//ָ����	ְλ
	m_tableFundManager.AddIndicatorColumn(30601010, Tx::Core::dtype_int4,array_ParamCol, 3);		//ָ����	��ʼ����
	m_tableFundManager.AddIndicatorColumn(30601011, Tx::Core::dtype_int4,array_ParamCol, 3);		//ָ����	��������
	m_tableFundManager.AddIndicatorColumn(30601012, Tx::Core::dtype_val_string,array_ParamCol, 3);	//ָ����	�Ա�
	m_tableFundManager.AddIndicatorColumn(30601013, Tx::Core::dtype_val_string,array_ParamCol, 3);	//ָ����	ѧ��
	m_tableFundManager.AddIndicatorColumn(30601014, Tx::Core::dtype_int4,array_ParamCol, 3);		//ָ����	����
	m_tableFundManager.AddIndicatorColumn(30601015, Tx::Core::dtype_val_string,array_ParamCol, 3);	//ָ����	����
	//-----------wangzhy-------20080821----

	//ȡ�ù�Ʊ��ƺ͹�Ʊ�����Ӧ��
	pSecurityTable = pBusiness->m_pLogicalBusiness->GetSecurityTable();
	if(pSecurityTable == NULL)
	{
		return;
	}
	CString str = _T(""),str1 = _T("");
	for(i = 0; i < pSecurityTable->GetRowCount(); i++)
	{
		pSecurityTable->GetCell(0, i, iSecurityID);
		if(iSecurityID > 0)
		{
			if(business.GetSecurityNow((LONG)iSecurityID))
			{
				if(business.m_pSecurity->ShouldHidden())
				{
					continue;
				}
				//Mantis:14401  2012-01-21  wangzf
				str = business.m_pSecurity->GetName(false);
				str1 = business.m_pSecurity->GetName(true);
				if (str == str1)
				{
					mmapNameCode.insert(StrPair(str, business.m_pSecurity->GetCode(true)));
				}
				else
				{
					mmapNameCode.insert(StrPair(str, business.m_pSecurity->GetCode(true)));
					mmapNameCode.insert(StrPair(str1, business.m_pSecurity->GetCode(true)));
				}
				//mmapNameCode.insert(StrPair(business.m_pSecurity->GetName(false), business.m_pSecurity->GetCode(true)));
				mapCodeID.insert(STRLONG_Pair(business.m_pSecurity->GetCode(true), (LONG)iSecurityID));
			}
		}
	}
}
//--------����Ϊȡ��������------------wangzhy--------------------
//�������Ϊ����������������Map�����Ϊ�ŵ���������¸���ʱ�䣬���ؽ��
CString TxIndicator::GetIndicator(LPCTSTR lpszRequest,Tx::Core::VariantData& vdReq)
{
	StartHisCalcThread();
	ReqMapIter iterReq;
	ReqItem itemReq;
	CString strResult;
	//strResult = _T("-");
	CString sMsg;
	strResult = _T("���ݴ�����...");
	CString strReq(lpszRequest);
	CString sLog;
	sLog.Format(_T("Req:%s"),strReq);
	//Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
	std::unordered_map<CString,int>::iterator iterRsLimit;
	CSingleLock singleLock(&_critsect);
	if ( m_bSynFlag )
	{
		strResult = HisIndCalcProcSyn(lpszRequest,vdReq);
		if(strResult.GetLength()>0)
		{
			ReqItem ri;
			ri.strValue = strResult;
			ri.vdReq = vdReq;
			mapRequest.insert(ReqPair(lpszRequest,ri));
			//CString sMsg;
			//sMsg.Format(_T("��ʷָ�꣬m_bSynFlag������ID��%s,���ؽ��%s"),strReq.Left(3),strResult);
			//Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sMsg);
		}
	}
	else
	{
		itemReq.nCalculateTime = 0;
		itemReq.nReqTime = 0;
		itemReq.strValue = strResult;
		itemReq.vdReq = Tx::Core::VariantData(_T(""));
		singleLock.Lock();	//�̻߳���

		//2012-10-23   mantis13391
		if (144==atoi(strReq.Left(3)) || 145==atoi(strReq.Left(3)) || 146==atoi(strReq.Left(3)) || 147==atoi(strReq.Left(3)))
		{
			iterReq = mapRequest.find(strReq);
			iterRsLimit = mapRequest_limit.find(strReq);
			if(iterReq == mapRequest.end() && iterRsLimit != mapRequest_limit.end())
				mapRequest_limit.erase(iterRsLimit);
			if (iterReq != mapRequest.end() &&  iterReq->second.strValue == _T("-") && iterRsLimit != mapRequest_limit.end() && iterRsLimit->second < 20)
			{
				mapRequest.erase(iterReq);
				iterRsLimit->second += 1;
			}
		}

		iterReq = mapRequest.find(strReq);
		if(iterReq == mapRequest.end())
		{	
			if (144==atoi(strReq.Left(3)) || 145==atoi(strReq.Left(3)) || 146==atoi(strReq.Left(3)) || 147==atoi(strReq.Left(3)))
			{
				iterRsLimit = mapRequest_limit.find(strReq);
				if(iterRsLimit == mapRequest_limit.end())
					mapRequest_limit.insert(std::pair<CString,int>(strReq,1));
			}
			//VBA����Ϊ������
			mapRequest.insert(ReqPair(strReq, itemReq));
			if(!bsetAIsBusy)
			{	//setA���ڿ���״̬
				setReqA.insert(strReq);
#if _DEBUG
				sMsg.Format(_T("����%s������Ϣ����A"),strReq);
				Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sMsg);
#endif
			}else
			{	//setA����æµ״̬
				setReqB.insert(strReq);
#if _DEBUG
				sMsg.Format(_T("����%s������Ϣ����B"),strReq);
				Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sMsg);
#endif
			}
		}else
		{	//VBA�����ѻ�����map��
			iterReq->second.nReqTime = (CTime::GetCurrentTime()).GetTime();
			strResult = iterReq->second.strValue;
			vdReq = iterReq->second.vdReq;
		//	sMsg.Format(_T("��ʷָ�꣬!m_bSynFlag������ID��%s,���ؽ��%s"),strReq.Left(3),strResult);
		//	Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sMsg);
		}
		singleLock.Unlock();
	}
	return strResult;
}
//2008-06-16
//ͬ��������ʷָ��
CString TxIndicator::GetIndicatorSyn(LPCTSTR lpszRequest,Tx::Core::VariantData& vdReq)
{  
	ReqMapIter iterReq;
	ReqItem itemReq;
	//CString strResult(_T("-"));
	CString strResult(_T("���ݴ�����..."));
	CString strReq(lpszRequest);
	std::unordered_map<CString,int>::iterator iterRsLimit;
	CSingleLock singleLock(&_critsect);
	
	itemReq.nCalculateTime = 0;
	itemReq.nReqTime = 0;
	itemReq.strValue = strResult;
	itemReq.vdReq = Tx::Core::VariantData(_T(""));

	singleLock.Lock();	//�̻߳���

	//2012-10-23   mantis13391
	if (144==atoi(strReq.Left(3)) || 145==atoi(strReq.Left(3)) || 146==atoi(strReq.Left(3)) || 147==atoi(strReq.Left(3)))
	{
		iterReq = mapRequest.find(strReq);
		iterRsLimit = mapRequest_limit.find(strReq);
		if(iterReq == mapRequest.end() && iterRsLimit != mapRequest_limit.end())
			mapRequest_limit.erase(iterRsLimit);
		if (iterReq != mapRequest.end() &&  iterReq->second.strValue == _T("-") && iterRsLimit != mapRequest_limit.end() && iterRsLimit->second < 20)
		{
			mapRequest.erase(iterReq);
			iterRsLimit->second += 1;
		}
	}

	iterReq = mapRequest.find(strReq);
	if(iterReq == mapRequest.end())
	{
		if (144==atoi(strReq.Left(3)) || 145==atoi(strReq.Left(3)) || 146==atoi(strReq.Left(3)) || 147==atoi(strReq.Left(3)))
		{
			iterRsLimit = mapRequest_limit.find(strReq);
			if(iterRsLimit == mapRequest_limit.end())
				mapRequest_limit.insert(std::pair<CString,int>(strReq,1));
		}
		//VBA����Ϊ������
		strResult = HisIndCalcProcSyn(lpszRequest,vdReq);
		if(strResult.GetLength()>0)
		{
			ReqItem ri;
			ri.strValue = strResult;
			ri.vdReq = vdReq;
			mapRequest.insert(ReqPair(lpszRequest,ri));
		}
		else
			strResult = _T("-");
	}else
	{	//VBA�����ѻ�����map��
		iterReq->second.nReqTime = (CTime::GetCurrentTime()).GetTime();
		strResult = iterReq->second.strValue;
		vdReq = iterReq->second.vdReq;
	}
	singleLock.Unlock();
	return strResult;
}
CString TxIndicator::GetRealTimeIndicator(LPCTSTR lpszRequest,Tx::Core::VariantData& vdReq)
{
	StartRealCalcThread();

	ReqMapIter iterReqRealTime;
	ReqItem itemReqRealTime;
	CString strReq(lpszRequest);
	CString strResult(_T("-"));
	std::unordered_map<CString,int>::iterator iterRsLimit;
	CSingleLock singleLock(&_critsect_RealTime);

	itemReqRealTime.nCalculateTime = 0;
	itemReqRealTime.nReqTime = 0;
	itemReqRealTime.strValue = strResult;
	itemReqRealTime.vdReq = Tx::Core::VariantData(_T(""));

	singleLock.Lock();	//�̻߳���

	//2012-10-23   mantis13391
	if (144==atoi(strReq.Left(3)) || 145==atoi(strReq.Left(3)) || 146==atoi(strReq.Left(3)) || 147==atoi(strReq.Left(3)))
	{
		iterReqRealTime = mapReqRealTime.find(strReq);
		iterRsLimit = mapRequest_limit.find(strReq);
		if(iterReqRealTime == mapReqRealTime.end() && iterRsLimit != mapRequest_limit.end())
			mapRequest_limit.erase(iterRsLimit);
		if (iterReqRealTime != mapReqRealTime.end() &&  iterReqRealTime->second.strValue == _T("-") && iterRsLimit != mapRequest_limit.end() && iterRsLimit->second < 20)
		{
			mapReqRealTime.erase(iterReqRealTime);
			iterRsLimit->second += 1;
		}
	}
	iterReqRealTime = mapReqRealTime.find(strReq);
	if(iterReqRealTime == mapReqRealTime.end())
	{
		if (144==atoi(strReq.Left(3)) || 145==atoi(strReq.Left(3)) || 146==atoi(strReq.Left(3)) || 147==atoi(strReq.Left(3)))
		{
			iterRsLimit = mapRequest_limit.find(strReq);
			if(iterRsLimit == mapRequest_limit.end())
				mapRequest_limit.insert(std::pair<CString,int>(strReq,1));
		}
		//VBA����Ϊ������
		mapReqRealTime.insert(ReqPair(strReq, itemReqRealTime));
	}else
	{	//VBA�����ѻ�����map��
		iterReqRealTime->second.nReqTime = (CTime::GetCurrentTime()).GetTime();
		strResult = iterReqRealTime->second.strValue;
		vdReq = iterReqRealTime->second.vdReq;
	}
	singleLock.Unlock();

	return strResult;
}
CString TxIndicator::GetRefreshTime(void)
{
	CString strResult;
	CSingleLock singleLock(&_critsect);
	// TODO: �ڴ���ӵ��ȴ���������
	singleLock.Lock();	//�̻߳���
	strResult.Format(_T("%ld"), m_nRefreshTime);
	singleLock.Unlock();
	
	return strResult;
}
void TxIndicator::SendShCalcMsg(void)
{
	// TODO: �ڴ���ӵ��ȴ���������
	CSingleLock singleLock(&_critsect);

	singleLock.Lock();	//�̻߳���
	m_nShCalcEvtCnt ++;
	if(!m_bAnyShCalcEvt)
	{
		m_bAnyShCalcEvt = TRUE;
	}
	if(m_bReqMapCleared)
	{
		m_bReqMapCleared = FALSE;
	}
	singleLock.Unlock();

	//Tx::Log::CLogRecorder::GetInstance()->WriteToLog(_T("Excel������������ź�"));
	//2008-06-13
	::SetEvent(m_hEvent_ever);
}
CString TxIndicator::GetRefreshTime_RealTime(void)
{
	CString strResult;
	CSingleLock singleLock(&_critsect_RealTime);
	// TODO: �ڴ���ӵ��ȴ���������
	singleLock.Lock();	//�̻߳���
	strResult.Format(_T("%ld"), m_nRefreshTime_RealTime);
	singleLock.Unlock();

	return strResult;
}

/// <summary>
///  ����ָ�����뷵��ָ�������롣
/// </summary>
LONG TxIndicator::GetIndexSecurityID(LPCTSTR lpszSecurity)
{
	if(lpszSecurity == NULL || *lpszSecurity == 0)
		return -1;

	// ���þɽӿڼ��� mapCodeID
	if(mapCodeID.size()== 0)
		GetSecurityID(_T(""),0);

	// �������ַ������������ƣ���ֹ����������������ַ�������
	// ������׺����˳������ƿ��ܻ��Ƴ�ȫ���򲿷ֺ�׺�ַ���
	size_t safeLength = ::_tcsnlen(lpszSecurity,MAX_PATH);
	if(safeLength == 0)
		return -2;

	// mapCodeID ʹ�� CString ���͵� Key
	CString strCode(lpszSecurity,safeLength);	

	// �Ƴ��հ��ַ�
	strCode.Trim();
	if(strCode.IsEmpty())
		return -1;

	strCode.MakeUpper();

	// ���Ҵ����׺����
	TCHAR *pszSuffix = ::PathFindExtension(strCode);	

	//
	// ������ת�����������͡�
	TCHAR *pszStopString = NULL;
	LONG lNumbericCode = ::_tcstol(strCode,&pszStopString,10);
		
	//
	// Ŀǰ����ָ������֧�ֵķ�Χ�Ǵ� 000001 �� 999999 ���������
	// ������ָ�����벻�ڴ˷�Χ���������ַ����ָ�����봦��
	//
	if(pszSuffix != pszStopString || lNumbericCode < 1 || lNumbericCode > 999999)
		lNumbericCode = LONG_MIN;

	// û�к�׺�Ĵ���Ĭ�ϴ����з����в���	
	LONG lFmtMask = (*pszSuffix == 0) ? ULONG_MAX : 0;
	CString searchKey = strCode;

	//
	// �����Զ���ָ������Ͱ����������������Ƶ��Ͻ���ָ�����롣
	while(pszSuffix == pszStopString && lNumbericCode > 0 && lNumbericCode < 1000000)
	{
		// �Զ���ָ����
		if(lNumbericCode > 998600 && lNumbericCode < 999000)
		{
			// �Զ���ָ�����룬ֱ�ӷ������롣
			if(*pszSuffix == 0 || ::_tcsnccmp(pszSuffix,_T(".TX"),::_tcslen(_T(".TX"))) == 0)
				return lNumbericCode + 4000000;

			break;
		}

		// �Ͻ���������ָ��������������ת��Ϊ�Ͻ�������
		if(lNumbericCode / 10000 == 98)
		{
			// ������׺�İ����������������Ƶ��Ͻ���ָ�����롣
			if(*pszSuffix == 0)
			{
				// ʹ�� .SH ����
				lNumbericCode -= 980000;
				lFmtMask = 1 << 1;
				break;
			}

			// ����׺�İ����������������Ƶ��Ͻ���ָ�����롣
			if(::_tcsnccmp(pszSuffix,_T(".TX"),::_tcslen(_T(".TX"))) == 0)
			{
				// ʹ�� .SH ����
				searchKey.Format(_T("%06ld.SH"),lNumbericCode - 980000);	
				break;
			}
		}

		break;
	}

	// �����ж�ȯ����
	Tx::Business::TxBusiness txBusiness;

	// ��������
	auto findSecurityID = [this,&txBusiness](const CString& findKey) -> LONG 
	{
		auto iter = mapCodeID.find(findKey);
		if(iter == mapCodeID.end())
			return -1;

		// ����Ƿ�Ϊָ��
		auto pSecurity = txBusiness.GetSecurityNow(iter->second);
		if(pSecurity == NULL || !pSecurity->IsIndex())
			return -1;

		return iter->second;
	};

	//
	// �������롣
	// �ҵ���Ӧ�������Ҫ����Ƿ�Ϊָ�������������������ҡ�
	switch(*pszSuffix)
	{
	case _T('.'):
	default:
		{			
			// ��������к�׺����ֱ��ʹ�� searchKey �������롣
			LONG securityID = findSecurityID(searchKey);
			return securityID;
		}
	case 0:
		{
			//
			// ����ָ������û�к�׺����ת���ɱ�׼�Ĵ�����ʽ��������롣
			//
			switch(lNumbericCode)
			{			
			case LONG_MIN:
				{					
					//
					// ���������ַ����������
					static const TCHAR * _S_fmtStrings[] = {
						_T("%s.SZ"),
						_T("%s.SH"),
						_T("%s.TX"),
						_T("%s.HK"),
						_T("%s.ZX"),
						_T("%s.SW"),
						_T("%s.ZB"),
						_T("%s.JC"),
						_T("%s.XF"),
						_T("%s.NASDAQ"),
						_T("%s.ZZ"),
						_T("%s.JZ")
					};

					for(int i = 0;i < ARRAYSIZE(_S_fmtStrings);i++)
					{
						if((lFmtMask & (1 << i)) == (1 << i))
						{					
							searchKey.Format(_S_fmtStrings[i],strCode);

							LONG securityID = findSecurityID(searchKey);
							if(securityID > 0)
								return securityID;
						}
					}

					return -1;
				}
			default:
				{
					//
					// �������ִ������
					static const TCHAR * _S_fmtStrings[] = {
						_T("%06ld.SZ"),
						_T("%06ld.SH"),
						_T("%06ld.TX"),
						_T("%05ld.HK"),
						_T("%06ld.ZX"),
						_T("%06ld.SW"),
						_T("%06ld.ZB"),
						_T("%06ld.JC"),
						_T("%06ld.XF"),
						_T("%06ld.NASDAQ"),
						_T("%06ld.ZZ"),
						_T("%06ld.JZ")
					};	

					for(int i = 0;i < ARRAYSIZE(_S_fmtStrings);i++)
					{
						if((lFmtMask & (1 << i)) == (1 << i))
						{					
							searchKey.Format(_S_fmtStrings[i],lNumbericCode);

							LONG securityID = findSecurityID(searchKey);
							if(securityID > 0)
								return securityID;
						}
					}

					return -1;
				}
			}
		}
	}
}

//������ֲ
LONG TxIndicator::GetSecurityID(LPCTSTR lpszSecurity, int nType)
{
	/*
	��ע��
	�������ܣ�������Codeת��Ϊ����ʵ��ID��֧�־�����ϵͳ(��98��ͷ����ָ֤����999��995��ͷ�Ĵ��롢�Զ���ָ������)
	������������˵����
	->mapCodeID : map<Code,ID> ��CString->CString ,���ȫ����code->id

	->�������: - ����Code - CString����(LPCTSTR lpszSecurity)
	->Ϊ�����ж�һЩ����:����Code������CStringת��Ϊint
	->�ж�����...
	->�ٴ�map�в���Code��Ӧ��ID������ǰ�轫Codeת��ΪCString����
	->�� return ID; else return -1;

	�޸ģ�
	MANTIS(14503)�������������Code�г�������ĸ'M',�����������������CStringתint�Ĺ�����ĸ�ᶪ�����Ҳ���/
	��ȷ��ID,/�޸�->������ִ���ĸ'M'�Ĵ�����������ô���תΪint����,������������ĸ���ᶪʧ������������䡣 2013-01-29
	�޸ģ�
	MANTIS(14550)��excel���棬֧�����������ĸ������   2013-03-18
	*/
	if ( mapCodeID.size()== 0 )
	{
		//ȡ�ù�Ʊ��ƺ͹�Ʊ�����Ӧ��
		Tx::Core::Table_Indicator *pSecurityTable = NULL;
		Tx::Business::TxBusiness business;

		pSecurityTable = business.m_pLogicalBusiness->GetSecurityTable();
		if(pSecurityTable == NULL)
		{
			return -1;
		}
		int iSecurityID = 0;
		CString str = _T(""),str1 = _T("");
		for(int i = 0; i < (int)pSecurityTable->GetRowCount(); i++)
		{
			pSecurityTable->GetCell(0, i, iSecurityID);
			if(iSecurityID > 0)
			{
				if(business.GetSecurityNow((LONG)iSecurityID))
				{
					if(business.m_pSecurity->ShouldHidden())
					{
						continue;
					}
					//Mantis:14401  2012-01-21  wangzf  GetName(false)->GetName(true)
					str = business.m_pSecurity->GetName(false);
					str1 = business.m_pSecurity->GetName(true);
					if (str == str1)
					{
						mmapNameCode.insert(StrPair(str, business.m_pSecurity->GetCode(true)));
					}
					else
					{
						mmapNameCode.insert(StrPair(str, business.m_pSecurity->GetCode(true)));
						mmapNameCode.insert(StrPair(str1, business.m_pSecurity->GetCode(true)));
					}
					//mmapNameCode.insert(StrPair(business.m_pSecurity->GetName(true), business.m_pSecurity->GetCode(true)));
					mapCodeID.insert(STRLONG_Pair(business.m_pSecurity->GetCode(true), (LONG)iSecurityID));
				}
			}
		}
	}
	//Ŀǰ�����׺
	    //1	    ���	SZ
		//2	    �Ͻ���	SH
		//3	    ���м�	YH
		//4	    �н���	ZJ
		//5	    �۽���	HK
		//6	    ���Ż����г�����	OF
		//7	    ����	TX
		//8	    ����	SW
		//9	    ����	ZX
		//10	��֤	ZZ
		//11	�¸�	XF
		//12	�����ծ�Ǽǽ����������ι�˾	ZD
		//13	NASDAQ	NASDAQ
		//14	�޳�	JC
		//15	�����ϵͳ	
		//16	�б�	ZB
	    //17    ��׼    JZ    ���̾�����һЩ��ƴ���׼�Ĵ���
		//99	δ֪	NULL
	CString strSecurity(lpszSecurity);
	LONG nCode = 1;
	strSecurity.Trim();
	CString strFix(_T(""));
	int nIndex = strSecurity.Find( _T('.'));
	if ( nIndex != -1 && nType == 0  )
	{   
		strFix = strSecurity.Mid( nIndex+1 );
		nCode = _ttol(strSecurity.Mid( 0, nIndex));
		if( strFix.MakeLower() == _T("sz"))
			nType = 1;
		else if ( strFix.MakeLower() == _T("sh"))
			nType = 2;
		else if ( strFix.MakeLower() == _T("yh"))
			nType = 3;
		else if ( strFix.MakeLower() == _T("zj"))
			nType = 4;
		else if ( strFix.MakeLower()== _T("hk"))
			nType = 5;
		else if ( strFix.MakeLower() == _T("of"))
			nType = 6;
		else if ( strFix.MakeLower() == _T("tx"))
			nType = 7;
		else if ( strFix.MakeLower() == _T("sw"))
			nType = 8;
		else if ( strFix.MakeLower() == _T("zx"))
			nType = 9;
		else if ( strFix.MakeLower() == _T("zz"))
			nType = 10;
		else if ( strFix.MakeLower() == _T("xf"))
			nType = 11;
		else if ( strFix.MakeLower() == _T("zd"))
			nType = 12;
		else if ( strFix.MakeLower() == _T("nasdaq"))
			nType = 13;
		else if ( strFix.MakeLower() == _T("jc"))
			nType = 14;
		else if ( strFix.MakeLower() == _T("zb"))
			nType = 16;
		else if ( strFix.MakeLower() == _T("jz"))
			nType = 17;
		else 
			nType = 99;
	}
	else
		nCode = _ttol( strSecurity );

	//�ж�����Code���Ƿ�����ĸ��������ĸ��ֱ�����ַ�������
	bool bHaveChar = false;
	CString  strCharCode = _T("");
	if ( nIndex != -1)
		strCharCode = strSecurity.Left(nIndex);
	else
		strCharCode = strSecurity;

	// ���ô�ѭ��ȥ����ѭ��Ӱ��Ч�ʣ�Ŀǰ��ĸֻ�������ͷ��β����ֻҪ�ж��������Ϳ�����

	CString char1 = strCharCode.Left(1);
	CString char2 = strCharCode.Right(1);

	if(char1 > _T("9") || char2 > _T("9"))
		bHaveChar = true;

	bool IsBondYH = false;

	//�޸�: 2013-03-01  wangzf  bug:14683   //Ŀǰֻ�����м������0��ͷ���Ҵ��볬��6λ
	if (strSecurity.GetLength() > 6 && strSecurity.Left(1) == _T("0"))
	{
		IsBondYH = true;
	}
	//--wangzhy--���ﴦ�����--20080703--
	//֧��98��ͷ����֤�ļ���ָ��
	//if (nCode>=980001&&nCode<=980020)
	if (!IsBondYH)
	{
		if( nCode / 10000 == 98 )
		{
			nCode -= 980000;
			nType = 2;  
		}
	}
	
	//BUG:13815  / 2012-11-30 / wangzf
	//bool bIsSpcFundId = false, bQJJYid = false;
	//if( nCode == 999584 || nCode == 999594 || nCode == 999690 )
	//{
	//	bIsSpcFundId = true;
	//	bQJJYid = true;
	//}
	
	//����������ϵͳ�Ĵ���
	if ( nCode / 1000 == 999 || nCode / 1000 == 995) //bug:11911
	{
		int iID = Tx::Data::TypeMapManage::GetInstance()->GetIDByValueITI(TYPE_OPENFUNDID_TO_TXCODE,nCode );
		if ( iID > 0 )
			return iID;
		else
			return -1;
	}

	//�����Զ���ָ������
	if ( nCode > 998600 && nCode < 999000 )
	{
		return nCode + 4000000;
	}
	//-----------------------------------
	if(nCode > 0L || bHaveChar)
	{
		CString strCode;
		CString strTmp;
		STRLONG_Map_Iter iterCodeID;
		switch( nType )
		{
		case 0:	 //δָ���г���׺
			// by 2010-10-17 wanglm 
			//��������߲���
			if (bHaveChar)  //����������к�����ĸ����
				strCode.Format(_T("%s.SZ"),strCharCode);
			else
			    strCode.Format(_T("%06ld.SZ"), nCode);
			iterCodeID = mapCodeID.find(strCode);
			if(iterCodeID != mapCodeID.end())
				return iterCodeID->second;
			//����֤��߲���
			if (bHaveChar)
				strCode.Format(_T("%s.SH"),strCharCode);
			else
				strCode.Format(_T("%06ld.SH"), nCode);
			iterCodeID = mapCodeID.find(strCode);
			if(iterCodeID != mapCodeID.end())
				return iterCodeID->second;
			//�ڿ��Ż��������
			if (bHaveChar)
				strCode.Format(_T("%s.OF"),strCharCode);
			else
				strCode.Format(_T("%06ld.OF"), nCode);
			iterCodeID = mapCodeID.find(strCode);
			if(iterCodeID != mapCodeID.end())
			{
				int idMaster = Tx::Data::FundNV2MasterMap::GetInstance()->GetMasterId(iterCodeID->second);
				if (idMaster >= 0 && idMaster != iterCodeID->second)
					return idMaster;
				else
					return iterCodeID->second;
			}
			//�����������
			if (bHaveChar)
				strCode.Format(_T("%s.TX"),strCharCode);
			else
			    strCode.Format(_T("%06ld.TX"), nCode);
			iterCodeID = mapCodeID.find(strCode);
			if(iterCodeID != mapCodeID.end())
				return iterCodeID->second;
			//�ڸ۹������
			if (bHaveChar)
				strCode.Format(_T("%s.HK"),strCharCode);
			else
			    strCode.Format(_T("%05ld.HK"), nCode);
			iterCodeID = mapCodeID.find(strCode);
			if(iterCodeID != mapCodeID.end())
				return iterCodeID->second;
			//�����������
			if (bHaveChar)
				strCode.Format(_T("%s.ZX"),strCharCode);
			else
			    strCode.Format(_T("%06ld.ZX"), nCode);
			iterCodeID = mapCodeID.find(strCode);
			if(iterCodeID != mapCodeID.end())
				return iterCodeID->second;
			//�����м���
			//�����м���������⴦��
			if (bHaveChar)
			{
				strCode.Format(_T("%s.YH"),strCharCode);
				iterCodeID = mapCodeID.find(strCode);
				if(iterCodeID != mapCodeID.end())
					return iterCodeID->second;
			}
			else
			{
				for (int i=6;i<=11;i++)
				{
					if(i==6)
					{
						strCode.Format(_T("%06ld.YH"), nCode);
						iterCodeID = mapCodeID.find(strCode);
						if(iterCodeID != mapCodeID.end())
							return iterCodeID->second;
					}
					if ( i>6 && IsBondYH )
					{
						strTmp.Format(_T("%s%d%s"),_T("%0"),i,_T("ld.YH"));
						strCode.Format(strTmp,nCode);
						iterCodeID = mapCodeID.find(strCode);
						if(iterCodeID != mapCodeID.end())
							return iterCodeID->second;
					}
				}
			}

			//����������
			if (bHaveChar)
				strCode.Format(_T("%s.SW"),strCharCode);
			else
			    strCode.Format(_T("%06ld.SW"), nCode);
			iterCodeID = mapCodeID.find(strCode);
			if(iterCodeID != mapCodeID.end())
				return iterCodeID->second;
			//���б�����
			if (bHaveChar)
				strCode.Format(_T("%s.ZB"),strCharCode);
			else
			    strCode.Format(_T("%06ld.ZB"),nCode);
			iterCodeID = mapCodeID.find(strCode);
			if(iterCodeID != mapCodeID.end())
				return iterCodeID->second;
			//�ھ޳�����
			if (bHaveChar)
				strCode.Format(_T("%s.JC"),strCharCode);
			else
			    strCode.Format(_T("%06ld.JC"),nCode);
			iterCodeID = mapCodeID.find(strCode);
			if(iterCodeID != mapCodeID.end())
				return iterCodeID->second;
			//�н���
			if (bHaveChar)
				strCode.Format(_T("%s.ZJ"),strCharCode);
			else
			    strCode.Format(_T("%06ld.ZJ"), nCode);
			iterCodeID = mapCodeID.find(strCode);
			if(iterCodeID != mapCodeID.end())
				return iterCodeID->second;
			//�¸�
			if (bHaveChar)
				strCode.Format(_T("%s.XF"),strCharCode);
			else
			    strCode.Format(_T("%06ld.XF"),nCode);
			iterCodeID = mapCodeID.find(strCode);
			if(iterCodeID != mapCodeID.end())
				return iterCodeID->second;
			//NASDAQ
			if (bHaveChar)
				strCode.Format(_T("%s.NASDAQ"),strCharCode);
			else
			    strCode.Format(_T("%06ld.NASDAQ"),nCode);
			iterCodeID = mapCodeID.find(strCode);
			if(iterCodeID != mapCodeID.end())
				return iterCodeID->second;
			//�����ծ�Ǽǽ����������ι�˾
			if (bHaveChar)
				strCode.Format(_T("%s.ZD"),strCharCode);
			else
			    strCode.Format(_T("%06ld.ZD"),nCode);
			iterCodeID = mapCodeID.find(strCode);
			if(iterCodeID != mapCodeID.end())
				return iterCodeID->second;
			//��֤
			if (bHaveChar)
				strCode.Format(_T("%s.ZZ"),strCharCode);
			else
			    strCode.Format(_T("%06ld.ZZ"),nCode);
			iterCodeID = mapCodeID.find(strCode);
			if(iterCodeID != mapCodeID.end())
				return iterCodeID->second;

			//��׼
			if (bHaveChar)
				strCode.Format(_T("%s.JZ"),strCharCode);
			else
				strCode.Format(_T("%06ld.JZ"), nCode);
			iterCodeID = mapCodeID.find(strCode);
			if(iterCodeID != mapCodeID.end())
				return iterCodeID->second;
			break;
		case 1: 
			//���
			if (bHaveChar)
				strCode.Format(_T("%s.SZ"),strCharCode);
			else
				strCode.Format(_T("%06ld.SZ"), nCode);
			iterCodeID = mapCodeID.find(strCode);
			if(iterCodeID != mapCodeID.end())
				return iterCodeID->second;
			break;
		case 2:
			//�Ͻ���
			if (bHaveChar)
				strCode.Format(_T("%s.SH"),strCharCode);
			else
				strCode.Format(_T("%06ld.SH"), nCode);
			iterCodeID = mapCodeID.find(strCode);
			if(iterCodeID != mapCodeID.end())
				return iterCodeID->second;
			break;
		case 3:
			//���м�
			if(bHaveChar)
			{
				strCode.Format(_T("%s.YH"),strCharCode);
				iterCodeID = mapCodeID.find(strCode);
				if(iterCodeID != mapCodeID.end())
					return iterCodeID->second;
			}
			else
			{
				for (int i=6;i<=11;i++)
				{
					strTmp.Format(_T("%s%d%s"),_T("%0"),i,_T("ld.YH"));
					strCode.Format(strTmp,nCode);
					iterCodeID = mapCodeID.find(strCode);
					if(iterCodeID != mapCodeID.end())
						return iterCodeID->second;
				}
			}

			break;
		case 4:
			//�н���
			if(bHaveChar)
				strCode.Format(_T("%s.ZJ"),strCharCode);
			else
			    strCode.Format(_T("%06ld.ZJ"), nCode);
			iterCodeID = mapCodeID.find(strCode);
			if(iterCodeID != mapCodeID.end())
				return iterCodeID->second;
			break;
		case 5:
			//�۽���
			if(bHaveChar)
				strCode.Format(_T("%s.HK"),strCharCode);
			else
			    strCode.Format(_T("%05ld.HK"), nCode);
			iterCodeID = mapCodeID.find(strCode);
			if(iterCodeID != mapCodeID.end())
				return iterCodeID->second;
			break;
		case 6:
			//���Ż����г�����
			if (bHaveChar)
				strCode.Format(_T("%s.OF"),strCharCode);
			else
				strCode.Format(_T("%06ld.OF"), nCode);
			iterCodeID = mapCodeID.find(strCode);
			if(iterCodeID != mapCodeID.end())
			{
				int idMaster = Tx::Data::FundNV2MasterMap::GetInstance()->GetMasterId(iterCodeID->second);
				if (idMaster >= 0 && idMaster != iterCodeID->second)
					return idMaster;
				else
				    return iterCodeID->second;
			}
			break;
		case 7:
			//����
			if(bHaveChar)
				strCode.Format(_T("%s.TX"),strCharCode);
			else
			    strCode.Format(_T("%06ld.TX"), nCode);
			iterCodeID = mapCodeID.find(strCode);
			if(iterCodeID != mapCodeID.end())
				return iterCodeID->second;
			break;
		case 8:
			//����
			if(bHaveChar)
				strCode.Format(_T("%s.SW"),strCharCode);
			else
			    strCode.Format(_T("%06ld.SW"), nCode);
			iterCodeID = mapCodeID.find(strCode);
			if(iterCodeID != mapCodeID.end())
				return iterCodeID->second;
			break;
		case 9:
			//����
			if(bHaveChar)
				strCode.Format(_T("%s.ZX"),strCharCode);
			else
			    strCode.Format(_T("%06ld.ZX"),nCode);
			iterCodeID = mapCodeID.find(strCode);
			if(iterCodeID != mapCodeID.end())
				return iterCodeID->second;
			break;
		case 10:
			//��֤
			if(bHaveChar)
				strCode.Format(_T("%s.ZZ"),strCharCode);
			else
			    strCode.Format(_T("%06ld.ZZ"),nCode);
			iterCodeID = mapCodeID.find(strCode);
			if(iterCodeID != mapCodeID.end())
				return iterCodeID->second;
			break;
		case 11:
			//�¸�
			if(bHaveChar)
				strCode.Format(_T("%s.XF"),strCharCode);
			else
			    strCode.Format(_T("%06ld.XF"),nCode);
			iterCodeID = mapCodeID.find(strCode);
			if(iterCodeID != mapCodeID.end())
				return iterCodeID->second;
			break;
		case 12:
			//�����ծ�Ǽǽ����������ι�˾
			if(bHaveChar)
				strCode.Format(_T("%s.ZD"),strCharCode);
			else
			    strCode.Format(_T("%06ld.ZD"),nCode);
			iterCodeID = mapCodeID.find(strCode);
			if(iterCodeID != mapCodeID.end())
				return iterCodeID->second;
			break;
		case 13:
			//NASDAQ
			if(bHaveChar)
				strCode.Format(_T("%s.NASDAQ"),strCharCode);
			else
			    strCode.Format(_T("%06ld.NASDAQ"),nCode);
			iterCodeID = mapCodeID.find(strCode);
			if(iterCodeID != mapCodeID.end())
				return iterCodeID->second;
			break;
		case 14:
			//�޳�
			if(bHaveChar)
				strCode.Format(_T("%s.JC"),strCharCode);
			else
			    strCode.Format(_T("%06ld.JC"),nCode);
			iterCodeID = mapCodeID.find(strCode);
			if(iterCodeID != mapCodeID.end())
				return iterCodeID->second;
			break;
		case 16:
			//�б�
			if(bHaveChar)
				strCode.Format(_T("%s.ZB"),strCharCode);
			else
			    strCode.Format(_T("%06ld.ZB"),nCode);
			iterCodeID = mapCodeID.find(strCode);
			if(iterCodeID != mapCodeID.end())
				return iterCodeID->second;
			break;
		case 17:
			//��׼ - ����д���׼��֤ȯ����
			if (bHaveChar)
				strCode.Format(_T("%s.JZ"), strCharCode);
			else
			    strCode.Format(_T("%06ld.JZ"), nCode);
			iterCodeID = mapCodeID.find(strCode);
			if(iterCodeID != mapCodeID.end())
				return iterCodeID->second;
		case 99:
		default:
			break;
		}
	}
	return -1;
}

//��ȡծȯ����ʵ��ID  2012-12-20
LONG TxIndicator::GetSecurityIDEx(LPCTSTR lpszSecurity, int nType /* = 0  */)
{
	if ( mapBondCodeID.size()== 0 )
	{
		//ȡ�ù�Ʊ��ƺ͹�Ʊ�����Ӧ��
		Tx::Core::Table_Indicator *pSecurityTable = NULL;
		Tx::Business::TxBusiness business;

		pSecurityTable = business.m_pLogicalBusiness->GetSecurityTable();
		if(pSecurityTable == NULL)
		{
			return -1;
		}
		int iSecurityID = 0;
		for(int i = 0; i < (int)pSecurityTable->GetRowCount(); i++)
		{
			pSecurityTable->GetCell(0, i, iSecurityID);
			if(iSecurityID > 0)
			{
				if(business.GetSecurityNow((LONG)iSecurityID))
				{
					if(business.m_pSecurity->ShouldHidden() || !business.m_pSecurity->IsBond())
					{
						continue;
					}
					mapBondCodeID.insert(STRLONG_Pair(business.m_pSecurity->GetCode(true), (LONG)iSecurityID));
				}
			}
		}
	}
	//Ŀǰ�����׺
	//1	    ���	SZ
	//2	    �Ͻ���	SH
	//3	    ���м�	YH
	//99	δ֪	NULL
	CString strSecurity(lpszSecurity);
	LONG nCode = 1;
	strSecurity.Trim();
	CString strFix(_T(""));
	int nIndex = strSecurity.Find( _T('.'));
	if ( nIndex != -1 && nType == 0  )
	{   
		strFix = strSecurity.Mid( nIndex+1 );
		nCode = _ttol(strSecurity.Mid( 0, nIndex));
		if( strFix.MakeLower() == _T("sz"))
			nType = 1;
		else if ( strFix.MakeLower() == _T("sh"))
			nType = 2;
		else if ( strFix.MakeLower() == _T("yh"))
			nType = 3;
		else
			nType = 99;
	}
	else
		nCode = _ttol( strSecurity );

	//����������ϵͳ�Ĵ���
	if ( nCode / 1000 == 999 || nCode / 1000 == 995) 
	{
		int iID = Tx::Data::TypeMapManage::GetInstance()->GetIDByValueITI(TYPE_OPENFUNDID_TO_TXCODE,nCode );
		if ( iID > 0 )
			return iID;
		else
			return -1;
	}

	//�ж�����Code���Ƿ�����ĸ��������ĸ��ֱ�����ַ�������
	bool bHaveChar = false;
	CString  strCharCode = _T("");
	if ( nIndex != -1)
		strCharCode = strSecurity.Left(nIndex);
	else
		strCharCode = strSecurity;

	for (int i=0;i<strCharCode.GetLength();i++)
	{
		if(_T("a") <= strCharCode.Mid(i,1) && _T("z") >= strCharCode.Mid(i,1)
			||_T("A") <= strCharCode.Mid(i,1) && _T("Z") >= strCharCode.Mid(i,1))
		{
			bHaveChar = true;
			break;
		}
	}

	//-----------------------------------
	if(nCode > 0L|| bHaveChar)
	{
		CString strCode;
		CString strTmp;
		STRLONG_Map_Iter iterCodeID;
		switch( nType )
		{
		case 0:	 //δָ���г���׺
			// by 2010-10-17 wanglm 
			//��������߲���
			if(bHaveChar)
				strCode.Format(_T("%s.SZ"), strCharCode);
			else
			    strCode.Format(_T("%06ld.SZ"), nCode);
			iterCodeID = mapBondCodeID.find(strCode);
			if(iterCodeID != mapBondCodeID.end())
				return iterCodeID->second;
			//����֤��߲���
			if(bHaveChar)
				strCode.Format(_T("%s.SH"), strCharCode);
			else
			    strCode.Format(_T("%06ld.SH"), nCode);
			iterCodeID = mapBondCodeID.find(strCode);
			if(iterCodeID != mapBondCodeID.end())
				return iterCodeID->second;
			//�����м���
			//�����м���������⴦��
			if (bHaveChar)
			{
				strCode.Format(_T("%s.YH"),strCharCode);
				iterCodeID = mapBondCodeID.find(strCode);
				if(iterCodeID != mapBondCodeID.end())
					return iterCodeID->second;
			}
			else
			{
				for (int i=6;i<=11;i++)
				{
					strTmp.Format(_T("%s%d%s"),_T("%0"),i,_T("ld.YH"));
					strCode.Format(strTmp,nCode);
					iterCodeID = mapBondCodeID.find(strCode);
					if(iterCodeID != mapBondCodeID.end())
						return iterCodeID->second;
				}
			}

			break;
		case 1: 
			//���
			if(bHaveChar)
				strCode.Format(_T("%s.SZ"), strCharCode);
			else		
			    strCode.Format(_T("%06ld.SZ"), nCode);
			iterCodeID = mapBondCodeID.find(strCode);
			if(iterCodeID != mapBondCodeID.end())
				return iterCodeID->second;
			break;
		case 2:
			//�Ͻ���
			if(bHaveChar)
				strCode.Format(_T("%s.SH"), strCharCode);
			else
			    strCode.Format(_T("%06ld.SH"), nCode);
			iterCodeID = mapBondCodeID.find(strCode);
			if(iterCodeID != mapBondCodeID.end())
				return iterCodeID->second;
			break;
		case 3:
			//���м�
			if (bHaveChar)
			{
				strCode.Format(_T("%s.YH"),strCharCode);
				iterCodeID = mapBondCodeID.find(strCode);
				if(iterCodeID != mapBondCodeID.end())
					return iterCodeID->second;
			}
			else
			{
				for (int i=6;i<=11;i++)
				{
					strTmp.Format(_T("%s%d%s"),_T("%0"),i,_T("ld.YH"));
					strCode.Format(strTmp,nCode);
					iterCodeID = mapBondCodeID.find(strCode);
					if(iterCodeID != mapBondCodeID.end())
						return iterCodeID->second;
				}
			}
			break;
		case 99:
		default:
			break;
		}
	}
	return -1;
}

//��ȡ������ʵ��ID  2012-12-25
LONG TxIndicator::GetFund_SecurityID(LPCTSTR lpszSecurity, int nType /* = 0  */)
{
	if ( mapFundCodeID.size()== 0 )
	{
		//ȡ�ù�Ʊ��ƺ͹�Ʊ�����Ӧ��
		Tx::Core::Table_Indicator *pSecurityTable = NULL;
		Tx::Business::TxBusiness business;

		pSecurityTable = business.m_pLogicalBusiness->GetSecurityTable();
		if(pSecurityTable == NULL)
		{
			return -1;
		}
		int iSecurityID = 0;
		for(int i = 0; i < (int)pSecurityTable->GetRowCount(); i++)
		{
			pSecurityTable->GetCell(0, i, iSecurityID);
			if(iSecurityID > 0)
			{
				if(business.GetSecurityNow((LONG)iSecurityID))
				{
					if(business.m_pSecurity->ShouldHidden() || !business.m_pSecurity->IsFund())
					{
						continue;
					}
					
					mapFundCodeID.insert(STRLONG_Pair(business.m_pSecurity->GetCode(true), (LONG)iSecurityID));
				}
			}
		}
	}
	//Ŀǰ�����׺
	//1	    ���	SZ
	//2	    �Ͻ���	SH
	//6	    ���Ż����г�����	OF
	//99	δ֪	NULL
	CString strSecurity(lpszSecurity);
	LONG nCode = 1;
	strSecurity.Trim();
	CString strFix(_T(""));
	int nIndex = strSecurity.Find( _T('.'));
	if ( nIndex != -1 && nType == 0  )
	{   
		strFix = strSecurity.Mid( nIndex+1 );
		nCode = _ttol(strSecurity.Mid( 0, nIndex));
		if( strFix.MakeLower() == _T("sz"))
			nType = 1;
		else if ( strFix.MakeLower() == _T("sh"))
			nType = 2;
		else if ( strFix.MakeLower() == _T("of"))
			nType = 3;
		else 
			nType = 99;
	}
	else
		nCode = _ttol( strSecurity );

	//����������ϵͳ�Ĵ���
	if ( nCode / 1000 == 999 || nCode / 1000 == 995 || nCode / 1000 == 994) // ���ӻ����������ת�г������994��֧�� modified by xujf 20141106
	{
		int iID = Tx::Data::TypeMapManage::GetInstance()->GetIDByValueITI(TYPE_OPENFUNDID_TO_TXCODE,nCode );
		if ( iID > 0 )
			return iID;
		else
			return -1;
	}

	//�ж�����Code���Ƿ�����ĸ��������ĸ��ֱ�����ַ�������
	bool bHaveChar = false;
	CString  strCharCode = _T("");
	if ( nIndex != -1)
		strCharCode = strSecurity.Left(nIndex);
	else
		strCharCode = strSecurity;

	for (int i=0;i<strCharCode.GetLength();i++)
	{
		if(_T("a") <= strCharCode.Mid(i,1) && _T("z") >= strCharCode.Mid(i,1)
			||_T("A") <= strCharCode.Mid(i,1) && _T("Z") >= strCharCode.Mid(i,1))
		{
			bHaveChar = true;
			break;
		}
	}


	if(nCode > 0L || bHaveChar)
	{
		CString strCode;
		CString strTmp;
		STRLONG_Map_Iter iterCodeID;
		switch( nType )
		{
		case 0:	 //δָ���г���׺
			//��������߲���
			if(bHaveChar)
				strCode.Format(_T("%s.SZ"), strCharCode);
			else
				strCode.Format(_T("%06ld.SZ"), nCode);
			iterCodeID = mapFundCodeID.find(strCode);
			if(iterCodeID != mapFundCodeID.end())
				return iterCodeID->second;
			//����֤��߲���
			if(bHaveChar)
				strCode.Format(_T("%s.SH"), strCharCode);
			else
				strCode.Format(_T("%06ld.SH"), nCode);
			iterCodeID = mapFundCodeID.find(strCode);
			if(iterCodeID != mapFundCodeID.end())
				return iterCodeID->second;
			//�ڿ��Ż��������
			if(bHaveChar)
				strCode.Format(_T("%s.OF"), strCharCode);
			else
				strCode.Format(_T("%06ld.OF"), nCode);
			iterCodeID = mapFundCodeID.find(strCode);
			//�޸ģ�2013-01-07 wangzf bug:14275
			if(iterCodeID != mapFundCodeID.end())
			{
				int idMaster = Tx::Data::FundNV2MasterMap::GetInstance()->GetMasterId(iterCodeID->second);
				if (idMaster >= 0 && idMaster != iterCodeID->second)
					return idMaster;
				else
					return iterCodeID->second;
			}
			break;
		case 1: 
			//���
			if(bHaveChar)
				strCode.Format(_T("%s.SZ"), strCharCode);
			else
				strCode.Format(_T("%06ld.SZ"), nCode);
			iterCodeID = mapFundCodeID.find(strCode);
			if(iterCodeID != mapFundCodeID.end())
				return iterCodeID->second;
			break;
		case 2:
			//�Ͻ���
			if(bHaveChar)
				strCode.Format(_T("%s.SH"), strCharCode);
			else
				strCode.Format(_T("%06ld.SH"), nCode);
			iterCodeID = mapFundCodeID.find(strCode);
			if(iterCodeID != mapFundCodeID.end())
				return iterCodeID->second;
			break;
		case 3:
			//���Ż����г�����
			if(bHaveChar)
				strCode.Format(_T("%s.OF"), strCharCode);
			else
				strCode.Format(_T("%06ld.OF"), nCode);
			iterCodeID = mapFundCodeID.find(strCode);
			if(iterCodeID != mapFundCodeID.end())
			{
				int idMaster = Tx::Data::FundNV2MasterMap::GetInstance()->GetMasterId(iterCodeID->second);
				if (idMaster >= 0 && idMaster != iterCodeID->second)
					return idMaster;
				else
					return iterCodeID->second;
				//{
				//	//��������߲���
				//	if(bHaveChar)
				//		strCode.Format(_T("%s.SZ"), strCharCode);
				//	else
				//		strCode.Format(_T("%06ld.SZ"), nCode);
				//	iterCodeID = mapFundCodeID.find(strCode);
				//	if(iterCodeID != mapFundCodeID.end())
				//		return iterCodeID->second;
				//	//����֤��߲���
				//	if(bHaveChar)
				//		strCode.Format(_T("%s.SH"), strCharCode);
				//	else
				//		strCode.Format(_T("%06ld.SH"), nCode);
				//	iterCodeID = mapFundCodeID.find(strCode);
				//	if(iterCodeID != mapFundCodeID.end())
				//		return iterCodeID->second;
				//	//�ڿ��Ż��������
				//	if(bHaveChar)
				//		strCode.Format(_T("%s.OF"), strCharCode);
				//	else
				//		strCode.Format(_T("%06ld.OF"), nCode);
				//	iterCodeID = mapFundCodeID.find(strCode);
				//	//�޸ģ�2013-01-07 wangzf bug:14275
				//	if(iterCodeID != mapFundCodeID.end())
				//	{
				//		int idMaster = Tx::Data::FundNV2MasterMap::GetInstance()->GetMasterId(iterCodeID->second);
				//		if (idMaster >= 0 && idMaster != iterCodeID->second)
				//			return idMaster;
				//		else
				//			return iterCodeID->second;
				//	}
				//}
			}
			break;
		case 99:
		default:
			break;
		}
	}
	return -1;
}
//--------------------ȡ����ĿID --- wanglm --------------------------
SHORT TxIndicator::GetItemID(SHORT iFuncID, LPCTSTR lpszItem)
{
	// TODO: �ڴ���ӵ��ȴ���������
	CString strItem(lpszItem);
	CString strItemTemp;
	strItemTemp = strItem;
	strItemTemp.Replace(_T("��"),_T("��"));
	strItemTemp.Replace(_T("��"),_T("("));
	strItemTemp.Replace(_T("��"),_T(")"));
	if ( strItem == _T("ʵ�ջ���") || strItem == _T("Ӧ�ջ���") )
	{
		strItemTemp.Replace(_T("Ӧ"),_T("ʵ"));
	}
	SHORT iIndex = 0;
	strItem = strItemTemp;
	INTSTR_Map_Iter IterItem = mapTXEEItem.find((INT)iFuncID * 100 + 1);
	while(IterItem != mapTXEEItem.end() && (IterItem->first / 100 == iFuncID) && (IterItem->second.Compare(strItem) != 0))
	{
		IterItem ++;
	}
	if(IterItem != mapTXEEItem.end() && (IterItem->first / 100 == iFuncID))
	{
		iIndex = (SHORT)(IterItem->first % 100);
	}
	return iIndex;
}

// ��ȡָ��ID��Ӧ����ҵID[֤���(��)]
LONG TxIndicator::GetIndicatorID2IndustryID(SHORT iFuncID, LPCTSTR lpszItem)
{
	SHORT iSubItemID = GetItemID(iFuncID,lpszItem);
	LONG nIndustryID = 0;
	INT nIndicatorID;
	// �ҵ�ָ��ID
	TXEE_Indicator_Iter iter = mapTXEEIndicator.find((INT)iFuncID * 100 + iSubItemID);
	if (iter == mapTXEEIndicator.end())
		return 0;
	else
		nIndicatorID = iter->second.nIndicatorID;
	// ��iFuncID�ҵ���Ӧmap
	map<INT,INT_Map>::iterator iterMMap = mmapIndicator2IndustryID.find((INT)iFuncID);
	if(iterMMap == mmapIndicator2IndustryID.end())
		return 0;
	// ��ָ��ID�ҵ���ҵID
	INT_Map_Iter iterItem = iterMMap->second.find((INT)nIndicatorID);

	if (iterItem != iterMMap->second.end())
	{
		nIndustryID = (LONG)iterItem->second;
	}
	return nIndustryID;
}

SHORT TxIndicator::GetReportType(LPCTSTR lpszReportType)
{
	// TODO: �ڴ���ӵ��ȴ���������
	CString strReportType(lpszReportType);
	SHORT i;
	
	for(i = 0; i < 2; i++)
	{
		if(strReportType.Compare(_T(cReportTypes[i])) == 0)
		{
			return i;
		}
	}

	return -1;
}

SHORT TxIndicator::GetAccountingPolicyType(LPCTSTR lpszAccountingPolicyType)
{
	// TODO: �ڴ���ӵ��ȴ���������
	CString strAccountingPolicyType(lpszAccountingPolicyType);
	SHORT i;
	
	for(i = 0; i < 2; i++)
	{
		if(strAccountingPolicyType.Compare(_T(cAccountingPolicyTypes[i])) == 0)
		{
			return i;
		}
	}

	return -1;
}

SHORT TxIndicator::GetIRType(LPCTSTR lpszIRType)
{
	// TODO: �ڴ���ӵ��ȴ���������
	CString strIRType(lpszIRType);
	SHORT i;
	
	for(i = 0; i < 2; i++)
	{
		if(strIRType.Compare(_T(cIRTypes[i])) == 0)
		{
			return i;
		}
	}

	return -1;
}

SHORT TxIndicator::GetPriceType(LPCTSTR lpszPriceType)
{
	// TODO: �ڴ���ӵ��ȴ���������
	CString strPriceType(lpszPriceType);
	SHORT i;
	
	for(i = 0; i < 6; i++)
	{
		if(strPriceType.Compare(_T(cPriceTypes[i])) == 0)
		{
			return i;
		}
	}

	return -1;
}

SHORT TxIndicator::GetValueType(LPCTSTR lpszValueType)
{
	// TODO: �ڴ���ӵ��ȴ���������
	CString strValueType(lpszValueType);
	SHORT i;
	
	for(i = 0; i < 8; i++)
	{
		if(strValueType.Compare(_T(cValueTypes[i])) == 0)
		{
			return i;
		}
	}

	return -1;
}

SHORT TxIndicator::GetPFADSubject(LPCTSTR lpszPFADSubject)
{
	// TODO: �ڴ���ӵ��ȴ���������
	CString strPFADSubject(lpszPFADSubject);
	SHORT i;
	
	for(i = 0; i < 4; i++)
	{
		if(strPFADSubject.Compare(_T(cPFADSubjects[i])) == 0)
		{
			return i + 1;
		}
	}

	return 0;
}
UINT pfnHisIndCalc_Proc(LPVOID lpParam)
{

	TxIndicator* pTxIndicator= (TxIndicator*)lpParam;
	if(pTxIndicator!=NULL)
	{
		pTxIndicator->HisIndCalc_Proc(NULL);
	}
	return 0;
}
UINT pfnRealTimeIndCalc_Proc(LPVOID lpParam)
{

	TxIndicator* pTxIndicator= (TxIndicator*)lpParam;
	if(pTxIndicator!=NULL)
	{
		pTxIndicator->RealTimeIndCalc_Proc(NULL);
	}
	return 0;
}

void TxIndicator::StartHisCalcThread(void)
{
	if(m_bIsHisThreadStarted==true)
		return;
	m_hEvent_ever = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	m_bRun_HisIndCalc = TRUE;
	m_pThread_HisIndCalc = AfxBeginThread(pfnHisIndCalc_Proc, this);
	m_bIsHisThreadStarted = true;
	CString sLog;
	sLog = _T("��ʷ�߳�������");
	//Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
}
void TxIndicator::StartRealCalcThread(void)
{
	if(m_bIsRealThreadStarted==true)
		return;

	m_hEvent_timer = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	m_bRun_RealTimeIndCalc = TRUE;
	m_pThread_RealTimeIndCalc = AfxBeginThread(pfnRealTimeIndCalc_Proc, this);
	m_bIsRealThreadStarted = true;
	CString sLog;
	sLog = _T("ʵʱ�߳�������");
	//Tx::Log::CLogRecorder::GetInstance()->WriteToLog(sLog);
}
void TxIndicator::EndHisCalcThread(void)
{
	if(m_bIsHisThreadStarted==false)
		return;
	m_bIsHisThreadStarted = false;
	if(m_bRun_HisIndCalc==FALSE)
		return;

	m_bRun_HisIndCalc = FALSE;			//��ֹ��ʷָ������߳�
	m_bRun_ClearReqMap = FALSE;			//��ֹVBA����map�����߳�

	//ֹͣ��ʱ��
	::SetEvent(m_hEvent_ever);
	CloseHandle(m_hEvent_ever);

	//�������Excel�������˳���ʱ��m_bTerm_HisIndCalc�����޸ģ��ͻ�������ѭ��--------------20080515-----wangzhy-------------------
	while(!m_bTerm_HisIndCalc)
	{	//�ȴ���ʷָ������߳̽���
		::Sleep(50);
	}
	//CWinThread *m_pThread_HisIndCalc, *m_pThread_RealTimeIndCalc, *m_pThread_ClearReqMap, *m_pThread_LoadData;
	WaitThreadExit(m_pThread_HisIndCalc);
}
void TxIndicator::EndRealCalcThread(void)
{
	if(m_bIsRealThreadStarted==false)
		return;
	m_bIsRealThreadStarted = false;

	if(m_bRun_RealTimeIndCalc==FALSE)
		return;

	m_bRun_RealTimeIndCalc = FALSE;		//��ֹʵʱָ������߳�
	m_bRun_ClearReqMap = FALSE;			//��ֹVBA����map�����߳�
	
	//ֹͣ��ʱ��
	::SetEvent(m_hEvent_timer);
	CloseHandle(m_hEvent_timer);

	//�������Excel�������˳���ʱ��m_bTerm_HisIndCalc�����޸ģ��ͻ�������ѭ��--------------20080515-----wangzhy-------------------
	while(!m_bTerm_RealTimeIndCalc)
	{	//�ȴ�ʵʱָ������߳̽���
		::Sleep(50);
	}
	//CWinThread *m_pThread_HisIndCalc, *m_pThread_RealTimeIndCalc, *m_pThread_ClearReqMap, *m_pThread_LoadData;
	WaitThreadExit(m_pThread_RealTimeIndCalc);
}
//void TxIndicator::ResetCalc()
//{
//	StartHisCalcThread();
//	CSingleLock singleLock(&_critsect);
//	singleLock.Lock();	//�̻߳���
//	mapRequest.clear();
//	singleLock.Unlock();
//}
//��û���ı�������1����2�����У�3����4�����걨
SHORT TxIndicator::Get_Fund_ReportType(LPCTSTR lpszReportType)
{
	// TODO: �ڴ���ӵ��ȴ���������
	CString strReportType(lpszReportType);
	SHORT i;
	
	for(i = 0; i < 6; i++)
	{
		if(strReportType.Compare(_T(cFundReportTypes[i])) == 0)
		{
			return i;
		}
	}
	return -1;
}

	}
}

