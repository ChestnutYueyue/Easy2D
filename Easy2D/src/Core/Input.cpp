#include <unordered_map>
#include <easy2d/e2dbase.h>
#include <easy2d/e2dmanager.h>
#include <SDL3/SDL.h>


using namespace easy2d;

namespace
{
	// 键盘状态缓冲区
	Uint8 s_KeyState[SDL_SCANCODE_COUNT] = { 0 };
	Uint8 s_KeyRecordState[SDL_SCANCODE_COUNT] = { 0 };

	// 鼠标状态
	Uint32 s_MouseState = 0;
	Uint32 s_MouseRecordState = 0;
	int s_MouseX = 0;
	int s_MouseY = 0;
	int s_MouseDeltaX = 0;
	int s_MouseDeltaY = 0;
	float s_MouseWheel = 0.0f;

	// SDL键码到Easy2D键码的映射 (SDL3使用大写字母键名)
	const std::unordered_map<SDL_Keycode, KeyCode::Value> s_SDLToKeyCode = {
		{ SDLK_UNKNOWN, KeyCode::Unknown },
		{ SDLK_UP, KeyCode::Up },
		{ SDLK_DOWN, KeyCode::Down },
		{ SDLK_LEFT, KeyCode::Left },
		{ SDLK_RIGHT, KeyCode::Right },
		{ SDLK_RETURN, KeyCode::Enter },
		{ SDLK_SPACE, KeyCode::Space },
		{ SDLK_ESCAPE, KeyCode::Esc },
		{ SDLK_LCTRL, KeyCode::LCtrl },
		{ SDLK_RCTRL, KeyCode::RCtrl },
		{ SDLK_LSHIFT, KeyCode::LShift },
		{ SDLK_RSHIFT, KeyCode::RShift },
		{ SDLK_LALT, KeyCode::LAlt },
		{ SDLK_RALT, KeyCode::RAlt },
		{ SDLK_TAB, KeyCode::Tab },
		{ SDLK_DELETE, KeyCode::Delete },
		{ SDLK_BACKSPACE, KeyCode::Back },

		{ SDLK_A, KeyCode::A },
		{ SDLK_B, KeyCode::B },
		{ SDLK_C, KeyCode::C },
		{ SDLK_D, KeyCode::D },
		{ SDLK_E, KeyCode::E },
		{ SDLK_F, KeyCode::F },
		{ SDLK_G, KeyCode::G },
		{ SDLK_H, KeyCode::H },
		{ SDLK_I, KeyCode::I },
		{ SDLK_J, KeyCode::J },
		{ SDLK_K, KeyCode::K },
		{ SDLK_L, KeyCode::L },
		{ SDLK_M, KeyCode::M },
		{ SDLK_N, KeyCode::N },
		{ SDLK_O, KeyCode::O },
		{ SDLK_P, KeyCode::P },
		{ SDLK_Q, KeyCode::Q },
		{ SDLK_R, KeyCode::R },
		{ SDLK_S, KeyCode::S },
		{ SDLK_T, KeyCode::T },
		{ SDLK_U, KeyCode::U },
		{ SDLK_V, KeyCode::V },
		{ SDLK_W, KeyCode::W },
		{ SDLK_X, KeyCode::X },
		{ SDLK_Y, KeyCode::Y },
		{ SDLK_Z, KeyCode::Z },

		{ SDLK_0, KeyCode::Num0 },
		{ SDLK_1, KeyCode::Num1 },
		{ SDLK_2, KeyCode::Num2 },
		{ SDLK_3, KeyCode::Num3 },
		{ SDLK_4, KeyCode::Num4 },
		{ SDLK_5, KeyCode::Num5 },
		{ SDLK_6, KeyCode::Num6 },
		{ SDLK_7, KeyCode::Num7 },
		{ SDLK_8, KeyCode::Num8 },
		{ SDLK_9, KeyCode::Num9 },

		{ SDLK_KP_0, KeyCode::Numpad0 },
		{ SDLK_KP_1, KeyCode::Numpad1 },
		{ SDLK_KP_2, KeyCode::Numpad2 },
		{ SDLK_KP_3, KeyCode::Numpad3 },
		{ SDLK_KP_4, KeyCode::Numpad4 },
		{ SDLK_KP_5, KeyCode::Numpad5 },
		{ SDLK_KP_6, KeyCode::Numpad6 },
		{ SDLK_KP_7, KeyCode::Numpad7 },
		{ SDLK_KP_8, KeyCode::Numpad8 },
		{ SDLK_KP_9, KeyCode::Numpad9 },

		{ SDLK_F1, KeyCode::F1 },
		{ SDLK_F2, KeyCode::F2 },
		{ SDLK_F3, KeyCode::F3 },
		{ SDLK_F4, KeyCode::F4 },
		{ SDLK_F5, KeyCode::F5 },
		{ SDLK_F6, KeyCode::F6 },
		{ SDLK_F7, KeyCode::F7 },
		{ SDLK_F8, KeyCode::F8 },
		{ SDLK_F9, KeyCode::F9 },
		{ SDLK_F10, KeyCode::F10 },
		{ SDLK_F11, KeyCode::F11 },
		{ SDLK_F12, KeyCode::F12 },
	};

	// Easy2D键码到SDL扫描码的映射
	const std::unordered_map<KeyCode::Value, std::vector<SDL_Scancode>> s_KeyCodeToSDL = {
		{ KeyCode::Unknown, { SDL_SCANCODE_UNKNOWN } },
		{ KeyCode::Up, { SDL_SCANCODE_UP } },
		{ KeyCode::Left, { SDL_SCANCODE_LEFT } },
		{ KeyCode::Right, { SDL_SCANCODE_RIGHT } },
		{ KeyCode::Down, { SDL_SCANCODE_DOWN } },
		{ KeyCode::Enter, { SDL_SCANCODE_RETURN } },
		{ KeyCode::Space, { SDL_SCANCODE_SPACE } },
		{ KeyCode::Esc, { SDL_SCANCODE_ESCAPE } },
		{ KeyCode::Ctrl, { SDL_SCANCODE_LCTRL, SDL_SCANCODE_RCTRL } },
		{ KeyCode::LCtrl, { SDL_SCANCODE_LCTRL } },
		{ KeyCode::RCtrl, { SDL_SCANCODE_RCTRL } },
		{ KeyCode::Shift, { SDL_SCANCODE_LSHIFT, SDL_SCANCODE_RSHIFT } },
		{ KeyCode::LShift, { SDL_SCANCODE_LSHIFT } },
		{ KeyCode::RShift, { SDL_SCANCODE_RSHIFT } },
		{ KeyCode::Alt, { SDL_SCANCODE_LALT, SDL_SCANCODE_RALT } },
		{ KeyCode::LAlt, { SDL_SCANCODE_LALT } },
		{ KeyCode::RAlt, { SDL_SCANCODE_RALT } },
		{ KeyCode::Tab, { SDL_SCANCODE_TAB } },
		{ KeyCode::Delete, { SDL_SCANCODE_DELETE } },
		{ KeyCode::Back, { SDL_SCANCODE_BACKSPACE } },

		{ KeyCode::A, { SDL_SCANCODE_A } },
		{ KeyCode::B, { SDL_SCANCODE_B } },
		{ KeyCode::C, { SDL_SCANCODE_C } },
		{ KeyCode::D, { SDL_SCANCODE_D } },
		{ KeyCode::E, { SDL_SCANCODE_E } },
		{ KeyCode::F, { SDL_SCANCODE_F } },
		{ KeyCode::G, { SDL_SCANCODE_G } },
		{ KeyCode::H, { SDL_SCANCODE_H } },
		{ KeyCode::I, { SDL_SCANCODE_I } },
		{ KeyCode::J, { SDL_SCANCODE_J } },
		{ KeyCode::K, { SDL_SCANCODE_K } },
		{ KeyCode::L, { SDL_SCANCODE_L } },
		{ KeyCode::M, { SDL_SCANCODE_M } },
		{ KeyCode::N, { SDL_SCANCODE_N } },
		{ KeyCode::O, { SDL_SCANCODE_O } },
		{ KeyCode::P, { SDL_SCANCODE_P } },
		{ KeyCode::Q, { SDL_SCANCODE_Q } },
		{ KeyCode::R, { SDL_SCANCODE_R } },
		{ KeyCode::S, { SDL_SCANCODE_S } },
		{ KeyCode::T, { SDL_SCANCODE_T } },
		{ KeyCode::U, { SDL_SCANCODE_U } },
		{ KeyCode::V, { SDL_SCANCODE_V } },
		{ KeyCode::W, { SDL_SCANCODE_W } },
		{ KeyCode::X, { SDL_SCANCODE_X } },
		{ KeyCode::Y, { SDL_SCANCODE_Y } },
		{ KeyCode::Z, { SDL_SCANCODE_Z } },

		{ KeyCode::Num0, { SDL_SCANCODE_0 } },
		{ KeyCode::Num1, { SDL_SCANCODE_1 } },
		{ KeyCode::Num2, { SDL_SCANCODE_2 } },
		{ KeyCode::Num3, { SDL_SCANCODE_3 } },
		{ KeyCode::Num4, { SDL_SCANCODE_4 } },
		{ KeyCode::Num5, { SDL_SCANCODE_5 } },
		{ KeyCode::Num6, { SDL_SCANCODE_6 } },
		{ KeyCode::Num7, { SDL_SCANCODE_7 } },
		{ KeyCode::Num8, { SDL_SCANCODE_8 } },
		{ KeyCode::Num9, { SDL_SCANCODE_9 } },

		{ KeyCode::Numpad0, { SDL_SCANCODE_KP_0 } },
		{ KeyCode::Numpad1, { SDL_SCANCODE_KP_1 } },
		{ KeyCode::Numpad2, { SDL_SCANCODE_KP_2 } },
		{ KeyCode::Numpad3, { SDL_SCANCODE_KP_3 } },
		{ KeyCode::Numpad4, { SDL_SCANCODE_KP_4 } },
		{ KeyCode::Numpad5, { SDL_SCANCODE_KP_5 } },
		{ KeyCode::Numpad6, { SDL_SCANCODE_KP_6 } },
		{ KeyCode::Numpad7, { SDL_SCANCODE_KP_7 } },
		{ KeyCode::Numpad8, { SDL_SCANCODE_KP_8 } },
		{ KeyCode::Numpad9, { SDL_SCANCODE_KP_9 } },

		{ KeyCode::F1, { SDL_SCANCODE_F1 } },
		{ KeyCode::F2, { SDL_SCANCODE_F2 } },
		{ KeyCode::F3, { SDL_SCANCODE_F3 } },
		{ KeyCode::F4, { SDL_SCANCODE_F4 } },
		{ KeyCode::F5, { SDL_SCANCODE_F5 } },
		{ KeyCode::F6, { SDL_SCANCODE_F6 } },
		{ KeyCode::F7, { SDL_SCANCODE_F7 } },
		{ KeyCode::F8, { SDL_SCANCODE_F8 } },
		{ KeyCode::F9, { SDL_SCANCODE_F9 } },
		{ KeyCode::F10, { SDL_SCANCODE_F10 } },
		{ KeyCode::F11, { SDL_SCANCODE_F11 } },
		{ KeyCode::F12, { SDL_SCANCODE_F12 } },
	};
}

bool Input::__init()
{
	// 初始化SDL输入子系统（视频子系统已在Window中初始化）
	// SDL输入处理集成在事件循环中，无需额外初始化
	memset(s_KeyState, 0, sizeof(s_KeyState));
	memset(s_KeyRecordState, 0, sizeof(s_KeyRecordState));
	s_MouseState = 0;
	s_MouseRecordState = 0;
	s_MouseX = 0;
	s_MouseY = 0;
	s_MouseDeltaX = 0;
	s_MouseDeltaY = 0;
	s_MouseWheel = 0;

	return true;
}

void Input::__uninit()
{
	// SDL输入无需显式卸载
}

void easy2d::Input::__update()
{
	// 保存上一帧的键盘状态
	memcpy(s_KeyRecordState, s_KeyState, sizeof(s_KeyState));

	// 获取当前键盘状态
	const bool* currentKeyState = SDL_GetKeyboardState(nullptr);
	for (int i = 0; i < SDL_SCANCODE_COUNT; ++i)
	{
		s_KeyState[i] = currentKeyState[i] ? 1 : 0;
	}

	// 保存上一帧的鼠标状态
	s_MouseRecordState = s_MouseState;

	// 获取当前鼠标状态 (SDL3使用浮点数坐标)
	float mouseX, mouseY;
	s_MouseState = SDL_GetMouseState(&mouseX, &mouseY);
	s_MouseX = static_cast<int>(mouseX);
	s_MouseY = static_cast<int>(mouseY);

	// 获取相对鼠标移动 (SDL3使用浮点数坐标)
	float deltaX, deltaY;
	SDL_GetRelativeMouseState(&deltaX, &deltaY);
	s_MouseDeltaX = static_cast<int>(deltaX);
	s_MouseDeltaY = static_cast<int>(deltaY);
}

void easy2d::Input::__onMouseWheel(float delta)
{
	s_MouseWheel += delta;
}

bool Input::isDown(KeyCode::Value key)
{
	auto it = s_KeyCodeToSDL.find(key);
	if (it == s_KeyCodeToSDL.end())
		return false;

	for (auto scancode : it->second)
	{
		if (s_KeyState[scancode])
			return true;
	}
	return false;
}

bool Input::isPressed(KeyCode::Value key)
{
	auto it = s_KeyCodeToSDL.find(key);
	if (it == s_KeyCodeToSDL.end())
		return false;

	for (auto scancode : it->second)
	{
		if (s_KeyState[scancode] && !s_KeyRecordState[scancode])
			return true;
	}
	return false;
}

bool Input::isReleased(KeyCode::Value key)
{
	auto it = s_KeyCodeToSDL.find(key);
	if (it == s_KeyCodeToSDL.end())
		return false;

	for (auto scancode : it->second)
	{
		if (!s_KeyState[scancode] && s_KeyRecordState[scancode])
			return true;
	}
	return false;
}

bool easy2d::Input::isDown(MouseCode::Value code)
{
	Uint32 buttonMask = 0;
	switch (code)
	{
	case MouseCode::Left:
		buttonMask = SDL_BUTTON_LMASK;
		break;
	case MouseCode::Right:
		buttonMask = SDL_BUTTON_RMASK;
		break;
	case MouseCode::Middle:
		buttonMask = SDL_BUTTON_MMASK;
		break;
	default:
		return false;
	}
	return (s_MouseState & buttonMask) != 0;
}

bool easy2d::Input::isPressed(MouseCode::Value code)
{
	Uint32 buttonMask = 0;
	switch (code)
	{
	case MouseCode::Left:
		buttonMask = SDL_BUTTON_LMASK;
		break;
	case MouseCode::Right:
		buttonMask = SDL_BUTTON_RMASK;
		break;
	case MouseCode::Middle:
		buttonMask = SDL_BUTTON_MMASK;
		break;
	default:
		return false;
	}
	return ((s_MouseState & buttonMask) != 0) && ((s_MouseRecordState & buttonMask) == 0);
}

bool easy2d::Input::isReleased(MouseCode::Value code)
{
	Uint32 buttonMask = 0;
	switch (code)
	{
	case MouseCode::Left:
		buttonMask = SDL_BUTTON_LMASK;
		break;
	case MouseCode::Right:
		buttonMask = SDL_BUTTON_RMASK;
		break;
	case MouseCode::Middle:
		buttonMask = SDL_BUTTON_MMASK;
		break;
	default:
		return false;
	}
	return ((s_MouseState & buttonMask) == 0) && ((s_MouseRecordState & buttonMask) != 0);
}

float Input::getMouseX()
{
	return static_cast<float>(s_MouseX);
}

float Input::getMouseY()
{
	return static_cast<float>(s_MouseY);
}

Point Input::getMousePos()
{
	return Point(static_cast<float>(s_MouseX), static_cast<float>(s_MouseY));
}

float Input::getMouseDeltaX()
{
	return static_cast<float>(s_MouseDeltaX);
}

float Input::getMouseDeltaY()
{
	return static_cast<float>(s_MouseDeltaY);
}

float Input::getMouseDeltaZ()
{
	float delta = s_MouseWheel;
	s_MouseWheel = 0.0f;
	return delta;
}
