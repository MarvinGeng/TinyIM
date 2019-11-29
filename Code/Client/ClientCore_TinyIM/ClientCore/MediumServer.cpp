#include "MainFunc.h"
#include "CFileUtil.h"
#ifdef _WIN32
int main(int argc, char *argv[])
{
	//if (argc < 2)
	//{
	//	PrintParamOption();
	//	return 0;
	//}

	HANDLE singleEvent_;
	std::string singleName = "Global\\ClientCore"+std::string(argv[1]);	// 全局唯一
	singleEvent_ = ::CreateEvent(NULL, FALSE, FALSE, singleName.data());

	if (!singleEvent_ || ::GetLastError() == ERROR_ALREADY_EXISTS)
	{
		return 0;
	}

	ParseParamResult result;
	CFileUtil util;
	result.m_cfgFile = util.GetCurDir() + "\\ClientCore.cfg";
	RunProgram(result);
	::CloseHandle(singleEvent_);
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
    MediumServer::CClientSess::ms_loger = theApp.m_loger;
	MediumServer::CServerSess::ms_loger = theApp.m_loger;
	MediumServer::CMediumServer::ms_loger = theApp.m_loger;
	MediumServer::CClientSessManager::ms_loger = theApp.m_loger;

    theApp.list([result](pid_t pid, const std::string &cmdline) {
        printf("\033[30m[%d]\033[32m %s \033[36m", pid, cmdline.c_str());
        if (result.m_cmd == ServerCmd::CMD_STOP) {
			LOG_INFO(theApp.m_loger,"terminating....");
            auto v = kill(pid, SIGKILL);
            if (v == EPERM) {
                printf("\033[31mfail! no permission.");
            } else
				printf("done \n");
        } else
			printf("done \n");
    });


	LOG_WARN(theApp.m_loger,"Server: {}",g_SERVER_VERSION);
      auto worker=[cfg]()->int{     //后台任务
        auto server = std::make_shared<MediumServer::CMediumServer>(theApp.m_ioService);
        LOG_INFO(theApp.m_loger,"starting Server [ {} {} ]",__FILENAME__,__LINE__);

        std::error_code ec;
        server->loadConfig(cfg,ec);

        if(ec){
            LOG_ERR(theApp.m_loger,"Bad config file, {}",ec.message());
            return 1;
        }
        LOG_INFO(theApp.m_loger,"starting Server2");
        try{
            LOG_INFO(theApp.m_loger,"starting Server3");
             server->start([](const std::error_code& ec){
                LOG_INFO(theApp.m_loger,"starting Server2");
                if(!ec)
                {
					LOG_INFO(theApp.m_loger,"The Service is up and running ");
                }
                else
				{
                   LOG_ERR(theApp.m_loger,"ERRORFail to start error:{}",ec.message());
				}
            });
        }catch(exception e){
            LOG_ERR(theApp.m_loger,"LOG_ERR:Exception:{}",e.what());
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
				LOG_WARN(theApp.m_loger,"Bind Failed ");
				exit(1);
			}
			LOG_ERR(theApp.m_loger,"LOG_ERR:On Worker Exit{}",status);
            auto tmnow=time(nullptr);
            if(tmnow<rstm){
                sleep(rstm-tmnow);
            }
			rstm=time(nullptr)+10;
        }).on_fork_error([](int level){
			LOG_WARN(theApp.m_loger,"on fork error :{}",level);
            sleep(10);
        }).daemon(worker);
    }
    return 0;
}
#endif
