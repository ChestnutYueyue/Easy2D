# Easy2D 渲染目标 API 文档

## 1. 渲染目标概述

渲染目标(RenderTarget)用于离屏渲染(Off-screen Rendering)，可以将场景渲染到纹理而不是屏幕上，常用于特效合成、后处理等。

### 1.1 头文件
```cpp
#include <easy2d/graphics/render_target.h>
```

---

## 2. 创建渲染目标

### 2.1 使用配置创建
```cpp
// 配置渲染目标
RenderTargetConfig rtConfig;
rtConfig.width = 256;           // 宽度
rtConfig.height = 256;          // 高度
rtConfig.hasDepth = false;      // 是否需要深度缓冲

// 创建渲染目标
auto renderTarget = RenderTarget::createFromConfig(rtConfig);

if (renderTarget) {
    E2D_INFO("渲染目标创建成功: {}x{}", rtConfig.width, rtConfig.height);
}
```

### 2.2 创建多个渲染目标
```cpp
// 创建多层渲染目标用于特效合成
auto layer1 = RenderTarget::createFromConfig(rtConfig);
auto layer2 = RenderTarget::createFromConfig(rtConfig);
auto finalTarget = RenderTarget::createFromConfig(rtConfig);
```

---

## 3. 使用渲染目标

### 3.1 绑定和解绑
```cpp
// 绑定渲染目标（后续绘制将渲染到此目标）
renderTarget->bind();

// 清除渲染目标
renderTarget->clear(Color(0.0f, 0.0f, 0.0f, 0.0f));

// 执行绘制操作...
renderer.fillRect(Rect(0, 0, 100, 100), Color::Red);

// 解绑渲染目标（恢复屏幕渲染）
renderTarget->unbind();
```

### 3.2 获取渲染结果
```cpp
// 获取颜色纹理
auto colorTexture = renderTarget->getColorTexture();

// 使用纹理创建精灵
auto sprite = Sprite::create(colorTexture);
sprite->setPosition(Vec2(400.0f, 300.0f));
addChild(sprite);
```

---

## 4. 投影矩阵设置

### 4.1 为渲染目标设置投影
```cpp
#include <glm/gtc/matrix_transform.hpp>

// 绑定渲染目标
renderTarget->bind();
renderTarget->clear(Color(0.0f, 0.0f, 0.0f, 0.0f));

// 设置正交投影矩阵（匹配渲染目标尺寸）
glm::mat4 ortho = glm::ortho(0.0f, 256.0f, 256.0f, 0.0f, -1.0f, 1.0f);
renderer.setViewProjection(ortho);

// 绘制内容...

// 解绑
renderTarget->unbind();

// 恢复屏幕投影
glm::mat4 screenOrtho = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f);
renderer.setViewProjection(screenOrtho);
renderer.setViewport(0, 0, 800, 600);
```

---

## 5. 特效合成流程

### 5.1 多层渲染示例
```cpp
class EffectCompositionScene : public Scene {
private:
    Ptr<RenderTarget> characterTarget_;  // 角色渲染目标
    Ptr<RenderTarget> effectTarget1_;    // 特效层1
    Ptr<RenderTarget> finalTarget_;      // 最终合成
    
    void performEffectComposition() {
        auto& renderer = Application::instance().renderer();
        
        // 创建256x256的投影矩阵
        glm::mat4 ortho96 = glm::ortho(0.0f, 96.0f, 96.0f, 0.0f, -1.0f, 1.0f);
        
        // ========== 第1步: 渲染角色到离屏目标 ==========
        if (characterTarget_) {
            characterTarget_->bind();
            characterTarget_->clear(Color(0.0f, 0.0f, 0.0f, 0.0f));
            
            renderer.setViewProjection(ortho96);
            
            // 绘制角色...
            renderer.beginSpriteBatch();
            renderer.drawSprite(*playerTexture_, 
                               Rect(0, 0, 96, 96),
                               frameRect,
                               Color::White);
            renderer.endSpriteBatch();
            
            characterTarget_->unbind();
        }
        
        // ========== 第2步: 应用第一层特效 ==========
        if (effectTarget1_ && characterTarget_) {
            effectTarget1_->bind();
            effectTarget1_->clear(Color(0.0f, 0.0f, 0.0f, 0.0f));
            
            renderer.setViewProjection(ortho96);
            
            // 绘制发光粒子
            glowParticles_.draw(renderer);
            
            // 绘制角色
            renderer.beginSpriteBatch();
            auto charTex = characterTarget_->getColorTexture();
            renderer.drawSprite(*charTex, 
                               Rect(0, 0, 96, 96),
                               Rect(0, 0, 96, 96),
                               Color::White);
            renderer.endSpriteBatch();
            
            effectTarget1_->unbind();
        }
        
        // ========== 第3步: 最终合成 ==========
        if (finalTarget_ && effectTarget1_) {
            finalTarget_->bind();
            finalTarget_->clear(Color(0.1f, 0.1f, 0.2f, 1.0f));
            
            renderer.setViewProjection(ortho96);
            
            // 绘制烟雾粒子
            smokeParticles_.draw(renderer);
            
            // 绘制特效层
            renderer.beginSpriteBatch();
            auto effTex = effectTarget1_->getColorTexture();
            renderer.drawSprite(*effTex,
                               Rect(0, 0, 96, 96),
                               Rect(0, 0, 96, 96),
                               Color::White);
            renderer.endSpriteBatch();
            
            finalTarget_->unbind();
        }
        
        // 恢复屏幕投影
        glm::mat4 orthoScreen = glm::ortho(0.0f, 900.0f, 600.0f, 0.0f, -1.0f, 1.0f);
        renderer.setViewProjection(orthoScreen);
        renderer.setViewport(0, 0, 900, 600);
    }
};
```

---

## 6. 显示渲染目标内容

### 6.1 创建显示精灵
```cpp
void createDisplaySprites() {
    float displayY = 180.0f;
    
    // 原始角色显示
    if (characterTarget_ && characterTarget_->getColorTexture()) {
        originalSprite_ = Sprite::create(characterTarget_->getColorTexture());
        originalSprite_->setPosition(Vec2(150.0f, displayY));
        originalSprite_->setScale(1.0f);
        originalSprite_->setAnchor(Vec2(0.5f, 0.5f));
        addChild(originalSprite_);
    }
    
    // 特效层显示
    if (effectTarget1_ && effectTarget1_->getColorTexture()) {
        effect1Sprite_ = Sprite::create(effectTarget1_->getColorTexture());
        effect1Sprite_->setPosition(Vec2(450.0f, displayY));
        effect1Sprite_->setScale(1.0f);
        effect1Sprite_->setAnchor(Vec2(0.5f, 0.5f));
        addChild(effect1Sprite_);
    }
    
    // 最终合成显示
    if (finalTarget_ && finalTarget_->getColorTexture()) {
        finalSprite_ = Sprite::create(finalTarget_->getColorTexture());
        finalSprite_->setPosition(Vec2(750.0f, displayY));
        finalSprite_->setScale(1.0f);
        finalSprite_->setAnchor(Vec2(0.5f, 0.5f));
        addChild(finalSprite_);
    }
}
```

---

## 7. 完整示例

```cpp
#include <easy2d/easy2d.h>
#include <easy2d/graphics/render_target.h>
#include <easy2d/utils/logger.h>
#include <glm/gtc/matrix_transform.hpp>
#include <sstream>

using namespace easy2d;

// ============================================================================
// 渲染目标演示场景
// ============================================================================
class RenderTargetDemoScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        setBackgroundColor(Color(0.05f, 0.05f, 0.1f, 1.0f));
        
        // 创建渲染目标
        createRenderTargets();
        
        // 加载字体
        auto& resources = Application::instance().resources();
        font_ = resources.loadFont("C:/Windows/Fonts/simsun.ttc", 18);
        
        // 创建显示精灵
        createDisplaySprites();
        
        // 初始化动画
        time_ = 0.0f;
    }
    
    void onUpdate(float dt) override {
        Scene::onUpdate(dt);
        time_ += dt;
        
        // 执行离屏渲染
        performOffscreenRender();
    }
    
    void onRender(RenderBackend& renderer) override {
        Scene::onRender(renderer);
        
        // 绘制UI
        drawUI(renderer);
    }

private:
    Ptr<FontAtlas> font_;
    Ptr<RenderTarget> offscreenTarget_;
    Ptr<Sprite> displaySprite_;
    float time_ = 0.0f;
    
    void createRenderTargets() {
        RenderTargetConfig rtConfig;
        rtConfig.width = 256;
        rtConfig.height = 256;
        rtConfig.hasDepth = false;
        
        offscreenTarget_ = RenderTarget::createFromConfig(rtConfig);
        
        if (offscreenTarget_) {
            E2D_INFO("渲染目标创建完成: 256x256");
        }
    }
    
    void createDisplaySprites() {
        if (offscreenTarget_ && offscreenTarget_->getColorTexture()) {
            displaySprite_ = Sprite::create(offscreenTarget_->getColorTexture());
            displaySprite_->setPosition(Vec2(400.0f, 300.0f));
            displaySprite_->setScale(1.5f);
            addChild(displaySprite_);
        }
    }
    
    void performOffscreenRender() {
        if (!offscreenTarget_) return;
        
        auto& renderer = Application::instance().renderer();
        
        // 绑定渲染目标
        offscreenTarget_->bind();
        offscreenTarget_->clear(Color(0.1f, 0.1f, 0.2f, 1.0f));
        
        // 设置投影矩阵
        glm::mat4 ortho = glm::ortho(0.0f, 256.0f, 256.0f, 0.0f, -1.0f, 1.0f);
        renderer.setViewProjection(ortho);
        
        // 绘制旋转的彩色矩形
        float centerX = 128.0f;
        float centerY = 128.0f;
        
        for (int i = 0; i < 6; ++i) {
            float angle = time_ + i * 3.14159f / 3;
            float x = centerX + std::cos(angle) * 60.0f;
            float y = centerY + std::sin(angle) * 60.0f;
            
            float r = 0.5f + 0.5f * std::sin(i * 1.0f);
            float g = 0.5f + 0.5f * std::sin(i * 1.0f + 2.0f);
            float b = 0.5f + 0.5f * std::sin(i * 1.0f + 4.0f);
            
            renderer.fillRect(Rect(x - 20, y - 20, 40, 40), 
                            Color(r, g, b, 0.8f));
        }
        
        // 绘制中心圆
        renderer.fillCircle(Vec2(centerX, centerY), 40.0f,
                           Color(1.0f, 0.8f, 0.2f, 0.9f));
        
        // 解绑
        offscreenTarget_->unbind();
        
        // 恢复屏幕投影
        glm::mat4 screenOrtho = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f);
        renderer.setViewProjection(screenOrtho);
        renderer.setViewport(0, 0, 800, 600);
    }
    
    void drawUI(RenderBackend& renderer) {
        if (!font_) return;
        
        // 标题
        renderer.drawText(*font_, "渲染目标(离屏渲染)演示",
                         Vec2(280.0f, 30.0f), Color(1.0f, 0.9f, 0.2f, 1.0f));
        
        // 说明
        renderer.drawText(*font_, "左侧内容渲染到离屏目标，然后通过精灵显示",
                         Vec2(180.0f, 500.0f), Color(0.8f, 0.8f, 0.8f, 1.0f));
        
        // FPS
        std::stringstream ss;
        ss << "FPS: " << Application::instance().fps();
        renderer.drawText(*font_, ss.str(),
                         Vec2(700.0f, 560.0f), Color(0.5f, 1.0f, 0.5f, 1.0f));
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
    config.title = "渲染目标演示";
    config.width = 800;
    config.height = 600;
    config.vsync = true;
    
    if (!app.init(config)) {
        E2D_LOG_ERROR("初始化失败！");
        return -1;
    }
    
    app.enterScene(makePtr<RenderTargetDemoScene>());
    app.run();
    
    Logger::shutdown();
    return 0;
}
```
