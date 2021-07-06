# Prerequisites
- [Premake 5]([Premake](https://github.com/premake/premake-core))
- [CMake >= 3.16](https://cmake.org)
- [Python >= 3.5](https://python.org)
- [Conan >= 1.21.1](https://conan.io)
- [Visual Studio 2019](https://visualstudio.microsoft.com/)

# Building
Currently only tested with Visual Studio 2019 (>= 16.9.2)

Download and build dependencies with conan
```
premake5 conan fullsetup
```


Create a Visual Studio solution
```
premake5 vs2019
```


# Reverse Dependencies List

## Premake
- Main Project

## Python
- Conan

## Conan
- Main Project

## CMake
- Box2D
- Dear ImGui
- Dear ImGui Node Editor
- FreeType
- Harfbuzz
- ImPlot
- SOIL (littlstar)
