
baseName = path.getbasename(os.getcwd());

project (workspaceName)
  	kind "ConsoleApp"
    location "../_build"
    targetdir "../_bin/%{cfg.buildcfg}"
	
    filter "configurations:Release"
		kind "WindowedApp"
		entrypoint "mainCRTStartup"

	filter "action:vs*"
        debugdir "$(SolutionDir)"
		
	filter {"action:vs*", "configurations:Release"}
			kind "WindowedApp"
			entrypoint "mainCRTStartup"
	filter {}
	
	vpaths 
	{
	  ["Header Files/*"] = { "include/**.h",  "include/**.hpp", "src/**.h", "src/**.hpp", "**.h", "**.hpp"},
	  ["Source Files/*"] = {"src/**.c", "src/**.cpp","**.c", "**.cpp"},
	}
	files {"**.c", "**.cpp", "**.h", "**.hpp"}

	includedirs { "./", "src", "include"}
	
	includedirs { "../raylib/src" }   -- 让编译器能找到 raylib.h
    dependson  { "raylib" }           -- 先编好 raylib 再链接
    links      { "raylib" }           -- 把 raylib 链接进来

    filter "system:windows"
        links { "winmm", "gdi32", "opengl32", "shell32" }  -- raylib 在 Windows 需要的系统库
    filter {}
	link_raylib();
	
	-- To link to a lib use link_to("LIB_FOLDER_NAME")