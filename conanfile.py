from conan import ConanFile
from conan.tools.files import load, collect_libs
from conan.tools.cmake import cmake_layout, CMake, CMakeToolchain, CMakeDeps
import re
import os


class PackageConan(ConanFile):
    license = "Apache 2.0"
    topics = ('stag', 'dca', 'open62541')
    build_requires = 'gtest/[~1.11]'
    requires = [
        "nlohmann_json/3.11.1",
        "open62541/1.3.4",
        "date/3.0.1",
        "HaSLL/[~0.3]@hahn-schickard/stable",
        "HSCUL/[~0.3]@hahn-schickard/stable",
        "Variant_Visitor/[~0.1]@hahn-schickard/stable",
        "Data_Consumer_Adapter_Interface/[~0.1]@hahn-schickard/stable"
    ]
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False],
               "fPIC": [True, False],
               "historization": [True, False]}
    default_options = {"open62541:cpp_compatible": True,
                       "open62541:multithreading": "Threadsafe",
                       "shared": True,
                       "fPIC": True,
                       "historization": False}
    default_user = "Hahn-Schickard"
    exports_sources = [
        "cmake*",
        'config*',
        "includes*",
        "sources*",
        "unit_tests*",
        "CMakeLists.txt",
        "conanfile.py",
        'README.md',
        "LICENSE",
        "NOTICE",
        "AUTHORS"
    ]
    short_paths = True

    def layout(self):
        cmake_layout(self)

    def requirements(self):
        if self.options.historization:
            self.requires("OODD/[~0.1]@hahn-schickard/stable")
            self.options["open62541"].historize = True

    @property
    def cwd(self):
        return os.path.dirname(os.path.realpath(__file__))

    def set_name(self):
        content = load(os.path.join(self.cwd, 'CMakeLists.txt'))
        name = re.search('set\(THIS (.*)\)', content).group(1)
        self.name = name.strip()

    def config_options(self):
        if self.settings.os == 'Windows':
            del self.options.fPIC

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables['STATIC_CODE_ANALYSIS'] = False
        tc.variables['RUN_TESTS'] = False
        tc.variables['USE_CONAN'] = True
        tc.variables['HISTORIZATION'] = self.options.historization
        tc.cache_variables["CMAKE_POLICY_DEFAULT_CMP0077"] = "NEW"
        tc.generate()
        tc = CMakeDeps(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
        self.copy(pattern='LICENSE', dst='licenses', src=self.cwd)
        self.copy(pattern='NOTICE', dst='licenses', src=self.cwd)
        self.copy(pattern='AUTHORS', dst='licenses', src=self.cwd)

    def package_info(self):
        self.cpp_info.libs = collect_libs(self)
        self.output.info('Collected libs: \n{}'.format(
            '\n'.join(self.cpp_info.libs)))
