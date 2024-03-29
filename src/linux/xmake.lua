add_requires("xcb", {system = true})
add_requires("libxi", {system = true})
add_requires("xcb-util", {system = true})
add_requires("xcb-util-wm", {system = true})
add_requires("xcb-util-errors", {system = false})
add_requires("libassert 1.2.2", { alias = "libassert", configs = { decompose = true} })

verson = 1
print (version)

target("libcapture")
    set_kind("static")
    add_files("./**.cpp")
    add_includedirs("$(projectdir)/include", "$(projectdir)/src/common")
    add_packages("spdlog")
    add_packages("xcb", "libxi", "xcb-util", "xcb-util-wm", "xcb-util-errors", "libassert")
    add_syslinks("pthread")
