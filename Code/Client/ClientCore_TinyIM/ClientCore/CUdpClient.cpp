#include "CUdpClient.h"
namespace ClientCore {
	std::shared_ptr<spdlog::logger> CUdpClient::ms_loger;
	CUdpClient::CUdpClient(asio::io_context& ioService, const std::string strIp, const int port)
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

	void CUdpClient::do_read()
	{
		if (m_udpSocket)
		{
			auto pSelf = shared_from_this();
			m_udpSocket->async_receive_from(asio::buffer(m_recvbuf, max_msg_length_udp), m_recvFromPt, [this,pSelf](std::error_code ec, std::size_t bytes) {
				if (!ec)
				{
					do_read();
				}
				if (!ec && bytes > 0)
				{
					TransBaseMsg_t trans(m_recvbuf);
					if (bytes >= trans.GetSize())
					{
						handle_msg(m_recvFromPt, &trans);
					}
				}
			});
		}
	}
	
	void CUdpClient::SendKeepAlive()
	{
		KeepAliveReqMsg reqMsg;
		reqMsg.m_strClientId = "UdpTest";
		TransBaseMsg_t trans(reqMsg.GetMsgType(), reqMsg.ToString());
		send_msg(m_udpServerPt, &trans);
	}

	void CUdpClient::handle_msg(const asio::ip::udp::endpoint endPt, TransBaseMsg_t* pMsg)
	{
		if (ms_loger && pMsg)
		{
			LOG_INFO(ms_loger, "{}:{} Send :{}", endPt.address().to_string(),endPt.port(), pMsg->to_string());
		}
	}
	
	void CUdpClient::sendToServer(TransBaseMsg_t* pMsg)
	{
		send_msg(m_udpServerPt, pMsg);
	}

	void CUdpClient::send_msg(const asio::ip::udp::endpoint endPt, TransBaseMsg_t* pMsg)
	{
		if (m_udpSocket)
		{
			memcpy(m_sendbuf, pMsg->GetData(), pMsg->GetSize());
			try {
				m_udpSocket->async_send_to(asio::buffer(m_sendbuf, pMsg->GetSize()), endPt, [this](std::error_code ec, std::size_t bytes) {
				});
			}
			catch (std::exception ec)
			{
				LOG_INFO(ms_loger, "{}", ec.what());
			}
		}
	}
}