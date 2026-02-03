#include <easy2d/e2dcommon.h>

easy2d::ByteString easy2d::FormatString(const char *format, ...) {
  easy2d::ByteString result;
  if (format) {
    va_list args = nullptr;
    va_start(args, format);

    const auto len = static_cast<size_t>(::_vscprintf(format, args) + 1);
    if (len) {
      result.resize(len - 1);
      ::_vsnprintf_s(&result[0], len, len, format, args);
    }
    va_end(args);
  }
  return result;
}

easy2d::WideString easy2d::FormatString(const wchar_t *format, ...) {
  easy2d::WideString result;
  if (format) {
    va_list args = nullptr;
    va_start(args, format);

    const auto len = static_cast<size_t>(::_vscwprintf(format, args) + 1);
    if (len) {
      result.resize(len - 1);
      ::_vsnwprintf_s(&result[0], len, len, format, args);
    }
    va_end(args);
  }
  return result;
}

easy2d::ByteString easy2d::WideToNarrow(const easy2d::WideString &str) {
  if (str.empty())
    return easy2d::ByteString();

  int num =
      ::WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, NULL, 0, NULL, NULL);
  if (num > 0) {
    easy2d::ByteString result;
    result.resize(num - 1);

    // C++11 保证了字符串是空结尾的
    ::WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, &result[0], num, NULL,
                          NULL);
    return result;
  }
  return easy2d::ByteString();
}

easy2d::WideString easy2d::NarrowToWide(const easy2d::ByteString &str) {
  if (str.empty())
    return easy2d::WideString();

  int num = ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
  if (num > 0) {
    easy2d::WideString result;
    result.resize(num - 1);

    // C++11 保证了字符串是空结尾的
    ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &result[0], num);
    return result;
  }
  return easy2d::WideString();
}

/**
 * @brief 将 ANSI 编码字符串转换为 UTF-8 编码字符串
 * @param str ANSI 编码的字符串
 * @return UTF-8 编码的字符串
 */
easy2d::ByteString easy2d::AnsiToUtf8(const easy2d::ByteString &str) {
  if (str.empty())
    return easy2d::ByteString();

  // 第一步：ANSI -> WideString
  int wideNum = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
  if (wideNum <= 0)
    return easy2d::ByteString();

  easy2d::WideString wideStr;
  wideStr.resize(wideNum - 1);
  ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, &wideStr[0], wideNum);

  // 第二步：WideString -> UTF-8
  int utf8Num = ::WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), -1, NULL, 0,
                                      NULL, NULL);
  if (utf8Num <= 0)
    return easy2d::ByteString();

  easy2d::ByteString result;
  result.resize(utf8Num - 1);
  ::WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), -1, &result[0], utf8Num,
                        NULL, NULL);

  return result;
}

/**
 * @brief 将 UTF-8 编码字符串转换为 ANSI 编码字符串
 * @param str UTF-8 编码的字符串
 * @return ANSI 编码的字符串
 */
easy2d::ByteString easy2d::Utf8ToAnsi(const easy2d::ByteString &str) {
  if (str.empty())
    return easy2d::ByteString();

  // 第一步：UTF-8 -> WideString
  int wideNum = ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
  if (wideNum <= 0)
    return easy2d::ByteString();

  easy2d::WideString wideStr;
  wideStr.resize(wideNum - 1);
  ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wideStr[0], wideNum);

  // 第二步：WideString -> ANSI
  int ansiNum = ::WideCharToMultiByte(CP_ACP, 0, wideStr.c_str(), -1, NULL, 0,
                                      NULL, NULL);
  if (ansiNum <= 0)
    return easy2d::ByteString();

  easy2d::ByteString result;
  result.resize(ansiNum - 1);
  ::WideCharToMultiByte(CP_ACP, 0, wideStr.c_str(), -1, &result[0], ansiNum,
                        NULL, NULL);

  return result;
}

/**
 * @brief 将 ANSI 编码字符串转换为宽字符串
 * @param str ANSI 编码的字符串
 * @return 宽字符串
 */
easy2d::WideString easy2d::AnsiToWide(const easy2d::ByteString &str) {
  if (str.empty())
    return easy2d::WideString();

  int num = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
  if (num > 0) {
    easy2d::WideString result;
    result.resize(num - 1);
    ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, &result[0], num);
    return result;
  }
  return easy2d::WideString();
}

/**
 * @brief 将宽字符串转换为 ANSI 编码字符串
 * @param str 宽字符串
 * @return ANSI 编码的字符串
 */
easy2d::ByteString easy2d::WideToAnsi(const easy2d::WideString &str) {
  if (str.empty())
    return easy2d::ByteString();

  int num =
      ::WideCharToMultiByte(CP_ACP, 0, str.c_str(), -1, NULL, 0, NULL, NULL);
  if (num > 0) {
    easy2d::ByteString result;
    result.resize(num - 1);
    ::WideCharToMultiByte(CP_ACP, 0, str.c_str(), -1, &result[0], num, NULL,
                          NULL);
    return result;
  }
  return easy2d::ByteString();
}
