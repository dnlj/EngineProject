from conan import ConanFile, tools

class Recipe(ConanFile):
	name = "assimp"
	description = "The Asset-Importer-Lib (in short assimp) is a library to load and process geometric scenes from various 3D-Dataformats."
	license = "BSD 3-clause"
	homepage = "https://www.assimp.org"
	url = "https://github.com/assimp/assimp.git"
	settings = "arch", "build_type", "compiler", "os"

	def source(self):
		isver = self.version[0].isdigit()
		tools.scm.Git(self).fetch_commit(
			url=self.url,
			commit=("v" if isver else "") + self.version,
		)

	def generate(self):
		tc = tools.cmake.CMakeToolchain(self)
		# TODO: I think a lot of these are out of date/version dependant, Probably assimp 4?
		tc.variables["BUILD_SHARED_LIBS"]          = "OFF"	# Generation of shared libs ( dll for windows, so for Linux ). Set this to OFF to get a static lib.
		tc.variables["BUILD_FRAMEWORK"]            = "OFF"	# Mac only. Build package as Mac OS X Framework bundle
		tc.variables["ASSIMP_DOUBLE_PRECISION"]    = "OFF"	# All data will be stored as double values.
		tc.variables["ASSIMP_OPT_BUILD_PACKAGES"]  = "OFF"	# Set to ON to generate CPack configuration files and packaging targets
		tc.variables["ASSIMP_ANDROID_JNIIOSYSTEM"] = "OFF"	# Android JNI IOSystem support is active
		tc.variables["ASSIMP_NO_EXPORT"]           = "OFF"	# Disable Assimp's export functionality
		tc.variables["ASSIMP_BUILD_ZLIB"]          = "OFF"	# Build your own zlib
		tc.variables["ASSIMP_BUILD_ASSIMP_TOOLS"]  = "OFF"	# If the supplementary tools for Assimp are built in addition to the library.
		tc.variables["ASSIMP_BUILD_SAMPLES"]       = "OFF"	# If the official samples are built as well (needs Glut).
		tc.variables["ASSIMP_BUILD_TESTS"]         = "OFF"	# If the test suite for Assimp is built in addition to the library.
		tc.variables["ASSIMP_COVERALLS"]           = "OFF"	# Enable this to measure test coverage.
		tc.variables["ASSIMP_ERROR_MAX"]           = "OFF"	# Enable all warnings.
		tc.variables["ASSIMP_WERROR"]              = "OFF"	# Treat warnings as errors.
		tc.variables["ASSIMP_ASAN"]                = "OFF"	# Enable AddressSanitizer.
		tc.variables["ASSIMP_UBSAN"]               = "OFF"	# Enable Undefined Behavior sanitizer.
		tc.variables["SYSTEM_IRRXML"]              = "OFF"	# Use system installed Irrlicht/IrrXML library.
		tc.variables["BUILD_DOCS"]                 = "OFF"	# Build documentation using Doxygen.
		tc.variables["INJECT_DEBUG_POSTFIX"]       = "ON"	# Inject debug postfix in .a/.so lib names
		tc.variables["IGNORE_GIT_HASH"]            = "ON"	# Don't call git to get the hash.
		tc.variables["ASSIMP_INSTALL_PDB"]         = "ON"	# Install MSVC debug files.
		tc.generate()

	def build(self):
		cmake = tools.cmake.CMake(self)
		cmake.configure()
		cmake.build()

	def package(self):
		tools.cmake.CMake(self).install()

	def package_info(self):
		self.cpp_info.libs = tools.files.collect_libs(self)
