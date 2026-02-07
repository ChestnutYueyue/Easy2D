# Easy2D 锚点系统 API 文档

## 1. 锚点系统概述

锚点(Anchor)是节点变换的基准点，决定了节点在进行位置、旋转、缩放操作时的中心点。锚点使用归一化坐标(0.0-1.0)，其中(0,0)表示左上角，(1,1)表示右下角，(0.5,0.5)表示中心。

### 1.1 锚点坐标系

```
(0,0) -------- (0.5,0) -------- (1,0)
  |               |               |
  |               |               |
(0,0.5) ------ (0.5,0.5) ----- (1,0.5)
  |               |               |
  |               |               |
(0,1) -------- (0.5,1) -------- (1,1)
```

### 1.2 默认锚点

- 默认锚点为 `(0, 0)`，即左上角
- 设置锚点不会改变节点的当前位置，但会影响后续的变换操作

---

## 2. 设置和获取锚点

### 2.1 设置锚点

```cpp
// 方式1: 使用Vec2设置
node->setAnchor(Vec2(0.5f, 0.5f));  // 中心锚点

// 方式2: 分别设置x和y
node->setAnchor(0.5f, 0.5f);        // 中心锚点

// 常用锚点设置
node->setAnchor(0.0f, 0.0f);        // 左上角（默认）
node->setAnchor(0.5f, 0.0f);        // 上边中点
node->setAnchor(1.0f, 0.0f);        // 右上角
node->setAnchor(0.0f, 0.5f);        // 左边中点
node->setAnchor(0.5f, 0.5f);        // 中心
node->setAnchor(1.0f, 0.5f);        // 右边中点
node->setAnchor(0.0f, 1.0f);        // 左下角
node->setAnchor(0.5f, 1.0f);        // 下边中点
node->setAnchor(1.0f, 1.0f);        // 右下角
```

### 2.2 获取锚点

```cpp
// 获取当前锚点
Vec2 anchor = node->getAnchor();
float anchorX = anchor.x;
float anchorY = anchor.y;
```

---

## 3. 锚点对变换的影响

### 3.1 锚点对位置的影响

```cpp
// 创建一个100x100的精灵
auto sprite = Sprite::create(texture);
sprite->setPosition(Vec2(400.0f, 300.0f));

// 情况1: 左上角锚点(0,0)
sprite->setAnchor(0.0f, 0.0f);
// 精灵的左上角位于(400, 300)
// 精灵占据区域: (400,300) 到 (500,400)

// 情况2: 中心锚点(0.5,0.5)
sprite->setAnchor(0.5f, 0.5f);
// 精灵的中心位于(400, 300)
// 精灵占据区域: (350,250) 到 (450,350)
```

### 3.2 锚点对旋转的影响

```cpp
auto sprite = Sprite::create(texture);
sprite->setPosition(Vec2(400.0f, 300.0f));

// 左上角锚点 - 围绕左上角旋转
sprite->setAnchor(0.0f, 0.0f);
sprite->setRotation(45.0f);

// 中心锚点 - 围绕中心旋转
sprite->setAnchor(0.5f, 0.5f);
sprite->setRotation(45.0f);
```

### 3.3 锚点对缩放的影响

```cpp
auto sprite = Sprite::create(texture);
sprite->setPosition(Vec2(400.0f, 300.0f));

// 左上角锚点 - 从左上角向外缩放
sprite->setAnchor(0.0f, 0.0f);
sprite->setScale(2.0f);

// 中心锚点 - 从中心向四周缩放
sprite->setAnchor(0.5f, 0.5f);
sprite->setScale(2.0f);
```

---

## 4. 常用锚点场景

### 4.1 UI元素 - 左上角锚点

```cpp
// UI元素通常使用左上角锚点，便于布局
class UIScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        
        // 标题 - 左上角
        auto title = Text::create("游戏标题", font);
        title->setAnchor(0.0f, 0.0f);
        title->setPosition(Vec2(20.0f, 20.0f));
        addChild(title);
        
        // 分数 - 右上角
        auto score = Text::create("Score: 0", font);
        score->setAnchor(1.0f, 0.0f);
        score->setPosition(Vec2(780.0f, 20.0f));
        addChild(score);
        
        // 生命值 - 左下角
        auto health = Text::create("HP: 100", font);
        health->setAnchor(0.0f, 1.0f);
        health->setPosition(Vec2(20.0f, 580.0f));
        addChild(health);
        
        // 按钮 - 中心锚点便于居中
        auto button = Sprite::create(buttonTexture);
        button->setAnchor(0.5f, 0.5f);
        button->setPosition(Vec2(400.0f, 300.0f));
        addChild(button);
    }
};
```

### 4.2 游戏角色 - 中心锚点

```cpp
// 游戏角色通常使用中心锚点，便于旋转和缩放
class Player : public Node {
public:
    void onEnter() override {
        Node::onEnter();
        
        // 角色精灵 - 中心锚点
        auto sprite = Sprite::create(playerTexture);
        sprite->setAnchor(0.5f, 0.5f);  // 中心锚点
        sprite->setPosition(Vec2::Zero); // 相对于父节点中心
        addChild(sprite);
        
        // 血条背景 - 底部中心
        auto hpBg = Sprite::create(hpBgTexture);
        hpBg->setAnchor(0.5f, 1.0f);     // 底部中心
        hpBg->setPosition(Vec2(0.0f, -40.0f)); // 角色下方
        addChild(hpBg);
    }
    
    void update(float dt) {
        // 旋转 - 围绕中心旋转
        setRotation(getRotation() + 90.0f * dt);
        
        // 缩放 - 从中心缩放
        float scale = 1.0f + 0.2f * sin(time_ * 3.0f);
        setScale(scale);
    }
};
```

### 4.3 地图层 - 左上角锚点

```cpp
// 地图层通常使用左上角锚点
class GameScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        
        // 地图层 - 左上角锚点
        mapLayer_ = makePtr<Node>();
        mapLayer_->setAnchor(0.0f, 0.0f);
        mapLayer_->setPosition(Vec2(0.0f, 0.0f));
        addChild(mapLayer_);
        
        // 创建地图格子
        for (int y = 0; y < mapHeight_; ++y) {
            for (int x = 0; x < mapWidth_; ++x) {
                auto tile = Sprite::create(tileTexture);
                tile->setAnchor(0.0f, 0.0f);  // 左上角锚点
                tile->setPosition(
                    Vec2(x * tileWidth_, y * tileHeight_)
                );
                mapLayer_->addChild(tile);
            }
        }
    }
    
private:
    Ptr<Node> mapLayer_;
    int mapWidth_ = 20;
    int mapHeight_ = 15;
    float tileWidth_ = 32.0f;
    float tileHeight_ = 32.0f;
};
```

---

## 5. 锚点与边界框

### 5.1 计算实际边界

```cpp
class GameObject : public Node {
public:
    Rect getBoundingBox() const override {
        Vec2 pos = getPosition();
        Vec2 anchor = getAnchor();
        float width = getWidth();
        float height = getHeight();
        
        // 根据锚点计算实际边界
        float left = pos.x - width * anchor.x;
        float top = pos.y - height * anchor.y;
        
        return Rect(left, top, width, height);
    }
    
private:
    float getWidth() const { return width_; }
    float getHeight() const { return height_; }
    
    float width_ = 100.0f;
    float height_ = 100.0f;
};
```

### 5.2 边界框可视化

```cpp
class DebugNode : public Node {
public:
    void onDraw(RenderBackend& renderer) override {
        // 获取边界框
        Rect bounds = getBoundingBox();
        
        // 绘制边界框
        renderer.drawRect(bounds, Color(0.0f, 1.0f, 0.0f, 1.0f), 1.0f);
        
        // 绘制锚点位置
        Vec2 pos = getPosition();
        renderer.fillCircle(pos, 3.0f, Color(1.0f, 0.0f, 0.0f, 1.0f));
    }
};
```

---

## 6. 锚点与父节点

### 6.1 父子节点锚点关系

```cpp
class ParentChildScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        
        // 父节点 - 中心锚点
        auto parent = makePtr<Node>();
        parent->setAnchor(0.5f, 0.5f);
        parent->setPosition(Vec2(400.0f, 300.0f));
        addChild(parent);
        
        // 子节点1 - 相对于父节点的左上角
        auto child1 = Sprite::create(texture1);
        child1->setAnchor(0.0f, 0.0f);
        child1->setPosition(Vec2(-50.0f, -50.0f)); // 父节点左上
        parent->addChild(child1);
        
        // 子节点2 - 相对于父节点的右下角
        auto child2 = Sprite::create(texture2);
        child2->setAnchor(1.0f, 1.0f);
        child2->setPosition(Vec2(50.0f, 50.0f)); // 父节点右下
        parent->addChild(child2);
        
        // 旋转父节点，子节点会跟随旋转
        parent->setRotation(45.0f);
    }
};
```

---

## 7. 完整示例

```cpp
#include <easy2d/easy2d.h>
#include <easy2d/utils/logger.h>
#include <sstream>

using namespace easy2d;

// ============================================================================
// 锚点演示场景
// ============================================================================
class AnchorDemoScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        setBackgroundColor(Color(0.05f, 0.05f, 0.1f, 1.0f));
        
        // 加载字体
        auto& resources = Application::instance().resources();
        font_ = resources.loadFont("C:/Windows/Fonts/simsun.ttc", 18);
        
        // 创建演示节点
        createDemoNodes();
    }
    
    void onUpdate(float dt) override {
        Scene::onUpdate(dt);
        
        time_ += dt;
        
        // 旋转所有演示节点
        for (auto& node : demoNodes_) {
            node->setRotation(time_ * 45.0f);
        }
    }
    
    void onRender(RenderBackend& renderer) override {
        Scene::onRender(renderer);
        
        // 绘制锚点标记
        drawAnchorMarkers(renderer);
        
        // 绘制UI
        drawUI(renderer);
    }

private:
    Ptr<FontAtlas> font_;
    float time_ = 0.0f;
    std::vector<Ptr<Node>> demoNodes_;
    
    void createDemoNodes() {
        // 创建不同锚点的演示节点
        struct DemoConfig {
            const char* name;
            Vec2 anchor;
            Vec2 position;
            Color color;
        };
        
        DemoConfig configs[] = {
            {"左上角(0,0)", Vec2(0.0f, 0.0f), Vec2(150.0f, 150.0f), Color(1.0f, 0.2f, 0.2f, 0.8f)},
            {"中心(0.5,0.5)", Vec2(0.5f, 0.5f), Vec2(400.0f, 150.0f), Color(0.2f, 1.0f, 0.2f, 0.8f)},
            {"右上角(1,0)", Vec2(1.0f, 0.0f), Vec2(650.0f, 150.0f), Color(0.2f, 0.2f, 1.0f, 0.8f)},
            {"左中(0,0.5)", Vec2(0.0f, 0.5f), Vec2(150.0f, 300.0f), Color(1.0f, 1.0f, 0.2f, 0.8f)},
            {"右中(1,0.5)", Vec2(1.0f, 0.5f), Vec2(650.0f, 300.0f), Color(0.2f, 1.0f, 1.0f, 0.8f)},
            {"左下角(0,1)", Vec2(0.0f, 1.0f), Vec2(150.0f, 450.0f), Color(1.0f, 0.2f, 1.0f, 0.8f)},
            {"下中(0.5,1)", Vec2(0.5f, 1.0f), Vec2(400.0f, 450.0f), Color(0.5f, 0.5f, 1.0f, 0.8f)},
            {"右下角(1,1)", Vec2(1.0f, 1.0f), Vec2(650.0f, 450.0f), Color(1.0f, 0.5f, 0.5f, 0.8f)},
        };
        
        for (const auto& config : configs) {
            auto node = makePtr<Node>();
            node->setAnchor(config.anchor);
            node->setPosition(config.position);
            node->setName(config.name);
            addChild(node);
            demoNodes_.push_back(node);
        }
    }
    
    void drawAnchorMarkers(RenderBackend& renderer) {
        for (auto& node : demoNodes_) {
            Vec2 pos = node->getPosition();
            Vec2 anchor = node->getAnchor();
            
            // 绘制节点矩形（80x60）
            float width = 80.0f;
            float height = 60.0f;
            
            // 根据锚点计算矩形位置
            float left = pos.x - width * anchor.x;
            float top = pos.y - height * anchor.y;
            
            // 获取颜色
            Color color;
            if (node->getName().find("左上角") != std::string::npos) color = Color(1.0f, 0.2f, 0.2f, 0.8f);
            else if (node->getName().find("中心") != std::string::npos) color = Color(0.2f, 1.0f, 0.2f, 0.8f);
            else if (node->getName().find("右上角") != std::string::npos) color = Color(0.2f, 0.2f, 1.0f, 0.8f);
            else if (node->getName().find("左中") != std::string::npos) color = Color(1.0f, 1.0f, 0.2f, 0.8f);
            else if (node->getName().find("右中") != std::string::npos) color = Color(0.2f, 1.0f, 1.0f, 0.8f);
            else if (node->getName().find("左下角") != std::string::npos) color = Color(1.0f, 0.2f, 1.0f, 0.8f);
            else if (node->getName().find("下中") != std::string::npos) color = Color(0.5f, 0.5f, 1.0f, 0.8f);
            else color = Color(1.0f, 0.5f, 0.5f, 0.8f);
            
            // 绘制填充矩形
            renderer.fillRect(Rect(left, top, width, height), color);
            
            // 绘制边框
            renderer.drawRect(Rect(left, top, width, height), Color(1.0f, 1.0f, 1.0f, 1.0f), 2.0f);
            
            // 绘制锚点（红色圆点）
            renderer.fillCircle(pos, 5.0f, Color(1.0f, 0.0f, 0.0f, 1.0f));
        }
    }
    
    void drawUI(RenderBackend& renderer) {
        if (!font_) return;
        
        // 标题
        renderer.drawText(*font_, "锚点系统演示",
                         Vec2(320.0f, 20.0f), Color(1.0f, 0.9f, 0.2f, 1.0f));
        
        // 说明
        renderer.drawText(*font_, "红色圆点 = 锚点位置，矩形围绕锚点旋转",
                         Vec2(200.0f, 50.0f), Color(0.8f, 0.8f, 0.8f, 1.0f));
        
        // 锚点标签
        struct LabelInfo {
            const char* text;
            Vec2 pos;
        };
        
        LabelInfo labels[] = {
            {"(0,0)", Vec2(150.0f, 130.0f)},
            {"(0.5,0.5)", Vec2(370.0f, 130.0f)},
            {"(1,0)", Vec2(630.0f, 130.0f)},
            {"(0,0.5)", Vec2(130.0f, 280.0f)},
            {"(1,0.5)", Vec2(630.0f, 280.0f)},
            {"(0,1)", Vec2(130.0f, 430.0f)},
            {"(0.5,1)", Vec2(370.0f, 430.0f)},
            {"(1,1)", Vec2(630.0f, 430.0f)},
        };
        
        for (const auto& label : labels) {
            renderer.drawText(*font_, label.text, label.pos, Color(0.7f, 0.9f, 0.7f, 1.0f));
        }
        
        // API说明
        renderer.drawText(*font_, "API: setAnchor(x, y) - 范围0.0~1.0",
                         Vec2(250.0f, 520.0f), Color(0.6f, 0.8f, 1.0f, 1.0f));
        
        // FPS
        std::stringstream ss;
        ss << "FPS: " << Application::instance().fps();
        renderer.drawText(*font_, ss.str(), Vec2(700.0f, 560.0f), Color(0.5f, 1.0f, 0.5f, 1.0f));
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
    config.title = "锚点系统演示";
    config.width = 800;
    config.height = 600;
    config.vsync = true;
    
    if (!app.init(config)) {
        E2D_LOG_ERROR("初始化失败！");
        return -1;
    }
    
    app.enterScene(makePtr<AnchorDemoScene>());
    app.run();
    
    Logger::shutdown();
    return 0;
}
```

---

## 8. 锚点最佳实践

### 8.1 选择合适的锚点

| 场景 | 推荐锚点 | 原因 |
|------|----------|------|
| UI元素布局 | (0,0) 左上角 | 便于从屏幕边缘定位 |
| 游戏角色 | (0.5,0.5) 中心 | 便于旋转和缩放 |
| 地图/背景 | (0,0) 左上角 | 便于格子布局 |
| 血条/进度条 | (0,0.5) 左中 | 便于从左侧开始增长 |
| 弹窗/面板 | (0.5,0.5) 中心 | 便于屏幕居中 |

### 8.2 注意事项

1. **设置顺序**: 先设置锚点，再设置位置，避免位置偏差
2. **父子关系**: 子节点的位置是相对于父节点的本地坐标
3. **边界计算**: 计算边界框时需要考虑锚点偏移
4. **旋转中心**: 锚点决定了旋转的中心点

### 8.3 常见错误

```cpp
// 错误：先设置位置再设置锚点
sprite->setPosition(Vec2(400, 300));
sprite->setAnchor(0.5f, 0.5f);  // 位置会改变！

// 正确：先设置锚点再设置位置
sprite->setAnchor(0.5f, 0.5f);
sprite->setPosition(Vec2(400, 300));
```
