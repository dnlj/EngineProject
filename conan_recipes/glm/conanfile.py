from conan import ConanFile, tools
import os

class Recipe(ConanFile):
	name = "glm"
	description = "OpenGL Mathematics (GLM): A C++ mathematics library for graphics programming."
	license = "Happy Bunny, MIT, https://glm.g-truc.net/copying.txt"
	homepage = "https://glm.g-truc.net"

	def source(self):
		# TODO: change to use git
		tools.files.get(self, url=f"https://github.com/g-truc/glm/archive/{self.version}.zip")

	def package(self):
		tools.files.copy(self, "*",
			src=os.path.join(self.source_folder, f"glm-{self.version}/glm"),
			dst=os.path.join(self.package_folder, "include/glm"),
			keep_path=True,
			excludes="*.txt"
		)

	def package_id(self):
		self.info.clear()
