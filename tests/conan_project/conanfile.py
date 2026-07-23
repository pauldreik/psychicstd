from conan import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout


class PsychicstdConanExample(ConanFile):
    name = "psychicstd_conan_example"
    version = "0.1"
    settings = "os", "compiler", "build_type", "arch"
    requires = "fmt/11.1.4"
    exports_sources = "CMakeLists.txt", "main.cpp"
    no_copy_source = True

    def layout(self):
        cmake_layout(self)

    def generate(self):
        toolchain = CMakeToolchain(self)
        toolchain.user_presets_path = False
        toolchain.generate()
        CMakeDeps(self).generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
