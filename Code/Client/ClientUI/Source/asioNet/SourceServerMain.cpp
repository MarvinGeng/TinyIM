#include <iostream>
#include <memory>

#include "Log.h"
#include "CommonFunction.h"
#include "CommonConfig.h"
#include "CSessManager.h"
#include "DaemonSvcApp.h"
#include "planform.h"
#include "CommonMsg.h"

using namespace std;
void GenerateCfgFile(std::string fileName)
{
    std::string strConfig=R"({
   "LogDir":"/home/test/Log/SourceServer/",
   "server":{
        "ip":"127.0.0.1",
        "port":9000
      }
    })";
    write_txtfile(fileName.c_str(),strConfig);
}


#ifdef WIN32
class UserRegisterCallBackHandler {

};
class ProtoInterface:public std::enable_shared_from_this<ProtoInterface> {
public:
	ProtoInterface() {

	}
public:
	void Init() {
		m_startTime = 0;
		auto pSess = SourceServer::CSessManager::GetManager();
		auto pIn = shared_from_this();
		pSess->SetTimeOutCallBack([this,pIn]()->void {
			OnTimeOut();
		});

		pSess->SetMsgCallBack(
			[this,pIn](const TransBaseMsg_t* pMsg)->bool {
			return pIn->OnMsgBack(pMsg);
		});
	}
	bool SendRegisterSync(int nSeconds) {
		m_startTime = 0;
		while (m_startTime < nSeconds) {
			std::cout << time(nullptr) << std::endl;
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		return false;
	}

	bool OnMsgBack(const TransBaseMsg_t* pMsg) {
		std::cout << "MsgInfo" << pMsg->to_string() << std::endl;
		return false;
	}
	void OnTimeOut() {
		m_startTime++;
	}
private:
	int m_startTime;
};
int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		PrintParamOption();
		return 0;
	}
	ParseParamResult result = ParseParam(argc, argv, GenerateCfgFile);
	if (result.m_bReturn)
	{
		return 0;
	}

	std::string strcfg, errinfo;
	load_txtfile(result.m_cfgFile.c_str(), strcfg);
	std::cout << strcfg << std::endl;
	//std::cout << strcfg << std::endl;
	if (!strcfg.length()) {
		printf("no Configure\n");
		return 1;
	}
	auto cfg = json11::Json::parse(strcfg, errinfo, json11::JsonParse::COMMENTS);

	auto logger = CreateLogger(cfg);
	if (!logger)
	{
		std::cout << "Can not Create Logger" << std::endl;
		return 0;
	}


	SourceServer::CSessManager::ms_loger = logger;
	SourceServer::CSessManager::Initialize();
	IpPortCfg cfg2;
	cfg2.m_strServerIp = "127.0.0.1";
	cfg2.m_nPort = 9000;
	auto pSessManager = SourceServer::CSessManager::GetManager();

	std::error_code ec;
	pSessManager->Start(ec);

	int x = 0;
	auto inter = std::make_shared<ProtoInterface>();
	inter->Init();
	std::cout << "-----------------------------" << std::endl;
	inter->SendRegisterSync(2);
	std::cout << "-----------------------------" << std::endl;
	inter->SendRegisterSync(4);
	std::cout << "-----------------------------" << std::endl;
	inter->SendRegisterSync(6);
	std::cout << "-----------------------------" << std::endl;


	while (true)
	{
		std::cin >> x;
		if (x < 0)
		{
			break;
		}
	}
	SourceServer::CSessManager::UnInstialize();
	return 0;
}
#else
int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		PrintParamOption();
		return 0;
	}
    ParseParamResult result = ParseParam(argc,argv,GenerateCfgFile);
    if(result.m_bReturn)
    {
        return 0;
    }
    theApp.set_tag(result.m_strTag.c_str());
    std::string strcfg,errinfo;
    load_txtfile(result.m_cfgFile.c_str(), strcfg);
    if(!strcfg.length()){
		printf("no Configure\n");
        return 1;
    }
    auto cfg=json11::Json::parse(strcfg,errinfo,json11::JsonParse::COMMENTS);

    auto logger = CreateLogger(cfg);
    if(!logger)
    {
        std::cout<<"Can not Create Logger"<<std::endl;
        return 0;
    }
	theApp.m_loger=logger;
	theApp.m_loger->set_level(spdlog::level::debug);
   	SourceServer::CClientSess::ms_loger = theApp.m_loger;
	SourceServer::CSourceServer::ms_loger = theApp.m_loger;
	SourceServer::CSessManager::ms_loger = theApp.m_loger;

    theApp.list([result](pid_t pid, const std::string &cmdline) {
        printf("\033[30m[%d]\033[32m %s \033[36m", pid, cmdline.c_str());
        if (result.m_cmd == ServerCmd::CMD_STOP) {
			INFO(theApp.m_loger,"terminating....");
            auto v = kill(pid, SIGKILL);
            if (v == EPERM) {
                printf("\033[31mfail! no permission.");
            } else
				printf("done \n");
        } else
			printf("done \n");
    });



	WARN(theApp.m_loger,"Server: {}",g_SERVER_VERSION);
      auto worker=[cfg]()->int{     //后台任务
        SourceServer::CSourceServer server(theApp.m_ioService);
        INFO(theApp.m_loger,"starting Server");

        std::error_code ec;
        server.loadConfig(cfg["server"],ec);

        if(ec){
            ERR(theApp.m_loger,"Bad config file, {}",ec.message());
            return 1;
        }
        INFO(theApp.m_loger,"starting Server2");
        try{
            INFO(theApp.m_loger,"starting Server3");
            server.start([](const std::error_code& ec){
                INFO(theApp.m_loger,"starting Server2");
                if(!ec)
                {
					INFO(theApp.m_loger,"The Service is up and running ");
                }
                else
				{
                   ERR(theApp.m_loger,"ERRORFail to start error:{}",ec.message());
				}
            });
        }catch(exception e){
            ERR(theApp.m_loger,"ERR:Exception:{}",e.what());
        }
        theApp.m_ioService.run();
        return 0;
    };
    if(result.m_cmd == ServerCmd::CMD_START){
        auto rstm=time(nullptr)+10;  //最快10秒启动一次，防止意外条件下频繁重启
        theApp.on_worker_exit([&rstm](int status){
           //TODO: 检查子进程退出的原因，有些情况下是不需要重启的	
			if(BIND_FAILED_EXIT == (status>>8) && (0 == (status&0xff)))
			{
				WARN(theApp.m_loger,"Bind Failed ");
				exit(1);
			}
			ERR(theApp.m_loger,"ERR:On Worker Exit{}",status);
            auto tmnow=time(nullptr);
            if(tmnow<rstm){
                sleep(rstm-tmnow);
            }
			rstm=time(nullptr)+10;
        }).on_fork_error([](int level){
			WARN(theApp.m_loger,"on fork error :{}",level);
            sleep(10);
        }).daemon(worker);
    }
    return 0;
}
#endif