[comment]: <> (MIT License)

[comment]: <> (Copyright &#40;c&#41; 2021 Marko Malenic)

[comment]: <> (Permission is hereby granted, free of charge, to any person obtaining a copy)

[comment]: <> (of this software and associated documentation files &#40;the "Software"&#41;, to deal)

[comment]: <> (in the Software without restriction, including without limitation the rights)

[comment]: <> (to use, copy, modify, merge, publish, distribute, sublicense, and/or sell)

[comment]: <> (copies of the Software, and to permit persons to whom the Software is)

[comment]: <> (furnished to do so, subject to the following conditions:)

[comment]: <> (The above copyright notice and this permission notice shall be included in all)

[comment]: <> (copies or substantial portions of the Software.)

[comment]: <> (THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR)

[comment]: <> (IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,)

[comment]: <> (FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE)

[comment]: <> (AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER)

[comment]: <> (LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,)

[comment]: <> (OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE)

[comment]: <> (SOFTWARE.)

# evget
Show system input events.

## Build
This project using [vcpkg] to download dependencies, and cmake to build using FindPackage. It's recommended
to use the CMakePresets.json file to configure the vcpkg toolchain. This following command can be used to
build the project, if `VCPKG_ROOT` has been set as an environment variable:

```sh
cmake -B build -S . --preset debug
```

[vcpkg]: https://learn.microsoft.com/en-us/vcpkg/users/buildsystems/cmake-integration

## Usage
```sh
./build/evget --help
```

xorg-dev xserver-xorg-dev