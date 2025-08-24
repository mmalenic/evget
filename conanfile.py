from conan.tools.build import check_min_cppstd
from conan.tools.cmake import cmake_layout, CMakeToolchain, CMake
from conan import ConanFile


class EvgetRecipe(ConanFile):
    name = "evget"
    description = "record input device events"
    url = "https://github.com/mmalenic/evget"
    license = "MIT"
    author = "Marko Malenic (mmalenic1@gmail.com)"
    # x-release-please-start-version
    version = "0.1.0"
    # x-release-please-end

    requires = (
        "boost/[^1]",
        "spdlog/[^1]",
        "nlohmann_json/[^3]",
        "sqlitecpp/[^3]",
        "cli11/[^2]",
        "openssl/[^3]",
    )
    test_requires = "gtest/[^1]"

    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps"

    options = {
        # Build the binary CLI.
        "build_bin": [True, False],
        # Whether to build test executables.
        "build_testing": [True, False],
        # Whether to run clang tidy.
        "run_clang_tidy": [True, False],
        # An optional clang tidy executable.
        "clang_tidy_executable": [None, "ANY"],
        # Fix clang tidy errors by passing "--fix-errors" to clang-tidy.
        "clang_tidy_fix_errors": [True, False],
        # Specifies a compiler launcher, such as sccache, which sets CMAKE_<LANG>_COMPILER_LAUNCHER.
        "compiler_launcher": [None, "ANY"],
        # Add a cmake install command after for the binary target if `build_binary` is also true:
        # https://cmake.org/cmake/help/latest/command/install.html
        "install_bin": [True, False],
        # Add a cmake install command after for the library target:
        # https://cmake.org/cmake/help/latest/command/install.html
        "install_lib": [True, False],
        # Export a compilation database using CMAKE_EXPORT_COMPILE_COMMANDS:
        # https://cmake.org/cmake/help/latest/variable/CMAKE_EXPORT_COMPILE_COMMANDS.html
        "export_compilation_database": [True, False],
        # Verify headers by setting CMAKE_VERIFY_INTERFACE_HEADER_SETS:
        # https://cmake.org/cmake/help/latest/variable/CMAKE_VERIFY_INTERFACE_HEADER_SETS.html#variable:CMAKE_VERIFY_INTERFACE_HEADER_SETS
        "verify_headers": [True, False],
        "build_evgetx11": [True, False],
        "install_system_packages": [True, False],
    }
    default_options = {
        "build_bin": True,
        "build_testing": False,
        "run_clang_tidy": False,
        "clang_tidy_executable": None,
        "clang_tidy_fix_errors": False,
        "compiler_launcher": None,
        "install_bin": True,
        "install_lib": True,
        "export_compilation_database": True,
        "verify_headers": False,
        "build_evgetx11": True,
        "install_system_packages": True,
    }

    def requirements(self):
        if self.options.build_evgetx11:
            self.requires("xorg/system")

    def configure(self):
        self.options["spdlog"].use_std_fmt = True

    def validate(self):
        if self.settings.compiler.cppstd:
            check_min_cppstd(self, 23)

    def generate(self):
        tc = CMakeToolchain(self)

        tc.variables["EVGET_BUILD_BIN"] = self.options.build_bin
        tc.variables["BUILD_TESTING"] = self.options.build_testing
        tc.variables["EVGET_RUN_CLANG_TIDY"] = self.options.run_clang_tidy
        tc.variables["EVGET_CLANG_TIDY_FIX_ERRORS"] = self.options.clang_tidy_fix_errors
        tc.variables["EVGET_INSTALL_BIN"] = self.options.install_bin
        tc.variables["EVGET_INSTALL_LIB"] = self.options.install_lib

        if self.options.clang_tidy_executable:
            tc.variables["EVGET_CLANG_TIDY_EXECUTABLE"] = (
                self.options.clang_tidy_executable
            )
        if self.options.compiler_launcher:
            tc.variables["CMAKE_C_COMPILER_LAUNCHER"] = self.options.compiler_launcher
            tc.variables["CMAKE_CXX_COMPILER_LAUNCHER"] = self.options.compiler_launcher
        if self.options.export_compilation_database:
            tc.variables["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        if self.options.verify_headers:
            tc.variables["CMAKE_VERIFY_INTERFACE_HEADER_SETS"] = True

        tc.generate()

    def package_info(self):
        self.cpp_info.requires = [
            "boost::algorithm",
            "boost::asio",
            "boost::uuid",
            "boost::numeric-conversion",
        ]

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
