#pragma once
#include <easy2d/e2dmacros.h>
#include <easy2d/e2dcommon.h>

// 前向声明SDL类型
struct SDL_Window;

// Base Classes

namespace easy2d
{


// 游戏控制
class Game
{
public:
	// 初始化游戏
	static bool init(
		const String& title = "Easy2D Game",	/* 窗口标题 */
		int width = 640,						/* 窗口宽度 */
		int height = 480,						/* 窗口高度 */
		const String& uniqueName = "",			/* 唯一标识 */
		bool singleton = false					/* 仅能启动一个游戏实例 */
	);

	// 启动游戏
	static void start(
		int fpsLimit = 0					/* FPS 限制，0为不限制 */
	);

	// 暂停游戏
	static void pause();

	// 继续游戏
	static void resume();

	// 结束游戏
	static void quit();

	// 回收游戏资源
	static void destroy();

	// 重置游戏内部计时
	static void reset();

	// 游戏是否暂停
	static bool isPaused();

	// 获取游戏唯一标识
	static String getUniqueName();
};

class Node;

// 窗口控制
class Window
{
	friend class Game;

public:
	// 鼠标指针样式
	enum class Cursor : int
	{
		None,		/* 无指针 */
		Normal,		/* 默认指针样式 */
		Hand,		/* 手状指针 */
		No,			/* 禁止指针 */
		Wait,		/* 沙漏指针 */
		ArrowWait,	/* 默认指针和小沙漏 */
	};

public:
	// 修改窗口大小
	static void setSize(
		int width,			/* 窗口宽度 */
		int height			/* 窗口高度 */
	);

	// 设置窗口标题
	static void setTitle(
		const String& title	/* 窗口标题 */
	);

	// 设置窗口图标（从资源ID加载，SDL2中已废弃）
	static void setIcon(
		int iconID
	);

	// 设置窗口图标（从文件路径加载，SDL2推荐方式）
	static void setIcon(
		const String& filePath
	);

	// 设置鼠标指针样式
	static void setCursor(
		Cursor cursor
	);

	// 设置自定义鼠标指针
	static void setCustomCursor(
		Node* cursor
	);

	// 设置自定义鼠标指针
	static void setCustomCursor(
		Function<Node*(Cursor)> cursorFunc
	);

	// 获取自定义鼠标指针
	static Node* getCustomCursor();

	// 获取窗口标题
	static String getTitle();

	// 获取窗口宽度
	static float getWidth();

	// 获取窗口高度
	static float getHeight();

	// 获取窗口大小
	static Size getSize();

	// 获取SDL窗口指针
	static SDL_Window* getSDLWindow();

	// 获取原生窗口句柄（Windows下为HWND）
	static void* getNativeWindowHandle();

	// 是否允许响应输入法
	static void setTypewritingEnable(
		bool enable
	);

	// 弹出提示窗口
	static void info(
		const String& text,					/* 内容 */
		const String& title = "Infomation"	/* 窗口标题 */
	);

	// 弹出警告窗口
	static void warning(
		const String& text,					/* 内容 */
		const String& title = "Warning"		/* 窗口标题 */
	);

	// 弹出错误窗口
	static void error(
		const String& text,					/* 内容 */
		const String& title = "Error"		/* 窗口标题 */
	);

private:
	// 初始化窗口
	static bool __init(
		const String& title,
		int width,
		int height
	);

	// 重置窗口属性
	static void __uninit();

	// 处理窗口消息
	static void __poll();

	// 更新指针
	static void __updateCursor();


};


// 时间控制
class Time
{
	friend class Game;

public:
	// 获取上一帧与当前帧的时间间隔（秒）
	static float getDeltaTime();

	// 获取上一帧与当前帧的时间间隔（毫秒）
	static unsigned int getDeltaTimeMilliseconds();

	// 获取游戏总时长（秒）
	static float getTotalTime();

	// 获取游戏总时长（毫秒）
	static unsigned int getTotalTimeMilliseconds();

private:
	// 初始化计时操作
	static void __init(int expectedFPS);

	// 是否达到更新时间
	static bool __isReady();

	// 更新当前时间
	static void __updateNow();

	// 更新时间信息
	static void __updateLast();

	// 重置时间信息
	static void __reset();

	// 挂起线程
	static void __sleep();
};


// 输入控制
class Input
{
	friend class Game;

public:
	// 检测键盘某按键是否正被按下
	static bool isDown(
		KeyCode::Value key
	);

	// 检测键盘某按键是否被点击
	static bool isPressed(
		KeyCode::Value key
	);

	// deprecated: 请使用 isPressed 替代
	// 检测键盘某按键是否被点击
	static inline bool isPress(
		KeyCode::Value key
	)
	{
		return Input::isPressed(key);
	}

	// 检测键盘某按键是否正在松开
	static bool isReleased(
		KeyCode::Value key
	);

	// deprecated: 请使用 isReleased 替代
	// 检测键盘某按键是否正在松开
	static inline bool isRelease(
		KeyCode::Value key
	)
	{
		return Input::isReleased(key);
	}

	// 检测鼠标按键是否正被按下
	static bool isDown(
		MouseCode::Value code
	);

	// 检测鼠标按键是否被点击
	static bool isPressed(
		MouseCode::Value code
	);

	// deprecated: 请使用 isPressed 替代
	// 检测鼠标按键是否被点击
	static inline bool isPress(
		MouseCode::Value code
	)
	{
		return Input::isPressed(code);
	}

	// 检测鼠标按键是否正在松开
	static bool isReleased(
		MouseCode::Value code
	);

	// deprecated: 请使用 isReleased 替代
	// 检测鼠标按键是否正在松开
	static inline bool isRelease(
		MouseCode::Value code
	)
	{
		return Input::isReleased(code);
	}

	// 获得鼠标X轴坐标值
	static float getMouseX();

	// 获得鼠标Y轴坐标值
	static float getMouseY();

	// 获得鼠标坐标值
	static Point getMousePos();

	// 获得鼠标X轴坐标增量
	static float getMouseDeltaX();

	// 获得鼠标Y轴坐标增量
	static float getMouseDeltaY();

	// 获得鼠标Z轴（鼠标滚轮）坐标增量
	static float getMouseDeltaZ();

private:
	// 初始化 DirectInput 以及键盘鼠标设备
	static bool __init();

	// 刷新输入信息
	static void __update();

	// 卸载 DirectInput
	static void __uninit();
};

// 前向声明
class TextLayout;
class GLRenderer;

// 渲染器
class Renderer
{
	friend class Game;
	friend class Window;

public:
	// 获取背景色
	static Color getBackgroundColor();

	// 修改背景色
	static void setBackgroundColor(
		Color color
	);

	// 显示 FPS
	static void showFps(
		bool show = true
	);

	// 显示身体形状
	static void showBodyShapes(
		bool show = true
	);

	// 设置垂直同步（会重载资源，可能造成卡顿）
	static void setVSync(
		bool enabled
	);

	// 是否开启了垂直同步
	static bool isVSyncEnabled();

	// 获取系统 DPI 缩放
	static float getDpiScaleX();

	// 获取系统 DPI 缩放
	static float getDpiScaleY();

	// 获取 ID2D1Factory 对象（已废弃，返回nullptr）
	static void* getID2D1Factory();

	// 获取渲染目标对象（返回GLRenderTarget指针）
	static void* getRenderTarget();

	// 获取 ID2D1SolidColorBrush 对象（已废弃，返回nullptr）
	static void* getSolidColorBrush();

	// 获取 IWICImagingFactory 对象（已废弃，返回nullptr）
	static void* getIWICImagingFactory();

	// 获取 IDWriteFactory 对象（已废弃，返回nullptr）
	static void* getIDWriteFactory();

	// 获取 Miter 样式的 ID2D1StrokeStyle（已废弃，返回nullptr）
	static void* getMiterID2D1StrokeStyle();

	// 获取 Bevel 样式的 ID2D1StrokeStyle（已废弃，返回nullptr）
	static void* getBevelID2D1StrokeStyle();

	// 获取 Round 样式的 ID2D1StrokeStyle（已废弃，返回nullptr）
	static void* getRoundID2D1StrokeStyle();

	// 渲染文字布局
	static void DrawTextLayout(
		TextLayout* layout,
		const DrawingStyle& style,
		const Point& offset = Point(),
		void* rt = nullptr,
		void* brush = nullptr
	);

	// 设备依赖资源是否刚刚重建
	static bool isDeviceResourceRecreated();

	// 获取 OpenGL 渲染器实例（新增）
	static GLRenderer* getGLRenderer();

private:
	// 渲染游戏画面
	static void __render();

	// 创建设备无关资源
	static bool __createDeviceIndependentResources();

	// 创建设备相关资源
	static bool __createDeviceResources();

	// 删除设备相关资源
	static void __discardDeviceResources();

	// 删除所有渲染相关资源
	static void __discardResources();
};


// 日志级别枚举
enum class LogLevel
{
	Trace = 0,		// 追踪信息
	Debug,			// 调试信息
	Info,			// 普通信息
	Warn,			// 警告信息
	Error,			// 错误信息
	Critical,		// 严重错误
	Off				// 关闭日志
};

// 日志
class Logger
{
public:
	// 初始化日志系统
	static void initialize();

	// 关闭日志系统
	static void shutdown();

	// 启用日志记录
	static void enable();

	// 关闭日志记录
	static void disable();

	// 设置日志级别
	static void setLevel(LogLevel level);

	// 输出追踪日志（最详细）
	static void trace(String format, ...);

	// 输出调试日志
	static void debug(String format, ...);

	// 输出普通信息
	static void info(String format, ...);

	// 输出警告
	static void warn(String format, ...);

	// 输出错误
	static void error(String format, ...);

	// 输出严重错误
	static void critical(String format, ...);

	// 输出日志消息（兼容旧接口）
	static void messageln(String format, ...);

	// 输出警告（兼容旧接口）
	static void warningln(String format, ...);

	// 输出错误（兼容旧接口）
	static void errorln(String format, ...);

	// 打开/关闭控制台
	static void showConsole(bool show = true);
};


// 垃圾回收装置
class GC
{
public:
	// 将对象放入 GC 池（新生代）
	static void trace(
		Object* pObject
	);

	// 将对象放入老年代（用于长期存活的对象）
	static void tracePersistent(
		Object* pObject
	);

	// 清理对象
	static void clear();

	// 强制完整GC（清理所有代）
	static void forceFullCollect();

	// 检查对象是否在 GC 池中
	static bool isInPool(Object* pObject);

	// 从 GC 池中移除对象（用于对象提前释放的安全处理）
	static void untrace(Object* pObject);

	// GC 池状态
	static bool isClearing();

	// 获取 GC 池中对象总数
	static size_t getPoolSize();

	// 获取新生代对象数量
	static size_t getYoungPoolSize();

	// 获取老年代对象数量
	static size_t getOldPoolSize();

	// 设置是否启用分代GC
	static void setGenerationalEnabled(bool enabled);

	// 是否启用了分代GC
	static bool isGenerationalEnabled();

	// 保留对象
	template <typename Type>
	static inline void retain(Type*& p)
	{
		if (p != nullptr)
		{
			p->retain();
		}
	}

	// 释放对象
	template <typename Type>
	static inline void release(Type*& p)
	{
		if (p != nullptr)
		{
			p->release();
			p = nullptr;
		}
	}
};


//
// GC macros
//

namespace __gc_helper
{
	class GCNewHelper
	{
	public:
		template <typename _Ty>
		inline _Ty* operator<< (_Ty* newObj) const
		{
			if (newObj)
			{
				newObj->autorelease();
			}
			return newObj;
		}

		// 使用 static 变量而不是 static inline 函数
		// 让 IntelliSense 正常工作
		static GCNewHelper instance;
	};
}

#ifndef gcnew
#	define gcnew __gc_helper::GCNewHelper::instance << new (std::nothrow)
#endif


//
// Log macros
//

// 追踪日志宏（仅在 Debug 模式下有效）
#ifndef E2D_TRACE
#   ifdef E2D_DEBUG
#       define E2D_TRACE(...) easy2d::Logger::trace(__VA_ARGS__)
#   else
#       ifdef _MSC_VER
#           define E2D_TRACE __noop
#       else
#           define E2D_TRACE(...) ((void)0)
#       endif
#   endif
#endif

// 调试日志宏（仅在 Debug 模式下有效）
#ifndef E2D_DEBUG_LOG
#   ifdef E2D_DEBUG
#       define E2D_DEBUG_LOG(...) easy2d::Logger::debug(__VA_ARGS__)
#   else
#       ifdef _MSC_VER
#           define E2D_DEBUG_LOG __noop
#       else
#           define E2D_DEBUG_LOG(...) ((void)0)
#       endif
#   endif
#endif

// 普通信息日志宏（仅在 Debug 模式下有效，兼容旧接口）
#ifndef E2D_LOG
#   ifdef E2D_DEBUG
#       define E2D_LOG(...) easy2d::Logger::info(__VA_ARGS__)
#   else
#       ifdef _MSC_VER
#           define E2D_LOG __noop
#       else
#           define E2D_LOG(...) ((void)0)
#       endif
#   endif
#endif

// 警告日志宏（始终有效）
#ifndef E2D_WARNING
#   define E2D_WARNING(...) easy2d::Logger::warn(__VA_ARGS__)
#endif

// 错误日志宏（始终有效）
#ifndef E2D_ERROR
#   define E2D_ERROR(...) easy2d::Logger::error(__VA_ARGS__)
#endif

// 严重错误日志宏（始终有效）
#ifndef E2D_CRITICAL
#   define E2D_CRITICAL(...) easy2d::Logger::critical(__VA_ARGS__)
#endif

// HRESULT 错误检查宏
#ifndef E2D_ERROR_IF_FAILED
#   define E2D_ERROR_IF_FAILED(HR, ...) do { if (FAILED(HR)) { E2D_ERROR(__VA_ARGS__); } } while (0)
#endif

// 断言宏（仅在 Debug 模式下有效）
#ifndef E2D_ASSERT
#   ifdef E2D_DEBUG
#       define E2D_ASSERT(COND, ...) \
            do { \
                if (!(COND)) { \
                    easy2d::Logger::critical("ASSERTION FAILED: " #COND); \
                    easy2d::Logger::critical(__VA_ARGS__); \
                    __debugbreak(); \
                } \
            } while(0)
#   else
#       define E2D_ASSERT(...) ((void)0)
#   endif
#endif

}
