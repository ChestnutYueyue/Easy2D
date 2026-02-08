# Easy2D 动画系统 API 文档

## 目录

1. [概述](#1-概述)
2. [核心概念](#2-核心概念)
3. [AnimationClip - 动画片段](#3-animationclip---动画片段)
4. [AnimatedSprite - 动画精灵](#4-animatedsprite---动画精灵)
5. [AnimationController - 动画控制器](#5-animationcontroller---动画控制器)
6. [SpriteFrame - 精灵帧](#6-spriteframe---精灵帧)
7. [缓存系统](#7-缓存系统)
8. [帧属性系统](#8-帧属性系统)
9. [动画事件](#9-动画事件)
10. [完整示例](#10-完整示例)
    - 10.1 [角色移动动画（帧范围限制方式）](#101-角色移动动画帧范围限制方式)
    - 10.2 [角色移动动画（动画字典方式）](#102-角色移动动画动画字典方式)
    - 10.3 [攻击动画与关键帧](#103-攻击动画与关键帧)
    - 10.4 [动画缓存最佳实践](#104-动画缓存最佳实践)

---

## 1. 概述

Easy2D 动画系统是一个融合 Cocos2d-x 设计理念的专业动画解决方案，具有以下特点：

- **数据与行为分离**：`AnimationClip` 纯数据可复用，一份数据可被多个节点共享
- **SpriteFrame 中间抽象**：解耦纹理物理存储与逻辑帧，提升渲染性能
- **全局缓存系统**：`AnimationCache` 和 `SpriteFrameCache` 避免重复创建
- **类型安全属性**：强类型枚举替代字符串键，编译期检查
- **灵活扩展**：保留不固定数据能力，支持自定义属性

### 架构层次

```
+--------------------------------------------------+
|  组件层  AnimatedSprite : Sprite                 |
|          面向用户的顶层API，接入场景图             |
+--------------------------------------------------+
|  事件层  AnimationEvent                           |
|          帧事件、关键帧回调、完成通知               |
+--------------------------------------------------+
|  控制层  AnimationController                      |
|          播放状态机、帧推进、插值计算               |
+--------------------------------------------------+
|  缓存层  AnimationCache / SpriteFrameCache       |
|          动画数据缓存、精灵帧全局管理               |
+--------------------------------------------------+
|  数据层  AnimationClip / AnimationFrame          |
|          SpriteFrame / FramePropertySet          |
+--------------------------------------------------+
```

---

## 2. 核心概念

### 2.1 精灵图 (Sprite Sheet)

精灵图是将多帧动画合并到一张大图中的技术，减少纹理切换开销。

```
+---+---+---+---+  第1行 (索引0-3): 向下走
| 0 | 1 | 2 | 3 |
+---+---+---+---+  第2行 (索引4-7): 向右走
| 4 | 5 | 6 | 7 |
+---+---+---+---+  第3行 (索引8-11): 向上走
| 8 | 9 |10 |11 |
+---+---+---+---+  第4行 (索引12-15): 向左走
|12 |13 |14 |15 |
+---+---+---+---+
```

### 2.2 动画播放流程

```cpp
// 1. 创建/加载 AnimationClip（动画数据）
auto clip = AnimationClip::createFromGrid(texture, 96, 96, 125.0f, 16);

// 2. 创建 AnimatedSprite（渲染节点）
auto sprite = AnimatedSprite::create(clip);

// 3. 添加到场景
scene->addChild(sprite);

// 4. 播放动画
sprite->play();
```

---

## 3. AnimationClip - 动画片段

`AnimationClip` 是纯数据对象，包含一组动画帧，可被多个 `AnimatedSprite` 共享。

### 3.1 从精灵图创建

```cpp
#include <easy2d/easy2d.h>

using namespace easy2d;

// 方式1: 从精灵图网格创建（自动按顺序提取所有帧）
auto texture = E2D_TEXTURE_POOL().get("player.png");
auto walkClip = AnimationClip::createFromGrid(
    texture,           // 纹理
    96,                // 帧宽度
    96,                // 帧高度
    125.0f,            // 每帧持续时间(毫秒)
    16,                // 总帧数(-1表示自动计算)
    0,                 // 帧间距
    0                  // 边距
);
walkClip->setLooping(true);

// 方式2: 从精灵图创建，指定特定帧索引
std::vector<int> downFrames = {0, 1, 2, 3};
auto downClip = AnimationClip::createFromGridIndices(
    texture, 96, 96,
    downFrames,        // 只使用指定索引的帧
    125.0f
);
```

### 3.2 手动构建动画

```cpp
// 创建空动画片段
auto clip = AnimationClip::create("explosion");

// 手动添加帧
for (int i = 0; i < 8; ++i) {
    AnimationFrame frame;
    frame.spriteFrame = E2D_SPRITE_FRAME_CACHE().getSpriteFrame("explosion#" + std::to_string(i));
    frame.delay = 100.0f;  // 100ms
    
    // 设置帧属性
    frame.properties.withColorTint(Color::Red);
    
    clip->addFrame(frame);
}

// 设置全局属性
clip->setLooping(false);
```

### 3.3 AnimationClip API

| 方法 | 说明 |
|------|------|
| `createFromGrid()` | 从精灵图网格创建动画 |
| `createFromGridIndices()` | 从精灵图创建，指定帧索引 |
| `addFrame()` | 添加帧 |
| `insertFrame()` | 插入帧 |
| `removeFrame()` | 移除帧 |
| `getFrame()` | 获取指定帧 |
| `getFrameCount()` | 获取总帧数 |
| `setLooping()` | 设置循环播放 |
| `isLooping()` | 是否循环 |
| `getTotalDuration()` | 获取总时长 |
| `getMaxFrameSize()` | 获取最大帧尺寸 |

---

## 4. AnimatedSprite - 动画精灵

`AnimatedSprite` 是动画系统的核心节点类，继承自 `Sprite`，负责动画渲染和播放控制。

### 4.1 基础使用

```cpp
class GameScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        
        // 加载纹理
        auto texture = E2D_TEXTURE_POOL().get("player.png");
        
        // 创建动画片段
        auto clip = AnimationClip::createFromGrid(texture, 96, 96, 125.0f, 16);
        clip->setLooping(true);
        
        // 创建动画精灵
        player_ = AnimatedSprite::create(clip);
        player_->setPosition(Vec2(400, 300));
        
        // 添加到场景
        addChild(player_);
        
        // 播放动画
        player_->play();
    }
    
private:
    Ptr<AnimatedSprite> player_;
};
```

### 4.2 动画字典（多动画管理）

```cpp
void onEnter() override {
    Scene::onEnter();
    
    auto texture = E2D_TEXTURE_POOL().get("player.png");
    
    // 创建各方向动画
    auto idleClip = AnimationClip::createFromGridIndices(texture, 96, 96, {0}, 200.0f);
    auto walkDownClip = AnimationClip::createFromGridIndices(texture, 96, 96, {0,1,2,3}, 125.0f);
    auto walkRightClip = AnimationClip::createFromGridIndices(texture, 96, 96, {4,5,6,7}, 125.0f);
    auto walkUpClip = AnimationClip::createFromGridIndices(texture, 96, 96, {8,9,10,11}, 125.0f);
    auto walkLeftClip = AnimationClip::createFromGridIndices(texture, 96, 96, {12,13,14,15}, 125.0f);
    
    // 创建动画精灵
    player_ = AnimatedSprite::create();
    
    // 添加到动画字典
    player_->addAnimation("idle", idleClip);
    player_->addAnimation("walk_down", walkDownClip);
    player_->addAnimation("walk_right", walkRightClip);
    player_->addAnimation("walk_up", walkUpClip);
    player_->addAnimation("walk_left", walkLeftClip);
    
    // 播放指定动画
    player_->play("walk_down", true);
    
    addChild(player_);
}

void handleInput() {
    auto& input = Application::instance().input();
    
    if (input.isKeyDown(Key::W)) {
        player_->play("walk_up", true);
    } else if (input.isKeyDown(Key::S)) {
        player_->play("walk_down", true);
    } else if (input.isKeyDown(Key::A)) {
        player_->play("walk_left", true);
    } else if (input.isKeyDown(Key::D)) {
        player_->play("walk_right", true);
    } else {
        player_->play("idle", true);
    }
}
```

### 4.3 帧范围限制（精灵图动画）

当使用包含多方向动画的精灵图时，可以使用帧范围限制功能：

#### 精灵图布局与方向映射

典型的角色行走精灵图布局（4x4=16帧）：

```
+---+---+---+---+  第1行 (索引 0-3): 向下走（正面）
| 0 | 1 | 2 | 3 |
+---+---+---+---+  第2行 (索引 4-7): 向右走（右侧面）
| 4 | 5 | 6 | 7 |
+---+---+---+---+  第3行 (索引 8-11): 向上走（背面）
| 8 | 9 |10 |11 |
+---+---+---+---+  第4行 (索引 12-15): 向左走（左侧面）
|12 |13 |14 |15 |
+---+---+---+---+
```

> **注意**：实际精灵图的布局可能不同，请根据您的资源调整帧索引映射。

#### 方式一：使用帧范围限制（推荐）

参考示例：`animation_demo/main.cpp`

```cpp
// 方向枚举 - 顺序必须与精灵图布局匹配
enum class Direction {
    Down,   // 0 -> 帧 0-3
    Right,  // 1 -> 帧 4-7
    Up,     // 2 -> 帧 8-11
    Left    // 3 -> 帧 12-15
};

void onEnter() override {
    Scene::onEnter();
    
    // 加载包含所有方向的精灵图（16帧：4方向 x 4帧）
    auto texture = E2D_TEXTURE_POOL().get("player.png");
    
    // 创建包含所有帧的动画
    auto walkClip = AnimationClip::createFromGrid(texture, 96, 96, 125.0f, 16);
    walkClip->setLooping(true);
    
    player_ = AnimatedSprite::create(walkClip);
    player_->setPosition(Vec2(400, 300));
    
    // 禁用帧变换（精灵图动画不需要）
    player_->setApplyFrameTransform(false);
    
    // 设置初始帧范围（向下走：帧 0-3）
    player_->setFrameRange(0, 3);
    
    addChild(player_);
    player_->play();
}

void moveCharacter(Direction dir, float dt) {
    // 计算新方向的帧范围
    int newFrameStart = static_cast<int>(dir) * 4;  // 每个方向4帧
    int newFrameEnd = newFrameStart + 4 - 1;
    
    // 方向改变时，切换到新方向的帧范围
    if (currentDir_ != dir) {
        character_->setFrameRange(newFrameStart, newFrameEnd);
        character_->setFrameIndex(newFrameStart);
    }
    
    // 确保动画在播放
    if (!character_->isPlaying()) {
        character_->play();
    }
    
    currentDir_ = dir;
    
    // 移动角色...
}
```

#### 方式二：使用动画字典

参考示例：`sprite_animation_demo/main.cpp`

```cpp
// 方向枚举
enum class Direction {
    Down,   // 向下
    Left,   // 向左
    Right,  // 向右
    Up      // 向上
};

void createCharacter() {
    auto texture = E2D_TEXTURE_POOL().get("player.png");
    float frameDurationMs = 125.0f;
    int spacing = 0;
    
    // 为每个方向创建独立的动画片段
    auto walkUp = AnimationClip::createFromGridIndices(
        texture, 96, 96, {0, 1, 2, 3}, frameDurationMs, spacing);
    
    auto walkRight = AnimationClip::createFromGridIndices(
        texture, 96, 96, {4, 5, 6, 7}, frameDurationMs, spacing);
    
    auto walkLeft = AnimationClip::createFromGridIndices(
        texture, 96, 96, {8, 9, 10, 11}, frameDurationMs, spacing);
    
    auto walkDown = AnimationClip::createFromGridIndices(
        texture, 96, 96, {12, 13, 14, 15}, frameDurationMs, spacing);
    
    // 创建 AnimatedSprite 并注册命名动画
    character_ = AnimatedSprite::create();
    character_->addAnimation("walk_up", walkUp);
    character_->addAnimation("walk_right", walkRight);
    character_->addAnimation("walk_left", walkLeft);
    character_->addAnimation("walk_down", walkDown);
    
    // 播放初始动画
    character_->play("walk_down", true);
    addChild(character_);
}

void moveCharacter(Direction dir, float dt) {
    // 切换动画（仅在方向改变或未播放时）
    if (currentDir_ != dir || !character_->isPlaying()) {
        std::string animName;
        switch (dir) {
            case Direction::Up:    animName = "walk_up";    break;
            case Direction::Down:  animName = "walk_down";  break;
            case Direction::Left:  animName = "walk_left";  break;
            case Direction::Right: animName = "walk_right"; break;
        }
        
        if (character_->getCurrentAnimationName() != animName) {
            character_->play(animName, true);
        } else if (!character_->isPlaying()) {
            character_->resume();
        }
    }
    
    currentDir_ = dir;
    // 移动角色...
}
```

#### 两种方式对比

| 特性 | 帧范围限制 | 动画字典 |
|------|-----------|----------|
| 内存占用 | 低（共享一份 AnimationClip） | 较高（多份 AnimationClip） |
| 切换速度 | 快（只需改范围） | 稍慢（切换 Clip） |
| 灵活性 | 适合统一规格精灵图 | 适合复杂/不规则动画 |
| 代码复杂度 | 简单 | 稍复杂 |
| 适用场景 | 角色行走、简单循环动画 | 复杂角色、多动作管理 |
```

### 4.4 播放控制

```cpp
// 播放控制
sprite->play();           // 播放
sprite->pause();          // 暂停
sprite->resume();         // 恢复
sprite->stop();           // 停止（回到第一帧）
sprite->reset();          // 重置

// 属性控制
sprite->setLooping(true);           // 设置循环
sprite->setPlaybackSpeed(2.0f);     // 2倍速播放
sprite->setPlaybackSpeed(0.5f);     // 0.5倍速播放

// 帧控制
sprite->setFrameIndex(5);           // 跳转到第5帧
sprite->nextFrame();                // 下一帧
sprite->prevFrame();                // 上一帧

// 状态查询
bool playing = sprite->isPlaying();
bool paused = sprite->isPaused();
size_t currentFrame = sprite->getCurrentFrameIndex();
size_t totalFrames = sprite->getTotalFrames();
```

### 4.5 回调函数

```cpp
// 动画完成回调
sprite->setCompletionCallback([]() {
    E2D_INFO("动画播放完成！");
});

// 关键帧回调（ANI 格式中的 SetFlag）
sprite->setKeyframeCallback([](int flagIndex) {
    E2D_INFO("触发关键帧: {}", flagIndex);
    
    // 根据 flagIndex 执行不同逻辑
    switch (flagIndex) {
        case 1:  // 攻击判定开始
            enableAttackHitbox();
            break;
        case 2:  // 攻击判定结束
            disableAttackHitbox();
            break;
        case 3:  // 播放音效
            AudioEngine::play("attack.wav");
            break;
    }
});

// 音效触发回调
sprite->setSoundTriggerCallback([](const std::string& path) {
    AudioEngine::play(path);
});
```

### 4.6 AnimatedSprite API

| 方法 | 说明 |
|------|------|
| `create()` | 静态创建方法 |
| `setAnimationClip()` | 设置动画片段 |
| `addAnimation()` | 添加命名动画 |
| `play()` | 播放动画 |
| `pause()` | 暂停 |
| `resume()` | 恢复 |
| `stop()` | 停止 |
| `setLooping()` | 设置循环 |
| `setPlaybackSpeed()` | 设置播放速度 |
| `setFrameIndex()` | 设置当前帧 |
| `setFrameRange()` | 设置帧范围限制 |
| `clearFrameRange()` | 清除帧范围 |
| `setApplyFrameTransform()` | 是否应用帧变换 |
| `setCompletionCallback()` | 完成回调 |
| `setKeyframeCallback()` | 关键帧回调 |

---

## 5. AnimationController - 动画控制器

`AnimationController` 是纯播放逻辑控制器，不持有渲染资源。`AnimatedSprite` 内部使用它来控制动画播放。

### 5.1 独立使用

```cpp
// 创建控制器
AnimationController controller;

// 绑定动画数据
controller.setClip(clip);

// 设置回调
controller.setFrameChangeCallback([](size_t oldIdx, size_t newIdx, const AnimationFrame& frame) {
    E2D_INFO("帧切换: {} -> {}", oldIdx, newIdx);
});

// 播放
controller.play();

// 在更新循环中调用
void onUpdate(float dt) override {
    controller.update(dt * 1000);  // 转换为毫秒
}
```

### 5.2 插值支持

```cpp
// 检查是否正在插值
if (controller.isInterpolating()) {
    float t = controller.getInterpolationFactor();
    // t 范围 [0, 1]，表示两帧之间的插值进度
}

// 获取当前帧
const AnimationFrame& frame = controller.getCurrentFrame();

// 手动设置帧
controller.setFrameIndex(5);
```

---

## 6. SpriteFrame - 精灵帧

`SpriteFrame` 是纹理和帧之间的中间抽象层，解耦纹理物理存储与逻辑帧。

### 6.1 创建 SpriteFrame

```cpp
// 方式1: 从纹理和矩形创建
auto texture = E2D_TEXTURE_POOL().get("player.png");
Rect frameRect(0, 0, 96, 96);  // x, y, width, height
auto frame = SpriteFrame::create(texture, frameRect);

// 方式2: 带偏移和原始尺寸（用于图集）
Vec2 offset(0, 0);
Size originalSize(96, 96);
auto frame = SpriteFrame::create(texture, frameRect, offset, originalSize);

// 设置名称（用于缓存）
frame->setName("player_walk_0");
```

### 6.2 SpriteFrame API

| 方法 | 说明 |
|------|------|
| `create()` | 静态创建 |
| `setTexture()` | 设置纹理 |
| `getTexture()` | 获取纹理 |
| `setRect()` | 设置矩形区域 |
| `getRect()` | 获取矩形区域 |
| `setOffset()` | 设置偏移 |
| `getOffset()` | 获取偏移 |
| `setOriginalSize()` | 设置原始尺寸 |
| `getOriginalSize()` | 获取原始尺寸 |
| `setRotated()` | 设置旋转标志 |
| `isRotated()` | 是否旋转 |
| `setName()` | 设置名称 |
| `getName()` | 获取名称 |
| `isValid()` | 是否有效 |

---

## 7. 缓存系统

### 7.1 AnimationCache - 动画缓存

```cpp
// 从文件加载（自动缓存）
auto clip = E2D_ANIMATION_CACHE().loadClip("animations/explosion.ani");

// 手动添加
auto myClip = createMyAnimation();
E2D_ANIMATION_CACHE().addClip(myClip, "my_animation");

// 从缓存获取
auto cached = E2D_ANIMATION_CACHE().getClip("my_animation");

// 检查是否存在
bool has = E2D_ANIMATION_CACHE().has("my_animation");

// 移除
E2D_ANIMATION_CACHE().removeClip("my_animation");

// 清理未使用的
E2D_ANIMATION_CACHE().removeUnusedClips();

// 清空
E2D_ANIMATION_CACHE().clear();

// 获取数量
size_t count = E2D_ANIMATION_CACHE().count();
```

### 7.2 SpriteFrameCache - 精灵帧缓存

```cpp
// 添加单个帧
auto frame = SpriteFrame::create(texture, rect);
E2D_SPRITE_FRAME_CACHE().addSpriteFrame(frame, "frame_0");

// 从纹理图集批量添加
E2D_SPRITE_FRAME_CACHE().addSpriteFramesFromGrid(
    "spritesheet.png",
    32,     // 帧宽
    32,     // 帧高
    -1,     // 帧数(-1自动计算)
    0,      // 间距
    0       // 边距
);

// 获取帧
auto frame = E2D_SPRITE_FRAME_CACHE().getSpriteFrame("spritesheet.png#5");

// 获取或创建
auto frame = E2D_SPRITE_FRAME_CACHE().getOrCreateFromFile("player.png", 0);

// 其他操作与 AnimationCache 类似
```

---

## 8. 帧属性系统

`FramePropertySet` 提供类型安全的帧属性管理。

### 8.1 内置属性键

```cpp
enum class FramePropertyKey : uint32 {
    // 事件触发
    SetFlag             = 0x0001,   // int: 关键帧回调索引
    PlaySound           = 0x0002,   // string: 音效路径

    // 变换属性
    ImageRate           = 0x0010,   // Vec2: 缩放比例
    ImageRotate         = 0x0011,   // float: 旋转角度
    ImageOffset         = 0x0012,   // Vec2: 位置偏移

    // 视觉效果
    BlendLinearDodge    = 0x0020,   // bool: 线性减淡
    BlendAdditive       = 0x0021,   // bool: 加法混合
    ColorTint           = 0x0022,   // Color: RGBA颜色

    // 控制标记
    Interpolation       = 0x0030,   // bool: 启用插值
    Loop                = 0x0031,   // bool: 循环标记
};
```

### 8.2 使用属性

```cpp
// 创建属性集
FramePropertySet props;

// 设置属性
props.set(FramePropertyKey::SetFlag, 1);
props.set(FramePropertyKey::ColorTint, Color::Red);
props.set(FramePropertyKey::ImageRate, Vec2(2.0f, 2.0f));

// 链式API
props.withSetFlag(1)
      .withColorTint(Color::Red)
      .withImageRate(Vec2(2.0f, 2.0f))
      .withInterpolation(true);

// 获取属性
auto flag = props.get<int>(FramePropertyKey::SetFlag);
if (flag.has_value()) {
    int value = flag.value();
}

// 带默认值的获取
auto rate = props.getOr<Vec2>(FramePropertyKey::ImageRate, Vec2(1.0f, 1.0f));

// 检查是否存在
bool has = props.has(FramePropertyKey::SetFlag);

// 自定义属性
props.setCustom("my_key", 42);
auto custom = props.getCustom("my_key");
```

---

## 9. 动画事件

### 9.1 事件类型

```cpp
enum class AnimationEventType : uint32_t {
    FrameChanged,    // 帧切换
    KeyframeHit,     // 关键帧触发
    SoundTrigger,    // 音效触发
    AnimationStart,  // 动画开始
    AnimationEnd,    // 动画结束
    AnimationLoop,   // 动画循环
};
```

### 9.2 事件数据

```cpp
struct AnimationEvent {
    AnimationEventType type;
    size_t frameIndex = 0;
    size_t previousFrameIndex = 0;
    int keyframeFlag = -1;
    std::string soundPath;
    Node* source = nullptr;
};
```

---

## 10. 完整示例

### 10.1 角色移动动画（帧范围限制方式）

参考示例：`animation_demo/main.cpp`

```cpp
#include <easy2d/easy2d.h>
#include <iostream>

using namespace easy2d;

// 方向枚举 - 顺序与精灵图布局匹配
// 精灵图布局：0-3下, 4-7右, 8-11上, 12-15左
enum class Direction {
    Down,   // 0 -> 帧 0-3
    Right,  // 1 -> 帧 4-7
    Up,     // 2 -> 帧 8-11
    Left    // 3 -> 帧 12-15
};

class CharacterScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        setBackgroundColor(Color(0.1f, 0.1f, 0.15f, 1.0f));
        
        createCharacter();
        
        E2D_INFO("角色动画场景初始化完成");
    }
    
    void onUpdate(float dt) override {
        Scene::onUpdate(dt);
        handleInput(dt);
    }
    
private:
    Ptr<AnimatedSprite> character_;
    Direction currentDir_ = Direction::Down;
    float moveSpeed_ = 150.0f;
    bool isMoving_ = false;
    
    static constexpr int kFrameWidth = 96;
    static constexpr int kFrameHeight = 96;
    static constexpr int kFramesPerDir = 4;
    
    void createCharacter() {
        // 加载精灵图
        auto texture = E2D_TEXTURE_POOL().get("player.png");
        if (!texture) {
            E2D_ERROR("无法加载角色精灵图！");
            return;
        }
        
        // 创建动画片段（16帧：4方向 x 4帧）
        auto walkClip = AnimationClip::createFromGrid(
            texture, kFrameWidth, kFrameHeight, 125.0f, 16, 0);
        walkClip->setLooping(true);
        
        // 创建动画精灵
        character_ = AnimatedSprite::create(walkClip);
        character_->setApplyFrameTransform(false);  // 精灵图动画不需要
        character_->setFrameRange(0, 3);            // 初始向下
        character_->setPosition(Vec2(450.0f, 300.0f));
        
        addChild(character_);
        character_->play();
    }
    
    void handleInput(float dt) {
        auto& input = Application::instance().input();
        
        isMoving_ = false;
        
        if (input.isKeyDown(Key::W) || input.isKeyDown(Key::Up)) {
            moveCharacter(Direction::Up, dt);
        } else if (input.isKeyDown(Key::S) || input.isKeyDown(Key::Down)) {
            moveCharacter(Direction::Down, dt);
        } else if (input.isKeyDown(Key::A) || input.isKeyDown(Key::Left)) {
            moveCharacter(Direction::Left, dt);
        } else if (input.isKeyDown(Key::D) || input.isKeyDown(Key::Right)) {
            moveCharacter(Direction::Right, dt);
        }
        
        if (!isMoving_) {
            character_->pause();
        }
    }
    
    void moveCharacter(Direction dir, float dt) {
        // 计算帧范围
        int frameStart = static_cast<int>(dir) * kFramesPerDir;
        int frameEnd = frameStart + kFramesPerDir - 1;
        
        // 方向改变时切换帧范围
        if (currentDir_ != dir) {
            character_->setFrameRange(frameStart, frameEnd);
            character_->setFrameIndex(frameStart);
            currentDir_ = dir;
        }
        
        // 确保动画在播放
        if (!character_->isPlaying()) {
            character_->resume();
        }
        
        isMoving_ = true;
        
        // 移动角色
        Vec2 pos = character_->getPosition();
        switch (dir) {
            case Direction::Down:  pos.y += moveSpeed_ * dt; break;
            case Direction::Up:    pos.y -= moveSpeed_ * dt; break;
            case Direction::Left:  pos.x -= moveSpeed_ * dt; break;
            case Direction::Right: pos.x += moveSpeed_ * dt; break;
        }
        character_->setPosition(pos);
    }
};

int main() {
    Logger::init();
    Logger::setLevel(LogLevel::Debug);
    
    auto& app = Application::instance();
    
    AppConfig config;
    config.title = "角色动画示例";
    config.width = 900;
    config.height = 600;
    config.vsync = true;
    
    if (!app.init(config)) {
        E2D_LOG_ERROR("初始化失败！");
        return -1;
    }
    
    app.resources().addSearchPath("assets");
    app.enterScene(makePtr<CharacterScene>());
    
    app.run();
    Logger::shutdown();
    return 0;
}
```

### 10.2 角色移动动画（动画字典方式）

参考示例：`sprite_animation_demo/main.cpp`

```cpp
#include <easy2d/easy2d.h>
#include <iostream>

using namespace easy2d;

enum class Direction {
    Down,   // 向下
    Left,   // 向左
    Right,  // 向右
    Up      // 向上
};

class CharacterScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        setBackgroundColor(Color(0.1f, 0.1f, 0.15f, 1.0f));
        
        createCharacter();
        
        E2D_INFO("角色动画场景初始化完成");
    }
    
    void onUpdate(float dt) override {
        Scene::onUpdate(dt);
        handleInput(dt);
    }
    
private:
    Ptr<AnimatedSprite> character_;
    Direction currentDir_ = Direction::Down;
    float moveSpeed_ = 150.0f;
    bool isMoving_ = false;
    
    static constexpr int kFrameWidth = 96;
    static constexpr int kFrameHeight = 96;
    static constexpr int kSpacing = 0;
    
    void createCharacter() {
        auto texture = E2D_TEXTURE_POOL().get("player.png");
        if (!texture) {
            E2D_ERROR("无法加载角色精灵图！");
            return;
        }
        
        float frameDurationMs = 125.0f;
        
        // 为每个方向创建独立的动画片段
        // 精灵图布局：0-3下, 4-7右, 8-11上, 12-15左
        auto walkUp = AnimationClip::createFromGridIndices(
            texture, kFrameWidth, kFrameHeight, {0, 1, 2, 3}, frameDurationMs, kSpacing);
        
        auto walkRight = AnimationClip::createFromGridIndices(
            texture, kFrameWidth, kFrameHeight, {4, 5, 6, 7}, frameDurationMs, kSpacing);
        
        auto walkLeft = AnimationClip::createFromGridIndices(
            texture, kFrameWidth, kFrameHeight, {8, 9, 10, 11}, frameDurationMs, kSpacing);
        
        auto walkDown = AnimationClip::createFromGridIndices(
            texture, kFrameWidth, kFrameHeight, {12, 13, 14, 15}, frameDurationMs, kSpacing);
        
        // 创建 AnimatedSprite 并注册命名动画
        character_ = AnimatedSprite::create();
        character_->addAnimation("walk_up", walkUp);
        character_->addAnimation("walk_right", walkRight);
        character_->addAnimation("walk_left", walkLeft);
        character_->addAnimation("walk_down", walkDown);
        
        // 播放初始动画
        character_->play("walk_down", true);
        character_->setPosition(Vec2(450.0f, 300.0f));
        
        addChild(character_);
    }
    
    void handleInput(float dt) {
        auto& input = Application::instance().input();
        
        isMoving_ = false;
        
        if (input.isKeyDown(Key::W) || input.isKeyDown(Key::Up)) {
            moveCharacter(Direction::Up, dt);
        } else if (input.isKeyDown(Key::S) || input.isKeyDown(Key::Down)) {
            moveCharacter(Direction::Down, dt);
        } else if (input.isKeyDown(Key::A) || input.isKeyDown(Key::Left)) {
            moveCharacter(Direction::Left, dt);
        } else if (input.isKeyDown(Key::D) || input.isKeyDown(Key::Right)) {
            moveCharacter(Direction::Right, dt);
        }
        
        if (!isMoving_ && character_) {
            character_->pause();
        }
    }
    
    void moveCharacter(Direction dir, float dt) {
        // 切换动画（仅在方向改变或未播放时）
        if (currentDir_ != dir || !character_->isPlaying()) {
            std::string animName;
            switch (dir) {
                case Direction::Up:    animName = "walk_up";    break;
                case Direction::Down:  animName = "walk_down";  break;
                case Direction::Left:  animName = "walk_left";  break;
                case Direction::Right: animName = "walk_right"; break;
            }
            
            if (character_->getCurrentAnimationName() != animName) {
                E2D_INFO("切换动画: {} -> {}", character_->getCurrentAnimationName(), animName);
                character_->play(animName, true);
            } else if (!character_->isPlaying()) {
                character_->resume();
            }
        }
        
        currentDir_ = dir;
        isMoving_ = true;
        
        // 移动角色
        Vec2 pos = character_->getPosition();
        switch (dir) {
            case Direction::Down:  pos.y += moveSpeed_ * dt; break;
            case Direction::Up:    pos.y -= moveSpeed_ * dt; break;
            case Direction::Left:  pos.x -= moveSpeed_ * dt; break;
            case Direction::Right: pos.x += moveSpeed_ * dt; break;
        }
        character_->setPosition(pos);
    }
};

int main() {
    Logger::init();
    Logger::setLevel(LogLevel::Debug);
    
    auto& app = Application::instance();
    
    AppConfig config;
    config.title = "角色动画示例（动画字典方式）";
    config.width = 900;
    config.height = 600;
    config.vsync = true;
    
    if (!app.init(config)) {
        E2D_LOG_ERROR("初始化失败！");
        return -1;
    }
    
    app.resources().addSearchPath("assets");
    app.enterScene(makePtr<CharacterScene>());
    
    app.run();
    Logger::shutdown();
    return 0;
}
```

### 10.3 攻击动画与关键帧

```cpp
class AttackScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        
        // 加载攻击动画
        auto attackClip = E2D_ANIMATION_CACHE().loadClip("attack.ani");
        
        player_ = AnimatedSprite::create(attackClip);
        player_->setPosition(Vec2(400, 300));
        
        // 关键帧回调（ANI 格式中的 SetFlag）
        player_->setKeyframeCallback([this](int flagIndex) {
            switch (flagIndex) {
                case 1:  // 攻击判定开始
                    E2D_INFO("攻击判定开始");
                    showAttackEffect();
                    break;
                case 2:  // 攻击判定结束
                    E2D_INFO("攻击判定结束");
                    hideAttackEffect();
                    break;
                case 3:  // 播放音效
                    AudioEngine::play("swoosh.wav");
                    break;
            }
        });
        
        // 动画完成回调
        player_->setCompletionCallback([this]() {
            E2D_INFO("攻击结束，回到待机");
            player_->play("idle");
        });
        
        addChild(player_);
    }
    
    void onUpdate(float dt) override {
        Scene::onUpdate(dt);
        
        auto& input = Application::instance().input();
        
        // 按空格攻击
        if (input.isKeyPressed(Key::Space)) {
            player_->play("attack");
        }
    }
    
private:
    Ptr<AnimatedSprite> player_;
    
    void showAttackEffect() {
        // 显示攻击特效
    }
    
    void hideAttackEffect() {
        // 隐藏攻击特效
    }
};
```

### 10.4 动画缓存最佳实践

```cpp
class GameScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        
        // 预加载常用动画
        preloadAnimations();
        
        // 创建多个共享同一动画的角色
        createEnemies();
    }
    
    void preloadAnimations() {
        // 加载到缓存，后续直接复用
        E2D_ANIMATION_CACHE().loadClip("enemies/slime_idle.ani");
        E2D_ANIMATION_CACHE().loadClip("enemies/slime_walk.ani");
        E2D_ANIMATION_CACHE().loadClip("enemies/slime_attack.ani");
        E2D_ANIMATION_CACHE().loadClip("effects/explosion.ani");
    }
    
    void createEnemies() {
        // 10个史莱姆共享同一份动画数据
        for (int i = 0; i < 10; ++i) {
            auto enemy = AnimatedSprite::create();
            
            // 从缓存获取动画（不重复加载）
            enemy->addAnimation("idle", 
                E2D_ANIMATION_CACHE().getClip("enemies/slime_idle.ani"));
            enemy->addAnimation("walk", 
                E2D_ANIMATION_CACHE().getClip("enemies/slime_walk.ani"));
            enemy->addAnimation("attack", 
                E2D_ANIMATION_CACHE().getClip("enemies/slime_attack.ani"));
            
            enemy->setPosition(Vec2(100 + i * 80, 300));
            enemy->play("idle");
            
            addChild(enemy);
            enemies_.push_back(enemy);
        }
    }
    
    void onExit() override {
        // 场景退出时清理未使用的缓存
        E2D_ANIMATION_CACHE().removeUnusedClips();
        E2D_SPRITE_FRAME_CACHE().removeUnusedSpriteFrames();
        
        Scene::onExit();
    }
    
private:
    std::vector<Ptr<AnimatedSprite>> enemies_;
};
```

---

## 总结

Easy2D 动画系统提供了完整的动画解决方案：

1. **AnimationClip** - 纯数据动画片段，可复用
2. **AnimatedSprite** - 动画渲染节点，接入场景图
3. **AnimationController** - 播放控制逻辑
4. **SpriteFrame** - 精灵帧抽象，优化渲染
5. **缓存系统** - 全局缓存避免重复创建
6. **属性系统** - 类型安全的帧属性管理
7. **事件系统** - 关键帧回调、完成通知

通过合理使用这些组件，可以高效地实现各种动画效果。
