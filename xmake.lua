-- ==============================================
-- Easy2D 静态库 Xmake 构建脚本（Debug 库名加 d 后缀版）
-- 功能：Debug 版生成 libeasy2dd.lib/a，Release 版生成 libeasy2d.lib/a，完善调试体验
-- 优化：MinGW 平台自动去掉 lib 前缀，避免生成 liblibeasy2d.a 重复前缀
-- ==============================================

-- 1. 项目基础配置（全局）
set_project("Easy2D")
set_version("2.1.27")
set_languages("c++17")
set_encodings("utf-8")
add_rules("mode.debug", "mode.release")

-- 定义项目根目录下的核心路径（避免硬编码，提升可维护性）
local EASY2D_SRC_DIR = "Easy2D/src"
local EASY2D_INC_DIR = "Easy2D/include"

-- ==============================================
-- 2. 目标静态库配置（核心）
-- ==============================================
target("easy2d")
    -- 设置目标产物为静态库
    set_kind("static")

    -- ==============================================
    -- 优化：按平台+构建模式动态设置库文件名（解决 MinGW 重复 lib 前缀问题）
    -- ==============================================
    local lib_basename
    -- 判断是否为 MinGW 平台（MinGW 会自动给 .a 文件添加 lib 前缀，无需手动指定）
    if is_plat("mingw") then
        lib_basename = "easy2d"
    else
        lib_basename = "libeasy2d"
    end

    -- 按构建模式追加 d 后缀（保持原有调试版命名规则）
    if is_mode("debug") then
        set_basename(lib_basename .. "d")
    else
        set_basename(lib_basename)
    end

    add_files(path.join(EASY2D_SRC_DIR, "**.cpp"))

    -- 声明头文件（用于 xmake install 安装，保留 easy2d 目录层级）
    add_headerfiles(path.join(EASY2D_INC_DIR, "easy2d/**.h"), {prefixdir = "easy2d"})
    add_headerfiles(path.join(EASY2D_INC_DIR, "spdlog/**.h"), {prefixdir = "spdlog"})

    -- 公开头文件目录（其他依赖该库的目标会自动继承这个头文件路径）
    add_includedirs(EASY2D_INC_DIR, {public = true})

    -- ==============================================
    -- Windows 平台通用配置（包含 MSVC/Clang-Cl/MinGW）
    -- ==============================================
    if is_plat("windows") then
        -- 定义宏，减少 Windows 头文件的冗余定义和编译时间
        add_defines("WIN32_LEAN_AND_MEAN", "NOMINMAX")

        -- 添加 Windows 系统库链接
        add_syslinks("user32", "gdi32", "shell32", "winmm", "imm32", "version", "ole32", "comdlg32", "dinput8", "d2d1", "dwrite", "dxguid", "oleaut32", "uuid")

        -- MSVC / Clang-Cl 工具链配置
        if get_config("toolchain") == "msvc" or get_config("toolchain") == "clang-cl" then
            add_cxxflags("/EHsc", "/Zc:__cplusplus", {force = true})
            add_cxxflags("/wd4996", {force = true})

            if is_mode("debug") then
                set_runtimes("MDd")
                add_defines("EASY2D_DEBUG", "_DEBUG", {public = true})
                add_cxxflags("/Od", "/Zi", {force = true})
            else
                set_runtimes("MD")
                add_defines("EASY2D_RELEASE", "NDEBUG", {public = true})
                add_cxxflags("/O2", "/Ob2", {force = true})
            end
        end
    end
    -- MinGW 平台配置
    if is_plat("mingw") then
            add_cxxflags("-Wall", "-Wextra", "-Wpedantic", {force = true})
            add_cxxflags("-Wno-unused-parameter", "-Wno-missing-field-initializers", {force = true})

        if is_mode("debug") then
            add_defines("EASY2D_DEBUG", "_DEBUG", {public = true})
            add_cxxflags("-O0", "-g", "-ggdb", {force = true})
            set_runtimes("MDd")
        else
            add_defines("EASY2D_RELEASE", "NDEBUG", {public = true})
            add_cxxflags("-O2", "-fno-strict-aliasing", "-fno-delete-null-pointer-checks", {force = true})
            set_runtimes("MD")
        end
    end
target_end()

-- ==============================================
-- 3. PushBox 示例程序配置
-- ==============================================
target("PushBox")
    -- 设置目标产物为可执行文件
    set_kind("binary")

    -- 设置输出目录
    set_targetdir("$(builddir)/bin")

    -- PushBox 源代码路径
    local PUSHBOX_DIR = "PushBox-Easy2D-v2.1.12/src"

    -- 添加源文件
    add_files(path.join(PUSHBOX_DIR, "**.cpp"))

    -- 添加 Windows 资源文件（仅 Windows 平台）
    if is_plat("windows") then
        add_files(path.join(PUSHBOX_DIR, "resources/PushBox.rc"))
    end

    -- 添加头文件搜索路径
    add_includedirs(PUSHBOX_DIR)
    add_includedirs(EASY2D_INC_DIR)

    -- 链接 Easy2D 库
    add_deps("easy2d")
    
    add_syslinks("user32", "gdi32", "shell32", "winmm", "imm32", "version", "ole32", "comdlg32", "dinput8", "d2d1", "dwrite", "dxguid", "oleaut32", "uuid")

    -- 设置 Windows 子系统为 Windows 应用程序（不显示控制台窗口）
    if is_plat("windows") then
        -- MSVC / Clang-Cl 工具链配置
        if get_config("toolchain") == "msvc" or get_config("toolchain") == "clang-cl" then
            add_cxxflags("/EHsc", "/Zc:__cplusplus", {force = true})
            add_cxxflags("/wd4996", {force = true})

            if is_mode("debug") then
                set_runtimes("MDd")
                add_cxxflags("/Od", "/Zi", {force = true})
            else
                add_ldflags("/SUBSYSTEM:WINDOWS", {force = true})
                set_runtimes("MD")
                add_cxxflags("/O2", "/Ob2", {force = true})
            end
        end
    end
        -- MinGW 平台配置
    if is_plat("mingw") then
        add_cxxflags("-Wall", "-Wextra", {force = true})
        add_cxxflags("-Wno-unused-parameter", {force = true})

        if is_mode("debug") then
            add_cxxflags("-O0", "-g", "-ggdb", {force = true})
            set_runtimes("MDd")
        else
            -- MinGW 需要显式指定 Windows 子系统
            add_ldflags("-mwindows", {force = true})
            add_cxxflags("-O2", {force = true})
            set_runtimes("MD")
        end
     end
    -- 复制资源文件到输出目录（构建后执行）
    after_build(function (target)
        local assets_dir = path.join(PUSHBOX_DIR, "assets")
        local target_dir = target:targetdir()

        -- 复制 assets 目录到输出目录
        if os.isdir(assets_dir) then
            os.cp(assets_dir, target_dir)
            print("Copied assets to: " .. target_dir)
        end
    end)
target_end()
