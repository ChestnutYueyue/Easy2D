# Easy2D Shader系统 API 文档

## 1. Shader系统概述

Shader系统允许你加载和使用自定义GLSL着色器，实现各种图形特效。

### 1.1 头文件
```cpp
#include <easy2d/graphics/shader_system.h>
#include <easy2d/graphics/shader_preset.h>
```

### 1.2 获取Shader系统实例
```cpp
// 使用宏获取Shader系统单例
auto& shaderSystem = E2D_SHADER_SYSTEM();
```

---

## 2. 初始化和关闭

### 2.1 初始化Shader系统
```cpp
// 初始化
if (!E2D_SHADER_SYSTEM().init()) {
    E2D_ERROR("Shader system initialization failed!");
    return;
}
```

### 2.2 关闭Shader系统
```cpp
// 在应用关闭时调用
E2D_SHADER_SYSTEM().shutdown();
```

---

## 3. 从源码加载Shader

### 3.1 定义Shader源码
```cpp
// 顶点着色器
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

// 片段着色器
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
```

### 3.2 加载Shader
```cpp
// 从源码加载Shader
auto glowShader = E2D_SHADER_SYSTEM().loadFromSource("glow", GLOW_VERT, GLOW_FRAG);

if (glowShader) {
    E2D_INFO("Successfully loaded glow Shader from source!");
} else {
    E2D_ERROR("Failed to load glow Shader!");
}
```

### 3.3 获取已加载的Shader
```cpp
// 通过名称获取Shader
auto shader = E2D_SHADER_SYSTEM().get("glow");

if (shader) {
    E2D_INFO("Successfully retrieved Shader by name!");
}

// 检查Shader是否存在
if (E2D_SHADER_SYSTEM().has("glow")) {
    E2D_INFO("Shader 'glow' exists in system");
}
```

---

## 4. Shader预设

### 4.1 水波纹效果
```cpp
#include <easy2d/graphics/shader_preset.h>

// 创建水波纹Shader
WaterParams waterParams;
waterParams.waveSpeed = 3.0f;
waterParams.waveAmplitude = 0.03f;
waterParams.waveFrequency = 15.0f;
auto waterShader = ShaderPreset::Water(waterParams);

// 更新时间参数
waterShader->setFloat("u_time", time_);
```

### 4.2 描边效果
```cpp
// 创建描边Shader
OutlineParams outlineParams;
outlineParams.color = Color(1.0f, 0.2f, 0.2f, 1.0f);  // 红色描边
outlineParams.thickness = 3.0f;
auto outlineShader = ShaderPreset::Outline(outlineParams);

// 更新纹理大小
if (texture) {
    outlineShader->setVec2("u_textureSize", 
        glm::vec2(static_cast<float>(texture->getWidth()), 
                 static_cast<float>(texture->getHeight())));
}
```

### 4.3 像素化效果
```cpp
// 创建像素化Shader
PixelateParams pixelateParams;
pixelateParams.pixelSize = 16.0f;
auto pixelateShader = ShaderPreset::Pixelate(pixelateParams);

// 更新纹理大小
if (texture) {
    pixelateShader->setVec2("u_textureSize",
        glm::vec2(static_cast<float>(texture->getWidth()),
                 static_cast<float>(texture->getHeight())));
}
```

### 4.4 灰度效果
```cpp
// 创建灰度Shader
GrayscaleParams grayscaleParams;
grayscaleParams.intensity = 0.8f;
auto grayscaleShader = ShaderPreset::Grayscale(grayscaleParams);
```

---

## 5. 更新Shader参数

### 5.1 常用Uniform设置
```cpp
// 设置float
shader->setFloat("u_time", time_);
shader->setFloat("u_intensity", 0.5f);

// 设置vec2
shader->setVec2("u_textureSize", glm::vec2(256.0f, 256.0f));
shader->setVec2("u_offset", glm::vec2(10.0f, 10.0f));

// 设置vec3
shader->setVec3("u_color", glm::vec3(1.0f, 0.0f, 0.0f));

// 设置vec4
shader->setVec4("u_tint", glm::vec4(1.0f, 1.0f, 1.0f, 0.5f));

// 设置int
shader->setInt("u_sampleCount", 4);

// 设置bool
shader->setBool("u_enabled", true);

// 设置mat4
glm::mat4 matrix = glm::mat4(1.0f);
shader->setMat4("u_transform", matrix);
```

### 5.2 动态更新参数
```cpp
void onUpdate(float dt) override {
    Scene::onUpdate(dt);
    
    time_ += dt;
    
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
}
```

---

## 6. 完整示例

```cpp
#include <easy2d/easy2d.h>
#include <easy2d/graphics/shader_system.h>
#include <easy2d/graphics/shader_preset.h>
#include <easy2d/utils/logger.h>
#include <sstream>

using namespace easy2d;

// ============================================================================
// Shader演示场景
// ============================================================================
class ShaderDemoScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        setBackgroundColor(Color(0.05f, 0.05f, 0.1f, 1.0f));
        
        // 初始化Shader系统
        initShaderSystem();
        
        // 创建Shader预设
        createShaderPresets();
        
        // 加载字体
        auto& resources = Application::instance().resources();
        font_ = resources.loadFont("C:/Windows/Fonts/simsun.ttc", 18);
    }
    
    void onUpdate(float dt) override {
        Scene::onUpdate(dt);
        
        time_ += dt;
        
        // 动态更新Shader参数
        updateShaderParams();
    }
    
    void onRender(RenderBackend& renderer) override {
        Scene::onRender(renderer);
        
        // 绘制说明
        drawInstructions(renderer);
    }
    
    void onExit() override {
        E2D_SHADER_SYSTEM().shutdown();
        Scene::onExit();
    }

private:
    float time_ = 0.0f;
    Ptr<FontAtlas> font_;
    
    Ptr<GLShader> waterShader_;
    Ptr<GLShader> outlineShader_;
    Ptr<GLShader> pixelateShader_;
    Ptr<GLShader> grayscaleShader_;
    
    void initShaderSystem() {
        if (!E2D_SHADER_SYSTEM().init()) {
            E2D_ERROR("Shader system initialization failed!");
            return;
        }
        E2D_INFO("Shader系统初始化完成");
    }
    
    void createShaderPresets() {
        // 1. 水波纹效果
        WaterParams waterParams;
        waterParams.waveSpeed = 3.0f;
        waterParams.waveAmplitude = 0.03f;
        waterParams.waveFrequency = 15.0f;
        waterShader_ = ShaderPreset::Water(waterParams);
        
        // 2. 描边效果
        OutlineParams outlineParams;
        outlineParams.color = Color(1.0f, 0.2f, 0.2f, 1.0f);
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
        
        E2D_INFO("Shader预设创建完成");
    }
    
    void updateShaderParams() {
        // 更新水波纹Shader的时间参数
        if (waterShader_) {
            waterShader_->setFloat("u_time", time_);
        }
    }
    
    void drawInstructions(RenderBackend& renderer) {
        if (!font_) return;
        
        float y = 30.0f;
        
        renderer.drawText(*font_, "Easy2D Shader系统演示", 
                         Vec2(280.0f, y), Color(1.0f, 0.9f, 0.2f, 1.0f));
        y += 35.0f;
        
        renderer.drawText(*font_, "支持的Shader预设:", 
                         Vec2(50.0f, y), Color(0.8f, 0.8f, 0.8f, 1.0f));
        y += 25.0f;
        
        renderer.drawText(*font_, "1. Water - 水波纹效果", 
                         Vec2(70.0f, y), Color(0.7f, 0.9f, 0.7f, 1.0f));
        y += 20.0f;
        
        renderer.drawText(*font_, "2. Outline - 描边效果", 
                         Vec2(70.0f, y), Color(0.7f, 0.9f, 0.7f, 1.0f));
        y += 20.0f;
        
        renderer.drawText(*font_, "3. Pixelate - 像素化效果", 
                         Vec2(70.0f, y), Color(0.7f, 0.9f, 0.7f, 1.0f));
        y += 20.0f;
        
        renderer.drawText(*font_, "4. Grayscale - 灰度效果", 
                         Vec2(70.0f, y), Color(0.7f, 0.9f, 0.7f, 1.0f));
        y += 30.0f;
        
        renderer.drawText(*font_, "API: ShaderPreset::XXX() 或 loadFromSource()", 
                         Vec2(50.0f, y), Color(0.6f, 0.8f, 1.0f, 1.0f));
        
        // FPS
        std::stringstream ss;
        ss << "FPS: " << Application::instance().fps();
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
    config.title = "Shader系统演示";
    config.width = 800;
    config.height = 600;
    config.vsync = true;
    
    if (!app.init(config)) {
        E2D_LOG_ERROR("初始化失败！");
        return -1;
    }
    
    app.enterScene(makePtr<ShaderDemoScene>());
    app.run();
    
    Logger::shutdown();
    return 0;
}
```
