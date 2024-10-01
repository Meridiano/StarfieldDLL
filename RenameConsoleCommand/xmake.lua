-- set minimum xmake version
set_xmakever("2.8.2")

-- includes
includes("lib/commonlibsf")

-- set project
set_project("RenameConsoleCommand")
set_version("1.2.2")
set_license("MIT")

-- set defaults
set_languages("c++23")
set_warnings("allextra")
set_defaultmode("releasedbg")

-- set policies
set_policy("package.requires_lock", true)

-- add rules
add_rules("mode.releasedbg", "mode.debug")
add_rules("plugin.vsxmake.autoupdate")

-- setup targets
target("RenameConsoleCommand")
    -- add dependencies to target
    add_deps("commonlibsf")

    -- add commonlibsf plugin
    add_rules("commonlibsf.plugin", {
        name = "RenameConsoleCommand",
        author = "Meridiano",
        description = "Rename Console Command SFSE DLL",
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
