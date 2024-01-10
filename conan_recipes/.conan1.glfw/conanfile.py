from conans import ConanFile, tools, errors, CMake
import os

class Recipe(ConanFile):
	name = "glfw"
	description = "A multi-platform library for OpenGL, OpenGL ES, Vulkan, windows and input."
	license = "Zlib"
	homepage = "https://www.glfw.org/"
	url = "none"
	settings = "arch", "build_type", "compiler"
	
	def source(self):
		tools.Git().clone(
			url="https://github.com/glfw/glfw.git",
			branch=self.version,
			shallow=True
		)
		
	def build(self):
		cmake = CMake(self)
		cmake.definitions["GLFW_BUILD_EXAMPLES"] = False
		cmake.definitions["GLFW_BUILD_TESTS"] = False
		cmake.definitions["GLFW_BUILD_DOCS"] = False
		cmake.configure()
		cmake.build()
		
	def package(self):
		self.copy("*.h", src="include", dst="include", keep_path=True)
		self.copy("*.lib", src="", dst="lib", keep_path=False)
		self.copy("*.a", src="", dst="lib", keep_path=False)
		
	def package_info(self):
		self.cpp_info.libs = tools.collect_libs(self)
