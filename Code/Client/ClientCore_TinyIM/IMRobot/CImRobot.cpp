#include "CImRobot.h"
#include <time.h>
#include <thread>
#include "CommonMsg.h"
#include "json11.hpp"
#include "../include/thirdparty/httpServer/client_http.hpp"
using HttpClient = SimpleWeb::Client<SimpleWeb::HTTP>;
static std::shared_ptr<HttpClient> g_httpClient;
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
void CIMRobot::Run()
{
	UserLogin();
	srand(static_cast<unsigned int>(time(nullptr)));
	while (true)
	{
		int nChoice = rand() % 3;
		switch (nChoice)
		{
		case 0:
		{
			SendMsg();
		}break;
		case 1:
		{
			//GetRecvMsg();
		}break;
		case 2:
		{
			GetFriendList();
		}break;
		case 3:
		{
			UserLogin();
		}break;
		default:
		{

		}break;
		std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}
	UserLogout();
}

void CIMRobot::UserLogin()
{
	if (m_strUserId.empty())
	{
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
}
void CIMRobot::UserLogout()
{
	UserLogoutRspMsg rspMsg;
	try {
		UserLogoutReqMsg reqMsg;
		reqMsg.m_strMsgId = "3333";
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
	FriendChatSendTxtReqMsg sendReq;
	FriendChatRecvTxtRspMsg sendRsp;
	for (auto friId : m_strFriendVec)
	{
		{
			try {
				sendReq.m_strMsgId = "123456";
				sendReq.m_strSenderId = m_strUserId;
				sendReq.m_strReceiverId = friId;
				sendReq.m_strContext = m_strUserId + " Send Msg To " + friId;
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
	FriendChatRecvTxtReqMsg reqMsg;
	try {
		for (int i = 0; i < 10; i++)
		{
			auto rsp = g_httpClient->request("GET", "/get_friend_chat_recv_text_msg", "");
			std::string strRsp = rsp->content.string();
			if (reqMsg.FromString(strRsp))
			{
				FriendChatRecvTxtRspMsg rspMsg;
				rspMsg.m_strFromId = reqMsg.m_strFromId;
				rspMsg.m_strToId = reqMsg.m_strToId;
				auto rsp = g_httpClient->request("POST", "/on_friend_chat_recv_text_msg", rspMsg.ToString());
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
