#include <easy2d/e2dbase.h>
#include <iostream>
#include <fstream>

// spdlog 头文件
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>

namespace
{
	bool s_bEnable = true;
	std::shared_ptr<spdlog::logger> s_logger = nullptr;
	bool s_initialized = false;

	std::streambuf* s_cinBuffer, * s_coutBuffer, * s_cerrBuffer;
	std::fstream s_consoleInput, s_consoleOutput, s_consoleError;

	std::wstreambuf* s_wcinBuffer, * s_wcoutBuffer, * s_wcerrBuffer;
	std::wfstream s_wconsoleInput, s_wconsoleOutput, s_wconsoleError;
}

namespace 
{
	void RedirectStdIO()
	{
		s_cinBuffer = std::cin.rdbuf();
		s_coutBuffer = std::cout.rdbuf();
		s_cerrBuffer = std::cerr.rdbuf();
		s_wcinBuffer = std::wcin.rdbuf();
		s_wcoutBuffer = std::wcout.rdbuf();
		s_wcerrBuffer = std::wcerr.rdbuf();

		s_consoleInput.open("CONIN$", std::ios::in);
		s_consoleOutput.open("CONOUT$", std::ios::out);
		s_consoleError.open("CONOUT$", std::ios::out);
		s_wconsoleInput.open("CONIN$", std::ios::in);
		s_wconsoleOutput.open("CONOUT$", std::ios::out);
		s_wconsoleError.open("CONOUT$", std::ios::out);

		FILE* dummy;
		::freopen_s(&dummy, "CONOUT$", "w+t", stdout);
		::freopen_s(&dummy, "CONIN$", "r+t", stdin);
		::freopen_s(&dummy, "CONOUT$", "w+t", stderr);
		(void)dummy;

		std::cin.rdbuf(s_consoleInput.rdbuf());
		std::cout.rdbuf(s_consoleOutput.rdbuf());
		std::cerr.rdbuf(s_consoleError.rdbuf());
		std::wcin.rdbuf(s_wconsoleInput.rdbuf());
		std::wcout.rdbuf(s_wconsoleOutput.rdbuf());
		std::wcerr.rdbuf(s_wconsoleError.rdbuf());
	}

	void ResetStdIO()
	{
		s_consoleInput.close();
		s_consoleOutput.close();
		s_consoleError.close();
		s_wconsoleInput.close();
		s_wconsoleOutput.close();
		s_wconsoleError.close();

		std::cin.rdbuf(s_cinBuffer);
		std::cout.rdbuf(s_coutBuffer);
		std::cerr.rdbuf(s_cerrBuffer);
		std::wcin.rdbuf(s_wcinBuffer);
		std::wcout.rdbuf(s_wcoutBuffer);
		std::wcerr.rdbuf(s_wcerrBuffer);

		fclose(stdout);
		fclose(stdin);
		fclose(stderr);

		s_cinBuffer = nullptr;
		s_coutBuffer = nullptr;
		s_cerrBuffer = nullptr;
		s_wcinBuffer = nullptr;
		s_wcoutBuffer = nullptr;
		s_wcerrBuffer = nullptr;
	}

	HWND allocated_console = nullptr;

	HWND AllocateConsole()
	{
		if (::AllocConsole())
		{
			allocated_console = ::GetConsoleWindow();

			if (allocated_console)
			{
				RedirectStdIO();
			}
		}
		return allocated_console;
	}

	void FreeAllocatedConsole()
	{
		if (allocated_console)
		{
			ResetStdIO();
			::FreeConsole();
			allocated_console = nullptr;
		}
	}

	HWND GetAllocatedConsole()
	{
		return allocated_console;
	}
}

void easy2d::Logger::enable()
{
	s_bEnable = true;
	if (s_logger)
	{
		s_logger->set_level(spdlog::level::trace);
	}
}

void easy2d::Logger::disable()
{
	s_bEnable = false;
	if (s_logger)
	{
		s_logger->set_level(spdlog::level::off);
	}
}

void easy2d::Logger::initialize()
{
	if (s_initialized)
	{
		return;
	}

	try
	{
		// 创建多 sink 日志器
		std::vector<spdlog::sink_ptr> sinks;

		// 1. 控制台彩色输出
		auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		console_sink->set_level(spdlog::level::trace);
		console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
		sinks.push_back(console_sink);

		// 2. 文件日志（带轮转）
		auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("logs/easy2d.log", 5 * 1024 * 1024, 3);
		file_sink->set_level(spdlog::level::trace);
		file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
		sinks.push_back(file_sink);

#ifdef _WIN32
		// 3. Visual Studio 调试输出窗口（仅 Windows）
		auto msvc_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
		msvc_sink->set_level(spdlog::level::trace);
		msvc_sink->set_pattern("[%l] %v");
		sinks.push_back(msvc_sink);
#endif

		// 创建 logger
		s_logger = std::make_shared<spdlog::logger>("easy2d", sinks.begin(), sinks.end());
		s_logger->set_level(spdlog::level::trace);
		s_logger->flush_on(spdlog::level::warn);

		// 注册为默认 logger
		spdlog::set_default_logger(s_logger);

		s_initialized = true;
	}
	catch (const spdlog::spdlog_ex& ex)
	{
		// 初始化失败，使用备用方案
		std::cerr << "Logger initialization failed: " << ex.what() << std::endl;
	}
}

void easy2d::Logger::shutdown()
{
	if (s_logger)
	{
		s_logger->flush();
		s_logger.reset();
	}
	spdlog::shutdown();
	s_initialized = false;
}

void easy2d::Logger::setLevel(LogLevel level)
{
	if (!s_logger)
	{
		return;
	}

	spdlog::level::level_enum spd_level = spdlog::level::trace;
	switch (level)
	{
	case LogLevel::Trace:
		spd_level = spdlog::level::trace;
		break;
	case LogLevel::Debug:
		spd_level = spdlog::level::debug;
		break;
	case LogLevel::Info:
		spd_level = spdlog::level::info;
		break;
	case LogLevel::Warn:
		spd_level = spdlog::level::warn;
		break;
	case LogLevel::Error:
		spd_level = spdlog::level::err;
		break;
	case LogLevel::Critical:
		spd_level = spdlog::level::critical;
		break;
	case LogLevel::Off:
		spd_level = spdlog::level::off;
		break;
	}
	s_logger->set_level(spd_level);
}

void easy2d::Logger::trace(String format, ...)
{
	if (!s_bEnable || !s_logger)
		return;

	va_list args;
	va_start(args, format);
	char buffer[4096];
	vsnprintf(buffer, sizeof(buffer), format.c_str(), args);
	va_end(args);

	s_logger->trace(buffer);
}

void easy2d::Logger::debug(String format, ...)
{
	if (!s_bEnable || !s_logger)
		return;

	va_list args;
	va_start(args, format);
	char buffer[4096];
	vsnprintf(buffer, sizeof(buffer), format.c_str(), args);
	va_end(args);

	s_logger->debug(buffer);
}

void easy2d::Logger::info(String format, ...)
{
	if (!s_bEnable || !s_logger)
		return;

	va_list args;
	va_start(args, format);
	char buffer[4096];
	vsnprintf(buffer, sizeof(buffer), format.c_str(), args);
	va_end(args);

	s_logger->info(buffer);
}

void easy2d::Logger::warn(String format, ...)
{
	if (!s_bEnable || !s_logger)
		return;

	va_list args;
	va_start(args, format);
	char buffer[4096];
	vsnprintf(buffer, sizeof(buffer), format.c_str(), args);
	va_end(args);

	s_logger->warn(buffer);
}

void easy2d::Logger::error(String format, ...)
{
	if (!s_bEnable || !s_logger)
		return;

	va_list args;
	va_start(args, format);
	char buffer[4096];
	vsnprintf(buffer, sizeof(buffer), format.c_str(), args);
	va_end(args);

	s_logger->error(buffer);
}

void easy2d::Logger::critical(String format, ...)
{
	if (!s_bEnable || !s_logger)
		return;

	va_list args;
	va_start(args, format);
	char buffer[4096];
	vsnprintf(buffer, sizeof(buffer), format.c_str(), args);
	va_end(args);

	s_logger->critical(buffer);
}

void easy2d::Logger::messageln(String format, ...)
{
	if (!s_bEnable || !s_logger)
		return;

	va_list args;
	va_start(args, format);
	char buffer[4096];
	vsnprintf(buffer, sizeof(buffer), format.c_str(), args);
	va_end(args);

	s_logger->info(buffer);
}

void easy2d::Logger::warningln(String format, ...)
{
	if (!s_bEnable || !s_logger)
		return;

	va_list args;
	va_start(args, format);
	char buffer[4096];
	vsnprintf(buffer, sizeof(buffer), format.c_str(), args);
	va_end(args);

	s_logger->warn(buffer);
}

void easy2d::Logger::errorln(String format, ...)
{
	if (!s_bEnable || !s_logger)
		return;

	va_list args;
	va_start(args, format);
	char buffer[4096];
	vsnprintf(buffer, sizeof(buffer), format.c_str(), args);
	va_end(args);

	s_logger->error(buffer);
}

void easy2d::Logger::showConsole(bool show)
{
	HWND currConsole = ::GetConsoleWindow();
	if (show)
	{
		if (currConsole)
		{
			::ShowWindow(currConsole, SW_SHOW);
		}
		else
		{
			HWND console = ::AllocateConsole();
			if (!console)
			{
				E2D_WARNING("AllocConsole failed");
			}
			else
			{
				// disable the close button of console
				HMENU hmenu = ::GetSystemMenu(console, FALSE);
				::RemoveMenu(hmenu, SC_CLOSE, MF_BYCOMMAND);
			}
		}
	}
	else
	{
		if (currConsole)
		{
			if (currConsole == GetAllocatedConsole())
			{
				FreeAllocatedConsole();
			}
			else
			{
				::ShowWindow(currConsole, SW_HIDE);
			}
		}
	}
}
