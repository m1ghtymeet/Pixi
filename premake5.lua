include "Dependencies.lua"

workspace "Hazel"
	architecture "x86_64"
	startproject "Hazelnut"
	
	configurations {
		"Debug",
		"Release",
		"Dist"
	}
	
	flags {
		"MultiProcessorCompile"
	}
	
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)

group "Dependencies"
	include "Hazel/vendor/GLFW"
	include "Hazel/vendor/glad"
	include "Hazel/vendor/imgui"
	include "Hazel/vendor/yaml-cpp"
	include "Hazel/vendor/PhysX"
	include "Hazel/vendor/jolt_premake.lua"
group ""

group "Core"
	include "Hazel"
group ""

group "Tools"
	include "Hazelnut"
group ""
		
project "HazelRuntime"
	location "HazelRuntime"
	kind "ConsoleApp"
	language "C++"
	
	targetdir ("bin/".. outputdir .."/%{prj.name}")
	objdir ("bin-int/".. outputdir .."/%{prj.name}")
	
	files {
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}
	
	includedirs {
		"%{prj.name}/../Hazel/vendor/spdlog/include/",
		"Hazel/src",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
	}
	
	links {
		"ImGui",
		"Hazel"
	}
	
	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "10.0"
		
		defines {
			"HZ_PLATFORM_WINDOWS",
		}
		
	filter "configurations:Debug"
		defines "HZ_DEBUG"
		symbols "on"
		defines {
			"HZ_DEBUG"
		}
		
	filter "configurations:Release"
		defines "HZ_RELEASE"
		symbols "on"
		
	filter "configurations:Dist"
		defines "HZ_DIST"
		symbols "on"
		
