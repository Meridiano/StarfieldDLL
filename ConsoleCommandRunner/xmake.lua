-- set minimum xmake version
set_xmakever("2.8.2")

-- add custom package repository
add_repositories("re https://github.com/Starfield-Reverse-Engineering/commonlibsf-xrepo")

-- set project
set_project("ConsoleCommandRunner")
set_version("1.4.6")
set_license("MIT")

-- set defaults
set_languages("c++23")
set_optimize("faster")
set_defaultmode("releasedbg")

-- add rules
add_rules("mode.releasedbg", "mode.debug")
add_rules("plugin.vsxmake.autoupdate")

-- required dependencies
add_requires("commonlibsf", "toml++")

-- setup targets
target("ConsoleCommandRunner")
    -- bind package dependencies
    add_packages("commonlibsf", "toml++")

    -- add commonlibsf plugin
    add_rules("@commonlibsf/plugin", {
        name = "ConsoleCommandRunner",
        author = "Bobbyclue-Meridiano",
        description = "Console Command Runner SFSE DLL",
        email = "www.nexusmods.com/users/9609463",
		
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
