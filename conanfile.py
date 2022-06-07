# /usr/bin/python3
import os
from conans import ConanFile, CMake, tools


class Traact(ConanFile):
    name = "traact_core"
    version = "0.1.0"

    description = "Core library of traact, defining the dataflow and component interfaces"
    url = "https://github.com/traact/traact_core"
    license = "MIT"
    author = "Frieder Pankratz"

    generators = "cmake", "TraactVirtualRunEnvGenerator"
    settings = "os", "compiler", "build_type", "arch"
    compiler = "cppstd"
    options = {
        "shared": [True, False],
        "with_tests": [True, False]
    }

    default_options = {
        "shared": True,
        "with_tests": True
    }

    exports_sources =  "src/*", "util/*", "apps/*", "tests/*", "CMakeLists.txt"

    def requirements(self):
        if self.options.with_tests:
            self.requires("gtest/[>=1.10.0]")
            self.requires("approvaltests.cpp/10.12.2")
        self.requires("traact_run_env/[>=1.0.0]@camposs/stable")
        self.requires("nlohmann_json/[>=3.7.3]")
        self.requires("spdlog/[>=1.8.2]")
        self.requires("rttr/0.9.7-dev@camposs/stable")
        self.requires("taskflow/3.3.0")
        self.requires("tclap/[>=1.2.4]")
        self.requires("cppfs/1.3.0@camposs/stable")

    def _configure_cmake(self):
        cmake = CMake(self)
        cmake.verbose = True

        def add_cmake_option(option, value):
            var_name = "{}".format(option).upper()
            value_str = "{}".format(value)
            var_value = "ON" if value_str == 'True' else "OFF" if value_str == 'False' else value_str
            cmake.definitions[var_name] = var_value

        for option, value in self.options.items():
            add_cmake_option(option, value)

        cmake.configure()
        return cmake

    def configure(self):
        if self.options.with_tests:
            self.options['gtest'].shared = self.options.shared
        self.options['Boost'].shared = self.options.shared

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = [self.name]
        # self.cpp_info.libs = tools.collect_libs(self)
