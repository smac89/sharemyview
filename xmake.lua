set_xmakever("2.8.8")
set_warnings("allextra")
set_defaultmode("debug")
set_config("cc", "clang")
set_config("cxx", "clang++")
set_policy("package.requires_lock", true)

add_repositories("myrepo pkgrepo")
add_rules("plugin.compile_commands.autoupdate", {outputdir = "$(projectdir)/.vscode"})
add_rules("mode.debug", "mode.release")
add_requires("libsdl 2.30.x", "vc 1.4.x")
add_requires("spdlog 1.13.x", {alias = "spdlog"})
add_requires("hello-imgui 1.4.x", {
    configs = {use_sdl2 = true, has_opengl3 = true, imgui_version = "1.90.4-docking"},
    debug = is_mode("debug")
})
add_cxflags("-mshstk", {tools = {"gcc", "clang"}})

if is_mode("debug") then
    set_symbols("debug")
    set_optimize("fast")
end

target("capture")
    set_kind("binary")
    set_languages("c17", "c++17")
    add_files("src/main.cpp")
    add_includedirs("include")
    add_cxflags("-fstack-protector-strong", {tools = {"gcc", "clang"}})
    add_packages("libsdl", "spdlog", "hello-imgui")
    if is_plat("linux") then
        includes("src/linux/xmake.lua")
        add_deps("libcapture")
    end

target("sdlapp")
    set_kind("binary")
    set_languages("c17", "c++17")
    add_files("example/sdl.cpp")
    add_packages("libsdl", "spdlog", "hello-imgui")
