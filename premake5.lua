workspace "Deliverable"
    architecture "x64"

    configurations
    {
        "Debug",
        "Release"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Deliverable"
    kind "ConsoleApp"
    language "C++"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("obj/" .. outputdir .. "/%{prj.name}")

    files
    {
        "include/**.h",
        "src/**.cpp",
        "media/shaders/**.vertex",
        "media/shaders/**.fragment",
        "media/shaders/**.vs",
        "media/shaders/**.fs",
        "media/shaders/**.compute"
    }

    includedirs
    {
        "include",
        "dependencies/glew/include",
        "dependencies/glfw/include",
        "dependencies/glm/include",
        "dependencies/stb/include",
        "dependencies/imgui/include"
    }

    libdirs
    {
        "dependencies/glew/lib",
        "dependencies/glfw/lib",
        "dependencies/imgui/lib"
    }

    links
    {
        "opengl32",
        "glfw3",
        "glew32s",
        "imgui"
    }

    filter "system:windows"
        cppdialect "C++17"
        staticruntime "Off"
        systemversion "10.0.17134.0"

		defines
		{
            "GLEW_STATIC",
            "GLM_FORCE_RADIANS",
            "GLM_FORCE_DEPTH_ZERO_TO_ONE",
            "_CRT_SECURE_NO_WARNINGS"
        }

    filter "configurations:Debug"
        symbols "On"

    filter "configurations:Release"
        optimize "On"
