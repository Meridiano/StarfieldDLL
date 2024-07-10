-- set minimum xmake version
set_xmakever("2.8.2")

-- add custom package repository
add_repositories("re https://github.com/Starfield-Reverse-Engineering/commonlibsf-xrepo")

-- set project
set_project("RenameConsoleCommand")
set_version("1.1.0")
set_license("MIT")

-- set defaults
set_languages("c++23")
set_optimize("faster")
-- disable warnings
-- set_warnings("allextra", "error")
set_defaultmode("releasedbg")

-- add rules
add_rules("mode.releasedbg", "mode.debug")
add_rules("plugin.vsxmake.autoupdate")

-- require package dependencies
add_requires("commonlibsf")

-- add missing links
add_syslinks("ws2_32")

-- setup targets
target("RenameConsoleCommand")
    -- bind package dependencies
    add_packages("commonlibsf")

    -- add commonlibsf plugin
    add_rules("@commonlibsf/plugin", {
        name = "RenameConsoleCommand",
        author = "Meridiano",
        description = "Rename Console Command SFSE DLL Plugin",
        email = "Discord:@meridiano"
    })

    -- add source files
    add_files("src/*.cpp")
    add_headerfiles("src/*.h")
    add_includedirs("src")
    set_pcxxheader("src/pch.h")
