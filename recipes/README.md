# `just` recipes

This directory contains the [`just`](https://just.systems) recipes for evget. Recipes
are organised into three levels, including the top-level justfile. The top-level justfile
auto-detects the compiler and OS, so commands like `just build` or `just run` will use the
system default.

There are OS-specific modules, and then compiler-specific modules that will run a command using
that OS or compiler explicitly. For example, `just linux::build` explicitly builds evgetx11 and
evgetlibinput using the default compiler, whereas `just linux::clang::build` builds the linux
components using clang. Some compilers have additional commands available, such as static analysis
or memory sanitizers.

To see a list of available commands, run `just --list` with an optional module:

```shell
just --list
just --list linux
just --list linux::clang
```

Each compiler will also use it's own build directory to not conflict with other builds. Common
recipes supported by all modules include `build`, `run` and `test`. `check` runs clang-tidy for
clang-based compilers or MSVC `/analyze`.

Compiler-specific address and memory sanitizers are also available. Note that clang requires re-building LLVM
for memory sanitizer (see [sanitizers.py](../scripts/sanitizers.py) for details). Windows hosts can use
runtime checks like pageheap and appverifier.

## Conventions

- Pass extra Conan flags through `*opts`, e.g. `just windows::msvc::build -o evget/*:build_bin=False`.
- Override the build type with `just build_type=Release <recipe>`.
- Run a subset of tests with a filter `just test 'MouseClick*'`.
- CLI arguments need to be quoted through `run`, e.g. `just run '--filter mouse,keyboard'`.
