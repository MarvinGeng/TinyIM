#ifndef _COMMON_CONFIG_H_
#define _COMMON_CONFIG_H_
#include <string>
#define MAIN_VERSION 0		//���汾��
#define MINOR_VERSION 1		//�ΰ汾��
#define BUGFIX_VERSION 0	//bug�޸��汾��
#ifdef _WIN32
static char g_vBuf[32] = { 0 };
static std::string g_SERVER_VERSION = "V0.1.0  "+ std::string(__DATE__) + " " + std::string(__TIME__) + " " + std::to_string(_MSC_VER);
#else
static std::string g_SERVER_VERSION = "V0.1.0  "+ std::string(__DATE__)+" "+std::string(__TIME__)+" "+std::string(__VERSION__);
#endif

#define DEBUG 0
#endif
