---@diagnostic disable: undefined-global
add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", { outputdir = "." })

if is_plat("windows") then
    add_cxflags("/utf-8", { tools = { "clang_cl", "cl" } })
end

-- Include engine subdirectory build configuration
includes("engine")

-- Project configuration
set_project("mirengine")
set_version("1.0.0")

-- Option to build as a macOS bundle
option("build_bundle")
    set_default(false)
    set_showmenu(true)
    set_description("Build as macOS app bundle")
option_end()

-- Define local zet package
package("zet")
    set_homepage("https://github.com/G1rmmr/mir-container")
    set_description("Zero-allocated Execution Toolkit")
    
    set_urls("/home/g1/source/zet-cpp/.git", "https://github.com/G1rmmr/mir-container.git")
    add_versions("main", "main")
    
    add_configs("namespace", {description = "Set the library namespace", default = "zet", type = "string"})
    
    on_install(function (package)
        io.writefile("xmake.lua", [[
add_rules("mode.debug", "mode.release")
target("zet")
    set_kind("static")
    set_languages("c++20")
    add_includedirs("src", "src/container", "src/memory", {public = true})
    add_headerfiles("src/**.hpp")
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

    add_deps("cmake")

    on_load(function (package)
        package:add("deps", "libsdl3", { configs = { shared = package:config("shared") }})
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

add_requires("zet", {configs = {namespace = "mir"}})
add_requires("lua 5.4.x")
add_requires("sol2")
add_requires("libsdl3")
add_requires("libsdl3_image")
add_requires("libsdl3_ttf")
add_requires("libsdl3_mixer")

target("mirengine")
    set_kind("binary")

    -- Set C++ standard
    set_languages("c++20")

    -- Add includes for engine source folders
    add_includedirs("engine", { public = true })

    -- Dependency on mirengine static library
    add_deps("mirengine-lib")

    -- Add source files
    add_files("main.cpp")

    -- Link libraries / packages
    add_packages("zet", "lua", "sol2", "libsdl3", "libsdl3_image", "libsdl3_ttf", "libsdl3_mixer")
    add_syslinks("png", "z")

    -- Compile-commands generation
    add_rules("plugin.compile_commands.autoupdate", { outputdir = "." })

    -- Build mode definitions
    add_defines("ZET_NAMESPACE=mir", "zet=mir")
    if is_mode("debug") then
        add_defines("DEBUG_MODE")
    end

if is_plat("macosx") then
    add_defines("GL_SILENCE_DEPRECATION")

    if has_config("build_bundle") then
        add_rules("xcode.application")
        add_values("xcode.bundle_identifier", "com.example.art-gallery-ghost")
        add_values("xcode.bundle_name", "Art Gallery Ghost")
        add_values("xcode.bundle_version", "1.0")
        add_values("xcode.bundle_short_version", "1.0")

        after_build(function(target)
            os.exec("codesign --force --deep --sign - %s", target:targetdir() .. "/Art Gallery Ghost.app")
        end)
    end
end
