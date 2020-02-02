from conans import ConanFile, tools, errors

class Recipe(ConanFile):
	name = "glm"
	description = "OpenGL Mathematics (GLM): A C++ mathematics library for graphics programming."
	license = "Happy Bunny, MIT, https://glm.g-truc.net/copying.txt"
	homepage = "https://glm.g-truc.net"
	url = "none"
	topics = ("glm", "opengl", "math")

	def source(self):
		# TODO: change to use git
		tools.get(f"https://github.com/g-truc/glm/archive/{self.version}.zip")

	def build(self):
		pass

	def package(self):
		self.copy("*",
			src=f"glm-{self.version}/glm",
			dst="include/glm",
			keep_path=True,
			excludes="*.txt"
		)

	def package_id(self):
		self.info.header_only()

	def package_info(self):
		pass
