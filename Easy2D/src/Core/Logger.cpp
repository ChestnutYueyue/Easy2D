#include <easy2d/e2dbase.h>
#include <iostream>
#include <fstream>

// spdlog 头文件
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>

// 平台特定头文件
#ifdef _WIN32
    #include <spdlog/sinks/msvc_sink.h>
    #include <windows.h>
#else
    #include <unistd.h>
    #include <sys/ioctl.h>
#endif

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
#ifdef _WIN32
	/**
	 * @brief 重定向标准输入输出流到 Windows 控制台
	 */
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

	/**
	 * @brief 重置标准输入输出流到原始状态
	 */
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

	/**
	 * @brief 分配 Windows 控制台窗口
	 * @return 控制台窗口句柄
	 */
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

	/**
	 * @brief 释放已分配的 Windows 控制台
	 */
	void FreeAllocatedConsole()
	{
		if (allocated_console)
		{
			ResetStdIO();
			::FreeConsole();
			allocated_console = nullptr;
		}
	}

	/**
	 * @brief 获取已分配的控制台窗口句柄
	 * @return 控制台窗口句柄
	 */
	HWND GetAllocatedConsole()
	{
		return allocated_console;
	}
#else
	// Unix/Linux 平台的全局变量
	bool s_consoleAllocated = false;

	/**
	 * @brief Unix/Linux 平台重定向标准输入输出流
	 */
	void RedirectStdIO()
	{
		s_cinBuffer = std::cin.rdbuf();
		s_coutBuffer = std::cout.rdbuf();
		s_cerrBuffer = std::cerr.rdbuf();

		// Unix/Linux 平台使用 /dev/tty 作为控制台设备
		s_consoleInput.open("/dev/tty", std::ios::in);
		s_consoleOutput.open("/dev/tty", std::ios::out);
		s_consoleError.open("/dev/tty", std::ios::out);

		if (s_consoleInput.is_open())
			std::cin.rdbuf(s_consoleInput.rdbuf());
		if (s_consoleOutput.is_open())
			std::cout.rdbuf(s_consoleOutput.rdbuf());
		if (s_consoleError.is_open())
			std::cerr.rdbuf(s_consoleError.rdbuf());
	}

	/**
	 * @brief Unix/Linux 平台重置标准输入输出流
	 */
	void ResetStdIO()
	{
		s_consoleInput.close();
		s_consoleOutput.close();
		s_consoleError.close();

		std::cin.rdbuf(s_cinBuffer);
		std::cout.rdbuf(s_coutBuffer);
		std::cerr.rdbuf(s_cerrBuffer);

		s_cinBuffer = nullptr;
		s_coutBuffer = nullptr;
		s_cerrBuffer = nullptr;
	}

	/**
	 * @brief Unix/Linux 平台分配控制台
	 * @return 是否成功分配
	 */
	bool AllocateConsoleUnix()
	{
		// Unix/Linux 平台通常已经有关联的终端
		// 这里我们只需要标记控制台已分配
		s_consoleAllocated = true;
		RedirectStdIO();
		return s_consoleAllocated;
	}

	/**
	 * @brief Unix/Linux 平台释放控制台
	 */
	void FreeAllocatedConsole()
	{
		if (s_consoleAllocated)
		{
			ResetStdIO();
			s_consoleAllocated = false;
		}
	}

	/**
	 * @brief Unix/Linux 平台检查是否已分配控制台
	 * @return 是否已分配
	 */
	bool IsConsoleAllocated()
	{
		return s_consoleAllocated;
	}
#endif
}

/**
 * @brief 启用日志记录
 */
void easy2d::Logger::enable()
{
	s_bEnable = true;
	if (s_logger)
	{
		s_logger->set_level(spdlog::level::trace);
	}
}

/**
 * @brief 禁用日志记录
 */
void easy2d::Logger::disable()
{
	s_bEnable = false;
	if (s_logger)
	{
		s_logger->set_level(spdlog::level::off);
	}
}

/**
 * @brief 初始化日志系统
 */
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

/**
 * @brief 关闭日志系统
 */
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

/**
 * @brief 设置日志级别
 * @param level 日志级别
 */
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

/**
 * @brief 记录 Trace 级别日志
 * @param format 格式化字符串
 * @param ... 可变参数
 */
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

/**
 * @brief 记录 Debug 级别日志
 * @param format 格式化字符串
 * @param ... 可变参数
 */
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

/**
 * @brief 记录 Info 级别日志
 * @param format 格式化字符串
 * @param ... 可变参数
 */
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

/**
 * @brief 记录 Warn 级别日志
 * @param format 格式化字符串
 * @param ... 可变参数
 */
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

/**
 * @brief 记录 Error 级别日志
 * @param format 格式化字符串
 * @param ... 可变参数
 */
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

/**
 * @brief 记录 Critical 级别日志
 * @param format 格式化字符串
 * @param ... 可变参数
 */
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

/**
 * @brief 记录普通消息日志（带换行）
 * @param format 格式化字符串
 * @param ... 可变参数
 */
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

/**
 * @brief 记录警告日志（带换行）
 * @param format 格式化字符串
 * @param ... 可变参数
 */
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

/**
 * @brief 记录错误日志（带换行）
 * @param format 格式化字符串
 * @param ... 可变参数
 */
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

/**
 * @brief 显示或隐藏控制台窗口
 * @param show true 显示控制台，false 隐藏控制台
 */
void easy2d::Logger::showConsole(bool show)
{
#ifdef _WIN32
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
#else
	// Unix/Linux 平台：控制台显示/隐藏通常由终端控制
	// 这里我们只记录状态，实际显示/隐藏由终端管理
	if (show)
	{
		if (!IsConsoleAllocated())
		{
			AllocateConsoleUnix();
		}
	}
	else
	{
		if (IsConsoleAllocated())
		{
			FreeAllocatedConsole();
		}
	}
#endif
}
