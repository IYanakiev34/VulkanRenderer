workspace "VulkanRenderer"
	architecture "x64"
	targetdir "build"
	startproject "Testbed"
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


project "Renderer"
	location "Renderer"
	kind "SharedLib"
	staticruntime "off"
	language "C"
	cdialect "C11"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	postbuildcommands 
{
	("{COPY} %{cfg.buildtarget.directory}/%{prj.name}.dll ../bin/" .. outputdir .. "/Testbed/")
}

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.c",
		"%{prj.name}/src/**.inl",
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{Include.VULKAN}",
	}

	links
	{
		"%{Library.VULKAN_lib}",
		"%{Library.VULKAN_utils}"
	}

	defines{
		"_CRT_SECURE_NO_WARNINGS",
		"VNEXPORT" -- Used to build the DLL
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
		symbols "On"


	filter "configurations:RELEASE"
		defines "VKR_RELEASE"
		runtime "Release"
		optimize "On"


	filter "configurations:DIST"
		defines "VKR_DIST"
		runtime "Release"
		optimize "Full"


project "Editor"
	location "Editor"
	kind "ConsoleApp"
	language "C"
	cdialect "C11"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files 
	{
		"%{prj.name}/src/**.c",
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.inl"
	}

	includedirs
	{
		"%{prj.name}/src",
	}

	dependson
	{
		"Renderer"
	}

	links
	{
		"Renderer"
	}

	filter "configurations:DEBUG"
		defines "ED_DEBUG"
		runtime "Debug"
		symbols "On"


	filter "configurations:RELEASE"
		defines "ED_RELEASE"
		runtime "Release"
		optimize "On"


	filter "configurations:DIST"
		defines "ED_DIST"
		runtime "Release"
		optimize "Full"


project "Testbed"
	location "Testbed"
	kind "ConsoleApp"
	language "C"
	cdialect "C11"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files 
	{
		"%{prj.name}/src/**.c",
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.inl"
	}

	includedirs
	{
		"%{wks.location}/Renderer/src",
		"%{prj.name}/src"
	}

	dependson
	{
		"Renderer"
	}

	links
	{
		"Renderer"
	}

	filter "configurations:DEBUG"
		defines "TB_DEBUG"
		runtime "Debug"
		symbols "On"


	filter "configurations:RELEASE"
		defines "TB_RELEASE"
		runtime "Release"
		optimize "On"


	filter "configurations:DIST"
		defines "TB_DIST"
		runtime "Release"
		optimize "Full"


