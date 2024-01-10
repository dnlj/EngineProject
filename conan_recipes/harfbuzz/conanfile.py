from conan import ConanFile, tools
import os

class Recipe(ConanFile):
	name = "harfbuzz"
	description = "HarfBuzz text shaping engine."
	license = "MIT"
	homepage = "https://github.com/harfbuzz/harfbuzz"
	settings = "arch", "build_type", "compiler", "os"
	options = {
		"with_freetype": [True, False] # TODO: potential issues https://github.com/conan-io/conan/issues/3620
	}

	def source(self):
		tools.scm.Git(self).fetch_commit(
			url="https://github.com/harfbuzz/harfbuzz.git",
			commit=self.version,
		)

	def requirements(self):
		if self.options.with_freetype:
			self.requires("freetype/[~2]")

	def layout(self):
		tools.cmake.cmake_layout(self)

	def generate(self):
		tc = tools.cmake.CMakeToolchain(self)
		tc.variables["HB_HAVE_FREETYPE"] = self.options.with_freetype
		tc.variables["HB_BUILD_SUBSET"] = False
		#tc.variables["CMAKE_PREFIX_PATH"] = ";".join(self.deps_cpp_info.builddirs).replace("\\", "/")
		tc.generate()

	def build(self):
		cmake = tools.cmake.CMake(self)
		cmake.configure()
		cmake.build()

	def package(self):
		cmake = tools.cmake.CMake(self)
		cmake.install()

	def package_info(self):
		self.cpp_info.libs = tools.files.collect_libs(self)
		self.cpp_info.includedirs = [os.path.join("include", "harfbuzz")]
