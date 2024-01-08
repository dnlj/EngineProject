from conan import ConanFile, tools
import os

class Recipe(ConanFile):
	name = "pcg"
	description = "PCG is a family of simple fast space-efficient statistically good algorithms for random number generation. Unlike many general-purpose RNGs, they are also hard to predict."
	license = "MIT, Apache-2.0"
	homepage = "https://www.pcg-random.org/"

	def source(self):
		isver = self.version[0].isdigit()
		tools.scm.Git(self).fetch_commit(
			url="https://github.com/imneme/pcg-cpp.git",
			commit=("v" if isver else "") + self.version,
		)

	def package(self):
		tools.files.copy(self, "*",
			src=os.path.join(self.source_folder, "include"),
			dst=os.path.join(self.package_folder, "include"),
			keep_path=True
		)

	def package_id(self):
		self.info.clear()
