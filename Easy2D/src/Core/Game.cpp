#include <easy2d/e2dbase.h>
#include <easy2d/e2dmanager.h>
#include <easy2d/e2dtool.h>
#include <easy2d/GLRenderer.h>
#include <SDL.h>
#include <cstdio>  // for std::remove


// 控制游戏终止
static bool s_bEndGame = true;
// 控制游戏暂停
static bool s_bPaused = false;
// 是否进行过初始化
static bool s_bInitialized = false;
// 游戏唯一标识
static easy2d::String s_sUniqueName;


bool easy2d::Game::init(const String& title, int width, int height, const String& uniqueName, bool singleton)
{
	if (s_bInitialized)
	{
		E2D_WARNING("The game has been initialized!");
		return false;
	}

	// 初始化日志系统
	Logger::initialize();
	E2D_LOG("Easy2D Game initializing...");

	// 保存唯一标识
	s_sUniqueName = uniqueName.empty() ? title : uniqueName;

	if (singleton)
	{
		// 使用文件锁实现单例检测
		String lockFileName = "Easy2DApp-" + s_sUniqueName + ".lock";
		SDL_RWops* lockFile = SDL_RWFromFile(lockFileName.c_str(), "r");
		if (lockFile != nullptr)
		{
			// 文件已存在，说明程序已在运行
			SDL_RWclose(lockFile);
			// 弹窗提示
			// SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "提示", "游戏已在其他窗口中打开！", nullptr);
			E2D_ERROR("游戏已在其他窗口中打开！");
			SDL_Quit();
			return false;
		}
		// 创建锁文件
		lockFile = SDL_RWFromFile(lockFileName.c_str(), "w");
		if (lockFile)
		{
			SDL_RWwrite(lockFile, "1", 1, 1);
			SDL_RWclose(lockFile);
		}
	}

	// 初始化SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
	{
		E2D_ERROR("SDL_Init Failed: %s", SDL_GetError());
		return false;
	}

	// 初始化窗口
	if (!Window::__init(title, width, height))
	{
		E2D_ERROR("初始化窗口失败");
		SDL_Quit();
		return false;
	}

	// 初始化OpenGL渲染器
	SDL_Window* sdlWindow = Window::getSDLWindow();
	if (!E2D_GL_RENDERER.initialize(sdlWindow, width, height))
	{
		E2D_ERROR("初始化OpenGL渲染器失败");
		Window::__uninit();
		SDL_Quit();
		return false;
	}

	// 初始化输入系统
	if (!Input::__init())
	{
		E2D_ERROR("初始化输入系统失败");
		E2D_GL_RENDERER.shutdown();
		Window::__uninit();
		SDL_Quit();
		return false;
	}

	// 初始化播放器
	if (!Music::__init())
	{
		E2D_ERROR("初始化miniaudio失败");
	}

	// 初始化Path
	if (!Path::__init(s_sUniqueName))
	{
		E2D_WARNING("Path::__init failed!");
	}

	// 初始化成功
	s_bInitialized = true;

	return s_bInitialized;
}

void easy2d::Game::start(int fpsLimit)
{
	if (!s_bInitialized)
	{
		E2D_ERROR("开始游戏前未进行初始化");
		return;
	}

	// 初始化场景管理器
	SceneManager::__init();

	// 显示窗口
	SDL_ShowWindow(Window::getSDLWindow());

	// 初始化计时
	Time::__init(fpsLimit);

	s_bEndGame = false;

	while (!s_bEndGame)
	{
		// 处理窗口消息
		Window::__poll();

		// 刷新时间
		Time::__updateNow();

		// 判断是否达到了刷新状态
		if (Time::__isReady())
		{
			Input::__update();			// 获取用户输入
			Timer::__update();			// 更新定时器
			ActionManager::__update();	// 更新动作管理器
			SceneManager::__update();	// 更新场景内容

			// 渲染游戏画面
			E2D_GL_RENDERER.beginFrame();
			E2D_GL_RENDERER.renderScene(E2D_GL_RENDERER.isDeviceResourceRecreated());
			E2D_GL_RENDERER.endFrame();

			GC::clear();				// 清理内存

			Time::__updateLast();		// 刷新时间信息
		}
		else
		{
			Time::__sleep();			// 挂起线程
		}
	}

	// 清除运行时产生的资源
	{
		// 删除动作
		ActionManager::__uninit();
		// 回收音乐播放器资源
		MusicPlayer::__uninit();
		// 清空定时器
		Timer::__uninit();
		// 删除所有场景
		SceneManager::__uninit();
		// 清理对象
		GC::clear();
	}
}

void easy2d::Game::pause()
{
	s_bPaused = true;
}

void easy2d::Game::resume()
{
	if (s_bInitialized && s_bPaused)
	{
		Game::reset();
	}
	s_bPaused = false;
}

void easy2d::Game::reset()
{
	if (s_bInitialized && !s_bEndGame)
	{
		Time::__reset();
		ActionManager::__resetAll();
		Timer::__resetAll();
	}
}

bool easy2d::Game::isPaused()
{
	return s_bPaused;
}

void easy2d::Game::quit()
{
	s_bEndGame = true;	// 这个变量将控制游戏是否结束
}

void easy2d::Game::destroy()
{
	if (!s_bInitialized)
		return;

	E2D_LOG("Easy2D Game destroying...");

	// 清空图片缓存
	Image::clearCache();
	// 回收音乐相关资源
	Music::__uninit();
	// 关闭输入
	Input::__uninit();
	// 关闭OpenGL渲染器
	E2D_GL_RENDERER.shutdown();
	// 销毁窗口
	Window::__uninit();
	// 关闭SDL
	SDL_Quit();

	// 删除单例锁文件
	String lockFileName = "Easy2DApp-" + s_sUniqueName + ".lock";
	std::remove(lockFileName.c_str());

	s_bInitialized = false;

	// 关闭日志系统
	Logger::shutdown();
}

easy2d::String easy2d::Game::getUniqueName()
{
	return s_sUniqueName;
}
