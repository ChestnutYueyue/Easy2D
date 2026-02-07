/**
 * Easy2D 精灵动画与特效合成演示
 *
 * 演示内容：
 * 1. 加载角色精灵图并解析动画帧
 * 2. 实现角色走动动画（上下左右四个方向）
 * 3. 使用纹理池缓存动画帧
 * 4. 将角色渲染到离屏目标并应用特效
 * 5. 展示完整的纹理池 + 渲染目标 + 特效合成工作流程
 */

#include <easy2d/easy2d.h>
#include <easy2d/graphics/render_target.h>
#include <easy2d/graphics/shader_preset.h>
#include <easy2d/graphics/texture_pool.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

using namespace easy2d;

// ============================================================================
// 动画帧数据
// ============================================================================
struct AnimationFrame {
  Rect textureRect; // 在精灵图中的位置
  float duration;   // 帧持续时间（秒）
};

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
    // 更新现有粒子
    for (size_t i = 0; i < particles_.size(); i++) {
      auto &p = particles_[i];
      if (p.life > 0.0f) {
        p.position += p.velocity * dt;
        p.life -= dt;
        // 根据生命值计算透明度
        p.alpha = p.life / p.maxLife;
        // 粒子逐渐变大（烟雾扩散效果）
        p.size += dt * 10.0f;
      }
    }
  }

  void emitGlow(const Vec2 &pos, const Color &baseColor, int count = 5) {
    for (int i = 0; i < count; i++) {
      ParticleData p;
      // 在角色边缘随机位置生成
      float angle = (rand() / (float)RAND_MAX) * 2.0f * 3.14159f;
      float radius = 35.0f + (rand() / (float)RAND_MAX) * 15.0f;
      p.position =
          Vec2(pos.x + cos(angle) * radius, pos.y + sin(angle) * radius);

      // 向外扩散的速度
      p.velocity = Vec2(cos(angle) * 20.0f, sin(angle) * 20.0f);

      // 发光颜色（基于角色颜色但更亮）
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
      // 在脚底位置生成
      p.position = Vec2(pos.x + (rand() / (float)RAND_MAX - 0.5f) * 30.0f,
                        pos.y + 40.0f + (rand() / (float)RAND_MAX) * 10.0f);

      // 向上飘动的速度
      p.velocity = Vec2((rand() / (float)RAND_MAX - 0.5f) * 30.0f,
                        -20.0f - (rand() / (float)RAND_MAX) * 20.0f);

      // 烟雾颜色（灰白色）
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
    // 移除死亡的粒子
    particles_.erase(
        std::remove_if(particles_.begin(), particles_.end(),
                       [](const ParticleData &p) { return p.life <= 0.0f; }),
        particles_.end());

    // 绘制粒子
    renderer.beginSpriteBatch();
    for (size_t i = 0; i < particles_.size(); i++) {
      const auto &p = particles_[i];
      if (p.life > 0.0f) {
        Color c = p.color;
        c.a = p.alpha * p.life / p.maxLife;
        // 绘制简单的圆形粒子（使用小矩形近似）
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
// 动画精灵类 - 支持多方向行走动画
// ============================================================================
class AnimatedSprite : public Node {
public:
  AnimatedSprite() = default;

  // 设置精灵图纹理
  void setSpriteSheet(Ptr<Texture> texture, int frameWidth, int frameHeight) {
    spriteSheet_ = texture;
    frameWidth_ = frameWidth;
    frameHeight_ = frameHeight;

    if (texture) {
      // 计算每行每列的帧数
      framesPerRow_ = texture->getWidth() / frameWidth;
      framesPerCol_ = texture->getHeight() / frameHeight;
    }
  }

  // 添加动画
  void addAnimation(const std::string &name, Direction dir,
                    const std::vector<int> &frameIndices, float fps = 8.0f) {
    AnimationData anim;
    anim.fps = fps;
    anim.frameDuration = 1.0f / fps;

    for (int idx : frameIndices) {
      AnimationFrame frame;
      frame.textureRect = calculateFrameRect(idx);
      frame.duration = anim.frameDuration;
      anim.frames.push_back(frame);
    }

    animations_[name][dir] = anim;
  }

  // 播放动画
  void play(const std::string &name, Direction dir, bool loop = true) {
    auto it = animations_.find(name);
    if (it != animations_.end()) {
      auto dirIt = it->second.find(dir);
      if (dirIt != it->second.end()) {
        currentAnim_ = name;
        currentDir_ = dir;
        currentFrame_ = 0;
        animTimer_ = 0.0f;
        isLooping_ = loop;
        isPlaying_ = true;

        // 更新显示精灵的纹理矩形
        updateSpriteFrame();
      }
    }
  }

  // 停止动画
  void stop() { isPlaying_ = false; }

  // 设置移动速度
  void setMoveSpeed(float speed) { moveSpeed_ = speed; }

  // 向指定方向移动
  void move(Direction dir, float dt) {
    // 启动行走动画（只在动画未播放或方向改变时）
    if (!isPlaying_ || currentAnim_ != "walk" || currentDir_ != dir) {
      play("walk", dir);
    }

    currentDir_ = dir;

    Vec2 pos = getPosition();
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
    setPosition(pos);
  }

  // 获取当前帧的纹理（用于渲染到离屏目标）
  Ptr<Texture> getCurrentFrameTexture() const { return spriteSheet_; }
  Rect getCurrentFrameRect() const { return currentFrameRect_; }
  int getFrameWidth() const { return frameWidth_; }
  int getFrameHeight() const { return frameHeight_; }

protected:
  void onUpdate(float dt) override {
    Node::onUpdate(dt);

    if (!isPlaying_)
      return;

    auto it = animations_.find(currentAnim_);
    if (it == animations_.end())
      return;

    auto dirIt = it->second.find(currentDir_);
    if (dirIt == it->second.end())
      return;

    const auto &anim = dirIt->second;
    if (anim.frames.empty())
      return;

    // 更新动画计时器
    animTimer_ += dt;

    // 检查是否需要切换到下一帧
    if (animTimer_ >= anim.frames[currentFrame_].duration) {
      animTimer_ -= anim.frames[currentFrame_].duration;
      currentFrame_++;

      // 循环或停止
      if (currentFrame_ >= anim.frames.size()) {
        if (isLooping_) {
          currentFrame_ = 0;
        } else {
          currentFrame_ = anim.frames.size() - 1;
          isPlaying_ = false;
        }
      }

      // 更新显示
      updateSpriteFrame();
    }
  }

  void onDraw(RenderBackend &renderer) override {
    if (!spriteSheet_)
      return;

    // 绘制当前帧
    auto pos = getPosition();
    // 使用中心锚点 (0.5, 0.5)，这样 pos 就是精灵的中心位置
    Vec2 anchor(0.5f, 0.5f);

    renderer.beginSpriteBatch();
    renderer.drawSprite(*spriteSheet_,
                        Rect(pos.x, pos.y, frameWidth_, frameHeight_),
                        currentFrameRect_, Color(1.0f, 1.0f, 1.0f, 1.0f),
                        getRotation(), anchor);
    renderer.endSpriteBatch();
  }

private:
  struct AnimationData {
    std::vector<AnimationFrame> frames;
    float fps = 8.0f;
    float frameDuration = 0.125f;
  };

  Ptr<Texture> spriteSheet_; // 精灵图纹理
  int frameWidth_ = 32;      // 单帧宽度
  int frameHeight_ = 48;     // 单帧高度
  int framesPerRow_ = 4;     // 每行帧数
  int framesPerCol_ = 4;     // 每列帧数

  // 动画数据
  std::map<std::string, std::map<Direction, AnimationData>> animations_;

  // 当前动画状态
  std::string currentAnim_;
  Direction currentDir_ = Direction::Down;
  int currentFrame_ = 0;
  float animTimer_ = 0.0f;
  bool isLooping_ = true;
  bool isPlaying_ = false;

  // 移动
  float moveSpeed_ = 100.0f;

  // 当前显示的纹理矩形
  Rect currentFrameRect_;

  // 计算帧在精灵图中的位置（96x96像素，1px间距）
  Rect calculateFrameRect(int frameIndex) const {
    int row = frameIndex / framesPerRow_;
    int col = frameIndex % framesPerRow_;

    // 每帧96x96，加上1px间距
    float x = col * (frameWidth_ + 1);
    float y = row * (frameHeight_ + 1);

    return Rect(x, y, frameWidth_, frameHeight_);
  }

  // 更新精灵显示的帧
  void updateSpriteFrame() {
    auto it = animations_.find(currentAnim_);
    if (it == animations_.end())
      return;

    auto dirIt = it->second.find(currentDir_);
    if (dirIt == it->second.end())
      return;

    const auto &anim = dirIt->second;
    if (currentFrame_ < anim.frames.size()) {
      currentFrameRect_ = anim.frames[currentFrame_].textureRect;
    }
  }
};

// ============================================================================
// 精灵动画演示场景
// ============================================================================
class SpriteAnimationScene : public Scene {
public:
  void onEnter() override {
    Scene::onEnter();

    setBackgroundColor(Color(0.1f, 0.1f, 0.15f, 1.0f));

    // 初始化系统
    initSystems();

    // 创建渲染目标（用于特效合成）
    createRenderTargets();

    // 创建角色动画精灵
    createCharacter();

    // 创建显示用的精灵（显示特效合成结果）
    createDisplaySprites();

    // 加载字体
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

    // 更新纹理池
    E2D_TEXTURE_POOL().update(dt);

    // 处理输入控制角色移动
    handleInput(dt);

    // 更新粒子系统
    glowParticles_.update(dt);
    smokeParticles_.update(dt);

    // 发射发光粒子（持续发射）
    glowEmitTimer_ += dt;
    if (glowEmitTimer_ >= 0.05f && character_) {
      glowEmitTimer_ = 0.0f;
      // 根据方向选择发光颜色
      Color glowColor;
      switch (currentDir_) {
      case Direction::Down:
        glowColor = Color(1.0f, 0.8f, 0.4f, 1.0f);
        break; // 金色
      case Direction::Up:
        glowColor = Color(0.4f, 0.8f, 1.0f, 1.0f);
        break; // 蓝色
      case Direction::Left:
        glowColor = Color(1.0f, 0.4f, 0.4f, 1.0f);
        break; // 红色
      case Direction::Right:
        glowColor = Color(0.4f, 1.0f, 0.4f, 1.0f);
        break; // 绿色
      }
      glowParticles_.emitGlow(Vec2(48.0f, 48.0f), glowColor, 3);
    }

    // 发射烟雾粒子（只在移动时）
    if (isMoving_) {
      smokeEmitTimer_ += dt;
      if (smokeEmitTimer_ >= 0.1f && character_) {
        smokeEmitTimer_ = 0.0f;
        smokeParticles_.emitSmoke(Vec2(48.0f, 48.0f), 2);
      }
    }

    // 执行特效合成
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
  Ptr<AnimatedSprite> character_; // 角色动画精灵

  // 渲染目标
  Ptr<RenderTarget> characterTarget_; // 角色渲染目标
  Ptr<RenderTarget> effectTarget1_;   // 特效层1
  Ptr<RenderTarget> finalTarget_;     // 最终合成

  // 显示精灵
  Ptr<Sprite> originalSprite_; // 原始角色
  Ptr<Sprite> effect1Sprite_;  // 第一层特效
  Ptr<Sprite> finalSprite_;    // 最终合成

  // 粒子系统
  SimpleParticleSystem glowParticles_;  // 边缘发光粒子
  SimpleParticleSystem smokeParticles_; // 脚底烟雾粒子
  float glowEmitTimer_ = 0.0f;          // 发光发射计时器
  float smokeEmitTimer_ = 0.0f;         // 烟雾发射计时器

  // 控制状态
  bool isMoving_ = false;
  Direction currentDir_ = Direction::Down;

  void initSystems() {
    TexturePoolConfig poolConfig;
    poolConfig.maxCacheSize = 64 * 1024 * 1024;
    poolConfig.maxTextureCount = 256;
    E2D_TEXTURE_POOL().init(poolConfig);
    E2D_INFO("系统初始化完成");
  }

  void createRenderTargets() {
    RenderTargetConfig rtConfig;
    rtConfig.width = 96;
    rtConfig.height = 96;
    rtConfig.hasDepth = false;

    characterTarget_ = RenderTarget::createFromConfig(rtConfig);
    effectTarget1_ = RenderTarget::createFromConfig(rtConfig);
    finalTarget_ = RenderTarget::createFromConfig(rtConfig);

    E2D_INFO("渲染目标创建完成");
  }

  void createCharacter() {
    auto &resources = Application::instance().resources();

    // 加载角色精灵图（使用资源搜索路径）
    auto spriteSheet = resources.loadTexture("player.png");

    if (!spriteSheet) {
      E2D_ERROR("无法加载角色精灵图！请检查文件路径。");
      return;
    }
    E2D_INFO("成功加载精灵图: player.png");

    // 将纹理加入纹理池
    E2D_TEXTURE_POOL().add("player_spritesheet", spriteSheet);

    // 创建动画精灵
    character_ = makePtr<AnimatedSprite>();

    // 设置精灵图（96x96像素每帧，1px间距，4x4布局=16帧）
    // 第1行: 向上走 (0,1,2,3) - 背面
    // 第2行: 向右走 (4,5,6,7) - 角色面朝右
    // 第3行: 向左走 (8,9,10,11) - 角色面朝左
    // 第4行: 向下走 (12,13,14,15) - 正面
    character_->setSpriteSheet(spriteSheet, 96, 96);

    // 添加行走动画（四个方向）
    character_->addAnimation("walk", Direction::Up, {0, 1, 2, 3},
                             8.0f); // 第1行是向上（背面）
    character_->addAnimation("walk", Direction::Right, {4, 5, 6, 7},
                             8.0f); // 第2行是向右
    character_->addAnimation("walk", Direction::Left, {8, 9, 10, 11},
                             8.0f); // 第3行是向左
    character_->addAnimation("walk", Direction::Down, {12, 13, 14, 15},
                             8.0f); // 第4行是向下（正面）

    // 设置初始位置
    character_->setPosition(Vec2(450.0f, 300.0f));
    character_->setMoveSpeed(150.0f);

    // 播放初始动画
    character_->play("walk", Direction::Down);

    // 添加到场景
    addChild(character_);

    E2D_INFO("角色动画精灵创建完成");
  }

  void createDisplaySprites() {
    // 创建显示特效合成结果的精灵
    // 文字标注在 y = 230 左右，精灵显示在文字上方
    float displayY = 180.0f; // 精灵中心位置

    // 1. 原始角色显示
    if (characterTarget_ && characterTarget_->getColorTexture()) {
      originalSprite_ = Sprite::create(characterTarget_->getColorTexture());
      originalSprite_->setPosition(Vec2(150.0f, displayY));
      originalSprite_->setScale(1.0f);
      originalSprite_->setAnchor(Vec2(0.5f, 0.5f)); // 设置中心锚点
      addChild(originalSprite_);
    }

    // 2. 第一层特效
    if (effectTarget1_ && effectTarget1_->getColorTexture()) {
      effect1Sprite_ = Sprite::create(effectTarget1_->getColorTexture());
      effect1Sprite_->setPosition(Vec2(450.0f, displayY));
      effect1Sprite_->setScale(1.0f);
      effect1Sprite_->setAnchor(Vec2(0.5f, 0.5f)); // 设置中心锚点
      addChild(effect1Sprite_);
    }

    // 3. 最终合成
    if (finalTarget_ && finalTarget_->getColorTexture()) {
      finalSprite_ = Sprite::create(finalTarget_->getColorTexture());
      finalSprite_->setPosition(Vec2(750.0f, displayY));
      finalSprite_->setScale(1.0f);
      finalSprite_->setAnchor(Vec2(0.5f, 0.5f)); // 设置中心锚点
      addChild(finalSprite_);
    }

    E2D_INFO("显示精灵创建完成");
  }

  void handleInput(float dt) {
    auto &input = Application::instance().input();

    isMoving_ = false;

    // 键盘控制移动（使用 isKeyDown 检测按住状态）
    if (input.isKeyDown(Key::W) || input.isKeyDown(Key::Up)) {
      character_->move(Direction::Up, dt);
      currentDir_ = Direction::Up;
      isMoving_ = true;
    } else if (input.isKeyDown(Key::S) || input.isKeyDown(Key::Down)) {
      character_->move(Direction::Down, dt);
      currentDir_ = Direction::Down;
      isMoving_ = true;
    } else if (input.isKeyDown(Key::A) || input.isKeyDown(Key::Left)) {
      character_->move(Direction::Left, dt);
      currentDir_ = Direction::Left;
      isMoving_ = true;
    } else if (input.isKeyDown(Key::D) || input.isKeyDown(Key::Right)) {
      character_->move(Direction::Right, dt);
      currentDir_ = Direction::Right;
      isMoving_ = true;
    }

    // 停止时暂停动画
    if (!isMoving_ && character_) {
      character_->stop();
    }
  }

  void performEffectComposition() {
    if (!character_)
      return;

    // 从纹理池获取角色纹理
    auto playerTex = E2D_TEXTURE_POOL().get("player_spritesheet");
    if (!playerTex)
      return;

    // 获取当前动画帧的纹理矩形
    Rect frameRect = character_->getCurrentFrameRect();

    // 创建96x96渲染目标的正交投影矩阵（Y轴翻转，使纹理正立）
    glm::mat4 ortho96 = glm::ortho(0.0f, 96.0f, 0.0f, 96.0f, -1.0f, 1.0f);

    // ========== 1. 渲染角色到离屏目标 ==========
    if (characterTarget_) {
      characterTarget_->bind();
      characterTarget_->clear(Color(0.0f, 0.0f, 0.0f, 0.0f));

      // 设置96x96的投影矩阵
      auto &renderer = Application::instance().renderer();
      renderer.setViewProjection(ortho96);

      // 计算居中位置
      float targetSize = 96.0f;
      float frameW = character_->getFrameWidth();
      float frameH = character_->getFrameHeight();
      float drawX = (targetSize - frameW) * 0.5f;
      float drawY = (targetSize - frameH) * 0.5f;

      // 绘制角色帧（锚点为左上角，因为drawX/drawY已经是左上角位置）
      renderer.beginSpriteBatch();
      renderer.drawSprite(*playerTex, Rect(drawX, drawY, frameW, frameH),
                          frameRect, Color(1.0f, 1.0f, 1.0f, 1.0f), 0.0f,
                          Vec2(0.0f, 0.0f));
      renderer.endSpriteBatch();

      characterTarget_->unbind();
    }

    // ========== 2. 应用第一层特效（发光效果） ==========
    if (effectTarget1_ && characterTarget_) {
      effectTarget1_->bind();
      effectTarget1_->clear(Color(0.0f, 0.0f, 0.0f, 0.0f));

      // 设置96x96的投影矩阵
      auto &renderer2 = Application::instance().renderer();
      renderer2.setViewProjection(ortho96);

      // 绘制发光粒子（在角色下方）
      glowParticles_.draw(renderer2);

      // 绘制原始角色（作为发光基础）- 使用Rect版本确保正确尺寸
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

      // 设置96x96的投影矩阵
      auto &renderer3 = Application::instance().renderer();
      renderer3.setViewProjection(ortho96);

      // 绘制烟雾粒子（在角色下方）
      smokeParticles_.draw(renderer3);

      // 绘制特效层 - 使用Rect版本确保正确尺寸
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

    // 标题
    renderer.drawText(*font_, "精灵动画与特效合成演示", Vec2(320.0f, y),
                      Color(1.0f, 0.9f, 0.2f, 1.0f));
    y += 30.0f;

    // 操作说明
    renderer.drawText(*font_, "使用 WASD 或方向键控制角色移动", Vec2(300.0f, y),
                      Color(0.8f, 0.8f, 0.8f, 1.0f));
    y += 40.0f;

    // 标注显示区域（精灵在 y=180，文字在精灵下方）
    float labelY = 230.0f;
    renderer.drawText(*font_, "原始", Vec2(130.0f, labelY),
                      Color(0.7f, 0.9f, 0.7f, 1.0f));
    renderer.drawText(*font_, "特效层", Vec2(430.0f, labelY),
                      Color(0.7f, 0.9f, 0.7f, 1.0f));
    renderer.drawText(*font_, "最终合成", Vec2(720.0f, labelY),
                      Color(0.7f, 0.9f, 0.7f, 1.0f));

    // 显示移动状态
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

    // 纹理池统计
    y = 560.0f;
    auto texCount = E2D_TEXTURE_POOL().getTextureCount();
    auto texSize = E2D_TEXTURE_POOL().getCacheSize() / 1024;

    ss.str("");
    ss << "纹理池: " << texCount << " 个, " << texSize << " KB";
    renderer.drawText(*font_, ss.str(), Vec2(20.0f, y),
                      Color(0.5f, 1.0f, 0.5f, 1.0f));

    // FPS
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

  // 添加资源搜索路径
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
