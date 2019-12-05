#include "stdafx.h"
#include "IULog.h"
#include <vector>
#include "Path.h"
static std::shared_ptr<spdlog::logger> g_loger = nullptr;
std::shared_ptr<spdlog::logger> CreateLogger()
{
	if (nullptr == g_loger)
	{
		std::string strDebug = Hootina::CPath::GetAppPathAscii() + "\\Logs\\";
		std::vector<spdlog::sink_ptr> sinks;
		srand(static_cast<unsigned int>(time(nullptr)));
		auto debugFile = std::make_shared<spdlog::sinks::daily_file_sink_st>(strDebug+std::to_string(rand()) + ".txt", 00, 00, true);

		debugFile->set_level(spdlog::level::trace);
		auto consoleSink = std::make_shared<spdlog::sinks::wincolor_stdout_sink_st>();

		sinks.push_back(debugFile);
		sinks.push_back(consoleSink);

		g_loger = std::make_shared<spdlog::logger>("T", begin(sinks), end(sinks));
	}
	return g_loger;
}