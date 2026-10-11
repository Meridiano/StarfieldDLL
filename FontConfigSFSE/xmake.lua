-- xmake requirement
set_xmakever("3.0.0")

package("glaze_9")
    set_base("glaze")
    add_versions("v9.0.0", "dd7b033c5bf6e4308615bb7834c541291d5b20f64121d9a68222a39414635517")
package_end()

-- include packages
includes("lib/commonlibsf")
add_requires("glaze_9 v9.0.0")

-- set project constants
set_project("FontConfigSFSE")
set_version("1.0.1")
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
target("FontConfigSFSE")
    add_packages("glaze_9")
    add_rules("commonlibsf.plugin", {
        name = "FontConfigSFSE",
        author = "Meridiano",
        description = "FontConfig SFSE DLL",
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
