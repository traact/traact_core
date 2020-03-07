# /usr/bin/python3
import os
from conans import ConanFile, CMake, tools


class Traact(ConanFile):
    name = "Traact"
    version = "0.0.1"
    default_user = "testuser"
    default_channel = "stable"

    description = "Traact library for realtime tracking frameworks"
    url = ""
    license = ""
    author = "Frieder Pankratz"

    short_paths = True

    generators = "cmake", "virtualenv"
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

    exports_sources = "include/*", "src/*", "util/*", "test/*", "CMakeLists.txt"

    def requirements(self):
        if self.options.with_tests:
            self.requires("gtest/1.10.0")
        # self.requires("opencv/4.1.1@conan/stable")
        # self.requires("eigen/3.3.7@conan/stable")
        self.requires("TBB/2019_U9@conan/stable")
        self.requires("eigen/3.3.7@camposs/stable")
        self.requires("rttr/0.9.7-dev@camposs/stable")
        self.requires("opencv/3.4.8@camposs/stable")
        self.requires("spdlog/1.4.2@camposs/stable")
        self.requires("Boost/1.70.0@camposs/stable")
        self.requires("nlohmann_json/3.7.3")

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
        self.options['opencv'].shared = self.options.shared
        self.options['TBB'].shared = self.options.shared
        self.options['gtest'].shared = self.options.shared
        self.options['Boost'].shared = self.options.shared

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = tools.collect_libs(self)
