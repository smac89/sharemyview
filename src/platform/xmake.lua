if is_plat("linux") then
    add_requires("xcb", {system = true, configs = {shared = true}})
    add_requires("xcb-util", {system = true, configs = {shared = true}})
    add_requires("xcb-util-wm", {system = true, configs = {shared = true}})
    add_requires("xcb-util-errors", {system = false, configs = {shared = true}})
    add_requires("xmake::stb 2023.12.15")
end

target("winclient")
    set_default(false)
    set_kind("static") --[[ TODO: set-kind: object?? ]]
    add_includedirs("$(projectdir)/include", "./internal")
    add_files("./$(host)/**.cpp", "./internal/**.cpp")
    -- add_files("common/**/*.cpp")
    add_packages("spdlog", "stb", "libassert")
    if is_plat("linux") then
        add_packages("xcb", "xcb-util", "xcb-util-wm", "xcb-util-errors")
    end
