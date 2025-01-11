project "Hazelnut"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"
	
	targetdir ("%{wks.location}/bin/".. outputdir .."/%{prj.name}")
	objdir ("%{wks.location}/bin-int/".. outputdir .."/%{prj.name}")
	
	files {
		"src/**.h",
		"src/**.cpp",
		
	}
	
	includedirs {
		"%{wks.location}/Hazel/vendor/spdlog/include/",
		"%{wks.location}/Hazel/src",
		"%{wks.location}/Hazel/vendor",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.glad}",
		"%{IncludeDir.Assimp}",
		"%{wks.location}/Hazel/vendor/nlohmann_json/include",
		"%{IncludeDir.Jolt}",
		"%{IncludeDir.PhysX}",
	}
	
	links {
		"ImGui",
		"Hazel",
		"PhysX",
		"JoltPhysics",
		"%{wks.location}/Hazel/vendor/assimp/lib/assimp-vc143-mtd.lib",
		--"%{wks.location}/Hazel/vendor/JoltPhysics/lib/Jolt.lib",
	}
	
	filter "system:windows"
		systemversion "latest"
		
		defines {
			"HZ_PLATFORM_WINDOWS",
		}
		
	filter "configurations:Debug"
		defines "HZ_DEBUG"
		runtime "Debug"
		buildoptions "/MTd"
		symbols "on"
		
	filter "configurations:Release"
		defines "HZ_RELEASE"
		symbols "on"
		
	filter "configurations:Dist"
		defines "HZ_DIST"
		symbols "on"