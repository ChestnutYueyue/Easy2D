# Easy2D 核心系统 API 文档

## 1. 应用程序 (Application)

### 1.1 单例获取
```cpp
// 获取应用程序单例实例
auto& app = easy2d::Application::instance();
```

### 1.2 应用配置 (AppConfig)
```cpp
easy2d::AppConfig config;
config.title = "游戏标题";           // 窗口标题
config.width = 800;                  // 窗口宽度
config.height = 600;                 // 窗口高度
config.vsync = true;                 // 垂直同步
config.fpsLimit = 60;                // FPS限制(0表示无限制)

// 初始化应用
if (!app.init(config)) {
    E2D_LOG_ERROR("初始化失败！");
    return -1;
}
```

### 1.3 常用方法
```cpp
// 运行主循环
app.run();

// 退出应用
app.quit();

// 获取配置
const AppConfig& cfg = app.getConfig();
float width = cfg.width;
float height = cfg.height;

// 获取当前FPS
int fps = app.fps();

// 获取渲染器
RenderBackend& renderer = app.renderer();

// 获取输入系统
InputManager& input = app.input();

// 获取资源管理器
ResourceManager& resources = app.resources();

// 获取场景管理器
SceneManager& scenes = app.scenes();

// 获取定时器管理器
TimerManager& timers = app.timers();
```

---

## 2. 日志系统 (Logger)

### 2.1 初始化和关闭
```cpp
// 初始化日志系统
easy2d::Logger::init();

// 设置日志级别
easy2d::Logger::setLevel(easy2d::LogLevel::Info);
// 可选级别: Debug, Info, Warning, Error, Fatal

// 关闭日志系统
easy2d::Logger::shutdown();
```

### 2.2 日志宏
```cpp
// 调试日志
E2D_LOG_DEBUG("调试信息: {}", variable);

// 信息日志
E2D_LOG_INFO("应用启动成功");

// 警告日志
E2D_LOG_WARN("纹理加载失败，使用默认纹理");

// 错误日志
E2D_LOG_ERROR("严重错误: {}", errorMessage);

// 简写形式
E2D_DEBUG("调试信息");
E2D_INFO("信息日志");
E2D_WARN("警告日志");
E2D_ERROR("错误日志");
```

---

## 3. 主函数标准模板

```cpp
#include <easy2d/easy2d.h>
#include <easy2d/utils/logger.h>

using namespace easy2d;

int main() {
    // 1. 初始化日志
    Logger::init();
    Logger::setLevel(LogLevel::Info);
    
    E2D_LOG_INFO("========================");
    E2D_LOG_INFO("游戏启动");
    E2D_LOG_INFO("========================");
    
    // 2. 获取应用实例
    auto& app = Application::instance();
    
    // 3. 配置应用
    AppConfig config;
    config.title = "游戏标题";
    config.width = 800;
    config.height = 600;
    config.vsync = true;
    config.fpsLimit = 60;
    
    // 4. 初始化应用
    if (!app.init(config)) {
        E2D_LOG_ERROR("初始化失败！");
        Logger::shutdown();
        return -1;
    }
    
    // 5. 进入初始场景
    app.enterScene(makePtr<YourScene>());
    
    // 6. 运行主循环
    E2D_LOG_INFO("启动主循环...");
    app.run();
    
    // 7. 清理
    E2D_LOG_INFO("应用结束。");
    Logger::shutdown();
    
    return 0;
}
```

---

## 4. 资源路径管理

### 4.1 添加搜索路径
```cpp
auto& resources = app.resources();

// 添加相对路径
resources.addSearchPath("assets");
resources.addSearchPath("assets/images");
resources.addSearchPath("assets/audio");

// 添加绝对路径（Windows）
resources.addSearchPath("C:/Windows/Fonts");

// 添加多层级路径
resources.addSearchPath("Easy2D/examples/demo/assets");
```

### 4.2 获取可执行文件目录
```cpp
#include <filesystem>

std::filesystem::path getExecutableDir(int argc, char** argv) {
    if (argc <= 0 || argv == nullptr || argv[0] == nullptr) {
        return std::filesystem::current_path();
    }
    
    std::error_code ec;
    auto exePath = std::filesystem::absolute(argv[0], ec);
    if (ec) {
        return std::filesystem::current_path();
    }
    return exePath.parent_path();
}

// 使用
const auto exeDir = getExecutableDir(argc, argv);
resources.addSearchPath(exeDir.string());
resources.addSearchPath((exeDir / "assets").string());
```

---

## 5. 数学工具

### 5.1 向量 (Vec2)
```cpp
// 创建向量
Vec2 pos(100.0f, 200.0f);
Vec2 origin = Vec2::ZERO;  // (0, 0)

// 向量运算
Vec2 a(1.0f, 2.0f);
Vec2 b(3.0f, 4.0f);
Vec2 c = a + b;      // (4, 6)
Vec2 d = a - b;      // (-2, -2)
Vec2 e = a * 2.0f;   // (2, 4)
float len = a.length();
```

### 5.2 矩形 (Rect)
```cpp
// 创建矩形
Rect rect(100.0f, 200.0f, 50.0f, 50.0f);  // x, y, width, height

// 获取属性
float x = rect.x;
float y = rect.y;
float w = rect.width;
float h = rect.height;

// 判断点是否在矩形内
bool contains = rect.contains(Vec2(120.0f, 220.0f));
```

### 5.3 颜色 (Color)
```cpp
// RGBA颜色 (0.0f - 1.0f)
Color red(1.0f, 0.0f, 0.0f, 1.0f);
Color semiTransparent(1.0f, 1.0f, 1.0f, 0.5f);

// 预定义颜色
easy2d::Colors::White
easy2d::Colors::Black
easy2d::Colors::Red
easy2d::Colors::Green
easy2d::Colors::Blue
easy2d::Colors::Yellow
easy2d::Colors::Transparent
```

---

## 6. 智能指针 (Ptr)

```cpp
// 创建对象并包装为智能指针
auto sprite = easy2d::makePtr<easy2d::Sprite>();
auto node = easy2d::makePtr<easy2d::Node>();

// 自定义类的智能指针
class MyNode : public easy2d::Node {};
auto myNode = easy2d::makePtr<MyNode>();
```

---

## 7. 定时器 (TimerManager)

```cpp
// 添加一次性定时器
app.timers().addTimer(5.0f, []() {
    E2D_INFO("5秒后执行");
});

// 添加带参数的定时器
float autoQuitSeconds = 10.0f;
app.timers().addTimer(autoQuitSeconds, [&app]() { 
    app.quit(); 
});
```

---

## 8. 完整示例

```cpp
#include <easy2d/easy2d.h>
#include <easy2d/utils/logger.h>

using namespace easy2d;

class DemoScene : public Scene {
public:
    void onEnter() override {
        setBackgroundColor(Color(0.1f, 0.1f, 0.2f, 1.0f));
        E2D_INFO("场景进入");
    }
    
    void onUpdate(float dt) override {
        // 每帧更新
    }
    
    void onRender(RenderBackend& renderer) override {
        // 渲染代码
    }
};

int main() {
    Logger::init();
    Logger::setLevel(LogLevel::Info);
    
    auto& app = Application::instance();
    
    AppConfig config;
    config.title = "Easy2D 核心系统演示";
    config.width = 800;
    config.height = 600;
    config.vsync = true;
    
    if (!app.init(config)) {
        E2D_LOG_ERROR("初始化失败！");
        return -1;
    }
    
    // 添加资源路径
    auto& resources = app.resources();
    resources.addSearchPath("assets");
    
    // 进入场景
    app.enterScene(makePtr<DemoScene>());
    
    app.run();
    
    Logger::shutdown();
    return 0;
}
```
