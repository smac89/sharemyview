package("hello-imgui")
    set_homepage("https://github.com/pthom/hello_imgui")
    set_description("Hello, Dear ImGui: unleash your creativity in app development and prototyping")
    set_license("MIT")
    set_policy("package.strict_compatibility", true)

    add_urls("https://github.com/pthom/hello_imgui/archive/refs/tags/$(version).tar.gz")
    add_versions("v1.4.2", "1de7df2400076e18bbb519759999ccc9e93381e1d25b382b5c503e41fce767eb")

    add_configs(
        "has_opengl3",
        {description = "Use OpenGL3 as a rendering backend. This is the recommended choice, especially for beginners", default = false, type = "boolean"}
    )
    add_configs("has_metal", {description = "Use Metal as a rendering backend. Apple only, advanced users only", default = false, type = "boolean"})
    add_configs("has_vulkan", {description = "Use Vulkan as a rendering backend. Advanced users only", default = false, type = "boolean"})
    add_configs("has_directx11", {description = "Use DirectX11 as a rendering backend. Windows only, still experimental", default = false, type = "boolean"})
    add_configs(
        "has_directx12",
        {description = "Use DirectX12 as a rendering backend. Windows only, advanced users only, still experimental", default = false, type = "boolean"}
    )
    add_configs("use_sdl2", {description = "Use Sdl2 as a platform backend", default = false, type = "boolean"})
    add_configs("use_glfw3", {description = "Use Glfw3 as a platform backend", default = false, type = "boolean"})
    add_configs("use_freetype", {description = "Use freetype for text rendering", default = true, type = "boolean"})

    add_configs("imgui_version", {description = "Use the specfied version of imgui", default = "*", type = "string"})

    add_deps("cmake")
    -- set link order
    add_links("hello_imgui", "stb_hello_imgui", "imgui", "lunasvg")

    on_load(function (package)
        if package:config("has_opengl3") then
            package:add("deps", "glad")
        elseif package:config("has_vulkan") then
            package:add("deps", "vulkansdk")
        end
        if package:config("use_sdl2") then
            package:add("deps", "libsdl")
        elseif package:config("use_glfw3") then
            package:add("deps", "glfw")
        end
        if package:config("use_freetype") then
            package:add("deps", "freetype")
        end
        -- https://xmake.io/#/manual/global_interfaces?id=use-as-a-private-package
        package:add("deps", "imgui-src " .. package:config("imgui_version"), {private = true})
    end)

    on_install("windows", "macosx", "linux", function (package)
        local configs = {}
        local imgui_src_dep = package:dep("imgui-src")

        table.insert(configs, "-DHELLOIMGUI_IMGUI_SOURCE_DIR=" .. path.join(imgui_src_dep:cachedir(), "source", imgui_src_dep:name()))

        table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:is_debug() and "Debug" or "Release"))
        table.insert(configs, "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))
        for name, enabled in pairs(package:configs()) do
            table.insert(configs, format("-DHELLOIMGUI_%s=%s", name:upper(), (enabled and "ON" or "OFF")))
        end

        import("package.tools.cmake").install(package, configs, {cmake_build = true})
    end)

    on_test(function (package)
        import("core.base.option", {alias = "opt"})
        assert(package:has_cxxincludes("hello_imgui/hello_imgui.h"))
        assert(package:check_cxxsnippets({[[
            #include <hello_imgui/hello_imgui.h>
            int main(int, char *[]) {
                HelloImGui::Run(
                    []{ ImGui::Text("Hello, world!"); }, // Gui code
                    "Hello!", true);                     // Window title + Window size auto
            }
        ]]}, {
            configs = {languages = "c++17"},
            verbose = opt.get("verbose") or false
        }))
    end)
