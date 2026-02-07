# Easy2D 渲染系统 API 文档

## 1. 渲染后端 (RenderBackend)

### 1.1 获取渲染器
```cpp
RenderBackend& renderer = Application::instance().renderer();
```

### 1.2 基本绘制方法
```cpp
// 绘制矩形边框
void drawRect(const Rect& rect, const Color& color, float borderWidth = 1.0f);

// 绘制填充矩形
void fillRect(const Rect& rect, const Color& color);

// 绘制圆形边框
void drawCircle(const Vec2& center, float radius, const Color& color, float borderWidth = 1.0f);

// 绘制填充圆形
void fillCircle(const Vec2& center, float radius, const Color& color);

// 绘制线条
void drawLine(const Vec2& start, const Vec2& end, const Color& color, float width = 1.0f);
```

### 1.3 精灵绘制
```cpp
// 绘制精灵（完整纹理）
void drawSprite(Texture& texture, 
                const Rect& dstRect,           // 目标矩形
                const Color& color = Color::White,
                float rotation = 0.0f,
                const Vec2& anchor = Vec2::ZERO);

// 绘制精灵（指定纹理区域）
void drawSprite(Texture& texture,
                const Rect& dstRect,           // 目标矩形
                const Rect& srcRect,           // 源纹理矩形
                const Color& color = Color::White,
                float rotation = 0.0f,
                const Vec2& anchor = Vec2::ZERO);
```

### 1.4 文本绘制
```cpp
// 绘制文本
void drawText(FontAtlas& font,
              const std::string& text,
              const Vec2& position,
              const Color& color = Color::White);

// 绘制文本（使用String类支持中文）
void drawText(FontAtlas& font,
              const String& text,
              const Vec2& position,
              const Color& color = Color::White);
```

### 1.5 批量渲染
```cpp
// 开始精灵批处理
void beginSpriteBatch();

// 结束精灵批处理
void endSpriteBatch();

// 使用示例
renderer.beginSpriteBatch();
for (auto& sprite : sprites) {
    renderer.drawSprite(*sprite->getTexture(), 
                       sprite->getDestRect(),
                       sprite->getSourceRect(),
                       sprite->getColor(),
                       sprite->getRotation(),
                       sprite->getAnchor());
}
renderer.endSpriteBatch();
```

### 1.6 投影矩阵
```cpp
// 设置视图投影矩阵（使用GLM）
#include <glm/gtc/matrix_transform.hpp>

glm::mat4 ortho = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f);
renderer.setViewProjection(ortho);

// 设置视口
renderer.setViewport(0, 0, width, height);
```

---

## 2. 在 onRender 中绘制

### 2.1 基本绘制流程
```cpp
void onRender(RenderBackend& renderer) override {
    // 1. 调用父类渲染（渲染所有子节点）
    Scene::onRender(renderer);
    
    // 2. 自定义绘制
    drawUI(renderer);
    drawEffects(renderer);
}
```

### 2.2 绘制UI示例
```cpp
void drawUI(RenderBackend& renderer) {
    if (!font_) return;
    
    // 绘制标题
    renderer.drawText(*font_, "游戏标题", 
                     Vec2(350.0f, 20.0f), 
                     Color(1.0f, 0.9f, 0.2f, 1.0f));
    
    // 绘制分数背景
    renderer.fillRect(Rect(20.0f, 20.0f, 150.0f, 40.0f),
                     Color(0.0f, 0.0f, 0.0f, 0.5f));
    
    // 绘制分数
    std::string scoreText = "Score: " + std::to_string(score_);
    renderer.drawText(*font_, scoreText,
                     Vec2(30.0f, 28.0f),
                     Color(1.0f, 1.0f, 1.0f, 1.0f));
    
    // 绘制血条背景
    renderer.fillRect(Rect(20.0f, 70.0f, 200.0f, 20.0f),
                     Color(0.3f, 0.0f, 0.0f, 1.0f));
    
    // 绘制血条
    float hpPercent = currentHP_ / maxHP_;
    renderer.fillRect(Rect(20.0f, 70.0f, 200.0f * hpPercent, 20.0f),
                     Color(1.0f, 0.0f, 0.0f, 1.0f));
    
    // 绘制血条边框
    renderer.drawRect(Rect(20.0f, 70.0f, 200.0f, 20.0f),
                     Color(1.0f, 1.0f, 1.0f, 1.0f),
                     2.0f);
}
```

### 2.3 绘制几何图形示例
```cpp
void drawGeometry(RenderBackend& renderer) {
    // 绘制网格
    Color gridColor(0.2f, 0.2f, 0.2f, 1.0f);
    for (int i = 0; i <= 10; ++i) {
        float x = i * 80.0f;
        renderer.drawLine(Vec2(x, 0.0f), Vec2(x, 600.0f), gridColor);
        
        float y = i * 60.0f;
        renderer.drawLine(Vec2(0.0f, y), Vec2(800.0f, y), gridColor);
    }
    
    // 绘制圆形目标
    renderer.fillCircle(Vec2(400.0f, 300.0f), 50.0f, 
                       Color(1.0f, 0.5f, 0.0f, 0.8f));
    renderer.drawCircle(Vec2(400.0f, 300.0f), 50.0f,
                       Color(1.0f, 1.0f, 0.0f, 1.0f), 3.0f);
    
    // 绘制多个彩色圆圈
    for (int i = 0; i < 5; ++i) {
        float angle = (i / 5.0f) * 2.0f * 3.14159f;
        float radius = 120.0f;
        float x = 400.0f + std::cos(angle) * radius;
        float y = 300.0f + std::sin(angle) * radius;
        
        float r = 0.5f + 0.5f * std::sin(i * 1.2f);
        float g = 0.5f + 0.5f * std::sin(i * 1.5f + 1.0f);
        float b = 0.5f + 0.5f * std::sin(i * 1.8f + 2.0f);
        
        renderer.fillCircle(Vec2(x, y), 30.0f, Color(r, g, b, 0.8f));
    }
}
```

---

## 3. 自定义节点绘制

### 3.1 重写 onDraw 方法
```cpp
class CustomShape : public Node {
public:
    void onDraw(RenderBackend& renderer) override {
        Vec2 pos = getPosition();
        Color color = getColor();
        
        // 绘制自定义形状
        renderer.fillRect(
            Rect(pos.x - width_ / 2, pos.y - height_ / 2, width_, height_),
            color
        );
        
        // 绘制边框
        renderer.drawRect(
            Rect(pos.x - width_ / 2, pos.y - height_ / 2, width_, height_),
            Color(1.0f, 1.0f, 1.0f, 1.0f),
            2.0f
        );
    }
    
private:
    float width_ = 100.0f;
    float height_ = 100.0f;
};
```

### 3.2 碰撞框可视化
```cpp
class CollisionBox : public Node {
public:
    CollisionBox(float width, float height, const Color& color)
        : width_(width), height_(height), color_(color), isColliding_(false) {
        setSpatialIndexed(true);
    }
    
    void setColliding(bool colliding) { isColliding_ = colliding; }
    
    Rect getBoundingBox() const override {
        Vec2 pos = getPosition();
        return Rect(pos.x - width_ / 2, pos.y - height_ / 2, width_, height_);
    }
    
    void onDraw(RenderBackend& renderer) override {
        Vec2 pos = getPosition();
        
        // 碰撞时变红色
        Color fillColor = isColliding_ ? Color(1.0f, 0.2f, 0.2f, 0.8f) : color_;
        renderer.fillRect(
            Rect(pos.x - width_ / 2, pos.y - height_ / 2, width_, height_),
            fillColor
        );
        
        // 碰撞时边框变粗
        Color borderColor = isColliding_ ? Color(1.0f, 0.0f, 0.0f, 1.0f)
                                         : Color(1.0f, 1.0f, 1.0f, 0.5f);
        float borderWidth = isColliding_ ? 3.0f : 2.0f;
        renderer.drawRect(
            Rect(pos.x - width_ / 2, pos.y - height_ / 2, width_, height_),
            borderColor, borderWidth);
    }
    
private:
    float width_, height_;
    Color color_;
    bool isColliding_;
};
```

---

## 4. 完整渲染示例

```cpp
#include <easy2d/easy2d.h>
#include <easy2d/utils/logger.h>
#include <sstream>

using namespace easy2d;

// ============================================================================
// 渲染演示场景
// ============================================================================
class RenderingDemoScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        setBackgroundColor(Color(0.05f, 0.05f, 0.1f, 1.0f));
        
        auto& resources = Application::instance().resources();
        font_ = resources.loadFont("C:/Windows/Fonts/simsun.ttc", 18);
    }
    
    void onUpdate(float dt) override {
        Scene::onUpdate(dt);
        time_ += dt;
        rotation_ += 45.0f * dt;
    }
    
    void onRender(RenderBackend& renderer) override {
        Scene::onRender(renderer);
        
        // 绘制几何图形
        drawShapes(renderer);
        
        // 绘制UI
        drawUI(renderer);
    }

private:
    Ptr<FontAtlas> font_;
    float time_ = 0.0f;
    float rotation_ = 0.0f;
    
    void drawShapes(RenderBackend& renderer) {
        float centerX = 400.0f;
        float centerY = 280.0f;
        
        // 绘制旋转的矩形
        float rectSize = 80.0f;
        Vec2 corners[4];
        float rad = rotation_ * 3.14159f / 180.0f;
        for (int i = 0; i < 4; ++i) {
            float angle = rad + i * 3.14159f / 2;
            float r = rectSize / 1.414f; // 对角线的一半
            corners[i] = Vec2(centerX + std::cos(angle) * r,
                             centerY + std::sin(angle) * r);
        }
        
        // 绘制旋转矩形（使用线条）
        Color rectColor(0.2f, 0.6f, 1.0f, 1.0f);
        for (int i = 0; i < 4; ++i) {
            renderer.drawLine(corners[i], corners[(i + 1) % 4], rectColor, 3.0f);
        }
        
        // 绘制脉冲圆圈
        float pulse = 1.0f + 0.2f * std::sin(time_ * 3.0f);
        renderer.drawCircle(Vec2(centerX, centerY), 100.0f * pulse,
                           Color(1.0f, 0.5f, 0.0f, 0.5f), 2.0f);
        
        // 绘制彩色小球
        for (int i = 0; i < 8; ++i) {
            float angle = (i / 8.0f) * 2.0f * 3.14159f + time_;
            float radius = 150.0f;
            float x = centerX + std::cos(angle) * radius;
            float y = centerY + std::sin(angle) * radius;
            
            float r = 0.5f + 0.5f * std::sin(i * 0.8f + time_);
            float g = 0.5f + 0.5f * std::sin(i * 0.8f + time_ + 2.0f);
            float b = 0.5f + 0.5f * std::sin(i * 0.8f + time_ + 4.0f);
            
            renderer.fillCircle(Vec2(x, y), 15.0f, Color(r, g, b, 0.8f));
        }
    }
    
    void drawUI(RenderBackend& renderer) {
        if (!font_) return;
        
        // 标题
        renderer.drawText(*font_, "渲染系统演示",
                         Vec2(330.0f, 20.0f), Color(1.0f, 0.9f, 0.2f, 1.0f));
        
        // 绘制信息面板背景
        renderer.fillRect(Rect(20.0f, 450.0f, 200.0f, 120.0f),
                         Color(0.0f, 0.0f, 0.0f, 0.5f));
        renderer.drawRect(Rect(20.0f, 450.0f, 200.0f, 120.0f),
                         Color(0.5f, 0.5f, 0.5f, 1.0f), 1.0f);
        
        // 绘制信息
        float y = 470.0f;
        renderer.drawText(*font_, "绘制方法:",
                         Vec2(30.0f, y), Color(0.8f, 0.8f, 0.8f, 1.0f));
        y += 25.0f;
        
        renderer.drawText(*font_, "- drawRect/fillRect",
                         Vec2(30.0f, y), Color(0.7f, 0.9f, 0.7f, 1.0f));
        y += 20.0f;
        
        renderer.drawText(*font_, "- drawCircle/fillCircle",
                         Vec2(30.0f, y), Color(0.7f, 0.9f, 0.7f, 1.0f));
        y += 20.0f;
        
        renderer.drawText(*font_, "- drawLine",
                         Vec2(30.0f, y), Color(0.7f, 0.9f, 0.7f, 1.0f));
        y += 20.0f;
        
        renderer.drawText(*font_, "- drawText",
                         Vec2(30.0f, y), Color(0.7f, 0.9f, 0.7f, 1.0f));
        
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
    config.title = "渲染系统演示";
    config.width = 800;
    config.height = 600;
    config.vsync = true;
    
    if (!app.init(config)) {
        E2D_LOG_ERROR("初始化失败！");
        return -1;
    }
    
    app.enterScene(makePtr<RenderingDemoScene>());
    app.run();
    
    Logger::shutdown();
    return 0;
}
```
