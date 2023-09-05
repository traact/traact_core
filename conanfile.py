# /usr/bin/python3
import os
from conan import ConanFile
from conan.tools.build import can_run

class TraactPackage(ConanFile):
    python_requires = "traact_base/0.0.0@traact/latest"
    python_requires_extend = "traact_base.TraactPackageCmake"
    
    name = "traact_core"
    version = "0.0.0"
    description = "Core library of traact, defining the dataflow and component interfaces"
    url = "https://github.com/traact/traact_core.git"
    license = "MIT"
    author = "Frieder Pankratz"

    settings = "os", "compiler", "build_type", "arch"
    compiler = "cppstd"

    exports_sources = "src/*", "include/*", "tests/*", "apps/*", "CMakeLists.txt"
    
    options = {
        "shared": [True, False],
        "with_tests": [True, False],
        "trace_logs_in_release": [True, False],
        "with_cuda": [True, False]
    }

    default_options = {
        "shared": True,
        "with_tests": True,
        "trace_logs_in_release": True,
        "with_cuda" : True
    }    

    def requirements(self):        
        self.requires("traact_base/0.0.0@traact/latest")
        if self.options.with_tests:
            self.requires("gtest/1.14.0", transitive_headers=True, transitive_libs=True)
            self.requires("approvaltests.cpp/10.12.2", transitive_headers=True, transitive_libs=True)
        # traact core
        self.requires("nlohmann_json/[>=3.11.2]", transitive_headers=True)
        self.requires("spdlog/1.11.0", transitive_headers=True, transitive_libs=True)
        #self.requires("fmt/10.0.0")
        self.requires("fmt/9.1.0", override=True)
        self.requires("rttr/0.9.7-dev@camposs/stable", transitive_headers=True, transitive_libs=True)
        self.requires("taskflow/3.4.0")
        self.requires("tclap/[>=1.2.4]")
        self.requires("cppfs/1.3.0@camposs/stable")
        self.requires("re2/20230801")
        #self.requires("magic_enum/0.8.0")
    
    def configure(self):
        #self.options['spdlog'].shared = self.options.shared
        pass

    def _after_package_info(self):
        self.cpp_info.libs = ["traact_core"]

    def test(self):
        if can_run(self):
            cmd = os.path.join(self.cpp.build.bindir, "tests/core_tests")
            self.run(cmd, env="conanrun")