from conans import ConanFile, tools, errors

class Recipe(ConanFile):
	name = "robin_hood"
	description = "A fast & memory efficient hashtable based on robin hood hashing."
	license = "MIT"
	homepage = "https://github.com/martinus/robin-hood-hashing"
	url = "none"
	topics = ()

	def source(self):
		tools.Git().clone(
			url="https://github.com/martinus/robin-hood-hashing.git",
			branch=self.version,
			shallow=True
		)

	def build(self):
		pass

	def package(self):
		self.copy("*.h", src="src/include", dst="include", keep_path=True)

	def package_id(self):
		self.info.header_only()

	def package_info(self):
		pass
