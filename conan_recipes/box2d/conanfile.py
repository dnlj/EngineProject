from conans import ConanFile, tools, errors, CMake
import os
import shutil

class Recipe(ConanFile):
	name = "box2d"
	description = "A 2D Physics Engine for Games."
	license = "MIT"
	homepage = "https://box2d.org/"
	url = "none"
	settings = "arch", "build_type", "compiler"
	
	def isLegacy(self):
		v = self.version.split(".")
		return len(v) == 3 and v[0] == "2" and v[1] == "3" and v[2] <= "1"
	
	def source(self):
		tools.Git().clone(
			url="https://github.com/erincatto/box2d.git",
			branch=("v" if self.isLegacy() else "") + self.version,
			shallow=True
		)
		
		if self.isLegacy():
			os.rename("Box2D", "Box2d_old")
			with tools.chdir("Box2D_old"):
				for name in os.listdir():
					shutil.move(name, self.source_folder)
			shutil.rmtree("Contributions")
			shutil.rmtree("Box2d_old")
			
		
	def build(self):
		cmake = CMake(self)
		cmake.definitions["BOX2D_BUILD_EXAMPLES"] = False
		cmake.definitions["BOX2D_BUILD_SAMPLES"] = False
		cmake.definitions["BOX2D_BUILD_TESTS"] = False
		cmake.definitions["BOX2D_BUILD_DOCS"] = False
		cmake.configure()
		cmake.build()
		
	def package(self):
		self.copy("*.h", src="Box2D", dst="include/box2d", keep_path=True)
		self.copy("*.h", src="include", dst="include", keep_path=True)
		self.copy("*.lib", src="", dst="lib", keep_path=False)
		self.copy("*.a", src="", dst="lib", keep_path=False)
		
	def package_info(self):
		self.cpp_info.libs = tools.collect_libs(self)
