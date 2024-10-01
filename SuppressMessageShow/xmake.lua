-- set minimum xmake version
set_xmakever("2.8.2")

-- includes
includes("lib/commonlibsf")

-- set project
set_project("SuppressMessageShow")
set_version("1.0.1")
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

-- add required
add_requires("xbyak")

-- setup targets
target("SuppressMessageShow")
    -- add dependencies to target
    add_deps("commonlibsf")
    add_packages("xbyak")

    -- add commonlibsf plugin
    add_rules("commonlibsf.plugin", {
        name = "SuppressMessageShow",
        author = "Meridiano",
        description = "Suppress Message Show SFSE DLL",
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
