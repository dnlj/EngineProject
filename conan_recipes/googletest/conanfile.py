from conan import ConanFile, tools
import os
import shutil

class Recipe(ConanFile):
	name = "googletest"
	description = "Google Testing and Mocking Framework"
	license = "BSD-3-Clause"
	homepage = "https://google.github.io/googletest/"
	url = "https://github.com/google/googletest.git"
	settings = "arch", "build_type", "compiler", "os"

	def source(self):
		tools.scm.Git(self).fetch_commit(
			url=self.url,
			commit="v" + self.version
		)

	def generate(self):
		tc = tools.cmake.CMakeToolchain(self)
		# This recipe is for Google test, not Google mock. Use the separate "googlemock" recipe if you want Google mock.
		tc.variables["BUILD_GMOCK"] = "OFF"
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

