from conan import ConanFile, tools
import os

class Recipe(ConanFile):
	name = "meta"
	description = "A header only C++ library that provides type manipulation utilities."
	homepage = "https://github.com/dnlj/Meta"

	def source(self):
		tools.scm.Git(self).fetch_commit(
			url="https://github.com/dnlj/Meta.git",
			commit=self.version,
		)

	def package(self):
		tools.files.copy(self, "*.hpp",
			src=os.path.join(self.source_folder, "include"),
			dst=os.path.join(self.package_folder, "include"),
			keep_path=True
		)

	def package_id(self):
		self.info.clear()
