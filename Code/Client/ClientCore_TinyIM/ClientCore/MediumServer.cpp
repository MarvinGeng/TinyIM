﻿#include <iostream>
#include <memory>
#include "CMediumServer.h"
#include "DaemonSvcApp.h"
#include "CServerSess.h"
#include "CommonFunction.h"
#include "asio_common.h"
#include "Log.h"
#include "planform.h"
#include "CommonConfig.h"
using asio::ip::tcp;
using namespace std;

void GenerateCfgFile(std::string fileName)
{
    std::string strConfig=R"({
   "LogDir":"/home/test/Log/SourceServer/",
   "NodeId":"MediumServer",
   "server":{
        "ip":"127.0.0.1",
        "port":9000
      },
    
    "clients":[
        {
            "ip":"127.0.0.1",
            "port":9000
        }
    ],
   
   "clientsBin":[
        {
            "ip":"127.0.0.1",
            "port":9001
        }
    ]
    }
    )";
    write_txtfile(fileName.c_str(),strConfig);
}

#ifdef _WIN32
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
	ClientCore::CClientSess::ms_loger = logger;
	ClientCore::CServerSess::ms_loger = logger;
	ClientCore::CMediumServer::ms_loger = logger;
	//MediumServer::CClientSessManager::ms_loger = logger;

	asio::io_service IoService;
	auto server = std::make_shared<ClientCore::CMediumServer>(IoService);
	LOG_INFO(logger, "starting Server [{} {}] ",__FILENAME__,__LINE__);

	std::error_code ec;
	server->loadConfig(cfg, ec);
	server->start([logger](const std::error_code& ec) {
		LOG_INFO(logger, "starting Server2 [{} {}] ", __FILENAME__, __LINE__);
		if (!ec)
		{
			LOG_INFO(logger, "The Service is up and running [{} {}] ", __FILENAME__, __LINE__);
		}
		else
		{
			LOG_ERR(logger, "ERRORFail to start error:{} [{} {}] ", ec.message(), __FILENAME__, __LINE__);
		}
	});
	IoService.run();
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
