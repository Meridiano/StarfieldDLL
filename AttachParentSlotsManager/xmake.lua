-- include subprojects
includes("lib/commonlibsf")

-- set project constants
set_project("AttachParentSlotsManager")
set_version("1.0.0")
set_license("GPL-3.0")
set_languages("c++23")
set_warnings("allextra")

-- add common rules
add_rules("mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

-- set customs options
set_defaultmode("releasedbg")
set_policy("package.requires_lock", true)

-- define targets
target("AttachParentSlotsManager")
    add_rules("commonlibsf.plugin", {
        name = "AttachParentSlotsManager",
        author = "Meridiano",
        description = "Attach Parent Slots Manager SFSE DLL",
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
