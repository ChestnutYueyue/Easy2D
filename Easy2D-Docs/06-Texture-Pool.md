# Easy2D 纹理池 API 文档

## 1. 纹理池概述

纹理池(TexturePool)是一个LRU(最近最少使用)缓存系统，用于管理和复用纹理资源，减少重复加载和内存占用。

### 1.1 头文件
```cpp
#include <easy2d/graphics/texture_pool.h>
```

### 1.2 获取纹理池实例
```cpp
// 使用宏获取纹理池单例
auto& pool = E2D_TEXTURE_POOL();
```

---

## 2. 初始化和关闭

### 2.1 初始化纹理池
```cpp
// 配置纹理池
TexturePoolConfig poolConfig;
poolConfig.maxCacheSize = 32 * 1024 * 1024;   // 32MB 最大缓存大小
poolConfig.maxTextureCount = 128;              // 最大纹理数量

// 初始化
E2D_TEXTURE_POOL().init(poolConfig);
```

### 2.2 关闭纹理池
```cpp
// 在场景退出或应用关闭时调用
void onExit() override {
    E2D_TEXTURE_POOL().shutdown();
    Scene::onExit();
}
```

---

## 3. 纹理管理

### 3.1 添加纹理到池
```cpp
// 方式1: 添加已加载的纹理
auto texture = resources.loadTexture("player.png");
E2D_TEXTURE_POOL().add("player", texture);

// 方式2: 从内存数据创建并添加
std::vector<uint8_t> pixels(width * height * 4);
// ... 填充像素数据 ...
auto tex = E2D_TEXTURE_POOL().createFromData(
    "procedural_texture",  // 名称
    pixels.data(),         // 像素数据
    width,                 // 宽度
    height,                // 高度
    PixelFormat::RGBA8     // 像素格式
);
```

### 3.2 从池中获取纹理
```cpp
// 获取纹理
auto texture = E2D_TEXTURE_POOL().get("player");

if (texture && texture->isValid()) {
    // 使用纹理创建精灵
    auto sprite = Sprite::create(texture);
}
```

### 3.3 检查纹理是否存在
```cpp
if (E2D_TEXTURE_POOL().has("player")) {
    // 纹理存在
}
```

### 3.4 移除纹理
```cpp
// 从池中移除纹理
E2D_TEXTURE_POOL().remove("player");
```

---

## 4. 统计信息

### 4.1 获取纹理池状态
```cpp
// 获取纹理数量
size_t count = E2D_TEXTURE_POOL().getTextureCount();

// 获取缓存大小（字节）
size_t size = E2D_TEXTURE_POOL().getCacheSize();

// 获取命中率（0.0 - 1.0）
float hitRate = E2D_TEXTURE_POOL().getHitRate();
```

### 4.2 显示统计信息
```cpp
void drawStats(RenderBackend& renderer) {
    if (!font_) return;
    
    auto texCount = E2D_TEXTURE_POOL().getTextureCount();
    auto texSize = E2D_TEXTURE_POOL().getCacheSize() / 1024;  // KB
    auto hitRate = E2D_TEXTURE_POOL().getHitRate() * 100.0f;  // 百分比
    
    std::stringstream ss;
    ss << "纹理池: " << texCount << " 个, " << texSize << " KB, 命中率: " 
       << std::fixed << std::setprecision(1) << hitRate << "%";
    renderer.drawText(*font_, ss.str(), Vec2(20.0f, 560.0f), 
                     Color(0.5f, 1.0f, 0.5f, 1.0f));
}
```

---

## 5. 程序化生成纹理

### 5.1 创建渐变圆形纹理
```cpp
void createCircleTexture(const std::string& name, int size) {
    std::vector<uint8_t> pixels(size * size * 4);
    
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            // 计算到中心的距离
            float dx = x - size / 2.0f;
            float dy = y - size / 2.0f;
            float dist = std::sqrt(dx * dx + dy * dy) / (size / 2.0f);
            
            // 创建圆形渐变
            float alpha = 1.0f - std::min(dist, 1.0f);
            
            int idx = (y * size + x) * 4;
            pixels[idx + 0] = static_cast<uint8_t>(255 * (1.0f - dist * 0.5f)); // R
            pixels[idx + 1] = static_cast<uint8_t>(255 * (0.5f + dist * 0.3f)); // G
            pixels[idx + 2] = static_cast<uint8_t>(255 * (0.8f - dist * 0.3f)); // B
            pixels[idx + 3] = static_cast<uint8_t>(255 * alpha);               // A
        }
    }
    
    E2D_TEXTURE_POOL().createFromData(name, pixels.data(), size, size, PixelFormat::RGBA8);
}
```

### 5.2 创建条纹纹理
```cpp
void createStripeTexture(const std::string& name, int size) {
    std::vector<uint8_t> pixels(size * size * 4);
    
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            // 创建条纹图案
            float stripe = std::sin(x * 0.1f) * 0.5f + 0.5f;
            
            int idx = (y * size + x) * 4;
            pixels[idx + 0] = static_cast<uint8_t>(255 * stripe);        // R
            pixels[idx + 1] = static_cast<uint8_t>(255 * (1.0f - stripe)); // G
            pixels[idx + 2] = static_cast<uint8_t>(255 * 0.5f);          // B
            pixels[idx + 3] = 200;                                        // A
        }
    }
    
    E2D_TEXTURE_POOL().createFromData(name, pixels.data(), size, size, PixelFormat::RGBA8);
}
```

---

## 6. 完整示例

```cpp
#include <easy2d/easy2d.h>
#include <easy2d/graphics/texture_pool.h>
#include <easy2d/utils/logger.h>
#include <sstream>

using namespace easy2d;

// ============================================================================
// 纹理池演示场景
// ============================================================================
class TexturePoolDemoScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        setBackgroundColor(Color(0.05f, 0.05f, 0.1f, 1.0f));
        
        // 初始化纹理池
        initTexturePool();
        
        // 创建程序化纹理
        createProceduralTextures();
        
        // 加载字体
        auto& resources = Application::instance().resources();
        font_ = resources.loadFont("C:/Windows/Fonts/simsun.ttc", 18);
        
        // 创建显示精灵
        createDisplaySprites();
    }
    
    void onExit() override {
        E2D_TEXTURE_POOL().shutdown();
        Scene::onExit();
    }
    
    void onUpdate(float dt) override {
        Scene::onUpdate(dt);
        E2D_TEXTURE_POOL().update(dt);
    }
    
    void onRender(RenderBackend& renderer) override {
        Scene::onRender(renderer);
        
        // 绘制UI
        drawUI(renderer);
    }

private:
    Ptr<FontAtlas> font_;
    std::vector<Ptr<Sprite>> sprites_;
    
    void initTexturePool() {
        TexturePoolConfig poolConfig;
        poolConfig.maxCacheSize = 64 * 1024 * 1024;  // 64MB
        poolConfig.maxTextureCount = 256;
        E2D_TEXTURE_POOL().init(poolConfig);
        
        E2D_INFO("纹理池初始化完成");
    }
    
    void createProceduralTextures() {
        const int size = 256;
        
        // 1. 创建圆形渐变纹理
        {
            std::vector<uint8_t> pixels(size * size * 4);
            for (int y = 0; y < size; ++y) {
                for (int x = 0; x < size; ++x) {
                    float dx = x - size / 2.0f;
                    float dy = y - size / 2.0f;
                    float dist = std::sqrt(dx * dx + dy * dy) / (size / 2.0f);
                    float alpha = 1.0f - std::min(dist, 1.0f);
                    
                    int idx = (y * size + x) * 4;
                    pixels[idx + 0] = static_cast<uint8_t>(255 * (1.0f - dist * 0.5f));
                    pixels[idx + 1] = static_cast<uint8_t>(255 * (0.5f + dist * 0.3f));
                    pixels[idx + 2] = static_cast<uint8_t>(255 * (0.8f - dist * 0.3f));
                    pixels[idx + 3] = static_cast<uint8_t>(255 * alpha);
                }
            }
            E2D_TEXTURE_POOL().createFromData("circle_gradient", 
                pixels.data(), size, size, PixelFormat::RGBA8);
        }
        
        // 2. 创建条纹纹理
        {
            std::vector<uint8_t> pixels(size * size * 4);
            for (int y = 0; y < size; ++y) {
                for (int x = 0; x < size; ++x) {
                    float stripe = std::sin(x * 0.1f) * 0.5f + 0.5f;
                    int idx = (y * size + x) * 4;
                    pixels[idx + 0] = static_cast<uint8_t>(255 * stripe);
                    pixels[idx + 1] = static_cast<uint8_t>(255 * (1.0f - stripe));
                    pixels[idx + 2] = static_cast<uint8_t>(255 * 0.5f);
                    pixels[idx + 3] = 200;
                }
            }
            E2D_TEXTURE_POOL().createFromData("stripe_pattern", 
                pixels.data(), size, size, PixelFormat::RGBA8);
        }
        
        // 3. 创建棋盘纹理
        {
            std::vector<uint8_t> pixels(size * size * 4);
            int tileSize = 32;
            for (int y = 0; y < size; ++y) {
                for (int x = 0; x < size; ++x) {
                    bool isWhite = ((x / tileSize) + (y / tileSize)) % 2 == 0;
                    uint8_t c = isWhite ? 240 : 80;
                    int idx = (y * size + x) * 4;
                    pixels[idx + 0] = c;
                    pixels[idx + 1] = c;
                    pixels[idx + 2] = c;
                    pixels[idx + 3] = 255;
                }
            }
            E2D_TEXTURE_POOL().createFromData("checkerboard", 
                pixels.data(), size, size, PixelFormat::RGBA8);
        }
        
        E2D_INFO("程序化纹理创建完成");
    }
    
    void createDisplaySprites() {
        std::vector<std::string> names = {"circle_gradient", "stripe_pattern", "checkerboard"};
        float startX = 150.0f;
        float spacing = 250.0f;
        
        for (size_t i = 0; i < names.size(); ++i) {
            auto tex = E2D_TEXTURE_POOL().get(names[i]);
            if (tex) {
                auto sprite = Sprite::create(tex);
                sprite->setPosition(Vec2(startX + i * spacing, 300.0f));
                sprite->setScale(0.8f);
                addChild(sprite);
                sprites_.push_back(sprite);
            }
        }
    }
    
    void drawUI(RenderBackend& renderer) {
        if (!font_) return;
        
        // 标题
        renderer.drawText(*font_, "纹理池系统演示",
                         Vec2(320.0f, 30.0f), Color(1.0f, 0.9f, 0.2f, 1.0f));
        
        // 纹理名称
        renderer.drawText(*font_, "圆形渐变",
                         Vec2(110.0f, 450.0f), Color(0.7f, 0.9f, 0.7f, 1.0f));
        renderer.drawText(*font_, "条纹图案",
                         Vec2(360.0f, 450.0f), Color(0.7f, 0.9f, 0.7f, 1.0f));
        renderer.drawText(*font_, "棋盘格",
                         Vec2(620.0f, 450.0f), Color(0.7f, 0.9f, 0.7f, 1.0f));
        
        // 统计信息
        auto texCount = E2D_TEXTURE_POOL().getTextureCount();
        auto texSize = E2D_TEXTURE_POOL().getCacheSize() / 1024;
        auto hitRate = E2D_TEXTURE_POOL().getHitRate() * 100.0f;
        
        std::stringstream ss;
        ss << "纹理池: " << texCount << " 个纹理, " << texSize << " KB, 命中率: " 
           << std::fixed << std::setprecision(1) << hitRate << "%";
        renderer.drawText(*font_, ss.str(), Vec2(20.0f, 560.0f), 
                         Color(0.5f, 1.0f, 0.5f, 1.0f));
        
        // FPS
        auto fps = Application::instance().fps();
        ss.str("");
        ss << "FPS: " << fps;
        renderer.drawText(*font_, ss.str(), Vec2(700.0f, 560.0f), 
                         Color(0.5f, 1.0f, 0.5f, 1.0f));
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
    config.title = "纹理池演示";
    config.width = 800;
    config.height = 600;
    config.vsync = true;
    
    if (!app.init(config)) {
        E2D_LOG_ERROR("初始化失败！");
        return -1;
    }
    
    app.enterScene(makePtr<TexturePoolDemoScene>());
    app.run();
    
    Logger::shutdown();
    return 0;
}
```
