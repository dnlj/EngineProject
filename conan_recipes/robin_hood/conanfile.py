from conan import ConanFile, tools
import os

class Recipe(ConanFile):
	name = "robin_hood"
	description = "A fast & memory efficient hashtable based on robin hood hashing."
	license = "MIT"
	homepage = "https://github.com/martinus/robin-hood-hashing"

	def source(self):
		tools.scm.Git(self).fetch_commit(
			url="https://github.com/martinus/robin-hood-hashing.git",
			commit=self.version,
		)

	def package(self):
		tools.files.copy(self, "*.h",
			src=os.path.join(self.source_folder, "src/include"),
			dst=os.path.join(self.package_folder, "include"),
			keep_path=True
		)

	def package_id(self):
		self.info.clear()
