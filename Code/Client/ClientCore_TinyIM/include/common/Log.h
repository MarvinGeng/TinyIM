
#ifndef __LOG_H_
#define __LOG_H_
#include <spdlog/spdlog.h>
#include <spdlog/sinks/sink.h>
#include <spdlog/sinks/daily_file_sink.h>
//#include <spdlog/sinks/syslog_sink.h>
#include <string.h>
#ifdef _WIN32
#define LOG_INFO(LOG,...) if(LOG){LOG->info(__VA_ARGS__);}
#define LOG_WARN(LOG,...) if(LOG){LOG->warn(__VA_ARGS__);}
#define LOG_ERR(LOG,...)  if(LOG){LOG->error(__VA_ARGS__);}
#define LOG_DBG(LOG,...) if(LOG){LOG->debug(__VA_ARGS__);}
#else
#define INFO(LOG,msg...) LOG->info(msg)
#define WARN(LOG,msg...) LOG->warn(msg)
#define ERR(LOG,msg...)  LOG->error(msg)
#define DBG(LOG,msg...) LOG->debug(msg)
#endif
#define __FILENAME__ (strrchr(__FILE__, '\\') ? (strrchr(__FILE__, '\\') + 1) : __FILE__)
#endif