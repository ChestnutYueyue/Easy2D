#include <easy2d/base/e2dbase.h>
#include <easy2d/manager/e2dmanager.h>
#include <unordered_map>
#include <GLFW/glfw3.h>

using namespace easy2d;

namespace {
// 键盘状态缓冲区
bool s_KeyBuffer[GLFW_KEY_LAST + 1] = {false};
bool s_KeyRecordBuffer[GLFW_KEY_LAST + 1] = {false};

// 鼠标状态
bool s_MouseButtonBuffer[GLFW_MOUSE_BUTTON_LAST + 1] = {false};
bool s_MouseButtonRecordBuffer[GLFW_MOUSE_BUTTON_LAST + 1] = {false};
double s_MouseX = 0.0;
double s_MouseY = 0.0;
double s_LastMouseX = 0.0;
double s_LastMouseY = 0.0;
double s_MouseDeltaX = 0.0;
double s_MouseDeltaY = 0.0;
double s_MouseWheelDelta = 0.0;

// Easy2D 键码到 GLFW 键码的映射
const std::unordered_map<int, int> s_KeyToGLFW = {
    {KeyCode::Unknown, GLFW_KEY_UNKNOWN},
    {KeyCode::Up, GLFW_KEY_UP},
    {KeyCode::Left, GLFW_KEY_LEFT},
    {KeyCode::Right, GLFW_KEY_RIGHT},
    {KeyCode::Down, GLFW_KEY_DOWN},
    {KeyCode::Enter, GLFW_KEY_ENTER},
    {KeyCode::Space, GLFW_KEY_SPACE},
    {KeyCode::Esc, GLFW_KEY_ESCAPE},
    {KeyCode::LCtrl, GLFW_KEY_LEFT_CONTROL},
    {KeyCode::RCtrl, GLFW_KEY_RIGHT_CONTROL},
    {KeyCode::LShift, GLFW_KEY_LEFT_SHIFT},
    {KeyCode::RShift, GLFW_KEY_RIGHT_SHIFT},
    {KeyCode::LAlt, GLFW_KEY_LEFT_ALT},
    {KeyCode::RAlt, GLFW_KEY_RIGHT_ALT},
    {KeyCode::Tab, GLFW_KEY_TAB},
    {KeyCode::Delete, GLFW_KEY_DELETE},
    {KeyCode::Back, GLFW_KEY_BACKSPACE},

    {KeyCode::A, GLFW_KEY_A},
    {KeyCode::B, GLFW_KEY_B},
    {KeyCode::C, GLFW_KEY_C},
    {KeyCode::D, GLFW_KEY_D},
    {KeyCode::E, GLFW_KEY_E},
    {KeyCode::F, GLFW_KEY_F},
    {KeyCode::G, GLFW_KEY_G},
    {KeyCode::H, GLFW_KEY_H},
    {KeyCode::I, GLFW_KEY_I},
    {KeyCode::J, GLFW_KEY_J},
    {KeyCode::K, GLFW_KEY_K},
    {KeyCode::L, GLFW_KEY_L},
    {KeyCode::M, GLFW_KEY_M},
    {KeyCode::N, GLFW_KEY_N},
    {KeyCode::O, GLFW_KEY_O},
    {KeyCode::P, GLFW_KEY_P},
    {KeyCode::Q, GLFW_KEY_Q},
    {KeyCode::R, GLFW_KEY_R},
    {KeyCode::S, GLFW_KEY_S},
    {KeyCode::T, GLFW_KEY_T},
    {KeyCode::U, GLFW_KEY_U},
    {KeyCode::V, GLFW_KEY_V},
    {KeyCode::W, GLFW_KEY_W},
    {KeyCode::X, GLFW_KEY_X},
    {KeyCode::Y, GLFW_KEY_Y},
    {KeyCode::Z, GLFW_KEY_Z},

    {KeyCode::Num0, GLFW_KEY_0},
    {KeyCode::Num1, GLFW_KEY_1},
    {KeyCode::Num2, GLFW_KEY_2},
    {KeyCode::Num3, GLFW_KEY_3},
    {KeyCode::Num4, GLFW_KEY_4},
    {KeyCode::Num5, GLFW_KEY_5},
    {KeyCode::Num6, GLFW_KEY_6},
    {KeyCode::Num7, GLFW_KEY_7},
    {KeyCode::Num8, GLFW_KEY_8},
    {KeyCode::Num9, GLFW_KEY_9},

    {KeyCode::Numpad0, GLFW_KEY_KP_0},
    {KeyCode::Numpad1, GLFW_KEY_KP_1},
    {KeyCode::Numpad2, GLFW_KEY_KP_2},
    {KeyCode::Numpad3, GLFW_KEY_KP_3},
    {KeyCode::Numpad4, GLFW_KEY_KP_4},
    {KeyCode::Numpad5, GLFW_KEY_KP_5},
    {KeyCode::Numpad6, GLFW_KEY_KP_6},
    {KeyCode::Numpad7, GLFW_KEY_KP_7},
    {KeyCode::Numpad8, GLFW_KEY_KP_8},
    {KeyCode::Numpad9, GLFW_KEY_KP_9},

    {KeyCode::F1, GLFW_KEY_F1},
    {KeyCode::F2, GLFW_KEY_F2},
    {KeyCode::F3, GLFW_KEY_F3},
    {KeyCode::F4, GLFW_KEY_F4},
    {KeyCode::F5, GLFW_KEY_F5},
    {KeyCode::F6, GLFW_KEY_F6},
    {KeyCode::F7, GLFW_KEY_F7},
    {KeyCode::F8, GLFW_KEY_F8},
    {KeyCode::F9, GLFW_KEY_F9},
    {KeyCode::F10, GLFW_KEY_F10},
    {KeyCode::F11, GLFW_KEY_F11},
    {KeyCode::F12, GLFW_KEY_F12},
};

// 获取 GLFW 窗口
GLFWwindow* getWindow() {
    return Window::getGLFWwindow();
}

// 获取 GLFW 键码
int getGLFWKey(KeyCode::Value key) {
    auto it = s_KeyToGLFW.find(key);
    if (it != s_KeyToGLFW.end()) {
        return it->second;
    }
    return GLFW_KEY_UNKNOWN;
}
} // namespace

bool Input::__init() {
    // 初始化缓冲区
    memset(s_KeyBuffer, 0, sizeof(s_KeyBuffer));
    memset(s_KeyRecordBuffer, 0, sizeof(s_KeyRecordBuffer));
    memset(s_MouseButtonBuffer, 0, sizeof(s_MouseButtonBuffer));
    memset(s_MouseButtonRecordBuffer, 0, sizeof(s_MouseButtonRecordBuffer));
    
    s_MouseX = s_MouseY = 0.0;
    s_LastMouseX = s_LastMouseY = 0.0;
    s_MouseDeltaX = s_MouseDeltaY = 0.0;
    s_MouseWheelDelta = 0.0;

    GLFWwindow* window = getWindow();
    if (!window) {
        E2D_WARNING("Window not available for input!");
        return false;
    }

    return true;
}

void Input::__uninit() {
    // GLFW 输入不需要显式卸载
}

void easy2d::Input::__update() {
    GLFWwindow* window = getWindow();
    if (!window) return;

    // 保存上一帧的键盘状态
    memcpy(s_KeyRecordBuffer, s_KeyBuffer, sizeof(s_KeyBuffer));
    
    // 获取当前键盘状态
    for (const auto& pair : s_KeyToGLFW) {
        if (pair.second != GLFW_KEY_UNKNOWN) {
            s_KeyBuffer[pair.second] = (glfwGetKey(window, pair.second) == GLFW_PRESS);
        }
    }

    // 保存上一帧的鼠标状态
    memcpy(s_MouseButtonRecordBuffer, s_MouseButtonBuffer, sizeof(s_MouseButtonBuffer));
    
    // 获取当前鼠标按钮状态
    s_MouseButtonBuffer[GLFW_MOUSE_BUTTON_LEFT] = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    s_MouseButtonBuffer[GLFW_MOUSE_BUTTON_RIGHT] = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
    s_MouseButtonBuffer[GLFW_MOUSE_BUTTON_MIDDLE] = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS);

    // 保存上一帧鼠标位置
    s_LastMouseX = s_MouseX;
    s_LastMouseY = s_MouseY;
    
    // 获取当前鼠标位置
    glfwGetCursorPos(window, &s_MouseX, &s_MouseY);
    
    // 计算鼠标增量
    s_MouseDeltaX = s_MouseX - s_LastMouseX;
    s_MouseDeltaY = s_MouseY - s_LastMouseY;
}

bool Input::isDown(KeyCode::Value key) {
    // 特殊处理组合键
    if (key == KeyCode::Ctrl) {
        return s_KeyBuffer[GLFW_KEY_LEFT_CONTROL] || s_KeyBuffer[GLFW_KEY_RIGHT_CONTROL];
    }
    if (key == KeyCode::Shift) {
        return s_KeyBuffer[GLFW_KEY_LEFT_SHIFT] || s_KeyBuffer[GLFW_KEY_RIGHT_SHIFT];
    }
    if (key == KeyCode::Alt) {
        return s_KeyBuffer[GLFW_KEY_LEFT_ALT] || s_KeyBuffer[GLFW_KEY_RIGHT_ALT];
    }

    int glfwKey = getGLFWKey(key);
    if (glfwKey != GLFW_KEY_UNKNOWN && glfwKey <= GLFW_KEY_LAST) {
        return s_KeyBuffer[glfwKey];
    }
    return false;
}

bool Input::isPressed(KeyCode::Value key) {
    // 特殊处理组合键
    if (key == KeyCode::Ctrl) {
        bool curr = s_KeyBuffer[GLFW_KEY_LEFT_CONTROL] || s_KeyBuffer[GLFW_KEY_RIGHT_CONTROL];
        bool prev = s_KeyRecordBuffer[GLFW_KEY_LEFT_CONTROL] || s_KeyRecordBuffer[GLFW_KEY_RIGHT_CONTROL];
        return curr && !prev;
    }
    if (key == KeyCode::Shift) {
        bool curr = s_KeyBuffer[GLFW_KEY_LEFT_SHIFT] || s_KeyBuffer[GLFW_KEY_RIGHT_SHIFT];
        bool prev = s_KeyRecordBuffer[GLFW_KEY_LEFT_SHIFT] || s_KeyRecordBuffer[GLFW_KEY_RIGHT_SHIFT];
        return curr && !prev;
    }
    if (key == KeyCode::Alt) {
        bool curr = s_KeyBuffer[GLFW_KEY_LEFT_ALT] || s_KeyBuffer[GLFW_KEY_RIGHT_ALT];
        bool prev = s_KeyRecordBuffer[GLFW_KEY_LEFT_ALT] || s_KeyRecordBuffer[GLFW_KEY_RIGHT_ALT];
        return curr && !prev;
    }

    int glfwKey = getGLFWKey(key);
    if (glfwKey != GLFW_KEY_UNKNOWN && glfwKey <= GLFW_KEY_LAST) {
        return s_KeyBuffer[glfwKey] && !s_KeyRecordBuffer[glfwKey];
    }
    return false;
}

bool Input::isReleased(KeyCode::Value key) {
    // 特殊处理组合键
    if (key == KeyCode::Ctrl) {
        bool curr = s_KeyBuffer[GLFW_KEY_LEFT_CONTROL] || s_KeyBuffer[GLFW_KEY_RIGHT_CONTROL];
        bool prev = s_KeyRecordBuffer[GLFW_KEY_LEFT_CONTROL] || s_KeyRecordBuffer[GLFW_KEY_RIGHT_CONTROL];
        return !curr && prev;
    }
    if (key == KeyCode::Shift) {
        bool curr = s_KeyBuffer[GLFW_KEY_LEFT_SHIFT] || s_KeyBuffer[GLFW_KEY_RIGHT_SHIFT];
        bool prev = s_KeyRecordBuffer[GLFW_KEY_LEFT_SHIFT] || s_KeyRecordBuffer[GLFW_KEY_RIGHT_SHIFT];
        return !curr && prev;
    }
    if (key == KeyCode::Alt) {
        bool curr = s_KeyBuffer[GLFW_KEY_LEFT_ALT] || s_KeyBuffer[GLFW_KEY_RIGHT_ALT];
        bool prev = s_KeyRecordBuffer[GLFW_KEY_LEFT_ALT] || s_KeyRecordBuffer[GLFW_KEY_RIGHT_ALT];
        return !curr && prev;
    }

    int glfwKey = getGLFWKey(key);
    if (glfwKey != GLFW_KEY_UNKNOWN && glfwKey <= GLFW_KEY_LAST) {
        return !s_KeyBuffer[glfwKey] && s_KeyRecordBuffer[glfwKey];
    }
    return false;
}

bool easy2d::Input::isDown(MouseCode::Value code) {
    int button = GLFW_MOUSE_BUTTON_LEFT;
    switch (code) {
        case MouseCode::Left: button = GLFW_MOUSE_BUTTON_LEFT; break;
        case MouseCode::Right: button = GLFW_MOUSE_BUTTON_RIGHT; break;
        case MouseCode::Middle: button = GLFW_MOUSE_BUTTON_MIDDLE; break;
    }
    return s_MouseButtonBuffer[button];
}

bool easy2d::Input::isPressed(MouseCode::Value code) {
    int button = GLFW_MOUSE_BUTTON_LEFT;
    switch (code) {
        case MouseCode::Left: button = GLFW_MOUSE_BUTTON_LEFT; break;
        case MouseCode::Right: button = GLFW_MOUSE_BUTTON_RIGHT; break;
        case MouseCode::Middle: button = GLFW_MOUSE_BUTTON_MIDDLE; break;
    }
    return s_MouseButtonBuffer[button] && !s_MouseButtonRecordBuffer[button];
}

bool easy2d::Input::isReleased(MouseCode::Value code) {
    int button = GLFW_MOUSE_BUTTON_LEFT;
    switch (code) {
        case MouseCode::Left: button = GLFW_MOUSE_BUTTON_LEFT; break;
        case MouseCode::Right: button = GLFW_MOUSE_BUTTON_RIGHT; break;
        case MouseCode::Middle: button = GLFW_MOUSE_BUTTON_MIDDLE; break;
    }
    return !s_MouseButtonBuffer[button] && s_MouseButtonRecordBuffer[button];
}

float Input::getMouseX() { return (float)s_MouseX; }

float Input::getMouseY() { return (float)s_MouseY; }

Point Input::getMousePos() {
    return Point((float)s_MouseX, (float)s_MouseY);
}

float Input::getMouseDeltaX() { return (float)s_MouseDeltaX; }

float Input::getMouseDeltaY() { return (float)s_MouseDeltaY; }

float Input::getMouseDeltaZ() { return (float)s_MouseWheelDelta; }
