#include <easy2d/e2dbase.h>
#include <easy2d/e2dmanager.h>
#include <easy2d/e2dnode.h>
#include <easy2d/e2dtool.h>
#include <easy2d/GLRenderer.h>
#include <SDL3/SDL.h>
#include <stb_image.h>


// SDL窗口句柄
static SDL_Window* s_Window = nullptr;
static easy2d::Window::Cursor s_currentCursor = easy2d::Window::Cursor::Normal;

namespace easy2d
{
class CustomCursor
{
public:
	void update(Window::Cursor cursor)
	{
		if (_cursor != cursor)
		{
			_cursor = cursor;
			loadCursor();
		}
	}

	void loadCursor()
	{
		if (_cursorFunc)
		{
			auto newCursor = _cursorFunc(_cursor);
			if (newCursor != _cursorNode)
			{
				clear();
				_cursorNode = newCursor;
				if (_cursorNode)
				{
					_cursorNode->retain();
				}
			}
		}
		else
		{
			clear();
		}
	}

	void setCursorFunc(const Function<Node* (Window::Cursor)>& f)
	{
		_cursorFunc = f;
		_cursor = Window::Cursor(-1);
	}

	Node* getCursorNode() const
	{
		return _cursorNode;
	}

	void clear()
	{
		if (_cursorNode)
		{
			_cursorNode->release();
			_cursorNode = nullptr;
		}
	}

	operator bool() const
	{
		return _cursorNode != nullptr;
	}

private:
	Window::Cursor _cursor = Window::Cursor(-1);
	Function<Node* (Window::Cursor)> _cursorFunc = nullptr;
	Node* _cursorNode = nullptr;
};
}

static easy2d::CustomCursor s_customCursor;
static SDL_Cursor* s_systemCursor = nullptr;
static SDL_SystemCursor s_systemCursorType = SDL_SYSTEM_CURSOR_DEFAULT;

static void SetSystemCursor(SDL_SystemCursor cursorType)
{
	if (s_systemCursor && s_systemCursorType == cursorType)
	{
		SDL_SetCursor(s_systemCursor);
		return;
	}

	if (s_systemCursor)
	{
		SDL_DestroyCursor(s_systemCursor);
		s_systemCursor = nullptr;
	}

	s_systemCursor = SDL_CreateSystemCursor(cursorType);
	s_systemCursorType = cursorType;
	if (s_systemCursor)
	{
		SDL_SetCursor(s_systemCursor);
	}
}

bool easy2d::Window::__init(const String& title, int nWidth, int nHeight)
{
	// 初始化SDL视频子系统 (SDL3返回bool)
	if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
	{
		Window::error("SDL_Init Video Failed!");
		return false;
	}

	// 创建SDL窗口
	s_Window = SDL_CreateWindow(
		title.c_str(),
		nWidth,
		nHeight,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	);

	if (!s_Window)
	{
		Window::error("Create Window Failed!");
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		return false;
	}

	// 禁用最大化按钮和边框调整（保持与原Win32行为一致）
	SDL_SetWindowResizable(s_Window, false);

	return true;
}

void easy2d::Window::__uninit()
{
	s_customCursor.clear();
	if (s_systemCursor)
	{
		SDL_DestroyCursor(s_systemCursor);
		s_systemCursor = nullptr;
	}

	// 销毁SDL窗口
	if (s_Window)
	{
		SDL_DestroyWindow(s_Window);
		s_Window = nullptr;
	}

	// 关闭SDL视频子系统
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void easy2d::Window::__poll()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_EVENT_QUIT:
		{
			easy2d::Scene* pCurrentScene = easy2d::SceneManager::getCurrentScene();
			if (!pCurrentScene || pCurrentScene->onCloseWindow())
			{
				easy2d::Game::quit();
			}
		}
		break;

		case SDL_EVENT_KEY_DOWN:
		{
			KeyCode::Value vk = static_cast<KeyCode::Value>(event.key.key);
			KeyDownEvent evt(vk, event.key.repeat);
			SceneManager::dispatch(&evt);
		}
		break;

		case SDL_EVENT_KEY_UP:
		{
			KeyCode::Value vk = static_cast<KeyCode::Value>(event.key.key);
			KeyUpEvent evt(vk, event.key.repeat);
			SceneManager::dispatch(&evt);
		}
		break;

		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		{
			MouseCode::Value btn = MouseCode::Left;
			if (event.button.button == SDL_BUTTON_LEFT) btn = MouseCode::Left;
			else if (event.button.button == SDL_BUTTON_RIGHT) btn = MouseCode::Right;
			else if (event.button.button == SDL_BUTTON_MIDDLE) btn = MouseCode::Middle;

			MouseDownEvent evt(
				event.button.x,
				event.button.y,
				btn
			);
			SceneManager::dispatch(&evt);
		}
		break;

		case SDL_EVENT_MOUSE_BUTTON_UP:
		{
			MouseCode::Value btn = MouseCode::Left;
			if (event.button.button == SDL_BUTTON_LEFT) btn = MouseCode::Left;
			else if (event.button.button == SDL_BUTTON_RIGHT) btn = MouseCode::Right;
			else if (event.button.button == SDL_BUTTON_MIDDLE) btn = MouseCode::Middle;

			MouseUpEvent evt(
				event.button.x,
				event.button.y,
				btn
			);
			SceneManager::dispatch(&evt);
		}
		break;

		case SDL_EVENT_MOUSE_MOTION:
		{
			MouseMoveEvent evt(
				event.motion.x,
				event.motion.y
			);
			SceneManager::dispatch(&evt);
		}
		break;

		case SDL_EVENT_MOUSE_WHEEL:
		{
			float mouseX, mouseY;
			SDL_GetMouseState(&mouseX, &mouseY);
			Input::__onMouseWheel(event.wheel.y);
			MouseWheelEvent evt(
				mouseX,
				mouseY,
				event.wheel.y
			);
			SceneManager::dispatch(&evt);
		}
		break;

		case SDL_EVENT_WINDOW_RESIZED:
		{
			int width = event.window.data1;
			int height = event.window.data2;
			auto glRenderer = Renderer::getGLRenderer();
			if (glRenderer) glRenderer->resize(width, height);
		}
		break;

		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
		{
			easy2d::Scene* pCurrentScene = easy2d::SceneManager::getCurrentScene();
			if (!pCurrentScene || pCurrentScene->onCloseWindow())
			{
				easy2d::Game::quit();
			}
		}
		break;

		case SDL_EVENT_WINDOW_FOCUS_GAINED:
			// 窗口获得焦点
			break;

		case SDL_EVENT_WINDOW_FOCUS_LOST:
			// 窗口失去焦点
			break;

		case SDL_EVENT_WINDOW_MINIMIZED:
			// 窗口最小化
			break;

		case SDL_EVENT_WINDOW_RESTORED:
			// 窗口恢复
			break;
		}
	}
}

void easy2d::Window::__updateCursor()
{
	s_customCursor.update(s_currentCursor);
	if (s_customCursor)
	{
		SetSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
		return;
	}

	SDL_SystemCursor cursorType = SDL_SYSTEM_CURSOR_DEFAULT;
	switch (s_currentCursor)
	{
	case Cursor::Normal:
		cursorType = SDL_SYSTEM_CURSOR_DEFAULT;
		break;

	case Cursor::Hand:
		cursorType = SDL_SYSTEM_CURSOR_POINTER;
		break;

	case Cursor::No:
		cursorType = SDL_SYSTEM_CURSOR_NOT_ALLOWED;
		break;

	case Cursor::Wait:
		cursorType = SDL_SYSTEM_CURSOR_WAIT;
		break;

	case Cursor::ArrowWait:
		cursorType = SDL_SYSTEM_CURSOR_PROGRESS;
		break;

	default:
		cursorType = SDL_SYSTEM_CURSOR_DEFAULT;
		break;
	}

	SetSystemCursor(cursorType);
}

float easy2d::Window::getWidth()
{
	return getSize().width;
}

float easy2d::Window::getHeight()
{
	return getSize().height;
}

easy2d::Size easy2d::Window::getSize()
{
	if (s_Window)
	{
		int width, height;
		SDL_GetWindowSize(s_Window, &width, &height);
		return Size(static_cast<float>(width), static_cast<float>(height));
	}
	return Size();
}

SDL_Window* easy2d::Window::getSDLWindow()
{
	return s_Window;
}

void* easy2d::Window::getNativeWindowHandle()
{
	if (!s_Window) return nullptr;

#ifdef SDL_PLATFORM_WIN32
	return SDL_GetPointerProperty(SDL_GetWindowProperties(s_Window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
#else
	return nullptr;
#endif
}

void easy2d::Window::setSize(int width, int height)
{
	if (s_Window)
	{
		SDL_SetWindowSize(s_Window, width, height);
	}
}

void easy2d::Window::setTitle(const String& title)
{
	if (s_Window)
	{
		SDL_SetWindowTitle(s_Window, title.c_str());
	}
}

void easy2d::Window::setIcon(int iconID)
{
	// SDL2设置图标需要加载表面数据
	// 资源ID方式在SDL2中不直接支持，建议使用setIcon(const String& filePath)
	E2D_WARNING("Window::setIcon(int) is deprecated in SDL2 mode, use setIcon(const String&) instead");
}

void easy2d::Window::setIcon(const String& filePath)
{
	if (!s_Window)
	{
		E2D_ERROR("Window not created, cannot set icon");
		return;
	}

	// 使用 Path::searchForFile 搜索文件
	String actualPath = Path::searchForFile(filePath);
	if (actualPath.empty())
	{
		E2D_ERROR("Failed to find icon image: %s", filePath.c_str());
		return;
	}

	// 使用stb_image加载图片
	int width, height, channels;
	unsigned char* pixels = stbi_load(actualPath.c_str(), &width, &height, &channels, 4);

	if (!pixels)
	{
		E2D_ERROR("Failed to load icon image: %s", actualPath.c_str());
		return;
	}

	// 将RGBA数据转换为ARGB格式（SDL需要）
	unsigned char* argbPixels = new unsigned char[width * height * 4];
	for (int i = 0; i < width * height; ++i)
	{
		argbPixels[i * 4 + 0] = pixels[i * 4 + 3]; // A
		argbPixels[i * 4 + 1] = pixels[i * 4 + 0]; // R
		argbPixels[i * 4 + 2] = pixels[i * 4 + 1]; // G
		argbPixels[i * 4 + 3] = pixels[i * 4 + 2]; // B
	}

	// 创建SDL_Surface (SDL3使用SDL_CreateSurface)
	SDL_Surface* surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_ARGB8888);
	if (surface)
	{
		// 复制像素数据到surface
		SDL_memcpy(surface->pixels, argbPixels, width * height * 4);
		SDL_SetWindowIcon(s_Window, surface);
		SDL_DestroySurface(surface);
		E2D_LOG("Window icon set successfully: %s", actualPath.c_str());
	}
	else
	{
		E2D_ERROR("Failed to create SDL_Surface for icon: %s", SDL_GetError());
	}

	// 清理
	delete[] argbPixels;
	stbi_image_free(pixels);
}

void easy2d::Window::setCursor(Cursor cursor)
{
	s_currentCursor = cursor;
	__updateCursor();
}

void easy2d::Window::setCustomCursor(Node* cursor)
{
	setCustomCursor([=](Cursor) -> Node*
		{
			return cursor;
		});
}

void easy2d::Window::setCustomCursor(Function<Node* (Cursor)> cursorFunc)
{
	s_customCursor.setCursorFunc(cursorFunc);
	__updateCursor();
}

easy2d::Node* easy2d::Window::getCustomCursor()
{
	return s_customCursor.getCursorNode();
}

easy2d::String easy2d::Window::getTitle()
{
	if (s_Window)
	{
		return SDL_GetWindowTitle(s_Window);
	}
	return "";
}

void easy2d::Window::setTypewritingEnable(bool enable)
{
	// SDL3中输入法管理由SDL_StartTextInput/SDL_StopTextInput处理
	// SDL3需要传入窗口参数
	if (enable)
	{
		SDL_StartTextInput(s_Window);
	}
	else
	{
		SDL_StopTextInput(s_Window);
	}
}

void easy2d::Window::info(const String& text, const String& title)
{
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, title.c_str(), text.c_str(), s_Window);
	Game::reset();
}

void easy2d::Window::warning(const String& text, const String& title)
{
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, title.c_str(), text.c_str(), s_Window);
	Game::reset();
}

void easy2d::Window::error(const String& text, const String& title)
{
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title.c_str(), text.c_str(), s_Window);
	Game::reset();
}
