# Easy2D API 文档与教程

本文档是根据 Easy2D 引擎的所有示例程序生成的完整 API 使用教程。

## 文档结构

| 序号 | 文档 | 内容 |
|------|------|------|
| 01 | [Core-System.md](01-Core-System.md) | 核心系统（Application、Logger、主函数模板） |
| 02 | [Scene-System.md](02-Scene-System.md) | 场景系统（Scene、Node、Sprite、Text、Button） |
| 03 | [Input-System.md](03-Input-System.md) | 输入系统（键盘、鼠标） |
| 04 | [Resource-System.md](04-Resource-System.md) | 资源系统（Texture、Font、Sound） |
| 05 | [Rendering-System.md](05-Rendering-System.md) | 渲染系统（RenderBackend、绘制API） |
| 06 | [Texture-Pool.md](06-Texture-Pool.md) | 纹理池（LRU缓存、程序化纹理） |
| 07 | [Render-Target.md](07-Render-Target.md) | 渲染目标（离屏渲染、特效合成） |
| 08 | [Shader-System.md](08-Shader-System.md) | Shader系统（自定义Shader、预设效果） |
| 09 | [Custom-Effects.md](09-Custom-Effects.md) | 自定义特效系统（粒子特效） |
| 10 | [Complete-Tutorial.md](10-Complete-Tutorial.md) | 完整游戏开发教程（推箱子示例） |
| 11 | [Camera-System.md](11-Camera-System.md) | 相机系统（正交相机、跟随、边界限制） |
| 12 | [Anchor-System.md](12-Anchor-System.md) | 锚点系统（变换基准点、最佳实践） |
| 13 | [Button-System.md](13-Button-System.md) | 按钮系统（Button、ToggleImageButton） |

## 快速开始

### 最小示例

```cpp
#include <easy2d/easy2d.h>
#include <easy2d/utils/logger.h>

using namespace easy2d;

class MyScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        setBackgroundColor(Color(0.1f, 0.1f, 0.2f, 1.0f));
    }
    
    void onRender(RenderBackend& renderer) override {
        Scene::onRender(renderer);
        renderer.fillCircle(Vec2(400, 300), 50, Color::Red);
    }
};

int main() {
    Logger::init();
    
    auto& app = Application::instance();
    
    AppConfig config;
    config.title = "My Game";
    config.width = 800;
    config.height = 600;
    
    if (!app.init(config)) return -1;
    
    app.enterScene(makePtr<MyScene>());
    app.run();
    
    Logger::shutdown();
    return 0;
}
```

## 示例程序参考

本文档基于以下示例程序生成：

1. **hello_world** - 基础场景、碰撞检测、字体渲染
2. **advanced_graphics_demo** - 纹理池、渲染目标、Shader预设
3. **cool_effects_demo** - 自定义特效系统
4. **custom_effects_demo** - 特效配置、JSON加载
5. **font_test** - 字体系统
6. **shader_loadfromsource_demo** - 自定义Shader
7. **texture_effect_composition_demo** - 纹理池与特效合成
8. **sprite_animation_demo** - 精灵动画、渲染目标、粒子系统
9. **push_box** - 完整游戏（推箱子）

## 核心概念

### 场景生命周期
```
onEnter() -> [onUpdate() -> onRender()]循环 -> onExit()
```

### 坐标系统
- 原点 (0, 0) 在左上角
- X轴向右增加
- Y轴向下增加

### 渲染顺序
1. 场景背景
2. 子节点（按添加顺序）
3. onRender() 自定义绘制

## 常用宏

| 宏 | 功能 |
|----|------|
| `E2D_LOG_INFO(...)` | 信息日志 |
| `E2D_LOG_WARN(...)` | 警告日志 |
| `E2D_LOG_ERROR(...)` | 错误日志 |
| `E2D_INFO(...)` | 简写信息日志 |
| `E2D_TEXTURE_POOL()` | 获取纹理池 |
| `E2D_SHADER_SYSTEM()` | 获取Shader系统 |
| `E2D_CUSTOM_EFFECT_MANAGER()` | 获取特效管理器 |

## 学习路径建议

1. **新手入门**: 01 -> 02 -> 03 -> 04 -> 05 -> 12
2. **进阶开发**: 06 -> 07 -> 08 -> 09 -> 11
3. **完整项目**: 10

## 更多资源

- Easy2D 引擎源码
- 示例程序目录：`Easy2D/examples/`
