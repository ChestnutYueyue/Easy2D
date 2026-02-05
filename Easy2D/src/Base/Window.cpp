#include <easy2d/base/e2dbase.h>
#include <easy2d/manager/e2dmanager.h>
#include <easy2d/node/e2dnode.h>
#include <GLFW/glfw3.h>

// GLFW 窗口句柄
static GLFWwindow* s_Window = nullptr;
static easy2d::Window::Cursor s_currentCursor = easy2d::Window::Cursor::Normal;

// GLFW 标准光标
static GLFWcursor* s_pArrowCursor = nullptr;
static GLFWcursor* s_pHandCursor = nullptr;

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

// 窗口关闭回调
static void window_close_callback(GLFWwindow* window)
{
	easy2d::Scene* pCurrentScene = easy2d::SceneManager::getCurrentScene();
	if (!pCurrentScene || pCurrentScene->onCloseWindow())
	{
		easy2d::Game::quit();
	}
	else
	{
		// 阻止窗口关闭
		glfwSetWindowShouldClose(window, GLFW_FALSE);
	}
}

// 窗口大小变化回调
static void window_size_callback(GLFWwindow* window, int width, int height)
{
	// 如果程序接收到窗口大小变化，这个方法将调整渲染目标
	// 在 DPI-aware 模式下，使用窗口的逻辑大小即可
	// Direct2D 会自动处理 DPI 缩放
	auto pRT = easy2d::Renderer::getRenderTarget();
	if (pRT) pRT->Resize(D2D1::SizeU(static_cast<UINT32>(width), static_cast<UINT32>(height)));
}

// 窗口内容缩放回调（当DPI变化时触发）
static void window_content_scale_callback(GLFWwindow* window, float xscale, float yscale)
{
	// 当窗口移动到不同DPI的显示器时，需要重新创建设备资源
	// Direct2D 渲染目标需要根据新的 DPI 重新创建
	// 通过切换垂直同步状态来触发资源重建
	bool vsync = easy2d::Renderer::isVSyncEnabled();
	easy2d::Renderer::setVSync(!vsync);
	easy2d::Renderer::setVSync(vsync);
}

// 键盘回调
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	using easy2d::KeyCode;
	
	// 将 GLFW 键码转换为 Easy2D 键码
	KeyCode::Value vk = KeyCode::Unknown;
	switch (key)
	{
		case GLFW_KEY_SPACE: vk = KeyCode::Space; break;
		case GLFW_KEY_APOSTROPHE: vk = KeyCode::Unknown; break;
		case GLFW_KEY_COMMA: vk = KeyCode::Unknown; break;
		case GLFW_KEY_MINUS: vk = KeyCode::Unknown; break;
		case GLFW_KEY_PERIOD: vk = KeyCode::Unknown; break;
		case GLFW_KEY_SLASH: vk = KeyCode::Unknown; break;
		case GLFW_KEY_0: vk = KeyCode::Num0; break;
		case GLFW_KEY_1: vk = KeyCode::Num1; break;
		case GLFW_KEY_2: vk = KeyCode::Num2; break;
		case GLFW_KEY_3: vk = KeyCode::Num3; break;
		case GLFW_KEY_4: vk = KeyCode::Num4; break;
		case GLFW_KEY_5: vk = KeyCode::Num5; break;
		case GLFW_KEY_6: vk = KeyCode::Num6; break;
		case GLFW_KEY_7: vk = KeyCode::Num7; break;
		case GLFW_KEY_8: vk = KeyCode::Num8; break;
		case GLFW_KEY_9: vk = KeyCode::Num9; break;
		case GLFW_KEY_SEMICOLON: vk = KeyCode::Unknown; break;
		case GLFW_KEY_EQUAL: vk = KeyCode::Unknown; break;
		case GLFW_KEY_A: vk = KeyCode::A; break;
		case GLFW_KEY_B: vk = KeyCode::B; break;
		case GLFW_KEY_C: vk = KeyCode::C; break;
		case GLFW_KEY_D: vk = KeyCode::D; break;
		case GLFW_KEY_E: vk = KeyCode::E; break;
		case GLFW_KEY_F: vk = KeyCode::F; break;
		case GLFW_KEY_G: vk = KeyCode::G; break;
		case GLFW_KEY_H: vk = KeyCode::H; break;
		case GLFW_KEY_I: vk = KeyCode::I; break;
		case GLFW_KEY_J: vk = KeyCode::J; break;
		case GLFW_KEY_K: vk = KeyCode::K; break;
		case GLFW_KEY_L: vk = KeyCode::L; break;
		case GLFW_KEY_M: vk = KeyCode::M; break;
		case GLFW_KEY_N: vk = KeyCode::N; break;
		case GLFW_KEY_O: vk = KeyCode::O; break;
		case GLFW_KEY_P: vk = KeyCode::P; break;
		case GLFW_KEY_Q: vk = KeyCode::Q; break;
		case GLFW_KEY_R: vk = KeyCode::R; break;
		case GLFW_KEY_S: vk = KeyCode::S; break;
		case GLFW_KEY_T: vk = KeyCode::T; break;
		case GLFW_KEY_U: vk = KeyCode::U; break;
		case GLFW_KEY_V: vk = KeyCode::V; break;
		case GLFW_KEY_W: vk = KeyCode::W; break;
		case GLFW_KEY_X: vk = KeyCode::X; break;
		case GLFW_KEY_Y: vk = KeyCode::Y; break;
		case GLFW_KEY_Z: vk = KeyCode::Z; break;
		case GLFW_KEY_LEFT_BRACKET: vk = KeyCode::Unknown; break;
		case GLFW_KEY_BACKSLASH: vk = KeyCode::Unknown; break;
		case GLFW_KEY_RIGHT_BRACKET: vk = KeyCode::Unknown; break;
		case GLFW_KEY_GRAVE_ACCENT: vk = KeyCode::Unknown; break;
		case GLFW_KEY_ESCAPE: vk = KeyCode::Esc; break;
		case GLFW_KEY_ENTER: vk = KeyCode::Enter; break;
		case GLFW_KEY_TAB: vk = KeyCode::Tab; break;
		case GLFW_KEY_BACKSPACE: vk = KeyCode::Back; break;
		case GLFW_KEY_INSERT: vk = KeyCode::Unknown; break;
		case GLFW_KEY_DELETE: vk = KeyCode::Delete; break;
		case GLFW_KEY_RIGHT: vk = KeyCode::Right; break;
		case GLFW_KEY_LEFT: vk = KeyCode::Left; break;
		case GLFW_KEY_DOWN: vk = KeyCode::Down; break;
		case GLFW_KEY_UP: vk = KeyCode::Up; break;
		case GLFW_KEY_PAGE_UP: vk = KeyCode::Unknown; break;
		case GLFW_KEY_PAGE_DOWN: vk = KeyCode::Unknown; break;
		case GLFW_KEY_HOME: vk = KeyCode::Unknown; break;
		case GLFW_KEY_END: vk = KeyCode::Unknown; break;
		case GLFW_KEY_CAPS_LOCK: vk = KeyCode::Unknown; break;
		case GLFW_KEY_SCROLL_LOCK: vk = KeyCode::Unknown; break;
		case GLFW_KEY_NUM_LOCK: vk = KeyCode::Unknown; break;
		case GLFW_KEY_PRINT_SCREEN: vk = KeyCode::Unknown; break;
		case GLFW_KEY_PAUSE: vk = KeyCode::Unknown; break;
		case GLFW_KEY_F1: vk = KeyCode::F1; break;
		case GLFW_KEY_F2: vk = KeyCode::F2; break;
		case GLFW_KEY_F3: vk = KeyCode::F3; break;
		case GLFW_KEY_F4: vk = KeyCode::F4; break;
		case GLFW_KEY_F5: vk = KeyCode::F5; break;
		case GLFW_KEY_F6: vk = KeyCode::F6; break;
		case GLFW_KEY_F7: vk = KeyCode::F7; break;
		case GLFW_KEY_F8: vk = KeyCode::F8; break;
		case GLFW_KEY_F9: vk = KeyCode::F9; break;
		case GLFW_KEY_F10: vk = KeyCode::F10; break;
		case GLFW_KEY_F11: vk = KeyCode::F11; break;
		case GLFW_KEY_F12: vk = KeyCode::F12; break;
		case GLFW_KEY_KP_0: vk = KeyCode::Numpad0; break;
		case GLFW_KEY_KP_1: vk = KeyCode::Numpad1; break;
		case GLFW_KEY_KP_2: vk = KeyCode::Numpad2; break;
		case GLFW_KEY_KP_3: vk = KeyCode::Numpad3; break;
		case GLFW_KEY_KP_4: vk = KeyCode::Numpad4; break;
		case GLFW_KEY_KP_5: vk = KeyCode::Numpad5; break;
		case GLFW_KEY_KP_6: vk = KeyCode::Numpad6; break;
		case GLFW_KEY_KP_7: vk = KeyCode::Numpad7; break;
		case GLFW_KEY_KP_8: vk = KeyCode::Numpad8; break;
		case GLFW_KEY_KP_9: vk = KeyCode::Numpad9; break;
		case GLFW_KEY_LEFT_SHIFT: vk = KeyCode::LShift; break;
		case GLFW_KEY_LEFT_CONTROL: vk = KeyCode::LCtrl; break;
		case GLFW_KEY_LEFT_ALT: vk = KeyCode::LAlt; break;
		case GLFW_KEY_LEFT_SUPER: vk = KeyCode::Unknown; break;
		case GLFW_KEY_RIGHT_SHIFT: vk = KeyCode::RShift; break;
		case GLFW_KEY_RIGHT_CONTROL: vk = KeyCode::RCtrl; break;
		case GLFW_KEY_RIGHT_ALT: vk = KeyCode::RAlt; break;
		case GLFW_KEY_RIGHT_SUPER: vk = KeyCode::Unknown; break;
		case GLFW_KEY_MENU: vk = KeyCode::Unknown; break;
		default: vk = KeyCode::Unknown; break;
	}

	if (vk != KeyCode::Unknown)
	{
		if (action == GLFW_PRESS)
		{
			easy2d::KeyDownEvent evt(vk, scancode);
			easy2d::SceneManager::dispatch(&evt);
		}
		else if (action == GLFW_RELEASE)
		{
			easy2d::KeyUpEvent evt(vk, scancode);
			easy2d::SceneManager::dispatch(&evt);
		}
	}
}

// 鼠标按钮回调
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	easy2d::MouseCode::Value btn = easy2d::MouseCode::Left;
	switch (button)
	{
		case GLFW_MOUSE_BUTTON_LEFT: btn = easy2d::MouseCode::Left; break;
		case GLFW_MOUSE_BUTTON_RIGHT: btn = easy2d::MouseCode::Right; break;
		case GLFW_MOUSE_BUTTON_MIDDLE: btn = easy2d::MouseCode::Middle; break;
	}

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	if (action == GLFW_PRESS)
	{
		easy2d::MouseDownEvent evt(static_cast<float>(xpos), static_cast<float>(ypos), btn);
		easy2d::SceneManager::dispatch(&evt);
	}
	else if (action == GLFW_RELEASE)
	{
		easy2d::MouseUpEvent evt(static_cast<float>(xpos), static_cast<float>(ypos), btn);
		easy2d::SceneManager::dispatch(&evt);
	}
}

// 鼠标移动回调
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	easy2d::MouseMoveEvent evt(static_cast<float>(xpos), static_cast<float>(ypos));
	easy2d::SceneManager::dispatch(&evt);
}

// 鼠标滚轮回调
static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	easy2d::MouseWheelEvent evt(static_cast<float>(xpos), static_cast<float>(ypos), static_cast<float>(yoffset));
	easy2d::SceneManager::dispatch(&evt);
}

bool easy2d::Window::__init(const String& title, int nWidth, int nHeight)
{
	// 初始化 GLFW
	if (!glfwInit())
	{
		easy2d::Window::error("Failed to initialize GLFW!");
		return false;
	}

	// 设置窗口属性
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // 先不显示窗口

	// 创建窗口
	s_Window = glfwCreateWindow(nWidth, nHeight, title.c_str(), nullptr, nullptr);
	if (!s_Window)
	{
		glfwTerminate();
		easy2d::Window::error("Failed to create GLFW window!");
		return false;
	}

	// 设置回调函数
	glfwSetWindowCloseCallback(s_Window, window_close_callback);
	glfwSetWindowSizeCallback(s_Window, window_size_callback);
	glfwSetWindowContentScaleCallback(s_Window, window_content_scale_callback);
	glfwSetKeyCallback(s_Window, key_callback);
	glfwSetMouseButtonCallback(s_Window, mouse_button_callback);
	glfwSetCursorPosCallback(s_Window, cursor_position_callback);
	glfwSetScrollCallback(s_Window, scroll_callback);

	// 居中窗口
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	if (monitor)
	{
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		if (mode)
		{
			int xpos = (mode->width - nWidth) / 2;
			int ypos = (mode->height - nHeight) / 2;
			glfwSetWindowPos(s_Window, xpos, ypos);
		}
	}

	return true;
}

void easy2d::Window::__uninit()
{
	s_customCursor.clear();
	
	// 销毁标准光标
	if (s_pArrowCursor)
	{
		glfwDestroyCursor(s_pArrowCursor);
		s_pArrowCursor = nullptr;
	}
	if (s_pHandCursor)
	{
		glfwDestroyCursor(s_pHandCursor);
		s_pHandCursor = nullptr;
	}
	
	// 销毁窗口
	if (s_Window)
	{
		glfwDestroyWindow(s_Window);
		s_Window = nullptr;
	}
	
	// 终止 GLFW
	glfwTerminate();
}

void easy2d::Window::__poll()
{
	glfwPollEvents();
}

void easy2d::Window::__updateCursor()
{
	s_customCursor.update(s_currentCursor);
	if (s_customCursor)
	{
		// 使用自定义光标，隐藏系统光标
		glfwSetInputMode(s_Window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		return;
	}

	// 确保标准光标已创建
	if (!s_pArrowCursor)
	{
		s_pArrowCursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	}
	if (!s_pHandCursor)
	{
		s_pHandCursor = glfwCreateStandardCursor(GLFW_POINTING_HAND_CURSOR);
	}

	// 设置系统光标
	switch (s_currentCursor)
	{
	case Cursor::None:
		glfwSetInputMode(s_Window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		break;

	case Cursor::Hand:
		glfwSetInputMode(s_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		if (s_pHandCursor)
		{
			glfwSetCursor(s_Window, s_pHandCursor);
		}
		break;

	case Cursor::Normal:
	case Cursor::No:
	case Cursor::Wait:
	case Cursor::ArrowWait:
	default:
		glfwSetInputMode(s_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		if (s_pArrowCursor)
		{
			glfwSetCursor(s_Window, s_pArrowCursor);
		}
		break;
	}
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
		glfwGetWindowSize(s_Window, &width, &height);
		return Size(static_cast<float>(width), static_cast<float>(height));
	}
	return Size();
}

GLFWwindow* easy2d::Window::getGLFWwindow()
{
	return s_Window;
}

void easy2d::Window::setSize(int width, int height)
{
	if (s_Window)
	{
		// 获取屏幕分辨率
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		if (monitor)
		{
			const GLFWvidmode* mode = glfwGetVideoMode(monitor);
			if (mode)
			{
				// 当输入的窗口大小比分辨率大时，给出警告
				if (mode->width < width || mode->height < height)
					E2D_WARNING("The window is larger than screen!");
				
				// 取最小值
				width = std::min(width, mode->width);
				height = std::min(height, mode->height);
			}
		}
		
		// 修改窗口大小，并设置窗口在屏幕居中
		glfwSetWindowSize(s_Window, width, height);
		
		// 重新居中
		if (monitor)
		{
			const GLFWvidmode* mode = glfwGetVideoMode(monitor);
			if (mode)
			{
				int xpos = (mode->width - width) / 2;
				int ypos = (mode->height - height) / 2;
				glfwSetWindowPos(s_Window, xpos, ypos);
			}
		}
	}
}

void easy2d::Window::setTitle(const String& title)
{
	if (s_Window)
	{
		glfwSetWindowTitle(s_Window, title.c_str());
	}
}

void easy2d::Window::setIcon(int iconID)
{
	// GLFW 设置图标需要图像数据，这里暂时不实现
	// 可以通过 glfwSetWindowIcon 实现
	E2D_WARNING("Window::setIcon is not implemented for GLFW yet!");
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
		return glfwGetWindowTitle(s_Window);
	}
	return "";
}

void easy2d::Window::setTypewritingEnable(bool enable)
{
	// GLFW 不直接支持输入法控制，暂时不实现
	// 可以通过平台特定代码实现
}

void easy2d::Window::info(const String & text, const String & title)
{
	// 使用简单的控制台输出或日志
	E2D_LOG("[INFO] %s: %s", title.c_str(), text.c_str());
	Game::reset();
}

void easy2d::Window::warning(const String& title, const String& text)
{
	E2D_WARNING("[WARNING] %s: %s", title.c_str(), text.c_str());
	Game::reset();
}

void easy2d::Window::error(const String & text, const String & title)
{
	E2D_ERROR("[ERROR] %s: %s", title.c_str(), text.c_str());
	Game::reset();
}
