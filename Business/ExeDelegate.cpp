/**************************************************************
	Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
	File name:		ExeDelegate.cpp
	Author:			�Ժ꿡
	Date:			2008-03-26
	Version:		1.0
	Description:	ί�д���
***************************************************************/
#include "stdafx.h"
#include <io.h>
#include <direct.h>
#include "ExeDelegate.h"
#include "..\..\ui\framebase\memdc.h"
#include "..\..\ui\framebase\gdiresource.h"
#include "..\..\ui\framebase\InputWnd.h"

#include "..\..\Core\Control\Manage.h"

void TxCreateLnk(
	CString lnkPath			/*��ݷ�ʽ�Ĵ洢·��*/,
	CString srcfile			/*Դ�ļ�*/,
	CString sShortcutName	/*��ݷ�ʽ����*/,
	CString sLnkDesc		/*��ݷ�ʽ����*/
	)
{
	HRESULT hres;
	IShellLink *psl;//IShellLink�ӿ�ָ��

	//GetUserPath
	//CString lnkPath/*��ݷ�ʽ�Ĵ洢·��*/;
	//lnkPath = Tx::Core::SystemPath::GetInstance()->GetUserPath();
	//lnkPath += _T("\\Delegate");
	CString sShortcutFile;
	sShortcutFile.Format(_T("%s\\%s.lnk"),lnkPath,sShortcutName);

	if (_taccess(lnkPath, 0) != 0)
		_tmkdir(lnkPath);

	// �õ�CLSID_ShellLink��ʶ��COM�����IShellLink�ӿ�
	hres = CoCreateInstance (
		CLSID_ShellLink, 
		NULL, 
		CLSCTX_INPROC_SERVER,
		IID_IShellLink, 
		(void **)&psl);
	if (!SUCCEEDED(hres))
	{
		if(psl!=NULL)
			psl->Release ();
		return ;
	}

	// IPersistFile�ӿ�ָ��
	IPersistFile *ppf;
	
	//��ѯIPersistFile�ӿ��Խ��п�ݷ�ʽ�Ĵ洢����
	hres = psl->QueryInterface (IID_IPersistFile, (void **)&ppf);
	if (!SUCCEEDED (hres))
	{
		if(psl!=NULL)
			psl->Release ();
		if(ppf!=NULL)
			ppf->Release ();
		return ;
	}
#ifndef UNICODE
	wchar_t wsz [MAX_PATH]; // Unicode�ַ����Ļ����ַ
						    // Ϊ��ӦCOM��׼һ��Ҫ��Unicode
#endif

	// ����Դ�ļ���ַ
	hres = psl->SetPath (srcfile);
	
	if (! SUCCEEDED (hres))
	{
		if(psl!=NULL)
			psl->Release ();
		if(ppf!=NULL)
			ppf->Release ();
		return ;
	}
	
	// ���ò��� (����������ڲ���)
	// hres = psl->SetArguments("/ArgumentsHere");
	// if (! SUCCEEDED (hres)) goto error;
	
	//���ÿ�ݷ�ʽ������
	hres = psl->SetDescription (sLnkDesc);
	if (! SUCCEEDED (hres))
	{
		if(psl!=NULL)
			psl->Release ();
		if(ppf!=NULL)
			ppf->Release ();
		return ;
	}
	
	//��ANSI�ַ���ת��ΪUnicode�ַ���
	//lnkPath
	//MultiByteToWideChar (CP_ACP, 0, pszDesPath, -1, wsz, MAX_PATH);
#ifndef UNICODE
	MultiByteToWideChar (CP_ACP, 0, sShortcutFile, -1, wsz, MAX_PATH);
	hres = ppf->Save (wsz, TRUE);
#else
	//����Save�������д洢
	hres = ppf->Save (sShortcutFile, TRUE);
#endif
	if (! SUCCEEDED (hres))
	{
		if(psl!=NULL)
			psl->Release ();
		if(ppf!=NULL)
			ppf->Release ();
		return ;
	}
}

namespace Tx
{
	namespace Business
	{

// �˵����ô���
//class ExeDelegate : public DialogWithoutResource
IMPLEMENT_DYNAMIC(ExeDelegate, DialogWithoutResource)

//////////////////////////////////////////////////////////////////////////////
// �ҵĲ˵��б�
//bool ExeDelegate::UserMenuList::MoveItem(int nFrom, int nTo)
//{
//	bool bRet = DragListBox::MoveItem(nFrom, nTo);
//	if (bRet)
//	{
//		// ֪ͨ�ܿظĲ˵�
//	}
//	return bRet;
//}
//
//// ȷ��point�Ƿ��� drag ������ (virtual)
//bool ExeDelegate::UserMenuList::_PtInDrag(CPoint point, int nIndex) const
//{
//	if (GetCount() < 1 || nIndex < 0 || nIndex >= GetCount()) return false;
//
//	CRect rect;
//	if (LB_ERR == GetItemRect(nIndex, rect)) return false;
//
//	return rect.PtInRect(point) ? true : false;
//}


//////////////////////////////////////////////////////////////////////////////
// ��ʾ����
void ExeDelegate::PromptWnd::SetPromptText(const CString& text)
{
	m_strPrompt = text;
	if (::IsWindow(m_hWnd)) Invalidate();
}

LRESULT ExeDelegate::PromptWnd::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_ERASEBKGND:
		return TRUE;
	case WM_PAINT:
		{
			CPaintDC dc(this);
			CRect rect;
			GetClientRect(rect);
			Tx::UI::MemDC memdc(&dc, rect, GetSysColor(COLOR_BTNFACE));
			DrawPrompt(&memdc, rect);
		}
		return TRUE;
	default:
		break;
	}
	return CWnd::WindowProc(message, wParam, lParam);
}

void ExeDelegate::PromptWnd::DrawPrompt(CDC* pdc, const CRect& rcClient)
{
	CRect rect(rcClient);
	UINT nFormat;
	CFont* old_font = NULL;
	int old_mode = pdc->SetBkMode(TRANSPARENT);
	COLORREF old_color = pdc->GetTextColor();
	if (m_nMode == 1)
	{
		// �ҵĲ˵�
		pdc->FillSolidRect(rect, GetSysColor(COLOR_APPWORKSPACE));
		rect.DeflateRect(2, 2);
		old_font = pdc->SelectObject(GDIResource::GetInstance()->GetFont(0, 0, true));
		pdc->SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
		nFormat = DT_CENTER|DT_VCENTER|DT_SINGLELINE;
		pdc->DrawText(m_strPrompt, rect, nFormat);
	}
	else
	{
		// ��ʾ
		// ��2��������
		pdc->FillSolidRect(rect, 0xff0000);
		rect.DeflateRect(2, 2);
		// �� COLOR_INFOBK ��䱳��
		pdc->FillSolidRect(rect, GetSysColor(COLOR_INFOBK));
		// ����������������2������
		old_font = pdc->SelectObject(GDIResource::GetInstance()->GetFont(0));
		pdc->SetTextColor(0xff0000);
		
		CRect rc(rect);
		rc.DeflateRect(5, 0);
		rc.bottom = rc.top + 16;
		pdc->DrawText(_T("����ί��˵��"), rc, DT_CENTER|DT_VCENTER|DT_SINGLELINE);
		rc.OffsetRect(0, rc.Height() + 2);
		// ���: �ӹ�������ק���ҵ�XX�˵��б�
		pdc->DrawText(_T("���: ��ί�м����б�"), rc, DT_LEFT|DT_VCENTER|DT_SINGLELINE);
		CRect rc1(rc);
		rc1.right = rc.right - 30;
		rc1.left = rc1.right - 8;
		rc1.DeflateRect(0, (rc.Height() - 8) / 2);
		pdc->FillSolidRect(rc1, 0x8000);
		
		rc.OffsetRect(0, rc.Height());
		pdc->DrawText(_T("{}"), rc, DT_RIGHT|DT_VCENTER|DT_SINGLELINE);
		rc.OffsetRect(0, rc.Height() + 3);
		
		// ɾ��: ���ҵ�XX�˵��б���ק���˿���
		pdc->DrawText(_T("ɾ��: ɾ��ί��"), rc, DT_LEFT|DT_VCENTER|DT_SINGLELINE);
		rc.OffsetRect(0, rc.Height());
		pdc->DrawText(_T("{}"), rc, DT_RIGHT|DT_VCENTER|DT_SINGLELINE);
		rc.OffsetRect(0, rc.Height() + 3);

		// ˳��: �� �ҵ�XX�˵��б� ����ק
		pdc->DrawText(_T("{}"), rc, DT_LEFT|DT_VCENTER|DT_SINGLELINE);

		//TRACE("\n\n\n-------height ------- %d ------\n\n\n", rc.bottom);
	}

	pdc->SelectObject(old_font);
	pdc->SetTextColor(old_color);
	pdc->SetBkMode(old_mode);
}

// ���ýӿ�	static 
void ExeDelegate::ExeDelegateWnd(bool bExeNow ,CWnd* parent)
{
	if(bExeNow==true)
	{
		if(TxDelegate::GetInstance()->GetCount()>0)
		{
			TxDelegate::GetInstance()->Launch();
			return;
		}
	}

	ExeDelegate dlg;
	int iWidth = 500;
	dlg.ShowModal(parent, _T("����ί��"), CRect(0,0,iWidth,(int)(iWidth*0.618)));
}


ExeDelegate::ExeDelegate()
{
	//m_pUserMenu = new UserMenuList;
	//m_pUserMenu = new TxDelegateList;//CListCtrl;
	m_pUserMenu = new CListCtrl;
	
	m_pBtnAdd = new CButton;
	m_pBtnDel = new CButton;
	m_pBtnMod = new CButton;
	m_pBtnRun = new CButton;
	m_pBtnSet = new CButton;
	m_pBtnExt = new CButton;

	m_pMyMenu = new PromptWnd(1, _T("ί���б�"));
	m_pPrompt = new PromptWnd(2, _T("ί�п���ק"));

	//m_sDelegatePath = Tx::Core::SystemPath::GetInstance()->GetUserPath();
	//m_sDelegatePath += _T("\\Delegate");
	//m_sDelegateDesc = _T("����ί��");
	m_bSmallIcon = FALSE;
	m_bPressESC = false;
}

ExeDelegate::~ExeDelegate()
{
	delete m_pPrompt;
	delete m_pMyMenu;

	delete m_pBtnAdd;
	delete m_pBtnDel;
	delete m_pBtnMod;
	delete m_pBtnRun;
	delete m_pBtnSet;
	delete m_pBtnExt;

	m_hImageList.Detach();
	delete m_pUserMenu;

}

BEGIN_MESSAGE_MAP(ExeDelegate, DialogWithoutResource)
END_MESSAGE_MAP()


// ����ҵĲ˵��б�
void ExeDelegate::ResetUserMenuList()
{
	ShowDelegate();

//	m_pUserMenu->ResetContent();
	//m_pUserMenu->DeleteAllItems();
	//CString sTxDelegateFile;

	//int count = TxDelegate::GetInstance()->GetCount();

	//int iDefaultId = TxDelegate::GetInstance()->GetDefault();
	//for (int i=0;i<count;i++)
	//{
	//	sTxDelegateFile = TxDelegate::GetInstance()->GetName(i);
	//	if(sTxDelegateFile.GetLength()>0)
	//	{
	//		int index = m_pUserMenu->AddString(sTxDelegateFile);
	//		m_pUserMenu->SetItemData(index, i);
	//		if(i==TxDelegate::GetInstance()->GetDefault())
	//			iDefaultId = index;
	//	}
	//}
	//if(count>=0)
//		m_pUserMenu->SetCurSel(iDefaultId);

}

BOOL ExeDelegate::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		m_bPressESC = false;
		if (pMsg->wParam == VK_RETURN && ::IsChild(m_pUserMenu->m_hWnd, pMsg->hwnd)==FALSE)
			return TRUE;
		if (pMsg->wParam == VK_ESCAPE && ::IsChild(m_pUserMenu->m_hWnd, pMsg->hwnd))
		{
			m_bPressESC = true;
			m_pUserMenu->SetFocus();
			return TRUE;
		}
		//{
		//	if (pMsg->wParam == VK_RETURN)
		//	{
		//		m_pUserMenu->SetFocus();
		//		return TRUE;
		//	}
		//	else if (pMsg->wParam == VK_ESCAPE)
		//	{
		//		m_pUserMenu->CancelEditLabel();
		//		return TRUE;
		//	}
		//}
		//else
		//{
		//	if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
		//		return TRUE;
		//}
	}
	return DialogWithoutResource::PreTranslateMessage(pMsg);
}

LRESULT ExeDelegate::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_NOTIFY:
		{
			NMHDR* pNMHDR = (NMHDR*)lParam;
			if(pNMHDR!=NULL)
			{
				//LVS_EDITLABELS
				if(pNMHDR->idFrom == 0x1001 && pNMHDR->code == LVN_ENDLABELEDIT)
				{
					NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
					if(pDispInfo!=NULL && pDispInfo->item.pszText!=0 && m_bPressESC==false)
					{
						m_pUserMenu->SetItemText(pDispInfo->item.iItem, pDispInfo->item.iSubItem, pDispInfo->item.pszText);
						CString sExe;
						sExe=_T("");
						sExe = TxDelegate::GetInstance()->GetName(pDispInfo->item.iItem,false);
						CString sName;
						sName = pDispInfo->item.pszText;
						TxDelegate::GetInstance()->Modify(pDispInfo->item.iItem,sName,sExe);
					}
				}
			}
		}
		break;
	case (WM_USER+99):
		{
			ResetUserMenuList();
		}
		break;
	case WM_COMMAND:
		{
			if (HIWORD(wParam) == BN_CLICKED)
			{
				switch (LOWORD(wParam))
				{
				case 0x1002:
					//m_pEditFind;
					break;
				case 0x1003:
					//���
					{
						InputDelegate* pInputDelegate = new InputDelegate;
						if(pInputDelegate!=NULL)
						{
							pInputDelegate->ShowModal(this, _T("���ί��"), CRect(0,0,300,100));
							if(pInputDelegate->m_bOk==true)
							{
								CString sDelegate;
								CString sName;
								sDelegate = pInputDelegate->m_sExe;
								sName = pInputDelegate->m_sName;
								if(sDelegate.GetLength()>0)
								{
									if(sName.GetLength()>0)
									{
										int count = TxDelegate::GetInstance()->GetCount();
										if(TxDelegate::GetInstance()->Add(sName,sDelegate)==true)
										{
											//int index = m_pUserMenu->AddString(sName);
											//m_pUserMenu->SetItemData(index,count);
											//TxCreateLnk(m_sDelegatePath,sDelegate,sName,m_sDelegateDesc);
											//ShowDelegate();
											AddDelegate(m_pUserMenu->GetItemCount(),sName,sDelegate);
										}
									}
									else
									{
										AfxMessageBox(_T("��������ί������!"));
									}
								}
								else
								{
									AfxMessageBox(_T("���������ѡ��Ҫί�еĿ�ִ���ļ�!"));
								}
							}
							delete pInputDelegate;
						}
					}
					break;
				case 0x1004:
					//ɾ��
					{
						int index = GetCurSel();

						if(index>=0)
						{
							int idel = index;//m_pUserMenu->GetItemData(index);
							CString sName;
							sName = TxDelegate::GetInstance()->GetName(idel);
							if(sName.GetLength()>0)
							{
								CString sPmp;
								sPmp.Format(_T("ȷʵҪɾ�� %s ��?"),sName);
								if(AfxMessageBox(sPmp,MB_OKCANCEL)==IDOK)
								{
									m_pUserMenu->DeleteItem(index);
									TxDelegate::GetInstance()->Remove(index);
								}
							}
						}
					}
					break;
				case 0x1005:
					//�޸�
					{
						int index = GetCurSel();
						if(index<0)
							break;
						int idel = index;//m_pUserMenu->GetItemData(index);
						CString sNameOld,sNameNew;
						sNameOld = _T("");
						sNameOld = TxDelegate::GetInstance()->GetName(idel);
						//m_pUserMenu->GetText(index,sNameOld);
						if(sNameOld.GetLength()>0)
						{
							CString sPmp;
							sPmp.Format(_T("������ί�� %s ��������"),
								sNameOld
								);
							sNameNew = sNameOld;
							if(InputWnd::InputString(sNameNew,sPmp)==true)
							{
								if(sNameNew.GetLength()>0)
								{
									//int idel = m_pUserMenu->GetItemData(index);
									CString sExe;
									sExe=_T("");
									sExe = TxDelegate::GetInstance()->GetName(idel,false);

									sPmp.Format(_T("ȷʵҪ��ί�� %s �޸�Ϊ %s ��?"),sNameOld,sNameNew);
									if(AfxMessageBox(sPmp,MB_OKCANCEL)==IDOK)
									{
										if(TxDelegate::GetInstance()->Modify(idel,sNameNew,sExe)==true)
										{
											m_pUserMenu->SetItemText(index,0,sNameNew);
											AfxMessageBox(_T("�޸ĳɹ�!"));
										}
									}
								}
							}
						}
					}
					break;
				case 0x1006:
					//ִ��
					{
						int index = GetCurSel();
						if(index>=0)
						{
							int idel = index;//m_pUserMenu->GetItemData(index);
							TxDelegate::GetInstance()->Launch(idel);
						}
					}
					break;
				case 0x1007:
					//��ΪĬ��
					{
						int index = GetCurSel();
						if(index>=0)
						{
							int idel = index;//m_pUserMenu->GetItemData(index);
							TxDelegate::GetInstance()->SetDefault(idel);
							//m_pUserMenu->SetCurSel(index);
							//m_pUserMenu->Refresh();
							AfxMessageBox(_T("���óɹ�!"));
						}
					}
					break;
				case 0x1008:
					//�˳�
					TxDelegate::GetInstance()->Save();
					PostMessage(WM_CLOSE);
					break;
				default:
					break;
				}
			}
		}
		break;

	case WM_SIZE:
		if (::IsWindow(m_pUserMenu->m_hWnd)

			&& ::IsWindow(m_pBtnAdd->m_hWnd)
			&& ::IsWindow(m_pBtnDel->m_hWnd)
			&& ::IsWindow(m_pBtnMod->m_hWnd)
			&& ::IsWindow(m_pBtnRun->m_hWnd)
			&& ::IsWindow(m_pBtnSet->m_hWnd)
			&& ::IsWindow(m_pBtnExt->m_hWnd)

			&& ::IsWindow(m_pMyMenu->m_hWnd)
			&& ::IsWindow(m_pPrompt->m_hWnd)
			)
		{
			//�����Ϊ����������
			//���£�����Ϊ����������
			int iwBtn = 65;
			int ihBtn = 21;
			int iBorder = 8;
			//ȡ�ô���client rect
			CRect rect, rc, rcLeft,rcRight;
			GetClientRect(rect);
			//��С8
			rect.DeflateRect(iBorder, iBorder);

			rcLeft = rect;
			rcLeft.right -= iwBtn + iBorder*2;

			rcRight = rect;
			rcRight.left = rcLeft.right+iBorder;

			//left
			//ί��ִ��ȫ·������
			// listbox
			rc = rcLeft;
			rc.bottom -= ihBtn + iBorder*2;
			m_pUserMenu->MoveWindow(rc);

			//run
			rc = rcLeft;
			rc.top = rc.bottom - ihBtn;
			rc.left = rc.right - iwBtn;
			m_pBtnRun->MoveWindow(rc);

			
			// right:
			rc = rcRight;
			//rc.top += m_pUserMenu->GetHeadHeight();
			rc.bottom = rc.top + ihBtn;
			m_pBtnAdd->MoveWindow(rc);

			rc.OffsetRect(0, rc.Height() + iBorder);
			m_pBtnDel->MoveWindow(rc);

			rc.OffsetRect(0, rc.Height() + iBorder);
			m_pBtnMod->MoveWindow(rc);

			rc.OffsetRect(0, rc.Height() + iBorder);

			rc.OffsetRect(0, rc.Height() + iBorder);
			m_pBtnSet->MoveWindow(rc);

			rc = rcRight;
			rc.top = rc.bottom - ihBtn;
			m_pBtnExt->MoveWindow(rc);
		}
		break;
	default:
		break;
	}
	return DialogWithoutResource::WindowProc(message, wParam, lParam);
}
	
// �ڶԻ��򴴽������(�������� Create �ӿؼ�)
void ExeDelegate::_CreateControls()
{

	//if (m_pUserMenu->Create(
	//	WS_CHILD|WS_VISIBLE|WS_VSCROLL
	//	|LBS_OWNERDRAWFIXED|LBS_NOINTEGRALHEIGHT|LBS_HASSTRINGS, 
	//	CRect(0,0,0,0), this, 0x1001))
	//{
	//	m_pUserMenu->ModifyStyleEx(0, WS_EX_CLIENTEDGE, SWP_FRAMECHANGED);
	//	m_pUserMenu->SetItemHeight(0, 18);
	//}
/*
���в���dwStyle����ȷ���б���Ƶķ��rect����ȷ���б���ƵĴ�С��λ�ã�pParentWnd����ȷ���б���Ƶĸ����ڣ�ͨ����һ���Ի���nID����ȷ���б���Ƶı�ʶ�������б���Ƶķ�����������ֵ����ϣ�  
   
  LVS_ALIGNLEFT   ����ȷ������Ĵ�Сͼ��������뷽ʽ��ʾ��   
  LVS_ALIGNTOP   ����ȷ������Ĵ�Сͼ���Զ����뷽ʽ��ʾ��   
  LVS_AUTOARRANGE   ����ȷ������Ĵ�Сͼ�����Զ����з�ʽ��ʾ��   
  LVS_EDITLABELS   ���ñ����ı����Ա༭�������ڱ�������LVN_ENDLABELEDIT���   
  LVS_ICON   ����ȷ����ͼ�����ʾ��ʽ��   
  LVS_LIST   ����ȷ���б�ʽ��ʾ��   
  LVS_NOCOLUMNHEADER   ����ȷ������ϸ���Ϸ�ʽʱ����ʾ�б�ͷ��   
  LVS_NOLABELWRAP   ����ȷ���Ե��з�ʽ��ʾͼ����ı��   
  LVS_NOSCROLL   �������ι�������    
  LVS_NOSORTHEADER   ����ȷ���б�ͷ����������ť���ܣ�   
  LVS_OWNERDRAWFIXED   ����ϸ�б�ʽʱ�����Ի洰�ڣ�   
  LVS_REPORT   ����ȷ������ϸ���ϼ����淽ʽ��ʾ��   
  LVS_SHAREIMAGELISTS����ȷ������ͼ���б�ʽ��   
  LVS_SHOWSELALWAYS   ����ȷ��һֱ��ʾ��ѡ�б��ʽ��   
  LVS_SINGLESEL   ����ȷ����ĳһʱ��ֻ����һ�ѡ�У�   
  LVS_SMALLICON   ����ȷ��Сͼ����ʾ��ʽ��   
  LVS_SORTASCENDING   ����ȷ����������ʱ�ǻ��ڱ����ı�������ʽ��   
  LVS_SORTDESCENDING   ����ȷ����������ʱ�ǻ��ڱ����ı��Ľ���ʽ��  
*/
	if(m_pUserMenu->Create(
		WS_CHILD
		|WS_VISIBLE
		|WS_VSCROLL
		|LVS_ICON
		|LVS_AUTOARRANGE
		|LVS_SINGLESEL
		|LVS_SHOWSELALWAYS
		|LVS_EDITLABELS
		, 
		CRect(0,0,0,0), this, 0x1001))
	{
		m_pUserMenu->ModifyStyleEx(0, WS_EX_CLIENTEDGE, SWP_FRAMECHANGED);
		InitImageList();
		//OnReport();
	}

	if (m_pBtnAdd->Create(_T("���ί��"), WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, CRect(0,0,0,0), this, 0x1003))
	{
	}
	if (m_pBtnDel->Create(_T("ɾ��ί��"), WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, CRect(0,0,0,0), this, 0x1004))
	{
	}
	if (m_pBtnMod->Create(_T("�޸�ί��"), WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, CRect(0,0,0,0), this, 0x1005))
	{
	}
	if (m_pBtnRun->Create(_T("����ί��"), WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, CRect(0,0,0,0), this, 0x1006))
	{
	}
	if (m_pBtnSet->Create(_T("��ΪĬ��"), WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, CRect(0,0,0,0), this, 0x1007))
	{
	}
	if (m_pBtnExt->Create(_T("ȡ��"), WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, CRect(0,0,0,0), this, 0x1008))
	{
	}

	if (m_pMyMenu->Create(NULL,NULL,WS_CHILD|WS_VISIBLE, CRect(0,0,0,0), this, 0x1010))
	{
		
	}
	m_pMyMenu->ShowWindow(SW_HIDE);
	if (m_pPrompt->Create(NULL,NULL,WS_CHILD|WS_VISIBLE, CRect(0,0,0,0), this, 0x1012))
	{
		
	}
	m_pPrompt->ShowWindow(SW_HIDE);


	// ��ʱ��ʼ��
	PostMessage(WM_USER + 99);
}
//2008-04-24
IMPLEMENT_DYNAMIC(TxDelegateList, MultiColListBoxBase)

TxDelegateList::TxDelegateList()
{
	m_iDelegateColCount = 3;
	m_colorHeaderBK = GetSysColor(COLOR_BTNFACE);
	m_colorBK = GetSysColor(COLOR_WINDOW);

	m_colorSelectedBK = RGB(125,100,125);
	m_colorSelectedText = RGB(255,0,0);

	m_colorText = GetSysColor(COLOR_WINDOWTEXT);

	m_arrColWidth.assign(m_iDelegateColCount, 110);
	
	m_nDefaultHeight = 18;
}

TxDelegateList::~TxDelegateList()
{
}


BEGIN_MESSAGE_MAP(TxDelegateList, MultiColListBoxBase)
END_MESSAGE_MAP()

LRESULT TxDelegateList::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_USER + 90:	// Delay refresh
		Refresh();
		break;
	case WM_CREATE:
		{
			LRESULT ret = MultiColListBoxBase::WindowProc(message, wParam, lParam);
			int count = TxDelegate::GetInstance()->GetCount();

			int iDefaultId = 0;
			for (int i=0;i<count;i++)
			{
				AddString(_T(""));
			}
			return ret;
		}
		break;
	default:
		break;
	}

	return MultiColListBoxBase::WindowProc(message, wParam, lParam);
}


bool TxDelegateList::_ResetColWidth(int nWidth)
{
	// ����, ����
	m_arrColWidth[0] = 40;			// #
	m_arrColWidth[1] = 120;			// ����
	m_arrColWidth[2] = 180;			// ����
	int tmp = 40;
	for (int i = 2; i < (int)m_arrColWidth.size(); i++) 
	{
		tmp += m_arrColWidth[i];
	}
	if (::IsWindow(m_hWnd))
	{
		if ((GetStyle() & WS_VSCROLL) == 0)
		{
			tmp += ::GetSystemMetrics(SM_CXVSCROLL);
		}
	}
	m_arrColWidth[1] = nWidth - tmp;
	return true;
}

void TxDelegateList::_DrawCell(CDC* pdc, const CRect& rcCell, int nRow, int nCol, bool bSelected, bool bCurrent)
{
	if (rcCell.Width() == 0 || rcCell.Height() == 0) return;
	if (TxDelegate::GetInstance()->GetCount() < nRow
		|| m_iDelegateColCount < nCol) return;

	CRect rect(rcCell);
	//Tx::Business::TxBusiness business;

	COLORREF color = 0xffffff;

	UINT nFormat = DT_VCENTER|DT_SINGLELINE;

	CString str = _T(""); 

	switch (nCol)
	{
	case 0:		// #

		color = 0xffffff;
		str.Format(_T("%d"), nRow + 1);
		nFormat |= DT_CENTER;
		break;
	case 1:		// ����
		str = TxDelegate::GetInstance()->GetName(nRow);
		nFormat |= DT_LEFT;
		break;
	case 2:		// ����
		str = TxDelegate::GetInstance()->GetName(nRow,false);
		nFormat |= DT_RIGHT;
		break;
	};

	color = m_colorText;

	if(nRow == TxDelegate::GetInstance()->GetDefault())
		color = 0xff0000;
	else if(bCurrent)
		color = m_colorSelectedText;

	rect.DeflateRect(5, 0);
	int old_mode = pdc->SetBkMode(TRANSPARENT);
	CFont* old_font = pdc->SelectObject(GDIResource::Font(0));
	COLORREF old_color = pdc->SetTextColor(color);//

	pdc->DrawText(str, rect, nFormat);

	pdc->SetTextColor(old_color);
	pdc->SelectObject(old_font);
	pdc->SetBkMode(old_mode);
}

//void TxDelegateList::_DrawHeader(CDC* pdc, const CRect& rcHeader)
//{
//	CRect rect(rcHeader);
//	pdc->FillSolidRect(rect, m_colorHeaderBK);
//	CRect rc(rect);
//	rc.top = rc.bottom - 1;
//	pdc->FillSolidRect(rc, 0xa0);
//	rect.bottom--;
//	for (int c = 0; c < (int)m_arrColWidth.size(); c++)
//	{
//		rect.right = rect.left + m_arrColWidth[c];
//		_DrawHeaderCell(pdc, rect, c);
//		rect.left = rect.right;
//	}
//}

void TxDelegateList::_DrawHeaderCell(CDC* pdc, const CRect& rcCell, int nCol)
{
	if (rcCell.Width() == 0 || rcCell.Height() == 0) return;

	CRect rect(rcCell);

	UINT nFormat = DT_VCENTER|DT_SINGLELINE;
	COLORREF color = 0xffff;

	CString str = _T("");
	
	switch (nCol)
	{
	case 0:
		str = _T("#");
		nFormat |= DT_CENTER;
		break;
	case 1:
		str = _T("ί������");
		nFormat |= DT_LEFT;
		break;
	case 2:
		str = _T("����·��");
		nFormat |= DT_RIGHT;
		break;
	};

	// �������־
	if (m_nSortDir != 0)
	{
		if (m_nSortCol == nCol)
		{
			str += m_nSortDir < 0 ? _T(" ��") :  _T(" ��");
		}
		else
		{
			rect.DeflateRect(5, 0);
		}
	}
	else
	{
		rect.DeflateRect(5, 0);
	}
	color = m_colorText;

	int old_mode = pdc->SetBkMode(TRANSPARENT);
	CFont* old_font = pdc->SelectObject(GDIResource::Font(0));
	COLORREF old_color = pdc->SetTextColor(color);

	pdc->DrawText(str, rect, nFormat);

	pdc->SetTextColor(old_color);
	pdc->SelectObject(old_font);
	pdc->SetBkMode(old_mode);
}

void TxDelegateList::PostRefresh()
{
	if (::IsWindow(m_hWnd)) PostMessage(WM_USER + 90);
}

void TxDelegateList::Refresh()
{
	if (InDrawing()) return;

	if (::IsWindow(m_hWnd)) 
	{
		Invalidate();
	}
}

///////////////////////////////////////////////////
IMPLEMENT_DYNAMIC(InputDelegate, DialogWithoutResource)
InputDelegate::InputDelegate()
{
	m_pEditFind = new EditEx;
	m_pName = new EditEx;

	m_pBtnBrw = new CButton;
	m_pBtnCan = new CButton;
	m_pBtnExt = new CButton;

	m_pEditStatic = new CStatic;
	m_pNameStatic = new CStatic;
}

InputDelegate::~InputDelegate()
{
	delete m_pEditStatic;
	delete m_pNameStatic;

	delete m_pBtnExt;
	delete m_pBtnBrw;
	delete m_pBtnCan;
	delete m_pEditFind;
	delete m_pName;
}

BEGIN_MESSAGE_MAP(InputDelegate, DialogWithoutResource)
END_MESSAGE_MAP()



BOOL InputDelegate::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		if (pMsg->hwnd == m_pEditFind->m_hWnd)
		{
			//FindMenu();
			//return TRUE;
		}
		return TRUE;
	}
	return DialogWithoutResource::PreTranslateMessage(pMsg);
}

LRESULT InputDelegate::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
		{
			if (HIWORD(wParam) == BN_CLICKED)
			{
				switch (LOWORD(wParam))
				{
				case 0x1004:
					//ȡ��
					m_bOk = false;
					PostMessage(WM_CLOSE);
					break;
				case 0x1003:
					//ȷ��
					m_pEditFind->GetWindowText(m_sExe);
					m_pName->GetWindowText(m_sName);

					m_bOk = true;
					PostMessage(WM_CLOSE);
					break;
				case 0x1005:
					{
						//���
						//1 open dialog for browser
						//2 select a file
						//3 add file name into m_pEditFind
						CString sFileName;
						CString OpenFilter;
								OpenFilter  = _T("��ִ���ļ� (*.exe)|*.exe|");
								OpenFilter += _T("��ִ���ļ� (*.com)|*.com|");
								OpenFilter += _T("�������ļ� (*.bat)|*.bat|");
								OpenFilter += _T("�������ļ� (*.bat;*.com;*.exe)|*.bat;*.com;*.exe|");
								OpenFilter += _T("�����ļ� (*.*)|*.*||");

						CFileDialog dlg(
									TRUE,
									_T("exe"), 
									sFileName, 
									OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, 
									OpenFilter
									);
						if (IDOK == dlg.DoModal())
						{
							sFileName = dlg.GetPathName();
							if(sFileName.GetLength()>0)
							{
								m_pEditFind->SetWindowText(sFileName);
								CString sName;
								m_pName->GetWindowText(sName);
								if(sName.IsEmpty())
								{
									//2008-07-23
									//m_pName->SetWindowText(sFileName);
									m_pName->SetWindowText(dlg.GetFileTitle());
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
		break;

	case WM_SIZE:
		if (
			 ::IsWindow(m_pBtnCan->m_hWnd)
			&& ::IsWindow(m_pBtnExt->m_hWnd)
			&& ::IsWindow(m_pBtnBrw->m_hWnd)

			&& ::IsWindow(m_pEditFind->m_hWnd)
			&& ::IsWindow(m_pName->m_hWnd)
			&& ::IsWindow(m_pEditStatic->m_hWnd)
			&& ::IsWindow(m_pNameStatic->m_hWnd)
			
			)
		{
			//�ܹ�3��
			//��һ�У�����
			//�ڶ��У�·��+���
			//�����У�ȷ��+ȡ��

			int iwBtn = 65;
			int ihBtn = 21;
			int iBorder = 8;

			int iwEdit = 100;

			int iwStatic = 72;

			//ȡ�ô���client rect
			CRect rect, rc;
			GetClientRect(rect);
			//��С8
			rect.DeflateRect(iBorder, iBorder);

			CRect r1,r2,r3;

			//��һ�У�����
			r1 = rect;
			r1.bottom = r1.top+ihBtn;

			//�ڶ��У�·��+���
			r2 = rect;
			r2.top = r1.bottom+iBorder;
			r2.bottom = r2.top+ihBtn;

			//�����У�ȷ��+ȡ��
			r3 = rect;
			//r3.top = r2.bottom+iBorder;
			//r3.bottom = r3.top+ihBtn;

			r3.top = r3.bottom-ihBtn;

			//1
			rc = r1;
			rc.right=rc.left+iwStatic;
			m_pNameStatic->MoveWindow(rc);

			rc.left = rc.right+iBorder;
			rc.right = r1.right-iwBtn-iBorder;
			m_pName->MoveWindow(rc);

			//2
			rc = r2;
			rc.right=rc.left+iwStatic;
			m_pEditStatic->MoveWindow(rc);

			rc = r2;
			rc.left += iwStatic+iBorder;
			rc.right -= iwBtn+iBorder;
			m_pEditFind->MoveWindow(rc);

			rc = r2;
			//rc.right=rc.left+iwBtn;
			//rc.left = rc.right+iBorder;
			//rc.right -= iBorder;
			rc.left = rc.right-iwBtn;
			m_pBtnBrw->MoveWindow(rc);

			//3
			//ȷ�ϰ�ť
			rc = r3;
			//rc.right=rc.left-iBorder;
			//rc.left=rc.right-iwBtn;

			//rc.right -= iBorder;
			rc.left = rc.right-iwBtn;
			m_pBtnCan->MoveWindow(rc);

			//rc.right=rc.left+iwStatic;
			//rc.left = rc.right+iBorder;
			//rc.right=rc.left+iwEdit;
			//rc.left = rc.right+iBorder;
			//rc.right=rc.left+iwBtn;

			//ȡ����ť
			//rc.OffsetRect(rc.left-(iwBtn+iBorder), 0);
			rc = r3;
			rc.right -= iwBtn+iBorder;
			rc.left  = rc.right - iwBtn;
			m_pBtnExt->MoveWindow(rc);

		}
		break;
	default:
		break;
	}
	return DialogWithoutResource::WindowProc(message, wParam, lParam);
}
	
// �ڶԻ��򴴽������(�������� Create �ӿؼ�)
void InputDelegate::_CreateControls()
{
	//name
	if (m_pName->Create(WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL, CRect(0,0,0,0), this, 0x1001))
	{
		m_pName->ModifyStyleEx(0, WS_EX_CLIENTEDGE, SWP_FRAMECHANGED);
	}
	//exe
	if (m_pEditFind->Create(WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL, CRect(0,0,0,0), this, 0x1002))
	{
		m_pEditFind->ModifyStyleEx(0, WS_EX_CLIENTEDGE, SWP_FRAMECHANGED);
	}
	if (m_pBtnExt->Create(_T("ȷ��"), WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, CRect(0,0,0,0), this, 0x1003))
	{
	}
	if (m_pBtnCan->Create(_T("ȡ��"), WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, CRect(0,0,0,0), this, 0x1004))
	{
	}
	if (m_pBtnBrw->Create(_T("���"), WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, CRect(0,0,0,0), this, 0x1005))
	{
	}
	m_pEditStatic->Create(_T("ί���ļ�·��"),WS_CHILD|WS_VISIBLE|SS_CENTERIMAGE,CRect(0,0,0,0),this);
	m_pNameStatic->Create(_T("ί������"),WS_CHILD|WS_VISIBLE|SS_CENTERIMAGE,CRect(0,0,0,0),this);

	// ��ʱ��ʼ��
	PostMessage(WM_USER + 99);
}
//	dlg.ShowModal(parent, _T("����ί��"), CRect(0,0,iWidth,(int)(iWidth*0.618)));
void ExeDelegate::ShowDelegate(void)
{
	if(m_pUserMenu==NULL)
		return;

	int nCount = 0;
	m_pUserMenu->DeleteAllItems();
	//CFileFind cff;
	//CString strDes = m_sDelegatePath + _T("\\*.*");
	//BOOL bFind = FALSE;
	//bFind = cff.FindFile( strDes );
	//while ( bFind )
	//{
	//	bFind = cff.FindNextFile();
	//	if ( !cff.IsDirectory()&&!cff.IsDots())
	//	{
	//		m_pUserMenu->InsertItem(nCount++,cff.GetFileName(),GetFileIconIndex(cff.GetFileName(),bSmallIcon));
	//	}
	//}


	CString sTxDelegateFile;

	int count = TxDelegate::GetInstance()->GetCount();

	int iDefaultId = TxDelegate::GetInstance()->GetDefault();
	for (int i=0;i<count;i++)
	{
		sTxDelegateFile = TxDelegate::GetInstance()->GetName(i);
		if(sTxDelegateFile.GetLength()>0)
		{
			int index = AddDelegate(i,sTxDelegateFile,TxDelegate::GetInstance()->GetName(i,false)); //m_pUserMenu->InsertItem(i,sTxDelegateFile,GetFileIconIndex(TxDelegate::GetInstance()->GetName(i,false),m_bSmallIcon));
			//m_pUserMenu->SetItemData(index, i);
			if(i==TxDelegate::GetInstance()->GetDefault())
				iDefaultId = index;
		}
	}
	if(iDefaultId>=0)
		m_pUserMenu->SetItemState(iDefaultId,LVIS_SELECTED, LVIS_SELECTED);
}
int ExeDelegate::AddDelegate(int i,CString ItemText,CString file)
{
	int index = m_pUserMenu->InsertItem(i,ItemText,GetFileIconIndex(file,m_bSmallIcon));
	m_pUserMenu->SetItemData(index, i);
	return index;
}
int ExeDelegate::GetFileIconIndex( CString strFileName , BOOL bSmallIcon )
{
	SHFILEINFO    sfi;

	if (bSmallIcon)
	{
		SHGetFileInfo(
			(LPCTSTR)strFileName, 
			FILE_ATTRIBUTE_NORMAL,
			&sfi, 
			sizeof(SHFILEINFO), 
			SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
	}
	else
	{
		SHGetFileInfo(
			(LPCTSTR)strFileName, 
			FILE_ATTRIBUTE_NORMAL,
			&sfi, 
			sizeof(SHFILEINFO), 
			SHGFI_SYSICONINDEX | SHGFI_LARGEICON | SHGFI_USEFILEATTRIBUTES);
	}

	return sfi.iIcon;

}
int ExeDelegate::GetCurSel(void)
{
	POSITION pos = m_pUserMenu->GetFirstSelectedItemPosition();
	if (pos == NULL)
		return -1;
	return m_pUserMenu->GetNextSelectedItem(pos);
	//return m_pUserMenu->GetNextItem (-1, LVNI_SELECTED); 

}
void ExeDelegate::InitImageList(void)
{
	HIMAGELIST  hSystemSmallImageList; 
	SHFILEINFO    ssfi; 

	//ֻ��Ҫ����ͼ��
	DWORD dwIconType[] = {
		SHGFI_DISPLAYNAME | SHGFI_ICON | SHGFI_SMALLICON,
		SHGFI_ICON | SHGFI_OPENICON | SHGFI_SMALLICON,
		SHGFI_SYSICONINDEX | SHGFI_ICON ,
		SHGFI_SYSICONINDEX | SHGFI_SMALLICON,
		SHGFI_SYSICONINDEX | SHGFI_LARGEICON,
		SHGFI_SYSICONINDEX | SHGFI_SMALLICON
	};

	int it_index = 2;

	if(m_bSmallIcon==TRUE)
		it_index = 0;
	else
		it_index = 4;
	//get a handle to the system small icon list 
	hSystemSmallImageList = 
		(HIMAGELIST)SHGetFileInfo( 
		//(LPCTSTR)m_sDelegatePath, 
		_T("c:\\"), 
		0, 
		&ssfi, 
		sizeof(SHFILEINFO), 
		dwIconType[it_index]); 

	m_hImageList.Detach();
	m_hImageList.Attach(hSystemSmallImageList);
	m_pUserMenu->SetImageList( &m_hImageList,TVSIL_NORMAL );
	//m_pUserMenu->SetImageList( CImageList::FromHandle( hSystemSmallImageList ),TVSIL_NORMAL );
}
  //��4�������б���ʾ��ʽ����
  //  ������Classwizard���򵼴��������ܰ�ť��ʾ���ܺ���֮�󣬱�������������Щ���ܺ����Ĵ��룬��Щ���ܺ������£�
void ExeDelegate::OnStdicon()//���ô�ͼ����ʾ��ʽ
{ // TODO: Add your control notification handler code here
	SetListStyle(LVS_ICON);
 //LONG lStyle;
 //lStyle=GetWindowLong(m_pUserMenu->m_hWnd,GWL_STYLE);//��ȡ��ǰ��������
 //lStyle&=~LVS_TYPEMASK; file://�����ʾ��ʽλ
 //lStyle|=LVS_ICON;       file://������ʾ��ʽ
 //SetWindowLong(m_pUserMenu->m_hWnd,GWL_STYLE,lStyle);//���ô�������
}
void ExeDelegate::OnSmlicon()// file://����Сͼ����ʾ��ʽ
{ // TODO: Add your control notification handler code here
	SetListStyle(LVS_SMALLICON);
 //LONG lStyle;
 //lStyle=GetWindowLong(m_pUserMenu->m_hWnd,GWL_STYLE);//��ȡ��ǰ��������
 //lStyle&=~LVS_TYPEMASK; file://�����ʾ��ʽλ
 //lStyle|=LVS_SMALLICON;  file://������ʾ��ʽ
 //SetWindowLong(m_pUserMenu->m_hWnd,GWL_STYLE,lStyle);//���ô�������
}
void ExeDelegate::OnList()// file://�����б���ʾ��ʽ
{ // TODO: Add your control notification handler code here
	SetListStyle(LVS_LIST);
 //LONG lStyle;
 //lStyle=GetWindowLong(m_pUserMenu->m_hWnd,GWL_STYLE);//��ȡ��ǰ��������
 //lStyle&=~LVS_TYPEMASK; file://�����ʾ��ʽλ
 //lStyle|=LVS_LIST;       file://������ʾ��ʽ
 //SetWindowLong(m_pUserMenu->m_hWnd,GWL_STYLE,lStyle);//���ô�������
}
void ExeDelegate::SetListStyle(LONG lStyleNew)
{
 LONG lStyle;
 lStyle=GetWindowLong(m_pUserMenu->m_hWnd,GWL_STYLE);//��ȡ��ǰ��������
 lStyle&=~LVS_TYPEMASK;// file://�����ʾ��ʽλ
 lStyle|=lStyleNew;//     file://������ʾ��ʽ
 SetWindowLong(m_pUserMenu->m_hWnd,GWL_STYLE,lStyle);//���ô�������
}
void ExeDelegate::OnReport()// file://��ϸ������ʾ��ʽ
{
	SetListStyle(LVS_REPORT);
	//// TODO: Add your control notification handler code here
 //LONG lStyle;
 //lStyle=GetWindowLong(m_pUserMenu->m_hWnd,GWL_STYLE);//��ȡ��ǰ��������
 //lStyle&=~LVS_TYPEMASK; file://�����ʾ��ʽλ
 //lStyle|=LVS_REPORT;     file://������ʾ��ʽ
 //SetWindowLong(m_pUserMenu->m_hWnd,GWL_STYLE,lStyle);//���ô�������
}

}
}