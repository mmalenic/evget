# MIT License
#
# Copyright (c) 2021 Marko Malenic
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

from conan.tools.cmake import cmake_layout
from conan import ConanFile


class App(ConanFile):
    name = 'evget'
    version = '0.1'
    description = 'Shows events from input devices.'
    url = 'https://github.com/mmalenic/evget.git'
    license = 'MIT'
    author = 'Marko Malenic'
    topics = 'ev', 'evdev', 'events'
    settings = 'os', 'arch', 'compiler', 'build_type'
    requires = 'boost/[^1.82]', 'fmt/[^9.1]', 'spdlog/[^1.11]', 'date/[^3.0]', 'zlib/[^1.2]'
    test_requires = 'gtest/cci.20210126'
    generators = 'CMakeDeps', 'CMakeToolchain'
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

    def layout(self):
        cmake_layout(self)

    def package_info(self):
        self.cpp_info.requires = ['boost::algorithm', 'boost::asio', 'boost::program_options']
