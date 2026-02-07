# Easy2D 完整游戏开发教程

## 1. 推箱子游戏完整示例

本教程将带你从零开始创建一个完整的推箱子游戏，涵盖Easy2D的所有核心功能。

### 1.1 项目结构
```
PushBox/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── core/
│   │   ├── data.h
│   │   ├── data.cpp
│   │   ├── storage.h
│   │   ├── storage.cpp
│   │   └── audio_context.h
│   │   └── audio_context.cpp
│   ├── nodes/
│   │   └── audio_controller.h
│   │   └── audio_controller.cpp
│   ├── scenes/
│   │   ├── start_scene.h
│   │   ├── start_scene.cpp
│   │   ├── play_scene.h
│   │   ├── play_scene.cpp
│   │   ├── success_scene.h
│   │   └── success_scene.cpp
│   └── assets/
│       ├── images/
│       └── audio/
```

### 1.2 数据定义 (data.h)
```cpp
#pragma once
#include <easy2d/easy2d.h>

#define MAX_LEVEL 8

namespace pushbox {

enum TYPE { Empty, Wall, Ground, Box, Man };

struct Piece {
    TYPE type;
    bool isPoint;
};

struct Map {
    int width;
    int height;
    int roleX;
    int roleY;
    Piece value[12][12];
};

// 全局数据
extern Map g_Maps[MAX_LEVEL];
extern int g_CurrentLevel;
extern bool g_SoundOpen;
extern int g_Direct;
extern bool g_Pushing;

} // namespace pushbox
```

### 1.3 主函数 (main.cpp)
```cpp
#include "scenes/start_scene.h"
#include <easy2d/easy2d.h>
#include <filesystem>

static std::filesystem::path getExecutableDir(int argc, char** argv) {
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

int main(int argc, char** argv) {
    easy2d::Logger::init();
    easy2d::Logger::setLevel(easy2d::LogLevel::Info);
    
    auto& app = easy2d::Application::instance();
    
    easy2d::AppConfig config;
    config.title = "推箱子";
    config.width = 640;
    config.height = 480;
    config.vsync = true;
    config.fpsLimit = 0;
    
    if (!app.init(config)) {
        easy2d::Logger::shutdown();
        return -1;
    }
    
    // 设置资源搜索路径
    const auto exeDir = getExecutableDir(argc, argv);
    auto& resources = app.resources();
    resources.addSearchPath(exeDir.string());
    resources.addSearchPath((exeDir / "assets").string());
    
    // 进入开始场景
    app.enterScene(easy2d::makePtr<pushbox::StartScene>());
    
    app.run();
    app.shutdown();
    
    easy2d::Logger::shutdown();
    return 0;
}
```

### 1.4 游戏场景 (play_scene.cpp)
```cpp
#include "play_scene.h"
#include "success_scene.h"
#include "start_scene.h"
#include "../nodes/audio_controller.h"
#include <easy2d/easy2d.h>

namespace pushbox {

static easy2d::Ptr<easy2d::FontAtlas> loadFont(int size) {
    auto& resources = easy2d::Application::instance().resources();
    const char* candidates[] = {
        "C:/Windows/Fonts/simsun.ttc",
        "C:/Windows/Fonts/simhei.ttf",
        "C:/Windows/Fonts/arial.ttf",
    };
    
    for (auto* path : candidates) {
        auto font = resources.loadFont(path, size);
        if (font) return font;
    }
    return nullptr;
}

PlayScene::PlayScene(int level) {
    setBackgroundColor(easy2d::Colors::Black);
    
    // 设置视口
    auto& app = easy2d::Application::instance();
    auto& cfg = app.getConfig();
    setViewportSize(static_cast<float>(cfg.width), static_cast<float>(cfg.height));
    
    auto& resources = app.resources();
    
    // 加载纹理
    texWall_ = resources.loadTexture("assets/images/wall.gif");
    texPoint_ = resources.loadTexture("assets/images/point.gif");
    texFloor_ = resources.loadTexture("assets/images/floor.gif");
    texBox_ = resources.loadTexture("assets/images/box.gif");
    texBoxInPoint_ = resources.loadTexture("assets/images/boxinpoint.gif");
    
    // 加载角色纹理
    texMan_[1] = resources.loadTexture("assets/images/player/manup.gif");
    texMan_[2] = resources.loadTexture("assets/images/player/mandown.gif");
    texMan_[3] = resources.loadTexture("assets/images/player/manleft.gif");
    texMan_[4] = resources.loadTexture("assets/images/player/manright.gif");
    
    // 加载字体
    font28_ = loadFont(28);
    font20_ = loadFont(20);
    
    // 创建UI
    levelText_ = easy2d::Text::create("", font28_);
    levelText_->setPosition(520.0f, 30.0f);
    levelText_->setTextColor(easy2d::Colors::White);
    addChild(levelText_);
    
    stepText_ = easy2d::Text::create("", font20_);
    stepText_->setPosition(520.0f, 100.0f);
    stepText_->setTextColor(easy2d::Colors::White);
    addChild(stepText_);
    
    // 创建地图层
    mapLayer_ = easy2d::makePtr<easy2d::Node>();
    addChild(mapLayer_);
    
    // 创建音频控制器
    auto audioNode = AudioController::create();
    audioNode->setName("AudioController");
    addChild(audioNode);
    
    setLevel(level);
}

void PlayScene::onUpdate(float dt) {
    Scene::onUpdate(dt);
    
    auto& app = easy2d::Application::instance();
    auto& input = app.input();
    
    // ESC返回
    if (input.isKeyPressed(easy2d::Key::Escape)) {
        app.scenes().replaceScene(
            easy2d::makePtr<StartScene>(), 
            easy2d::TransitionType::Fade, 0.2f);
        return;
    }
    
    // 回车重开
    if (input.isKeyPressed(easy2d::Key::Enter)) {
        setLevel(g_CurrentLevel);
        return;
    }
    
    // 移动控制
    if (input.isKeyPressed(easy2d::Key::Up)) {
        move(0, -1, 1);
        flush();
    } else if (input.isKeyPressed(easy2d::Key::Down)) {
        move(0, 1, 2);
        flush();
    } else if (input.isKeyPressed(easy2d::Key::Left)) {
        move(-1, 0, 3);
        flush();
    } else if (input.isKeyPressed(easy2d::Key::Right)) {
        move(1, 0, 4);
        flush();
    }
    
    // 检查胜利条件
    for (int i = 0; i < map_.width; i++) {
        for (int j = 0; j < map_.height; j++) {
            Piece p = map_.value[j][i];
            if (p.type == TYPE::Box && p.isPoint == false) {
                return;  // 还有箱子不在目标点
            }
        }
    }
    
    gameOver();
}

void PlayScene::move(int dx, int dy, int direct) {
    int targetX = dx + map_.roleX;
    int targetY = dy + map_.roleY;
    g_Direct = direct;
    
    // 边界检查
    if (targetX < 0 || targetX >= map_.width || 
        targetY < 0 || targetY >= map_.height) {
        return;
    }
    
    // 墙壁检查
    if (map_.value[targetY][targetX].type == TYPE::Wall) {
        return;
    }
    
    // 空地移动
    if (map_.value[targetY][targetX].type == TYPE::Ground) {
        g_Pushing = false;
        map_.value[map_.roleY][map_.roleX].type = TYPE::Ground;
        map_.value[targetY][targetX].type = TYPE::Man;
    }
    // 推箱子
    else if (map_.value[targetY][targetX].type == TYPE::Box) {
        g_Pushing = true;
        
        int boxX = targetX;
        int boxY = targetY;
        
        // 计算箱子目标位置
        switch (g_Direct) {
            case 1: boxY--; break;
            case 2: boxY++; break;
            case 3: boxX--; break;
            case 4: boxX++; break;
        }
        
        // 检查箱子能否移动
        if (boxX < 0 || boxX >= map_.width || 
            boxY < 0 || boxY >= map_.height) {
            return;
        }
        if (map_.value[boxY][boxX].type == TYPE::Wall || 
            map_.value[boxY][boxX].type == TYPE::Box) {
            return;
        }
        
        // 移动箱子
        map_.value[boxY][boxX].type = TYPE::Box;
        map_.value[targetY][targetX].type = TYPE::Man;
        map_.value[map_.roleY][map_.roleX].type = TYPE::Ground;
    } else {
        return;
    }
    
    map_.roleX = targetX;
    map_.roleY = targetY;
    setStep(step_ + 1);
}

void PlayScene::flush() {
    mapLayer_->removeAllChildren();
    
    int tileW = texFloor_ ? texFloor_->getWidth() : 32;
    int tileH = texFloor_ ? texFloor_->getHeight() : 32;
    
    float offsetX = static_cast<float>((12 - map_.width) / 2) * tileW;
    float offsetY = static_cast<float>((12 - map_.height) / 2) * tileH;
    
    for (int i = 0; i < map_.width; i++) {
        for (int j = 0; j < map_.height; j++) {
            Piece piece = map_.value[j][i];
            easy2d::Ptr<easy2d::Texture> tex;
            
            // 选择纹理
            if (piece.type == TYPE::Wall) {
                tex = texWall_;
            } else if (piece.type == TYPE::Ground && piece.isPoint) {
                tex = texPoint_;
            } else if (piece.type == TYPE::Ground) {
                tex = texFloor_;
            } else if (piece.type == TYPE::Box && piece.isPoint) {
                tex = texBoxInPoint_;
            } else if (piece.type == TYPE::Box) {
                tex = texBox_;
            } else if (piece.type == TYPE::Man && g_Pushing) {
                tex = texManPush_[g_Direct];
            } else if (piece.type == TYPE::Man) {
                tex = texMan_[g_Direct];
            } else {
                continue;
            }
            
            if (!tex) continue;
            
            auto sprite = easy2d::Sprite::create(tex);
            sprite->setAnchor(0.0f, 0.0f);
            sprite->setPosition(offsetX + static_cast<float>(i * tileW),
                              offsetY + static_cast<float>(j * tileH));
            mapLayer_->addChild(sprite);
        }
    }
}

void PlayScene::setLevel(int level) {
    g_CurrentLevel = level;
    
    if (levelText_) {
        levelText_->setText("第" + std::to_string(level) + "关");
    }
    
    setStep(0);
    map_ = g_Maps[level - 1];
    g_Direct = 2;
    g_Pushing = false;
    flush();
}

void PlayScene::setStep(int step) {
    step_ = step;
    if (stepText_) {
        stepText_->setText("当前" + std::to_string(step) + "步");
    }
}

void PlayScene::gameOver() {
    if (g_CurrentLevel == MAX_LEVEL) {
        // 通关
        easy2d::Application::instance().scenes().pushScene(
            easy2d::makePtr<SuccessScene>(),
            easy2d::TransitionType::Fade, 0.25f);
    } else {
        // 下一关
        setLevel(g_CurrentLevel + 1);
    }
}

} // namespace pushbox
```

---

## 2. 开发流程总结

### 2.1 创建新游戏的步骤

```cpp
// 步骤1: 包含头文件
#include <easy2d/easy2d.h>
#include <easy2d/utils/logger.h>

// 步骤2: 使用命名空间
using namespace easy2d;

// 步骤3: 创建场景类
class MyGameScene : public Scene {
public:
    void onEnter() override;
    void onUpdate(float dt) override;
    void onRender(RenderBackend& renderer) override;
    void onExit() override;
};

// 步骤4: 实现场景方法
void MyGameScene::onEnter() {
    Scene::onEnter();
    // 初始化资源
    // 创建游戏对象
}

void MyGameScene::onUpdate(float dt) {
    Scene::onUpdate(dt);
    // 游戏逻辑更新
    // 输入处理
}

void MyGameScene::onRender(RenderBackend& renderer) {
    Scene::onRender(renderer);
    // 自定义绘制
}

void MyGameScene::onExit() {
    // 清理资源
    Scene::onExit();
}

// 步骤5: 主函数
int main() {
    Logger::init();
    Logger::setLevel(LogLevel::Info);
    
    auto& app = Application::instance();
    
    AppConfig config;
    config.title = "游戏标题";
    config.width = 800;
    config.height = 600;
    config.vsync = true;
    
    if (!app.init(config)) {
        return -1;
    }
    
    app.enterScene(makePtr<MyGameScene>());
    app.run();
    
    Logger::shutdown();
    return 0;
}
```

### 2.2 常用功能速查

| 功能 | API |
|------|-----|
| 加载纹理 | `resources.loadTexture("path.png")` |
| 加载字体 | `resources.loadFont("path.ttf", size)` |
| 创建精灵 | `Sprite::create(texture)` |
| 创建文本 | `Text::create("text", font)` |
| 键盘检测 | `input.isKeyPressed(Key::Space)` |
| 鼠标检测 | `input.isMousePressed(MouseButton::Left)` |
| 切换场景 | `scenes().replaceScene(makePtr<Scene>())` |
| 绘制矩形 | `renderer.fillRect(rect, color)` |
| 绘制文本 | `renderer.drawText(font, text, pos, color)` |

---

## 3. 最佳实践

### 3.1 资源管理
- 在 `onEnter()` 中集中加载资源
- 使用纹理池缓存重复使用的纹理
- 添加多个资源搜索路径以适配不同环境

### 3.2 性能优化
- 使用 `beginSpriteBatch()` / `endSpriteBatch()` 批量渲染
- 启用空间索引进行碰撞检测
- 合理设置纹理池大小

### 3.3 代码组织
- 按功能划分文件（场景、节点、数据等）
- 使用命名空间避免命名冲突
- 合理使用智能指针管理内存

---

## 4. 调试技巧

### 4.1 日志输出
```cpp
E2D_LOG_INFO("信息: {}", value);
E2D_LOG_WARN("警告");
E2D_LOG_ERROR("错误: {}", error);
```

### 4.2 FPS显示
```cpp
void onRender(RenderBackend& renderer) override {
    std::stringstream ss;
    ss << "FPS: " << Application::instance().fps();
    renderer.drawText(*font_, ss.str(), Vec2(10, 10), Color::White);
}
```

### 4.3 碰撞框可视化
```cpp
void onDraw(RenderBackend& renderer) override {
    renderer.drawRect(getBoundingBox(), Color::Red, 1.0f);
}
```
