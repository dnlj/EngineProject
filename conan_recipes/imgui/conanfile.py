from conans import ConanFile, tools, errors, CMake
import shutil

cmakedata = """
cmake_minimum_required(VERSION 3.16)
project(imgui)
file(GLOB SOURCES *.cpp *.h)
add_library(imgui STATIC ${SOURCES})
"""

class Recipe(ConanFile):
	name = "imgui"
	description = "Bloat-free Immediate Mode Graphical User interface for C++ with minimal dependencies."
	license = "MIT"
	homepage = "https://github.com/ocornut/imgui"
	url = "none"
	settings = "arch", "build_type", "compiler"
	
	def source(self):
		isver = self.version[0].isdigit()
		tools.Git().clone(
			url="https://github.com/ocornut/imgui.git",
			branch=("v" if isver else "") + self.version,
			shallow=True
		)
		
		shutil.rmtree(".github")
		shutil.rmtree("docs")
		shutil.rmtree("examples")
		shutil.rmtree("misc")
		
		with open("CMakeLists.txt", "w") as f:
			f.write(cmakedata)
		
	def build(self):
		cmake = CMake(self)
		cmake.configure()
		cmake.build()
	
	def package(self):
		self.copy("*.h", src="", dst="include", keep_path=True)
		self.copy("*.lib", src="", dst="lib", keep_path=False)
		self.copy("*.a", src="", dst="lib", keep_path=False)
		
	def package_info(self):
		self.cpp_info.libs = tools.collect_libs(self)
		
