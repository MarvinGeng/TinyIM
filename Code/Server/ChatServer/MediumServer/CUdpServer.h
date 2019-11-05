#ifndef _DENNIS_THINK_C_UDP_SERVER_H_
#define _DENNIS_THINK_C_UDP_SERVER_H_
#include "json11.hpp"
#include "CommonMsg.h"
#include "asio_common.h"
#include "Log.h"
#include <atomic>
namespace ChatServer
{
	using asio::ip::tcp;
	class CClientSessManager;
	/**
	 * @brief �������ӵ�Զ�˷�������Sess
	 *
	 */
	class CUdpServer : public std::enable_shared_from_this<CUdpServer> {
	protected:
		asio::io_service& m_ioService;

		std::string m_serverIp;
		int 		m_serverPort;



		//������Ϣ,��Ҫ��λ�ȡ,��˱��浽������
		std::string m_connectInfo;

		enum { max_length = 16384, msg_max_len = 8192 };

		//����buf
		char m_recvbuf[max_length];

		//����buf
		char m_sendbuf[max_length];

		//����λ��
		uint32_t m_recvpos = 0;
	public:
		static	std::shared_ptr<spdlog::logger> ms_loger;
	public:
		//����Ƿ�Ϊͬһ������
		bool is_same_connect(const std::string& strIp, const int nport)
		{
			return ((m_serverIp.compare(strIp) == 0) &&
				(nport == m_serverPort));
		}
		//��ȡ���ӵ�ip��ַ�Ͷ˿ں�,��ʽΪ xx.xx.xx.xx:port
		std::string GetConnectInfo() { return m_connectInfo; }

		bool SendKeepAlive();



		CUdpServer(asio::io_service& ioService, std::string strIp, int port);

		int StartConnect();
		int StopConnect();
		bool SendMessage(std::shared_ptr<TransBaseMsg_t> pMsg);
		void HandleRecvMsg(const asio::ip::udp::endpoint senderPt, const TransBaseMsg_t* msg);
	private:
		asio::ip::udp::endpoint m_senderPt;
		int do_read();
		

		void SendMsg(const asio::ip::udp::endpoint senderPt, const BaseMsg* pMsg);
		void handleKeepAliveReq(const KeepAliveReqMsg& reqMsg);

		void handleKeepAliveRsp(const KeepAliveRspMsg& rspMsg);

		//tcp���ӵ�socket
		std::shared_ptr<asio::ip::udp::socket>   m_socket;
	};
}
#endif