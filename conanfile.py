from conans import ConanFile, CMake


class App(ConanFile):
    name = 'evget'
    settings = 'os', 'arch', 'compiler', 'build_type'
    requires = 'boost/1.76.0', 'spdlog/1.9.0'
    generators = 'cmake'
    options = {
        'build_tests': [True, False],
        'download_dependencies': [True, False],
        'shared': [True, False]
    }
    default_options = {
        'build_tests': True,
        'download_dependencies': True,
        'shared': True
    }

    def requirements(self):
        if self.options.build_tests:
            self.requires('gtest/cci.20210126')

    def package_info(self):
        self.cpp_info.requires = ['boost::program_options', 'boost::algorithm']

    def build(self):
        cmake = CMake()

        cmake.definitions['BUILD_TESTING'] = self.options.build_tests
        cmake.definitions['DOWNLOAD_DEPENDENCIES'] = self.options.download_dependencies

        if self.options.build_tests:
            cmake.test()

        cmake.build()
