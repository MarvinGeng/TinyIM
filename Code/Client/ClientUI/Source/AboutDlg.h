﻿/**
 * @file AboutDlg.h
 * @author DennisMi (https://www.dennisthink.com/)
 * @brief 关于对话框
 * @version 0.1
 * @date 2019-08-07
 * 
 * @copyright Copyright (c) 2019
 * 
 */


#ifndef _DENNIS_THINK_C_ABOUT_DLG_H_ 
#define _DENNIS_THINK_C_ABOUT_DLG_H_
#include "resource.h"
#include "SkinLib/SkinLib.h"


class CAboutDlg : public CDialogImpl<CAboutDlg>
{
public:
	enum { IDD = IDD_ABOUT_BOX };

	BEGIN_MSG_MAP(CAboutDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	CSkinDialog		m_SkinDlg;//对话框类
	CSkinStatic		m_staAboutInfo;//关于信息
	CSkinHyperLink	m_hyperLinkVersion;//公司或者版本的链接
};
#endif