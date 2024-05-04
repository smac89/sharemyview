set_xmakever("2.8.8")
set_warnings("allextra")
set_defaultmode("debug")
set_config("cxx", "g++")
set_config("qt_sdkver", "5")
set_config("debugger", "lldb")
set_config("linkjobs", math.max(os.cpuinfo("ncpu") // 2, 2))
set_encodings("utf-8")
set_policy("package.requires_lock", true)
set_policy("build.warning", true)

add_rules("plugin.compile_commands.autoupdate", {outputdir = "$(projectdir)/.vscode"})
add_repositories("myrepo pkgrepo")
add_rules("mode.debug", "mode.release")
add_requires("spdlog 1.13.x", {system = false})
add_requires("libassert 1.2.2", { configs = { decompose = true} })
add_requires("qml-colors 1.0")

if is_mode("debug") then
    -- https://doc.qt.io/qt-5/qtquick-debugging.html#debugging-module-imports
    -- set_runenv("QML_IMPORT_TRACE", "1")
    -- add_runenvs("QT_DEBUG_PLUGINS", "1")
end

target("capture")
    set_default(true)
    set_kind("binary")
    -- https://github.com/xmake-io/xmake/blob/d8c1f612cdbd5c6f29a7b789d46f91292c1abc1e/xmake/rules/qt/xmake.lua
    add_rules("qt.quickapp")
    add_frameworks("QtQuickControls2", "QtWidgets", "QtSvg", "QtQuickWidgets")
    set_languages("c++17")
    add_packages("spdlog", "libassert", "qml-colors")
    -- include .hpp files so that the moc compiler can do it's thing
    add_files("src/main.cpp", "src/app/**.cpp", "src/app/**.hpp")
    -- include .qrc files to that they are transpiled by rcc
    add_files("qml/main.qrc")
    add_includedirs("include", {public = true})
    -- add_cxflags("-fstack-protector-strong", "-mshstk", {tools = {"g++", "clang"}})
    add_runenvs("SPDLOG_LEVEL", "=info,smv::winclient=off,smv::autocancel=off")
    includes("src/platform")
    add_deps("winclient")

includes("examples")
