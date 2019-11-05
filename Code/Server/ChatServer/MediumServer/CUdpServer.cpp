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

	int CUdpServer::StartConnect() {
		return do_read();
	}

	int CUdpServer::StopConnect() {
		return 0;
	}
	bool SendMessage(std::shared_ptr<TransBaseMsg_t> pMsg) {
		return false;
	}
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
					LOG_INFO(ms_loger, "{}", bytes_recvd);

					
				}
				if (!ec)
				{
					do_read();
				}
			});
		}
		return 0;
	}
	void CUdpServer::SendMsg(const asio::ip::udp::endpoint senderPt, const BaseMsg* pMsg)
	{
		if (m_socket)
		{
			auto pSelf = shared_from_this();
			TransBaseMsg_t trans(pMsg->GetMsgType(), pMsg->ToString());
			memcpy(m_sendbuf, trans.GetData(), trans.GetSize());
			m_socket->async_send_to(asio::buffer(m_sendbuf, trans.GetSize()),senderPt,
				[this, pSelf](std::error_code ec, std::size_t bytes_recvd) {
				if (bytes_recvd > 0)
				{

				}
				if (!ec)
				{
					//do_read();
				}
			});
		}
	}

	std::string CUdpServer::EndPoint(const asio::ip::udp::endpoint senderPt)
	{
		std::string strResult = senderPt.address().to_string() + ":" + std::to_string(senderPt.port());
		return strResult;
	}
	void CUdpServer::HandleRecvMsg(const asio::ip::udp::endpoint senderPt, const TransBaseMsg_t* msg)
	{
		if (ms_loger && msg )
		{
			LOG_INFO(ms_loger, "UDP {} ===> {} Send Msg:{}",EndPoint(senderPt),EndPoint(m_serverPt), msg->to_string());
		}
		if (msg)
		{
			m_udpCallBack(senderPt, msg);
			switch (msg->GetType())
			{
			case MessageType::KeepAliveReq_Type:
			{
				KeepAliveRspMsg rspMsg;
				SendMsg(senderPt, &rspMsg);
			}break;
			default:
			{

			}break;
			}
		}
	}
}