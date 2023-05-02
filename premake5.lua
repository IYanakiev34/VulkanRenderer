workspace "VulkanRenderer"
	architecture "x64"
	targetdir "build"
	startproject "Renderer"
	configurations
	{
		"DEBUG",
		"RELEASE",
		"DIST"
	}


outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

VULKAN_SDK = os.getenv("VULKAN_SDK")

Library = {}
Library["VULKAN"] = "%{VULKAN_SDK}/Lib"
Library["VULKAN_lib"] = "%{VULKAN_SDK}/Lib/vulkan-1.lib"
Library["VULKAN_utils"] = "%{VULKAN_SDK}/Lib/VkLayer_utils.lib"
Include = {}
Include["VULKAN"] = "%{VULKAN_SDK}/Include"

include "vendor/GLFW"


project "Renderer"
	location "Renderer"
	kind "ConsoleApp"
	staticruntime "off"
	language "C++"
	cppdialect "C++17"

	targetdir ("bin/" .. outputdir .. "/{%prj.name}")
	objdir ("bin-int/" .. outputdir .. "/{%prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.hpp",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.inl",
		"%{wks.name}/Renderer/vendor/glm/glm/**.hpp",
		"%{wks.name}/Renderer/vendor/glm/glm/**.inl",
		"%{wks.name}/Renderer/vendor/glm/glm/**.cpp",
		"%{wks.name}/Renderer/vendor/glm/glm/**.h",
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{Include.VULKAN}",
		"vendor/glm",
		"vendor/GLFW/include",
	}

	links
	{
		"GLFW",
		"opengl32.lib",
		"%{Library.VULKAN_lib}",
		"%{Library.VULKAN_utils}"
	}

	defines{
		"_CRT_SECURE_NO_WARNINGS"
	}

	filter "system:windows"
		systemversion "latest"

		defines{
			"VKR_WINDOWS_PLATFORM",
			"VKR_ENABLE_ASSERTS",
		}

	filter "configurations:DEBUG"
		defines "VKR_DEBUG"
		runtime "Debug"
		symbols "on"


	filter "configurations:RELEASE"
		defines "VKR_RELEASE"
		runtime "Release"
		optimize "on"


	filter "configurations:DIST"
		defines "VKR_DIST"
		runtime "Release"
		optimize "on"



