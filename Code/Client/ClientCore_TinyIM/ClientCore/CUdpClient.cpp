/**
 * @file CUdpClient.cpp
 * @author DennisMi (https://www.dennisthink.com/)
 * @brief UDP的收发数据的类实现文件
 * @version 0.1
 * @date 2019-12-27
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include "CUdpClient.h"
namespace ClientCore {
	std::shared_ptr<spdlog::logger> CUdpClient::ms_loger;
	
	CUdpClient::CUdpClient(asio::io_context& ioService, const std::string strIp, const int port, UDP_CALL_BACK&& callBack):m_callBack(callBack)
	{
		m_udpServerIp = strIp;
		m_udpServerPort = port;
		asio::ip::address addr;
		addr.from_string("127.0.0.1");
		m_udpSocket = std::make_shared<asio::ip::udp::socket>(ioService, asio::ip::udp::endpoint(asio::ip::udp::v4(), 0));
		
		asio::ip::udp::resolver resolver(ioService);
		asio::ip::udp::resolver::results_type endpoints =
			resolver.resolve(asio::ip::udp::v4(), "127.0.0.1",std::to_string(20000));

		m_udpServerPt = *endpoints.begin();
	}

	void CUdpClient::StartConnect()
	{
		do_read();
	}

	/**
	 * @brief 从UDP读取数据
	 * 
	 */
	void CUdpClient::do_read()
	{
		if (m_udpSocket)
		{
			auto pSelf = shared_from_this();
			m_udpSocket->async_receive_from(asio::buffer(m_recvbuf, max_msg_length_udp), m_recvFromPt, [this,pSelf](std::error_code ec, std::size_t bytes) {
				if (!ec && bytes > 0)
				{
					TransBaseMsg_t trans(m_recvbuf);
					LOG_INFO(this->ms_loger, "[{}] UDP RECV: {} MSG:{} {} [{} {}]",UserId(), EndPoint(m_recvFromPt),MsgType(trans.GetType()), trans.to_string(),__FILENAME__,__LINE__);
					if (bytes >= trans.GetSize())
					{
						handle_msg(m_recvFromPt, &trans);
					}
					do_read();
				}
			});
		}
	}
	
	/**
	 * @brief 处理收到的UDP消息
	 * 
	 * @param endPt UDP消息的发送者的地址
	 * @param pMsg UDP消息
	 */
	void CUdpClient::handle_msg(const asio::ip::udp::endpoint endPt, TransBaseMsg_t* pMsg)
	{
		m_callBack(endPt, pMsg);
	}
	
	
	/**
	 * @brief 发送消息到UDP服务器
	 * 
	 * @param pMsg 
	 */
	void CUdpClient::sendToServer(const BaseMsg* pMsg)
	{
		send_msg(m_udpServerIp,m_udpServerPort, pMsg);
	}


	/**
	 * @brief 发送消息到对应的IP和端口
	 * 
	 * @param strIp UDP的IP
	 * @param port UDP的端口
	 * @param pMsg 待发送的消息
	 */
	void CUdpClient::send_msg(const std::string strIp, const int port, const BaseMsg* pMsg)
	{
		asio::ip::udp::resolver resolver(m_udpSocket->get_io_context());
		asio::ip::udp::resolver::results_type endpoints =
			resolver.resolve(asio::ip::udp::v4(), strIp, std::to_string(port));

		if (!endpoints.empty())
		{
			
			auto pSend = std::make_shared<TransBaseMsg_t>(pMsg->GetMsgType(), pMsg->ToString());
			send_msg(*endpoints.begin(), pSend);
		}
	}
	
	/**
	 * @brief 获取UDP地址的字符串表示
	 * 
	 * @param endPt UDP地址
	 * @return std::string 地址的字符串表示
	 */
	std::string CUdpClient::EndPoint(const asio::ip::udp::endpoint endPt)
	{
		std::string strResult = endPt.address().to_string() + ":" + std::to_string(endPt.port());
		return strResult;
	}
	
	/**
	 * @brief 发送UDP消息
	 * 
	 * @param endPt 
	 * @param pMsg 
	 */
	void CUdpClient::send_msg(const asio::ip::udp::endpoint endPt, const BaseMsg* pMsg)
	{
		auto pSend= std::make_shared<TransBaseMsg_t>(pMsg->GetMsgType(), pMsg->ToString());
		send_msg(endPt, pSend);
	}

	/**
	 * @brief 发送UDP消息
	 * 
	 * @param endPt 
	 * @param pMsg 
	 */
	void CUdpClient::send_msg(const asio::ip::udp::endpoint endPt, TransBaseMsg_S_PTR pMsg)
	{
		if (pMsg->GetType() != E_MsgType::FileSendDataReq_Type && pMsg->GetType() != E_MsgType::FileRecvDataReq_Type)
		{
			LOG_INFO(ms_loger, "[{}] UDP SEND: {} Msg:{} {} [{} {}]", UserId(), EndPoint(endPt), MsgType(pMsg->GetType()), pMsg->to_string(), __FILENAME__, __LINE__);
		}
		//m_sendQueue.push({ endPt,pMsg });
		/*if (!m_bDoSend) {
			m_bDoSend = true;
			do_SendMsg();
		}*/
		
		if (m_udpSocket)
		{
			memcpy(m_sendbuf, pMsg->GetData(), pMsg->GetSize());
			try {
				m_udpSocket->async_send_to(asio::buffer(m_sendbuf, pMsg->GetSize()), endPt, [this](std::error_code /*ec*/, std::size_t /*bytes*/) {
				});
			}
			catch (std::exception ec)
			{
				LOG_INFO(ms_loger, "{}", ec.what());
			}
		}
	}
	void CUdpClient::DoSend() {
		//if (!m_bDoSend) {
		//	m_bDoSend = true;
		//	do_SendMsg();
		//}
	}
	void CUdpClient::do_SendMsg() {
		if (!m_sendQueue.empty()) {
			if (m_udpSocket)
			{
				auto item = m_sendQueue.front();
				memcpy(m_sendbuf, item.msgToSend->GetData(), item.msgToSend->GetSize());
				try {
					auto pSelf = shared_from_this();
					m_udpSocket->async_send_to(asio::buffer(m_sendbuf, item.msgToSend->GetSize()), item.endPt, [this, pSelf](std::error_code ec, std::size_t bytes) {
						if (!ec && bytes > 0) {
							m_sendQueue.pop();
							do_SendMsg();
						}
					});
				}
				catch (std::exception ec)
				{
					LOG_INFO(ms_loger, "{}", ec.what());
				}
			}
		}
		else
		{
			m_bDoSend = false;
		}
	}

}