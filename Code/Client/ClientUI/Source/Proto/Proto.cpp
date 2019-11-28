﻿/**
 * @file Proto.cpp
 * @author DennisMi (https://www.dennisthink.com/)
 * @brief 
 * @version 0.1
 * @date 2019-07-28
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#include "Proto.h"
#include "asioNet/CSessManager.h"
#include "CommonMsg.h"
#include "UI_USER_INFO.h"
#include "./net/IUProtocolData.h"
#include "EncodingUtil.h"
#include "UIWndMsgDef.h"

/**
 * @brief 将TCP消息中的字体结构体转为界面的字体结构体
 * 
 * @param font TCP消息的字体结构体
 * @return C_UI_FontInfo 界面的字体结构体
 */
C_UI_FontInfo CoreToUi(const FontInfo_s font)
{
	C_UI_FontInfo result;
	{
		TCHAR chBuff[32] = { 0 };
		//RGBToHexStr(font.m_clrText, chBuff, 32);
		result.ColorHexString(font.m_strFontColorHex);
		result.m_nSize = font.m_nFontSize;
		if (font.IsBold())
		{
			result.m_bBold = TRUE;
		}
		else
		{
			result.m_bBold = FALSE;
		}

		if (font.IsItalic())
		{
			result.m_bItalic = TRUE;
		}
		else
		{
			result.m_bItalic = FALSE;
		}

		if (font.IsUnderScore())
		{
			result.m_bUnderLine = TRUE;
		}
		else
		{
			result.m_bUnderLine = FALSE;
		}
	}
	return result;
}

/**
 * @brief 将界面的字体结构体转为TCP消息的字体结构体
 * 
 * @param font 界面字体结构体
 * @return FontInfo_s TCP消息字体结构体
 */
FontInfo_s UiToCore(const C_UI_FontInfo font)
{
	FontInfo_s result;
	{
		TCHAR chBuff[32] = { 0 };
		//RGBToHexStr(font.m_clrText, chBuff, 32);
		result.m_strFontColorHex = font.ColorHexString();
		result.m_strFontName = EncodeUtil::UnicodeToUtf8(font.m_strName);
		result.m_nFontSize = font.m_nSize;
		if (font.m_bBold)
		{
			result.SetBold();
		}
		if (font.m_bItalic)
		{
			result.SetItalic();
		}
		if (font.m_bUnderLine)
		{
			result.SetUnderScore();
		}
	}

	return result;
}
static std::shared_ptr<CMsgProto> g_MsgProtoIns=nullptr;


CMsgProto::CMsgProto() {
		
		m_bRecvRegisterRsp = false;
		
		m_bRecLoginRsp = false;
		m_eOnLineStatus = E_UI_ONLINE_STATUS::STATUS_OFFLINE;
}

/**
 * @brief 协议的初始化
 * 
 */
void CMsgProto::Init() {
		m_startTime = 0;
		auto pSess = SourceServer::CSessManager::GetManager();
		auto pIn = shared_from_this();
		pSess->SetTimeOutCallBack([this, pIn]()->void {
			OnTimeOut();
		});

		pSess->SetMsgCallBack(
			[this, pIn](const TransBaseMsg_t* pMsg)->bool {
			return OnMsgBack(pMsg);
		});
}

/**
	 * @brief 发送用户的注册请求，同步模式
	 * 
	 * @param strUserName 用户名
	 * @param strPassword 密码
	 * @param strNickName 昵称
	 * @param nSeconds 等待秒数
	 * @return UserRegisterRspMsg  注册结果的回复
	 */
	UserRegisterRspMsg  CMsgProto::SendRegisterSync(const std::string strUserName, const std::string strPassword, const std::string strNickName, const int nSeconds) {
		m_bRecvRegisterRsp = false;
		m_startTime = 0;
		SendUserRegister(strUserName, strPassword, strNickName);
		while (m_startTime < nSeconds && !m_bRecvRegisterRsp) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		
		return m_userRegisterRsp;
	}


	/**
	 * @brief 用户登录请求，同步模式
	 * 
	 * @param strUserName 用户名
	 * @param strPassword 密码
	 * @param nSeconds 等待秒数
	 * @return UserLoginRspMsg 登录结果
	 */
	UserLoginRspMsg CMsgProto::SendLoginSync(const std::string strUserName, const std::string strPassword, const int nSeconds) {
		if (strUserName.empty() || strPassword.empty())
		{
			UserLoginRspMsg rspMsg;
			return rspMsg;
		}
		m_bRecLoginRsp = false;
		m_startTime = 0;
		SendUserLogin(strUserName, strPassword);
		int index = 0;
		while (index < nSeconds && m_startTime < nSeconds && !m_bRecLoginRsp) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
			index++;
		}
		UserLoginRspMsg rspMsg = m_userLoginRsp;
		return rspMsg;
	}


		/**
	 * @brief 由网络层判断收到一个完整消息以后回调，在此处进行处理
	 * 
	 * @param pMsg 接收到的完整消息
	 * @return true 成功
	 * @return false 失败
	 */
	bool CMsgProto::OnMsgBack(const TransBaseMsg_t* pMsg) {
		E_MsgType type = pMsg->GetType();
		auto item = m_msgMap.find(type);
		if (item != m_msgMap.end()) {
			auto pNewMsg = std::make_shared<TransBaseMsg_t>(pMsg->GetType(), pMsg->to_string());
			HandleMsg(pNewMsg);
		}
		else
		{
			auto pNewMsg = std::make_shared<TransBaseMsg_t>(pMsg->GetType(), pMsg->to_string());
			HandleMsg(pNewMsg);
		}
		return false;
	}

/**
 * @brief 响应超时消息
 * 
 */
void CMsgProto::OnTimeOut() {
		if (m_startTime % 5 == 0 && 
			(GetStatus() != E_UI_ONLINE_STATUS::STATUS_OFFLINE)) {
			SendKeepAliveReq();
		}
		SourceServer::CSessManager::GetManager()->CheckSessConn();
		m_startTime++;
	}

/**
 * @brief 发送用户注册消息
 * 
 * @param strUserName 用户名
 * @param strPassword 用户密码
 * @param strNickName 用户昵称
 * @return true 
 * @return false 
 */
bool CMsgProto::SendUserRegister(const std::string strUserName, const std::string strPassword, const std::string strNickName)
{
	auto pSess = SourceServer::CSessManager::GetManager();

	UserRegisterReqMsg reqMsg;
	reqMsg.m_strUserName = strUserName;
	reqMsg.m_strPassword = strPassword;
	reqMsg.m_strNickName = strNickName;
	TransBaseMsg_t trans(reqMsg.GetMsgType(), reqMsg.ToString());
	pSess->SendMsg(&trans);
	return true;
}

/**
 * @brief 发送用户登录消息
 * 
 * @param strUserName 用户名
 * @param strPassword 密码
 * @return true 成功
 * @return false 失败
 */
bool CMsgProto::SendUserLogin(const std::string strUserName, const std::string strPassword) {
	auto pSess = SourceServer::CSessManager::GetManager();
	m_strUserName = strUserName;
	m_strPassword = strPassword;
	UserLoginReqMsg reqMsg;
	reqMsg.m_strUserName = m_strUserName;
	reqMsg.m_strPassword = strPassword;
	TransBaseMsg_t trans(reqMsg.GetMsgType(), reqMsg.ToString());
	pSess->SendMsg(&trans);
	m_bRecLoginRsp = false;
	return true;
}

/**
 * @brief 发送用户退出登录
 * 
 * @return true 
 * @return false 
 */
bool CMsgProto::SendUserLogout()
{
	m_eOnLineStatus = E_UI_ONLINE_STATUS::STATUS_OFFLINE;
	auto pSess = SourceServer::CSessManager::GetManager();
	
	UserLogoutReqMsg reqMsg;
	reqMsg.m_strUserName = m_strUserName;
	reqMsg.m_strPassword = m_strPassword;
	TransBaseMsg_t trans(reqMsg.GetMsgType(), reqMsg.ToString());
	pSess->SendMsg(&trans);
	return true;
}

/**
 * @brief 发送查找好友请求
 * 
 * @param strUserName 用户名
 * @param friendName 被查找的用户名
 * @param winHandle 回调的窗口Handle
 * @return true 
 * @return false 
 */
bool CMsgProto::SendFindFriendAsnyc(const std::string strUserName, const std::string friendName, HWND winHandle) {
	auto pSess = SourceServer::CSessManager::GetManager();
	FindFriendReqMsg reqMsg;
	reqMsg.m_strUserId = m_strUserName;
	reqMsg.m_strWantedName = friendName;
	TransBaseMsg_t trans(reqMsg.GetMsgType(), reqMsg.ToString());
	AddMap(E_MsgType::FindFriendRsp_Type, winHandle);
	return pSess->SendMsg(&trans);
}

/**
 * @brief 发送添加好友请求
 * 
 * @param strFriendId 好友ID
 * @param friendName 好友昵称
 * @param winHandle 回调的窗口的Handle
 * @return true 
 * @return false 
 */
bool CMsgProto::SendAddFriendAsnyc(const std::string strFriendId, const std::string friendName, HWND winHandle) {
	auto pSess = SourceServer::CSessManager::GetManager();
	{
		AddFriendSendReqMsg reqMsg;
		reqMsg.m_strUserId = m_strUserId;

		reqMsg.m_strFriendId = strFriendId;

		TransBaseMsg_t trans(reqMsg.GetMsgType(), reqMsg.ToString());
		return pSess->SendMsg(&trans);
	}
}

/**
 * @brief 处理服务器发送过来的消息
 * 
 * @param pOrgMsg 
 */
void CMsgProto::HandleMsg(const std::shared_ptr<TransBaseMsg_t> pOrgMsg) {
	switch (pOrgMsg->GetType()) {
	case E_MsgType::UserRegisterRsp_Type:
	{
		HandleUserRegisterRsp(pOrgMsg);
	}break;
	case E_MsgType::UserLoginRsp_Type: {
		HandleUserLoginRsp(pOrgMsg);
	}break;
	case E_MsgType::FindFriendRsp_Type: {
		HandleFindFriendRsp(pOrgMsg);
	}break;
	case E_MsgType::AddFriendRecvReq_Type: {
		HandleAddFriendRecvReq(pOrgMsg);
	}break;
	case E_MsgType::AddFriendNotifyReq_Type: {
		HandleAddFriendNotifyReq(pOrgMsg);
	}break;
	case E_MsgType::GetFriendListRsp_Type: {
		HandleGetFriendListRsp(pOrgMsg);
	}break;
	case E_MsgType::FriendChatReceiveTxtMsgReq_Type: {
		HandleRecvChatTxtReq(pOrgMsg);
	}break;
	case E_MsgType::NetRecoverReport_Type:
	{
		HandleNetRecoverReportMsg(pOrgMsg);
	}break;
	case E_MsgType::NetFailedReport_Type:
	{
		HandleNetFailedMsg(pOrgMsg);
		m_eOnLineStatus = E_UI_ONLINE_STATUS::STATUS_OFFLINE;
	}break;
	case E_MsgType::CreateGroupRsp_Type:
	{
		HandleCreateGroupRspMsg(pOrgMsg);
	}break;
	case E_MsgType::DestroyGroupRsp_Type:
	{
		HandleDestroyGroupRspMsg(pOrgMsg);
	}break;
	case E_MsgType::GetGroupListRsp_Type:
	{
		HandleGetGroupListRspMsg(pOrgMsg);
	}break;
	case E_MsgType::UserLogoutRsp_Type:
	{
		HandleLogoutRspMsg(pOrgMsg);
	}break;
	case E_MsgType::FindGroupRsp_Type:
	{
		HandleFindGroupRsp(pOrgMsg);
	}break;
	case E_MsgType::RecvGroupTextMsgReq_Type:
	{
		HandleRecvGroupTextMsgReq(pOrgMsg);
	}break;
	case E_MsgType::FriendSendFileMsgRsp_Type:
	{
		HandleFriendSendFileRsp(pOrgMsg);
	}break;
	case E_MsgType::FriendNotifyFileMsgReq_Type:
	{
		HandleFriendNotifyFileReq(pOrgMsg);
	}break;
	case E_MsgType::FriendRecvFileMsgReq_Type:
	{
		HandleFriendRecvFileReq(pOrgMsg);
	}break;
	case E_MsgType::UserKickOffReq_Type:
	{
		HandleUserKickOffReq(pOrgMsg);
	}break;
	case E_MsgType::FriendUnReadMsgNotifyReq_Type:
	{
		HandleFriendUnReadNotifyReq(pOrgMsg);
	}break;
	case E_MsgType::SendGroupTextMsgRsp_Type:
	{
		HandleSendGroupTextRspMsg(pOrgMsg);
	}break;
	case E_MsgType::FriendChatSendTxtMsgRsp_Type:
	{
		HandleSendChatTxtRsp(pOrgMsg);
	}break;
	case E_MsgType::UpdateFriendListNotifyReq_Type:
	{
		HandleUpdateFriendListNotifyReq(pOrgMsg);
	}break;
	case E_MsgType::GetFriendChatHistoryRsp_Type:
	{
		HandleGetFriendChatHistory(pOrgMsg);
	}break;
	case E_MsgType::GetGroupChatHistoryRsp_Type:
	{
		HandleGetGroupChatHistory(pOrgMsg);
	}break;
	default: {
		
	}break;
	}
}

/**
 * @brief 获取和某个好友的聊天消息，显示历史消息记录的时候使用
 * 
 * @param strFriendId 好友ID
 * @return CBuddyChatUiMsgVector 好友聊天消息数据
 */
CBuddyChatUiMsgVector CMsgProto::GetBuddyMsgList(const std::string strFriendId)
{
	auto item = m_friendMsgMap.find(strFriendId);
	if (item != m_friendMsgMap.end())
	{
		return item->second;
	}
	else
	{
		CBuddyChatUiMsgVector result;
		return result;
	}
}

//根据好友ID获取好友信息
/**
 * @brief 根据好友ID获取好友信息
 * 
 * @param strFriendId 好友ID
 * @return UserBaseInfo 好友基础信息
 */
UserBaseInfo CMsgProto::GetFriendInfoById(const std::string strFriendId)
{
	auto item = m_friendInfoMap.find(strFriendId);
	if (item != m_friendInfoMap.end())
	{
		return item->second;
	}
	UserBaseInfo result;
	result.m_strUserId = strFriendId;
	result.m_strUserName = strFriendId;
	result.m_strNickName = strFriendId;
	return result;
}

/**
 * @brief 获取用户的登录状态
 * 
 * @return E_UI_ONLINE_STATUS 用户登录状态
 */
E_UI_ONLINE_STATUS CMsgProto::GetStatus()
{
	return m_eOnLineStatus;
}

/**
 * @brief 处理收到的群组文本消息
 * 
 * @param pOrgMsg 
 */
void CMsgProto::HandleRecvGroupTextMsgReq(const std::shared_ptr<TransBaseMsg_t> pOrgMsg)
{
	RecvGroupTextMsgReqMsg reqMsg;
	if (reqMsg.FromString(pOrgMsg->to_string())) {

		{
			{
				C_UI_GroupMessage* pResult = new C_UI_GroupMessage();
				pResult->m_strSenderName = EncodeUtil::Utf8ToUnicode(GetFriendName(reqMsg.m_strSenderId));
				pResult->m_strSenderId = EncodeUtil::AnsiToUnicode(reqMsg.m_strSenderId);
				pResult->m_strContext = EncodeUtil::AnsiToUnicode(reqMsg.m_strContext);
				pResult->m_strGroupId = reqMsg.m_strGroupId;
				pResult->m_eType = E_UI_CONTENT_TYPE::CONTENT_TYPE_TEXT;
				pResult->m_stFontInfo = CoreToUi(reqMsg.m_stFontInfo);
				pResult->m_strMsgTime = EncodeUtil::AnsiToUnicode(reqMsg.m_strMsgTime);
				auto item = m_msgMap.find(reqMsg.GetMsgType());
				if (item != m_msgMap.end())
				{
					::PostMessage(item->second, FMG_MSG_RECV_GROUP_MSG, 0, (LPARAM)(pResult));
				}
			}
		}
		{
			RecvGroupTextMsgRspMsg rspMsg;
			rspMsg.m_strMsgId = reqMsg.m_strMsgId;
			rspMsg.m_strGroupId = reqMsg.m_strGroupId;
			rspMsg.m_strSenderId = reqMsg.m_strSenderId;
			rspMsg.m_strUserId = m_strUserId;
			auto pSess = SourceServer::CSessManager::GetManager();
			TransBaseMsg_t trans(rspMsg.GetMsgType(), rspMsg.ToString());
			pSess->SendMsg(&trans);
		}
	}
}

/**
 * @brief 处理未读消息通知
 * 
 * @param pOrgMsg 未读消息通知
 */
void CMsgProto::HandleFriendUnReadNotifyReq(const std::shared_ptr<TransBaseMsg_t> pOrgMsg)
{
	FriendUnReadNotifyReqMsg reqMsg;
	if(reqMsg.FromString(pOrgMsg->to_string()))
	{
		FriendUnReadNotifyRspMsg rspMsg;
		rspMsg.m_strMsgId = reqMsg.m_strMsgId;
		rspMsg.m_strUserId = reqMsg.m_strUserId;

		auto pSess = SourceServer::CSessManager::GetManager();
		TransBaseMsg_t trans(rspMsg.GetMsgType(), rspMsg.ToString());
		pSess->SendMsg(&trans);
	}
}


C_UI_BuddyInfo CoreToUI(const UserBaseInfo& userInfo)
{
	C_UI_BuddyInfo result;
	result.m_nBirthday = std::atoi(userInfo.m_strBirthDate.c_str());
	result.m_nFace = std::atoi(userInfo.m_strFaceId.c_str());
	result.m_nGender = std::atoi(userInfo.m_strGender.c_str());

	result.m_strUserId = userInfo.m_strUserId;
	result.m_strUserName = EncodeUtil::Utf8ToUnicode(userInfo.m_strUserName);


	if (userInfo.m_eOnlineState == CLIENT_ONLINE_TYPE::C_ONLINE_TYPE_ONLINE)
	{
		result.m_nStatus = E_UI_ONLINE_STATUS::STATUS_ONLINE;
	}
	else
	{
		result.m_nStatus = E_UI_ONLINE_STATUS::STATUS_OFFLINE;
	}
	result.m_strAccount = EncodeUtil::Utf8ToUnicode(userInfo.m_strUserName);
	result.m_strNickName = EncodeUtil::Utf8ToUnicode(userInfo.m_strNickName);
	result.m_nTeamIndex = 0;

	return result;
}
/**
 * @brief 处理获取好友列表的回复
 * 
 * @param pOrgMsg 好友列表回复
 */
void CMsgProto::HandleGetFriendListRsp(const std::shared_ptr<TransBaseMsg_t> pOrgMsg) {
	GetFriendListRspMsg rspMsg;
	{
		m_BuddyList.m_arrBuddyTeamInfo.clear();
	}
	if (rspMsg.FromString(pOrgMsg->to_string())) {
		int nTeamIndex = 0;
		for (auto teamItem : rspMsg.m_teamVec)
		{
			C_UI_BuddyTeamInfo * teamInfo = new C_UI_BuddyTeamInfo();
			teamInfo->m_strName = EncodeUtil::Utf8ToUnicode(teamItem.m_strTeamName);
			teamInfo->m_strTeamId = teamItem.m_strTeamId;
			for (auto userItem : teamItem.m_teamUsers)
			{
				C_UI_BuddyInfo * pBuddyInfo = new C_UI_BuddyInfo();
				m_friendInfoMap.insert({ userItem.m_strUserId,userItem });
				*pBuddyInfo = CoreToUI(userItem);
				pBuddyInfo->m_nTeamIndex = nTeamIndex;
				teamInfo->m_arrBuddyInfo.push_back(pBuddyInfo);
			}
			nTeamIndex++;
			m_BuddyList.m_arrBuddyTeamInfo.push_back(teamInfo);
		}
		//m_userManager.AddFriend()
		auto item = m_msgMap.find(rspMsg.GetMsgType());
		if (item != m_msgMap.end()) {
			::PostMessage(item->second, FMG_MSG_UPDATE_BUDDY_LIST, 0, 0);
		}
		{
			UpdateFriendListNotifyRspMsg notifyRsp;
			notifyRsp.m_strMsgId = m_strUserId;
			notifyRsp.m_strUserId = m_strUserId;
			auto pSess = SourceServer::CSessManager::GetManager();
			TransBaseMsg_t trans(notifyRsp.GetMsgType(), notifyRsp.ToString());
			pSess->SendMsg(&trans);
		}
	}

}

/**
 * @brief 处理添加好友请求通知消息
 * 
 * @param pOrgMsg 添加好友通知请求
 */
void CMsgProto::HandleAddFriendNotifyReq(const std::shared_ptr<TransBaseMsg_t> pOrgMsg) {
	AddFriendNotifyReqMsg reqMsg;
	if (reqMsg.FromString(pOrgMsg->to_string())) {
		auto item = m_msgMap.find(reqMsg.GetMsgType());
		if (item != m_msgMap.end())
		{
			C_WND_MSG_AddFriendNotifyRequest * pResult = new C_WND_MSG_AddFriendNotifyRequest();
			pResult->m_nRetryTimes = 0;
			strcpy_s(pResult->m_szUserName, reqMsg.m_strUserId.c_str());
			strcpy_s(pResult->m_szFriendName, reqMsg.m_strFriendId.c_str());
			strcpy_s(pResult->m_szFriendNickName, reqMsg.m_strFriendId.c_str());
			strcpy_s(pResult->m_szMsgId, reqMsg.m_strMsgId.c_str());
			pResult->m_eOption = reqMsg.m_option;
			::PostMessage(item->second, FMG_MSG_ADD_FRIEND_NOTIFY_REQ, 0, (LPARAM)pResult);
		}
	}
}


/**
 * @brief 处理收到的添加好友请求
 * 
 * @param pOrgMsg 
 */
void CMsgProto::HandleAddFriendRecvReq(const std::shared_ptr<TransBaseMsg_t> pOrgMsg) {
	AddFriendRecvReqMsg reqMsg;
	if (reqMsg.FromString(pOrgMsg->to_string())) {
		auto item = m_msgMap.find(reqMsg.GetMsgType());
		if (item != m_msgMap.end())
		{
			C_WND_MSG_OperateFriendResult * pResult = new C_WND_MSG_OperateFriendResult();
			pResult->m_nRetryTimes = 0;
			strcpy_s(pResult->m_szAccountName, reqMsg.m_strUserId.c_str());
			strcpy_s(pResult->m_szNickName, reqMsg.m_strUserId.c_str());
			strcpy_s(pResult->m_szMsgId, reqMsg.m_strMsgId.c_str());
			pResult->m_uCmd = E_UI_OPERATE_FRIEND::Apply;
			::PostMessage(item->second, FMG_MSG_RECVADDFRIENDREQUSET, 0, (LPARAM)pResult);
		}
	}
}

/**
 * @brief 同步发送添加好友请求回复
 * 
 * @param friendName 好友名称
 * @param msgId 消息ID
 * @param option 用户选择
 * @return true 
 * @return false 
 */
bool CMsgProto::SendAddFriendRecvAsnyc(const std::string friendName,const std::string msgId, const E_UI_OPERATE_FRIEND option) {
	auto pSess = SourceServer::CSessManager::GetManager();
	AddFriendRecvRspMsg reqMsg;
	reqMsg.m_strUserId = m_strUserName;
	reqMsg.m_strFriendId = friendName;
	reqMsg.m_strMsgId = msgId;
	if (option == E_UI_OPERATE_FRIEND::Agree)
	{
		reqMsg.m_option = E_FRIEND_OPTION::E_AGREE_ADD;
	}
	else
	{
		reqMsg.m_option = E_FRIEND_OPTION::E_REFUSE_ADD;
	}
	TransBaseMsg_t trans(reqMsg.GetMsgType(), reqMsg.ToString());
	return pSess->SendMsg(&trans);
}

/**
 * @brief 发送 发送文件 请求
 * 
 * @param strFriendId 好友ID
 * @param strFileName 文件名
 * @return true 
 * @return false 
 */
bool CMsgProto::SendFriendOnLineFile(const std::string strFriendId, std::string strFileName)
{
	auto pSess = SourceServer::CSessManager::GetManager();
	FriendSendFileMsgReqMsg reqMsg;
	reqMsg.m_strMsgId = "1234234";
	reqMsg.m_strFromId = m_strUserId;
	reqMsg.m_strToId = strFriendId;
	reqMsg.m_strFileName = strFileName;
	TransBaseMsg_t trans(reqMsg.GetMsgType(), reqMsg.ToString());
	return pSess->SendMsg(&trans);
}

/**
 * @brief 发送 接收文件 回复消息
 * 
 * @param reqMsg 
 * @param eOption 
 * @return true 
 * @return false 
 */
bool CMsgProto::SendFriendRecvFileRsp(const C_WND_MSG_FileRecvReq& reqMsg, const E_FRIEND_OPTION eOption)
{
	auto pSess = SourceServer::CSessManager::GetManager();
	FriendRecvFileMsgRspMsg rspMsg;
	rspMsg.m_strMsgId = reqMsg.m_szMsgId;
	rspMsg.m_strFromId = m_strUserId;
	rspMsg.m_strToId = reqMsg.m_szFriendId;
	rspMsg.m_eOption = eOption;
	rspMsg.m_strFileName = reqMsg.m_szFileName;
	TransBaseMsg_t trans(rspMsg.GetMsgType(), rspMsg.ToString());
	return pSess->SendMsg(&trans);
}

/**
 * @brief 发送  对方已经接收文件通知消息的 回复
 * 
 * @param reqMsg 
 * @return true 
 * @return false 
 */
bool CMsgProto::SendFriendNotifyFileRsp(const C_WND_MSG_FileNotifyReq& reqMsg)
{
	FriendNotifyFileMsgRspMsg rspMsg;
	rspMsg.m_strMsgId = reqMsg.m_szMsgId;
	auto pSess = SourceServer::CSessManager::GetManager();
	TransBaseMsg_t trans(rspMsg.GetMsgType(), rspMsg.ToString());
	return pSess->SendMsg(&trans);

}


/**
 * @brief 发送添加好友消息通知的回复
 * 
 * @param strMsgId 
 * @return true 
 * @return false 
 */
bool CMsgProto::SendAddFriendNotifyRsp(const std::string strMsgId) {
	
	auto pSess = SourceServer::CSessManager::GetManager();
	AddFriendNotifyRspMsg rspMsg;
	rspMsg.m_strMsgId = strMsgId;
	TransBaseMsg_t trans(rspMsg.GetMsgType(), rspMsg.ToString());
	return pSess->SendMsg(&trans); 
}

/**
 * @brief 发送查找群组请求
 * 
 * @param strGroupId 群组ID
 * @return true 
 * @return false 
 */
bool CMsgProto::SendFindGroupReq(const std::string strGroupId) {
	auto pSess = SourceServer::CSessManager::GetManager();
	FindGroupReqMsg reqMsg;
	reqMsg.m_strUserId = m_strUserId;
	reqMsg.m_strGroupName = strGroupId;
	TransBaseMsg_t trans(reqMsg.GetMsgType(), reqMsg.ToString());
	return pSess->SendMsg(&trans);
}

/**
 * @brief 处理查找群组回复消息
 * 
 * @param pOrgMsg 
 */
void CMsgProto::HandleFindGroupRsp(const std::shared_ptr<TransBaseMsg_t> pOrgMsg) {
	FindGroupRspMsg rspMsg;
	if (rspMsg.FromString(pOrgMsg->to_string())) {
		auto item = m_msgMap.find(rspMsg.GetMsgType());
	
		if (item != m_msgMap.end())
		{
			C_WND_MSG_FindFriendResult * pResult = new C_WND_MSG_FindFriendResult();
			pResult->m_nRetryTimes = 0;
			if (rspMsg.m_eErrCode == ERROR_CODE_TYPE::E_CODE_SUCCEED)
			{
				pResult->m_nResultCode = E_UI_ADD_FRIEND_RESULT::FIND_FRIEND_FOUND;
				strcpy_s(pResult->m_szAccountId, rspMsg.m_strGroupId.c_str());
				strcpy_s(pResult->m_szAccountName, rspMsg.m_strGroupName.c_str());
				strcpy_s(pResult->m_szNickName, rspMsg.m_strGroupOwner.c_str());
			}
			else
			{
				pResult->m_nResultCode = E_UI_ADD_FRIEND_RESULT::FIND_FRIEND_NOT_FOUND;
			}
			::PostMessage(item->second, FMG_MSG_FINDFREIND, 0, (LPARAM)pResult);
		}
	}
}

/**
 * @brief 处理发送文件的回复消息
 * 
 * @param pOrgMsg 
 */
void CMsgProto::HandleFriendSendFileRsp(const std::shared_ptr<TransBaseMsg_t> pOrgMsg)
{
	FriendSendFileMsgRspMsg rspMsg;
	if (rspMsg.FromString(pOrgMsg->to_string())) {
		auto item = m_msgMap.find(pOrgMsg->GetType());
		C_WND_MSG_FileSendRsp * pResult = new C_WND_MSG_FileSendRsp();
		pResult->m_eErrCode = rspMsg.m_eErrCode;
		strcpy_s(pResult->m_szMsgId, rspMsg.m_strMsgId.c_str());
		strcpy_s(pResult->m_szUserId, rspMsg.m_strFromId.c_str());
		strcpy_s(pResult->m_szFriendId, rspMsg.m_strToId.c_str());
		strcpy_s(pResult->m_szFileName, rspMsg.m_strFileName.c_str());
		if (item != m_msgMap.end())
		{
			::PostMessage(item->second, FMG_MSG_FRIEND_FILE_SEND_RSP, 0, (LPARAM)pResult);
		}
	}
}

/**
 * @brief 处理接收文件的请求消息
 * 
 * @param pOrgMsg 
 */
void CMsgProto::HandleFriendRecvFileReq(const std::shared_ptr<TransBaseMsg_t> pOrgMsg)
{
	FriendRecvFileMsgReqMsg reqMsg;
	if (reqMsg.FromString(pOrgMsg->to_string())) {
		auto item = m_msgMap.find(pOrgMsg->GetType());
		C_WND_MSG_FileRecvReq * pResult = new C_WND_MSG_FileRecvReq();
		strcpy_s(pResult->m_szMsgId, reqMsg.m_strMsgId.c_str());
		strcpy_s(pResult->m_szUserId, reqMsg.m_strFromId.c_str());
		strcpy_s(pResult->m_szFriendId, reqMsg.m_strFromId.c_str());
		strcpy_s(pResult->m_szFileName, reqMsg.m_strFileName.c_str());
		if (item != m_msgMap.end())
		{
			::PostMessage(item->second, FMG_MSG_FRIEND_FILE_RECV_REQ, 0, (LPARAM)pResult);
		}
	}
}

/**
 * @brief 处理文件被接收的通知消息
 * 
 * @param pOrgMsg 
 */
void CMsgProto::HandleFriendNotifyFileReq(const std::shared_ptr<TransBaseMsg_t> pOrgMsg)
{
	FriendNotifyFileMsgReqMsg reqMsg;
	if (reqMsg.FromString(pOrgMsg->to_string())) {
		auto item = m_msgMap.find(pOrgMsg->GetType());
		C_WND_MSG_FileNotifyReq * pResult = new C_WND_MSG_FileNotifyReq();
		strcpy_s(pResult->m_szMsgId, reqMsg.m_strMsgId.c_str());
		strcpy_s(pResult->m_szUserId, reqMsg.m_strFromId.c_str());
		strcpy_s(pResult->m_szFriendId, reqMsg.m_strFromId.c_str());
		strcpy_s(pResult->m_szFileName, reqMsg.m_strFileName.c_str());
		if (item != m_msgMap.end())
		{
			::PostMessage(item->second, FMG_MSG_FRIEND_FILE_NOTIFY_REQ, 0, (LPARAM)pResult);
		}
	}
}


/**
 * @brief 处理查找好友的回复消息
 * 
 * @param pOrgMsg 
 */
void CMsgProto::HandleFindFriendRsp(const std::shared_ptr<TransBaseMsg_t> pOrgMsg) {
	std::shared_ptr<FindFriendRspMsg> pMsg = std::make_shared<FindFriendRspMsg>();
	if (pMsg->FromString(pOrgMsg->to_string())) {
		auto item = m_msgMap.find(pMsg->GetMsgType());
		C_WND_MSG_FindFriendResult * pResult = new C_WND_MSG_FindFriendResult();
		if (item != m_msgMap.end())
		{
			
			pResult->m_nRetryTimes = 0;
			if (!pMsg->m_friendInfoVec.empty())
			{
				pResult->m_nResultCode = E_UI_ADD_FRIEND_RESULT::FIND_FRIEND_FOUND;
				strcpy_s(pResult->m_szAccountId, pMsg->m_friendInfoVec[0].m_strUserId.c_str());
				strcpy_s(pResult->m_szAccountName,pMsg->m_friendInfoVec[0].m_strUserName.c_str());
				strcpy_s(pResult->m_szNickName, pMsg->m_friendInfoVec[0].m_strNickName.c_str());
			}
			else
			{
				pResult->m_nResultCode = E_UI_ADD_FRIEND_RESULT::FIND_FRIEND_NOT_FOUND;
			}
			::PostMessage(item->second, FMG_MSG_FINDFREIND, 0, (LPARAM)pResult);
		}
		
	}
}

/**
 * @brief 处理用户注册的回复消息
 * 
 * @param pOrgMsg 
 */
void CMsgProto::HandleUserRegisterRsp(const std::shared_ptr<TransBaseMsg_t> pOrgMsg) {
	if (!m_bRecvRegisterRsp) {
		if (m_userRegisterRsp.FromString(pOrgMsg->to_string())) {
			m_bRecvRegisterRsp = true;
		}
	}
}

/**
 * @brief 处理用户登录的回复消息
 * 
 * @param pOrgMsg 
 */
void CMsgProto::HandleUserLoginRsp(const std::shared_ptr< TransBaseMsg_t> pOrgMsg) {
	if (!m_bRecLoginRsp)
	{
		if (m_userLoginRsp.FromString(pOrgMsg->to_string())) {
			m_bRecLoginRsp = true;
		}

		if (m_userLoginRsp.m_eErrCode == ERROR_CODE_TYPE::E_CODE_SUCCEED) {
			m_eOnLineStatus = E_UI_ONLINE_STATUS::STATUS_ONLINE;
			m_strUserId = m_userLoginRsp.m_strUserId;
			m_friendInfoMap.erase(m_userLoginRsp.m_strUserId);
			m_friendInfoMap.insert({ m_userLoginRsp.m_strUserId,m_userLoginRsp.m_userInfo });
		}

		EncodeUtil::Utf8ToUnicode(m_userLoginRsp.m_userInfo.m_strUserName.c_str(), m_loginResult.m_szAccountName,32);
		EncodeUtil::Utf8ToUnicode(m_userLoginRsp.m_userInfo.m_strNickName.c_str(), m_loginResult.m_szNickName, 32);
		EncodeUtil::Utf8ToUnicode(m_userLoginRsp.m_userInfo.m_strSignature.c_str(), m_loginResult.m_szSignature, 256);

		/*auto item = m_msgMap.find(pOrgMsg->GetType());
		if(item != m_msgMap.end())
		{
			if (m_userLoginRsp.Valid() && m_userLoginRsp.m_eErrCode == ERROR_CODE_TYPE::E_CODE_SUCCEED)
			{
				::PostMessage(item->second, FMG_MSG_LOGIN_RESULT, 0, (LPARAM)(0));
			}
			else
			{
				::PostMessage(item->second, FMG_MSG_LOGIN_RESULT, 0, (LPARAM)(LOGIN_FAILED));
			}
		}*/
	}
}

/**
 * @brief 处理添加到群组的回复消息
 * 
 * @param pOrgMsg 
 */
void CMsgProto::HandleAddToGroupRspMsg(const std::shared_ptr<TransBaseMsg_t> pOrgMsg)
{
	AddToGroupRspMsg rspMsg;
	if (rspMsg.FromString(pOrgMsg->to_string())) {

	}
}

/**
 * @brief 处理用户退出登录的消息回复
 * 
 * @param pOrgMsg 
 */
void CMsgProto::HandleLogoutRspMsg(const std::shared_ptr<TransBaseMsg_t> pOrgMsg)
{
	UserLogoutRspMsg rspMsg;
	if (rspMsg.FromString(pOrgMsg->to_string())) {
		if (rspMsg.m_eErrCode == ERROR_CODE_TYPE::E_CODE_SUCCEED) {
			m_eOnLineStatus = E_UI_ONLINE_STATUS::STATUS_OFFLINE;
		}
		std::error_code ec;
		SourceServer::CSessManager::GetManager()->Stop(ec);
	}
}


/**
 * @brief 设置服务器的IP和端口
 * 
 * @param strIp 服务器IP
 * @param port 服务器端口
 */
void CMsgProto::SetIpPort(const std::string strIp, const int port)
{
	auto pManager = SourceServer::CSessManager::GetManager();
	if (pManager)
	{
		pManager->SetServerIpPort(strIp, port);
	}
}

/**
 * @brief 开始连接
 * 
 */
void CMsgProto::StartConnect()
{
	auto pManager = SourceServer::CSessManager::GetManager();
	if (pManager)
	{
		pManager->StartConnect();
	}
}

/**
 * @brief 协议初始化
 * 
 */
void CMsgProto::Initialize() {
	SourceServer::CSessManager::Initialize();
	std::error_code ec;
	SourceServer::CSessManager::GetManager()->Start(ec);
	if (g_MsgProtoIns == nullptr)
	{
		g_MsgProtoIns = std::shared_ptr<CMsgProto>(new CMsgProto());
		g_MsgProtoIns->Init();
	}
}

/**
 * @brief 协议反初始化
 * 
 */
void CMsgProto::UnInstialize() {
	SourceServer::CSessManager::UnInstialize();
	g_MsgProtoIns->m_eOnLineStatus = E_UI_ONLINE_STATUS::STATUS_OFFLINE;
}

/**
 * @brief 获取协议的共享指针
 * 
 * @return std::shared_ptr<CMsgProto> 
 */
std::shared_ptr<CMsgProto> CMsgProto::GetInstance() {
	return g_MsgProtoIns;
}


/**
 * @brief 发送获取好友列表消息
 * 
 * @return true 
 * @return false 
 */
bool CMsgProto::GetFriendList() {
	auto pSess = SourceServer::CSessManager::GetManager();
	{
		GetFriendListReqMsg reqMsg;
		reqMsg.m_strUserId = m_strUserId;
		TransBaseMsg_t trans(reqMsg.GetMsgType(), reqMsg.ToString());
		return pSess->SendMsg(&trans);
	}
}

bool CMsgProto::SendRemoveFriend(const std::string strFriendId)
{
	auto pSess = SourceServer::CSessManager::GetManager();
	{
		RemoveFriendReqMsg reqMsg;
		reqMsg.m_strUserId = m_strUserId;
		reqMsg.m_strFriendId = strFriendId;
		TransBaseMsg_t trans(reqMsg.GetMsgType(), reqMsg.ToString());
		return pSess->SendMsg(&trans);
	}
}

/**
 * @brief 发送加入群组的请求消息
 * 
 * @param strGroupId 
 * @return true 
 * @return false 
 */
bool CMsgProto::SendAddToGroupReq(const std::string strGroupId)
{
	auto pSess = SourceServer::CSessManager::GetManager();
	{
		AddToGroupReqMsg reqMsg;
		reqMsg.m_strUserId = m_strUserId;
		reqMsg.m_strGroupId = strGroupId;
		TransBaseMsg_t trans(reqMsg.GetMsgType(), reqMsg.ToString());
		return pSess->SendMsg(&trans);
	}
}
bool CMsgProto::SendFileDataBeginReq(const std::string strFriendId, const std::string strFileName)
{
	auto pSess = SourceServer::CSessManager::GetManager();
	{
		FileSendDataBeginReq reqMsg;
		reqMsg.m_strUserId = m_strUserId;
		reqMsg.m_strFriendId = strFriendId;
		reqMsg.m_strFileName = strFileName;
		reqMsg.m_strMsgId = "123456";
		reqMsg.m_nFileId = rand();

		TransBaseMsg_t trans(reqMsg.GetMsgType(), reqMsg.ToString());
		return pSess->SendMsg(&trans);
	}
}

void CMsgProto::HandleFileSendDataRsp(const std::shared_ptr<TransBaseMsg_t> pOrgMsg)
{
	FileSendDataBeginRsp rspMsg;
	if (rspMsg.FromString(pOrgMsg->to_string())) {
		if (rspMsg.m_errCode == ERROR_CODE_TYPE::E_CODE_SUCCEED) {

		}
	}
}
/**
 * @brief 发送文本聊天
 * 
 * @param strFriendName 好友ID
 * @param strContext 文本消息
 * @return true 
 * @return false 
 */
bool CMsgProto::SendChatTxtMsg(const std::string strFriendName, const std::string strContext, const C_UI_FontInfo font)
{
	auto pSess = SourceServer::CSessManager::GetManager();
	{
		FriendChatSendTxtReqMsg reqMsg;
		reqMsg.m_strSenderId = m_strUserId;
		reqMsg.m_strReceiverId = strFriendName;
		reqMsg.m_strContext = strContext;
		reqMsg.m_fontInfo = UiToCore(font);
	
		TransBaseMsg_t trans(reqMsg.GetMsgType(), reqMsg.ToString());
		return pSess->SendMsg(&trans);
	}
}
ChatMsgElemVec UiToCore(const RichEditMsgList& richMsg)
{
	ChatMsgElemVec result;
	for (auto item : richMsg)
	{
		switch (item.m_eType)
		{
		case E_RichEditType::TEXT:
		{
			ChatMsgElem elem;
			elem.m_eType = CHAT_MSG_TYPE::E_CHAT_TEXT_TYPE;
			elem.m_strContext = EncodeUtil::UnicodeToAnsi(item.m_strContext);
			result.push_back(elem);
		}break;
		case E_RichEditType::FACE:
		{
			ChatMsgElem elem;
			elem.m_eType = CHAT_MSG_TYPE::E_CHAT_EMOJI_TYPE;
			elem.m_nFaceId = item.m_nFaceId;
			result.push_back(elem);
		}break;
		case E_RichEditType::IMAGE:
		{
			ChatMsgElem elem;
			elem.m_eType = CHAT_MSG_TYPE::E_CHAT_IMAGE_TYPE;
			elem.m_strImageName = EncodeUtil::UnicodeToAnsi(item.m_strImageName);
			result.push_back(elem);
		}break;
		default:
		{
			break;
		}

		}
	}
	return result;
}

RichEditMsgList CoreToUi(const ChatMsgElemVec& coreMsgVec)
{
	RichEditMsgList result;
	for (const auto item : coreMsgVec)
	{
		switch (item.m_eType)
		{
		case CHAT_MSG_TYPE::E_CHAT_TEXT_TYPE:
		{
			RichEditMsg_st elem;
			elem.m_eType = E_RichEditType::TEXT;
			elem.m_strContext = EncodeUtil::AnsiToUnicode(item.m_strContext);
			result.push_back(elem);
		}break;
		case CHAT_MSG_TYPE::E_CHAT_EMOJI_TYPE:
		{
			RichEditMsg_st elem;
			elem.m_eType = E_RichEditType::FACE;
			elem.m_nFaceId = item.m_nFaceId;
			result.push_back(elem);
		}break;
		case CHAT_MSG_TYPE::E_CHAT_IMAGE_TYPE:
		{
			RichEditMsg_st elem;
			elem.m_eType = E_RichEditType::IMAGE;
			elem.m_strImageName = EncodeUtil::AnsiToUnicode(item.m_strImageName);
			result.push_back(elem);
		}break;
		default:
			break;
		}
	}
	return result;
}

bool CMsgProto::SendChatTxtMsg(const std::string strFriendName, RichEditMsgList msgList, const C_UI_FontInfo font)
{
	auto pSess = SourceServer::CSessManager::GetManager();
	{
		FriendChatSendTxtReqMsg reqMsg;
		reqMsg.m_strSenderId = m_strUserId;
		reqMsg.m_strReceiverId = strFriendName;
		reqMsg.m_strContext = MsgElemVec(UiToCore(msgList));
		reqMsg.m_fontInfo = UiToCore(font);
		TransBaseMsg_t trans(reqMsg.GetMsgType(), reqMsg.ToString());
		pSess->SendMsg(&trans);
	}
	//for (const auto item : msgList)
	//{
	//	if (item.m_eType == E_RichEditType::IMAGE)
	//	{
	//		SendFileDataBeginReq(strFriendName, EncodeUtil::UnicodeToAnsi(item.m_strImageName));
	//	}
	//}
	return true;
}
void CMsgProto::HandleGetGroupChatHistory(const std::shared_ptr<TransBaseMsg_t> pOrgMsg)
{
	GetGroupChatHistoryRsp rspMsg;
	if (rspMsg.FromString(pOrgMsg->to_string())) {
		if (!rspMsg.m_msgHistory.empty())
		{
			m_friendChatLogMap.erase(rspMsg.m_strGroupId);
			LogMsgPair secondValue;
			secondValue.m_strFirstMsgId = rspMsg.m_msgHistory.begin()->m_strMsgId;
			secondValue.m_strLastMsgId = rspMsg.m_msgHistory.rbegin()->m_strMsgId;
			m_friendChatLogMap.insert({ rspMsg.m_strGroupId,secondValue });
		}
		for (const auto& item : (rspMsg.m_msgHistory))
		{
			auto WndItem = m_msgMap.find(rspMsg.GetMsgType());
			C_UI_GroupMessage * pResult = new C_UI_GroupMessage();
			pResult->m_strSenderName = EncodeUtil::Utf8ToUnicode(GetFriendName(m_strUserId));
			pResult->m_strSenderId = EncodeUtil::AnsiToUnicode(item.m_strSenderId);
			pResult->m_strContext = EncodeUtil::AnsiToUnicode(item.m_strContext);
			pResult->m_strGroupId = rspMsg.m_strGroupId;
			pResult->m_eType = E_UI_CONTENT_TYPE::CONTENT_TYPE_TEXT;
			pResult->m_stFontInfo = CoreToUi(item.m_fontInfo);
			pResult->m_strMsgTime = EncodeUtil::AnsiToUnicode(item.m_strMsgTime);
			auto item = m_msgMap.find(rspMsg.GetMsgType());
			if (item != m_msgMap.end())
			{
				::PostMessage(item->second, FMT_MSG_GROUP_CHAT_HISTORY, 0, (LPARAM)(pResult));
			}
		}
	}
}

bool CMsgProto::GetGroupHistoryReq(const std::string strGroupId, const HISTORY_DIRECTION eDirection)
{
	auto pSess = SourceServer::CSessManager::GetManager();
	if (pSess)
	{
		GetGroupChatHistoryReq reqMsg;
		reqMsg.m_eDirection = eDirection;
		switch (eDirection)
		{
		case HISTORY_DIRECTION::E_NEXT_MSG:
		{
			auto item = m_friendChatLogMap.find(strGroupId);
			if (item != m_friendChatLogMap.end()) {
				reqMsg.m_strChatMsgId = item->second.m_strLastMsgId;
			}
		}break;
		case HISTORY_DIRECTION::E_PREV_MSG:
		{
			auto item = m_friendChatLogMap.find(strGroupId);
			if (item != m_friendChatLogMap.end()) {
				reqMsg.m_strChatMsgId = item->second.m_strFirstMsgId;
			}
		}break;
		}
		reqMsg.m_strUserId = this->m_strUserId;
		reqMsg.m_strGroupId = strGroupId;
		reqMsg.m_strMsgId = "1234567";

		TransBaseMsg_t trans(reqMsg.GetMsgType(), reqMsg.ToString());
		return pSess->SendMsg(&trans);
	}
	return false;
}

bool CMsgProto::GetChatHistoryReq(const std::string strFriendId, const std::string strChatMsgId, const HISTORY_DIRECTION eDirection)
{
	auto pSess = SourceServer::CSessManager::GetManager();
	if (pSess)
	{
		GetFriendChatHistoryReq reqMsg;
		reqMsg.m_eDirection = eDirection;
		switch (eDirection)
		{
		case HISTORY_DIRECTION::E_NEXT_MSG:
		{
			auto item = m_friendChatLogMap.find(strFriendId);
			if (item != m_friendChatLogMap.end()) {
				reqMsg.m_strChatMsgId = item->second.m_strLastMsgId;
			}
		}break;
		case HISTORY_DIRECTION::E_PREV_MSG:
		{
			auto item = m_friendChatLogMap.find(strFriendId);
			if (item != m_friendChatLogMap.end()) {
				reqMsg.m_strChatMsgId = item->second.m_strFirstMsgId;
			}
		}break;
		}
		reqMsg.m_strUserId = this->m_strUserId;
		reqMsg.m_strFriendId = strFriendId;
		reqMsg.m_strMsgId = "1234567";

		TransBaseMsg_t trans(reqMsg.GetMsgType(), reqMsg.ToString());
		return pSess->SendMsg(&trans);
	}
	return false;
}
/**
 * @brief 发送收到文本消息之后的回复
 * 
 * @param strUserName 
 * @param strFriendName 
 * @param strMsgId 消息ID
 * @return true 
 * @return false 
 */
bool CMsgProto::SendRecvChatTxtRsp(const std::string strUserName, const std::string strFriendName, const std::string strMsgId)
{
	/*auto pSess = SourceServer::CSessManager::GetManager();
	{
		FriendChatRecvTxtRspMsg rspMsg;
		rspMsg.m_strMsgId = strMsgId;
		rspMsg.m_strFromId = strUserName;
		rspMsg.m_strToId = strFriendName;
		TransBaseMsg_t trans(rspMsg.GetMsgType(), rspMsg.ToString());
		return pSess->SendMsg(&trans);
	}*/
	return true;
}

void CMsgProto::HandleUpdateFriendListNotifyReq(const std::shared_ptr<TransBaseMsg_t> pOrgMsg)
{
	GetFriendList();
}

void CMsgProto::HandleGetFriendChatHistory(const std::shared_ptr<TransBaseMsg_t> pOrgMsg)
{
	GetFriendChatHistoryRsp rspMsg;
	if (rspMsg.FromString(pOrgMsg->to_string())) {
		

		if (!rspMsg.m_msgHistory.empty())
		{
			m_friendChatLogMap.erase(rspMsg.m_strFriendId);
			LogMsgPair secondValue;
			secondValue.m_strFirstMsgId = rspMsg.m_msgHistory.begin()->m_strChatMsgId;
			secondValue.m_strLastMsgId = rspMsg.m_msgHistory.rbegin()->m_strChatMsgId;
			m_friendChatLogMap.insert({rspMsg.m_strFriendId,secondValue});
		}
		for (const auto& item : (rspMsg.m_msgHistory))
		{
			auto WndItem = m_msgMap.find(rspMsg.GetMsgType());
			C_WND_MSG_BuddyTextMessage * pResult = new C_WND_MSG_BuddyTextMessage();
			pResult->m_uiMsg = CoreMsgToUiMsg(item);
			if (item.m_strSenderId == m_strUserId)
			{
				pResult->m_strSender = item.m_strReceiverId;
			}
			else
			{
				pResult->m_strSender = item.m_strSenderId;
			}

			if (WndItem != m_msgMap.end())
			{
				::PostMessage(WndItem->second, FMT_MSG_FRIEND_CHAT_HISTORY, 0, (LPARAM)pResult);
			}
		}
	}
}


/**
 * @brief 处理发送文本回复消息
 * 
 * @param pOrgMsg 
 */
void CMsgProto::HandleSendChatTxtRsp(const std::shared_ptr<TransBaseMsg_t> pOrgMsg)
{
	FriendChatSendTxtRspMsg rspMsg;
	if (rspMsg.FromString(pOrgMsg->to_string())) {

		{
			auto item = m_msgMap.find(rspMsg.GetMsgType());
			C_WND_MSG_BuddyTextMessage * pResult = new C_WND_MSG_BuddyTextMessage();
			pResult->m_strSender = rspMsg.m_chatMsg.m_strReceiverId;
			pResult->m_uiMsg = CoreMsgToUiMsg(rspMsg.m_chatMsg);

			if (item != m_msgMap.end())
			{
				::PostMessage(item->second, FMG_MSG_RECV_FRIEND_TEXT_MSG, 0, (LPARAM)pResult);
			}
		}

		{
			{
				auto item = m_friendMsgMap.find(rspMsg.m_chatMsg.m_strReceiverId);
				CBuddyChatUiMsg newMsg = this->CoreMsgToUiMsg(rspMsg.m_chatMsg);
				if (item != m_friendMsgMap.end())
				{
					item->second.push_back(newMsg);
				}
				else
				{
					CBuddyChatUiMsgVector msgVec;
					msgVec.push_back(newMsg);
					m_friendMsgMap.insert({ rspMsg.m_chatMsg.m_strReceiverId,msgVec });
				}
			}
		}

	}
}

/**
 * @brief 处理网络恢复消息，网络恢复的时候，重新登录
 * 
 * @param pOrgMsg 
 */
void CMsgProto::HandleNetRecoverReportMsg(const std::shared_ptr<TransBaseMsg_t> pOrgMsg) {
	auto item = m_msgMap.find(pOrgMsg->GetType());
	if (item != m_msgMap.end())
	{
		::PostMessage(item->second, FMG_MSG_NET_RECOVER, 0, (LPARAM)0);
	}
	//如果离线了，在网络恢复的时候，重新登录,此时用户名，用户密码，用户ID都不为空
	if ( (E_UI_ONLINE_STATUS::STATUS_OFFLINE == GetStatus()) &&
		 (!m_strUserName.empty()) &&
		 (!m_strPassword.empty()) &&
		 (!m_strUserId.empty()) )
	{
		SendUserLogin(m_strUserName, m_strPassword);
	}
}

/**
 * @brief 处理网络断开消息
 * 
 * @param pOrgMsg 
 */
void CMsgProto::HandleNetFailedMsg(const std::shared_ptr<TransBaseMsg_t> pOrgMsg)
{
	auto item = m_msgMap.find(pOrgMsg->GetType());
	if (item != m_msgMap.end())
	{
		::PostMessage(item->second,FMG_MSG_NET_FAILED, 0, (LPARAM)0);
	}
}


/**
 * @brief 好友聊天消息转为界面可以显示的消息
 * 
 * @param reqMsg TCP好友聊天消息
 * @return CBuddyChatUiMsg 界面好友聊天消息
 */
CBuddyChatUiMsg CMsgProto::CoreMsgToUiMsg(FriendChatMsg_s reqMsg)
{
	CBuddyChatUiMsg result;
	result.m_eMsgType = E_UI_CONTENT_TYPE::CONTENT_TYPE_TEXT;
	result.m_strContent = EncodeUtil::Utf8ToUnicode(reqMsg.m_strContext);
	result.m_strSenderName = EncodeUtil::Utf8ToUnicode(GetFriendName(reqMsg.m_strSenderId));
	result.m_strTime = EncodeUtil::Utf8ToUnicode(reqMsg.m_strMsgTime);
	result.m_strOtherInfo = EncodeUtil::Utf8ToUnicode(reqMsg.m_fontInfo.ToString());
	result.m_strChatMsgId = reqMsg.m_strChatMsgId;
	{
		result.m_stFontInfo.m_bBold = reqMsg.m_fontInfo.IsBold();
		result.m_stFontInfo.m_bItalic = reqMsg.m_fontInfo.IsItalic();
		result.m_stFontInfo.m_bUnderLine = reqMsg.m_fontInfo.IsUnderScore();
		result.m_stFontInfo.m_nSize = reqMsg.m_fontInfo.m_nFontSize;
		result.m_stFontInfo.ColorHexString(reqMsg.m_fontInfo.m_strFontColorHex);
		result.m_stFontInfo.m_strName = EncodeUtil::Utf8ToUnicode(reqMsg.m_fontInfo.m_strFontName);
	}
	return result;
}

/**
 * @brief 发送的聊天消息转为界面消息
 * 
 * @param reqMsg 
 * @return CBuddyChatUiMsg 
 */
//CBuddyChatUiMsg CMsgProto::CoreMsgToUiMsg(FriendChatSendTxtRspMsg reqMsg)
//{
//	CBuddyChatUiMsg result;
//	result.m_eMsgType = E_UI_CONTENT_TYPE::CONTENT_TYPE_TEXT;
//	result.m_strContent = EncodeUtil::Utf8ToUnicode(reqMsg.m_strContext);
//	result.m_strSenderName = EncodeUtil::Utf8ToUnicode(GetFriendName(reqMsg.m_strSenderId));
//	result.m_strTime = EncodeUtil::Utf8ToUnicode(reqMsg.m_strMsgTime);
//	{
//		result.m_stFontInfo.m_bBold = reqMsg.m_fontInfo.IsBold();
//		result.m_stFontInfo.m_bItalic = reqMsg.m_fontInfo.IsItalic();
//		result.m_stFontInfo.m_bUnderLine = reqMsg.m_fontInfo.IsUnderScore();
//		result.m_stFontInfo.m_nSize = reqMsg.m_fontInfo.m_nFontSize;
//		result.m_stFontInfo.ColorHexString(reqMsg.m_fontInfo.m_strFontColorHex);
//		result.m_stFontInfo.m_strName = EncodeUtil::Utf8ToUnicode(reqMsg.m_fontInfo.m_strFontName);
//	}
//	return result;
//}

/**
 * @brief 根据好友ID获取好友名称
 * 
 * @param strFriendId 
 * @return std::string 
 */
std::string CMsgProto::GetFriendName(const std::string strFriendId)
{
	auto item = m_friendInfoMap.find(strFriendId);
	if (item != m_friendInfoMap.end())
	{
		return item->second.m_strNickName;
	}
	else
	{
		return strFriendId;
	}
}
/**
 * @brief 处理好友文本消息
 * 
 * @param pOrgMsg 
 */
void CMsgProto::HandleRecvChatTxtReq(const std::shared_ptr<TransBaseMsg_t> pOrgMsg)
{

	FriendChatRecvTxtReqMsg reqMsg;
	if (reqMsg.FromString(pOrgMsg->to_string())) {
		auto item = m_msgMap.find(reqMsg.GetMsgType());
		C_WND_MSG_BuddyTextMessage * pResult = new C_WND_MSG_BuddyTextMessage();
		pResult->m_strSender = reqMsg.m_chatMsg.m_strSenderId;
		pResult->m_uiMsg = CoreMsgToUiMsg(reqMsg.m_chatMsg);

		if (item != m_msgMap.end())
		{
			::PostMessage(item->second,FMG_MSG_RECV_FRIEND_TEXT_MSG, 0, (LPARAM)pResult);
		}
		else
		{

		}
	}
}


/**
 * @brief 发送心跳请求消息
 * 
 * @return true 
 * @return false 
 */
bool CMsgProto::SendKeepAliveReq()
{
	auto pSess = SourceServer::CSessManager::GetManager();
	{
		KeepAliveReqMsg reqMsg;
		reqMsg.m_strClientId = m_strUserName;
		TransBaseMsg_t trans(reqMsg.GetMsgType(), reqMsg.ToString());
		return pSess->SendMsg(&trans);
	}
}

/**
 * @brief 发送添加分组请求
 * 
 * @param strUserName 
 * @param strTeamName 分组名
 * @return true 
 * @return false 
 */
bool CMsgProto::SendAddTeamReq(const std::string strTeamName)
{
	auto pSess = SourceServer::CSessManager::GetManager();
	{
		AddTeamReqMsg reqMsg;
		reqMsg.m_strUserId = m_strUserId;
		reqMsg.m_strTeamName = strTeamName;
		TransBaseMsg_t trans(reqMsg.GetMsgType(), reqMsg.ToString());
		return pSess->SendMsg(&trans);
	}
}

/**
 * @brief 发送删除分组请求
 * 
 * @param strTeamId 分组ID
 * @return true 
 * @return false 
 */
bool CMsgProto::SendRemoveTeamReq(const std::string strTeamId)
{
	auto pSess = SourceServer::CSessManager::GetManager();
	{
		RemoveTeamReqMsg reqMsg;
		reqMsg.m_strTeamId = strTeamId;
		reqMsg.m_strUserId = m_strUserId;
		TransBaseMsg_t trans(reqMsg.GetMsgType(), reqMsg.ToString());
		return pSess->SendMsg(&trans);
	}
}


/**
 * @brief 发送移动好友到分组的请求
 * 
 * @param strUserName 
 * @param strFriendName 
 * @param strSrcTeamId 
 * @param strDstTeamId 
 * @return true 
 * @return false 
 */
bool CMsgProto::MoveFriendToTeam(const std::string strUserName, const std::string strFriendName, const std::string strSrcTeamId, const std::string strDstTeamId)
{
	auto pSess = SourceServer::CSessManager::GetManager();
	{
		MoveFriendToTeamReqMsg reqMsg;
		reqMsg.m_strUserId = m_strUserId;
		reqMsg.m_strFriendId = strFriendName;
		reqMsg.m_strSrcTeamId = strSrcTeamId;
		reqMsg.m_strDstTeamId = strDstTeamId;
		TransBaseMsg_t trans(reqMsg.GetMsgType(), reqMsg.ToString());
		return pSess->SendMsg(&trans);
	}
}

/**
 * @brief 发送创建群组请求
 * 
 * @param strUserName 
 * @param strGroupName 
 * @return true 
 * @return false 
 */
bool CMsgProto::SendCreateGroupReq(const std::string strUserName, const std::string strGroupName)
{
	auto pSess = SourceServer::CSessManager::GetManager();
	{
		CreateGroupReqMsg reqMsg;
		reqMsg.m_strUserId = m_strUserId;
		reqMsg.m_strGroupName = strGroupName;
		TransBaseMsg_t trans(reqMsg.GetMsgType(), reqMsg.ToString());
		return pSess->SendMsg(&trans);
	}
}

/**
 * @brief 发送销毁群组请求
 * 
 * @param strUserName 
 * @param strGroupId 
 * @return true 
 * @return false 
 */
bool CMsgProto::SendDestroyGroupReq(const std::string strUserName, const std::string strGroupId) {
	auto pSess = SourceServer::CSessManager::GetManager();
	{
		DestroyGroupReqMsg reqMsg;
		reqMsg.m_strMsgId = m_strUserId;
		reqMsg.m_strGroupId = strGroupId;

		TransBaseMsg_t trans(reqMsg.GetMsgType(), reqMsg.ToString());
		return pSess->SendMsg(&trans);
	}
}

/**
 * @brief 处理创建群组回复消息
 * 
 * @param pOrgMsg 
 */
void CMsgProto::HandleCreateGroupRspMsg(const std::shared_ptr<TransBaseMsg_t> pOrgMsg)
{
	CreateGroupRspMsg rspMsg;
	if (rspMsg.FromString(pOrgMsg->to_string())) {
		C_WND_MSG_CreateNewGroupResult * pResult = new C_WND_MSG_CreateNewGroupResult;
		pResult->m_uError = 0;
		strcpy_s(pResult->m_szGroupName,rspMsg.m_strGroupName.c_str());
		strcpy_s(pResult->m_szGroupId, rspMsg.m_strGroupId.c_str());

		auto item = m_msgMap.find(pOrgMsg->GetType());

		if (item != m_msgMap.end())
		{
			::PostMessage(item->second, FMG_MSG_CREATE_NEW_GROUP_RESULT, 0, (LPARAM)pResult);
		}
	}
}

/**
 * @brief 处理销毁群组回复
 * 
 * @param pOrgMsg 
 */
void CMsgProto::HandleDestroyGroupRspMsg(const std::shared_ptr<TransBaseMsg_t> pOrgMsg)
{

}

bool CMsgProto::SendQuitFromGroupReq(const std::string strGroupId)
{
	auto pSess = SourceServer::CSessManager::GetManager();
	{
		QuitFromGroupReqMsg reqMsg;
		reqMsg.m_strUserId = m_strUserId;
		reqMsg.m_strGroupId = strGroupId;

		TransBaseMsg_t trans(reqMsg.GetMsgType(), reqMsg.ToString());
		return pSess->SendMsg(&trans);
	}
	return false;
}

/**
 * @brief 发送获取群组列表请求
 * 
 * @return true 
 * @return false 
 */
bool CMsgProto::SendGetGroupList()
{
	auto pSess = SourceServer::CSessManager::GetManager();
	{
		GetGroupListReqMsg reqMsg;
		reqMsg.m_strUserId = m_strUserId;

		TransBaseMsg_t trans(reqMsg.GetMsgType(), reqMsg.ToString());
		return pSess->SendMsg(&trans);
	}
}

/**
 * @brief 发送群组文本消息
 * 
 * @param strGroupId 
 * @param strContext 
 * @return true 
 * @return false 
 */
bool CMsgProto::SendGroupChatTextMsg(const std::string strGroupId, const std::string strContext, const C_UI_FontInfo font)
{
	SendGroupTextMsgReqMsg reqMsg;
	reqMsg.m_strSenderId = m_strUserId;
	reqMsg.m_strGroupId = strGroupId;
	reqMsg.m_strContext = strContext;
	reqMsg.m_stFontInfo = UiToCore(font);
	auto pSess = SourceServer::CSessManager::GetManager();
	TransBaseMsg_t trans(reqMsg.GetMsgType(), reqMsg.ToString());
	return pSess->SendMsg(&trans);
}

/**
 * @brief 处理获取群组列表请求
 * 
 * @param pOrgMsg 
 */
void CMsgProto::HandleGetGroupListRspMsg(const std::shared_ptr<TransBaseMsg_t> pOrgMsg)
{
	{
		GetGroupListRspMsg rspMsg;
		m_GroupList.m_arrGroupInfo.clear();
		int nIndex = 1;
		if (rspMsg.FromString(pOrgMsg->to_string())) {
			for (const auto groupItem : rspMsg.m_GroupList) {
				auto pInfo = new C_UI_GroupInfo();
				pInfo->m_strGroupId = groupItem.m_strGroupId;
				pInfo->m_strAccount = EncodeUtil::Utf8ToUnicode(groupItem.m_strGroupId);
				pInfo->m_strName = EncodeUtil::Utf8ToUnicode(groupItem.m_strGroupName);
				{

					for (const auto buddyItem : groupItem.m_GroupUsers) {
						C_UI_BuddyInfo * pBuddyInfo = new C_UI_BuddyInfo();
						*pBuddyInfo = CoreToUI(buddyItem);
			
						pBuddyInfo->m_uUserIndex = nIndex;
						nIndex++;
						pInfo->m_arrMember.push_back(pBuddyInfo);
						{
							m_friendInfoMap.erase(buddyItem.m_strUserId);
							m_friendInfoMap.insert({ buddyItem.m_strUserId,buddyItem });
						}
					}
				}
				m_GroupList.m_arrGroupInfo.push_back(pInfo);
			}
		}

		auto item = m_msgMap.find(rspMsg.GetMsgType());
		if (item != m_msgMap.end()) {
			::PostMessage(item->second, FMG_MSG_UPDATE_GROUP_LIST, 0, 0);
		}
	}
	{
		UpdateGroupListNotifyRspMsg notifyRsp;
		notifyRsp.m_strUserId = m_strUserId;
		TransBaseMsg_t trans(notifyRsp.GetMsgType(), notifyRsp.ToString());
		auto pSess = SourceServer::CSessManager::GetManager();
		pSess->SendMsg(&trans);
	}
}

/**
 * @brief 处理用户被其他登录踢出的消息请求
 * 
 * @param pOrgMsg 用户被踢消息请求
 */
void CMsgProto::HandleUserKickOffReq(const std::shared_ptr<TransBaseMsg_t> pOrgMsg)
{
	UserKickOffReqMsg reqMsg;
	if (reqMsg.FromString(pOrgMsg->to_string()))
	{
		UserKickOffRspMsg rspMsg;
		rspMsg.m_strMsgId = reqMsg.m_strMsgId;
		rspMsg.m_strUserId = reqMsg.m_strUserId;
		TransBaseMsg_t trans(rspMsg.GetMsgType(), rspMsg.ToString());
		auto pSess = SourceServer::CSessManager::GetManager();
		pSess->SendMsg(&trans);

		auto item = m_msgMap.find(reqMsg.GetMsgType());
		if (item != m_msgMap.end()) {
			::PostMessage(item->second, FMG_MSG_KICK_MSG, 0, 0);
		}
	}
}

/**
 * @brief 处理发送完群组消息的回复
 * 
 * @param pOrgMsg 
 */
void CMsgProto::HandleSendGroupTextRspMsg(const std::shared_ptr<TransBaseMsg_t> pOrgMsg)
{
	SendGroupTextMsgRspMsg rspMsg;
	if (rspMsg.FromString(pOrgMsg->to_string()))
	{
		C_UI_GroupMessage* pResult = new C_UI_GroupMessage();
		pResult->m_strSenderName = EncodeUtil::Utf8ToUnicode(GetFriendName(m_strUserId));
		pResult->m_strSenderId = EncodeUtil::AnsiToUnicode(rspMsg.m_strSenderId);
		pResult->m_strContext = EncodeUtil::AnsiToUnicode(rspMsg.m_strContext);
		pResult->m_strGroupId = rspMsg.m_strGroupId;
		pResult->m_eType = E_UI_CONTENT_TYPE::CONTENT_TYPE_TEXT;
		pResult->m_stFontInfo = CoreToUi(rspMsg.m_fontInfo);
		pResult->m_strMsgTime = EncodeUtil::AnsiToUnicode(rspMsg.m_strMsgTime);
		auto item = m_msgMap.find(rspMsg.GetMsgType());
		if (item != m_msgMap.end())
		{
			::PostMessage(item->second, FMG_MSG_RECV_GROUP_MSG, 0, (LPARAM)(pResult));
		}
	}
}