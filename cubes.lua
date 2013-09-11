--
-- Dependencies
--


sdk_project "cubes"
    sdk_kind "ConsoleApp"
    sdk_location "."


    -- Application builds for all configurations and static_dcrt platforms
    configurations { sdk.CONFIGS_ALL }
    platforms { sdk.WIN_ALL, sdk.ORBIS_ALL }

    uuid "99189624-54c8-4347-a0f6-4dbb7104c3f8"
    
	removeflags {"FatalWarnings"}

    vpaths
    {
        ["Headers"] = { "*.h", },
        ["Source"] = { "*.cpp", },
    }

    files
    {
        "*.cpp",
        "*.h",
    }
    

    includedirs
    {
        "../../include",
        "../../3rdparty/openal/include",
        "../../../bx/include",
        "../common",
    }

    defines
    {
        "ATGTOOLS_NO_PRAGMA_COMMENT_LIB",
        "BOOST_ALL_NO_LIB",
    }
    
    links
    {
        "wws_bgfx",
		"common"
    }

	configuration { "Orbis*" }
		links { 
			"SceNet_stub_weak", "ScePosix_stub_weak"
		}

	configuration { "Win*" }

		buildoptions { "/wd4267", "/wd4511", "/wd4512", "/wd4702" }

		links { 
			"psapi", 
			"dbghelp", 
			"shlwapi", 
			"shell32", 
			"advapi32", 
			"ws2_32" 
		}

