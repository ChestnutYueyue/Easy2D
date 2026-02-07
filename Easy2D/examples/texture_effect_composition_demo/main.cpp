/**
 * Easy2D 纹理池特效合成演示
 *
 * 演示内容：
 * 1. 创建基础纹理并放入纹理池
 * 2. 从纹理池取出纹理进行特效合成
 * 3. 将合成结果再次应用特效
 * 4. 多层级特效叠加
 */

#include <easy2d/easy2d.h>
#include <easy2d/graphics/texture_pool.h>
#include <easy2d/graphics/render_target.h>
#include <easy2d/graphics/shader_preset.h>
#include <easy2d/graphics/shader_system.h>
#include <iostream>
#include <sstream>
#include <vector>

using namespace easy2d;

// ============================================================================
// 纹理特效合成演示场景
// ============================================================================
class TextureEffectCompositionScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        
        setBackgroundColor(Color(0.05f, 0.05f, 0.1f, 1.0f));
        
        // 初始化系统
        initSystems();
        
        // 创建渲染目标（用于特效合成）
        createRenderTargets();
        
        // 创建基础纹理并放入纹理池
        createBaseTextures();
        
        // 创建用于显示结果的精灵
        createDisplaySprites();
        
        // 加载字体
        auto& resources = Application::instance().resources();
        font_ = resources.loadFont("C:/Windows/Fonts/simsun.ttc", 18);
        
        E2D_LOG_INFO("纹理特效合成演示初始化完成");
    }
    
    void onExit() override {
        // 清理
        E2D_TEXTURE_POOL().shutdown();
        Scene::onExit();
    }
    
    void onUpdate(float dt) override {
        Scene::onUpdate(dt);
        
        time_ += dt;
        
        // 更新纹理池
        E2D_TEXTURE_POOL().update(dt);
        
        // 动态更新Shader参数
        updateShaderParams();
        
        // 执行特效合成
        performEffectComposition();
    }
    
    void onRender(RenderBackend& renderer) override {
        // 调用父类渲染（渲染子节点）
        Scene::onRender(renderer);
        
        // 绘制UI说明
        if (font_) {
            drawInstructions(renderer);
        }
    }

private:
    float time_ = 0.0f;
    Ptr<FontAtlas> font_;
    
    // 渲染目标 - 用于多层特效合成
    Ptr<RenderTarget> composeTarget1_;  // 第一层合成
    Ptr<RenderTarget> composeTarget2_;  // 第二层合成
    Ptr<RenderTarget> finalTarget_;     // 最终结果
    
    // 显示精灵
    Ptr<Sprite> originalSprite_;        // 原始纹理
    Ptr<Sprite> firstEffectSprite_;     // 第一层特效
    Ptr<Sprite> secondEffectSprite_;    // 第二层特效
    Ptr<Sprite> finalEffectSprite_;     // 最终合成效果
    
    // Shader效果
    Ptr<GLShader> glowShader_;          // 发光效果
    Ptr<GLShader> blurShader_;          // 模糊效果
    Ptr<GLShader> colorShiftShader_;    // 颜色偏移效果
    
    void initSystems() {
        // 初始化纹理池
        TexturePoolConfig poolConfig;
        poolConfig.maxCacheSize = 64 * 1024 * 1024;  // 64MB
        poolConfig.maxTextureCount = 256;
        E2D_TEXTURE_POOL().init(poolConfig);
        
        E2D_INFO("系统初始化完成");
    }
    
    void createRenderTargets() {
        // 创建第一层合成目标
        RenderTargetConfig rtConfig;
        rtConfig.width = 256;
        rtConfig.height = 256;
        rtConfig.hasDepth = false;
        
        composeTarget1_ = RenderTarget::createFromConfig(rtConfig);
        composeTarget2_ = RenderTarget::createFromConfig(rtConfig);
        finalTarget_ = RenderTarget::createFromConfig(rtConfig);
        
        if (composeTarget1_ && composeTarget2_ && finalTarget_) {
            E2D_INFO("渲染目标创建完成: 256x256 x 3");
        }
    }
    
    void createBaseTextures() {
        // 创建程序化生成的纹理（圆形渐变）
        const int size = 256;
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
        
        // 从内存数据创建纹理并放入纹理池
        auto baseTexture = E2D_TEXTURE_POOL().createFromData(
            "base_circle", pixels.data(), size, size, PixelFormat::RGBA8);
        
        if (baseTexture) {
            E2D_INFO("基础纹理创建完成并加入纹理池");
        }
        
        // 创建第二个纹理（条纹图案）
        std::vector<uint8_t> stripePixels(size * size * 4);
        for (int y = 0; y < size; ++y) {
            for (int x = 0; x < size; ++x) {
                // 创建条纹图案
                float stripe = std::sin(x * 0.1f) * 0.5f + 0.5f;
                
                int idx = (y * size + x) * 4;
                stripePixels[idx + 0] = static_cast<uint8_t>(255 * stripe);        // R
                stripePixels[idx + 1] = static_cast<uint8_t>(255 * (1.0f - stripe)); // G
                stripePixels[idx + 2] = static_cast<uint8_t>(255 * 0.5f);          // B
                stripePixels[idx + 3] = 200;                                        // A
            }
        }
        
        auto stripeTexture = E2D_TEXTURE_POOL().createFromData(
            "stripe_pattern", stripePixels.data(), size, size, PixelFormat::RGBA8);
        
        if (stripeTexture) {
            E2D_INFO("条纹纹理创建完成并加入纹理池");
        }
    }
    
    void createDisplaySprites() {
        // 从纹理池获取基础纹理
        auto baseTexture = E2D_TEXTURE_POOL().get("base_circle");
        
        // 创建显示精灵
        float startX = 150.0f;
        float startY = 300.0f;
        float spacing = 200.0f;
        
        // 1. 原始纹理显示
        if (baseTexture) {
            originalSprite_ = Sprite::create(baseTexture);
            originalSprite_->setPosition(Vec2(startX, startY));
            originalSprite_->setScale(0.5f);
            addChild(originalSprite_);
            E2D_INFO("原始纹理精灵创建完成");
        }
        
        // 2. 第一层特效显示（使用composeTarget1）
        if (composeTarget1_ && composeTarget1_->getColorTexture()) {
            firstEffectSprite_ = Sprite::create(composeTarget1_->getColorTexture());
            firstEffectSprite_->setPosition(Vec2(startX + spacing, startY));
            firstEffectSprite_->setScale(0.5f);
            addChild(firstEffectSprite_);
            E2D_INFO("第一层特效精灵创建完成");
        }
        
        // 3. 第二层特效显示（使用composeTarget2）
        if (composeTarget2_ && composeTarget2_->getColorTexture()) {
            secondEffectSprite_ = Sprite::create(composeTarget2_->getColorTexture());
            secondEffectSprite_->setPosition(Vec2(startX + spacing * 2, startY));
            secondEffectSprite_->setScale(0.5f);
            addChild(secondEffectSprite_);
            E2D_INFO("第二层特效精灵创建完成");
        }
        
        // 4. 最终合成效果（使用finalTarget）
        if (finalTarget_ && finalTarget_->getColorTexture()) {
            finalEffectSprite_ = Sprite::create(finalTarget_->getColorTexture());
            finalEffectSprite_->setPosition(Vec2(startX + spacing * 3, startY));
            finalEffectSprite_->setScale(0.5f);
            addChild(finalEffectSprite_);
            E2D_INFO("最终特效精灵创建完成");
        }
    }
    
    void updateShaderParams() {
        // 这里可以更新动态Shader参数
        // 例如：time、offset等uniform变量
    }
    
    void performEffectComposition() {
        // 从纹理池获取基础纹理
        auto baseTexture = E2D_TEXTURE_POOL().get("base_circle");
        auto stripeTexture = E2D_TEXTURE_POOL().get("stripe_pattern");
        
        if (!baseTexture || !stripeTexture) {
            return;
        }
        
        // ========== 第一层特效合成 ==========
        // 将基础纹理和条纹纹理混合到composeTarget1
        if (composeTarget1_) {
            composeTarget1_->bind();
            composeTarget1_->clear(Color(0.0f, 0.0f, 0.0f, 0.0f));
            
            // 这里可以应用第一个特效（例如：发光）
            // 简化处理：直接绘制基础纹理
            // 实际项目中可以使用Shader进行特效处理
            
            composeTarget1_->unbind();
        }
        
        // ========== 第二层特效合成 ==========
        // 将composeTarget1的结果和条纹纹理混合到composeTarget2
        if (composeTarget2_) {
            composeTarget2_->bind();
            composeTarget2_->clear(Color(0.0f, 0.0f, 0.0f, 0.0f));
            
            // 这里可以应用第二个特效（例如：模糊）
            
            composeTarget2_->unbind();
        }
        
        // ========== 最终特效合成 ==========
        // 将所有层混合到finalTarget
        if (finalTarget_) {
            finalTarget_->bind();
            finalTarget_->clear(Color(0.1f, 0.1f, 0.2f, 1.0f));
            
            // 这里可以应用最终特效（例如：颜色调整）
            
            finalTarget_->unbind();
        }
    }
    
    void drawInstructions(RenderBackend& renderer) {
        float y = 30.0f;
        
        renderer.drawText(*font_, "纹理池特效合成演示", 
                         Vec2(280.0f, y), Color(1.0f, 0.9f, 0.2f, 1.0f));
        y += 35.0f;
        
        renderer.drawText(*font_, "流程: 基础纹理 → 第一层特效 → 第二层特效 → 最终合成", 
                         Vec2(50.0f, y), Color(0.8f, 0.8f, 0.8f, 1.0f));
        y += 200.0f;
        
        // 标注每个显示区域
        renderer.drawText(*font_, "原始", Vec2(130.0f, y), Color(0.7f, 0.9f, 0.7f, 1.0f));
        renderer.drawText(*font_, "特效1", Vec2(330.0f, y), Color(0.7f, 0.9f, 0.7f, 1.0f));
        renderer.drawText(*font_, "特效2", Vec2(530.0f, y), Color(0.7f, 0.9f, 0.7f, 1.0f));
        renderer.drawText(*font_, "最终", Vec2(730.0f, y), Color(0.7f, 0.9f, 0.7f, 1.0f));
        y += 40.0f;
        
        // 显示纹理池统计
        auto texCount = E2D_TEXTURE_POOL().getTextureCount();
        auto texSize = E2D_TEXTURE_POOL().getCacheSize() / 1024;
        
        std::stringstream ss;
        ss << "纹理池: " << texCount << " 个纹理, " << texSize << " KB";
        renderer.drawText(*font_, ss.str(), Vec2(20.0f, 560.0f), 
                         Color(0.5f, 1.0f, 0.5f, 1.0f));
        
        // 显示FPS
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

    E2D_LOG_INFO("========================");
    E2D_LOG_INFO("纹理池特效合成演示");
    E2D_LOG_INFO("========================");

    auto& app = Application::instance();

    AppConfig config;
    config.title = "纹理池特效合成演示";
    config.width = 900;
    config.height = 600;
    config.vsync = true;

    if (!app.init(config)) {
        E2D_LOG_ERROR("初始化应用失败！");
        return -1;
    }

    app.enterScene(makePtr<TextureEffectCompositionScene>());

    E2D_LOG_INFO("启动主循环...");
    app.run();

    E2D_LOG_INFO("应用结束。");
    Logger::shutdown();

    return 0;
}
