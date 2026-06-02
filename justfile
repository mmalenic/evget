# Only takes effect on Windows.
set windows-shell := ['cmd.exe', '/d', '/s', '/c']

import 'recipes/common.just'
import 'recipes/check.just'

mod linux 'recipes/linux.just'
mod windows 'recipes/windows.just'

# Top-level builds default to the host's backend and compiler. We use Ninja on windows to simplify recipes
# cross-platform.
profile_args := if os_family() == 'windows' { '-c tools.cmake.cmaketoolchain:generator=Ninja' } else { '' }
backend_args := ''
check_args := if os_family() == 'windows' { '-o evget/*:run_msvc_analyze=True' } else { '-o evget/*:run_clang_tidy=True' }

# Remove the build directory.
clean:
    cmake -E rm -rf build

# Update the lockfile, allowing cross platform dependencies to take effect from any platform.
update:
    python scripts/update_lockfile.py

# Run pre-commit and other lints.
lint:
    pre-commit run --all-files

# Generate documentation.
doc *opts='': (build opts)
    -git clone --depth 1 --branch v2.3.4 https://github.com/jothepro/doxygen-awesome-css.git build/doxygen-awesome-css
    cmake -E env CLANG_DATABASE_PATH=build/{{ capitalize(build_type) }} doxygen

# Package the last build's binaries. `linux::build` packages both backends; a plain `build` the host default.
package name='evget.tar.gz': rebuild
    cd build/{{ capitalize(build_type) }} && cmake -E tar czf {{ name }} bin && cmake -E sha256sum {{ name }} > {{ name }}.sha256

# Install the last build's targets.
install: rebuild
    cmake --install build/{{ capitalize(build_type) }}
