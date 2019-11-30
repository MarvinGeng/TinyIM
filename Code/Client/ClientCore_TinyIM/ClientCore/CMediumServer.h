/**
 * @file CMediumServer.h
 * @author DennisMi (https://www.dennisthink.com/)
 * @brief 总控类
 * @version 0.1
 * @date 2019-08-30
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#ifndef _MEDIUM_SERVER_C_MEDIUM_SERVER_H_
#define _MEDIUM_SERVER_C_MEDIUM_SERVER_H_

#include <functional>

#include <map>

#include "CommonMsg.h"
#include "Log.h"
#include "asio_common.h"
#include "json11.hpp"

#include "CClientSess.h"
#include "CServerSess.h"
#include "CHttpServer.h"
#include "CUdpClient.h"
#include "planform.h"
#include "CMsgPersistentUtil.h"
#include "CFileUtil.h"
namespace ClientCore
{
using tcp = asio::ip::tcp;
class CClientSess;
class CMediumServer : public std::enable_shared_from_this<CMediumServer>
{
  protected:
    asio::io_service &m_ioService;

    tcp::socket m_socket;

    tcp::acceptor m_acceptor;

    //服务器的IP端口设置
    IpPortCfg m_serverCfg;
	//所有的客户端的连接客户端的IP端口设置
	std::vector<IpPortCfg> m_clientCfgVec;

	IpPortCfg m_httpCfg;
	
	IpPortCfg m_udpCfg;

	CFileUtil m_fileUtil;

    
	std::vector<std::shared_ptr<CServerSess>> m_listenList; //监听的套接字的列表

	std::map<std::shared_ptr<CServerSess>, std::shared_ptr<CClientSess>> m_ForwardSessMap;
	std::map<std::shared_ptr<CClientSess>, std::shared_ptr<CServerSess>> m_BackSessMap;

	std::map<std::string, std::shared_ptr<CClientSess>> m_userClientSessMap;
	std::map<std::string, std::string> m_userId_UserNameMap;
	std::map<std::string, FriendChatRecvTxtReqMsg> m_waitImageMsgMap;
    std::shared_ptr<asio::high_resolution_timer> m_timer = nullptr;

	std::shared_ptr<CHttpServer> m_httpServer;
	CClientSess_SHARED_PTR m_freeClientSess;
	CMsgPersistentUtil_SHARED_PTR m_msgPersisUtil;
    void SetTimer(int nSeconds);
    void OnTimer();
	void CheckWaitMsgVec();
    std::error_code m_ec;

    void do_accept();
	void OnHttpRsp(std::shared_ptr<TransBaseMsg_t> pMsg);

	void HandleFileDataSendRsp(const FileDataSendRspMsg& rspMsg);

	void HandleFileDataRecvReq(const FileDataRecvReqMsg& reqMsg);

	void HandleFileVerifyReq(const FileVerifyReqMsg& msg);
	void HandleFriendNotifyFileMsgReq(const FriendNotifyFileMsgReqMsg& reqMsg);
	void Handle_UdpMsg(const asio::ip::udp::endpoint endPt,const FileDataRecvReqMsg& reqMsg);
  public:
    static std::shared_ptr<spdlog::logger> ms_loger;
    inline IpPortCfg &config() { return m_serverCfg; }


    void SendBack(const std::shared_ptr<CClientSess>& pClientSess,const TransBaseMsg_t& msg);
	void SendFoward(const std::shared_ptr<CServerSess>& pServerSess,const TransBaseMsg_t& msg);

	void Handle_RecvFileOnlineRsp(const FriendRecvFileMsgRspMsg& rspMsg);
	void Handle_UdpMsg(const asio::ip::udp::endpoint endPt, TransBaseMsg_t* pMsg);
	void Handle_UdpMsg(const asio::ip::udp::endpoint endPt, const FileDataSendRspMsg& Msg);
	void Handle_UdpMsg(const asio::ip::udp::endpoint endPt, const FileDataSendReqMsg& reqMsg);
    void CheckAllConnect();

	CClientSess_SHARED_PTR GetClientSess(const std::string strUserId);
	CClientSess_SHARED_PTR CreateClientSess();
	CUdpClient_PTR CreateUdpSess();
	CUdpClient_PTR GetUdpSess(const std::string strUserId);
	CMsgPersistentUtil_SHARED_PTR GetMsgPersisUtil() {
		return m_msgPersisUtil;
	}

	std::string GetUserId(const std::string strUserName);
    CMediumServer(asio::io_service &io_service)
        : m_ioService(io_service), m_socket(io_service), m_acceptor(io_service)
    {
        if (!m_timer)
        {
            m_timer = std::make_shared<asio::high_resolution_timer>(m_ioService);
        }
		m_httpServer = std::make_shared<CHttpServer>(m_ioService,this);
		m_msgPersisUtil = std::make_shared<CMsgPersistentUtil>();
		m_msgPersisUtil->InitDataBase();
		m_timeCount = 0;
		m_nNoSessTimeCount = 0;
    }

	void ServerSessClose(const CServerSess_SHARED_PTR pSess);

	void start(const std::function<void(const std::error_code &)> &callback);

    void loadConfig(const json11::Json &cfg, std::error_code &ec);
    //获取server的ip和端口,
    std::string getServerIpPort();
	GetFriendChatHistoryRsp HandleFriendChatHistoryReq(const GetFriendChatHistoryReq& reqMsg);
	GetGroupChatHistoryRsp HandleGroupChatHistoryReq(const GetGroupChatHistoryReq& reqMsg);
	SearchChatHistoryRsp HandleSearchChatHistoryReq(const SearchChatHistoryReq& reqMsg);

private:
	CServerSess_SHARED_PTR GetSendBackSess(const std::string strUserId);
	bool HandleSendForward(const std::shared_ptr<CServerSess>& pServerSess, const TransBaseMsg_t& msg);
	bool HandleSendBack(const std::shared_ptr<CClientSess>& pClientSess, const TransBaseMsg_t& msg);
	void HandleSendBack(const std::shared_ptr<CClientSess>& pClientSess, const FriendChatRecvTxtReqMsg reqMsg);
	void HandleSendBack(const std::shared_ptr<CClientSess>& pClientSess, const FriendChatSendTxtRspMsg reqMsg);
	void HandleSendBack(const std::shared_ptr<CClientSess>& pClientSess, const FileSendDataBeginReq reqMsg);
	void HandleSendBack(const std::shared_ptr<CClientSess>& pClientSess, const FileSendDataBeginRsp reqMsg);
	void HandleSendBack(const std::shared_ptr<CClientSess>& pClientSess, const UserLoginRspMsg reqMsg);
	void HandleSendBack(const std::shared_ptr<CClientSess>& pClientSess, const UserLogoutRspMsg rspMsg);
	void HandleSendBack_NetFailed(const std::shared_ptr<CClientSess>& pClientSess);
	void HandleSendBack(const std::shared_ptr<CClientSess>& pClientSess, const QueryUserUdpAddrRspMsg rspMsg);
	void HandleSendBack(const std::shared_ptr<CClientSess>& pClientSess, const FileDownLoadRspMsg rspMsg);

	void HandleSendForward(const std::shared_ptr<CServerSess>& pServerSess, const GetFriendChatHistoryReq& msg);
	void HandleSendForward(const std::shared_ptr<CServerSess>& pServerSess, const GetGroupChatHistoryReq& msg);
	void HandleSendForward(const std::shared_ptr<CServerSess>& pServerSess,FileSendDataBeginReq& msg);
	void HandleSendForward(const std::shared_ptr<CServerSess>& pServerSess,FriendChatSendTxtReqMsg& msg);


	void HandleFriendChatSendTextMsgRsp(const FriendChatSendTxtRspMsg& rspMsg);
	void HandleFriendChatRecvTextMsgRsp(const FriendChatRecvTxtReqMsg& reqMsg);
	std::map<std::string, std::string> m_userName_UserIdMap;
	std::map<std::string, CLIENT_SESS_STATE>  m_userStateMap;
	std::map<std::string, UserLoginReqMsg> m_userLoginMsgMap;
	std::map<std::string, CUdpClient_PTR> m_userUdpSessMap;
	std::map<std::string, IpPortCfg> m_userIdUdpAddrMap;
	std::map<std::string, std::string> m_fileHashMsgIdMap;
	std::vector<std::string> m_fileHashTransVec;

	long long m_timeCount;
	int m_nNoSessTimeCount;
	std::map<std::string, std::vector<std::shared_ptr<BaseMsg>> > m_WaitMsgMap;
};
} // namespace MediumServer

#endif // HTTP2T1_CAGENTSERVER_H
