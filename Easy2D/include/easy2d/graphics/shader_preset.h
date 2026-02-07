#pragma once

#include <easy2d/core/types.h>
#include <easy2d/core/color.h>
#include <easy2d/graphics/opengl/gl_shader.h>
#include <string>
#include <vector>

namespace easy2d {

// ============================================================================
// Shader预设参数
// ============================================================================

/**
 * @brief 水波纹效果参数
 */
struct WaterParams {
    float waveSpeed = 2.0f;      // 波动速度
    float waveAmplitude = 0.02f; // 波动幅度
    float waveFrequency = 10.0f; // 波动频率
};

/**
 * @brief 描边效果参数
 */
struct OutlineParams {
    Color color = Colors::Red;   // 描边颜色
    float thickness = 2.0f;      // 描边厚度
    bool inner = false;          // 是否内描边
};

/**
 * @brief 扭曲效果参数
 */
struct DistortionParams {
    float strength = 0.1f;       // 扭曲强度
    float speed = 1.0f;          // 扭曲速度
    float scale = 10.0f;         // 扭曲缩放
};

/**
 * @brief 像素化效果参数
 */
struct PixelateParams {
    float pixelSize = 8.0f;      // 像素大小
};

/**
 * @brief 反相效果参数
 */
struct InvertParams {
    float strength = 1.0f;       // 反相强度 (0-1)
};

/**
 * @brief 灰度效果参数
 */
struct GrayscaleParams {
    float intensity = 1.0f;      // 灰度强度 (0-1)
};

/**
 * @brief 复古效果参数
 */
struct SepiaParams {
    float intensity = 1.0f;      // 复古强度 (0-1)
};

// ============================================================================
// Shader预设系统
// ============================================================================
class ShaderPreset {
public:
    // ------------------------------------------------------------------------
    // 水波纹效果
    // ------------------------------------------------------------------------
    static Ptr<GLShader> Water(const WaterParams& params = WaterParams{});
    
    // ------------------------------------------------------------------------
    // 描边效果
    // ------------------------------------------------------------------------
    static Ptr<GLShader> Outline(const OutlineParams& params = OutlineParams{});
    
    // ------------------------------------------------------------------------
    // 扭曲效果
    // ------------------------------------------------------------------------
    static Ptr<GLShader> Distortion(const DistortionParams& params = DistortionParams{});
    
    // ------------------------------------------------------------------------
    // 像素化效果
    // ------------------------------------------------------------------------
    static Ptr<GLShader> Pixelate(const PixelateParams& params = PixelateParams{});
    
    // ------------------------------------------------------------------------
    // 反相效果
    // ------------------------------------------------------------------------
    static Ptr<GLShader> Invert(const InvertParams& params = InvertParams{});
    
    // ------------------------------------------------------------------------
    // 灰度效果
    // ------------------------------------------------------------------------
    static Ptr<GLShader> Grayscale(const GrayscaleParams& params = GrayscaleParams{});
    
    // ------------------------------------------------------------------------
    // 复古效果
    // ------------------------------------------------------------------------
    static Ptr<GLShader> Sepia(const SepiaParams& params = SepiaParams{});
    
    // ------------------------------------------------------------------------
    // 组合效果
    // ------------------------------------------------------------------------
    
    /**
     * @brief 创建组合效果（灰度 + 描边）
     */
    static Ptr<GLShader> GrayscaleOutline(const GrayscaleParams& grayParams = GrayscaleParams{},
                                          const OutlineParams& outlineParams = OutlineParams{});
    
    /**
     * @brief 创建组合效果（像素化 + 反相）
     */
    static Ptr<GLShader> PixelateInvert(const PixelateParams& pixParams = PixelateParams{},
                                        const InvertParams& invParams = InvertParams{});

    // ------------------------------------------------------------------------
    // 工具方法
    // ------------------------------------------------------------------------
    
    /**
     * @brief 获取标准顶点着色器
     */
    static const char* GetStandardVertexShader();
    
    /**
     * @brief 获取带UV的标准顶点着色器
     */
    static const char* GetStandardVertexShaderWithUV();
};

// ============================================================================
// 预设Shader源码
// ============================================================================
namespace ShaderSource {

// 标准顶点着色器
inline const char* StandardVert = R"(
#version 330 core
layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 a_texCoord;

uniform mat4 u_viewProjection;
uniform mat4 u_model;

out vec2 v_texCoord;

void main() {
    gl_Position = u_viewProjection * u_model * vec4(a_position, 0.0, 1.0);
    v_texCoord = a_texCoord;
}
)";

// 水波纹效果片段着色器
inline const char* WaterFrag = R"(
#version 330 core
in vec2 v_texCoord;

uniform sampler2D u_texture;
uniform float u_time;
uniform float u_waveSpeed;
uniform float u_waveAmplitude;
uniform float u_waveFrequency;

out vec4 fragColor;

void main() {
    vec2 uv = v_texCoord;
    
    // 计算波纹偏移
    float wave = sin(uv.y * u_waveFrequency + u_time * u_waveSpeed) * u_waveAmplitude;
    uv.x += wave;
    
    fragColor = texture(u_texture, uv);
}
)";

// 描边效果片段着色器
inline const char* OutlineFrag = R"(
#version 330 core
in vec2 v_texCoord;

uniform sampler2D u_texture;
uniform vec4 u_outlineColor;
uniform float u_thickness;
uniform vec2 u_textureSize;

out vec4 fragColor;

void main() {
    vec4 color = texture(u_texture, v_texCoord);
    
    // 计算周围像素的不透明度
    float alpha = 0.0;
    vec2 offset = u_thickness / u_textureSize;
    
    alpha += texture(u_texture, v_texCoord + vec2(-offset.x, 0.0)).a;
    alpha += texture(u_texture, v_texCoord + vec2(offset.x, 0.0)).a;
    alpha += texture(u_texture, v_texCoord + vec2(0.0, -offset.y)).a;
    alpha += texture(u_texture, v_texCoord + vec2(0.0, offset.y)).a;
    
    // 如果当前像素透明但周围不透明，则显示描边
    if (color.a < 0.1 && alpha > 0.0) {
        fragColor = u_outlineColor;
    } else {
        fragColor = color;
    }
}
)";

// 扭曲效果片段着色器
inline const char* DistortionFrag = R"(
#version 330 core
in vec2 v_texCoord;

uniform sampler2D u_texture;
uniform float u_time;
uniform float u_strength;
uniform float u_speed;
uniform float u_scale;

out vec4 fragColor;

void main() {
    vec2 uv = v_texCoord;
    
    // 计算扭曲偏移
    float distortion = sin(uv.y * u_scale + u_time * u_speed) * u_strength;
    uv.x += distortion;
    distortion = cos(uv.x * u_scale + u_time * u_speed) * u_strength;
    uv.y += distortion;
    
    fragColor = texture(u_texture, uv);
}
)";

// 像素化效果片段着色器
inline const char* PixelateFrag = R"(
#version 330 core
in vec2 v_texCoord;

uniform sampler2D u_texture;
uniform float u_pixelSize;
uniform vec2 u_textureSize;

out vec4 fragColor;

void main() {
    vec2 pixel = u_pixelSize / u_textureSize;
    vec2 uv = floor(v_texCoord / pixel) * pixel + pixel * 0.5;
    
    fragColor = texture(u_texture, uv);
}
)";

// 反相效果片段着色器
inline const char* InvertFrag = R"(
#version 330 core
in vec2 v_texCoord;

uniform sampler2D u_texture;
uniform float u_strength;

out vec4 fragColor;

void main() {
    vec4 color = texture(u_texture, v_texCoord);
    vec3 inverted = 1.0 - color.rgb;
    color.rgb = mix(color.rgb, inverted, u_strength);
    
    fragColor = color;
}
)";

// 灰度效果片段着色器
inline const char* GrayscaleFrag = R"(
#version 330 core
in vec2 v_texCoord;

uniform sampler2D u_texture;
uniform float u_intensity;

out vec4 fragColor;

void main() {
    vec4 color = texture(u_texture, v_texCoord);
    float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));
    color.rgb = mix(color.rgb, vec3(gray), u_intensity);
    
    fragColor = color;
}
)";

// 复古效果片段着色器
inline const char* SepiaFrag = R"(
#version 330 core
in vec2 v_texCoord;

uniform sampler2D u_texture;
uniform float u_intensity;

out vec4 fragColor;

void main() {
    vec4 color = texture(u_texture, v_texCoord);
    
    vec3 sepia = vec3(
        dot(color.rgb, vec3(0.393, 0.769, 0.189)),
        dot(color.rgb, vec3(0.349, 0.686, 0.168)),
        dot(color.rgb, vec3(0.272, 0.534, 0.131))
    );
    
    color.rgb = mix(color.rgb, sepia, u_intensity);
    
    fragColor = color;
}
)";

} // namespace ShaderSource

} // namespace easy2d
