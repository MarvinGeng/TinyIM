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
		auto httpCfg = cfg["httpserver"];
		m_httpCfg.m_strServerIp = httpCfg["ip"].string_value();
		m_httpCfg.m_nPort = httpCfg["port"].int_value();
		LOG_INFO(ms_loger, "Http Server:{} {} [{} {}]", m_httpCfg.m_strServerIp, m_httpCfg.m_nPort, __FILENAME__, __LINE__);
	}

	{
		auto udpJson = cfg["UdpServer"];
		m_udpCfg.m_strServerIp = udpJson["ip"].string_value();
		m_udpCfg.m_nPort = udpJson["port"].int_value();
	}
}
void CMediumServer::Handle_UdpMsg(const asio::ip::udp::endpoint endPt, const UdpP2pStartRspMsg& reqMsg)
{
	if (m_userKeepAliveMap.find(reqMsg.m_strUserId) != m_userKeepAliveMap.end())
	{
		LOG_INFO(ms_loger, "P2P Line OK");
	}
	else
	{
		m_userKeepAliveMap.insert({ reqMsg.m_strUserId,time(nullptr) });
	}
}

void CMediumServer::Handle_UdpMsg(const asio::ip::udp::endpoint endPt, const UdpP2pStartReqMsg& reqMsg)
{
	UdpP2pStartRspMsg rspMsg;
	rspMsg.m_strMsgId = reqMsg.m_strMsgId;
	rspMsg.m_strUserId = reqMsg.m_strFriendId;
	rspMsg.m_strFriendId = reqMsg.m_strUserId;
	auto pUdpSess = GetUdpSess(reqMsg.m_strFriendId);
	if (pUdpSess)
	{
		pUdpSess->send_msg(endPt,&rspMsg);
	}
}
void CMediumServer::Handle_UdpMsg(const asio::ip::udp::endpoint endPt, const KeepAliveRspMsg& Msg)
{
	if (m_userKeepAliveMap.find(Msg.m_strClientId) == m_userKeepAliveMap.end())
	{
		UdpP2pStartReqMsg reqMsg;
		reqMsg.m_strMsgId = m_httpServer->GenerateMsgId();
		reqMsg.m_strUserId = Msg.m_strClientId;
		reqMsg.m_strFriendId = Msg.m_strClientId;
		auto pUdpSess = GetUdpSess(Msg.m_strClientId);
		if (pUdpSess)
		{
			pUdpSess->send_msg(endPt, &reqMsg);
		}
		else
		{
			LOG_ERR(ms_loger, "No Udp Sess For {} [{} {}]", Msg.m_strClientId, __FILENAME__, __LINE__);
		}
	}
}

/**
 * @brief 处理从UDP收到的文件数据发送请求消息(点对点的方式发送文件数据)
 * 
 * @param endPt UDP消息的发送者地址
 * @param reqMsg 文件数据发送请求消息
 */
void CMediumServer::Handle_UdpMsg(const asio::ip::udp::endpoint endPt, const FileDataSendReqMsg& reqMsg)
{
	if (reqMsg.m_nDataIndex <= reqMsg.m_nDataTotalCount)
	{
		m_fileUtil.OnWriteData(reqMsg.m_nFileId + 1, reqMsg.m_szData, reqMsg.m_nDataLength);
		FileDataSendRspMsg rspMsg;
		rspMsg.m_strMsgId = reqMsg.m_strMsgId;
		rspMsg.m_nFileId = reqMsg.m_nFileId;
		rspMsg.m_strUserId = reqMsg.m_strUserId;
		rspMsg.m_strFriendId = reqMsg.m_strFriendId;
		rspMsg.m_nDataTotalCount = reqMsg.m_nDataTotalCount;
		rspMsg.m_nDataIndex = reqMsg.m_nDataIndex;
		auto pSess = GetUdpSess(reqMsg.m_strUserId);
		if (pSess)
		{
			pSess->send_msg(endPt, &rspMsg);
		}
	}
}
void CMediumServer::HandleSendBack(const std::shared_ptr<CClientSess>& pClientSess,const FileDataSendRspMsg& rspMsg)
{
	auto pMsg = DoSendBackFileDataSendRsp(rspMsg);
	pClientSess->SendMsg(pMsg);
}

TransBaseMsg_S_PTR CMediumServer::DoSendBackFileDataSendRsp(const FileDataSendRspMsg& rspMsg)
{
	if (rspMsg.m_nDataIndex < rspMsg.m_nDataTotalCount)
	{
		FileDataSendReqMsg reqMsg;
		if (m_fileUtil.OnReadData(rspMsg.m_nFileId, reqMsg.m_szData, reqMsg.m_nDataLength, 1024))
		{
			LOG_INFO(ms_loger, "Read Data ", rspMsg.m_nFileId);
			reqMsg.m_strMsgId = m_httpServer->GenerateMsgId();
			reqMsg.m_nFileId = rspMsg.m_nFileId;
			reqMsg.m_strUserId = rspMsg.m_strUserId;
			reqMsg.m_strFriendId = rspMsg.m_strFriendId;
			reqMsg.m_nDataTotalCount = rspMsg.m_nDataTotalCount;
			reqMsg.m_nDataIndex = rspMsg.m_nDataIndex + 1;
		}
		auto pResult = std::make_shared<TransBaseMsg_t>(reqMsg.GetMsgType(), reqMsg.ToString());
		return pResult;
	}
	else
	{
		FileVerifyReqMsg verifyReqMsg;
		verifyReqMsg.m_strMsgId = m_httpServer->GenerateMsgId();
		verifyReqMsg.m_nFileId = rspMsg.m_nFileId;
		verifyReqMsg.m_strUserId = rspMsg.m_strUserId;
		verifyReqMsg.m_strFriendId = rspMsg.m_strFriendId;
		std::string strFileName = m_fileUtil.GetFileName(rspMsg.m_nFileId);
		verifyReqMsg.m_strFileName = m_fileUtil.GetFileNameFromPath(strFileName);
		m_fileUtil.GetFileSize(verifyReqMsg.m_nFileSize, strFileName);
		verifyReqMsg.m_strFileHash = m_fileUtil.CalcHash(strFileName);
		m_fileUtil.OnCloseFile(rspMsg.m_nFileId);
		auto pResult = std::make_shared<TransBaseMsg_t>(verifyReqMsg.GetMsgType(), verifyReqMsg.ToString());
		return pResult;
	}
}
/**
 * @brief 处理从UDP收到的文件数据发送回复消息
 * 
 * @param endPt UDP消息的来源地址
 * @param rspMsg 文件数据发送回复消息
 */
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
			reqMsg.m_strUserId = rspMsg.m_strUserId;
			reqMsg.m_strFriendId = rspMsg.m_strFriendId;
			reqMsg.m_nDataTotalCount = rspMsg.m_nDataTotalCount;
			reqMsg.m_nDataIndex = rspMsg.m_nDataIndex + 1;
			auto pUdpSess = GetUdpSess(reqMsg.m_strUserId);
			if(pUdpSess)
			{
				pUdpSess->send_msg(endPt, &reqMsg);
			}
			else
			{
				LOG_ERR(ms_loger, "UDP Sess Failed:{}", reqMsg.m_strUserId);
			}
		}
	}
	else
	{
		FileVerifyReqMsg verifyReqMsg;
		verifyReqMsg.m_strMsgId = m_httpServer->GenerateMsgId();
		verifyReqMsg.m_nFileId = rspMsg.m_nFileId;
		verifyReqMsg.m_strUserId = rspMsg.m_strUserId;
		verifyReqMsg.m_strFriendId = rspMsg.m_strFriendId;
		std::string strFileName = m_fileUtil.GetFileName(rspMsg.m_nFileId);
		verifyReqMsg.m_strFileName = m_fileUtil.GetFileNameFromPath(strFileName);
		m_fileUtil.GetFileSize(verifyReqMsg.m_nFileSize, strFileName);
		verifyReqMsg.m_strFileHash = m_fileUtil.CalcHash(strFileName);
		auto pSess = GetClientSess(verifyReqMsg.m_strUserId);
		if (pSess != nullptr)
		{
			auto pSend = std::make_shared<TransBaseMsg_t>(verifyReqMsg.GetMsgType(), verifyReqMsg.ToString());
			pSess->SendMsg(pSend);
		}
		m_fileUtil.OnCloseFile(rspMsg.m_nFileId);
	}
}

/**
 * @brief 处理UDP消息,用于UDP的Client的回调
 * 
 * @param endPt 远端UDP的地址
 * @param pMsg 从UDP收到的消息
 */
void CMediumServer::DispatchUdpMsg(const asio::ip::udp::endpoint endPt, TransBaseMsg_t* pMsg)
{
	if (pMsg)
	{
		LOG_INFO(ms_loger, "Recv UDP Msg:{} {} [{} {}]", MsgType(pMsg->GetType()), pMsg->to_string(), __FILENAME__, __LINE__);

		switch (pMsg->GetType())
		{
		case E_MsgType::KeepAliveRsp_Type:
		{
			KeepAliveRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())) {
				Handle_UdpMsg(endPt, rspMsg);
				LOG_INFO(ms_loger, "{}", rspMsg.ToString());
			}
		}break;
		case E_MsgType::FileSendDataReq_Type:
		{
			FileDataSendReqMsg reqMsg;
			if (reqMsg.FromString(pMsg->to_string())) {
				Handle_UdpMsg(endPt, reqMsg);
			}
		}break;
		case E_MsgType::FileSendDataRsp_Type:
		{
			FileDataSendRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string()))
			{
				Handle_UdpMsg(endPt, rspMsg);
			}
		}break;
		case E_MsgType::FileRecvDataReq_Type:
		{
			FileDataRecvReqMsg reqMsg;
			if (reqMsg.FromString(pMsg->to_string())) {
				Handle_UdpMsg(endPt, reqMsg);
			}
		}break;
		case E_MsgType::UdpP2PStartReq_Type:
		{
			UdpP2pStartReqMsg reqMsg;
			if (reqMsg.FromString(pMsg->to_string())) {
				Handle_UdpMsg(endPt, reqMsg);
			}
		}break;
		case E_MsgType::UdpP2PStartRsp_Type:
		{
			UdpP2pStartRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())) {
				Handle_UdpMsg(endPt, rspMsg);
			}
		}break;
		default:
		{
			LOG_ERR(ms_loger, "Unhandle Msg:{} {} [{} {}]", MsgType(pMsg->GetType()), pMsg->to_string(), __FILENAME__, __LINE__);
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
		LOG_INFO(ms_loger, "Http Server On :{} [{} {} ]", m_httpCfg.m_nPort,__FILENAME__,__LINE__);
		m_httpServer->Start(m_httpCfg.m_nPort);

		m_freeClientSess = std::make_shared<CClientSess>(m_ioService,
			m_clientCfgVec[0].m_strServerIp,
			m_clientCfgVec[0].m_nPort, this);

		m_freeClientSess->StartConnect();


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

void CMediumServer::CheckFriendP2PConnect()
{
	if (m_userFriendListMap.empty())
	{
		LOG_ERR(ms_loger, "CheckFriendP2PConnect No User");
	}
	else
	{
		LOG_INFO(ms_loger, "CheckFriendP2PConnect Begin");
	}
	for (auto userItem : m_userFriendListMap)
	{
		auto pUdpSess = GetUdpSess(userItem.first);
		if (pUdpSess)
		{
			for (auto friItem : userItem.second)
			{
				auto item = m_userIdUdpAddrMap.find(friItem);
				if (item != m_userIdUdpAddrMap.end())
				{
					UdpP2pStartReqMsg reqMsg;
					reqMsg.m_strMsgId = m_httpServer->GenerateMsgId();
					reqMsg.m_strUserId = userItem.first;
					reqMsg.m_strFriendId = friItem;
					pUdpSess->send_msg(item->second.m_strServerIp, item->second.m_nPort, &reqMsg);
				}
				else
				{
					auto pTcpSess = GetClientSess(userItem.first);
					if (pTcpSess)
					{
						QueryUserUdpAddrReqMsg reqMsg;
						reqMsg.m_strMsgId = m_httpServer->GenerateMsgId();
						reqMsg.m_strUserId = userItem.first;
						reqMsg.m_strUdpUserId = friItem;
						pTcpSess->SendMsg(&reqMsg);
					}
				}
			}
		}
		else
		{

		}
	}
}

bool CMediumServer::HandleSendBack(const std::shared_ptr<CClientSess>& pClientSess, const GetFriendListRspMsg& msg)
{
	//if(m_userFriendListMap.find(pClientSess->UserId()) == m_userFriendListMap.end())
	//{
		m_userFriendListMap.erase(pClientSess->UserId());
		std::vector<std::string> strFriendVec;
		for (auto teamItem : msg.m_teamVec)
		{
			for (auto friItem : teamItem.m_teamUsers) {
				strFriendVec.push_back(friItem.m_strUserId);
			}
		}
		m_userFriendListMap.insert({ pClientSess->UserId(),strFriendVec });
	//}
		return true;
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

	for (auto& item : m_userUdpSessMap)
	{
		KeepAliveReqMsg reqMsg;
		reqMsg.m_strClientId = item.first;
		item.second->sendToServer(&reqMsg);
	}
	CheckFriendP2PConnect();
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
               

			   if (!m_clientCfgVec.empty() )
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

/**
 * @brief 根据用户名获取用户ID
 * 
 * @param strUserName 用户名
 * @return std::string 用户ID
 */
std::string CMediumServer::GetUserId(const std::string strUserName)
{
	auto item = m_userName_UserIdMap.find(strUserName);
	if (item != m_userName_UserIdMap.end())
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

/**
 * @brief 创建连接到服务器的TCP连接
 * 
 * @return CClientSess_SHARED_PTR 到服务器的TCP连接
 */
	CClientSess_SHARED_PTR CMediumServer::CreateClientSess()
	{
		auto pReturn = m_freeClientSess;
		m_freeClientSess = std::make_shared<CClientSess>(m_ioService,
			m_clientCfgVec[0].m_strServerIp,
			m_clientCfgVec[0].m_nPort, this);

		m_freeClientSess->StartConnect();
		return pReturn;
	}

	CClientSess_SHARED_PTR CMediumServer::CreateClientSess(const std::string strUserName)
	{
		if (strUserName.empty())
		{
			return nullptr;
		}
		auto pReturn = m_freeClientSess;
		m_freeClientSess = std::make_shared<CClientSess>(m_ioService,
			m_clientCfgVec[0].m_strServerIp,
			m_clientCfgVec[0].m_nPort, this);

		m_freeClientSess->StartConnect();
		m_userClientSessMap.insert(std::pair<std::string, CClientSess_SHARED_PTR>(strUserName, pReturn));
		return pReturn;
	}

/**
 * @brief 检查待发送消息
 * 
 */
void CMediumServer::CheckWaitMsgVec()
{
	decltype(m_RecvWaitMsgMap) notSendMap;
	if (m_timeCount % 10 == 0)
	{
		for (auto item : m_RecvWaitMsgMap)
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
		m_RecvWaitMsgMap.clear();
		m_RecvWaitMsgMap = notSendMap;
	}
}
/**
 * @brief 响应定时器的任务，一些需要定时处理的任务，可以写成一个函数，在此函数中调用
 * 
 */
void CMediumServer::OnTimer()
{
	if (m_httpServer)
	{
		m_httpServer->OnTimer();
	}
	m_timeCount++;
	CheckAllConnect();
	if (m_timeCount % 60 == 0)
	{
		if (!m_BackSessMap.empty())
		{

			CheckWaitMsgVec();
		}
		else
		{
			m_nNoSessTimeCount++;
			LOG_ERR(ms_loger, "No Sess Count {} [ {} {} ]", m_timeCount, __FILENAME__, __LINE__);
		}
	}
}

/**
 * @brief 设置定时器，单位秒
 * 
 * @param nSeconds 定时的秒数
 */
void CMediumServer::SetTimer(int nSeconds)
{
	if(m_timer && m_nNoSessTimeCount < 30)
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
	else
	{
		try
		{
			m_ioService.stop();
		}
		catch (std::exception ec)
		{
			LOG_ERR(ms_loger, "{} [{} {}]",ec.what(), __FILENAME__, __LINE__);
		}
	}
}

/**
 * @brief 处理好友聊天发送文本消息的回复
 * 
 * @param rspMsg 
 */
void CMediumServer::HandleFriendChatSendTextMsgRsp(const FriendChatSendTxtRspMsg& rspMsg)
{
	LOG_ERR(ms_loger,"{}",rspMsg.ToString());
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
void CMediumServer::SendBack(const std::shared_ptr<CClientSess>& pClientSess, const TransBaseMsg_t& msg)
{
	if (HandleSendBack(pClientSess, msg))
	{

	}
	else
	{
		auto pMsg = std::make_shared<TransBaseMsg_t>(msg.GetType(), msg.to_string());
		auto item = m_BackSessMap.find(pClientSess);
		if (item != m_BackSessMap.end())
		{
			item->second->SendMsg(pMsg);
		}
		else
		{
			OnHttpRsp(pMsg);
		}

	}
}

/**
 * @brief 处理文件校验请求,在发送文件完成以后进行
 * 
 * @param msg 文件校验请求消息
 */
void CMediumServer::HandleFileVerifyReq(const FileVerifyReqMsg& msg)
{
	FileVerifyRspMsg rspMsg;
	m_fileUtil.OnCloseFile(msg.m_nFileId + 1);
	std::string strFileName = m_fileUtil.GetCurDir() + msg.m_strUserId + "\\" + msg.m_strFileName;
	std::string strRecvHash = m_fileUtil.CalcHash(strFileName);
	//
	if (msg.m_strFileHash == strRecvHash)
	{

		auto hashItem = std::find(m_fileHashTransVec.begin(), m_fileHashTransVec.end(), strRecvHash);
		if (hashItem != m_fileHashTransVec.end())
		{
			m_fileHashTransVec.erase(hashItem);
		}

		rspMsg.m_eErrCode = ERROR_CODE_TYPE::E_CODE_SUCCEED;
		{
			auto item = m_fileHashMsgIdMap.find(strRecvHash);
			if (item != m_fileHashMsgIdMap.end()) {
				auto msgItem = m_waitImageMsgMap.find(item->second);
				if (msgItem != m_waitImageMsgMap.end()) {
					ChatMsgElemVec oldVec = MsgElemVec(msgItem->second.m_chatMsg.m_strContext);
					ChatMsgElemVec newVec;
					for (auto item : oldVec) {
						if (item.m_eType == CHAT_MSG_TYPE::E_CHAT_IMAGE_TYPE) {
							ChatMsgElem elem;
							elem.m_eType = item.m_eType;
							elem.m_strImageName = strFileName;
							newVec.push_back(elem);
						}
						else {
							newVec.push_back(item);
						}
					}
					msgItem->second.m_chatMsg.m_strContext = MsgElemVec(newVec);
					auto pSess = GetSendBackSess(msgItem->second.m_chatMsg.m_strReceiverId);
					if (pSess)
					{
						pSess->SendMsg(&(msgItem->second));
					}
					else
					{
						LOG_ERR(ms_loger, "UserId:{} No GUI Sess [{} {}]", msgItem->second.m_chatMsg.m_strReceiverId, __FILENAME__, __LINE__);
					}

					{
						auto pMsgUtil = GetMsgPersisUtil(msgItem->second.m_chatMsg.m_strReceiverId);
						if (pMsgUtil)
						{
							LOG_ERR(ms_loger, "UserId:{} No Msg Util[ {} {} ]", msgItem->second.m_chatMsg.m_strReceiverId, __FILENAME__, __LINE__);
						}
						else
						{
							//TODO: 数据记录保存到数据库会Crash
							//pMsgUtil->Save_FriendChatSendTxtRspMsg(msgItem->second.m_chatMsg);
						}

					}
					{
						FriendChatRecvTxtRspMsg rspMsg;
						rspMsg.m_strMsgId = msgItem->second.m_strMsgId;
						rspMsg.m_strFriendId = msgItem->second.m_chatMsg.m_strSenderId;
						rspMsg.m_strUserId = msgItem->second.m_chatMsg.m_strReceiverId;
						rspMsg.m_strChatMsgId = msgItem->second.m_chatMsg.m_strChatMsgId;
						auto pClientSess = GetClientSess(rspMsg.m_strUserId);
						if (pClientSess)
						{
							pClientSess->SendMsg(&rspMsg);
						}
					}
					m_waitImageMsgMap.erase(item->second);
					m_fileHashMsgIdMap.erase(strRecvHash);

				}
			}
		}
		auto pMsgUtil = GetMsgPersisUtil(msg.m_strUserId);
		if (pMsgUtil)
		{
			pMsgUtil->Save_FileHash(strFileName, strRecvHash);
		}
		else
		{
			LOG_ERR(ms_loger, "UserId:{} No Msg Util[ {} {} ]", msg.m_strUserId, __FILENAME__, __LINE__);
		}
		LOG_INFO(ms_loger, "Download File {} Finished [{} {}]", strFileName, __FILENAME__, __LINE__);

	}
	else
	{
		m_fileUtil.RemoveFile(strFileName);
		rspMsg.m_eErrCode = ERROR_CODE_TYPE::E_CODE_LOGIN_FAILED;
	}
	rspMsg.m_strMsgId = msg.m_strMsgId;
	rspMsg.m_strFileName = msg.m_strFileName;
	rspMsg.m_strUserId = msg.m_strUserId;
	rspMsg.m_strFriendId = msg.m_strFriendId;
	rspMsg.m_nFileId = msg.m_nFileId;
	auto pSess = GetClientSess(rspMsg.m_strUserId);
	if (pSess != nullptr)
	{
		auto pSend = std::make_shared<TransBaseMsg_t>(rspMsg.GetMsgType(), rspMsg.ToString());
		pSess->SendMsg(pSend);
	}

}

void CMediumServer::HandleSendBack(const std::shared_ptr<CClientSess>& pClientSess, const FileVerifyRspMsg& rspMsg)
{
	if (rspMsg.m_eErrCode == ERROR_CODE_TYPE::E_CODE_SUCCEED)
	{
		auto item = m_SendWaitMsgMap.find(rspMsg.m_strFileHash);
		if (item != m_SendWaitMsgMap.end()) {
			pClientSess->SendMsg(&(item->second));
			m_SendWaitMsgMap.erase(rspMsg.m_strFileHash);
		}
	}
}

/**
 * @brief 处理对于已经接收或者拒绝接收文件的通知消息
 * 
 * @param notifyMsg 文件接收结果通知消息
 */
void CMediumServer::HandleFriendNotifyFileMsgReq(const FriendNotifyFileMsgReqMsg& notifyMsg)
{
	LOG_ERR(ms_loger,"{}",notifyMsg.ToString());

	/*if (notifyMsg.m_eOption == E_FRIEND_OPTION::E_AGREE_ADD)
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
				auto pUdpSess = GetUdpSess(sendReqMsg.m_strFromId);
				if (pUdpSess)
				{
					auto udpItem = m_userIdUdpAddrMap.find(sendReqMsg.m_strToId);
					if (udpItem != m_userIdUdpAddrMap.end())
					{
						pUdpSess->send_msg(udpItem->second.m_strServerIp, udpItem->second.m_nPort, &sendReqMsg);
					}
					else
					{
						pUdpSess->sendToServer(&sendReqMsg);
					}
				}
				else
				{
					LOG_ERR(ms_loger, "UDP Sess Failed:{}", sendReqMsg.m_strFromId);
				}
			}
		}
	}*/
}

void CMediumServer::Handle_TcpMsg(const std::shared_ptr<CClientSess>& pClientSess, const FileDataRecvReqMsg& reqMsg)
{
	if (reqMsg.m_nDataIndex <= reqMsg.m_nDataTotalCount)
	{
		m_fileUtil.OnWriteData(reqMsg.m_nFileId + 1, reqMsg.m_szData, reqMsg.m_nDataLength);
		LOG_INFO(ms_loger, "WriteData ", reqMsg.m_nFileId);
		FileDataRecvRspMsg rspMsg;
		rspMsg.m_strMsgId = reqMsg.m_strMsgId;
		rspMsg.m_nFileId = reqMsg.m_nFileId;
		rspMsg.m_strUserId = reqMsg.m_strUserId;
		rspMsg.m_strFriendId = reqMsg.m_strFriendId;
		rspMsg.m_nDataTotalCount = reqMsg.m_nDataTotalCount;
		rspMsg.m_nDataIndex = reqMsg.m_nDataIndex;
		pClientSess->SendMsg(&rspMsg);
	}

	if (reqMsg.m_nDataIndex == reqMsg.m_nDataTotalCount)
	{
		m_fileUtil.OnCloseFile(reqMsg.m_nFileId + 1);
	}
}
/**
 * @brief 处理从UDP收到文件数据的接收请求消息
 * 
 * @param endPt 消息发送者的地址
 * @param reqMsg 文件数据接收消息
 */
void CMediumServer::Handle_UdpMsg(const asio::ip::udp::endpoint endPt, const FileDataRecvReqMsg& reqMsg)
{
	if (reqMsg.m_nDataIndex <= reqMsg.m_nDataTotalCount)
	{
		m_fileUtil.OnWriteData(reqMsg.m_nFileId + 1, reqMsg.m_szData, reqMsg.m_nDataLength);
		LOG_INFO(ms_loger, "WriteData ", reqMsg.m_nFileId);
		FileDataRecvRspMsg rspMsg;
		rspMsg.m_strMsgId = reqMsg.m_strMsgId;
		rspMsg.m_nFileId = reqMsg.m_nFileId;
		rspMsg.m_strUserId = reqMsg.m_strUserId;
		rspMsg.m_strFriendId = reqMsg.m_strFriendId;
		rspMsg.m_nDataTotalCount = reqMsg.m_nDataTotalCount;
		rspMsg.m_nDataIndex = reqMsg.m_nDataIndex;
		auto pUdpSess = GetUdpSess(reqMsg.m_strUserId);
		if (pUdpSess)
		{
			pUdpSess->send_msg(endPt,&rspMsg);
		}
		else
		{
			//LOG_ERR(ms_loger, "UDP Sess Failed:{}", reqMsg.m_strFromId);
		}
	}
	
	if(reqMsg.m_nDataIndex == reqMsg.m_nDataTotalCount)
	{
		m_fileUtil.OnCloseFile(reqMsg.m_nFileId + 1);
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
		case E_MsgType::UserRegisterRsp_Type:
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
		case E_MsgType::UserUnRegisterRsp_Type:
		{
			UserUnRegisterRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())) {
				if (m_httpServer) {
					m_httpServer->On_UserUnRegisterRsp(rspMsg);
				}
			}
		}break;
		case E_MsgType::UserLoginRsp_Type:
		{
			UserLoginRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())) {
				if (m_httpServer) {
					m_httpServer->On_UserLoginRsp(rspMsg);
				}
			}
		}break;
		case E_MsgType::UserLogoutRsp_Type:
		{
			UserLogoutRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())) {
				if (m_httpServer) {
					m_httpServer->On_UserLogoutRsp(rspMsg);
				}
			}
		}break;
		case E_MsgType::FindFriendRsp_Type:
		{
			FindFriendRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())) {
				if (m_httpServer) {
					m_httpServer->On_FindFriendRsp(rspMsg);
				}
			}
		}break;
		case E_MsgType::AddTeamRsp_Type:
		{
			AddTeamRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())) {
				if (m_httpServer) {
					m_httpServer->On_AddFriendTeamRsp(rspMsg);
				}
			}
		}break;
		case E_MsgType::RemoveTeamRsp_Type:
		{
			RemoveTeamRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())) {
				if (m_httpServer) {
					m_httpServer->On_RemoveFriendTeamRsp(rspMsg);
				}
			}
		}break;
		case E_MsgType::MoveFriendToTeamRsp_Type:
		{
			MoveFriendToTeamRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())) {
				if (m_httpServer) {
					m_httpServer->On_MoveFriendToTeamRsp(rspMsg);
				}
			}
		}break;
		case E_MsgType::AddToGroupRsp_Type:
		{
			AddToGroupRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())) {
				if (m_httpServer) {
					m_httpServer->On_AddToGroupRsp(rspMsg);
				}
			}
		}break;
		case E_MsgType::CreateGroupRsp_Type:
		{
			CreateGroupRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())) {
				if (m_httpServer) {
					m_httpServer->On_CreateGroupRsp(rspMsg);
				}
			}
		}break;
		case E_MsgType::GetGroupListRsp_Type:
		{
			GetGroupListRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())) {
				if (m_httpServer) {
					m_httpServer->On_GetGroupListRsp(rspMsg);
				}
			}
		}break;
		case E_MsgType::DestroyGroupRsp_Type:
		{
			DestroyGroupRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())) {
				if (m_httpServer) {
					m_httpServer->On_DestroyGroupRsp(rspMsg);
				}
			}
		}break;
		case E_MsgType::FindGroupRsp_Type:
		{
			FindGroupRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())){
				if (m_httpServer) {
					m_httpServer->On_FindGroupRsp(rspMsg);
				}
			}
		}break;
		case E_MsgType::SendGroupTextMsgRsp_Type:
		{
			SendGroupTextMsgRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())) {
				if (m_httpServer) {
					m_httpServer->On_SendGroupTextMsgRsp(rspMsg);
				}
			}
		}break;
		case E_MsgType::RecvGroupTextMsgReq_Type:
		{
			RecvGroupTextMsgReqMsg reqMsg;
			if (reqMsg.FromString(pMsg->to_string())) {
				//if (m_msgPersisUtil) {
				//	m_msgPersisUtil->Save_RecvGroupTextMsgReqMsg(reqMsg);
				//}
			}
		}break;
		case E_MsgType::FriendSendFileMsgRsp_Type:
		{
			FriendSendFileMsgRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())) {
				if (m_httpServer) {
					m_httpServer->On_SendFriendFileOnlineRspMsg(rspMsg);
				}
			}
		}break;
		case E_MsgType::FriendRecvFileMsgReq_Type:
		{
			FriendRecvFileMsgReqMsg reqMsg;
			if (reqMsg.FromString(pMsg->to_string())) {
				//if (m_msgPersisUtil) {
				//	m_msgPersisUtil->Save_FriendRecvFileMsgReqMsg(reqMsg);
				//}
			}
		}break;
		case E_MsgType::FriendNotifyFileMsgReq_Type:
		{
			FriendNotifyFileMsgReqMsg reqMsg;
			if (reqMsg.FromString(pMsg->to_string())) {
				//if (m_msgPersisUtil) {
				//	m_msgPersisUtil->Save_FriendNotifyFileMsgReqMsg(reqMsg);
				//}
				HandleFriendNotifyFileMsgReq(reqMsg);
			}
		}break;
		case E_MsgType::AddFriendSendRsp_Type:
		{
			AddFriendSendRspMsg reqMsg;
			if (reqMsg.FromString(pMsg->to_string())) {
				if (m_httpServer) {
					m_httpServer->On_AddFriendSendRspMsg(reqMsg);
				}
			}
		}break;
		case E_MsgType::RemoveFriendRsp_Type:
		{
			RemoveFriendRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())) {
				if (m_httpServer) {
					m_httpServer->On_RemoveFriendRspMsg(rspMsg);
				}
			}
		}break;
		case E_MsgType::AddFriendRecvReq_Type:
		{
			AddFriendRecvReqMsg reqMsg;
			if (reqMsg.FromString(pMsg->to_string())) {
				auto pUtil = GetMsgPersisUtil(reqMsg.m_strUserId);
				if (pUtil)
				{
					pUtil->Save_AddFriendRecvReqMsg(reqMsg);
				}
			}
		}break;
		case E_MsgType::FriendChatSendTxtMsgRsp_Type:
		{
			FriendChatSendTxtRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())) {
				if (m_httpServer) {
					m_httpServer->On_FriendChatSendTxtRsp(rspMsg);
				}
			}
		}break;
		case E_MsgType::AddFriendNotifyReq_Type:
		{
			AddFriendNotifyReqMsg reqMsg;
			if (reqMsg.FromString(pMsg->to_string())) {
				auto pUtil = GetMsgPersisUtil(reqMsg.m_strUserId);
				if (pUtil)
				{
					pUtil->Save_AddFriendNotifyReqMsg(reqMsg);
				}
			}
		}break;
		case E_MsgType::FriendChatReceiveTxtMsgReq_Type:
		{
			FriendChatRecvTxtReqMsg reqMsg;
			if (reqMsg.FromString(pMsg->to_string())) {
				//if (m_msgPersisUtil) {
				//	//m_msgPersisUtil->Save_FriendChatRecvTxtReqMsg(reqMsg.);
				//}
				//{
				//	FriendChatRecvTxtRspMsg rspMsg;
				//	rspMsg.m_strFriendId = reqMsg.m_chatMsg.m_strSenderId;
				//	rspMsg.m_strUserId = reqMsg.m_chatMsg.m_strReceiverId;
				//	rspMsg.m_strMsgId = reqMsg.m_strMsgId;
				//	auto pSess = GetClientSess(rspMsg.m_strUserId);
				//	if (pSess != nullptr)
				//	{
				//		auto pSend = std::make_shared<TransBaseMsg_t>(rspMsg.GetMsgType(), rspMsg.ToString());
				//		pSess->SendMsg(pSend);
				//	}
				//}
			}

		}break;
		case E_MsgType::FriendUnReadMsgNotifyReq_Type:
		{
			//FriendUnReadNotifyReqMsg reqMsg;
			//if (reqMsg.FromString(pMsg->to_string())) {
			//	FriendUnReadNotifyRspMsg rspMsg;
			//	rspMsg.m_strMsgId = reqMsg.m_strMsgId;
			//	rspMsg.m_strUserId = reqMsg.m_strUserId;
			//	auto pSess = GetClientSess(rspMsg.m_strUserId);
			//	if (pSess != nullptr)
			//	{
			//		auto pSend = std::make_shared<TransBaseMsg_t>(rspMsg.GetMsgType(), rspMsg.ToString());
			//		pSess->SendMsg(pSend);
			//	}
			//}

		}break;
		case E_MsgType::FileVerifyReq_Type:
		{
			FileVerifyReqMsg reqMsg;
			if (reqMsg.FromString(pMsg->to_string())) {
				HandleFileVerifyReq(reqMsg);
			}
		}break;
		case E_MsgType::GetFriendListRsp_Type:
		{
			GetFriendListRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())) {
				if (m_httpServer) {
					m_httpServer->On_GetFriendListRsp(rspMsg);
				}
			}
		}break;
		case E_MsgType::GetRandomUserRsp_Type:
		{
			GetRandomUserRspMsg rspMsg;
			if (rspMsg.FromString(pMsg->to_string())) {
				if (m_httpServer) {
					m_httpServer->On_RandomUserRsp(rspMsg);
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

/**
 * @brief 处理接收在线文件请求回复消息
 * 
 * @param rspMsg 好友文件请求回复消息
 */
void CMediumServer::Handle_RecvFileOnlineRsp(const FriendRecvFileMsgRspMsg& rspMsg)
{
	if (rspMsg.m_eOption == E_FRIEND_OPTION::E_AGREE_ADD)
	{
		m_fileUtil.OpenWriteFile(rspMsg.m_nFileId+1, rspMsg.m_strFromId+"_"+std::to_string(rand())+"_"+m_fileUtil.GetFileNameFromPath(rspMsg.m_strFileName));
	}
}

/**
 * @brief 处理GUI客户端发过来的请求好友聊天记录的消息
 * 
 * @param pServerSess GUI的socket会话
 * @param reqMsg 获取好友聊天记录的请求消息
 */
void CMediumServer::HandleSendForward(const std::shared_ptr<CServerSess>& pServerSess, const GetFriendChatHistoryReq& reqMsg)
{
	GetFriendChatHistoryRsp rspMsg;
	rspMsg.m_strMsgId = reqMsg.m_strMsgId;
	rspMsg.m_strUserId = reqMsg.m_strUserId;
	rspMsg.m_strFriendId = reqMsg.m_strFriendId;
	rspMsg.m_strChatMsgId = reqMsg.m_strMsgId;
	auto pMsgUtil = GetMsgPersisUtil(reqMsg.m_strUserId);
	if (pMsgUtil)
	{
		//pMsgUtil->Save_FriendChatSendTxtRspMsg(reqMsg.m_chatMsg);
		rspMsg.m_msgHistory = pMsgUtil->Get_FriendChatHistory(reqMsg);

	}
	else
	{
		LOG_ERR(ms_loger, "UserId {} No Msg Util [{} {}] ", reqMsg.m_strUserId, __FILENAME__, __LINE__);
	}
	//rspMsg.m_msgHistory = m_msgPersisUtil->Get_FriendChatHistory(reqMsg);
	pServerSess->SendMsg(&rspMsg);
}


/**
 * @brief 处理GUI客户端发送的获取群组聊天记录的请求消息
 * 
 * @param pServerSess GUI的socket会话
 * @param reqMsg 获取群组聊天记录的请求消息
 */
void CMediumServer::HandleSendForward(const std::shared_ptr<CServerSess>& pServerSess, const GetGroupChatHistoryReq& reqMsg)
{
	GetGroupChatHistoryRsp rspMsg;
	rspMsg.m_strMsgId = reqMsg.m_strMsgId;
	rspMsg.m_strUserId = reqMsg.m_strUserId;
	rspMsg.m_strGroupId = reqMsg.m_strGroupId;
	rspMsg.m_strChatMsgId = reqMsg.m_strMsgId;
	{
		auto pMsgUtil = GetMsgPersisUtil(reqMsg.m_strUserId);
		if (pMsgUtil)
		{
			rspMsg.m_msgHistory = pMsgUtil->Get_GroupChatHistory(reqMsg);

		}
		else
		{
			LOG_ERR(ms_loger, "UserId:{} No Msg Util[ {} {} ]", reqMsg.m_strUserId, __FILENAME__, __LINE__);
		}
	}
	pServerSess->SendMsg(&rspMsg);
}


/**
 * @brief 处理发送文件数据开始消息
 * 
 * @param pServerSess GUI客户端会话
 * @param reqMsg 发送文件数据开始消息
 */
void CMediumServer::HandleSendForward(const std::shared_ptr<CServerSess>& pServerSess,FileSendDataBeginReq& reqMsg)
{
	reqMsg.m_strFileHash = m_fileUtil.CalcHash(reqMsg.m_strFileName);
	{
		
	}
	std::string strFileName;// = m_msgPersisUtil->Get_FileByHash(reqMsg.m_strFileHash);
	auto pMsgUtil = GetMsgPersisUtil(reqMsg.m_strUserId);
	if (pMsgUtil)
	{
		pMsgUtil->Get_FileByHash(reqMsg.m_strFileHash);
	}
	else
	{
		LOG_ERR(ms_loger, "UserId {} No Msg Util [{} {}] ", reqMsg.m_strUserId, __FILENAME__, __LINE__);
	}
	if (!strFileName.empty())
	{
		reqMsg.m_strFileName = strFileName;
	}
	else
	{
		std::string strOldFileName = reqMsg.m_strFileName;
		reqMsg.m_strFileName = m_fileUtil.GetFileNameFromPath(reqMsg.m_strFileName);
		std::string strNewFileName = m_fileUtil.GetCurDir()  + reqMsg.m_strUserId + "\\" + reqMsg.m_strFileName;
		if (m_fileUtil.UtilCopy(strOldFileName, strNewFileName))
		{
			LOG_INFO(ms_loger, "CopyFile Succeed {} {}", strOldFileName, strNewFileName);
		}
		else
		{
			LOG_ERR(ms_loger, "CopyFile Failed {} {}", strOldFileName, strNewFileName);
		}
	}
	//对于原始消息，原封不动的转发
	auto item = m_ForwardSessMap.find(pServerSess);
	if (item != m_ForwardSessMap.end())
	{
		auto pMsg = std::make_shared<TransBaseMsg_t>(reqMsg.GetMsgType(), reqMsg.ToString());
		item->second->SendMsg(pMsg);
	}
}

bool CMediumServer::HandleSendForward(FriendChatSendTxtReqMsg& reqMsg)
{
	ChatMsgElemVec msgVec = MsgElemVec(reqMsg.m_strContext);
	ChatMsgElemVec newVec;
	std::string strFileHash;
	bool bHaveImage = false;
	for (const auto item : msgVec)
	{
		if (item.m_eType == CHAT_MSG_TYPE::E_CHAT_IMAGE_TYPE)
		{
			bHaveImage = true;
			FileSendDataBeginReq beginReqMsg;
			{
				beginReqMsg.m_nFileId = rand();
				beginReqMsg.m_strMsgId = m_httpServer->GenerateMsgId();
				beginReqMsg.m_strFileName = m_fileUtil.GetFileNameFromPath(item.m_strImageName);
				beginReqMsg.m_strUserId = reqMsg.m_strSenderId;
				beginReqMsg.m_strFriendId = reqMsg.m_strReceiverId;
				beginReqMsg.m_strFileHash = m_fileUtil.CalcHash(item.m_strImageName);
				strFileHash = beginReqMsg.m_strFileHash;
				std::string strNewFileName = m_fileUtil.GetCurDir() + reqMsg.m_strSenderId + "\\" + beginReqMsg.m_strFileName;
				if (m_fileUtil.UtilCopy(item.m_strImageName, strNewFileName))
				{

				}
				else
				{
					LOG_ERR(ms_loger, "CopyFile Failed {} {}", item.m_strImageName, strNewFileName);
				}
				auto item = m_userClientSessMap.find(reqMsg.m_strSenderId);
				if (item != m_userClientSessMap.end())
				{
					auto pMsg = std::make_shared<TransBaseMsg_t>(beginReqMsg.GetMsgType(), beginReqMsg.ToString());
					item->second->SendMsg(pMsg);
				}
			}
			ChatMsgElem elem;
			elem.m_eType = CHAT_MSG_TYPE::E_CHAT_IMAGE_TYPE;
			elem.m_strImageName = beginReqMsg.m_strFileName;
			newVec.push_back(elem);
		}
		else
		{
			newVec.push_back(item);
		}
	}



	if (bHaveImage)
	{
		reqMsg.m_strContext = MsgElemVec(newVec);
		m_SendWaitMsgMap.insert({ strFileHash,reqMsg });
	}
	else
	{
		auto item = m_userClientSessMap.find(reqMsg.m_strSenderId);
		if (item != m_userClientSessMap.end())
		{
			auto pMsg = std::make_shared<TransBaseMsg_t>(reqMsg.GetMsgType(), reqMsg.ToString());
			item->second->SendMsg(pMsg);
		}
	}

	return true;
}

/**
 * @brief 处理发送好友聊天消息 在ClientCore中对消息进行解析,发送其中的图片消息
 * 
 * 
 * @param pServerSess GUI客户端的Sess
 * @param reqMsg 好友聊天请求消息
 */
void CMediumServer::HandleSendForward(const std::shared_ptr<CServerSess>& pServerSess,FriendChatSendTxtReqMsg& reqMsg)
{
	if(pServerSess)
	{
		HandleSendForward(reqMsg);
	}
	/*{
		ChatMsgElemVec msgVec = MsgElemVec(reqMsg.m_strContext);
		ChatMsgElemVec newVec;
		std::string strFileHash;
		bool bHaveImage = false;
		for (const auto item : msgVec)
		{
			if (item.m_eType == CHAT_MSG_TYPE::E_CHAT_IMAGE_TYPE)
			{
				bHaveImage = true;
				FileSendDataBeginReq beginReqMsg;
				{
					beginReqMsg.m_nFileId = rand();
					beginReqMsg.m_strMsgId = m_httpServer->GenerateMsgId();
					beginReqMsg.m_strFileName = m_fileUtil.GetFileNameFromPath(item.m_strImageName);
					beginReqMsg.m_strUserId = reqMsg.m_strSenderId;
					beginReqMsg.m_strFriendId = reqMsg.m_strReceiverId;
					beginReqMsg.m_strFileHash = m_fileUtil.CalcHash(item.m_strImageName);
					strFileHash = beginReqMsg.m_strFileHash;
					std::string strNewFileName = m_fileUtil.GetCurDir() + reqMsg.m_strSenderId + "\\" + beginReqMsg.m_strFileName;
					if (m_fileUtil.UtilCopy(item.m_strImageName, strNewFileName))
					{

					}
					else
					{
						LOG_ERR(ms_loger, "CopyFile Failed {} {}", item.m_strImageName, strNewFileName);
					}
					auto item = m_ForwardSessMap.find(pServerSess);
					if (item != m_ForwardSessMap.end())
					{
						auto pMsg = std::make_shared<TransBaseMsg_t>(beginReqMsg.GetMsgType(), beginReqMsg.ToString());
						item->second->SendMsg(pMsg);
					}
				}
				ChatMsgElem elem;
				elem.m_eType = CHAT_MSG_TYPE::E_CHAT_IMAGE_TYPE;
				elem.m_strImageName = beginReqMsg.m_strFileName;
				newVec.push_back(elem);
			}
			else
			{
				newVec.push_back(item);
			}
		}



		if (bHaveImage)
		{
			reqMsg.m_strContext = MsgElemVec(newVec);
			m_SendWaitMsgMap.insert({ strFileHash,reqMsg });
		}
		else
		{
			auto item = m_ForwardSessMap.find(pServerSess);
			if (item != m_ForwardSessMap.end())
			{
				auto pMsg = std::make_shared<TransBaseMsg_t>(reqMsg.GetMsgType(), reqMsg.ToString());
				item->second->SendMsg(pMsg);
			}
		}
	}*/

}

/**
 * @brief 来自GUI客户端的部分消息,不需要发送到远端的服务器,在此函数进行处理
 * 
 * @param pServerSess GUI客户端的连接
 * @param msg 
 * @return true 成功处理,不需要发送到远端
 * @return false 没有处理,需要发送到远端处理
 */
bool CMediumServer::HandleSendForward(const std::shared_ptr<CServerSess>& pServerSess, const TransBaseMsg_t& msg)
{
	{
		if (msg.GetType() == E_MsgType::GetFriendChatHistroyReq_Type)
		{
			GetFriendChatHistoryReq reqMsg;
			if (reqMsg.FromString(msg.to_string())) {
				HandleSendForward(pServerSess, reqMsg);
			}
			return true;
		}
		if (msg.GetType() == E_MsgType::GetGroupChatHistoryReq_Type)
		{
			GetGroupChatHistoryReq reqMsg;
			if (reqMsg.FromString(msg.to_string())) {
				HandleSendForward(pServerSess, reqMsg);
			}
			return true;
		}
		if (msg.GetType() == E_MsgType::FileSendDataBeginReq_Type)
		{
			FileSendDataBeginReq reqMsg;
			if (reqMsg.FromString(msg.to_string())) {
				HandleSendForward(pServerSess, reqMsg);
			}
			return true;
		}
		if (msg.GetType() == E_MsgType::FriendChatSendTxtMsgReq_Type)
		{
			FriendChatSendTxtReqMsg reqMsg;
			if (reqMsg.FromString(msg.to_string())) {
				HandleSendForward(pServerSess, reqMsg);
			}
			return true;
		}
	}
	return false;
}

/**
 * @brief 根据用户ID获取到GUI客户端的连接
 * 
 * @param strUserId 用户ID
 * @return CServerSess_SHARED_PTR 
 */
CServerSess_SHARED_PTR CMediumServer::GetSendBackSess(const std::string strUserId)
{
	auto pSess = GetClientSess(strUserId);
	if (pSess)
	{
		auto item = m_BackSessMap.find(pSess);
		if (item != m_BackSessMap.end())
		{
			return item->second;
		}
		else
		{
			return nullptr;
		}
	}
	else
	{
		return nullptr;
	}
}


/**
 * @brief 处理收到的好友聊天消息
 * 
 * @param pClientSess 和服务器的连接
 * @param reqMsg 收到的好友聊天消息
 */
void CMediumServer::HandleSendBack(const std::shared_ptr<CClientSess>& pClientSess, const FriendChatRecvTxtReqMsg reqMsg)
{
	{
		bool bWaitImage = false;
		ChatMsgElemVec elemVec = MsgElemVec(reqMsg.m_chatMsg.m_strContext);
		for (const auto item : elemVec)
		{
			if (item.m_eType == CHAT_MSG_TYPE::E_CHAT_IMAGE_TYPE)
			{
				FileDownLoadReqMsg downMsg;
				downMsg.m_strUserId = reqMsg.m_chatMsg.m_strReceiverId;
				downMsg.m_strFriendId = reqMsg.m_chatMsg.m_strSenderId;
				downMsg.m_strMsgId = m_httpServer->GenerateMsgId();
				downMsg.m_strFileName = item.m_strImageName;
				downMsg.m_strRelateMsgId = reqMsg.m_chatMsg.m_strChatMsgId;
				LOG_INFO(ms_loger, "Download File {} Begin [{} {}]", item.m_strImageName, __FILENAME__, __LINE__);
				pClientSess->SendMsg(&downMsg);
				bWaitImage = true;
			}
		}

		if (bWaitImage)
		{
			m_waitImageMsgMap.insert({ reqMsg.m_chatMsg.m_strChatMsgId,reqMsg });
			FriendChatRecvTxtRspMsg rspMsg;
			rspMsg.m_strUserId = reqMsg.m_chatMsg.m_strReceiverId;
			rspMsg.m_strFriendId = reqMsg.m_chatMsg.m_strSenderId;
			rspMsg.m_strMsgId = reqMsg.m_strMsgId;
			rspMsg.m_strChatMsgId = reqMsg.m_chatMsg.m_strChatMsgId;
			pClientSess->SendMsg(&rspMsg);
		}
		else
		{
			auto pSess = GetSendBackSess(reqMsg.m_chatMsg.m_strReceiverId);
			if (pSess)
			{
				pSess->SendMsg(&reqMsg);
				auto pMsgUtil = GetMsgPersisUtil(reqMsg.m_chatMsg.m_strReceiverId);
				if(pMsgUtil)
				{
					pMsgUtil->Save_FriendChatSendTxtRspMsg(reqMsg.m_chatMsg);
				}
				else
				{
					LOG_ERR(ms_loger, "UserId {} No Msg Util [{} {}] ", reqMsg.m_chatMsg.m_strReceiverId,__FILENAME__, __LINE__);
				}
			}
			else
			{
				LOG_ERR(ms_loger, "User Id {} No Sess [{} {}]", reqMsg.m_chatMsg.m_strReceiverId, __FILENAME__, __LINE__);
			}
			FriendChatRecvTxtRspMsg rspMsg;
			rspMsg.m_strUserId = reqMsg.m_chatMsg.m_strReceiverId;
			rspMsg.m_strFriendId = reqMsg.m_chatMsg.m_strSenderId;
			rspMsg.m_strMsgId = reqMsg.m_strMsgId;
			rspMsg.m_strChatMsgId = reqMsg.m_chatMsg.m_strChatMsgId;
			pClientSess->SendMsg(&rspMsg);
		}

	}

	//m_msgPersisUtil->Save_FriendChatSendTxtRspMsg(reqMsg.m_chatMsg);
}


/**
 * @brief 处理发送的好友聊天消息的回复
 * 
 * @param pClientSess 和服务器的连接
 * @param rspMsg 好友聊天消息的回复
 */
void CMediumServer::HandleSendBack(const std::shared_ptr<CClientSess>& pClientSess, const FriendChatSendTxtRspMsg rspMsg)
{
	{
		ChatMsgElemVec oldVec = MsgElemVec(rspMsg.m_chatMsg.m_strContext);
		ChatMsgElemVec newVec;
		for (const auto item : oldVec)
		{
			if (item.m_eType == CHAT_MSG_TYPE::E_CHAT_IMAGE_TYPE)
			{
				std::string strImageName = m_fileUtil.GetCurDir() + rspMsg.m_chatMsg.m_strSenderId + "\\" + item.m_strImageName;
				ChatMsgElem elem;
				elem.m_eType = item.m_eType;
				elem.m_strImageName = strImageName;
				newVec.push_back(elem);
			}
			else
			{
				newVec.push_back(item);
			}
		}
		FriendChatSendTxtRspMsg newRspMsg = rspMsg;
		newRspMsg.m_chatMsg.m_strContext = MsgElemVec(newVec);
		auto pMsg = std::make_shared<TransBaseMsg_t>(newRspMsg.GetMsgType(), newRspMsg.ToString());
		auto item = m_BackSessMap.find(pClientSess);
		if (item != m_BackSessMap.end())
		{
			item->second->SendMsg(pMsg);
		}

		try {
			auto pMsgUtil = GetMsgPersisUtil(newRspMsg.m_chatMsg.m_strReceiverId);
			if (pMsgUtil)
			{
				pMsgUtil->Save_FriendChatSendTxtRspMsg(newRspMsg.m_chatMsg);
			}
			else
			{
				LOG_ERR(ms_loger, "UserId {} No Msg Util [{} {}] ", newRspMsg.m_chatMsg.m_strReceiverId, __FILENAME__, __LINE__);
			}
		}
		catch (std::exception ex)
		{
			LOG_ERR(ms_loger, "Ex:{} [{} {}]", ex.what(), __FILENAME__, __LINE__);
		}
	}
}

/**
 * @brief 处理服务器发送过来的文件数据开始请求
 * 
 * @param pClientSess 和服务器的连接
 * @param reqMsg 发送文件数据开始消息
 */
void CMediumServer::HandleSendBack(const std::shared_ptr<CClientSess>& pClientSess, const FileSendDataBeginReq reqMsg)
{
	FileSendDataBeginRsp rspMsg;
	std::string strFileName;
	auto pMsgUtil = GetMsgPersisUtil(reqMsg.m_strUserId);
	if (pMsgUtil)
	{
		strFileName = pMsgUtil->Get_FileByHash(reqMsg.m_strFileHash);
	}
	else
	{
		LOG_ERR(ms_loger, "UserId {} No Msg Util [{} {}] ", reqMsg.m_strUserId, __FILENAME__, __LINE__);
	}



	//auto item = m_fileHashMsgIdMap.find(reqMsg.m_strFileHash);
	//auto hashItem = std::find(m_fileHashTransVec.begin(), m_fileHashTransVec.end(), reqMsg.m_strFileHash);
	if(strFileName.empty())
	{
		m_fileHashTransVec.push_back(reqMsg.m_strFileHash);
		rspMsg.m_strMsgId = reqMsg.m_strMsgId;
		rspMsg.m_errCode = ERROR_CODE_TYPE::E_CODE_SUCCEED;
		rspMsg.m_strFileName = reqMsg.m_strFileName;
		rspMsg.m_strUserId = reqMsg.m_strUserId;
		rspMsg.m_strFriendId = reqMsg.m_strFriendId;
		rspMsg.m_nFileId = reqMsg.m_nFileId;
		pClientSess->SendMsg(&rspMsg);
		std::string strFileName = m_fileUtil.GetCurDir() + pClientSess->UserId() + "\\";
		strFileName += m_fileUtil.GetFileNameFromPath(reqMsg.m_strFileName);
		if (m_fileUtil.OpenWriteFile(reqMsg.m_nFileId + 1, strFileName))
		{
			LOG_INFO(ms_loger, "{} Open File Succeed:{} [{} {}]", reqMsg.m_strUserId, strFileName, __FILENAME__, __LINE__);
		}
		else
		{
			LOG_ERR(ms_loger, "{} Open File Failed:{} [{} {}]", reqMsg.m_strUserId, strFileName, __FILENAME__, __LINE__);
		}
	}
	else
	{

		rspMsg.m_strMsgId = reqMsg.m_strMsgId;
		rspMsg.m_errCode = ERROR_CODE_TYPE::E_CODE_FILE_TRANSING;
		rspMsg.m_strFileName = reqMsg.m_strFileName;
		rspMsg.m_strUserId = reqMsg.m_strUserId;
		rspMsg.m_strFriendId = reqMsg.m_strFriendId;
		rspMsg.m_nFileId = reqMsg.m_nFileId;
		pClientSess->SendMsg(&rspMsg);

		auto hashItem = m_fileHashMsgIdMap.find(reqMsg.m_strFileHash);
		if (hashItem != m_fileHashMsgIdMap.end())
		{
			auto waitItem = m_waitImageMsgMap.find(hashItem->second);
			if (waitItem != m_waitImageMsgMap.end())
			{
				ChatMsgElemVec oldMsgVec = MsgElemVec(waitItem->second.m_chatMsg.m_strContext);
				ChatMsgElemVec newVec;
				for (const auto item : oldMsgVec)
				{
					if (item.m_eType == CHAT_MSG_TYPE::E_CHAT_IMAGE_TYPE) {
						ChatMsgElem elem;
						elem.m_eType = item.m_eType;
						elem.m_strImageName = strFileName;
						newVec.push_back(elem);
					}
					else {
						newVec.push_back(item);
					}
				}
				waitItem->second.m_chatMsg.m_strContext = MsgElemVec(newVec);
				auto pMsg = std::make_shared<TransBaseMsg_t>(waitItem->second.GetMsgType(), waitItem->second.ToString());
				auto item = m_BackSessMap.find(pClientSess);
				if (item != m_BackSessMap.end())
				{
					item->second->SendMsg(pMsg);
				}
				{
					FriendChatRecvTxtRspMsg rspMsg;
					rspMsg.m_strMsgId = waitItem->second.m_strMsgId;
					rspMsg.m_strFriendId = waitItem->second.m_chatMsg.m_strSenderId;
					rspMsg.m_strUserId = waitItem->second.m_chatMsg.m_strReceiverId;
					rspMsg.m_strChatMsgId = waitItem->second.m_chatMsg.m_strChatMsgId;
					pClientSess->SendMsg(&rspMsg);
				}
				m_waitImageMsgMap.erase(hashItem->second);
			}
		}
	}
}

/**
 * @brief 处理服务器发送过来的文件下载回复消息
 * 
 * @param pClientSess 客户端连接
 * @param rspMsg 文件下载回复消息
 */
void CMediumServer::HandleSendBack(const std::shared_ptr<CClientSess>& pClientSess, const FileDownLoadRspMsg rspMsg)
{
	if(rspMsg.m_errCode == ERROR_CODE_TYPE::E_CODE_SUCCEED)
	{
		auto msgItem = m_waitImageMsgMap.find(rspMsg.m_strRelateMsgId);
		if(msgItem != m_waitImageMsgMap.end())
		{
			std::string strFileName;// = m_msgPersisUtil->Get_FileByHash(rspMsg.m_strFileHash);

			{
				auto pMsgUtil = GetMsgPersisUtil(rspMsg.m_strUserId);
				if (pMsgUtil)
				{
					strFileName = pMsgUtil->Get_FileByHash(rspMsg.m_strFileHash);
				}
				else
				{
					LOG_ERR(ms_loger, "UserId {} No Msg Util [{} {}] ",rspMsg.m_strUserId, __FILENAME__, __LINE__);
				}
			}
			bool bFileExit = m_fileUtil.IsFileExist(strFileName);
			if (!bFileExit)
			{
				//m_msgPersisUtil->Delete_FileByHash(rspMsg.m_strFileHash);
				auto pMsgUtil = GetMsgPersisUtil(rspMsg.m_strUserId);
				if (pMsgUtil)
				{
					pMsgUtil->Delete_FileByHash(rspMsg.m_strFileHash);
				}
				else
				{
					LOG_ERR(ms_loger, "UserId {} No Msg Util [{} {}] ", rspMsg.m_strUserId, __FILENAME__, __LINE__);
				}
			}
			if (strFileName.empty() || !bFileExit)
			{
				m_fileHashMsgIdMap.insert({ rspMsg.m_strFileHash,rspMsg.m_strRelateMsgId });
			}
			else
			{
				ChatMsgElemVec oldMsgVec = MsgElemVec(msgItem->second.m_chatMsg.m_strContext);
				ChatMsgElemVec newVec;
				for (const auto item : oldMsgVec)
				{
					if (item.m_eType == CHAT_MSG_TYPE::E_CHAT_IMAGE_TYPE) {
						ChatMsgElem elem;
						elem.m_eType = item.m_eType;
						elem.m_strImageName = strFileName;
						newVec.push_back(elem);
					}
					else {
						newVec.push_back(item);
					}
				}
				msgItem->second.m_chatMsg.m_strContext = MsgElemVec(newVec);
				//SendBack
				{
					auto pMsg = std::make_shared<TransBaseMsg_t>(msgItem->second.GetMsgType(), msgItem->second.ToString());
					auto item = m_BackSessMap.find(pClientSess);
					if (item != m_BackSessMap.end())
					{
						item->second->SendMsg(pMsg);
					}
				}
				//SendForward
				{
					FriendChatRecvTxtRspMsg rspMsg;
					rspMsg.m_strMsgId = msgItem->second.m_strMsgId;
					rspMsg.m_strFriendId = msgItem->second.m_chatMsg.m_strSenderId;
					rspMsg.m_strUserId = msgItem->second.m_chatMsg.m_strReceiverId;
					rspMsg.m_strChatMsgId = msgItem->second.m_chatMsg.m_strChatMsgId;
					pClientSess->SendMsg(&rspMsg);
				}
				m_waitImageMsgMap.erase(rspMsg.m_strRelateMsgId);
			}
		}
	}
	else
	{
		auto msgItem = m_waitImageMsgMap.find(rspMsg.m_strRelateMsgId);
		if (msgItem != m_waitImageMsgMap.end())
		{
			FriendChatRecvTxtRspMsg sendRspMsg;
			sendRspMsg.m_strMsgId = msgItem->second.m_strMsgId;
			sendRspMsg.m_strFriendId = msgItem->second.m_chatMsg.m_strSenderId;
			sendRspMsg.m_strUserId = msgItem->second.m_chatMsg.m_strReceiverId;
			sendRspMsg.m_strChatMsgId = msgItem->second.m_chatMsg.m_strChatMsgId;
			pClientSess->SendMsg(&sendRspMsg);
			m_waitImageMsgMap.erase(rspMsg.m_strRelateMsgId);
		}
	}
}

/**
 * @brief 处理发送数据开始的回复消息,在文件上传的时候使用
 * 
 * @param pClientSess 和服务器的连接会话
 * @param rspMsg 发送文件数据开始回复消息
 */
void CMediumServer::HandleSendBack(const std::shared_ptr<CClientSess>& pClientSess, const FileSendDataBeginRsp rspMsg)
{
	if (rspMsg.m_errCode == ERROR_CODE_TYPE::E_CODE_SUCCEED)
	{
		//int nFileId = static_cast<int>(time(nullptr));
		int nFileSize = 0;
		std::string strImageName = m_fileUtil.GetCurDir() + rspMsg.m_strUserId + "\\" + rspMsg.m_strFileName;
		m_fileUtil.GetFileSize(nFileSize, strImageName);
		if (m_fileUtil.OpenReadFile(rspMsg.m_nFileId, strImageName)) {
			FileDataSendReqMsg sendReqMsg;
			sendReqMsg.m_strMsgId = m_httpServer->GenerateMsgId();
			sendReqMsg.m_strFriendId = rspMsg.m_strFriendId;
			sendReqMsg.m_strUserId = rspMsg.m_strUserId;
			sendReqMsg.m_nFileId = rspMsg.m_nFileId;

			sendReqMsg.m_nDataTotalCount = nFileSize / 1024 + (nFileSize % 1024 == 0 ? 0 : 1);
			sendReqMsg.m_nDataIndex = 1;
			sendReqMsg.m_nDataLength = 0;
			m_fileUtil.OnReadData(sendReqMsg.m_nFileId, sendReqMsg.m_szData, sendReqMsg.m_nDataLength, 1024);
			pClientSess->SendMsg(&sendReqMsg);
			//auto pUdpSess = GetUdpSess(sendReqMsg.m_strUserId);
			//if (pUdpSess)
			//{
			//	pUdpSess->sendToServer(&sendReqMsg);
			//}
			//else
			//{
			//	//LOG_ERR(ms_loger, "UDP Sess Failed:{}", sendReqMsg.m_strFromId);
			//}
		}
	}
	else {
		std::string strImageName = m_fileUtil.GetCurDir() + rspMsg.m_strUserId + "\\" + rspMsg.m_strFileName;
		std::string strFileHash = m_fileUtil.CalcHash(strImageName);
		auto item = m_SendWaitMsgMap.find(strFileHash);
		if (item != m_SendWaitMsgMap.end()) {
			pClientSess->SendMsg(&(item->second));
			m_SendWaitMsgMap.erase(strFileHash);
		}
	}
}

/**
 * @brief 处理用户登录回复消息
 * 
 * @param pClientSess 和服务器的连接会话
 * @param rspMsg 登录回复消息
 */
void CMediumServer::HandleSendBack(const std::shared_ptr<CClientSess>& pClientSess, const UserLoginRspMsg rspMsg) {
	if (rspMsg.m_eErrCode == ERROR_CODE_TYPE::E_CODE_SUCCEED)
	{
		pClientSess->SetUserId(rspMsg.m_strUserId);
		pClientSess->SetUserName(rspMsg.m_strUserName);
		m_userStateMap.erase(rspMsg.m_strUserId);
		m_userStateMap.insert({ rspMsg.m_strUserId,CLIENT_SESS_STATE::SESS_LOGIN_FINISHED });
		{
			m_userClientSessMap.insert({ rspMsg.m_strUserId,pClientSess });
			auto item = m_BackSessMap.find(pClientSess);
			if (item != m_BackSessMap.end())
			{
				m_userId_ServerSessMap.erase(rspMsg.m_strUserId);
				m_userId_ServerSessMap.insert({rspMsg.m_strUserId,item->second});
			}
		}
		m_userName_UserIdMap.erase(rspMsg.m_strUserName);
		m_userName_UserIdMap.insert({ rspMsg.m_strUserName, rspMsg.m_strUserId });

		m_userId_UserNameMap.erase(rspMsg.m_strUserId);
		m_userId_UserNameMap.insert({ rspMsg.m_strUserId,rspMsg.m_strUserName });
		{
			auto pUdpSess = GetUdpSess(rspMsg.m_strUserId);
			if (pUdpSess == nullptr)
			{
				auto pNewUdp = CreateUdpSess();
				m_userUdpSessMap.insert({ rspMsg.m_strUserId,pNewUdp });
			}
		}
		if (!m_fileUtil.IsFolder(rspMsg.m_strUserName))
		{
			m_fileUtil.CreateFolder(rspMsg.m_strUserName);
		}
		if (!m_fileUtil.IsFolder(rspMsg.m_strUserId))
		{
			m_fileUtil.CreateFolder(rspMsg.m_strUserId);
		}

		{
			auto pOldUtil = GetMsgPersisUtil(rspMsg.m_strUserId);
			if (pOldUtil == nullptr)
			{
				auto pMsgUtil = std::make_shared<CMsgPersistentUtil>(rspMsg.m_strUserId);
				if (pMsgUtil->InitDataBase())
				{
					m_UserId_MsgPersistentUtilMap.insert({ rspMsg.m_strUserId,pMsgUtil });
				}
				else
				{

				}
			}
		}
		m_userClientSessMap.erase(rspMsg.m_strUserName);
		{
			GetFriendListReqMsg reqMsg;
			reqMsg.m_strMsgId = m_httpServer->GenerateMsgId();
			reqMsg.m_strUserId = rspMsg.m_strUserId;
			pClientSess->SendMsg(&reqMsg);
		}
	}

}

/**
 * @brief 处理用户退出登录回复消息
 * 
 * @param pClientSess 和服务器的连接
 * @param rspMsg 用户退出登录回复消息
 */
void CMediumServer::HandleSendBack(const std::shared_ptr<CClientSess>& pClientSess, const UserLogoutRspMsg rspMsg) {
	if (rspMsg.m_eErrCode == ERROR_CODE_TYPE::E_CODE_SUCCEED) {
		m_userClientSessMap.erase(rspMsg.m_strUserName);
		m_userStateMap.erase(pClientSess->UserId());
		m_userStateMap.insert({ pClientSess->UserId(),CLIENT_SESS_STATE::SESS_UN_LOGIN });
	}
}


/**
 * @brief 响应GUI客户端断开连接
 * 
 * @param pServerSess 客户端连接会话
 */
void CMediumServer::ServerSessClose(const CServerSess_SHARED_PTR pServerSess)
{
	auto itemForwad = m_ForwardSessMap.find(pServerSess);
	if (itemForwad != m_ForwardSessMap.end()) {
		m_BackSessMap.erase(itemForwad->second);
		//Clear UserID
		{
			std::string strUserId = itemForwad->second->UserId();
			m_userId_UserNameMap.erase(strUserId);
			m_userIdUdpAddrMap.erase(strUserId);
			m_userUdpSessMap.erase(strUserId);
			m_userClientSessMap.erase(strUserId);
		}
		//Clear UserName
		{
			std::string strUserName = itemForwad->second->UserName();
			m_userName_UserIdMap.erase(strUserName);
		}

		{
			m_ForwardSessMap.erase(pServerSess);
		}

	}
	else
	{
		LOG_ERR(ms_loger, "Could Not Close Server Sess [{} {}]",__FILENAME__,__LINE__);
	}
}

/**
 * @brief 响应发送消息到服务器失败
 * 
 * @param pClientSess 和服务器的连接
 */
void CMediumServer::HandleSendBack_NetFailed(const std::shared_ptr<CClientSess>& pClientSess)
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
	if (nullptr != pSess)
	{
		pSess->SetUserId(pClientSess->UserId());
		pSess->SetUserName(pClientSess->UserName());
	}
}

/**
 * @brief 处理查询UDP地址的回复消息
 * 
 * @param pClientSess 和服务器的连接
 * @param rspMsg UDP地址回复消息
 */
void CMediumServer::HandleSendBack(const std::shared_ptr<CClientSess>& pClientSess, const QueryUserUdpAddrRspMsg rspMsg)
{
	if(pClientSess)
	{

	}
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
		auto item = m_RecvWaitMsgMap.find(pMsg->m_strUserId);
		if (item != m_RecvWaitMsgMap.end())
		{
			m_RecvWaitMsgMap[rspMsg.m_strUserId].push_back(pMsg);
		}
		else
		{
			std::vector<std::shared_ptr<BaseMsg>> mapVec;
			mapVec.push_back(pMsg);
			m_RecvWaitMsgMap.insert({ rspMsg.m_strUserId,mapVec });
		}
	}
}

/**
 * @brief 部分消息不需要返回给GUI客户端，在此函数进行处理
 * 
 * @param pClientSess TCP链接会话
 * @param msg 收到的消息
 * @return true 处理成功
 * @return false 处理失败
 */
bool CMediumServer::HandleSendBack(const std::shared_ptr<CClientSess>& pClientSess, const TransBaseMsg_t& msg)
{
	switch (msg.GetType())
	{
	case E_MsgType::FriendChatReceiveTxtMsgReq_Type:
	{
		FriendChatRecvTxtReqMsg reqMsg;
		if (reqMsg.FromString(msg.to_string())) {
			HandleSendBack(pClientSess, reqMsg);
		}
		return true;
	}break;
	case E_MsgType::FriendChatSendTxtMsgRsp_Type:
	{
		FriendChatSendTxtRspMsg rspMsg;
		if (rspMsg.FromString(msg.to_string())) {
			HandleSendBack(pClientSess, rspMsg);
			return true;
		}
	}break;
	case E_MsgType::SendGroupTextMsgRsp_Type:
	{
		SendGroupTextMsgRspMsg rspMsg;
		if (rspMsg.FromString(msg.to_string())) {
			//m_msgPersisUtil->Save_RecvGroupTextMsgReqMsg(rspMsg);
			auto pMsgUtil = GetMsgPersisUtil(rspMsg.m_strSenderId);
			if (pMsgUtil)
			{
				pMsgUtil->Save_RecvGroupTextMsgReqMsg(rspMsg);
			}
			else
			{
				LOG_ERR(ms_loger, "UserId {} No Msg Util [{} {}] ", rspMsg.m_strSenderId, __FILENAME__, __LINE__);
			}
		}
	}break;
	case E_MsgType::RecvGroupTextMsgReq_Type:
	{
	}break;
	case E_MsgType::FileSendDataBeginReq_Type:
	{
		FileSendDataBeginReq reqMsg;
		if (reqMsg.FromString(msg.to_string())) {
			HandleSendBack(pClientSess, reqMsg);
		}
		return true;
	}break;
	case E_MsgType::FileSendDataBeginRsp_Type:
	{
		FileSendDataBeginRsp rspMsg;
		if (rspMsg.FromString(msg.to_string())) {
			HandleSendBack(pClientSess, rspMsg);
		}
	}break;
	case E_MsgType::FileVerifyReq_Type:
	{
		FileVerifyReqMsg reqMsg;
		if (reqMsg.FromString(msg.to_string())) {
			HandleFileVerifyReq(reqMsg);
		}
	}break;
	case E_MsgType::UserLoginRsp_Type:
	{
		UserLoginRspMsg rspMsg;
		if (rspMsg.FromString(msg.to_string())) {
			HandleSendBack(pClientSess, rspMsg);
		}
	}break;
	case E_MsgType::UserLogoutRsp_Type:
	{
		UserLogoutRspMsg rspMsg;
		if (rspMsg.FromString(msg.to_string())) {
			HandleSendBack(pClientSess, rspMsg);
		}
	}break;
	case E_MsgType::NetFailedReport_Type:
	{
		HandleSendBack_NetFailed(pClientSess);
	}break;
	case E_MsgType::FileDownLoadRsp_Type:
	{
		FileDownLoadRspMsg rspMsg;
		if (rspMsg.FromString(msg.to_string())) {
			HandleSendBack(pClientSess,rspMsg);
		}
	}break;
	case E_MsgType::NetRecoverReport_Type: {
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
	}break;
	case E_MsgType::KeepAliveRsp_Type:
	{
		KeepAliveRspMsg rspMsg;
		if (rspMsg.FromString(msg.to_string()))
		{
			KeepAliveReqMsg reqMsg;
			reqMsg.m_strClientId = pClientSess->UserId();
			{
				auto pUdpSess = GetUdpSess(pClientSess->UserId());
				if (nullptr != pUdpSess)
				{
					pUdpSess->sendToServer(&reqMsg);
				}
			}
		}
	}break;
	case E_MsgType::KeepAliveReq_Type:
	{
		KeepAliveReqMsg reqMsg;
		if (reqMsg.FromString(msg.to_string()))
		{
			KeepAliveReqMsg reqMsg2;
			reqMsg2.m_strClientId = pClientSess->UserId();
			{
				auto pUdpSess = GetUdpSess(pClientSess->UserId());
				if (nullptr != pUdpSess)
				{
					pUdpSess->sendToServer(&reqMsg2);
				}
			}
		}
	}break;
	case E_MsgType::QueryUserUdpAddrRsp_Type:
	{
		QueryUserUdpAddrRspMsg rspMsg;
		if (rspMsg.FromString(msg.to_string())) {
			HandleSendBack(pClientSess, rspMsg);
		}
	}break;
	case E_MsgType::FriendUnReadMsgNotifyReq_Type:
	{
		FriendUnReadNotifyReqMsg reqMsg;
		if (reqMsg.FromString(msg.to_string())) {
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
		return true;
	}break;
	case E_MsgType::FileSendDataRsp_Type:
	{
		FileDataSendRspMsg rspMsg;
		if (rspMsg.FromString(msg.to_string())) {
			HandleSendBack(pClientSess,rspMsg);
		}
	}break;
	case E_MsgType::FileRecvDataReq_Type:
	{
		FileDataRecvReqMsg reqMsg;
		if (reqMsg.FromString(msg.to_string())) {
			Handle_TcpMsg(pClientSess, reqMsg);
		}
		return true;
	}break;
	case E_MsgType::FileVerifyRsp_Type:
	{
		FileVerifyRspMsg rspMsg;
		if (rspMsg.FromString(msg.to_string())) {
			HandleSendBack(pClientSess, rspMsg);
		}
	}break;
	case E_MsgType::GetFriendListRsp_Type:
	{
		GetFriendListRspMsg rspMsg;
		if (rspMsg.FromString(msg.to_string())) {
			HandleSendBack(pClientSess, rspMsg);
		}
	}break;
	default:
	{
		LOG_ERR(ms_loger, "UnHandle MsgType:{} Content:{} [{} {}]", MsgType(msg.GetType()), msg.to_string(), __FILENAME__, __LINE__);
	}break;
	}
	return false;
}
/**
 * @brief 将消息转发到远端的服务器
 * 
 * @param pServerSess 连接客户端的会话
 * @param msg 消息
 */
void CMediumServer::SendFoward(const std::shared_ptr<CServerSess>& pServerSess,const TransBaseMsg_t& msg)
{
	if (HandleSendForward(pServerSess, msg))
	{

	}
	else
	{
		//对于原始消息，原封不动的转发
		auto item = m_ForwardSessMap.find(pServerSess);
		if (item != m_ForwardSessMap.end())
		{
			auto pMsg = std::make_shared<TransBaseMsg_t>(msg.GetType(), msg.to_string());
			item->second->SendMsg(pMsg);
		}
	}
}


/**
 * @brief 处理获取好友聊天记录的请求 
 * 
 * @param reqMsg 获取好友聊天记录的请求
 * @return GetFriendChatHistoryRsp 获取好友聊天记录的回复
 */
GetFriendChatHistoryRsp CMediumServer::DoFriendChatHistoryReq(const GetFriendChatHistoryReq& reqMsg)
{
	GetFriendChatHistoryRsp result;


	auto pMsgUtil = GetMsgPersisUtil(reqMsg.m_strUserId);
	if (pMsgUtil)
	{
		auto rspMsgVec = pMsgUtil->Get_FriendChatHistory(reqMsg);
		result.m_msgHistory = rspMsgVec;
	}
	else
	{
		LOG_ERR(ms_loger, "UserId {} No Msg Util [{} {}] ", reqMsg.m_strUserId, __FILENAME__, __LINE__);
	}
	//if (m_msgPersisUtil)
	//{
	//	auto rspMsgVec = m_msgPersisUtil->Get_FriendChatHistory(reqMsg);
	//	result.m_msgHistory = rspMsgVec;
	//}
	result.m_strMsgId = reqMsg.m_strMsgId;
	result.m_strUserId = reqMsg.m_strUserId;
	result.m_strFriendId = reqMsg.m_strFriendId;
	result.m_strChatMsgId = reqMsg.m_strChatMsgId;
	return result;
}

/**
 * @brief 处理获取群组聊天记录的请求
 * 
 * @param reqMsg 获取群组聊天记录的请求消息
 * @return GetGroupChatHistoryRsp 群组聊天记录的回复消息
 */
GetGroupChatHistoryRsp CMediumServer::DoFriendChatHistoryReq(const GetGroupChatHistoryReq& reqMsg)
{
	GetGroupChatHistoryRsp result;
	auto pMsgUtil = GetMsgPersisUtil(reqMsg.m_strUserId);
	if (pMsgUtil)
	{
		auto rspMsgVec = pMsgUtil->Get_GroupChatHistory(reqMsg);
		result.m_msgHistory = rspMsgVec;

	}
	else
	{
		LOG_ERR(ms_loger, "UserId {} No Msg Util [{} {}] ", reqMsg.m_strUserId, __FILENAME__, __LINE__);
	}

	result.m_strMsgId = reqMsg.m_strMsgId;
	result.m_strUserId = reqMsg.m_strUserId;
	result.m_strGroupId = reqMsg.m_strGroupId;
	result.m_strChatMsgId = reqMsg.m_strChatMsgId;
	return result;
}

/**
 * @brief 处理聊天记录查找请求
 * 
 * @param reqMsg 聊天记录查找请求消息
 * @return SearchChatHistoryRsp 聊天记录查找回复消息
 */
SearchChatHistoryRsp CMediumServer::DoSearchChatHistoryReq(const SearchChatHistoryReq& reqMsg)
{
	SearchChatHistoryRsp result;
	auto pMsgUtil = GetMsgPersisUtil(reqMsg.m_strUserId);
	if (pMsgUtil)
	{
		auto friendMsgVec = pMsgUtil->Get_SearchFriendChatMsg(reqMsg);
		auto groupMsgVec = pMsgUtil->Get_SearchGroupChatMsg(reqMsg);
		result.m_friendChatMsgVec = friendMsgVec;
		result.m_groupChatMsgVec = groupMsgVec;
	}
	else
	{
		LOG_ERR(ms_loger, "UserId {} No Msg Util [{} {}] ", reqMsg.m_strUserId, __FILENAME__, __LINE__);
	}

	result.m_strMsgId = reqMsg.m_strMsgId;
	result.m_strUserId = reqMsg.m_strUserId;
	return result;
}

/**
 * @brief 创建UDP链接
 * 
 * @return CUdpClient_PTR 
 */
CUdpClient_PTR CMediumServer::CreateUdpSess()
{
	auto pSelf = shared_from_this();
	auto pSess = std::make_shared<CUdpClient>(m_ioService, m_udpCfg.m_strServerIp,m_udpCfg.m_nPort, [this, pSelf](const asio::ip::udp::endpoint endPt, TransBaseMsg_t* pMsg) {
		DispatchUdpMsg(endPt, pMsg);
	});
	pSess->StartConnect();
	return pSess;
}

/**
 * @brief 根据用户ID获取UDP链接
 * 
 * @param strUserId 用户ID
 * @return CUdpClient_PTR UDP链接
 */
CUdpClient_PTR CMediumServer::GetUdpSess(const std::string strUserId) {
	auto item = m_userUdpSessMap.find(strUserId);
	if (item != m_userUdpSessMap.end()) {
		return item->second;
	}
	else
	{
		return nullptr;
	}
}
}