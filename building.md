# Prerequisites
- [Premake 5](https://github.com/premake/premake-core)
- [CMake >= 3.16](https://cmake.org)
- [Python >= 3.5](https://python.org)
- [Conan >= 1.21.1](https://conan.io)
- [Visual Studio 2022](https://visualstudio.microsoft.com/)

# Building
Currently only tested with Visual Studio 2022 (>= 17.4.5)

Download and build dependencies with conan
```
premake5 conan fullsetup
```


Create a Visual Studio solution
```
premake5 vs2022
```


# Reverse Dependencies List

## Premake
- Main Project

## Conan
- Main Project

## Python
- Conan

## CMake
- Box2D
- FreeType
- Harfbuzz
- SOIL (littlstar)
