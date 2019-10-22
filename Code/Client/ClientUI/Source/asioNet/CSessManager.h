#ifndef _SOURCE_SERVER_C_SESS_MANAGER_H_
#define _SOURCE_SERVER_C_SESS_MANAGER_H_

#include <map>

#include "json11.hpp"
#include "Log.h"
#include "CommonMsg.h"

using MSG_CALL_BACK_FUNC_T = std::function<bool(const TransBaseMsg_t* pMsg)>;
using TIME_OUT_CALL_BACK_FUNC_T = std::function<void()>;
//管理链接的第二级控制类
namespace SourceServer
{
  class CClientSess;
  class CSourceServer;


class CSessManager : public std::enable_shared_from_this<CSessManager>
{
 protected:
    //TCP连接的IP和端口配置
    IpPortCfg m_config;

    std::map<int, std::shared_ptr<CClientSess>> m_SessMap;
	
 public:
	  CSessManager()
	  {
		  m_config.m_strServerIp = "127.0.0.1";
		  m_config.m_nPort = 9000;

		  m_bHaveMsgFunc = false;
		  m_bHaveTimeOutFunc = false;
		  m_bStopTimer = false;
	  }

	  bool SetServerIpPort(const std::string strIp, const int port)
	  {
		  m_config.m_strServerIp = strIp;
		  m_config.m_nPort = port;
		  return true;
	  }

	  bool StartConnect();
    //用于记录日志
    static std::shared_ptr<spdlog::logger> ms_loger;
	bool SetMsgCallBack(MSG_CALL_BACK_FUNC_T callback) {
		if (!m_bHaveMsgFunc)
		{
			m_bHaveMsgFunc = true;
			m_msgFunc = callback;
			return true;
		}
		return false;
	}
	bool SetTimeOutCallBack(TIME_OUT_CALL_BACK_FUNC_T callback)
	{
		if (!m_bHaveTimeOutFunc)
		{
			m_bHaveTimeOutFunc = true;
			m_timeOutFunc = callback;
			return true;
		}
		return false;
	}
  public:
	static void Initialize();
	static void UnInstialize(); 
	static std::shared_ptr<CSessManager> GetManager();

    //检查Sess的连接情况
    void CheckSessConn();
    
	
	void StartTimer(int nSeconds);
    //执行定时任务
    void OnTimer();
    
    //启动队列
    void Start(std::error_code &ec);
    
	void Stop(std::error_code &ec);
    //响应Sess收到的消息
    void handleMessage(const TransBaseMsg_t* pMsg);

	bool SendMsg(const TransBaseMsg_t* pMsg);
	bool m_bStopTimer;
private:

	bool m_bHaveMsgFunc;
	MSG_CALL_BACK_FUNC_T m_msgFunc;

	bool m_bHaveTimeOutFunc;
	TIME_OUT_CALL_BACK_FUNC_T m_timeOutFunc;
	
};
} // namespace SourceServer
#endif
