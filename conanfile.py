# /usr/bin/python3
import os
from conans import ConanFile, CMake, tools


class Traact(ConanFile):
    name = "traact_core"
    version = "0.0.1"    

    description = "Convenience wrapper around tbb for realtime tracking in heterogeneous, distributed sensor environments"
    url = ""
    license = "BSD 3-Clause"
    author = "Frieder Pankratz"

    short_paths = True

    generators = "cmake", "traact_virtualrunenv_generator"
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

    exports_sources = "include/*", "src/*", "util/*", "tests/*", "CMakeLists.txt"

    def requirements(self):
        if self.options.with_tests:
            self.requires("gtest/1.10.0")
            self.requires("fakeit/2.0.7")
        self.requires("traact_run_env/%s@camposs/stable" % self.version)
        self.requires("nlohmann_json/3.7.3")        
        self.requires("spdlog/1.8.2")
        self.requires("tbb/2020.2")
        self.requires("rttr/0.9.7-dev@camposs/stable")
        self.requires("Boost/1.75.0@camposs/stable")
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
        #self.cpp_info.libs = tools.collect_libs(self)
