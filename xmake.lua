-- ==============================================
-- Easy2D 静态库 Xmake 构建脚本（Debug 库名加 d 后缀版）
-- 功能：Debug 版生成 libeasy2dd.lib/a，Release 版生成 libeasy2d.lib/a，完善调试体验
-- 优化：MinGW 平台自动去掉 lib 前缀，避免生成 liblibeasy2d.a 重复前缀
-- ==============================================

-- 1. 项目基础配置（全局）
set_project("Easy2D")
set_version("2.1.27")
set_languages("c++17")
add_rules("mode.debug", "mode.release")

-- 定义项目根目录下的核心路径（避免硬编码，提升可维护性）
local EASY2D_SRC_DIR = "Easy2D/src"
local EASY2D_INC_DIR = "Easy2D/include"

-- 2. 目标静态库配置（核心）
target("easy2d")
    -- 设置目标产物为静态库
    set_kind("static")

    -- ==============================================
    -- 优化：按平台+构建模式动态设置库文件名（解决 MinGW 重复 lib 前缀问题）
    -- ==============================================
    local lib_basename
    -- 判断是否为 MinGW 平台（MinGW 会自动给 .a 文件添加 lib 前缀，无需手动指定）
    if is_plat("mingw") then
        -- MinGW 平台：基础名称不带 lib 前缀
        lib_basename = "easy2d"
    else
        -- 其他平台（MSVC/Clang-Cl 等）：保留 lib 前缀
        lib_basename = "libeasy2d"
    end

    -- 按构建模式追加 d 后缀（保持原有调试版命名规则）
    if is_mode("debug") then
        -- Debug 模式：追加 d 后缀
        -- MinGW: easy2dd -> 最终生成 libeasy2dd.a
        -- 其他平台: libeasy2dd -> 最终生成 libeasy2dd.lib
        set_basename(lib_basename .. "d")
    else
        -- Release 模式：保持基础名称
        -- MinGW: easy2d -> 最终生成 libeasy2d.a
        -- 其他平台: libeasy2d -> 最终生成 libeasy2d.lib
        set_basename(lib_basename)
    end

    -- 收集所有 C++ 源码文件（递归匹配 src 目录下所有 .cpp）
    add_files(path.join(EASY2D_SRC_DIR, "**.cpp"))

    -- 声明头文件（用于 xmake install 安装，保留 easy2d 目录层级）
    add_headerfiles(path.join(EASY2D_INC_DIR, "easy2d/**.h"), {prefixdir = "easy2d"})
    add_headerfiles(path.join(EASY2D_INC_DIR, "**.h"), {exclude = path.join(EASY2D_INC_DIR, "easy2d/**.h"), prefixdir = ""})

    -- 公开头文件目录（其他依赖该库的目标会自动继承这个头文件路径）
    add_includedirs(EASY2D_INC_DIR, {public = true})

    -- ==============================================
    -- 全局 Debug/Release 模式通用配置（所有编译器生效）
    -- ==============================================
    if is_mode("debug") then
        -- 生成完整调试符号（核心：方便断点调试、查看变量、调用栈）
        set_symbols("debug")
        -- 禁止代码优化（O0 级别，避免编译器修改代码逻辑，保证调试和源码一致）
        set_optimize("none")
        -- 定义 Debug 专属宏（项目源码中可通过 #ifdef EASY2D_DEBUG 区分调试模式）
        add_defines("EASY2D_DEBUG", "_DEBUG", {public = true})
    -- Release 模式：侧重运行效率
    elseif is_mode("release") then
        -- 生成精简调试符号（可选，方便线上问题排查，不影响运行效率）
        set_symbols("hidden")
        -- 开启最高级别优化（O3 级别，提升运行速度和减小文件体积）
        set_optimize("fastest")
        -- 定义 Release 专属宏
        add_defines("EASY2D_RELEASE", "NDEBUG", {public = true})
        -- 开启代码混淆（可选，防止反编译，不影响功能）
        set_strip("all")
    end

    -- ==============================================
    -- Windows 平台通用配置（包含 MSVC/Clang-Cl/MinGW）
    -- ==============================================
    if is_plat("windows") then
        -- 定义宏，减少 Windows 头文件的冗余定义和编译时间
        add_defines("WIN32_LEAN_AND_MEAN", "NOMINMAX")

        -- Windows 平台必需的系统库（通用，Xmake 会自动适配编译器格式）
        local win_sys_libs = {
            "user32", "gdi32", "shell32", "winmm", 
            "imm32", "version", "ole32", "comdlg32", 
            "dinput8", "d2d1", "dwrite", "dxguid"
        }
        add_syslinks(win_sys_libs)

        -- ==============================================
        -- MSVC / Clang-Cl 编译器专属配置
        -- ==============================================
        local toolchain = get_config("toolchain") or "msvc"
        if toolchain == "msvc" or toolchain == "clang-cl" then
            -- 强制设置 C++ 编译选项（所有模式生效）
            add_cxxflags("/EHsc", "/Zc:__cplusplus", "/utf-8", {force = true})
            -- 禁用不安全函数警告（避免 Windows 头文件报 C4996 错误）
            add_cxxflags("/wd4996", {force = true})

            -- 按构建模式配置运行时库
            if is_mode("debug") then
                set_runtimes("MDd")
                add_cxxflags("/Od", "/Zi", {force = true})
            else
                set_runtimes("MD")
                add_cxxflags("/O2", "/Ob2", {force = true})
            end
        end

        -- ==============================================
        -- MinGW 编译器专属配置
        -- ==============================================
        -- 使用 is_plat("mingw") 判断 MinGW 环境
        if is_plat("mingw") then

            -- 开启更多警告，帮助发现潜在问题
            add_cxxflags("-Wall", "-Wextra", "-Wpedantic", {force = true})
            -- 禁用部分无意义的警告
            add_cxxflags("-Wno-unused-parameter", "-Wno-missing-field-initializers", {force = true})

            -- 配置 UTF-8 编码
            add_cxxflags("-finput-charset=UTF-8", "-fexec-charset=UTF-8", {force = true})

            -- 启用异常处理和 RTTI
            add_cxxflags("-fexceptions", "-frtti", {force = true})

            -- 补充 MinGW 缺失的系统库链接
            local mingw_extra_libs = {"windowscodecs", "mfplat", "mfreadwrite", "shlwapi", "xaudio2_8"}
            add_syslinks(mingw_extra_libs)

            -- MinGW 按构建模式配置专属选项
            if is_mode("debug") then
                add_cxxflags("-O0", "-g", "-ggdb", {force = true})
                set_runtimes("MDd")
            else
                add_cxxflags("-O3", "-fomit-frame-pointer", {force = true})
                set_runtimes("MD")
            end
        end
    end