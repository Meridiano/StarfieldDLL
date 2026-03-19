-- include subprojects
includes("lib/commonlibsf")

-- set project constants
set_project("UltimateCutterSFSE")
set_version("1.0.0")
set_license("MIT")
set_languages("c++23")
set_warnings("allextra")

-- add common rules
add_rules("mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

-- set custom things
set_policy("package.requires_lock", true)
set_defaultmode("releasedbg")

-- define targets
target("UltimateCutterSFSE")
    add_rules("commonlibsf.plugin", {
        name = "UltimateCutterSFSE",
        author = "Meridiano",
        description = "Ultimate Cutter SFSE DLL",
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

    -- curl downloader
    local function curl(url, path)
        return format('curl -k "%s" -o "%s" --create-dirs', url, path)
    end
    on_load(function (target)
        os.run(curl("https://raw.githubusercontent.com/metayeti/mINI/refs/heads/master/src/mini/ini.h", "lib/mini/ini.h"))
    end)