---@diagnostic disable: undefined-global
add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", { outputdir = "." })

if is_plat("windows") then
    add_cxflags("/utf-8", { tools = { "clang_cl", "cl" } })
end

-- Project configuration
set_project("mirengine")
set_version("1.0.0")

MIR_LOCAL_ZET_DIR = path.absolute("../zetcontainer-cpp", os.scriptdir())
option("local_zet")
    -- The sibling checkout is a development convenience, not a compatibility
    -- guarantee. Keep the version pinned by this project as the default.
    set_default(false)
    set_showmenu(true)
    set_description("Use the sibling zetcontainer-cpp checkout (requires the compatible MIR API revision)")
option_end()
MIR_USE_LOCAL_ZET = has_config("local_zet") and os.isdir(path.join(MIR_LOCAL_ZET_DIR, "src"))

-- Define local zet package
package("zet")
    set_homepage("https://github.com/G1rmmr/zetcontainer-cpp")
    set_description("Zero-allocated Execution Toolkit")
    
    set_urls("https://github.com/G1rmmr/zetcontainer-cpp.git")
    -- Pin the allocation-free ZET API used by this engine. Tracking `main`
    -- allowed CI package caches to resolve the older void Push()/no IsValid API.
    add_versions("a2e0fd7", "a2e0fd7badf934fd186da1983648937407b2d539")
    
    add_configs("namespace", {description = "Set the library namespace", default = "zet", type = "string"})
    
    on_install(function (package)
        io.writefile("xmake.lua", [[
add_rules("mode.debug", "mode.release")
target("zet")
    set_kind("static")
    set_languages("c++20")
    add_includedirs("src", "src/container", "src/memory", {public = true})
    add_headerfiles("src/(**.hpp)")
    add_files("src/*.cpp")
    add_defines("ZET_NAMESPACE=mir", {public = true})
]])
        import("package.tools.xmake").install(package, {})
    end)
package_end()

-- Define local libsdl3_image package to bypass shared dynamic loading check
package("libsdl3_image")
    set_homepage("https://github.com/libsdl-org/SDL_image")
    set_description("Image decoding for many popular formats for Simple Directmedia Layer.")
    set_license("zlib")

    add_urls("https://www.libsdl.org/projects/SDL_image/release/SDL3_image-$(version).zip",
             "https://github.com/libsdl-org/SDL_image/releases/download/release-$(version)/SDL3_image-$(version).zip", { alias = "archive" })
    add_urls("https://github.com/libsdl-org/SDL_image.git", { alias = "github" })

    add_versions("archive:3.4.0", "158d89a217afc9869d85dcef58800ea90626fa0c96c588979055cb34f567bd7f")
    add_versions("archive:3.2.0", "144715a6afae430adc275fd3ab0e3e96177a2752cc10a49ca78511b1e665964e")
    add_versions("github:3.4.0", "release-3.4.0")
    add_versions("github:3.2.0", "release-3.2.0")

    add_deps("cmake", "libpng", "libjpeg")

    on_load(function (package)
        package:add("deps", "libsdl3", { configs = { shared = package:config("shared") }})
        package:add("deps", "libpng", { configs = { shared = package:config("shared") }})
        package:add("deps", "libjpeg", { configs = { shared = package:config("shared") }})
    end)

    on_install(function (package)
        local configs = {
            "-DSDLIMAGE_SAMPLES=OFF",
            "-DSDLIMAGE_TESTS=OFF",
            "-DSDLIMAGE_VENDORED=OFF",
            "-DSDLIMAGE_PNG_SHARED=OFF",
            "-DSDLIMAGE_JPG_SHARED=OFF"
        }
        table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:debug() and "Debug" or "Release"))
        table.insert(configs, "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))
        import("package.tools.cmake").install(package, configs)
    end)
package_end()

if not MIR_USE_LOCAL_ZET then
    add_requires("zet a2e0fd7", {configs = {namespace = "mir"}})
end
add_requireconfs("lua", {version = "5.4.x", configs = {shared = true}})
add_requires("lua 5.4.x", {configs = {shared = true}})
add_requires("sol2")
add_requires("libsdl3")
add_requires("libsdl3_image")
add_requires("libsdl3_ttf")
add_requires("libsdl3_mixer")

-- Include targets after dependency selection so engine/xmake.lua can use the
-- local sibling checkout during development without patching package sources.
includes("engine")
