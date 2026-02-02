-- Easy2D 项目 xmake 配置文件

-- 设置项目名称
set_project("Easy2D")

-- 设置项目版本
set_version("3.0.0")

-- 设置 C++ 标准
set_languages("c++17")

-- 添加编译模式规则
add_rules("mode.debug", "mode.release")

-- 添加依赖库
add_requires("miniaudio", "glad", "freetype", "stb", "zlib", "glm", "simpleini")
-- 使用 SDL3 替代 SDL2
add_requires("libsdl3")
add_requires("freetype", { configs = { zlib = true } })
add_requires("spdlog", { configs = { header_only = true } })



--