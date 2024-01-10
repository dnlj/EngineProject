from conan import ConanFile, tools
import os
import shutil

class Recipe(ConanFile):
	name = "box2d"
	description = "A 2D Physics Engine for Games."
	license = "MIT"
	homepage = "https://box2d.org/"
	settings = "arch", "build_type", "compiler", "os"

	def isLegacy(self):
		v = self.version.split(".")
		return len(v) == 3 and v[0] == "2" and v[1] == "3" and v[2] <= "1"

	def source(self):
		tools.scm.Git(self).fetch_commit(
			url="https://github.com/erincatto/box2d.git",
			commit=("v" if self.isLegacy() else "") + self.version
		)

		if self.isLegacy():
			os.rename("Box2D", "Box2d_old")
			with tools.chdir("Box2D_old"):
				for name in os.listdir():
					shutil.move(name, self.source_folder)
			shutil.rmtree("Contributions")
			shutil.rmtree("Box2d_old")

	def generate(self):
		tc = tools.cmake.CMakeToolchain(self)
		tc.variables["BOX2D_BUILD_EXAMPLES"] = False
		tc.variables["BOX2D_BUILD_SAMPLES"] = False
		tc.variables["BOX2D_BUILD_TESTS"] = False
		tc.variables["BOX2D_BUILD_DOCS"] = False
		tc.generate()

	def build(self):
		cmake = tools.cmake.CMake(self)
		cmake.configure()
		cmake.build()

	def package(self):
		tools.files.copy(self, "*.h", src=os.path.join(self.source_folder, "box2d"), dst=os.path.join(self.package_folder, "include/box2d"), keep_path=True)
		tools.files.copy(self, "*.h", src=os.path.join(self.source_folder, "include"), dst=os.path.join(self.package_folder, "include"), keep_path=True)
		tools.files.copy(self, "*.lib", src=self.source_folder, dst=os.path.join(self.package_folder, "lib"), keep_path=False)
		tools.files.copy(self, "*.a", src=self.source_folder, dst=os.path.join(self.package_folder, "lib"), keep_path=False)

	def package_info(self):
		self.cpp_info.libs = tools.files.collect_libs(self)

