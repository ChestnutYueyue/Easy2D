# Easy2D 按钮系统 API 文档

## 1. 按钮系统概述

Easy2D 提供了两种按钮组件：
- **Button** - 基础按钮，支持文字、图片、纯色背景
- **ToggleImageButton** - 切换按钮，支持两种状态切换（如开关按钮）

### 1.1 头文件
```cpp
#include <easy2d/ui/button.h>
```

---

## 2. 基础按钮 (Button)

### 2.1 创建按钮
```cpp
// 创建基础按钮
auto button = Button::create();

// 设置文字
button->setText("点击我");

// 设置字体
auto font = resources.loadFont("C:/Windows/Fonts/simsun.ttc", 24);
button->setFont(font);

// 设置点击回调
button->setOnClick([]() {
    E2D_INFO("按钮被点击！");
});

// 添加到场景
button->setPosition(Vec2(400.0f, 300.0f));
addChild(button);
```

### 2.2 按钮文字设置
```cpp
// 设置文字
button->setText("开始游戏");

// 获取文字
String text = button->getText();

// 设置文字颜色
button->setTextColor(Color(1.0f, 1.0f, 1.0f, 1.0f));

// 设置内边距（影响按钮大小）
button->setPadding(Vec2(20.0f, 10.0f));
```

### 2.3 纯色背景按钮
```cpp
auto button = Button::create();
button->setText("纯色按钮");
button->setFont(font);

// 设置三种状态的背景色（正常、悬停、按下）
button->setBackgroundColor(
    Color(0.2f, 0.6f, 1.0f, 1.0f),   // 正常状态
    Color(0.3f, 0.7f, 1.0f, 1.0f),   // 悬停状态
    Color(0.1f, 0.5f, 0.9f, 1.0f)    // 按下状态
);

// 设置边框
button->setBorder(Color(1.0f, 1.0f, 1.0f, 1.0f), 2.0f);

button->setPosition(Vec2(400.0f, 200.0f));
addChild(button);
```

### 2.4 图片背景按钮
```cpp
auto button = Button::create();

// 加载图片
auto normalTex = resources.loadTexture("assets/images/btn_normal.png");
auto hoverTex = resources.loadTexture("assets/images/btn_hover.png");
auto pressedTex = resources.loadTexture("assets/images/btn_pressed.png");

// 设置背景图片（支持三种状态）
button->setBackgroundImage(normalTex, hoverTex, pressedTex);

// 设置图片缩放模式
button->setBackgroundImageScaleMode(ImageScaleMode::ScaleFit);
// 可选模式：
// - Original: 使用原图大小
// - Stretch: 拉伸填充
// - ScaleFit: 等比缩放，保持完整显示
// - ScaleFill: 等比缩放，填充整个区域（可能裁剪）

// 设置自定义大小
button->setCustomSize(200.0f, 60.0f);

button->setPosition(Vec2(400.0f, 300.0f));
button->setOnClick([]() {
    E2D_INFO("图片按钮被点击！");
});
addChild(button);
```

### 2.5 圆角按钮
```cpp
auto button = Button::create();
button->setText("圆角按钮");
button->setFont(font);

// 启用圆角
button->setRoundedCornersEnabled(true);

// 设置圆角半径
button->setCornerRadius(12.0f);

// 设置背景色
button->setBackgroundColor(
    Color(0.2f, 0.8f, 0.4f, 1.0f),
    Color(0.3f, 0.9f, 0.5f, 1.0f),
    Color(0.1f, 0.7f, 0.3f, 1.0f)
);

button->setPosition(Vec2(400.0f, 400.0f));
addChild(button);
```

### 2.6 鼠标光标设置
```cpp
// 设置悬停时的鼠标光标形状
button->setHoverCursor(CursorShape::Hand);      // 手型（默认）
button->setHoverCursor(CursorShape::Arrow);     // 箭头
button->setHoverCursor(CursorShape::IBeam);     // 文本输入
button->setHoverCursor(CursorShape::Crosshair); // 十字准星
```

### 2.7 Alpha遮罩点击检测
```cpp
// 对于不规则形状按钮（如圆形按钮图片），启用Alpha遮罩检测
button->setUseAlphaMaskForHitTest(true);
// 开启后，只有图片非透明区域才能被点击
```

---

## 3. 切换按钮 (ToggleImageButton)

### 3.1 创建切换按钮
```cpp
// 创建切换按钮
auto toggleBtn = ToggleImageButton::create();

// 加载两种状态的图片
auto soundOn = resources.loadTexture("assets/images/sound_on.png");
auto soundOff = resources.loadTexture("assets/images/sound_off.png");

// 设置状态图片（关闭状态，开启状态）
toggleBtn->setStateImages(soundOff, soundOn);

// 设置大小
toggleBtn->setCustomSize(64.0f, 64.0f);

// 设置位置
toggleBtn->setPosition(Vec2(700.0f, 50.0f));

// 设置状态改变回调
toggleBtn->setOnStateChange([](bool on) {
    if (on) {
        E2D_INFO("声音开启");
    } else {
        E2D_INFO("声音关闭");
    }
});

// 设置初始状态
toggleBtn->setOn(true);

addChild(toggleBtn);
```

### 3.2 完整的状态图片设置
```cpp
auto toggleBtn = ToggleImageButton::create();

// 设置所有状态图片（更精细的控制）
toggleBtn->setStateImages(
    soundOffNormal,  // 关闭-正常
    soundOnNormal,   // 开启-正常
    soundOffHover,   // 关闭-悬停（可选）
    soundOnHover,    // 开启-悬停（可选）
    soundOffPressed, // 关闭-按下（可选）
    soundOnPressed   // 开启-按下（可选）
);
```

### 3.3 切换按钮文字
```cpp
auto toggleBtn = ToggleImageButton::create();
toggleBtn->setFont(font);

// 设置两种状态的文字
toggleBtn->setStateText("OFF", "ON");

// 设置两种状态的文字颜色
toggleBtn->setStateTextColor(
    Color(0.8f, 0.8f, 0.8f, 1.0f),  // 关闭状态颜色
    Color(0.2f, 1.0f, 0.2f, 1.0f)   // 开启状态颜色
);
```

### 3.4 控制切换状态
```cpp
// 获取当前状态
bool isOn = toggleBtn->isOn();

// 设置状态
toggleBtn->setOn(true);
toggleBtn->setOn(false);

// 切换状态
toggleBtn->toggle();
```

---

## 4. 自定义按钮

### 4.1 继承Button创建自定义按钮
```cpp
class MenuButton : public Button {
public:
    static Ptr<MenuButton> create(Ptr<FontAtlas> font, const String& text, Function<void()> onClick) {
        auto btn = makePtr<MenuButton>();
        
        // 基础设置
        btn->setFont(font);
        btn->setText(text);
        btn->setPadding(Vec2(0.0f, 0.0f));
        btn->setBackgroundColor(Colors::Transparent, Colors::Transparent, Colors::Transparent);
        btn->setBorder(Colors::Transparent, 0.0f);
        btn->setTextColor(Colors::Black);
        
        // 保存回调
        btn->onClick_ = std::move(onClick);
        btn->setOnClick([wbtn = WeakPtr<MenuButton>(btn)]() {
            if (auto self = wbtn.lock()) {
                if (self->enabled_ && self->onClick_) {
                    self->onClick_();
                }
            }
        });
        
        // 添加悬停效果
        btn->getEventDispatcher().addListener(
            EventType::UIHoverEnter,
            [wbtn = WeakPtr<MenuButton>(btn)](Event&) {
                if (auto self = wbtn.lock()) {
                    if (self->enabled_) {
                        self->setTextColor(Colors::Blue);
                    }
                }
            });
        
        btn->getEventDispatcher().addListener(
            EventType::UIHoverExit,
            [wbtn = WeakPtr<MenuButton>(btn)](Event&) {
                if (auto self = wbtn.lock()) {
                    if (self->enabled_) {
                        self->setTextColor(Colors::Black);
                    }
                }
            });
        
        return btn;
    }
    
    void setEnabled(bool enabled) {
        enabled_ = enabled;
        setTextColor(enabled ? Colors::Black : Colors::LightGray);
    }
    
    bool isEnabled() const { return enabled_; }

private:
    bool enabled_ = true;
    Function<void()> onClick_;
};
```

---

## 5. 完整示例

```cpp
#include <easy2d/easy2d.h>
#include <easy2d/utils/logger.h>

using namespace easy2d;

// ============================================================================
// 按钮演示场景
// ============================================================================
class ButtonDemoScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        setBackgroundColor(Color(0.1f, 0.1f, 0.15f, 1.0f));
        
        // 加载字体
        auto& resources = Application::instance().resources();
        font_ = resources.loadFont("C:/Windows/Fonts/simsun.ttc", 20);
        
        // 创建各种按钮
        createTextButton();
        createColorButton();
        createRoundedButton();
        createToggleButton();
    }
    
    void onRender(RenderBackend& renderer) override {
        Scene::onRender(renderer);
        
        // 绘制标题
        if (font_) {
            renderer.drawText(*font_, "Easy2D 按钮系统演示",
                            Vec2(280.0f, 30.0f), Color(1.0f, 0.9f, 0.2f, 1.0f));
        }
    }

private:
    Ptr<FontAtlas> font_;
    
    void createTextButton() {
        auto btn = Button::create();
        btn->setText("文字按钮");
        btn->setFont(font_);
        btn->setPosition(Vec2(150.0f, 150.0f));
        btn->setOnClick([]() {
            E2D_INFO("文字按钮被点击");
        });
        addChild(btn);
    }
    
    void createColorButton() {
        auto btn = Button::create();
        btn->setText("彩色按钮");
        btn->setFont(font_);
        btn->setBackgroundColor(
            Color(0.2f, 0.6f, 1.0f, 1.0f),
            Color(0.3f, 0.7f, 1.0f, 1.0f),
            Color(0.1f, 0.5f, 0.9f, 1.0f)
        );
        btn->setBorder(Color(1.0f, 1.0f, 1.0f, 0.5f), 2.0f);
        btn->setPosition(Vec2(400.0f, 150.0f));
        btn->setOnClick([]() {
            E2D_INFO("彩色按钮被点击");
        });
        addChild(btn);
    }
    
    void createRoundedButton() {
        auto btn = Button::create();
        btn->setText("圆角按钮");
        btn->setFont(font_);
        btn->setRoundedCornersEnabled(true);
        btn->setCornerRadius(15.0f);
        btn->setBackgroundColor(
            Color(0.2f, 0.8f, 0.4f, 1.0f),
            Color(0.3f, 0.9f, 0.5f, 1.0f),
            Color(0.1f, 0.7f, 0.3f, 1.0f)
        );
        btn->setPosition(Vec2(200.0f, 300.0f));
        btn->setOnClick([]() {
            E2D_INFO("圆角按钮被点击");
        });
        addChild(btn);
    }
    
    void createToggleButton() {
        auto btn = ToggleImageButton::create();
        btn->setFont(font_);
        btn->setStateText("声音: 关", "声音: 开");
        btn->setStateTextColor(
            Color(0.8f, 0.8f, 0.8f, 1.0f),
            Color(0.2f, 1.0f, 0.2f, 1.0f)
        );
        btn->setBackgroundColor(
            Color(0.3f, 0.3f, 0.3f, 1.0f),
            Color(0.4f, 0.4f, 0.4f, 1.0f),
            Color(0.2f, 0.2f, 0.2f, 1.0f)
        );
        btn->setBorder(Color(0.6f, 0.6f, 0.6f, 1.0f), 2.0f);
        btn->setCustomSize(120.0f, 40.0f);
        btn->setPosition(Vec2(600.0f, 300.0f));
        btn->setOnStateChange([](bool on) {
            E2D_INFO("切换状态: {}", on ? "开启" : "关闭");
        });
        btn->setOn(true);
        addChild(btn);
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
    config.title = "按钮系统演示";
    config.width = 800;
    config.height = 450;
    config.vsync = true;
    
    if (!app.init(config)) {
        E2D_LOG_ERROR("初始化失败！");
        return -1;
    }
    
    app.enterScene(makePtr<ButtonDemoScene>());
    app.run();
    
    Logger::shutdown();
    return 0;
}
```

---

## 6. 按钮事件类型

```cpp
// 按钮相关事件类型
EventType::UIHoverEnter    // 鼠标进入按钮
EventType::UIHoverExit     // 鼠标离开按钮
EventType::UIPressed       // 鼠标按下
EventType::UIReleased      // 鼠标释放
EventType::UIClicked       // 点击完成

// 使用事件监听器
button->getEventDispatcher().addListener(
    EventType::UIHoverEnter,
    [](Event&) {
        E2D_INFO("鼠标进入按钮");
    });

button->getEventDispatcher().addListener(
    EventType::UIHoverExit,
    [](Event&) {
        E2D_INFO("鼠标离开按钮");
    });
```

---

## 7. 最佳实践

### 7.1 按钮大小设置
```cpp
// 方式1: 根据文字自动调整（设置字体和文字后自动计算）
button->setFont(font);
button->setText("点击我");
button->setPadding(Vec2(20.0f, 10.0f));  // 设置内边距

// 方式2: 自定义大小
button->setCustomSize(200.0f, 60.0f);

// 方式3: 根据图片大小（使用Original模式）
button->setBackgroundImage(texture);
button->setBackgroundImageScaleMode(ImageScaleMode::Original);
```

### 7.2 按钮状态颜色搭配
```cpp
// 推荐的颜色搭配
// 蓝色主题
button->setBackgroundColor(
    Color(0.2f, 0.5f, 1.0f, 1.0f),  // 正常：蓝色
    Color(0.3f, 0.6f, 1.0f, 1.0f),  // 悬停：亮蓝
    Color(0.1f, 0.4f, 0.9f, 1.0f)   // 按下：深蓝
);

// 绿色主题
button->setBackgroundColor(
    Color(0.2f, 0.7f, 0.3f, 1.0f),  // 正常：绿色
    Color(0.3f, 0.8f, 0.4f, 1.0f),  // 悬停：亮绿
    Color(0.1f, 0.6f, 0.2f, 1.0f)   // 按下：深绿
);

// 红色主题
button->setBackgroundColor(
    Color(0.9f, 0.2f, 0.2f, 1.0f),  // 正常：红色
    Color(1.0f, 0.3f, 0.3f, 1.0f),  // 悬停：亮红
    Color(0.8f, 0.1f, 0.1f, 1.0f)   // 按下：深红
);
```

### 7.3 禁用按钮
```cpp
// 自定义禁用效果
class MyButton : public Button {
public:
    void setEnabled(bool enabled) {
        enabled_ = enabled;
        if (enabled) {
            setTextColor(Colors::White);
            setBackgroundColor(
                Color(0.2f, 0.6f, 1.0f, 1.0f),
                Color(0.3f, 0.7f, 1.0f, 1.0f),
                Color(0.1f, 0.5f, 0.9f, 1.0f)
            );
        } else {
            setTextColor(Color(0.5f, 0.5f, 0.5f, 1.0f));
            setBackgroundColor(
                Color(0.3f, 0.3f, 0.3f, 1.0f),
                Color(0.3f, 0.3f, 0.3f, 1.0f),
                Color(0.3f, 0.3f, 0.3f, 1.0f)
            );
        }
    }
    
private:
    bool enabled_ = true;
};
```
