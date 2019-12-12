#ifndef _COMMON_FUNCTION_H_
#define _COMMON_FUNCTION_H_

#ifdef _WIN32
	#else
	#include <dirent.h>
	#include <unistd.h>
#endif

#include <functional>
#include <string>
#include <memory>
#include <fstream>
#include <iostream>


#include <string.h>

#include "CommonConfig.h"
#include "Log.h"
#include "json11.hpp"

#ifdef  _WIN32
	#include <windows.h>
#endif 


std::string unified_path(const char* path);
int load_txtfile(const char *file, std::string &content);

int write_txtfile(const char *file, std::string &content);
//打印参数选项
void PrintParamOption();
enum class ServerCmd
{
    CMD_STOP,
    CMD_START,
    CMD_RESTART,
    CMD_INFO,
};
struct ParseParamResult
{
    std::string m_strTag;
    ServerCmd m_cmd;
    std::string m_cfgFile;
    bool m_bReturn;
};
void RunProgram(ParseParamResult result);
ParseParamResult ParseParam(int argc, char * argv[]);
std::shared_ptr<spdlog::logger> CreateLogger(json11::Json& cfg);
#endif