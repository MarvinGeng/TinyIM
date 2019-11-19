/**
 * @file CMediumServer.cpp
 * @author DennisMi (https://www.dennisthink.com/)
 * @brief 
 * @version 0.1
 * @date 2019-09-12
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include "CMediumServer.h"
#include "json11.hpp"
#include "DaemonSvcApp.h"
#include "Msg.h"
#include "UiMsg.h"
#include "IUProtocolData.h"
namespace ClientCore
{
std::shared_ptr<spdlog::logger> CMediumServer::ms_loger;

/**
 * @brief 加载配置文件
 * 
 * @param cfg 配置文件的json
 * @param ec 发生的异常
 */
void CMediumServer::loadConfig(const json11::Json &cfg, std::error_code& ec) 
{
	auto serverCfg = cfg["server"];
	LOG_INFO(ms_loger,"loadConfig");
    ec.clear();
    m_serverCfg.m_strServerIp=serverCfg["ip"].string_value();
    m_serverCfg.m_nPort=(uint16_t)serverCfg["port"].int_value();
    LOG_INFO(ms_loger,"ServerIp: {}",m_serverCfg.to_string());
    if(!m_serverCfg.Valid())
	{
		LOG_ERR(ms_loger,"Config Error {}",m_serverCfg.to_string());
        return;
    }

	{
		auto clientsCfg = cfg["clients"];
		if (!clientsCfg.is_array())
		{
			LOG_ERR(ms_loger, "Clients Config Error {}", cfg.string_value());
			return;
		}
		for (auto item : clientsCfg.array_items())
		{
			IpPortCfg clientCfg;
			clientCfg.m_strServerIp = item["ip"].string_value();
			clientCfg.m_nPort = item["port"].int_value();
			LOG_INFO(ms_loger, "Client Config: {}", clientCfg.to_string());
			m_clientCfgVec.push_back(clientCfg);
		}
	}

	{
		{
			auto clientsCfg = cfg["clientsBin"];
			if (!clientsCfg.is_array())
			{
				LOG_ERR(ms_loger, "Clients Config Error {}", cfg.string_value());
				return;
			}
			for (auto item : clientsCfg.array_items())
			{
				IpPortCfg clientCfg;
				clientCfg.m_strServerIp = item["ip"].string_value();
				clientCfg.m_nPort = item["port"].int_value();
				LOG_INFO(ms_loger, "Client Config: {}", clientCfg.to_string());
				m_clientBinCfgVec.push_back(clientCfg);
			}
		}
	}

	{
		auto httpCfg = cfg["httpserver"];
		if (httpCfg.is_object())
		{
			m_httpCfg.m_strServerIp = httpCfg["ip"].string_value();
			m_httpCfg.m_nPort = httpCfg["port"].int_value();
		}
	}
}
void CMediumServer::Handle_UdpMsg(const asio::ip::udp::endpoint endPt,const FileDataSendRspMsg& rspMsg)
{
	if (rspMsg.m_nDataIndex < rspMsg.m_nDataTotalCount)
	{
		FileDataSendReqMsg reqMsg;
		if (m_fileUtil.OnReadData(rspMsg.m_nFileId, reqMsg.m_szData, reqMsg.m_nDataLength, 1024))
		{
			LOG_INFO(ms_loger, "Read Data ", rspMsg.m_nFileId);
			reqMsg.m_strMsgId = m_httpServer->GenerateMsgId();
			reqMsg.m_nFileId = rspMsg.m_nFileId;
			reqMsg.m_strFromId = rspMsg.m_strToId;
			reqMsg.m_strToId = rspMsg.m_strFromId;
			reqMsg.m_nDataTotalCount = rspMsg.m_nDataTotalCount;
			reqMsg.m_nDataIndex = rspMsg.m_nDataIndex + 1;
			m_udpClient->send_msg(endPt, &reqMsg);
		}
	}
	else
	{
		FileVerifyReqMsg verifyReqMsg;
		verifyReqMsg.m_strMsgId = m_httpServer->GenerateMsgId();
		verifyReqMsg.m_nFileId = rspMsg.m_nFileId;
		verifyReqMsg.m_strFromId = rspMsg.m_strToId;
		verifyReqMsg.m_strToId = rspMsg.m_strFromId;
		verifyReqMsg.m_strFileName = "";
		verifyReqMsg.m_nFileSize = 0;
		verifyReqMsg.m_strFileHash = "FILE_HASH";
		auto pSess = GetClientSess(verifyReqMsg.m_strToId);
		if (pSess != nullptr)
		{
			auto pSend = std::make_shared<TransBaseMsg_t>(verifyReqMsg.GetMsgType(), verifyReqMsg.ToString());
			pSess->SendMsg(pSend);
		}
		m_fileUtil.OnCloseFile(rspMsg.m_nFileId);
	}
}
void CMediumServer::Handle_UdpMsg(const asio::ip::udp::endpoint endPt, TransBaseMsg_t* pMsg)
{
	if (pMsg)
	{
		switch (pMsg->GetType())
		{
		case MessageType::KeepAliveReq_Type:
		{

		}break;
		case MessageType::FileSendDataRsp_Type:
		{
			FileDataSendRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string()))
			{
				Handle_UdpMsg(endPt, rspMsg);
			}
		}break;
		default:
		{

		}break;
		}
	}
}
/**
 * @brief 启动服务
 * 
 * @param callback 
 */
void CMediumServer::start(const std::function<void(const std::error_code &)> &callback)
{
	if (!m_serverCfg.Valid())
	{
		LOG_ERR(ms_loger, "ServerConfig Is Error {}", m_serverCfg.to_string());
		return;
	}
	LOG_INFO(ms_loger, "Server Start Service");
	std::error_code ec;
	asio::ip::tcp::endpoint endpoint;
	if (m_serverCfg.m_strServerIp.length() > 0)
	{
		endpoint = asio::ip::tcp::endpoint(
			asio::ip::address::from_string(m_serverCfg.m_strServerIp),
			m_serverCfg.m_nPort);
	}
	else
	{
		endpoint =
			asio::ip::tcp::endpoint(asio::ip::tcp::v4(), m_serverCfg.m_nPort);
	}
	LOG_WARN(ms_loger, "Before Open [ {} {} ]",__FILENAME__,__LINE__);
	m_acceptor.open(endpoint.protocol());
	LOG_WARN(ms_loger, "Before Set Option [ {} {} ]", __FILENAME__, __LINE__);
	m_acceptor.set_option(asio::socket_base::reuse_address(true));
	LOG_WARN(ms_loger, "Before Bind [ {} {} ]", __FILENAME__, __LINE__);
	m_acceptor.bind(endpoint, ec);
	LOG_WARN(ms_loger, "Before Listen [ {} {} ]", __FILENAME__, __LINE__);
	if (!ec)
	{
		LOG_WARN(ms_loger, "Bind To {} Succeed [{} {}]", m_serverCfg.to_string(),__FILENAME__,__LINE__);
		m_acceptor.listen(asio::socket_base::max_connections, ec);
		if (!ec)
		{
			LOG_WARN(ms_loger, "Listen To {} Succeed [{} {}]", m_serverCfg.to_string(), __FILENAME__, __LINE__);
		}
		else
		{
			LOG_WARN(ms_loger, "Listen To {} Failed, Reason:{} {} [{} {}]",
				m_serverCfg.to_string(), ec.value(), ec.message(), __FILENAME__, __LINE__);
		}
		SetTimer(5);
		do_accept();
		m_httpServer->Start(m_httpCfg.m_nPort);

		m_freeClientSess = std::make_shared<CClientSess>(m_ioService,
			m_clientCfgVec[0].m_strServerIp,
			m_clientCfgVec[0].m_nPort, this);

		m_freeClientSess->StartConnect();
		auto pSelf = shared_from_this();
		m_udpClient = std::make_shared<CUdpClient>(m_ioService, "127.0.0.1", 9000, [this,pSelf](const asio::ip::udp::endpoint endPt, TransBaseMsg_t* pMsg) {
			Handle_UdpMsg(endPt, pMsg);
		});
		m_udpClient->StartConnect();


		/*{
			for (auto item : m_clientCfgVec)
			{
				if (item.Valid())
				{
					auto pQueue = std::make_shared<CClientSessManager>(item);
					m_queueVec.push_back(pQueue);
					std::error_code ec;
					pQueue->Start(this, m_ioService, ec);
					LOG_INFO(ms_loger, "ConnectTo: {}", item.to_string());
				}
			}
		}*/
	}
	else
	{
		LOG_WARN(ms_loger, "Bind To {} Failed [{} {}]", m_serverCfg.to_string(), __FILENAME__, __LINE__);
		callback(ec);
#ifndef WIN32
		exit(BIND_FAILED_EXIT);
#endif
	}
}
/**
 * @brief 检查所有的socket连接
 * 
 */
void CMediumServer::CheckAllConnect()
{
	for (auto& item : m_userClientSessMap)
	{
		item.second->StartConnect();
		item.second->SendKeepAlive();
	}

	if (m_udpClient)
	{
		m_udpClient->SendKeepAlive();
	}
}

/**
 * @brief 接受来自客户端的连接
 * 
 */
void CMediumServer::do_accept() 
{
		LOG_INFO(ms_loger,"CMediumServer do_accept [{} {}]", __FILENAME__, __LINE__);
        m_acceptor.async_accept(m_socket, [this](std::error_code ec) {
            if (!ec)
			{
			   LOG_INFO(ms_loger,"Server accept Successed [{} {}]",__FILENAME__, __LINE__);
               

			   if (!m_clientCfgVec.empty() && !m_clientBinCfgVec.empty())
			   {
				   //
				   auto clientSess = std::make_shared<CClientSess>(m_ioService, 
					                                               m_clientCfgVec[0].m_strServerIp, 
					                                               m_clientCfgVec[0].m_nPort, this);

				   clientSess->StartConnect();


				   //
				   auto serverSess = std::make_shared<CServerSess>(std::move(m_socket), this);
				   serverSess->Start();


				   //
				   m_ForwardSessMap.insert(std::pair<CServerSess_SHARED_PTR, CClientSess_SHARED_PTR>(serverSess, clientSess));
				   m_BackSessMap.insert(std::pair<CClientSess_SHARED_PTR,CServerSess_SHARED_PTR>(clientSess, serverSess));

			   }
			}
            do_accept();
        });
}

std::string CMediumServer::GetUserId(const std::string strUserName)
{
	auto item = m_userNameUserIdMap.find(strUserName);
	if (item != m_userNameUserIdMap.end())
	{
		return item->second;
	}
	else
	{
		return "";
	}
}
/**
 * @brief 根据用户ID或用户名获取客户端会话连接
 * 
 * @param strUserName 用户名或者用户ID
 * @return CClientSess_SHARED_PTR 
 */
CClientSess_SHARED_PTR CMediumServer::GetClientSess(const std::string strUserName) {
	auto item = m_userClientSessMap.find(strUserName);
	if (item != m_userClientSessMap.end())
	{
		return item->second;
	}
	else
	{
		return nullptr;
	}
}

	CClientSess_SHARED_PTR CMediumServer::CreateClientSess()
	{
		//auto pReturn = m_freeClientSess;
		m_freeClientSess = std::make_shared<CClientSess>(m_ioService,
			m_clientCfgVec[0].m_strServerIp,
			m_clientCfgVec[0].m_nPort, this);

		m_freeClientSess->StartConnect();
		//m_userClientSessMap.insert(std::pair<std::string, CClientSess_SHARED_PTR>(strUserName, pReturn));
		return m_freeClientSess;
	}

void CMediumServer::CheckWaitMsgVec()
{
	decltype(m_WaitMsgMap) notSendMap;
	if (m_timeCount % 10 == 0)
	{
		for (auto item : m_WaitMsgMap)
		{
			auto pSess = GetClientSess(item.first);
			if (pSess)
			{
				for (auto msgItem : item.second)
				{
					pSess->SendMsg(msgItem.get());
				}
			}
			else
			{
				notSendMap.insert(item);
			}
		}
		m_WaitMsgMap.clear();
		m_WaitMsgMap = notSendMap;
	}
}
/**
 * @brief 响应定时器的任务，一些需要定时处理的任务，可以写成一个函数，在此函数中调用
 * 
 */
void CMediumServer::OnTimer()
{
	CheckAllConnect();
	CheckWaitMsgVec();
	m_timeCount++;
	if (m_httpServer)
	{
		m_httpServer->OnTimer();
	}
}

/**
 * @brief 设置定时器，单位秒
 * 
 * @param nSeconds 定时的秒数
 */
void CMediumServer::SetTimer(int nSeconds)
{
	if(m_timer)
	{
		m_timer->expires_from_now(std::chrono::seconds(nSeconds));
		auto self = shared_from_this();
		m_timer->async_wait([this,nSeconds,self](const std::error_code& ec){
			if(!ec)
			{
				LOG_INFO(this->ms_loger,"On Timer at MediumServer [{} {}]",__FILENAME__, __LINE__);
				this->OnTimer();
				this->SetTimer(nSeconds);
			}
			else
			{
				LOG_WARN(this->ms_loger,"On Timer at MediumServer {} [{} {}]",ec.message(),__FILENAME__, __LINE__);
			}
		});
	}
}
void CMediumServer::HandleFriendChatSendTextMsgRsp(const FriendChatSendTxtRspMsg& rspMsg)
{

}

void CMediumServer::HandleFriendChatRecvTextMsgRsp(const FriendChatRecvTxtReqMsg& reqMsg)
{

}
/**
 * @brief 获取Listen的服务器的IP和端口
 * 
 * @return std::string 
 */
std::string CMediumServer::getServerIpPort()
{
	return m_serverCfg.to_string();
}

/**
 * @brief 处理从ClientSess中，回传的消息
 * 
 * @param msg 消息
 */
void CMediumServer::SendBack(const std::shared_ptr<CClientSess>& pClientSess,const TransBaseMsg_t& msg)
{
	auto pMsg = std::make_shared<TransBaseMsg_t>(msg.GetType(), msg.to_string());
	auto item = m_BackSessMap.find(pClientSess);
	HandleMsg(msg);
	if (item != m_BackSessMap.end())
	{
		item->second->SendMsg(pMsg);
	}
	else
	{
		if (msg.GetType() == MessageType::UserLoginRsp_Type)
		{
			UserLoginRspMsg rspMsg;
			rspMsg.FromString(msg.to_string());
			if (rspMsg.m_eErrCode == ERROR_CODE_TYPE::E_CODE_SUCCEED)
			{
				pClientSess->SetUserId(rspMsg.m_strUserId);
				pClientSess->SetUserName(rspMsg.m_strUserName);
				m_userStateMap.erase(rspMsg.m_strUserId);
				m_userStateMap.insert({ rspMsg.m_strUserId,CLIENT_SESS_STATE::SESS_LOGIN_FINISHED });
				m_userClientSessMap.insert({ rspMsg.m_strUserId,pClientSess });

				m_strUserNameUserIdMap.erase(rspMsg.m_strUserName);
				m_strUserNameUserIdMap.insert({ rspMsg.m_strUserName, rspMsg.m_strUserId });
			}
			if (!m_fileUtil.IsFolder(rspMsg.m_strUserName))
			{
				m_fileUtil.CreateFolder(rspMsg.m_strUserName);
			}
		}
		if (msg.GetType() == MessageType::UserLogoutRsp_Type)
		{
			UserLogoutRspMsg rspMsg;
			if (rspMsg.FromString(msg.to_string())) {
				if (rspMsg.m_eErrCode == ERROR_CODE_TYPE::E_CODE_SUCCEED) {
					m_userClientSessMap.erase(rspMsg.m_strUserName);
					m_userStateMap.erase(pClientSess->UserId());
					m_userStateMap.insert({ pClientSess->UserId(),CLIENT_SESS_STATE::SESS_UN_LOGIN });
				}
			}
		}
		if (msg.GetType() == MessageType::NetFailedReport_Type)
		{
			auto item = m_userStateMap.find(pClientSess->UserId());
			if (item != m_userStateMap.end())
			{
				if (item->second == CLIENT_SESS_STATE::SESS_LOGIN_FINISHED)
				{
					m_userStateMap.erase(pClientSess->UserId());
					m_userStateMap.insert({ pClientSess->UserId(),CLIENT_SESS_STATE::SESS_LOGIN_SEND });
				}
			}
			m_userClientSessMap.erase(pClientSess->UserId());
			
			auto pSess = GetClientSess(pClientSess->UserId());
			pSess->SetUserId(pClientSess->UserId());
			pSess->SetUserName(pClientSess->UserName());
		}
		if (msg.GetType() == MessageType::NetRecoverReport_Type)
		{
			auto loginReq = m_httpServer->GetUserLoginReq(pClientSess->UserName());
			auto item = m_userStateMap.find(pClientSess->UserId());
			if (item != m_userStateMap.end())
			{
				if (item->second == CLIENT_SESS_STATE::SESS_LOGIN_SEND)
				{
					auto pSendMsg = std::make_shared<TransBaseMsg_t>(loginReq.GetMsgType(), loginReq.ToString());
					pClientSess->SendMsg(pSendMsg);
				}
			}
		}
		if (msg.GetType() == MessageType::KeepAliveRsp_Type )
		{
			KeepAliveRspMsg rspMsg;
			if(rspMsg.FromString(msg.to_string()))
			{
				KeepAliveReqMsg reqMsg;
				reqMsg.m_strClientId = pClientSess->UserId();
				if (m_udpClient)
				{
					m_udpClient->sendToServer(&reqMsg);
				}
			}
		}
		if (msg.GetType() == MessageType::KeepAliveReq_Type)
		{
			KeepAliveReqMsg reqMsg;
			if (reqMsg.FromString(msg.to_string()))
			{
				KeepAliveReqMsg reqMsg2;
				reqMsg2.m_strClientId = pClientSess->UserId();
				if (m_udpClient)
				{
					m_udpClient->sendToServer(&reqMsg2);
				}
			}
		}
		if (msg.GetType() == MessageType::QueryUserUdpAddrRsp_Type)
		{
			QueryUserUdpAddrRspMsg rspMsg;
			if (rspMsg.FromString(msg.to_string())) {
				if (ERROR_CODE_TYPE::E_CODE_SUCCEED == rspMsg.m_errCode)
				{
					m_userIdUdpAddrMap.erase(rspMsg.m_strUdpUserId);
					m_userIdUdpAddrMap.insert({ rspMsg.m_strUdpUserId,rspMsg.m_udpEndPt });
				}
				else
				{
					//TODO再次获取时间
					auto pMsg = std::make_shared<QueryUserUdpAddrReqMsg>();
					pMsg->m_strUserId = rspMsg.m_strUserId;
					pMsg->m_strUdpUserId = rspMsg.m_strUdpUserId;
					auto item = m_WaitMsgMap.find(pMsg->m_strUserId);
					if (item != m_WaitMsgMap.end())
					{
						m_WaitMsgMap[rspMsg.m_strUserId].push_back(pMsg);
					}
					else
					{
						std::vector<std::shared_ptr<BaseMsg>> mapVec;
						mapVec.push_back(pMsg);
						m_WaitMsgMap.insert({ rspMsg.m_strUserId,mapVec});
					}
				}
			}
		}
		OnHttpRsp(pMsg);
	}

}
void CMediumServer::HandleFileVerifyReq(const FileVerifyReqMsg& msg)
{
	FileVerifyRspMsg rspMsg;
	std::string strRecvHash = m_fileUtil.CalcHash(msg.m_strFileName);
	m_fileUtil.OnCloseFile(msg.m_nFileId + 1);
	if (msg.m_strFileHash == strRecvHash)
	{
		rspMsg.m_eErrCode = ERROR_CODE_TYPE::E_CODE_SUCCEED;
	}
	else
	{
		rspMsg.m_eErrCode = ERROR_CODE_TYPE::E_CODE_LOGIN_FAILED;
	}
	rspMsg.m_strMsgId = msg.m_strMsgId;
	rspMsg.m_strFileName = msg.m_strFileName;
	rspMsg.m_strFromId = msg.m_strToId;
	rspMsg.m_strToId = msg.m_strFromId;
	rspMsg.m_nFileId = msg.m_nFileId;
	auto pSess = GetClientSess(rspMsg.m_strFromId);
	if (pSess != nullptr)
	{
		auto pSend = std::make_shared<TransBaseMsg_t>(rspMsg.GetMsgType(), rspMsg.ToString());
		pSess->SendMsg(pSend);
	}

}
void CMediumServer::HandleFileDataSendRsp(const FileDataSendRspMsg& rspMsg)
{
	if (rspMsg.m_nDataIndex < rspMsg.m_nDataTotalCount)
	{
		FileDataSendReqMsg reqMsg;
		if (m_fileUtil.OnReadData(rspMsg.m_nFileId, reqMsg.m_szData, reqMsg.m_nDataLength, 1024))
		{
			LOG_INFO(ms_loger, "Read Data ", rspMsg.m_nFileId);
			reqMsg.m_strMsgId = m_httpServer->GenerateMsgId();
			reqMsg.m_nFileId = rspMsg.m_nFileId;
			reqMsg.m_strFromId = rspMsg.m_strToId;
			reqMsg.m_strToId = rspMsg.m_strFromId;
			reqMsg.m_nDataTotalCount = rspMsg.m_nDataTotalCount;
			reqMsg.m_nDataIndex = rspMsg.m_nDataIndex + 1;
			auto pSess = GetClientSess(reqMsg.m_strFromId);
			if (pSess != nullptr)
			{
				auto pSend = std::make_shared<TransBaseMsg_t>(reqMsg.GetMsgType(), reqMsg.ToString());
				pSess->SendMsg(pSend);
			}
		}
	}
	else
	{
		FileVerifyReqMsg verifyReqMsg;
		verifyReqMsg.m_strMsgId = m_httpServer->GenerateMsgId();
		verifyReqMsg.m_nFileId = rspMsg.m_nFileId;
		verifyReqMsg.m_strFromId = rspMsg.m_strToId;
		verifyReqMsg.m_strToId = rspMsg.m_strFromId;
		verifyReqMsg.m_strFileName = "";
		verifyReqMsg.m_nFileSize = 0;
		verifyReqMsg.m_strFileHash = "FILE_HASH";
		auto pSess = GetClientSess(verifyReqMsg.m_strToId);
		if (pSess != nullptr)
		{
			auto pSend = std::make_shared<TransBaseMsg_t>(verifyReqMsg.GetMsgType(), verifyReqMsg.ToString());
			pSess->SendMsg(pSend);
		}
		m_fileUtil.OnCloseFile(rspMsg.m_nFileId);
	}
}

void CMediumServer::HandleFriendNotifyFileMsgReq(const FriendNotifyFileMsgReqMsg& notifyMsg)
{
	if (notifyMsg.m_eOption == E_FRIEND_OPTION::E_AGREE_ADD)
	{
		int nFileId = static_cast<int>(time(nullptr));
		std::string strFileName = notifyMsg.m_strFileName;
		int nFileSize = 0;
		m_fileUtil.GetFileSize(nFileSize, strFileName);
		if (m_fileUtil.OpenReadFile(notifyMsg.m_nFileId, strFileName)) {
			FileDataSendReqMsg sendReqMsg;
			sendReqMsg.m_strMsgId = m_httpServer->GenerateMsgId();
			sendReqMsg.m_strFromId = notifyMsg.m_strToId;
			sendReqMsg.m_strToId = notifyMsg.m_strFromId;
			sendReqMsg.m_nFileId = notifyMsg.m_nFileId;

			sendReqMsg.m_nDataTotalCount =  nFileSize / 1024 + (nFileSize%1024 == 0 ? 0 : 1);
			sendReqMsg.m_nDataIndex = 1;
			sendReqMsg.m_nDataLength = 0;
			m_fileUtil.OnReadData(sendReqMsg.m_nFileId, sendReqMsg.m_szData, sendReqMsg.m_nDataLength, 1024);
			
			{
				auto udpItem = m_userIdUdpAddrMap.find(sendReqMsg.m_strToId);
				if (udpItem != m_userIdUdpAddrMap.end())
				{
					m_udpClient->send_msg(udpItem->second.m_strServerIp, udpItem->second.m_nPort, &sendReqMsg);
				}
				else
				{
					m_udpClient->sendToServer(&sendReqMsg);
				}
			}
		}
	}
}

void CMediumServer::Handle_UdpMsg(const asio::ip::udp::endpoint endPt, const FileDataRecvReqMsg& reqMsg)
{
	if (reqMsg.m_nDataIndex <= reqMsg.m_nDataTotalCount)
	{
		m_fileUtil.OnWriteData(reqMsg.m_nFileId + 1, reqMsg.m_szData, reqMsg.m_nDataLength);
		LOG_INFO(ms_loger, "WriteData ", reqMsg.m_nFileId);
		FileDataRecvRspMsg rspMsg;
		rspMsg.m_strMsgId = reqMsg.m_strMsgId;
		rspMsg.m_nFileId = reqMsg.m_nFileId;
		rspMsg.m_strFromId = reqMsg.m_strToId;
		rspMsg.m_strToId = reqMsg.m_strFromId;
		rspMsg.m_nDataTotalCount = reqMsg.m_nDataTotalCount;
		rspMsg.m_nDataIndex = reqMsg.m_nDataIndex;
		m_udpClient->send_msg(endPt, &rspMsg);
	}
}
void CMediumServer::HandleFileDataRecvReq(const FileDataRecvReqMsg& reqMsg)
{
	if (reqMsg.m_nDataIndex <= reqMsg.m_nDataTotalCount)
	{
		m_fileUtil.OnWriteData(reqMsg.m_nFileId + 1, reqMsg.m_szData, reqMsg.m_nDataLength);
		LOG_INFO(ms_loger, "WriteData ", reqMsg.m_nFileId);
		FileDataRecvRspMsg rspMsg;
		rspMsg.m_strMsgId = reqMsg.m_strMsgId;
		rspMsg.m_nFileId = reqMsg.m_nFileId;
		rspMsg.m_strFromId = reqMsg.m_strToId;
		rspMsg.m_strToId = reqMsg.m_strFromId;
		rspMsg.m_nDataTotalCount = reqMsg.m_nDataTotalCount;
		rspMsg.m_nDataIndex = reqMsg.m_nDataIndex;

		auto pSess = GetClientSess(rspMsg.m_strFromId);
		if (pSess != nullptr)
		{
			auto pSend = std::make_shared<TransBaseMsg_t>(rspMsg.GetMsgType(), rspMsg.ToString());
			pSess->SendMsg(pSend);
		}
	}
}
/**
 * @brief 将TCP的回复消息变为HTTP消息
 * 
 * @param pMsg TCP的回复消息
 */
void CMediumServer::OnHttpRsp(std::shared_ptr<TransBaseMsg_t> pMsg)
{
	if (pMsg)
	{
		switch (pMsg->GetType())
		{
		case MessageType::UserRegisterRsp_Type:
		{
			UserRegisterRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string()))
			{
				if (m_httpServer)
				{
					m_httpServer->On_UserRegisterRsp(rspMsg);
				}
			}
		}break;
		case MessageType::UserUnRegisterRsp_Type:
		{
			UserUnRegisterRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())) {
				if (m_httpServer) {
					m_httpServer->On_UserUnRegisterRsp(rspMsg);
				}
			}
		}break;
		case MessageType::UserLoginRsp_Type:
		{
			UserLoginRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())) {
				if (m_httpServer) {
					m_httpServer->On_UserLoginRsp(rspMsg);
				}
			}
		}break;
		case MessageType::UserLogoutRsp_Type:
		{
			UserLogoutRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())) {
				if (m_httpServer) {
					m_httpServer->On_UserLogoutRsp(rspMsg);
				}
			}
		}break;
		case MessageType::FindFriendRsp_Type:
		{
			FindFriendRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())) {
				if (m_httpServer) {
					m_httpServer->On_FindFriendRsp(rspMsg);
				}
			}
		}break;
		case MessageType::AddTeamRsp_Type:
		{
			AddTeamRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())) {
				if (m_httpServer) {
					m_httpServer->On_AddFriendTeamRsp(rspMsg);
				}
			}
		}break;
		case MessageType::RemoveTeamRsp_Type:
		{
			RemoveTeamRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())) {
				if (m_httpServer) {
					m_httpServer->On_RemoveFriendTeamRsp(rspMsg);
				}
			}
		}break;
		case MessageType::MoveFriendToTeamRsp_Type:
		{
			MoveFriendToTeamRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())) {
				if (m_httpServer) {
					m_httpServer->On_MoveFriendToTeamRsp(rspMsg);
				}
			}
		}break;
		case MessageType::AddToGroupRsp_Type:
		{
			AddToGroupRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())) {
				if (m_httpServer) {
					m_httpServer->On_AddToGroupRsp(rspMsg);
				}
			}
		}break;
		case MessageType::CreateGroupRsp_Type:
		{
			CreateGroupRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())) {
				if (m_httpServer) {
					m_httpServer->On_CreateGroupRsp(rspMsg);
				}
			}
		}break;
		case MessageType::GetGroupListRsp_Type:
		{
			GetGroupListRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())) {
				if (m_httpServer) {
					m_httpServer->On_GetGroupListRsp(rspMsg);
				}
			}
		}break;
		case MessageType::DestroyGroupRsp_Type:
		{
			DestroyGroupRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())) {
				if (m_httpServer) {
					m_httpServer->On_DestroyGroupRsp(rspMsg);
				}
			}
		}break;
		case MessageType::FindGroupRsp_Type:
		{
			FindGroupRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())){
				if (m_httpServer) {
					m_httpServer->On_FindGroupRsp(rspMsg);
				}
			}
		}break;
		case MessageType::SendGroupTextMsgRsp_Type:
		{
			SendGroupTextMsgRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())) {
				if (m_httpServer) {
					m_httpServer->On_SendGroupTextMsgRsp(rspMsg);
				}
			}
		}break;
		case MessageType::RecvGroupTextMsgReq_Type:
		{
			RecvGroupTextMsgReqMsg reqMsg;
			if (reqMsg.FromString(pMsg->to_string())) {
				if (m_msgPersisUtil) {
					m_msgPersisUtil->Save_RecvGroupTextMsgReqMsg(reqMsg);
				}
			}
		}break;
		case MessageType::FriendSendFileMsgRsp_Type:
		{
			FriendSendFileMsgRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())) {
				if (m_httpServer) {
					m_httpServer->On_SendFriendFileOnlineRspMsg(rspMsg);
				}
			}
		}break;
		case MessageType::FriendRecvFileMsgReq_Type:
		{
			FriendRecvFileMsgReqMsg reqMsg;
			if (reqMsg.FromString(pMsg->to_string())) {
				if (m_msgPersisUtil) {
					m_msgPersisUtil->Save_FriendRecvFileMsgReqMsg(reqMsg);
				}
			}
		}break;
		case MessageType::FriendNotifyFileMsgReq_Type:
		{
			FriendNotifyFileMsgReqMsg reqMsg;
			if (reqMsg.FromString(pMsg->to_string())) {
				if (m_msgPersisUtil) {
					m_msgPersisUtil->Save_FriendNotifyFileMsgReqMsg(reqMsg);
				}
				HandleFriendNotifyFileMsgReq(reqMsg);
			}
		}break;
		case MessageType::FileRecvDataReq_Type:
		{
			FileDataRecvReqMsg reqMsg;
			if (reqMsg.FromString(pMsg->to_string())) {
				HandleFileDataRecvReq(reqMsg);
			}
		}break;
		case MessageType::FileSendDataRsp_Type:
		{
			FileDataSendRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())) {
				HandleFileDataSendRsp(rspMsg);
			}
		}break;
		case MessageType::AddFriendSendRsp_Type:
		{
			AddFriendSendRspMsg reqMsg;
			if (reqMsg.FromString(pMsg->to_string())) {
				if (m_httpServer) {
					m_httpServer->On_AddFriendSendRspMsg(reqMsg);
				}
			}
		}break;
		case MessageType::RemoveFriendRsp_Type:
		{
			RemoveFriendRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())) {
				if (m_httpServer) {
					m_httpServer->On_RemoveFriendRspMsg(rspMsg);
				}
			}
		}break;
		case MessageType::AddFriendRecvReq_Type:
		{
			AddFriendRecvReqMsg reqMsg;
			if (reqMsg.FromString(pMsg->to_string())) {
				if (m_msgPersisUtil) {
					m_msgPersisUtil->Save_AddFriendRecvReqMsg(reqMsg);
				}
			}
		}break;
		case MessageType::FriendChatSendTxtMsgRsp_Type:
		{
			FriendChatSendTxtRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())) {
				if (m_httpServer) {
					m_httpServer->On_FriendChatSendTxtRsp(rspMsg);
				}
			}
		}break;
		case MessageType::AddFriendNotifyReq_Type:
		{
			AddFriendNotifyReqMsg reqMsg;
			if (reqMsg.FromString(pMsg->to_string())) {
				if (m_msgPersisUtil) {
					m_msgPersisUtil->Save_AddFriendNotifyReqMsg(reqMsg);
				}
			}
		}break;
		case MessageType::FriendChatReceiveTxtMsgReq_Type:
		{
			FriendChatRecvTxtReqMsg reqMsg;
			if (reqMsg.FromString(pMsg->to_string())) {
				if (m_msgPersisUtil) {
					//m_msgPersisUtil->Save_FriendChatRecvTxtReqMsg(reqMsg.);
				}
				{
					FriendChatRecvTxtRspMsg rspMsg;
					rspMsg.m_strFromId = reqMsg.m_chatMsg.m_strSenderId;
					rspMsg.m_strToId = reqMsg.m_chatMsg.m_strReceiverId;
					rspMsg.m_strMsgId = reqMsg.m_strMsgId;
					auto pSess = GetClientSess(rspMsg.m_strToId);
					if (pSess != nullptr)
					{
						auto pSend = std::make_shared<TransBaseMsg_t>(rspMsg.GetMsgType(), rspMsg.ToString());
						pSess->SendMsg(pSend);
					}
				}
			}

		}break;
		case MessageType::FriendUnReadMsgNotifyReq_Type:
		{
			FriendUnReadNotifyReqMsg reqMsg;
			if (reqMsg.FromString(pMsg->to_string())) {
				FriendUnReadNotifyRspMsg rspMsg;
				rspMsg.m_strMsgId = reqMsg.m_strMsgId;
				rspMsg.m_strUserId = reqMsg.m_strUserId;
				auto pSess = GetClientSess(rspMsg.m_strUserId);
				if (pSess != nullptr)
				{
					auto pSend = std::make_shared<TransBaseMsg_t>(rspMsg.GetMsgType(), rspMsg.ToString());
					pSess->SendMsg(pSend);
				}
			}

		}break;
		case MessageType::FileVerifyReq_Type:
		{
			FileVerifyReqMsg reqMsg;
			if (reqMsg.FromString(pMsg->to_string())) {
				HandleFileVerifyReq(reqMsg);
			}
		}break;
		case MessageType::GetFriendListRsp_Type:
		{
			GetFriendListRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())) {
				if (m_httpServer) {
					m_httpServer->On_GetFriendListRsp(rspMsg);
				}
			}
		}break;
		default:
		{
			LOG_ERR(ms_loger, "UnHandle MsgType:{} MsgContent:{} [{} {}]",MsgType(pMsg->GetType()),pMsg->to_string(), __FILENAME__, __LINE__);
		}break;
		}
	}
}

void CMediumServer::Handle_RecvFileOnlineRsp(const FriendRecvFileMsgRspMsg& rspMsg)
{
	if (rspMsg.m_eOption == E_FRIEND_OPTION::E_AGREE_ADD)
	{
		m_fileUtil.OpenWriteFile(rspMsg.m_nFileId+1, rspMsg.m_strFromId+"_"+std::to_string(rand())+"_"+m_fileUtil.GetFileNameFromPath(rspMsg.m_strFileName));
	}
}
/**
 * @brief 将消息转发到远端的服务器
 * 
 * @param pServerSess 连接客户端的会话
 * @param msg 消息
 */
void CMediumServer::SendFoward(const std::shared_ptr<CServerSess>& pServerSess,const TransBaseMsg_t& msg)
{
	{
		if (msg.GetType() == MessageType::GetFriendChatHistroyReq_Type)
		{
			GetFriendChatHistoryReq reqMsg;
			if (reqMsg.FromString(msg.to_string())) {
				GetFriendChatHistoryRsp rspMsg;
				rspMsg.m_strMsgId = reqMsg.m_strMsgId;
				rspMsg.m_strUserId = reqMsg.m_strUserId;
				rspMsg.m_strFriendId = reqMsg.m_strFriendId;
				rspMsg.m_strChatMsgId = reqMsg.m_strMsgId;
				rspMsg.m_msgHistory = m_msgPersisUtil->Get_FriendChatHistory(reqMsg);
				pServerSess->SendMsg(&rspMsg);
			}
			return;
		}
	}
	//对于原始消息，原封不动的转发
	auto item = m_ForwardSessMap.find(pServerSess);
	if (item != m_ForwardSessMap.end())
	{
		auto pMsg = std::make_shared<TransBaseMsg_t>(msg.GetType(), msg.to_string());
		item->second->SendMsg(pMsg);
	}
}

GetFriendChatHistoryRsp CMediumServer::HandleFriendChatHistoryReq(const GetFriendChatHistoryReq& reqMsg)
{
	GetFriendChatHistoryRsp result;
	if (m_msgPersisUtil)
	{
		auto rspMsgVec = m_msgPersisUtil->Get_FriendChatHistory(reqMsg);
		result.m_msgHistory = rspMsgVec;
	}
	result.m_strMsgId = reqMsg.m_strMsgId;
	result.m_strUserId = reqMsg.m_strUserId;
	result.m_strFriendId = reqMsg.m_strFriendId;
	result.m_strChatMsgId = reqMsg.m_strChatMsgId;
	return result;
}

GetGroupChatHistoryRsp CMediumServer::HandleGroupChatHistoryReq(const GetGroupChatHistoryReq& reqMsg)
{
	GetGroupChatHistoryRsp result;
	if (m_msgPersisUtil)
	{
		auto rspMsgVec = m_msgPersisUtil->Get_GroupChatHistory(reqMsg);
		result.m_msgHistory = rspMsgVec;
	}
	result.m_strMsgId = reqMsg.m_strMsgId;
	result.m_strUserId = reqMsg.m_strUserId;
	result.m_strGroupId = reqMsg.m_strGroupId;
	result.m_strChatMsgId = reqMsg.m_strChatMsgId;
	return result;
}

SearchChatHistoryRsp CMediumServer::HandleSearchChatHistoryReq(const SearchChatHistoryReq& reqMsg)
{
	SearchChatHistoryRsp result;
	
	if (m_msgPersisUtil)
	{
		auto friendMsgVec = m_msgPersisUtil->Get_SearchFriendChatMsg(reqMsg);
		auto groupMsgVec = m_msgPersisUtil->Get_SearchGroupChatMsg(reqMsg);
		result.m_friendChatMsgVec = friendMsgVec;
		result.m_groupChatMsgVec = groupMsgVec;
	}
	result.m_strMsgId = reqMsg.m_strMsgId;
	result.m_strUserId = reqMsg.m_strUserId;
	return result;
}

void CMediumServer::HandleMsg(const TransBaseMsg_t& msg)
{
	switch (msg.GetType())
	{
	case MessageType::FriendChatReceiveTxtMsgReq_Type:
	{
		FriendChatRecvTxtReqMsg reqMsg;
		if (reqMsg.FromString(msg.to_string())) {
			m_msgPersisUtil->Save_FriendChatSendTxtRspMsg(reqMsg.m_chatMsg);
		}
	}break;
	case MessageType::FriendChatSendTxtMsgRsp_Type:
	{
		FriendChatSendTxtRspMsg rspMsg;
		if (rspMsg.FromString(msg.to_string())) {
			m_msgPersisUtil->Save_FriendChatSendTxtRspMsg(rspMsg.m_chatMsg);
			//m_msgPersisUtil->Save_FriendChatSendTxtRspMsg(rspMsg);
		}
	}break;
	default:
	{

	}break;
	}
}
}