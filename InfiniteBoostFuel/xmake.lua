-- set minimum xmake version
set_xmakever("2.8.2")

-- add custom package repository
add_repositories("re https://github.com/Starfield-Reverse-Engineering/commonlibsf-xrepo")

-- set project
set_project("InfiniteBoostFuel")
set_version("1.3.4")
set_license("MIT")

-- set defaults
set_languages("c++23")
set_optimize("faster")
set_defaultmode("releasedbg")

-- add rules
add_rules("mode.releasedbg", "mode.debug")
add_rules("plugin.vsxmake.autoupdate")

-- require package dependencies
add_requires("commonlibsf")

-- setup targets
target("InfiniteBoostFuel")
    -- bind package dependencies
    add_packages("commonlibsf")

    -- add commonlibsf plugin
    add_rules("@commonlibsf/plugin", {
        name = "InfiniteBoostFuel",
        author = "Meridiano",
        description = "Infinite Boost Fuel SFSE DLL",
        email = "Discord:@meridiano",
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
