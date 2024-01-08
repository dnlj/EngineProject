from conan import ConanFile, tools
import os

cmakedata = """
cmake_minimum_required(VERSION 3.16)
project(soil_littlstar)
file(GLOB_RECURSE SOURCES *.cpp *.c *.h *.hpp)
add_library(soil_littlstar STATIC ${SOURCES})
include_directories(soil_littlstar include)
"""

class Recipe(ConanFile):
	name = "soil_littlstar"
	description = "A tiny C library used primarily for uploading textures into OpenGL."
	license = "Public Domain"
	homepage = "https://github.com/littlstar/soil"
	settings = "arch", "build_type", "compiler"

	def source(self):
		tools.scm.Git(self).fetch_commit(
			url="https://github.com/littlstar/soil.git",
			commit=self.version,
		)

		with open("CMakeLists.txt", "w") as f:
			f.write(cmakedata)

	def generate(self):
		tc = tools.cmake.CMakeToolchain(self)
		tc.generate()

	def build(self):
		cmake = tools.cmake.CMake(self)
		cmake.configure()
		cmake.build()

	def package(self):
		tools.files.copy(self, "*.h",
			src=self.source_folder,
			dst=os.path.join(self.package_folder, "include/soil"),
			keep_path=False
		)
		tools.files.copy(self, "*.lib",
			src=self.source_folder,
			dst=os.path.join(self.package_folder,"lib"),
			keep_path=False
		)
		tools.files.copy(self, "*.a",
			src=self.source_folder,
			dst=os.path.join(self.package_folder,"lib"),
			keep_path=False
		)

	def package_info(self):
		self.cpp_info.libs = tools.files.collect_libs(self)
