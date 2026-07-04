target("mirengine-lib")
    set_kind("static")

    -- Set C++ standard
    set_languages("c++20")

    -- Add includes relative to engine directory
    add_includedirs(".", { public = true })

    -- Add source files recursively under engine
    add_files("**.cpp|test/**.cpp")

    -- Link libraries / packages
    add_packages("zet", "lua", "sol2", "libsdl3", "libsdl3_image", "libsdl3_ttf", "libsdl3_mixer")

    -- Build mode definitions
    add_defines("ZET_NAMESPACE=mir", "zet=mir", { public = true })
    if is_mode("debug") then
        add_defines("DEBUG_MODE")
    end

target("mirengine-tests")
    set_kind("binary")
    set_default(false)

    set_languages("c++20")
    add_includedirs(".", { public = true })
    add_files("test/**.cpp")
    
    add_deps("mirengine-lib")
    add_packages("zet", "lua", "sol2", "libsdl3", "libsdl3_image", "libsdl3_ttf", "libsdl3_mixer")
    if is_plat("linux") then
        add_syslinks("png", "z")
    end
    add_defines("ZET_NAMESPACE=mir", "zet=mir")
    if is_mode("debug") then
        add_defines("DEBUG_MODE")
    end
