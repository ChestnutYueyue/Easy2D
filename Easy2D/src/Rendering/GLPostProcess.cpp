#include <easy2d/GLPostProcess.h>
#include <easy2d/e2dbase.h>
#include <easy2d/GLRenderer.h>
#include <easy2d/GLFullScreenQuad.h>

namespace easy2d
{

// 后期处理通用顶点着色器
static const char* POSTPROCESS_VERTEX_SHADER = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
)";

// 模糊片段着色器
static const char* BLUR_FRAGMENT_SHADER = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D uTexture;
uniform vec2 uDirection;
uniform vec2 uResolution;
uniform float uRadius;

void main()
{
    vec2 off1 = vec2(1.3846153846) * uDirection / uResolution;
    vec2 off2 = vec2(3.2307692308) * uDirection / uResolution;
    
    FragColor = texture(uTexture, TexCoord) * 0.2270270270;
    FragColor += texture(uTexture, TexCoord + off1 * uRadius) * 0.3162162162;
    FragColor += texture(uTexture, TexCoord - off1 * uRadius) * 0.3162162162;
    FragColor += texture(uTexture, TexCoord + off2 * uRadius) * 0.0702702703;
    FragColor += texture(uTexture, TexCoord - off2 * uRadius) * 0.0702702703;
}
)";

// 亮度提取片段着色器
static const char* EXTRACT_FRAGMENT_SHADER = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D uTexture;
uniform float uThreshold;

void main()
{
    vec4 color = texture(uTexture, TexCoord);
    float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    FragColor = brightness > uThreshold ? color : vec4(0.0);
}
)";

// 泛光合成片段着色器
static const char* BLOOM_COMBINE_FRAGMENT_SHADER = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D uSceneTexture;
uniform sampler2D uBloomTexture;
uniform float uIntensity;

void main()
{
    vec4 sceneColor = texture(uSceneTexture, TexCoord);
    vec4 bloomColor = texture(uBloomTexture, TexCoord);
    FragColor = sceneColor + bloomColor * uIntensity;
}
)";

// 色调映射片段着色器
static const char* TONEMAP_FRAGMENT_SHADER = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D uTexture;
uniform float uExposure;

vec3 acesToneMapping(vec3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main()
{
    vec4 color = texture(uTexture, TexCoord);
    vec3 mapped = acesToneMapping(color.rgb * uExposure);
    FragColor = vec4(mapped, color.a);
}
)";

// 颜色调整片段着色器
static const char* COLOR_ADJUST_FRAGMENT_SHADER = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D uTexture;
uniform float uBrightness;
uniform float uContrast;
uniform float uSaturation;

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{
    vec4 color = texture(uTexture, TexCoord);
    
    // 亮度调整
    color.rgb += uBrightness;
    
    // 对比度调整
    color.rgb = (color.rgb - 0.5) * uContrast + 0.5;
    
    // 饱和度调整
    vec3 hsv = rgb2hsv(color.rgb);
    hsv.y *= uSaturation;
    color.rgb = hsv2rgb(hsv);
    
    FragColor = color;
}
)";

// GLPostProcessEffect 实现

GLPostProcessEffect::GLPostProcessEffect(const std::string& name)
    : _name(name)
    , _enabled(true)
    , _intensity(1.0f)
{
}

GLPostProcessEffect::~GLPostProcessEffect()
{
}

// GLBlurEffect 实现
GLBlurEffect::GLBlurEffect()
    : GLPostProcessEffect("Blur")
    , _shader(nullptr)
    , _radius(2.0f)
    , _horizontal(true)
{
}

GLBlurEffect::~GLBlurEffect()
{
    delete _shader;
}

bool GLBlurEffect::initialize()
{
    _shader = new GLShader();
    if (!_shader->loadFromSource(POSTPROCESS_VERTEX_SHADER, BLUR_FRAGMENT_SHADER))
    {
        E2D_ERROR("Failed to initialize blur shader");
        return false;
    }
    
    return true;
}

void GLBlurEffect::apply(GLTexture* inputTexture, GLFrameBuffer* outputFramebuffer, int width, int height)
{
    if (!_shader || !inputTexture) return;
    
    outputFramebuffer->bind();
    
    _shader->use();
    _shader->setInt("uTexture", 0);
    _shader->setVec2("uDirection", _horizontal ? 1.0f : 0.0f, _horizontal ? 0.0f : 1.0f);
    _shader->setVec2("uResolution", static_cast<float>(width), static_cast<float>(height));
    _shader->setFloat("uRadius", _radius);
    
    inputTexture->bind(0);
    
    E2D_GL_FULLSCREEN_QUAD.render();
    
    outputFramebuffer->unbind();
}

// GLBloomEffect 实现

GLBloomEffect::GLBloomEffect()
    : GLPostProcessEffect("Bloom")
    , _extractShader(nullptr)
    , _combineShader(nullptr)
    , _blurEffect(nullptr)
    , _extractBuffer(nullptr)
    , _blurBuffer1(nullptr)
    , _blurBuffer2(nullptr)
    , _threshold(0.8f)
{
}

GLBloomEffect::~GLBloomEffect()
{
    delete _extractShader;
    delete _combineShader;
    delete _blurEffect;
    delete _extractBuffer;
    delete _blurBuffer1;
    delete _blurBuffer2;
}

bool GLBloomEffect::initialize()
{
    // 创建提取着色器
    _extractShader = new GLShader();
    if (!_extractShader->loadFromSource(POSTPROCESS_VERTEX_SHADER, EXTRACT_FRAGMENT_SHADER))
    {
        E2D_ERROR("Failed to initialize extract shader");
        return false;
    }

    // 创建合成着色器
    _combineShader = new GLShader();
    if (!_combineShader->loadFromSource(POSTPROCESS_VERTEX_SHADER, BLOOM_COMBINE_FRAGMENT_SHADER))
    {
        E2D_ERROR("Failed to initialize bloom combine shader");
        return false;
    }

    // 创建模糊效果
    _blurEffect = new GLBlurEffect();
    if (!_blurEffect->initialize())
    {
        E2D_ERROR("Failed to initialize blur effect");
        return false;
    }

    // 创建帧缓冲
    _extractBuffer = new GLFrameBuffer();
    _blurBuffer1 = new GLFrameBuffer();
    _blurBuffer2 = new GLFrameBuffer();

    return true;
}

void GLBloomEffect::apply(GLTexture* inputTexture, GLFrameBuffer* outputFramebuffer, int width, int height)
{
    if (!inputTexture) return;

    // 确保帧缓冲已创建
    if (!_extractBuffer->isValid())
    {
        _extractBuffer->create(width / 2, height / 2, false);
        _blurBuffer1->create(width / 2, height / 2, false);
        _blurBuffer2->create(width / 2, height / 2, false);
    }

    // 步骤1: 提取亮部
    _extractBuffer->bind();
    _extractShader->use();
    _extractShader->setInt("uTexture", 0);
    _extractShader->setFloat("uThreshold", _threshold);
    inputTexture->bind(0);
    
    // 渲染全屏四边形（这里简化处理，实际应该使用VAO）
    // TODO: 使用共享的全屏四边形VAO
    _extractBuffer->unbind();

    // 步骤2: 模糊亮部
    _blurEffect->setHorizontal(true);
    _blurEffect->apply(_extractBuffer->getColorTexture(), _blurBuffer1, width / 2, height / 2);
    
    _blurEffect->setHorizontal(false);
    _blurEffect->apply(_blurBuffer1->getColorTexture(), _blurBuffer2, width / 2, height / 2);

    // 步骤3: 合成
    outputFramebuffer->bind();
    _combineShader->use();
    _combineShader->setInt("uSceneTexture", 0);
    _combineShader->setInt("uBloomTexture", 1);
    _combineShader->setFloat("uIntensity", _intensity);
    
    inputTexture->bind(0);
    _blurBuffer2->getColorTexture()->bind(1);
    
    // 渲染全屏四边形
    outputFramebuffer->unbind();
}

// GLToneMappingEffect 实现

GLToneMappingEffect::GLToneMappingEffect()
    : GLPostProcessEffect("ToneMapping")
    , _shader(nullptr)
    , _exposure(1.0f)
{
}

GLToneMappingEffect::~GLToneMappingEffect()
{
    delete _shader;
}

bool GLToneMappingEffect::initialize()
{
    _shader = new GLShader();
    if (!_shader->loadFromSource(POSTPROCESS_VERTEX_SHADER, TONEMAP_FRAGMENT_SHADER))
    {
        E2D_ERROR("Failed to initialize tone mapping shader");
        return false;
    }
    return true;
}

void GLToneMappingEffect::apply(GLTexture* inputTexture, GLFrameBuffer* outputFramebuffer, int width, int height)
{
    if (!_shader || !inputTexture) return;

    outputFramebuffer->bind();
    
    _shader->use();
    _shader->setInt("uTexture", 0);
    _shader->setFloat("uExposure", _exposure);
    
    inputTexture->bind(0);
    
    // 渲染全屏四边形
    
    outputFramebuffer->unbind();
}

// GLColorAdjustEffect 实现

GLColorAdjustEffect::GLColorAdjustEffect()
    : GLPostProcessEffect("ColorAdjust")
    , _shader(nullptr)
    , _brightness(0.0f)
    , _contrast(1.0f)
    , _saturation(1.0f)
{
}

GLColorAdjustEffect::~GLColorAdjustEffect()
{
    delete _shader;
}

bool GLColorAdjustEffect::initialize()
{
    _shader = new GLShader();
    if (!_shader->loadFromSource(POSTPROCESS_VERTEX_SHADER, COLOR_ADJUST_FRAGMENT_SHADER))
    {
        E2D_ERROR("Failed to initialize color adjust shader");
        return false;
    }
    return true;
}

void GLColorAdjustEffect::apply(GLTexture* inputTexture, GLFrameBuffer* outputFramebuffer, int width, int height)
{
    if (!_shader || !inputTexture) return;

    outputFramebuffer->bind();
    
    _shader->use();
    _shader->setInt("uTexture", 0);
    _shader->setFloat("uBrightness", _brightness);
    _shader->setFloat("uContrast", _contrast);
    _shader->setFloat("uSaturation", _saturation);
    
    inputTexture->bind(0);
    
    // 渲染全屏四边形
    
    outputFramebuffer->unbind();
}

// GLPostProcessManager 实现

GLPostProcessManager& GLPostProcessManager::getInstance()
{
    static GLPostProcessManager instance;
    return instance;
}

bool GLPostProcessManager::initialize(int width, int height)
{
    _width = width;
    _height = height;
    _enabled = true;
    _capturing = false;

    // 创建场景缓冲
    _sceneBuffer = new GLFrameBuffer();
    if (!_sceneBuffer->create(width, height, false))
    {
        E2D_ERROR("Failed to create scene buffer");
        return false;
    }

    // 创建乒乓缓冲
    _pingPongBuffer1 = new GLFrameBuffer();
    _pingPongBuffer2 = new GLFrameBuffer();
    
    if (!_pingPongBuffer1->create(width, height, false) ||
        !_pingPongBuffer2->create(width, height, false))
    {
        E2D_ERROR("Failed to create ping-pong buffers");
        return false;
    }

    E2D_LOG("GLPostProcessManager initialized: %dx%d", width, height);
    return true;
}

void GLPostProcessManager::shutdown()
{
    clearEffects();
    
    delete _sceneBuffer;
    delete _pingPongBuffer1;
    delete _pingPongBuffer2;
    
    _sceneBuffer = nullptr;
    _pingPongBuffer1 = nullptr;
    _pingPongBuffer2 = nullptr;
}

void GLPostProcessManager::resize(int width, int height)
{
    _width = width;
    _height = height;

    if (_sceneBuffer) _sceneBuffer->resize(width, height);
    if (_pingPongBuffer1) _pingPongBuffer1->resize(width, height);
    if (_pingPongBuffer2) _pingPongBuffer2->resize(width, height);

    // 调整所有效果的内部缓冲
    for (auto* effect : _effects)
    {
        (void)effect;
    }
}

void GLPostProcessManager::addEffect(GLPostProcessEffect* effect)
{
    if (effect)
    {
        _effects.push_back(effect);
    }
}

void GLPostProcessManager::removeEffect(const std::string& name)
{
    for (auto it = _effects.begin(); it != _effects.end(); ++it)
    {
        if ((*it)->getName() == name)
        {
            delete *it;
            _effects.erase(it);
            return;
        }
    }
}

GLPostProcessEffect* GLPostProcessManager::getEffect(const std::string& name)
{
    for (auto* effect : _effects)
    {
        if (effect->getName() == name)
        {
            return effect;
        }
    }
    return nullptr;
}

void GLPostProcessManager::clearEffects()
{
    for (auto* effect : _effects)
    {
        delete effect;
    }
    _effects.clear();
}

void GLPostProcessManager::beginCapture()
{
    if (!_enabled || !_sceneBuffer) return;
    
    _capturing = true;
    _sceneBuffer->bind();
    _sceneBuffer->clear(0.0f, 0.0f, 0.0f, 0.0f);
}

void GLPostProcessManager::endCapture()
{
    if (!_capturing) return;
    
    _sceneBuffer->unbind();
    _capturing = false;

    if (!_enabled || _effects.empty())
    {
        // 直接渲染场景纹理到屏幕
        // TODO: 实现屏幕渲染
        return;
    }

    // 应用效果链
    GLTexture* input = _sceneBuffer->getColorTexture();
    GLFrameBuffer* output = _pingPongBuffer1;

    for (size_t i = 0; i < _effects.size(); ++i)
    {
        auto* effect = _effects[i];
        if (!effect->isEnabled()) continue;

        effect->apply(input, output, _width, _height);

        // 交换输入输出
        input = output->getColorTexture();
        output = (output == _pingPongBuffer1) ? _pingPongBuffer2 : _pingPongBuffer1;
    }

    // 最后输出到屏幕
    if (input)
    {
        renderToScreen(input);
    }
}

void GLPostProcessManager::renderToScreen(GLTexture* texture)
{
    if (!texture) return;
    
    // 解绑帧缓冲，渲染到默认帧缓冲（屏幕）
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, _width, _height);
    
    // 使用简单的全屏四边形着色器渲染
    GLShader* shader = E2D_GL_SHADER_MANAGER.getTextureShader();
    if (shader)
    {
        Matrix32 identityMatrix;
        identityMatrix.identity();
        
        shader->use();
        shader->setMat4("uProjection", identityMatrix);
        shader->setMat4("uModel", identityMatrix);
        shader->setFloat("uOpacity", 1.0f);
        shader->setBool("uUseTexture", true);
        shader->setInt("uTexture", 0);
        
        texture->bind(0);
        E2D_GL_FULLSCREEN_QUAD.render();
    }
}

GLTexture* GLPostProcessManager::getSceneTexture() const
{
    return _sceneBuffer ? _sceneBuffer->getColorTexture() : nullptr;
}

} // namespace easy2d
