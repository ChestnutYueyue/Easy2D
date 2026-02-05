#include <easy2d/base/e2dbase.h>
#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>


#ifdef _WIN32
#include <spdlog/sinks/msvc_sink.h>
#include <windows.h>
#endif

#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <vector>


namespace {
// Debug 模式下默认启用，Release 模式下默认禁用
#ifdef _DEBUG
bool s_bEnable = true;
#else
bool s_bEnable = false;
#endif
bool s_bConsoleAllocated = false;

// 保存原始流缓冲区
std::streambuf *s_cinBuffer = nullptr;
std::streambuf *s_coutBuffer = nullptr;
std::streambuf *s_cerrBuffer = nullptr;

// 控制台文件流
std::fstream s_consoleInput;
std::fstream s_consoleOutput;
std::fstream s_consoleError;

// freopen_s 返回的文件指针，需要保存以便关闭
FILE *s_fpStdout = nullptr;
FILE *s_fpStdin = nullptr;
FILE *s_fpStderr = nullptr;

// 线程安全锁
std::mutex s_logMutex;

#ifdef _WIN32
HWND s_allocatedConsole = nullptr;
#endif
} // namespace

namespace {
/**
 * @brief 使用 spdlog 格式化并输出日志
 * @param prompt 日志前缀提示
 * @param format 格式化字符串
 * @param args 可变参数列表
 * @return 格式化后的字符串
 */
std::string FormatLogMessage(const char *prompt, const char *format,
                             va_list args) {
  std::string result;

  if (prompt) {
    result = prompt;
  }

  if (format) {
    // 使用 vsnprintf 进行格式化
    va_list argsCopy;
    va_copy(argsCopy, args);

#ifdef _WIN32
    int len = ::_vscprintf(format, argsCopy);
#else
    int len = ::vsnprintf(nullptr, 0, format, argsCopy);
#endif
    va_end(argsCopy);

    if (len > 0) {
      std::vector<char> buffer(len + 1);
      ::vsnprintf(buffer.data(), buffer.size(), format, args);
      result += buffer.data();
    }
  }

  return result;
}

/**
 * @brief 输出日志到控制台和调试器
 * @param level 日志级别
 * @param prompt 日志前缀
 * @param format 格式化字符串
 * @param args 可变参数列表
 */
void OutputLog(spdlog::level::level_enum level, const char *prompt,
               const char *format, va_list args) {
  if (!s_bEnable)
    return;

  std::string message = FormatLogMessage(prompt, format, args);

  // 使用 spdlog 输出
  auto logger = spdlog::default_logger();
  if (logger) {
    switch (level) {
    case spdlog::level::debug:
      logger->debug(message);
      break;
    case spdlog::level::warn:
      logger->warn(message);
      break;
    case spdlog::level::err:
      logger->error(message);
      break;
    default:
      logger->info(message);
      break;
    }
  }

#ifdef _WIN32
  // Windows 平台额外输出到调试器
  message += "\n";
  ::OutputDebugStringA(message.c_str());
#endif
}

#ifdef _WIN32
/**
 * @brief 重定向标准 IO 流到控制台
 */
void RedirectStdIO() {
  s_cinBuffer = std::cin.rdbuf();
  s_coutBuffer = std::cout.rdbuf();
  s_cerrBuffer = std::cerr.rdbuf();

  s_consoleInput.open("CONIN$", std::ios::in);
  s_consoleOutput.open("CONOUT$", std::ios::out);
  s_consoleError.open("CONOUT$", std::ios::out);

  // 保存 freopen_s 返回的文件指针，以便后续正确关闭
  ::freopen_s(&s_fpStdout, "CONOUT$", "w+t", stdout);
  ::freopen_s(&s_fpStdin, "CONIN$", "r+t", stdin);
  ::freopen_s(&s_fpStderr, "CONOUT$", "w+t", stderr);

  std::cin.rdbuf(s_consoleInput.rdbuf());
  std::cout.rdbuf(s_consoleOutput.rdbuf());
  std::cerr.rdbuf(s_consoleError.rdbuf());
}

/**
 * @brief 恢复标准 IO 流
 */
void ResetStdIO() {
  // 先恢复 C++ 流缓冲区
  std::cin.rdbuf(s_cinBuffer);
  std::cout.rdbuf(s_coutBuffer);
  std::cerr.rdbuf(s_cerrBuffer);

  // 关闭 fstream 对象
  s_consoleInput.close();
  s_consoleOutput.close();
  s_consoleError.close();

  // 关闭 freopen_s 打开的文件指针
  if (s_fpStdout) {
    ::fclose(s_fpStdout);
    s_fpStdout = nullptr;
  }
  if (s_fpStdin) {
    ::fclose(s_fpStdin);
    s_fpStdin = nullptr;
  }
  if (s_fpStderr) {
    ::fclose(s_fpStderr);
    s_fpStderr = nullptr;
  }

  s_cinBuffer = nullptr;
  s_coutBuffer = nullptr;
  s_cerrBuffer = nullptr;
}
#endif
} // namespace

/**
 * @brief 初始化 spdlog 日志系统
 * @note 线程安全，使用双重检查锁定模式
 */
static void InitializeSpdlog() {
  static std::once_flag initFlag;
  std::call_once(initFlag, []() {
    // 创建多目标 sink
    std::vector<spdlog::sink_ptr> sinks;

    // 添加彩色控制台输出
    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    consoleSink->set_level(spdlog::level::debug);
    sinks.push_back(consoleSink);

#ifdef _WIN32
    // Windows 平台添加 Visual Studio 调试输出
    auto msvcSink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
    msvcSink->set_level(spdlog::level::debug);
    sinks.push_back(msvcSink);
#endif

    // 创建 logger
    auto logger =
        std::make_shared<spdlog::logger>("easy2d", sinks.begin(), sinks.end());
    logger->set_level(spdlog::level::debug);

    // 设置日志格式: [时间] [级别] 消息
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

    // 注册为默认 logger
    spdlog::set_default_logger(logger);
  });
}

/**
 * @brief 启用日志输出
 */
void easy2d::Logger::enable() {
  s_bEnable = true;
  InitializeSpdlog();
}

/**
 * @brief 禁用日志输出
 */
void easy2d::Logger::disable() { s_bEnable = false; }

/**
 * @brief 输出普通日志消息
 * @param format 格式化字符串
 * @param ... 可变参数
 */
void easy2d::Logger::messageln(String format, ...) {
  InitializeSpdlog();

  va_list args;
  va_start(args, format);

  OutputLog(spdlog::level::debug, "Debug: ", format.c_str(), args);

  va_end(args);
}

/**
 * @brief 输出警告日志消息
 * @param format 格式化字符串
 * @param ... 可变参数
 */
void easy2d::Logger::warningln(String format, ...) {
  InitializeSpdlog();

  va_list args;
  va_start(args, format);

  OutputLog(spdlog::level::warn, "Warning: ", format.c_str(), args);

  va_end(args);
}

/**
 * @brief 输出错误日志消息
 * @param format 格式化字符串
 * @param ... 可变参数
 */
void easy2d::Logger::errorln(String format, ...) {
  InitializeSpdlog();

  va_list args;
  va_start(args, format);

  OutputLog(spdlog::level::err, "Error: ", format.c_str(), args);

  va_end(args);
}

/**
 * @brief 显示或隐藏控制台窗口
 * @param show true 显示控制台，false 隐藏控制台
 * @note Windows 平台支持完整的控制台操作
 *       其他平台仅支持基本的 stdout 输出
 */
void easy2d::Logger::showConsole(bool show) {
#ifdef _WIN32
  HWND currConsole = ::GetConsoleWindow();
  if (show) {
    if (currConsole) {
      ::ShowWindow(currConsole, SW_SHOW);
    } else {
      if (::AllocConsole()) {
        s_allocatedConsole = ::GetConsoleWindow();
        s_bConsoleAllocated = true;

        if (s_allocatedConsole) {
          RedirectStdIO();

          // 禁用关闭按钮
          HMENU hmenu = ::GetSystemMenu(s_allocatedConsole, FALSE);
          if (hmenu) {
            ::RemoveMenu(hmenu, SC_CLOSE, MF_BYCOMMAND);
          }
        }
      } else {
        E2D_WARNING("AllocConsole failed");
      }
    }
  } else {
    if (currConsole) {
      if (s_bConsoleAllocated && currConsole == s_allocatedConsole) {
        ResetStdIO();
        ::FreeConsole();
        s_allocatedConsole = nullptr;
        s_bConsoleAllocated = false;
      } else {
        ::ShowWindow(currConsole, SW_HIDE);
      }
    }
  }
#else
  // 非 Windows 平台：仅记录状态，实际控制台由启动环境决定
  // 日志输出仍然通过 spdlog 工作
  (void)show;
#endif
}
