# Easy2D 场景系统 API 文档

## 1. 场景 (Scene)

场景是游戏的基本组织单位，每个场景代表游戏的一个状态（如主菜单、游戏关卡、结算画面等）。

### 1.1 创建自定义场景
```cpp
#include <easy2d/easy2d.h>

using namespace easy2d;

class MyScene : public Scene {
public:
    // 构造函数
    MyScene() = default;
    
    // 析构函数
    ~MyScene() override = default;
    
    // 场景进入时调用
    void onEnter() override {
        Scene::onEnter();  // 调用父类方法
        E2D_INFO("场景进入");
        
        // 设置背景色
        setBackgroundColor(Color(0.1f, 0.1f, 0.2f, 1.0f));
        
        // 设置视口大小
        setViewportSize(800.0f, 600.0f);
    }
    
    // 场景退出时调用
    void onExit() override {
        E2D_INFO("场景退出");
        Scene::onExit();  // 调用父类方法
    }
    
    // 每帧更新
    void onUpdate(float dt) override {
        Scene::onUpdate(dt);  // 调用父类方法（更新子节点）
        
        // 自定义更新逻辑
    }
    
    // 每帧渲染
    void onRender(RenderBackend& renderer) override {
        Scene::onRender(renderer);  // 调用父类方法（渲染子节点）
        
        // 自定义渲染逻辑
    }
};
```

### 1.2 场景生命周期
```cpp
// 场景进入时调用 onEnter()
// 每帧调用: onUpdate(dt) -> onRender(renderer)
// 场景退出时调用 onExit()
```

### 1.3 场景常用方法
```cpp
// 设置背景色
setBackgroundColor(Color(0.1f, 0.1f, 0.2f, 1.0f));

// 设置视口大小
setViewportSize(800.0f, 600.0f);

// 添加子节点
auto sprite = Sprite::create();
addChild(sprite);

// 移除子节点
removeChild(sprite);

// 移除所有子节点
removeAllChildren();
```

---

## 2. 场景管理器 (SceneManager)

### 2.1 获取场景管理器
```cpp
auto& scenes = Application::instance().scenes();
```

### 2.2 场景切换方式
```cpp
// 方式1: 进入场景（替换当前场景）
app.enterScene(makePtr<MyScene>());

// 方式2: 替换场景
scenes.replaceScene(makePtr<MyScene>());

// 方式3: 替换场景并添加过渡效果
scenes.replaceScene(makePtr<MyScene>(), 
                   TransitionType::Fade,  // 淡入淡出
                   0.2f);                  // 过渡时间（秒）

// 方式4: 压入场景（当前场景保留在栈中）
scenes.pushScene(makePtr<MenuScene>());

// 方式5: 弹出场景（返回上一个场景）
scenes.popScene();

// 方式6: 弹出场景并添加过渡效果
scenes.popScene(TransitionType::Fade, 0.2f);
```

### 2.3 过渡效果类型
```cpp
TransitionType::Fade       // 淡入淡出
TransitionType::SlideLeft  // 向左滑动
TransitionType::SlideRight // 向右滑动
TransitionType::SlideUp    // 向上滑动
TransitionType::SlideDown  // 向下滑动
```

---

## 3. 节点 (Node)

节点是场景中的基本元素，所有可显示对象都继承自Node。

### 3.1 创建节点
```cpp
// 创建普通节点
auto node = makePtr<Node>();

// 创建精灵节点
auto sprite = Sprite::create();

// 创建文本节点
auto text = Text::create("Hello", font);
```

### 3.2 节点变换属性
```cpp
// 位置
node->setPosition(Vec2(100.0f, 200.0f));
Vec2 pos = node->getPosition();

// 缩放
node->setScale(2.0f);           // 等比缩放
node->setScale(1.5f, 0.5f);     // 非等比缩放
float scale = node->getScale();

// 旋转（角度）
node->setRotation(45.0f);
float rotation = node->getRotation();

// 锚点（0-1范围，默认为0,0左上角）
node->setAnchor(0.5f, 0.5f);    // 中心锚点
node->setAnchor(0.0f, 0.0f);    // 左上角锚点
node->setAnchor(1.0f, 1.0f);    // 右下角锚点
Vec2 anchor = node->getAnchor();

// 颜色
node->setColor(Color(1.0f, 0.0f, 0.0f, 1.0f));
Color color = node->getColor();

// 可见性
node->setVisible(true);
bool visible = node->isVisible();

// 名称（用于调试）
node->setName("Player");
std::string name = node->getName();
```

### 3.3 节点层级关系
```cpp
// 添加子节点
parentNode->addChild(childNode);

// 在指定索引位置添加子节点
parentNode->addChild(childNode, 0);  // 添加到最底层

// 移除子节点
parentNode->removeChild(childNode);

// 移除所有子节点
parentNode->removeAllChildren();

// 获取父节点
Node* parent = node->getParent();

// 获取子节点数量
size_t count = node->getChildCount();
```

### 3.4 自定义节点
```cpp
class CustomNode : public Node {
public:
    CustomNode() = default;
    ~CustomNode() override = default;
    
    void onEnter() override {
        Node::onEnter();
        // 节点进入场景时的初始化
    }
    
    void onExit() override {
        // 节点离开场景时的清理
        Node::onExit();
    }
    
    void onUpdate(float dt) override {
        Node::onUpdate(dt);
        // 每帧更新逻辑
    }
    
    void onDraw(RenderBackend& renderer) override {
        // 自定义绘制（可选）
        // 如果不重写，使用默认绘制
    }
    
    // 获取边界框（用于碰撞检测）
    Rect getBoundingBox() const override {
        Vec2 pos = getPosition();
        return Rect(pos.x - width_ / 2, pos.y - height_ / 2, width_, height_);
    }
    
private:
    float width_ = 100.0f;
    float height_ = 100.0f;
};
```

---

## 4. 精灵 (Sprite)

精灵是显示图片的基本节点。

### 4.1 创建精灵
```cpp
// 方式1: 创建空精灵
auto sprite = Sprite::create();

// 方式2: 从纹理创建
auto texture = resources.loadTexture("player.png");
auto sprite = Sprite::create(texture);

// 方式3: 从纹理池获取
auto tex = E2D_TEXTURE_POOL().get("player");
auto sprite = Sprite::create(tex);

// 方式4: 从渲染目标创建
auto rt = RenderTarget::createFromConfig(config);
auto sprite = Sprite::create(rt->getColorTexture());
```

### 4.2 精灵属性
```cpp
// 设置纹理
auto newTexture = resources.loadTexture("enemy.png");
sprite->setTexture(newTexture);

// 获取纹理
Texture* tex = sprite->getTexture();

// 设置纹理矩形（用于精灵图）
sprite->setTextureRect(Rect(0, 0, 32, 32));

// 获取纹理矩形
Rect texRect = sprite->getTextureRect();

// 设置翻转
sprite->setFlipX(true);   // 水平翻转
sprite->setFlipY(false);  // 垂直翻转
```

---

## 5. 文本 (Text)

文本节点用于显示文字。

### 5.1 创建文本
```cpp
// 加载字体
auto font = resources.loadFont("C:/Windows/Fonts/simsun.ttc", 24);

// 创建文本节点
auto text = Text::create("Hello World", font);
text->setPosition(Vec2(400.0f, 300.0f));
addChild(text);
```

### 5.2 文本属性
```cpp
// 设置文本内容
text->setText("新的文本内容");

// 设置文本颜色
text->setTextColor(Color(1.0f, 1.0f, 0.0f, 1.0f));

// 设置位置
text->setPosition(Vec2(100.0f, 50.0f));

// 获取文本尺寸
float width = text->getWidth();
float height = text->getHeight();
```

---

## 6. 按钮 (Button)

### 6.1 图片切换按钮 (ToggleImageButton)
```cpp
// 加载图片
auto soundOn = resources.loadTexture("sound_on.png");
auto soundOff = resources.loadTexture("sound_off.png");

// 创建按钮
auto btn = ToggleImageButton::create();
btn->setStateImages(soundOff, soundOn);  // 关闭状态, 开启状态
btn->setCustomSize(64.0f, 64.0f);
btn->setBorder(Color::Transparent, 0.0f);
btn->setPosition(560.0f, 360.0f);

// 设置状态变化回调
btn->setOnStateChange([](bool on) {
    if (on) {
        E2D_INFO("按钮开启");
    } else {
        E2D_INFO("按钮关闭");
    }
});

// 设置初始状态
btn->setOn(true);

addChild(btn);
```

---

## 7. 空间索引与碰撞检测

### 7.1 启用空间索引
```cpp
class CollisionObject : public Node {
public:
    CollisionObject() {
        // 启用空间索引
        setSpatialIndexed(true);
    }
    
    // 重写边界框
    Rect getBoundingBox() const override {
        Vec2 pos = getPosition();
        return Rect(pos.x - width_ / 2, pos.y - height_ / 2, width_, height_);
    }
    
private:
    float width_ = 50.0f;
    float height_ = 50.0f;
};
```

### 7.2 查询碰撞
```cpp
// 在场景中查询碰撞
auto collisions = scene->queryCollisions();

// 处理碰撞结果
for (const auto& [nodeA, nodeB] : collisions) {
    // nodeA 和 nodeB 发生碰撞
    if (auto objA = dynamic_cast<CollisionObject*>(nodeA)) {
        objA->onCollision();
    }
    if (auto objB = dynamic_cast<CollisionObject*>(nodeB)) {
        objB->onCollision();
    }
}
```

---

## 8. 完整场景示例

```cpp
#include <easy2d/easy2d.h>
#include <easy2d/utils/logger.h>

using namespace easy2d;

// ============================================================================
// 游戏场景
// ============================================================================
class GameScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        
        // 设置背景色
        setBackgroundColor(Color(0.05f, 0.05f, 0.1f, 1.0f));
        
        // 设置视口
        auto& app = Application::instance();
        setViewportSize(static_cast<float>(app.getConfig().width),
                       static_cast<float>(app.getConfig().height));
        
        // 加载资源
        loadResources();
        
        // 创建游戏对象
        createGameObjects();
        
        E2D_INFO("游戏场景初始化完成");
    }
    
    void onUpdate(float dt) override {
        Scene::onUpdate(dt);
        
        // 处理输入
        handleInput();
        
        // 碰撞检测
        checkCollisions();
    }
    
    void onRender(RenderBackend& renderer) override {
        Scene::onRender(renderer);
        
        // 绘制UI
        drawUI(renderer);
    }
    
    void onExit() override {
        E2D_INFO("游戏场景退出");
        Scene::onExit();
    }

private:
    Ptr<FontAtlas> font_;
    Ptr<Sprite> player_;
    std::vector<Ptr<Sprite>> enemies_;
    
    void loadResources() {
        auto& resources = Application::instance().resources();
        font_ = resources.loadFont("C:/Windows/Fonts/simsun.ttc", 20);
    }
    
    void createGameObjects() {
        // 创建玩家
        player_ = Sprite::create();
        player_->setPosition(Vec2(400.0f, 300.0f));
        player_->setName("Player");
        addChild(player_);
        
        // 创建敌人
        for (int i = 0; i < 5; ++i) {
            auto enemy = Sprite::create();
            enemy->setPosition(Vec2(100.0f + i * 100, 100.0f));
            enemy->setName("Enemy" + std::to_string(i));
            addChild(enemy);
            enemies_.push_back(enemy);
        }
    }
    
    void handleInput() {
        auto& input = Application::instance().input();
        
        if (input.isKeyPressed(Key::Escape)) {
            // 返回主菜单
            Application::instance().scenes().replaceScene(
                makePtr<MenuScene>(), TransitionType::Fade, 0.2f);
        }
    }
    
    void checkCollisions() {
        // 空间索引自动处理碰撞检测
    }
    
    void drawUI(RenderBackend& renderer) {
        if (!font_) return;
        
        renderer.drawText(*font_, "Game Scene - Press ESC to exit",
                         Vec2(20.0f, 20.0f), Color(1.0f, 1.0f, 1.0f, 1.0f));
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
    config.title = "场景系统演示";
    config.width = 800;
    config.height = 600;
    config.vsync = true;
    
    if (!app.init(config)) {
        E2D_LOG_ERROR("初始化失败！");
        return -1;
    }
    
    app.enterScene(makePtr<GameScene>());
    app.run();
    
    Logger::shutdown();
    return 0;
}
```
