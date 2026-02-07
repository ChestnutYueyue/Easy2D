# Easy2D 自定义特效系统 API 文档

## 1. 特效系统概述

自定义特效系统允许你创建和管理粒子特效，支持从JSON文件加载配置或通过代码创建。

### 1.1 头文件
```cpp
#include <easy2d/easy2d.h>
```

### 1.2 获取特效管理器
```cpp
// 使用宏获取自定义特效管理器
auto& effectManager = E2D_CUSTOM_EFFECT_MANAGER();
```

---

## 2. 初始化和关闭

### 2.1 初始化特效系统
```cpp
void onEnter() override {
    Scene::onEnter();
    
    // 初始化特效管理器
    E2D_CUSTOM_EFFECT_MANAGER().init();
    
    // 创建特效配置
    createEffects();
    
    // 播放特效
    playEffects();
}
```

### 2.2 关闭特效系统
```cpp
void onExit() override {
    // 停止所有特效
    E2D_CUSTOM_EFFECT_MANAGER().stopAll();
    
    // 关闭特效管理器
    E2D_CUSTOM_EFFECT_MANAGER().shutdown();
    
    Scene::onExit();
}
```

---

## 3. 创建特效配置

### 3.1 使用EffectBuilder创建
```cpp
void createEffects() {
    // 1. 火焰特效
    auto fireConfig = EffectBuilder::Fire("MyFire");
    fireConfig.description = "火焰效果";
    E2D_CUSTOM_EFFECT_MANAGER().registerConfig("MyFire", fireConfig);
    
    // 2. 烟雾特效
    auto smokeConfig = EffectBuilder::Smoke("MySmoke");
    smokeConfig.description = "烟雾效果";
    E2D_CUSTOM_EFFECT_MANAGER().registerConfig("MySmoke", smokeConfig);
    
    // 3. 魔法特效
    auto magicConfig = EffectBuilder::Magic("MyMagic");
    magicConfig.description = "魔法效果";
    E2D_CUSTOM_EFFECT_MANAGER().registerConfig("MyMagic", magicConfig);
}
```

### 3.2 自定义粒子特效
```cpp
void createCustomEffects() {
    // 创建自定义粒子特效
    auto customParticle = EffectBuilder::Particle("MyCustomParticle");
    customParticle.description = "完全自定义的粒子效果";
    
    // 发射器配置
    customParticle.emitterConfig.emissionRate = 200.0f;    // 发射速率
    customParticle.emitterConfig.minLife = 0.5f;           // 最小生命周期
    customParticle.emitterConfig.maxLife = 1.5f;           // 最大生命周期
    customParticle.emitterConfig.minStartSize = 5.0f;      // 最小起始大小
    customParticle.emitterConfig.maxStartSize = 15.0f;     // 最大起始大小
    customParticle.emitterConfig.minEndSize = 2.0f;        // 最小结束大小
    customParticle.emitterConfig.maxEndSize = 5.0f;        // 最大结束大小
    customParticle.emitterConfig.minVelocity = Vec2(-100.0f, -100.0f);  // 最小速度
    customParticle.emitterConfig.maxVelocity = Vec2(100.0f, -50.0f);    // 最大速度
    customParticle.emitterConfig.startColor = Color(0.0f, 1.0f, 0.5f, 1.0f);  // 起始颜色
    customParticle.emitterConfig.endColor = Color(0.0f, 0.5f, 1.0f, 0.0f);    // 结束颜色
    customParticle.emitterConfig.blendMode = BlendMode::Additive;  // 混合模式
    
    // 注册配置
    E2D_CUSTOM_EFFECT_MANAGER().registerConfig("MyCustomParticle", customParticle);
}
```

### 3.3 高级特效配置
```cpp
void createAdvancedEffects() {
    // 彩虹渐变特效
    auto rainbowConfig = EffectBuilder::Particle("RainbowFlow");
    rainbowConfig.emitterConfig.emissionRate = 200;
    rainbowConfig.emitterConfig.minLife = 2.0f;
    rainbowConfig.emitterConfig.maxLife = 3.0f;
    rainbowConfig.emitterConfig.minStartSize = 20.0f;
    rainbowConfig.emitterConfig.maxStartSize = 40.0f;
    rainbowConfig.emitterConfig.minEndSize = 10.0f;
    rainbowConfig.emitterConfig.maxEndSize = 20.0f;
    rainbowConfig.emitterConfig.minVelocity = Vec2(-30.0f, -50.0f);
    rainbowConfig.emitterConfig.maxVelocity = Vec2(30.0f, -100.0f);
    rainbowConfig.emitterConfig.startColor = Color(1.0f, 0.0f, 0.0f, 1.0f);
    rainbowConfig.emitterConfig.endColor = Color(1.0f, 0.0f, 1.0f, 0.0f);
    rainbowConfig.emitterConfig.blendMode = BlendMode::Additive;
    E2D_CUSTOM_EFFECT_MANAGER().registerConfig("RainbowFlow", rainbowConfig);
    
    // 魔法光环特效
    auto magicRingConfig = EffectBuilder::Particle("MagicRing");
    magicRingConfig.emitterConfig.emissionRate = 100;
    magicRingConfig.emitterConfig.minLife = 1.5f;
    magicRingConfig.emitterConfig.maxLife = 2.5f;
    magicRingConfig.emitterConfig.minStartSize = 5.0f;
    magicRingConfig.emitterConfig.maxStartSize = 10.0f;
    magicRingConfig.emitterConfig.minEndSize = 2.0f;
    magicRingConfig.emitterConfig.maxEndSize = 5.0f;
    magicRingConfig.emitterConfig.shape = EmitterConfig::Shape::Circle;  // 圆形发射
    magicRingConfig.emitterConfig.shapeRadius = 80.0f;                   // 发射半径
    magicRingConfig.emitterConfig.minVelocity = Vec2(-20.0f, -20.0f);
    magicRingConfig.emitterConfig.maxVelocity = Vec2(20.0f, 20.0f);
    magicRingConfig.emitterConfig.startColor = Color(1.0f, 0.8f, 0.2f, 1.0f);
    magicRingConfig.emitterConfig.endColor = Color(0.8f, 0.2f, 0.8f, 0.0f);
    magicRingConfig.emitterConfig.blendMode = BlendMode::Additive;
    E2D_CUSTOM_EFFECT_MANAGER().registerConfig("MagicRing", magicRingConfig);
    
    // 爆炸特效（一次性）
    auto explosionConfig = EffectBuilder::Particle("Explosion");
    explosionConfig.emitterConfig.emissionRate = 300;
    explosionConfig.duration = 0.3f;      // 持续时间
    explosionConfig.loop = false;          // 不循环
    explosionConfig.emitterConfig.minLife = 0.3f;
    explosionConfig.emitterConfig.maxLife = 0.6f;
    explosionConfig.emitterConfig.minStartSize = 10.0f;
    explosionConfig.emitterConfig.maxStartSize = 20.0f;
    explosionConfig.emitterConfig.minEndSize = 40.0f;
    explosionConfig.emitterConfig.maxEndSize = 80.0f;
    explosionConfig.emitterConfig.minVelocity = Vec2(-250.0f, -250.0f);
    explosionConfig.emitterConfig.maxVelocity = Vec2(250.0f, 250.0f);
    explosionConfig.emitterConfig.startColor = Color(1.0f, 0.9f, 0.2f, 1.0f);
    explosionConfig.emitterConfig.endColor = Color(1.0f, 0.2f, 0.0f, 0.0f);
    explosionConfig.emitterConfig.blendMode = BlendMode::Additive;
    E2D_CUSTOM_EFFECT_MANAGER().registerConfig("Explosion", explosionConfig);
}
```

---

## 4. 播放和管理特效

### 4.1 播放特效
```cpp
void playEffects() {
    // 在指定位置播放特效
    E2D_CUSTOM_EFFECT_MANAGER().play("MyFire", Vec2(150.0f, 450.0f));
    E2D_CUSTOM_EFFECT_MANAGER().play("MySmoke", Vec2(650.0f, 450.0f));
    E2D_CUSTOM_EFFECT_MANAGER().play("MyMagic", Vec2(400.0f, 300.0f));
}

// 保存特效引用以便后续控制
Ptr<CustomEffect> effect_;

void playAndStore() {
    effect_ = E2D_CUSTOM_EFFECT_MANAGER().play("MyFire", Vec2(400.0f, 300.0f));
}
```

### 4.2 停止特效
```cpp
// 停止所有特效
E2D_CUSTOM_EFFECT_MANAGER().stopAll();

// 停止特定特效（如果有引用）
if (effect_) {
    effect_->stop();
}
```

### 4.3 从JSON文件加载
```cpp
void initCustomEffects() {
    // 初始化
    E2D_CUSTOM_EFFECT_MANAGER().init();
    
    // 从JSON文件加载特效配置
    if (E2D_CUSTOM_EFFECT_MANAGER().loadFromFile("assets/effects/custom_effects.json")) {
        E2D_LOG_INFO("成功加载自定义特效配置");
    } else {
        E2D_LOG_WARN("无法加载自定义特效配置文件，将使用代码创建");
        // 使用代码创建备用特效
        createEffectsFromCode();
    }
}
```

---

## 5. 更新和渲染

### 5.1 在场景中使用
```cpp
void onUpdate(float dt) override {
    Scene::onUpdate(dt);
    
    // 更新特效管理器
    E2D_CUSTOM_EFFECT_MANAGER().update(dt);
    
    // 动态改变特效参数
    updateRainbowEffect();
}

void onRender(RenderBackend& renderer) override {
    Scene::onRender(renderer);
    
    // 渲染特效
    E2D_CUSTOM_EFFECT_MANAGER().render(renderer);
}
```

### 5.2 动态修改特效
```cpp
void updateRainbowEffect() {
    // 动态改变彩虹颜色
    auto config = E2D_CUSTOM_EFFECT_MANAGER().getConfig("RainbowFlow");
    if (config) {
        float hue = fmod(time_ * 0.5f, 1.0f);
        config->emitterConfig.startColor = hsvToRgb(hue, 1.0f, 1.0f, 1.0f);
        config->emitterConfig.endColor = hsvToRgb(fmod(hue + 0.5f, 1.0f), 1.0f, 1.0f, 0.0f);
    }
}

// HSV转RGB辅助函数
Color hsvToRgb(float h, float s, float v, float a) {
    float r, g, b;
    
    int i = static_cast<int>(h * 6);
    float f = h * 6 - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);
    
    switch (i % 6) {
        case 0: r = v, g = t, b = p; break;
        case 1: r = q, g = v, b = p; break;
        case 2: r = p, g = v, b = t; break;
        case 3: r = p, g = q, b = v; break;
        case 4: r = t, g = p, b = v; break;
        case 5: r = v, g = p, b = q; break;
    }
    
    return Color(r, g, b, a);
}
```

---

## 6. 获取特效信息

### 6.1 查询特效配置
```cpp
// 获取所有配置名称
auto effectNames = E2D_CUSTOM_EFFECT_MANAGER().getConfigNames();

// 获取特定配置
auto config = E2D_CUSTOM_EFFECT_MANAGER().getConfig("MyFire");
if (config) {
    E2D_INFO("特效描述: {}", config->description);
}
```

---

## 7. 完整示例

```cpp
#include <easy2d/easy2d.h>
#include <easy2d/utils/logger.h>
#include <sstream>
#include <cmath>

using namespace easy2d;

// ============================================================================
// 自定义特效演示场景
// ============================================================================
class CustomEffectsDemoScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        setBackgroundColor(Color(0.02f, 0.02f, 0.05f, 1.0f));
        
        // 初始化特效系统
        initCustomEffects();
        
        // 创建特效
        createEffects();
        
        // 播放特效
        playEffects();
        
        // 加载字体
        auto& resources = Application::instance().resources();
        font_ = resources.loadFont("C:/Windows/Fonts/simsun.ttc", 18);
    }
    
    void onExit() override {
        E2D_CUSTOM_EFFECT_MANAGER().stopAll();
        E2D_CUSTOM_EFFECT_MANAGER().shutdown();
        Scene::onExit();
    }
    
    void onUpdate(float dt) override {
        Scene::onUpdate(dt);
        
        time_ += dt;
        
        // 更新特效管理器
        E2D_CUSTOM_EFFECT_MANAGER().update(dt);
        
        // 动态改变彩虹特效颜色
        updateRainbowEffect();
        
        // 检测鼠标输入
        checkMouseInput();
    }
    
    void onRender(RenderBackend& renderer) override {
        Scene::onRender(renderer);
        
        // 渲染特效
        E2D_CUSTOM_EFFECT_MANAGER().render(renderer);
        
        // 绘制UI
        drawUI(renderer);
    }

private:
    float time_ = 0.0f;
    Ptr<FontAtlas> font_;
    std::vector<Ptr<CustomEffect>> activeEffects_;
    
    void initCustomEffects() {
        E2D_CUSTOM_EFFECT_MANAGER().init();
        E2D_INFO("特效系统初始化完成");
    }
    
    void createEffects() {
        // 1. 彩虹渐变特效
        auto rainbowConfig = EffectBuilder::Particle("RainbowFlow");
        rainbowConfig.emitterConfig.emissionRate = 200;
        rainbowConfig.emitterConfig.minLife = 2.0f;
        rainbowConfig.emitterConfig.maxLife = 3.0f;
        rainbowConfig.emitterConfig.minStartSize = 20.0f;
        rainbowConfig.emitterConfig.maxStartSize = 40.0f;
        rainbowConfig.emitterConfig.minEndSize = 10.0f;
        rainbowConfig.emitterConfig.maxEndSize = 20.0f;
        rainbowConfig.emitterConfig.minVelocity = Vec2(-30.0f, -50.0f);
        rainbowConfig.emitterConfig.maxVelocity = Vec2(30.0f, -100.0f);
        rainbowConfig.emitterConfig.startColor = Color(1.0f, 0.0f, 0.0f, 1.0f);
        rainbowConfig.emitterConfig.endColor = Color(1.0f, 0.0f, 1.0f, 0.0f);
        rainbowConfig.emitterConfig.blendMode = BlendMode::Additive;
        E2D_CUSTOM_EFFECT_MANAGER().registerConfig("RainbowFlow", rainbowConfig);
        
        // 2. 脉冲发光特效
        auto pulseConfig = EffectBuilder::Particle("PulseGlow");
        pulseConfig.emitterConfig.emissionRate = 50;
        pulseConfig.emitterConfig.minLife = 1.0f;
        pulseConfig.emitterConfig.maxLife = 1.5f;
        pulseConfig.emitterConfig.minStartSize = 10.0f;
        pulseConfig.emitterConfig.maxStartSize = 20.0f;
        pulseConfig.emitterConfig.minEndSize = 60.0f;
        pulseConfig.emitterConfig.maxEndSize = 100.0f;
        pulseConfig.emitterConfig.minVelocity = Vec2(0.0f, 0.0f);
        pulseConfig.emitterConfig.maxVelocity = Vec2(0.0f, 0.0f);
        pulseConfig.emitterConfig.startColor = Color(0.0f, 1.0f, 1.0f, 0.8f);
        pulseConfig.emitterConfig.endColor = Color(0.0f, 0.5f, 1.0f, 0.0f);
        pulseConfig.emitterConfig.blendMode = BlendMode::Additive;
        E2D_CUSTOM_EFFECT_MANAGER().registerConfig("PulseGlow", pulseConfig);
        
        // 3. 爆炸特效（用于点击）
        auto explosionConfig = EffectBuilder::Particle("ClickExplosion");
        explosionConfig.emitterConfig.emissionRate = 300;
        explosionConfig.duration = 0.3f;
        explosionConfig.loop = false;
        explosionConfig.emitterConfig.minLife = 0.3f;
        explosionConfig.emitterConfig.maxLife = 0.6f;
        explosionConfig.emitterConfig.minStartSize = 10.0f;
        explosionConfig.emitterConfig.maxStartSize = 20.0f;
        explosionConfig.emitterConfig.minEndSize = 40.0f;
        explosionConfig.emitterConfig.maxEndSize = 80.0f;
        explosionConfig.emitterConfig.minVelocity = Vec2(-250.0f, -250.0f);
        explosionConfig.emitterConfig.maxVelocity = Vec2(250.0f, 250.0f);
        explosionConfig.emitterConfig.startColor = Color(1.0f, 0.9f, 0.2f, 1.0f);
        explosionConfig.emitterConfig.endColor = Color(1.0f, 0.2f, 0.0f, 0.0f);
        explosionConfig.emitterConfig.blendMode = BlendMode::Additive;
        E2D_CUSTOM_EFFECT_MANAGER().registerConfig("ClickExplosion", explosionConfig);
        
        E2D_INFO("特效配置创建完成");
    }
    
    void playEffects() {
        // 彩虹流 - 左上方
        auto rainbow = E2D_CUSTOM_EFFECT_MANAGER().play("RainbowFlow", Vec2(200.0f, 450.0f));
        if (rainbow) activeEffects_.push_back(rainbow);
        
        // 脉冲发光 - 中间
        auto pulse = E2D_CUSTOM_EFFECT_MANAGER().play("PulseGlow", Vec2(400.0f, 300.0f));
        if (pulse) activeEffects_.push_back(pulse);
        
        E2D_INFO("特效播放完成");
    }
    
    void updateRainbowEffect() {
        // 动态改变彩虹颜色
        auto config = E2D_CUSTOM_EFFECT_MANAGER().getConfig("RainbowFlow");
        if (config) {
            float hue = fmod(time_ * 0.5f, 1.0f);
            config->emitterConfig.startColor = hsvToRgb(hue, 1.0f, 1.0f, 1.0f);
            config->emitterConfig.endColor = hsvToRgb(fmod(hue + 0.5f, 1.0f), 1.0f, 1.0f, 0.0f);
        }
    }
    
    void checkMouseInput() {
        auto& input = Application::instance().input();
        
        // 检测鼠标左键按下
        if (input.isMousePressed(MouseButton::Left)) {
            auto pos = input.getMousePosition();
            createExplosion(pos);
        }
    }
    
    void createExplosion(const Vec2& pos) {
        auto explosion = E2D_CUSTOM_EFFECT_MANAGER().play("ClickExplosion", pos);
        if (explosion) {
            // 随机颜色
            float hue = fmod(time_ * 0.3f, 1.0f);
            auto config = E2D_CUSTOM_EFFECT_MANAGER().getConfig("ClickExplosion");
            if (config) {
                config->emitterConfig.startColor = hsvToRgb(hue, 1.0f, 1.0f, 1.0f);
                config->emitterConfig.endColor = hsvToRgb(fmod(hue + 0.3f, 1.0f), 1.0f, 0.5f, 0.0f);
            }
        }
    }
    
    Color hsvToRgb(float h, float s, float v, float a) {
        float r, g, b;
        
        int i = static_cast<int>(h * 6);
        float f = h * 6 - i;
        float p = v * (1 - s);
        float q = v * (1 - f * s);
        float t = v * (1 - (1 - f) * s);
        
        switch (i % 6) {
            case 0: r = v, g = t, b = p; break;
            case 1: r = q, g = v, b = p; break;
            case 2: r = p, g = v, b = t; break;
            case 3: r = p, g = q, b = v; break;
            case 4: r = t, g = p, b = v; break;
            case 5: r = v, g = p, b = q; break;
        }
        
        return Color(r, g, b, a);
    }
    
    void drawUI(RenderBackend& renderer) {
        if (!font_) return;
        
        // 标题
        renderer.drawText(*font_, "Easy2D 自定义特效演示", 
                        Vec2(280.0f, 15.0f), Color(1.0f, 0.9f, 0.2f, 1.0f));
        
        // 特效说明
        renderer.drawText(*font_, "彩虹流(左上)  脉冲发光(中)", 
                        Vec2(250.0f, 45.0f), Color(0.8f, 0.9f, 1.0f, 1.0f));
        
        renderer.drawText(*font_, "点击产生爆炸", 
                        Vec2(320.0f, 70.0f), Color(0.7f, 0.85f, 1.0f, 1.0f));
        
        // FPS
        auto fps = Application::instance().fps();
        std::stringstream ss;
        ss << "FPS: " << fps;
        renderer.drawText(*font_, ss.str(), Vec2(20.0f, 570.0f), 
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
    config.title = "自定义特效演示";
    config.width = 800;
    config.height = 600;
    config.vsync = true;
    
    if (!app.init(config)) {
        E2D_LOG_ERROR("初始化失败！");
        return -1;
    }
    
    app.enterScene(makePtr<CustomEffectsDemoScene>());
    app.run();
    
    Logger::shutdown();
    return 0;
}
```
