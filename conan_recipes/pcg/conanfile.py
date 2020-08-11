from conans import ConanFile, tools, errors

class Recipe(ConanFile):
	name = "pcg"
	description = "PCG is a family of simple fast space-efficient statistically good algorithms for random number generation. Unlike many general-purpose RNGs, they are also hard to predict."
	license = "MIT, Apache-2.0"
	homepage = "https://www.pcg-random.org/"
	url = "none"

	def source(self):
		isver = self.version[0].isdigit()
		tools.Git().clone(
			url="https://github.com/imneme/pcg-cpp.git",
			branch=("v" if isver else "") + self.version,
			shallow=True
		)

	def build(self):
		pass

	def package(self):
		self.copy("*", src="include", dst="include", keep_path=True)

	def package_id(self):
		self.info.header_only()

	def package_info(self):
		pass
