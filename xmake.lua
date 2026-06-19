---@diagnostic disable: undefined-global
add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", { outputdir = "." })

if is_plat("windows") then
    add_cxflags("/utf-8", { tools = { "clang_cl", "cl" } })
end

-- Add doctest package
add_requires("doctest")

-- Project configuration
set_project("art-gallery-ghost")
set_version("1.0.0")

-- Define support modes
add_rules("mode.debug", "mode.release")

-- Option to build as a macOS bundle
option("build_bundle")
set_default(false)
set_showmenu(true)
set_description("Build as macOS app bundle")
option_end()

-- 1. Define local zet package
package("zet")
set_homepage("https://github.com/G1rmmr/zetcontainer-cpp")
set_description("Zero-allocated Execution Toolkit")

    set_urls("C:/Users/g1/source/repos/G1rmmr/zetcontainer-cpp/.git")
    add_versions("main", "main")

    on_install(function(package)
        for _, filepath in ipairs(os.files("src/**.hpp")) do
            local content = io.readfile(filepath)
            content = content:gsub("namespace zet", "namespace mir")
            content = content:gsub("zet::", "mir::")
            io.writefile(filepath, content)
        end
        for _, filepath in ipairs(os.files("src/**.cpp")) do
            local content = io.readfile(filepath)
            content = content:gsub("namespace zet", "namespace mir")
            content = content:gsub("zet::", "mir::")
            io.writefile(filepath, content)
        end
        import("package.tools.xmake").install(package)
    end)
package_end()

-- Specify package requirements
add_requires("sfml", { configs = { shared = false } })
add_requires("zet")

-- Main Target
target("art-gallery-ghost")
set_kind("binary")

-- Specify source files
add_files("main.cpp")

-- Set C++ standard
set_languages("c++20")

-- Add engine include directory
add_includedirs("engine", "engine/engine", { public = true })

-- Compile definitions
add_defines("SFML_STATIC")

-- Link libraries / packages
add_packages("sfml", "zet")

-- Absolute path of asset directory as macro
add_defines("ASSET_DIR=\"" .. path.absolute("assets") .. "\"")

-- Compile-commands generation
add_rules("plugin.compile_commands.autoupdate", { outputdir = "." })

-- Build mode definitions
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
