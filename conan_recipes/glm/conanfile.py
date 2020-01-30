from conans import ConanFile, tools, errors

class Recipe(ConanFile):
	name = "glm"
	version = "0.9.9.7"
	description = "OpenGL Mathematics (GLM): A C++ mathematics library for graphics programming."
	license = "Happy Bunny, MIT, https://glm.g-truc.net/copying.txt"
	homepage = "https://glm.g-truc.net"
	url = "none"
	topics = ("glm", "opengl", "math")

	def source(self):
		tools.get("https://github.com/g-truc/glm/archive/{}.zip".format(self.version))

	def build(self):
		pass

	def package(self):
		self.copy("*",
			src="glm-{}/glm".format(self.version),
			dst="include/glm",
			keep_path=True,
			excludes="*.txt"
		)

	def package_id(self):
		self.info.header_only()

	def package_info(self):
		pass
