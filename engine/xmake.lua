target("mirengine-lib")
    set_kind("static")

    -- Set C++ standard
    set_languages("c++20")

    -- Add includes relative to engine directory
    add_includedirs(".", { public = true })

    -- Add source files recursively under engine (excluding main and tests)
    add_headerfiles("**.hpp|test/**.hpp")
    add_files("**.cpp|main.cpp|test/**.cpp")

    -- Link libraries / packages
    add_packages("zet", "lua", "sol2", "libsdl3", "libsdl3_image", "libsdl3_ttf", "libsdl3_mixer")

    -- Build mode definitions
    add_defines("ZET_NAMESPACE=mir", "zet=mir", { public = true })
    if is_mode("debug") then
        add_defines("DEBUG_MODE")
    end

    -- Read project-specific config if it exists in the shell's current working directory
    on_load(function (target)
        local pwd = os.getenv("PWD") or (os.pwd and os.pwd()) or "."
        local local_config = path.join(pwd, "config.lua")
        if os.isfile(local_config) then
            for line in io.lines(local_config) do
                local key, val = line:match("^%s*([%w_]+)%s*=%s*(%d+)%s*$")
                if key and val then
                    if key == "MAX_ENTITY" then
                        target:add("defines", "CONFIG_MAX_ENTITY=" .. val, { public = true })
                    elseif key == "MAX_COMPONENT" then
                        target:add("defines", "CONFIG_MAX_COMPONENT=" .. val, { public = true })
                    elseif key == "MAX_SYSTEM" then
                        target:add("defines", "CONFIG_MAX_SYSTEM=" .. val, { public = true })
                    end
                end
            end
        end
    end)

target("mirengine")
    set_kind("binary")

    set_languages("c++20")
    add_includedirs(".", { public = true })
    add_files("main.cpp")
    
    add_deps("mirengine-lib")
    add_packages("zet", "lua", "sol2", "libsdl3", "libsdl3_image", "libsdl3_ttf", "libsdl3_mixer")
    if is_plat("linux") then
        add_syslinks("png", "z")
    end
    add_defines("ZET_NAMESPACE=mir", "zet=mir")
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
