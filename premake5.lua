workspace "VulkanRenderer"
	architecture "x64"

	configurations
	{
		"DEBUG",
		"RELEASE",
		"DIST"
	}

	startproject "VulkanRenderer"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

VULKAN_SDK = os.getenv("VULKAN_SDK")

Library = {}
Library["VULKAN"] = "%{VULKAN_SDK}/Lib"

Include = {}
Include["VULKAN"] = "%{VULKAN_SDK}/Include"

include "vendor/GLFW"

project "VulkanRenderer"
	location "VulkanRenderer"
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
		"%{wks.name}/vendor/glm/glm/**.hpp",
		"%{wks.name}/vendor/glm/glm/**.inl",
		"%{wks.name}/vendor/glm/glm/**.cpp",
		"%{wks.name}/vendor/glm/glm/**.h",
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{Include.VULKAN}",
		"%{wks.name}/vendor/glm",
		"%{wks.name}/vendor/GLFW/include"
	}

	links
	{
		"GLFW",
		"opengl32.lib"
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

		links
		{

		}

	filter "configurations:RELEASE"
		defines "VKR_RELEASE"
		runtime "Release"
		optimize "on"

		links
		{

		}

	filter "configurations:DIST"
		defines "VKR_DIST"
		runtime "Release"
		optimize "on"



