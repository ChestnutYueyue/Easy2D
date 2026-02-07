/**
 * Easy2D 酷炫特效演示
 * 
 * 特效列表：
 * 1. 彩虹渐变 - 颜色随时间流动变化
 * 2. 脉冲发光 - 有节奏地闪烁扩大
 * 3. 粒子爆炸 - 点击产生爆炸效果
 * 4. 魔法光环 - 旋转的光环效果
 * 5. 雨滴效果 - 模拟下雨
 */

#include <easy2d/easy2d.h>
#include <iostream>
#include <sstream>
#include <cmath>

using namespace easy2d;

// ============================================================================
// 酷炫特效场景
// ============================================================================
class CoolEffectsScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        
        setBackgroundColor(Color(0.02f, 0.02f, 0.05f, 1.0f));
        
        // 初始化特效管理器
        E2D_CUSTOM_EFFECT_MANAGER().init();
        
        // 创建酷炫特效配置
        createCoolEffects();
        
        // 播放特效
        playEffects();
        
        // 加载字体
        auto& resources = Application::instance().resources();
        font_ = resources.loadFont("C:/Windows/Fonts/simsun.ttc", 18);
    }
    
    void onExit() override {
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
        
        // 脉冲效果
        updatePulseEffect();
        
        // 检测鼠标输入
        checkMouseInput();
    }
    
    void checkMouseInput() {
        auto& input = Application::instance().input();
        
        // 检测鼠标左键按下
        if (input.isMousePressed(MouseButton::Left)) {
            auto pos = input.getMousePosition();
            createExplosion(pos);
        }
    }

private:
    float time_ = 0.0f;
    Ptr<FontAtlas> font_;
    std::vector<Ptr<CustomEffect>> activeEffects_;
    
    void createCoolEffects() {
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
        
        // 3. 魔法光环特效
        auto magicRingConfig = EffectBuilder::Particle("MagicRing");
        magicRingConfig.emitterConfig.emissionRate = 100;
        magicRingConfig.emitterConfig.minLife = 1.5f;
        magicRingConfig.emitterConfig.maxLife = 2.5f;
        magicRingConfig.emitterConfig.minStartSize = 5.0f;
        magicRingConfig.emitterConfig.maxStartSize = 10.0f;
        magicRingConfig.emitterConfig.minEndSize = 2.0f;
        magicRingConfig.emitterConfig.maxEndSize = 5.0f;
        magicRingConfig.emitterConfig.shape = EmitterConfig::Shape::Circle;
        magicRingConfig.emitterConfig.shapeRadius = 80.0f;
        magicRingConfig.emitterConfig.minVelocity = Vec2(-20.0f, -20.0f);
        magicRingConfig.emitterConfig.maxVelocity = Vec2(20.0f, 20.0f);
        magicRingConfig.emitterConfig.startColor = Color(1.0f, 0.8f, 0.2f, 1.0f);
        magicRingConfig.emitterConfig.endColor = Color(0.8f, 0.2f, 0.8f, 0.0f);
        magicRingConfig.emitterConfig.blendMode = BlendMode::Additive;
        E2D_CUSTOM_EFFECT_MANAGER().registerConfig("MagicRing", magicRingConfig);
        
        // 4. 雨滴特效
        auto rainConfig = EffectBuilder::Particle("CoolRain");
        rainConfig.emitterConfig.emissionRate = 500;
        rainConfig.emitterConfig.minLife = 0.8f;
        rainConfig.emitterConfig.maxLife = 1.2f;
        rainConfig.emitterConfig.minStartSize = 2.0f;
        rainConfig.emitterConfig.maxStartSize = 4.0f;
        rainConfig.emitterConfig.minEndSize = 2.0f;
        rainConfig.emitterConfig.maxEndSize = 4.0f;
        rainConfig.emitterConfig.minVelocity = Vec2(-100.0f, 400.0f);
        rainConfig.emitterConfig.maxVelocity = Vec2(100.0f, 600.0f);
        rainConfig.emitterConfig.acceleration = Vec2(0.0f, 200.0f);
        rainConfig.emitterConfig.startColor = Color(0.4f, 0.7f, 1.0f, 0.6f);
        rainConfig.emitterConfig.endColor = Color(0.2f, 0.5f, 0.8f, 0.2f);
        rainConfig.emitterConfig.blendMode = BlendMode::Alpha;
        E2D_CUSTOM_EFFECT_MANAGER().registerConfig("CoolRain", rainConfig);
        
        // 5. 星空闪烁特效
        auto starConfig = EffectBuilder::Particle("StarTwinkle");
        starConfig.emitterConfig.emissionRate = 20;
        starConfig.emitterConfig.minLife = 1.0f;
        starConfig.emitterConfig.maxLife = 2.0f;
        starConfig.emitterConfig.minStartSize = 2.0f;
        starConfig.emitterConfig.maxStartSize = 5.0f;
        starConfig.emitterConfig.minEndSize = 0.0f;
        starConfig.emitterConfig.maxEndSize = 2.0f;
        starConfig.emitterConfig.minVelocity = Vec2(-10.0f, -10.0f);
        starConfig.emitterConfig.maxVelocity = Vec2(10.0f, 10.0f);
        starConfig.emitterConfig.startColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
        starConfig.emitterConfig.endColor = Color(1.0f, 1.0f, 0.8f, 0.0f);
        starConfig.emitterConfig.blendMode = BlendMode::Additive;
        E2D_CUSTOM_EFFECT_MANAGER().registerConfig("StarTwinkle", starConfig);
        
        // 6. 爆炸特效（用于点击）
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
    }
    
    void playEffects() {
        // 彩虹流 - 左上方
        auto rainbow = E2D_CUSTOM_EFFECT_MANAGER().play("RainbowFlow", Vec2(200.0f, 450.0f));
        if (rainbow) activeEffects_.push_back(rainbow);
        
        // 脉冲发光 - 中间
        auto pulse = E2D_CUSTOM_EFFECT_MANAGER().play("PulseGlow", Vec2(400.0f, 300.0f));
        if (pulse) activeEffects_.push_back(pulse);
        
        // 魔法光环 - 右上方
        auto magic = E2D_CUSTOM_EFFECT_MANAGER().play("MagicRing", Vec2(600.0f, 450.0f));
        if (magic) activeEffects_.push_back(magic);
        
        // 雨滴 - 全屏
        for (int i = 0; i < 5; ++i) {
            float x = 100.0f + i * 150.0f;
            auto rain = E2D_CUSTOM_EFFECT_MANAGER().play("CoolRain", Vec2(x, 50.0f));
            if (rain) activeEffects_.push_back(rain);
        }
        
        // 星空 - 随机位置
        for (int i = 0; i < 10; ++i) {
            float x = 50.0f + (i % 5) * 180.0f;
            float y = 100.0f + (i / 5) * 150.0f;
            auto star = E2D_CUSTOM_EFFECT_MANAGER().play("StarTwinkle", Vec2(x, y));
            if (star) activeEffects_.push_back(star);
        }
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
    
    void updatePulseEffect() {
        // 脉冲效果 - 改变发射速率
        auto config = E2D_CUSTOM_EFFECT_MANAGER().getConfig("PulseGlow");
        if (config) {
            float pulse = 50.0f + 30.0f * sin(time_ * 3.0f);
            config->emitterConfig.emissionRate = pulse;
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
    
    // HSV 转 RGB
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
    
    void onRender(RenderBackend& renderer) override {
        Scene::onRender(renderer);
        
        // 渲染特效
        E2D_CUSTOM_EFFECT_MANAGER().render(renderer);
        
        // 绘制文字
        if (font_) {
            // 标题
            renderer.drawText(*font_, "Easy2D 特效演示", 
                            Vec2(300.0f, 15.0f), Color(1.0f, 0.9f, 0.2f, 1.0f));
            
            // 特效说明
            renderer.drawText(*font_, "彩虹流(左上)  脉冲发光(中)  魔法光环(右上)", 
                            Vec2(180.0f, 45.0f), Color(0.8f, 0.9f, 1.0f, 1.0f));
            
            renderer.drawText(*font_, "雨滴(顶部)  星空闪烁(随机)  点击产生爆炸", 
                            Vec2(200.0f, 70.0f), Color(0.7f, 0.85f, 1.0f, 1.0f));
            
            // FPS
            auto fps = Application::instance().fps();
            std::stringstream ss;
            ss << "FPS: " << fps;
            renderer.drawText(*font_, ss.str(), Vec2(20.0f, 570.0f), 
                            Color(0.5f, 1.0f, 0.5f, 1.0f));
        }
    }
};

// ============================================================================
// 主函数
// ============================================================================
int main() {
    Logger::init();
    Logger::setLevel(LogLevel::Info);

    E2D_LOG_INFO("========================");
    E2D_LOG_INFO("Easy2D 酷炫特效演示");
    E2D_LOG_INFO("========================");

    auto& app = Application::instance();

    AppConfig config;
    config.title = "Easy2D 酷炫特效演示";
    config.width = 800;
    config.height = 600;
    config.vsync = true;

    if (!app.init(config)) {
        E2D_LOG_ERROR("初始化应用失败！");
        return -1;
    }

    app.enterScene(makePtr<CoolEffectsScene>());

    E2D_LOG_INFO("启动主循环...");
    app.run();

    E2D_LOG_INFO("应用结束。");
    Logger::shutdown();

    return 0;
}
