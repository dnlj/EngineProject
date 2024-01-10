from conan import ConanFile, tools
import os

class Recipe(ConanFile):
	name = "freetype"
	description = "A freely available software library to render fonts."
	license = "FTL, GPLv2"
	homepage = "https://www.freetype.org/"
	settings = "arch", "build_type", "compiler", "os"

	def source(self):
		tools.scm.Git(self).fetch_commit(
			url="https://gitlab.freedesktop.org/freetype/freetype.git",
			commit="VER-" + self.version.replace(".", "-"),
		)

	def layout(self):
		tools.cmake.cmake_layout(self)

	def generate(self):
		tc = tools.cmake.CMakeToolchain(self)
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
		self.cpp_info.includedirs = [os.path.join("include", "freetype2")]
