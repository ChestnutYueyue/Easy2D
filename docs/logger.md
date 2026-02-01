# Easy2D 日志系统文档

## 概述

Easy2D 使用 **spdlog** 作为日志后端，这是一个高性能的 C++ 日志库。日志系统支持多级别日志记录、多目标输出、自动轮转等功能。

## 日志级别

| 级别 | 值 | 说明 | 使用场景 |
|:---:|:---:|:---|:---|
| Trace | 0 | 最详细的追踪信息 | 函数入口、变量跟踪 |
| Debug | 1 | 调试信息 | 开发调试、状态输出 |
| Info | 2 | 普通信息 | 关键流程记录 |
| Warn | 3 | 警告信息 | 潜在问题提示 |
| Error | 4 | 错误信息 | 功能异常 |
| Critical | 5 | 严重错误 | 系统级错误 |
| Off | 6 | 关闭日志 | 禁用所有输出 |

## 日志宏

### Debug 模式专用（Release 模式下被优化掉）

```cpp
// 追踪日志
E2D_TRACE("Entering function: %s", __FUNCTION__);

// 调试日志
E2D_DEBUG_LOG("Player position: x=%f, y=%f", x, y);

// 信息日志（兼容旧接口）
E2D_LOG("Game initialized successfully");

// 断言
E2D_ASSERT(ptr != nullptr, "Pointer should not be null");
```

### 始终可用（Debug & Release）

```cpp
// 警告
E2D_WARNING("Texture not found: %s", textureName);

// 错误
E2D_ERROR("Failed to load shader: %s", errorMsg);

// 严重错误
E2D_CRITICAL("Out of memory!");

// HRESULT 错误检查
E2D_ERROR_IF_FAILED(hr, "DirectX call failed");
```

## Logger 类接口

### 初始化与关闭

```cpp
// 初始化日志系统（自动创建 logs 目录）
Logger::initialize();

// 关闭日志系统（自动 flush 缓冲区）
Logger::shutdown();
```

### 启用/禁用日志

```cpp
// 启用日志输出
Logger::enable();

// 禁用日志输出
Logger::disable();
```

### 设置日志级别

```cpp
// 设置全局日志级别
Logger::setLevel(LogLevel::Debug);

// 只显示警告及以上
Logger::setLevel(LogLevel::Warn);

// 关闭所有日志
Logger::setLevel(LogLevel::Off);
```

### 控制台控制

```cpp
// 显示控制台窗口
Logger::showConsole(true);

// 隐藏控制台窗口
Logger::showConsole(false);
```

### 直接输出接口

```cpp
// 各级别直接输出
Logger::trace("Trace message: %d", value);
Logger::debug("Debug message: %s", str);
Logger::info("Info message");
Logger::warn("Warning message");
Logger::error("Error message");
Logger::critical("Critical message");

// 兼容旧接口
Logger::messageln("Message");    // 映射到 info
Logger::warningln("Warning");    // 映射到 warn
Logger::errorln("Error");        // 映射到 error
```

## 日志输出目标

### 1. 控制台输出

- 彩色输出（Windows 支持 ANSI 颜色）
- 实时显示
- 格式：`[时间] [级别] 消息`

### 2. 文件输出

- 文件路径：`logs/easy2d.log`
- 自动轮转：单文件最大 5MB
- 保留数量：3 个备份（`easy2d.log.1`, `easy2d.log.2`, `easy2d.log.3`）
- 格式：`[时间] [级别] 消息`

### 3. VS 调试窗口

- 仅在 Windows + Visual Studio 环境下有效
- 格式：`[级别] 消息`

## 日志格式

### 控制台格式

```
[2024-01-15 10:30:25.123] [info] 消息内容
[2024-01-15 10:30:25.456] [warning] 警告内容
[2024-01-15 10:30:25.789] [error] 错误内容
```

### 文件格式

```
[2024-01-15 10:30:25.123] [info] Easy2D Game initializing...
[2024-01-15 10:30:25.456] [warning] Texture not found: player.png
[2024-01-15 10:30:25.789] [error] Shader compilation failed
```

## 最佳实践

### 1. 日志级别选择

```cpp
// Trace: 函数入口、执行流程
void MyFunction() {
    E2D_TRACE("Entering MyFunction");
    // ...
}

// Debug: 变量值、状态信息
E2D_DEBUG_LOG("Player HP: %d/%d", currentHP, maxHP);

// Info: 关键流程节点
E2D_LOG("Level %d loaded successfully", levelId);

// Warning: 可恢复的问题
E2D_WARNING("Using fallback texture for %s", textureName);

// Error: 功能异常
E2D_ERROR("Failed to save game: %s", errorMsg);

// Critical: 系统级错误
E2D_CRITICAL("Memory allocation failed!");
```

### 2. 性能考虑

```cpp
// 避免在循环中频繁输出日志
for (int i = 0; i < 10000; ++i) {
    // 不推荐：会严重降低性能
    // E2D_DEBUG_LOG("Processing item %d", i);
}

// 推荐：只在关键位置输出
E2D_DEBUG_LOG("Starting to process %d items", count);
for (int i = 0; i < 10000; ++i) {
    // 处理逻辑
}
E2D_DEBUG_LOG("Finished processing %d items", count);
```

### 3. 敏感信息处理

```cpp
// 不要在日志中记录敏感信息
// 不推荐：
E2D_LOG("User password: %s", password);

// 推荐：
E2D_LOG("User login attempt: %s", username);
```

## 配置说明

### 修改日志行为

日志系统的配置在 `Logger.cpp` 中，可以通过修改源码来调整：

```cpp
// 修改日志文件路径和大小限制
auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
    "logs/mygame.log",     // 文件路径
    10 * 1024 * 1024,      // 单文件大小：10MB
    5                      // 保留文件数：5个
);

// 修改输出格式
console_sink->set_pattern("[%H:%M:%S] [%^%l%$] %v");

// 修改日志级别
s_logger->set_level(spdlog::level::debug);
```

## 故障排除

### 日志文件未创建

1. 检查 `logs` 目录是否有写入权限
2. 确认 `Logger::initialize()` 已被调用
3. 检查磁盘空间是否充足

### 控制台无输出

1. 确认日志级别设置正确
2. 检查是否调用了 `Logger::disable()`
3. 确认 `Logger::showConsole(true)` 已被调用

### VS 调试窗口无输出

1. 确认在 Windows 平台下编译
2. 确认使用 Visual Studio 调试器运行
3. 检查输出窗口是否打开（视图 → 输出）

## 参考资料

- [spdlog GitHub](https://github.com/gabime/spdlog)
- [spdlog 文档](https://github.com/gabime/spdlog/wiki)
