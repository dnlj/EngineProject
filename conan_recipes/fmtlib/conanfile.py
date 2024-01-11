from conan import ConanFile, tools
import os

class Recipe(ConanFile):
	name = "fmtlib"
	description = "A modern formatting library."
	license = "MIT"
	homepage = "https://fmt.dev"
	url = "https://github.com/fmtlib/fmt.git"
	settings = "arch", "build_type", "compiler", "os"

	def source(self):
		tools.scm.Git(self).fetch_commit(
			url=self.url,
			commit=self.version,
		)

	def generate(self):
		tc = tools.cmake.CMakeToolchain(self)
		tc.variables["FMT_DOC"] = False
		tc.variables["FMT_INSTALL"] = False
		tc.variables["FMT_TEST"] = False
		tc.variables["FMT_FUZZ"] = False
		tc.variables["FMT_CUDA_TEST"] = False
		tc.variables["FMT_OS"] = True
		tc.variables["FMT_MODULE"] = False
		tc.generate()

	def build(self):
		cmake = tools.cmake.CMake(self)
		cmake.configure()
		cmake.build()

	def package(self):
		tools.files.copy(self, "*.h", src=os.path.join(self.source_folder, "include"), dst=os.path.join(self.package_folder, "include"), keep_path=True)
		tools.files.copy(self, "*.lib", src=self.source_folder, dst=os.path.join(self.package_folder, "lib"), keep_path=False)
		tools.files.copy(self, "*.pdb", src=self.source_folder, dst=os.path.join(self.package_folder, "lib"), keep_path=False)
		tools.files.copy(self, "*.a", src=self.source_folder, dst=os.path.join(self.package_folder, "lib"), keep_path=False)

	def package_info(self):
		self.cpp_info.libs = tools.files.collect_libs(self)
