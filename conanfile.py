import os

from conan import ConanFile
from conan.tools.files import copy, rmdir, load
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout


class IsWireConan(ConanFile):
    name = "is-wire"
    version = "1.1.5"
    license = "MIT"
    url = "https://github.com/labvisio/is-wire"
    description = "C++ implementation of the ::is messaging layer"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "build_tests": [True, False],
        "build_benchmarks": [True, False]
    }
    default_options = {
        "shared": True,
        "fPIC": True,
        "build_tests": False,
        "build_benchmarks": False,
    }

    exports_sources = "*"

    def requirements(self):
        self.requires("boost/1.80.0")
        self.requires("spdlog/1.11.0")
        self.requires("protobuf/3.20.0")
        self.requires("prometheus-cpp/0.4.1@is/stable")
        self.requires("opentracing-cpp/1.4.0@is/stable")
        self.requires("simpleamqpclient/2.5.0@is/stable")
        if self.options.build_benchmarks:
            self.requires("benchmark/1.4.1@is/stable")
        if self.options.build_tests:
            self.requires("gtest/1.10.0")
            self.requires("zipkin-cpp-opentracing/0.3.1@is/stable")

    def build_requirements(self):
        self.tool_requires("protobuf/3.20.0")

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")
        self.options["simpleamqpclient"].shared = self.options.shared
        self.options["opentracing-cpp"].shared = self.options.shared 
        self.options["prometheus-cpp"].shared = self.options.shared 
        self.options["protobuf"].shared = self.options.shared 
        self.options["boost"].shared = self.options.shared

    def generate(self):
        tc = CMakeToolchain(self)
        tc.cache_variables["build_tests"] = self.options.build_tests
        tc.cache_variables["build_benchmarks"] = self.options.build_benchmarks
        tc.generate()
        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        if self.options.build_tests:
            cmake.test()

    def package(self):
        copy(self, "LICENSE.txt", src=self.source_folder, dst=os.path.join(self.package_folder, "licenses"))
        cmake = CMake(self)
        cmake.install()
        rmdir(self, os.path.join(self.package_folder, "lib", "pkgconfig"))
        rmdir(self, os.path.join(self.package_folder, "cmake"))

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "is-wire")
        self.cpp_info.set_property("cmake_target_name", "is-wire::is-wire")
        self.cpp_info.set_property("pkg_config_name", "is-wire")
        self.cpp_info.components["is-wire"].libs = ["is-wire-core", "is-wire-rpc"]
        self.cpp_info.components["is-wire"].requires =[
            "opentracing-cpp::opentracing-cpp",
            "simpleamqpclient::simpleamqpclient",
            "prometheus-cpp::prometheus-cpp",
            "spdlog::spdlog",
            "protobuf::protobuf",
            "boost::boost",
        ]
        if self.options.build_tests:
            self.cpp_info.components["is-msgs"].requires.extend([
                "gtest::gtest",
                "zipkin-cpp-opentracing::zipkin-cpp-opentracing",
            ])
        if self.options.build_benchmarks:
            self.cpp_info.components["is-msgs"].requires.extend([
                "benchmark::benchmark",
            ])