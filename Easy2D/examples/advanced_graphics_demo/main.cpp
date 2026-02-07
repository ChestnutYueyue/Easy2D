/**
 * Easy2D 高级图形功能演示
 * 
 * 演示内容：
 * 1. 纹理池 - LRU缓存管理
 * 2. 渲染目标 - FBO离屏渲染
 * 3. Shader预设 - 水波纹、描边、像素化等效果
 * 4. 后处理效果 - 泛光、模糊、颜色分级
 */

#include <easy2d/easy2d.h>
#include <easy2d/graphics/texture_pool.h>
#include <easy2d/graphics/render_target.h>
#include <easy2d/graphics/shader_preset.h>
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace easy2d;

// ============================================================================
// 高级图形演示场景
// ============================================================================
class AdvancedGraphicsScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        
        setBackgroundColor(Color(0.05f, 0.05f, 0.1f, 1.0f));
        
        // 初始化各个系统
        initSystems();
        
        // 创建渲染目标
        createRenderTargets();
        
        // 创建Shader预设
        createShaderPresets();
        
        // 加载字体
        auto& resources = Application::instance().resources();
        font_ = resources.loadFont("C:/Windows/Fonts/simsun.ttc", 18);
        
        // 创建测试精灵
        createTestSprites();
    }
    
    void onExit() override {
        // 关闭系统
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

        // 旋转测试精灵
        if (testSprite_) {
            testSprite_->setRotation(time_ * 30.0f);
        }
    }

    void onRender(RenderBackend& renderer) override {
        // 渲染到离屏目标
        renderToOffscreen();

        // 调用父类渲染（渲染子节点）
        Scene::onRender(renderer);

        // 绘制UI文字
        if (font_) {
            drawInstructions(renderer);
            drawStats(renderer);
        }
    }

private:
    float time_ = 0.0f;
    Ptr<FontAtlas> font_;
    Ptr<Sprite> testSprite_;
    
    // 渲染目标
    Ptr<RenderTarget> offscreenTarget_;
    
    // Shader预设
    Ptr<GLShader> waterShader_;
    Ptr<GLShader> outlineShader_;
    Ptr<GLShader> pixelateShader_;
    Ptr<GLShader> grayscaleShader_;
    
    void initSystems() {
        // 初始化纹理池
        TexturePoolConfig poolConfig;
        poolConfig.maxCacheSize = 32 * 1024 * 1024;  // 32MB
        poolConfig.maxTextureCount = 128;
        E2D_TEXTURE_POOL().init(poolConfig);
        
        E2D_INFO("高级图形系统初始化完成");
    }
    
    void createRenderTargets() {
        // 创建离屏渲染目标
        RenderTargetConfig rtConfig;
        rtConfig.width = 400;
        rtConfig.height = 300;
        rtConfig.hasDepth = false;
        
        offscreenTarget_ = RenderTarget::createFromConfig(rtConfig);
        
        if (offscreenTarget_) {
            E2D_INFO("创建离屏渲染目标: {}x{}", rtConfig.width, rtConfig.height);
        }
    }
    
    void createShaderPresets() {
        // 创建各种Shader预设
        
        // 1. 水波纹效果
        WaterParams waterParams;
        waterParams.waveSpeed = 3.0f;
        waterParams.waveAmplitude = 0.03f;
        waterParams.waveFrequency = 15.0f;
        waterShader_ = ShaderPreset::Water(waterParams);
        
        // 2. 描边效果
        OutlineParams outlineParams;
        outlineParams.color = Color(1.0f, 0.2f, 0.2f, 1.0f);  // 红色描边
        outlineParams.thickness = 3.0f;
        outlineShader_ = ShaderPreset::Outline(outlineParams);
        
        // 3. 像素化效果
        PixelateParams pixelateParams;
        pixelateParams.pixelSize = 16.0f;
        pixelateShader_ = ShaderPreset::Pixelate(pixelateParams);
        
        // 4. 灰度效果
        GrayscaleParams grayscaleParams;
        grayscaleParams.intensity = 0.8f;
        grayscaleShader_ = ShaderPreset::Grayscale(grayscaleParams);
        
        E2D_INFO("创建Shader预设完成");
    }
    
    void createTestSprites() {
        E2D_INFO("开始创建测试精灵...");

        // 创建测试精灵
        testSprite_ = Sprite::create();
        testSprite_->setPosition(Vec2(200.0f, 300.0f));
        testSprite_->setScale(2.0f);
        testSprite_->setColor(Color(0.2f, 0.6f, 1.0f, 1.0f));
        addChild(testSprite_);
        E2D_INFO("主精灵创建完成");

        // 创建显示离屏渲染结果的精灵（使用纯色纹理）
        if (offscreenTarget_) {
            auto colorTex = offscreenTarget_->getColorTexture();
            if (colorTex && colorTex->isValid()) {
                auto displaySprite = Sprite::create(colorTex);
                if (displaySprite) {
                    displaySprite->setPosition(Vec2(600.0f, 300.0f));
                    displaySprite->setScale(1.0f);
                    addChild(displaySprite);
                    E2D_INFO("离屏渲染显示精灵创建完成");
                }
            } else {
                E2D_WARN("离屏渲染目标纹理无效");
            }
        }

        E2D_INFO("创建测试精灵完成");
    }
    
    void updateShaderParams() {
        // 更新水波纹Shader的时间参数
        if (waterShader_) {
            waterShader_->setFloat("u_time", time_);
        }
        
        // 更新描边Shader的纹理大小
        if (outlineShader_ && testSprite_ && testSprite_->getTexture()) {
            auto tex = testSprite_->getTexture();
            outlineShader_->setVec2("u_textureSize", 
                glm::vec2(static_cast<float>(tex->getWidth()), 
                         static_cast<float>(tex->getHeight())));
        }
        
        // 更新像素化Shader的纹理大小
        if (pixelateShader_ && testSprite_ && testSprite_->getTexture()) {
            auto tex = testSprite_->getTexture();
            pixelateShader_->setVec2("u_textureSize",
                glm::vec2(static_cast<float>(tex->getWidth()),
                         static_cast<float>(tex->getHeight())));
        }
    }
    
    void renderToOffscreen() {
        if (!offscreenTarget_) {
            return;
        }
        
        // 绑定离屏渲染目标
        offscreenTarget_->bind();
        offscreenTarget_->clear(Color(0.1f, 0.1f, 0.2f, 1.0f));
        
        // 解绑渲染目标
        offscreenTarget_->unbind();
    }
    
    void drawInstructions(RenderBackend& renderer) {
        float y = 20.0f;
        
        renderer.drawText(*font_, "Easy2D 高级图形功能演示", 
                         Vec2(250.0f, y), Color(1.0f, 0.9f, 0.2f, 1.0f));
        y += 25.0f;
        
        renderer.drawText(*font_, "功能列表:", 
                         Vec2(20.0f, y), Color(0.8f, 0.8f, 0.8f, 1.0f));
        y += 20.0f;
        
        renderer.drawText(*font_, "1. 纹理池 - LRU缓存管理纹理资源", 
                         Vec2(40.0f, y), Color(0.7f, 0.9f, 0.7f, 1.0f));
        y += 18.0f;
        
        renderer.drawText(*font_, "2. 渲染目标 - FBO离屏渲染", 
                         Vec2(40.0f, y), Color(0.7f, 0.9f, 0.7f, 1.0f));
        y += 18.0f;
        
        renderer.drawText(*font_, "3. Shader预设 - 水波纹、描边、像素化等", 
                         Vec2(40.0f, y), Color(0.7f, 0.9f, 0.7f, 1.0f));
        y += 18.0f;
        
        renderer.drawText(*font_, "4. 后处理效果 - 泛光、模糊、颜色分级", 
                         Vec2(40.0f, y), Color(0.7f, 0.9f, 0.7f, 1.0f));
    }
    
    void drawStats(RenderBackend& renderer) {
        if (!font_) return;
        
        float y = 550.0f;
        
        // 纹理池统计
        auto texCount = E2D_TEXTURE_POOL().getTextureCount();
        auto texSize = E2D_TEXTURE_POOL().getCacheSize() / 1024;
        auto hitRate = E2D_TEXTURE_POOL().getHitRate() * 100.0f;
        
        std::stringstream ss;
        ss << "纹理池: " << texCount << " 个, " << texSize << " KB, 命中率: " 
           << std::fixed << std::setprecision(1) << hitRate << "%";
        renderer.drawText(*font_, ss.str(), Vec2(20.0f, y), 
                         Color(0.5f, 1.0f, 0.5f, 1.0f));
        y += 20.0f;
        
        // FPS
        auto fps = Application::instance().fps();
        ss.str("");
        ss << "FPS: " << fps;
        renderer.drawText(*font_, ss.str(), Vec2(20.0f, y), 
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
    E2D_LOG_INFO("Easy2D 高级图形功能演示");
    E2D_LOG_INFO("========================");

    auto& app = Application::instance();

    AppConfig config;
    config.title = "Easy2D 高级图形功能演示";
    config.width = 800;
    config.height = 600;
    config.vsync = true;

    if (!app.init(config)) {
        E2D_LOG_ERROR("初始化应用失败！");
        return -1;
    }

    app.enterScene(makePtr<AdvancedGraphicsScene>());

    E2D_LOG_INFO("启动主循环...");
    app.run();

    E2D_LOG_INFO("应用结束。");
    Logger::shutdown();

    return 0;
}
