if is_plat("linux") then
    add_requires("xcb", {system = true, configs = {shared = true}})
    add_requires("xcb-util", {system = true, configs = {shared = true}})
    add_requires("xcb-util-wm", {system = true, configs = {shared = true}})
    add_requires("xcb-util-errors", {system = false, configs = {shared = true}})
end

target("winclient")
    set_default(false)
    set_kind("static") --[[ TODO: set-kind: object?? ]]
    add_includedirs("$(projectdir)/include")
    add_files("./$(host)/**.cpp")
    -- add_files("common/**/*.cpp")
    if is_plat("linux") then
        add_packages("spdlog")
        add_packages("xcb", "xcb-util", "xcb-util-wm", "xcb-util-errors", "libassert")
    end
