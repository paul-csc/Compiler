workspace "Compiler"
    architecture "x64"
    location "."

    configurations
    {
        "Debug",
        "Release"
    }

project "Compiler"
    language "C++"
    cppdialect "C++20"
    kind "ConsoleApp"
    staticruntime "off"

    pchheader "pch.h"
    pchsource "src/pch.cpp"
    filter "toolset:gcc or toolset:clang"
        pchsource ""
    filter {}

    files
    {
        "src/**.h",
        "src/**.hpp",
        "src/**.cpp"
    }

    includedirs { "src" }

    filter "toolset:gcc or toolset:clang"
        buildoptions {
            "-Winvalid-pch",
            "-H"
        }
    filter {}

    filter "configurations:Debug"
        defines { "DEBUG", "_DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "RELEASE", "NDEBUG" }
        runtime "Release"
        optimize "On"
        symbols "Off"