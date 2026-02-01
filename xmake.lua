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
add_requires("miniaudio", "libsdl2", "glad", "freetype", "stb", "zlib", "glm", "simpleini")
add_requires("freetype", { configs = { zlib = true } })
add_requires("spdlog", { configs = { header_only = true } })

-- 添加静态库目标
target("easy2d")
    -- 设置目标类型为静态库
    -- 可选值: static (静态库), shared (动态库)
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
    add_packages("miniaudio", "libsdl2", "glad", "freetype", "stb", "zlib", "glm", "simpleini", "spdlog")

    -- Windows 平台特定库文件管理
    if is_plat("windows") then
        -- Windows MSVC 平台需要链接的系统库文件列表
        add_syslinks("user32", "gdi32", "shell32", "kernel32", "winmm")
    elseif is_plat("mingw") then
        -- MinGW 平台需要链接的系统库文件列表
        local libdirs = {
            "libcomdlg32.a",
            "libgdi32.a",
            "libole32.a",
            "libwinmm.a"
        }
        -- 遍历添加 MinGW 库文件
        for _, lib in ipairs(libdirs) do
            add_files("$(mingw)/x86_64-w64-mingw32/lib/" .. lib)
        end
    end

    -- 添加系统链接库 (跨平台)
    if is_plat("windows") then
        -- Windows MSVC 平台链接 OpenGL
        add_syslinks("opengl32")
        -- Windows 平台链接其他系统库
        add_syslinks("user32", "gdi32", "shell32", "kernel32", "winmm")
    elseif is_plat("mingw") then
        -- MinGW 平台链接 OpenGL
        add_syslinks("opengl32")
        -- MinGW 平台链接其他系统库
        add_syslinks("shell32", "kernel32", "winmm")
    elseif is_plat("linux") then
        -- Linux 平台链接 OpenGL
        add_syslinks("GL")
        -- Linux 平台需要 X11
        add_syslinks("X11")
        -- Linux 平台需要线程支持
        add_syslinks("pthread", "dl")
    elseif is_plat("macosx") then
        -- macOS 平台链接框架
        add_frameworks("OpenGL", "Cocoa", "IOKit", "CoreFoundation", "CoreVideo")
    elseif is_plat("android") then
        -- Android 平台链接库
        add_syslinks("GLESv2", "EGL", "android", "log")
    elseif is_plat("iphoneos") then
        -- iOS 平台链接框架
        add_frameworks("OpenGLES", "UIKit", "Foundation", "CoreGraphics", "QuartzCore")
    end

    -- 添加源文件
    add_files("Easy2D/src/**.cpp")

    -- 配置回调函数
    on_config(function(target)
        -- Debug 模式配置
        if is_mode("debug") then
            -- 启用调试信息
            target:set("symbols", "debug")
            -- 禁用优化
            target:set("optimize", "none")
            -- 定义 DEBUG 宏
            target:add("defines", "DEBUG", "_DEBUG")
        -- Release 模式配置
        elseif is_mode("release") then
            -- 启用速度优化（平衡大小和速度）
            target:set("optimize", "faster")
            -- 禁用 LTO 以减小库文件大小
            target:set("policy", "build.optimization.lto", false)
            -- 定义 NDEBUG 宏
            target:add("defines", "NDEBUG")
        end
    end)

    -- 设置编译警告级别 (跨平台)
    if is_plat("windows") then
        -- Windows MSVC 平台特定配置
        add_cxxflags("/W3", { force = true })
        -- 将警告视为错误 (可选，取消注释启用)
        -- add_cxxflags("/WX", {force = true})
        -- 启用多处理器编译
        add_cxxflags("/MP", { force = true })
        -- 设置源文件和执行字符集为 UTF-8
        add_cxxflags("/utf-8", { force = true })
        -- 启用异常处理
        add_cxxflags("/EHsc", { force = true })

    elseif is_plat("mingw") then
        -- MinGW 平台特定配置
        add_cxxflags("-Wall", "-Wextra", "-Wpedantic", { force = true })
        -- 设置源文件编码为 UTF-8
        add_cxxflags("-finput-charset=UTF-8", { force = true })
        -- 设置执行字符集为 UTF-8
        add_cxxflags("-fexec-charset=UTF-8", { force = true })
        -- 启用异常处理
        add_cxxflags("-fexceptions", { force = true })

    elseif is_plat("linux") then
        -- Linux 平台配置
        add_cxxflags("-Wall", "-Wextra", "-Wpedantic", { force = true })
        -- 启用异常处理
        add_cxxflags("-fexceptions", { force = true })
        -- 启用位置无关代码 (PIC)
        add_cxxflags("-fPIC", { force = true })

    elseif is_plat("macosx", "iphoneos") then
        -- macOS/iOS 平台配置
        add_cxxflags("-Wall", "-Wextra", { force = true })
        -- 启用异常处理
        add_cxxflags("-fexceptions", { force = true })
        -- 启用位置无关代码 (PIC)
        add_cxxflags("-fPIC", { force = true })

    elseif is_plat("android") then
        -- Android 平台配置
        add_cxxflags("-Wall", "-Wextra", { force = true })
        -- 启用异常处理
        add_cxxflags("-fexceptions", { force = true })
        -- 启用 RTTI
        add_cxxflags("-frtti", { force = true })
    end

    -- Release 模式下的额外配置
    if is_mode("release") then
        if is_plat("windows") then
            -- 禁用缓冲区安全检查（减小体积）
            add_cxxflags("/GS-", { force = true })
        elseif is_plat("mingw", "linux", "macosx") then
            -- 禁用 LTO 以减小库文件大小
            add_cxxflags("-fno-lto", { force = true })
        end
    end

-- 目标定义结束
target_end()

-- 示例程序
includes("GreedyMonster-Easy2D")
--