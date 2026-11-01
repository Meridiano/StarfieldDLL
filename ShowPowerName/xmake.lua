-- xmake requirement
set_xmakever("3.0.0")

-- include packages
includes("lib/commonlibsf")
add_requires("imgui", {configs = {dx12 = true, win32 = true}})
add_requires("toml++")

-- set project constants
set_project("ShowPowerName")
set_version("1.0.2")
set_license("GPL-3.0")
set_languages("c++23")
set_warnings("allextra")

-- add common rules
add_rules("mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

-- set optionals
set_policy("package.requires_lock", true)
set_defaultmode("releasedbg")

-- define targets
target("ShowPowerName")
    add_packages("imgui")
    add_packages("toml++")
    add_rules("commonlibsf.plugin", {
        name = "ShowPowerName",
        author = "Meridiano",
        description = "Show Power Name SFSE DLL",
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
