from conans import ConanFile, tools, errors, CMake
import os

class Recipe(ConanFile):
	name = "assimp"
	description = "The Asset-Importer-Lib (in short assimp) is a library to load and process geometric scenes from various 3D-Dataformats."
	license = "BSD 3-clause"
	homepage = "https://www.assimp.org"
	url = "https://github.com/assimp/assimp.git"
	settings = "arch", "build_type", "compiler"

	def source(self):
		isver = self.version[0].isdigit()
		tools.Git().clone(
			url=self.url,
			branch=("v" if isver else "") + self.version,
			shallow=True
		)
		
	def build(self):
		cmake = CMake(self)
		
		cmake.definitions["BUILD_SHARED_LIBS"]          = "OFF"	# Generation of shared libs ( dll for windows, so for Linux ). Set this to OFF to get a static lib.
		cmake.definitions["BUILD_FRAMEWORK"]            = "OFF"	# Mac only. Build package as Mac OS X Framework bundle
		cmake.definitions["ASSIMP_DOUBLE_PRECISION"]    = "OFF"	# All data will be stored as double values.
		cmake.definitions["ASSIMP_OPT_BUILD_PACKAGES"]  = "OFF"	# Set to ON to generate CPack configuration files and packaging targets
		cmake.definitions["ASSIMP_ANDROID_JNIIOSYSTEM"] = "OFF"	# Android JNI IOSystem support is active
		cmake.definitions["ASSIMP_NO_EXPORT"]           = "OFF"	# Disable Assimp's export functionality
		cmake.definitions["ASSIMP_BUILD_ZLIB"]          = "OFF"	# Build your own zlib
		cmake.definitions["ASSIMP_BUILD_ASSIMP_TOOLS"]  = "OFF"	# If the supplementary tools for Assimp are built in addition to the library.
		cmake.definitions["ASSIMP_BUILD_SAMPLES"]       = "OFF"	# If the official samples are built as well (needs Glut).
		cmake.definitions["ASSIMP_BUILD_TESTS"]         = "OFF"	# If the test suite for Assimp is built in addition to the library.
		cmake.definitions["ASSIMP_COVERALLS"]           = "OFF"	# Enable this to measure test coverage.
		cmake.definitions["ASSIMP_ERROR_MAX"]           = "OFF"	# Enable all warnings.
		cmake.definitions["ASSIMP_WERROR"]              = "OFF"	# Treat warnings as errors.
		cmake.definitions["ASSIMP_ASAN"]                = "OFF"	# Enable AddressSanitizer.
		cmake.definitions["ASSIMP_UBSAN"]               = "OFF"	# Enable Undefined Behavior sanitizer.
		cmake.definitions["SYSTEM_IRRXML"]              = "OFF"	# Use system installed Irrlicht/IrrXML library.
		cmake.definitions["BUILD_DOCS"]                 = "OFF"	# Build documentation using Doxygen.
		cmake.definitions["INJECT_DEBUG_POSTFIX"]       = "ON"	# Inject debug postfix in .a/.so lib names
		cmake.definitions["IGNORE_GIT_HASH"]            = "ON"	# Don't call git to get the hash.
		cmake.definitions["ASSIMP_INSTALL_PDB"]         = "ON"	# Install MSVC debug files.
		
		cmake.configure()
		cmake.build()

	def package(self):
		self.copy("*", src="include", dst="include", keep_path=True)
		self.copy("*.lib", src="", dst="lib", keep_path=False)
		self.copy("*.a", src="", dst="lib", keep_path=False)

	def package_info(self):
		self.cpp_info.libs = tools.collect_libs(self)
