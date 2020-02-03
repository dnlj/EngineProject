from conans import ConanFile, tools, errors

class Recipe(ConanFile):
	name = "meta"
	description = "A header only C++ library that provides type manipulation utilities."
	license = ""
	homepage = "https://github.com/dnlj/Meta"
	url = "none"
	topics = ()

	def source(self):
		tools.Git().clone(
			url="https://github.com/dnlj/Meta.git",
			branch=self.version,
			shallow=True
		)

	def build(self):
		pass

	def package(self):
		self.copy("*.hpp", src="include", dst="include", keep_path=True)

	def package_id(self):
		self.info.header_only()

	def package_info(self):
		pass
