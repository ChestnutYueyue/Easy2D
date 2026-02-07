# Easy2D 资源系统 API 文档

## 1. 资源管理器 (ResourceManager)

### 1.1 获取资源管理器
```cpp
auto& resources = Application::instance().resources();
```

---

## 2. 纹理 (Texture)

### 2.1 加载纹理
```cpp
// 从文件加载纹理
auto texture = resources.loadTexture("assets/images/player.png");

// 检查纹理是否加载成功
if (texture && texture->isValid()) {
    E2D_INFO("纹理加载成功: {}x{}", texture->getWidth(), texture->getHeight());
} else {
    E2D_ERROR("纹理加载失败!");
}
```

### 2.2 纹理属性
```cpp
// 获取纹理尺寸
int width = texture->getWidth();
int height = texture->getHeight();

// 检查纹理是否有效
bool valid = texture->isValid();
```

### 2.3 创建精灵使用纹理
```cpp
// 方式1: 直接创建
auto sprite = Sprite::create(texture);

// 方式2: 先创建再设置
auto sprite = Sprite::create();
sprite->setTexture(texture);

// 添加到场景
sprite->setPosition(Vec2(400.0f, 300.0f));
addChild(sprite);
```

---

## 3. 字体 (FontAtlas)

### 3.1 加载字体
```cpp
// 方式1: 通过资源管理器加载
auto font = resources.loadFont("C:/Windows/Fonts/simsun.ttc", 24);

// 方式2: 通过渲染器创建（支持更多选项）
auto font = app.renderer().createFontAtlas("C:/Windows/Fonts/arial.ttf", 24, true);
```

### 3.2 字体回退策略
```cpp
// 尝试加载多个字体，直到成功
Ptr<FontAtlas> loadFont(int size) {
    auto& resources = Application::instance().resources();
    
    const char* candidates[] = {
        "C:/Windows/Fonts/simsun.ttc",    // 宋体（中文）
        "C:/Windows/Fonts/simhei.ttf",    // 黑体（中文）
        "C:/Windows/Fonts/segoeui.ttf",   // Segoe UI
        "C:/Windows/Fonts/arial.ttf",     // Arial
    };
    
    for (auto* path : candidates) {
        auto font = resources.loadFont(path, size);
        if (font) {
            E2D_INFO("成功加载字体: {}", path);
            return font;
        }
    }
    
    E2D_WARN("无法加载任何字体");
    return nullptr;
}
```

### 3.3 字体属性
```cpp
// 获取字体度量
float ascent = font->getAscent();       // 上升高度
float descent = font->getDescent();     // 下降深度
float lineGap = font->getLineGap();     // 行间距
float lineHeight = font->getLineHeight(); // 行高

// 获取字形信息
const Glyph* glyph = font->getGlyph(U'A');
if (glyph) {
    float bearingY = glyph->bearingY;   // 基线到顶部的距离
    float height = glyph->height;       // 字形高度
}
```

### 3.4 使用字体渲染文本
```cpp
// 方式1: 使用Text节点
auto text = Text::create("Hello World", font);
text->setPosition(Vec2(400.0f, 50.0f));
text->setTextColor(Color(1.0f, 1.0f, 0.0f, 1.0f));
addChild(text);

// 方式2: 在onRender中直接绘制
void onRender(RenderBackend& renderer) override {
    renderer.drawText(*font_, "直接绘制的文字", 
                     Vec2(100.0f, 100.0f), 
                     Color(1.0f, 1.0f, 1.0f, 1.0f));
}
```

---

## 4. 音频 (Sound)

### 4.1 加载音频
```cpp
// 加载音效
auto sound = resources.loadSound("assets/audio/jump.wav");

// 加载背景音乐
auto bgm = resources.loadSound("assets/audio/background.mp3");
```

### 4.2 播放音频
```cpp
// 播放音效（一次性）
sound->play();

// 循环播放背景音乐
bgm->setLoop(true);
bgm->play();

// 停止播放
bgm->stop();

// 暂停
bgm->pause();

// 恢复
bgm->resume();

// 设置音量 (0.0 - 1.0)
bgm->setVolume(0.5f);
```

### 4.3 音频控制器节点
```cpp
// AudioController.h
#pragma once
#include <easy2d/easy2d.h>

class AudioController : public easy2d::Node {
public:
    static easy2d::Ptr<AudioController> create();
    
    void onEnter() override;
    void setEnabled(bool enabled);
    bool isEnabled() const { return enabled_; }
    
    void playManMove();
    void playBoxMove();
    void playBackground();
    
private:
    bool loaded_ = false;
    bool enabled_ = true;
    
    easy2d::Ptr<easy2d::Sound> background_;
    easy2d::Ptr<easy2d::Sound> manMove_;
    easy2d::Ptr<easy2d::Sound> boxMove_;
};

// AudioController.cpp
easy2d::Ptr<AudioController> AudioController::create() {
    return easy2d::makePtr<AudioController>();
}

void AudioController::onEnter() {
    Node::onEnter();
    
    if (loaded_) return;
    
    auto& resources = easy2d::Application::instance().resources();
    
    background_ = resources.loadSound("assets/audio/background.wav");
    manMove_ = resources.loadSound("assets/audio/manmove.wav");
    boxMove_ = resources.loadSound("assets/audio/boxmove.wav");
    
    if (background_) {
        background_->setLoop(true);
        background_->setVolume(0.3f);
    }
    
    loaded_ = true;
}

void AudioController::setEnabled(bool enabled) {
    enabled_ = enabled;
    if (background_) {
        if (enabled) {
            background_->play();
        } else {
            background_->stop();
        }
    }
}

void AudioController::playManMove() {
    if (enabled_ && manMove_) {
        manMove_->play();
    }
}

void AudioController::playBoxMove() {
    if (enabled_ && boxMove_) {
        boxMove_->play();
    }
}
```

---

## 5. 资源搜索路径

### 5.1 添加搜索路径
```cpp
auto& resources = app.resources();

// 添加相对路径
resources.addSearchPath("assets");
resources.addSearchPath("assets/images");
resources.addSearchPath("assets/audio");
resources.addSearchPath("assets/fonts");

// 添加绝对路径
resources.addSearchPath("C:/Windows/Fonts");

// 添加多层级路径（用于开发时）
resources.addSearchPath("Easy2D/examples/demo/assets");
resources.addSearchPath("examples/demo/assets");
```

### 5.2 自动检测可执行文件路径
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

// 在main函数中使用
int main(int argc, char** argv) {
    // ... 初始化代码 ...
    
    const auto exeDir = getExecutableDir(argc, argv);
    auto& resources = app.resources();
    
    // 添加可执行文件所在目录及其子目录
    resources.addSearchPath(exeDir.string());
    resources.addSearchPath((exeDir / "assets").string());
    resources.addSearchPath((exeDir / "assets" / "images").string());
    resources.addSearchPath((exeDir / "assets" / "audio").string());
    
    // 添加父目录（用于开发环境）
    resources.addSearchPath((exeDir.parent_path() / "assets").string());
    
    // ... 后续代码 ...
}
```

---

## 6. 资源加载最佳实践

### 6.1 集中加载
```cpp
class GameScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        
        // 集中加载所有资源
        if (!loadResources()) {
            E2D_ERROR("资源加载失败！");
            return;
        }
        
        // 创建游戏对象
        createGameObjects();
    }
    
private:
    bool loadResources() {
        auto& resources = Application::instance().resources();
        
        // 加载纹理
        playerTexture_ = resources.loadTexture("player.png");
        enemyTexture_ = resources.loadTexture("enemy.png");
        bgTexture_ = resources.loadTexture("background.png");
        
        if (!playerTexture_ || !enemyTexture_) {
            E2D_ERROR("关键纹理加载失败！");
            return false;
        }
        
        // 加载字体
        font_ = resources.loadFont("C:/Windows/Fonts/simsun.ttc", 20);
        if (!font_) {
            E2D_WARN("字体加载失败，将使用默认字体");
        }
        
        // 加载音频
        jumpSound_ = resources.loadSound("jump.wav");
        
        E2D_INFO("所有资源加载完成");
        return true;
    }
    
    void createGameObjects() {
        // 使用已加载的资源创建对象
        auto player = Sprite::create(playerTexture_);
        player->setPosition(Vec2(400.0f, 300.0f));
        addChild(player);
    }
    
    Ptr<Texture> playerTexture_;
    Ptr<Texture> enemyTexture_;
    Ptr<Texture> bgTexture_;
    Ptr<FontAtlas> font_;
    Ptr<Sound> jumpSound_;
};
```

### 6.2 资源预加载
```cpp
class LoadingScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        setBackgroundColor(Color(0.1f, 0.1f, 0.2f, 1.0f));
        
        // 显示加载界面
        showLoadingUI();
        
        // 异步加载资源
        loadResourcesAsync();
    }
    
    void onUpdate(float dt) override {
        Scene::onUpdate(dt);
        
        // 检查加载进度
        if (isLoadingComplete()) {
            // 切换到游戏场景
            Application::instance().scenes().replaceScene(
                makePtr<GameScene>(), TransitionType::Fade, 0.5f);
        }
    }
    
private:
    void showLoadingUI() {
        // 显示"加载中..."
    }
    
    void loadResourcesAsync() {
        // 异步加载资源
    }
    
    bool isLoadingComplete() {
        // 检查是否加载完成
        return true;
    }
};
```

---

## 7. 完整资源管理示例

```cpp
#include <easy2d/easy2d.h>
#include <easy2d/utils/logger.h>

using namespace easy2d;

// ============================================================================
// 资源演示场景
// ============================================================================
class ResourceDemoScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        setBackgroundColor(Color(0.05f, 0.05f, 0.1f, 1.0f));
        
        // 加载资源
        loadResources();
        
        // 创建显示对象
        createDisplayObjects();
    }
    
    void onRender(RenderBackend& renderer) override {
        Scene::onRender(renderer);
        
        // 绘制说明文字
        drawInstructions(renderer);
    }

private:
    Ptr<FontAtlas> titleFont_;
    Ptr<FontAtlas> infoFont_;
    Ptr<Texture> texture_;
    Ptr<Sprite> sprite_;
    
    void loadResources() {
        auto& resources = Application::instance().resources();
        
        // 加载字体
        titleFont_ = resources.loadFont("C:/Windows/Fonts/simsun.ttc", 28);
        infoFont_ = resources.loadFont("C:/Windows/Fonts/simsun.ttc", 18);
        
        if (!titleFont_) {
            E2D_WARN("标题字体加载失败");
        }
        if (!infoFont_) {
            E2D_WARN("信息字体加载失败");
        }
        
        E2D_INFO("资源加载完成");
    }
    
    void createDisplayObjects() {
        // 创建精灵（使用纯色代替纹理）
        sprite_ = Sprite::create();
        sprite_->setPosition(Vec2(400.0f, 300.0f));
        sprite_->setColor(Color(0.2f, 0.6f, 1.0f, 1.0f));
        addChild(sprite_);
    }
    
    void drawInstructions(RenderBackend& renderer) {
        if (!titleFont_ || !infoFont_) return;
        
        // 标题
        renderer.drawText(*titleFont_, "资源系统演示",
                         Vec2(300.0f, 30.0f), Color(1.0f, 0.9f, 0.2f, 1.0f));
        
        // 说明
        float y = 80.0f;
        renderer.drawText(*infoFont_, "支持的资源类型:",
                         Vec2(50.0f, y), Color(0.8f, 0.8f, 0.8f, 1.0f));
        y += 30.0f;
        
        renderer.drawText(*infoFont_, "1. 纹理 (Texture) - PNG, JPG, BMP等",
                         Vec2(70.0f, y), Color(0.7f, 0.9f, 0.7f, 1.0f));
        y += 25.0f;
        
        renderer.drawText(*infoFont_, "2. 字体 (FontAtlas) - TTF, TTC等",
                         Vec2(70.0f, y), Color(0.7f, 0.9f, 0.7f, 1.0f));
        y += 25.0f;
        
        renderer.drawText(*infoFont_, "3. 音频 (Sound) - WAV, MP3, OGG等",
                         Vec2(70.0f, y), Color(0.7f, 0.9f, 0.7f, 1.0f));
        y += 25.0f;
        
        renderer.drawText(*infoFont_, "API: resources.loadTexture(), loadFont(), loadSound()",
                         Vec2(50.0f, y + 20.0f), Color(0.6f, 0.8f, 1.0f, 1.0f));
    }
};

// ============================================================================
// 主函数
// ============================================================================
int main() {
    Logger::init();
    Logger::setLevel(LogLevel::Info);
    
    auto& app = Application::instance();
    
    AppConfig config;
    config.title = "资源系统演示";
    config.width = 800;
    config.height = 600;
    config.vsync = true;
    
    if (!app.init(config)) {
        E2D_LOG_ERROR("初始化失败！");
        return -1;
    }
    
    // 添加资源搜索路径
    auto& resources = app.resources();
    resources.addSearchPath("assets");
    resources.addSearchPath("assets/images");
    resources.addSearchPath("assets/audio");
    
    app.enterScene(makePtr<ResourceDemoScene>());
    app.run();
    
    Logger::shutdown();
    return 0;
}
```
