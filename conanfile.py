from conan import ConanFile
from conan.tools.build import check_min_cppstd
from conan.tools.files import load, copy, collect_libs
from conan.tools.cmake import cmake_layout, CMake, CMakeToolchain
import re
import os


class PackageConan(ConanFile):
    # @+ START USER META CONFIG
    license = "Apache 2.0"
    description = "OPC UA Server Technology Adapter implementation with open62541"
    topics = ('stag', 'dca', 'open62541')
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False],
               "fPIC": [True, False],
               "historization": [True, False]}
    default_options = {"shared": True,
                       "fPIC": True,
                       "historization": True}
    default_user = "Hahn-Schickard"
    # @- END USER META CONFIG
    exports = [
        "CMakeLists.txt",
        "conanfile.py"
    ]
    exports_sources = [
        "cmake*",
        "includes*",
        "sources*",
        "CMakeLists.txt",
        "conanfile.py",
        # @+ START USER EXPORTS
        'config*',
        # @- END USER EXPORTS
    ]
    generators = "CMakeDeps"
    package_type = "library"
    short_paths = True

    @property
    def cwd(self):
        return os.path.dirname(os.path.realpath(__file__))

    @property
    def full_name(self):
        content = load(self, path=os.path.join(
            self.recipe_folder, 'CMakeLists.txt'))
        return re.search('set\(THIS (.*)\)', content).group(1).strip()

    def set_name(self):
        self.name = self.full_name.lower()

    def validate(self):
        if self.settings.compiler.get_safe("cppstd"):
            check_min_cppstd(self, "17")

    def requirements(self):
        # @+ START USER REQUIREMENTS
        self.requires(
            "data_consumer_adapter_interface/[~0.4]@hahn-schickard/stable")
        self.requires("variant_visitor/[~0.2]@hahn-schickard/stable",
                      visible=False
                      )
        self.requires("nlohmann_json/[~3.12]",
                      visible=False
                      )
        self.requires("open62541/[~1.4]",
                      visible=False
                      )
        if self.options.historization:
            self.requires("date/[~3.0]",
                          visible=False
                          )
            self.requires("soci/[~4.1]",
                          visible=False
                          )
        # @- END USER REQUIREMENTS

    def build_requirements(self):
        self.test_requires(
            "information_model_mocks/[~0.1]@hahn-schickard/stable")
        # @+ START USER BUILD REQUIREMENTS
        # @- END USER BUILD REQUIREMENTS

    def configure(self):
        # @+ START USER REQUIREMENTS OPTION CONFIGURATION
        self.options["gtest/*"].shared = True
        self.options["open62541"].shared = False
        self.options["open62541"].cpp_compatible = True
        self.options["open62541"].parsing = True
        self.options["open62541"].json_support = True
        self.options["open62541"].multithreading = "Threadsafe"
        if self.options.historization:
            self.options["soci"].shared = False
            self.options["soci"].with_postgresql = True
            self.options["date"].header_only = True
            self.options["open62541"].historize = True

        # @- END USER REQUIREMENTS OPTION CONFIGURATION

    def layout(self):
        cmake_layout(self)

    def config_options(self):
        if self.settings.os == 'Windows':
            del self.options.fPIC

    def generate(self):
        tc = CMakeToolchain(self)
        tc.user_presets_path = False
        tc.variables['STATIC_CODE_ANALYSIS'] = False
        tc.variables['RUN_TESTS'] = False
        tc.variables['COVERAGE_TRACKING'] = False
        tc.variables['CMAKE_CONAN'] = False
        tc.variables['HISTORIZATION'] = self.options.historization
        # @+ START USER CMAKE OPTIONS
        # @- END USER CMAKE OPTIONS
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
        copy(self, pattern='LICENSE', dst='licenses', src=self.cwd)
        copy(self, pattern='NOTICE', dst='licenses', src=self.cwd)
        copy(self, pattern='AUTHORS', dst='licenses', src=self.cwd)

    def package_info(self):
        self.cpp_info.libs = collect_libs(self)
        self.cpp_info.set_property("cmake_find_mode", "both")
        # @+ START USER DEFINES
        # @- END USER DEFINES
        self.cpp_info.set_property("cmake_file_name", self.full_name)
        cmake_target_name = self.full_name + "::" + self.full_name
        self.cpp_info.set_property("cmake_target_name", cmake_target_name)
