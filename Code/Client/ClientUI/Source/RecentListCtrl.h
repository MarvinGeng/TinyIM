﻿/**
 * @file RecentListCtrl.h
 * @author DennisMi (https://www.dennisthink.com/)
 * @brief 最近列表控件
 * @version 0.1
 * @date 2019-08-05
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#ifndef _DENNIS_THINK_C_RECENT_LIST_CTRL_H_
#define _DENNIS_THINK_C_RECENT_LIST_CTRL_H_

#include <vector>
#include "SkinLib/SkinLib.h"

#define DELETE_IMAGE(p) \
	if(p!=NULL)			\
	{					\
	delete p;			\
	p=NULL;				\
	}

//enum BLCTRL_STYLE					// 好友列表控件风格
//{
//	BLC_BIG_ICON_STYLE,				// 大头像风格
//	BLC_SMALL_ICON_STYLE,			// 小头像风格
//	BLC_STANDARD_ICON_STYLE			// 标准头像风格
//};
//
//enum BLCTRL_TYPE
//{
//	BUDDY_LIST_TYPE_BUDDY,
//	BUDDY_LIST_TYPE_GROUPE,
//	BUDDY_LIST_TYPE_RECENT
//};

enum RLCTRL_STATUS_TYPE
{
	RCLCTRL_STATUS_TYPE_PC,			//电脑在线
	RCLCTRL_STATUS_TYPE_MOBILE,		//手机在线
	RCLCTRL_STATUS_TYPE_OFFLINE		//离线
};

class CRecentItem
{
public:
	CRecentItem(void);
	~CRecentItem(void);

public:
	long					m_nID;					// 项ID
	UINT					m_uAccountID;			// 账户ID
	CString					m_strNickName;			// 昵称
	CString					m_strDate;				// 日期
	CString					m_strLastMsg;			// 最后一条消息内容
	RLCTRL_STATUS_TYPE		m_nOnlineStatus;		// 是否在线标志
	CImageEx*				m_lpHeadImg;			// 头像图片
	CImageEx*				m_lpMobileImg;			// 手机图标
	long					m_nNewMsgCount;			// 新消息个数
};

class CRecentTeamItem
{
public:
	CRecentTeamItem(void);
	~CRecentTeamItem(void);

public:
	int			m_nID;								// 项ID
	CString		m_strName;							// 好友分组名称
	int			m_nCurCnt;							// 当前在线好友数
	int			m_nMaxCnt;							// 该好友分组的好友数
	BOOL		m_bExpand;							// 是否展开标志
	int			m_nHeadFlashAnim;					// 头像闪烁动画标志
	int			m_nHeadFlashAnimState;				// 头像闪烁动画状态
	std::vector<CRecentItem*> m_arrBuddys;			// 该好友分组的好友项
};

//最近聊天列表
class CRecentListCtrl : public CWindowImpl<CRecentListCtrl, CWindow>
{
public:
	CRecentListCtrl(void);
	~CRecentListCtrl(void);

	BEGIN_MSG_MAP_EX(CRecentListCtrl)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		MSG_WM_PAINT(OnPaint)
		MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_LBUTTONUP(OnLButtonUp)
		MSG_WM_RBUTTONUP(OnRButtonUp)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_MOUSELEAVE(OnMouseLeave)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_VSCROLL(OnVScroll)
		MSG_WM_MOUSEWHEEL(OnMouseWheel)
		MSG_WM_KEYDOWN(OnKeyDown)
		MSG_WM_SIZE(OnSize)
		MSG_WM_GETDLGCODE(OnGetDlgCode)
		MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST, WM_MOUSELAST, OnMouseMessage)
		MSG_WM_DESTROY(OnDestroy)
	END_MSG_MAP()

public:
	BOOL SetBgPic(LPCTSTR lpszFileName);
	BOOL SetBuddyTeamHotBgPic(LPCTSTR lpszFileName);
	BOOL SetBuddyItemHotBgPic(LPCTSTR lpszFileName, RECT* lpNinePart = NULL);
	BOOL SetBuddyItemSelBgPic(LPCTSTR lpszFileName, RECT* lpNinePart = NULL);
	BOOL SetHeadFramePic(LPCTSTR lpszFileName, RECT* lpNinePart = NULL);
	BOOL SetNormalArrowPic(LPCTSTR lpszFileName);
	BOOL SetHotArrowPic(LPCTSTR lpszFileName);
	BOOL SetSelArrowPic(LPCTSTR lpszFileName);
	BOOL SetNormalExpArrowPic(LPCTSTR lpszFileName);
	BOOL SetHotExpArrowPic(LPCTSTR lpszFileName);
	BOOL SetSelExpArrowPic(LPCTSTR lpszFileName);
	BOOL SetStdGGHeadPic(LPCTSTR lpszFileName);
	BOOL SetStdMMHeadPic(LPCTSTR lpszFileName);

	//void SetStyle(BLCTRL_STYLE nStyle);
	void SetShowBigIconInSel(BOOL bShowBigIconInSel);
	void SetBuddyTeamHeight(int nHeight);
	void SetBuddyItemHeightInBigIcon(int nHeight);
	void SetBuddyItemHeightInSmallIcon(int nHeight);
	void SetBuddyItemHeightInStandardIcon(int nHeight);
	void SetBuddyTeamPadding(int nPadding);
	void SetBuddyItemPadding(int nPadding);
	void SetMargin(const RECT* lpMargin);

	int AddBuddyTeam(int nID);
	int InsertBuddyTeam(int nTeamIndex, int nID);
	BOOL DelBuddyTeam(int nTeamIndex);

	int AddBuddyItem(int nTeamIndex, int nID);
	int InsertBuddyItem(int nTeamIndex, int nIndex, int nID);
	BOOL DelBuddyItem(int nTeamIndex, int nIndex);

	void DelAllItems();

	void SetBuddyTeamID(int nTeamIndex, int nID);
	void SetBuddyTeamName(int nTeamIndex, LPCTSTR lpszText);
	void SetBuddyTeamMaxCnt(int nTeamIndex, int nMaxCnt);
	void SetBuddyTeamCurCnt(int nTeamIndex, int nCurCnt);
	void SetBuddyTeamExpand(int nTeamIndex, BOOL bExpand);
	//inline void SetBuddyListCtrlType(BLCTRL_TYPE type) {m_nType = type;}

	int GetBuddyTeamID(int nTeamIndex);
	CString GetBuddyTeamName(int nTeamIndex);
	int GetBuddyTeamMaxCnt(int nTeamIndex);
	int GetBuddyTeamCurCnt(int nTeamIndex);
	BOOL IsBuddyTeamExpand(int nTeamIndex);

	void SetBuddyItemID(int nTeamIndex, int nIndex, int nID);
	void SetBuddyItemAccountID(int nTeamIndex, int nIndex, UINT  uAccountID);
	void SetBuddyItemNickName(int nTeamIndex, int nIndex, LPCTSTR lpszText, BOOL bShow=TRUE);
	void SetBuddyItemAccount(int nTeamIndex, int nIndex, LPCTSTR lpszText, BOOL bShow=TRUE);
	void SetBuddyItemDate(int nTeamIndex, int nIndex, LPCTSTR lpszText);
	void SetBuddyItemLastMsg(int nTeamIndex, int nIndex, LPCTSTR lpszText);
	void SetBuddyItemGender(int nTeamIndex, int nIndex, BOOL bGender);
	BOOL SetBuddyItemHeadPic(int nTeamIndex, int nIndex, LPCTSTR lpszFileName, BOOL bGray);
	BOOL SetBuddyItemMobilePic(int nTeamIndex, int nIndex, LPCTSTR lpszFileName);
	void SetBuddyItemHeadFlashAnim(int nTeamIndex, int nIndex, BOOL bHeadFlashAnim);
	void SetBuddyItemOnline(int nTeamIndex, int nIndex, BOOL bOnline, BOOL bAnim);
	void SetBuddyItemNewMsgCount(int nTeamIndex, int nIndex, long nNewMsgCount);

	int GetBuddyItemID(int nTeamIndex, int nIndex);
	UINT GetBuddyItemAccountID(int nTeamIndex, int nIndex);
	CString GetBuddyItemUTalkNum(int nTeamIndex, int nIndex);
	CString GetBuddyItemNickName(int nTeamIndex, int nIndex);
	CString GetBuddyItemAccount(int nTeamIndex, int nIndex);
	CString GetBuddyItemSign(int nTeamIndex, int nIndex);
	CString GetBuddyItemLastMsg(int nTeamIndex, int nIndex);
	BOOL GetBuddyItemGender(int nTeamIndex, int nIndex);
	BOOL IsBuddyItemHasMsg(int nTeamIndex, int nIndex);

	int GetBuddyTeamCount();
	int GetBuddyItemCount(int nTeamIndex);

	BOOL GetItemIndexByID(int nID, int& nTeamIndex, int& nIndex);
	BOOL GetItemRectByIndex(int nTeamIndex, int nIndex, CRect& rect);
	
	void GetCurSelIndex(int& nTeamIndex, int& nIndex);
	//BLCTRL_STYLE GetStyle();
	BOOL IsShowBigIconInSel();

	void SetTransparent(BOOL bTransparent, HDC hBgDC);

private:
	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	BOOL OnEraseBkgnd(CDCHandle dc);
	void OnPaint(CDCHandle dc);
	void OnLButtonDblClk(UINT nFlags, CPoint point);
	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnLButtonUp(UINT nFlags, CPoint point);
	void OnRButtonUp(UINT nFlags, CPoint point);
	void OnMouseMove(UINT nFlags, CPoint point);
	void OnMouseLeave();
	void OnTimer(UINT_PTR nIDEvent);
	void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar pScrollBar);
	BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnSize(UINT nType, CSize size);
	UINT OnGetDlgCode(LPMSG lpMsg);
	LRESULT OnMouseMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnDestroy();

	void DrawParentWndBg(HDC hDC);
	void DrawBuddyTeam(HDC hDC, int nIndex);
	void DrawBuddyItem(HDC hDC, int nTeamIndex, int nIndex);
	void DrawBuddyItemInBigIcon(HDC hDC, int nTeamIndex, int nIndex);
	void DrawBuddyItemInSmallIcon(HDC hDC, int nTeamIndex, int nIndex);
	void DrawBuddyItemInStdIcon(HDC hDC, int nTeamIndex, int nIndex);

	BOOL StartTrackMouseLeave();
	void CalcCenterRect(CRect& rcDest, int cx, int cy, CRect& rcCenter);
	void HitTest(POINT pt, int& nTeamIndex, int& nIndex);

	CRecentTeamItem* GetBuddyTeamByIndex(int nIndex);
	CRecentItem* GetBuddyItemByIndex(int nTeamIndex, int nIndex);

	void CheckScrollBarStatus();
	void Scroll(int cx, int cy);
	void EnsureVisible(int nTeamIndex, int nIndex);

	void SortBuddyTeam(int nTeamIndex, int nIndex);
	void OnTimer_HeadFlashAnim(UINT_PTR nIDEvent);	// 头像闪动动画
	void OnTimer_OnlineAnim(UINT_PTR nIDEvent);		// 上线动画
	void OnTimer_OfflineAnim(UINT_PTR nIDEvent);	// 下线动画

private:
	CToolTipCtrl			m_ToolTipCtrl;
	CSkinScrollBar			m_VScrollBar;

	CImageEx*				m_lpBgImg;
	CImageEx*				m_lpBuddyTeamBgImgH;
	CImageEx*				m_lpArrowImgN; 
	CImageEx*				m_lpArrowImgH;
	CImageEx*				m_lpArrowImgS;
	CImageEx*				m_lpExpArrowImgN;
	CImageEx*				m_lpExpArrowImgH;
	CImageEx*				m_lpExpArrowImgS;
	CImageEx*				m_lpBuddyItemBgImgH;
	CImageEx*				m_lpBuddyItemBgImgS;
	CImageEx*				m_lpHeadFrameImg;
	CImageEx*				m_lpStdGGHeadImg;
	CImageEx*				m_lpStdMMHeadImg;

	int						m_nSelTeamIndex;
	int 					 m_nSelIndex;
	int						m_nHoverTeamIndex;
	int 					 m_nHoverIndex;
	int						m_nHoverSmallIconIndex;
	int						m_nPressTeamIndex;
	int 					m_nPressIndex;
	int						m_nLeft, m_nTop;
	//BLCTRL_TYPE				m_nType;		// 好友列表，群列表，最近联系人列表类型标识
	BOOL					m_bMouseTracking;
	BOOL					m_bTransparent;
	HDC						m_hBgDC;
	//BLCTRL_STYLE			m_nStyle;
	BOOL					m_bShowBigIconInSel;
	int						m_nBuddyTeamHeight;
	int						m_nBuddyItemHeightInBig;
	int						m_nBuddyItemHeightInSmall;
	int						m_nBuddyItemHeightInStd;
	int						m_nBuddyTeamPadding;
	int						m_nBuddyItemPadding;
	CRect					m_rcMargin;
	DWORD					m_dwOnlineAnimTimerId;		// 上线动画计时器
	DWORD					m_dwOfflineAnimTimerId;		// 下线动画计时器
	DWORD					m_dwHeadFlashAnimTimerId;	// 头像闪动动画计时器

	std::vector<CRecentTeamItem*>	m_arrBuddyTeamItems;
	std::vector<CRecentItem*>		m_arrRecentItems;
};
#endif