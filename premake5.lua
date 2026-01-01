workspace "Glassy"
    architecture "x64"
    location "."
    toolset "clang"

    configurations
    {
        "Debug",
        "Release"
    }

    buildoptions { "-fcolor-diagnostics" }

project "Glassy"
    language "C++"
    cppdialect "C++20"
    kind "ConsoleApp"
    staticruntime "off"

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