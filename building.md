# Prerequisites
- [Premake 5](https://github.com/premake/premake-core)
- [CMake >= 3.16](https://cmake.org)
- [Visual Studio 2022](https://visualstudio.microsoft.com/)

# Building
Currently only tested with Visual Studio 2022 (>= 17.4.5)

Download package and build tools:
```
premake5 setup download
```

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

## CMake
- assimp
- Box2D
- fmt
- FreeType
- GoogleTest
- Harfbuzz
- SOIL (littlstar)
