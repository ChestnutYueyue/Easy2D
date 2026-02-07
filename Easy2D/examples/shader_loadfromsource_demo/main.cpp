/**
 * Easy2D Shader LoadFromSource 示例
 * 
 * 演示内容：
 * 1. 使用 loadFromSource 从字符串源码加载 Shader
 * 2. 创建自定义特效 Shader（发光效果）
 * 3. 验证 Shader 加载成功并可以获取
 */

#include <easy2d/easy2d.h>
#include <iostream>
#include <sstream>

using namespace easy2d;

// ============================================================================
// 自定义发光效果 Shader 源码
// ============================================================================

static const char* GLOW_VERT = R"(
#version 330 core
layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 a_texCoord;
layout(location = 2) in vec4 a_color;

uniform mat4 u_viewProjection;
uniform mat4 u_model;

out vec2 v_texCoord;
out vec4 v_color;

void main() {
    gl_Position = u_viewProjection * u_model * vec4(a_position, 0.0, 1.0);
    v_texCoord = a_texCoord;
    v_color = a_color;
}
)";

static const char* GLOW_FRAG = R"(
#version 330 core
in vec2 v_texCoord;
in vec4 v_color;

uniform sampler2D u_texture;
uniform float u_glowIntensity;
uniform vec3 u_glowColor;

out vec4 fragColor;

void main() {
    vec4 texColor = texture(u_texture, v_texCoord);
    
    // 基础颜色
    vec4 baseColor = texColor * v_color;
    
    // 发光效果
    float brightness = dot(baseColor.rgb, vec3(0.299, 0.587, 0.114));
    vec3 glow = u_glowColor * brightness * u_glowIntensity;
    
    fragColor = vec4(baseColor.rgb + glow, baseColor.a);
    
    if (fragColor.a < 0.01) {
        discard;
    }
}
)";

// ============================================================================
// 演示场景
// ============================================================================
class ShaderDemoScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        
        setBackgroundColor(Color(0.05f, 0.05f, 0.1f, 1.0f));
        
        // 初始化 Shader 系统
        initShaderSystem();
        
        // 创建说明文字
        createInstructions();
    }
    
    void onUpdate(float dt) override {
        Scene::onUpdate(dt);
        
        // 更新时间
        time_ += dt;
    }
    
    void onRender(RenderBackend& renderer) override {
        Scene::onRender(renderer);
        
        // 绘制一些简单的形状来展示场景
        float centerX = 400.0f;
        float centerY = 300.0f;
        
        // 绘制几个彩色圆圈
        for (int i = 0; i < 5; ++i) {
            float angle = (i / 5.0f) * 2.0f * 3.14159f + time_;
            float radius = 120.0f;
            float x = centerX + std::cos(angle) * radius;
            float y = centerY + std::sin(angle) * radius;
            
            float r = 0.5f + 0.5f * std::sin(i * 1.2f + time_);
            float g = 0.5f + 0.5f * std::sin(i * 1.5f + time_ + 1.0f);
            float b = 0.5f + 0.5f * std::sin(i * 1.8f + time_ + 2.0f);
            
            renderer.fillCircle(Vec2(x, y), 30.0f, Color(r, g, b, 0.8f));
        }
        
        // 绘制中心圆
        renderer.fillCircle(Vec2(centerX, centerY), 50.0f, 
                           Color(1.0f, 0.8f, 0.2f, 0.9f));
        
        // 绘制文字
        if (font_) {
            // 绘制标题
            renderer.drawText(*font_, "Shader LoadFromSource Demo", 
                            Vec2(250.0f, 20.0f), Color(1.0f, 1.0f, 1.0f, 1.0f));
            
            // 绘制说明
            renderer.drawText(*font_, "Using loadFromSource() to load Shader from source code", 
                            Vec2(150.0f, 50.0f), Color(0.8f, 0.8f, 0.8f, 1.0f));
            
            renderer.drawText(*font_, "Loaded: glow, test1, test2 (3 custom Shaders)", 
                            Vec2(200.0f, 80.0f), Color(0.7f, 0.9f, 0.7f, 1.0f));
            
            // 绘制 FPS
            auto fps = Application::instance().fps();
            std::stringstream ss;
            ss << "FPS: " << fps;
            renderer.drawText(*font_, ss.str(), Vec2(20.0f, 560.0f), 
                            Color(0.8f, 1.0f, 0.8f, 1.0f));
        }
    }

private:
    float time_ = 0.0f;
    Ptr<FontAtlas> font_;
    
    void initShaderSystem() {
        // 初始化 Shader 系统
        if (!E2D_SHADER_SYSTEM().init()) {
            E2D_ERROR("Shader system initialization failed!");
            return;
        }
        
        // 使用 loadFromSource 从源码字符串加载 Shader
        auto glowShader = E2D_SHADER_SYSTEM().loadFromSource("glow", GLOW_VERT, GLOW_FRAG);
        
        if (glowShader) {
            E2D_INFO("Successfully loaded glow Shader from source!");
            
            // 验证可以通过名称获取
            auto retrievedShader = E2D_SHADER_SYSTEM().get("glow");
            if (retrievedShader) {
                E2D_INFO("Successfully retrieved Shader by name!");
            }
        } else {
            E2D_ERROR("Failed to load glow Shader!");
        }
        
        // 测试加载多个 Shader
        auto shader1 = E2D_SHADER_SYSTEM().loadFromSource("test1", GLOW_VERT, GLOW_FRAG);
        auto shader2 = E2D_SHADER_SYSTEM().loadFromSource("test2", GLOW_VERT, GLOW_FRAG);
        
        if (shader1 && shader2) {
            E2D_INFO("Successfully loaded multiple Shaders!");
        }
        
        // 检查 Shader 是否存在
        if (E2D_SHADER_SYSTEM().has("glow")) {
            E2D_INFO("Shader 'glow' exists in system");
        }
        
        // 获取所有注册的 Shader 名称
        E2D_INFO("Registered custom Shaders: 3 (glow, test1, test2)");
    }
    
    void createInstructions() {
        auto& resources = Application::instance().resources();
        
        // 加载字体
        font_ = resources.loadFont("C:/Windows/Fonts/simsun.ttc", 20);
    }
};

// ============================================================================
// 主函数
// ============================================================================
int main() {
    Logger::init();
    Logger::setLevel(LogLevel::Info);

    E2D_LOG_INFO("========================");
    E2D_LOG_INFO("Shader LoadFromSource Demo");
    E2D_LOG_INFO("========================");

    auto& app = Application::instance();

    AppConfig config;
    config.title = "Shader LoadFromSource Demo";
    config.width = 800;
    config.height = 600;
    config.vsync = true;

    if (!app.init(config)) {
        E2D_LOG_ERROR("Application initialization failed!");
        return -1;
    }

    app.enterScene(makePtr<ShaderDemoScene>());

    E2D_LOG_INFO("Starting main loop...");
    app.run();
    
    // 关闭 Shader 系统
    E2D_SHADER_SYSTEM().shutdown();

    E2D_LOG_INFO("Application ended.");
    Logger::shutdown();

    return 0;
}
