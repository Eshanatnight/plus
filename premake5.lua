workspace "plus"
	configurations { "Debug", "Release" }
	platforms { "Win64" , "Win32" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "plus"
	kind "ConsoleApp"
	language "C++" -- "C", "C++", "C#"
	cppdialect "C++latest"
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
    files { "src/*.cpp", "src/cppgit2/*.cpp", "include/cppgit2/*.h", "include/*.h" }
    includedirs {"include", "include/cppgit2"}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
		defines { "DEBUG" }

	filter "configurations:Release"
		runtime "Release"
		optimize "on" -- "on", "off", "Debug", "Size", "Speed", "Full"
		defines { "NDEBUG" }

	filter "configurations:*32"
		architecture "x86"

	filter "configurations:*64"
		architecture "x86_64"
