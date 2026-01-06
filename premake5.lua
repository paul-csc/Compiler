workspace "Chess"
    architecture "x64"
    location "."

    configurations
    {
        "Debug",
        "Release"
    }

project "Chess"
    language "C++"
    cppdialect "C++20"
    kind "ConsoleApp"
    staticruntime "off"

    pchheader "pch.h"
    pchsource "src/pch.cpp"

    files
    {
        "src/**.h",
        "src/**.hpp",
        "src/**.cpp"
    }

    includedirs { "src" }
    
    filter "configurations:Debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "RELEASE" }
        runtime "Release"
        optimize "On"
        symbols "Off"