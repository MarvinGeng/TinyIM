#ifndef __LOG_H__
#define __LOG_H__
#include "asioNet/include/common/Log.h"
std::shared_ptr<spdlog::logger> CreateLogger();
#endif // !__LOG_H__