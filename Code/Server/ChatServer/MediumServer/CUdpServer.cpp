#include "CUdpServer.h"

namespace ChatServer
{
	std::shared_ptr<spdlog::logger> CUdpServer::ms_loger;
	bool CUdpServer::SendKeepAlive()
	{
		return false;
	}

	CUdpServer::CUdpServer(asio::io_service& ioService, std::string strIp, int port, UDP_CALL_BACK&& callBack):m_ioService(ioService),m_udpCallBack(callBack)
	{	asio::ip::address addr;
		addr.from_string(strIp);
		asio::ip::udp::endpoint endPt(addr, port);
		std::error_code ec;
		m_serverPt = endPt;
		m_socket = std::make_shared<asio::ip::udp::socket>(ioService, endPt);
	}

	/**
	 * @brief 启动连接
	 * 
	 * @return int 
	 */
	int CUdpServer::StartConnect() {
		return do_read();
	}

	/**
	 * @brief 停止连接
	 * 
	 * @return int 
	 */
	int CUdpServer::StopConnect() {
		return 0;
	}

	
	/**
	 * @brief 从UDP端口读取接收到的数据
	 * 
	 * @return int 
	 */
	int CUdpServer::do_read() {
		if (m_socket)
		{
			auto pSelf = shared_from_this();
			m_socket->async_receive_from(asio::buffer(m_recvbuf, max_length), m_senderPt,
				[this, pSelf](std::error_code ec, std::size_t bytes_recvd){
				if (bytes_recvd > 0)
				{
					TransBaseMsg_t trans(m_recvbuf);
					if (bytes_recvd >= trans.GetSize())
					{
						HandleRecvMsg(m_senderPt, &trans);
					}				
				}
				if (!ec)
				{
					do_read();
				}
			});
		}
		return 0;
	}

	/**
	 * @brief 发送UDP消息到对应的IP和端口
	 * 
	 * @param strIp 对端的IP地址
	 * @param port 对端的端口
	 * @param pMsg 待发送的消息
	 */
	void CUdpServer::sendMsg(const std::string strIp, const int port, const BaseMsg* pMsg)
	{
		asio::ip::udp::resolver resolver(m_socket->get_io_context());
		asio::ip::udp::resolver::results_type endpoints =
			resolver.resolve(asio::ip::udp::v4(), strIp, std::to_string(port));

		if (!endpoints.empty())
		{
			sendMsg(*endpoints.begin(), pMsg);
		}
	}

	/**
	 * @brief 发送UDP消息到senderPt
	 * 
	 * @param senderPt 接收方的UDP地址
	 * @param pMsg 待发送的消息
	 */
	void CUdpServer::sendMsg(const asio::ip::udp::endpoint senderPt, const BaseMsg* pMsg)
	{
		if (m_socket)
		{
			auto pSelf = shared_from_this();
			TransBaseMsg_t trans(pMsg->GetMsgType(), pMsg->ToString());
			memcpy(m_sendbuf, trans.GetData(), trans.GetSize());
			m_socket->async_send_to(asio::buffer(m_sendbuf, trans.GetSize()),senderPt,
				[this, pSelf, senderPt, trans](std::error_code ec, std::size_t bytes_recvd) {
				if (bytes_recvd > 0)
				{
					LOG_DBG(ms_loger, "UDP Send To:{} Msg:{} [{} {}]", EndPoint(senderPt), trans.to_string(), __FILENAME__, __LINE__);
				}
				if (!ec)
				{
					LOG_ERR(ms_loger, "UDP Send To:{} Msg:{} ERR:{} [{} {}]", EndPoint(senderPt), trans.to_string(),ec.value(), __FILENAME__, __LINE__);
				}
			});
		}
	}

	/**
	 * @brief UDP地址转字符串
	 * 
	 * @param senderPt UDP地址
	 * @return std::string 地址的字符串表示
	 */
	std::string CUdpServer::EndPoint(const asio::ip::udp::endpoint senderPt)
	{
		std::string strResult = senderPt.address().to_string() + ":" + std::to_string(senderPt.port());
		return strResult;
	}

	/**
	 * @brief 处理收到的UDP消息
	 * 
	 * @param senderPt UDP发送者的地址
	 * @param msg UDP消息
	 */
	void CUdpServer::HandleRecvMsg(const asio::ip::udp::endpoint senderPt, const TransBaseMsg_t* msg)
	{
		if (msg)
		{
			LOG_INFO(ms_loger, "Udp Recv From: {}  Msg:{} [{} {}]",EndPoint(senderPt), msg->to_string(),__FILENAME__,__LINE__);
		}
		if (msg)
		{
			m_udpCallBack(senderPt, msg);
		}
	}
}