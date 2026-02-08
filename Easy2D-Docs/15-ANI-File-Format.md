# Easy2D ANI 文件格式详解

## 目录

1. [概述](#1-概述)
2. [ANI 文件基本结构](#2-ani-文件基本结构)
3. [全局属性标签](#3-全局属性标签)
4. [帧属性标签](#4-帧属性标签)
5. [完整示例](#5-完整示例)
6. [C++ API 使用](#6-c-api-使用)
7. [高级应用](#7-高级应用)
8. [调试与故障排除](#8-调试与故障排除)

---

## 1. 概述

ANI 文件格式是一种基于文本的动画脚本格式，最初用于 DNF（地下城与勇士）游戏的动画系统。Easy2D 引擎完整支持 ANI 格式，并进行了扩展和优化。

### ANI 格式的优势

- **人类可读**：纯文本格式，易于编辑和调试
- **功能丰富**：支持碰撞盒、关键帧回调、音效触发等
- **灵活扩展**：不固定数据系统允许自定义属性
- **工具生态**：有大量现成的 ANI 编辑工具

### 文件扩展名

- `.ani` - 单动画文件
- `.als` - 复合动画文件（多图层组合）

---

## 2. ANI 文件基本结构

```
[FRAME MAX]
数字                          # 总帧数（可选，用于信息展示）

[FRAME000]                    # 第0帧开始标记
[IMAGE]                       # 图片路径和索引
`图片路径` 索引号
[IMAGE POS]                   # 图片位置偏移
X坐标 Y坐标
[DELAY]                       # 帧延迟（毫秒）
数字
... 其他帧属性 ...

[FRAME001]                    # 第1帧开始标记
... 帧属性 ...

[FRAME002]
... 帧属性 ...
```

### 基本规则

1. **标签格式**：所有标签用方括号包裹，如 `[FRAME000]`
2. **大小写敏感**：标签必须大写
3. **注释**：以 `#` 开头的行是注释
4. **数值**：可以是整数或浮点数
5. **字符串**：用反引号 `` ` `` 包裹，如 `` `path/to/image.png` ``

---

## 3. 全局属性标签

全局属性在帧定义之外，影响整个动画。

### [FRAME MAX]

声明动画的总帧数（仅用于信息展示，不影响解析）。

```
[FRAME MAX]
12
```

### [LOOP]

设置动画循环播放。

```
[LOOP]
```

> 注意：在 Easy2D 中，也可以通过代码 `clip->setLooping(true)` 设置。

### [SHADOW]

启用阴影效果（全局）。

```
[SHADOW]
```

---

## 4. 帧属性标签

帧属性在 `[FRAMEXXX]` 标签之后定义，只影响当前帧。

### [IMAGE] - 图片资源

指定当前帧显示的图片路径和索引。

```
[IMAGE]
`character/walk.png` 0
```

**格式说明**：
- 第一行：图片路径（用反引号包裹）
- 第二行：精灵图集索引（可选，默认为0）

**跨行写法**：
```
[IMAGE]
`character/walk.png`
0
```

**精灵图集索引**：
当图片是精灵图（包含多帧）时，索引指定使用第几帧。

```
# 使用 walk.png 的第5帧
[IMAGE]
`character/walk.png` 5
```

### [IMAGE POS] - 图片位置偏移

设置图片相对于节点原点的偏移。

```
[IMAGE POS]
-48 -96
```

**坐标系**：
- X轴：正值向右，负值向左
- Y轴：正值向下，负值向上
- 单位：像素

### [DELAY] - 帧延迟

设置当前帧显示的持续时间。

```
[DELAY]
100
```

**单位**：毫秒

**常用值**：
- 60 FPS 游戏：约 16-17ms
- 30 FPS 游戏：约 33ms
- 普通动画：100-200ms

### [SET FLAG] - 关键帧标记

设置关键帧回调索引，用于触发游戏逻辑。

```
[SET FLAG]
1
```

**应用场景**：
- 攻击判定开始/结束
- 音效播放时机
- 特效触发点

**代码处理**：
```cpp
sprite->setKeyframeCallback([](int flagIndex) {
    switch (flagIndex) {
        case 1: /* 攻击判定开始 */ break;
        case 2: /* 攻击判定结束 */ break;
    }
});
```

### [PLAY SOUND] - 播放音效

指定当前帧播放的音效文件。

```
[PLAY SOUND]
`sounds/attack.wav`
```

**代码处理**：
```cpp
sprite->setSoundTriggerCallback([](const std::string& path) {
    AudioEngine::play(path);
});
```

### [IMAGE RATE] - 图片缩放

设置当前帧图片的缩放比例。

```
[IMAGE RATE]
1.5 1.5
```

**格式**：`X缩放 Y缩放`

**说明**：
- 1.0 表示原始大小
- 大于1.0 放大
- 小于1.0 缩小
- 负值 翻转

### [IMAGE ROTATE] - 图片旋转

设置当前帧图片的旋转角度。

```
[IMAGE ROTATE]
45.0
```

**单位**：度（°）

**方向**：正值为顺时针旋转

### [RGBA] - 颜色叠加

设置当前帧的颜色叠加效果。

```
[RGBA]
255 0 0 128
```

**格式**：`R G B A`

**范围**：0-255

**应用**：
- 受伤闪烁：红色半透明
- 隐身效果：降低Alpha值
- 属性buff：蓝色/黄色等

### [INTERPOLATION] - 插值

启用帧间插值，使动画更平滑。

```
[INTERPOLATION]
```

**说明**：
- 当前帧到下一帧之间会进行插值计算
- 适用于位置、缩放、旋转、颜色的平滑过渡

### [DAMAGE TYPE] - 伤害类型

设置当前帧的伤害类型（用于格斗游戏）。

```
[DAMAGE TYPE]
`SUPERARMOR`
```

**可选值**：
- `NORMAL` - 普通（可被击飞）
- `SUPERARMOR` - 霸体（不会被击飞）
- `UNBREAKABLE` - 无敌

### [DAMAGE BOX] - 伤害碰撞盒

定义角色受击的碰撞区域。

```
[DAMAGE BOX]
-20 -60 40 80 0 0
```

**格式**：`X Y 宽度 高度 未知 未知`

**说明**：
- 通常每帧只有一个伤害盒
- 覆盖角色身体范围

### [ATTACK BOX] - 攻击碰撞盒

定义角色攻击的命中区域。

```
[ATTACK BOX]
20 -40 60 50 0 0
```

**格式**：`X Y 宽度 高度 未知 未知`

**说明**：
- 一帧可以有多个攻击盒
- 通常只在攻击动作中出现

### [FLIP TYPE] - 翻转类型

设置图片翻转方式。

```
[FLIP TYPE]
`HORIZON`
```

**可选值**：
- `HORIZON` - 水平翻转
- `VERTICAL` - 垂直翻转
- `ALL` - 水平和垂直都翻转

### [GRAPHIC EFFECT] - 图形特效

设置渲染特效。

```
[GRAPHIC EFFECT]
`LINEARDODGE`
```

**可选值**：
- `NORMAL` - 正常
- `DODGE` - 减淡
- `LINEARDODGE` - 线性减淡（加法混合）
- `DARK` - 变暗
- `XOR` - 异或
- `MONOCHROME` - 单色
- `SPACEDISTORT` - 空间扭曲

### [CLIP] - 裁剪区域

设置图片的裁剪区域。

```
[CLIP]
0 0 100 100
```

**格式**：`X Y 宽度 高度`

### [LOOP START] / [LOOP END] - 循环标记

定义动画的循环区间。

```
[FRAME005]
...
[LOOP START]
...

[FRAME010]
...
[LOOP END]
10
```

**说明**：
- `[LOOP START]` 标记循环开始
- `[LOOP END]` 后跟循环结束帧号
- 播放完结束帧后跳回开始帧

---

## 5. 完整示例

### 5.1 简单的待机动画

```ani
# 角色待机动画
# 2帧循环播放

[LOOP]

[FRAME MAX]
2

[FRAME000]
[IMAGE]
`character/idle.png` 0
[IMAGE POS]
-48 -96
[DELAY]
200

[FRAME001]
[IMAGE]
`character/idle.png` 1
[IMAGE POS]
-48 -96
[DELAY]
200
```

### 5.2 带攻击判定的攻击动画

```ani
# 角色攻击动画
# 包含攻击判定和关键帧回调

[FRAME MAX]
8

# 起手
[FRAME000]
[IMAGE]
`character/attack.png` 0
[IMAGE POS]
-48 -96
[DELAY]
50
[SET FLAG]
1

# 攻击判定开始
[FRAME001]
[IMAGE]
`character/attack.png` 1
[IMAGE POS]
-48 -96
[DELAY]
50
[ATTACK BOX]
20 -40 80 60 0 0
[PLAY SOUND]
`sounds/slash.wav`

[FRAME002]
[IMAGE]
`character/attack.png` 2
[IMAGE POS]
-48 -96
[DELAY]
50
[ATTACK BOX]
40 -30 100 50 0 0

# 攻击判定结束
[FRAME003]
[IMAGE]
`character/attack.png` 3
[IMAGE POS]
-48 -96
[DELAY]
50
[SET FLAG]
2

# 收手
[FRAME004]
[IMAGE]
`character/attack.png` 4
[IMAGE POS]
-48 -96
[DELAY]
80

[FRAME005]
[IMAGE]
`character/attack.png` 5
[IMAGE POS]
-48 -96
[DELAY]
80

[FRAME006]
[IMAGE]
`character/attack.png` 6
[IMAGE POS]
-48 -96
[DELAY]
80

[FRAME007]
[IMAGE]
`character/attack.png` 7
[IMAGE POS]
-48 -96
[DELAY]
100
```

### 5.3 带受伤闪烁的受击动画

```ani
# 角色受击动画
# 带红色闪烁效果

[FRAME MAX]
6

[FRAME000]
[IMAGE]
`character/hit.png` 0
[IMAGE POS]
-48 -96
[DELAY]
50
[RGBA]
255 255 255 255

[FRAME001]
[IMAGE]
`character/hit.png` 1
[IMAGE POS]
-48 -96
[DELAY]
50
[RGBA]
255 100 100 200

[FRAME002]
[IMAGE]
`character/hit.png` 2
[IMAGE POS]
-48 -96
[DELAY]
100
[RGBA]
255 50 50 180
[PLAY SOUND]
`sounds/hit.wav`

[FRAME003]
[IMAGE]
`character/hit.png` 3
[IMAGE POS]
-48 -96
[DELAY]
100
[RGBA]
255 100 100 200

[FRAME004]
[IMAGE]
`character/hit.png` 4
[IMAGE POS]
-48 -96
[DELAY]
80
[RGBA]
255 200 200 220

[FRAME005]
[IMAGE]
`character/hit.png` 5
[IMAGE POS]
-48 -96
[DELAY]
80
[RGBA]
255 255 255 255
```

### 5.4 带缩放效果的技能动画

```ani
# 技能释放动画
# 带缩放和旋转效果

[FRAME MAX]
10

# 蓄力
[FRAME000]
[IMAGE]
`effects/skill.png` 0
[IMAGE POS]
-64 -128
[DELAY]
100
[IMAGE RATE]
0.8 0.8
[RGBA]
200 200 255 200

[FRAME001]
[IMAGE]
`effects/skill.png` 1
[IMAGE POS]
-64 -128
[DELAY]
100
[IMAGE RATE]
0.9 0.9
[RGBA]
220 220 255 220
[INTERPOLATION]

# 释放
[FRAME002]
[IMAGE]
`effects/skill.png` 2
[IMAGE POS]
-64 -128
[DELAY]
80
[IMAGE RATE]
1.2 1.2
[RGBA]
255 255 255 255
[GRAPHIC EFFECT]
`LINEARDODGE`
[PLAY SOUND]
`sounds/skill.wav`
[SET FLAG]
1

[FRAME003]
[IMAGE]
`effects/skill.png` 3
[IMAGE POS]
-64 -128
[DELAY]
80
[IMAGE RATE]
1.5 1.5
[IMAGE ROTATE]
15

[FRAME004]
[IMAGE]
`effects/skill.png` 4
[IMAGE POS]
-64 -128
[DELAY]
80
[IMAGE RATE]
1.8 1.8
[IMAGE ROTATE]
30

# 消散
[FRAME005]
[IMAGE]
`effects/skill.png` 5
[IMAGE POS]
-64 -128
[DELAY]
60
[IMAGE RATE]
2.0 2.0
[RGBA]
255 255 255 200

[FRAME006]
[IMAGE]
`effects/skill.png` 6
[IMAGE POS]
-64 -128
[DELAY]
60
[IMAGE RATE]
2.2 2.2
[RGBA]
255 255 255 150

[FRAME007]
[IMAGE]
`effects/skill.png` 7
[IMAGE POS]
-64 -128
[DELAY]
60
[IMAGE RATE]
2.5 2.5
[RGBA]
255 255 255 100

[FRAME008]
[IMAGE]
`effects/skill.png` 8
[IMAGE POS]
-64 -128
[DELAY]
60
[IMAGE RATE]
2.8 2.8
[RGBA]
255 255 255 50
[SET FLAG]
2

[FRAME009]
[IMAGE]
`effects/skill.png` 9
[IMAGE POS]
-64 -128
[DELAY]
60
[IMAGE RATE]
3.0 3.0
[RGBA]
255 255 255 0
```

---

## 6. C++ API 使用

### 6.1 基础加载

```cpp
#include <easy2d/easy2d.h>

using namespace easy2d;

class GameScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        
        // 方式1: 通过 AnimationCache 加载（推荐）
        auto clip = E2D_ANIMATION_CACHE().loadClip("animations/player_idle.ani");
        
        // 方式2: 直接使用 AniParser
        AniParser parser;
        auto result = parser.parse("animations/player_idle.ani");
        if (result.success) {
            auto clip = result.clip;
        }
        
        // 创建动画精灵
        player_ = AnimatedSprite::create(clip);
        player_->setPosition(Vec2(400, 300));
        addChild(player_);
        
        // 播放
        player_->play();
    }
    
private:
    Ptr<AnimatedSprite> player_;
};
```

### 6.2 路径解析器

当 ANI 文件中的路径需要转换时，使用路径解析器：

```cpp
// 设置全局路径解析器
E2D_ANIMATION_CACHE().setPathResolver([](const std::string& path) {
    // 将相对路径转换为绝对路径
    if (path.find("sprite/") == 0) {
        return "assets/images/" + path.substr(7);
    }
    return "assets/" + path;
});

// 或者针对单个解析器设置
AniParser parser;
parser.setPathResolver([](const std::string& path) {
    return "resources/" + path;
});
parser.setBasePath("game/assets");

auto result = parser.parse("player.ani");
```

### 6.3 关键帧回调处理

```cpp
void setupPlayerAnimation() {
    auto clip = E2D_ANIMATION_CACHE().loadClip("animations/player_attack.ani");
    player_ = AnimatedSprite::create(clip);
    
    // 关键帧回调
    player_->setKeyframeCallback([this](int flagIndex) {
        switch (flagIndex) {
            case 1:  // 攻击判定开始
                enableAttackHitbox();
                spawnAttackEffect();
                break;
            case 2:  // 攻击判定结束
                disableAttackHitbox();
                break;
            case 3:  // 可以连击
                canCombo_ = true;
                break;
        }
    });
    
    // 音效回调
    player_->setSoundTriggerCallback([](const std::string& path) {
        AudioEngine::play(path);
    });
    
    // 动画完成回调
    player_->setCompletionCallback([this]() {
        if (currentState_ == State::Attacking) {
            currentState_ = State::Idle;
            player_->play("idle");
        }
    });
}
```

### 6.4 碰撞盒访问

```cpp
void onUpdate(float dt) override {
    Scene::onUpdate(dt);
    
    // 获取当前帧的碰撞盒
    const auto& damageBoxes = player_->getCurrentDamageBoxes();
    const auto& attackBoxes = player_->getCurrentAttackBoxes();
    
    // 检测攻击命中
    for (const auto& attackBox : attackBoxes) {
        Rect attackRect(attackBox[0], attackBox[1], attackBox[2], attackBox[3]);
        
        for (auto& enemy : enemies_) {
            const auto& enemyDamageBoxes = enemy->getCurrentDamageBoxes();
            for (const auto& damageBox : enemyDamageBoxes) {
                Rect damageRect(damageBox[0], damageBox[1], damageBox[2], damageBox[3]);
                
                if (attackRect.intersects(damageRect)) {
                    enemy->takeDamage(10);
                }
            }
        }
    }
}
```

### 6.5 动画字典管理

```cpp
void onEnter() override {
    Scene::onEnter();
    
    player_ = AnimatedSprite::create();
    
    // 加载多个动画到字典
    player_->addAnimation("idle", 
        E2D_ANIMATION_CACHE().loadClip("animations/idle.ani"));
    player_->addAnimation("walk", 
        E2D_ANIMATION_CACHE().loadClip("animations/walk.ani"));
    player_->addAnimation("attack", 
        E2D_ANIMATION_CACHE().loadClip("animations/attack.ani"));
    player_->addAnimation("hit", 
        E2D_ANIMATION_CACHE().loadClip("animations/hit.ani"));
    player_->addAnimation("die", 
        E2D_ANIMATION_CACHE().loadClip("animations/die.ani"));
    
    player_->setPosition(Vec2(400, 300));
    addChild(player_);
    
    // 播放待机动画
    player_->play("idle", true);
}

void changeState(State newState) {
    if (currentState_ == newState) return;
    
    currentState_ = newState;
    
    switch (newState) {
        case State::Idle:
            player_->play("idle", true);
            break;
        case State::Walking:
            player_->play("walk", true);
            break;
        case State::Attacking:
            player_->play("attack", false);  // 不循环
            break;
        case State::Hit:
            player_->play("hit", false);
            break;
        case State::Dead:
            player_->play("die", false);
            break;
    }
}
```

---

## 7. 高级应用

### 7.1 ALS 复合动画

ALS 文件用于组合多个 ANI 动画为图层：

```als
# player.als - 角色复合动画

[LAYER]
`body.ani`
0
0 0

[LAYER]
`weapon.ani`
1
10 -5

[LAYER]
`effect.ani`
2
0 0
```

**格式说明**：
- `[LAYER]` 标记新图层开始
- 第一行：ANI 文件路径
- 第二行：Z轴顺序（越大越在上层）
- 第三行：X Y 偏移

**C++ 使用**：
```cpp
AlsParser parser;
auto result = parser.parse("animations/player.als");

if (result.success) {
    for (const auto& layer : result.layers) {
        auto clip = E2D_ANIMATION_CACHE().loadClip(layer.aniPath);
        auto sprite = AnimatedSprite::create(clip);
        sprite->setPosition(layer.offset);
        sprite->setZOrder(layer.zOrder);
        addChild(sprite);
    }
}
```

### 7.2 运行时修改动画

```cpp
// 获取动画片段并修改
auto clip = E2D_ANIMATION_CACHE().getClip("player_attack.ani");

// 修改特定帧的延迟
auto& frame = clip->getFrame(2);
frame.delay = 200.0f;  // 变慢

// 添加新的碰撞盒
std::array<int32_t, 6> newBox = {0, -50, 100, 100, 0, 0};
frame.attackBoxes.push_back(newBox);

// 修改属性
frame.properties.withColorTint(Color::Red);
```

### 7.3 动态创建 ANI

```cpp
// 程序化创建 AnimationClip（等效于解析 ANI）
auto clip = AnimationClip::create("dynamic_animation");

for (int i = 0; i < 8; ++i) {
    AnimationFrame frame;
    
    // 设置精灵帧
    frame.spriteFrame = E2D_SPRITE_FRAME_CACHE().getSpriteFrame(
        "effect.png#" + std::to_string(i));
    
    // 设置延迟
    frame.delay = 50.0f;
    
    // 设置属性
    frame.properties
        .withImageRate(Vec2(1.0f + i * 0.1f, 1.0f + i * 0.1f))
        .withColorTint(Color(255, 255, 255, 255 - i * 30));
    
    // 关键帧
    if (i == 4) {
        frame.properties.withSetFlag(1);
    }
    
    clip->addFrame(frame);
}

clip->setLooping(false);
```

---

## 8. 调试与故障排除

### 8.1 常见问题

#### 图片不显示

```cpp
// 检查纹理是否加载成功
auto texture = E2D_TEXTURE_POOL().get("path/to/image.png");
if (!texture) {
    E2D_ERROR("纹理加载失败！");
}

// 检查 SpriteFrame 是否有效
auto& frame = player_->getController().getCurrentFrame();
if (!frame.hasTexture()) {
    E2D_ERROR("当前帧没有纹理！路径: {}", frame.texturePath);
}
```

#### 动画不播放

```cpp
// 检查动画状态
E2D_INFO("播放状态: {}", player_->isPlaying() ? "播放中" : "停止");
E2D_INFO("当前帧: {}/{}", player_->getCurrentFrameIndex(), player_->getTotalFrames());
E2D_INFO("是否循环: {}", player_->isLooping());
```

#### 关键帧不触发

```cpp
// 检查帧属性
auto& frame = player_->getController().getCurrentFrame();
if (frame.hasKeyframeCallback()) {
    E2D_INFO("关键帧索引: {}", frame.getKeyframeIndex());
}
```

### 8.2 调试技巧

```cpp
// 启用详细日志
Logger::setLevel(LogLevel::Debug);

// 逐帧调试
void onUpdate(float dt) override {
    static float accum = 0;
    accum += dt;
    
    // 每0.5秒前进一帧
    if (accum > 0.5f) {
        accum = 0;
        player_->nextFrame();
        
        auto& frame = player_->getController().getCurrentFrame();
        E2D_INFO("帧 {}: 延迟={}ms, 偏移=({}, {})",
                 player_->getCurrentFrameIndex(),
                 frame.delay,
                 frame.offset.x, frame.offset.y);
    }
}
```

### 8.3 ANI 文件验证清单

- [ ] 文件编码为 UTF-8 或 ASCII
- [ ] 所有标签使用大写
- [ ] 字符串用反引号 `` ` `` 包裹
- [ ] 图片路径正确（相对或绝对）
- [ ] 帧索引从 000 开始连续
- [ ] 数值没有多余字符
- [ ] 没有遗漏 `[IMAGE]` 标签

---

## 附录：标签速查表

| 标签 | 作用域 | 说明 |
|------|--------|------|
| `[FRAME MAX]` | 全局 | 声明总帧数 |
| `[LOOP]` | 全局 | 循环播放 |
| `[SHADOW]` | 全局 | 阴影效果 |
| `[FRAMEXXX]` | - | 帧开始标记 |
| `[IMAGE]` | 帧 | 图片路径和索引 |
| `[IMAGE POS]` | 帧 | 位置偏移 |
| `[DELAY]` | 帧 | 帧延迟 |
| `[SET FLAG]` | 帧 | 关键帧标记 |
| `[PLAY SOUND]` | 帧 | 播放音效 |
| `[IMAGE RATE]` | 帧 | 缩放比例 |
| `[IMAGE ROTATE]` | 帧 | 旋转角度 |
| `[RGBA]` | 帧 | 颜色叠加 |
| `[INTERPOLATION]` | 帧 | 启用插值 |
| `[DAMAGE TYPE]` | 帧 | 伤害类型 |
| `[DAMAGE BOX]` | 帧 | 受击碰撞盒 |
| `[ATTACK BOX]` | 帧 | 攻击碰撞盒 |
| `[FLIP TYPE]` | 帧 | 翻转类型 |
| `[GRAPHIC EFFECT]` | 帧 | 图形特效 |
| `[CLIP]` | 帧 | 裁剪区域 |
| `[LOOP START]` | 帧 | 循环开始标记 |
| `[LOOP END]` | 帧 | 循环结束标记 |

---

通过本文档，您应该能够熟练创建和使用 ANI 动画文件。如需更多帮助，请参考 Easy2D 示例程序中的动画相关代码。
