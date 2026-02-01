-- Easy2D 项目 xmake 配置文件

-- 设置项目名称
set_project("Easy2D")

-- 设置项目版本
set_version("3.0.0")

-- 设置 C++ 标准
set_languages("c++17")

-- 添加依赖库
add_requires("miniaudio")
add_requires("libsdl2")
add_requires("glad")
add_requires("freetype")
add_requires("stb")
add_requires("zlib")
add_requires("glm")
add_requires("spdlog", {configs = {header_only = true}})

-- 添加静态库目标
target("easy2d")
    -- 设置目标类型为静态库
    set_kind("static")

    -- 设置库文件名 (与 easy2d.h 中的 #pragma comment 匹配)
    if is_mode("debug") then
        set_basename("libeasy2dd")
    else
        set_basename("libeasy2d")
    end

    -- 添加包含目录
    add_includedirs("Easy2D/include")

    -- 添加第三方库包
    add_packages("miniaudio")
    add_packages("libsdl2")
    add_packages("glad")
    add_packages("zlib")
    add_packages("freetype", {configs = {zlib = true}})
    add_packages("stb")
    add_packages("glm")
    add_packages("spdlog")

    -- 添加系统链接库
    if is_plat("windows") then
        -- Windows 平台链接 OpenGL
        add_syslinks("opengl32")
        -- Windows 平台链接其他系统库
        add_syslinks("user32", "gdi32", "shell32", "dinput8", "dxguid", "comdlg32")
    elseif is_plat("mingw") then
        -- MinGW 平台链接 OpenGL
        add_syslinks("opengl32")
        -- MinGW 平台链接其他系统库
        add_syslinks("dinput8", "dxguid", "comdlg32")
    else
        -- Linux 平台链接 OpenGL
        add_syslinks("GL")
        -- Linux 平台可能需要 X11
        add_syslinks("X11")
    end

    -- 添加源文件
    add_files("Easy2D/src/**.cpp")

    -- Debug 配置
    on_config(function (target)
        if is_mode("debug") then
            -- 启用调试信息
            target:set("symbols", "debug")
            -- 禁用优化
            target:set("optimize", "none")
        elseif is_mode("release") then
            -- 启用最大速度优化
            target:set("optimize", "fastest")
            -- 启用函数级链接
            target:set("policy", "build.optimization.lto", true)
        end
    end)

    -- 设置编译警告级别
    if is_plat("windows") then
        -- Windows MSVC 平台特定配置
        add_cxxflags("/W3", {force = true})
        -- 将警告视为错误
        -- add_cxxflags("/WX", {force = true})  -- 暂时禁用，避免迁移期间的编译错误
        -- 启用多处理器编译
        add_cxxflags("/MP", {force = true})
        -- 设置源文件和执行字符集为 UTF-8（使用 /utf-8 同时设置两者）
        add_cxxflags("/utf-8", {force = true})

    elseif is_plat("mingw") then
        -- MinGW 平台特定配置
        add_cxxflags("-Wall", "-Wextra", "-Wpedantic", {force = true})
        -- 设置源文件编码为 UTF-8
        add_cxxflags("-finput-charset=UTF-8", {force = true})
        -- 设置执行字符集为 UTF-8
        add_cxxflags("-fexec-charset=UTF-8", {force = true})
    else
        -- 其他平台配置
        add_cxxflags("-Wall", "-Wextra", "-Wpedantic", {force = true})
    end

    -- Release 模式下的额外配置
    if is_mode("release") then
        if is_plat("windows") then
            -- 禁用缓冲区安全检查
            add_cxxflags("/GS-", {force = true})
        end
    end


-- 包含演示程序
includes("GreedyMonster-Easy2D")
