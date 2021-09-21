from conans import ConanFile, tools, errors, CMake
import os

class Recipe(ConanFile):
	name = "fmtlib"
	description = "A modern formatting library."
	license = "MIT"
	homepage = "https://fmt.dev"
	url = "https://github.com/fmtlib/fmt.git"
	settings = "arch", "build_type", "compiler"

	def source(self):
		tools.Git().clone(
			url=self.url,
			branch=self.version,
			shallow=True
		)

	def build(self):
		cmake = CMake(self)

		cmake.definitions["FMT_DOC"] = False
		cmake.definitions["FMT_INSTALL"] = False
		cmake.definitions["FMT_TEST"] = False
		cmake.definitions["FMT_FUZZ"] = False
		cmake.definitions["FMT_CUDA_TEST"] = False
		cmake.definitions["FMT_OS"] = True
		cmake.definitions["FMT_MODULE"] = False

		cmake.configure()
		cmake.build()

	def package(self):
		self.copy("*.h", src="include", dst="include", keep_path=True)
		self.copy("*.lib", src="", dst="lib", keep_path=False)
		self.copy("*.a", src="", dst="lib", keep_path=False)

	def package_info(self):
		self.cpp_info.libs = tools.collect_libs(self)
