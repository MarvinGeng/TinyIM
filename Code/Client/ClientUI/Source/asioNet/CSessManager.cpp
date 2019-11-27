#include "CSessManager.h"
#include "CClientSess.h"
#include "asio_common.h"
#include <algorithm>
using namespace SourceServer;
static asio::io_service g_ioService;
static std::thread * g_thread=nullptr;
static std::shared_ptr<asio::high_resolution_timer> g_timer=nullptr;
static std::shared_ptr<SourceServer::CSessManager> g_Server=nullptr;
HANDLE g_threadHandle;
DWORD  g_threadId;
namespace SourceServer
{
std::shared_ptr<spdlog::logger> CSessManager::ms_loger;

static int g_TimerIndex = 0;
/**
 * @brief 启动SessManager
 * 
 * @param pServer SouceServer服务类
 * @param ioService asio::io_service 实例
 * @param ec 发生的异常
 */
void CSessManager::StartTimer(int nSeconds) {
	
	if(!m_bStopTimer)
	{
		g_TimerIndex++;
		g_timer->expires_from_now(std::chrono::seconds(nSeconds));
		auto pSelf = shared_from_this();
		g_timer->async_wait([this, nSeconds, pSelf](const std::error_code &ec) {
			INFO(this->ms_loger, "On Timer at APNsServer");
			this->OnTimer();
			if (!ec)
			{
				this->StartTimer(nSeconds);
			}
			else
			{
			}
		});
	}
}

DWORD WINAPI SocketThread(LPVOID lpParam)
{
	try {
		g_ioService.run();
	}
	catch (std::exception& ec)
	{
		INFO(CSessManager::ms_loger, "On Timer at APNsServer {} ",ec.what());
	}
	
	return 0;
}

bool CSessManager::StartConnect()
{
	INFO(ms_loger, "Queue Start");

	int nSessIndex = 0;
	CClientSess::ms_loger = ms_loger;
	std::shared_ptr<CClientSess> pSess = std::make_shared<CClientSess>(g_ioService, m_config.m_strServerIp, m_config.m_nPort, this);

	if (pSess)
	{
		INFO(ms_loger, "Add Sess To Map");
		m_SessMap.insert(std::pair<int, std::shared_ptr<CClientSess>>(nSessIndex, pSess));
	}
	nSessIndex++;

	for (auto &item : m_SessMap)
	{
		WARN(ms_loger, "Sess id:{}  Connect:{}", item.first, item.second->GetConnectInfo());
		item.second->StartConnect();
	}
	return true;
}
void CSessManager::Start(std::error_code &ec)
{

	m_bStopTimer = false;
	StartTimer(1);
	
	//StartThread
	{
		g_threadHandle = CreateThread(NULL, 0, SocketThread,NULL, 0, &g_threadId);
	}
	//g_thread = new std::thread([this] {
	////	g_ioService.run();
	//});
	//SetThreadPriority(g_thread->native_handle(), THREAD_PRIORITY_HIGHEST);
	//g_thread->detach();

	ec.clear();
}

/**
 * @brief 检查所有的连接
 * 
 */
void CSessManager::CheckSessConn()
{
	for (auto &item : m_SessMap)
	{
		if (item.second->IsConnect())
		{
		}
		else
		{
			item.second->StartConnect();
		}
	}
}

void CSessManager::Stop(std::error_code &ec)
{
	m_bStopTimer = true;
	m_bHaveTimeOutFunc = false;
}

void CSessManager::Initialize() {


	g_timer = std::make_shared<asio::high_resolution_timer>(g_ioService);

	g_Server = std::make_shared<SourceServer::CSessManager>();
}
void CSessManager::UnInstialize() {
	if (!g_ioService.stopped())
	{
		g_ioService.stop();
	}
	g_Server->m_bStopTimer = true;
	g_Server->m_bHaveTimeOutFunc = false;
	CloseHandle(g_threadHandle);
	
}
std::shared_ptr<CSessManager> CSessManager::GetManager() {
	return g_Server;
}
/**
 * @brief 响应定时器
 * 
 */
void CSessManager::OnTimer()
{
	INFO(this->ms_loger, "Queue On Timer");
	if (m_bHaveTimeOutFunc)
	{
		m_timeOutFunc();
	}
}

/**
 * @brief 响应收到的消息
 * 
 * @param pMsg 接收到的消息
 */
void CSessManager::handleMessage(const TransBaseMsg_t *pMsg)
{
	if (m_bHaveMsgFunc)
	{
		m_msgFunc(pMsg);
	}
}

bool CSessManager::SendMsg(const TransBaseMsg_t *pMsg) {
	
	for (auto &item : m_SessMap)
	{
		if (item.second->IsConnect())
		{
			auto pMsgSend = std::make_shared<TransBaseMsg_t>(pMsg->GetType(), pMsg->to_string());
			item.second->send_msg(pMsgSend);
			return true;
		}
	}
	return false;
}
} // namespace SourceServer