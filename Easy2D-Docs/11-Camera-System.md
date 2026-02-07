# Easy2D 相机系统 API 文档

## 1. 相机系统概述

Easy2D 的相机系统使用正交投影(Orthographic Projection)，适用于2D游戏开发。相机控制视图的显示区域、位置、旋转和缩放。

### 1.1 头文件
```cpp
#include <easy2d/graphics/camera.h>
```

### 1.2 获取相机
```cpp
// 方式1: 获取应用默认相机
auto& camera = Application::instance().camera();

// 方式2: 为场景设置自定义相机
auto customCamera = makePtr<Camera>();
setCamera(customCamera);

// 方式3: 获取场景当前激活的相机
Camera* activeCam = getActiveCamera();
```

---

## 2. 创建相机

### 2.1 默认构造
```cpp
// 创建默认相机
auto camera = makePtr<Camera>();
```

### 2.2 使用视口尺寸创建
```cpp
// 使用指定视口创建
auto camera = makePtr<Camera>(0.0f, 800.0f, 600.0f, 0.0f);
// 参数: left, right, bottom, top

// 或使用Size结构
Size viewportSize(800.0f, 600.0f);
auto camera = makePtr<Camera>(viewportSize);
```

---

## 3. 相机位置和变换

### 3.1 设置位置
```cpp
// 方式1: 使用Vec2
camera.setPosition(Vec2(100.0f, 200.0f));

// 方式2: 使用坐标
camera.setPosition(100.0f, 200.0f);

// 获取当前位置
Vec2 pos = camera.getPosition();
```

### 3.2 设置旋转
```cpp
// 设置旋转角度（度）
camera.setRotation(45.0f);

// 获取当前旋转角度
float rotation = camera.getRotation();
```

### 3.3 设置缩放
```cpp
// 设置缩放级别 (1.0 = 原始大小, 2.0 = 放大2倍, 0.5 = 缩小一半)
camera.setZoom(2.0f);

// 获取当前缩放
float zoom = camera.getZoom();
```

### 3.4 移动相机
```cpp
// 方式1: 使用Vec2偏移
camera.move(Vec2(10.0f, 0.0f));  // 向右移动10像素

// 方式2: 使用坐标偏移
camera.move(10.0f, 0.0f);
```

### 3.5 看向某点
```cpp
// 让相机中心对准目标点
camera.lookAt(Vec2(500.0f, 300.0f));
```

---

## 4. 视口设置

### 4.1 设置视口
```cpp
// 方式1: 使用四个边界值
camera.setViewport(0.0f, 800.0f, 600.0f, 0.0f);
// 参数: left, right, bottom, top

// 方式2: 使用Rect
camera.setViewport(Rect(0.0f, 0.0f, 800.0f, 600.0f));

// 获取当前视口
Rect viewport = camera.getViewport();
```

---

## 5. 坐标转换

### 5.1 屏幕坐标转世界坐标
```cpp
// 将鼠标位置转换为世界坐标
Vec2 mousePos = input.getMousePosition();
Vec2 worldPos = camera.screenToWorld(mousePos);

// 或直接传入坐标
Vec2 worldPos = camera.screenToWorld(mouseX, mouseY);
```

### 5.2 世界坐标转屏幕坐标
```cpp
// 将世界坐标转换为屏幕坐标
Vec2 screenPos = camera.worldToScreen(worldPos);

// 或直接传入坐标
Vec2 screenPos = camera.worldToScreen(worldX, worldY);
```

### 5.3 使用场景获取激活相机转换
```cpp
class MyScene : public Scene {
public:
    void onUpdate(float dt) override {
        auto& input = Application::instance().input();
        
        // 获取激活相机并转换坐标
        if (auto* camera = getActiveCamera()) {
            Vec2 mouseWorldPos = camera->screenToWorld(input.getMousePosition());
            
            // 检查点击了哪个游戏对象
            for (auto& obj : gameObjects_) {
                if (obj->getBoundingBox().contains(mouseWorldPos)) {
                    obj->onClick();
                }
            }
        }
    }
};
```

---

## 6. 边界限制

### 6.1 设置相机边界
```cpp
// 限制相机移动范围
camera.setBounds(Rect(0.0f, 0.0f, 2000.0f, 1500.0f));

// 清除边界限制
camera.clearBounds();

// 手动将相机限制在边界内
camera.clampToBounds();
```

### 6.2 边界限制示例
```cpp
class GameScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        
        // 创建自定义相机
        auto camera = makePtr<Camera>();
        camera->setViewport(0.0f, 800.0f, 600.0f, 0.0f);
        
        // 设置世界边界（地图大小为2000x1500）
        camera->setBounds(Rect(0.0f, 0.0f, 2000.0f, 1500.0f));
        
        // 应用到场景
        setCamera(camera);
    }
    
    void onUpdate(float dt) override {
        Scene::onUpdate(dt);
        
        auto& input = Application::instance().input();
        auto* camera = getActiveCamera();
        if (!camera) return;
        
        // 相机移动控制
        float speed = 200.0f * dt;
        if (input.isKeyDown(Key::W)) {
            camera->move(0.0f, -speed);
        }
        if (input.isKeyDown(Key::S)) {
            camera->move(0.0f, speed);
        }
        if (input.isKeyDown(Key::A)) {
            camera->move(-speed, 0.0f);
        }
        if (input.isKeyDown(Key::D)) {
            camera->move(speed, 0.0f);
        }
        
        // 确保相机不超出边界
        camera->clampToBounds();
    }
};
```

---

## 7. 跟随相机

### 7.1 简单的相机跟随
```cpp
class FollowCameraScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        
        // 创建相机
        camera_ = makePtr<Camera>();
        camera_->setViewport(0.0f, 800.0f, 600.0f, 0.0f);
        setCamera(camera_);
        
        // 创建玩家
        player_ = Sprite::create(playerTexture_);
        player_->setPosition(Vec2(400.0f, 300.0f));
        addChild(player_);
    }
    
    void onUpdate(float dt) override {
        Scene::onUpdate(dt);
        
        // 玩家移动
        handlePlayerMovement(dt);
        
        // 相机跟随玩家
        if (camera_ && player_) {
            Vec2 playerPos = player_->getPosition();
            
            // 平滑跟随
            Vec2 currentPos = camera_->getPosition();
            Vec2 targetPos = playerPos - Vec2(400.0f, 300.0f);  // 中心偏移
            
            float smoothSpeed = 5.0f * dt;
            Vec2 newPos = currentPos + (targetPos - currentPos) * smoothSpeed;
            
            camera_->setPosition(newPos);
            camera_->clampToBounds();
        }
    }

private:
    Ptr<Camera> camera_;
    Ptr<Sprite> player_;
    
    void handlePlayerMovement(float dt) {
        auto& input = Application::instance().input();
        float speed = 200.0f * dt;
        Vec2 pos = player_->getPosition();
        
        if (input.isKeyDown(Key::W)) pos.y -= speed;
        if (input.isKeyDown(Key::S)) pos.y += speed;
        if (input.isKeyDown(Key::A)) pos.x -= speed;
        if (input.isKeyDown(Key::D)) pos.x += speed;
        
        player_->setPosition(pos);
    }
};
```

---

## 8. 矩阵获取

### 8.1 获取变换矩阵
```cpp
// 获取视图矩阵
glm::mat4 viewMatrix = camera.getViewMatrix();

// 获取投影矩阵
glm::mat4 projMatrix = camera.getProjectionMatrix();

// 获取视图投影矩阵（VP矩阵）
glm::mat4 vpMatrix = camera.getViewProjectionMatrix();
```

### 8.2 自定义渲染使用
```cpp
void onRender(RenderBackend& renderer) override {
    Scene::onRender(renderer);
    
    if (auto* camera = getActiveCamera()) {
        // 使用相机的VP矩阵进行自定义渲染
        renderer.setViewProjection(camera->getViewProjectionMatrix());
        
        // 绘制世界坐标系下的内容
        drawWorldContent(renderer);
    }
}
```

---

## 9. 完整示例

```cpp
#include <easy2d/easy2d.h>
#include <easy2d/utils/logger.h>
#include <sstream>

using namespace easy2d;

// ============================================================================
// 相机演示场景
// ============================================================================
class CameraDemoScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        setBackgroundColor(Color(0.05f, 0.05f, 0.1f, 1.0f));
        
        // 创建自定义相机
        camera_ = makePtr<Camera>();
        camera_->setViewport(0.0f, 800.0f, 600.0f, 0.0f);
        
        // 设置世界边界
        camera_->setBounds(Rect(0.0f, 0.0f, 2000.0f, 1500.0f));
        
        setCamera(camera_);
        
        // 加载字体
        auto& resources = Application::instance().resources();
        font_ = resources.loadFont("C:/Windows/Fonts/simsun.ttc", 18);
        
        // 创建网格背景
        createGrid();
        
        // 创建玩家
        player_ = makePtr<Node>();
        player_->setPosition(Vec2(1000.0f, 750.0f));
        addChild(player_);
    }
    
    void onUpdate(float dt) override {
        Scene::onUpdate(dt);
        
        time_ += dt;
        
        handleInput(dt);
        updateCamera(dt);
    }
    
    void onRender(RenderBackend& renderer) override {
        Scene::onRender(renderer);
        
        // 绘制玩家
        if (player_) {
            Vec2 pos = player_->getPosition();
            renderer.fillCircle(pos, 20.0f, Color(1.0f, 0.5f, 0.0f, 1.0f));
        }
        
        // 绘制UI
        drawUI(renderer);
    }

private:
    Ptr<Camera> camera_;
    Ptr<Node> player_;
    Ptr<FontAtlas> font_;
    float time_ = 0.0f;
    float zoomLevel_ = 1.0f;
    
    void createGrid() {
        // 创建网格作为参考
        for (int i = 0; i < 20; ++i) {
            for (int j = 0; j < 15; ++j) {
                auto node = makePtr<Node>();
                node->setPosition(Vec2(i * 100.0f + 50.0f, j * 100.0f + 50.0f));
                addChild(node);
            }
        }
    }
    
    void handleInput(float dt) {
        auto& input = Application::instance().input();
        
        // 玩家移动
        if (!player_) return;
        
        float speed = 300.0f * dt;
        Vec2 pos = player_->getPosition();
        
        if (input.isKeyDown(Key::W)) pos.y -= speed;
        if (input.isKeyDown(Key::S)) pos.y += speed;
        if (input.isKeyDown(Key::A)) pos.x -= speed;
        if (input.isKeyDown(Key::D)) pos.x += speed;
        
        player_->setPosition(pos);
        
        // 缩放控制
        if (input.isKeyPressed(Key::Q)) {
            zoomLevel_ = std::max(0.5f, zoomLevel_ - 0.1f);
        }
        if (input.isKeyPressed(Key::E)) {
            zoomLevel_ = std::min(3.0f, zoomLevel_ + 0.1f);
        }
        
        // 重置相机
        if (input.isKeyPressed(Key::R)) {
            zoomLevel_ = 1.0f;
            if (camera_) {
                camera_->setRotation(0.0f);
            }
        }
    }
    
    void updateCamera(float dt) {
        if (!camera_ || !player_) return;
        
        // 平滑缩放
        float currentZoom = camera_->getZoom();
        float newZoom = currentZoom + (zoomLevel_ - currentZoom) * dt * 5.0f;
        camera_->setZoom(newZoom);
        
        // 相机跟随玩家
        Vec2 playerPos = player_->getPosition();
        Vec2 currentPos = camera_->getPosition();
        
        // 视口中心偏移
        Vec2 targetPos = playerPos - Vec2(400.0f, 300.0f);
        
        // 平滑跟随
        Vec2 newPos = currentPos + (targetPos - currentPos) * dt * 3.0f;
        camera_->setPosition(newPos);
        
        // 限制在边界内
        camera_->clampToBounds();
    }
    
    void drawUI(RenderBackend& renderer) {
        if (!font_) return;
        
        // 标题
        renderer.drawText(*font_, "相机系统演示",
                         Vec2(320.0f, 20.0f), Color(1.0f, 0.9f, 0.2f, 1.0f));
        
        // 操作说明
        float y = 60.0f;
        renderer.drawText(*font_, "WASD - 移动玩家",
                         Vec2(20.0f, y), Color(0.8f, 0.8f, 0.8f, 1.0f));
        y += 25.0f;
        
        renderer.drawText(*font_, "Q/E - 缩放",
                         Vec2(20.0f, y), Color(0.8f, 0.8f, 0.8f, 1.0f));
        y += 25.0f;
        
        renderer.drawText(*font_, "R - 重置相机",
                         Vec2(20.0f, y), Color(0.8f, 0.8f, 0.8f, 1.0f));
        
        // 相机信息
        if (camera_) {
            Vec2 pos = camera_->getPosition();
            float zoom = camera_->getZoom();
            
            std::stringstream ss;
            ss << "相机位置: (" << (int)pos.x << ", " << (int)pos.y << ")";
            renderer.drawText(*font_, ss.str(),
                            Vec2(500.0f, 500.0f), Color(0.5f, 1.0f, 0.5f, 1.0f));
            
            ss.str("");
            ss << "缩放: " << std::fixed << std::setprecision(2) << zoom << "x";
            renderer.drawText(*font_, ss.str(),
                            Vec2(500.0f, 525.0f), Color(0.5f, 1.0f, 0.5f, 1.0f));
        }
        
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
    config.title = "相机系统演示";
    config.width = 800;
    config.height = 600;
    config.vsync = true;
    
    if (!app.init(config)) {
        E2D_LOG_ERROR("初始化失败！");
        return -1;
    }
    
    app.enterScene(makePtr<CameraDemoScene>());
    app.run();
    
    Logger::shutdown();
    return 0;
}
```
