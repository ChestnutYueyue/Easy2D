#pragma once
#include <easy2d/e2dmacros.h>
#include <easy2d/e2dmath.h>
#include <easy2d/e2dcommon.h>
#include "GLFrameBuffer.h"
#include "GLShader.h"
#include "GLTexture.h"
#include <vector>

namespace easy2d
{

/**
 * @brief 后期处理效果基类
 * 
 * 所有后期处理效果的基类
 */
class GLPostProcessEffect
{
public:
    /**
     * @brief 构造函数
     * @param name 效果名称
     */
    explicit GLPostProcessEffect(const std::string& name);

    /**
     * @brief 虚析构函数
     */
    virtual ~GLPostProcessEffect();

    /**
     * @brief 初始化效果
     * @return 是否初始化成功
     */
    virtual bool initialize() = 0;

    /**
     * @brief 应用效果
     * @param inputTexture 输入纹理
     * @param outputFramebuffer 输出帧缓冲
     * @param width 宽度
     * @param height 高度
     */
    virtual void apply(GLTexture* inputTexture, GLFrameBuffer* outputFramebuffer, int width, int height) = 0;

    /**
     * @brief 设置效果是否启用
     * @param enabled 是否启用
     */
    void setEnabled(bool enabled) { _enabled = enabled; }

    /**
     * @brief 获取效果是否启用
     * @return 是否启用
     */
    bool isEnabled() const { return _enabled; }

    /**
     * @brief 获取效果名称
     * @return 效果名称
     */
    const std::string& getName() const { return _name; }

    /**
     * @brief 设置效果强度
     * @param intensity 强度值（0.0 - 1.0）
     */
    virtual void setIntensity(float intensity) { _intensity = intensity; }

    /**
     * @brief 获取效果强度
     * @return 强度值
     */
    float getIntensity() const { return _intensity; }

protected:
    std::string _name;
    bool _enabled;
    float _intensity;
};

/**
 * @brief 模糊效果
 * 
 * 高斯模糊后期处理效果
 */
class GLBlurEffect : public GLPostProcessEffect
{
public:
    /**
     * @brief 构造函数
     */
    GLBlurEffect();

    /**
     * @brief 析构函数
     */
    ~GLBlurEffect() override;

    bool initialize() override;
    void apply(GLTexture* inputTexture, GLFrameBuffer* outputFramebuffer, int width, int height) override;

    /**
     * @brief 设置模糊半径
     * @param radius 模糊半径（像素）
     */
    void setRadius(float radius) { _radius = radius; }

    /**
     * @brief 获取模糊半径
     * @return 模糊半径
     */
    float getRadius() const { return _radius; }

    /**
     * @brief 设置模糊方向
     * @param horizontal 是否水平模糊
     */
    void setHorizontal(bool horizontal) { _horizontal = horizontal; }
private:
    GLShader* _shader;
    float _radius;
    bool _horizontal;
};

/**
 * @brief 泛光效果
 * 
 * 提取亮部并模糊产生光晕效果
 */
class GLBloomEffect : public GLPostProcessEffect
{
public:
    /**
     * @brief 构造函数
     */
    GLBloomEffect();

    /**
     * @brief 析构函数
     */
    ~GLBloomEffect() override;

    bool initialize() override;
    void apply(GLTexture* inputTexture, GLFrameBuffer* outputFramebuffer, int width, int height) override;

    /**
     * @brief 设置亮度阈值
     * @param threshold 阈值（0.0 - 1.0）
     */
    void setThreshold(float threshold) { _threshold = threshold; }

    /**
     * @brief 获取亮度阈值
     * @return 阈值
     */
    float getThreshold() const { return _threshold; }

private:
    GLShader* _extractShader;
    GLShader* _combineShader;
    GLBlurEffect* _blurEffect;
    GLFrameBuffer* _extractBuffer;
    GLFrameBuffer* _blurBuffer1;
    GLFrameBuffer* _blurBuffer2;
    float _threshold;
};

/**
 * @brief 色调映射效果
 * 
 * HDR到LDR的色调映射
 */
class GLToneMappingEffect : public GLPostProcessEffect
{
public:
    /**
     * @brief 构造函数
     */
    GLToneMappingEffect();

    /**
     * @brief 析构函数
     */
    ~GLToneMappingEffect() override;

    bool initialize() override;
    void apply(GLTexture* inputTexture, GLFrameBuffer* outputFramebuffer, int width, int height) override;

    /**
     * @brief 设置曝光值
     * @param exposure 曝光值
     */
    void setExposure(float exposure) { _exposure = exposure; }

    /**
     * @brief 获取曝光值
     * @return 曝光值
     */
    float getExposure() const { return _exposure; }

private:
    GLShader* _shader;
    float _exposure;
};

/**
 * @brief 颜色调整效果
 * 
 * 亮度、对比度、饱和度调整
 */
class GLColorAdjustEffect : public GLPostProcessEffect
{
public:
    /**
     * @brief 构造函数
     */
    GLColorAdjustEffect();

    /**
     * @brief 析构函数
     */
    ~GLColorAdjustEffect() override;

    bool initialize() override;
    void apply(GLTexture* inputTexture, GLFrameBuffer* outputFramebuffer, int width, int height) override;

    /**
     * @brief 设置亮度
     * @param brightness 亮度值（-1.0 - 1.0）
     */
    void setBrightness(float brightness) { _brightness = brightness; }

    /**
     * @brief 设置对比度
     * @param contrast 对比度值（0.0 - 2.0）
     */
    void setContrast(float contrast) { _contrast = contrast; }

    /**
     * @brief 设置饱和度
     * @param saturation 饱和度值（0.0 - 2.0）
     */
    void setSaturation(float saturation) { _saturation = saturation; }

    float getBrightness() const { return _brightness; }
    float getContrast() const { return _contrast; }
    float getSaturation() const { return _saturation; }

private:
    GLShader* _shader;
    float _brightness;
    float _contrast;
    float _saturation;
};

/**
 * @brief 后期处理管理器
 * 
 * 管理所有后期处理效果链
 */
class GLPostProcessManager
{
public:
    /**
     * @brief 获取单例实例
     */
    static GLPostProcessManager& getInstance();

    /**
     * @brief 初始化
     * @param width 宽度
     * @param height 高度
     * @return 是否初始化成功
     */
    bool initialize(int width, int height);

    /**
     * @brief 关闭
     */
    void shutdown();

    /**
     * @brief 调整大小
     * @param width 新宽度
     * @param height 新高度
     */
    void resize(int width, int height);

    /**
     * @brief 添加效果
     * @param effect 效果对象
     */
    void addEffect(GLPostProcessEffect* effect);

    /**
     * @brief 移除效果
     * @param name 效果名称
     */
    void removeEffect(const std::string& name);

    /**
     * @brief 获取效果
     * @param name 效果名称
     * @return 效果对象，未找到返回nullptr
     */
    GLPostProcessEffect* getEffect(const std::string& name);

    /**
     * @brief 清空所有效果
     */
    void clearEffects();

    /**
     * @brief 开始捕获场景
     */
    void beginCapture();

    /**
     * @brief 结束捕获并应用所有效果
     */
    void endCapture();

    /**
     * @brief 获取场景纹理
     * @return 场景纹理
     */
    GLTexture* getSceneTexture() const;

    /**
     * @brief 渲染纹理到屏幕
     * @param texture 要渲染的纹理
     */
    void renderToScreen(GLTexture* texture);

    /**
     * @brief 设置是否启用后期处理
     * @param enabled 是否启用
     */
    void setEnabled(bool enabled) { _enabled = enabled; }

    /**
     * @brief 是否启用后期处理
     * @return 是否启用
     */
    bool isEnabled() const { return _enabled; }

private:
    GLPostProcessManager() = default;
    ~GLPostProcessManager() = default;
    GLPostProcessManager(const GLPostProcessManager&) = delete;
    GLPostProcessManager& operator=(const GLPostProcessManager&) = delete;

private:
    std::vector<GLPostProcessEffect*> _effects;
    GLFrameBuffer* _sceneBuffer;
    GLFrameBuffer* _pingPongBuffer1;
    GLFrameBuffer* _pingPongBuffer2;
    int _width;
    int _height;
    bool _enabled;
    bool _capturing;
};

// 便捷宏定义
#define E2D_GL_POST_PROCESS easy2d::GLPostProcessManager::getInstance()

} // namespace easy2d
