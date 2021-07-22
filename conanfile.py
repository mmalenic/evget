from conans import ConanFile


class App(ConanFile):
    settings = 'os', 'arch', 'compiler', 'build_type'
    requires = 'boost/1.76.0', 'spdlog/1.9.0'
    generators = 'cmake'
    options = {'build_tests': [True, False]}
    default_options = {'build_tests': True}

    def requirements(self):
        if self.options.build_tests:
            self.requires('gtest/cci.20210126')

    def package_info(self):
        self.cpp_info.requires = ['boost::program_options', 'boost::algorithm']