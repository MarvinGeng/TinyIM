
#ifndef __LOG_H_
#define __LOG_H_
#include <spdlog/spdlog.h>
#include <spdlog/sinks/sink.h>
#include <spdlog/sinks/daily_file_sink.h>
//#include <spdlog/sinks/syslog_sink.h>
#include <string.h>
#define __FILENAME__ (strrchr(__FILE__, '\\') ? (strrchr(__FILE__, '\\') + 1) : __FILE__)
#ifdef WIN32
#define INFO(LOG,...) { if(LOG)  LOG->info(__VA_ARGS__);}
#define WARN(LOG,...) { if(LOG)  LOG->warn(__VA_ARGS__);}
#define ERR(LOG,...)  { if(LOG) LOG->error(__VA_ARGS__);}
#define ERR_LOG(LOG,...)  { if(LOG) LOG->error(__VA_ARGS__);}
#define DBG(LOG,...) { if(LOG) LOG->debug(__VA_ARGS__);}
#define RUN_HERE(LOG,...) {if(LOG) LOG->info("Run Here [{} {}]",__FILENAME__,__LINE__);}
#else
#define INFO(LOG,msg...) LOG->info(msg)
#define WARN(LOG,msg...) LOG->warn(msg)
#define ERR(LOG,msg...)  LOG->error(msg)
#define DBG(LOG,msg...) LOG->debug(msg)
#endif

#endif