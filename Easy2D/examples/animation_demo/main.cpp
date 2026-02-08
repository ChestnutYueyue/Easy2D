/**
 * Easy2D 精灵动画演示
 *
 * 演示内容：
 * 1. 使用引擎 AnimationClip::createFromGridIndices 从精灵图创建动画
 * 2. 使用 AnimatedSprite 动画字典管理多方向行走动画
 * 3. 使用 WASD 或方向键控制角色移动
 */

#include <easy2d/easy2d.h>
#include <iostream>
#include <sstream>

using namespace easy2d;

// ============================================================================
// 方向枚举
// ============================================================================
// 精灵图布局（96x96像素每帧，4x4=16帧）：
// 第1行 (索引 0-3):  向下走（正面，人物朝向你）
// 第2行 (索引 4-7):  向左走（左侧面）
// 第3行 (索引 8-11): 向右走（右侧面）
// 第4行 (索引 12-15): 向上走（背面，人物背对你）
enum class Direction {
  Down,  // 向下 - 帧 0-3（人物正面朝向你）
  Left,  // 向左 - 帧 4-7
  Right, // 向右 - 帧 8-11
  Up     // 向上 - 帧 12-15（人物背对你）
};

// ============================================================================
// 精灵动画演示场景
// ============================================================================
class SpriteAnimationScene : public Scene {
public:
  void onEnter() override {
    Scene::onEnter();

    setBackgroundColor(Color(0.1f, 0.1f, 0.15f, 1.0f));

    createCharacter();

    auto &resources = Application::instance().resources();
    font_ = resources.loadFont("C:/Windows/Fonts/simsun.ttc", 16);

    E2D_LOG_INFO("精灵动画演示初始化完成");
  }

  void onUpdate(float dt) override {
    Scene::onUpdate(dt);
    handleInput(dt);
    // 帧范围限制现在由 AnimatedSprite 内部处理
  }

  void onRender(RenderBackend &renderer) override {
    Scene::onRender(renderer);

    if (font_) {
      drawUI(renderer);
    }
  }

private:
  Ptr<FontAtlas> font_;
  Ptr<AnimatedSprite> character_;
  Ptr<Texture> spriteSheet_;

  // 控制状态
  bool isMoving_ = false;
  Direction currentDir_ = Direction::Down;  // 初始方向：向下（人物正面朝向你）
  float moveSpeed_ = 150.0f;

  // 帧尺寸
  static constexpr int kFrameWidth = 96;
  static constexpr int kFrameHeight = 96;
  static constexpr int kSpacing = 0;      // 精灵图实际没有间距
  static constexpr int kFramesPerDir = 4; // 每个方向的帧数

  void createCharacter() {
    auto &resources = Application::instance().resources();

    spriteSheet_ = resources.loadTexture("player.png");
    if (!spriteSheet_) {
      E2D_ERROR("无法加载角色精灵图！请检查文件路径。");
      return;
    }
    E2D_INFO("成功加载精灵图: player.png");

    // 使用引擎 API 从精灵图创建动画片段
    // 精灵图布局（96x96像素每帧，4x4=16帧）：
    // 第1行 (索引 0-3):  向下走（正面，人物朝向你）
    // 第2行 (索引 4-7):  向左走（左侧面）
    // 第3行 (索引 8-11): 向右走（右侧面）
    // 第4行 (索引 12-15): 向上走（背面，人物背对你）
    float frameDurationMs = 125.0f; // 8 FPS

    // 一次性创建整个精灵图的动画（16帧）
    auto walkClip = AnimationClip::createFromGrid(
        spriteSheet_, kFrameWidth, kFrameHeight, frameDurationMs, 16, kSpacing);
    if (!walkClip || walkClip->empty()) {
      E2D_ERROR("创建动画片段失败！");
      return;
    }
    walkClip->setLooping(true);
    E2D_INFO("动画片段创建成功，帧数: {}", walkClip->getFrameCount());

    // 创建 AnimatedSprite 并设置动画
    character_ = AnimatedSprite::create();
    // 禁用帧变换，避免动画帧数据覆盖节点位置
    character_->setApplyFrameTransform(false);
    character_->setAnimationClip(walkClip);

    // 设置初始帧范围（Direction::Down = 0，对应帧 0-3，人物正面朝向你）
    character_->setFrameRange(0, 3);
    character_->setFrameIndex(0);

    E2D_INFO("当前帧索引: {}", character_->getCurrentFrameIndex());
    E2D_INFO("总帧数: {}", character_->getTotalFrames());

    // 设置位置
    character_->setPosition(Vec2(450.0f, 300.0f));

    addChild(character_);

    E2D_INFO("角色动画精灵创建完成");
  }

  void handleInput(float dt) {
    auto &input = Application::instance().input();

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
    if (!character_)
      return;

    // 计算新方向的帧范围
    int newFrameStart = static_cast<int>(dir) * kFramesPerDir;
    int newFrameEnd = newFrameStart + kFramesPerDir - 1;

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
    isMoving_ = true;

    Vec2 pos = character_->getPosition();
    switch (dir) {
    case Direction::Down:
      pos.y += moveSpeed_ * dt;
      break;
    case Direction::Up:
      pos.y -= moveSpeed_ * dt;
      break;
    case Direction::Left:
      pos.x -= moveSpeed_ * dt;
      break;
    case Direction::Right:
      pos.x += moveSpeed_ * dt;
      break;
    }
    character_->setPosition(pos);
  }

  void drawUI(RenderBackend &renderer) {
    float y = 20.0f;

    renderer.drawText(*font_, "精灵动画演示", Vec2(380.0f, y),
                      Color(1.0f, 0.9f, 0.2f, 1.0f));
    y += 30.0f;

    renderer.drawText(*font_, "使用 WASD 或方向键控制角色移动", Vec2(300.0f, y),
                      Color(0.8f, 0.8f, 0.8f, 1.0f));

    y = 550.0f;
    std::string dirStr;
    switch (currentDir_) {
    case Direction::Down:
      dirStr = "下";
      break;
    case Direction::Up:
      dirStr = "上";
      break;
    case Direction::Left:
      dirStr = "左";
      break;
    case Direction::Right:
      dirStr = "右";
      break;
    }

    std::stringstream ss;
    ss << "方向: " << dirStr << "  状态: " << (isMoving_ ? "移动中" : "静止");
    renderer.drawText(*font_, ss.str(), Vec2(20.0f, y),
                      Color(0.5f, 1.0f, 0.5f, 1.0f));

    auto fps = Application::instance().fps();
    ss.str("");
    ss << "FPS: " << fps;
    renderer.drawText(*font_, ss.str(), Vec2(800.0f, y),
                      Color(0.5f, 1.0f, 0.5f, 1.0f));
  }
};

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char **argv) {
  Logger::init();
  Logger::setLevel(LogLevel::Debug);

  E2D_LOG_INFO("========================");
  E2D_LOG_INFO("精灵动画演示");
  E2D_LOG_INFO("========================");

  auto &app = Application::instance();

  AppConfig config;
  config.title = "精灵动画演示";
  config.width = 900;
  config.height = 600;
  config.vsync = true;

  if (!app.init(config)) {
    E2D_LOG_ERROR("初始化应用失败！");
    return -1;
  }

  auto &resources = app.resources();
  resources.addSearchPath("Easy2D/examples/sprite_animation_demo/assets");
  resources.addSearchPath("examples/sprite_animation_demo/assets");
  resources.addSearchPath("assets");
  E2D_INFO("资源搜索路径添加完成");

  app.enterScene(makePtr<SpriteAnimationScene>());

  E2D_LOG_INFO("启动主循环...");
  app.run();

  E2D_LOG_INFO("应用结束。");
  Logger::shutdown();

  return 0;
}
