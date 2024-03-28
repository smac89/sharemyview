set_xmakever("2.8.8")
set_warnings("allextra")
set_defaultmode("debug")
set_config("cc", "clang")
set_config("cxx", "clang++")

add_rules("mode.debug", "mode.release")
add_requires("libsdl ~2.30.*")
add_requires("conan::spdlog 1.13.0", { alias = "spdlog" })
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
