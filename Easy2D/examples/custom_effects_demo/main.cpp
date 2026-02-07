/**
 * Easy2D 自定义特效系统演示
 * 
 * 演示内容：
 * 1. 从JSON文件加载特效配置
 * 2. 使用代码创建自定义特效
 * 3. 播放和管理特效
 * 4. 保存特效配置到文件
 */

#include <easy2d/easy2d.h>
#include <easy2d/utils/logger.h>
#include <iostream>
#include <sstream>

using namespace easy2d;

// ============================================================================
// 自定义特效演示场景
// ============================================================================
class CustomEffectsDemoScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        
        // 设置背景色
        setBackgroundColor(Color(0.05f, 0.05f, 0.1f, 1.0f));
        
        // 初始化自定义特效系统
        initCustomEffects();
        
        // 创建UI说明
        createInstructions();
    }
    
    void onUpdate(float dt) override {
        Scene::onUpdate(dt);
        
        // 更新自定义特效管理器
        E2D_CUSTOM_EFFECT_MANAGER().update(dt);
    }
    
    void onRender(RenderBackend& renderer) override {
        Scene::onRender(renderer);
        
        // 渲染自定义特效
        E2D_CUSTOM_EFFECT_MANAGER().render(renderer);
        
        // 绘制说明文字
        drawInstructions(renderer);
    }
    
    void onExit() override {
        // 关闭自定义特效管理器
        E2D_CUSTOM_EFFECT_MANAGER().stopAll();
        Scene::onExit();
    }

private:
    Ptr<FontAtlas> font_;
    
    void initCustomEffects() {
        // 初始化自定义特效管理器
        E2D_CUSTOM_EFFECT_MANAGER().init();
        
        // 从JSON文件加载特效配置
        if (E2D_CUSTOM_EFFECT_MANAGER().loadFromFile("assets/effects/custom_effects.json")) {
            E2D_LOG_INFO("成功加载自定义特效配置");
        } else {
            E2D_LOG_WARN("无法加载自定义特效配置文件，将使用代码创建");
        }
        
        // 使用代码创建自定义特效配置
        createEffectsFromCode();
        
        // 播放一些特效
        playDemoEffects();
    }
    
    void createEffectsFromCode() {
        // 使用EffectBuilder快速创建特效配置
        auto fireConfig = EffectBuilder::Fire("CodeFire");
        fireConfig.description = "代码创建的火焰效果";
        E2D_CUSTOM_EFFECT_MANAGER().registerConfig("CodeFire", fireConfig);
        
        auto smokeConfig = EffectBuilder::Smoke("CodeSmoke");
        smokeConfig.description = "代码创建的烟雾效果";
        E2D_CUSTOM_EFFECT_MANAGER().registerConfig("CodeSmoke", smokeConfig);
        
        auto magicConfig = EffectBuilder::Magic("CodeMagic");
        magicConfig.description = "代码创建的魔法效果";
        E2D_CUSTOM_EFFECT_MANAGER().registerConfig("CodeMagic", magicConfig);
        
        // 创建自定义粒子特效
        auto customParticle = EffectBuilder::Particle("MyCustomParticle");
        customParticle.description = "完全自定义的粒子效果";
        customParticle.emitterConfig.emissionRate = 200.0f;
        customParticle.emitterConfig.minLife = 0.5f;
        customParticle.emitterConfig.maxLife = 1.5f;
        customParticle.emitterConfig.minStartSize = 5.0f;
        customParticle.emitterConfig.maxStartSize = 15.0f;
        customParticle.emitterConfig.minVelocity = Vec2(-100.0f, -100.0f);
        customParticle.emitterConfig.maxVelocity = Vec2(100.0f, -50.0f);
        customParticle.emitterConfig.startColor = Color(0.0f, 1.0f, 0.5f, 1.0f);  // 绿色
        customParticle.emitterConfig.endColor = Color(0.0f, 0.5f, 1.0f, 0.0f);    // 蓝色透明
        customParticle.emitterConfig.blendMode = BlendMode::Additive;
        E2D_CUSTOM_EFFECT_MANAGER().registerConfig("MyCustomParticle", customParticle);
        
        E2D_LOG_INFO("代码创建特效配置完成");
    }
    
    void playDemoEffects() {
        // 播放JSON文件中的特效
        E2D_CUSTOM_EFFECT_MANAGER().play("MyFire", Vec2(150, 450));
        E2D_CUSTOM_EFFECT_MANAGER().play("MySmoke", Vec2(650, 450));
        E2D_CUSTOM_EFFECT_MANAGER().play("MySparkle", Vec2(400, 300));
        
        // 播放代码创建的特效
        E2D_CUSTOM_EFFECT_MANAGER().play("CodeFire", Vec2(250, 450));
        E2D_CUSTOM_EFFECT_MANAGER().play("CodeSmoke", Vec2(550, 450));
        E2D_CUSTOM_EFFECT_MANAGER().play("MyCustomParticle", Vec2(400, 400));
    }
    
    void createInstructions() {
        auto& resources = Application::instance().resources();
        
        // 尝试加载系统字体
        std::vector<std::string> fontPaths = {
            "C:/Windows/Fonts/simsun.ttc",
            "C:/Windows/Fonts/seguiemj.ttf",
            "C:/Windows/Fonts/arial.ttf",
        };
        
        for (const auto& path : fontPaths) {
            font_ = resources.loadFont(path, 18);
            if (font_) {
                E2D_LOG_INFO("成功加载字体: {}", path);
                break;
            }
        }
    }
    
    void drawInstructions(RenderBackend& renderer) {
        if (!font_) return;
        
        // 绘制标题
        renderer.drawText(*font_, "Easy2D 自定义特效系统演示", Vec2(220.0f, 20.0f),
                         Color(1.0f, 1.0f, 1.0f, 1.0f));
        
        // 绘制说明
        renderer.drawText(*font_, "JSON特效: MyFire(左), MySmoke(右), MySparkle(中)", 
                         Vec2(20.0f, 60.0f), Color(0.8f, 0.8f, 0.8f, 1.0f));
        renderer.drawText(*font_, "代码特效: CodeFire, CodeSmoke, MyCustomParticle", 
                         Vec2(20.0f, 85.0f), Color(0.8f, 0.8f, 0.8f, 1.0f));
        
        // 绘制可用特效列表
        auto effectNames = E2D_CUSTOM_EFFECT_MANAGER().getConfigNames();
        std::stringstream ss;
        ss << "可用特效 (" << effectNames.size() << "个): ";
        for (size_t i = 0; i < effectNames.size() && i < 5; ++i) {
            if (i > 0) ss << ", ";
            ss << effectNames[i];
        }
        if (effectNames.size() > 5) ss << "...";
        renderer.drawText(*font_, ss.str(), Vec2(20.0f, 110.0f), Color(0.7f, 0.9f, 0.7f, 1.0f));
        
        // 绘制FPS
        auto& app = Application::instance();
        ss.str("");
        ss << "FPS: " << app.fps();
        renderer.drawText(*font_, ss.str(), Vec2(20.0f, 560.0f),
                         Color(0.8f, 1.0f, 0.8f, 1.0f));
        
        // 绘制操作提示
        renderer.drawText(*font_, "点击鼠标左键在点击位置播放爆炸效果", 
                         Vec2(20.0f, 530.0f), Color(1.0f, 0.9f, 0.7f, 1.0f));
    }
};

// ============================================================================
// 主函数
// ============================================================================
int main() {
    Logger::init();
    Logger::setLevel(LogLevel::Info);

    E2D_LOG_INFO("========================");
    E2D_LOG_INFO("Easy2D 自定义特效系统演示");
    E2D_LOG_INFO("========================");

    auto& app = Application::instance();

    AppConfig config;
    config.title = "Easy2D 自定义特效系统演示";
    config.width = 800;
    config.height = 600;
    config.vsync = true;
    config.fpsLimit = 0;

    if (!app.init(config)) {
        E2D_LOG_ERROR("初始化应用失败！");
        return -1;
    }

    app.enterScene(makePtr<CustomEffectsDemoScene>());

    E2D_LOG_INFO("启动主循环...");

    app.run();
    
    // 关闭自定义特效管理器
    E2D_CUSTOM_EFFECT_MANAGER().shutdown();

    E2D_LOG_INFO("应用结束。");

    Logger::shutdown();

    return 0;
}
