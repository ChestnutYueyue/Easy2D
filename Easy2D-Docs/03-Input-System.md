# Easy2D 输入系统 API 文档

## 1. 输入管理器 (InputManager)

### 1.1 获取输入管理器
```cpp
auto& input = Application::instance().input();
```

---

## 2. 键盘输入

### 2.1 按键检测方式
```cpp
// 方式1: 检测按键是否被按下（只触发一次）
// 适用于：菜单选择、触发动作
if (input.isKeyPressed(Key::Space)) {
    // 空格键被按下（只触发一次）
    fireBullet();
}

// 方式2: 检测按键是否持续按住
// 适用于：持续移动、加速
if (input.isKeyDown(Key::W) || input.isKeyDown(Key::Up)) {
    // W键或上方向键被持续按住
    player->moveUp(dt);
}

// 方式3: 检测按键是否释放
if (input.isKeyReleased(Key::Escape)) {
    // ESC键被释放
}
```

### 2.2 常用按键
```cpp
// 方向键
Key::Up      // 上
Key::Down    // 下
Key::Left    // 左
Key::Right   // 右

// WASD
Key::W       // W
Key::A       // A
Key::S       // S
Key::D       // D

// 功能键
Key::Escape  // ESC
Key::Space   // 空格
Key::Enter   // 回车
Key::Tab     // Tab

// 数字键
Key::Num0    // 0
Key::Num1    // 1
// ...
Key::Num9    // 9

// 字母键
Key::A       // A
Key::B       // B
// ...
Key::Z       // Z

// 控制键
Key::LShift  // 左Shift
Key::RShift  // 右Shift
Key::LCtrl   // 左Ctrl
Key::RCtrl   // 右Ctrl
Key::LAlt    // 左Alt
Key::RAlt    // 右Alt
```

### 2.3 键盘输入示例
```cpp
void onUpdate(float dt) override {
    auto& input = Application::instance().input();
    
    // 持续移动（按住）
    bool isMoving = false;
    
    if (input.isKeyDown(Key::W) || input.isKeyDown(Key::Up)) {
        player->move(Direction::Up, dt);
        isMoving = true;
    } else if (input.isKeyDown(Key::S) || input.isKeyDown(Key::Down)) {
        player->move(Direction::Down, dt);
        isMoving = true;
    } else if (input.isKeyDown(Key::A) || input.isKeyDown(Key::Left)) {
        player->move(Direction::Left, dt);
        isMoving = true;
    } else if (input.isKeyDown(Key::D) || input.isKeyDown(Key::Right)) {
        player->move(Direction::Right, dt);
        isMoving = true;
    }
    
    // 如果没有移动，停止动画
    if (!isMoving) {
        player->stop();
    }
    
    // 触发动作（按下）
    if (input.isKeyPressed(Key::Space)) {
        player->attack();
    }
    
    // 菜单操作（按下）
    if (input.isKeyPressed(Key::Escape)) {
        openMenu();
    }
    
    // 重新开始（按下）
    if (input.isKeyPressed(Key::Enter)) {
        restartGame();
    }
}
```

---

## 3. 鼠标输入

### 3.1 鼠标位置
```cpp
// 获取鼠标位置（屏幕坐标）
Vec2 mousePos = input.getMousePosition();
float mouseX = mousePos.x;
float mouseY = mousePos.y;
```

### 3.2 鼠标按键检测
```cpp
// 检测鼠标按键是否被按下（只触发一次）
if (input.isMousePressed(MouseButton::Left)) {
    // 鼠标左键被按下
    handleClick(input.getMousePosition());
}

// 检测鼠标按键是否持续按住
if (input.isMouseDown(MouseButton::Left)) {
    // 鼠标左键被持续按住
    handleDrag(input.getMousePosition());
}

// 检测鼠标按键是否释放
if (input.isMouseReleased(MouseButton::Left)) {
    // 鼠标左键被释放
    handleRelease();
}
```

### 3.3 鼠标按键类型
```cpp
MouseButton::Left    // 左键
MouseButton::Right   // 右键
MouseButton::Middle  // 中键
```

### 3.4 鼠标输入示例
```cpp
void onUpdate(float dt) override {
    auto& input = Application::instance().input();
    
    // 检测鼠标左键点击
    if (input.isMousePressed(MouseButton::Left)) {
        auto pos = input.getMousePosition();
        createExplosion(pos);
    }
    
    // 检测鼠标右键
    if (input.isMousePressed(MouseButton::Right)) {
        openContextMenu(input.getMousePosition());
    }
}
```

---

## 4. 输入处理最佳实践

### 4.1 移动控制模式对比
```cpp
// 模式1: 按下触发一次（适合回合制、菜单）
if (input.isKeyPressed(Key::Right)) {
    player->moveOneStepRight();  // 只移动一步
}

// 模式2: 按住持续触发（适合实时动作游戏）
if (input.isKeyDown(Key::Right)) {
    player->moveRight(dt);  // 持续移动
}
```

### 4.2 组合键检测
```cpp
// Shift + 方向键 = 冲刺
if (input.isKeyDown(Key::LShift) || input.isKeyDown(Key::RShift)) {
    if (input.isKeyDown(Key::Left)) {
        player->dashLeft(dt);
    } else if (input.isKeyDown(Key::Right)) {
        player->dashRight(dt);
    }
}
```

### 4.3 输入状态管理
```cpp
class GameScene : public Scene {
public:
    void onUpdate(float dt) override {
        handleKeyboardInput(dt);
        handleMouseInput();
    }
    
private:
    void handleKeyboardInput(float dt) {
        auto& input = Application::instance().input();
        
        // 移动控制
        Direction dir = Direction::None;
        
        if (input.isKeyDown(Key::W) || input.isKeyDown(Key::Up)) {
            dir = Direction::Up;
        } else if (input.isKeyDown(Key::S) || input.isKeyDown(Key::Down)) {
            dir = Direction::Down;
        } else if (input.isKeyDown(Key::A) || input.isKeyDown(Key::Left)) {
            dir = Direction::Left;
        } else if (input.isKeyDown(Key::D) || input.isKeyDown(Key::Right)) {
            dir = Direction::Right;
        }
        
        if (dir != Direction::None) {
            player_->move(dir, dt);
        } else {
            player_->stop();
        }
        
        // 动作控制
        if (input.isKeyPressed(Key::Space)) {
            player_->jump();
        }
        
        if (input.isKeyPressed(Key::Escape)) {
            pauseGame();
        }
        
        if (input.isKeyPressed(Key::Enter)) {
            restartLevel();
        }
    }
    
    void handleMouseInput() {
        auto& input = Application::instance().input();
        
        if (input.isMousePressed(MouseButton::Left)) {
            Vec2 pos = input.getMousePosition();
            onClick(pos);
        }
    }
    
    void onClick(const Vec2& pos) {
        // 处理点击
    }
    
    Ptr<Player> player_;
};
```

---

## 5. 完整输入示例

```cpp
#include <easy2d/easy2d.h>
#include <easy2d/utils/logger.h>

using namespace easy2d;

// ============================================================================
// 输入演示场景
// ============================================================================
class InputDemoScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        setBackgroundColor(Color(0.1f, 0.1f, 0.15f, 1.0f));
        
        auto& resources = Application::instance().resources();
        font_ = resources.loadFont("C:/Windows/Fonts/simsun.ttc", 18);
        
        playerPos_ = Vec2(400.0f, 300.0f);
    }
    
    void onUpdate(float dt) override {
        Scene::onUpdate(dt);
        
        auto& input = Application::instance().input();
        
        // 键盘移动（按住）
        float speed = 200.0f;
        isMoving_ = false;
        
        if (input.isKeyDown(Key::W) || input.isKeyDown(Key::Up)) {
            playerPos_.y -= speed * dt;
            currentDir_ = "上";
            isMoving_ = true;
        } else if (input.isKeyDown(Key::S) || input.isKeyDown(Key::Down)) {
            playerPos_.y += speed * dt;
            currentDir_ = "下";
            isMoving_ = true;
        } else if (input.isKeyDown(Key::A) || input.isKeyDown(Key::Left)) {
            playerPos_.x -= speed * dt;
            currentDir_ = "左";
            isMoving_ = true;
        } else if (input.isKeyDown(Key::D) || input.isKeyDown(Key::Right)) {
            playerPos_.x += speed * dt;
            currentDir_ = "右";
            isMoving_ = true;
        }
        
        // 按键触发（只触发一次）
        if (input.isKeyPressed(Key::Space)) {
            actionText_ = "跳跃!";
            actionTimer_ = 1.0f;
        }
        
        if (input.isKeyPressed(Key::Escape)) {
            actionText_ = "暂停菜单";
            actionTimer_ = 1.0f;
        }
        
        // 鼠标输入
        mousePos_ = input.getMousePosition();
        
        if (input.isMousePressed(MouseButton::Left)) {
            clickText_ = "左键点击: (" + std::to_string((int)mousePos_.x) + 
                        ", " + std::to_string((int)mousePos_.y) + ")";
            clickTimer_ = 2.0f;
        }
        
        // 更新计时器
        if (actionTimer_ > 0) {
            actionTimer_ -= dt;
        }
        if (clickTimer_ > 0) {
            clickTimer_ -= dt;
        }
        
        // 边界限制
        playerPos_.x = std::max(20.0f, std::min(780.0f, playerPos_.x));
        playerPos_.y = std::max(20.0f, std::min(580.0f, playerPos_.y));
    }
    
    void onRender(RenderBackend& renderer) override {
        Scene::onRender(renderer);
        
        if (!font_) return;
        
        // 绘制玩家（简单矩形表示）
        renderer.fillRect(
            Rect(playerPos_.x - 20, playerPos_.y - 20, 40, 40),
            Color(0.2f, 0.6f, 1.0f, 1.0f)
        );
        
        // 绘制说明
        renderer.drawText(*font_, "Easy2D 输入系统演示",
                         Vec2(300.0f, 20.0f), Color(1.0f, 0.9f, 0.2f, 1.0f));
        
        renderer.drawText(*font_, "WASD 或方向键移动（按住）",
                         Vec2(20.0f, 60.0f), Color(0.8f, 0.8f, 0.8f, 1.0f));
        renderer.drawText(*font_, "空格键跳跃（按下触发）",
                         Vec2(20.0f, 85.0f), Color(0.8f, 0.8f, 0.8f, 1.0f));
        renderer.drawText(*font_, "ESC 打开菜单（按下触发）",
                         Vec2(20.0f, 110.0f), Color(0.8f, 0.8f, 0.8f, 1.0f));
        renderer.drawText(*font_, "鼠标左键点击",
                         Vec2(20.0f, 135.0f), Color(0.8f, 0.8f, 0.8f, 1.0f));
        
        // 显示状态
        std::string status = "方向: " + currentDir_ + "  状态: " + 
                            (isMoving_ ? "移动中" : "静止");
        renderer.drawText(*font_, status,
                         Vec2(20.0f, 500.0f), Color(0.5f, 1.0f, 0.5f, 1.0f));
        
        // 显示动作
        if (actionTimer_ > 0) {
            renderer.drawText(*font_, actionText_,
                            Vec2(350.0f, 300.0f), Color(1.0f, 1.0f, 0.0f, 1.0f));
        }
        
        // 显示鼠标位置
        std::string mouseText = "鼠标: (" + std::to_string((int)mousePos_.x) + 
                               ", " + std::to_string((int)mousePos_.y) + ")";
        renderer.drawText(*font_, mouseText,
                         Vec2(600.0f, 500.0f), Color(0.8f, 0.8f, 1.0f, 1.0f));
        
        // 显示点击信息
        if (clickTimer_ > 0) {
            renderer.drawText(*font_, clickText_,
                            Vec2(250.0f, 350.0f), Color(1.0f, 0.5f, 0.5f, 1.0f));
        }
    }

private:
    Ptr<FontAtlas> font_;
    Vec2 playerPos_;
    Vec2 mousePos_;
    std::string currentDir_ = "无";
    bool isMoving_ = false;
    std::string actionText_;
    float actionTimer_ = 0.0f;
    std::string clickText_;
    float clickTimer_ = 0.0f;
};

// ============================================================================
// 主函数
// ============================================================================
int main() {
    Logger::init();
    Logger::setLevel(LogLevel::Info);
    
    auto& app = Application::instance();
    
    AppConfig config;
    config.title = "输入系统演示";
    config.width = 800;
    config.height = 600;
    config.vsync = true;
    
    if (!app.init(config)) {
        E2D_LOG_ERROR("初始化失败！");
        return -1;
    }
    
    app.enterScene(makePtr<InputDemoScene>());
    app.run();
    
    Logger::shutdown();
    return 0;
}
```
