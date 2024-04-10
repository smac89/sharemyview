set_xmakever("2.8.8")
set_warnings("allextra")
set_defaultmode("debug")
set_config("cc", "gcc")
set_config("cxx", "g++")
set_config("qt_sdkver", "5")
set_config("linkjobs", math.max(os.cpuinfo("ncpu") // 2, 2))
set_encodings("utf-8")
set_policy("package.requires_lock", true)

add_rules("plugin.compile_commands.autoupdate", {outputdir = "$(projectdir)/.vscode"})
add_repositories("myrepo pkgrepo")
add_rules("mode.debug", "mode.release")
add_requires("libsdl 2.30.x")
add_requires("spdlog 1.13.x", {system = false})
add_requires("hello-imgui 1.4.x", {
    configs = {use_glfw3 = true, has_opengl3 = true, imgui_version = "1.90.4-docking"},
    debug = is_mode("debug")
})
add_requires("opengl", {system = true})
add_cxflags("-mshstk", {tools = {"gcc", "clang"}})

if is_mode("debug") then
    set_symbols("debug")
    set_optimize("fast")
end

target("capture")
    set_default(true)
    set_kind("binary")
    -- https://github.com/xmake-io/xmake/blob/d8c1f612cdbd5c6f29a7b789d46f91292c1abc1e/xmake/rules/qt/xmake.lua
    add_rules("qt.quickapp")
    add_frameworks("QtQuickControls2", "QtWidgets", "QtQuickWidgets")
    set_languages("c17", "c++17")
    add_packages("spdlog", "opengl")
    -- include .hpp files so that the moc compiler can do it's thing
    add_files("src/main.cpp", "src/app/**.cpp", "src/app/**.hpp")
    -- include .qrc files to that they are transpiled by rcc
    add_files("capture.qrc")
    add_includedirs("include")
    add_cxflags("-fstack-protector-strong", {tools = {"gcc", "clang"}})
    if is_mode("debug") then
        add_defines("SMV_DEBUG")
    end
    if is_plat("linux") then
        includes("src/linux/xmake.lua")
        add_deps("libcapture")
    end

includes("examples/xmake.lua")
