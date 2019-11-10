﻿/**
 * @file CHttpServer.cpp
 * @author DennisMi (https://www.dennisthink.com/)
 * @brief HTTP请求处理的实现
 * @version 0.1
 * @date 2019-09-12
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include "CHttpServer.h"
#include "CMediumServer.h"
#include <time.h>
namespace ClientCore
{
	std::shared_ptr<spdlog::logger> CHttpServer::ms_loger;

	/**
	 * @brief 处理用户注册请求
	 * 
	 * @param response HTTP回复
	 * @param request HTTP请求
	 */
	void CHttpServer::Post_RegisterUser(std::shared_ptr<HttpServer::Response> response,std::shared_ptr<HttpServer::Request> request)
	{
		std::string  strReq = request->content.string();
		UserRegisterReqMsg reqMsg;
		if (reqMsg.FromString(strReq))
		{
			reqMsg.m_strMsgId = GenerateMsgId();
			auto pSendMsg = std::make_shared<TransBaseMsg_t>(reqMsg.GetMsgType(), reqMsg.ToString());
			auto pClientSess = m_pServer->CreateClientSess();
			if (pClientSess)
			{
				pClientSess->SendMsg(pSendMsg);
				m_httpRspMap.insert(HTTP_RSP_MAP_PAIR(reqMsg.m_strMsgId, HTTP_RSP_SECOND(time(nullptr), response)));
			}
			else
			{
				*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
			}
		}
		else
		{
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
		}
	}


	/**
	 * @brief 处理用户注销请求
	 * 
	 * @param response HTTP回复
	 * @param request HTTP请求
	 */
	void CHttpServer::Post_UnRegisterUser(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
	{
		std::string  strReq = request->content.string();
		UserUnRegisterReqMsg reqMsg;
		if (reqMsg.FromString(strReq))
		{
			reqMsg.m_strMsgId = GenerateMsgId();
			auto pSendMsg = std::make_shared<TransBaseMsg_t>(reqMsg.GetMsgType(), reqMsg.ToString());
			auto pClientSess = m_pServer->CreateClientSess();
			if (pClientSess)
			{
				pClientSess->SendMsg(pSendMsg);
				m_httpRspMap.insert(HTTP_RSP_MAP_PAIR(reqMsg.m_strMsgId, HTTP_RSP_SECOND(time(nullptr), response)));
			}
			else
			{
				*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
			}
		}
		else
		{
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
		}
	}

	/**
	 * @brief 处理用户登录的HTTP请求
	 * 
	 * @param response HTTP回复
	 * @param request HTTP请求
	 */
	void CHttpServer::Post_UserLogin(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
	{
		std::string  strReq = request->content.string();
		UserLoginReqMsg reqMsg;
		if (reqMsg.FromString(strReq))
		{
			reqMsg.m_strMsgId = GenerateMsgId();
			auto pSendMsg = std::make_shared<TransBaseMsg_t>(reqMsg.GetMsgType(), reqMsg.ToString());
			auto pClientSess = m_pServer->GetClientSess(m_pServer->GetUserId(reqMsg.m_strUserName));
			if (pClientSess)
			{
				pClientSess = m_pServer->CreateClientSess();
				if (pClientSess)
				{
					pClientSess->SendMsg(pSendMsg);
					m_httpRspMap.insert(HTTP_RSP_MAP_PAIR(reqMsg.m_strMsgId, HTTP_RSP_SECOND(time(nullptr), response)));
				}
				else
				{
					*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
				}
			}
			m_userLoginMsgMap.erase(reqMsg.m_strUserName);
			m_userLoginMsgMap.insert({ reqMsg.m_strUserName,reqMsg });
		}
		else
		{
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
		}
	}

	/**
	 * @brief 处理用户登出的HTTP请求
	 * 
	 * @param response 
	 * @param request 
	 */
	void CHttpServer::Post_UserLogout(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
	{
		std::string  strReq = request->content.string();
		UserLogoutReqMsg reqMsg;
		if (reqMsg.FromString(strReq))
		{
			auto pSendMsg = std::make_shared<TransBaseMsg_t>(reqMsg.GetMsgType(), reqMsg.ToString());
			auto pClientSess = m_pServer->GetClientSess(m_pServer->GetUserId(reqMsg.m_strUserName));
			if (pClientSess)
			{
				pClientSess->SendMsg(pSendMsg);
				m_httpRspMap.insert(HTTP_RSP_MAP_PAIR(reqMsg.m_strMsgId, HTTP_RSP_SECOND(time(nullptr), response)));
			}
			else
			{
				*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
			}
		}
		else
		{
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
		}
	}

	/**
	 * @brief 处理接收到好友文本消息的HTTP请求
	 * 
	 * @param response HTTP回复
	 * @param request HTTP请求
	 */
	void CHttpServer::Get_FriendChatRecvTxtReq(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
	{
		FriendChatRecvTxtReqMsg reqMsg;
		auto msgUtil = m_pServer->GetMsgPersisUtil();
		if (msgUtil && msgUtil->Get_FriendChatRecvTxtReqMsg(reqMsg))
		{
			std::string strContent = reqMsg.ToString();
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << strContent.length() << "\r\n\r\n"
				<< strContent;
		}
		else
		{
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n";
		}
	}

	/**
	 * @brief 回复已经接收到好友文本消息的HTTP请求
	 * 
	 * @param response 
	 * @param request 
	 */
	void CHttpServer::Post_FriendChatRecvTxtRsp(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
	{
		std::string  strReq = request->content.string();
		FriendChatRecvTxtRspMsg reqMsg;
		if (reqMsg.FromString(strReq))
		{
			auto pSendMsg = std::make_shared<TransBaseMsg_t>(reqMsg.GetMsgType(), reqMsg.ToString());
			auto pClientSess = m_pServer->GetClientSess(reqMsg.m_strFromId);
			if (pClientSess)
			{
				pClientSess->SendMsg(pSendMsg);
			}
			else
			{
				*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
			}
		}
		else
		{
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
		}
	}


	/**
	 * @brief 处理添加好友的HTTP请求
	 * 
	 * @param response 
	 * @param request 
	 */
	void CHttpServer::Post_AddFriendReq(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
	{
		std::string  strReq = request->content.string();
		AddFriendSendReqMsg reqMsg;
		if (reqMsg.FromString(strReq))
		{
			reqMsg.m_strMsgId = GenerateMsgId();
			auto pSendMsg = std::make_shared<TransBaseMsg_t>(reqMsg.GetMsgType(), reqMsg.ToString());
			auto pClientSess = m_pServer->GetClientSess(reqMsg.m_strUserId);
			if (pClientSess)
			{
				pClientSess->SendMsg(pSendMsg);
				m_httpRspMap.insert(HTTP_RSP_MAP_PAIR(reqMsg.m_strMsgId, HTTP_RSP_SECOND(time(nullptr), response)));
			}
			else
			{
				*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
			}
		}
		else
		{
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
		}
	}

	/**
	 * @brief 处理添加到群组的HTTP请求
	 * 
	 * @param response 
	 * @param request 
	 */
	void CHttpServer::Post_SendAddToGroupReq(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
	{
		std::string  strReq = request->content.string();
		AddToGroupReqMsg reqMsg;
		if (reqMsg.FromString(strReq))
		{
			reqMsg.m_strMsgId = GenerateMsgId();
			auto pSendMsg = std::make_shared<TransBaseMsg_t>(reqMsg.GetMsgType(), reqMsg.ToString());
			auto pClientSess = m_pServer->GetClientSess(reqMsg.m_strUserId);
			if (pClientSess)
			{
				pClientSess->SendMsg(pSendMsg);
				m_httpRspMap.insert(HTTP_RSP_MAP_PAIR(reqMsg.m_strMsgId, HTTP_RSP_SECOND(time(nullptr), response)));
			}
			else
			{
				*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
			}
		}
		else
		{
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
		}

	}


	/**
	 * @brief 处理查找群组的HTTP请求
	 * 
	 * @param response 
	 * @param request 
	 */
	void CHttpServer::Post_FindGroupReq(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
	{
		std::string  strReq = request->content.string();
		FindGroupReqMsg reqMsg;
		if (reqMsg.FromString(strReq))
		{
			reqMsg.m_strMsgId = GenerateMsgId();
			auto pSendMsg = std::make_shared<TransBaseMsg_t>(reqMsg.GetMsgType(), reqMsg.ToString());
			auto pClientSess = m_pServer->GetClientSess(reqMsg.m_strUserId);
			if (pClientSess)
			{
				pClientSess->SendMsg(pSendMsg);
				m_httpRspMap.insert(HTTP_RSP_MAP_PAIR(reqMsg.m_strMsgId, HTTP_RSP_SECOND(time(nullptr), response)));
			}
			else
			{
				*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
			}
		}
		else
		{
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
		}
	}

	/**
	 * @brief 处理创建群组的HTTP请求
	 * 
	 * @param response 
	 * @param request 
	 */
	void CHttpServer::Post_CreateGroupReq(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
	{
		std::string  strReq = request->content.string();
		CreateGroupReqMsg reqMsg;
		if (reqMsg.FromString(strReq))
		{
			reqMsg.m_strMsgId = GenerateMsgId();
			auto pSendMsg = std::make_shared<TransBaseMsg_t>(reqMsg.GetMsgType(), reqMsg.ToString());
			auto pClientSess = m_pServer->GetClientSess(reqMsg.m_strUserId);
			if (pClientSess)
			{
				pClientSess->SendMsg(pSendMsg);
				m_httpRspMap.insert(HTTP_RSP_MAP_PAIR(reqMsg.m_strMsgId, HTTP_RSP_SECOND(time(nullptr), response)));
			}
			else
			{
				*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
			}
		}
		else
		{
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
		}
	}

	/**
	 * @brief 处理解散群组的HTTP请求
	 * 
	 * @param response 
	 * @param request 
	 */
	void CHttpServer::Post_DestroyGroupReq(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
	{
		std::string  strReq = request->content.string();
		DestroyGroupReqMsg reqMsg;
		if (reqMsg.FromString(strReq))
		{
			reqMsg.m_strMsgId = GenerateMsgId();
			auto pSendMsg = std::make_shared<TransBaseMsg_t>(reqMsg.GetMsgType(), reqMsg.ToString());
			auto pClientSess = m_pServer->GetClientSess(reqMsg.m_strUserId);
			if (pClientSess)
			{
				pClientSess->SendMsg(pSendMsg);
				m_httpRspMap.insert(HTTP_RSP_MAP_PAIR(reqMsg.m_strMsgId, HTTP_RSP_SECOND(time(nullptr), response)));
			}
			else
			{
				*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
			}
		}
		else
		{
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
		}
	}

	/**
	 * @brief 处理发送群组文本消息的HTTP请求
	 * 
	 * @param response 
	 * @param request 
	 */
	void CHttpServer::Post_SendGroupTextMsg(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
	{
		std::string  strReq = request->content.string();
		SendGroupTextMsgReqMsg reqMsg;
		if (reqMsg.FromString(strReq))
		{
			reqMsg.m_strMsgId = GenerateMsgId();
			auto pSendMsg = std::make_shared<TransBaseMsg_t>(reqMsg.GetMsgType(), reqMsg.ToString());
			auto pClientSess = m_pServer->GetClientSess(reqMsg.m_strSenderId);
			if (pClientSess)
			{
				pClientSess->SendMsg(pSendMsg);
				m_httpRspMap.insert(HTTP_RSP_MAP_PAIR(reqMsg.m_strMsgId, HTTP_RSP_SECOND(time(nullptr), response)));
			}
			else
			{
				*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
			}
		}
		else
		{
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
		}
	}


	/**
	 * @brief 处理好友聊天发送文本消息的HTTP请求
	 * 
	 * @param response 
	 * @param request 
	 */
	void CHttpServer::Post_FriendChatSendTxt(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
	{
		std::string  strReq = request->content.string();
		FriendChatSendTxtReqMsg reqMsg;
		if (reqMsg.FromString(strReq))
		{
			reqMsg.m_strMsgId = GenerateMsgId();
			auto pSendMsg = std::make_shared<TransBaseMsg_t>(reqMsg.GetMsgType(), reqMsg.ToString());
			auto pClientSess = m_pServer->GetClientSess(reqMsg.m_strSenderId);
			if (pClientSess)
			{
				pClientSess->SendMsg(pSendMsg);
				m_httpRspMap.insert(HTTP_RSP_MAP_PAIR(reqMsg.m_strMsgId, HTTP_RSP_SECOND(time(nullptr), response)));
			}
			else
			{
				*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
			}
		}
		else
		{
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
		}
	}

	/**
	 * @brief 处理查找好友的HTTP请求
	 * 
	 * @param response 
	 * @param request 
	 */
	void CHttpServer::Post_FindFriendReq(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
	{
		std::string  strReq = request->content.string();
		FindFriendReqMsg reqMsg;
		if (reqMsg.FromString(strReq))
		{
			auto pSendMsg = std::make_shared<TransBaseMsg_t>(reqMsg.GetMsgType(), reqMsg.ToString());
			auto pClientSess = m_pServer->GetClientSess(reqMsg.m_strUserId);
			if (pClientSess)
			{
				pClientSess->SendMsg(pSendMsg);
				m_httpRspMap.insert(HTTP_RSP_MAP_PAIR(reqMsg.m_strMsgId, HTTP_RSP_SECOND(time(nullptr), response)));
			}
			else
			{
				*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
			}
		}
		else
		{
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";

		}
	}

	/**
	 * @brief 从HTTP接口获取收到的群组文本消息
	 * 
	 * @param response 
	 * @param request 
	 */
	void CHttpServer::Get_RecvGroupTextMsgReq(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
	{
		LOG_INFO(ms_loger, " {} {}", __FUNCTION__, __FILENAME__);
		RecvGroupTextMsgReqMsg reqMsg;
		auto msgUtil = m_pServer->GetMsgPersisUtil();
		if (msgUtil && msgUtil->Get_RecvGroupTextMsgReqMsg(reqMsg))
		{
			std::string strContent = reqMsg.ToString();
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << strContent.length() << "\r\n\r\n"
				<< strContent;
		}
		else
		{
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
		}
	}

	void CHttpServer::Get_FriendListReq(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
	{
		LOG_INFO(ms_loger, " {} {}", __FUNCTION__, __FILENAME__);
		std::string strUserId = GetHttpParamUserId(request);
		if (strUserId.length() > 0)
		{
			GetFriendListReqMsg reqMsg;
			reqMsg.m_strUserId = strUserId;
			reqMsg.m_strMsgId = GenerateMsgId();
			auto pSendMsg = std::make_shared<TransBaseMsg_t>(reqMsg.GetMsgType(), reqMsg.ToString());
			auto pClientSess = m_pServer->GetClientSess(reqMsg.m_strUserId);
			if (pClientSess)
			{
				pClientSess->SendMsg(pSendMsg);
				m_httpRspMap.insert(HTTP_RSP_MAP_PAIR(reqMsg.m_strMsgId,HTTP_RSP_SECOND(time(nullptr), response)));
			}
			else
			{
				*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n"<<"";
			}

		}
		else
		{
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
		}
	}
	/**
	 * @brief 发送已经收到群组文本消息的回复
	 * 
	 * @param response 
	 * @param request 
	 */
	void CHttpServer::Post_RecvGroupTextMsgRsp(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
	{
		LOG_INFO(ms_loger, " {} {}", __FUNCTION__, __FILENAME__);
		std::string  strReq = request->content.string();
		RecvGroupTextMsgRspMsg reqMsg;
		if (reqMsg.FromString(strReq))
		{
			reqMsg.m_strMsgId = GenerateMsgId();
			auto pSendMsg = std::make_shared<TransBaseMsg_t>(reqMsg.GetMsgType(), reqMsg.ToString());
			auto pClientSess = m_pServer->GetClientSess(reqMsg.m_strSenderId);
			if (pClientSess)
			{
				pClientSess->SendMsg(pSendMsg);
			}
			else
			{
				*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
			}
		}
		else
		{
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
		}
	}

	/**
	 * @brief 获取收到添加好友的邀请
	 * 
	 * @param response 
	 * @param request 
	 */
	void CHttpServer::Get_RecvAddFriendReq(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
	{
		AddFriendRecvReqMsg reqMsg;
		auto msgUtil = m_pServer->GetMsgPersisUtil();
		if (msgUtil && msgUtil->Get_AddFriendRecvReqMsg(reqMsg))
		{
			std::string strContent = reqMsg.ToString();
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << strContent.length() << "\r\n\r\n"
				<< strContent;
		}
		else
		{
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n";
		}
	}

	/**
	 * @brief 处理删除好友请求
	 * 
	 * @param response 
	 * @param request 
	 */
	void CHttpServer::Post_RemoveFriend(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
	{
		std::string  strReq = request->content.string();
		RemoveFriendReqMsg reqMsg;
		if (reqMsg.FromString(strReq))
		{
			auto pSendMsg = std::make_shared<TransBaseMsg_t>(reqMsg.GetMsgType(), reqMsg.ToString());
			auto pClientSess = m_pServer->GetClientSess(reqMsg.m_strUserId);
			if (pClientSess)
			{
				pClientSess->SendMsg(pSendMsg);
				m_httpRspMap.insert(HTTP_RSP_MAP_PAIR(reqMsg.m_strMsgId, HTTP_RSP_SECOND(time(nullptr), response)));
			}
			else
			{
				*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
			}
		}
		else
		{
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
		}
	}

	/**
	 * @brief 发送添加好友邀请的回复
	 * 
	 * @param response 
	 * @param request 
	 */
	void CHttpServer::Post_RecvAddFriendRsp(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
	{
		std::string  strReq = request->content.string();
		AddFriendRecvRspMsg reqMsg;
		if (reqMsg.FromString(strReq))
		{
			{
				auto msgUtil = m_pServer->GetMsgPersisUtil();
				AddFriendRecvReqMsg reqMsg2;
				reqMsg2.m_strMsgId = reqMsg.m_strMsgId;
				if (msgUtil && msgUtil->Remove_AddFriendRecvReqMsg(reqMsg2))
				{

				}
			}
			auto pSendMsg = std::make_shared<TransBaseMsg_t>(reqMsg.GetMsgType(), reqMsg.ToString());
			auto pClientSess = m_pServer->GetClientSess(reqMsg.m_strUserId);
			if (pClientSess)
			{
				pClientSess->SendMsg(pSendMsg);
			}
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n";
			//m_httpRspMap.insert(HTTP_RSP_MAP_PAIR(reqMsg.m_strMsgId, response));
		}
		else
		{
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
		}
	}


	/**
	 * @brief 获取添加好友的通知
	 * 
	 * @param response 
	 * @param request 
	 */
	void CHttpServer::Get_AddFriendNotify(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
	{
		AddFriendNotifyReqMsg reqMsg;
		auto msgUtil = m_pServer->GetMsgPersisUtil();
		if (msgUtil && msgUtil->Get_AddFriendNotifyReqMsg(reqMsg))
		{
			std::string strContent = reqMsg.ToString();
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << strContent.length() << "\r\n\r\n"
				<< strContent;


			{
				AddFriendNotifyRspMsg rspMsg;
				rspMsg.m_strMsgId = reqMsg.m_strMsgId;
				auto pSendMsg = std::make_shared<TransBaseMsg_t>(rspMsg.GetMsgType(), rspMsg.ToString());
				auto pClientSess = m_pServer->GetClientSess(reqMsg.m_strUserId);
				if (pClientSess)
				{
					pClientSess->SendMsg(pSendMsg);
				}
				else
				{
					LOG_ERR(ms_loger, "Can find UserID:{} ", reqMsg.m_strUserId);
				}
				msgUtil->Remove_AddFriendNotifyReqMsg(reqMsg);
			}
		}
		else
		{
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n";
		}
	}

	/**
	 * @brief POST发送文件的请求
	 * 
	 * @param response 
	 * @param request 
	 */
	void CHttpServer::Post_SendFileOnlineReq(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
	{
		std::string  strReq = request->content.string();
		FriendSendFileMsgReqMsg reqMsg;
		if (reqMsg.FromString(strReq))
		{
			reqMsg.m_strMsgId = GenerateMsgId();
			auto pClientSess = m_pServer->GetClientSess(reqMsg.m_strFromId);
			if (pClientSess)
			{
				pClientSess->SendMsg(&reqMsg);
				{
					QueryUserUdpAddrReqMsg queryReq;
					queryReq.m_strMsgId = GenerateMsgId();
					queryReq.m_strUserId = reqMsg.m_strFromId;
					queryReq.m_strUdpUserId = reqMsg.m_strToId;
					pClientSess->SendMsg(&queryReq);
				}
				m_httpRspMap.insert(std::pair<std::string, HTTP_RSP_SECOND>(reqMsg.m_strMsgId, { time(nullptr), response }));

			}
			else
			{
				*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n";
			}
			//m_httpRspMap.insert(HTTP_RSP_MAP_PAIR(reqMsg.m_strMsgId, response));
		}
		else
		{
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n";
		}
	}



	/**
	 * @brief 获取接收文件的请求消息
	 * 
	 * @param response 
	 * @param request 
	 */
	void CHttpServer::Get_RecvFileOnlineReq(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
	{
		FriendRecvFileMsgReqMsg reqMsg;
		auto msgUtil = m_pServer->GetMsgPersisUtil();
		if (msgUtil && msgUtil->Get_FriendRecvFileMsgReqMsg(reqMsg))
		{
			std::string strContent = reqMsg.ToString();
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << strContent.length() << "\r\n\r\n"
				<< strContent;
		}
		else
		{
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n";
		}

	}

	/**
	 * @brief 发送接收文件的HTTP回复
	 * 
	 * @param response 
	 * @param request 
	 */
	void CHttpServer::Post_RecvFileOnlineRsp(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
	{
		std::string  strReq = request->content.string();
		FriendRecvFileMsgRspMsg rspMsg;
		
		if (rspMsg.FromString(strReq))
		{
			rspMsg.m_nFileId = static_cast<int>(time(nullptr));
			auto pSendMsg = std::make_shared<TransBaseMsg_t>(rspMsg.GetMsgType(), rspMsg.ToString());
			auto pClientSess = m_pServer->GetClientSess(rspMsg.m_strFromId);
			if (pClientSess)
			{
				pClientSess->SendMsg(pSendMsg);
			}
			else
			{
				LOG_ERR(ms_loger, "{} ", rspMsg.m_strFromId);
			}
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n";
			//m_httpRspMap.insert(HTTP_RSP_MAP_PAIR(reqMsg.m_strMsgId, response));
			m_pServer->Handle_RecvFileOnlineRsp(rspMsg);
		}
		else
		{
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n";
		}
	}

	
	/**
	 * @brief 相应删除好友的回复消息
	 * 
	 * @param msg 
	 */
	void CHttpServer::On_RemoveFriendRspMsg(const RemoveFriendRspMsg& msg)
	{
		if (!msg.m_strMsgId.empty()) {
			auto item = m_httpRspMap.find(msg.m_strMsgId);
			if (item != m_httpRspMap.end()) {
				std::string strContent = msg.ToString();
				(*(item->second.m_response)) << "HTTP/1.1 200 OK\r\nContent-Length: " << strContent.length() << "\r\n\r\n"
					<< strContent;
				m_httpRspMap.erase(msg.m_strMsgId);
			}
		}
	}

	/**
	 * @brief 响应发送群组文本聊天消息的回复，TCP消息转HTTP消息回复
	 * 
	 * @param msg 
	 */
	void CHttpServer::On_SendGroupTextMsgRsp(const SendGroupTextMsgRspMsg& msg)
	{
		if (!msg.m_strMsgId.empty()) {
			auto item = m_httpRspMap.find(msg.m_strMsgId);
			if (item != m_httpRspMap.end()) {
				std::string strContent = msg.ToString();
				(*(item->second.m_response)) << "HTTP/1.1 200 OK\r\nContent-Length: " << strContent.length() << "\r\n\r\n"
					<< strContent;
				m_httpRspMap.erase(msg.m_strMsgId);
			}
		}
	}

	/**
	 * @brief 响应用户注册请求的回复，TCP消息转HTTP消息
	 * 
	 * @param msg 
	 */
	void CHttpServer::On_UserRegisterRsp(const UserRegisterRspMsg& msg) {
		if (!msg.m_strMsgId.empty()) {
			auto item = m_httpRspMap.find(msg.m_strMsgId);
			if (item != m_httpRspMap.end()) {
				std::string strContent = msg.ToString();
				*(item->second.m_response) << "HTTP/1.1 200 OK\r\nContent-Length: " << strContent.length() << "\r\n\r\n"
					<< strContent;
				m_httpRspMap.erase(msg.m_strMsgId);
			}
		}
	}

	/**
	 * @brief 响应添加好友发送回复消息，TCP消息转消息
	 * 
	 * @param msg 
	 */
	void CHttpServer::On_AddFriendSendRspMsg(const AddFriendSendRspMsg& msg)
	{
		if (!msg.m_strMsgId.empty()) {
			auto item = m_httpRspMap.find(msg.m_strMsgId);
			if (item != m_httpRspMap.end()) {
				std::string strContent = msg.ToString();
				(*(item->second.m_response)) << "HTTP/1.1 200 OK\r\nContent-Length: " << strContent.length() << "\r\n\r\n"
					<< strContent;
				m_httpRspMap.erase(msg.m_strMsgId);
			}
		}
	}

	/**
	 * @brief 响应发送好友文本聊天消息的回复，TCP消息转HTTP消息
	 * 
	 * @param msg 
	 */
	void CHttpServer::On_FriendChatSendTxtRsp(const FriendChatSendTxtRspMsg& msg) {
		if (!msg.m_strMsgId.empty()) {
			auto item = m_httpRspMap.find(msg.m_strMsgId);
			if (item != m_httpRspMap.end()) {
				std::string strContent = msg.ToString();
				(*(item->second.m_response)) << "HTTP/1.1 200 OK\r\nContent-Length: " << strContent.length() << "\r\n\r\n"
					<< strContent;
				m_httpRspMap.erase(msg.m_strMsgId);
			}
		}
	}


	void CHttpServer::Get_GroupListReq(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
	{
		LOG_INFO(ms_loger, " {} {}", __FUNCTION__, __FILENAME__);
		std::string strUserId = GetHttpParamUserId(request);
		if (strUserId.length() > 0)
		{
			GetGroupListReqMsg reqMsg;
			reqMsg.m_strUserId = strUserId;
			reqMsg.m_strMsgId = GenerateMsgId();
			auto pSendMsg = std::make_shared<TransBaseMsg_t>(reqMsg.GetMsgType(), reqMsg.ToString());
			auto pClientSess = m_pServer->GetClientSess(reqMsg.m_strUserId);
			if (pClientSess)
			{
				pClientSess->SendMsg(pSendMsg);
				m_httpRspMap.insert(HTTP_RSP_MAP_PAIR(reqMsg.m_strMsgId, { time(nullptr),response }));
			}
			else
			{
				*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
			}
		}
		else
		{
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
		}
	}

	void CHttpServer::Post_AddFriendTeamReq(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
	{
		std::string  strReq = request->content.string();
		AddTeamReqMsg reqMsg;
		if (reqMsg.FromString(strReq))
		{
			reqMsg.m_strMsgId = GenerateMsgId();
			auto pSendMsg = std::make_shared<TransBaseMsg_t>(reqMsg.GetMsgType(), reqMsg.ToString());
			auto pClientSess = m_pServer->GetClientSess("");
			if (pClientSess)
			{
				pClientSess->SendMsg(pSendMsg);
			}
			m_httpRspMap.insert(HTTP_RSP_MAP_PAIR(reqMsg.m_strMsgId,HTTP_RSP_SECOND(time(nullptr), response)));
		}
		else
		{
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
		}
	}

	void CHttpServer::Post_DestroyFriendTeamReq(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
	{
		std::string  strReq = request->content.string();
		RemoveTeamReqMsg reqMsg;
		if (reqMsg.FromString(strReq))
		{
			reqMsg.m_strMsgId = GenerateMsgId();
			auto pSendMsg = std::make_shared<TransBaseMsg_t>(reqMsg.GetMsgType(), reqMsg.ToString());
			auto pClientSess = m_pServer->GetClientSess("");
			if (pClientSess)
			{
				pClientSess->SendMsg(pSendMsg);
				m_httpRspMap.insert(HTTP_RSP_MAP_PAIR(reqMsg.m_strMsgId, { time(nullptr),response }));
			}
			else
			{
				*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n";
			}
		}
		else
		{
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
		}
	}

	void CHttpServer::Post_MoveFriendToTeamReq(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
	{
		std::string  strReq = request->content.string();
		MoveFriendToTeamReqMsg reqMsg;
		if (reqMsg.FromString(strReq))
		{
			reqMsg.m_strMsgId = GenerateMsgId();
			auto pSendMsg = std::make_shared<TransBaseMsg_t>(reqMsg.GetMsgType(), reqMsg.ToString());
			auto pClientSess = m_pServer->GetClientSess("");
			if (pClientSess)
			{
				pClientSess->SendMsg(pSendMsg);
				m_httpRspMap.insert(HTTP_RSP_MAP_PAIR(reqMsg.m_strMsgId, { time(nullptr), response }));
			}
		}
		else
		{
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n" << "";
		}
	}

	void CHttpServer::On_AddFriendTeamRsp(const AddTeamRspMsg& msg)
	{
		if (!msg.m_strMsgId.empty()) {
			auto item = m_httpRspMap.find(msg.m_strMsgId);
			if (item != m_httpRspMap.end()) {
				std::string strContent = msg.ToString();
				(*(item->second.m_response)) << "HTTP/1.1 200 OK\r\nContent-Length: " << strContent.length() << "\r\n\r\n"
					<< strContent;
				m_httpRspMap.erase(msg.m_strMsgId);
			}
		}
	}

	void CHttpServer::On_RemoveFriendTeamRsp(const RemoveTeamRspMsg& msg)
	{
		if (!msg.m_strMsgId.empty()) {
			auto item = m_httpRspMap.find(msg.m_strMsgId);
			if (item != m_httpRspMap.end()) {
				std::string strContent = msg.ToString();
				(*(item->second.m_response)) << "HTTP/1.1 200 OK\r\nContent-Length: " << strContent.length() << "\r\n\r\n"
					<< strContent;
				m_httpRspMap.erase(msg.m_strMsgId);
			}
		}
	}



	void CHttpServer::On_MoveFriendToTeamRsp(const MoveFriendToTeamRspMsg& msg)
	{
		if (!msg.m_strMsgId.empty()) {
			auto item = m_httpRspMap.find(msg.m_strMsgId);
			if (item != m_httpRspMap.end()) {
				std::string strContent = msg.ToString();
				(*(item->second.m_response)) << "HTTP/1.1 200 OK\r\nContent-Length: " << strContent.length() << "\r\n\r\n"
					<< strContent;
				m_httpRspMap.erase(msg.m_strMsgId);
			}
		}
	}


	void CHttpServer::On_GetGroupListRsp(const GetGroupListRspMsg& msg)
	{
		if (!msg.m_strMsgId.empty()) {
			auto item = m_httpRspMap.find(msg.m_strMsgId);
			if (item != m_httpRspMap.end()) {
				std::string strContent = msg.ToString();
				(*(item->second.m_response)) << "HTTP/1.1 200 OK\r\nContent-Length: " << strContent.length() << "\r\n\r\n"
					<< strContent;
				m_httpRspMap.erase(msg.m_strMsgId);
			}
		}
	}
	/**
	 * @brief 响应用户登录消息回复，TCP消息转HTTP消息
	 * 
	 * @param msg 
	 */
	void CHttpServer::On_UserLoginRsp(const UserLoginRspMsg&  msg) {
		if (!msg.m_strMsgId.empty()) {
			auto item = m_httpRspMap.find(msg.m_strMsgId);
			if (item != m_httpRspMap.end()) {
				std::string strContent = msg.ToString();
				(*(item->second.m_response)) << "HTTP/1.1 200 OK\r\nContent-Length: " << strContent.length() << "\r\n\r\n"
					<< strContent;
				m_httpRspMap.erase(msg.m_strMsgId);
			}
		}
	}

	
	/**
	 * @brief 响应用户注销消息回复，TCP消息转HTTP消息
	 * 
	 * @param msg 
	 */
	void CHttpServer::On_UserUnRegisterRsp(const UserUnRegisterRspMsg& msg) {
		if (!msg.m_strMsgId.empty()) {
			auto item = m_httpRspMap.find(msg.m_strMsgId);
			if (item != m_httpRspMap.end()) {
				std::string strContent = msg.ToString();
				(*(item->second.m_response)) << "HTTP/1.1 200 OK\r\nContent-Length: " << strContent.length() << "\r\n\r\n"
					<< strContent;
				m_httpRspMap.erase(msg.m_strMsgId);
			}
		}
	}

	/**
	 * @brief 响应用户退出登录消息回复，TCP消息转HTTP消息
	 * 
	 * @param msg 
	 */
	void CHttpServer::On_UserLogoutRsp(const UserLogoutRspMsg& msg) {
		if (!msg.m_strMsgId.empty()) {
			auto item = m_httpRspMap.find(msg.m_strMsgId);
			if (item != m_httpRspMap.end()) {
				std::string strContent = msg.ToString();
				(*(item->second.m_response)) << "HTTP/1.1 200 OK\r\nContent-Length: " << strContent.length() << "\r\n\r\n"
					<< strContent;
				m_httpRspMap.erase(msg.m_strMsgId);
			}
		}
	}

	/**
	 * @brief 响应查找好友消息回复，TCP消息转HTTP消息
	 * 
	 * @param msg 
	 */
	void CHttpServer::On_FindFriendRsp(const FindFriendRspMsg& msg) {
		if (!msg.m_strMsgId.empty()) {
			auto item = m_httpRspMap.find(msg.m_strMsgId);
			if (item != m_httpRspMap.end()) {
				std::string strContent = msg.ToString();
				(*(item->second.m_response)) << "HTTP/1.1 200 OK\r\nContent-Length: " << strContent.length() << "\r\n\r\n"
					<< strContent;
				m_httpRspMap.erase(msg.m_strMsgId);
			}
		}
	}

	/**
	 * @brief 响应查找群组消息回复，TCP消息转HTTP消息
	 * 
	 * @param msg 
	 */
	void CHttpServer::On_FindGroupRsp(const FindGroupRspMsg& msg)
	{
		if (!msg.m_strMsgId.empty()) {
			auto item = m_httpRspMap.find(msg.m_strMsgId);
			if (item != m_httpRspMap.end()) {
				std::string strContent = msg.ToString();
				(*(item->second.m_response)) << "HTTP/1.1 200 OK\r\nContent-Length: " << strContent.length() << "\r\n\r\n"
					<< strContent;
				m_httpRspMap.erase(msg.m_strMsgId);
			}
		}
	}

	/**
	 * @brief 响应加入群组消息的回复，TCP消息转HTTP消息
	 * 
	 * @param msg 
	 */
	void CHttpServer::On_AddToGroupRsp(const AddToGroupRspMsg& msg)
	{
		if (!msg.m_strMsgId.empty()) {
			auto item = m_httpRspMap.find(msg.m_strMsgId);
			if (item != m_httpRspMap.end()) {
				std::string strContent = msg.ToString();
				(*(item->second.m_response)) << "HTTP/1.1 200 OK\r\nContent-Length: " << strContent.length() << "\r\n\r\n"
					<< strContent;
				m_httpRspMap.erase(msg.m_strMsgId);
			}
		}
	}

	/**
	 * @brief 响应创建群组消息的回复，TCP消息转HTTP消息
	 * 
	 * @param msg 
	 */
	void CHttpServer::On_CreateGroupRsp(const CreateGroupRspMsg& msg)
	{
		if (!msg.m_strMsgId.empty()) {
			auto item = m_httpRspMap.find(msg.m_strMsgId);
			if (item != m_httpRspMap.end()) {
				std::string strContent = msg.ToString();
				(*(item->second.m_response)) << "HTTP/1.1 200 OK\r\nContent-Length: " << strContent.length() << "\r\n\r\n"
					<< strContent;
				m_httpRspMap.erase(msg.m_strMsgId);
			}
		}
	}

	void CHttpServer::On_GetFriendListRsp(const GetFriendListRspMsg& msg)
	{
		if (!msg.m_strMsgId.empty()) {
			auto item = m_httpRspMap.find(msg.m_strMsgId);
			if (item != m_httpRspMap.end()) {
				std::string strContent = msg.ToString();
				(*(item->second.m_response)) << "HTTP/1.1 200 OK\r\nContent-Length: " << strContent.length() << "\r\n\r\n"
					<< strContent;
				m_httpRspMap.erase(msg.m_strMsgId);
			}
		}
	}
	/**
	 * @brief 响应发送在线文件请求的回复消息
	 * 
	 * @param msg 在线文件请求的回复消息
	 */
	void CHttpServer::On_SendFriendFileOnlineRspMsg(const FriendSendFileMsgRspMsg& msg)
	{
		if (!msg.m_strMsgId.empty()) {
			auto item = m_httpRspMap.find(msg.m_strMsgId);
			if (item != m_httpRspMap.end()) {
				std::string strContent = msg.ToString();
				(*(item->second.m_response)) << "HTTP/1.1 200 OK\r\nContent-Length: " << strContent.length() << "\r\n\r\n"
					<< strContent;
				m_httpRspMap.erase(msg.m_strMsgId);
			}
		}
	}
	
	/**
	 * @brief 响应解散群组消息，TCP消息转HTTP消息
	 * 
	 * @param msg 
	 */
	void CHttpServer::On_DestroyGroupRsp(const DestroyGroupRspMsg& msg)
	{
		if (!msg.m_strMsgId.empty()) {
			auto item = m_httpRspMap.find(msg.m_strMsgId);
			if (item != m_httpRspMap.end()) {
				std::string strContent = msg.ToString();
				(*(item->second.m_response)) << "HTTP/1.1 200 OK\r\nContent-Length: " << strContent.length() << "\r\n\r\n"
					<< strContent;
				m_httpRspMap.erase(msg.m_strMsgId);
			}
		}
	}


	/**
	 * @brief 获取服务程序的版本
	 * 
	 * @param response 
	 * @param request 
	 */
	void CHttpServer::Get_Version(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
		std::string content = "https://www.dennisthink.com/";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << content.length() << "\r\n\r\n"
			<< content;
	}


	/**
	 * @brief 获取发送在线的文件的通知请求
	 * 
	 * @param response 
	 * @param request 
	 */
	void CHttpServer::Get_SendFileOnlineNotifyReq(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
	{
		FriendNotifyFileMsgReqMsg reqMsg;
		auto msgUtil = m_pServer->GetMsgPersisUtil();
		if (msgUtil && msgUtil->Get_FriendNotifyFileMsgReqMsg(reqMsg))
		{
			std::string strContent = reqMsg.ToString();
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << strContent.length() << "\r\n\r\n"
				<< strContent;
			{
				FriendNotifyFileMsgRspMsg rspMsg;
				rspMsg.m_strMsgId = reqMsg.m_strMsgId;
				auto pSendMsg = std::make_shared<TransBaseMsg_t>(rspMsg.GetMsgType(), rspMsg.ToString());
				auto pClientSess = m_pServer->GetClientSess(reqMsg.m_strToId);
				if (pClientSess)
				{
					pClientSess->SendMsg(pSendMsg);
				}
				msgUtil->Update_FriendNotifyFileMsgReqMsg(reqMsg);
			}
		}
		else
		{
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n";
		}
	}

	void CHttpServer::Post_SendFileOnlineNotifyRsp(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
	{

	}

	/**
	 * @brief 从HTTP的Get请求中，获取UserId参数
	 * 
	 * @param request HTTP的请求
	 * @return std::string 用户ID
	 */
	std::string CHttpServer::GetHttpParamUserId(std::shared_ptr<HttpServer::Request> request)
	{
		auto paramMap = request->parse_query_string();
		if (0 == paramMap.count("UserId"))
		{
			return "";
		}
		else
		{
			return paramMap.equal_range("UserId").first->second;
		}
	}
	void CHttpServer::OnTimer()
	{
		const time_t TIME_OUT_VALUE = 5;
		std::vector<std::string> timeOutMsgIdVec;
		time_t curTime = time(nullptr);
		for (auto item : m_httpRspMap)
		{
			if (curTime - item.second.m_recvTime > TIME_OUT_VALUE)
			{
				timeOutMsgIdVec.emplace_back(item.first);
			}
		}

		for (const auto msgId : timeOutMsgIdVec)
		{
			auto item = m_httpRspMap.find(msgId);
			*(item->second.m_response)<< "HTTP/1.1 200 OK\r\nContent-Length: " << 0 << "\r\n\r\n";
			m_httpRspMap.erase(msgId);
		}
	}
	/**
	 * @brief 初始化URL和函数的对应关系
	 * 
	 */
	void CHttpServer::Init(const int port=8000) {
		m_httpServer.config.port = port;
		auto pSelf = shared_from_this();
		
		m_httpServer.resource["/version"]["GET"] = [pSelf, this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
			this->Get_Version(response, request);
		};

		//User Begin
		m_httpServer.resource["/register_user"]["POST"] = [pSelf, this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
			this->Post_RegisterUser(response, request);
		};

		m_httpServer.resource["/un_register_user"]["POST"] = [pSelf, this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
			this->Post_UnRegisterUser(response, request);
		};

		m_httpServer.resource["/user_login"]["POST"] = [pSelf, this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
			this->Post_UserLogin(response, request);
		};

		m_httpServer.resource["/user_logout"]["POST"] = [pSelf, this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
			this->Post_UserLogout(response, request);
		};

		//User End


		//Friend Begin
		m_httpServer.resource["/find_friend"]["POST"] = [pSelf, this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
			this->Post_FindFriendReq(response, request);
		};

		m_httpServer.resource["/add_friend_req"]["POST"] = [pSelf, this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
			this->Post_AddFriendReq(response, request);
		};

		m_httpServer.resource["/get_add_friend_require"]["GET"] = [pSelf, this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
			this->Get_RecvAddFriendReq(response, request);
		};

		m_httpServer.resource["/on_add_friend_require"]["POST"] = [pSelf, this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
			this->Post_RecvAddFriendRsp(response, request);
		};

		m_httpServer.resource["/get_add_friend_notify"]["GET"] = [pSelf, this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
			this->Get_AddFriendNotify(response, request);
		};

		m_httpServer.resource["/remove_friend"]["POST"] = [pSelf, this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
			this->Post_RemoveFriend(response, request);
		};

		m_httpServer.resource["/send_friend_chat_text_msg"]["POST"] = [pSelf, this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
			this->Post_FriendChatSendTxt(response, request);
		};

		m_httpServer.resource["/get_friend_chat_recv_text_msg"]["GET"] = [pSelf, this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
			this->Get_FriendChatRecvTxtReq(response, request);
		};

		m_httpServer.resource["/on_friend_chat_recv_text_msg"]["POST"] = [pSelf, this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
			this->Post_FriendChatRecvTxtRsp(response, request);
		};

		m_httpServer.resource["/create_friend_team"]["POST"] = [pSelf, this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
			this->Post_AddFriendTeamReq(response, request);
		};

		m_httpServer.resource["/destroy_friend_team"]["POST"] = [pSelf, this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
			this->Post_DestroyFriendTeamReq(response, request);
		};

		m_httpServer.resource["/move_friend_to_team"]["POST"] = [pSelf, this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
			this->Post_MoveFriendToTeamReq(response, request);
		};

		//File相关
		m_httpServer.resource["/send_file_online_to_friend_req"]["POST"] = [pSelf, this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
			this->Post_SendFileOnlineReq(response, request);
		};


		m_httpServer.resource["/recv_file_online_from_friend_req"]["GET"] = [pSelf, this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
			this->Get_RecvFileOnlineReq(response, request);
		};

		m_httpServer.resource["/on_recv_file_online_from_friend_rsp"]["POST"] = [pSelf, this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
			this->Post_RecvFileOnlineRsp(response, request);
		};


		m_httpServer.resource["/send_file_online_to_friend_notify"]["GET"] = [pSelf, this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
			this->Get_SendFileOnlineNotifyReq(response, request);
		};

		m_httpServer.resource["/on_send_file_online_to_friend_rsp_notify"]["POST"] = [pSelf, this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
			this->Post_SendFileOnlineNotifyRsp(response, request);
		};


		m_httpServer.resource["/get_friend_list"]["GET"] = [pSelf, this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
			this->Get_FriendListReq(response, request);
		};
		//Friend End

		//GroupBegin 
		m_httpServer.resource["/find_group"]["POST"] = [pSelf, this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
			this->Post_FindGroupReq(response, request);
		};

		m_httpServer.resource["/get_group_list"]["GET"] = [pSelf, this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
			this->Get_GroupListReq(response, request);
		};
		
		m_httpServer.resource["/add_to_group"]["POST"] = [pSelf, this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
			this->Post_SendAddToGroupReq(response, request);
		};

		m_httpServer.resource["/create_group"]["POST"] = [pSelf, this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
			this->Post_CreateGroupReq(response, request);
		};

		m_httpServer.resource["/destroy_group"]["POST"] = [pSelf, this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
			this->Post_DestroyGroupReq(response, request);
		};

		m_httpServer.resource["/send_group_text_msg"]["POST"] = [pSelf, this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
			this->Post_SendGroupTextMsg(response, request);
		};

		m_httpServer.resource["/get_recv_group_text_msg"]["GET"] = [pSelf, this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
			this->Get_RecvGroupTextMsgReq(response, request);
		};

		m_httpServer.resource["/on_recv_group_text_msg"]["POST"] = [pSelf, this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
			this->Post_RecvGroupTextMsgRsp(response, request);
		};

		//Group End
	}

	UserLoginReqMsg CHttpServer::GetUserLoginReq(const std::string strUserName) const {
		auto item = m_userLoginMsgMap.find(strUserName);
		if (item != m_userLoginMsgMap.end()) {
			return item->second;
		}
		else {
			UserLoginReqMsg reqMsg;
			reqMsg.m_strUserName = strUserName;
			return reqMsg;
		}
	}
}































//#include "client_http.hpp"
//#include "server_http.hpp"
//
//// Added for the json-example
//
//// Added for the default_resource example
//#include <algorithm>
//#include <fstream>
//#include <vector>
//#ifdef HAVE_OPENSSL
//#include "crypto.hpp"
//#endif
//
//using namespace std;
//// Added for the json-example:
//
//using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
//using HttpClient = SimpleWeb::Client<SimpleWeb::HTTP>;
//
//int mainTest() {
//	asio::io_service ioService;
//  // HTTP-server at port 8080 using 1 thread
//  // Unless you do more heavy non-threaded processing in the resources,
//  // 1 thread is usually faster than several threads
//  HttpServer server(ioService);
//  server.config.port = 8080;
//
//  // Add resources using path-regex and method-string, and an anonymous function
//  // POST-example for the path /string, responds the posted string
//  server.resource["^/string$"]["POST"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
//    // Retrieve string:
//    auto content = request->content.string();
//    // request->content.string() is a convenience function for:
//    // stringstream ss;
//    // ss << request->content.rdbuf();
//    // auto content=ss.str();
//
//    *response << "HTTP/1.1 200 OK\r\nContent-Length: " << content.length() << "\r\n\r\n"
//              << content;
//
//
//    // Alternatively, use one of the convenience functions, for instance:
//    // response->write(content);
//  };
//
//  // POST-example for the path /json, responds firstName+" "+lastName from the posted json
//  // Responds with an appropriate error message if the posted json is not valid, or if firstName or lastName is missing
//  // Example posted json:
//  // {
//  //   "firstName": "John",
//  //   "lastName": "Smith",
//  //   "age": 25
//  // }
//  server.resource["^/json$"]["POST"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
//    try {
//      //ptree pt;
//      //read_json(request->content, pt);
//
//      //auto name = pt.get<string>("firstName") + " " + pt.get<string>("lastName");
//	  std::string name = "https://www.dennisthink.com";
//      *response << "HTTP/1.1 200 OK\r\n"
//                << "Content-Length: " << name.length() << "\r\n\r\n"
//                << name;
//    }
//    catch(const exception &e) {
//      *response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n"
//                << e.what();
//    }
//
//
//    // Alternatively, using a convenience function:
//    // try {
//    //     ptree pt;
//    //     read_json(request->content, pt);
//
//    //     auto name=pt.get<string>("firstName")+" "+pt.get<string>("lastName");
//    //     response->write(name);
//    // }
//    // catch(const exception &e) {
//    //     response->write(SimpleWeb::StatusCode::client_error_bad_request, e.what());
//    // }
//  };
//
//  // GET-example for the path /info
//  // Responds with request-information
//  server.resource["^/info$"]["GET"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
//    stringstream stream;
//    stream << "<h1>Request from " << request->remote_endpoint_address() << ":" << request->remote_endpoint_port() << "</h1>";
//
//    stream << request->method << " " << request->path << " HTTP/" << request->http_version;
//
//    stream << "<h2>Query Fields</h2>";
//    auto query_fields = request->parse_query_string();
//    for(auto &field : query_fields)
//      stream << field.first << ": " << field.second << "<br>";
//
//    stream << "<h2>Header Fields</h2>";
//    for(auto &field : request->header)
//      stream << field.first << ": " << field.second << "<br>";
//
//    response->write(stream);
//  };
//
//  // GET-example for the path /match/[number], responds with the matched string in path (number)
//  // For instance a request GET /match/123 will receive: 123
//  server.resource["^/match/([0-9]+)$"]["GET"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
//    response->write(request->path_match[1]);
//  };
//
//  // GET-example simulating heavy work in a separate thread
//  server.resource["^/work$"]["GET"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> /*request*/) {
//    thread work_thread([response] {
//      this_thread::sleep_for(chrono::seconds(5));
//      response->write("Work done");
//    });
//    work_thread.detach();
//  };
//
//  // Default GET-example. If no other matches, this anonymous function will be called.
//  // Will respond with content in the web/-directory, and its subdirectories.
//  // Default file: index.html
//  // Can for instance be used to retrieve an HTML 5 client that uses REST-resources on this server
//  server.default_resource["GET"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
////    try {
////     /* auto web_root_path = boost::filesystem::canonical("web");
////      auto path = boost::filesystem::canonical(web_root_path / request->path);
////      // Check if path is within web_root_path
////      if(distance(web_root_path.begin(), web_root_path.end()) > distance(path.begin(), path.end()) ||
////         !equal(web_root_path.begin(), web_root_path.end(), path.begin()))
////        throw invalid_argument("path must be within root path");
////      if(boost::filesystem::is_directory(path))
////        path /= "index.html";*/
////
////      SimpleWeb::CaseInsensitiveMultimap header;
////
////      // Uncomment the following line to enable Cache-Control
////      // header.emplace("Cache-Control", "max-age=86400");
////
////#ifdef HAVE_OPENSSL
//////    Uncomment the following lines to enable ETag
//////    {
//////      ifstream ifs(path.string(), ifstream::in | ios::binary);
//////      if(ifs) {
//////        auto hash = SimpleWeb::Crypto::to_hex_string(SimpleWeb::Crypto::md5(ifs));
//////        header.emplace("ETag", "\"" + hash + "\"");
//////        auto it = request->header.find("If-None-Match");
//////        if(it != request->header.end()) {
//////          if(!it->second.empty() && it->second.compare(1, hash.size(), hash) == 0) {
//////            response->write(SimpleWeb::StatusCode::redirection_not_modified, header);
//////            return;
//////          }
//////        }
//////      }
//////      else
//////        throw invalid_argument("could not read file");
//////    }
////#endif
////
////      auto ifs = make_shared<ifstream>();
////      ifs->open(path.string(), ifstream::in | ios::binary | ios::ate);
////
////      if(*ifs) {
////        auto length = ifs->tellg();
////        ifs->seekg(0, ios::beg);
////
////        header.emplace("Content-Length", to_string(length));
////        response->write(header);
////
////        // Trick to define a recursive function within this scope (for example purposes)
////        class FileServer {
////        public:
////          static void read_and_send(const shared_ptr<HttpServer::Response> &response, const shared_ptr<ifstream> &ifs) {
////            // Read and send 128 KB at a time
////            static vector<char> buffer(131072); // Safe when server is running on one thread
////            streamsize read_length;
////            if((read_length = ifs->read(&buffer[0], static_cast<streamsize>(buffer.size())).gcount()) > 0) {
////              response->write(&buffer[0], read_length);
////              if(read_length == static_cast<streamsize>(buffer.size())) {
////                response->send([response, ifs](const SimpleWeb::error_code &ec) {
////                  if(!ec)
////                    read_and_send(response, ifs);
////                  else
////                    cerr << "Connection interrupted" << endl;
////                });
////              }
////            }
////          }
////        };
////        FileServer::read_and_send(response, ifs);
////      }
////      else
////        throw invalid_argument("could not read file");
////    }
////    catch(const exception &e) {
////      response->write(SimpleWeb::StatusCode::client_error_bad_request, "Could not open path " + request->path + ": " + e.what());
////    }
//	  response->write(SimpleWeb::StatusCode::client_error_bad_request, "Could not open path " + request->path + ": ");
//  };
//
//  server.on_error = [](shared_ptr<HttpServer::Request> /*request*/, const SimpleWeb::error_code & /*ec*/) {
//    // Handle errors here
//  };
//
//    // Start server
//    server.start();
// 
//	ioService.run();
//
//	return 0;
//}
