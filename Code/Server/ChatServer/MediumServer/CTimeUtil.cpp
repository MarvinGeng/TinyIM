/**
 * @file CTimeUtil.cpp
 * @author DennisMi (https://www.dennisthink.com/)
 * @brief 处理和时间有关的问题
 * @version 0.1
 * @date 2019-11-25
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include "CTimeUtil.h"
#include <time.h>

/**
 * @brief 获取当前时间 返回格式为 YYYY-MM-DD hh:mm:ss
 * 
 * @return std::string 
 */
std::string CTimeUtil::GetYMD_HMS_Time() {
	time_t nowTime = time(nullptr);
	struct tm nowTm;
#ifdef _WIN32
    localtime_s(&nowTm,&nowTime);
	char buff[32] = { 0 };
	sprintf_s(buff, "%04d-%02d-%02d %02d:%02d:%02d ",
		nowTm.tm_year + 1900,
		nowTm.tm_mon + 1,
		nowTm.tm_mday,
		nowTm.tm_hour,
		nowTm.tm_min,
		nowTm.tm_sec);
#else
	localtime_r(&nowTime,&nowTm);
	char buff[32] = { 0 };
	sprintf(buff, "%04d-%02d-%02d %02d:%02d:%02d ",
		nowTm.tm_year + 1900,
		nowTm.tm_mon + 1,
		nowTm.tm_mday,
		nowTm.tm_hour,
		nowTm.tm_min,
		nowTm.tm_sec);
#endif
	return buff;
}