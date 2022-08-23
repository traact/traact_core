# /usr/bin/python3
import os
from conans import ConanFile, CMake, tools


class TraactPackage(ConanFile):
    python_requires = "traact_run_env/1.0.0@traact/latest"
    python_requires_extend = "traact_run_env.TraactPackageCmake"

    name = "traact_core"
    description = "Core library of traact, defining the dataflow and component interfaces"
    url = "https://github.com/traact/traact_core.git"
    license = "MIT"
    author = "Frieder Pankratz"

    settings = "os", "compiler", "build_type", "arch"
    compiler = "cppstd"

    exports_sources = "src/*", "util/*", "apps/*", "tests/*", "CMakeLists.txt"

    def requirements(self):
        self.traact_requires("traact_run_env", "latest")
        if self.options.with_tests:
            self.requires("gtest/[>=1.11.0]")
            self.requires("approvaltests.cpp/10.12.2")
        self.requires("nlohmann_json/[>=3.11.2]")
        self.requires("spdlog/1.10.0")
        self.requires("rttr/0.9.7-dev@camposs/stable")
        self.requires("taskflow/3.4.0")
        self.requires("tclap/[>=1.2.4]")
        self.requires("cppfs/1.3.0-r1@camposs/stable")
