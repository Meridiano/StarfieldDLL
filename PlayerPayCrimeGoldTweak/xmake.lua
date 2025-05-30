-- set minimum xmake version
set_xmakever("2.8.2")

-- includes
includes("lib/commonlibsf")

-- set project
set_project("PlayerPayCrimeGoldTweak")
set_version("1.4.0")
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

-- add libs
add_requires("simpleini")

-- setup targets
target("PlayerPayCrimeGoldTweak")
    -- add dependencies to target
    add_deps("commonlibsf")
    add_packages("simpleini")

    -- add commonlibsf plugin
    add_rules("commonlibsf.plugin", {
        name = "PlayerPayCrimeGoldTweak",
        author = "Meridiano",
        description = "PlayerPayCrimeGold Tweak SFSE DLL",
        email = "discord:@meridiano",
        options = {
            address_library = true,
            no_struct_use = true
        }
    })

    -- add source files
    add_files("src/*.cpp")
    add_headerfiles("src/*.h")
    add_includedirs("src")
    set_pcxxheader("src/pch.h")
