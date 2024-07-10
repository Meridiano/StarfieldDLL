-- set minimum xmake version
set_xmakever("2.8.2")

-- add custom package repository
add_repositories("re https://github.com/Starfield-Reverse-Engineering/commonlibsf-xrepo")

-- set project
set_project("PlayerPayCrimeGoldTweak")
set_version("1.2.0")
set_license("MIT")

-- set defaults
set_languages("c++23")
set_optimize("faster")
set_warnings("allextra", "error")
set_defaultmode("releasedbg")

-- add rules
add_rules("mode.releasedbg", "mode.debug")
add_rules("plugin.vsxmake.autoupdate")

-- require package dependencies
add_requires("simpleini", "commonlibsf")

-- add missing links
add_syslinks("ws2_32")

-- setup targets
target("PlayerPayCrimeGoldTweak")
    -- bind package dependencies
    add_packages("simpleini", "commonlibsf")

    -- add commonlibsf plugin
    add_rules("@commonlibsf/plugin", {
        name = "PlayerPayCrimeGoldTweak",
        author = "Meridiano",
        description = "PlayerPayCrimeGold Tweak SFSE DLL",
        email = "Discord:@meridiano"
    })

    -- add source files
    add_files("src/*.cpp")
    add_headerfiles("src/*.h")
    add_includedirs("src")
    set_pcxxheader("src/pch.h")
