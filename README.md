<div align="center">

![Easy2D](./logo/logo_text_dark.png)

[![Build status](https://ci.appveyor.com/api/projects/status/ah39otggqy7xu0qm/branch/master?svg=true)](https://ci.appveyor.com/project/Nomango/easy2d/branch/master)
[![GitHub release](https://img.shields.io/github/release/easy2d/easy2d)](https://github.com/Easy2D/Easy2D/releases/latest)
[![GitHub license](https://img.shields.io/github/license/easy2d/easy2d)](https://github.com/Easy2D/Easy2D/blob/master/LICENSE)

</div>

---

## 📖 简介

> 🎮 **Easy2D** 是一个为 C++ 设计的轻量级 2D 游戏引擎，专注于简化游戏开发流程，让开发者能够快速构建 Windows 平台的 2D 游戏。

### 🎯 设计目标

```mermaid
mindmap
  root((Easy2D<br/>设计目标))
    简单易用
      简洁的API设计
      快速上手
      低学习成本
    功能完整
      场景管理
      动画系统
      音频支持
      GUI组件
    性能优先
      高效的渲染
      内存优化
      流畅的体验
    学习友好
      清晰的代码结构
      丰富的示例
      详细的文档
```

---

## 🏗️ 系统架构

### 核心架构图

```mermaid
flowchart TB
    subgraph 应用层["🎮 应用层"]
        A[游戏项目] --> B[Game 类]
        B --> C[Scene 场景管理]
    end

    subgraph 核心层["⚙️ 核心层"]
        C --> D[Node 节点系统]
        D --> E[Sprite 精灵]
        D --> F[Animation 动画]
        D --> G[GUI 组件]
    end

    subgraph 渲染层["🎨 渲染层"]
        E --> H[Direct2D 渲染]
        F --> H
        G --> H
        H --> I[GPU 加速]
    end

    subgraph 资源层["📦 资源层"]
        J[图片资源] --> K[ResourceManager]
        L[音频资源] --> K
        M[字体资源] --> K
        K --> D
    end

    subgraph 系统层["🔧 系统层"]
        N[窗口管理]
        O[输入处理]
        P[音频播放]
        Q[文件IO]
    end

    H --> N
    D --> O
    P --> L

    style 应用层 fill:#e1f5fe
    style 核心层 fill:#fff3e0
    style 渲染层 fill:#f3e5f5
    style 资源层 fill:#e8f5e9
    style 系统层 fill:#fce4ec
```

### 场景生命周期

```mermaid
stateDiagram-v2
    [*] --> 初始化: 创建场景
    初始化 --> 运行中: onEnter
    
    state 运行中 {
        [*] --> 更新
        更新 --> 渲染: 每帧调用
        渲染 --> 更新
        更新 --> 事件处理: 用户输入
        事件处理 --> 更新
    }
    
    运行中 --> 暂停: 切换场景
    暂停 --> 运行中: 恢复场景
    运行中 --> 销毁: onExit
    销毁 --> [*]
```

---

## ✨ 功能特性

### 功能全景图

```mermaid
mindmap
  root((Easy2D<br/>功能特性))
    场景系统
      场景管理
      场景切换
      过渡动画
      层级管理
    渲染系统
      精灵渲染
      文字渲染
      图集支持
      混合模式
    动画系统
      属性动画
      序列动画
      并行动画
      循环动画
    音频系统
      WAV播放
      背景音乐
      音效控制
    GUI系统
      按钮
      标签
      事件响应
    工具类
      数据持久化
      定时器
      随机数
      数学工具
```

### 核心模块详解

| 模块 | 功能描述 | 关键类 |
|:---:|:---|:---|
| 🎬 **场景管理** | 多场景切换、过渡动画、层级控制 | `Scene`, `Director`, `Transition` |
| 🎨 **渲染系统** | 2D图形渲染、文字显示、纹理管理 | `Sprite`, `Label`, `Texture` |
| 🎭 **动画系统** | 属性动画、组合动画、缓动函数 | `Action`, `Animate`, `Ease` |
| 🔊 **音频系统** | WAV音频播放、音量控制 | `Audio`, `Music`, `SoundEffect` |
| 🖱️ **GUI系统** | 按钮、菜单、事件响应 | `Button`, `Menu`, `EventListener` |
| 💾 **数据存储** | 本地数据持久化、配置文件 | `UserDefault`, `FileUtils` |

---

## 🚀 快速开始

### 环境要求

```mermaid
flowchart LR
    A[系统要求] --> B[Windows 7+]
    A --> C[Visual Studio 2013+]
    A --> D[DirectX 11]
    
    B --> E[支持平台]
    C --> F[开发工具]
    D --> G[图形API]
    
    style A fill:#e3f2fd
    style B fill:#e8f5e9
    style C fill:#e8f5e9
    style D fill:#e8f5e9
```

### 安装流程

```mermaid
flowchart TD
    A[开始安装] --> B{选择安装方式}
    
    B -->|简易安装| C[下载安装包]
    B -->|源码编译| D[克隆仓库]
    
    C --> E[运行安装程序]
    E --> F[选择VS版本]
    F --> G[自动配置环境]
    
    D --> H[git clone]
    H --> I[打开sln文件]
    I --> J[编译生成lib]
    J --> K[配置项目属性]
    
    G --> L[安装完成]
    K --> L
    
    L --> M[创建新项目]
    M --> N[开始游戏开发]
    
    style A fill:#c8e6c9
    style L fill:#c8e6c9
    style N fill:#bbdefb
```

### 第一步：获取源码

```bash
# 克隆仓库
git clone https://github.com/nomango/easy2d.git

# 或者下载 ZIP 压缩包
# 访问 https://github.com/Easy2D/Easy2D/releases
```

### 第二步：编译引擎

```mermaid
sequenceDiagram
    participant 开发者
    participant VS as Visual Studio
    participant 编译器
    participant 输出

    开发者->>VS: 打开 Easy2D.sln
    VS->>VS: 选择配置(Debug/Release)
    VS->>VS: 选择平台(x86/x64)
    开发者->>VS: 点击生成解决方案
    VS->>编译器: 调用MSVC编译
    编译器->>编译器: 编译源文件
    编译器->>输出: 生成Easy2D.lib
    输出->>VS: 输出到/Easy2D/output/
    VS->>开发者: 编译完成
```

### 第三步：配置项目

```mermaid
flowchart LR
    subgraph 配置步骤
        A[项目属性] --> B[附加包含目录]
        A --> C[附加库目录]
        A --> D[附加依赖项]
    end
    
    B --> B1[Easy2D/include/]
    C --> C1[Easy2D/output/]
    D --> D1[Easy2D.lib]
    
    style A fill:#fff3e0
    style B fill:#e8f5e9
    style C fill:#e8f5e9
    style D fill:#e8f5e9
```

**详细配置：**

| 配置项 | 路径/值 |
|:---|:---|
| C/C++ → 附加包含目录 | `$(SolutionDir)../Easy2D/include/` |
| 链接器 → 附加库目录 | `$(SolutionDir)../Easy2D/output/` |
| 链接器 → 附加依赖项 | `Easy2D.lib` |

### 第四步：Hello World

```cpp
#include <easy2d/easy2d.h>

using namespace easy2d;

int main()
{
    // 初始化引擎
    if (Game::init())
    {
        // 创建场景
        auto scene = new Scene;
        
        // 创建精灵
        auto sprite = new Sprite("player.png");
        sprite->setPos(400, 300);
        scene->addChild(sprite);
        
        // 运行动画
        auto move = gcnew MoveBy(2, 200, 0);
        auto rotate = gcnew RotateBy(2, 360);
        sprite->runAction(gcnew Sequence({ move, rotate }));
        
        // 运行场景
        Director::getInstance()->runScene(scene);
        
        // 启动游戏循环
        Game::start();
    }
    
    Game::destroy();
    return 0;
}
```

---

## 📁 项目结构

```mermaid
flowchart TB
    subgraph Easy2D项目结构
        Root[Easy2D/] --> Src[src/]
        Root --> Inc[include/]
        Root --> Out[output/]
        Root --> Samples[samples/]
        Root --> Docs[docs/]
        Root --> Logo[logo/]
    end
    
    Src --> SrcCore[core/ 核心模块]
    Src --> SrcRender[renderer/ 渲染模块]
    Src --> SrcAudio[audio/ 音频模块]
    Src --> SrcGUI[gui/ GUI模块]
    Src --> SrcUtils[utils/ 工具模块]
    
    Inc --> IncEasy2d[easy2d/ 头文件]
    
    Samples --> SampleHello[HelloWorld/]
    Samples --> SampleDemo[Demo/]
    
    style Root fill:#e3f2fd
    style Src fill:#e8f5e9
    style Inc fill:#fff3e0
    style Out fill:#fce4ec
    style Samples fill:#f3e5f5
```

### 目录说明

```
Easy2D/
├── src/                    # 源代码
│   ├── core/              # 核心功能（场景、节点、导演）
│   ├── renderer/          # 渲染系统（精灵、纹理、文字）
│   ├── animation/         # 动画系统
│   ├── audio/             # 音频系统
│   ├── gui/               # GUI组件
│   └── utils/             # 工具类
├── include/               # 头文件
│   └── easy2d/
│       └── easy2d.h       # 主头文件
├── output/                # 编译输出（.lib文件）
├── samples/               # 示例项目
│   ├── HelloWorld/        # Hello World示例
│   └── Demo/              # 综合演示
├── docs/                  # 文档
└── logo/                  # Logo资源
```

---

## 🎮 核心概念

### 节点层级结构

```mermaid
flowchart TB
    subgraph 场景图["场景图 (Scene Graph)"]
        Scene[Scene<br/>场景] --> Node1[Node<br/>节点]
        Scene --> Node2[Node<br/>节点]
        Scene --> Node3[Node<br/>节点]
        
        Node1 --> Sprite1[Sprite<br/>精灵]
        Node1 --> Label1[Label<br/>文字]
        
        Node2 --> Sprite2[Sprite<br/>精灵]
        Sprite2 --> Particle[Particle<br/>粒子]
        
        Node3 --> Button1[Button<br/>按钮]
        Node3 --> Menu1[Menu<br/>菜单]
    end
    
    style Scene fill:#ffccbc
    style Node1 fill:#c8e6c9
    style Node2 fill:#c8e6c9
    style Node3 fill:#c8e6c9
```

### 游戏循环

```mermaid
flowchart LR
    A[初始化] --> B[游戏循环]
    B --> C[处理输入]
    C --> D[更新逻辑]
    D --> E[渲染画面]
    E --> F{是否退出?}
    F -->|否| B
    F -->|是| G[清理资源]
    G --> H[结束]
    
    style B fill:#bbdefb
    style F fill:#ffccbc
```

### 动画系统

```mermaid
flowchart TB
    subgraph 动画组合["动画组合方式"]
        Action[Action<br/>基础动作] --> Sequence[Sequence<br/>序列动画]
        Action --> Spawn[Spawn<br/>并行动画]
        Action --> Repeat[Repeat<br/>重复动画]
        
        Sequence --> Ease[Ease<br/>缓动效果]
        Spawn --> Ease
        
        Ease --> Callback[CallFunc<br/>回调函数]
    end
    
    subgraph 常用动作["常用动作类型"]
        Move[MoveBy/MoveTo<br/>移动]
        Rotate[RotateBy/RotateTo<br/>旋转]
        Scale[ScaleBy/ScaleTo<br/>缩放]
        Fade[FadeIn/FadeOut<br/>淡入淡出]
        Delay[DelayTime<br/>延迟]
    end
    
    style Action fill:#e3f2fd
    style Sequence fill:#c8e6c9
    style Spawn fill:#c8e6c9
    style Repeat fill:#c8e6c9
```

---

## 📚 示例代码

### 场景切换示例

```cpp
// 创建第一个场景
auto scene1 = new Scene;
auto label1 = new Label("场景 1");
label1->setPos(400, 300);
scene1->addChild(label1);

// 创建第二个场景
auto scene2 = new Scene;
auto label2 = new Label("场景 2");
label2->setPos(400, 300);
scene2->addChild(label2);

// 3秒后切换到场景2，使用淡入淡出效果
auto delay = gcnew DelayTime(3);
auto transition = gcnew TransitionFade(1, scene2);
auto sequence = gcnew Sequence({ delay, transition });
scene1->runAction(sequence);
```

### 动画组合示例

```cpp
// 创建一个复杂的动画序列
auto sprite = new Sprite("hero.png");

// 移动动画
auto move = gcnew MoveBy(1, 100, 0);
// 旋转动画
auto rotate = gcnew RotateBy(1, 180);
// 缩放动画
auto scale = gcnew ScaleTo(0.5, 1.5, 1.5);

// 并行动画（同时执行）
auto spawn = gcnew Spawn({ move, rotate });

// 序列动画（按顺序执行）
auto sequence = gcnew Sequence({ spawn, scale });

// 添加缓动效果
auto ease = gcnew EaseInOut(sequence);

// 无限循环
auto repeat = gcnew RepeatForever(ease);

sprite->runAction(repeat);
```

---

## 🔧 进阶主题

### 自定义节点

```mermaid
flowchart TB
    subgraph 继承体系["节点继承体系"]
        Node[Node<br/>基类] --> Sprite[Sprite<br/>精灵]
        Node --> Label[Label<br/>文字]
        Node --> Layer[Layer<br/>层]
        Node --> Custom["MyCustomNode<br/>自定义节点"]
        
        Sprite --> Button[Button<br/>按钮]
        Layer --> ColorLayer[ColorLayer<br/>颜色层]
    end
    
    style Node fill:#ffccbc
    style Custom fill:#c8e6c9
```

```cpp
class MyCustomNode : public Node
{
public:
    virtual void onUpdate() override
    {
        // 每帧更新逻辑
    }
    
    virtual void onRender() override
    {
        // 自定义渲染
    }
};
```

### 事件处理流程

```mermaid
sequenceDiagram
    participant User as 用户
    participant Window as 游戏窗口
    participant Dispatcher as 事件分发器
    participant Listener as 事件监听器
    participant Node as 游戏节点

    User->>Window: 点击鼠标
    Window->>Dispatcher: 发送鼠标事件
    Dispatcher->>Dispatcher: 事件排序
    Dispatcher->>Listener: 分发事件
    Listener->>Node: 调用回调函数
    Node->>Node: 执行响应逻辑
    Node-->>Listener: 返回处理结果
    Listener-->>Dispatcher: 是否继续传递
    Dispatcher-->>Window: 事件处理完成
```

---

## 📖 学习资源

### 官方资源

```mermaid
mindmap
  root((学习资源))
    官方文档
      API参考
      教程指南
      示例代码
    社区支持
      QQ群 608406540
      GitHub Issues
      官方论坛
    视频教程
      入门教程
      实战案例
      进阶技巧
```

| 资源类型 | 链接 | 说明 |
|:---|:---|:---|
| 🌐 官方网站 | [easy2d.cn](https://easy2d.cn) | 完整文档和教程 |
| 📦 GitHub | [github.com/Easy2D/Easy2D](https://github.com/Easy2D/Easy2D) | 源码和发布版本 |
| 💬 QQ群 | 608406540 | 技术交流和答疑 |
| 📝 示例项目 | `/samples/` 目录 | 官方示例代码 |

---

## 🗺️ 路线图

```mermaid
timeline
    title Easy2D 发展历程
    section 过去
        初始版本 : 基础渲染系统
                 : 场景管理
                 : 简单动画
    section 当前
        稳定版本 : 完整动画系统
                 : GUI组件
                 : 音频支持
    section 未来
        Kiwano引擎 : 全新架构
                   : 跨平台支持
                   : 更强大的功能
```

> ⚠️ **重要提示**：Easy2D 是作者个人的早期作品，目前处于维护状态。新的游戏引擎项目 [Kiwano](https://github.com/nomango/kiwano) 已经更加庞大且专业，建议关注新项目的发展。

---

## 🤝 贡献指南

```mermaid
flowchart LR
    A[Fork项目] --> B[创建分支]
    B --> C[提交更改]
    C --> D[Push到Fork]
    D --> E[创建PR]
    E --> F[代码审查]
    F --> G[合并到主分支]
    
    style A fill:#e8f5e9
    style G fill:#c8e6c9
```

我们欢迎各种形式的贡献：
- 🐛 提交 Bug 报告
- 💡 提出新功能建议
- 📝 改进文档
- 🔧 提交代码修复
- 🎨 设计新示例

---

## 📄 许可证

```
MIT License

Copyright (c) 2018-2024 Easy2D Contributors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

---

<div align="center">

### 🌟 如果这个项目对你有帮助，请给个 Star！

[![GitHub stars](https://img.shields.io/github/stars/Easy2D/Easy2D?style=social)](https://github.com/Easy2D/Easy2D/stargazers)

**Made with ❤️ by [Nomango](https://github.com/nomango)**

</div>
