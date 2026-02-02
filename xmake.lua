set_project("Easy2D")
set_version("3.0.0")
set_languages("c++17")
add_rules("mode.debug", "mode.release")

add_requires("libsdl3")
add_requires("glad")
add_requires("stb")
add_requires("zlib")
add_requires("glm")
add_requires("simpleini")
add_requires("miniaudio")
add_requires("freetype", { configs = { zlib = true } })
add_requires("spdlog", { configs = { header_only = true } })

target("easy2d")
    set_kind("static")
    set_basename("libeasy2d")
    add_files("Easy2D/src/**.cpp")
    add_headerfiles("Easy2D/include/**.h")
    add_includedirs("Easy2D/include", {public = true})
    add_packages(
        "libsdl3",
        "glad",
        "stb",
        "zlib",
        "glm",
        "simpleini",
        "miniaudio",
        "freetype",
        "spdlog",
        {public = true}
    )

    if is_plat("windows") then
        add_defines("WIN32_LEAN_AND_MEAN")
        add_syslinks("opengl32", "user32", "gdi32", "shell32", "winmm", "imm32", "version", "ole32")

        local toolchain = get_config("toolchain") or "msvc"
        if toolchain == "msvc" or toolchain == "clang-cl" then
            add_cxxflags("/EHsc", "/Zc:__cplusplus", "/utf-8", {force = true})
        else
            add_cxxflags("-fexceptions", "-frtti", "-finput-charset=UTF-8", "-fexec-charset=UTF-8")
        end

        if toolchain == "msvc" or toolchain == "clang-cl" then
            if is_mode("debug") then
                set_runtimes("MDd")
            else
                set_runtimes("MD")
            end
        end
    else 
        add_cxxflags("-finput-charset=utf-8", "-fexec-charset=utf-8")
    end

target("GreedyMonster")
    set_kind("binary")
    add_deps("easy2d")
    add_files("GreedyMonster-Easy2D/src/**.cpp")
    add_includedirs("GreedyMonster-Easy2D/src")
    set_rundir("GreedyMonster-Easy2D/src")

    if is_plat("windows") then
        add_cxxflags("/utf-8")
        add_rules("win.sdk.resource")
        add_files("GreedyMonster-Easy2D/src/*.rc")
    end
