/**
 * Easy2D 精灵动画与特效合成演示
 *
 * 演示内容：
 * 1. 使用引擎 AnimationClip::createFromGridIndices 从精灵图创建动画
 * 2. 使用 AnimatedSprite 动画字典管理多方向行走动画
 * 3. 将角色渲染到离屏目标并应用特效
 * 4. 展示完整的纹理池 + 渲染目标 + 特效合成工作流程
 */

#include <easy2d/easy2d.h>
#include <easy2d/graphics/render_target.h>
#include <easy2d/graphics/shader_preset.h>
#include <easy2d/graphics/texture_pool.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <sstream>
#include <vector>

using namespace easy2d;

// 前向声明
struct ParticleData;

// ============================================================================
// 粒子数据
// ============================================================================
struct ParticleData {
  Vec2 position;
  Vec2 velocity;
  Color color;
  float size;
  float life;
  float maxLife;
  float alpha;
};

// ============================================================================
// 简单粒子系统 - 用于特效（发光、烟雾等）
// ============================================================================
class SimpleParticleSystem {
public:
  void update(float dt) {
    for (size_t i = 0; i < particles_.size(); i++) {
      auto &p = particles_[i];
      if (p.life > 0.0f) {
        p.position += p.velocity * dt;
        p.life -= dt;
        p.alpha = p.life / p.maxLife;
        p.size += dt * 10.0f;
      }
    }
  }

  void emitGlow(const Vec2 &pos, const Color &baseColor, int count = 5) {
    for (int i = 0; i < count; i++) {
      ParticleData p;
      float angle = (rand() / (float)RAND_MAX) * 2.0f * 3.14159f;
      float radius = 35.0f + (rand() / (float)RAND_MAX) * 15.0f;
      p.position =
          Vec2(pos.x + cos(angle) * radius, pos.y + sin(angle) * radius);
      p.velocity = Vec2(cos(angle) * 20.0f, sin(angle) * 20.0f);
      p.color = Color(std::min(1.0f, baseColor.r * 1.5f),
                      std::min(1.0f, baseColor.g * 1.5f),
                      std::min(1.0f, baseColor.b * 1.5f), 1.0f);
      p.size = 3.0f + (rand() / (float)RAND_MAX) * 5.0f;
      p.maxLife = 0.3f + (rand() / (float)RAND_MAX) * 0.3f;
      p.life = p.maxLife;
      p.alpha = 1.0f;
      particles_.push_back(p);
    }
  }

  void emitSmoke(const Vec2 &pos, int count = 3) {
    for (int i = 0; i < count; i++) {
      ParticleData p;
      p.position = Vec2(pos.x + (rand() / (float)RAND_MAX - 0.5f) * 30.0f,
                        pos.y + 40.0f + (rand() / (float)RAND_MAX) * 10.0f);
      p.velocity = Vec2((rand() / (float)RAND_MAX - 0.5f) * 30.0f,
                        -20.0f - (rand() / (float)RAND_MAX) * 20.0f);
      float gray = 0.6f + (rand() / (float)RAND_MAX) * 0.3f;
      p.color = Color(gray, gray, gray, 1.0f);
      p.size = 5.0f + (rand() / (float)RAND_MAX) * 8.0f;
      p.maxLife = 0.5f + (rand() / (float)RAND_MAX) * 0.5f;
      p.life = p.maxLife;
      p.alpha = 0.6f;
      particles_.push_back(p);
    }
  }

  void draw(RenderBackend &renderer) {
    particles_.erase(
        std::remove_if(particles_.begin(), particles_.end(),
                       [](const ParticleData &p) { return p.life <= 0.0f; }),
        particles_.end());

    renderer.beginSpriteBatch();
    for (size_t i = 0; i < particles_.size(); i++) {
      const auto &p = particles_[i];
      if (p.life > 0.0f) {
        Color c = p.color;
        c.a = p.alpha * p.life / p.maxLife;
        renderer.drawRect(Rect(p.position.x - p.size * 0.5f,
                               p.position.y - p.size * 0.5f, p.size, p.size),
                          c);
      }
    }
    renderer.endSpriteBatch();
  }

  void clear() { particles_.clear(); }

private:
  std::vector<ParticleData> particles_;
};

// ============================================================================
// 方向枚举
// ============================================================================
enum class Direction {
  Down,  // 向下（正面）
  Left,  // 向左
  Right, // 向右
  Up     // 向上（背面）
};

// ============================================================================
// 精灵动画演示场景
// ============================================================================
class SpriteAnimationScene : public Scene {
public:
  void onEnter() override {
    Scene::onEnter();

    setBackgroundColor(Color(0.1f, 0.1f, 0.15f, 1.0f));

    initSystems();
    createRenderTargets();
    createCharacter();
    createDisplaySprites();

    auto &resources = Application::instance().resources();
    font_ = resources.loadFont("C:/Windows/Fonts/simsun.ttc", 16);

    E2D_LOG_INFO("精灵动画演示初始化完成");
  }

  void onExit() override {
    E2D_TEXTURE_POOL().shutdown();
    Scene::onExit();
  }

  void onUpdate(float dt) override {
    Scene::onUpdate(dt);

    E2D_TEXTURE_POOL().update(dt);
    handleInput(dt);

    glowParticles_.update(dt);
    smokeParticles_.update(dt);

    glowEmitTimer_ += dt;
    if (glowEmitTimer_ >= 0.05f && character_) {
      glowEmitTimer_ = 0.0f;
      Color glowColor;
      switch (currentDir_) {
      case Direction::Down:
        glowColor = Color(1.0f, 0.8f, 0.4f, 1.0f);
        break;
      case Direction::Up:
        glowColor = Color(0.4f, 0.8f, 1.0f, 1.0f);
        break;
      case Direction::Left:
        glowColor = Color(1.0f, 0.4f, 0.4f, 1.0f);
        break;
      case Direction::Right:
        glowColor = Color(0.4f, 1.0f, 0.4f, 1.0f);
        break;
      }
      glowParticles_.emitGlow(Vec2(48.0f, 48.0f), glowColor, 3);
    }

    if (isMoving_) {
      smokeEmitTimer_ += dt;
      if (smokeEmitTimer_ >= 0.1f && character_) {
        smokeEmitTimer_ = 0.0f;
        smokeParticles_.emitSmoke(Vec2(48.0f, 48.0f), 2);
      }
    }

    performEffectComposition();
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

  // 渲染目标
  Ptr<RenderTarget> characterTarget_;
  Ptr<RenderTarget> effectTarget1_;
  Ptr<RenderTarget> finalTarget_;

  // 显示精灵
  Ptr<Sprite> originalSprite_;
  Ptr<Sprite> effect1Sprite_;
  Ptr<Sprite> finalSprite_;

  // 粒子系统
  SimpleParticleSystem glowParticles_;
  SimpleParticleSystem smokeParticles_;
  float glowEmitTimer_ = 0.0f;
  float smokeEmitTimer_ = 0.0f;

  // 控制状态
  bool isMoving_ = false;
  Direction currentDir_ = Direction::Down;
  float moveSpeed_ = 150.0f;

  // 帧尺寸（用于渲染到离屏目标）
  static constexpr int kFrameWidth = 96;
  static constexpr int kFrameHeight = 96;
  static constexpr int kSpacing = 0; // 精灵图实际没有间距

  void initSystems() {
    TexturePoolConfig poolConfig;
    poolConfig.maxCacheSize = 64 * 1024 * 1024;
    poolConfig.maxTextureCount = 256;
    E2D_TEXTURE_POOL().init(poolConfig);
    E2D_INFO("系统初始化完成");
  }

  void createRenderTargets() {
    RenderTargetConfig rtConfig;
    rtConfig.width = kFrameWidth;
    rtConfig.height = kFrameHeight;
    rtConfig.hasDepth = false;

    characterTarget_ = RenderTarget::createFromConfig(rtConfig);
    effectTarget1_ = RenderTarget::createFromConfig(rtConfig);
    finalTarget_ = RenderTarget::createFromConfig(rtConfig);

    E2D_INFO("渲染目标创建完成");
  }

  void createCharacter() {
    auto &resources = Application::instance().resources();

    spriteSheet_ = resources.loadTexture("player.png");
    if (!spriteSheet_) {
      E2D_ERROR("无法加载角色精灵图！请检查文件路径。");
      return;
    }
    E2D_INFO("成功加载精灵图: player.png");

    E2D_TEXTURE_POOL().add("player_spritesheet", spriteSheet_);

    // 使用引擎 API 从精灵图创建动画片段
    // 精灵图布局（96x96像素每帧，1px间距，4x4=16帧）：
    // 根据实际测试修正映射：
    // 第1行 (索引 0-3): 实际显示为 向下走（正面）
    // 第2行 (索引 4-7): 实际显示为 向右走（右侧面）
    // 第3行 (索引 8-11): 实际显示为 向上走（背面）- 但会消失，可能是空白
    // 第4行 (索引 12-15): 实际显示为 向左走（左侧面）
    float frameDurationMs = 125.0f; // 8 FPS

    // 根据实际显示效果重新映射
    E2D_INFO("精灵图尺寸: {}x{}, 帧尺寸: {}x{}, 间距: {}",
             spriteSheet_->getWidth(), spriteSheet_->getHeight(), kFrameWidth,
             kFrameHeight, kSpacing);

    auto walkUp = AnimationClip::createFromGridIndices(
        spriteSheet_, kFrameWidth, kFrameHeight, {0, 1, 2, 3}, frameDurationMs,
        kSpacing);
    E2D_INFO("walkUp 帧数: {}", walkUp->getFrameCount());

    auto walkRight = AnimationClip::createFromGridIndices(
        spriteSheet_, kFrameWidth, kFrameHeight, {4, 5, 6, 7}, frameDurationMs,
        kSpacing);
    E2D_INFO("walkRight 帧数: {}", walkRight->getFrameCount());

    auto walkLeft = AnimationClip::createFromGridIndices(
        spriteSheet_, kFrameWidth, kFrameHeight, {8, 9, 10, 11},
        frameDurationMs, kSpacing);
    E2D_INFO("walkUp 帧数: {}", walkLeft->getFrameCount());

    auto walkDown = AnimationClip::createFromGridIndices(
        spriteSheet_, kFrameWidth, kFrameHeight, {12, 13, 14, 15},
        frameDurationMs, kSpacing);
    E2D_INFO("walkDown 帧数: {}", walkDown->getFrameCount());

    // 创建 AnimatedSprite 并注册命名动画
    character_ = AnimatedSprite::create();
    character_->addAnimation("walk_up", walkUp);
    character_->addAnimation("walk_right", walkRight);
    character_->addAnimation("walk_left", walkLeft);
    character_->addAnimation("walk_down", walkDown);

    // 播放初始动画
    character_->play("walk_down", true);

    // play() 后设置位置（play 内部已关闭帧变换，不会覆盖位置）
    character_->setPosition(Vec2(450.0f, 400.0f));

    addChild(character_);

    E2D_INFO("角色动画精灵创建完成");
  }

  void createDisplaySprites() {
    float displayY = 180.0f;

    if (characterTarget_ && characterTarget_->getColorTexture()) {
      originalSprite_ = Sprite::create(characterTarget_->getColorTexture());
      originalSprite_->setPosition(Vec2(150.0f, displayY));
      originalSprite_->setScale(1.0f);
      originalSprite_->setAnchor(Vec2(0.5f, 0.5f));
      addChild(originalSprite_);
    }

    if (effectTarget1_ && effectTarget1_->getColorTexture()) {
      effect1Sprite_ = Sprite::create(effectTarget1_->getColorTexture());
      effect1Sprite_->setPosition(Vec2(450.0f, displayY));
      effect1Sprite_->setScale(1.0f);
      effect1Sprite_->setAnchor(Vec2(0.5f, 0.5f));
      addChild(effect1Sprite_);
    }

    if (finalTarget_ && finalTarget_->getColorTexture()) {
      finalSprite_ = Sprite::create(finalTarget_->getColorTexture());
      finalSprite_->setPosition(Vec2(750.0f, displayY));
      finalSprite_->setScale(1.0f);
      finalSprite_->setAnchor(Vec2(0.5f, 0.5f));
      addChild(finalSprite_);
    }

    E2D_INFO("显示精灵创建完成");
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

    // 切换动画（仅在方向改变或未播放时）
    if (currentDir_ != dir || !character_->isPlaying()) {
      std::string animName;
      switch (dir) {
      case Direction::Up:
        animName = "walk_up";
        break;
      case Direction::Down:
        animName = "walk_down";
        break;
      case Direction::Left:
        animName = "walk_left";
        break;
      case Direction::Right:
        animName = "walk_right";
        break;
      }

      if (character_->getCurrentAnimationName() != animName) {
        E2D_INFO("切换动画: {} -> {}", character_->getCurrentAnimationName(),
                 animName);
        character_->play(animName, true);
      } else if (!character_->isPlaying()) {
        character_->resume();
      }
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

  void performEffectComposition() {
    if (!character_ || !spriteSheet_)
      return;

    auto playerTex = E2D_TEXTURE_POOL().get("player_spritesheet");
    if (!playerTex)
      return;

    // 获取当前帧的纹理矩形
    auto clip = character_->getAnimationClip();
    if (!clip || clip->empty())
      return;

    size_t frameIdx = character_->getCurrentFrameIndex();
    static size_t lastFrameIdx = 999;
    static std::string lastAnimName = "";
    std::string currAnimName = character_->getCurrentAnimationName();

    const auto &frame = clip->getFrame(frameIdx);
    if (!frame.spriteFrame)
      return;
    Rect frameRect = frame.spriteFrame->getRect();

    if (frameIdx != lastFrameIdx || currAnimName != lastAnimName) {
      E2D_INFO("动画: {}, 帧索引: {}, 纹理矩形: ({}, {}) - {}x{}", currAnimName,
               frameIdx, frameRect.origin.x, frameRect.origin.y,
               frameRect.size.width, frameRect.size.height);
      lastFrameIdx = frameIdx;
      lastAnimName = currAnimName;
    }

    glm::mat4 ortho96 = glm::ortho(0.0f, 96.0f, 0.0f, 96.0f, -1.0f, 1.0f);

    // ========== 1. 渲染角色到离屏目标 ==========
    if (characterTarget_) {
      characterTarget_->bind();
      characterTarget_->clear(Color(0.0f, 0.0f, 0.0f, 0.0f));

      auto &renderer = Application::instance().renderer();
      renderer.setViewProjection(ortho96);

      float targetSize = 96.0f;
      float drawX = (targetSize - kFrameWidth) * 0.5f;
      float drawY = (targetSize - kFrameHeight) * 0.5f;

      renderer.beginSpriteBatch();
      renderer.drawSprite(
          *playerTex, Rect(drawX, drawY, kFrameWidth, kFrameHeight), frameRect,
          Color(1.0f, 1.0f, 1.0f, 1.0f), 0.0f, Vec2(0.0f, 0.0f));
      renderer.endSpriteBatch();

      characterTarget_->unbind();
    }

    // ========== 2. 应用第一层特效（发光效果） ==========
    if (effectTarget1_ && characterTarget_) {
      effectTarget1_->bind();
      effectTarget1_->clear(Color(0.0f, 0.0f, 0.0f, 0.0f));

      auto &renderer2 = Application::instance().renderer();
      renderer2.setViewProjection(ortho96);

      glowParticles_.draw(renderer2);

      renderer2.beginSpriteBatch();
      auto charTex = characterTarget_->getColorTexture();
      renderer2.drawSprite(*charTex, Rect(0.0f, 0.0f, 96.0f, 96.0f),
                           Rect(0.0f, 0.0f, 96.0f, 96.0f),
                           Color(1.0f, 1.0f, 1.0f, 1.0f), 0.0f,
                           Vec2(0.0f, 0.0f));
      renderer2.endSpriteBatch();

      effectTarget1_->unbind();
    }

    // ========== 3. 最终合成 ==========
    if (finalTarget_ && effectTarget1_) {
      finalTarget_->bind();
      finalTarget_->clear(Color(0.1f, 0.1f, 0.2f, 1.0f));

      auto &renderer3 = Application::instance().renderer();
      renderer3.setViewProjection(ortho96);

      smokeParticles_.draw(renderer3);

      renderer3.beginSpriteBatch();
      auto effTex = effectTarget1_->getColorTexture();
      renderer3.drawSprite(*effTex, Rect(0.0f, 0.0f, 96.0f, 96.0f),
                           Rect(0.0f, 0.0f, 96.0f, 96.0f),
                           Color(1.0f, 1.0f, 1.0f, 1.0f), 0.0f,
                           Vec2(0.0f, 0.0f));
      renderer3.endSpriteBatch();

      finalTarget_->unbind();
    }

    // 恢复屏幕的投影矩阵
    auto &renderer = Application::instance().renderer();
    glm::mat4 orthoScreen = glm::ortho(0.0f, 900.0f, 600.0f, 0.0f, -1.0f, 1.0f);
    renderer.setViewProjection(orthoScreen);
    renderer.setViewport(0, 0, 900, 600);
  }

  void drawUI(RenderBackend &renderer) {
    float y = 20.0f;

    renderer.drawText(*font_, "精灵动画与特效合成演示", Vec2(320.0f, y),
                      Color(1.0f, 0.9f, 0.2f, 1.0f));
    y += 30.0f;

    renderer.drawText(*font_, "使用 WASD 或方向键控制角色移动", Vec2(300.0f, y),
                      Color(0.8f, 0.8f, 0.8f, 1.0f));

    float labelY = 230.0f;
    renderer.drawText(*font_, "原始", Vec2(130.0f, labelY),
                      Color(0.7f, 0.9f, 0.7f, 1.0f));
    renderer.drawText(*font_, "特效层", Vec2(430.0f, labelY),
                      Color(0.7f, 0.9f, 0.7f, 1.0f));
    renderer.drawText(*font_, "最终合成", Vec2(720.0f, labelY),
                      Color(0.7f, 0.9f, 0.7f, 1.0f));

    y = 350.0f;
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

    y = 560.0f;
    auto texCount = E2D_TEXTURE_POOL().getTextureCount();
    auto texSize = E2D_TEXTURE_POOL().getCacheSize() / 1024;

    ss.str("");
    ss << "纹理池: " << texCount << " 个, " << texSize << " KB";
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
  E2D_LOG_INFO("精灵动画与特效合成演示");
  E2D_LOG_INFO("========================");

  auto &app = Application::instance();

  AppConfig config;
  config.title = "精灵动画与特效合成演示";
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
