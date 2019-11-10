/**
 * @file CHttpServer.h
 * @author DennisMi (https://www.dennisthink.com/)
 * @brief 用来处理HTTP请求的类
 * @version 0.1
 * @date 2019-08-30
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#ifndef _DENNIS_THINK_C_HTTP_SERVER_H_
#define _DENNIS_THINK_C_HTTP_SERVER_H_
#include "asio_common.h"
#include <memory>
#include <map>
#include "server_http.hpp"
#include "CClientSess.h"
#include "SnowFlake.h"
using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
class RSP_SECOND {
public:
	time_t m_recvTime;
	std::shared_ptr<HttpServer::Response> m_response;
	
	RSP_SECOND(time_t curTime, std::shared_ptr<HttpServer::Response> rsp)
	{
		m_recvTime=curTime;
		m_response=rsp;
	}
};
using HTTP_RSP_SECOND = RSP_SECOND;
using HTTP_RSP_MAP = std::map<std::string, HTTP_RSP_SECOND>;
using HTTP_RSP_MAP_PAIR = std::pair<std::string, HTTP_RSP_SECOND>;

namespace ClientCore {
	class CMediumServer;
	class CHttpServer : public std::enable_shared_from_this<CHttpServer>
	{
	public:
		explicit CHttpServer(asio::io_service& ioService,CMediumServer* pServer) :m_httpServer(ioService),m_pServer(pServer),m_msgIdUtil(2,2) {

		}

		void Start(const int port)
		{
			Init(port);
			m_httpServer.start();
		}

		void Stop()
		{
			m_httpServer.stop();
		}

		void OnTimer();

		void On_UserRegisterRsp(const UserRegisterRspMsg& msg);

		void On_UserLoginRsp(const UserLoginRspMsg&  msg);

		void On_UserUnRegisterRsp(const UserUnRegisterRspMsg& msg);

		void On_UserLogoutRsp(const UserLogoutRspMsg& msg);

		//Friend 
		void On_FindFriendRsp(const FindFriendRspMsg& msg);

		void On_FriendChatSendTxtRsp(const FriendChatSendTxtRspMsg& msg);

		void On_AddFriendSendRspMsg(const AddFriendSendRspMsg& msg);

		void On_RemoveFriendRspMsg(const RemoveFriendRspMsg& msg);

		void On_SendFriendFileOnlineRspMsg(const FriendSendFileMsgRspMsg& msg);

		void On_GetFriendListRsp(const GetFriendListRspMsg& msg);
		
		void On_AddFriendTeamRsp(const AddTeamRspMsg& msg);

		void On_RemoveFriendTeamRsp(const RemoveTeamRspMsg& msg);

		void On_MoveFriendToTeamRsp(const MoveFriendToTeamRspMsg& msg);

		//Group
		void On_GetGroupListRsp(const GetGroupListRspMsg& msg);
		void On_FindGroupRsp(const FindGroupRspMsg& msg);

		void On_AddToGroupRsp(const AddToGroupRspMsg& msg);

		void On_CreateGroupRsp(const CreateGroupRspMsg& msg);

		void On_DestroyGroupRsp(const DestroyGroupRspMsg& msg);

		void On_SendGroupTextMsgRsp(const SendGroupTextMsgRspMsg& msg);
		//void On_UserLogoutRsp(const )

		static std::shared_ptr<spdlog::logger> ms_loger;

		std::string GenerateMsgId() {
			return std::to_string(m_msgIdUtil.nextId());
		}

		UserLoginReqMsg GetUserLoginReq(const std::string strUserName) const; 
	private:
		void Get_Version(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request);

		void Post_RegisterUser(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request);

		void Post_UnRegisterUser(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request);


		void Post_UserLogin(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request);

		void Post_UserLogout(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request);

		//Friend Begin
		void Post_FindFriendReq(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request);

		void Post_AddFriendReq(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request);

		void Get_RecvAddFriendReq(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request);

		void Post_RecvAddFriendRsp(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request);

		void Get_AddFriendNotify(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request);

		void Post_RemoveFriend(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request);

		void Post_FriendChatSendTxt(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request);

		void Get_FriendChatRecvTxtReq(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request);

		void Post_FriendChatRecvTxtRsp(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request);
		
		void Post_SendFileOnlineReq(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request);

		void Post_AddFriendTeamReq(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request);
		void Post_DestroyFriendTeamReq(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request);
		void Post_MoveFriendToTeamReq(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request);

		//void Get_SendFileOnlineRsp(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request);

		void Get_RecvFileOnlineReq(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request);

		void Post_RecvFileOnlineRsp(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request);
	
		void Get_SendFileOnlineNotifyReq(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request);

		void Post_SendFileOnlineNotifyRsp(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request);
		
		void Get_FriendListReq(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request);
		//Friend End

		//Group Msg

		void Get_GroupListReq(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request);
		void Post_FindGroupReq(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request);

		void Post_SendAddToGroupReq(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request);
		
		void Post_CreateGroupReq(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request);

		void Post_DestroyGroupReq(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request);

		void Post_SendGroupTextMsg(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request);

		void Get_RecvGroupTextMsgReq(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request);

		void Post_RecvGroupTextMsgRsp(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request);

		void Init(const int port);

		std::string GetHttpParamUserId(std::shared_ptr<HttpServer::Request> request);
	

		SnowFlake m_msgIdUtil;
		HttpServer m_httpServer;
		CMediumServer* m_pServer;
		std::map<CClientSess_SHARED_PTR, std::shared_ptr<HttpServer::Response>> m_clientSessResponseMap;
		HTTP_RSP_MAP m_httpRspMap;

		std::map<std::string, UserLoginReqMsg> m_userLoginMsgMap;
	};
}
#endif