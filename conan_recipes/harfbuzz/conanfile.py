from conans import ConanFile, tools, errors, CMake
import os

class Recipe(ConanFile):
	name = "harfbuzz"
	description = "HarfBuzz text shaping engine."
	license = "MIT"
	homepage = "https://github.com/harfbuzz/harfbuzz"
	url = "none"
	settings = "arch", "build_type", "compiler"
	options = {
		"with_freetype": [True, False] # TODO: potential issues https://github.com/conan-io/conan/issues/3620
	}
	
	def source(self):
		tools.Git().clone(
			url="https://github.com/harfbuzz/harfbuzz.git",
			branch=self.version,
			shallow=True
		)
	
	def requirements(self):
		if self.options.with_freetype:
			self.requires("freetype/master")
		
	def build(self):
		cmake = CMake(self)
		cmake.definitions["HB_HAVE_FREETYPE"] = self.options.with_freetype
		cmake.definitions["HB_BUILD_SUBSET"] = False
		cmake.definitions["CMAKE_PREFIX_PATH"] = ";".join(self.deps_cpp_info.builddirs).replace("\\", "/")
		cmake.configure(build_folder="build")
		cmake.build()
		
	def package(self):
		self.copy("*.h", src="src", dst="include", keep_path=True)
		self.copy("*.lib", src="", dst="lib", keep_path=False)
		self.copy("*.a", src="", dst="lib", keep_path=False)
		
	def package_info(self):
		self.cpp_info.libs = tools.collect_libs(self)
