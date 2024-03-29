set_xmakever("2.8.8")
set_warnings("allextra")
set_defaultmode("debug")
set_config("cc", "clang")
set_config("cxx", "clang++")
set_policy("package.requires_lock", true)

add_repositories("myrepo pkgrepo")
add_rules("mode.debug", "mode.release")
add_requires("libsdl 2.30.x")
add_requires("spdlog 1.13.x", { alias = "spdlog" })
-- add_requires("glfw 3.x", {system = true})
-- add_requires("vcpkg::imgui 1.90.x", { alias = "imgui"})
-- add_requires("vcpkg::hello-imgui 1.4.x", { alias = "hello_imgui" })
-- add_requires("vcpkg::hello-imgui[sdl2-binding]")
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
    add_packages("libsdl", "spdlog")
    if is_plat("linux") then
        includes("src/linux/xmake.lua")
        add_deps("libcapture")
    end

target("sdlapp")
    set_kind("binary")
    set_languages("c17", "c++17")
    add_files("example/sdl.cpp")
    add_defines("HELLOIMGUI_USE_SDL2", "HELLOIMGUI_HAS_OPENGL3")
    add_packages("libsdl", "spdlog", "hello_imgui", "imgui")

-- target("qtapp")
--     add_rules("qt.widgetapp")
--     set_kind("binary")
--     set_languages("c17", "c++17")
--     add_files("example/qt.cpp")
--     add_frameworks("QtGui")
