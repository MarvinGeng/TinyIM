#include "CImRobot.h"
#include <time.h>
#include <thread>
#include "CommonMsg.h"
#include "json11.hpp"

#include "../include/thirdparty/httpServer/client_http.hpp"
#include "SnowFlake.h"
using HttpClient = SimpleWeb::Client<SimpleWeb::HTTP>;
static std::shared_ptr<HttpClient> g_httpClient;
static SnowFlake m_msgIdUtil(4,4);
static std::string CreateMsgId() {
	return std::to_string(m_msgIdUtil.nextId());
}
CIMRobot::CIMRobot()
{

}

CIMRobot::~CIMRobot()
{
}
bool CIMRobot::loadConfig(const std::string strCfg)
{
	using namespace json11;
	std::string strErr;
	auto cfgJson = Json::parse(strCfg.c_str(), strErr, json11::COMMENTS);
	if (!strErr.empty())
	{
		return false;
	}

	if (cfgJson["HttpIp"].is_string())
	{
		m_strHttpIp = cfgJson["HttpIp"].string_value();
	}
	else
	{
		return false;
	}

	if (cfgJson["HttpPort"].is_number())
	{
		m_nHttpPort = cfgJson["HttpPort"].int_value();
	}
	else
	{
		return false;
	}

	if (cfgJson["UserName"].is_string())
	{
		m_strUserName = cfgJson["UserName"].string_value();
	}
	else
	{
		return false;
	}

	if (cfgJson["PassWord"].is_string())
	{
		m_strPassWord = cfgJson["PassWord"].string_value();
	}
	else
	{
		return false;
	}
	std::string strAddr = m_strHttpIp + ":" + std::to_string(m_nHttpPort);
	g_httpClient = std::make_shared<HttpClient>(strAddr);
	return true;
}

void CIMRobot::RegisterUser()
{
	std::cout << __FUNCTION__ << std::endl;
	UserRegisterRspMsg rspMsg;
	try {
		UserRegisterReqMsg reqMsg;
		reqMsg.m_strMsgId=CreateMsgId();
		reqMsg.m_strUserName = m_strUserName;
		reqMsg.m_strPassword = m_strPassWord;
		reqMsg.m_strNickName = m_strUserName;

		auto rsp = g_httpClient->request("POST", "/register_user", reqMsg.ToString());
		std::string strRsp = rsp->content.string();
		std::cout << strRsp << std::endl;
	}
	catch (const SimpleWeb::system_error& e) {
		std::cerr << "Client Req Error " << e.what() << std::endl;
	}
}

void CIMRobot::Run()
{
	RegisterUser();
	UserLogin();
	srand(static_cast<unsigned int>(time(nullptr)));
	while (true)
	{
		int nChoice = rand() % 12;
		switch (nChoice)
		{
		case 0:
		{
			SendMsg();
		}break;
		case 1:
		{
			GetRecvMsg();
		}break;
		case 2:
		{
			GetFriendList();
		}break;
		case 3:
		{
			UserLogin();
		}break;
		case 4:
		{
			SendFaceMsg();
		}break;
		case 5:
		{
			SendSingleImageMsg();
		}break;
		case 6:
		{
			RegisterUser();
		}break;
		case 7:
		{
			GetRandomUserName();
			SendFindFriendReq();
			SendAddFriendReq();
			GetRecvAddFriendReq();
			GetAddFriendNotify();
		}break;
		case 8:
		{
			SendFindFriendReq();
		}break;
		case 9:
		{
			GetRecvAddFriendReq();
		}break;
		case 10:
		{
			GetAddFriendNotify();
		}break;
		/*case 4:
		{
			GetGroupList();
		}break;
		case 5:
		{
			SendGroupMsg();
		}break;
		case 6:
		{
			GetRecvGroupMsg();
		}break;
		case 7:
		{
			AddFriendTeam();
		}break;
		case 8:
		{
			RemoveFriendTeam();
		}break;
		case 9:
		{
			MoveFriendToTeam();
		}break;
		case 10:
		{
			GetFriendHistoryLast();
		}break;
		case 11:
		{
			GetFriendHistoryNext();
		}break;
		case 12:
		{
			GetFriendHistoryPrev();
		}break;
		case 13:
		{
			GetFriendHistroyFirst();
		}break;
		case 14:
		{
			GetGroupHistoryLast();
		}break;
		case 15:
		{
			GetGroupHistoryNext();
		}break;
		case 16:
		{
			GetGroupHistroyFirst();
		}break;
		case 17:
		{
			SearchChatMsg();
		}break;
		case 18:
		{

		}break;*/
		
		default:
		{

		}break;
		std::this_thread::sleep_for(std::chrono::seconds(5));
		}
	}
	UserLogout();
}

void CIMRobot::UserLogin()
{
	std::cout << __FUNCTION__ << std::endl;
	UserLoginRspMsg rspMsg;
	try {
		UserLoginReqMsg reqMsg;
		reqMsg.m_strMsgId = "1234567890";
		reqMsg.m_strUserName = m_strUserName;
		reqMsg.m_strPassword = m_strPassWord;
		reqMsg.m_eNetType = CLIENT_NET_TYPE::C_NET_TYPE_WIFI;
		reqMsg.m_eOnlineType = CLIENT_ONLINE_TYPE::C_ONLINE_TYPE_ONLINE;
		reqMsg.m_eOsType = CLIENT_OS_TYPE::OS_TYPE_WINDOWS;
		auto rsp = g_httpClient->request("POST", "/user_login", reqMsg.ToString());
		std::string strRsp = rsp->content.string();
		std::cout << strRsp << std::endl;
		if (rspMsg.FromString(strRsp))
		{
			m_strUserId = rspMsg.m_strUserId;
		}
	}
	catch (const SimpleWeb::system_error& e) {
		std::cerr << "Client Req Error " << e.what() << std::endl;
	}
}
void CIMRobot::UserLogout()
{
	std::cout << __FUNCTION__ << std::endl;
	UserLogoutRspMsg rspMsg;
	try {
		UserLogoutReqMsg reqMsg;
		reqMsg.m_strMsgId = CreateMsgId();
		reqMsg.m_strUserName = m_strUserName;
		reqMsg.m_strPassword = m_strPassWord;
		reqMsg.m_eOsType = CLIENT_OS_TYPE::OS_TYPE_WINDOWS;


		auto rsp = g_httpClient->request("POST", "/user_logout", reqMsg.ToString());
		std::string strRsp = rsp->content.string();
		std::cout << strRsp << std::endl;
	}
	catch (const SimpleWeb::system_error& e) {
		std::cerr << "Client Req Error " << e.what() << std::endl;
	}
}
void CIMRobot::GetFriendList()
{
	std::cout << __FUNCTION__ << std::endl;
	GetFriendListRspMsg rspMsg;
	try {
		std::string strUrl = "/get_friend_list?UserId=" + m_strUserId;
		auto rsp = g_httpClient->request("GET", strUrl,"");
		std::string strRsp = rsp->content.string();
		std::cout << strRsp << std::endl;
		if (rspMsg.FromString(strRsp))
		{
			m_strFriendVec.clear();
			for (auto item : rspMsg.m_teamVec)
			{
				for (auto friItem : item.m_teamUsers)
				{
					m_strFriendVec.push_back(friItem.m_strUserId);
				}
			}
		}
	}
	catch (const SimpleWeb::system_error& e) {
		std::cerr << "Client Req Error " << e.what() << std::endl;
	}
}
void CIMRobot::SendMsg()
{
	std::cout << __FUNCTION__ << std::endl;
	FriendChatSendTxtReqMsg sendReq;
	FriendChatRecvTxtRspMsg sendRsp;
	ChatMsgElem elem;
	ChatMsgElemVec elemVec;
	elem.m_eType = CHAT_MSG_TYPE::E_CHAT_TEXT_TYPE;
	elem.m_strContext = "I'm a robot\"\",Create By DennisThink(https://www.dennisthink.com/)";
	elemVec.push_back(elem);
	for (auto friId : m_strFriendVec)
	{
		{
			try {
				sendReq.m_strMsgId = CreateMsgId();
				sendReq.m_strSenderId = m_strUserId;
				sendReq.m_strReceiverId = friId;
				sendReq.m_strContext = MsgElemVec(elemVec);
				auto rsp = g_httpClient->request("POST", "/send_friend_chat_text_msg", sendReq.ToString());
				std::string strRsp = rsp->content.string();
				std::cout << strRsp << std::endl;
				if (sendRsp.FromString(strRsp)) {

				}
			}
			catch (const SimpleWeb::system_error& e) {
				std::cerr << "Client Req Error " << e.what() << std::endl;
			}
		}
	}
}
void CIMRobot::GetRandomUserName()
{
	std::cout << __FUNCTION__ << std::endl;
	GetRandomUserRspMsg rspMsg;
	try {
		std::string strUrl = "/get_random_user_name?UserId=" + m_strUserId;
		auto rsp = g_httpClient->request("GET", strUrl, "");
		std::string strRsp = rsp->content.string();
		std::cout << strRsp << std::endl;
		if (rspMsg.FromString(strRsp))
		{
			if (rspMsg.m_errCode == ERROR_CODE_TYPE::E_CODE_SUCCEED)
			{
				m_strFriendName = rspMsg.m_strFriendName;
			}
		}
	}
	catch (const SimpleWeb::system_error& e) {
		std::cerr << "Client Req Error " << e.what() << std::endl;
	}
}

void CIMRobot::SendFindFriendReq()
{
	FindFriendReqMsg reqMsg;
	FindFriendRspMsg rspMsg;
	try {
		reqMsg.m_strMsgId = CreateMsgId();
		reqMsg.m_strUserId = m_strUserId;
		reqMsg.m_strWantedName = m_strFriendName;
		auto rsp = g_httpClient->request("POST", "/find_friend", reqMsg.ToString());
		std::string strRsp = rsp->content.string();
		std::cout << strRsp << std::endl;
		if (rspMsg.FromString(strRsp)) {
			if (rspMsg.m_eErrCode == ERROR_CODE_TYPE::E_CODE_SUCCEED)
			{
				if (!(rspMsg.m_friendInfoVec.empty()))
				{
					m_strFriendId = rspMsg.m_friendInfoVec.begin()->m_strUserId;
				}
			}
		}
	}
	catch (const SimpleWeb::system_error& e) {
		std::cerr << "Client Req Error " << e.what() << std::endl;
	}
}

void CIMRobot::SendAddFriendReq()
{
	AddFriendSendReqMsg reqMsg;
	AddFriendSendRspMsg rspMsg;
	try {
		reqMsg.m_strMsgId = CreateMsgId();
		reqMsg.m_strUserId = m_strUserId;
		reqMsg.m_strFriendId = m_strFriendId;
		auto rsp = g_httpClient->request("POST", "/add_friend_req", reqMsg.ToString());
		std::string strRsp = rsp->content.string();
		std::cout << strRsp << std::endl;
		if (rspMsg.FromString(strRsp)) {
			if (rspMsg.m_eErrCode == ERROR_CODE_TYPE::E_CODE_SUCCEED)
			{
			}
		}
	}
	catch (const SimpleWeb::system_error& e) {
		std::cerr << "Client Req Error " << e.what() << std::endl;
	}
}

void CIMRobot::GetRecvAddFriendReq()
{
	AddFriendRecvReqMsg reqMsg;
	{
		std::cout << __FUNCTION__ << std::endl;
		try {
			std::string strUrl = "/get_add_friend_require?UserId=" + m_strUserId;
			auto rsp = g_httpClient->request("GET", strUrl, "");
			std::string strRsp = rsp->content.string();
			std::cout << strRsp << std::endl;
			if (reqMsg.FromString(strRsp))
			{

			}
		}
		catch (const SimpleWeb::system_error& e) {
			std::cerr << "Client Req Error " << e.what() << std::endl;
		}
	}

	{
		AddFriendRecvRspMsg rspMsg;
		rspMsg.m_errCode = ERROR_CODE_TYPE::E_CODE_SUCCEED;
		rspMsg.m_errMsg = ErrMsg(rspMsg.m_errCode);
		rspMsg.m_strMsgId = reqMsg.m_strMsgId;
		rspMsg.m_strUserId = reqMsg.m_strUserId;
		rspMsg.m_strFriendId = reqMsg.m_strFriendId;
		rspMsg.m_option = rand() % 2 == 0 ? E_FRIEND_OPTION::E_AGREE_ADD : E_FRIEND_OPTION::E_REFUSE_ADD;
		try {
			auto rsp = g_httpClient->request("POST", "/on_add_friend_require", rspMsg.ToString());
		}
		catch (const SimpleWeb::system_error& e) {
			std::cerr << "Client Req Error " << e.what() << std::endl;
		}
	}
}

void CIMRobot::GetAddFriendNotify()
{
	std::cout << __FUNCTION__ << std::endl;
	AddFriendNotifyReqMsg reqMsg;
	try {
		std::string strUrl = "/get_add_friend_notify?UserId=" + m_strUserId;
		auto rsp = g_httpClient->request("GET", strUrl, "");
		std::string strRsp = rsp->content.string();
		std::cout << strRsp << std::endl;
		if (reqMsg.FromString(strRsp))
		{
		}
	}
	catch (const SimpleWeb::system_error& e) {
		std::cerr << "Client Req Error " << e.what() << std::endl;
	}
}

void CIMRobot::SendFaceMsg()
{
	std::cout << __FUNCTION__ << std::endl;
	FriendChatSendTxtReqMsg sendReq;
	FriendChatRecvTxtRspMsg sendRsp;
	ChatMsgElem elem;
	ChatMsgElemVec elemVec;
	elem.m_eType = CHAT_MSG_TYPE::E_CHAT_EMOJI_TYPE;
	elem.m_nFaceId = 1;
	elemVec.push_back(elem);
	for (auto friId : m_strFriendVec)
	{
		{
			try {
				sendReq.m_strMsgId = CreateMsgId();
				sendReq.m_strSenderId = m_strUserId;
				sendReq.m_strReceiverId = friId;
				sendReq.m_strContext = MsgElemVec(elemVec);
				auto rsp = g_httpClient->request("POST", "/send_friend_chat_text_msg", sendReq.ToString());
				std::string strRsp = rsp->content.string();
				std::cout << strRsp << std::endl;
				if (sendRsp.FromString(strRsp)) {

				}
			}
			catch (const SimpleWeb::system_error& e) {
				std::cerr << "Client Req Error " << e.what() << std::endl;
			}
		}
	}
}
void CIMRobot::SendSingleImageMsg()
{
	std::cout << __FUNCTION__ << std::endl;
	FriendChatSendTxtReqMsg sendReq;
	FriendChatRecvTxtRspMsg sendRsp;
	ChatMsgElem elem;
	ChatMsgElemVec elemVec;
	elem.m_eType = CHAT_MSG_TYPE::E_CHAT_IMAGE_TYPE;
#ifdef _WIN32
	elem.m_strImageName = R"(C:\Users\Public\Pictures\Sample Pictures\Desert.jpg)";
#else
	elem.m_strImageName = R"(C:\Users\Public\Pictures\Sample Pictures\Desert.jpg)";
#endif
	elemVec.push_back(elem);
	for (auto friId : m_strFriendVec)
	{
		{
			try {
				sendReq.m_strMsgId = CreateMsgId();
				sendReq.m_strSenderId = m_strUserId;
				sendReq.m_strReceiverId = friId;
				sendReq.m_strContext = MsgElemVec(elemVec);
				auto rsp = g_httpClient->request("POST", "/send_friend_chat_text_msg", sendReq.ToString());
				std::string strRsp = rsp->content.string();
				std::cout << strRsp << std::endl;
				if (sendRsp.FromString(strRsp)) {

				}
			}
			catch (const SimpleWeb::system_error& e) {
				std::cerr << "Client Req Error " << e.what() << std::endl;
			}
		}
	}
}
void CIMRobot::GetRecvMsg()
{
	std::cout << __FUNCTION__ << std::endl;
	FriendChatRecvTxtReqMsg reqMsg;
	try {
		for (int i = 0; i < 10; i++)
		{
			std::string strGetUrl = "/get_friend_chat_recv_text_msg?UserId=" + m_strUserId;
			auto rsp = g_httpClient->request("GET", strGetUrl, "");
			std::string strRsp = rsp->content.string();
			if (reqMsg.FromString(strRsp))
			{
				ChatMsgElemVec elemVec = MsgElemVec(reqMsg.m_chatMsg.m_strContext);
				std::cout << "Friend:" << reqMsg.m_chatMsg.m_strSenderId << " Send an Msg" << std::endl;
				FriendChatRecvTxtRspMsg rspMsg;
				rspMsg.m_strFriendId = reqMsg.m_chatMsg.m_strSenderId;
				rspMsg.m_strUserId = reqMsg.m_chatMsg.m_strReceiverId;
				rspMsg.m_strChatMsgId = reqMsg.m_chatMsg.m_strChatMsgId;
				rspMsg.m_strMsgId = CreateMsgId();
				auto rsp = g_httpClient->request("POST", "/on_friend_chat_recv_text_msg", rspMsg.ToString());
				std::string strRsp = rsp->content.string();
				std::cout << strRsp << std::endl;
			}
			else
			{
				std::cout << "No Friend Msg" << std::endl;
				break;
			}
		}
	}
	catch (const SimpleWeb::system_error& e) {
		std::cerr << "Client Req Error " << e.what() << std::endl;
	}
}

void CIMRobot::AddFriendTeam()
{
	std::cout << __FUNCTION__ << std::endl;
	AddTeamRspMsg rspMsg;
	try {
		AddTeamReqMsg reqMsg;
		reqMsg.m_strMsgId = CreateMsgId();
		reqMsg.m_strUserId = m_strUserId;
		reqMsg.m_strTeamName = m_strUserName+std::to_string(rand()%100);

		auto rsp = g_httpClient->request("POST", "/create_friend_team", reqMsg.ToString());
		std::string strRsp = rsp->content.string();
		std::cout << strRsp << std::endl;
	}
	catch (const SimpleWeb::system_error& e) {
		std::cerr << "Client Req Error " << e.what() << std::endl;
	}
}

void CIMRobot::RemoveFriendTeam()
{
	std::cout << __FUNCTION__ << std::endl;
	RemoveTeamRspMsg rspMsg;
	try {
		RemoveTeamReqMsg reqMsg;
		reqMsg.m_strMsgId = CreateMsgId();
		reqMsg.m_strUserId = m_strUserId;
		reqMsg.m_strTeamId = "1234568";

		auto rsp = g_httpClient->request("POST", "/destroy_friend_team", reqMsg.ToString());
		std::string strRsp = rsp->content.string();
		std::cout << strRsp << std::endl;
	}
	catch (const SimpleWeb::system_error& e) {
		std::cerr << "Client Req Error " << e.what() << std::endl;
	}
}

void CIMRobot::MoveFriendToTeam()
{
	if (!m_strFriendVec.empty())
	{
		std::cout << __FUNCTION__ << std::endl;
		MoveFriendToTeamRspMsg rspMsg;
		try {
			MoveFriendToTeamReqMsg reqMsg;
			reqMsg.m_strMsgId = CreateMsgId();
			reqMsg.m_strUserId = m_strUserId;
			reqMsg.m_strFriendId = *(m_strFriendVec.begin());
			reqMsg.m_strDstTeamId = "11111111";
			reqMsg.m_strSrcTeamId = "22222222";


			auto rsp = g_httpClient->request("POST", "/move_friend_to_team", reqMsg.ToString());
			std::string strRsp = rsp->content.string();
			std::cout << strRsp << std::endl;
		}
		catch (const SimpleWeb::system_error& e) {
			std::cerr << "Client Req Error " << e.what() << std::endl;
		}
	}

}


void CIMRobot::GetGroupList()
{
	GetGroupListReqMsg reqMsg;
	GetGroupListRspMsg rspMsg;
	try {
		reqMsg.m_strMsgId = "33333333";
		reqMsg.m_strUserId = m_strUserId;

		auto rsp = g_httpClient->request("POST", "/send_group_text_msg", reqMsg.ToString());
		std::string strRsp = rsp->content.string();
		std::cout << strRsp << std::endl;
		if (rspMsg.FromString(strRsp)) {
			m_strGroupVec.clear();
			for (const auto item : rspMsg.m_GroupList) {
				m_strGroupVec.push_back(item.m_strGroupId);
			}
		}
	}
	catch (const SimpleWeb::system_error& e) {
		std::cerr << "Client Req Error " << e.what() << std::endl;
	}
}

void CIMRobot::SendGroupMsg()
{
	SendGroupTextMsgRspMsg rspMsg;
	SendGroupTextMsgReqMsg reqMsg;
	for (const auto item : m_strGroupVec)
	{
		try {
			reqMsg.m_strMsgId = "33333333";
			reqMsg.m_strSenderId = m_strUserId;
			reqMsg.m_strGroupId = item;
			reqMsg.m_strContext = m_strUserId + " Send Msg To " + item;


			auto rsp = g_httpClient->request("POST", "/send_group_text_msg", reqMsg.ToString());
			std::string strRsp = rsp->content.string();
			std::cout << strRsp << std::endl;

		}
		catch (const SimpleWeb::system_error& e) {
			std::cerr << "Client Req Error " << e.what() << std::endl;
		}
	}
}

void CIMRobot::GetRecvGroupMsg()
{
	std::cout << __FUNCTION__ << std::endl;
	RecvGroupTextMsgReqMsg reqMsg;
	try {
		for (int i = 0; i < 10; i++)
		{
			auto rsp = g_httpClient->request("GET", "/get_recv_group_text_msg", "");
			std::string strRsp = rsp->content.string();
			if (reqMsg.FromString(strRsp))
			{
				RecvGroupTextMsgRspMsg rspMsg;
				rspMsg.m_strMsgId = reqMsg.m_strMsgId;
				auto rsp = g_httpClient->request("POST", "/on_recv_group_text_msg", rspMsg.ToString());
				std::string strRsp = rsp->content.string();
				std::cout << strRsp << std::endl;
			}
			else
			{
				break;
			}
		}
	}
	catch (const SimpleWeb::system_error& e) {
		std::cerr << "Client Req Error " << e.what() << std::endl;
	}
}

void CIMRobot::GetFriendHistroyFirst()
{
	std::cout << __FUNCTION__ << std::endl;
	GetFriendChatHistoryRsp rspMsg;
	try {
		GetFriendChatHistoryReq reqMsg;
		reqMsg.m_strMsgId = CreateMsgId();
		reqMsg.m_strUserId = m_strUserId;
		reqMsg.m_strFriendId = m_strUserId;
		reqMsg.m_eDirection = HISTORY_DIRECTION::E_FIRST_MSG;

		auto rsp = g_httpClient->request("POST", "/friend_chat_history", reqMsg.ToString());
		std::string strRsp = rsp->content.string();
		std::cout << strRsp << std::endl;
	}
	catch (const SimpleWeb::system_error& e) {
		std::cerr << "Client Req Error " << e.what() << std::endl;
	}
}

void CIMRobot::GetFriendHistoryLast()
{
	std::cout << __FUNCTION__ << std::endl;
	GetFriendChatHistoryRsp rspMsg;
	try {
		GetFriendChatHistoryReq reqMsg;
		reqMsg.m_strMsgId = CreateMsgId();
		reqMsg.m_strUserId = m_strUserId;
		reqMsg.m_strFriendId = m_strUserId;
		reqMsg.m_eDirection = HISTORY_DIRECTION::E_LAST_MSG;

		auto rsp = g_httpClient->request("POST", "/friend_chat_history", reqMsg.ToString());
		std::string strRsp = rsp->content.string();
		std::cout << strRsp << std::endl;
	}
	catch (const SimpleWeb::system_error& e) {
		std::cerr << "Client Req Error " << e.what() << std::endl;
	}
}

void CIMRobot::GetFriendHistoryPrev()
{
	std::cout << __FUNCTION__ << std::endl;
	GetFriendChatHistoryRsp rspMsg;
	try {
		GetFriendChatHistoryReq reqMsg;
		reqMsg.m_strMsgId = CreateMsgId();
		reqMsg.m_strUserId = m_strUserId;
		reqMsg.m_strFriendId = m_strUserId;
		reqMsg.m_eDirection = HISTORY_DIRECTION::E_PREV_MSG;

		auto rsp = g_httpClient->request("POST", "/friend_chat_history", reqMsg.ToString());
		std::string strRsp = rsp->content.string();
		std::cout << strRsp << std::endl;
	}
	catch (const SimpleWeb::system_error& e) {
		std::cerr << "Client Req Error " << e.what() << std::endl;
	}
}

void CIMRobot::GetFriendHistoryNext()
{
	std::cout << __FUNCTION__ << std::endl;
	GetFriendChatHistoryRsp rspMsg;
	try {
		GetFriendChatHistoryReq reqMsg;
		reqMsg.m_strMsgId = CreateMsgId();
		reqMsg.m_strUserId = m_strUserId;
		reqMsg.m_strFriendId = m_strUserId;
		reqMsg.m_eDirection = HISTORY_DIRECTION::E_NEXT_MSG;

		auto rsp = g_httpClient->request("POST", "/friend_chat_history", reqMsg.ToString());
		std::string strRsp = rsp->content.string();
		std::cout << strRsp << std::endl;
	}
	catch (const SimpleWeb::system_error& e) {
		std::cerr << "Client Req Error " << e.what() << std::endl;
	}
}



void CIMRobot::GetGroupHistroyFirst()
{
	std::cout << __FUNCTION__ << std::endl;
	GetGroupChatHistoryRsp rspMsg;
	try {
		GetGroupChatHistoryReq reqMsg;
		reqMsg.m_strMsgId = CreateMsgId();
		reqMsg.m_strUserId = m_strUserId;
		reqMsg.m_strGroupId = m_strUserId;
		reqMsg.m_eDirection = HISTORY_DIRECTION::E_FIRST_MSG;

		auto rsp = g_httpClient->request("POST", "/group_chat_history_req", reqMsg.ToString());
		std::string strRsp = rsp->content.string();
		std::cout << strRsp << std::endl;
	}
	catch (const SimpleWeb::system_error& e) {
		std::cerr << "Client Req Error " << e.what() << std::endl;
	}
}

void CIMRobot::GetGroupHistoryLast()
{
	std::cout << __FUNCTION__ << std::endl;
	GetGroupChatHistoryRsp rspMsg;
	try {
		GetGroupChatHistoryReq reqMsg;
		reqMsg.m_strMsgId = CreateMsgId();
		reqMsg.m_strUserId = m_strUserId;
		reqMsg.m_strGroupId = m_strUserId;
		reqMsg.m_eDirection = HISTORY_DIRECTION::E_LAST_MSG;

		auto rsp = g_httpClient->request("POST", "/group_chat_history_req", reqMsg.ToString());
		std::string strRsp = rsp->content.string();
		std::cout << strRsp << std::endl;
	}
	catch (const SimpleWeb::system_error& e) {
		std::cerr << "Client Req Error " << e.what() << std::endl;
	}
}

void CIMRobot::GetGroupHistoryPrev()
{
	std::cout << __FUNCTION__ << std::endl;
	GetGroupChatHistoryRsp rspMsg;
	try {
		GetGroupChatHistoryReq reqMsg;
		reqMsg.m_strMsgId = CreateMsgId();
		reqMsg.m_strUserId = m_strUserId;
		reqMsg.m_strGroupId = m_strUserId;
		reqMsg.m_eDirection = HISTORY_DIRECTION::E_PREV_MSG;

		auto rsp = g_httpClient->request("POST", "/group_chat_history_req", reqMsg.ToString());
		std::string strRsp = rsp->content.string();
		std::cout << strRsp << std::endl;
	}
	catch (const SimpleWeb::system_error& e) {
		std::cerr << "Client Req Error " << e.what() << std::endl;
	}
}

void CIMRobot::GetGroupHistoryNext()
{
	std::cout << __FUNCTION__ << std::endl;
	GetGroupChatHistoryRsp rspMsg;
	try {
		GetGroupChatHistoryReq reqMsg;
		reqMsg.m_strMsgId = CreateMsgId();
		reqMsg.m_strUserId = m_strUserId;
		reqMsg.m_strGroupId = m_strUserId;
		reqMsg.m_eDirection = HISTORY_DIRECTION::E_NEXT_MSG;

		auto rsp = g_httpClient->request("POST", "/group_chat_history_req", reqMsg.ToString());
		std::string strRsp = rsp->content.string();
		std::cout << strRsp << std::endl;
	}
	catch (const SimpleWeb::system_error& e) {
		std::cerr << "Client Req Error " << e.what() << std::endl;
	}
}

void CIMRobot::SearchChatMsg()
{
	std::cout << __FUNCTION__ << std::endl;
	SearchChatHistoryRsp rspMsg;
	try {
		SearchChatHistoryReq reqMsg;
		reqMsg.m_strMsgId = CreateMsgId();
		reqMsg.m_strUserId = m_strUserId;
		reqMsg.m_strSearchWord = "nihao";

		auto rsp = g_httpClient->request("POST", "/search_chat_msg", reqMsg.ToString());
		std::string strRsp = rsp->content.string();
		std::cout << strRsp << std::endl;
	}
	catch (const SimpleWeb::system_error& e) {
		std::cerr << "Client Req Error " << e.what() << std::endl;
	}
}