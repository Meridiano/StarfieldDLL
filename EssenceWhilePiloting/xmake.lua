-- set minimum xmake version
set_xmakever("2.8.2")

-- includes
includes("lib/commonlibsf")

-- set project
set_project("EssenceWhilePiloting")
set_version("1.0.0")
set_license("MIT")

-- set defaults
set_languages("c++23")
set_warnings("allextra")

-- set policies
set_policy("package.requires_lock", true)

-- add rules
add_rules("mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")
set_config("mode", "releasedbg")

-- setup targets
target("EssenceWhilePiloting")
    -- add dependencies to target
    add_deps("commonlibsf")

    -- add commonlibsf plugin
    add_rules("commonlibsf.plugin", {
        name = "EssenceWhilePiloting",
        author = "Meridiano",
        description = "Essence While Piloting SFSE DLL",
        email = "discord:@meridiano",
        options = {
            address_library = true,
            no_struct_use = true
        }
    })

    -- add src files
    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    set_pcxxheader("src/pch.h")
