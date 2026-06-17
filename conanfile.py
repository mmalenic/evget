import os

from conan.tools.build import check_min_cppstd
from conan.tools.cmake import cmake_layout, CMakeToolchain, CMake
from conan.tools.system import package_manager
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
        "mbedtls/[^3]",
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
        # Whether to run MSVC /analyze.
        "run_msvc_analyze": [True, False],
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
        # Whether system packages like X11 or libudev are installed as well as regular dependencies.
        "require_system_packages": [True, False],
        # Whether to build the evgetx11 library.
        "build_evgetx11": [True, False],
        # Whether to build the evgetlibinput library.
        "build_evgetlibinput": [True, False],
        # Whether to build the evgetwindows library.
        "build_evgetwindows": [True, False],
    }
    default_options = {
        "build_bin": True,
        "build_testing": False,
        "run_clang_tidy": False,
        "clang_tidy_executable": None,
        "clang_tidy_fix_errors": False,
        "run_msvc_analyze": False,
        "compiler_launcher": None,
        "install_bin": True,
        "install_lib": True,
        "export_compilation_database": True,
        "verify_headers": False,
        "require_system_packages": True,
        "build_evgetx11": None,
        "build_evgetlibinput": None,
        "build_evgetwindows": None,
    }

    def config_options(self):
        is_wayland = os.getenv("WAYLAND_DISPLAY") is not None
        is_linux = self.settings.os == "Linux" or self.settings.os.subsystem == "wsl"
        is_windows = self.settings.os == "Windows"
        # Must compare with ==
        # https://github.com/conan-io/conan/issues/3524
        if self.options.build_evgetx11 == None:  # noqa: E711
            self.options.build_evgetx11 = not is_wayland and is_linux
        if self.options.build_evgetlibinput == None:  # noqa: E711
            self.options.build_evgetlibinput = is_wayland and is_linux
        if self.options.build_evgetwindows == None:  # noqa: E711
            self.options.build_evgetwindows = is_windows

    def configure(self):
        self.options["spdlog"].use_std_fmt = True

        # evget uses only basic SQL
        sqlite_opts = self.options["sqlite3"]
        sqlite_opts.enable_rtree = False
        sqlite_opts.enable_math_functions = False
        sqlite_opts.enable_unlock_notify = False
        sqlite_opts.build_executable = False

        # Every compiled boost library.
        boost_components = frozenset(
            {
                "atomic",
                "charconv",
                "chrono",
                "cobalt",
                "container",
                "context",
                "contract",
                "coroutine",
                "date_time",
                "exception",
                "fiber",
                "filesystem",
                "graph",
                "graph_parallel",
                "iostreams",
                "json",
                "locale",
                "log",
                "math",
                "mpi",
                "nowide",
                "process",
                "program_options",
                "python",
                "random",
                "regex",
                "serialization",
                "stacktrace",
                "system",
                "test",
                "thread",
                "timer",
                "type_erasure",
                "url",
                "wave",
            }
        )
        # Boost libraries that evget needs.
        boost_evget_requires = frozenset(
            {"atomic", "chrono", "date_time", "exception", "random", "system"}
        )
        boost_opts = self.options["boost"]
        for component in boost_components - boost_evget_requires:
            boost_opts.__setattr__(f"without_{component}", True)

    def requirements(self):
        if self.options.build_evgetx11:
            if self.options.require_system_packages:
                self.requires("xorg/system")
        if self.options.build_evgetlibinput:
            if self.options.require_system_packages:
                self.requires("libudev/system")

            self.requires("libevdev/[^1]")
            self.requires("xkbcommon/[^1]")
            self.requires("wayland/[^1]")
            self.requires("libdrm/[^2]")

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.30 <4]")
        self.tool_requires("ninja/[^1.11]")

    def system_requirements(self):
        # The conan libinput package has a broken runtime paths for device quirks, so install a
        # system package here instead if requested.
        if self.options.build_evgetlibinput:
            apt = package_manager.Apt(self)
            apt.install(["libinput-dev"], update=True, check=True)

            dnf = package_manager.Dnf(self)
            dnf.install(["libinput-devel"], update=True, check=True)

            yum = package_manager.Yum(self)
            yum.install(["libinput-devel"], update=True, check=True)

            pacman = package_manager.PacMan(self)
            pacman.install(["libinput"], update=True, check=True)

            zypper = package_manager.Zypper(self)
            zypper.install(["libinput-devel"], update=True, check=True)

    def validate(self):
        if self.settings.compiler.cppstd:
            check_min_cppstd(self, 23)

    def generate(self):
        tc = CMakeToolchain(self)

        tc.cache_variables["EVGET_BUILD_BIN"] = self.options.build_bin
        tc.cache_variables["BUILD_TESTING"] = self.options.build_testing
        tc.cache_variables["EVGET_RUN_CLANG_TIDY"] = self.options.run_clang_tidy
        tc.cache_variables["EVGET_CLANG_TIDY_FIX_ERRORS"] = self.options.clang_tidy_fix_errors
        tc.cache_variables["EVGET_RUN_MSVC_ANALYZE"] = self.options.run_msvc_analyze
        tc.cache_variables["EVGET_INSTALL_BIN"] = self.options.install_bin
        tc.cache_variables["EVGET_INSTALL_LIB"] = self.options.install_lib
        tc.cache_variables["EVGET_BUILD_EVGETX11"] = self.options.build_evgetx11
        tc.cache_variables["EVGET_BUILD_EVGETLIBINPUT"] = self.options.build_evgetlibinput
        tc.cache_variables["EVGET_BUILD_EVGETWINDOWS"] = self.options.build_evgetwindows

        if self.options.clang_tidy_executable:
            tc.cache_variables["EVGET_CLANG_TIDY_EXECUTABLE"] = (
                self.options.clang_tidy_executable
            )
        if self.options.compiler_launcher:
            tc.cache_variables["CMAKE_C_COMPILER_LAUNCHER"] = self.options.compiler_launcher
            tc.cache_variables["CMAKE_CXX_COMPILER_LAUNCHER"] = self.options.compiler_launcher
        if self.options.export_compilation_database:
            tc.cache_variables["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        if self.options.verify_headers:
            tc.cache_variables["CMAKE_VERIFY_INTERFACE_HEADER_SETS"] = True

        # No need to scan for modules in this project.
        tc.cache_variables["CMAKE_CXX_SCAN_FOR_MODULES"] = False

        tc.generate()

    def package_info(self):
        self.cpp_info.requires = [
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
